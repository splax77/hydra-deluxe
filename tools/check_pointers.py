"""Check that the files the agent docs and memory notes point at still exist.

A pointer is a name a document gives for a file: a path, or the bare name of a hook
script. This script reads every pointer and lists the ones that no longer lead anywhere,
the way the docs-vs-code test does for numbers. It never fixes a document.

    py -I tools/check_pointers.py [--memory <folder>] [--gates <folder>] [--repo <root>] [--session-start]

documents() says which files are read, Pointers.judge() how one pointer is judged, and
is_pointer() which names count. Each broken pointer prints as
"file:line: pointer (missing | retired | found at <other place>)", then one summary line.
Exit 0 when nothing is broken, 1 when something is, 2 on a bad argument. With
--session-start (the SessionStart hook's form) it always exits 0 and prints a hook reply
only when something is broken, or when the check itself failed.
"""

import argparse
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import doc_tokens  # noqa: E402  (beside this script; -I keeps the script folder off the path)
import check_handoff  # noqa: E402  (its git() and Repo are the one home of the git calls)

MEMORY_DEFAULT = r"C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory"
GATES_DEFAULT = r"C:\Users\Patrick\.claude\gates"
HOOK_EXTENSIONS = (".py", ".ps1", ".sh")
# Subfolders of the gates folder that hold hook scripts and their tests.
HOOK_FOLDERS = ("", "lib", "tests")
SKIP_FOLDERS = {".git", "__pycache__", "state"}
SESSIONS_LINE = "Sessions:"


def documents(repo_root, memory):
    """(name to show, full path) of every document read. The memory folder is read at its top
    only: its archive subfolder holds retired notes, which name retired things on purpose."""
    docs = [("CLAUDE.md", os.path.join(repo_root, "CLAUDE.md"))]
    agents = os.path.join(repo_root, "docs", "agents")
    for folder, subs, files in os.walk(agents):
        subs.sort()
        for f in sorted(files):
            if f.endswith(".md"):
                full = os.path.join(folder, f)
                docs.append((os.path.relpath(full, repo_root), full))
    for f in sorted(os.listdir(memory)):
        full = os.path.join(memory, f)
        if f.endswith(".md") and os.path.isfile(full):
            docs.append((full, full))
    return [d for d in docs if os.path.isfile(d[1])]


def is_hook_name(p):
    return not doc_tokens.has_separator(p) and os.path.splitext(p)[1].lower() in HOOK_EXTENSIONS


def _inside(folder, full):
    rel = os.path.relpath(full, folder)
    return rel != "." and rel != ".." and not rel.startswith(".." + os.sep)


class Pointers:
    def __init__(self, repo_root, gates):
        self.repo = check_handoff.Repo(repo_root)
        self.gates = gates
        self.hooks_root = os.path.dirname(os.path.normpath(gates))
        self.old_hooks = os.path.join(self.hooks_root, "hooks")
        self._refs = None
        self._index = None
        self._old_names = None

    def is_pointer(self, t):
        """Whether a token names a file: the tokeniser's path shape, minus the names that only
        look like one. Those are a name with a colon past the drive (a registry key), one that
        starts with a slash (a slash command, a widget id, a compiler flag or a URL's path; no
        doc here names a file that way on Windows), one with an elided "..." part, a git ref,
        and a remote repo's owner/name as a remote URL of either repo spells it."""
        if doc_tokens.shape(t) != "path":
            return False
        p = doc_tokens.strip_line_ref(t)
        if ":" in (p[2:] if doc_tokens.is_absolute(p) else p) or p.startswith(("/", "\\")):
            return False
        if "..." in p.replace("\\", "/").split("/"):
            return False
        if self._refs is None:
            hooks = check_handoff.Repo(self.hooks_root)
            self._refs = set(self.repo.branches()) | set(hooks.branches())
            for repo in (self.repo, hooks):
                for url in check_handoff.git(repo.root, ["remote", "-v"])[1].split():
                    slug = url[:-4] if url.endswith(".git") else url
                    self._refs.add("/".join(slug.replace(":", "/").split("/")[-2:]))
        return t not in self._refs

    def judge(self, t, doc_folder):
        """None when the pointer leads to a live file as written, else why it is broken. A
        relative path may be written from the document's own folder, the repo's root or the
        hooks repo's root; a bare hook name may also sit in a hook folder. Only the path as
        written counts, in the case the folders hold, so a file that merely ends the same way
        is a hint and never a pass."""
        p = doc_tokens.strip_line_ref(t)
        if doc_tokens.is_absolute(p):
            full = os.path.normpath(os.path.expanduser(p))
            drive, rest = os.path.splitdrive(full)
            if os.path.exists(full) and self._exact_case(drive + os.sep, rest):
                return self._live(full)
            return self._broken(p)
        roots = [doc_folder, self.repo.root, self.hooks_root]
        if is_hook_name(p):
            roots += [os.path.join(self.gates, f) for f in HOOK_FOLDERS]
        for root in roots:
            full = os.path.normpath(os.path.join(root, p))
            if os.path.exists(full) and self._exact_case(root, os.path.relpath(full, root)):
                return self._live(full)
        return self._broken(p)

    def _exact_case(self, root, rel):
        """Whether rel is spelled below root in the case the folders hold; on Windows a file
        opens under any case, so a wrong-case pointer would pass here and fail elsewhere."""
        rel = rel.replace("\\", "/").strip("/")
        if rel in ("", "."):
            return True
        if rel.startswith("../"):
            drive, rest = os.path.splitdrive(os.path.normpath(os.path.join(root, rel)))
            root, rel = drive + os.sep, rest.replace("\\", "/").strip("/")
        return check_handoff.Repo(root).exact_case(rel)

    def _live(self, full):
        return "retired" if _inside(self.old_hooks, full) else None

    def _broken(self, p):
        written = p.replace("\\", "/").strip("/").lower()
        base = written.rsplit("/", 1)[-1]
        index = self.index()
        for test in (lambda k: k == written, lambda k: k.endswith("/" + written),
                     lambda k: k.rsplit("/", 1)[-1] == base):
            near = [shown for key, shown in index if test(key)]
            if near:
                return "found at %s%s" % (near[0], " and %d more" % (len(near) - 1) if len(near) > 1 else "")
        if base in self.old_names():
            return "retired"
        return "missing"

    def index(self):
        """(lower-case key, name to show) of every live file a hint may name: the repo's tracked
        files, and the gates folder and top-level files of the hooks repo."""
        if self._index is None:
            self._index = [(f.lower(), f) for f in self.repo.files()]
            for folder, subs, files in os.walk(self.hooks_root):
                rel_folder = os.path.relpath(folder, self.hooks_root)
                if rel_folder == ".":
                    subs[:] = [s for s in subs if s == os.path.basename(self.gates)]
                else:
                    subs[:] = [s for s in subs if s not in SKIP_FOLDERS]
                for f in files:
                    full = os.path.join(folder, f)
                    key = os.path.relpath(full, self.hooks_root).replace("\\", "/").lower()
                    self._index.append((key, full))
        return self._index

    def old_names(self):
        """Lower-case names of the files in the old hooks folder."""
        if self._old_names is None:
            self._old_names = set()
            for folder, subs, files in os.walk(self.old_hooks):
                subs[:] = [s for s in subs if s not in SKIP_FOLDERS]
                self._old_names.update(f.lower() for f in files)
        return self._old_names


def check(repo_root, memory, gates):
    """(broken lines, number of pointers checked)."""
    pointers = Pointers(repo_root, gates)
    broken, checked, verdicts = [], 0, {}
    for shown, full in documents(repo_root, memory):
        with open(full, encoding="utf-8", errors="replace") as f:
            text = f.read()
        lines = text.splitlines()
        folder = os.path.dirname(full)
        for tok in doc_tokens.tokens(text):
            if lines[tok.line - 1].lstrip().startswith(SESSIONS_LINE) or not pointers.is_pointer(tok.text):
                continue
            checked += 1
            key = (tok.text, folder)
            if key not in verdicts:
                verdicts[key] = pointers.judge(tok.text, folder)
            if verdicts[key]:
                broken.append("%s:%d: %s (%s)" % (shown, tok.line, tok.text, verdicts[key]))
    return broken, checked


def summary(broken, checked):
    return "pointers: %d checked, %d broken" % (checked, len(broken))


def folders(args):
    """The repo root, or a ValueError naming the argument that is wrong."""
    for flag, folder in (("--memory", args.memory), ("--gates", args.gates)):
        if not os.path.isdir(folder):
            raise ValueError("%s is not a folder: %s" % (flag, folder))
    root = args.repo or check_handoff.git(os.path.dirname(os.path.abspath(__file__)),
                                          ["rev-parse", "--show-toplevel"])[1].strip()
    if not root or not os.path.isdir(root) or check_handoff.git(root, ["rev-parse", "--git-dir"])[0] != 0:
        raise ValueError("not a git repo: %s" % root)
    return os.path.abspath(root)


def main(argv):
    for stream in (sys.stdout, sys.stderr):
        stream.reconfigure(errors="replace")
    ap = argparse.ArgumentParser(prog="check_pointers.py", description=__doc__.split("\n\n")[0])
    ap.add_argument("--memory", default=MEMORY_DEFAULT, help="memory notes folder")
    ap.add_argument("--gates", default=GATES_DEFAULT, help="the hooks repo's gates folder")
    ap.add_argument("--repo", default=None, help="Hydra checkout (default: the one holding this script)")
    ap.add_argument("--session-start", action="store_true", help="reply as a SessionStart hook")
    args = ap.parse_args(argv)
    if args.session_start:
        try:
            broken, checked = check(folders(args), args.memory, args.gates)
        except Exception as e:  # a hook that fails says so and never blocks the session
            print(json.dumps({"systemMessage": "check_pointers.py could not check the doc pointers, so "
                              "nothing was checked: %s: %s" % (type(e).__name__, e)}))
            return 0
        if broken:
            context = ("Broken file pointers in the agent docs and memory notes "
                       "(tools/check_pointers.py). Fix them alongside the user's request:\n"
                       + "\n".join(broken) + "\n" + summary(broken, checked))
            print(json.dumps({"hookSpecificOutput": {"hookEventName": "SessionStart",
                                                     "additionalContext": context}}))
        return 0
    try:
        root = folders(args)
    except ValueError as e:
        ap.error(str(e))
    broken, checked = check(root, args.memory, args.gates)
    for line in broken:
        print(line)
    print(summary(broken, checked))
    return 1 if broken else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
