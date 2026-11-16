"""F1 step 2: each copied line -> its doctest test -> what the full run says it caught.

Usage: py -I tools/mutate/first_use.py <first_use_lines.txt> <run folder> <out.md> [--mutants <mutants.json>]

The test for a line is the test case in that file whose registered line is the
nearest at or before it (the listener's ids carry doctest's registered line).
A line that sits outside every TEST_CASE (a helper) gets no test and is marked so.

Every figure comes from report.py's build_report, the same result tests.txt
is written from, so a test's "Bugs it caught" here is its "caught" there.
That count holds both kinds of catch report.py's catches gives; the table
shows the part through shared state beside it. Nothing is judged here.
--mutants defaults to the place report.py looks.
"""
from __future__ import annotations

import argparse
import collections
import os
import sys
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import report  # noqa: E402

REPO = Path(HERE).parents[1]


def _slash(f: str) -> str:
    return f.replace("\\\\", "\\").replace("\\", "/")


def read_entries(lines_path: Path) -> tuple[list[tuple[str, str, str, int]], list[str]]:
    """The copied lines (slice, re-read line, file, line), and the rows the copy could not place."""
    entries, not_placed = [], []
    section = "entries"
    for raw in lines_path.read_text(encoding="utf-8").splitlines():
        if not raw.strip() or raw.startswith("#"):
            if "not placed" in raw.lower():
                section = "not placed"
            continue
        if section == "not placed":
            not_placed.append(raw)
            continue
        parts = raw.split("\t")
        if len(parts) < 4 or not parts[3].strip().isdigit():
            if "not placed" in raw.lower():
                section = "not placed"
                continue
            sys.exit(f"first_use.py: cannot read entry: {raw!r}")
        entries.append((parts[0], parts[1], parts[2].strip().replace("\\", "/"), int(parts[3])))
    return entries, not_placed


def inside_test_case(repo: Path, f: str, ln: int) -> bool:
    """True when the nearest top-level line at or above `ln` opens a TEST_CASE.

    Walking up from the line, the first line that starts at column 0 and is
    not a brace, a blank, a comment or a preprocessor line is the definition
    the line sits in. A helper function there means the line is in no test.
    """
    src = (repo / f).read_text(encoding="utf-8", errors="replace").splitlines()
    for i in range(ln - 1, -1, -1):
        t = src[i]
        if not t or t[0] in " \t}" or t.startswith("//") or t.startswith("#") or t.startswith("/*") or t.startswith("*"):
            continue
        return t.startswith("TEST_CASE") or t.startswith("TEST_SUITE")
    return False


def table_rows(entries, r: dict, repo: Path) -> list[dict]:
    """One row per copied line, every figure read from build_report's result `r`."""
    tests_by_file = collections.defaultdict(list)
    for tid in r["map"]["order"]:
        t = r["map"]["tests"][tid]
        tests_by_file[_slash(t["file"])].append((t["line"], tid, t["name"]))
    for v in tests_by_file.values():
        v.sort()
    # (file, line) -> the bugs whose catch listed a failed check on that line.
    line_failed = collections.defaultdict(set)
    shared = collections.defaultdict(set)  # test id -> bugs it caught through shared state
    for n, cs in r["catches"].items():
        for c in cs:
            for f, ln in c["checks"]:
                line_failed[(_slash(f), int(ln))].add(n)
            if c["how"] == report.HOW_SHARED:
                shared[c["test"]].add(n)

    rows = []
    for s, md, f, ln in entries:
        cands = [t for t in tests_by_file.get(f, []) if t[0] <= ln]
        exact = sorted(line_failed.get((f, ln), set()))
        if not cands or not inside_test_case(repo, f, ln):
            rows.append(dict(slice=s, md=md, file=f, line=ln, test=None, test_id=None, reaches=None,
                             caught=[], shared=[], exact=exact))
            continue
        _, tid, name = cands[-1]
        rec = r["tests"][tid]
        rows.append(dict(slice=s, md=md, file=f, line=ln, test=name, test_id=tid, reaches=len(rec["reaches"]),
                         caught=sorted(rec["caught"]), shared=sorted(shared[tid]), exact=exact))
    return rows


def _exact_text(exact: list[int], none_text: str) -> str:
    if not exact:
        return none_text
    more = ", ..." if len(exact) > 8 else ""
    return f"{len(exact)} bugs ({', '.join(map(str, exact[:8]))}{more})"


def table_text(rows: list[dict], not_placed: list[str], run_name: str) -> str:
    out = [f"# F1: the 62 disputed rows against the full mutation run {run_name}", "",
           "The tool plants its own kinds of bug. It does not plant the change a row names. "
           "So a row whose test caught nothing is \"not shown to fail by this run\". That is all it says. "
           "No row holds an agent's verdict.", "",
           "One re-read row can name several lines, so the table has one line per named line "
           "(75 lines from 61 rows). The 62nd row names no line and is listed at the end.", "",
           "\"Bugs it reaches\" is how many planted bugs sit in code that test runs. "
           "\"Bugs it caught\" is how many bugs made the test fail, the same count as the run's tests.txt. "
           "It counts a bug the test reaches, and also a bug the test does not reach but an earlier test in the "
           "same run did, so the bug could reach it through state the run keeps between tests. "
           "The part in brackets is how many of the caught bugs were of that second kind. "
           "\"This line failed in\" counts the bugs where the check on that exact line was one of the failed checks. "
           "A line with no check of its own (a comment, a closing brace, or a message string inside a check) can never "
           "match, so it reads none.", "",
           "\"Outside every test case\" means the line sits in a helper function, not in a test. "
           "No test is named for it. The table still says whether a check on that line failed.", "",
           "| Slice | Re-read line | Test line | Test | Bugs it reaches | Bugs it caught | This line failed in |",
           "|---|---|---|---|---|---|---|"]
    for r in rows:
        if r["test"] is None:
            out.append(f"| {r['slice']} | {r['md']} | {r['file']}:{r['line']} | outside every test case | | | "
                       f"{_exact_text(r['exact'], 'none: not shown to fail by this run')} |")
            continue
        if not r["caught"]:
            caught_txt = "0: not shown to fail by this run"
        elif r["shared"]:
            caught_txt = f"{len(r['caught'])} ({len(r['shared'])} through shared state)"
        else:
            caught_txt = f"{len(r['caught'])}"
        out.append(f"| {r['slice']} | {r['md']} | {r['file']}:{r['line']} | {r['test']} | {r['reaches']} | "
                   f"{caught_txt} | {_exact_text(r['exact'], 'none')} |")
    if not_placed:
        out += ["", "Rows the copy could not place on a line, as the copy gave them:", ""]
        out += [f"- {x}" for x in not_placed]
    return "\n".join(out) + "\n"


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="F1's table: each copied line, its test and what the run says.")
    ap.add_argument("lines", type=Path, help="first_use_lines.txt")
    ap.add_argument("run_dir", type=Path, help="the run folder run.py wrote")
    ap.add_argument("out", type=Path, help="the table to write")
    ap.add_argument("--mutants", type=Path, default=None,
                    help="the build's mutants.json (default: where report.py looks)")
    a = ap.parse_args(argv)
    run_dir = a.run_dir.resolve()
    mutants = (a.mutants or run_dir.parent.parent / "mutants.json").resolve()
    entries, not_placed = read_entries(a.lines)
    r = report.build_report(run_dir, mutants)
    rows = table_rows(entries, r, REPO)
    a.out.write_text(table_text(rows, not_placed, r["run"]), encoding="utf-8")
    print("entries", len(rows), "| test caught something:", sum(1 for x in rows if x["caught"]),
          "| exact line failed:", sum(1 for x in rows if x["exact"]), "| not placed:", len(not_placed))
    return 0


if __name__ == "__main__":
    sys.exit(main())
