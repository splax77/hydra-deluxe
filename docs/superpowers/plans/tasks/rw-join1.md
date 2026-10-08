Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task J1: join T3 onto T2, fix T3's review, apply D103 items 16 to 18

Task id: RW-J1. Base: main after T2's merge; your prompt gives the hash. Branch `claude/rw-w1-join`, worktree `.claude\worktrees\rw-w1-join`; make it as the preamble says. T3's signed tip is not clean yet: its review found five fixable copies, several of them in T2's files. You join the two and fix them in one place.

## Steps

**1. Merge T3.** In your worktree, `git merge --no-ff claude/rw-t3-view` (tip `0e3cdd2925dbb1d3637a63cc52fad1a38546bb51`), with the trailers as `-m` lines because `git merge` has no `--trailer`. CMakeLists.txt may conflict on neighbouring lines; keep both sides' lines. Build `hydra_tests` through the slot helper and run `-sf=*test_report_view*` once to prove the join compiles before you change anything.

**2. Fix T3's review findings 1 to 5.** The review file is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\c4589620-c1de-4119-bfdd-c82e8fbe6a86\scratchpad\review-rw-t3.md`. Read it in full. The fix round rules in `docs/agents/fix-round.md` apply, including that a kind A fix ends in a scan row in `tests/test_single_owner.cpp` whose must-match examples include the removed copy's line. In short:

- Finding 1: the SqIn and SqOut column titles trip the squeeze-kind scan row. Add the exceptions the review proposes, with the reason.
- Finding 2: one `beyond_edge_text`, public in `report.h`, called by `tier_label`; delete `path_report_view.cpp`'s copy.
- Finding 3: one accessor for which `timing_tiers` entries are the open Beyond and None tiers, called by both `tier_for` and `tier_label`. T2 added a private `beyond_tier` in `report.cpp`; build on it rather than adding a third answer. Put it where the review suggests or beside `beyond_tier` in `report.h`, whichever keeps one owner.
- Finding 4: the eight dm status words get one owner in `dm_report.h`, used by `collect_dm_rows`, `tally_dm_rows`, `status_token`, `dm_tiles`, `status_choices` and the Points left tone. T2's fix round added a scan row for the "Not analyzed" tile label; check it still says the right thing once the words have an owner.
- Finding 5: `kPercentDecimals` is private in `dm_report.cpp` (line 178 on T2's branch). Expose it in `dm_report.h` and use it in the % of opt cell, or give the cell a text function that writes the shown percent; one decimals owner either way.
- Finding 6 is answered: D103 item 16 keeps the Library's order. Make no change for it. Put one sentence in `TableView`'s text-sort comment naming D103 item 16, without restating the rule.

**3. One dash.** T2 made `report::kDash` (`report.h`) the dash's owner and added a scan row for it. T3 made a second `kDash` in `report_view.h`. Delete T3's and use `report::kDash`. Run the scan row and confirm it would have flagged T3's line.

**4. D103 item 17: an empty Posted date sinks.** In `dm_report_view.cpp`, Posted's sort key is empty when the date is empty, so it sinks in both directions like every other empty value. Pin it in `tests/test_report_view.cpp`, failing first.

**5. D103 item 18: the count line uses `counted`.** `count_line` takes the singular and plural nouns and writes the noun with `hydra::counted`'s rule on the total (`core/model.h:863`), so "1 of 1 path" and "3 of 7 paths". Don't restate the singular-at-1 rule; call `counted` and reuse its noun. The two `kNoun` constants become singular and plural pairs. Pin "1 of 1 path", "0 of 1 path" and "4 of 4 scores", failing first.

## Owned files

`src/app/report.h`, `src/app/report.cpp`, `src/app/dm_report.h`, `src/app/dm_report.cpp`, `src/app/report_view.{h,cpp}`, `src/app/path_report_view.{h,cpp}`, `src/app/dm_report_view.{h,cpp}`, `src/core/squeeze_rating.h` only if finding 3's accessor goes there, `tests/test_report.cpp`, `tests/test_dm_report.cpp`, `tests/test_report_view.cpp`, `tests/test_single_owner.cpp` (rows for these findings only), and `CMakeLists.txt` (the merge's lines only).

Owned-file check: every step is a fix inside the files T2 and T3 already changed, plus their scan rows; `squeeze_rating.h` is listed in case finding 3's owner lands beside `beyond_edge_ms`.

## Preflight

Command: `Grep "kPercentDecimals\s*=|kDash\s*=" .claude/worktrees` and `Grep "beyond_edge_ms|counted\(" src --glob *.h` in T2's worktree, run by the orchestrator on 2026-10-08.
Output: `kDash` at `rw-t2-data/src/app/report.h:73` and `rw-t3-view/src/app/report_view.h:25`; T2's dash scan row at `rw-t2-data/tests/test_single_owner.cpp:1349-1356`; `kPercentDecimals` private at `rw-t2-data/src/app/dm_report.cpp:178`; `beyond_edge_ms` declared at `src/core/squeeze_rating.h:198`; `counted` declared at `src/core/model.h:863`.

## Tests

`-sf=*test_report_view*`, `-sf=*test_report.cpp*`, `-sf=*test_dm_report*`, and the single-owner cases your rows touch, each once (`-tc="single-owner*"` is allowed for this task). Nothing else.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
