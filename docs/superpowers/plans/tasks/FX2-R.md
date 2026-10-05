Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task FX2-R: M_D review round 2 fixes, reports and search

Task id: FX2-R. Base: fc55f8b. Branch `claude/p3-fx2-r`. Worktree `.claude\worktrees\p3-fx2-r`.

## Goal

Round 2 of the M_D review: `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\review-M_D-r2.md`, section "Part: reports". Read it first; it has every file and line. User decisions: D56 and D57 in `docs/audit/2026-10-03-fix-decisions.md` on main (read them in the main checkout).

## What is left

1. **UTF-8 lead bytes (reports finding 1).** `fold_into` in `src/app/library_query.cpp` says which lead bytes start a two- and three-byte character twice (around lines 170, 183 and 197-199). One small helper (for example `utf8_length(lead)`) answers it; both spots call it. If the repo already has such a helper, call that. List other copies the reviewer names (batch.cpp, html_page.cpp) in your report; do not edit them.
2. **Stamp compare (reports 2).** `src/cli/report.cpp` (around lines 85-87) compares the database stamp to "ch10" by hand; call `fill_rule_from_stamp`.
3. **Leaderboard percent (reports 3).** `dm_report.cpp` (around line 231) still divides score by optimal into a double `pct` that only the sort reads. Sort on the whole-hundredths value and drop `pct`. The order of rows must not change except where two rows tie at the shown precision; pin one ordering case.
4. **Search wording (reports 4, D56 item 1, D57 item 3).** `tests/test_report.cpp` (around line 592) and `docs/UserGuide.md` (around line 207) still say the pages search "the way the Library does". Say what they do: accents fold, every typed word must appear in the text the page shows; quotes and field prefixes are ordinary words. Keep phase 4's docs test green.
5. **Scan rows (reports 5).** Add `tests/test_single_owner.cpp` rows the review proposes for `display_charter`, the `legacy_fills` flag reading (`fill_rule_for`) and the stamp reading (`fill_rule_from_stamp`), each with its must-match and must-not-match lines. Record `src/store/record_store.cpp`'s one-time migration compare (around line 641) as a known copy whose note says a migration reads the historic stamp text and store never includes search. Fix `tools/bench.cpp`'s "%d paths" through `counted` and drop its known-copy entry.

The `timing.h` comment and the `display_format.h` comment belong to FX2-P and FX2-L. The dynamics "1 of 1 kick notes" count is FX2-L's.

## Owned files

`src/app/library_query.cpp`, `src/app/library_query.h`, `src/cli/report.cpp`, `src/app/dm_report.cpp`, `src/app/dm_report.h`, `tools/bench.cpp`, `docs/UserGuide.md` (the search sentence only), `tests/test_report.cpp` (the search comment and an ordering case only), `tests/test_dm_report.cpp`, `tests/test_library_query.cpp`, `tests/test_cli.cpp`, `tests/test_single_owner.cpp` (your rows and known copies only).

## Tests you may run

`-sf=*test_library_query*`, `-sf=*test_dm_report*`, `-sf=*test_report.cpp`, `-tc="hydra_report*"`, `-tc="single-owner*"`, `-sf=*docs_match_code*`; build `hydra_uitest` and run `--test dm-compare-flow`, `--test report-buttons`.

## Done when

Each item has one owner and a test where it is code. No score, path, record or visible change except the guide's sentence.
