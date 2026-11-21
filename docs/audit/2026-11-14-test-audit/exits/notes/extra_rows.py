"""E5 rows the ledger's note pull does not make one by one. Usage: py -I extra_rows.py <AUD>
1. Each test or record SUMMARY.md names under "Side notes that need an exit in T8" and "Rule slip"
   (plan E5: "Each named test or record under those headings becomes a row"). notes.tsv holds that
   section only as two block rows (lines 31 and 33).
2. Each run test no record row matched, from mutation/mutation-unmatched.md (plan E1c: "a test the
   audit has no record for").
Writes exits/notes/extra-rows.tsv. Every row is marked `finding`: SUMMARY says these are "beyond the
ruled faults the recount already counted", so no record verdict covers them, and a run test with no
record is a missing record. Fails loudly if the unmatched file's counts change."""
import csv, os, re, sys
AUD = sys.argv[1]
S = 'capture/byreading/SUMMARY.md'
SUMMARY = [
    (35, 'tools/test_effort_test_faults.py:204', 'a test that cannot fail: no input makes a removed line look like a diff header (B9)'),
    (36, 'tests/test_precheck_git.cpp near 297 (C8 4.10)', 'title says the code mask keeps every line break; it blanks the \\r of \\r\\n and the expected string relies on that (B3)'),
    (37, 'A11 tests 4 and 6', 'check no Hydra code: they call only helpers in the test file (B1)'),
    (37, 'C4 test 1', 'checks no Hydra code: tests a CMake reader that lives in the test file (B3)'),
    (37, 'tools test_game_tools.py:3169 and :3174 (D5)', 'check no Hydra code: they test Python and Windows behaviour (B6)'),
    (37, "D12's nine PC and PS rows", 'never give the chart to the engine (B5)'),
    (38, 'E3 RJ:147, 232 and 285', 'would pass with no gate installed (B5)'),
    (38, 'D11 test 99', 'would pass if presses[1] had no end_read_after (B5)'),
    (39, "B8's whole batch", 'builds hand-written handoffs and repos where the record asked for real ones'),
    (39, 'DL2 test 1', 'builds old database files from typed CREATE TABLE text where the record asked for real ones'),
    (39, 'make_sng', 'writes no file-data length field, which a real .sng may hold (unchecked, B3)'),
    (40, 'DL3 click records (index.md)', 'misfiled as batch screens'),
    (40, 'A7 #43 and #47', 'misfiled under the Path report'),
    (40, 'D10 test 63', 'record says it needs a live game; stale or misfiled'),
    (40, 'A19 PL13', 'misfiled under the Path Builder'),
    (40, 'stale line numbers in B1, B2 and B6', 'many records cite stale line numbers'),
    (44, 'the B6 scout', 'rule slip: ran its batch\'s Python tests (98, 75 and 295 passed) though its brief named none'),
]
rows = [{'source': 'SUMMARY', 'where': '%s:%d' % (S, ln), 'names': n, 'mark': 'finding', 'why': w} for ln, n, w in SUMMARY]
U = 'mutation/mutation-unmatched.md'
lines = open(os.path.join(AUD, U), encoding='utf-8').read().splitlines()
want = {'caught no bug': 15, 'caught at least one bug': 50}
for i, ln in enumerate(lines, start=1):
    m = re.match(r'^## Run tests no record row matched, under "Tests that (caught no bug|caught at least one bug)" in tests.txt \((\d+)\)', ln)
    if not m:
        continue
    if int(m.group(2)) != want[m.group(1)]:
        raise SystemExit('%s: %s now holds %s tests' % (U, m.group(1), m.group(2)))
    got = 0
    for j in range(i, len(lines)):
        t = lines[j]
        if t.startswith('## '):
            break
        if t.startswith('- '):
            got += 1
            rows.append({'source': 'mutation-unmatched (%s)' % m.group(1), 'where': '%s:%d' % (U, j + 1),
                         'names': t[2:].strip(), 'mark': 'finding',
                         'why': 'the run saw this test and no audit record names it, so the audit has no record for it'})
    if got != int(m.group(2)):
        raise SystemExit('%s: section says %s tests, %d listed' % (U, m.group(2), got))
out = os.path.join(AUD, 'exits', 'notes', 'extra-rows.tsv')
with open(out, 'w', encoding='utf-8', newline='') as fh:
    w = csv.DictWriter(fh, fieldnames=['source', 'where', 'names', 'mark', 'why'], delimiter='\t', lineterminator='\n')
    w.writeheader(); w.writerows(rows)
print(len(rows), 'rows:', len(SUMMARY), 'from SUMMARY,', len(rows) - len(SUMMARY), 'run tests with no record')
