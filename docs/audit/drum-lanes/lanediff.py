# For every lane in a folder of charts (Expert, pro drums), which colour
# columns Clone Hero's rule lights and which Onyx's rule lights.
# CH bit order: R=0x02, Ytom 0x04, Btom 0x08, Gtom 0x10, Ycym 0x20, Bcym 0x40, Gcym 0x80.
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from midlanes import tracks, events

COL = {0x02: 'R', 0x04: 'Y', 0x08: 'B', 0x10: 'G', 0x20: 'Y', 0x40: 'B', 0x80: 'G'}

def bit(colour, cym):  # colour 1..4 = R,Y,B,G
    if colour == 1: return 0x02
    return (0x20 << (colour - 2)) if cym else (0x04 << (colour - 2))

def rules(chords, start, end, double):
    # chords: sorted list of (tick, mask) of non-kick pads
    i = next((k for k, (t, m) in enumerate(chords) if t >= start - 1), None)
    if i is None or chords[i][0] >= end:
        return None, None
    t0, m0 = chords[i]
    low = m0 & -m0
    if double:
        if bin(m0).count('1') > 1:
            second = 1 << (m0.bit_length() - 1)
        elif i + 1 < len(chords):
            m1 = chords[i + 1][1]
            second = 1 << (m1.bit_length() - 1)
        else:
            second = 0
        ch = {COL[low]} | ({COL[second]} if second else set())
        if i + 1 < len(chords):
            on = {COL[b] for b in COL if (m0 | chords[i + 1][1]) & b}
        else:
            on = set()  # Onyx: fewer than two groups, no lane
    else:
        ch = {COL[low]}
        on = {COL[b] for b in COL if m0 & b}
    return ''.join(sorted(ch)), ''.join(sorted(on))

def mid_lanes(p):
    out = []
    for res, t in tracks(p):
        name, ev = events(t)
        if name != 'PART DRUMS':
            continue
        ev.sort(key=lambda e: (e[0], not e[1]))
        tom = {2: False, 3: False, 4: False}
        chords = {}
        lanes = []
        open_ = {}
        # tom markers apply to notes on their own tick, so take marker on/offs first per tick
        by_tick = {}
        for e in ev:
            by_tick.setdefault(e[0], []).append(e)
        for tk in sorted(by_tick):
            es = by_tick[tk]
            for _, on, pi, v in es:
                if pi in (110, 111, 112) and not on:
                    tom[pi - 108] = False
            for _, on, pi, v in es:
                if pi in (110, 111, 112) and on:
                    tom[pi - 108] = True
            for _, on, pi, v in es:
                if on and 97 <= pi <= 100:
                    c = pi - 96
                    chords[tk] = chords.get(tk, 0) | bit(c, c != 1 and not tom.get(c, False))
                if pi in (126, 127):
                    if on:
                        open_[pi] = (tk, v)
                    elif pi in open_:
                        s, v0 = open_.pop(pi)
                        lanes.append((s, tk, pi == 127, v0))
        out.append((sorted(chords.items()), lanes))
    return out

def chart_lanes(p):
    txt = open(p, encoding='utf-8-sig', errors='replace').read()
    m = re.search(r'\[ExpertDrums\]\s*\{(.*?)\}', txt, re.S)
    if not m:
        return []
    notes, cyms, lanes = {}, {}, []
    for a, b in re.findall(r'(\d+)\s*=\s*N\s+(\d+)\s', m.group(1)):
        a, b = int(a), int(b)
        if 1 <= b <= 4: notes.setdefault(a, set()).add(b)
        if 66 <= b <= 68: cyms.setdefault(a, set()).add(b - 64)
    for a, k, ln in re.findall(r'(\d+)\s*=\s*S\s+(6[56])\s+(\d+)', m.group(1)):
        lanes.append((int(a), int(a) + int(ln), k == '66', None))
    chords = sorted((t, sum(bit(c, c in cyms.get(t, ())) for c in cs)) for t, cs in notes.items())
    return [(chords, lanes)]

root = sys.argv[1]
total = differ = 0
for dp, _, fs in os.walk(root):
    for f in fs:
        p = os.path.join(dp, f)
        try:
            got = mid_lanes(p) if f.endswith('.mid') else chart_lanes(p) if f.endswith('.chart') else []
        except Exception:
            continue
        for chords, lanes in got:
            for s, e, dbl, vel in lanes:
                ch, on = rules(chords, s, e, dbl)
                total += 1
                if ch != on:
                    differ += 1
                    print(f"{'double' if dbl else 'single'} tick {s} vel {vel}: CH {ch} vs Onyx {on}  {os.path.relpath(p, root)}")
print('lanes', total, 'differ', differ)
