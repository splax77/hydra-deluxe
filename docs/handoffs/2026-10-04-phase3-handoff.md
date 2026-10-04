# Handoff: phase 3 (display format), 2026-10-04

You run phase 3 of `docs/superpowers/plans/2026-10-04-phases-3-5.md`: give each displayed fact one owner function, then point every screen at it. Two other sessions run at the same time. Phase 0 and 5 (`docs/handoffs/2026-10-04-phase5-and-phase0-handoff.md`) build the fleet fixes and the shell-path owner. Phase 4 (`docs/handoffs/2026-10-04-phase4-handoff.md`) does the docs. The user approved the plan and answered all 33 questions "all recommended" (D48 in `docs/audit/2026-10-03-fix-decisions.md`, questions in `docs/audit/2026-10-04-phase-3-5-questions.md`). Start straight away.

## What's open

All 49 display findings are open at main. The recheck drafts `docs/audit/2026-10-04-session-audit/step3a.md` and `step3b.md` give, for every finding: what the code does today, with file and line, the owner, the files, and the test cases that cover it. Use them as the source for each task's brief. Each answer's recommended form is what to build. Nothing here may change a score, a path or a stored record; corpus scores byte-identical proves it.

## Your tasks, in three waves

The plan's tables give each task's findings, owned files and "done when". In short:

**Wave C (code-only, starts now).**
- C3b: the typed no-notes error (71) and `hydra_uitest state` printing the chip words (87). `pather.cpp` is at `src/search/pather.cpp`.
- C4a: one `path_identity` (249) and one song-fraction helper (the refactor half of 9).
- C4b: named UI timing constants and colour names, with values unchanged (R7.33, and the code halves of 218 and 136).
- C4c: the budget edge read from `squeeze_budget_ms` (the code half of 94).
- H1: the five shared test fixtures every later test uses. These are a record with a tied top-score variant, a path with zero base score, a chart whose audio runs 5 s past its last note, a batch result arriving for the open chart, and a title made only of tags.

  Join these on `claude/p3-c`. Its derive-once review runs while wave D works.

**Wave D1 (owners).** Fork from `claude/p3-c` once it builds and its tasks' tests pass.
- **O1, core model.** `counted`, `note_label`, `HydraRecord::is_optimal`, `Activation::hardest`, the needs-a-squeeze rule, the ±10 ms named constant, the "(uncounted)" tag and one difficult-floor test.
- **O2, display format.** `format_ms_whole`, `format_percent`, the `clock_str` rounding and `time_left_text`.
- **O3a, parse, search and timing.** `display_title`, `fill_rule_name` and `fill_rule_description`, and `display_tick_at_ms`.
- **O3b, app and UI helpers.** `struck_at`, `progress_fraction`, one ellipsis rule, `stale_text`, the two error mappings, and the Preview's parsed-song key.

  Each owner lands with its test and its scan row.

**Wave D2 (callers, one owner task per file).** These fork from the D1 join.
- K1a: the path report page.
- K1b: the leaderboard and fill pages.
- K2: the Paths tab and song panel.
- K3: the Library, settings bar and batch strip.
- K4a: the Preview view.
- K4b: Preview loading.
- K5: hydra_batch and the Dynamics tab.

**Wave D3.** S1, report-page search, after D2 joins.

Plan task K6 (guide and records) belongs to the phase 4 session, not to you.

## Phase 4 findings that live in your files

You own these code files, so you make these edits as part of the named task:
- the report's Avg multiplier tooltip (6) and subtitle "every mode at the current cap, top N per chart and mode" (104), in K1a;
- the two hydra_batch refusals with the real reason for the 1.0 guard (41) and the closing line "rows for every setting" (106), in K5;
- the comments at `model.h` 624 (6) and 645-646 (47), in O1;
- `path_view.h` 82, "bars banked when you activate" (93), in K2;
- `preview_view.cpp` 308-309 (115), in K4a;
- the "never writes the database the GUI reads" comments in `graph.cpp` 76-77 and `timing.h` 47-53 (41), in O3a. Verify the `tick_at_ms` claim in `timing.h` before rewording it.

The exact sentences are in `step45.md`.

Don't edit `CONTEXT.md`, `docs/UserGuide.md`, `docs/development.md` or any ADR. Phase 4 owns them, including CONTEXT.md line 68 for D48 Q1.

## Files shared with the other sessions

- **`tests/test_single_owner.cpp`.** You add rows. Phase 5 edits the shell-path and `std::filesystem` rules. Whoever merges second rebuilds the file as ours plus their additions, relative to the base.
- **`src/cli/report.cpp` and `src/cli/fillcompare.cpp`.** Phase 5 wraps one `absolute` call in each, a few lines from your empty-report message. Expect a small conflict at merge.
- **Phase 4's docs test** checks backticked names. If your renames break it, fix the docs it names in your merge commit, if you merge second.

## How to run it

- **Execution.** Use one Workflow per wave: Opus implementers, each in its own worktree, and a light Sonnet answer-check per task. The answer-check reads the task brief, the diff and the implementer's evidence, runs only that task's tests once, and doesn't rebuild. Dependents start on the earlier task's commit, not its review. The script exits on its last result. Write each task's short brief under `docs/superpowers/plans/tasks/` first; that planning sub-step runs on Fable.
- **Cold builds.** Start at most three at a time machine-wide; the other sessions build too. Use `tools/new_worktree.ps1` once phase 0 lands it.
- **Agent briefs.** Every brief carries:
  - the status-line rule (`C:\Users\Patrick\.claude\hooks\state\status\<agent id>.md` every 10 calls or 5 minutes, via `status_append.ps1`; an agent learns its ID from the progress hook's first reply);
  - "run only the tests named in your brief, never the full suite";
  - commit with `-m` trailers (Task, Agent, Session, Co-Authored-By), never on stdin;
  - Read with offset and limit, never `sed`, `head` or `tail`;
  - commit before 110 calls;
  - helpers in the foreground;
  - when blocked, stop and report the exact error.
- **Brief content.** Use `docs/agents/brief-preamble.md` once it lands. No code listings in briefs: give owners, files, test case names and the pinned numbers. Every test uses H1's fixtures and the existing helpers. A new test helper defined in two files is a review finding.
- **Test limits.** No task adds a floor, seed, depth or band. If one is needed, the agent stops and reports.
- **GUI checks.** Use `hydra_uitest` by widget label (`docs/agents/ui-testing.md`), never screenshots.
- **Two merges to main, M_C then M_D.** Each goes:
  1. Merge main into your branch.
  2. Run `tools/derive_once_precheck.ps1` if phase 0 has landed it. If not, don't wait; say so.
  3. If the precheck lists any new number or behaviour with no decision, ask the user in one batch before review.
  4. Get a Sonnet derive-once review (`docs/agents/derive-once-review.md`, with the key and range the hook gives).
  5. Run one sweep fix round by kind: two fresh agents, production findings and test findings, 100-call cap, commit per finding.
  6. The main session runs the full suite once, `hydra_uitest` once, and checks the corpus scores are byte-identical.
  7. Merge.
- **Library spot check at M_D.** Saved records stay identical; the results stamp stays "2.1.0".
- **Bugs outside the plan.** A review finding outside the plan goes to the ledger with a library count. It isn't fixed unless the user says "fix now".

## Rules (from the user, in memory)

- Display and record changes go to the user first. D48 settles everything in this plan. If a task finds a display choice D48 doesn't cover, it stops, and you ask the user in one batch with the reason in game terms.
- Code-only work proceeds on your recommendation and gets reported afterwards.
- Agents never run the full suite. The main session runs it once per merge.
- Agents never stall. They report every 10 calls or 5 minutes and fail loudly.
- Planning agents run on Fable, implementers on Opus, reviewers on Sonnet.
- Stage files by name, never `git add -A`. Nothing is pushed; ask before pushing.

## When you're done

Record M_C and M_D's commits in the plan, mark the closed findings in `docs/audit/2026-10-03-fix-triage.json`, and write a short closing handoff for steps 6 and 7.
