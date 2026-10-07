import sqlite3, sys, time, statistics, random
db = sys.argv[1]
mode = 'Expert Pro Drums, 2x Bass'
c = sqlite3.connect('file:' + db + '?mode=ro', uri=True)
paths = {}
for md5, p in c.execute('select md5, path from charts'):
    paths.setdefault(md5, p)
# Bytes of structure + referenced nodes, per result in this mode.
q = '''select r.result_id, r.hyhash, length(r.structure),
       (select coalesce(sum(length(p.payload)),0) from path_refs pr join paths p
          on p.hyhash=pr.hyhash and p.chartmode=pr.chartmode and p.phash=pr.phash
        where pr.result_id=r.result_id),
       (select count(*) from path_refs pr where pr.result_id=r.result_id),
       (select length(tempomap) from songmeta s where s.hyhash=r.hyhash)
       from results r where r.chartmode=?'''
t0 = time.perf_counter()
res = c.execute(q, (mode,)).fetchall()
print(f'{len(res)} results in mode, query {time.perf_counter()-t0:.1f}s')
tot = sorted(((s or 0) + (n or 0) + (tm or 0)) for _, _, s, n, _, tm in res)
def pct(a, p):
    return a[min(len(a) - 1, int(p * len(a)))]
print(f'record bytes (structure+nodes+tempomap): median {statistics.median(tot)/1e3:.1f} KB p90 {pct(tot,.9)/1e3:.1f} KB p99 {pct(tot,.99)/1e3:.1f} KB max {tot[-1]/1e6:.2f} MB sum {sum(tot)/1e6:.0f} MB')
res.sort(key=lambda r: -((r[2] or 0) + (r[3] or 0)))
with open(sys.argv[2], 'w', encoding='utf-8') as f:
    for rid, h, s, n, cnt, tm in res[:20]:
        print(f'  {h} struct {s/1e3:7.1f} KB nodes {n/1e6:6.2f} MB ({cnt} refs) tempomap {(tm or 0)/1e3:6.1f} KB  {paths.get(h, "(not in charts)")}')
        if h in paths:
            f.write(paths[h] + '\n')
