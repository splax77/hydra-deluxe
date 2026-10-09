"""Count ticks where a 2x kick and a normal kick share a tick, per drum difficulty, in a chart folder tree."""
import sys, os, re, mido
from collections import defaultdict
root = sys.argv[1]
DIFF = {'Expert': (96, 95), 'Hard': (84, 83), 'Medium': (72, 71), 'Easy': (60, 59)}
res = {}
for dp, _, fs in os.walk(root):
    for f in fs:
        p = os.path.join(dp, f)
        hits = defaultdict(int)
        if f.lower().endswith('.mid'):
            m = mido.MidiFile(p)
            for tr in m.tracks:
                if tr.name.strip() != 'PART DRUMS': continue
                t = 0; on = defaultdict(set)
                for msg in tr:
                    t += msg.time
                    if msg.type == 'note_on' and msg.velocity > 0: on[t].add(msg.note)
                for tk, ns in on.items():
                    for d, (n, x) in DIFF.items():
                        if n in ns and x in ns: hits[d] += 1
        elif f.lower().endswith('.chart'):
            sec = None; on = defaultdict(set)
            for line in open(p, encoding='utf-8-sig', errors='replace'):
                line = line.strip()
                if line.startswith('['): sec = line[1:-1]; continue
                mm = re.match(r'(\d+)\s*=\s*N\s+(\d+)', line)
                if mm and sec and sec.endswith('Drums'): on[(sec, int(mm[1]))].add(int(mm[2]))
            for (sec, tk), ns in on.items():
                if 0 in ns and 32 in ns: hits[sec.replace('Drums', '')] += 1
        if hits: res[os.path.relpath(p, root)] = dict(hits)
for k, v in sorted(res.items()): print(k, v)
print('charts with same-tick 2x+normal kick:', len(res))
