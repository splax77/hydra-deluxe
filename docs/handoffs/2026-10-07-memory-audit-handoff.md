# Handoff: the memory audit's decisions and fixes (2026-10-07)

Written 2026-10-07. Nothing is running and nothing from the audit has been changed in code. The findings are in [2026-10-07-memory-audit.md](2026-10-07-memory-audit.md); read that first. This handoff says what the next session has to do with it. The user will make the four decisions below in that session.

## Where things stand

Main has mimalloc in every exe (D88, fa2071e) and the summary-only storage work (D87, D89 to D91) from another session. The audit measured today's main (2337539, with D87) on a fresh backup copy of the installed database. Its key number: a whole-library `hydra_batch --redo` commits 874 MB with mimalloc and 479 MB with the Windows segment heap, while the data Hydra actually holds peaks at 190 to 355 MiB. So most of mimalloc's extra memory is freed memory it keeps for reuse. Hydra has no leak that grows in normal use. It has one leak when the Preview renderer fails to start.

## First: the four decisions for the user

Ask these before any work that depends on them. Each one needs the user's own answer, because it changes something the user sees, adds a new number, or reverses an earlier decision. Explain each in game and app terms, with the numbers from the report.

The first is what to do about mimalloc's kept memory, worth about 400 MB of committed memory at a batch's peak. There are four options. One `mi_collect(true)` call when a batch finishes is the cheapest, but needs one timing run to price it. A per-chart mimalloc heap (`mi_heap_new` per chart, `mi_heap_destroy` at the end) frees a chart's memory in one step. A purge delay between 0 and the default 1 second trades speed for memory; 0 measured 333 MB but slowed the batch from 9.6 to 13.1 s. The last option is going back to the segment heap, which reverses D88. The user shipped mimalloc for its 10% CPU saving, so present this as tuning first, reversal last.

The second is whether to stop giant charts running side by side. The batch hands charts out in folder order, and the three Endless Setlist files sit next to each other, so they start on three workers at once. A cap such as "only one chart above some size parses at a time" needs a size threshold, which is a new number the user must pick.

The third is whether to drop the Yu Gothic font merge (14 MB). The bundled Shippori Antique B1 is a Japanese typeface and may already cover what Yu Gothic adds. Dropping the merge could change how some titles draw. Check which library titles would lose or change glyphs before asking, and show the user examples.

The fourth is whether to free the Preview's GPU render targets when the Preview closes (about 40 MB est.). It makes reopening the Preview a few ms slower. This is a visible behaviour change, so it is the user's call.

## Then: the code-only fixes

These change no stored result and nothing the user sees, so they go ahead on the session's recommendation and get reported after (agent rule 3). They are independent, so run them as parallel tasks in worktrees with one review each, then one full suite and one correctness check at the join, per the user's rules in `one-library-check-at-the-join` memory and `docs/superpowers/plans/tasks/_perf-preamble.md`.

1. Free each chart's song and analysis result on its worker once its database row is built. Encode the tempo map on the worker and pass `save_analysis` the bytes, not the Song. The perf exploration already prototyped this as `HYDRA_LEAN` (see `docs/handoffs/2026-10-06-perf-exploration/notes/write-NOTES.md`) and measured no time cost. Keep the failed-group retry path working; it currently reads `wr.analysis->song`.
2. Give the big per-chord arrays their size up front: `Song::sequence`, the .chart drum lines, the .mid lean messages and the score graph's `all_backends_` (its final length is exactly `sequence.size()`). Then trim the Song's spare room after parsing.
3. Hold the Preview renderer's `Impl` in a `std::unique_ptr` (`src/render/preview_renderer.cpp`), which fixes the one real leak.
4. Free the batch's report seed as soon as the report is written, not when the strip is dismissed. Also free `hydra_batch`'s scan leftovers (the cache map and copied scan items) before analysis starts.
5. Read the font files through a memory map with `FontDataOwnedByAtlas=false` instead of letting ImGui copy them to the heap. Check the map's lifetime covers the atlas's.
6. Trim the library rows of fields only the clicked song needs: the raw title/artist/charter copy, `sig`, the unused path-summary fields, the pre-built best label, and the md5 as text. Load what the click needs from the store instead.
7. Fix the UI test runner's frame backlog (`tests/ui/uitest_harness.cpp`): flush and wait on the GPU every few frames, so a long `wait` can't pile up gigabytes.
8. Add a leak test: a Debug doctest helper that takes `_CrtMemCheckpoint` before and after a test body and asserts `_CrtMemDifference` finds nothing, run with mimalloc's redirect off (`MIMALLOC_DISABLE_REDIRECT=1`), since the CRT debug heap can't see mimalloc's blocks.

Before writing any brief, re-read the line numbers. The audit's agents read the shared checkout while another session had uncommitted edits, so their line numbers are approximate, and D87's work has since landed.

## How to measure

Measure memory the way the audit did. Run `hydra_batch --redo` on a fresh backup-API copy of the installed database (`C:\Program Files\Hydra\hydra.db`, opened read-only), through `tools/bench_run.ps1`, one run at a time, with the song files warm on both sides. Copy `mimalloc.dll` and `mimalloc-redirect.dll` with any exe you move out of a build folder. `MIMALLOC_SHOW_STATS=1` prints mimalloc's live peak ("total" row), its committed peak ("arenas committed") and the process's peak RSS and commit at exit. `MIMALLOC_DISABLE_REDIRECT=1` runs the same exe on the segment heap for a fair A/B. Compare private (committed) memory, not only the working set.

## Files

The report is `docs/handoffs/2026-10-07-memory-audit.md`. The earlier speed work it builds on is in `docs/handoffs/2026-10-07-perf-speedups-results.md` and `docs/handoffs/2026-10-07-mimalloc-trial.md`. The agents' scratch scripts (synthetic long charts, the UI-test memory sampler, the split-run script) were in this session's scratchpad and are not kept; the report records everything they measured.
