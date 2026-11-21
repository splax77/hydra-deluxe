Agent id: abe6b51f2d1a79c4a (task T8-E3-V-L-R01-L-R12, session f3bbb32b)

I read each row's record text in full (records A1, A10, A11, A12, A13, D1, D10, D11), not the test files. The `run` column was not used. Both scouts' files were written before key list revision 1. Their six `no key` rows are agreed where they match the revision: `records/A10.md:617` (K-partial), `records/D1.md:606` (K-duplicate; I add K-namebody, so it is in the table), `records/D10.md:385` (K-namebody), `records/D10.md:470` (K-duplicate), `records/D11.md:445` (K-nocode). The sixth, `counts/T5-IR10.md:59`, is in another batch.

## Counts

| batch | rows in the batch | lines in the label file | rows I agree with | rows I change |
|---|---|---|---|---|
| L-R01 | 70 | 70 | 59 | 11 |
| L-R12 | 87 | 87 | 66 | 21 |

Every batch row has exactly one line in its label file, in batch order, with no extras and no repeats (checked by comparing the row column of each batch file with the label table). In both ends tables, every row named for a key is in the batch and carries that key in the scout's table. Every key a scout used has a line in its ends table.

Two of the rows I count as agreed carry a revision key that the scout could not use: `records/A10.md:617` (scout `no key`, revision K-partial) and `records/D10.md:385` and `records/D10.md:470` and `records/D11.md:445` (scout `no key`, revision K-namebody, K-duplicate and K-nocode). Their label files still say `no key`; the main session should write the revision key when it merges.

## Rows I change

Most changes add one key the record names and the scout left out. The two main groups are the new revision keys (K-duplicate, K-namebody, K-nocode, K-partial), which the scouts did not have, and typed expected values that the record says should come from an owner.

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R01 | records/A10.md:68 | O-reply | O-reply, P-other | The record says the default speed 100 is `kBaseSpeedPercent` typed as a literal, which is a typed constant the code owns. |
| L-R01 | records/A10.md:211 | O-reply | O-reply, P-other | The verdict says the expected names should be taken from `display_title`, `display_artist` and `display_charter`, so the typed expected names are a typed constant the code owns. |
| L-R01 | records/A10.md:231 | O-reply | O-reply, P-other | The verdict is "same as 10, expected from the owners", so the typed expected names are fault-bearing in the same way. |
| L-R01 | records/A10.md:249 | O-reply | O-reply, P-other | The verdict is "as 10": the expected charter words should come from the owners, not typed. |
| L-R01 | records/A10.md:269 | F-row, P-other, K-cantfail | F-row, P-other, K-cantfail, K-duplicate | The verdict is Remove because `test_display_format.cpp:25` makes the same "rounds once" promise with the real function. |
| L-R01 | records/A10.md:578 | O-reply | O-reply, P-message | The record says the expected status word ("not in library") is typed, and the scout keyed the same typed words in the other rows of this record family (19, 20). |
| L-R01 | records/A12.md:226 | P-message | P-message, K-namebody | The name promises "no version list" and the body pins the whole sentence, which the record says in its item 9. |
| L-R01 | records/A13.md:105 | S-snapshot, K-floor | S-snapshot, K-floor, K-duplicate | The verdict is Remove because case 4 states the same property for every chart without a typed count. |
| L-R01 | records/A13.md:222 | K-cantfail, K-skip | K-cantfail, K-skip, K-floor | The record names "three charts is enough" (no reason) and an `analyzed > 0` floor. |
| L-R01 | records/A13.md:261 | K-selfmade, K-skip, K-floor | K-selfmade, K-skip, K-floor, K-namebody | The name says "display-layer recomputation" but the record says no display calls the function and the test's name is wrong. |
| L-R01 | records/A13.md:541 | S-snapshot, C-ownchecker | S-snapshot, C-ownchecker, K-partial | The name says "every field moves it" and the body lists six fields by hand, which the record's item 9 says stands in for "every field". |
| L-R12 | records/D1.md:65 | P-message | P-message, K-duplicate | The verdict is Remove because test 431 checks the same stop and the same line through `finish_run`. |
| L-R12 | records/D1.md:73 | P-other, C-clock | P-other, C-clock, K-namebody | The name says "still restores" while line 247 asserts the opposite for settings.ini, and the verdict says to rename it. |
| L-R12 | records/D1.md:226 | P-message | P-message, K-duplicate | The verdict is Remove because test 251 states the same promise through `run_bot`. |
| L-R12 | records/D1.md:421 | C-ownchecker | C-ownchecker, K-nocode | The record says it reads the source text with a regex as a stand-in for behaviour, which is the "source text" part of K-nocode. |
| L-R12 | records/D1.md:606 | no key | K-duplicate, K-namebody | The verdict is Remove as a duplicate of test 142, and the record also says the name says "base class method" but the input holds no such method. |
| L-R12 | records/D1.md:812 | F-result, Z-cite | F-result, Z-cite, S-snapshot | The record calls the fixture "a snapshot of one run" taken from a pass line. |
| L-R12 | records/D10.md:80 | P-other, O-gamemem | P-other, O-gamemem, Z-cite | The record says the number is justified by a comment ("constants.py says what +0x2e0 holds"), the same shape the scout keyed as Z-cite at :184 and :130. |
| L-R12 | records/D10.md:86 | K-selfmade, O-gamemem, E-echo | K-selfmade, O-gamemem, E-echo, K-duplicate | The verdict is Remove: same promise as the tests at :151 and :155. |
| L-R12 | records/D10.md:136 | P-other | P-other, K-duplicate | The verdict is Remove: same promise as four other tests. |
| L-R12 | records/D10.md:278 | P-other, C-clock, P-message | P-other, C-clock, P-message, K-comment | The record says the 4 is a raw result and "cover the comment and nobody can say what 4 is". |
| L-R12 | records/D10.md:284 | P-message, C-clock | P-message, C-clock, K-duplicate | The verdict is Remove: same promise as the test at :73. |
| L-R12 | records/D10.md:290 | C-clock | C-clock, K-duplicate | The verdict is Remove: same promise as the test at :90. |
| L-R12 | records/D11.md:130 | K-cantfail | K-cantfail, K-nocode | The record says the test greps two source files for deleted names and checks "names, not behaviour". |
| L-R12 | records/D11.md:255 | C-clock, O-gamemem, P-other | C-clock, O-gamemem, P-other, F-schedule | The record says "the schedule, the clock, the game and the score are fakes", and the scout's own note lists the fake schedule. |
| L-R12 | records/D11.md:472 | C-ownchecker | C-ownchecker, K-nocode, K-namebody, K-partial | The test scans source text, its title says "only the harness" but the harness is never scanned, and two files stand in for "every caller". |
| L-R12 | records/D11.md:609 | C-clock | C-clock, K-duplicate | The verdict is Remove because record 43's equality proves the same thing. |
| L-R12 | records/D11.md:820 | P-other | P-other, K-duplicate | The verdict is Remove: record 1 compares `end_read_after` and `aim_ms` for every press against the file. |
| L-R12 | records/D11.md:848 | P-message, O-gamemem | P-message, O-gamemem, K-duplicate | The verdict is Remove: record 80 makes the same promise on the same fixture. |
| L-R12 | records/D11.md:1011 | P-other, O-gamemem | P-other, O-gamemem, K-comment | The record names the 1539 lead as a raw result whose reason is in a comment. |
| L-R12 | records/D11.md:1073 | P-other, O-gamemem | P-other, O-gamemem, K-selfmade | The record says "the expected lead repeats the production sum". |
| L-R12 | records/D11.md:1183 | K-unreachable | K-unreachable, K-duplicate | The verdict is Remove: record 100 makes the same promise at the loader. |

## Rows I looked at hard and kept

- `records/D11.md:176`, `D11.md:191`, `D10.md:190`, `D10.md:476` and `D10.md:516` (Remove records about names that are gone): kept as K-cantfail. Their reason is "a standing check or another test covers the behaviour", not "the same check twice", so I did not add K-duplicate. They test a name with `hasattr`, not source text, so I did not add K-nocode. The main session may want to decide whether `hasattr` on a module counts for K-nocode.
- `records/A12.md:13`, `32`, `49`, `68`, `122`, `156`, `191` and `243`, and `records/A11.md:57`: kept as C-ownchecker. K-nocode ("only helpers in the test file") also fits them in words, but the key list gave the helper-tests-itself fault to C-ownchecker (question 2), so I left them.
- `records/A10.md:287` (verdict Rewrite) says line 454 duplicates the display format's own test. I did not add K-duplicate, because the key says the record says Remove and the test also checks something no other test does.

## Side notes

- `tests/test_display_format.cpp:25` is named as the real owner for the rounding promise by `records/A10.md:269`; the audit read it only by grep. Whether it really calls `percent_steps` with the same numbers is unchecked here.
- `tests/test_engine_sweeps.cpp:228` (A13:105): the comment cites "Finding 255", which the record could not find in `docs/`. I did not add Z-cite here, though the scout used Z-cite for the same kind of unfound "Finding 170" at A10:434; the two rows are labelled differently on the same fault. The main session should pick one reading.
- `tests/test_engine_sweeps.cpp:670` (A13:427): line 680 checks a `std::vector` capacity, which is a language-library behaviour, so it may fit K-nocode in part. The scout noted "that part fits no key". I did not add K-nocode, since the rest of the test does check chord rows.
- `tools/ch_probe/tests/test_overlay_logs.py:23, 44, 54, 64, 71, 78, 85` (D10:790 to 833): eight rows keyed P-other for the typed log column names. P-name ("a name or key the product writes") may fit them better. I left them as the scout did.
- `tools/ch_mcp/tests/test_bot_run.py:237, 247` (D1:73): the name says restore and the body asserts settings.ini is left as the game left it. The record notes it as a side note.
- Splits the records themselves name (not new): C++ `check_ascii_spans`, `fact_form` and `scan_page` against Python `dump_facts.py` (A12 records 1, 3, 4, `tests/test_docs_match_code.cpp:1109, 1134, 1156`); two walks of "within the limit" (`tests/test_engine_sweeps.cpp:147`); the guide's six defaults stated twice (A11 record 1, `docs/UserGuide.md:712-717`); the picture-size limit in two homes (`tests/test_docs_match_code.cpp:2032` and `tools/doc_shots/shrink.py:20`).
- `docs/agents/rulings/code-and-engine.md` line 50, cited by D1:722 for "we don't need golden anymore": I did not open it, so I did not check the quote.

## Not checked

I did not open the test files themselves, only the records' account of them, so a test line that moved since the records were written is not caught. I did not open `docs/agents/rulings/`, `counts/` or `exits/questions.md`.
