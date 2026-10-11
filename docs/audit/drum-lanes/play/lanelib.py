import sys, os, re, struct, csv, bisect, hashlib, statistics, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")

ROOT = r"C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input"
OUT = '.'
DIFFS = ["Easy", "Medium", "Hard", "Expert"]
BASE = {"Expert": 96, "Hard": 84, "Medium": 72, "Easy": 60}
# colours 1..4 = Red, Yellow, Blue, Green; rank: R 0, tom Y/B/G 1..3, cym Y/B/G 4..6
NAME = {1: 'R', 2: 'Y', 3: 'B', 4: 'G'}


def rank(c, cym):
    if c == 1:
        return 0
    return (3 + (c - 1)) if cym else (c - 1)


def vlq(d, i):
    v = 0
    while True:
        b = d[i]; i += 1
        v = (v << 7) | (b & 0x7F)
        if not b & 0x80:
            return v, i


def mid_tracks(path):
    d = open(path, 'rb').read()
    res = struct.unpack('>H', d[12:14])[0]
    i = 14
    out = []
    while i + 8 <= len(d):
        ln = struct.unpack('>I', d[i + 4:i + 8])[0]
        out.append(d[i + 8:i + 8 + ln])
        i += 8 + ln
    return res, out


def mid_events(t):
    i, tick, run, name = 0, 0, 0, ''
    notes, tempos = [], []
    while i < len(t):
        dt, i = vlq(t, i); tick += dt
        s = t[i]
        if s == 0xFF:
            typ = t[i + 1]; ln, j = vlq(t, i + 2)
            if typ == 3 and not name: name = t[j:j + ln].decode('latin1')
            if typ == 0x51 and ln == 3:
                tempos.append((tick, int.from_bytes(t[j:j + 3], 'big') / 1000000.0))  # sec per beat
            i = j + ln; continue
        if s in (0xF0, 0xF7):
            ln, j = vlq(t, i + 1); i = j + ln; continue
        if s & 0x80: run = s; i += 1
        hi = run & 0xF0
        if hi in (0xC0, 0xD0): i += 1; continue
        a, b = t[i], t[i + 1]; i += 2
        if hi in (0x80, 0x90): notes.append((tick, hi == 0x90 and b > 0, a, b))
    return name, notes, tempos


def secs(tempos, res, a, b):
    # tempos: sorted [(tick, sec_per_beat)]; time between ticks a and b
    if not tempos or tempos[0][0] > 0:
        tempos = [(0, 0.5)] + tempos
    total = 0.0
    for k, (tk, spb) in enumerate(tempos):
        nxt = tempos[k + 1][0] if k + 1 < len(tempos) else 1 << 60
        lo, hi = max(a, tk), min(b, nxt)
        if hi > lo:
            total += (hi - lo) / res * spb
    return total


def lowest_diff(vel):
    if 21 <= vel <= 30: return "Easy"
    if 31 <= vel <= 40: return "Medium"
    if 41 <= vel <= 50: return "Hard"
    return "Expert"


def shows(lowest, diff):
    return DIFFS.index(diff) >= DIFFS.index(lowest)


def load_mid(path):
    """-> res, tempos, {diff: dict(chords, kicks, lanes, sp)} ; chords: tick -> {colour: cymbal flag}"""
    res, trs = mid_tracks(path)
    tempos = []
    drums = None
    for t in trs:
        name, notes, tp = mid_events(t)
        tempos += tp
        if name == 'PART DRUMS' and drums is None:
            drums = notes
    tempos.sort()
    if drums is None:
        return res, tempos, None
    ev = sorted(drums, key=lambda e: (e[0], not e[1]))
    by_tick = {}
    for e in ev:
        by_tick.setdefault(e[0], []).append(e)
    per = {d: dict(chords={}, kicks=set(), lanes=[], sp=[]) for d in DIFFS}
    tom = {2: False, 3: False, 4: False}
    open_lane = {d: {126: None, 127: None} for d in DIFFS}
    sp_open = None
    sp = []
    for tk in sorted(by_tick):
        es = by_tick[tk]
        for _, on, pi, v in es:
            if pi in (110, 111, 112) and not on: tom[pi - 108] = False
        for _, on, pi, v in es:
            if pi in (110, 111, 112) and on: tom[pi - 108] = True
        for _, on, pi, v in es:
            if pi == 116:
                if on: sp_open = tk
                elif sp_open is not None:
                    sp.append((sp_open, tk)); sp_open = None
            if pi in (126, 127):
                if on:
                    low = lowest_diff(v)
                    for d in DIFFS:
                        if shows(low, d): open_lane[d][pi] = tk
                        else: pass
                else:
                    for d in DIFFS:
                        s = open_lane[d][pi]
                        if s is not None:
                            per[d]['lanes'].append((s, tk, pi == 127))
                            open_lane[d][pi] = None
            if on:
                for d in DIFFS:
                    b = BASE[d]
                    if pi == b or pi == b - 1:
                        per[d]['kicks'].add(tk)
                    elif b < pi <= b + 4:
                        c = pi - b
                        per[d]['chords'].setdefault(tk, {})[c] = (c != 1 and not tom[c])
    for d in DIFFS:
        per[d]['sp'] = sp
    return res, tempos, per


def load_chart(path):
    txt = open(path, encoding='utf-8-sig', errors='replace').read()
    sec = None
    secs_ = {}
    for line in txt.splitlines():
        s = line.strip()
        if s.startswith('[') and s.endswith(']'):
            sec = s[1:-1]; secs_.setdefault(sec, []); continue
        if s in ('{', '}') or sec is None: continue
        secs_[sec].append(s)
    res = 192
    for s in secs_.get('Song', []):
        m = re.match(r'Resolution\s*=\s*(\d+)', s)
        if m: res = int(m.group(1))
    tempos = []
    for s in secs_.get('SyncTrack', []):
        m = re.match(r'(\d+)\s*=\s*B\s+(\d+)', s)
        if m and int(m.group(2)) > 0:
            tempos.append((int(m.group(1)), 60.0 / (int(m.group(2)) / 1000.0)))
    tempos.sort()
    per = {}
    for d in DIFFS:
        name = d + 'Drums'
        if name not in secs_: continue
        chords, kicks, lanes, sp = {}, set(), [], []
        notes, cym = {}, {}
        for s in secs_[name]:
            m = re.match(r'(\d+)\s*=\s*N\s+(\d+)\s*(\d*)', s)
            if m:
                tk, c = int(m.group(1)), int(m.group(2))
                if c in (0, 32): kicks.add(tk)
                elif 1 <= c <= 4: notes.setdefault(tk, set()).add(c)
                elif 66 <= c <= 68: cym.setdefault(tk, set()).add(c - 64)
                continue
            m = re.match(r'(\d+)\s*=\s*S\s+(\d+)\s+(\d+)', s)
            if m:
                tk, k, ln = int(m.group(1)), int(m.group(2)), int(m.group(3))
                if k in (65, 66): lanes.append((tk, tk + ln, k == 66))
                elif k == 2: sp.append((tk, tk + ln))
        for tk, cs in notes.items():
            chords[tk] = {c: (c in cym.get(tk, ())) for c in cs}
        per[d] = dict(chords=chords, kicks=kicks, lanes=lanes, sp=sp)
    return res, tempos, per


def lit_set(chords_sorted, ticks, lane, pro=True):
    """Hydra's light_drum_lanes rule. chords_sorted: list of (tick, {c: cym}). returns (set of colours, dropped)"""
    s, e, dbl = lane
    i = bisect.bisect_left(ticks, s - 1)
    def pads(ch):
        items = [(rank(c, cym and pro), c) for c, cym in ch.items()]
        return items
    first = None
    while i < len(ticks) and ticks[i] < e:
        p = pads(chords_sorted[i][1])
        if p:
            first = p; break
        i += 1
    if first is None:
        return None
    low = min(first)[1]
    out = {low}
    if dbl:
        second = None
        if len(first) >= 2:
            second = max(first)[1]
        else:
            for j in range(i + 1, len(ticks)):
                p = pads(chords_sorted[j][1])
                if p:
                    second = max(p)[1]; break
        if second: out.add(second)
    return out


def main():
    files = []
    for dp, _, fs in os.walk(ROOT):
        for f in sorted(fs):
            if f.endswith('.mid') or f.endswith('.chart'):
                files.append(os.path.join(dp, f))
    files.sort()
    rows = []
    nofile_drums = []
    per_file = collections.defaultdict(lambda: collections.defaultdict(int))
    hashes = {}
    for p in files:
        fmt = 'mid' if p.endswith('.mid') else 'chart'
        h = hashlib.md5(open(p, 'rb').read()).hexdigest()
        dup = hashes.get(h)
        hashes.setdefault(h, p)
        try:
            res, tempos, per = load_mid(p) if fmt == 'mid' else load_chart(p)
        except Exception as ex:
            print("FAIL", p, ex); continue
        if per is None:
            nofile_drums.append(os.path.relpath(p, ROOT)); continue
        rel = os.path.relpath(p, ROOT).replace('\\', '/')
        for d, data in per.items():
            cs = sorted(data['chords'].items())
            ticks = [t for t, _ in cs]
            kicks = sorted(data['kicks'])
            for (s, e, dbl) in sorted(data['lanes']):
                lit = lit_set(cs, ticks, (s, e, dbl), True)
                lit_np = lit_set(cs, ticks, (s, e, dbl), False)
                lo = bisect.bisect_left(ticks, s); hi = bisect.bisect_right(ticks, e)
                inside = cs[lo:hi]
                nk = sum(len(c) for _, c in inside)
                chords = sum(1 for _, c in inside if len(c) > 1)
                kk = bisect.bisect_right(kicks, e) - bisect.bisect_left(kicks, s)
                nlit = nun = None
                if lit is not None:
                    nlit = sum(1 for _, c in inside for col in c if col in lit)
                    nun = nk - nlit
                spb = 0
                for (a, b) in data['sp']:
                    for t in (a, b):
                        if s <= t <= e: spb += 1
                sp_ov = any(a < e and b > s for a, b in data['sp'])
                rows.append(dict(file=rel, fmt=fmt, dup_of=(os.path.relpath(dup, ROOT).replace('\\', '/') if dup else ''),
                                 diff=d, kind='double' if dbl else 'single', start=s, end=e,
                                 len_ticks=e - s, len_sec=round(secs(tempos, res, s, e), 3),
                                 dropped=int(lit is None),
                                 lit=''.join(sorted(NAME[c] for c in lit)) if lit else '',
                                 lit_nonpro=''.join(sorted(NAME[c] for c in lit_np)) if lit_np else '',
                                 nonkick_notes=nk, chords=chords, kick_notes=kk,
                                 notes_on_lit=nlit if nlit is not None else '', notes_on_unlit=nun if nun is not None else '',
                                 sp_boundaries=spb, sp_overlap=int(sp_ov)))
    cols = list(rows[0].keys())
    with open(os.path.join(OUT, 'lanes.csv'), 'w', newline='', encoding='utf-8') as f:
        w = csv.DictWriter(f, cols); w.writeheader(); w.writerows(rows)
    print("files", len(files), "no PART DRUMS:", nofile_drums)
    print("rows", len(rows))

