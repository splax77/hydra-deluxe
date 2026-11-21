"""List literals that appear in test files' assertion lines, grouped by value,
kept when the value appears in 3+ files. Usage: py -I repeats.py <repo> <out.tsv>"""
import os, re, sys
from collections import defaultdict

repo, out = sys.argv[1], sys.argv[2]
ASSERT = re.compile(r'\b(CHECK|REQUIRE|WARN|CHECK_EQ|REQUIRE_EQ|CHECK_MESSAGE|IM_CHECK\w*|assert\w*|self\.assert\w*|Check)\b|==|!=')
NUM = re.compile(r'(?<![\w.])-?\d[\d,]*\.?\d*(?![\w])')
STR = re.compile(r'"((?:[^"\\]|\\.){3,})"')
TRIVIAL = {'0', '1', '2', '3', '4', '5', '10', '100', '0.0', '1.0', '-1'}
hits = defaultdict(list)   # value -> [(file, line)]
for root, _, files in os.walk(os.path.join(repo, 'tests')):
    for f in files:
        if not f.endswith(('.cpp', '.h', '.py')):
            continue
        p = os.path.join(root, f)
        rel = os.path.relpath(p, repo).replace('\\', '/')
        try:
            lines = open(p, encoding='utf-8', errors='replace').read().splitlines()
        except OSError:
            continue
        for i, line in enumerate(lines, 1):
            s = line.strip()
            if s.startswith('//') or s.startswith('#'):
                continue
            consts = s.startswith(('inline constexpr', 'constexpr', 'static constexpr', 'const '))
            if not (ASSERT.search(s) or consts):
                continue
            for m in NUM.finditer(s):
                v = m.group(0).replace(',', '')
                if v.lstrip('-') in TRIVIAL or len(v.lstrip('-').replace('.', '')) < 3:
                    continue
                hits['num:' + v].append((rel, i))
            for m in STR.finditer(s):
                v = m.group(1)
                if not re.search(r'\d', v) and len(v) < 6:
                    continue
                hits['str:' + v].append((rel, i))
rows = []
for v, occ in hits.items():
    files = sorted({f for f, _ in occ})
    if len(files) >= 3:
        rows.append((len(files), len(occ), v, files, occ))
rows.sort(key=lambda r: (-r[0], -r[1]))
with open(out, 'w', encoding='utf-8', newline='\n') as fh:
    fh.write('files\tuses\tvalue\tplaces\n')
    for nf, nu, v, files, occ in rows:
        fh.write(f"{nf}\t{nu}\t{v}\t" + '; '.join(f'{f}:{l}' for f, l in occ) + '\n')
print(len(rows), 'values in 3+ files')
for r in rows[:40]:
    print(r[0], r[1], r[2][:70])
