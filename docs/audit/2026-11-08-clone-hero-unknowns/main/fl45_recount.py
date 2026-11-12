"""FL-45's recount: which test-corpus charts' Clone Hero 1.0 fills and best
scores moved when the 1.0 generator went onto the shared measure grid
(merge 18a2c64f, against its first parent 5f357402).

For every chart file under the corpus folder (notes.mid, notes.chart, .sng,
.srb), each of the two hydra_replay builds runs twice with --legacy-fills:
`score` with no activations, for the chords marked is_fill (each with its
fill_start), and `dump`, for the best path's score. The two builds' fill sets
and best scores are then compared chart by chart.

Usage: py fl45_recount.py <before hydra_replay.exe> <after hydra_replay.exe>
       <corpus folder> <scratch folder for the JSON files>
It prints one line per chart that moved, then the totals. It writes JSON
only into the scratch folder it is given.
"""
import json
import os
import subprocess
import sys

CHART_NAMES = ("notes.mid", "notes.chart")
CHART_SUFFIXES = (".sng", ".srb")


def charts(corpus):
    found = []
    for folder, _, files in os.walk(corpus):
        for name in files:
            low = name.lower()
            if low in CHART_NAMES or low.endswith(CHART_SUFFIXES):
                found.append(os.path.join(folder, name))
    return sorted(found)


def run(exe, command, chart, out):
    result = subprocess.run([exe, command, "--chart", chart, "--legacy-fills", "--out", out],
                            capture_output=True, text=True, encoding="utf-8", errors="replace")
    if result.returncode != 0:
        raise SystemExit(f"FAILED: {exe} {command} {chart}: exit {result.returncode}: "
                         f"{result.stderr.strip()[:400]}")
    with open(out, encoding="utf-8") as f:
        return json.load(f)


def measure(exe, chart, scratch, tag, i):
    score = run(exe, "score", chart, os.path.join(scratch, f"{i:03d}-{tag}-score.json"))
    fills = sorted((c.get("fill_start"), c["tick"]) for c in score["chords"] if c["is_fill"])
    dump = run(exe, "dump", chart, os.path.join(scratch, f"{i:03d}-{tag}-dump.json"))
    return fills, dump["result"]["score"], dump["result"]["bestpath"]


def main():
    before_exe, after_exe, corpus, scratch = sys.argv[1:5]
    os.makedirs(scratch, exist_ok=True)
    found = charts(corpus)
    fills_moved = scores_moved = 0
    for i, chart in enumerate(found):
        b_fills, b_score, b_path = measure(before_exe, chart, scratch, "before", i)
        a_fills, a_score, a_path = measure(after_exe, chart, scratch, "after", i)
        name = os.path.relpath(chart, corpus)
        if b_fills != a_fills:
            fills_moved += 1
            gone, new = sorted(set(b_fills) - set(a_fills)), sorted(set(a_fills) - set(b_fills))
            print(f"fills moved: {name}: {len(b_fills)} -> {len(a_fills)} fills; "
                  f"only before {len(gone)} {gone[:4]}; only after {len(new)} {new[:4]}")
        if b_score != a_score or b_path != a_path:
            scores_moved += 1
            print(f"best moved: {name}: {b_score} ({b_path}) -> {a_score} ({a_path})")
    print(f"charts read {len(found)}; 1.0 fills moved on {fills_moved}; "
          f"best score or path moved on {scores_moved}")


if __name__ == "__main__":
    main()
