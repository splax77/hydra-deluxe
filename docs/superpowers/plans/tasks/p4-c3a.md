# Task p4-c3a: replay and bench text (phase 4, C3a)

Plan: `docs/superpowers/plans/2026-10-04-phases-3-5.md`, task C3a. Handoff: `docs/handoffs/2026-10-04-phase4-handoff.md`. The recheck with evidence is `docs/audit/2026-10-04-session-audit/step45.md` (section C: 41, 107, 110, 326). Read them first, with offset and limit.

Dev-tool output only (hydra_replay and hydra_bench). The app's screens, scores, paths and stored records don't change. The results stamp stays "2.1.0".

## Worktree

From the main checkout: `git worktree add .claude\worktrees\p4-c3a -b claude/p4-c3a main`. Work only there. Build with `pwsh .\build_cpp.ps1` in that worktree (hydra_replay and hydra_bench are dev tools; if they are EXCLUDE_FROM_ALL, build them with `-Target hydra_replay`, `-Target hydra_bench`, `-Target hydra_tests`).

## The four fixes

- **107, the dump's not-analyzed line** (`tools/replay.cpp` ~551-556). Today it prints the typed text (`a.cap`, `a.ms`, `a.depth_mode`). Print the parsed settings it really looked up, with the app's own words for them. Find the app's existing formatter for that text (the step45 note points at `batch_settings_summary` in `src/ui/library_dialogs.cpp`); if it lives in UI code the tool can't link, look for a lower owner or move nothing and report it. Don't write a second formatter. Add a test that pins the line for a non-default input (e.g. cap 3, timing limit off).
- **110, the squeeze-out warning** (`src/core/replay.cpp` ~334-377, `replay.h` ~259-268). `ambiguous_window_warnings` takes no rules today and says the score is "high by that note's first-hit share". Make it take the rules and quote that chord's real reduction under `sqout_rule`, by calling the existing owner (`sqout_reduction` or whatever the code names it; grep). Update the one caller in `tools/replay.cpp` and the existing tests in `tests/test_replay.cpp` for the new signature. Add one case: `whole_chord` on a two-note chord quotes the whole chord's cost. Pin the expected number as a literal; don't recompute it in the test.
- **326, the bench label** (`tools/bench.cpp` ~193-207). Corpus mode keeps running exactly as now (no timing limit). The label reads "cap4 d4 no-ms", with the 4 read from `kCloneHeroSpCap` instead of a literal.
- **41 (hydra_replay half), the usage text** (`tools/replay.cpp` ~129-132). "Every stored row is a 1.1 result" is no longer true: since the app's "1.0 fills" setting (ADR 0010), stored rows carry their fill rule. The real reason: `--legacy-fills` always analyzes fresh and never reads the database, because this path predates stored 1.0 results and was kept as it was (the comment at ~501-506 says so). Make the usage text say that. Leave the guard's behaviour alone. (The hydra_batch half of 41 and the `graph.cpp`/`timing.h` comments belong to phase 3. Don't touch them.)

Run only the relevant tests: `build-cpp\Release\hydra_tests.exe -sf=*test_replay*` and any new test file you add. Never the full suite. Also run hydra_replay once on a corpus chart to show the new not-analyzed line, and hydra_bench's label path if it can run in under a minute.

## Owned files

`src/core/replay.h`, `src/core/replay.cpp`, `tools/replay.cpp`, `tools/bench.cpp`, `tests/test_replay.cpp`. Nothing else; if a fix needs another file, stop and report.

## Commits

One commit per finding. Trailers: `Task: p4-c3a`.
