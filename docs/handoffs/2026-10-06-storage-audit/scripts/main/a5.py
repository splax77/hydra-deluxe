# Lossless re-encode estimate: varint/zigzag ints, delta ticks within an activation, chord as 1-byte length+bytes, f64 kept unless exactly representable as small int*1/1000? (kept f64 to stay lossless)
import sqlite3,struct,collections
from compression import zstd
c=sqlite3.connect('hydra.db')
def vlen(x):
  x=(x<<1)^(x>>63); n=1
  while x>=128: x>>=7; n+=1
  return n
F=collections.Counter(); offs=collections.Counter()
class R:
  def __init__(s,b): s.b=b; s.i=0
  def g(s,f,n): v=struct.unpack(f,s.b[s.i:s.i+n])[0]; s.i+=n; return v
  def u8(s): return s.g('<B',1)
  def u32(s): return s.g('<I',4)
  def i32(s): return s.g('<i',4)
  def i64(s): return s.g('<q',8)
  def f64(s): return s.g('<d',8)
  def str(s): n=s.u32(); v=s.b[s.i:s.i+n]; s.i+=n; return v
def fbytes(x):
  # f64 kept lossless: 4 bytes if float32-exact, else 8
  return 4 if struct.unpack('<f',struct.pack('<f',x))[0]==x else 8
new=0; old=0; f32ok=0; fall=0
for (p,) in c.execute('select payload from paths'):
  if struct.unpack('<I',p[:4])[0]!=7: continue
  old+=len(p); r=R(p); r.u32(); n=r.u32(); out=1+vlen(n)
  prev=0
  for _ in range(n):
    t=r.i64(); out+=vlen(t-prev); prev=t
    ch=r.str(); out+=1+len(ch); out+=vlen(r.i32())
    nb=r.u32(); out+=vlen(nb); bp=t
    for _ in range(nb):
      bt=r.i64(); out+=vlen(bt-bp); bp=bt
      ch=r.str(); out+=1+len(ch); out+=vlen(r.i32())+vlen(r.i32())
      if r.u8():
        x=r.f64(); fb=fbytes(x); out+=fb; fall+=1; f32ok+= fb==4; offs[round(x,6)]+=1
      out+=0  # flag folded into chord length byte high bit
    nq=r.u32(); out+=vlen(nq)
    for _ in range(nq):
      out+=fbytes(r.f64()); out+=1
      if r.u8(): out+=fbytes(r.f64())+fbytes(r.f64())
    out+=fbytes(r.f64()); out+=1
    if r.u8(): out+=fbytes(r.f64())+fbytes(r.f64())
    out+=1
    if r.u8(): out+=vlen(r.i64()-t)
    ns=r.u32(); out+=vlen(ns); sp=t
    for _ in range(ns):
      a=r.i64(); b=r.i64(); out+=vlen(a-sp)+vlen(b-a)+1; r.u8(); sp=a
    for k in range(2):
      m=r.u32(); out+=vlen(m); q=t
      for _ in range(m): x=r.i64(); out+=vlen(x-q); q=x
  assert r.i==len(p); new+=out
print(f'paths payload: now {old/1e6:.1f} MB -> varint/delta est {new/1e6:.1f} MB ({100*new/old:.0f}%)')
print(f'backend offsets: {fall} values, {100*f32ok/max(1,fall):.0f}% exact in float32; distinct values {len(offs)}; top {offs.most_common(5)}')
# tempomap
old=new=0
for (b,) in c.execute('select tempomap from songmeta'):
  old+=len(b); r=R(b); r.i64(); out=2; n=r.u32(); out+=vlen(n); pv=0
  for _ in range(n): t=r.i64(); out+=vlen(t-pv)+vlen(r.i64()); pv=t
  n=r.u32(); out+=vlen(n); pv=0
  for _ in range(n): t=r.i64(); out+=vlen(t-pv)+fbytes(r.f64()); pv=t
  new+=out
print(f'tempomap: now {old/1e6:.1f} MB -> varint/delta est {new/1e6:.1f} MB ({100*new/old:.0f}%)')
