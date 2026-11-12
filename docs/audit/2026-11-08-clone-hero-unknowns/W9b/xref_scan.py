"""Byte scan of GameAssembly.dll (build 6142) for IN-73.

Finds, in every executable section (il2cpp and .text), direct call/jmp rel32 to the
target methods, and RIP-relative disp32 references to the string-literal slots named
below. Finds, in every section, 8-byte pointers to the targets' VAs. Each hit is
mapped to the method it sits in, by the nearest lower "// RVA:" line in dump.cs.
Usage: python -I xref_scan.py <GameAssembly.dll> <dump.cs>
"""
import bisect
import re
import struct
import sys

IMAGE_BASE = 0x180000000
TARGETS = {0x285AD0: "gateway session request method"}
SLOTS = {
    0x3CACE00: "https://gateway-service.clonehero.net/api/session/request",
    0x3CACEB8: "https://gateway-service.clonehero.net/api/session/stats?backendVersion=",
    0x3CC2640: "Failed to fetch gateway stats: ",
    0x3CC9F00: "Fetching gateway stats for version: ",
    0x3CCEC60: "Making session request to gateway: ...",
    0x3CAC840: "https://analytics.clonehero.net/analytics/api/v1/",
    0x3CAC8F8: "https://api.clonehero.net/v1",
}


def sections(data):
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    count = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    base = pe + 24 + opt
    out = []
    for i in range(count):
        s = base + 40 * i
        name = data[s:s + 8].rstrip(b"\0").decode()
        vsize, va, rsize, raw = struct.unpack_from("<IIII", data, s + 8)
        chars = struct.unpack_from("<I", data, s + 36)[0]
        out.append((name, va, vsize, raw, rsize, bool(chars & 0x20000000)))
    return out


def methods(dump_path):
    starts, names = [], []
    rx = re.compile(r"// RVA: 0x([0-9A-F]+) ")
    with open(dump_path, encoding="utf-8") as f:
        lines = f.readlines()
    for i, line in enumerate(lines):
        m = rx.search(line)
        if m and i + 1 < len(lines):
            starts.append(int(m.group(1), 16))
            names.append(f"dump.cs line {i + 1}: {lines[i + 1].strip()[:90]}")
    order = sorted(range(len(starts)), key=lambda k: starts[k])
    return [starts[k] for k in order], [names[k] for k in order]


def owner(starts, names, rva):
    k = bisect.bisect_right(starts, rva) - 1
    return f"0x{starts[k]:X} ({names[k]})" if k >= 0 else "none"


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    data = open(sys.argv[1], "rb").read()
    starts, names = methods(sys.argv[2])
    secs = sections(data)
    print("sections:", ", ".join(f"{n} rva 0x{va:X} size 0x{vs:X}{' exec' if x else ''}"
                                 for n, va, vs, _, _, x in secs))
    for name, va, vsize, raw, rsize, execu in secs:
        blob = data[raw:raw + min(vsize, rsize)]
        if execu:
            print(f"== {name}: direct call/jmp rel32 and RIP-relative disp32")
            for p in range(len(blob) - 5):
                op = blob[p]
                if op in (0xE8, 0xE9):
                    rel = struct.unpack_from("<i", blob, p + 1)[0]
                    tgt = va + p + 5 + rel
                    if tgt in TARGETS:
                        print(f"call/jmp at 0x{va + p:X} -> 0x{tgt:X} in {owner(starts, names, va + p)}")
                disp = struct.unpack_from("<i", blob, p)[0]
                end = va + p + 4
                if end + disp in SLOTS:
                    print(f"disp32 at 0x{va + p:X} -> slot 0x{end + disp:X} '{SLOTS[end + disp]}' "
                          f"in {owner(starts, names, va + p)}")
        print(f"== {name}: 8-byte pointers to target VAs")
        for t in TARGETS:
            needle = struct.pack("<Q", IMAGE_BASE + t)
            i = blob.find(needle)
            while i >= 0:
                print(f"pointer at rva 0x{va + i:X} -> 0x{t:X}")
                i = blob.find(needle, i + 1)


if __name__ == "__main__":
    main()
