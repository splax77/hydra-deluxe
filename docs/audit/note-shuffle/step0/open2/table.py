import sys
sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
sys.path.insert(0, r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp")
import chdis
off = chdis.rva2off(0x210d824)
tbl = chdis.data[off:off+43]
cases = {0:13,1:14,2:15,3:16,4:18,5:17,6:0}
for i,b in enumerate(tbl):
    print(f"midi {59+i:3d} -> case {b} -> type {cases.get(b,'?')}")
