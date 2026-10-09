Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task J4: the comparison's counts agree (D103 item 27)

Task id: RW-J4. Base: the main tip your prompt gives. Branch `claude/rw-j4-dm-counts`, worktree `.claude\worktrees\rw-j4-dm-counts`; make it as the preamble says.

## Why

The hands-on check found two counts of one thing disagreeing in the comparison window. With "apex" searched, the subtitle read "bigdaniel — 1,147 scores: 20 under optimal, 0 at optimal, 0 above optimal, 0 not analyzed, 1,096 not in your library, 31 at other speeds", while the tiles under it read "Scores 1" and "Not in library 0", and the count line read "1 of 1,147 scores". The subtitle counts every score; the tiles count the rows that pass the filters. The user decided (D103 item 27, `docs/audit/2026-10-03-fix-decisions.md`):

- The tiles keep counting the rows that pass the filters (unchanged).
- The subtitle drops its per-status breakdown. It reads "<player> — N scores", where N is every score, counted with `counted` the way the count line writes its noun (singular at 1).
- The first tile's label becomes "Scores shown", like the path report's "Charts shown".
- The count line is unchanged.

## What to change

- Find where `generate_dm_report` (or whatever builds `GeneratedDmReport::subtitle`) writes the breakdown, in `src/app/dm_report.cpp`. Drop the breakdown. If the breakdown text is built by a function nothing else calls once it's gone, delete that function too. Keep `tally_dm_rows` itself if the tiles or anything else still read it.
- The first tile's label is in `dm_tiles` (`src/app/dm_report.cpp`, or a constant beside it). Change it to "Scores shown". Check whether a shared label constant or the path report's "Charts shown" has an owner the two should share; if they are separate words for separate things, leave them separate.
- Test first: update the cases that pin the subtitle and the first tile's label (`tests/test_dm_report.cpp`, `tests/test_s2_offspeed.cpp` if it pins the subtitle, the uitest scripts in `tests/ui/uitest_report_windows.cpp` and `tests/ui/uitest_batch_reports.cpp` that read them, and `tests/report_samples.*` only if they hold the expected text). Run each red, then green.
- Any user-visible text other than the two strings above stops the task and goes in `questions`.

## Owned files

`src/app/dm_report.{h,cpp}`, `tests/test_dm_report.cpp`, `tests/test_s2_offspeed.cpp`, `tests/ui/uitest_report_windows.cpp`, `tests/ui/uitest_batch_reports.cpp`, `tests/report_samples.{h,cpp}` and `tests/test_single_owner.cpp` only if a scan row names the deleted breakdown. No docs: the main session updates them.

## Tests

`-sf=*test_dm_report*`, `-sf=*test_s2_offspeed*`, `-sf=*test_single_owner*`, and the uitest scripts you touch, by name. Never `--all`, never the full suite.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.

## Orchestrator answer (2026-10-08)

`tests/test_report.cpp` is owned too, for the one pin at line 1013 ("dm_tiles: the four sample scores"): `{"Scores", "4"}` becomes `{"Scores shown", "4"}`. Nothing else in that file.
