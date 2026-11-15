a3debc52b27b11a6a

# T5-KR5: Keep records re-read by the three test rulings

Task T5-KR5, session 63c70014-4328-49dc-9205-742235732752. Files: A21, A16, A6, A31, A32, A38, A49, A45, A15, A34, A33, A30, A27, A25, A18, A17, A50, A47, A3, A26, A19, A14, A12.

Line numbers are the test's lines in this branch today. They sit one to a few lines off the record's own numbers in several files (the files moved since the records were written).

Two of the 85 records my search found are not Keep. A18 record 20 (notationstr) and A18 record 47 (set_sqout) say "Verdict: Keep is ruled out ... Investigate". They match a search for "Keep" but their verdict is Investigate. I left them out: 83 Keep records read, all of them.

A38 holds no test: its 7 Keep records are verbs of the command interpreter `tests/ui/uitest_commands.cpp`, which has no checks. I read the file and marked each one `none`. A49 record 25 (`usage`) is also no test, only a usage-message function; `none`.

Calls I judged borderline and did not count: a hand-written library row (`library_row`, `library_entry`) used only as a chart list, with no result or analysis in it; a deliberately wrong md5 or an unknown failure kind used as a probe (A21 19 and 24, A33 T23); a typed 0.0 figure from a real corpus chart (A27 SF4). Say if you want these counted as X9.

## Table

Columns: record, test (file:line), faults, faulty check (file:line), reason, did the record name it.

### A21 (tests/test_preview_controller.cpp)

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A21 6 | :348 | X10 | :355 | `error() == "cancelled"` is the typed cancel text | yes (said "typed here as a literal") |
| A21 10 | :434 | none | | throws logic_error on a real controller | |
| A21 12 | :475 | none | | real WARP device, texture reference count | |
| A21 16 | :548 | none | | real corpus chart and path, real threads | |
| A21 17 | :575 | none | | real analysed path, key behaviour | |
| A21 19 | :672 | none | | real chart and path; wrong md5 is a probe | |
| A21 20 | :729 | none | | audio opens once, playhead kept; 1500 ms is an input | |
| A21 21 | :791 | none | | same flow, error cleared and spot kept | |
| A21 24 | :895 | none | | real scan, real store, fingerprint decides the hash | |
| A21 25 | :938 | none | | real chart and path, busy() | |
| A21 28 | :992 | X10 | :1001 | `on.fills == FillDeadlineRule::Ch10` states the setting-to-rule table back | yes ("the owner's mapping stated back") |

### A16 (tests/test_library_jobs.cpp)

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A16 5 | :193 | X9 | :197-198 | `fake_charts` rows and `released_in_turn`, an analyzer that only throws | yes ("with a fake analyzer") |
| A16 6 | :215 | X9 | :218-219 | same fake charts and fake analyzer | yes |
| A16 7 | :243 | X9 | :247-249 | same, `gated_failure` analyzer | yes |
| A16 8 | :271 | X9 | :275-277 | same | yes |
| A16 14 | :722 | none | | asks the real link whether malloc is redirected | |
| A16 15 | :750 helper | none | | real result plus padding to measure committed memory; 64 MB margin is the test's own | |
| A16 19 | :937 | none | | real old-build library, real trigger; sentence from `plain_error` | |
| A16 20 | :1000 | X9 | :1024-1025 | `library_row("a","A")...` hand-built chart rows stand for a library (three of four stops) | yes ("hand-built library rows") |
| A16 21 | :1108 | X9 | :1114-1135 | hand-built `library_row`s, `save_batch_record`, `empty_analysis_under` stale records; typed progress lists follow the typed slice sizes | yes ("fixture rows built by hand") |
| A16 25 | :1349 | X11 | :1357 | `error() == "boom"` after `fail(runtime_error("boom"))`, the typed text read back | partly ("boom is the test's input") |

### A6 (tests/test_app_library.cpp)

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A6 16 | :660 | none | | real corpus chart scanned | |
| A6 17 | :672 | none | | same | |
| A6 22 | :778 | X9 | :790-796 | analyzer that throws "no chart file in this test" for every chart stands in for the real analysis | no |
| A6 23 | :810 | none | | junk file as bad input; kind from the real open | |
| A6 25 | :898 | none | | | |
| A6 27 | :937 | none | | saved real v2.1.0 database copy; notice from the owner function | |
| A6 29 | :1015 | none | | | |
| A6 30 | :1049 | none | | | |
| A6 31 | :1061 | none | | | |
| A6 33 | :1106 | none | | | |

### A31 (tests/test_stem_reader.cpp, tests/test_store_library.cpp)

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A31 SR8 | stem_reader:382 | none | | real opus fixture file | |
| A31 ST1 | store_library:70 | X11 | :77-85 | builds the entry from seven typed strings and requires each back | no |
| A31 ST15 | :432 | none | | | |
| A31 ST16 | :451 | none | | | |
| A31 ST18 | :525 | none | | | |
| A31 ST20 | :598 | none | | | |
| A31 ST21 | :620 | none | | | |
| A31 ST22 | :647 | none | | | |
| A31 ST23 | :678 | none | | | |

### A32 (tests/test_store_open.cpp)

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A32 8 | :403 | none | | | |
| A32 14 | :704 | none | | real old-database fixture | |
| A32 15 | :723 | X10, X12 | :730 | `journal_size_limit == 4194304`, a typed constant, from the "value D93 chose" | yes (quotes D93) |
| A32 19 | :1094 | X10 | :1095 | SQLite compile option `ENABLE_FTS5 == 0`: a build flag typed | partly (cites sqlite.org) |
| A32 23 | :1165 | X10 | :1179 | `e.what() == "sqlite exec failed: database is locked"`, typed message | no |
| A32 24 | :1218 | none | | | |
| A32 26 | :1269 | X10 | :1270-1271 | two compile options typed | partly ("a build choice") |

### A38 (tests/ui/uitest_commands.cpp: no test case)

All seven records (click, type, open, timeout, wait-idle, wait-text, screenshot): none. There are no checks in the file; the records judge a verb's code against its page.

### A49

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A49 11 | single_owner/scan.cpp:170 | none | | rules against their own typed examples (the examples are the rule data) | |
| A49 14 | scan.cpp:283 | X10 | :322-325, :343, :347 | the typed problem and error sentences | no |
| A49 22 | scan.cpp:564 | none | | | |
| A49 25 | pb_ui/pbtest_main.cpp:25 | none | | `usage()` is no test | |

### A45 (tests/ui/uitest_preview.cpp)

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A45 8 | :564 | X10 | :573, :575, :580, :593, :595 | jump of 15000 and 10010 follow the typed "5 s" and "10 ms" button labels | yes ("5 s and 10 ms are the button labels") |
| A45 9 | :619 | none | | | |
| A45 10 | :647 | none | | | |

### A15, A34, A33, A30, A27, A25

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A15 25 | test_highway_draw.cpp:871 | X9 | :848-866 | `activation_until` hand-builds a PreviewActivation and the scene's notes (a made-up Activation) | no |
| A15 26 | :914 | X9 | :916-917 | hand-built scene notes | no |
| A34 R5 | test_store_results.cpp:295 | none | | records from the real corpus fixture | |
| A34 R12 | :460 | none | | | |
| A33 T20 | test_store_records.cpp:963 | none | | | |
| A33 T23 | :1023 | none | | the unknown kind 'LaterKind' is a probe (borderline) | |
| A30 10 | test_squeeze_rating.cpp:462 | X10 | :481-482 | `kScaleIdentityTolerance == 1e-3` and `kScaleIdentityDigits == 3` typed; the typed edges 1.0009 etc. follow the same typed D256 thousandth; the Tapestry/D256 comment is also a cited source (X12) | partly (says "1e-3 from D256") |
| A30 18 | :681 | X9 | :682-697 | `Activation`, `SPSqueeze`, `BackendSqueeze` hand-built and rated | no |
| A27 SF2 | test_saved_figure_fill.cpp:121 | none | | | |
| A27 SF4 | :147 | none | | | |
| A25 R3 | test_report_rows.cpp:235 | none | | | |
| A25 R26 | :919 | none | | | |

### A18 (not Keep)

| Rec | Test | Verdict in the record |
|---|---|---|
| A18 20 | test_model.cpp:654 | Investigate ("Keep is ruled out by the edge rows") |
| A18 47 | test_model.cpp:1151 | Investigate ("Keep is ruled out by 3") |

### A17, A50, A47, A3, A26, A19, A14, A12

| Rec | Test | Fault | Check | Reason | Record named |
|---|---|---|---|---|---|
| A17 5 | test_library_query.cpp:92 | none | | plain fold against traced fold on the corpus and on typed text (inputs to a text function) | |
| A17 12 | :211 | none | | | |
| A50 1 | test_store_query_plans.cpp:300 | none | | real fixture record | |
| A50 2 | :322 | none | | | |
| A47 hidden_lets_go | ui/uitest_report_windows.cpp:1373 | none | | | |
| A3 5 | test_activation_rows.cpp:925 | X10 | :930-933 | expects the typed labels "m2", "m2.1.0", "m1", "m1.1.0" | yes ("Hydra's own wording") |
| A26 RV4 | test_report_view.cpp:254 | X9, X10 | :255-258, :267-268 | hand-built `path_row` report rows; typed "2 of 3 paths" | no |
| A19 PC4 | test_precheck.cpp:495 | X10 | :499 | expects the typed text "not a git repository: ..." | yes ("the test copies it") |
| A14 41 | test_helpers.cpp:85 | X10 (weak) | :93 | typed prefix `"hydra_test_probe_"` | yes (flagged the literal) |
| A12 6 | test_docs_match_code.cpp:1270 | none | | scans the real committed docs | |

## Counts

- Keep records read: 83 (85 matched a search for "Keep"; A18 20 and A18 47 are Investigate).
- Records with at least one fault: 25 (A21 2, A16 7, A6 1, A31 1, A32 4, A49 1, A45 1, A15 2, A30 2, A3 1, A26 1, A19 1, A14 1).
- Records per fault: X9 11 (A16 5, 6, 7, 8, 20, 21; A6 22; A15 25, 26; A30 18; A26 RV4). X10 13 (A21 6, 28; A32 15, 19, 23, 26; A49 14; A45 8; A30 10; A3 5; A26 RV4; A19 PC4; A14 41). X11 2 (A16 25; A31 ST1). X12 2 (A32 15; A30 10). Snapshot 0. Some records hold two faults, so the per-fault numbers add to more than 25.
- Records whose fault the record already named (wholly or partly): 16 (A21 6, 28; A16 5, 6, 7, 8, 20, 21; A32 15, 19, 26; A45 8; A30 10; A3 5; A19 PC4; A14 41). The records said so and kept the test anyway.
- Records not reached: none.
