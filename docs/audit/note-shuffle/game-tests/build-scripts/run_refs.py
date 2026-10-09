"""Child runner: run one scout's shuffle reference on a note list, print the result as JSON.

usage: py -I run_refs.py <flag|strings> <shuffle_ref.py path> <input.json>
input.json: {"instrument": 6|9, "notes": [[tick, mask], ...]} in game order.
Prints [[tick, mask], ...] in the same order. Hangs if the reference hangs; the
caller runs this with a timeout.
"""
import importlib.util, json, sys

kind, ref_path, inp = sys.argv[1], sys.argv[2], sys.argv[3]
spec = importlib.util.spec_from_file_location("ref", ref_path)
ref = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ref)
data = json.load(open(inp))
inst = data["instrument"]

if kind == "flag":
    notes = [{"tick": t, "lanes": m, "ext_sustain": False} for t, m in data["notes"]]
    groups = {}
    for n in notes:
        groups.setdefault(n["tick"], []).append(n)
    for n in notes:
        n["chord"] = groups[n["tick"]]
    ref.shuffle(notes, inst)
    out = [[n["tick"], n["lanes"]] for n in notes]
else:
    class N:
        def __init__(s, t, m):
            s.tick, s.mask, s.flags = t, m, 0
    notes = [N(t, m) for t, m in data["notes"]]
    groups = {}
    for n in notes:
        groups.setdefault(n.tick, []).append(n)
    ref.shuffle(notes, inst, lambda n: groups[n.tick])
    out = [[n.tick, n.mask] for n in notes]
print(json.dumps(out))
