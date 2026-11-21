# T0 coverage: every test file and its slice

Listed at main `a19b9cb2` from the root `CMakeLists.txt` (the `tests/test_*.cpp` glob at line 739, `tests/report_samples.cpp` and the `tests/single_owner` files at lines 742 to 746 and 926, the `tests/ui` files at lines 604 to 630 and `tests/ui/uitest_main.cpp` at line 796, the `tests/pb_ui` files at lines 712 to 717, the `tests/fuzz/fuzz_*.cpp` glob at line 963) and every `test_*.py`, `*_test.py` and `test_*.ps1` under `tools/` and `tests/`.

Files listed: 263, in 270 rows, because seven files are cut into two parts and each part has its own row. Of the 263, 262 are in slices and one is the calibration file. Files with no slice: 0.

The user was asked on 2026-11-15 whether the hook tests and the two experiment scripts could stay out, and answered "No, audit them too". So both are in:

- The two experiment scripts are slice D16. Neither holds a `def test_`; each is one `main()` script that drives the real game, so each file is one record.
- The hook tests are the 22 `test_*` files in `C:\Users\Patrick\.claude\gates\tests\`, read at that repo's main, commit `0586e15`. They are packed by the plan's Python rule (2,500 lines, eight files) into slices E1 to E4, Python files before PowerShell ones, in name order. A Python file's unit is one `def test_` function. A PowerShell file's unit is one `Check` or `Check-Decision` call, the two helpers in `tests\lib\harness.ps1` that each print one line. The folder's other files (`replay.py`, `replay_extract.py`, `shell_output_sizes.py`, `probe_results.md`, `reflog_moves.json` and `tests\lib`) are tools and helpers, not tests; an auditor opens them when a test it judges uses them. The hook tests test the hooks, not Hydra, so question 3 (the engine's record) does not apply to them, and an E auditor writes "not a Hydra claim" there.

D16 joins round 22, which then holds four slices (D14, D15, A50 and D16). E1 to E4 are a new round 23. The audit is 92 slices over 23 rounds.

`tests/test_store_query_plans.cpp` was added to main after the plan was written (commit 8486e93c), so it has the new slice A50, appended to the last round (round 22, now three slices). The plan's packing rule, re-run on main, would put it in A33 beside `tests/test_store_records.cpp`; T0 follows the plan's rule for a missing file and appends instead. Every other slice keeps the plan's files; only some line totals moved (A18 1273 to 1324, A19 1305 to 1312, A28 862 to 856, A29 729 to 990, A33 793 to 863, A48 1455 to 1460, D15 2293 to 2297), all under the caps.

`tools/precheck/test_pick.cpp` was opened: it is the precheck's own code (`namespace precheck`, the code that picks which tests a change needs), not a test, so it has no slice.

The PowerShell helper is `Check` in all five scripts (`function Check(` in each). `tools/test_effort_test_guard.ps1` also defines `Check-Run`, which calls `Check`; its records count `Check` calls.

| File | Lines | Slice | Round |
|---|---|---|---|
| `tests/report_samples.cpp` | 105 | A1 | 1 |
| `tests/test_activation_rows.cpp` | 1695 | A2 (part 1 of 2) | 1 |
| `tests/test_activation_rows.cpp` | 1695 | A3 (part 2 of 2) | 1 |
| `tests/test_analysis.cpp` | 1410 | A4 | 1 |
| `tests/test_app_click.cpp` | 771 | A5 | 2 |
| `tests/test_app_library.cpp` | 1010 | A6 | 2 |
| `tests/test_app_reports.cpp` | 845 | A7 | 2 |
| `tests/test_app_shell.cpp` | 481 | A7 | 2 |
| `tests/test_batch_text.cpp` | 221 | A8 | 2 |
| `tests/test_best_finish.cpp` | 138 | A8 | 2 |
| `tests/test_column_widths.cpp` | 484 | A8 | 2 |
| `tests/test_config.cpp` | 955 | A9 | 3 |
| `tests/test_dm_other_speed.cpp` | 105 | A9 | 3 |
| `tests/test_dm_report.cpp` | 1059 | A10 | 3 |
| `tests/test_docs_match_code.cpp` | 2212 | A11 (part 1 of 2) | 3 |
| `tests/test_docs_match_code.cpp` | 2212 | A12 (part 2 of 2) | 3 |
| `tests/test_engine_sweeps.cpp` | 813 | A13 | 4 |
| `tests/test_fill_deadline.cpp` | 181 | A13 | 4 |
| `tests/test_fill_report.cpp` | 710 | A14 | 4 |
| `tests/test_fills_and_skips.cpp` | 328 | A14 | 4 |
| `tests/test_fitted_text_cache.cpp` | 115 | A14 | 4 |
| `tests/test_helpers.cpp` | 122 | A14 | 4 |
| `tests/test_highway_draw.cpp` | 1031 | A15 | 4 |
| `tests/test_library_jobs.cpp` | 1052 | A16 | 4 |
| `tests/test_library_query.cpp` | 652 | A17 | 5 |
| `tests/test_long_paths.cpp` | 278 | A17 | 5 |
| `tests/test_main.cpp` | 61 | A17 | 5 |
| `tests/test_model.cpp` | 1324 | A18 | 5 |
| `tests/test_path_list.cpp` | 436 | A19 | 5 |
| `tests/test_pb_text.cpp` | 365 | A19 | 5 |
| `tests/test_precheck.cpp` | 511 | A19 | 5 |
| `tests/test_preview_scene.cpp` | 1005 | A20 | 5 |
| `tests/test_preview_controller.cpp` | 998 | A21 | 6 |
| `tests/test_preview_golden.cpp` | 415 | A21 | 6 |
| `tests/test_preview_load_progress.cpp` | 445 | A22 | 6 |
| `tests/test_preview_renderer.cpp` | 380 | A22 | 6 |
| `tests/test_preview_timeline.cpp` | 592 | A22 | 6 |
| `tests/test_replay.cpp` | 1678 | A23 (part 1 of 2) | 6 |
| `tests/test_replay.cpp` | 1678 | A24 (part 2 of 2) | 6 |
| `tests/test_report_page.cpp` | 361 | A25 | 7 |
| `tests/test_report_rows.cpp` | 998 | A25 | 7 |
| `tests/test_report_view.cpp` | 612 | A26 | 7 |
| `tests/test_rules.cpp` | 503 | A26 | 7 |
| `tests/test_run_guard.cpp` | 72 | A26 | 7 |
| `tests/test_saved_figure_fill.cpp` | 404 | A27 | 7 |
| `tests/test_search_target.cpp` | 517 | A27 | 7 |
| `tests/test_song_audio.cpp` | 140 | A27 | 7 |
| `tests/test_song_input.cpp` | 401 | A27 | 7 |
| `tests/test_sp_end.cpp` | 856 | A28 | 7 |
| `tests/test_sp_meter_curve.cpp` | 990 | A29 | 8 |
| `tests/test_squeeze_rating.cpp` | 993 | A30 | 8 |
| `tests/test_stem_reader.cpp` | 602 | A31 | 8 |
| `tests/test_store_library.cpp` | 725 | A31 | 8 |
| `tests/test_store_open.cpp` | 1258 | A32 | 8 |
| `tests/test_store_records.cpp` | 863 | A33 | 9 |
| `tests/test_store_results.cpp` | 731 | A34 | 9 |
| `tests/test_theme.cpp` | 189 | A34 | 9 |
| `tests/test_tied_variants.cpp` | 619 | A35 | 9 |
| `tests/test_track_state.cpp` | 1114 | A36 | 9 |
| `tests/test_user_messages.cpp` | 453 | A37 | 10 |
| `tests/test_winstr.cpp` | 301 | A37 | 10 |
| `tests/ui/uitest_batch_reports.cpp` | 746 | A37 | 10 |
| `tests/ui/uitest_commands.cpp` | 438 | A38 | 10 |
| `tests/ui/uitest_details.cpp` | 1384 | A39 | 10 |
| `tests/ui/uitest_engine.cpp` | 477 | A40 | 10 |
| `tests/ui/uitest_harness.cpp` | 670 | A40 | 10 |
| `tests/ui/uitest_library.cpp` | 2069 | A41 (part 1 of 2) | 11 |
| `tests/ui/uitest_library.cpp` | 2069 | A42 (part 2 of 2) | 11 |
| `tests/ui/uitest_live.cpp` | 762 | A43 | 11 |
| `tests/ui/uitest_main.cpp` | 358 | A43 | 11 |
| `tests/ui/uitest_paths.cpp` | 1057 | A44 | 11 |
| `tests/ui/uitest_preview.cpp` | 1110 | A45 | 12 |
| `tests/ui/uitest_report_windows.cpp` | 1659 | A46 (part 1 of 2) | 12 |
| `tests/ui/uitest_report_windows.cpp` | 1659 | A47 (part 2 of 2) | 12 |
| `tests/pb_ui/pbtest_build.cpp` | 533 | A48 | 12 |
| `tests/pb_ui/pbtest_coach.cpp` | 574 | A48 | 12 |
| `tests/pb_ui/pbtest_files.cpp` | 78 | A48 | 12 |
| `tests/pb_ui/pbtest_harness.cpp` | 178 | A48 | 12 |
| `tests/ui/uitest_script.cpp` | 59 | A48 | 12 |
| `tests/ui/uitest_tests.cpp` | 38 | A48 | 12 |
| `tests/pb_ui/pbtest_main.cpp` | 85 | A49 | 13 |
| `tests/pb_ui/pbtest_window.cpp` | 630 | A49 | 13 |
| `tests/single_owner/scan.cpp` | 626 | A49 | 13 |
| `tests/test_cli.cpp` | 723 | B1 | 13 |
| `tests/test_dynamics_breakdown.cpp` | 505 | B1 | 13 |
| `tests/test_fuzz_regressions.cpp` | 144 | B1 | 13 |
| `tests/test_generated_fills.cpp` | 281 | B2 | 13 |
| `tests/test_midi.cpp` | 926 | B2 | 13 |
| `tests/test_pathstring_read.cpp` | 218 | B2 | 13 |
| `tests/test_replay_schedule.cpp` | 1731 | B3 (part 1 of 2) | 13 |
| `tests/test_replay_schedule.cpp` | 1731 | B4 (part 2 of 2) | 14 |
| `tests/test_replay_windows.cpp` | 598 | B5 | 14 |
| `tests/test_scoring.cpp` | 96 | B5 | 14 |
| `tests/test_song_parse.cpp` | 1392 | B6 | 14 |
| `tests/test_timing.cpp` | 460 | B7 | 14 |
| `tests/test_activation_row_layout.cpp` | 261 | C1 | 15 |
| `tests/test_app_settings.cpp` | 241 | C1 | 15 |
| `tests/test_audio_decode.cpp` | 179 | C1 | 15 |
| `tests/test_audio_device.cpp` | 46 | C1 | 15 |
| `tests/test_audio_mixer.cpp` | 117 | C1 | 15 |
| `tests/test_audio_player.cpp` | 160 | C1 | 15 |
| `tests/test_badge_pill.cpp` | 165 | C1 | 15 |
| `tests/test_build_flags.cpp` | 57 | C1 | 15 |
| `tests/test_chart_index.cpp` | 167 | C2 | 15 |
| `tests/test_chord_code.cpp` | 91 | C2 | 15 |
| `tests/test_difficulty_table.cpp` | 62 | C2 | 15 |
| `tests/test_disco_flip.cpp` | 137 | C2 | 15 |
| `tests/test_display_format.cpp` | 63 | C2 | 15 |
| `tests/test_dynamics_tag.cpp` | 147 | C2 | 15 |
| `tests/test_fast_tempo.cpp` | 418 | C2 | 15 |
| `tests/test_fill_landing.cpp` | 397 | C2 | 15 |
| `tests/test_five_lane.cpp` | 534 | C3 | 15 |
| `tests/test_font_coverage.cpp` | 164 | C3 | 15 |
| `tests/test_http_download.cpp` | 131 | C3 | 15 |
| `tests/test_icons.cpp` | 32 | C3 | 15 |
| `tests/test_image_decode.cpp` | 60 | C3 | 15 |
| `tests/test_kick2x.cpp` | 245 | C3 | 15 |
| `tests/test_label_layout.cpp` | 81 | C3 | 15 |
| `tests/test_launch.cpp` | 101 | C3 | 15 |
| `tests/test_layering.cpp` | 653 | C4 | 15 |
| `tests/test_library_model.cpp` | 992 | C5 | 16 |
| `tests/test_library_read_order.cpp` | 78 | C5 | 16 |
| `tests/test_list_scroll.cpp` | 51 | C5 | 16 |
| `tests/test_live_protocol.cpp` | 84 | C5 | 16 |
| `tests/test_note_shuffle.cpp` | 243 | C5 | 16 |
| `tests/test_note_window.cpp` | 394 | C6 | 16 |
| `tests/test_obj_loader.cpp` | 186 | C6 | 16 |
| `tests/test_overlay_layout.cpp` | 516 | C6 | 16 |
| `tests/test_path_costs.cpp` | 253 | C6 | 16 |
| `tests/test_pb_coach.cpp` | 677 | C7 | 16 |
| `tests/test_pb_files.cpp` | 222 | C7 | 16 |
| `tests/test_pb_state.cpp` | 124 | C7 | 16 |
| `tests/test_perf_digest.cpp` | 166 | C7 | 16 |
| `tests/test_phrase_end.cpp` | 123 | C7 | 16 |
| `tests/test_precheck_check1.cpp` | 210 | C8 | 16 |
| `tests/test_precheck_check24.cpp` | 219 | C8 | 16 |
| `tests/test_precheck_check3.cpp` | 159 | C8 | 16 |
| `tests/test_precheck_check5.cpp` | 165 | C8 | 16 |
| `tests/test_precheck_git.cpp` | 326 | C8 | 16 |
| `tests/test_precheck_pick.cpp` | 646 | C9 | 17 |
| `tests/test_precheck_pick_exe.cpp` | 204 | C9 | 17 |
| `tests/test_precheck_pick_lines.cpp` | 140 | C9 | 17 |
| `tests/test_precheck_pick_names.cpp` | 206 | C9 | 17 |
| `tests/test_precheck_rows.cpp` | 224 | C9 | 17 |
| `tests/test_preview_clock.cpp` | 56 | C9 | 17 |
| `tests/test_preview_config.cpp` | 141 | C10 | 17 |
| `tests/test_preview_source.cpp` | 526 | C10 | 17 |
| `tests/test_preview_transport.cpp` | 318 | C10 | 17 |
| `tests/test_rules_file.cpp` | 93 | C10 | 17 |
| `tests/test_section_names.cpp` | 88 | C10 | 17 |
| `tests/test_sng.cpp` | 347 | C11 | 17 |
| `tests/test_song_length.cpp` | 335 | C11 | 17 |
| `tests/test_song_panel_state.cpp` | 130 | C11 | 17 |
| `tests/test_sp_end_clamp.cpp` | 342 | C11 | 17 |
| `tests/test_srb.cpp` | 454 | C12 | 17 |
| `tests/test_stars.cpp` | 365 | C12 | 17 |
| `tests/test_stream_mix.cpp` | 479 | C12 | 17 |
| `tests/test_strutil.cpp` | 142 | C12 | 17 |
| `tests/test_text_decode.cpp` | 90 | C13 | 18 |
| `tests/test_text_role.cpp` | 239 | C13 | 18 |
| `tests/test_timeline_view.cpp` | 136 | C13 | 18 |
| `tests/test_update_release.cpp` | 245 | C13 | 18 |
| `tests/test_update_state.cpp` | 531 | C13 | 18 |
| `tests/test_update_verify.cpp` | 174 | C13 | 18 |
| `tests/fuzz/fuzz_audio.cpp` | 6 | C14 | 18 |
| `tests/fuzz/fuzz_chart.cpp` | 6 | C14 | 18 |
| `tests/single_owner/clones.cpp` | 302 | C14 | 18 |
| `tests/single_owner/engine.cpp` | 383 | C14 | 18 |
| `tests/single_owner/special.cpp` | 63 | C14 | 18 |
| `tests/test_upgrade_files.cpp` | 206 | C14 | 18 |
| `tests/test_utf8_paths.cpp` | 47 | C14 | 18 |
| `tests/test_wait_util.cpp` | 18 | C14 | 18 |
| `tests/fuzz/fuzz_image.cpp` | 6 | C15 | 18 |
| `tests/fuzz/fuzz_mid.cpp` | 6 | C15 | 18 |
| `tests/fuzz/fuzz_sng.cpp` | 6 | C15 | 18 |
| `tests/fuzz/fuzz_srb.cpp` | 6 | C15 | 18 |
| `tools/ch_mcp/tests/test_bot_run.py` | 1249 | D1 | 18 |
| `tools/ch_mcp/tests/test_build_facts.py` | 244 | D1 | 18 |
| `tools/ch_mcp/tests/test_build_stamp.py` | 193 | D1 | 18 |
| `tools/ch_mcp/tests/test_chdis.py` | 141 | D1 | 18 |
| `tools/ch_mcp/tests/test_compare_dumps.py` | 263 | D1 | 18 |
| `tools/ch_mcp/tests/test_compare_scorestats.py` | 216 | D1 | 18 |
| `tools/ch_mcp/tests/test_dump_facts.py` | 828 | D2 | 19 |
| `tools/ch_mcp/tests/test_foreground.py` | 145 | D2 | 19 |
| `tools/ch_mcp/tests/test_frida_attach.py` | 1498 | D2 | 19 |
| `tools/ch_mcp/tests/test_frida_tools.py` | 1006 | D3 | 19 |
| `tools/ch_mcp/tests/test_front.py` | 880 | D3 | 19 |
| `tools/ch_mcp/tests/test_game_files.py` | 410 | D3 | 19 |
| `tools/ch_mcp/tests/test_game_tools.py` | 3557 | D4 (part 1 of 2) | 19 |
| `tools/ch_mcp/tests/test_game_tools.py` | 3557 | D5 (part 2 of 2) | 19 |
| `tools/ch_mcp/tests/test_hydra_end.py` | 213 | D6 | 20 |
| `tools/ch_mcp/tests/test_kit_tools.py` | 1814 | D6 | 20 |
| `tools/ch_mcp/tests/test_launch_core.py` | 248 | D6 | 20 |
| `tools/ch_mcp/tests/test_launch_patch.py` | 138 | D6 | 20 |
| `tools/ch_mcp/tests/test_machine_recorder.py` | 960 | D7 | 20 |
| `tools/ch_mcp/tests/test_machine_report.py` | 689 | D7 | 20 |
| `tools/ch_mcp/tests/test_menu_tools.py` | 568 | D7 | 20 |
| `tools/ch_mcp/tests/test_method_watch.py` | 487 | D8 | 20 |
| `tools/ch_mcp/tests/test_mono_facts.py` | 227 | D8 | 20 |
| `tools/ch_mcp/tests/test_press_schedule.py` | 335 | D8 | 20 |
| `tools/ch_mcp/tests/test_quiet.py` | 416 | D8 | 20 |
| `tools/ch_mcp/tests/test_recorder_link.py` | 331 | D8 | 20 |
| `tools/ch_mcp/tests/test_server.py` | 390 | D8 | 20 |
| `tools/ch_mcp/tests/test_sp_tools.py` | 797 | D9 | 20 |
| `tools/ch_mcp/tests/test_trace_hits.py` | 586 | D9 | 20 |
| `tools/ch_probe/tests/test_cheap_checks.py` | 274 | D9 | 20 |
| `tools/ch_probe/tests/test_debugger.py` | 334 | D9 | 20 |
| `tools/ch_probe/tests/test_early_fill_walk.py` | 155 | D9 | 20 |
| `tools/ch_probe/tests/test_edge_probe.py` | 183 | D9 | 20 |
| `tools/ch_probe/tests/test_engine.py` | 386 | D10 | 21 |
| `tools/ch_probe/tests/test_engine_finder.py` | 188 | D10 | 21 |
| `tools/ch_probe/tests/test_find_checksum.py` | 352 | D10 | 21 |
| `tools/ch_probe/tests/test_hit_window_scripts.py` | 219 | D10 | 21 |
| `tools/ch_probe/tests/test_input_driver.py` | 206 | D10 | 21 |
| `tools/ch_probe/tests/test_ocr.py` | 62 | D10 | 21 |
| `tools/ch_probe/tests/test_overlay_logs.py` | 92 | D10 | 21 |
| `tools/ch_probe/tests/test_play_chart.py` | 2312 | D11 | 21 |
| `tools/ch_probe/tests/test_play_in_game.py` | 380 | D12 | 21 |
| `tools/ch_probe/tests/test_prep_inputs.py` | 170 | D12 | 21 |
| `tools/ch_probe/tests/test_probe_chart.py` | 174 | D12 | 21 |
| `tools/ch_probe/tests/test_probe_songs.py` | 156 | D12 | 21 |
| `tools/ch_probe/tests/test_process.py` | 194 | D12 | 21 |
| `tools/ch_probe/tests/test_process_readonly.py` | 61 | D12 | 21 |
| `tools/ch_probe/tests/test_s2_window_constants.py` | 88 | D12 | 21 |
| `tools/ch_probe/tests/test_score_compare.py` | 344 | D12 | 21 |
| `tools/ch_probe/tests/test_sp_compare.py` | 897 | D13 | 21 |
| `tools/ch_probe/tests/test_sp_watch.py` | 417 | D13 | 21 |
| `tools/ch_probe/tests/test_stall_harness.py` | 372 | D13 | 21 |
| `tools/ch_probe/tests/test_tick_rate_vs_hydra.py` | 67 | D13 | 21 |
| `tools/ch_probe/tests/test_watch_checks.py` | 212 | D13 | 21 |
| `tools/test_agent_cost.py` | 271 | D13 | 21 |
| `tools/test_check_handoff.py` | 310 | D14 | 22 |
| `tools/test_check_pointers.py` | 269 | D14 | 22 |
| `tools/test_compare_db.py` | 312 | D14 | 22 |
| `tools/test_effort_test_faults.py` | 238 | D14 | 22 |
| `tools/test_effort_test_scout.py` | 91 | D14 | 22 |
| `tools/test_effort_test_shuffle.py` | 178 | D14 | 22 |
| `tools/test_effort_test_taint.py` | 228 | D14 | 22 |
| `tools/test_build_pass.ps1` | 445 | D15 | 22 |
| `tools/test_ci_build_folder.ps1` | 134 | D15 | 22 |
| `tools/test_effort_test_guard.ps1` | 194 | D15 | 22 |
| `tools/test_hydra_ui_mcp.py` | 1129 | D15 | 22 |
| `tools/test_mutation_probe.ps1` | 271 | D15 | 22 |
| `tools/test_release_sign.ps1` | 124 | D15 | 22 |
| `tests/test_store_query_plans.cpp` | 369 | A50 | 22 |
| `tools/ch_probe/experiments/key_delivery_test.py` | 72 | D16 | 22 |
| `tools/ch_probe/experiments/pad_flash_test.py` | 75 | D16 | 22 |
| `C:\Users\Patrick\.claude\gates\tests\test_agent_files.py` | 58 | E1 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_agent_stop.py` | 493 | E1 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_bash_windows_rule.py` | 331 | E1 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_common.py` | 183 | E1 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_delete_rule.py` | 416 | E1 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_docs_numbers.py` | 200 | E1 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_media_rule.py` | 235 | E1 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_percall_key.py` | 222 | E1 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_percall_rewrite.py` | 162 | E2 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_posture.py` | 277 | E2 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_progress.py` | 771 | E2 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_publish.py` | 445 | E2 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_ref_judge.py` | 850 | E3 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_refusals.py` | 88 | E3 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_review_dispatch.py` | 363 | E3 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_review_rules.py` | 1017 | E3 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_ruling_rules.py` | 448 | E4 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_shell_output.py` | 422 | E4 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_stall.py` | 436 | E4 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_h52_cleanup_sweep.ps1` | 489 | E4 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_h53_session_cards_start.ps1` | 269 | E4 | 23 |
| `C:\Users\Patrick\.claude\gates\tests\test_status_append.ps1` | 49 | E4 | 23 |
| `tests/test_preview_boxes.cpp` | 1097 | T3 |  |
