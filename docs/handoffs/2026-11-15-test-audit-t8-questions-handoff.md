# Test-suite audit: T6 and T7 done, T8 planned; next, the plan's questions (2026-11-15)

Session 7a62df56-50df-4039-9626-291bb3bfa8d1. It continued `docs/handoffs/2026-11-15-test-audit-t8-exits-plan-handoff.md`, which this branch removes because its work is done (read it with `git show 1672c773813a3babf84b91c9c102462cb5796b7d:docs/handoffs/2026-11-15-test-audit-t8-exits-plan-handoff.md`).

## Goal

The goal is to finish the audit of every test in Hydra's suite, by the approved plan `docs/superpowers/plans/2026-11-14-test-suite-audit.md`. The audit is read-only: it changes no test and no code. Its files are in `docs/audit/2026-11-14-test-audit/` (AUD below).

The next session starts by asking the user the T8 plan's six questions. No T8 task runs before the user has answered them and said yes to the plan.

## What was done this session

The user said "start the t8 plan now, run t6 and t7 in parallel, incorporate your findings into the plan when they finish". Three things ran at once.

**T6** changed the test charts and ran the full suite each time, in a throwaway worktree cut from main. Before any change, all 2,280 test cases passed. With a copy of one chart added, three tests went red, each pinning a count of corpus charts or folders. With the `test_path_reduction` folder removed instead, one went red. All three tests already had Rewrite records that called their number a corpus count, so T6 confirms those records by a run. The write-up is `AUD/mechanical/corpus.md`.

**T7** planted one bug that each auditor named in question 8, and ran the test the record names. Every plant sat behind a switch (the `HYDRA_T7` environment variable), so one build held every plant and each test ran with only its own bug on. With every switch off, every named test passed. There are 112 slice files; 98 got a plant and 14 got none. Of the 98, 91 went red at a check the record names. Six stayed green: A3, A9, A30, C7, WN3 and WN4a. In each, the plant is the record's own change and the test cannot see it, so the auditor's claim did not hold. WN8 crashed before any check, and WN4b went red but one named line stayed green. The write-up is `AUD/mechanical/planted.md`. For most of the 91, "named" compared the failing line with a scout's copy of the record's line, not a fresh read of each record.

**The T8 plan** was written by a planner agent, then the main session filled in the T6 and T7 results and added a first step: eight Sonnet verifiers re-read every question 8 answer in slices A3, A9, A30, C7, WN3, WN4a, WN4b and WN8, as the approved plan's T7 rule says. One fresh reviewer read the plan, fixed one thing (the eight verifiers launch four to a round) and signed it off CLEAN. The plan is `docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md`.

Housekeeping: session cards for e633f7f6 and f893fd7e were written and committed on main.

## The user's words this session

"start the t8 plan now, run t6 and t7 in parallel, incorporate your findings into the plan when they finish"

"create handoff and start with the questiosn for me next session"

The user has not yet said yes to the T8 plan.

## What is next

Ask the user the six questions in the plan's "Questions" section, in question boxes, before anything else. They are:

1. Does the choice "No, input is fine" stand, and will the user file it? It is not under `## Confirmed` in `docs/agents/rulings/tests-and-runs.md`, and it decides 49 fault marks (`AUD/counts/TOTALS.md` section 3).
2. Eight tests in `tools/ch_mcp/tests/test_frida_attach.py` feed hand-typed source lines to a scanner defined in the test file itself. Is that fake code?
3. Seven tests in `tools/test_compare_db.py` feed the real compare tool hand-written SQL stores. Input, or a stand-in for Hydra's stored rows?
4. About 25 tests in the hooks repo's `test_review_dispatch.py` feed a hand-typed hook payload to the real hook. Input, or a stand-in? This is the largest block.
5. `tools/ch_mcp/tests/test_game_tools.py` 616 to 674 reads hand-written stats and score JSON, and `test_fits_screen` reads hand-typed saved-window text. Input, or a stand-in?
6. The size of T8, and with it the yes to the plan.

Each question follows `docs/agents/audit-method.md`, stage 7. Before questions 2 to 5 go out, open each test and its real input on that day. Quote what the test feeds the code, give one example from each end, name the options without leaning, then give a recommendation and the strongest case against it. Send them in small groups, five or fewer to a message. Question 1 comes first, because questions 2 to 5 rest on what "input" means. A line the user wants filed as a ruling is filed only through the steps in `docs/agents/rulings/README.md`.

Once the user says yes, T8 starts at its E0 step: the eight question 8 re-reads, four verifiers to a round.

Housekeeping left:
- **Session cards.** None yet for 63c70014, f9b4f64e or this session, 7a62df56. The card tool treated 63c70014 as still running.
- **Throwaway T6 and T7 worktrees.** `worktrees/test-audit-t6`, `worktrees/test-audit-t7` and the six `worktrees/test-audit-t7-p*` worktrees, plus the hooks worktree `C:\Users\Patrick\.claude\worktrees\test-audit-t7-hooks`. They hold the planted bugs and must never be merged. They can be removed once the user agrees.
- **Older branches and worktrees** carried from earlier handoffs: `worktrees/test-audit-cal2`, `worktrees/test-audit-capture-handoff`, `worktrees/test-audit-capture-runs-handoff`, `worktrees/test-audit-t8-plan-handoff`, and branch `claude/early-fill-reach-slack-handoff`.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| branch | `claude/test-audit-t8-questions-handoff` | this handoff, cut from the plan branch's tip; it removes the previous handoff | `git log claude/test-audit-t8-plan..claude/test-audit-t8-questions-handoff` |
| branch | `claude/test-audit-t8-plan` | the T8 plan and the reviewer's fix, on top of the whole audit branch; not merged to main | `git log main..claude/test-audit-t8-plan` |
| branch | `claude/test-audit-records` | the audit branch, at a4ad3306446b83d60975b745b8ac89893594450b, with the T6 and T7 write-ups; not merged to main | `git rev-parse claude/test-audit-records` |
| commit | `1672c773813a3babf84b91c9c102462cb5796b7d` | the plan branch's tip: the reviewer's one fix (verifiers four to a round); signed off CLEAN; not on main | `git show --stat` |
| commit | `b6fd0e7587665c6d7013f1de6ac279cb00b6e3c4` | the main session filled in T6 and T7 results and added the E0 re-read to the plan; not on main | `git show --stat` |
| commit | `1a6445d4a87969dba671335a3085f29588796cec` | the planner's first version of the T8 plan; not on main | `git show --stat` |
| commit | `6aa6d6843715ba8bee52f367c3cb1602b7d8803a` | T6 write-up, `AUD/mechanical/corpus.md`; not on main | `git show --stat` |
| commit | `a4ad3306446b83d60975b745b8ac89893594450b` | T7 write-up, `AUD/mechanical/planted.md`; not on main | `git show --stat` |
| commit | `32bc9b21601fdaddd6c5e6ee8bbb2c0262019064` | session cards for e633f7f6 and f893fd7e and the rebuilt index; main's tip; on main, not pushed | `git rev-parse main` |
| branch | `claude/test-audit-t7` | throwaway: every T7 plant behind `t7_bug`, built; never merge | `git log --oneline -3 claude/test-audit-t7` |
| branch | `claude/test-audit-t6` | throwaway T6 worktree's branch, at main's old tip with no commits; its source is clean, but its build was last made with A28's constant change and never rebuilt, so never use that build | `git rev-parse claude/test-audit-t6` |
| path | `docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md` | the T8 plan; its "Questions" section holds the six questions; not yet approved | read it |
| path | `docs/audit/2026-11-14-test-audit/mechanical/corpus.md` | T6: 2,280 cases green at baseline; three tests red with a chart added, one with a folder removed | read it |
| path | `docs/audit/2026-11-14-test-audit/mechanical/planted.md` | T7: 98 planted, 91 red at a named check, six green (A3, A9, A30, C7, WN3, WN4a), WN8 crashed, WN4b one green named line | read it |
| path | `docs/audit/2026-11-14-test-audit/counts/TOTALS.md` | section 3 holds the 49 fault marks question 1 decides; section 6 the four blocks behind questions 2 to 5 | read it |
| path | `docs/audit/2026-11-14-test-audit/briefs/T5-keep-recount.md` | line 17 reports the choice "No, input is fine" | read it |
| path | `docs/agents/audit-method.md` | stage 7 says how each question is written | read it |
| path | `docs/agents/rulings/README.md` | how a ruling is filed | read it |
| state | the user has not approved the T8 plan | the last ask went unanswered; the user asked for this handoff instead | this session's card, once written |
| state | no line under `## Confirmed` in main's `docs/agents/rulings/tests-and-runs.md` holds "input is fine" | why question 1 exists | `git show main:docs/agents/rulings/tests-and-runs.md` |
| state | `AUD/mechanical/reread/` and `AUD/exits.md` do not exist | no T8 task has run | `Test-Path` on each |
| state | session cards for 63c70014, f9b4f64e and 7a62df56 are not written | `docs/sessions/` holds no card for them | `Get-ChildItem docs/sessions -Filter *63c70014*` |
| state | the throwaway worktrees `worktrees/test-audit-t6`, `worktrees/test-audit-t7`, `worktrees/test-audit-t7-p1`, `-p1b`, `-p2a`, `-p2b`, `-p3`, `-p4` and `C:\Users\Patrick\.claude\worktrees\test-audit-t7-hooks` exist | they hold planted bugs; never merge them | `git worktree list` |
| ruling | `7a62df56` | "start the t8 plan now, run t6 and t7 in parallel, incorporate your findings into the plan when they finish" | this session's card, once written |
| ruling | `7a62df56` | "create handoff and start with the questiosn for me next session" | this session's card, once written |
| ruling | `63c70014` | "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation" | `tests-and-runs.md` on main, Confirmed |

## Checker output

```
ok | branch | claude/test-audit-t8-questions-handoff | this handoff, cut from the plan branch's tip; it removes the previous handoff
ok | branch | claude/test-audit-t8-plan | the T8 plan and the reviewer's fix, on top of the whole audit branch; not merged to main
ok | branch | claude/test-audit-records | the audit branch, at a4ad3306446b83d60975b745b8ac89893594450b, with the T6 and T7 write-ups; not merged to main
not-on-main | commit | 1672c773813a3babf84b91c9c102462cb5796b7d | the plan branch's tip: the reviewer's one fix (verifiers four to a round); signed off CLEAN; not on main
not-on-main | commit | b6fd0e7587665c6d7013f1de6ac279cb00b6e3c4 | the main session filled in T6 and T7 results and added the E0 re-read to the plan; not on main
not-on-main | commit | 1a6445d4a87969dba671335a3085f29588796cec | the planner's first version of the T8 plan; not on main
not-on-main | commit | 6aa6d6843715ba8bee52f367c3cb1602b7d8803a | T6 write-up, `AUD/mechanical/corpus.md`; not on main
not-on-main | commit | a4ad3306446b83d60975b745b8ac89893594450b | T7 write-up, `AUD/mechanical/planted.md`; not on main
not-pushed | commit | 32bc9b21601fdaddd6c5e6ee8bbb2c0262019064 | session cards for e633f7f6 and f893fd7e and the rebuilt index; main's tip; on main, not pushed
ok | branch | claude/test-audit-t7 | throwaway: every T7 plant behind `t7_bug`, built; never merge
ok | branch | claude/test-audit-t6 | throwaway T6 worktree's branch, at main's old tip with no commits; its source is clean, but its build was last made with A28's constant change and never rebuilt, so never use that build
ok | path | docs/superpowers/plans/2026-11-15-test-audit-t8-exits.md | the T8 plan; its "Questions" section holds the six questions; not yet approved
ok | path | docs/audit/2026-11-14-test-audit/mechanical/corpus.md | T6: 2,280 cases green at baseline; three tests red with a chart added, one with a folder removed
ok | path | docs/audit/2026-11-14-test-audit/mechanical/planted.md | T7: 98 planted, 91 red at a named check, six green (A3, A9, A30, C7, WN3, WN4a), WN8 crashed, WN4b one green named line
ok | path | docs/audit/2026-11-14-test-audit/counts/TOTALS.md | section 3 holds the 49 fault marks question 1 decides; section 6 the four blocks behind questions 2 to 5
ok | path | docs/audit/2026-11-14-test-audit/briefs/T5-keep-recount.md | line 17 reports the choice "No, input is fine"
ok | path | docs/agents/audit-method.md | stage 7 says how each question is written
ok | path | docs/agents/rulings/README.md | how a ruling is filed
manual | state | the user has not approved the T8 plan | the last ask went unanswered; the user asked for this handoff instead
manual | state | ## Confirmed | why question 1 exists
manual | state | AUD/mechanical/reread/ | no T8 task has run
manual | state | session cards for 63c70014, f9b4f64e and 7a62df56 are not written | `docs/sessions/` holds no card for them
manual | state | worktrees/test-audit-t6 | they hold planted bugs; never merge them
manual | ruling | 7a62df56 | "start the t8 plan now, run t6 and t7 in parallel, incorporate your findings into the plan when they finish"
manual | ruling | 7a62df56 | "create handoff and start with the questiosn for me next session"
manual | ruling | 63c70014 | "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation"
checked 26 rows: 12 ok, 0 missing, 5 not-on-main, 1 not-pushed, 8 manual
```
