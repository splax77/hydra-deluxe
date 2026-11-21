ac75ccc58bfefd13f

# Verify: window round WN8 (T5-V-WN8)

Task T5-V-WN8, session e633f7f6-f313-4787-b622-8d901b577b64. Code read at the worktree HEAD. Nothing was run; every claim is "by reading" unless it names a run. Part checked: `records/window-WN8.md`.

Filed window rulings (`docs/agents/rulings/code-and-engine.md` lines 55-57): "so we should use the hit window. and stop inventing our own numbers." / "make the early fill threshold an adjustable setting pinned to one number in both directions" / "analysis settings panel, default 60ms, supports 0ms to 85ms".

## Scope

I counted test cases by grep: test_dm_report.cpp 30, test_dm_other_speed.cpp 4, test_fill_report.cpp 22, test_report_rows.cpp 30 (22 judged, 8 left to WN4c). Line numbers match the auditor's tables. The `batch_run()` calls in test_config.cpp are at :182, :200/:206, :301 and :788, which are the cases 172, 187, 279 (delta DL4) and 779. No test case is missing from the tables. I found no direct search call in the dm and fill files; they reach the search only through `testdm::sample_chart()` (`tests/dm_fixture.h:46-52`: `first_analyzed_with_paths` on an `AnalysisSettings` that sets only `depth_value`), so the default 60 ms (`src/search/pather.h:45`, `kDefaultEarlyFillWindowMs = 60.0` in `src/core/model.h`). Agreed. No A rows exist and none should.

## B rows: 73 judged by the auditor, 0 disputed

I read the bodies of dm_report 61, 471, other_speed 36, the `put` helper in test_fill_report.cpp (:62-72), report_rows 253, 395, 494, 645, and config 172, 187, 779.
- test_fill_report.cpp: `put` overwrites `paths.front()`, and `best_path()` is `paths.at(0)` (`src/core/model.h:996`); the row's score comes from the stored best path's summary (`src/store/record_store.cpp:732-734`). So every asserted score in the file is typed. Reasons hold.
- test_dm_report.cpp:471: the 10000, 10000 and 5000 hundredths hold for any optimal above about 20,000. I checked `percent_steps` (`src/app/display_format.cpp:36`, round to nearest). The auditor's inference that the first chart's optimal is far above that rests on 9956, so it is itself a claim not checked.
- test_dm_other_speed.cpp:36: statuses and `delta == 1000` come from typed offsets from the same optimal. Holds.
- test_report_rows.cpp:451 and :812: the footer's 85 and 170 come from `hit_window_text()` over `kHitWindowMs` (`src/app/report.cpp:92`), a constant apart from the early-fill window. Holds.
- :253: both sides run `fixture_analyzer` on one `run.settings`. Holds. :494: the sum is arithmetic over three counts; `CHECK(page.songs > 0)` is the only search-dependent line and fails only if no Hard chart keeps any path. I agree with B and note it as a weak spot of the same kind as the C at :395, over many charts rather than one.
- test_config.cpp:172, :187, :779: `batch_run()` builds settings only; no search runs. Holds.
- Not read past heads and structure by me: the loopback HTTP cases (666, 683, 714), 119, and the 8 WN4c rows. Their reasons are plausible (no record is saved) and I did not dispute them.

## C records: 2 checked, 2 agreed on verdict

Record 1, test_dm_report.cpp:61, Rewrite: agree.
- 9956 at :91 with the comment at :88-89 "read from one run": confirmed by reading. The range is right: 100 - 100000/N rounds to 99.56 for N from about 224,719 to 229,885.
- It can move at 60 ms: the optimal is the best score of a search the window feeds. Q3 "claim not checked" is correct; no engine record of this chart exists.
- The old verdict Rewrite is confirmed in `records/A10.md` (it says "L91 is a snapshot").
- Question 8 is weak. It lists deletions and says deleting :91 "goes unnoticed" for a wrong total. That is not so: with the wrong total (the delta) the number would not be 9956, so :91 would catch it. The bug :91 stands for, a percent rounded down instead of to nearest at `src/app/dm_report.cpp:133`, moves the figure by a hundredth on some charts only. Verdict is unchanged; Q8's wording is disputed.

Record 2, test_report_rows.cpp:395, "Investigate, claim not checked only": agree.
- `REQUIRE(out.paths.size() > 1)` at :419 is the one search-dependent line; the counts of 2 come from library rows. Confirmed. It moves at 60 ms only if the chart's kept paths shrink to one; a dump at `--efill 60` would show that.
- Q8 names no bug at a line. It should say what :419 guards: that the loop at :420 runs over more than one row, so a copy count read from the first row only would pass. The auditor's text says this loosely.
- Old record R7 at `records/A25.md:324`: Investigate, confirmed.

## Typed numbers

The auditor's two (:91 of dm_report, :419 of report_rows) are right. I found no further one that a window change moves. The line to watch is :494 `CHECK(page.songs > 0)`, above.

## Splits

1. Auditor split 1 (test_fill_report.cpp:49-56, :62-72: a record searched under the 1.1 rule is filed as a 1.0 result by setting `record.legacy_fills`): confirmed by reading.
2. Auditor split 2 (record and profile each state a window and no test would notice a difference): **disputed**. `prepare_record` throws when the profile's lens window differs from the record's window (`src/store/record_store.cpp:695-699`; comment at `src/store/record_store.h:309-312`), and `testdm::save` goes through it. A one-sided default change would fail at the first save in this group. It is a guarded coupling.
3. The gap the auditor named, that no test moves `early_fill_window_ms` alone through `settings_change_touches` (test_dm_report.cpp:1030): confirmed; that case moves other fields. Not judged.

## Not reached

The three loopback HTTP cases and the JSON case beyond heads, and the 8 WN4c rows. No run was possible (rule 14).
