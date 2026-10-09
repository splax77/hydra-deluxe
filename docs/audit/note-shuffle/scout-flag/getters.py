"""Disassemble short getters (until first ret). usage: py -P -E getters.py <rva> ..."""
import sys, os
sys.stdout.reconfigure(encoding="utf-8")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
os.chdir(IL)
from chdis import data, rva2off, BASE
import capstone
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
for a in sys.argv[1:]:
    r = int(a, 16)
    off = rva2off(r)
    parts = []
    for addr, sz, mn, ops in md.disasm_lite(data[off:off + 0x60], BASE + r):
        parts.append(f"{mn} {ops}".strip())
        if mn in ("ret", "jmp", "int3"):
            break
    print(f"{r:#x}: " + " ; ".join(parts))
