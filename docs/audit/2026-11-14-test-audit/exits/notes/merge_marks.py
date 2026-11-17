"""Merge the E5 marks into one file. Usage: py -I merge_marks.py <AUD>
Each pulled note takes its last mark made from the file: R09 over R01..R08 over N01..N09. New rows
(notes the script missed) come from each N batch's second table. Writes exits/notes/marks.tsv and
exits/notes/new-rows.tsv, and fails loudly when a table does not equal its batch."""
import csv, os, re, sys
from collections import Counter
AUD = sys.argv[1]
csv.field_size_limit(10**9)
N = os.path.join(AUD, 'exits', 'notes')
MARKS = {'repeat', 'split', 'prod', 'finding', 'empty', 'not read'}

def tables(path):
    """Every markdown table in the file, as (header, rows)."""
    out, cur, head = [], None, None
    for ln in open(path, encoding='utf-8'):
        if ln.startswith('|'):
            c = [x.strip() for x in ln.strip().strip('|').split('|')]
            if cur is None:
                head, cur = c, []
            elif re.match(r'^\|\s*-', ln):
                continue
            else:
                cur.append(c)
        elif cur is not None:
            out.append((head, cur)); cur = None
    if cur is not None:
        out.append((head, cur))
    return out

def first(path, batch):
    t = [r for h, r in tables(path) if h[:2] == ['note', 'mark']]
    if not t:
        raise SystemExit(path + ': no note/mark table')
    rows = [(c[0].strip('`'), c[1].strip('`'), ' | '.join(c[2:])) for c in t[0]]
    if [r[0] for r in rows] != [b['note'] for b in batch]:
        raise SystemExit('%s: table does not equal its batch' % path)
    bad = {m for _, m, _ in rows if m not in MARKS}
    if bad:
        raise SystemExit('%s: unknown marks %s' % (path, bad))
    return rows

def load(p):
    return list(csv.DictReader(open(p, encoding='utf-8'), delimiter='\t'))

final, new = {}, []
order = []
for i in range(1, 10):
    b = 'N%02d' % i
    batch = load(os.path.join(N, 'batches', b + '.tsv'))
    for k, (row, (note, mark, why)) in enumerate(zip(batch, first(os.path.join(N, b + '.md'), batch))):
        key = (b, k)
        order.append(key)
        final[key] = dict(row, mark=mark, why=why, marked_by=b)
    for h, rs in tables(os.path.join(N, b + '.md')):
        if h and h[0].startswith('file:line'):
            for c in rs:
                new.append({'batch': b, 'file_line': c[0].strip('`'), 'mark': c[1].strip('`'), 'why': ' | '.join(c[2:])})
# Map each re-mark row back to its N row by note and occurrence, in N order.
slots = {}
for key in order:
    slots.setdefault((final[key]['note'], final[key]['marked_by']), []).append(key)
for i in range(1, 10):
    b = 'R%02d' % i
    p = os.path.join(N, 'remark', b + '.md')
    if not os.path.exists(p):
        print(b, 'not written yet'); continue
    batch = load(os.path.join(N, 'remark', b + '.tsv'))
    used = Counter()
    for row, (note, mark, why) in zip(batch, first(p, batch)):
        src = row['from_batch']
        ks = slots[(note, src)]
        key = ks[used[(note, src)]]
        used[(note, src)] += 1
        # A later re-mark wins, but "not read" never replaces a mark made from the file.
        if mark == 'not read' and final[key]['marked_by'].startswith('R'):
            continue
        final[key].update(mark=mark, why=why, marked_by=b)
cols = ['note', 'kind', 'ledger_id', 'record_heading', 'mark', 'marked_by', 'why']
with open(os.path.join(N, 'marks.tsv'), 'w', encoding='utf-8', newline='') as fh:
    w = csv.writer(fh, delimiter='\t', lineterminator='\n')
    w.writerow(cols)
    for key in order:
        w.writerow([str(final[key][c]).replace('\t', ' ').replace('\n', ' ') for c in cols])
with open(os.path.join(N, 'new-rows.tsv'), 'w', encoding='utf-8', newline='') as fh:
    w = csv.DictWriter(fh, fieldnames=['batch', 'file_line', 'mark', 'why'], delimiter='\t', lineterminator='\n')
    w.writeheader(); w.writerows(new)
print(len(order), 'notes', dict(Counter(final[k]['mark'] for k in order)))
print('marked by', dict(Counter(final[k]['marked_by'][0] for k in order)))
print(len(new), 'new rows', dict(Counter(r['mark'] for r in new)))
