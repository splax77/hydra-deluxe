"""Cut the faulty rows into executor chunks of about CAP rows each.

The user's answers on 2026-11-18 (session 8e6a2908): "raise the executor cap to 16. smaller
branches."; rubric-only rows "Yes, fix them too"; tests that cannot be made real are deleted
("Delete what can't be made real"). So every row in row-files.tsv is now actionable.

A chunk holds whole files, packed together until it reaches CAP rows. A file with more than
CAP rows is cut into parts that run one after another (part n+1 starts after part n merges),
because two branches never edit one file at once. Rows that name no real test file go in
mop-up chunks by audit file.

Usage: chunks.py <row-files.tsv> <out.tsv> [CAP]
"""
import csv, sys, collections

src, out = sys.argv[1], sys.argv[2]
CAP = int(sys.argv[3]) if len(sys.argv) > 3 else 15
BAD = {'unresolved', 'notinrepo', 'ambiguous'}

def is_test(f):
    return ('/tests/' in f or f.startswith('tests/') or '/test_' in f or f.startswith('test_')
            or f.rsplit('/', 1)[-1].startswith('test_') or '/ui/uitest' in f or 'report_samples' in f)

rows = list(csv.DictReader(open(src, encoding='utf-8'), delimiter='\t'))
by_file = collections.OrderedDict()
mop = collections.OrderedDict()
for r in rows:
    if r['how'] in BAD or not is_test(r['test_file']):
        mop.setdefault(r['row'].rsplit(':', 1)[0], []).append(r)
    else:
        by_file.setdefault(r['test_file'], []).append(r)

chunks = []  # (id, kind, part, files, rows)
n = 0
bundle, bundle_rows = [], []
def flush():
    global n, bundle, bundle_rows
    if bundle:
        n += 1
        chunks.append((f'C{n:03d}', 'files', '1/1', list(bundle), list(bundle_rows)))
        bundle, bundle_rows = [], []

# big files first, each cut into sequential parts
for f, rs in sorted(by_file.items(), key=lambda kv: -len(kv[1])):
    if len(rs) > CAP:
        parts = (len(rs) + CAP - 1) // CAP
        size = (len(rs) + parts - 1) // parts
        for p in range(parts):
            n += 1
            chunks.append((f'C{n:03d}', 'file-part', f'{p+1}/{parts}', [f], rs[p*size:(p+1)*size]))
    else:
        if len(bundle_rows) + len(rs) > CAP:
            flush()
        bundle.append(f); bundle_rows += rs
flush()
for audit, rs in mop.items():
    for i in range(0, len(rs), CAP):
        n += 1
        chunks.append((f'C{n:03d}', 'mop-up', f'{i//CAP+1}/{(len(rs)+CAP-1)//CAP}', [audit], rs[i:i+CAP]))

with open(out, 'w', encoding='utf-8', newline='') as fh:
    w = csv.writer(fh, delimiter='\t', lineterminator='\n')
    w.writerow(['chunk', 'kind', 'part', 'files', 'rows', 'row_ids'])
    for cid, kind, part, files, rs in chunks:
        w.writerow([cid, kind, part, ' '.join(files), len(rs), ' '.join(r['row'] for r in rs)])
total = sum(len(c[4]) for c in chunks)
print(f'{len(chunks)} chunks, {total} rows (of {len(rows)}); kinds:',
      dict(collections.Counter(c[1] for c in chunks)))
