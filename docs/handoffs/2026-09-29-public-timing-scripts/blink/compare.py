import json, os
import sys
# The folder holding the c<i>-<mode>.json files (default: results/ beside this script).
here = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), 'results')
for i in range(8):
    runs = {}
    for m in ('orig', 'share', 'fix', 'both'):
        p = os.path.join(here, f'c{i}-{m}.json')
        runs[m] = json.load(open(p)) if os.path.exists(p) else None
    o = runs['orig']
    name = os.path.basename(os.path.dirname(o['chart'])) if o else f'c{i}'
    line = [f"{name[:45]:45}"]
    for m, r in runs.items():
        if not r or not r.get('finished'):
            line.append(f"{m}: not finished")
            continue
        same = 'SAME' if o and o.get('finished') and r['result'] == o['result'] else 'DIFF'
        line.append(f"{m}: {r['search_s']:7.2f}s {r['paths']:4d} paths {same if m != 'orig' else 'ref '}")
    print(' | '.join(line))
