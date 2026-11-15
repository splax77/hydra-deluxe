# Test-suite audit: the recount is done and the ready capture runs are taken (2026-11-15)

Session 63c70014-4328-49dc-9205-742235732752. It continued the handoff `docs/handoffs/2026-11-15-test-audit-capture-pass-handoff.md` on branch `claude/test-audit-capture-handoff`, which the audit branch now contains.

## Goal

Finish the audit of every test in Hydra's suite, by the approved plan `docs/superpowers/plans/2026-11-14-test-suite-audit.md`. The audit is read-only. It records, verifies, chooses each finding's exit, checks closures and asks the user. It fixes no test. Its work is on branch `claude/test-audit-records`, in `docs/audit/2026-11-14-test-audit/`. This handoff sits on its own branch, `claude/test-audit-capture-runs-handoff`, cut from the audit branch's tip. Fast-forward the audit branch to it before going on.

## What was done this session

The repeated-values pass finished. RP2 sorted the middle rows, three verifiers checked RP1 to RP3, and the findings X3 to X12 were added to `cross-findings.md`. Everything is logged in `log.md`, "The repeated-values pass and the capture list".

The user then ruled on what a test may rest on. A test must check what the real code does, on real data. It may not pin a typed constant, cite something that is not code, make up a meaningless string, or rest on anything fake. The user also chose that input a test builds and feeds to the real code is fine. Only three things are fake: a stand-in for the code's own output, a stand-in for outside data, and fake code (fake analyzers, readers, workers, clocks, sessions). The four rulings are filed on main.

With those rulings, 26 counters re-read every Keep and Investigate record, and a fresh tallier totalled them in `counts/TOTALS.md`. 1,861 of the 2,449 records hold a ruled fault: 239 of 421 Keep records and 1,622 of 2,028 Investigate records. So the capture pass can only move the clean Investigate records to Keep. `capture/CLEAN.md` lists those 397 records and the run each needs.

The main session ran every capture run that was ready: R1, R2, R3, R4a, R5, R6 and R8. They ran on `testdata/input`, `testdata/store` and the real reflog, never the user's library. `capture/runs/index.md` says what each did. R1 to R6 ran clean. R8 fails: today's ref gate refuses 65 of the real historical main moves, where its test allows one known exception. That is a finding to judge, not a run to fix. Nothing has been re-judged against these outputs yet.

## The user's words this session

"Yes, start them" (to: start RP2, the repeat verifiers and the capture scout)

"that doesnt make it any better. tests should test their intended behavior directly, not make up an arbitrary meaningless string" (about the overlay-layout test's made-up "3- 1 2" string; X1 stays at 10 files)

"Yes, re-read them" (to: re-read the K3 rows for made-up stand-ins)

"tests should not be pinned to typed constants. they should be pinned to the behavior of the code. i have repeatedly told you this is bad and you never listen."

"tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong"

"fake data will never appear in the real hydra and we should be testing the behavior of the real code"

"and why do we have a fake analyzer? what the fuck is the point of a fake analyzer? who came up with this shit?" (a fake analyzer that returns a real result is X9 too)

"tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation"

"No, input is fine" (to: does hand-built input fed to the real code count as fake)

"capture the real ones then stop and create handoff"

## What is next

First, write the R4b to R4h scripts and run them. They serve 61 clean records: the Library view, the batch strip, the Settings panel, the Dynamics tab, the Path report and the Path Builder. `capture/RUNS.md` names only the verbs, and R4h's Path Builder opening lines are unchecked. Build each script from `docs/agents/ui-testing.md`.

Then send one re-judge agent per slice. Each reads the run outputs against its clean records in `CLEAN.md` and says Keep or not. R8's 65 refusals go to the hook records it serves. The 171 unplaced records need a run found or written for them first.

Then the T8 exits: X1 to X12 and the 1,861 faulty records, each by one of the four exits in `docs/agents/audit-method.md`. About 40 rows in `TOTALS.md` are unsettled (IR1's hand-written compare databases, KR3's D2 self-scanner, IR4's hook payloads) and go to the user. Consider asking the user to add X9 to X12 as questions in `docs/agents/test-rubric.md`, since the rubric's silence is why so many records were kept.

Two housekeeping items. The session cards for e633f7f6 and for this session (63c70014) are not written. The older open points still stand: branch `claude/early-fill-reach-slack-handoff` and worktree `worktrees/test-audit-cal2` are unresolved.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| branch | `claude/test-audit-records` | the audit's branch, 399 commits ahead of main; not merged to main; it does not hold main's two rulings commits | `git log main..claude/test-audit-records` |
| branch | `claude/test-audit-capture-runs-handoff` | this handoff, cut from the audit tip `4f9f2d3f25405c9c8657f815851fa91c7bc6fa96`; fast-forward the audit branch to it | `git log claude/test-audit-records..claude/test-audit-capture-runs-handoff` |
| commit | `4f9f2d3f25405c9c8657f815851fa91c7bc6fa96` | the audit branch's tip: the log entry for the capture runs; not on main | `git show --stat` |
| commit | `9649c468c12d731a415a04c9d4d9a4a01267c42b` | the capture outputs R2, R3, R4a, R5, R6 and R8, 830 files, no `.db`; not on main | `git show --stat` |
| commit | `7883080fa0ea71fc7b7df2fbf855bdcaa73996a0` | R1's 116 dumps; not on main | `git show --stat` |
| commit | `c68baa41a024d5272e5a2c7b46c5553c0c74dcf2` | `capture/CLEAN.md`, the 397 clean records and their runs; not on main | `git show --stat` |
| commit | `8e79b2ce55d279f2172eb361352b7d647641b6e8` | `capture/RUNS.md`, the nine runs; not on main | `git show --stat` |
| commit | `16cf8ed47faf63c04dad915fd18c2b5cf7b93394` | `counts/TOTALS.md`, the recount's totals; not on main | `git show --stat` |
| commit | `d297e3cb5006f370498c49d83108bca675221806` | three test rulings filed by the user; on main, not pushed | `git branch --contains` |
| commit | `4712839817b2e80b97aeb422063d357a696f2123` | the "nothing fake" ruling filed by the user; main's tip; on main, not pushed | `git rev-parse main` |
| path | `docs/agents/rulings/tests-and-runs.md` | on main, the four rulings under `## Confirmed`, dated 2026-11-15; the audit branch's copy predates them | `git show main:docs/agents/rulings/tests-and-runs.md` |
| path | `docs/audit/2026-11-14-test-audit/cross-findings.md` | X1 to X12 and "The Keep recount"; no exit chosen | read it |
| path | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md` | 1,861 of 2,449 records faulty; the unsettled rows | read it |
| path | `docs/audit/2026-11-14-test-audit/capture/CLEAN.md` | 397 clean Investigate records, each with its run or mark | read it |
| path | `docs/audit/2026-11-14-test-audit/capture/RUNS.md` | the run commands R1 to R9; R8's command does not import in isolated mode | read it |
| path | `docs/audit/2026-11-14-test-audit/capture/runs/index.md` | what each capture run did and returned | read it |
| path | `docs/audit/2026-11-14-test-audit/capture/runs/R8/reflog_replay.txt` | the failing reflog replay, 65 refused lines | read it |
| path | `docs/audit/2026-11-14-test-audit/briefs/T5-keep-recount.md` | the recount brief, with the fake list and the input choice | read it |
| path | `docs/audit/2026-11-14-test-audit/log.md` | the session's three log sections | read it |
| test | `GATES_REFLOG_REPLAY=1 python -I -m unittest discover -t . -s gates/tests -p test_review_rules.py -k ReflogReplay` (from `C:\Users\Patrick\.claude`) | last run printed `FAILED (failures=1)` after 1 test in 115.9 s | run it |
| ruling | `63c70014` | "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation" | `tests-and-runs.md`, Confirmed |
| ruling | `63c70014` | "tests should not be pinned to typed constants. they should be pinned to the behavior of the code." | `tests-and-runs.md`, Confirmed |
| ruling | `63c70014` | "tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong" | `tests-and-runs.md`, Confirmed |
| ruling | `63c70014` | "tests should test their intended behavior directly, not make up an arbitrary meaningless string" | `tests-and-runs.md`, Confirmed |
| ruling | `63c70014` | "No, input is fine" (hand-built input fed to the real code is not fake; a question-box choice, not filed) | `briefs/T5-keep-recount.md` quotes it |
| state | the R4b to R4h scripts do not exist | no capture for those 61 clean records | `Get-ChildItem docs/audit/2026-11-14-test-audit/capture/runs` |
| state | no clean record has been re-judged against the captures | the capture outputs are unread by any re-judge | `git log --oneline -- docs/audit/2026-11-14-test-audit/capture` |
| state | session cards for e633f7f6 and 63c70014 are not written | `docs/sessions/` holds no card for either | `Get-ChildItem docs/sessions -Filter *e633f7f6*` |
| state | the worktree `worktrees/test-audit-cal2` and branch `claude/early-fill-reach-slack-handoff` are unresolved | carried over from the last handoff | `git worktree list` |

## Checker output

```
ok | branch | claude/test-audit-records | the audit's branch, 399 commits ahead of main; not merged to main; it does not hold main's two rulings commits
ok | branch | claude/test-audit-capture-runs-handoff | this handoff, cut from the audit tip `4f9f2d3f25405c9c8657f815851fa91c7bc6fa96`; fast-forward the audit branch to it
not-on-main | commit | 4f9f2d3f25405c9c8657f815851fa91c7bc6fa96 | the audit branch's tip: the log entry for the capture runs; not on main
not-on-main | commit | 9649c468c12d731a415a04c9d4d9a4a01267c42b | the capture outputs R2, R3, R4a, R5, R6 and R8, 830 files, no `.db`; not on main
not-on-main | commit | 7883080fa0ea71fc7b7df2fbf855bdcaa73996a0 | R1's 116 dumps; not on main
not-on-main | commit | c68baa41a024d5272e5a2c7b46c5553c0c74dcf2 | `capture/CLEAN.md`, the 397 clean records and their runs; not on main
not-on-main | commit | 8e79b2ce55d279f2172eb361352b7d647641b6e8 | `capture/RUNS.md`, the nine runs; not on main
not-on-main | commit | 16cf8ed47faf63c04dad915fd18c2b5cf7b93394 | `counts/TOTALS.md`, the recount's totals; not on main
not-pushed | commit | d297e3cb5006f370498c49d83108bca675221806 | three test rulings filed by the user; on main, not pushed
not-pushed | commit | 4712839817b2e80b97aeb422063d357a696f2123 | the "nothing fake" ruling filed by the user; main's tip; on main, not pushed
ok | path | docs/agents/rulings/tests-and-runs.md | on main, the four rulings under `## Confirmed`, dated 2026-11-15; the audit branch's copy predates them
ok | path | docs/audit/2026-11-14-test-audit/cross-findings.md | X1 to X12 and "The Keep recount"; no exit chosen
ok | path | docs/audit/2026-11-14-test-audit/counts/TOTALS.md | 1,861 of 2,449 records faulty; the unsettled rows
ok | path | docs/audit/2026-11-14-test-audit/capture/CLEAN.md | 397 clean Investigate records, each with its run or mark
ok | path | docs/audit/2026-11-14-test-audit/capture/RUNS.md | the run commands R1 to R9; R8's command does not import in isolated mode
ok | path | docs/audit/2026-11-14-test-audit/capture/runs/index.md | what each capture run did and returned
ok | path | docs/audit/2026-11-14-test-audit/capture/runs/R8/reflog_replay.txt | the failing reflog replay, 65 refused lines
ok | path | docs/audit/2026-11-14-test-audit/briefs/T5-keep-recount.md | the recount brief, with the fake list and the input choice
ok | path | docs/audit/2026-11-14-test-audit/log.md | the session's three log sections
manual | test | GATES_REFLOG_REPLAY=1 python -I -m unittest discover -t . -s gates/tests -p test_review_rules.py -k ReflogReplay | last run printed `FAILED (failures=1)` after 1 test in 115.9 s
manual | ruling | 63c70014 | "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation"
manual | ruling | 63c70014 | "tests should not be pinned to typed constants. they should be pinned to the behavior of the code."
manual | ruling | 63c70014 | "tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong"
manual | ruling | 63c70014 | "tests should test their intended behavior directly, not make up an arbitrary meaningless string"
manual | ruling | 63c70014 | "No, input is fine" (hand-built input fed to the real code is not fake; a question-box choice, not filed)
manual | state | the R4b to R4h scripts do not exist | no capture for those 61 clean records
manual | state | no clean record has been re-judged against the captures | the capture outputs are unread by any re-judge
manual | state | session cards for e633f7f6 and 63c70014 are not written | `docs/sessions/` holds no card for either
manual | state | worktrees/test-audit-cal2 | carried over from the last handoff
checked 29 rows: 11 ok, 0 missing, 6 not-on-main, 2 not-pushed, 10 manual
```
