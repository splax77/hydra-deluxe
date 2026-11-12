"""List byte stores to given offsets inside one class's methods (build 6142).

Disassembles every method that dump.cs gives an RVA under the named class
(nested classes such as coroutines count when their name starts with
"<class>."), from its RVA to the next method's RVA, and prints each `mov byte
ptr [reg + off], ...` for the offsets asked for. It does not check which object
the register holds; each hit is to be read in a listing.

Run from the worktree root (capstone sits in the user site-packages):
  C:\\Python314\\python.exe docs/audit/2026-11-08-clone-hero-unknowns/W9a/classstores.py PlayerSelectionMenu 240 241
"""
import os
import sys

sys.path.insert(0, os.getcwd())
import capstone  # noqa: E402
from capstone import x86  # noqa: E402

from tools.ch_mcp import dump_build, game_files  # noqa: E402

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from callscan import methods, sections  # noqa: E402


def main():
    cls_name = sys.argv[1]
    offsets = {int(a, 16) for a in sys.argv[2:]}
    dll, _ = dump_build.game_inputs(game_files.GAME_DIR)
    data = open(dll, "rb").read()
    secs = [s for s in sections(data) if s[5]]
    meths = methods()
    starts = sorted({m[0] for m in meths})
    nxt = {starts[i]: starts[i + 1] for i in range(len(starts) - 1)}
    cs = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    cs.detail = True
    n = 0
    for rva, cls, sig, line in meths:
        if not (cls == cls_name or cls.startswith(cls_name + ".")) or rva not in nxt:
            continue
        n += 1
        sec = next((s for s in secs if s[1] <= rva < s[1] + s[2]), None)
        if not sec:
            continue
        code = data[sec[3] + rva - sec[1]: sec[3] + nxt[rva] - sec[1]]
        for ins in cs.disasm(code, rva):
            ops = ins.operands
            if (ins.mnemonic == "mov" and len(ops) == 2 and ops[0].type == x86.X86_OP_MEM
                    and ops[0].size == 1 and ops[0].mem.disp in offsets and ops[0].mem.index == 0):
                print(f"0x{ins.address:X} {ins.mnemonic} {ins.op_str}    in 0x{rva:X} {cls} :: {sig} (dump.cs {line})")
    print(f"methods scanned: {n}")


if __name__ == "__main__":
    main()
