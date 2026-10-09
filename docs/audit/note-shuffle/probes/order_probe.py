"""Does the 22-tick video check tell within-chord note orders apart?

Runs the scout-strings reference on the video chart with each chord's notes in
lane order (as the original check did) and in reverse lane order, then with
ticks rescaled (x0.4 = 480 -> 192 res) to see whether rescaling also changes
the result.

usage: py -I order_probe.py <scout-strings dir>
"""
import importlib.util, sys

spec = importlib.util.spec_from_file_location("ref", sys.argv[1] + "/shuffle_ref.py")
B = importlib.util.module_from_spec(spec)
spec.loader.exec_module(B)

K, R, YT, BT, GT, YC, BC, GC = 1, 2, 4, 8, 16, 32, 64, 128
chart = [(2640, [K]), (2700, [K]), (2760, [K]), (2820, [R]), (2880, [YT]), (2940, [BT])]
chart += [(3000 + 60 * i, [GT]) for i in range(12)]
chart += [(3720, [BT]), (3780, [YT]), (4080, [K, R, GC]), (4200, [K, YT, GT])]
seen = {2640: {K}, 2700: {K}, 2760: {K}, 2820: {BT}, 2880: {YT}, 2940: {BC}, 3720: {GT}, 3780: {R},
        4080: {K, BT, GC}, 4200: {K, R, YT}}
for i in range(12):
    seen[3000 + 60 * i] = {GT} if i % 2 == 0 else {YC}


class N:
    def __init__(s, tick, mask):
        s.tick, s.mask, s.flags = tick, mask, 0


def run(order, scale_num=1, scale_den=1):
    notes, chords = [], {}
    for t, lanes in chart:
        ls = lanes if order == "lane" else (
            list(reversed(lanes)) if order == "reverse" else [lanes[0]] + list(reversed(lanes[1:])))
        grp = [N(t * scale_num // scale_den, m) for m in ls]
        chords[grp[0].tick] = grp
        notes += grp
    B.shuffle(notes, 9, lambda n: chords[n.tick])
    got = {}
    for n in notes:
        got.setdefault(n.tick * scale_den // scale_num, set()).add(n.mask)
    return sum(got.get(t) == seen[t] for t in seen)


for order in ("lane", "reverse", "kick-first-pads-reversed"):
    print(f"{order:28s} raw ticks: {run(order)}/{len(seen)}   ticks x0.4: {run(order, 2, 5)}/{len(seen)}")
