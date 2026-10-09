import sys, struct
sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
sys.path.insert(0, r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp")
import chdis
base = 0x210d7be  # the lea rdx target: rip-relative [0x0] means rdx = image base (RVA 0)
off = chdis.rva2off(0x210d808)
targets = [struct.unpack_from("<i", chdis.data, off + 4*i)[0] for i in range(7)]
for i, t in enumerate(targets):
    print(f"case {i}: jump to RVA {t:#x}")
off2 = chdis.rva2off(0x213db10)
print("chart N0..5 table:", [hex(struct.unpack_from('<i', chdis.data, off2 + 4*i)[0]) for i in range(6)])
