"""Check a blind recheck comparison file (pass 2's output) against its format.

Run with: py -I tools/blind_copy/check_compare.py <comparison file> <facts file> <main root>

The comparison file sets quotes from main's comments, names, messages and old records beside
the facts file pass 1 wrote, one row per claim. This script is the one home of that format.
It prints one line per rejection, as `<comparison file>:<line>: <reason>`, and exits 1 if
there is any; a clean file prints `ok` and exits 0. Main's root is always given; a quote is
looked up in the file it cites, read from that root.

Every line of the file is blank or one of these five kinds:

    ## <test name>
    claim: "<exact quote>" | from: <kind> <path>:<line> | rests on: <rest> | facts: <lines> | result: <phrase>
    claims: none
    open: "<exact quote of a claim row in the same section>" | settled by: <the search or run>
    count: records the facts contradict: <N> | records resting on a comment, name or claim word: <M> | total records in either count, each counted once: <T>

The words each field may hold are the lists below (KINDS, RESTS, RESULTS). `<path>` is
relative to main's root, forward or back slashes. `<lines>` is `none`, or line numbers of the
facts file, single (`12`) or ranges (`12-14`), joined by ", ". The count line comes once, last. Its total counts a record row once even when
the row is in both counts. A `facts:` line number must lie in the facts section of the claim's
own test. A quote must be a whole phrase of at least MIN_QUOTE characters.
"""

import os
import re
import sys
import unicodedata

KINDS = ("comment", "name", "message", "record")
RESTS = ("nothing", "a comment", "a name", "a claim word")
BACKED = "the facts back this"
UNBACKED = "no fact backs this"
CONTRADICT = "the facts contradict this"
RESULTS = (BACKED, UNBACKED, CONTRADICT)
CLAIM_FIELDS = ("from", "rests on", "facts", "result")
TOTAL_LABEL = "total records in either count, each counted once"
COUNT_LABELS = ("records the facts contradict", "records resting on a comment, name or claim word",
                TOTAL_LABEL)
# Verdict words pass 2 may never write of a comment or record (plan fix 3), with their other
# forms. Quotes, and the `from:` path (it must name a real file on main), are exempt.
VERDICT_WORDS = re.compile(
    r"\b(tru(?:e|ly|th)|hold(?:s|ing)?|held|(?:in)?consisten\w*|(?:in)?correct\w*|"
    r"(?:in)?accura\w*|match(?:es|ed|ing)?|(?:in)?valid\w*|confirm\w*|verif\w*|false|wrong|"
    r"agree\w*|accept\w*|as\s+designed|by\s+design|works\s+as)\b", re.IGNORECASE)
# A quote shorter than this could sit in almost any line, so it proves nothing about its line.
MIN_QUOTE = 10

SEP = " | "


def plain(text):
    """Text with look-alike letters folded and zero-width characters dropped, for the word scan."""
    text = unicodedata.normalize("NFKC", text)
    return "".join(c for c in text if unicodedata.category(c) != "Cf")


def read_lines(path):
    """The file's lines, numbered from 1 as an editor numbers them (only \\n ends a line)."""
    with open(path, encoding="utf-8-sig", errors="replace") as f:
        text = f.read()
    lines = text.split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    return [ln[:-1] if ln.endswith("\r") else ln for ln in lines]


def facts_tests(facts_lines):
    """Test names of the facts file's `## ` lines, in order."""
    return [ln[3:].strip() for ln in facts_lines if ln.startswith("## ")]


def facts_sections(facts_lines):
    """Test name -> (first, last) facts line numbers under its `## ` line, heading excluded."""
    spans = {}
    name = None
    for n, ln in enumerate(facts_lines, 1):
        if ln.startswith("## "):
            name = ln[3:].strip()
            spans.setdefault(name, [n + 1, n])
        elif name is not None and spans[name][1] == n - 1:
            spans[name][1] = n
    return {k: tuple(v) for k, v in spans.items()}


def split_quote(body):
    """(quote, rest) of a field list that opens with a double-quoted quote, or None.

    The quote ends at the first `" | `; a quote may hold double quotes of its own.
    """
    if not body.startswith('"'):
        return None
    end = body.find('"' + SEP, 1)
    if end < 0:
        return None
    return body[1:end], body[end + 1:]


def parse_fields(rest, names):
    """Field values for `| name: value` pieces in the given order, or a reason string."""
    pieces = rest.split(SEP)
    if pieces and pieces[0] == "":
        pieces = pieces[1:]
    got = []
    for piece in pieces:
        name, colon, value = piece.partition(": ")
        got.append((name, value.strip() if colon else None))
    for i, want in enumerate(names):
        if i >= len(got):
            return "missing field '%s'" % want
        name, value = got[i]
        if name != want:
            return "missing field '%s' (found '%s' in its place)" % (want, name)
        if not value:
            return "field '%s' is empty" % want
    if len(got) > len(names):
        return "text after the last field: '%s'" % SEP.join(p for p in pieces[len(names):])
    return [v for _, v in got]


def parse_facts_field(value, n_facts, facts_lines, span, test):
    """List of facts line numbers, [] for `none`, or a reason string.

    Every line must fall inside the facts section of the claim's own test (`span`).
    """
    if value == "none":
        return []
    nums = []
    for part in value.split(", "):
        m = re.fullmatch(r"(\d+)(?:-(\d+))?", part)
        if not m:
            return "facts field '%s' is not line numbers joined by ', '" % value
        a = int(m.group(1))
        b = int(m.group(2)) if m.group(2) else a
        if b < a:
            return "facts range %s runs backwards" % part
        for n in range(a, b + 1):
            if n < 1 or n > n_facts:
                return "facts line %d is not in the facts file (it has %d lines)" % (n, n_facts)
            if not facts_lines[n - 1].strip():
                return "facts line %d is blank" % n
            if span is None or not span[0] <= n <= span[1]:
                return "facts line %d is not in the facts section of '%s'" % (n, test)
            nums.append(n)
    return nums


class SourceCache:
    def __init__(self, root):
        self.root = os.path.realpath(root)
        self.files = {}

    def line(self, rel, n):
        """(text of line n of rel under main's root, or None) and a reason when None."""
        rel_norm = rel.replace("\\", "/")
        if os.path.isabs(rel_norm) or re.match(r"[A-Za-z]:", rel_norm):
            return None, "cited path '%s' is not relative to main's root" % rel
        full = os.path.realpath(os.path.join(self.root, rel_norm))
        if os.path.commonpath([full, self.root]) != self.root:
            return None, "cited path '%s' leaves main's root" % rel
        if full not in self.files:
            self.files[full] = read_lines(full) if os.path.isfile(full) else None
        lines = self.files[full]
        if lines is None:
            return None, "cited file '%s' is not on main" % rel
        if n < 1 or n > len(lines):
            return None, "cited line %s:%d is past the end of the file (%d lines)" % (rel, n, len(lines))
        return lines[n - 1], None


def check(compare_path, facts_path, main_root):
    """List of (line number, reason) rejections of the comparison file; empty means clean."""
    out = []
    lines = read_lines(compare_path)
    facts_lines = read_lines(facts_path)
    wanted = facts_tests(facts_lines)
    spans = facts_sections(facts_lines)
    src = SourceCache(main_root)

    sections = {}          # test name -> line of its `## `
    section = None         # current test name
    claims = {}            # test name -> set of quotes of its claim rows
    has_none = {}          # test name -> line of its `claims: none`
    rows = {}              # test name -> count of claim / claims-none rows
    opens = []             # (line, test, quote) to match against the section's claims
    tally_contra = 0
    tally_rest = 0
    tally_either = 0       # record rows in either count, each once
    count_line = None
    last_content = 0

    for n, ln in enumerate(lines, 1):
        if not ln.strip():
            continue
        last_content = n
        if count_line is not None:
            out.append((n, "line after the count line; the count line comes last"))
        # Verdict words: checked on the line with its quote taken out.
        outside = ln
        for prefix in ("claim: ", "open: "):
            if ln.startswith(prefix):
                q = split_quote(ln[len(prefix):])
                if q:
                    pieces = [p for p in q[1].split(SEP) if not p.startswith("from: ")]
                    outside = prefix + SEP.join(pieces)
        if ln.startswith("## ") and ln[3:].strip() in wanted:
            outside = ""
        m = VERDICT_WORDS.search(plain(outside))
        if m:
            out.append((n, "the word '%s' outside a quote; pass 2 never judges a comment or record"
                        % m.group(1)))

        if ln.startswith("## "):
            name = ln[3:].strip()
            if not name:
                out.append((n, "a '## ' line with no test name"))
                section = None
                continue
            if name in sections:
                out.append((n, "test '%s' has a second section (first at line %d)"
                            % (name, sections[name])))
            elif name not in wanted:
                out.append((n, "test '%s' is not in the facts file" % name))
            sections.setdefault(name, n)
            section = name
            claims.setdefault(name, set())
            rows.setdefault(name, 0)
            continue

        if ln.startswith("count: "):
            count_line = n
            got = parse_fields(SEP + ln[len("count: "):], COUNT_LABELS)
            if isinstance(got, str):
                out.append((n, "count line: " + got))
                continue
            vals = []
            for label, v in zip(COUNT_LABELS, got):
                if not re.fullmatch(r"\d+", v):
                    out.append((n, "count '%s' is not a whole number: '%s'" % (label, v)))
                    break
                vals.append(int(v))
            else:
                count_line = (n, vals)
            continue

        if section is None:
            out.append((n, "line outside any '## <test name>' section: '%s'" % ln[:80]))
            continue

        if ln == "claims: none":
            if section in has_none:
                out.append((n, "a second 'claims: none' in '%s'" % section))
            has_none[section] = n
            rows[section] += 1
            continue

        if ln.startswith("claim: "):
            rows[section] += 1
            q = split_quote(ln[len("claim: "):])
            if not q:
                out.append((n, "claim row does not open with a double-quoted quote then ' | '"))
                continue
            quote, rest = q
            if len(quote) < MIN_QUOTE or quote != quote.strip():
                out.append((n, "claim row's quote is under %d characters or has spaces at its"
                            " ends; quote a whole phrase" % MIN_QUOTE))
                continue
            got = parse_fields(rest, CLAIM_FIELDS)
            if isinstance(got, str):
                out.append((n, got))
                continue
            frm, rest_on, facts, result = got
            claims[section].add(quote)

            fm = re.fullmatch(r"(\S+) (.+):(\d+)", frm)
            if not fm or fm.group(1) not in KINDS:
                out.append((n, "from field '%s' is not '<%s> <path>:<line>'"
                            % (frm, ", ".join(KINDS))))
            else:
                text, why = src.line(fm.group(2), int(fm.group(3)))
                if why:
                    out.append((n, why))
                elif quote not in text:
                    out.append((n, "quote not found at %s:%s on main" % (fm.group(2), fm.group(3))))

            if rest_on not in RESTS:
                out.append((n, "rests on '%s' is not one of: %s" % (rest_on, "; ".join(RESTS))))

            nums = parse_facts_field(facts, len(facts_lines), facts_lines, spans.get(section),
                                     section)
            if isinstance(nums, str):
                out.append((n, nums))
                nums = None

            if result not in RESULTS:
                extra = [r for r in RESULTS if result.startswith(r)]
                if extra:
                    out.append((n, "text after the result: '%s'" % result[len(extra[0]):]))
                else:
                    out.append((n, "result '%s' is not one of: %s" % (result, "; ".join(RESULTS))))
            elif nums is not None:
                if result in (BACKED, CONTRADICT) and not nums:
                    out.append((n, "result '%s' with facts: none; it needs a facts line" % result))
                if result == UNBACKED and nums:
                    out.append((n, "result '%s' names facts lines; use none or another result"
                                % result))

            if fm and fm.group(1) == "record":
                if result == CONTRADICT:
                    tally_contra += 1
                rests = rest_on in ("a comment", "a name", "a claim word")
                if rests:
                    tally_rest += 1
                if rests or result == CONTRADICT:
                    tally_either += 1
            continue

        if ln.startswith("open: "):
            q = split_quote(ln[len("open: "):])
            if not q:
                out.append((n, "open row does not open with a double-quoted quote then ' | '"))
                continue
            got = parse_fields(q[1], ("settled by",))
            if isinstance(got, str):
                out.append((n, "open row: " + got))
                continue
            opens.append((n, section, q[0]))
            continue

        out.append((n, "not one of the five line kinds: '%s'" % ln[:80]))

    for name, n in has_none.items():
        if claims.get(name):
            out.append((n, "'claims: none' in '%s', which also has claim rows" % name))
    for n, name, quote in opens:
        if quote not in claims.get(name, set()):
            out.append((n, "open row's quote is not the quote of a claim row in '%s'" % name))
    for name, n in sections.items():
        if rows.get(name, 0) == 0:
            out.append((n, "test '%s' has no claim row and no 'claims: none'" % name))
    for name in wanted:
        if name not in sections:
            out.append((0, "test '%s' in the facts file has no section" % name))

    if count_line is None:
        out.append((last_content, "no count line"))
    elif isinstance(count_line, tuple):
        n, (contra, rest_c, total) = count_line
        if contra != tally_contra:
            out.append((n, "count of records the facts contradict is %d; the rows give %d"
                        % (contra, tally_contra)))
        if rest_c != tally_rest:
            out.append((n, "count of records resting on a comment, name or claim word is %d;"
                        " the rows give %d" % (rest_c, tally_rest)))
        if total != tally_either:
            out.append((n, "total is %d; the rows give %d records in either count"
                        % (total, tally_either)))

    out.sort(key=lambda r: r[0])
    return out


def main(argv):
    if len(argv) != 4:
        sys.stderr.write("usage: check_compare.py <comparison file> <facts file> <main root>\n")
        return 2
    compare_path, facts_path, root = argv[1:]
    for p, what in ((compare_path, "comparison file"), (facts_path, "facts file")):
        if not os.path.isfile(p):
            sys.stderr.write("%s not found: %s\n" % (what, p))
            return 2
    if not os.path.isdir(root):
        sys.stderr.write("main root is not a folder: %s\n" % root)
        return 2
    rejections = check(compare_path, facts_path, root)
    for n, why in rejections:
        print("%s:%d: %s" % (compare_path, n, why))
    if rejections:
        print("%d rejections" % len(rejections))
        return 1
    print("ok")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
