# Executor brief for the concept audit's fix wave (phase 6)

Every fix-wave executor reads `docs/agents/brief-preamble.md` first, then this page, then its dispatch (task id, agent id rule). Where they disagree, stop and report it.

## Where things are

You work in your own git worktree, which the Agent tool made. Its branch forked from `origin/main` (`0997bbac`), which is older than local `main`. Nothing under `src/` or `tests/` differs between the two, so your code base is current. The audit's documents and the newest precheck exist only in the main checkout, so read them there, by absolute path:

- Your task: the row for your task id in the tables, and the section `### <your id>. ...` under "The briefs", in `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\audit\2026-10-11-concept-audit\fix-wave.md`. The row lists the files you own and the tests you may run. The section says what to change.
- The findings your task carries: `...\2026-10-11-concept-audit\findings.md`, one block per finding id (M01 to M84), with each home's file and lines, the tables and the proposed owner. Line numbers there were read on 2026-10-11 and may be off by a line or two; find the symbol.
- The decisions: D114 in `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\audit\2026-10-03-fix-decisions.md`. Any displayed or stored change you make must match its item word for word.
- The review page your reviewer will use: `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\derive-once-review.md` (main's copy has kind F and step 8).

Don't merge, rebase or move your branch to bring in main. Just commit on it.

## Building and testing

Build from your worktree with `.\build_cpp.ps1 -Target hydra_tests`; add `-Target hydra_uitest` only if your row names uitest scripts. The first build is cold (three to six minutes) and other executors build at the same time, so start it once and early, while you read. Run only the filters and uitest names in your row, as `build-cpp\Release\hydra_tests.exe <filter>` and `build-cpp\Release\hydra_uitest.exe --test <name>`. Never the whole suite, never `--all`, never anything over the song library.

A fix that changes text a test or uitest pins changes that test in the same commit. Where a finding's block proposes a failing-first test, run it red before the fix and keep the red line for your report.

## Each commit

One finding per commit, subject naming the finding id. The body says what changed in plain words, the grep you ran to find every home (and its one-line result), and either the scan row you added (in your task's own `tests/single_owner/rules/<area>/concept-<task>.json`, or the area file your section names) or one sentence on why no text search can catch the copy. Run `hydra_tests.exe -tc="single-owner*"` after adding a row.

A fix that turns out to need a new number, a new sentence or a new word that D114 doesn't give stops there. Commit what is done and report the question; don't invent it.

## Before your report

Run the precheck on your range from your worktree: `pwsh -NoProfile -File C:/Users/Patrick/Downloads/Hydra/hydra-test/tools/derive_once_precheck.ps1 -Repo .` (it compares `main...HEAD`, which is your commits only). Fix what it flags that is yours, or say in the report why a line is not a finding.

## Your final report

Your branch and worktree path, your commit hashes with the finding each carries, the exact test commands with their pass counts, any red line, the precheck's item count, and anything you stopped on. Keep it under 30 lines.
