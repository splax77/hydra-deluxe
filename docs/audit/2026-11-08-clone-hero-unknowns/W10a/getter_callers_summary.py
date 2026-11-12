r"""Sum up xref_note_getters.txt: per getter, how many direct call/jmp sites,
in how many methods, and the callers whose class or method name is readable
(ASCII letters), up to five.

How to run it: C:\Python314\python.exe -I getter_callers_summary.py xref_note_getters.txt
Its output is getter_callers_summary.txt beside this file.
"""
import re, sys

sys.stdout.reconfigure(encoding="utf-8")
cur = None
rows = {}
order = []
for line in open(sys.argv[1], encoding="utf-8"):
    m = re.match(r"== target (\w+)", line)
    if m:
        cur = m.group(1)
        rows[cur] = []
        order.append(cur)
        continue
    m = re.match(r"\s+(CALL|JMP|RIPREF|PTR)\s+at (\w+) \[[^\]]*\] in (\w+) (.*)", line)
    if m and m.group(1) in ("CALL", "JMP"):
        rows[cur].append((m.group(3), m.group(4).strip()))
for t in order:
    meths = dict(rows[t])
    named = [f"{a} {n}" for a, n in meths.items() if re.search(r"[A-Za-z]{3}", n)][:5]
    print(f"{t}: {len(rows[t])} sites in {len(meths)} methods; named: {'; '.join(named) or 'none'}")
