"""Find stores to GlobalVariables instance fields in GameAssembly.dll (build 6142).

A byte-and-listing scan, not a full data-flow analysis. It reads every method in
dump.cs that has an RVA. A method is scanned when it loads GlobalVariables'
type info (RIP-relative operand on the type-info slot) or belongs to the class
GlobalVariables itself. Inside a scanned method, walking the instructions in
address order and ignoring branches, a register is marked as "the instance"
when it is loaded as [typeinfo_reg + 0xB8] then [that + 8] (the static field
`instance`, dump.cs line 55829), or, in GlobalVariables' own instance methods,
when it holds rcx at entry or a copy of it. Any other write to a marked
register unmarks it, and a call unmarks the volatile registers. A store to
[marked + offset] for an offset asked for is printed with its method.

Misses: an instance spilled to the stack, passed in from a caller, reached
through another object's field, or used in code after the last named method.
Every hit is to be read in a listing before it is used.

Run from the worktree root (capstone sits in the user site-packages):
  C:\\Python314\\python.exe docs/audit/2026-11-08-clone-hero-unknowns/W9a/fieldwrites.py 28 7C
"""
import os
import re
import sys

sys.path.insert(0, os.getcwd())
import capstone  # noqa: E402
from capstone import x86  # noqa: E402

from tools.ch_mcp import dump_build, game_files  # noqa: E402

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from callscan import methods, sections  # noqa: E402

TYPEINFO_SLOT = 0x3CEF890  # meta:GlobalVariables_TypeInfo, as chdis names it
VOLATILE = {"rax", "rcx", "rdx", "r8", "r9", "r10", "r11"}


def full(cs, reg):
    name = cs.reg_name(reg)
    m = {"eax": "rax", "ecx": "rcx", "edx": "rdx", "ebx": "rbx", "esi": "rsi", "edi": "rdi",
         "ebp": "rbp", "esp": "rsp"}
    if name in m:
        return m[name]
    mm = re.match(r"(r\d+)[dwb]?$", name)
    if mm:
        return mm.group(1)
    return name


def main():
    offsets = {int(a, 16) for a in sys.argv[1:]}
    dll, _ = dump_build.game_inputs(game_files.GAME_DIR)
    data = open(dll, "rb").read()
    secs = [s for s in sections(data) if s[5]]
    meths = methods()
    starts = sorted({m[0] for m in meths})
    nxt = {starts[i]: starts[i + 1] for i in range(len(starts) - 1)}
    cs = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    cs.detail = True
    seen = set()
    for rva, cls, sig, line in meths:
        if rva in seen or rva not in nxt:
            continue
        seen.add(rva)
        end = min(nxt[rva], rva + 0x8000)
        sec = next((s for s in secs if s[1] <= rva < s[1] + s[2]), None)
        if not sec:
            continue
        code = data[sec[3] + rva - sec[1]: sec[3] + end - sec[1]]
        own = cls == "GlobalVariables" and " static " not in f" {sig} "
        insns = list(cs.disasm(code, rva))
        uses_ti = False
        for ins in insns:
            for op in ins.operands:
                if op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_RIP:
                    if ins.address + ins.size + op.mem.disp == TYPEINFO_SLOT:
                        uses_ti = True
        if not (own or uses_ti):
            continue
        ti, sf, inst = set(), set(), set()
        if own:
            inst.add("rcx")
        for ins in insns:
            ops = ins.operands
            dst = full(cs, ops[0].reg) if ops and ops[0].type == x86.X86_OP_REG else None
            if ins.mnemonic.startswith("mov") and len(ops) == 2 and ops[0].type == x86.X86_OP_MEM:
                b = full(cs, ops[0].mem.base) if ops[0].mem.base else None
                if b in inst and ops[0].mem.disp in offsets and ops[0].mem.index == 0:
                    print(f"0x{ins.address:X} {ins.mnemonic} {ins.op_str}    in 0x{rva:X} {cls} :: {sig} (dump.cs {line})")
                continue
            newti = newsf = newinst = False
            if ins.mnemonic == "mov" and len(ops) == 2 and ops[1].type == x86.X86_OP_MEM and dst:
                m = ops[1].mem
                base = full(cs, m.base) if m.base else None
                if m.base == x86.X86_REG_RIP and ins.address + ins.size + m.disp == TYPEINFO_SLOT:
                    newti = True
                elif base in ti and m.disp == 0xB8 and m.index == 0:
                    newsf = True
                elif base in sf and m.disp == 8 and m.index == 0:
                    newinst = True
            if ins.mnemonic == "mov" and len(ops) == 2 and ops[1].type == x86.X86_OP_REG and dst:
                src = full(cs, ops[1].reg)
                for s in (ti, sf, inst):
                    if src in s:
                        s.add(dst)
                    elif dst in s:
                        s.discard(dst)
                continue
            if ins.mnemonic == "call":
                for s in (ti, sf, inst):
                    s.difference_update(VOLATILE)
                continue
            if dst and ins.mnemonic not in ("cmp", "test", "push"):
                for s in (ti, sf, inst):
                    s.discard(dst)
            if newti:
                ti.add(dst)
            if newsf:
                sf.add(dst)
            if newinst:
                inst.add(dst)


if __name__ == "__main__":
    main()
