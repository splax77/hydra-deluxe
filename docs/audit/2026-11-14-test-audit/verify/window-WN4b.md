a80ff5aece2096e1e

# Verify: window round WN4b (T5-V-WN4b)

Task T5-V-WN4b, session e633f7f6-f313-4787-b622-8d901b577b64. Code read at the worktree HEAD (`5ec682c7`'s code). Nothing was run. Every claim below is "by reading" unless it names a run. Part checked: `records/window-WN4b.md`.

The filed rulings on the window, under `## Confirmed` in `docs/agents/rulings/code-and-engine.md` lines 55 to 57: "so we should use the hit window. and stop inventing our own numbers." / "make the early fill threshold an adjustable setting pinned to one number in both directions" / "analysis settings panel, default 60ms, supports 0ms to 85ms". Nothing in the auditor's file contradicts them.

## Scope

I counted the test cases myself with grep. `tests/test_pb_coach.cpp` has 19, `tests/test_pb_text.cpp` 10, `tests/test_saved_figure_fill.cpp` 10. The auditor's tables list 19, 10 and 10 with the same line numbers. No test case is missing from the tables.

`tests/dm_fixture.h`: the auditor's correction stands. `sample_chart()` (lines 46-52) runs `corpus::first_analyzed_with_paths` on an `AnalysisSettings` that sets only `depth_value`, so the window is the default. I agree with the helper's kind C and with the split the auditor lists (the earlier WN4 files called it "no search"). The five users are other groups' files.

## A rows

There are none, and none should exist. I found no `early_fill_window_ms`, `efill` or `e_window` set in the three files, `scratch_settings.h`, `store_fixtures.h` or `corpus_default_cfg.h`. `Settings::early_fill_window_ms` (`src/app/config.h:195`) defaults to `kDefaultEarlyFillWindowMs`, and `src/app/config.cpp:411` copies it into `AnalysisSettings`. So the default-window claim holds for all three files. Rows checked: 0, disputed: 0.

## B rows (9 checked, 0 disputed)

- coach 99 and 112: both click tick 0 on Burnout and assert a sentence or generation counters. A smaller window only refuses more fills (`fill_refuses`, `src/core/model.h:331`), so a tick with no fill cannot gain one. The reason holds. One gap I note, not a dispute: "tick 0 holds no fill" is the test's own claim (line 104) and no engine record shows it. That is "claim not checked" and does not depend on the window.
- coach 427: `remove_window` returns `{}` before any search when no pinned window has the tick (`src/pathbuilder/pb_coach.cpp:195-198`, read). Holds.
- coach 469: assertions are generation counters and empty lists. One line does depend on the search: 498 to 499 require the click on `best[1]`'s tick, with an empty path, to succeed and pin one window at that tick. The auditor's B note covers 475 but not these lines. I judge it still B, because the tick comes from the same search's own best path, so a fill exists there at any window. But it assumes the fill is reachable with the first activation skipped. That is a small gap in the reason, and a dispute only if the main session wants B to mean "no search-dependent assertion at all".
- coach 503: `ladder(c, {})` of an empty path; asserts only empty rows. Holds.
- coach 516: counts depend on timing, not on the window; asserts `bf.size() == 1` for no windows. Holds.
- pb_text 52: constant against string. Holds.
- saved_figure_fill 121: `chart_figures` returns at `chart_still_there` before the graph (`src/app/saved_figure_fill.cpp:96-97`, read). Holds.
- saved_figure_fill 212: the six charts come from `rows_missing_hardest`, and the checks compare runs against each other. The floor `figures > 0` rests on the engine's own path replaying on the same graph at the same window (the save and the replay both read `Settings{}`, so both are at 60). Holds.

## C records (30 checked)

Facts I checked against the code, each by reading:

- The default is 60 (`src/core/model.h:112`, `src/search/pather.h:45`). `fill_refuses` and `is_e0` take the window (`model.h:331,335`). `Activation::within_ms_limit` passes `e_window_ms` into `fill_within_limit` (`src/core/model.cpp:535`, `model.h:369-372`). So coach record 3's added finding is right: the limit row can move with the window.
- `PbState::open_with` builds the graph and the record from `settings.batch_run().settings` (`src/pathbuilder/pb_state.cpp:15-28`). `scratch_settings()` changes only folders, depth and auto-open. So every coach case runs at 60.
- `saved_figure_fill`: `chart_figures` builds `search_graph(song, settings)` from the same `AnalysisSettings` (`saved_figure_fill.cpp:100`), and `save_as_old_build` saves from `corpus::analyzed(chart, settings.to_analysis_settings())` (`tests/store_fixtures.h:78-79`). Save and replay agree on the window. Claim holds.
- Line quotes spot-checked in `test_pb_coach.cpp`: 66 to 68, 88 to 89, 208 to 219, 249 to 273, 282 to 293. The typed values and the comments "From one run" are as the auditor says.
- The engine records: only `believer.dump.json` is one of these charts, and it is at 85. I did not re-open the dump. The auditor's statement that no dump exists at 60 matches `records-engine/index.md` as quoted in the earlier verifier file.

Each record's Q3 is "claim not checked" or "not checked at 60", which is right: no dump at 60 exists, and a hand-built or 85 record alone does not prove a claim at 60. I mark every claim in the 30 records "by reading", none "proven by" a run at 60.

Verdicts, agree or dispute:

- Agree, Rewrite: 1, 3, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15, 16 to 24, 25, 28, 29, 30, 32. The faults are in the tests (typed numbers, typed ticks, typed text, index-based picks), and each record names a bug at a line in question 8.
- Agree, Investigate (claim not checked only): 7 (coach 276), 26 (saved 133), 31 (saved 341).
- Dispute, record 27 (saved_figure_fill 147, was Keep, now Investigate): line 164 types `0.0`, which is a typed engine figure at 85 and the record rates it as "claim not checked only". But it also admits the figure is "the engine's figure at 85 only", a typed number from one run. By the audit's own rule (a test whose fault is a typed number is a Rewrite; Investigate is for question 3 alone) this should be Rewrite: replace 164 with a comparison against the figure a fresh save holds, as record 25 asks. Investigate is not wrong for the stop and marker lines, but the typed 0.0 is a fault in the test, so I put it at Rewrite.
- Dispute, record 7 (coach 276): line 282 types `ends.size() == 2` and lines 284 to 293 assume the order [squeeze-in, squeeze-out] by index. That is the same fault the record flags for 249 to 253 in record 6, where the verdict is Rewrite. The kept-end rule is checkable without typed indexes (pick the end whose `window.deact_tick` equals `four[3].deact_tick`; line 286 already does). I put it at Rewrite for the same reason as record 27.
- Dispute, record 31 (saved 341): the typed `squeezed > 0` floor is a guard rather than a typed engine value, and the record says what to do (require the named charts skipped zero times). That is a fault in the test (a floor standing in for "the loop ran"), so Rewrite fits better than Investigate. I rate this a soft dispute.
- Dispute, record 26 (saved 133): line 143 types `0.0` as a fresh-save figure, the same fault as record 27. Rewrite, not Investigate, for the same reason.

So the auditor's count (Rewrite 26, Investigate 4, Keep 0, Remove 0) becomes, by my reading, Rewrite 30 and Investigate 0. The four are the same line, "a typed engine figure or index from one run, which the rubric calls Rewrite". If the main session reads the "claim not checked only" rule as covering a typed zero, the four stand. I leave that to it.

Question 8 bugs at a line: I checked records 1, 3, 5, 6, 9, 12, 13, 25 and 32. Each names a line and what would pass wrongly. Several are "reasoned", not named from a past failure; the record says so, and I agree with the label. Record 5's failure is named in the test's own comment (lines 199 to 201), which I read.

## Typed numbers

I read the auditor's lists against the test files for the lines I opened (coach 66 to 96, 144 to 175, 187 to 189, 208 to 209, 249 to 273, 282; pb_text unchecked by line beyond the headings). They match. Two I add:

- `tests/test_pb_coach.cpp:498-499` (one pinned window, `act_tick == tick`): not a typed number, but a search-dependent assertion in a case kept as B (see B rows).
- `tests/test_pb_coach.cpp:210` `REQUIRE(kFillTick < best[0].act_tick)`: a typed tick compared to a search result, a guard that stops the case at 60 if the best path's first window moves before tick 36480. The auditor lists 209 but not this comparison.

## Splits

- `tests/dm_fixture.h` read as "no search" in `records/window-WN4.md` and `verify/window-WN4.md`, "search at the default 60" in `records/window-WN4b.md`. The code agrees with the latter.
- `Settings::early_fill_window_ms` is an `int` (`config.h:195`); `SearchSettings::early_fill_window_ms` and `Activation::e_window_ms` are `double` (`pather.h:45`, `model.h:606`). The auditor lists this as a dependence, not a split of answers; I agree.
- No other split found in these three files.

## Not reached

I did not open `build_ladder`, `cost_list`, `candidate_costs`, `badge_timings`, `tight_window_note`, `shown_timing` or `engine_hardest_ms` bodies; the auditor did not either. I did not re-read pb_text's typed lines (68 to 354) or `test_saved_figure_fill.cpp` line by line; I counted test cases, checked the code the records stand on, and spot-read the coach cases.
