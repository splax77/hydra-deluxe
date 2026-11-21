# T7: plant a sample of the auditors' bugs (session 7a62df56, 2026-11-15)

This is the second mechanical check in the approved plan (`docs/superpowers/plans/2026-11-14-test-suite-audit.md`, "T7"). Each record's question 8 names a bug its test should catch. T7 plants one such bug from each auditor's slice, runs the tests that record names, and checks whether the auditor was right. The user asked for it on 2026-11-15: "start the t8 plan now, run t6 and t7 in parallel, incorporate your findings into the plan when they finish".

## How it ran

Scouts read each slice file in `records/` and picked one record whose question 8 names a small, exact change in a named function. They checked each against main at 5d88062e279407e44ebd31f542b7e4793e4870d4. Executors on Opus planted the picks on throwaway branches cut from main, which were merged into `claude/test-audit-t7` and are never merged to main. The hook plants (E1 to E4) went in a throwaway worktree of the hooks repo, branch `claude/test-audit-t7-hooks`, never in the live gates folder.

Every plant sits behind a switch: it is live only when the environment variable `HYDRA_T7` names its slice (`src/core/t7_bug.h` on that branch). So one build held every plant, and each test ran with only its own bug turned on. No bug could turn another bug's test red, which the plan's batching by build could not rule out. Three plants change a constant the compiler needs as a constant (A19 and A28 in `src/core/model.h`, C9 in `tools/precheck/test_pick.h`). Those were built one change at a time in a separate throwaway worktree.

The main session ran each named test twice, with the switch off and then on. With every switch off, every named test passed. The full suite with every switch off failed only two text-reading tests ("single-owner rules hold across src/, tools/ and tests/" and "layering: every src file is in one library"), because the plants changed source text and added one header. So the plants did nothing while off.

## Totals

There are 112 slice files. 98 got a planted bug, and 14 got none. The 14 are A22 (not finished by its scout), A38, A39, A40, C4, C15, D16, WN2, WN4, WN4c, WN5 and WN6 (no record names an exact change), and T3 and T3-run2 (the calibration runs, not auditors).

Of the 98 plants:

- **91 went red at a check the record names.** For A1, A19, A28, C9, WN4b and WN8 the checker read the record itself. For the rest it compared the failing line with the picks scout's copy of the record's line.
- **1 crashed before any check (WN8).** The planted call divided by zero at the start of "collect_dm_rows joins scores to records and labels them" (`tests/test_dm_report.cpp` line 61). The record names line 91. The plant differs from the record's words: the record passes `row.delta` and the planter used `*row.delta`, because the first does not compile. So this run does not settle WN8's claim either way.
- **6 stayed green: A3, A9, A30, C7, WN3 and WN4a.** In each, the plant is the change the record names, and the test cannot see it. The reasons are below. The main session checked two of them against the code (WN4a's `free_after` is set only at `src/search/pather.cpp` line 384, and A30's plant is the record's own `<=` to `<`).

One red slice holds a smaller wrong claim. WN4b went red at line 163, a line its record names, but the record's line 145 (`CHECK_FALSE(acts[3].within_ms_limit(10.0))`) stayed green. That line calls the limit check directly, not the coach-note code the record says it guards.

## The six claims that did not hold
### A3
Question 8 (record A3, test 1, "early fill with time to spare"): "850 (reasoned): the orange flag set on a spare fill (`required && past_difficult_floor` replaced by `past_difficult_floor`)."
Planted line (src/core/model.cpp:491): `t7_bug("A3") ? past_difficult_floor(ms) : required && past_difficult_floor(ms)`
Why: the plant is exactly what the record names. The test (tests/test_activation_rows.cpp:841-852) sets skips 2 and e_offset 56.2, so ms = early_fill_difficulty(56.2) = -56.2 (model.h:341). past_difficult_floor is `ms > 2.0` (model.h:92, 97), so it is false for -56.2 and the plant's flag is false too, same as the original. The planted branch only differs for a not-required fill whose ms is above 2 ms (skips > 0 and a negative e_offset between 0 and -window), and the test never builds one. Warns stays {false,false}.

### A9
Question 8 (record A9, test 9): "262 (trim), 264 (a line after blank lines not read), 266 (comment line setting something)."
Planted line (src/core/strutil.cpp:110): value is `std::string(text.substr(eq + 1))` instead of `trim(text.substr(eq + 1))`.
Why: the value is not "trimmed whole before the plant" only on the right side. `ini_line_text` already trims the whole line (strutil.cpp:99-101), so trailing spaces are gone. The leading spaces after `=` do survive the plant, but the test's only padded value is `mslimit_value =  25  `, an int key. It is read by `parse_whole_number` (src/app/ini_errors.h:44), which calls `parse_finite_number`, and that trims again (strutil.cpp:86-87). So "  25" still gives 25. Only free-text keys use the raw value (config.cpp:248, 252), and the test sets none with padding. So the record's "262 (trim)" cannot fail from this plant.

### A30
Question 8 (record A30, test 1, line 111): "237: `m <= 0.0` turned into `m < 0.0` (reasoned; 0 BPM is covered by 233-234, negative only by 237)."
Planted line (src/core/squeeze_rating.cpp:21): `(!std::isfinite(m) || m < 0.0)` instead of `m <= 0.0`.
Why: the plant is exactly the record's words, and the record's reasoning is wrong. Line 237 uses a -120 BPM section, so m is negative, and `m < 0.0` still refuses it. The only value the change lets through is m exactly 0.0. The test's 0 BPM case (lines 233-234) gives an infinite measure, already refused by `isfinite`, and no BPM in the test gives a measure of exactly 0. So the one case the change affects is never built.

### C7
Question 8 (record C7, "a refusal is current only while..."): "118: `message_current` true on an empty message, reasoned."
Planted line (src/pathbuilder/pb_coach.h:359): `if (t7_bug("C7")) return message_generation_ == generation;` (drops the `!message_.empty()` half).
Why: the plant is the record's change. Line 118 asks `message_current(s.history().generation())` right after `open_and_wait`. Opening a chart calls `history_.reset(...)` (pb_state.cpp:136), which does `++generation_` (pb_state.cpp:80-85), so the generation is 1 or more. `message_generation_` is still its initial 0 (pb_coach.h:407), so `0 == generation` is false and the planted version also returns false. The empty-message check only matters when the two generations are equal, and the test never has them equal before a refusal. Lines 126 and 130 run after the message is set, so they do not use the dropped half either.

### WN3
Question 8 (record, window slice WN3, test_engine_sweeps.cpp:147): "line 174 hides an engine keep the model rejects; line 189 hides an engine drop the model keeps; both are checks of window-read code, `fill_within_limit` against `e_window_ms`."
Planted line (src/search/engine.cpp:2032): `act.e_window_ms = t7_bug("WN3") ? kHitWindowMs : graph.early_fill_window_ms();` (85 instead of the graph's 60).
Why: the plant changes the window the model's walk reads, as the record says. But the walk's answer does not depend on the window here. `fill_within_limit` (model.h:370-373) is true when the fill is not E0, or when `-e_offset <= limit`. The window only decides E0 for e_offset below the window. For a positive e_offset (SP early) the difficulty is negative, so it is within the test's 20 ms limit either way. For a negative e_offset the fill is E0 under both 60 and 85. Only a refused fill (e_offset below -window) would differ, and the engine never produces one. So 60 vs 85 never changes the verdict for a limit of 20 ms, whatever the chart.

### WN4a
Question 8 (record, window slice WN4a, test_replay.cpp:209): "search_target prices a chosen path differently from the full search, e.g. `follows_pins` at `engine.cpp:592` stops following pins one fill early".
Planted line (src/search/engine.cpp:593-594): `fill_tick < target_act_ticks_->back()` instead of `<=`.
Why: the plant matches the record's words. It sits after `if (!free_after_) return true;` (engine.cpp:592). The test calls `search_target` (test_replay.cpp:216), which goes to `run_target` (pather.cpp:288-291). That builds plain `EngineOptions` and only calls `pin_windows`; `free_after` stays false. Only `best_finish_options` sets it true (pather.cpp:384). So in this test `follows_pins` returns true at line 592 and never reaches the changed comparison. The test would only see it through a free-after search (the cases in tests/test_search_target.cpp).

## What this means for the audit

By the approved plan, each auditor whose claim did not hold gets a verifier who re-reads every question 8 answer in that auditor's slice. That is A3, A9, A30, C7, WN3 and WN4a. WN4b's line 145 and WN8's unsettled run go to the same re-read. The T8 plan carries this as its first step, because the re-read can change which records hold a wrong question 8 answer.

The 91 records whose named check went red move from "reasoned" to "confirmed" on question 8. A green named test proves the auditor's example wrong, not the test: the test may still catch other bugs.

The scratch files the run used (picks, manifests, the run list and the raw results) are not kept. The table below was built from them.

## One row per slice
| Slice | Plant | Tests run | Off | On | Verdict | First failing line |
|---|---|---|---|---|---|---|
| A1 | switch `A1` | -tc=path_tiles: the six sample rows; -tc=path_tiles: the tiles count only the rows shown;... | green | red | named | tests\test_report_page.cpp(138): ERROR: CHECK( tile_pairs(tiles_against(rows, every_row(rows.size()), library)) == TilePairs{{"Charts shown", "3 of 3... |
| A2 | switch `A2` | -tc=display format: the average multiplier rounds the exact double | green | red | named | tests\test_activation_rows.cpp(62): ERROR: CHECK( format_avg_mult(2.3456) == "2.346" ) is NOT correct! |
| A3 | switch `A3` | -tc=activation badges: an early fill with time to spare shows, grey (D261) | green | green | claim does not hold | [doctest] test cases: 1 \| 1 passed \| 0 failed \| 2230 skipped |
| A4 | switch `A4` | -tc=charts_with_result counts a Ready record and not a Stale or sibling-profile one | green | red | named | tests\test_analysis.cpp(220): ERROR: CHECK( charts_with_result(store, run, true).empty() ) is NOT correct! |
| A5 | switch `A5` | -tc=the click reads a chart's stated length | green | red | named | tests\test_app_click.cpp(1010): ERROR: CHECK( app->viewed.song_length_ms == 5321.0 ) is NOT correct! |
| A6 | switch `A6` | -tc=set_search narrows the library and the match count | green | red | named | tests\test_app_library.cpp(60): ERROR: CHECK( app->library.counts().all == 1 ) is NOT correct! |
| A7 | switch `A7` | -tc=app_shell: a saved window is reopened only where its title bar can be reached | green | red | named | tests\test_app_shell.cpp(74): ERROR: CHECK_FALSE( placement_on_screen({-1250, 100, 30, 820}, kOneMonitor) ) is NOT correct! |
| A8 | switch `A8` | -tc=batch text: counts group thousands and pick the right noun | green | red | named | tests\test_batch_text.cpp(19): ERROR: CHECK( counted(0, "chart", "charts") == "0 charts" ) is NOT correct! |
| A9 | switch `A9` | -tc=blank and comment lines load, and keys and values are trimmed | green | green | claim does not hold | [doctest] test cases: 1 \| 1 passed \| 0 failed \| 2230 skipped |
| A10 | switch `A10` | -tc=collect_dm_rows joins scores to records and labels them | green | red | named | tests\test_dm_report.cpp(86): ERROR: CHECK( rows[0].delta == 1000 ) is NOT correct! |
| A11 | switch `A11` | -tc=docs match code: the User Guide's hydra_rules.ini sample is the defaults | green | red | named | tests\test_docs_match_code.cpp(463): ERROR: CHECK( loaded.fill_cooldown_measures == def.fill_cooldown_measures ) is NOT correct! |
| A12 | switch `A12` | -tc=docs match code: what counts as a game fact on a Clone Hero page | green | red | named | tests\test_docs_match_code.cpp(1153): ERROR: CHECK( fact_form("0x20DF").form == FactForm::None ) is NOT correct! |
| A13 | switch `A13` | -tc=fill deadline: CH 1.1 is unchanged 4-beats-before math | green | red | named | tests\test_fill_deadline.cpp(140): ERROR: CHECK( ch11(t, fill_end, fill_lengths[f]) == doctest::Approx(want[r][b][f]) ) is NOT correct! |
| A14 | switch `A14` | -tc=collect_fill_rows: 1.0 higher and same | green | red | named | tests\test_fill_report.cpp(157): ERROR: CHECK( *dropped->delta == -50000 ) is NOT correct! |
| A15 | switch `A15` | -tc=far_time: now plus speed times secs_future | green | red | named | tests\test_highway_draw.cpp(56): ERROR: CHECK( far_time(cfg, 2.0, 2.0) == doctest::Approx(4.7) ) is NOT correct! |
| A16 | switch `A16` | -tc=jobs: the progress line's artist reads (unknown) when it is only tags | green | red | named | tests\test_library_jobs.cpp(331): ERROR: CHECK( job.snapshot().current_artist == "(unknown)" ) is NOT correct! |
| A17 | switch `A17` | -tc=library query: stars:N keeps analyzed rows with exactly N stars | green | red | named | tests\test_library_query.cpp(332): ERROR: CHECK( query_matches(q, burnout(), analyzed(7, 163.0)) ) is NOT correct! |
| A18 | switch `A18` | -tc=Activation::within_ms_limit: 0 ms timings pass a 0 ms limit | green | red | named | tests\test_model.cpp(555): ERROR: CHECK( on_end_out.within_ms_limit(0.0) ) is NOT correct! |
| A19 | src/core/model.h:92: `constexpr double kDifficultMs = 1.0;` (own build) | -tc=path buttons: each path's own hardest timing, warn past the difficult floor | green (baseline) | red | named | tests\test_path_list.cpp(69): CHECK_FALSE( b.timing_warn ) is NOT correct! |
| A20 | switch `A20` | -tc=build_preview_scene: the song length is the one handed in, never a note's time | green | red | named | tests\test_preview_scene.cpp(89): ERROR: CHECK( scene.song_length_ms == 0.0 ) is NOT correct! |
| A21 | switch `A21` | -tc=preview_config: asked before the first render it fails loudly | green | red | named | TEST CASE:  preview_config: asked before the first render it fails loudly |
| A22 | none | | | | skipped: The picks scout pinned no record for this slice. | |
| A23 | switch `A23` | -tc=shown_multiplier doubles the combo multiplier only under Star Power | green | red | named | tests\test_replay.cpp(495): ERROR: CHECK( shown_multiplier(1, true) == 2 ) is NOT correct! |
| A24 | switch `A24` | -tc=category_scores reports the multiplier each note was paid at | green | red | named | tests\test_replay.cpp(1400): ERROR: CHECK( category_scores(one, 19).multiplier == 3 ) is NOT correct! |
| A25 | switch `A25` | -tc=path report counts charts by hash in the Charts shown tile | green | red | named | tests\test_report_page.cpp(359): ERROR: CHECK( tiles_against({a, a2, b}, every_row(3), library)[0].value == "2 of 2" ) is NOT correct! |
| A26 | switch `A26` | -tc=backend_row_timing: the timing the player needs for each row | green | red | named | tests\test_rules.cpp(194): ERROR: CHECK( backend_row_timing(3.0, false, lw) == 3.0 ) is NOT correct! |
| A27 | switch `A27` | -tc=a row that disagrees with its replay keeps its empty figure | green | red | named | tests\test_saved_figure_fill.cpp(142): ERROR: CHECK_FALSE( after[0].has_value() ) is NOT correct! |
| A28 | src/core/model.h:122: `constexpr double kSqueezeWindowMs = 700.0;` (own build) | -tc=SP past the last note: tail rows use the 500 ms window from the SP end | green (baseline) | red | named | tests\test_sp_end.cpp(168): FATAL ERROR: REQUIRE( act.backends.size() == 1 ) is NOT correct! |
| A29 | switch `A29` | -tc=sp meter curve: each phrase's last note banks one bar | green | red | named | tests\test_sp_meter_curve.cpp(78): ERROR: CHECK( sp_meter_bars_at(c, 1000.0) == doctest::Approx(1.0) ) is NOT correct! |
| A30 | switch `A30` | -tc=transfer_scale_between: a measure that is not a positive finite length is refused | green | green | claim does not hold | [doctest] test cases: 1 \| 1 passed \| 0 failed \| 2230 skipped |
| A31 | switch `A31` | -tc=the scan's first copy names a chart whatever copy was analyzed | green | red | named | tests\test_store_library.cpp(181): ERROR: CHECK( names == std::map<std::string, std::string>{{"h", "Scanned Title"}, {"s", "Single Copy"}} ) is NOT c... |
| A32 | switch `A32` | -tc=set_hardest writes one chart's rows together | green | red | named | tests\test_store_open.cpp(1387): ERROR: CHECK( hydra::test::saved_figures(store, "a", profile) == std::vector<std::optional<double>>{0.0, -315.8, 12.... |
| A33 | switch `A33` | -tc=store: prepare_record refuses a profile the analysis did not run under | green | red | named | tests\test_store_records.cpp(581): ERROR: CHECK_THROWS_AS( prepare_record("h1", Profile{"mode", CapQuery::at(8), Lens{}}, analysis), std::invalid_arg... |
| A34 | switch `A34` | -tc=prepare_record refuses a profile whose ms limit isn't the analysis's | green | red | named | tests\test_store_results.cpp(309): ERROR: CHECK_THROWS_AS( prepare_record("h", profile_at(4, kLensB), ten), std::invalid_argument ) did NOT throw at ... |
| A35 | switch `A35` | -tc=banked_phrase_in_reach banks exactly what activation_can_squeeze refuses | green | red | named | tests\test_tied_variants.cpp(315): ERROR: CHECK( core::banked_phrase_in_reach(song, act_tick, act.timecode) == want ) is NOT correct! |
| A36 | switch `A36` | -tc=TrackStateOptions: equal when pro matches | green | red | named | tests\test_track_state.cpp(169): ERROR: CHECK( TrackStateOptions{true} != TrackStateOptions{false} ) is NOT correct! |
| A37 | switch `A37` | -tc=user_messages: refused chart timing names the tick | green | red | named | tests\test_user_messages.cpp(52): ERROR: CHECK( plain_error(hydra::ChartFileError(kRefused, "the tempo at tick 384 is not above 0 BPM")) == "Hydra ca... |
| A38 | none | | | | skipped: No plantable bug (manifest). | |
| A39 | none | | | | skipped: No plantable bug (manifest). | |
| A40 | none | | | | skipped: No plantable bug (manifest). | |
| A41 | switch `A41` | --test library-twin-rows | green | red | named | [FAIL] hydra/library-twin-rows |
| A42 | switch `A42` | --test library-next-scrolls-just-enough | green | red | named | [FAIL] hydra/library-next-scrolls-just-enough |
| A43 | switch `A43` | --test command-modal-named | green | red | named | [FAIL] hydra/command-modal-named |
| A44 | switch `A44` | --test paths-uncounted | green | red | named | [FAIL] hydra/paths-uncounted |
| A45 | switch `A45` | --test preview-controls | green | red | named | [FAIL] hydra/preview-controls |
| A46 | switch `A46` | --test report-window-sort | green | red | named | [FAIL] hydra/report-window-sort |
| A47 | switch `A47` | --test report-window-open-asks | green | red | named | [FAIL] hydra/report-window-open-asks |
| A48 | switch `A48` | --test cant-activate-reason | green | red | named | [FAIL] hydra/cant-activate-reason |
| A49 | switch `A49` | -tc=single-owner: the baseline dates fail on a made-up entry | green | red | named | TEST CASE:  single-owner: the baseline dates fail on a made-up entry |
| A50 | switch `A50` | -tc=store query plans: a chart's save reads records and record_paths only by key | green | red | named | tests\test_store_query_plans.cpp(237): ERROR: this statement reads every row of record_paths |
| B1 | switch `B1` | -tc=dynamics_breakdown: played_total includes/excludes 2x kick | green | red | named | tests\test_dynamics_breakdown.cpp(137): ERROR: CHECK( bd.played_total(false).all() == 14 ) is NOT correct! |
| B2 | switch `B2` | -tc=generated fills: a chord 241 ticks after the second measure line gets a fill | green | red | named | tests\test_generated_fills.cpp(67): FATAL ERROR: REQUIRE( ts.has_activation() ) is NOT correct! |
| B3 | switch `B3` | -tc=schedule: the planned end counts one frame | green | red | named | tests\test_replay_schedule.cpp(747): ERROR: CHECK( within_cent(reads[0]["planned_end_ms"].get<double>(), 7813.5) ) is NOT correct! |
| B4 | switch `B4` | -tc=schedule: sp_ready_tick is the second bank arrival | green | red | named | tests\test_replay_schedule.cpp(863): FATAL ERROR: REQUIRE( act.sp_ready_tick() == std::optional<int64_t>(384) ) is NOT correct! |
| B5 | switch `B5` | -tc=replay_path refuses a table of the wrong size or detail | green | red | named | tests\test_replay_windows.cpp(591): ERROR: CHECK_THROWS_AS( replay_path(squeeze, wl, rules, scores), std::logic_error ) did NOT throw at all! |
| B6 | switch `B6` | -tc=a parsed song keeps no spare room in its chord list | green | red | named | tests\test_song_parse.cpp(101): ERROR: CHECK( song.sequence.capacity() == song.sequence.size() ) is NOT correct! |
| B7 | switch `B7` | -tc=timing: display_tick_at_ms rounds to the nearest tick and never goes below 0 | green | red | named | tests\test_timing.cpp(408): ERROR: CHECK( st.display_tick_at_ms(999.9) == 960 ) is NOT correct! |
| C1 | switch `C1` | -tc=ms_of_frames and frames_of_ms convert at the sample rate | green | red | named | tests\test_audio_player.cpp(37): ERROR: CHECK( frames_of_ms(2.5, 1000) == 3 ) is NOT correct! |
| C2 | switch `C2` | -tc=fill landing: .chart window is trunc(res/32) + 1 ticks | green | red | named | tests\test_fill_landing.cpp(177): ERROR: CHECK( chart_landing(192, 7) == 775 ) is NOT correct! |
| C3 | switch `C3` | -tc=.mid: Expert's 2x kick never swallows a Hard kick on the same tick | green | red | named | tests\test_kick2x.cpp(62): ERROR: CHECK_FALSE( hard.sequence[0].chord.at(NoteColor::Kick)->is2x ) is NOT correct! |
| C4 | none | | | | skipped: No plantable bug; scout pinned no file or line. Test named: layering: every include stays... | |
| C5 | switch `C5` | -tc=library model: Best path sorts a Can't analyze row after Not analyzed both ways | green | red | named | tests\test_library_model.cpp(674): ERROR: CHECK( titles(m) == std::vector<std::string>{"Burnout", "Chair", "Halo", "Other", "YYZ", "Acid Romance"} ) ... |
| C6 | switch `C6` | -tc=cost tier: the 1% boundary | green | red | named | tests\test_path_costs.cpp(171): ERROR: CHECK( cost_tier(10, 1000) == CostTier::Red ) is NOT correct! |
| C7 | switch `C7` | -tc=coach click: a refusal is current only while the player's path stays as it was | green | green | claim does not hold | [doctest] test cases: 1 \| 1 passed \| 0 failed \| 2230 skipped |
| C8 | switch `C8` | -tc=precheck braces: each close, parent and children | green | red | named | tests\test_precheck_check1.cpp(143): ERROR: CHECK( b.close(1, 8) == std::nullopt ) is NOT correct! |
| C9 | tools/precheck/test_pick.h:27: `constexpr int kPickSizeLimit = 41;` (own build) | -tc=precheck pick: the three limits are the user's numbers | green (baseline) | red | named | tests\test_precheck_pick.cpp(76): CHECK( kPickSizeLimit == 40 ) is NOT correct! |
| C10 | switch `C10` | -tc=preview_audio_offset_ms combines delay and Offset | green | red | named | tests\test_preview_source.cpp(303): ERROR: CHECK( preview_audio_offset_ms(0.0, 0.25) == doctest::Approx(250.0) ) is NOT correct! |
| C11 | switch `C11` | -tc=sng: truncated input stops early instead of reading past the end | green | red | named | tests\test_sng.cpp(95): ERROR: CHECK_FALSE( sng_decode_file(whole, bad).has_value() ) THREW exception: "vector too long" |
| C12 | switch `C12` | -tc=stars: path_stars counts cutoffs reached without the solo bonus | green | red | named | tests\test_stars.cpp(337): ERROR: CHECK( stars_for_score(sc, 130) == 1 ) is NOT correct! |
| C13 | switch `C13` | -tc=text_decode: a UTF-8 byte-order mark is dropped | green | red | named | tests\test_text_decode.cpp(41): ERROR: CHECK( decode_chart_text("\xEF\xBB\xBF[song]") == "[song]" ) is NOT correct! |
| C14 | switch `C14` | -tc=the library file's side files and set-aside name sit beside it; -tc=side_files_presen... | green | red | named | tests\test_upgrade_files.cpp(66): ERROR: CHECK( with_side_files("C:\\x\\hydra.db") == std::vector<std::string>{"C:\\x\\hydra.db", "C:\\x\\hydra.db-wa... |
| C15 | none | | | | skipped: No record (skipped). | |
| C14-C15-fuzz | switch `C14-C15-fuzz` | -tc=single-owner: select_rule_files picks the rules files from a path list | green | red | named | tests\single_owner\scan.cpp(383): ERROR: CHECK( select_rule_files(paths) == std::vector<std::string>{"rules/Area.json", "rules/app.json", "rules/app/... |
| D1 | switch `D1` | -I tools/ch_mcp/tests/test_bot_run.py BotRunTest.test_the_exit_check_reports_a_game_that_... | green | red | named | FAIL: test_the_exit_check_reports_a_game_that_stays (__main__.BotRunTest.test_the_exit_check_reports_a_game_that_stays) |
| D2 | switch `D2` | -I tools/ch_mcp/tests/test_foreground.py ForegroundWatchTest.test_a_game_in_front_at_the_... | green | red | named | FAIL: test_a_game_in_front_at_the_first_poll_counts (__main__.ForegroundWatchTest.test_a_game_in_front_at_the_first_poll_counts) |
| D3 | switch `D3` | -I tools/ch_mcp/tests/test_game_files.py SetGameSettingTest.test_changes_exactly_the_line... | green | red | named | ERROR: test_changes_exactly_the_line_in_the_game_section (__main__.SetGameSettingTest.test_changes_exactly_the_line_in_the_game_section) |
| D4 | switch `D4` | -I tools/ch_mcp/tests/test_game_tools.py GameToolsTest.test_ensure_backup_takes_none_when... | green | red | named | FAIL: test_ensure_backup_takes_none_when_a_run_holds_one (__main__.GameToolsTest.test_ensure_backup_takes_none_when_a_run_holds_one) |
| D5 | switch `D5` | -I tools/ch_mcp/tests/test_game_tools.py GameToolsTest.test_path_index_must_match_path_so... | green | red | named | FAIL: test_path_index_must_match_path_source (__main__.GameToolsTest.test_path_index_must_match_path_source) (said='was made from path 2 of dump.json... |
| D6 | switch `D6` | -I tools/ch_mcp/tests/test_hydra_end.py HydraEndTest.test_hydra_end_refuses_null_or_missi... | green | red | named | FAIL: test_hydra_end_refuses_null_or_missing (__main__.HydraEndTest.test_hydra_end_refuses_null_or_missing) (text='{"song_length_ms": true}') |
| D7 | switch `D7` | -I tools/ch_mcp/tests/test_machine_recorder.py CapturedTable.test_the_idle_process_adds_n... | green | red | named | FAIL: test_the_idle_process_adds_nothing_to_the_run_queue (__main__.CapturedTable.test_the_idle_process_adds_nothing_to_the_run_queue) |
| D8 | switch `D8` | -I tools/ch_mcp/tests/test_method_watch.py StartTest.test_the_start_refuses_when_nothing_... | green | red | named | FAIL: test_the_start_refuses_when_nothing_is_attached (__main__.StartTest.test_the_start_refuses_when_nothing_is_attached) |
| D9 | switch `D9` | -I tools/ch_mcp/tests/test_sp_tools.py SpToolsTest.test_a_second_start_while_running_is_r... | green | red | named | FAIL: test_a_second_start_while_running_is_refused (__main__.SpToolsTest.test_a_second_start_while_running_is_refused) |
| D11 | switch `D11` | -I tools/ch_probe/tests/test_play_chart.py LoadScheduleTest.test_press_plan_returns_the_p... | green | red | named | FAIL: test_press_plan_returns_the_presses_in_firing_order (__main__.LoadScheduleTest.test_press_plan_returns_the_presses_in_firing_order) |
| E1 | switch `E1` | -I -m unittest tests.test_bash_windows_rule.LengthTest.test_length_is_raw_len | green | red | named | FAIL: test_length_is_raw_len (tests.test_bash_windows_rule.LengthTest.test_length_is_raw_len) |
| E2 | switch `E2` | -I -m unittest tests.test_posture.PostureResend.test_recent_send_is_skipped | green | red | named | FAIL: test_recent_send_is_skipped (tests.test_posture.PostureResend.test_recent_send_is_skipped) |
| E3 | switch `E3` | -I -m unittest tests.test_ref_judge.RewriteTests.test_non_fast_forward_move_of_main_is_re... | green | red | named | FAIL: test_non_fast_forward_move_of_main_is_refused (tests.test_ref_judge.RewriteTests.test_non_fast_forward_move_of_main_is_refused) |
| E4 | switch `E4` | -I -m unittest tests.test_stall.StalledAgents.test_exactly_ten_minutes_is_not_yet_stale | green | red | named | FAIL: test_exactly_ten_minutes_is_not_yet_stale (tests.test_stall.StalledAgents.test_exactly_ten_minutes_is_not_yet_stale) |
| DL1 | switch `DL1` | -tc=Profile compares on every part of the identity | green | red | named | tests\test_store_results.cpp(355): ERROR: CHECK( kLensA != Lens::from(10, 0, 20, false, kMaxEarlyFillWindowMs) ) is NOT correct! |
| DL2 | switch `DL2` | -tc=classify_library_file names each generation of library file | green | red | named | tests\test_store_open.cpp(202): ERROR: CHECK( classify_library_file(path) == LibraryFileKind::Current ) is NOT correct! |
| DL3 | switch `DL3` | -tc=a batch that fails as a whole still re-reads every row when it ends | green | red | named | TEST CASE:  a batch that fails as a whole still re-reads every row when it ends |
| DL4 | switch `DL4` | -tc=jobs: a chart in two folders is asked about once | green | red | named | tests\test_library_jobs.cpp(514): ERROR: CHECK( answers_for(rows, "fake0") == 1 ) is NOT correct! |
| DL5 | switch `DL5` | -tc=Skipped fills: under the 1.1 rule a fill exactly at the reach edge is stored | green | red | named | tests\test_fills_and_skips.cpp(113): ERROR: CHECK( (passed_over(edge, kHitWindowMs) == std::vector<int64_t>{9600}) ) is NOT correct! |
| DL5b | switch `DL5b` | -tc=dynamic hit window: no chord times, no row anywhere | green | red | named | tests\test_activation_rows.cpp(1548): ERROR: CHECK( av.squeeze_windows.size() == av.squeeze_sentences.size() ) is NOT correct! |
| WN1 | switch `WN1` | -tc=a record counts every kept path once | green | red | named | tests\test_store_results.cpp(734): ERROR: CHECK( record_of("", test::tied_variant_record()).pathcount == 3 ) is NOT correct! |
| WN2 | none | | | | skipped: No plantable bug. | |
| WN3 | switch `WN3` | -tc=the engine's limit walk agrees with Activation::within_ms_limit | green | green | claim does not hold | [doctest] test cases: 1 \| 1 passed \| 0 failed \| 2230 skipped |
| WN4 | none | | | | skipped: No plantable bug. | |
| WN4a | switch `WN4a` | -tc=targeted search reproduces every corpus path | green | green | claim does not hold | [doctest] test cases:   1 \|   1 passed \| 0 failed \| 2230 skipped |
| WN4b | switch `WN4b` | -tc=coach rows: a window over the Path limit carries the tight-window note | green | red | named | tests\test_pb_coach.cpp(154): ERROR: CHECK( list.rows[0].note.empty() ) is NOT correct! |
| WN4c | none | | | | skipped: No plantable bug. | |
| WN5 | none | | | | skipped: No plantable bug. | |
| WN6 | none | | | | skipped: No plantable bug. | |
| WN7 | switch `WN7` | -tc=pb files: a path file's first path becomes the player's | green | red | named | tests\test_pb_files.cpp(43): ERROR: CHECK( act_ticks(got) == act_ticks(want) ) is NOT correct! |
| WN8 | switch `WN8` | -tc=collect_dm_rows joins scores to records and labels them | green | red | crash | tests\test_dm_report.cpp(61): FATAL ERROR: test case CRASHED: Divide by zero error |
| T3 | none | | | | skipped: Calibration run, not an auditor slice. | |
| T3-run2 | none | | | | skipped: Calibration run, not an auditor slice. | |
| D10 | switch `D10` | -m unittest tools.ch_probe.tests.test_ocr.ParseAccuracyTextTests.test_number_without_unit... | green | red | named | FAIL: test_number_without_unit_is_none (tools.ch_probe.tests.test_ocr.ParseAccuracyTextTests.test_number_without_unit_is_none) |
| D12 | switch `D12` | -m unittest tools.ch_probe.tests.test_process_readonly.AccessMaskTest.test_read_only_asks... | green | red | named | FAIL: test_read_only_asks_for_read_and_query_rights (tools.ch_probe.tests.test_process_readonly.AccessMaskTest.test_read_only_asks_for_read_and_query... |
| D13 | switch `D13` | -I tools/test_agent_cost.py Pricing.test_haiku_at_exactly_the_context_limit_keeps_the_sho... | green | red | named | FAIL: test_haiku_at_exactly_the_context_limit_keeps_the_short_price (__main__.Pricing.test_haiku_at_exactly_the_context_limit_keeps_the_short_price) |
| D14 | switch `D14` | -I tools/test_check_handoff.py ScrapeMode.test_changed_since_date | green | red | named | FAIL: test_changed_since_date (__main__.ScrapeMode.test_changed_since_date) |
| D15 | switch `D15` | -NoProfile -File tools/test_build_pass.ps1 | green | red | named | FAIL three sessions: expected three different passes, got 'f5544b53968f46638b977b1411660a0b' '' '' |
| D16 | none | | | | skipped: No plantable bug. | |
