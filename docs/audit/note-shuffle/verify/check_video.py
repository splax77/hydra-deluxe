"""Run both scouts' Note Shuffle references on the video chart and compare with the frames.

usage: py -I check_video.py <scout-strings dir> <scout-flag dir>
"""
import importlib.util, sys

def load(path, name):
    spec = importlib.util.spec_from_file_location(name, path)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m

B = load(sys.argv[1] + "/shuffle_ref.py", "refB")
A = load(sys.argv[2] + "/shuffle_ref.py", "refA")

K, R, YT, BT, GT, YC, BC, GC = 1, 2, 4, 8, 16, 32, 64, 128
NAME = {K: "kick", R: "red", YT: "Y tom", BT: "B tom", GT: "G tom", YC: "Y cym", BC: "B cym", GC: "G cym", 0: "NONE"}

# original chart (from the video scout's MIDI dump), chords in lane order
chart = [(2640, [K]), (2700, [K]), (2760, [K]), (2820, [R]), (2880, [YT]), (2940, [BT])]
chart += [(3000 + 60 * i, [GT]) for i in range(12)]
chart += [(3720, [BT]), (3780, [YT]), (4080, [K, R, GC]), (4200, [K, YT, GT])]

# shown in video (chords as sets, no within-chord pairing)
seen = {2640: {K}, 2700: {K}, 2760: {K}, 2820: {BT}, 2880: {YT}, 2940: {BC}, 3720: {GT}, 3780: {R},
        4080: {K, BT, GC}, 4200: {K, R, YT}}
for i in range(12):
    seen[3000 + 60 * i] = {GT} if i % 2 == 0 else {YC}

def run_B():
    class N:
        def __init__(s, tick, mask): s.tick, s.mask, s.flags = tick, mask, 0
    notes, chords = [], {}
    for t, lanes in chart:
        grp = [N(t, m) for m in lanes]
        chords[t] = grp
        notes += grp
    B.shuffle(notes, 9, lambda n: chords[n.tick])
    return [(n.tick, n.mask) for n in notes]

def run_A():
    notes = []
    for t, lanes in chart:
        grp = [{"tick": t, "lanes": m, "ext_sustain": False} for m in lanes]
        for g in grp: g["chord"] = grp
        notes += grp
    A.shuffle(notes, 9)
    return [(n["tick"], n["lanes"]) for n in notes]

for label, res in (("scout B", run_B()), ("scout A", run_A())):
    got = {}
    for t, m in res: got.setdefault(t, set()).add(m)
    ok = sum(got[t] == seen[t] for t in seen)
    print(f"== {label}: {ok}/{len(seen)} ticks match the video")
    for t, _ in chart:
        mark = "ok " if got[t] == seen[t] else "BAD"
        print(f"  {mark} {t}: predicted {sorted(NAME[m] for m in got[t])}  video {sorted(NAME[m] for m in seen[t])}")
