"""Wrapper: py -I myref.py xref <rva> | callers <rva>... (adds paths lost under -I)."""
import sys, os
sys.path.insert(0, r"C:\Users\Patrick\AppData\Roaming\Python\Python314\site-packages")
IL = r"C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\9168b711-0081-4bbb-9a2b-20ca0ba79902\scratchpad\il2cpp"
sys.path.insert(0, IL)
os.chdir(IL)
sys.stdout.reconfigure(encoding="utf-8")
mode = sys.argv[1]
sys.argv = ["x"] + sys.argv[2:]
exec(open(os.path.join(IL, mode + ".py"), encoding="utf-8").read())
