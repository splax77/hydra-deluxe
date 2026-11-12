"""Find direct callers of RVAs in Clone Hero's GameAssembly.dll (build 6142).

Direct callers only (byte scan): an E8 (call) or E9 (jmp) whose rel32 lands on
the RVA, inside an executable section, plus any 8-byte little-endian pointer to
the RVA's VA anywhere in the file. Each hit is named by the dump.cs method whose
RVA is the closest one at or below it.

Run from the worktree root:
  C:\\Python314\\python.exe docs/audit/2026-11-08-clone-hero-unknowns/W9a/callscan.py 32F590 [more RVAs]
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.getcwd())
from tools.ch_mcp import dump_build, game_files  # noqa: E402

DUMP = r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs"
BASE = 0x180000000


def sections(data):
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    optsize = struct.unpack_from("<H", data, pe + 20)[0]
    off = pe + 24 + optsize
    out = []
    for i in range(nsec):
        name, vsize, va, rawsize, rawptr = struct.unpack_from("<8sIIII", data, off + 40 * i)
        chars = struct.unpack_from("<I", data, off + 40 * i + 36)[0]
        out.append((name.rstrip(b"\0").decode(), va, vsize, rawptr, rawsize, bool(chars & 0x20000000)))
    return out


def methods():
    rvas = []
    cls = ""
    pending = None
    rx_cls = re.compile(r"^(?:\S+ )*(?:class|struct) (.+?) [:/{]")
    rx_rva = re.compile(r"// RVA: 0x([0-9A-F]+) ")
    with open(DUMP, encoding="utf-8") as f:
        for n, line in enumerate(f, 1):
            m = rx_cls.match(line)
            if m:
                cls = m.group(1)
                continue
            m = rx_rva.search(line)
            if m:
                pending = (int(m.group(1), 16), n)
                continue
            if pending:
                rvas.append((pending[0], cls, line.strip(), pending[1]))
                pending = None
    rvas.sort()
    return rvas


def owner(meths, rva):
    lo, hi = 0, len(meths) - 1
    best = None
    while lo <= hi:
        mid = (lo + hi) // 2
        if meths[mid][0] <= rva:
            best = meths[mid]
            lo = mid + 1
        else:
            hi = mid - 1
    return best


def main():
    targets = [int(a, 16) for a in sys.argv[1:]]
    dll, _ = dump_build.game_inputs(game_files.GAME_DIR)
    data = open(dll, "rb").read()
    secs = sections(data)
    meths = methods()
    for t in targets:
        print(f"== target 0x{t:X}")
        same = [m for m in meths if m[0] == t]
        for m in same:
            print(f"   dump.cs line {m[3]}: {m[1]} :: {m[2]}")
        for name, va, vsize, rawptr, rawsize, exe in secs:
            if not exe:
                continue
            blob = data[rawptr:rawptr + rawsize]
            for op in (0xE8, 0xE9):
                i = blob.find(bytes([op]))
                while i != -1 and i + 5 <= len(blob):
                    rel = struct.unpack_from("<i", blob, i + 1)[0]
                    src = va + i
                    if src + 5 + rel == t:
                        o = owner(meths, src)
                        kind = "call" if op == 0xE8 else "jmp"
                        print(f"   {kind} at 0x{src:X} in 0x{o[0]:X} {o[1]} :: {o[2]} (dump.cs {o[3]})")
                    i = blob.find(bytes([op]), i + 1)
        ptr = struct.pack("<Q", BASE + t)
        i = data.find(ptr)
        while i != -1:
            sec = next((s for s in secs if s[3] <= i < s[3] + s[4]), None)
            rva = sec[1] + (i - sec[3]) if sec else None
            print(f"   pointer at file 0x{i:X} (section {sec[0] if sec else '?'}, rva 0x{rva:X})")
            i = data.find(ptr, i + 1)


if __name__ == "__main__":
    main()
