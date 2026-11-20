"""Check chunks.tsv against row-files.tsv and against an earlier chunks.tsv.

Usage: check_chunks.py <row-files.tsv> <chunks.tsv> <earlier chunks.tsv> <repo>

It checks that every row is in exactly one chunk, that each chunk's rows count matches its row_ids,
that each row's chunk names the row's file, that no done chunk (its claude/tf-<chunk> branch merged
into main) lost or gained a row except T5-KR1 rows 45-52 going from C191 to C192, and that a file in
more than one open chunk is only in single-file parts numbered 1..m that run one after another.
"""
import csv, sys, subprocess, collections

rf, chunks, earlier, repo = sys.argv[1:5]
BAD = {'unresolved', 'notinrepo', 'ambiguous'}

def is_test(f):
    return ('/tests/' in f or f.startswith('tests/') or '/test_' in f or f.startswith('test_')
            or f.rsplit('/', 1)[-1].startswith('test_') or '/ui/uitest' in f or 'report_samples' in f)

def key(r):
    return r['row'].rsplit(':', 1)[0] if r['how'] in BAD or not is_test(r['test_file']) else r['test_file']

def is_done(cid):
    b = 'claude/tf-' + ('T08a' if cid == 'C069' else cid)
    if subprocess.run(['git', '-C', repo, 'rev-parse', '--verify', '-q', b], capture_output=True).returncode:
        return False
    return subprocess.run(['git', '-C', repo, 'merge-base', '--is-ancestor', b, 'main']).returncode == 0

R = {r['row']: r for r in csv.DictReader(open(rf, encoding='utf-8'), delimiter='\t')}
C = list(csv.DictReader(open(chunks, encoding='utf-8'), delimiter='\t'))
E = {c['chunk']: set(c['row_ids'].split()) for c in csv.DictReader(open(earlier, encoding='utf-8'), delimiter='\t')}
fails = []

count = collections.Counter(rid for c in C for rid in c['row_ids'].split())
missing = [r for r in R if count[r] == 0]
twice = [r for r, n in count.items() if n > 1]
extra = [r for r in count if r not in R]
print('rows in row-files.tsv:', len(R), '| rows in chunks:', sum(count.values()), '| chunks:', len(C))
for name, lst in (('in no chunk', missing), ('in two or more chunks', twice), ('not in row-files.tsv', extra)):
    if lst:
        fails.append('%d rows %s: %s' % (len(lst), name, lst[:10]))

for c in C:
    ids = c['row_ids'].split()
    if int(c['rows']) != len(ids):
        fails.append('%s says %s rows but lists %d' % (c['chunk'], c['rows'], len(ids)))
    for rid in ids:
        if rid in R and key(R[rid]) not in c['files'].split():
            fails.append('%s holds %s, whose file %s it does not name' % (c['chunk'], rid, key(R[rid])))

ids_now = {c['chunk']: set(c['row_ids'].split()) for c in C}
q = {'counts/T5-KR1.md:%d' % n for n in range(45, 53)}
done = sorted(cid for cid in set(E) | set(ids_now) if is_done(cid))
print('done chunks:', len(done))
for cid in done:
    if cid not in ids_now:
        fails.append('done chunk %s is gone' % cid)
        continue
    lost, gained = E.get(cid, set()) - ids_now[cid], ids_now[cid] - E.get(cid, set())
    ok_lost, ok_gained = (q if cid == 'C191' else set()), (q if cid == 'C192' else set())
    if lost != ok_lost or gained != ok_gained:
        fails.append('done chunk %s lost %s and gained %s' % (cid, sorted(lost), sorted(gained)))

open_of = collections.defaultdict(list)
for c in C:
    if c['chunk'] not in done and c['kind'] != 'mop-up':
        for f in c['files'].split():
            open_of[f].append(c)
for f, cs in open_of.items():
    if len(cs) < 2:
        continue
    if any(c['files'].split() != [f] or c['kind'] != 'file-part' for c in cs):
        fails.append('%s is in open chunks %s that are not all single-file parts' % (f, [c['chunk'] for c in cs]))
        continue
    parts = [tuple(map(int, c['part'].split('/'))) for c in cs]
    ns = [p[0] for p in parts]
    if len(set(p[1] for p in parts)) != 1 or ns != sorted(ns) or len(set(ns)) != len(ns):
        fails.append('%s open parts out of order: %s' % (f, [(c['chunk'], c['part']) for c in cs]))
    else:
        print('runs in order:', f, [(c['chunk'], c['part']) for c in cs])

print('\n'.join(['FAIL: ' + x for x in fails]) if fails else 'all checks pass')
sys.exit(1 if fails else 0)
