Read docs/superpowers/plans/tasks/_perf-preamble.md first; it holds the rules.

# Task P1: lean chart and MIDI readers, with the old readers deleted

Task id: P1. Base: H1's tip, which your prompt gives. Branch: `claude/perf-p1`, worktree `.claude\worktrees\perf-p1`; make it as the preamble says.

Your spec is the plan's "Task P1" section, word for word. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

The expected files for the 41 crafted edge files come from the baseline's exe, not yours: `<baseline folder>\hydra_bench.exe --parse <your worktree>\testdata\parse_edge\list.txt --out <...>\expected_expert.tsv`, and the same from `<baseline folder>\hard\` for `expected_hard.tsv`. The baseline folder's `README.md` names the commit it was built from; quote that hash and the two commands in your report. If the baseline is not `READY` when you get there, write the generator first, as the plan orders anyway.

Paths in the expected files must not depend on where the worktree lives. If `--parse` prints absolute paths, store them relative to `testdata/parse_edge/`, and say how the test maps them back.

Whole-library correctness compares against the baseline's saved `parse_expert.tsv`, `parse_hard.tsv`, `engine_rows.txt` and `fresh.db`; you only run your own side. The parse-time criterion needs the same-session baseline, so run baseline and yours as one interleaved `--parse` pair through the lock and quote both times.

P1 is the heaviest task. Plan for the budget: commit after each step, and at 100 tool calls return `complete: false` with a handoff naming the next step and what already passes.

## Owned files

The plan's P1 list: `src/parse/song.cpp`, `src/parse/midi.h`, `src/parse/midi.cpp`, everything under `testdata/parse_edge/` (the generator, `list.txt`, the two expected files and the generated files), `tests/test_song.cpp`, `tests/test_midi.cpp`, `tests/test_single_owner.cpp` (your own rows at the end only). `tests/song_digest.h` is read, not changed.

Owned-file check: every P1 acceptance criterion is met inside the files above.

## Added by the main session (2026-10-06, after the review)

The review's kind B/C finding (the per-chart parse digest recipe copied into `tests/test_song.cpp`) puts the shared helper next to `tests/song_digest.h`. The fix round may change `tests/song_digest.h` and `tests/test_perf_digest.cpp` for that finding only. No other wave-1 task touches either file. The pinned digests in `test_perf_digest.cpp` must not move.

## Preflight

Command: `grep -n "load_sections\|word_stoi\|try_parse_int" src/parse/song.cpp` and `grep -n "parse_track" src/parse/midi.cpp`, run by the orchestrator on the base's code on 2026-10-06.
Output: `try_parse_int` at song.cpp 99, `word_stoi` at 1066, `ChartParser::load_sections` declared at 1195 and defined at 1259; `MidiFile::parse_track` defined at midi.cpp 220. `git apply --check` of `patches/parse.patch` passes cleanly on this code.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
