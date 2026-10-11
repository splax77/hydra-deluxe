import re,struct,sys
sys.stdout.reconfigure(encoding="utf-8")
data=open(r"C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll","rb").read()
pe=struct.unpack_from("<I",data,0x3C)[0]; nsec=struct.unpack_from("<H",data,pe+6)[0]; optsz=struct.unpack_from("<H",data,pe+20)[0]
secs=[]
for i in range(nsec):
    o=pe+24+optsz+40*i
    n=data[o:o+8].rstrip(b"\0").decode(); vs,va,rs,raw=struct.unpack_from("<IIII",data,o+8); secs.append((n,va,vs,raw,rs))
def sec(off):
    for n,va,vs,raw,rs in secs:
        if raw<=off<raw+rs: return n,va+off-raw
targets={"20DCC90 press-A":0x20DCC90,"20DD190 press-B":0x20DD190,"20DD700 press-C":0x20DD700,"20DEAA0 press LIVE":0x20DEAA0,"20DEFF0 press-E":0x20DEFF0,
 "20DE680 wrongpress-A":0x20DE680,"20DE9A0 wrongpress LIVE":0x20DE9A0,"20DD630 join-A":0x20DD630,"20DE420 join LIVE":0x20DE420,"20DE5B0 timeout":0x20DE5B0,"20DDEC0 slot9":0x20DDEC0}
# direct call/jmp rel32 callers across il2cpp section
code=[s for s in secs if s[0]=="il2cpp"][0]; cva,craw,crs=code[1],code[3],code[4]
seg=data[craw:craw+crs]
for name,rva in targets.items():
    abs8=struct.pack("<Q",0x180000000+rva); abs4=struct.pack("<I",rva)
    ptr=[sec(m.start()) for m in re.finditer(re.escape(abs8),data)]
    # rel32 call (E8) / jmp (E9)
    calls=[]
    for opc in (0xE8,0xE9):
        for m in re.finditer(bytes([opc])+rb"....",seg,re.S):
            rel=struct.unpack("<i",m.group(0)[1:])[0]; at=cva+m.start()
            if at+5+rel==rva: calls.append(f"{'call' if opc==0xE8 else 'jmp'}@{at:X}")
    print(f"{name}: abs-pointers {[(a,hex(b)) for a,b in ptr if a]}  direct {calls}")
