"""Grade a facts file against the seams a search finds in its slice.

    py -I tools/blind_copy/grade_seams.py <copy> <slice ranges> <facts>

<copy> is the blind copy's root, the folder that holds `code/`. <slice ranges>
is one or more `<path>:<first>-<last>` joined by commas, with paths as the
facts cite them, with or without the leading `code/`. The forms searched for
are in seam_patterns.txt.

A test is a TEST_CASE, a UI test (see ui_registrations), or a top-level unit
nothing else in its file names. A test owns a hit on its own lines, and a hit
inside any helper in the slice its body names, directly or through another helper. It also owns the hits in
a helper, type or macro outside the slice that the test or one of those slice
helpers names, when it is defined in a slice file, in a file the slice files
include from tests/, or in the .cpp of such a header; those are read one step
only. A seam two steps into such a file, or in a file the slice does not
include, is not found. A hit is found when an
`input:` row of that test cites its line under one of the seam source words in
facts_format.py; otherwise it is a miss. It reads only `## `, `location:` and
`input:` rows, so it also grades facts written before facts_format.py.
"""

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402

PATTERNS = os.path.join(HERE, "seam_patterns.txt")
TEST_MACROS = ("TEST_CASE", "TEST_CASE_FIXTURE", "SCENARIO")
NAMESPACE_HEAD = re.compile(r'^\s*(?:inline\s+)?(?:namespace\b|extern\s+"C")')
TYPE_HEAD = re.compile(r"^\s*(?:template\s*<[^>]*>\s*)?(?:struct|class|union|enum(?:\s+class)?)\s+([A-Za-z_]\w*)")
CALL_HEAD = re.compile(r"([A-Za-z_]\w*)\s*\(")
STRING = re.compile(r'"((?:[^"\\]|\\.)*)"')
UI_TEST_HEAD = re.compile(r"\(\s*ImGuiTestContext\s*\*\s*\w*\s*\)")
# Matched on a unit's code, where comments are gone and a string literal is
# cut out, so a test's name is the empty gap before the comma.
REG_ENTRY = re.compile(r"\{\s*,\s*&?([A-Za-z_]\w*)\s*\}")
REG_CALL = re.compile(r"\bregister_test\s*\(\s*\w+\s*,\s*,\s*&?([A-Za-z_]\w*)\s*\)")


class Pattern:
    def __init__(self, label, regex, example_file, example):
        self.label = label
        self.regex = re.compile(regex)
        self.example_file = example_file
        self.example = example


def load_patterns(path=PATTERNS):
    out = []
    with open(path, encoding="utf-8") as f:
        for raw in f:
            line = raw.rstrip("\r\n")
            if not line.strip() or line.startswith("#"):
                continue
            parts = line.split(ff.SEP, 3)
            if len(parts) != 4:
                raise ValueError(f"seam pattern line needs four fields: {line!r}")
            out.append(Pattern(*parts))
    return out


def code_marks(text):
    """Per line: the `{`, `}` and `;` outside comments, strings and chars,
    and the line's text with comments and literals blanked."""
    lines = text.split("\n")
    marks = [[] for _ in lines]
    code = [[] for _ in lines]
    i, n, ln = 0, len(text), 0
    state = None
    raw_end = None
    while i < n:
        ch = text[i]
        if ch == "\n":
            ln += 1
            if state == "line":
                state = None
            i += 1
            continue
        if state == "line":
            i += 1
            continue
        if state == "block":
            if text.startswith("*/", i):
                state = None
                i += 2
            else:
                i += 1
            continue
        if state == "raw":
            if text.startswith(raw_end, i):
                i += len(raw_end)
                state = None
            else:
                i += 1
            continue
        if state in ('"', "'"):
            if ch == "\\":
                i += 2
                continue
            if ch == state:
                state = None
            i += 1
            continue
        if text.startswith("//", i):
            state = "line"
            i += 2
            continue
        if text.startswith("/*", i):
            state = "block"
            i += 2
            continue
        m = re.match(r'(?:u8|u|U|L)?R"([^(\s]*)\(', text[i:i + 24])
        if m and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_")):
            raw_end = ")" + m.group(1) + '"'
            state = "raw"
            i += m.end()
            continue
        if ch == '"':
            state = '"'
            i += 1
            continue
        if ch == "'":
            prev = text[i - 1] if i else ""
            nxt = text[i + 1] if i + 1 < n else ""
            if prev.isalnum() and nxt.isalnum():
                i += 1
                continue
            state = "'"
            i += 1
            continue
        if ch in "{};":
            marks[ln].append(ch)
        code[ln].append(ch)
        i += 1
    return marks, ["".join(c) for c in code]


class Unit:
    def __init__(self, file, start, end, name, is_test_case, text, code):
        self.file = file
        self.start = start
        self.end = end
        self.name = name
        self.is_test_case = is_test_case
        self.text = text
        self.code = code
        self.tokens = set(re.findall(r"\w+", code))

    def label(self):
        return self.name or f"{self.file}:{self.start}"


def unit_name(body):
    stripped = body.lstrip()
    for macro in TEST_MACROS:
        if re.match(macro + r"\s*\(", stripped):
            m = STRING.search(body)
            return (m.group(1) if m else None), True
    head = body.split("{", 1)[0].split(";", 1)[0]
    m = TYPE_HEAD.match(head)
    if m:
        return m.group(1), False
    m = CALL_HEAD.search(head)
    if m:
        return m.group(1), False
    return None, False


def in_parens(head):
    """True when the head text has more '(' than ')' so far."""
    return head.count("(") > head.count(")")


def top_units(file, text):
    """The file's top-level definitions and declarations, one Unit each.
    A namespace's own braces do not make a unit; what is inside them does."""
    lines = text.split("\n")
    marks, code = code_marks(text)
    units = []
    ns = []
    depth = 0
    cur = None
    opened = False
    in_macro = False
    for idx, line in enumerate(lines):
        n = idx + 1
        s = line.strip()
        if in_macro:
            in_macro = s.endswith("\\")
            continue
        if cur is None:
            if not s:
                continue
            if s.startswith("#"):
                in_macro = s.endswith("\\")
                continue
            cur, opened = n, False
        close = False
        spots = [i for i, c in enumerate(code[idx]) if c in "{};"]
        for k, mk in enumerate(marks[idx]):
            base = len(ns)
            if mk == "{":
                if cur is not None and not opened and depth == base and \
                        in_parens("\n".join(code[cur - 1:idx]) + "\n" + code[idx][:spots[k]]):
                    # A brace inside the head's open parenthesis, such as a
                    # default argument `= {}`, is not the body.
                    depth += 1
                    continue
                if cur is not None and not opened and depth == base:
                    head = "\n".join(lines[cur - 1:n])
                    if NAMESPACE_HEAD.match(head):
                        ns.append(depth)
                        depth += 1
                        cur = None
                        continue
                    opened = True
                depth += 1
            elif mk == "}":
                depth -= 1
                if ns and depth == ns[-1]:
                    ns.pop()
                    if cur is not None and not opened:
                        cur = None
                elif cur is not None and opened and depth == len(ns):
                    close = True
            elif mk == ";":
                if cur is not None and not opened and depth == len(ns):
                    close = True
        if close and cur is not None:
            body = "\n".join(lines[cur - 1:n])
            name, is_tc = unit_name(body)
            units.append(Unit(file, cur, n, name, is_tc, body, "\n".join(code[cur - 1:n])))
            cur = None
    return units


def macro_units(file, text):
    """One Unit per `#define NAME`, with its continued lines."""
    lines = text.split("\n")
    _, code = code_marks(text)
    out = []
    idx = 0
    while idx < len(lines):
        m = re.match(r"\s*#\s*define\s+(\w+)", lines[idx])
        if not m:
            idx += 1
            continue
        end = idx
        while end < len(lines) - 1 and lines[end].rstrip().endswith("\\"):
            end += 1
        body = "\n".join(lines[idx:end + 1])
        out.append(Unit(file, idx + 1, end + 1, m.group(1), False, body, "\n".join(code[idx:end + 1])))
        idx = end + 1
    return out


INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)


def scope_files(copy, files):
    """The slice files, the quoted-include closure of them under code/tests,
    and the .cpp that shares each header's name: the files whose helpers,
    types and macros a test can reach. Paths are as the facts cite them."""
    seen = []
    todo = list(files)
    while todo:
        path = todo.pop()
        if path in seen:
            continue
        real = ff.copy_file(copy, path)
        if real is None or not os.path.isfile(real):
            continue
        seen.append(path)
        with open(real, encoding="utf-8", errors="replace") as f:
            text = f.read()
        here = os.path.dirname(path)
        for inc in INCLUDE.findall(text):
            for base in (here, "code/tests", "code"):
                cand = os.path.normpath(os.path.join(base, inc)).replace("\\", "/")
                if cand.startswith("code/tests/") and os.path.isfile(ff.copy_file(copy, cand) or ""):
                    todo.append(cand)
                    break
        stem, ext = os.path.splitext(path)
        if ext == ".h":
            sibling = stem + ".cpp"
            if os.path.isfile(ff.copy_file(copy, sibling) or ""):
                todo.append(sibling)
    return seen


def parse_ranges(arg):
    out = []
    for part in arg.split(","):
        part = part.strip()
        m = re.match(r"^(?P<path>.+):(?P<a>\d+)-(?P<b>\d+)$", part)
        if not m:
            raise ValueError(f"slice range must be <path>:<first>-<last>: {part!r}")
        path = m.group("path").replace("\\", "/")
        if not path.startswith("code/"):
            path = "code/" + path
        out.append((path, int(m.group("a")), int(m.group("b"))))
    return out


def names(text, name):
    return re.search(r"(?<![\w])" + re.escape(name) + r"(?![\w])", text) is not None


def ui_registrations(units):
    """The UI tests among `units`, and the tables that register them.

    A UI test is a function the Dear ImGui test engine runs: its head takes an
    `ImGuiTestContext*`, and a table in the same file lists it by name in a
    `{"<name>", <function>}` entry or a `register_test(<engine>, "<name>",
    <function>)` call, read from code with comments and strings cut out. Such a function is a test although the
    table names it, and the table is not a test. Returns two sets of unit ids."""
    runnable = {u.name: u for u in units
                if u.name and UI_TEST_HEAD.search(u.text.split("{", 1)[0])}
    registered, tables = set(), set()
    for u in units:
        listed = [runnable[n]
                  for n in REG_ENTRY.findall(u.code) + REG_CALL.findall(u.code)
                  if n in runnable and n != u.name]
        if listed:
            registered.update(id(t) for t in listed)
            tables.add(id(u))
    return registered, tables


def grade(copy, ranges, facts, patterns=None):
    """A dict with the slice files' line counts, the tests, and every
    (test, hit) pair with whether the facts cite it."""
    patterns = patterns if patterns is not None else load_patterns()
    sections = ff.read_lenient(facts)
    files = []
    for path, _, _ in ranges:
        if path not in files:
            files.append(path)
    counts = {}
    slice_units = []
    hits = []
    for path in files:
        real = ff.copy_file(copy, path)
        if real is None or not os.path.isfile(real):
            raise FileNotFoundError(f"slice file is not in the copy: {path}")
        counts[path] = ff.line_count(real)
        with open(real, encoding="utf-8", errors="replace") as f:
            text = f.read()
        lines = text.split("\n")
        _, code = code_marks(text)
        spans = [(a, b) for p, a, b in ranges if p == path]
        inside = lambda n: any(a <= n <= b for a, b in spans)  # noqa: E731
        for u in top_units(path, text):
            if any(u.start <= b and a <= u.end for a, b in spans):
                slice_units.append(u)
        for idx, line in enumerate(lines):
            n = idx + 1
            if not inside(n):
                continue
            labels = [p.label for p in patterns if p.regex.search(line)]
            if labels:
                hits.append((path, n, labels))
    registered, tables = ui_registrations(slice_units)
    for u in slice_units:
        if id(u) in registered:
            u.is_test_case = True
    named = [u for u in slice_units if u.name and not u.is_test_case]
    helpers = set()
    for h in named:
        for u in slice_units:
            if u is not h and names(u.code, h.name):
                helpers.add(id(h))
                break
    tests = [u for u in slice_units if id(u) not in helpers and id(u) not in tables]

    # Helpers, types and macros outside the slice: in the slice files' own
    # other lines, in the files they include, and in each header's .cpp.
    in_slice = {(u.file, u.start) for u in slice_units}
    outside = []
    file_lines = {}
    for path in scope_files(copy, files):
        real = ff.copy_file(copy, path)
        with open(real, encoding="utf-8", errors="replace") as f:
            text = f.read()
        file_lines[path] = text.split("\n")
        for u in top_units(path, text) + macro_units(path, text):
            if u.name and not u.is_test_case and (u.file, u.start) not in in_slice:
                outside.append(u)
    pool = [u for u in named if id(u) in helpers] + outside
    by_name = {}
    for u in pool:
        by_name.setdefault(u.name, []).append(u)

    def reached(t):
        seen, todo = [], [t]
        while todo:
            u = todo.pop()
            for tok in u.tokens:
                for h in by_name.get(tok, ()):
                    if h not in seen and h is not t:
                        seen.append(h)
                        # Slice helpers lead on to what they name. A helper
                        # outside the slice is read one step only: following
                        # the whole harness would give every test every seam.
                        if id(h) in helpers:
                            todo.append(h)
        return seen

    def outside_hits(u):
        out = []
        lines = file_lines.get(u.file, [])
        for n in range(u.start, min(u.end, len(lines)) + 1):
            labels = [p.label for p in patterns if p.regex.search(lines[n - 1])]
            if labels:
                out.append((u.file, n, labels))
        return out

    def sections_of(t):
        out = []
        for s in sections:
            loc = s.location
            if loc is not None and ff.same_path(loc.path, t.file) and t.start <= loc.a <= t.end:
                out.append(s)
            elif t.name and s.name.strip().strip('"') == t.name and (
                    loc is None or ff.same_path(loc.path, t.file)):
                out.append(s)
        return out

    rows = []
    test_rows = []
    for t in tests:
        own = [t] + reached(t)
        secs = sections_of(t)
        test_rows.append((t, bool(secs)))
        mine = [h for h in hits
                if any(ff.same_path(u.file, h[0]) and u.start <= h[1] <= u.end for u in own)]
        for u in own:
            if u is not t:
                mine += [h for h in outside_hits(u) if h[:2] not in {x[:2] for x in mine}]
        for path, n, labels in mine:
            found = any(
                inp.source in ff.SEAM_SOURCES and any(c.covers(path, n) for c in inp.trace)
                for s in secs for inp in s.inputs)
            rows.append((t, path, n, labels, found))
    owned = {(p, n) for _, p, n, _, _ in rows}
    for path, n, labels in hits:
        if (path, n) not in owned:
            rows.append((None, path, n, labels, False))
    return {"counts": counts, "tests": test_rows, "rows": rows}


def render(result, ranges):
    out = []
    for path, count in result["counts"].items():
        spans = ", ".join(f"{a}-{b}" for p, a, b in ranges if p == path)
        out.append(f"slice file: {path} | lines: {count} | range: {spans}")
    for t, has in result["tests"]:
        if t.is_test_case or (t.name and "{" in t.text):
            out.append(f"test: {t.label()} | {t.file}:{t.start}-{t.end} | facts section: {'yes' if has else 'no'}")
    rows = result["rows"]
    for t, path, n, labels, found in rows:
        out.append(f"hit: {t.label() if t else 'no test'} | {path}:{n} | {', '.join(labels)} | "
                   f"{'found' if found else 'missed'}")
    missed = [r for r in rows if not r[4]]
    out.append(f"hits: {len(rows)}")
    out.append(f"found: {len(rows) - len(missed)}")
    out.append(f"missed: {len(missed)}")
    for t, path, n, labels, _ in missed:
        out.append(f"miss: {t.label() if t else 'no test'} | {path}:{n} | {', '.join(labels)}")
    return "\n".join(out) + "\n"


def main(argv):
    if len(argv) != 4:
        sys.stderr.write(__doc__)
        return 2
    try:
        ranges = parse_ranges(argv[2])
        result = grade(argv[1], ranges, argv[3])
    except (ValueError, FileNotFoundError) as e:
        sys.stderr.write(f"grade_seams: {e}\n")
        return 2
    sys.stdout.write(render(result, ranges))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
