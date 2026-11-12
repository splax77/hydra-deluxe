"""Find code in GameAssembly.dll (build 6142) that points at given RVAs.

A byte scan: for every byte offset in the executable sections, read the
next four bytes as a signed 32-bit displacement and count it as a reference
when the address right after those four bytes, plus the displacement, is a
target. That form covers RIP-relative loads (a string literal slot, say),
`call rel32` and `jmp rel32`. It finds direct references only; a call through
a register or a table is not seen. Each hit is printed with the named method
that starts at or before it (from script.json), so a hit inside unnamed code
names the last named method before it.

Run from the repo root:
    C:\\Python314\\python.exe docs/audit/2026-11-08-clone-hero-unknowns/W8c/xref.py <rva hex> [<rva hex> ...]
"""

import bisect
import os
import sys

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "..")))

import numpy as np

from tools.ch_mcp import chdis


def main(argv):
    targets = [int(a, 16) for a in argv]
    image = chdis.Image(*chdis.installed_paths())
    starts = sorted(image.names)
    for name, va, vsize, raw, rsize in image.secs:
        if name not in (".text", "il2cpp"):
            continue
        blob = np.frombuffer(image.data[raw:raw + rsize], dtype=np.uint8)
        n = len(blob) - 4
        disp = (blob[0:n].astype(np.int64)
                | (blob[1:n + 1].astype(np.int64) << 8)
                | (blob[2:n + 2].astype(np.int64) << 16)
                | (blob[3:n + 3].astype(np.int64) << 24))
        disp = np.where(disp >= 1 << 31, disp - (1 << 32), disp)
        dest = va + np.arange(n, dtype=np.int64) + 4 + disp
        for t in targets:
            for i in np.nonzero(dest == t)[0]:
                rva = va + int(i) - 1  # the opcode byte before a call/jmp disp
                k = bisect.bisect_right(starts, rva) - 1
                owner = f"{starts[k]:#x} {image.names[starts[k]]}" if k >= 0 else "?"
                print(f"target {t:#x}  disp at {va + int(i):#x} ({name})  in {owner}")


if __name__ == "__main__":
    main(sys.argv[1:])
