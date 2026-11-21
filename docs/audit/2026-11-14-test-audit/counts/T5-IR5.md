a871878d1067aa4be

# T5-IR5 count: D9 Investigate records

Task T5-IR5. Session 63c70014-4328-49dc-9205-742235732752.

The brief said 89 records. D9.md holds 100 records whose last verdict is Investigate (89 start a line with "Verdict: Investigate", 11 more carry it mid-line). All 100 were read against the test code. Line numbers in "faulty check lines" are in the test file named in the "test" column; for a header or helper they are lines of the same file. "D9 line" is the line of the record's heading in D9.md.

The "record named it" column is a keyword check of the record text, not a reading of each word: it says the record mentions the fault in some form. Almost every record is "Investigate, claim not checked only", which treats the fakes as part of the setup and does not call them a fault.

| D9 line | test | faults | faulty check lines | reason | record named it |
|---|---|---|---|---|---|
| D9.md:24 | test_sp_tools.py:191 | X9 X10 X11 | 192,199,206-207 | scripted snapshots and fake game/opener/finder; typed reply text "watcher ended: stopped; rows written 3"; 2880 typed into a snapshot and required back in the CSV | X10 |
| D9.md:32 | test_sp_tools.py:210 | X9 X11 | 212-218 | fake game, fake process, fake opener; BACKUP_LINE is the fake's own return, required back | X9 |
| D9.md:40 | test_sp_tools.py:220 | X9 X10 | 223-233 | fake finder and game; typed refusal sentence | X9, X10 |
| D9.md:47 | test_sp_tools.py:237 | X9 X10 | 238-246 | fake game with hook list, scripted reader; typed "sp watch: watcher ended: stopped" | no |
| D9.md:54 | test_sp_tools.py:248 | X9 | 252 | fakes.searching_for_no_song replaces the engine search | X9 |
| D9.md:61 | test_sp_tools.py:257 | X9 X10 X11 | 258-268 | fake finder raises a typed NO_SONG text that is required back; typed status phrases | no |
| D9.md:68 | test_sp_tools.py:273 | X9 X10 X11 | 276,281,287-288 | fake dump of Hydra end (4000), fake cap_for_end returns 240.0 and the test requires it back; typed "run cap: 240 s" text | X9, X10 |
| D9.md:81 | test_sp_tools.py:302 | X10 | 303-304 | typed reply sentence | no |
| D9.md:85 | test_sp_tools.py:306 | X9 X10 | 309-320 | fake finder; typed reply text | no |
| D9.md:96 | test_sp_tools.py:347 | X9 X10 | 348-354 | hand-built snapshots; typed label text | X10 |
| D9.md:102 | test_sp_tools.py:365 | X9 X10 | 359-376 | scripted snapshots with typed end time; typed activation sentence | X10 |
| D9.md:107 | test_sp_tools.py:378 | X9 X10 | 359-386 | scripted snapshots; typed sentence | no |
| D9.md:113 | test_sp_tools.py:390 | X9 X11 | 393-402 | fake finders, fake game; ENGINE and 300.0 typed in and required back | no |
| D9.md:118 | test_sp_tools.py:405 | X9 X11 | 406-411 | fake finder raising typed NO_SONG, required back | no |
| D9.md:129 | test_sp_tools.py:429 | X9 X11 | 431-434 | fake opener raising OSError("no game"), message required back | no |
| D9.md:133 | test_sp_tools.py:436 | X9 X10 | 438-443 | fake game session_dir and finder; typed "watcher ended: failed:" | X10 |
| D9.md:144 | test_sp_tools.py:453 | X9 X10 X11 | 455-463 | scripted snapshots; typed sentences; 2880 typed in and required back | X10 |
| D9.md:153 | test_sp_tools.py:504 | X9 X10 | 505-508 | hand-typed (wall, clock) samples fed to a SpTools over a fake game; typed gap-line text and the typed song end | X10 |
| D9.md:160 | test_sp_tools.py:510 | X9 X10 | 511-513 | hand-typed (wall, clock) samples fed to a SpTools over a fake game; typed gap-line text and the typed song end | no |
| D9.md:164 | test_sp_tools.py:515 | X9 X10 | 516-519 | hand-typed (wall, clock) samples fed to a SpTools over a fake game; typed gap-line text and the typed song end | no |
| D9.md:168 | test_sp_tools.py:521 | X9 X10 | 522-524 | hand-typed (wall, clock) samples fed to a SpTools over a fake game; typed gap-line text and the typed song end | no |
| D9.md:179 | test_sp_tools.py:536 | X9 X10 | 539-547 | fake game with a made-up open run (SimpleNamespace) and/or fake hydra_replay dump, scripted snapshot, hand-typed gap rows; typed gap-line / "song length not known" text | X9 |
| D9.md:184 | test_sp_tools.py:549 | X9 X10 | 552-559 | fake game with a made-up open run (SimpleNamespace) and/or fake hydra_replay dump, scripted snapshot, hand-typed gap rows; typed gap-line / "song length not known" text | no |
| D9.md:188 | test_sp_tools.py:561 | X9 X10 | 562-570 | fake game with a made-up open run (SimpleNamespace) and/or fake hydra_replay dump, scripted snapshot, hand-typed gap rows; typed gap-line / "song length not known" text | no |
| D9.md:192 | test_sp_tools.py:572 | X9 X10 | 573-582 | fake game with a made-up open run (SimpleNamespace) and/or fake hydra_replay dump, scripted snapshot, hand-typed gap rows; typed gap-line / "song length not known" text | no |
| D9.md:196 | test_sp_tools.py:584 | X9 X10 | 588-597 | fake game with a made-up open run (SimpleNamespace) and/or fake hydra_replay dump, scripted snapshot, hand-typed gap rows; typed gap-line / "song length not known" text | no |
| D9.md:200 | test_sp_tools.py:599 | X9 X10 | 602-609 | fake game with a made-up open run (SimpleNamespace) and/or fake hydra_replay dump, scripted snapshot, hand-typed gap rows; typed gap-line / "song length not known" text | no |
| D9.md:204 | test_sp_tools.py:611 | X9 X10 | 613-620 | fake game with a made-up open run (SimpleNamespace) and/or fake hydra_replay dump, scripted snapshot, hand-typed gap rows; typed gap-line / "song length not known" text | no |
| D9.md:208 | test_sp_tools.py:622 | X9 X10 | 623-629 | fake game with a made-up open run (SimpleNamespace) and/or fake hydra_replay dump, scripted snapshot, hand-typed gap rows; typed gap-line / "song length not known" text | no |
| D9.md:218 | test_sp_tools.py:640 | X9 X10 | 641-644 | hand-typed (wall, clock) samples written straight into a watch over a fake game; typed gap/section label text | X10 |
| D9.md:222 | test_sp_tools.py:646 | X9 X10 | 647-649 | hand-typed (wall, clock) samples written straight into a watch over a fake game; typed gap/section label text | X10 |
| D9.md:226 | test_sp_tools.py:651 | X9 X10 | 652-653 | hand-typed (wall, clock) samples written straight into a watch over a fake game; typed gap/section label text | no |
| D9.md:235 | test_sp_tools.py:660 | X9 X10 | 661 | hand-typed (wall, clock) samples written straight into a watch over a fake game; typed gap/section label text | no |
| D9.md:239 | test_sp_tools.py:663 | X9 X10 | 664-666 | hand-typed (wall, clock) samples written straight into a watch over a fake game; typed gap/section label text | no |
| D9.md:243 | test_sp_tools.py:668 | X9 X10 | 669-672 | hand-typed (wall, clock) samples written straight into a watch over a fake game; typed gap/section label text | no |
| D9.md:251 | test_sp_tools.py:679 | X10 | 681-682 | typed refusal text | no |
| D9.md:255 | test_sp_tools.py:684 | X9 X10 | 685-689 | hand-built play-session dict; typed refusal sentence | no |
| D9.md:259 | test_sp_tools.py:691 | X9 X10 | 693-699 | hand-built watch rows (_mouths_files) and play dict; typed PASS/FAIL sentences | X10 |
| D9.md:266 | test_sp_tools.py:701 | X9 X10 X11 | 703-709 | hand-built rows and play dict; typed message "no song started within 120 s" required back | no |
| D9.md:287 | test_sp_tools.py:775 | X9 X10 snapshot | 763,778-779 | three hand-typed watch rows; expected "+1536 (0 -> 1536) at 118441.0 ms, full" pinned text/numbers | X10 |
| D9.md:293 | test_sp_tools.py:781 | X9 X10 X11 | 763,785-793 | hand-typed rows; the refusal sentence is typed into the file and required back inside a typed message | X9, X10 |
| D9.md:306 | test_trace_hits.py:64 | X9 | 65-67 | hand-built method table instead of an agent's real table | no |
| D9.md:313 | test_trace_hits.py:72 | X9 | 73 | hand-built method table | no |
| D9.md:317 | test_trace_hits.py:79 | X9 | 77,80-81 | hand-built calls and notes instead of a real trace | no |
| D9.md:323 | test_trace_hits.py:90 | X9 | 91 | hand-built call and notes | no |
| D9.md:327 | test_trace_hits.py:94 | X9 X10 | 96-99 | hand-built calls; the limit is read by name but the inputs 915/1085 and 85.0 are typed copies of it | no |
| D9.md:346 | test_trace_hits.py:124 | X9 X11 | 127-129 | fake hydra_replay dump; 193934.0 typed in and required back | X9 |
| D9.md:356 | test_trace_hits.py:141 | X9 X10 | 144-146 | fake dump that fails; typed "hydra_replay dump failed with exit code 1" | X9 |
| D9.md:368 | test_trace_hits.py:204 | X9 X10 X11 | 205-227 | fake frida session with hand-built call messages; typed report lines; args [7],[8] and rva 0x1234 typed in, printed/posted back | X10 |
| D9.md:374 | test_trace_hits.py:229 | X9 X10 X11 | 231-238 | fake session; typed "Miss: no such method" required back in typed text | no |
| D9.md:378 | test_trace_hits.py:240 | X9 X10 | 243-249 | fake session and hand-built calls; typed report line | X10 |
| D9.md:383 | test_trace_hits.py:257 | X9 X10 | 160,260-263 | fake session and fake dump in setUp; typed sentence | no |
| D9.md:391 | test_trace_hits.py:275 | X9 X10 | 276-294 | fake session and fake dump (62080); typed cap seconds 212.08/92.08 and typed report lines | X10 |
| D9.md:396 | test_trace_hits.py:296 | X9 X10 X11 | 297-303 | fake session whose typed error string is required back in a typed line | no |
| D9.md:400 | test_trace_hits.py:305 | X9 X10 | 306-309 | no-process fake frida; typed sentence | X9 |
| D9.md:417 | test_trace_hits.py:336 | none | 338 | source-text scan of trace.ts by a helper; no input made up and no typed expected value (rubric fault, outside the four) | no |
| D9.md:428 | test_trace_hits.py:351 | X9 X10 X11 | 352-354 | hand-built RunResult; "song cap" typed in and required back in typed line | X10 |
| D9.md:432 | test_trace_hits.py:356 | X9 | 358 | hand-built agent payload | no |
| D9.md:448 | test_trace_hits.py:394 | X9 X11 | 390-398 | hand-written score JSON; its times required back unchanged | no |
| D9.md:499 | test_trace_hits.py:556 | X9 X10 | 557-561 | fake frida with no process; typed "ended: not running" and sentence | no |
| D9.md:522 | test_cheap_checks.py:52 | X10 X12 | 54-56 | typed event label "loading" and title; comment cites a log file on the user's machine as the source | X12 |
| D9.md:528 | test_cheap_checks.py:58 | X10 X12 | 60-61 | typed label "countdown_start"; source cited as a log file | no |
| D9.md:532 | test_cheap_checks.py:63 | X10 X12 | 65-66 | typed label "countdown_end"; source cited as a log file | no |
| D9.md:536 | test_cheap_checks.py:68 | X10 X11 X12 | 70-71 | typed label "seek"; 121.880544 typed in and required back; source cited as a log file | no |
| D9.md:540 | test_cheap_checks.py:73 | none | 74-75 | a copied real log line must give None | no |
| D9.md:544 | test_cheap_checks.py:79 | none | 92 | files made by the test with set mtimes; the rule is checked | no |
| D9.md:581 | test_cheap_checks.py:194 | X9 X10 | 160-205 | FakeClock, fake find and reader callables with typed step lengths; typed report lines | X10 |
| D9.md:587 | test_cheap_checks.py:208 | X9 X10 | 209-215 | fake clock and fake finds; typed DISAGREE sentence | no |
| D9.md:591 | test_cheap_checks.py:239 | X9 X10 X11 | 225-253 | fake process over fake memory, fake scan, fake clock; OBJ typed in and required back in typed text | X9 |
| D9.md:610 | test_debugger.py:51 | X9 X11 | 52-59 | FakeMemory stands in for game memory; 0x55 typed in and required back | no |
| D9.md:614 | test_debugger.py:61 | X9 X11 | 62-69 | FakeMemory; 0x90 typed in and required back | no |
| D9.md:617 | test_debugger.py:71 | X9 X11 | 74-82 | FakeMemory; 0xAB typed in and required back | no |
| D9.md:621 | test_debugger.py:84 | X9 X11 | 85-92 | FakeMemory stands in for game memory; 0x48 typed in and required back | no |
| D9.md:625 | test_debugger.py:94 | X9 X11 | 95-102 | FakeMemory; 0x33 typed in and required back | no |
| D9.md:629 | test_debugger.py:104 | X9 X11 | 105-111 | FakeMemory; 0x11 typed in and required back | no |
| D9.md:633 | test_debugger.py:113 | X9 X11 | 114-121 | FakeMemory; 0x77 typed in and required back | no |
| D9.md:637 | test_debugger.py:125 | none | 127-131 | pure byte function; the masking is the behaviour | no |
| D9.md:641 | test_debugger.py:133 | none | 134-138 | pure byte function; input unchanged is a real behaviour | no |
| D9.md:644 | test_debugger.py:140 | none | 141-142 | pure byte function; input unchanged is a real behaviour | no |
| D9.md:647 | test_debugger.py:146 | none | 149-151 | packs a double and decodes it; the codec is the behaviour | no |
| D9.md:655 | test_debugger.py:157 | none | 158-159 | error type on a short buffer | no |
| D9.md:658 | test_debugger.py:163 | none | 165 | one-subtract rule on typed address | no |
| D9.md:663 | test_debugger.py:169 | X11 | 170-173 | registers typed into a dict and required back as attributes | no |
| D9.md:666 | test_debugger.py:175 | none | 176-178 | pack then decode; codec behaviour | no |
| D9.md:678 | test_debugger.py:253 | X9 | 216-258 | FakeK32/FakeWin32 replace the Windows debug API | X9 |
| D9.md:683 | test_debugger.py:260 | X9 | 258-264 | FakeK32 fails on purpose | no |
| D9.md:687 | test_debugger.py:267 | X9 | 270-298 | mocked reader/writer and fake Win32 | X9 |
| D9.md:692 | test_debugger.py:306 | X9 | 200-212,308 | hand-built DEBUG_EVENT | X9 |
| D9.md:697 | test_debugger.py:314 | X9 | 200-212,319-322 | hand-built DEBUG_EVENT and a replaced _rewind_rip | no |
| D9.md:701 | test_debugger.py:324 | X9 | 325-330 | FakeMemory stands in for game memory | no |
| D9.md:719 | test_early_fill_walk.py:65 | none | 66-70 | reads the generated chart text and checks the generator's own notes | no |
| D9.md:734 | test_early_fill_walk.py:89 | X10 X12 | 27,94 | typed 1920 deadline, its source cited as docs/clone-hero/04-fills.md FL-19 | X10, X12 |
| D9.md:751 | test_early_fill_walk.py:113 | X10 X12 | 114,119 | typed 20000 and 12000; file header says the numbers are the plan's | no |
| D9.md:756 | test_early_fill_walk.py:121 | X10 X12 | 125 | typed 6000; file header says the numbers are the plan's | X12 |
| D9.md:792 | test_edge_probe.py:104 | X9 | 5-6,32,105-112 | made-up kicks in a hand-built schedule | X9 |
| D9.md:797 | test_edge_probe.py:114 | X9 X10 | 116 | hand-built schedule; typed refusal text | X9 |
| D9.md:801 | test_edge_probe.py:118 | X10 | 123 | typed refusal text (input is a real saved schedule) | no |
| D9.md:812 | test_edge_probe.py:138 | X9 X10 | 142-163 | hand-built schedule, fake wall/clock/game; typed "done" | X9 |
| D9.md:818 | test_edge_probe.py:165 | X9 X10 | 166-174 | hand-typed rows; typed line text | X10 |
| D9.md:823 | test_edge_probe.py:176 | X10 | 177-179 | typed line text | X10 |

## Counts

- Investigate records read: 100
- Records with at least one fault: 89
- Records with no fault: 11
- Records with X9: 77
- Records with X10: 62
- Records with X11: 26
- Records with X12: 7
- Records with snapshot: 1
- Records where the record text already names at least one of its faults (keyword check): 36
