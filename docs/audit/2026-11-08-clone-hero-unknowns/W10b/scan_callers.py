"""Direct callers only (byte scan) of the given RVAs in GameAssembly.dll.

It finds every E8 (call) or E9 (jmp) byte in the two code sections (il2cpp, which holds the game's
methods, and .text) whose rel32 operand lands
on one of the RVAs, and names the method holding each site by the nearest
method start in script.json at or below it. Indirect calls (through a
vtable or a pointer) are not found.

Run from the worktree root:
    py docs/audit/2026-11-08-clone-hero-unknowns/W10b/scan_callers.py <rva hex> ...
"""
import bisect
import os
import sys

import numpy as np

sys.path.insert(0, os.getcwd())
from tools.ch_mcp import chdis  # noqa: E402


def main(argv):
    dll, dumps = chdis.installed_paths()
    img = chdis.Image(dll, dumps)
    print(f"dll {dll}")
    targets = [int(a, 16) for a in argv]
    starts = sorted(img.names.items())
    addrs = [s[0] for s in starts]
    for name, va, vsize, raw, rsize in img.secs:
        if name not in ("il2cpp", ".text"):
            continue
        buf = np.frombuffer(img.data, dtype=np.uint8, count=rsize, offset=raw)
        ops = np.nonzero((buf[:-5] == 0xE8) | (buf[:-5] == 0xE9))[0]
        b = buf.astype(np.int64)
        rel = b[ops + 1] | (b[ops + 2] << 8) | (b[ops + 3] << 16) | (b[ops + 4] << 24)
        rel = np.where(rel >= 1 << 31, rel - (1 << 32), rel)
        tgt = va + ops + 5 + rel
        for t in targets:
            for o in ops[tgt == t]:
                site = va + int(o)
                i = bisect.bisect_right(addrs, site) - 1
                kind = "call" if buf[o] == 0xE8 else "jmp"
                print(f"{t:X} <- {kind} at {site:X} in {starts[i][1]} ({starts[i][0]:X})")
            print(f"{t:X}: {int((tgt == t).sum())} sites in {name}")


if __name__ == "__main__":
    main(sys.argv[1:])
