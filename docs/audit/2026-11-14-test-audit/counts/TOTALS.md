Agent id: af18b5ec198cb3696 (task T5-TOTALS, session 63c70014-4328-49dc-9205-742235732752)

# T5-TOTALS: the 26 recount files added up, after the user's reading of "made-up input"

Plain version. The 26 counters re-read the audit's per-test records. I added up their tables by script and re-marked the files that finished before the user's choice (input a test builds and feeds to the real code is not fake). I changed no counter file. After my corrections: 421 Keep records read, 239 hold a fault; 2028 Investigate records read, 1622 hold a fault; 2449 records in all, 1861 hold a fault. Where my figure differs from the counter's, the counter's is in brackets.

How I counted. I parsed the table rows of each file and counted by the fault cell of each row. For KR5 and IR19 the parse could not see a few records (KR5 one X12; IR19 three faulty records and three X10), so I added the counter's figure for those. The "Named" column is each counter's own figure; I did not recompute it, and a re-marked record keeps what its counter said about it. For KR2 and KR5 the read count is the counter's (some clean records sit in prose).

The reading I used for X9. X9 is a stand-in for the code's own output (hand-built results, rows, records, summaries, schedules, recorder samples, press logs, watcher logs, review verdicts), a stand-in for outside data (service replies, transcripts, game memory, process lists, hook payloads, the game's own scorestats, OCR text), or fake code (fake analyzers, readers, workers, rigs, clocks, sessions, stub programs, stub gate modules). A hand-written chart, ini, settings, SQL, doc, note, rule, source text, malformed or fuzz bytes that the real code reads is input, not X9.

## 1. One row per count file

Columns: records read, records with at least one fault, then records per fault, then records whose fault the record already named (the counter's figure).

| File | Read | Faulty | X9 | X10 | X11 | X12 | Snapshot | Named |
|---|---|---|---|---|---|---|---|---|
| T5-KR1 | 91 | 59 | 41 | 45 | 22 | 0 | 0 | 39 |
| T5-KR2 | 97 | 63 (counter 68) | 40 (counter 50) | 37 | 9 | 1 | 2 | 17 |
| T5-KR3 | 84 | 52 (counter 54) | 29 (counter 37) | 31 | 13 | 0 | 2 | 30 |
| T5-KR4 | 66 | 34 | 26 (counter 28) | 11 | 2 | 3 | 0 | 5 (1 yes, 4 partly) |
| T5-KR5 | 83 | 31 (counter 25) | 17 (counter 11) | 13 | 2 | 2 | 0 | 16 |
| T5-IR1 | 119 | 45 (counter 51) | 23 (counter 30) | 23 | 0 | 1 | 0 | 9 |
| T5-IR2 | 111 | 108 | 108 | 68 | 27 | 5 | 0 | 35 own; X9 in all 108 via shared header |
| T5-IR3 | 104 | 99 (counter 100) | 88 (counter 91) | 50 | 23 | 1 | 0 | X9 in 91 via shared answer; X10 3 of those opened |
| T5-IR4 | 104 | 94 | 88 | 37 | 14 | 0 | 1 | 22 |
| T5-IR5 | 100 | 89 | 77 | 62 | 26 | 7 | 1 | 36 |
| T5-IR6 | 114 | 108 | 99 | 41 | 6 | 4 | 0 | 106 |
| T5-IR7 | 86 | 79 | 70 | 28 | 10 | 1 | 0 | not stated (Named column) |
| T5-IR8 | 85 | 84 (counter 83) | 80 (counter 76) | 37 (counter 44) | 20 (counter 24) | 3 (counter 8) | 2 (counter 7) | 0 by label, 4 in part |
| T5-IR9 | 85 | 82 | 76 | 47 | 4 | 11 | 1 | 14 (58 more as claim-not-checked) |
| T5-IR10 | 85 | 34 | 6 | 30 | 1 | 1 | 0 | 18 or more |
| T5-IR11 | 98 | 91 (counter 93) | 80 (counter 83) | 27 | 10 | 6 | 0 | 75 (+9 partly) |
| T5-IR12 | 86 | 66 | 51 (counter 52) | 40 | 9 | 3 | 2 | 29 (+24 partly) |
| T5-IR13 | 89 | 73 | 65 | 45 | 10 | 5 | 2 | about 26 |
| T5-IR14 | 90 | 76 | 51 | 57 | 4 | 12 | 8 | 36 (+6 partly) |
| T5-IR15 | 91 | 85 | 54 | 34 | 3 | 12 | 8 | 36 |
| T5-IR16 | 90 | 54 (counter 55) | 29 (counter 30) | 24 | 10 | 15 | 1 | about 28 |
| T5-IR17 | 142 | 117 (counter 126) | 92 (counter 101) | 81 | 8 | 2 | 2 | 117 |
| T5-IR18 | 86 | 70 | 44 | 29 | 6 | 21 | 2 | about 22 |
| T5-IR19 | 90 | 67 | 51 | 23 | 7 | 2 | 3 | 17 |
| T5-IR20 | 91 | 38 (counter 40) | 33 (counter 38) | 13 | 0 | 3 | 0 | 3 |
| T5-IR21 | 82 | 63 | 34 | 30 | 3 | 14 | 9 | 9 (A41 14 in part) |

## 2. Totals

| Group | Read | Faulty | X9 | X10 | X11 | X12 | Snapshot |
|---|---|---|---|---|---|---|---|
| Keep (KR1 to KR5), mine | 421 | 239 | 153 | 137 | 48 | 6 | 4 |
| Keep, counters' sum | 421 | 240 | 167 | 137 | 48 | 6 | 4 |
| Investigate (IR1 to IR21), mine | 2028 | 1622 | 1299 | 826 | 201 | 129 | 42 |
| Investigate, counters' sum | 2028 | 1642 | 1324 | 833 | 205 | 134 | 47 |
| Both, mine | 2449 | 1861 | 1452 | 963 | 249 | 135 | 46 |
| Both, counters' sum | 2449 | 1882 | 1491 | 970 | 253 | 140 | 51 |

A record can hold several faults, so the fault columns do not add to the faulty column. IR8's per-fault figures were tallied by hand by its counter and called approximate; mine come from its table filtered to Investigate records (section 4).

## 3. Corrections: 55 in all (49 X9 removed, 6 X9 added)

Reason by file. "By reason" means I decided from the counter's own reason text for the row and did not open the test.


KR2. removed: input is a hand-written file or text the real code reads (malformed JSON bytes, a git ref line that is not one, an old shim file, press-log rows written by the real writer play_chart.write_press_log, deliberately wrong csv headers); X10/X11 stay where the row has them. Opened test_score_compare.py, test_refusals.py, test_review_dispatch.py.

- T5-KR2, record 2 test_review_dispatch.py:211: X9 to none
- T5-KR2, record 3 test_review_dispatch.py:353: X9 to none
- T5-KR2, record 12 test_ref_judge.py:412: X10, X9 to X10
- T5-KR2, record 29 test_ref_judge.py:654: X9 to none
- T5-KR2, record 6 test_machine_recorder.py:708: X9 to none
- T5-KR2, record 34 test_score_compare.py:172: X11, X9 to X11
- T5-KR2, record 35 test_score_compare.py:181: X9 to none
- T5-KR2, record 37 test_score_compare.py:283: X11, X9 to X11
- T5-KR2, record 38 test_score_compare.py:308: X11, X9 to X11
- T5-KR2, record 40 test_score_compare.py:326: X10, X9 to X10

KR3. removed: hand-typed source text and hand-written data/kit text fed to the scanner (the scanner lives in the test file, so these also test the test; see unsettled item 1).

- T5-KR3, record 26 test_frida_attach.py:976 check_catches_every_pattern: X9 to none
- T5-KR3, record 27 test_frida_attach.py:985 reads_and_comparisons_pass: X9 to none
- T5-KR3, record 29 test_frida_attach.py:1039 only_the_listed_agents_hold_a_list: X10, X9 to X10
- T5-KR3, record 30 test_frida_attach.py:1048 method_outside_the_list_fails: X10, X9 to X10
- T5-KR3, record 31 test_frida_attach.py:1054 name_on_another_files_list_fails: X10, X9 to X10
- T5-KR3, record 32 test_frida_attach.py:1104 kit_list_name_the_data_file_lacks_: X10, X9 to X10
- T5-KR3, record 33 test_frida_attach.py:1111 data_file_name_the_kit_list_lacks_: X10, X9 to X10
- T5-KR3, record 35 test_frida_attach.py:1275 a_number_of_its_own_fails: X10, X9 to X10

KR4. removed: hand-planted rules folder and hand-written rule JSON read by the real rule loader and parser.

- T5-KR4, record C14-C15 load_rules, rule_files: X10, X9 to X10
- T5-KR4, record C14-C15 parse_rule, parse_rules_file: X10, X9 to X10

KR5. added: hand-built library row or hand-built record the scan did not make (A21 19 and 24: entry whose md5 or sig is typed over a scanned one; A33 T20 and T23: failure_record built by hand; A25 R3 and R26: fill_store builds library rows with library_entry_for_file and R26 also tied_variant_record). Opened tests/test_preview_controller.cpp, test_store_records.cpp, test_report_rows.cpp.

- T5-KR5, record A21 19 :672: none to X9
- T5-KR5, record A21 24 :895: none to X9
- T5-KR5, record A33 T20 test_store_records.cpp:963: none to X9
- T5-KR5, record A33 T23 :1023: none to X9
- T5-KR5, record A25 R3 test_report_rows.cpp:235: none to X9
- T5-KR5, record A25 R26 :919: none to X9

IR1. removed: hand-written SQL stores read by the real compare tool (same treatment IR21 gave the compare tool's databases, DL5b 9 and 10); gray, see unsettled item 2.

- T5-IR1, record test_result_id_order_alone_compares_clean :205 X9: X9 to none
- T5-IR1, record test_one_changed_payload_byte_is_reported :230 X9: X9 to none
- T5-IR1, record test_chart_scan_order_is_part_of_the_answer :241 X9: X9 to none
- T5-IR1, record test_summary_only_old_layout_matches_new_layout :258 X9, X10: X10, X9 to X10
- T5-IR1, record test_summary_only_reports_one_changed_score :275 X9: X9 to none
- T5-IR1, record test_profile_id_order_alone_compares_clean :286 (2 cases) X9: X9 to none
- T5-IR1, record test_one_changed_record_path_is_reported :299 X9: X9 to none

IR3. removed: hand-built DLL bytes (606), hand-written names map (687, 720) read by the real code; by reason.

- T5-IR3, record 606 header_md5 DF:538: X11, X9 to X11
- T5-IR3, record 687 our_names_resolve DF:681: X9 to none
- T5-IR3, record 720 uncited_row_warning DF:713: X10, X9 to X10

IR11. removed: hand-written settings, report page and ImGui ini text read by the real file code (test_long_paths); by reason.

- T5-IR11, record A17 35: X9 to none
- T5-IR11, record A17 36: X11, X9 to X11
- T5-IR11, record A17 39: X9 to none

IR12. removed: hand-written settings.ini read by the real code; by reason.

- T5-IR12, record D8.md test_server.py :263: X10, X9 to X10

IR16. removed: made-up path text fed to the real parser, malformed input; by reason.

- T5-IR16, record B2 PR:99: X9 to none

IR17. removed: hand-built ini and song.ini text read by the real game_files code; by reason.

- T5-IR17, record D3 (test): X9 to none
- T5-IR17, record D3 (test): X9 to none
- T5-IR17, record D3 (test): X9 to none
- T5-IR17, record D3 (test): X9 to none
- T5-IR17, record D3 (test): X9 to none
- T5-IR17, record D3 (test): X9 to none
- T5-IR17, record D3 (test): X9 to none
- T5-IR17, record D3 (test): X9 to none
- T5-IR17, record D3 (test): X9 to none

IR20. removed: hand-written ps1/yml text, a deliberately broken settings or rules file, a hand-typed path list, and rule files written to a temp folder, all read by the real code; by reason.

- T5-IR20, record C14-C15 lexer_language/skipped_comment_line scan.cpp:452-476: X9 to none
- T5-IR20, record A49 7 bad-settings-refused pbtest_window.cpp:517-518: X10, X9 to X10
- T5-IR20, record A49 8 bad-rules-refused pbtest_window.cpp:553-554: X10, X9 to X10
- T5-IR20, record A49 16 select_rule_files scan.cpp:379-383: X9 to none
- T5-IR20, record A49 21 area folder rows scan.cpp:538-561: X10, X9 to X10

Post-choice files (IR2, IR3, IR5, IR6, IR7, IR10, IR13, IR14, IR18, IR19, IR21). I screened every X9 row whose reason names a hand-written file or text and no stand-in (41 rows) and read those rows. Most stand on a rig, a dumper line, a recorder or watcher file, or a game file, so I left them. I changed three in IR3 (above). I did not change these, which look inconsistent with what the same counters cleared and which go to the main session: IR3 551 (hand constants file), IR3 319 and 388 (hand-written class lines of the dumper output, kept as outside data), IR6 RP 103 and 104 (hand-written settings.ini; the X9 rests on the Fixture recording), IR5 71 (hand-written score JSON; kept because the score summary is Hydra's own output).

Revisions I made and then reversed after reading the counters' own reasons: hand-written recorder sample files (KR1 RL 274, 280, 286, 292; KR2 machine_report 13 to 15) and score JSON files (KR3 D9 8, IR5 71) stay X9, because IR2 and IR16 treat the recorder's samples, press logs and score summaries as the code's own output.

## 4. Record-count mismatches

- IR8: its table has 88 D4 rows but D4.md holds 75 records whose last verdict is Investigate. The other 13 are Rewrite (696, 770, 833, 839, 846, 926, 1173, and the grouped 1178, 1183, 1188, 1193) or Keep (1154, 1283). I filtered to the 75 plus A3 5 plus A46 5 = 85 records, 84 faulty (only A3 #20 is clean). Its X12 and snapshot figures fall because the non-Investigate rows held most of them.
- IR2: 111 records (110 begin the verdict line with "Investigate", 2688 carries it inline).
- IR5: 100 records (89 begin a line with "Verdict: Investigate", 11 carry it mid-line).
- IR1: 119 records; D14 holds 121, the two Rewrite records are left out.
- KR4: 66 Keep records, where the brief's numbers add to 64 (D5 5, C14-C15 7, C9 2 by the verdict lines).
- KR5: 83 Keep records read; 85 matched a search for "Keep" because A18 20 and 47 are Investigate (they belong to IR15, which holds A18).
- IR20: 91 records, 51 of them clean ones listed by block; A38 has no test case, the counter judged it by reading run_command.
- IR17: 142 (D3 95, A25 23, A27 24); the D3 records carry no numbers, so rows are the tests.
- IR10 85, IR19 90, IR21 82 (A41 16, DL1 9, DL2 8, DL4 9, DL5 3, DL5b 11, B3 7, A30 7, A45 7, T3-run2 5): the counters report no mismatch.
- True totals: Keep 421, Investigate 2028 by the counters' read counts (IR8 corrected to 85), 2449 in all. I did not recount the main session's `^Verdict: Investigate` lines over the other 25 files; only IR8 was checked against its record file.

## 5. Overlaps

Each file at HEAD matches the working tree line for line, begins with its own agent id line and ends with its own last section, so none is cut or mixed: IR9 103 lines and IR16 118 (commit dfb6938c); IR4 143 and IR15 119 (cfd1045c); IR14 134 and IR19 119 (f9137d56, which only edits both). Each of the six has its own counter's agent id on its first line.

## 6. Unsettled

1. KR3 D2 26 to 35: the scanner that reads the hand-typed source lines is defined in the test file itself, so removing X9 leaves a test that checks its own checker. If the user counts that as fake code, these eight go back to X9.
2. IR1 compare_db: hand-written SQL stores laid in a schema the file calls a stand-in for an older layout. I cleared X9 (hand-written SQL the real tool reads); if the stores count as stand-ins for the code's own output (hand-typed library rows), the seven go back.
3. IR4: about 25 hook-payload rows (RD) and the hand-written verdict rows (RR) stay X9 as outside data and the code's own output; a hook payload is the nearest case to hand-built input fed to real code, so this is the largest single block that could flip.
4. IR8 D4 616 to 674 (hand-written stats and score JSON), IR8 A46 test_fits_screen (hand-typed saved-window text): left X9.
5. I opened test code only for the rows named in section 3 and the KR5 scan. Everything else is from the counters' reasons.

