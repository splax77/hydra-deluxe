Agent addb27fa4ce7eea88 (counter T5-KR3, session 63c70014-4328-49dc-9205-742235732752)

# Keep recount, records D9, D2, D13, D3, D10, D11

Plain version: I opened the test for every Keep record in these six files (84 in all, matching the brief) and read its checks. 54 of the 84 hold at least one fault. Most are made-up input (a fake worker, fake game replies, hand-typed source lines) or a typed message text. Paths are under `tools/ch_mcp/tests/` (D9 sp_tools and trace_hits, D2, D3) and `tools/ch_probe/tests/` (D9 cheap_checks and edge_probe, D10, D11, D13), except D13's agent_cost test in `tools/test_agent_cost.py`.

How I read the "named" column: "yes" means the record text says the same thing in its own words (fake, hand-written, typed, echo). For D9 and D2 I read the record bodies. For D13, D10, D11 and D3 I only searched each record for those words, so "no" there means the word search found nothing, not that I read every line. Line numbers are the test's own lines in the current files; the records' line numbers are sometimes a few lines off, so I matched by test name.

## D9 (21 Keep records)

| # | test | faults | lines | reason | named |
|---|------|--------|-------|--------|-------|
| 1 | test_sp_tools.py:445 watch_for_run_is_refused_while_a_watch_runs | X9 | 446-447 | a scripted fake game feeds the watch (snap_at) | no |
| 2 | test_sp_tools.py:674 ch_sp_compare_names_a_missing_file | X10 | 677 | typed reply prefix "compare failed: FileNotFoundError" | yes |
| 3 | test_trace_hits.py:132 folder_with_no_chart_is_refused_with_no_dump | X10 | 136-138 | whole refusal sentence typed | yes |
| 4 | test_trace_hits.py:265 no_chart_prints_one_line | X10 | 270-272 | whole line typed | yes |
| 5 | test_trace_hits.py:311 missing_agent_says_how_to_build_it | X10 | 316 | typed "not found; " text | no |
| 6 | test_trace_hits.py:327 modes_refuse_wrong_method_flags | none | | argparse refusal on a real command line | |
| 7 | test_trace_hits.py:400 manifest_wins | X11 | 402-404 | notes typed in, same notes required back | no |
| 8 | test_trace_hits.py:406 score_json_is_used | X9, X11 | 390-392, 409 | hand-written score chords, same times required back | no |
| 9 | test_trace_hits.py:411 no_source_names_both | X10 | 416-423 | two full error sentences typed | yes |
| 10 | test_trace_hits.py:470 stop_ends_a_run_that_never_ends | X9, X10 | 471-472, 477 | fake frida session with an invented agent message; typed "ended: not yet..." | yes (fake frida) |
| 11 | test_trace_hits.py:485 calls_grow_as_the_agent_sends_them | X9, X11 | 486-495 | invented hit calls sent in, required back unchanged | no |
| 12 | test_trace_hits.py:498 lined_up_starts_at_since | X9 | 499-509 | invented hit calls from a fake session | no |
| 13 | test_trace_hits.py:511 run_the_agent_ends_keeps_the_agents_end | X9, X10 | 512-516 | fake agent messages; typed "ended: done (agent: song over)" | no |
| 14 | test_trace_hits.py:520 agent_end_that_lands_as_stop | X9, X10 | 523-541 | fake session; typed end line | no |
| 15 | test_trace_hits.py:563 runner_that_raises_ends_as_failed | X9, X11 | 564-568 | invented RuntimeError("script broke") required back as text | yes |
| 16 | test_trace_hits.py:571 missing_agent_says_how_to_build_it (session) | X10 | 577 | typed "not found; " prefix | no |
| 17 | test_trace_hits.py:579 session_needs_a_method | none | | | |
| 18 | test_cheap_checks.py:94 empty_folder_gives_none | none | | real empty folder | |
| 19 | test_edge_probe.py:85 probes_file_is_a_list_of_index_and_lead | X11 | 87-88 | typed 88.72 / -82 required back as Probe values | partly (said "typed values") |
| 20 | test_edge_probe.py:90 probes_file_not_a_list_is_refused | none | | | |
| 21 | test_edge_probe.py:97 chord_named_twice_is_refused | X10 | 99 | typed message regex | no |

## D2 (16 Keep records)

| # | test | faults | lines | reason | named |
|---|------|--------|-------|--------|-------|
| 22 | test_dump_facts.py:280 output_folder_inside_the_repo_is_refused | X10 | 283 | typed message words | yes ("pins wording slightly") |
| 23 | test_frida_attach.py:369 returns_false_when_the_cap_passes | none | | real wait | |
| 24 | test_frida_attach.py:783 blocked_log_write_does_not_hold_up_the_reply | X9, X11 | 784, 796 | invented agent replies; {"pressed": True} required back | yes ("what the game sends is not real") |
| 25 | test_frida_attach.py:811 every_line_is_on_disk_before_detach | X9, X11 | 812-818 | invented messages written and required back unchanged | no |
| 26 | test_frida_attach.py:976 check_catches_every_pattern | X9 | 978-980 | hand-typed source line to the scanner | yes (hand-written) |
| 27 | test_frida_attach.py:985 reads_and_comparisons_pass | X9 | 986-1003 | hand-typed source lines | yes |
| 28 | test_frida_attach.py:1009 no_agent_reads_by_offset | none | | real agent files | |
| 29 | test_frida_attach.py:1039 only_the_listed_agents_hold_a_list | X9, X10 | 1043-1046 | hand-written text; typed finding sentence | yes (X9) |
| 30 | test_frida_attach.py:1048 method_outside_the_list_fails | X9, X10 | 1049-1052 | same | yes (X9) |
| 31 | test_frida_attach.py:1054 name_on_another_files_list_fails | X9, X10 | 1056-1059 | same | yes (X9) |
| 32 | test_frida_attach.py:1104 kit_list_name_the_data_file_lacks_fails | X9, X10 | 1105-1109 | hand-built data and kit text; typed sentence | yes (X9) |
| 33 | test_frida_attach.py:1111 data_file_name_the_kit_list_lacks_fails | X9, X10 | 1112-1116 | same | yes (X9) |
| 34 | test_frida_attach.py:1145 second_native_function_in_kit_ts_fails | X10 | 1150 | typed message tail ": calls a game function: " | no |
| 35 | test_frida_attach.py:1275 a_number_of_its_own_fails | X9, X10 | 1276-1277 | hand-typed block; typed finding text | yes (X9) |
| 36 | test_frida_attach.py:1390 launch_files_are_scanned | none | | real folder listing | |
| 37 | test_frida_attach.py:1404 neither_launch_file_holds_an_address | none | | real files | |

## D13 (15 Keep records)

| # | test | faults | lines | reason | named |
|---|------|--------|-------|--------|-------|
| 38 | test_sp_compare.py:405 sp_max_check_refuses_without_a_resolution | X9, X10 | 406-407 | hand-built rows (build_rows); typed error text | no |
| 39 | test_sp_compare.py:563 main_needs_the_schedule | none | | argparse refusal; rows unused | |
| 40 | test_sp_compare.py:571 main_needs_the_resolution | none | | same | |
| 41 | test_sp_watch.py:92 every_logged_column_is_a_snapshot_field | none | | compares two code lists | |
| 42 | test_sp_watch.py:364 opens_the_game_read_only | X9 | 365-371 | fake process and fake engine search | no |
| 43 | test_stall_harness.py:81 forced_stall_writes_a_dump | none | | real harness on a saved schedule, names read from code | |
| 44 | test_stall_harness.py:90 writer_sink_drops_nothing | X10 | 94-95 | typed report text "dropped 0 chords in all", "press-1.csv (4 rows)" | no |
| 45 | test_stall_harness.py:97 cap_ends_the_run_and_says_so | X10 | 102, 104 | typed "cap reached" and report line | yes |
| 46 | test_stall_harness.py:107 defaults_are_the_users_numbers | none | | reads constants by name (the test name cites "the user's numbers" but the check is on code) | |
| 47 | test_stall_harness.py:119 hunt_run_past_the_most_is_refused | none | | | |
| 48 | test_stall_harness.py:130 missing_cdb_is_said | X10, X11 | 53, 136 | the "WinDbg is not installed" text is typed in the test's stand-in and required back | yes (fake) |
| 49 | test_stall_harness.py:140 second_stall_starts_no_second_cdb | X9, X10, X11 | 144-164 | fake launcher returns "fake cdb stacks", required back; typed "most recent call first" | yes |
| 50 | test_stall_harness.py:166 sleep_counts_only_past_what_it_asked | none | | injected clock only | |
| 51 | test_agent_cost.py:173 agent_without_meta_file_is_unknown_type | X9, X10 | 174-175 | hand-written transcript line; typed "unknown" | no |
| 52 | test_agent_cost.py:196 model_with_no_price_fails_loudly | X9 | 197 | invented model name and transcript | yes (invented) |

## D3 (12 Keep records)

| # | test | faults | lines | reason | named |
|---|------|--------|-------|--------|-------|
| 53 | test_frida_tools.py:279 facts_load_once_per_build | X9 | 280-283 | fake game replies [STATE, STATE] | yes (fake loader) |
| 54 | test_frida_tools.py:481 search_runs_without_the_servers_lock | X9 | 493-496 | invented engine returned by a fake finder | no |
| 55 | test_frida_tools.py:787 every_page_word_has_a_row_rule | none | | two code sets compared | |
| 56 | test_front.py:300 tool_list_and_instructions_come_from_the_worker | X9, X10, X11 | 305-320 | fake worker; typed instructions and tool names required back | yes |
| 57 | test_front.py:322 call_comes_back_as_the_worker_sent_it | X9 | 323-331 | fake worker | yes (echo) |
| 58 | test_front.py:351 replies_find_their_calls_out_of_order | X9, X11 | 357-358 | fake worker replies "slept 1.5" required back | yes |
| 59 | test_front.py:362 mute_worker_ends_at_the_cap | X9, X10 | 363-377 | fake mute worker; typed cap sentences | no |
| 60 | test_front.py:420 two_calls_open_at_a_death | X9, X10 | 421-430 | fake dying worker; typed "exited with code 7" | no |
| 61 | test_front.py:493 reload_reports_pids_times_and_tool_difference | X9, X10 | 494-522 | fake worker settings; typed report lines | yes |
| 62 | test_front.py:533 connection_on_2026_07_28 | X9, X10 | 534-545 | fake worker; typed envelope and sentences | yes |
| 63 | test_front.py:720 shapes_the_hook_cannot_read | X9, X10 | 721-738 | fake worker; typed "refused: ch_call's" | yes (echo) |
| 64 | test_game_files.py:147 backup_refuses_an_existing_folder | none | | real folder collision | |

## D10 (10 Keep records)

| # | test | faults | lines | reason | named |
|---|------|--------|-------|--------|-------|
| 65 | test_engine.py:175 reads_before_capture_raise | none | | guard with no data | |
| 66 | test_engine.py:333 constants_need_no_captured_object | X9 | 334-336 | fake process memory holding every constant | no |
| 67 | test_engine.py:371 use_object_takes_a_scanned_pointer | X9, X11 | 372-377 | fake process; 1.25 typed in, required back | no |
| 68 | test_hit_window_scripts.py:136 block_spans_split_in_the_silence | none | | the window-map chart the tools build | |
| 69 | test_input_driver.py:47 defaults_are_present | none | | | |
| 70 | test_input_driver.py:63 missing_lane_raises | none | | | |
| 71 | test_input_driver.py:98 every_lane_has_a_short_name | X10 | 100 | typed label "Kick" | yes |
| 72 | test_input_driver.py:106 all_down_then_all_up | none | | only the key-send seam is replaced | |
| 73 | test_input_driver.py:140 press_key_sends_down_hold_then_up | none | | | |
| 74 | test_input_driver.py:197 focus_window_skips_a_missing_window | X11 | 201-203 | typed handle 0x1234 required back | no |

## D11 (10 Keep records)

| # | test | faults | lines | reason | named |
|---|------|--------|-------|--------|-------|
| 75 | test_play_chart.py:126 load_schedule_gives_one_press_per_group | none | | compared with the saved schedule file | |
| 76 | test_play_chart.py:163 declined_chord_sends_its_kept_gems | snapshot | 165-170 | chord 51 lanes and kept gems pinned from one fixture, no rule given | no |
| 77 | test_play_chart.py:259 lane_names_come_from_the_key_table | none | | identity with the owner | |
| 78 | test_play_chart.py:263 window_helper_comes_from_input_driver | none | | | |
| 79 | test_play_chart.py:272 songclock_lives_in_live | none | | | |
| 80 | test_play_chart.py:975 every_row_is_on_disk_before_the_writer_closes | X9, snapshot | 976-985 | fake game, fake clock; row indexes "38","41","45" pinned from the fixture | no |
| 81 | test_play_chart.py:990 start_writes_the_file_over_with_the_header | none | | header read from the code's own column list | |
| 82 | test_play_chart.py:1016 log_nowhere_writable_is_kept_as_the_error | none | | | |
| 83 | test_play_chart.py:1337 focus_comes_from_input_driver | none | | | |
| 84 | test_play_chart.py:1544 read_equal_to_aim_end_moves_nothing | X9 | 1545-1547 | made-up end reads fed to a fake game | yes (fake) |

## Counts

- Keep records read: 84 (D9 21, D2 16, D13 15, D3 12, D10 10, D11 10).
- Records with at least one fault: 54 (D9 17, D2 12, D13 8, D3 10, D10 4, D11 3). Records with none: 30.
- By fault (a record can hold more than one): X9 made-up input 37; X10 constant or text pin 31; X11 echo 13; X12 cited non-code 0 (the one candidate, record 46's test name "the user's numbers", checks code, so I did not count it); snapshot 2.
- Records whose fault the record already named: 30 of 54 (D9 7, D2 10, D13 4, D3 7, D10 1, D11 1). The D9 and D2 figures come from reading the records; the others from a word search of the record (see the note at the top), so treat them as a floor.

## Judgment calls

- A fake that stands in for the game, a worker or a transcript counts as X9 even when the check follows correctly from it, as the brief says.
- Typed message or report text counted as X10 only when the check requires that exact sentence. Checks that read a constant or the owner's own function by name did not count.
- The scanner self-tests (D2 26-35) feed hand-typed source lines to a checker that lives in the test file. I counted them X9; the records already said so and excused them as self-tests.

## Not reached

All 84 records were reached.
