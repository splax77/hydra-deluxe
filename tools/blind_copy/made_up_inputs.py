"""Write the made-up-input list from a facts file and the grader's output.

    py -I tools/blind_copy/made_up_inputs.py <facts> <grader output> [<out file>]

It writes one row for every `input:` whose source word is in
facts_format.MADE_UP_SOURCES, and one row for every `miss:` line of
grade_seams.py's output. Every row is open: this script never closes one.
With no <out file> it writes to stdout.
"""

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import facts_format as ff  # noqa: E402

COLUMNS = ("test", "input", "source word", "replaced function", "stand-in word", "cites",
           "found by", "status")
STATUS = "open"
BY_FACTS = "pass 1"
BY_SEARCH = "found by search, not by pass 1"
NONE = "-"


def row(test, name, source, replaced, stand_in, cites, found_by):
    """One list row. It takes no status: every row is open."""
    return (test, name, source, replaced, stand_in, cites, found_by, STATUS)


def stand_in_word(text):
    if not text:
        return NONE
    for word in sorted(ff.STAND_IN_WORDS, key=len, reverse=True):
        if text == word or text.startswith(word + " "):
            return word
    return text


def facts_rows(facts):
    out = []
    for s in ff.read_lenient(facts):
        for inp in s.inputs:
            if inp.source not in ff.MADE_UP_SOURCES:
                continue
            trace = inp.raw.split("trace: ", 1)[1] if "trace: " in inp.raw else ""
            cites = trace or "; ".join(c.text for c in inp.trace) or NONE
            out.append(row(s.name, inp.name, inp.source, inp.replaces or NONE,
                           stand_in_word(inp.stands_in), cites, BY_FACTS))
    return out


def miss_rows(grader_output):
    out = []
    with open(grader_output, encoding="utf-8") as f:
        for raw in f:
            line = raw.rstrip("\r\n")
            if not line.startswith("miss: "):
                continue
            parts = line[len("miss: "):].split(ff.SEP)
            if len(parts) != 3:
                raise ValueError(f"grader miss line needs three fields: {line!r}")
            test, cite, labels = parts
            out.append(row(test, f"seam: {labels}", NONE, NONE, NONE, cite, BY_SEARCH))
    return out


def cell(text):
    return text.replace("|", "\\|")


def render(rows):
    lines = ["| " + " | ".join(COLUMNS) + " |", "|" + "---|" * len(COLUMNS)]
    for r in rows:
        lines.append("| " + " | ".join(cell(c) for c in r) + " |")
    return "\n".join(lines) + "\n"


def main(argv):
    if len(argv) not in (3, 4):
        sys.stderr.write(__doc__)
        return 2
    try:
        rows = facts_rows(argv[1]) + miss_rows(argv[2])
    except (OSError, ValueError) as e:
        sys.stderr.write(f"made_up_inputs: {e}\n")
        return 2
    text = render(rows)
    if len(argv) == 4:
        with open(argv[3], "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        print(f"wrote {len(rows)} rows to {argv[3]}")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
