# Task p4-integrate: join phase 4 on `claude/p4`

Read `docs/agents/brief-preamble.md` and `docs/agents/integrate.md` first (in the main checkout); they apply in full. This file gives what `integrate.md` says your brief names.

- **Task id:** `p4-integrate`. **Session:** `56d157ab-f000-4753-af0f-4213aa6f3f6f`.
- **Wave branch:** `claude/p4`, worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\p4`. It already holds main at 876b507 plus the merge of `claude/p4-k6g` (af7718a, the User Guide and development.md).
- **Fork point of the task branches:** `dc7e582`.
- **Join order:** first `main` (it gained 23a5d97, the .sng/.srb ranged reads, after the wave branch was made), then `claude/p4-c3a`, then `claude/p4-c1`. Hold the `claude/p4-c1` merge uncommitted until step 4 passes, as `integrate.md` says.
- **Precheck:** `tools/derive_once_precheck.ps1` has not landed on main. Skip it and say so in your report.
- **Build:** the worktree has no build folder yet. Make one cold build: `pwsh .\build_cpp.ps1 -Target hydra_tests` in the worktree. Then `-Target hydra_replay` if you need it.
- **Commit trailers:** for a merge, put the lines at the end of one `-m` message, each on its own line: `Task: p4-integrate`, `Agent: <your agent id>`, `Session: 56d157ab-f000-4753-af0f-4213aa6f3f6f`, `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`. The git hook refuses trailers spread over separate `-m` paragraphs.

## The join fix you must make: one source-tree walker

`claude/p4-c1` adds `tests/test_docs_match_code.cpp`, which reads `HYDRA_SOURCE_DIR` and walks the source tree with its own `recursive_directory_iterator` loop, copied from `tests/test_single_owner.cpp`. Two rows in `test_single_owner.cpp` say only that file may do either ("Which test reads the source tree?" and "Which test walks the source tree?"), so `-tc="single-owner rules hold*"` fails on that branch. The orchestrator's decision (code-only): don't list the docs test as a second owner, because that keeps two copies of the walker. Instead:

1. Add `tests/source_tree.h`, a small header-only owner. It is the one place that reads `HYDRA_SOURCE_DIR` (with the `#error` guard moved there) and the one place that walks `src/`, `tools/` and `tests/` (a function that calls back with each file's path and its repo-relative generic path). Keep its interface as small as both callers need; read both callers first.
2. Point `tests/test_single_owner.cpp` (its scan case and its stored_versions.h read) and `tests/test_docs_match_code.cpp` at it. Neither keeps its own walker or its own `HYDRA_SOURCE_DIR` read.
3. Change the two rows' owner text and owner file to `tests/source_tree.h`. Keep each row's must-match and must-not-match examples, so the rows still catch a third walker. Update the file's header comment if it says this file is the only reader.
4. Behaviour of both tests is unchanged: the same files are scanned, the same docs read.

Phase 5 (another session) is editing other rows of `tests/test_single_owner.cpp` (the shell-path and filesystem-call rules). Keep your edit to those two rows, the scan case's walker and the stored_versions.h read, so their later merge is easy.

## Tests to run (nothing else)

- `build-cpp\Release\hydra_tests.exe -tc="single-owner*"`
- `build-cpp\Release\hydra_tests.exe -sf=*docs_match_code*`
- `build-cpp\Release\hydra_tests.exe -sf=*test_replay*`
- `build-cpp\Release\hydra_tests.exe -sf=*preview_config*` and `-sf=*decode*`

If the docs test fails on the joined tree because main's 23a5d97 (ADR 0024 and its code) or the guide merge added a backticked name that doesn't exist, or a doc another task changed, fix the doc line in the join. Never widen the allow-list for a name that is simply wrong.

## Owned files

Only what the join needs: conflict resolution in any file the joined branches touch, `tests/source_tree.h` (new), `tests/test_single_owner.cpp` (the two rows, the walker, the one read), `tests/test_docs_match_code.cpp` (switch to the helper), and doc lines the docs test flags on the joined tree. Nothing else; stop and report if you need more.

## Report

What `integrate.md` asks for, plus: the head hash of `claude/p4`, and the exact pass counts of each test command above.
