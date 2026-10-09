"""Print a jump table of dword RVAs. usage: py -P -E jt.py <table rva hex> <count>"""
import sys, struct, os
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
os.chdir(IL)
from chdis import data, rva2off
t = int(sys.argv[1], 16)
n = int(sys.argv[2])
off = rva2off(t)
for i in range(n):
    print(i, hex(struct.unpack_from("<I", data, off + 4 * i)[0]))
