"""Merge E3 scout labels with verifier changes. Usage: py -I merge_labels.py <AUD> <out.tsv>
Each batch row's keys: the verifier's 'your keys' where it changed the row, else the scout's.
Fails loudly when a batch row has no label line, or a change names a row not in its batch."""
import csv, glob, os, re, sys
AUD, OUT = sys.argv[1:3]
csv.field_size_limit(10**9)
LAB = os.path.join(AUD, 'exits', 'labels')
keys_md = open(os.path.join(AUD, 'exits', 'keys.md'), encoding='utf-8').read()
KEYS = sorted(set(k for k in re.findall(r'^\| ([A-Z]-[a-z]+) \|', keys_md, re.M) if not k.startswith('R-')), key=len, reverse=True)
if len(KEYS) != 53:
    raise SystemExit('keys.md holds %d keys, not 53' % len(KEYS))
KEY_RE = re.compile(r'(?<![A-Za-z-])(' + '|'.join(re.escape(k) for k in KEYS) + r')(?![a-z])')

def parse(cell):
    ks = []
    for k in KEY_RE.findall(cell):
        if k not in ks:
            ks.append(k)
    nk = 'no key' in cell.lower()
    return ks, nk

def table(path, first):
    rows = []
    on = False
    for ln in open(path, encoding='utf-8'):
        if ln.startswith(first):
            on = True
            continue
        if on:
            if ln.startswith('|---') or ln.startswith('| ---'):
                continue
            if not ln.startswith('|'):
                if rows:
                    break
                continue
            rows.append([c.strip() for c in ln.strip().strip('|').split('|')])
    return rows

batches = {}
for p in sorted(glob.glob(os.path.join(LAB, 'batches', 'L-*.tsv'))):
    b = os.path.basename(p)[:-4]
    batches[b] = list(csv.DictReader(open(p, encoding='utf-8'), delimiter='\t'))

labels = {}
notes = {}
for b, rs in batches.items():
    lab = {}
    for c in table(os.path.join(LAB, b + '.md'), '| row | keys |'):
        lab[c[0].strip('`')] = c[1]
        notes[(b, c[0].strip('`'))] = c[2] if len(c) > 2 else ''
    for r in rs:
        if r['row'] not in lab:
            raise SystemExit('%s row %s has no label line' % (b, r['row']))
    labels[b] = lab

vfiles = [p for p in glob.glob(os.path.join(LAB, 'V*.md')) if not os.path.basename(p).startswith('V-L-R02-L-R03')]
changes = {}
for p in sorted(vfiles):
    for c in table(p, "| batch | row | scout's keys |"):
        b, row, theirs = c[0].strip('`'), c[1].strip('`'), c[3]
        if b not in batches:
            raise SystemExit('%s names batch %s' % (os.path.basename(p), b))
        if row not in labels[b]:
            raise SystemExit('%s changes %s %s, not in the batch' % (os.path.basename(p), b, row))
        changes[(b, row)] = (theirs, os.path.basename(p))
        notes[(b, row)] = c[4] if len(c) > 4 else notes.get((b, row), '')

REV1 = {'counts/T5-IR10.md:59': 'O-toolout', 'records/D1.md:606': 'K-duplicate', 'records/D10.md:470': 'K-duplicate',
        'records/D10.md:385': 'K-namebody', 'records/A10.md:617': 'K-partial', 'records/D11.md:445': 'K-nocode'}

out = []
for b, rs in batches.items():
    for r in rs:
        row = r['row']
        src = 'scout'
        cell = labels[b][row]
        if (b, row) in changes:
            cell, src = changes[(b, row)]
        ks, nk = parse(cell)
        if row in REV1 and REV1[row] not in ks and not ks:
            ks, nk, src = [REV1[row]], False, src + '+rev1'
        out.append({'batch': b, 'kind': r['kind'], 'row': row, 'record': r.get('record', ''),
                    'keys': ', '.join(ks), 'no_key': 'yes' if (nk or not ks) else '',
                    'source': src, 'cell': cell, 'note': notes.get((b, row), '')})
with open(OUT, 'w', encoding='utf-8', newline='') as fh:
    w = csv.DictWriter(fh, fieldnames=list(out[0]), delimiter='\t', lineterminator='\n')
    w.writeheader()
    w.writerows(out)
from collections import Counter
cnt = Counter(k for o in out for k in o['keys'].split(', ') if k)
print(len(out), 'rows;', sum(1 for o in out if not o['keys']), 'with no key at all;',
      sum(1 for o in out if o['keys'] and o['no_key']), 'keyed but also a no-key part;', len(changes), 'changes')
for k in KEYS[::-1]:
    pass
print(sorted(cnt.items(), key=lambda kv: -kv[1]))
print('unused keys:', sorted(set(KEYS) - set(cnt)))
