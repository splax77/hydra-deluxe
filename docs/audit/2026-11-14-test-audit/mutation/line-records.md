# F1: the 62 disputed rows against the full mutation run 20261116-123523

The tool plants its own kinds of bug. It does not plant the change a row names. So a row whose test caught nothing is "not shown to fail by this run". That is all it says. No row holds an agent's verdict.

One re-read row can name several lines, so the table has one line per named line (75 lines from 61 rows). The 62nd row names no line and is listed at the end.

"Bugs it reaches" is how many planted bugs sit in code that test runs. "Bugs it caught" is how many bugs made the test fail, the same count as the run's tests.txt. It counts a bug the test reaches, and also a bug the test does not reach but an earlier test in the same run did (or the run noted the bug's switch between tests before it), so the bug could reach it through state the run keeps between tests. The part in brackets is how many of the caught bugs were of that second kind. "This line failed in" counts the bugs where the check on that exact line was one of the failed checks. A line with no check of its own (a comment, a closing brace, or a message string inside a check) can never match, so it reads none.

"Outside every test case" means the line sits in a helper function, not in a test. No test is named for it. The table still says whether a check on that line failed.

| Slice | Re-read line | Test line | Test | Bugs it reaches | Bugs it caught | This line failed in |
|---|---|---|---|---|---|---|
| A12 | 13 | tests/test_docs_match_code.cpp:1109 | docs match code: which Clone Hero page lines name Hydra code | 2 | 0: not shown to fail by this run | none |
| A12 | 32 | tests/test_docs_match_code.cpp:1122 | docs match code: a scrambled name on a Clone Hero page fails with its line | 2 | 0: not shown to fail by this run | none |
| A12 | 49 | tests/test_docs_match_code.cpp:1134 | docs match code: a page span is found by the table's cited column | 2 | 0: not shown to fail by this run | none |
| A12 | 68 | tests/test_docs_match_code.cpp:1156 | docs match code: what counts as a game fact on a Clone Hero page | 2 | 0: not shown to fail by this run | none |
| A12 | 87 | tests/test_docs_match_code.cpp:1195 | docs match code: which spans and which build a Clone Hero page line has | 2 | 0: not shown to fail by this run | none |
| A12 | 105 | tests/test_docs_match_code.cpp:1266 | outside every test case | | | none: not shown to fail by this run |
| A12 | 122 | tests/test_docs_match_code.cpp:1273 | docs match code: no committed doc names a path into a scratch folder | 0 | 0: not shown to fail by this run | none |
| A12 | 139 | tests/test_docs_match_code.cpp:1416 | outside every test case | | | none: not shown to fail by this run |
| A12 | 156 | tests/test_docs_match_code.cpp:1447 | docs match code: no permanent doc or code comment names a docs path that is not in the tree | 0 | 0: not shown to fail by this run | none |
| A12 | 173 | tests/test_docs_match_code.cpp:1883 | outside every test case | | | none: not shown to fail by this run |
| A12 | 191 | tests/test_docs_match_code.cpp:1890 | docs match code: every plan that counts holds a Premise map in the shape | 2 | 0: not shown to fail by this run | none |
| A12 | 208 | tests/test_docs_match_code.cpp:2032 | outside every test case | | | none: not shown to fail by this run |
| A12 | 226 | tests/test_docs_match_code.cpp:2139 | docs match code: every picture the public pages link is a shot the capture script takes, and the file exists | 2 | 0: not shown to fail by this run | none |
| A14 | 459 | tests/test_fills_and_skips.cpp:145 | outside every test case | | | none: not shown to fail by this run |
| A14 | 476 | tests/test_fills_and_skips.cpp:154 | outside every test case | | | none: not shown to fail by this run |
| A32 | 28 | tests/test_store_open.cpp:132 | outside every test case | | | none: not shown to fail by this run |
| A32 | 273 | tests/test_store_open.cpp:763 | outside every test case | | | none: not shown to fail by this run |
| A32 | 508 | tests/test_store_open.cpp:1205 | outside every test case | | | none: not shown to fail by this run |
| B1 | 316 | tests/test_cli.cpp:635 | hydra_fillcompare compares both rules out of one database | 725 | 115 (18 through shared state) | none |
| B2 | 23 | tests/test_generated_fills.cpp:62 | generated fills: a chord 241 ticks after the second measure line gets a fill | 30 | 6 | none |
| B2 | 23 | tests/test_generated_fills.cpp:73 | generated fills: a chord 241 ticks after the second measure line gets a fill | 30 | 6 | none |
| B2 | 41 | tests/test_generated_fills.cpp:75 | generated fills: Clone Hero 1.0 mode gives half-measure fills, 1.1 line-start fills | 40 | 9 | none |
| B2 | 41 | tests/test_generated_fills.cpp:86 | generated fills: Clone Hero 1.0 mode gives half-measure fills, 1.1 line-start fills | 40 | 9 | none |
| B2 | 55 | tests/test_generated_fills.cpp:88 | generated fills: a chord 242 ticks from its line gets none | 34 | 3 | none |
| B2 | 55 | tests/test_generated_fills.cpp:91 | generated fills: a chord 242 ticks from its line gets none | 34 | 3 | none |
| B2 | 71 | tests/test_generated_fills.cpp:93 | generated fills: a chord exactly on the line has length 0 | 29 | 5 | none |
| B2 | 71 | tests/test_generated_fills.cpp:96 | generated fills: a chord exactly on the line has length 0 | 29 | 5 | none |
| B2 | 84 | tests/test_generated_fills.cpp:98 | generated fills: a chord just before its line has a negative length | 32 | 8 | none |
| B2 | 84 | tests/test_generated_fills.cpp:103 | generated fills: a chord just before its line has a negative length | 32 | 8 | none |
| B2 | 96 | tests/test_generated_fills.cpp:105 | generated fills: the 1.0 deadline of a 0 or negative length is the line minus 250 ms | 33 | 6 | none |
| B2 | 96 | tests/test_generated_fills.cpp:117 | generated fills: the 1.0 deadline of a 0 or negative length is the line minus 250 ms | 33 | 6 | none |
| B2 | 109 | tests/test_generated_fills.cpp:119 | generated fills: the nearer chord wins, and a tie goes to the later one | 32 | 4 | none |
| B2 | 109 | tests/test_generated_fills.cpp:123 | generated fills: the nearer chord wins, and a tie goes to the later one | 32 | 4 | none |
| B2 | 122 | tests/test_generated_fills.cpp:125 | generated fills: after a fill the next candidate is 4 lines on | 32 | 4 | none |
| B2 | 122 | tests/test_generated_fills.cpp:130 | generated fills: after a fill the next candidate is 4 lines on | 32 | 4 | none |
| B2 | 134 | tests/test_generated_fills.cpp:132 | generated fills: the first line is never a candidate | 23 | 4 | none |
| B2 | 134 | tests/test_generated_fills.cpp:136 | generated fills: the first line is never a candidate | 23 | 4 | none |
| B2 | 147 | tests/test_generated_fills.cpp:138 | generated fills: a first chord in measure 3 still counts lines from the second | 37 | 6 | none |
| B2 | 147 | tests/test_generated_fills.cpp:145 | generated fills: a first chord in measure 3 still counts lines from the second | 37 | 6 | none |
| B2 | 159 | tests/test_generated_fills.cpp:147 | generated fills: a 4/4 to 7/8 change places fills on the restarted grid | 46 | 6 | none |
| B2 | 159 | tests/test_generated_fills.cpp:153 | generated fills: a 4/4 to 7/8 change places fills on the restarted grid | 46 | 6 | none |
| B2 | 171 | tests/test_generated_fills.cpp:155 | generated fills: Clone Hero 1.0 mode reads the same restarted grid | 39 | 7 | none |
| B2 | 171 | tests/test_generated_fills.cpp:161 | generated fills: Clone Hero 1.0 mode reads the same restarted grid | 39 | 7 | none |
| B2 | 180 | tests/test_generated_fills.cpp:163 | generated fills: a 1.0 fill whose chord sits on a meter change takes the new meter | 39 | 6 | none |
| B2 | 180 | tests/test_generated_fills.cpp:168 | generated fills: a 1.0 fill whose chord sits on a meter change takes the new meter | 39 | 6 | none |
| B2 | 189 | tests/test_generated_fills.cpp:170 | generated fills: a chart with its own fills gets none generated | 21 | 3 | none |
| B2 | 189 | tests/test_generated_fills.cpp:185 | generated fills: a chart with its own fills gets none generated | 21 | 3 | none |
| B2 | 201 | tests/test_generated_fills.cpp:187 | generated fills: a chart with its own fills gets none generated | 21 | 3 | none |
| B2 | 201 | tests/test_generated_fills.cpp:281 | generated fills: every corpus chart's generated fills are pinned | 3 | 5 (5 through shared state) | none |
| B2 | 227 | tests/test_midi.cpp:60 | midi: every corpus .mid reads with a sane structure | 70 | 20 | none |
| B2 | 227 | tests/test_midi.cpp:85 | midi: every corpus .mid reads with a sane structure | 70 | 20 | none |
| B2 | 243 | tests/test_midi.cpp:87 | midi: every corpus .mid reads with a sane structure | 70 | 20 | none |
| B2 | 243 | tests/test_midi.cpp:98 | midi: from_file on a missing path fails with the file owner's error | 11 | 6 | none |
| B2 | 256 | tests/test_midi.cpp:100 | midi: running status and zero-velocity note_on | 28 | 11 | none |
| B2 | 256 | tests/test_midi.cpp:116 | midi: running status and zero-velocity note_on | 28 | 11 | none |
| B2 | 269 | tests/test_midi.cpp:118 | midi: running status survives a meta event | 39 | 17 | none |
| B2 | 269 | tests/test_midi.cpp:138 | midi: running status survives a meta event | 39 | 17 | none |
| B2 | 280 | tests/test_midi.cpp:140 | midi: a recognized track_name is not displaced by a later name | 49 | 12 | none |
| B2 | 280 | tests/test_midi.cpp:153 | midi: a recognized track_name is not displaced by a later name | 49 | 12 | none |
| B2 | 293 | tests/test_midi.cpp:155 | midi: a recognized track_name is not displaced by a later name | 49 | 12 | none |
| B2 | 293 | tests/test_midi.cpp:163 | midi: a recognized track_name wins over an earlier unrecognized one | 50 | 13 | none |
| B2 | 303 | tests/test_midi.cpp:165 | midi: a recognized track_name wins over an earlier unrecognized one | 50 | 13 | none |
| B2 | 303 | tests/test_midi.cpp:175 | midi: a recognized track_name after tick 0 still wins | 50 | 13 | none |
| B2 | 313 | tests/test_midi.cpp:177 | midi: the first of two recognized track_names wins | 52 | 12 | none |
| B2 | 313 | tests/test_midi.cpp:189 | midi: the first of two recognized track_names wins | 52 | 12 | none |
| B2 | 325 | tests/test_midi.cpp:191 | midi: a track with no recognized track_name keeps its first name | 44 | 14 | none |
| B2 | 325 | tests/test_midi.cpp:197 | midi: a track with no recognized track_name keeps its first name | 44 | 14 | none |
| B2 | 335 | tests/test_midi.cpp:199 | midi: a track with no recognized track_name keeps its first name | 44 | 14 | none |
| B2 | 335 | tests/test_midi.cpp:214 | midi: a track named by either drum name, in any case, is the drum track | 53 | 13 | none |
| B2 | 348 | tests/test_midi.cpp:216 | outside every test case | | | none: not shown to fail by this run |
| B2 | 348 | tests/test_midi.cpp:250 | midi: of two drum tracks, the first in file order is the drum track | 52 | 12 | none |
| B2 | 359 | tests/test_midi.cpp:252 | midi: a drum track name wins over an earlier unrecognized one in any case | 52 | 13 | none |
| B2 | 359 | tests/test_midi.cpp:259 | midi: a drum track name wins over an earlier unrecognized one in any case | 52 | 13 | none |
| B2 | 371 | tests/test_midi.cpp:261 | midi: sysex and skipped metas do not lose time | 39 | 17 | none |
| B2 | 371 | tests/test_midi.cpp:276 | midi: sysex and skipped metas do not lose time | 39 | 17 | none |
| B2 | 381 | tests/test_midi.cpp:278 | midi: track_name is not exposed as text | 49 | 13 | none |
| B2 | 381 | tests/test_midi.cpp:300 | midi: track_name is not exposed as text | 49 | 13 | none |
| B2 | 393 | tests/test_midi.cpp:302 | midi: each text event is decoded on its own, UTF-8 first, else Windows-1252 | 76 | 23 | none |
| B2 | 393 | tests/test_midi.cpp:316 | midi: each text event is decoded on its own, UTF-8 first, else Windows-1252 | 76 | 23 | none |
| B2 | 405 | tests/test_midi.cpp:318 | midi: each string meta gets its type, mido name and attribute | 49 | 20 | none |
| B2 | 405 | tests/test_midi.cpp:356 | midi: each string meta gets its type, mido name and attribute | 49 | 20 | none |
| B2 | 418 | tests/test_midi.cpp:358 | midi: non-MIDI input is rejected | 9 | 1 | none |
| B2 | 418 | tests/test_midi.cpp:362 | midi: non-MIDI input is rejected | 9 | 1 | none |
| B2 | 429 | tests/test_midi.cpp:364 | midi: SMPTE division is rejected | 11 | 4 | none |
| B2 | 429 | tests/test_midi.cpp:372 | midi: SMPTE division is rejected | 11 | 4 | none |
| B2 | 440 | tests/test_midi.cpp:374 | midi: a five-byte delta accumulates past 32 bits, as mido does | 28 | 10 | none |
| B2 | 440 | tests/test_midi.cpp:385 | midi: a five-byte delta accumulates past 32 bits, as mido does | 28 | 10 | none |
| B2 | 451 | tests/test_midi.cpp:387 | midi: a message longer than mido's 1,000,000-byte cap refuses the file, as mido does | 32 | 7 | none |
| B2 | 451 | tests/test_midi.cpp:410 | midi: a message longer than mido's 1,000,000-byte cap refuses the file, as mido does | 32 | 7 | none |
| B2 | 463 | tests/test_midi.cpp:412 | midi: a message longer than mido's 1,000,000-byte cap refuses the file, as mido does | 32 | 7 | none |
| B2 | 463 | tests/test_midi.cpp:463 | midi: the lean reader keeps only what each track's role reads, at the same ticks | 66 | 25 | none |
| B2 | 475 | tests/test_midi.cpp:465 | midi: the lean reader keeps the notes of the first drum track by either name | 59 | 9 | none |
| B2 | 475 | tests/test_midi.cpp:472 | midi: the lean reader keeps the notes of the first drum track by either name | 59 | 9 | none |
| B2 | 486 | tests/test_midi.cpp:474 | midi: the lean reader sizes each track's messages once | 65 | 19 | none |
| B2 | 486 | tests/test_midi.cpp:501 | midi: the lean reader sizes each track's messages once | 65 | 19 | none |
| B2 | 499 | tests/test_midi.cpp:503 | midi: the lean reader refuses a file where the full reader does | 49 | 7 | none |
| B2 | 499 | tests/test_midi.cpp:530 | midi: the lean reader refuses a file where the full reader does | 49 | 7 | none |
| B2 | 511 | tests/test_midi.cpp:532 | midi: is_midi_header reads the first four bytes only | 9 | 9 | none |
| B2 | 511 | tests/test_midi.cpp:542 | midi: is_midi_header reads the first four bytes only | 9 | 9 | none |
| B2 | 522 | tests/test_midi.cpp:544 | midi: a file with no track chunk is refused by both readers | 11 | 4 | none |
| B2 | 522 | tests/test_midi.cpp:559 | midi: a file with no track chunk is refused by both readers | 11 | 4 | none |
| B2 | 532 | tests/test_midi.cpp:561 | midi: a track header cut short by the end of the file ends the chunk walk | 41 | 5 | none |
| B2 | 532 | tests/test_midi.cpp:575 | midi: a track header cut short by the end of the file ends the chunk walk | 41 | 5 | none |
| B2 | 542 | tests/test_midi.cpp:577 | midi: a track header cut short by the end of the file ends the chunk walk | 41 | 5 | none |
| B2 | 542 | tests/test_midi.cpp:600 | midi: a drum track on track 0 is not the drum track, so the chart has no notes | 110 | 21 | none |
| B2 | 553 | tests/test_midi.cpp:602 | midi: with drum tracks on track 0 and track 1, track 1 is read | 153 | 30 | none |
| B2 | 553 | tests/test_midi.cpp:614 | midi: with drum tracks on track 0 and track 1, track 1 is read | 153 | 30 | none |
| B2 | 565 | tests/test_midi.cpp:616 | midi: the events track is named events in any case, and track 0 can be it | 205 | 35 | none |
| B2 | 565 | tests/test_midi.cpp:645 | midi: the events track is named events in any case, and track 0 can be it | 205 | 35 | none |
| B2 | 577 | tests/test_midi.cpp:647 | midi: of two events tracks, only the first one's sections count | 182 | 34 | none |
| B2 | 577 | tests/test_midi.cpp:662 | midi: of two events tracks, only the first one's sections count | 182 | 34 | none |
| B2 | 587 | tests/test_midi.cpp:664 | midi: of two events tracks, only the first one's sections count | 182 | 34 | none |
| B2 | 587 | tests/test_midi.cpp:688 | midi: [section _ENDOFSONG] is not a section | 174 | 31 | none |
| B2 | 598 | tests/test_midi.cpp:690 | midi: a section text holding _ENDOFSONG in its middle is not a section | 174 | 31 | none |
| B2 | 598 | tests/test_midi.cpp:695 | midi: a section text holding _ENDOFSONG in its middle is not a section | 174 | 31 | none |
| B2 | 608 | tests/test_midi.cpp:697 | midi: [section _endofsong] in lower case stays a section | 174 | 30 | none |
| B2 | 608 | tests/test_midi.cpp:705 | midi: [section _endofsong] in lower case stays a section | 174 | 30 | none |
| B2 | 620 | tests/test_midi.cpp:707 | midi: [prc__ENDOFSONG] stays a section | 175 | 31 | none |
| B2 | 620 | tests/test_midi.cpp:713 | midi: [prc__ENDOFSONG] stays a section | 175 | 31 | none |
| B2 | 631 | tests/test_midi.cpp:715 | midi: [prc__ENDOFSONG] stays a section | 175 | 31 | none |
| B2 | 631 | tests/test_midi.cpp:737 | midi: [end] on the events track is the end event, first one wins | 198 | 36 | none |
| B2 | 644 | tests/test_midi.cpp:739 | midi: the lean read keeps the end event | 176 | 34 | none |
| B2 | 644 | tests/test_midi.cpp:763 | midi: the lean read keeps the end event | 176 | 34 | none |
| B2 | 656 | tests/test_midi.cpp:765 | midi: the lean read keeps the end event | 176 | 34 | none |
| B2 | 656 | tests/test_midi.cpp:798 | midi: a bass sustain past the last drum note is the last note end; BEAT, VENUE and EVENTS don't count | 192 | 37 | none |
| B2 | 669 | tests/test_midi.cpp:800 | midi: the last note end reads each instrument once, from track 1, by instrument name | 159 | 40 | none |
| B2 | 669 | tests/test_midi.cpp:859 | midi: the last note end reads each instrument once, from track 1, by instrument name | 159 | 40 | none |
| B2 | 684 | tests/test_midi.cpp:861 | midi: the last note end reads each instrument once, from track 1, by instrument name | 159 | 40 | none |
| B2 | 684 | tests/test_midi.cpp:907 | mid: a lane's velocity picks the lowest difficulty that shows it | 182 | 39 | none |
| B2 | 696 | tests/test_midi.cpp:909 | mid: 126 is a single lane and 127 a double, note-on to note-off | 187 | 40 | none |
| B2 | 696 | tests/test_midi.cpp:926 | mid: 126 is a single lane and 127 a double, note-on to note-off | 187 | 40 | none |
| B2 | 719 | tests/test_pathstring_read.cpp:24 | outside every test case | | | none: not shown to fail by this run |
| B2 | 719 | tests/test_pathstring_read.cpp:57 | outside every test case | | | none: not shown to fail by this run |
| B2 | 734 | tests/test_pathstring_read.cpp:59 | outside every test case | | | none: not shown to fail by this run |
| B2 | 734 | tests/test_pathstring_read.cpp:77 | outside every test case | | | none: not shown to fail by this run |
| B2 | 748 | tests/test_pathstring_read.cpp:97 | pathstring read: a token no fill can place is refused with its position | 344 | 42 | none |
| B2 | 748 | tests/test_pathstring_read.cpp:111 | pathstring read: every refusal is ErrorKind::PathTextRefused | 273 | 7 | none |
| B2 | 760 | tests/test_pathstring_read.cpp:113 | pathstring read: every refusal is ErrorKind::PathTextRefused | 273 | 7 | none |
| B2 | 760 | tests/test_pathstring_read.cpp:122 | pathstring read: "(No activations.)" gives a path with no activations | 194 | 7 | none |
| B2 | 771 | tests/test_pathstring_read.cpp:124 | pathstring read: "(No activations.)" gives a path with no activations | 194 | 7 | none |
| B2 | 771 | tests/test_pathstring_read.cpp:130 | pathstring read: every corpus path round-trips from Hydra's long Copy path text | 379 | 68 | none |
| B2 | 783 | tests/test_pathstring_read.cpp:132 | pathstring read: every corpus path round-trips from Hydra's long Copy path text | 379 | 68 | none |
| B2 | 783 | tests/test_pathstring_read.cpp:143 | pathstring read: long text with no activations gives a path with no activations | 209 | 18 (7 through shared state) | none |
| B2 | 794 | tests/test_pathstring_read.cpp:145 | pathstring read: long text with no activations gives a path with no activations | 209 | 18 (7 through shared state) | none |
| B2 | 794 | tests/test_pathstring_read.cpp:169 | pathstring read: a long-text window no fill can place is refused with its position | 359 | 47 | none |
| B2 | 804 | tests/test_pathstring_read.cpp:171 | pathstring read: a long-text window no fill can place is refused with its position | 359 | 47 | none |
| B2 | 804 | tests/test_pathstring_read.cpp:198 | pathstring read: the graph entry point reads the same path as the settings one | 383 | 67 | none |
| B2 | 817 | tests/test_pathstring_read.cpp:200 | pathstring read: the graph entry point reads the same path as the settings one | 383 | 67 | none |
| B2 | 817 | tests/test_pathstring_read.cpp:208 | pathstring read: the graph entry point refuses a token no fill can place | 344 | 40 | none |
| B2 | 829 | tests/test_pathstring_read.cpp:210 | pathstring read: the graph entry point refuses a token no fill can place | 344 | 40 | none |
| B2 | 829 | tests/test_pathstring_read.cpp:218 | pathstring read: long text with the wrong number of sections is refused | 73 | 3 | none |
| B4 | 38 | tests/test_replay.cpp:851 | paths_json writes the end ratio the search stored | 24 | 1 | none |
| B4 | 38 | tests/test_replay.cpp:903 | paths_json prints the stored passed-over fills, not a guess | 265 | 18 | none |
| B4 | 56 | tests/test_replay_schedule.cpp:905 | schedule: the late press holds sqout_late_notes | 91 | 17 | none |
| B4 | 56 | tests/test_replay_schedule.cpp:937 | schedule: the late press holds sqout_late_notes | 91 | 17 | none |
| B4 | 73 | tests/test_replay_schedule.cpp:939 | schedule: a whole_chord chord with no inside group has an empty inside list and an empty join_gems | 91 | 17 | none |
| B4 | 73 | tests/test_replay_schedule.cpp:962 | schedule: a whole_chord chord with no inside group has an empty inside list and an empty join_gems | 91 | 17 | none |
| B4 | 90 | tests/test_replay_schedule.cpp:964 | schedule: first_note writes the same schedule as before | 137 | 43 (2 through shared state) | none |
| B4 | 90 | tests/test_replay_schedule.cpp:1003 | schedule: first_note writes the same schedule as before | 137 | 43 (2 through shared state) | none |
| B4 | 108 | tests/test_replay_schedule.cpp:1005 | schedule: hydra_replay_build is written | 63 | 12 | none |
| B4 | 108 | tests/test_replay_schedule.cpp:1014 | schedule: hydra_replay_build is written | 63 | 12 | none |
| B4 | 126 | tests/test_replay_schedule.cpp:1062 | schedule: one frame everywhere | 3 | 0: not shown to fail by this run | none |
| B4 | 126 | tests/test_replay_schedule.cpp:1096 | schedule: one frame everywhere | 3 | 0: not shown to fail by this run | none |
| B4 | 144 | tests/test_replay_schedule.cpp:1098 | schedule: far press follows TM-50 | 4 | 0: not shown to fail by this run | none |
| B4 | 144 | tests/test_replay_schedule.cpp:1128 | schedule: far press follows TM-50 | 4 | 0: not shown to fail by this run | none |
| B4 | 162 | tests/test_replay_schedule.cpp:1130 | schedule: every skip writes a refusal and a flag | 137 | 49 (8 through shared state) | none |
| B4 | 162 | tests/test_replay_schedule.cpp:1270 | schedule: every skip writes a refusal and a flag | 137 | 49 (8 through shared state) | none |
| B4 | 180 | tests/test_replay_schedule.cpp:1272 | schedule: a squeeze-out that is also a skipped fill | 91 | 18 | none |
| B4 | 180 | tests/test_replay_schedule.cpp:1343 | schedule: a squeeze-out that is also a skipped fill | 91 | 18 | none |
| B4 | 198 | tests/test_replay_schedule.cpp:1345 | schedule: one decline rule | 66 | 13 | none |
| B4 | 198 | tests/test_replay_schedule.cpp:1376 | schedule: one decline rule | 66 | 13 | none |
| B4 | 216 | tests/test_replay_schedule.cpp:1378 | schedule: the first press group is the activation note | 137 | 47 (8 through shared state) | none |
| B4 | 216 | tests/test_replay_schedule.cpp:1425 | schedule: the first press group is the activation note | 137 | 47 (8 through shared state) | none |
| B4 | 234 | tests/test_replay_schedule.cpp:1514 | schedule: Pitch Dark prices each read at its own step's scale | 114 | 43 (7 through shared state) | none |
| B4 | 234 | tests/test_replay_schedule.cpp:1619 | schedule: Pitch Dark prices each read at its own step's scale | 114 | 43 (7 through shared state) | none |
| B4 | 252 | tests/test_replay_schedule.cpp:1621 | schedule: Pitch Dark's read range is its read end's steady span | 114 | 43 (10 through shared state) | none |
| B4 | 252 | tests/test_replay_schedule.cpp:1649 | schedule: Pitch Dark's read range is its read end's steady span | 114 | 43 (10 through shared state) | none |
| B4 | 270 | tests/test_replay_schedule.cpp:1651 | schedule: a lead that moves the read end past a tempo change is refused naming its span | 1 | 0: not shown to fail by this run | none |
| B4 | 270 | tests/test_replay_schedule.cpp:1678 | schedule: a lead that moves the read end past a tempo change is refused naming its span | 1 | 0: not shown to fail by this run | none |
| B4 | 287 | tests/test_replay_schedule.cpp:1680 | schedule: a tempo change on an end splits the conversion's sides | 91 | 18 | none |
| B4 | 287 | tests/test_replay_schedule.cpp:1714 | schedule: a tempo change on an end splits the conversion's sides | 91 | 18 | none |
| B4 | 305 | tests/test_replay_schedule.cpp:1716 | schedule: read_at_own_ms runs the conversion backwards on its own side | 0 | 0: not shown to fail by this run | none |
| B4 | 305 | tests/test_replay_schedule.cpp:1731 | schedule: read_at_own_ms runs the conversion backwards on its own side | 0 | 0: not shown to fail by this run | none |
| C1 | 22 | tests/test_activation_row_layout.cpp:1 | outside every test case | | | none: not shown to fail by this run |
| C1 | 22 | tests/test_activation_row_layout.cpp:100 | activation row: the layout scales with the DPI | 2 | 0: not shown to fail by this run | none |
| C1 | 53 | tests/test_activation_row_layout.cpp:83 | activation row: an 11-character measure pushes the bars past it | 2 | 0: not shown to fail by this run | none |
| C1 | 53 | tests/test_activation_row_layout.cpp:90 | activation row: an 11-character measure pushes the bars past it | 2 | 0: not shown to fail by this run | none |
| C1 | 79 | tests/test_activation_row_layout.cpp:92 | activation row: the layout scales with the DPI | 2 | 0: not shown to fail by this run | none |
| C1 | 79 | tests/test_activation_row_layout.cpp:99 | activation row: the layout scales with the DPI | 2 | 0: not shown to fail by this run | none |
| C1 | 104 | tests/test_activation_row_layout.cpp:101 | activation row: no badge leaves nothing to place and one line | 2 | 0: not shown to fail by this run | none |
| C1 | 104 | tests/test_activation_row_layout.cpp:106 | activation row: no badge leaves nothing to place and one line | 2 | 0: not shown to fail by this run | none |
| C1 | 127 | tests/test_activation_row_layout.cpp:108 | activation row: one badge that fits sits where it always has (B1) | 2 | 0: not shown to fail by this run | none |
| C1 | 127 | tests/test_activation_row_layout.cpp:121 | activation row: one badge that fits sits where it always has (B1) | 2 | 0: not shown to fail by this run | none |
| C1 | 158 | tests/test_activation_row_layout.cpp:123 | activation row: badges beside the bars never overlap them (B3) | 2 | 0: not shown to fail by this run | none |
| C1 | 158 | tests/test_activation_row_layout.cpp:133 | activation row: badges beside the bars never overlap them (B3) | 2 | 0: not shown to fail by this run | none |
| C1 | 186 | tests/test_activation_row_layout.cpp:135 | activation row: a pill that just touches the bars drops under the measure | 2 | 0: not shown to fail by this run | none |
| C1 | 186 | tests/test_activation_row_layout.cpp:145 | activation row: a pill that just touches the bars drops under the measure | 2 | 0: not shown to fail by this run | none |
| C1 | 214 | tests/test_activation_row_layout.cpp:147 | activation row: a short badge drops when the longest badge's pill would not fit | 2 | 0: not shown to fail by this run | none |
| C1 | 214 | tests/test_activation_row_layout.cpp:157 | activation row: a short badge drops when the longest badge's pill would not fit | 2 | 0: not shown to fail by this run | none |
| C1 | 246 | tests/test_activation_row_layout.cpp:159 | activation row: badges under the measure clear the bars by the row's top padding | 2 | 0: not shown to fail by this run | none |
| C1 | 246 | tests/test_activation_row_layout.cpp:186 | activation row: badges under the measure clear the bars by the row's top padding | 2 | 0: not shown to fail by this run | none |
| C1 | 273 | tests/test_activation_row_layout.cpp:188 | activation row: no pill runs past the flush-right edge | 2 | 0: not shown to fail by this run | none |
| C1 | 273 | tests/test_activation_row_layout.cpp:207 | activation row: no pill runs past the flush-right edge | 2 | 0: not shown to fail by this run | none |
| C1 | 293 | tests/test_activation_row_layout.cpp:209 | activation row: pills never overlap, on one line or between lines | 2 | 0: not shown to fail by this run | none |
| C1 | 293 | tests/test_activation_row_layout.cpp:232 | activation row: pills never overlap, on one line or between lines | 2 | 0: not shown to fail by this run | none |
| C1 | 315 | tests/test_activation_row_layout.cpp:234 | activation row: badges under the measure start at the measure (D257) | 2 | 0: not shown to fail by this run | none |
| C1 | 315 | tests/test_activation_row_layout.cpp:254 | activation row: badges under the measure start at the measure (D257) | 2 | 0: not shown to fail by this run | none |
| C1 | 338 | tests/test_activation_row_layout.cpp:256 | activation row: every row of a path puts its bars at the same x | 2 | 0: not shown to fail by this run | none |
| C1 | 338 | tests/test_activation_row_layout.cpp:261 | activation row: every row of a path puts its bars at the same x | 2 | 0: not shown to fail by this run | none |
| C1 | 367 | tests/test_badge_pill.cpp:1 | outside every test case | | | none: not shown to fail by this run |
| C1 | 367 | tests/test_badge_pill.cpp:165 | badge pill: one badge alone sits flush right | 2 | 0: not shown to fail by this run | none |
| C1 | 392 | tests/test_badge_pill.cpp:80 | badge pill: a badge wider than the pill text width grows the pill | 2 | 0: not shown to fail by this run | none |
| C1 | 392 | tests/test_badge_pill.cpp:84 | badge pill: a badge wider than the pill text width grows the pill | 2 | 0: not shown to fail by this run | none |
| C1 | 414 | tests/test_badge_pill.cpp:86 | badge pill: text sits centred in its pill | 2 | 0: not shown to fail by this run | none |
| C1 | 414 | tests/test_badge_pill.cpp:104 | badge pill: text sits centred in its pill | 2 | 0: not shown to fail by this run | none |
| C1 | 435 | tests/test_badge_pill.cpp:106 | badge pill: pills side by side match and keep one gap between them | 2 | 0: not shown to fail by this run | none |
| C1 | 435 | tests/test_badge_pill.cpp:117 | badge pill: pills side by side match and keep one gap between them | 2 | 0: not shown to fail by this run | none |
| C1 | 456 | tests/test_badge_pill.cpp:119 | badge pill: stacked pills share both edges | 2 | 0: not shown to fail by this run | none |
| C1 | 456 | tests/test_badge_pill.cpp:130 | badge pill: stacked pills share both edges | 2 | 0: not shown to fail by this run | none |
| C1 | 475 | tests/test_badge_pill.cpp:144 | window tag: safe and short take the mockup's colours | 0 | 0: not shown to fail by this run | none |
| C1 | 475 | tests/test_badge_pill.cpp:153 | window tag: safe and short take the mockup's colours | 0 | 0: not shown to fail by this run | none |
| C1 | 511 | tests/test_badge_pill.cpp:155 | badge pill: one badge alone sits flush right | 2 | 0: not shown to fail by this run | none |
| C1 | 511 | tests/test_badge_pill.cpp:165 | badge pill: one badge alone sits flush right | 2 | 0: not shown to fail by this run | none |
| C1 | 539 | tests/test_app_settings.cpp:49 | commit_settings analyzes the open song again when the chart mode changes | 910 | 103 | none |
| C1 | 539 | tests/test_app_settings.cpp:73 | commit_settings analyzes the open song again when the chart mode changes | 910 | 103 | none |
| C1 | 573 | tests/test_app_settings.cpp:75 | commit_settings analyzes the open song again on an SP cap change | 885 | 83 | none |
| C1 | 573 | tests/test_app_settings.cpp:88 | commit_settings analyzes the open song again on an SP cap change | 885 | 83 | none |
| C1 | 598 | tests/test_app_settings.cpp:90 | commit_settings analyzes again when the ms limit or the score range changes | 900 | 92 | none |
| C1 | 598 | tests/test_app_settings.cpp:114 | commit_settings analyzes again when the ms limit or the score range changes | 900 | 92 | none |
| C1 | 623 | tests/test_app_settings.cpp:116 | commit_settings analyzes again when the ms limit or the score range changes | 900 | 92 | none |
| C1 | 623 | tests/test_app_settings.cpp:172 | a burst of setting changes ends with one analysis of the final settings | 886 | 78 | none |
| C1 | 653 | tests/test_app_settings.cpp:174 | a burst of setting changes ends with one analysis of the final settings | 886 | 78 | none |
| C1 | 653 | tests/test_app_settings.cpp:192 | number boxes apply at once but write the INI only on flush | 900 | 42 | none |
| C1 | 676 | tests/test_app_settings.cpp:194 | number boxes apply at once but write the INI only on flush | 900 | 42 | none |
| C1 | 676 | tests/test_app_settings.cpp:223 | a number setting edited outside its range lands on the edge the file loader uses | 230 | 31 | none |
| C1 | 706 | tests/test_app_settings.cpp:225 | a number setting edited outside its range lands on the edge the file loader uses | 230 | 31 | none |
| C1 | 706 | tests/test_app_settings.cpp:241 | a settings change whose reads fail shows no other settings' record | 205 | 14 | none |
| C1 | 742 | tests/test_audio_decode.cpp:46 | sniff_format classifies audio containers by their magic bytes | 36 | 15 | none |
| C1 | 742 | tests/test_audio_decode.cpp:66 | sniff_format classifies audio containers by their magic bytes | 36 | 15 | none |
| C1 | 764 | tests/test_audio_decode.cpp:68 | sniff_format: a FLAC behind an ID3 tag is Flac, a tagged MP3 stays Mp3 | 26 | 12 | none |
| C1 | 764 | tests/test_audio_decode.cpp:80 | sniff_format: a FLAC behind an ID3 tag is Flac, a tagged MP3 stays Mp3 | 26 | 12 | none |
| C1 | 785 | tests/test_audio_decode.cpp:82 | audio_sniff: the ID3v2 tag length counts the header, the size and the footer | 19 | 11 | none |
| C1 | 785 | tests/test_audio_decode.cpp:91 | audio_sniff: the ID3v2 tag length counts the header, the size and the footer | 19 | 11 | none |
| C1 | 806 | tests/test_audio_decode.cpp:93 | decode_audio: PCM16 WAV decodes to matching float samples | 22 | 6 | none |
| C1 | 806 | tests/test_audio_decode.cpp:105 | decode_audio: PCM16 WAV decodes to matching float samples | 22 | 6 | none |
| C1 | 830 | tests/test_audio_decode.cpp:107 | decode_audio: MP3 fixture decodes to the 220 Hz sine | 89 | 9 | none |
| C1 | 830 | tests/test_audio_decode.cpp:115 | decode_audio: MP3 fixture decodes to the 220 Hz sine | 89 | 9 | none |
| C1 | 856 | tests/test_audio_decode.cpp:117 | decode_audio: OGG Vorbis fixture decodes to the 220 Hz sine | 38 | 16 | none |
| C1 | 856 | tests/test_audio_decode.cpp:125 | decode_audio: OGG Vorbis fixture decodes to the 220 Hz sine | 38 | 16 | none |
| C1 | 874 | tests/test_audio_decode.cpp:127 | decode_stem: a file-path stem and a bytes stem decode identically | 45 | 20 | none |
| C1 | 874 | tests/test_audio_decode.cpp:149 | decode_stem: a file-path stem and a bytes stem decode identically | 45 | 20 | none |
| C1 | 895 | tests/test_audio_decode.cpp:151 | decode_audio: Ogg-Opus fixture decodes to the 220 Hz sine at 48 kHz | 100 | 41 | none |
| C1 | 895 | tests/test_audio_decode.cpp:159 | decode_audio: Ogg-Opus fixture decodes to the 220 Hz sine at 48 kHz | 100 | 41 | none |
| C1 | 914 | tests/test_audio_decode.cpp:161 | decode_audio: Ogg-Opus fixture decodes to the 220 Hz sine at 48 kHz | 100 | 41 | none |
| C1 | 914 | tests/test_audio_decode.cpp:179 | decode_audio: Ogg-Opus output is pinned bit for bit | 100 | 43 | none |
| C1 | 944 | tests/test_audio_device.cpp:1 | outside every test case | | | none: not shown to fail by this run |
| C1 | 944 | tests/test_audio_device.cpp:46 | PreviewAudioDevice opens and starts the default output device | 6 | 3 | none |
| C1 | 977 | tests/test_audio_mixer.cpp:55 | mix_stems sums same-format stems and zero-extends the shorter | 7 | 0: not shown to fail by this run | none |
| C1 | 977 | tests/test_audio_mixer.cpp:67 | mix_stems sums same-format stems and zero-extends the shorter | 7 | 0: not shown to fail by this run | none |
| C1 | 1000 | tests/test_audio_mixer.cpp:69 | stem_converter_config: same rate and channels is a passthrough, anything else converts | 3 | 3 | none |
| C1 | 1000 | tests/test_audio_mixer.cpp:79 | stem_converter_config: same rate and channels is a passthrough, anything else converts | 3 | 3 | none |
| C1 | 1019 | tests/test_audio_mixer.cpp:81 | mix_stems of no stems is empty at the requested format | 0 | 0: not shown to fail by this run | none |
| C1 | 1019 | tests/test_audio_mixer.cpp:86 | mix_stems of no stems is empty at the requested format | 0 | 0: not shown to fail by this run | none |
| C1 | 1038 | tests/test_audio_mixer.cpp:88 | mix_stems resamples to the output rate and unifies channels | 9 | 2 | none |
| C1 | 1038 | tests/test_audio_mixer.cpp:104 | mix_stems resamples to the output rate and unifies channels | 9 | 2 | none |
| C1 | 1060 | tests/test_audio_mixer.cpp:106 | mix_stems adds hand-built stems bit for bit; a zero-channel stem adds nothing | 7 | 3 | none |
| C1 | 1060 | tests/test_audio_mixer.cpp:117 | mix_stems adds hand-built stems bit for bit; a zero-channel stem adds nothing | 7 | 3 | none |
| C1 | 1086 | tests/test_audio_player.cpp:18 | Playhead starts paused at the start with the mix's format | 2 | 0: not shown to fail by this run | none |
| C1 | 1086 | tests/test_audio_player.cpp:27 | Playhead starts paused at the start with the mix's format | 2 | 0: not shown to fail by this run | none |
| C1 | 1105 | tests/test_audio_player.cpp:29 | Playhead starts paused at the start with the mix's format | 2 | 0: not shown to fail by this run | none |
| C1 | 1105 | tests/test_audio_player.cpp:40 | ms_of_frames and frames_of_ms convert at the sample rate | 0 | 0: not shown to fail by this run | none |
| C1 | 1126 | tests/test_audio_player.cpp:42 | play, pause, and toggle drive the playhead state | 3 | 1 | none |
| C1 | 1126 | tests/test_audio_player.cpp:52 | play, pause, and toggle drive the playhead state | 3 | 1 | none |
| C1 | 1141 | tests/test_audio_player.cpp:54 | read_frames while paused writes silence and does not advance | 4 | 0: not shown to fail by this run | none |
| C1 | 1141 | tests/test_audio_player.cpp:61 | read_frames while paused writes silence and does not advance | 4 | 0: not shown to fail by this run | none |
| C1 | 1160 | tests/test_audio_player.cpp:63 | read_frames while playing copies frames and advances the clock | 10 | 0: not shown to fail by this run | none |
| C1 | 1160 | tests/test_audio_player.cpp:74 | read_frames while playing copies frames and advances the clock | 10 | 0: not shown to fail by this run | none |
| C1 | 1181 | tests/test_audio_player.cpp:76 | read_frames past the end zero-fills, auto-pauses, clamps position | 10 | 1 | none |
| C1 | 1181 | tests/test_audio_player.cpp:89 | read_frames past the end zero-fills, auto-pauses, clamps position | 10 | 1 | none |
| C1 | 1201 | tests/test_audio_player.cpp:91 | outside every test case | | | none: not shown to fail by this run |
| C1 | 1201 | tests/test_audio_player.cpp:117 | read_frames counts only the frames a short source really gave | 9 | 0: not shown to fail by this run | none |
| C1 | 1225 | tests/test_audio_player.cpp:119 | seek clamps to the valid range in both frames and ms | 3 | 0: not shown to fail by this run | none |
| C1 | 1225 | tests/test_audio_player.cpp:129 | seek clamps to the valid range in both frames and ms | 3 | 0: not shown to fail by this run | none |
| C1 | 1245 | tests/test_audio_player.cpp:131 | seek clamps to the valid range in both frames and ms | 3 | 0: not shown to fail by this run | none |
| C1 | 1245 | tests/test_audio_player.cpp:142 | frames_of_ms and ms_of_frames: pinned at 48000 and 44100 | 0 | 0: not shown to fail by this run | none |
| C1 | 1269 | tests/test_audio_player.cpp:144 | Playhead applies the output gain to served frames | 11 | 2 | none |
| C1 | 1269 | tests/test_audio_player.cpp:160 | Playhead applies the output gain to served frames | 11 | 2 | none |
| C1 | 1295 | tests/test_build_flags.cpp:1 | outside every test case | | | none: not shown to fail by this run |
| C1 | 1295 | tests/test_build_flags.cpp:57 | a Release exe stores its .pdb's bare file name | 0 | 0: not shown to fail by this run | none |
| C10 | 13 | tests/test_preview_config.cpp:1 | outside every test case | | | none: not shown to fail by this run |
| C10 | 13 | tests/test_preview_config.cpp:98 | the shipped 3d-config.json loads to the Onyx values | 34 | 17 | none |
| C10 | 77 | tests/test_preview_config.cpp:100 | load_preview_config refuses a missing key and names it | 34 | 8 | none |
| C10 | 77 | tests/test_preview_config.cpp:121 | load_preview_config refuses a missing key and names it | 34 | 8 | none |
| C10 | 113 | tests/test_preview_config.cpp:123 | load_preview_config rejects malformed JSON | 0 | 0: not shown to fail by this run | none |
| C10 | 113 | tests/test_preview_config.cpp:125 | load_preview_config rejects malformed JSON | 0 | 0: not shown to fail by this run | none |
| C10 | 141 | tests/test_preview_config.cpp:127 | parse_hex_color follows Onyx stackColor | 20 | 13 | none |
| C10 | 141 | tests/test_preview_config.cpp:132 | parse_hex_color follows Onyx stackColor | 20 | 13 | none |
| C10 | 172 | tests/test_preview_config.cpp:134 | load_preview_config reads the time box size and margin | 34 | 7 | none |
| C10 | 172 | tests/test_preview_config.cpp:141 | load_preview_config reads the time box size and margin | 34 | 7 | none |
| C10 | 204 | tests/test_rules_file.cpp:1 | outside every test case | | | none: not shown to fail by this run |
| C10 | 204 | tests/test_rules_file.cpp:36 | each retired rules key loads silently, whatever it says | 13 | 4 | none |
| C10 | 245 | tests/test_rules_file.cpp:38 | a key that is neither live nor retired still stops the rules file | 12 | 3 | none |
| C10 | 245 | tests/test_rules_file.cpp:40 | a key that is neither live nor retired still stops the rules file | 12 | 3 | none |
| C10 | 269 | tests/test_rules_file.cpp:42 | a key that is neither live nor retired still stops the rules file | 12 | 3 | none |
| C10 | 269 | tests/test_rules_file.cpp:63 | the rules file refuses below each of its five floors and reads the floor itself | 26 | 10 | none |
| C10 | 311 | tests/test_rules_file.cpp:65 | the rules file refuses below each of its five floors and reads the floor itself | 26 | 10 | none |
| C10 | 311 | tests/test_rules_file.cpp:93 | a bad rules line refuses in the settings file's shapes | 24 | 9 | none |
| C10 | 351 | tests/test_section_names.cpp:1 | outside every test case | | | none: not shown to fail by this run |
| C10 | 351 | tests/test_section_names.cpp:41 | section names: the generated table agrees with the TSV row for row | 0 | 0: not shown to fail by this run | none |
| C10 | 391 | tests/test_section_names.cpp:43 | section names: a listed marker shows Clone Hero's words | 9 | 2 | none |
| C10 | 391 | tests/test_section_names.cpp:48 | section names: a listed marker shows Clone Hero's words | 9 | 2 | none |
| C10 | 423 | tests/test_section_names.cpp:50 | section names: an unlisted marker shows its name with spaces for underscores | 9 | 1 | none |
| C10 | 423 | tests/test_section_names.cpp:58 | section names: an unlisted marker shows its name with spaces for underscores | 9 | 1 | none |
| C10 | 449 | tests/test_section_names.cpp:60 | section names: only the two prefixes mark a section | 5 | 3 | none |
| C10 | 449 | tests/test_section_names.cpp:72 | section names: only the two prefixes mark a section | 5 | 3 | none |
| C10 | 477 | tests/test_section_names.cpp:74 | section names: only the two prefixes mark a section | 5 | 3 | none |
| C10 | 477 | tests/test_section_names.cpp:88 | section names: the .mid reader drops a section body holding _ENDOFSONG | 9 | 4 | none |
| C10 | 511 | tests/test_preview_source.cpp:79 | is_container: a .sng or .srb is a container, a loose notes file is not | 13 | 5 | none |
| C10 | 511 | tests/test_preview_source.cpp:86 | is_container: a .sng or .srb is a container, a loose notes file is not | 13 | 5 | none |
| C10 | 539 | tests/test_preview_source.cpp:88 | is_audio_filename / looks_like_audio recognize the formats | 38 | 17 | none |
| C10 | 539 | tests/test_preview_source.cpp:111 | is_audio_filename / looks_like_audio recognize the formats | 38 | 17 | none |
| C10 | 575 | tests/test_preview_source.cpp:113 | is_song_stem: an audio file that is not a standalone preview clip | 16 | 7 | none |
| C10 | 575 | tests/test_preview_source.cpp:120 | is_song_stem: an audio file that is not a standalone preview clip | 16 | 7 | none |
| C10 | 598 | tests/test_preview_source.cpp:122 | find_loose_audio: stems beside the notes, preview and art excluded | 28 | 12 | none |
| C10 | 598 | tests/test_preview_source.cpp:138 | find_loose_audio: stems beside the notes, preview and art excluded | 28 | 12 | none |
| C10 | 623 | tests/test_preview_source.cpp:140 | extract_sng_audio: audio entries come back XOR-demasked | 337 | 81 (6 through shared state) | none |
| C10 | 623 | tests/test_preview_source.cpp:168 | extract_sng_audio: audio entries come back XOR-demasked | 337 | 81 (6 through shared state) | none |
| C10 | 651 | tests/test_preview_source.cpp:170 | extract_srb_audio: trailing audio streams inflate; art is skipped | 324 | 60 (6 through shared state) | none |
| C10 | 651 | tests/test_preview_source.cpp:192 | extract_srb_audio: trailing audio streams inflate; art is skipped | 324 | 60 (6 through shared state) | none |
| C10 | 676 | tests/test_preview_source.cpp:194 | extract_srb_audio: a stream with an audio magic but no codec is not a stem | 88 | 31 (6 through shared state) | none |
| C10 | 676 | tests/test_preview_source.cpp:208 | extract_srb_audio: a stream with an audio magic but no codec is not a stem | 88 | 31 (6 through shared state) | none |
| C10 | 696 | tests/test_preview_source.cpp:210 | extract_srb_audio: a stream with an audio magic but no codec is not a stem | 88 | 31 (6 through shared state) | none |
| C10 | 696 | tests/test_preview_source.cpp:228 | resolve_preview_source: a .srb with no extractable audio falls back to a loose file beside it | 326 | 61 (1 through shared state) | none |
| C10 | 723 | tests/test_preview_source.cpp:230 | resolve_preview_source: a loose chart parses and finds its audio | 322 | 59 (2 through shared state) | none |
| C10 | 723 | tests/test_preview_source.cpp:242 | resolve_preview_source: a loose chart parses and finds its audio | 322 | 59 (2 through shared state) | none |
| C10 | 747 | tests/test_preview_source.cpp:244 | containers pass the difficulty through to the chart inside | 330 | 70 | none |
| C10 | 747 | tests/test_preview_source.cpp:274 | containers pass the difficulty through to the chart inside | 330 | 70 | none |
| C10 | 777 | tests/test_preview_source.cpp:276 | read_ini_delay_ms reads song.ini delay in milliseconds | 63 | 8 | none |
| C10 | 777 | tests/test_preview_source.cpp:295 | read_ini_delay_ms reads song.ini delay in milliseconds | 63 | 8 | none |
| C10 | 804 | tests/test_preview_source.cpp:297 | preview_audio_offset_ms combines delay and Offset | 0 | 0: not shown to fail by this run | none |
| C10 | 804 | tests/test_preview_source.cpp:307 | preview_audio_offset_ms combines delay and Offset | 0 | 0: not shown to fail by this run | none |
| C10 | 838 | tests/test_preview_source.cpp:309 | resolve_preview_source reads delay from a song.ini in any case | 278 | 55 | none |
| C10 | 838 | tests/test_preview_source.cpp:316 | resolve_preview_source reads delay from a song.ini in any case | 278 | 55 | none |
| C10 | 860 | tests/test_preview_source.cpp:318 | sng_delay_ms reads the delay key in any case | 27 | 6 | none |
| C10 | 860 | tests/test_preview_source.cpp:330 | sng_delay_ms reads the delay key in any case | 27 | 6 | none |
| C10 | 883 | tests/test_preview_source.cpp:332 | resolve_preview_source: a .sng's metadata delay replaces the chart Offset | 263 | 70 | none |
| C10 | 883 | tests/test_preview_source.cpp:352 | resolve_preview_source: a .sng's metadata delay replaces the chart Offset | 263 | 70 | none |
| C10 | 907 | tests/test_preview_source.cpp:354 | resolve_preview_source: a .srb uses its chart's Offset | 287 | 55 | none |
| C10 | 907 | tests/test_preview_source.cpp:359 | resolve_preview_source: a .srb uses its chart's Offset | 287 | 55 | none |
| C10 | 927 | tests/test_preview_source.cpp:361 | outside every test case | | | none: not shown to fail by this run |
| C10 | 927 | tests/test_preview_source.cpp:436 | resolve_preview_source reads a .sng or .srb from disk once | 347 | 88 | none |
| C10 | 954 | tests/test_preview_source.cpp:438 | resolve_preview_source reads a .sng or .srb from disk once | 347 | 88 | none |
| C10 | 954 | tests/test_preview_source.cpp:468 | the song and stem halves share one container read and match the whole | 330 | 88 | none |
| C10 | 984 | tests/test_preview_source.cpp:361 | outside every test case | | | none: not shown to fail by this run |
| C10 | 984 | tests/test_preview_source.cpp:392 | outside every test case | | | none: not shown to fail by this run |
| C10 | 1022 | tests/test_preview_transport.cpp:34 | an unloaded transport plays and scrubs with no audio | 12 | 8 | none |
| C10 | 1022 | tests/test_preview_transport.cpp:53 | an unloaded transport plays and scrubs with no audio | 12 | 8 | none |
| C10 | 1045 | tests/test_preview_transport.cpp:55 | the transport's end is the song's length; unload clears it | 10 | 1 | none |
| C10 | 1045 | tests/test_preview_transport.cpp:75 | the transport's end is the song's length; unload clears it | 10 | 1 | none |
| C10 | 1070 | tests/test_preview_transport.cpp:77 | the transport's end is the song's length; unload clears it | 10 | 1 | none |
| C10 | 1070 | tests/test_preview_transport.cpp:108 | set_song_length_ms moves the end and pulls the clock back to it | 15 | 1 | none |
| C10 | 1092 | tests/test_preview_transport.cpp:110 | set_song_length_ms moves the end and pulls the clock back to it | 15 | 1 | none |
| C10 | 1092 | tests/test_preview_transport.cpp:126 | playback stops at the song's length even when the audio runs longer | 17 | 0: not shown to fail by this run | none |
| C10 | 1114 | tests/test_preview_transport.cpp:128 | playback stops at the song's length even when the audio runs longer | 17 | 0: not shown to fail by this run | none |
| C10 | 1114 | tests/test_preview_transport.cpp:140 | with no song length the clock runs free past the audio's end | 8 | 4 | none |
| C10 | 1135 | tests/test_preview_transport.cpp:142 | play seeks the playhead to the clock and both run | 9 | 2 | none |
| C10 | 1135 | tests/test_preview_transport.cpp:168 | play seeks the playhead to the clock and both run | 9 | 2 | none |
| C10 | 1154 | tests/test_preview_transport.cpp:170 | tick pauses at the end and pins the time | 17 | 2 | none |
| C10 | 1154 | tests/test_preview_transport.cpp:189 | tick pauses at the end and pins the time | 17 | 2 | none |
| C10 | 1174 | tests/test_preview_transport.cpp:191 | tick pauses at the end and pins the time | 17 | 2 | none |
| C10 | 1174 | tests/test_preview_transport.cpp:215 | a seek while playing brings the audio back after it ran out | 21 | 2 | none |
| C10 | 1199 | tests/test_preview_transport.cpp:217 | seek clamps to [0, length] | 13 | 1 | none |
| C10 | 1199 | tests/test_preview_transport.cpp:228 | seek clamps to [0, length] | 13 | 1 | none |
| C10 | 1219 | tests/test_preview_transport.cpp:230 | read_frames serves audio while playing and silence while paused | 11 | 0: not shown to fail by this run | none |
| C10 | 1219 | tests/test_preview_transport.cpp:251 | read_frames serves audio while playing and silence while paused | 11 | 0: not shown to fail by this run | none |
| C10 | 1241 | tests/test_preview_transport.cpp:253 | gain set before load applies to the next playhead | 12 | 1 | none |
| C10 | 1241 | tests/test_preview_transport.cpp:273 | gain set before load applies to the next playhead | 12 | 1 | none |
| C10 | 1262 | tests/test_preview_transport.cpp:275 | an audio offset seeks the playhead ahead of the clock | 9 | 2 | none |
| C10 | 1262 | tests/test_preview_transport.cpp:288 | an audio offset seeks the playhead ahead of the clock | 9 | 2 | none |
| C10 | 1287 | tests/test_preview_transport.cpp:290 | an audio offset seeks the playhead ahead of the clock | 9 | 2 | none |
| C10 | 1287 | tests/test_preview_transport.cpp:303 | scrubber: a song length past the audio ends playback there | 13 | 0: not shown to fail by this run | none |
| C10 | 1311 | tests/test_preview_transport.cpp:305 | scrubber: a song length past the audio ends playback there | 13 | 0: not shown to fail by this run | none |
| C10 | 1311 | tests/test_preview_transport.cpp:310 | scrubber: with no audio the end is still the song's length | 5 | 0: not shown to fail by this run | none |
| C10 | 1330 | tests/test_preview_transport.cpp:312 | load with no audio offset seeks to the asked position | 9 | 2 | none |
| C10 | 1330 | tests/test_preview_transport.cpp:318 | load with no audio offset seeks to the asked position | 9 | 2 | none |
| window-WN1 | 200 | tests/test_rules.cpp:318 | rules: max_tied_paths is one count per score, inside paths first | 619 | 78 | none |
| window-WN1 | 216 | tests/test_rules.cpp:349 | rules: a path over the Path limit stays kept when it ties the optimal score | 32 | 116 (78 through shared state) | none |
| window-WN1 | 232 | tests/test_rules.cpp:384 | rules: a path over the Path limit below the best score is dropped even as a tie | 9 | 108 (76 through shared state) | none |
| window-WN1 | 292 | tests/test_library_jobs.cpp:937 | jobs: the startup open reads saved paths with its own words, then the library | 911 | 112 (1 through shared state) | none |
| window-WN1 | 331 | tests/test_search_target.cpp:104 | search_target: a path missing a named activation is dropped, the rest kept | 512 | 99 | none |
| window-WN1 | 347 | tests/test_search_target.cpp:141 | search_target: a variant that took every named activation outlives its dropped root | 547 | 105 | none |
| window-WN1 | 363 | tests/test_search_target.cpp:198 | search_target: a kept root's tied variant that missed a named activation is dropped | 523 | 100 | none |
| window-WN1 | 379 | tests/test_search_target.cpp:299 | search_target: a full pin no path realizes names its window and why | 550 | 109 | none |
| window-WN1 | 395 | tests/test_search_target.cpp:345 | search_target: activation pins return the tick search's paths and its failing tick | 531 | 86 | none |
| window-WN1 | 411 | tests/test_search_target.cpp:370 | search_target: the binary search names the last or a middle window that breaks | 325 | 93 (19 through shared state) | none |
| window-WN1 | 427 | tests/test_search_target.cpp:420 | free after: fills past the last pin branch, fills before it follow the pins | 314 | 66 (42 through shared state) | none |
| window-WN2 | 260 | tests/test_best_finish.cpp:65 | best finish: with nothing pinned it is Hydra's best path on every corpus chart | 362 | 45 (3 through shared state) | none |
| window-WN3 | 326 | tests/test_path_costs.cpp:224 | search target: a built graph answers as the song does | 292 | 31 | none |
