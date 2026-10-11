import re,struct,sys
sys.stdout.reconfigure(encoding="utf-8")
data=open(r"C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll","rb").read()
pe=struct.unpack_from("<I",data,0x3C)[0]; nsec=struct.unpack_from("<H",data,pe+6)[0]; optsz=struct.unpack_from("<H",data,pe+20)[0]
for i in range(nsec):
    o=pe+24+optsz+40*i
    if data[o:o+8].rstrip(b"\0")==b"il2cpp": va,rsize,raw=struct.unpack_from("<III",data,o+12)
seg=data[raw:raw+rsize]
for m in re.finditer(rb"[\xF7\x81][\x40-\x7f]\x7c....",seg,re.S):
    s=m.group(0); mr=s[1]
    if (mr&7)==4: continue
    v=struct.unpack("<I",s[3:7])[0]
    if v in (0x800,0xFFFFF7FF):
        print(f"{va+m.start():X} {'test' if s[0]==0xf7 else ('or' if (mr>>3)&7==1 else 'and' if (mr>>3)&7==4 else 'op%d'%((mr>>3)&7))} 0x{v:X}")
for m in re.finditer(rb"[\xF6\x80][\x40-\x7f]\x7d[\x08\xf7]",seg,re.S):
    s=m.group(0)
    if (s[1]&7)!=4: print(f"{va+m.start():X} byte+7d op={s[0]:x} /{(s[1]>>3)&7} imm={s[3]:x}")
