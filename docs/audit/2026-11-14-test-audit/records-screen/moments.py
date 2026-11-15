"""Print hydra_uitest command lines that set the Preview's playhead to each
moment the engine record names, and screenshot the highway there.
Usage: py -I moments.py <schedule.json> <name> <out_dir_forward_slashes>
Moments per activation: its start (the activation chord), 1 ms before its
SP end, its SP end, the SP-ready moment, and each squeeze's end."""
import json, sys

sched, name, out = sys.argv[1], sys.argv[2], sys.argv[3]
d = json.load(open(sched))
ms_of = {c["tick"]: c["ms"] for c in d["chords"]}
moments = []
for a in d["activations"]:
    n = a["index"] + 1
    if a["act_tick"] in ms_of:
        moments.append((f"a{n}_start", ms_of[a["act_tick"]]))
    if a.get("sp_ready_ms") is not None:
        moments.append((f"a{n}_sp_ready", a["sp_ready_ms"]))
    for k, q in enumerate(a["squeezes"]):
        if q.get("end_ms") is not None:
            moments.append((f"a{n}_{q['kind'].lower()}{k+1}_end", q["end_ms"]))
    moments.append((f"a{n}_end_minus_1ms", a["deact_ms"] - 1.0))
    moments.append((f"a{n}_end", a["deact_ms"]))
moments.append(("song_start", 0.0))
for label, ms in moments:
    print(f"type **/##scrub | {ms / 1000.0:.6f}")
    print(f"screenshot {out}/{name}_{label}_{ms:.1f}ms.png | **/##highway_image")
