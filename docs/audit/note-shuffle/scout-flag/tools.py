"""Scout helpers over the earlier il2cpp dump (imports chdis from there).
usage:
  py -P -E tools.py callers <rva> [<rva>...]   direct call/jmp sites
  py -P -E tools.py xref <rva>                 RIP-relative references
  py -P -E tools.py dis <rva> [maxbytes]       chdis listing, utf-8 safe
"""
import sys, struct, bisect, os
sys.stdout.reconfigure(encoding="utf-8")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
os.chdir(IL)
import chdis
from chdis import data, secs, names

addrs = sorted(names)


def owner(rva):
    k = bisect.bisect_right(addrs, rva) - 1
    return addrs[k]


cmd = sys.argv[1]
if cmd == "callers":
    targets = {int(a, 16) for a in sys.argv[2:]}
    for name, va, vsize, raw, rsize in secs:
        if name not in (".text", "il2cpp"):
            continue
        blob = data[raw:raw + rsize]
        for i in range(len(blob) - 5):
            if blob[i] not in (0xE8, 0xE9):
                continue
            disp = struct.unpack_from("<i", blob, i + 1)[0]
            t = va + i + 5 + disp
            if t in targets:
                rva = va + i
                f = owner(rva)
                print(f"{t:#x} <- {rva:#x} in {f:#x} {names[f]}")
elif cmd == "xref":
    target = int(sys.argv[2], 16)
    for name, va, vsize, raw, rsize in secs:
        if name not in (".text", "il2cpp"):
            continue
        blob = data[raw:raw + rsize]
        for i in range(len(blob) - 4):
            disp = struct.unpack_from("<i", blob, i)[0]
            if va + i + 4 + disp == target:
                rva = va + i
                f = owner(rva)
                print(f"{rva:#x} in {f:#x} {names[f]}")
elif cmd == "dis":
    chdis.dis(int(sys.argv[2], 16), int(sys.argv[3], 16) if len(sys.argv) > 3 else 0x1200)
