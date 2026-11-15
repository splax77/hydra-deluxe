# Test-suite audit: every clean record is judged; next, plan the exits (2026-11-15)

Session f9b4f64e-df75-4ee2-b5f3-9f2bc5dfc7aa. It continued `docs/handoffs/2026-11-15-test-audit-capture-runs-handoff.md`, which this branch removes because its work is done (read it with `git show 7ea131fa8d7b0ddd61af86a13ab7e4ca6e3ea34f:docs/handoffs/2026-11-15-test-audit-capture-runs-handoff.md`). This branch also removes the handoff before that one, `docs/handoffs/2026-11-15-test-audit-capture-pass-handoff.md` (`git show 7ea131fa8d7b0ddd61af86a13ab7e4ca6e3ea34f:docs/handoffs/2026-11-15-test-audit-capture-pass-handoff.md`).

## Goal

The goal is to finish the audit of every test in Hydra's suite, by the approved plan `docs/superpowers/plans/2026-11-14-test-suite-audit.md`. The audit is read-only. It changes no test and no code. Its work is on branch `claude/test-audit-records`, in `docs/audit/2026-11-14-test-audit/` (called AUD below).

The next session's job is to plan T8, the step that gives every finding an exit. An exit is how a finding leaves the audit: fixed, a question to the user, covered by a ruling, or not a problem (`docs/agents/audit-method.md`, "The four exits"). The plan becomes a document that waits for the user's yes before anything runs.

This handoff sits on its own branch, `claude/test-audit-t8-plan-handoff`, cut from the audit branch's tip `7ea131fa8d7b0ddd61af86a13ab7e4ca6e3ea34f`. Fast-forward the audit branch to it before going on.

## What was done this session

The screen captures ran first. The user said "Yes, run them". Read-only scouts drafted one `hydra_uitest` script per screen, and the main session ran them on `testdata/input`: the Library view, a song click and the batch, the settings panel, the Dynamics tab and the Path report. A19 PL13 turned out to test Hydra's own Paths tab, so it got a Paths tab run. All of these passed after the main session fixed several scout lines (commit `f2bdc3186aa6e0a7f7475bed65f42ca3fd4951d2`). The Path Builder's test runner cannot open a chart, so its records have no run.

The user then said the main session had declared things impossible without trying. Two more captures followed (commit `9dd8221dfd013ea2719f44ad69567bdf82b3ce01`). The Dynamics tab ran at Hard through a settings file. A wide search held a whole batch open long enough to capture Analyzing, Pause, Resume and Stop. A single click's Cancel stayed out of reach after four tries, because one analysis ends before the button can be clicked.

Three re-judges then read the runs against their records (`AUD/capture/rejudge/`). Most came back "not shown". The runs had been planned for whole groups of records, so they did not show each record's own claim. The user then said to judge the rest by reading the code. Nine scouts did, batch by batch (`AUD/capture/byreading/`, commit `7ea131fa8d7b0ddd61af86a13ab7e4ca6e3ea34f`). The totals are counted from the verdict files' table rows:

- 34 rows are shown by a capture run.
- 333 hold by reading.
- 20 cannot be told without a run, files outside the repo, or the live game.
- None is contradicted.

The main session checked four "holds by reading" rows against the code, and all four held. `AUD/capture/byreading/SUMMARY.md` has the totals, the 20 cannot-tell rows and the side notes that need an exit. Those side notes include a test that cannot fail, a test title the code does not keep, tests that check no Hydra code, and hook tests that pass with no gate installed. A verdict is not an exit, and no exit has been chosen.

Housekeeping: the missing session card for 16892967 was written and committed on main (`5d88062e279407e44ebd31f542b7e4793e4870d4`). The `hydra_pathbuilder_uitest` runner was rebuilt from main. `AUD/log.md` has this session's section.

## The user's words this session

"Yes, run them" (to: write and run the R4b to R4h capture scripts)

"why are you asking for a re judge? what will this accomplish?"

"okay proceed" (after the answer: re-judge the shown records with few agents, then the exits)

"so instead of figuring out how to capture those you took the lazy route and declared it impossible"

"just proceed without captures for the remaining and use your best judgement with static code analysis"

"create handoff fot plan"

## What is next

First, decide where T6 and T7 stand. The approved plan runs T6, which tests which tests lean on the corpus, and T7, which plants a sample of the auditors' bugs, before T8. Neither has run: `AUD/mechanical/` does not exist. The capture-runs handoff named T8 next and never mentioned them. Ask the user whether T6 and T7 run first, or whether T8's plan goes ahead without them. That is the user's call, not the session's.

Then plan T8 by the plan's own section, "T8: Choose each finding's exit", and `docs/agents/audit-method.md`, stage 5. The plan must cover four kinds of finding:

1. **The faulty records.** 1,861 of the 2,449 records hold a ruled fault (`AUD/counts/TOTALS.md`, section 2, "Both, mine"). By the plan, every Rewrite, Remove and Investigate goes to the user, so most of these are questions. Tests that share one rule share one question (rubric question 10).
2. **The findings X1 to X12** in `AUD/cross-findings.md`, each with every split inside it as its own row.
3. **The side notes in `AUD/capture/byreading/SUMMARY.md`.** These are findings too, so each gets an exit like any other.
4. **The 20 cannot-tell records.** Each goes on the open list with the run that would settle it, or becomes a question. None quietly becomes "holds".

`AUD/counts/TOTALS.md`, section 6, lists four unsettled blocks that hinge on what counts as fake. They are KR3's D2 self-scanner, IR1's hand-written compare stores, IR4's hook payloads and IR8's hand-written JSON. They go to the user as questions before their records get an exit.

The plan also needs these:
- **A Premise map** (`docs/agents/premise-map.md`), so that every fact it stands on names where it was read.
- **One fresh review**, as a committed file on a branch, before the user sees it (`docs/agents/doc-review.md`).
- **The user's yes** before any exit agent starts.

One more question can wait until after T8. The previous handoff suggested asking the user to add X9 to X12 to `docs/agents/test-rubric.md`.

Two housekeeping items remain:
- **Session cards.** None is written for e633f7f6 or 63c70014: the start hook's batch did not include them. This session (f9b4f64e) also needs a card once it ends.
- **Old branches and worktrees.** Branch `claude/early-fill-reach-slack-handoff` and worktree `worktrees/test-audit-cal2` are still unresolved. Worktrees `worktrees/test-audit-capture-handoff` and `worktrees/test-audit-capture-runs-handoff` hold branches that the audit branch now contains.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| branch | `claude/test-audit-records` | the audit's branch, at `7ea131fa8d7b0ddd61af86a13ab7e4ca6e3ea34f`, 403 commits ahead of main; not merged to main | `git log main..claude/test-audit-records` |
| branch | `claude/test-audit-t8-plan-handoff` | this handoff, cut from the audit tip; fast-forward the audit branch to it | `git log claude/test-audit-records..claude/test-audit-t8-plan-handoff` |
| commit | `7ea131fa8d7b0ddd61af86a13ab7e4ca6e3ea34f` | the audit tip: every clean record judged, the by-reading brief and verdicts; not on main | `git show --stat` |
| commit | `9dd8221dfd013ea2719f44ad69567bdf82b3ce01` | Dynamics at Hard, the running batch strip, the R4a and store re-judges; not on main | `git show --stat` |
| commit | `f2bdc3186aa6e0a7f7475bed65f42ca3fd4951d2` | the R4b to R4f screen captures and the Paths tab run; not on main | `git show --stat` |
| commit | `5d88062e279407e44ebd31f542b7e4793e4870d4` | the session card for 16892967; main's tip; on main, not pushed | `git rev-parse main` |
| path | `docs/audit/2026-11-14-test-audit/capture/byreading/SUMMARY.md` | the verdict totals, the 20 cannot-tell rows and the side notes that need an exit | read it |
| path | `docs/audit/2026-11-14-test-audit/capture/runs/index.md` | what every capture run did, including the R4 runs and the main session's fixes | read it |
| path | `docs/audit/2026-11-14-test-audit/capture/rejudge/J1-engine.md` | the engine re-judge: 7 shown, 39 not shown | read it |
| path | `docs/audit/2026-11-14-test-audit/briefs/T5c-by-reading.md` | the brief the nine by-reading scouts followed | read it |
| path | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md` | 1,861 of 2,449 records faulty (section 2); four unsettled blocks (section 6) | read it |
| path | `docs/audit/2026-11-14-test-audit/cross-findings.md` | X1 to X12 and "The Keep recount"; no exit chosen | read it |
| path | `docs/superpowers/plans/2026-11-14-test-suite-audit.md` | the approved plan; T6 and T7 come before T8; T8 is "Choose each finding's exit" | read it |
| path | `docs/agents/audit-method.md` | the four exits and the stage 5 rules T8 follows | read it |
| path | `docs/agents/rulings/tests-and-runs.md` | the user's four test rulings under `## Confirmed`; main's copy is newer than the audit branch's | `git show main:docs/agents/rulings/tests-and-runs.md` |
| path | `docs/audit/2026-11-14-test-audit/log.md` | this session's section, "The screen captures and every clean record judged" | read it |
| state | `docs/audit/2026-11-14-test-audit/mechanical/` does not exist | T6 and T7 have not run | `Test-Path docs/audit/2026-11-14-test-audit/mechanical` |
| state | no `exits.md`, `closures.md` or `questions.md` in the audit folder | T8 to T10 have not started | `Get-ChildItem docs/audit/2026-11-14-test-audit` |
| state | the by-reading verdicts total 34 shown by run, 333 holds, 20 cannot tell, 0 contradicted | counted from the table rows of the verdict files | `SUMMARY.md`, "Totals" |
| state | session cards for e633f7f6 and 63c70014 are not written | `docs/sessions/` holds no card for either | `Get-ChildItem docs/sessions -Filter *e633f7f6*` |
| state | worktree `worktrees/test-audit-cal2` and branch `claude/early-fill-reach-slack-handoff` are unresolved | carried over from the last two handoffs | `git worktree list` |
| ruling | `f9b4f64e` | "just proceed without captures for the remaining and use your best judgement with static code analysis" | this session's card, once written |
| ruling | `f9b4f64e` | "so instead of figuring out how to capture those you took the lazy route and declared it impossible" | this session's card, once written |
| ruling | `f9b4f64e` | "Yes, run them" (to: write and run the R4b to R4h capture scripts) | this session's card, once written |
| ruling | `63c70014` | "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation" | `tests-and-runs.md` on main, Confirmed |
| ruling | `63c70014` | "No, input is fine" (hand-built input fed to the real code is not fake; a question-box choice, not filed) | `briefs/T5-keep-recount.md` quotes it |

## Checker output

```
ok | branch | claude/test-audit-records | the audit's branch, at `7ea131fa8d7b0ddd61af86a13ab7e4ca6e3ea34f`, 403 commits ahead of main; not merged to main
ok | branch | claude/test-audit-t8-plan-handoff | this handoff, cut from the audit tip; fast-forward the audit branch to it
not-on-main | commit | 7ea131fa8d7b0ddd61af86a13ab7e4ca6e3ea34f | the audit tip: every clean record judged, the by-reading brief and verdicts; not on main
not-on-main | commit | 9dd8221dfd013ea2719f44ad69567bdf82b3ce01 | Dynamics at Hard, the running batch strip, the R4a and store re-judges; not on main
not-on-main | commit | f2bdc3186aa6e0a7f7475bed65f42ca3fd4951d2 | the R4b to R4f screen captures and the Paths tab run; not on main
not-pushed | commit | 5d88062e279407e44ebd31f542b7e4793e4870d4 | the session card for 16892967; main's tip; on main, not pushed
ok | path | docs/audit/2026-11-14-test-audit/capture/byreading/SUMMARY.md | the verdict totals, the 20 cannot-tell rows and the side notes that need an exit
ok | path | docs/audit/2026-11-14-test-audit/capture/runs/index.md | what every capture run did, including the R4 runs and the main session's fixes
ok | path | docs/audit/2026-11-14-test-audit/capture/rejudge/J1-engine.md | the engine re-judge: 7 shown, 39 not shown
ok | path | docs/audit/2026-11-14-test-audit/briefs/T5c-by-reading.md | the brief the nine by-reading scouts followed
ok | path | docs/audit/2026-11-14-test-audit/counts/TOTALS.md | 1,861 of 2,449 records faulty (section 2); four unsettled blocks (section 6)
ok | path | docs/audit/2026-11-14-test-audit/cross-findings.md | X1 to X12 and "The Keep recount"; no exit chosen
ok | path | docs/superpowers/plans/2026-11-14-test-suite-audit.md | the approved plan; T6 and T7 come before T8; T8 is "Choose each finding's exit"
ok | path | docs/agents/audit-method.md | the four exits and the stage 5 rules T8 follows
ok | path | docs/agents/rulings/tests-and-runs.md | the user's four test rulings under `## Confirmed`; main's copy is newer than the audit branch's
ok | path | docs/audit/2026-11-14-test-audit/log.md | this session's section, "The screen captures and every clean record judged"
manual | state | docs/audit/2026-11-14-test-audit/mechanical/ | T6 and T7 have not run
manual | state | exits.md | T8 to T10 have not started
manual | state | the by-reading verdicts total 34 shown by run, 333 holds, 20 cannot tell, 0 contradicted | counted from the table rows of the verdict files
manual | state | session cards for e633f7f6 and 63c70014 are not written | `docs/sessions/` holds no card for either
manual | state | worktrees/test-audit-cal2 | carried over from the last two handoffs
manual | ruling | f9b4f64e | "just proceed without captures for the remaining and use your best judgement with static code analysis"
manual | ruling | f9b4f64e | "so instead of figuring out how to capture those you took the lazy route and declared it impossible"
manual | ruling | f9b4f64e | "Yes, run them" (to: write and run the R4b to R4h capture scripts)
manual | ruling | 63c70014 | "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation"
manual | ruling | 63c70014 | "No, input is fine" (hand-built input fed to the real code is not fake; a question-box choice, not filed)
checked 26 rows: 12 ok, 0 missing, 3 not-on-main, 1 not-pushed, 10 manual
```
