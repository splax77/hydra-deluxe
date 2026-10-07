Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules.

# Task H1: committed comparison tools and the bench harness modes

Task id: H1. Base: main at 7403685 (bc58282 plus two docs-only commits, so the code is bc58282's). Branch: `claude/perf-h1`, worktree `.claude\worktrees\perf-h1`; make it as the preamble says.

Your spec is the plan's "Task H1" section, word for word: goal, files, acceptance criteria, verify line and steps. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

The pinned digests in `tests/test_perf_digest.cpp` come from one run of your own `--engine` and `--parse` modes on `testdata/input`, before you write the test. Your harness is new, so "a run on main" means a run of your harness build on the unchanged engine code, which is what your worktree holds.

`tools/bench_run.ps1` keeps its lock file and `bench_log.md` in `C:\Users\Patrick\.claude\hooks\state\bench\`. Create that folder if it is missing. Five wave-1 agents and a baseline builder will call this script from their own worktrees at once, so test the "second caller waits or exits 3" rule for real, with two PowerShell calls.

`hydra_bench --parse` must take the analysis settings from the same place `hydra_batch` and `--engine` do (the ini in the folder it runs from, through `app::Settings`), so a later task can run it at Hard, Pro Drums off, 2x Bass off by pointing it at a second ini. If the patch's `parse_mode` reads settings some other way, say so in the report and make it read the ini.

Wave 1 forks from your branch tip the moment you return, before your review. So return only once every acceptance criterion passes and everything is committed.

## Owned files

Exactly the plan's list: `tools/compare_db.py`, `tools/test_compare_db.py`, `tools/bench_run.ps1`, `tests/song_digest.h`, `tests/test_perf_digest.cpp`, `tools/bench.cpp`, `CMakeLists.txt` (one line), `docs/development.md`.

Owned-file check: every deliverable in the plan's H1 acceptance list lives in one of the eight files above; the bench lock and log are outside the repo by design.

## Preflight

Command: `git apply --check docs/handoffs/2026-10-06-perf-exploration/patches/engine.patch` and the same for `parse.patch`, run in the main checkout at 7403685 by the orchestrator on 2026-10-06.
Output: both passed with no message (every one of the seven patches applies cleanly; `--ignore-whitespace` is not needed). `tools/bench.cpp` on main has `bench_main` at line 245 and a `--scan` dispatch at line 275; `docs/handoffs/2026-10-06-perf-exploration/notes/bench_run.ps1` exists.

## Return

`complete`, `branch`, `worktree`, `tip` (full hash), `engine_hash` and `parse_hash` (what the modes print on `testdata/input`), `report`, `questions`, `handoff`.
