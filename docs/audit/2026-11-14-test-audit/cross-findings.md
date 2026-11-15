# Findings that cross slices

A record judges one test. Some faults are one pattern repeated across many tests in many slices, and each record sees only its own copy. This file holds each such pattern as one finding, so T8 gives the pattern one exit and the closure check sees it whole. Each test's own record still carries its own verdict.

Counts below are from `git grep` over `tests/` at the audit branch's HEAD (code of main `5ec682c7`), run by the main session on 2026-11-15.

## X1. Burnout's best path and score are typed by hand in many tests

The best path of Burnout (`corpus::burnout()`, the corpus chart the GUI and unit tests use most) is the engine's answer. Many tests type that answer as a literal instead of reading it from the engine's record:

- The score `378315` or `378,315` appears 25 times in 10 test files: `tests/test_library_model.cpp` (10), `pb_ui/pbtest_build.cpp` (3), `pb_ui/pbtest_window.cpp` (2), `test_stars.cpp` (2), `ui/uitest_details.cpp` (2), `ui/uitest_paths.cpp` (2), and once each in `pb_ui/pbtest_coach.cpp`, `test_activation_rows.cpp`, `test_path_list.cpp` and `test_pb_coach.cpp`.
- The path text `"3- 1 2"` appears 23 times in 10 test files: `test_library_model.cpp` (7), `ui/uitest_paths.cpp` (4), and two or one each in `test_activation_rows.cpp`, `test_overlay_layout.cpp`, `test_path_list.cpp`, `test_store_records.cpp`, `test_preview_scene.cpp`, `test_sp_meter_curve.cpp`, `ui/uitest_details.cpp` and `ui/uitest_preview.cpp`.
- Other Burnout figures from the same run are typed beside them: "378,175", "375,955", "0 4 1", "163.0 ms", "2,360 below optimal", m20 → m24, m32, and the activation count 3.

Why it is a bad test: the rubric's question 5 (an expected value with no named source) and the derive-once rule. Each copy is a second answer to "what is Burnout's best path", with no owner. A change to the engine that moves Burnout's result turns all of them red at once, whether or not the change is a bug. A real bug that moves it gets the same flood. So the tests cannot tell the two apart, and the one test that should own the fact is lost among the copies.

Whether the window change from 85 to 60 ms moved Burnout's result is not yet known. The tests pass on main, which suggests it did not, but no engine record of Burnout exists at either window. The capture pass takes `hydra_replay dump` of Burnout at `--efill 60`.

The records that already see a copy (each judges only its own): `records/A2.md`, `A3.md`, `A19.md`, `A26.md`, `A33.md`, `A39.md`, `A44.md`, `A45.md`, `A48.md`, `A49.md`, `C5.md`, `C6.md`, `C7.md`, `C12.md`, `delta-DL4.md`, `window-WN2.md`, `window-WN4b.md`, `window-WN4c.md`, `window-WN5.md`, `window-WN6.md`, `window-WN7.md`, and their verify files. WN5, V-WN6 and V-WN7 name it as a split (one fact, several homes).

Exit: not chosen. Only the main session picks it, in T8.

## X2. The Path Builder tests click fill ticks typed from one run

`tests/pb_ui/pbtest_harness.h:60-61` types `kFirstFillTick = 36480` and `kSecondFillTick = 74880`, the fills the player can activate on, "from one run under the GUI tests' settings". The same ticks are typed again as `kEarliestTick` (`pb_ui/pbtest_window.cpp:178`) and appear in `test_pb_coach.cpp`, `test_pb_state.cpp` and `test_fills_and_skips.cpp`. Related ticks from the same kind of run: `kLongSongFillTick` 133920 (`pbtest_build.cpp:63`) and `kLateFillTick` 162240 (`pbtest_window.cpp:390`).

Why it is a bad test: which fills are offered is decided by the engine's early-fill rule (`fill_refuses`, `src/core/model.h:331`). The tests type its answer instead of asking the engine for the offered fills. A rule change that stops offering one of these fills makes every test that clicks it fail at the click, with no bug, and the failure names the click, not the rule. The run the ticks came from predates the window change, so whether both fills are still offered at 60 ms is not yet known; the capture pass dumps the chart at `--efill 60`.

The records that see it: `records/window-WN5.md`, `window-WN6.md`, `A48.md`, `A49.md`, and their verify files.

Exit: not chosen. Only the main session picks it, in T8.
