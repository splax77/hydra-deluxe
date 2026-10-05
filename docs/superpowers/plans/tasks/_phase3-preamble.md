# Phase 3 brief preamble (every task brief starts here)

You are one implementer in phase 3 of `docs/superpowers/plans/2026-10-04-phases-3-5.md`. Phase 3 gives each displayed fact one owner function, then points every screen at it. Your task brief (the file that sent you here) names your findings, the files you own, the tests you run and the numbers your tests pin. The main checkout is `C:\Users\Patrick\Downloads\Hydra\hydra-test`; read briefs and audit drafts there by absolute path, but never edit it.

Also read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md` (the shared preamble on main). Where the two differ, this file wins on paths and builds; its "Before you commit" self-check applies to you in full.

## Hard rules

- **Status lines.** Append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or 5 minutes, form `HH:MM done ... | next: ...`, with `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"`. You learn your agent id from the progress hook's first reply.
- **Your worktree.** Make it first: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\p3-<task id lower case> -b claude/p3-<task id lower case> <base>`. The brief names the base. Work, build and commit only there. Never edit the main checkout or another worktree. Never delete, move or rewrite anything outside your owned files and your scratchpad.
- **Owned files only.** `git diff --stat <base>..HEAD` must list only the files your brief owns. If you need another file, stop and report which and why.
- **First build** goes through the slot helper, so at most three cold builds run at once: `pwsh -NoProfile -File C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\build_slot.ps1 -Repo <your worktree> -Target hydra_tests` (the main checkout's copy, the one owner of the three-slot rule; your worktree may predate it). It waits in the foreground for a slot; let it. Later builds: `.\build_cpp.ps1 -Target hydra_tests` (or the target you need) from your worktree.
- **Tests.** Run only the tests your brief names, with `-tc=` or `-sf=` filters on `build-cpp\Release\hydra_tests.exe`. Never the full suite. Never `hydra_uitest --all`; run only the uitest scripts your brief names.
- **Test first.** For each behaviour, write the failing test, run it red, then make it green. Keep the red run's output line for your report.
- **Shared test helpers.** Use the fixtures in `tests/record_fixtures.h` and `tests/display_fixtures.h` (task H1) and the existing helpers. Never define a helper that already exists elsewhere under `tests/`; grep for it first. A test that recomputes what production computes, instead of calling it or pinning a literal, is a copy and a review finding.
- **No new limits.** Never add a floor, seed, depth, deadline, tolerance or band to a test or to code unless your brief lists it. If you need one, stop and report.
- **No display change beyond the brief.** Every visible text change must be one your brief names (D48 settles them). If you find another display choice, stop and report it in game terms; do not pick.
- **Scores never move.** Nothing in phase 3 changes a score, a path or a stored record. The results stamp stays "2.1.0".
- **Edits.** Use the Edit tool for source. Read with offset and limit, never `sed`, `head` or `tail`. No patch scripts writing source. To replace most of a file, Write `<file>.new` and `Move-Item` it over (the shrink guard blocks big Edit deletions). The "file modified on disk" notice after your own edits is expected, not an outsider.
- **Commits.** Stage files by name, never `git add -A` or `git add .`. Never stdin. The trailers must be one block at the end of the message, so pass them with `--trailer` (separate `-m` trailers become separate paragraphs and the git gate refuses them):
  `git commit -m "<plain subject>" -m "<body>" --trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: 06ef47ec-7105-4611-b41f-4db00557e4a1" --trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"`.
  Commit before 110 tool calls, and after each finished step. Never amend, rebase or move a branch.
- **Helpers in the foreground.** Never use `run_in_background`, never end your turn waiting for a job.
- **When blocked**, stop and report the exact command and error. Never wait, never guess around it.
- **Plain English** in comments, user-visible text and your report. Say what a thing does, not how clever it is.

## Your final report

Return: the branch and last commit hash; for each finding the owner function, its file, and one plain sentence of what changed; each test case name with its red line and its green result; the `git diff --stat` file list; and anything you stopped on.
