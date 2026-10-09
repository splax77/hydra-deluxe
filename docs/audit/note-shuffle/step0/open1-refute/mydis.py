"""Independent re-disassembly wrapper. py -I mydis.py <rva hex> [maxbytes hex]."""
import sys, os
sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
sys.path.insert(0, r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp")
os.chdir(sys.path[0])
sys.stdout.reconfigure(encoding="utf-8")
import chdis
chdis.dis(int(sys.argv[1], 16), int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x1200)
