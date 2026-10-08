# Report windows brief preamble (every rw task brief starts here)

You are one agent in the report windows plan, `docs/superpowers/plans/2026-10-08-path-report-window.md`. Your task's section of that plan is your spec, together with the design spec it names, `docs/superpowers/specs/2026-10-08-path-report-window-design.md`. The user's decisions are D103 in `docs/audit/2026-10-03-fix-decisions.md`. The mock is `docs/superpowers/specs/2026-10-08-path-report-window-mockup/report-window.html`. Your task brief (the file that sent you here) names your task id, base, branch, worktree and owned files, and adds whatever the plan left to dispatch time. Where the brief and the plan disagree, the brief wins; where the brief and the spec disagree, stop and report it.

The main checkout is `C:\Users\Patrick\Downloads\Hydra\hydra-test`. Read the plan, spec and mock there by absolute path, but never edit the main checkout.

Also read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md`, the shared preamble. Where the two differ, this file wins on paths and builds. Its "If you write code" self-check applies to you in full.

Session id: `c4589620-c1de-4119-bfdd-c82e8fbe6a86`.

## Hard rules

- **Status lines.** Append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or 5 minutes. Use the form `HH:MM done ... | next: ...`, written with `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"`. The progress hook's first reply tells you your agent id.
- **Your worktree.** If your brief says to make it, make it first: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\<name> -b claude/<name> <base>`, with the name your brief gives. If it already exists (a finisher or fix agent), work in it as it is. Work, build and commit only there. Never edit the main checkout or another worktree. Never delete, move or rewrite anything outside your owned files and your scratchpad.
- **Owned files only.** `git diff --stat <base>..HEAD` must list only the files your brief owns. If you need another file, stop and report which one and why.
- **First build.** Run it through the shared slot helper, so at most three cold builds run at once: `pwsh -NoProfile -File <your worktree>\tools\build_slot.ps1 -Repo <your worktree> -Target <target>`. It waits in the foreground for a free slot; let it wait. Later builds: `.\build_cpp.ps1 -Target <target>` from your worktree.
- **Tests.** Run only the tests your brief names. Use `-tc=` or `-sf=` filters on `build-cpp\Release\hydra_tests.exe`, and only the named `hydra_uitest` scripts. Never the full suite, never `hydra_uitest --all`. No whole-library run of any kind.
- **Test first.** For each new behaviour, write the failing test and run it red, then green. Keep the red run's output line for your report.
- **Rows don't change.** No task in wave 1 changes a stored record, a score, a path or any text Hydra shows today. Never touch `src/store/stored_versions.h`. The HTML pages keep building exactly as today until task T7 deletes them.
- **Pages still carry their own copies until T7.** In wave 1 the C++ you add takes over rules the pages' JavaScript also computes (tiles, search, keep-rules, sort). The JavaScript copies stay until T7 deletes the pages; don't edit or delete them. Say in your report which JavaScript copy each new function replaces, so the T7 agent can delete it.
- **Edits.** Edit source with the Edit tool. Read with offset and limit, never `sed`, `head` or `tail`. Never write source through a patch script. To replace most of a file, Write `<file>.new` and `Move-Item` it over the old one. A "file modified on disk" notice after your own edit is expected.
- **Commits.** Stage files by name. `git commit -m "<plain subject>" -m "<body>" --trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: c4589620-c1de-4119-bfdd-c82e8fbe6a86" --trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"` (use your own model's name in the last trailer). Commit after each finished step and before your 100th tool call. Never amend, rebase, reset or move a branch. Never push.
- **Budget.** At 100 tool calls, commit what passes and return with `complete: false` and a handoff that says exactly what is left; a finisher agent picks it up in your worktree. 150 is a hard stop.
- **Foreground only.** Never use `run_in_background`, and never end your turn waiting for a job.
- **No helper agents.** Workflow agents have no Agent tool. Do the mechanical work yourself and say so in your report.
- **When blocked,** stop and report the exact command and error. Never wait on a prompt, never guess a way around a hook.
- **New words.** The spec's "New words on screen" list is complete. Any other new user-visible text stops the task and goes in `questions`.
- **New numbers.** A new threshold, size, colour or limit with no decision in D103 or the spec stops and goes in `questions`; do not pick.
- **Plain English** in comments, docs and your report. Comments name the function that owns a rule; they never restate the rule.

## Your final report

You return structured output. `report` is plain English: what changed, each test case with its red line and green result, the exact test commands and pass counts, which page JavaScript each new function replaces, and the `git diff --stat <base>..HEAD` file list.
