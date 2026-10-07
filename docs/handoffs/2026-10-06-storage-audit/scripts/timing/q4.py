import sqlite3, sys, os, hashlib, random, collections
db = sys.argv[1]
c = sqlite3.connect('file:' + db + '?mode=ro', uri=True)
rows = c.execute('select md5, path from charts').fetchall()
print('chart rows', len(rows), 'distinct md5', len({r[0] for r in rows}))
missing = [r for r in rows if not os.path.exists(r[1])]
print('chart rows whose file is missing:', len(missing))
for r in missing[:10]:
    print('   ', r[1])
exts = collections.Counter(os.path.splitext(r[1])[1].lower() for r in rows)
print('extensions', dict(exts))

chart_md5 = {r[0] for r in rows}
for t, col in [('results', 'hyhash'), ('songmeta', 'hyhash'), ('paths', 'hyhash'), ('dynamics', 'md5')]:
    allh = [r[0] for r in c.execute(f'select {col} from {t}')]
    orphan_rows = [h for h in allh if h not in chart_md5]
    print(f'{t}: rows {len(allh)}, rows whose md5 is not in charts {len(orphan_rows)}, distinct such md5 {len(set(orphan_rows))}')

# md5 recheck, seeded sample
random.seed(20261006)
present = [r for r in rows if os.path.exists(r[1])]
sample = random.sample(present, min(int(sys.argv[2]), len(present)))
changed = []
nbytes = 0
for md5, p in sample:
    h = hashlib.md5()
    with open(p, 'rb') as f:
        while True:
            b = f.read(1 << 20)
            if not b:
                break
            nbytes += len(b)
            h.update(b)
    if h.hexdigest() != md5:
        changed.append((md5, h.hexdigest(), p))
print(f'md5 recheck sample {len(sample)} ({nbytes/1e6:.0f} MB): changed {len(changed)}')
for x in changed[:10]:
    print('   ', x)
