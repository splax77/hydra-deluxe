# Window round, group WN1 (T5-WN1, agent aefd8a581c918e130)

Files: `tests/test_library_query.cpp`, `test_search_target.cpp`, `test_rules.cpp`, `test_library_jobs.cpp`, `test_store_results.cpp`, `test_store_records.cpp`, and the helpers `tests/store_fixtures.h`, `tests/record_fixtures.h`. Code read at the worktree HEAD (holds `5ec682c7`'s code). Nothing was built or run.

## How the window reaches these tests (read once, used by every row)

- `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`). `SearchSettings::early_fill_window_ms` starts at it (`src/core/model.h:977`). `app::Settings::early_fill_window_ms` starts at `static_cast<int>(kDefaultEarlyFillWindowMs)` (`src/app/config.h:195`) and `to_analysis_settings` copies it (`src/app/config.cpp:411`). `ScoreGraph`'s constructor takes `early_fill_window_ms = kDefaultEarlyFillWindowMs` (`src/search/graph.h:198-201`), and the engine reads it from the graph (`src/search/engine.cpp:2030`, `:2176`).
- `fill_refuses` and `is_e0` are applied in `branch_activate` (`engine.cpp:1007`), in the group key (`:859-869`), in the ms filter (`:1224`) and when the path is built (`:1937`). They apply to the 1.0 fill rule too: the legacy flag does not bypass `branch_activate`.
- `store_fixtures.h::fixture()` (lines 27-33) sets cap 4, Scores depth 0 and no ms filter, and never the window, so it is the default 60 ms. `corpus_util.h::analyzed`, `first_analyzed_with_paths` and `analyzed_with_paths` pass the settings straight to `analyze_chart` or `analyze_chart_file` (`corpus_util.h:100-150`, `:237-258`). `record_fixtures.h::scores_settings` (line 582) is `app::Settings().to_analysis_settings()` with a cap and depth, so also 60.
- No test case in this group sets the window to a named value. So there are no A rows. Every case that runs the search runs it on the default 60 ms.
- Filed rulings (`docs/agents/rulings/code-and-engine.md`, `## Confirmed`, lines 55-57), quoted: "so we should use the hit window. and stop inventing our own numbers." / "make the early fill threshold an adjustable setting pinned to one number in both directions" / "analysis settings panel, default 60ms, supports 0ms to 85ms".
- Engine records: `records-engine/` holds seven dumps at `326cd4e0` under the fixed 85 ms rule (`index.md`). None is at 60 ms. Every C record below therefore says "claim not checked at 60" and names the dump it needs.

## Counts

| File | A | B | C | delta round |
|---|---|---|---|---|
| test_library_query.cpp | 0 | 31 | 0 | 0 |
| test_search_target.cpp | 0 | 5 | 7 | 0 |
| test_rules.cpp | 0 | 23 | 3 | 0 |
| test_library_jobs.cpp | 0 | 24 | 1 | 8 |
| test_store_results.cpp | 0 | 18 | 2 | 1 |
| test_store_records.cpp | 0 | 22 | 0 | 34 |
| store_fixtures.h, record_fixtures.h | helper table at the end | | | |

C verdicts: Rewrite 10, Investigate 3 ("claim not checked only"), Keep 0, Remove 0.

---

# tests/test_library_query.cpp (31 cases, all B)

Search reached: none. The only corpus call is `discover_charts({corpus::root()})` at line 98 (a scan that reads chart metadata). No case calls `analyze_chart`, `analyzed`, `search_target` or builds an `Engine` or `ScoreGraph`. The file's `analyzed(stars, hardest_ms)` at line 30 is a hand-built `RowFacts`, not a run. I searched the file for `analyze|search_target|ScoreGraph|Engine|corpus::|first_analyzed`: only `corpus::root` at lines 19 and 98.

| Line | Case | Kind | Reason |
|---|---|---|---|
| 61 | folding lowercases and removes accents | B | text fold of literal strings |
| 70 | folding turns full-width ASCII into ASCII | B | same |
| 76 | folding collapses whitespace and keeps other scripts | B | same |
| 83 | a mixed ASCII and non-ASCII string folds to its pinned text | B | same |
| 92 | the plain fold and the traced fold give the same text | B | compares two folds over chart titles read from the scan, never an analysis |
| 137 | invalid UTF-8 passes through byte for byte | B | text fold |
| 144 | only a real lead byte starts a kept character | B | text fold |
| 162 | rich-text tags are stripped and other angle brackets kept | B | text |
| 173 | a tagged charter is found by its plain name | B | query match on hand-built entries |
| 182 | an accented name is found without its accents | B | same |
| 189 | the report pages' fold table is the fold, one character at a time | B | table compare |
| 211 | the report pages' fold table holds every character the fold changes | B | table compare |
| 231 | words match in any order and across fields | B | query on hand-built entries |
| 244 | a quoted phrase must appear whole in one field | B | same |
| 266 | artist: charter: folder: and title: limit a term to one field | B | same |
| 300 | a term applies to its own column, and Any applies everywhere | B | same |
| 313 | unknown field names are words and empty field values are dropped | B | same |
| 327 | stars:N keeps analyzed rows with exactly N stars | B | `RowFacts` typed by hand (`analyzed(7, 163.0)`), so stars and squeeze are inputs, not engine output |
| 340 | squeeze<=N keeps analyzed rows whose hardest squeeze is at most N ms | B | same |
| 357 | a row at exactly N ms matches <=N and >=N, not <N or >N (D266) | B | same |
| 381 | squeeze filters: a path whose hardest is 0.0 ... | B | same |
| 391 | squeeze>N and squeeze>=N keep rows whose hardest timing is above N ms | B | same |
| 420 | a squeeze floor and limit combine into a range | B | same |
| 433 | an unknown squeeze operator gives a plain error | B | parse of query text |
| 455 | bad filter values give a plain error | B | same |
| 488 | an empty query matches every row | B | same |
| 501 | highlight spans cover the displayed bytes | B | text |
| 543 | a chart with no album matches nothing through its album | B | hand-built entry |
| 554 | the report pages' search text holds no album | B | text |
| 561 | every Search tips example parses into the key its entry names | B | parse |
| 630 | matching 20000 rows finds the pinned hit count | B | rows built by the test itself; no search |

---

# tests/test_store_records.cpp (56 cases)

Search reached: through `store_fixtures.h::fixture()` (one corpus chart, default 60 ms; see the helper table) and `corpus_analyses()` (line 59; every corpus chart under `SearchSettings{}`, default 60 ms). Every non-delta case saves that record into a store and reads it back, or compares the store's answer with the same record or with a second store answer. No non-delta case types a value the search produced. Common B reason: each assertion compares the store's output with the record the search handed in (or with a status the test set by hand: old-build stamp, other rules, failure record), so any window gives the same verdict. The one shape that could move is `REQUIRE_FALSE(paths.empty())` on the fixture (lines 566, 946). A window cannot turn the fixture into a chart with no paths, because `first_analyzed_with_paths` (`corpus_util.h:132`) picks the first chart that does have paths under the run's own settings.

Delta round (in `records/delta-DL1.md`, not judged here): lines 124, 143, 157, 171, 178, 188, 215, 231, 250, 260, 280, 334, 352, 376, 390, 416, 430, 456, 503, 521, 536, 591, 608, 699, 724, 753, 917, 963, 1005, 1023, 1113, 1206, 1288, 1328 (34 cases).

| Line | Case | Kind | Reason |
|---|---|---|---|
| 130 | an in-memory store reads through itself | B | compares the store's reader to the store, on `fixture().record` |
| 557 | a saved analysis reads back Ready with its paths in rank order | B | every expected value is read from the same record (`analysis.all_paths().size()`, `best.pathstring()`, `best.totalscore()`); rank order is the store's |
| 578 | prepare_record refuses a profile the analysis did not run under | B | cap and lens come from the record's fields; the window is not in these three profiles |
| 641 | every path of one analysis carries the same note count | B | `Path::notecount` is assigned from the graph's chart-wide count (`engine.cpp:2007`), not per path |
| 652 | a save under the same profile replaces the record and every path | B | paths are rewritten by hand (lines 656-663) |
| 676 | a lookup after a second save reads the second record | B | second record built by editing the first |
| 785 | a record saved under another build's stamp reads back Stale | B | stamp set by hand |
| 801 | a save keeps a readable record made under other rules | B | `other_rules_record` edits the fingerprint |
| 819 | record_count counts every real record | B | counts saves of one record |
| 841 | the remembered failures are exactly the four... | B | no search; error kinds only |
| 871 | a chart with no notes throws NoNotes | B | no search; the error is thrown by hand |
| 881 | a failure saves and reads back CantAnalyze | B | no search; failure record |
| 904 | a failure saved under another build's stamp reads Stale | B | no search |
| 940 | a real save replaces a failure, and a failure replaces a real record | B | compares the stored record with the one saved |
| 975 | settled_song_ids lists exactly lookup's Ready and CantAnalyze charts | B | statuses set by hand; the record is only a payload |
| 1039 | lookup's three answers against sibling profiles | B | same |
| 1067 | ready_song_ids and ready_records list exactly lookup's Ready charts | B | the order "high, low" comes from `low.score -= 100` (line 1071), not from the search |
| 1094 | a save inside a save group is one SAVEPOINT | B | a duplicated path row makes the save fail (line 1099) whatever the path is |
| 1185 | a record's best path text is rank 1's | B | hand-typed path rows; the one search value is `no_paths_at_cap(4)`, empty by construction |
| 1267 | a record an old migration filed with unknown settings reads Not analyzed | B | `at_cap(8)` is the fixture record relabeled; the test is about `ms_enabled = -1` |
| 1354 | a Stale lookup says why | B | statuses |
| 1384 | a current record with no paths is Ready, not Stale | B | `at_cap(4)` at lines 1401-1402: the check is `has_scored_best_path()` on a record that has paths; true at any window as long as the fixture has paths (see above) |

No C records.

---

# tests/test_store_results.cpp (21 cases)

Search reached: `corpus::analyzed(path, settings)` in the matrix (line 124, default window), `corpus::first_analyzed_with_paths` (line 173), and `fixture()` / `at_cap` everywhere else. Delta round: 339 (`delta-DL1.md`, "Profile compares on every part of the identity").

| Line | Case | Kind | Reason |
|---|---|---|---|
| 103 | a saved record reads back whole across the corpus and config matrix | B | 9 settings x every chart on the default window; the check is `got.record == prepared.record` per record (line 156); nothing typed. The `Config::legacy_fills` field is declared and never set |
| 167 | RecordStore maintenance: a lookup, the Ready list and the record count | B | expected values read from the same record (`record.best_path().pathstring()`, line 186) |
| 214 | records at different caps coexist | B | line 244 compares with `fixture().record.best_path().totalscore()`, the same record; caps are relabeled |
| 247 | the listing and a lookup agree on which record is a chart's answer | B | statuses by hand |
| 295 | prepare_record refuses a profile whose ms limit isn't the analysis's | B | `at_cap_ms10` sets `ms_limit` by hand |
| 312 | prepare_record files the record under the analysis's own rules fingerprint | B | a fingerprint of the rules, not of the window |
| 322 | the rules_fp column holds a fingerprint's bytes as one run wrote them | B | fingerprint typed by hand (`0x0102...`) |
| 339 | Profile compares on every part of the identity | delta round | |
| 370 | the same chart at the same cap keeps one result per lens | B | relabeled records |
| 400 | a current-version save purges the chart's old-version records | B | relabeled records |
| 438 | 1.0 and 1.1 results for one chart sit side by side | B | `legacy_fills` set by hand |
| 460 | prepare_record refuses a profile that names the other fill rule | B | same |
| 468 | lookups on one thread and saves on another | B | compares reads with the two saved records |
| 511 | ready_song_ids names exactly the charts lookup reads as Ready | B | statuses |
| 535 | ready_song_ids names exactly the charts a library page reads as Ready | B | statuses |
| 555 | lookup answers a page the same as lookup_one | B | lines 581-583 compare with `fixture().record.best_path().pathstring()`, the same record |
| 591 | a saved record stores its best path's star count | **C** | record W1 |
| 625 | the saved hardest figure is the engine's, sign included | B | hand-built `Activation` and `Path`; `record_with_sqin` never calls the search |
| 653 | lookup answers a whole library in chunks | B | `stars.has_value()` on the saved fixture; fake ids |
| 689 | PathSummary equality covers every field | B | typed `PathSummary`, no search |
| 728 | a record counts every kept path once | **C** | record W2 |

### W1. Delta round: window, replaces A34 R17 (a saved record stores its best path's star count) (tests/test_store_results.cpp:591)

Read: 591-623; `fixture()` (`store_fixtures.h:27-33`); `engine_hardest_ms` and `HydraRecord::best_path` (`model.h`); the old record `records/A34.md:283-299`. Old verdict: Rewrite ("605 states a result of the first corpus chart, not a rule"). It still holds, and the window sharpens the reason.
1 Claim: the library's saved summary of a chart holds the best path's stars, score and timing; a stale or never-analyzed chart carries none. Line 609 (comment 606-608): "the fixture's best path has no squeeze and no E0 fill, so none".
2 Owner: `path_stars` (stars), `engine_hardest_ms` (the figure), `prepare_record` (store). Unchanged from A34.
3 Real chart: the first corpus chart that yields paths, searched at cap 4, Scores depth 0, default window 60. Claim: its best path has no squeeze and no E0 fill. E0 is `e_offset < window_ms && skips == 0` (`model.h:335`), so the window is in the claim. "Pinned from one run" (line 607) means a run at 85. No engine record exists at 60. Claim not checked at 60. Needs: `hydra_replay dump --efill 60` (and `schedule` on path 0) of that chart. The test does not name the chart; the rewrite should.
4 Direct: 598-611 and 621-622 compare with `path_stars(best)`, `best.totalscore()` and `best.difficulty()` (owner's answers, window-free); 609 `CHECK_FALSE(hardest_ms.has_value())` is a typed fact about one chart.
5 Reason: 609 has none except the one-run comment.
6 Display: the Library's Timing column reads this figure. Not captured.
7 Failure: bc01fa6f (stars kept in the record).
8 Deleted: 609: a figure invented for a path with no squeeze or E0 (reasoned). The rest as in A34.
9 Stand-in: 609 stands in for "the saved figure is `engine_hardest_ms(best)`" (A34's own fix).
10 Same rule: none new.
11 Real: yes.
Verdict: Rewrite (old verdict holds at 60). Replace 609 by `summary().hardest_ms == engine_hardest_ms(best)`. Typed value that can move: the "none" at line 609.

### W2. Delta round: window, replaces A34 R21 (a record counts every kept path once) (tests/test_store_results.cpp:728)

Read: 728-735; `record_of`; `HydraRecord::all_paths`; the old record `records/A34.md:348-362`. Old verdict: Rewrite (compare with `all_paths().size()`, not a literal). Holds.
1 Claim: `pathcount` equals the number of paths the Paths tab lists; the fixture's count is 1.
2 Owner: `all_paths` (`model.h:988`).
3 Real chart: line 732 `pathcount == 1` is a literal "from one run" (comment, line 730) of the fixture's search at depth Scores 0. The number of kept paths depends on ties and on which fills the search may take, so on the window. Claim not checked at 60. Needs: `hydra_replay dump --efill 60` of the fixture chart (the same chart as W1).
4 Direct: 732 literal (snapshot); 734 `== 3` on a hand-built `tied_variant_record()` (not a search).
5 Reason: 732 none but the run.
6 Display: Paths tab count. Not captured.
7 Failure: finding 172.
8 Deleted: 732 a changed count (snapshot); 734 variants not counted (reasoned).
9 Stand-in: the literal 1 stands in for `all_paths().size()`.
10 Same rule: A34 R21 only.
11 Real: yes.
Verdict: Rewrite (holds). `record_of(...).pathcount == record.all_paths().size()` on the fixture and the tied-variant record. Typed value that can move: `1` at line 732.

---

# tests/test_rules.cpp (26 cases)

Search reached: lines 282-283 (`ScoreGraph(song, 4, Ch11, r)` and `run_search`, default window), 305-306, 329, 358, 395 (`corpus::analyzed`, default window), 420-422, 500-502 (`analyze_chart`). Cases at lines 67-259 and 430-494 read or write rules text and fingerprints and never reach the search.

| Line | Case | Kind | Reason |
|---|---|---|---|
| 67 | defaults are the values Hydra always used | B | rule values |
| 77 | a missing file and an empty file both load the defaults | B | file parse |
| 85 | every key in the file is read | B | file parse |
| 102 | a # starts a comment anywhere on a line | B | file parse |
| 111 | a bad value or an unknown key is an error that names the key | B | file parse |
| 140 | a whole-number key takes a whole value written as a decimal | B | file parse |
| 149 | no rules value has the no-rules fingerprint | B | fingerprint |
| 159 | whole_chord takes every note's SP doubling on a squeeze-out | B | hand-built row |
| 175 | the leeway decides whether a plain backend row is counted | B | hand-built row |
| 185 | backend_row_timing | B | hand-built row |
| 212 | summarystr: every counted plain row reads Normal | B | hand-built row |
| 223 | summarystr: one rule for the squeezed-out and the plain row | B | same |
| 235 | summarystr: an uncounted squeezed-out row carries the tag | B | same |
| 250 | summarystr: the tier edges for both row kinds | B | same |
| 269 | the leeway changes what the engine counts | B | runs the search on every corpus chart at the default window, but every check compares two runs at the same window (`s_none <= s_default <= s_wide`, lines 288-289) or asks that some chart moved (line 295). The leeway acts on counted backend rows, not on fills. The existential would fail only if every chart lost its change; the code shows no route to that. A verifier should look at this reason |
| 298 | max_tied_paths caps the tied paths the engine keeps | B | `tied_pathcount() == 1` under a limit of 1 on the first five charts: a property of the tie fold (`engine.cpp:1273-1314`), same at any window |
| 318 | max_tied_paths is one count per score, inside paths first | **C** | record W3 |
| 349 | a path over the Path limit stays kept when it ties the optimal score | **C** | record W4 |
| 384 | a path over the Path limit below the best score is dropped even as a tie | **C** | record W5 |
| 415 | the engine's tied count matches the recount | B | `REQUIRE_NOTHROW(run_search)` and `tied_pathcount() >= 1` on five charts; no typed value; the recount throws on any disagreement at any window |
| 430 | the generated-fill values come from the rules | B | rules values |
| 443 | the retired fill_length_measures key is read and ignored | B | file parse |
| 456 | the retired Auto keys are read and ignored | B | file parse |
| 470 | the fingerprint text is pinned | B | fingerprint of the rules |
| 487 | the default stamp is built once and matches a fresh record | B | stamp |
| 496 | a run is stamped with the rules fingerprint at cap 4 and at cap 8 | B | `rules_fingerprint` is the rules' hash; the window is not part of it |

### W3. Delta round: window, replaces A26 RU17 (max_tied_paths is one count per score, inside paths first) (tests/test_rules.cpp:318)

Read: 314-345; `records/A26.md:354-368`. Old verdict: Rewrite (keep 342; state inside-first as a property; drop the typed string at 344). Holds.
1 Claim: on "black midi - Sugar", 1.0 fills, 0 ms limit, tie limit 1: one count at the top and the best path reads `0 E3+ E5 E1`.
2 Owner: the engine's tie fold (`engine.cpp:1273-1320`).
3 Real chart: Sugar is real; no dump of it exists in `records-engine/` at any window. The string has three E marks, each an `is_e0` (`model.h:335`) result, which depends on the window. The comment at 316-317 says the E0 path's early fill is what puts it over the 0 ms limit. The figure of that fill is `-e_offset` and does not move with the window, but whether the fill is allowed (`fill_refuses`) and marked E does. Claim not checked at 60. Needs: `hydra_replay dump --efill 60` of Sugar with legacy fills, ms 0, tie limit 1.
4 Direct: 342 and 343 properties; 344 typed string.
5 Reason: 344 none.
6 Display: no.
7 Failure: finding 95.
8 Deleted: 342 per-side counting; 344 any change to the kept path.
9 Stand-in: 344 stands in for "the inside path is the one kept".
10 Same rule: W4, W5.
11 Real: yes.
Verdict: Rewrite (holds). Typed value that can move: `"0 E3+ E5 E1"` at line 344.

### W4. Delta round: window, replaces A26 RU18 (a path over the Path limit stays kept when it ties the optimal score) (tests/test_rules.cpp:349)

Read: 347-378; `records/A26.md:369-383`. Old verdict: Rewrite (find the over-limit tied path from the record's fields; no score or string typed). Holds.
1 Claim: on HopH2O "I Am... All Of Me" with ms 10: top score 694985; `0+ 2 0 0- 2 E0` ties it with difficulty 78.9 ms and is kept; `3 E0 1 0- 2 E0` is not kept.
2 Owner: the engine's Path-limit keep rule.
3 Real chart: real, no dump. Typed: 694985 (362), the two strings (370, 371), `"78.9 ms"` (373). The 78.9 ms is an early-fill difficulty (`early_fill_difficulty(e_offset) = -e_offset`), and E0 needs `e_offset < window`. A path whose early fill is refused at 60 but allowed at 85 would drop out of the tie. Claim not checked at 60. Needs: `hydra_replay dump --efill 60` of HopH2O at cap 4, ms 10.
4 Direct: 362 snapshot; 370-377 typed strings and `"78.9 ms"`.
5 Reason: none.
6 Display: no.
7 Failure: finding 330.
8 Deleted: 377 over-limit below-top path kept; 376 over-limit tie dropped.
9 Stand-in: typed strings.
10 Same rule: W3, W5.
11 Real: yes.
Verdict: Rewrite (holds). Typed values that can move: `694985` (line 362), `"0+ 2 0 0- 2 E0"` and `"3 E0 1 0- 2 E0"` (370-371), `"78.9 ms"` (373).

### W5. Delta round: window, replaces A26 RU19 (a path over the Path limit below the best score is dropped even as a tie) (tests/test_rules.cpp:384)

Read: 380-410; `records/A26.md:384-398`. Old verdict: Rewrite. Holds.
1 Claim: on "Unbound (The Wild Ride)" with 1.1 fills, ms 10, tie limit 4: `2 2+ 3 0+` at 859580 is kept and `2 2+ 0- 3` (428.6 ms) is not.
2 Owner: the Path-limit keep rule in the engine.
3 Real chart: real, no dump. 859580 and both strings (405-406) come from a run at 85. Neither string has an E mark, but the score and the existence of both paths depend on which fills the search may take. Claim not checked at 60. Needs: `hydra_replay dump --efill 60` of Unbound at cap 4, ms 10, tie limit 4.
4 Direct: typed strings and a typed score.
5 Reason: none.
6 Display: no.
7 Failure: D55 item 5.
8 Deleted: 409 over-limit path re-filed as a tie; 408 inside path lost.
9 Stand-in: typed strings.
10 Same rule: W3, W4.
11 Real: yes.
Verdict: Rewrite (holds). Typed values that can move: `859580`, `"2 2+ 3 0+"`, `"2 2+ 0- 3"` (lines 405-406).

---

# tests/test_library_jobs.cpp (33 cases)

Search reached: fixtures only. `corpus::first_analyzed_with_paths(AnalysisSettings{})` at lines 382, 466, 482, 499, 562, 588, 623, 683 and 774 hands one real result to a test analyzer (`set_analyzer_for_test`); the batch job never runs the search in those cases. `write_old_build_library` (line 846) calls `save_as_old_build`, which calls `corpus::analyzed(chart, settings.to_analysis_settings())` (`store_fixtures.h:79`) on Believer with a default `app::Settings` (window 60, `config.h:195`). The startup open (`StoreOpenJob`) then replays the saved path (`fill_saved_hardest`, `src/app/saved_figure_fill.cpp`) at that profile's window.

Delta round (in `records/delta-DL4.md`): 464, 481, 498, 524, 560, 587, 622, 682.

| Line | Case | Kind | Reason |
|---|---|---|---|
| 120 | a scan that throws finishes with its error | B | no analysis |
| 135 | the batch clock leaves paused time out | B | fake charts; the analyzer is a test lambda that throws (`gated_failure`, lines 94-103) |
| 157 | time left is the rate over the last 2W finished charts | B | same |
| 182 | time left shows nothing until 2W charts have finished | B | same |
| 193 | the batch strip's time left waits for two rounds | B | same |
| 215 | time left leaves a pause out of its window | B | same |
| 243 | pause lets the chart in flight finish | B | `gated_failure` throws |
| 271 | stop while paused ends the run | B | same |
| 294 | the snapshot names the chart being analyzed | B | same |
| 319 | the progress line's artist reads (unknown) | B | text |
| 336 | a failed chart reads in plain words | B | thrown error |
| 360 | a batch that fails as a whole | B | thrown error |
| 378 | the snapshot's counts come from the batch in one piece | B | the test analyzer returns `real` unchanged for two charts; the assertions are counts of charts (analyzed 2, failed 1) |
| 464 | answered_count ... | delta round | |
| 481 | a batch job's tray holds the store's answer | delta round | |
| 498 | a chart in two folders is asked about once | delta round | |
| 524 | a failed chart whose failure was stored | delta round | |
| 560 | the tray is empty after it is taken | delta round | |
| 587 | a paused batch job's tray keeps its answers | delta round | |
| 622 | a lookup that fails leaves its sentence in the tray | delta round | |
| 682 | a frame's snapshot does not wait while the batch job waits on the store | delta round | |
| 722 | allocator: malloc goes to mimalloc | B | no search |
| 773 | a finished batch hands the memory its charts freed back to Windows | B | `real` is a result the analyzer lambda copies (lines 739-742); the asserts are about memory |
| 785 | a report job lists the records filed under its run | B | profile compare |
| 796 | an empty report shows generate_report's reason | B | `save_batch_record` (hand-built `empty_analysis_under`, `display_fixtures.h:55`) |
| 856 | the startup open reads saved paths with its own words, then the library | B | the step sequence depends on one row missing its hardest figure: the helper empties rank 1 itself (`{1}`, line 851), and the fill marks the profile done whether or not it found a figure (`saved_figure_fill.cpp:151`). No assertion reads a figure |
| 937 | a fill that fails still opens the library and leaves its sentence | **C** | record W6 |
| 1000 | a stop while the startup open reads saved paths ends it as a cancel | B | stops before any write; `rows_missing_hardest(...).size() == 1` (line 1016) holds because the helper emptied rank 1, not because of the search |
| 1108 | results read in slices match one read | B | `save_batch_record`, hand-built |
| 1274 | timing: the startup library read by part | B | runs only with `HYDRA_TIMING_DB`; no search |
| 1291 | a cancelled leaderboard fetch is not an error | B | no search |
| 1332 | a failed leaderboard fetch says what to do | B | no search |
| 1349 | a failed job records the raw text | B | no search |

### W6. Delta round: window, replaces A16 record 19 (a fill that fails still opens the library) (tests/test_library_jobs.cpp:937)

Read: 819-852 and 937-967; `StoreOpenJob::run` (`src/ui/library_jobs.cpp:190-240`); `fill_saved_hardest` (`src/app/saved_figure_fill.cpp:112-155`); `RecordStore::set_hardest` (`record_store.cpp`, `UPDATE record_paths ... WHERE hardest_ms IS NULL`); `records/A16.md:334-349`. Old verdict: Keep (reasoned).
1 Claim: when the fill's write is refused, the job is still ok, `library_problem()` equals the DatabaseWrite sentence, the library was read to its last step, and the marker stays unset.
2 Owner: `StoreOpenJob::run`.
3 Real chart: the claim needs the fill to have a row to write. The helper's comment (lines 844-845) says "Its best path carries a squeeze-in at exactly 0.0 ms, so the fill has that row to replay." The write happens only when the replay of the saved path text matches and gives a figure (`saved_figure_fill.cpp:130`, `if (!p.figures.empty()) store.set_hardest(...)`). The trigger at lines 946-948 fires only on an UPDATE. If Believer's best path at 60 ms has no squeeze-in, or a different path leads, there is no UPDATE, no failure, and line 957 fails. At 85 the engine record has it: `records-engine/index.md` line 25, Believer path 0 "1 0+ 0 0" (354615) squeezes in at activation 1 (tick 57600), dumped at 326cd4e0 under 85. No record at 60. Claim not checked at 60. Needs: `hydra_replay dump --efill 60` of Believer at the default `app::Settings` cap and depth, and its path 0 `schedule` showing the squeeze-in and its offset.
4 Direct: 956 ok; 957-958 sentence equals `plain_error(KindedError(DatabaseWrite))` (owner's answer); 962 last step; 964 marker unset.
5 Reason: no numbers typed in the test; the number is in the helper's comment.
6 Display: not a display.
7 Failure: b51ce66e.
8 Deleted: as A16. Added: if the fill finds no figure, 957 fails; that shows as red, not as a silent pass.
9 Stand-in: none.
10 Same rule: the helper is shared with lines 856 and 1000, which do not depend on the figure (B above).
11 Real: yes.
Verdict: Investigate. Claim not checked only (Believer's best path at 60 ms). Typed claim that can move: "squeeze-in at exactly 0.0 ms" (comment, line 844).

---

# tests/test_search_target.cpp (12 cases)

Search reached: `search_target(song, cfg, ...)` and `corpus::analyzed(chart, cfg)` with `cfg` from `scores_settings(fc.cap)` (fuzz charts) or `app::Settings().to_analysis_settings()` (corpus); both 60 ms. Lines 405, 427 and 456 build `ScoreGraph graph(song, kCloneHeroSpCap)` with no window, so that graph is at the constructor's default 60, not at `cfg`'s.

| Line | Case | Kind | Reason |
|---|---|---|---|
| 104 | a path missing a named activation is dropped, the rest kept | **C** | record W7 |
| 141 | a variant that took every named activation outlives its dropped root | **C** | record W8 |
| 198 | a kept root's tied variant that missed a named activation is dropped | **C** | record W9 |
| 225 | keep_target_paths: every kept path and tied variant took exactly the named ticks | B | hand-built `made(label, ticks)` paths (lines 78-87); no search |
| 274 | every corpus path pinned in full comes back alone, the same | B | both sides of every compare come from runs at `cfg`'s one window: the record (`analyzed(chart, cfg)`) and the pinned search (`search_target(song, cfg, ...)`); no value typed; only `paths > 0` |
| 299 | a full pin no path realizes names its window and why | **C** | record W10 |
| 345 | activation pins return the tick search's paths and its failing tick | **C** | record W11 |
| 370 | the binary search names the last or a middle window that breaks | **C** | record W12 |
| 397 | with nothing pinned the search is the plain search | B | compares two searches on one graph and window (lines 409-410); nothing typed |
| 420 | fills past the last pin branch, fills before it follow the pins | **C** | record W13 |
| 451 | a pinned window over the Path limit survives a soft and a hard limit | B | properties of whichever chart the loop picks (lines 453-462). The final `FAIL` needs one corpus chart whose best path fails a 0 ms limit; the corpus gives that at any window the setting allows (reasoned, not shown; a verifier should look) |
| 500 | keep_prefix_paths keeps a prefix match | B | hand-built paths |

A fact shared by W7 to W13: the fuzz charts (`fast_tempo_charts.h`, `fuzz_chart(5)` and `fuzz_chart(199)`) have no engine record at any window in `records-engine/`.

### W7. Delta round: window, replaces A27 ST1 (a path missing a named activation is dropped) (tests/test_search_target.cpp:104)

Read: 91-129; `records/A27.md:239-255`. Old verdict: Rewrite (keep 121-128; replace snapshots by "the kept set equals the unfiltered search's paths whose ticks equal the named ones"). Holds.
1 Claim: fuzz seed 5, told to activate at 3168 and 12864, returns exactly six paths with a typed text, and `0+- 0++` is the promoted one.
2 Owner: `search_target` / `keep_target_paths`.
3 Real chart: a fuzzed chart; no dump. The pinned text has E0 marks (`0++- E0++ 7350`, `0++- E0- 7150`) and totals; E0 is `e_offset < window`. The comment (line 95) says "pinned as read from one run". Claim not checked at 60. Needs: `hydra_replay dump --efill 60` of fuzz seed 5 (written to a file from `fuzz_chart(5)`), cap `fc.cap`, Scores depth 40.
4 Direct: 113 `kept.size() == 6`; 114-117 text; 119 promoted flags; 121-125 ticks property; 128 unrealizable tick.
5 Reason: 113-119 none but the run.
6 Display: no.
7 Failure: D45.
8 Deleted: 121-125 a path with a missing activation kept (reasoned); the typed lines catch any change.
9 Stand-in: typed text for the set rule.
10 Same rule: W8, W9, W10, W11.
11 Real: fuzz only.
Verdict: Rewrite (holds). Typed values that can move: `6`, the six path strings and totals (113-117), the promoted vector (119).

### W8. Delta round: window, replaces A27 ST2 (a variant that took every named activation outlives its dropped root) (tests/test_search_target.cpp:141)

Read: 131-190; `records/A27.md:256-271`. Old verdict: Rewrite (keep 160-164 and 177-181; drop totals, text and windows string; move 183-189). Holds.
1 Claim: with one more note in seed 5, the same six paths with higher totals; `0+- 0++` at 7750 is promoted; its windows text equals the full search's.
2 Owner: `search_target`.
3 Real chart: fuzzed; no dump. Typed: 7750 (162, 180), the text (154-157) and the long windows text (169-174, including `e 1250` and `e 127.5`). E marks as in W7. Claim not checked at 60. Needs: the dump named for W7 with the extra note.
4 Direct: 154-157, 162, 169-174 typed; 163-164, 177-181 properties.
5 Reason: none for the typed lines.
6 Display: no.
7 Failure: D45.
8 Deleted: as A27.
9 Stand-in: the windows text stands in for "the promoted path's windows are its own".
10 Same rule: W7.
11 Real: fuzz only.
Verdict: Rewrite (holds). Typed values that can move: `7750`, the six-path text and the windows text (lines 154-157, 162, 169-174, 180).

### W9. Delta round: window, replaces A27 ST3 (a kept root's tied variant that missed a named activation is dropped) (tests/test_search_target.cpp:198)

Read: 192-221; `records/A27.md:272-287`. Old verdict: Rewrite (keep 210-218; replace 220 by a count rule; drop 207-209). Holds.
1 Claim: fuzz seed 199 at ticks 4896, 12288, 21792 returns six roots (text at 207-209) with tie counts `{1,1,2,1,1,1}`.
2 Owner: `search_target`.
3 Real chart: fuzzed; no dump. E marks in `0++- E0++- 0` and others, tie counts pinned (220). Claim not checked at 60. Needs: `hydra_replay dump --efill 60` of fuzz seed 199.
4 Direct: 207-209 text; 213-217 properties; 220 counts.
5 Reason: none.
6 Display: no.
7 Failure: D45 addendum.
8 Deleted: 213, 216 a variant with a missing tick kept.
9 Stand-in: typed text and counts.
10 Same rule: W7.
11 Real: fuzz only.
Verdict: Rewrite (holds). Typed values that can move: the text at 207-209 and the counts at 220.

### W10. Delta round: window, replaces A27 ST6 (a full pin no path realizes names its window and why) (tests/test_search_target.cpp:299)

Read: 295-339; `records/A27.md:319-333`. Old verdict: Investigate, claim not checked only.
1 Claim: from seed 5's path `0+- 0-`, a full pin comes back alone; a moved end, or a flipped squeeze-out answer, comes back empty and names tick 12864 with reason `window_end` or `sqout`.
2 Owner: `search_target` / `pinned_windows`.
3 Real chart: fuzzed; no dump. The path `0+- 0-` is looked up by name (line 305) and must exist. It is one of W7's typed paths (`0+- 0- 6950`), so its existence at 60 is the same open fact. Claim not checked at 60. Needs: the seed 5 dump at 60.
4 Direct: 312-316 realized; 322-325 and 335-338 failure answers; ticks 12864 and 22848 are chart ticks.
5 Reason: 12864 is the chart's second activation tick (the test's own comment, line 296).
6 Display: no.
7 Failure: Track R2.
8 Deleted: each failure reason.
9 Stand-in: none beyond the named path.
10 Same rule: W11, W12.
11 Real: fuzz only.
Verdict: Investigate. Claim not checked only (the path `0+- 0-` at 60 ms). Typed value that can move: the path name `"0+- 0-"` (line 305).

### W11. Delta round: window, replaces A27 ST7 (activation pins return the tick search's paths and its failing tick) (tests/test_search_target.cpp:345)

Read: 341-364; `records/A27.md:334-349`. Old verdict: Rewrite (drop 354-356, true by construction; keep 358-363; add a real-chart capture).
1 Claim: pins and ticks give the same result; pins `{3168, 12865}` fail at 12865 with `activation`.
2 Owner: `search_target`.
3 Real chart: fuzzed; no dump. Line 356 `realized_prefix == 2` needs both 3168 and 12864 to be realizable at 60, the same fact W7 pins. Claim not checked at 60. Needs: the seed 5 dump at 60.
4 Direct: 354-355 compare two answers of one function; 356 and 361-363 typed.
5 Reason: 12865 is "one tick past a fill node" by comment.
6 Display: no.
7 Failure: Track R2.
8 Deleted: 354 true by construction.
9 Stand-in: none.
10 Same rule: W7.
11 Real: fuzz only.
Verdict: Rewrite (holds). Typed value that can move: `realized_prefix == 2` (line 356).

### W12. Delta round: window, replaces A27 ST8 (the binary search names the last or a middle window that breaks) (tests/test_search_target.cpp:370)

Read: 366-392; `records/A27.md:350-365`. Old verdict: Rewrite (expect `failed_tick == pins[3].act_tick`, `realized_prefix == 3`, not typed ticks). Holds.
1 Claim: on Mutemath "Allies", the first stored path has four windows; moving the last window's end breaks at tick 49920 (prefix 3) and the second's at tick 22272 (prefix 1).
2 Owner: `search_target`, `pinned_windows`.
3 Real chart: Allies is real; no dump at any window. 49920 and 22272 are the act ticks of windows 4 and 2 of the first stored path ("pinned as read from one run", line 369). `REQUIRE(pins.size() == 4)` (line 375) is also a search result. At 60 the first stored path can be a different path. Claim not checked at 60. Needs: `hydra_replay dump --efill 60` of Allies at the default `app::Settings`.
4 Direct: 380-383 and 388-391 typed ticks.
5 Reason: none.
6 Display: no.
7 Failure: Track R2.
8 Deleted: 382, 390.
9 Stand-in: the ticks stand in for `pins[3].act_tick` and `pins[1].act_tick`.
10 Same rule: W10.
11 Real: yes.
Verdict: Rewrite (holds). Typed values that can move: `49920`, `22272` (lines 382, 390), and `pins.size() == 4` (line 375).

### W13. Delta round: window, replaces A27 ST10 (fills past the last pin branch, fills before it follow the pins) (tests/test_search_target.cpp:420)

Read: 416-443; `records/A27.md:382-397`. Old verdict: Investigate, claim not checked only (Allies' record).
1 Claim: pinning the first window of Allies' best path gives paths that all start with it and branch afterwards (`afters.size() > 1`).
2 Owner: `run_search` with `free_after`.
3 Real chart: Allies; no dump. `best.size() >= 3` (425) and `afters.size() > 1` (442) both depend on the best path and on how many different fills the search may take after the pin, which depend on the window. The graph at line 427 is built with the default window, not `cfg`'s (both 60 today). Claim not checked at 60. Needs: the Allies dump at 60.
4 Direct: 425, 436, 441, 442 properties and thresholds.
5 Reason: `>= 3` and `> 1` have no stated source.
6 Display: no.
7 Failure: free_after branching.
8 Deleted: 442 a search that stops branching.
9 Stand-in: none.
10 Same rule: W12.
11 Real: yes.
Verdict: Investigate. Claim not checked only. Typed values that can move: `best.size() >= 3` (425), `afters.size() > 1` (442).

---

# Helpers

| File | Function | Window | Reaches the search |
|---|---|---|---|
| `tests/store_fixtures.h` | `fixture()` (27-33) | default 60 (settings set cap 4, Scores 0, no ms; the window is never set) | yes, one corpus chart (`first_analyzed_with_paths`) |
| | `at_cap`, `at4`, `no_paths_at_cap`, `saved_figures` | none | no (relabel or edit `fixture().record`) |
| | `save_as_old_build` (74-82) | default 60 via `settings.to_analysis_settings()`; the caller's `app::Settings` | yes, `corpus::analyzed` on a named chart |
| `tests/record_fixtures.h` | `scores_settings` (582) | default 60 | settings only |
| | `lone_pricing` (374), `lone_pricing_mismatch` (408) | the caller's settings | yes, `search_target` |
| | `wide_search` (263), `sp_track_start`, `deact_edge_at`, `extension_of`, `windows_text`, `collect_tied`, `all_tied`, `roots_and_tied`, `act_ticks`, `path_named`, `check_lines`, `tick_ms` | none | read an existing graph or path |
| | the `*_song` builders (45-250), `set_*` helpers, `tied_variant_record`, `zero_base_path` | none | hand-built songs and records; the search runs on them only in other groups' files |

## Splits and open items

- Split: `test_search_target.cpp` lines 405, 427 and 456 build `ScoreGraph graph(song, kCloneHeroSpCap)` with the constructor's default window, while the record they are compared with (lines 280, 71) is searched at `cfg.early_fill_window_ms`. Both are 60 today, so no test is red, but two places answer "which window does this search run on". `test_rules.cpp` lines 282, 305 and 420 build their graph the same way.
- Split: the helper comment at `test_library_jobs.cpp:844` states a fact about Believer's best path (a 0.0 ms squeeze-in) that only the 85 ms dump `believer.schedule.json` shows.
- `Config::legacy_fills` at `test_store_results.cpp:46` is declared and never set or read (the matrix never runs a 1.0 chart).
- Typed numbers found that can move (each is also in its record): test_store_results.cpp:609 (no hardest figure) and :732 (pathcount 1); test_rules.cpp:344, :362, :370-371, :373, :405-406; test_library_jobs.cpp:844 (comment) and the figure the fill must find for :957; test_search_target.cpp:113-119, :154-157, :162, :169-174, :180, :207-209, :220, :305, :356, :375, :382, :390, :425, :442.
- Not reached: nothing. Every test case in the six files is in a table above, and both helpers are in the helper table.
