# Summary-only storage: results (2026-10-07)

The plan `docs/superpowers/plans/2026-10-07-summary-only-storage.md` is merged on main. T5 and T6 went in together as 6bddaa8. hydra.db now keeps the chart library, one summary row per chart and settings, and meta. Every path detail comes from `analyze_chart` when a song is clicked or the report runs. The design is in ADR 0026. The user decisions are D87 and D89–D94 in `docs/audit/2026-10-03-fix-decisions.md`.

## The library check

These checks ran once on the joined build, against a baseline built from 523f719 (main just before the storage work). Both used the installed settings: Expert Pro Drums, 2x Bass, scores depth 4, 10 ms Path limit, SP cap 4. Every timed run went through the bench lock.

The batch saves the same summaries as before. `hydra_batch` with no folder arguments found the same 19,436 charts and analyzed 19,378. The same 58 failed, each because the chart has no Expert Pro Drums notes. `compare_db.py --summary-only` found 0 differences in 18,811 results rows, 19,436 charts rows and 3 meta rows.

The path report shows the same rows. Both pages hold 67,929 rows, and the two sets are equal once sorted. Rows that tie on score may come out in a different order, which is allowed (ruling 9). One field, `c`, was left out of the comparison: it is a first-seen counter that depends on row order. No chart was left out, so the page shows no "Left out:" line.

## The upgrade check

The upgrade ran on a backup-API copy of the installed `C:\Program Files\Hydra\hydra.db`.

- **Size:** the file went from 292,372,480 bytes to 12,972,032, down 95.6%. No -wal file was left beside it.
- **Tables dropped:** paths, path_refs, songmeta and dynamics are gone, and results has no structure column.
- **Rows removed:** results went from 19,106 rows to 18,816. The 290 removed rows are exactly those with a score and no stars, all from Hydra 1.6.2 to 1.7.10, so they were already Stale.
- **Rows kept:** every surviving row's summary columns are unchanged. 18,811 rows are Ready before and after, and they are the same rows, not just the same count.
- **Report:** the upgraded file's path report equals the fresh batch's.

## Timings

| What | Before | After |
|---|---|---|
| Whole-library batch, wall time | 20.76 s | 6.75 s |
| The batch's own "Analyzed in" time | 8.1 s | 2.6 s |
| Path report (it now analyzes the library) | read stored rows | 2.52 s |
| One-time upgrade at open | — | about 0.3 s (difference of two single runs) |
| Database built by the batch | 284,995,584 bytes | 13,160,448 bytes |

The batch got faster because it no longer encodes and writes path details. The report is slower, because it now analyzes every chart instead of reading stored rows. It still finishes in about 2.5 s on 8 workers, close to the 2.4 s T0 estimated.

## What changed for the user

- Clicking a song analyzes it, and saves its summary when that is missing or different (D87, D90).
- The Analyze button is gone.
- The Stale and not-analyzed tooltips say to click the song or run a batch (D87, D91).
- The Preview's changed-chart line says to click the song again (D94).
- The DM page and hydra_fillcompare cover only charts in the library (D92).
- The database's side file is capped at 4 MB (D93).

## Open items

- **Track R** ("Make hydra_replay use the engine, not a copy") is a proposed session that hasn't started.
- **Hook bug:** the agent call-budget hook can lose its count when two tool calls run at once. One agent ran to about 237 calls unseen. A fix is proposed as its own session.
- **Two throwaway worktrees** are left for the user to remove: `.claude\worktrees\storage-baseline` and `.claude\worktrees\t5c-base`. A hook stops agents from removing worktrees they don't own.
- **Deferred minors from the reviews:**
  - The GUI report job shows "no records stored yet" on a database with no library.
  - The naming-copy join relies on SQLite's automatic column name for MIN(rowid).
  - The no-box half of the view-progress-delay UI test skips on a busy machine.
  - A file whose modified time changed but whose content didn't is re-hashed on every click until the next rescan.
  - The panel-split UI test failed once under `--jobs 4`.
  - report.cpp asks "does the store hold any results" twice in one flow.
  - The "chart is in the library" lookup is spelled with two maps, in collect_rows and records_by_hash.
