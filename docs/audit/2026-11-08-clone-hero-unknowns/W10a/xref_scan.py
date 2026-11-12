r"""W10a's byte scan: find code in GameAssembly.dll that reaches given RVAs.

For each target RVA it prints every place in the executable sections where
a 4-byte relative offset lands on the target, counted from the end of those
4 bytes. That covers a direct `call`/`jmp` rel32 (opcode E8 or E9 just in
front, marked CALL or JMP) and any RIP-relative memory operand, such as the
`mov reg, [rip+X]` that loads a string literal slot (marked RIPREF, with the
two bytes in front shown so a reader can check the opcode). It also prints
every 8-byte absolute pointer to the target's virtual address anywhere in
the file (PTR). Each hit is named by the method that holds it: the last
method start in script.json at or before the hit, with its name.

These are direct references only. A call through a vtable, a delegate or an
interface slot is not found.

How to run it (no -I is needed; it uses only the standard library):

    C:\Python314\python.exe xref_scan.py <GameAssembly.dll> <script.json> <rva hex> [<rva hex> ...]

Its output for build 6142 is kept beside this file as xref_<what>.txt.
"""
import bisect, json, struct, sys

dll, script = sys.argv[1], sys.argv[2]
targets = [int(a, 16) for a in sys.argv[3:]]
data = open(dll, "rb").read()
pe = struct.unpack_from("<I", data, 0x3C)[0]
nsec = struct.unpack_from("<H", data, pe + 6)[0]
optsz = struct.unpack_from("<H", data, pe + 20)[0]
base = struct.unpack_from("<Q", data, pe + 24 + 24)[0]
sec0 = pe + 24 + optsz
secs = []
for i in range(nsec):
    o = sec0 + 40 * i
    name = data[o:o + 8].rstrip(b"\0").decode()
    vsz, va, rsz, raw = struct.unpack_from("<IIII", data, o + 8)
    ch = struct.unpack_from("<I", data, o + 36)[0]
    secs.append((name, va, vsz, rsz, raw, bool(ch & 0x20000000)))

meth = json.load(open(script, encoding="utf-8"))["ScriptMethod"]
meth.sort(key=lambda m: m["Address"])
starts = [m["Address"] for m in meth]

def owner(rva):
    i = bisect.bisect_right(starts, rva) - 1
    if i < 0:
        return "?"
    return f"{meth[i]['Address']:X} {meth[i]['Name']}"

tset = set(targets)
hits = {t: [] for t in targets}
for name, va, vsz, rsz, raw, exe in secs:
    if not exe:
        continue
    n = min(vsz, rsz)
    blob = data[raw:raw + n]
    for i in range(0, n - 4):
        d = struct.unpack_from("<i", blob, i)[0]
        dest = va + i + 4 + d
        if dest in tset:
            pre = blob[i - 1] if i else 0
            pre2 = blob[max(0, i - 2):i].hex()
            kind = {0xE8: "CALL", 0xE9: "JMP"}.get(pre, "RIPREF")
            hits[dest].append((va + i - (1 if kind != "RIPREF" else 0), kind, pre2))
for t in targets:
    pat = struct.pack("<Q", base + t)
    p = data.find(pat)
    while p != -1:
        rva = None
        for name, va, vsz, rsz, raw, exe in secs:
            if raw <= p < raw + rsz:
                rva = va + p - raw
        hits[t].append((rva if rva is not None else -1, "PTR", f"file+{p:X}"))
        p = data.find(pat, p + 1)

for t in targets:
    print(f"== target {t:X} ({len(hits[t])} hits)")
    for rva, kind, extra in sorted(hits[t]):
        print(f"  {kind:6} at {rva:X} [{extra}] in {owner(rva)}")
