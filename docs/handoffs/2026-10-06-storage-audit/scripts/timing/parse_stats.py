import sys, os, statistics
rows = []
for line in open(sys.argv[1], encoding='utf-8', errors='replace'):
    parts = line.rstrip('\n').split('\t')
    if len(parts) < 3:
        continue
    rows.append((parts[0], int(parts[1]) / 1000.0, len(parts) > 3))
ok = [r for r in rows if not r[2]]
ts = sorted(r[1] for r in ok)
def pct(p):
    return ts[min(len(ts) - 1, int(p * len(ts)))]
print(f'charts {len(rows)} ok {len(ok)} failed {len(rows)-len(ok)}')
print(f'parse ms: median {statistics.median(ts):.2f} p90 {pct(0.90):.2f} p99 {pct(0.99):.2f} max {ts[-1]:.1f} sum {sum(ts)/1000:.2f} s mean {statistics.mean(ts):.2f}')
for ext in ['.mid', '.chart', '.sng', '.srb']:
    e = sorted(r[1] for r in ok if r[0].lower().endswith(ext))
    if e:
        print(f'  {ext}: n {len(e)} median {statistics.median(e):.2f} p99 {e[min(len(e)-1,int(.99*len(e)))]:.2f} max {e[-1]:.1f}')
print('slowest 15:')
for p, t, f in sorted(ok, key=lambda r: -r[1])[:15]:
    sz = os.path.getsize(p) / 1e6 if os.path.exists(p) else -1
    print(f'  {t:8.1f} ms  {sz:7.1f} MB  {p}')
with open(sys.argv[2], 'w', encoding='utf-8') as f:
    for p, t, fl in sorted(ok, key=lambda r: -r[1])[:40]:
        f.write(p + '\n')
