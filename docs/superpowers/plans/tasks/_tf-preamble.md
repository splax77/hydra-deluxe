# Test fidelity fixes: brief preamble (every tf task brief starts here)

You are one agent in the test fidelity plan, `docs/superpowers/plans/2026-10-10-test-fidelity-fixes.md`. Your task's section of that plan is your spec. The user approved every recommendation in it on 2026-10-10 (see `docs/handoffs/2026-10-10-test-fidelity-fixes-handoff.md`). Your task brief names your task id, worktree and owned files, and adds what the plan left to dispatch time. Where the brief and the plan disagree, the brief wins. Where the brief and the code disagree, stop and report it.

The main checkout is `C:\Users\Patrick\Downloads\Hydra\hydra-test`. Read the plan there by absolute path, but never edit the main checkout.

Read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md` first. Its rules apply in full, including the "If you write code" self-check. Where it and this file differ, this file wins on paths and builds.

Session id: `88f58f4f-74e2-4da8-bf4a-49622afa49e2`. Base: `main` at `013f4c66`.

## Hard rules

- **Status lines.** One line every 10 tool calls or 5 minutes: `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "HH:MM done ... | next: ..."`. The progress hook's first reply tells you your agent id.
- **Your worktree.** Make it first, unless your brief says it exists: `pwsh -NoProfile -File C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\new_worktree.ps1 -TaskId <task id>`. That makes `.claude\worktrees\<task id>` on branch `claude/<task id>` from main and builds `hydra_tests` there inside a machine-wide build slot (it waits in the foreground for a free slot; let it wait, up to 15 minutes, then stop and report). Work, build and commit only in that worktree. Later builds: `.\build_cpp.ps1 -Target hydra_tests` from the worktree.
- **Owned files only.** `git diff --stat main...HEAD` must list only the files your brief owns. If you need another file, stop and report which one and why.
- **Tests.** Run only the tests your brief names, with `-tc=` or `-sf=` filters on `build-cpp\Release\hydra_tests.exe`. Never the full suite. No whole-library run of any kind; only `testdata/input` or hand-built files.
- **Failing first.** Your brief names a red run. Run it red, keep the exact red output line for your report, then restore green.
- **New numbers.** Every number this plan adds is either named in your brief or pinned from one run of today's code. A threshold, tolerance or cap with no decision named stops the task and goes in `questions`; do not pick.
- **No visible change.** Nothing here changes a score, path, stored record or any text Hydra shows. Never touch `src/store/stored_versions.h`.
- **Other sessions' work.** Two unmerged branches touch nearby code; leave their parts alone so the merges stay small. `claude/unruffled-yalow-d290b2` ("trim the slow test and dead tests") deletes the `HYDRA_PREVIEW_DUMP` dev-aid case in `test_preview_golden.cpp`, the "upgrade timing" case in `test_store.cpp`, `delete_results_without_chart`, `leak_checked`, "every corpus chord", the one-chart loop in `test_rules` and a skipped case in `test_path_view.cpp`. `claude/agitated-blackwell-5761dc` ("cap every uncapped wait") adds `tests/wait_util.h`. Neither has merged; don't fork from them and don't copy from them.
- **Edits.** Edit tool for source. To replace most of a file, Write `<file>.new` and `Move-Item` it over. A "file modified on disk" notice after your own edit is expected.
- **Commits.** Stage by name. `git commit -m "<plain subject>" -m "<body>" --trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: 88f58f4f-74e2-4da8-bf4a-49622afa49e2" --trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"`. Commit after each finished step. Never amend, rebase, reset, move a branch or push.
- **Budget.** 100 tool calls or 20 minutes: commit what passes and return `complete: false` with an exact handoff. 150 calls or 30 minutes is a hard stop.
- **Foreground only.** Never `run_in_background`; never end your turn waiting for a job. No helper agents.
- **Blocked:** stop and report the exact command and error.
- **Plain English** in comments, commits and your report. A comment names the function that owns a rule; it never restates the rule. A pinned literal's comment says where it came from ("pinned from one run on 2026-10-10 at <commit>").

## Your final report

Return, in plain English: `complete` (true or false), branch, worktree, tip commit, what changed, each red line and green result, the exact test commands with pass counts, any numbers you pinned and where they came from, the `git diff --stat main...HEAD` file list, `questions`, and a `handoff` if incomplete.
