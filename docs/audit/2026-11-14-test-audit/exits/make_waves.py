"""Lay every open chunk that has a file into waves 13 to 18 and write waves.tsv.

Usage: make_waves.py <chunks.tsv> <repo> <waves.tsv>

Chunks are taken with the main session's wave-13 list first, then every part of a file split into
two or more open parts, then the rest; each group keeps plan order (the order of chunks.tsv). The
split files go early because a file in four parts needs room in four waves, one after another.
Each chunk goes in the earliest wave that is not full, holds no other chunk naming one of its files,
and comes after the wave of the part before it. Waves are filled to an even share of the
open chunks, the first ones taking the remainder; check_waves.py checks the result.
"""
import csv, sys
from waves_common import (FIRST_WAVE, LAST_WAVE, read_tsv, is_done, has_file, repo_of, multi_part)

chunks, repo, out = sys.argv[1:4]
SEED = ['C012', 'C200', 'C202', 'C204', 'C205', 'C207', 'C208', 'C209', 'C210', 'C211', 'C212',
        'C213', 'C214', 'C215', 'C216', 'C217']

C = [c for c in read_tsv(chunks) if not is_done(repo, c['chunk']) and has_file(repo, c)]
by_id = {c['chunk']: c for c in C}
chains = {f for f, cs in multi_part(C).items() if len(cs) > 1}
rest = [c for c in C if c['chunk'] not in SEED]
order = ([by_id[i] for i in SEED] + [c for c in rest if c['files'] in chains]
         + [c for c in rest if c['files'] not in chains])
waves = list(range(FIRST_WAVE, LAST_WAVE + 1))
n, k = divmod(len(C), len(waves))
cap = {w: n + (i < k) for i, w in enumerate(waves)}
held = {w: [] for w in waves}
files_in = {w: set() for w in waves}
last_part = {}

for c in order:
    fs = set(c['files'].split())
    lo = last_part.get(c['files'], FIRST_WAVE - 1) + 1
    w = next((w for w in waves if w >= lo and len(held[w]) < cap[w] and not fs & files_in[w]), None)
    if w is None:
        sys.exit('no wave fits %s' % c['chunk'])
    held[w].append(c)
    files_in[w] |= fs
    last_part[c['files']] = w

wr = csv.writer(open(out, 'w', encoding='utf-8', newline=''), delimiter='\t', lineterminator='\n')
wr.writerow(['wave', 'chunk', 'repo', 'files', 'rows'])
for w in waves:
    for c in held[w]:
        wr.writerow([w, c['chunk'], repo_of(c), c['files'], c['rows']])
    print('wave %d: %d chunks' % (w, len(held[w])))
