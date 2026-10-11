import re,struct,sys
sys.stdout.reconfigure(encoding="utf-8")
data=open(r"C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll","rb").read()
pe=struct.unpack_from("<I",data,0x3C)[0]; nsec=struct.unpack_from("<H",data,pe+6)[0]; optsz=struct.unpack_from("<H",data,pe+20)[0]
for i in range(nsec):
    o=pe+24+optsz+40*i
    if data[o:o+8].rstrip(b"\0")==b"il2cpp": va,rsize,raw=struct.unpack_from("<III",data,o+12)
seg=data[raw:raw+rsize]
# byte ops on [reg+7d]/[reg+7e]: F6 /0 imm8, 80 /x imm8
for m in re.finditer(rb"[\xF6\x80][\x40-\x7f][\x7d\x7e].",seg,re.S):
    s=m.group(0); op,mr,d,imm=s
    if (mr&7)==4: continue
    if op==0xf6 and (mr>>3)&7!=0: continue
    if (d==0x7d and imm&0xC0) or (d==0x7e and imm&3):
        print(f"{va+m.start():X} op={op:x} /{(mr>>3)&7} disp={d:x} imm={imm:x}")
# bt/bts/btr/btc dword [reg+7c], 14..16
for m in re.finditer(rb"\x0F\xBA[\x40-\x7f]\x7c[\x0e\x0f\x10]",seg):
    s=m.group(0)
    if (s[2]&7)!=4 and (s[2]>>3)&7>=4: print(f"{va+m.start():X} bt-family imm={s[4]:x}")
