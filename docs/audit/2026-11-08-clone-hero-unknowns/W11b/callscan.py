"""Direct call/jmp rel32 callers of given RVAs in GameAssembly.dll (byte scan)."""
import struct, sys

DLL = r"C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll"
data = open(DLL, "rb").read()
pe = struct.unpack_from("<I", data, 0x3C)[0]
nsec = struct.unpack_from("<H", data, pe + 6)[0]
opt = struct.unpack_from("<H", data, pe + 20)[0]
sec = pe + 24 + opt
targets = [int(a, 16) for a in sys.argv[1:]]
hits = {t: [] for t in targets}
for i in range(nsec):
    name, vsize, va, rsize, raw = struct.unpack_from("<8sIIII", data, sec + 40 * i)
    if not (name.startswith(b".text") or name.startswith(b"il2cpp")):
        continue
    blob = data[raw:raw + rsize]
    for off in range(len(blob) - 5):
        op = blob[off]
        if op not in (0xE8, 0xE9):
            continue
        rel = struct.unpack_from("<i", blob, off + 1)[0]
        dest = va + off + 5 + rel
        if dest in hits:
            hits[dest].append(va + off)
for t in targets:
    print(f"{t:#x}: {len(hits[t])} direct call/jmp sites: " + ", ".join(f"{h:#x}" for h in hits[t]))
