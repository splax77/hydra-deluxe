Agent id: a165a668e52a29364 (task T8-E3-V-L-C15-L-C16, session f3bbb32b)

I opened the six test files of L-C15 (`tools/ch_mcp/tests/test_sp_tools.py`, `test_trace_hits.py`, `tools/ch_probe/tests/test_cheap_checks.py`, `test_debugger.py`, `test_early_fill_walk.py`, `test_edge_probe.py`) and the three of L-C16 (`tools/ch_mcp/tests/test_machine_recorder.py`, `test_machine_report.py`, `test_menu_tools.py`). I read the test bodies for the rows named below. I did not label from the `run` column. Row ids are the counter's ids, `counts/T5-IR5.md:N` for L-C15 and `counts/T5-IR6.md:N` for L-C16.

## Counts

- L-C15: 89 rows in the batch, 89 lines in the label file (every batch row has exactly one line, checked by comparing the two row lists), 76 rows I agree with, 13 rows I change.
- L-C16: 108 rows in the batch, 108 lines in the label file (same check), 102 rows I agree with, 6 rows I change.

## Rows I change

| batch | row | scout's keys | your keys | why, one sentence |
|---|---|---|---|---|
| L-C15 | counts/T5-IR5.md:14 | C-session, C-stub, O-process, E-echo | C-session, C-stub, E-echo | FakeProc in `test_sp_tools.py:122` is a fake game connection with `verify_targets` and `close`, not a process list. |
| L-C15 | counts/T5-IR5.md:45 | F-sample, C-session, P-message | C-session, P-message | `test_sp_tools.py:660` calls `status_over([])` with no samples at all, so no hand-typed sample exists. |
| L-C15 | counts/T5-IR5.md:49 | F-record, P-message | F-summary, P-message | The "play-session dict" (`last_row`, `press_count`, `stop`, `message`) is what a play run reports, not a record written to the store and read back. |
| L-C15 | counts/T5-IR5.md:50 | F-watchlog, F-record, P-message | F-watchlog, F-summary, P-message | Same play dict as row 49; the watch rows stay F-watchlog. |
| L-C15 | counts/T5-IR5.md:51 | F-watchlog, F-record, P-message, E-echo | F-watchlog, F-summary, P-message, E-echo | Same play dict as row 49. |
| L-C15 | counts/T5-IR5.md:56 | F-presslog | O-gamemem | The hand-built `call(...)` dicts are the game's traced method calls (the keys list "the game's method calls" under O-gamemem), not a log of presses. |
| L-C15 | counts/T5-IR5.md:57 | F-presslog | O-gamemem | Same hand-built traced calls as row 56. |
| L-C15 | counts/T5-IR5.md:58 | F-presslog, P-other | O-gamemem, P-other | Same traced calls; the typed 915/1085/85.0 stay P-other. |
| L-C15 | counts/T5-IR5.md:63 | O-gamemem, C-session, F-presslog, P-message | O-gamemem, C-session, P-message | `test_trace_hits.py:240` feeds a fake session and hand-built traced calls; no press log is built. |
| L-C15 | counts/T5-IR5.md:71 | O-scorestats, E-echo | F-schedule, E-echo | The hand-written JSON at `test_trace_hits.py:378` is a `hydra_replay score` file (`chords` with `index` and `ms`; `notes_from_score` docstring says so), Hydra's own output, not the game's scorestats. |
| L-C15 | counts/T5-IR5.md:81 | O-process, O-gamemem, C-clock, P-message, E-echo | O-gamemem, C-clock, C-stub, P-message, E-echo | `test_cheap_checks.py:225` builds a real `Process` over fake memory (`fakes.fake_process`), not a process list, and also passes a fake opener and a fake `scan` lambda (C-stub). |
| L-C15 | counts/T5-IR5.md:99 | C-stub, O-gamemem | C-stub, O-gamemem, C-reader | The reason and the test (`test_debugger.py:272-285`) replace `process.make_reader` and `make_writer` with hand-written ones, which is a fake reader. |
| L-C15 | counts/T5-IR5.md:111 | F-row, P-message | F-presslog, P-message | The hand-typed rows at `test_edge_probe.py:166` are press rows (`clock_ms_at_press`, `score_before`, `send`), not Library or report rows. |
| L-C16 | counts/T5-IR6.md:53 | C-reader, E-echo | C-stub, E-echo | The `lambda *args: 0xC0000022` replaces the Windows `NtQuerySystemInformation` call, a fake of a Windows API, which L-C15 keyed C-stub (rows 97 to 99), not a reader the app owns. |
| L-C16 | counts/T5-IR6.md:54 | C-reader | C-stub | Same: `query` and the patched `ctypes` calls stand for the Windows API. |
| L-C16 | counts/T5-IR6.md:90 | F-sample | F-sample, Z-cite | The counter's reason says the stall data is made up "as in LS-O2", and the class docstring at `test_machine_report.py:397-398` names that run as the source of the numbers. |
| L-C16 | counts/T5-IR6.md:125 | C-rig, O-gamemem, C-clock | O-gamemem | `test_menu_tools.py:345` is a single `MT.online_refusal(state(...))` call: no FakeGame, FakeKit, FakeFrida or FakeClock is built. |
| L-C16 | counts/T5-IR6.md:126 | C-rig, O-gamemem, C-clock, P-message | O-gamemem, P-message | `test_menu_tools.py:348` also calls `MT.online_refusal` on hand-built dicts and builds no fake; the four typed sentences stay P-message. |
| L-C16 | counts/T5-IR6.md:136 | P-other, Z-cite | P-onefile, Z-cite | The typed 10, 10, 0.5 are pinned in one test file only (`MENU_STEP_CAP_S`, `SEARCH_DOWN_CAP`, `MENU_POLL_S` appear in no other test), the same shape the scout keyed P-onefile at counts/T5-IR6.md:55. |

## Ends tables after my changes

- L-C15: these keys no longer sit on any row of the batch, so their ends lines should go: O-process (was 14 / 81), F-record (was 49 / 51), F-row (was 111 / 111), O-scorestats (was 71 / 71). F-presslog is now carried only by counts/T5-IR5.md:111, so its ends are 111 / 111. New keys: F-summary (plainest counts/T5-IR5.md:49, least fitting counts/T5-IR5.md:51) and C-reader (counts/T5-IR5.md:99 for both). F-schedule gains counts/T5-IR5.md:71. All other ends rows exist in the batch and carry their key.
- L-C16: C-reader is no longer carried by counts/T5-IR6.md:54, so its plainest row becomes counts/T5-IR6.md:48 (least fitting stays counts/T5-IR6.md:57). New key C-stub: plainest counts/T5-IR6.md:54, least fitting counts/T5-IR6.md:53. P-onefile gains counts/T5-IR6.md:136. All other ends rows exist in the batch and carry their key (C-rig 116 and 52, E-echo 112 and 53, and so on).

## Side notes

- `tools/ch_mcp/tests/test_sp_tools.py:176-182`: every `self.tools()` builds a FakeGame, a FakeProc, a fake opener, a fake clock (`now=lambda: 1000.0`) and a `Script` reader. Rows such as counts/T5-IR5.md:20 (line 302, `ch_sp` with no watch) and 48 to 53 (the compare tests) go through that fake, but the counter marked only X10 or hand-typed rows. The labels follow the counter's faults, so those rows carry no C key.
- `test_sp_tools.py:133-152`: `Script.reader` is a fake reader. L-C15 keyed the scripted snapshots O-gamemem and never used C-reader (rows 13, 16, 23, 24, 29, 34 to 41 use it; row 16's own reason says "scripted reader"). I left those rows alone because O-gamemem covers the fault, but the main session should decide whether a scripted reader also takes C-reader.
- `test_machine_recorder.py:422-464`: `FakeLoop` is a harness with a fake clock, reader and table, like the `MenuToolsCase` that L-C16 keyed C-rig. L-C16 gave C-rig to the `FakeFile` row (counts/T5-IR6.md:52) and the menu rows, but not to FakeLoop rows 40 to 51. Same decision as the note above.
- `test_machine_recorder.py:208-212` (counts/T5-IR6.md:31): the expected side is built by the production owner's `M._layout` (`layout_of`, line 132), a value made by production code. This is K-selfmade in the key list, a Rewrite and Remove kind, so it carries no key on a count row, but it is a second fault on this row.
- `test_machine_recorder.py:264-271` (`test_no_test_builds_bytes_from_the_parser_offsets`) reads the test file's own source text. This is K-nocode, outside this batch (the L-C16 scout noted it too).
- `test_menu_tools.py:233-241` (counts/T5-IR6.md:118): the comment cites "D318 item 4" as the source of the refusal rule, and `test_menu_tools.py:471` cites "ruling 3". The counter marked X12 only on the second.
- `test_menu_tools.py:491-500` cites two wiki pages (the test only checks that repo files exist). Outside this batch; the L-C16 scout noted it.
- `test_machine_recorder.py:743-745` (FakeFile, counts/T5-IR6.md:52): no C key names a fake file; C-rig is kept as the nearest. The key list has no exact fit.
- Split to watch: a fake Windows API (FakeK32 and `query` lambdas) was keyed C-stub in L-C15 and C-reader in L-C16. If a later batch keys it a third way, the main session should fix one key for it before the questions are built. I changed L-C16 to C-stub to match L-C15.
- `test_debugger.py:306` and `:314` (counts/T5-IR5.md:100 and 101): the hand-built `DEBUG_EVENT` is a Windows debug message, not game memory. I left O-gamemem on them because no other key fits better; the key list has no exact fit for a Windows event.

## Not checked

I did not open the record files (`records/D9.md`, `records/D7.md`); every row here is a `count` row and the counter's reason plus the test were enough. I did not read test bodies for these rows beyond the counter's reason: counts/T5-IR5.md:104 to 106 (`test_early_fill_walk.py:89, 113, 121`, read only in part), counts/T5-IR6.md:59 to 89 and 91 to 114 (the `Fixture` rows in `test_machine_report.py`; I read the Fixture class and a sample of the tests, and every one goes through `Fixture`), and counts/T5-IR6.md:115 to 124 and 127 to 135 beyond what is quoted above (read, but agreed from the class setup).
