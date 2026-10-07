import sqlite3,zlib,random,time
from compression import zstd
c=sqlite3.connect('hydra.db'); T0=time.time()
def bench(name,sql,n=3000):
  rows=[r[0] for r in c.execute(sql)]; random.seed(7); random.shuffle(rows)
  test=rows[:n]; train=rows[n:n+5000]; raw=sum(map(len,test)); full=sum(map(len,rows))
  print(f'== {name}: sample {len(test)} of {len(rows)} rows, {raw/1e6:.1f} of {full/1e6:.1f} MB',flush=True)
  def run(label,fn):
    t=time.time(); z=sum(len(fn(r)) for r in test); dt=time.time()-t
    print(f'  {label:22s} {100*z/raw:4.0f}% of raw  -> full column ~{full*z/raw/1e6:5.1f} MB   {raw/dt/1e6:6.1f} MB/s',flush=True)
  run('zlib-6 (miniz-like)',lambda r:zlib.compress(r,6))
  run('zstd-3',lambda r:zstd.compress(r,3))
  run('zstd-19',lambda r:zstd.compress(r,19))
  for ds in (16384,114688):
    t=time.time(); d=zstd.ZstdDict(zstd.train_dict(train,ds).dict_content); print(f'  (trained {ds//1024}K dict in {time.time()-t:.1f}s)',flush=True)
    run(f'zstd-3 + {ds//1024}K dict',lambda r:zstd.compress(r,3,zstd_dict=d))
    run(f'zstd-19 + {ds//1024}K dict',lambda r:zstd.compress(r,19,zstd_dict=d))
bench('paths.payload','select payload from paths')
bench('songmeta.tempomap','select tempomap from songmeta')
bench('results.structure','select structure from results')
print(f'total {time.time()-T0:.0f}s')
