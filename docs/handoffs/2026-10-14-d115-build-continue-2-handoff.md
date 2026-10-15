# Handoff: continue building D115 after A2-2 (one record per chart and profile)

Written 2026-10-14 by session 6cc6ff7a-5288-4f56-b2e4-dee239466dd5, when the user said "stop after the merge and create a handoff". It replaces `docs/handoffs/2026-10-14-d115-build-continue-handoff.md` as the place to start. That file still holds the decisions made before this session, and it stays valid. The plan is still `docs/superpowers/plans/2026-10-12-one-record-per-chart.md`, with its part files in `docs/superpowers/plans/2026-10-12-one-record-per-chart/`.

## Where things stand

Main is at 7c3adacb, the A2-2 merge. Nothing is pushed. No agent is running: every agent this session started has handed back its report. The journal's list of "unfinished" agents (a1e26ca073bb5f435, a3afd1a240d803ee3, a3f67ea84ff0f529f, a6360b82e2642a3f8) only means their last call was the hand-back; all four reported and are done.

This session merged three tasks, each after its one fresh reviewer signed off and the derive-once gate passed:

- A2-3 at 52e71f9f (reviewed clean on 43343920). The batch, the click, hydra_batch and tools/bench.cpp save through `prepare_record` and `RecordStore::save`. The batch's ReportSeed hand-over is gone. A click that saves marks both open reports out of date. `plan_batch` takes its copies from `ChartIndex`.
- A2-4 at ce5a5a01 (reviewed clean on e6f2095a). The leaderboard and fill comparison pages read one profile each. `report::listed_records` is the one filter of a profile's Ready records to the charts the library lists. `compare_db.py` matches the three record tables across files by each profile's own columns.
- A2-2 at 7c3adacb (reviewed clean on bb6d5ead). The path report is one database read of the current profile's Ready records and analyzes nothing. The Mode column is gone, with shares Song 31, Artist 17, Charter 16, Path 36. A chart mode switch marks the report out of date. The empty database reads "Nothing is analyzed yet. Click a song or run a batch to analyze it." The dead "Left out" line, its Show files strip, `failures` and the progress overlay are deleted. The report's analysis seam is now `set_report_hold_for_test`.

After each merge the main session built main and ran the test files the change touched, plus the single-owner scan and the perf digest. All passed. After A2-2, main builds 1437 test cases (doctest's count; the drop from 1440 is the deleted seed, progress and left-out tests). The full suite was not run, under the new CLAUDE.md rule 1 (only the touched tests, unless the change touches shared code). The perf digest (6 cases) never moved.

A2-3 and A2-4 ran side by side, against the plan's serial order, because they owned separate regions of the files they shared. The rule files merged cleanly.

## User decisions made this session

1. The empty-under-settings sentence names the chart mode first ("yes go with recommend", 2026-10-14). The reviewer built it from the one mode spelling, `app::chartmode_key`. The path report now reads: "Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, SP cap 8, Clone Hero 1.1 fills). Analyze with these settings, or change them."

## Open questions for the user

Ask these at the start of the next session. Neither blocks A1-5.

1. The sentence above names "2x Bass", which the user's example ("Expert Pro Drums, SP cap 4, ...") did not. `chartmode_key` always names the bass setting, so dropping it needs a second spelling of the mode. Keep it, or drop it?
2. The fill comparison page shares `nothing_under_settings`, so its empty sentence changed too. It now puts the mode first: "Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, SP cap 4) in either database. Analyze with these settings, or change them." Is that fine?
3. A2-4's reviewer noticed one small difference on the fill comparison page. When a chart has a record only in the 1.0 database, the page takes its name from the 1.0 library, as it did before this work. The main session's brief had said "names from the 1.1 library first". It was left as before. The main session told the user, and they have not answered yet.

Two calls the main session made, told to the user in chat with no objection: the leaderboard window's row click keeps its mode switch (the user's answer 2 covered the path report only), and the dead "Left out" line was deleted (no visible change).

## Next: A1-5, split four ways

A1-5 deletes the old store API (part A1, section "A1-5: the cleanup"). It is too big for one executor. A grep at main 138e5587, before A2-2, still found old calls in about 5 source files, two tools, the perf digest and roughly 20 test files. Recount on 7c3adacb first: A2-2 removed the report.cpp callers of `list_records`, `RecordListing`, `SortColumn`, `counts()`, `library_copies`, `naming_copy_paths`, `copies_of`, `kNamingCopiesSql` and `summarize_path`.

The proposed split, which the user has not seen yet. The plan is approved, so this is an execution detail, but say it in one line before dispatch:

- **A1-5a, the app's library code.** `src/ui/library_model.*`, `library_table.cpp`, `app_state.cpp` and `library_jobs.cpp` move off `get_summaries`, `SummaryLookup`, `counts()`, `library_copies` and `summarize_record`, onto `lookup`, `RecordLookup`, `ChartIndex` and a new `record_count()`. `BatchRun` holds a `store::Profile` in place of `chartmode`, `lens`, `cap_query()` and `BatchRun::profile()`. The readers of those fields are cli/batch.cpp, test_config.cpp, test_library_jobs.cpp, test_app_reports.cpp:455 and the hand-built runs in test_analysis.cpp. `Settings::record_key` leaves src. `src/app/user_messages.h` names `SummaryLookup` and `add_row`.
- **A1-5b, the tools and the digest.** `tools/bench.cpp`'s engine mode (`prepare_row`), `tests/song_digest.h` (`digest::row_hash` takes a `PreparedRow`) and `tests/test_perf_digest.cpp` move to `PreparedRecord`, folding the same facts in the same order. **The six digests must not change.** If one moves, stop and report; never re-pin. `tools/replay_json.cpp`/`.h` and `tests/test_replay.cpp` move off `summarize_record` and `best_path_text`.
- **A1-5c, the tests.** `tests/display_fixtures.h`, `store_fixtures.h`, test_analysis.cpp's group tests (`RecordKey`, `get_summary`, `save_analysis`), test_app_click, test_app_settings, test_cli, test_config, test_library_model, test_long_paths, test_preview_controller and `tests/ui/uitest_details.cpp` move to the one save and the lookup. `tests/test_store_records.cpp` around line 166 moves from `store.counts()` to `record_count`. Check this task's file list against A1-5a's: they share test_library_model.cpp, test_config.cpp and test_analysis.cpp. Either give each file to one of them, or run c after a.
- **A1-5d, the deletion, after a to c merge.** This step deletes from `record_store.*` everything the part lists: `ResultSettings`, `SummaryLookup`, `RecordKey`, `PreparedRow`, `prepare_row`, `add_record`, `add_row`, `save_analysis`, `get_summary(ies)`, `has_record`, `analyzed_hashes`, `list_records`, `RecordListing`, `SortColumn`, `counts()`, `library_copies`, `naming_copy_paths`, `copies_of`, `kNamingCopiesSql`, `summarize_record`, `summarize_path`, `best_path_text` and `write_row`. It also deletes the interim PurgeScope split in `file_record`, the tests that tested only the deleted functions, and their single-owner rows and baseline entries. A test that pins a status rule moves to test_store_records.cpp. The app.json rows "How is a chart hash spelled for matching?" (~649, gains `normalize_chart_hash\(` with dmbot_client.cpp exempt) and the raw-row-count row (~1599, points at `record_count()`) are A1-5d's. CONTEXT.md entries move to D1, per the plan.

Other notes for A1-5, from this session's reviewers:

- A2-3's reviewer added a test, "a BatchRun made from Settings has Settings::profile as its profile". It pins the two profile builders to one answer. It goes once `BatchRun` holds the profile.
- The batch's old "files results under the lens it is given" test is gone. The lens reaching the stored record is now covered only through that agreement test. A custom-lens batch test is optional.
- `tests/single_owner/rules/store.json`'s row "How many charts does a page or its tool count?" now names `ChartIndex::count` as owner, so it survives the deletion of `copies_of`.
- `ViewJob::key()` is now `profile()`.

## After A1-5

Wave 8 runs B1-6, then B1-7, then B1-8, one at a time (the plan's renames). Before each, grep its rename list against open tasks' files. Wave 9 is D1, the documents. Wave 10 is the real-app check on a copy of the user's database, then the push. A full library batch needs the user's yes in chat first (rule 8).

Additions to D1's list, on top of those in the earlier handoff:

- **ADR 0030 is taken.** MCP-T7 merged an ADR 0030 for the MCP server (138e5587). The plan's "ADR 0030 supersedes ADR 0026 in part" must take the next free number.
- `docs/UserGuide.md` line ~408 shows the old empty-report sentence; use the new one above. The guide's path report section (~354-410) still describes Mode, the building bar, "every mode" and the Left out strip.
- `CONTEXT.md`'s "Analyze" entry says the record "is never saved (docs/adr/0026)". Since A2-3, a click saves it.
- While the path report builds, its window shows the first line, placeholder rows and Cancel, with no moving bar. That is the plan's design; the real-app check shows it to the user.

## Promise to another session

The column-shares work moved to the session "Column shares handoff continuation" (pipe `uds:\\.\pipe\LOCAL\cc-msg-c6b89f2df3009031b844f7f91b8b1838`). It was sent A2-2's hash, 7c3adacb. It still wants **B1-7's merge hash**, because B1-6 and B1-7 rename `path.score_base`, which CS-T5b's `build_score_breakdown` in `src/app/path_view.cpp` reads. The old pipe in the earlier handoff is retired.

## How this session ran things

Each task got one Opus executor in its own worktree, briefed with `d115-exec-common.md`. Each change got one Sonnet reviewer, briefed with `d115-review-common.md`. When an executor ran out of tool calls, a finisher continued on the same branch. A2-2's finish was split in two worktrees (tests and rows, GUI tests) and joined by the main session. All three first executors ran out of budget before their tests were done, so budget A1-5's pieces small.

The two brief pages, now carrying this session's id, live in this session's scratchpad: `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6cc6ff7a-5288-4f56-b2e4-dee239466dd5\scratchpad\`, beside `d115-session2-notes.md`. The scratchpad is temporary, so copy them first, and change the `Session:` trailer line in both to the new session's id. The review files are in `C:\Users\Patrick\AppData\Local\Temp\claude\d115-review\` (A2-2.md, A2-3.md, A2-4.md this session).

Review keys come from `pwsh -NoProfile -File C:/Users/Patrick/.claude/hooks/main_move_check.ps1 <branch> -NoFF`. A merge on main needs the trailers Task, Agent and Session plus Co-Authored-By, and `git -C "<folder written out in full>"`; the hook refuses `git -C $VAR merge`. Never `cd` into a worktree from the main session's shell, because it moves the session's working folder.

Worktrees `.claude/worktrees/d115-a2-2`, `d115-a2-2ui`, `d115-a2-3` and `d115-a2-4` are merged and done with. The cleanup sweep removes them after 24 hours. The main checkout has an uncommitted `CLAUDE.md` edit and two untracked docs that belong to other sessions; stage by name.
