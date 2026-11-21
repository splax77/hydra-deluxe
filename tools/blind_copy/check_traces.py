"""Check the shape of the stand-in traces the blind auditors write.

    py -I tools/blind_copy/check_traces.py <copy> <kinds.md> <trace file> [<trace file> ...]
    py -I tools/blind_copy/check_traces.py --format

<copy> is the blind copy's root, the folder that holds `code/`. <kinds.md> is
the kinds file of the script-first recheck. Each trace file is one agent's
output from the trace brief,
docs/audit/2026-11-20-script-first-recheck/trace-brief.md, which carries
FORMAT_TEXT below word for word.

It checks shape only, never whether a trace is right: every cite names a file
and line that exist in the copy, every kind in kinds.md has exactly one trace
or the words "not traced", every trace names a kind of kinds.md, and no trace
holds a status or a claim word. It prints one line per problem and exits 1,
or prints the counts and exits 0. A usage error exits 2.
"""

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402

KIND_HEADING = "## "
RETURNS = "returns"
READ_FROM = "read from"
CHANGES = "changes before use"
CASE_KEYS = (RETURNS, READ_FROM, CHANGES)
NOT_TRACED = "not traced"
NONE_WORD = "none"
NOTHING_WORD = "nothing"
CANNOT_TELL = ff.CANNOT_TELL
CITES_KEY = "cites"

# A trace states facts. These words state what someone wanted or where a
# finding stands, which only the main session and the user decide. The first
# four are the claim words of docs/agents/audit-method.md, "The four exits";
# the rest are the recheck plan's states and the exit names.
STATUS_WORDS = (
    "by design", "on purpose", "deliberately", "kept",
    "flagged", "contradicted", "says something else", "no record",
    "stand-in found", "not a problem", "covered by a ruling",
)
STATUS_RE = re.compile(r"(?<![A-Za-z0-9_])(" + "|".join(re.escape(w) for w in STATUS_WORDS)
                       + r")(?![A-Za-z0-9_])", re.IGNORECASE)

# Anything shaped like path:line, so a cite without its leading code/ is
# refused rather than passed over.
CITE_LIKE = re.compile(r"(?<![\w/.-])(?P<cite>[\w./\\-]+\.[A-Za-z0-9]+:\d+(?:-\d+)?)")

FORMAT_TEXT = f"""\
The trace file has these line kinds and no others. Blank lines are allowed.
Nothing comes before the first heading.

{KIND_HEADING}<kind, copied exactly from its heading in the kinds you were given>
{RETURNS}: <what the stand-in gives back when the code under test calls it> | {CITES_KEY}: <cites>
{READ_FROM}: <a file or folder cite, {NONE_WORD}, or {CANNOT_TELL}> | {CITES_KEY}: <cites>
{CHANGES}: <what the test rewrites in it before the stand-in hands it over, or {NOTHING_WORD}> | {CITES_KEY}: <cites>

A traced kind holds one or more cases. A case is the three lines above, in that
order. When the kind's stand-ins do not all give back the same thing, write one
case per different thing.

A kind you did not trace holds one line and nothing else:

{NOT_TRACED}: <what stopped you>

Cites in a "{CITES_KEY}" field are joined by "{ff.CITE_SEP.strip()}" with a space each side, and
there is at least one. A cite is one of:
  code/<path>:<N>          one line
  code/<path>:<A>-<B>      a range of at most {ff.MAX_RANGE_LINES} lines

The "{READ_FROM}" answer is a cite with no line, of a file or folder inside
code/testdata or code/tests (never one of those two folders itself); or
{NONE_WORD}, when the cites show the value built in code; or {CANNOT_TELL}.
Any other text shaped like a file and a line, anywhere in the file, must
also be a cite of the form above, of a line that exists.

No line holds any of these words or phrases, in any letter case:
""" + "".join(f"  {w}\n" for w in STATUS_WORDS)


class Checker:
    def __init__(self, copy):
        self.copy = copy
        self.errors = []
        self.lines_of = {}

    def err(self, where, why):
        self.errors.append(f"{where}: {why}")

    def file_lines(self, cite_path):
        path = ff.copy_file(self.copy, cite_path)
        if path is None or not os.path.isfile(path):
            return None
        if path not in self.lines_of:
            with open(path, "rb") as f:
                self.lines_of[path] = len(f.read().splitlines())
        return self.lines_of[path]

    def line_cite(self, where, text):
        """Check one line cite; True when it holds."""
        c = ff.parse_cite(text)
        if c is None or c.kind != "line":
            self.err(where, f"{text!r} is not a line cite of the form code/<path>:<N> or code/<path>:<A>-<B>")
            return False
        count = self.file_lines(c.path)
        if count is None:
            self.err(where, f"the file of {text!r} is not in the copy")
            return False
        if c.a < 1 or c.b < c.a or c.b > count:
            self.err(where, f"the line of {text!r} is not in the file, which has {count} lines")
            return False
        if c.b - c.a + 1 > ff.MAX_RANGE_LINES:
            self.err(where, f"the range {text!r} is wider than {ff.MAX_RANGE_LINES} lines")
            return False
        return True

    def read_from(self, where, value):
        if value in (NONE_WORD, CANNOT_TELL):
            return
        c = ff.parse_cite(value)
        if c is None or c.kind != "bare":
            self.err(where, f"{READ_FROM} must be a file or folder cite with no line, "
                            f"{NONE_WORD} or {CANNOT_TELL}, not {value!r}")
            return
        parts = c.path.split("/")
        if len(parts) < 3 or parts[1] not in ("testdata", "tests"):
            self.err(where, f"{READ_FROM} {value!r} is not a file or folder inside code/testdata or code/tests")
            return
        path = ff.copy_file(self.copy, c.path)
        if path is None or not os.path.exists(path):
            self.err(where, f"{READ_FROM} {value!r} is not in the copy")

    def stray_cites(self, where, text):
        """Check every cite-shaped text on a line outside its cites field."""
        for m in CITE_LIKE.finditer(text):
            self.line_cite(where, m.group("cite"))

    def answer(self, where, key, rest):
        """Check one case line's text after "<key>: "."""
        value, sep, cites = rest.rpartition(ff.SEP + CITES_KEY + ": ")
        if not sep:
            self.err(where, f"a {key} line ends with \"{ff.SEP}{CITES_KEY}: <cites>\"")
            return
        value = value.strip()
        if not value:
            self.err(where, f"a {key} line has no answer before its cites")
        parts = [p for p in cites.split(ff.CITE_SEP)]
        if not cites.strip() or any(not p.strip() for p in parts):
            self.err(where, f"a {key} line needs at least one cite, joined by \"{ff.CITE_SEP}\"")
        for p in parts:
            if p.strip():
                self.line_cite(where, p.strip())
        if key == READ_FROM:
            self.read_from(where, value)
        else:
            self.stray_cites(where, value)


def kinds_in(path):
    """The kind headings of kinds.md, in order, and any repeated one."""
    kinds, seen, repeats = [], set(), []
    with open(path, encoding="utf-8") as f:
        for raw in f:
            line = raw.rstrip("\r\n")
            if line.startswith(KIND_HEADING):
                kind = line[len(KIND_HEADING):].strip()
                if kind in seen:
                    repeats.append(kind)
                seen.add(kind)
                kinds.append(kind)
    return kinds, repeats


def check_trace_file(checker, path, traced):
    """Check one trace file. traced maps each kind to the file:line that
    traced it, across all files, and gains this file's kinds."""
    name = os.path.basename(path)
    kind = None
    kind_line = 0
    body = []

    def close():
        if kind is None:
            return
        keys = [k for k, _ in body]
        where = f"{name}:{kind_line}"
        if keys == [NOT_TRACED]:
            return
        if not keys:
            checker.err(where, f"the trace of {kind!r} has no case and no \"{NOT_TRACED}\" line")
        elif NOT_TRACED in keys:
            checker.err(where, f"the trace of {kind!r} holds \"{NOT_TRACED}\" beside other lines")
        elif len(keys) % 3 or any(keys[i] != CASE_KEYS[i % 3] for i in range(len(keys))):
            checker.err(where, f"the trace of {kind!r} is not whole cases of "
                               f"{RETURNS}, {READ_FROM}, {CHANGES}, in that order")

    with open(path, encoding="utf-8") as f:
        for n, raw in enumerate(f, 1):
            line = raw.rstrip("\r\n")
            where = f"{name}:{n}"
            if not line.strip():
                continue
            if line.startswith(KIND_HEADING):
                # The heading is the kind's own text, copied from kinds.md, so
                # it is not read for status words.
                close()
                kind, kind_line, body = line[len(KIND_HEADING):].strip(), n, []
                if kind in traced:
                    first = traced[kind]
                    checker.err(where, f"{kind!r} is traced twice; the first is at {first[0]}:{first[1]}")
                else:
                    traced[kind] = (name, n)
                continue
            for m in STATUS_RE.finditer(line):
                checker.err(where, f"holds the status or claim word {m.group(1)!r}")
            if kind is None:
                checker.err(where, "a line outside any kind's trace")
                continue
            key, sep, rest = line.partition(": ")
            if not sep or key not in CASE_KEYS + (NOT_TRACED,):
                checker.err(where, "a line the trace format does not have")
                continue
            body.append((key, n))
            if key == NOT_TRACED:
                if not rest.strip():
                    checker.err(where, f"a \"{NOT_TRACED}\" line says what stopped the trace")
                checker.stray_cites(where, rest)
            else:
                checker.answer(where, key, rest)
    close()


def check(copy, kinds_path, trace_paths):
    """Every problem found, as printable lines, and the counts."""
    checker = Checker(copy)
    kinds, repeats = kinds_in(kinds_path)
    for kind in repeats:
        checker.err(os.path.basename(kinds_path), f"the kind {kind!r} has two sections")
    if not kinds:
        checker.err(os.path.basename(kinds_path), "holds no kind heading")
    traced = {}
    for p in trace_paths:
        check_trace_file(checker, p, traced)
    known = set(kinds)
    for kind, (name, n) in traced.items():
        if kind not in known:
            checker.err(f"{name}:{n}", f"{kind!r} is not a kind in {os.path.basename(kinds_path)}")
    for kind in kinds:
        if kind not in traced:
            checker.err(os.path.basename(kinds_path), f"the kind {kind!r} has no trace and no \"{NOT_TRACED}\" line")
    return checker.errors, len(kinds), len(traced)


def main(argv):
    if argv == ["--format"]:
        sys.stdout.write(FORMAT_TEXT)
        return 0
    if len(argv) < 3:
        sys.stderr.write(__doc__)
        return 2
    copy, kinds_path, traces = argv[0], argv[1], argv[2:]
    for p in [kinds_path] + traces:
        if not os.path.isfile(p):
            sys.stderr.write(f"no such file: {p}\n")
            return 2
    if not os.path.isdir(os.path.join(copy, "code")):
        sys.stderr.write(f"{copy} holds no code folder, so it is not the copy's root\n")
        return 2
    errors, n_kinds, n_traced = check(copy, kinds_path, traces)
    for e in errors:
        print(e)
    if errors:
        print(f"FAIL: {len(errors)} problems; {n_kinds} kinds, {n_traced} traced or marked not traced")
        return 1
    print(f"OK: {n_kinds} kinds, each traced or marked not traced once; every cite is in the copy")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
