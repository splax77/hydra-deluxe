# Handoff: phase 6, the duplicate folds (2026-10-04)

Written by session 1f13cd5d at the user's request. The plan is written and committed, but the user has **not** answered its questions or approved it. Nothing from phase 6 has been launched. Don't start any work until the user says yes.

## What phase 6 is

Phase 6 is step 6 of the derivation audit. Step 6 covers the places where one fact is worked out in two or more places that agree today. Each fact gets one owner function. The other copies call that owner, and a row in `tests/test_single_owner.cpp` stops the copy coming back. Where the second copy is a test that re-implements production code, the copy is deleted and the test pins a literal from one run instead (D42). Nothing changes a score, a path or a stored result, so every merge proves itself with byte-identical corpus scores.

## The files

- The plan is `docs/superpowers/plans/2026-10-04-phase-6.md`. It has 28 tasks in four waves (J1 to J4), a file-collision table, a "Recorded as recommended" list and an appendix of files per wave.
- The questions are in `docs/audit/2026-10-04-phase-6-questions.md`. There are four, answerable as "all recommended".
- The four recheck drafts it was joined from are `docs/audit/2026-10-04-phase6-recheck/a.md` to `d.md`. They hold the per-finding status table with evidence, and the fold paragraphs that the task briefs will draw on.

All of these were committed to main as 6a1bb49 (docs only, not pushed).

## The four open questions

1. Finding 298: stop writing the database's unused `user_version` slot (recommended), or make it the real upgrade gate.
2. Finding 112: size the Score range box with `widest_digits(6)`. It grows by about half a pixel (recommended yes).
3. Findings 76 and R7.14: when two taken fills touch, each lights its own lane colour (recommended yes; no library chart reaches it).
4. Finding 1's second half: the Preview's xN disc stays doubled until the next chord after SP ends. Recommended: leave it and record it as a known gap.

When the user answers, record the answers and the plan's "Recorded as recommended" list in `docs/audit/2026-10-03-fix-decisions.md`, under the next free D numbers. Other sessions are numbering decisions too: D51 is phase 7's and D52 is phase 3's. Grep the file for the highest number first. The plan deliberately names decisions by question, not number.

## Scope and counts

185 findings are in scope: step 6's 182 plus the step-1 leftovers 1, 32 and 128.
- 11 are already fixed and only need their triage rows corrected: 122, 148, 176, 178, 244, 246, 269, 350, R7.29, R7.30, and 153, which lands with phase 3's O1.
- 11 were handed to phase 7, and its planner agreed. E2 has 247 and 335, E1 has 173 and 179, E3 has 180, 245, 243, 324 and 261, and SE1 has 199 and 200.
- 341 is closed as not a fold.
- The other 162 are in tasks.

The handoff's two review notes (the "where does the SP track start" copies and `deact_edge_at`) were already closed by D36's review. The probe-script folds that sat in the six scripts phase 7's PR1 deletes were dropped.

## The joint calendar with phases 3 and 7

Phases 3, 6 and 7 write the same tree, so the waves follow one rule: one writer per file per wave, across all three phases. The phase 7 planner (session "Phase 7 planning") agreed to it, and also that idle files go to whoever is ready.
- **J1** starts on the user's yes, alongside phase 7 wave 1. Its files are untouched by phase 3 wave D and by phase 7 wave 1. It holds all phase 6 work on `analysis.cpp`/`.h` and `work_pool.h`, because phase 7 writes `analysis.*` in both its wave 2 and wave 3.
- **J2** starts after phase 3's M_D and phase 7's M7-1 have both merged. It runs alongside phase 7 wave 2, on phase 7's wave 1 and wave 3 files. Phase 7's wave 3 forks after M6-J2 merges.
- **J3** starts after M7-2. It runs alongside phase 7 wave 3, on phase 7's wave 2 files.
- **J4** starts after M7-3 and finishes before phase 7's ER wave. ER rebases onto it once.

The phase 7 planner checked the appendix and found four collisions. This session agreed to how each is settled, and the phase 7 handoff (`docs/handoffs/2026-10-04-phase7-handoff.md`) carries the same rules:
1. `tests/test_replay.cpp` is edited by both J1-1 and phase 7's T1. T1 forks from J1-1's commit, after SE1's.
2. `tests/test_stream_mix.cpp` is edited by both J1-4 and phase 7's AU1. AU1 forks from J1-4's commit and adds its cases on top. AU1 doesn't touch `stream_mix.cpp`.
3. `tests/test_app_state.cpp` is edited by both J2-4 and phase 7's SE2. SE2 forks from J2-4's commit. The same applies to `tests/test_cli.cpp` (J2-3) if phase 7's ST2 needs a CLI case.
4. Phase 7 moved its "has a scored best path" flag in `summarize_record` into ST2 (its wave 2), so J3-6 owns `record_store.cpp` alone in J3.

Because of 1 and 2, phase 7's wave 1 waits on two phase 6 commits. Launch J1-1 and J1-4 no later than phase 7's wave 1, and tell the phase 7 session their commit hashes as soon as they land. If phase 7's wave 1 is ready before the user approves phase 6, agree with the phase 7 session to swap the order: T1 and AU1 go first, and J1-1 and J1-4 fork from them. Phase 7's PR1 may also move `audio::sniff_format`'s byte check into hydra_core, which touches `CMakeLists.txt` and `decode.cpp`/`.h`. J1 owns none of those, and J2-6 forks after M7-1, so it's fine. Phase 7 adds no test files in its wave 2, so J2-1 has `CMakeLists.txt` to itself.

## State of the other phases when this was written

- **Phase 3.** Wave C is merged (16c836c). Wave D is still on its `claude/p3-*` branches (o1 to o3b, k1a to k5, d1, d2, f1 to f5). M_D is not merged.
- **Phase 7.** The plan is approved (D51). Its handoff (f6515da) says nothing is launched, and its wave 1 briefs are written (4d2ce48). No `claude/p7-*` branch existed yet. J1's check against phase 7 wave 1 rests on phase 7's plan, so the J1 integrator must run `git diff main...claude/p7-<id> --stat` on all nine wave 1 branches before forking and again before merging. The nine are E1, E2, ST1, SE1, T1, AU1, PS1, PR1 and PR2.
- **Phase 0.** M0 is not merged. `tools/derive_once_precheck.ps1` and `tools/new_worktree.ps1` exist only on `claude/p35-p0-1` and `claude/p35-p0-4`. Each review does the precheck step by hand until M0 lands.

## What to do next, once the user says yes

1. Record the decisions (see above), and mark the 11 fixed findings' `status_at_head` in `docs/audit/2026-10-03-fix-triage.json`.
2. Write one brief per J1 task in `docs/superpowers/plans/tasks/p6-<id>.md`, on Fable. Each brief has the shared preamble `docs/agents/brief-preamble.md`, the task's findings, owners, files, test filters and pinned literals. No code listings. Pull the fold details from the recheck drafts.
3. Launch J1 as one Workflow. Each of the six tasks gets an Opus implementer in its own worktree, plus a light Sonnet answer check. J1-6 forks from J1-5's commit. Start at most three cold builds at a time.
4. Merge M6-J1 the way the plan says. Merge main in, run the precheck if it has landed, get a Sonnet derive-once review, then one sweep fix round of two fresh agents. Then run the full suite once and check corpus scores are byte-identical, then merge.

## The user's rules for this phase (given at the start of this session)

1. Agents never run the full test suite. The main session runs it once, when merging. Agents run only the tests relevant to their change.
2. Planning agents run on Fable, code executors on Opus, code reviewers on Sonnet.
3. Anything that changes an app display or a record goes to the user first. Code-only changes go ahead on the recommendation, without input.
4. Agents never stall or go long without reporting. They fail loudly when they hit a problem: a status line every 10 calls or 5 minutes, and a blocked agent stops and reports the exact error.
5. A reviewer agent must clear a derive-once audit before the change is merged.
6. Agents run in parallel workflows, with the main session merging. Waves of parallel agents are fine.

Also from memory: stage files by name, never `git add -A`. Commit trailers go in the last paragraph (`--trailer`). Nothing is pushed; ask before pushing.

## Things a task brief must carry (found during the recheck)

- J1-1 deletes the two `replay.cpp` known-copy rows and the dead `std::min(..., 0.0)` clamp.
- J2-2 writes the Beyond edge against K1a's "more than twice" wording, not the audit's table.
- J2-7 folds the hottest function in analysis, `category_scores`. Corpus scores identical and timing within noise at the merge are its real test. Its per-note pins go in `test_stars.cpp`, because no phase 6 task adds a test file.
- J2-8 confirms `kOnyxDefaults` is unreachable before it drops the `PreviewConfig` initializers.
- J3-1 checks the roughly 30 test sites that build `BackendSqueeze` rows. Its new offset accessor fails loudly on a missing value.
- J3-3 checks whether an activation badge can carry a minus sign before pinning the longest badge.
- J3-7 and J3-8 re-read `tools/ch_probe` after PR1, because PR1 deleted six scripts and moved helpers.
- J4-6's `GetTempPathW` scan row is added by the J4 integrator, once J4-1, J4-2 and J4-4 have switched their own files.

## Loose ends (not phase 6)

The earlier handoff's loose ends still stand. This session pushed nothing; ask the user before pushing. The `s2-proto-*` and `s1-flake` branches need the user's yes before deletion. The `.bak-d46` hook backups are still there. `blink-both.json` stays untracked on purpose.
