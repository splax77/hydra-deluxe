# Verifier: window round, group WN4c (agent a24f3a0278c4d2900)

Task T5-V-WN4c, session e633f7f6-f313-4787-b622-8d901b577b64. Part checked: `records/window-WN4c.md`. Code read at the worktree HEAD. Nothing was run. A claim is "by reading" unless it names a run. No engine record at 60 ms exists, so no claim here is "proven by" a run.

## Scope

I counted the test cases in each file with a grep for `^TEST_CASE` and compared with the tables. The counts match: test_activation_rows 37, test_app_click 40, test_app_reports 30, test_cli 21, test_analysis 48, test_report_rows 30, test_song_parse 52, test_preview_golden 2, test_perf_digest 8 (268 in all). I found no test case missing from the tables.

I redid the search for search entry points (`analyze_chart_file`, `analyzed(`, `analyzed_by_ten_scores`, `charts_with_paths`, `burnout()`, `digest_settings`, `analyzed_like_the_gui`, `ScoreGraph`, `Engine`, `click(`) across the group. Where each one is called:

- `test_activation_rows.cpp`: `analyzed_by_ten_scores` at 119 (case 118), `burnout()` at 583 and 1332, `corpus::analyzed` at 1623 (case 1607), `analyzed_like_the_gui` and its three wrappers at 1309, 1356, 1371, 1397, 1446, 1471, 1504, 1524, 1544 and 1558. Every one of those last ten sits inside a delta-round case (1308, 1354, 1396, 1439, 1469, 1543, 1556), so the table is right. No other case reaches the search.
- `test_analysis.cpp`: `charts_with_paths` (175), and `analyze_chart_file` at 211, 481, 622, 697, 1332, 1341, 1366. Each is either compared with the stored copy of the same run, a canned result handed to a fake analyzer, or cancelled before it ends.
- `test_song_parse.cpp`: `analyze_chart_file` at 1276 (throws at load) and `digest_settings` at 1355, which is the helper for the delta-round case 1385.
- `test_app_reports.cpp`: the one search is the `click` inside `app_with_unanalyzed` (557); the five seeded charts have no file.
- `test_report_rows.cpp`: `analyze_chart_file` at 66, 112, 517 and `corpus::analyzed` at 386; every use is compared with the same settings' own result.
- `test_cli.cpp`: `analyze_chart_file` at 594 and 616 (case 581, a C).

I did not read the hand-built B bodies line by line either; the auditor's own list of unread cases stands, and my search finds no entry point in them.

## The window claims (by reading)

- Default 60: `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`), `SearchSettings` (`src/search/pather.h:45`), `Settings` (`src/app/config.h:195`). Agreed. `AnalysisSettings` has no field of its own in the files I searched under `src/app/*.h`; `grep early_fill_window_ms` there finds only `config.h`. The auditor's wording "AnalysisSettings made bare searches at 60" is by way of the search default; agreed as to effect.
- `digest_settings` sets `kMaxEarlyFillWindowMs` (`tests/song_digest.h:332`); `kMaxEarlyFillWindowMs = int(kHitWindowMs)` (`model.h:115`), 85. Agreed.
- `analyzed_like_the_gui` sets `kHitWindowMs` (`tests/test_activation_rows.cpp:1228`), its own comment says so. Agreed.
- `burnout()` uses `scratch_settings().to_analysis_settings()` and sets no window (`tests/song_details_fixtures.h:22-34`). Agreed, 60.
- The window reaches the legacy-fills rule too: `engine.cpp:859, :862, :864, :869, :1007, :1224` take `early_fill_window_ms_` with no rule branch. Agreed.
- The four corrections the auditor lists: 1292 is B (I read 1292-1310: only `dynamics_for_settings`, `count_dynamics`, `load_shuffle_song`; no search). Agreed. 304 in `test_preview_golden.cpp` is C: `render_chart` (85-111) calls `analyze_chart_file` with a bare `AnalysisSettings`. Agreed. 1331 is C. Agreed. 1186 is B. Agreed (the WN4 verifier's finding).

## Kinds checked

| File | A rows | A disputed | B rows | B disputed | C rows | C disputed |
|---|---|---|---|---|---|---|
| test_activation_rows | 0 | 0 | 27 | 0 | 3 | 0 |
| test_app_click | 0 | 0 | 28 | 0 | 1 | 0 |
| test_app_reports | 0 | 0 | 20 | 0 | 0 | 0 |
| test_cli | 0 | 0 | 18 | 0 | 1 | 0 |
| test_analysis | 0 | 0 | 48 | 0 | 0 | 0 |
| test_report_rows | 0 | 0 | 30 | 0 | 0 | 0 |
| test_song_parse | 0 | 0 | 51 | 0 | 0 | 0 |
| test_preview_golden | 0 | 0 | 1 | 0 | 1 | 0 |
| test_perf_digest | 0 | 0 | 0 | 0 | 0 | 0 |

The delta-round rows (7, 11, 10, 2, 0, 0, 1, 0, 8 = 39) I did not judge, as the brief says. Where a B reason leans on a claim I could only check by reading, the weak spots are these, none of them a dispute:

- `test_cli.cpp:429`, `:529`, `:242`: the reason says every chart with two or more Star Power phrases has a path under both rules. No code or record in the file proves it at 60. The assertions are counts of rows present, so the reason holds unless a chart loses all its paths at 60; that is not shown either way.
- `test_cli.cpp:581` is the same fact feeding the C row, where it is typed ("1 same").
- `test_app_click.cpp:840` (37 2x kicks): the count is `count_dynamics` over the parsed song (line 857), so B holds.
- `test_report_rows.cpp:376` (best-first order on every corpus chart) reads a property of the same run; B holds.

## The C records

All six verdicts: agreed.

1. `test_activation_rows.cpp:582`, Rewrite. Claims read against the code: lines 586-587 type "3- 1 2" and 378315; 592 "3" summary; 597/615/622 measures; 601 "163.0 ms"; 607-608 "260 fewer points"; 610/618/623 counts 3, 6, 7; 635 `squeezed_out == 1`. All present at the lines named (by reading). `burnout()` is 60. Agree. Question 8's bug (the path or score changing at 586, and a lost squeeze sentence at 607-608) is named at lines. One number the list leaves out: 592 (`"3" + kDot + "no SP left over"`) and 635 (`squeezed_out == 1`). Both can move with the path. Add them to the typed-number list.
2. `test_activation_rows.cpp:1331`, Investigate. Lines 1334, 1340, 1341 read as the record says. Agree with Investigate: the verdict rests on a split (the tag rule in `note_window.cpp` against the schedule's margin, A3 record 22); I did not re-read `note_window.cpp:113/132` or `replay_schedule.cpp`, so the split itself is "by reading A3 22", not rechecked here. Question 8 names lines. Agree.
3. `test_activation_rows.cpp:1607`, Rewrite. Settings at 1608 (`Settings().to_analysis_settings()`, 60), the silent `continue` at 1620 and 1624-1625, and the floors at 1691, 1692, 1693, 1695, 1696, 1700 all match (I checked with grep: 1691, 1693, 1695, 1700 are exact). Agree. The record's point that the comments at 1694 and 1697 cite facts true at 85 holds by reading: the same file runs those charts at 85 through `analyzed_like_the_gui`.
4. `test_app_click.cpp:162`, Investigate (claim not checked only). Line 168 `fresh.record.paths.size() > 1` and 179 `{1,1}` read as stated; 168 is a search-dependent guard that fails loudly if false. Agree. The wording matches the brief's rule.
5. `test_cli.cpp:581`, Rewrite. Lines 604-605 and 628-629 type the sentences. The test builds the first chart's two records from `analyze_chart_file` at 594 and 616 on default settings (60). The "1 same" at 628 needs equal scores under both rules; no engine record at 60 shows it. Agree. The auditor's text says 604-605 type the one-chart line; correct, and that line does not contain "1 same" (it holds only "0 only under..., 0 in both, not both scored"), so the "1 same" that can move is in 628-629 alone.
6. `test_preview_golden.cpp:304`, Investigate. Ruling quoted: `docs/agents/rulings/code-and-engine.md:50`, "we don't need golden anymore, remove it all" (confirmed under `## Confirmed`; I read lines 50-57). ADR `docs/adr/0008-preview-is-a-faithful-port-of-onyx.md` exists. `golden.json` holds tolerance 7, pixel_delta 32, tile_percent 9.4, frame_percent 0.08 (read). `render_chart` swallows an analysis exception (106-108) and draws with no path: confirmed by reading. Agree, and the new fact (the overlay is searched at 60, the screenshot is Onyx's) holds. The Investigate carries a real split with a ruling on one side, so the user decides.

The old-record headings the records cite exist (A2.md 13, A3.md 15 and 22, A5.md 4, B1.md at `### 18.`, A21.md 30). One line is off: the B1 record's heading is `### 18.`, not `## 18.` as the new record's Delta line says; the number is right.

## Typed numbers

The auditor's list holds. Added: `test_activation_rows.cpp:592` (summary "3") and `:635` (`squeezed_out == 1`). I found no other typed value on an assertion line in the B cases that a 60 ms run can move. The grep for typed numbers I ran in `test_app_click.cpp`, `test_report_rows.cpp` and `test_app_reports.cpp` returned only: 857 (37), 1010 (5321.0), 1027 (1100.0), 360 (-315.8, a hand-built value), the tier edges at 774-783, 883, 897, `test_app_reports.cpp:491, 509, 637`. None is read from a search.

## Splits (one per line)

- One file runs two windows: `test_activation_rows.cpp` reads Burnout at 60 (582, 1331) and the same family of numbers at 85 elsewhere (`analyzed_like_the_gui`, line 1228).
- Case 1607 runs the whole corpus at 60 while its comments (1694, 1697) cite charts at 85.
- `tests/song_digest.h:332` pins digests at 85 while the app default is 60.
- The Dynamic Hit Window tag and the engine schedule's room margin answer one question differently (A3 record 22); not rechecked here.
- `test_preview_golden.cpp:304` draws a 60 ms path over a screenshot from Onyx.
- The WN4c file's "Not read line by line" list is long (about 80 cases). Those rows are sorted by two token scans plus their names, which the auditor admits. My own scan of the same files finds no search entry in them.

## Not reached

I did not run anything, did not open `note_window.cpp` or `tools/replay_schedule.cpp`, did not read the B bodies in the auditor's unread list, and did not open `test_path_list.cpp` or `uitest_paths.cpp` (record 1 question 10).

## Stopped on

Nothing.
