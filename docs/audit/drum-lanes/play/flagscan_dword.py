import re,struct,sys,bisect
sys.stdout.reconfigure(encoding="utf-8")
data=open(r"C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll","rb").read()
pe=struct.unpack_from("<I",data,0x3C)[0]; nsec=struct.unpack_from("<H",data,pe+6)[0]; optsz=struct.unpack_from("<H",data,pe+20)[0]
for i in range(nsec):
    o=pe+24+optsz+40*i
    if data[o:o+8].rstrip(b"\0")==b"il2cpp":
        va,rsize,raw=struct.unpack_from("<III",data,o+12)
seg=data[raw:raw+rsize]
pat=re.compile(rb"[\xF7\x81][\x40-\x7f]\x7c....",re.S)
n=0
for m in pat.finditer(seg):
    s=m.group(0); op=s[0]; mr=s[1]; v=struct.unpack("<I",s[3:7])[0]
    if (mr&7)==4: continue
    if op==0xf7 and (mr>>3)&7!=0: continue
    n+=1
    if v&0x1C000 and v!=0xFFFFFFFF:
        print(f"{va+m.start():X} op={op:x} /{(mr>>3)&7} imm=0x{v:X}")
print("total",n)
