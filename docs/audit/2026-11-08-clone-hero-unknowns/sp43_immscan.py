r"""SP-43's scan: look for a 3 ms constant in the drum engine and its base classes.

What it scans: every method of the classes given by TypeDefIndex in the game's
dump.cs. A class's methods are the RVA lines in its dump.cs block, its own
methods and the generic instances listed under them. Nested types have their
own TypeDefIndex and are not included unless named. A method ends at the next
method start in script.json. Each method is disassembled with capstone. Every
immediate operand and every RIP-relative memory read is checked against 0.003,
3.0 and 3000.0 as float or double bit patterns. Every immediate is also
checked against the integers 3 and 3000, and each one found is printed.

How to run it (capstone lives in the user site-packages, so no -I):

    set PYTHONIOENCODING=utf-8
    C:\Python314\python.exe sp43_immscan.py ^
        "C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll" ^
        C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs ^
        C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\script.json ^
        8253 8255 8288 8544

Its output for build 6142 is sp43_immscan_output.txt beside this file. W7.md,
SP-43, says what the four classes are and reads the hits.
"""
import bisect, json, re, struct, sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_RIP

dll, dumpcs, script = sys.argv[1], sys.argv[2], sys.argv[3]
want = set(sys.argv[4:])
data = open(dll, "rb").read()
pe = struct.unpack_from("<I", data, 0x3C)[0]
nsec = struct.unpack_from("<H", data, pe + 6)[0]
optsz = struct.unpack_from("<H", data, pe + 20)[0]
sec0 = pe + 24 + optsz
secs = []
for i in range(nsec):
    o = sec0 + 40 * i
    vsz, va, rsz, raw = struct.unpack_from("<IIII", data, o + 8)
    secs.append((va, max(vsz, rsz), raw))

def off(rva):
    for va, sz, raw in secs:
        if va <= rva < va + sz:
            return raw + rva - va
    return None

starts = sorted({m["Address"] for m in json.load(open(script, encoding="utf-8"))["ScriptMethod"]})

vals = {}
for v in (0.003, 3.0, 3000.0):
    vals[struct.unpack("<I", struct.pack("<f", v))[0]] = f"float {v}"
    vals[struct.unpack("<Q", struct.pack("<d", v))[0]] = f"double {v}"

cls_re = re.compile(r"^(public|private|internal|protected).*\b(class|struct|enum|interface)\b.*TypeDefIndex: (\d+)")
rva_re = re.compile(r"RVA: 0x([0-9A-F]+)")
methods = {}
cur = None
for line in open(dumpcs, encoding="utf-8"):
    m = cls_re.match(line)
    if m:
        cur = m.group(3) if m.group(3) in want else None
        continue
    if cur:
        r = rva_re.search(line)
        if r:
            methods.setdefault(cur, set()).add(int(r.group(1), 16))

md = Cs(CS_ARCH_X86, CS_MODE_64)
md.detail = True
total_insn = total_imm = total_rip = 0
small = {3: 0, 3000: 0}
hits = []
for cls in sorted(methods):
    rvas = sorted(methods[cls])
    nbytes = 0
    for s in rvas:
        i = bisect.bisect_right(starts, s)
        end = starts[i] if i < len(starts) else s + 0x1000
        o = off(s)
        code = data[o:o + (end - s)]
        nbytes += len(code)
        for ins in md.disasm(code, 0x180000000 + s):
            total_insn += 1
            for op in ins.operands:
                if op.type == X86_OP_IMM:
                    total_imm += 1
                    u32 = op.imm & 0xFFFFFFFF
                    u64 = op.imm & 0xFFFFFFFFFFFFFFFF
                    if u32 in vals or u64 in vals:
                        hits.append((cls, s, ins.address - 0x180000000, ins.mnemonic, ins.op_str, vals.get(u64) or vals.get(u32)))
                    if op.imm in small:
                        small[op.imm] += 1
                        print(f"  int {op.imm} in #{cls} method {s:#x} at {ins.address - 0x180000000:#x}: {ins.mnemonic} {ins.op_str}")
                elif op.type == X86_OP_MEM and op.mem.base == X86_REG_RIP:
                    total_rip += 1
                    tgt = ins.address + ins.size + op.mem.disp - 0x180000000
                    to = off(tgt)
                    if to is not None and to + 8 <= len(data):
                        u32 = struct.unpack_from("<I", data, to)[0]
                        u64 = struct.unpack_from("<Q", data, to)[0]
                        if (op.size == 4 and u32 in vals) or (op.size == 8 and u64 in vals) or (op.size == 16 and u64 in vals):
                            hits.append((cls, s, ins.address - 0x180000000, ins.mnemonic, ins.op_str, "rip-load " + (vals.get(u64) or vals.get(u32))))
    print(f"class #{cls}: {len(rvas)} method starts, {nbytes} bytes")
print(f"instructions {total_insn}, immediate operands {total_imm}, rip-relative reads {total_rip}")
print(f"integer immediates equal to 3: {small[3]}, equal to 3000: {small[3000]}")
for h in hits:
    print("HIT class #%s method %#x at %#x: %s %s (%s)" % h)
print(f"{len(hits)} hits")
