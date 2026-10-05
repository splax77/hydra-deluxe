# Phase 6 brief preamble (every task brief starts here)

You are one implementer in phase 6 of `docs/superpowers/plans/2026-10-04-phase-6.md`. Phase 6 is the duplicate folds: one fact worked out in two places gets one owner, and the copies call it. Where the copy is a test that recomputes production, the test pins a literal from one run instead (D42). Your task brief names your findings, owners, files, tests and pinned literals. The decisions are D53 and D54 in `docs/audit/2026-10-03-fix-decisions.md`. The fold details come from the recheck drafts `docs/audit/2026-10-04-phase6-recheck/a.md` to `d.md`, and the finding texts from `docs/audit/2026-10-03-derivation-audit.md`.

The main checkout is `C:\Users\Patrick\Downloads\Hydra\hydra-test`. Read briefs and audit files there by absolute path, but never edit it.

Also read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md`, the shared preamble. Where the two differ, this file wins on paths, builds and call limits. Its "If you write code" self-check applies to you in full.

## Hard rules

- **Status lines.** Append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or 5 minutes, whichever comes first. Use the form `HH:MM done ... | next: ...`, written with `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"`. The progress hook's first reply tells you your agent id.
- **Call limits.** You have a soft cap of 100 tool calls and a hard cap of 150. Commit what passes before call 95. If work is left at 100, stop, commit, and report exactly what remains so the main session can hand it to a fresh agent. Never go past 150.
- **Fail loudly.** When something blocks you, stop and report the exact command and error. Never wait on a prompt, never retry the same call hoping it passes, never guess a way around a hook.
- **Your worktree.** Make it first: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\p6-<task id lower case> -b claude/p6-<task id lower case> <base>`. Your brief names the base. Work, build and commit only there. Never edit the main checkout or another worktree.
- **Owned files only.** `git diff --stat <base>..HEAD` must list only the files your brief owns. If you need another file, stop and report which one and why.
- **First build.** Run it through the shared slot helper, so at most three cold builds run at once across every phase: `pwsh -NoProfile -File C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\cold_build.ps1 -Repo <your worktree> -Target hydra_tests`. It waits in the foreground for a free slot; let it wait. For later builds, run `.\build_cpp.ps1 -Target hydra_tests` (or the target your brief names) from your worktree.
- **Tests.** Run only the tests your brief names, with `-tc=` or `-sf=` filters on `build-cpp\Release\hydra_tests.exe`, or the `hydra_uitest` scripts it names. Never the full suite. Never `hydra_uitest --all`.
- **Test first where a behaviour is new.** For a new owner function, write its pinned case first and run it red (it fails to compile or fails), then make it green. Keep the red line for your report. A fold that only repoints a caller needs no red run; the existing cases are its test.
- **Pins are literals or production calls.** A test pins a literal from one run, or calls the production owner. It never works the expected value out again. Never define a helper that already exists under `tests/`; grep first.
- **No new limits.** Never add a floor, seed, depth, deadline, tolerance or band unless your brief or the plan's "Test limits" section lists it. If you need one, stop and report.
- **Nothing visible or stored changes** unless your brief names a D53 item. Scores, paths, stored records and every label stay byte-identical. Never change a stamp. If you find a display choice your brief does not name, stop and report it in game terms.
- **Scan rows.** Add your `tests/test_single_owner.cpp` rows at the end of the file only. The main session joins every task's rows at the merge.
- **Edits.** Use the Edit tool. Read with offset and limit. Never write source through a patch script. To replace most of a file, Write `<file>.new` and `Move-Item` it over; the shrink guard blocks big deletions through Edit. A "file modified on disk" notice after your own edit is expected.
- **Commits.** Stage files by name; never `git add -A` or `git add .`. Pass each trailer with `--trailer`:
  `git commit -m "<plain subject>" -m "<body>" --trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: 4f777343-4b5a-4c3a-a586-89b5f49d24d5" --trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"`.
  Never amend, rebase, reset or move a branch. Never push.
- **Helpers in the foreground.** Never use `run_in_background`, and never end your turn waiting for a job. You have no helper agents; do the work yourself.
- **Plain English** in comments, commit messages and your report.

## Your final report

Return:
- Your branch and last commit hash.
- For each finding: the owner function, its file, and one plain sentence on what changed.
- Each new test case name, with its red line (if any) and its green result, and the exact filter commands you ran with pass counts.
- The `git diff --stat` file list.
- Anything you stopped on, or work left for a fresh agent.
