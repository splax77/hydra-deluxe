# SW6: question-3 sweep, re-send, records C6, C9, C14, D1, D3, D5, D13, D15, E1, E3, E4

Agent a53c27452f33b17a3, task T5-SW6. Read-only. Result in one line: no Rewrite record in these eleven files is "Q3 only". Every new record names at least one other failing question.

How I found the records. For each file I counted every record whose verdict line is `Verdict: Rewrite` (any layout), and matched them to the lines in `SW1.md` to `SW4.md` by heading, number and the verdict's own words. C6 (28), C9 (6), D1 (17), D3 (23), D5 (16), E1 (14) and E4 (42) hold exactly the Rewrite records that `rewrites.tsv` lists, and SW3 and SW4 judged each, so I added nothing for them. In E4 the words "### Rewrite.test_..." are a test class named `Rewrite`; those records are Investigate, not Rewrite. The gaps were four files whose tsv row is a section heading: C14, D13, D15 and E3. For those I judged every Rewrite record that had no line of its own. C14: SW3 judged the first record and gave the other six only a short note ("Stops and notes", C14 (7 verdicts)); I judged those six again below and agree with SW3. D13: SW4's one row covers `test_watch_checks.py` tests 90 to 96 and 109 (8 records); I judged the other 21. D15: SW4's row covers records 47, 51 and 52; I judged the other 19. E3: SW4's row covers `test_deny_writes_one_line`; I judged the other 6.

Test used, from `briefs/T5-sweep.md`: Q3 only means the record's reason for Rewrite is only that the claim was not checked on the real thing, and every other question holds. I opened no test code. A "Q3 only" count of zero means the record's own words name a second fault each time. Format: `heading | verdict | the record's words, and the verify file's | capture or real input named`.

## C14 (6 records SW3 gave only a note line; the first record is SW3's tsv row)

single-owner: no block of code is written in two places (tests/single_owner/clones.cpp:282) | Other fault (Q9 floor) | "The floor 100 stands in for 'the walk read the sources it should'... `std::ifstream in(path)` at :208 is never checked, so an unreadable file reads as no lines and is silently skipped"; "Verdict: Rewrite. Replace the floor by: no file failed to open" | n/a
set_aside_old_library moves the file and its side files, replacing an earlier one (tests/test_upgrade_files.cpp:89) | Other fault (Q9 count stands for the right files) | "104 is a count standing for 'the right files are left'"; "Verdict: Rewrite. List the folder's names and compare them with the two expected"; verify agrees | none (record 3 line: "Yes", real files)
set_aside_old_library keeps an earlier set-aside file when the move fails (tests/test_upgrade_files.cpp:119) | Other fault (Q9 count) | "131 is a count standing for 'no other file'"; "Verdict: Rewrite. Compare the folder's names with the three expected names"; verify F6: "Agree, Rewrite" | n/a
set_aside_old_library puts back an earlier set-aside file a stopped run left (:136) | Other fault (Q9 count) | "144 is a count"; "Verdict: Rewrite. Replace 144 with the names expected"; verify F7: "Agree, Rewrite (count 2, line 144)" | n/a
set_aside_old_library drops a stopped run's file that sits beside a set-aside file (:180) | Other fault (Q9 count) | "188 is a count"; "Verdict: Rewrite. Replace 188 with the one name expected"; verify F10: "Agree, Rewrite" | n/a
set_aside_old_library leaves the file in place when a folder has the set-aside name (:193) | Other fault (Q9 count) | "205 is a count"; "Verdict: Rewrite. Replace 205 with the three expected names"; verify F11: "Agree, Rewrite" | n/a

## D13 (21 records new to this sweep; headings are record numbers and test names)

20. test_the_hydra_full_meter_line_follows_question_4 (test_sp_compare.py:409) | Other fault (Q9: reads source text) | "416-417 reads the Python source and asserts the words are absent. This last pair checks source text, not behaviour (Q9 stand-in)"; "3 Claim not checked; C-WATCH" is a second fault; verify: "Agree Rewrite" | n/a
22. test_no_ready_copy_remains (:426) | Other fault (Q9 stand-in) | "A rename slips through it; absent names do not show a copied rule (a stand-in)"; verify: "Agree Rewrite (a source-text check)" | n/a
39. test_column_names_follow_the_width (:642) | Other fault (Q5 copy of production strings) | "typed at 634-635 (copies of the production strings, a copy)"; "3 Claim not checked; C-MEM" is a second fault; verify: "Agree Rewrite" | n/a
46. test_mouths_740_reaches_its_late_kick_press (:729) | Other fault (Q4/Q5 typed results, copy of the arithmetic) | "two typed ms, raw results whose working lives in the comment (Q4 fault); copies of the fixture arithmetic (Q5)"; verify: "Agree Rewrite", with a caution that the proposed rewrite can fail only if min or max in `press_reach` is turned round | none needed (record 3: a real schedule file)
48. test_bomber_147_reaches_its_solved_early_press (:743) | Other fault (Q4 raw pair) | "749 raw result pair"; verify: "Agree Rewrite" | n/a
49. test_bomber_180_reaches_its_late_press (:757) | Other fault (Q4) | "761 typed raw pair (32583.33 + 38.90 = 32622.24, working in the comment)"; verify agrees | n/a
50. test_senri_with_no_squeeze_reaches_its_one_press (:767) | Other fault (Q4) | "772-773 four typed ms"; verify agrees | n/a
63. test_the_real_ls_o2_run_inside_its_song (:874) | Other fault (Q5 snapshot) | "all numbers printed by one run (Q5 snapshot)"; the record names a real input, so Q3 is met; verify: "Agree Rewrite" | the real LS-O2 CSV `testdata/ch_watch/ls-o2-run1-sp_watch-1.csv` (named as met)
64. test_the_real_ls_o2_run_with_no_song_end (:885) | Other fault (Q5 snapshot) | "890 371; 891 5084.784; 892 2146.834; 893 63700.788: snapshot numbers"; verify agrees | same CSV, named as met
66. test_every_star_power_field_at_each_width (:78) | Other fault (Q4 typed results) | "eleven typed results... Raw results whose working is in the comment at 61-62 (Q4 fault)"; "3 Claim not checked; C-MEM" is a second fault; verify: "Agree Rewrite" | none named
70. test_a_sample_costs_three_reads_past_the_engine_block (:147) | Other fault (Q5 copy of offsets) | "a copy of the offsets in the fixture and the walk's own constants"; verify agrees | n/a
78. test_writes_a_row_only_when_a_logged_field_changes (:235) | Other fault (Q4/Q5 typed copy of POLL_S) | "`clock.sleeps == [0.004] * 4`: a typed copy of POLL_S"; verify: "Agree Rewrite" | n/a
81. test_the_wall_clock_cap_ends_a_run_whose_clock_keeps_moving (:273) | Other fault (Q4/Q5 typed cap) | "240 is `RUN_CAP_S` typed; 241 is 1 + 240 steps, working not in the test"; verify agrees | n/a
82. test_should_stop_ends_the_run_after_two_polls (:284) | Other fault (Q5 typed POLL_S) | "298 `[0.004]*2` (typed POLL_S)"; verify agrees | n/a
85. test_the_default_search_prints_its_dots_to_out_only (:342) | Other fault (Q9 floor) | "a floor of four dots with no reason"; verify agrees | n/a
86. test_find_engine_live_sends_its_dots_to_the_out_it_is_given (:352) | Other fault (Q9 floor) | "same four-dot floor"; verify agrees | n/a
88. test_a_whole_run_writes_the_header_and_prints_the_summary (:374) | Other fault (Q4 typed results) | "the first row as 17 typed strings... raw results"; "3 Claim not checked; C-MEM" is a second fault; verify: "I counted 17 typed strings" | none named
115. test_a_clean_run_says_no_stall (:124) | Other fault (Q4/Q5 copy of inputs) | "an f-string with no placeholder, so the typed `500` and `0.01` repeat the inputs set on line 126"; verify agrees | n/a
120. test_the_load_process_is_stopped_at_the_end (:214) | Other fault (Q9 pins layout) | "Drop the `basename == '.git'` check at 242 (it pins the layout, not the rule)"; verify agrees | n/a
121. test_the_step_names_are_the_timing_columns (:244) | Other fault (Q4 weak assertion) | "`zip` stops at the shorter list, so a missing column is not caught"; verify: "zip stops at the shorter list, confirmed at 247" | n/a
161. test_summary_groups_by_type_and_effort (:263) | Other fault (Q4 typed results) | "(12.20 and 6.10 give median 9.15: typed results)... Fragile: `split()` column positions"; "3 Claim not checked; C-PRICE" is a second fault; verify agrees | none named

## D15 (19 records new to this sweep)

BP-3 the reply names the pass's limit and the command to build with (test_build_pass.ps1:127) | Other fault (Q4/Q5 snapshot, untraceable number) | "a typed number copied from `$BuildPassUnclaimedSeconds`... a snapshot of the constant"; "120 comes from an Unconfirmed list line... not a filed ruling"; verify: "agree Rewrite", and the 120 has no filed ruling | none (real script)
BP-13 off the gate, builds as before (:235) | Other fault (Q9 floor) | "a floor `-ge 2`... the floor has no named reason"; verify: "agree Rewrite" | n/a
BP-14 with a pass: claim before configure... (:248) | Other fault (Q4/Q5 copy of a command line) | "257 is a raw copy of the production command line, so it passes with any bug that prints the same text"; verify agrees | n/a
BP-15 a Ninja preset has no low-priority switch (:263) | Other fault (Q9 floor) | "The `-ge 1` is a floor"; verify agrees | n/a
CF-3 an ancestor record, one changed file (ci_build_folder tests :86) | Other fault (Q5 copy of a constant) | "2000-01-01 is a copy; any old time would do"; verify agrees | n/a
MP-2 Select-Spread (:112) | Other fault (Q4 typed raw results) | "The typed answers (1,3,5) are raw results"; verify: "agree Rewrite" | n/a
MP-4 a normal run (:213) | Other fault (Q4 derived number typed) | "219's '(60%)' is a derived number typed; the rest are fine"; verify checks the rest holds | n/a
MP-8 under the gate with a pass, one pass holds across all rebuilds (:244) | Other fault (Q4 typed working) | "Typed 8 is derived from the `-Max 6` plus two; the working is in the comment only"; verify: "agree Rewrite" | n/a
RS-5 the keygen's refusals (:105) | Other fault (Q9 exit code only) | "Exit code only, not the message, so a refusal for the wrong reason would pass (a stand-in)"; verify agrees | n/a
9. test_a_cut_reply_keeps_a_line_whole (:243) | Other fault (Q4 raw result) | "Raw result of `cap_reply`, working in the code"; verify: "agree Rewrite" | n/a
13. test_long_reply_is_cut_at_the_cap (:286) | Other fault (Q5 copy) | "'12,000' copies `REPLY_CAP_CHARS`"; verify agrees | n/a
14. test_a_cut_reply_ends_with_the_given_hint (:296) | Other fault (Q4 raw result) | "Raw result as 9"; verify agrees | n/a
62. test_headless_session (:824) | Other fault (Q5 snapshot) | "Snapshot: 98 is the corpus count"; verify: "`scan()` line 774 types `charts=98`" | none (corpus)
67. test_the_wait_cap_and_the_timeout_every_reply_carries (:862) | Other fault (Q4 typed derived numbers) | "The 31 and 30 are derived by the test (30 plus one)"; verify agrees | n/a
70. test_text_and_dump_filters_are_hydras (:896) | Other fault (Q4 raw results pinned from one run) | "raw results pinned from one run (window size, positions, ids)"; verify agrees | n/a
78. test_a_focused_wildcard_ref_finds_the_popups_checkbox (:969) | Other fault (Q4) | "a raw pinned result"; verify agrees | n/a
80. test_dump_shows_the_whole_label (:982) | Other fault (Q4) | "Raw pinned result"; verify agrees | n/a
81. test_a_popup_dump_lists_the_checkboxes_inside_its_tables (:990) | Other fault (Q4) | "Raw pinned result"; verify agrees | n/a
84. test_window_session_on_a_seed_from_headless (:1059) | Other fault (Q5 snapshot) | "including `charts=98` at 1070"; verify agrees | n/a

## E3 (6 records new to this sweep)

test_prompt_hash_is_utf8_sha256_pinned (test_review_dispatch.py:116) | Other fault (Q4/Q5 snapshot from one run) | "a 40-hex hash 'pinned from one run of the fixture'... a snapshot"; verify: "agree it is not Keep; dispute the recipe" | n/a
test_installer_writes_both_shims_with_real_paths (test_ref_judge.py:616) | Other fault (Q9 can fail by luck) | "no '__' anywhere... can fail by luck"; verify: "agree Rewrite... a flaky failure" | n/a
test_installer_refuses_a_foreign_hook_and_writes_neither (:645) | Other fault (Q4 half-checked claim) | "the claim 'not overwritten' is half-checked"; verify agrees | n/a
test_bad_gate_line_raises (:273) | Other fault (Q9 broad exception) | "`assertRaises(Exception)`: any exception passes"; verify: "agree, Rewrite" | n/a
test_doc_key_matches_the_publish_gate_vector (:331) | Other fault (Q5 snapshot; name does not match body) | "equals a 64-hex literal 'from one run'... the test never calls `publish_gate.py`"; verify disputes the fix and moves it to Remove: `publish_gate.py` has no key function of its own | n/a
test_malformed_records_count_for_nothing (:476) | Other fault (Q9 one check after a loop) | "one assertion after three bad lines and seven bad variants"; verify: "agree, Rewrite" | n/a

## Count table

| Records file | Rewrite records in all | New to me (no line in SW1 to SW4) | Q3 only | Other fault | Unclear |
|---|---|---|---|---|---|
| C6 | 28 | 0 | 0 | 0 | 0 |
| C9 | 6 | 0 | 0 | 0 | 0 |
| C14 | 7 | 0 (6 had only a note line in SW3; re-judged above) | 0 | 6 | 0 |
| D1 | 17 | 0 | 0 | 0 | 0 |
| D3 | 23 | 0 | 0 | 0 | 0 |
| D5 | 16 | 0 | 0 | 0 | 0 |
| D13 | 29 | 21 | 0 | 21 | 0 |
| D15 | 22 | 19 | 0 | 19 | 0 |
| E1 | 14 | 0 | 0 | 0 | 0 |
| E3 | 7 | 6 | 0 | 6 | 0 |
| E4 | 42 | 0 | 0 | 0 | 0 |
| Total | 211 | 46 | 0 | 52 (46 new + 6 C14 re-judged) | 0 |

Rows for the not-new files sit in `SW3.md` (C6, C9, D1) and `SW4.md` (D3, D5, E1, E4); I did not repeat them.

## For the main session

1. D13 test 63 and 64 read a real CSV, so Q3 is met there; their fault is the snapshot numbers.
2. E3 `test_doc_key_matches_the_publish_gate_vector`: the verify file moves it to Remove, which changes the verdict, not just the fix.
3. D15 MP-4's verdict line reads "Keep, except the 60%... Verdict: Rewrite": one line of a mostly sound test.
