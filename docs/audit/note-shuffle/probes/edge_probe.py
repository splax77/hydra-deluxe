"""How often do the decode's odd outcomes happen on synthetic pro-drums charts?

Counts, per chart: a hang (every lane blocked), a note given no lane (0), and two
notes of one chord given the same pad colour (Hydra's Chord can hold one per colour).
Charts are random 16th-note grooves at 480 res: kick on some steps, 1-3 pads per step,
drawn from realistic shapes (snare+hat, hat, tom fills, crash+kick, 4-pad hits).

usage: py -I edge_probe.py <dir holding shuffle_ref_capped.py>
"""
import importlib.util, random, sys

spec = importlib.util.spec_from_file_location("ref", sys.argv[1] + "/shuffle_ref_capped.py")
B = importlib.util.module_from_spec(spec)
spec.loader.exec_module(B)

K, R, YT, BT, GT, YC, BC, GC = 1, 2, 4, 8, 16, 32, 64, 128
COLOUR = {R: "R", YT: "Y", YC: "Y", BT: "B", BC: "B", GT: "G", GC: "G"}
SHAPES = [[R, YC], [YC], [R], [YT], [BT], [GT], [R, GC], [YC, BT], [R, YC], [YC], [R, YC]]
FOUR = [R, YC, BT, GC]


class N:
    def __init__(s, tick, mask):
        s.tick, s.mask, s.flags = tick, mask, 0


def chart(rng, four_rate):
    out = []
    for step in range(rng.randint(40, 400)):
        t = 480 + 120 * step
        lanes = [K] if rng.random() < 0.3 else []
        lanes += FOUR if rng.random() < four_rate else rng.choice(SHAPES)
        out.append((t, lanes))
    return out


def run(ch):
    notes, chords = [], {}
    for t, lanes in ch:
        grp = [N(t, m) for m in lanes]
        chords[t] = grp
        notes += grp
    try:
        B.shuffle(notes, 9, lambda n: chords[n.tick])
    except RuntimeError:
        return "hang"
    for t, grp in chords.items():
        pads = [n.mask for n in grp if n.mask != K]
        if any(m == 0 for m in pads):
            return "zero"
        cols = [COLOUR[m] for m in pads]
        if len(cols) != len(set(cols)):
            return "collide"
    return "ok"


rng = random.Random(1)
for four_rate in (0.0, 0.02):
    tally = {}
    for _ in range(2000):
        r = run(chart(rng, four_rate))
        tally[r] = tally.get(r, 0) + 1
    print(f"4-pad chord rate {four_rate}: {tally}")

# seed 0: four notes on tick 0, then a normal groove
ch = [(0, [K, R, YC, BT])] + chart(rng, 0.0)
print("first 4 notes on tick 0:", run(ch))
