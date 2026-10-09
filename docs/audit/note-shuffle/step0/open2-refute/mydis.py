"""Skeptic wrapper for py -I: adds site-packages and the il2cpp folder, then runs chdis.
usage: py -I mydis.py <rva hex> [maxbytes hex]
"""
import sys, os
sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
os.chdir(IL)
sys.stdout.reconfigure(encoding="utf-8")
import chdis
chdis.dis(int(sys.argv[1], 16), int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x1200)
