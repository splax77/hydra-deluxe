"""Compare the section table's Add pairs in the static-constructor listing with Hydra's section-name TSV.

Run from the repo root: py docs/audit/2026-11-08-clone-hero-unknowns/main/cr30_section_table_compare.py .
Writes to stdout only.
"""
import re, sys
root = sys.argv[1]
lst = open(root + "/docs/clone-hero/code/6142/section_table_cctor_2095040.asm", encoding="utf-8").read().split("\n")
pairs = []
r8 = rdx = None
for i, l in enumerate(lst, 1):
    if i < 10876:
        continue
    m = re.search(r"str:(.*)$", l)
    if "mov    r8, qword ptr [rip" in l and m:
        r8 = m.group(1)
    elif "mov    rdx, qword ptr [rip" in l and m:
        rdx = m.group(1)
    elif "call   0x182c503e0" in l:
        pairs.append((rdx, r8)); r8 = rdx = None

def unq(s):
    s = s.strip()
    if s.startswith("'") and s.endswith("'"):
        return s[1:-1]
    if s.startswith('"') and s.endswith('"'):
        return s[1:-1]
    return s

pairs = [(unq(a), unq(b)) for a, b in pairs]
tsv = [l.rstrip("\n").split("\t") for l in open(root + "/data/clone-hero-1.1.0.6142-section-names.tsv", encoding="utf-8") if not l.startswith("#")]
tsv = [(a, b) for a, b in tsv if a or b]
print("listing pairs", len(pairs), "tsv rows", len(tsv))
bad = [(i, p, t) for i, (p, t) in enumerate(zip(pairs, tsv)) if p != tuple(t)]
print("mismatches", len(bad))
for x in bad[:8]:
    print(x)
print("dup keys in listing", len(pairs) - len(set(a for a, _ in pairs)))
