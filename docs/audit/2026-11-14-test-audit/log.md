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

Round 4 ran as one workflow (wf_29e1beb8-eac): auditors for A13 to A16 and verifiers for A9 to A12, launched together.
