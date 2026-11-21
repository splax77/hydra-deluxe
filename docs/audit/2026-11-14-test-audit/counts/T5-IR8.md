Agent a19af74feb5ffc3ac, task T5-IR8 (Investigate records of D4, A3, A46). Session 63c70014-4328-49dc-9205-742235732752.

Read: the test code of all 85 Investigate records (D4 75, A3 5, A46 5). "Named?" says whether the record's own text named the fault. All three record files were written before the rulings, and none uses the words X9 to X12. For D4 I checked by keyword search plus the shared notes, not by re-reading every record in full, so "no" there means "no fault label found". Nothing was run.

## What D4 shares (applies to every D4 row unless the row says otherwise)

`tools/ch_mcp/tests/test_game_tools.py` builds every game-tool test on `ToolRig` (lines 276-317). It stands up a fake game and launcher (`Rig`), a fake registry key (`FakeReg`, 285), a fake quiet check (`FakeQuiet`, 150-170, patched in at 170-175), fake chart dumps (`FAKE`, 177), a fake popen and recorder (296, 311), a fake key driver (185), a fake play loop (`FakeLivePlay`, 194), and fake watch, player, kit, Frida and SP parts (217-273, 1363-1417). The score and schedule files are hand-written JSON (`SCORE_JSON`, `SCHEDULE_JSON`, `ROWS`, `DECLINE_SCORE`, 1075). This is X9 for every test that goes through the rig. Most checks also compare reply text against a typed message (X10).

## Table: D4 (test_game_tools.py)

Format: record number (= test line) | faults | faulty check lines | reason | named?

- 327 | X9 | 328, 333 | rig with fake game, fake quiet check | no
- 338 | X9, X10 | 340, 342-344, 349-350 | FakeQuiet(cpu=40) and typed refusal text | no
- 364 | X9 | 365-369 | FakeQuiet records the calls it was handed | no
- 371 | X9, X10 | 376, 378-379 | fake quiet refusal; typed lines | no
- 388 | X9 | 389-393 | FakeQuiet; no real sample | no
- 401 | X9, X10 | 412-415, 422, 424 | fake dump, fake pid 4242, typed "song length: 4000 ms" | no
- 443 | X9, X10 | 445-459 | fake game and registry; typed lines | no
- 463 | X9, X10 | 465-477 | fake game/registry; typed reply lines | no
- 481 | X9, X11 | 485-489 | fake hooks return "trace stopped" and it is required back | no
- 503 | X9, X10, X11 | 508-516 | restore_all replaced by a fake that raises "locked"; text echoed | no
- 521 | X9 | 522-531 | fake game, fake registry | no
- 533 | X9, X10 | 534-550 | fake adopted game; long typed advice line | no
- 554 | X9 | 555-563 | fake rig | no
- 565 | X9 | 566-568 | fake rig | no
- 572 | X10, X12 | 573 | INSTALL_CAP_S == 60.0, named "approved" | no
- 575 | X9, X10 | 576-587 | fake install worker; typed "not finished after 60 s" | no
- 589 | X9, X10, X11 | 590-600 | fake install; names typed and returned | no
- 602 | X9, X10 | 604-614 | fake game writes stats at 5 s; typed lines | no
- 616 | X9, X11 | 618-626 | hand-written stats JSON, values required back in typed lines | no
- 649 | X9 | 636-657 | hand-written stats and score files | no
- 659 | X9 | 636-664 | same files | no
- 666 | X9, X10 | 669-672 | same files; typed refusal | no
- 674 | X9 | 677-680 | same files | no
- 684 | X9, X10 | 685-694 | fake window, fake driver; typed "keys sent: S, L" | no
- 696 | X10 | 699-700 | typed refusal and lane list | no
- 704 | X9, X10 | 706-707 | fake window handle 0 | no
- 746 | X9, X10, X11 | 727-762 | fake driver and sleep; typed reply | no
- 765 | X9, X11 | 766-768 | typed "a,b" required back | no
- 770 | X9, X10 | 772-773 | typed valid-key list | no
- 777 | X9 | 778-780 | fake driver | no
- 782 | X9 | 783-786 | fake window | no
- 788 | X9, X11 | 791-794 | typed "x,y" echo | no
- 796 | X9 | 797-807 | fake driver, window | no
- 809 | X9 | 810-816 | fake rig | no
- 818 | X9 | 819-824 | fake rig, focus mocked | no
- 826 | X11 | 827-829 | typed strings required back whole | no
- 833 | X10 | 835-837 | 320.504 and 240.0 typed (song end plus a typed minute) | no
- 839 | X9, X10 | 840-844 | hand-built schedule JSON; typed 320.504 | no
- 846 | X9, X10 | 853-864 | FAKE.set_end, fake play, typed 320.504 | no
- 866 | X9, X11 | 867-873 | fake play result with typed 64 required back | no
- 875 | X9 | 876-879 | fake play | no
- 881 | X9, X11 | 883-892 | typed rows (index 7, 5398.0) required back | no
- 894 | X9 | 895-899 | fake play and ROWS | no
- 903 | X9 | 904-908 | fake play | no
- 926 | X9, X10 | 927-935 | fake play; typed lines | no
- 939 | X9, X10, X11 | 940-964 | ROWS typed in, required back in typed lines | no
- 966 | X9 | 967-969 | fake rig | no
- 971 | X9, X11 | 972-979 | fake play message echoed | no
- 981 | X9 | 982-990 | fake play | no
- 1008 | X9, X10 | 1009-1017 | fake kit and player | no
- 1019 | X9 | 1020-1024 | fake kit | no
- 1026 | X9 | 1027-1033 | fake kit | no
- 1038 | X9, X10 | 1040 | typed refusal NO_ROUTE_REFUSED | no
- 1044 | X9, X10 | 1046 | same | no
- 1051 | X9, X10 | 1052-1057 | fake kit; typed route line | no
- 1059 | X9, X10 | 1061-1065 | typed refusals | no
- 1103 | X9, X10 | 1075-1110 | hand-built chords, typed "declined: chords 1 (kick)" | no
- 1112 | X9, X10 | 1114-1118 | hand-built; typed | no
- 1120 | X9 | 1122-1128 | hand-built | no
- 1130 | X9 | 1134-1140 | hand-built; asserts only that two arguments are gone | no
- 1142 | X9, X10, X11 | 1146-1150 | hand-written refusal text returned verbatim | no
- 1154 | X9, X10 | 1156-1160 | fake rig; typed message start | no
- 1173 | X9, X10, snapshot, X12 | 1174-1176 | saved freak fixture "of the 2026-11-07 player plan"; typed FREAK_* lines pinned (comment at 140: "too long to pin") | no
- 1178 | same (X9, X10, snapshot, X12) | 1179-1181 | mouths fixture, typed MOUTHS_* lines | no
- 1183 | same | 1184-1186 | bomber fixture | no
- 1188 | same | 1189-1191 | typed MOUTHS_ORDER (141-143) | no
- 1193 | same | 1194-1196 | typed BOMBER_ORDER (144-147) | no
- 1198 | X9, X10 | 1199-1216 | "twelve made-up chords", typed expected words | no (the test calls them made-up)
- 1218 | X9 | 1219-1224 | hand-built schedule | no
- 1232 | X9, X10 | 1235-1247 | a real fixture hand-edited to feasible False; typed message | no
- 1249 | X9, X11 | 1251-1262 | made-up rows | no
- 1264 | X9 | 1267-1281 | made-up rows | no
- 1283 | none | 1284 | checks the lane-word table covers every Lane (code against code) | n/a
- 1286 | X9 | 1287-1300 | made-up rows | no
- 1302 | X9 | 1305-1315 | made-up row | no
- 1319 | X9, X11 | 1320-1334 | typed kit columns echoed | no
- 1336 | X9, X11 | 1340-1357 | typed timing values echoed | no
- 1463 | X9 | 1464-1468 | fake Frida, kit, SP | no
- 1470 | X9 | 1471-1474 | same | no
- 1476 | X9 | 1477-1480 | same | no
- 1482 | X9 | 1483-1494 | same; fake dumps | no
- 1502 | X9, X10 | 1503-1517 | fake parts; typed line labels and "machine recorder: none" | no
- 1604 | X9, X10 | 1605-1610 | fake parts; typed "press-1.csv" | no
- 1612 | X9, X10 | 1613-1619 | typed "press-2.csv" | no
- 1621 | X9 | 1622-1637 | fake play | no
- 1673 | X9, X10, X11 | 1679-1707 | writer replaced to raise "held open", echoed | no
- 1769 | X9, X10 | 1770-1773 | typed "play session: " | no
- 1775 | X9, X11 | 1776-1782 | typed row echoed | no

(Where I wrote "same" the cases share one fault set and one reason.)

## Table: A3 (tests/test_activation_rows.cpp)

- A3.md #11, test line 1167 | X9, X10 | 1169-1182 | hand-built Activation, HydraRecord and BackendSqueeze row; typed "Transfer scale unknown." | partly: the record said the state cannot arise from a fresh record (the hand-built input), not as a fault label
- A3.md #12, line 1186 | X9, X10 | 1188-1209 | hand-built chord; typed "[Kick - Red snare (Ghost) ..." strings | no
- A3.md #14, line 1308 | snapshot, X12, X10 | 1318-1328 | real chart, but the figures "77.9 ms", "72.0 ms", "65.9 ms short" are pinned; the section header (1213) says they are what `hydra_replay schedule` prints (a tool, not the engine's own record) | partly: the record says the claim is a game fact the engine does not own
- A3.md #15, line 1331 | snapshot, X12, X10 | 1334, 1340-1341 | pathstring "3- 1 2" and "84.5 ms", "5.4 ms safe" pinned | partly, same
- A3.md #20, line 1543 | none | 1544-1553 | real charts, only checks that no row appears when no chord times are passed | n/a

## Table: A46 (tests/ui/uitest_report_windows.cpp)

All five open the fixture built at 98-145: `report_samples` rows, "sample path subtitle", a fake build state and a made-up time (`local_time`). That is X9 for all.

- A46.md test_states, line 306 | X9, X10, X11 | 310-394 | forced build states; typed "sample failure message" echoed (329-334); typed messages (315-349, 391); made-up times (345-346) | no
- A46.md test_close_button, line 523 | X9, X10 | 526-569 | sample report; typed "Building the report from your library..." | no
- A46.md test_chip_label, line 621 | X9 | 624-654 | sample rows; the expected cells come from the code (`tier_label`, `columns[c].cell`), so no constant pin | no
- A46.md test_header_narrow, line 752 | X9, X11 | 756-769 | "sample empty reason", made-up time 13:42 required back as "Built 13:42" | no
- A46.md test_fits_screen, line 802 | X9 | 814-815, 830 | hand-typed saved-window text stands in for the user's file; sample report | no

## Counts

- Investigate records read: 85 (D4 75, A3 5, A46 5). All reached.
- Records with at least one fault: 83 (D4 74, A3 4, A46 5). Records with none: 2 (D4 1283, A3 #20).
- Records per fault (a record can hold several):
  - X9 made-up input or fake: D4 about 69 (every row that goes through the rig or a hand-built file, i.e. all D4 rows except 572, 696, 826, 833, 1283), A3 2, A46 5. Total about 76.
  - X10 constant pin: D4 about 38, A3 4, A46 2. Total about 44.
  - X11 echo: D4 about 22, A3 0, A46 2. Total about 24.
  - X12 cited non-code: D4 6 (572, 1173, 1178, 1183, 1188, 1193), A3 2 (#14, #15). Total 8.
  - Snapshot: D4 5 (1173-1193), A3 2. Total 7.
  These per-fault numbers are tallied by hand from the table above and are approximate to within a few rows.
- Records whose fault the record already named: 0 by label. 4 in part (A3 #11, #14, #15 noted the made-up state or the missing game fact, without calling it a fault; D4 1198 calls its own chords "made-up").

## Five clearest

1. D4 1198: the test itself says "twelve made-up chords ... hand-built", with a typed expected order line.
2. A46 test_states: every reply is a forced state with made-up messages and times, checked against typed text.
3. D4 338: a fake quiet check returns 40 percent CPU and the test requires typed refusal text.
4. D4 1173: expected lines are typed copies of one run's output from a plan's fixtures (the file says "too long to pin here").
5. A3 #14: real chart, but every figure is a number pinned from `hydra_replay schedule` output, with no rule given.

## Not reached

None.
