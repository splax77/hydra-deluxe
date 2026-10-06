Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task FX-R: M_D review fixes, reports and search

Task id: FX-R. Base: b6ec982 (the reviewed claude/p3-d2 tip). Branch `claude/p3-fx-r`. Worktree `.claude\worktrees\p3-fx-r`.

## Goal

The M_D derive-once review found copies in the report pages, search and CLI. Its full text, with every file and line, is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\review-M_D.md`, section "Part: reports" (findings 1 to 12), plus finding 2 of "Part: library" (the same charter copy). Read those findings before you start. Fix each item below so the question has one owner and everything else calls it. User decisions: D56 in `docs/audit/2026-10-03-fix-decisions.md` on main (57981dd); read it in the main checkout.

## What is left

1. **Charter cleaning (reports 1, library 2).** Add `display_charter` beside `display_artist` in `src/parse/song.h`: strip tags and trim, with no "(unknown)" fallback (the charter keeps today's text otherwise). Point every shown charter at it: `report.cpp`, `dm_report.cpp`, `fill_report.cpp`, `src/ui/library_model.cpp`, `src/ui/details_panel.cpp`. Search keeps the stored text.
2. **Empty artist (reports 12, D56 item 2).** A missing artist reads "(unknown)" everywhere: empty, the scan's "<unknown artist>" placeholder, or tags only. Do it in `display_artist`'s rule the way `display_title` already treats its old placeholder (one rule, no second copy). Fix the comment that calls `artist_or_unknown` "the one fallback". Stored text is unchanged.
3. **Leaderboard average tile (reports 2).** The tile rounds with JS `toFixed(2)`; the cells use `format_percent`. Make the tile print by the same rule as the cells (the tile must still follow the filters). Pin 198,010 of 200,000 reading "99.01%" in both.
4. **Fill rule from `legacy_fills` (reports 3).** One `fill_rule_for` beside `engine_mode_stamp`; `report.cpp`, `src/cli/batch.cpp` (both spots, including the stamp-to-name lambda), `src/search/pather.cpp` and `src/ui/library_dialogs.cpp` call it.
5. **"1 charts" (reports 4).** `fill_report.cpp` and `src/cli/fillcompare.cpp` print the count through the existing count owner so one chart reads "1 chart". Re-pin `test_cli`. Tighten the counts scan row in `tests/test_single_owner.cpp` as the review proposes.
6. **Page search (reports 5, D56 item 1).** Words only, as built. Make the shared script's comment and the `visible()` comment say so: accents fold, every word must match, quotes and field prefixes are ordinary words. No parser port.
7. **Fold table (reports 6).** Name the three fold ranges once, used by both `fold_into` and `search_fold_table` in `src/app/library_query.cpp`. Replace the hand-written UTF-8 encoder with the existing one (`wide_to_utf8` or whatever the repo already uses). Add a completeness check: every character in the named ranges that `fold_for_search` changes is in the table.
8. **Average multiplier in JS (reports 8).** The path report prints the multiplier as C++ text through `format_avg_mult`, the way the timing columns already do; drop `r.mult.toFixed(3)`.
9. **`tier_for` floor (reports 9).** Remove the duplicate 2 ms floor check so the table's first row answers it once. Keep every tier test green.
10. **Empty-record fixture (reports 10).** `tests/test_fill_report.cpp` (three places) and `tests/test_cli.cpp` use `store_batch_result` from `tests/display_fixtures.h` instead of building the empty ready record by hand.
11. **Docs and comments (reports 11).** The `display_format.h` comment names `format_ms_spaced`. The fill footer calls `fill_rule_description` instead of restating "Four beats".

Item 7 of "Part: reports" (path_view's tick rounding) belongs to FX-L. Audit findings 155, 312 and 51 stay as they are.

## Owned files

`src/parse/song.h`, `src/parse/song.cpp`, `src/app/report.cpp`, `src/app/report.h`, `src/app/dm_report.cpp`, `src/app/dm_report.h`, `src/app/fill_report.cpp`, `src/app/fill_report.h`, `src/app/html_page.cpp`, `src/app/html_page.h`, `src/app/library_query.cpp`, `src/app/library_query.h`, `src/app/display_format.h`, `src/app/display_format.cpp`, `src/core/squeeze_rating.cpp`, `src/core/squeeze_rating.h`, `src/core/rules.h`/`.cpp` or wherever `engine_mode_stamp` lives, `src/search/pather.cpp`, `src/cli/batch.cpp`, `src/cli/fillcompare.cpp`, `src/ui/library_model.cpp`, `src/ui/details_panel.cpp`, `src/ui/library_dialogs.cpp`, the scan code that writes "<unknown artist>" (comment only), `tests/test_song.cpp`, `tests/test_report.cpp`, `tests/test_dm_report.cpp`, `tests/test_fill_report.cpp`, `tests/test_library_query.cpp`, `tests/test_cli.cpp`, `tests/test_library_model.cpp`, `tests/test_squeeze_rating.cpp`, `tests/display_fixtures.h` (only if a helper is missing), `tests/test_single_owner.cpp` (rows for these items only), `tests/ui/uitest_details.cpp`.

## Tests you may run

`-sf=*test_song*`, `-sf=*test_report.cpp`, `-sf=*test_dm_report*`, `-sf=*test_fill_report*`, `-sf=*test_library_query*`, `-sf=*test_library_model*`, `-sf=*test_squeeze_rating*`, `-tc="hydra_fillcompare*"`, `-tc="hydra_batch*"`, `-tc="hydra_report*"`, `-tc="single-owner*"`; build `hydra_uitest` and run `--test report-buttons`, `--test dm-compare-flow`, `--test panel-headline`, `--test library-search`.

## Done when

Each item has one owner and a test. No score, path or record changes. Visible text changes only where an item names one: the charter's surrounding spaces, "(unknown)" for an empty artist, the tile's rounding edge, "1 chart".
