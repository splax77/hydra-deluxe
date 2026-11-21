"""Check a blind auditor's facts file against the format in facts_format.py.

    py -I tools/blind_copy/check_facts.py <copy> <facts>

<copy> is the blind copy's root, the folder that holds `code/`. Every cite is
checked against the files there. It prints one line per rejected line and
exits 1, or prints the counts and exits 0.
"""

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402

BRACKETS = re.compile(r"[()\[\]{}]")
FUNCTION_NAME = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*(::[A-Za-z_][A-Za-z0-9_]*)*$")
WORD_SIDES = ("true", "false")


class Checker:
    def __init__(self, copy):
        self.copy = copy
        self.errors = []
        self.lines_of = {}
        self.text_of = {}
        self.cannot_tell = 0

    def err(self, n, why):
        self.errors.append((n, why))

    def file_lines(self, cite_path):
        path = ff.copy_file(self.copy, cite_path)
        if path is None or not os.path.isfile(path):
            return None
        if path not in self.lines_of:
            with open(path, "rb") as f:
                self.lines_of[path] = len(f.read().splitlines())
        return self.lines_of[path]

    def collapsed_text(self, cite_path):
        path = ff.copy_file(self.copy, cite_path)
        if path is None or not os.path.isfile(path):
            return ""
        if path not in self.text_of:
            with open(path, encoding="utf-8", errors="replace") as f:
                self.text_of[path] = " ".join(f.read().split())
        return self.text_of[path]

    def cite(self, n, text, what):
        """The parsed cite, or None after recording why it is rejected."""
        c = ff.parse_cite(text)
        if c is None:
            self.err(n, f"{what}: {text!r} is not a cite")
            return None
        if c.kind == "word":
            if c.text == ff.CANNOT_TELL:
                self.cannot_tell += 1
            return c
        if c.kind == "bare":
            parts = c.path.split("/")
            path = ff.copy_file(self.copy, c.path)
            if len(parts) < 2 or parts[1] not in ("testdata", "tests"):
                self.err(n, f"{what}: a cite with no line must name a path under code/testdata or code/tests: {text!r}")
                return None
            if path is None or not os.path.exists(path):
                self.err(n, f"{what}: {text!r} is not in the copy")
                return None
            return c
        count = self.file_lines(c.path)
        if count is None:
            self.err(n, f"{what}: file of {text!r} is not in the copy")
            return None
        if c.a < 1 or c.b < c.a or c.b > count:
            self.err(n, f"{what}: line of {text!r} is not in the file, which has {count} lines")
            return None
        if c.b - c.a + 1 > ff.MAX_RANGE_LINES:
            self.err(n, f"{what}: range {text!r} is wider than {ff.MAX_RANGE_LINES} lines")
            return None
        return c

    def cites(self, n, text, sep, what):
        out = []
        for part in text.split(sep):
            c = self.cite(n, part, what)
            if c is not None:
                out.append(c)
        return out

    def plain_field(self, n, text, what):
        if not text:
            self.err(n, f"{what} is empty")
        elif BRACKETS.search(text):
            self.err(n, f"{what} holds parentheses or brackets: {text!r}")

    def keyed(self, n, field, key):
        v = ff.field_value(field, key)
        if v is None:
            self.err(n, f"expected the field {key!r}, found {field!r}")
        return v

    def check_line(self, n, body, section):
        fields = body.split(ff.SEP)
        keys = ("left made at", "right made at", "same function")
        if len(fields) < 5:
            have = [k for k in keys if any(f.startswith(k + ": ") for f in fields)]
            missing = [k for k in keys if k not in have]
            self.err(n, "check is missing the field " + ", ".join(repr(k) for k in missing or keys))
            return
        if len(fields) > 5:
            self.err(n, f"text after the last field: {ff.SEP.join(fields[5:])!r}")
            return
        left, right = fields[0], fields[1]
        made = []
        for field, key in zip(fields[2:4], keys[:2]):
            v = self.keyed(n, field, key)
            if v is not None:
                made += self.cites(n, v, ff.CITE_SEP, key)
        files = [c.path for c in made if c.kind == "line"]
        if section.location is not None:
            files.append(section.location.path)
        for side, what in ((left, "left"), (right, "right")):
            self.code_side(n, side, what, files)
        same = self.keyed(n, fields[4], "same function")
        if same is None:
            return
        if same == "no":
            return
        if same == ff.CANNOT_TELL:
            self.cannot_tell += 1
            return
        if same.startswith("yes "):
            c = self.cite(n, same[4:], "same function")
            if c is not None and (c.kind != "line" or c.a != c.b):
                self.err(n, f"same function: yes needs one line cite, found {same[4:]!r}")
            return
        self.err(n, f"same function must be yes with a line cite, no, or cannot tell: {same!r}")

    def code_side(self, n, side, what, files):
        if not side:
            self.err(n, f"check's {what} side is empty")
            return
        if side in WORD_SIDES:
            return
        want = " ".join(side.split())
        if not any(want in self.collapsed_text(p) for p in files):
            self.err(n, f"check's {what} side is not code in the test's file or its cited files: {side!r}")

    def input_line(self, n, body):
        fields = body.split(ff.SEP)
        if len(fields) < 3:
            self.err(n, "input needs a name, a source and a trace")
            return
        self.plain_field(n, fields[0], "input name")
        source = self.keyed(n, fields[1], "source")
        if source is None:
            return
        if source not in ff.SOURCE_WORDS:
            self.err(n, f"source word is not on the list: {source!r}")
            return
        if source == ff.CANNOT_TELL:
            self.cannot_tell += 1
        want = 5 if source == ff.REPLACES else 3
        if len(fields) < want:
            self.err(n, f"input with source {source!r} needs {want} fields, found {len(fields)}")
            return
        if len(fields) > want:
            self.err(n, f"text after the last field: {ff.SEP.join(fields[want:])!r}")
            return
        if source == ff.REPLACES:
            name = self.keyed(n, fields[2], "replaces")
            if name is not None and not FUNCTION_NAME.match(name):
                self.err(n, f"replaces must be one function name: {name!r}")
            stand = self.keyed(n, fields[3], "stands in")
            if stand is not None:
                self.stands_in(n, stand)
        trace = self.keyed(n, fields[-1], "trace")
        if trace is None:
            return
        cites = self.cites(n, trace, ff.TRACE_SEP, "trace")
        bare = [c for c in cites if c.kind == "bare"]
        if source == ff.TESTDATA:
            if not any(c.path.split("/")[1] == "testdata" for c in bare):
                self.err(n, "an input from a file under testdata must cite that path under code/testdata, with no line")
        elif source == ff.NETWORK:
            if not any(os.path.isfile(ff.copy_file(self.copy, c.path)) for c in bare):
                self.err(n, "a real network reply must cite its recorded reply file under code/testdata or code/tests, with no line")

    def stands_in(self, n, text):
        for word in sorted(ff.STAND_IN_WORDS, key=len, reverse=True):
            if text.startswith(word + " "):
                if word == ff.CANNOT_TELL:
                    self.cannot_tell += 1
                self.cite(n, text[len(word) + 1:], "stands in")
                return
        self.err(n, f"stands in must be a stand-in word from the list and a cite: {text!r}")

    def run(self, facts):
        section = None
        counts = {"tests": 0, "checks": 0, "inputs": 0}
        with open(facts, encoding="utf-8") as f:
            lines = f.read().splitlines()
        for n, line in enumerate(lines, 1):
            if not line.strip():
                if line:
                    self.err(n, "a blank line holds spaces")
                continue
            if line.startswith("## "):
                if section is not None and section.location is None:
                    self.err(section.lineno, "test section has no location line")
                section = ff.Section(line[3:], n)
                counts["tests"] += 1
                if not section.name.strip():
                    self.err(n, "test name is empty")
                continue
            kind, _, body = line.partition(": ")
            if kind not in ("location", "check", "input"):
                self.err(n, "not one of the four line kinds")
                continue
            if section is None:
                self.err(n, f"{kind} line before any test section")
                continue
            if kind == "location":
                if section.location is not None:
                    self.err(n, "second location line in one test section")
                    continue
                c = self.cite(n, body, "location")
                if c is not None and (c.kind != "line" or c.a != c.b):
                    self.err(n, f"location must be one line cite: {body!r}")
                    continue
                section.location = c
            elif kind == "check":
                counts["checks"] += 1
                self.check_line(n, body, section)
            else:
                counts["inputs"] += 1
                self.input_line(n, body)
        if section is not None and section.location is None:
            self.err(section.lineno, "test section has no location line")
        return counts


def main(argv):
    if len(argv) != 3:
        sys.stderr.write(__doc__)
        return 2
    checker = Checker(argv[1])
    counts = checker.run(argv[2])
    for n, why in sorted(checker.errors):
        print(f"line {n}: {why}")
    summary = (f"{counts['tests']} tests, {counts['checks']} checks, {counts['inputs']} inputs, "
               f"{checker.cannot_tell} cannot tell fields")
    if checker.errors:
        print(f"rejected: {len(checker.errors)} problems; {summary}")
        return 1
    print(f"ok: {summary}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
