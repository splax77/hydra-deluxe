"""Dump the .mid note-type table and the .chart N-value jump table from the game binary."""
import sys, struct
sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
import os
os.chdir(IL)
import chdis
from chdis import data, rva2off

# .mid: byte table at 0x210D824 indexed by (note - 59), 0..42; case targets at 0x210D808 (6 dwords, relative to 0x210D7B0+? no: relative to rdx = image base 0)
tab = data[rva2off(0x210D824):rva2off(0x210D824) + 43]
cases = struct.unpack_from("<6i", data, rva2off(0x210D808))
print("mid case targets (RVA):", [hex(c) for c in cases])
ret = {0x210d7df: 13, 0x210d7e5: 14, 0x210d7eb: 15, 0x210d7f1: 16, 0x210d7f7: 18, 0x210d7fd: 17}
for i, b in enumerate(tab):
    note = 59 + i
    tgt = cases[b] if b < len(cases) else None
    print(f"note {note:3d}: case {b} -> {hex(tgt) if tgt else '?'} type {ret.get(tgt, '?')}")

# .chart: N 0..5 table at 0x213DB10 (6 dwords), outer table at 0x213DAE4 (11 dwords)
nt = struct.unpack_from("<6i", data, rva2off(0x213DB10))
print("chart N table (RVA):", [hex(c) for c in nt])
cret = {0x213d737: 13, 0x213d740: 14, 0x213d749: 15, 0x213d752: 16, 0x213d75d: 18, 0x213d766: 17}
for n, t in enumerate(nt):
    print(f"N {n}: -> {hex(t)} type {cret.get(t, '?')}")
ot = struct.unpack_from("<11i", data, rva2off(0x213DAE4))
print("chart outer table (byte [rbx+0xd] - 4):", [hex(c) for c in ot])
