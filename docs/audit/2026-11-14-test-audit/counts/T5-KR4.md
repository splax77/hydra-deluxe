ae479e25ff212b1ba (T5-KR4, session 63c70014-4328-49dc-9205-742235732752)

Counts of the Keep records in 15 record files against X9 to X12 and snapshot. I read each test case in the code and did not trust the records' reasons. "Named?" says whether the record's text already named the fault. Borderline calls are marked (b). I read the test bodies but not every helper they call.

What I counted as made-up: a hand-built library row (`fake_charts`, `library_entry`, `timed_library`), a hand-written failure record, a fake analyzer, a canned service reply, a hand-built Activation or press row. A real corpus record or a rule-built path is not made-up.

| File | Record | Test | Faults | Faulty check | Reason | Named? |
|---|---|---|---|---|---|---|
| delta-DL1 | 143 | test_store_records.cpp:143 | none | | real file, real corpus record | |
| delta-DL1 | 157 | :157 | none | | same | |
| delta-DL1 | 188 | :188 | X9 (b) | :195 | the second record's path text "second" is typed in | no |
| delta-DL1 | 215 | :215 | X9 (b) | :88 via save_four_charts | hand-built failure record "No notes." | no |
| delta-DL1 | 231 | :231 | none | | | |
| delta-DL1 | 250 | :250 | none | | | |
| delta-DL1 | 280 | :280 | none | | | |
| delta-DL1 | 352 | :352 | none | | | |
| delta-DL1 | 376 | :376 | none | | | |
| delta-DL1 | 390 | :390 | none | | | |
| delta-DL1 | 416 | :416 | none | | | |
| delta-DL1 | 430 | :430 | X9 (b) | :88 via save_four_charts | hand-built failure record | no |
| delta-DL1 | 503 | :503 | none | | | |
| delta-DL1 | 521 | :521 | none | | | |
| delta-DL4 | 1 | test_library_jobs.cpp:464 | X9 (b) | :464-478 | fake_charts rows and a fake analyzer (the result it returns is a real corpus analysis) | no |
| delta-DL4 | 2 | :481 | X9 (b) | :481 | same | no |
| delta-DL4 | 3 | :498 | X9 (b) | :498 | same | no |
| delta-DL4 | 4 | :524 | X9 | :538-545 | fake analyzer throws made-up errors ("cannot open file: x") | no |
| delta-DL4 | 5 | :560 | X9 (b) | :560-590 | fake rows, fake analyzer; typed ids "fake0" | no |
| delta-DL4 | 7 | :622 | X9 (b) | :622-660 | fake rows, fake analyzer | no |
| delta-DL2 | 13 | test_store_library.cpp:806 | X9 (b) | :808 | hand-built timed_library rows | partly (record says "hand-built") |
| delta-DL2 | 16 | :886 | X9 (b) | :889 | same | no |
| delta-DL2 | 18 | store_file_fixtures.h:22 | none | | helper | |
| delta-DL2 | 19 | store_file_fixtures.h:38 | none | | helper | |
| delta-DL2 | 21 | store_lock_holder.h:61 | none | | helper | |
| delta-DL3 | 21 | test_app_library.cpp:1242 | X9 (b) | :1245 | library_entry(5) rows are hand-built | no |
| delta-DL3 | 22 | :1261 | X9 (b) | :1264 | same | no |
| delta-DL3 | 37 | test_app_reports.cpp:1084 | X9 | :1086 | canned_dm_fetcher is a hand-written dmleaderboards reply | no |
| D4 | test_a_missing_schedule_is_refused | tools/ch_mcp/tests/test_game_tools.py:1154 | X10 (b) | :1159 | expects typed message "could not read ...: FileNotFoundError" | no |
| D4 | test_every_lane_has_a_word_in_the_reply | :1283 | none | | compares two code tables by name | |
| D4 | test_no_file_call_runs_on_the_player_thread | :1548 | X9 | :1588 | hand-written ROWS and a fake live_play | no |
| D4 | test_a_named_log_is_written_row_by_row_too | :1639 | X9 | :1644 | same | no |
| D4 | test_a_run_with_no_press_leaves_a_file_with_the_header | :1666 | X9 (b) | :1667 | fake live_play and RunResult | no |
| D4 | test_the_end_waits_for_the_writer_and_says_so... | :1709 | X9 | :1709-1735 | fake writer, hand-written rows | no |
| D4 | test_a_log_nowhere_writable_is_said... | :1755 | X9 | :1758 | hand-written ROWS, fake live_play | no |
| D1 | test_resume_main_thread_refuses_a_process_with_no_thread | tools/ch_mcp/tests/test_bot_run.py:917 | X10 (b) | :921 | typed message text "has no thread to resume" | no |
| D1 | test_the_toolhelp_and_thread_time_calls... | :924 | none | | real process, real kernel32 | |
| D1 | test_a_spawned_games_process_handle... | :933 | none | | real own process | |
| D1 | test_a_read_only_song_ini_is_marked... | :1224 | X9, X10 | :1225, :1230 | made-up song.ini bytes and a fake ogg writer; typed "Hydra Probe - " prefix in the expected value | no |
| D1 | test_real_6142_facts_give_the_probe_engine_ctor | test_build_facts.py:238 | none | | real saved facts file | |
| D5 | test_a_ninth_activation_fails_the_activation_line | test_compare_scorestats.py:211 | X9 | :212 | hand-built _target_stats | no |
| D5 | test_a_schedule_it_cannot_read_stops_before_the_attach | test_game_tools.py:1841 | X9 (b), X10 (b) | :1845 | fake chain rig; typed message | no |
| D5 | test_a_run_that_never_presses_ends_at_the_run_cap... | :1892 | X9 (b), X10 | :1892-1900 | fake live_play; typed sentence "no press inside the run cap of 0.05 s..." | no |
| D5 | test_this_process_runs | :3129 | none | | real own pid | |
| D5 | test_a_process_that_exited_does_not | :3132 | none | | real child process | |
| D6 | test_launch_and_defer_hold_values_the_launch_knows | test_launch_patch.py:87 | none | | checks the kind, reads the table by name | |
| D6 | test_the_drum_maps_category_and_layout_are_pinned | test_kit_tools.py:280 | X10, X12 | :282-283 | typed 1 and 2, comment cites "Page 07, IN-76" | no |
| C14-C15 | load_tables | tests/single_owner/engine.cpp:270 | none | | real rules files | |
| C14-C15 | load_rules, rule_files | :263 / :253 | X9, X10 | scan.cpp:538-548 | hand-planted rules folder; typed error text | partly ("made-up folder") |
| C14-C15 | select_rule_files | :242 | none | | rule-built path list, answer follows the rule | |
| C14-C15 | parse_rule, parse_rules_file | :86 / :140 | X9, X10 | scan.cpp:515-559 | hand-written JSON rows; typed error text | partly |
| C14-C15 | iso_plus_days | :151 | none | | date arithmetic | |
| C14-C15 | walk_language, walk_reads_for | :308 / :320 | X10 (b) | scan.cpp:440-450 | expected answers are typed folder and extension choices | no |
| C14-C15 | listed_lines | :327 | X9 | scan.cpp:354 | built from a made-up row | partly ("made-up row") |
| C5 | 57 | test_live_protocol.cpp:58 | none | | paths built from the real temp root | |
| C5 | 59 | :67 | none | | | |
| C5 | 60 | :75 | none | | | |
| C5 | 61 | :80 | none | | | |
| C4 | 2 | test_layering.cpp:590 | none | | real rows | |
| C4 | 4 | :629 | none | | | |
| C9 | 2.4 | test_precheck_pick_exe.cpp:155 | none | | real git repo and git's real error | |
| C9 | 6.2 | test_preview_clock.cpp:35 | X11 | :36-52 | seek_ms(30000) required back as 30000, with a typed fake time source | no |
| B3 | 1 | test_replay_schedule.cpp:106 | X9, X10 (b) | :111-120, :140 | hand-built Activation with typed sp_end_steps; typed label "clamped" | partly ("typed") |
| B3 | 23 | :793 | X9, X11, X12 | :797-803 | typed scale 1.0044/0.9970 "from the plan's figures", returned unchanged | no |
| B1 | 25 | test_dynamics_breakdown.cpp:206 | none | | real corpus MIDI | |
| B7 | 2 | test_timing.cpp:73 | X10, X12 | :78-92 | typed tier edges 10/20/30, cites SC-05 doc page | yes (record flags the doc source) |

## Counts

- Keep records read: 66. The brief's numbers add to 64; D5 has 5 and C14-C15 has 7 by my count, and C9 has 2, so I counted every record whose verdict line says Keep.
- Records with at least one fault: 34.
- Records per fault: X9 28 (about 14 of them borderline), X10 11 (about 5 borderline), X11 2, X12 3, snapshot 0.
- Records whose fault the record already named: 1 yes (B7 record 2), 4 partly (DL2 13, C14 load_rules, C14 parse_rule, C14 listed_lines, B3 1 counted as the fifth); 29 did not.

## Not reached

None skipped. The weakest calls are the borderline X9 marks on records that use hand-built library rows or a fake analyzer returning a real corpus result. The main session should decide whether library rows count as made-up outside data.
