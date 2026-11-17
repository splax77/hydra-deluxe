Agent id: ad5b4b415bbeed7a5 (task T8-E3-V-L-C02-L-C03, session f3bbb32b)

I opened the tests behind the rows where the counter's reason did not settle the key (test_engine.py, fakes.py, test_engine_finder.py, test_find_checksum.py, test_hit_window_scripts.py, test_input_driver.py, test_ocr.py, test_library_query.cpp, test_long_paths.cpp, test_method_watch.py, test_mono_facts.py, test_quiet.py, test_recorder_link.py, test_server.py) and record D10.md for the row at counts/T5-IR11.md:62. I did not open test_app_reports.cpp or test_app_shell.cpp; those L-C03 rows stand on the counter's reason.

## Counts

- L-C02: 91 rows in the batch, 91 lines in the label file, 77 rows I agree with, 14 rows I change.
- L-C03: 66 rows in the batch, 66 lines in the label file, 62 rows I agree with, 4 rows I change.

## The fake debugger and fake process (the lead's question)

A fake debugger is fake code, and keys.md has no key named for it. C-rig is the nearest, so I keep C-rig on the two rows whose counter reason names the FakeDebugger (counts/T5-IR11.md:20 and :21) and add it to :19, whose reason also names it.

A fake "Process" is different. In `tools/ch_probe/tests/fakes.py` (lines 1-6 and 36-83) `fake_process` builds a real `process.Process` over a reader that answers from typed memory. Nothing in the code under test is faked there. What is faked is the game's memory, so the key is O-gamemem. It is not O-process either, because O-process is a list of running processes, and this is not one. The scout's C-rig on rows :33, :35, :37, :38 and :51 therefore does not fit. I replaced it with O-gamemem where the row lacked it.

In L-C03, a typed process list (rows 40 to 44 and 49 to 51 in test_quiet.py) is O-process, as the scout had it. A `FakePopen` pid (counts/T5-IR12.md:60) is a stub program's pid, not a list, so that row goes to C-stub.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C02 | counts/T5-IR11.md:19 | O-gamemem, E-echo | O-gamemem, E-echo, C-rig | The counter's reason names a FakeDebugger that hands the pointer back (test_engine.py:53-71), which is fake code and was left without a key. |
| L-C02 | counts/T5-IR11.md:20 | C-rig, E-echo | C-rig, K-selfmade | Nothing is typed in and read back; the expected address is `proc.resolve(...)`, the production call again (test_engine.py:121), which is a value checked against the function that made it. |
| L-C02 | counts/T5-IR11.md:31 | F-presslog, O-scorestats | F-presslog | The test passes plain score integers (100, 150) to `pressed_input_hit` (test_engine.py:259); no scorestats JSON is involved. |
| L-C02 | counts/T5-IR11.md:33 | C-rig | O-gamemem | The engine runs on `fakes.fake_process`, a real Process over empty fake memory (test_engine.py:380), so the stand-in is game memory, not a rig. |
| L-C02 | counts/T5-IR11.md:35 | C-rig, C-clock, O-gamemem | C-clock, O-gamemem | FakeProc is a real Process over fake memory (test_engine_finder.py:30-35), so C-rig does not fit; the fake sleep and typed clocks and decoys are still named. |
| L-C02 | counts/T5-IR11.md:37 | C-rig, C-clock | O-gamemem, C-clock | Same reason: the process is real over fake memory, and the typed addresses and clock lists are game memory. |
| L-C02 | counts/T5-IR11.md:38 | C-rig, K-unreachable | O-gamemem, K-unreachable | Same reason: two typed engine objects over a real Process on fake memory. |
| L-C02 | counts/T5-IR11.md:51 | C-rig, C-clock, O-gamemem, P-message, P-other | C-clock, O-gamemem, P-message, P-other | The Process is real over fake memory (test_find_checksum.py:303-306); the fake clock is `FakeTime`, and C-rig names nothing here. |
| L-C02 | counts/T5-IR11.md:52 | C-clock, P-other | O-gamemem, P-message | No clock is faked: `clock_state` is called with typed clock readings (test_find_checksum.py:324-329), and the "state words" are printed text, as in the neighbouring rows :53 and :54. |
| L-C02 | counts/T5-IR11.md:62 | F-watchlog, F-presslog, K-unreachable | F-watchlog, F-sample, Z-cite, K-unreachable | The counter marks X12 and the scout gave it no Z-cite (the premise comes from IN-12 and D10.md:557 names it); the typed items are `WW.Sample` objects and chart notes, not a press log. |
| L-C02 | counts/T5-IR11.md:63 | F-watchlog, P-message, P-other | F-sample, P-message, P-other | The typed 11 ms case is built from `WW.Sample` objects (test_hit_window_scripts.py:186-187), the same stand-in as rows :60 and :61. |
| L-C02 | counts/T5-IR11.md:67 | C-stub | no key | The code shows no recorder: `InputDriver()` is used as it is and 0x99 is a typed input (test_input_driver.py:68-73), so the counter's X9 reason does not match the test and no key fits. |
| L-C02 | counts/T5-IR11.md:72 | K-unreachable | K-unreachable, C-stub | The counter marks X9, and K-unreachable is outside the X9 family; the test replaces `send_key` with a recorder (test_input_driver.py:123). |
| L-C02 | counts/T5-IR11.md:76 | C-stub, E-echo | C-stub, K-selfmade | Nothing typed in comes back; the expected title is the code's own constant `C.WINDOW_TITLE` (test_input_driver.py:194), a value checked against what the code made. |
| L-C03 | counts/T5-IR12.md:15 | C-worker, P-other, P-message | C-worker, P-other | The test types a cap (120.0) and an agent file name (test_method_watch.py:202-203), and no message text is checked. |
| L-C03 | counts/T5-IR12.md:18 | C-rig, O-gamemem, P-message | C-rig, P-message | The test (test_method_watch.py:273-276) feeds no agent payload; only the fake game and fake Frida harness and a typed reply sentence are present. |
| L-C03 | counts/T5-IR12.md:53 | O-toolout, C-stub | O-toolout, C-stub, O-process | The hand-written JSON rows are a process list (Name, ProcessId, CommandLine; test_quiet.py:383-384), a process-list stand-in. |
| L-C03 | counts/T5-IR12.md:60 | O-process, P-message | C-stub, P-message | The fake pid 777 comes from `FakePopen` (test_recorder_link.py setUp), a stub program, not a process list. |

(L-C03 changes: rows :15, :18, :53 and :60. L-C02 changes: rows :19, :20, :31, :33, :35, :37, :38, :51, :52, :62, :63, :67, :72 and :76.)

## Checks that passed

- Every row of both batches has exactly one line in its label file (91 of 91 and 66 of 66). The scout's note that :81 and :86 are absent from L-C02 is right: they are not in the batch file either.
- Every fault mark on a count row in both batches has a key of its family after my changes, except counts/T5-IR11.md:67, which I mark `no key` for the reason above.
- The ends tables name rows that are in the batch. After my changes the following ends rows no longer carry the key named and need the main session to pick another row: L-C02 E-echo least-fitting :76, C-rig least-fitting :33, C-clock least-fitting :52, F-presslog least-fitting :62, F-watchlog least-fitting :63, O-scorestats least-fitting :31. In L-C03 the O-process least-fitting row :60 no longer carries O-process. Every other ends row in both files carries its key.

## Not checked

- L-C03 rows counts/T5-IR12.md:82 to :91 (test_app_reports.cpp) and :92 to :97 (test_app_shell.cpp): I read the counter's reason only; the keys fit it.
- L-C02 rows counts/T5-IR11.md:87 to :96, :99 to :109 (test_library_query.cpp) beyond the lines I read (the helpers at lines 20-60, the stars and squeeze cases around 327-382, and the rich-text cases around 150-180).
- The `run` column was not used for any row.

## Side notes

- keys.md has no key for a fake debugger or other fake program-under-test collaborator; C-rig is used for it in L-C02 rows :19 to :21 as the nearest key. If the main session wants a separate key, it is a revision keys.md does not allow after Revision 1.
- counts/T5-IR11.md:42 (test_find_checksum.py:124-128): the invalid hash strings are hand-typed malformed inputs, which keys.md question 1 calls input, yet the counter marks X9. The scout's O-scorestats is the only X9-family key near; I kept it but the row is a candidate for the same "is this a stand-in" question as rows :44 to :46.
- counts/T5-IR11.md:87 to :90 and :110, :111 (test_library_query.cpp:61-90, test_long_paths.cpp:92-99): the typed strings go straight into `fold_for_search`, `win32_path` and similar pure functions. F-row (a library row) is the nearest X9-family key but not a close fit; I kept the scout's key.
- counts/T5-IR11.md:40 (test_find_checksum.py:31-33): the comment says HASH equals the game's checksum in scorestats.json "(the spike plan, 'What the code does today')"; the counter says the hash is not in scorestats.json (F8). I checked only that the comment exists.
- counts/T5-IR11.md:62: D10.md:557 says IN-12 refutes the report's per-hit premise and D10.md:559 sends it to the user with F9. That is a split between the test's claim and the page, for the main session.
- counts/T5-IR11.md:73 (test_input_driver.py:137): the comment cites "Ruling 3 of the 2026-10-30 ch_mcp Star Power plan", a plan, not a filed ruling; the scout's note is right.
- counts/T5-IR11.md:101 (test_library_query.cpp:357-358): the comment says "The user's ruling" for D266. I did not look D266 up in `docs/agents/rulings/`.
- tools/ch_probe/tests/test_input_driver.py:140-146: `test_press_key_sends_down_hold_then_up` is not in either batch and has no row; not a finding.
- keys.md line 131 still says "The first list held 48 keys" while Revision 1 brings it to 53 (the L-C03 scout's note is right); it is explained by the text, so only a wording point.
