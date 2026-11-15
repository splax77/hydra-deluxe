Batch B1, agent a73ec552efea687a4 (all 43 records judged)

Counts: SHOWN BY RUN 1; HOLDS BY READING 37; CONTRADICTED 0; CANNOT TELL 5; not reached 0.


| record | test | claim | verdict | evidence |
|---|---|---|---|---|
| A11 4 | DM:506 (record says 502) | code_name_parts tells code names from prose | HOLDS BY READING | code_name_parts, test_docs_match_code.cpp:236-263; every CHECK traced (paren, `/\=`, is_file_name, qualified regex, k-name or underscore rule) |
| A11 6 | DM:1096 (record says 1092) | code_name_lines picks Hydra: lines and Evidence: of an assumed block | HOLDS BY READING | code_name_lines :828-845 with scan_page :776-822; traced the 14-line page to the expected vector |
| A12 10 | DM:1887 | every plan that counts holds a premise map in the shape | CANNOT TELL | check_premise_maps runs on the real plans; all 15 plans dated 2026-11 contain "Premise map" (grep -L empty), but whether each row and call list is in the shape needs the docs check run |
| A16 22 | test_library_jobs.cpp:1274 | startup read timing | CANNOT TELL | no claim: returns at once unless HYDRA_TIMING_DB names a file (:1276), then only prints ms; needs the user's library |
| A17 16 | test_library_query.cpp:300 | term applies to its own column, Any everywhere | SHOWN BY RUN | R4b/output.txt:159-201: artist:Burnout 0 charts, title:Burnout 1; album:Mothership 2, artist:Mothership "Nothing matches". Code: term_applies_to library_query.cpp:329-331 |
| A17 17 | test_library_query.cpp:313 | unknown field name is a word; empty field value dropped | HOLDS BY READING | parse_library_query :395-410, field_named :229-236, add_term :248-254 drops empty; fold keeps ':'. Run R4b output.txt:208-221 agrees (genre:rock 0 charts, "artist:" shows all 98) |
| A17 40 | test_main.cpp whole | main refuses bad args and zero-match runs | HOLDS BY READING | main :44-61 calls refuse_arguments before, refuse_zero_match after; run_guard.h:94-137 refuses ".cpp" piece (no dash) and repeated filters |
| A17 35 | test_long_paths.cpp:167 | settings, report page, db work past 260 chars | HOLDS BY READING | load_file config.cpp:258 via os_path; write_file_replacing report_files.cpp:121-152 renames temp away; win32_path winstr.cpp:94. Not run on a real long path |
| A17 39 | test_long_paths.cpp:261 | long page copied to short temp path, overwritten next time | HOLDS BY READING | copy_to_short_temp report_files.cpp:62-74 (fits_shell check, overwrite_existing). Fails by design if TEMP is itself over 259 chars |
| A18 2 | test_model.cpp:43 | tied chord notes priced in lane order | HOLDS BY READING | Chord::note_list model.cpp:156-174 stable insertion sort; NoteColor order Kick..Green model.h:56 |
| A18 37 | test_model.cpp:1034 (record text says 944) | difficulty_from_name any case, nothing else | HOLDS BY READING | difficulty_from_name song.cpp:557-563, equals_ci strutil.cpp:29-34 checks size. Record's 944 is now a different test (needs_no_timing) |
| A19 PL13 | test_path_list.cpp:362 | one row open at a time, expand/collapse all, footer folds | HOLDS BY READING | PathsTabUi reset/click_row/set_all path_view.cpp:600-624; mult_open/breakdown_open true path_view.h:283-284. Run R4h-paths/output.txt: click act1 -> "Backend timings##act1 ... opened", second click closes, Expand all opens all three, Collapse all closes |
| A19 PC5 | test_precheck.cpp:505 | unknown option prints the one line, exit 1 | HOLDS BY READING | parse_options tools/precheck/main.cpp:78 throws, wmain :149-152 prints e.what()+"\n", returns 1 |
| A20 2 | test_preview_scene.cpp:70 | color_of inverts lane_of | HOLDS BY READING | preview_view.cpp:20-37 |
| A20 3 | :80 | song length is the one handed in | HOLDS BY READING | build_preview_base :356-359, notes only from note_list :311, beats from last drawn note :370-382; hand song fixture not re-read |
| A20 7 | :174 | solo spans from song.solo_sections | HOLDS BY READING | :335-346; fixture solo_sections {1,2} preview_song_fixtures.h:73 |
| A20 10 | :227 | no path: every fill Offered | HOLDS BY READING | overlay :465-468 ("path == nullptr"); make_fill_song 4 chords with activation_length (fixtures.h:79-92) |
| A20 14 | :289 | 1.0 rule offers the fill the engine charged | CANNOT TELL | overlay reads skipped_fill_ticks from the engine path; whether the Ch10 search skips A and takes C needs the search run on make_ch10_fill_song |
| A20 15 | :306 | beats from timing, 9 lines for one 4/4 bar | HOLDS BY READING | build_beat_events :537-552 and MeasureIndex::walk timing.cpp:170-186, step 480 |
| A20 17 | :351 | 6/8 steps by the eighth | HOLDS BY READING | step=480*4/8=240 timing.cpp:124; WeakBeat folded to Beat :546; halves 120,360..1320 traced |
| A20 18 | :373 | fainter line half back, at a restart | HOLDS BY READING | restarts 1920 and 2400 computed timing.cpp:137-151; walk gives lines 0,480,960,1440,1920,2160,2400,2880, halves match |
| A20 20 | :411 | beat lines stop at song length | HOLDS BY READING | build_preview_base :371-377; display_tick_at_ms llround timing.cpp:349; beat_song tempo (240 bpm implied) not opened |
| A25 P1 | test_report_page.cpp:62 | write_report_file swaps page in, no .tmp, bytes as given | HOLDS BY READING | report_files.cpp:121-157 binary mode for AsGiven, rename, temp removed |
| A25 R25 | test_report_rows.cpp:902 | generate_report with cancel set returns nothing | HOLDS BY READING | report.cpp:283-284 sets EmptyReason::Cancelled before building (rest of the function not read) |
| A25 R29 | :962 | PageSettings::of differs on chart mode only | HOLDS BY READING | PageSettings = Settings::profile() report.cpp:358-361; profile includes chartmode_key config.cpp:436 |
| A25 R30 | :976 | settings_change_touches reads cap and lens, not auto_open | HOLDS BY READING | report.cpp:363-365 compares profiles; auto_open_report is not in profile (profile = {chartmode_key, cap_query, lens}); cap_query/lens bodies not opened |
| A26 RG3 | test_run_guard.cpp:36 | different spellings of one filter pass | HOLDS BY READING | filter_of run_guard.h:85-90 traced for each arg; the comment's claim about doctest not checked |
| A26 RG4 | :43 | well-formed lines pass | HOLDS BY READING | refuse_arguments :94-115 traced for every CHECK, incl. "-sf=" (size test :88) |
| A26 RG6 | :65 | exclude-only and matched runs pass zero check | HOLDS BY READING | refuse_zero_match :124-137 |
| A26 RU2 | test_rules.cpp:77 | missing/empty file loads defaults | HOLDS BY READING | load_rules_file rules_file.cpp:76-96 |
| A26 RU4 | :102 | # comment anywhere | HOLDS BY READING | ini_line_text strutil.cpp cuts at '#' |
| A26 RU5 | :111 | bad value / unknown key names the key | HOLDS BY READING | set_field :56-72, parse_finite_number strutil.cpp:85-96 (nan, inf, 0x10 fail), parse_whole_number ini_errors.h:44, messages ini_errors.h:23-33 contain the key |
| A26 RU6 | :140 | 4.0 and 1e1 load as whole | HOLDS BY READING | parse_whole_number via from_chars general |
| A27 SI3 | test_song_input.cpp:85 | PART DRUM track reads like PART DRUMS | HOLDS BY READING | kDrumsTrackNames {"part drums","part drum"} midi.h:98; is_drums_track_name midi.cpp:447-450 uses equals_ci. MidiFile::drums_track body and the reader's pick not opened |
| A27 SI4 | :99 | modifier on a missing note throws ChartFileError | HOLDS BY READING | Chord::apply_cymbal/ghost/accent model.cpp:294-307 each throw when at(color) is empty |
| A27 SI6 | :130 | parse_finite_number reads only a whole finite number | HOLDS BY READING | strutil.cpp:85-96 traced for all 4 good and 13 bad inputs (trim, '+' strip, from_chars general, end==last, isfinite) |
| A27 SI7 | :143 | .chart Offset not a plain number is absent | HOLDS BY READING | song.cpp:2076-2083 sets chart_offset_s from parse_finite_number of the raw text |
| A27 SI8 | :160 | song.ini delay not a plain number is absent | HOLDS BY READING | read_ini_delay_ms preview_source.cpp:291-299, ini_delay_ms :301-305, parse_delay_ms :113-115 all end in parse_finite_number. read_song_ini_keys not opened |
| A27 SI13 | :351 | display_title: tags go, words stay, trimmed | HOLDS BY READING | display_title song.cpp:676 -> clean_name :672 -> strip_rich_tags :652 / rich_tag_length (color, b, etc.) traced for all 3 inputs |
| A27 SI17 | :377 | display_charter: tags go, ends trimmed | HOLDS BY READING | display_charter song.cpp:685-688, same clean_name path |
| A29 13 | test_sp_meter_curve.cpp:406 | no SP and no path: empty curve, 0 bars | HOLDS BY READING | preview_view.cpp:517-518 picks build_unanalyzed_sp_meter_curve; :107 returns an empty curve when sp_phrases is empty (make_fill_song sets no SP flag); sp_meter_bars_at :522-523 returns 0.0 on no segments |
| A31 SR1 | test_stem_reader.cpp:225 | straight-through read equals the full decode, 5 formats | CANNOT TELL | the claim compares open_stem_reader with old_full_decode (a test-local reference, :146); equality of decoded floats needs the decoders run on the fixtures |
| A31 SR14 | :557 | damaged Opus stem plays again after a seek | CANNOT TELL | depends on libopus rejecting the zero-frame code-3 packet and the reader's recovery; needs the test run (or a read of the Opus reader and libopus) |

Side notes
- A11 4 and 6: the record's line numbers (502, 1092) are stale; the tests are at 506 and 1096. A17 40 and A16 22 records also cite old lines (A16 22 text says :956, test is at :1274).
- A18 37: the record text names test_model.cpp:944, which is now the needs_no_timing test; the difficulty_from_name test is at :1034 (the row's number).
- A18 2 and A25 P1: the A25 P1 test passes a std::string where write_report_file takes a path (converted); fine.
- A11 4 (:506-527) and A11 6 (:1104-1110) test helpers that live in the test file itself (code_name_parts, code_name_lines): no src/ function is called. Their claim is the docs check's own rule.
- A12 10 test (:1887) runs on whatever plans exist; its pass depends on the docs at test time, not on code.
- A16 22 (:1274): asserts nothing about Hydra; with HYDRA_TIMING_DB unset it returns and reports as passed. "kRuns = 5 // the plan's run count" cites a plan.
- A17 35 depends on LongDir (created in the test) reaching 260+ chars; a real hydra_settings.ini copy was not used (typed depth_value 7).
- A19 PC5 (:505-513): expected text is built with the same hydra::unknown_option_line the tool calls, so the wording itself is not pinned there (test_cli.cpp:298 pins "Unknown option: --bogus").
- A20 20 (:433): CHECK(at.beats.back().ms == 4000.0) is exact double equality on a computed value.
- A20 15 (:326), A20 17-18: tick numbers typed from the test's own arithmetic; traced and they agree with the code.
- A26 RG3 (:37 comment): "doctest reads sf and source-file separately and joins their patterns" is a comment about doctest; I did not read doctest.h to confirm it. The guard's own logic holds.
- A26 RU2-RU6: the CLEAN row says "a real older hydra_rules.ini"; the tests use only typed text, and the code path read (load_rules_file) takes any ini.
- A31 SR1: old_full_decode is a test-local stand-in for the old decoder (test_stem_reader.cpp:3, :146).
- A25 R30: cap_query() and Lens bodies (config.cpp ~:436 onward) were not opened; the test's REQUIREs on lens() and the profile tuple cover them.
- No capture served A20 (R4a is replay screenshots with no Preview text), A18, A25, A29; no row in R4a/R4f/R5 outputs was needed for a verdict. R5 holds only batch output, not long-path reports.
