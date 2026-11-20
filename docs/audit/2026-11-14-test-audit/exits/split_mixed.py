"""Split each chunk that names both hook files and Hydra files into one chunk per repo.

Usage: split_mixed.py <row-files.tsv> <chunks.tsv>

A hook file is one under ~/.claude/. The Hydra half keeps the old id. The hook half takes the next
free id and goes on the line after the Hydra half, so plan order stays. Rows whose hook file is gone
(named in GONE) go to a chunk of their own after that, the way C183 and C184 hold the rows of a
deleted test file; waves.tsv gives that chunk no wave. The file is rewritten in place.
"""
import csv, sys

rf, chunks = sys.argv[1:3]
GONE = {'~/.claude/gates/tests/test_agent_files.py', '~/.claude/gates/tests/test_docs_numbers.py'}
BAD = {'unresolved', 'notinrepo', 'ambiguous'}


def is_test(f):  # the same test as check_chunks.py, which owns it
    return ('/tests/' in f or f.startswith('tests/') or '/test_' in f or f.startswith('test_')
            or f.rsplit('/', 1)[-1].startswith('test_') or '/ui/uitest' in f or 'report_samples' in f)


def key(r):
    return r['row'].rsplit(':', 1)[0] if r['how'] in BAD or not is_test(r['test_file']) else r['test_file']


R = {r['row']: r for r in csv.DictReader(open(rf, encoding='utf-8'), delimiter='\t')}
rd = csv.DictReader(open(chunks, encoding='utf-8'), delimiter='\t')
cols, C = rd.fieldnames, list(rd)
nxt = max(int(c['chunk'][1:]) for c in C) + 1
out = []


def part(c, rows):
    files = []
    for rid in rows:
        if key(R[rid]) not in files:
            files.append(key(R[rid]))
    order = {f: i for i, f in enumerate(c['files'].split())}
    files.sort(key=lambda f: order[f])
    return dict(c, files=' '.join(files), rows=str(len(rows)), row_ids=' '.join(rows), part='1/1')


for c in C:
    fs = c['files'].split()
    hook = [f for f in fs if f.startswith('~/.claude/')]
    if not hook or len(hook) == len(fs) or c['kind'] == 'mop-up':
        out.append(c)
        continue
    ids = c['row_ids'].split()
    groups = [[r for r in ids if not key(R[r]).startswith('~/.claude/')],
              [r for r in ids if key(R[r]).startswith('~/.claude/') and key(R[r]) not in GONE],
              [r for r in ids if key(R[r]) in GONE]]
    out.append(part(c, groups[0]))
    for g in groups[1:]:
        if g:
            out.append(part(dict(c, chunk='C%03d' % nxt), g))
            print('%s -> %s: %s rows, %s' % (c['chunk'], 'C%03d' % nxt, len(g), out[-1]['files']))
            nxt += 1
    print('%s keeps %d rows, %s' % (c['chunk'], len(groups[0]), out[-1 - sum(1 for g in groups[1:] if g)]['files']))

w = csv.DictWriter(open(chunks, 'w', encoding='utf-8', newline=''), cols, delimiter='\t', lineterminator='\n')
w.writeheader()
w.writerows(out)
