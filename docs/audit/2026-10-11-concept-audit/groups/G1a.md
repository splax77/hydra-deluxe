# G1a: the three reports' generators (path report, dmleaderboards page, fill comparison)

Scout: G1a, phase 2. Read in full: all 16 files in the dispatch (src/app/report, dm_report, fill_report, path_report_view, dm_report_view, report_view, html_page, display_format, .h and .cpp). Read outside the group, only the lines needed: record_store.cpp (list_records, get_summaries, rank_row, WinnerPicker), library_model.cpp, config.cpp, app_state.cpp, library_jobs.cpp, dm_report_window.cpp, library_query.cpp, squeeze_rating.cpp, CONTEXT.md lines 34-44, and the derivation audit by grep.

## The short answer to the group question

The three reports do not each build their own generator pieces. They share one join (`records_by_hash`), one "is this chart in the library" answer (`library_lists`), one empty-library gate (`lacks_chart_library` and `kNoChartLibrary`), one sentence builder for "nothing analyzed under these settings" (used by two of the three), and one set of name-cleaning functions (`display_title`, `display_artist`, `display_charter`). The path report and the DM page share one table (`TableView`). That part is derived once.

What is not shared is the vocabulary and the leftovers. The status words are re-spelled on every screen. The fill page is the last page that is real HTML, and its script re-implements table behaviour that the two ImGui windows get from `TableView` in C++. The two view files each carry their own small helpers. And the list of settings the path report reads is written twice and the two copies disagree on one setting.

## Candidates

### G1a-1. Does Hydra have a result for this chart?
Looks like: K2 (same word family, different rules; the words are on screen).

I confirmed the plan's first worked example against the code. `RecordStore::list_records` (src/store/record_store.cpp 1440-1503) offers every candidate row to `WinnerPicker`, then keeps a row only when `keep[i] && picker.rank(i).ready()` (line 1498). A Stale winner is dropped, and nothing takes its place. `RecordStore::get_summaries` (1307-1381) uses the same picker but reports the Stale winner as `RecordStatus::Stale` (lines 1366-1374), with the comment "A stale winner is reported as Stale, not hidden". CONTEXT.md lines 42-44 state the split on purpose: "A listing returns only ready rows, so a stale row reads the same as no row at all." So the store side is decided. The words the user sees on the other side are what differ.

Homes:
- Library: `status_label`, src/ui/library_model.cpp 67-74, fed by `get_summaries` (inputs: chart, chart mode, the settings' SP cap, the lens). Also `best_path_label` 98-106.
- DM page: `collect_dm_rows`, src/app/dm_report.cpp 51-132, lines 93-127. Inputs: the score's identifier, `records_by_hash` at Clone Hero's cap (4) and the lens, `library_copies_by_hash`. `records_by_hash` is src/app/report.cpp 155-169 and reads `list_records`.
- Path report: `collect_rows`, src/app/report.cpp 201-315, reads `list_records` directly (lines 232-252).
- Fill page: `collect_fill_rows`, src/app/fill_report.cpp 167-244, reads `records_by_hash` once per database.

Table, home 1 (library, one chart in the scan, same mode, lens and cap as shown). Words from `status_label` and `best_path_label`:

| Stored state of the chart | Status chip | Best path cell |
|---|---|---|
| no row at all | Not analyzed | Not analyzed |
| row whose build stamp or rules are not current (the winner is not `ready()`) | Stale | Stale |
| Ready, scored best path | Analyzed | `<score>  <path>` |
| Ready, no paths kept | Analyzed | empty path text |
| result exists but chart not in the scan | row does not exist | none |

Table, home 2 (DM page, same chart, joined by `identifier`, always at cap 4):

| Stored state | DM status word (chip text is the raw word) | Tile and dropdown wording |
|---|---|---|
| no row, chart in library | not analyzed | Not analyzed / "Not analyzed (in your library)" |
| Stale row, chart in library | not analyzed (same as above, `list_records` hides the row) | same |
| Ready, scored path, base speed | under optimal / at optimal / above optimal | Under optimal etc. |
| Ready, scored path, other speed | other speed | Other speed |
| Ready, no paths kept | no paths | "No paths (analyzed, none kept)"; no tile |
| result exists, chart not in library | not in library | Not in library / "Not in your library" |
| no result, chart not in library | not in library | same |

Table, home 3 (path report): no row, Stale and Ready-with-no-paths all give the same thing, no rows for the chart on the page (Ready with no paths: `chart_rows` returns nothing because `record.all_paths()` is empty, lines 84-88). Only Ready with a scored path gives rows. A chart not in the library is left out (line 237).

Table, home 4 (fill page, per database; "side" = the 1.0 database or the 1.1 database; each side is read through `records_by_hash`, so Stale hides a side):

| 1.0 side | 1.1 side | Status word (C++ literal, 233-239) |
|---|---|---|
| Ready, scored | Ready, scored, higher | 1.1 higher |
| Ready, scored | Ready, scored, lower | 1.0 higher |
| Ready, scored | Ready, scored, equal | same |
| Ready, scored | no row, or Stale, or other cap | only 1.0 |
| no row, or Stale | Ready, any | only 1.1 |
| Ready, scored | Ready, no paths | in both |
| Ready, no paths | Ready, scored | in both |
| Ready, no paths | Ready, no paths | in both |
| Ready, no paths | no row or Stale | only 1.0 |
| no row or Stale | no row or Stale | no row on the page |

Differ on: a Stale row. The library says "Stale". The DM page says "not analyzed" and its footer and dropdown tell the user "analyze them, then compare again". The fill page says "Only in 1.1 db" for a chart whose 1.0 database holds a Stale row, which is literally untrue for that database. The path report says nothing and leaves the chart out. A second difference on the same input: the library asks at the cap in Settings, the DM page always asks at cap 4, so a user on cap 5 sees "Analyzed" in the library and "not analyzed" on the DM page with no stale row involved (the DM footer covers this case; it does not cover the Stale case). A third: library "Not analyzed (N)" counts only never-analyzed charts, while the DM "Not analyzed" tile counts never-analyzed plus Stale.
Note for the verifier: the "Ready, no paths" rows follow `has_scored_best_path` (src/store, used at dm_report.cpp 109 and fill_report.cpp 211). The derivation audit item 88 (lines 1880-1895) covers the no-paths case and the DM "no paths" status has since fixed it. The Stale case is not in that audit item. The library tooltip names the cause (stale_build, stale_rules), which the DM page cannot, because `list_records` returns no Stale row to ask. I did not run anything; every row above is by reading.

### G1a-2. What is each state called, and how many spellings does one state have?
Looks like: K2 (one state, several wordings, all on screen; for the fill page, one word with two meanings).

Homes (DM page, the four words that the lead names): the raw status string `kStatusNotInLibrary = "not in library"` and `kStatusNotAnalyzed = "not analyzed"` (src/app/dm_report.h 41-43); the tile labels (dm_report.cpp 184-193); the dropdown labels (dm_report_view.cpp 182-194); `counts_phrase` (dm_report.cpp 196-207); the status column definition (dm_report_view.cpp 164-171); the unclickable hint (src/ui/dm_report_window.cpp 99); the footer (dm_report.cpp 225-230). The table cell draws the raw status string as it is (dm_report_view.cpp 173), so the stored value is also the on-screen word.

Table, home 1 (DM page, one state per row, every place that writes it):

| State | Chip text in the table | Tile | Dropdown | Counts phrase (empty-player box) | Other |
|---|---|---|---|---|---|
| not in library | not in library | Not in library | Not in your library | not in your library | hint "Not in your library" |
| not analyzed | not analyzed | Not analyzed | Not analyzed (in your library) | not analyzed | footer "Not analyzed charts are in your library without a current result" |
| no paths | no paths | none | No paths (analyzed, none kept) | with no paths (only when above zero) | |
| other speed | other speed | Other speed | Other speed | at other speeds (via `counted`; "at another speed" for 1) | |
| under/at/above optimal | under optimal etc. | Under optimal etc. | Under optimal etc. | under optimal etc. | |

Table, home 2 (the fill page, same exercise; wording is in both C++ literals and the page script):

| State | Status literal (chip text, C++ and JS) | Dropdown option (fill_report.cpp 49-57) | Tile (JS stats 141-150) | Subtitle clause (C++ 339-346) |
|---|---|---|---|---|
| 1.1 higher | 1.1 higher | 1.1 higher | 1.1 higher | score higher under 1.1 |
| 1.0 higher | 1.0 higher | 1.0 higher | 1.0 higher | higher under 1.0 |
| same | same | Same score | Same | unchanged |
| only 1.0 / only 1.1 | only 1.0 / only 1.1 | Only in 1.0 db / Only in 1.1 db | Only one side (the two added) | in one database only |
| in both | in both | In both | Score on one side only | with a score on one side only |

Differ on: the DM page uses "library" in some places and "your library" in others for the same state. The fill page's `in both` is the leftover case (a record on each side, a score on one) but `same`, `1.1 higher` and `1.0 higher` charts are also in both databases; the word hides which case it is, and its tile says "Score on one side only" while its dropdown says "In both". The fill page names its statuses by raw literals in two C++ functions (collect 233-239, tally 253-258), the dropdown markup, and four script places (STATUS_CLASS 74-76, option values 50-56, `cells` 118-121, `stats` 137-146, `scoreText` 81); the DM page has named constants (`kStatus*`) and its header says "Every reader compares against these names, never the words". The fill header says "Exact literals: tally_fill_rows and the page's chip colors compare them" (fill_report.h 36-38). A test (tests/test_fill_report.cpp 313-339) pins the JS by searching the HTML text for `n('<literal>')`, not by running the script.
Note for the verifier: both sets agree on what each status means today. The risk is the next rename, which has to touch five or more places per page. "Known, line-level" for the fill page's literals in part: derivation audit line 3403 covers the delta sign and the colour pick; it does not cover the wording spread.

### G1a-3. How is a text column sorted?
Looks like: K2 (two sort orders for the same visible column; the windows' order is a recorded decision, the fill page is not covered by it).

Homes:
- `TableView::resort` with `folded_key` and `compare_keys`, src/app/report_view.h 178-205 and report_view.cpp 29-49. Inputs: the column's `SortKey` per row. Text goes through `fold_for_search`, then a plain byte compare. The comment (report_view.h 78-80) says the windows take "the Library table's text order, not the pages'" (D103 item 16).
- The page script's `render`, src/app/html_page.cpp 332-339. Inputs: the raw cell value. A string is compared with `localeCompare` (the browser's language order, no fold). A number is subtracted. Null sorts last in both directions. One sort key only.
- Third home, outside my group: `LibraryModel::resort`, src/ui/library_model.cpp 225-273, compares folded `searchable` strings the same way as the first home (so the first home agrees with the library; G1b or G3 to confirm).

Table, home 1 (TableView, columns of text):

| Input pair | Result |
|---|---|
| "apple" vs "Banana" | apple first (folded to lower case, then bytes) |
| "Étude" vs "Etude" | tie after folding, rows keep their order |
| two empty strings | tie; an empty string is a text key, not the sunk kind |
| empty string vs "a" | empty first ascending, last descending (text "" is a value) |
| a numeric column with a missing value (`SortKey{}`) | sinks to the bottom either direction |
| second key | a second column breaks ties (up to `kMaxSortKeys` = 2) |

Table, home 2 (fill page script):

| Input pair | Result |
|---|---|
| "apple" vs "Banana" | apple first under `localeCompare` in most browser locales |
| "Étude" vs "Etude" | ordered by the browser's collation (accent after plain); not a tie |
| two empty strings | tie |
| empty string vs "a" | empty first ascending |
| numeric column with null | bottom either direction |
| second key | not available (one key; shift-click does not exist) |

Differ on: accented and mixed-case names, and anything the browser's collation orders differently from folded bytes (punctuation, digits against letters). The remaining inputs I could not run; the table rows for `localeCompare` are by reading, and `localeCompare` depends on the viewer's browser and language.
Note for the verifier: D103 item 16 is a decision about the windows. I did not find a decision about what the fill page should do, so this may be a gap rather than a choice.

### G1a-4. How is a "shown of total" count line written, and what does it count?
Looks like: K2 at the edge of one (n = 1); K3 for the rest.

Homes:
- `report_view::count_line`, report_view.cpp 51-54, used by both windows through `TableView::count_line` with `kNoun` = {"path","paths"} and {"score","scores"}. Writes `group_thousands(shown) + " of " + counted(total, one, many)`. Counts rows.
- The page script, html_page.cpp 389-390: `fmt(countOf(rows)) + ' of ' + fmt(countOf(ROWS)) + ' ' + PAGE.noun`. The fill page sets `noun: 'charts'` and `count: charts` (fill_report.cpp 90-95), where `charts` adds each row's `k` (library copies). Counts library copies, on purpose (D79).

Table, home 1:

| shown / total | Output |
|---|---|
| 0 / 0 | 0 of 0 paths |
| 1 / 1 | 1 of 1 path |
| 3 / 1,000 | 3 of 1,000 paths |
| 0 / 2 | 0 of 2 paths |

Table, home 2 (fill page, with `noun: 'charts'`):

| shown / total (in copies) | Output |
|---|---|
| 0 / 0 | 0 of 0 charts |
| 1 / 1 | 1 of 1 charts |
| 3 / 1,000 | 3 of 1,000 charts |
| 0 / 2 | 0 of 2 charts |

Differ on: a total of exactly 1. C++ writes the singular ("1 of 1 path"; pinned by tests/test_report_view.cpp 224 and 228), the script always writes the plural ("1 of 1 charts"). `counted` (core/model.h 873-875) says "Every screen writes counts here", and the script is a screen that does not. Thousands grouping agrees: `fmt` is `toLocaleString('en-US')` and `group_thousands` always uses commas, including for negatives.
Note for the verifier: reachable on a one-chart database only; low impact. The window and the fill page also count different things (rows against library copies); that is deliberate (D76, D79), K0.

### G1a-5. Small view helpers copied between the two window views
Looks like: K1 (same rule, two homes; the tables agree on every input I could think of).

Homes:
- `text_column`, src/app/path_report_view.cpp 26-36 (inputs: id, title, definition, member pointer to a string field, look) and `text_column`, src/app/dm_report_view.cpp 37-44 (same, without the definition argument; it reaches `column()` with an empty definition). Both set `sort_key` to `SortKey{r.*field}` and `cell` to `r.*field`, and `numeric` false.
- `ms_key`, path_report_view.cpp 21-23 (`optional<double>` to number or empty key) and `optional_key<T>`, dm_report_view.cpp 46-49 (any `optional<T>` to number or empty key).
- The missing-value cell: `ms_cell` (path_report_view.cpp 17-19, dash for nullopt) and the inline lambdas in `dm_columns` for optimal (80), delta (92), rank (146) and posted (158), each writing `kDash` for a missing value. `count_column` (path 40-54) has no DM twin; DM writes its number columns inline.

Table, home 1 (path view):

| Input | Sort key | Cell |
|---|---|---|
| string field "" | text "" (sorts as a value, first ascending) | "" |
| `optional<double>` unset | empty (sinks) | dash |
| `optional<double>` 0.0 | number 0 | "0.0 ms" |

Table, home 2 (DM view):

| Input | Sort key | Cell |
|---|---|---|
| string field "" | text "" | "" |
| `optional<int64>` unset | empty (sinks) | dash |
| `optional<int64>` 0 | number 0 | "0" (or `group_thousands` of 0) |
| `posted` "" | empty (sinks), not text | dash (D103 item 17) |

Differ on: agree today for like-typed inputs. The one real difference is intentional and recorded: an empty `posted` sinks while an empty `song` string is a text value. If someone changes how an empty text cell sorts, there are two `text_column`s and the DM-only `posted` rule to keep in step. Both views also build their definitions through two paths (DM has `fill_constants` with `__SP_CAP__` and `__BASE_SPEED__`; the path view hard-codes its numbers; see Note).
Note for the verifier: the path Timing definition says "Beyond means more than twice the hit window" (path_report_view.cpp 122) and the footer says "'Beyond' means past the N ms window" (report.cpp 423). The rule is in `timing_tiers` (squeeze_rating.cpp 165-176: Insane+ cutoff is `nominal_budget_ms`, which is the window times `1 + kIdentityScale`, and `kIdentityScale` is 1.0, so "twice"). The prose is a second copy that agrees today. It would split only if `kIdentityScale` changed, which its name says it won't.

### G1a-6. Which settings does the path report read, and which settings make an open report out of date?
Looks like: K1 with a gap (two homes of one list; they disagree on one input). Group G3 also reads the `settings_change_touches` pair; this is from the report side.

Homes:
- The list as the report is built: `AppState::launch_path_report` and `request_path_report`, src/ui/app_state.cpp 698-718. It passes `settings.cap_query()`, `settings.lens()`, `settings.batch_run()` and `settings.hit_window_ms` to `ReportJob`, which hands them to `generate_report` (src/ui/library_jobs.cpp 587-608). In the report, `hit_window_ms` labels every row's tier (report.cpp 299, `place_rows`), names the "Past N ms" tile and the footer, and is kept as `GeneratedReport::hit_window_ms`, which the window then uses for the Timing chip and the dropdown (path_report_window.cpp 86, 98, 111).
- The list as it is checked: `report::settings_change_touches`, src/app/report.cpp 431-437: `before.cap_query() != after.cap_query() || before.lens() != after.lens()`. Called from `AppState::apply_settings`, app_state.cpp 866-870.
- The DM page, same question: `dm_report::settings_change_touches`, dm_report.cpp 237-241: `chartmode_key` and `lens`. `launch_dm_report` (app_state.cpp 681-686) builds the job from `chartmode_key()` and `lens()`, so these two agree. The cap is forced to Clone Hero's in `collect_dm_rows`, so leaving it out is correct.
- Third home, outside my group: `AppState::apply_settings`, app_state.cpp 858-859, which checks `chartmode_key`, `cap_query` and `lens` for the library.

Table, home 1 (what the path report is built from, per setting changed):

| Setting changed | Reaches the built report? |
|---|---|
| chart mode (difficulty, drums, bass, shuffle) | no: it lists every mode |
| SP cap | yes |
| lens (ms limit, depth, depth mode, fill rule) | yes |
| hit window (`hit_window_ms`) | yes: tier labels, "Past N ms" tile, footer, Timing chip |
| auto-open report | no |

Table, home 2 (`settings_change_touches` for the path report):

| Setting changed | Marks the report out of date? |
|---|---|
| chart mode | no |
| SP cap | yes |
| lens | yes |
| hit window | no |
| auto-open report | no |

Differ on: `hit_window_ms`. After a change, the open report keeps the old tier bands and the old edge, and nothing marks it out of date. The test that pins the function (tests/test_report_rows.cpp 839-868) covers mode, cap, ms limit, fill rule and auto-open, and not the hit window. `Settings::hit_window_ms` (config.h 93-97) says it is "display-layer only" and never invalidates stored records; that is about the store. The report copies it into the page's rows.
Note for the verifier: check by reading app_state.cpp 713 and report.cpp 299 (the setting is read at build time) and path_report_window.cpp 111 (the window shows the build-time value, not the live setting). I did not run the app.

### G1a-7. "Keep listed records whose chart the library lists": one rule, two call sites
Looks like: K1 (agree on every input).

Homes:
- `records_by_hash`, report.cpp 155-169: for one chart mode, drops a listing unless `library_lists(library, normalize_chart_hash(r.hyhash))`, keys by the normalized hash. Used by the DM page and the fill page.
- `collect_rows`, report.cpp 232-237: for every mode, the same drop through `library_lists(copies, hash)`.

Table, home 1 (`records_by_hash`) and home 2 (`collect_rows`; same rows in both):

| Listing | Kept? |
|---|---|
| hash in the library map | yes |
| hash not in the library map (batch run on a folder the scan does not list, D92) | no |
| hash with different letter case | normalized in both before the lookup, so as above |
| two listings with one hash | `records_by_hash` keeps the first (`emplace`); `collect_rows` keeps both. One chart mode per call in the first, so there are no duplicates there |

Differ on: agree today. They would split if one path changed how hashes are normalized, or what "the library lists" means; the answer is already in one function (`library_lists`) and both call it, so what is repeated is the 4-line loop around it.
Note for the verifier: this may be K0 rather than a finding; the shared owner is real and the two loops differ in what they need (a map by hash, or a slot per chart and mode).

### G1a-8. Words the fill page's script and the two windows both write
Looks like: K1 (the same words in two languages; agree today).

Homes: the fill page's HTML and script in fill_report.cpp / html_page.cpp, and the windows in src/ui.

| Words | Fill page | Windows |
|---|---|---|
| "Search song, artist, or charter" | fill_report.cpp 48 (the searched text is `search_field(song, artist, charter)`, line 287) | dm_report_window.cpp 80 (searched text `dm_search_text`, dm_report_view.cpp 202-204) |
| "...charter, or path notation" | none | path_report_window.cpp 116 (`path_search_text`, which adds the path) |
| "Nothing matches those filters." | html_page.cpp 387 | report_window.cpp 336 |
| the dash | `DASH = '—'` (html_page.cpp 295) and `report::kDash` for the acts cell (fill_report.cpp 299-302) | `report::kDash` (report.h 74) |
| thousands grouping | `fmt` (html_page.cpp 299) | `group_thousands` |

Differ on: agree today. Each placeholder is a second statement of which fields the search matches; the other statement is the `search_field(...)` call. Adding a field to one without the other leaves the box lying. G1b has the "Nothing matches" line.
Note for the verifier: the search match itself is derived once for both: `search_fold_table()` (library_query.cpp 294-306) builds the script's FOLD table from `fold_for_search`, and the script splits on whitespace and requires every word, as `search_words` and `search_matches` do. The derivation audit line 1434 (JS filter lowercases with `toLowerCase` and matches the whole string) is out of date for the code I read.

## Parallel on purpose

- **The three row types (`ReportRow`, `DmReportRow`, `FillCompareRow`) and `net::DmScore`.** Each side knows only per-report facts: the path row holds one path's numbers, tier and copies; the DM row holds one score's player fields (copied one by one from `DmScore`, dm_report.cpp 85-91), the optimal, delta, `pct_h` and status; the fill row holds both databases' scores, paths, acts and a status. What they share is the song, artist and charter trio, each written through the one set of `display_*` owners (report.cpp 52-54, dm_report.cpp 100-103, fill_report.cpp 206-208). The only name-choosing rule (board names against the record's when the board entry is "known" and has a song name; charter falls back to the record's when empty) lives once, in `collect_dm_rows`. The fill page's choice of the 1.1 side's names is also one line. The `copies` field is set through `RecordStore::copies_of` in both places that have it; the DM row has none on purpose (the page counts scores, dm_report.cpp 78).
- **`path_report_view` and `dm_report_view` over `TableView`.** Each file holds its column ids, titles, hover definitions, per-row cell text, its filter controls (Timing and Best path only; Status), its first sort (score; points left), its count noun and its search text. Those are per-report facts. The rules inside them are G1a-5 (helpers) and the sort and count rules in G1a-3 and G1a-4, which live once in `report_view`.
- **`generate_report`, `generate_dm_report`, `generate_fill_report`.** Each is collect, tally, then the page's own subtitle and footer sentence. The pieces that could drift are shared: the gate (`lacks_chart_library`, `kNoChartLibrary`), the empty sentence (`nothing_under_settings`, used by the path and fill reports; the DM page has no such case because it lists scores, not records), and name cleaning. The footers and subtitles are each report's own sentences. The fill footer takes its rule text from `fill_rule_description`, so it retells nothing.
- **The fill page's chip colours, the DM page's `status_token`, and the path report's `tier_token`.** Each maps its own status words to a colour. The map is a per-report fact (the same token `tn` means "no squeeze" on the path report, "same" on the fill page and "not in library" on the DM page). The fill page's CSS classes and the windows' `ChipToken` are the same colour choice in two languages; the page defines only `--t0`, `--t3`, `--tn` and `--muted`.
- **`tally_dm_rows` / `tally_fill_rows` / `path_tiles` against the fill page's script `stats()`.** The C++ tallies feed subtitles and the DM tiles. On the fill page the tiles are redone in script per filtered view, because the page filters in the browser. A test pins that they count the same things (tests/test_fill_report.cpp 313-339), but only by finding strings in the page text. Per-report fact: the statuses each page counts. The arithmetic (sum each row's copies by status) is a rule written twice for the fill page; I did not find a case where they differ.

## Dropped leads

- `page_template` (the lead's "K0 wrapper"): K0. `fill_report.cpp`'s local `page_template()` (157-163) calls `html::page_template` and only fills `__OLD_RULE__` and `__NEW_RULE__`. Same name, different jobs, so nothing is copied.
- `count_line`: `report_view::count_line` (text) and `report_frame::count_line` in src/ui/report_window.cpp 323 (draws a given string) share a name, not an idea: K0. The text rule is G1a-4.
- `text_column`: moved to G1a-5.
- "Above optimal", "Under optimal", "Not in library", "Not analyzed": moved to G1a-1 and G1a-2.
- "Not in your library" in dm_report_view.cpp and dm_report_window.cpp: moved to G1a-2.
- "Search song, artist, or charter" in fill_report.cpp and dm_report_window.cpp: moved to G1a-8.
- `settings_change_touches`: moved to G1a-6.
- The struct pairs `DmReportRow` against `DmScore` and `ReportRow`, and `ReportRow` against `DmScore`: cleared under "Parallel on purpose".
- `html_escape`, `json_escape_into`, `render_page` in html_page.cpp, and display_format.cpp (`py_round3`, `format_ms`, `format_percent`, `percent_steps`, `clock_str`): single owners, not duplicated within the group.
- The derivation audit's items about the report pages' JavaScript for the path and DM pages (lines 1434, 2095, 3403, 3439): those pages are now ImGui windows. What is left of them is the fill page, and is covered by G1a-3, G1a-4 and G1a-8.

## Not read

None. All 16 files in the dispatch were read in full.
