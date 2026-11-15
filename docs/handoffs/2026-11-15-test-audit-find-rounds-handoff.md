# Test-suite audit: calibration passed, the find rounds are next (2026-11-15)

Session 5caaef5d-321e-4f84-b720-518c46d428ba.

## Goal

Run the rest of the audit of every test in Hydra's suite, by the approved plan at `docs/superpowers/plans/2026-11-14-test-suite-audit.md`. The audit is read-only. It writes records, verifies them, chooses each finding's exit, checks every closure and asks the user. Every fix waits for the user's yes and gets its own plan afterwards. All the audit's work is on branch `claude/test-audit-records`, in the folder `docs/audit/2026-11-14-test-audit/`; this handoff sits on that branch too.

## What was done this session

**T0.** The branch and its worktree (`worktrees/test-audit-records`) were cut from main. `coverage.md` lists every test file with its slice: 263 files, 92 slices over 23 rounds. Three things changed from the plan's table. A test file added to main after the plan, `tests/test_store_query_plans.cpp`, has the new slice A50. The user then answered the plan's question 2 with "No, audit them too", so the two experiment scripts under `tools/ch_probe/experiments/` are slice D16, and the 22 hook test files in `C:\Users\Patrick\.claude\gates\tests\` are slices E1 to E4 (read at that repo's commit `0586e15`). And after T2, `tests/test_preview_scene.cpp` took the calibration file's place in slice A20.

**T1.** The main session built `hydra_replay` and dumped six charts into `records-engine/`, with an index of which chart shows which case. A squeeze-in, a squeeze-out and a skipped fill are found. The early fill at the SP-ready moment is not: the replay output carries no `e_offset`. Later, for the new question 3 (below), the main session captured the screen side into `records-screen/`: the Paths and Stars tabs as text, and Preview screenshots at each activation, for Pitch Dark and I'm A Believer. The other four dumped charts are loose fixtures no screen shows. Each capture is a command file run by `hydra_uitest.exe --script`, so it can be run again.

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

These are chat words, not filed rulings under `## Confirmed`.

## What is next

1. Check every claim below against the code (agent rule 9). Main is local only and not pushed; check it has not moved.
2. **Bring main into the audit branch first.** Another session (a3ba79d1) filed a ruling on main at `c77421c9eb586aee7c1549e91fb35fba012eac30`: "so we should use the hit window. and stop inventing our own numbers." Its branch `claude/early-fill-hit-window` makes the early-fill window one number and edits the tests and the rubric examples. If that branch lands before the find rounds, the auditors judge the new tests; merge main into `claude/test-audit-records` before each round's brief.
3. **T5, the find and verify rounds.** Four auditors a round, one slice each, in `coverage.md`'s order (A, then B, C, D, E). From round two, two verifiers a round for the round before, never on a slice they audited. Each brief starts with `docs/agents/brief-preamble.md` and carries the plan's "What every brief carries"; `briefs/T3.md` is a working model (change the task id, the slice id and file list, the records file name, and add both `records-engine/index.md` and `records-screen/index.md` to the question 3 paragraph). The user asked for workflows; a scout launched from a workflow works (both calibration runs were). Each auditor owns `records/<slice>.md`; each verifier `verify/<slice>.md`.
4. After each round, read the records' "claim not checked" lists. Capture the charts and screen moments they name in one pass after the last find round (plan T1's rule), adding command files to `records-screen/`.
5. T6 (corpus change, two full-suite runs) and T7 (one planted bug per auditor) are the main session's, alongside the rounds. T8 to T11 follow, as the plan says.
6. The early-fill handoff on `claude/early-fill-reach-slack-handoff` (`ce2ad29f55b7de37214a438b320b19b36586e04b`) was written before the ruling above and is overtaken by it. It is not merged. The user decides whether it is dropped.
7. Throwaway: `worktrees/test-audit-cal2` (branch `claude/test-audit-cal2`) held calibration run 2. Its records are copied to the audit branch, so it can go once the user agrees.

## Open points the next session must not lose

- Run 2's record 15 says Pitch Dark's first activation "ends at a chord that was paid doubled (leeway)". Nobody checked that. The verify step should.
- Production findings from the calibration file, to carry to T8: `build_score_box` (`src/app/preview_view.cpp` lines 634 to 635) picks doubled or plain from its own window, not the replay row's `in_sp` (a split with the replay, D11 on the box's side); the drain box's "full meter" time is the display's own estimate across a tempo change; `kOnActivationMs` (0.5 ms) has no decision found; "Is Star Power running" is answered by both `PreviewActivation::sp_running_at` and the highway's sweep in `src/render/track_state.cpp`.
- The replay tools print no `e_offset` or early-fill flag, so no engine record can show the early-fill case. A side note for the audit.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| commit | `c77421c9eb586aee7c1549e91fb35fba012eac30` | `main` when this handoff was written: another session's filed ruling "so we should use the hit window. and stop inventing our own numbers."; local only, not pushed | `git rev-parse main`; `git show --stat` |
| commit | `4c6272e22896f4b824dfab67b71801e142de9b47` | the rubric change: question 3 reads a real chart for every display rule, the hard rule, the bad record beside the good one; on main, not pushed | `git show --stat` |
| branch | `claude/test-audit-records` | the audit's branch: coverage, engine and screen records, calibration records and key, briefs, log, this handoff; not merged | `git log main..claude/test-audit-records` |
| commit | `2232c1b92c7d563945fdd3697e124beef220cd35` | the screen captures; the audit branch's tip before this handoff; not on main | `git branch --contains` |
| commit | `010236d78c9db4809f60a2d79e5593c62d71dcbd` | calibration run 2 passes; its records, its brief and the key join the audit branch; not on main | `git show --stat` |
| commit | `069cd3a79c305d2474b1ef29b704fae95f74830d` | run 2's records on the throwaway branch `claude/test-audit-cal2`; not on main | `git show --stat` |
| branch | `claude/test-audit-cal2` | the throwaway tree for calibration run 2, with run 1's records and the log removed; not merged, not to be merged | `git log claude/test-audit-records..claude/test-audit-cal2` |
| branch | `claude/early-fill-hit-window` | another session's work making the early-fill window the hit window; not merged when this was written | `git log main..claude/early-fill-hit-window` |
| branch | `claude/early-fill-reach-slack-handoff` | this session's early-fill handoff, overtaken by the ruling on main; not merged | `git log main..claude/early-fill-reach-slack-handoff` |
| path | `docs/superpowers/plans/2026-11-14-test-suite-audit.md` | the approved plan, T0 to T11 | read it |
| path | `docs/audit/2026-11-14-test-audit/coverage.md` | every test file and its slice: 263 files, 92 slices, 23 rounds; A50, D16 and E1 to E4 added; the calibration file marked T3 | read it |
| path | `docs/audit/2026-11-14-test-audit/log.md` | every step, dump, run and score, in order | read it |
| path | `docs/audit/2026-11-14-test-audit/records-engine/index.md` | the six charts' engine records and which case each shows; the early-fill numbers' sources | read it |
| path | `docs/audit/2026-11-14-test-audit/records-screen/index.md` | the screen captures for Pitch Dark and I'm A Believer, how to re-run them, and what is not captured | read it |
| path | `docs/audit/2026-11-14-test-audit/calibration/key.md` | the calibration key: six tests, two whose claim is the fault | read it |
| path | `docs/audit/2026-11-14-test-audit/records/T3-run2.md` | the passing calibration records, 53 tests | read it |
| path | `docs/audit/2026-11-14-test-audit/briefs/T3.md` | the calibration brief, a model for the auditor briefs | read it |
| path | `docs/agents/test-rubric.md` | question 3 and its hard rule; "A record that misses the claim" | read it |
| path | `tests/test_preview_boxes.cpp` | the calibration file, 53 `TEST_CASE` lines | `git grep -c "^TEST_CASE" -- tests/test_preview_boxes.cpp` |
| function | `build_score_box()` | the score box picks doubled or plain from its own window at lines 634 to 635 of `src/app/preview_view.cpp`; a calibration production finding | read it |
| state | rounds not started | no `records/A*.md`, `B*`, `C*`, `D*` or `E*` file and no `verify/` folder exist | `ls docs/audit/2026-11-14-test-audit/records` lists only `T3.md` and `T3-run2.md` |
| state | `hydra_uitest.exe` in the main checkout was rebuilt from main on 2026-11-15 | the captures used it; rebuild before re-running a capture if `src/` changed | `build_cpp.ps1 -Target hydra_uitest` through a build pass |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "No, audit them too" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "yes use a workflow" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those." | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "draft 1 looks good. draft 2 is weaker because it is only talking about abstract concepts with room for interpretation. harder rules with concrete examples of good and bad work better. if there isnt a good example right now, create one." | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "okay proceed" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "capture then stop and create a handoff for the rest" | this session's card, once written |
| ruling | `a3ba79d1` | "so we should use the hit window. and stop inventing our own numbers." | `docs/agents/rulings/code-and-engine.md`, under `## Confirmed` |

## Checker output

```
not-pushed | commit | c77421c9eb586aee7c1549e91fb35fba012eac30 | `main` when this handoff was written: another session's filed ruling "so we should use the hit window. and stop inventing our own numbers."; local only, not pushed
not-pushed | commit | 4c6272e22896f4b824dfab67b71801e142de9b47 | the rubric change: question 3 reads a real chart for every display rule, the hard rule, the bad record beside the good one; on main, not pushed
ok | branch | claude/test-audit-records | the audit's branch: coverage, engine and screen records, calibration records and key, briefs, log, this handoff; not merged
not-on-main | commit | 2232c1b92c7d563945fdd3697e124beef220cd35 | the screen captures; the audit branch's tip before this handoff; not on main
not-on-main | commit | 010236d78c9db4809f60a2d79e5593c62d71dcbd | calibration run 2 passes; its records, its brief and the key join the audit branch; not on main
not-on-main | commit | 069cd3a79c305d2474b1ef29b704fae95f74830d | run 2's records on the throwaway branch `claude/test-audit-cal2`; not on main
ok | branch | claude/test-audit-cal2 | the throwaway tree for calibration run 2, with run 1's records and the log removed; not merged, not to be merged
ok | branch | claude/early-fill-hit-window | another session's work making the early-fill window the hit window; not merged when this was written
ok | branch | claude/early-fill-reach-slack-handoff | this session's early-fill handoff, overtaken by the ruling on main; not merged
ok | path | docs/superpowers/plans/2026-11-14-test-suite-audit.md | the approved plan, T0 to T11
ok | path | docs/audit/2026-11-14-test-audit/coverage.md | every test file and its slice: 263 files, 92 slices, 23 rounds; A50, D16 and E1 to E4 added; the calibration file marked T3
ok | path | docs/audit/2026-11-14-test-audit/log.md | every step, dump, run and score, in order
ok | path | docs/audit/2026-11-14-test-audit/records-engine/index.md | the six charts' engine records and which case each shows; the early-fill numbers' sources
ok | path | docs/audit/2026-11-14-test-audit/records-screen/index.md | the screen captures for Pitch Dark and I'm A Believer, how to re-run them, and what is not captured
ok | path | docs/audit/2026-11-14-test-audit/calibration/key.md | the calibration key: six tests, two whose claim is the fault
ok | path | docs/audit/2026-11-14-test-audit/records/T3-run2.md | the passing calibration records, 53 tests
ok | path | docs/audit/2026-11-14-test-audit/briefs/T3.md | the calibration brief, a model for the auditor briefs
ok | path | docs/agents/test-rubric.md | question 3 and its hard rule; "A record that misses the claim"
ok | path | tests/test_preview_boxes.cpp | the calibration file, 53 `TEST_CASE` lines
ok | function | build_score_box() | the score box picks doubled or plain from its own window at lines 634 to 635 of `src/app/preview_view.cpp`; a calibration production finding
manual | state | rounds not started | no `records/A*.md`, `B*`, `C*`, `D*` or `E*` file and no `verify/` folder exist
manual | state | hydra_uitest.exe | the captures used it; rebuild before re-running a capture if `src/` changed
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "No, audit them too"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "yes use a workflow"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those."
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "draft 1 looks good. draft 2 is weaker because it is only talking about abstract concepts with room for interpretation. harder rules with concrete examples of good and bad work better. if there isnt a good example right now, create one."
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "okay proceed"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "capture then stop and create a handoff for the rest"
manual | ruling | a3ba79d1 | "so we should use the hit window. and stop inventing our own numbers."
checked 29 rows: 15 ok, 0 missing, 3 not-on-main, 2 not-pushed, 9 manual
```
