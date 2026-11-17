Agent id: ad19a2520389cdf15 (task T8-E3-V-L-C17-L-C18, session f3bbb32b)

Neither label file carries my id (L-C17 is adc13e479d5c744f3, L-C18 is aef08d9cfbe47d367).

## Counts

| batch | rows in batch | lines in label file | rows I agree with | rows I change |
|---|---|---|---|---|
| L-C17 | 79 | 79 | 66 | 13 |
| L-C18 | 93 | 93 | 86 | 7 |

Every batch row has exactly one line in each label file (checked by a script comparing the row lists: no row missing, none doubled). Every key in each ends table names rows that are in the batch and carry that key in the scout's own table. Two of those ends cells break if my changes are taken: see "Ends tables" below.

## Rows I change

I read `tools/ch_probe/tests/test_play_chart.py` and `tools/ch_mcp/tests/test_game_tools.py` at each row's test. I checked both errors: a fake left without a key, and a key on a test that never uses that fixture.

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C17 | counts/T5-IR7.md:28 | C-stub | C-stub, C-clock | TP:470 runs `_main_with_no_song`, which builds `_FakeTime` and feeds its `sleep` and `now` to the finder, the same fake clock that gave :27 its C-clock. |
| L-C17 | counts/T5-IR7.md:45 | C-clock, C-stub | C-clock, C-stub, O-gamemem | TP:900 runs `play_chords` on `_FakeGame` (fake score and press log), which the scout's own rule says gets O-gamemem. |
| L-C17 | counts/T5-IR7.md:46 | C-clock | C-clock, C-stub, O-gamemem | TP:912 is `_run_with_a_slow_press`, with `_FakeGame` and a hand-written press function, the same fakes as :45. |
| L-C17 | counts/T5-IR7.md:49 | C-clock, C-reader, C-stub | C-clock, C-reader, C-stub, O-gamemem | TP:1116 presses `_SplitReplyPress`, whose replies come from `fakes.kit_reply`, the fake game reply that gave :44 its O-gamemem. |
| L-C17 | counts/T5-IR7.md:50 | C-clock, C-reader, C-stub, P-message | P-message | TP:1138 never runs `play_chords`: it calls `press_messages([lone])` on one hand-built Press and reads the error text, so no clock, reader or stub is used (X9 is then left without an X9-family key, as on :74 to :79). |
| L-C17 | counts/T5-IR7.md:55 | C-clock, C-reader, O-gamemem | no key | TP:1254 only builds three `Press` objects and calls `retime_from_end` on them; `_play_schedule` and the fakes are in other tests (1223), not this one, so it is the same hand-built-Press case as :74 to :79. |
| L-C17 | counts/T5-IR7.md:64 | C-clock | C-clock, C-reader, C-stub, O-gamemem | TP:1460 goes through `DropBoundaryTest._run`: a scripted score reader (`lambda: next(scores)`), `_SplitReplyPress` and `_ScriptedSongClock`. |
| L-C17 | counts/T5-IR7.md:65 | C-clock | C-clock, C-reader, C-stub, O-gamemem | TP:1467 is the same `_run` and the same fakes as :64. |
| L-C17 | counts/T5-IR7.md:66 | C-clock | C-clock, C-reader, C-stub, O-gamemem | TP:1474 is the same `_run` and the same fakes as :64. |
| L-C17 | counts/T5-IR7.md:72 | C-clock | C-clock, C-reader, C-stub, O-gamemem | TP:1654 calls `DropBoundaryTest._run` with `_SplitReplyPress`, so the press, score reader and clock are all fakes. |
| L-C17 | counts/T5-IR7.md:86 | C-stub | C-stub, C-clock, O-gamemem | TP:1964 calls `_run({"start_s": 12.0})`, which is `_FakeSongClock`, `_FakeTime` and `_FakeGame`. |
| L-C17 | counts/T5-IR7.md:97 | C-clock, C-stub | C-clock, C-stub, C-reader, O-gamemem | TP:2282 passes `lambda: 0` as the score reader and `_SplitReplyPress` (kit reply) to `play_chords`. |
| L-C17 | counts/T5-IR7.md:98 | C-clock, C-stub | C-clock, C-stub, C-reader, O-gamemem | TP:2295 has the same `lambda: 0` reader and `_SplitReplyPress` as :97. |
| L-C18 | counts/T5-IR8.md:20 | C-rig, O-gamemem, P-message | C-rig, C-reader, O-gamemem, P-message | Test 463 calls `ch_start` (lines 465), so the module-wide `FakeQuiet` patch (170-177) applies, which the scout gave to every other `ch_start` row. |
| L-C18 | counts/T5-IR8.md:21 | C-rig, O-gamemem, E-echo | C-rig, C-reader, O-gamemem, E-echo | Test 481 calls `ch_start` at 483 and 494, so it runs on `FakeQuiet`. |
| L-C18 | counts/T5-IR8.md:26 | C-rig | C-rig, C-reader | Test 565 calls `ch_start` at 567, so it runs on `FakeQuiet`. |
| L-C18 | counts/T5-IR8.md:48 | C-rig, E-echo | E-echo | Test 826 only calls the pure function `GT.split_keys` on typed strings; no `ToolRig` is built in it. |
| L-C18 | counts/T5-IR8.md:50 | C-rig, F-schedule, P-other | F-schedule, P-other | Test 839 only calls `GT.schedule_run_cap_s` on a hand-built schedule file; no `ToolRig` is built. |
| L-C18 | counts/T5-IR8.md:80 | C-rig, F-schedule, P-message | F-schedule, P-message | Test 1198 calls `GT.chords_to_play` and `GT.order_lines` directly on a hand-built schedule; no `ToolRig`. |
| L-C18 | counts/T5-IR8.md:81 | C-rig, F-schedule | F-schedule | Test 1218 is the same direct calls as :80; no `ToolRig`. |

## Ends tables

L-C17: the ends cell for C-reader names `counts/T5-IR7.md:55` as its least-fitting row. With my change that row has no key, so the cell needs another row (`:68` or `:72` fit). The other ends cells (C-clock, O-gamemem, C-stub, C-ownchecker, E-echo, P-message, P-name, P-other, Z-cite, F-row) name rows in the batch that carry the key, before and after my changes.

L-C18: every ends cell names a row in the batch that carries that key. None of my changes touches an ends cell (C-rig plainest `:25`, least `:65`; C-reader plainest `:15`, least `:30` all still carry their keys).

## Not changed, looked at

- L-C17 :73 (TP:1663): the scout gave P-name and no fake. Right: the test loads the real Freak fixture and calls `dataclasses.replace` on one real Press; no fake class is used.
- L-C17 :80, :82, :83, :87 to :90: hand-built schedule files read by real code, so no fake. Right.
- L-C17 :27 and :28 sit on `_FakeProc` and a fake scan; C-stub is right.
- L-C18 :27 has no C-rig: right, test 572 only compares `GT.INSTALL_CAP_S` to 60.0.
- L-C18 :49 (TP 833) builds a `ToolRig` only to call `cap_for_end`, which does not use the rig's fakes; I kept the scout's C-rig because the test does build it.
- L-C18 :39 to :42 reach `ToolRig` through `_key_rig`, and :75, :76 and :82 through `_decline_rig`; C-rig is right.
- L-C18 `ch_start` rows: the only `ch_start` calls inside the batch's test ranges are at lines 333 to 606 and fall in the tests of :13 to :23, :26 and :30. I found no `ch_start` call in any helper between 606 and 1775 (`_chain_rig`, `_kit_rig`, `_decline_rig`).

## Side notes

- `tools/ch_probe/tests/test_play_chart.py` (rows :74 to :79, :73, :55 and :50 in L-C17): the counter's X9 mark stands on these rows and no X9-family key fits, because the stand-in is a hand-built `play_chart.Press` or a press range given to a pure function (`retime_from_end`, `press_messages`). The scout of L-C17 flagged this; my changes add :50 and :55 to the same group. Whether a hand-built Press is a stand-in for `load_schedule`'s output is the main session's call.
- Hand-built schedule files: L-C17's scout treated them as input (no key). L-C18's scout labelled the same kind of thing F-schedule (rows :50, :69 to :72, :80 to :82, :75 and :76). `questions.md` question 1 says a hand-written doc the real code reads is input. This is a split between the two files, left for the main session; I did not change those rows.
- `test_play_chart.py:1738` and `:1745` quote plan paths marked removed, fine; `:2172-2174` cites `docs/superpowers/plans/2026-11-12-player-stall-fixes.md` as the source of `_QUIET_MS`; that matches the Z-cite on :95.
- `test_game_tools.py:826` (L-C18 :48): the counter marked only X11, but the test echoes strings through a pure function, with no `ToolRig` at all.

## Not checked

I did not open the C++ rows (L-C17 :99, :100, :105; L-C18 :106 to :109, :116 to :120) beyond the counter's reason and the scouts' notes. I did not read the records under AUD for any row. For L-C18 I read test bodies fully only for rows :20 to :26, :28, :29, :39 to :50, :69 to :82 (test lines 746 to 840, 1096 to 1345) and used a script to list `ch_start` and `ToolRig` use for the rest; rows :51 to :64, :83 to :100 were not read test-by-test and rest on the scout's labels.
