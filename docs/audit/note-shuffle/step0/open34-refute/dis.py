"""Wrapper: run chdis/callers under py -I by adding the needed paths by hand.

usage: py -I dis.py dis <rva hex> [max_bytes hex] > out
       py -I dis.py callers <rva hex>... > out
"""
import sys, os
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
os.chdir(IL)
import chdis

mode = sys.argv[1]
if mode == "dis":
    chdis.dis(int(sys.argv[2], 16), int(sys.argv[3], 16) if len(sys.argv) > 3 else 0x1200)
elif mode == "callers":
    import struct, bisect
    names = chdis.names
    addrs = sorted(names)
    targets = {int(a, 16) for a in sys.argv[2:]}
    for name, va, vsize, raw, rsize in chdis.secs:
        if name not in (".text", "il2cpp"):
            continue
        blob = chdis.data[raw:raw + rsize]
        for i in range(len(blob) - 5):
            if blob[i] not in (0xE8, 0xE9):
                continue
            disp = struct.unpack_from("<i", blob, i + 1)[0]
            t = va + i + 5 + disp
            if t in targets:
                rva = va + i
                k = bisect.bisect_right(addrs, rva) - 1
                print(f"{t:#x} <- {rva:#x} in {addrs[k]:#x} {names[addrs[k]]}")
