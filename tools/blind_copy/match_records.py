"""Join each old audit record to the test it names, and flag its lines 9 and 11.

    py -I tools/blind_copy/match_records.py --out <folder>
        [--copy C:\\Users\\Patrick\\Downloads\\Hydra\\hydra-blind]
        [--repo <git repo>] [--records-commit a4ad3306]

The records are read with git at --records-commit, from the records folder
of the November 14 test audit; nothing is checked out. The tests the
headings are matched against are read from the stripped copy (--copy), the
copy of commit 539c6de0 that the recheck reads, under its `code/` folder.

It writes `matched.tsv` (one row per record) and `unmatched.tsv` (the records
that join no test, with their state) into --out. The rules it applies are the
plan's section "T2: Join each test to its newest old record"
(docs/superpowers/plans/2026-11-20-script-first-recheck.md); this script adds
none of its own beyond what its comments name.

It stops, writing nothing, when one of its counts disagrees with git's, when
a line 9 row sits where no record of the plan's heading rule reaches it, or
when blame times cannot tell any repeated test's copies apart.
"""

import argparse
import ast
import collections
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402  (copy_file owns the copy's layout)
import grade_seams  # noqa: E402  (top_units is the C++ test-name reader)

RECORDS_COMMIT = "a4ad3306"
TESTS_COMMIT = "539c6de0"
COPY = r"C:\Users\Patrick\Downloads\Hydra\hydra-blind"
RECORDS_DIR = "docs/audit/2026-11-14-test-audit/records/"
REPO = os.path.dirname(os.path.dirname(HERE))

# One pattern each, used both for the script's own reading and for the
# `git grep` count it is checked against.
HEADING = "^## "
LINE9 = "^9 Stand-in:"
LINE11 = "^11 Real:"
LEVEL1 = "^# "

HEAD_RE = re.compile(r"^## (?P<rest>.*) \((?P<inner>[^()]*)\)\s*$")
PLACE_RE = re.compile(r"^(?P<file>.+):(?P<line>\d[\d, -]*)$")

SAME_PLACE = "same test, same place"
MOVED = "same test, moved"
GONE = "test gone"
NO_TEST = "record names no test the script can find"

FLAGGED = "flagged"
SOMETHING_ELSE = "says something else"
# A tie keeps both copies and the test takes the worse one; flagged is worse.
WORSE = {FLAGGED: 0, SOMETHING_ELSE: 1}

CPP_EXT = (".cpp", ".h", ".hpp", ".cc", ".cxx")


class Stop(Exception):
    pass


def git(repo, *args):
    r = subprocess.run(["git", "-C", repo, *args], capture_output=True, text=True,
                       encoding="utf-8", errors="replace")
    return r.returncode, r.stdout


def grep_count(repo, commit, pattern, folder):
    code, out = git(repo, "grep", "-c", "-E", pattern, commit, "--", folder)
    if code not in (0, 1):
        raise Stop(f"git grep failed for {pattern!r} at {commit}")
    total = 0
    for line in out.splitlines():
        total += int(line.rsplit(":", 1)[1])
    return total


class Record:
    def __init__(self, file, line, heading):
        self.file = file
        self.line = line
        self.heading = heading
        self.line9 = None   # (record line, text after the label)
        self.line11 = None
        self.test_file = ""
        self.test_name = ""
        self.test_lines = []
        self.match = NO_TEST
        self.flag = SOMETHING_ELSE
        self.why = ""
        self.newest = ""
        self.test_state = ""
        self.time9 = None


def read_records(repo, commit, folder):
    """Every `## ` heading opens a record that runs to the next `## ` or `# `.
    Returns the records and the line 9 and line 11 rows no record owns."""
    code, out = git(repo, "ls-tree", "--name-only", commit, folder)
    if code != 0:
        raise Stop(f"git ls-tree failed for {folder} at {commit}")
    records, loose9, loose11 = [], [], []
    for path in out.splitlines():
        code, text = git(repo, "show", f"{commit}:{path}")
        if code != 0:
            raise Stop(f"git show failed for {path} at {commit}")
        name = path.rsplit("/", 1)[-1]
        cur = None
        for n, raw in enumerate(text.split("\n"), 1):
            line = raw.rstrip("\r")
            if re.match(HEADING, line):
                cur = Record(name, n, line[3:])
                records.append(cur)
                continue
            if re.match(LEVEL1, line):
                cur = None
                continue
            for pat, attr, loose in ((LINE9, "line9", loose9), (LINE11, "line11", loose11)):
                m = re.match(pat, line)
                if not m:
                    continue
                if cur is None or getattr(cur, attr) is not None:
                    loose.append(f"{name}:{n}")
                else:
                    setattr(cur, attr, (n, line[m.end():].strip()))
    return records, loose9, loose11


def unescape(s):
    return re.sub(r"\\(.)", r"\1", s)


def python_names(text):
    try:
        tree = ast.parse(text)
    except SyntaxError:
        return []
    out = []
    for node in tree.body:
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            out.append((node.name, node.lineno))
        if isinstance(node, ast.ClassDef):
            for sub in node.body:
                if isinstance(sub, (ast.FunctionDef, ast.AsyncFunctionDef)):
                    out.append((sub.name, sub.lineno))
    return out


def test_names(text, path):
    """(name, line) for each test or helper in one file. C++ goes through
    grade_seams.top_units, the existing reader; Python through its ast."""
    if path.endswith(".py"):
        return python_names(text)
    if path.endswith(CPP_EXT):
        return [(unescape(u.name), u.start) for u in grade_seams.top_units(path, text) if u.name]
    return []


class Tests:
    """The test files of the stripped copy, read once each on demand. The
    copy is read, not the commit itself, because top_units names a unit by
    its first line, and a comment above a TEST_CASE would become that line."""

    def __init__(self, copy):
        self.copy, self.cache = copy, {}

    def names(self, path):
        if path not in self.cache:
            real = ff.copy_file(self.copy, "code/" + path)
            if real is None or not os.path.isfile(real):
                self.cache[path] = None
            else:
                with open(real, encoding="utf-8", errors="replace") as f:
                    text = f.read()
                self.cache[path] = collections.defaultdict(list)
                for name, line in test_names(text, path):
                    self.cache[path][name].append(line)
        return self.cache[path]


def match(rec, tests):
    m = HEAD_RE.match("## " + rec.heading)
    p = PLACE_RE.match(m.group("inner").strip()) if m else None
    if not p:
        return
    rest, file = m.group("rest").strip(), p.group("file").strip()
    first = int(re.match(r"\d+", p.group("line")).group(0))
    rec.test_file = file
    names = tests.names(file)
    if names is None:
        rec.match = GONE
        return
    candidates = [rest]
    if ": " in rest:
        candidates.append(rest.split(": ", 1)[1])
    for cand in candidates:
        if cand in names:
            rec.test_name = cand
            rec.test_lines = names[cand]
            rec.match = SAME_PLACE if first in names[cand] else MOVED
            return
    rec.match = NO_TEST


def flag(rec):
    missing = [n for n, row in (("line 9", rec.line9), ("line 11", rec.line11)) if row is None]
    if missing:
        rec.flag, rec.why = SOMETHING_ELSE, "no " + " and no ".join(missing)
        return
    by = []
    if rec.line9[1].lower().startswith("none"):
        by.append("line 9")
    if rec.line11[1].lower().startswith("yes"):
        by.append("line 11")
    rec.flag = FLAGGED if by else SOMETHING_ELSE
    rec.why = " and ".join(by)


def blame_times(repo, commit, path):
    code, out = git(repo, "blame", "--line-porcelain", commit, "--", path)
    if code != 0:
        raise Stop(f"git blame failed for {path} at {commit}")
    times, ln = {}, None
    for line in out.split("\n"):
        m = re.match(r"^[0-9a-f]{40} \d+ (\d+)", line)
        if m:
            ln = int(m.group(1))
        elif line.startswith("committer-time "):
            times[ln] = int(line.split()[1])
    return times


def pick_newest(records, repo, commit, folder):
    """The newest copy of a test is the one whose line 9 was committed last.
    A copy with no line 9 has no line 9 time, so it is never the newest
    while another copy has one. Returns three counts: tests with more than
    one copy, tests with two or more timed copies, and of those the tests
    whose timed copies all share one time."""
    by_test = collections.defaultdict(list)
    for r in records:
        if r.test_name:
            by_test[(r.test_file, r.test_name)].append(r)
    blames = {}
    repeated = timed_repeated = all_tied = 0
    for copies in by_test.values():
        timed = [r for r in copies if r.line9 is not None]
        if len(copies) > 1:
            repeated += 1
            for r in timed:
                if r.file not in blames:
                    blames[r.file] = blame_times(repo, commit, folder + r.file)
                r.time9 = blames[r.file][r.line9[0]]
            if len(timed) > 1:
                timed_repeated += 1
                if len({r.time9 for r in timed}) == 1:
                    all_tied += 1
        if len(copies) == 1:
            newest = copies
        elif timed:
            top = max(r.time9 for r in timed)
            newest = [r for r in timed if r.time9 == top]
        else:
            newest = copies
        for r in copies:
            if r in newest:
                r.newest = "tie" if len(newest) > 1 else "yes"
            else:
                r.newest = "no"
        state = min((r.flag for r in newest), key=WORSE.get)
        for r in copies:
            r.test_state = state
    return repeated, timed_repeated, all_tied


def cell(s):
    return str(s).replace("\t", " ").replace("\r", " ").replace("\n", " ")


COLUMNS = ["record", "heading", "match", "test file", "test name", "test lines at " + TESTS_COMMIT,
           "state", "flagged by or missing", "newest copy", "test state", "line 9", "line 11"]


def row(r):
    q9 = f"{r.file}:{r.line9[0]}: {r.line9[1]}" if r.line9 else ""
    q11 = f"{r.file}:{r.line11[0]}: {r.line11[1]}" if r.line11 else ""
    return [f"{r.file}:{r.line}", r.heading, r.match, r.test_file, r.test_name,
            ",".join(str(x) for x in r.test_lines), r.flag, r.why, r.newest, r.test_state, q9, q11]


def write(path, rows):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\t".join(COLUMNS) + "\n")
        for r in rows:
            f.write("\t".join(cell(c) for c in row(r)) + "\n")


def run(out, copy=COPY, repo=REPO, records_commit=RECORDS_COMMIT, folder=RECORDS_DIR, say=print):
    records, loose9, loose11 = read_records(repo, records_commit, folder)
    tests = Tests(copy)
    for r in records:
        match(r, tests)
        flag(r)
    repeated, timed_repeated, all_tied = pick_newest(records, repo, records_commit, folder)

    heads = grep_count(repo, records_commit, HEADING, folder)
    rows9 = grep_count(repo, records_commit, LINE9, folder)
    rows11 = grep_count(repo, records_commit, LINE11, folder)
    own9 = sum(1 for r in records if r.line9) + len(loose9)
    own11 = sum(1 for r in records if r.line11) + len(loose11)
    states = collections.Counter(r.match for r in records)
    flags = collections.Counter(r.flag for r in records)
    unmatched = [r for r in records if r.match in (GONE, NO_TEST)]

    say(f"records read at {records_commit}; tests read from the stripped copy {copy}")
    say(f"headings: {len(records)} matched or listed; git grep counts {heads} `## ` headings")
    for s in (SAME_PLACE, MOVED, GONE, NO_TEST):
        say(f"  {s}: {states[s]}")
    say(f"states: {FLAGGED} {flags[FLAGGED]}, {SOMETHING_ELSE} {flags[SOMETHING_ELSE]}")
    say(f"line 9 rows: script {own9}, git grep {rows9}; owned by a record {own9 - len(loose9)}, "
        f"reached by no record {len(loose9)}")
    say(f"line 11 rows: script {own11}, git grep {rows11}; owned by a record {own11 - len(loose11)}, "
        f"reached by no record {len(loose11)}")
    say(f"tests with more than one record: {repeated}; with two or more line 9 rows to time: "
        f"{timed_repeated}; of those, every copy at one blame time: {all_tied}")

    stops = []
    if len(records) != heads:
        stops.append(f"{len(records)} headings matched or listed, but git grep counts {heads}")
    if own9 != rows9:
        stops.append(f"the script counts {own9} line 9 rows, git grep counts {rows9}")
    if loose9:
        stops.append(f"{len(loose9)} line 9 rows sit under no `## ` record of their own (a second "
                     f"row in one record, or a row before any `## ` heading); the plan's heading "
                     f"rule does not reach them. First ones: {', '.join(loose9[:5])}")
    if timed_repeated and all_tied == timed_repeated:
        stops.append("blame gives one time to every copy of every repeated test, so no newest copy "
                     "can be picked")
    if stops:
        for s in stops:
            say("STOP: " + s)
        say("nothing written")
        return 1
    os.makedirs(out, exist_ok=True)
    write(os.path.join(out, "matched.tsv"), records)
    write(os.path.join(out, "unmatched.tsv"), unmatched)
    say(f"wrote matched.tsv ({len(records)} rows) and unmatched.tsv ({len(unmatched)} rows) to {out}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--out", required=True)
    ap.add_argument("--copy", default=COPY)
    ap.add_argument("--repo", default=REPO)
    ap.add_argument("--records-commit", default=RECORDS_COMMIT)
    a = ap.parse_args(argv)
    try:
        return run(a.out, a.copy, a.repo, a.records_commit)
    except Stop as e:
        print("STOP: " + str(e))
        return 1


if __name__ == "__main__":
    sys.exit(main())
