"""Check a handoff's claims against what exists in the repo, before any work starts from it.

A handoff is another agent's summary, so nothing in it is trusted until it is checked.
This script checks that the commits, branches, paths and functions a handoff names exist.
It does not judge what a claim says about them; the session still reads the code.

    py -I tools/check_handoff.py <handoff.md> [--repo <root>] [--since <YYYY-MM-DD>]

docs/agents/handoff.md is the guide to the handoff shape, the two modes and what each
prints. A handoff with a "## Claims" heading is read in table mode, row by row; any other
handoff is scraped for names (doc_tokens.tokens is the tokeniser). Exit codes: 0 when
every check passed, 1 when one failed, 2 on a bad argument.
"""

import argparse
import datetime
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import doc_tokens  # noqa: E402  (beside this script; -I keeps the script folder off the path)

_GIT_PINS = ["-c", "core.quotePath=false", "-c", "log.showSignature=false", "-c", "core.useReplaceRefs=false"]
_GIT_ENV_DROP = ("GIT_DIR", "GIT_WORK_TREE", "GIT_INDEX_FILE", "GIT_COMMON_DIR", "GIT_PREFIX",
                 "GIT_REPLACE_REF_BASE")
_FULL_HASH = re.compile(r"^[0-9a-fA-F]{40}$")
_DATE = re.compile(r"\d{4}-\d{2}-\d{2}")
_IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
CLAIMS_HEADING = "## Claims"
MANUAL_KINDS = ("test", "ruling", "state")
TABLE_STATUSES = ("ok", "missing", "not-on-main", "not-pushed", "manual")


def git(repo, args):
    """(exit code, stdout text) of one git command in repo, with the caller's GIT_* folder
    settings dropped so they cannot point it at another repo."""
    env = {k: v for k, v in os.environ.items() if k not in _GIT_ENV_DROP}
    r = subprocess.run(["git", "-C", repo, *_GIT_PINS, *args], capture_output=True, env=env)
    return r.returncode, r.stdout.decode("utf-8", "replace")


def _same(a, b):
    return os.path.normcase(os.path.normpath(a)) == os.path.normcase(os.path.normpath(b))


class Repo:
    """One repo the handoff is checked against, with git answers cached for the run."""

    def __init__(self, root, label=""):
        self.root = root
        self.label = label  # "" for the main repo, "in <root>: " for another one
        self._branches = None
        self._files = None

    def common_dir(self):
        code, out = git(self.root, ["rev-parse", "--path-format=absolute", "--git-common-dir"])
        return out.strip() if code == 0 else None

    def commit(self, h):
        code, out = git(self.root, ["rev-parse", "--verify", "--quiet", h + "^{commit}"])
        return out.strip() if code == 0 else None

    def subject(self, rev):
        return git(self.root, ["log", "-1", "--format=%s", rev])[1].strip()

    def contains(self, ref, full):
        """Whether ref exists and full is one of its ancestors."""
        if git(self.root, ["rev-parse", "--verify", "--quiet", ref])[0] != 0:
            return False
        return git(self.root, ["merge-base", "--is-ancestor", full, ref])[0] == 0

    def branches(self):
        """Short branch name -> full ref, local and remote-tracking."""
        if self._branches is None:
            out = git(self.root, ["for-each-ref", "--format=%(refname)", "refs/heads", "refs/remotes"])[1]
            self._branches = {}
            for ref in out.split():
                short = ref[len("refs/heads/"):] if ref.startswith("refs/heads/") else ref[len("refs/remotes/"):]
                if not short.endswith("/HEAD"):
                    self._branches.setdefault(short, ref)
        return self._branches

    def files(self):
        if self._files is None:
            self._files = [f for f in git(self.root, ["ls-files", "-z"])[1].split("\0") if f]
        return self._files

    def grep(self, name, ignore_case=False, docs=False, fixed=False):
        """(file, line, matched text) of every whole-word hit of name in tracked text files.
        Docs are searched only when docs is set, because a doc that mentions a function is
        no proof the function exists."""
        args = (["grep", "-n", "-I", "-o", "--full-name", "-w"] + (["-F"] if fixed else [])
                + (["-i"] if ignore_case else []) + ["-e", name])
        if not docs:
            args += ["--", ".", ":(exclude)*.md", ":(exclude)docs/"]
        hits = []
        for line in git(self.root, args)[1].splitlines():
            parts = line.split(":", 2)
            if len(parts) == 3:
                hits.append((parts[0], parts[1], parts[2]))
        return hits

    def changes_since(self, rel, since):
        """Subjects of the commits touching rel since midnight at the start of the date, on
        this checkout and on main (a checkout that has fallen behind main would miss them)."""
        revs = ["HEAD"]
        if git(self.root, ["rev-parse", "--verify", "--quiet", "refs/heads/main"])[0] == 0:
            revs.append("refs/heads/main")
        out = git(self.root, ["log", "--since=%s 00:00:00" % since, "--format=%s", *revs, "--", rel])[1]
        return [s for s in out.splitlines() if s.strip()]

    def exact_case(self, rel):
        """Whether rel is spelled in the case the folder holds. On Windows a file opens under
        any case, so a wrong-case path would pass here and fail on another machine."""
        cur = self.root
        for part in rel.split("/"):
            try:
                if part not in os.listdir(cur):
                    return False
            except OSError:
                return False
            cur = os.path.join(cur, part)
        return True

    def relative(self, absolute):
        """absolute as a path inside this repo, or None when it is outside."""
        try:
            rel = os.path.relpath(absolute, self.root)
        except ValueError:
            return None
        return None if rel == ".." or rel.startswith(".." + os.sep) else rel.replace(os.sep, "/")


def other_repos(text, main):
    """Line number -> the other repos whose root folder that line names."""
    main_common = main.common_dir()
    found, by_line = {}, {}
    for n, line in enumerate(text.splitlines(), 1):
        for p in doc_tokens.absolute_paths(line):
            p = os.path.normpath(p)
            key = os.path.normcase(p)
            if key not in found:
                found[key] = None
                if os.path.isdir(p):
                    code, top = git(p, ["rev-parse", "--show-toplevel"])
                    repo = Repo(p, "in %s: " % p)
                    if code == 0 and _same(top.strip(), p) and not _same(repo.common_dir() or "", main_common or ""):
                        found[key] = repo
            if found[key] is not None:
                by_line.setdefault(n, []).append(found[key])
    return by_line


# ---- Looking a name up. Each returns (where, repo, path for CHANGED SINCE) or None.

def find_commit(repo, t):
    full = repo.commit(t)
    if not full:
        return None
    on_main = "on main" if repo.contains("refs/heads/main", full) else "not on main"
    return '%scommit %s "%s", %s' % (repo.label, full[:10], repo.subject(full), on_main), None


def find_branch(repo, t):
    ref = repo.branches().get(t)
    if not ref:
        return None
    full = repo.commit(ref)
    return '%sbranch, tip %s "%s"' % (repo.label, full[:10], repo.subject(full)), None


def find_function(repo, t):
    """Where the function is: the qualified name as written when the code holds it, else the
    bare name, so a function moved to another file still shows its new home. A method
    written in its class (a definition inside the class body is not written qualified)
    is accepted only in a file that names the class, so a common method name does not
    pass on an unrelated hit."""
    name = doc_tokens.function_name(t) or (t if _IDENT.match(t) else None)
    if not name:
        return None
    qualified = t.split("(", 1)[0]
    hits = repo.grep(qualified, fixed=True) if "::" in qualified else []
    if not hits:
        hits = repo.grep(name)
        if "::" in qualified:
            owner = qualified.split("::")[-2]
            owner_files = {h[0] for h in repo.grep(owner)}
            hits = [h for h in hits if h[0] in owner_files]
    if not hits:
        return None
    f, line, _ = hits[0]
    more = len({h[0] for h in hits}) - 1
    return "%s%s:%s%s" % (repo.label, f, line, " and %d more files" % more if more else ""), f


def find_path(repo, t, absolute):
    p = doc_tokens.strip_line_ref(t)
    if absolute:
        full = os.path.expanduser(p)
        if not os.path.exists(full):
            return None
        rel = repo.relative(full)
        return "%s %s" % ("folder" if os.path.isdir(full) else "file", full), rel
    # Only the path as written counts. A name that matches the tail of some other path
    # ("summary.md" under another folder) is a different file, so it is shown as the
    # nearest name of a missing path and never as found.
    full = os.path.join(repo.root, p.replace("\\", "/"))
    rel = repo.relative(full)
    if rel in (None, ".") or not os.path.exists(full) or not repo.exact_case(rel):
        return None
    return "%s%s %s" % (repo.label, "folder" if os.path.isdir(full) else "file", rel), rel


def nearest(repo, kind, t):
    """A case-only or extension-only match for a missing name, or ""."""
    if kind == "branch":
        near = [b for b in repo.branches() if b.lower() == t.lower()]
    elif kind == "function":
        near = ["%s at %s:%s" % (h[2], h[0], h[1]) for h in repo.grep(doc_tokens.function_name(t), True)]
        near += ["named only in docs, %s:%s" % (h[0], h[1]) for h in repo.grep(doc_tokens.function_name(t), docs=True)]
    elif kind == "path":
        p = doc_tokens.strip_line_ref(t).replace("\\", "/")
        if doc_tokens.is_absolute(p):
            folder, base = os.path.split(os.path.expanduser(p))
            names = os.listdir(folder) if os.path.isdir(folder) else []
            return _near_name(base, names, lambda n: os.path.join(folder, n))
        files = repo.files()
        if not doc_tokens.has_separator(p):
            return _near_name(p, files, lambda f: f, key=lambda f: f.rsplit("/", 1)[-1])
        near = _near_name(p, files, lambda f: f)
        if near:
            return near
        tail = [f for f in files if f.endswith("/" + p.strip("/"))]
        if tail:
            return "a different file ends the same way: %s%s" % (
                tail[0], " and %d more" % (len(tail) - 1) if len(tail) > 1 else "")
        return ""
    else:
        near = []
    return near[0] if near else ""


def _near_name(name, candidates, show, key=lambda c: c):
    low, stem = name.lower(), os.path.splitext(name)[0].lower()
    for c in candidates:
        if key(c).lower() == low:
            return show(c)
    for c in candidates:
        if os.path.splitext(key(c))[0].lower() == stem:
            return show(c)
    return ""


def classify(t, repos):
    if doc_tokens.is_commit(t):
        return "commit"
    if doc_tokens.is_branch_shape(t) or any(t in r.branches() for r in repos):
        return "branch"
    return doc_tokens.shape(t)


def look_up(kind, t, repos):
    for repo in repos:
        if kind == "commit":
            hit = find_commit(repo, t)
        elif kind == "branch":
            hit = find_branch(repo, t)
        elif kind == "function":
            hit = find_function(repo, t)
        else:
            hit = find_path(repo, t, doc_tokens.is_absolute(doc_tokens.strip_line_ref(t)))
        if hit:
            return hit[0], repo, hit[1]
    return None


# ---- Scrape mode

def scrape(text, main, since):
    by_line = other_repos(text, main)
    claims = {}  # token -> [first line, repos to search]
    for tok in doc_tokens.tokens(text):
        extra = by_line.get(tok.line, [])
        if tok.text in claims:
            claims[tok.text][1] += [r for r in extra if r not in claims[tok.text][1]]
        else:
            claims[tok.text] = [tok.line, [main] + extra]
    exists, missing, changed, seen = [], [], [], set()
    for t, (line, repos) in claims.items():
        kind = classify(t, repos)
        if kind is None:
            continue
        hit = look_up(kind, t, repos)
        if hit is None:
            near = nearest(main, kind, t)
            missing.append("%s  line %d: %s not found%s" % (t, line, kind, "; nearest: " + near if near else ""))
            continue
        where, repo, rel = hit
        exists.append("%s  line %d: %s" % (t, line, where))
        if rel and (repo.root, rel) not in seen:
            seen.add((repo.root, rel))
            subjects = repo.changes_since(rel, since)
            if subjects:
                changed.append('%s%s: %d commit%s, newest "%s"' % (
                    rel, "" if repo is main else " (in %s)" % repo.root,
                    len(subjects), "" if len(subjects) == 1 else "s", subjects[0]))
    for title, lines in (("EXISTS", exists), ("MISSING", missing), ("CHANGED SINCE " + since, changed)):
        print(title)
        for line in lines or ["(none)"]:
            print("  " + line)
    print("checked %d claims: %d exist, %d missing, %d changed since %s"
          % (len(exists) + len(missing), len(exists), len(missing), len(changed), since))
    return 1 if missing else 0


# ---- Table mode

def split_row(line):
    """The cells of one markdown table row; a "|" inside backticks stays in its cell."""
    cells, cell, ticks = [], "", False
    for ch in line.strip().strip("|"):
        if ch == "`":
            ticks = not ticks
        if ch == "|" and not ticks:
            cells.append(cell.strip())
            cell = ""
        else:
            cell += ch
    cells.append(cell.strip())
    return cells


def claims_rows(text):
    """(line number, cells) of every data row under the "## Claims" heading."""
    lines, rows, inside = text.splitlines(), [], False
    for n, line in enumerate(lines, 1):
        if line.startswith("#"):
            inside = line.rstrip() == CLAIMS_HEADING
            continue
        # A row written without its outer pipes still counts: dropping it would be a claim
        # that is never checked.
        if inside and (line.lstrip().startswith("|") or line.count("|") >= 2):
            cells = split_row(line)
            if cells[0].lower() == "kind" or all(re.fullmatch(r":?-+:?", c) for c in cells if c):
                continue
            rows.append((n, cells))
    return rows


def row_status(kind, value, repos):
    if kind in MANUAL_KINDS:
        return "manual"
    if kind == "commit":
        if not _FULL_HASH.match(value):
            return "missing"
        for repo in repos:
            full = repo.commit(value)
            if full:
                if not repo.contains("refs/heads/main", full):
                    return "not-on-main"
                return "ok" if repo.contains("refs/remotes/origin/main", full) else "not-pushed"
        return "missing"
    if kind in ("path", "branch", "function"):
        return "ok" if value and look_up(kind, value, repos) else "missing"
    return "unknown"


def table(text, main):
    by_line = other_repos(text, main)
    rows = claims_rows(text)
    if not rows:
        # A table nobody could read must not pass as "nothing to check".
        print("error: no rows under the %s heading" % CLAIMS_HEADING, file=sys.stderr)
    counts = {s: 0 for s in TABLE_STATUSES}
    for n, cells in rows:
        kind = cells[0].lower()
        raw = cells[1] if len(cells) > 1 else ""
        span = re.search(r"`([^`]+)`", raw)
        value = span.group(1).strip() if span else raw
        claim = cells[2] if len(cells) > 2 else ""
        status = row_status(kind, value, [main] + by_line.get(n, []))
        counts[status] = counts.get(status, 0) + 1
        print("%s | %s | %s | %s" % (status, kind, value, claim))
    parts = ["%d %s" % (counts[s], s) for s in TABLE_STATUSES]
    if counts.get("unknown"):
        parts.append("%d unknown" % counts["unknown"])
    print("checked %d rows: %s" % (len(rows), ", ".join(parts)))
    if not rows:
        return 1
    return 0 if all(c == 0 for s, c in counts.items() if s not in ("ok", "manual")) else 1


# ---- Command line

def handoff_date(path):
    """The handoff's date: the file name's, else its git author date, else today with a warning."""
    m = _DATE.search(os.path.basename(path))
    if m:
        try:
            return datetime.date.fromisoformat(m.group(0)).isoformat()
        except ValueError:
            pass
    folder, name = os.path.split(os.path.abspath(path))
    code, out = git(folder, ["log", "-1", "--format=%as", "--", name])
    if code == 0 and out.strip():
        return out.strip()
    today = datetime.date.today().isoformat()
    print("warning: %s has no date in its name and no git date; changes are counted since today, %s"
          % (path, today), file=sys.stderr)
    return today


def main(argv):
    here = os.path.dirname(os.path.abspath(__file__))
    # A commit subject or claim with a character the console cannot show must not crash a
    # run half way through its output.
    for stream in (sys.stdout, sys.stderr):
        stream.reconfigure(errors="replace")
    ap =argparse.ArgumentParser(prog="check_handoff.py", description=__doc__.split("\n\n")[0])
    ap.add_argument("handoff")
    ap.add_argument("--repo", default=None, help="repo to check against (default: the one holding this script)")
    ap.add_argument("--since", default=None, help="YYYY-MM-DD (default: the handoff's date)")
    args = ap.parse_args(argv)
    if not os.path.isfile(args.handoff):
        ap.error("no such handoff file: %s" % args.handoff)
    if args.since is not None:
        try:
            datetime.date.fromisoformat(args.since)
        except ValueError:
            ap.error("--since takes a date written YYYY-MM-DD, not %r" % args.since)
    root = args.repo
    if root is None:
        code, out = git(here, ["rev-parse", "--show-toplevel"])
        root = out.strip() if code == 0 else here
    if not os.path.isdir(root) or git(root, ["rev-parse", "--git-dir"])[0] != 0:
        ap.error("not a git repo: %s" % root)
    with open(args.handoff, encoding="utf-8", errors="replace") as f:
        text = f.read()
    repo = Repo(os.path.abspath(root))
    if any(line.rstrip() == CLAIMS_HEADING for line in text.splitlines()):
        return table(text, repo)
    return scrape(text, repo, args.since or handoff_date(args.handoff))


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
