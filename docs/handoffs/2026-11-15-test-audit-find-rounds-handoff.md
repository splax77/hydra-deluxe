# Test-suite audit: calibration passed, the find rounds are next (2026-11-15)

Session 5caaef5d-321e-4f84-b720-518c46d428ba.

## Goal

Run the rest of the audit of every test in Hydra's suite, by the approved plan at `docs/superpowers/plans/2026-11-14-test-suite-audit.md`. The audit is read-only. It writes records, verifies them, chooses each finding's exit, checks every closure and asks the user. Every fix waits for the user's yes and gets its own plan afterwards. All the audit's work is on branch `claude/test-audit-records`, in the folder `docs/audit/2026-11-14-test-audit/`; this handoff sits on that branch too.

## What was done this session

**T0.** The branch and its worktree (`worktrees/test-audit-records`) were cut from main. `coverage.md` lists every test file with its slice: 263 files, 92 slices over 23 rounds. Three things changed from the plan's table. A test file added to main after the plan, `tests/test_store_query_plans.cpp`, has the new slice A50. The user then answered the plan's question 2 with "No, audit them too", so the two experiment scripts under `tools/ch_probe/experiments/` are slice D16, and the 22 hook test files in `C:\Users\Patrick\.claude\gates\tests\` are slices E1 to E4 (read at that repo's commit `0586e15`). And after T2, `tests/test_preview_scene.cpp` took the calibration file's place in slice A20.

**T1, the engine side.** The main session built `hydra_replay` and dumped seven charts into `records-engine/`, with an index of which chart shows which case. All four cases are found: a squeeze-in (Pitch Dark, late_sqin_twice), a squeeze-out (Believer's path 4, and Beg), a skipped fill (folded_sqin and others) and an early fill (Beg's last activation, "E0", 0.0 ms). The records were made again at main 326cd4e0 after the hit-window change landed there; the content came out the same.

**T1, the screen side.** For the new question 3 (below), `records-screen/` holds what Hydra shows on all seven charts: the Paths and Stars tabs as text, and 102 highway screenshots, each at a moment the engine record names (an activation's start, its SP-ready moment, each squeeze's end, 1 ms before the SP end and the SP end). The four fixture charts reach the library through copies with a `song.ini`, in `records-screen/fixture_songs/`. Believer's squeeze-out path is shown with the score range widened to 4. Every capture is a command file run by `hydra_uitest.exe --script` built at c10c7f54, so it can be run again; `records-screen/index.md` says how and which case sits where.

**T2 and T3.** The calibration file became `tests/test_preview_boxes.cpp` (the plan's fallback; the scene file had no test whose claim is the fault). The key is in `calibration/key.md`. Run 1 missed: it reported one claim-fault test as a check problem only. Between the runs the rubric changed on main (below). Run 2, a fresh agent in a throwaway worktree with run 1's records and the log removed, passed: all six key tests found, both claim faults as Investigate with a production-code finding and a split. Run 2's records are `records/T3-run2.md`; run 1's are `records/T3.md`. T4: run 2 finished its whole file well inside the warning, so the slice table stands.

**The rubric change** (`docs/agents/test-rubric.md`, on main at `4c6272e22896f4b824dfab67b71801e142de9b47`). Question 3 now reads a real chart for every display rule, putting what the screen shows beside what the chart and the engine say. A hard rule follows for a claim that differs from the record, and a bad record of B1's test now sits beside the good one. The user asked for that shape: "harder rules with concrete examples of good and bad work better".

`log.md` records every step above, in order.

## The user's words this session

"No, audit them too" (to the plan's question 2: may the hook tests and the two experiment scripts stay out).

"yes use a workflow"

"well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those."

"draft 1 looks good. draft 2 is weaker because it is only talking about abstract concepts with room for interpretation. harder rules with concrete examples of good and bad work better. if there isnt a good example right now, create one."

"okay proceed"

"capture then stop and create a handoff for the rest"

"no. you dont get to stop and refuse to do the work. figure out how to get what you need and do it." (when the first screen pass stopped with gaps; every gap was then closed)

These are chat words, not filed rulings under `## Confirmed`.

## What is next

1. Check every claim below against the code (agent rule 9). Main is local only and not pushed; check it has not moved.
2. **Bring main into the audit branch first.** Another session (a3ba79d1) filed "so we should use the hit window. and stop inventing our own numbers.", and main merged its branch `claude/early-fill-hit-window` at `0bfa2dce`: the early-fill E mark now reads the 85 ms hit window on both sides, and that change edited tests. The audit branch was cut at a19b9cb2, before it. Merge main into `claude/test-audit-records` before each round's brief, so the auditors judge the tests main has.
3. **The early-fill code changes again soon.** The same session filed, at `c10c7f5485cb34ab77e15c7298403223771b5ccc`: "make the early fill threshold an adjustable setting pinned to one number in both directions" and "analysis settings panel, default 60ms, supports 0ms to 85ms". Its plan branch is `claude/early-fill-setting-plan`. When that code lands, re-dump and recapture any record whose E mark could move. Beg's E0 cannot: its offset is 0.0 ms, inside any window from 0 to 85.
4. **T5, the find and verify rounds.** Four auditors a round, one slice each, in `coverage.md`'s order (A, then B, C, D, E). From round two, two verifiers a round for the round before, never on a slice they audited. Each brief starts with `docs/agents/brief-preamble.md` and carries the plan's "What every brief carries"; `briefs/T3.md` is a working model (change the task id, the slice id and file list, the records file name, and add both `records-engine/index.md` and `records-screen/index.md` to the question 3 paragraph). The user asked for workflows; a scout launched from a workflow works (both calibration runs were). Each auditor owns `records/<slice>.md`; each verifier `verify/<slice>.md`.
5. After each round, read the records' "claim not checked" lists. Capture the charts and screen moments they name in one pass after the last find round (plan T1's rule), adding command files to `records-screen/` the way its index says.
6. T6 (corpus change, two full-suite runs) and T7 (one planted bug per auditor) are the main session's, alongside the rounds. T8 to T11 follow, as the plan says.
7. The early-fill handoff on `claude/early-fill-reach-slack-handoff` (`ce2ad29f55b7de37214a438b320b19b36586e04b`) was written before the ruling above and is overtaken by it. It is not merged. The user decides whether it is dropped.
8. Throwaway: `worktrees/test-audit-cal2` (branch `claude/test-audit-cal2`) held calibration run 2. Its records are copied to the audit branch, so it can go once the user agrees.

## Open points the next session must not lose

- Run 2's record 15 says Pitch Dark's first activation "ends at a chord that was paid doubled (leeway)". Nobody checked that. The verify step should.
- Production findings from the calibration file, to carry to T8: `build_score_box` (`src/app/preview_view.cpp` lines 634 to 635) picks doubled or plain from its own window, not the replay row's `in_sp` (a split with the replay, D11 on the box's side); the drain box's "full meter" time is the display's own estimate across a tempo change; `kOnActivationMs` (0.5 ms) has no decision found; "Is Star Power running" is answered by both `PreviewActivation::sp_running_at` and the highway's sweep in `src/render/track_state.cpp`.
- The replay tools print no `e_offset`. The E shows in a dump's pathstring, and the offset only on screen ("early fill N ms" shows its negative). A side note for the audit.
- Seen in the screenshots, for the find rounds to judge: at Believer path 4's second SP end, the frame 1 ms before and the frame at the end show the same score and both still draw Star Power as running ("empties in 0.0 s"). `records-screen/index.md` names the two files.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| commit | `c10c7f5485cb34ab77e15c7298403223771b5ccc` | `main` when this handoff was last updated: another session's two filed rulings on an adjustable early-fill setting; local only, not pushed | `git rev-parse main`; `git show --stat` |
| commit | `326cd4e0c24933cedc7bdcc5a0cd47e856adbc29` | the main commit the engine records were made from; its `src/` is the same as c10c7f54's, which the screen records were made from; on main, not pushed | `git diff --stat 326cd4e0 c10c7f54 -- src` is empty |
| commit | `0bfa2dcee38a14562b1b8dbd7ec7fef68d8da528` | main's merge of `claude/early-fill-hit-window`: the E mark reads the hit window on both sides; not in the audit branch yet | `git merge-base --is-ancestor 0bfa2dce claude/test-audit-records` fails |
| commit | `4c6272e22896f4b824dfab67b71801e142de9b47` | the rubric change: question 3 reads a real chart for every display rule, the hard rule, the bad record beside the good one; on main, not pushed | `git show --stat` |
| branch | `claude/test-audit-records` | the audit's branch: coverage, engine and screen records, calibration records and key, briefs, log, this handoff; not merged | `git log main..claude/test-audit-records` |
| commit | `2232c1b92c7d563945fdd3697e124beef220cd35` | the first screen pass (Pitch Dark and Believer only), since replaced; not on main | `git branch --contains` |
| commit | `010236d78c9db4809f60a2d79e5593c62d71dcbd` | calibration run 2 passes; its records, its brief and the key join the audit branch; not on main | `git show --stat` |
| commit | `069cd3a79c305d2474b1ef29b704fae95f74830d` | run 2's records on the throwaway branch `claude/test-audit-cal2`; not on main | `git show --stat` |
| branch | `claude/test-audit-cal2` | the throwaway tree for calibration run 2, with run 1's records and the log removed; not merged, not to be merged | `git log claude/test-audit-records..claude/test-audit-cal2` |
| branch | `claude/early-fill-hit-window` | another session's work making the early-fill window the hit window; merged into main at 0bfa2dce | `git log main..claude/early-fill-hit-window` |
| branch | `claude/early-fill-reach-slack-handoff` | this session's early-fill handoff, overtaken by the ruling on main; not merged | `git log main..claude/early-fill-reach-slack-handoff` |
| path | `docs/superpowers/plans/2026-11-14-test-suite-audit.md` | the approved plan, T0 to T11 | read it |
| path | `docs/audit/2026-11-14-test-audit/coverage.md` | every test file and its slice: 263 files, 92 slices, 23 rounds; A50, D16 and E1 to E4 added; the calibration file marked T3 | read it |
| path | `docs/audit/2026-11-14-test-audit/log.md` | every step, dump, run and score, in order | read it |
| path | `docs/audit/2026-11-14-test-audit/records-engine/index.md` | the seven charts' engine records, made at 326cd4e0, and which case each shows; the early-fill code at that commit | read it |
| path | `docs/audit/2026-11-14-test-audit/records-screen/index.md` | the screen records for all seven charts, at each moment the engine records name; how to re-run them; where each case is; what is not captured | read it |
| path | `docs/audit/2026-11-14-test-audit/records-screen/moments.py` | writes the playhead and screenshot lines for one schedule | read it |
| branch | `claude/early-fill-setting-plan` | another session's plan for the adjustable early-fill setting | `git log -1 claude/early-fill-setting-plan` |
| path | `docs/audit/2026-11-14-test-audit/calibration/key.md` | the calibration key: six tests, two whose claim is the fault | read it |
| path | `docs/audit/2026-11-14-test-audit/records/T3-run2.md` | the passing calibration records, 53 tests | read it |
| path | `docs/audit/2026-11-14-test-audit/briefs/T3.md` | the calibration brief, a model for the auditor briefs | read it |
| path | `docs/agents/test-rubric.md` | question 3 and its hard rule; "A record that misses the claim" | read it |
| path | `tests/test_preview_boxes.cpp` | the calibration file, 53 `TEST_CASE` lines | `git grep -c "^TEST_CASE" -- tests/test_preview_boxes.cpp` |
| function | `build_score_box()` | the score box picks doubled or plain from its own window at lines 634 to 635 of `src/app/preview_view.cpp`; a calibration production finding | read it |
| state | rounds not started | no `records/A*.md`, `B*`, `C*`, `D*` or `E*` file and no `verify/` folder exist | `ls docs/audit/2026-11-14-test-audit/records` lists only `T3.md` and `T3-run2.md` |
| state | `hydra_uitest.exe` and `hydra_replay.exe` in the main checkout were built at c10c7f54 and 326cd4e0 | the records used them; rebuild both and redo the records if `src/` changed | `build_cpp.ps1 -Target hydra_uitest` and `-Target hydra_replay` through a build pass |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "No, audit them too" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "yes use a workflow" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those." | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "draft 1 looks good. draft 2 is weaker because it is only talking about abstract concepts with room for interpretation. harder rules with concrete examples of good and bad work better. if there isnt a good example right now, create one." | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "okay proceed" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "capture then stop and create a handoff for the rest" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "no. you dont get to stop and refuse to do the work. figure out how to get what you need and do it." | this session's card, once written |
| ruling | `a3ba79d1` | "so we should use the hit window. and stop inventing our own numbers." | `docs/agents/rulings/code-and-engine.md`, under `## Confirmed` |
| ruling | `a3ba79d1` | "make the early fill threshold an adjustable setting pinned to one number in both directions" | `docs/agents/rulings/code-and-engine.md`, under `## Confirmed` |
| ruling | `a3ba79d1` | "analysis settings panel, default 60ms, supports 0ms to 85ms" | `docs/agents/rulings/code-and-engine.md`, under `## Confirmed` |

## Checker output

```
not-pushed | commit | c10c7f5485cb34ab77e15c7298403223771b5ccc | `main` when this handoff was last updated: another session's two filed rulings on an adjustable early-fill setting; local only, not pushed
not-pushed | commit | 326cd4e0c24933cedc7bdcc5a0cd47e856adbc29 | the main commit the engine records were made from; its `src/` is the same as c10c7f54's, which the screen records were made from; on main, not pushed
ok | commit | 0bfa2dcee38a14562b1b8dbd7ec7fef68d8da528 | main's merge of `claude/early-fill-hit-window`: the E mark reads the hit window on both sides; not in the audit branch yet
ok | commit | 4c6272e22896f4b824dfab67b71801e142de9b47 | the rubric change: question 3 reads a real chart for every display rule, the hard rule, the bad record beside the good one; on main, not pushed
ok | branch | claude/test-audit-records | the audit's branch: coverage, engine and screen records, calibration records and key, briefs, log, this handoff; not merged
not-on-main | commit | 2232c1b92c7d563945fdd3697e124beef220cd35 | the first screen pass (Pitch Dark and Believer only), since replaced; not on main
not-on-main | commit | 010236d78c9db4809f60a2d79e5593c62d71dcbd | calibration run 2 passes; its records, its brief and the key join the audit branch; not on main
not-on-main | commit | 069cd3a79c305d2474b1ef29b704fae95f74830d | run 2's records on the throwaway branch `claude/test-audit-cal2`; not on main
ok | branch | claude/test-audit-cal2 | the throwaway tree for calibration run 2, with run 1's records and the log removed; not merged, not to be merged
ok | branch | claude/early-fill-hit-window | another session's work making the early-fill window the hit window; merged into main at 0bfa2dce
ok | branch | claude/early-fill-reach-slack-handoff | this session's early-fill handoff, overtaken by the ruling on main; not merged
ok | path | docs/superpowers/plans/2026-11-14-test-suite-audit.md | the approved plan, T0 to T11
ok | path | docs/audit/2026-11-14-test-audit/coverage.md | every test file and its slice: 263 files, 92 slices, 23 rounds; A50, D16 and E1 to E4 added; the calibration file marked T3
ok | path | docs/audit/2026-11-14-test-audit/log.md | every step, dump, run and score, in order
ok | path | docs/audit/2026-11-14-test-audit/records-engine/index.md | the seven charts' engine records, made at 326cd4e0, and which case each shows; the early-fill code at that commit
ok | path | docs/audit/2026-11-14-test-audit/records-screen/index.md | the screen records for all seven charts, at each moment the engine records name; how to re-run them; where each case is; what is not captured
ok | path | docs/audit/2026-11-14-test-audit/records-screen/moments.py | writes the playhead and screenshot lines for one schedule
ok | branch | claude/early-fill-setting-plan | another session's plan for the adjustable early-fill setting
ok | path | docs/audit/2026-11-14-test-audit/calibration/key.md | the calibration key: six tests, two whose claim is the fault
ok | path | docs/audit/2026-11-14-test-audit/records/T3-run2.md | the passing calibration records, 53 tests
ok | path | docs/audit/2026-11-14-test-audit/briefs/T3.md | the calibration brief, a model for the auditor briefs
ok | path | docs/agents/test-rubric.md | question 3 and its hard rule; "A record that misses the claim"
ok | path | tests/test_preview_boxes.cpp | the calibration file, 53 `TEST_CASE` lines
ok | function | build_score_box() | the score box picks doubled or plain from its own window at lines 634 to 635 of `src/app/preview_view.cpp`; a calibration production finding
manual | state | rounds not started | no `records/A*.md`, `B*`, `C*`, `D*` or `E*` file and no `verify/` folder exist
manual | state | hydra_uitest.exe | the records used them; rebuild both and redo the records if `src/` changed
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "No, audit them too"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "yes use a workflow"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those."
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "draft 1 looks good. draft 2 is weaker because it is only talking about abstract concepts with room for interpretation. harder rules with concrete examples of good and bad work better. if there isnt a good example right now, create one."
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "okay proceed"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "capture then stop and create a handoff for the rest"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "no. you dont get to stop and refuse to do the work. figure out how to get what you need and do it."
manual | ruling | a3ba79d1 | "so we should use the hit window. and stop inventing our own numbers."
manual | ruling | a3ba79d1 | "make the early fill threshold an adjustable setting pinned to one number in both directions"
manual | ruling | a3ba79d1 | "analysis settings panel, default 60ms, supports 0ms to 85ms"
checked 36 rows: 19 ok, 0 missing, 3 not-on-main, 2 not-pushed, 12 manual
```
