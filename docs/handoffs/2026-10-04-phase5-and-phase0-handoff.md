# Handoff: phase 5 (shell path) and phase 0 (fleet fixes), 2026-10-04

You run phase 0 and phase 5 of `docs/superpowers/plans/2026-10-04-phases-3-5.md`. Two other sessions run phases 3 and 4 at the same time, from `docs/handoffs/2026-10-04-phase3-handoff.md` and `docs/handoffs/2026-10-04-phase4-handoff.md`. The user approved the plan and answered every question "all recommended" (D48 in `docs/audit/2026-10-03-fix-decisions.md`). Start straight away; you don't need to ask anything first.

You go first in spirit. The other two sessions' reviews lean on your pre-review script (P0-1), and every agent in all three sessions runs under your hooks (P0-3). Land P0-3 and P0-1 as early as you can.

## Why phase 0 exists

The audit of steps 1 and 2 (`docs/audit/2026-10-04-step1-2-session-audit.md`) found the time went to five things: review rounds that a script could have shortened, questions asked mid-review, the fleet freezing on permission dialogs, repeated full test suites, and cold builds piling up. Phase 0 turns those lessons into checks that enforce themselves. The slice reports in `docs/audit/2026-10-04-session-audit/` hold the evidence. In particular, `merge-gate.md` section 4 lists every review finding by kind, which P0-1 is tested against.

## Your tasks

The plan's Phase 0 table and task C2 give the files, owners and "done when" for each.
- **P0-1, the pre-review script** (`tools/derive_once_precheck.ps1`). It lists test helpers defined in two files, recompute spellings, new numbers with no decision, and scans outside `tests/test_single_owner.cpp`. Prove it on the commits steps 1 and 2 sent to their first reviews. It must flag at least 25 of the 29 findings the merge-gate audit called script-findable.
- **P0-2, the briefs.** These are `docs/agents/brief-preamble.md`, the updated `derive-once-review.md`, a new `fix-round.md` and a new `integrate.md`. They're docs only, so they commit straight to main. Do these first. The other two sessions will use them once they land.
- **P0-3, the hooks**, in `C:\Users\Patrick\.claude\hooks`:
  - Deny unfiltered `hydra_tests.exe`, `hydra_uitest --all`, and more than two `test_h*.ps1` runs from subagents. The main session's merge step passes.
  - Find and fix the two hooks that answered "ask" in step 2: a `cat >>` heredoc append and a plain Read. See `step2-exec.md` section 2.
  - Pre-allow Edit and Write under `docs/audit`, `docs/handoffs` and `.superpowers`.
  - Add an empty-fleet warning to the watchdog.
  - Set the status monitor to 10 minutes, sending only changed lines.

  Every hook change ships with its test under `hooks\tests`. Hooks fail loudly: deny with a reason, never "ask". Run only the hook test files you changed, never the whole hook suite more than once.
- **P0-4, warm worktrees.** Measure seeding a worktree from an existing `build-cpp` against a cold build, on a quiet machine. The other sessions are building, so pick a quiet moment, or measure in one go and say how loaded the box was. Keep `tools/new_worktree.ps1` doing whichever wins. If seeding doesn't save half, the helper just staggers cold builds machine-wide to at most three at once.
- **C2 / phase 5, the shell path.**
  - Give the 260-character shell limit one owner, `fits_shell`, in `src/core/winstr.h`/`.cpp`, and point `src/app/report_files.cpp` at it.
  - Delete the two known-copy entries in the scan.
  - Widen the `std::filesystem` scan rule to `recursive_directory_iterator`, `absolute`, `status`, `equivalent`, `space` and `current_path`, and wrap the two `absolute` calls in `src/cli/report.cpp` and `src/cli/fillcompare.cpp` with `os_path`.
  - Record both 248 and 260 in ADR 0020, with the third viewer-launch fallback (R7.40). D48 confirmed keeping both numbers.

## Files you share with the other sessions

- **`tests/test_single_owner.cpp`.** You edit two existing rules (the shell-path rule and the `std::filesystem` rule). Phase 3 adds new rows at the same time. Git merges appended blocks badly, so whoever merges second rebuilds the file as ours plus their additions, relative to the base.
- **`src/cli/report.cpp` and `src/cli/fillcompare.cpp`.** You change only the `absolute` lines. Phase 3 (tasks K1a and K1b) rewrites the empty-report message a few lines above. Expect a small conflict at merge.
- **ADR 0020 is yours alone.** Phase 4 doesn't touch it.

## How to run it

- **Execution.** Use one Workflow: P0-1, P0-4 and C2 as Opus implementers, each in its own worktree from main. P0-2 and P0-3 run in the same launch; they don't need worktrees, since docs and hooks live outside the build. Reviewers are Sonnet. The planning sub-step, writing each task's short brief file under `docs/superpowers/plans/tasks/`, runs on Fable.
- **Agent briefs.** Every brief carries:
  - the status line rule (`C:\Users\Patrick\.claude\hooks\state\status\<agent id>.md` every 10 calls or 5 minutes, via `status_append.ps1`);
  - "run only the tests relevant to your change, never the full suite";
  - commit with `git commit -m "<subject>" -m "Task: <id>" -m "Agent: <id>" -m "Session: <sid>" -m "Co-Authored-By: ..."`, never on stdin;
  - Read with offset and limit, never `sed`, `head` or `tail`;
  - commit before 110 calls;
  - helpers in the foreground;
  - when blocked, stop and report the exact error.
- **Agent IDs.** An agent that doesn't know its own ID learns it from the progress hook's first reply.
- **Merges.** Merge M0 (P0-1 and P0-4) first, then C2, or both together if they're ready together. Each merge goes:
  1. the precheck, once it exists;
  2. a Sonnet derive-once review (`docs/agents/derive-once-review.md`), using the key and range the hook gives;
  3. a sweep fix round by kind if there are findings;
  4. the main session runs the full suite once;
  5. merge to main.

  Main moves under you, because the other sessions merge too. Merge main into your branch before the final suite run.
- **Library check for C2.** It changes no display and no record. Corpus scores byte-identical is the proof.

## Rules (from the user, in memory)

- Anything that changes a display or a record goes to the user first. Code-only work proceeds on your recommendation and gets reported afterwards. Everything in this handoff is code-only.
- Agents never run the full suite. The main session runs it once per merge.
- Agents never stall. They report every 10 calls or 5 minutes and fail loudly.
- Planning agents run on Fable, implementers on Opus, reviewers on Sonnet.
- Stage files by name (`git add <path>`), never `git add -A`. Other sessions share this checkout.
- Nothing is pushed. Ask before pushing.

## When you're done

Update `docs/superpowers/plans/2026-10-04-phases-3-5.md` with phase 0 and phase 5's merge commits. Note any new hook rule in memory (`fleet-control-hooks-2026-09.md`). Write a short closing handoff only if something is left open.
