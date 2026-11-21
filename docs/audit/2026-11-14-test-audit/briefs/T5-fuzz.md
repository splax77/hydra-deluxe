# Brief: T5, the fuzz targets and the single-owner loader that C14 and C15 left without records

Read `docs/agents/brief-preamble.md` first, in the worktree named below. Its rules bind you. Where this brief and that page disagree, stop and report it.

## Who and where

- Task id: T5-FUZZ. Session id: 16892967-2cf8-4437-a7cd-e3dad250f621.
- Your worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records`, branch `claude/test-audit-records`. Read code there. Do every git command there.
- The one file you own and write: `docs/audit/2026-11-14-test-audit/records/C14-C15-fuzz.md`. Commit it by name, with the trailers the preamble gives. Other agents commit on this branch at the same time: stage only your file by name, and if git reports an `index.lock`, wait a few seconds and retry. You change no test, no code and no other file.
- Tests you may run: none. You build nothing and run no `hydra_*` program (agent rule 14).
- Caps and status: as the preamble says.

## Why this brief exists

The plan (`docs/superpowers/plans/2026-11-14-test-suite-audit.md`, T5) counts each `tests/fuzz/*.cpp` file as one libFuzzer target and so one record. The auditors of C14 and C15 wrote no record for six fuzz files and for `tests/single_owner/engine.cpp`, calling them "no test case". Write those records.

## The files

- `tests/fuzz/fuzz_audio.cpp`, `fuzz_chart.cpp`, `fuzz_image.cpp`, `fuzz_mid.cpp`, `fuzz_sng.cpp` and `fuzz_srb.cpp`: one record each. Each calls `hydra::fuzz::run_one` in `tests/fuzz/harness.h` on one parser. Judge, per target: what it promises (the parser never crashes, hangs or reads out of bounds on any input), what turns it red (read `run_one` and the harness whole), which parser it reaches and whether that parser is the one Hydra uses for that file kind, what seed corpus it has under `testdata/fuzz/`, and how and where it runs (the build file and any CI job). B1's record for `tests/test_fuzz_regressions.cpp` found that `run_one` returns 0 on every path that returns; say what that means for each target.
- `tests/single_owner/engine.cpp`: one record per function the single-owner scan tests (`tests/single_owner/scan.cpp`, audited as A49) call, judging whether it does what they rely on.

Ask every record the twelve questions in `docs/agents/test-rubric.md`, in the record shape that page gives, with the reading rules in `briefs/T5-C13.md` (question 3 for a claim not about a chart; threads, timing, transports; "claim not checked only" is Investigate, not Rewrite).

## Your final report

Return: the commit hash; the number of records and the count per verdict; anything you did not reach, by exact name; and anything you stopped on.
