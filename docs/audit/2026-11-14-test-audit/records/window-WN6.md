# Window round, group WN6 (T5-WN6, agent ae12a6fdb4544973f)

Group: `tests/ui/uitest_batch_reports.cpp`, `tests/ui/uitest_tests.cpp` (with `register_script_test`, `tests/ui/uitest_script.cpp:57`), `tests/pb_ui/pbtest_build.cpp`, `pbtest_coach.cpp`, `pbtest_files.cpp`, `pbtest_window.cpp`, and the typed fill ticks `kFirstFillTick` and `kSecondFillTick` (`tests/pb_ui/pbtest_harness.h:60-61`). Every test body below was read whole, at this worktree's HEAD. No test case of this group has a record in `records/delta-DL*.md` (a grep for `pbtest` and `uitest_batch` there finds none), so no row is "delta round".

## Which window these tests run under

Both runners write their scratch `hydra_settings.ini` from `scratch_settings()` (`tests/scratch_settings.h:20-25`, called at `tests/ui/uitest_harness.cpp:249` and `tests/pb_ui/pbtest_harness.cpp:101`). That struct sets only `chartfolders`, `auto_open_report` and `depth_value`. `Settings::early_fill_window_ms` defaults to `kDefaultEarlyFillWindowMs` (`src/app/config.h:195`, `src/core/model.h:112` = 60.0) and reaches the search through `Settings::to_analysis_settings` (`src/app/config.cpp:411`). No test of this group sets the window. So there is no kind A row. Every test here that runs a search runs it at 60 ms. `fill_refuses` and `is_e0` (`src/core/model.h:331,335`) are read by the search's activation branching and by `Activation::is_E0` and `is_e_critical` (`src/core/model.cpp:473-475`), so they change which fills a path may activate on and which E marks it carries.

No engine record at 60 ms exists. All of `records-engine/` is at 85, and a grep for `--efill` over `records-engine`, `records` and `verify` finds only window-round files. So each C record's question 3 needs the dump named in it (`hydra_replay dump --efill 60` of that chart), and until it exists the claim is "claim not checked".

How the Path Builder decides which fills show as offered: `src/app/preview_view.h:70-72` says a fill is Hidden when "not enough SP banked, or SP was already running", Offered when "shown and passed over: the path could have activated here", Taken when the path activates on it. `preview_view.cpp:470-495` marks Offered only the fills a path's activations list as passed over. So the set of fills the list rows and the clicks offer comes from the search's paths, which the window moves. A fill drawn on the timeline in any state, including Hidden, does not depend on the window: the build test clicks hidden fills at ticks 83040, 59520 and 21120.

## Typed fill ticks in the harness header

`kFirstFillTick = 36480` and `kSecondFillTick = 74880` (`pbtest_harness.h:60-61`) are commented "Burnout's first two fills the player can activate on (m20 and m40), by chord tick, from one run under the GUI tests' settings". They come from a run before the default window became 60. A run at 60 can offer a different first or second fill. Readers: `pbtest_build.cpp:163,178,202-203,215`, `pbtest_coach.cpp:364,374,385,389-390,467,489,497,506,521,548,550`, `pbtest_window.cpp:487`. The same holds for `kEarliestTick = 36480` (`pbtest_window.cpp:178`), `kLongSongFillTick = 133920` (`pbtest_build.cpp:63`) and `kLateFillTick = 162240` (`pbtest_window.cpp:390`), which are offered fills. `kCantFillTick` 83040, `kLockedFillTick` 59520 (`pbtest_build.cpp:45-46`) and `kUnderTwoTick` 21120 name fills the window cannot unhide.

## Table of test cases

| File:line | Test case | Kind | Reason or record |
|---|---|---|---|
| uitest_tests.cpp:17 | `register_tests` | none | Holds no test body; it registers other files' entries (lines 20-35). No search. |
| uitest_script.cpp:57 | `script` (`register_script_test`, body `run_script` :19) | B | Asserts nothing of its own: it runs whatever command file it is given and prints each result (lines 32-42); it fails only on a refused command. No typed value to move. |
| uitest_batch_reports.cpp:109 | batch-strip-drift | B | Every chart waits at `BatchGate` (`uitest_harness.cpp:316-336`). Asserts the Stop button and strip width positions and that titles change. No result value is read. |
| :158 | settings-and-reports | B (flag) | A real search runs on library row 0, "Acid Romance" (`uitest_details.cpp:261`). Asserts `view_bass2x` toggles, report window state, and `under_optimal == 1` for a canned score of 100,000 (`uitest_harness.cpp:206`). The status rule is `src/app/dm_report.cpp:131-137`: under when the score is below the optimal. The window moves an optimal by a few fills' worth of points. Acid Romance has 962 played notes (`tests/test_dynamics_breakdown.cpp:197`); at 50 points a note with up to a 4x multiplier the score with no Star Power is near 190,000, well above 100,000. That figure is my arithmetic, not a run. A verifier should confirm it; if the optimal could come within a few thousand of 100,000 this would be C. |
| :204 | dm-compare-flow | B | No analysis runs, only `scan_library`. Asserts `not_analyzed == 1`, `under_optimal == 0` because nothing has a result (comment 257). The counts are the join's, not search values. |
| :282 | report-buttons | B | Batches one chart and asserts button presence, `auto_open_report`, window open flags and `skipped == 0` for a redo (line 327). A redo skips nothing whatever paths are found. No result value. |
| :339 | batch-confirm-note-shuffle | B | No analysis runs. Asserts the confirm line's setting words. |
| :379 | batch-confirm | B | The text read is the confirm's settings line: "Analyze 98 charts that have no result yet?", "4 bars", "2 scores", "10 ms" (lines 389-393). 98 is a library count, 10 ms the Path limit; the window is not named. The held run is stopped at the gate before it analyzes. |
| :418 | batch-pause-stop | B | Gate holds the run; asserts Pause, Resume, Stop and "Stopped:". |
| :455 | batch-strip-workers | B | Gate-held. Asserts counts of charts started and counted (2 and 5 of 98): chart counts, not path values. |
| :530 | batch-done-strip | B | Real search on "Burnout". Asserts report build state Ready, strip words, buttons and the persisted `auto_open_report`. The report's contents are not read. |
| :582 | batch-open-failure | B | Real search. Asserts the report window opens and the strip names no browser. |
| :599 | status-line | B | No search. |
| :625 | analyze-db-fails | B | No search; the database table is dropped first. |
| :650 | compare-disabled | B | No search; reads the button's disabled flag. |
| :677 | dialog-keys | B | Scan only. |
| :705 | batch-chip-count | B (flag) | Whole-library batch (98 charts). Asserts `done.analyzed > 0` and that the Analyzed chip shows that number (lines 716-718). A chart counts as analyzed when its job finishes, not by the path found; I did not read the code that sets `analyzed`, so this is by reading of the assertion only. A verifier should check that no chart moves between analyzed and failed with the window. |
| pbtest_build.cpp:135 | build-a-path-by-clicking | C | Record 1. |
| :334 | build-a-path-on-a-long-song | C | Record 2. |
| :460 | follow-the-playhead | B | The search runs (the long song opens), but the test asserts only the scrollbar readout windows, the clock, Play/Pause state and the owners `open_view`, `follow_playing`, `wheel` (lines 463-520). Their inputs are the song length and the clock. The only typed text, "0:00 – 3:00 of 5:59" and "2:59 – 5:59 of 5:59", is set by the chart's length. None reads a path, a score or a fill. |
| pbtest_coach.cpp:76 | finish-for-me | C | Record 3. |
| :119 | cant-activate-reason | B | Clicks the fill at 21120, which has one phrase saved. Asserts no edit and the card sentence "You have 1 phrase saved here. You need 2 to activate." `fill_reason` (`pb_text.cpp:49-77`) reaches that branch only when the fill is not Offered. A fill with 1 bar cannot be activated by any path (it needs `kSpActivationBars` = 2). A narrower window only refuses more fills (`fill_refuses` is `e_offset < -window`), so it cannot make this fill offered. The phrase count is read from the Preview's meter curve (chart data). |
| :141 | locked-fill-says-left-to-right | C | Record 4. |
| :168 | tight-window-note | C | Record 5. |
| :220 | choose-an-ending | C | Record 6. |
| :360 | hover-moves-nothing | C | Record 7. |
| :405 | drag-moves-the-clock | B | Reads clocks at pointer shares of the track ("0:32.428", "1:04.981") and at the hit box of the second fill ("0:51.459"). The track length is the chart's. The fill's item is drawn in every state, at a place the chart sets. No path or offer is read and no edit is made (`generation` unchanged). |
| :463 | activations-row-never-drags | B | Press on the first fill's item, drag away, release. Asserts the clock stays and `generation` is unchanged. The item exists in any state. No path read. |
| :485 | fill-click-moves-clock | C | Record 8. |
| :517 | fill-card | C | Record 9. |
| :545 | row-click-moves-clock | C | Record 10. |
| pbtest_files.cpp:26 | paste-refused-keeps-path | B | Pastes the record's own `best_path()` text, copies it back, pastes "99". Every compared value is read from the same run (`best.pathstring()`, `copy_text`). The refusal "window 1 ("99") matches no fill on this chart" needs only that no fill is numbered 99. The best path the builder accepts is the one its own search found under the same `chart->settings`. A moved window changes which path is best, not that it round-trips. |
| pbtest_window.cpp:68 | open-starts-empty | C | Record 11. |
| :237 | head-row-wraps | B | Layout floats of the head row at three frame widths, after the search has finished (`wait_idle`). No search value. |
| :341 | scroll-the-timeline | C | Record 12. |
| :410 | drag-stays-in-the-window | B | Clocks and readouts set by the song length and the pointer's place. No fill is clicked. |
| :447 | no-star-power | B | A hand-built three-note chart with no Star Power phrase (lines 451-453). With no SP there is nothing for `fill_refuses` or `is_e0` to decide. Asserts "nothing to build" and the title. |
| :467 | file-open-error | B | The chart file does not exist; no search runs. |
| :509 | bad-settings-refused | C | Record 13. |
| :546 | bad-rules-refused | C | Record 14. |
| :592 | rules-reach-the-builder | C | Record 15. |

Counts per file. `uitest_tests.cpp`: 0 test cases (1 registrar). `uitest_script.cpp` (the script test): A 0, B 1, C 0. `uitest_batch_reports.cpp`: A 0, B 15, C 0, delta 0. `pbtest_build.cpp`: A 0, B 1, C 2. `pbtest_coach.cpp`: A 0, B 3, C 8. `pbtest_files.cpp`: A 0, B 1, C 0. `pbtest_window.cpp`: A 0, B 4, C 5. Total: B 25, C 15, A 0, delta 0.

Verdicts of the 15 C records: Rewrite 11, Investigate 4 (all "claim not checked only"), Keep 0, Remove 0.

## Facts shared by all C records

- The old records are `records/A48.md` (pbtest_build, pbtest_coach, pbtest_files) and `records/A49.md` (pbtest_window). The file headers say the figures are "pinned from one run under the GUI tests' settings" (`pbtest_build.cpp:7-8`, `pbtest_coach.cpp:2-3`). Those runs were before the default window became 60. So each typed figure below is a snapshot of an earlier window, and each is a finding on its own line.
- Question 2 for all: the screen's words are the Path Builder's (`src/pathbuilder/pb_text.cpp`, `pb_coach.cpp`). The numbers are the engine's (`HydraRecord::best_path()`, `Path::totalscore`, `Activation::window_end_tick`, `DecisionList`). The display decides nothing here (agent rule 15); the tests type what it shows.
- Question 6 for all: these are display tests with typed expected values, which the rubric marks Fails. The old verdicts said the same.
- Question 11 for all: the charts are corpus charts (`testdata/input`), real.

## Records

### 1. build-a-path-by-clicking (`tests/pb_ui/pbtest_build.cpp:135`)
Delta round: window, replaces records/A48.md record 1
The old verdict was Rewrite. It still holds at 60 ms: its fault, typed figures, is in the test, and the window adds a second reason for the same fix.
1 Claim: Clicking fills and list rows on Burnout builds a path step by step; scores, rows and cards show the engine's windows; refusals change nothing; Undo, Finish and Start over each do their one thing.
2 Owner: as shared. The "best" mark and the "Best choice now" line come from the decision list (`src/pathbuilder/pb_coach.cpp`).
3 Real chart: claim not checked. Burnout is not among the seven dumped charts, and every dump is at 85. Needs `hydra_replay dump --efill 60` and `schedule` of Burnout (`corpus::burnout_notes`), beside a Path Builder capture.
4 Direct: every figure is typed. Movable by a 60 ms run, by line: 147-148 "378,315" (best possible, best reach); 155 the count of "best" fills (1); 156 the row "m32 0:40.945 · 3 phrases · ends m37 · squeeze out 163.0 ms best"; 162 and 164 "Best choice now is m32 (378,315)."; 167 "Best finish here | 378,175"; 168 "Cost | −140"; 169 "Hydra would end it at m24."; 177 "m20 → m24 2 phrases −140"; 192 "327,135"; 193 "378,175"; 200 and 214 "m40 ... −1,580"; 228 "340,675"; 229 "376,595"; 292-293 "m58 → m68 4 phrases best" and "m88 → m92 2 phrases best"; 297 "378,175"; ticks 36480 and 74880 (harness). Not movable: 146 "313,635" (score with no activation), 262 "0 phrases saved" (chart data), 83040 and 59520. Properties that stay valid: 185, 191, 227, 251, 264-265, 277, 284, 295-296, 304.
5 Reason: "from one run"; the ticks were what that run offered.
6 Display: Fails.
7 Failure: commit `6f98b9d5`, subject "One GUI test builds a whole path on Burnout by clicking (PB-6)": a regression of building by clicks. Still possible.
8 Bug at a line: `pbtest_build.cpp:227` `two_step_score != one_step_score` goes red if the second click added a step row but left the score unchanged (a skipped rescore in `PbApp::click_fill`, `pb_window.cpp:668-690`). Reasoned, no planted run.
9 Stand-in: `on_screen` is a substring over the whole window; `figure_under` takes "the line after a label".
10 Same rule: coach `fill-card`, `cant-activate-reason`, `locked-fill-says-left-to-right`, `finish-for-me` and `row-click-moves-clock` repeat its figures; `tests/test_pb_text.cpp` pins the sentences. A 60 ms change moves them together.
11 Real: yes.
Verdict: Rewrite. Read each step's window, cost and the three scores from the record at the run's own settings, type no figure, and take the fills to click from the record's offered list. Fresh evidence needed: Burnout at `--efill 60`.

### 2. build-a-path-on-a-long-song (`pbtest_build.cpp:334`)
Delta round: window, replaces records/A48.md record 2
The old verdict was Rewrite. It still holds.
1 Claim: On the six-minute Set the World on Fire the window turns forward; a card on a far fill shows its figures; a click there starts a path; "< Act" brings the window back to it.
2 Owner: window readouts are `ui/timeline_view.h`'s; card and scores as shared.
3 Real chart: claim not checked; that chart is not dumped. Needs its dump at `--efill 60` and a capture.
4 Direct: movable: 349 "Best choice now is m15 (1,150,660)."; 352 "m69 · 3:15.859" and 353 "Phrases saved | 4" (only the offer, not the time, can move); 354 "Best finish here | 944,680"; 355 "Cost | −205,980"; 356 "Hydra would end it at m90."; 362 "m69 → m90 4 phrases −205,980"; 363 "879,860"; 364 "944,680"; tick 133920 (line 63). Not movable: 340 "729,960" (no activation); the readouts 339, 346, 371, 376, 381 and the clocks (chart length, 5 s jump, 45 s lead).
5 Reason: "Every figure is from one run".
6 Display: Fails.
7 Failure: commit `fb1211fc`: fills past the first window not reachable. Still possible.
8 Bug at a line: line 381 `"2:30 – 5:30 of 5:59"` goes red if "< Act" stopped moving the window.
9 Stand-in: `readout_reads` is a substring of the window.
10 Same rule: `follow-the-playhead` pins the same readouts; `scroll-the-timeline` (`pbtest_window.cpp:341`).
11 Real: yes.
Verdict: Rewrite. Window readouts from `open_view` and the chart length; card and step from the record at the run's window.

### 3. finish-for-me (`pbtest_coach.cpp:76`)
Delta round: window, replaces records/A48.md record 4
The old verdict was Rewrite. It still holds.
1 Claim: After one click on Burnout's second best-path fill, "Finish the rest for me" makes the player's path the best finish that keeps that window, and the panel lists the second step as best.
2 Owner: `DecisionList::click_fill` and the best-finish ladder (`pb_coach.cpp`). The fill clicked is read from the record's best path (line 80), which is the right shape.
3 Real chart: claim not checked: Burnout at `--efill 60`.
4 Direct: movable: 89 pathstring "9"; 90 total 349,875; 93 "m58 → m68 4 phrases −13,640"; 101 "9 1"; 102 total 364,675; 106 "m88 → m92 2 phrases best". Properties: 81, 83, 86-87, 103-104.
5 Reason: "From one run" (88, 100, 105).
6 Display: Fails; the click tick is the engine's, the figures typed.
7 Failure: commit `462ea5cf`: Finish building the wrong path. Still possible.
8 Bug at a line: line 103 `windows.size() == 2` goes red if Finish added no second window.
9 Stand-in: the row text.
10 Same rule: build test 286-297.
11 Real: yes.
Verdict: Rewrite. Compare the path with the record's best path that keeps the one window.

### 4. locked-fill-says-left-to-right (`pbtest_coach.cpp:141`)
Delta round: window, replaces records/A48.md record 6
The old verdict was Rewrite. It still holds.
1 Claim: After one activation, a click on a fill before it changes nothing and its card says the builder goes left to right.
2 Owner: `fill_reason`'s BeforeLast branch (`pb_text.cpp:65-67`) and `kBeforeLastActivationText`.
3 Real chart: claim not checked: Burnout at `--efill 60`.
4 Direct: movable: 151 the step row "m58 → m68 4 phrases −13,640" (the setup click is on `best[1]`, from the engine). Not movable: 150 tick 21120, 158 "m12 →" absent, 159 the owner's constant.
5 Reason: "From one run".
6 Display: Fails on the typed row; the constant is read from the owner.
7 Failure: commit `c5eccf08`. Still possible.
8 Bug at a line: line 157 `on_screen(h, step_row)` goes red if the click added a step at m12 and displaced the first row.
9 Stand-in: substring.
10 Same rule: build test 267-277.
11 Real: yes.
Verdict: Rewrite. Take the step row from the record.

### 5. tight-window-note (`pbtest_coach.cpp:168`)
Delta round: window, replaces records/A48.md record 7
The old verdict was Investigate (claim not checked only). It still holds, with a stronger reason.
1 Claim: On White Washed with a Path limit of 10 ms, the best path's fourth window needs more, so its opened step shows its ending line and a violet note "needs 130.4 ms, your limit is 10 ms".
2 Owner: `Activation::window_end_tick`, the squeeze figure, chip token t5.
3 Real chart: claim not checked; White Washed is not dumped. Needs `hydra_replay dump --efill 60` and `schedule`.
4 Direct: movable: 174 `windows.size() == 5` (the best path's activation count); 182 "needs 130.4 ms"; 183 "(x) ends m135 · squeeze out 130.4 ms"; line 179 clicks step index 3 and assumes the fourth step is the tight one. A different best path at 60 moves which step is fourth and the figure. Not movable: 172 `ms_filter == 10`.
5 Reason: comment 175-178 "From one run".
6 Display: Fails on the typed figures.
7 Failure: the note missing or in the wrong place. Still possible.
8 Bug at a line: line 187 `text.find(kept_line) < text.find(note)` goes red if the note were drawn above the ending line.
9 Stand-in: substring of the window.
10 Same rule: none found.
11 Real: yes.
Verdict: Investigate: claim not checked only (a dump at `--efill 60`). The typed 5, 130.4 and m135 must then come from that record.

### 6. choose-an-ending (`pbtest_coach.cpp:220`)
Delta round: window, replaces records/A48.md record 8
The old verdict was Investigate (claim not checked only). It still holds.
1 Claim: On "I'm A Believer", whose best path "1 0+ 0 0" has a second activation that can end at m78 (squeeze in) or m76 (squeeze out), both 0.0 ms, the opened step lists both lines, a click on the other moves the mark and the score, and after a third activation the lines are fixed.
2 Owner: the engine's tied variants and `end_choices` (`pb_coach.cpp:78`).
3 Real chart: partly checked, at 85 only. `records-engine/believer.schedule.json` holds activations at ticks 26880, 52992 and 74496 (the `act_tick` lines), a deact at 59136 and a nominal end at 57600 for the second, and `believer.dump.json:107` the pathstring "1 0+ 0 0". That agrees with the typed 228-232 at 85. It says nothing about 60. Needs a dump and schedule at `--efill 60`.
4 Direct: movable: 228-230 ticks 26880, 52992, 74496; 231-232 end ids 59136 and 57600; 240 "m70 → m78 3 phrases · squeeze in 0.0 ms best"; 242-243 the two ending lines; 244 "336,575"; 252 "m70 → m76 ... −2,680"; 255 "330,075"; 263, 272. A 60 ms run can change which ending carries the E mark or the tie.
5 Reason: "From one run".
6 Display: Fails on typed rows; badge words are the Paths tab's.
7 Failure: choosing an ending not moving the mark or the score. Still possible.
8 Bug at a line: lines 277-278 (`kept` and `other` disabled) go red if the fixed step stayed clickable.
9 Stand-in: glyph-colour counting (`glyphs_in`).
10 Same rule: none.
11 Real: yes.
Verdict: Investigate: claim not checked only (a 60 ms dump of Believer). The typed ticks and scores then come from it.

### 7. hover-moves-nothing (`pbtest_coach.cpp:360`)
Delta round: window, replaces records/A48.md record 9
The old verdict was Rewrite. It still holds. The test is C and not B because lines 373-374 and 389-390 scroll to and hover `choice_ref(kFirstFillTick)`, a list row that exists only when that fill is offered, and `scroll_side_to` returns false (the test fails) when it is not.
1 Claim: A hover over a fill or list row moves neither the clock nor Play/Pause; a hover does not pause a playing Preview.
2 Owner: the Preview controller and the timeline.
3 Real chart: claim not checked: a capture; the offered list needs Burnout at `--efill 60`.
4 Direct: movable: tick 36480 as an offered list row (373, 389). "m20 · 0:25.123" (365) has a chart-set time, but only if the fill's card shows. Not movable: the parked clock "0:10.000 / 2:10.088" (322).
5 Reason: ticks "from one run".
6 Display: Fails on typed clocks.
7 Failure: a hover seeking the clock. Still possible.
8 Bug at a line: line 366 goes red if a hover seeked the clock.
9 Stand-in: substring clock text.
10 Same rule: row-click-moves-clock.
11 Real: yes.
Verdict: Rewrite. Find the fill from the record's offered list; compare the clock before and after.

### 8. fill-click-moves-clock (`pbtest_coach.cpp:485`)
Delta round: window, replaces records/A48.md record 12
The old verdict was Rewrite. It still holds.
1 Claim: A click on a fill leaves the clock on that fill's dot, also after the pointer leaves, also while playing.
2 Owner: the Preview controller; the click is `PbApp::click_fill`.
3 Real chart: claim not checked: Burnout at `--efill 60`.
4 Direct: movable: lines 490 and 507 `wait_for_edit` after a click on kFirstFillTick and kSecondFillTick fail if a click on that tick no longer makes an edit (not offered at 60). 510 "0:51.421 / 2:10.088" and 305 "0:25.123" are chart times of those fills, tied to those two ticks.
5 Reason: "from one run".
6 Display: Fails.
7 Failure: the clock not following a click. Still possible.
8 Bug at a line: line 491 goes red if the clock stayed at the parked 10 s after the click.
9 Stand-in: substring.
10 Same rule: row-click-moves-clock.
11 Real: yes.
Verdict: Rewrite. The clock equals the fill's chord time from the record.

### 9. fill-card (`pbtest_coach.cpp:517`)
Delta round: window, replaces records/A48.md record 13
The old verdict was Rewrite. It still holds.
1 Claim: The card on Burnout's first fill shows its heading, three rows and two lines.
2 Owner: `fill_hover_card` (`pb_text.cpp`).
3 Real chart: claim not checked: Burnout at `--efill 60`.
4 Direct: movable: 522 "Best choice now is m32 (378,315)."; 527 and 534 "378,175"; 529 and 535 "−140"; 530 "Hydra would end it at m24.". Not movable: 533 "Phrases saved | 2" (chart data). Lines 525-530 repeat 533-535.
5 Reason: "the figures ... from one run".
6 Display: Fails.
7 Failure: the card's rows wrong. Still possible.
8 Bug at a line: line 534 goes red if "Best finish here" showed the cost figure.
9 Stand-in: substring.
10 Same rule: build test 162-169; `tests/test_pb_text.cpp:301-316`.
11 Real: yes.
Verdict: Rewrite (one verdict with record 1).

### 10. row-click-moves-clock (`pbtest_coach.cpp:545`)
Delta round: window, replaces records/A48.md record 14
The old verdict was Rewrite. It still holds.
1 Claim: After one activation, a click on the second fill's list row leaves the clock on that fill's dot.
2 Owner: the Preview controller.
3 Real chart: claim not checked: Burnout at `--efill 60`.
4 Direct: movable: 548 `click_fill(kFirstFillTick)` and 550 `choice_ref(kSecondFillTick)` need those fills offered; 555 "0:51.421 / 2:10.088" is that fill's chart time.
5 Reason: "From one run".
6 Display: Fails.
7 Failure: a list-row click not moving the clock. Still possible.
8 Bug at a line: line 555 goes red if the row click left the clock on the first fill.
9 Stand-in: substring.
10 Same rule: fill-click-moves-clock.
11 Real: yes.
Verdict: Rewrite (with record 8).

### 11. open-starts-empty (`pbtest_window.cpp:68`)
Delta round: window, replaces records/A49.md record 1
The old verdict was Rewrite. It still holds.
1 Claim: A chart from the command line opens with no path, three scores, hover-only legends and a disabled "< Act"; a click on the highway at the earliest takeable fill activates there.
2 Owner: `PbApp`, `fill_reason`, `project_to_image` for the click.
3 Real chart: claim not checked: Burnout at `--efill 60`.
4 Direct: movable: 76-78 the pending row "m20 0:25.123 · 2 phrases · ..." (the first listed fill); 178 `kEarliestTick` 36480, "earliest fill the player can take"; 203 "m20 → m24 2 phrases −140". Not movable: 171 "You have 1 phrase saved here..." and line 167's not-Offered check on 21120 (a narrower window cannot offer it), 97 "0:00 – 2:10 of 2:10", 92, 88.
5 Reason: "From one run".
6 Display: Fails on the typed row and tick.
7 Failure: the screen starting with a path, or the highway click not activating. Still possible.
8 Bug at a line: line 84 `!current().path.has_value()` goes red if the builder pre-filled a path.
9 Stand-in: substring.
10 Same rule: build test 144-156.
11 Real: yes.
Verdict: Rewrite. Take the first fill and the earliest takeable fill from the record.

### 12. scroll-the-timeline (`pbtest_window.cpp:341`)
Delta round: window, replaces records/A49.md record 3
The old verdict was Rewrite. It still holds. The test is C because lines 397-398 click `choice_ref(kLateFillTick)` (162240, about 3:58) and `wait_for_edit` needs that fill offered in the list.
1 Claim: The scrollbar buttons, the wheel, a thumb drag and a list-row click move the window as the readouts say.
2 Owner: `ui/timeline_view.h`.
3 Real chart: claim not checked: Set the World on Fire at `--efill 60`, offered list near 3:58.
4 Direct: movable: 390 tick 162240 as an offered list row; 399 "2:28 – 5:28 of 5:59" (centred on that fill; its time is chart data). Not movable: the other readouts 358-384.
5 Reason: "from one run".
6 Display: Fails on typed readouts.
7 Failure: the window not following a list-row click. Still possible.
8 Bug at a line: line 395 goes red if hovering the row scrolled the window.
9 Stand-in: substring.
10 Same rule: the long-song test.
11 Real: yes.
Verdict: Rewrite.

### 13. bad-settings-refused (`pbtest_window.cpp:509`)
Delta round: window, replaces records/A49.md record 7
The old verdict was Investigate (claim not checked only). It still holds. The test is C only through its setup: `check_open_song_stays` (line 487) calls `click_fill(ctx, kFirstFillTick)` and checks that one window was made (line 490). If 36480 is no longer offered at 60, the test is red with no bug in the refusal.
1 Claim: A bad settings file refuses the song on the top line with the file's detail; with a song open the song and the player's path stay; a good file opens it.
2 Owner: `app::kSettingsRefusedSentence`, `PbApp::open`.
3 Real chart: claim not checked (a capture of the top line); the setup needs Burnout at `--efill 60`.
4 Direct: movable: kFirstFillTick (487). Not movable: 523-527, 533-536, 495-501.
5 Reason: the "volume line before the bad one" comment explains its input.
6 Display: Partly.
7 Failure: a bad file accepted. Still possible.
8 Bug at a line: line 495 `chart() == open_song` goes red if a refused open replaced the chart.
9 Stand-in: substring.
10 Same rule: bad-rules-refused.
11 Real: yes.
Verdict: Investigate: claim not checked only; and the setup click must take its fill from the record.

### 14. bad-rules-refused (`pbtest_window.cpp:546`)
Delta round: window, replaces records/A49.md record 8
The old verdict was Investigate (claim not checked only). It still holds, for the same setup reason as record 13 (`check_open_song_stays`, called at line 575).
1 Claim: A bad rules file refuses like a bad settings file; with both bad, the settings error shows.
2 Owner: `app::kRulesRefusedSentence`.
3 Real chart: claim not checked: a capture; the setup needs the 60 ms Burnout dump.
4 Direct: movable: kFirstFillTick through line 487. Not movable: the rest.
5 Reason: input comment 551-552.
6 Display: Partly.
7 Failure: as record 13.
8 Bug at a line: line 584 `!on_screen(kRulesRefusedSentence)` goes red if both files bad showed the rules error first.
9 Stand-in: substring.
10 Same rule: bad-settings-refused.
11 Real: yes.
Verdict: Investigate: claim not checked only.

### 15. rules-reach-the-builder (`pbtest_window.cpp:592`)
Delta round: window, replaces records/A49.md record 9
The old verdict was Rewrite (A49 says "do not type 381,075"). It still holds.
1 Claim: A `backend_leeway_ms = 500` rules file changes what the builder computes, and a later open with the file gone reads the default.
2 Owner: `core::default_rules`, `Settings::rules`; the score is the engine's best possible.
3 Real chart: claim not checked: Burnout at `--efill 60` under both leeways.
4 Direct: movable: 603 "381,075" (best possible with leeway 500), 604 and 613 "378,315" (best possible at the default). Both are best-possible scores from a search run in which `is_e0` and `fill_refuses` take part. Not movable: 602 and 611 (the setting values).
5 Reason: "The figures are from one run".
6 Display: Fails.
7 Failure: the rules file not reaching the search. Still possible.
8 Bug at a line: line 603 goes red if the 500 ms leeway did not change the best possible score.
9 Stand-in: `on_screen` substring "378,315" matches any line that holds that text.
10 Same rule: build test 147-148 types "378,315" too.
11 Real: yes.
Verdict: Rewrite. Read both scores from the record under each rule and compare them.

## Typed numbers found that a run at 60 ms can move, by line

- `pbtest_harness.h:60-61`: `kFirstFillTick` 36480, `kSecondFillTick` 74880.
- `pbtest_build.cpp`: 63 `kLongSongFillTick` 133920; 147-148, 162, 164 "378,315"; 155 count 1; 156 row (m32, 163.0 ms); 167, 193, 297 "378,175"; 168 and 177 "−140"; 169 "m24"; 192 "327,135"; 200 and 214 "−1,580"; 228 "340,675"; 229 "376,595"; 292-293 rows; 349 "m15 (1,150,660)"; 354 and 364 "944,680"; 355 and 362 "−205,980"; 356 "m90"; 363 "879,860".
- `pbtest_coach.cpp`: 89-90 "9" and 349,875; 93 and 151 "−13,640"; 101-102 "9 1" and 364,675; 106; 174 five windows; 182-183 "130.4 ms" and "m135"; 228-232 ticks 26880, 52992, 74496, 59136, 57600; 240-244, 252-255 (336,575, 330,075, −2,680); 522, 527, 529-530, 534-535.
- `pbtest_window.cpp`: 76-78 the pending row; 178 36480; 203 the row; 390 162240; 399 the readout; 603 "381,075"; 604 and 613 "378,315".
- Typed numbers that cannot move through the window: "313,635" and "729,960" (no activation), 21120, 83040 and 59520 (hidden or locked fills), clocks and readouts set by chart length, the 962-note count.

## Not reached, and stops

Every test case of the six files and the registrar was read whole. Not read: the bodies of `pbtest_harness.cpp` (WN5's), `src/pathbuilder/pb_coach.cpp` beyond signatures, and the code that sets `BatchJob::Snapshot::analyzed` (the batch-chip-count flag). The two flag rows (settings-and-reports, batch-chip-count) are B by reading, with the reason stated. A verifier should settle them. Outside this group but noticed: `tests/ui/uitest_library.cpp:370,378,383` type 85 for the early-fill window setting (the settings box's upper limit, lines 374-378); that is WN5's file.
