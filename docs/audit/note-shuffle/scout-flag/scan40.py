"""Scan functions that read a field at a given displacement and test bit 0x40 (or bit 6) near it.
usage: py -P -E scan40.py <disp hex> [mask hex]
"""
import sys, struct, pickle, bisect, re
sys.stdout.reconfigure(encoding="utf-8")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
import os
os.chdir(IL)
from chdis import data, secs, BASE, rva2off, names
import capstone

disp = int(sys.argv[1], 16)
mask = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x40
bit = mask.bit_length() - 1
addrs = sorted(names)
pat = struct.pack("<I", disp)
cands = set()
for name, va, vsize, raw, rsize in secs:
    if name not in (".text", "il2cpp"):
        continue
    blob = data[raw:raw + rsize]
    i = blob.find(pat)
    while i != -1:
        rva = va + i
        k = bisect.bisect_right(addrs, rva) - 1
        if k >= 0:
            cands.add(addrs[k])
        i = blob.find(pat, i + 1)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True
immre = re.compile(r"(?:0x%x|%d)$" % (mask, mask))
hits = 0
for f in sorted(cands):
    k = addrs.index(f)
    end = addrs[k + 1] if k + 1 < len(addrs) else f + 0x1000
    size = min(end - f, 0x20000)
    off = rva2off(f)
    if off is None:
        continue
    ins = list(md.disasm(data[off:off + size], BASE + f))
    for j, x in enumerate(ins):
        hasdisp = any(op.type == capstone.x86.X86_OP_MEM and op.mem.disp == disp and op.mem.base != capstone.x86.X86_REG_RIP for op in x.operands)
        if not hasdisp:
            continue
        window = ins[j:j + 12]
        for y in window:
            s = y.op_str
            if (y.mnemonic in ("test", "and") and immre.search(s)) or (y.mnemonic == "bt" and s.endswith(", %d" % bit)) or (y.mnemonic in ("shr", "sar") and s.endswith(", %d" % bit)):
                print(f"{f:#x} {names[f]} : load {x.address-BASE:#x} '{x.mnemonic} {x.op_str}' -> {y.address-BASE:#x} '{y.mnemonic} {y.op_str}'")
                hits += 1
                break
print("candidates", len(cands), "hits", hits)
