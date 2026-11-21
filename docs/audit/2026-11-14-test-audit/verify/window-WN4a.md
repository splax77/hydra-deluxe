# Verify: window round WN4a (agent a77a43649e3f4582b)

Task T5-V-WN4a, session e633f7f6-f313-4787-b622-8d901b577b64. I checked `records/window-WN4a.md` against the code at the worktree HEAD. Nothing was run. Every claim below is "by reading" unless it names a run; no run in the records folders exists at 60 ms, so no claim here is "proven by" a 60 ms run.

## Scope

I counted the test cases in each of the six files with a grep for `TEST_CASE`. The counts match the auditor's tables: test_replay 48, test_replay_schedule 40, test_replay_windows 7, test_pathstring_read 10, test_preview_scene 33, test_preview_controller 28. No test case is missing from the tables.

I also searched each file for every route into the search (`analyze`, `corpus::analyzed`, `first_chart_with_paths`, `analyzed_by_ten_scores`, `run_search`, `search_target`, `ScoreGraph`, `search_graph`, `paths_from_pathstring`, `HydraRecord`, `early_fill`). Every hit is on a row the auditor sorted; no uncovered hit.

## A rows (0)

The auditor says no test sets `early_fill_window_ms`. Agreed: the only hit in `tests/` for the six files is none, and `src/` sets it only through `Settings`, `to_analysis_settings` and `analyze_chart` (`src/app/config.cpp:411`, `src/core/model.h:977`). `ScoreGraph(song, 4, FillDeadlineRule::Ch10)` defaults to 60 (`model.h:112`, graph constructor as the auditor cites). `analyzed_by_ten_scores` (`tests/analyzed_chart.h:16-25`) and `analysis_settings()` (`test_preview_controller.cpp:139-145`) are bare `AnalysisSettings`, so 60. `scores_settings` starts from `Settings()` (`record_fixtures.h:582-589`). Checked, agreed.

## B rows

Rows checked: all 150 B-row reasons were read against the table text; I opened the bodies of these and they hold: test_replay 340, 550, 649, 1649; test_pathstring_read 82, 90, 99, 115, 126, 135, 148, 175, 203, 212; test_preview_scene 475, 744, 967; test_preview_controller 171, 548, 575, 938; test_replay_schedule 267, 337. I did not open the bodies of the hand-built rows (the "no search runs" ones in test_replay 386 to 1517, schedule 106 to 1716, preview_scene hand-song rows); I confirmed by the grep above that none of them names a search route.

Disputes (none moves a row to C; each is a weaker B than the table says):

- `test_pathstring_read.cpp:90`, `148`, `203` (B). The reason "which chart is first moves nothing asserted" is not the whole truth. Each case ends in `FAIL("no corpus chart's best path has two windows")` (line 76) or `FAIL("... exactly two windows")` (line 168). So a 60 ms search that leaves no chart with two windows (148: exactly two) fails the case. The auditor listed these in "Weak floors" but left the B reason wrong. The refusal text itself ("window 2 (\"99\") ...") does not depend on the window. I keep B and ask the reason to say the FAIL branch can be reached.
- `test_replay.cpp:340` (B). The reason names `engine.cpp:574-581` finding a target by exact tick; the body (lines 340-358) also does `FAIL` at 357 when no corpus chart's best path has an activation, and picks `acts[0]` from the 60 ms best path. The assertion on the tick (`tick + 1`, empty result) holds at any window. Same shape as the three above: B, but with a reachable FAIL.
- `test_preview_controller.cpp:938`, `548`, `575`, `171` (B). The reason "no search value asserted" is right for the assertions (keys of one path against itself, a timeline compared with the renderer's), but each case starts from `first_chart_with_a_path()` or `analyze_with_paths`, which `REQUIRE` that the 60 ms search finds a path. That cannot fail on a normal corpus. B holds.
- `test_preview_scene.cpp:475` (B). The auditor says "no value typed". The body (lines 475-519) compares the scene with the best path on `analyzed_by_ten_scores`; it types nothing, but its loop is empty if the best path has no activation, and `CHECK(d.has_value())` reads the engine's stamp. B holds; the loop being able to run empty is the same floor the auditor lists for other rows.

I found no B row whose reason fails outright. The A and B tables stand.

## C records (15)

I read every record against the test lines and the helpers it cites. All 15 are accurate on what the test asserts and on the typed numbers, with the line numbers given. I confirmed by opening the lines:

- Record 1 (replay.cpp:104): `382, 382, 294` at lines 119-121, `CHECK(run.paths >= c.min_paths)` at 142 (by reading). Verdict Rewrite: agree.
- Record 2 (209): the 300 floor at 251, `search_target(song, cfg, ...)` at 216 (by reading). Rewrite: agree.
- Record 3 (311): settings rows at 321-322 and the typed skip counts; `Setting{4,4,0,10}` etc. (by reading). Rewrite: agree.
- Record 4 (611): `"0++"` and `"17360"` at 629/635, `CHECK(saw_0pp)` at 638, chart `test_fast_tempo/spent_then_next.chart` at `scores_settings(2)` (by reading, lines 613-614). Investigate, claim not checked only: agree. Note the case is red at 60 ms if "0++" is not a path, as the auditor says.
- Record 5 (871): `228710` and `"1 2"` at 881-882 on Allister - Overrated, one path through `corpus::analyzed` at the default settings (lines 878-880). Rewrite: agree.
- Record 6 (888) and record 9 (preview_scene 289): both run `make_ch10_fill_song` on `ScoreGraph(song, 4, Ch10)` with target 28800 at the default 60 ms. Agree the numbers `19200` and the three fill states are typed from a run on a hand-built fixture. Investigate, claim not checked only: agree. Question 3 correctly has no real chart to name.
- Record 7 (schedule 1514): `pitch_dark()` (1448-1461) takes `paths.front()` of `corpus::analyzed` at the defaults, so the first `REQUIRE(pd.acts.size() == 3)` and `steps.size() == 4` plus the seven ticks (1521-1532) are search output at 60 ms. All typed figures the auditor lists are on the lines named, and `kPitchDark243OwnEndMs = 100949.4546` is "pinned from one run of this test" in its own comment (line 1436). Rewrite: agree. One correction: the auditor says the REQUIREs "may fail outright at 60"; the CHECKs at 1527-1532 do not stop the case, only 1521, 1523, 1533-1534 `REQUIRE`s do. Not a verdict change.
- Record 8 (1621): `REQUIRE(p644.size() == 1)` at 1630 and `157545.2` / `158350.6` at 1638-1639 (by reading). Rewrite: agree.
- Record 10 (scene 898): `{2, 7}` at 903 on `analyzed_by_ten_scores`. Rewrite: agree.
- Record 11 (scene 925): `"3"` at 932, `{123838}` at 937, `{244800, 281759}` at 938, `"157922:H 180960:H 244800:O 281759:O "` at 953. Rewrite: agree.
- Record 12 (controller 243), 13 (614), 14 (672), 15 (869): checked against `first_chart_with_a_path`, `analysis_settings()` (depth 2, 10 ms) and `analyzed_by_ten_scores` (depth 10, 10 ms). Records 12 and 13 Rewrite, 14 and 15 Investigate (claim not checked only): agree. Record 13's line-620 `REQUIRE(path_identity(found.best) == path_identity(ghost))` is two searches at different depths (2 and 10) that must end on one best path: confirmed at 618-620.

Question 8 in each record names a bug at a line in plain words ("a replay that drops a doubling", "`follows_pins` at `engine.cpp:592`" and so on). The auditor marks them "reasoned". I read them; none is planted or run. I did not check `follows_pins` at `engine.cpp:592` line by line, only that `engine.cpp` has the target lookup near 574-592 as the auditor says.

Verdicts agreed: 15 of 15. Verdicts disputed: 0.

## Typed numbers

The auditor's list matches the typed numbers I found on the lines it names. I looked for others in the six files (a grep for typed `CHECK(... == <number>)` on search output). I found none the list misses in these files. The hand-built geometry numbers (`60545.26`, `53.59`, `-21.825`) are inputs; agreed they are not window-sensitive. The `85.0` at `test_replay_schedule.cpp:281` (`window_back_ms`) is the Clone Hero hit window, not the early-fill window; agreed.

## Splits (each one line)

- `test_preview_scene.cpp:903` and `test_preview_controller.cpp:643` type the same ghost list `{2, 7}` for one chart and path (auditor's split; agreed; the two charts come from `analyzed_by_ten_scores` in both).
- `test_replay.cpp:881-882` types Allister - Overrated's best score and path; `test_pb_text.cpp` (WN4b's file) types the same chart's score (auditor's split; I did not open `test_pb_text.cpp`).
- `test_preview_controller.cpp:620` requires two searches at different depths to end on one best path (auditor's split; agreed).
- `test_replay_schedule.cpp:355-356` pins `kHitWindowMs = 85.0` and `kSqueezeWindowMs = 500.0` in `src/core/model.h` as "Hydra's own two numbers", beside the filed ruling `code-and-engine.md` line 55 ("so we should use the hit window. and stop inventing our own numbers.") and the 60 default at `model.h:112`. Confirmed at the lines named. Whether these two statements conflict is for the main session.
- `test_preview_controller.cpp:139` (`analysis_settings()`: depth 2, 10 ms) against `tests/analyzed_chart.h:16-25` (depth 10, 10 ms) and `test::scores_settings` (top 40, no ms limit): three settings sets answer "the first chart's best path" in this one group of files. Listed by me, not by the auditor.

## Not reached

- `tests/test_preview_controller.cpp` bodies at lines 320-500, 730-865 and 926-1002 were checked by the grep only (no search token), as the auditor says. I did not open them.
- `tests/test_replay_windows.cpp` 226-493: grep only; its only search hits are at 205-208 and 495-537, on the rows the auditor sorted.
- Hand-built rows listed under "B rows" above: grep only.
- `test_pb_text.cpp` (WN4b), `engine.cpp:592`, the adding commits.

## Anything I stopped on

Nothing. No status call was made through the helper (the work was under the five-minute mark per call; I did not time it).
