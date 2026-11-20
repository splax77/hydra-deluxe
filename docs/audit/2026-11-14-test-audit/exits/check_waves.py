"""Check waves.tsv against chunks.tsv.

Usage: check_waves.py <chunks.tsv> <waves.tsv> <repo>

It checks that every open chunk with a file left is in exactly one wave, from 13 to 18, and that no
done chunk, no chunk whose files are all gone and no unknown chunk is in one. No wave holds more
than 21 chunks, no file is named by two chunks of one wave, and the open parts of a split file run
one per wave in part order. Each row's repo, files and rows must match chunks.tsv.
waves_common.py owns what open, has a file and repo mean. It prints a count per wave and per repo.
"""
import sys, collections
from waves_common import (FIRST_WAVE, LAST_WAVE, MAX_PER_WAVE, read_tsv, is_done, has_file,
                          repo_of, part_no, multi_part)

chunks, waves, repo = sys.argv[1:4]
C = {c['chunk']: c for c in read_tsv(chunks)}
W = read_tsv(waves)
fails = []

open_ids = {i for i in C if not is_done(repo, i)}
want = {i for i in open_ids if has_file(repo, C[i])}
print('open chunks:', len(open_ids), '| with a file left:', len(want),
      '| with none:', sorted(open_ids - want))

seen = collections.Counter(r['chunk'] for r in W)
wave_of = {r['chunk']: int(r['wave']) for r in W}
for i in sorted(want - set(seen)):
    fails.append('%s is open with a file but in no wave' % i)
for i, n in seen.items():
    if n > 1:
        fails.append('%s is in %d waves' % (i, n))
    if i not in C:
        fails.append('%s is not in chunks.tsv' % i)
    elif i not in open_ids:
        fails.append('%s is done but in a wave' % i)
    elif i not in want:
        fails.append('%s has no file left but is in a wave' % i)

for r in W:
    c = C.get(r['chunk'])
    if not FIRST_WAVE <= int(r['wave']) <= LAST_WAVE:
        fails.append('%s is in wave %s, outside %d-%d' % (r['chunk'], r['wave'], FIRST_WAVE, LAST_WAVE))
    if c and (r['repo'], r['files'], r['rows']) != (repo_of(c), c['files'], c['rows']):
        fails.append('%s row does not match chunks.tsv' % r['chunk'])

by_wave = collections.defaultdict(list)
for r in W:
    by_wave[int(r['wave'])].append(r)
for w in sorted(by_wave):
    rs = by_wave[w]
    if len(rs) > MAX_PER_WAVE:
        fails.append('wave %d holds %d chunks, more than %d' % (w, len(rs), MAX_PER_WAVE))
    files = collections.Counter(f for r in rs for f in r['files'].split())
    for f, n in files.items():
        if n > 1:
            fails.append('wave %d names %s in %d chunks' % (w, f, n))
    reps = collections.Counter(r['repo'] for r in rs)
    print('wave %d: %d chunks, %d hydra, %d hooks' % (w, len(rs), reps['hydra'], reps['hooks']))

for f, cs in multi_part([C[i] for i in want]).items():
    cs = sorted(cs, key=part_no)
    ws = [wave_of.get(c['chunk']) for c in cs]
    if None in ws:
        continue
    if any(b <= a for a, b in zip(ws, ws[1:])):
        fails.append('%s parts out of order: %s' % (f, [(c['chunk'], c['part'], w) for c, w in zip(cs, ws)]))
    elif len(cs) > 1:
        print('parts in order:', f, ' '.join('%s@%d' % (c['chunk'], w) for c, w in zip(cs, ws)))

print('\n'.join(['FAIL: ' + x for x in fails]) if fails else 'all wave checks pass')
sys.exit(1 if fails else 0)
