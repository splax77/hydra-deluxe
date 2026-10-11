# Handoff: the concept duplication audit (2026-10-11)

**For the next session: start the audit at phase 1.** On 2026-10-11 the user read a summary of the plan and replied "go with recommendations". That yes covers the whole plan, including launching its scouts and verifiers. The decisions are D113 in `docs/audit/2026-10-03-fix-decisions.md`. Nothing has run yet.

The plan is `docs/superpowers/plans/2026-10-11-concept-duplication-audit.md`. Read it first. This note only records what the plan can't: where things stand, what was checked, and the practical traps.

## Why this exists

The derive-once reviews catch a rule written twice when the copies share inputs. They miss a module that builds its own version of an idea under its own names. The user's example was the path report against the dmleaderboards (DM) report. The audit looks for one idea with two homes across all of `src/`, which is 181 `.cpp` and `.h` files and 39,724 non-blank lines (counted in session 8bffee2b).

## What was checked in session 8bffee2b

The main session checked the plan's three worked examples against the code before showing it to the user.

The "Not analyzed" clash holds, and it is sharper than first thought. The library's word comes from `status_label` in `src/ui/library_model.cpp`, which says "Stale" for a stale row. The DM page joins on `records_by_hash` in `src/app/report.cpp`, which reads `list_records`. The comment on `list_records` in `src/store/record_store.h` says it returns only Ready rows and leaves a stale chart out. So a stale chart reads "Stale" in the library and "Not analyzed (in your library)" on the DM page. Phase 4 still writes both truth tables for it, and phase 5 asks the user.

The two `settings_change_touches` functions (in `src/app/report.cpp` and `src/app/dm_report.cpp`) and their one caller in `src/ui/app_state.cpp` are as the plan says. The paired column helpers (`text_column`, and `ms_key` against `optional_key`) are as the plan says too.

Two corrections were made to the plan before it was committed. Its line counts are non-blank lines, not all lines (`wc -l` gives 44,999). Its phase 1 said the probe scripts were saved in scratch; they were not, so the next session writes `leads.ps1` from the plan's five probe descriptions.

## How to start

Phase 1 is the main session's own job. Write `docs/audit/2026-10-11-concept-audit/leads.ps1` from the five probes in the plan, run it once, and write `leads.md` with each probe's count at the top. The planner's counts were 56 repeated strings (17 crossing a top-level folder), 9 twice-defined function names, 17 struct pairs, 5 store methods with two or more callers and 4 twice-used type names. A large gap from those goes into `leads.md` with a reason.

Before dispatching phase 2, run the plan's coverage check: every `src/` file must sit in some group's list. Fix the group table if one is missing.

Then launch wave A together: scouts G1a, G1b, G2, G3, G4a, G5, G6a and G7a, plus inventory scouts C1 and C2. That's ten Sonnet scouts (`subagent_type: scout`, `model: sonnet`). Wave B (the other eight groups) waits for wave A and gets its candidate list. Each brief starts with `docs/agents/brief-preamble.md` and then gives the task id, the session id, the status rule, the group's files, its phase 1 leads, the finding kinds from the plan, and the output shape.

Scouts are read-only for the repo. Have each one write its report into its own scratch folder, then copy the file into `docs/audit/2026-10-11-concept-audit/groups/` with a shell copy. That keeps eighteen reports out of the main session's context. Read a report only to merge candidates in phase 4.

Phase 7's gate work (kind F in `docs/agents/derive-once-review.md`, check 5 in `tools/derive_once_precheck.ps1` with its self-test) can start once phase 3 is done. It doesn't wait for the fixes. The precheck change is code, so it goes to an Opus executor in a worktree and gets a derive-once review.

## What still needs the user

Phases 1 to 4 need nothing more from the user. Phase 5 does: the K2 findings and any K1 fix that changes displayed text, a stored record or a score become questions in `questions.md`. The user-visible status words for the library, the DM page and the fill page go to the user as one batch, before any fix wave (D113 item 7). The fix wave in phase 6 starts only after the user answers those and says yes to the wave.

## Journal state at handoff

The hook's journal listed this when the handoff was written:

```
Subagents active in the last 30 minutes and not finished:
  agent-a6a4653bebe64096b: unfinished, last tool call SubagentHandback, touched 11:35
```

That agent was the planner. Its last call was its hand-back, and the main session received its report and committed its plan (4c0000df). Nothing else was launched.
