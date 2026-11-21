"""Pick the E5 notes whose marks were not made from the file, and batch them by file.
Usage: py -I make_remark.py <AUD>
A row is re-marked unless its scout opened it in its file: N09 (all read whole), N04's non-empty
rows (re-read in their files), and N07's rows outside the seven files it did not read through."""
import csv, os, sys
from collections import OrderedDict
AUD = sys.argv[1]
csv.field_size_limit(10**9)
N = os.path.join(AUD, 'exits', 'notes')
N07_UNREAD = {'records/window-WN4c.md', 'records/window-WN5.md', 'records/window-WN8.md',
              'sweep/SW1.md', 'sweep/SW2.md', 'sweep/SW3.md', 'sweep/SW4.md'}

def first_table(path):
    rows, on = [], False
    for ln in open(path, encoding='utf-8'):
        if ln.startswith('| note | mark'):
            on = True
            continue
        if on:
            if ln.startswith('|---') or ln.startswith('| ---'):
                continue
            if not ln.startswith('|'):
                if rows:
                    break
                continue
            c = [x.strip() for x in ln.strip().strip('|').split('|')]
            rows.append((c[0].strip('`'), c[1].strip('`'), ' | '.join(c[2:])))
    return rows

pick = []
for i in range(1, 10):
    b = 'N%02d' % i
    batch = list(csv.DictReader(open(os.path.join(N, 'batches', b + '.tsv'), encoding='utf-8'), delimiter='\t'))
    marks = first_table(os.path.join(N, b + '.md'))
    if [m[0] for m in marks] != [r['note'] for r in batch]:
        raise SystemExit('%s: first table does not equal the batch (%d vs %d rows)' % (b, len(marks), len(batch)))
    for r, (note, mark, why) in zip(batch, marks):
        f = note.rsplit(':', 1)[0]
        if b == 'N09':
            continue
        if b == 'N04' and mark != 'empty':
            continue
        if b == 'N07' and f not in N07_UNREAD:
            continue
        pick.append(dict(r, from_batch=b, old_mark=mark, old_note=why))
by_file = OrderedDict()
for r in sorted(pick, key=lambda r: (r['note'].rsplit(':', 1)[0], int(r['note'].rsplit(':', 1)[1]))):
    by_file.setdefault(r['note'].rsplit(':', 1)[0], []).append(r)
CAP = 190
out, cur, n = [], [], 0
for f, rs in by_file.items():
    if cur and n + len(rs) > CAP:
        out.append(cur); cur, n = [], 0
    cur += rs; n += len(rs)
if cur:
    out.append(cur)
d = os.path.join(N, 'remark')
os.makedirs(d, exist_ok=True)
cols = ['note', 'kind', 'ledger_id', 'record_heading', 'from_batch', 'old_mark', 'old_note', 'text']
for i, b in enumerate(out, start=1):
    name = 'R%02d' % i
    with open(os.path.join(d, name + '.tsv'), 'w', encoding='utf-8', newline='') as fh:
        w = csv.writer(fh, delimiter='\t', lineterminator='\n')
        w.writerow(cols)
        for r in b:
            w.writerow([str(r[c]).replace('\t', ' ').replace('\n', ' ') for c in cols])
    files = sorted({r['note'].rsplit(':', 1)[0] for r in b})
    print(name, len(b), 'rows', len(files), 'files')
print('total', len(pick))
