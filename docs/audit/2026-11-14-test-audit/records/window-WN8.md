# Window round, group WN8 (T5-WN8, agent aa3a405e0cfd2a777)

Read at HEAD 2727c4d9 of `claude/test-audit-records` (code of `5ec682c7`). Briefs: `briefs/T5-window.md` and `briefs/T5-window-resend.md` ("WN7 and WN8, the last gaps"). Files: `tests/test_dm_report.cpp`, `tests/test_dm_other_speed.cpp`, `tests/test_fill_report.cpp`, the `testdm::` cases of `tests/test_report_rows.cpp`, and the `batch_run()` cases of `tests/test_config.cpp`. I read every test body named below. None was sorted by a token scan alone.

## How the search is reached in this group

`testdm::sample_chart()` (`tests/dm_fixture.h:46-52`) runs `corpus::first_analyzed_with_paths` on a default `AnalysisSettings` with only `depth_value = 0` set. `AnalysisSettings` inherits `SearchSettings::early_fill_window_ms = kDefaultEarlyFillWindowMs` (`src/search/pather.h`, 60.0). So the fixture's one chart is searched at 60 ms. `testdm::fill_store` (`dm_fixture.h:75-90`) saves that record under `testdm::settings()`. That profile's lens takes `Settings::early_fill_window_ms` (`src/app/config.h:195`, `static_cast<int>(kDefaultEarlyFillWindowMs)`, so 60). Both defaults come from the one constant, so the record and the profile it is filed under agree today. No test in this group sets `early_fill_window_ms` to a value, so there are no A rows.

The first corpus chart with paths is the first entry of `corpus::chart_paths()` (sorted full paths) that analyzes to any path. By the sort order I expect `testdata/input/common/IB24/T1/Allister - Overrated/notes.mid`. I did not run it, so I cannot confirm that. No engine record in `records-engine/` covers it (they hold `beg`, `believer` and five fixtures), so every claim about that chart is "claim not checked".

`test_report_rows.cpp` reaches the search in three more ways. Its own `fill_store` (`:96-115`) analyzes the first `want` corpus charts with `analyze_chart_file` under `fixture_settings(cap)` (a default `Settings`, so 60). `corpus::charts_with_paths` is used at `:259` and `:692`. `corpus::analyzed` is used at `:386`, and `analyze_chart_file` at `:517` and `:653`. WN4c sorted this file B 30 without knowing `sample_chart` searches. Its reasons mostly hold. The one case I move to C is below.

`test_fill_report.cpp`'s `put()` (`:62-72`) takes `sample_chart().record` and overwrites the best path's `path`, `score`, `actcount` and the record's `notecount`. No assertion reads any other field the search made.

## Counts

| File | A | B | C | delta round | cases |
|---|---|---|---|---|---|
| tests/test_dm_report.cpp | 0 | 29 | 1 | 0 | 30 |
| tests/test_dm_other_speed.cpp | 0 | 4 | 0 | 0 | 4 |
| tests/test_fill_report.cpp | 0 | 22 | 0 | 0 | 22 |
| tests/test_report_rows.cpp (22 of its 30 cases: the ones that save through `testdm::` or read a search; the other 8 stay WN4c's B) | 0 | 21 | 1 | 0 | 22 |
| tests/test_config.cpp (the `batch_run()` cases) | 0 | 3 | 0 | 1 | 4 |

C verdicts: Keep 0, Rewrite 1, Remove 0, Investigate 1 (claim not checked only).

## tests/test_dm_report.cpp

All 30 cases. "F" means the case calls `testdm::fill_store`, so its `optimal` is `sample_chart().record.best_path().totalscore()`.

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 61 | collect_dm_rows joins scores to records and labels them | C | `CHECK(*rows[0].pct_h == 9956)` at `:91`; record 1 below |
| 102 | collect_dm_rows: no percent off 100% speed; store identity fallback | B | F. Asserts that `pct_h` is missing at speed 150 (`collect_dm_rows` sets it only at base speed, `src/app/dm_report.cpp:133`) and a stored name. No search number is asserted |
| 119 | dmbot JSON parsers handle canned payloads | B | canned JSON text into `net::parse_users_json` and friends; no record, no search |
| 171 | collect_dm_rows answers above-optimal beside the status | B | F. Scores are `optimal` plus or minus 5, and the asserted deltas are 5 and -5, read against the same record's `optimal` |
| 196 | dm comparison: a score's search text is folded and tag-free | B | one hand-built `DmReportRow`; no record |
| 205 | generate_dm_report: tally and framing behind one seam | B | F. Scores are `optimal` minus 1000, minus 10, equal and plus 5; the counts 2/1/1 follow from those offsets. The optimal only has to exceed 1000 |
| 242 | generate_dm_report hands over its rows, subtitle, footer, player and mode | B | F. Asserts statuses, and subtitle and footer text from `clone_hero_cap_phrase()` and the mode string |
| 282 | dm_tiles: the tiles count only the scores shown | B | typed rows (`row("under optimal", 2000, 9901)`); the tile text is a function of those rows, no record |
| 331 | status_token: each status's chip colour | B | status strings into a colour token; no record |
| 345 | collect_dm_rows: the stored names go through display_title, display_artist and display_charter | B | F. Asserts names from the library row; `optimal - 10` only picks "under optimal" |
| 368 | collect_dm_rows: a leaderboard row's DMBot names lose their Clone Hero tags | B | F. Asserts tag-stripped names |
| 392 | collect_dm_rows: a leaderboard row with no charter shows the library's, else (unknown) (D266 item 2) | B | F. Asserts a charter name |
| 433 | dm comparison: a percent rounds once | B | typed `actual 198010`, `optimal 198020`, `pct_h 9999` on a hand-built row; no record |
| 451 | dm comparison: the average tile reads a percent the way the cells do | B | typed 198010 of 200000 on a hand-built row; no record |
| 471 | collect_dm_rows: the % of opt column sorts on the shown hundredths | B | F. Asserts 10000, 10000 and 5000 hundredths (`:489-491`) for `optimal`, `optimal - 1`, `optimal / 2`. They hold for any optimal above about 20,000: one point under N is 100% minus 100/N percent, which rounds to 100.00% while 100/N is under 0.005. A window change moves a corpus chart's score by a small share, not down to that. Reasoned from `percent_steps` (`src/app/display_format.cpp:36`), not run. The assumption is that the first chart's optimal is far above 20,000; case 61's 9956 puts it near 227,000 |
| 666 | http transport: cancel aborts a request the server never answers | B | a loopback server and the WinHTTP transport; no record |
| 683 | http transport: cancel aborts a body that stops after the headers | B | same; no record |
| 714 | http transport: a 200 reply is read in full, a 404 keeps its message | B | same; no record |
| 740 | collect_dm_rows tells no current result from not in library | B | F. Asserts statuses and tally counts; `optimal - 1000` only picks "under optimal" |
| 777 | collect_dm_rows: a Ready record with no paths reads "no paths" | B | F plus `testdm::save_no_paths` (an empty record, no search). Asserts statuses and a counts phrase |
| 829 | why_not_comparable names the missing Clone Hero rule | B | F, but the join only throws (`make_score(kHash, 1)`); asserts sentences |
| 865 | why_not_comparable for settings asks about the Clone Hero cap profile | B | `testdm::settings` only; no record |
| 887 | clone_hero_cap_phrase and compare_tooltip name SP cap 4 | B | two literal strings; no record |
| 895 | why_not_comparable refuses Note Shuffle, and only Note Shuffle changes | B | F; the join throws, or returns one row for score 1; asserts sentences and `size() == 1` |
| 929 | collect_dm_rows refuses a non-Expert chart mode with the toolbar's sentence | B | F; the join throws; asserts the sentence |
| 948 | collect_dm_rows refuses a chart mode it cannot read | B | F; the join throws |
| 959 | collect_dm_rows: a database with no chart library stops with the path report's sentence | B | `save_no_paths`; no search |
| 975 | collect_dm_rows: a result whose chart the library doesn't list is left out | B | F. Statuses only; `optimal - 1` picks "under optimal" |
| 994 | collect_dm_rows reads the Clone Hero cap profile by name | B | F plus `sample_chart().record` copied to cap 8. Asserts statuses and `rows[0].optimal == optimal`; both sides are read from the one record, so they are equal at any window |
| 1030 | settings_change_touches: the comparison reads its chart mode and the lens, not the cap | B | `Settings` copies compared; no record. It never moves `early_fill_window_ms`; see "Gaps" |

## tests/test_dm_other_speed.cpp

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 27 | s2 offspeed: one base speed, read everywhere | B | the constant `net::kBaseSpeedPercent`; no record |
| 36 | s2 offspeed: an off-speed score on a listed chart is 'other speed'; on an unlisted one, 'not in library' | B | F. Scores are `optimal` plus 5 and minus 1000 at speeds 150, 75, 100; asserts statuses, `rows[0].optimal == optimal`, `rows[1].delta == 1000` (the typed offset, not a search number) and tally counts |
| 76 | s2 offspeed: the counts sentence names other speeds only when there are some | B | typed `DmReportStats` into a sentence; no record |
| 92 | s2 offspeed: the subtitle counts every score; the Other speed tile counts 'other speed' | B | F. Asserts the subtitle text, `other_speed == 2` and a tile "2"; scores relative to `optimal` |

## tests/test_fill_report.cpp

All 22 cases. `put` is `sample_chart().record` with the best path's path, score, acts and the notecount overwritten by typed values. `put_no_paths` is an empty record with no search.

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 117 | collect_fill_rows: delta sign and status literals | B | put. Every asserted number (1000000, 1050000, 50000, 3, 4, 1234) is typed over the record by `put` |
| 142 | collect_fill_rows: 1.0 higher and same | B | put; typed scores |
| 166 | collect_fill_rows: a chart with a record under one rule only still gets a row | B | put; typed scores |
| 195 | collect_fill_rows: a stored failure is no record on its side | B | put plus `failure_record`; typed scores |
| 215 | collect_fill_rows: a 1.0 record with no score and no 1.1 record | B | `put_no_paths`; no search |
| 232 | collect_fill_rows: a chart with records under both fill rules in one database gives one row, named from that database's library | B | put; typed scores and library names |
| 261 | tally_fill_rows counts every status | B | hand-built rows |
| 291 | collect_fill_rows: copies come from the library | B | put; typed scores; copies and counts from the library rows |
| 339 | collect_fill_rows counts each status through ChartIndex::count | B | put; typed equal scores |
| 369 | fill page: the tiles add up the copies tally_fill_rows adds up | B | hand-built rows; HTML text |
| 429 | build_fill_html substitutes every placeholder | B | put; HTML text |
| 476 | build_fill_html: text columns sort on the windows' folded text | B | hand-built row |
| 499 | build_fill_html: the count line's total is singular at one chart | B | hand-built row |
| 515 | build_fill_html colours and sums the delta from the status | B | HTML of an empty row set |
| 533 | report payload: the search field is folded and tag-free | B | hand-built row |
| 544 | generate_fill_report: tally and framing behind one seam | B | put; typed scores; sentences with cap and mode |
| 585 | collect_fill_rows: the names go through display_title, display_artist and display_charter | B | put; names |
| 601 | generate_fill_report: one chart reads "1 chart" in the subtitle | B | put; text |
| 611 | collect_fill_rows: a record under both rules with a score under one is in both | B | put and put_no_paths; typed scores |
| 646 | generate_fill_report: a score on one side only is counted, and the parts add up | B | put and put_no_paths; typed scores; counts from the typed rows |
| 684 | generate_fill_report: a database with no chart library stops with the path report's sentence | B | `put_no_paths`; no search |
| 697 | collect_fill_rows: a result whose chart the library doesn't list is left out | B | put and put_no_paths; typed scores |

## tests/test_report_rows.cpp

Judged here: the cases that save through `testdm::save` directly, through `fill_store`, `store_tied`, `fill_store_with_copies` or `check_cap`, or that run a corpus loop. The other 8 cases (`:755`, `:761`, `:786`, `:801`, `:887`, `:951`, `:962`, `:976`) read no stored or searched record and stay WN4c's B.

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 204 | report rows: a tied top-score variant is optimal too | B | `store_tied` saves `test::tied_variant_record()`, hand-built; no search |
| 230 | report rows list every stored record (4 bars) | B | `check_cap(4)`: `rank1 == added`, `rows >= added`, each title present. Counts are relative to the charts `fill_store` found with paths (`:141`); no search value is typed |
| 235 | collect_rows: the row list is sized to its rows | B | asserts `capacity() == size()` of a vector; `REQUIRE(fill_store(...) == 5)` counts charts that analyze, not a value |
| 253 | path report: a batch's records read back as a fresh analysis's rows | B | the expected rows come from `fixture_analyzer` at the same `run.settings` as the batch, so both sides are one search at one window. `page.songs == 6` is five charts plus the tied record, from `REQUIRE(library.size() == 5)` |
| 340 | path report: a saved figure below 0 shows no timing and the none tier | B | a hand-built `Activation` saved and read back; `e_offset 300.0` and `-315.8` are typed inputs |
| 376 | all_paths lists paths best first on every corpus chart | B | asserts `totalscore()` non-increasing within one run's paths (`:389`), an order inside one result that holds at any window |
| 395 | report counts every library copy of a chart, and leaves out a chart the library dropped | C | `REQUIRE(out.paths.size() > 1)` at `:419`; record 2 below |
| 430 | path report: the Charts shown tile counts what generate_report counts | B | `rows_h0 > 0` and `rows_h1 > 0` (`:440-441`) need one path per chart, which `fill_store` already requires. `songs == 5` and `library_charts == 5` come from the 2 + 3 library rows the helper writes |
| 451 | generate_report hands over its rows, subtitle and footer | B | asserts `rows > 0`, two readings of one record agreeing, "5 charts" from the library, and the footer text. The footer's "85 ms hit window" and "170 ms" come from `kHitWindowMs` (`src/app/report.cpp:92`, `src/core/model.h:106`), a constant separate from the early-fill window, so a 60 ms run cannot move them |
| 494 | path report coverage: shown + can't analyze + no result is the library count (corpus) | B | counts come from a parse failure (`NoNotesError` at Hard) and the typed `i % 7 == 6` skip. `songs > 0` needs one Hard chart to keep a path. The sum is arithmetic over the three parts |
| 548 | path report coverage: a chart with a stored failure gets no row and one list entry | B | `store_tied` plus stored failures; hand-built |
| 579 | path report coverage: the lines' words | B | `store_tied` plus stored failures; hand-built |
| 628 | path report coverage: no Ready chart but a stored failure reads Nothing to show | B | stored failures only; no search |
| 645 | report lists only the wanted cap and names it | B | `rank1 == 1` for one chart at cap 4, and the subtitle endings. The cap-8 record is a second row for the same chart. The only thing read from the search is that a path exists |
| 689 | generate_report reports a 1.0-fills database under the 1.0 rule | B | `charts_with_paths(legacy settings, 1)`; asserts the rule's name, `songs == 1`, that paths exist, and that the 1.1 page is empty. No path or score is asserted |
| 713 | collect_rows: the library names go through display_title, display_artist and display_charter | B | `store_tied`, hand-built |
| 737 | the report names a chart the scan found twice by its first copy | B | asserts the name "Zed" on every row and `songs == 2`; needs paths to exist |
| 812 | generate_report: one seam frames the report for every entry point | B | `songs == added` and `rows >= added`; subtitle and footer text (footer's 85 and 170 are the hit window, see 451) |
| 870 | generate_report: "every path" is the sentinel's, a huge --paths stays a count | B | subtitle text; needs one chart with paths |
| 902 | generate_report hands back nothing when its cancel flag is set | B | cancel flag; nothing from the search is read |
| 919 | a report with no analysis settings is refused | B | throws `std::invalid_argument`; nothing from the search is read |
| 930 | a report on results with no chart library says the library is missing | B | hand-built tied record; no search |

## tests/test_config.cpp: the `batch_run()` cases with no record in `records/delta-DL4.md`

DL4's headings are the cases at `:76`, `:104`, `:235`, `:254`, `:279`, `:370`, `:403` and `:990`. `:279` is its only `batch_run()` case.

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 172 | the 1.0 fills setting reaches the search and the result's key together | B | `batch_run()` only builds the settings bundle (`Settings::batch_run`); no search runs. Asserts `legacy_fills` on the analysis settings and the lens, not the early-fill window |
| 187 | command-line settings take the fill rule from the flag, the rest from the INI | B | `load_for_command_line`, then `batch_run().settings.legacy_fills` and `profile.lens.legacy_fills`; no search runs; the INI holds `sp_cap` and `legacy_fills`, no window |
| 279 | early_fill_window_ms reaches the search and the result's key together | delta round | `records/delta-DL4.md` heading 5; not judged again |
| 779 | batch_run bundles one Settings' profile and search settings | B | no search runs; asserts `run.profile == s.profile()` (the lens window travels equally on both sides), the cap, difficulty, depth and ms filter; none reads a search result |

## C records

```
Delta round: window, replaces records/A10.md record 1
collect_dm_rows joins scores to records and labels them
            (tests/test_dm_report.cpp:61)
Read:       lines 61-100; testdm::fill_store (tests/dm_fixture.h:75-90);
            testdm::sample_chart (:46-52); collect_dm_rows
            (src/app/dm_report.cpp, percent at :133); percent_steps
            (src/app/display_format.cpp:36); callers of collect_dm_rows:
            generate_dm_report (src/app/dm_report.cpp) and
            src/ui/dm_jobs.cpp; old record A10.md record 1.
Old verdict: Rewrite (A10: "L91 is a snapshot", drop the pinned 9956 and
            check pct_h against percent_steps). It still holds at 60 ms,
            and the window adds a reason: the number can move.
1 Claim:    When a leaderboard score at base speed is 1,000 points under the
            best score Hydra found for the chart, the row says "under
            optimal", carries the optimal and a delta of 1,000, and shows
            99.56%.
2 Owner:    collect_dm_rows works out status, delta and pct_h;
            percent_steps turns score and optimal into hundredths. Callers:
            the comparison window (via dm_jobs) and generate_dm_report.
            Owner found.
3 Real chart: claim not checked. The row's optimal is the best path's score
            of the first corpus chart with paths, searched at 60 ms
            (sample_chart, depth 0). No engine record of that chart exists
            in records-engine/, and none was made at 60 ms. 9956 holds only
            if 1000 / optimal lies in [0.00435, 0.00445), that is an optimal
            from about 224,700 to 229,900. The test's own comment says
            "read from one run". A different early-fill window changes the
            best path's score, so 9956 can move. The record the claim needs:
            hydra_replay dump of that chart at --efill 60.
4 Direct:   Statuses and deltas are read directly against the same optimal.
            Line 91 types the percent in hundredths with no working.
5 Reason:   9956 has its working only in the comment "read from one run".
            The 1,000 comes from the typed offset at :67.
6 Display:  The expected side of :91 is typed. The row's percent is a
            function of the score and optimal, both in the test, so the
            test can state it from them.
7 Failure:  No finding cited at :88-89. Not opened.
8 Deleted:  :91 deleted: percent_steps called with the wrong total (the
            delta instead of the optimal) goes unnoticed here (reasoned);
            :85 deleted: the join copying the wrong record's score;
            :79-82 deleted: a status word renamed.
9 Stand-in: none; the record is a real search result.
10 Same rule: tests/test_dm_report.cpp:471 asserts percents on the same
            fixture without typing the optimal.
11 Real:    Yes, a real chart; the number is that chart's and moves with
            the window.
Verdict:    Rewrite. State :91 as percent_steps(optimal - 1000, optimal,
            kPercentDecimals) read from the row's own inputs, as A10 asked.
Finding:    Typed number: 9956 at tests/test_dm_report.cpp:91 (comment
            :88-89), taken from one run of the first corpus chart.
```

```
Delta round: window, replaces records/A25.md record R7
report counts every library copy of a chart, and leaves out a chart the library dropped
            (tests/test_report_rows.cpp:395)
Read:       lines 395-425; fill_store (:96-115); fixture_settings (:46-55);
            corpus::charts_with_paths (tests/corpus_util.h:100-113);
            report::generate_report and path_tiles (src/app/report.cpp);
            old record A25.md R7.
Old verdict: Investigate, claim not checked only. It still holds at 60 ms.
1 Claim:    A chart in two library folders counts twice, a chart whose result
            has no library row is left out, and each row of the doubled
            chart says it has two copies.
2 Owner:    report::generate_report and the library's copy count
            (store::count_charts). Owner found.
3 Real chart: claim not checked. The counts (2 songs, 2 copies, "2 of 2")
            come from the library rows, not the search. One line reads the
            search: REQUIRE(out.paths.size() > 1) at :419, "h0 keeps more
            than one path". It holds when the first corpus chart with paths
            keeps two or more of its top ten scores at cap 4. A smaller
            early-fill window refuses more early fills (fill_refuses,
            src/core/model.h:331), so a chart can keep fewer distinct paths
            at 60 than at 85. No dump of this chart at 60 ms exists.
4 Direct:   Copy counts are read from the structure. The floor `> 1` states
            no source.
5 Reason:   The 1 in `> 1` is there so the loop at :420 is not over one row;
            the test does not say why one row would be wrong.
6 Display:  The tile text "2 of 2" is checked against the library count, not
            a typed run value.
7 Failure:  D76 and D87 item 4 in the comment (:396-398); not opened.
8 Deleted:  :419 deleted: nothing breaks if the loop ran over one row; a copy
            count read from the first row only passes (reasoned). :408
            deleted: count_charts counting copies once. :413 deleted: the
            library-less chart h1 leaking in.
9 Stand-in: none; h0 is a real analysis.
10 Same rule: tests/test_report_rows.cpp:430 and :737 count copies on the
            same fixture.
11 Real:    Yes.
Verdict:    Investigate, claim not checked only. The engine record the claim
            needs: hydra_replay dump of the first corpus chart with paths at
            --efill 60 (depth 10, cap 4), to show how many paths it keeps.
Finding:    Not a number from a run but a count floor, `> 1` at
            tests/test_report_rows.cpp:419, that can move at 60 ms.
```

## Typed numbers on assertion lines that a run at 60 ms can move

1. `tests/test_dm_report.cpp:91`: `CHECK(*rows[0].pct_h == 9956)`, and the comment at `:88-89` ("99.56% ... read from one run").
2. `tests/test_report_rows.cpp:419`: `REQUIRE(out.paths.size() > 1)`, a floor on how many paths the first corpus chart keeps.

Typed numbers I looked at and judge cannot move:
- `tests/test_dm_report.cpp:489-491` (10000, 10000, 5000 hundredths) hold for any optimal above about 20,000. Case 61's number puts the first chart's near 227,000.
- The offsets 1000, 10 and 5 from `optimal` (`:67-69`, `:211-214` and others) are typed inputs, not search numbers.
- `tests/test_report_rows.cpp:833-837`: the footer's "85 ms hit window" and "170 ms" come from `kHitWindowMs`, a constant separate from the early-fill window.
- `tests/test_fill_report.cpp`: 1000000, 1050000, 777000 and the like, and `notecount = 1234`, are typed over the record by `put`.

## Splits

1. `tests/test_fill_report.cpp:49-56` and `:62-72`: the fixture files `sample_chart().record`, searched under the 1.1 rule, as a Clone Hero 1.0 result (`record.legacy_fills = true`). The record's own search rule and the label it is filed under differ. No assertion reads the difference.
2. `testdm::sample_chart()` searches at a default `AnalysisSettings`, and `testdm::settings()` files the record under a `Settings` lens. Two objects each state the early-fill window, joined only because both read `kDefaultEarlyFillWindowMs`. They agree at 60 today. If either default changed alone, the record and its profile would disagree and no test here would say so.

## Gaps and what I did not do

- I ran nothing (agent rule 14), so both C records say "claim not checked".
- `settings_change_touches` in `src/app/dm_report.cpp` has no test that moves `early_fill_window_ms` alone (`tests/test_dm_report.cpp:1030` moves chart mode, cap, ms limit, fills and auto-open). Whether a window change marks the comparison out of date is untested. I note it and do not judge it here.
- For the three loopback HTTP cases (`:666`, `:683`, `:714`) I read the heads and the helper, not every line. None of them touches a record, so the B reason does not depend on the unread lines. The `:119` JSON-parser case I read to line 135 and the rest by its structure (canned strings into parsers); no record is involved.
- I did not re-read the other 8 `test_report_rows.cpp` cases WN4c sorted B; they read no stored or searched record.
