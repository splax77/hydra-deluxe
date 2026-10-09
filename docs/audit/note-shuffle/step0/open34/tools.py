"""Wrapper around the il2cpp scratch helpers so they run under `py -I`.

usage:
  py -I tools.py dis <rva hex> [max_bytes hex]   -> prints listing
  py -I tools.py callers <rva hex>...            -> direct call/jmp sites
  py -I tools.py xref <rva hex>                  -> RIP-relative references
  py -I tools.py name <rva hex>                  -> function name containing rva
"""
import sys, os, struct, pickle, bisect
sys.stdout.reconfigure(encoding="utf-8")
IL2 = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL2)
# capstone lives in the user site-packages, which `py -I` leaves off sys.path.
sys.path.append(r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
os.chdir(IL2)
import chdis
from chdis import data, secs, BASE, rva2off, names, meta
addrs = sorted(names)


def owner(rva):
    k = bisect.bisect_right(addrs, rva) - 1
    return addrs[k], names[addrs[k]]


def callers(targets):
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
                f, n = owner(rva)
                print(f"{t:#x} <- {rva:#x} ({'call' if blob[i]==0xE8 else 'jmp'}) in {f:#x} {n}")


def xref(target):
    for name, va, vsize, raw, rsize in secs:
        if name not in (".text", "il2cpp"):
            continue
        blob = data[raw:raw + rsize]
        for i in range(len(blob) - 4):
            disp = struct.unpack_from("<i", blob, i)[0]
            if va + i + 4 + disp == target:
                rva = va + i
                f, n = owner(rva)
                print(f"{rva:#x} in {f:#x} {n}")


def vcalls(disp32):
    """Sites of `call qword ptr [reg + disp32]` (FF /2 with mod=10) for one vtable offset."""
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    md.detail = True
    needle = struct.pack("<i", disp32)
    for name, va, vsize, raw, rsize in secs:
        if name not in (".text", "il2cpp"):
            continue
        blob = data[raw:raw + rsize]
        i = blob.find(needle)
        while i != -1:
            # expect FF <modrm> [sib] disp32 ; modrm mod=10 reg=/2 (call)
            for back in (2, 3):
                j = i - back
                if j >= 0 and blob[j] == 0xFF and (blob[j + 1] & 0xF8) == 0x90 and (back == 2 or (blob[j + 1] & 7) == 4):
                    ins = next(md.disasm(blob[j:j + 8], BASE + va + j), None)
                    if ins and ins.mnemonic == "call" and ins.address - BASE == va + j:
                        f, n = owner(va + j)
                        print(f"{va + j:#x}: {ins.mnemonic} {ins.op_str}  in {f:#x} {n}")
            i = blob.find(needle, i + 1)


if __name__ == "__main__":
    cmd = sys.argv[1]
    if cmd == "vcalls":
        vcalls(int(sys.argv[2], 16))
    if cmd == "dis":
        chdis.dis(int(sys.argv[2], 16), int(sys.argv[3], 16) if len(sys.argv) > 3 else 0x1200)
    elif cmd == "callers":
        callers({int(a, 16) for a in sys.argv[2:]})
    elif cmd == "xref":
        xref(int(sys.argv[2], 16))
    elif cmd == "name":
        for a in sys.argv[2:]:
            f, n = owner(int(a, 16))
            print(f"{int(a,16):#x} in {f:#x} {n}")
