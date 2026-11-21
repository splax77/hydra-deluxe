Agent id: a2612df416b369e78 (task T8-E3-V-L-C19-L-C20, session f3bbb32b)

Method: for each row I opened the row's own test (at the line in the batch's `test_or_title`, in the repo, or in `C:\Users\Patrick\.claude\gates\tests\` for the two gate files) and read which fixtures it uses: fakes, mocks, stubs, hand-built inputs, helper builders. I then compared with the scout's keys. `MW`, `MF`, `PS`, `Q`, `RL` and `SV` in the L-C20 batch are `test_method_watch.py`, `test_mono_facts.py`, `test_press_schedule.py`, `test_quiet.py`, `test_recorder_link.py` and `test_server.py` under `tools/ch_mcp/tests/`.

## Counts

- L-C19: rows in the batch 82, lines in the label file 82 (each row exactly once, no duplicates), rows I agree with 52, rows I change 30.
- L-C20: rows in the batch 122, lines in the label file 122 (each row exactly once, no duplicates), rows I agree with 114, rows I change 8.

The ends tables: every row they name is in its batch and carries the key. One cell goes stale after my changes (L-C19, F-schedule, least-fitting row `counts/T5-IR9.md:75`; see the table, that row loses F-schedule). L-C20's ends tables stay true after my changes.

## Rows I change

Two kinds of error showed up. In the first kind, the test uses a fixture and no key names it. In `test_sp_compare.py` almost every `run(...)`, `_main(...)` and `_last_chord_check(...)` row feeds a hand-typed `SCORE` (chords and activations) and a schedule the file itself calls "hand-built" (`_write_schedule`, `_schedule_of`), and the scout keyed only the hand-built watcher rows. In the second kind, a key names a fixture the test never touches (`:57` has no FakeClock, `mono_facts :155` never calls `fake_runner`, `:74` and `:75` read a saved real schedule).

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C19 | counts/T5-IR9.md:9 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | `run(...)` feeds the hand-typed `SCORE` chords and activations and a schedule written by `_write_schedule` (test_sp_compare.py:46-54, 109-121, 123-128). |
| L-C19 | counts/T5-IR9.md:10 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same: the test goes through `run(...)` with `SCORE` and the hand-built schedule. |
| L-C19 | counts/T5-IR9.md:11 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:12 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:13 | F-watchlog, P-other, Z-cite | F-watchlog, F-result, F-schedule, P-other, Z-cite | Same. |
| L-C19 | counts/T5-IR9.md:14 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:15 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:16 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:17 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:18 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | The test uses `SCORE` and `_score_reach()` (the hand-built schedule). |
| L-C19 | counts/T5-IR9.md:19 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same as :9, through `run(...)`. |
| L-C19 | counts/T5-IR9.md:20 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:21 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:22 | F-watchlog, Z-cite, S-snapshot, P-other | F-watchlog, F-result, F-schedule, Z-cite, S-snapshot, P-other | The test builds a hand-copied chord list (`LS_M2_CHORDS`, `_chord(...)` lines 294-299) and calls `_schedule_of`, so it has a typed chord list and a hand-built schedule. |
| L-C19 | counts/T5-IR9.md:26 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | The test uses `run(...)` with `SCORE` (test_sp_compare.py:397). |
| L-C19 | counts/T5-IR9.md:27 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same, through `run(...)`. |
| L-C19 | counts/T5-IR9.md:28 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:29 | F-watchlog, P-message | F-watchlog, F-result, P-message | `_last_chord_check` builds three typed chords with `_chord(...)` (test_sp_compare.py:465-466) and the scout keyed only the rows. |
| L-C19 | counts/T5-IR9.md:30 | F-watchlog, P-message | F-watchlog, F-result, P-message | Same helper, same typed chords. |
| L-C19 | counts/T5-IR9.md:31 | F-watchlog, P-message | F-watchlog, F-result, P-message | Same helper, same typed chords. |
| L-C19 | counts/T5-IR9.md:35 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | `_write` dumps `SCORE` to score.json and `_main` adds `_write_schedule` (test_sp_compare.py:545-562). |
| L-C19 | counts/T5-IR9.md:36 | F-watchlog | F-watchlog, F-result, F-schedule | Same `_write` and `_main` helpers. |
| L-C19 | counts/T5-IR9.md:37 | F-watchlog, P-message | F-watchlog, F-result, F-schedule, P-message | Same. |
| L-C19 | counts/T5-IR9.md:38 | F-watchlog, P-message, P-other | F-watchlog, F-result, F-schedule, P-message, P-other | Same. |
| L-C19 | counts/T5-IR9.md:40 | F-watchlog, F-presslog | F-watchlog, F-result, F-presslog | The test types two chord dicts (index, tick, ms) by hand (test_sp_compare.py:658-661). |
| L-C19 | counts/T5-IR9.md:46 | F-schedule | F-schedule, F-result | The test also uses `SCORE`'s typed chords next to the hand-built schedule. |
| L-C19 | counts/T5-IR9.md:57 | O-gamemem, C-clock, Z-cite, P-other | O-gamemem, Z-cite, P-other | The test at test_sp_watch.py:199 times `snapshot()` with the real `time.perf_counter` and never uses `FakeClock`. |
| L-C19 | counts/T5-IR9.md:74 | C-stub, F-schedule, Z-cite | C-stub, Z-cite | `_dense_cut()` cuts chords from a saved real LS-P3 schedule (`load_schedule`, test_stall_harness.py:281-287), which is a saved run, not a hand-built schedule. |
| L-C19 | counts/T5-IR9.md:75 | C-stub, F-schedule, Z-cite | C-stub, Z-cite | Same `_dense_cut()` from the saved run, nothing hand-built. |
| L-C19 | counts/T5-IR9.md:91 | no key | F-row, C-stub | The test hand-sets a made-up md5 on a library entry (`entry.md5 = "0123..."`), and F-row names hand-built library entries; it also installs `no_device_factory()`, the same fake audio-device factory the scout keyed C-stub on row :88. |
| L-C20 | counts/T5-KR1.md:29 | O-toolout, P-message, C-stub | O-toolout, P-message | The test at test_mono_facts.py:155 deep-copies `READER_OUTPUT` and never calls `fake_runner` (that is used at :176 and :185); the C-stub key sits on a fixture this test does not touch. |
| L-C20 | counts/T5-KR1.md:33 | O-gamemem | O-gamemem, F-schedule | The test feeds `run(play([entry(...)]), ...)`, a hand-built play message of schedule entries (test_press_schedule.py:61-91), as well as the made-up score. |
| L-C20 | counts/T5-KR1.md:63 | P-message, C-reader | P-message, C-reader, C-clock | `ScanTest.scan` passes `ClockFake()` by default (test_quiet.py:268), and this test goes through `self.scan`. |
| L-C20 | counts/T5-KR2.md:49 | E-echo | E-echo, F-presslog | The test types a press-log row by hand, writes it with `write_press_log` and reads it back; :46 and :51 got F-presslog for the same shape. |
| L-C20 | counts/T5-KR2.md:50 | E-echo | E-echo, F-presslog | Same: a hand-typed press row with "retime_missed" is written and read back. |
| L-C20 | counts/T5-KR2.md:58 | O-payload, E-echo, P-message | O-payload, E-echo, P-message, C-stub | `run_hook(lambda d: common.deny("no, because"), ...)` puts a hand-written lambda in place of the real hook body (test_refusals.py:76). |
| L-C20 | counts/T5-KR2.md:103 | E-echo | E-echo, C-stub | The test at test_machine_recorder.py:822 runs through `CommandLine.run_main`, which patches `M.record`, `open_parent_wait`, `sys.stdin`, `os.fdopen` and `print` with mocks (lines 806-813). C-stub is the closest key. |
| L-C20 | counts/T5-KR2.md:118 | C-stub, P-message | C-rig, P-message | The test builds FakeGame, FakeKit and FakeFrida, which the scout keyed C-rig on :111 and :112; the same fixtures should carry the same key. |

The scout's question for KR2 :100, :103 to :106: `:100` (line 638) is `test_the_first_end_to_arrive_is_kept`, which uses only `M.Stop()`, so no FakeLoop and no C key is right. `:104`, `:105` and `:106` (lines 896, 903, 915) are in `RealRun`, which starts the real recorder in a real subprocess, so no FakeLoop there either. `:103` is not FakeLoop but a mock-patched `main` (in the table above).

The scout's question for the `test_ref_judge.py` rows: the docstring is true for all of them. Every class in the file derives from `Fixture` (TrailerTests, RewriteTests, ..., SelftestAndInstallerTests, RulingGateTests, RefusalLogTests), and the gate files in the temp folder get a fake `review_rules.py`. I agree with C-gate on every ref_judge row. For the `test_server.py` and `test_recorder_link.py` rows, C-stub is right: `ServerTest.setUp` builds `Helper(... game_kw={"driver": object()})` and `FakePopen` and `FakeProcess` are built in the recorder-link tests' setUp.

## Side notes

- L-C19 `counts/T5-IR9.md:87` (test_preview_controller.cpp:338): the scout's side note says the line is stale and the test is at 343. It is not. `TEST_CASE("track_options: one builder, compared whole")` is at line 338. The scout's note is wrong.
- L-C19 `counts/T5-IR9.md:90` (test_preview_controller.cpp:441): the scout's side note says line 441 is the `preview_config` logic_error test and the sentence is at 446. It is the other way round. `preview_config: asked before the first render` starts at 434, and line 441 is `a Preview asset failure reads Reinstall Hydra, with the raw text as its detail`, whose `CHECK(pc.error() == "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them.")` is the typed sentence. P-message is right, and the counter's line is right.
- L-C19 `counts/T5-IR9.md:92` (test_preview_golden.cpp:258): I agree with `no key`. The hand-built per-pixel delta map is made up, and its comment says so. Nearest key is F-result, but a delta map is not one of that key's kinds (activations, squeezes, paths, scores). It is a candidate for the key list.
- L-C19 `counts/T5-IR9.md:88` and `:89` (test_preview_controller.cpp:379, 403): C-stub is the closest key for a fake audio-device factory and for the headless and start-fails switches, but C-stub means a stub program. The factory is a fake of production code, which no key names. I left the scout's keys but a fake-device key may be wanted. `:89` uses production-owned test switches (`set_headless`, `set_start_fails`), not a fake class, so C-stub is the weakest fit there.
- L-C19 `tools/ch_probe/tests/test_sp_compare.py:409` and `:426`: I agree with the scout, these read the tool's own source text for a name and look like K-nocode. They are outside both batches.
- L-C20 `counts/T5-KR1.md:87` to `:89` and `:98` to `:109` (test_server.py and test_recorder_link.py): C-stub is used for a Helper on `object()` driver and for FakePopen; neither is a stub program, as the key says. The same pattern as the L-C19 factory note, so one more fake-code key may be wanted.
- L-C20 `counts/T5-KR2.md:87` to `:89` (test_ref_judge.py:795, 806, 814): `RulingGateTests.setUp` writes a fake `review_rules.py` and a made-up transcript folder (`_typed`, `_answer`, test_ref_judge.py:683-745). Those three tests use neither helper in their own body, so I kept C-gate only and added no O-transcript key.
- The shared scratch folder's `scan.py` was overwritten by another agent while I worked. I did not use it after that and changed nothing there.

## Not checked

- L-C20 rows `counts/T5-KR1.md:45` to `:58` and `:70` beyond the `FakeReaders` check (I read the class layout of test_quiet.py but not each body).
- L-C20 `counts/T5-KR2.md:31` to `:40` (test_process.py): I read lines 130-185 and the fake-process helper use, not each of the :31 to :35 bodies.
- L-C20 `counts/T5-KR2.md:96` and `:98` (test_machine_recorder.py:161, 252): I read the class comment only ("the parser on bytes Windows wrote"), not the full bodies.
- L-C20 `counts/T5-KR2.md:108` to `:110` (test_machine_report.py): I read the class list only.
- L-C19 `counts/T5-IR9.md:33`, `:34`, `:39`, `:41` to `:45`, `:62` to `:72` (hand-written CSV and watcher rows): I read them through the pattern scan and the class layout, not each body.
- L-C19 `counts/T5-IR9.md:78` to `:82` and `:84`, `:85` (test_agent_cost.py, test_path_list.cpp, test_precheck.cpp): I did not open these bodies; they carry the counter's reason keys only.
