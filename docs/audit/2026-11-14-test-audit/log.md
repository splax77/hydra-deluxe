# Test audit log

Every dump, run, planted bug, launch and verdict of the audit, in order, by the main session.

## 2026-11-15, T0

The branch `claude/test-audit-records` was cut from main at a19b9cb2, with its worktree at `worktrees/test-audit-records`. Main had moved 17 commits past the plan's merge (277bdb50) by then.

Two of those commits matter to the audit. The Preview fix landed (the merge 72c67aca of `claude/preview-engine-sp`): the Preview's own refill-tick function is gone, and the examples page now says B1 to B4 quote their tests as they stood at 003d76a3. The calibration file `tests/test_preview_scene.cpp` changed by 9 lines in that work (1,004 lines to 1,005, still 33 `TEST_CASE` lines), so whether it still holds a test whose claim is the fault is for T2 to find, as P11 already says.

The second is a new test file, `tests/test_store_query_plans.cpp` (commit 8486e93c, 369 lines, 2 `TEST_CASE` lines). It had no slice, so it has the new slice A50, appended to round 22. The plan's table now has 87 slices over 22 rounds.

The packing rule was re-run on main by `t0_slices.py` in the main session's scratch folder. Apart from the new file, it gives the plan's table file for file; only line totals moved, all under the caps. `coverage.md` lists 241 files, none without a slice.

The plan's question 2 asked whether the hook tests in `C:\Users\Patrick\.claude\gates\` and the two scripts under `tools/ch_probe/experiments/` could stay out. The user answered "No, audit them too" (2026-11-15, in chat; not a filed ruling). So the two scripts are slice D16, in round 22, and the 22 hook test files are slices E1 to E4, a new round 23, read at the `~\.claude` repo's main `0586e15`. The audit is now 92 slices over 23 rounds. `coverage.md` lists 263 files.

## 2026-11-15, T1

`hydra_replay` was built in the audit worktree at a19b9cb2 (build pass 93e233f6, slot 1). The main session ran `dump`, then `schedule` on path 0, for one chart file from each of the five starting folders, then `dump` on I'm A Believer from `testdata/input/common` and `schedule` on its path 4, for the squeeze-out case. It also ran `score` on each dump's path 0 to read each activation chord's `fill_start`; that output is not saved. Six charts, all under `testdata/input`. No agent ran anything.

Three of the four cases are found: a squeeze-in step, a squeeze-out and a skipped fill. The early fill at the SP-ready tick is not: the replay tools write no `e_offset` or E-critical flag, so the record cannot say which charts show it. A first check compared each activation's fill start with its SP-ready tick; that was the wrong test (the rule measures against the fill's deadline, `src/core/model.h` line 316), and it was withdrawn the same day. `records-engine/index.md` says so, and tests resting on that case read "claim not checked".

## 2026-11-15, T2

The main session wrote the calibration key to its scratch folder before any calibration agent existed. It used the plan's fallback file, as T2 allows: the calibration file is now `tests/test_preview_boxes.cpp`, marked T3 in `coverage.md`, and `tests/test_preview_scene.cpp` takes its place in slice A20 (1,005 lines, under the cap). The key's contents and reasons stay out of the repo until T3 is scored. Key count: 6.

## 2026-11-15, T3, run 1

One Sonnet scout, launched by a workflow from `briefs/T3.md` (committed before the launch; the key was written at 09:04:49, before it). It judged all 53 test cases, lines 115 to 1097, in 49 tool calls and about 6 minutes, and committed `records/T3.md` at 3e227299. Its verdicts: Rewrite 33, Remove 2, Investigate 18, Keep 0.

Score against the key: key count 6, found 6 (all six reported as not Keep, the four snapshot tests for the snapshot lines the key names). Verdict: **miss**. One of the two tests whose claim is the fault was reported as a check problem only (typed strings, questions 4 and 6), and its rewrite keeps the claim. The other was reported through a split between two owners, which passes. By T3, the audit stops here, the miss goes to the user, and the brief is fixed before any auditor starts. One calibration run is left, with a fresh agent.

Size, for T4: the agent finished its whole file (1,097 lines) well inside the 20-minute warning. So the slice table stands, if the audit goes on.

The agent also raised a point about the rubric itself (its S0): question 3 asks for an engine record, and a rule only a display owns (a text format, a clamp) has none, so no display-only test can get Keep. Thirteen of its eighteen Investigate verdicts rest on that alone.

## 2026-11-15, the fix between the runs

The user's words on the agent's point: "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those." On the first draft of a brief rule: "draft 2 is weaker because it is only talking about abstract concepts with room for interpretation. harder rules with concrete examples of good and bad work better. if there isnt a good example right now, create one." Then "okay proceed". Chat words, not filed rulings.

So the rubric changed on main (4c6272e2, merged here at 29759095), not the brief. Question 3 now reads a real chart for every display rule, putting what the screen shows beside what the chart and the engine say. A hard rule follows it for a claim that differs from the record, and a bad record of B1's test now sits beside the good one, showing the mistake. The T3 brief stayed as it was, apart from its worktree and output file.

## 2026-11-15, T3, run 2

One fresh Sonnet scout, launched by a workflow from `briefs/T3-run2.md`, in a throwaway worktree on `claude/test-audit-cal2` where run 1's records and this log were removed, so it could not read them. It judged all 53 test cases in 43 calls and about 8 minutes (records at 069cd3a7 there, copied here). Its verdicts: Rewrite 39, Remove 9, Investigate 5, Keep 0.

Score against the key: key count 6, found 6. Verdict: **pass**. Both tests whose claim is the fault were reported as failing question 2 or 3, each with a production-code finding and a split, and given Investigate. The four snapshot tests were reported on the key's lines. The key is now in `calibration/key.md`.

T4: the agent finished its whole file well inside the warning, so the slice table stands.

One thing to carry into the find rounds: most of its records still read "claim not checked" on question 3, because T1 saved only engine dumps, not what the screens show on those charts. Under the new question 3, the main session captures both sides, so the screen side is still to capture before the rounds.

## 2026-11-15, the screen side

First pass (commit 2232c1b9): Pitch Dark and I'm A Believer, the Paths and Stars tabs as text and a Preview screenshot at each activation's start. The four fixture charts were left out because the corpus library does not list them, and no screenshot sat at an SP end, a squeeze or an SP-ready moment. The main session proposed stopping there with a handoff. The user's words: "no. you dont get to stop and refuse to do the work. figure out how to get what you need and do it." (chat, not a filed ruling).

What closed each gap:

- **The four fixtures.** Each fixture's chart was copied byte for byte into `records-screen/fixture_songs/<name>/` beside a three-line `song.ini`, and `hydra_uitest --ini fixtures.ini` points the library there. A first pass in window mode (Hydra.exe on a scratch library) proved it; headless command files replaced it, because they can be rerun and they save the replies themselves.
- **Exact moments.** `records-screen/moments.py` reads a schedule and writes playhead lines (`type **/##scrub | <seconds>`) and highway screenshots at each activation's start, SP-ready moment, squeeze ends, 1 ms before the SP end and the SP end. 102 screenshots in all.
- **Believer's squeeze-out path.** Path 4 is the fourth score below the best, so "Score range 2 scores" does not list it. `corpus_depth4.ini` widens the range to 4, and the command file clicks `**/##path4`. The Preview then reads "Showing { 1 0- 1 }". Believer's path 0 was also scheduled (`records-engine/believer.schedule.json`) and captured at its moments.
- **The early-fill case.** Beg (Evans Blue), from `testdata/input/common`, was dumped, scheduled and captured: its best path "1- 0 0 E0" ends with an E0 activation whose row reads "early fill 0.0 ms". It also has a second squeeze-out.

Main moved during this work. Another session merged the hit-window change (0bfa2dce, from eef6b1d6: the E mark reads the 85 ms hit window on both sides; the 60 ms slack is gone) and filed two more rulings at c10c7f54 (an adjustable early-fill setting, default 60 ms, 0 to 85 ms). The engine records had come from a19b9cb2, and the first screen captures from programs built before that merge. So `hydra_replay` was rebuilt at 326cd4e0 (pass 36e1ec78, slot 2) and every chart re-dumped and re-scheduled: every dump came out the same, and every schedule changed only in its build stamp and source path. `hydra_uitest` was rebuilt at c10c7f54 (pass 6a816298, slot 1; `src/` the same as 326cd4e0) and every capture rerun: the five that had run before gave the same text. Two window sessions left records in `.git/realapp.jsonl` that name 326cd4e0, though the Hydra.exe they ran was built at 09:47, before the merge; all their captures were replaced by the headless runs.

One correction to the engine index came out of the check: it said `flammarker_ingame`'s dump has no paths. It has one, "(No activations.)" at 155295.

## 2026-11-15, T5, find round 1

Session 16892967 took over from the handoff. Every git claim in it was checked: main is still c10c7f54, `src/` is the same at 326cd4e0 and c10c7f54, and 0bfa2dce was not yet in the audit branch. Main was merged into the audit branch (515e05c1) before the briefs, so the auditors judge the tests main has.

Briefs `briefs/T5-A1.md` to `T5-A4.md` are made from `briefs/T3.md`, with the screen records added to question 3. `tests/test_activation_rows.cpp` is cut at line 841, a `TEST_CASE` boundary near the plan's 847: A2 holds 15 test cases, A3 holds 22. `tests/report_samples.cpp` (A1) holds no `TEST_CASE`; it is sample data for four other test files, so A1 writes one record per helper function.

Round 1 ran as one workflow (wf_58e275eb-941): four Sonnet scouts, launched together, about 8 minutes, 210 tool calls in all. Every slice finished; no test case was left unreached, so there is no re-send.

- A1, `records/A1.md` (b1b524ad): 4 helper records. Rewrite 2, Investigate 2. Its auditor found that only `test_report_page.cpp` and `uitest_report_windows.cpp` call the helpers; the other two files use only `every_row` from `report_samples.h`.
- A2, `records/A2.md` (014a1ece): 15 records. Rewrite 10, Investigate 5.
- A3, `records/A3.md` (c67f9f19): 22 records. Keep 1, Rewrite 13, Remove 3, Investigate 5.
- A4, `records/A4.md` (c5ad424a, 789b0fbb): 48 records. Keep 31, Rewrite 14, Remove 1, Investigate 2. `tests/test_analysis.cpp` is 1,544 lines on main now, not the 1,410 in `coverage.md`; it still holds 48 `TEST_CASE` lines.

Two claims were checked by reading before round 2: `build_activations` takes its `HydraRecord` unnamed (`src/app/path_view.cpp` line 243, A2's production finding), and `docs/agents/rulings/ui-and-display.md` has `## Unconfirmed` at line 102, as A3 says of the Dynamic Hit Window lines it cites.

Charts and screen moments the round asked for, for the one capture pass after the last find round: from A2, a nonzero early fill and an E1 path, a Paths tab with a Backend limit set (Believer path 4 at 50 ms, Pitch Dark activation 2 at 100 ms), Round and Round, Burnout, one chart's song length beside its timeline labels; from A3, Tapped Out, Burnout "3- 1 2", Focus, Epidermis, Get Ur Freak On, Weltseele, Sun of Nothing, Tapestry of the Starless Abstract, The Age of Betrayal and Round and Round, an orange badge, the copied-path text, and a chord line under Pro Drums on and off; from A1, a library with copies, the path report and comparison windows, and a leaderboard score above the engine's on a chart that is not yet a full combo.

Round 2 ran as one workflow (wf_8b86557d-0af): auditors for A5 to A8 and verifiers for A1 to A4 (`briefs/T5-verify-R1a.md`, `T5-verify-R1b.md`), launched together.

Round 2 finished in about 8.5 minutes, 306 tool calls in all; every slice finished, so there is no re-send. Records: A5 (b651ec38) 29, Keep 21, Rewrite 3, Investigate 5; A6 (c76b81f9, 1f1ed806, 5b6d387e) 38, Keep 10, Rewrite 14, Investigate 14; A7 (c0d87d91) 47, Rewrite 21, Remove 1, Investigate 25; A8 (1685f313) 39, Rewrite 23, Remove 1, Investigate 15. Verify files: A1 and A2 (ca1c8d5e), agreeing with 18 of 19 verdicts and disputing A2 case 6; A3 and A4 (9dfed356), disputing A3 tests 18 and 22 and A4 tests 7, 28, 29, 30, 31 and 41, all six of them Keeps.

The auditors answered question 3 two ways for a test that is not about a chart. A4, A5 and A6 wrote "not an engine claim" or "no chart fact is claimed" and gave Keep; A7 wrote "claim not checked" and gave no Keep. The rubric's verdict section rules Keep out unless the claim was checked on a real chart, and "not an engine claim" is no check, so the main session fixed one reading for the rest of the audit: a claim about files, folders, the store or the settings is checked on real input against an independent owner, and a claim about what a window shows is checked on a capture, or it is "claim not checked". The rule, with a bad and a good line, was added to the round 3 auditor briefs (A9 to A12) and the round 2 verifier briefs (A5 to A8). A4's Keeps that its verifier did not dispute go to one fresh agent under the same rule (`briefs/T5-recheck-A4-q3.md`, writing `verify/A4-q3.md`). A6's Keeps are covered by its verifier's brief.

Round 3 ran as one workflow (wf_2c1ce9d6-90a): auditors for A9 to A12, verifiers for A5 to A8, and the A4 recheck, launched together. About 7 minutes, 238 tool calls; every slice finished.

- A9 (12346de6, 3e256e8f): 46 records. Rewrite 37, Remove 8, Investigate 1. No Keep: no settings claim was checked on a real settings file.
- A10 (4e442a67, bd102e42): 30 records. Rewrite 26, Remove 1, Investigate 3.
- A11 (2b319b44): 6 records. Rewrite 4, Investigate 2.
- A12 (1f7505b4): 14 records. Keep 1, Rewrite 12, Investigate 1.
- verify A5 and A6 (d995b2a4): disputes all 21 of A5's Keeps and 10 of A6's, every one on question 3; A6 record 30 also on question 5.
- verify A7 and A8 (35157454): agrees with all 86 verdicts. It did not read the code of A7 records 11, 17, 18 to 21, 35, 38, 41 and 44 line by line; those go to round 3's verifier of A11 and A12 (`verify/A7-reread.md`).
- recheck A4 (04f3537b, `verify/A4-q3.md`): of 25 undisputed Keeps, 11 stand (8, 10, 11, 13, 15, 33, 34, 36, 38, 39, 48) and 14 fall to "claim not checked". It asked whether the thread-pool tests count; the main session's reading is that a claim about threads, timing or a transport is checked on the real mechanism, so they stand. It also found test 2's reason wrong: `src/ui/app_state.cpp:596` allows a scan only when `chartfolders` is non-empty.

Two reading rules were added for round 4 on: a claim about threads, timing or a transport is checked on the real mechanism (the real pool, the real clock, real WinHTTP against a loopback server), which checks behaviour but not a real server's reply; and a verifier reads the code behind every record, never agreeing from a verdict's shape. Round 1's verifier of A3 and A4 left A3 records 7, 9, 10, 11, 19, 21 and A4 records 12, 13, 15, 17 to 22, 34, 35, 38 to 40 not re-opened; they go to round 3's verifier of A9 and A10 (`verify/A3-A4-reread.md`).

Round 4 ran as one workflow (wf_29e1beb8-eac): auditors for A13 to A16 and verifiers for A9 to A12, launched together. About 9 minutes, 328 tool calls; every slice finished.

- A13 (f4f140f6): 25 records. Rewrite 23, Remove 2.
- A14 (9abed756): 42 records. Keep 1, Rewrite 41. Most Rewrites are marked "input only": the test is sound and only its claim is unchecked.
- A15 (fd53f717, a11daf91): 30 records. Keep 2, Rewrite 16, Remove 1, Investigate 11.
- A16 (7e39f41b): 25 records. Keep 10, Rewrite 14, Investigate 1.
- verify A9, A10 and the A3/A4 reread (7c66d86a): A9 42 agreed, 4 disputed in part (records 6, 14, 33, 34).
- verify A11, A12 and the A7 reread: the reread found 9 of 10 A7 verdicts hold and disputes shell line 408 (line 417 cannot fail).

A14's "input only" Rewrites show a third reading gap: a sound test whose only fault is an unchecked claim was being given Rewrite, which would put a test change to the user that nobody needs. From round 5 on, such a test is Investigate, marked "claim not checked only", with the capture it needs; Rewrite is for a fault in the test itself. Records from earlier rounds that gave Rewrite for that reason alone (A9 and A14 at least) are swept once after the last find round, before T8 picks exits.

Round 5 ran as one workflow (wf_9f326d56-f28): auditors for A17 to A20 and verifiers for A13 to A16 (`briefs/T5-verify-R4a.md`, `T5-verify-R4b.md`), launched together. About 8 minutes, 259 tool calls; every slice finished.

- A17 (3ba6111f): 40 records (31, 8, and one for `test_main.cpp`). Keep 2, Rewrite 7, Remove 1, Investigate 30.
- A18 (09be1613, ae86f090, 49f78cdd): 54 records. Rewrite 7, Remove 3, Investigate 44. Its auditor ran short of time, so records 19 to 54 are shorter and leave reads undone; its verifier's brief says so.
- A19 (a84f34b2): 30 records. Keep 1, Rewrite 20, Investigate 8 (plus one it counts both ways). `tests/test_precheck.cpp` holds 5 real test cases, not 7: the `TEST_CASE` lines at 208 and 224 are inside fixture text.
- A20 (837e314e): 33 records. Rewrite 15, Remove 1, Investigate 17.
- verify A13 and A14 (79500919): A13 24 agreed, 1 disputed (record 13 Remove should be Rewrite); A14 29 agreed, 13 disputed, among them its only Keep (record 41) and 12 "input only" Rewrites that should be Investigate, "claim not checked only".
- verify A15 and A16 (364943a0): A15 28 agreed, 2 disputed (records 20 and 21 should be Rewrite); A16 21 agreed, 4 disputed, all Keeps (records 7, 15, 21, 25). It also found that records 11 and 13 of A15 name `pitch_dark_preview_1_act.png` as chord 437; the capture is at 85963 ms, chord 144.

Round 6 ran as one workflow (wf_c6addab9-6e9): auditors for A21 to A24 and verifiers for A17 to A20, launched together. About 10 minutes, 275 tool calls; every slice finished.

- A21 (b465e417): 30 records. Keep 9, Rewrite 11, Investigate 10. Case 30 compares against the saved Onyx screenshot `golden_onyx.png`; the filed ruling at `code-and-engine.md:50` reads "we don't need golden anymore, remove it all", and the auditor found the card showing it answered a question about the Python `golden/` folder. Whether it reaches the Onyx image is a question for the user.
- A22 (d268dd66): 40 records. Keep 2, Rewrite 20, Investigate 18. Production finding: `saved_enough` (`src/ui/preview_timeline.cpp:493-515`) works out "enough Star Power to activate" from drawn y positions, not the engine's `sp_ready_ms`.
- A23 (413f78ab): 21 records. Rewrite 13, Investigate 8.
- A24 (a3c7d531, 2ed115b9): 27 records. Rewrite 18, Remove 4, Investigate 5.
- verify A17 and A18, verify A19 and A20: committed; the A19 verifier moved PB1 to Remove, PC2 and PC3 to Rewrite (each returns after a MESSAGE when its commit is missing), and PC5 to Keep.

Round 7 ran as one workflow (wf_b33dc705-fe4): auditors for A25 to A28 and verifiers for A21 to A24, launched together. About 9 minutes, 345 tool calls; every slice finished.

- A25 (b5d3038c): 42 records. Keep 2, Rewrite 14, Remove 3, Investigate 23. Split: D103 item 22 (decisions line 549) says a chart-mode change "never marks" the path report out of date; `src/app/report.cpp:358-364` and test R29 say it does.
- A26 (b1331ce5, 9877345f): 46 records. Keep 1, Rewrite 17, Remove 4, Investigate 24.
- A27 (da0fb17a, e5153458, 234df5b3): 44 records. Keep 2, Rewrite 18, Investigate 24.
- A28 (15e10100): 24 records. Rewrite 13, Remove 1, Investigate 10.
- verify A21 and A22, verify A23 and A24 (4dfaeb38): committed. The A23 verifier finished record 2 from the existing Beg captures and moved it to Keep.

Round 8 ran as one workflow (wf_927e024b-d9e): auditors for A29 to A32 and verifiers for A25 to A28, launched together. About 9 minutes, 265 tool calls; every find slice finished.

- A29 (58685164, a90e27d2): 27 records. Rewrite 13, Remove 2, Investigate 12. Split A: `CONTEXT.md` lines 965 to 968 still say a late squeeze-in refills at the old SP end; D9 and the code refill on the squeezed chord.
- A30 (ef02b89f, 83f432d2): 29 records. Keep 2, Rewrite 19, Remove 1, Investigate 7.
- A31 (30053308): 39 records. Keep 9, Rewrite 11, Remove 1, Investigate 18. `testdata/input` holds no audio, so every stem claim needs a real stem.
- A32 (b5f1bb57): 31 records. Keep 7, Rewrite 8, Remove 1, Investigate 15.
- verify A25 and A26 (551752c0): A25 checked in full, 36 agreed, 6 disputed. A26 only partly: 35 of 46 records not reached.
- verify A27 and A28 (828bd40a): A28 checked in full, 20 agreed, 4 disputed. A27 only partly: 16 of 44 checked. It also corrects A28's production finding: `nominal_end()` is called from `tools/replay_schedule.cpp:415`, `tools/replay_json.cpp:135` and `tools/replay.cpp:766`; only `collected_phrase_ticks()` has no caller.

A26 and A27 get their one re-send each (`briefs/T5-verify-resend.md`), for the records their verify files list under "Not reached".

A capture defect, from A29: the screen records' "SP end" screenshots set the playhead just before the end (`believer_path4.cmd.txt:26` sets 109.255572 s; the engine's end is 109255.57244 ms), so they still draw Star Power running, and no capture sits at or after a window end. This also explains the handoff's open point about Believer path 4's second SP end. The capture pass after the last round fixes `moments.py`'s rounding so the end frame lands at or after the end.

Round 9 ran as one workflow (wf_95bdef43-d21): auditors for A33 to A36, verifiers for A29 to A32, and the A26 and A27 re-sends, launched together. About 8 minutes, 346 tool calls; every slice finished.

- A33 (50f9ac37): 34 records. Keep 2, Rewrite 12, Investigate 20. `RecordStore::reader()` and `StoreReads` have no caller in `src/` or `tools/`.
- A34 (9c815abc): 33 records. Keep 2, Rewrite 6, Remove 3, Investigate 22. `RecordStore::ready_song_ids` and `ready_records` have no caller in `src/` or `tools/` (A33 found the same).
- A35 (344d2e3e): 13 records. Rewrite 10, Investigate 3.
- A36 (0cdef4a4): 23 records. Rewrite 13, Remove 1, Investigate 9.
- verify A29 and A30 (87d377b5), verify A31 and A32 (67bb4bbe): checked in full. A29 23 agreed, 4 disputed; A31 33 agreed, 6 disputed.
- re-send A26 (2dcc5f36): 35 of 35 checked, 26 agreed, 9 disputed. Re-send A27 (01a46767): 28 of 28 checked, 26 agreed, 2 disputed. Both slices are now fully verified.

Round 10's files: `tests/ui/uitest_commands.cpp`, `uitest_engine.cpp` and `uitest_harness.cpp` hold no test. Their unit is one record per verb (commands) or per function a GUI test calls (engine and harness), as A1 was done for `report_samples.cpp`. `uitest_batch_reports.cpp` has 15 and `uitest_details.cpp` 26 `void test_*(ImGuiTestContext*)` functions, one record each; the main session still checks those counts against `hydra_uitest.exe --list` (plan T5).

Round 10 ran as one workflow (wf_79987c72-c50): auditors for A37 to A40 and verifiers for A33 to A36, launched together. About 8.5 minutes, 211 tool calls; every slice finished.

- A37 (482a83b3, b3037a92, 63efb255): 57 records. Keep 1, Rewrite 10, Remove 5, Investigate 41.
- A38 (29e7b6b1): 21 verb records. Keep 7 (click, type, open, timeout, wait-idle, wait-text, screenshot, each used by the screen records), Investigate 14. `docs/agents/ui-testing.md` line 88 says `timeout` limits wait-text and wait-idle only; the code also uses it for open, reset and the live cap on wait.
- A39 (0af10bbf): 26 records. Rewrite 11, Investigate 15. Its auditor flags three of its own "claim not checked only" records that also carry a second fault (`test_panel_split`, `test_panel_hide_library`, `test_settings_lock`); their verifier settles them.
- A40 (88a60614): 58 records. Keep 9, Rewrite 1, Remove 1, Investigate 47.
- verify A33 and A34 (5fa9cbe4), verify A35 and A36 (efa7281c): checked in full. A33 33 agreed, 1 disputed (T23, Keep to Investigate); A35 11 agreed, 2 disputed.

Round 11's split: `tests/ui/uitest_library.cpp` is cut at line 1069, a test-function boundary near the plan's 1034: A41 holds 18 test functions, A42 holds 12.

Round 11 ran as one workflow (wf_74d48d8a-2cc): auditors for A41 to A44 and verifiers for A37 to A40, launched together. About 4.5 minutes, 181 tool calls; every slice finished.

- A41 (f6feba92): 18 records. Rewrite 2, Investigate 16.
- A42 (02b05f5b): 12 records. Rewrite 3, Investigate 9. Record 12: line 1986 checks that "Narrow with" is not on screen, a string no file under `src/` holds, so it cannot fail.
- A43 (956c5f26): 21 records (16 live tests, 5 functions of `uitest_main.cpp`). Keep 4, Rewrite 1, Investigate 16. Split: `src/ui/main.cpp:145-167` and `tests/ui/uitest_main.cpp` each parse `--db`, `--ini`, `--shots`, `--pipe` and `--keep-temp`.
- A44 (ad9b293d): 12 records; the entry table and the 12 functions match. Rewrite 10, Investigate 2.
- verify A37 and A38 (0d68f42e): A37 50 agreed, 7 disputed. verify A39 and A40 (3d5db91c): A39 21 agreed, 5 disputed, among them the three records A39's auditor flagged.

Round 12's split: `tests/ui/uitest_report_windows.cpp` is cut at line 991, a test-function boundary near the plan's 830: A46 holds 13 test functions, A47 holds 11.

Round 12 ran as one workflow (wf_bdb96218-840): auditors for A45 to A48 and verifiers for A41 to A44, launched together. About 5.5 minutes, 223 tool calls; every slice finished.

- A45 (6528220f): 16 records. Keep 3, Rewrite 6, Investigate 7.
- A46 (013bed28): 13 records. Rewrite 7, Remove 1, Investigate 5 (its auditor corrected its own count).
- A47 (3db8efb6): 11 records. Keep 1, Rewrite 2, Investigate 8. The file's 24 test functions match its two entry lists (10 and 14).
- A48 (76210ddd): 15 test records (Rewrite 11, Investigate 4) and 16 function records.
- verify A41 (1889b17d) and A42 (6d6fafc5): A41 14 agreed, 4 disputed; among them `test_scan` line 72 cannot fail, because the Scan library button has one fixed label (`library_toolbar.cpp:71`), and `is_rescan` is written at `library_dialogs.cpp:207, 249, 310` and read nowhere in `src/`.
- verify A43 and A44 (7fd5c24e): A43 19 agreed, 2 disputed (L13 and L16, both Keeps).

Round 13's split: `tests/test_replay_schedule.cpp` is cut at line 851, a `TEST_CASE` boundary near the plan's 865: B3 holds 24 test cases, B4 holds 16.

Round 13 ran as one workflow (wf_11a4346e-081): auditors for A49, B1, B2 and B3 and verifiers for A45 to A48, launched together. About 9 minutes, 254 tool calls; every slice finished.

- A49 (f2e71327): 26 records. Keep 4, Rewrite 8, Investigate 14 (its auditor corrected its own count).
- B1 (0bbae758): 44 records. Keep 1, Rewrite 22, Remove 2, Investigate 19. `tests/test_fuzz_regressions.cpp` line 57 checks `run_one(...) == 0`, and `run_one` returns 0 on every path that returns.
- B2 (4a12b52f, 9aa11498, a2fac3e5): 66 records. Rewrite 21, Remove 2, Investigate 43.
- B3 (0badf6cc): 24 records. Keep 2, Rewrite 14, Investigate 8.
- verify A45 and A46 (3915242b): A45 13 agreed, 3 disputed (two Keeps among them). verify A47 (c0e25154) and A48 (52f944f9): A47 11 agreed; it found that the "Esc only hides" words exist, D271 item 7 at `fix-decisions.md:1233` ("Yes, both keep"), so the records' split lines on `test_open_path` and `test_reopen` are wrong.

Round 14 ran as one workflow (wf_75371b8a-90a): auditors for B4 to B7 and verifiers for A49 to B3, launched together. About 7 minutes, 259 tool calls; every slice finished.

- B4 (a96e0928): 16 records. Rewrite 11, Investigate 5. Test 964's `WARN(late_rows > 0)` (line 1002) cannot fail.
- B5 (c0480b0c): 9 records. Rewrite 7, Remove 1, Investigate 1. `rows_off_payment` (`tests/test_replay_windows.cpp:186`) is true by construction; `ReplayOptions::table` is set only by tests.
- B6 (1742fa4e, 8c3acd19): 52 records. Rewrite 16, Investigate 36. Line 1385 compares against saved expected-output files, which the filed ruling at `code-and-engine.md:50` rules out.
- B7 (57f88201): 21 records. Keep 1, Rewrite 5, Remove 1, Investigate 14.
- verify A49 and B1 (17d7590b): A49 21 agreed, 5 disputed. verify B3 (a024cc44) and B2 (0b722b37): B3 20 agreed, 4 disputed.

From round 15 the briefs' file lists are built from `coverage.md` by a script (each file's lines and unit count read from the file).

Round 15 ran as one workflow (wf_9c90cf68-ba4): auditors for C1 to C4 and verifiers for B4 to B7, launched together. About 10.5 minutes, 259 tool calls; every slice finished.

- C1 (025e63ef, 833dd89b): 53 records. Keep 3, Rewrite 23, Remove 2, Investigate 25. `decode_audio`, `decode_stem` and `mix_stems` have no caller in `src/` or `tools/`. The window-tag colour test pins a tag that `ui-and-display.md` line 108 (Unconfirmed) quotes the user as not wanting.
- C2 (four commits, last faa2c6c2): 59 records. Rewrite 17, Investigate 42. Two splits with the game, each backed by a decision: fill landing (D30) and `drums0dnoflip` (D19).
- C3 (e9ea0c6c): 68 records. Keep 3, Rewrite 11, Remove 5, Investigate 49.
- C4: 6 records. Keep 2, Rewrite 2, Remove 1, Investigate 1.
- verify B4 and B5, verify B6 and B7 (8ee29920 and the commits before it): committed.

Round 16 ran as one workflow (wf_4a09b438-aa6): auditors for C5 to C8 and verifiers for C1 to C4, launched together. About 11.5 minutes, 352 tool calls; every find slice finished.

- C5 (1eb5c4e9, 584ec070): 71 records. Keep 4, Rewrite 15, Remove 3, Investigate 49.
- C6 (8e3a4a0f): 55 records. Rewrite 28, Remove 3, Investigate 24. `tests/test_overlay_layout.cpp:396-399` measures the label "[Kick - YellowCym - GreenCym]"; the Preview now prints "[Kick - Yellow cymbal]" (`pitch_dark_preview_1_act.png`).
- C7 (1ed607ec, f1967f60): 44 records. Keep 5, Rewrite 15, Investigate 24. All 8 `test_perf_digest.cpp` cases pin a hash of the whole corpus from one run; whether a hash is a saved expected output under the ruling at `code-and-engine.md:50` is a question for the user.
- C8 (5c532bf5, b2966346, 5f930ea3): 28 records.
- verify C1 to C4: C1 and C2 reached every record. C3's verifier did not open six kick2x records (test lines 24, 51, 65, 176, 195 and 213), which get the slice's one re-send.

Round 17 ran as one workflow: auditors for C9 to C12, verifiers for C5 to C8 and the C3 re-send, launched together.
