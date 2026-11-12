r"""W10a's static-field scan: find code that reads one static field of a class.

IL2CPP code reaches a static field by loading the class's TypeInfo slot
(RIP-relative), then its static block at TypeInfo+0xB8, then the field's
offset. This script finds every RIP-relative load of the TypeInfo slot,
disassembles the next 64 bytes with capstone, and prints the place when a
memory read at +<field offset> follows a read at +0xB8. A store to the field
(the field as destination) is printed as WRITE. It is a pattern scan: a read
that keeps the static block in a register across a longer stretch is missed.

How to run it (capstone lives in the user site-packages, so no -I):

    C:\Python314\python.exe static_field_scan.py <GameAssembly.dll> <script.json> <TypeInfo slot hex> <field offset hex>
"""
import bisect, json, struct, sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

dll, script, slot, field = sys.argv[1], sys.argv[2], int(sys.argv[3], 16), int(sys.argv[4], 16)
data = open(dll, "rb").read()
pe = struct.unpack_from("<I", data, 0x3C)[0]
nsec = struct.unpack_from("<H", data, pe + 6)[0]
optsz = struct.unpack_from("<H", data, pe + 20)[0]
sec0 = pe + 24 + optsz
meth = sorted(json.load(open(script, encoding="utf-8"))["ScriptMethod"], key=lambda m: m["Address"])
starts = [m["Address"] for m in meth]
md = Cs(CS_ARCH_X86, CS_MODE_64)

def owner(rva):
    i = bisect.bisect_right(starts, rva) - 1
    return f"{meth[i]['Address']:X} {meth[i]['Name']}" if i >= 0 else "?"

for k in range(nsec):
    o = sec0 + 40 * k
    vsz, va, rsz, raw = struct.unpack_from("<IIII", data, o + 8)
    if not struct.unpack_from("<I", data, o + 36)[0] & 0x20000000:
        continue
    n = min(vsz, rsz)
    blob = data[raw:raw + n]
    for i in range(3, n - 4):
        if va + i + 4 + struct.unpack_from("<i", blob, i)[0] != slot:
            continue
        if blob[i - 2] != 0x8B:  # mov reg, [rip+X]; skips the lea of the lazy init
            continue
        start = va + i - 3
        seen_b8 = False
        for ins in md.disasm(blob[i - 3:i + 64], start):
            s = ins.op_str
            if "+ 0xb8]" in s:
                seen_b8 = True
            elif seen_b8 and f"+ 0x{field:x}]" in s:
                kind = "WRITE" if s.startswith("qword ptr") or s.startswith("dword ptr") else "READ"
                print(f"{kind:5} at {ins.address:X}: {ins.mnemonic} {s}  in {owner(ins.address)}")
                break
