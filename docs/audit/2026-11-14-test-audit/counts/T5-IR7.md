Agent id: aafe93a4bea16d8e9 (task T5-IR7, record files D11.md, A1.md, A11.md, A35.md, A44.md; verdict re-read: Investigate)

# T5-IR7: Investigate records of D11, A1, A11, A35 and A44 read by the user's test rulings

I read all 86 Investigate records in my five files: 78 in D11.md (every one of them is a case of `tools/ch_probe/tests/test_play_chart.py`), 2 each in A1.md, A11.md, A35.md and A44.md. For D11 I read the test file's helpers and fakes (lines 1-130, 180-1180, 1340-1470, 1690-1870) and checked each record against the code it names. I took lines 1180-1340, 1470-1690 and 1870-2312 from the record's own line citations and did not re-read them one by one. Where a row rests on that, the reason says "record's lines". For the other four files I opened the test code.

## What decides most of D11

The test file's own header (lines 13-15) says "A run is tested on fakes for the song clock, the score and the keys". Every case that runs `play_chords` or `live_play` gets a fake clock (`_FakeSongClock`, `_JumpingSongClock`, `_ScriptedSongClock`), a fake game (`_FakeGame`), and a schedule from `_schedule_of`. That helper (lines 64-83) hand-builds the file `hydra_replay schedule` writes: its own comment calls the aims, send-by times and song end "hand-picked". A schedule is engine output, so a hand-built one is X9 even when the loader then reads it correctly. The other fakes are `_ThresholdEngine` (fake engine memory), `_FakeProc` (fake game process), `fakes.kit_reply` and `_SplitReplyPress` (fake replies from the kit), hand-written fallback blocks and hand-picked ratios. All of these are X9.

Two cases that use a real fixture file (`freak_limits_schedule.json`, `mouths_refine_schedule.json`) still feed a typed game read or a fake clock, so I marked them X9 on that part only.

The second common fault is X10: many cases require the code's own message text, label or column name back as typed text ("cannot make", "no engine", "....", "Done.", column names). The third is X11: a value typed into a fake and required back (a threshold of 64, a window handle 0x77, a cap of 320.504, a fake memory value 32.61592).

## Table

Columns: record, test (file:line), faults, faulty checks, reason, did the record already name it. "Named" means the record's own words said the input was a fake, hand-built, typed or "claim not checked" for that reason. No record called any of it a fault, because the rubric had no question for it. `TP` is `tools/ch_probe/tests/test_play_chart.py`. `RS` is `tests/report_samples.cpp`. `DM` is `tests/test_docs_match_code.cpp`. `TV` is `tests/test_tied_variants.cpp`. `UP` is `tests/ui/uitest_paths.cpp`.

| Record | Test | Faults | Faulty checks | Reason | Named |
|---|---|---|---|---|---|
| D11 6 | TP:184 refusal stops the load | X9, X11 | TP:185-189 | hand-built schedule with a made-up refusal; the made-up text is required back | yes |
| D11 7 | TP:191 infeasible squeeze | X9, X10 | TP:193-196, 199-200 | hand-built activation and squeeze; "cannot make" is the loader's own message | yes |
| D11 9 | TP:228 no timing constant | X10 | TP:232-233 | the exact typed list of three constant names; the values have no stated reason | yes |
| D11 19 | TP:384 frozen clock | X9 | TP:386-388 | fake song clock and hand-built schedule | yes |
| D11 20 | TP:391 wall cap | X9 | TP:394-398 | fake wall time and clock | yes |
| D11 21 | TP:400 caller cap | X9, X10 | TP:403-409, 414 | fake clock; "run cap of 1 s reached" typed | yes |
| D11 22 | TP:462 no song exits 2 | X9, X10 | TP:436-449, 465-466 | fake process, fake scan, fake clock; the finder's message and its 120 typed | yes |
| D11 23 | TP:470 opened read only | X9 | TP:437-460, 472-473 | fake process opener, so the real open mode is never run | yes |
| D11 24 | TP:491 on_press sees every row | X9 | TP:481-489 | fake clock and game, hand-built schedule | yes |
| D11 25 | TP:499 stop after second chord | X9 | TP:481-505 | same fakes | yes |
| D11 26 | TP:507 stop leaves no key held | X9 | TP:513-529 | `send_key` replaced, fake clock and game; the real key sender is never run | yes |
| D11 27 | TP:531 stop false | X9 | TP:532-534 | same fakes | yes |
| D11 28 | TP:549 no engine | X9, X10 | TP:553-564, 566 | fake finder, fake process; "no engine" typed back | yes |
| D11 29 | TP:574 dots go to out only | X9, X10 | TP:578-585 | fake search and process; "...." typed | yes |
| D11 30 | TP:621 cap reaches play_chords | X9, X11 | TP:590-618, 622 | fake engine and mocked `play_chords`; 320.504 typed in and read back | X9 yes, X11 no |
| D11 32 | TP:645 keys through the driver | X9, X11 | TP:649-657, 664 | mocked `play_chords`, fake window and driver; handle 0x77 typed in and read back | X9 yes, X11 partly |
| D11 34 | TP:754 dynamics beside lanes | X9, X11 | TP:690-694, 756-758 | hand-built chord dump; the typed dynamics are read back | X9 yes, X11 no |
| D11 35 | TP:760 ghost threshold | X9, X11 | TP:701-725, 764 | `_ThresholdEngine` fake; threshold 64 typed in and read back; expected axes come from the same `pad_axis` | yes |
| D11 36 | TP:769 threshold 0 raises | X9, X10 | TP:701-725, 772 | fake engine reading 0; field name "OFF_DYN_THRESHOLD" typed | X9 yes, X10 partly |
| D11 37 | TP:774 keys route reads no threshold | X9 | TP:775-785 | fake engine, mocked `play_chords` | yes |
| D11 38 | TP:787 no ghost, no read | X9 | TP:788-793 | fake engine, hand-built schedule | yes |
| D11 39 | TP:795 axes reach the press | X9 | TP:796-808 | fake engine, clock, game; threshold 64 typed | yes |
| D11 40 | TP:810 ghost or accent kick | X9, X11 | TP:816-821 | hand-built dump, fake engine; threshold 64 typed in and read back | yes |
| D11 41 | TP:835 dict reply in row | X9, X11 | TP:842, 849-855 | `fakes.kit_reply`; the reply's frames and times typed in and required back in the row | yes |
| D11 43 | TP:900 wall times around press | X9 | TP:881-904 | fake wall clock and fake press | yes |
| D11 45 | TP:912 no-press chord stamps once | X9 | TP:914-920 | hand-built declined chord with an empty press; no real chart shown to produce it | yes |
| D11 46 | TP:922 wall clock is the watch's | none | TP:924-925 | reads two function defaults from the module; no fake, no typed value, no cited non-code. It pins an implementation detail, which no ruling names | n/a |
| D11 50 | TP:997 failed write kept | X9, X10 | record's lines 1000-1012, 1014 | fake writer class raising `PermissionError`; first column name "index" typed | X9 yes, X10 no |
| D11 54 | TP:1116 one group no then | X9 | TP:1055-1122 | fake reply press, hand-built chord, scripted scores | yes |
| D11 56 | TP:1138 later group no first press | X9, X10 | TP:1139-1145 | hand-built Press no real file produces; message text typed | yes |
| D11 57 | TP:1172 rise between presses | X9 | TP:1153-1177 | scripted score reads, hand-built schedule | yes |
| D11 58 | TP:1179 status credits declined chord | X9, X10 | record's lines 1182-1183 | scripted run; the status sentence typed in full | yes |
| D11 59 | TP:1185 rise unexplained | X9, X10 | record's lines 1188-1190 | scripted reads; the sentence typed in full | yes |
| D11 60 | TP:1192 new columns reach log | X9, X10 | record's lines 1199-1202 | fake run; column names typed ("probe" is not exercised) | X9 yes, X10 partly |
| D11 64 | TP:1254 retime moves only that activation | X9 | record's lines 1254-1265 | hand-built presses and typed read | yes |
| D11 65 | TP:1285 end in song seconds | X9, X11 | record's line 1286 | fake game memory built from the offsets the code reads; 32.61592 written then read back | yes |
| D11 66 | TP:1288 none while byte clear | X9 | record's line 1289 | same fake memory | yes |
| D11 67 | TP:1291 live_play hands the reader | X9 | record's line 1306 | fake engine, mocked `play_chords` | yes |
| D11 68 | TP:1327 focus then press | X9, X11 | record's lines 1329-1331 | recorders in place of focus and keys; handle 0x1234 typed in and read back | yes |
| D11 69 | TP:1333 no window no focus | X9 | record's line 1335 | same recorders | yes |
| D11 71 | TP:1376 late chord dropped | X9, X10 | TP:1370-1392 | `_JumpingSongClock`, 50 ms send-by hand-picked; lateness bounds 100 and 110 typed | yes |
| D11 72 | TP:1394 counts leave the drop out | X9 | TP:1396 | same fakes | yes |
| D11 73 | TP:1398 printed line says dropped | X9, X10 | TP:1403-1406 | same fakes; the word "dropped" typed | yes |
| D11 74 | TP:1460 read at last send is sent | X9 | TP:1422-1465 | fake clock stepped to the exact float; hand-built schedule | yes |
| D11 75 | TP:1467 0.01 ms past is dropped | X9 | TP:1467-1472 | same | yes |
| D11 76 | TP:1474 split chord past last send | X9 | record's lines 1478-1483 | hand-built split chord, fake clock | yes |
| D11 80 | TP:1569 no activation read | X9 | record's lines 1571-1574 | real Freak file, but the reader returning None and the clock are fakes | yes |
| D11 81 | TP:1597 later read moves late gem | X9 | record's lines 1601-1602 | real file, typed +3 ms game read | yes |
| D11 83 | TP:1621 read at threshold keeps late | X9 | record's line 1624 | typed read at the threshold; no source for the edge | yes |
| D11 84 | TP:1626 late gem with inside group | X9 | record's lines 1630-1638 | real Mouths file, typed read | yes |
| D11 85 | TP:1640 squeeze-in before threshold | X9 | record's lines 1642-1651 | typed reads | yes |
| D11 86 | TP:1654 press past last send dropped | X9 | record's lines 1659-1661 | real chord, fake clock | yes |
| D11 87 | TP:1663 other action refused | X9, X10 | record's line 1667 | a fallback action the writer never writes; `'drop'` typed | yes |
| D11 89 | TP:1711 early and late ratios | X9 | TP:1706-1718 | hand-picked ratios 0.5 and 2.0; no real file has them differ | yes |
| D11 93 | TP:1760 read just inside | X9 | TP:1748-1766 | hand-built range (990, 1010) | yes |
| D11 94 | TP:1768 null side open | X9 | TP:1769-1776 | hand-built range, 1e9 stand-ins | yes |
| D11 95 | TP:1778 no fallback unmoved | X9 | TP:1780-1784 | hand-built press | yes |
| D11 96 | TP:1786 squeeze-in outside plain | X9 | TP:1788-1791 | hand-built fallback block | yes |
| D11 97 | TP:1793 squeeze-out late gem outside | X9 | TP:1795-1804 | hand-built presses | yes |
| D11 98 | TP:1822 fields are read | X9, X11 | TP:1813-1831 | hand-written end fields in a hand-built schedule, read back | yes |
| D11 99 | TP:1833 null range | X9 | TP:1835-1838 | hand-built | yes |
| D11 100 | TP:1840 missing field named | X9, X10 | TP:1840-1847 | hand-built schedule with a key deleted; message format typed | yes |
| D11 101 | TP:1849 null number named | X9, X10 | TP:1849-1861 | hand-built; "null `<key>`" and "has no `late`" typed | yes |
| D11 104 | TP:1923 reply without groups | X9, X10 | record's lines 1926-1938 | hand-made kit replies; sentence typed | yes |
| D11 105 | TP:1940 reply not a dict | X9, X10 | record's lines 1952-1962 | hand-made replies; text typed | yes |
| D11 106 | TP:1964 keys reply not read | X9 | record's line 1967 | fake keys run | yes |
| D11 107 | TP:1979 missing key refused | X9, X10 | record's lines 1987-1995 | hand-built doc with a key deleted; texts typed | yes |
| D11 108 | TP:1997 chord with no notes | X9, X10 | record's line 2000 | hand-built chord; "chord 45 has no gems" typed; no sign the C++ can write it | X9 yes, X10 partly |
| D11 109 | TP:2002 empty chord list | X9, X10 | record's line 2004 | hand-built schedule; "schedule.json has no chords to play" typed | yes |
| D11 110 | TP:2006 press names missing chord | X9, X10 | record's lines 2009, 2012 | hand-built; messages typed | X9 yes, X10 no |
| D11 111 | TP:2043 main exits nonzero | X9, X10 | record's lines 2067-2080 | `live_play` replaced by fakes; reasons and "Done." typed (and they disagree on spacing) | yes |
| D11 112 | TP:2082 main prints notes | X9, X10 | record's lines 2086-2088 | made-up note; "Done.\n" typed | yes |
| D11 114 | TP:2127 clock past last note | X9, X10 | record's lines 2129-2132 | fake clock at 200 s; line typed in full | yes |
| D11 115 | TP:2155 fakes send real reply shape | X9 | record's lines 2158-2169 | checks the fakes against a text parse of `kit.ts`, not against a saved real reply | yes |
| D11 116 | TP:2239 stall in each step | X9, X10, X12 | record's lines 2244-2252 | fake wall jumps of 0.3 s; 300 and 20 ms typed; 20 ms is "PS3's acceptance line" in a plan the record did not open | X9 yes, X12 yes, X10 partly |
| D11 117 | TP:2254 dropped row carries waits | X9 | record's lines 2276-2280 | fake 2 s jump | yes |
| D11 118 | TP:2282 first row carries times | X9 | record's lines 2291-2293 | fake, hand-built split chord | yes |
| D11 119 | TP:2295 timing columns in log | X9 | record's lines 2305-2308 | fake run | yes |
| A1 sample_index | RS:47 | X9 | RS:47-62 | a hand-built list of library rows (made-up folders and ids) goes into the real index builder | partly (record says "types only the input rows" and finds the rows disagree with the paths) |
| A1 counted_against | RS:64 | X9 | RS:64-68 and `sample_path_rows` RS:12-45 | glue over hand-built report rows (acts, skip and sqin made up and disagreeing with the path text) | partly |
| A11 4 | DM:502 what counts as a code name | X9 | DM:502-520 and the record's lines | typed text spans fed to the classifier; the real doc pages are not used | yes ("no real input") |
| A11 6 | DM:1092 which CH lines name Hydra code | X9 | DM:1092-1105 | a hand-written page ("docs/clone-hero/04-fills.md" and its lines) | yes ("only fixture input") |
| A35 5 | TV:263 clamped_sqin charts | none | | loads two real corpus charts and runs the real analysis; checks a property of the engine's own record | n/a |
| A35 8 | TV:361 twin SP ends | none | | a chart the test names runs through the real search; the expected offsets are worked from the tempo and named ticks, comment at 367-370 | n/a |
| A44 6 | UP:513 paths-fit-narrow | X9, X10 | UP:69-90 (`FakeClipboard`), UP:528 | a fake clipboard stands in for the OS one; "Copied!" typed | no |
| A44 7 | UP:540 paths-long-path | none | | real library scan of the test corpus, real window sizes, expected sides are the display's constants read by name | n/a |

## Judgment calls the reader may overrule

- D11 46 is marked "none": it checks two defaults, which is an implementation pin, but it has no fake, typed number or cited non-code.
- A11 4 and 6 are marked X9 because the doc-checker is fed typed or hand-written page text. The helper functions are pure text rules, so a reader who counts typed spans as ordinary inputs would call both "none".
- A35 8's expected offsets (-491 and 2/3 and so on) are typed numbers. I did not mark them because the test names the rule (tempo and ticks) and the inputs they follow from.
- D11 71's lateness bounds and D11 116's 300 and 20 ms I marked X10 as typed bounds. They follow from the fake's own jump, so a reader could call them inputs.
- A44 6's `FakeClipboard` I marked X9 under "a fake of any kind".

## Counts

Counted from the table's own rows.

- Investigate records read: 86 (D11 78, A1 2, A11 2, A35 2, A44 2).
- Records with at least one fault: 82. Records with none: 4 (D11 46, A35 5, A35 8, A44 7).
- X9, made-up input: 81 records (every faulty record except D11 9).
- X10, typed constant or text: 28 records.
- X11, echo: 10 records.
- X12, cited non-code: 1 record (D11 116).
- Snapshot: 0 records.
- Records whose fault the record already named in its own words ("fake", "hand-built", "typed", "claim not checked"): 70 of the 82 name every fault they hold. 12 name only part of it: D11 30, 32, 34, 36, 50, 60, 108, 110, 116, A1 sample_index, A1 counted_against, and A44 6 (which names none). No record called any of it a fault, because the rubric had no question for it.

## What was not done

I read the test file's code directly for lines 1-130, 180-1180, 1340-1470 and 1690-1870. For the rest of D11 (lines 1180-1340, 1470-1690, 1870-2312) I relied on the record's own line citations and the fakes' definitions I had read. Rows say "record's lines" where that applies. The four "none" rows and the A11 rows were judged from the code I opened.
