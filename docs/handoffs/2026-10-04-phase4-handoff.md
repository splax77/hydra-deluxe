# Handoff: phase 4 (docs), 2026-10-04

You run phase 4 of `docs/superpowers/plans/2026-10-04-phases-3-5.md`: make the docs say what the code does, and add a test that fails when they drift. Two other sessions run at the same time. Phase 0 and 5 (`docs/handoffs/2026-10-04-phase5-and-phase0-handoff.md`) build the fleet fixes and the shell-path owner. Phase 3 (`docs/handoffs/2026-10-04-phase3-handoff.md`) does the display work. The user approved the plan and answered every question "all recommended" (D48 in `docs/audit/2026-10-03-fix-decisions.md`). Start straight away.

## What's open

The recheck draft `docs/audit/2026-10-04-session-audit/step45.md` has every finding's status at main, with evidence, and the exact sentences to fix. Four findings are already fixed (46, 48, 96, 114) and five partly (45, 47, 91, 339, R7.12). R7.12 belongs to the phase 5 session; leave it alone.

## Your tasks

**C1, docs tell the truth.** This is code-only and starts now.
- Correct the ADR and comment findings 44, 45, 47 (the ADR 0014 half), 92, 102, 118, R7.38, R7.41, R7.42 and R7.43.
- Write down the two decisions already made: 338 (the Preview audio offset, in CONTEXT's Transport entry) and 351 (the ETA after three charts).
- Add `tests/test_docs_match_code.cpp` with its CMake line. It runs three checks:
  - every backticked code name in the ADRs, CONTEXT.md, the User Guide and development.md exists in `src/`, `tools/` or `tests/`;
  - the guide's `hydra_rules.ini` sample equals the `Rules{}` defaults;
  - every default marked with a comment beside its number in the docs matches the code.

  It skips "Superseded" sections and keeps a short allow-list. Prove it fails on a planted stale name.

**C3a, replay and bench text.** This is dev-tool output, so it counts as code-only.
- 107: hydra_replay's not-analyzed line echoes the parsed settings.
- 110: the squeeze-out warning takes the rules and quotes that chord's real reduction under `sqout_rule`. Add one `whole_chord` two-note case.
- 326: the bench label reads "cap4 d4 no-ms", from `kCloneHeroSpCap`.
- The hydra_replay half of 41: the usage text gives the real reason for the 1.0 guard.

  Files: `src/core/replay.h`/`.cpp`, `tools/replay.cpp`, `tools/bench.cpp`, `tests/test_replay.cpp`.

**K6, guide and records.** This needs the answers, and you have them (D48).
- **Guide wording.** Fix 39, 40, 42, 43, 91 (guide and CONTEXT halves), the guide half of 93 ("bars banked when you activate"), and the guide and development.md halves of 104.
- **Guide text for the new display wording.** Phase 3 changes on-screen wording under D48: the leaderboard chips around line 216, the fill-rule names, "1 bar", and the tied optimal paths. Update the guide's descriptions to match D48's wording, not whatever happens to be on main when you write. CONTEXT.md line 68 ("the first path in a record is optimal") changes to D48 Q1 too. That line is yours, not phase 3's.
- **Record the Q33 numbers.** Write each number into CONTEXT.md or its ADR (0006, 0008, 0014, 0019): R7.31, 310, 333, R7.33, 327, 336, 337, 339, 348, 349, R7.32, R7.34, R7.36, and R7.39 (correct the ADRs: only links matching the first link's channel count play). Name the magenta fallback once in `src/render/preview_config.cpp` and the "OpusHead" tag once in `src/audio/decode.h`.
- **R7.37** waits on the audit's open FLAC length-0 and Opus seek-after-damage calls. If the user hasn't answered those, leave that paragraph and say so.
- Mark the documented defaults (10 ms on, 50 ms off, 3 ms leeway, 500 ms squeeze window) so the docs test checks them.

C1 and K6 both edit CONTEXT.md and ADR 0019. Run them as one lane, C1 first then K6, or as two lanes where K6 starts from C1's commit.

## What is not yours

Some phase 4 findings live in code files the phase 3 session owns, so phase 3 makes those edits:
- the report tooltip (6) and the report subtitle (104), in `report.cpp`;
- the hydra_batch refusals and closing line (41, 106), in `cli/batch.cpp`;
- the comments at `model.h` 624 and 645-646 (6, 47), `path_view.h` 82 (93), `preview_view.cpp` 308-309 (115), and in `graph.cpp`/`timing.h` (41).

Don't edit any file under `src/` except the ones listed above as yours. ADR 0020 and `tests/test_single_owner.cpp`'s shell-path and filesystem rules belong to the phase 5 session.

Your docs test reads names that phase 3 may rename later. Whichever session merges second runs the docs test on the joined tree and fixes what it broke in its merge commit.

## How to run it

- **Execution.** Use one Workflow with Opus implementers in their own worktrees from main, and Sonnet reviewers. Your task brief files go under `docs/superpowers/plans/tasks/`; any planning sub-step there runs on Fable.
- **Agent briefs.** Every brief carries:
  - the status-line rule (`C:\Users\Patrick\.claude\hooks\state\status\<agent id>.md` every 10 calls or 5 minutes, via `status_append.ps1`);
  - "run only the tests relevant to your change, never the full suite";
  - commit with `-m` trailers (Task, Agent, Session, Co-Authored-By), never on stdin;
  - Read with offset and limit, never `sed`, `head` or `tail`;
  - commit before 110 calls;
  - helpers in the foreground;
  - when blocked, stop and report the exact error.
- **Phase 0's briefs.** Once `docs/agents/brief-preamble.md` lands on main, use it.
- **Cold builds.** Start at most three at a time. Docs-only lanes don't need a build.
- **Merge.** Your merge to main touches `src/`, `tools/` and `tests/`, so it needs the derive-once review:
  1. Merge main into your branch.
  2. Run `tools/derive_once_precheck.ps1` if phase 0 has landed it. If it hasn't, don't wait; say so in the review request.
  3. Get a Sonnet derive-once review (`docs/agents/derive-once-review.md`).
  4. If there are findings, run a sweep fix round by kind.
  5. The main session runs the full suite once.
  6. Merge.

  Corpus scores byte-identical proves nothing moved.

## Rules (from the user, in memory)

- Display and record changes go to the user first. D48 already covers everything listed here. If a fix needs wording D48 doesn't settle, ask in one batch.
- Code-only work proceeds on your recommendation and gets reported afterwards.
- Agents never run the full suite. The main session runs it once per merge.
- Agents never stall. They report every 10 calls or 5 minutes and fail loudly.
- Planning agents run on Fable, implementers on Opus, reviewers on Sonnet.
- Stage files by name, never `git add -A`. Nothing is pushed; ask before pushing.

## When you're done

Record phase 4's merge commit in the plan. Note in `docs/audit/2026-10-03-fix-triage.json` (status field) which findings closed.
