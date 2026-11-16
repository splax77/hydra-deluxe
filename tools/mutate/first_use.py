"""F1 step 2: each copied line -> its doctest test -> what the full run says it caught.

Usage: py -I tools/mutate/first_use.py <first_use_lines.txt> <run folder> <out.md>

The test for a line is the test case in that file whose registered line is the
nearest at or before it (the listener's ids carry doctest's registered line).
A line that sits outside every TEST_CASE (a helper) gets no test and is marked so. Every figure is read from the run's
bug jsons; nothing is judged.
"""
import collections, json, re, sys
from pathlib import Path

lines_path, run_dir, out_path = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
stored = json.loads((run_dir / "map.json").read_text(encoding="utf-8"))
tests_by_file = collections.defaultdict(list)
for t in stored["tests"]:
    f = t["file"].replace("\\\\", "\\").replace("\\", "/")
    tests_by_file[f].append((t["line"], t["id"], t["name"]))
for v in tests_by_file.values():
    v.sort()
reach_count = collections.Counter()
for n, tids in stored["reach"].items():
    for tid in tids:
        reach_count[tid] += 1

caught = collections.defaultdict(list)       # test id -> bug numbers it caught
line_failed = collections.defaultdict(set)   # (file, line) -> bug numbers where that check failed
for p in (run_dir / "bugs").glob("*.json"):
    b = json.loads(p.read_text(encoding="utf-8"))
    for c in b.get("caught_by") or []:
        caught[c["test"]].append(b["number"])
        for f, ln in c.get("checks") or []:
            line_failed[(f.replace("\\\\", "\\").replace("\\", "/"), int(ln))].add(b["number"])

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
        sys.exit(f"f1_table: cannot read entry: {raw!r}")
    entries.append((parts[0], parts[1], parts[2].strip().replace("\\", "/"), int(parts[3])))

REPO = Path(__file__).resolve().parents[2]


def inside_test_case(f, ln):
    """True when the nearest top-level line at or above `ln` opens a TEST_CASE.

    Walking up from the line, the first line that starts at column 0 and is
    not a brace, a blank, a comment or a preprocessor line is the definition
    the line sits in. A helper function there means the line is in no test.
    """
    src = (REPO / f).read_text(encoding="utf-8", errors="replace").splitlines()
    for i in range(ln - 1, -1, -1):
        t = src[i]
        if not t or t[0] in " \t}" or t.startswith("//") or t.startswith("#") or t.startswith("/*") or t.startswith("*"):
            continue
        return t.startswith("TEST_CASE") or t.startswith("TEST_SUITE")
    return False


rows = []
for s, md, f, ln in entries:
    cands = [t for t in tests_by_file.get(f, []) if t[0] <= ln]
    exact = sorted(line_failed.get((f, ln), set()))
    if not cands or not inside_test_case(f, ln):
        rows.append(dict(slice=s, md=md, file=f, line=ln, test=None, test_id=None, reaches=None,
                         caught=[], exact=exact))
        continue
    _, tid, name = cands[-1]
    bugs = sorted(caught.get(tid, []))
    rows.append(dict(slice=s, md=md, file=f, line=ln, test=name, test_id=tid, reaches=reach_count[tid],
                     caught=bugs, exact=exact))

out = ["# F1: the 62 disputed rows against the full mutation run 20261116-123523", "",
       "The tool plants its own kinds of bug. It does not plant the change a row names. "
       "So a row whose test caught nothing is \"not shown to fail by this run\". That is all it says. "
       "No row holds an agent's verdict.", "",
       "One re-read row can name several lines, so the table has one line per named line "
       "(75 lines from 61 rows). The 62nd row names no line and is listed at the end.", "",
       "\"Bugs it reaches\" is how many planted bugs sit in code that test runs. "
       "\"Bugs it caught\" is how many of those made the test fail. "
       "\"This line failed in\" counts the bugs where the check on that exact line was one of the failed checks. "
       "A line with no check of its own (a comment, a closing brace, or a message string inside a check) can never match, so it reads none.", "",
       "\"Outside every test case\" means the line sits in a helper function, not in a test. "
       "No test is named for it. The table still says whether a check on that line failed.", "",
       "| Slice | Re-read line | Test line | Test | Bugs it reaches | Bugs it caught | This line failed in |",
       "|---|---|---|---|---|---|---|"]
for r in rows:
    if r["test"] is None:
        exact_txt = (f"{len(r['exact'])} bugs ({', '.join(map(str, r['exact'][:8]))}{', ...' if len(r['exact']) > 8 else ''})"
                     if r["exact"] else "none: not shown to fail by this run")
        out.append(f"| {r['slice']} | {r['md']} | {r['file']}:{r['line']} | outside every test case | | | {exact_txt} |")
        continue
    caught_txt = f"{len(r['caught'])}" if r["caught"] else "0: not shown to fail by this run"
    exact_txt = (f"{len(r['exact'])} bugs ({', '.join(map(str, r['exact'][:8]))}{', ...' if len(r['exact']) > 8 else ''})"
                 if r["exact"] else "none")
    out.append(f"| {r['slice']} | {r['md']} | {r['file']}:{r['line']} | {r['test']} | {r['reaches']} | "
               f"{caught_txt} | {exact_txt} |")
if not_placed:
    out += ["", "Rows the copy could not place on a line, as the copy gave them:", ""]
    out += [f"- {x}" for x in not_placed]
out_path.write_text("\n".join(out) + "\n", encoding="utf-8")
print("entries", len(rows), "| test caught something:", sum(1 for r in rows if r["caught"]),
      "| exact line failed:", sum(1 for r in rows if r["exact"]), "| not placed:", len(not_placed))


