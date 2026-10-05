Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task FX2-L: M_D review round 2 fixes, ms text, Library, Dynamics, tests

Task id: FX2-L. Base: fc55f8b. Branch `claude/p3-fx2-l`. Worktree `.claude\worktrees\p3-fx2-l`.

## Goal

Round 2 of the M_D review: `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\review-M_D-r2.md`, section "Part: library" (findings 1, 2, 4, 5, 6, 7), plus the `display_format.h` comment from "Part: reports" finding 4 and the dynamics count from reports finding 5. Read them first. User decisions: D57 in `docs/audit/2026-10-03-fix-decisions.md` on main (read it in the main checkout).

## What is left

1. **One-decimal ms with a space (library 1, D57 item 1).** Every one-decimal ms figure reads "163.5 ms". Make one formatter answer it: fold `format_ms` and `format_ms_spaced` in `src/app/display_format.*` into one that prints the space, and point every caller at it (Paths tab rows, tooltips, "(eff. …)", the report's timing columns, the early-fill line). Re-pin the tests that pinned "163.5ms". Fix the `display_format.h` comment so it names what each remaining formatter is for.
2. **Backend tooltip budget (library 2, D57 item 2).** The tooltip in `src/app/path_view.cpp` (around 330-336) names the normal budget at one decimal in both places: "on the normal 170.5 ms scale … not 170.5 ms" at a hit window of 85.25. Re-pin that test.
3. **Stale-record recipes (library 4).** The one-row `hyversion = "0.0.0"` recipe typed nine times in `tests/test_store.cpp`, the other-rules recipe (around 631-641) and `add_stale_rows` in `tests/display_fixtures.h` all build the same records; give each recipe one fixture and call it everywhere.
4. **`searching()` inside its class (library 5).** `src/ui/library_model.cpp` (around 174) writes `query_.empty()` where `searching()` answers it.
5. **Scan rows (library 6).** In `tests/test_single_owner.cpp`, make the Dynamics row scan catch the `r == DynamicsRow::X` shape, the whole-ms row catch `std::llround`, and the tick-rounding row catch a rounding split over two lines, with the review's must-match and must-not-match lines.
6. **Dynamics note in a test (library 7).** The test "every row's own note counts in that row" rebuilds the note with the same three lines as `dynamics_row_label`. Add the owner the review proposes (`dynamics_row_note` or similar) in `src/app/dynamics_breakdown.*`; the label and the test call it.
7. **"1 of 1 kick notes" (reports 5).** `src/ui/dynamics_tab.cpp` (around line 165) types the noun after a count; print it through `counted` so one note reads "1 of 1 kick note" only if `counted` gives that for the total. Check what `counted` does with "N of M" and follow it; if the phrase needs a form `counted` lacks, stop and report.

## Owned files

`src/app/display_format.h`, `src/app/display_format.cpp`, every file whose only change is the ms formatter call (list them in your report; `src/app/report.cpp` and `src/app/path_view.cpp` included), `src/ui/library_model.cpp`, `src/ui/dynamics_tab.cpp`, `src/app/dynamics_breakdown.cpp`, `src/app/dynamics_breakdown.h`, `tests/test_display_format.cpp`, `tests/test_path_view.cpp`, `tests/test_report.cpp` (re-pins only), `tests/test_store.cpp`, `tests/display_fixtures.h`, `tests/test_library_model.cpp`, `tests/test_dynamics_breakdown.cpp`, `tests/ui/uitest_paths.cpp` (re-pins only), `tests/test_single_owner.cpp` (your rows only).

## Tests you may run

`-sf=*test_display_format*`, `-sf=*test_path_view*`, `-sf=*test_report.cpp`, `-sf=*test_store*`, `-sf=*test_library_model*`, `-sf=*test_dynamics_breakdown*`, `-tc="single-owner*"`, plus the test file of any other formatter caller you change; build `hydra_uitest` and run `--test paths-rows`, `--test paths-folds-copy`, `--test library-search`.

## Done when

Each item has one owner and a test. No score, path or record change. Visible changes only: the space in one-decimal ms, the tooltip's 170.5, and the dynamics count if `counted` changes it.
