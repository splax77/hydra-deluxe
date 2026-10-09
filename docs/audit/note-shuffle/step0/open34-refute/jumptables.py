"""Decode the two switch tables in the shuffle 0x20F0C30 (lane count N and the excluded mask per instrument)."""
import sys, struct
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
import chdis

def table(rva, n):
    off = chdis.rva2off(rva)
    return [struct.unpack_from("<I", chdis.data, off + 4 * i)[0] for i in range(n)]

# table 1 at 0x20F11CC: target = r8 + entry, r8 = image base (lea r8,[rip-0x20f0e21] -> 0)
t1 = table(0x20F11CC, 15)
t2 = table(0x20F1208, 15)
labels1 = {0x20F0E2E: "N=5", 0x20F0E3B: "N=6", 0x20F0E48: "N=8", 0x20F0E55: "N=7", 0x20F0E62: "N=1 (default)", 0x20F0E69: "N=edx+1 (default path)"}
labels2 = {0x20F0E82: "si=1 (excl bit 1), di=1", 0x20F0E8C: "si=1, di=1 (same)", 0x20F0E99: "si=0, di=1"}
for i in range(15):
    print(f"instrument {i:2d}: table1 -> {t1[i]:#x} {labels1.get(t1[i], '?')} | table2 -> {t2[i]:#x} {labels2.get(t2[i], '?')}")
