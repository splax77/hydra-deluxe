Agent id: aae4654ffd03f1a00 (task T5-IR6, verdict Investigate, record files D7, A9, A12, C4, A16, D16)

# T5-IR6: Investigate records of D7, A9, A12, C4, A16 and D16 read by the three test rulings

I read every Investigate record in my six files and every check of the test case it names. D7 has 108 Investigate records, not 79: the Python files hold 27 (recorder), 57 (report) and 24 (menu). The other five files hold one each, except D16 with two: 114 records in all. In D7 I read the three test files whole. In the others I read the named test case.

Short names. RC is `tools/ch_mcp/tests/test_machine_recorder.py`, RP is `test_machine_report.py`, MT is `test_menu_tools.py`.

How I called the faults.
- X9: the test feeds a hand-built table, recording, fake clock, fake reader, fake game, fake kit, fake file or hand-written settings.ini to the code. In RC every test below the CapturedTable class goes through `fake_table` or `FakeLoop`. In RP every test goes through `Fixture`, rows the test types in. In MT every walk goes through FakeGame, FakeKit, FakeFrida and FakeClock, and the expect-word tests use the hand-built `state()` dict.
- X10: the expected value is a typed table or the code's own message or label text. In the display tests (RP, MT) I marked X10 only where the check is a notice or refusal sentence or a typed word list, not where the line carries a computed number. A reader may want to widen or narrow that.
- X11: a value typed in and required back (a fake's own reply words, a settings value, a stop reason and process time).
- X12: the expected value or the test's title names a doc, plan, ruling or citation as its source.
- Snapshot: folded into X10 here (typed number tables). None found separately.

"Named" says whether the record already said it in its own words. For X9 I count a record as naming it when it says the input is hand-built, fake or "not real input", which nearly all of them do under "claim not checked". No record called any of this a fault under the rulings.

## Rerun under the user's later ruling on hand-built input

The main session told me: input a test builds and feeds to the real code (a hand-built chart, a hand-written settings.ini or CMake file, a throwaway repo, fuzz bytes) is not fake. Only a stand-in for the code's own output, a stand-in for outside data, or fake code is X9. I went through every X9 mark again.
- Cleared: C4 record 1 (hand-written CMake text read by the real reader). It is now none.
- Kept, as fake code: every FakeLoop, fake clock, fake reader, FakeFile, FakeGame, FakeKit, FakeFrida and patched ctypes (RC :506 to :791 and :938, MT).
- Kept, as stand-in for outside data: `fake_table` (a hand-built Windows process table, RC :323 to :401) and the hand-built `state()` game reads (MT).
- Kept, as stand-in for the code's own output: the hand-built SampleRow and gains (RC :367, :384, :401) and the report tests' `Fixture` (RP). A `Fixture` is a recording written by the test in the shape the recorder writes, so I read it as a stand-in for the recorder's output. This is the one call that could go the other way: if the user counts a recording file the real report reads as input a test builds, the 56 RP X9 marks clear. The hand-written settings.ini in RP :527 and :532 is no longer a reason on its own; those two keep X9 only through their `Fixture`.
- X10, X11 and X12 marks are not touched by this ruling.

## Table

| Rec file | No | Test | Faults | Faulty checks | Reason | Named |
|---|---|---|---|---|---|---|
| D7 RC | :208 layout is Windows layout | RC:208 | X10, X12 | RC:209-212, 35-50 | typed offsets and sizes copied from a cited web page; expected side built by the owner's `_layout` | yes |
| D7 RC | :258 each ready state | RC:258 | X10 | RC:259 | typed ready states 1, 3, 7 | yes (no source) |
| D7 RC | :304 each state number | RC:304 | X10 | RC:306-307 | typed map equal to the production map | yes |
| D7 RC | :323 first seen | RC:323 | X9 | RC:324-327 | hand-built tables | yes |
| D7 RC | :329 thread first seen | RC:329 | X9 | RC:330-332 | hand-built tables | yes |
| D7 RC | :334 reused pid | RC:334 | X9 | RC:335-338 | hand-built tables | yes |
| D7 RC | :367 columns pinned | RC:367 | X9, X10, X11 | RC:368-369, 370-379, 381-382 | hand-built sample row and tables; headers typed again; row echoes its typed inputs | yes |
| D7 RC | :384 process row needs gain | RC:384 | X9 | RC:386-399 | hand-built gains and table | yes |
| D7 RC | :401 thread row rule | RC:401 | X9 | RC:402-415 | hand-built threads and gains | yes |
| D7 RC | :506 stdin lines switch | RC:506 | X9 | RC:509-529 | fake reader, fake table, fake clock | yes |
| D7 RC | :531 game, worker, recorder rows | RC:531 | X9 | RC:532-539 | fake table and clock | yes |
| D7 RC | :552 first sample lists all | RC:552 | X9 | RC:553-558 | fake table and clock | yes |
| D7 RC | :560 before stamp | RC:560 | X9 | RC:562-581 | fake wall counter, fake reader | yes |
| D7 RC | :590 closed stdin | RC:590 | X9 | RC:592-605 | fake clock; latency on a fake | yes |
| D7 RC | :607 signalled parent | RC:607 | X9 | RC:610-626 | fake clock | yes |
| D7 RC | :628 cap | RC:628 | X9 | RC:629-632 | fake loop | yes |
| D7 RC | :634 seconds | RC:634 | X9 | RC:635-636 | fake loop | yes |
| D7 RC | :653 clock read raises | RC:653 | X9 | RC:654-669 | fake reader that raises | yes |
| D7 RC | :671 game cannot open | RC:671 | X9 | RC:672-680 | fake opener | yes |
| D7 RC | :682 short read (the one that runs) | RC:682 | X9 | RC:683-688 | fake reader | yes |
| D7 RC | :690 late sample | RC:690 | X9 | RC:691-706 | fake clock | yes |
| D7 RC | :743 rows on disk | RC:743 | X9 | RC:721-757 | FakeFile stands for the disk | yes |
| D7 RC | :787 another failure loud | RC:787 | X9, X11 | RC:788-789 | fake query returns a typed status; the message must hold it | yes (X9) |
| D7 RC | :791 past the ulong limit | RC:791 | X9 | RC:792-802 | patches ctypes with fakes | yes |
| D7 RC | :816 numbers the plan fixes | RC:816 | X10, X12 | RC:817 | typed 50 and 60; title cites the plan | yes |
| D7 RC | :927 parent cannot be opened | RC:927 | X10 | RC:934 | the error sentence typed | no |
| D7 RC | :938 nonfinite clock | RC:938 | X9, X12 | RC:941-956, 939-940 | hand-built reader values; the comment cites TM-11 and D325 | X9 yes, X12 no |
| D7 RP | :120 files and words | RP:120 | X10 | RP:125 | "ready" and "running" typed; reads real constants, no fake | no |
| D7 RP | :144 whole recording | RP:144 | X9 | RP:145-149 | Fixture rows typed in | yes |
| D7 RP | :151 window changes shares | RP:151 | X9 | RP:152-157 | Fixture | yes |
| D7 RP | :159 window edge | RP:159 | X9 | RP:160-162 | Fixture | yes |
| D7 RP | :164 first sample ends no slice | RP:164 | X9 | RP:165-167 | Fixture | yes |
| D7 RP | :169 always listed names | RP:169 | X9, X10 | RP:170-173 | Fixture; four names typed (a copy of the owner's list) | yes |
| D7 RP | :175 listed with no cpu | RP:175 | X9 | RP:176-180 | Fixture | yes |
| D7 RP | :182 gained nothing stays out | RP:182 | X9 | RP:183-185 | Fixture | yes |
| D7 RP | :187 window with no slice | RP:187 | X9, X10 | RP:188-191 | Fixture; refusal sentence typed | X9 yes |
| D7 RP | :210 three ready samples | RP:210 | X9 | RP:211-214 | Fixture | yes |
| D7 RP | :216 one ready sample | RP:216 | X9 | RP:217-218 | Fixture | yes |
| D7 RP | :220 boundary of two | RP:220 | X9 | RP:221-228 | Fixture | yes |
| D7 RP | :233 window clips | RP:233 | X9 | RP:234-236 | Fixture | yes |
| D7 RP | :238 read by word | RP:238 | X9 | RP:239-243 | Fixture | yes |
| D7 RP | :245 worker and recorder threads | RP:245 | X9 | RP:246-253 | Fixture | yes |
| D7 RP | :255 pid with no rows | RP:255 | X9, X10 | RP:256 | Fixture; "no thread rows in the window" typed | X9 yes |
| D7 RP | :258 one pid two roles | RP:258 | X9 | RP:259-264 | Fixture | yes |
| D7 RP | :284 four late samples | RP:284 | X9, X10 | RP:268-289 | Fixture; the late-mark sentence typed as a copy | X9 yes |
| D7 RP | :291 window counts switches | RP:291 | X9 | RP:292-294 | Fixture | yes |
| D7 RP | :296 exactly twice | RP:296 | X9, X10 | RP:297-299 | Fixture; typed mark text | X9 yes |
| D7 RP | :301 just over twice | RP:301 | X9, X10 | RP:302-303 | Fixture; typed mark text | X9 yes |
| D7 RP | :309 one late gap | RP:309 | X9, X10 | RP:310 | Fixture; typed mark text | X9 yes |
| D7 RP | :323 six pairs | RP:323 | X9 | RP:324-333 | Fixture | yes |
| D7 RP | :335 calm run | RP:335 | X9, X10 | RP:336-339 | Fixture; "none is listed" sentence typed | X9 yes |
| D7 RP | :345 six pairs lead | RP:345 | X9 | RP:346-348 | Fixture | yes |
| D7 RP | :350 excess exactly the limit | RP:350 | X9 | RP:351-352 | Fixture | yes |
| D7 RP | :354 excess just over | RP:354 | X9 | RP:355-356 | Fixture | yes |
| D7 RP | :358 list stops at ten | RP:358 | X9 | RP:359-366 | Fixture | yes |
| D7 RP | :367 steps in a window | RP:367 | X9 | RP:368-370 | Fixture | yes |
| D7 RP | :372 no clock ends the run | RP:372 | X9 | RP:373-380 | Fixture | yes |
| D7 RP | :382 no clock | RP:382 | X9, X10 | RP:383-386 | Fixture; sentence typed | X9 yes |
| D7 RP | :388 clock with no stamps | RP:388 | X9, X10 | RP:389-393 | Fixture with a cell blanked; sentence typed | X9 yes |
| D7 RP | :409 dropped, not listed | RP:409 | X9 | RP:410-416 | Fixture; stall data made up "as in LS-O2" | yes |
| D7 RP | :418 neighbours pair up | RP:418 | X9 | RP:419-420 | Fixture | yes |
| D7 RP | :422 tight stamps kept | RP:422 | X9 | RP:423-426 | Fixture | yes |
| D7 RP | :428 exactly one period | RP:428 | X9 | RP:429-430 | Fixture | yes |
| D7 RP | :432 just over one period | RP:432 | X9 | RP:433-434 | Fixture | yes |
| D7 RP | :436 drops in the window | RP:436 | X9 | RP:437-438 | Fixture | yes |
| D7 RP | :442 120 ms gap | RP:442 | X9 | RP:443-448 | Fixture | yes |
| D7 RP | :450 gap of exactly GAP_MS | RP:450 | X9 | RP:451-455 | Fixture | yes |
| D7 RP | :459 share and bytes | RP:459 | X9 | RP:460-470 | Fixture | yes |
| D7 RP | :474 counts thread samples | RP:474 | X9 | RP:475-481 | Fixture | yes |
| D7 RP | :483 none refused | RP:483 | X9, X10 | RP:484-491 | Fixture; two refusal lines typed | X9 yes |
| D7 RP | :504 bytes in window | RP:504 | X9 | RP:505-508 | Fixture | yes |
| D7 RP | :510 hard faults mark | RP:510 | X9, X10 | RP:511-513 | Fixture; heading "(meaning unchecked)" typed | X9 yes |
| D7 RP | :527 prints the two lines | RP:527 | X9, X11 | RP:528-530 | hand-written settings.ini; 330 and 0 typed in, required back | X9 yes |
| D7 RP | :532 line not there | RP:532 | X9, X10 | RP:533-535 | hand-written settings.ini; "no framerate line" typed | X9 yes |
| D7 RP | :553 grandparent is the front | RP:553 | X9 | RP:544-558 | Fixture | yes |
| D7 RP | :560 front with no rows | RP:560 | X9, X10 | RP:561-565 | Fixture; "(no rows)" typed | X9 yes |
| D7 RP | :567 launcher with no row | RP:567 | X9, X10 | RP:568-571 | Fixture; sentence typed | X9 yes |
| D7 RP | :573 worker with no row | RP:573 | X9, X10 | RP:574-575 | Fixture; sentence typed | X9 yes |
| D7 RP | :577 no worker no front | RP:577 | X9 | RP:578 | Fixture | yes |
| D7 RP | :599 nameless process | RP:599 | X9 | RP:585-602 | Fixture "as the recorder writes it", still typed | yes |
| D7 RP | :604 parent only in sample one | RP:604 | X9 | RP:605-608 | Fixture | yes |
| D7 RP | :618 last row says why | RP:618 | X9, X11 | RP:619-621 | Fixture; "seconds" and "9.5" typed in, required back | X9 yes |
| D7 RP | :623 missing last row | RP:623 | X9, X10 | RP:624-626 | Fixture; sentence typed | X9 yes |
| D7 RP | :628 half-written row | RP:628 | X9, X10 | RP:629-637 | hand-written cut row; "left out 1 half-written rows" typed | X9 yes |
| D7 MT | :196 full run prints a line per step | MT:196 | X9, X11 | MT:197-208 | fake game, kit, Frida, clock; "pressed kit:start for 2 frames" is the fake's own words | yes (X9) |
| D7 MT | :210 checked-in walk | MT:210 | X9 | MT:211-224 | fake game script | yes |
| D7 MT | :233 wrong folder | MT:233 | X9, X10 | MT:236-242 | fakes; refusal sentence typed | X9 yes |
| D7 MT | :255 instrument and difficulty | MT:255 | X9, X10 | MT:259-271 | fakes; refusal words typed | X9 yes |
| D7 MT | :273 plays asked player | MT:273 | X9 | MT:274-277 | fakes | yes |
| D7 MT | :288 never checked player screen | MT:288 | X9, X10 | MT:291-297 | fakes; refusal sentence typed | X9 yes |
| D7 MT | :299 read never changes | MT:299 | X9, X10 | MT:300-308 | fake clock and read; stop sentence and read line typed | X9 yes |
| D7 MT | :310 online open first read | MT:310 | X9, X10 | MT:311-316 | fakes; sentence typed | X9 yes |
| D7 MT | :318 online before step 2 | MT:318 | X9, X10 | MT:319-336 | fakes; sentence typed | X9 yes |
| D7 MT | :338 error entry | MT:338 | X9, X10 | MT:339-343 | fakes; sentence typed | X9 yes |
| D7 MT | :345 null entry | MT:345 | X9 | MT:346 | hand-built state | yes |
| D7 MT | :348 failed menu block | MT:348 | X9, X10 | MT:349-355 | hand-built dicts; four sentences typed | X9 yes |
| D7 MT | :357 two scrolls | MT:357 | X9, X10 | MT:358-364 | fakes; scroll sentence typed; scroll key a guess | X9 yes |
| D7 MT | :366 scroll cap | MT:366 | X9, X10 | MT:367-372 | fakes; cap sentence typed | X9 yes |
| D7 MT | :374 unattached | MT:374 | X9, X10 | MT:375-380 | fake raising a typed text; sentence typed | X9 yes |
| D7 MT | :388 refused kit press | MT:388 | X9, X11 | MT:389-397, 95 | FakeKit's typed refusal text required back | X9 yes |
| D7 MT | :399 refused scroll | MT:399 | X9 | MT:400-405 | fakes | yes |
| D7 MT | :407 key gap | MT:407 | X9 | MT:408-412 | fake clock | yes |
| D7 MT | :435 each active word | MT:435 | X9, X10 | MT:430-441 | hand-built state; word list typed (copy of dict keys) | yes |
| D7 MT | :443 one active instance | MT:443 | X9 | MT:444-446 | hand-built state | yes |
| D7 MT | :462 old words stay | MT:462 | X9, X10 | MT:463-467 | typed list of five dict keys; hand-built state | X10 yes |
| D7 MT | :471 three numbers | MT:471 | X10, X12 | MT:472-474 | typed 10, 10, 0.5; title cites "ruling 3" in a removed plan | yes |
| D7 MT | :507 item check | MT:507 | X10 | MT:508-512 | typed vocabulary lists, copies of the kit's | yes |
| D7 MT | :558 object reads unproven | MT:558 | X10 | MT:559-561 | pins a flag typed in the data file | yes |
| A9 | 29 whole-number key | tests/test_config.cpp:764 | none | | real parser, typed ini lines are input; the record's doubt is a decision, not one of the faults | |
| A12 | 10 premise maps | tests/test_docs_match_code.cpp:1887 | none | | reads the real plans against the real shape page; the date constant's source is a card quote, in the code not in a check | |
| C4 | 1 CMake reader, made-up file | tests/test_layering.cpp:547 | none | | hand-written CMake text fed to the real reader is input a test builds, not a stand-in (cleared by the user's later ruling; first marked X9) | |
| A16 | 22 startup read by part | tests/test_library_jobs.cpp:1274 | none | | no check exists; returns when the env var is unset; the comment cites "the plan's run count" for 5 but nothing checks it | |
| D16 | key_delivery_test.py | tools/ch_probe/experiments/key_delivery_test.py:46 | none | | no assertions; a person reads Notepad | |
| D16 | pad_flash_test.py | tools/ch_probe/experiments/pad_flash_test.py:49 | none | | no assertions; a person reads pad flashes | |

## Counts

- Investigate records read: 114 (D7 108, A9 1, A12 1, C4 1, A16 1, D16 2).
- Records with at least one fault: 108. None: 6 (A9 29, A12 10, C4 1, A16 22, D16 two).
- By fault, records: X9 99 (RC 22, RP 56, MT 21); X10 41 (RC 6, RP 19, MT 16); X11 6 (RC 2, RP 2, MT 2); X12 4 (RC 3, MT 1); snapshot 0 separately (typed number tables are in X10).
- Records whose fault the record already named in its own words: 106 of 108. The two that did not: RC :927 (a typed error sentence) and RP :120 (typed "ready" and "running"). For X10 and X11 on message text, most records named the fault only as "typed" or "a copy"; the RP and MT notice sentences were not named.

## What I did not do

I did not open production code for the menu and kit tests beyond what the test shows. I did not run any test. I did not open the cited web page, plan, or decision logs behind the X12 marks; those marks come from what the test file's own comments and titles say.
