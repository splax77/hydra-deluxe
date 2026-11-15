# Sweep SW4: question-3 sweep of records D2 to D9, D12 to D15, E1 to E4

Task T5-SW4. Method: for each Rewrite row of `sweep/rewrites.tsv` I read the record and its verify entries. I did not open any test code. "Q3 only" means the record's reason for Rewrite is only that the claim was not checked on the real thing. A row is "Other fault" when the record names any other failed question (4, 5, 6, 8, 9 or 10), a name that does not match the test, or a timing sleep. "none named" in the last column means the record names no capture or real input. Where a verify file disputes the record, the dispute is quoted on the row.

Result in one line: no row of this group is Q3 only. Every record names a second fault. Some verify files say a verdict should be Investigate for a different reason (for example D2's field-offset test, D3's ghost-axis test, D4's no-song launch test, D5's probes-line test, D6's two DLL-bytes tests, E1's finished-flag test, and most of E2's progress tests); the main session decides those.

Note on aggregate rows: `rewrites.tsv` gives D13, D15 and E3 one row each (a section heading), although the records files hold more Rewrite verdict lines (D13 29, D15 22, E3 7). I judged the Rewrite verdicts inside each row's own section. The other Rewrite verdicts in those three files are not rows of the tsv and I did not judge them.

Row format: heading | verdict | the record's words (and the verify file's if it disputes) | the capture or real input named, for Q3-only rows.

## D2

test_the_line_lists_when_the_game_came_forward_since_the_start (line 112) | Other fault (Q4) | "These are raw results: the working ... lives only in the comment"; "Name the inputs ... and state the expected seconds as the arrival minus the start" | -
test_field_with_a_metadata_offset_note_keeps_its_offset (line 192) | Other fault (Q4/5/9) | "Whole-dict equality is the stand-in for 'keeps its offset'"; "'0D58' has no reason". Verify disputes: "should be Investigate (claim not checked only)" and disproves the record's production finding about the name | -
test_three_done_lines_and_a_crash_exit_count_as_good (line 294) | Other fault (Q4/5/9) | "the file list is compared with the same tuple the code uses (a copy)"; "Split the claims ... drop the copies" | -
test_a_note_under_code_4045_is_looked_up_in_the_4045_table_not_the_6142_one (line 603) | Other fault (Q4) | "three claims in one test ... split into three tests" | -
test_map_rows_that_do_not_match_the_dump_are_errors (line 722) | Other fault (Q4/9) | "Eight claims in one test"; "a message can appear for the wrong row" | -
test_a_4045_map_names_its_class_by_name (line 759) | Other fault (Q4) | "Four claims in one test; split into one per claim" | -
test_a_4045_span_naming_a_mapped_property_gets_an_unlisted_row_and_a_warning (line 790) | Other fault (Q4) | "Three claims in one test. Split the three claims" | -
test_attaches_to_the_game_by_process_name (line 300) | Other fault (Q9) | "compares production with production" (the imported constant) | -
test_returns_true_at_once_when_the_event_is_set (line 364) | Other fault (Q4) | "'at once' is not asserted; only True" (the record says Q3 is met on the real mechanism) | -
test_wakes_when_the_event_is_set_during_the_wait (line 372) | Other fault (Q4/8) | "the line cannot fail for the bug named" | -
test_every_message_reaches_the_log_in_order_once (line 798) | Other fault (Q4/5/9) | "200 ... unnamed"; "line 807 pins the exact separator form" | -
test_detach_waits_for_the_writer_and_says_so_when_the_cap_passes (line 821) | Other fault (Q4) | "Two claims in one test; split" | -
test_summary_counts_a_clock_that_went_backwards (line 899) | Other fault (Q4, with Q3) | "Say where the 2 comes from (the two drops in the named input) so the line reads as the rule; the same capture is needed" | none named
test_the_command_line_refuses_a_zero_cap (line 965) | Other fault (Q4/9) | "any SystemExit passes ... Real stand-in" | -
test_agent_calls_only_named_methods (line 1033) | Other fault (Q4/9) | "Drop the typed copy of menu.ts's list"; "line 1035 duplicates the list" | -
test_kit_load_map_passes_the_method_pointer (line 1079) | Other fault (Q4/9) | "whitespace-exact pins"; "a source layout pin stands in for 'the call passes the pointer'" | -
test_one_more_invoke_in_kit_ts_fails (line 1118) | Other fault (Q5/9) | "The count ... is a snapshot of how many call sites exist today" | -
test_an_invoke_in_a_second_file_fails (line 1124) | Other fault (Q5/9) | "shares their snapshot-count fault" | -
test_a_lookup_by_a_name_that_is_not_a_literal_fails (line 1135) | Other fault (Q4/9) | "`len(...) == 1` stands in for 'the right finding'" | -
test_a_native_function_fails (line 1140) | Other fault (Q4/9) | "Compare the finding text, as the kit variant does" | -
test_kit_hooks_game_manager_update_once (line 1254) | Other fault (Q5/9) | "25387 ... a copy of the data file's own number"; "a source count stands in for 'the hook is placed once'" | -
test_kit_play_holds_no_number_of_its_own (line 1263) | Other fault (Q9) | "drop the three pins of source text" | -
test_a_play_that_waits_fails (line 1282) | Other fault (Q9) | "depends on kit.ts having exactly ... (layout)"; "build the bad text by hand" | -
test_a_stop_that_skips_unhook_fails (line 1292) | Other fault (Q9) | "a harmless reformat of those two spots fails the test" | -
test_only_gamemanager_ts_names_the_native_object_field (line 1319) | Other fault (Q9) | "menu.ts's `isDestroyed` check is weak ... a comment would pass" | -
test_the_state_agent_sends_the_destroyed_count_in_both_replies (line 1331) | Other fault (Q4/9, with Q3) | "the test searches source strings including an indented line ... until then Investigate" | none named (a real found state reply and a real menu reply)
test_the_check_catches_a_patch_outside_the_observer (line 1410) | Other fault (Q4/9) | "any finding passes: a stand-in"; "Each of the four bad cases should compare its own finding text" | -
test_launch_ts_sends_an_error_key_only_for_a_fault (line 1456) | Other fault (Q9) | "the exact line pins the wording of the message" | -
test_build_scripts_end_by_themselves (line 1482) | Other fault (Q9) | "the name list stands in for 'every agent has a build script'"; "drop the typed name list" | -

## D3

test_attach_before_the_delay_waits_the_rest_then_attaches (test_frida_tools.py:242) | Other fault (Q4/5, with Q3) | "3.0 and 5 are the test's working, not named"; "write them from the inputs ... A real-capture check is still needed" | none named
test_a_stale_trace_agent_starts_no_trace (:353) | Other fault (name does not match / Q4) | "Move to the `trace_hits` test file or call `ch_trace_start`; split the three claims" | -
test_the_threshold_line_names_the_value_and_the_offsets_constant (:430) | Other fault (Q4/5) | "'+0x2c0' copy of the constant's value"; "Build the expected '+0x...' from `C.OFF_DYN_THRESHOLD`" | -
test_the_ghost_value_is_the_one_pad_axis_gives_at_full_precision (:434) | Other fault (Q9) | "wiring check only; state what the ghost axis must be from the game's rule". Verify disputes: "should be Investigate (claim not checked only)" | -
test_it_parses_with_one_table_per_enum_field (:552) | Other fault (Q4/5) | "a snapshot of the json's content ... Q4/Q5 fail (snapshot)" | -
PlayerScreenHookTest.test_the_hooked_method_is_the_player_screens_own (:651) | Other fault (Q4/5) | "the method name is typed as an obfuscated string ... a snapshot of the build's name"; "Replace the typed obfuscated name" | -
test_state_leaves_the_menu_block_to_ch_menu (:696) | Other fault (Q9/8) | "the word 'menu' is a loose stand-in"; verify: "no line of MENU_LINES holds a lowercase 'menu'" so line 700 cannot fail | -
test_player_screen_on_the_instrument_page_with_drum_rows (:719) | Other fault (Q4) | "the raw result ... follows from cursor 1 + scroll 1 ...; state it from the named inputs" | -
test_trace_start_refuses_with_no_live_game_manager (:856) | Other fault (Q5, small) | "copy of `NO_GAME_MANAGER`'s string ... Q5 copy"; "Rewrite (small): use `FT.NO_GAME_MANAGER`. Claim not checked either" | none named
test_trace_start_takes_the_backup_just_before_it_hooks (:871) | Other fault (Q4/9) | "whether the backup call precedes the hook in time is not observed"; "record the call order" | -
test_notes_from_since_are_lined_up (:954) | Other fault (Q5) | "make the within-limit count a function of `C.EXPECT_NORMAL_BACK_MS` ... or the limit value is hidden" | -
test_both_hook_rvas_name_one_engine_method_each (RealFactsTest, :995) | Other fault (Q9) | "`len(names) == 2`: a count, stand-in"; verify: "true by construction" | -
test_an_exit_between_calls_fails_the_next_call_the_same_way (:408) | Other fault (timing sleep) | "leans on timing"; "wait for the worker's exit as an observable instead of a fixed sleep" | -
test_an_initialize_offering_2026_07_28_is_answered_as_a_handshake_connection (:557) | Other fault (Q5) | "typed as a copy"; "use `LATEST_HANDSHAKE_VERSION`" | -
test_a_call_in_flight_refuses_the_reload_and_is_named (:577) | Other fault (timing sleep) | "A busy machine can make the reload win"; "not a sleep" | -
test_a_call_in_flight_through_ch_call_is_named_by_its_inner_tool (:594) | Other fault (timing sleep) | "Rewrite (same sleep fault)" | -
test_a_worker_that_never_answers_the_check_is_left_running (:615) | Other fault (timing sleep) | "Rewrite (sleep fault, as :577)" | -
test_a_worker_dead_before_the_check_is_replaced_and_the_reply_says_so (:640) | Other fault (timing sleep) | "Rewrite (sleep fault; same fix as :408)" | -
test_an_open_call_does_not_hold_the_exit_past_the_cap (:799) | Other fault (timing sleep / Q9) | "the sleep is not a correctness risk but the claim ... " (record cut by my view; verify "Agree" Rewrite) | -
test_the_front_holds_no_reply_cut_and_no_process_group (:863) | Other fault (Q9) | "a text search standing in for behaviour (Q9)" | -
test_a_changed_key_is_deleted_imported_and_checked (:225) | Other fault (Q4/5) | "copy of the algorithm's call list"; "drop the typed call list" | -
test_restore_all_puts_the_key_back (:268) | Other fault (Q10 / name) | "Overlap with :217"; "Name says 'puts the key back' but the key is unchanged" | -
test_the_song_ini_length_reader_is_gone (:404) | Other fault (Q9) | "a negative check that a name does not exist"; "test the behaviour" | -

## D4

test_a_quiet_machine_puts_the_report_before_the_launch_lines (test_game_tools.py:352) | Other fault (Q5) | "5 s and 30 percent: Q5 fails here, no source named"; "typed copies of the fake" | -
test_ch_start_has_no_argument_that_skips_the_check (:395) | Other fault (Q4/9, with Q3) | "a copy of the signature, so 'any change at all' fails it, a snapshot"; "stand-in" | none named (the server's real tool schema for ch_start)
test_ch_start_with_no_song_launches_launch_commands_no_song_line (:428) | Other fault (Q9) | "a copy: if launch_command is wrong, both agree"; "two Hydra functions compared with each other". Verify disputes: "should be Investigate (claim not checked only: a real no-song launch line)" | none named (a real no-song launch line)
test_the_command_line_comes_from_launch_command_with_known_flags_only (:435) | Other fault (Q5/9) | "The three flag names have no source cited"; "owner compared with itself on 439" | -
test_ch_stop_says_when_the_game_outlives_the_wait_and_still_restores (:491) | Other fault (Q5) | "30 s: no source named ... Q5 fails for the 30" | -
test_ch_press_refuses_an_unknown_lane_with_the_list (:696) | Other fault (Q5) | "The list is the Lane enum's names; typed rather than read" | -
test_ch_key_refuses_an_unknown_key_with_the_list_and_sends_nothing (:770) | Other fault (Q5) | "list is MENU_KEYS; typed" | -
test_run_cap_is_the_song_plus_the_extra_minute_else_240_s (:833) | Other fault (Q4) | "320.504 is a raw result built from it"; "State it as `cap_for_end(end_ms) == end_ms/1000 + 60`" (60 and 240 are the user's choice at decisions line 1241) | none named
test_the_schedules_run_cap_reads_its_song_end_and_refuses_null (:839) | Other fault (Q4, with Q3) | "a raw result (260504 + 60 s)"; "Write the expected cap through the rule on the named input ... and check on a real schedule file" | none named
test_ch_play_chart_with_a_song_folder_passes_the_cap_to_live_play (:846) | Other fault (Q4, with Q3) | "Express 320.504 as the rule on the named 260504 input" | none named
test_a_second_ch_play_chart_is_refused_while_one_runs (:926) | Other fault (Q5) | "120 s: Q5 fails, no source in the test" | -
test_the_freak_reply_names_both_anchors_off_the_schedule (:1173) | Other fault (Q6/4) | "Q6 fails: a display test whose expected side is typed instead of read from the engine's record"; "Every number is a raw result copied from one run's file" | none named (Freak is not in records-engine)
test_the_mouths_reply_names_the_squeezes_and_the_declined_fills (:1178) | Other fault (Q6/4) | "Rewrite, as 1173" | -
test_the_bomber_reply_names_the_early_anchor (:1183) | Other fault (Q6/4) | "Rewrite, as 1173" | -
test_the_mouths_order_line_follows_the_anchor_lines (:1188) | Other fault (Q6) | "Q6 fails as 1173 (typed expected)"; "no chord number typed" | -
test_the_bomber_order_line_follows_the_anchor_line (:1193) | Other fault (Q6) | "Rewrite, as 1188" | -
test_the_search_is_the_auto_players_with_its_dots_kept_off_stdout (:1496) | Other fault (Q9, with Q3) | "'not None' for 'not stdout'"; "stand-in" | none named (a real run's stdout)
test_the_rows_reach_the_file_in_order_and_only_once (:1536) | Other fault (Q5/4) | "'range(200)' has no name"; "check the whole row, not only column 0" | -
test_a_run_reusing_a_log_name_starts_the_file_over (:1653) | Other fault (Q4/5/9) | "typed as a count with no working shown"; "a count passes with wrong rows" | -

## D5

test_the_mouths_run_reply_names_its_squeezes_after_the_declined_line (1816) | Other fault (Q4/5, with Q3) | "numbers typed with no working on the line"; "Read each number from the fixture JSON ... drop the 700-character typed lines" | none named (Mouths is not in records-engine)
test_the_freak_run_reply_names_its_anchors (1823) | Other fault (Q4/5) | "same fault as 1816 (raw typed fixture numbers)" | -
test_the_bomber_run_reply_names_its_early_anchor (1827) | Other fault (Q4/5) | "Same faults as 1816" | -
test_the_tools_take_schedule_json_and_no_dump_json (1951) | Other fault (Q4/9) | "a snapshot of three signatures: adding any parameter for any reason fails it"; "their only source is the code under test (copy)" | -
test_game_tools_names_no_kit_press_source_and_no_timing (1990) | Other fault (Q9) | "a stand-in: a source-text search"; "Replace the text search with a check that a kit run never reaches `live_play`" | -
test_a_human_launch_adds_the_launch_agent_to_the_builds_record (2108) | Other fault (name does not match) | "Check the builds record of a human run (the file the name says) or rename to what it checks" | -
test_run_chart_with_probes_says_the_probes_line_after_the_plays (2168) | Other fault (Q9) | "the expected side calls the producer, a copy: it passes if the line is wrong in the same way". Verify disputes: "Should be Investigate (claim not checked only: a real reply)" | none named (a real reply)
test_the_marker_sits_under_the_scratch_root_and_never_the_real_temp_folder (2470) | Other fault (Q9) | "The expected side repeats the production line 1441 ...: a copy" | -
test_a_song_with_no_length_never_takes_a_backup_or_a_marker (2574) | Other fault (name does not match) | "Name claims more than the checks"; "Add the backup check" | -
test_an_ordinary_adopted_backup_is_restored_at_shutdown (2645) | Other fault (name does not match) | "the name says the opposite of what it checks"; "Rename" | -
test_the_command_carries_this_worker_and_the_songs_run_cap (2937) | Other fault (Q4/5) | "64.0 fails Q4/Q5: nobody can tell what 64.0 is" | -
test_a_run_with_no_song_gets_the_cap_with_no_folder (2948) | Other fault (name does not match) | "The name says 'with no folder': no assertion about a folder" | -
test_the_planned_ends_are_the_schedules_on_each_chord_read_after (3247) | Other fault (Q4/5, with Q3) | "the file is the source; the test types a copy"; "Raw results with 14 digits" | none named (Freak is not in records-engine)
test_the_probe_moved_chords_reach_the_agent_as_play_entries_at_literal_times (3357) | Other fault (Q4) | "four typed numbers ... Name the inputs ... and write the expected as `chart_ms - aim + lead_shift`" | -
test_a_result_with_no_rows_of_its_own_counts_the_drop_as_logged (3436) | Other fault (name does not match) | "nothing asserts a count or the dropped line" | -
test_the_same_block_on_the_player_thread_still_drops (3550) | Other fault (Q9) | "`> 0` ... says 'some', not which chord" | -

## D6

test_the_end_is_the_number_as_written (:123) | Other fault (Q9, with Q3) | "the input is a whole number, so the check does not aim at the real risk (truncation)". Verify agrees on Rewrite and says the proposed shape "is a copy" | none named (real dumps in records-engine hold 26812.5 and 68916.66666666667)
test_chart_song_length_ms_reads_the_dump_it_made (:144) | Other fault (Q4/9, with Q3) | "(b) leans on `prep_inputs.file_names` naming; fine to drop"; verify: "(c) at :158 compares with the function that built the text, and (d) retypes the sentence" | none named (a real dump of one corpus chart)
test_it_patches_the_game_assembly_module (:84) | Other fault (Q9) | "file value equals a typed copy of the value. A copy." | -
test_expect_is_mov_edx_1 (:97) | Other fault (Q9). Verify disputes: "Verdict should be Investigate, claim not checked only" (the bytes exist only in the game's DLL) | "json `expect` equals a typed copy"; "Decode `expect` from the listing" | the game's DLL bytes at the patch RVA (IN-81 names a tool), not in the repo
test_the_loader_rva_has_a_method_row_in_the_facts_table (:122) | Other fault (Q10) | "the second line duplicates :93"; "so one line holds" | -
test_the_listing_reads_mov_edx_1_at_the_patch_rva (:128) | Other fault (Q9/10) | "The listing line is typed, not built from the json's rva"; "The second duplicates :93" | -
test_the_listing_opens_with_the_instructions_entry_expect_encodes (:132) | Other fault (Q9). Verify disputes: "Verdict should be Investigate, claim not checked only (needs the DLL's first twelve bytes read by a tool)" | "the name promises 'encodes'; the test checks two strings separately" | the DLL's first twelve bytes, not in the repo
test_the_data_file_reads_as_a_patch (:238) | Other fault (Q5/9) | "The module line is true by construction"; "`launch == "suspended"` ... no reason in the test" | -
test_a_clean_probe_prints_each_number_and_passes (:1067) | Other fault (Q4) | "One list equality of twelve typed lines"; verify: "The fault is in the test (a retyped printout), not only question 3" | -

## D7

test_the_parser_reads_the_captured_table (:184) | Other fault (Q4/5) | "The typed process-field numbers are unexplained snapshots of the fixture. Fix: read each from the sample bytes" | -
test_the_filter_keeps_counts_for_the_whole_table (:237) | Other fault (Q4) | "506 is `IDLE_THREADS + SYSTEM_THREADS`"; "Write 506 as `IDLE_THREADS + SYSTEM_THREADS`" | -
test_an_empty_filter_parses_no_threads (:243) | Other fault (Q4) | "same fix as :237 (506 from the two named counts)" | -
test_gains_and_a_vanished_process (:311) | Other fault (Q4, with Q3) | "15.625 raw (= (306_250 - 150_000) / 10_000 ticks per ms; working not shown)"; "Give each expected gain as the named input difference" | none named (two successive real tables from a real recording)
test_until_the_pid_line_no_process_is_left_out_of_the_gains (:541) | Other fault (Q4) | "1.0 ms is `10_000` ticks ... unnamed in the test; raw result" | -
test_a_short_clock_read_leaves_the_column_empty (:583) DEAD | Other fault (dead test, never runs) | "this function is replaced by the same-named function at :682, so Python never runs it"; verify disputes: "Verdict should be Remove" | -
test_each_row_carries_the_pid_and_period_and_only_the_last_the_process_time (:644) | Other fault (Q4) | "15.625 (raw; it is `C.s_to_ms(PERIOD_S)`)"; "Write 15.625 as `C.s_to_ms(PERIOD_S)`" | -
test_a_table_that_outgrows_the_buffer_grows_it_and_keeps_it (:761) | Other fault (Q9) | "a copy of the production growth"; "No schedule typed" | -
test_the_rate_defaults_to_the_one_constant (:819) | Other fault (Q4) | "raw: 1 / `RATE_PER_S`"; "Say `1 / M.RATE_PER_S`" | -
test_run_cap_s_adds_the_extra (:825) | Other fault (Q4) | "raw: 100 + `RECORDER_CAP_EXTRA_S`" | -
test_it_imports_neither_the_game_tools_nor_the_watch (:845) | Other fault (Q9) | "Two named modules stand in for 'the recorder stays light'" | -
test_a_one_second_run_on_this_process (:865) | Other fault (Q5/9) | "20 and 0.95 have no reason. 9 floors." | -
test_only_true_counts_as_active (:448) | Other fault (Q4/9) | "`[1]` and `"true"` cannot come from `agent/menu.ts`"; "drop ... or say what bad agent they stand for" | -
test_it_names_the_two_manual_pages_and_only_repo_files_that_exist (:491) | Other fault (Q9) | "a snapshot of the file's citation count"; "Drop the count of two" | -

## D8

test_the_start_sends_each_classes_skips_and_assembly (:234) | Other fault (Q4/9/10) | "line 241 `"UnityEngine.CoreModule"` is a typed copy"; "lines 238 and 240 restate test_an_all_class_skips_..." | -
test_the_start_refuses_without_the_builds_facts (:243) | Other fault (Q9) | "a copy of the message text which `build_facts.UnknownBuild` ... owns"; "not a second typed copy" | -
test_a_stale_agent_starts_no_watch (:261) | Other fault (Q9) | "whole reply typed (a copy of `build_stamp`'s stale message)" | -
test_listed_classes_send_their_rvas_and_by_name_classes_their_names (:341) | Other fault (Q4/9) | "every value is a typed copy of start_candidates.json" | -
test_shared_addresses_are_those_more_than_one_class_runs (:354) | Other fault (Q4) | "set of three raw addresses ... the line does not [explain]"; "Name the fixture's addresses" | -
test_an_all_class_skips_its_shared_addresses_and_a_listed_one_does_not (:357) | Other fault (Q4) | "Raw hex as in :354" | -
test_only_scene_manager_names_an_assembly (:366) | Other fault (Q9/10) | "whole dict typed: a copy of the candidates file" | -
test_only_scene_manager_is_a_scene_loader (:385) | Other fault (Q9) | "typed copy of the candidates file's scene flag" | -
test_the_addresses_each_class_skips_in_this_build (:442) | Other fault (Q5/9) | "a snapshot of the dump"; "no reason: the numbers are what the code printed for this build" | -
test_top_level_keys_and_version (test_mono_facts.py:92) | Other fault (Q9, with Q3) | "`len == 9`: a count (stand-in)"; "typed copy". Verify: "line 95 types the join" | none named (real input, "S-MF3")
test_signatures_name_the_three_readable_classes_only (:145) | Other fault (Q5/9, with Q3) | "counts pass with wrong items"; "the three names are `dump_facts.NAMED_SIGNATURES`; the test types them" | none named
test_a_missing_assembly_is_named (:162) | Other fault (Q9) | "the last entry of the production list, copied (stand-in)" | -
test_the_decided_numbers (:160) | Other fault (Q5/9) | "All typed copies of the production constants"; verify: "no number here has a filed ruling" | -
test_a_folder_stops_at_its_first_recent_file_and_is_still_recent (:327) | Other fault (Q4) | "written as a raw 21"; "Write 21 as `1 + 20`" | -
test_the_command_line_is_the_fixed_contract (:122) | Other fault (Q9) | "whole command typed: a copy of the production command" | -
test_build_server_registers_the_thirty_seven_tools (test_server.py:65) | Other fault (Q5/name) | "the list is a copy of the registry"; "the set holds 38" while the name says thirty-seven | -
test_open_items_uses_public_checks (:121) | Other fault (Q9) | "A source-text search, a stand-in for the property" | -
test_a_long_reply_is_cut_at_a_line_with_the_clone_hero_closing_line (:150) | Other fault (Q5) | "a typed copy of `REPLY_CAP_CHARS` formatted" | -
test_ch_kit_press_says_what_value_does (:369) | Other fault (Q9) | "A phrase check; it stands in for 'every argument is described'" | -

## D9

test_no_song_folder_takes_the_watcher_cap (test_sp_tools.py:290) | Other fault (Q5) | "a copy of the constant's value"; "Build the reply text from `sp_watch.RUN_CAP_S` instead of typing 240" | -
test_ch_sp_prints_the_latest_read (:324) | Other fault (Q4/5, with Q3) | "5760 copies the owner's value"; "61.25 s minus 12.5 s ... typed as 48750.0" | none named (a real read)
test_watch_for_run_stops_a_search_that_outlasts_its_cap_and_says_so (:413) | Other fault (Q4, timing) | "a typed result of the sum"; "0.05 s waits can flake on a busy machine" | -
test_ch_sp_watch_status_lists_activations_and_rises (:465) | Other fault (Q4/5, with Q3) | "typed copy of ENGINE"; "Write the address from `ENGINE`" | none named
test_the_real_ls_o2_gap_line (:526) | Other fault (Q4/5/10) | "snapshot numbers from one run"; verify: the same real CSV and four numbers are pinned at `test_sp_compare.py:874-883`, Rewrite to Remove | the real CSV `testdata/ch_watch/ls-o2-run1-sp_watch-1.csv` is used (Q3 is met)
test_the_real_game_tools_holds_the_run_the_song_end_reads (:631) | Other fault (name pin) | "an implementation-name pin ... the claim is 'the field is called `_run`', which is a code name" | -
test_a_gap_of_exactly_gap_ms_is_not_printed (:655) | Other fault (Q5/10) | "a copy of the constant"; verify: boundary tested on the owner, Rewrite to Remove | -
test_ready_when_comes_from_the_schedule_and_whether_from_the_games_half (:711) | Other fault (Q4/9, with Q3) | "a scan of sp_tools.py source for seven words"; "the Schedule here is hand-built ... Claim not checked" | none named (a real chart schedule)
test_run_cap_line_says_default_when_no_song_length (:735) | Other fault (Q5) | "a copy of the default cap"; "Build 240 from `sp_watch.RUN_CAP_S`" | -
test_ch_sp_compare_on_mouths_reports_as_compare_does (:766) | Other fault (Q9) | "two functions compared with each other is a Q9 stand-in" | -
test_the_song_cap_is_hydras_end_plus_the_margin (test_trace_hits.py:110) | Other fault (Q4/5) | "typed 90.08 contains the margin (30 s) as a number the test never names: a raw result" | -
test_the_wall_cap_is_the_song_cap_plus_the_pause_room (:113) | Other fault (Q4/5) | "typed 210.08: raw result, the 120 s pause room unnamed. Same fault as 110" | -
test_count_mode_posts_the_settings_and_prints_the_table (:171) | Other fault (Q4/5, with Q3) | "typed caps"; "one test holds ten claims" | none named
test_a_stale_agent_runs_nothing (:318) | Other fault (Q5/9) | "a copy of the owner's message (build_stamp)" | -
test_only_gamemanager_ts_looks_up_the_game_manager (:340) | Other fault (Q9) | "only a re-added lookup under those two spellings fails"; "Move into the single-owner scan" | -
test_the_agent_gets_mains_args_mode_settings (:458) | Other fault (Q5, with Q3) | "the test never compares with main's, it repeats main's literals: a copy" | none named (real thread, fake frida)
test_the_wall_cap_still_ends_a_run_nobody_stops (:544) | Other fault (Q5) | "157.0 (typed, past the typed 156 s cap: a copy of the cap sum)" | -
test_wall_stamp_is_to_the_millisecond (test_cheap_checks.py:102) | Other fault (Q8) | "it cannot tell rounding from truncation"; "Use 123999 µs so truncation and rounding differ" | -
test_prints_new_lines_whole_and_stops_at_the_cap (:109) | Other fault (Q5, with Q3) | "'300 s' typed in the text (copy of `TAIL_CAP_S`)"; "Claim not checked also" | none named (a real tail capture)
test_no_song_exits_2_with_the_finders_message (:255) | Other fault (Q5) | "a copy of the finder's message"; "Take the text from the finder's constant" | -
test_five_cases_from_the_plan (test_early_fill_walk.py:57) | Other fault (Q4/5) | "a copy of `P.EFW_CASES`: the plan's numbers, retyped"; "5 reason: not opened" | -
test_each_phrase_holds_exactly_its_one_note (:72) | Other fault (Q9) | "the test applies SP-01 itself in line 75: a copy of the rule written in the test" | -
test_each_fill_ends_on_a_lone_kick (:81) | Other fault (Q9) | "The rule application is a copy of FL-03/05 as the test understands them" | -
test_sp_ready_notes_are_a_second_from_both_neighbours (:104) | Other fault (name does not match / Q5) | "a name that claims seconds, a check in ticks"; "Reason not stated anywhere I read" | -
test_manifest_cases_match_the_chart (:127) | Other fault (Q5) | "Drop the retyped 1920 (use the engine's deadline)" | -
test_the_song_is_listed (:149) | Other fault (Q9) | "Keep the list check (151) and drop the name pin (150)" | -
test_a_plain_chord_of_a_real_schedule_is_pressed_at_its_lead (test_edge_probe.py:125) | Other fault (Q4/5, with Q3) | "a number copied from the file at write time ... a long float is a raw result"; "Read the chord's ms from the loaded schedule inside the test instead of typing it" | the real Freak schedule is used, but "the file's chord 386 ... were not opened"

## D12

test_the_play_message_carries_the_schedule_and_every_number (test_play_in_game.py:107) | Other fault (Q4/5, with Q3) | "5.0/150.0/5000.0/1000.0 have no reason in the test; the test copies constants"; "Expect each setting from its named owner" | none named
test_a_sent_row_and_a_dropped_row_fill_every_column_score_compare_reads (:124) | Other fault (Q5, with Q3) | "the six typed column names ... Take the column list from `overlay_logs.PRESS_LOG_COLUMNS`" | none named
test_an_end_read_posts_one_retime_of_retime_from_ends_output (:155) | Other fault (Q9/B11) | "the expected message is built by calling the production `P.retime_from_end` ... (copy, rubric B11)" | none named
test_an_end_read_with_star_power_off_still_posts_a_retime (:198) | Other fault (Q9) | "built with the same production code that makes the posted message (copy)" | -
test_each_agent_stop_reason_gives_its_stop_word (:273) | Other fault (Q9) | "the production dictionary ... typed again: a copy that agrees with any bug" | -
test_a_split_chords_rows_match_play_chords_rows_but_the_timing_columns (:342) | Other fault (Q9, two claims) | "Two Hydra routes compared with each other where neither is checked against a rule" | -
test_prep_inputs_runs_three_commands_and_dry_loads_the_schedule (:94) | Other fault (Q5) | "1170 and 624 are what the fixture holds today; nothing in the test says why those" | -
test_the_default_exe_is_this_checkouts_release_build (:164) | Other fault (Q9, with Q3) | "a copy of the constant's own expression"; "Claim not checked" | none named (the build script's output folder)
test_one_second_at_various_bpm (:50) | Other fault (Q4) | "raw results; the working is in the comment above ... not on the lines" | -
test_rounds_to_nearest (:55) | Other fault (Q4) | "12 means nothing without the comment" | -
test_braces_balanced (:96) | Other fault (Q9) | "a count standing in for 'the chart parses'" | -
test_pairs_do_not_overlap (:122) | Other fault (Q5) | "The real risk ... up to 170 ms"; "Require the silence in ms to be at least `C.CAP_FROM_GAP_MS`" | -
test_probe_note_ticks_are_the_written_ticks (:149) | Other fault (Q4) | "raw results; the working ... is in a comment" | -
test_chart_ticks_match_manifest_times (:38) | Other fault (Q5, with Q3) | "three format lines typed ... copies"; "drop 54 or say why the first note is at 3000" | none named
test_song_names_are_spelled_once (:95) | Other fault (name does not match / Q9) | "The name says 'spelled once', which the test does not check" | -
test_exact_values_pass (:130) | Other fault (Q4/5) | "the rule numbers have no page or rule id beside them in the test" | -
test_back_off_by_one_fails (:139) | Other fault (Q4) | "a raw result; its working is in a comment" | -
test_front_off_fails (:144) | Other fault (Q4) | "Name the 2.5 ms offset" | -
test_drifted_back_constant_raises (:173) | Other fault (Q4) | "the same raw 1 ms offset as `:139`" | -
test_read_only_asks_for_read_and_query_rights (test_process_readonly.py:30) | Other fault (Q4) | "a raw result ... named in the test only by its name" | -
test_full_open_keeps_the_four_rights (:33) | Other fault (Q4) | "raw result"; "name the four rights" | -
test_derived_rules_come_from_the_per_side_constants (:39) | Other fault (Q4/5) | "put TM-19/TM-21 beside the two numbers"; verify disputes the suggested fix | -
test_normal_back_ms_is_derived_from_the_seconds_value (:52) | Other fault (Q4/5, with Q3) | "85 and 0.085 are TM-18's back clamp ..., not cited in the test"; "cite TM-18 beside the numbers" | none named
test_the_real_capped_run_reads_as_capped_not_unclamped (:60) | Other fault (Q6) | "The expected side is typed text of the tool's own report"; "(rubric question 6)" | the committed CSV is real input (Q3 is met)
test_a_reading_past_the_cap_is_no_clamp (:67) | Other fault (Q6) | "same fix (facts, not wording)" | -
test_precision_mode_says_the_cap_is_unknown (:71) | Other fault (Q6/5) | "same fix; cite TM-18 for 40.0" | -
test_floor_runs_are_checked_against_the_measured_floor (:77) | Other fault (Q5) | "`75` typed (copy of `FLOOR_UP_TO_GAP_MS`)" | -
test_a_chord_passes_only_when_the_step_equals_hydras_points (:77) | Other fault (Q9) | "the test reads the source text of `score_c...`"; "Drop 84-89" | -
test_the_route_words_are_game_tools (:119) | Other fault (Q9/10) | "Two Hydra modules compared where each holds its own copy" | -
test_clock_line_wording (:153) | Other fault (Q4/9) | "A snapshot of wording"; "the whole line typed" | -
test_median_and_percentiles (:159) | Other fault (Q4) | "raw results"; verify: the record's "method not stated" is wrong, "Verdict Rewrite stays (raw results, no working on the line)" | -
test_the_retime_word_play_chart_writes_reads_back (:257) | Other fault (Q5) | "typed literal ... pins the constant's value rather than reading the owner" | -

## D13

test_watch_checks.py (section heading; the row covers every Rewrite verdict inside the section, tests 90 to 96 and the last one at record line 592) | Other fault (wrong rule: the claim contradicts the Clone Hero page) | "These tests check that the engine's field at +0x2E0 ... changes at each chord ... `docs/clone-hero/03-timing.md` TM-14 ... says the opposite"; "Verdict: Rewrite. The expected side comes from TM-14"; tests 91 to 96: "Rewrite, as 90". The other tests in the section (97 to 101, 102 on) are marked Investigate with "claim not checked only", not Rewrite | none (not a Q3-only row)

## D15

BuildPassTool (real `build_slot.ps1` on a temp slot folder; the row covers records 47, 51 and 52, the Rewrite verdicts in that section) | Other fault (Q5/9: copy of the script's wording) | 47: "Copy of the script's wording. See BP-3. Verdict: Rewrite. Same fix as BP-3: check the command carries the token the first line names, not the full typed command"; 51: "Typed name copies `SESSION_ENV` (line 89). Verdict: Rewrite"; 52: "Same wording copy as 51. Verdict: Rewrite (same fix, merge with 51)" | none (not a Q3-only row)

## E3

File 1: test_refusals.py (88 lines, 5 cases; the row covers the one Rewrite in that section, test_deny_writes_one_line) | Other fault (Q5/9) | "300 has no source ... the `[:300]` in the test is a copy of production"; "hook-name check compares production with itself"; "Verdict: Rewrite. Check the logged reason is the start of the full reason ... do not retype 300 or recompute argv". The record also says "3 Claim not checked: payload is hand-typed", but Q3 is not the only fault | none named (a real recorded deny payload through a real hook)

## D14

test_tables_print_in_the_fixed_order (:216) | Other fault (Q5/9) | "a copy of `TABLES`"; "a snapshot of the constant (rubric: a copy is not a reason)"; verify: the record's rewrite "is circular" | -
test_the_brief_names_the_three_inputs_and_the_labels (:170) | Other fault (Q9) | "copies of the brief's own words ... a snapshot of a template" | -

## E1

test_planner_asks_for_the_premise_map (test_agent_files.py:44) | Other fault (Q9) | "the strings are the test's own copy of what the file says"; "drop the typed word list". Verify disputes the rewrite | -
test_reviewer_checks_the_premise_map (:47) | Other fault (Q9) | "phrases ... are copied wording" | -
test_finished_flag_written_when_no_record_exists (test_agent_stop.py:171) | Other fault (Q9) per the record ("Stand-in: whole-dict equality"). Verify disputes: "The fault is q3 only (hand-typed), so Investigate" | "Check `finished` is True; do not pin the full dict" | none named (the verify file says it is "hand-typed")
test_reviewer_commits_on_two_diverged_branches_record_the_handed_tip_with_a_problem (:223) | Other fault (Q9) | "message text pins (wording copy of agent_stop.py:93)" | -
test_transcript_whose_first_user_record_is_not_the_brief_writes_nothing (:343) | Other fault (name/story does not match the code) | "`first_user_message` does not read isMeta at all"; verify: "Dispute Rewrite ... the outcome ... is the same promise" | none named (hand-typed transcript)
test_no_saved_handback_gives_no_verdict_even_with_one_in_the_transcript (:384) | Other fault (Q9) | "Wording pins on 390 and 393 (copy of agent_stop.py:124)" | -
test_recorded_verdict_logs_the_tip_and_the_fingerprinted_paths (:442) | Other fault (Q9) | "lines pin the whole log shape (a snapshot of keys)" | -
test_no_verdict_logs_its_reason (:454) | Other fault (Q9) | "'no handover record matches its brief' (wording copy)" | -
test_unwritable_log_still_records_the_verdict_and_says_so (:482) | Other fault (Q9) | "assert the message is non-empty without the phrases" (wording pins) | -
test_two_concurrent_writers_never_leave_a_torn_file (test_common.py:104) | Other fault (Q8) | "the loop may read nothing at all and still pass"; "require `reads > 0`". Verify: the record's reasoning is wrong in one place, verdict agreed | -
test_an_agents_call_is_refused_and_says_the_main_session_builds (test_percall_key.py:196) | Other fault (Q9) | verify: "phrase pin at 199 copies `percall.py:109`" | -
test_one_hook_after_a_tool_call (test_docs_numbers.py:151) | Other fault (Q5) | "timeout equals 10 (no reason found for 10)"; "drop the timeout number and matcher literal or cite their reason" | -
test_readme_names_new_records (test_docs_numbers.py:170) | Other fault (Q9) | "mention stands in for 'documented'"; "derive the names from the hooks that own them" | -
test_the_refusal_names_the_write_tool_and_no_commit_route (test_bash_windows_rule.py:59) | Other fault (Q9) | "Wording pins on the production message (copy of :212-214)"; "the two negatives guard old text, not a behaviour" | -

## E2

test_a_build_gets_the_first_call_notice_and_the_rewrite_in_one_answer (test_percall_rewrite.py:52) | Other fault (Q5/9, with Q3) | "`BASH_TAIL` and the notice wording are copies of strings owned by shell_output.py and progress.py"; "Plus claim not checked on a recorded build call" | none named
test_a_call_past_the_budget_is_not_rewritten (:77) | Other fault (Q5) | "10_000 is a free input but is not named as 'past `progress.CALL_STOP`'" | -
test_a_build_with_nothing_else_to_say_gets_a_new_answer (:114) | Other fault (Q9) | "whole dict equality with `BUILD + BASH_TAIL` typed (copy of owner's answer)" | -
test_the_rewrite_rides_beside_a_stall_notice (:118) | Other fault (Q9) | "whole-dict equality is a snapshot of the exact reply layout" | -
test_the_rewrite_sits_beside_stalls_crash_message (:124) | Other fault (Q9) | "snapshot of layout" | -
test_skip_does_not_push_the_next_send_back (test_posture.py:77) | Other fault (Q5) | "use `RESEND_MINUTES`, not 61" | -
test_call_100_warns_once (test_progress.py:166) | Other fault (Q5) | "98 and the text; rule 100 not found in rulings". Verify disputes: "Verdict Investigate: question 5 has no Confirmed ruling for 100 ... and claim not checked" | none named
test_call_150_denies_read (:174) | Other fault (Q5) | "seed from `progress.CALL_STOP - 1`". Verify disputes: "150 unruled; Investigate" | none named
test_call_149_still_passes (:178) | Other fault (Q5) | "from `progress.CALL_STOP - 2`". Verify: "dispute, same" | none named
test_call_150_lets_report_git_and_status_through (:186) | Other fault (Q5, with Q3) | "seed from `CALL_STOP`; pass list from `progress.REPORT_TOOLS` ... Also claim not checked". Verify: "Investigate (150 unruled, claim not checked)" | none named
test_minute_20_warns_once (:196) | Other fault (Q5) | "take 20 from `progress.MINUTES_WARN`". Verify: "20 has a Confirmed ruling ... remedy wrong; Investigate, claim not checked only, cite the ruling beside the number" | none named
test_minute_19_is_silent (:205) | Other fault (Q5) | "build from `MINUTES_WARN*60 - 1`". Verify: "dispute, same" | none named
test_minute_30_minus_a_second_passes (:211) | Other fault (Q5) | "from `MINUTES_STOP`". Verify: "dispute, same" | none named
test_minute_30_denies (:217) | Other fault (Q5) | "from `MINUTES_STOP`". Verify: "dispute, same" | none named
test_5_minutes_old_nags (:226) | Other fault (Q5) | "time from `STATUS_NAG_MINUTES`". Verify: "5 has Confirmed ruling at review-and-hooks.md:34 ...; Investigate, claim not checked only" | none named
test_under_5_minutes_is_silent (:233) | Other fault (Q5) | "from `STATUS_NAG_MINUTES*60 - 1`". Verify: "dispute, same" | none named
test_just_under_10_minutes_nags_but_passes (:238) | Other fault (Q5) | "from `STATUS_DENY_MINUTES`". Verify: "dispute, same" | none named
test_10_minutes_old_denies (:251) | Other fault (Q5) | "from `STATUS_DENY_MINUTES`". Verify: "dispute, same" | none named
test_no_status_file_counts_from_first_call (:258) | Other fault (Q5) | "from `STATUS_DENY_MINUTES`". Verify: "dispute, same" | none named
test_status_helper_passes_when_overdue (:262) | Other fault (Q5) | "time from `STATUS_DENY_MINUTES`". Verify: "dispute, same" | none named
test_fresh_status_file_resets (:266) | Other fault (Q5) | "from `STATUS_DENY_MINUTES`". Verify: "dispute, same ... it still has a hand-typed payload" | none named
test_shell_call_runs_the_real_rewrite_without_crashing (:300) | Other fault (Q9) | "the absence of one word stands for 'worked'"; verify agrees: "cannot tell 'rewrite ran and did nothing' from 'rewrite never ran'" | -
test_progress_deny_wins (:318) | Other fault (Q5/8) | "Seed from `CALL_STOP - 1`"; verify: line 320 stays green if the early return is deleted, "the test does not pin 'deny wins'" | -
test_out_of_budget_subagent_keeps_its_allow (:566) | Other fault (Q5, with Q3) | "seed `progress.CALL_STOP`. Also claim not checked". Verify: "Investigate: 150 unruled and claim not checked; remedy copies the constant" | none named
test_chained_status_call_past_budget_is_refused (:728) | Other fault (Q5, with Q3) | "seed `progress.CALL_STOP`; claim not checked". Verify: "Investigate (150 unruled, claim not checked)" | none named
test_plain_status_call_past_budget_passes (:732) | Other fault (Q5/10, with Q3) | "seed `CALL_STOP`; claim not checked". Verify: "Remove, or Investigate" (duplicate of tests 186 and 153) | none named
test_refusal_names_the_commit_and_review_route (test_publish.py:233) | Other fault (Q9) | "a snapshot of wording"; "typed copy of `publish_gate.py:104-105`" | -
test_key_flag_is_gone_and_so_is_doc_key_ps1 (:240) | Other fault (Q9) | "usage string fails on any edit: a snapshot"; "drop the typed usage string" | -

## E4

ApprovalTests.test_pointer_whose_transcript_is_gone_refused (test_ruling_rules.py:267) | Other fault (name does not match / Q8) | "The test cannot fail on a pointer bug. 9: name does not match body." | -
EmptyFleet.test_exactly_five_minutes_warns (test_stall.py:279) | Other fault (Q9) | "copy of the owner's text as the expected value" | -
EmptyFleet.test_spell_starts_at_newest_agent_call (:284) | Other fault (Q9) | "same copy of the owner's text as 279" | -
EmptyFleet.test_open_journal_is_the_target (:290) | Other fault (Q9, with Q3) | "Rewrite (copy of owner text; plus claim not checked)" | none named (a real Workflow `journal.jsonl`)
EmptyFleet.test_journal_from_an_earlier_run_does_not_count (:310) | Other fault (Q9) | "Rewrite (copy of owner text)" | -
EmptyFleet.test_warns_once_per_quiet_spell (:321) | Other fault (Q9) | "copy of owner text at 328; the other lines fine" | -
EmptyFleet.test_new_launch_starts_a_new_spell (:331) | Other fault (Q9) | "copy of owner text at 336" | -
EmptyFleet.test_fleet_paragraph_comes_before_stuck_agents (:338) | Other fault (Q9) | "copy of owner text for the first part" | -
EmptyFleet.test_launch_inside_the_scan_throttle_is_still_stamped (:344) | Other fault (Q9) | "full text with 16 minutes (copy of owner text)" | -
Rewrite.test_tails_are_the_designed_text (test_shell_output.py:49) | Other fault (Q9) | "retyped copies of the owner's constants ... it cannot catch a bug" | -
Rewrite.test_the_delete_rule_still_does_not_read_a_quoted_word (:99) | Other fault (Q9/10) | "a test of another module's output, in this file"; "Move it to `test_delete_rule.py`" | -
Trim.test_a_result_of_exactly_the_cap_is_left_alone (:142) | Other fault (Q5) | "4000 retyped from `CAP` (copy)" | -
Trim.test_one_over_the_cap_shows_start_error_lines_end_and_path (:145) | Other fault (Q4/5) | "the name is a number pinned from one run; 1500 is `HEAD_CHARS` retyped"; "the hash name is a snapshot" | -
Trim.test_error_lines_show_in_a_trim_with_the_over_long_one_cut (:198) | Other fault (Q9/10) | "copy of the owner's format and a pinned count"; "overlaps 145 and 186" | -
Trim.test_each_error_word_counts (:219) | Other fault (Q5/9) | "in a loop with no subTest"; "copy of the owner's list" | -
Trim.test_persisted_result_that_failed_reads_that_file_and_saves_nothing_new (:248) | Other fault (Q4) | "pinned count at 255; take the length from the written text" | -
Trim.test_a_long_failed_result_saves_the_failed_line_first (:320) | Other fault (Q5) | "pinned hash and count; derive the path with `saved_path`" | -
Trim.test_a_persisted_no_trim_failure_is_still_trimmed (:357) | Other fault (Q4) | "pinned count" | -
SavedPath.test_the_scratch_field_wins (:375) | Other fault (Q5) | "a pinned hash ... a snapshot" | -
SavedPath.test_with_no_scratch_field_it_is_claude_codes_folder_for_the_transcript (:379) | Other fault (Q5) | "Pinned hash. ... same as 375" | -
SavedPath.test_an_empty_scratch_field_counts_as_absent (:383) | Other fault (Q5) | "Pinned hash. ... same as 375" | -
Check 'the note names the card-writer agent and never-run-check' (test_h53_session_cards_start.ps1:137) | Other fault (Q9) | "copy of the owner's wording" | -
Check 'the note says the agents write to staging and check.py moves passing cards ...' (:138) | Other fault (Q9) | "same cause as 137" | -
Check 'the commit step stages by name, on main, with the three trailers, and needs no reviewer' (:140) | Other fault (Q5/9) | "'needs no reviewer' ... not checked against the ref gate"; "copy of the owner's wording" | -
Check 'the commit step confirms the branch and commits only docs/sessions' (:145) | Other fault (Q9) | "copy" | -
Check 'the note says not to hold up the user's request' (:146) | Other fault (Q9) | "copy" | -
Check 'sessions waiting but no batch file gives a systemMessage' (:195) | Other fault (Q9) | "same cause as 137; the condition under test is real" | -
Check 'and the hook returns long before the script would have' (:212) | Other fault (Q5, timing) | "30 s has no reason given"; "timing on a busy machine" | -
Check 'and batch.py, with the budget spent, is not started and no note is given' (:214) | Other fault (Q9) | "copy of wording; behaviour real" | -
Check 'at the threshold: the reminder sentence, with the list path' (:236) | Other fault (Q9) | "a retyped whole sentence (copy of the owner's message)" | -
Check 'and the note names that folder and passes it to check, index and link_memory' (:266) | Other fault (Q9) | "retyped command shapes (copies)" | -
Check 'the main checkout is untouched' (test_h52_cleanup_sweep.ps1:180) | Other fault (Q8) | "weak: only one file's existence" | -
Check 'the report names each removed worktree' (:205) | Other fault (Q4/9) | "retyped phrases (a count 3 follows from the fixture)"; "copy of the owner's wording" | -
Check 'the report counts the scratch entries, folders and files together' (:206) | Other fault (Q4/5) | "a pinned count ... the working is not in the test" | -
Check 'the report gives errors without PowerShell's wrapper text' (:208) | Other fault (Q8) | "passes whenever no error is reported at all; a weak negative" | -
Check 'and the report says why' (:274) | Other fault (Q9) | "copy of wording; the reason is real" | -
Check 'the background worker finished within 60 s and wrote its report' (:291) | Other fault (Q5, timing) | "60 s: no reason for the number is written" | -
Check 'the test's writer finished within 120 s' (:412) | Other fault (Q5) | "a timeout with no reason; same cause as 291" | -
Check 'the report counts the pruned state records and log lines' (:451) | Other fault (Q4/5) | "pinned counts: the working is not written" | -
Check 'and names no failure' (:452) | Other fault (Q8) | "passes whenever the report is silent; weak negative" | -
Check 'and the report says which sessions still run could not be read' (:470) | Other fault (Q9) | "copy of wording; behaviour real" | -
Check 'a broken input helper gives a systemMessage' (:479) | Other fault (Q9) | "copy of wording; behaviour real" | -

## Count table

| Records file | Rewrite rows | Q3 only | Other fault | Unclear |
|---|---|---|---|---|
| D2 | 29 | 0 | 29 | 0 |
| D3 | 23 | 0 | 23 | 0 |
| D4 | 19 | 0 | 19 | 0 |
| D5 | 16 | 0 | 16 | 0 |
| D6 | 9 | 0 | 9 | 0 |
| D7 | 14 | 0 | 14 | 0 |
| D8 | 19 | 0 | 19 | 0 |
| D9 | 27 | 0 | 27 | 0 |
| D12 | 32 | 0 | 32 | 0 |
| D13 | 1 | 0 | 1 | 0 |
| D14 | 2 | 0 | 2 | 0 |
| D15 | 1 | 0 | 1 | 0 |
| E1 | 14 | 0 | 14 | 0 |
| E2 | 28 | 0 | 28 | 0 |
| E3 | 1 | 0 | 1 | 0 |
| E4 | 42 | 0 | 42 | 0 |
| Total | 277 | 0 | 277 | 0 |
