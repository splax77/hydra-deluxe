# Verify window WN7 (agent aa1a0bfbad7518ad2)

Task T5-V-WN7, session e633f7f6-f313-4787-b622-8d901b577b64. Checked `records/window-WN7.md` against the worktree HEAD. Nothing run. All claims below are "by reading"; no run at 60 ms exists (`records-engine/` is at 85).

## Scope

I counted `TEST_CASE`s: test_path_list 15, test_pb_files 8, test_preview_boxes 53, test_app_library 52. Every case line has a row in the auditor's tables. None missing.

I redid the search. In `test_preview_boxes.cpp` the only search calls are `analyzed_by_ten_scores()` at 469 and 499 (the includes at 35-36 are unused). In `test_app_library.cpp` the only search reached is `run_redo_batch_over` (called at 95 and 121, both delta cases) and the click view job (selects at 87, 663, 675, 709, 1247, 1320). In `test_pb_files.cpp` every case calls `open_burnout`. In `test_path_list.cpp` the entry points are `analyzed_by_ten_scores` and `burnout()`. No unlisted entry point found.

## Rows

| Kind | Rows checked | Disputed |
|---|---|---|
| A | 0 | 0 |
| B | 109 (13 + 6 + 52 + 38) | 0 |
| C | 5 | 0 as to kind |
| delta round | 14 | not judged |

B reasons I checked in code, which hold:
- Squeezes (path_list 327, 339): `pather.cpp:27` copies `graph.multsqueezes()`. The engine's `early_fill_window_ms_` is read only in `engine.cpp` (859-869, 1007, 1224, 1937). `graph.cpp` stores the window and never branches on it.
- Replay and preview do not read the window: `fill_refuses` and `is_e0` appear outside `src/search` only in `model.h`, `model.cpp` and a comment in `path_view.cpp`. One caveat: `Activation::is_E0()` (`model.cpp:475`) reads the `e_window_ms` the engine stored on the activation (`engine.cpp:2030`). That does not touch the B rows, which use hand-built activations or relational checks.
- test_pb_files 123: ticks 1 and 2 hold no fill at any window (read 123-150).
- test_pb_files 170 and 179: both sides read the same best path or record path.
- test_path_list 94, 143, 169, 197, 229: relational to the same record (read 94-135 and 229-262). The silent skip at line 254 (`if (all.size() > 1)`) is confirmed.
- test_app_library 660, 672, 693: a click starts a real view analysis, but the asserts read only `selected`, `details_open()`, notespath, md5 and `any_job_running` (read 655-680).

Not proven: bodies neither of us read (the auditor's "Not read whole" list: many test_preview_boxes cases and test_app_library 810-1180). I grepped both files whole for `analy`, `Engine`, `corpus::`, `burnout`, `open_and_wait`, `early`, `window_ms`, `batch_run`, `run_batch` and found nothing beyond what the auditor lists. That supports B but does not prove it for those bodies.

## C records

| Record | Test | Verdict | Agree |
|---|---|---|---|
| 1 | path_list 268 | Rewrite | Yes |
| 2 | path_list 398 | Rewrite | Yes, one correction below |
| 3 | pb_files 50 | Rewrite | Yes |
| 4 | pb_files 84 | Investigate | Yes |
| 5 | preview_boxes 498 | Rewrite | Yes |

Claims confirmed by reading: record 1's typed values at path_list 272-299 and its source `burnout()` at the default window. Record 2's `act_open.size() == 3` at 406 and the unguarded `rec.all_paths()[1]` at 420. Record 3's lines 56, 59, 61 and its finding that lines 65-66 compare `path_file_json(...)["paths"]` with `paths_json(...)` of the same path. Record 4's lines 88, 91, 93, 103, 105-107. Record 5's lines 505-510 and that the chart is the unnamed first corpus chart with paths. Question 3 is "claim not checked" on all five, which is right: no Burnout dump at 60 exists. Question 8's bug at a line, per record (320 and 299; 412 and 422; 72; 103; 505 and 506), names a real mutation that would fail. I did not re-search the "first path wins" ruling for record 4; it still stands as the auditor left it.

Correction to record 2: the auditor lists "`:426-430` button build counts" as numbers that hold only while the best path has that many rows. Those counts (1, 2, 3) depend on the record generation and score range passed to `cache.buttons`, not on row count. The window cannot move them. Only 406 and the `{0, 0, 0}` at 407 can move. The verdict is unchanged.

## Splits

- path_list 272-299 and `tests/test_activation_rows.cpp:582` type the same Burnout values ("3- 1 2", 163.0 ms): two typed copies of one run.
- test_pb_files 56 and 88, and path_list 406, each type a fact about Burnout's best path's activation count: three copies.
- preview_boxes 509 types `stars == 7` beside 505, which compares with `path_stars(best)`, the owner: a typed second answer to a question the owner answers.

## Typed numbers

The auditor's list is complete for what I read, except the path_list 426-430 correction above. I add none.

## Not reached

Bodies of the B cases in the auditor's "Not read whole" list, beyond the grep and helper checks above.

## Stopped on

Nothing.
