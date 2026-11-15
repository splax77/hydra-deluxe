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
