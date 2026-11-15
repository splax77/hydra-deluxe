# Test-suite audit: the catch-up rounds are done, the capture pass is half started (2026-11-15)

Session e633f7f6-f313-4787-b622-8d901b577b64. It continued the handoff `docs/handoffs/2026-11-15-test-audit-after-rounds-handoff.md` (removed; `git show 7881ba80f1fdc9f134958821d8862fa31e75566c:docs/handoffs/2026-11-15-test-audit-after-rounds-handoff.md`).

## Goal

Finish the audit of every test in Hydra's suite, by the approved plan `docs/superpowers/plans/2026-11-14-test-suite-audit.md`. The audit is read-only. It writes records, verifies them, chooses each finding's exit, checks every closure and asks the user. It fixes nothing; every fix waits for the user's yes and gets its own plan. Its work is on branch `claude/test-audit-records`, in `docs/audit/2026-11-14-test-audit/`. This handoff sits on its own branch, `claude/test-audit-capture-handoff`, cut from that branch's tip; fast-forward the audit branch to it before going on.

## What was done this session

Everything below is logged in `docs/audit/2026-11-14-test-audit/log.md`, in the sections "Delta round and the question-3 sweep" and "The window round".

**Main merged in.** The audit branch merged main at `5ec682c7`.

**`moments.py` fixed** (commit 84912ea3). The scrubber is an `ImGui::SliderFloat`, so the playhead holds float32 seconds. Each moment is now printed as the smallest float32 at or after it. Believer path 4's end prints 109.255577 s against the engine's 109255.57244 ms. No screenshot has been retaken yet.

**The delta round.** 161 records for the test cases main's 68 commits added, changed or removed, in `records/delta-DL1.md` to `delta-DL5b.md`, each slice checked by a fresh verifier (`verify/delta-*.md`). Verifiers agreed with 148 and disputed 13.

**The question-3 sweep.** `sweep/SW1.md` to `SW6.md` read every old Rewrite. 32 have question 3 as their only fault and move to Investigate in T8: A9 records 5, 9, 13, 16, 17, 22, 25, 27, 30, 35, 36, 37; A10 3, 5, 19, 27, 28; A12 2, 3, 14; A14 3, 4, 5, 6, 9, 21, 33, 34, 35, 36, 37, 38.

**The window round.** At `c10c7f54` the early-fill rule was fixed at 85 ms. At `5ec682c7` it takes a window and the search defaults to 60 ms (`kDefaultEarlyFillWindowMs`, `src/core/model.h`). The user chose to re-read the unchanged tests that run the search. 140 C records ("the window can move the result") are in `records/window-WN*.md`, each group verified. `window-WN4.md` is superseded by WN4a, WN4b and WN4c. About 80 B rows in WN4c were sorted by a scan, not a body read; they are listed at that file's head.

**Cross-slice findings.** The user asked whether hand-typed Burnout figures are recorded as a finding. They were only in pieces, so `cross-findings.md` now holds X1 (Burnout's "3- 1 2" and 378315 typed in 10 test files) and X2 (the Path Builder's typed fill ticks 36480 and 74880). No exit is chosen.

**The capture pass, started.** `hydra_uitest` and `hydra_replay` were rebuilt from main. Findings so far, in `records-engine/index.md`:

- The seven engine-record charts dumped at 60 ms (`records-engine/efill60/`) are identical to their 85 ms dumps apart from the build stamp.
- Burnout dumped at both windows with the GUI tests' settings is identical: "3- 1 2" 378315.
- All 116 charts under `testdata/input`, dumped at 85 and 60 ms under two settings sets, give 232 identical pairs (`records-engine/window-sweep/sweep.tsv`). The flag works: Beg at `--efill 0` loses its E0. So the window change moved nothing `dump` writes, for any corpus chart.
- A dump does not record which window made it.

**The capture lists.** `capture/CL1.md` to `CL4.md` list what the "claim not checked" records name: about 130 engine-dump rows, 90 screen-capture rows, 60 real-input rows and 40 real-hook-input rows. CL3 is coarse (one need row per record, first rule that matched). About 2,800 records read "Investigate, claim not checked only" by a text count (this includes the superseded WN4 file).

**The repeated-values pass, stopped part way.** `repeats/repeats.py` lists every literal typed on a test assertion line or test constant in three or more test files: 207 rows in `repeats/repeats.tsv`. Sorters RP1 (rows 1 to 69, `repeats/RP1.md`) and RP3 (rows 139 to 207, `repeats/RP3.md`) committed. RP2 (rows 70 to 138) did not commit, and no verifier ran. The user stopped the session here.

**Session cards.** Three missing cards were written and committed on main. The card `2026-10-11-14ef4882` named `83efb401`, a scratch merge commit from the `lanes-join` worktree that never reached main and that git has since dropped; it was removed from the card's commit list.

## The user's words this session

"cont worktrees\test-audit-records\docs\handoffs\2026-11-15-test-audit-after-rounds-handoff.md"

"Yes, start it" (to: start the next round: merge main, then the delta audit, the sweep and the moments.py fix together)

"why does a session card name a commit that doesnt exist? what work was committed that session?"

"Yes, fix the card"

"Yes, re-read them" (to: should the audit re-read the unchanged tests that run the engine, now that the default window moved from 85 to 60 ms)

"so where are we in the audit? are we fixing tests?"

"Yes, start captures"

"is this being recorded as a finding? because this is precisely the kind of bad test we are trying to audit" (about Burnout's best path and score typed by hand in many test files, and the Path Builder's fill ticks)

"Yes, add the pass" (to: add a pass that searches the whole suite for other values typed in many files)

"One run per kind" (to: how the capture pass handles the records waiting on a real input; the option was one real run per kind of claim, then a fresh agent per slice reads that run's output against its records)

"stop and create handoff"

These are chat words, not filed rulings. The words from earlier sessions that still bind this audit are quoted in the Claims table.

## What is next

1. Check every claim below against the code (agent rule 9). Fast-forward `claude/test-audit-records` to `claude/test-audit-capture-handoff`.
2. **Finish the repeated-values pass.** Send RP2 (rows 70 to 138) by `briefs/T5-repeats.md`, then one fresh verifier for each of RP1, RP2 and RP3. Fold every K1 and K2 candidate the verifiers agree with into `cross-findings.md` as X3 and on.
3. **The capture pass, one run per kind** (the user's choice). First a scout turns `capture/CL1.md` to `CL4.md` into a short list of concrete runs: for each kind of claim, the one command and real input that checks it, and the records it serves. The kinds seen so far: hook tests (feed recorded calls through the hooks; `C:\Users\Patrick\.claude\gates\tests\replay_extract.py` and `replay.py`, and the real state files under `C:\Users\Patrick\.claude\gates\state\agents\`); tool and script tests (run each tool once on `testdata/input`); screens (one `hydra_uitest --script` capture per screen, retaking the `records-screen/` captures with the fixed `moments.py`); and store and settings tests (a real store made by `hydra_batch` over `testdata/input`, and real settings files). Some named inputs do not exist on this machine (for example a real library file from release 2.8.0); list them as unavailable. Builds and runs are the main session's, through `hydra_build_pass`. Nothing runs on the user's library without the user's yes in chat.
4. **Re-judge.** After the runs, one fresh agent per slice reads the runs' output against its "claim not checked only" records and says which now hold, so the main session can move them to Keep in T8.
5. Then the plan's remaining steps: T6 and T7 (the main session's suite runs, which need the user's yes first), P5 (the test-case counts against `--list-test-cases`), the per-slice count check in `log.md`, and T8 to T11. T8 must also settle: the 32 sweep moves; WN4b's four disputed Investigates (records 7, 26, 27, 31: does a typed engine value make "claim not checked only" a Rewrite); X1, X2 and any new X findings; and the splits each round logged.
6. Open points from earlier sessions that still stand: `claude/early-fill-reach-slack-handoff` was overtaken by a filed ruling and the user decides whether to drop it; the throwaway worktree `worktrees/test-audit-cal2` (branch `claude/test-audit-cal2`) can go once the user agrees.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| branch | `claude/test-audit-records` | the audit's branch: every records, verify, sweep, capture and repeats file, the briefs and the log; not merged to main | `git log main..claude/test-audit-records` |
| branch | `claude/test-audit-capture-handoff` | this handoff, cut from the audit branch's tip `89c23169289958da366b0ed0b0a7180a5a6a5210`; fast-forward the audit branch to it | `git log claude/test-audit-records..claude/test-audit-capture-handoff` |
| commit | `89c23169289958da366b0ed0b0a7180a5a6a5210` | the audit branch's tip when the session stopped (RP1's sorted rows); not on main | `git show --stat` |
| commit | `5ec682c7d23ca76c1ff6dc06e9fefb9d309eadc8` | the merge of main into the audit branch; the code every record of this session was read at; not on main | `git show --stat` |
| commit | `c10c7f5485cb34ab77e15c7298403223771b5ccc` | the main commit the find rounds read; on main, not pushed | `git merge-base --is-ancestor` |
| commit | `e60ab3c3f60d34cbd9a0135e18a991d3313fcc15` | main when this session started; on main, not pushed | `git branch --contains` |
| commit | `8d77623d0dd9664cff2c6c2d50b2c37c0234ca2d` | main's tip at the end: the session-index refresh after the card fix; on main, not pushed | `git rev-parse main` |
| commit | `50a8b53b6384be45ee3325d87529910877458a1f` | the three new session cards; on main, not pushed | `git show --stat` |
| commit | `5ed7547fc634b86ae7f7171f9d3015035916fd76` | the card `2026-10-11-14ef4882` without `83efb401`; on main, not pushed | `git show` |
| commit | `84912ea3458230f60f726f5f8dd58cc3168e9f9d` | `moments.py` prints each moment as the smallest float32 at or after it; not on main | `git show` |
| commit | `0addb11075e732a2c977cd8818f29ff66dc4670c` | the delta round and the sweep logged; not on main | `git show --stat` |
| commit | `19eb2a17f2ea6a8e7c238c9347121a981d32777b` | the window round logged; not on main | `git show --stat` |
| commit | `07ebfeb77e82d7b17bbe5983cbc00bf5bd1569d5` | X1 and X2 updated with Burnout's 60 and 85 ms dumps; not on main | `git show` |
| commit | `d3639390e707d95f1e7d8118df1bfd1f8de5def5` | the corpus window sweep: 232 identical pairs; not on main | `git show --stat` |
| commit | `65f75f3c68f4e9188a1db0d6f6bd4b88e4848952` | the repeated-values brief, script and 207 rows; not on main | `git show --stat` |
| path | `docs/superpowers/plans/2026-11-14-test-suite-audit.md` | the approved plan, T0 to T11 | read it |
| path | `docs/audit/2026-11-14-test-audit/log.md` | every round in order, with this session's two new sections at the end | read it |
| path | `docs/audit/2026-11-14-test-audit/cross-findings.md` | X1 and X2, the findings that cross slices; no exit chosen | read it |
| path | `docs/audit/2026-11-14-test-audit/records-engine/index.md` | the engine records, the 60 ms dumps and the corpus sweep, with what the sweep does not cover | read it |
| path | `docs/audit/2026-11-14-test-audit/records-engine/window-sweep/sweep.tsv` | 232 rows, every one `True` in its `same` column | read it |
| path | `docs/audit/2026-11-14-test-audit/records-screen/moments.py` | the fixed moment printer; the screenshots in `records-screen/` were taken before the fix | read it |
| path | `docs/audit/2026-11-14-test-audit/capture/CL3.md` | the capture list for C and D, coarse by its own account | read it |
| path | `docs/audit/2026-11-14-test-audit/repeats/repeats.tsv` | 207 values typed in three or more test files | read it |
| path | `docs/audit/2026-11-14-test-audit/repeats/RP1.md` | rows 1 to 69 sorted; not verified | read it |
| path | `docs/audit/2026-11-14-test-audit/repeats/RP3.md` | rows 139 to 207 sorted; not verified | read it |
| path | `docs/audit/2026-11-14-test-audit/briefs/T5-repeats.md` | the brief to send RP2 and the three verifiers by | read it |
| path | `docs/audit/2026-11-14-test-audit/records/window-WN4c.md` | WN4c's records; its head lists the B rows sorted by scan | read it |
| path | `C:\Users\Patrick\.claude\gates\tests\replay_extract.py` | writes recorded real hook calls, the real input for the hook tests | read it |
| path | `C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe` | built from main this session; the sweep and the dumps used it | `--help` |
| state | `docs/audit/2026-11-14-test-audit/repeats/RP2.md` does not exist | RP2 was stopped before it committed | `git ls-files docs/audit/2026-11-14-test-audit/repeats` |
| state | no verifier has read RP1 or RP3 | the workflow was stopped first | no `repeats/V-*.md` file |
| state | local `main` is 77 commits ahead of `origin/main` (`0bfa2dcee38a14562b1b8dbd7ec7fef68d8da528`) | main is not pushed; this session pushed nothing | `git log origin/main..main --oneline` |
| ruling | `e633f7f6-f313-4787-b622-8d901b577b64` | "Yes, re-read them" | this session's card, once written |
| ruling | `e633f7f6-f313-4787-b622-8d901b577b64` | "is this being recorded as a finding? because this is precisely the kind of bad test we are trying to audit" | this session's card, once written |
| ruling | `e633f7f6-f313-4787-b622-8d901b577b64` | "Yes, add the pass" | this session's card, once written |
| ruling | `e633f7f6-f313-4787-b622-8d901b577b64` | "One run per kind" | this session's card, once written |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those." | card `docs/sessions/2026-11-15-5caaef5d.md` |
| ruling | `5caaef5d-321e-4f84-b720-518c46d428ba` | "no. you dont get to stop and refuse to do the work. figure out how to get what you need and do it." | card `docs/sessions/2026-11-15-5caaef5d.md` |
| ruling | `a3ba79d1` | "analysis settings panel, default 60ms, supports 0ms to 85ms" | `docs/agents/rulings/code-and-engine.md`, under `## Confirmed` |

## Checker output

```nok | branch | claude/test-audit-records | the audit's branch: every records, verify, sweep, capture and repeats file, the briefs and the log; not merged to main
ok | branch | claude/test-audit-capture-handoff | this handoff, cut from the audit branch's tip `89c23169289958da366b0ed0b0a7180a5a6a5210`; fast-forward the audit branch to it
not-on-main | commit | 89c23169289958da366b0ed0b0a7180a5a6a5210 | the audit branch's tip when the session stopped (RP1's sorted rows); not on main
not-on-main | commit | 5ec682c7d23ca76c1ff6dc06e9fefb9d309eadc8 | the merge of main into the audit branch; the code every record of this session was read at; not on main
not-pushed | commit | c10c7f5485cb34ab77e15c7298403223771b5ccc | the main commit the find rounds read; on main, not pushed
not-pushed | commit | e60ab3c3f60d34cbd9a0135e18a991d3313fcc15 | main when this session started; on main, not pushed
not-pushed | commit | 8d77623d0dd9664cff2c6c2d50b2c37c0234ca2d | main's tip at the end: the session-index refresh after the card fix; on main, not pushed
not-pushed | commit | 50a8b53b6384be45ee3325d87529910877458a1f | the three new session cards; on main, not pushed
not-pushed | commit | 5ed7547fc634b86ae7f7171f9d3015035916fd76 | the card `2026-10-11-14ef4882` without `83efb401`; on main, not pushed
not-on-main | commit | 84912ea3458230f60f726f5f8dd58cc3168e9f9d | `moments.py` prints each moment as the smallest float32 at or after it; not on main
not-on-main | commit | 0addb11075e732a2c977cd8818f29ff66dc4670c | the delta round and the sweep logged; not on main
not-on-main | commit | 19eb2a17f2ea6a8e7c238c9347121a981d32777b | the window round logged; not on main
not-on-main | commit | 07ebfeb77e82d7b17bbe5983cbc00bf5bd1569d5 | X1 and X2 updated with Burnout's 60 and 85 ms dumps; not on main
not-on-main | commit | d3639390e707d95f1e7d8118df1bfd1f8de5def5 | the corpus window sweep: 232 identical pairs; not on main
not-on-main | commit | 65f75f3c68f4e9188a1db0d6f6bd4b88e4848952 | the repeated-values brief, script and 207 rows; not on main
ok | path | docs/superpowers/plans/2026-11-14-test-suite-audit.md | the approved plan, T0 to T11
ok | path | docs/audit/2026-11-14-test-audit/log.md | every round in order, with this session's two new sections at the end
ok | path | docs/audit/2026-11-14-test-audit/cross-findings.md | X1 and X2, the findings that cross slices; no exit chosen
ok | path | docs/audit/2026-11-14-test-audit/records-engine/index.md | the engine records, the 60 ms dumps and the corpus sweep, with what the sweep does not cover
ok | path | docs/audit/2026-11-14-test-audit/records-engine/window-sweep/sweep.tsv | 232 rows, every one `True` in its `same` column
ok | path | docs/audit/2026-11-14-test-audit/records-screen/moments.py | the fixed moment printer; the screenshots in `records-screen/` were taken before the fix
ok | path | docs/audit/2026-11-14-test-audit/capture/CL3.md | the capture list for C and D, coarse by its own account
ok | path | docs/audit/2026-11-14-test-audit/repeats/repeats.tsv | 207 values typed in three or more test files
ok | path | docs/audit/2026-11-14-test-audit/repeats/RP1.md | rows 1 to 69 sorted; not verified
ok | path | docs/audit/2026-11-14-test-audit/repeats/RP3.md | rows 139 to 207 sorted; not verified
ok | path | docs/audit/2026-11-14-test-audit/briefs/T5-repeats.md | the brief to send RP2 and the three verifiers by
ok | path | docs/audit/2026-11-14-test-audit/records/window-WN4c.md | WN4c's records; its head lists the B rows sorted by scan
ok | path | C:\Users\Patrick\.claude\gates\tests\replay_extract.py | writes recorded real hook calls, the real input for the hook tests
ok | path | C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe | built from main this session; the sweep and the dumps used it
manual | state | docs/audit/2026-11-14-test-audit/repeats/RP2.md | RP2 was stopped before it committed
manual | state | no verifier has read RP1 or RP3 | the workflow was stopped first
manual | state | main | main is not pushed; this session pushed nothing
manual | ruling | e633f7f6-f313-4787-b622-8d901b577b64 | "Yes, re-read them"
manual | ruling | e633f7f6-f313-4787-b622-8d901b577b64 | "is this being recorded as a finding? because this is precisely the kind of bad test we are trying to audit"
manual | ruling | e633f7f6-f313-4787-b622-8d901b577b64 | "Yes, add the pass"
manual | ruling | e633f7f6-f313-4787-b622-8d901b577b64 | "One run per kind"
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "well the display doesnt exist in a vacuum, does it? its displaying things that are tied to real charts. and we can measure those."
manual | ruling | 5caaef5d-321e-4f84-b720-518c46d428ba | "no. you dont get to stop and refuse to do the work. figure out how to get what you need and do it."
manual | ruling | a3ba79d1 | "analysis settings panel, default 60ms, supports 0ms to 85ms"
checked 39 rows: 16 ok, 0 missing, 8 not-on-main, 5 not-pushed, 10 manual
```
