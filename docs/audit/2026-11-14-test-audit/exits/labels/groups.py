"""Build exits/groups.md body from merged.tsv and the scouts' ends tables.
Usage: py -I groups.py <AUD> <merged.tsv> <out.md-body> <labels-out.tsv>
For each key: rows by kind, and one plainest and one least-fitting row. Both come from the scouts'
ends tables, kept only where the row still carries the key after the verifiers' changes; the pick is
from the batch holding the most rows of that key. Fails loudly on an ends cell naming no batch row."""
import csv, glob, os, re, sys
from collections import Counter, defaultdict
AUD, MERGED, OUT, LOUT = sys.argv[1:5]
csv.field_size_limit(10**9)
LAB = os.path.join(AUD, 'exits', 'labels')
m = list(csv.DictReader(open(MERGED, encoding='utf-8'), delimiter='\t'))
keys_of = {(r['batch'], r['row']): set(k for k in r['keys'].split(', ') if k) for r in m}
per_batch = defaultdict(Counter)
by_key = defaultdict(lambda: Counter())
for r in m:
    for k in keys_of[(r['batch'], r['row'])]:
        per_batch[k][r['batch']] += 1
        by_key[k][r['kind']] += 1
ROW = re.compile(r'((?:records|counts|capture)/[\w./-]+\.md:\d+)')
ends = defaultdict(lambda: {'plain': [], 'least': []})
dropped = []
for p in sorted(glob.glob(os.path.join(LAB, 'L-*.md'))):
    b = os.path.basename(p)[:-3]
    on = False
    for ln in open(p, encoding='utf-8'):
        if ln.startswith('| key | plainest'):
            on = True
            continue
        if on:
            if ln.startswith('|---') or ln.startswith('| ---'):
                continue
            if not ln.startswith('|'):
                break
            c = [x.strip() for x in ln.strip().strip('|').split('|')]
            if len(c) < 3:
                continue
            k = c[0].strip('`')
            for slot, cell in (('plain', c[1]), ('least', c[2])):
                for row in ROW.findall(cell):
                    if (b, row) not in keys_of:
                        raise SystemExit('%s ends names %s, not in the batch' % (b, row))
                    if k in keys_of[(b, row)]:
                        ends[k][slot].append((b, row))
                    else:
                        dropped.append((b, k, slot, row))
order = [l.split('|')[1].strip() for l in open(os.path.join(AUD, 'exits', 'keys.md'), encoding='utf-8')
         if re.match(r'^\| [A-Z]-[a-z]+ \|', l) and not l.startswith('| R-')]
lines = []
for k in order:
    tot = sum(by_key[k].values())
    def pick(slot):
        cands = ends[k][slot]
        if not cands:
            return 'none named'
        best = max(cands, key=lambda br: (per_batch[k][br[0]], br[0]))
        more = len(set(cands)) - 1
        return '`%s` (%s)%s' % (best[1], best[0], '; %d more named' % more if more else '')
    lines.append('| %s | %d | %d | %d | %s | %s |' % (k, by_key[k]['record'], by_key[k]['count'], tot,
                                                       pick('plain'), pick('least')))
nk = [r for r in m if not r['keys']]
with open(OUT, 'w', encoding='utf-8', newline='\n') as fh:
    fh.write('| Key | Records | Count rows | All rows | Plainest row | Least-fitting row |\n|---|---|---|---|---|---|\n')
    fh.write('\n'.join(lines) + '\n')
with open(LOUT, 'w', encoding='utf-8', newline='') as fh:
    w = csv.writer(fh, delimiter='\t', lineterminator='\n')
    w.writerow(['batch', 'kind', 'row', 'keys', 'no_key', 'source'])
    for r in m:
        w.writerow([r['batch'], r['kind'], r['row'], r['keys'], r['no_key'], r['source']])
print('keys', len(order), 'no-key rows', len(nk), 'by kind', Counter(r['kind'] for r in nk))
print('ends cells dropped (row lost its key):', len(dropped))
print('keys with no plainest:', [k for k in order if not ends[k]['plain']])
print('keys with no least:', [k for k in order if not ends[k]['least']])
