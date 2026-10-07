# Handoff: speed exploration of scanning and analysis (2026-10-06)

Written 2026-10-06. Nothing from this session is still running, and no code changed. Main is at `bc58282` (Hydra Deluxe 2.1.0).

## Read this first

The full findings are in [the perf exploration report](2026-10-06-perf-exploration/REPORT.md). It explains how every experiment was run, the numbers, the correctness checks and the verdicts. This handoff is only the summary and what's next.

## What happened

The user asked whether chart scanning and analysis could go faster. Four web scouts researched alternatives, then six experiment agents tried the ideas in throwaway worktrees. They covered profiling, the scan, the database writer, the engine and build settings, the search algorithm, and parsing. Every timing ran one at a time behind a shared lock. Every change was checked for identical stored results across the whole library (`C:\Clone Hero`, 18,811 distinct charts).

A whole-library batch is held back by the one thread that saves results to SQLite, not by analysis. The workers spent more time waiting for it than analyzing. Inside each worker, parsing and graph building are most of the work; the path search is about 7%.

All of these kept stored results byte-identical:

- **Database writer:** statement cache, group commit and a larger WAL checkpoint took a full run from about 16 s to 11.6 s.
- **Graph build:** 13.6 s to 2.7 s on one thread.
- **Parse:** 15.0 s to 8.9 s on one thread, and the kick dynamics count 3.8 s to 0.5 s.
- **Library scan:** 4.2 s to 2.1 s; a rescan 0.8 s to 0.2 s.

These were measured one area at a time, never all together.

Dominance pruning was tested on the whole library and is unsound. It changed 58 results and lost the true best score on 3 charts. Earlier sessions had only reasoned it away, never measured it. Don't propose it again without a new idea.

## Where things are

The report folder, `docs/handoffs/2026-10-06-perf-exploration/`, is untracked and not committed. It holds:

- `REPORT.md`, the full report.
- `patches/`, one patch per experiment. Each applies cleanly to `bc58282` (checked with `git apply --check`). They are experiment code, not shippable: every idea sits behind an environment-variable switch next to the old code, and some carry temporary timers. `prof.patch` also changes CMakeLists.txt to add debug info, which must not ship.
- `notes/`, each agent's working notes, the experiment brief, the benchmark lock script `bench_run.ps1` and the log of every timed run.

The perf worktrees under `.claude\worktrees\perf-*` and the session scratchpad were deleted on the user's request. Scratchpad paths that appear in the notes no longer exist. The database comparison scripts and the copied databases went with the scratchpad, so a rerun needs new ones. The report describes what each script compared.

Two related files from other sessions are also untracked in `docs/handoffs/`:

- `2026-10-06-cpp-optimization-research-handoff.md` covers compiler settings: link-time optimization, Release symbols, mimalloc, SQLite build options and PGO. It is summarized in the report.
- A second session's follow-up is folded into the report's "Follow-up from a second research session" section. It covers the song.ini parse, word_stoi, precompiled headers, the build's missing `--parallel`, and the 44 s first-read question.

## Decisions waiting on the user

Nothing ships until the user answers these. The details are in the report's "Decisions needed" section.

1. **Group commit failure scope.** If a group's COMMIT fails, all of up to 16 charts in it fail, not just one. Progress also updates per group.
2. **WAL checkpoint at 10,000 pages.** The log file next to `hydra.db` can grow to about 40 MB during a run.
3. **Group size of 16.** It's a new number; 64 measured no better.
4. **mimalloc.** A new third-party dependency, untested.
5. **`--parallel` in `build_cpp.ps1`.** About 28% faster cold builds in one rough test. This is build speed only.

## Suggested next steps

1. **Ask the user the decisions above.**
2. **Build the writer fix first,** because it unblocks the rest. It still needs a test that cancel stops writes promptly, and a check of the GUI's Analyze library.
3. **Then, in parallel worktrees:** the graph-build changes (6b, 6c and the segment heap), the parse changes (dropping the duplicated old code paths the patch keeps), and the scan changes (adding live progress to the parallel walk). Each needs a derive-once review.
4. **Measure a first-ever scan once,** on a fresh reboot or an unopened library copy. Another session saw 44 s on a first song.ini read against 0.9 s on the second, and our warm-cache numbers don't cover that case.
5. **Then link-time optimization and Release symbols, and one combined whole-library timing** on a copy of the real database with `--redo`.

## Journal state at handoff

The hook's journal listed these lines. Both agents handed back their final reports before this file was written, and nothing is running.

```
agent-a69bbcdcb48c8dfa1: unfinished, last tool call SubagentHandback, touched 20:52
agent-aa7b97996c0f9ef1e: unfinished, last tool call SubagentHandback, touched 20:28
```
