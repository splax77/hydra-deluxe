"""Give each clean, unjudged Investigate record one question group. Usage: py -I investigate_groups.py <AUD>
"Clean" means no joined count row with a fault mark (after answer 4); "unjudged" means no by-reading
verdict. The group comes from the record's own verdict line. A line saying "Claim not checked only"
goes to I-realinput by script; so do the shorthands the record files define for it: "(RC)" and
"(RC')" in records/E1.md ("claim not checked only; every other question holds; needs capture RC",
E1.md line 12), "cnp" in records/C3.md ('"cnp" below is "claim not checked only"', C3.md line 5),
and "(only Q3)" / "on question 3 only". Every other line was read by the main session, session
f3bbb32b, 2026-11-18, and is assigned below by its number in that reading order. Writes
exits/investigate-groups.tsv and fails loudly if a record has no group or two."""
import csv, os, re, sys
from collections import Counter
AUD = sys.argv[1]
csv.field_size_limit(10**9)
GROUPS = {
    'I-realinput': 'the claim is not checked on real input, and every other rubric question holds',
    'I-nosource': 'a value or rule the test pins has no source the auditor could find (a page, a ruling, a decision)',
    'I-decision': 'the record puts a decision to the user (keep a seam, which rule wins, is a test wanted)',
    'I-notread': 'the auditor did not read something its verdict needs (an owner, a page, a helper)',
    'I-split': 'two parts of Hydra, or Hydra and the game, answer the same question differently',
    'I-nocaller': 'the code under test has no production caller',
    'I-unreachable': 'the case the test guards may not be reachable on a real chart or run',
    'I-livegame': 'only a live game run can settle the claim',
    'I-namedfault': 'the record names a test fault (a snapshot, a restated check) and still says Investigate',
    'I-pointer': 'the verdict only points to another record, whose own verdict is not Investigate',
}
G = {}
def put(group, *nums):
    for x in nums:
        for n in (range(x[0], x[1] + 1) if isinstance(x, tuple) else [x]):
            if n in G:
                raise SystemExit('line %d assigned twice' % n)
            G[n] = group
put('I-realinput', (7, 11), 20, 22, 33, 34, 39, (41, 49), (52, 54), 57, (59, 62), 64, 95, (101, 103), 113,
    (115, 121), 124, 125, 127, (130, 147), (149, 153), (155, 161), 192, (230, 231), (233, 243), (246, 254), (256, 264), (266, 382), (409, 418),
    421, 422, 423)
put('I-nosource', 3, (12, 15), 27, 30, 31, 32, 36, 38, 40, 50, 51, 68, 69, 70, 71, 72, 73, 75, 78, (82, 85),
    89, 94, 100, 104, 105, 109, 111, 112, 162, 165, 166, 168, 170, 179, 183, 185, 187, 194, 196, 199, 201,
    203, 206, 207, 232, 244, 245, 255, 265, 383, 384, (386, 388), (391, 397), 407, 419)
put('I-decision', 1, 2, 4, 29, 35, 63, 65, 90, 97, 99, 148, 184, 188, 189, 208, 209)
put('I-notread', 16, 37, 55, 56, 58, 79, 80, 98, 110, 123, 163, 167, 169, 186, 193, 197, 198, (210, 229),
    385, (398, 403), 420)
put('I-split', 6, 21, 66, 74, 76, 77, 81, 91, 106, 107, 108, 164, 404, 405, 408, 424, 425)
put('I-nocaller', 17, 18, (24, 26), 28, 67, 86, 87, 88, 96, 180, 181, 182)
put('I-unreachable', 5, 19, 23, 92, 93, 190, 191, 195, 406)
put('I-livegame', 114, 122, 126, 128, 129, 154)
put('I-namedfault', (171, 178), 200, 202)
put('I-pointer', 204, 205, 389, 390)

L = list(csv.DictReader(open(os.path.join(AUD, 'exits', 'ledger.tsv'), encoding='utf-8'), delimiter='\t'))
a4 = {(r['file'], r['line']): r['faults_after'] for r in
      csv.DictReader(open(os.path.join(AUD, 'exits', 'answer4.tsv'), encoding='utf-8'), delimiter='\t')}
faulty = Counter()
for r in L:
    if r['kind'] == 'count' and r['joins_record']:
        f = a4.get((r['file'], r['line']), r['faults'])
        if f and f != 'none':
            faulty[r['joins_record']] += 1
inv = [r for r in L if r['kind'] == 'record' and r['verdict'] == 'Investigate'
       and not faulty[r['id']] and not r['byreading_verdict']]
cache, out, n = {}, [], 0
for r in inv:
    ls = cache.setdefault(r['file'], open(os.path.join(AUD, r['file']), encoding='utf-8').read().splitlines())
    v = ls[int(r['line']) - 1].strip()
    if 'claim not checked only' in v.lower():
        g, how = 'I-realinput', 'script: "Claim not checked only"'
    else:
        n += 1
        if n not in G:
            raise SystemExit('line %d (%s:%s) has no group' % (n, r['file'], r['line']))
        g, how = G[n], 'read: line %d' % n
    out.append({'id': r['id'], 'verdict_at': r['file'] + ':' + r['line'], 'group': g, 'how': how,
                'verdict': v[:300].replace('\t', ' ')})
if n != len(G) or n != 425:
    raise SystemExit('read %d lines, assigned %d, expected 425' % (n, len(G)))
with open(os.path.join(AUD, 'exits', 'investigate-groups.tsv'), 'w', encoding='utf-8', newline='') as fh:
    w = csv.DictWriter(fh, fieldnames=list(out[0]), delimiter='\t', lineterminator='\n')
    w.writeheader(); w.writerows(out)
c = Counter(o['group'] for o in out)
for g in GROUPS:
    print('%5d  %s  %s' % (c[g], g, GROUPS[g]))
print(len(out), 'records')
