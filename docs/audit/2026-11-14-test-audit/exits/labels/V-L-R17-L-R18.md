Agent id: ae69c905b46756b50 (task T8-E3-V-L-R17-L-R18, session f3bbb32b)

Every row was checked against its record text, opened at the line in `row`. Records read: E4.md (lines 60-120, 225-500, 585-714, 735-964), T3-run2.md (whole), T3.md (lines 26-964), delta-DL1.md (25-50, 395-456), A33.md (130-190, 351-368, 429-445, for DL1 rows 399, 406, 410, 422, 430), delta-DL2.md (38-218, 335-385), delta-DL3.md (38-88, 164-223, 334-354, 444-469, 597-615), delta-DL4.md (100-315, 372-497), delta-DL5.md (14-355). The run column was not used. No test was opened.

## Counts

L-R17: rows in the batch 91, lines in the label file 91 (each row exactly once, none extra, none missing), rows I agree with 86, rows I change 5.

L-R18: rows in the batch 88, lines in the label file 88 (each row exactly once, none extra, none missing), rows I agree with 65, rows I change 23.

Both ends tables are sound: every row named in them is in its batch and carries the key it is listed under. My changes only add keys, so they stay sound.

## The nine `no key` rows in L-R17 (records/T3-run2.md:71, 136, 151, 166, 181, 213, 228, 331, 719)

I opened all nine records (tests 4, 8, 9, 10, 11, 13, 14, 20, 43). Each one's only verdict reason is that the claim was not checked on a real chart. None names a second fault a key fits. Test 4 (71): the section names it types are the ones it was given. Tests 8 to 10 (136, 151, 166): pure arithmetic on the playhead, "reads as the rule". Test 11 (181): three checks that read as the claim. Test 13 (213): "0" and "x1" are the rule's start values. Test 14 (228): the exact-hit rule, with D48 Q27 named as its source in the record. Test 20 (331): every expected value is the engine's own (`totalscore`). Test 43 (719): a positivity rule on three numbers. I agree all nine are `no key`. The scout's side note is right that no key names "claim not checked on a real chart".

## The five A33 rows in L-R18 (records/delta-DL1.md:399, 406, 410, 422, 430)

I opened the A33 records those rows point to: T8 (A33.md:132), T9 (149), T10 (166), T22 (353) and T27 (431). The scout's labels for 399, 406, 422 and 430 left faults those records name without a key; 410 stands. Details are in the table.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R17 | records/E4.md:371 | P-message, S-snapshot | P-message, S-snapshot, P-other | The record also says 4001 and the 1500 are retyped copies of the owner's `CAP` and `HEAD_CHARS`. |
| L-R17 | records/E4.md:401 | P-message, P-other | P-message, P-other, S-snapshot | The record calls "7008 characters" a pinned count, the same fault the scout keyed S-snapshot at lines 425, 479 and 798. |
| L-R17 | records/T3-run2.md:19 | K-comment, P-builtchart | K-comment, P-builtchart, K-duplicate | Item 10 says lines 122 and 131 repeat `test_display_format.cpp:29-37`: "two tests, one rule". |
| L-R17 | records/T3-run2.md:105 | K-comment, K-unreachable | K-comment, K-unreachable, P-builtchart | Item 4 says the strings are typed results of the engine on a built chart, the same fault the scout keyed on tests 5 and 7. |
| L-R17 | records/T3-run2.md:196 | S-snapshot, P-builtchart | S-snapshot, P-builtchart, Z-cite | Item 4 says the comment at 291-292 names the source as "pinned from one run", which the scout keyed Z-cite on tests 21 to 23. |
| L-R18 | records/T3.md:222 | S-snapshot, P-builtchart | S-snapshot, P-builtchart, Z-cite | The record quotes the test's own comment "pinned from one run as a fixed oracle". |
| L-R18 | records/T3.md:310 | P-builtchart | P-builtchart, K-duplicate | Item 10 says line 395 "repeats 362" at the same millisecond, and the verdict keeps only the unique half. |
| L-R18 | records/T3.md:379 | S-snapshot | S-snapshot, Z-cite | The record quotes the test's own words "Pinned from one run." as the only source of 20. |
| L-R18 | records/T3.md:397 | S-snapshot | S-snapshot, Z-cite | The record quotes the comment "Pinned from one run: this chart's best path earns all seven". |
| L-R18 | records/T3.md:415 | S-snapshot, P-builtchart | S-snapshot, P-builtchart, Z-cite | The record quotes the comment "The literals below were pinned from one run". |
| L-R18 | records/T3.md:485 | P-builtchart | P-builtchart, K-invented | The verdict itself says line 610 "pins a display estimate" (S3), and the scout keyed K-invented on the same fault in L-R17 at T3-run2.md:421 and 456. |
| L-R18 | records/T3.md:503 | P-builtchart | P-builtchart, K-duplicate | Item 10 lists three more boxes with the same clamp and the verdict asks for "one shared test for all four boxes". |
| L-R18 | records/T3.md:700 | P-builtchart | P-builtchart, K-duplicate | The verdict is "shared with 28; one test of the clamp for all boxes", the same copy of the clamp. |
| L-R18 | records/T3.md:804 | K-namebody | K-namebody, K-cantfail | The record says no line passes a playhead beyond the end, so the clamp's removal changes nothing; the scout keyed both on the same test in L-R17 (T3-run2.md:753). |
| L-R18 | records/T3.md:822 | P-builtchart | P-builtchart, P-other | The record calls the 0.5 ms (`kOnActivationMs`) "an unexplained display constant" the test does not name, keyed P-other on the same test in L-R17 (T3-run2.md:770). |
| L-R18 | records/T3.md:856 | P-builtchart | P-builtchart, K-duplicate | Item 10 says "four copies" of one clamp and the verdict is "one shared test for all boxes". |
| L-R18 | records/delta-DL1.md:399 | F-record, P-other | F-record, P-other, K-comment | A33 T8 item 4 says the counts "are raw results whose meaning is in the comments". |
| L-R18 | records/delta-DL1.md:406 | P-other | P-other, F-row | A33 T9 item 3 says the library rows are hand-made (`library_row`), and the capture it asks for is a real rescan. |
| L-R18 | records/delta-DL1.md:422 | P-other | P-other, F-row, F-record | A33 T22 item 3 says the test uses "a typed failure and hand-made library rows". |
| L-R18 | records/delta-DL1.md:430 | K-skip, K-cantfail | K-skip, K-cantfail, K-floor | A33 T27 item 4 calls line 644 (`records > 0`) "a floor for the loop ran". |
| L-R18 | records/delta-DL2.md:131 | S-snapshot | S-snapshot, K-cantfail, Z-cite | The record says line 925 is "implied by 926" and goes (the same fault the scout keyed K-cantfail on tests 2 to 6), and the comment "read off the one run that made it" is the cited source. |
| L-R18 | records/delta-DL3.md:41 | no key | K-floor | The record says `Sleep(20)` is "an input with no working" the test leans on, a number with no reason, the same as the scout's `K-floor` rows for waits in L-R17 (E4.md:849, 858). |
| L-R18 | records/delta-DL3.md:167 | no key | K-floor | Same `Sleep(20)` with no stated reason. |
| L-R18 | records/delta-DL3.md:209 | C-analyzer, P-other | C-analyzer, P-other, K-comment | The record says the typed "1,000" is "a raw result" with its working in a helper comment. |
| L-R18 | records/delta-DL3.md:336 | C-analyzer, P-other | C-analyzer, P-other, K-comment | The record says the vector is "raw ... with its working in a helper comment". |
| L-R18 | records/delta-DL3.md:446 | C-analyzer, P-other | C-analyzer, P-other, K-comment | The record says the raw results have "their working in the helper comment `gated_scores`". |
| L-R18 | records/delta-DL4.md:104 | C-analyzer | C-analyzer, K-floor | The record says the fixed 200 ms is "no source for that length" and a slower bug passes. |
| L-R18 | records/delta-DL4.md:137 | no key | K-floor | The record says the 50 ms wait "is the test's own", nothing proves the worker reached the lock, a number with no reason. |

That is 28 lines in the table. They are 5 rows in L-R17 and 23 in L-R18, as counted above.

## Rows I checked and agree with, where I looked hard

L-R17 `records/E4.md:248` (P-message, O-payload): the journal records are hand-written Workflow output, so O-payload fits. `records/T3-run2.md:554` (K-invented, P-other): the record calls the percent rule the game's with Label Unknown and the display's own copy, so K-invented fits though weakly. `records/T3-run2.md:818` and `records/T3.md:873` (P-name): the note names "Red" and "Red snare" are what the product writes, so P-name fits. `records/delta-DL2.md:361` (no key): the fault is a helper that repeats `HeldSaveGroup` and lacks its error handling; K-duplicate says "repeats another test's check", which is not a helper, so I leave it `no key`. `records/delta-DL3.md:599` (no key): reading the raw `report_started` flag where the claim is the strip's words fits no key; I agree.

## Side notes

- records/T3-run2.md:196 and 12 (test 12, records/T3.md:222): the scout used Z-cite on "Pinned from one run" in L-R17 (tests 21 to 23) but not in L-R18 (T3.md:379, 397, 415) or on test 12. The same fault now has the same key on both sides; the main session should read both batches' Z-cite groups together.
- records/T3-run2.md:284 (test 15, Investigate) and records/T3.md:328 (test 18): both name a split between the score box and the replay (the box shows plain at the SP end while the replay paid that chord doubled). These rows are not in either batch.
- records/T3-run2.md:421 to 456 and records/T3.md:485: the idle and tempo-change "full meter" seconds are the display's own estimate, while the search integrates across tempo changes (`src/search/graph.cpp:303`). The record marks it "carry out". That is a possible split.
- records/T3-run2.md:835 (test 50) and 770: `struck_at` is exact while `kOnActivationMs` is 0.5 ms for the same "is the playhead on this chord" question. The record already names it a split.
- records/delta-DL3.md:195: A5 record 3 says Keep and A7 record 23 says Rewrite for the same kind of check (reading `out_of_date` raw). The record already sends it to the main session.
- records/delta-DL5.md:95 and 287 and records/delta-DL1.md:450: the all-0 answer is stored in two places, the test cache key and `store::Lens` answer the same question twice, and a test comment quotes a user answer ("Mark them Stale") that no card or ruling holds. All three are in the scout's notes and in the records; I confirm them.
- records/delta-DL2.md:340 and 374: `StoreFile` and `remove_db`, and `HeldSaveGroup` and `StoreLockHolder`, are pairs of helpers for one job. I confirm the scout's note.
- records/T3.md:62 and other names: test names with commas break doctest's `-tc` filter (T:223, 662, 702, 738, 835, 929, 949). Confirmed as the records say.
- records/E4.md:748 (test_h52_cleanup_sweep.ps1:180, "the main checkout is untouched") is correctly `K-partial`; not a change.

## Not checked

Nothing. Every row of both batches was opened in its record. I did not open the original A32 and A14 records that some DL2 and DL5 delta rows say they replace, because the delta record states its own verdict and its own claim and I judged those rows from the delta text.
