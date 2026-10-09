"""Search for chart layouts whose predictions differ between hypotheses. Prints candidates."""
import sys, random
from ns_lib import *

which = sys.argv[1]


def pad_rows(res):
    return [(t, s) for t, s in res if s - {K}]


def distinct(preds, first=4, need=2):
    """every pair of hypotheses differs on >= need of the first `first` pad chords."""
    keys = list(preds)
    rows = {k: pad_rows(sets(preds[k])) for k in keys}
    for i in range(len(keys)):
        for j in range(i + 1, len(keys)):
            a, b = rows[keys[i]][:first], rows[keys[j]][:first]
            if sum(x != y for x, y in zip(a, b)) < need:
                return False
    return True


def clean(res):
    return res != "HANG" and all(0 not in ms for _, ms in res)


if which == "video":  # sanity: the runner reproduces the video
    chart = [(2640, [K]), (2700, [K]), (2760, [K]), (2820, [R]), (2880, [YT]), (2940, [BT])]
    chart += [(3000 + 60 * i, [GT]) for i in range(12)]
    chart += [(3720, [BT]), (3780, [YT]), (4080, [K, R, GC]), (4200, [K, YT, GT])]
    for t, ms in predict(chart):
        print(t, pads_text(ms))

if which == "A":
    # 8th-note groove at 192 res: one measure of hi-hat groove, one of toms and crash.
    groove = [[K, YC], [YC], [R], [YC], [K, YC], [K], [R], [YC],
              [K, GC], [BT], [R], [GT], [K, YT], [BT], [R, ], [K, GC]]
    for k in range(0, 40):
        start = 768 + 96 * k
        chords = [(start + 96 * i, list(c)) for i, c in enumerate(groove)]
        ok = True
        preds_on, preds_off = {}, {}
        for res_name, num, den in (("192", 1, 1), ("480", 5, 2), ("960", 5, 1)):
            preds_on[res_name] = predict(chords, num=num, den=den, pro=True)
            preds_off[res_name] = predict(chords, num=num, den=den, pro=False)
        if not all(clean(p) for p in list(preds_on.values()) + list(preds_off.values())):
            continue
        d_on = distinct(preds_on, 4, 2)
        d_off = distinct(preds_off, 4, 1)
        print(k, start, "on-distinct", d_on, "off-distinct", d_off)
        if d_on and d_off:
            for name, p in preds_on.items():
                print("  ON ", name, [pads_text(ms) for _, ms in p[:6]])
            for name, p in preds_off.items():
                print("  OFF", name, [pads_text(ms) for _, ms in p[:6]])
            break

if which == "B":
    rnd = random.Random(int(sys.argv[2]) if len(sys.argv) > 2 else 1)
    HOWS = ("file", "lane", "bit", "reverse")
    # fixed opening shapes: 3-note chord, then 3-note chord (seed cut falls inside it),
    # both mixing a cymbal and a tom so lane order and lane-bit order differ.
    pool = [[K, R, YC], [K, YC, BT], [R, YC, BT], [YC, BT, GC], [K, R, BT], [R, YT, GC],
            [K, YC, GT], [R, BC, GT], [YT, BC], [R, GC], [K, YC], [R, BT, GC], [YC, GT]]
    for trial in range(400):
        chords = []
        t = 1920
        for i in range(8):
            c = list(rnd.choice(pool if i >= 2 else pool[:8]))
            # a file order that is none of the hypotheses' orders
            for _ in range(20):
                rnd.shuffle(c)
                if all(c != order(c, h) for h in ("lane", "bit", "reverse")):
                    break
            chords.append((t, c))
            t += 240
        if len(chords[0][1]) != 3 or len(chords[1][1]) != 3:
            continue
        on = {h: predict(chords, how=h, pro=True) for h in HOWS}
        off = {h: predict(chords, how=h, pro=False) for h in HOWS}
        if not all(clean(p) for p in list(on.values()) + list(off.values())):
            continue
        if distinct(on, 3, 2) and distinct({h: off[h] for h in ("file", "lane", "reverse")}, 4, 1):
            print("trial", trial, "chords", [(t, [NAME[m] for m in c]) for t, c in chords])
            print("CHORDS", chords)
            for h, p in on.items():
                print("  ON ", h, [pads_text(ms) for _, ms in p[:5]])
            for h, p in off.items():
                print("  OFF", h, [pads_text(ms) for _, ms in p[:5]])
            break
