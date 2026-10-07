import sqlite3,struct,collections
c=sqlite3.connect('hydra.db')
B=collections.Counter(); N=collections.Counter()
class R:
  def __init__(s,b,cat): s.b=b; s.i=0
  def take(s,n,cat): B[cat]+=n; s.i+=n; return s.b[s.i-n:s.i]
  def u8(s,cat): return s.take(1,cat)[0]
  def u32(s,cat): return struct.unpack('<I',s.take(4,cat))[0]
  def i32(s,cat): return struct.unpack('<i',s.take(4,cat))[0]
  def i64(s,cat): return struct.unpack('<q',s.take(8,cat))[0]
  def f64(s,cat): return struct.unpack('<d',s.take(8,cat))[0]
  def str(s,cat): n=s.u32(cat+'.len'); return s.take(n,cat)
  def opt(s,fn,cat):
    if s.u8(cat+'.flag'): return fn(cat)
  def scale(s,cat):
    if s.u8(cat+'.flag'): s.f64(cat); s.f64(cat)
def act(r):
  N['act']+=1
  r.i64('act.tick'); r.str('act.chord'); r.i32('act.points')
  nb=r.u32('backend.count'); N['backend']+=nb
  for _ in range(nb):
    r.i64('backend.tick'); r.str('backend.chord'); r.i32('backend.points'); r.i32('backend.sqout_points'); r.opt(r.f64,'backend.offset')
  nq=r.u32('sqin.count'); N['sqin']+=nq
  for _ in range(nq): r.f64('sqin.offset'); r.scale('sqin.scale')
  r.f64('act.e_offset'); r.scale('act.transfer_post'); r.opt(r.i64,'act.sqout_tick')
  ns=r.u32('spend.count'); N['spend']+=ns
  for _ in range(ns): r.i64('spend.tick'); r.i64('spend.end_tick'); r.u8('spend.kind')
  nb=r.u32('bank.count'); N['bank']+=nb
  for _ in range(nb): r.i64('bank.tick')
  nf=r.u32('fills.count'); N['fill']+=nf
  for _ in range(nf): r.i64('fills.tick')
tot=0; fmt=collections.Counter(); bad=0
for (p,) in c.execute('select payload from paths'):
  v=struct.unpack('<I',p[:4])[0]; fmt[v]+=len(p)
  if v!=7: continue
  snap=B.copy(); sn=N.copy()
  try:
    r=R(p,0); r.u32('hdr'); n=r.u32('hdr')
    for _ in range(n): act(r)
    assert r.i==len(p); tot+=len(p)
  except Exception: B.clear();B.update(snap);N.clear();N.update(sn);bad+=1
print('bytes by format',{k:round(v/1e6,1) for k,v in fmt.items()},'bad',bad)
print('total',tot/1e6,'MB'); print(dict(N))
grp=collections.Counter()
for k,v in B.items(): grp[k.split('.')[0]]+=v
for k,v in grp.most_common(): print('%-12s %6.1f MB %4.1f%%'%(k,v/1e6,100*v/tot))
print('--- detail')
for k,v in B.most_common(25): print('%-24s %6.1f MB'%(k,v/1e6))
