import json, sys, statistics, random, os
A = os.path.dirname(os.path.abspath(__file__))
res = json.load(open(sys.argv[1], encoding='utf-8'))
by = {r['path']: r for r in res}
# The random part: re-draw the same sample the driver drew.
ok_paths = [line.split('\t')[0] for line in open(os.path.join(A, 'parse.tsv'), encoding='utf-8')
            if len(line.rstrip('\n').split('\t')) == 3]
random.seed(20261006)
rand = [by[p] for p in random.sample(ok_paths, int(sys.argv[2])) if p in by]
def pct(a, p):
    a = sorted(a)
    return a[min(len(a) - 1, int(p * len(a)))]
for r in res:
    r['search'] = max(0.0, r['analyze'] - r['graph'])
    r['total'] = r['parse'] + r['analyze'] + r['prepare']
print(f'random sample n={len(rand)} (ms; bench prints ms resolution, analyze is mean of 5 reps)')
for k in ['parse', 'graph', 'analyze', 'search', 'prepare', 'total']:
    v = [r[k] for r in rand]
    print(f'  {k:8s} median {statistics.median(v):6.2f} p90 {pct(v,.9):6.2f} p99 {pct(v,.99):6.2f} max {max(v):7.1f} mean {statistics.mean(v):6.2f}')
print('slowest 12 of all measured, by parse+analyze+prepare:')
for r in sorted(res, key=lambda r: -r['total'])[:12]:
    name = os.path.basename(os.path.dirname(r['path'])) if not r['path'].lower().endswith(('.sng', '.srb')) else os.path.basename(r['path'])
    print(f"  total {r['total']:6.1f} | parse {r['parse']:5.0f} graph {r['graph']:4.0f} analyze {r['analyze']:5.1f} prepare {r['prepare']:3.0f} | {name[:80]}")
