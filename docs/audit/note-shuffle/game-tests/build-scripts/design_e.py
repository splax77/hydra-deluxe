"""E: star cutoffs before vs after the shuffle. Original chart has no cymbals; the shuffle (Pro
Drums on) turns some notes into cymbals. Hydra scores the shuffled chart with no Star Power; the
star count is worked out against the base of the unshuffled chart and of the shuffled chart.

usage: py -I design_e.py <bpm> <max chords>
"""
import json, os, struct, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ns_lib import *

HERE = os.path.dirname(os.path.abspath(__file__))
EXE = REPO + "/build-cpp/Release/hydra_replay.exe"
TABLE = [0.1, 0.5, 1.0, 2.0, 2.8, 3.6, 4.4]  # memory ch-star-cutoffs (decoded table)


def f32(x):
    return struct.unpack("f", struct.pack("f", x))[0]


def stars(score, base):
    import math
    n = 0
    for m in TABLE:
        if score >= math.ceil(f32(f32(base) * f32(m))):
            n += 1
        else:
            break
    return n


def cutoff(base, i):
    import math
    return math.ceil(f32(f32(base) * f32(TABLE[i])))


# 8th notes, single pads only (no two-pad chords, so in-chord order can't matter);
# kick on the beat with the pad. First four notes are single-note chords.
PATTERN = [[R], [BT], [K, YT], [GT], [R], [K], [K, BT], [YT],
           [R], [GT], [K, BT], [YT], [K, R], [GT], [K, YT], [BT]]
OPEN = [[YT], [BT], [GT], [R]]


def original(nchords, res=480):
    cs = [list(c) for c in OPEN]
    while len(cs) < nchords:
        cs.append(list(PATTERN[(len(cs) - 4) % len(PATTERN)]))
    return [(1920 + (res // 2) * i, c) for i, c in enumerate(cs[:nchords])]


if __name__ == "__main__":
    bpm, nmax = int(sys.argv[1]), int(sys.argv[2])
    orig = original(nmax)
    sh = predict(orig, pro=True)
    assert sh != "HANG"
    shuffled = [(t, ms) for t, ms in sh]
    p = os.path.join(HERE, "e_shuffled.mid")
    with open(p, "wb") as f:
        f.write(mid_bytes("E shuffled", 480, shuffled, bpm))
    r = subprocess.run([EXE, "score", "--chart", p, "--prodrums", "1", "--bass2x", "0"],
                       capture_output=True, text=True, check=True)
    j = json.loads(r.stdout)
    print("top-level keys:", list(j))
    gems = cym = 0
    best = []
    for i, c in enumerate(j["chords"]):
        assert c["tick"] == shuffled[i][0]
        gems += len(c["notes"])
        cym += sum(n["cymbal"] for n in c["notes"])
        score = c["cum"]["total"]
        b_before, b_after = 50 * gems, 50 * gems + 15 * cym
        s_b, s_a = stars(score, b_before), stars(score, b_after)
        if s_b != s_a and i + 1 >= 20:
            k = s_b - 1  # the cutoff the before-base clears and the after-base misses
            margin = min(score - cutoff(b_before, k), cutoff(b_after, k) - score)
            best.append((margin, i + 1, gems, cym, score, b_before, b_after, s_b, s_a,
                         cutoff(b_before, k), cutoff(b_after, k)))
    best.sort(reverse=True)
    print("margin, chords, gems, cymbals, score, base_before, base_after, stars_before, stars_after, cut_before, cut_after")
    for row in best[:8]:
        print(row)
