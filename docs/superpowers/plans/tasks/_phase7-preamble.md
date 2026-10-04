# Phase 7 brief preamble (every task brief starts here)

You are one implementer in phase 7 of `docs/superpowers/plans/2026-10-04-phase-7.md`. Phase 7 gives each rule one owner, then points the code that asked the question itself at that owner. Your task brief (the file that sent you here) names your findings, the files you own, the tests you run and the numbers your tests pin. The decisions are D51 in `docs/audit/2026-10-03-fix-decisions.md`, which answers `docs/audit/2026-10-04-phase-7-questions.md`. The main checkout is `C:\Users\Patrick\Downloads\Hydra\hydra-test`. Read briefs and audit files there by absolute path, but never edit it.

Also read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md`, the shared preamble on main. Where the two differ, this file wins on paths and builds. Its "If you write code" self-check applies to you in full.

## Hard rules

- **Status lines.** Append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or 5 minutes. Use the form `HH:MM done ... | next: ...`, written with `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"`. The progress hook's first reply tells you your agent id.
- **Your worktree.** Make it first: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\p7-<task id lower case> -b claude/p7-<task id lower case> <base>`. Your brief names the base. Work, build and commit only there. Never edit the main checkout or another worktree. Never delete, move or rewrite anything outside your owned files and your scratchpad.
- **Owned files only.** `git diff --stat <base>..HEAD` must list only the files your brief owns. If you need another file, stop and report which one and why.
- **First build.** Run it through the shared slot helper, so at most three cold builds run at once across every phase: `pwsh -NoProfile -File C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\cold_build.ps1 -Repo <your worktree> -Target hydra_tests`. It waits in the foreground for a free slot; let it wait. For later builds, run `.\build_cpp.ps1 -Target hydra_tests` (or the target you need) from your worktree. Python-only tasks need no build.
- **Tests.** Run only the tests your brief names. For C++, use `-tc=` or `-sf=` filters on `build-cpp\Release\hydra_tests.exe`. For Python, run `python -m pytest <the named files> -q`. Never run the full suite. Never run `hydra_uitest --all`.
- **Test first.** For each behaviour, write the failing test and run it red, then make it green. Keep the red run's output line for your report.
- **Shared test helpers.** Use the fixtures in `tests/record_fixtures.h`, `tests/display_fixtures.h` and the existing helpers. Never define a helper that already exists elsewhere under `tests/`; grep for it first. A test that works out the expected value again, instead of calling production or pinning a literal, is a copy and a review finding.
- **No new limits.** Never add a floor, seed, depth, deadline, tolerance or band to a test or to code unless your brief or the plan's "Test limits" section lists it. If you need one, stop and report.
- **Only the display changes D51 names.** Every visible text change must be one your brief names. If you find another display choice, stop and report it in game terms; don't pick one yourself.
- **Scores and stored results.** Only E1 and E2 change stored results in wave 1. Every other task leaves scores, paths and stored records byte-identical. The results stamp stays "2.1.0", because no release carries it yet; never change a stamp yourself.
- **Edits.** Edit source with the Edit tool. Read with offset and limit, never `sed`, `head` or `tail`. Never write source through a patch script. To replace most of a file, Write `<file>.new` and `Move-Item` it over the old one; the shrink guard blocks big deletions through Edit. A "file modified on disk" notice after your own edit is expected, not an outsider.
- **Commits.** Stage files by name; never `git add -A` or `git add .`, and never pass the message through stdin. The trailers must be one block at the end of the message, so pass each with `--trailer`:
  `git commit -m "<plain subject>" -m "<body>" --trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: b9839bd7-dd4d-42dc-b313-c0a3c6723f61" --trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"`.
  Commit before your 110th tool call, and after each finished step. Never amend, rebase or move a branch. Never push.
- **Helpers in the foreground.** Never use `run_in_background`, and never end your turn waiting for a job.
- **When blocked,** stop and report the exact command and error. Never wait, and never guess a way around it.
- **Plain English** in comments, user-visible text and your report. Say what a thing does.

## Your final report

Return:
- Your branch and last commit hash.
- For each finding: the owner function, its file, and one plain sentence on what changed.
- Each test case name, with its red line and its green result.
- The `git diff --stat` file list.
- Anything you stopped on.
