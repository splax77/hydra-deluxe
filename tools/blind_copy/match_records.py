"""Join each old audit record to the test it names, and flag its lines 9 and 11.

    py -I tools/blind_copy/match_records.py --out <folder>
        [--copy C:\\Users\\Patrick\\Downloads\\Hydra\\hydra-blind]
        [--repo <git repo>] [--records-commit a4ad3306]

The records are read with git at --records-commit, from the records folder
of the November 14 test audit; nothing is checked out. The tests the
headings are matched against are read from the stripped copy (--copy), the
copy of commit 539c6de0 that the recheck reads, under its `code/` folder.

It writes `matched.tsv` (one row per record), `unmatched.tsv` (the records
that join no test, with their state), `sections.tsv` (the headings that
are not records) and `shared_rows.tsv` (the line 9 and 11 rows a shared or
group section gives to many records, and the rows left with no record because
a heading ended theirs; the script never assigns them to a test) into --out. The rules it applies are the plan's section
"T2: Join each test to its newest old record"
(docs/superpowers/plans/2026-11-20-script-first-recheck.md), with the reading
rules of briefs sfr-t2b and sfr-t2c in place of the plan's `## ` heading
rule; this script adds none of its own beyond what its comments name.

It stops, writing nothing, when one of its counts disagrees with git's, when
a line 9 or line 11 row sits under no record and no shared heading, or when
blame times cannot tell any repeated test's copies apart.
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

# The `git grep -E` patterns the script's own counts are checked against.
# They are written apart from the reader below on purpose: a line the grep
# finds and the reader does not read makes the counts differ, and the run stops.
# The old audit's records answer questions 1 to 11 (line 11 is the last).
# No record line opens with a higher number: `git grep -E "^12 [A-Z]"` over the
# audit folder at the records commit finds none.
LAST_ANSWER = 11

HEADING = "^#{2,4} "
LEVEL1 = "^# "


def answer_grep(n):
    """Every line holding answer n: at the line's start, or after a full stop
    on a line that opens with another numbered answer or with `Read:`."""
    return (rf"^((- )?(\*\*)?({n}(\*\*)?[ :]|([1-9]|1[0-{LAST_ANSWER - 10}])([-/][0-9]+)*(\*\*)?[ :].*\. {n}[ :])"
            rf"|Read:.*\. {n}[ :])")


LINE9 = answer_grep(9)
LINE11 = answer_grep(11)

HEAD_LINE_RE = re.compile(r"^(?P<level>#{2,4}) (?P<text>.*)$")
TAIL_RE = re.compile(r"^(?P<rest>.*?)\s*\((?P<inner>[^()]*)\)\s*$")
FULL_PLACE_RE = re.compile(r"^(?P<file>[^\s:]+\.[A-Za-z0-9]+):(?P<line>\d+)")
SHORT_PLACE_RE = re.compile(r"^(?::|line )(?P<line>\d+)")
IDENTIFIER_RE = re.compile(r"^\w+$")
FILE_ONLY_RE =re.compile(r"^[^\s:]+\.[A-Za-z0-9]+$")
AT_LINE_RE = re.compile(r"^`?(?P<name>[\w.]+)`? at line (?P<line>\d+)$")
BARE_LINE_RE =re.compile(r"^\d+(?:-\d+)?$")
FILE_TOKEN_RE = re.compile(r"[\w./-]*[\w-]\.(?:cpp|h|hpp|cc|cxx|py)\b")
# A leading record number ("44. ") or record label ("PL1. ", "E1 ").
LEAD_NUMBER_RE = re.compile(r"^\d+[a-z]?\.\s+")
LEAD_LABEL_RE = re.compile(r"^[A-Z]{1,4}-?\d+[a-z]?\.?\s+")

# Record titles that are not headings (brief sfr-t2c, kind 1): a bold title
# with a place in brackets just after it, a plain line ending in a place, or a
# plain line followed by a line that holds only the place.
BOLD_TITLE_RE = re.compile(r"^\*\*(?P<text>[^*]+?)\s*\*\*\s*(?P<place>\([^()]*\))")
PLACE_ONLY_RE = re.compile(r"^\s*(?P<place>\([^()]*\))\s*$")
# A file's own short names for its files, as T3.md line 5 defines them:
# "Short names. T = tests/test_preview_boxes.cpp. PV = ...".
ALIAS_LINE = "Short names. "
ALIAS_RE = re.compile(r"^(?P<alias>[A-Za-z]+) = (?P<file>[\w./-]+\.[A-Za-z0-9]+)$")
ALIAS_PLACE_RE = re.compile(r"(?<![\w./-])(?P<alias>[A-Za-z]+):(?=\d)")
# Answers that a section gives to many records at once (kind 2): the
# section's heading calls them shared or a group.
SHARED_HEADING_RE = re.compile(r"\b(?:shared|group)\b", re.IGNORECASE)

ANSWER_START_RE = re.compile(r"^(?P<dash>- )?(?P<bold>\*\*)?(?P<num>\d+(?:[-/]\d+)*)(?:\*\*)?(?P<sep>[ :])")
ANSWER_INLINE_RE = re.compile(r"(?<=\. )(?P<num>\d+(?:[-/]\d+)*)(?P<sep>[ :])")
LABELS = {9: "Stand-in:", 11: "Real:"}

SHARED_WHY = "under a shared heading"
ENDED_WHY = "its record was ended by a heading"

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


def grep_lines(repo, commit, pattern, folder):
    """`<file name>:<line>` for each line the pattern finds."""
    code, out = git(repo, "grep", "-n", "-E", pattern, commit, "--", folder)
    if code not in (0, 1):
        raise Stop(f"git grep failed for {pattern!r} at {commit}")
    found = set()
    for line in out.splitlines():
        path, n = line.split(":", 3)[1:3]
        found.add(f"{path.rsplit('/', 1)[-1]}:{n}")
    return found


class Record:
    def __init__(self, file, line, heading, from_heading=True, level=0):
        self.file = file
        self.level = level   # the heading's `#` count; 0 for a title that is not a heading
        self.line = line
        self.heading = heading
        self.from_heading = from_heading   # False for a title that is not a heading
        self.line9 = []   # (record line, answer text) for each line 9 row it owns
        self.line11 = []
        self.place_file = ""   # the file the heading's place names, "" for a short place
        self.place_line = 0
        self.names = ([], [])  # name candidates from the heading's text, then from its parenthesis
        self.test_file = ""
        self.test_name = ""
        self.test_lines = []
        self.match = NO_TEST
        self.note = ""
        self.flag = SOMETHING_ELSE
        self.why = ""
        self.newest = ""
        self.test_state = ""
        self.time9 = None


def unquote(s):
    return s.strip().strip("`\"'").strip()


def parse_heading(text):
    """A record heading's place and name candidates, or None for a section
    heading. A record heading's final parenthesis holds a place: `file:line`,
    `:line` or `line N`, or is a bare line number. Its other parts, such as
    a backticked test name, are name candidates."""
    m = TAIL_RE.match(text)
    if not m:
        return None
    inner = m.group("inner").strip()
    parts = [unquote(p) for p in re.split(r",\s*|\s+and\s+", inner)]
    place, tail_names = None, []
    if BARE_LINE_RE.match(unquote(inner)):
        place = ("", int(re.match(r"\d+", unquote(inner)).group(0)))
    else:
        for p in parts:
            full, short = FULL_PLACE_RE.match(p), SHORT_PLACE_RE.match(p)
            if place is None and full:
                place = (full.group("file"), int(full.group("line")))
            elif place is None and short:
                place = ("", int(short.group("line")))
            elif p and not full and not short:
                tail_names.append(p)
    if place is None:
        # `file, name at line N`: the file is one part and the line follows "at line".
        files = [p for p in parts if FILE_ONLY_RE.match(p)]
        at = [AT_LINE_RE.match(p) for p in parts]
        at = [a for a in at if a]
        if files and at:
            place = (files[0], int(at[0].group("line")))
            tail_names = [at[0].group("name")]
    if place is None:
        return None
    rest = unquote(m.group("rest"))
    names = []
    for base in (rest, LEAD_NUMBER_RE.sub("", rest), LEAD_LABEL_RE.sub("", rest)):
        base = unquote(base)
        names.append(base)
        if ": " in base:
            # `group: name` names the test after the colon; `test_name: what it
            # checks` names it before. Both are tried; a name no test has matches nothing.
            head, after = base.split(": ", 1)
            names.append(after)
            if IDENTIFIER_RE.match(head):
                names.append(head)
    names = list(dict.fromkeys(n for n in names if n))
    return place, (names, [n for n in dict.fromkeys(tail_names) if n not in names])


def read_aliases(lines):
    """The short names a records file defines for its files, from its
    `Short names. ` line; {} when it has none."""
    out = {}
    for line in lines:
        if line.startswith(ALIAS_LINE):
            for part in re.split(r"\.\s+", line[len(ALIAS_LINE):].rstrip().rstrip(".")):
                m = ALIAS_RE.match(part)
                if m:
                    out[m.group("alias")] = m.group("file")
    return out


def expand_aliases(place, aliases):
    """A place written with the file's short name, with the file put back.
    A short name the file does not define is left as written."""
    return ALIAS_PLACE_RE.sub(
        lambda m: aliases[m.group("alias")] + ":" if m.group("alias") in aliases else m.group(0), place)


def plain_line(line):
    """A line that could be a plain record title: not indented, not a
    heading, a fence, a table row, a list item, a `Read:` line or an answer."""
    return bool(line.strip()) and not line[0].isspace() and \
        not line.startswith(("#", "```", "|", "- ", "Read:", "**")) and not answers(line)


def parse_title(lines, i, aliases):
    """A record title that is not a heading, starting at lines[i]: (title as
    written, what parse_heading reads from it, the number of lines it takes),
    or None. A plain title must name its file in its place; a bold title may
    give any place a heading may."""
    line = lines[i]
    m = BOLD_TITLE_RE.match(line)
    if m:
        place = expand_aliases(m.group("place"), aliases)
        parsed = parse_heading(f"{m.group('text')} {place}")
        return (line, parsed, 1) if parsed else None
    if not plain_line(line):
        return None
    tail = TAIL_RE.match(line)
    if tail and tail.group("rest").strip():
        text = f"{tail.group('rest')} {expand_aliases('(' + tail.group('inner') + ')', aliases)}"
        parsed = parse_heading(text)
        if parsed and parsed[0][0]:
            return line, parsed, 1
    if i + 1 < len(lines):
        p = PLACE_ONLY_RE.match(lines[i + 1])
        if p:
            parsed = parse_heading(f"{line} {expand_aliases(p.group('place'), aliases)}")
            if parsed and parsed[0][0]:
                return f"{line} {p.group('place')}", parsed, 2
    return None


def files_named(text):
    """The files a heading names. A bare file name beside the same file with
    its folder is one file, not two."""
    found = list(dict.fromkeys(unquote(t) for t in FILE_TOKEN_RE.findall(text)))
    bases = {f.rsplit("/", 1)[-1] for f in found if "/" in f}
    return [f for f in found if "/" in f or f not in bases]


def answers(line):
    """The numbered answers on one record line: (number text, answer text,
    separator, dash, bold, after another answer). The first answer opens the
    line, or the line opens with `Read:`; each later one follows a full stop
    and carries a higher number."""
    m = ANSWER_START_RE.match(line)
    if line.startswith("Read:"):
        marks, last, start, dash, bold = [], 0, 0, False, False
    elif m:
        nums = [int(x) for x in re.split(r"[-/]", m.group("num"))]
        if not 1 <= nums[0] <= LAST_ANSWER:
            return []
        marks = [(m.start("num"), m.end(), m.group("num"), m.group("sep"), False)]
        last, start, dash, bold = max(nums), m.end(), bool(m.group("dash")), bool(m.group("bold"))
    else:
        return []
    for im in ANSWER_INLINE_RE.finditer(line, start):
        inums = [int(x) for x in re.split(r"[-/]", im.group("num"))]
        if inums[0] > last and inums[0] <= LAST_ANSWER:
            marks.append((im.start("num"), im.end(), im.group("num"), im.group("sep"),
                          "read" if line.startswith("Read:") else True))
            last = max(inums)
    out = []
    for i, (_, end, num, sep, inline) in enumerate(marks):
        stop = marks[i + 1][0] if i + 1 < len(marks) else len(line)
        out.append((num, line[end:stop].strip(), sep, dash, bold, inline))
    return out


def answer_text(n, text):
    """The answer with any bold marks and the line's label dropped. Returns
    (text, whether a label was there)."""
    t = text.strip().lstrip("*").strip()
    labelled = t.startswith(LABELS[n])
    if labelled:
        t = t[len(LABELS[n]):]
    return t.lstrip("*").strip(), labelled


def shape(sep, dash, bold, labelled, inline):
    words = ["labelled" if labelled else "no label"]
    if dash:
        words.append("after '- '")
    if bold:
        words.append("in bold")
    if sep == ":":
        words.append("number then ':'")
    if inline == "read":
        words.append("inside a 'Read:' line")
    elif inline:
        words.append("after another answer on its line")
    return ", ".join(words)


class Read:
    """What read_records found, beside the records themselves."""

    def __init__(self):
        self.records, self.sections = [], []
        self.loose = {9: [], 11: []}       # rows under no record and no shared heading
        self.shared = []                   # (file, line, heading above, 9 or 11, answer, why listed)
        self.titles = 0                    # records read from a title that is not a heading
        self.lines = {9: 0, 11: 0}         # record lines holding a row
        self.line_set = {9: set(), 11: set()}
        self.shapes = {9: collections.Counter(), 11: collections.Counter()}
        self.across = {9: 0, 11: 0}        # rows that reach their record across a section heading
        self.unread = []                   # a range or list of numbers that covers 9 or 11


def open_record(name, n, text, parsed, file_above, from_heading, level=0):
    rec = Record(name, n, text, from_heading, level)
    (rec.place_file, rec.place_line), rec.names = parsed
    if not rec.place_file:
        # A short place takes its file from the nearest heading above that names one.
        rec.place_file = file_above[0] if len(file_above) == 1 else ""
        if len(file_above) > 1:
            rec.note = "the nearest heading above names more than one file"
        elif not file_above:
            rec.note = "no heading above names a file"
    return rec


def read_records(repo, commit, folder):
    """A record heading opens a record; it runs until the next record heading
    or `# ` heading, or a section heading of the same level or a higher one
    (fewer `#`), or any section heading for a title that is not a heading,
    or a shared heading. A row belongs to the open record, past the deeper
    section headings in between. A row under no record goes to `shared` when
    the heading above it is a shared one, or when a section heading ended its
    record (it is listed with that heading, never given to an earlier
    record); else it goes to `loose`.

    While no record heading is open, a title that is not a heading
    (parse_title) opens a record, but only once a numbered answer row
    follows it with no other title or heading between them. Its record then
    runs as a heading's does, until the next title or heading."""
    code, out = git(repo, "ls-tree", "--name-only", commit, folder)
    if code != 0:
        raise Stop(f"git ls-tree failed for {folder} at {commit}")
    got = Read()
    for path in out.splitlines():
        code, text = git(repo, "show", f"{commit}:{path}")
        if code != 0:
            raise Stop(f"git show failed for {path} at {commit}")
        name = path.rsplit("/", 1)[-1]
        lines = [raw.rstrip("\r") for raw in text.split("\n")]
        aliases = read_aliases(lines)
        cur, crossed, file_above, pending, above, skip, ended = None, False, [], None, "", 0, False
        for n, line in enumerate(lines, 1):
            if skip:
                skip -= 1
                continue
            h = HEAD_LINE_RE.match(line)
            if h:
                pending, above = None, h.group("text")
                parsed = parse_heading(h.group("text"))
                if parsed:
                    cur = open_record(name, n, h.group("text"), parsed, file_above, True, len(h.group("level")))
                    crossed, ended = False, False
                    got.records.append(cur)
                else:
                    got.sections.append((name, n, h.group("text")))
                    crossed = True
                    if cur is not None and (not cur.from_heading or len(h.group("level")) <= cur.level):
                        cur, ended = None, True
                    if SHARED_HEADING_RE.search(h.group("text")):
                        cur = None
                named = [parsed[0][0]] if parsed and parsed[0][0] else files_named(h.group("text"))
                if named:
                    file_above = named
                continue
            if re.match(LEVEL1, line):
                cur, crossed, pending, above, ended = None, False, None, line[2:], False
                named = files_named(line)
                file_above = named
                continue
            if cur is None or not cur.from_heading:
                title = parse_title(lines, n - 1, aliases)
                if title:
                    pending, skip = (n, title[0], title[1]), title[2] - 1
                    continue
            found = answers(line)
            if found and pending:
                cur = open_record(name, pending[0], pending[1], pending[2], file_above, False)
                crossed, pending, ended = False, None, False
                got.records.append(cur)
                got.titles += 1
            seen = set()
            for num, ans, sep, dash, bold, inline in found:
                nums = [int(x) for x in re.split(r"[-/]", num)]
                if len(nums) > 1:
                    covered = range(nums[0], nums[-1] + 1) if "-" in num else nums
                    if 9 in covered or 11 in covered:
                        got.unread.append(f"{name}:{n}: {line}")
                    continue
                k = nums[0]
                if k not in (9, 11):
                    continue
                t, labelled = answer_text(k, ans)
                got.shapes[k][shape(sep, dash, bold, labelled, inline)] += 1
                seen.add(k)
                if cur is None:
                    if SHARED_HEADING_RE.search(above):
                        got.shared.append((name, n, above, k, t, SHARED_WHY))
                    elif ended:
                        got.shared.append((name, n, above, k, t, ENDED_WHY))
                    else:
                        got.loose[k].append(f"{name}:{n}")
                    continue
                (cur.line9 if k == 9 else cur.line11).append((n, t))
                if crossed:
                    got.across[k] += 1
            for k in seen:
                got.lines[k] += 1
                got.line_set[k].add(f"{name}:{n}")
    return got


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
        self.copy, self.cache, self.by_base = copy, {}, None

    def resolve(self, file):
        """A file with a folder as written; a bare file name only when exactly
        one file in the copy has that name. Returns (path or None, note)."""
        if "/" in file:
            return file, ""
        if self.by_base is None:
            self.by_base = collections.defaultdict(list)
            root = os.path.join(self.copy, "code")
            for d, _, files in os.walk(root):
                for f in files:
                    self.by_base[f].append(os.path.relpath(os.path.join(d, f), root).replace("\\", "/"))
        hits = self.by_base.get(file, [])
        if len(hits) == 1:
            return hits[0], ""
        return None, f"bare file name {file} names {len(hits)} files in the copy"

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
    rec.match = NO_TEST
    if not rec.place_file:
        return
    file, note = tests.resolve(rec.place_file)
    if file is None:
        rec.note = note
        return
    rec.test_file = file
    names = tests.names(file)
    if names is None:
        rec.match = GONE
        return
    # The heading's own text is tried first; the names inside its parenthesis
    # only when the text names no test.
    rest, tail = rec.names
    hits = list(dict.fromkeys(c for c in rest if c in names)) or \
        list(dict.fromkeys(c for c in tail if c in names))
    if len(hits) > 1:
        # Two candidates name two different tests; the script does not pick one.
        rec.note = "the heading names more than one test: " + ", ".join(hits)
        return
    if hits:
        rec.test_name = hits[0]
        rec.test_lines = names[hits[0]]
        rec.match = SAME_PLACE if rec.place_line in names[hits[0]] else MOVED


def flag(rec):
    missing = [n for n, rows in (("line 9", rec.line9), ("line 11", rec.line11)) if not rows]
    if missing:
        rec.flag, rec.why = SOMETHING_ELSE, "no " + " and no ".join(missing)
        return
    twice = [n for n, rows in (("line 9", rec.line9), ("line 11", rec.line11)) if len(rows) > 1]
    if twice:
        rec.flag, rec.why = SOMETHING_ELSE, "two or more rows for " + " and ".join(twice)
        return
    by = []
    if rec.line9[0][1].lower().startswith("none"):
        by.append("line 9")
    if rec.line11[0][1].lower().startswith("yes"):
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
        timed = [r for r in copies if r.line9]
        if len(copies) > 1:
            repeated += 1
            for r in timed:
                if r.file not in blames:
                    blames[r.file] = blame_times(repo, commit, folder + r.file)
                # A record holding two line 9 rows is timed by the later one.
                r.time9 = max(blames[r.file][n] for n, _ in r.line9)
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


COLUMNS = ["record", "heading", "match", "match note", "test file", "test name",
           "test lines at " + TESTS_COMMIT, "state", "flagged by or missing", "newest copy", "test state",
           "line 9", "line 11"]


def quote(r, rows):
    return " | ".join(f"{r.file}:{n}: {t}" for n, t in rows)


def row(r):
    return [f"{r.file}:{r.line}", r.heading, r.match, r.note, r.test_file, r.test_name,
            ",".join(str(x) for x in r.test_lines), r.flag, r.why, r.newest, r.test_state,
            quote(r, r.line9), quote(r, r.line11)]


def write(path, rows):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\t".join(COLUMNS) + "\n")
        for r in rows:
            f.write("\t".join(cell(c) for c in row(r)) + "\n")


def run(out, copy=COPY, repo=REPO, records_commit=RECORDS_COMMIT, folder=RECORDS_DIR, say=print):
    got = read_records(repo, records_commit, folder)
    records = got.records
    tests = Tests(copy)
    for r in records:
        match(r, tests)
        flag(r)
    repeated, timed_repeated, all_tied = pick_newest(records, repo, records_commit, folder)

    heads = grep_count(repo, records_commit, HEADING, folder)
    grep_rows = {9: grep_count(repo, records_commit, LINE9, folder),
                 11: grep_count(repo, records_commit, LINE11, folder)}
    owned = {9: sum(len(r.line9) for r in records), 11: sum(len(r.line11) for r in records)}
    states = collections.Counter(r.match for r in records)
    flags = collections.Counter(r.flag for r in records)
    unmatched = [r for r in records if r.match in (GONE, NO_TEST)]
    notes = collections.Counter(re.sub(r"(bare file name) \S+ (names \d+) .*", r"\1 that \2 files",
                                       r.note.split(":")[0]) for r in unmatched if r.note)

    say(f"records read at {records_commit}; tests read from the stripped copy {copy}")
    say(f"headings: {len(records) - got.titles} record headings and {len(got.sections)} section headings; "
        f"git grep counts {heads} `##`, `###` and `####` headings")
    say(f"records read from a title that is not a heading: {got.titles}")
    for s in (SAME_PLACE, MOVED, GONE, NO_TEST):
        say(f"  {s}: {states[s]}")
    for note, c in sorted(notes.items()):
        say(f"    of the unmatched, {note}: {c}")
    say(f"states: {FLAGGED} {flags[FLAGGED]}, {SOMETHING_ELSE} {flags[SOMETHING_ELSE]}")
    for s in (SAME_PLACE, MOVED, GONE, NO_TEST):
        c = collections.Counter(r.flag for r in records if r.match == s)
        say(f"  {s}: {FLAGGED} {c[FLAGGED]}, {SOMETHING_ELSE} {c[SOMETHING_ELSE]}")
    why = collections.Counter(r.why for r in records if r.flag == SOMETHING_ELSE and r.why)
    for w, c in sorted(why.items()):
        say(f"  {SOMETHING_ELSE} because {w}: {c}")
    for k in (9, 11):
        shared_k = sum(1 for s in got.shared if s[3] == k and s[5] == SHARED_WHY)
        ended_k = sum(1 for s in got.shared if s[3] == k and s[5] == ENDED_WHY)
        say(f"line {k} rows: script {owned[k] + shared_k + ended_k + len(got.loose[k])} on {got.lines[k]} lines, "
            f"git grep {grep_rows[k]} lines; owned by a record {owned[k]} ({got.across[k]} of them past a "
            f"section heading), under a shared heading {shared_k}, listed because a heading ended "
            f"their record {ended_k}, under neither {len(got.loose[k])}")
        for s, c in sorted(got.shapes[k].items()):
            say(f"  shape {s}: {c}")
    say(f"ranges or lists of answer numbers covering line 9 or 11, not read: {len(got.unread)}")
    for u in got.unread:
        say("  " + u[:200])
    say(f"tests with more than one record: {repeated}; with two or more line 9 rows to time: "
        f"{timed_repeated}; of those, every copy at one blame time: {all_tied}")

    stops = []
    if len(records) - got.titles + len(got.sections) != heads:
        stops.append(f"{len(records) - got.titles + len(got.sections)} headings read, but git grep counts {heads}")
    for k in (9, 11):
        if got.lines[k] != grep_rows[k]:
            found = grep_lines(repo, records_commit, LINE11 if k == 11 else LINE9, folder)
            only_grep = sorted(found - got.line_set[k])
            only_script = sorted(got.line_set[k] - found)
            stops.append(f"the script reads line {k} rows on {got.lines[k]} lines, git grep finds "
                         f"{grep_rows[k]} lines. Found only by git grep ({len(only_grep)}): "
                         f"{', '.join(only_grep[:20])}. Read only by the script ({len(only_script)}): "
                         f"{', '.join(only_script[:20])}")
        if got.loose[k]:
            stops.append(f"{len(got.loose[k])} line {k} rows sit under no record and no shared heading: "
                         f"{', '.join(got.loose[k][:20])}")
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
    with open(os.path.join(out, "sections.tsv"), "w", encoding="utf-8", newline="\n") as f:
        f.write("heading line\theading\n")
        for name, n, text in got.sections:
            f.write(f"{name}:{n}\t{cell(text)}\n")
    with open(os.path.join(out, "shared_rows.tsv"), "w", encoding="utf-8", newline="\n") as f:
        f.write("file\tline\tsection heading\tanswer\trow\tlisted because\n")
        for name, n, text, k, t, why in got.shared:
            f.write(f"{name}\t{n}\t{cell(text)}\t{k}\t{cell(t)}\t{why}\n")
    say(f"wrote matched.tsv ({len(records)} rows), unmatched.tsv ({len(unmatched)} rows), "
        f"sections.tsv ({len(got.sections)} rows) and shared_rows.tsv ({len(got.shared)} rows) to {out}")
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
