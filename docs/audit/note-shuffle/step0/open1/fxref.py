"""Fast xref: find RIP-relative disp32 refs (data) or rel32 call/jmp (code) to target RVAs.
usage: py -I fxref.py <il2cpp dir> [-c] <rva hex>...   (-c = direct call/jmp targets)"""
import sys, struct, pickle, bisect, os
import numpy as np
sys.stdout.reconfigure(encoding="utf-8")
d = sys.argv[1]
sys.path.insert(0, d); sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
from chdis import data, secs
names, meta = pickle.load(open(os.path.join(d, "names.pkl"), "rb"))
addrs = sorted(names)
args = sys.argv[2:]
calls = False
if args and args[0] == "-c":
    calls = True; args = args[1:]
targets = [int(a, 16) for a in args]
for name, va, vsize, raw, rsize in secs:
    if name not in (".text", "il2cpp"):
        continue
    blob = np.frombuffer(data, dtype=np.uint8, count=rsize, offset=raw)
    n = rsize - 4
    # int32 at every offset
    b = blob.astype(np.int64)
    v = (b[0:n] | (b[1:n+1] << 8) | (b[2:n+2] << 16) | (b[3:n+3] << 24))
    v = np.where(v >= 2**31, v - 2**32, v)
    pos = np.arange(n, dtype=np.int64) + va + 4
    tgt = pos + v
    for t in targets:
        hits = np.nonzero(tgt == t)[0]
        for i in hits:
            rva = va + int(i)
            if calls:
                if i == 0 or blob[i-1] not in (0xE8, 0xE9):
                    continue
                rva -= 1
            k = bisect.bisect_right(addrs, rva) - 1
            print(f"{t:#x} <- {rva:#x} in {addrs[k]:#x} {names[addrs[k]]}")
