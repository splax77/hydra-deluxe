"""Sweep every named function; report ones where test/and/bt of bit 0x40 (Shuffle) and 0x20 (Mirror)
on the same operand occur within a window. Also report test/bt of 0x40 next to known modifier field disps.
usage: py -P -E scanpair.py > out.txt
"""
import sys, struct, bisect, re, os
sys.stdout.reconfigure(encoding="utf-8")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
os.chdir(IL)
from chdis import data, secs, BASE, rva2off, names
import capstone

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
addrs = sorted(names)
r = re.compile(r"^(test|and|bt)$")


def operand_and_bit(mn, ops):
    parts = [p.strip() for p in ops.rsplit(",", 1)]
    if len(parts) != 2:
        return None
    a, b = parts
    try:
        v = int(b, 0)
    except ValueError:
        return None
    if mn == "bt":
        return a, 1 << v
    return a, v


for idx, f in enumerate(addrs):
    end = addrs[idx + 1] if idx + 1 < len(addrs) else f + 0x1000
    size = end - f
    if size <= 0 or size > 0x40000:
        continue
    off = rva2off(f)
    if off is None:
        continue
    ins = list(md.disasm_lite(data[off:off + size], BASE + f))
    tests = []
    for j, (addr, sz, mn, ops) in enumerate(ins):
        if mn in ("test", "and", "bt"):
            ob = operand_and_bit(mn, ops)
            if ob and ob[1] in (0x20, 0x40, 0x60):
                tests.append((j, addr, ob[0], ob[1], mn, ops))
    if not tests:
        continue
    has40 = [t for t in tests if t[3] & 0x40]
    has20 = [t for t in tests if t[3] & 0x20]
    for a in has40:
        for b in has20:
            if a is b and a[3] != 0x60:
                continue
            if abs(a[0] - b[0]) <= 60 and (a[2] == b[2] or "[" in a[2] and a[2] == b[2]):
                print(f"{f:#x} {names[f]} : {a[1]-BASE:#x} '{a[4]} {a[5]}'  ~ {b[1]-BASE:#x} '{b[4]} {b[5]}'")
                break
        else:
            continue
        break
