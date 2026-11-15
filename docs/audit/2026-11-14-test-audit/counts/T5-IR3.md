Agent id: ab8eb3f9ab560126a (task T5-IR3, record file D2.md, verdict Investigate)

# T5-IR3: Investigate records of D2.md read by the three test rulings

D2.md holds 155 records. 104 end "Investigate". I read all 104, against the three test files in `tools/ch_mcp/tests/`: FG is test_foreground.py, DF is test_dump_facts.py, FA is test_frida_attach.py. I read each test file whole and every check of every case. I read the records' own text only for a sample (records 42, 1587, 1757 and the shared Q3 answers), and did not re-read the code under test beyond what the test files show.

How I called the faults. X9: the case feeds the code a made-up input: scripted window handles and a fake clock (FG); the hand-cut dump.cs fragment, tiny dump, hand-built facts dict, hand-written pages and constants file, or fake runner (DF); a fake frida module, fake session or script, hand-written replies, a hand-built RunResult, or launch_core run on fake memory (FA). X10: the expected value is the code's own message text, a column line, a key or protocol shape, a default, or a text pin on agent source. X11: a value typed in (an error text, a pid, a reply dict, a version) is required back unchanged. X12: the expected value names a plan. Snapshot: none found.

Named: every record points at a shared Q3 answer (Q3-FG, Q3-DF, Q3-FA) that says the case runs on scripted, hand-cut or fake input and that the real capture is missing. I count that as the record naming X9, as a description of an unchecked claim, not as a fault. For X10, X11 and X12 I write "no" unless I saw the record say it; I opened only records 42, 1587 and 1757 for that, so "no" means "not seen".

The first column gives the record's line number in D2.md. File:line in the table is in the test file named by the prefix.

| Record (D2.md line) | Test file:line | Faults | Faulty checks | Reason | Named |
|---|---|---|---|---|---|
| 42 poll_is_users_tenth | FG:48 | X9 X10 | FG:49-50 | 0.1 typed constant required back; scripted windows | X10 yes ("pins a value"); X9 yes |
| 57 counts_two_changes | FG:52 | X9 | FG:53-57 | scripted handles, fake clock | X9 yes |
| 72 other_windows_alone | FG:59 | X9 | FG:60-64 | scripted handles | X9 yes |
| 87 game_in_front_first_poll | FG:66 | X9 | FG:67-69 | scripted handles | X9 yes |
| 102 found_one_poll_late | FG:71 | X9 | FG:72-76 | scripted handles | X9 yes |
| 117 no_game_window | FG:78 | X9 | FG:79-82 | scripted handles | X9 yes |
| 132 nothing_counts_after_stop | FG:84 | X9 | FG:85-90 | scripted handles; 0.0 comes from the fake clock | X9 yes |
| 147 thread_polls_until_stop | FG:92 | X9 X10 | FG:106-107, 110 | scripted handles; line text typed | X9 yes; X10 no |
| 177 line_has_no_times | FG:124 | X9 X10 | FG:128 | scripted handles; line text typed | X9 yes; X10 no |
| 192 poll_that_raises | FG:130 | X9 X10 X11 | FG:136-137 | broken poll fake; text typed; "no desktop" typed in and required back | X9 yes; others no |
| 230 game_manager_fields | DF:139 | X9 | DF:140-143 | hand-cut dump.cs fragment | X9 yes |
| 242 scrambled_class | DF:145 | X9 | DF:146-147 | hand-cut fragment | X9 yes |
| 253 constants_and_properties | DF:149 | X9 | DF:150-153 | hand-cut fragment | X9 yes |
| 264 static_field | DF:155 | X9 | DF:156-158 | hand-cut fragment | X9 yes |
| 275 methods_carry_rva | DF:160 | X9 | DF:161-164 | hand-cut fragment | X9 yes |
| 286 method_with_no_code | DF:166 | X9 | DF:167-168 | hand-cut fragment | X9 yes |
| 297 generic_instance_lines | DF:170 | X9 | DF:171-172 | hand-cut fragment | X9 yes |
| 308 first_listed_type_is_base | DF:174 | X9 | DF:175-178 | hand-cut fragment | X9 yes |
| 319 interface_listed_first | DF:180 | X9 | DF:181-184 | hand-written class line | X9 yes |
| 330 empty_class_one_brace | DF:186 | X9 | DF:187-190 | hand-cut fragment | X9 yes |
| 352 method_without_rva | DF:200 | X10 | DF:201-203 | hand-written broken dump; error text regex typed | X10 no |
| 366 ctor_rva_finds_engine | DF:207 | X9 X11 | DF:208-213 | hand-cut fragment; ctor RVA and version typed in, returned | X9 yes; X11 no |
| 377 ctor_rva_no_class | DF:215 | X9 X10 | DF:216-217 | hand-cut fragment; message typed | X9 yes; X10 no |
| 388 ctor_rva_two_classes | DF:219 | X9 X10 | DF:220-221 | hand-written second class; message typed | X9 yes; X10 no |
| 399 game_manager_signature | DF:223 | X9 | DF:224-233 | hand-cut fragment | X9 yes |
| 410 write_facts_beside_dump | DF:235 | X9 X10 | DF:236-242 | hand-written dump file; file name typed | X9 yes; X10 no |
| 436 cap_reports_cap_line | DF:288 | X9 X10 | DF:289-292 | fake subprocess runner; message and 600 typed | X9 yes; X10 no |
| 459 fewer_than_three_done | DF:304 | X9 X10 | DF:305-310 | fake runner; messages typed | X9 yes; X10 no |
| 470 missing_output_file | DF:312 | X9 X10 | DF:313-315 | fake runner; message typed | X9 yes; X10 no |
| 484 column_line_ends_with_cited | DF:453 | X9 X10 | DF:454-455 | tiny dump; column line typed | X9 yes; X10 no |
| 495 cited_name_is_found | DF:457 | X9 | DF:458-462 | tiny dump, hand-written page | X9 yes |
| 506 cited_address_any_class | DF:464 | X9 | DF:465-469 | tiny dump, hand page | X9 yes |
| 517 missing_fact_error | DF:471 | X9 X10 | DF:472-477 | hand page; error shape typed | X9 yes; X10 no |
| 528 shared_address | DF:479 | X9 | DF:480-484 | tiny dump, hand page | X9 yes |
| 540 offset_on_base_class | DF:486 | X9 | DF:487-490 | tiny dump, hand page | X9 yes |
| 551 constants_give_rows | DF:492 | X9 | DF:493-499 | hand constants file | X9 yes |
| 562 generic_instantiation_block | DF:501 | X9 | DF:502-508 | tiny dump | X9 yes |
| 573 instantiations_collected | DF:510 | X9 | DF:511-521 | tiny dump | X9 yes |
| 584 split_instance | DF:523 | X9 | DF:524-527 | typed name strings, no real dump name | X9 yes |
| 595 facts_json_no_instantiation | DF:529 | X9 | DF:530-536 | hand-built facts dict, hand page | X9 yes |
| 606 header_md5 | DF:538 | X9 X11 | DF:539-541 | hand DLL bytes; version typed in, returned in header | X9 yes; X11 no |
| 617 python_name_not_game_facts | DF:543 | none | DF:553-555 | hand page (lines 544-552 use plain strings) | - |
| 629 check_fresh_and_stale | DF:557 | X9 | DF:558-568 | tiny dump, hand page, hand-edited table | X9 yes |
| 640 block_naming_another_build | DF:570 | X9 | DF:571-575 | hand page | X9 yes |
| 651 4045_facts_json | DF:577 | X9 X11 | DF:578-591 | hand-built facts dict; version typed in, returned (591) | X9 yes; X11 no |
| 662 file_under_code_and_build | DF:593 | none | DF:594-601 | made-up "9999" heading, hand paths | - |
| 687 our_names_resolve | DF:681 | X9 | DF:682-694 | hand-written names map | X9 yes |
| 698 namespaced_class | DF:696 | X9 | DF:697-702 | map dump, hand page | X9 yes |
| 709 scrambled_span_error | DF:704 | X10 | DF:705-711 | hand page; message text typed | X10 no |
| 720 uncited_row_warning | DF:713 | X9 X10 | DF:714-720 | hand map; message text typed | X9 yes; X10 no |
| 742 check_exits_1_on_map_error | DF:743 | none | DF:744-751 | hand map and page | - |
| 753 bad_column_line | DF:753 | X10 | DF:754-757 | hand map; message typed | X10 no |
| 786 6142_span_member_lacking | DF:818 | X9 X10 | DF:819-824 | hand map; message typed | X9 yes; X10 no |
| 805 detaches_after_normal_end | FA:256 | X9 X11 | FA:257-267 | fake frida, fake messages; written messages equal messages typed in | X9 yes; X11 no |
| 817 start_settings | FA:269 | X9 X11 | FA:270-277 | fake frida; start dict typed, posted back | X9 yes; X11 no |
| 829 detaches_after_cap | FA:279 | X9 X10 | FA:280-286 | fake frida; message typed | X9 yes; X10 no |
| 841 detaches_after_thrown_error | FA:288 | X9 | FA:289-292 | fake session that raises | X9 yes |
| 853 detaches_after_ctrl_c | FA:294 | X9 | FA:295-298 | fake session that raises | X9 yes |
| 878 agent_error_ends_run | FA:307 | X9 X11 | FA:308-313 | fake messages; "GameManager not found" in and out | X9 yes; X11 no |
| 890 script_exception_ends_run | FA:315 | X9 X11 | FA:316-320 | hand-written error message; "TypeError: boom" in and out | X9 yes; X11 no |
| 902 no_message_after_end | FA:322 | X9 | FA:323-326 | fake session | X9 yes |
| 914 game_closing_ends_run | FA:328 | X9 X10 | FA:329-341 | fake detach reason; message typed | X9 yes; X10 no |
| 928 detach_that_raises | FA:345 | X9 X10 | FA:346-360 | fake session; "detach reported: ..." typed | X9 yes; X10 no |
| 980 not_running_says_so | FA:379 | X9 X10 | FA:380-384 | fake frida with no process; message typed | X9 yes; X10 no |
| 992 command_line_exits_1 | FA:386 | X9 X10 | FA:387-394 | fake frida; hand-built agent file; message typed | X9 yes; X10 no |
| 1003 refuses_stale_agent | FA:396 | X9 X10 | FA:397-417 | fake frida; hand-built agent; message typed | X9 yes; X10 no |
| 1017 start_posts_a_read | FA:505 | X9 X11 | FA:506-515 | fake session; STATE typed in, returned and logged | X9 yes; X11 no |
| 1029 payload_with_no_id | FA:517 | X9 X11 | FA:518-528 | fake session; event typed in, heard back | X9 yes; X11 no |
| 1041 start_game_not_running | FA:530 | X9 X10 | FA:531-539 | fake frida; message typed | X9 yes; X10 no |
| 1053 start_no_first_reply | FA:541 | X9 X10 | FA:542-554 | fake session; messages typed | X9 yes; X10 no |
| 1065 start_agent_fails_to_load | FA:556 | X9 X10 X11 | FA:557-564 | fake session; message typed, "bad script" in and out | X9 yes; others no |
| 1077 start_ctrl_c | FA:566 | X9 | FA:567-571 | fake session | X9 yes |
| 1089 read_returns_next_payload | FA:573 | X9 X11 | FA:574-580 | hand replies returned unchanged | X9 yes; X11 no |
| 1101 no_live_game_manager | FA:582 | X9 X11 | FA:583-585 | hand MENU reply returned | X9 yes; X11 no |
| 1113 read_no_reply | FA:587 | X9 X10 | FA:588-593 | fake session; message typed | X9 yes; X10 no |
| 1125 late_reply_not_taken | FA:595 | X9 | FA:596-603 | hand replies; the id matching is real behaviour | X9 yes |
| 1137 request_posts_any_message | FA:605 | X9 X11 | FA:606-614 | typed reply and message returned | X9 yes; X11 no |
| 1149 request_no_reply | FA:616 | X9 X10 | FA:617-622 | fake session; message typed | X9 yes; X10 no |
| 1161 request_bad_id_or_type | FA:624 | X9 | FA:625-631 | fake session | X9 yes |
| 1173 read_after_game_closes | FA:633 | X9 X10 | FA:634-645 | fake detach; messages typed | X9 yes; X10 no |
| 1185 detach_during_read | FA:647 | X9 X10 | FA:648-663 | fake detach; message typed | X9 yes; X10 no |
| 1197 post_refused_by_dropped_session | FA:665 | X9 X10 X11 | FA:666-676 | fake post that raises; message and "the connection is closed" in and out | X9 yes; others no |
| 1209 agent_error_fails_read | FA:678 | X9 X10 X11 | FA:679-686 | hand error reply; text typed in and out | X9 yes; others no |
| 1221 agent_error_whole_reply | FA:688 | X9 X11 | FA:689-694 | hand reply required back whole | X9 yes; X11 no |
| 1233 failure_with_no_reply_none | FA:696 | X10 | FA:697 | typed default (None) of an attribute; no fake input | X10 no |
| 1245 script_exception_fails_read | FA:699 | X9 X10 X11 | FA:700-705 | hand message; text typed in and out | X9 yes; others no |
| 1257 detach_twice | FA:707 | X9 X10 | FA:708-718 | fake session; messages typed | X9 yes; X10 no |
| 1269 detach_after_game_closed | FA:719 | X9 X10 | FA:720-724 | fake detach; message typed | X9 yes; X10 no |
| 1281 detach_raises_log_closes | FA:726 | X9 X10 | FA:727-737 | fake session; message typed | X9 yes; X10 no |
| 1293 pid_attaches | FA:739 | X9 X11 | FA:740-747 | fake frida; pid 1234 typed in, seen back | X9 yes; X11 no |
| 1317 pid_not_running | FA:754 | X9 X10 X11 | FA:755-765 | fake frida; message typed, pid typed in and out | X9 yes; others no |
| 1377 failed_log_write | FA:852 | X9 X10 X11 | FA:853-866 | fake session, fake write that raises; message typed; replies typed | X9 yes; others no |
| 1401 read_before_start | FA:875 | X9 X10 | FA:876-880 | fake frida; message typed | X9 yes; X10 no |
| 1415 summary_of_a_song_that_ends | FA:886 | X9 X10 | FA:887-897 | hand-built RunResult and samples; summary lines typed | X9 yes; X10 no |
| 1439 summary_with_error | FA:906 | X9 X10 X11 | FA:907-917 | hand RunResult; lines typed; error text in and out | X9 yes; others no |
| 1451 command_line_prints_summary | FA:919 | X9 X10 | FA:920-939 | fake frida, hand-built agent; lines typed | X9 yes; X10 no |
| 1463 command_line_ends_at_cap | FA:941 | X9 X10 | FA:942-963 | fake frida; lines typed | X9 yes; X10 no |
| 1575 kit_calls_match_data_file | FA:1061 | none | - | reads the real kit.ts and the real kit_methods.json; the scanner is the test's own rule | - |
| 1587 kit_loads_map_by_category | FA:1072 | X10 X12 | FA:1073-1077 | kit.ts source text pinned ('"LoadMap", 5' count 1); the comment cites a removed plan as the reason | X10 yes ("a pin of today's wording"); X12 yes (record 5 cites the plan) |
| 1757 kit_play_replies_before_end | FA:1279 | X10 | FA:1206-1217, 1280 | exact source line and words pinned | X10 yes ("source words stand in") |
| 1781 every_play_stop_through_unhook | FA:1289 | X10 | FA:1237-1245, 1290 | typed function names and call text pinned in kit.ts | X10 no (not opened) |
| 1807 lookup_reads_native_field_by_name | FA:1311 | X10 | FA:1315-1317 | typed source strings in gamemanager.ts | X10 no (not opened) |
| 1857 patch_only_from_module_observer | FA:1399 | X10 | FA:1359-1381, 1401-1402 | typed API names and call counts pinned in launch.ts | X10 no (not opened) |
| 1895 every_status_comes_back_whole | FA:1440 | X9 X10 X11 | FA:1441-1454 | launch_core run on fake memory (LC.apply, write_sticks, read_throws); fake frida; status in and out; "nothing was written" typed | X9 yes; others no |

## Counts

Investigate records read: 104 of 104. None left unread.

Records with at least one fault: 100. Four are clean: 1575, and after the ruling on hand-built input, 617, 662 and 742.

Records per fault (a record can hold more than one):

- X9, made-up input: 91.
- X10, constant pin: 50. About 33 are message or line text typed as the expected value; 5 are source-text pins on agent files (1587 to 1857 less the clean one); the rest are a typed constant, column line, file name or default.
- X11, echo: 23.
- X12, cited non-code: 1 (record 1587).
- Snapshot: 0.

Records whose fault the record already named: X9 in 91, through the shared Q3 answer, as an unchecked claim and not as a fault. X10 named by the record in the 3 I opened for it (42, 1587, 1757). X11 named in none I saw. X12 named in 1 (1587). I did not open every record for X10 and X11, so those two "named" counts are a floor.

Judgement calls the reader may overrule. First, every FG, DF and FA case except the source scans is X9, because the whole file runs on scripted, hand-cut or fake input; the DF fixtures were copied from the real dump (Q3-DF), but a hand-written dump is still hand-written. Second, I marked few DF cases as echo: a parser turning typed text into numbers is not an echo. Third, the source-text scans (1587 to 1857) read the real agent files, so they carry X10 only, not X9.

Revision after the ruling on hand-built input: a hand-written file or chart that the real code reads is not fake. Only a stand-in for the code's own output, for outside data, or fake code is X9.

I cleared X9 on six DF records whose claim depends only on a hand-written page, map or malformed text the real code reads: 352, 617, 662, 709, 742 and 753. Records 352, 709 and 753 keep X10. Records 617, 662 and 742 now have no fault.

I kept X9 on every case using the hand-cut dump.cs or hand-built facts dict (a stand-in for the dumper output), the fake runner, the scripted windows and fake clock, and the fake frida sessions with hand-written game replies. X9 went from 97 to 91. X10 stays 50, X11 stays 23, X12 stays 1.
