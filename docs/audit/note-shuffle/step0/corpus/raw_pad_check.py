"""Independent raw count of Expert drum pad notes per tick, to check Hydra's max of 2 pads per chord.
.mid: PART DRUMS note-ons 97-101 (red..green, 101 = 5-lane orange). .chart: [ExpertDrums] N 1-5.
Prints every tick with 3 or more pad notes, and Hydra's pad count at that tick (Expert, Pro on, 2x on).
"""
import json, re, struct, sys
from pathlib import Path
from collections import Counter, defaultdict

HERE = Path(__file__).resolve().parent


def vlq(b, j):
    v = 0
    while True:
        x = b[j]; j += 1; v = (v << 7) | (x & 127)
        if x < 128:
            return v, j


def mid_ticks(path):
    b = Path(path).read_bytes(); i = 14; per = defaultdict(set)
    while i < len(b) and b[i:i + 4] == b"MTrk":
        ln = struct.unpack(">I", b[i + 4:i + 8])[0]; tr = b[i + 8:i + 8 + ln]; i += 8 + ln
        name_ok = False; j = 0; rs = 0; t = 0; evs = []
        while j < len(tr):
            d, j = vlq(tr, j); t += d; st = tr[j]
            if st == 0xFF:
                typ = tr[j + 1]; ln2, k = vlq(tr, j + 2)
                if typ == 3 and tr[k:k + ln2] == b"PART DRUMS":
                    name_ok = True
                j = k + ln2; continue
            if st in (0xF0, 0xF7):
                ln2, k = vlq(tr, j + 1); j = k + ln2; continue
            if st & 0x80:
                rs = st; j += 1
            hi = rs & 0xF0
            if hi in (0xC0, 0xD0):
                j += 1; continue
            p, vel = tr[j], tr[j + 1]; j += 2
            if hi == 0x90 and vel > 0 and 97 <= p <= 101:
                evs.append((t, p))
        if name_ok:
            for t, p in evs:
                per[t].add(p)
    return per


def chart_ticks(path):
    txt = Path(path).read_text(encoding="utf-8-sig", errors="replace")
    m = re.search(r"\[ExpertDrums\]\s*\{(.*?)\}", txt, re.S)
    per = defaultdict(set)
    if m:
        for t, n in re.findall(r"(\d+)\s*=\s*N\s+(\d+)\s+\d+", m.group(1)):
            if 1 <= int(n) <= 5:
                per[int(t)].add(int(n))
    return per


recs = json.loads((HERE / "chords.json").read_text(encoding="utf-8"))
hydra = {r["chart"]: {t: sum(1 for c, _ in ns if c != "Kick") for t, ns in r["chords"]}
         for r in recs if r["exit"] == 0 and r["difficulty"] == "expert" and r["pro"] == 1 and r["bass2x"] == 1}
hist = Counter(); hits = []
for chart, hy in hydra.items():
    per = mid_ticks(chart) if chart.endswith(".mid") else chart_ticks(chart)
    for t, s in per.items():
        hist[len(s)] += 1
        if len(s) >= 3:
            hits.append((chart.split("input\\")[-1], t, sorted(s), hy.get(t)))
print("raw expert pad notes per tick:", sorted(hist.items()))
for h in hits[:30]:
    print(h)
print("ticks with 3+ raw pads:", len(hits))
