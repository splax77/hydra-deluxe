"""Print a jump table: py -E jt.py <il2cpp dir> <table rva hex> <count>  (entries are int32 rva offsets from image base)"""
import sys, struct
sys.stdout.reconfigure(encoding="utf-8")
sys.path.insert(0, sys.argv[1])
from chdis import data, rva2off
t = int(sys.argv[2], 16); n = int(sys.argv[3])
off = rva2off(t)
for i in range(n):
    v = struct.unpack_from("<i", data, off + 4 * i)[0]
    print(i, hex(v))
