Verifier agent a3632c4b6f1208ebb (T5-V-WN1, session e633f7f6-f313-4787-b622-8d901b577b64). Checked: `records/window-WN1.md` at commit 4093c2b6. Code read at the worktree HEAD. Nothing was built or run.

# Verdict in one line

The auditor's file holds. No test case is missing, no B is disputed, every C verdict is agreed. Two small notes (one wrong line cite, one B reason that is narrower than its case) are below; neither changes a kind or a verdict.

# 1. Scope: did every search-reaching case get a row

I listed every `TEST_CASE` line in the six files and matched each to a row. The counts and line numbers agree with the auditor's tables.

| File | Cases | Rows A / B / C / delta | Missing |
|---|---|---|---|
| test_library_query.cpp | 31 | 0 / 31 / 0 / 0 | none |
| test_search_target.cpp | 12 | 0 / 5 / 7 / 0 | none |
| test_rules.cpp | 26 | 0 / 23 / 3 / 0 | none |
| test_library_jobs.cpp | 33 | 0 / 24 / 1 / 8 | none |
| test_store_results.cpp | 21 | 0 / 18 / 2 / 1 | none |
| test_store_records.cpp | 56 | 0 / 22 / 0 / 34 | none |

Where I searched, beyond the case list: I grepped all six files and `record_fixtures.h` for `analyze_chart`, `corpus::analyzed`, `first_analyzed`, `search_target`, `ScoreGraph`, `run_search`, `Engine`, `fixture()`, `corpus_analyses`, `lone_pricing`, `save_as_old_build` and `set_analyzer`. Every hit sits in a case that has a row. In `test_library_jobs.cpp` the hits are lines 382, 466, 482, 499, 562, 588, 623, 683, 774 (a real result handed to a test analyzer) and 850 (`save_as_old_build`); all are in rows. `test_library_query.cpp` has no search call at all (only `corpus::root` at 19 and 98). `lone_pricing` (`record_fixtures.h:374`) runs `search_target` but no case in this group calls it.

The A rows are none. I confirmed no case or helper in the group sets `early_fill_window_ms`: `store_fixtures.h::fixture()` (lines 22-33) sets cap, depth and ms filter only; `save_as_old_build` (74-82) uses the caller's `app::Settings`; `corpus_analyses()` (`test_store_records.cpp:59-71`) uses `SearchSettings{}`. The default is 60: `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`), and the `fill_refuses` / `is_e0` signatures are at `model.h:331` and `:335`, as quoted. The engine reads the window from the graph (`engine.cpp:2030`, `:2176`; `graph.h:195-207`). All "proven" below means read in code; I ran nothing.

The delta list is right. `delta-DL1.md` holds records for every `test_store_records.cpp` line the auditor lists as delta (34 lines) and for `test_store_results.cpp:339`. `delta-DL4.md` holds `test_library_jobs.cpp` 464, 481, 498, 524, 560, 587, 622, 682. No delta record covers a line the auditor judged here, so nothing is judged twice.

# 2. B rows: reasons checked against the code

Agreed (by reading), with the fact I checked:

- **test_library_query.cpp, all 31**: no search; `analyzed(stars, ms)` at line 30 is a hand-built `RowFacts`; `discover_charts` at line 98 reads metadata only.
- **test_search_target.cpp 225 and 500**: hand-built paths (`made(...)`, lines 78-87, 501-504).
- **test_search_target.cpp 274**: lines 275-293. The record (`corpus::analyzed(chart, cfg)`, line 280) and the pinned search (`search_target(song, cfg, ...)`, line 286) both use `cfg`'s one window; only `paths > 0` is typed. Agreed.
- **test_search_target.cpp 397**: lines 405-411 compare two `run_search` calls on one graph. Agreed.
- **test_search_target.cpp 451**: lines 453-492. Every check is a property of whichever chart the loop picks, except the final `FAIL` at 492, which needs one chart whose best path has a window over a 0 ms limit. That is an existential over the corpus. Agreed as B, with the auditor's own caveat. Note for the main session: `Activation::within_ms_limit` reads the activation's `e_window_ms` (`engine.cpp:2030`), so the window feeds the limit test; the claim "some chart still qualifies at 60" is reasoned, not shown by a dump.
- **test_rules.cpp 269**: lines 272-295, three searches per chart at one window each, with `<=` checks and an existential "some chart moved". The leeway acts on backend rows. Agreed.
- **test_rules.cpp 298 and 415**: `tied_pathcount() == 1` and `>= 1`, no typed value. Agreed.
- **test_rules.cpp, the other 18 B rows**: rules text, fingerprints, hand-built rows. Agreed.
- **test_library_jobs.cpp**: 120-378 use test analyzers that throw or return a copied result (`gated_failure`, `released_in_turn`, `always`); 722, 773, 785, 796, 1108, 1274, 1291, 1332, 1349 never run the search or only copy a fixed result. Agreed.
- **test_library_jobs.cpp 856**: read at 856-935. The step sequence depends on rank 1 being emptied by the helper (`{1}`, line 851), and `fill_saved_hardest` marks the profile done whether or not a figure turned up (`saved_figure_fill.cpp:150-151`: `if (done < charts) return false; store.mark_hardest_figure_filled(profile);`). No assertion reads a figure. Agreed.
- **test_library_jobs.cpp 1000**: stops before any write; `rows_missing_hardest(...).size() == 1` (line 1016) follows from the helper emptying rank 1. Agreed.
- **test_store_results.cpp**: 103 (matrix compares each record with itself, line 156; no typed value), 167, 214 (line 244 compares with `fixture().record.best_path().totalscore()`), 247, 295, 312, 322, 370, 400, 438, 460, 468, 511, 535, 555, 653, 689. Agreed. **625**: the hand-built activation sets `e_offset = 300.0` (line 631), and `is_e0` needs `e_offset < window` with the window at most 85, so it is never E0 at any allowed window. Agreed.
- **test_store_records.cpp**: 130, 557 (every expected value is read from the same record, lines 570-575), 578, 641, 652, 676, 785, 801, 819, 841, 871, 881, 904, 940, 975, 1039, 1067, 1094, 1185, 1267, 1354, 1384. Agreed. **641**: `Path::notecount` is set to the chart-wide count (`engine.cpp:2007`) and tied variants copy it (`model.cpp:666`), so the per-path equality cannot move. Note: the reason does not mention the `CHECK(checked > 0)` at line 649, which needs one corpus chart with more than one path. That is an existential across the corpus and I judge it safe at any window; it is not shown by a record. No change to the kind.

No B is disputed.

# 3. C records

All thirteen records quote their test lines correctly (I read each range in the files). In every one, question 3 names a `hydra_replay dump --efill 60` run that does not yet exist; the `--efill` flag exists in `tools/replay.cpp`. `records-engine/` holds seven dumps all made under 85 (`index.md`), none at 60, and none of Sugar, HopH2O, Unbound, Allies or the fuzz charts. I checked the old record each one names exists at the cited heading: `A34.md` R17 (line 283) and R21 (348); `A26.md` RU17, RU18, RU19 (354, 369, 384); `A27.md` ST1, ST2, ST3, ST6, ST7, ST8, ST10 (239, 256, 272, 319, 334, 350, 382); `A16.md` (334). The old test line numbers in those headings are a few lines off the current ones (the files grew); the cases are the same.

| Record | Case (line) | Verdict agreed | How the key claims were checked | Typed values that can move (confirmed in the test file) |
|---|---|---|---|---|
| W1 | test_store_results 591 | Rewrite, agreed | by reading: `fixture()` at the default window; line 609 `CHECK_FALSE(...hardest_ms.has_value())` is a typed fact about one chart, and its own comment (606-607) says "pinned from one run" | line 609 |
| W2 | test_store_results 728 | Rewrite, agreed | by reading: line 732 `pathcount == 1`, comment 730 "a literal from one run" | line 732 (the `== 3` at 734 is a hand-built record, correctly not listed as movable) |
| W3 | test_rules 318 | Rewrite, agreed | by reading: Sugar, legacy fills, ms 0, tie limit 1; the typed string `0 E3+ E5 E1` has three E marks | line 344 |
| W4 | test_rules 349 | Rewrite, agreed | by reading | 362 (694985), 370, 371, 373 ("78.9 ms", an early-fill difficulty per `early_fill_difficulty`, `model.h:339-340`) |
| W5 | test_rules 384 | Rewrite, agreed | by reading | 405, 406 (859580 and both strings) |
| W6 | test_library_jobs 937 | Investigate (claim not checked only), agreed | by reading: the trigger fires only on `UPDATE` (lines 946-948); `set_hardest` runs only when a figure was found (`saved_figure_fill.cpp:145`); a fill with no figure makes line 957 fail, not pass | the "squeeze-in at exactly 0.0 ms" comment at 844-845; the 85 ms fact is `records-engine/index.md` line 25 (Believer path 0 `1 0+ 0 0`, squeeze-in at tick 57600), which is an 85 ms dump |
| W7 | test_search_target 104 | Rewrite, agreed | by reading; fuzz seed 5, no engine record at any window | 113-119 |
| W8 | test_search_target 141 | Rewrite, agreed | by reading | 154-157, 162, 169-174, 180 |
| W9 | test_search_target 198 | Rewrite, agreed | by reading | 207-209, 220 |
| W10 | test_search_target 299 | Investigate (claim not checked only), agreed | by reading: lines 299-339; `REQUIRE(path != nullptr)` at 306 means a missing path at 60 fails visibly | line 305 (`"0+- 0-"`) |
| W11 | test_search_target 345 | Rewrite, agreed | by reading | line 356 (`realized_prefix == 2`); 361-363 are chart ticks and reasons |
| W12 | test_search_target 370 | Rewrite, agreed | by reading | 375 (`pins.size() == 4`), 382 (49920), 390 (22272) |
| W13 | test_search_target 420 | Investigate (claim not checked only), agreed | by reading | 425 (`>= 3`), 442 (`> 1`) |

Counts: Rewrite 10, Investigate 3, Keep 0, Remove 0, as the auditor says. I agree with all 13. None of the Rewrite verdicts proposes a saved expected-output file (ruling `code-and-engine.md` line 50); each proposes comparing with an engine answer or a property.

Question 8 (named bug), checked at a line or in git: W1 `bc01fa6f` is "Store: each result keeps its best path's star count" (git, exists); W6 `b51ce66e` is "A4b tests: ... a test for a fill that fails" (git, exists); W3 "finding 95" is at `test_rules.cpp:314`; W4 "finding 330" at `:347`; W5 "D55 item 5" at `:380`; W2 "finding 172" at `test_store_results.cpp:729`; W7 to W9 "D45" at `test_search_target.cpp:3`, `:91`, `:131`, `:192`; W10 to W12 "Track R2" at `:3` and `:269`. The W13 failure ("free_after branching") is named by what the test checks, not by a finding or commit; it has no id, which matches the old A27 ST10 record.

The user's words the auditor quotes (`docs/agents/rulings/code-and-engine.md`, `## Confirmed`, lines 55-57) I did not re-read in full; the brief's own quote of the same lines matches the three phrases in the auditor's file.

# 4. Small notes (no kind or verdict changes)

- **Wrong line cite in W6.** Record W6 question 3 says the write happens at "`saved_figure_fill.cpp:130`, `if (!p.figures.empty()) store.set_hardest(...)`". In the code that line is 145. Line 130 is a comment. The claim is true; the number is not.
- **Comment in `graph.h` is the owner of the window split.** `graph.h:195-197` says the graph "carries" the window to every engine run. The auditor's split (lines 405, 427, 456 of `test_search_target.cpp` and 282, 305, 420 of `test_rules.cpp` build a `ScoreGraph` without the window while a compared record uses `cfg`'s) is real: those graphs get the constructor default, not `cfg.early_fill_window_ms`. I confirmed the six lines. Both are 60 today. Listed as a split below.

# 5. Splits (one line each)

1. `test_search_target.cpp` lines 405, 427, 456 and `test_rules.cpp` lines 282, 305, 420 build `ScoreGraph` on its constructor default window, while the record or options they work with come from `cfg.early_fill_window_ms`; two places answer "which window does this search run under". Both are 60 now.
2. The helper comment at `test_library_jobs.cpp:844-845` states a fact about Believer's best path ("a squeeze-in at exactly 0.0 ms") that only the 85 ms dump `records-engine/believer.schedule.json` shows; nothing at 60 backs it.
3. `Config::legacy_fills` at `test_store_results.cpp:58` is declared and never set or read by the matrix, so the matrix never runs a 1.0 chart (the auditor's open item; I confirmed no `legacy_fills` use in `kMatrix` at lines 63-73 or in the loop at 115-127).

# 6. Not reached and stops

Not reached: nothing in the group's six test files or two helpers. I did not re-read `docs/agents/rulings/code-and-engine.md` lines 55-57 in full. I did not open the old A-records beyond their headings, so each "old verdict" is taken from the auditor's quote of it, not re-read. No stop condition.
