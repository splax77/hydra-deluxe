Agent id: ab6d37adb356d1b17 (task T8-E3-V-L-R11-L-R13, session f3bbb32b)

Method: both batches were diffed against their label files by script (batch rows against label lines, duplicates checked), and the ends tables were checked by script against the label rows. Then every row's record was opened at the line in `row` (by a script that cut each record from that line to the next heading, so the whole record was read, not only its Direct and Verdict lines). Nothing was judged from the scouts' notes alone. I labelled neither file. The scout ids are a8b5d6e77a98d316c (L-R11) and ac7b286c9f8a360df (L-R13).

## Counts

L-R11: rows in the batch 77, lines in the label file 77 (exactly one per row, none missing, none extra, none repeated), rows I agree with 61, rows I change 16.

L-R13: rows in the batch 95, lines in the label file 95 (exactly one per row, none missing, none extra, none repeated), rows I agree with 83, rows I change 12.

Ends tables: every row an ends table names is in its batch and carries that key (checked by script, both batches). Two things follow from my changes and are listed under "Ends tables after my changes".

## Rows I change

Why most L-R11 changes are K-comment: the L-R11 scout never used K-comment. Its records say "raw results, working in a comment" (or "working not on the line") at ten rows, which is the definition of K-comment in `keys.md` (rubric question 4). The L-R13 scout used K-comment for the same wording at nine rows (for example D12.md:266, D13.md:295, D13.md:492), so the two files treat the same fault differently.

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-R11 | records/C5.md:895 | P-other, K-nocode | P-other, K-nocode, K-comment | Direct line 4 says the typed 66 is "a raw number whose meaning is in the comment". |
| L-R11 | records/C6.md:165 | P-other, Z-cite | P-other, Z-cite, K-comment | Direct says "two raw results" and Reason says the working "is not on the line". |
| L-R11 | records/C6.md:226 | P-other | P-other, K-comment | Direct says 157.14... and 194.28... are "raw results; the working (2.0 vs 4.0, 1.5 vs 3.0) is in comments". |
| L-R11 | records/C6.md:398 | P-other | P-other, K-comment | Direct says 71.97216363656233 is "a raw result" and Reason says its working "is not on the line". |
| L-R11 | records/C6.md:426 | P-other, Z-cite | P-other, Z-cite, K-comment | Direct says "four raw results" and Reason says "the sums have no working on their lines". |
| L-R11 | records/C6.md:454 | P-other | P-other, K-comment | Direct says 75.7156... and 95.7156... are "raw results" that "are not typed as the working". |
| L-R11 | records/C6.md:593 | no key | K-cantfail | Direct says CHECK_THROWS "accepts any exception, so a wrong error passes", the same too-weak check the L-R13 scout keyed K-cantfail at D12.md:305, D13.md:659 and D15.md:280 (if the main session gives weak checks their own key, move all four together). |
| L-R11 | records/C6.md:750 | P-setting | P-setting, K-cantfail | Deleted line says lines 68-69 "cannot fail except" at one place, and the verdict says to "say why 68-69 can fail". |
| L-R11 | records/C6.md:906 | P-other | P-other, K-comment | Direct says the raw sums have working "in none of the lines" and Reason says the inputs "are named in the comment only". |
| L-R11 | records/C6.md:929 | P-other | P-other, K-comment | Direct says 242 is "raw" and Reason says the inputs are "in comments". |
| L-R11 | records/C6.md:970 | P-other | P-other, K-comment | Direct says "raw results with working in comments". |
| L-R11 | records/C6.md:995 | P-other, Z-cite | P-other, Z-cite, K-comment | Direct says y = 20, 38.75, 57.5 are "raw; working in neither line". |
| L-R11 | records/C7.md:70 | S-snapshot, P-message | S-snapshot, P-message, Z-cite | The record's side note says line 162's comment "From one run: the tight window is the fourth" names the source in a comment, as C7.md:20 and :122 (keyed Z-cite) do. |
| L-R11 | records/C9.md:21 | P-setting | P-other | The three limits 30, 40, 20 are constants of the precheck's pick tool (`kCommonNameLimit` and the other two in `test_pick.h`), not a settings default or a screen word, so P-other fits and P-setting does not. |
| L-R11 | records/C9.md:957 | K-floor | K-floor, P-message | Display line says the printed `why` and `what` text are "typed expected strings" that "restate the code's wording". |
| L-R11 | records/C9.md:985 | E-echo | E-echo, K-skip | Stand-in line says lines 198-199 are "a negative loop" that "passes when there are no hits at all", the same silent-empty check the scout keyed K-skip at C9.md:192. |
| L-R13 | records/D12.md:101 | P-other | P-name | The typed dictionary holds the stop words the run writes (done, stopped, cap, frozen, past_end), which is a name the product writes, as the retime word at D12.md:884 is (keyed P-name). |
| L-R13 | records/D12.md:252 | K-duplicate | K-duplicate, K-comment | Line 4/8 says 38 is a "raw result, working in the comment above. Cover the comment and 38 means nothing." |
| L-R13 | records/D13.md:307 | P-other | P-other, K-comment | Test 48's record says line 749 is a "raw result pair" and points to test 46, whose record names the working "in the comment"; D13.md:312 (test 49) has the same fault and carries K-comment. |
| L-R13 | records/D13.md:402 | P-other | P-other, O-gamemem | Test 70 runs the real walk over fake game memory and types the exact read list of that memory, as D13.md:376 and :492 (same fixture) are keyed O-gamemem. |
| L-R13 | records/D13.md:501 | F-watchlog, P-other | F-watchlog, P-other, plus no key for the refuted claim | The record says TM-14 refutes the claim the tests pin (the +0x2E0 field is a hit time at every chord), and no key names that fault. |
| L-R13 | records/D13.md:513 | F-watchlog, P-other, P-message | F-watchlog, P-other, P-message, plus no key for the refuted claim | Line 2, 3 of the record says the test "differs from TM-14". |
| L-R13 | records/D13.md:517 | F-watchlog, P-other | F-watchlog, P-other, plus no key for the refuted claim | Record says "As 90", which is the TM-14 contradiction. |
| L-R13 | records/D13.md:521 | F-watchlog, P-other | F-watchlog, P-other, plus no key for the refuted claim | Record says "As 90". |
| L-R13 | records/D13.md:525 | F-watchlog | F-watchlog, plus no key for the refuted claim | Record says "As 90"; the scout's own note says the row "sits in the refuted claim". |
| L-R13 | records/D13.md:529 | F-watchlog | F-watchlog, plus no key for the refuted claim | Record says the test "sits inside the refuted claim". |
| L-R13 | records/D13.md:533 | F-watchlog | F-watchlog, plus no key for the refuted claim | Record says "As 90". |
| L-R13 | records/D13.md:590 | F-watchlog, P-message | F-watchlog, P-message, plus no key for the refuted claim | Record says "the PASS +0x2E0 line on hand rows is the refuted claim". |

The eight D13.md:501 to :590 rows share one cause. I wrote "plus no key" and did not invent a key, because the list is closed after Revision 1 (a `no key` row is then a question of its own). The scout named this fault in its side notes but left it off every row's keys.

## Ends tables after my changes

- L-R11, P-setting: the scout's plainest row records/C9.md:21 no longer carries P-setting after my change. The key then has one row, records/C6.md:750, which becomes both plainest and least-fitting.
- L-R11: K-comment (ten rows) and K-cantfail (two rows) now need ends rows. K-comment: plainest records/C6.md:226 (its Direct line names the comment), least-fitting records/C5.md:895 (a flag constant). K-cantfail: plainest records/C6.md:593, least-fitting records/C6.md:750.
- L-R13: nothing else breaks. K-comment ends (D12.md:266, D13.md:779) and O-gamemem ends (D12.md:34, D13.md:492) stay valid. The new D12.md:101 P-name row is fine beside the existing P-name ends.

## Agreed rows worth a second look (not changed)

- records/C6.md:774 (OL4) and records/C6.md:1018 (OL15) are `no key` in L-R11. OL4's fault is two unexplained inputs (rows 60 and 390). OL15's fault is hand-typed label text that the real label owner no longer prints, plus several claims in one case. I agree that no key fits either, but OL18 (C6.md:1090) and OL19 (C6.md:1115) carry the same "strings read from a run of the old label" fault and got P-other. So the same fault is `no key` in one row and P-other in two others.
- records/C6.md:1341 (PC6): the Display line says line 202 "types the display's text" (a cost printed with a true minus sign and a comma). I left it as S-snapshot, K-namebody, because P-setting (a screen word) is a stretch and the record's verdict is to move that line to the display-format test. The main session may want to add P-setting.
- records/C7.md:268 and records/C7.md:317: both record a race (the supersede count is true only if the first job is still running). C7.md:268 is `no key` for this and C7.md:317 is keyed P-other only, so the same fault is marked on one row and not the other.
- records/D13.md:367 and records/D13.md:501 are file headings (`## test_sp_watch.py`, `## test_watch_checks.py`), as the L-R13 scout says. Their verdicts match the first record under each heading: test 65 (Remove, line 371-374) and test 90 (Rewrite, line 511). The scout labelled them from those two records and I accept it, but the heading line itself holds no fault text.

## Side notes

- records/C7.md:88 (record item 2), against src/app/pb_coach.cpp:15 and :19: the record says `tight_window_note` asks `within_ms_limit` and then `timing_within_limit` for the same question, "two answers to is this window within the limit". This is a possible split in production code, not read by me.
- records/D13.md:501 to :590: `docs/clone-hero/03-timing.md` TM-14 says the +0x2E0 field is not a hit time, while `watch_checks.round_start_check` and `constants.ROUND_START_MAX_GAP_MS` check the opposite. That is a split between a Clone Hero page and a tool, named in the record's own text.
- records/C8.md:275 and tests/test_precheck_git.cpp:279: the stale `-WholeTree` text (the scout's side note, confirmed from the record's Finding line). The production message at tools/precheck/check_3.cpp:421 names a flag the tool does not accept.
- records/C6.md:22: the comment says "Freak's chord 263" and the record found no source for 56.18 (confirmed from the record's Reason line).
- records/D15.md:56 (BP-3): the record's Split line says the 120 s limit has only an Unconfirmed ruling line. That is a question for the user per the record, not a label matter.
- records/C5.md:11, :29, :47 pin the same three tooltip sentences that tests/test_user_messages.cpp:152-167 already pins (confirmed from record item 10).

## Not checked

Nothing. All 77 L-R11 rows and all 95 L-R13 rows were opened at their record line. For rows whose record text did not name a key's evidence outright, I opened the test where it was cheap: only D12.md:186 (C-stub) needed it, and `tools/ch_probe/tests/test_prep_inputs.py` line 89 (`exe = self._stub()`) confirms it. I did not open any production source file or any test file for the other rows.
