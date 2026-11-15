agent a861e7341d89d7c3d (T5-IR9)

# T5-IR9 count: Investigate records of D13, A19, A21, re-read by the user's fake/constant/echo/non-code rulings

Method: for each Investigate record I opened the named test and read every check, and read the code each check feeds where it was in the same file. `named` column: `y` = the record itself named this fault in its text; `p` = the record only called the test 'claim not checked' / needs a real run (hand-built fixture as the reason for Investigate, not as a fault); `n` = not named (A19 and A21 records use 'stand-in' for a different thing, a weak proxy). The y/p/n column is from reading the records plus a keyword scan, so treat y/p as approximate. Test line numbers are the test's own TEST_CASE/def line. Fault lines are the checks (or the data they use) that carry the fault.

| record | # | test | faults | faulty lines | reason | record named it |
|---|---|---|---|---|---|---|
| D13 | 1 | tools/ch_probe/tests/test_sp_compare.py:150 | X9 X10 | 151, 153 | hand-built rows; whole 12-line report typed as MATCHING_REPORT | p |
| D13 | 2 | tools/ch_probe/tests/test_sp_compare.py:155 | X9 X10 | 156, 158 | hand-built rows; summary text typed | p |
| D13 | 3 | tools/ch_probe/tests/test_sp_compare.py:163 | X9 X10 | 166-167 | hand-built rows; stop message typed | p |
| D13 | 4 | tools/ch_probe/tests/test_sp_compare.py:170 | X9 X10 | 172 | hand-built rows; message typed | p |
| D13 | 5 | tools/ch_probe/tests/test_sp_compare.py:177 | X9 X10 X12 | 179-180 | hand-built rows; window numbers 1998.0/2100.0 typed; window comes from the plan's Task S | p |
| D13 | 6 | tools/ch_probe/tests/test_sp_compare.py:183 | X9 X10 | 185-188 | hand-built rows; report lines typed | p |
| D13 | 7 | tools/ch_probe/tests/test_sp_compare.py:191 | X9 X10 | 193-196 | hand-built rows; report lines typed | p |
| D13 | 8 | tools/ch_probe/tests/test_sp_compare.py:199 | X9 X10 | 201-207 | hand-built rows; report lines typed | p |
| D13 | 9 | tools/ch_probe/tests/test_sp_compare.py:210 | X9 X10 | 214-223 | hand-built rows; report lines typed | p |
| D13 | 10 | tools/ch_probe/tests/test_sp_compare.py:226 | X9 X10 | 229-232 | one hand-written row; message typed | y |
| D13 | 11 | tools/ch_probe/tests/test_sp_compare.py:235 | X9 X10 | 237, 242 | hand-built rows; message typed | p |
| D13 | 12 | tools/ch_probe/tests/test_sp_compare.py:247 | X9 X10 | 248-262 | hand-built rows; report lines typed | p |
| D13 | 13 | tools/ch_probe/tests/test_sp_compare.py:265 | X9 X10 | 269-271 | hand-built rows; line typed | p |
| D13 | 14 | tools/ch_probe/tests/test_sp_compare.py:304 | X9 X10 X12 snapshot | 279-331 | CSV rows 'copied from that run', hand-picked 2 ms reach, expected lines and -3550.2 pinned from the run's figure | y |
| D13 | 15 | tools/ch_probe/tests/test_sp_compare.py:334 | X9 | 337-347 | hand-built rows, chords and reach | p |
| D13 | 16 | tools/ch_probe/tests/test_sp_compare.py:352 | X9 | 355-366 | hand-built rows, chords and reach | p |
| D13 | 17 | tools/ch_probe/tests/test_sp_compare.py:388 | X9 X10 X12 | 371-395 | hand-built row; 14400 and 15360 typed (not read from the code); sourced to 'the plan's F5 acceptance' | p |
| D13 | 18 | tools/ch_probe/tests/test_sp_compare.py:397 | X9 X10 | 400-403 | hand-built rows; line typed | p |
| D13 | 23 | tools/ch_probe/tests/test_sp_compare.py:437 | X9 X10 | 438-441 | hand-built rows; report line typed | p |
| D13 | 24 | tools/ch_probe/tests/test_sp_compare.py:444 | X9 X10 | 445-447 | hand-built rows; report line typed | p |
| D13 | 25 | tools/ch_probe/tests/test_sp_compare.py:477 | X9 X10 | 481-485 | hand-written watcher rows and chords; report line typed | p |
| D13 | 26 | tools/ch_probe/tests/test_sp_compare.py:487 | X9 X10 | 490-494 | hand-written rows; line typed | p |
| D13 | 27 | tools/ch_probe/tests/test_sp_compare.py:496 | X9 X10 | 499-504 | hand-written rows; line typed | p |
| D13 | 28 | tools/ch_probe/tests/test_sp_compare.py:508 | X9 X11 | 512-518 | hand-written CSV of the value 7 in every column; 7 typed in and required back | y |
| D13 | 29 | tools/ch_probe/tests/test_sp_compare.py:522 | X9 | 526-533 | hand-written CSV of 0..n; the index is read back | p |
| D13 | 30 | tools/ch_probe/tests/test_sp_compare.py:535 | X9 | 537-541 | hand-written header-only file | p |
| D13 | 33 | tools/ch_probe/tests/test_sp_compare.py:578 | X9 X10 | 580-584 | hand-built log written to a file; whole MATCHING_REPORT typed | y |
| D13 | 34 | tools/ch_probe/tests/test_sp_compare.py:586 | X9 | 588-591 | hand-built log file | p |
| D13 | 35 | tools/ch_probe/tests/test_sp_compare.py:593 | X9 X10 | 595-598 | hand-built log file; message typed | p |
| D13 | 36 | tools/ch_probe/tests/test_sp_compare.py:600 | X9 X10 | 602-610 | hand-built log file; 11520 and message typed | p |
| D13 | 40 | tools/ch_probe/tests/test_sp_compare.py:647 | X9 | 648-652 | hand-written row | p |
| D13 | 41 | tools/ch_probe/tests/test_sp_compare.py:654 | X9 | 658-667 | hand-typed chords and press times (998, 1002.2) | p |
| D13 | 42 | tools/ch_probe/tests/test_sp_compare.py:669 | X9 | 670-675 | hand-written rows | p |
| D13 | 43 | tools/ch_probe/tests/test_sp_compare.py:677 | X9 | 678-680 | hand-written rows | p |
| D13 | 44 | tools/ch_probe/tests/test_sp_compare.py:682 | X9 | 683-688 | hand-written rows | p |
| D13 | 45 | tools/ch_probe/tests/test_sp_compare.py:690 | X9 | 691-692 | hand-written rows | p |
| D13 | 47 | tools/ch_probe/tests/test_sp_compare.py:736 | X9 X10 | 738-740 | hand-written rows; credited line typed (schedule file is saved tool output, rows are not) | y |
| D13 | 51 | tools/ch_probe/tests/test_sp_compare.py:775 | X9 | 776-783 | hand-built schedule plus a made-up refusal sentence | y |
| D13 | 52 | tools/ch_probe/tests/test_sp_compare.py:798 | X9 | 799-805 | hand-written (wall, clock) samples | p |
| D13 | 61 | tools/ch_probe/tests/test_sp_compare.py:853 | X9 | 856-860 | hand-written samples | p |
| D13 | 68 | tools/ch_probe/tests/test_sp_watch.py:97 | X9 X10 | 100-106 | hand-packed engine block; 7200 and 8589940736 typed; comment says it 'pins constants.OFF_SP_HALF' | p |
| D13 | 69 | tools/ch_probe/tests/test_sp_watch.py:142 | X9 X10 | 143-145 | fake game memory; 12.75, 12.5, 0.25 typed; offsets written as literals to pin constants.py | p |
| D13 | 71 | tools/ch_probe/tests/test_sp_watch.py:153 | X9 | 154-156 | fake memory | y |
| D13 | 72 | tools/ch_probe/tests/test_sp_watch.py:163 | X9 | 164-167 | fake memory | p |
| D13 | 73 | tools/ch_probe/tests/test_sp_watch.py:169 | X9 | 170-174 | fake memory | p |
| D13 | 74 | tools/ch_probe/tests/test_sp_watch.py:176 | X9 | 177-180 | fake memory | p |
| D13 | 75 | tools/ch_probe/tests/test_sp_watch.py:182 | X9 | 183-184 | hand-packed engine block | p |
| D13 | 76 | tools/ch_probe/tests/test_sp_watch.py:186 | X9 X10 | 187-197 | fake memory and fake clock; stop message typed | p |
| D13 | 77 | tools/ch_probe/tests/test_sp_watch.py:199 | X9 X12 X10 | 200-212 | fake memory; bound 0.10 'the plan's T4' | y |
| D13 | 79 | tools/ch_probe/tests/test_sp_watch.py:249 | X9 X10 | 252-260 | fake clock, hand-made snapshot; message typed | p |
| D13 | 80 | tools/ch_probe/tests/test_sp_watch.py:262 | X9 X10 | 265-271 | fake clock, hand-made snapshots; message typed | p |
| D13 | 84 | tools/ch_probe/tests/test_sp_watch.py:322 | X9 X10 | 323-340 | fake process, fake scan, fake clock; message typed | p |
| D13 | 89 | tools/ch_probe/tests/test_sp_watch.py:399 | X9 X10 | 400-413 | fake process memory; 12.75/12.5/0.25 typed | p |
| D13 | 97 | tools/ch_probe/tests/test_watch_checks.py:91 | X9 X10 | 92-98 | hand-written rows; report text typed | p |
| D13 | 98 | tools/ch_probe/tests/test_watch_checks.py:100 | X9 X10 | 101-106 | hand-written rows; report text typed | p |
| D13 | 99 | tools/ch_probe/tests/test_watch_checks.py:108 | X9 X10 | 109-113 | hand-written rows; report text typed | p |
| D13 | 100 | tools/ch_probe/tests/test_watch_checks.py:115 | X9 X10 | 116-119 | hand-written rows; text typed | p |
| D13 | 101 | tools/ch_probe/tests/test_watch_checks.py:121 | X9 X10 | 122-123 | hand-written rows; text typed | p |
| D13 | 102 | tools/ch_probe/tests/test_watch_checks.py:141 | X9 X11 | 142-149 | hand-typed values written to a file and required back | y |
| D13 | 103 | tools/ch_probe/tests/test_watch_checks.py:151 | X9 | 154-160 | hand-written log of 0..n | p |
| D13 | 104 | tools/ch_probe/tests/test_watch_checks.py:162 | X9 | 163-167 | hand-written log of sevens | p |
| D13 | 105 | tools/ch_probe/tests/test_watch_checks.py:169 | X9 | 170-173 | hand-written header | p |
| D13 | 106 | tools/ch_probe/tests/test_watch_checks.py:175 | X9 | 176-182 | hand-written log of sevens | p |
| D13 | 107 | tools/ch_probe/tests/test_watch_checks.py:184 | X9 | 185-191 | hand-written log of 0..n | p |
| D13 | 119 | tools/ch_probe/tests/test_stall_harness.py:181 | X9 | 184-212 | a fake open() that blocks, fake game; comment cites PS5 plan | y |
| D13 | 122 | tools/ch_probe/tests/test_stall_harness.py:330 | X9 X12 | 256-268, 330-344 | fake game (stall_game.mjs); schedule from a saved LS-P3 run outside the repo; sizes from IG0/PS1 of a plan | y |
| D13 | 123 | tools/ch_probe/tests/test_stall_harness.py:346 | X9 X12 | 346-349 | same fake game and saved-run schedule | p |
| D13 | 124 | tools/ch_probe/tests/test_stall_harness.py:351 | X9 X10 X12 | 351-368 | fake game; message note typed; construction from a plan and run | y |
| D13 | 125 | tools/ch_probe/tests/test_tick_rate_vs_hydra.py:46 | none |  | real hydra_replay on a chart the test builds; compared with the probe's own function (not typed) | n |
| D13 | 129 | tools/test_agent_cost.py:107 | X9 X10 X12 | 108 | hand-written transcript; dollar figure typed, 'worked out by hand' (file header) | p |
| D13 | 135 | tools/test_agent_cost.py:125 | X9 X10 | 126-132 | hand-written transcript; 0.0050 and message typed | p |
| A19 | PL3 | tests/test_path_list.cpp:80 | X9 X10 | 83, 86 | hand-built record with an empty Path; sentence typed | n |
| A19 | PL5 | tests/test_path_list.cpp:143 | X9 | 152-161 | record edited by hand: a variant made by end_tick += 1 | n |
| A19 | PL12 | tests/test_path_list.cpp:339 | X10 | 352-358 | display labels typed ('+15 pts', '[Red - Yellow cymbal]'); the chart is real | n |
| A19 | PL13 | tests/test_path_list.cpp:362 | none |  | checks fold state transitions; no engine value or typed product text | n |
| A19 | PC2 | tests/test_precheck.cpp:456 | X12 X10 | 463-472 | expected findings are the reviewers' hand-written list; typed file/regex rows | n |
| A19 | PC3 | tests/test_precheck.cpp:476 | X12 X10 | 483-491 | same: reviewers' findings typed | n |
| A19 | PC5 | tests/test_precheck.cpp:505 | none |  | expected line comes from hydra::unknown_option_line; the option is a deliberate bad input | n |
| A21 | 5 | tests/test_preview_controller.cpp:338 | X11 | 340-343 | compares a function with itself and a flag with the argument typed in | y |
| A21 | 8 | tests/test_preview_controller.cpp:379 | X9 X11 | 382-391 | fake audio-device factory throws a made-up string, required back as the warning | n |
| A21 | 9 | tests/test_preview_controller.cpp:403 | X9 X10 | 410-422 | headless/forced-fail audio switches; message typed | n |
| A21 | 11 | tests/test_preview_controller.cpp:441 | X10 | 447-449 | the sentence typed | n |
| A21 | 23 | tests/test_preview_controller.cpp:869 | X9 | 880 | a made-up md5 string stands in for a changed file | n |
| A21 | 29 | tests/test_preview_golden.cpp:258 | X9 | 265-300 | delta map the test's own comment calls made-up | y |
| A21 | 30 | tests/test_preview_golden.cpp:304 | X12 | 312-353 | golden is a screenshot from Onyx, another program; limits typed in golden.json | n |

## Counts

- Investigate records read: 85 (D13 71, A19 7, A21 7; my read of all of them).
- Records with at least one fault: 82.
- Records per fault: X10 47, X11 4, X12 11, X9 76, snapshot 1.
- Records whose fault the record itself named (y): 14. Named only as 'claim not checked' (p): 58. The rest were not named.

No record with no fault is hidden: the three none rows are D13 125, A19 PL13, A19 PC5.
Read but not recounted: D13 records of other verdicts. Not reached: none.
