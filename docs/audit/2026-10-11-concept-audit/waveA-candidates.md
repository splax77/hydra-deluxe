# Wave A candidates (headings and homes only)

Pulled from the wave A reports by script. Read the report itself (named on each block) only if you need the tables.

## From G1a.md

### G1a-1. Does Hydra have a result for this chart?
Looks like: K2 (same word family, different rules; the words are on screen).
Homes:
- Library: `status_label`, src/ui/library_model.cpp 67-74, fed by `get_summaries` (inputs: chart, chart mode, the settings' SP cap, the lens). Also `best_path_label` 98-106.
- DM page: `collect_dm_rows`, src/app/dm_report.cpp 51-132, lines 93-127. Inputs: the score's identifier, `records_by_hash` at Clone Hero's cap (4) and the lens, `library_copies_by_hash`. `records_by_hash` is src/app/report.cpp 155-169 and reads `list_records`.
- Path report: `collect_rows`, src/app/report.cpp 201-315, reads `list_records` directly (lines 232-252).
- Fill page: `collect_fill_rows`, src/app/fill_report.cpp 167-244, reads `records_by_hash` once per database.

### G1a-2. What is each state called, and how many spellings does one state have?
Looks like: K2 (one state, several wordings, all on screen; for the fill page, one word with two meanings).

### G1a-3. How is a text column sorted?
Looks like: K2 (two sort orders for the same visible column; the windows' order is a recorded decision, the fill page is not covered by it).
Homes:
- `TableView::resort` with `folded_key` and `compare_keys`, src/app/report_view.h 178-205 and report_view.cpp 29-49. Inputs: the column's `SortKey` per row. Text goes through `fold_for_search`, then a plain byte compare. The comment (report_view.h 78-80) says the windows take "the Library table's text order, not the pages'" (D103 item 16).
- The page script's `render`, src/app/html_page.cpp 332-339. Inputs: the raw cell value. A string is compared with `localeCompare` (the browser's language order, no fold). A number is subtracted. Null sorts last in both directions. One sort key only.
- Third home, outside my group: `LibraryModel::resort`, src/ui/library_model.cpp 225-273, compares folded `searchable` strings the same way as the first home (so the first home agrees with the library; G1b or G3 to confirm).

### G1a-4. How is a "shown of total" count line written, and what does it count?
Looks like: K2 at the edge of one (n = 1); K3 for the rest.
Homes:
- `report_view::count_line`, report_view.cpp 51-54, used by both windows through `TableView::count_line` with `kNoun` = {"path","paths"} and {"score","scores"}. Writes `group_thousands(shown) + " of " + counted(total, one, many)`. Counts rows.
- The page script, html_page.cpp 389-390: `fmt(countOf(rows)) + ' of ' + fmt(countOf(ROWS)) + ' ' + PAGE.noun`. The fill page sets `noun: 'charts'` and `count: charts` (fill_report.cpp 90-95), where `charts` adds each row's `k` (library copies). Counts library copies, on purpose (D79).

### G1a-5. Small view helpers copied between the two window views
Looks like: K1 (same rule, two homes; the tables agree on every input I could think of).
Homes:
- `text_column`, src/app/path_report_view.cpp 26-36 (inputs: id, title, definition, member pointer to a string field, look) and `text_column`, src/app/dm_report_view.cpp 37-44 (same, without the definition argument; it reaches `column()` with an empty definition). Both set `sort_key` to `SortKey{r.*field}` and `cell` to `r.*field`, and `numeric` false.
- `ms_key`, path_report_view.cpp 21-23 (`optional<double>` to number or empty key) and `optional_key<T>`, dm_report_view.cpp 46-49 (any `optional<T>` to number or empty key).
- The missing-value cell: `ms_cell` (path_report_view.cpp 17-19, dash for nullopt) and the inline lambdas in `dm_columns` for optimal (80), delta (92), rank (146) and posted (158), each writing `kDash` for a missing value. `count_column` (path 40-54) has no DM twin; DM writes its number columns inline.

### G1a-6. Which settings does the path report read, and which settings make an open report out of date?
Looks like: K1 with a gap (two homes of one list; they disagree on one input). Group G3 also reads the `settings_change_touches` pair; this is from the report side.
Homes:
- The list as the report is built: `AppState::launch_path_report` and `request_path_report`, src/ui/app_state.cpp 698-718. It passes `settings.cap_query()`, `settings.lens()`, `settings.batch_run()` and `settings.hit_window_ms` to `ReportJob`, which hands them to `generate_report` (src/ui/library_jobs.cpp 587-608). In the report, `hit_window_ms` labels every row's tier (report.cpp 299, `place_rows`), names the "Past N ms" tile and the footer, and is kept as `GeneratedReport::hit_window_ms`, which the window then uses for the Timing chip and the dropdown (path_report_window.cpp 86, 98, 111).
- The list as it is checked: `report::settings_change_touches`, src/app/report.cpp 431-437: `before.cap_query() != after.cap_query() || before.lens() != after.lens()`. Called from `AppState::apply_settings`, app_state.cpp 866-870.
- The DM page, same question: `dm_report::settings_change_touches`, dm_report.cpp 237-241: `chartmode_key` and `lens`. `launch_dm_report` (app_state.cpp 681-686) builds the job from `chartmode_key()` and `lens()`, so these two agree. The cap is forced to Clone Hero's in `collect_dm_rows`, so leaving it out is correct.
- Third home, outside my group: `AppState::apply_settings`, app_state.cpp 858-859, which checks `chartmode_key`, `cap_query` and `lens` for the library.

### G1a-7. "Keep listed records whose chart the library lists": one rule, two call sites
Looks like: K1 (agree on every input).
Homes:
- `records_by_hash`, report.cpp 155-169: for one chart mode, drops a listing unless `library_lists(library, normalize_chart_hash(r.hyhash))`, keys by the normalized hash. Used by the DM page and the fill page.
- `collect_rows`, report.cpp 232-237: for every mode, the same drop through `library_lists(copies, hash)`.

### G1a-8. Words the fill page's script and the two windows both write
Looks like: K1 (the same words in two languages; agree today).
Homes: the fill page's HTML and script in fill_report.cpp / html_page.cpp, and the windows in src/ui.

## From G1b.md

### G1b-1. What text does a name column sort by (title, artist, charter)?
Looks like: K2 (the library sorts on folded raw text, the report windows sort on folded display text; the user sees both orders).
Homes: `LibraryModel::resort` (key lambda lines 262-271) with `make_searchable` (`app/library_query.cpp` 362-370), `ui/library_model.cpp` 225-276 (inputs: the scan's raw title, artist, charter, folder);
       `TableView::resort` + `folded_key` (`app/report_view.h` 178-205, `app/report_view.cpp` 29-34) fed by the report columns' `sort_key` (`path_report_view.cpp` 26-36, `dm_report_view.cpp` 37-44), which read `ReportRow::song/artist/charter` (inputs: the display strings set by `display_title/artist/charter`, `report.cpp` 52-54, `dm_report.cpp` 100-103).
Homes: `LibraryModel::resort` (key lambda lines 262-271) with `make_searchable` (`app/library_query.cpp` 362-370), `ui/library_model.cpp` 225-276 (inputs: the scan's raw title, artist, charter, folder);
       `TableView::resort` + `folded_key` (`app/report_view.h` 178-205, `app/report_view.cpp` 29-34) fed by the report columns' `sort_key` (`path_report_view.cpp` 26-36, `dm_report_view.cpp` 37-44), which read `ReportRow::song/artist/charter` (inputs: the display strings set by `display_title/artist/charter`, `report.cpp` 52-54, `dm_report.cpp` 100-103).

### G1b-2. How do ties and missing values order in a sorted table?
Looks like: K2 (same sort header, two rule sets; some parts agree on purpose).
Homes: `LibraryModel::resort` + `unscored_rank` + `tie_break`, `ui/library_model.cpp` 23-32 and 225-276 (inputs: sort column, direction, score presence, status, folded title/folder, notespath);
       `TableView::resort`, `app/report_view.h` 178-205 (inputs: up to two `SortSpec`s, `SortKey` variant, row order).
Homes: `LibraryModel::resort` + `unscored_rank` + `tie_break`, `ui/library_model.cpp` 23-32 and 225-276 (inputs: sort column, direction, score presence, status, folded title/folder, notespath);
       `TableView::resort`, `app/report_view.h` 178-205 (inputs: up to two `SortSpec`s, `SortKey` variant, row order).

### G1b-3. What does the search box match, and what does the user have to type?
Looks like: K2, written down as deliberate (D56 item 1, comment in `report_view.h` 69-72).
Homes: `parse_library_query` + `query_matches` + `term_matches`, `app/library_query.cpp` 312-396 (inputs: the typed text; the row's four folded fields; stars and hardest ms facts);
       `search_words` + `search_matches`, `app/report_view.cpp` 8-27 (inputs: the typed text; one joined search string per row built by `search_field`, `library_query.cpp` 372-382).
Homes: `parse_library_query` + `query_matches` + `term_matches`, `app/library_query.cpp` 312-396 (inputs: the typed text; the row's four folded fields; stars and hardest ms facts);
       `search_words` + `search_matches`, `app/report_view.cpp` 8-27 (inputs: the typed text; one joined search string per row built by `search_field`, `library_query.cpp` 372-382).

### G1b-4. Does Hydra have a result for this chart ("Not analyzed", "No paths", "Not in your library")?
Looks like: K2 (the same status words, different inputs and a different lumping of Stale).
Homes: `get_summaries` + `rank_row`/`WinnerPicker` -> `RecordStatus` -> `status_label` (`store/record_store.cpp` 1307-1380, `ui/library_model.cpp` 67-74; inputs: chart hash, chartmode, **`settings.cap_query()`**, lens; called from `ui/library_jobs.cpp` 67-68);
       `collect_dm_rows` (`app/dm_report.cpp` 93-128) with `records_by_hash` -> `list_records` (`app/report.cpp` 155-169, `store/record_store.cpp` 1440-1503; inputs: chart hash, chartmode, **`CapQuery::at(kCloneHeroSpCap)`**, lens, the library's chart list).
Homes: `get_summaries` + `rank_row`/`WinnerPicker` -> `RecordStatus` -> `status_label` (`store/record_store.cpp` 1307-1380, `ui/library_model.cpp` 67-74; inputs: chart hash, chartmode, **`settings.cap_query()`**, lens; called from `ui/library_jobs.cpp` 67-68);
       `collect_dm_rows` (`app/dm_report.cpp` 93-128) with `records_by_hash` -> `list_records` (`app/report.cpp` 155-169, `store/record_store.cpp` 1440-1503; inputs: chart hash, chartmode, **`CapQuery::at(kCloneHeroSpCap)`**, lens, the library's chart list).

### G1b-5. How is "X of Y things" written, and when is the "of Y" left out?
Looks like: K2 (small, visible text).
Homes: `render_heading`, `ui/library_table.cpp` 60-69 (inputs: shown rows, all rows, `counted`, `group_thousands`);
       `report_view::count_line`, `app/report_view.cpp` 51-54, used by `TableView::count_line` (inputs: shown, total, `CountNoun`);
       the page script, `app/html_page.cpp` 389-390 (inputs: `countOf(rows)`, `countOf(ROWS)`, `PAGE.noun`).
Homes: `render_heading`, `ui/library_table.cpp` 60-69 (inputs: shown rows, all rows, `counted`, `group_thousands`);
       `report_view::count_line`, `app/report_view.cpp` 51-54, used by `TableView::count_line` (inputs: shown, total, `CountNoun`);
       the page script, `app/html_page.cpp` 389-390 (inputs: `countOf(rows)`, `countOf(ROWS)`, `PAGE.noun`).

### G1b-6. The report table written again in JavaScript (page script)
Looks like: K2 (the page and the windows were one set of rules; D103 moved the windows to the library's text order and left the page).
Homes: `kReportJs`, `app/html_page.cpp` 303-402 (`visible()` for search, the `rows.sort` comparator, the empty text, the count line);
       `TableView` + `search_words` + `compare_keys` + `count_line` + `nothing_matches` (`app/report_view.*`, `ui/report_window.cpp` 334-338).
Homes: `kReportJs`, `app/html_page.cpp` 303-402 (`visible()` for search, the `rows.sort` comparator, the empty text, the count line);
       `TableView` + `search_words` + `compare_keys` + `count_line` + `nothing_matches` (`app/report_view.*`, `ui/report_window.cpp` 334-338).

### G1b-7. What a report window forgets, and when (close, reopen, refresh)
Looks like: K3-with-a-rule-inside (two mechanisms for one product rule).
Homes: path report: `PathWindow::rebuild(keep)` (`path_report_window.cpp` 66-104), the `drop_rows` on close (172-178), `PathReportInput::keep_filters`, `ReportSlot::built_on_open` and `close_path_report` (`app_state.cpp` 730-739, 781-782);
       DM report: `DmWindow::rebuild` (`dm_report_window.cpp` 50-69) with no keep flag, no close handler (`dm_report_input` sets none), and nothing on close.
Homes: path report: `PathWindow::rebuild(keep)` (`path_report_window.cpp` 66-104), the `drop_rows` on close (172-178), `PathReportInput::keep_filters`, `ReportSlot::built_on_open` and `close_path_report` (`app_state.cpp` 730-739, 781-782);
       DM report: `DmWindow::rebuild` (`dm_report_window.cpp` 50-69) with no keep flag, no close handler (`dm_report_input` sets none), and nothing on close.

### G1b-8. What "Clear filters" resets, and what "no filter" means
Looks like: K3-with-a-rule-inside.
Homes: `PathWindow` (`path_report_window.cpp` 29-43 field defaults, 74-77 rebuild resets, 123-130 Clear filters, 51-56 apply_filters);
       `DmWindow` (`dm_report_window.cpp` 25-36 field defaults, 54 rebuild reset, 84-90 Clear filters, 44-48 apply_filters).
Homes: `PathWindow` (`path_report_window.cpp` 29-43 field defaults, 74-77 rebuild resets, 123-130 Clear filters, 51-56 apply_filters);
       `DmWindow` (`dm_report_window.cpp` 25-36 field defaults, 54 rebuild reset, 84-90 Clear filters, 44-48 apply_filters).

### G1b-9. LibraryRow copies the five fields of SummaryLookup, and a hand-written "did it change" test
Looks like: K3-with-a-rule-inside (small).
Homes: `LibraryRow` (`ui/library_model.h` 60-71: `status`, `stale_build`, `stale_rules`, `bestpath`, `summary`) and `apply_summary` (`library_model.cpp` 51-63);
       `store::SummaryLookup` (`store/record_store.h` 210-221: the same five fields) and `PathSummary::operator==` (`record_store.h` 81-87, compares all ten fields).
Homes: `LibraryRow` (`ui/library_model.h` 60-71: `status`, `stale_build`, `stale_rules`, `bestpath`, `summary`) and `apply_summary` (`library_model.cpp` 51-63);
       `store::SummaryLookup` (`store/record_store.h` 210-221: the same five fields) and `PathSummary::operator==` (`record_store.h` 81-87, compares all ten fields).

## From G2.md

### G2-1. What does one row of the `charts` table say about a chart file, and who reads it into a struct?
Looks like: K1 (the column list and the field order), with a deliberate filter inside the cache (a rule that has one owner, `sig_can_show_unchanged`).
Homes: `RecordStore::chart_library_cache`, record_store.cpp 1553-1573 (inputs: the `charts` table, the stamp `chart_meta_current()`, `sig_can_show_unchanged`); `read_library_entries` with `kLibraryEntrySelect`, record_store.cpp 1591-1612 (inputs: the same table, the same stamp, passed in as `timing_read`).
Homes: `RecordStore::chart_library_cache`, record_store.cpp 1553-1573 (inputs: the `charts` table, the stamp `chart_meta_current()`, `sig_can_show_unchanged`); `read_library_entries` with `kLibraryEntrySelect`, record_store.cpp 1591-1612 (inputs: the same table, the same stamp, passed in as `timing_read`).

### G2-2. How does a scan row become a library row, and back?
Looks like: K3 (parallel on purpose with named converters), with one real difference: the types hold identical fields.
Homes: `ScanItem`, analysis.h 36-48 (md5, title, artist, charter, notespath, rootfolder, sig, timing); `ChartLibraryEntry`, record_store.h 262-274 (the same eight fields in the same order). Converters: `to_library_entry`, analysis.cpp 421-432 (all eight fields); `scan_item_of`, library_jobs.cpp 280-290 (seven fields; `sig` is left empty because "nothing the batch runs reads it", library_jobs.h 227-230).
Homes: `ScanItem`, analysis.h 36-48 (md5, title, artist, charter, notespath, rootfolder, sig, timing); `ChartLibraryEntry`, record_store.h 262-274 (the same eight fields in the same order). Converters: `to_library_entry`, analysis.cpp 421-432 (all eight fields); `scan_item_of`, library_jobs.cpp 280-290 (seven fields; `sig` is left empty because "nothing the batch runs reads it", library_jobs.h 227-230).

### G2-3. Where is a ScanItem filled from its source, and how many ways?
Looks like: K1 (two inline fill blocks in one function plus the metadata step before them).
Homes: discover_charts cache-hit block, analysis.cpp 669-686 (inputs: a ChartCacheEntry, a PendingChart); discover_charts fresh-read block, analysis.cpp 687-703 (inputs: `song_id_read` result, a ChartMeta, a PendingChart); then one shared fallback loop, 719-731 (`title_or_unknown`, `artist_or_unknown`, `charter_or_unknown`).
Homes: discover_charts cache-hit block, analysis.cpp 669-686 (inputs: a ChartCacheEntry, a PendingChart); discover_charts fresh-read block, analysis.cpp 687-703 (inputs: `song_id_read` result, a ChartMeta, a PendingChart); then one shared fallback loop, 719-731 (`title_or_unknown`, `artist_or_unknown`, `charter_or_unknown`).

### G2-4. Which metadata key means which field, for .ini, .sng and .srb?
Looks like: K1 (key-to-field mapping, written once per container) with a K2 edge on empty values for .srb.
Homes: `read_metadata_ini`, analysis.cpp 151-164; `parse_sng_metadata`, analysis.cpp 173-185; `srb_chart_meta`, analysis.cpp 194-204. The delay key is read by `ini_delay_ms` and `sng_metadata_delay_ms` in app/preview_source.cpp 181-187 and 300-304 (outside the group), which share `parse_delay_ms`.
Homes: `read_metadata_ini`, analysis.cpp 151-164; `parse_sng_metadata`, analysis.cpp 173-185; `srb_chart_meta`, analysis.cpp 194-204. The delay key is read by `ini_delay_ms` and `sng_metadata_delay_ms` in app/preview_source.cpp 181-187 and 300-304 (outside the group), which share `parse_delay_ms`.

### G2-5. What counts does a batch report, and who words them?
Looks like: K2 on wording, K1 on the counts (counts come from one owner).
Homes: `BatchProgress`, analysis.h 217-224, written only by `run_batch` (analysis.cpp 880-913); `BatchJob::Snapshot`, library_jobs.h 271-298, copied from BatchProgress in one lambda at library_jobs.cpp 422-429 (total, completed, analyzed, skipped, failed; five of six fields). Wording homes: `batch_counts` in ui/library_dialogs.cpp 54-58 and the CLI summary printf in cli/batch.cpp 260-263.
Homes: `BatchProgress`, analysis.h 217-224, written only by `run_batch` (analysis.cpp 880-913); `BatchJob::Snapshot`, library_jobs.h 271-298, copied from BatchProgress in one lambda at library_jobs.cpp 422-429 (total, completed, analyzed, skipped, failed; five of six fields). Wording homes: `batch_counts` in ui/library_dialogs.cpp 54-58 and the CLI summary printf in cli/batch.cpp 260-263.

### G2-6. Which key does a chart's result go under for the current settings?
Looks like: K1, possibly K3 (BatchRun is built from Settings so the pieces cannot disagree).
Homes: `Settings::record_key`, config.cpp 346-348 (`RecordKey{hash, chartmode_key(), cap_query(), lens()}`); `run_batch`, analysis.cpp 955-956 (`RecordKey{item.md5, run.chartmode, cap, run.lens}` with `cap = run.cap_query()`); the loose triple in `read_library_summaries`, library_jobs.cpp 67-68, and in cli/fillcompare.cpp 78-80 (`chartmode_key()`, `cap_query()`, `lens()` called one by one).
Homes: `Settings::record_key`, config.cpp 346-348 (`RecordKey{hash, chartmode_key(), cap_query(), lens()}`); `run_batch`, analysis.cpp 955-956 (`RecordKey{item.md5, run.chartmode, cap, run.lens}` with `cap = run.cap_query()`); the loose triple in `read_library_summaries`, library_jobs.cpp 67-68, and in cli/fillcompare.cpp 78-80 (`chartmode_key()`, `cap_query()`, `lens()` called one by one).

### G2-7. What does a CLI tool do to start, and how does the app do the same?
Looks like: K1 between the two tools; K2 against the app (the failure policy differs on purpose).
Homes: cli/batch.cpp 95-116 and 137-143; cli/fillcompare.cpp 50-77 and 82-89; ui/app_state.cpp 23-31 and 40-46 (outside the group).
Homes: cli/batch.cpp 95-116 and 137-143; cli/fillcompare.cpp 50-77 and 82-89; ui/app_state.cpp 23-31 and 40-46 (outside the group).

### G2-8. How does Hydra open a chart file for reading, and what does it say when it cannot?
Looks like: K1, line-level.
Homes: `stream_md5` in analysis.cpp 121-123 (`fopen_utf8`, own throw of `KindedError(SongFileMissing, "cannot open file: " + path)`); `file_byte_source` in core/winstr.cpp 238-243 (`CreateFileW`, the same kind and message). Also audio/mapped_file.cpp 25-46 (G9) throws the same text four times.
Homes: `stream_md5` in analysis.cpp 121-123 (`fopen_utf8`, own throw of `KindedError(SongFileMissing, "cannot open file: " + path)`); `file_byte_source` in core/winstr.cpp 238-243 (`CreateFileW`, the same kind and message). Also audio/mapped_file.cpp 25-46 (G9) throws the same text four times.

## From G3.md

### G3-1. Which settings pick a stored result? (the chart mode, cap and lens, kept as three loose parts)
Looks like: K3-with-a-rule-inside. The shared framework is `RecordKey`. The per-module parts should be only facts. Several parts hold a rule: "which parts must match" and "which settings count".
Homes: `Settings::record_key`, `app/config.cpp` 346-348 (hash, chartmode_key, cap_query, lens);
       `AppState::apply_settings`, `ui/app_state.cpp` 858-859 (chartmode_key, cap_query, lens compared one by one);
       `report::settings_change_touches`, `app/report.cpp` 431-437 (cap_query, lens);
       `dm_report::settings_change_touches`, `app/dm_report.cpp` 237-241 (chartmode_key, lens);
       `run_batch` seed guard, `app/analysis.cpp` 877 (seed chartmode, cap, lens against the run);
       `collect_rows` guards, `app/report.cpp` 207 and 210-211 (cap, lens, max_paths against options and seed);
       the loose-argument calls `library_jobs.cpp` 67-68, `app_state.cpp` 683-684 and 702, `cli/fillcompare.cpp` 78-80, `RecordStore::get_summaries`, `list_records` and `analyzed_hashes` (chartmode, cap, lens as three arguments).
Homes: `Settings::record_key`, `app/config.cpp` 346-348 (hash, chartmode_key, cap_query, lens);
       `AppState::apply_settings`, `ui/app_state.cpp` 858-859 (chartmode_key, cap_query, lens compared one by one);
       `report::settings_change_touches`, `app/report.cpp` 431-437 (cap_query, lens);
       `dm_report::settings_change_touches`, `app/dm_report.cpp` 237-241 (chartmode_key, lens);
       `run_batch` seed guard, `app/analysis.cpp` 877 (seed chartmode, cap, lens against the run);
       `collect_rows` guards, `app/report.cpp` 207 and 210-211 (cap, lens, max_paths against options and seed);
       the loose-argument calls `library_jobs.cpp` 67-68, `app_state.cpp` 683-684 and 702, `cli/fillcompare.cpp` 78-80, `RecordStore::get_summaries`, `list_records` and `analyzed_hashes` (chartmode, cap, lens as three arguments).

### G3-2. Can this chart mode be compared with the leaderboard? (two callers of one gate, different inputs)
Looks like: K2.
Homes: `why_not_comparable` called from the toolbar, `ui/library_toolbar.cpp` 104-107 (real `settings.difficulty()`, `settings.sp_cap`, `legacy_fills`, `view_noteshuffle`);
       `why_not_comparable` called from `collect_dm_rows`, `app/dm_report.cpp` 61-65 (difficulty is the literal `Difficulty::Expert`, cap is the literal `kCloneHeroSpCap`, legacy_fills from the lens, Note Shuffle read back from the chart mode text through `with_chartmode`).
Homes: `why_not_comparable` called from the toolbar, `ui/library_toolbar.cpp` 104-107 (real `settings.difficulty()`, `settings.sp_cap`, `legacy_fills`, `view_noteshuffle`);
       `why_not_comparable` called from `collect_dm_rows`, `app/dm_report.cpp` 61-65 (difficulty is the literal `Difficulty::Expert`, cap is the literal `kCloneHeroSpCap`, legacy_fills from the lens, Note Shuffle read back from the chart mode text through `with_chartmode`).

### G3-3. Which settings does the path report read? (a setting it reads that its out-of-date rule does not name)
Looks like: K1, hand list shorter than what the consumer reads.
Homes: `AppState::launch_path_report`, `ui/app_state.cpp` 705-713 (hands `settings.hit_window_ms` to `ReportJob`, which uses it for the timing tier labels, `app/report.cpp` 299 and 359);
       `report::settings_change_touches`, `app/report.cpp` 431-437 (cap_query, lens; no hit window).
Homes: `AppState::launch_path_report`, `ui/app_state.cpp` 705-713 (hands `settings.hit_window_ms` to `ReportJob`, which uses it for the timing tier labels, `app/report.cpp` 299 and 359);
       `report::settings_change_touches`, `app/report.cpp` 431-437 (cap_query, lens; no hit window).

### G3-4. Which settings does the Analysis panel own, and what is their set? (the ten fields spelled by hand in six or more places)
Looks like: K3-with-a-rule-inside (the rule is "which settings are in the set").
Homes: `with_analysis_defaults`, config.cpp 281-295 (10 fields);
       `to_analysis_settings`, config.cpp 315-328 (the same 10 fields, plus rules);
       the key trio `chartmode_key` + `cap_query` + `lens`, config.cpp 266-279 and 336-344 (the same 10 fields);
       `with_chartmode`, config.cpp 297-313 (4 of them, by loop);
       `settings_change_phrases`, `ui/library_dialogs.cpp` 104-123 (8 settings, as phrases);
       `batch_settings_summary`, library_dialogs.cpp 86-98 (8 settings, as five rows);
       `render_chart_group`, `render_rules_group`, `render_paths_group`, `ui/settings_bar.cpp` 81-194 (8 settings, 10 widgets).
Homes: `with_analysis_defaults`, config.cpp 281-295 (10 fields);
       `to_analysis_settings`, config.cpp 315-328 (the same 10 fields, plus rules);
       the key trio `chartmode_key` + `cap_query` + `lens`, config.cpp 266-279 and 336-344 (the same 10 fields);
       `with_chartmode`, config.cpp 297-313 (4 of them, by loop);
       `settings_change_phrases`, `ui/library_dialogs.cpp` 104-123 (8 settings, as phrases);
       `batch_settings_summary`, library_dialogs.cpp 86-98 (8 settings, as five rows);
       `render_chart_group`, `render_rules_group`, `render_paths_group`, `ui/settings_bar.cpp` 81-194 (8 settings, 10 widgets).

### G3-5. What does 0 and 1 mean for the score-range unit? (one switch kept as an enum and as two ints)
Looks like: K1.
Homes: `Settings::search_depth_mode`, config.cpp 249-251 (1 is points, anything else is scores);
       the panel combo `modes[] = {"scores","points"}` with `mode_idx = settings.depth_mode`, `ui/settings_bar.cpp` 168-173 (index 0 is scores, index 1 is points);
       `store::Lens::depth_mode`, `store/record_store.h` 120 ("0 = scores, 1 = points -- the INI's own ints"), filled raw by `Settings::lens()` (config.cpp 340-344);
       `hydra::settings_key`, pather.cpp 69 (`static_cast<int>(s.depth_mode)`, which relies on the enum order in `search/engine.h` 20).
Homes: `Settings::search_depth_mode`, config.cpp 249-251 (1 is points, anything else is scores);
       the panel combo `modes[] = {"scores","points"}` with `mode_idx = settings.depth_mode`, `ui/settings_bar.cpp` 168-173 (index 0 is scores, index 1 is points);
       `store::Lens::depth_mode`, `store/record_store.h` 120 ("0 = scores, 1 = points -- the INI's own ints"), filled raw by `Settings::lens()` (config.cpp 340-344);
       `hydra::settings_key`, pather.cpp 69 (`static_cast<int>(s.depth_mode)`, which relies on the enum order in `search/engine.h` 20).

### G3-6. What is each analysis setting called when it is shown in words? (same setting, different names on different surfaces)
Looks like: K2 (user-visible words).
Homes: panel row labels, `ui/settings_bar.cpp` 83-173 ("Difficulty", "Pro Drums", "2x Bass", "Note Shuffle", "SP cap", "1.0 fills", "Score range", "Path limit");
       summary phrases, `ui/library_dialogs.cpp` 104-123 (same labels; "Pro Drums off", "2x Bass off", "Note Shuffle off", "1.0 fills off", "Path limit off");
       batch confirm rows, library_dialogs.cpp 369-373 ("Difficulty", "SP cap", "Fills", "Score range", "Path limit");
       CLI header, `cli/batch.cpp` 182-189 ("Chart mode", "Depth", "SP cap", "Timing cap", "Squeeze win", "Fill rule");
       the comparison gate's sentences, dm_report.cpp 35-47 ("1.0 fills", "Note Shuffle", "SP cap");
       the INI keys, config.cpp 122-149 (`legacy_fills`, `mslimit_*`, `depth_*`).
Homes: panel row labels, `ui/settings_bar.cpp` 83-173 ("Difficulty", "Pro Drums", "2x Bass", "Note Shuffle", "SP cap", "1.0 fills", "Score range", "Path limit");
       summary phrases, `ui/library_dialogs.cpp` 104-123 (same labels; "Pro Drums off", "2x Bass off", "Note Shuffle off", "1.0 fills off", "Path limit off");
       batch confirm rows, library_dialogs.cpp 369-373 ("Difficulty", "SP cap", "Fills", "Score range", "Path limit");
       CLI header, `cli/batch.cpp` 182-189 ("Chart mode", "Depth", "SP cap", "Timing cap", "Squeeze win", "Fill rule");
       the comparison gate's sentences, dm_report.cpp 35-47 ("1.0 fills", "Note Shuffle", "SP cap");
       the INI keys, config.cpp 122-149 (`legacy_fills`, `mslimit_*`, `depth_*`).

### G3-7. Which of a record's key parts does the store check against the record? (three of four lens fields)
Looks like: K1, hand list shorter than the key.
Homes: `store::prepare_row`, `store/record_store.cpp` 546-574 (checks cap, ms limit, fill rule);
       `store::Lens` and `RecordKey`, record_store.h 115-160 (cap, ms on/off, ms value, depth mode, depth value, fill rule).
Homes: `store::prepare_row`, `store/record_store.cpp` 546-574 (checks cap, ms limit, fill rule);
       `store::Lens` and `RecordKey`, record_store.h 115-160 (cap, ms on/off, ms value, depth mode, depth value, fill rule).

## From G4a.md

### G4a-1. Which score categories does a path have, and in what order?
Looks like: K3-with-a-rule-inside (parallel on purpose, mostly known). One of the parallel parts holds a rule: the engine's running total.
Homes (all spell the same six categories base, combo, sp, solo, accent, ghost):
- `Path::score_base .. score_ghosts`, core/model.h 745-750. Inputs: the stored record.
- `ReplayScore`, core/replay.h 105-126. Inputs: the six values; owns `total()`, `add()`, `operator==`.
- `kReplayScoreFields`, core/replay.h 135-139. Inputs: the six JSON names, used by tools/replay.cpp 650 and tools/replay_json.cpp 93.
- `score_of` and `assign_score`, core/replay.cpp 214-232. Inputs: the Path/ReplayScore pairing, both directions.
- `ScoreGraphEdge::basescore .. ghostscore`, search/graph.h 144-149, with six `store_*score` methods (graph.h 232-237, graph.cpp 235-256 and the callers at graph.cpp 135-144).
- `EdgeView::basescore .. ghostscore` (engine.cpp 64-66), `OutPath::score_base .. score_ghosts` (engine.cpp 313-315), and the positional array `sc[6]` (engine.cpp 288), read as `sc[0]`..`sc[5]` at engine.cpp 820-825 and 1722-1727.
- `Path::prepare_variants` copies the six fields to each variant (model.cpp 664-669); engine.cpp 1909-1914 copies `OutPath` into `Path`.

### G4a-2. How is a chord's chart base score related to the stored "base" category?
Looks like: K2 (same word "base", two quantities). Probably harmless; one question for the verifier.
Homes: `category_scores`, core/scoring.cpp 49-51 (inputs: a note's 1x value `ChordNote::basescore`, its ghost/accent flags); `Path::chart_base_score`, core/model.cpp 848-850 (inputs: the three stored categories `score_base`, `score_ghosts`, `score_accents`).
Homes: `category_scores`, core/scoring.cpp 49-51 (inputs: a note's 1x value `ChordNote::basescore`, its ghost/accent flags); `Path::chart_base_score`, core/model.cpp 848-850 (inputs: the three stored categories `score_base`, `score_ghosts`, `score_accents`).

### G4a-3. What does an accent or ghost earn?
Looks like: K2 (two numbers, documented), possibly K0.
Homes: `CategoryScores::accent` / `::ghost`, scoring.cpp 49-50 (always the pad's 50, before the multiplier); `CategoryScores::dynamics_bonus`, scoring.cpp 80-84 (paid value less a plain note's paid value on the same pad, multiplier included).
Homes: `CategoryScores::accent` / `::ghost`, scoring.cpp 49-50 (always the pad's 50, before the multiplier); `CategoryScores::dynamics_bonus`, scoring.cpp 80-84 (paid value less a plain note's paid value on the same pad, multiplier included).

### G4a-4. Is this chord after the SP end?
Looks like: K1 (two tests of one question, with a documented equivalence).
Homes: `core::paid_by_sp_walk`, core/backend_value.h 30 (input: offset in ms from the SP end; answer: `offset_ms <= 0`); `core::after_sp_end`, backend_value.h 36 (inputs: chord tick, end tick; answer: `chord_tick > end_tick`).
Homes: `core::paid_by_sp_walk`, core/backend_value.h 30 (input: offset in ms from the SP end; answer: `offset_ms <= 0`); `core::after_sp_end`, backend_value.h 36 (inputs: chord tick, end tick; answer: `chord_tick > end_tick`).

## From G5.md

### G5-1. Who words one cell of the backend table's Rating column, and what does "(uncounted)" mean?
Looks like: K3-with-a-rule-inside (three authors of one cell; the rule inside is "is this row counted", asked from three places but answered by one function).
Homes: `BackendSqueeze::summarystr`, core/model.cpp 339-361 (inputs: squeezed_out flag, offset, hit window W, leeway; writes the ladder word and the " (uncounted)" tag);
       `build_activations` backend loop, app/path_view.cpp 359-391 (appends " (eff. N ms)", then " <-- squeezed out (-N)" or " <-- squeezed out"; sets the warning colour; computes its own `counted` with `core::counted_without_squeeze`);
       `kBackendTimingsLead`, app/path_view.cpp 170-174 (prose that names the tag "(uncounted)" and describes what it means).
Homes: `BackendSqueeze::summarystr`, core/model.cpp 339-361 (inputs: squeezed_out flag, offset, hit window W, leeway; writes the ladder word and the " (uncounted)" tag);
       `build_activations` backend loop, app/path_view.cpp 359-391 (appends " (eff. N ms)", then " <-- squeezed out (-N)" or " <-- squeezed out"; sets the warning colour; computes its own `counted` with `core::counted_without_squeeze`);
       `kBackendTimingsLead`, app/path_view.cpp 170-174 (prose that names the tag "(uncounted)" and describes what it means).

### G5-2. How is one squeeze or fill timing written for the user?
Looks like: K3-with-a-rule-inside (the rule inside is the rounding: whole ms against one decimal).
Homes: `activation_badge`, app/path_view.cpp 91-99 (the activation's `badge_timing()`; words from `kBadgeWords`; `format_ms_whole`);
       `Activation::notationstr_verbose`, core/model.cpp 561-574 (every squeeze's `difficulty()` and the E-critical fill's `e_difficulty(true)`; `format_ms_whole`; joined with "/"; feeds Ctrl+C and the Preview's next-activation box);
       `squeeze_sentences`, app/path_view.cpp 116-168 (each squeeze's `timing()`; `format_ms`, one decimal);
       the path button's timing, app/path_view.cpp 510-513 (`Path::difficulty()`; `format_ms`).
Homes: `activation_badge`, app/path_view.cpp 91-99 (the activation's `badge_timing()`; words from `kBadgeWords`; `format_ms_whole`);
       `Activation::notationstr_verbose`, core/model.cpp 561-574 (every squeeze's `difficulty()` and the E-critical fill's `e_difficulty(true)`; `format_ms_whole`; joined with "/"; feeds Ctrl+C and the Preview's next-activation box);
       `squeeze_sentences`, app/path_view.cpp 116-168 (each squeeze's `timing()`; `format_ms`, one decimal);
       the path button's timing, app/path_view.cpp 510-513 (`Path::difficulty()`; `format_ms`).

### G5-3. What does "free" mean for a squeeze?
Looks like: K2 (same word, two rules; low weight because only one side is a word the user sees).
Homes: `SPSqueeze::is_free`, core/model.h 359-362 (offset; a SqIn is free at offset <= 0, a SqOut is free at offset > 0, via `core::paid_by_sp_walk`);
       `BackendSqueeze::summarystr`, core/model.cpp 348-355 (offset, W; the label "Free" or "Free SqOut" is a slack of a whole hit window).
Homes: `SPSqueeze::is_free`, core/model.h 359-362 (offset; a SqIn is free at offset <= 0, a SqOut is free at offset > 0, via `core::paid_by_sp_walk`);
       `BackendSqueeze::summarystr`, core/model.cpp 348-355 (offset, W; the label "Free" or "Free SqOut" is a slack of a whole hit window).

### G5-4. What is each path group called on screen?
Looks like: K3 (each screen words the group in its own place; no rule inside).
Homes: `render_path_list` headings, ui/paths_tab.cpp 174-176 ("Optimal", the `within_label`, "Best all-0 path");
       `within_label`, app/path_view.cpp 476-480 ("Within 2 scores" / "Within 5,000 points");
       `preview_path_label`, app/preview_view.cpp 710-717 (suffixes "  (optimal)" and "  (best all-0)").
Homes: `render_path_list` headings, ui/paths_tab.cpp 174-176 ("Optimal", the `within_label`, "Best all-0 path");
       `within_label`, app/path_view.cpp 476-480 ("Within 2 scores" / "Within 5,000 points");
       `preview_path_label`, app/preview_view.cpp 710-717 (suffixes "  (optimal)" and "  (best all-0)").

### G5-5. Which kind of squeeze is this, and what is it called?
Looks like: K3-with-a-rule-inside.
Homes: `enum class SqueezeKind { SqIn, SqOut }` with `type_name()` and `symbol()`, core/model.h 263 and 347-365;
       `SQ_IN = 0` / `SQ_OUT = 1`, search/engine.cpp 34-35 (a plain-int copy for the flat decision log, mapped back at engine.cpp 1141-1142 and 1962-1967);
       `summarize_path` in store/record_store.cpp 513-519 (counts SqIns and SqOuts by comparing `sq.kind`);
       badge words in app/path_view.cpp 77-81, and the report's column titles "SqIn" / "SqOut" in app/path_report_view.cpp 153 and 157.
Homes: `enum class SqueezeKind { SqIn, SqOut }` with `type_name()` and `symbol()`, core/model.h 263 and 347-365;
       `SQ_IN = 0` / `SQ_OUT = 1`, search/engine.cpp 34-35 (a plain-int copy for the flat decision log, mapped back at engine.cpp 1141-1142 and 1962-1967);
       `summarize_path` in store/record_store.cpp 513-519 (counts SqIns and SqOuts by comparing `sq.kind`);
       badge words in app/path_view.cpp 77-81, and the report's column titles "SqIn" / "SqOut" in app/path_report_view.cpp 153 and 157.

### G5-6. How is the activation's deactivation tick read?
Looks like: K1 (line-level).
Homes: `Activation::deact_tick`, core/model.cpp 709-712; `activation_deact_tick`, core/squeeze_rating.cpp 22-24 (returns `act.deact_tick()` and nothing else).
Homes: `Activation::deact_tick`, core/model.cpp 709-712; `activation_deact_tick`, core/squeeze_rating.cpp 22-24 (returns `act.deact_tick()` and nothing else).

### G5-7. Does squeezing out this row cost points?
Looks like: K1 (inside one file; would become K2 if the two tests part).
Homes: `build_activations`, app/path_view.cpp 365-390 (`counted_without_squeeze` decides "(-N)" and the warning colour; the comment at 376 says "only when the squeeze-out really costs points");
       `squeeze_sentences`, app/path_view.cpp 140-150 (`counted_without_squeeze`, then `sqout_cost` > 0 picks "scores N fewer points" or "costs no points").
Homes: `build_activations`, app/path_view.cpp 365-390 (`counted_without_squeeze` decides "(-N)" and the warning colour; the comment at 376 says "only when the squeeze-out really costs points");
       `squeeze_sentences`, app/path_view.cpp 140-150 (`counted_without_squeeze`, then `sqout_cost` > 0 picks "scores N fewer points" or "costs no points").

### G5-8. The middle-dot separator " · " between facts on a line
Looks like: K1 (line-level).
Homes: `kDot`, app/path_view.cpp 28; literal "\xC2\xB7" in ui/paths_tab.cpp 466, ui/details_panel.cpp 74 and 124, app/preview_view.cpp 699.
Homes: `kDot`, app/path_view.cpp 28; literal "\xC2\xB7" in ui/paths_tab.cpp 466, ui/details_panel.cpp 74 and 124, app/preview_view.cpp 699.

## From G6a.md

### G6a-1. Where can the playhead be, and what does the screen show when it is outside?
Looks like: K1 with one edge that differs (a K2 on the empty chart and on a scrub range longer than the playback range).
Homes:
- `PreviewTransport::seek_ms` and `PreviewTransport::tick`, `ui/preview_transport.cpp` 77-107 (inputs: a requested ms, `length_ms_` = max of last drawn note and audio end).
- `shown_length` and `shown_ms`, `app/preview_view.cpp` 484-494 (inputs: the clock's `now_ms`, and `length_ms`, which the controller passes as `playback_end_ms()` for the time, score, drain and next-activation boxes and for `step_tick_ms`).
- `scrub_thumb_ms`, `app/preview_view.cpp` 660-664 (inputs: `now_ms`, the scrub edge).
- `song_fraction`, `app/preview_view.cpp` 640-643 (clamp 0..1 for the gold marks).
- The gauge fill clamp, `ui/preview_tab.cpp` 736-737 (bars over cap, clamp 0..1). This is a different quantity, so I list it only as a fourth spelling of "clamp to 0..1".
Homes:
- `PreviewTransport::seek_ms` and `PreviewTransport::tick`, `ui/preview_transport.cpp` 77-107 (inputs: a requested ms, `length_ms_` = max of last drawn note and audio end).
- `shown_length` and `shown_ms`, `app/preview_view.cpp` 484-494 (inputs: the clock's `now_ms`, and `length_ms`, which the controller passes as `playback_end_ms()` for the time, score, drain and next-activation boxes and for `step_tick_ms`).
- `scrub_thumb_ms`, `app/preview_view.cpp` 660-664 (inputs: `now_ms`, the scrub edge).
- `song_fraction`, `app/preview_view.cpp` 640-643 (clamp 0..1 for the gold marks).
- The gauge fill clamp, `ui/preview_tab.cpp` 736-737 (bars over cap, clamp 0..1). This is a different quantity, so I list it only as a fourth spelling of "clamp to 0..1".

### G6a-2. Is Star Power running at this moment?
Looks like: K1 with an edge that differs at the exact instant (probably intended; G6b should confirm).
Homes:
- `running_activation`, `app/preview_view.cpp` 241-247 (inputs: the playhead ms, each activation's `sp_window()`; used by `build_score_box` and `build_drain_box`).
- `SpanSweep::at` over the `sp_active_` intervals, `render/track_state.cpp` 78-96, with the intervals filled from the same `sp_window()` at 262-266 (inputs: the time of each highway row, the same window). Not in my group; I read only these lines.
Homes:
- `running_activation`, `app/preview_view.cpp` 241-247 (inputs: the playhead ms, each activation's `sp_window()`; used by `build_score_box` and `build_drain_box`).
- `SpanSweep::at` over the `sp_active_` intervals, `render/track_state.cpp` 78-96, with the intervals filled from the same `sp_window()` at 262-266 (inputs: the time of each highway row, the same window). Not in my group; I read only these lines.

### G6a-3. Which settings make a Preview "another song", and which make only a new overlay?
Looks like: K1 (a list of fields written in several places) with a K2 risk on `rules`.
Homes:
- `PreviewSongKey`, `ui/preview_controller.h` 56-68 (md5, difficulty, pro, bass2x, noteshuffle) and its use in `PreviewController::open`.
- `Settings::chartmode_key` (`app/config.cpp` 266-279) and `Settings::to_analysis_settings` (315-328): the same four mode fields, spelled for the store key and for analysis. `Settings::with_chartmode` (297-313) is the inverse and loops over the same four.
- The loose parameter list `(pro, bass2x, difficulty, noteshuffle)` carried by `PreviewLoadJob`, `PreviewNotesJob`, `resolve_preview_source`, `resolve_preview_song`, `chart_song_length_ms`, `dynamics_for_settings`.
- The overlay comparison in `open()` (`preview_controller.cpp` 85-95): path key and `sp_cap`.
Homes:
- `PreviewSongKey`, `ui/preview_controller.h` 56-68 (md5, difficulty, pro, bass2x, noteshuffle) and its use in `PreviewController::open`.
- `Settings::chartmode_key` (`app/config.cpp` 266-279) and `Settings::to_analysis_settings` (315-328): the same four mode fields, spelled for the store key and for analysis. `Settings::with_chartmode` (297-313) is the inverse and loops over the same four.
- The loose parameter list `(pro, bass2x, difficulty, noteshuffle)` carried by `PreviewLoadJob`, `PreviewNotesJob`, `resolve_preview_source`, `resolve_preview_song`, `chart_song_length_ms`, `dynamics_for_settings`.
- The overlay comparison in `open()` (`preview_controller.cpp` 85-95): path key and `sp_cap`.

### G6a-4. When does the last note start?
Looks like: K1, agrees today. Known, line-level (derivation audit line 232 names it); listed because a second owner now exists.
Homes:
- `last_drawn_note` / `last_note_ms`, `app/preview_view.cpp` 251-258 (inputs: `scene.notes`, built from `song.sequence` in tick order; takes `notes.back()`).
- `last_note_start_ms`, `app/song_length.cpp` 20-26 (inputs: `song.sequence` from the end; first timestamp whose `chord.count() > 0`).
Homes:
- `last_drawn_note` / `last_note_ms`, `app/preview_view.cpp` 251-258 (inputs: `scene.notes`, built from `song.sequence` in tick order; takes `notes.back()`).
- `last_note_start_ms`, `app/song_length.cpp` 20-26 (inputs: `song.sequence` from the end; first timestamp whose `chord.count() > 0`).

### G6a-5. What is "the end of the song", and which end does each label show?
Looks like: K2 (same word, different rules), in front of a user.
Homes:
- `PreviewTransport::length_ms` = max(last drawn note, audio end in chart time), `ui/preview_transport.cpp` 26-32. The time box's `length` and the "m:ss / m:ss" clock text come from it (`build_time_box`, `preview_view.cpp` 498-512, with `length_ms` = `playback_end_ms()`).
- `app::song_length_ms`, the metadata length or the last Expert-2x note (`app/song_length.cpp` 28-40), shown by the scrubber's edge and gold marks (`scrub_end_ms`) and by the Paths tab's timeline end (`path_view.cpp` 408-412, `timeline_end`).
- Two stored copies of the same song length: the controller's `song_length_ms_` and `PreviewScene::song_length_ms`, and a third on `ViewedSong::song_length_ms` for the Paths tab. All three come from the same owner function, called separately by `read_preview_notes` and by the click's analysis.
Homes:
- `PreviewTransport::length_ms` = max(last drawn note, audio end in chart time), `ui/preview_transport.cpp` 26-32. The time box's `length` and the "m:ss / m:ss" clock text come from it (`build_time_box`, `preview_view.cpp` 498-512, with `length_ms` = `playback_end_ms()`).
- `app::song_length_ms`, the metadata length or the last Expert-2x note (`app/song_length.cpp` 28-40), shown by the scrubber's edge and gold marks (`scrub_end_ms`) and by the Paths tab's timeline end (`path_view.cpp` 408-412, `timeline_end`).
- Two stored copies of the same song length: the controller's `song_length_ms_` and `PreviewScene::song_length_ms`, and a third on `ViewedSong::song_length_ms` for the Paths tab. All three come from the same owner function, called separately by `read_preview_notes` and by the click's analysis.

### G6a-6. Is this length one the timeline can use?
Looks like: K1, agrees today. Known, line-level.
Homes: `has_song_length` (`preview_view.cpp` 638, "positive") with `stated_length_ms`, `song_fraction`, `scrub_end_ms`, `scrub_thumb_ms` and path_view all calling it; against `length_ms_ > 0.0` in `PreviewTransport::seek_ms` (line 79) and `tick` (line 102), and `len_s > 0.0f` in `ui/preview_tab.cpp` 497.
Homes: `has_song_length` (`preview_view.cpp` 638, "positive") with `stated_length_ms`, `song_fraction`, `scrub_end_ms`, `scrub_thumb_ms` and path_view all calling it; against `length_ms_ > 0.0` in `PreviewTransport::seek_ms` (line 79) and `tick` (line 102), and `len_s > 0.0f` in `ui/preview_tab.cpp` 497.

### G6a-7. Does the drawn path have activations?
Looks like: K1, agrees today.
Homes: `no_acts = scrub_marks.empty()` in `ui/preview_tab.cpp` 428-429 (decides whether "< Act" and "Act >" are greyed); `scene_.activations` in `jump_activation` and `seek_activation` (`preview_controller.cpp` 489-502) and `activation_jump_ms` (`preview_view.cpp` 671).
Homes: `no_acts = scrub_marks.empty()` in `ui/preview_tab.cpp` 428-429 (decides whether "< Act" and "Act >" are greyed); `scene_.activations` in `jump_activation` and `seek_activation` (`preview_controller.cpp` 489-502) and `activation_jump_ms` (`preview_view.cpp` 671).

### G6a-8. Where does each text box sit on the Preview image?
Looks like: K1 inside one file (`ui/preview_tab.cpp`), with a restating comment in `render/overlay_layout.h`.
Homes:
- The fit pass, `preview_tab.cpp` 591-648: left column height = time box + gap + score box (if shown); drain box at `v_margin`; next box under the drain box via the `next_top_at` lambda.
- The draw pass, 660-812: score box at `box_max.y + sz.gap`; drain box at `origin.y + v_margin`; next box at `next_top_at(sz)`.
- `render::OverlayBoxes`, `render/overlay_layout.h` 52-63, whose comments describe the same stacking ("score box under it", "next-activation box, under the drain box").
Homes:
- The fit pass, `preview_tab.cpp` 591-648: left column height = time box + gap + score box (if shown); drain box at `v_margin`; next box under the drain box via the `next_top_at` lambda.
- The draw pass, 660-812: score box at `box_max.y + sz.gap`; drain box at `origin.y + v_margin`; next box at `next_top_at(sz)`.
- `render::OverlayBoxes`, `render/overlay_layout.h` 52-63, whose comments describe the same stacking ("score box under it", "next-activation box, under the drain box").

## From G7a.md

### G7a-1. In what order does the game list a drum pad's lane bits (red, tom pads, cymbal pads)?
Looks like: K1 (two spellings of one game fact; agree today)
Homes: lane_bit_rank, src/parse/song.cpp 112-116 (a note's colour and cymbal flag; used by lane_pads 118-135 to find the lowest and highest pad of a chord);
       lane_bit, src/parse/note_shuffle.cpp 24-33 (a note's colour and cymbal flag; used to build the shuffle's lane masks). Two more spellings of the same fact sit in note_shuffle.cpp: moved_to 37-51 (bit back to colour and flag) and pads() 55-58 (cymbal bit = tom bit shifted left by 3).
Homes: lane_bit_rank, src/parse/song.cpp 112-116 (a note's colour and cymbal flag; used by lane_pads 118-135 to find the lowest and highest pad of a chord);
       lane_bit, src/parse/note_shuffle.cpp 24-33 (a note's colour and cymbal flag; used to build the shuffle's lane masks). Two more spellings of the same fact sit in note_shuffle.cpp: moved_to 37-51 (bit back to colour and flag) and pads() 55-58 (cymbal bit = tom bit shifted left by 3).

### G7a-2. When is a drum note a cymbal? (Pro Drums gate on cymbals)
Looks like: K1 (line-level; agree today)
Homes: MidiParser::op_note, src/parse/song.cpp 903-911 (`allows_cymbals(color) && mode_pro_` then the tom-marker state);
       ChartParser::note_optype, src/parse/song.cpp 1714-1722 (N 66, 67, 68 only `if (mode_pro_)`, mapped by hand to Yellow, Blue, Green).
       Further copies outside the group, for the verifier: render/track_state.cpp 42 (`pro && n.cymbal && allows_cymbals(...)`), core/model.cpp 60-66 (note_label).
Homes: MidiParser::op_note, src/parse/song.cpp 903-911 (`allows_cymbals(color) && mode_pro_` then the tom-marker state);
       ChartParser::note_optype, src/parse/song.cpp 1714-1722 (N 66, 67, 68 only `if (mode_pro_)`, mapped by hand to Yellow, Blue, Green).
       Further copies outside the group, for the verifier: render/track_state.cpp 42 (`pro && n.cymbal && allows_cymbals(...)`), core/model.cpp 60-66 (note_label).

### G7a-3. What happens to one tick's events when one of them is malformed, and where does the 2x bass step sit?
Looks like: K1 (same rule in both parsers; agree today)
Homes: MidiParser::run_ops and push_timestamp, src/parse/song.cpp 1095-1136 (catch ChartFileError per op and carry on; `apply_2x_bass(tick)` after the notes, before the chord is emitted);
       ChartParser::push_timestamp, src/parse/song.cpp 1746-1784 (the same catch inside run_phase; `chord_.apply_2x_bass(mode_bass2x_)` after the Notes phase, before the Pre/emit phases).
Homes: MidiParser::run_ops and push_timestamp, src/parse/song.cpp 1095-1136 (catch ChartFileError per op and carry on; `apply_2x_bass(tick)` after the notes, before the chord is emitted);
       ChartParser::push_timestamp, src/parse/song.cpp 1746-1784 (the same catch inside run_phase; `chord_.apply_2x_bass(mode_bass2x_)` after the Notes phase, before the Pre/emit phases).

### G7a-4. What does the library show for a song with no title, artist or charter?
Looks like: K2 (the same idea gives different words for title, artist and charter; the stored and shown values differ)
Homes: title_or_unknown, src/parse/song.cpp 459-463, with name_or_unknown 452-455 (stored title text);
       artist_or_unknown, src/parse/song.cpp 550-552 (stored artist text);
       charter_or_unknown, src/parse/song.cpp 554-556 (stored charter text);
       display_title 535-537, display_artist 539-542, display_charter 544 (what screens show).
Homes: title_or_unknown, src/parse/song.cpp 459-463, with name_or_unknown 452-455 (stored title text);
       artist_or_unknown, src/parse/song.cpp 550-552 (stored artist text);
       charter_or_unknown, src/parse/song.cpp 554-556 (stored charter text);
       display_title 535-537, display_artist 539-542, display_charter 544 (what screens show).

### G7a-5. Which meter is in force just before a chord? (check_activations' own walk)
Looks like: K1 with a real difference on some inputs. KNOWN, line-level: finding 53 in docs/audit/2026-10-03-derivation-audit.md (lines 1086-1108).
Homes: Song::check_activations, src/parse/song.cpp 593-617 and 651-655 (its own walk over tpm_changes, one change per chord, writes `pre_tpm`);
       MeasureIndex::section_at / tpm_at, src/core/timing.cpp 109-114 (the owner).
Homes: Song::check_activations, src/parse/song.cpp 593-617 and 651-655 (its own walk over tpm_changes, one change per chord, writes `pre_tpm`);
       MeasureIndex::section_at / tpm_at, src/core/timing.cpp 109-114 (the owner).
