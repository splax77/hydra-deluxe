"""Move the rows whose test file changed in row-files.tsv to chunks of their real file, keeping every
chunk id. Done chunks (their claude/tf-<chunk> branch merged into main) keep their ids and branches.

Usage: fix_chunks.py <old row-files.tsv> <new row-files.tsv> <chunks.tsv> <repo>   (rewrites chunks.tsv)

The rules, from the fix brief of task tf-rowfix:
- A moved row in an open chunk goes to the open chunk of its real file when that chunk stays at 16 rows
  or fewer (16 is the user's executor cap). A single-file chunk that would go over gets a new chunk
  after it, as the file's next part. A bundle that would go over gives up that file: its rows and the
  moved ones go to new chunks of their own, so one file is never in two open chunks except as parts
  that run one after another.
- A file with no open chunk gets new chunks, numbered on from the largest id, packed the way
  chunks.py packs: files of more than CAP rows cut into parts, smaller files bundled up to CAP rows.
- T5-KR1 rows 45-52 go from C191 to C192: C192 fixed all 16 Q rows in test_quiet.py.
- Any other moved row in a done chunk goes to an open chunk and is printed as found in a done chunk.
- A chunk left with no rows is dropped (none of them had a branch).
"""
import csv, sys, subprocess, collections

old_rf, new_rf, chunks_path, repo = sys.argv[1:5]
CAP, MAX = 15, 16
BAD = {'unresolved', 'notinrepo', 'ambiguous'}

def is_test(f):
    return ('/tests/' in f or f.startswith('tests/') or '/test_' in f or f.startswith('test_')
            or f.rsplit('/', 1)[-1].startswith('test_') or '/ui/uitest' in f or 'report_samples' in f)

def key(r):
    """The file a row's chunk names: its test file, or its audit file for a mop-up row (chunks.py)."""
    return r['row'].rsplit(':', 1)[0] if r['how'] in BAD or not is_test(r['test_file']) else r['test_file']

def branch(cid):
    return 'claude/tf-' + ('T08a' if cid == 'C069' else cid)

def is_done(cid):
    b = branch(cid)
    if subprocess.run(['git', '-C', repo, 'rev-parse', '--verify', '-q', b], capture_output=True).returncode:
        return False
    return subprocess.run(['git', '-C', repo, 'merge-base', '--is-ancestor', b, 'main']).returncode == 0

O = {r['row']: r for r in csv.DictReader(open(old_rf, encoding='utf-8'), delimiter='\t')}
N = {r['row']: r for r in csv.DictReader(open(new_rf, encoding='utf-8'), delimiter='\t')}
C = list(csv.DictReader(open(chunks_path, encoding='utf-8'), delimiter='\t'))
by_id = {c['chunk']: c for c in C}
for c in C:
    c['ids'] = c['row_ids'].split()
before = {c['chunk']: (c['part'], c['files'], len(c['ids'])) for c in C}
done = {c['chunk'] for c in C if is_done(c['chunk'])}
where = {rid: c['chunk'] for c in C for rid in c['ids']}
touched = set()

# The C191 to C192 move, fixed in test_quiet.py under C192.
q = ['counts/T5-KR1.md:%d' % n for n in range(45, 53)]
for rid in q:
    assert where[rid] == 'C191' and key(N[rid]) == 'tools/ch_mcp/tests/test_quiet.py', rid
by_id['C191']['ids'] = [r for r in by_id['C191']['ids'] if r not in q]
by_id['C192']['ids'] = sorted(q + by_id['C192']['ids'], key=lambda r: int(r.rsplit(':', 1)[1]))
touched |= {'C191', 'C192'}

# C192's own eight rows (53-58, 63, 70) moved to test_quiet.py too, and C192 fixed them there.
q192 = [rid for rid in by_id['C192']['ids'] if key(N[rid]) == 'tools/ch_mcp/tests/test_quiet.py']
if len(q192) != 16 or len(by_id['C192']['ids']) != 16:
    sys.exit('C192 does not hold the 16 Q rows of T5-KR1; stop')
moved = [rid for rid in O if key(O[rid]) != key(N[rid]) and rid not in q192]
found_done = []
incoming = collections.OrderedDict()
for rid in moved:
    cid = where[rid]
    if cid in done:
        found_done.append(rid)
    by_id[cid]['ids'].remove(rid)
    touched.add(cid)
    incoming.setdefault(key(N[rid]), []).append(rid)

def open_chunks_of(f):
    return [c for c in C if c['chunk'] not in done and c['ids'] and f in {key(N[r]) for r in c['ids']}]

def files_of(c):
    return list(dict.fromkeys(key(N[r]) for r in c['ids']))

pool = collections.OrderedDict()
new_after = []  # (file, rows) parts that run after an existing single-file chunk
for f, rs in incoming.items():
    oc = open_chunks_of(f)
    if not oc:
        pool[f] = list(rs)
        continue
    last = oc[-1]
    if len(last['ids']) + len(rs) <= MAX:
        last['ids'] += rs
        touched.add(last['chunk'])
    elif files_of(last) == [f]:
        new_after.append((f, list(rs)))
        touched.add(last['chunk'])
    else:
        own = []
        for c in oc:
            mine = [r for r in c['ids'] if key(N[r]) == f]
            c['ids'] = [r for r in c['ids'] if key(N[r]) != f]
            own += mine
            touched.add(c['chunk'])
        pool[f] = own + list(rs)

nid = max(int(c['chunk'][1:]) for c in C)
def new_chunk(kind, files, ids):
    global nid
    nid += 1
    c = {'chunk': 'C%03d' % nid, 'kind': kind, 'part': '1/1', 'files': ' '.join(files), 'ids': list(ids)}
    C.append(c)
    by_id[c['chunk']] = c
    touched.add(c['chunk'])
    return c

for f, rs in new_after:
    for i in range(0, len(rs), CAP):
        new_chunk('file-part', [f], rs[i:i + CAP])

# The pool, packed as chunks.py packs: big files first, cut into parts; small ones bundled.
bundle, bundle_rows = [], []
for f, rs in sorted(pool.items(), key=lambda kv: -len(kv[1])):
    if len(rs) > CAP:
        parts = (len(rs) + CAP - 1) // CAP
        size = (len(rs) + parts - 1) // parts
        for p in range(parts):
            new_chunk('file-part', [f], rs[p * size:(p + 1) * size])
    else:
        if len(bundle_rows) + len(rs) > CAP:
            new_chunk('files', bundle, bundle_rows)
            bundle, bundle_rows = [], []
        bundle.append(f)
        bundle_rows += rs
if bundle:
    new_chunk('files', bundle, bundle_rows)

# Rewrite files and parts of the touched chunks. A single-file chunk's part counts every chunk of
# that file, done ones included, in id order; a bundle is part 1/1.
for c in C:
    if c['chunk'] in touched and c['ids']:
        fs = files_of(c)
        if not c['ids'][0].startswith(('counts/', 'records/')):
            sys.exit('unexpected row id ' + c['ids'][0])
        c['files'] = ' '.join(fs)
for c in C:
    if c['chunk'] not in touched or not c['ids']:
        continue
    fs = c['files'].split()
    if c['kind'] == 'mop-up':
        continue
    if len(fs) == 1:
        same = [d for d in C if d['ids'] and d['files'].split() == fs]
        c['kind'] = 'file-part' if len(same) > 1 else 'files'
        c['part'] = '%d/%d' % ([d['chunk'] for d in same].index(c['chunk']) + 1, len(same))
    else:
        c['kind'], c['part'] = 'files', '1/1'

dropped = [c['chunk'] for c in C if not c['ids']]
for cid in dropped:
    if cid in done:
        sys.exit('a done chunk was left empty: ' + cid)
C = [c for c in C if c['ids']]

with open(chunks_path, 'w', encoding='utf-8', newline='') as fh:
    w = csv.writer(fh, delimiter='\t', lineterminator='\n')
    w.writerow(['chunk', 'kind', 'part', 'files', 'rows', 'row_ids'])
    for c in C:
        w.writerow([c['chunk'], c['kind'], c['part'], c['files'], len(c['ids']), ' '.join(c['ids'])])

for cid in sorted(touched):
    b = before.get(cid)
    a = by_id[cid]
    after = (a['part'], a['files'], len(a['ids'])) if a['ids'] else ('dropped', '', 0)
    print(cid, 'done' if cid in done else 'open', 'before:', b if b else 'new', 'after:', after, sep='\t')
print('found in a done chunk, not fixed:', [(r, next(c['chunk'] for c in C if r in c['ids'])) for r in found_done])
print('dropped (empty):', dropped)
