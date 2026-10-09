"""Break down the order-reversal changes, and the pad-count spread, for the summary."""
import json, sys
from pathlib import Path
from collections import Counter, defaultdict

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.dont_write_bytecode = True
import classify as C  # noqa: E402

recs = json.loads((HERE / "chords.json").read_text(encoding="utf-8"))
pads_hist = Counter()
kinds = Counter()
seed_changed = 0
for r in recs:
    if r["exit"] != 0:
        continue
    for t, ns in r["chords"]:
        pads_hist[sum(1 for c, _ in ns if c != "Kick")] += 1
    inst = 9 if r["pro"] else 6
    a = C.run(r["chords"], inst)
    b = C.run(r["chords"], inst, reverse=True)
    if a["seed"] != b["seed"]:
        seed_changed += 1
    if a["out"] == b["out"]:
        kinds["identical"] += 1
        continue
    sa, sb = defaultdict(set), defaultdict(set)
    for (t, _), m in a["out"].items():
        sa[t].add(m)
    for (t, _), m in b["out"].items():
        sb[t].add(m)
    kinds["same pads per chord, different note->pad pairing" if sa == sb else "different pads on screen"] += 1
print("pad notes per chord (all runs):", sorted(pads_hist.items()))
print("runs whose seed changes when reversed:", seed_changed)
print(kinds)
