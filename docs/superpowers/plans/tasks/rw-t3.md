Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T3: the shared table view and both reports' columns

Task id: RW-T3. Base: the commit your prompt gives. Branch `claude/rw-t3-view`, worktree `.claude\worktrees\rw-t3-view`; make it as the preamble says.

Your spec is the plan's "Task 3" section, plus the spec's "Table behaviour", "Cells" and "How the code is arranged" (the shared table view and each report's columns). This brief adds only what the plan leaves to dispatch time.

## What this brief adds

**Search keeps D56 item 1.** The plan says search "uses `app/library_query`'s parser". It does not. D56 item 1, a standing user decision, says a report's search folds accents and matches every typed word in any order, and nothing more: quotes and the Library's field prefixes (`artist:`, `title:`, `charter:`) are ordinary words, and `stars:` or `squeeze<=` filters don't apply. So don't call `parse_library_query`. Fold the typed text with `fold_for_search` (`app/library_query.h`), split it into words on whitespace, and keep a row when every word appears in its search text. Each row's search text is what the pages use today, `html::search_field` (`src/app/html_page.cpp:538`), which already builds it from `make_searchable`. Call it; don't build a second one. T7 decides where `search_field` lives once the pages go. The JavaScript copy of the word match is `visible()` at `html_page.cpp:341-347`; the C++ version must give the same answer, including on an empty search and on text with accents.

**Where today's rules live.** The shared sort, count line and header-click direction are in the shared page script in `src/app/html_page.cpp` (lines 332-505: `sortKey`, `sortDir`, the count line at 412, a header click at 471-477). The path page's columns are `cols` at `report.cpp:121` and its cells `cells` at `report.cpp:154`. The comparison's are `cols` at `dm_report.cpp:81` and `cells(r)` at `dm_report.cpp:103`. Each page's keep-rule is its `PAGE.filter`. Read those, then write the C++ to the same truth table. Pay attention to how the page sorts a column whose payload key differs from its cell text, such as % of opt, which sorts on the shown hundredths (`percent_steps`). The sort key must read the same value the page sorts on.

**Cells call existing C++.** Every cell text either comes from a field the row already holds, or from the existing C++ text function the page's payload calls today (`ms_text`, `efill_text`, `format_avg_mult`, `format_percent`, the thousands separator the payload uses). Find each in `report.cpp` and `dm_report.cpp`'s payload builders (`build_html` at `report.cpp:520`, `build_dm_html` at `dm_report.cpp:272`). If a cell's text is made only in JavaScript today, with no C++ function behind it, and it's more than a literal word, stop and report it: T2 or a new owner may need to take it, and that's an orchestrator call.

**The timing tier keep-rule.** A path row's tier comes from `tier_for` and the `timing_tiers` table (`report.h`). `path_keep(tier, best_only)` calls them; it never works a band out again. The Timing dropdown's choices are the table's tier names plus "All timing tiers".

**Definitions.** The column definitions move word for word. The `__BASE_SPEED__` and `__SP_CAP__` substitution is done today at `dm_report.cpp:166-172`; fill them the same way, from `net::kBaseSpeedPercent` and `kCloneHeroSpCap`. Check whether the path page's definitions carry placeholders too (`report.cpp` near line 121) and fill those from their constants the same way.

**The count line's noun and count.** The page counts `countOf(rows)` with a noun (`html_page.cpp:412`). Find what `countOf` counts for each page and match it.

**CMake.** Add the three new `.cpp` files next to `src/app/report.cpp` (line 259) and `src/app/dm_report.cpp` (line 341) in their libraries, and `tests/test_report_view.cpp` next to `tests/test_report.cpp` (line 496).

## Owned files

New `src/app/report_view.h`, `src/app/report_view.cpp`, `src/app/path_report_view.h`, `src/app/path_report_view.cpp`, `src/app/dm_report_view.h`, `src/app/dm_report_view.cpp`, `tests/test_report_view.cpp`, and their four lines in `CMakeLists.txt`.

Owned-file check: every T3 step adds new files and reads the existing report headers; nothing in `report.{h,cpp}`, `dm_report.{h,cpp}` or `html_page.{h,cpp}` changes (T2 owns the first four in this wave).

## Preflight

Command: `Grep "cols|cells" src/app/{report,dm_report}.cpp`, `Grep "sortDir|countOf| of '" src/app/html_page.cpp`, `Grep "^[A-Za-z].*\(" src/app/library_query.h` and `Grep "report\.cpp|dm_report\.cpp|test_report\.cpp" CMakeLists.txt`, run by the orchestrator on base 658abcf on 2026-10-08.
Output: path `cols` at `report.cpp:121`, `cells` at 154; dm `cols` at `dm_report.cpp:81`, `cells(r)` at 103; the shared sort state at `html_page.cpp:332`, `visible()` at 341-347, count line at 412, header click at 471-477; `search_field` at `html_page.cpp:538`; `library_query.h` offers `fold_for_search` (line 28), `parse_library_query` (72) and `make_searchable` (78); CMake lists `src/app/report.cpp` at 259, `src/app/dm_report.cpp` at 341, `tests/test_report.cpp` at 496, `tests/test_dm_report.cpp` at 499.

## Tests

`-sf=*test_report_view*` only.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
