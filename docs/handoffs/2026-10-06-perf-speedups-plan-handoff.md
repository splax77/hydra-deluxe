# Handoff: ship the speedups (2026-10-06)

Written 2026-10-06. Nothing is running and no code has changed. Main holds the plan and its sources in two docs-only commits on top of `bc58282` (Hydra Deluxe 2.1.0).

## Read this first

The plan is [2026-10-06-perf-speedups.md](../superpowers/plans/2026-10-06-perf-speedups.md), with its task list in the `.tasks.json` beside it. It argues from the [perf exploration report](2026-10-06-perf-exploration/REPORT.md), whose patches and notes are now committed, so worktrees can read them. This handoff only says where things stand and what to do first.

## What happened this session

The previous handoff (`2026-10-06-perf-exploration-handoff.md`) left five decisions for the user. Before asking, this session checked each one against the code and the live install. Each question explained where its number came from, what it costs, what it buys and what the alternatives are. The user took every recommendation. They are recorded as **D86** in `docs/audit/2026-10-03-fix-decisions.md`:

1. A group COMMIT that fails is rolled back, and each of its charts is saved again alone. So only a chart whose own save fails is marked failed, and D71 item 6 holds exactly.
2. A group holds at most 16 charts.
3. The WAL checkpoint is 10,000 pages only while a batch runs. At batch end one `wal_checkpoint(TRUNCATE)` forces everything to disk and empties the log, then the setting goes back to 1,000.
4. mimalloc gets a measured trial after everything else lands. Shipping it is a separate call, and the agent must name the download and get the user's yes first.
5. `build_cpp.ps1` builds with `--parallel`, uncapped.
6. The user times a first-ever scan after their next reboot, with the command in the plan.
7. hydra_batch's per-chart lines may arrive in bursts of up to 16. This one came up during planning.

One fact found along the way changes how D86.3 reads. The installed Hydra's `hydra.db-wal` was already 45.6 MB (11,074 pages) while it ran. SQLite reuses the log without shrinking it, so the report's "about 4 MB today" was never true in practice.

A Fable planning agent then wrote the plan. It made one judgement worth knowing. The dynamics-count fix moved from the parse task to the graph task (G1). The patch gave that count its own copy of the chord's note order, and `Chord` in `src/core/model.cpp` already owns that order. G1 now gives that order one owner that both fixes read.

## The plan in one paragraph

Wave 0 is one task, H1. It commits the proof tools the experiments lost with their scratchpad: `tools/compare_db.py`, the `--engine` and `--parse` modes in `hydra_bench`, the benchmark lock script, and a test that pins the 97-chart corpus's digests. Wave 1 is five worktrees forked from H1's tip as soon as its code exists, without waiting for H1's review. They are W1 (the writer), G1 (the graph build plus the dynamics count and the segment heap), P1 (the lean parsers, with 41 crafted error files as fixtures), S1 (the scan) and B1 (`--parallel`, link-time optimization, Release symbols, two SQLite options). They join through `docs/agents/integrate.md`, then get one derive-once review, one full suite run, and a merge. Wave 2 is T1, the combined A/B timing against bc58282 on a copy of the real database, alongside the mimalloc trial. Every code task must show 0 differing rows over the whole library, and no stamp in `stored_versions.h` moves.

## What the next session does first

1. **Choose how to run it.** The user asked for this handoff instead of choosing between running it in this session (subagent-driven) and a separate session. The plan fits `superpowers-extended-cc:subagent-driven-development` in the next session, or a workflow if the user asks for one. CLAUDE.md rule 6 means wave 1's five tasks launch together, never one after another.
2. **Write the per-task briefs** as `docs/superpowers/plans/tasks/perf-<id>.md`, each starting from `docs/agents/brief-preamble.md`, the way phases 6 and 7 did. The plan names the patch hunks each task lifts.
3. **Build the baseline exe set once** before wave 1 starts, from the commit the wave forks from, into `C:\Users\Patrick\.claude\hooks\state\bench\baseline-<short hash>\`. That way five agents don't each cold-build one. The plan's recipe section says how.
4. **Build `hydra_bench` from main before the user's next reboot,** so their first-scan command runs current code. The command and how to read its result are in the plan's "The user's step" section.

## Still open

One question can only be answered once W1 measures it. If the batch-end checkpoint takes more than about a second, the progress strip would sit still while it runs. W1 stops and reports the number, and the user then picks: accept the pause, show a "Saving..." state (new text, which needs the user's words), or run the checkpoint after the strip says finished.

The patches were committed with a CRLF line-ending warning. The plan has agents lift hunks by reading them, not by running `git apply`. Nobody has checked whether `git apply --check` still passes on a fresh checkout; if it fails on line endings, try `--ignore-whitespace`.

Two other untracked files in `docs/handoffs/` came from other sessions and were left alone: `release-2.1.0-notes.md` and `2026-09-29-public-timing-scripts/blink/results/blink-both.json`.
