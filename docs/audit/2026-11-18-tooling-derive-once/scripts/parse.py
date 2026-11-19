import re, sys, os
from collections import Counter

D = sys.argv[1]
KINDS = r'doc-vs-code|doc-vs-doc|memory-vs-other|moved-pointer|code-vs-code|ruling-vs-other|claimed-ruling|homeless-rule'
rows = []
for n in range(1, 7):
    t = open(os.path.join(D, f"reports/TOOL-AUDIT-T8-R{n}.md"), encoding="utf-8").read()
    for sec in re.split(r'(?m)^### ', t)[1:]:
        head = sec.splitlines()[0]
        ids = re.findall(r'L-F\d+-\d+', head)
        if not ids:
            continue
        v = re.search(r'(?mi)^\*\*Verdict:?\*\*:?\s*(.*)', sec)
        v = v.group(1).strip() if v else '?'
        k = re.search(r'(?mi)^\*\*Kind:?\*\*:?\s*(.*)', sec)
        k = k.group(1).strip() if k else ''
        base = re.match(r'(REAL|GONE|NOT A SPLIT)', v, re.I)
        base = base.group(1).upper() if base else '?'
        kinds = sorted(set(re.findall(KINDS, k)))
        for i in ids:
            rows.append((i, f"R{n}", base, ";".join(kinds), v[:80], k[:400]))
print("rows", len(rows), "unique", len({r[0] for r in rows}), file=sys.stderr)
print(Counter(r[2] for r in rows), file=sys.stderr)
for r in rows:
    print("\t".join(r))
