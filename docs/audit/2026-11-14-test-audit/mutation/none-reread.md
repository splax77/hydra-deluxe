# M5: the 16 record rows the join could not match, read once

One Sonnet scout (agent ab27653a85a7f05b1, session 899dc0dd, 2026-11-16) read each row listed under "Record rows: none (16)" in `mutation-unmatched.md`. It named each test as `tests.txt` spells it, matching by the record's test name rather than its line, because these lines moved after the audit read the files. It wrote no number from the run and picked no exit. `ledger.py` does not read this file yet, so the 16 rows stay `none` in `exits/ledger.tsv` and stay listed in `mutation-unmatched.md`; E7 reads them with this page.

| Record row | Test in the run (as in tests.txt), or why none | How it was matched |
|---|---|---|
| records/A12.md:105 | `tests\test_docs_match_code.cpp:1270 docs match code: no committed doc names a path into a scratch folder` | heading equals the test name; line moved from 1266 |
| records/A12.md:139 | `tests\test_docs_match_code.cpp:1420 docs match code: no permanent doc or code comment names a docs path that is not in the tree` | heading equals the test name; line moved from 1416 |
| records/A12.md:173 | `tests\test_docs_match_code.cpp:1887 docs match code: every plan that counts holds a Premise map in the shape` | heading equals the test name; line moved from 1883 |
| records/A12.md:208 | `tests\test_docs_match_code.cpp:2037 docs match code: every picture the public pages link is a shot the capture script takes, and the file exists` | heading equals the test name; line moved from 2032 |
| records/A14.md:459 | `tests\test_fills_and_skips.cpp:181 search: a fill that needs a 70 ms early hit is taken at an 85 ms window, not at 60` | same subject; the test was renamed and moved from 145 |
| records/A14.md:476 | `tests\test_fills_and_skips.cpp:220 search: SP ready 70 ms before a fill's deadline marks the activation E at 85, not at 60` | same subject; the test was renamed and moved from 154 |
| records/A32.md:28 | `tests\test_store_open.cpp:142 a database from Hydra 1.6 or older is too old: refused, or set aside for a fresh library` | heading is the start of the test name; line moved from 132 |
| records/A32.md:273 | `tests\test_store_open.cpp:790 a database any release before 2.2.0 wrote is too old: refused, or set aside for a fresh library` | heading is the start of the test name; line moved from 763 |
| records/A32.md:508 | `tests\test_store_open.cpp:1391 the hardest-figure marker is per profile: unset at first, set for one profile after it is recorded, still unset for another, and it survives a reopen` | heading is the start of the test name; line moved from 1205 |
| records/B2.md:719 | `tests\test_pathstring_read.cpp:82 pathstring read: every corpus best path round-trips` | the row is a Read line inside that test's record |
| records/B2.md:734 | `tests\test_pathstring_read.cpp:90 pathstring read: a token no fill can place is refused with its position` | the row is a Read line inside that test's record |
| records/C10.md:984 | `tests\test_preview_source.cpp:470 container charts give the same Song, stems and offset as the chart inside` | the row is a Read line inside that test's record |
| records/D16.md:17 | not a test in the run: the Python script `tools/ch_probe/experiments/key_delivery_test.py`, which asserts nothing (a person watches Notepad) | no .py entry in tests.txt |
| records/D16.md:51 | not a test in the run: the Python script `tools/ch_probe/experiments/pad_flash_test.py`, which asserts nothing (a person watches the pads) | no .py entry in tests.txt |
| records/window-WN8.md:145 | not a test in the run: a "Delta round" heading that replaces records/A10.md record 1, which is about `tests\test_dm_report.cpp:61 collect_dm_rows joins scores to records and labels them` | the heading names the record it replaces |
| records/window-WN8.md:198 | not a test in the run: a "Delta round" heading that replaces records/A25.md record R7, which is about `tests\test_report_rows.cpp:395 report counts every library copy of a chart, and leaves out a chart the library dropped` | the heading names the record it replaces |

The two A14 rows were matched by subject, not name, because the tests were renamed after the audit read them. A third test in the same file, `test_fills_and_skips.cpp:249` (the all-0 pass with 70 ms to spare), is about 70 ms too but fits neither record.
