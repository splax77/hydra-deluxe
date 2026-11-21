Agent id: afc202724fa32e933 (task T8-E3-V-L-C12-L-C13, session f3bbb32b)

## Counts

- L-C12: 101 rows in the batch, 101 lines in the label file, 85 rows I agree with, 16 rows I change.
- L-C13: 99 rows in the batch, 99 lines in the label file, 50 rows I agree with, 49 rows I change.

Every batch row has exactly one line in its label file, in both batches. I opened the tests behind most rows and the fixtures every test in a class builds. Where a "by title" or ":NNN" row names a region, I matched it to the test that starts there.

## Rows I change

For L-C12, F-row became C-ownchecker on the scan rows because the thing under test is a checker that lives in the test tree (see Side notes). For L-C13, a hand-cut `dump.cs` fragment, hand page or hand constants file that the real dump code reads is input, so the O-gamemem key goes. Where the row then holds no other stand-in, it is `no key`. Where the row also carries an X10 or X11 key, the X9 mark has nothing of its family left, and I say so.

| batch | row | scout's keys | your keys | why |
|---|---|---|---|---|
| L-C12 | counts/T5-IR20.md:13 | F-row, P-message, P-onefile | C-ownchecker, P-message, P-onefile | The "made-up entry" is fed to `parse_dates`, a checker defined in `tests/single_owner/engine.cpp`; it is not a Library or report row. |
| L-C12 | counts/T5-IR20.md:14 | F-row, P-message | C-ownchecker, P-message | The hand-built KnownClone goes to `baseline_due`, the same test-tree checker. |
| L-C12 | counts/T5-IR20.md:16 | F-row, P-message | C-ownchecker, P-message | `kMadeUpFile` and the made-up row go to `scan_file`, the test-tree scanner. |
| L-C12 | counts/T5-IR20.md:20 | F-row | C-ownchecker | The hand-built listed lines go to `take_listed_line`, a test-tree function. |
| L-C12 | counts/T5-IR20.md:21 | F-row, P-message | C-ownchecker, P-message | Made-up row and file go to the same test-tree scanner. |
| L-C12 | counts/T5-IR20.md:22 | F-row | C-ownchecker | Made-up row and file go to `scan_file`. |
| L-C12 | counts/T5-IR20.md:24 | F-row, P-message | C-ownchecker, P-message | Made-up row and hand-written ps1/yml go to the test-tree scanner; the files themselves are input. |
| L-C12 | counts/T5-IR20.md:25 | F-row, P-message | C-ownchecker, P-message | Made-up row goes to the test-tree scanner. |
| L-C12 | counts/T5-IR20.md:26 | P-message | C-ownchecker, P-message | The counter marks no X9, but the rule files are loaded by the same test-tree checker, so the fixture is the same one. |
| L-C12 | counts/T5-IR20.md:27 | F-row, P-message, P-onefile | C-ownchecker, P-message, P-onefile | Made-up entry goes to `parse_dates`. |
| L-C12 | counts/T5-IR20.md:28 | F-row, P-message | C-ownchecker, P-message | Made-up OwnerRule goes to `anchor_misses`, a test-tree function. |
| L-C12 | counts/T5-IR20.md:31 | F-result | F-record | `save_batch_record` writes an empty analysis record to the store; the row is a stored record, not a result. |
| L-C12 | counts/T5-IR20.md:32 | F-row, Z-cite | F-row, F-record, Z-cite | `save_scored` saves a hand-built `scored_record` (one typed `notes_points`) through `prepare_record`. |
| L-C12 | counts/T5-IR21.md:16 | P-setting | P-message | The typed `view_prodrums = "yes"` is a detail inside the settings-error message, not a setting default. |
| L-C12 | counts/T5-IR21.md:54 | S-snapshot, P-burnout | S-snapshot, P-other | The test runs on Tapped Out (`tapped_out()`), not Burnout; the key is Burnout only, so the typed figures are the generic constant pin. |
| L-C12 | counts/T5-IR21.md:81 | P-other | P-other, C-analyzer | `test_preview_controls` installs `ViewGate`, which sets `set_view_analyzer_for_test`; the key lists that seam (it hands back the real analysis after a hold). |
| L-C13 | counts/T5-IR3.md:15 | P-other, C-reader | P-other, C-reader, C-clock | `_watch` builds a fake clock (`lambda: float(next(clock))`) and a no-op sleep for every case that uses it. |
| L-C13 | counts/T5-IR3.md:17 | C-reader | C-reader, C-clock | Built with `_watch`: fake clock. |
| L-C13 | counts/T5-IR3.md:18 | C-reader | C-reader, C-clock | Built with `_watch`: fake clock. |
| L-C13 | counts/T5-IR3.md:19 | C-reader | C-reader, C-clock | Built with `_watch`: fake clock. |
| L-C13 | counts/T5-IR3.md:20 | C-reader | C-reader, C-clock | Built with `_watch`: fake clock; the test reads `w.events` stamped with it. |
| L-C13 | counts/T5-IR3.md:23 | C-reader, P-message | C-reader, C-clock, P-message | Built with `_watch`: fake clock. |
| L-C13 | counts/T5-IR3.md:25 | O-gamemem | no key | Hand-cut `dump.cs` fragment read by the real parser is input (keys.md, question 1); nothing else here is a stand-in. |
| L-C13 | counts/T5-IR3.md:26 | O-gamemem | no key | Same: hand-cut dump fragment is input. |
| L-C13 | counts/T5-IR3.md:27 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:28 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:29 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:30 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:31 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:32 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:33 | O-gamemem | no key | A hand-written class line read by the real parser is input. |
| L-C13 | counts/T5-IR3.md:34 | O-gamemem | no key | Hand-cut fragment is input. |
| L-C13 | counts/T5-IR3.md:35 | O-gamemem, P-message | P-message | The counter marks X10 only; a hand-written broken dump is input. |
| L-C13 | counts/T5-IR3.md:36 | O-gamemem, E-echo | E-echo | Hand-cut fragment is input; the typed ctor RVA and version are the echo. The X9 mark has no key of its family. |
| L-C13 | counts/T5-IR3.md:37 | O-gamemem, P-message | P-message | Hand-cut fragment is input; the typed message is the pin. The X9 mark has no key of its family. |
| L-C13 | counts/T5-IR3.md:38 | O-gamemem, P-message | P-message | Same: hand-written second class is input. |
| L-C13 | counts/T5-IR3.md:39 | O-gamemem | no key | Hand-cut fragment is input. |
| L-C13 | counts/T5-IR3.md:40 | O-gamemem, P-name | P-name | Hand-written dump file is input; the typed file name stays as P-name. The X9 mark has no key of its family. |
| L-C13 | counts/T5-IR3.md:44 | O-gamemem, P-message | P-message | Tiny hand dump is input; the typed column line stays. The X9 mark has no key of its family. |
| L-C13 | counts/T5-IR3.md:45 | O-gamemem | no key | Tiny hand dump with a hand page is input. |
| L-C13 | counts/T5-IR3.md:46 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:47 | O-gamemem, P-message | P-message | Hand page is input; the typed error shape stays. The X9 mark has no key of its family. |
| L-C13 | counts/T5-IR3.md:48 | O-gamemem | no key | Tiny hand dump and page are input. |
| L-C13 | counts/T5-IR3.md:49 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:50 | O-gamemem | no key | A hand constants file the real reader reads is input. |
| L-C13 | counts/T5-IR3.md:51 | O-gamemem | no key | Tiny hand dump is input. |
| L-C13 | counts/T5-IR3.md:52 | O-gamemem | no key | Same. |
| L-C13 | counts/T5-IR3.md:53 | O-gamemem | no key | `test_python_name_and_file_names_are_not_game_facts` passes typed names to `D.game_fact`; no dump, game memory or message is faked. |
| L-C13 | counts/T5-IR3.md:54 | F-result, O-gamemem | F-result | The hand-built facts dict is the stand-in (it is written as `facts.json` and read back); the hand page is input. |
| L-C13 | counts/T5-IR3.md:57 | O-gamemem | no key | Tiny hand dump, hand page and hand-edited table are input. |
| L-C13 | counts/T5-IR3.md:58 | O-gamemem | no key | Hand page is input. |
| L-C13 | counts/T5-IR3.md:62 | O-gamemem | no key | Map dump and hand page are input. |
| L-C13 | counts/T5-IR3.md:67 | O-gamemem, P-message | P-message | Hand map is input; the typed message stays. The X9 mark has no key of its family. |
| L-C13 | counts/T5-IR3.md:73 | O-gamemem, E-echo | C-session, O-gamemem, E-echo | The test builds `FakeSession([sent({"error": ...}), ...])` and `FakeFrida`. |
| L-C13 | counts/T5-IR3.md:74 | O-gamemem, E-echo | C-session, O-gamemem, E-echo | `FakeSession` carries the hand error message. |
| L-C13 | counts/T5-IR3.md:87 | O-gamemem, E-echo | C-session, O-gamemem, E-echo | `self.started(...)` builds a `ReplySession` under `FakeFrida`. |
| L-C13 | counts/T5-IR3.md:88 | O-gamemem, E-echo | C-session, O-gamemem, E-echo | Same fixture. |
| L-C13 | counts/T5-IR3.md:90 | O-gamemem | C-session, O-gamemem | Same fixture. |
| L-C13 | counts/T5-IR3.md:91 | O-gamemem, E-echo | C-session, O-gamemem, E-echo | Same fixture. |
| L-C13 | counts/T5-IR3.md:97 | O-gamemem, P-message, E-echo | C-session, O-gamemem, P-message, E-echo | Same fixture (`started` with an error reply). |
| L-C13 | counts/T5-IR3.md:98 | O-gamemem, E-echo | C-session, O-gamemem, E-echo | Same fixture. |
| L-C13 | counts/T5-IR3.md:100 | O-gamemem, P-message, E-echo | C-session, O-gamemem, P-message, E-echo | Same fixture. |
| L-C13 | counts/T5-IR3.md:114 | P-other, K-nocode | P-other, K-nocode, C-ownchecker | The test calls `play_waits_findings`, defined at line 1201 of the same test file, on the real `kit.ts` text. |
| L-C13 | counts/T5-IR3.md:115 | P-other, K-nocode | P-other, K-nocode, C-ownchecker | The test calls `stop_path_findings`, defined at line 1220 of the same test file. |
| L-C13 | counts/T5-IR3.md:117 | P-other, K-nocode | P-other, K-nocode, C-ownchecker | The test calls `patch_findings`, defined at line 1352 of the same test file. |

(That is 16 changed rows in L-C12 and 49 in L-C13; the 49 are rows 15, 17, 18, 19, 20, 23, 25 to 40, 44 to 54, 57, 58, 62, 67, 73, 74, 87, 88, 90, 91, 97, 98, 100, 114, 115 and 117 of counts/T5-IR3.md.)

Doubts the main session asked about, answered:

- `T5-IR21.md:54` P-burnout: wrong. The test is `tapped_out()`; P-burnout is Burnout only. I changed it to P-other.
- `T5-IR21.md:77` P-tiers: right. `test_squeeze_rating.cpp:939` checks the words "Extreme", "Hard" and "Hard (uncounted)", which are tier words. Kept.
- `T5-IR21.md:86`, `:88`, `:90`: kept. They type box text ("x4 . combo 13", "full meter ", a header) and a 0.5 ms edge, which are screen words and one other constant.
- L-C13 `dump.cs` rows: changed as above, per your brief.
- L-C13 scripted foreground-window handles as C-reader: kept. `Script` stands in for `foreground_window()`, the reader of the front window. Nothing closer in `keys.md`.

## Ends tables

L-C12: P-burnout has no row any more (its one row moved to P-other). C-ownchecker is new in this batch: plainest `counts/T5-IR20.md:22`, least fitting `counts/T5-IR20.md:26`. F-row's least-fitting row `counts/T5-IR20.md:20` no longer carries F-row; use `counts/T5-IR20.md:33` instead. C-analyzer: only `counts/T5-IR21.md:81`. All other ends in the scout's table name rows that carry the key.

L-C13: O-gamemem's least-fitting row `counts/T5-IR3.md:33` is now `no key`; use `counts/T5-IR3.md:68` instead. C-session's plainest row stays `counts/T5-IR3.md:71`; its least-fitting row stays `counts/T5-IR3.md:110`. C-ownchecker: plainest `counts/T5-IR3.md:115`, least fitting `counts/T5-IR3.md:117`. C-clock: plainest `counts/T5-IR3.md:21`, least fitting `counts/T5-IR3.md:15`. All other ends name rows that carry the key.

## Side notes

- `tests/single_owner/scan.cpp` (lines 156 to 625) tests `take_listed_line`, `scan_file`, `parse_dates`, `anchor_misses` and the rest, all defined in `tests/single_owner/engine.cpp` and used by `hydra_precheck_lib` (`CMakeLists.txt:743`, `:926`). The checker is in the test tree, not in the test file itself, so C-ownchecker (question 2: "defined in its own test file") is the nearest key and not an exact fit. The main session should decide whether it counts.
- `tools/ch_mcp/tests/test_frida_attach.py` lines 46 to 150 and 1174 to 1360 define six checkers (`call_findings`, `kit_methods_findings`, `own_number_findings`, `play_waits_findings`, `stop_path_findings`, `patch_findings`) that scan the real `kit.ts` and `launch.ts`. Rows `T5-IR3.md:114`, `:115` and `:117` use them. Other rows in the same region of that file (not in these batches) probably do too.
- `tests/ui/uitest_harness.cpp:201` and `:210`: `reset_app` installs a canned fetcher and a throwing downloader for every GUI test, so every uitest row in L-C12 (`T5-IR21.md:10` to `:23`, `:53`, `:81`, `:83`) runs with a faked network. Only `T5-IR21.md:17` is keyed O-reply because only it reads the canned reply on screen.
- `test_app_library.cpp:204` (`gated_scores`) and `:296`, `:321`, `:397`, `:790` install a fake analyzer. None of the L-C12 rows start in a test that uses it (I read each), so no C-analyzer key was missing there.
- `counts/T5-IR3.md:24` (FG:130) uses a real clock with a broken fake front-window reader; I left it C-reader, P-message, E-echo.
- `T5-IR3.md:108` and `:109` (FA:886, :906) hand-build a `RunResult` of clock samples; F-sample ("recorder samples") is a loose fit because these are agent payloads, not recorder samples. Kept, not changed.
- The `dump.cs` ruling has a gap: `keys.md` question 1 makes hand-written source text input, but question 5 ("json is fake") makes hand-written game JSON a stand-in. The DF rows with a hand-built facts dict (`T5-IR3.md:54`, `:59`) are JSON of the dump tool's own output; I kept F-result for them, which is a split-adjacent call for the main session.

## Not checked

- I did not open the record files (`records/*.md`) for any row; I used the counter reasons and the tests.
- I did not read `test_squeeze_rating.cpp` rows `T5-IR21.md:72`, `:73`, `:74`, `:75`, `:78`, `test_highway_draw.cpp` rows `T5-IR20.md:42` to `:52`, or `test_store_*.cpp` rows beyond a grep for fake classes (none found).
