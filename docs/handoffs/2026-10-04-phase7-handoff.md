# Handoff: run phase 7 of the derivation fixes (2026-10-04)

Written by the phase 7 planning session (b9839bd7). Phase 7 is planned and approved, but no code has started. The user said not to launch the work from this session. Your job is to launch it, merge it wave by wave, and keep in step with phase 6.

## What to read first

The plan is `docs/superpowers/plans/2026-10-04-phase-7.md`. It lists every task, the files each one owns, the tests it runs, and the merge steps.

The answers are D51 at the end of `docs/audit/2026-10-03-fix-decisions.md`, which answers `docs/audit/2026-10-04-phase-7-questions.md`. The user answered "all recommended", with two notes in their own words. On 4- and 5-note chords: they never appear in a real drums chart, but Clone Hero allows them, so Hydra must count them correctly. On the early-fill window: they tightened it from 85 to 60 ms on purpose, because early fills that never matter were cluttering paths.

Every brief starts from `docs/superpowers/plans/tasks/_phase7-preamble.md`.

All of the above is committed on main as 81a2519.

## Where things stand

Phase 7 is the audit's step 7: 75 findings. Four Fable scouts rechecked them at main. Six were already fixed: 30, 37, 304, 344, 222 and 77. Their rows in `docs/audit/2026-10-03-fix-triage.json` still say open. R7.5 and R7.6 are settled by D27 and D6, and their rows are stale too. **Correcting those eight rows' `status_at_head` is still to do.** The plan's "Decisions written before code" section says the main session does it; D51 itself is written.

The scouts' reports were not saved to the repo. Their transcripts were empty when this session tried to extract them. The facts they found are in the plan's task tables and in the brief writers' prompts below.

Wave 1 briefs: three Fable writers were writing `docs/superpowers/plans/tasks/p7-<id>.md` for E1, E2, ST1, SE1, T1, AU1, PS1, PR1 and PR2 when the user asked for this handoff. They write only those brief files, never code. Journal state at handoff, pasted as the hook gave it:

```
Subagents active in the last 30 minutes and not finished:
  agent-aecaf70e353802eee: unfinished, last tool call Grep, touched 18:28
  agent-af5a9e63c46a65be5: unfinished, last tool call PowerShell, touched 18:28
```

The third writer (E1, E2 and ST1) was also running at 18:27. **Before you launch anything**, check which of the nine `p7-*.md` files exist. Read each one against the plan row, D51 and the "Rules for the brief" in this session's writer brief (copied under "Brief writer rules" below). Write any missing brief with a fresh Fable agent. The briefs are untracked until you commit them.

Phase 3 is still running in another session ("Phase 3 continuation"). Wave C merged as 16c836c; wave D (O1 to K5, then S1) is on the `claude/p3-*` worktrees. Phase 7's wave 2 waits for phase 3's merge M_D.

Phase 6 is being planned in another session ("Phase 6 planning"), and it numbers its decisions from D52.

## The split with phase 6 (agreed by message with the phase 6 planner)

Phase 6 owns the step 6 leftovers (146, 149, 150, 177, 183, R7.11, R7.25, and the test-builder halves of 117, 277 and 286), plus 1, 32 and 128. Phase 7 took eleven of phase 6's folds because they answer a phase 7 task's question:
- E1 took 173 and 179.
- E2 took 247 and 335.
- E3 took 180, 245, 243, 324 and 261.
- SE1 took 199 and 200.

The ordering rule is one writer per file per wave across both phases, and an idle file goes to whichever phase is ready. In practice:
- **During phase 7's wave 1:** phase 6 folds `analysis.cpp`/`.h`. Phase 7 writes those files in both waves 2 and 3, so wave 1 is phase 6's only window.
- **During phase 7's wave 2:** phase 6 folds the wave 3 files: report pages, `library_*`, `app_state`, `uitest_harness` and `cli/batch.cpp`. Phase 7's wave 3 forks after that merge. To keep `app_state.cpp` free in wave 2, ST2 adds the per-difficulty song length beside the old store call, and LB switches `update_song_length` in wave 3.
- **During phase 7's wave 3:** phase 6 folds the wave 2 files: model, path_view, pather, preview_*, settings_bar, paths_tab, timing, record_store and engine.
- **After M7-1:** phase 6 folds the wave 1 files: `tools/ch_probe`, the audio readers, preview_source, scoring, config/strutil/rules_file, and tools/replay and bench.
- **Before wave 4:** all of phase 6 lands, so ER rebases onto the folds once.

The phase 6 planner promised its full per-task file list. If it arrives in a later session, check it against the plan's tables.

## How to run it (the user's global rules)

1. Agents never run the full test suite; they run only the tests their brief names. The main session runs the full suite once, when merging.
2. Planning agents run on Fable, code executors on Opus, and code reviewers on Sonnet.
3. Anything that changes a display or a stored record goes to the user first. D51 covers everything planned; a new display or record choice found mid-task stops the task and goes to the user. Code-only changes go ahead on your recommendation and get reported afterwards.
4. Agents never stall or go quiet. They write status lines and fail loudly when blocked.
5. Every merge to main needs a CLEAN derive-once review from a fresh Sonnet agent first (`docs/agents/derive-once-review.md`).
6. Each wave is one parallel Workflow, and the main session merges.

### Wave by wave

- **Wave 1** (E1, E2, ST1, SE1, AU1, PS1, PR1 from 81a2519, or main's head if only docs moved):
  - Launch all seven together in one Workflow.
  - T1 forks from SE1's first commit, and PR2 from PR1's; launch each as soon as that commit exists.
  - Cold builds go through phase 3's slot helper, which the preamble names, so at most three run at once across both phases.
- **M7-1:**
  - Join on `claude/p7-w1`. If the precheck script (`tools/derive_once_precheck.ps1`, from merge M0) has reached main, run it; otherwise the reviewer does that step by hand.
  - Then the Sonnet derive-once review, then fix rounds until it's CLEAN.
  - E1 and E2 change stored results. Before merging, measure corpus scores and tied or multiplier-squeeze counts against main, and show the user the counts and why, in game terms.
  - Every other task must leave corpus scores byte-identical.
  - The phase 3 scripts `scores.ps1` and `wave_c_tests.ps1` sit in phase 3's scratchpad beside `cold_build.ps1` and can be reused.
- **Wave 2** (E3, ST2, PV, SE2, TM): after phase 3's M_D and M7-1. Write the briefs first with Fable writers, the same way. E3 may change all-0 lists, and the user sees those counts before M7-2.
- **Wave 3:** RP and DOC in parallel; LB forks from RP's commit. M7-3 adds one `hydra_uitest` run by the main session.
- **Wave 4:** ER alone. It goes after M7-3 and after all of phase 6 has landed.

## Brief writer rules (what each brief must satisfy)

- No code listings: name the owners, files, test cases and pinned literals.
- Plain English.
- The owned files are the plan row's files, plus their tests.
- Pinned numbers come only from D51, the plan's Test limits, or values today's code prints.
- Display text uses D51's exact wording.
- Test filters must exist today.
- Each brief stays under about 120 lines, in the format of `tasks/C4c.md`.

## Loose ends from earlier handoffs, still open

- Three empty folders under `.claude/worktrees` (`admiring-fermi-4a140e`, `bold-wilson-6af6e3`, `objective-maxwell-56ed0c`). Delete them once nothing holds them open.
- Unmerged prototype branches (`claude/s2-proto-*`, `claude/s1-flake`). Ask the user before deleting them.
- `docs/handoffs/2026-09-29-public-timing-scripts/blink/results/blink-both.json` is untracked on purpose.
- The hook backup files (`*.bak-d46*`) in `C:\Users\Patrick\.claude\hooks`.
- Nothing from phase 7 is pushed. Ask the user before pushing.
