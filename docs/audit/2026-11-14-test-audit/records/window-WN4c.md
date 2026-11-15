# Window round WN4c (agent a96060d3d869b3e9e)

Task T5-WN4c, session e633f7f6-f313-4787-b622-8d901b577b64. Code read at the worktree HEAD (`5ec682c7`'s code). Nothing was run. A claim is "by reading" unless it names a run.

This re-sends the WN4 auditor's work (`records/window-WN4.md`, `verify/window-WN4.md`) for nine test files and three helpers. Rulings on the window, under `## Confirmed` in `docs/agents/rulings/code-and-engine.md` lines 55 to 57, quoted:

- "so we should use the hit window. and stop inventing our own numbers."
- "make the early fill threshold an adjustable setting pinned to one number in both directions"
- "analysis settings panel, default 60ms, supports 0ms to 85ms"

## What I found about the window (by reading)

- The default is 60. `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`) feeds `SearchSettings::early_fill_window_ms` (`src/search/pather.h:45`) and `app::Settings::early_fill_window_ms` (`src/app/config.h:195`). So `AnalysisSettings` made bare (`AnalysisSettings settings;`) and `Settings{}.to_analysis_settings()` both search at 60. `scratch_settings()` (`tests/scratch_settings.h:18`) sets only the chart folder, auto-open and depth 2, so `song_details::burnout()` (`tests/song_details_fixtures.h:22`) searches at 60.
- The window is set to 85 in two places in this group: `digest_settings` (`tests/song_digest.h:332`, `kMaxEarlyFillWindowMs`) and `analyzed_like_the_gui` (`tests/test_activation_rows.cpp:1228`, `kHitWindowMs`). Every case that calls either runs at 85. In this group those are the delta-round cases only (the perf digest cases, `tapped_out()`, `focus()`, `epidermis()` and line 1446), plus the parse-only edge-file cases.
- Corrections to the WN4 file. (1) `test_song_parse.cpp:1292` was sorted A at 85 and its verifier agreed. The body (1292 to 1310) calls `dynamics_for_settings`, `count_dynamics` and `load_shuffle_song`, which parse and count; it calls neither `digest_settings` nor the search. It is B. (2) `test_preview_golden.cpp:304` was sorted B on a token scan of its body. `render_chart` (line 88, called from 304) runs `analyze_chart_file` on `testdata/preview/notes.mid` at the default window and draws the best path's overlay. It is C. (3) `test_activation_rows.cpp:1331` was sorted B ("burnout(,early_fill_window" is a token hit); the verifier moved it to C and I agree. (4) The A row at `test_activation_rows.cpp:1186` is B (the verifier's dispute holds: the body is hand-built).

## How far I read, and what I did not read

I read the body of every C case, every case that has a corpus run in its body or its helper, and the helpers. For the B rows that hold no search I read the file's search entry points by token (`analyzed`, `analyze_chart`, `search_target`, `search_graph`, `ScoreGraph`, `batch_run`, `run_batch`, `best_finish`, `burnout()`, `digest_settings`, `early_fill`) over the whole file, then every typed number on an assertion line (`CHECK(... NNN` grep), then the bodies listed below. A B row whose body is not in this list was sorted by those two scans plus its name, not by reading its lines. The cap (150 calls, 30 minutes) stopped a line-by-line read of the hand-built B rows. The names are the rows of the tables below, by line number.

Lines whose bodies I read: `test_activation_rows.cpp` 1-150 (the cases at 60, 68, 76, 118 and the head of 142), 570-645, 1217-1236, 1296-1350, 1596-1701. `test_app_click.cpp` 1-260 (the cases at 98, 118, 140, 162, 195, 220), 836-865, 995-1032, and `tests/app_state_fixtures.h` 90-125. `test_app_reports.cpp` 60-112, 850-885, 994-1022. `test_cli.cpp` 105-135, 429-467, 511-566, 581-634, 682-712. `test_analysis.cpp` 173-222, 418-503, 1025-1050, 1322-1337, 1365-1390. `test_report_rows.cpp` 44-125, 134-232, 253-375. `test_song_parse.cpp` 1225-1246, 1284-1395. `test_preview_golden.cpp` 85-125, 304-328. `tests/song_digest.h` 289-338, `tests/song_details_fixtures.h` whole.

Not read line by line: `test_activation_rows.cpp` cases 142, 301, 310, 357, 419, 477, 525, 638, 740, 841, 854, 871, 897, 925, 936, 945, 1025, 1080, 1096, 1167, 1186 (the verifier read 1186); `test_app_click.cpp` cases (254, 277, 306, 340, 362, 382, 403, 424, 439, 450, 461, 792, 819, 866, 903, 957, 966); every `test_app_reports.cpp` case except the three reading ranges above; `test_cli.cpp` cases 223, 242, 268, 285, 297, 313, 334, 378, 396, 412, 468, 490, 568, 714; `test_analysis.cpp` every case outside the ranges above; `test_report_rows.cpp` every case from 395 on; `test_song_parse.cpp` cases 78 to 1207 (parse-only by token scan); `test_preview_golden.cpp` case 258. Each of these has no search entry point by the whole-file token scan above, and no typed number in the assertion-number grep except the ones listed in "Typed numbers".

## tests/test_activation_rows.cpp: A 0, B 27, C 3, delta round 7 (37 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 60 | display format: the average multiplier rounds the exact double | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 68 | display format: ms text is one decimal, a space and the unit | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 76 | build_activations: the early fill reads positive = early on both lines | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 118 | build_activations: rows and backend rows line up | B | analyzed_by_ten_scores (AnalysisSettings default window 60) is built into a view and every assertion compares the view with the same best path's own fields (notation, difficult, counts of rows); a value is never typed |
| 142 | build_activations: the scale line shows the multipliers its rows read, early first | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 301 | kTransferScaleHint explains the measure scaling | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 310 | build_activations: overfill warning text | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 357 | build_activations: the backend limit hides far rows but never ""squeezed-out ones | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 419 | build_activations: the backend limit says how many rows it hides on each side | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 477 | build_activations: a squeezed-out row past the leeway is worth 0 | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 525 | build_activations: plain rows past the leeway show 0 | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 570 | format_measure: one form for both tabs | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 582 | activation rows: Burnout's three activations | C | default window 60, see the record below |
| 638 | activation timeline: onset over the song's length, and the end measure | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 740 | activation badges: one per timing, the fill first, then each squeeze | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 841 | activation badges: an early fill with time to spare shows, grey (D261) | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 854 | activation badges: the row is difficult exactly when a badge is orange | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 871 | activation badges: the user's row agrees with the copied path | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 897 | activation badge and copied path agree | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 925 | measure labels: one numbering rule for the rows, the timeline and the first measure | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 936 | longest_activation_badge: the widest wording with the window's figure | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 945 | squeeze sentences: SqIn, SqOut, and what a squeeze-out costs | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 1025 | squeeze sentences at the SP end: the squeeze-in earns, the squeeze-out earns | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 1080 | backend table: a counted row inside SP shows its early-scale eff. figure | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 1096 | build_activations: a near-1 multiplier prints its decimals and its row's figure | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 1167 | build_activations: an unknown scale says so and shows no eff. | B | hand-built Activation or HydraRecord handed to build_activations, or a pure formatter; no search runs, so the window cannot reach the asserted strings |
| 1186 | activation rows: the chord names its notes in the Pro Drums setting's words | B | case body lines 1186-1215 is a hand-built Activation and build_activations over a HydraRecord; analyzed_like_the_gui (1221, window 85) sits after it and is not called here (the WN4 file sorted this A, and its verifier disputed it) |
| 1304 | dynamic hit window: the label is one constant | B | asserts one string constant |
| 1308 | dynamic hit window: Tapped Out's squeeze out is out of reach | delta round | records/delta-DL*.md has a record |
| 1331 | dynamic hit window: Burnout's squeeze out has 5.4 ms safe | C | default window 60, see the record below |
| 1354 | dynamic hit window: a squeeze in with a timing names its anchor the frontend | delta round | records/delta-DL*.md has a record |
| 1396 | dynamic hit window: an early fill gets one window, required or optional | delta round | records/delta-DL*.md has a record |
| 1439 | dynamic hit window: a squeeze in that needs no timing still gets its row | delta round | records/delta-DL*.md has a record |
| 1469 | dynamic hit window: a row for every timing badge, edited figures too | delta round | records/delta-DL*.md has a record |
| 1543 | dynamic hit window: no chord times, no row anywhere | delta round | records/delta-DL*.md has a record |
| 1556 | dynamic hit window: the row sits alongside and changes no other field | delta round | records/delta-DL*.md has a record |
| 1607 | dynamic hit window: every corpus row | C | default window 60, see the record below |

## tests/test_app_click.cpp: A 0, B 28, C 1, delta round 11 (40 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 98 | clicking a not-analyzed song shows its paths and turns its row Ready | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 118 | clicking a Ready song shows its paths and stores nothing new | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 140 | the click's save marks both open reports out of date | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 162 | a Stale converted record is replaced on click | C | default window 60, see the record below |
| 195 | clicking a Stale song turns its row Ready | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 220 | clicking a chart edited since the scan re-identifies it and shows the new file | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 254 | a click whose re-identify fails shows the error and saves nothing | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 277 | clicking a chart touched since the scan saves its new fingerprint | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 306 | a touched chart whose fingerprint save fails shows the error and saves nothing | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 340 | clicking a touched chart that lost its song.ini leaves its library row alone | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 362 | clicking an edited chart that lost its song.ini still re-identifies it | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 382 | clicking A then B in one frame shows B, and A is never shown or saved | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 403 | Cancel shows Cancelled and saves nothing; Try again shows the paths | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 424 | closing the panel cancels the click's analysis and saves nothing | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 439 | a song whose file is gone shows FileMissing and starts no job | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 450 | the settings lock is the batch's alone | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 461 | a click whose save fails shows the paths and says so | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 549 | a click's save does not stop the window while a save group is open | delta round | records/delta-DL*.md has a record |
| 578 | a click's worker writes a changed chart's hash before it saves | delta round | records/delta-DL*.md has a record |
| 607 | a click cancelled before its save saves nothing | delta round | records/delta-DL*.md has a record |
| 621 | a click closed before its save saves nothing | delta round | records/delta-DL*.md has a record |
| 636 | a click replaced before its save saves nothing | delta round | records/delta-DL*.md has a record |
| 658 | a click whose job was let go after it wrote still gives the row the store's answer | delta round | records/delta-DL*.md has a record |
| 697 | a click cancelled during its save keeps the panel cancelled and gives the row the store's answer | delta round | records/delta-DL*.md has a record |
| 708 | a click closed during its save gives the row the store's answer | delta round | records/delta-DL*.md has a record |
| 720 | a click replaced during its save gives the row the store's answer | delta round | records/delta-DL*.md has a record |
| 739 | a dropped click that wrote a new hash still reloads the library | delta round | records/delta-DL*.md has a record |
| 758 | a dropped click's save marks both reports out of date | delta round | records/delta-DL*.md has a record |
| 792 | a click on a chart with no notes stores the failure once | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 819 | a click whose song file can't be opened stores nothing | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 840 | the click's Dynamics count keeps the 2x kicks with 2x Bass off | B | Dynamics count is a parse count (count_dynamics over load_dynamics_song) on notes.mid; the 37 is a count of 2x kicks in the parse, not a search value |
| 866 | a bad hydra_rules.ini names the key and keeps analysis off | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 903 | a bad hydra_settings.ini keeps analysis off and saves nothing | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 957 | no hydra_rules.ini leaves analysis on | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 966 | under a bad hydra_rules.ini no stored record reads Ready | B | store rows, row status, panel and job state after a click; the one search is the first corpus .chart through the app, and each assertion compares the click with the same stored record or with a fresh analyze_chart_file on the same settings, or reads a count the search never decides |
| 998 | the click reads a chart's stated length | B | a two-note hand-built chart; no fill exists for the search to refuse, and the asserted value is song_length_ms from song.ini plus a pad |
| 1015 | a chart with no stated length and no audio reads its last note | B | the same two-note chart without audio; song_length_ms is the last note plus a pad |
| 1034 | the chart-file check runs on open and then every two seconds | B | a clock and file-existence check; no search |
| 1056 | UI timings: Copied!, search re-filter, batch refresh and the progress box delay | B | UI timing constants; no search |
| 1067 | the toolbar's narrowed label comes from one function | B | a label string from a count; no search |

## tests/test_app_reports.cpp: A 0, B 20, C 0, delta round 10 (30 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 86 | a path report request builds it in memory, one build at a time | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 114 | showing the path report opens its window and builds it only the first time | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 128 | --open-report opens the path report once the button shows, and only once | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 155 | the path report stays shut at a tick without --open-report | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 166 | closing the path report keeps its rows, and reopening shows them without a build | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 190 | a path report built on opening its window says so; Refresh's does not | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 213 | closing the path report mid-build lets the build finish and land | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 245 | reopening the path report after a cancelled build shows it cancelled | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 272 | the path report goes out of date on a batch or a setting it was built from | delta round | records/delta-DL*.md has a record |
| 332 | a setting changed during a build leaves that build out of date | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 349 | Cancel ends a path report build as cancelled and keeps the last report | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 370 | select_chart selects the first copy in the bar's mode | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 398 | select_chart opens the first copy by name when the search hides every copy | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 433 | a comparison row click selects the chart and keeps the bar's mode | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 467 | picking a player opens the comparison; Refresh reuses the player | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 569 | opening the path report runs a batch over the charts with no result, then builds | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 613 | the window's Cancel stops the report's own batch, and no report follows | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 636 | more rows to run than the threshold open the batch confirm first | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 741 | closing the path report while it asks ends the question, and reopening asks again | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 766 | a batch already running is waited on, and its finish builds the report | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 782 | the toolbar offers the path report whenever the library has rows | B | report state flags, urls, store reads and report rows built from one stored record (two builds of it compared); a fake analyzer where a batch runs; no typed value comes out of a search |
| 793 | the post-batch report lists the batch's cap and lens, not the live settings | delta round | records/delta-DL*.md has a record |
| 816 | the strip's report_started holds through a cancel, a refresh and a close | delta round | records/delta-DL*.md has a record |
| 952 | a path report builds while a save group is open | delta round | records/delta-DL*.md has a record |
| 972 | a DM report builds while a save group is open | delta round | records/delta-DL*.md has a record |
| 994 | a path report reads one moment | delta round | records/delta-DL*.md has a record |
| 1022 | a path report's separate store calls read one moment | delta round | records/delta-DL*.md has a record |
| 1056 | a report whose connection cannot open fails with the store's sentence | delta round | records/delta-DL*.md has a record |
| 1084 | a finished report holds no connection | delta round | records/delta-DL*.md has a record |
| 1097 | a cancelled report closes its connection | delta round | records/delta-DL*.md has a record |

## tests/test_cli.cpp: A 0, B 18, C 1, delta round 2 (21 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 183 | hydra_batch fills a new database under the rule it ran under | delta round | records/delta-DL*.md has a record |
| 223 | hydra_batch and hydra_fillcompare say why a database won't open | B | tool exit codes and option sentences; the only search is a one-chart batch that is not read for a score |
| 242 | hydra_batch --legacy-fills writes the tool's own hydra.db under the 1.0 rule | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 268 | hydra_batch refuses a library file too old to convert in one sentence | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 285 | hydra_batch refuses an unknown option | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 297 | hydra_batch and hydra_fillcompare print the one unknown-option line | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 313 | hydra_batch refuses a run with no folder and no --whole-library, and a mix of both | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 334 | hydra_batch --whole-library refuses a chart folder that cannot be read | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 378 | hydra_batch names a bad hydra_rules.ini and stops | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 396 | hydra_batch names a bad hydra_settings.ini and stops | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 412 | hydra_fillcompare names a bad hydra_settings.ini and stops | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 429 | hydra_batch writes a 1.0 run and a 1.1 run into one database | B | writes a 1.0 run and a 1.1 run of one chart; asserts two records exist (one per rule), "1 analyzed", and "0 only under 1.0, 0 only under 1.1", which counts rows present under each rule; every chart with two or more Star Power phrases has a path under both rules |
| 468 | hydra_batch reuses the GUI's scan cache | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 490 | hydra_batch prints an artist made only of tags as (unknown) | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 511 | hydra_batch names the cap with the one count rule | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 529 | hydra_fillcompare reads the tool's own hydra.db without --db | B | same reading as 429 through the tool's default hydra.db: it counts which rule has a record, not a score |
| 568 | hydra_fillcompare refuses --old and --new and names --db | B | tool exit codes, option sentences, store rows and the header text of a one-chart batch; the asserted counts are rows present under each rule, not scores |
| 581 | hydra_fillcompare compares both rules out of one database | C | default window 60, see the record below |
| 636 | hydra_replay refuses --ms, --efill and --depth values that are not whole numbers | delta round | records/delta-DL*.md has a record |
| 682 | hydra_replay score writes act_ms and deact_ms for each window | B | hydra_replay score on a hand-built chart with typed activation ticks; the tool scores and runs no search |
| 714 | hydra_batch and hydra_fillcompare import no winhttp.dll | B | an import-table scan of the exe bytes |

## tests/test_analysis.cpp: A 0, B 48, C 0, delta round 0 (48 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 54 | discover_charts walks the corpus without throwing | B | discover_charts walks folders; no search |
| 68 | discover_charts returns nothing for an empty root list | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 89 | discover_charts output matches the checked-in scan snapshot | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 122 | rescan cache reproduces the scan without reading chart files | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 173 | run_batch saves a record with its top paths through the one save | B | charts_with_paths runs the search at the default window and the test saves it; every assertion compares the stored row with the same run's best path, so the window moves both sides together |
| 198 | charts_with_result counts a Ready record and not a Stale or sibling-profile one | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 223 | run_work_pool hands every item to the consumer once | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 230 | run_work_pool: a slow consumer holds the workers back | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 270 | run_work_pool: a consumer that throws while workers wait for room | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 291 | run_work_pool: a cancel mid-run never strands the consumer | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 345 | run_work_pool refuses a worker count below 1 | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 358 | save_scan_as_library stores every ScanItem field | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 389 | normalize_chart_hash lowers a hash and leaves a lowercase one alone | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 418 | run_batch: cancel stops running searches within seconds | B | a fake analyzer that sleeps; the cancel is what is read |
| 470 | run_batch: a cancelled real search is neither a result nor a failure | B | cancel is pressed at the real search's first progress tick, and the case asserts no error, no result and no stored record; nothing a path or an E mark decides |
| 504 | run_batch analyzes a chart found in two folders once, and counts both rows | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 545 | plan_batch: a second copy runs with its first and counts as a row | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 560 | plan_batch: every copy of a stored chart counts as skipped | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 572 | plan_batch: a stored failure is skipped and counted as can't analyze, every copy | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 586 | plan_batch takes its copies from ChartIndex | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 617 | run_batch reports analyzed, skipped and failed itself | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 653 | run_batch reports a failed chart once per copy, under its first copy's name | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 718 | run_batch counts a failed save as a failed chart and goes on | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 753 | run_batch hands on_error the sentence its exception's kind names | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 776 | run_batch hands on_error no details line for a chart with no notes | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 801 | run_batch stores a chart with no notes, and the next batch skips it | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 846 | run_batch stores nothing for a song file it can't open | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 889 | run_batch hands on_failure_saved the chart whose failure it stored | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 914 | run_batch hands on_failure_saved nothing for a failure the store does not keep | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 934 | run_batch hands on_failure_saved nothing when the failure record's save fails | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 973 | run_batch hands on_failure_saved nothing after a run fails as a whole | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 999 | prepare_failed_chart: a remembered kind becomes a failure record, anything else nothing | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1025 | run_batch: a run whose profile asks for another cap than its search fails each chart | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1051 | discover_charts: a song with no usable name reads (unknown) | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1095 | read_song_ini_keys reads a Windows-1252 song.ini as UTF-8 | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1112 | rescan cache: an old placeholder or blank title reads (unknown) | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1136 | discover_charts: a blank or missing artist or charter reads the placeholder, ""fresh and from the rescan cache | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1195 | discover_charts reads the album from song.ini, a .sng and an .srb, trimmed, and a ""blank album stays blank | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1242 | read_chart_meta reads the names the scan reads from song.ini, a .sng and an .srb | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1303 | discover_charts finds a folder whose notes and ini names are capitalized | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1322 | analyze_chart_file reads a drum track whose first name is unrecognized | B | a hand-built one-note .mid; asserts the song is not empty |
| 1338 | run_batch: a group commits before any of its charts is reported | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1365 | run_batch: cancel during a group writes nothing more and leaves no transaction open | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1391 | run_batch: a failed group COMMIT saves each chart alone, and only a chart whose own ""save fails is failed | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1436 | the scan's folder count is reported while the walk runs, from the calling thread | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1460 | the scan's items come out in walk order whatever order they were hashed in | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1498 | a folder two roots reach takes its rootfolder from the root the walk reaches it by | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |
| 1528 | discover_charts walks a folder given twice in two spellings once | B | scan, plan_batch, run_work_pool, store and failure-callback cases; a real search appears only as the record that is saved and read back from the same run, or is cancelled before it ends |

## tests/test_report_rows.cpp: A 0, B 30, C 0, delta round 0 (30 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 204 | report rows: a tied top-score variant is optimal too | B | store_tied saves test::tied_variant_record(), a hand-built record; no search runs |
| 230 | report rows list every stored record (4 bars) | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 235 | collect_rows: the row list is sized to its rows | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 253 | path report: a batch's records read back as a fresh analysis's rows | B | a batch of five corpus charts and one hand-built tied record; the expected rows are a fresh analysis of the same charts on the same settings, so both sides come from the same search |
| 340 | path report: a saved figure below 0 shows no timing and the none tier | B | a hand-built Activation saved and read back |
| 376 | all_paths lists paths best first on every corpus chart | B | asserts best-first order of the paths one run keeps, a property of that run |
| 395 | report counts every library copy of a chart, and leaves out a chart the library dropped | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 430 | path report: the Charts shown tile counts what generate_report counts | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 451 | generate_report hands over its rows, subtitle and footer | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 494 | path report coverage: shown + can't analyze + no result is the library count (corpus) | B | counts of shown, can't-analyze and no-result charts; "can't analyze" is a parse or stored-failure result (Hard has charts with no charting), and the sum equals the library count |
| 548 | path report coverage: a chart with a stored failure gets no row and one list entry | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 579 | path report coverage: the lines' words | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 628 | path report coverage: no Ready chart but a stored failure reads Nothing to show | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 645 | report lists only the wanted cap and names it | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 689 | generate_report reports a 1.0-fills database under the 1.0 rule | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 713 | collect_rows: the library names go through display_title, display_artist and ""display_charter | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 737 | the report names a chart the scan found twice by its first copy | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 755 | fill_rule_for: the legacy_fills flag names one fill rule | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 761 | tier_for: raw-ms bands derived from the two-hit budget | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 786 | tier_for walks the timing_tiers table edge by edge | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 801 | path report: a row's search text is folded and tag-free | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 812 | generate_report: one seam frames the report for every entry point | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 870 | generate_report: \"every path\" is the sentinel's, a huge --paths stays a count | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 887 | nothing_under_settings frames the chart mode, the cap and the fills | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 902 | generate_report hands back nothing when its cancel flag is set | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 919 | a report with no analysis settings is refused | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 930 | a report on results with no chart library says the library is missing | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 951 | tier_for over a built table matches the one-argument form | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 962 | PageSettings::of differs when only the chart mode changes | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |
| 976 | settings_change_touches: the path report reads the SP cap and the lens | B | report rows and counts read from one stored record (a tied-variant record is hand-built), compared with a fresh analysis of the same chart on the same settings or with the record itself; coverage counts come from parse failures and stored failures |

## tests/test_song_parse.cpp: A 0, B 51, C 0, delta round 1 (52 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 78 | song parse holds its invariants over the corpus | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 92 | a parsed song keeps no spare room in its chord list | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 106 | song parse holds its invariants at Hard too | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 143 | cymbal_counts: a cymbal marker counts only under Pro Drums | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 167 | .chart: each difficulty reads its own section | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 208 | .mid: each difficulty reads its own pitch base | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 234 | .chart: [Events] section markers become practice sections | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 273 | .chart: E \"end\" and E \"[end]\" in [Events] are the end event | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 319 | .chart: the last note end is the latest N line's tick plus sustain in any section | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 343 | corpus: the last note end spans every instrument, through the tempo map | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 397 | .chart: the note on the solo end tick is in the solo | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 406 | .chart: time signatures are kept as written | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 427 | .mid: the note on the solo marker's note-off tick is outside the solo | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 435 | song: solo_sections lists each run of solo chords once | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 491 | drum lanes: the testdata lanes, with the pads Clone Hero lights | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 542 | .chart: a double lane on a three-pad chord takes its second pad from the next chord | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 561 | .chart: a double lane on a two-pad chord lights the next chord's highest pad | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 570 | .chart: a double lane on a two-pad chord skips a lone kick | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 579 | .chart: a double lane takes its second pad from a chord past the lane's end | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 589 | .chart: a double lane whose next chord repeats its first pad lights it once | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 598 | .chart: a double lane on the song's last chord lights its first pad alone | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 606 | .chart: N 66, 67 and 68 mark the yellow, blue and green cymbals; N 65 marks none | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 631 | .chart: a double lane on a one-pad chord takes the next chord with a pad | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 650 | .chart: a lane over kicks only, or over nothing, is dropped | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 660 | .chart: a lane starts on the first pad from one tick before it to its end | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 673 | .chart: a lane shows on each difficulty whose section holds it | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 683 | drum lanes take their pads from the shuffled notes with Note Shuffle on | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 725 | .chart: with Note Shuffle off, a lane over several pads keeps the game's pads | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 739 | .chart: with Note Shuffle on, a single lane over a red roll lights every pad | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 750 | Song::note_count: the chart's note total under each 2x Bass setting | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 769 | .mid: EVENTS text metas become practice sections | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 808 | .chart: a timing line that can't measure time is refused | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 841 | .chart: a TS 0 timing line is still ignored | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 852 | .mid: a timing line that can't measure time | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 878 | a resolution that is 0 or negative is refused in both formats | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 891 | mid: kick velocity is read as ghost/accent, like a pad's | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 943 | mid: Won't Get Fooled Again (O) has 36 ghost kicks | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 969 | mid: the marker pitch table is the one list the parser reads | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 981 | chart_files: loose-folder notes names match in any case | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 992 | chart_files: a path's format comes from its extension in any case | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1001 | mid: a stray SP note-off flags nothing | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1017 | .chart: [Song] Offset is read in seconds | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1090 | .chart: disco markers match the pinned tables | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1117 | .mid: disco markers match the pinned tables | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1147 | .chart: section headers are found as the pinned table says | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1200 | the no-notes error carries no_notes_message's sentence | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1207 | load-and-check throws the no-notes error for a missing difficulty | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1232 | Note Shuffle switch: a game-confirmed song loads shuffled on and as written off | B | parse output (chord codes) with Note Shuffle on and off |
| 1253 | Note Shuffle switch: a chart the game freezes on gives the freeze error | B | analyze_chart_file throws the freeze error at load, before any search |
| 1284 | Note Shuffle switch: the Preview's song key differs when only the switch differs | B | parser output (lanes, sections, times, pads, errors); the parse never reaches fill_refuses or is_e0 |
| 1292 | Note Shuffle switch: the Dynamics pads follow the analysis's shuffle | B | dynamics_for_settings and count_dynamics over parsed songs; the case does not call digest_settings or the search (the WN4 file sorted this A at 85, and its verifier agreed; the body shows no search) |
| 1385 | the crafted edge files parse to their pinned lines | delta round | records/delta-DL*.md has a record |

## tests/test_preview_golden.cpp: A 0, B 1, C 1, delta round 0 (2 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 258 | preview golden: over_share counts one bad tile against golden.json's limits | B | arithmetic on a made-up delta map against golden.json limits; no search runs |
| 304 | preview golden: a Hydra frame matches the Onyx screenshot | C | default window 60, see the record below |

## tests/test_perf_digest.cpp: A 0, B 0, C 0, delta round 8 (8 cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 128 | the corpus's prepared-row digest is pinned | delta round | records/delta-DL*.md has a record |
| 132 | the corpus's prepared-row digest is pinned at Hard, Pro Drums off, 2x Bass off | delta round | records/delta-DL*.md has a record |
| 136 | the corpus's prepared-row digest is pinned with the CH 1.0 fill rule | delta round | records/delta-DL*.md has a record |
| 142 | the corpus's prepared-row digest is pinned with Note Shuffle on and the ms limit off | delta round | records/delta-DL*.md has a record |
| 149 | the corpus's parse digest is pinned | delta round | records/delta-DL*.md has a record |
| 154 | the corpus's parse digest is pinned at Hard, Pro Drums off, 2x Bass off | delta round | records/delta-DL*.md has a record |
| 160 | the corpus's parse digest is pinned at Medium, Pro Drums off, 2x Bass off | delta round | records/delta-DL*.md has a record |
| 164 | the corpus's parse digest is pinned at Easy, Pro Drums off, 2x Bass off | delta round | records/delta-DL*.md has a record |

## Helpers

| File | Window | Reason |
|---|---|---|
| `tests/app_state_fixtures.h` | default 60 | `app_with` builds `AppState(Settings{}, store)` (line 125), and `click` runs the app's own analysis on the first corpus `.chart` (`write_corpus_chart`, line 96). No window is set. The test analyzer hook (line 166) calls `analyze_chart_file` with the settings it is given. |
| `tests/song_digest.h` | A, 85 | `digest_settings` sets `s.early_fill_window_ms = kMaxEarlyFillWindowMs` (line 332, 85 per `src/core/model.h:115`) so the pinned digests keep describing the search they were pinned under. `chart_parse_hash` (289) parses and never searches. |
| `tests/song_details_fixtures.h` | default 60 | `burnout()` is `analyze_chart_file` on Green Day - Burnout with `scratch_settings().to_analysis_settings()` (depth 2). Feeds the C records for 582 and 1331. |

## C records

### 1. activation rows: Burnout's three activations (tests/test_activation_rows.cpp:582)

```
Delta round: window, replaces records/A2.md record 13 (heading "## 13. activation rows: Burnout's three activations"). Old verdict Rewrite. It still holds at 60 ms and now has one more reason: the case's first REQUIRE types the search's own result.
Read:       582-636; check_word_agrees_with_badge (38-56); song_details::burnout (tests/song_details_fixtures.h:22); build_activations and its callers (src/app/path_view.cpp:246, :637, tests only otherwise); sqout_cost (src/app/path_view.cpp:128); records/A2.md 13; no engine record for Burnout exists (records-engine holds 16 files, none Burnout). Adding commit: not read.
1 Claim:    On Burnout the best path is "3- 1 2", and its three rows read: 3 bars at m32.1.0 with the orange badge "squeeze out 163.0 ms" and a sentence naming the [  Y  ] note and "260 fewer points"; 6 and 7 notes near the SP end for rows 2 and 3.
2 Owner:    The engine owns the path, the squeeze and the measures (the record's Activation fields). build_activations words them; sqout_cost (core) prices the 260. build_activations is called by the Song Details screen only (path_view.cpp:637). Every value is the engine's, shown by the display. The test types them instead of reading them.
3 Real chart: Burnout is a real chart. Engine: no dump (the engine records are at 85 and none is Burnout). Claim: "3- 1 2" at score 378315 at 60 ms. Claim not checked. Needed: hydra_replay dump and schedule of Green Day - Burnout at --efill 60 (and 85, to see if the path moves).
4 Direct:   586 and 587 type a path string and a score taken from one run. 597, 601, 607-608, 610, 615, 618, 622, 623 are raw results. 592, 595-596, 603, 609 read as properties but each equals a value typed beside it.
5 Reason:   "3- 1 2", 378315, m32.1.0, m58.1.0, m88.1.0, 163.0, 260, 3, 6, 7: no reason is written; each is what the code printed. The comment-free test cannot say whether they hold at 60.
6 Display:  Fails. The expected side is typed, not read from best.activations item by item.
7 Failure:  The adding commit is not read. A plausible failure: a row built from the wrong activation, or a lost squeeze sentence, on a real chart. Still possible.
8 Deleted:  586 (reasoned): the path or score changing, a snapshot that hides every later line when it moves (the window can move it). 601 (reasoned): `format_ms` or the badge text changing. 607-608 (reasoned): the sentence or sqout_cost. 610, 618, 623 (reasoned): display_backends' count. 635 (reasoned): a second squeezed-out row appearing.
9 Stand-in: REQUIRE(best.pathstring() == ...) stands for "the chart is the one this test reads". The Burnout lookup throws if the chart is missing (song_details_fixtures.h:30), which is loud.
10 Same rule: 1331 reads the same chart and activation; tests/test_path_list.cpp uses burnout() (per the old record, not re-opened); tests/ui/uitest_paths.cpp shows the same strings (not read).
11 Real:    Yes.
Verdict:    Rewrite. For each activation of the record, each shown field equals the record's field (number, notationstr, format_measure of its timecode, bars, badge_timings), and the sentence's points equal core::sqout_cost; no path string, score or count is typed. Claim not checked at 60 ms.
Finding:    Typed numbers that a 60 ms run can move: 586 "3- 1 2", 587 378315, 597/615/622 the measures, 601 and 607 "163.0 ms", 608 "260", 610/618/623 counts 3, 6, 7.
Today it catches: a lost squeeze sentence and a wrong badge on a real chart (kept by the rewrite).
```

### 2. dynamic hit window: Burnout's squeeze out has 5.4 ms safe (tests/test_activation_rows.cpp:1331)

```
Delta round: window, replaces records/A3.md record 15 (heading "## 15. dynamic hit window: Burnout's squeeze out has 5.4 ms safe"). Old verdict Investigate (with record 14, chart missing and the safe/short tag split). It still holds at 60 ms.
Read:       1331-1349; burnout (song_details_fixtures.h:22); window_rows_of, squeeze_row (tests/test_activation_rows.cpp, near 1240-1300, names only); squeeze_window_ends and early_fill_window_end (src/core/note_window.cpp:113, :132; callers src/app/path_view.cpp:227, :238 only); records/A3.md 15 and 22. Adding commit: not read.
1 Claim:    On Burnout's best path "3- 1 2", activation 1's squeeze out shows "frontend 84.5 ms [K   G]   backend 84.3 ms [  Y  ]" and "5.4 ms safe"; activations 2 and 3 show no row.
2 Owner:    squeeze_window_ends works out the ends; its only caller is path_view.cpp. A game fact (whether a squeeze fits the hit windows) worked out in code only a display calls: derive-once finding (kind A), already on A3 record 22; not re-judged. The path itself is the engine's.
3 Real chart: Burnout again; no engine record at any window. Claim not checked. Needed: hydra_replay dump and schedule of Burnout at --efill 60.
4 Direct:   1334 types the path string. 1340 types two ms figures and two chords; 1341 types "5.4 ms safe". Raw results.
5 Reason:   84.5, 84.3 and 5.4 are what the display printed. The 5.4 follows from the display's own rule (safe = ends minus a hit window); the engine's schedule (tools/replay_schedule.cpp, anchor_room_ms, a 3 ms kRoomMarginMs per A3 record 22) answers the same question with other numbers.
6 Display:  Fails. Typed expected values.
7 Failure:  Not read (adding commit). A plausible failure: the row's figures swapping frontend and backend, or a row on an activation with no timing badge.
8 Deleted:  1334 (reasoned): the path moving (the window can move it); a stand-in guard. 1340-1341 (reasoned): the figure or tag text changing. 1346-1347 (reasoned): a row appearing on activations 2 and 3.
9 Stand-in: 1334 stands for "this is the chart".
10 Same rule: 582 (same chart and row); the 1607 corpus case checks the roles on every chart.
11 Real:    Yes.
Verdict:    Investigate. The tag rule and the engine's schedule disagree on the same question (A3 record 22's split B), so a decision goes to the user; claim not checked as well.
Finding:    Typed numbers a 60 ms run can move: 1334 "3- 1 2", 1340 "84.5 ms" and "84.3 ms", 1341 "5.4 ms".
```

### 3. dynamic hit window: every corpus row (tests/test_activation_rows.cpp:1607)

```
Delta round: window, replaces records/A3.md record 22 (heading "## 22. dynamic hit window: every corpus row"). Old verdict Rewrite. It still holds at 60 ms.
Read:       1596-1701; check_any_row (above 1596, read by name only); corpus::analyzed, corpus::song (headers not opened); records/A3.md 22. Adding commit 62fe7b2b (from the old record, not re-read).
1 Claim:    On every kept path of every corpus chart at the default window, each timing badge has one Dynamic Hit Window row; an early fill row has one end and no word; a squeeze row names its anchor frontend and its squeezed note backend in the owner's order; the tag word follows the sum's sign.
2 Owner:    squeeze_window_ends and early_fill_window_end (callers: path_view.cpp:227, :238 only). The test compares the view with the same functions that fed it (1650, 1664, 1676-1677): a function against its own caller. The paths are the engine's, searched at 60 (Settings().to_analysis_settings(), line 1608).
3 Real chart: the corpus, run at 60. Engine: no dump at 60. The old record checked Beg at 85 only. Claim not checked at 60.
4 Direct:   1647 (rows equal badges) is a property. 1668-1669 roles are fine. 1691-1696 are floors of the search's output.
5 Reason:   1691-1700 `> 0` floors: the comment at 1694 says "Tapped Out's squeeze out is short, Burnout's safe" and 1697 says Focus and Epidermis squeeze ins press early. Those are facts about charts' paths at 85 (these charts are run at 85 elsewhere in the file through analyzed_like_the_gui, 1228); here the corpus runs at 60.
6 Display:  Partly: the expected side is the owner's, not typed, but the owner is the display's own function (see 2).
7 Failure:  A badge without a row, or roles switched by press direction (comment 1603-1606). Possible.
8 Deleted:  1647 (reasoned): a badge with no row. 1668-1669, 1676-1677 (reasoned): frontend and backend switched. 1685 (reasoned): the sentence's word missing. 1691-1700 (reasoned): the loop quietly skipping every chart. 1620 and 1624 `continue` silently on an empty song or a thrown analysis: a chart that fails to load disappears from the loop, and `charts > 0` does not name it.
9 Stand-in: 1691-1700 floors stand for "the loop checked something, with the right kinds of row"; `short_tags > 0` and `safe_tags > 0` need Tapped Out's short squeeze out and Burnout's safe one to survive at 60.
10 Same rule: 1308 and 1354 (delta round) state the same role rule on named charts; no verdict here.
11 Real:    Yes.
Verdict:    Rewrite. Keep the shape (view against the owner, chord by chord, every chart); count the skips at 1620 and 1624 and require zero or name them; replace the floors with a check by chart name; and take the sum's sign from the engine's schedule once the tag rule is decided.
Finding:    Floors that a 60 ms run can move: 1691 charts > 0, 1692 squeeze_rows > 0, 1693 fill_rows > 0, 1695 short_tags > 0, 1696 safe_tags > 0, 1700 note_pressed_early > 0. Each is a count over searches at 60.
```

### 4. a Stale converted record is replaced on click (tests/test_app_click.cpp:162)

```
Delta round: window, replaces records/A5.md record 4 (heading "## 4. a Stale converted record is replaced on click"). Old verdict Keep. At 60 ms the Keep stands only once its one search-dependent premise (below) is checked, so the verdict becomes Investigate (claim not checked only).
Read:       162-193; app_state_fixtures.h 96-125 (write_corpus_chart, corpus_chart, app_with); records/A5.md 4. Adding commit 94e61a27 (old record, not re-read).
1 Claim:    A chart whose only stored record came from an old file layout reads Stale; clicking it replaces that record with one new record holding every path the fresh analysis keeps.
2 Owner:    save_view_record and stored_says_same (store side; old record 2). The test's expected record is prepare_record over analyze_chart_file, computed apart from the click. A store rule, not a display rule.
3 Real chart: the corpus's first .chart (corpus::first_chart_with_suffix, `app_state_fixtures.h:99`). Engine: no dump; chart not named by the test. Claim not checked for the one fact it rests on: that this chart keeps more than one path at 60 ms.
4 Direct:   168 REQUIRE(fresh.record.paths.size() > 1) is an input guard ("so the converted record lacks some paths"); 179 types {1,1} from v220_layout_sql (the starting state); 186-187 the owner's answer; 191-192 a property.
5 Reason:   168's reason is in the comment. The number 1 is a count of paths from the search at 60 and could be 1 on a chart where the window removes a variant.
6 Display:  Not a display test; row status is read from the store.
7 Failure:  A converted record left beside the new one, or Ready shown while Stale. Possible.
8 Deleted:  168 (reasoned): the test passing with nothing to replace. 187 (reasoned): a leftover converted row. 186 (reasoned): a partial path list saved.
9 Stand-in: none.
10 Same rule: 195 (Stale to Ready, no typed numbers) and the old record's test 5.
11 Real:    A user with a library from 2.2.0 to 2.7.0: the range is the comment's claim, not checked.
Verdict:    Investigate (claim not checked only): a dump of the corpus's first .chart at --efill 60 showing more than one kept path. If it fails, 168 goes red loudly on the main session's next build.
Finding:    Number a 60 ms run can move: 168 `paths.size() > 1`.
```

### 5. hydra_fillcompare compares both rules out of one database (tests/test_cli.cpp:581)

```
Delta round: window, replaces records/B1.md record 18 (heading "## 18. hydra_fillcompare compares both rules out of one database"). Old verdict Rewrite (state the sum as a property). It still holds at 60 ms.
Read:       581-634; CliSandbox (tests/test_cli.cpp:144-165, chart copied from small_chart()); small_chart (121-137); records/B1.md 18. Adding commit: not read.
1 Claim:    A database holding one chart's record under the 1.0 rule and one under the 1.1 rule compares as "1 chart: 0 only under 1.0, 0 only under 1.1, 0 in both, not both scored"; after a second chart with a score under 1.1 only is added, it reads "2 charts: 1 same, 0 1.0 higher, 0 1.1 higher, 0 only under 1.0, 0 only under 1.1, 1 in both, not both scored".
2 Owner:    hydra_fillcompare's counting (the tool); the scores come from the engine's two searches that the test runs at 589-594 and 614-616 on the default window. The fill window applies to both rules (src/search/engine.cpp:859, :862, :1007 have no rule branch).
3 Real chart: the smallest corpus .chart with two or more Star Power phrases (small_chart). Engine: no dump. The claim needs the 1.0 and the 1.1 searches to give equal scores on this chart ("1 same") at 60. Claim not checked.
4 Direct:   628-629 type a sentence whose count "1 same" is a search result (the first chart scores equally under both rules); the other counts are 0s or the one-sided chart. 604-605 type the one-chart line.
5 Reason:   "1 same" has no reason except that the chart scores equally under both rules on the day it was written.
6 Display:  A report line from the tool, typed. Fails.
7 Failure:  Not read. A plausible failure: both rules' records in one file read back under the wrong rule; possible.
8 Deleted:  628-629 (reasoned): a count, or the sum of the six parts, being wrong. 604-605 (reasoned): the same count for one chart.
9 Stand-in: none.
10 Same rule: 429 and 529 assert the "0 only under" half and are B.
11 Real:    Yes (a corpus chart).
Verdict:    Rewrite. State the sum as a property (the six counts add to the compared total) and, for the same-score count, read each chart's two scores from the stored records instead of typing "1 same". Claim not checked at 60 ms.
Finding:    Typed counts a 60 ms run can move: 628-629 "1 same", "1 in both, not both scored", "Compared 2 charts".
```

### 6. preview golden: a Hydra frame matches the Onyx screenshot (tests/test_preview_golden.cpp:304)

```
Delta round: window, replaces records/A21.md record 30 (heading "## 30. preview golden: a Hydra frame matches the Onyx screenshot"). Old verdict Investigate (the ruling "we don't need golden anymore, remove it all" at code-and-engine.md:50 against ADR 0008). It still holds. The WN4 file sorted this case B ("no search call in the body"); that was a token scan of the body only.
Read:       85-125 (render_chart), 304-328; testdata/preview/golden.json (chart notes.mid, blink-182 "What's My Age Again", Expert Pro Drums, 2x Bass, paused at 36913 ms); code-and-engine.md:50-51; records/A21.md 30.
1 Claim:    Hydra's Preview frame of the chart at 36.913 s, including the path overlay (active Star Power windows, fills), looks like a screenshot of Onyx at that moment, within the file's tolerances.
2 Owner:    The engine's best path draws the overlay (build_preview_scene r.song, best, line 105). The test searches with AnalysisSettings at the default window 60 (lines 95-103: depth by score 10, ms limit 10; no window set).
3 Real chart: notes.mid. Engine: no dump. Claim: the best path at 60 shows (or not) a Star Power window and fills at 36.913 s the same way as at 85; the screenshot is from Onyx, which shows no Hydra path. Claim not checked. Needed: hydra_replay dump of testdata/preview/notes.mid at --efill 60 and 85, and whether an activation is open at 36913 ms.
4 Direct:   412 to 414 (from the old record): mean error, worst tile and frame under the file's limits. The thresholds are typed in golden.json.
5 Reason:   tolerance 7, pixel_delta 32, tile_percent 9.4 and frame_percent 0.08 are tuned to the screenshot (README.md "The thresholds", per old record, not re-read).
6 Display:  A display test whose expected side is a saved screenshot, not the engine's record.
7 Failure:  The Preview drifting from the Onyx port (ADR 0008). Possible.
8 Deleted:  The tolerance lines (reasoned): any texture or camera change moves them; a pass means closeness only.
9 Stand-in: render_chart catches an analysis exception (106) and draws with no path, so a failing analysis hides behind a path-free frame (old record's finding; confirmed by reading line 106-108).
10 Same rule: none.
11 Real:    Yes.
Verdict:    Investigate. The ruling (a saved expected-output file) and ADR 0008 are both in the repo and no rule says which wins; it goes to the user. New at 60 ms: whether the overlay under the default window is what the screenshot was matched to is unchecked.
Finding:    Value a 60 ms run can move: the best path overlay in the frame at 36913 ms (line 105), compared against the saved image at lines 374-389.
```

## Typed numbers that a run at 60 ms can move (assertion lines, non-delta cases)

- `tests/test_activation_rows.cpp:586` "3- 1 2"; `:587` 378315; `:597` "m32.1.0"; `:601` "squeeze out 163.0 ms"; `:607-608` "163.0 ms" and "260 fewer points"; `:610` "3 notes near the SP end"; `:615` "m58.1.0"; `:618` "6 notes near the SP end"; `:622` "m88.1.0"; `:623` "7 notes near the SP end".
- `tests/test_activation_rows.cpp:1334` "3- 1 2"; `:1340` "84.5 ms" and "84.3 ms"; `:1341` "5.4 ms safe".
- `tests/test_activation_rows.cpp:1691-1696, 1700`: the `> 0` floors named in record 3.
- `tests/test_app_click.cpp:168` `paths.size() > 1`.
- `tests/test_cli.cpp:628-629` "Compared 2 charts: 1 same ... 1 in both, not both scored"; `:604-605` "Compared 1 chart" and "0 in both, not both scored".
- None that I can name in the B cases: `test_app_click.cpp:857` (37 2x kicks), `:1010` (5321.0) and `:1027` (1100.0) are parse or song.ini values; `test_cli.cpp:704-707` are typed ticks into hydra_replay score; `test_app_reports.cpp:491, 509, 637` are ids and a constant.

## Splits

- `tests/test_activation_rows.cpp` runs two windows in one file: `analyzed_like_the_gui` (85, line 1228) feeds the delta-round cases, and `burnout()` (60) feeds 582 and 1331, which type the same family of numbers ("163.0 ms", "84.5 ms", "5.4 ms safe"). The case at 1607 runs the whole corpus at 60 while its comments (1694, 1697) cite charts found at 85.
- `tests/song_digest.h:332` runs the digests at 85 while `app::Settings` defaults to 60: the parse and prepared-row digests pin parse output only, and their window is a second one beside the app's.
- Same question, two answers: the Dynamic Hit Window tag ("safe at 0.0 or more, short below") at `src/core/note_window.cpp` and the engine schedule's room margin (records/A3.md 22, split B), unchanged by the window.
- `tests/test_preview_golden.cpp:304` draws the best path at 60 ms over a screenshot made by Onyx, which has no early-fill window at all.

## Not reached

See the reading list at the head of this file. No test case was left without a row. Not read line by line: the case names under "Not read line by line" above, by line number in the tables.

## Anything I stopped on

- The `test_song_parse.cpp:1292` A row of the WN4 file was wrong in the group's count (A 1 becomes 0, B 51).
