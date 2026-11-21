Agent id: a638463e1f3a3c6cc (task T8-E3-V-L-C21-L-R10, session f3bbb32b)

## Counts

| batch | rows in the batch | lines in the label file | rows I agree with | rows I change |
|---|---|---|---|---|
| L-C21 | 119 | 119 | 112 | 7 |
| L-R10 | 88 | 88 | 84 | 4 |

Every batch row has exactly one line in its label file, in batch order (I compared the row column of each .tsv with the row column of each label table by script). The ends tables name rows that are in their batch and carry the key, except one cell: L-C21's least-fitting row for C-clock is counts/T5-KR3.md:64, which loses that key below. Its new least-fitting row is counts/T5-KR3.md:117.

How I checked. For L-R10 I opened the record at the line in `row` for all 88 rows (C11, C12, C13, C14, C2, C3, C4) and read its seven questions and verdict. For L-C21 I read the counter's reason for every row and opened the test file for the rows that name a fake, a rig or a fixture (test_trace_hits.py, test_sp_watch.py, test_stall_harness.py, test_edge_probe.py, test_input_driver.py, test_engine.py, test_play_chart.py, test_agent_cost.py, test_game_tools.py, test_bot_run.py, test_replay_schedule.cpp, test_highway_draw.cpp, test_store_records.cpp, test_report_rows.cpp with dm_fixture.h). I did not open every fake-using test in test_front.py, test_frida_tools.py or test_frida_attach.py; I accepted the scout's keys there because the counter's reasons name the fake and the scout's lines match the class names I saw.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C21 | counts/T5-KR3.md:15 | P-message | P-message, C-stub | The test (test_trace_hits.py:132) runs `song_end_ms` against `FAKE`, the stand-in for the hydra_replay dump from test_hydra_end.py (`FakeDumps`, started in setUpModule); a stub program with no key. |
| L-C21 | counts/T5-KR3.md:16 | P-message | P-message, C-session, C-stub | The test (test_trace_hits.py:265) runs `TH.main` with `FakeSession([])` and `FAKE.set_end`, so a fake session and the fake dump program stand behind the typed line. |
| L-C21 | counts/T5-KR3.md:17 | P-message | P-message, C-session | The test (test_trace_hits.py:311) runs `TH.main` with `FakeSession([])` and `FakeFrida`, a fake session the scout's grep did not match to this test. |
| L-C21 | counts/T5-KR3.md:64 | O-process, C-clock | O-process | `test_opens_the_game_read_only` (test_sp_watch.py:364) uses `fakes.fake_process` and a local `no_song` function but never `FakeClock`; the C-clock key is for a fixture the test does not touch. |
| L-C21 | counts/T5-KR4.md:37 | P-message | P-message, C-rig | The test (test_game_tools.py:1154) builds `_decline_rig()`, a `ToolRig` around a fake `live_play`, and asserts the fake saw nothing, so the rig is part of the test. |
| L-C21 | counts/T5-KR4.md:72 | E-echo, Z-cite, P-other | E-echo, Z-cite, P-other, F-result | The counter's X9 mark has no F, O or C key: the test (test_replay_schedule.cpp:793) uses `hand_built_squeeze_in()` and a hand-made `SqueezeGeometry` with a typed scale, a hand-built result. |
| L-C21 | counts/T5-KR5.md:121 | no key | F-record, F-db | The test (test_store_records.cpp:1023) saves a hand-built `failure_record(... "No notes.")` (the same fault the scout keyed F-record on KR5:120) and then rewrites a stored column with a hand-typed SQL `UPDATE`, which is F-db. |
| L-R10 | records/C11.md:161 | K-selfmade | K-selfmade, P-message | The record's question 4 names the typed message "unexpected chart type: x.txt" at line 245, a typed message left without a key. |
| L-R10 | records/C11.md:488 | K-namebody | K-namebody, F-record, E-echo | The record's question 3 says the test uses one made-up row (`e.md5 = "abc"`) written to an in-memory store, and its question 4 says 200000.0 and 250.0 are checked as the same values read back. |
| L-R10 | records/C13.md:278 | K-comment, P-other, Z-cite | K-comment, P-other, Z-cite, P-onefile | Question 4 says line 28 pins the window as 180000, "a snapshot of a constant", and the verdict says to drop it; that is a constant pinned in one test file. |
| L-R10 | records/C13.md:665 | P-setting, K-floor | P-setting, K-floor, C-stub | Question 3 says the download and the launch are test seams (a file copy and a fake process), so the test runs on stand-ins for the real programs. |

## Rows I looked at hard and kept

- L-C21 counts/T5-KR3.md:73 and :74 (test_agent_cost.py:173, 196). The scout's O-transcript stays. The test writes a hand-made agent transcript file that the real script reads. Keys.md says a hand-written file the real code reads is input, but it also defines O-transcript as a transcript stand-in; the two readings disagree for these two rows. This is a key-list question for the main session, not a label error I can settle (see Side notes).
- L-C21 counts/T5-KR3.md:20 (test_trace_hits.py:406). X9 and X11 are marked and the scout gave only E-echo. The score file is hand-written and read by the real `notes_from_score`, so under keys.md it is input. I agree.
- L-C21 counts/T5-KR4.md:57, :59, :62 and counts/T5-KR5.md:28, :31, :117. Each has an X9 mark with no X9-family key. I agree with the scout: each hand-written rules folder, JSON row, md5 probe, real scan or scene note is input to real code (checked test_highway_draw.cpp:914: only `scene.notes` is hand-built, no activation). See Side notes for the one-line rule these rows share.
- L-C21 counts/T5-KR5.md:126 and :127. The counter's reason is blank. Reading `fill_store` in test_report_rows.cpp:96-115: the library entries are hand-built (`library_entry_for_file`) and the records come from real corpus charts, so F-row on :126 is right. At :919 the test saves `test::tied_variant_record()`, a hand-made record, so F-record is right.
- L-C21 counts/T5-KR3.md:31 and :33 (test_edge_probe.py:85, 97). The file defines `_Wall`, `_Clock` and `_Game`, but neither test uses them (both only call `load_probes`), so no C key is right.
- L-C21 counts/T5-KR3.md:14 (test_sp_tools.py:674) carries no C key. It is built on the file's `self.tools()` fake game, but the counter marks X10 only and the test never asks the game anything; I left it.
- L-R10 counts of record rows where the scout wrote one key and the record names a second fault only as a side matter (for example C12.md:523 `Approx(2.0)` typed, C3.md:969 the cited user answer in the reason). I did not add keys there because the record's own verdict does not name the fault as a failing line.

## Side notes

- records/C3.md:969 (test_launch.cpp:83): the record's question 5 quotes "The user's answer of 2026-10-29" as the reason the shortcut line is pinned. If that sentence sits in the test's own comment, Z-cite would also fit. I did not open the test, so I did not add it.
- counts/T5-KR3.md:73 and :74: key-list gap. A hand-written Claude Code transcript file read by the real cost script is both "hand-written file the real code reads" (input) and "a transcript" (O-transcript). Keys.md does not say which wins. Two rows depend on it.
- counts/T5-KR3.md:64 (test_sp_watch.py:364): the test replaces the engine search with a local function `no_song` that raises. No key fits a locally defined replacement for a search function (C-reader and C-stub do not name it), so I left it unkeyed beyond O-process.
- X9 marks resting on hand-written input: counts/T5-KR3.md:20, counts/T5-KR4.md:57, :59, :62 and counts/T5-KR5.md:28, :31, :117 carry an X9 mark and no X9-family key by the scout's choice. The brief says each X9 mark needs one key of its family. The scout's reading follows keys.md ("input, not a stand-in"). The counter's mark and the key list disagree for these seven rows; the main session should decide whether the mark or the key list stands.
- tests/test_replay_schedule.cpp:793-800 (KR4:72): the test name says "moving_side reads the anchor's press, not the squeeze's kind", and its comment cites "the Dynamic Hit Window plan's figures" for 1.0044 and 0.9970. That plan is a document, so the Z-cite on this row is a document cite, not a code cite.
- records/C12.md:331 and records/C12.md:497 side notes in the scout's file are accurate to the records (derive-once finding on `sqout_cost`, empty chart shows 7 stars). I read the records and agree they are in the records as written.
- records/C4.md:80 holds a split the record marks for the user (a file in two libraries). The scout's side note names it correctly.
- Time note: the keys list header says Revision 1 added K-nocode, K-duplicate and the others; every key either scout used is in keys.md (checked against both files).

## Not checked

- The fake-using tests in test_front.py (KR3:83 to :90), test_frida_tools.py (KR3:80, :81) and test_frida_attach.py (KR3:41, :42) were not opened beyond the class names and the counter's reasons. I did not look for a second fake in them that the scout missed.
- test_play_chart.py:975 and :1544 (KR3:117, :121): I read the test bodies and helpers; I did not read the whole of `_FakeGame` and `_FakeSongClock`.
- The test for counts/T5-KR4.md:57, :59, :61, :62 (tests/single_owner/engine.cpp lines 86, 140, 253, 263, 308, 320, 327): I read only `listed_lines` at 327-337, which are the function's own lines, not the test case.
