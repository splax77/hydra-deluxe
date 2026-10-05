Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task FX-L: M_D review fixes, Library, Paths, model

Task id: FX-L. Base: b6ec982 (the reviewed claude/p3-d2 tip). Branch `claude/p3-fx-l`. Worktree `.claude\worktrees\p3-fx-l`.

## Goal

The M_D derive-once review found copies in the Library, Paths tab, Dynamics and tests. Its full text, with every file and line, is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\review-M_D.md`, section "Part: library" (findings 1 to 9), plus finding 7 of "Part: reports" and the `path_view.cpp` half of finding 3 of "Part: preview" (the same tick rounding). Read those first.

## What is left

1. **Dynamics row table (library 1).** `row_for` and `row_note` in `src/app/dynamics_breakdown.cpp` write the nine row/note pairs in opposite directions, and `pad_color` plus the cymbal-row hide in `src/ui/dynamics_tab.cpp` repeat it. Make one nine-entry table and derive all four from it. Colours and rows on screen stay the same.
2. **Hand-typed ms formats (library 3).** `src/app/path_view.cpp` (around lines 332, 334, 357) writes `%.1fms` and `%.0fms` by hand. Call the `display_format` owners (`format_ms`, `format_ms_whole` or whichever matches each spot today). The whole-ms owner's rounding wins: at a hit window of 85.25 the text reads 171, as the owner prints elsewhere. Pin that edge.
3. **Tick rounding (reports 7, preview 3, audit 183).** `path_view.cpp` (around line 393) rounds a time to a tick with its own `llround`; call `display_tick_at_ms`. Results differ only below 0.
4. **Ellipsis fit test (library 4, second half).** The library table's fit test in `src/ui/library_table.cpp` is a second copy of `ellipsize`'s; call the owner in `src/ui/widgets.h`. (FX-P removes the lambda in `preview_tab.cpp`; do not touch that file.)
5. **"Searching" test (library 5).** `!app.library.query().empty()` sits in both `library_toolbar.cpp` and `library_table.cpp`. Give it one owner (a small function on the library model or app state) and call it from both.
6. **Row colour (library 6).** `src/ui/paths_tab.cpp` picks "warning or dim" for the timeline outline, the badge text and the path button separately. One helper; all three call it. Colours stay the same.
7. **Fixture copies (library 7).** `stale_rows()` in `tests/test_library_model.cpp` rebuilds the three-cause Stale trio from `tests/test_store.cpp`; move it to one shared test fixture both use. Fold the repeated five-line `cache.details(...)` call in `tests/ui/uitest_paths.cpp` into one helper.
8. **Colour rule in a test (library 8).** `check_marks_follow_rows` in `tests/ui/uitest_paths.cpp` restates the orange/grey rule over the view's fields; pin the expected colour per row as literals.
9. **Loose scan rows (library 9).** In `tests/test_single_owner.cpp`, tighten the whole-ms row so `%.0fms` (no space) is caught and the ellipsis row so a typed "…" literal is caught, with the must-match and must-not-match lines the review proposes. (The counts row is FX-R's.)

The "Touched, already on the fix list" items (audit 261, 14, 172, 13, ledger fills-34, 219) stay as they are.

## Owned files

`src/app/dynamics_breakdown.cpp`, `src/app/dynamics_breakdown.h`, `src/ui/dynamics_tab.cpp`, `src/app/path_view.cpp`, `src/app/path_view.h`, `src/ui/library_table.cpp`, `src/ui/library_toolbar.cpp`, `src/ui/library_model.h` (the searching owner only; `library_model.cpp` belongs to FX-R, so put the owner in the header or in `app_state`), `src/ui/app_state.h`, `src/ui/app_state.cpp`, `src/ui/widgets.h`, `src/ui/paths_tab.cpp`, `tests/test_dynamics_breakdown.cpp`, `tests/test_path_view.cpp`, `tests/test_library_model.cpp` (the fixture move only; FX-R adds cases there too, so keep your edit to `stale_rows`), `tests/test_store.cpp`, a shared fixture header under `tests/` (new or existing), `tests/test_overlay_layout.cpp`, `tests/test_app_state.cpp`, `tests/ui/uitest_paths.cpp`, `tests/test_single_owner.cpp` (rows for these items only).

## Tests you may run

`-sf=*test_dynamics_breakdown*`, `-sf=*test_path_view*`, `-sf=*test_library_model*`, `-sf=*test_store*`, `-sf=*test_overlay_layout*`, `-sf=*test_app_state*`, `-tc="single-owner*"`; build `hydra_uitest` and run `--test paths-rows`, `--test paths-folds-copy`, `--test library-search`, `--test library-layout`, `--test dynamics` if that script exists.

## Done when

Each item has one owner and a test. No score, path or record change. The only visible change is the whole-ms rounding edge in item 2.
