# window-WN5: the GUI test files, window round (auditor a054229880912048c, T5-WN5)

Group WN5: `tests/ui/uitest_details.cpp`, `uitest_engine.cpp`, `uitest_engine.h`, `uitest_preview.cpp`, `uitest_paths.cpp`, `uitest_harness.cpp`, `uitest_harness.h`, `uitest_live.cpp`, `uitest_main.cpp`, `uitest_library.cpp`, `uitest_commands.cpp`, `uitest_report_windows.cpp`, `uitest_script.cpp`, and `tests/pb_ui/pbtest_harness.cpp`, `pbtest_harness.h`, `pbtest_main.cpp`. Read at branch HEAD `3b28d869` (code of `5ec682c7`).

## Which window the search gets in these files

No file in the group sets the early-fill window to a named value for a search. I grepped `tests/ui` and `tests/pb_ui` for `efill`, `early_fill`, `EarlyFill` and `SearchSettings`. The only hits are in `uitest_library.cpp` (lines 316 to 383) and they type the number into the settings box, which is a screen test of the box, not a search setting (those are delta-DL5 records 22 and 23).

How the search is reached:

- The GUI harness writes its scratch ini from `scratch_settings()` (`tests/scratch_settings.h`, called at `tests/ui/uitest_harness.cpp:249` and `tests/pb_ui/pbtest_harness.cpp:101`). That struct is `app::Settings` with only the chart folder, `auto_open_report` and `depth_value = 2` changed. `Settings::early_fill_window_ms` defaults to `kDefaultEarlyFillWindowMs` (`src/app/config.h:195`), which is 60 (`src/core/model.h:112`). So every analysis these tests start through the app runs at 60 ms.
- The click's analysis, the batch and the Preview all take their settings from the app (`BatchGate` and `ViewGate`, `uitest_harness.cpp:316-362`, call `analyze_chart_file(path, settings, ...)` with the settings the app gave them). The window reaches the search as `SearchSettings::early_fill_window_ms` (`src/search/pather.cpp:265`) and is stamped on each activation (`src/search/engine.cpp:2030`, `act.e_window_ms`). The paths screen's "(required)" or "(optional)" on an early fill is `act.is_E0()` (`src/app/path_view.cpp:280`, `src/core/model.cpp:475`), which is `is_e0(e_offset, skips, e_window_ms)`. So that word reads the window directly.
- The pathbuilder harness (`PbApp`) opens charts on the same scratch ini and its `click_fill` passes `s.chart()->settings` to the search (`pbtest_harness.cpp:119`). Same default, 60.

So an A row does not occur in this group. Every test that reaches the search is B or C.

No engine record at 60 ms exists for any chart these tests use. `records-engine/` holds `beg`, `believer`, `flammarker_ingame`, `folded_sqin`, `late_sqin_twice`, `path_reduction_test0` and `pitch_dark_ratio`, dumped at 85 (`326cd4e0`). Burnout, Tapped Out and Tapestry, the charts the GUI tests pin, have no record at all. So every C record below has question 3 as "claim not checked" and names the dump it needs.

The three dumps that would close question 3 for almost every C row (`hydra_replay dump` and `schedule` at `--efill 60`, then at `--efill 85` for the comparison):
- Burnout: `testdata/input/common/Summer Blast _25 Setlist/Tier 4/Green Day - Burnout/` (its notes file).
- Tapped Out: `testdata/input/common/IB24/T5/Mike Orlando - Tapped Out/`.
- Tapestry of the Starless Abstract (Shortened): `testdata/input/common/IB24/T7/Ne Obliviscaris - Tapestry of the Starless Abstract (Shortened)/`.
- Chart 0 of the scratch library for the two chart-0 rows (the first row of `scan_library`'s order; its name needs the main session to read it from the screen).

## Typed numbers that can move (findings, one line each)

- `tests/pb_ui/pbtest_harness.h:60-61`: `kFirstFillTick = 36480` and `kSecondFillTick = 74880`. The comment says "Burnout's first two fills the player can activate on (m20 and m40), by chord tick, from one run under the GUI tests' settings". Which fills the player can activate on is what `fill_refuses` decides. A fill whose SP became ready between 60 and 85 ms late was allowed under the old rule and is refused at 60. Used by `pbtest_build.cpp:163,178` (outside this group).
- `tests/ui/uitest_paths.cpp:119-128`: `378,315`, `378,175`, `375,955` and `2,360 below optimal`, plus the best path `"3- 1 2"` (line 102, 122), `"0 4 1"` (126, 140, 152) and `"0 0 0 0"` (127, 145): Burnout's three score folds and paths. Any path set that needs a late fill moves them.
- `tests/ui/uitest_details.cpp:1032, 1057, 1066`: `378,315` and `Optimal path · 7 stars` (the headline).
- `tests/ui/uitest_paths.cpp:264, 273, 293, 123`: `163.0 ms` squeeze out and the sentence "more than 163.0 ms late"; `338-339`: Beg's `squeeze out 0.0 ms` and `early fill 0.0 ms`; `344`: `Insane+ (eff. 163.5 ms) <-- squeezed out (-260)`; `393-404`: the `-489.1`, `-326.1`, `-163.0` backend rows.
- `tests/ui/uitest_paths.cpp:941-942, 975-1010`: Tapped Out and Burnout hit-window pieces (`frontend 84.5 ms`, `backend 84.3 ms`, `5.4 ms safe`, `85.7 ms`, `82.9 ms`, `77.9`, `72.0`, `65.9 ms short`) and `Early fill: 0.0 ms (optional)` on activation 4 and `Early fill: 0.0 ms (required)` on activation 3. The words in brackets are `is_E0()` and so read the window (`path_view.cpp:280`).
- `tests/ui/uitest_preview.cpp:679, 700, 754, 763, 781-783`: `"3- 1 2"`, `"3- 1 2  (optimal)"`, `"0 0 0 0  (best all-0)"`, `"0 4 1"`, `Next: activation 1 of 3`, `3- (163.0 ms)`, `m32.1.0`.
- `tests/ui/uitest_paths.cpp:925, 935`: Burnout's song length `130087.87999999993` is parse output (`analysis_song_length`, D255), not window-dependent. Listed because the same test starts from `open_burnout`, whose line 102 pins `"3- 1 2"`.
- `tests/ui/uitest_details.cpp:504`: `"4.4"` on the Stars tab has no comment. It sits beside `star_cutoffs`, which reads one path's chart base score; not read further. Listed as not read.

## Row tables

Legend: B reason names the code fact. "delta round" means the test case already has a record in `records/delta-DL5.md` and is not judged here. "helper" rows are not test cases; they say what the helper does for the window.

### tests/ui/uitest_details.cpp (26 test cases: C 1, B 24, delta 1)

| Test | Kind | Reason or record |
|---|---|---|
| test_analyze (106) | B | Every check reads the record the click produced (`viewed.record->best_path().pathstring()` against the screen) or a store row status or the ini cap; no typed path or score. The window cannot make the screen differ from the record it was drawn from. |
| test_cap_switch (152) | B | Same: per-cap best path read from the record at each cap, plus the gauge suffix `/5`, which is the Settings cap (`settings.sp_cap`), not a search output. |
| test_legacy_fills (198) | B | Asserts the record's `legacy_fills` flag, row counts, and that unticking returns `best11`, the string read from the first run in the same test (both runs at the same window). No typed path. |
| test_dynamics (249) | B | The counts (`37 of 455`, ghost counts) come from `app::dynamics_for_settings(path, ChartReading, song)` (`src/ui/library_jobs.cpp:731`, `src/app/dynamics_breakdown.h:198`), which takes the chart reading and the song, not `SearchSettings`. The search window is not an input. |
| test_dynamics_reopen (346) | B | Same function; the assertions are the count job's output and "no second job". |
| test_dynamics_hard (421) | B | Same function at Hard. |
| test_stars (467) | B | Expected side is `star_cutoffs(record->best_path())` (`src/core/stars.h:42`) compared with the screen, plus a typed `"4.4"` that is not read (see above). `solo_bonus > 0` and `== 0` are chart facts (a drum solo on "87", none on Believer), read from the same record. |
| test_details_close_teardown (539) | B | Asserts the panel is torn down (window and state), not any analysis value. |
| test_panel_follows_rescan (566) | B | Asserts the header text after a `song.ini` change; the header is parse output (title, artist, charter). |
| test_preview_load_bar (617) | B | Asserts the Preview load label and fraction (`Reading chart` and the other three names). Load steps are the Preview's, not the search's. |
| test_panel_open_close (655) | B | Panel open and close state only. |
| test_panel_split (686) | B | Split ratio and layout from an ini (line 738 `LibraryShare=0.3000`). |
| test_panel_hide_library (750) | B | Window layout only. |
| test_open_in_path_builder_greyed (849) | B | Asserts the button is greyed while the click's analysis is held by `ViewGate` (`no_button`-style check); no search output. |
| test_open_in_path_builder_enabled (853) | B | Asserts the button is enabled once `wait_song_analyzed` returns; the state is "record ready", not what the record holds. |
| test_layout_sweep (879) | B | Layout sweep of window sizes and the chip row; asserts positions and fit. |
| test_long_error_wraps (957) | B | A forced failure (a long path in an error); no search result is asserted. |
| test_panel_prev_next (999) | B | Moves between library rows; asserts the selected title. |
| test_panel_headline (1019) | C | Record below. |
| test_panel_close_cancels (1079) | B | Asserts the job is cancelled and the row stays Not analyzed; no search output. |
| test_view_cancel_try_again (1099) | B | Asserts Cancel then Try again on the same job; states, not values. |
| test_view_rules_broken (1134) | B | The rules file makes analysis refuse (`max_tied_paths = 0`, "analysis is off"); the search never runs. |
| test_no_analyze_button (1167) | B | Asserts no Analyze button text in each state. |
| test_view_progress_delay (1203) | B | Timing of the progress box (0.15 s delay). |
| test_view_setting_burst (1256) | B | Asserts which caps were saved (`record_count`, row statuses) and that the screen shows the last record's `best` read from the record. |
| test_settings_lock (1289) | delta round | `records/delta-DL5.md` record 21. |

### tests/ui/uitest_preview.cpp (16 test cases: C 5, B 11)

| Test | Kind | Reason or record |
|---|---|---|
| test_preview (34) | B | Preview loading and Play/Pause on chart 0; the load-progress numbers are typed `Progress` objects (no search). |
| test_analyze_on_preview (90) | B | Asserts the record has paths, the row is Ready, and the screen holds `record->best_path().pathstring()` read from it. |
| test_preview_path_overlay (151) | C | Record below. |
| test_preview_controls (228) | B | Expected side is the record: `end.score == group_thousands(all_paths().front()->totalscore())` and `end.stars == path_stars(*shown)` (lines 254-259). Rest is time and box layout. Lines 260-376 were read by assertion grep only. |
| test_preview_solo_box (377) | B | Levitating's chart has no Star Power phrase (comment at line 50: "The chart has no Star Power phrase"), so the path has no activation and no early fill the search could use; the asserts are the solo box (chart events) and the gauge reading `0.0/` (the gauge is empty because the chart has no SP phrase). |
| test_preview_drain_box (471) | C | Record below. |
| test_preview_overlay_fit (534) | B | Layout of the five boxes (`check_overlay_boxes` returns a count of boxes drawn). |
| test_preview_buttons_keys (564) | B | Key and button handling on chart 0 (playhead moves, play and pause). No score or path value. |
| test_scrub_hold (619) | B | Scrubber hold; positions only. |
| test_layout_drift (647) | B | Box positions; no analysis values. |
| test_preview_path_picker (694) | C | Record below. |
| test_preview_error_wraps (788) | B | A forced error sentence; no search output. |
| test_preview_mode_reload (849) | B | Difficulty and Pro Drums changes reload the notes; asserts `no_notes_message` and playhead position. |
| test_settings_change_holds_layout (922) | B | Compares the screen's score with `record->best_path().totalscore()` before and after (`score0`, `score1` read from the record), and tab and scrub positions. |
| test_preview_overlay_steady (986) | C | Record below. |
| test_preview_activation_jumps (1025) | C | Record below. |

### tests/ui/uitest_paths.cpp (12 test cases: C 11, B 1)

| Test | Kind | Reason or record |
|---|---|---|
| test_paths_list (109) | C | Record below. |
| test_paths_rows (254) | C | Record below. |
| test_paths_backend_timings (348) | C | Record below. |
| test_paths_folds_copy (424) | C | Record below. |
| test_paths_uncounted (488) | C | Record below. |
| test_paths_fit_narrow (513) | C | Record below (helper precondition only). |
| test_paths_long_path (540) | B | Opens The Spiraling Void and asserts only window widths against `kMinPathListW` and `kMaxPathListShare` (lines 561-566). It pins no path, count or text; the comment names "9 activations" but nothing asserts it. A different path leaves the widths rules unchanged. |
| test_paths_backend_fit (604) | C | Record below (helper precondition and typed `kBurnoutSqOutRating`). |
| test_paths_row_layout (734) | C | Record below (helper precondition only). |
| test_paths_length_from_click (920) | C | Record below (helper precondition only). |
| test_paths_hit_window (966) | C | Record below. |
| test_paths_hit_window_narrow (1015) | C | Record below. |

### tests/ui/uitest_library.cpp (31 test cases: B 29, delta 2)

All 29 B rows: the file's analysis-reaching tests (`test_difficulty` 131, `test_analyze_follows_chip` 706, `test_library_widths_hold` 1015, `test_view_settings` 485, `test_library_state_per_app` 447, `test_rescan_on_open` 1607, `test_library_cant_analyze` 755, `test_library_twin_rows` 80) assert row statuses (`RecordStatus::Ready`, `NotAnalyzed`, `CantAnalyze`), chip counts (`counts().analyzed`, `not_analyzed < total`), widths and `viewed.record->best_path().pathstring()` read back from the same record (line 184). The status or count comes from whether a record was stored, which a window cannot change: every chart in the corpus that analyzes at 85 analyzes at 60 (the window changes which paths are refused, not whether the search returns a record). Nothing pins a path, score or activation.

| Test | Kind |
|---|---|
| test_scan (62), test_library_twin_rows (80), test_difficulty (131), test_note_shuffle_switch (203), test_settings_button (260), test_engine_wildcard_patches (281), test_rules_error (389), test_settings_error (419), test_library_state_per_app (447), test_view_settings (485), test_library_search (541), test_analyze_follows_chip (706), test_library_cant_analyze (755), test_library_sort_scroll (805), test_library_layout (863), test_library_column_widths (939), test_library_widths_hold (1015), test_library_click_keeps_row_in_place (1125), test_library_next_scrolls_just_enough (1189), test_library_column_order (1253), test_library_columns_at_start (1293), test_startup_screen (1413), test_startup_library_read_bar (1469), test_old_library_set_aside (1539), test_rescan_on_open (1607), test_rescan_on_open_cancel (1637), test_scan_missing_folder (1669), test_version_bar (1801), test_library_search_tips (2025) | B (29 names) |
| test_settings_reset (298) | delta round (`delta-DL5.md` record 22) |
| test_settings_early_fill (342) | delta round (`delta-DL5.md` record 23) |

### tests/ui/uitest_live.cpp (16 test cases: B 15, delta 1)

Every case runs a command (`open`, `click`, `wait`, `reset`) and asserts the reply text, the selected title or the live-mode refusal. The open starts a click's analysis, but no assertion reads its result (checked by grepping the file for `viewed`, `best_path`, `record`: no hit in these tests). B reason: the asserted values are command replies and selection state.

test_command_open (376), test_command_open_unknown (388), test_command_row_click_refused (401), test_command_row_click_after_search (416), test_library_scroll_guard_absolute_ref (434), test_library_scroll_guard_keys (472), test_command_bare_ref_retried (512), test_command_bare_ref_missing (522), test_command_modal_named (558), test_command_wait_text_reply (607), test_live_reset_draws_while_opening (638), test_live_reset_open_outlasts_timeout (653), test_live_reset_open_failed (686), test_reset_app_blocks (721), test_live_start_gives_up_at_cap (733): B.
test_command_bare_ref_ambiguous (537): delta round (`delta-DL5.md` record 24).

### tests/ui/uitest_report_windows.cpp (24 test cases: B 24)

B reason, from the file: the report-window tests build rows from a sample report (`sample row 4, 250,000`, `77,000`; lines 258-272, 712) or run a real batch of the corpus behind a `BatchGate` and assert counts, states and layout (`path_window() != nullptr`, `path_report_awaits_confirm()`, `page.songs + page.cant_analyze_charts == page.library_charts` at line 1160, the "n of M paths" count read back with `shown_count`). The typed numbers I found by grep (`250,000`, `77,000`, `3,456`, `+1,000 over`) are sample-row data the test supplies, not search output. I did not read lines 1051 to 1105, 1171 to 1256 and 1345 to 1620 in full; the reason rests on the typed-number grep (no chart-derived number or path string in the file) and the test headers. A verifier should read those bodies.

test_sort (248), test_filters (276), test_states (306), test_keys (478), test_close_button (523), test_hidden_new_result (574), test_both (604), test_chip_label (621), test_chip_outline (659), test_column_widths (673), test_cells_left (713), test_header_narrow (752), test_fits_screen (802), test_open_asks (991), test_open_path (1051), test_open_analyzes (1112), test_cant_analyze_list (1171), test_dm_handover (1206), test_row_click (1257), test_reopen (1345), test_hidden_lets_go (1373), test_reopen_keeps_filters (1419), test_fresh_state (1498), test_out_of_date (1548): B.

### Helper and non-case files (no test case; what each does for the window)

| File | Kind | What it does |
|---|---|---|
| tests/ui/uitest_harness.cpp, uitest_harness.h | helper, default window | `reset_app` writes the ini from `scratch_settings()` (line 249); `BatchGate` and `ViewGate` call `analyze_chart_file` with the app's settings. Window = the app default, 60. Registers no test. |
| tests/ui/uitest_engine.cpp, uitest_engine.h | helper, no search | Capture, temp folder, ImGui test engine start, result printing (`E1`-`E6` of A40). No analysis call. |
| tests/ui/uitest_main.cpp | helper, no search | Argument parsing and run loop; passes `--db` and `--ini` seeds. |
| tests/ui/uitest_commands.cpp | helper, no search | `run_command` verbs and refusals; no test case. |
| tests/ui/uitest_script.cpp | helper | `register_script_test` runs a command file line by line through `run_command`. Whatever the script opens runs at the app's setting (60 unless the script's ini sets it); no case. |
| tests/pb_ui/pbtest_harness.cpp, pbtest_harness.h | helper, default window; typed ticks | `reset_app` uses `scratch_settings()`; `click_fill` runs the search on `chart()->settings`. `kFirstFillTick` and `kSecondFillTick` are typed from one run (finding above). |
| tests/pb_ui/pbtest_main.cpp | helper, no search | Run loop. |

## C records

Reading common to every record below: the whole test body and its helpers (`open_burnout`, `open_burnout_preview`, `open_titled`, `wait_song_analyzed`), `src/app/path_view.cpp:235-290` (the early fill row and its "(required)" word), `src/core/model.cpp:473-475` (`is_e_critical`, `is_E0`), `src/core/model.h:325-336` (`fill_refuses`, `is_e0`), `tests/scratch_settings.h`, the old records A39, A44 and A45 (verdict lines and headings), `records-engine/index.md` (only to see which charts have a record), the code of `5ec682c7` at HEAD. Adding commits: not read for any test. The decision log and the rulings were not needed to judge a window question; the ruling lines the brief points at (`docs/agents/rulings/code-and-engine.md` 55 to 57) are not quoted here because none of the tests above is about the setting's range except the two delta rows.

The window can move a typed Burnout number in three ways, each visible in code: (1) `fill_refuses(e_offset, window_ms)` now refuses a fill whose SP became ready more than 60 ms late (it was 85), so a path that used such a fill is gone and its score and string with it; (2) `is_E0` now reads `e_offset < 60` (it read `< 85`), so "(required)" and "(optional)" can swap and an E mark can appear or vanish; (3) a path that was tied or ranked behind the best may be gone, so "Within 2 scores" lists and "n paths" counts move. A Burnout dump at 60 and at 85 settles all three at once.

---

### 1. test_panel_headline (tests/ui/uitest_details.cpp:1019)

Delta round: window, replaces A39.md record "test_panel_headline" (verdict Rewrite: the headline shows the best path's total, string, stars and hardest timing as `viewed.record` names them). At 60 ms the old verdict still holds, and a second fault joins it: the test types `378,315`, `3- 1 2`, `Optimal path · 7 stars` and `163.0 ms` (lines 1032, 1057, 1065-1070) from Burnout at 85.
Read: lines 1019-1078; common reading.
1 Claim: after the click lands, the song panel header shows Burnout's best score `378,315`, its path `3- 1 2` and `Optimal path · 7 stars` with nothing after the stars, no "hardest squeeze" word, and `163.0 ms` on screen.
2 Owner: the best path is the engine's record. `path_stars` and the headline are the display's. The 163.0 ms is `build_activations`' badge, an engine value carried out (`path_view.cpp`).
3 Real chart: claim not checked. Needs `hydra_replay dump` and `schedule` on Burnout at `--efill 60`: the best path string, total and first activation's squeeze-out ms, beside the typed four values. No Burnout record exists at either window.
4 Direct: four typed strings read off one run (the test's own comment, line 1062: "read off one run of this test"). Raw results.
5 Reason: none beyond the run at 85.
6 Display: typed expected side, not read from the record.
7 Failure: the headline stuck on a previous song's score while the analysis lands (`wait_until` at 1057). Still possible.
8 Deleted: 1057 (`378,315` shown): headline not drawn; 1066-1067: a stray fact after the stars (reasoned).
9 Stand-in: the typed score stands for "the headline shows the best path's total".
10 Same rule: `uitest_paths.cpp` test_paths_list lines 119 to 128 type the same Burnout score.
11 Real: yes, Burnout.
Verdict: Rewrite, as A39. New: the typed numbers are a snapshot at the old window; take each from `viewed.record` at run time.
Finding: split: three tests (this one, test_paths_list, `open_burnout`) each pin Burnout's `3- 1 2` and `378,315` separately, with no one owner of "Burnout's optimal path".

### 2. test_preview_path_overlay (tests/ui/uitest_preview.cpp:151)

Delta round: window, replaces A45.md record "test_preview_path_overlay" (Investigate, claim not checked only: a capture of the overlay on chart 0 for each path). Old verdict still holds. New: line 171 asserts `other != nullptr` ("the fixture must keep 2+ distinguishable paths"): chart 0's search must return at least two paths with unique pathstrings. At 60 ms the refused fills can drop a path.
Read: lines 151-223; common reading.
1 Claim: picking another path in the Preview's list swaps the overlay in place, keeps the playhead and no reload, and picking the first again restores its overlay.
2 Owner: the overlay key and the path list are the app's; the paths are the engine's record.
3 Real chart: claim not checked. Needs chart 0's dump at `--efill 60`: the number of paths with a unique pathstring beyond the first. Which chart is chart 0 needs the main session to read the first library row.
4 Direct: comparisons of keys and labels read from the record; `first_rows == 1` etc.
5 Reason: "2+ distinguishable paths" is the test's input need with no number.
6 Display: expected side read from the record. Good.
7 Failure: overlay stuck on the first path. Still possible.
8 Deleted: 171 `other != nullptr`: the test would later dereference null (a fixture guard; fails only if the search returns one path). 209, 211: overlay not swapped (reasoned).
9 Stand-in: none.
10 Same rule: test_preview_path_picker (Burnout, record 4).
11 Real: yes.
Verdict: Investigate, claim not checked only (a dump of chart 0 at 60: at least two unique path labels, and a capture of the overlay for each).

### 3. test_preview_drain_box (tests/ui/uitest_preview.cpp:471)

Delta round: window, replaces A45.md record "test_preview_drain_box" (Rewrite: the box is active at each activation's start and idle after, with the moments from the engine record). Old verdict holds. New: the loop at lines 508 to 514 walks the playhead until the drain box reports `active`, and line 516 `IM_CHECK(active_ms >= 0.0)` requires chart 0's best path to have at least one activation. At 60 ms a path with none is not likely but is not shown impossible.
Read: lines 471-533 (assertion lines and comments; the middle of the body was trimmed in my reading); common reading.
1 Claim: before any activation the drain box is idle with its header and "1 bar / ..." rate; somewhere along the path it turns active; all five boxes draw.
2 Owner: `build_drain_box` (pinned by unit tests per its own comment); the activations are the engine's.
3 Real chart: claim not checked: chart 0's schedule at `--efill 60` (first activation start tick).
4 Direct: header string is a rule label; `rfind("1 bar / ")` a prefix.
5 Reason: no numbers beyond 50 ms steps (an input).
6 Display: the moments are searched at run time, not typed. Fine in shape.
7 Failure: drain box not fed the playhead.
8 Deleted: 516: a path with no activation would leave `active_ms = -1` and the test would fail the later seek (reasoned).
9 Stand-in: the 50 ms scan stands for "the path's activations".
10 Same rule: test_preview_solo_box (no SP phrase, B).
11 Real: yes.
Verdict: Rewrite, as A45 (take the moments from the record). Window effect: claim not checked only for the "chart 0 has an activation" fact.

### 4. test_preview_path_picker (tests/ui/uitest_preview.cpp:694)

Delta round: window, replaces A45.md record "test_preview_path_picker" (Rewrite: expected labels, activation count, notation and tick from Burnout's record; drop the width copy at 717). Old verdict holds; the typed side is now also a 60 ms question.
Read: lines 664-784; common reading.
1 Claim: the Showing list names `3- 1 2  (optimal)`, lists `0 0 0 0  (best all-0)`; picking the second path selects `0 4 1`; after a jump the next-activation box reads `Next: activation 1 of 3`, `3- (163.0 ms)` and `at m32.1.0`.
2 Owner: `build_path_buttons`, `preview_path_label` and the record own the labels. The test types them.
3 Real chart: claim not checked: Burnout at `--efill 60`: path 0 string, the second path in `all_paths()[1]`, its string, activation count and first activation's squeeze-out ms and measure.
4 Direct: typed strings (lines 679, 700, 754, 763, 768, 781-783). Raw results.
5 Reason: from one run at 85.
6 Display: fails; typed expected side.
7 Failure: picker labels or jump landing wrong. Possible.
8 Deleted: 700: label format drift; 763: the pick selecting the wrong path (reasoned).
9 Stand-in: the second path standing as `all_paths()[1]`; a different path set changes who is second.
10 Same rule: `uitest_paths.cpp` test_paths_list (`0 4 1`).
11 Real: yes.
Verdict: Rewrite, as A45; new: the typed side is a snapshot at the old window.

### 5. test_preview_overlay_steady (tests/ui/uitest_preview.cpp:986)

Delta round: window, replaces A45.md record "test_preview_overlay_steady" (Rewrite: loop over the record's activations, not "3"; no measure strings typed). Old verdict holds. New: `open_burnout_preview` pins `best_path().pathstring() == "3- 1 2"` (line 679) and `scrub_marks().size() == 3` (line 684): three activations.
Read: lines 986-1024 by assertion grep and the helper at 670-688; common reading.
1 Claim: the text boxes keep one size through the path; no box shrinks on Burnout's long second chord.
2 Owner: the Preview's overlay scale (`overlay_scale`).
3 Real chart: claim not checked: Burnout at 60; the second chord's length and the number of activations (three).
4 Direct: scale equality along the path.
5 Reason: the helper's `"3- 1 2"` and `3` are the one run's values.
6 Display: fine.
7 Failure: scale jump on a long chord. Possible.
8 Deleted: helper line 679 and 684: the setup fails if the search gives another path or count, hiding the scale check (reasoned).
9 Stand-in: "second chord long" stands for a chord in Burnout.
10 Same rule: none.
11 Real: yes.
Verdict: Rewrite, as A45; the setup precondition is the window-sensitive part.

### 6. test_preview_activation_jumps (tests/ui/uitest_preview.cpp:1025)

Delta round: window, replaces A45.md record "test_preview_activation_jumps" (Rewrite: marks equal the record's activation ticks as fractions of the song's length; jumps land on each chord's tick; no m-strings typed). Old verdict holds. New: `open_burnout_preview` helper pins three marks and `"3- 1 2"`; the file types `m99.4.352` and `/4`.
Read: lines 1025-1085 by assertion grep; common reading.
1 Claim: the scrubber has one mark per activation, `[` and `]` jump between them, the SP readout ends `/4`, the end time reads `m99.4.352`.
2 Owner: the record's activations and `song_length_ms` (D255).
3 Real chart: claim not checked: Burnout at 60: the activation ticks.
4 Direct: typed `m99.4.352` is a parse fact (end event tick 188280, comment lines 1071-1072); the marks depend on the search.
5 Reason: D255 for the end; the three marks are one run.
6 Display: marks typed as count 3.
7 Failure: jumps skipping a mark.
8 Deleted: helper 684: marks size.
9 Stand-in: 3 for "all activations".
10 Same rule: test_paths_length_from_click.
11 Real: yes.
Verdict: Rewrite, as A45; the count 3 comes from the search.

### 7. test_paths_list (tests/ui/uitest_paths.cpp:109)

Delta round: window, replaces A44.md record "test_paths_list" (Rewrite: the list shows each path the record keeps within the score range, in the record's order). Old verdict holds; at 60 the typed numbers are a second reason.
Read: lines 92-156; common reading.
1 Claim: Burnout shows an optimal path `3- 1 2` at `378,315`, a within-2-scores fold at `378,175` with `0 4 1` and `2 1 2`, and an all-0 path `0 0 0 0` at `375,955`, `2,360 below optimal`.
2 Owner: the record's paths, `build_path_buttons`; the test types them.
3 Real chart: claim not checked: Burnout dump at 60 against the typed list.
4 Direct: typed strings (119-128, 140, 145, 152). Raw results.
5 Reason: one run.
6 Display: typed.
7 Failure: heading, fold and selection wrong.
8 Deleted: 127: the all-0 line dropped.
9 Stand-in: typed strings for "the record's list".
10 Same rule: panel_headline, path_picker.
11 Real: yes.
Verdict: Rewrite, as A44.

### 8. test_paths_rows (tests/ui/uitest_paths.cpp:254)

Delta round: window, replaces A44.md record "test_paths_rows" (Rewrite: each mark's outline orange exactly when the record's badge is a warning badge; typed Burnout text goes). Old holds. Window-sensitive lines: `3 · no SP left over`, measures `m32.1.0`, `m58.1.0`, `m88.1.0`, `squeeze out 163.0 ms`, `3`, `6` and `7 notes near the SP end`, the outlines `{Orange, None, None}` for Burnout (the orange is a warning badge) and `{Grey, None, None, Grey}` for Beg with `early fill 0.0 ms` on row 4. For Beg: the engine record exists, but at 85 (`records-engine/beg.dump.json`).
Read: lines 254-340; `check_marks`, `drawn_marks` (158-244); common reading.
1 Claim: Burnout's three activations show with their typed badges; marks outline orange or grey as typed; Beg's row 4 is an early fill of 0.0 ms.
2 Owner: `build_activations` and its `warn` flag; the engine's activation.
3 Real chart: Beg at 85 is in `records-engine/beg.dump.json`; claim at 60 not checked: Beg and Burnout at `--efill 60`. An early fill of 0.0 ms is not refused at 60 or 85 (refusal needs e_offset below -60), so Beg's row 4 is likely stable; Burnout's is not known.
4 Direct: typed text and outlines.
5 Reason: one run.
6 Display: outlines typed per row.
7 Failure: mark outline wrong.
8 Deleted: 284: outline colour wrong (reasoned).
9 Stand-in: the outlines list stands for "each warning badge".
10 Same rule: A44 record.
11 Real: yes.
Verdict: Rewrite, as A44. Beg part: Investigate, claim not checked only (Beg dump at 60 against the early fill 0.0 ms).

### 9. test_paths_backend_timings (tests/ui/uitest_paths.cpp:348)

Delta round: window, replaces A44.md record "test_paths_backend_timings" (Rewrite: hidden count and rows kept from the record's backend rows at the limit). Old holds. Typed: `kBurnoutSqOutRating` (`Insane+ (eff. 163.5 ms) <-- squeezed out (-260)`), the rows `-489.1`, `-326.1`, `-163.0`, `3 notes near the SP end · 2 hidden: 2 before, 0 after`. The rows are the squeeze-out neighbourhood of Burnout's first activation: a different path or chord mix changes them.
Read: lines 342-421; common reading.
1 Claim: row 1's backend table lists three notes near the SP end; at limit 30 two are hidden; the squeezed-out row stays.
2 Owner: the record's backend rows.
3 Real chart: claim not checked: Burnout at 60, first activation's backend rows.
4 Direct: typed rating line and offsets. Raw.
5 Reason: one run.
6 Display: typed.
7 Failure: the limit hiding the wrong rows.
8 Deleted: 401-404.
9 Stand-in: `2 hidden` for "rows beyond the limit".
10 Same rule: kBurnoutSqOutRating is also at lines 357, 361, 365, 374, 381 and in test_paths_backend_fit.
11 Real: yes.
Verdict: Rewrite, as A44.

### 10. test_paths_folds_copy (tests/ui/uitest_paths.cpp:424)

Delta round: window, replaces A44.md record "test_paths_folds_copy" (Rewrite: replace the three typed Burnout strings with the record's multiplier-squeeze entries). Old holds. Typed: `+15`, `Hit [Red snare] first.`, `2x   (+15 pts):   [Red snare - Yellow cymbal]`; line 479 requires Tapestry's best path is not `3- 1 2`. The multiplier squeeze lines come from the best path's multiplier squeezes; a different best path can change them.
Read: lines 424-483; common reading.
1 Claim: both folds start open, close and reopen; Copy path copies the best path's verbose string; folds keep the last choice on the next song.
2 Owner: `pathstring_verbose(rec.multsqueezes)` (record).
3 Real chart: claim not checked: Burnout multiplier squeeze at 60.
4 Direct: typed text; clipboard is compared with the record's own string (good).
5 Reason: one run.
6 Display: partly typed.
7 Failure: fold state lost.
8 Deleted: 432-434.
9 Stand-in: none.
10 Same rule: A44.
11 Real: yes.
Verdict: Rewrite, as A44.

### 11. test_paths_uncounted (tests/ui/uitest_paths.cpp:488)

Delta round: window, replaces A44.md record "test_paths_uncounted" (Rewrite: the table row says "(uncounted)" iff the record's `counted_without_squeeze` is false). Old holds. New and sharper: the comment at lines 485-487 says "The chart was found by searching the corpus under the GUI tests' settings for such a row". The search that found Tapestry ran at 85 ms; the test needs an uncounted squeezed-out row at 60.
Read: lines 485-506; common reading.
1 Claim: Tapestry (Shortened) shows a squeezed-out row the engine never counted: "(uncounted) <-- squeezed out" and "It costs no points".
2 Owner: the engine's `counted_without_squeeze`.
3 Real chart: claim not checked: Tapestry at 60: an uncounted squeezed-out row among its optimal path's activations. A path change can remove it.
4 Direct: two typed phrases.
5 Reason: the chart was picked by a corpus search at the old window.
6 Display: typed.
7 Failure: "uncounted" wording missing.
8 Deleted: 503.
9 Stand-in: one chart stands for the case.
10 Same rule: none.
11 Real: yes.
Verdict: Rewrite, as A44. If Tapestry at 60 has no uncounted row, the test fails for a reason unrelated to its promise.

### 12. test_paths_fit_narrow (tests/ui/uitest_paths.cpp:513)

Delta round: window, replaces A44.md record "test_paths_fit_narrow" (Investigate, claim not checked only: a capture of Burnout's Paths tab at the narrowest panel). Old holds. Window reaches only the setup: `open_burnout` line 102 asserts `best_path().pathstring() == "3- 1 2"`. The widths asserted are layout.
Read: lines 513-535; `open_burnout` 93-105.
1 Claim: at the narrowest panel, every row, table and fold open with Copied! showing, nothing runs past its edge.
2 Owner: layout rules.
3 Real chart: claim not checked: capture, and Burnout at 60 (to confirm the precondition).
4 to 9: layout checks; the precondition is a typed string (raw).
10 Same rule: test_paths_backend_fit, test_paths_row_layout.
11 Real: yes.
Verdict: Investigate, claim not checked only.

### 13. test_paths_backend_fit (tests/ui/uitest_paths.cpp:604)

Delta round: window, replaces A44.md record "test_paths_backend_fit" (Rewrite: replace the typed 80 by the width rule's answer). Old holds. New: `kBurnoutSqOutRating` typed (lines 344, 610, 622-624) and the `open_burnout` precondition.
Read: lines 604-653; common reading.
1 Claim: at the narrowest panel Burnout's first backend table fits: Rating wraps, the fixed columns fit their text.
2 Owner: `place_columns` for Points; the rating text is the record's.
3 Real chart: claim not checked: Burnout at 60: first activation's squeezed-out rating line.
4 Direct: 610 and 624 type the rating line; the rest read layout.
5 Reason: the rating line is one run.
6 Display: partly typed.
7 Failure: Rating overflow.
8 Deleted: 610: the table not showing the squeezed-out row.
9 Stand-in: one line for "the longest line".
10 Same rule: backend_timings.
11 Real: yes.
Verdict: Rewrite, as A44. New: take the longest rating line from the record's backend rows.

### 14. test_paths_row_layout (tests/ui/uitest_paths.cpp:734)

Delta round: window, replaces A44.md record "test_paths_row_layout" (Rewrite: drop 867 and 806). Old holds. Window reaches the setup only (`open_burnout` line 102) and the badge texts the rows carry (`shown_activations(*h.app)`, line 782): the layout check runs on whatever badges Burnout's rows hold, so a different set changes which widths are checked.
Read: lines 734-800; common reading.
1 Claim: every activation row's measure, bars and badges lay out without overlap.
2 Owner: `activation_row_layout`.
3 Real chart: claim not checked.
4 to 9: as A44.
10 Same rule: none.
11 Real: yes.
Verdict: Rewrite, as A44.

### 15. test_paths_length_from_click (tests/ui/uitest_paths.cpp:920)

Delta round: window, replaces A44.md record "test_paths_length_from_click" (Rewrite: the length the screen holds is the owner's `song_length_ms` for the chart; keep the no-second-job lines). Old holds. Window reaches the setup only: `open_burnout` pins `"3- 1 2"` (line 102) and line 928 requires drawn marks (activations) exist. The typed `130087.87999999993` is parse output (`analysis_song_length`), not window-dependent.
Read: lines 920-936; common reading.
1 Claim: the click works out the song length with its analysis and nothing reads the chart again.
2 Owner: `app::song_length_ms` (D255).
3 Real chart: the typed length: claim not checked; a Burnout dump at 60 for the setup path.
4 Direct: typed literal "one logged run" (raw).
5 Reason: D255 end event rule.
6 Display: typed.
7 Failure: a second job reading the chart.
8 Deleted: 925: length not set.
9 Stand-in: `!drawn_marks().empty()` for "marks drawn".
10 Same rule: preview_activation_jumps.
11 Real: yes.
Verdict: Rewrite, as A44.

### 16. test_paths_hit_window (tests/ui/uitest_paths.cpp:966)

Delta round: window, replaces A44.md record "test_paths_hit_window" (Rewrite: each row's windows, notes and tag are the engine's note-window row; typed ms go). Old holds. This one reads the window most directly: line 991 `Early fill: 0.0 ms (optional)` on Tapped Out's activation 4 and line 1004 `Early fill: 0.0 ms (required)` on activation 3. The word is `act.is_E0()` (`path_view.cpp:280`): `is_e0(e_offset, skips, e_window_ms)` with `e_window_ms` stamped by the search at 60 (`engine.cpp:2030`). Optional versus required turns on `e_offset < window && skips == 0`; at e_offset 0.0 the first part holds at 60 and at 85, so the required activation likely stays required; whether activation 4 stays optional depends on its skips and e_offset, which no record shows.
Read: lines 938-1011; `path_view.cpp:235-290`; `model.cpp:473-475`; common reading.
1 Claim: Burnout's activation 1 shows frontend 84.5 and backend 84.3 ms and "5.4 ms safe"; Tapped Out's activation 4 shows its four window pieces, an optional early fill in a box with `85.7 ms`; activation 3 a required early fill with `82.9 ms`.
2 Owner: the engine's note-window row and `is_E0`.
3 Real chart: claim not checked: Tapped Out and Burnout at `--efill 60` and at 85: activations 3 and 4, e_offset, skips and the tag.
4 Direct: typed ms and words.
5 Reason: "Literals from T0's run" (line 971, 939). One run, at 85.
6 Display: typed.
7 Failure: the early fill row missing or mislabelled.
8 Deleted: 991, 1004: label flips.
9 Stand-in: none.
10 Same rule: `tests/test_activation_rows.cpp` pins the same row's text (named in the test's comment, not read here).
11 Real: yes.
Verdict: Rewrite, as A44. If the 60 dump shows a different activation count or tag, the test fails for the window and not for a display bug.

### 17. test_paths_hit_window_narrow (tests/ui/uitest_paths.cpp:1015)

Delta round: window, replaces A44.md record "test_paths_hit_window_narrow" (Rewrite: take the four pieces from the engine's row for activation 4). Old holds. The four pieces (`kTappedOutWindowPieces`, 941-942: `frontend 77.9 ms [K   G]`, `backend 72.0 ms [ R B ]`, `65.9 ms short`) are typed from one run; activation 4 must still exist and keep its squeeze out at 60.
Read: lines 938-953, 1015-1026; common reading.
1 Claim: at the narrowest panel Tapped Out's activation 4 still shows its four window pieces and fits its box.
2 Owner: the engine's note-window row.
3 Real chart: claim not checked: Tapped Out at 60.
4 Direct: typed.
5 Reason: "Literals from T0's run".
6 Display: typed.
7 Failure: clipping.
8 Deleted: 1024.
9 Stand-in: none.
10 Same rule: hit_window.
11 Real: yes.
Verdict: Rewrite, as A44.

---

## Counts and notes

Tests judged by file: uitest_details.cpp 26 cases (B 24, C 1, delta 1); uitest_preview.cpp 16 (B 11, C 5); uitest_paths.cpp 12 (B 1, C 11); uitest_library.cpp 31 (B 29, delta 2; the heading above the library table says 30 and 28, the correct counts are 31 and 29, the 29 names are listed); uitest_live.cpp 16 (B 15, delta 1); uitest_report_windows.cpp 24 (B 24); helper files: no test cases.

Verdicts for the 17 C records: Rewrite 15 (records 1, 3 to 11 and 13 to 17; record 8 has an extra Investigate part for Beg), Investigate 2 (records 2 and 12). Every C has question 3 as "claim not checked" with the dump named; none is Keep.

Not fully read (the verifier should open these): `uitest_report_windows.cpp` lines 1051-1105, 1171-1256, 1345-1620; `uitest_preview.cpp` lines 260-376 (the controls test body) and 534-660; `uitest_details.cpp` lines 879-956 (layout sweep), 1203-1288; `uitest_library.cpp` lines 541-704 and 805-1012; `uitest_live.cpp` 1-375 and 432-606. For these my B reason rests on the typed-number grep (no chart-derived number or path string typed) and the test headers. I did not open the commit that added any test (question 7 answered by naming a failure myself).

Splits: (a) three tests each pin Burnout's `3- 1 2`/`378,315` independently (records 1, 4, 7 and the helpers `open_burnout`, `open_burnout_preview`) with no one owner of "Burnout's optimal path". (b) `scratch_settings.h` comment says the settings are "the ones the Burnout reference results in the unit tests were made with" while the window default moved from 85 to 60; `tests/song_digest.h` pins 85 explicitly (delta-DL5 record 26) while these GUI tests inherit 60, so unit digests and GUI tests describe Burnout at different windows.
