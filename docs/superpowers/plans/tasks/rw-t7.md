Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task T7: remove the pages, the plumbing and the CLI

Task id: RW-T7. Base: `1a0bdd8` (main, the wave-2 tip plus the wave-2 handoff). Branch `claude/rw-t7-delete`, worktree `.claude\worktrees\rw-t7-delete`; make it as the preamble says.

Your spec is the plan's "Task 7" section and the spec's "What gets deleted" (all of it). The wave-2 handoff, `docs/handoffs/2026-10-08-report-windows-wave2-handoff.md`, adds the moves below. In this wave the preamble's "pages still carry their own copies" rule ends: you are the task that deletes them.

**Deleting big blocks.** A hook denies an Edit that keeps under 40% of its block. To remove most of a file, Write the new file to `<file>.new` and `Move-Item` it over; remove a whole file with `git rm`.

## What this brief adds

**1. Move two helpers out of `html_page` first.** The report views use two `html::` functions for work that isn't HTML:
- `html::search_field` (`html_page.h:58`), called by `path_search_text` (`path_report_view.cpp:177`), `dm_search_text` (`dm_report_view.cpp:202`) and `fill_report.cpp:285`. Its natural home is beside `make_searchable` in `src/app/library_query.{h,cpp}`, since it is the report's search text built the library's way.
- `html::replace_all` (`html_page.h:15`), called by `dm_report_view.cpp:20` for the column definitions, by `fill_report.cpp:156` and inside `html_page.cpp`. Look for an existing plain string helper header in `src/app` or `src/util` first; use it if there is one, otherwise put it beside `search_field`.

Move each one; never leave a copy. Every caller, the fill page included, calls the new home. Then the path argument of `search_field` stays only if a caller still passes it (the path report's search does, so it likely stays; the spec's line about dropping it meant the page's JavaScript argument).

**2. Delete each symbol in the spec's "What gets deleted",** after grepping for every reader across `src`, `tests`, `tools`, `installer`, `CMakeLists.txt` and `.github`. Every wave-1 and wave-2 report listed the JavaScript copy its C++ replaced; those copies all live in the page strings you delete. Also unused by the GUI now: `report_outcome.h`, `publish_report`, `show_in_folder` and its seam, `report_file_exists`, the `open_*_in_browser` pair, the result structs' `html` field, and the uitest harness's browser and folder recorders.

**3. Keep what the fill page needs.** `open_in_browser`, `copy_to_short_temp` and `write_report_file` stay for `hydra_fillcompare`. Before you cut anything from `html_page.cpp`, run the samples test and render `fill.html` in headless Edge, before and after (ADR 0016's check). It must not change visibly. Attach both screenshots' paths in your report. That check, not judgment, settles which CSS and script pieces go.

**4. `hydra_report` goes,** with its CMake target, its install line, its place in `hydra_tests`' dependencies, and its two lines in `THIRD_PARTY_NOTICES.txt`. `tests/test_cli.cpp` runs it in three cases (lines 36, 126, 209, 371, 519 on the base):
- Drop it from the database-won't-open case and the `#if` guard.
- "hydra_report writes a page for a filled database..." goes with the page.
- "hydra_report reports a --legacy-fills database under the 1.0 rule" pins a real behaviour: a legacy-fills database's report follows the 1.0 rule. Keep that check by calling `generate_report` directly in `tests/test_report.cpp` on the same kind of database, and say so in your report. If the GUI can never build a report from a legacy-fills database, say that instead and ask whether the check still matters.

The comment at `src/app/config.h:108` names `hydra_report`; reword it. The comments naming it in `src/ui/library_jobs.cpp` (581, 606) and `tests/test_library_jobs.cpp` (345) are T6's files this wave; T6 rewords them.

**5. Tests that pin page HTML** are deleted or rewritten against the views and tile functions, as the spec lists: the page cases and `reports_dir` cases in `tests/test_report.cpp`, `tests/test_dm_report.cpp`'s page cases, and `tests/test_s2_offspeed.cpp`'s page cases ("The page counts the same statuses" goes, since there's no second count left). A page case that also pins something the views or tiles still own gets rewritten against them, not dropped; name each one in your report.

**6. `tests/test_single_owner.cpp`.** Remove the single-owner scans and clone-allowlist rows that name deleted code. The scan row for the comparison's status words skips HTML `<option value=...>` lines only because the dm page exists; tighten it now. Leave the four older right-align copies in `known_copies` alone (a later task).

**7. Rows don't change.** The views, tiles and `generate_report` rows are untouched. You delete; you don't rewrite a rule.

## Shared files with T6 (same wave)

T6 wires the windows at the same time. The main session merges you first, then T6, and resolves any overlap.
- `src/app/report.{h,cpp}`: you own the page functions (`build_html`, `kBody`, `kPageJs`, `kTitle`, `py_repr`, `ms_text_into`, the page-only `beyond_edge_text` use, `page_charts` if it still exists) and `GeneratedReport::html`. T6 owns `ReportOptions` and the chart loop in `generate_report`. Don't touch T6's region. `beyond_edge_text` is public in `report.h` and the views call it; delete only page-only code.
- `tests/test_report.cpp`: you delete the page cases and add the legacy-fills case. T6 adds one case at the end.
- `tests/ui/uitest_batch_reports.cpp` and `tests/ui/uitest_harness.{h,cpp}`: you remove the browser and folder recorders and every call to them. T6 edits scripts' checks.

## Owned files

`src/app/report.{h,cpp}` (the region above), `src/app/dm_report.{h,cpp}`, `src/app/html_page.{h,cpp}`, `src/app/report_files.{h,cpp}`, `src/app/library_query.{h,cpp}` (the moved helpers), a string helper header if you use one, `src/app/fill_report.cpp` (callers only), `src/app/path_report_view.cpp` and `src/app/dm_report_view.cpp` (callers only), `src/app/config.h` (the comment), `src/ui/report_outcome.h` (delete), `src/ui/win32_dialogs.{h,cpp}`, `src/cli/report.cpp` (delete), `CMakeLists.txt`, `THIRD_PARTY_NOTICES.txt`, the installer script if it names `hydra_report`, `tests/test_report.cpp`, `tests/test_dm_report.cpp`, `tests/test_s2_offspeed.cpp`, `tests/test_single_owner.cpp`, `tests/test_cli.cpp`, `tests/ui/uitest_harness.{h,cpp}`, `tests/ui/uitest_batch_reports.cpp` (recorder calls only), and any test of the moved helpers.

This is about 25 files. Plan for the 100-call wrap-up: commit after each numbered job, in order, so a finisher can pick up at the next one.

## Preflight

Run by the orchestrator on main at `1a0bdd8` on 2026-10-08:
- `git grep -l hydra_report` outside `docs` and `.superpowers`: `CMakeLists.txt`, `THIRD_PARTY_NOTICES.txt`, `src/app/config.h`, `src/app/report.h`, `src/app/report_files.h`, `src/cli/report.cpp`, `src/ui/library_jobs.cpp`, `tests/test_cli.cpp`, `tests/test_library_jobs.cpp`, `tests/test_single_owner.cpp`.
- `html::search_field` callers: `fill_report.cpp:285`, `path_report_view.cpp:177`, `dm_report_view.cpp:202`, `dm_report.cpp:302`, `report.cpp:612` (the last two are page code you delete). `html::replace_all` callers: `fill_report.cpp:156-157`, `dm_report_view.cpp:20-21`, `dm_report.cpp:170-171` (page code).
- Full suite on `1a0bdd8`: 1,223 cases pass (3 skipped), every `hydra_uitest --all` script passes.

## Tests

`-sf=*test_report.cpp*`, `-sf=*test_dm_report*`, `-sf=*test_s2_offspeed*`, `-sf=*test_single_owner*`, `-sf=*test_cli*`, `-sf=*test_fill_report*` if it exists (otherwise the fill cases in `test_report.cpp`), `-sf=*test_library_query*` for the moved helper, `-sf=*test_report_view*` (the callers moved), and `hydra_uitest` on the scripts in `uitest_batch_reports.cpp` by name. Never the full suite, never `--all`.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
