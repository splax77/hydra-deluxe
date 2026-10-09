"""Step 1: pull Hydra's own chord list (with ticks) for every corpus chart x 16 drum settings.

Uses build-cpp/Release/hydra_replay.exe "score" with no activations; its JSON has one row per
chord with "tick" and "notes" (each note's color + cymbal). Output: chords.json in this folder.
"""
import json, subprocess, sys, os
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

REPO = Path(r"C:/Users/Patrick/Downloads/Hydra/hydra-test")
EXE = REPO / "build-cpp/Release/hydra_replay.exe"
OUT = Path(__file__).resolve().parent / "chords.json"
DIFFS = ["expert", "hard", "medium", "easy"]
SETTINGS = [(d, p, b) for d in DIFFS for p in (1, 0) for b in (1, 0)]


def charts():
    root = REPO / "testdata/input"
    return sorted(str(p) for p in root.rglob("*") if p.is_file() and p.suffix.lower() in (".mid", ".chart"))


def one(job):
    chart, (d, p, b) = job
    r = subprocess.run([str(EXE), "score", "--chart", chart, "--difficulty", d,
                        "--prodrums", str(p), "--bass2x", str(b)],
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    rec = {"chart": chart, "difficulty": d, "pro": p, "bass2x": b, "exit": r.returncode}
    if r.returncode != 0:
        rec["stderr"] = r.stderr.strip()[-400:]
        return rec
    try:
        j = json.loads(r.stdout)
    except Exception as e:
        rec["exit"] = -999
        rec["stderr"] = "bad json: %s" % e
        return rec
    rec["chartmode"] = j["chartmode"]
    rec["chords"] = [[c["tick"], [[n["color"], bool(n["cymbal"])] for n in c["notes"]]] for c in j["chords"]]
    return rec


def main():
    jobs = [(c, s) for c in charts() for s in SETTINGS]
    print("charts", len(jobs) // len(SETTINGS), "runs", len(jobs), flush=True)
    with ThreadPoolExecutor(max_workers=6) as ex:
        recs = list(ex.map(one, jobs))
    OUT.write_text(json.dumps(recs), encoding="utf-8")
    bad = [r for r in recs if r["exit"] != 0]
    print("nonzero exits", len(bad))
    from collections import Counter
    print(Counter((r["exit"], r.get("stderr", "")[:80]) for r in bad).most_common(20))


if __name__ == "__main__":
    main()
