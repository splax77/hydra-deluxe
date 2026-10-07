import sqlite3,struct
c=sqlite3.connect('hydra.db'); objs=c.execute("select name,rootpage from sqlite_master where rootpage>0").fetchall(); c.close()
f=open('hydra.db','rb').read(); PS=struct.unpack('>H',f[16:18])[0]; PS=65536 if PS==1 else PS
U=PS-f[20]
def page(n): return f[(n-1)*PS:n*PS]
def varint(b,i):
  v=0
  for k in range(8):
    x=b[i+k]; v=(v<<7)|(x&0x7f)
    if x<0x80: return v,i+k+1
  return (v<<8)|b[i+8],i+9
def ovf(n,cnt):
  while n: cnt['ovf']+=1; n=struct.unpack('>I',page(n)[:4])[0]
def walk(n,cnt):
  p=page(n); h=100 if n==1 else 0; t=p[h]
  cnt['pages']+=1
  nc=struct.unpack('>H',p[h+3:h+5])[0]; hs=12 if t in (2,5) else 8
  ptrs=[struct.unpack('>H',p[h+hs+2*i:h+hs+2*i+2])[0] for i in range(nc)]
  if t in (2,5): walk(struct.unpack('>I',p[h+8:h+12])[0],cnt)
  for o in ptrs:
    if t==5: walk(struct.unpack('>I',p[o:o+4])[0],cnt); continue
    if t==2: child=struct.unpack('>I',p[o:o+4])[0]; walk(child,cnt); o+=4
    pl,i=varint(p,o)
    if t==13: _,i=varint(p,i)
    cnt['cells']+=1; cnt['payload']+=pl
    X=U-35 if t==13 else ((U-12)*64//255)-23
    if pl>X:
      M=((U-12)*32//255)-23; K=M+((pl-M)%(U-4)); local=K if K<=X else M
      ovf(struct.unpack('>I',p[i+local:i+local+4])[0],cnt)
tot=0
for name,root in objs:
  cnt=dict(pages=0,ovf=0,cells=0,payload=0); walk(root,cnt)
  mb=(cnt['pages']+cnt['ovf'])*PS/1e6; tot+=cnt['pages']+cnt['ovf']
  print('%-30s %8.1f MB  pages %6d ovf %6d  payload %7.1f MB  fill %3.0f%%'%(name,mb,cnt['pages'],cnt['ovf'],cnt['payload']/1e6,100*cnt['payload']/max(1,(cnt['pages']+cnt['ovf'])*PS)))
print('accounted pages',tot,'of',len(f)//PS)
