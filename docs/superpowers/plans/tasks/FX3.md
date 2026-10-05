Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task FX3: M_D round 2 leftovers

Task id: FX3. Base: c0d231b (claude/p3-d2 with fix round 2 joined). Branch `claude/p3-fx3`. Worktree `.claude\worktrees\p3-fx3`.

## Goal

Three small items left after fix round 2. Context: `FX2-R.md` item 3 and its stop report, and round 2's review `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\review-M_D-r2.md` ("Part: reports" finding 3). User decisions: D57 on main.

## What is left

1. **Leaderboard percent (reports 3).** `collect_dm_rows` in `src/app/dm_report.cpp` (around line 231) still divides score by optimal into a double `pct`, beside the whole-hundredths value from `percent_steps`. Replace the double with one optional whole-hundredths field on `DmReportRow` (filled by `percent_steps`; empty exactly where `pct` is empty today, so it still marks "base speed with an optimal score"). The page sorts on it and the cells and tile read it; drop the `pct` key from the payload. Update every reader: `tests/test_dm_report.cpp`, `tests/test_s2_offspeed.cpp` (around lines 60-62), and `tests/test_report.cpp` (the page fixture around 716-717 and the column-key lookup around 1002). Pin one ordering case: two rows whose percents differ only past the second decimal sort as equals and keep a stable order; two that differ at the shown precision sort by it. No shown number changes.
2. **Guide examples (D57 item 1).** `docs/UserGuide.md` lines around 118 and 122 show "0.0ms"; the app now prints "0.0 ms". Keep phase 4's docs test green.
3. **`SPSqueeze::description()` (`src/core/model.cpp`).** It prints "%.1fms" by hand. Grep `src/` and `tools/` for callers. If nothing outside tests calls it, delete it and the test lines in `tests/test_squeeze_rating.cpp` that pin it. If something does, stop and report the caller.

## Owned files

`src/app/dm_report.cpp`, `src/app/dm_report.h`, `src/core/model.cpp`, `src/core/model.h`, `docs/UserGuide.md` (the two examples only), `tests/test_dm_report.cpp`, `tests/test_s2_offspeed.cpp`, `tests/test_report.cpp` (the leaderboard fixture and key lookup only), `tests/test_squeeze_rating.cpp`, `tests/test_single_owner.cpp` (only if a row names `pct`).

## Tests you may run

`-sf=*test_dm_report*`, `-sf=*test_s2_offspeed*`, `-sf=*test_report.cpp`, `-sf=*test_squeeze_rating*`, `-sf=*test_model*`, `-tc="single-owner*"`, `-sf=*docs_match_code*`; build `hydra_uitest` and run `--test dm-compare-flow`.

## Done when

The leaderboard has one percent value, from `percent_steps`, and an ordering test. The guide shows "0.0 ms". `SPSqueeze::description` is gone or reported. No score, path, record or shown number changes.
