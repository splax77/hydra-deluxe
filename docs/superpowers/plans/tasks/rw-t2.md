Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T2: the data layer hands over tiles and text

Task id: RW-T2. Base: the commit your prompt gives. Branch `claude/rw-t2-data`, worktree `.claude\worktrees\rw-t2-data`; make it as the preamble says.

Your spec is the plan's "Task 2" section, plus the spec's "How the code is arranged" paragraphs on the data layer and the tiles. This brief adds only what the plan leaves to dispatch time.

## What this brief adds

**Where today's rules live.** The page's five tiles are computed in the path page's JavaScript in `src/app/report.cpp` (around line 182, the `Past ... ms` tile). The nine dm tiles, the average % of optimal and the points left are in the dm page's JavaScript in `src/app/dm_report.cpp` (around lines 124-140), next to its mirror of `tally_dm_rows`. `STATUS_CLASS` is at `dm_report.cpp:70`. Read those to learn what each tile shows, then write the C++ so it gives the same answer. The JavaScript stays until T7.

**Charts shown.** D103 item 6 renames the Charts tile "Charts shown" and makes it count the charts among the rows that pass the filters. So `path_tiles` takes the rows the window shows, not all rows, and the label is "Charts shown". The subtitle keeps counting every chart through the same "copies once per chart" rule. That rule lives in `page_charts` (`report.cpp:234`), used by the page builder (line 556) and by the subtitle count (line 653). Make one function own it, called by both `path_tiles` and the subtitle; `page_charts` may stay as that function if `path_tiles` calls it. The page's own Charts tile keeps its old label until T7 deletes the page; don't change page text.

**The chip token.** `status_token(status)` returns a token naming the chip colour for a dm status, the C++ home of the page's `STATUS_CLASS`. T5 later adds `chip_color(token)` in the theme, which also colours the path report's timing tiers (`--t0` to `--t5` and `--tn` in `html_page.cpp:146-147`). So make the token a small enum in an app header both reports can include, naming those seven tier colours and the dim colour, and map each status to one. Name the members after the page's CSS variables or classes, so the mapping is checkable against the page. The path report's tier-to-token mapping is not yours; leave it to T3 or T5, but make the enum able to express it.

**Number text.** Use the existing text functions for every number: `format_percent` and `percent_steps` (`app/display_format.h`) for the average, and whatever the page's payload already uses for thousands separators. Find the C++ owner of the thousands separator before writing one; if none exists in C++, stop and report it.

**The sample rows.** The plan's "six sample path rows from the samples test" are built in `tests/test_report.cpp`'s skipped samples case (line 1003). The four sample scores are the dm sample in the same case or in `tests/test_dm_report.cpp`. If the rows are built inline inside the skipped case, move the builder into a helper in the same file so both the samples case and your new cases call it; don't copy it.

**The dm literals** the plan gives (4, 1, 0, 1, 1, 1, 0, "98.80%", "3,456") are the spec's, read off today's sample page. If your `dm_tiles` gives a different value on the four sample scores, stop and report both values; never change the expected literal to match.

## Owned files

`src/app/report.h`, `src/app/report.cpp`, `src/app/dm_report.h`, `src/app/dm_report.cpp`, `tests/test_report.cpp`, `tests/test_dm_report.cpp`.

Owned-file check: every T2 step adds to the two result structs, the two report files and their two test files; the chip-token enum goes in `report.h` or `dm_report.h`, whichever the other includes, so no new file is needed.

## Review fix round (added by the orchestrator, 2026-10-08)

The derive-once review's first two findings need files this brief didn't own. For the fix round only, you may also edit:

- `tests/test_single_owner.cpp`, only the rows for the "Not analyzed" tile label (finding 1) and a new "What text shows where a value is missing?" row owned by `report::kDash` (finding 2). Run `-tc="single-owner rules hold across src/*"` and the other single-owner cases you touch, once each.
- `src/app/fill_report.cpp`, only its two em dash literals near lines 297 and 300, which become `report::kDash` (finding 2).

T3 added its own `kDash` in `src/app/report_view.h` in parallel. Leave it; the orchestrator joins the two at the merge, keeping yours in `report.h`. Write the scan row so its must-match examples include a literal em dash like T3's, so the join is checked.

## Preflight

Command: `Grep "struct GeneratedReport|struct GeneratedDmReport|page_charts|tally_dm_rows|STATUS_CLASS|beyond_edge_ms|subtitle|footer" src/app/{report,dm_report}.{h,cpp}` and `Grep TEST_CASE tests/test_{report,dm_report}.cpp`, run by the orchestrator on base 658abcf on 2026-10-08.
Output: `GeneratedReport` at `report.h:234` (holds `html`); `GeneratedDmReport` at `dm_report.h:106`; `page_charts` defined at `report.cpp:234`, called at 556 and 653; the path subtitle built at `report.cpp:666`, footer at 673, `out.html = build_html(...)` at 679; `tally_dm_rows` declared `dm_report.h:98`, defined `dm_report.cpp:319`; dm subtitle at `dm_report.cpp:357`, footer at 360, `out.html` at 366; `STATUS_CLASS` at `dm_report.cpp:70`; the JS mirror of the tally at `dm_report.cpp:124`. Test cases: "path page: the Charts tile adds up the copies generate_report adds up" (`test_report.cpp:315`), "path report counts charts by hash in the tile and the subtitle" (1328), the samples case (1003, skipped), "generate_dm_report: tally and framing behind one seam" (`test_dm_report.cpp:232`), "build_dm_html: the average tile reads a percent the way the cells do" (369).

## Tests

`-sf=*test_report.cpp*` and `-sf=*test_dm_report*`, with `-tc=` filters on your new cases plus the existing cases that pin `page_charts`, the subtitle and the tally (named above), since you touch them.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
