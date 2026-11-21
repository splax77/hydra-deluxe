"""The one home of the blind auditor's facts format.

check_facts.py checks a facts file against it, and grade_seams.py and
made_up_inputs.py read facts through it. Run it to print the format text a
blind auditor's brief copies:

    py -I tools/blind_copy/facts_format.py
"""

import os
import re
import sys

# The seven source words, from the recheck plan's T5 brief template.
SOURCE_WORDS = (
    "a file under testdata",
    "a file the test writes itself",
    "a value the test builds in code",
    "a production function the test replaces",
    "an error or reply the test produces itself",
    "a real network reply",
    "cannot tell",
)
TESTDATA = "a file under testdata"
WRITES_ITSELF = "a file the test writes itself"
BUILDS_IN_CODE = "a value the test builds in code"
REPLACES = "a production function the test replaces"
PRODUCES_ITSELF = "an error or reply the test produces itself"
NETWORK = "a real network reply"
CANNOT_TELL = "cannot tell"

# The source words a seam hit may be cited under: the test made the value or
# swapped the function, so the seam is named as made up.
SEAM_SOURCES = (REPLACES, PRODUCES_ITSELF, BUILDS_IN_CODE)

# The source words whose inputs go on the made-up-input list. An unknown
# source is listed, never passed over.
MADE_UP_SOURCES = SEAM_SOURCES + (WRITES_ITSELF, CANNOT_TELL)

STAND_IN_WORDS = (
    "throws",
    "returns a value the test builds",
    "returns a file's content",
    "waits then calls the real function",
    "records its arguments",
    "cannot tell",
)

# The widest range one cite may name, in lines, so one range cannot cover a
# whole test.
MAX_RANGE_LINES = 30

SEP = " | "
CITE_SEP = " ; "
TRACE_SEP = " -> "
NO_LINE_CITES = ("library", "cannot tell")

LINE_CITE = re.compile(r"^code/(?P<path>[^:|]+?):(?P<a>\d+)(?:-(?P<b>\d+))?$")
# A line cite anywhere in a line, for readers that accept older facts files.
LINE_CITE_ANYWHERE = re.compile(r"code/(?P<path>[^\s:|;,()\[\]]+):(?P<a>\d+)(?:-(?P<b>\d+))?")

FORMAT_TEXT = f"""\
The facts file has these line kinds and no others. Blank lines are allowed.
Fields are separated by "{SEP}". No field except a cite and a check's two
sides may hold parentheses or brackets, and nothing may follow the last field.

## <test name>
location: <cite of the test's first line>
check: <left> | <right> | left made at: <cites> | right made at: <cites> | same function: <yes <line cite>, no, or cannot tell>
input: <name> | source: <source word> | trace: <cites>

An input whose source is "{REPLACES}" has two more fields
between source and trace:

input: <name> | source: {REPLACES} | replaces: <function name> | stands in: <stand-in word> <cite> | trace: <cites>

A check's left and right are code copied from the copy, or the word true or
false. Cites in a "made at" field are joined by "{CITE_SEP.strip()}" with a space each side.
Cites in a trace are joined by "{TRACE_SEP.strip()}" with a space each side.

A cite is one of:
  code/<path>:<N>          one line
  code/<path>:<A>-<B>      a range of at most {MAX_RANGE_LINES} lines
  code/<path>              a file or folder under code/testdata or code/tests
  library                  code outside the copy
  cannot tell

An input whose source is "{TESTDATA}" cites the file or folder under
code/testdata it comes from, with no line. An input whose source is
"{NETWORK}" cites the recorded reply file under code/testdata or code/tests,
with no line.

Source words, one of:
""" + "".join(f"  {w}\n" for w in SOURCE_WORDS) + """
Stand-in words, one of:
""" + "".join(f"  {w}\n" for w in STAND_IN_WORDS)


class Cite:
    """One parsed cite. kind is "line", "bare" or "word"."""

    def __init__(self, kind, text, path=None, a=None, b=None):
        self.kind = kind
        self.text = text
        self.path = path
        self.a = a
        self.b = b if b is not None else a

    def covers(self, path, line):
        return self.kind == "line" and same_path(self.path, path) and self.a <= line <= self.b


def norm_path(path):
    return path.replace("\\", "/").strip("/").lower()


def same_path(p, q):
    return norm_path(p) == norm_path(q)


def parse_cite(text):
    """A Cite, or None when the text is no cite form."""
    if text in NO_LINE_CITES:
        return Cite("word", text)
    m = LINE_CITE.match(text)
    if m:
        a = int(m.group("a"))
        b = int(m.group("b")) if m.group("b") else None
        return Cite("line", text, "code/" + m.group("path"), a, b)
    if text.startswith("code/") and ":" not in text and text.strip() == text:
        return Cite("bare", text, text.rstrip("/"))
    return None


def cites_anywhere(line):
    """Every line cite in a line of any facts file, old or new."""
    out = []
    for m in LINE_CITE_ANYWHERE.finditer(line):
        a = int(m.group("a"))
        b = int(m.group("b")) if m.group("b") else None
        out.append(Cite("line", m.group(0), "code/" + m.group("path"), a, b))
    return out


def copy_file(copy, cite_path):
    """The file a code/... cite names inside the copy, or None if it escapes."""
    rel = cite_path.replace("\\", "/")
    parts = rel.split("/")
    if parts[0] != "code" or any(p in ("", ".", "..") for p in parts) or ":" in rel:
        return None
    return os.path.join(copy, *parts)


def line_count(path):
    """Newline count of a file, the number wc -l prints."""
    with open(path, "rb") as f:
        return f.read().count(b"\n")


def field_value(field, key):
    prefix = key + ": "
    if field.startswith(prefix):
        return field[len(prefix):]
    return None


class Section:
    def __init__(self, name, lineno):
        self.name = name
        self.lineno = lineno
        self.location = None
        self.checks = []
        self.inputs = []


class Input:
    def __init__(self, lineno, name, source, trace, replaces=None, stands_in=None, raw=""):
        self.lineno = lineno
        self.name = name
        self.source = source
        self.trace = trace
        self.replaces = replaces
        self.stands_in = stands_in
        self.raw = raw


def read_lenient(path):
    """Sections with their location and inputs, read from any facts file.

    It reads only `## `, `location:` and `input:` lines and takes each input's
    name, source word and the line cites anywhere on its line, so it also reads
    facts written before this format. Fields it cannot find are None."""
    sections = []
    cur = None
    with open(path, encoding="utf-8") as f:
        for n, raw in enumerate(f, 1):
            line = raw.rstrip("\r\n")
            if line.startswith("## "):
                cur = Section(line[3:].strip(), n)
                sections.append(cur)
            elif cur is None:
                continue
            elif line.startswith("location: "):
                cites = cites_anywhere(line)
                cur.location = cites[0] if cites else None
            elif line.startswith("input: "):
                fields = line[len("input: "):].split(SEP)
                values = {}
                for fl in fields[1:]:
                    for key in ("source", "replaces", "stands in", "trace"):
                        v = field_value(fl, key)
                        if v is not None and key not in values:
                            values[key] = v
                cur.inputs.append(Input(
                    n, fields[0], values.get("source"), cites_anywhere(line),
                    values.get("replaces"), values.get("stands in"), line))
    return sections


if __name__ == "__main__":
    sys.stdout.write(FORMAT_TEXT)
