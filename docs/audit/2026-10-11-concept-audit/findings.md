# Phase 4 findings: the concept duplication audit

Nine Sonnet verifiers (V1 to V9) graded the 84 merged candidates in `candidates.md` on 2026-10-11, by reading the code; nobody built or ran anything in this phase. Every candidate was verified; none is parked or unverified. This file is their nine reports joined in order, unedited. Each block says where its evidence came from and names the run that would prove a claim made by reading.

By the first kind each block names: 34 are K1 (same rule, two homes), 25 are K2 (same word or question, different rules), 22 are K3 (parallel on purpose, some with a K1 inside) and 3 are K0 (not a copy). Counted by the main session from the "Kind:" lines; a block that names two kinds counts under its first.

Which findings need the user, and the questions for them, are in `questions.md`. Things the verifiers noticed that are not duplication questions are in each report's "Side notes" and are gathered in `side-notes.md`.


---

## V1: the reports and their tables (candidates M01 to M10)

Verifier: V1 (task concept-audit-V1; the progress hook named my agent id `abfa91300d60209bc` late, my first status lines went to `V1.md`). Everything below is by reading. I ran nothing, built nothing and changed no source. Line numbers are from the tree as it stands in `C:\Users\Patrick\Downloads\Hydra\hydra-test`.

Short version. M01 is a real K2 and the plan's worked example holds, with two more differences the scouts only half caught (the SP cap each page asks at, and a "Ready, no paths" chart). M03 and M05 are small K2s where the windows follow the Library or `counted` and the fill page's script does not. D103 item 16 and item 18 do not cover the fill page. M02 and M07 are mostly K1 with a K2 inside. M04, M09 and M10 are mostly K3 with one small K1 each. M06 is a plain K1. M08 is K1 and low value.

---

### M01. Does Hydra have a result for this chart, and which status word does each page show?

Kind: K2 (one stored fact, "what result does this chart have", read through three different predicates; the words are on screen). Fix: needs the user (display text, and a count: the DM page's "Not analyzed" tile would move if Stale got its own word).

Homes:
- `status_label` and `best_path_label`, `src/ui/library_model.cpp` 67-74 and 98-106, fed by `RecordStore::get_summaries`, `src/store/record_store.cpp` 1307-1381. Inputs: chart hash, chart mode, the Settings SP cap (`settings.cap_query()`), the lens. The count chips are `LibraryModel::refilter`, library_model.cpp 278-298, via `chip_of` (14-21); the chip text is `chip_label` (85-96).
- `collect_dm_rows`, `src/app/dm_report.cpp` 51-132 (status chosen at 93-128), via `report::records_by_hash`, `src/app/report.cpp` 155-169, via `RecordStore::list_records`, record_store.cpp 1440-1503. Inputs: the score's identifier, chart mode, the lens, and always `CapQuery::at(kCloneHeroSpCap)` (cap 4), plus `library_copies_by_hash` (report.cpp 194-199).
- `collect_rows`, report.cpp 201-315 (the path report), also via `list_records` at the Settings cap (report.cpp 232-237), then re-analyses each listed chart (or uses the batch's seed rows).
- `collect_fill_rows`, `src/app/fill_report.cpp` 167-244, via `records_by_hash` once per database, at the cap and fill rule `hydra_fillcompare` was given.

Tables. Rows are the four inputs you asked for, plus a fifth the code makes real (a chart analyzed at a different SP cap than the one in Settings, which is the case that splits the library and the DM page even with no Stale row). "In library" means the last scan listed it. The Settings cap is not 4 only in the last row.

| Input | Library: Best path cell and status word | Library: which chip counts it | DM page: status word in the table | DM page: tile / dropdown it falls in | Path report | Fill page |
|---|---|---|---|---|---|---|
| A. Never analyzed (no results row), chart in library | "Not analyzed" (status_label; hover says "Not analyzed yet. Click the song or run a batch to analyze it.") | "Not analyzed (N)" | "not analyzed" | tile "Not analyzed"; dropdown "Not analyzed (in your library)"; counts phrase "not analyzed" | no rows for the chart; nothing says it is missing | no row (needs a record on a side) |
| B. Stale (row from another build, or other rules), chart in library | "Stale" (own colour; hover names stale_build / stale_rules) | "Stale (N)" | "not analyzed" (list_records drops a Stale winner, so the page cannot tell it from A) | same tile and dropdown as A, so the DM "Not analyzed" count = A + B | no rows, nothing said | the side holding the Stale row counts as absent: "only 1.0" or "only 1.1" if the other side is Ready; no row if neither is |
| C. Ready result, no scored best path (analysis kept no path), chart in library | "Analyzed"; Best path cell is empty (the path text) | "Analyzed (N)" | "no paths" | no tile; dropdown "No paths (analyzed, none kept)"; counted in "Scores shown"; counts phrase adds "with no paths" only when above zero | no rows (`chart_rows` returns nothing for no paths, report.cpp 84-88); nothing said | "in both" if both sides hold a record; "only 1.0" / "only 1.1" if one side does; empty score cell reads "no paths" |
| D. Chart not in library (a leftover result, or no result) | no row at all | not counted | "not in library" (a result is ignored: `records_by_hash` drops it, report.cpp 163-165) | tile "Not in library"; dropdown "Not in your library"; row hover "Not in your library"; row click does nothing | left out (report.cpp 237) | left out |
| E. Ready at cap 4 only, Settings cap is 5 | "Not analyzed" (cap_match finds nothing at 5) | "Not analyzed (N)" | "under / at / above optimal" (it reads cap 4) | the optimal tiles | no rows (reads cap 5) | follows the cap given to the tool |

Edge notes for the table. An off-speed score has its status overwritten last (dm_report.cpp 128), so a score at another speed on a chart that is not in the library reads "other speed", not "not in library". The DM page's own wording defines "Not analyzed" as "no current result" (dm_report.h 39-41, and the Status column hover, dm_report_view.cpp 166-168), so the DM page is consistent with itself; the clash is with the Library's "Not analyzed", which means only "no results row at all". The chip counts are over rows the search keeps (refilter applies the search first), and they count charts; the DM tiles count scores; so the two numbers never claim to be the same unit.

The two predicates, quoted, for the question to the user:
- Library: `RecordStatus::NotAnalyzed` when `get_summaries` finds no candidate row for (hash, mode, cap, lens); `Stale` when the winning candidate is not `ready()` (`current && same_rules`, record_store.cpp 370); `Ready` otherwise.
- DM page: `row.status = kStatusNotAnalyzed` when `records_by_hash` has no entry for the hash and `library_lists(library, hash)` is true (dm_report.cpp 121-127). `records_by_hash` is `list_records`, which drops every chart whose winner is not Ready (record_store.cpp 1498).

G10's claim confirmed. `list_records` and `get_summaries` share one winner rule. Both build a `WinnerPicker` (record_store.cpp 440-461), offer each candidate row with `rank_row` (375-384), and let `outranks` (427-431) decide: this build's results version first, then this process's rules, then the newest `result_id`. Both use the same candidate filter (`append_candidate_filter`, 467-475: lens and cap). They differ only after the winner is picked: `get_summaries` turns a Stale winner into `RecordStatus::Stale` (1370-1374), `list_records` skips any winner that is not ready (1498) and does not fall back to the next candidate. Two smaller differences: `get_summaries` asks for the hashes you hand it with an exact `IN (...)` and groups on the one chart mode you pass; `list_records` lists every row of the mode (or of all modes when the mode is empty), LEFT JOINs the naming copy for names, and groups on each row's own mode. The split is recorded as the design in CONTEXT.md lines 42-44 and in the comment at record_store.cpp 1490-1494.

Disagree on: a Stale row (library "Stale", DM "not analyzed", path and fill silent or "only one side"); the SP cap each page asks at (library and path report use Settings, DM always 4); the Ready-no-paths chart (library says Analyzed with a blank cell, DM says "no paths", path report omits it, fill says "in both" or "only ...").
Owner proposed: the store's three-valued `RecordStatus` from `get_summaries` stays the one owner of "what result does this chart have". The DM page keeps `list_records` for its scores and names, and gets the status of a chart with no listed record from the same owner (a `get_summaries` call over the charts it needs), so it can tell A from B without a second predicate. What the DM page then calls B is the user's call: add a Stale word, rename "not analyzed" to something that does not clash ("no current result" is the DM page's own definition), or widen the library's word. The path report and fill page need no word of their own for A to D today.
Evidence: by reading, proven by a DM report over a copy of the user's db (or a one-chart test db) holding one Stale row and one never-analyzed chart, checking the two rows' status; and a `get_summaries` call at cap 5 against a cap-4 row for row E.
Found by: G1a-1, G1b-4, C1-1, C2-2; verified by V1.
Scan row or reason: a row on the literal `"Not analyzed"` (and on `kStatusNotAnalyzed`) outside `status_label` would flag a re-spelling, but the DM tile and dropdown are live spellings today, so the row can only be written after the user decides which words stay, with the kept spellings in its allowlist. A second status vocabulary with other words has no shared spelling; that is what the recurrence gate (check 5) is for.

---

### M02. How many spellings does one state have ("not in library", "in both" and the rest)?

Kind: K1 for the DM page (one state, one predicate, written out by hand in six places; every reader compares the `kStatus*` constants, but the words the user reads are typed separately), with a K2 inside the fill page's "in both" (one word, three meanings). Fix: constants and injection are code-only; unifying the words is needs the user (display text). D113 item 7 already puts the status words in the user's first batch of questions.

Homes:
- DM status words: `kStatus*`, `src/app/dm_report.h` 31-45 (the stored value is also the on-screen word: the Status cell draws `r.status` raw, dm_report_view.cpp 172-173); tile labels, dm_report.cpp 184-193; `counts_phrase`, 196-207; footer sentence, 225-230; Status column hover, dm_report_view.cpp 164-171; dropdown labels, 182-194; row hover, `src/ui/dm_report_window.cpp` 99. Counting and colour go through the constants (`count_status`, dm_report.cpp 20-29, and `status_token`, 147-155).
- Fill statuses: C++ literals in `collect_fill_rows`, fill_report.cpp 233-239, and `tally_fill_rows`, 253-258; the comment in fill_report.h 36-38 ("Exact literals"); the page's dropdown markup, fill_report.cpp 49-57; the page script, 74-76 (`STATUS_CLASS`), 81 (`scoreText`), 112-115 (`filter`), 118-121 (`cells`), 136-150 (`stats`). There are no named constants.

Tables. DM page, one state per row, every place that writes it:

| State | Table cell | Tile | Dropdown | Counts phrase | Other |
|---|---|---|---|---|---|
| not in library | "not in library" | "Not in library" | "Not in your library" | "not in your library" | hover "Not in your library" (window); Status hover "Not in your library: the last scan did not find it" |
| not analyzed | "not analyzed" | "Not analyzed" | "Not analyzed (in your library)" | "not analyzed" | footer "Not analyzed charts are in your library without a current result ..." |
| no paths | "no paths" | none (D62 item 1) | "No paths (analyzed, none kept)" | "with no paths" (only when above zero) | |
| other speed | "other speed" | "Other speed" | "Other speed" | "at another speed" / "at other speeds" | |
| under / at / above optimal | lower case | capitalised | capitalised | lower case | |

Fill page:

| State (the stored literal) | Chip text | Dropdown option | Tile | Subtitle clause |
|---|---|---|---|---|
| 1.1 higher | 1.1 higher | 1.1 higher | 1.1 higher | score higher under 1.1 |
| 1.0 higher | 1.0 higher | 1.0 higher | 1.0 higher | higher under 1.0 |
| same | same | Same score | Same | unchanged |
| only 1.0 / only 1.1 | only 1.0 / only 1.1 | Only in 1.0 db / Only in 1.1 db | Only one side (the two added) | in one database only |
| in both | in both | In both | "Score on one side only" | "with a score on one side only" |

Edges. The fill page's `in both` is reached whenever both sides hold a record and the pair is not (scored, scored): that includes one side scored, and also both sides Ready with no paths, where no side has a score. The tile and the subtitle both say "a score on one side only", which is false for that case. The dropdown says only "In both". A status string the DM page does not know falls into the last branch of `count_status` and is counted as "not in library" (dm_report.cpp 28), and `status_token` gives it the `tn` colour; the fill script gives an unknown status `s-only`. So a new status added in one place and missed in another does not fail; it shows in the wrong tile.

Disagree on: the DM wording for "not in library" is four strings (cell, tile, dropdown plus hover plus hover text, counts phrase) in six places; "not analyzed" is three strings in six places. They agree on meaning today. Fill: `in both` means different things in the dropdown (any chart with a record on each side) and in the tile and subtitle ("a score on one side only"), and that differs on the both-unscored input.
Owner proposed: the DM page keeps `kStatus*` and gains one table beside them that gives each status its table cell, tile, dropdown label, counts word and hover, so the six places read it. The fill page gets named constants in fill_report.h, used by the C++ and written into the script through placeholders the way `__OLD_RULE__` already is. Whether the words themselves are unified (one "your library" or "library") and what `in both` should say for the both-unscored chart are the user's calls.
Evidence: by reading. A `uitest` over a DM report with one score per state, and one `tally_fill_rows` case with both sides Ready and no paths, would prove the tile text.
Found by: G1a-2; verified by V1.
Scan row or reason: a row on `"under optimal"|"not analyzed"|"not in library"|"no paths"|"other speed"` as a bare string literal outside `dm_report.h` would flag a status typed by hand; same for the six fill literals outside the new constants. It will not catch a new display word, which has no shared spelling.

---

### M03. How is a text column sorted, and where do ties and missing values go?

Kind: K2 (same visible column, three sort rules; the order of rows is on screen). The windows follow the Library on purpose (D103 item 16); the fill page's script does not, and nothing records that as a choice. Fix: needs the user (a choice between behaviours; the Library part also changes which row sits where).

Homes:
- `LibraryModel::resort`, `src/ui/library_model.cpp` 225-276. Inputs: one sort column; the row's `searchable` strings, which `make_searchable` (library_query.cpp 362-370) builds as `fold_for_search(strip_rich_tags(raw))`; for Best path the score, then `unscored_rank(status)`.
- `TableView::resort` with `folded_key` and `compare_keys`, `src/app/report_view.h` 178-205 and report_view.cpp 29-49. Inputs: up to two `SortSpec`s; each column's `SortKey` (empty, number or text); text is `fold_for_search` of the display string, then plain byte compare.
- The page script's `render` sort, `src/app/html_page.cpp` 332-339. Inputs: one key, the raw cell value; strings by `localeCompare`; numbers by subtraction; null sorts last.

Does D103 item 16 cover the fill page? I grepped `docs/audit/2026-10-03-fix-decisions.md` for D103, "item 16", "localeCompare", "browser order" and "fill page". Item 16 (line 466) says "Text columns sort the way the Library table does ... The pages' browser order is not copied." Item 23 (line 473) applies it to the Path column and names the old page's browser order. Both sit under the D103 header (line 450), which says the fill-spawn comparison "stays an HTML page, because only `hydra_fillcompare` makes it". No item says the fill page's script follows or keeps its order. So the fill page is not covered. The comment at report_view.h 78-80 repeats item 16 for the windows only.

Tables. Text column, ascending, with the edges:

| Input | Library (Title, Artist, Charter, Folder) | Windows (Song, Artist, Charter, Mode, Path) | Fill page script |
|---|---|---|---|
| "apple" vs "Banana" | apple first (folded) | apple first (folded) | apple first in most locales (localeCompare ignores case at first level) |
| "Étude" vs "Etude" | tie (folded), then title/folder/path | tie, rows keep their order | not a tie; browser collation puts the accented one second |
| "Song 10" vs "Song 2" | "Song 10" first | "Song 10" first | "Song 10" first (digits compared one by one without a numeric option) |
| Path "0 1 3 0+" vs "0 1 3 0- 0" | not a column | "+" first (bytes) | "-" first (browser collation puts the hyphen before "+"), the reverse; D103 item 23 measured this on 81 of 369 corpus rows for the old page |
| empty text vs "a" | empty first ascending, last descending | empty first ascending, last descending (an empty string is a value) | empty first ascending |
| missing value (number column) | Best path: scored rows first in both directions | empty key sinks in both directions | null sinks in both; two nulls return "1" both ways (not a strict order) |
| title that is only tags, or blank | folded raw text is `""`, so it sorts first, while the cell shows "(unknown)" | folded "(unknown)" (the sort key is the display string, `display_title`) | raw cell value, already "(unknown)" |
| title with a leading space | one leading space kept (fold collapses runs, does not trim; `strip_rich_tags` does not trim), so it sorts before the same title without | trimmed by `display_title` (`clean_name`, song.cpp 531), sorts with it | trimmed, as the windows |
| artist `<unknown artist>` | raw `<unknown artist>` sorts under "<" | "(unknown)" | "(unknown)" |
| two rows equal on the sort key | title, then folder, then file path, always ascending whichever direction is on | original row order (`stable_sort`), unaffected by direction | engine order, unspecified |
| second sort key (Shift+click) | none (`Specs[0]` only, library_table.cpp 311-314) | up to `kMaxSortKeys` = 2 | none |
| first click on a number column | descending only on Best path (`PreferSortDescending`, library_table.cpp 288) | `first_direction(numeric)` descending | `setSort` picks descending for numeric |

Disagree on: (1) fill page against the windows: accented names, punctuation-against-letters (the "+" and "-" case), two sort keys, and tie order. (2) Library against windows: a title or artist the display rule rewrites to "(unknown)" and a name with a leading space sort at different places in the Library than the same song's row in a report, because the Library sorts on the stored text and the windows on the shown text. The Library's own cell shows `row.title = display_title(entry.title)` (library_model.cpp 136), so its sort key is not what its cell shows. The Library's own ties and the windows' ties also differ (fixed three-step tie-break against row order). Whether the scan ever stores a name with surrounding spaces I did not check; the Library case needs a chart whose title is blank or all tags, which the "(unknown)" fallback exists for.
Owner proposed: `fold_for_search` is already the one fold. The order itself needs one owner. The cheapest honest merge is a function that gives "the sort key for a name" (display string, folded) used by `make_searchable`'s sort field in the Library and by `folded_key` in the windows; but that moves "(unknown)" rows in the Library, which is a display change. For the fill page, the data JSON already carries `search` text folded with the library's fold; adding a folded sort field to each row would let the script compare bytes (`<`) instead of `localeCompare`, with no new rule. Which way each should go is the user's call.
Evidence: by reading. A `localeCompare` result depends on the viewer's browser and language, so the fill-page column of the table is by reading and cannot be pinned by a C++ test; the run that proves it is opening the fill page in a browser on a corpus with an accented title and the two Path strings above. The Library-against-window rows can be proven by a test that builds a `LibraryModel` and a `TableView` over one blank-title chart.
Found by: G1a-3, G1b-1, G1b-2, C1-2 (sort part), C2-7; verified by V1.
Scan row or reason: a row on `localeCompare\(` in `src/` outside `html_page.cpp`'s shared script would flag a second text sort in a page; the C++ sorts have no shared spelling beyond `fold_for_search(`, and the Library and windows already call it, so no row catches a new comparator written with other names.

---

### M04. What does the search box match, and what must the user type?

Kind: K3 between the Library and the report windows (two rules on purpose, recorded in D56 item 1 and in the comments at report_view.h 69-72 and html_page.cpp 314-320). K1 between the windows and the fill page's script (the every-word rule is written once in C++ and once in JavaScript; the fold is derived once). Fix: no merge available while the fill page is HTML; leave it. Revisit only if the fill page becomes a window (needs the user then).

Homes:
- `parse_library_query`, `query_matches`, `term_matches`, `src/app/library_query.cpp` 312-396 (inputs: typed text; the row's four folded fields; stars and hardest-ms facts).
- `search_words` and `search_matches`, `src/app/report_view.cpp` 8-27 (inputs: typed text; one joined, folded string per row built by `search_field`, library_query.cpp 372-382).
- `visible()` in the page script, html_page.cpp 321-327 (inputs: typed text folded through the `FOLD` table that `search_fold_table()` builds from `fold_for_search`, library_query.cpp 294-306; a per-row `search` string).

Tables. What a typed query does (all three fold case and accents the same way):

| Typed | Library | Report windows | Fill page script |
|---|---|---|---|
| `dream theater` | two terms, each in some field | two words, each in the joined string | same as windows |
| `"dream theater"` | one phrase, must sit inside one field | words `"dream` and `theater"` (quotes kept by the fold), matches nothing unless the text holds a quote | same as windows |
| `artist:foo` | term limited to the artist field | ordinary word, matches nothing | same |
| `stars:7`, `squeeze<=20` | filters; a row with no scored best path never matches (`facts_of`); a bad value shows an error line and drops the filter | ordinary words, match nothing | same |
| `foo:bar` (unknown prefix) | plain word | plain word | plain word |
| empty or only spaces | all rows | all rows | all rows |
| a word with an accent, `beyonce` for "Beyoncé" | matches | matches | matches (FOLD table) |
| two words that both sit in the one joined string but in different fields | matches (any field) | matches | matches |
| empty-result wording | "No charts match your search." and a "Clear search" button | "Nothing matches those filters." and a "Clear filters" button | "Nothing matches those filters.", no button |
| box hint | "Search title, artist, charter or folder" plus a line "Quotes match an exact phrase. Narrow with artist: charter: folder: stars:7 squeeze<=20" | "Search song, artist, or charter" (path report adds ", or path notation"); no help line | "Search song, artist, or charter" |

Words are space-free, so joining fields with a space cannot create a false match; the three agree on plain unquoted words. A query word cannot straddle the joining space.

Disagree on: any quote, `field:` prefix or filter, and the wording of the empty state and the hint. All recorded as on purpose except the empty-state wording ("No charts match your search." against "Nothing matches those filters."), which I found no decision for; it reads as a per-module sentence, not a shared one.
Owner proposed: no change. `fold_for_search` is the one fold and `make_searchable` the one field fold. The hints name which fields the matched text holds in a second place from the `search_field(...)` call; adding a field to one without the other leaves the box lying, but both are one-line per-module facts.
Evidence: by reading. A test that feeds one query set through `parse_library_query` and `search_words` (the plan says the windows are pinned in tests/test_report_view.cpp) already exists for the C++ halves; the script cannot be run from C++ tests.
Found by: G1b-3, C1-2 (search part); verified by V1.
Scan row or reason: no row catches a second matcher. The single place a hand-written fold could appear is `tolower`/`toLowerCase` on search text, and a row on `toLowerCase\(` in `html_page.cpp`'s script would flag a script that stops using the FOLD table; I'd add it only if the user wants the script frozen as is.

---

### M05. How is a "shown of total" count line written, and what does it count?

Kind: K2, small (same sentence shape, three writers; they differ at a total of exactly 1, and the Library leaves out "of Y" when nothing is filtered). Fix: needs the user (display text, one-line decision; D103 item 18 is the precedent for "singular at exactly 1").

Homes:
- `report_view::count_line`, `src/app/report_view.cpp` 51-54, used by both windows through `TableView::count_line` (nouns `{"path","paths"}`, `{"score","scores"}`); text is `group_thousands(shown) + " of " + counted(total, one, many)`.
- `render_heading`, `src/ui/library_table.cpp` 60-69: "N charts" when nothing is hidden, "N of M charts" when something is, noun by `counted`.
- The page script, html_page.cpp 389-390: `fmt(countOf(rows)) + ' of ' + fmt(countOf(ROWS)) + ' ' + PAGE.noun`; the fill page sets `noun: 'charts'` and counts each row's library copies (fill_report.cpp 88-95).

Tables (shown / total):

| Input | Windows | Library | Fill page |
|---|---|---|---|
| 0 / 0 | never drawn (no view is built for an empty report) | never drawn (`render_library` returns early on no rows) | never made (an empty comparison gives a reason sentence, not a page) |
| 1 / 1 | "1 of 1 path" | "1 chart" | "1 of 1 charts" |
| 0 / 1 | "0 of 1 path" | "0 of 1 chart" | "0 of 1 charts" |
| 3 / 3 | "3 of 3 paths" | "3 charts" | "3 of 3 charts" |
| 5 / 1,000 | "5 of 1,000 paths" | "5 of 1,000 charts" | "5 of 1,000 charts" |

Thousands grouping agrees (`group_thousands` and `toLocaleString('en-US')`; both write "-" before a negative; no negatives are counted). The fill page counts library copies, the windows count rows; deliberate (D76, D79).

Disagree on: a total of exactly 1 (the script never writes a singular), which is the input D103 item 18 decided for the windows ("The old page always wrote the plural"); and "of Y" left out by the Library when unfiltered, which is a title decoration, not a table count line.
Owner proposed: `counted` (core/model.h 873-875, whose comment says "Every screen writes counts here") stays the owner. The fill page's script should take its noun as a singular/plural pair from C++ (`PAGE.noun` becomes `nounOne` and `nounMany`) so the plural rule is written once.
Evidence: by reading; pinned for the C++ side by tests/test_report_view.cpp. The fill page needs a one-chart database run through `hydra_fillcompare` to see "1 of 1 charts".
Found by: G1a-4, G1b-5; verified by V1.
Scan row or reason: a row on `" of "\s*\+` (a string built with " of " in `src/`) outside `report_view.cpp` and `library_table.cpp` would flag a third writer in C++; the script's `' of '` is a JS literal and a row on `' of '` in `html_page.cpp` is the way to flag it once the pair is in place.

---

### M06. Small view helpers copied between the two window views (`text_column`, `ms_key` against `optional_key`)

Kind: K1 (same rule, two homes; I could find no input where they differ for like-typed values). Fix: code-only.

Homes:
- `text_column`, `src/app/path_report_view.cpp` 26-36 (id, title, definition, member pointer, look) and `text_column`, `src/app/dm_report_view.cpp` 37-44 (same without `definition`; it goes through `column(...)` with an empty definition, which passes it through `fill_constants`, lines 28-35). Both set `sort_key` to `SortKey{r.*field}`, `cell` to `r.*field`, `numeric` false.
- `ms_key`, path_report_view.cpp 21-23 (`optional<double>` to a number or an empty key) and `optional_key<T>`, dm_report_view.cpp 46-49 (any `optional<T>` to a double or an empty key).
- The missing-value cell: `ms_cell` (path_report_view.cpp 17-19) and inline lambdas in `dm_columns` (optimal 79-81, delta 91-95, rank 145-147, posted 157-159), each writing `report::kDash` for a missing value; the dash itself is one constant.

Tables.

| Input | Path view | DM view |
|---|---|---|
| text field "" | sort key text "" (a value, first ascending), cell "" | same |
| optional number unset | empty key (sinks), cell dash | same |
| optional number exactly 0 | number 0, cell "0.0 ms" (format_ms) | number 0, cell "0" (group_thousands) |
| a `double` that is not optional | `SortKey{r.mult}` direct | n/a |
| `posted` "" | n/a | empty key (sinks), cell dash (D103 item 17) |

The one deliberate difference, an empty `posted` sinking while an empty `song` sorts as text, is recorded (D103 items 16 and 17) and lives in the DM view only.

Disagree on: none today; changing how an empty text cell sorts would have to be made in two `text_column`s, and in the DM-only `posted` rule. A third column type with an optional (a new report) would write a third converter.
Owner proposed: `report_view.h` keeps `SortKey` and gains one `optional_key` template (the DM spelling, which already takes any type) and one `text_column` that takes the definition; both views call them. The one-line DM wrapper that applies `fill_constants` stays in the DM view.
Evidence: by reading.
Found by: G1a-5; verified by V1.
Scan row or reason: a row on an optional-to-`SortKey` conversion outside `report_view.h`: spelling `\?\s*SortKey\{[^}]*\}\s*:\s*SortKey\{\}` in `src/app/*_view.cpp` (the plan's own example). It would flag `ms_key` and `optional_key` if either came back.

---

### M07. The report table written again in the fill page's JavaScript, and the words and colours it shares with the windows

Kind: K2 for the rules (sort order, singular at 1, no second sort key; see M03 and M05), K1 for the shared words and the three shared colours. Fix: needs the user for the sort order and the plural (display); the colours and the words are code-only if no word changes. The tile arithmetic is a K1 that agrees today.

Homes:
- `kReportJs`, `src/app/html_page.cpp` 303-402, used by one page: `html::page_template` has one caller, `fill_report.cpp` 159. It holds the search (`visible`), sort, count, empty text and tile redraw. The page's own `PAGE` (fill_report.cpp 90-152) holds `filter`, `cells` and `stats`.
- The windows' side: `TableView`, `search_words`, `compare_keys`, `count_line` (report_view.*), `report_frame::nothing_matches` (src/ui/report_window.cpp 334-338), `report_frame::tiles`.
- Colours: `chip_color`, `src/ui/theme.cpp` 9-25 and the page CSS, html_page.cpp 117-141 (`--t0`, `--t3`, `--tn`).

Tables.

| Rule | Windows | Fill page script |
|---|---|---|
| search | fold, split on spaces, every word in the joined text | same (FOLD table from the library's fold, split on `\s+`, every word) |
| text sort | folded bytes, two keys, row order on ties | `localeCompare`, one key; see M03 |
| missing value | sinks, two missing values tie | sinks; two missing values return "1" in both orders |
| count line | "X of Y <noun>" with `counted` | "X of Y charts", never singular |
| empty state | "Nothing matches those filters." plus Clear filters | same sentence (html_page.cpp 387 against report_window.cpp 336), no button |
| search box hint | "Search song, artist, or charter" (DM; fill_report.cpp 48 is the same sentence) | same |
| thousands | `group_thousands` | `toLocaleString('en-US')` (agrees) |
| dash | `report::kDash` | `DASH = '\u2014'` (html_page.cpp 295) and `report::kDash` for the Acts cell (fill_report.cpp 299-302) |
| tile arithmetic | `tally_fill_rows` sums each row's copies by status | `stats()` sums `k` by status (fill_report.cpp 136-150); pinned only by a test that searches the HTML text (tests/test_fill_report.cpp 313-339), not by running the script |
| colours (dark scheme) | t0 4fbf94, t1 e0b13f, t2 f0894e, t3 f2686b, t4 e07ac0, t5 a78bfa, tn 949aa6 | `--t0` 4fbf94, `--t3` f2686b, `--tn` 949aa6 only; no variable for t1, t2, t4, t5 |

The comment at theme.cpp 11-13 says the window's tiers are "the HTML pages' dark-scheme --t0 to --t5 and --tn". That is true for three of the eight only; the other pages that used the rest are gone (D103), so the comment is out of date. The page also has a light scheme and a print scheme the windows lack. The same token name means three things: `tn` is "no squeeze" on the path report, "not in library" on the DM page and "same" on the fill page; that is a per-report choice (K3), not a copy.

Disagree on: text order, singular at 1, second sort key, and tie order (the rules); none on the words, the three colours or the tile sums today. A change to t0, t3 or tn has to be made in both files.
Owner proposed: `TableView` and `counted` stay the owners. While the fill page stays HTML, C++ writes everything the script would otherwise restate: the noun pair, the empty-state sentence, the hint and the status words go in through placeholders (the way `__OLD_RULE__` does), and the three colours come from `chip_color` as hex. Only the rules that cannot be shared across languages (sort and filter loops) stay in the script. The cleaner end state is to move the fill page into a window like the other two; that is the user's call and a bigger change.
Evidence: by reading. The sort and plural differences are proven by opening the fill page; the colour claim by diffing the hex in theme.cpp against html_page.cpp 117-141 (done by reading here).
Found by: G1b-6, G1a-8, G11b-4; verified by V1.
Scan row or reason: a row on the literal `Nothing matches those filters` in `src/` outside the two files that hold it today would flag a third copy (no row catches a different sentence); the colours have a spelling, `#4fbf94`/`#f2686b`/`#949aa6` hex in `src/` outside `theme.cpp` and the CSS block, and a row on those three hex strings would flag a fourth copy.

---

### M08. "Keep listed records whose chart the library lists": the SQL join against the in-memory `library_lists` test

Kind: K1 for the two loops in `report.cpp`; for the store's SQL, K1 on every input the scan can write, with one rare input where they split (a hash differing only in letter case). Fix: code-only for the two loops (low value); the SQL pair is needs the user only if the store starts lowercasing hashes on write (a stored record).

Homes:
- `records_by_hash`, `src/app/report.cpp` 155-169: builds the library map, lists one chart mode, drops a listing unless `library_lists(library, normalize_chart_hash(hash))`, keys by the normalized hash (`emplace`, first wins).
- `collect_rows`, report.cpp 232-237: lists every mode, the same drop through `library_lists(copies, hash)`, one slot per listing.
- `library_lists`, report.cpp 171-174: `library.find(hash) != library.end()`; the one owner of the in-memory test. `library_copies_by_hash` (194-199) lowercases the library's md5s and adds up copies.
- The store: `not_in_library`, `src/store/record_store.cpp` 1247-1249, `NOT EXISTS (SELECT 1 FROM charts WHERE charts.md5 = results.hyhash)` (exact text), used by `purge_charts_not_in_library` on every rescan and every `reidentify_chart` (1265, 1273-1276, 1543); and the `LEFT JOIN kNamingCopiesSql ... ON c.md5 = r.hyhash` in `list_records` 1455-1456 (exact; an unmatched result is listed with empty names, per the comment at 1446-1448).

`list_records` has exactly two callers, both in report.cpp (160 and 233), and both follow it with the `library_lists` drop. So the store lists results the library does not list (D77) and both callers then throw them away.

Tables (listing hash against the library's md5):

| Listing hash | library has the same md5 | Callers (`library_lists`) | Store join in `list_records` | Rescan purge (`not_in_library`) |
|---|---|---|---|---|
| `abc` | `abc` | kept | names filled | kept |
| `abc` | none | dropped | listed, names empty | deleted on the next rescan |
| `ABC` (upper case) | `abc` | kept (both lower-cased) | listed, names empty | deleted on the next rescan |
| two listings, one hash, one mode | any | `records_by_hash` keeps the first, `collect_rows` keeps both; one mode per call so no duplicate arises there | n/a | n/a |

Disagree on: a result whose hash differs from the library's md5 only in letter case. `analysis.h` 130-133 says the scan writes lower-case hex, so I found no code path that writes an upper-case `results.hyhash`; an older row could hold one. If one exists, the report counts it and a rescan deletes it. I did not read the user's database. The run that settles whether such a row exists: on a copy of `hydra.db`, `SELECT COUNT(*) FROM results WHERE hyhash <> lower(hyhash)`.
Owner proposed: `library_lists` stays the in-memory owner. The two loops in `records_by_hash` and `collect_rows` could share a helper "listings the library lists", which is the only code-only part. The SQL purge stays exact; if the count above is ever non-zero, make the store lower-case on write (a stored change, user's call) and the callers' `normalize_chart_hash` becomes redundant.
Evidence: by reading, proven by the one-line query above on a copy of the db, or a store test that writes `ABC` and scans `abc`.
Found by: G1a-7, G10-3; verified by V1.
Scan row or reason: a row on `library_lists(` is not needed (it has one owner). A row on `charts.md5 = results.hyhash` outside `record_store.cpp` would flag a second SQL spelling of the join; the in-memory test is already one call.

---

### M09. What a report window remembers between frames and forgets on close, and who closes it

Kind: K3 (two windows that differ on purpose; every rule is in one place). Fix: none. The difference between them is recorded: D103 item 28 says the path report frees its rows on close and "the comparison window is unchanged", and `app_state.h` 213-217 says "the path report only".

Homes:
- Path: `PathWindow` and `rebuild(w, result, keep)`, `src/ui/path_report_window.cpp` 29-104; `drop_rows` 59-64 and the close branch 172-178; `ReportSlot::built_on_open` and `AppState::close_path_report`, `src/ui/app_state.cpp` 730-739, 781-782.
- DM: `DmWindow` and `rebuild`, `src/ui/dm_report_window.cpp` 25-69; no close callback (`dm_report_input` sets none, lines 132-154), no `drop_rows`.
- The shared frame: `report_frame::Memory`, `src/ui/report_window.h` 190-208, and the close detection in `report_frame::end`, `src/ui/report_window.cpp` 270-271 (`m.was_open && !*open` calls `callbacks->close` if set).
- Who opens: `show_path_report` (app_state.cpp 724-728) and `start_dm_report` (657-672) set `window_open`; the window's own `open` flag clears it; library_view.cpp 206-209 only draws a window whose flag is set.

Tables.

| Event | Path report | DM comparison |
|---|---|---|
| Close (X, Esc, Ctrl+W) | `close_path_report` runs through the frame: a running build is cancelled and parked, the slot is reset to never built, the window drops its copy of the rows (`drop_rows`) and `built_from`; search box, Timing dropdown, "Best path only" and sort stay in `PathWindow` | `window_open` false and nothing else: the slot keeps its result, and the window keeps its `TableView`, a copy of the rows |
| Reopen | `show_path_report` builds again; the new result is flagged `built_on_open`, so `rebuild(keep=true)` restores the search, and the timing choice, best-only and sort survive; selection and "Show files" reset (`Memory::reset`) | there is no reopen: the window only reopens through `start_dm_report`, which starts a new job; while it builds, the window shows the old result and old memory, then `rebuild` runs on landing and starts everything over |
| Refresh, or a new result | `built_on_open` false, `rebuild(keep=false)` starts everything over | `rebuild` starts everything over (status index 0, search cleared) |
| Hit window or cap changes (path only) | a kept timing index is reset when the labels change (path_report_window.cpp 98-102); a kept sort naming a missing column falls back to the first (88-95) | n/a |

Disagree on: what is freed on close (path frees, DM keeps) and what survives a reopen. Both documented. The `if (!*open)` branch at the top of each `draw_*_report_window` (path 143-146, DM 108-111) can only run if a caller passes a closed flag, and `library_view.cpp` only calls them when open, so it is dead code today (see Side notes).
Owner proposed: none. The two rules that exist only in the path window (a kept sort must name a real column; a kept index resets when the choices change) are consequences of that window keeping state across rebuilds. If the user ever asks the DM window to free its rows, it needs its own copy of the three-part mechanism, or the mechanism moves into the shared frame.
Evidence: by reading; D103 item 28 (fix-decisions line 478) records the intent. A uitest that closes and reopens each window would prove the table.
Found by: G1b-7, C2-8, G11a-8 and the "also seen" block in G11a; verified by V1.
Scan row or reason: no text search can catch a second place that forgets state on close; the state sits in module-local structs under different names (`PathWindow`, `DmWindow`), and the recurrence gate is the catch.

---

### M10. What "Clear filters" resets, and what "no filter" means

Kind: K1, small (the convention "choice index 0 keeps every row" is written as a literal 0 in seven places and owned only by the order of two lists); the control sets and what Clear touches are per-module facts (K3). Fix: code-only.

Homes:
- `PathWindow`, `src/ui/path_report_window.cpp`: field default `timing = 0` (36), `rebuild` (75, and again at 101 when the labels change), Clear filters branch (123-130: search cleared, `timing = 0`, "Best path only" left, sort kept). `timing_choices` (path_report_view.cpp 66-72) puts `{nullopt, "All timing tiers"}` first; `path_keep` treats a null tier as keep.
- `DmWindow`, `src/ui/dm_report_window.cpp`: field default `status = 0` (32), `rebuild` (54), Clear filters branch (84-90: search cleared, `status = 0`, sort kept). `status_choices` (dm_report_view.cpp 182-194) puts `{nullopt, "All charts"}` first; `dm_keep` treats a null status as keep.
- The Library has no twin: its chip falls back to All silently when its group empties (`refilter`, library_model.cpp 291-297), and "Clear search" is its own button.

Tables.

| Event | Path window | DM window |
|---|---|---|
| new result, not kept | search cleared, timing 0, best-only true, sort cleared | search cleared, status 0, sort the first sort |
| new result, kept (reopen) | search kept, timing and best-only and sort kept | n/a |
| Clear filters | search cleared, timing 0; best-only and sort untouched | search cleared, status 0; sort untouched |
| a filter leaves nothing | "Nothing matches those filters." plus Clear filters | same |
| the "all" choice | `choices[0]`, `tier == nullopt` | `choices[0]`, `status == nullopt` |

Disagree on: none today. The convention "index 0 is the choice that keeps everything" is stated only in the header comments (path_report_view.h 43-45, dm_report_view.h 38-39) and by the first entry of each list. Reordering a list silently makes Clear filters reset to a real filter.
Owner proposed: the choice lists own their "all" entry: a named constant (or a function on the list) in `report_view.h`, used by both windows' default, rebuild and Clear branches, so the literal 0 is gone.
Evidence: by reading.
Found by: G1b-8; verified by V1.
Scan row or reason: a row on `\b(timing|status)\s*=\s*0\b` in `src/ui/*report_window.cpp` would flag a literal index back in the windows.

---

### Side notes

- `src/app/path_report_view.cpp` 124: the Timing column sorts on `r.tier`, the tier's name as text, so a click sorts the tiers alphabetically, not by how hard they are. I did not check whether the tier names happen to sort in ladder order; by reading it looks like a bug or an unstated choice.
- `src/ui/dm_report_window.cpp` 96-98 with `src/app/dm_report.cpp` 128: an off-speed score on a chart the library does not list reads "other speed" (the last assignment wins), so the row is clickable (the test is `status != kStatusNotInLibrary`), and the click calls `select_chart`, which may switch the Settings bar to the row's mode before it finds no copy and selects nothing (`src/ui/app_state.cpp` 479-512). D103 item 12 says a "not in library" row's click does nothing. By reading.
- `src/app/html_page.cpp` 332-339: the page's sort returns 1 for two missing values in either order, which is not a consistent comparator. Harmless in practice (both sink), but it is not a strict order.
- `src/ui/dm_report_window.cpp` 108-111 and `src/ui/path_report_window.cpp` 143-146: the `if (!*open) { w.memory.was_open = false; return; }` branch is unreachable while `library_view.cpp` 206-209 only calls them with an open flag. The close is noticed in `report_frame::end` (report_window.cpp 270-271) before it.
- `src/app/fill_report.cpp` 238-239 with the tile and subtitle wording: "in both" also covers a chart where both sides hold a record and neither kept a path; "a score on one side only" is wrong for it (see M02).
- `src/app/path_report_view.cpp` 122 and `src/app/report.cpp` 421-423: the prose "Beyond means more than twice the hit window" and "past the N ms window" restate the rule that `beyond_edge_ms` / `timing_tiers` own. Agrees today (kIdentityScale is 1.0); it would split only if that scale changed.
- `src/ui/theme.cpp` 11-13: the comment about the HTML pages' `--t0` to `--t5` is out of date for four of the eight tokens.
- `src/store/record_store.cpp` 1383-1387 (`has_record`) and `src/ui/app_state.cpp` 429-431: scout G10-5 noted `has_record` has no production caller and `save_view_summary` spells `get_summary(key)` out by hand. Not in my batch; I did not re-verify.

---

## V2 findings: which settings matter, and how they are named

Verifier: concept-audit-V2 (read-only, phase 4). Agent id used for status lines: `V2` (the progress hook never named another). Batch: M11 to M20 in `candidates.md`. Everything below was checked against the code as read on 2026-10-11. Paths are under `src/` unless a `tests/` path is given. Nothing was built or run; where a run would settle a claim, it is named.

Summary: one real hole (M17), one dormant gap with a known fix (M11), two clean K1 merges with no behaviour change (M12, M19), one K2 word question for the user (M15), two low-weight K1s (M13, M14), one K1 that is really a test helper living in `src/` (M18), two K0 (M16, M20).

### M11. Which settings does each report page read, and which make an open report out of date?
Kind: K1 (the list "which settings does this page read" is kept by hand in `settings_change_touches` and has no owner; it is one short of what the path report reads). Fix: code-only for the path report (adding `hit_window_ms` to the comparison changes nothing anyone can trigger today); the wider fix (derive both lists from one statement per generator) is code-only too.
Homes: `report::settings_change_touches`, `app/report.cpp` 431-437 (cap_query, lens);
       `dm_report::settings_change_touches`, `app/dm_report.cpp` 237-241 (chartmode_key, lens);
       what the path report actually reads: `AppState::launch_path_report`, `ui/app_state.cpp` 705-718 (cap, lens, `settings.hit_window_ms`, `batch_run()`), consumed in `generate_report` / `collect_rows`, `app/report.cpp` 201-300, 356-360 (hit window at 299 and 359-360);
       what the comparison reads: `AppState::launch_dm_report`, `ui/app_state.cpp` 681-686 (chartmode_key, lens) into `generate_dm_report`;
       caller: `AppState::apply_settings`, `ui/app_state.cpp` 866-870.
Tables (setting changed -> does the open page go out of date? / does the generator read it?):

| Setting changed | Path report reads it | Path touches says | Comparison reads it | Comparison touches says |
|---|---|---|---|---|
| difficulty, Pro Drums, 2x Bass, Note Shuffle | no (lists every mode; `settings_for_mode` takes each mode's own parse choices) | no | yes (`chartmode_key`) | yes |
| SP cap | yes | yes | no (forced to `kCloneHeroSpCap` in `collect_dm_rows`) | no |
| ms limit on/off, ms value, score range mode/value, 1.0 fills (the lens) | yes | yes | yes | yes |
| `hit_window_ms` | yes (tier bands, "Past N ms" tile, footer, Timing chip) | NO | no | no |
| `auto_open_report`, volume, backend limit, folders | no | no | no | no |

The `hit_window_ms` dispute, settled. Both scouts are right and they do not conflict.
(1) "No control writes it" is true. A grep of every source file finds one writer: the INI loader (`Settings::load_file`, via the `hit_window_ms` entry in the key table, `app/config.cpp` 140). No widget in `ui/` assigns it, and `Settings::load()` is called once at start-up (`ui/app_state.cpp` 24). The panel has no row for it (`ui/settings_bar.cpp` 81-194).
(2) "The report is built from it" is true. `launch_path_report` hands `settings.hit_window_ms` to `ReportJob` (`app_state.cpp` 713), which sets `options.hit_window_ms` (`ui/library_jobs.cpp` 591); `collect_rows` builds the tier bands from it (`report.cpp` 299) and `generate_report` stores it in `GeneratedReport::hit_window_ms` (360), which the window reads for the column labels and Timing choices (`ui/path_report_window.cpp` 86, 98, 111).
So the gap is real in the list and dormant in use: the list leaves out a setting the report reads, and nothing can change that setting while a report is open. It becomes live the day a hit-window control is added. The 2026-10-08 handoff already says so. The Paths tab carries the same dependency a third way: it re-reads the value every frame and compares it with the last build (`PathsTabCache::details`, `app/path_view.cpp` 564-577), so one dependency has a pull-style cache key in one place and a push-style list (missing it) in another.
Disagree on: the hit window (read, not listed). No other setting differs. The comparison list agrees with what the comparison reads.
Owner proposed: each generator states once the settings it reads (a small function or struct beside `generate_report` / `generate_dm_report`); `settings_change_touches` and the Paths tab cache compare through it. Smallest step: add `before.hit_window_ms != after.hit_window_ms` to the path report's function now. Why: the list lives next to the code that reads the settings, so a new read updates it in the same place.
Evidence: by reading. A test would pin it: in `tests/test_report_rows.cpp` the existing case at 839-868 uses `auto_open_report` as "a setting no report reads"; a new `-tc` case that changes `hit_window_ms` and expects `report::settings_change_touches` true would fail today (red) and pass after the one-line fix.
Found by: G1a-6, G3-3, also-seen G3-3 in G11a; verified by V2.
Scan row or reason: no text search can catch it. The wrong shape is a missing comparison in a function body, which has no spelling to match. The recurrence gate or a test that walks the fields `generate_report` reads is the catch.

### M12. Which settings pick a stored result: chart mode, cap and lens as three loose parts?
Kind: K1 (one rule, "a result is filed under (chart, chart mode, cap, lens)", spelled as one struct in some places and as three loose arguments in others; the pieces always come from the same `Settings` call so they agree today). Fix: code-only.
Homes: `RecordKey{hyhash, chartmode, cap, lens}`, `store/record_store.h` 150-160, built by `Settings::record_key`, `app/config.cpp` 346-348, and by `run_batch`, `app/analysis.cpp` 956 (`RecordKey{item.md5, run.chartmode, cap, run.lens}`);
       the loose triple (chart mode, cap, lens): `RecordStore::get_summaries` (`record_store.cpp` 1307-1309), `analyzed_hashes` (1389-1391), `list_records` (1440), and their callers `ui/library_jobs.cpp` 67-68, `app/analysis.cpp` 828, `app/report.cpp` 160 and 233, `cli/fillcompare.cpp` 78-80, `ui/app_state.cpp` 429-430 (`save_view_summary` unpacks a `RecordKey` into the triple by hand);
       the loose triple compared by hand: `AppState::apply_settings`, `app_state.cpp` 858-859; the two `settings_change_touches` (M11); `run_batch` seed guard `analysis.cpp` 877; `collect_rows` guards `report.cpp` 207 and 210-211 (`ReportOptions` carries `cap` and `lens` and also `run`, which carries the same two again, so the code checks they agree).
Tables: no input splits them. Edges checked: ms limit off with any stored value (the lens is canonical, `ms_value` 0, `record_store.h` 112-114); chart mode text built by `chartmode_key` in both paths; cap 4 vs a what-if cap (`CapQuery::at` in both). `BatchRun::cap_query()` and `Settings::cap_query()` are the same one-line `CapQuery::at(sp_cap)`.
Two small rules sit inside the loose form:
- `save_view_summary` tests `!found.empty() && status == Ready` (`app_state.cpp` 431). `!found.empty()` can never be false, because `get_summaries` returns one slot per hash asked (`record_store.cpp` 1310). The same test is `RecordStore::has_record` (1383-1386), which has no production caller (tests only). `get_summary(key)` is the one-slot form.
- The guards in `collect_rows` and `run_batch` exist only because the same identity travels as separate fields.
Disagree on: none today; a fifth key part (a new lens field) must be added to each triple's signature and each hand comparison, and the compiler stays silent if one is missed.
Owner proposed: `RecordKey` stays the identity; give the lookups a hash-less identity (chart mode, cap, lens) that `RecordKey` embeds, and have `get_summaries`, `list_records`, `analyzed_hashes`, `ReportOptions` and `BatchRun` take it. The hand comparisons become one `==`, and the guards in `collect_rows` and `run_batch` shrink to one comparison each. `save_view_summary` calls `get_summary(key)`. Why: the key struct already exists and already has `==`; the loose triple is that struct taken apart.
Evidence: by reading. A compile plus the existing store and report tests (`-sf=test_record_store.cpp`, `-sf=test_report_rows.cpp`) would prove no behaviour moved.
Found by: G3-1, G2-6, G10-5, also-seen G3-1 in G4b and G11a; verified by V2.
Scan row or reason: a row in `rules/app.json` or the store area file on the loose spelling `chartmode_key\(\)\s*,\s*[\w.>-]*cap_query\(\)` outside `config.cpp` would flag a caller re-assembling the triple. It cannot see a triple passed as already-named variables (`run.chartmode, cap, run.lens`), so the type change, not the scan, is the real guard.

### M13. The Analysis panel's settings spelled by hand in six or more places (including `settings_at_defaults`)
Kind: K1 (one rule, "which `Settings` fields are the analysis set", repeated as hand lists that agree today).  Fix: code-only.
Homes (field coverage; "10 fields" = difficulty, Pro Drums, 2x Bass, Note Shuffle, SP cap, 1.0 fills, score range mode, score range value, path limit on/off, path limit value; the panel shows these as 8 settings):
       `Settings::with_analysis_defaults`, `app/config.cpp` 281-295 (all 10);
       `Settings::to_analysis_settings`, config.cpp 315-328 (all 10, plus rules);
       `chartmode_key` + `cap_query` + `lens`, config.cpp 266-279, 336-344 (all 10);
       `settings_change_phrases` and so `settings_at_defaults`, `ui/library_dialogs.cpp` 104-123, 140-142 (all 10, as 8 phrases);
       `batch_settings_summary`, library_dialogs.cpp 86-98 (all 10, as 5 rows);
       the three panel groups, `ui/settings_bar.cpp` 81-194 (all 10 fields, 8 settings, 10 widgets);
       `with_chartmode`, config.cpp 297-313 (the 4 parse choices, by loop);
       `PreviewSongKey`, `ui/preview_controller.h` 56-68 (the 4 parse choices; see M19);
       the INI key table `keys()`, config.cpp 122-149 (every persisted field; it carries range and default per key but no "changes a result" flag).
Tables: each field in each list. All ten fields are in every full list; `rules` is in `to_analysis_settings` only (correct, it changes at restart only); hit window, backend limit, volume, auto-open, dm user, folders are in none. The summary's edge cases agree with the reset: path limit off with any value shows "Path limit off" (enabled differs from the default, which is on); path limit on at 10 shows nothing; 2x Bass goes through `effective_bass2x()` in both.
Two edges where `settings_at_defaults` and the field-by-field reset can differ, both unreachable from the panel: an uncleaned `view_difficulty` string such as "expert" compares equal through `difficulty()` (finding 134's reason) while the field differs from the default string; a raw `depth_mode` of 2 maps to scores in `search_depth_mode()` (no phrase) while `lens()` carries 2 (M14). Neither changes what the user sees.
The test that guards drift exists: `tests/test_config.cpp` 605-658 walks the ten fields one at a time and checks `settings_at_defaults` before and after `with_analysis_defaults`, and checks the key, the lens and `settings_key` match the defaults after a full reset. It would not catch a new setting forgotten in all lists at once, because it names the ten fields by hand.
Disagree on: none today; a new analysis setting needs a field, a line in each list above, a line in `settings_key` and a line in that test, with nothing forcing the set.
Owner proposed: `Settings` states the set once (a small table of the analysis fields: member, default, whether it is part of the chart mode, the cap or the lens), and `with_analysis_defaults`, the phrases, the Reset test and the key trio read it. This is the larger refactor in the batch and the one with the least payoff; a cheaper step is a test that walks `keys()` and requires every key either appear in the set or be named display-only. Why: the key table already exists and already carries per-key facts.
Evidence: by reading. `-tc="settings reset*"` in `tests/test_config.cpp` proves the current agreement.
Found by: G3-4, also-seen G3-4 in G11b; verified by V2.
Scan row or reason: no text search can catch a forgotten field in a list. A walk-the-key-table test is the catch.

### M14. What 0 and 1 mean for the score-range unit, kept as an enum and two ints
Kind: K1 (agree on every reachable input). Fix: code-only.
Homes: `Settings::search_depth_mode`, `app/config.cpp` 249-251 (1 is points, any other int is scores);
       the panel combo, `ui/settings_bar.cpp` 168-173 (`mode_idx = depth_mode`, items "scores", "points"; it only ever writes 0 or 1);
       `store::Lens::depth_mode` and `Settings::lens()`, `store/record_store.h` 120 and `config.cpp` 340-344 (the raw int goes straight in);
       `hydra::settings_key`, `search/pather.cpp` 69 (`static_cast<int>` of the enum, which relies on the order in `search/engine.h` 20, `{ Scores, Points }`);
       the INI key, `config.cpp` 133 (`number("depth_mode", ..., 0, 1, Outside::Default)`, so a file value outside 0..1 loads as the default 0).
Table (int -> unit seen by each reader): 0 -> scores everywhere; 1 -> points everywhere; 2 or -1 (code or tests only; the INI loader and the combo cannot produce them) -> `search_depth_mode()` scores, panel combo shows an out-of-range index, `Lens` files it as 2 (a different stored row from 0 even though the search ran identically), `settings_key` of the converted enum is 0. `tests/test_config.cpp` 598-599 pins that 7 maps to scores in `to_analysis_settings`.
Disagree on: an int other than 0 or 1, unreachable in the app. The `Lens` form is the only one that does not normalise, although the `Lens` header says "Canonical form, so equal settings always compare equal" (`record_store.h` 112).
Owner proposed: `search_depth_mode()` stays the one reader; `Settings::lens()` passes `search_depth_mode() == DepthMode::Points` rather than the raw int, so the lens is canonical like the ms value already is. The panel still binds the int (the combo needs an index). Why: this makes the rule "1 is points, anything else is scores" live once. Stored rows for 0 and 1 are untouched, so no record changes.
Evidence: by reading. A test case calling `Settings::lens()` with `depth_mode = 7` and expecting `Lens::depth_mode == 0` would be red now, green after.
Found by: G3-5, also-seen G3-5 in G4b and G11b; verified by V2.
Scan row or reason: a row on `static_cast<int>\(.*depth_mode\)` outside `pather.cpp` and on `depth_mode == 1` outside `config.cpp` would flag a reader that re-decides the unit; it cannot flag a missing normalisation.

### M15. What each analysis setting is called in words, on each surface
Kind: K2 for the words the user sees (same setting, different words), with two K1 pieces hidden inside that are code-only (the same count-with-noun mapping written in separate places). Fix: needs the user (display text). The two K1 pieces are code-only if the merged function gives the same words each caller printed.
Homes (the wording functions): `describe_settings`, `app/config.cpp` 354-361 (one caller, `cli/batch.cpp` 183; the header at `config.h` 219-226 calls it "the one place they are spelled");
       `sp_cap_value`, `score_range_value`, `path_limit_ms_value`, `batch_settings_summary`, `settings_change_phrases`, `ui/library_dialogs.cpp` 60-123;
       the panel's labels and help, `ui/settings_bar.cpp` 83-193;
       the batch confirm's row labels, `ui/library_dialogs.cpp` 369-373;
       `within_label`, `app/path_view.cpp` 476-480;
       the path report's subtitle label, `app/report.cpp` 408-411, and the empty-page sentence, `nothing_under_settings`, report.cpp 350-354;
       the comparison gate's sentences, `app/dm_report.cpp` 35-47, and its footer, 225-230.
Exact words for the same values (default values unless stated; "-" = not shown there):

| Setting and value | Panel (`settings_bar.cpp`) | Settings button / Reset tooltip (`settings_change_phrases`) | Batch confirm rows (`batch_settings_summary`) | CLI header (`cli/batch.cpp` 181-190, `describe_settings`) | Path report / Paths tab / comparison |
|---|---|---|---|---|---|
| Difficulty Hard, Pro Drums on, 2x Bass on | rows "Difficulty", "Pro Drums", "2x Bass", "Note Shuffle" (checkboxes, a combo) | "Hard" (Pro Drums and 2x Bass say nothing while on; "Pro Drums off", "2x Bass off" when off) | row "Difficulty": "Hard · Pro Drums · 2x Bass" (Note Shuffle appends " · Note Shuffle") | "Chart mode : Hard Pro Drums, 2x Bass" | chart mode text "Hard Pro Drums, 2x Bass" |
| Note Shuffle on | "Note Shuffle" | "Note Shuffle" (off: "Note Shuffle off", only if default differs) | appended to Difficulty value | "..., Note Shuffle" in chart mode | " ..., Note Shuffle" |
| SP cap 4 | row "SP cap", box then "bars", help "4 bars is Clone Hero's rule. Any other cap is a what-if..." | (default, so nothing) | row "SP cap": "4 bars (Clone Hero's rule)" | "SP cap     : 4 bars" | report subtitle "SP cap 4 bars"; empty page "(SP cap 4, ...)"; gate "Needs SP cap 4, Clone Hero's rule: ..."; comparison footer "at SP cap 4" |
| SP cap 1 | same | "SP cap 1 bar" | "1 bar (a what-if)" | "SP cap     : 1 bar" | subtitle "SP cap 1 bar"; empty page "(SP cap 1, ...)" |
| 1.0 fills on | row "1.0 fills" (checkbox) | "1.0 fills" (off: "1.0 fills off") | row "Fills": "Clone Hero 1.0" (off: "Clone Hero 1.1") | "Fill rule  : Clone Hero 1.0" | subtitle "SP cap 4 bars — Clone Hero 1.0 fills" (nothing said for 1.1); empty page "Clone Hero 1.0 fills"; gate "Needs Clone Hero 1.1 fills: untick \"1.0 fills\"." |
| Score range 4, scores | row "Score range": number box, combo "scores" | (default, nothing); 100 -> "Score range 100 scores" | row "Score range": "4 scores" | "Depth      : scores 4" | Paths tab heading "Within 4 scores" |
| Score range 1, points | combo "points" | "Score range 1 point" | "1 point" | "Depth      : points 1" | "Within 1 point" |
| Score range 2000, points | - | "Score range 2,000 points" | "2,000 points" | "Depth      : points 2000" | "Within 2,000 points" |
| Path limit 10 ms | row "Path limit": checkbox, box, "ms" | (default, nothing); 20 -> "Path limit 20 ms" | row "Path limit": "10 ms" | "Timing cap : 10 ms" | - |
| Path limit off | checkbox unticked | "Path limit off" | "off" | "Timing cap : none" | - |
| At all defaults | - | button reads "Analysis settings: defaults" | - | - | - |

Disagree on (the values a player can compare side by side):
1. The CLI header names three settings differently from the panel, the dialogs and `CONTEXT.md` ("Score range", "Path limit"): "Depth" for the score range, "Timing cap" for the path limit, "Fill rule" for 1.0 fills. It also writes "none" where the GUI writes "off", "scores 4" where the GUI writes "4 scores", and "points 2000" where the GUI writes "2,000 points" (no thousands comma). `tests/test_config.cpp` 670-685 pins the CLI words as "hydra_batch's header words", so this was written on purpose, but the same file's header comment contradicts it ("the one place they are spelled").
2. "Fills" (confirm row) against "1.0 fills" (panel, summary, gate) for one setting; the confirm's value is the rule name, the others name the box.
3. The SP cap is written four ways: "SP cap 4 bars" (path report subtitle, `counted`), "(SP cap 4, ...)" (empty page, `group_thousands`, no noun), "4 bars (Clone Hero's rule)" (confirm), "SP cap 4" with no unit (comparison footer and gate).
4. The score-range unit comes before the number in the CLI and after it everywhere else.
5. The fill rule is said when it is the odd one out in the report subtitle (1.1 goes unsaid) and always in the confirm and CLI.
Two code-only K1 pieces (same words, written twice): `score_range_value` (`library_dialogs.cpp` 64-67) and `within_label` (`path_view.cpp` 476-480) are the same mapping from (unit, number) to "4 scores" / "1 point" / "2,000 points"; and `counted(cap, "bar", "bars")` is typed in `sp_cap_value`, `describe_settings` and the path report subtitle. `fill_rule_name` already owns the fill words and is called everywhere (graph.h 68).
Owner proposed: the unit-and-number words come from one function per value in `app/` (extend `describe_settings` to produce them, or replace it), and the dialogs and `within_label` call it; whether the CLI keeps its own labels ("Depth", "Timing cap", "Fill rule", "none") is the user's call. The code-only part is making `score_range_value` and `within_label` one function and the cap noun one function; every printed word stays the same. Why: `describe_settings` already exists to own these words and has one caller; the dialogs' versions are the ones actually seen by players.
Evidence: by reading. A uitest with `Analysis settings` summary text and a `-tc` on `describe_settings` would prove no printed word moved in the code-only part.
Found by: G3-6, C1-3, also-seen G3-6 in G11b; verified by V2.
Scan row or reason: a row on `counted\([^,]+,\s*"(score|point|bar)"` outside the owner flags a re-typed noun; the CLI's `printf("Timing cap` and `Depth      :` labels are text a row on those two spellings in `cli/batch.cpp` would pin if the user keeps them.

### M16. Which of a record's key parts does the store check against the record?
Kind: K0. The key has six parts that vary (chart mode, cap, ms on/off, ms value, score-range mode, score-range value, fill rule) and `prepare_row` can only vouch for the ones the record states about itself. This is a missing stored fact, not a second copy of a rule.
Homes read: `prepare_row`, `store/record_store.cpp` 546-586 (checks cap, ms limit in both directions, fill rule); `HydraRecord`, `core/model.h` 812-831 (stores `ms_limit`, `sp_cap`, `legacy_fills`, `rules_fingerprint`; a grep of `model.h` for "depth" finds nothing); `analyze_chart`, `search/pather.cpp` 495-500 (stamps `rules_fingerprint` and `legacy_fills` by hand after the search).
Table: score-range mode and value, and the chart mode, are not checked, because the record does not carry them. The row is filed under whatever the key says for those parts. The chart mode could be checked only by a new field; the score range only widens which paths are kept, so a wrong band is not a wrong score.
Disagree on: n/a. Not a duplication. Worth recording so a later reader does not read the `prepare_row` header comment ("settings it doesn't belong to") as complete.
Found by: G3-7; verified by V2.

### M17. Can this chart mode be compared with the leaderboard? (toolbar passes the real difficulty, `collect_dm_rows` passes Expert)
Kind: K2 (same question, "may these settings be compared with the leaderboard", answered by two callers of one gate with different inputs; for a Hard, Medium or Easy mode the answers differ, and the result is a page a player can see). Fix: needs the user (a choice between behaviours: a Hard mode's Refresh either keeps building a Hard comparison or starts failing with "Needs Expert: ..."; this changes what a displayed window shows).
Homes: `why_not_comparable`, `app/dm_report.cpp` 33-49 (difficulty, cap, 1.0 fills, Note Shuffle);
       caller 1, the toolbar button, `ui/library_toolbar.cpp` 104-122 (`app.settings.difficulty()`, `sp_cap`, `legacy_fills`, `view_noteshuffle`: all four real);
       caller 2, `collect_dm_rows`, `dm_report.cpp` 61-65 (`Difficulty::Expert` as a literal, `kCloneHeroSpCap` as a literal, `lens.legacy_fills`, Note Shuffle read back from the chart mode text through `Settings{}.with_chartmode`, `mode && mode->view_noteshuffle`);
       the rebuild paths that reach caller 2 without touching caller 1: `AppState::request_dm_report` / `launch_dm_report`, `ui/app_state.cpp` 674-686 (current `settings.chartmode_key()` and `settings.lens()`), reached from `in.callbacks.refresh` and `in.callbacks.try_again` in `ui/dm_report_window.cpp` 142-143, and from `start_dm_report` (`app_state.cpp` 657-672) after "Compare another player..." (`dm_report_window.cpp` 151, `reopen_dm_picker`, picker `library_dialogs.cpp` 657).
Tables (inputs: the settings; answers: refused or allowed):

| Settings | Toolbar button (caller 1) | Refresh / Try again / Compare another (caller 2 via `collect_dm_rows`) |
|---|---|---|
| Expert, cap 4, 1.1 fills, no shuffle | allowed | builds |
| Hard (or Medium, Easy), cap 4, 1.1 fills, no shuffle | refused: "Needs Expert: the leaderboard only has Expert scores." | NOT refused: builds against the Hard records |
| Expert, cap 8 | refused: "Needs SP cap 4, Clone Hero's rule: ..." | not refused; builds from the cap-4 records on purpose (cap forced; `dm_report::settings_change_touches` ignores the cap) |
| Hard and cap 8 | refused with the Expert sentence (first rule that fails) | not refused |
| 1.0 fills on (any difficulty) | refused with the fills sentence (Expert first for non-Expert) | refused with the fills sentence; shown as a failed build ("Could not build the report." plus the sentence) |
| Note Shuffle on, Expert | refused with the shuffle sentence | refused with the same sentence |
| Chart mode text that `with_chartmode` cannot parse | n/a (the toolbar reads fields, not text) | passes the shuffle test silently (`mode` is empty so Note Shuffle reads as off); not reachable from the UI, only from a caller passing a hand-typed string |

The trace the notes asked for, by reading:
1. A comparison is built while Expert is set. The toolbar button was allowed.
2. With the comparison window open the user changes Difficulty to Hard in the Analysis settings panel. The settings lock covers only a running batch (`AppState::settings_lock`, `app_state.cpp` 447-449), so the change is allowed. `apply_settings` (853-872) sees `chartmode_key()` changed, and `dm_report::settings_change_touches` returns true (`dm_report.cpp` 240), so `mark_reports_out_of_date` sets the comparison's out-of-date mark.
3. The window shows its out-of-date strip, "The settings changed since this report was built.", with a Refresh button (`ui/report_window.cpp` 102-116). The header's Refresh (line 77) is there too. Both call `refresh` = `AppState::request_dm_report`. The strip is the expected next click.
4. `request_dm_report` returns early only when there is no store, no remembered player, or a job is running. It then calls `launch_dm_report`, which builds `DmReportJob` with `settings.chartmode_key()` = "Hard Pro Drums, 2x Bass" and the current lens. Nothing on this path calls `why_not_comparable` with the real difficulty.
5. `DmReportJob::run` (`ui/dm_jobs.cpp` 37-56) fetches the scores and calls `generate_dm_report` -> `collect_dm_rows`. There the gate gets `Difficulty::Expert`, so the difficulty rule cannot fire. The join uses the chart hash, which is the same for every difficulty of one chart file, so `records_by_hash(store, "Hard Pro Drums, 2x Bass", cap 4, lens)` finds each chart's Hard record where one exists.
6. What the page then shows: the comparison builds and lands as Ready. Charts the player analyzed under Hard get "under optimal / at optimal / above optimal" against Hard optimals, each row's Expert leaderboard score compared with a Hard path; the other listed charts read "not analyzed". Subtitle: "<player> - N scores" (no mode). The only place the Hard mode appears is the footer, "Actual scores from dmleaderboards.com against Hydra's optimal for Hard Pro Drums, 2x Bass." No warning, and the toolbar for the same settings says the comparison cannot be made.
Try again (shown only after a Cancelled or Failed build, `report_window.cpp` 198-214) reaches the same code the same way. "Compare another player..." (shown in Ready and Failed) reopens the picker without the toolbar's gate and picking a player starts the same build. So there are three entry points past the toolbar gate. A repeat of the toolbar test inside the window (disabling Refresh, Try again and Compare another with the gate's sentence) is the other way to close it.
Not found: any test that calls `collect_dm_rows` or `generate_dm_report` with a non-Expert chart mode. `tests/test_dm_report.cpp` pins `why_not_comparable(Difficulty::Hard, ...)` directly (lines 701-745) and uses `kMode` = "Expert Pro Drums, 2x Bass" (`tests/dm_fixture.h` 25) everywhere else; the only Hard case is the `settings_change_touches` test (806-835).
Disagree on: a non-Expert chart mode in caller 2. The cap difference is deliberate (cap forced, comment at `dm_report.cpp` 71-72); the difficulty difference reads as a hole: the comment at 55-60 says the difficulty "is folded into `chartmode`" and the toolbar asks for all four, but `collect_dm_rows` reads Note Shuffle back out of the mode text and does not read the difficulty back out (`mode->difficulty()` exists).
Owner proposed: `why_not_comparable` stays the owner. `collect_dm_rows` reads the difficulty from the parsed chart mode (`mode->difficulty()`) and, if the text does not parse, refuses rather than treating the missing mode as comparable. Then Refresh, Try again and Compare another all fail loudly with the toolbar's own sentence. Alternative for the user: leave `collect_dm_rows` as it is and grey the window's buttons, which keeps a stale Expert page visible but cannot rebuild it. Why the first: the gate would then have the same four inputs from every caller, and a UI path can no longer skip it.
Evidence: by reading, proven by a one-case test: `collect_dm_rows(store, {make_score(kHash, 1)}, "Hard Pro Drums, 2x Bass", store::Lens{})` should throw an AlreadyPlain error with the Expert sentence; today it returns a row (run with `-tc="collect_dm_rows*"` after adding the case). A uitest in `tests/ui/uitest_batch_reports.cpp` style (set Hard, click `Refresh##outofdate`, read `app.dm_report.result->chartmode`) would prove the window path; it needs the leaderboard fetch stubbed, which I did not check exists.
Found by: G3-2, G11b-3, also-seen G3-2 in G11b; verified by V2. (G11b-3's other half, "is this SP cap Clone Hero's rule", is M17's second line, see below.)
Scan row or reason: the scan row `Why can't these settings be compared with the leaderboard?` (`tests/single_owner/rules/app.json` 1004-1024) already flags re-typed gate sentences. A caller that passes a literal `Difficulty::Expert` or `kCloneHeroSpCap` to `why_not_comparable` outside the toolbar is the spelling to add as a row (`why_not_comparable\(\s*Difficulty::Expert`); it would flag the current `dm_report.cpp` 63 line, which is the owner file and so needs the row's owner-file exception removed for that one call.

Second half of M17, "is this SP cap Clone Hero's?": K1, agree today. Homes: `batch_settings_summary` (`library_dialogs.cpp` 93, `s.sp_cap == kCloneHeroSpCap` -> " (Clone Hero's rule)" / " (a what-if)"), `why_not_comparable` (`dm_report.cpp` 37, `sp_cap != kCloneHeroSpCap`), the panel's SP cap help (`settings_bar.cpp` 125-127, the words "is Clone Hero's rule. Any other cap is a what-if"). Table: cap 4 -> Clone Hero's rule on all three; any other cap (1, 5, 8) -> a what-if / refused / what-if; cap 0 is unreachable (the box clamps to 1, `config.cpp` 144). They would split only if a second cap counted as Clone Hero's. Fix code-only: a `bool is_clone_hero_cap(int)` beside `kCloneHeroSpCap` (`core/model.h` 36). Evidence: by reading. Scan row: a row on `== kCloneHeroSpCap` / `!= kCloneHeroSpCap` outside the owner.

### M18. Which search settings change a stored analysis (`settings_key` against `RecordKey`)?
Kind: K1, low weight (two lists of "which `SearchSettings` fields change the result"; they agree on every field except the rules, by design). One finding the scouts did not make: `settings_key` has no caller in `src/`. Fix: code-only.
Homes: `hydra::settings_key`, `search/pather.cpp` 65-75, declared `search/pather.h` 46-49 (sp_cap, depth mode, depth value, ms limit or "none", legacy fill flag, rules fingerprint);
       `store::Lens` / `RecordKey`, `store/record_store.h` 115-160 (cap, ms on/off and value, depth mode, depth value, fill rule, plus the chart mode; rules sit in a `rules_fp` column that makes a row Stale, `record_store.cpp` 583 and 346-383).
Callers of `settings_key`: `tests/corpus_util.h` 222 (the test cache), `tests/test_config.cpp` 647, `tests/test_engine_sweeps.cpp` 549-565. No file under `src/` calls it (grep of the whole repo).
Table (field changed -> key text changes / store key changes):

| Field | `settings_key` | `RecordKey` / store |
|---|---|---|
| sp_cap | yes | yes (`CapQuery`) |
| depth mode, depth value | yes | yes (`Lens`) |
| ms limit on/off, value | yes (number or "none") | yes (canonical 0 when off) |
| legacy fills | yes | yes |
| rules | yes (fingerprint) | no, a stamp: a different rules file makes the old row Stale, not a new row |
| difficulty, Pro Drums, 2x Bass, Note Shuffle | no (not in `SearchSettings`; `corpus_util.h` 221-222 adds prodrums, bass2x and difficulty beside it and leaves Note Shuffle out) | yes (chart mode text) |

Disagree on: rules (on purpose) and the parse choices (outside `SearchSettings`). Because `settings_key` is production-dead, its job is a test cache, and the only guard that a new `SearchSettings` field is added is the hand-written line in `test_engine_sweeps.cpp` 554-560.
Owner proposed: move `settings_key` out of `src/search/` into `tests/corpus_util.h` (its only user), so no source comment promises a maintenance duty on a test helper. `test_config.cpp` 647 can compare the lens instead (the lines above it already do). If the user prefers to keep it, derive `Settings::lens()` and the key from one `SearchSettings`-to-`Lens` conversion. Why: the store's key is the one the app uses; the other is test support.
Evidence: by reading (the zero-caller claim is a repo-wide grep of `settings_key`).
Found by: G4b-4; verified by V2.
Scan row or reason: no row needed after the move; a row on `settings_key` in `src/` would flag it coming back.

### M19. Which settings make a Preview "another song", which only a new overlay, and which reading of a chart is asked for?
Kind: K1 (one rule, "which four parse choices pick the notes", spelled as four loose values or a struct in several places; they agree today). Fix: code-only. The rules question is a dormant gap, not a copy.
Homes: `Settings::chartmode_key` + `to_analysis_settings` + `with_chartmode`, `app/config.cpp` 266-279, 315-328, 297-313 (difficulty, Pro Drums, 2x Bass through `effective_bass2x()`, Note Shuffle);
       `AnalysisSettings` fields, `app/analysis.h` 158-168;
       `PreviewSongKey`, `ui/preview_controller.h` 56-68, built at `preview_controller.cpp` 84 from `open(entry, pro, bass2x, difficulty, ..., rules, noteshuffle)` (`.h` 94-97), fed by `preview_tab.cpp` 356-358 (`view_prodrums`, `effective_bass2x()`, `difficulty()`, `view_noteshuffle`, `sp_cap`, `rules`);
       the positional list `(pro, bass2x, difficulty, noteshuffle)` carried by `PreviewLoadJob`, `PreviewNotesJob` (`preview_controller.cpp` 112 and 146-148), `resolve_preview_song`, `dynamics_for_settings` (`app/dynamics_breakdown.cpp` 143-160), `PathsTabCache` (`path_view.cpp` 562-599, reads Pro Drums only);
       the overlay comparison, `preview_controller.cpp` 85-95 (`path_key` + `sp_cap`, composed by `overlay_key`, 29-31).
Table (one input changes, same chart, already open): difficulty / Pro Drums / 2x Bass / Note Shuffle -> `PreviewSongKey` differs; once the first load is done only the notes reload (`open`, 98-102), audio and playhead stay; before the first load finishes, a full reload. Path changes -> overlay job only. SP cap changes -> overlay job only (cap is in the overlay key). Ms limit, score range, 1.0 fills -> not in the song key; they change the viewed record, which changes the selected path and `record_generation`, which rebuilds `overlay_key` (`preview_tab.cpp` 350-355) and so the overlay. `chartmode_key` / `RecordKey`: the same four fields change the key; cap changes `cap_query`; the three lens fields change `Lens`. md5 differs -> full close and reload.
The rules edge: `open()` stores `rules_ = rules` first (line 83) and returns early at 87 when the song key, cap and path key are unchanged, so a rules-only change builds nothing. Dormant: `settings.rules` is assigned once at start-up (`ui/app_state.cpp` 26) and no UI path writes it again, so the case cannot occur today. It would matter if rules ever became editable at run time, since the running score in the scene is priced from the rules.
Also (from G11a): `launch_view_job`, `app_state.cpp` 303-308, converts the same `Settings` twice, `record_key(md5)` and `to_analysis_settings()`, for one job. Same rule in two conversions; they cannot split because both read the same fields.
Disagree on: none today. A fifth parse choice would need to be added to `chartmode_key`, `to_analysis_settings`, `with_chartmode`, `PreviewSongKey`, `open()` and every job constructor by hand.
Owner proposed: one value for the four parse choices (`AnalysisSettings` already holds them; a `Settings::reading()` that returns just those four), used by `chartmode_key`, `with_chartmode`, `PreviewSongKey` and the job constructors. The overlay-versus-song split stays in `PreviewController` (it is a fact about the Preview). If rules ever become editable, the rules go into the song key. Why: one place to add a fifth choice.
Evidence: by reading. A rules-only call to `PreviewController::open` would need a GPU device; a unit test of the key comparison cannot run without one, so I name no run.
Found by: G6a-3, C2-5, also-seen G6a-3 in G6b and G11a; verified by V2.
Scan row or reason: a row on `PreviewSongKey\{` outside `preview_controller.cpp` and a row on a fourth `view_prodrums, ... effective_bass2x\(\)` argument tail outside `preview_tab.cpp` are weak. The real guard is the shared value, since a positional bool list has no spelling a search can match.

### M20. Which columns make one result row's identity, and where is each spelled?
Kind: K1 inside the store (every site reads the same columns today). Fix: code-only.
Homes: `kResultsColumnDefs` and the `UNIQUE (...)` line, `store/record_store.cpp` 215-239 (hyhash, chartmode, sp_cap, ms_enabled, ms_value, depth_mode, depth_value, legacy_fills, rules_fp);
       `lens_match` + `bind_lens` (316-330) and `cap_match` + `bind_cap` (331-339), used by `get_summaries`, `list_records` and `analyzed_hashes` through `append_candidate_filter` / `bind_candidate_filter`;
       `write_row`'s replace purge, 1207-1215: spells `hyhash=? AND chartmode=? AND sp_cap=?` by hand, takes `lens_match("")`, binds the cap with `sqlite3_bind_int(s, 3, row.sp_cap)` (not `cap_match` / `bind_cap`, because it has an int from the row rather than a `CapQuery`), and adds `rules_fp = ?`;
       `write_row`'s insert list `kRowColumns`, 1220-1223;
       `Lens::operator==` and `Lens::from`, `record_store.h` 128-145;
       `chartmode=?` typed separately at lines 1195, 1207, 1338, 1399 and 1457.
Table: no input splits them. Edges: ms limit off (canonical 0 in the lens, so `ms_enabled=0 AND ms_value=0` matches); a row with `ms_enabled = -1` (an old migration) matches no lens, by design (314); a row under other rules sits beside the current one and is found only through the rules stamp ranking.
Disagree on: none today. The hand-spelled cap in the replace purge is a second spelling of "sp_cap equals" beside `cap_match`. A sixth lens field means touching the struct, `==`, `from`, `lens_match`, `bind_lens`, the column defs / UNIQUE and the insert list; missing one still compiles. `kSummaryColumnCount` has a `static_assert`; the key columns have none.
Owner proposed: the replace purge uses `cap_match("")` and `bind_cap` (take a `CapQuery::at(row.sp_cap)`); one list of identity column names generates the UNIQUE line, the insert list and `lens_match`. The fix is local to one file. Why: the store is the one owner of "which row ran under these settings", and it already has helpers for three of the five spellings.
Evidence: by reading. The store tests (`-sf=test_record_store.cpp`) prove no behaviour moved; the migration upgrade tests cover the UNIQUE line.
Found by: G10-4; verified by V2.
Scan row or reason: a row on `sp_cap=\?` and `chartmode\s*=\s*\?` outside the store's lookup helpers would flag a new hand-spelled key condition; it would flag the five existing `chartmode=?` sites unless the row allows them.

### Side notes

- `tests/corpus_util.h` 221-227: the test cache key and `song(...)` leave Note Shuffle out, so `analyzed(path, settings)` with `noteshuffle` on returns the same cached record and parse as with it off. Harmless while no test sets Note Shuffle through this helper; wrong the day one does.
- `src/app/dm_report.cpp` 61-64: a chart mode text that `Settings{}.with_chartmode` cannot parse passes the Note Shuffle test as "off" (the `mode &&` guard). Not reachable from the UI (the text always comes from `chartmode_key`), but the gate fails open rather than loud (rule 4).
- `src/app/dm_report.h` 77-82 and 89-91: the header says "The library toolbar's button and collect_dm_rows both ask it" and that `collect_dm_rows` throws "when `lens` breaks a Clone Hero rule". Both are true only for the fill rule and Note Shuffle; the comment on `collect_dm_rows` at `dm_report.cpp` 55-60 should say it does not ask about difficulty (M17).
- `src/ui/app_state.h` 623-635: `commit_settings` is called "the single place that knows which settings change a record's identity". Not true of the reports (their own functions, M11) or the Preview and Paths tab (their own keys, M19); the sentence is a rule restated in a comment.
- `src/store/record_store.cpp` 1383-1386: `RecordStore::has_record` has no caller in `src/`, only tests (M12).

---

### Findings V3: a chart's identity, the library and the store

Verifier id: V3 (the progress hook never named another id). Task: concept-audit-V3. Candidates M21 to M29. Everything below is by reading the code in the main checkout; nothing was built or run. Line numbers are from the code as it stands now.

Summary: M21 K1 code-only. M22 K3 (one latent rule). M23 K2 (needs the user). M24 K2 (needs the user). M25 K1 code-only. M26 K1 code-only. M27 known (derivation-audit finding 54), partly fixed. M28 K1 code-only. M29 K2 (needs the user).

### M21. What does one scanned chart hold, and how many places copy its eight fields by hand?
Kind: K1 (same eight fields, same meaning, hand-copied; the one real rule inside, "which stored signature can show a chart unchanged", already has one owner, `sig_can_show_unchanged`). Fix: code-only. None of the proposed merges reaches the database.
Homes, the shapes: `ScanItem`, src/app/analysis.h 36-48 (md5, title, artist, charter, notespath, rootfolder, sig, optional timing); `ChartLibraryEntry`, src/store/record_store.h 262-274 (the same eight, same order); `ChartCacheEntry`, record_store.h 278-285 (sig, md5, title, artist, charter, non-optional timing; the path is the map key and the folder is absent); `ChartMeta`, analysis.cpp 138-143 (title, artist, charter, timing; file-only, never stored); `LibraryChart`, src/ui/library_model.h 46-50 (md5, notespath, rootfolder; a documented subset).
Homes, the copy sites. I counted eight, the same as the scouts, but the eighth is not an eight-field copy:
1. `discover_charts`, cache-hit block, analysis.cpp 674-683 (cache entry to ScanItem by name; notespath, rootfolder, sig come from the fresh walk).
2. `discover_charts`, fresh-read block, analysis.cpp 689-701 (SongIdRead and ChartMeta to ScanItem by name).
3. `to_library_entry`, analysis.cpp 421-432 (ScanItem to entry, all eight; named).
4. `scan_item_of`, src/ui/library_jobs.cpp 280-290 (entry to ScanItem, seven fields, leaves `sig` empty; named).
5. `rebuild_chart_library`, record_store.cpp 1529-1538 (entry to nine SQL columns, timing split in two).
6. `chart_library_cache`, record_store.cpp 1568-1570 (SQL to ChartCacheEntry, by position in a brace list: five strings in a row. This is exactly the risk the comment at analysis.cpp 672-673 names and sites 1 to 4 avoid).
7. `read_library_entries`, record_store.cpp 1601-1608 (SQL to entry, by name).
8. `LibraryModel::set_charts`, library_model.cpp 135-146 (copies only md5, notespath, rootfolder; title, artist, charter go through `display_*` and `make_searchable`; sig and timing are dropped on purpose; named).
Named converters: sites 3, 4, 8. Unnamed hand copies: 1, 2, 5, 6, 7. Also `AppState::update_view_job` patches two fields (md5, sig) of `selected` at app_state.cpp 396-399 (that is M29). Outside src/: tools/bench.cpp 150-178 only calls these; it builds no entry itself.
The SQL column list for the `charts` table is spelled in four more places: CREATE TABLE (record_store.cpp 684-692 and the ALTER at 707), the INSERT (1525-1526), the cache SELECT (1563-1564, own column order: path, sig, md5, name, artist, charter, length, delay) and `kLibraryEntrySelect` (1591-1592, another order). A new stored field must be added in all of them; leaving one out compiles.
Tables (input to answer):
| Input | Cache read (`chart_library_cache`) | Entry read (`read_library_entries`) |
|---|---|---|
| stamp not current (`chart_meta_current` false) | empty cache | every row listed, timing unset |
| row with empty sig | row left out | row listed, sig "" |
| row with a sig | keyed by path, timing set (members may be NULL = no value) | listed, all eight, timing set |
| no rows | empty | empty |
| ScanItem via `scan_item_of` vs from a scan | sig "" vs the file's fingerprint | n/a |
Disagree on: `sig` in one direction only (`scan_item_of` blanks it), the empty-sig row, and the stale-stamp case. The last two are deliberate and owned once. The `sig` blank is invisible today: the batch reads md5, notespath and titles from a ScanItem, and only `save_scan_as_library` writes sig, and it is only ever fed items from `discover_charts`. A batch's items passed to `save_scan_as_library` would blank every stored sig and make every chart look changed on the next rescan; no code does that.
Owner proposed: `ChartLibraryEntry` stays the one shape. `ScanItem` becomes an alias of it (finding 256's second option), or holds a `ChartMeta` member; `scan_item_of` and `to_library_entry` then disappear. `chart_library_cache` is built on `read_library_entries` (stamp check, then skip rows where `sig_can_show_unchanged` is false, key by notespath) so the positional brace copy goes. One column-list constant feeds INSERT and both SELECTs. `set_charts` stays: it makes a different thing (display forms), which is a per-module fact.
Does a merge change a stored record? No. None of these touch the `charts` table's columns, their order, the stamp, or what is written. They change only in-memory types and the SQL text that reads them (same rows, same values).
Evidence: by reading. A run that would prove the cache merge: a one-chart `hydra_batch` twice on a chart in `testdata/input` with a `--db`, comparing the "reused" count on the second scan, plus the existing rescan-cache test case before and after.
Found by: G2-1, G2-2, G2-3, G10-2, C2-1; verified by V3.
Scan row or reason: a row on `\.(rootfolder|notespath)\s*=[^=]` allowed only in the files that own a shape (analysis.cpp, record_store.cpp, library_model.cpp) would flag a new hand copy. A row on ` FROM charts` and `INTO charts` outside record_store.cpp would flag a stray SQL reader. It cannot flag a new field added to one copy and forgotten in another; that needs the single constant above.

### M22. Why is a result Stale: do `LibraryRow` and `SummaryLookup` copy it, and is the "did it change" test a second rule?
Kind: K3. Every entry is a per-type fact; the only rule is "Ready means this build's results version and this process's rules", and `Candidate::ready()` (record_store.cpp 370) owns it. One latent rule is spelled in four places (see Disagree on).
Fix: code-only.
Homes: `Candidate`, record_store.cpp 359-371 (current, same_rules, plus the negated pair stale_build, stale_rules set in `rank_row` 381-382); `SummaryLookup`, record_store.h 210-221 (status, stale_build, stale_rules, bestpath, summary); `LibraryRow`, library_model.h 60-71 (the same five); `apply_summary`, library_model.cpp 51-63; `save_view_summary`, app_state.cpp 426-437.
Tables:
| Input | store `get_summaries` | `apply_summary` ("changed?") | `save_view_summary` ("already stored?") |
|---|---|---|---|
| winner built by another build | Stale, stale_build true, stale_rules per rules | changed when status or either flag differs | Ready test fails, so it writes |
| winner under other rules | Stale, stale_rules true | same | same |
| winner ready, only `actcount` (or maxskip, avgmult, notecount, sqin_count, sqout_count, pathcount) differs | Ready, new numbers | reported unchanged; row keeps the old numbers | compares all ten via `PathSummary::operator==`, so writes |
| winner ready, score/stars/hardest_ms differ | Ready | changed, row updated | writes |
| no row | NotAnalyzed, empty | status equal, so unchanged | not ready, writes |
Nothing computes Stale a second time; every caller tests `status` only (library_model.cpp 14-32, 39; library_table.cpp 437 passes the two flags to `stale_text`).
Disagree on: none today. The library reads only `score` (label, sort, `has_scored_best_path`), `stars` and `hardest_ms` (`facts_of`, library_model.cpp 38-42). `apply_summary` compares exactly those three. Which summary fields the library shows is spelled in `best_path_label`, `facts_of`, the sort (library_model.cpp 244-252) and `apply_summary`. If the library starts to show a fourth field, `apply_summary` would silently keep the old value. The two "did it change" tests answer different questions (what the table shows, versus what is stored), so they are not duplicates.
Owner proposed: keep the types. Put the "fields the library reads" test next to `facts_of` as one function `LibraryRow::shows_same_as(lookup)` so a new shown field is added in one place. Optional: drop `Candidate::stale_build` and `stale_rules` (derivable as `!current`, `!same_rules`); they are set in one place and only read at record_store.cpp 1372-1373.
Evidence: by reading.
Found by: G1b-9, G10-6; verified by V3.
Scan row or reason: no text search catches a new field read from `row.summary`; the spelling varies. A row on `\.summary\.(actcount|maxskip|avgmult|notecount|sqin_count|sqout_count|pathcount)` under src/ui/ would flag the day the library starts to read a field `apply_summary` ignores.

### M23. Which metadata key means which field for .ini, .sng and .srb, and how are values trimmed?
Kind: K2 on one input (whitespace around a value); K1 on the key-to-field map. The key map is the same idea written three times; the trimming rule is not the same.
Fix: needs the user (stored record, and a displayed word for the charter).
Homes: `read_metadata_ini`, analysis.cpp 151-164 on `read_song_ini_keys` 345-385; `parse_sng_metadata`, analysis.cpp 173-185 on `sng_read_metadata` (src/parse/sng.cpp 77-99); `srb_chart_meta`, analysis.cpp 194-204 on `srb_parse_metadata` (src/parse/srb.cpp 134-184). The delay key is read by `ini_delay_ms` (preview_source.cpp 300-304) and `sng_metadata_delay_ms` (181-186), both through `parse_delay_ms` (112-114), which only forwards to `parse_finite_number`.
Tables:
| Input | .ini | .sng | .srb |
|---|---|---|---|
| key name | lower-cased, trailing space/tab trimmed; only inside `[song]` | lower-cased per pair, not trimmed | fixed field order, no keys |
| `name`, `artist`, `charter`, `song_length`, `delay` | all five | all five | name, artist, charter, length; no delay |
| `frets` alias for charter | not read | not read | n/a |
| key twice | last wins (map assign) | last wins (loop overwrite); for delay the last wins even if it does not parse (gives no value), same as .ini | n/a |
| value `"  A  "` | `"A"` (line trimmed, value left-trimmed) | `"  A  "` kept | `"  A  "` kept |
| value `"A # B"` | `"A # B"` (no inline `#` cut) | same | same |
| empty name | title empty | title empty | title left empty (only set when non-empty); artist and charter assigned empty |
| all three then pass through `title_or_unknown`, `artist_or_unknown`, `charter_or_unknown` (analysis.cpp 728-730), which test for an empty string only |
Disagree on: a value with spaces around it, from a .sng or .srb. Example: charter `" "` in a .sng is stored as `" "` and shows as empty text (`display_charter` trims, song.cpp 531, 544); the same blank charter in a song.ini is stored as `"<unknown charter>"` and shows that text. A padded title or artist differs only in the stored text, not on screen, because `display_title` trims. Whether such files exist in the user's library, I could not tell by reading. The song.ini `#` and trim rules mirror the game's reader on purpose (G9-8's difference from `split_ini_line` is a per-format fact), so that part is K3.
Owner proposed: one trim of every name field in `discover_charts`' existing fallback loop (analysis.cpp 719-731), before `*_or_unknown`, with the song.ini reader keeping its own line rules. That changes stored titles for any padded value, so the chart-metadata stamp (`kChartMetaStamp`) would need a bump to re-read charts.
Evidence: by reading, proven by one .sng in `testdata/input` (or a made copy with a padded charter) through `hydra_batch` on a throwaway `--db`, then reading the charts table.
Found by: G2-4, G9-8, also seen in G7b; verified by V3.
Scan row or reason: a row on `to_lower_ascii(` followed by `"name"` or `== "artist"` outside analysis.cpp would flag a fourth key reader. It cannot flag a missing trim; no spelling stands for "this value was trimmed".

### M24. What counts does a batch report, and who words them?
Kind: K2 on wording, K1 on the counts (one writer, `run_batch`, analysis.cpp 880-913; BatchProgress is copied, never recounted).
Fix: needs the user (display text).
Homes: `BatchProgress`, analysis.h 217-224; `BatchJob::Snapshot`, library_jobs.h 271-298, filled by one lambda at library_jobs.cpp 422-429 (total, completed, analyzed, skipped, failed); GUI wording `batch_counts`, library_dialogs.cpp 54-58, plus lines 443, 481-482, 532-536; CLI wording, src/cli/batch.cpp 240, 250, 260-263.
Exact words:
| Surface | Text |
|---|---|
| GUI strip, running (443, 458) | `Analyzing...  <completed> of <total>` and, below, `<n> analyzed · <n> failed · <n> skipped (already had a result)` |
| GUI done strip (532) | `Finished: <the same counts> · took <m:ss>` or `Stopped: ...` |
| GUI Stop tooltip (481) | `Keeps the <n> charts already finished.` (uses `analyzed`, one chart: `1 chart`) |
| GUI confirm (348-354) | `Every chart here already has a result.`; `Analyze <n> charts that have no result yet?` |
| CLI per chart (250) | `[<completed>/<total>] <score>  <artist - title> <path>` |
| CLI failure (240) | `[<completed>/<total>] FAILED <title>: <error>` |
| CLI end (260-263) | `Analyzed <n>, skipped <n> already stored, <n> failed in <s>s.` |
Counts: both show the same three numbers in library rows. They agree for every run because both read `BatchProgress` as `run_batch` wrote it (GUI copies five of six fields; CLI keeps the whole last struct). Edge: a stopped batch has partial counts on the GUI; the CLI has no stop. A plan with nothing to run: GUI asks nothing and says "Every chart here already has a result."; CLI prints `Analyzed 0, skipped <all> already stored, 0 failed ...`.
Disagree on: the order (GUI analyzed, failed, skipped; CLI analyzed, skipped, failed) and the gloss for skipped ("already had a result" against "already stored"). Grouping of thousands is the same (`counted` and `group_thousands` both group). The word "finished" means analyzed charts in the Stop tooltip and analyzed plus failed in the "N of M" progress (`completed`, analysis.h 218).
Owner proposed: one function in app/ (beside `BatchProgress`) that returns the three counts as words; the GUI and CLI both print its text. Which wording wins is the user's call: the GUI order and "already had a result" read as plain English; the CLI line is the shorter one. Not for an agent to pick.
Evidence: by reading. Proven by a one-chart `hydra_batch` run on `testdata/input` and a `hydra_uitest` batch script reading the done strip.
Found by: G2-5, also seen in G11a and G11b; verified by V3.
Scan row or reason: a row on `already stored` or `already had a result` in src/ would flag a third spelling of the skipped gloss. A rewritten count line in new words would slip through.

### M25. What does a command-line tool do to start, and does the app do the same?
Kind: K1 among the tools; K0 against the app (the app keeps going with a bad rules file on purpose: app_state.cpp 21-31, 40-46 opens the store with `RulesStamp::none()`; the tools exit 2).
Fix: code-only for the shared helper if it prints what both tools print today.
Homes: the "load the rules file or print the error and return 2" frame, four times: src/cli/batch.cpp 109-116, src/cli/fillcompare.cpp 70-77, tools/bench.cpp 514-521, tools/replay.cpp 774-781. All print `e.what()` to stderr and return 2; the path is `--rules` if given, else `default_rules_path()`. The unknown-option line: `Unknown option: %s` in batch.cpp 101-103 and fillcompare.cpp 57-60 (same text), but `unknown option %s` plus usage in tools/replay.cpp 767. The open-the-store frame (`open_store`, then `tool_error(e, 2)`): batch.cpp 137-143 and fillcompare.cpp 82-89. `open_store` is a `make_unique` (config.cpp 46-49).
Tables:
| Input | batch | fillcompare | bench / replay | app |
|---|---|---|---|---|
| bad rules file | message, exit 2 | message, exit 2 | message, exit 2 | keeps default rules, analysis off |
| `--rules path` | honoured | honoured | honoured | no such option |
| unknown `--x` | `Unknown option: --x`, exit 2 | same text | replay: lower-case `unknown option`, usage | n/a |
| store will not open | `tool_error(e, 2)` | same | n/a | startup box |
Disagree on: replay's lower-case unknown-option text (a different tool, and its wording is user-visible). The settings load also differs on purpose: batch uses `Settings::load_for_command_line(legacy_fills)`, fillcompare `Settings::load()`.
Owner proposed: `app::load_tool_rules(optional<string>)` returning the rules or printing the line and an exit code, called by all four. Replay's lower-case text is left alone unless the user wants it changed.
Evidence: by reading.
Found by: G2-7; verified by V3.
Scan row or reason: a row on `catch (const (hydra::)?app::RulesFileError&` in src/cli and tools/ allowed only in the helper's file would flag a fifth tool.

### M26. "Scan the folders against the last scan's cache, then save as the library": is it written in more than one place?
Kind: K1 (a four-line frame), with two per-caller facts: batch saves only when no folder arguments were given, and ScanJob does not save when cancelled.
Fix: code-only.
Homes: `ScanJob::run`, src/ui/library_jobs.cpp 180-227 (cache read in a try, `cache.empty() ? nullptr : &cache` at 205, save at 227); `batch_main`, src/cli/batch.cpp 203-215 (same try, same ternary at 210, save only when `folder_args.empty()`); `scan_mode`, tools/bench.cpp 149-182 (cache read with no try, ternary at 163, save at 178); `engine_mode`, tools/bench.cpp 260-267 (cache read, ternary 267, no save). The scout counted two; there are four.
Tables (input to answer):
| Input | ScanJob | batch | bench scan_mode |
|---|---|---|---|
| cache read throws | empty cache, slower scan | same | whole tool fails (no try) |
| cache empty | pass null | pass null | pass null |
| cancelled | no save | n/a | n/a |
| folder arguments | n/a | no save | one folder, saves if `--db` |
| save fails | problem added to progress errors | `  ! <problem>` printed | `  ! <problem>` printed |
`discover_charts` reads the cache pointer only at analysis.cpp 669-671 (`cache->find`); a miss and a null both mean "read the chart", so the ternary is a habit and not a rule (the other overload passes null, 736-742).
Disagree on: bench's missing try (a per-tool choice). Nothing else.
Owner proposed: `discover_charts` takes `const ChartLibraryCache&` (empty means no help); `app::load_scan_cache(store)` owns the swallowed failure. Save gate stays with each caller.
Evidence: by reading. The existing test case "rescan cache reproduces the scan without reading chart files" (tests/test_analysis.cpp) would pin it.
Found by: G10-1; verified by V3.
Scan row or reason: a row on `cache.empty() ? nullptr` in src/ and tools/ outside the helper would flag a new copy.

### M27. The fill rule a result ran under is stored twice (known, finding 54, partly fixed)
Kind: K2-by-design on the stored pair; the reader duplicates are mostly fixed. Fix: needs the user for the pair (a stored record); code-only for the remaining reader.
Still open in the code: each result row's `legacy_fills` (the lens key) and the file's `engine_mode` meta row (record_store.cpp 1095-1118) are both kept. The app's "1.0 fills" setting writes 1.0 rows into hydra.db and never stamps; `stamped_fill_rule` then answers 1.1 (rows, no stamp, line 1116). Readers: `stamped_fill_rule` in `upgrade_results_key` (line 744, now the store's); batch.cpp 150-169 (reads both the raw stamp and the rule, and refuses an unknown stamp); fillcompare.cpp 91-103 (reads the raw stamp itself; no stamp stays silent, an unreadable stamp warns, on purpose per its comment). `generate_fill_report` (fill_report.cpp 176-178) ignores the stamp and asks the rows for 1.0 and 1.1, so the report is right whatever the stamp says.
Fixed since the audit: the store owns stamp-to-rule (`stamped_fill_rule`) and the schema-3 migration no longer spells "ch10".
Found by: G10-7; verified by V3 (short block, known).
Scan row or reason: `"engine_mode"` outside record_store.cpp, batch.cpp and fillcompare.cpp.

### M28. "Begin, do the work, commit, roll back if still open" written five times
Kind: K1, line-level. Fix: code-only (no stored record, same statements in the same order).
Homes, all in record_store.cpp: `upgrade_results_key` 752-765; `add_row` 1160-1167; `reidentify_chart` 1255-1270; `delete_auto_results` 1290-1300; `rebuild_chart_library` 1521-1550. Variants: `save_analysis` 1124-1146 (`ctl("BEGIN")` or SAVEPOINT, groups); `begin_save_group` and `commit_save_group` 1002-1032 (`ctl`); `copy_upgrade` step 4, 857-861 (BEGIN and COMMIT, rolled back by its outer `tidy_up` at 812-821).
Table: failure inside the work -> `rollback_if_open(db_)` (485-487) then rethrow, in all five; SQLite already rolled back by itself -> `rollback_if_open` skips the second ROLLBACK, in all five because they share the helper. Differ only in the opening call: `exec("BEGIN")` in the five and in `copy_upgrade`; `ctl("BEGIN")` in the save paths, which compile transaction control through the statement cache (995-1000). `exec` throws "sqlite exec failed: <msg>"; `ctl` words its failure the same.
Disagree on: none today.
Owner proposed: `template<class F> void RecordStore::in_transaction(F work)` doing BEGIN, work, COMMIT, rollback-and-rethrow; the five callers pass their bodies. The save paths stay (savepoints and groups are a different shape).
Evidence: by reading.
Found by: G10-8; verified by V3.
Scan row or reason: a row on `exec("BEGIN")` in src/store/ outside the helper would flag a sixth frame.

### M29. The chart open in the song panel: can a rescan leave its copy stale, and what would the user see?
Kind: K2 (two answers to "what does the library say about the open chart": the panel's by-value copy and the library rows/store). Fix: needs the user (display text, and a choice between behaviours: the panel follows the rescan, or keeps what was clicked).
Homes: `AppState::selected`, src/ui/app_state.h 321 (a `ChartLibraryEntry` copied whole at click, app_state.cpp 216-223); the library rows (`LibraryModel::set_charts`, rebuilt on each reload); `ViewJob::song_` (library_jobs.cpp 473-480, a second copy made from `selected` at app_state.cpp 304); the Preview's entry copy (`PreviewController::open(*app.selected ...)`, preview_tab.cpp 356).
What the code does: a scan can start with the panel open. `can_scan` (app_state.cpp 458-460) checks only folders, an unfinished scan and a running batch. The panel's own "Rescan library" button closes the panel first (details_panel.cpp 137-140); the toolbar button and the Song folders' "Scan now" do not. After the scan, `tick_library` calls `reload_after_scan` (app_state.cpp 159-164, 166-167), which runs `reload_library()` and nothing else: `selected` is never touched (it is replaced only by `select`, line 220, and patched for md5 and sig at 396-399 after a click finds edited files).
Table, after a rescan that read changed files:
| Field | Library table row | `selected` (panel) |
|---|---|---|
| title, artist, charter | new text (`set_charts`) | old text; header and byline read `selected` (details_panel.cpp 68-77) until the next click or previous/next |
| md5, sig | new | old; healed on the next analysis: `ViewJob::run` sees the file differs from the old sig (library_jobs.cpp 504-513), `reidentify_chart` is then a no-op update, `selected` is patched (app_state.cpp 395-399) |
| timing (stated length, delay) | new | old; `ViewJob::run` hands `song_.timing` to `analysis_song_length` (line 523), so a re-analysis under changed settings shows the old song length |
| rootfolder | new | old |
| chart gone from the library, file still there | row gone | panel stays; a re-analysis would save a result under an md5 no library row lists, and the next scan's purge deletes it |
So yes, a rescan can leave the panel's copy stale. The user sees the old title and charter in the panel header while the table row beside it shows the new ones, and, if a song.ini length changed, a song length that follows the old file until they click the row again. The numbers on the paths do not go stale: the analysis re-reads the chart file.
Disagree on: any field a rescan changes other than md5 and sig.
Owner proposed: the library/store row is the owner. After `reload_after_scan` (and after `reidentify_chart`) re-read the open chart's entry by `row_key` with `list_chart_library_copies` (the same lookup `select` does, app_state.cpp 242-249) instead of patching two fields. The panel header would then change under the user's eyes after a rescan, which is why this is the user's call.
Evidence: by reading, proven by a `hydra_uitest` script: select a chart, rewrite its song.ini `name`, run the scan, then read the panel header text and the table row title.
Found by: G11a-9; verified by V3.
Scan row or reason: a row on `selected->[a-z_]+ =` outside `select` would flag a new hand patch. A forgotten refresh cannot be found by text.

### Side notes
- `BatchProgress::current_title` (analysis.h 223, written at analysis.cpp 912) has no reader in src/, tools/ or tests/. `Snapshot::current_title` is a different, live field with the same name. Dead field.
- `AppState::save_view_summary` tests `!found.empty()` (app_state.cpp 431); `get_summaries` returns one slot per hash asked (record_store.cpp 1310), so it is always true. `RecordStore::has_record` (record_store.cpp 1383) has no caller in src/ or tools/.
- `list_chart_library(offset, limit)` is called with `(0, -1)` at library_jobs.cpp 53 and app_state.cpp 145, and `(0, INT_MAX)` at tools/bench.cpp 400; the limit and offset and the ORDER BY name look unused in effect (`LibraryModel` re-sorts with a notespath tie-break).
- `tick_library` (app_state.cpp 174) watches `snap.completed` (analyzed plus failed) to decide "the batch stored something"; a failed chart triggers a pointless summary re-read. Harmless.
- The `charts` table has no UNIQUE on `path`; the UI and the store both use `notespath` as the row identity (row_key, `reidentify_chart`'s `WHERE path = ?`, the cache key). Not checked whether two overlapping roots can list one path twice.

---

## V4: scoring and the search engine (M30 to M38)

Verifier V4, session ec019463, task concept-audit-V4. Status lines went to `V4.md` in the status folder, because the progress hook never named another id. Everything below is by reading. I ran no build and no test.

Short version. Of nine candidates, one is a real K1 with a code-only fix that is worth doing now (M35), two are small K1s with a code-only fix (the engine's second `after_sp_end` call inside M33, and the "4" in the fill sentence, M38). One is a K1 that should stay as it is, because a runtime check already guards it (M36). One is a K1 about the engine's running total inside a K3 list (M30). M31, M32, M34 and M37 are not duplication. M33's main question (merge the tick test and the ms test) gets a firm "do not merge": the two tests give different answers on one kind of input.

### M30. Which six score categories does a path have, and how many places spell them out?
Kind: K3 for the list of six names (every home is a per-module fact: a stored result, a replay total, a graph accumulator, a flat engine record). One rule hides inside it and is a K1 on its own: "a path's running score is the sum of its six categories", written by hand in the engine next to the owner `score_total`. Fix: code-only for the running-total rule; nothing to do for the list.
Homes (all in src/):
- `Path::score_base .. score_ghosts`, src/core/model.h 745-750; `Path::totalscore` (model.cpp 585-588) and `Path::prepare_variants` (model.cpp 664-669) read the six.
- `ReplayScore` with `total`, `add`, `operator==`, src/core/replay.h 105-126; `kReplayScoreFields` (replay.h 135-139); `score_of` and `assign_score` (replay.cpp 214-232); the six `row.points.*` lines (replay.cpp 180-185).
- `CategoryScores` (five of the six; solo is a separate function, `solo_bonus`) and `ChordScoreRow` (all six), src/core/scoring.h and scoring.cpp 108-117.
- `ScoreGraphEdge::basescore .. ghostscore` (src/search/graph.h 144-149) with six `store_*score` methods (graph.cpp 235-257) and their callers (graph.cpp 135-144).
- `EdgeView` (engine.cpp 66), `enumerate` (engine.cpp 165-170), engine `Path::sc[6]` (engine.cpp 288) read and written by slot number at 820-825, 962, 1075, 1715, 1722-1727, `OutPath` (engine.cpp 313-315), and the copy into the core `Path` (engine.cpp 1909-1914).
- `build_score_breakdown`, src/app/path_view.cpp 429-434 (the six labels).
- Not a home: the store. `src/store` keeps only `score` (the total, record_store.cpp 508). The six categories are not stored, so no stored field is involved.
Tables: fields and order agree everywhere. Slot meaning: 0 base, 1 combo, 2 sp, 3 solo, 4 accents, 5 ghosts, in `advance`, `emit_path`, `rebuild` and `score_of`. Edge cases: a path with no scores (all zeros) copies as all zeros in every home; a variant gets its leader's six through `prepare_variants` (zeros in its `OutPath` mean "not set"). The one sum written twice: `score_total` (model.h 149) against `p.score += ...` (engine.cpp 826-827) and the two paired adds `c.score += frontend_points` (963) and `c.score += sp_delta` (1076) that go with `sc[2]`.
Disagree on: none today. `emit_path` (engine.cpp 1715-1720) throws if the running score and `score_total` of the six ever differ, so a seventh category added to one and not the other fails at the first path found. It does not catch a category added to every list but one label, or a swapped slot number where both slots feed `score_total` (the sum is symmetric).
Owner proposed: `Path`'s six fields stay the one list. `advance` should build its running add with `score_total(e.basescore, ..., e.ghostscore)` (an inline function, no call cost) instead of its own six-term sum, so the rule has one spelling. The slot numbers could become a named enum local to engine.cpp; that is optional. The replay.h claim that `score_of`/`assign_score` are the only pairing is wrong by reading (engine.cpp 1909-1914 and model.cpp 664-669 also spell it), but those two are copy-outs between differently shaped structs, so I would reword the comment, not merge them.
Evidence: by reading. `test_replay.cpp` 1395-1418 pins `kReplayScoreFields` and `score_of`/`assign_score`. Nothing pins the engine's slot order against `Path` except `emit_path`'s sum check (which cannot see a swap). A one-chart `hydra_replay` selfcheck on a chart in `testdata/input` would show the engine and replay agree per category; I did not run it.
Found by: G4a-1, C2-3, also-seen G4a-1 in G4b; verified by V4.
Scan row or reason: no text search can catch a seventh category, because it arrives under a new name. The only catchable spelling is the hand sum, `p.score +=` of an `e.*score` chain, which a row in rules/search.json could match outside `Engine::advance`; I did not test its noise.

### M31. How does a chord's chart base score relate to the stored "base" category?
Kind: K0 for duplication (two different quantities with one owner each, tied by an inverse and a test). There is a naming note, not a finding. Fix: none. Any wording change is "needs the user" (display text).
Homes: `category_scores`, src/core/scoring.cpp 49-51 (inputs: `ChordNote::basescore()` of each note, its ghost and accent flags) splits a note's 1x value into base, accent and ghost; `Path::chart_base_score`, src/core/model.cpp 848-850 (inputs: `score_base`, `score_ghosts`, `score_accents`) puts the split back together. Callers of the sum: `star_cutoffs` (stars.cpp 18) and `Path::avg_mult` (model.cpp 854). No other site adds them (grep for `score_base +` finds model.cpp 849 only).
Tables (a ghost yellow cymbal at 1x: `basescore()` = (50+15)*2 = 130):
| input | `score_base` ("Notes") | `score_accents` | `score_ghosts` | `chart_base_score` ("Base score") |
|---|---|---|---|---|
| plain red snare | 50 | 0 | 0 | 50 |
| ghost yellow cymbal | 80 | 0 | 50 | 130 |
| accent red snare | 50 | 50 | 0 | 100 |
| accent kick | 50 | 50 | 0 | 100 |
| empty chart (record_fixtures.h 561) | 0 | 0 | 0 | 0 |
| chart with a solo | solo is not in any of the three; it is `score_solo` | | | |
Which number each tab shows for the same path. The Stars tab line "Base score" (src/ui/stars_tab.cpp 27) prints `StarCutoffs::base`, which is `chart_base_score()`: every note once at 1x with no Star Power (the User Guide, docs/UserGuide.md line 289, says the same, so this label is accurate). The Paths tab line "Notes:" (path_view.cpp 429) prints `score_base` alone. On a chart with no ghosts and no accents the two are equal. With any ghost or accent, "Notes" is smaller than "Base score" by exactly "Accent Notes" plus "Ghost Notes" (path_view.cpp 433-434). The label "Notes" is therefore only partly accurate: it is the remainder after the accent and ghost 50s are taken out, so a lone accent snare shows "Notes: 50" for a note worth 100. "Accent Notes" and "Ghost Notes" are accurate as "the 50 each dynamic note adds", not as "the value of the accent notes". All three labels together add up to the Base score, and the Stars tab reads the best path because the sum is the same on every path (no path-dependent term feeds base, accents or ghosts; the graph adds them to both tracks).
Disagree on: none (the two are exact inverses). They would split if the 50 share moved out of `category_scores` without `chart_base_score` knowing.
Owner proposed: no change. `test_stars.cpp` 89 (`n.base + n.accent + n.ghost == row.note.basescore()`) already ties the two directions. If the user ever wants the Paths labels to explain the split, that is a separate wording question (display text), not a merge.
Evidence: by reading. A one-chart `hydra_uitest` script that opens the Stars tab and the Paths tab on a `testdata/input` chart with ghosts would show `Notes + Accent Notes + Ghost Notes == Base score`; I did not run it.
Found by: G4a-2; verified by V4.
Scan row or reason: no row needed. The sum has one spelling already; a second would have to repeat `score_base + score_ghosts`, which a row on `score_ghosts\s*\+` outside model.cpp could flag, but nothing suggests it will come back.

### M32. What does an accent or ghost earn?
Kind: K0 (two different quantities, both documented at the definition). Fix: none; any relabel is "needs the user" (display text).
Homes: the `accent` and `ghost` categories, src/core/scoring.cpp 49-50 and 65-66 (always the pad-independent 50 at 1x); `CategoryScores::dynamics_bonus`, scoring.cpp 80-84 (a note's paid value less a plain note's paid value on the same pad, multiplier included). scoring.h 39-44 states the split on purpose.
Tables (note on one pad at 1x, then at x2):
| note | accent/ghost category | dynamics_bonus at 1x | at x2 |
|---|---|---|---|
| plain note | 0 | 0 | 0 |
| accent or ghost, kick or tom or snare | 50 | 50 | 100 |
| accent or ghost, cymbal | 50 | 65 | 130 |
(The cymbal's extra 15 stays in the base category.) The two differ on every cymbal accent or ghost, by 15 at 1x. That is the documented reason they are two fields.
Correction to the scout: the claim "the Preview reads `dynamics_bonus`" is wrong. In src/ the field is produced (scoring.cpp 84), copied (replay.cpp 136) and never read; its only readers are tools/replay.cpp 401 (JSON output) and tests. What the user sees is only the category totals ("Accent Notes", "Ghost Notes"), which show the 50s. See Side notes.
Disagree on: n/a (not the same question).
Owner proposed: none.
Evidence: by reading; tests/test_stars.cpp 69-89 and test_replay.cpp 1206-1255 pin the numbers.
Found by: G4a-3; verified by V4.
Scan row or reason: no text search applies; the two are different quantities, not a copy.

### M33. Is this chord after the SP end? (`paid_by_sp_walk`, `after_sp_end`, the engine's second call)
Kind: two parts. Part A, the engine's second call to `after_sp_end` on a choice that already carries the answer: K1. Part B, the tick test against the ms test: K1 on paper, but not mergeable (they split on one input). Fix: Part A code-only. Part B no change; a merge would be "needs the user" because it can move a score on a pathological chart.
Homes:
- `core::paid_by_sp_walk(offset_ms)`, src/core/backend_value.h 30 (`offset_ms <= 0.0`). Callers: engine.cpp 1067 (a backend row's offset), model.h 360 (`SPSqueeze::is_free`, a stored squeeze's offset), squeeze_rating.cpp 119, and inside `counted_without_squeeze` (backend_value.h 42).
- `core::after_sp_end(chord_tick, end_tick)`, backend_value.h 36 (`chord_tick > end_tick`). Callers: graph.cpp 430 (stores `SqueezeChoice::late`), engine.cpp 1033, replay.cpp 161 and 416, sqout_chord.h 92 and 152.
- engine.cpp 1033 asks `after_sp_end(sq->chord, sp_end)` while the same choice's answer is `sq->late`.
Where the premise "time rises with tick" is guaranteed. D6 is the decision (docs/audit/2026-10-03-fix-decisions.md line 17). Its code is `check_timing_maps`, src/parse/song.cpp 29-53, which both parsers reach through `Song::build_timing`: resolution above 0, every time signature at least one tick long, every tempo a finite BPM above 0 (an infinite tempo from a 0 us `.mid` tempo gets its own message). The ms of any tick is a pure function of that tick: `MsIndex::at` (src/core/timing.cpp 47-54) reads `elapsed_[i] + (ticks - keys_[i]) / tps_[i] * 1000.0`, with the section start times built by the same formula (timing.cpp 41-44), so ms is continuous across a tempo change and never falls as ticks grow. Edges I checked:
- Tempo change exactly on the SP end tick or on a chord tick: right-continuous and monotone (a tick on a change reads the new tempo and the old section's end value is computed by the same formula). Agree.
- Same tick, same ms: a chord on the end tick has offset exactly 0.0; `paid_by_sp_walk(0.0)` is true and `after_sp_end(t, t)` is false. Agree (this is the "inside, inclusive" edge; test_model.cpp 697-699 pins 0.0, -0.5, 0.5).
- The chart Offset or audio delay: not an input. `offset_ms` in these functions means "distance from the SP end", not the chart Offset; the chart Offset is read only by the Preview (parse/song.h 239-241), and scoring works in chart time. No edge there.
- A tempo that is positive and finite but so fast that two neighbouring ticks round to one double ms: possible. `check_timing_maps` tests only that the BPM is above 0, not that the resulting ms still tell neighbouring ticks apart. Then a chord one tick after the end has ms equal to the end's: `paid_by_sp_walk` says inside (offset 0), `after_sp_end` says after. How fast: the per-tick time must fall below about 1e-16 of the running time, which for a 192-tick chart three minutes in is a tempo around 1e14 BPM. A `.mid` cannot reach it (its tempo is an integer count of microseconds per beat, so at most 6e7 BPM, src/parse/song.cpp 869). A `.chart` can: its `B` value is read as an integer over 1000 (song.cpp 1327), so a hand-made `B 100000000000000000` is accepted. No real chart has this. A tempo so slow that ms overflows to infinity gives NaN offsets, where `paid_by_sp_walk(NaN)` is false; also unreachable in practice.
Tables (rows are chord vs end):
| chord vs end | `after_sp_end` | `paid_by_sp_walk(offset)` | agree? |
|---|---|---|---|
| tick before | false | true (offset < 0) | yes |
| same tick | false | true (offset 0.0) | yes |
| tick after, ms distinct | true | false | yes |
| tick after, ms equal (absurd fast tempo) | true | true | NO |
| offset NaN (absurd slow tempo) | by ticks only | false | NO |
Disagree on: the two absurd-tempo rows. Otherwise none.
Why they cannot simply be one: `paid_by_sp_walk` is the only form available to stored data. `SPSqueeze` and `BackendSqueeze` keep an offset in ms and no tick, so `is_free` and the squeeze rating (model.h 359-362, squeeze_rating.cpp 116-120) cannot ask `after_sp_end`. Making `after_sp_end` ms-based instead would change the answer on the absurd-tempo row for the graph and the replay, and that moves where the search puts a choice (a score). So: needs the user, and I recommend not doing it.
Part A, engine.cpp 1033. `sq` is one of the choices of the edge `node(p.node).branch_edge` (deactivation_type, engine.cpp 997-1008, offers a choice from that edge's `choice_begin..choice_end`). `create_deactivated_path` reads the same edge (`deact_edge = node(p.node).branch_edge`, line 1021). The choice's `late` is `after_sp_end(choice.chord.ticks(), deact_edge->dest->timecode.ticks())` (graph.cpp 430, copied at engine.cpp 182), and the engine's `sp_end` is `node(e.dest).tick` of the same edge, `sq->chord` is the same chord tick. So `sq->late` and `after_sp_end(sq->chord, sp_end)` are the same two integers compared, on every input. Replacing line 1033's call with `sq->late` gives the same answer; the engine already reads `sq->late` at 1115 and 1128.
Owner proposed: `after_sp_end` stays the one tick test and `paid_by_sp_walk` the one ms test, each with its comment (backend_value.h 32-35 already names the equivalence and D6). Add one test that pins the equivalence over a chart with a tempo change on the SP end tick. Change engine.cpp 1033 to read `sq->late`.
Evidence: by reading. The run that would settle the absurd-tempo row: a unit test with a `.chart` whose second tempo is `B 100000000000000000`, checking `after_sp_end` against `paid_by_sp_walk` for neighbouring ticks; the existing test_song_parse.cpp 548-574 shows how such a chart is built.
Found by: G4a-4, G4b-1, also-seen G4a-4 in G4b; verified by V4.
Scan row or reason: the existing row "Is this phrase chord after the SP end?" in tests/single_owner/rules/core.json 236-255 flags `.ticks() > end.ticks()` and `tick > w.deact_tick`. Part A needs no new row (the call already goes through the owner; reading `late` is not a second spelling). A row on `offset_ms <= 0` outside backend_value.h would catch a hand-written ms test; I did not test its noise.

### M34. Which chart rows fall inside an SP end's squeeze window?
Kind: K3. The rule has one owner, `within_squeeze_window` (src/core/model.h 122-124, strict at 500 ms). Every selection loop calls it; the loops differ only in how they walk their data. Fix: none.
Homes (all call the owner):
- Graph sweep: `is_recent_to_head` / `set_head_time` (graph.cpp 320-346), the edge range `backend_begin..backend_end` set in `add_deact_edge` (408-416) and closed at 336, and the per-phrase check at graph.cpp 154-159.
- `core::squeeze_window_phrases` (sqout_chord.h 28-43): binary search to the end tick, then walk outward; phrase chords only (`flag_sp`).
- The tail loop in `Engine::rebuild`, engine.cpp 1943-1952: filters `tail_backends` for an SP end that outlasted the chart.
- `banked_phrase_in_reach` (sqout_chord.h 121-125) and `Activation::display_backends` (model.cpp 834) also call it; the scouts did not list those.
Tables (offset of a row from the end, in ms):
| offset | in window |
|---|---|
| -499.9 | in |
| -500 | out |
| 0 | in (a row on the end tick) |
| +499.9 | in |
| +500 | out |
All homes agree. Sweep check, confirming the scout's reading: `set_head_time` closes an edge (`backend_end = all_backends_.size()`) when the head is 500 ms or more past the edge's end; in `build` the row for the head is stored after `set_head_time` (graph.cpp 133, 146), so the row exactly 500 ms past is excluded and the row at 499.9 is in. A new edge starts at `recent_lo_`, the first row within 500 ms of the head, and its end node is made at that head (advance_tracks then add_deact_edge), so the lower side is the same strict test. Edge: a pending end handled between rows (graph.cpp 129-130) sets the head to the end itself; the same logic holds. The tail loop: `tail_backends_` holds rows within 500 ms of the last chord (graph.cpp 231); an SP end at or after the last chord can only match rows inside that set, so the second filter loses nothing.
Disagree on: none today. All of them rely on ms never falling as ticks grow (the same premise as M33, `squeeze_window_phrases` most strongly, since it stops at the first row outside). The scout's note that the comment at engine.cpp 1939-1942 names `add_deact_edge` where `edge_row_offset` (graph.h 185-187) does the offset is right; the comment is stale by one name.
Owner proposed: unchanged (`within_squeeze_window`).
Evidence: by reading. A unit test over a chart with rows at exactly -500, 0 and +500 ms from an SP end, checking each home's row set, would pin it; tests/test_sp_end.cpp is where such cases live.
Found by: G4b-2; verified by V4.
Scan row or reason: existing rows on `within_squeeze_window` in rules/core.json (owner entries at lines 544 and 593, 1303) already flag a hand-written `fabs(...) < 500`.

### M35. How is the search graph built for a set of settings (three builders)?
Kind: K1 (the same expression, put together twice in src/, plus copies in a tool and a test). Fix: code-only.
Homes in src/:
- `target_graph`, src/search/pather.cpp 257-261: `ScoreGraph(song, graph_build_cap(sp_cap, phrase count), fill_rule_for(legacy_fill_deadline), rules)`. Used at pather.cpp 304 and 332.
- `analyze_chart` then `analyze_at_cap`, pather.cpp 480-502 and 455-476: `graph_build_cap` is called in `analyze_chart` (494) and handed down as `build_cap`; `analyze_at_cap` builds `ScoreGraph graph(song, cap, fill_rule_for(legacy_fills), rules)` at 463.
Outside the audit's src/ scope, noted only: tools/bench.cpp 289-291 (an exact copy of `target_graph`'s expression, to time the build by itself) and tests/test_fast_tempo.cpp 185 (the same `graph_build_cap` expression for a test graph). The scouts said "three"; in src/ it is two, plus those two copies.
Tables (inputs sp_cap, song phrase count, legacy flag):
| input | both homes build at | rule |
|---|---|---|
| cap 4, 12 phrases | 4 | Ch11 if flag clear |
| cap 8, 3 phrases | 3 | |
| cap 0 or less, any phrases | 1 (floor in `graph_build_cap`, pather.cpp 445-447) | |
| legacy flag set | same cap | Ch10 |
| song with 0 phrases | 1 | |
Extra row in `analyze_at_cap`: `build_cap` unset builds at the raw `sp_cap` (pather.cpp 461-462). I checked all callers across the repo: `analyze_chart` is the only caller of `analyze_at_cap` and always passes `build_cap`, so the row is never taken.
Disagree on: none today. They would split if a future caller of `analyze_at_cap` leaves `build_cap` empty, or if a new graph input (for example another rules field) were added to one builder and not the other.
Owner proposed: `target_graph` (renamed to something like `search_graph`) is the one builder. `analyze_chart` calls it and passes the graph to the all-0 pass and `read`, instead of unpacking six settings into `analyze_at_cap` that rebuilds the arguments. The unused `build_cap`-unset branch goes with it. The answer is identical on every input above. Because the cap also feeds the SP end clamp (`extend_deacts`), the identity matters; it holds because both sides call `graph_build_cap` and `fill_rule_for` (the existing test at tests/test_engine_sweeps.cpp 569-605 pins that a 4-bar graph built at the song's phrase count stores the same paths).
Evidence: by reading. The run that would prove no change: the same `hydra_batch --db` run on three charts from `testdata/input` before and after, comparing stored best scores and path strings (not a library run).
Found by: G4b-3; verified by V4.
Scan row or reason: a row on `graph_build_cap\(` called outside pather.cpp would flag the bench and the test copy if they were in scope; the existing "Which fill rule does a legacy_fills flag mean?" row (search.json 217-248) only matches a ternary. For the `ScoreGraph` expression itself: a row on `ScoreGraph\s+\w+\s*\(` followed by `fill_rule_for(` outside pather.cpp is the spelling, if the user wants it.

### M36. How many paths are tied under a root?
Kind: K1 in principle (the same recurrence, "one plus the sum of the variants' counts", in two forms), but the fix is to leave it alone. Fix: none; merging is not possible without building the tree first.
Homes:
- The engine's running count: `Path::tied_count`, `Variant::tied_count`, `OutPath::tied_count` (engine.cpp 245, 287, 323); set to 1 on a root (1747); added on a fold (1263) after the limit test `leader.tied_count + p.tied_count <= max_tied_paths_` (1241); copied out (1730) and kept as `engine_tied` (2032).
- `Path::recount_tied_paths`, src/core/model.cpp 649-656: leaf is 1, a node with variants is 1 plus each variant's recount. Called in `Engine::rebuild` (engine.cpp 2059) and twice in `keep_paths_where` (pather.cpp 187 and 201) after pruning. This is the number that is stored and shown (`tied_pathcount`, model.h 770).
Tables:
| case | engine count | recount |
|---|---|---|
| a root with no ties | 1 | 1 |
| a root with 2 folded variants, one of which carries 1 folded variant | 1+1+(1+1) = 4 | 1+1+(1+1) = 4 |
| a fold that would pass the limit | not folded, variant dropped (1241, 1265) | the dropped variant is not in the tree, so not counted |
| after `keep_target_paths` prunes variants | no longer used | the only count |
Disagree on: none today. `rebuild` throws if the two disagree for any root (engine.cpp 2055-2063, finding 179), so a drift fails at the first search.
Owner proposed: unchanged. The recount owns "what is stored and shown". The engine's running number answers a different question in the middle of the search (may this variant still fold?) and cannot read a tree that does not exist yet. The runtime check is the right tool here.
Evidence: by reading. The run that would prove it: any test that drives `rebuild` on a tied chart (tests/test_tied_variants.cpp) already exercises the check.
Found by: G4b-5; verified by V4.
Scan row or reason: core.json 2128 (`tied_count += v.tied_count;`) is a must-match example for an existing row, so a hand-written recount outside `recount_tied_paths` is already flagged.

### M37. Is every timing of this activation inside the ms limit?
Kind: K3. Every comparison goes through `timing_within_limit`, `SPSqueeze::within_limit` and `fill_within_limit` (model.h 276, 354, 306-308); the two walks differ only in the shape of the data they loop over. Fix: none.
Homes: `Engine::act_within_limit`, src/search/engine.cpp 1137-1146 (the engine's squeeze chain, then the early fill; no limit set or no activation means true); `Activation::within_ms_limit`, src/core/model.cpp 543-547 (the stored `sqinouts`, then the fill). A third use of the same helper: `ready_class`, engine.cpp 800, for a fill about to be passed.
Tables (limit L):
| activation | both walks |
|---|---|
| no limit set (engine) / limit passed as a number (record) | engine true; record compares to the number |
| no activation (act < 0) | true |
| any squeeze with difficulty above L | false |
| early fill that is an E0 with difficulty above L | false |
| optional fill (not an E0) | ignored in both, by `fill_within_limit` |
| difficulty exactly L | within (the comparison is `<=`) |
Disagree on: none. Note for whoever works near this: `Activation::hardest` / `hardest_part` (model.cpp 490-512) is a third walk over the same timings with deliberately different rules (it skips free squeezes and counts the optional fill of an E-critical activation). That is the "hardest part" question, not the limit question, and must not be folded into `within_ms_limit`.
Owner proposed: unchanged.
Evidence: by reading.
Found by: G4b-6; verified by V4.
Scan row or reason: already covered. The row in rules/core.json (the "any comparison against a ms limit by its name" entry, notes at lines 2329-2346) names both walks as calling the owner.

### M38. The fill-deadline "4 beats" rule computed in code and restated in a user sentence
Kind: K1 (one number, 4, written as a calculation and as a word in a sentence). Fix: code-only, provided the sentence reads exactly the same afterwards.
Homes: `activation_fill_deadline_ms`, src/search/graph.cpp 62-91 (Ch11: fill start minus `4 * tick_resolution()` ticks; Ch10: lead time clamped to 250..10000 ms, with the fill-length and `res/16` pad); `fill_rule_description`, src/search/graph.h 76-81 ("... 4 beats before the fill starts." / "... about one fill-length before the fill starts."), printed by the settings tooltip (src/ui/settings_bar.cpp 29-30) and the fill report footer; the header comment graph.h 31-35 (restates both rules, including 250..10000). Outside src/: docs/UserGuide.md line 96 says "4 beats" and "fill-length" too; tests/test_fill_deadline.cpp 180-181 pins the sentence as a literal.
Tables (Ch11 vs the sentence, fill of length F ticks ending at E, resolution R):
| rule | code | sentence |
|---|---|---|
| Ch11 | deadline tick = E - F - 4*R | "4 beats before the fill starts" |
| Ch10 | max(250, min(fend - pad, 10000)) ms lead | "about one fill-length" (no numbers) |
Only the Ch11 "4" is a copy. The Ch10 sentence carries no number on purpose.
Disagree on: none today. If the 4 changed in code, the sentence (and the User Guide) would still say 4.
Owner proposed: a named constant for the lead in beats (for example `kCh11FillLeadBeats = 4`) in graph.h, used by `activation_fill_deadline_ms` and by `fill_rule_description` (which writes `std::to_string(kCh11FillLeadBeats) + " beats"`). The displayed sentence stays word for word, so no displayed text changes. Also reword the graph.h header comment to name the owner instead of repeating 4 and 250..10000 (the preamble's comment rule). The User Guide line stays hand-written (a public page; checking it against the code belongs to the public-doc review).
Evidence: by reading. `tests/test_fill_deadline.cpp` 178-181 would catch a sentence change.
Found by: G4b-7; verified by V4.
Scan row or reason: a row on a quoted string containing `\b4 beats\b` in src and tools, owner `fill_rule_description` in graph.h, would flag a second sentence that states the number.

### Side notes

- `ReplayNote::dynamics_bonus` (src/core/replay.h 156, filled at replay.cpp 136, computed at scoring.cpp 84) has no reader in src/. Only tools/replay.cpp 401 and tests read it. The scout's statement that the Preview reads it is wrong. It is either unused app code kept for the tool, or a half-finished feature; the header comment replay.h 153-156 says it is for "a mis-hit". Not a duplication question.
- src/search/engine.cpp 1939-1942: the comment says the tail loop uses "the same offset rule add_deact_edge uses"; `add_deact_edge` does not compute an offset, `edge_row_offset` (graph.h 185-187) does. Stale by one name.
- src/core/replay.h 236-238 says `score_of`/`assign_score` "spell the Path-to-ReplayScore field pairing, and nothing else does". By reading, engine.cpp 1909-1914 (OutPath to Path) and model.cpp 664-669 (leader to variant) also spell the six; the sentence overstates (see M30).
- `EdgeView v;` in `enumerate` (engine.cpp 163) has no initialisers; a field added to `EdgeView` and forgotten in the converter would be garbage, and the compiler does not check it (scout G4b already noted this; confirmed by reading).
- `check_timing_maps` (src/parse/song.cpp 29-53) checks that a tempo is above 0 and finite, but not that the resulting ms still tell neighbouring ticks apart (see M33). Reachable only by a hand-made `.chart`.

---

## V5: paths and squeezes on screen (candidates M39 to M46)

Task id: concept-audit-V5. Agent id used for status lines: V5 (the progress hook never named another). Read-only; nothing built or run. Every claim below was checked against the code at the main checkout, not against the scout text. The old ledger in `docs/audit/2026-10-03-derivation-audit.md` is cited by finding number; its status comes from `docs/audit/2026-10-03-fix-triage.json` (`status_at_head`).

Short answer: M39 is a K2 the user already decided to keep (finding 2, D50 item 1). M41 is a K2 too, already looked at once and called "not a copy". M40, M42 and M43 are small. M44 and M45 are known line-level findings and both are settled. M46 is real but wider than the scout saw (19 uses in 5 files, not 5).

### M39. Who words the Rating cell, what does "(uncounted)" mean, and which word rates a squeeze ("Hard", "Insane" with different edges)?
Kind: K2 (same words "Hard" and "Insane" on two ladders with different edges, and the user sees both). Known and decided: derivation audit finding 2, answered by D50 item 1 (keep both ladders, say so in the report footer). Still true in the code. Fix: needs the user (displayed text) if the ladders were ever merged; as it stands nothing is open except a wording tidy-up.
Homes: `BackendSqueeze::summarystr`, src/core/model.cpp 339-361 (signed offset of one backend row from the SP end, squeezed-out flag, hit window W, leeway, `kBackendInnerBandMs` = 10, model.h 96);
       `timing_tiers`, src/core/squeeze_rating.cpp 165-176 (one ms figure: the path's hardest squeeze, W; read by `tier_for`, src/app/report.cpp 123-149);
       the Rating cell's other two authors: `build_activations`, src/app/path_view.cpp 372-391 (adds " (eff. N ms)" and " <-- squeezed out (-N)" or " <-- squeezed out"), and `kBackendTimingsLead`, path_view.cpp 170-174 (prose that names the tag).
Where each set is shown: the `summarystr` words go in the Rating column of the "Backend timings" table on the Paths tab (`row.rating`, path_view.cpp 372, drawn by `render_backend_table` in ui/paths_tab.cpp). The `timing_tiers` words go in the Paths report window: the "Timing" chip column and the Timing dropdown (src/app/path_report_view.cpp 58-72 and 115-127), the "Past N ms" tile and the footer sentence (report.cpp 337 and 419-424). `docs/UserGuide.md` line 198 quotes `Free SqOut (uncounted)`.
Tables (W = 85, band = 10, leeway = 3; the defaults; edges in):

The report ladder (input: hardest squeeze in ms, a SqOut counts as minus its offset; each edge belongs to the band below it, report.cpp 142-147):

| Hardest ms | Tier |
|---|---|
| no squeeze | None |
| up to and including 2 | Normal (negative values too) |
| over 2, up to 42.5 | Hard |
| over 42.5, up to 85 | Extreme |
| over 85, up to 127.5 | Insane |
| over 127.5, up to 170 | Insane+ |
| over 170 | Beyond |

The row ladder, plain row (input: the row's offset from the SP end; every edge strict):

| Offset | Label |
|---|---|
| below -85 | Free |
| -85 up to below -10 | Easy |
| -10 up to below 3 (the leeway) | Standard |
| 3 up to below 85 | Hard (uncounted) |
| 85 and up | Insane (uncounted) |

The row ladder, the activation's squeezed-out row (the tag " (uncounted)" is added when offset is 3 or more):

| Offset | Label |
|---|---|
| below -85 | Insane SqOut |
| -85 up to below -10 | Hard SqOut |
| -10 up to below 10 | Standard SqOut |
| 10 up to below 85 | Easy SqOut |
| 85 and up | Free SqOut |

One SqOut at each offset, both homes side by side (the SqOut's report figure is -offset; `squeeze_difficulty`, model.h 268):

| SqOut offset | Report tier | Row label |
|---|---|---|
| -130 | Insane+ | Insane SqOut |
| -90 | Insane | Insane SqOut |
| -85 | Extreme | Hard SqOut |
| -5 | Hard | Standard SqOut |
| 0 | Normal | Standard SqOut |
| +40 | Normal | Easy SqOut (uncounted) |

One SqIn (report figure is +offset) against its phrase chord's plain row: +40 is Hard and "Hard (uncounted)" (the words happen to agree); +85 is Extreme and "Insane (uncounted)".
Disagree on: nearly every input. Same words, different questions: the tier says how big the squeeze is in steps of the hit window; the label says how far the note sits from the SP end and whether the engine counted it. Example: a SqOut 85 ms before the end is "Extreme" in the report and "Hard SqOut" in the table; a plain row 85 ms after the end is "Insane (uncounted)" in the table and (as an 85 ms SqIn) "Extreme" in the report. The Rating cell's own "(uncounted)" rule agrees today across its three authors: `summarystr` and the view both call `core::counted_without_squeeze` (model.cpp 347 and 358, path_view.cpp 366), the lead sentence only names the tag, and `is_frontend_decided` (squeeze_rating.cpp 88-92) asks the same function. A change to the leeway edge in one place only would split them; today none can.
Owner proposed: leave both ladders where they are (the user chose this in D50 item 1). The cheap tidy-up is not a merge: put the label ladder's words beside `timing_tiers` in `core/squeeze_rating` (CONTEXT.md files "Squeeze rating" there, yet `summarystr` sits in model.cpp), and make the tag a named constant read by the lead sentence. That is a move, not a rewording.
Evidence: by reading. Both ladders have unit tests that pin edges (tests/test_report_rows.cpp 499-524, tests/test_squeeze_rating.cpp 639-687).
Found by: G5-1, C1-4; verified by V5.
Scan row or reason: no text search can say two ladders "should" agree; the user has said they need not. A row on the tag spelling `(uncounted)` outside model.cpp and path_view.cpp (rules/app.json) would catch a fourth author of the tag.

### M40. How is one squeeze or fill timing written for the user?
Kind: K3 (one shared fact: how many decimals a screen shows; the rule "whole ms rounds to nearest" lives once in `format_ms_whole`, the rule "one decimal" once in `format_ms`). Known: derivation audit finding 4 (and 18, 36), all `fixed` in the triage file (M_D 57dcf23). Still true; no open rule.
Homes: `format_ms_whole`, src/core/model.cpp 920 (`std::lround`, then " ms"), called by `activation_badge` (path_view.cpp 98), `Activation::notationstr_verbose` (model.cpp 561-574), `longest_activation_badge`, `settings` text (app/config.cpp 359), library_dialogs.cpp 70;
       `format_ms`, src/app/display_format.cpp 24 (`%.1f ms`), called by `squeeze_sentences`, the `(eff. ...)` suffix, the tooltip, the early-fill line, the path buttons and the path report columns.
Tables: one SqIn at +12.6: verbose "1+ (13 ms)", badge "squeeze in 13 ms", sentence "more than 12.6 ms early", button "12.6 ms". A free SqIn at -316: verbose "0+ (-316 ms)", badge "squeeze in -316 ms" (free squeeze fallback, D80), sentence "no more than 316.0 ms late" (sentence takes `fabs`), button shows nothing (`Path::difficulty` skips free squeezes). Edges: -0.4 reads "0 ms" whole and "-0.4 ms" one-decimal; `SPSqueeze::timing` and `squeeze_difficulty` add `+ 0.0` so a dead-on squeeze never prints "-0.0".
Disagree on: rounding and sign, on purpose (display_format.h 18-25 states the split). The only repeated fact is the " ms" suffix, spelled in both formatters because they sit in two layers (core cannot call app).
Owner proposed: leave. If the user wants one place, move `format_ms_whole` next to `format_ms` is blocked by the layer rule (core/model.cpp uses it), so `format_ms` would have to move down to core instead; code-only, no word changes.
Evidence: by reading.
Found by: G5-2; verified by V5.
Scan row or reason: a row on a hand-typed `"%.1f ms"` or `lround(` followed by `" ms"` outside those two functions (rules/app.json) would flag a third formatter.

### M41. What does "free" mean for a squeeze?
Kind: K2 (same word, different rules; low weight, because the user sees "Free" from only one home). The 2026-10-03 audit looked at this and cleared it as "not a copy" (derivation audit line 2956: `summarystr`'s "Free SqOut" means slack of a whole hit window). Fix: needs the user if the word changes on screen; code-only if only the internal name `is_free` changes.
Homes: `SPSqueeze::is_free`, src/core/model.h 359-362 (offset; SqIn is free at offset <= 0, SqOut is free at offset > 0, through `core::paid_by_sp_walk`, backend_value.h 30). Callers: `hardest_part` (model.cpp 496), `rate_activation` (squeeze_rating.cpp 147), `squeeze_sentences` (path_view.cpp 133 and 155). No caller prints the word "free".
       `BackendSqueeze::summarystr`, src/core/model.cpp 352 and 355 (a whole hit window of slack).
Exactly where "free" reaches the screen: two string literals, `return "Free";` at model.cpp 355 (plain row, offset below -W) and `: "Free SqOut";` at model.cpp 352 (squeezed-out row, offset W or more). They show in the Rating cell (path_view.cpp 372) and in `docs/UserGuide.md` line 198. No other string literal in src/ contains "free" as a word (searched all of src/ for quoted text).
Tables (W = 85):

| Case | `is_free` | Row label |
|---|---|---|
| SqIn at -316 | free | plain row "Free" |
| SqIn at -5 | free | plain row "Standard" |
| SqIn at 0 | free | "Standard" |
| SqIn at +5 | not free | "Hard (uncounted)" |
| SqOut at +90 | free | "Free SqOut (uncounted)" |
| SqOut at +20 | free | "Easy SqOut (uncounted)" |
| SqOut at +1 | free | "Standard SqOut" |
| SqOut at 0 | not free | "Standard SqOut" |
| SqOut at -5 | not free | "Standard SqOut" |

(A SqIn's offset is measured from the end before its phrase extended SP, a backend row's from the final SP end, so one note can differ between the two columns for that reason too; `rate_activation` comment, squeeze_rating.cpp 127-131.)
Disagree on: SqIn between -85 and 0, SqOut between 0 and 85. `is_free` means "already done, needs no hit timing"; the label "Free" means "at least a whole hit window of slack". Example from each end: SqIn at -5 (free in code, "Standard" on screen) and SqOut at +90 (free in both).
Owner proposed: no merge. If the user cares, the cheap move is renaming `is_free` to a name that says what it tests (for example `needs_no_timing`, which is what its comments and the all-0 rule already call it), so the word "free" is only the table's. Rename is code-only.
Evidence: by reading.
Found by: G5-3; verified by V5.
Scan row or reason: a row on `\bis_free\b` after a rename would flag its return; a row cannot say the two meanings must agree.

### M42. What is each path group called on screen?
Kind: K3 for the words (per-view text on one shared enum, `PathButtonView::Group`). One real rule is hiding in the neighbourhood: the all-0 hint in ui/paths_tab.cpp 179-181 restates the all-0 rule in prose. Fix: code-only for the word table; the hint is a displayed sentence, so a change to it needs the user.
Homes: `render_path_list` headings, src/ui/paths_tab.cpp 174-176 ("Optimal", `list.within_label`, "Best all-0 path");
       `within_label`, src/app/path_view.cpp 476-480 ("Within N scores" or "Within N points", through `counted`);
       `preview_path_label`, src/app/preview_view.cpp 710-717 (notation + "  (optimal)", notation + "  (best all-0)", notation alone);
       the headline line, src/ui/details_panel.cpp 123 ("Optimal path", a third spelling of the first group, found beyond the scouts' list);
       the all-0 button detail, path_view.cpp 515-518 ("N below optimal" / "N above optimal").
Table: Optimal -> heading "Optimal", picker "<notation>  (optimal)", headline "Optimal path". Within -> heading "Within N ...", picker plain. AllZero -> heading "Best all-0 path", picker "<notation>  (best all-0)", detail "N below optimal".
Prose check for the hint: "no skips" and "E1 counts as no skip" match `allzero_activation` (model.h 317: skips == 0, or skips == 1 and the early fill is inside its window); "0 ms or easier" matches the all-0 pass's 0 ms limit (`allzero_options`, search/pather.h 74, `hard_ms_filter`). It agrees today; nothing ties it to them.
Disagree on: none today (same meaning, per-view spelling). A rename of the all-0 path touches paths_tab.cpp, preview_view.cpp and the hint; none of the three points at a shared word. The header comment of paths_tab.cpp saying every string comes from `PathsTabCache` is not true for the two heading words.
Owner proposed: if the user wants renames to be one-place, a small `group_name(Group)` table in app/path_view.cpp (next to `within_label`) returning "optimal" / "best all-0" noun phrases that each view decorates. Only worth doing together with the next rename.
Evidence: by reading.
Found by: G5-4, C1-5; verified by V5.
Scan row or reason: a row on the literals `all-0` and `(optimal)` in string literals outside path_view.cpp, paths_tab.cpp, preview_view.cpp (rules/ui.json) would catch a fourth screen wording the group; no text search can catch the hint drifting from the rule it describes.

### M43. Which kind of squeeze is this: the engine's SQ_IN and SQ_OUT ints against `SqueezeKind`?
Kind: K1 (the mapping "0 is SqIn, 1 is SqOut" is a rule written twice: once as the enum's implicit values and once as hand-typed constants, with nothing tying them). Fix: code-only.
Homes: `enum class SqueezeKind { SqIn, SqOut }`, src/core/model.h 263;
       `const int32_t SQ_IN = 0; SQ_OUT = 1;`, src/search/engine.cpp 34-35, stored in `SqNode::kind` (215-217) and `OutSq::kind` (340-341).
Uses of the ints: pushed at engine.cpp 1047 and 1107; compared at 1141 (`== SQ_IN`, else SqOut), 1548 (`== SQ_OUT`), 1567 (`== SQ_IN`, handed to `sqin_rank`), 1962 (`== SQ_OUT`, else SqIn); turned back to the enum at 1142 and 1967. There is no `static_assert` linking the two (searched engine.cpp).
Tables: int 0 -> SqIn, 1 -> SqOut at line 1142 (anything other than 0 reads SqOut). Line 1962 reads anything other than 1 as SqIn. They agree for the only two values ever pushed (1047, 1107); they would split on a third value or a flipped enum order.
Disagree on: none today; adding a third kind, or reordering the enum, splits them.
Owner proposed: `SqNode::kind` and `OutSq::kind` hold `SqueezeKind` directly; `SQ_IN`/`SQ_OUT` go away. `SqueezeKind` is a plain enum class with an int underlying type, the same four bytes as `int32_t`, so the flat arrays do not grow. The lambda at 1567 compares against `SqueezeKind::SqIn`. Stored records are not touched (the record does not store the kind; only `sqin_count`/`sqout_count` reach the database, from `summarize_path`, record_store.cpp 513-519, which already reads the enum).
Other readers checked and fine: `summarize_path` and path_view.cpp 129 compare the enum; `squeeze_kind_from_name` (model.cpp 319) is the one name-to-enum reader (tools/replay_json.cpp 33 uses it now; the older hand-typed "SqOut" read-back in the audit is gone). The display words ("SqIn" column title, path_report_view.cpp 153 and 157, and the badge words) are per-screen text.
Evidence: by reading. The engine's output is pinned by the engine digest tests (`tests/test_perf_digest.cpp`), which would catch a flipped mapping.
Found by: G5-5 and also-seen in G4b; verified by V5.
Scan row or reason: a row on `\bSQ_(IN|OUT)\b` in rules/search.json would flag the constants if they came back.

### M44. How is an activation's deactivation tick read?
Kind: K1, known line-level; no tables. Confirmed still open in the code. It is an old finding of the previous audit's kind (a pass-through function); no finding number was found for it in `2026-10-03-derivation-audit.md`.
Homes: `Activation::deact_tick`, src/core/model.cpp 709-712 (empty `sp_end_steps` gives nullopt, else the last step's `end_tick`); `activation_deact_tick`, src/core/squeeze_rating.cpp 22-24, which returns `act.deact_tick()` and nothing else (the header says so, squeeze_rating.h 53-58, and model.h 671-672 repeats it).
Callers: the free function is called by src/app/preview_view.cpp 374 (one production caller) and by tests (test_sp_end.cpp, test_sp_meter_curve.cpp, test_preview_scene.cpp, test_squeeze_rating.cpp 280-298); the member is called by model.cpp 642, replay.cpp 245, squeeze_rating.cpp 29, engine.cpp 2005 and tools. The only expression `sp_end_steps.back()` in src/ and tools/ is the one inside the member (searched).
Fix: code-only. Owner: `Activation::deact_tick`; the Preview calls it and `activation_deact_tick` and its test case are deleted.
Found by: G5-6; verified by V5.
Scan row or reason: a row on `sp_end_steps\.back\(\)` outside `Activation::deact_tick` (rules/core.json) would flag a second reader; today it matches one line.

### M45. Does squeezing out this row cost points?
Kind: K1, known: derivation audit finding 150, `fixed` in the triage file (M6-J3 e9e2eeb). Still true in the code as one shared helper; short block as asked.
Homes: `core::sqout_cost`, src/core/backend_value.h 56-59 (full SP value less `backend_row_value` at the Exact position); callers `build_activations` (path_view.cpp 381-389, "(-N)" and warning colour when `counted_without_squeeze`) and `squeeze_sentences` (path_view.cpp 139-150, "scores N fewer points" when the cost is above 0, else "costs no points").
Can a counted row cost 0? No, by reading. For a counted row the cost equals the chord's `sqout_reduction` (points less sqout_points). `category_scores` (core/scoring.cpp 55-67) sets `sp = (kStarPowerMultiplier - 1) * paid` and `sqout_reduction += paid` for note 0 on every chord (`loses_sp = i == 0 || WholeChord`), where `paid = basescore * to_multiplier(combo)`. `ChordNote::basescore` is `kNoteBasePoints` (50) plus the cymbal bonus, doubled for ghosts and accents (model.cpp 83-87), and `to_multiplier` is 1 to 4 (timing.cpp 9-14). So the cost is at least 50. A one-note chord stores `sqout_points = 0` and costs its whole SP value, not zero. The only way to cost 0 is a chord with no notes; the chart sequence holds none (not proven to the last constructor; low risk). Consequence: the branch `" It costs no points, and its SP phrase banks for later."` (path_view.cpp 145-146) cannot run, and no test or UI script pins its text (the only src hit is the line itself).
The run that would prove it for a real chart: `hydra_tests -tc="activation rows*"` (the Burnout case reads "260 fewer points", tests/test_activation_rows.cpp 447; the Tapestry-style "never counted" case, 672). A universal proof would be a new test case that loops `category_scores` over every pad, flag and combo band and checks `sqout_reduction > 0`.
Fix: code-only (delete the dead branch, or leave it as a guard). Not a user question: no displayed word that can occur changes.
Found by: G5-7; verified by V5.
Scan row or reason: both callers already call the owner; no text search is needed. A row on `points - ` plus `backend_row_value` outside backend_value.h (rules/core.json) would catch the subtraction coming back.

### M46. The " · " separator between facts on a line
Kind: K1 (one character and spacing, typed at every use). Wider than the scout saw: 19 uses in 5 files, one named constant. Fix: code-only if the constant is shared and the two odd spellings stay as they are; needs the user if those two are made to match (a displayed spacing change).
Homes: `kDot`, src/app/path_view.cpp 28 (used at 208, 406, 508); the literal `\xC2\xB7` typed at ui/details_panel.cpp 74 and 124; ui/library_dialogs.cpp 55, 56, 89, 90, 91, 130, 445 (two spaces each side: `"  \xC2\xB7  Now: "`), 446, 456, 532; ui/paths_tab.cpp 466 (leading bullet with no space before: `"\xC2\xB7 %s"`); app/preview_view.cpp 537, 580, 699. No shared constant exists (searched src/ for the character and for `kDot`).
Table: " · " (space, U+00B7, space) at 17 of 19; library_dialogs.cpp 445 uses double-space; paths_tab.cpp 466 is a prefix bullet, not a separator.
Disagree on: spacing at library_dialogs.cpp 445 only (and 466's role). Same on every other input.
Owner proposed: one `kFactSeparator` in app/display_format.h (ui/ already includes app/ headers), `kDot` becomes a use of it. Leave 445 and 466 spelled out with a comment, or put them to the user.
Evidence: by reading.
Found by: G5-8; verified by V5.
Scan row or reason: a row on the escaped character `\\xC2\\xB7` outside the constant's definition (rules/ui.json and rules/app.json) would flag a returning copy. The current rule files quote two such lines as flagged-line examples (store.json 184, app.json 957), so those two rows will need the constant's call as their allowed form when this is done.

### Side notes

- src/app/path_view.cpp 145-146: the "costs no points" branch for a counted row is dead (see M45).
- CONTEXT.md (Hit window entry, around lines 234-237) says "Normal below 2 ms ... Hard below half a hit window"; the code is inclusive (`*ms <= *t.cutoff`, report.cpp 147, and a test pins 2.0 as Normal and 42.5 as Hard). The doc says "below" where the edge belongs to the lower band. Wording drift only.
- The derivation audit's finding 2 table lists "-85 -> Insane" for the report; today -85 is Extreme (cutoffs were reworked after it was written). The footnote there is out of date, not the code.
- Finding 152 (the engine's hand walk of an activation's timings): the leaf rules are now shared (`SPSqueeze::within_limit`, `fill_within_limit`) and the triage file marks it fixed, but `Engine::act_within_limit` (search/engine.cpp 1137-1146) still loops over the squeezes itself beside `Activation::within_ms_limit` (model.cpp 543-547). It agrees on every input today; it is the same int-to-enum conversion M43 removes.

---

## V6: the Preview (M47 to M56)

Verifier: concept-audit-V6 (status lines went to `V6.md`; the progress hook never named another id). Read-only, no builds, no tests. Every home below was found by symbol and read in the main checkout. Paths are under `src/` unless they start with `tests/`.

Short version. Two candidates are real K2 findings that need the user: M47 (the playhead's range is decided in several places that disagree at two edges) and M50 (three different "ends of the song", shown under similar words). One is a small K1 that is safe to merge: M56 (a dead copy of the solo flag). M49 is half K1, half "different questions on purpose". M48, M51, M52, M53 agree today and are small K1s with a cheap fix. M54 and M55 are K3 with one two-line K1 tail each.

### M47. Where can the playhead be, and what does the screen show outside that?
Kind: K2 (the clamps are the same idea with different ranges, and at two edges the user sees the difference). Fix: needs the user (a choice between behaviours: which range the scrubber and the boxes should use).
Homes (every place the playhead is clamped or pinned):
- `PreviewTransport::seek_ms`, ui/preview_transport.cpp 77-80 (a requested ms, `length_ms_` L = later of the last drawn note and the audio end in chart time). Range [0, L] when L > 0, [0, no limit) when L = 0.
- `PreviewTransport::tick`, 99-107 (clock, L). When playing, L > 0 and clock >= L: pause and seek to L. With L = 0 it never stops.
- `PreviewTransport::set_last_note_ms`, 34-42 (a shorter range re-seeks the clock through `seek_ms`, so it is the same clamp, not a new one).
- `shown_length` and `shown_ms`, app/preview_view.cpp 485-494 (now, `length_ms` = L from `playback_end_ms()`). Range [0, max(L, 0)]. Called from the time box (503), score box (554), drain box (598), `step_tick_ms` (633) and next-activation box (688).
- `scrub_thumb_ms`, 660-664 (now, scrub edge E from `scrub_end_ms`). min(now, E) when E > 0, else now. No lower clamp.
- `song_fraction`, 640-643 (an ms, a song length). Clamp to [0, 1], none when the length is not above 0. Used for the gold marks and the Paths timeline marks.
- `step_tick_ms`, 630-636: target tick floored at 0, no upper clamp (the transport clamps after).
- The slider, ui/preview_tab.cpp 497: range [0, E in seconds], or [0, 1] when E is not above 0.
- Under the transport, in audio frames, not chart ms: `Playhead::seek_frames` (audio/player.cpp 24-26, [0, length]); `Playhead::read_frames` end (70-73, position pinned to length, auto-pause); `BufferSource::seek` (audio/stream_mix.cpp 33); `StreamMix::seek` (255-256); `seek_clamp_or_end` (audio/stem_reader.h 119-128, called from ma_reader.cpp 222 and 425, opus_reader.cpp 285, vorbis_reader.cpp 84). These four agree with each other ([0, length], below 0 gives 0, above gives length). Their range is the audio's, reached through `audio_ms_of_chart_ms` (chart ms plus the offset), so it is a different interval from the transport's.
- Not clamped at all: the highway time (`renderer_->render(transport_.tick())`, ui/preview_controller.cpp 367), the SP meter readout (`sp_meter_readout(scene_.sp_meter, transport_.now_ms())`, 486) and `activation_jump_ms` read the raw clock. The clock can sit past L for a frame between two ticks; `shown_ms` is what hides that from the boxes.

Tables (L = playback end, E = scrub edge = the song's length when `has_song_length`, else L):
| input | transport seek / tick | boxes (`shown_ms`) | thumb (`scrub_thumb_ms`) | highway |
| --- | --- | --- | --- | --- |
| ms < 0 | 0 | 0 | now (no lower clamp) | raw clock |
| 0 <= ms <= min(L, E) | ms | ms | ms | ms |
| L < ms (L > 0) | L; tick pauses at L | L | min(ms, E) | stops at L, because tick pauses |
| L = 0 (no notes, no audio) | any ms allowed; tick never pauses | pinned to 0 | E > 0 gives min(ms, E), else ms | clock runs on |
| E > L (stated length longer than notes and audio) | seek to E becomes L | L | thumb waits at min(now, E) = now | n/a |
Edge claim 1 (no notes, no audio): confirmed by reading. `load` gives L = max(0, nullopt -> 0) = 0 (transport.cpp 26-32), `seek_ms` skips its upper clamp when L is 0 (line 79), `tick` skips its stop when L is 0 (line 102), and `shown_ms` pins to 0 (line 493). It is deliberate on the transport side: tests/test_preview_transport.cpp 73 and 77 ("scrubbing past a zero length is allowed", "tick() never pauses"). So the clock runs on while the boxes stay at 0:00 / 0:00. Reachability is low: a chart with no drum notes and no audio.
Edge claim 2 (scrubber range can exceed playback range): confirmed by reading, not by a run. `PreviewController::scrub_end_ms` (preview_controller.cpp 403-405) gives the stated song length when `has_song_length` accepts it. The slider spans [0, E]. A drag to the right end calls `seek_ms(E)`; the transport clamps to L when E > L. Next frame the thumb is drawn at `scrub_thumb_ms(now = L, E)` = L, short of the right end, so the thumb snaps back from the edge. This needs a stated length longer than both the last note and the audio, which is ordinary for a chart whose song.ini states a length with a short audio tail. The existing test (tests/test_preview_transport.cpp 282-294) passes the audio end as the "song length", so it cannot catch this.
Third edge: `scrub_thumb_ms` has no lower clamp and `shown_ms` has one. Harmless today because the clock is never negative after `seek_ms`.
Disagree on: (a) L = 0, transport free-runs while the boxes pin to 0; (b) E > L, the slider range is longer than the range a seek can reach.
Owner proposed: the transport's range stays the owner of "where the playhead can be" (it is the only one that moves anything). `shown_ms`, `scrub_thumb_ms` and the slider should read one range value from it, not each derive their own. Whether the scrubber should be allowed to reach E (seek past L, as L = 0 already allows) or be limited to L is the user's choice, so this is not a code-only fix. The audio-layer clamps stay: they guard a buffer, a different thing.
Evidence: by reading. A run that would prove edge 2: a uitest script that opens a chart in `testdata/input` whose song.ini states a length longer than its audio, drags the scrubber to the right end and reads the time box; edge 1 needs a chart with no notes.
Found by: G6a-1, G8-7, also seen in G8 (G6a-1); verified by V6.
Scan row or reason: the display clamp already has a row on `now_ms < 0\.0 \? 0\.0 : now_ms` and `now_ms > len \? len` (rules/app.json, the "Which moment do the Preview's overlay boxes read?" row). A row for the other spellings cannot be written: `std::clamp(x, 0, length)` and `ms < 0.0` are everyday idioms, so a text search would flag unrelated code. The fix is one shared range value, which a test (not a scan) can pin.

### M48. Is Star Power running at this moment?
Kind: K1 (same idea, same rule, two spellings; the tables agree on every input including both window edges). Fix: code-only, and low value.
Homes: `running_activation`, app/preview_view.cpp 241-247 (now, each activation's `sp_window()`; test `struck_at(now, s) && now < e`); `SpanSweep::at` over `sp_active_`, render/track_state.cpp 78-96 (a row's time, the same `sp_window()` filled at 262-266), read only through `toggle_on_after` (render/track_state.h 31-33) in `walk_window` (track_state.cpp 188-209) and `make_toggle_bounds` for the floor tint (render/highway_draw.cpp 253-276) and in the end marks.
Tables (window [s, e)):
| input | boxes | highway |
| --- | --- | --- |
| now < s | not running | Empty, off after |
| now = s | running (`s <= now`) | Start, on after |
| s < now < e | running | On, on after |
| now = e | not running (`now < e` false) | End, off after, end mark drawn |
| back-to-back windows at b | second one runs | Restart, on after |
| no stored end, or e <= s | `sp_window()` empty, never | no interval, nothing tinted |
Both sides get the window from `PreviewActivation::sp_window()` (preview_view.h 114-117), so the edges have one owner. The only thing spelled twice is "start inclusive, end exclusive". One difference of kind: the highway works in seconds (`s_of(ms)`), the boxes in ms; the same value goes through a division, so the equal-at-the-edge test could differ by rounding only if `s_of` were not exact on the stored ms. Not reachable with the current `s_of`.
Overlapping windows: the sweep calls an edge inside another window On; `running_activation` returns the first match. A real path cannot overlap two activations (G6b read this from code; I did not trace it further).
Disagree on: none today; a change of `struck_at` to a strict `<`, or of `toggle_on_after` to leave Start out, would split them.
Owner proposed: `sp_window()` stays the owner of the window; a one-line `sp_window_contains(now)` beside it would own the half-open rule for the boxes. The highway keeps its sweep (it answers a per-row drawing question). Not worth doing unless one of the two is touched anyway.
Evidence: by reading.
Found by: G6a-2, also seen in G6b (G6a-2); verified by V6.
Scan row or reason: rules/app.json already has two rows that catch a hand-written copy: the `sp_window` row (pattern `has_sp_end && (struck_at\(now, a\.ms\)|a\.sp_end_ms > a\.ms)`) and the `struck_at` row. A third spelling of "now inside the window" with a different shape would slip past both.

### M49. When does the last note start?
Kind: K1 for the loop itself (two spellings of "the last chord that has a note, its onset"), but the two homes are not always asked of the same chart, so they are partly different questions. Fix: code-only for the merge; the comment fix is free.
Homes: `last_drawn_note` and `last_note_ms`, app/preview_view.cpp 251-258 (the scene's notes, built from `song.sequence` at the selected difficulty, Pro Drums and 2x Bass; takes `notes.back()`; 0.0 when empty); `last_note_start_ms`, app/song_length.cpp 20-26 (a Song, scanned from the end for the first timestamp with `chord.count() > 0`; nullopt when none).
Tables (same Song):
| input | scene | song_length |
| --- | --- | --- |
| no notes | nullptr, 0.0 | nullopt |
| last timestamp holds a chord | its ms | its ms |
| trailing timestamps with empty chords | skipped (no note drawn for them) | skipped (`count() > 0`) |
| chord with several lanes | the shared ms | the shared ms |
| empty chord because 2x Bass is off and the chord held only a 2x kick (`Chord::apply_2x_bass` resets the kick, model.cpp 280-285) | skipped | skipped |
`Chord::notes()` yields one entry per occupied slot and `count()` counts the same slots (model.cpp 175-185), so "has a note" and "is drawn" cannot differ. Agree on every input for one Song.
Where they are really asked of different Songs: `chart_song_length_ms` hands `last_note_start_ms` the Song as is only when the chart is Expert with 2x Bass on; otherwise it re-parses Expert with 2x (song_length.cpp 47-58). The scene is at the user's difficulty. So on Hard, or with 2x Bass off, the transport's last note (`last_note_ms(scene)`, used for the playback range) and the song length's backup (`last_note_start_ms`) are different numbers on purpose (D75: the song length is the whole chart's, not the drawn mode's). The comment in song_length.h 38 ("It agrees with last_note_ms of the Preview's scene") is therefore true only for Expert with 2x Bass on.
Disagree on: none for one Song; by design for different difficulty or 2x settings.
Owner proposed: `last_note_start_ms(song)` stays the owner of "the last note's onset in a Song". `build_preview_base` could store `scene.last_note_ms` from it so `last_note_ms(scene)` stops walking `scene.notes`. This is a three-line saving; the scan row and a test already pin the agreement (tests/test_preview_scene.cpp 100). Leave unless touched.
Evidence: by reading. A test already pins one chart (tests/test_preview_scene.cpp 100, `last_note_start_ms(song) == last_note_ms(scene)`).
Found by: G6a-4 (known, line-level per the scout; the derivation audit's line 232 names it); verified by V6. I did not open the derivation audit, so "still open" here means the two spellings still exist in the code.
Scan row or reason: rules/app.json has a row for the tests ("When is the song's last note? (tests)", pattern `sequence\.back\(\)\.timecode\.ms\(\)`). For src, a row on `chord\.count\(\) > 0` inside a reverse loop over `sequence` would match the song_length spelling only; text search cannot catch a third way of writing it.

### M50. What "the end of the song" is, and which end each label shows
Kind: K2 (same word, different rules, in front of the user). Fix: needs the user (display text, and a choice between behaviours).
Homes (the ends, and who shows them):
- Playback end L = max(last drawn note, audio end in chart time), `PreviewTransport::update_length_locked`, ui/preview_transport.cpp 26-32. Shown by the clock text "m:ss / m:ss" (`build_time_box`, preview_view.cpp 505) and by the "<position> of <length>" measure line (`box.length`, preview_view.cpp 510-512, drawn at ui/preview_tab.cpp 567).
- Song length S = `app::song_length_ms`, app/song_length.cpp 28-40: the metadata length moved into chart time by the delay or Offset, else the last Expert-with-2x note. Shown as the scrubber's right edge and the gold marks (`scrub_end_ms`, preview_view.cpp 655-658; preview_controller.cpp 403), and as the end measure label on the Paths timeline (`view.timeline_end`, app/path_view.cpp 408-411, drawn at ui/paths_tab.cpp 268-269). It also closes the SP meter curve (`close_curve(... scene.song_length_ms ...)`, preview_view.cpp 95 and 203).
- Audio end A = `audio_end_chart_ms` (audio/frames.h 49-55). Ends the beat grid (preview_view.cpp 320-321) and feeds L.
Table, one chart, by case:
| case | time box clock and "of" measure | scrubber right edge | Paths end measure | beat grid ends |
| --- | --- | --- | --- | --- |
| song.ini states a length S, audio longer | L = A | S | S | A |
| song.ini states S longer than audio and notes | L = max(note, A) | S | S | A (or 2 measures past the last note without audio) |
| no stated length, Expert with 2x On | max(last note, A) | S = last Expert-2x note start (equal to the last note) | same S | A |
| no stated length, Hard or 2x Off | max(drawn last note, A) | S = last Expert-2x note start (may be later than the drawn last note) | same S | A |
| no notes, no audio, no stated length | 0 | falls back to L = 0 | no timeline (nothing shown) | none |
So for the same chart the time box says "ends at X" and the Paths tab says "ends at Y" whenever the audio outlasts the song's stated length, or the Preview mode differs from Expert with 2x Bass. The scrubber sits in between: it follows S but cannot always be dragged there (M47).
Disagree on: any chart whose audio outlasts its stated or last-note length, or whose Preview mode drops notes.
Owner proposed: there is no merge. These are three different quantities (what plays, what the chart says the song is, where the audio stops), each with one owner already (`update_length_locked`, `song_length_ms`, `audio_end_chart_ms`). The finding is the label: if both "of <measure>" and the Paths end are meant to read as "the song's end", one should say "playback ends" or the other "song length". That is a wording question, and per D70, D75 and D48 Q25 the split is deliberate, so the question for the user is only whether the words should tell the two apart. I did not read the Paths tab's surrounding heading text beyond `timeline_end`.
Evidence: by reading. A uitest script on a `testdata/input` chart with a stated length shorter than its audio would show both numbers side by side.
Found by: G6a-5, also seen in G8 (G6a-5); verified by V6. The G8 aside is also confirmed: `mix_song_stems` guards `length_frames() > front_pad` (audio/song_audio.cpp 79) before asking `audio_end_chart_ms`; the bare function on a pad-only mix would return the pad's length (frames.h only rejects length <= 0). Agrees on every input the Preview passes.
Scan row or reason: no text search can catch it. The risk is a new screen computing "the end" from a different quantity; the three owners' names (`song_length_ms`, `audio_end_chart_ms`, `transport length_ms`) are already single-owner by symbol.

### M51. Is this length one the timeline can use?
Kind: K1 for two spellings; the rest is K0 (a different quantity). Fix: code-only.
Homes: `has_song_length`, app/preview_view.cpp 638 (`length_ms > 0.0`), called from `stated_length_ms` and `song_length_ms` (song_length.cpp 12, 38), `song_fraction` (641), `scrub_end_ms` (656), `scrub_thumb_ms` (663), path_view.cpp 408. Other spellings of "above zero": `length_ms_ > 0.0` in `PreviewTransport::seek_ms` (preview_transport.cpp 79) and `tick` (102); `len_s > 0.0f` in ui/preview_tab.cpp 497.
Tables: length < 0, = 0, > 0 give unusable, unusable, usable in every spelling. NaN: `has_song_length(NaN)` is false; `shown_length(NaN)` returns NaN (its test is `< 0.0`). NaN cannot arrive: stated lengths come through `parse_finite_number`, and L is a `max` of finite values. Infinity is excluded by the same parser.
- The transport's `L > 0` answers "is there a playback range to stop at", not "is a song length usable", so it is a different question on a different quantity. K0.
- The tab's `len_s > 0.0f` is `has_song_length(scrub_len_ms)` re-typed on a float: the tab already calls `scrub_thumb_ms` on the same `scrub_len_ms` two lines earlier, which asks `has_song_length` of it. This is the one true second spelling. A positive value under about 1e-45 ms underflows to 0 in float and would split them; no real length does that.
Disagree on: none today; they split if "usable" ever gets a floor (for example one second).
Owner proposed: `has_song_length` stays. The tab asks a new `scrub_range_seconds` (or passes `has_song_length(scrub_len_ms)`) rather than `len_s > 0.0f`. The `1.0f` fallback is an ImGui guard (a slider cannot have an empty range) and stays in the tab.
Evidence: by reading.
Found by: G6a-6 (the scout marked it known, line-level); verified by V6. Still open in the code: the tab spelling exists.
Scan row or reason: a row on `len_s > 0\.0f` or `length_ms_? > 0\.0` outside the owner would flag the tab's spelling, but the transport's spelling is legitimate, so the row would need `scope` on ui/preview_tab.cpp only.

### M52. Does the drawn path have activations?
Kind: K1 (derived predicate: "no marks" stands in for "no activations"). Fix: code-only.
Homes: `no_acts = scrub_marks.empty()`, ui/preview_tab.cpp 428-429 (greys "< Act" and "Act >"); `scene_.activations`, read by `jump_activation` and `seek_activation` (ui/preview_controller.cpp 489-502) and `activation_jump_ms` (preview_view.cpp 671-681); the `[` and `]` keys at preview_tab.cpp 532-533 are not gated at all.
Tables:
| input | buttons | jump / keys |
| --- | --- | --- |
| no path, or a path with no activations | greyed (no marks) | `activation_jump_ms` returns nothing |
| activations, scrub edge E > 0 | enabled (`build_scrub_marks` gives one mark per activation, because `song_fraction` clamps instead of dropping) | work |
| activations, E <= 0 | greyed (no marks) | work |
| a scene still loading (`job_` set) | enabled if the old scene had marks | `jump_activation` returns false with no effect |
The E <= 0 row needs activations and a playback range of zero together; activations come from notes, so L >= the last note, which is above 0 unless every note sits at 0 ms. I could not construct it either. Marks and activations are the same count whenever E > 0.
Disagree on: none today; they split if `song_fraction` ever drops a mark, or if marks start to be limited to a sub-range.
Owner proposed: a `PreviewController::has_activations()` that reads `scene_.activations` (and is false while `job_` is set), and the tab asks it instead of the mark list. Greys exactly when `jump_activation` would do nothing. The loading row changes behaviour slightly (buttons grey during a load, where today they are live and do nothing), which a user can see, but it only removes a dead click; I would call it code-only with that noted.
Evidence: by reading.
Found by: G6a-7; verified by V6.
Scan row or reason: no text search can catch it; the copy is a different variable standing in for the fact.

### M53. Where does each text box sit on the Preview image?
Kind: K1 inside one function, not across modules. Fix: code-only.
Homes: the fit pass, ui/preview_tab.cpp 591-648 (`fit.left_h`, `fit.right_top = v_margin`, `next_top_at`); the draw pass, 660-812 (score box at `box_max.y + sz.gap`, drain box at `origin.y + v_margin`, next box at `origin.y + next_top_at(sz)`); `render::OverlayBoxes` (render/overlay_layout.h 52-63) with `overlay_scale` and `fit_right` (render/overlay_layout.cpp 53-63), which take the tops as inputs and decide no order.
Tables (fit vs draw):
| input | fit | draw |
| --- | --- | --- |
| score shown | left height = time + gap + score | score top = time bottom + gap |
| drain shown | right top = `v_margin` | top = `origin.y + v_margin` |
| drain hidden | next top = `v_margin` | same lambda |
| right edge | `gauge_left - d_gap - origin.x` (602) | next and drain right edges from the same `gauge_left` and `right_edge` |
`v_margin` is one variable used in both passes, so "the number is typed in both spots" (G6a) is not true; the shared thing is the variable. What is written twice is the stacking order (score under time; next under drain), once as height sums in the fit pass and once as tops in the draw pass. `overlay_layout.h` repeats the order only in comments ("score box under it", "under the drain box"), which is a comment echo, not a rule.
Disagree on: none today; reordering the boxes needs edits in both passes of one function.
Owner proposed: have `overlay_box_sizes` or a small `overlay_tops(sizes, drain_drawn, score_shown)` return the tops once; both passes read them. It would also let the comments in overlay_layout.h point at a function. Optional.
Evidence: by reading. The uitest or the real-app check would show any move (the user's top rule: see a layout change in the real app).
Found by: G6a-8, also seen in G6b (G6a-8); verified by V6.
Scan row or reason: no text search can catch it; the positions are arithmetic on local variables.

### M54. Has the chart file changed since its record was analyzed?
Kind: K3 for the shared rule (one owner: `app::chart_changed_since`, app/analysis.cpp 411-417, with `sig_unchanged` at 253-255 and `store::sig_can_show_unchanged`, store/record_store.h 294). The part after it is per-caller outcome handling, with a two-line K1 tail (the md5 comparison typed twice). Fix: code-only if the tail is merged; the two outcomes themselves are a choice, so leave them.
Homes: `PreviewLoadJob::run`, ui/preview_load_job.cpp 186-190 (entry md5 and sig; answer `now && !now->md5.empty() && now->md5 != entry_.md5`, drops the drawn path through `drawn_path`); `ViewJob::run`, ui/library_jobs.cpp 504-513 (row md5 and sig; throws on an empty md5; sets `files_changed` on `now->md5 != song_.md5 || store::sig_can_show_unchanged(now->sig)`).
Table (both call `chart_changed_since(notespath, stored_sig)` first; "hash" means `hash_chart_file`, which gives lowercase hex, so a case-variant md5 is not an input: the row's md5 is the scan's, same spelling):
| input | Preview (`chart_changed`) | Click (`ViewJob`) |
| --- | --- | --- |
| stored sig non-empty and equal to the sig now | not changed; nothing hashed | no change; nothing saved |
| edited file, different md5 | changed; path not drawn; warning "This chart changed since it was analyzed. Click the song again..." (preview_tab.cpp 397-401) | `files_changed`; new md5 and sig saved (`reidentify_chart`, app_state.cpp 379-401) and the row updates |
| unreadable or gone file (md5 empty) | not changed (the parse error speaks) | the job fails: "could not read <path>" |
| rewritten file, same content, sig differs | not changed (md5 equal) | `files_changed` = true when the new sig can show unchanged (non-empty): bookkeeping, new sig saved, md5 stays |
| stored sig empty (cannot show unchanged), same md5 | not changed | `files_changed` only if the sig now is non-empty |
| folder chart with no song.ini (sig now empty), same md5 | not changed | not changed (neither condition holds) |
Disagree on: the unreadable file (Preview says not changed; click throws), and the rewritten file with identical content (Preview not changed; click `files_changed`). Both are defensible per caller: the Preview cannot fail and has no row to update; the click owns the row. The header comment on `ChartNow` (analysis.h 125-126, "an empty md5 means the file is unreadable, which is no change") describes only the Preview's reading; the click treats the same value as an error, so that sentence is misleading for the second caller.
Where the work is done twice: the click refreshes the selected row's md5 and sig (app_state.cpp 396-399), so a Preview opened after a click normally takes the no-change shortcut; the Preview's own hash only runs when the file changed after the click, or when the Preview is opened without that refresh. I did not trace every entry route to the Preview, so "hashed twice" is possible but not the common case.
Owner proposed: `chart_changed_since` stays. Add `ChartNow::differs_from(md5)` (true when `md5` is non-empty and unequal) for both callers' comparison, and fix the header comment to say what each caller does with an empty md5. No change of behaviour.
Evidence: by reading; the order of row updates would be proven by a uitest script that edits a chart in a temp copy of `testdata/input`, clicks it, then opens the Preview.
Found by: G6b-1; verified by V6.
Scan row or reason: rules/app.json (the `hash_chart_file|chart_files_unchanged|chart_files_sig` row, near line 126) already flags a third caller hashing a chart on its own. A row on `->md5 != ` would be too broad to be useful.

### M55. Two ways to assemble a Preview scene and its highway timeline
Kind: K3 (parallel on purpose: the split build exists so a path change rebuilds only the overlay), with a two-line K1 inside. Fix: code-only.
Homes: `build_scene_and_highway`, ui/preview_load_job.cpp 54-66 (`app::build_preview_scene` = base + overlay, then `render::build_track_state`; used by `PreviewLoadJob::run` and `PreviewNotesJob::run`); `build_scene_base` (348-358) then `PreviewSceneJob::run` (398-419: `apply_preview_overlay` and `render::rebuild_overlay_fields` on copies of the base); `PreviewRenderer::set_scene(scene, opts)`, render/preview_renderer.cpp 416-418, a fallback that calls `build_track_state` on the render thread when no matching timeline was built (preview_controller.cpp 353-362).
Table (same song, path, cap, rules, Pro setting): all three give the same `TrackState`. The path-free half is `build_preview_base` + `build_track_state` in all of them; the overlay half is `apply_preview_overlay` + `set_overlay_intervals` + `merge_overlay_edges` in all of them. The rule "which fields are overlay" lives once, in `TrackState::set_overlay_intervals` (track_state.cpp 249-277). Equality is pinned by tests/test_track_state.cpp (the scout cites lines 845 and 907; I did not open them).
The two-line K1: `build_track_state` ends with `st.set_overlay_intervals(scene); st.merge_overlay_edges();` (track_state.cpp 356-357), which is the whole body of `rebuild_overlay_fields` (399-402).
Disagree on: none today; they split if a span field is added to one half and not the other.
Owner proposed: `build_track_state` ends with `rebuild_overlay_fields(st, scene)` (needs a forward use; both are in the same file). The three-way assembly stays. The struct pair `SceneAndHighway` (preview_load_job.cpp 44-48) and `PreviewSceneBase` hold the same three fields; not worth merging.
Evidence: by reading. rules/ui.json already has a row on `app::build_preview_scene(` that makes `build_scene_and_highway` the one caller.
Found by: G6b-2; verified by V6.
Scan row or reason: the existing ui.json row (pattern `\bapp::build_preview_scene\(`) covers the combined build. The two-line tail would need a row on `set_overlay_intervals\(scene\)` outside track_state.cpp, but it is a private member (friend-only), so a new caller cannot be written elsewhere.

### M56. Is `PreviewNote::solo` a second home of "is this note in a solo", and is it read at all?
Kind: K1 (a dead copy, not a live second rule). Fix: code-only (no word, no stored field, no score or count changes).
Homes: `PreviewNote::solo = ts.flag_solo`, app/preview_view.cpp 50 (declared at preview_view.h 56 with the comment "falls inside a solo section"); `Song::solo_sections` from `find_solo_sections` (parse/song.cpp 88-98, called at 1219 and 1875), which becomes `scene.solos` (preview_view.cpp 290-294) and then the track state's `solo_` spans (track_state.cpp 288-289) and `TrackInstant::solo` (track_state.cpp 230, read at highway_draw.cpp 253).
Reader check: `PreviewNote::solo` has no reader in src/. I searched `.solo`, `->solo` and `solo =` across src/ and every hit is a different `solo` (`TrackInstant::solo`, `ReplayScore::solo`, `CategoryScores::solo`, `row.solo`, `score_solo`); the only write is preview_view.cpp 50. `PreviewNote` is built only in `note_from`, and the only consumers of `scene.notes` in src are `gem_of` (track_state.cpp 37-48, which copies lane, cymbal, ghost, accent, 2x kick but not solo) and `last_drawn_note`. Readers in tests only: tests/test_preview_scene.cpp 49, 55, 61, 140-141 (asserts the flag) and 418 (a field-by-field note comparison in a base-versus-full-build test).
Tables: a note whose timestamp has `flag_solo`: `PreviewNote::solo` true, and its tick lies inside a `solo_sections` span, because the sections are built from the same flags. A note without the flag: false, outside every span. With `song.solo_sections` cleared by hand, the note still says true but there is no span (tests/test_preview_scene.cpp 134-141 pins exactly that). So the field is the raw flag; the spans are the grouping. They cannot disagree for a parsed Song.
Side find: `note_in_span` (render/track_state.cpp 404-407, track_state.h 266) answers "is this note inside this span" and has no caller in src/; only tests/test_track_state.cpp 521-526 calls it.
Disagree on: none today.
Owner proposed: `ts.flag_solo` (parse/song.h 146) is the one source; `solo_sections` is its grouping. Drop `PreviewNote::solo` and the three assertions and the one comparison term in the tests. Nothing in the app changes.
Evidence: by reading; the grep above is a search, so a build would confirm (compile with the field removed). Note 9: a grep that misses X is not X absent, so the compile is the proof.
Found by: G6b-3; verified by V6.
Scan row or reason: a row on `\.solo\s*=\s*ts\.flag_solo` in src/app would catch the field coming back. A better guard is the compiler: once the field is gone, a new reader cannot exist.

### Side notes
- tests/test_preview_transport.cpp 282-294 is named "a drag to the right end seeks to the song's length" but passes the audio end as the song length, so it never reaches the case where the stated length is longer than the playback range (M47 edge 2).
- app/analysis.h 125-126: the `ChartNow` comment says an empty md5 "is no change", but `ViewJob::run` (ui/library_jobs.cpp 506-507) throws on it. Wording only (M54).
- app/preview_view.h 449-450 (G6a): the comment says activations are "those carrying a timecode" while the code at preview_view.cpp 366 filters nothing. I did not re-read it.
- `note_in_span` (render/track_state.cpp 404) is used only by a test (M56).
- The scout's hand-off that `rules` is held by four objects and keyed by none (G6a-3 and G6b) was outside this batch (no M number); not verified here.

---

## V7 findings: parsing and chart files (M57 to M66)

Verifier: concept-audit-V7 (agent id used for status lines: `V7` at first, then `ac223a5f1c822a496` once the progress hook named it). Everything below is by reading. No build, no test, no run.

Short version: nothing here is a score bug. M61 is the one real K2 (two answers to "which meter is in force", both kept on purpose or by decision). M66 is the one a user could meet (a loose stem another program is writing is silently left out of the Preview). M57, M59, M62, M63 and M65 are small, safe, code-only tidy-ups or K3 with a line-level piece. M58 and M64 are K3: one real decider with cheap pre-filters in front.

### M57. In what order does the game list a drum pad's lane bits?
Kind: K1 (two spellings of one game fact; they agree on every input a parser can produce). Fix: code-only.
Homes: `lane_bit_rank`, src/parse/song.cpp 112-116 (a note's colour step plus a cymbal flag; read by `lane_pads` 118-135);
       `lane_bit`, src/parse/note_shuffle.cpp 24-33 (colour plus cymbal flag to the game's bit value). Two more spellings in the same file: `moved_to` 37-51 (bit back to colour and flag) and `pads` 55-58 (cymbal bit = tom bit shifted left by 3).
Tables: rank in song.cpp, low to high: Red 0, Yellow tom 1, Blue tom 2, Green tom 3, Yellow cymbal 4, Blue cymbal 5, Green cymbal 6. Bit value in note_shuffle.cpp: Red 0x02, Y/B/G toms 0x04, 0x08, 0x10, Y/B/G cymbals 0x20, 0x40, 0x80. Reading the bits low to high gives the same order as the ranks. Kick: `lane_pads` skips it (line 122); `lane_bit` gives 0x01. Empty chord: `lane_pads` returns count 0 and the caller ignores lowest/highest. Odd input: a Red note with a cymbal flag ranks 3 (ties Green tom) while `lane_bit` ignores the flag and gives 0x02. That input cannot be made: `set_lane_flag` throws for Red (model.cpp 95-101) and both parsers only flag Yellow, Blue or Green (`allows_cymbals`).
Disagree on: none today. A correction to the game's lane order, or a new pad, would have to be made twice (a rank formula and a bit table). The two files cite different sources (song.cpp "the game's lane builder", note_shuffle.cpp "the reference's docstring"), so the two copies could drift for lack of anyone seeing both.
Owner proposed: `lane_bit` stays (it holds the game's actual numbers, and the shuffle needs them). Expose it from the note model next to `allows_cymbals`, and let `lane_bit_rank` be the position of the lowest set bit of `lane_bit(note)`. Then the order is read off the bit table and cannot split from it.
Evidence: by reading, proven by a test case that loops the seven pads and checks the rank order equals the bit-value order (it would also pin the order before the merge).
Found by: G7a-1; verified by V7.
Scan row or reason: weak. A row on the hex lane literals `0x20`, `0x40`, `0x80` beside a `colortype` or `NoteColor::` in src/parse outside note_shuffle.cpp would flag a third bit table. A rank formula built from `static_cast<int>(note.colortype)` has no stable spelling, so a text search cannot catch a second rank formula.

### M58. When is a drum note a cymbal, and what is one drum note to each layer?
Kind: K3 for the layer enums (`NoteColor`/`PreviewLane`/`Pad`, each conversion kept on purpose, comment at preview_view.h 35-36), plus one K1 rule inside: "with Pro Drums off a pad is a plain pad" is applied four times, and the "only Yellow, Blue and Green carry a cymbal" test is spelled twice more in the `.chart` parser. All copies agree today. Fix: code-only (and optional).
Homes: `MidiParser::op_note`, src/parse/song.cpp 903-911 (colour, Pro setting, last tom marker); `ChartParser::note_optype`, 1714-1722 (N 66, 67, 68 gated by `mode_pro_`, mapped by hand to Yellow, Blue, Green); `allows_cymbals`, core/model.cpp 17-20 (the owner of which colours can carry a cymbal, used at 63, 92 and song.cpp 909);
       `gem_of`, render/track_state.cpp 37-48 (`pro && n.cymbal && allows_cymbals`); `note_label`, core/model.cpp 60-66 (text only); Dynamics tab, ui/dynamics_tab.cpp 172 (hides the three cymbal rows when Pro is off).
Tables, Pro Drums off, a note written as a Yellow cymbal: parser (.mid or .chart) stores a plain Yellow note; `gem_of` draws a tom (its `pro &&` is a second gate on a flag that is already clear); `note_label` reads "Yellow"; Dynamics tab hides the cymbal rows (they would all be zero anyway). Pro Drums on: all four keep the cymbal. A Red note with a cymbal marker: `.mid` never sets it (`allows_cymbals(Red)` is false); `.chart` has no Red cymbal number, so it cannot arrive. Kick: never.
Which Song the Dynamics tab reads (the scout's open question): `dynamics_for_settings` / `load_dynamics_song`, app/dynamics_breakdown.cpp 135-157, parses with the same `pro` as the setting (`view_prodrums`). So with Pro off its Song has no cymbals, and hiding the rows is purely a presentation choice, not a second data rule.
Is `gem_of`'s gate live? Its `pro` comes from `track_options(pro)` (ui/preview_load_job.h 42-46), built from the same key as the parse. No path feeds a Pro-on Song to a Pro-off renderer today, so the gate is redundant, not different.
Disagree on: none today. It would split if the Preview could show a Song parsed with Pro on in a view with Pro off (for example a cached Song reused across a Pro toggle), because only `gem_of` would then hide the cymbals.
Owner proposed: the parser keeps the rule (it already writes the flag only when allowed). In the `.chart` parser, state "Y/B/G" once by asking `allows_cymbals` instead of three hand cases. Leave `gem_of`'s gate: it states what the renderer option means (finding R7.16).
Evidence: by reading.
Found by: G7a-2, C1-7, also-seen G7a-2 in G6b; verified by V7.
Scan row or reason: none worth adding. The three hand cases have no stable spelling (`case 66:`/`67`/`68` beside `NoteColor::Yellow`). The Pro gate is a boolean test and a text search cannot tell it from any other use of `pro`.

### M59. What happens to one tick's events when one is malformed, and where does the 2x bass step sit?
Kind: K3 (the phase order is a fact about each file format) with a K1 line-level piece (the per-op `catch` and the `dynamics_enabled_ ? dyn : Normal` test). Fix: code-only, optional.
Homes: `MidiParser::run_ops` and `push_timestamp`, src/parse/song.cpp 1095-1136; `ChartParser::push_timestamp`, 1746-1784; the one owner of the 2x bass rule itself is `Chord::apply_2x_bass` (core/model.cpp).
Tables: a malformed op (duplicate note, marker with no note under it throws `ChartFileError`): .mid skips that op and runs the rest of the tick; .chart does the same inside `run_phase`. Time ops (tempo, meter) are not wrapped in either (1161, 1827). Reader failures (`MidiError`, SNG/SRB throws) are `KindedError(ChartUnreadable)`, not `ChartFileError`, so neither catch can swallow them. 2x bass step: .mid after Notes, before emit; .chart after Notes, before NoteMods (accent/ghost/cymbal markers). Phase lists differ (.mid has PreDelayed, .chart has NoteMods and Pre) because the file grammars differ. The .mid wrapper `apply_2x_bass(tick)` also corrects the "marks before the dynamics tag" count when the kick is removed (926-931); that is .mid-only.
Disagree on: none today. They would split if one parser's phase list changed so the 2x step moved relative to the markers. Today no marker can target a kick in .chart (N 34-43 are pads only), so the position of the step does not change any answer.
Owner proposed: no change needed for the rule (single owner already). If wanted, a tiny shared helper for "run one op, skipping a ChartFileError" and for the dynamics-in-force test.
Evidence: by reading.
Found by: G7a-3, also-seen G7a-3 in G7b; verified by V7.
Scan row or reason: none. Both are three-line loops; the phase order has no text a search can pin.

### M60. What does the library show for a song with no title, artist or charter?
Kind: K2 (same idea, different words per field, a stored value and a shown value). It is a recorded decision, not an accident. Fix: needs the user (display text and stored record) if anyone wants it changed; no code fix is proposed.
Homes: `title_or_unknown` + `name_or_unknown`, src/parse/song.cpp 452-463 (stored title); `artist_or_unknown` 550-552 and `charter_or_unknown` 554-556 (stored artist and charter, applied once at app/analysis.cpp 728-730); `display_title` 535-537, `display_artist` 539-542, `display_charter` 544 (what screens show; all callers go through these: library table, details panel, three reports, batch, job status).
Tables, stored value: empty title or the old "<unknown title>" -> "(unknown)"; empty artist -> "<unknown artist>"; empty charter -> "<unknown charter>"; a whitespace-only title stays as typed (the stored helper does not trim).
Shown value: title empty, only tags or only spaces -> "(unknown)"; artist empty, only tags/spaces, or the stored "<unknown artist>" -> "(unknown)"; charter empty or only tags/spaces -> blank; the stored "<unknown charter>" -> shown as written. Details panel (ui/details_panel.cpp 72-74): blank charter adds nothing; the stored placeholder reads "charted by <unknown charter>".
Disagree on: a charter with nothing in it. Artist and title read "(unknown)"; a charter that is only tags reads blank, while one that the scan stored as blank reads "<unknown charter>". The same song can therefore show "<unknown charter>" in the library and a blank charter where a report takes the name from the leaderboard board instead (dm_report.cpp 100-103). song.h 122-137 says this is deliberate, and `fix-decisions` D27 says: "blank artist or charter shows the placeholder through one helper next to `title_or_unknown`", with D48/D50/D56 for the shown rules.
Owner proposed: none; the owners are already single (`display_*` for the screen, `*_or_unknown` for the store). The question for the user, if raised, is only whether a charter should read "(unknown)" like the other two.
Evidence: by reading.
Found by: G7a-4; verified by V7.
Scan row or reason: the placeholder literals are already one spelling each (`kUnknownTitle` etc. in song.h). A row on the quoted strings `"<unknown charter>"`/`"<unknown artist>"`/`"(unknown)"` outside src/parse/song.* would flag a hand-typed copy; I did not find one in src.

### M61. Which tempo and meter section is in force at a tick?
Kind: K2 for the meter side (two answers to one question at a tick exactly on a change); K1 for the tempo side (agree everywhere). Finding 53 is a known K2 that is still open in the code. Fix: needs the user (a score or count moves for finding 53; display text changes for the label).

**Finding 53, `check_activations`' own walk: confirmed still open, and recorded as kept.** The walk is unchanged at song.cpp 593-617 (`tpm_pos`, advances one meter change per chord, writes `pre_tpm`, used at 636 and 652-653 for the generated-fill length). The dead tempo walk (`bpm_pos`, 599 and 618, value never read) is still there too. The user decided to keep today's rule: `fix-decisions` D26 ("findings 53, 314, open half of 315 ... keep today's rule, written down as unverified against Clone Hero"), written in docs/adr/0023-chart-rules-follow-clone-hero-code.md, "Not verified against Clone Hero" (line 72 on). I re-read the walk against `section_at`: with two or more meter changes between two chords the walk is one change behind; with none, or one (even on the chord's own tick, where both take the old meter), they agree. The dead `bpm_pos` walk can go with no effect (code-only on its own).

**Homes for the Preview label question:**
`MeasureIndex::section_at`, core/timing.cpp 109-114 (`lower_bound`, so a tick exactly on a change belongs to the section before it; used by `Timecode`, `measures_at_tick_f`, `ms_per_measure_at` through tick+1, and `build_beat_events`' grid);
`build_time_box`, app/preview_view.cpp 531-535 (`upper_bound` over `scene.time_sigs`, last entry at or before the tick, so a tick exactly on a change takes the new entry). The sigs come from `song.timesig_changes`, written together with `tpm_changes` in `apply_timesig` (song.cpp 316-325), so the two maps cannot differ in keys.
The label's tick is `display_tick_at_ms(now)`, a rounded whole tick (timing.cpp 245-248), and the position text next to it comes from the same tick through `Timecode`.

**Both tables, resolution 480 (4/4 is 1920 ticks per measure):**

Change on a bar line (4/4 to 3/4 at tick 3840, a barline):
| Tick | `section_at` / position text | Label at that tick |
|---|---|---|
| 3839 | old section, m2.4.479 (4/4) | 4/4 |
| 3840 | old section, remaining 0 after 2 whole measures: m3.1.0 | 3/4 |
| 3841 | new section, m3.1.1 | 3/4 |
Position text is the same whichever section is used (the remainder is 0), and the label at 3840 is the right one for a measure that starts there. They agree. The label never shows a wrong meter.

Change in the middle of a measure, to a longer or equal remainder (4/4 to 3/4 at tick 3000; the barline before it is 1920):
| Tick | Position text (old section until 3000 inclusive) | Label |
|---|---|---|
| 3000 | m2.3.120 (old section: 1 whole measure, remainder 1080) | 3/4 |
| 3001 | m2.3.121 (new section: starts_ is 1920, 0 whole 3/4 measures, remainder 1081) | 3/4 |
Same text either way (the new section counts from the old barline, `MeasureIndex` ctor timing.cpp 97-106). Only `measures_decimal` differs at 3000 (old 1.5625, new 1.75), and the label does not use it.

Change in the middle of a measure, to a shorter meter than the remainder (4/4 to 2/4 at tick 3120; barline 1920; remainder 1200 is more than a 2/4 measure of 960):
| Tick | Position text | Label |
|---|---|---|
| 3119 | m2.3.239 (old) | 4/4 |
| 3120 | m2.3.240 (old section, `lower_bound` rule) | 2/4 |
| 3121 | m3.1.241 (new section: one whole 2/4 measure fits) | 2/4 |
So at the one tick 3120 the label reads "2/4" beside a position "m2.3.240", a third beat that a 2/4 measure does not have. That is the only case where the two disagree, and it is one rounded tick (about 1 ms at 120 BPM). It shows when the playhead is paused or scrubbed to the change tick, or a note sits exactly there. The label is not wrong by its own rule (the meter that starts at that tick); the position is the one counted by the engine's rule (D51 call 22). `build_beat_events` (preview_view.cpp 446-479) draws its bar lines from the same `MeasureIndex` sections, so no bar line is drawn at the jump; the measure number moves from 2 to 3 between ticks 3120 and 3121 with no line. That is cosmetic and applies only to a chart with a mid-measure change into a shorter meter. `check_timing_maps` (song.cpp 29-53) does not refuse a mid-measure change.

`ms_per_measure_at(K)` asks `section_at(K+1)`, so it takes the new meter at K (deliberately, timing.cpp 197-204). Tempo side: `MsIndex::at`, `tps_at`, `ms_at_tick_f` and the Preview's BPM `upper_bound` all take the tempo that starts at the tick; before the first key and negative ticks give the opening tempo in all four. They agree.

Other readers of the same question, from the 2026-10-03 ledger (tempo-4): `measures_at_tick_f`/`tick_at_measures_f` (K3, display only, the header says never to feed the engine; they agree with `section_at` on bar lines and are the only place with a section-entry walk in measure space) and `plusmeasure` (engine; bisect on whole measures). I did not run a mid-measure case through these two; I only read them.
Disagree on: (a) `check_activations`' walk versus `section_at` when two or more meter changes fall between two chords; (b) the label versus the position text on a tick that is exactly a mid-measure meter change to a shorter meter.
Owner proposed: for (a), `MeasureIndex::section_at` via `Song::timing()` as finding 53 said, but only after the user's yes (it moves a fill length). For (b), keep the label's rule and add one comment in `build_time_box` pointing at `section_at`, or leave it; changing the label to take the old meter would show the wrong meter from the second tick on, so it is the position text that is odd, not the label.
Evidence: by reading. A run that would prove (b): a `build_time_box` test case with a 4/4 to 2/4 change at tick 3120 and now_tick 3119, 3120, 3121 (test_preview_boxes.cpp is where the existing cases live). A run that would prove (a): a one-chart `hydra_batch` on a hand-made chart in testdata/input with two meter changes between two chords, before and after.
Found by: G7a-5, G7b-5, also-seen G7a-5 in G7b; verified by V7.
Scan row or reason: rules/core.json already has "Which meter section is a tick measured in?" on `keys_at(...)` orderings. The Preview label's `upper_bound` over `scene.time_sigs` is a different list and would need its own row on `time_sigs.begin(), scene.time_sigs.end()` in src/app; the walk in `check_activations` is spelled `tpm_keys[tpm_pos]` and a row on `tpm_keys\[` outside timing.cpp would flag its return.

### M62. How does Hydra decide a chart's notes format?
Kind: K3 (each of three tests is a fact about its own source: a path's extension, an entry's exact name, an .srb stream's bytes) with a K1 line-level piece (the "MThd" magic spelled twice). Fix: code-only, optional.
Homes: `chart_format_of`, src/parse/chart_files.cpp 9-15 (extension, any case); `notes_file_format` 17-22 (exact "notes.mid" or "notes.chart", any case; used by `pick_notes_file` for a loose folder and a .sng, and by `load_container_srb`); the byte sniff, parse/song.cpp 1988-1995 (4 or more bytes starting "MThd", only for an .srb whose stated name is outside the rule); `walk_chunks`, parse/midi.cpp 169-172 (the same magic as a validity check, with a 14-byte minimum).
Tables: extension: "a.mid"/"A.MID" Mid; "x.chart" Chart; ".sng", ".srb" containers; "notes.txt" and "" None (the loader then throws "unexpected chart type"). Name: "notes.mid"/"Notes.MID" Mid; "notes.chart" Chart; "song.mid", "x.chart", "notes.mid.bak" None. Bytes (.srb, odd name only): 4 or more bytes starting "MThd" Mid reader; 3 bytes, anything else, or empty Chart reader. midi.cpp: fewer than 14 bytes or no "MThd" throws `MidiError` (a 4-13 byte "MThd" stub gets the Mid reader and then the same error the validity check would give).
Disagree on: "song.mid" in a loose folder (path rule says Mid, name rule says None; the scan skips it, which matches Clone Hero reading only notes.*); an .sng with only "chart.chart" (throws "No chart files found in SNG file.", sng.cpp 160) against an .srb whose metadata names "chart.chart" (loads, the bytes decide). Neither is a path a normal library holds. Nothing checks the .sng or .srb file magic; both are chosen by extension only (sng.h and srb.h say the prefixes are never needed).
Owner proposed: no merge of the three tests. Give the "is this a MIDI header" check one function in midi.h (used by the .srb sniff and `walk_chunks`) so the magic has one spelling.
Evidence: by reading.
Found by: G7b-1; verified by V7.
Scan row or reason: rules/parse.json already has "Which chart format is a path?" on `ends_with_ci(... ".sng|.srb|.mid|.chart")`. A second row on the quoted `"MThd"` or `'M' ... 'T' ... 'h' ... 'd'` outside midi.cpp and song.cpp would catch a third header test.

### M63. Is this path a container (.sng or .srb) or a plain chart file?
Kind: K1 (small; the two homes agree on every input). Fix: code-only.
Homes: `chart_kind_of`, app/analysis.cpp 259-265 (`ChartKind` Folder/Sng/Srb, used by the scan, `song_id_read`, `scan_folder` line 486, `hash_chart_file`); `is_container_path`, app/preview_source.cpp 339-343 (Sng or Srb from `chart_format_of`), whose result decides whether a container is read at all (line 347) and so every later `container ? ... : ...` branch at 354, 365, 379; plus direct `ChartFormat::Sng` tests at 368 and 380.
Tables (same input to both): ".sng" container; ".srb" container; ".SNG" container (the path rule ignores case); ".mid", ".chart" not; ".txt" or no extension not a container (the scan calls it a folder chart; the Preview does not read a container and the loader then throws "unexpected chart type"). Empty path not a container.
Disagree on: none today. A third archive format would need an edit in `ChartKind`/`chart_kind_of` and in `is_container_path` (and the two `== Sng` lines), because the list "which formats are containers" is typed in both. `ChartKind` is a coarser enum on purpose (it tells hashing which reader to use), so only the container list is shared.
Owner proposed: add `is_container(ChartFormat)` next to `chart_format_of` in parse/chart_files.h. `is_container_path` calls it, and `chart_kind_of` keeps its two cases for Sng and Srb but gets "is it a container" from the same place.
Evidence: by reading.
Found by: G7b-2, C1-8, C2-4; verified by V7.
Scan row or reason: a row on a line that names both `ChartFormat::Sng` and `ChartFormat::Srb` joined by `||` (or two `ends_with_ci` calls on ".sng" and ".srb"; the second is already caught by "Which chart format is a path?") in src outside chart_files.*. The `switch (chart_format_of(...))` form in `chart_kind_of` is not catchable by text because it spells no pair.

### M64. Is this entry a playable, usable song stem (by name, by bytes, channels and rate)?
Kind: K3 (name test and byte test are different pre-filters; the single final decider is `open_stem_reader`, and a failed open is handled in one place). One line-level K1: the list of five audio formats is spelled in the name test and in the byte sniff. Fix: code-only for the list; the pre-filter split itself would be "needs the user (a choice between behaviours)" if anyone wanted loose files with the wrong extension to play.
Homes: `is_audio_filename`/`is_song_stem`, app/preview_source.cpp 142-151 (extension in .ogg .opus .mp3 .wav .flac, base name not "preview"; used for a loose folder at 160 and for a .sng at 123); `looks_like_audio` over `sniff_format`, preview_source.cpp 153-155 and core/audio_sniff.cpp 49-79 (bytes; used for .srb chain streams at 223 and decrypted blobs at 273); `open_stem_reader`, audio/stem_reader.cpp 11-26 (the decoder's own dispatch on `sniff_format`; throws `AudioDecode` "unrecognized audio container" on Unknown); the reader constructors for channels and rate (G8-5).
Tables: name: "guitar.ogg" stem; "SONG.MP3" stem; "preview.ogg" not (folder and .sng alike); "song.m4a", "song" no; a subfolder named "x.ogg" no. Bytes: RIFF....WAVE Wav; "fLaC" Flac; ID3 tag then fLaC Flac; ID3 then anything else Mp3; FF Ex/Fx sync Mp3; OggS with OpusHead in the first 64 bytes OggOpus; OggS with "vorbis" OggVorbis; OggS with neither Unknown; under 2 bytes Unknown; JPEG Unknown. Channels/rate at open: (0 channels) every reader refuses; (above 0 channels, 0 rate) `MaReader` and `read_all` accept, Vorbis, Mp3 and `StreamMix` refuse (probably unreachable, the decoder fills the rate); (3 or more channels) Opus refuses (opus_reader.cpp 216), the others accept.
**Where the Preview catches a stem that was kept by name but will not decode:** `open_song_stems`, audio/song_audio.cpp 50-62. It wraps `open_stem_reader` in a try; `OpenCancelled` is rethrown, and any other `std::exception` is dropped ("A stem that won't open is skipped", header lines 41-44: one corrupt stem never silences the rest). The only caller is `PreviewLoadJob::open_audio`, ui/preview_load_job.cpp 152. A loose file that will not even map is dropped one step earlier at `map_song_stems`, song_audio.cpp 19-24, and `open_song_stems` skips its empty slot (line 48). Nothing reports it: `audio_warning_` in the Preview controller is only set for an output-device failure (preview_controller.cpp 320). So a ".ogg" holding something that is not Vorbis or Opus, or a stem with the wrong channel count, simply is not in the mix. If every stem is dropped the song plays muted on the chart clock with no message. The loading bar's byte total still counts a mapped stem that later fails to decode (cosmetic).
Disagree on: (a) a ".ogg" entry whose bytes are not Vorbis or Opus: the name test keeps it, the byte test says Unknown, so it is dropped at open. (b) a "preview.wav" inside an .srb: the byte test keeps it, since an .srb entry has no name to test. (c) a file that is audio by bytes but has no audio extension (".bin"): dropped by name. In every case the player ends with the stem absent, so the user-visible outcome is the same, silently.
Owner proposed: `sniff_format` + `open_stem_reader` stays the one decider; the name test is a documented cheap pre-filter. Put the five formats in one table (extension and format side by side) so that adding a format is a one-place edit. Do not make the folder/.sng path sniff bytes unless the user wants it (a choice between behaviours).
Evidence: by reading. A run that would prove the silent drop: a uitest script or test case with a folder chart holding one valid stem and one ".ogg" of garbage, checking the mix holds one reader.
Found by: G7b-3, G8-5; verified by V7.
Scan row or reason: a row on `ends_with_ci(... ".ogg")` / ".opus" / ".mp3" / ".wav" / ".flac" outside preview_source.cpp would flag a second extension list (the existing "Which chart format is a path?" row does not cover audio extensions). The byte-sniff side is already one function; a second sniff would be spelled `"OggS"`, `"fLaC"` or `"RIFF"` outside core/audio_sniff.cpp.

### M65. Do n bytes fit at this position in the buffer?
Kind: K3 (every reader's answer to "does not fit" is a different policy on purpose: metadata lenient, payload strict) with a K1 line-level piece in the yes/no itself. Fix: code-only, optional.
Homes: `fits`, src/parse/sng.cpp 15-20 (`pos <= size && n <= size - pos`, overflow-safe); `read_u32`/`read_string`/`skip` lambdas in `srb_parse_metadata`, srb.cpp 143-163 (`n > meta.size() - pos`, safe only because pos starts checked and moves forward by checked amounts); `walk_chunks`/`walk_track`, midi.cpp 189-200 and 268 (`pos + 4 > size`, `pos + 8 > size`, `std::min(start + chunk_len, size)`, `pos + nbytes > end`; adds, safe on a 64-bit size_t because every length is a 32-bit field); `srb_audio_from`, app/preview_source.cpp 201, 250-268 (`offset + 8 > size`, `cursor + 24 > size`, `blob_size > size - cursor`); `range_length`, core/winstr.cpp 233-236 (a clamp, answers "how many bytes exist").
Tables (size, pos, n): n = 0 at pos = size: sng fits; srb lambdas fit; `range_length` 0. n = 1 at pos = size: all say no. pos above size: sng says no; `range_length` says 0; the srb lambdas would wrap to a huge number and say yes, but pos never passes size there. n near 2^64: sng no; srb no; midi cannot see it (lengths are 32-bit). What a "no" does: sng metadata and file table stop and keep what they have; sng notes entry throws "Truncated SNG file."; an audio entry that does not fit is skipped; srb metadata keeps the fields read so far and returns true; an srb stream throws; midi stops the walk and keeps the messages so far (except under 14 bytes or no MTrk chunk, which throw); the Preview's blob walk breaks and returns the stems found. little_endian.h 5-6 says each caller checks its own "because each one handles short input its own way".
Also the same test twice: "the file is no bigger than the 16-byte .srb header" (`src.size <= kSrbHeaderSize`, srb.cpp 187 throws; `buf.size() <= kSrbHeaderSize`, preview_source.cpp 201 returns no stems). One shared constant, two spellings of the test, two different outcomes by design.
Disagree on: none on the yes/no for any input a real file can give.
Owner proposed: move the sng.cpp `fits` pair into core next to `little_endian.h` and let the srb lambdas and the Preview's blob walk call it. Keep every caller's own "no" policy.
Evidence: by reading.
Found by: G7b-4; verified by V7.
Scan row or reason: weak. A row on `\.size\(\) - pos` and `size - pos` comparisons would flag a hand-written bounds test, but would also flag valid arithmetic; I would not add one.

### M66. How a song file is opened for reading, the share mode, and what the user sees when it can't be
Kind: K2 on the share rule (loose stems are opened with a stricter share mode than every other read of a song file); K1 on the "cannot open file" sentence. Fix: needs the user (a choice between behaviours: a stem another program is writing would start to play).
Homes: `file_byte_source`, core/winstr.cpp 238-248 (`CreateFileW`, `GENERIC_READ`, `FILE_SHARE_READ | FILE_SHARE_WRITE`; throws `SongFileMissing` "cannot open file: " and, for a size failure, "cannot read file size: "; its own comment says why: "a file another program holds open still reads"); `stream_md5`, app/analysis.cpp 121-123 (`_wfopen` "rb", which shares read and write by default, per the C runtime; throws the same kind and sentence); `MappedFile::open`, audio/mapped_file.cpp 20-51 (`GENERIC_READ`, `FILE_SHARE_READ` only; four throws of "cannot open file: " at 25, 30, 43, 46, covering open, size, mapping and view failures, and it words a size failure as "cannot open file" where winstr says "cannot read file size"); the click path, `hash_chart_file` (analysis.cpp 389-396) swallows any exception and returns empty, then ui/library_jobs.cpp 506-507 throws a plain `std::runtime_error("could not read " + path)` with no kind.
Share modes, confirmed in the code: the first two share read and write, the third shares read only. By the Win32 rule (not run), a second open of a file already held open for writing by another program fails with a sharing violation if the new open does not itself allow writers. So `file_byte_source` and `stream_md5` open a file another program is writing; `MappedFile::open` does not. `MappedFile` is only used for loose stems of a folder chart (`map_song_stems` when `from_file()`), plus the font loader (ui/app_shell.cpp 82-101, which falls back to ImGui's loader). Stems inside a .sng or .srb come from `read_file_bytes` (which goes through `file_byte_source`), so they are not affected.
**What the user sees:** nothing. `map_song_stems` catches the throw (song_audio.cpp 22-24) and leaves the stem out; `open_song_stems` then skips the empty slot. The Preview loads, shows no warning, and plays without that instrument (muted if it was the only stem). The "cannot open file" text from `MappedFile` never reaches a screen in the product: its only other callers are decoder helpers used by tests and tools. A missing chart file, by contrast, reaches the user as the `SongFileMissing` sentence ("Hydra couldn't open the song file. It may have been moved or deleted; run Scan library..." per app/user_messages.cpp), and the click path's untyped throw reads the generic "Something went wrong" line instead.
Tables: file missing: `file_byte_source`, `stream_md5`, `MappedFile` all throw `SongFileMissing` "cannot open file: <path>"; the user reads the moved-or-deleted sentence for the first two; for stems, nothing. Chart file held open for writing: opens through winstr and `_wfopen`; a loose stem held open for writing: refused by `MappedFile`, stem dropped. Zero-byte file: winstr gives an empty source; `MappedFile` gives an empty map with no error (lines 34-38). Size unreadable: winstr "cannot read file size"; `MappedFile` "cannot open file".
Disagree on: "can Hydra read a file another program holds open for writing": yes for notes, containers, hashes and .sng/.srb stems; no for loose stems. A sharing reason might exist for mapping (a mapped file that another program shrinks can fault on read), and no comment in mapped_file.h states one, so widening the share mode is a decision for the user, not a tidy-up.
Owner proposed: `file_byte_source` owns the open rule and the two "cannot open file" / "cannot read file size" sentences; a small helper in winstr (`open_for_reading(path)` returning the handle or throwing the one sentence) would let `MappedFile` call it for the open step and keep only the mapping. Whether that helper shares writers is the user's call. The click path's untyped `runtime_error` should become a `KindedError(SongFileMissing, ...)` (a display-text change, so also the user's call).
Evidence: by reading, proven by a test or a uitest script that holds a stem open for writing with a second handle and opens the Preview, looking at whether the stem is in the mix.
Found by: G2-8, G9-3, also-seen G2-8 in G7b, G8, G9; verified by V7.
Scan row or reason: a row on the quoted text `"cannot open file: "` and `"cannot read file size: "` outside core/winstr.cpp would flag the hand-typed copies (today: app/analysis.cpp 123 and four in audio/mapped_file.cpp). A second `CreateFileW(` call outside winstr.cpp would flag a fresh open with its own share mode (today only mapped_file.cpp, apart from winstr's own two).

### Side notes

- parse/note_shuffle.cpp 142 (scout's note, not re-checked): `prev_tick` starts at 0, so a first non-kick note at tick 0 counts as in the same chord as an earlier one. I did not compare with the reference.
- audio/mapped_file.h 22 says it throws `std::runtime_error`; the code throws `KindedError`, a subclass, so the comment still holds.
- app/preview_view.cpp 454-456 comment says "a mid-measure meter change restarts the measure there", but `MeasureIndex` counts the new section's bars from the previous barline (timing.cpp 97-106), so the measure is finished in the new meter's length, not restarted. The code and `Timecode` agree with each other; only the comment words it differently. Worth a reword when someone is there.
- `Song::check_activations` calls `timing().plusmeasure` for downbeats (song.cpp 624) and its own tpm walk for `pre_tpm` (607-617) in one function, so two meter lookups with two rules sit side by side. That is finding 53 again, not a new one.

---

## V8 findings: audio, ini files and file helpers (M67 to M75)

Verifier V8 (the progress hook never named another id, so I used `V8`). Read-only: I changed no file but this one. Everything below is by reading; I ran nothing.

Two facts frame the audio candidates. First, `decode_audio`, `decode_stem`, `read_all` (decode.cpp) and `mix_stems` (mixer.cpp) have no caller anywhere in `src/` or `tools/`. Only tests and `tests/fuzz/harness.h` call them. The Preview opens readers (`open_song_stems`) and plays a `StreamMix` (`mix_song_stems`). So in M68, M69 and M71 one of the two homes is a test oracle that happens to live in `src/`. Second, `failed()` has exactly one reader in the code base: `read_all`, decode.cpp line 42.

### M67. Do the four readers repeat the same cursor bookkeeping, and does the 4096-frame block have one home?
Kind: K3 (every repeated member is a fact about one decoder; the one rule inside, "a seek to or past the end stops the stem there", already has one owner). The 4096 number is K0 on meaning. Fix: none needed. An optional tidy is code-only.
Homes: `MaReader`, ma_reader.cpp 170-258 (members `at_end_`, `pos_`, `channels_`, `rate_`, `length_`, `scratch_`, `counted_`, `failed_`);
       `Mp3Reader`, ma_reader.cpp 382-540 (same minus `counted_`, `failed_`);
       `VorbisReader`, vorbis_reader.cpp 30-117 (same as Mp3 plus `counted_`);
       `OpusReader`, opus_reader.cpp 208-415 (`at_end_`, `failed_`, `length_`, `channels_`; no `pos_` or `rate_`);
       the block sizes: `kSkipChunk` ma_reader.cpp 166, `kChunkFrames` vorbis_reader.cpp 28, `StreamMix::kBlockFrames` stream_mix.h 89, the literal `4096` twice in decode.cpp 34-35.
Tables: the end-of-stem rule, per reader.
| Input | Ma | Mp3 | Vorbis | Opus |
|---|---|---|---|---|
| seek to frame >= length | ends there (`seek_clamp_or_end`) | same | same | same |
| seek to 0 on an empty stem | `length_` 0 so ends at once | same | same | cannot exist (open throws on length 0) |
| decoder returns fewer frames than asked, mid-stream | keeps looping until 0 frames or MA_AT_END | ends the stem on the short answer (ma_reader.cpp 419) | keeps looping until 0 | keeps decoding until links run out |
| decoder refuses a seek below the end | `at_end_ = true`, `pos_ = frame` | `restart_and_skip` fails: `at_end_ = true`, `pos_ = frame` | `at_end_ = true`, `pos_ = frame` | no refusal path |
| read after the end | 0 | 0 | 0 | 0 |
The only row where a reader has its own rule is the short read in `Mp3Reader::read`. dr_mp3's `read_pcm_frames_f32` loops over MP3 frames and returns fewer frames than asked only at the end of data or at its padding cap (miniaudio.h 95213-95262), so it agrees with the other three on every input I could reach.
The 4096 table: `kSkipChunk`, `kChunkFrames` and the `read_all` literals each size one scratch buffer in their own file and nothing needs them equal. `kBlockFrames` is tied to the resampler alignment (`align_out <= kBlockFrames`, stream_mix.cpp 161). The two `4096` literals inside `read_all` do have to match each other (buffer size and read count); that is one function, one place to edit.
Disagree on: none today; a decoder that returns a short read mid-stream would split `Mp3Reader` from the other three.
Owner proposed: leave the four classes as they are. `StemReader` is an interface and `seek_clamp_or_end` / `CountedLength` already hold the shared rules. Lifting `at_end_` and `pos_` onto a base would save about six lines per class and make Opus (which counts in link samples) an exception. Optional tidy: give `read_all`'s two literals one named constant.
Evidence: by reading.
Found by: G8-1, G8-8; verified by V8.
Scan row or reason: no text search can catch it. The repeated members are different decoders' state under the same member names, which is not a spelling a scan row could match without flagging every class.

### M68. How many frames does a stem have, and who decides?
Kind: K2 on paper (same word "length", the rules differ at one edge, a decoder that yields more frames than it promised), but only one home is production code. Effectively a Preview-only rule today. Fix: code-only.
Homes: each reader's `length_frames()` (header, a count by decoding, a library count, or the Opus page index sum), the one source;
       `read_all`, decode.cpp 18-45 (reserves `length_frames()`, then keeps reading until the reader returns 0, so extra frames are kept);
       `StreamMix` constructor and `read`, stream_mix.cpp 183-221 and 225-253 (takes `length_frames()`, converts it, and stops reading the stem at that converted length).
Tables: a stem whose decoder yields D frames while `length_frames()` says L.
| Case | `read_all` (tests only) | `StreamMix` (the Preview) |
|---|---|---|
| D == L | L frames | L frames |
| D > L | D frames (the extra is kept, comment decode.cpp 13-17) | L frames (`want = min(m, converted_length - out_pos)` stops there; the extra is never played) |
| D < L | D frames; throws only if `failed()` | D frames, then the stem is marked `ended`, silent; the mix's length still includes the silent tail |
| L == 0 | empty result, no throw (Opus throws at open instead) | the stem plays nothing; contributes length 0 |
Can a real file split them? By reader:
- Opus: D > L cannot happen (`decode_next` clips each packet to `end_g`). D < L can: a page with a bad checksum is skipped silently (opus_reader.cpp 341) after the index counted it, so the stem ends short and `failed()` stays false.
- Vorbis and WAV/FLAC with a header of 0: `CountedLength` counts by running the same decode loop that `read` runs, so D == L by construction.
- Vorbis and WAV/FLAC with a real header: L is the header's claim (stb_vorbis end page, dr_wav data chunk size, dr_flac total). A truncated file gives D < L. A header that understates gives D > L. I did not look for a real file that does either.
- MP3 with a Xing/LAME tag: `ma_dr_mp3_get_pcm_frame_count` returns the tag's total minus delay and padding (miniaudio.h 95483-95493), and `read_pcm_frames_f32` stops at that same total (95222-95256), so D <= L and a truncated file gives D < L. MP3 without a tag: the count walks every frame with the same frame decoder the read uses, so they agree.
- The tests pin D == L only for the bundled fixtures (tests/test_stem_reader.cpp 225-251 for every fixture, 488-502 for the tagged and the long untagged MP3), against a test-local `old_full_decode`.
Which one the Preview uses: `StreamMix`, always. The Preview never reads a stem whole, so `length_frames()` is the only length the user ever meets (the playback range and `end_chart_ms` come from it, song_audio.cpp 78-80). The one place the two disagree, D > L, is a case where the Preview plays less than the file holds and the test oracle plays all of it. Nothing visible changes today because no reader is known to produce D > L on a real file.
Disagree on: D > L (a header shorter than what the decoder yields). Truncated files (D < L) go the same way in both except that only `StreamMix` pads the tail with silence.
Owner proposed: `length_frames()` stays the one definition and `StreamMix` stays the kept behaviour. Either stop `read_all` at `length_frames()` or move it and `mix_stems` into the tests. No displayed word or stored value changes because `read_all` has no production caller.
Evidence: by reading. To prove D > L is unreachable for real files, run a one-stem test in tests/test_stem_reader.cpp that opens a WAV whose data chunk size is smaller than the bytes present and compares `length_frames()` with `read_to_end`. To prove D < L, the same with the chunk size larger than the bytes present.
Found by: G8-2; verified by V8.
Scan row or reason: no text search can catch it. The two rules are an inequality in two loops, not a shared spelling.

### M69. What does a decode error do, and who finds out?
Kind: K2 (same idea, different rules per reader and per caller), but nothing the user reads changes today. Fix: code-only if it is limited to making the contract say which readers can report a failure; needs the user (display text) if the Preview should ever tell the user a stem was damaged.
Homes: `failed_` and `failed()` in `MaReader` (ma_reader.cpp 200, 210-215, 225-227) and `OpusReader` (opus_reader.cpp 255, 375-379); default `failed()` of false in `Mp3Reader` and `VorbisReader` (stem_reader.h 60); the one asker, `read_all` (decode.cpp 42); the Preview's handling, `StreamMix::read` (stream_mix.cpp 246) and `open_song_stems` (song_audio.cpp 57-62).
Tables: what each reader does on a decode error, and what `StreamMix` and the Preview then do.
| Reader | What stops the stem | `failed()` | What `read_all` does (tests) | What `StreamMix` does | What the Preview shows |
|---|---|---|---|---|---|
| MaReader (WAV, FLAC) | decode call returns an error code with frames delivered: `failed_ = true`, `at_end_ = true`. An error with 0 frames delivered is treated as the end, no `failed_` | true after the first such error, stays true through seeks | throws `KindedError(AudioDecode)` | `got < want`: that stem is `ended`, silent until the next seek; others keep playing; mix length unchanged | nothing |
| MaReader, counted stem | skip during `CountedLength::seek` ends in an error: `failed_ = true` (ma_reader.cpp 225-227) | true | throws | as above | nothing |
| OpusReader | `opus_decode_float` returns below 0: `failed_ = true`, `at_end_ = true`. A page with a bad checksum is skipped silently and the open-time index resyncs silently over damage | true only for the decode error, never for a skipped page | throws for a decode error; a skipped page returns short audio without throwing | same silent stem | nothing |
| Mp3Reader | a short read ends the stem | always false | returns short audio, no throw | same silent stem | nothing |
| VorbisReader | `got <= 0` ends the stem | always false (comment vorbis_reader.cpp 86-87: stb_vorbis cannot tell an error from the end) | returns short audio, no throw | same silent stem | nothing |
After a seek back before the damage, all four play again from there (`seek_to` clears `ended` and `reader_done`). `StreamMix` never calls `failed()`. A stem that throws at open is skipped by `open_song_stems` with no message (every exception except `OpenCancelled` is swallowed), so an unopenable stem and a damaged stem both end as silence.
Disagree on: the same damaged stem throws in `decode_audio` if it is FLAC/WAV or Opus (decode error), and returns short audio if it is MP3 or Ogg Vorbis. In the Preview all four go silent the same way.
Owner proposed: `StemReader`'s contract (stem_reader.h 49-60) stays the owner and should say plainly that only `MaReader` and `OpusReader` report `failed()`, and that the Preview does not read it. `StreamMix::read`'s "ended, silent from here" is the one Preview rule. If the user wants a damaged stem reported, that is a new display sentence and a question for the user.
Evidence: by reading. A test that proves the table: feed `decode_audio` a truncated MP3 and a truncated Ogg Vorbis from tests/test_stem_reader.cpp's fixtures and assert it does not throw, next to the existing Opus case at test_stem_reader.cpp 571-602 (which asserts it throws). I found no test that pins the non-throw for MP3 or Vorbis.
Found by: G8-3; verified by V8.
Scan row or reason: a row on the spelling `failed()` outside src/audio and the tests would flag a second asker, but the question is "who may ask", which one reader answers today; I would not add a row, the table above is the record.

### M70. Getting to a frame by decoding forward and throwing the audio away
Kind: K3 (the loop shape belongs to each decoder's fallback; the shared part, `CountedLength::seek`, is already shared by the two readers that need it; the distance rule in `Mp3Reader` is a speed trick with a table seek behind it). Fix: none.
Homes: `CountedLength::seek`, stem_reader.h 172-192 (used by `MaReader` and `VorbisReader` on a counted stem);
       `Mp3Reader::seek`, `skip`, `restart_and_skip`, ma_reader.cpp 424-443 and 509-523.
Tables: seek target T, current position P, stem length N (T < N after the clamp).
| Input | `CountedLength::seek` | `Mp3Reader::seek` |
|---|---|---|
| T == P, not at end | loop body never runs; nothing | returns at once |
| P < T, any distance | decodes forward from P | decodes forward only if T - P <= one second; otherwise table seek, checked by `landed`, else restart at 0 and decode forward |
| T < P | restart at 0, decode forward | table seek, else restart at 0 and decode forward |
| stem at end, T < N | restart at 0, decode forward | table seek or restart (the short hop is skipped when `at_end_`) |
| decode ends before T | `at_end` true, returns the step's `error` | `at_end_ = !skip(...)`; `pos_ = frame` even though the decode stopped early; no error reported |
| restart refused | `at_end` true; `pos` is left stale | `restart_and_skip` returns false, `at_end_ = true`, `pos_ = frame` |
Every row agrees on where a successful seek lands. The error row differs by reader (`MaReader` sets `failed_`; `VorbisReader` and `Mp3Reader` have no way to), which is M69's table, not a second rule.
Disagree on: none that changes audio; the leftover `pos` after a refused restart or a short skip is wrong in both homes but only matters at the end, where every read returns 0 anyway.
Owner proposed: leave both. `Mp3Reader` has a table seek to use so it should not follow `CountedLength`'s decode-any-distance rule; `CountedLength` has none.
Evidence: by reading.
Found by: G8-4; verified by V8.
Scan row or reason: no text search can catch it; the two loops share no spelling (`skip(` against `decode(`).

### M71. The mixing rule written twice (`mix_stems` and `StreamMix`), and the plain `runtime_error` throws
Kind: K3 (parallel on purpose: `mix_stems` is a test reference kept in src/, comment mixer.h 12-15; the one shared rule, which converter settings and whether a stem passes through, is `stem_converter_config`, mixer.cpp 10-19, called by both). Two small rule differences below are K1/K2 line-level and internal. Fix: code-only if anything is done.
Homes: `convert_stem` and `add_into` in mixer.cpp 28-65 and `mix_stems` 69-77;
       `StreamMix` constructor and `read`, stream_mix.cpp 183-253.
Tables: one stem converted, then summed.
| Input | `mix_stems` | `StreamMix` |
|---|---|---|
| stem with channels <= 0 or no samples | contributes nothing (`convert_stem` returns {}) | channels <= 0 or rate <= 0: dropped; zero-length stem with a rate is kept and contributes length 0 |
| stem with rate 0, channels > 0 | `ma_data_converter_init` fails, throws `runtime_error` | dropped |
| same rate and channels as the output | passes through, length D | passes through, length `length_frames()` (M68's table) |
| different rate | length is what the converter produced (`out_produced`) | length is miniaudio's expected count (`ma_data_converter_get_expected_output_frame_count`); the test pins them equal (tests/test_stream_mix.cpp 162-179, `mix.length_frames() == ref.frames()` for each rate pair in `mix_cases()`) |
| sum | every sample starts at 0.0f and adds stem by stem in order | same, in 4096-frame blocks; equal within 1e-6 (the same test) |
| converter fails | `std::runtime_error("mix_stems: ...")` | `KindedError(AudioDecode, "StreamMix: ...")` |
Disagree on: a rate pair where the converter produces fewer frames than miniaudio's expected count would make `StreamMix` one or two frames longer than `mix_stems`, with a silent tail. The tests show equality only for their own rate pairs. A stem with rate 0 throws in one and is dropped in the other, which cannot happen for a real reader (every reader rejects rate <= 0 or reads it from the file).
The `runtime_error` throws: mixer.cpp 37 and 52, device.cpp 61 and 78. `mix_stems` has no production caller, so its error type never reaches a user. The device's failure shows a fixed sentence with the raw text on hover (preview_tab.cpp 389-393), so the plain type causes no wrong wording. Only `StreamMix`'s throw can reach `plain_error`, and it is a `KindedError`.
Owner proposed: no merge. `mix_stems` stays a test oracle; move it into the tests directory if the user wants `src/audio` to hold only production code. If it stays, change its two throws to `KindedError(AudioDecode, ...)` for consistency (code-only, since nothing displays them).
Evidence: by reading. A run that would settle the length question: add rate pairs 22050, 32000 and 96000 to `mix_cases()` in tests/test_stream_mix.cpp.
Found by: G8-6; verified by V8.
Scan row or reason: a row on `std::runtime_error(` in src/audio would flag a plain throw; it would also flag device.cpp's two, so a row only helps after those are decided.

### M72. What does the user read when hydra_rules.ini is bad?
Kind: K2 (one state, three wordings, two of them on screen or in a terminal; the third is dead in production). Fix: needs the user (display text).
Homes: `kRulesFile`, user_messages.cpp 48-50, reached by `kind_sentence` line 96 for `ErrorKind::RulesFile`;
       `AppState::kAnalysisOffSentence`, app_state.h 279-281, shown by library_toolbar.cpp 148-152 and path_report_window.cpp 160-161, with the loader's raw text `rules_error` (set at app_state.cpp 27-28);
       the four tool entry points that print `e.what()` raw: cli/batch.cpp 113-115, cli/fillcompare.cpp 74-77, tools/replay.cpp 778-781, tools/bench.cpp 518-521 (each returns 2).
Tables: the same input, any bad line in hydra_rules.ini (an unreadable value, an unknown key, a line with no `=`).
| Surface | What the user reads |
|---|---|
| Hydra app, library toolbar and path report window | "hydra_rules.ini has an error, so analysis is off until the file is fixed and Hydra is restarted." and under it the raw text such as `<path>: <key> = "<value>" is not a number` |
| `kRulesFile` via `plain_error` | "hydra_rules.ini has a line Hydra can't read. Fix or delete that line, then restart Hydra." Reached only by tests/test_user_messages.cpp 94 and 171 |
| the four tools | the raw text alone, exit code 2 |
Your check on the catch order: `load_rules_file` is called in exactly five places (app_state.cpp 26, batch.cpp 111, fillcompare.cpp 72, replay.cpp 775, bench.cpp 515), each directly inside a `try` whose catch is `RulesFileError`, so no wider `catch (const std::exception&)` upstream turns it into `plain_error`. The G9 scout's claim holds: `kRulesFile` is unreachable in production. Its wording also says "restart Hydra", which is wrong advice for `hydra_batch`.
Disagree on: every input. Three wordings for one state.
Owner proposed: `user_messages.cpp` should own the sentence, since its header says every error sentence lives there. Keep the app's wording (it is under a uitest, uitest_report_windows.cpp 368, and a D103 decision) and move it from `AppState` into `user_messages.cpp`, with `kind_sentence(RulesFile)` returning it; delete the unused `kRulesFile` text. For the four tools, the question for the user is whether they print the same sentence, a tool-specific one without "restart Hydra", or stay raw. That is the part that needs the user.
Evidence: by reading. A one-chart `hydra_batch` against a copy of hydra_rules.ini containing `nope = 1` would show the tool's current wording (the raw `unknown key` line).
Found by: G9-1; verified by V8.
Scan row or reason: a row on `hydra_rules.ini has` outside user_messages.cpp, in rules/app.json, would flag a second copy of either sentence. There is already one entry in app.json (line 98) that lists the dead sentence as a `must_not_match` under "Why is a stored result out of date?", which would need updating when the sentence is deleted.

### M73. What text counts as a number in a hand-edited ini file (does "nan" pass `check_min`)?
Kind: K2 (three grammars for "a number" in hand-edited ini text; the rules file's grammar is wider than the other two, and the extra width is a bug, not a policy). Fix: needs the user (a choice between behaviours: a file that loads today would start reporting an error).
Homes: `parse_finite_number`, strutil.cpp 80-92 (`std::from_chars`, whole text used, finite only);
       `std::atoi` for whole-number keys of hydra_settings.ini, config.cpp 177-179;
       `to_double` and `to_int` (`std::stod`, `std::stoi`, "all text used"), rules_file.cpp 28-42, feeding `check_min` rules_file.cpp 22-26.
The path from ini text to stored value, for a double key: `load_rules_file` reads a line, `split_ini_line` (strutil.cpp 98-105) cuts at `#`, splits at the first `=` and trims both sides, `set_field` (rules_file.cpp 45-60) sees a `double core::Rules::*` member, calls `to_double`, then `check_min`, then `r.**real = d`. The four double keys are `backend_leeway_ms`, `fill_max_distance_beats`, `fill_length_measures`, `fill_land_slop_beats` (rules.cpp 84-91). `max_tied_paths` and `fill_cooldown_measures` are int keys and go through `to_int`. `sqout_rule` is a word.
Settling the claim by reading: `to_double` calls `std::stod(v, &used)` and treats a thrown exception or `used != v.size()` as an error. `stod` is `strtod` underneath, and `strtod` in MSVC's UCRT (the project builds with MSVC) accepts `nan`, `inf`, `infinity` in any case and hex floats like `0x10`. `stod` throws only when nothing converts (`invalid_argument`) or the result is flagged out of range (`out_of_range`, which `1e400` hits). `inf` and `nan` are not flagged out of range. So `nan` converts, `used == 3 == v.size()`, and `d` is NaN. `check_min` then tests `d < f.min` (false for NaN) and `!f.min_allowed && d == f.min` (also false), so the NaN passes. Yes, the claim is right. `inf` passes every `check_min`, because no key has an upper bound. For int keys: `std::stoi("nan")` and `"inf"` throw, and `"0x10"` converts only the `0`, so `used != size` and it is rejected. Only the four double keys take the extra spellings.
Tables, value text to result (rules file double key | rules file int key | settings whole-number key | settings decimal key):
| Text | rules double (`stod`) | rules int (`stoi`) | settings int (`atoi`) | settings decimal (`parse_finite_number`) |
|---|---|---|---|---|
| `4` | 4 | 4 | 4 | 4 |
| `4.0` | 4 | error | 4 | 4 |
| `+5` | 5 | 5 | 5 | 5 |
| `5abc` | error | error | 5 | reads as 0, then range |
| `abc` | error | error | 0 | reads as 0, then range |
| `nan` | NaN, passes `check_min` | error | 0 | reads as 0, then range |
| `inf` | infinity, passes `check_min` | error | 0 | reads as 0, then range |
| `0x10` | 16 | error | 0 | reads as 0, then range |
| `1e400` | error (out of range) | error | 1 (atoi stops at the `e`) | reads as 0, then range |
Consequence: `fill_length_measures = nan` loads without error. Every comparison with NaN is false, so each rule that compares against it silently flips one way or the other; I did not trace the engine. The value also reaches the fingerprint text (`add_line` writes `%.17g`, rules.cpp 33-45) as a NaN spelling, so stored results get a stamp for rules nobody meant. It breaks the promise in rules_file.h 3-5 that a typo never silently runs.
Disagree on: `nan`, `inf`, hex, and `4.0` in a whole-number key. The junk policy (error against reads-as-0) is deliberate and documented; the grammar is not.
Owner proposed: `parse_finite_number` is the owner of "what text is a decimal number". `to_double` should call it and keep the rules file's error on `nullopt`. For whole numbers, keep `to_int` (strict is the rules file's policy) but have it reject exactly what `parse_finite_number` rejects. The strutil.h comment ("read one way for every such number") then becomes true.
Evidence: by reading; the C runtime's `strtod` behaviour is the part a run would prove. The test case: in tests/test_rules.cpp, extend "rules: a bad value or an unknown key is an error that names the key" (line 115) with `message_for("bad6", "fill_length_measures = nan\n")`, then `inf`, `NaN`, `infinity` and `0x10` for the same key. Each should name the key. Today `message_for` returns "" for them (no exception), so `"".find(key)` is `npos` and the CHECK fails red.
Found by: G9-2; verified by V8.
Scan row or reason: a row on the spelling `std::stod(` in src/ would flag a second number grammar; today it matches only rules_file.cpp. `std::stoi(` also appears in parse/song.cpp 1289 on purpose (the chart-number rule is documented there), so a row would need that one site as an allowed line.

### M74. How is a file written so a reader never sees half of it?
Kind: K2 (three rules for writing a file; the reason is written down for only one). Fix: needs the user (a choice between behaviours: the settings file would gain a `.tmp` sibling and a replacing rename).
Homes: `write_report_file`, report_files.cpp 89-117 (write `<path>.tmp`, then `std::filesystem::rename` over the target, remove the temp on failure);
       `Settings::save_file`, config.cpp 217-239 (opens the target with `trunc`, writes in place, returns `f.good()`);
       the library file swap, store/upgrade_files.cpp 40-49 and 67-100 (two plain `MoveFileExW` renames with `MOVEFILE_WRITE_THROUGH`, no replace flag, recovery code finishes or undoes the swap);
       also ImGui's own `hydra_ui.ini` write, which goes through `ImFileOpen` "wt" in place (ui/imgui_files.cpp 15-18). Tools write outputs in place too (replay.cpp 201, bench.cpp 198 and 409), not shipped code.
Tables: the process stops in the middle.
| Writer | What a reader finds afterwards |
|---|---|
| report page | the old page, or the new one; never a partial one. A stray `.tmp` may remain |
| settings ini | a truncated file. Config.cpp 197 loads a missing or short file as defaults, and unwritten keys take their defaults, so the user loses settings after the cut point |
| library file swap | `recover_upgrade_files` completes or undoes it on the next open |
| hydra_ui.ini | a truncated file; ImGui reads what it can |
Target on FAT or exFAT: the report writer's replacing `std::filesystem::rename` is a delete and then a rename there, per the upgrade code's own comment (upgrade_files.cpp 41-44). A stop between the two loses the page, which is regenerable. The upgrade code avoids that because it cannot regenerate.
Disagree on: the stop-in-the-middle case and the replace rule.
Owner proposed: no code change is forced. The settings file is small and rewritten often, so the weaker rule loses little, but no comment says it is a choice. If the user wants settings to survive a cut, lift `write_report_file`'s write-then-rename into one helper in `core/winstr` and have `Settings::save_file` call it; the report page keeps its rule; the library swap stays as it is (it needs the recovery step, which a helper cannot give). Otherwise add a one-line comment at `save_file` that in-place is on purpose.
Evidence: by reading.
Found by: G9-6; verified by V8.
Scan row or reason: after a merge, a row on `std::ios::trunc` in src/ outside the helper would flag a new in-place writer. Today it matches config.cpp 218 and report_files.cpp 100, both legitimate, so a row now would only say "these two", which is the question.

### M75. Path-string helpers: where does a path get cut at its last separator?
Kind: K1, line-level (the cut and the trailing-separator trim agree on every input the callers pass). Fix: code-only.
Homes: `parent_folder` and `join_folder`, core/winstr.cpp 123-134 (trims trailing separators, then cuts at the last `\` or `/`);
       `base_name` and `stem_of`, app/preview_source.cpp 31-42 (cuts at the last `/` or `\`, no trailing trim);
       `relpath`, app/analysis.cpp 48-57 (trims trailing separators from both paths, then strips the base);
       `relative_slash_path`, core/strutil.cpp 71-78 (strips the root plus one byte, turns `\` into `/`).
Tables (input: `parent_folder` | `base_name` | `relpath(input, "C:\a")`):
| Input | `parent_folder` | `base_name` | `relpath` |
|---|---|---|---|
| `C:\a\b` | `C:\a` | `b` | `b` |
| `C:\a\b\` | `C:\a` | empty | `b` |
| `C:\` | empty (trims to `C:`, then no separator) | empty | `C:` |
| `abc` | empty | `abc` | `abc` |
| empty | empty | empty | empty |
| `a/b\c` | `a/b` | `c` | `a/b\c` |
Callers: `parent_folder` gets file paths and folders (config.cpp 21, analysis.cpp 401, 454, 613, preview_source.cpp 366, 379, 385); `base_name` gets entry names (a list entry's file name or a container entry's name, preview_source.cpp 126, 150, 162), so a trailing separator never reaches it. The only duplicated piece is the trailing-separator trim loop: `parent_folder` once (winstr.cpp 125), `relpath` twice (analysis.cpp 50-51), and its test for a separator at `t[b.size()]` (54).
Disagree on: a path that ends in a separator (`base_name` returns empty, the other two trim first). No caller passes one.
Owner proposed: `core/winstr` owns separators. Add one `trim_trailing_separators` there; `parent_folder` and `relpath` call it. Leave `base_name` private or move it beside `parent_folder` as the leaf-name twin, and let it use the same separator set.
Evidence: by reading. `relative_slash_path`'s extra byte is a known wrong-for-`C:/input/x`-under-`C:/in` case (audit finding 274, comment in strutil.cpp); it is kept on purpose for stored keys and is not part of this fix.
Found by: G9-7; verified by V8.
Scan row or reason: a row on the spelling `.find_last_of("` with a backslash and slash in src/ outside core/winstr.cpp would flag a re-spelled cut (today it matches preview_source.cpp 33). A row on `back() == '\\'` would flag the trim loop (today analysis.cpp 50, 51 and winstr.cpp 125, 133).

### Side notes

- ma_reader.cpp 181-189 comment says a WAV can't have length 0 with audio; fine, but `MaReader::skip` marks an error as `error` only when `got != 0` (line 244), so an error that delivers no frames looks like the end in both `read` and `CountedLength::seek`. They agree; worth knowing for M69.
- `CountedLength::seek` (stem_reader.h 175-179): if `restart()` fails it sets `at_end = true` but leaves `pos` at its old value. `Mp3Reader::seek` (ma_reader.cpp 430-431) sets `pos_ = frame` after a short skip that stopped early. Both leave a wrong position, but only at the end, where every read returns 0.
- `rules_file.cpp` has a lower bound per key but no upper bound, so `fill_max_distance_beats = 1e300` and `backend_leeway_ms = inf` pass (see M73).
- `write_report_file` always uses the same `<path>.tmp` name, so two writers of one report would collide. Not a duplication question.
- The stale comments G8 listed (decode.h 48-49, mapped_file.h 22-23, song_audio.h 68-69, stream_mix.h 76 say `std::runtime_error` where the code throws `KindedError`) are still true as written, since `KindedError` derives from `std::runtime_error`. Not re-checked.

---

## V9: background jobs and the app's messages (verifier report)

Task id: concept-audit-V9. Agent id used for status lines: `a228ba35ed2526b33` (named by the progress hook late; the first lines went under `V9`). Read-only: no builds, no tests, nothing run. Every claim below is "by reading" unless a run is named.

Summary in plain words. Of nine candidates, two are real questions for the user (M81 and M82, wording of two states, plus the Failed wording in M83), one is a real behaviour split that is reachable today (M77, the click against the reports), and the rest are either parallel on purpose or small code-only tidy-ups. The most useful single fact: the "stopped" state of a batch is read a third way (from the cancel flag after the job has finished), and the base class's `finished()` is a flag that two of the eleven jobs never set and nobody reads.

---

### M76. Do the jobs each hand-roll "seconds since start", the time-left clock feed and the 3-second gate?
Kind: K3 (parallel on purpose) with one line-level K1 inside it. The shared rules, the 3-second gate, the words and the byte-rate maths, are already written once. What is written twice is one expression and a five-line mutex block.
Fix: code-only (no word, stored field, score or count moves).
Homes:
- `StoreOpenJob::elapsed_s` and `progress`, src/ui/library_jobs.cpp 90-92 and 104-109 (inputs: `started_`, rows done and total, step);
- `PreviewLoadJob::progress`, src/ui/preview_load_job.cpp 246-265 (inputs: `started_`, bytes done and total, step);
- `AppState::view_progress_shown`, src/ui/app_state.cpp 341-346 (input: `view_requested_at_`, the time of the request, set at app_state.cpp 279, not the job's start);
- `BatchClock` and `steady_seconds`, src/ui/library_jobs.cpp 239-273 (pauses left out; a different clock on purpose);
- `batch_eta_s`, library_jobs.cpp 275-278, against `ByteRateClock::update`, preview_load_job.cpp 94-113, with the shared gate `gated_time_left_text`, preview_load_job.cpp 299-302, and the shared words `time_left_text`, src/ui/library_parts.h 63-65.
Tables:
| input | StoreOpenJob | PreviewLoadJob |
|---|---|---|
| clock fed before the rate step (rows 0 of 0) | yes, every frame from the first | not fed (Preview feeds only in its Opening-audio step, line 260) |
| clock fed in the rate step | Copying | Opening audio |
| time-left text outside the rate step | "" (step check, library_jobs.cpp 44) | "" (step check, preview_load_job.cpp 307) |
| job younger than 3 s | "" | "" (same function) |
| rate unknown (under 1 s measured, or nothing moved in the last second) | "" | "" (same clock) |
| done greater than total | rows not clamped here; the bar helper clamps (`progress_fraction`) | clamped at line 257 |

| batch_eta_s(elapsed, completed, total) | answer |
|---|---|
| total 0 | none |
| completed 0, 1, 2 | none (gate is 3 charts) |
| 3 of 10 done in 30 s | 70 s |
| completed greater than total | none |
Disagree on: nothing that reaches a different word for the same input. Two differences exist and both are per-job facts. (1) Which steps feed the clock. The startup open feeds `ByteRateClock` from the first frame, so when Copying starts the clock's window can be up to one second old and span idle time; the first Copying estimate may read one tick high. Only shown after 3 s elapsed, and it self-corrects at the next one-second window. By reading; a run on a copy of the real db with the settings rows count would settle it, but it is a rounding-sized effect, not a finding. (2) The clock origin: `view_progress_shown` counts from the request (a click can wait for an older job to exit), the other two from the job's own start. That is deliberate (the box is for the user's wait).
C1-6 (two estimators, two gates): the batch uses the mean rate since start and a 3-chart gate; the Preview and startup copy use a trailing one-second rate and a 3-second gate. CONTEXT.md lines 93-99 and 108-109 write down both rules and say the startup screen follows the Preview's. The user sees "about 1:30 left" from both, from two different rules, by design and documented. I class this K3: no input is mis-answered by either.
Owner proposed: nothing needs to move. Optional tidy: a one-line `elapsed_since(start)` helper for the two identical expressions (library_jobs.cpp 91, preview_load_job.cpp 258-259) and a home for `ByteRateClock` outside the Preview's header (library_jobs.h 40 forward-declares it from `ui/preview_load_job.h`, library_jobs.cpp 15 includes it). G6b-4's idea of a progress base class would remove about 12 lines in two jobs and cannot help `BatchJob`, which has its own pause-aware clock.
Evidence: by reading.
Found by: G6b-4, G11a-5, C1-6; verified by V9.
Scan row or reason: no useful text search. The duplicated expression `duration<double>(steady_clock::now() - started_)` could be matched, but one expression in two files is not worth a row; the real rule (the gate and the words) is already one function.

---

### M77. Is this job finished, and how does it become Ready, Cancelled or Failed?
Kind: K2 for the outcome order (same two inputs, two answers on one input, and the click shows a different word); K1 line-level for "finished" (a second flag that is never set).
Fix: the order is a choice between behaviours only if someone wants to change it; to keep behaviour and make the split visible it is code-only. The `finished()` trap is code-only.
Homes:
- `land_report`, src/ui/app_state.cpp 752-771 (inputs: `ok()`, `is_cancelled()`, `message()`, `error()`), for the path report (via `collect_path_report`) and the DM report (via `collect_dm_report`);
- `AppState::update_view_job`, app_state.cpp 348-424, test at 361-367 (same four inputs), for the click's analysis;
- readers that ignore the cancel flag: `collect_store_open` app_state.cpp 78-91 (`ok()` only), `render_dm_picker_modal` src/ui/library_dialogs.cpp 609-613 (`ok()`, `message()`, `error()`), `PreviewController::poll` src/ui/preview_controller.cpp 153, 261, 276, 288 (`ok()`; `message()`, `error()` on failure);
- the batch, which has no `ok`: "stopped" is `batch_job->is_cancelled()` read after the snapshot says finished, at src/ui/library_dialogs.cpp 493 (strip says "Stopped" or "Finished") and app_state.cpp 593 (a stopped run builds no path report).

Table of the outcome, inputs (ok, cancelled). The base class sets `ok_` from the body's return (job_base.h 83) and the cancel flag is a separate atomic, so all four pairs can exist.
| (ok, cancelled) | land_report (reports) | update_view_job (click) | collect_store_open / picker / Preview |
|---|---|---|---|
| (true, false) | Ready, result kept, message and error cleared | goes on to read the outcome (Ready, RulesBroken or Failed) | success |
| (true, true) | Ready, result kept (ok is tested first, line 756) | Cancelled, result thrown away (cancelled is tested first, line 361) | success (flag not read) |
| (false, true) | Cancelled, old result and message kept | Cancelled | failure branch; `message()` is empty because `fail()` skips it when cancelled (job_base.h 96) |
| (false, false) | Failed, message and error copied | Failed, message and error copied | failure |
Disagree on: (true, true). Same inputs, two different slots. The one place the repo writes the reports' rule in words is `cancel_dm_report`, app_state.cpp 632: "One that already finished has its result: keep it, not the cancel." The click's opposite order is supported by `close_details` (app_state.h 348-353, "nothing is saved for a song whose analysis didn't finish") and `cancel_view` (app_state.h 419-420, the panel then shows "Analysis cancelled.").

Can either path reach (true, true)?
- The click, yes, and it is a real window, not a nanosecond race. `ViewJob::run` returns true (library_jobs.cpp 544) after its last `throw_if_cancelled()` (line 533); between them runs `dynamics_for_settings` (537), which can read the chart again. A Cancel press, a new request, or the panel closing in that stretch gives (true, true). `update_view_job` then shows "Analysis cancelled." and drops a result that was complete. For a new request the old result is dropped by generation anyway (line 358), so only the Cancel button and the panel closing see it. That is what the code intends.
- The path report, only in a tiny gap. `ReportJob::run` checks `is_cancelled()` at library_jobs.cpp 613 and returns false; after that comes the `report.rows == 0` test (618), a move into `make_shared` (624) and `return true`. A cancel landing there, or in the check-then-store gap of `cancel_path_report` (app_state.cpp 721: `!finished()` then `cancel()`), gives (true, true). `land_report` then lands Ready with the full report although the user pressed Cancel. Harmless, and it matches the rule quoted from `cancel_dm_report`. Reached only through the window's Cancel button (path_report_window.cpp 195); `close_path_report` and `launch_path_report` park the job, and a parked job never lands.
- The DM report, in practice no. `DmReportJob::run` has a wide gap (it builds the report after its cancel check at dm_jobs.cpp 44), but a cancelled DM job is always parked at once by `cancel_dm_report` (app_state.cpp 637-639) or `start_dm_report` (660-663) and never reaches `land_report`. So `land_report`'s cancelled branch is reached only by the path report's own Cancel button.
- The DM fetch cannot reach it: it returns `!is_cancelled()` (dm_jobs.cpp 21).
- The batch, yes, in one more spelling. Stop disappears once the snapshot says finished (library_dialogs.cpp 422), but a Stop pressed on the frame the last chart finishes sets the cancel flag on a finished run. `render_batch_done` then says "Stopped" and `update_background_jobs` builds no path report, though every chart ran. A real edge, by reading; settling it needs a uitest that presses Stop on the last chart.

Does anything read the base class's `finished()` for the scan or batch? `ScanJob` and `BatchJob` derive from `JobBase`, not `ResultJobBase` (library_jobs.h 183 and 232). The only writers of `finished_` in the tree are `run_guarded` and `fail` in job_base.h (lines 88 and 98; grep of `\bfinished_\b` over src and tests). `ScanJob::run` ends by setting `progress_.finished` under its mutex (library_jobs.cpp 218 and 232); `BatchJob::run` ends in `finish_locked` (395), which sets `snap_.finished`. So `scan_job->finished()` and `batch_job->finished()` compile and are false forever. Grep for `scan_job->` and `batch_job->` over src and tests finds no caller of either `finished()`; every reader asks `snapshot().finished` (app_state.cpp 160, 445, 466, 578; library_dialogs.cpp 422, 492; tests/ui harness). So it is a trap, not a live bug. The "is a job running" test also has six same-shape inline spellings for the report and DM jobs (`report_job && !report_job->finished()` at app_state.cpp 467, 689, 700, 709, 721, 776), which the existing rows in tests/single_owner/rules/ui.json (lines 604-643 and 939-961) deliberately allow.
Owner proposed: (1) Keep `land_report` as the one place the reports land. (2) Give `ResultJobBase` one `outcome()` (done, cancelled, failed) using the reports' order (ok wins), and have `update_view_job` keep its extra line "cancelled after finishing: drop the result" explicitly, with the reason beside it, so the exception stops being a hidden difference. That keeps today's behaviour in every case, so code-only. (3) Make `ScanJob` and `BatchJob` either set `finished_` at their ends or hide it (a `JobBase` that has no `finished()` for jobs that publish through a snapshot). If anyone wants the batch's "stopped" to be "stopped before the last chart finished", that is a behaviour choice and needs the user.
Evidence: by reading. A uitest that presses the panel's Cancel after the analysis bar fills, using the test analyzer seam (`set_view_analyzer_for_test`) with a slow `dynamics_for_settings`, would prove the click's window; not run.
Found by: G11a-1, G11a-2, C2-6; verified by V9.
Scan row or reason: the finished-flag spelling is already covered by rows in rules/ui.json (the batch/view row at lines 604-643 and the any-job row at 939-961). A new row on `(scan|batch)_job->finished\(\)` would catch the trap if someone starts calling it. The (ok, cancelled) order cannot be caught by text; it is a rule about two branches in different functions.

---

### M78. What does Cancel do to a report build, and what does the slot say afterwards?
Kind: K3 with one small K1 inside (the same two slot writes made in two places). The timing difference is on purpose.
Fix: code-only.
Homes:
- Path report: `cancel_path_report` app_state.cpp 720-722 (sets the flag only), `launch_path_report` 705-718, `close_path_report` 730-739, `collect_path_report` 775-794 through `land_report`;
- DM report: `cancel_dm_report` 630-642, `start_dm_report` 657-672, `collect_dm_report` 796-800; window Cancel at src/ui/dm_report_window.cpp 146-150, toolbar at library_toolbar.cpp 113-114.
Tables, a Cancel press while building:
| step | path report | DM report |
|---|---|---|
| job after the press | stays in `report_job` until its thread exits | parked at once, `dm_report_job` reset |
| what `*_build()` reads right after | Building (app_state.cpp 689) until the thread stops between charts | `dm_report.last`, set to Cancelled by `cancel_dm_report` itself (640) |
| `last` when it settles | Cancelled, via `land_report` (763-764) | Cancelled (640) |
| `out_of_date_during_build` | cleared by `std::exchange` at the top of `land_report` (754-755) | cleared by hand (641) |
| old result and message | kept | kept |
| job already finished at the press | the Cancel button is not offered; `cancel_path_report` is guarded by `!finished()` | `cancel_dm_report` collects it first (633-635), keeping the result |
| window afterwards | stays open, "Report cancelled." with Try again (report_window.cpp 198-200) | the window's own Cancel then sets `window_open = false` and reopens the picker (dm_report_window.cpp 146-150); the toolbar's "Compare with dmleaderboards..." keeps the window open if it was, so "Report cancelled." is reachable there |
Why the timing differs, in the code's own reasons: the DM fetch cannot be joined quickly (WinHTTP checks cancel only between reads, app_state.h 595-598) and the DM Cancel closes the window (D103 item 10); the path report stops between charts and `request_path_report` refuses to start a second build while one runs (app_state.cpp 700), so its Cancelled has to land first.
Disagree on: how soon the screen says Cancelled (immediately for the DM report, after the current chart for the path report). Final slot state is the same. The two slot writes `last = Cancelled; out_of_date_during_build = None` exist at app_state.cpp 640-641 and, via `land_report`, 754-764. Agree today.
Owner proposed: leave both flows. Optional: one `ReportSlot::mark_cancelled()` used by `land_report` and `cancel_dm_report`. The path report's `close_path_report` (resets the whole slot) against the DM report having no close is G1b-7's question, not this one.
Evidence: by reading. A uitest that presses Cancel on each window would show the one-frame difference; not needed to settle the slot state.
Found by: G11a-3; verified by V9.
Scan row or reason: no text search catches it; the two writes are different statements in different functions, and there is no distinctive spelling. If `mark_cancelled()` were added, a row on `last = ReportBuild::Cancelled` outside report_state.h and app_state.cpp would catch a new hand copy.

---

### M79. How is "this job was cancelled" thrown, and how does it read on screen?
Kind: K1 for the throw (line level, one construction in three modules); the screen words are K3, one word per state, each written once.
Fix: code-only for the throw. The screen words are not touched.
Homes:
- `JobCancelled`, src/ui/job_base.h 24-26, raised by `throw_if_cancelled` and by the Preview and startup jobs;
- `OpenCancelled`, src/audio/stem_reader.h 72-74, raised in src/audio/opus_reader.cpp 98, 109, 205 and src/audio/song_audio.cpp 64;
- `throw_cancelled`, src/net/dmbot_client.cpp 47;
- a fourth with different text: src/store/record_store.cpp 971, `KindedError(ErrorKind::Cancelled, "opening the database was stopped")`.
Each of the first three builds `KindedError(ErrorKind::Cancelled, "cancelled")`. `AnalysisCancelled` (src/app/analysis.h 188) is a plain struct that is not an `std::exception`, on purpose (comment 184-187), so it is not a copy.
Tables:
| thrown at | type | kind | raw text | caught by |
|---|---|---|---|---|
| job step | `JobCancelled` | Cancelled | "cancelled" | library_jobs.cpp 547 by type; `run_guarded` as `std::exception` |
| audio open | `OpenCancelled` | Cancelled | "cancelled" | by type at preview_load_job.cpp 157 and song_audio.cpp 58 |
| network read | `KindedError` | Cancelled | "cancelled" | by kind only |
| database open | `KindedError` | Cancelled | "opening the database was stopped" | by kind at record_store.cpp 656 and 899 |
Words on screen, each from the state, not from the exception: "Analysis cancelled." (src/ui/details_panel.cpp 188, click state Cancelled), "Scan cancelled. The library was left unchanged." (src/ui/library_dialogs.cpp 295, scan modal), "Report cancelled." (src/ui/report_window.cpp 199, both report windows), "Cancelling..." (details_panel.cpp 158, while a click's job winds down). The exception's own sentence is `kStopped` = "Stopped before it finished." (src/app/user_messages.cpp 58, chosen at 75). I found no path that shows `kStopped`: every raise of a Cancelled-kind error happens with the job's cancel flag set, and `ResultJobBase::fail` then leaves the message empty (job_base.h 96). The raw word "cancelled" is also never drawn (the Cancelled branches show no `error()`). So `kStopped` and the raw text are unreachable on screen today, and `OpenCancelled` against `JobCancelled` must stay two types because two places catch the audio one by type.
Disagree on: none today for the kind; the four raw texts differ but are never shown.
Owner proposed: put one `Cancelled` error type in core/error_kind.h (a `KindedError` subtype with kind Cancelled and the text "cancelled"); `JobCancelled` and `OpenCancelled` derive from it so the by-type catches keep working; `throw_cancelled` throws it. record_store.cpp 971 uses it too. Why: the three layers (ui, audio, net) cannot include each other, but all include core.
Evidence: by reading.
Found by: G9-5; verified by V9.
Scan row or reason: a row on the construction `KindedError\(\s*ErrorKind::Cancelled` outside core/error_kind.h would flag a new hand copy; it matches the four sites above, so the row must come with the merge.

---

### M80. May a batch (or analysis) start now, and how many charts does the Analyze button cover?
Kind: K1 for the two-input gate (blocked, running); the zero tests are two different questions on purpose (not a finding); the count is K1 that agrees.
Fix: code-only.
Homes:
- toolbar `analyze_off`, src/ui/library_toolbar.cpp 94: `analyzable == 0 || analysis_blocked() || batch_busy`;
- confirm `can_start`, src/ui/library_dialogs.cpp 391: `to_run > 0 && !analysis_blocked()`;
- `AppState::start_batch`, app_state.cpp 556-558: `analysis_blocked()` then `batch_running()`;
- `AppState::open_batch_confirm`, app_state.cpp 526-548: no guard;
- for comparison the scan's one named gate `AppState::can_scan`, app_state.cpp 458-460, and the existing row for it in rules/ui.json 906-938.
Table, edges included (analysis_blocked, running, chart count):
| situation | toolbar button | confirm Start | `start_batch` |
|---|---|---|---|
| empty library (0 rows) | off | not reachable | would start a job over an empty plan (reachable only by a direct call, app_state.cpp 559-560 says tests do) |
| search matches 0 rows | off ("Analyze search (0)...") | not reachable | same |
| all 5 rows already have a result, redo unticked | on (counts matched rows) | off, text "Every chart here already has a result." (library_dialogs.cpp 348) | would start over an empty to-do plan |
| same, redo ticked | on | on, `to_run` 5 | runs 5 |
| bad hydra_rules.ini | off | off | returns |
| batch running | off, tooltip | not asked (the confirm cannot be open: only the toolbar button opens it, and it is off) | returns |
The count: toolbar N = `searching ? library_match_count() : rows().size()`, with `library_match_count()` = `library.counts().all` (app_state.h 314), built in `refilter` from the same `matches_query` (library_model.cpp 283-285). `open_batch_confirm` uses `library_matches()`, which starts from `library.matches()` (library_model.cpp 217-223, the same filter) and then keeps only rows the store still lists (app_state.cpp 137-157). Agree today. They split only when the library reload failed (a status message already says so), where the confirm counts fewer rows than the button's N.
Disagree on: nothing reachable. (a) The confirm does not ask "running": unreachable. (b) `start_batch` does not ask "nothing to run": reachable only from tests. (c) "Zero" means matched rows on the toolbar and to-do rows in the confirm, deliberately (the confirm can say "Every chart here already has a result").
Owner proposed: add `AppState::can_analyze()` = `!analysis_blocked() && !batch_running()`, named like `can_scan`, and call it from the toolbar, the confirm and `start_batch`. The zero tests stay with each screen, because they count different things. Why: the pair of conditions is the rule; the counts are per screen.
Evidence: by reading. A uitest that opens the confirm, then starts a batch from a second harness call while one runs, would prove (a); not worth running.
Found by: G11a-4, G11b-1; verified by V9.
Scan row or reason: a row could match `analysis_blocked\(\)[^;]*batch_(running|busy)` or the reverse order outside app_state.cpp. It must list the toolbar's `batch_busy` local as part of the same expression, so it is brittle; the existing `batch_running` row (rules/ui.json 604-643) does not cover it.

---

### M81. How is "a batch is running" worded? (needs the user)
Kind: K2. One state (`batch_running()`), wording that differs by where it reaches the screen. The predicate has one owner; only words repeat.
Fix: needs the user (display text).
Every wording, exact, with its file and where it reaches the screen:
1. `AppState::kBatchRunningStatus` = "A batch is running." (src/ui/app_state.h 473). Posted with `set_status` at app_state.cpp 483 (a report row click that would switch the chart mode while the settings are locked) and 518 (a scan requested by the song panel's "Rescan library" or the folder dialog's "Scan now" while a batch runs). Reaches the screen as the fading status line under the toolbar (library_toolbar.cpp 20-41; also drawn inside the Song folders box, library_dialogs.cpp 239). Neutral colour, fades after `kStatusFadeSeconds`.
2. "Busy: a batch is running." (literal, src/ui/library_toolbar.cpp 53, in the `busy_tooltip` lambda). Reaches the screen as a hover tooltip on the greyed "Scan library" button (line 75) and the greyed "Analyze library..." / "Analyze search (N)..." button (line 98). It shows whenever a batch runs, even if the Scan button is also off for another reason.
3. `kSettingsLockedText` = "Stop the batch to change these." (src/ui/library_parts.h 88). Reaches the screen as the lock line at the bottom of the Analysis settings panel (settings_bar.cpp 219-221) and as the tooltip of the greyed "Reset to defaults" button (library_dialogs.cpp 145). The settings button's own label also ends in " (locked)" (library_dialogs.cpp 137).
4. The batch confirm's paragraph, src/ui/library_dialogs.cpp 385-387: "It runs in the background, so you can keep browsing. Analysis settings stay locked until it finishes or you stop it. Stop keeps every result finished so far." It is shown before a batch starts, so it is a forewarning, not a refusal.
5. The Stop button's tooltip, library_dialogs.cpp 481-482: "Keeps the N charts already finished." (N is `s.analyzed`). Not about the lock; listed so nobody counts it as a wording.
Table (same predicate `batch_running()`; `settings_lock()` wraps it for 3 and 4):
| moment | wording |
|---|---|
| clicked something that needs the lock released, no disabled button to explain | 1 |
| hovering a disabled Scan or Analyze button | 2 |
| looking at the locked settings | 3 |
| about to start one | 4 |
Disagree on: 1 against 2. Same state, same moment type (a refusal), the second adds "Busy: " in front. 3 and 4 say what to do about the lock or warn about it, so they are different sentences for different jobs. The existing scan row (rules/ui.json 816-843) pins sentence 1 and lists sentence 2 under `must_not_match`, so the row's author chose to leave the tooltip alone.
Question for the user (the real rule, quoted): when the batch is running, the status line says "A batch is running." and the greyed Scan or Analyze button's tooltip says "Busy: a batch is running." Do you want one sentence for both, and which: the plain one, or the one with "Busy: "? (The panel's "Stop the batch to change these." and the confirm's lock sentence stay as they are unless you say otherwise.)
Owner proposed: if one sentence wins, `kBatchRunningStatus` (rename to a refusal-sentence name) and the tooltip calls it.
Evidence: by reading.
Found by: G11a-6, G11b-7; verified by V9.
Scan row or reason: the existing row on `"A batch is running\."` catches the status sentence. A second row on `[Bb]usy: a batch` would catch the tooltip literal if it came back after a merge.

---

### M82. How is "analysis is off" worded? (needs the user)
Kind: K2. One state (`analysis_blocked()`: `rules_error` is non-empty), three sentences, one with the file's error under it and one without.
Fix: needs the user (display text).
Every wording, exact, with its file and where it reaches the screen:
1. `AppState::kAnalysisOffSentence` (src/ui/app_state.h 279-281): "hydra_rules.ini has an error, so analysis is off until the file is fixed and Hydra is restarted." Drawn in warning colour by the toolbar (src/ui/library_toolbar.cpp 148-152), always, under the toolbar, with `rules_error` in a second line beneath it; and by the path report window when no report exists (src/ui/path_report_window.cpp 160-161, drawn at src/ui/report_window.cpp 166-170, sentence then error text).
2. Literal in `render_panel_notices` (src/ui/details_panel.cpp 143-145): "Analysis is off until hydra_rules.ini is fixed and Hydra is restarted." Drawn in warning colour in the song panel whenever the panel is open (called at details_panel.cpp 262). No error text under it.
3. `kRulesFile` (src/app/user_messages.cpp 48-50): "hydra_rules.ini has a line Hydra can't read. Fix or delete that line, then restart Hydra." It is the plain sentence for a `RulesFileError`. I found no GUI path that turns a `RulesFileError` into this sentence: `load_startup_settings` (app_state.cpp 23-31) catches it and stores `e.what()` raw into `rules_error`, and the CLI prints `e.what()` (src/cli/batch.cpp 113-115). Only tests/test_user_messages.cpp 94-95 and 171-172 pin it. By reading, so it does not reach the screen today.
What `rules_error` looks like (the second line under 1): the loader's raw message, built at src/app/rules_file.cpp 18, 77, 85, for example `<path>: unknown key "x"`.
Table (`rules_error` non-empty):
| surface | words | file's error text follows |
|---|---|---|
| toolbar | 1 | yes |
| path report window, nothing built | 1 | yes |
| song panel | 2 | no |
Both the toolbar and the panel show together whenever a song is selected, so with a bad rules file one fact is on screen twice with two wordings (1 and 2). Other effects of the same state: the "Analyze library..." button is greyed with no explanation of its own (library_toolbar.cpp 94), the confirm's Start is off (library_dialogs.cpp 391), and a click counts only the Dynamics (`ViewedSong::State::RulesBroken`, set at app_state.cpp 403, which no display reads).
Question for the user (the real rule, quoted): with a broken hydra_rules.ini the toolbar and the path report say "hydra_rules.ini has an error, so analysis is off until the file is fixed and Hydra is restarted." and then show the file's error, while the song panel says "Analysis is off until hydra_rules.ini is fixed and Hydra is restarted." and shows no error. Should the song panel use the toolbar's sentence (and the error), or should it stay shorter because the toolbar already shows the error?
Owner proposed: `kAnalysisOffSentence` stays the one; the panel calls it. Its header comment (app_state.h 277-278) lists only the toolbar and the path report as readers and would need the panel added. `kRulesFile` could be left as the CLI-less dead sentence or dropped; that is a separate small question.
Evidence: by reading. A uitest with a bad rules file (the uitest harness can set `rules_error`) would show both lines together; not run.
Found by: G11a-7; verified by V9.
Scan row or reason: a row on the panel's literal `Analysis is off until` would catch it coming back. The other wording is an owned constant.

---

### M83. Where do user-facing failure sentences live, and how is the path report's outcome worded? (needs the user for the Failed wording)
Kind: K3 for the kind sentences (one switch, `kind_sentence`, src/app/user_messages.cpp 73-101, which is sound). K1 for the words that the three path-report surfaces write out as literals (identical text, three files). K2 for the DM report against the path report ("Could not build the report." against "The path report could not be built.") and for the status line dropping the raw error.
Fix: the K1 part is code-only (one sentence constant, same words). The K2 part, and the status-line change, need the user (display text).
Homes of the batch's path report outcome:
| state | finished strip (src/ui/library_dialogs.cpp 537-545) | status line (src/ui/app_state.cpp 789-793) | window (src/ui/path_report_window.cpp 154, 162; report_window.cpp 173-214) | toolbar (library_toolbar.cpp 140) |
|---|---|---|---|---|
| Building | "Building the path report..." (line 538) | none | "Building the report from your library..." | button "Building path report..." |
| Ready | "The path report is ready." (544) with an "Open report" button | `set_status("The path report is ready.")` (791) | the table | button "Open path report" |
| Failed | "The path report could not be built." (540, warning colour), then `path_report.message`, then `path_report.error` dimmed | `set_problem("The path report could not be built. " + path_report.message)` (793), no raw error | "The path report could not be built." (162), then the message, then the raw error dimmed | n/a |
| Cancelled | nothing | nothing | "Report cancelled." (report_window.cpp 199) with Try again | n/a |
| stopped batch | "Every result finished before Stop is kept." (536); no report is built | n/a | n/a | n/a |
Who speaks when: the strip while `batch_job` exists and `report_started` is true; the status line only if the strip was dismissed before the report landed (`if (batch_job) return;`, app_state.cpp 789); the window whenever it is open. A failed batch report can therefore show in the strip and the window at once, never in the strip and the status line at once.
Other failure sentences written outside the one switch (exact):
- DM window: "Could not build the report." (src/ui/dm_report_window.cpp 123), and its building lines "Fetching scores and building the report..." and "The leaderboard server can take a moment to wake up." (121-122);
- the picker: "Loading the dmleaderboards user list..." and "The leaderboard server can take a moment to wake up." (library_dialogs.cpp 599-600), "Could not load the user list:" (611);
- "Analyzed, but saving failed. " + plain error (app_state.cpp 412, status line, problem colour);
- "Settings could not be saved — " + ini path + " is not writable." (app_state.cpp 849-850);
- "Failed to write chart library: " + raw `e.what()` (src/app/analysis.cpp 442), shown raw in the scan modal's problem list (library_dialogs.cpp 303-311), the one case where raw text is the user-facing text;
- "Preview failed: " (src/ui/preview_tab.cpp 365), "Dynamics failed: " (src/ui/dynamics_tab.cpp 135), "Hydra couldn't use your audio device; the preview is muted." (preview_tab.cpp 391);
- kinds that carry their own sentence (`AlreadyPlain`, user_messages.cpp 85): `no_notes_message` (src/parse/song.cpp), `why_not_comparable` (src/app/dm_report.cpp 33-49), `kNoChartLibrary`, and `report.why_empty` thrown at library_jobs.cpp 619-620.
The header claim "Every job and view that shows an error takes the wording from here, so it lives in one place." (src/app/user_messages.h 4-5) is not true as written; it is true of the sentences chosen by kind.
Disagree on: (a) "The path report could not be built." is the same text in three files and agrees today; a wording change must be made three times. (b) The status line leaves out the raw error and joins the sentence and message with a space; the strip and the window show three lines. An empty message would leave a trailing space. (c) The DM report's failure sentence has a different word order and no noun: "Could not build the report." against "The path report could not be built."
Questions for the user (the real rule, quoted): when the batch's path report fails, the strip and the window show "The path report could not be built." then the plain message and then the raw error; the status line (after the strip is dismissed) shows "The path report could not be built. <message>" with no raw error. Should the status line also carry the raw error, or is the short form right? Second: the comparison window's failure reads "Could not build the report." where the path report's reads "The path report could not be built."; should the comparison say "The comparison could not be built." to match?
Owner proposed: a `kPathReportFailed` constant next to the other app sentences (not in `kind_sentence`, which is for exceptions), read by the strip, the window and the status line; "Ready" the same way. The three-line failure layout (warning sentence, plain message, dimmed raw error) is drawn in four places (library_dialogs.cpp 540-542, 558-559 and 611-613, report_window.cpp 203-206) and could be one small helper in widgets, but that is a layout rule, not a word.
Evidence: by reading. A uitest that fails the report job (the test analyzer seam `set_report_analyzer_for_test`) with the strip dismissed would show the status line text; not run.
Found by: G9-4, G11a-10, G11b-2, G11b-6, and the "path report is ready" lead; verified by V9. G11b-6 (the "can take a moment to wake up" sentence in library_dialogs.cpp 600 and dm_report_window.cpp 122) agrees today and the second site's comment names the first; it is K1, code-only, and fixed by the same constant idea.
Scan row or reason: rows on `The path report could not be built\.` and `The path report is ready\.` and `can take a moment to wake up` outside the owner file would flag a copy coming back. The sentences are text, so text search catches them.

---

### M84. How is a modal named, and who opens it?
Kind: K3 at worst. Bare strings are used as ImGui popup ids, but each id is shared by an opener and its owner in the same screen module, and the tests drive the dialogs by these labels, so a rename is not silent.
Fix: code-only.
Homes (exact literals and where):
- "Scanning charts": opened at src/ui/library_toolbar.cpp 72 (after `start_scan()`, unconditionally) and src/ui/library_view.cpp 185 (only `if (app.scan_job)`); owned at src/ui/library_dialogs.cpp 277.
- "Song folders": opened at library_toolbar.cpp 64; owned at library_dialogs.cpp 174.
- "Analyze library": opened, tested and owned in one function, library_dialogs.cpp 334-337. (The toolbar button's label "Analyze library..." is a different string and is not an id.)
- "Remove folder?": library_dialogs.cpp 224, 242, 243, 244, one function.
- "Compare dmleaderboards user": opened at library_dialogs.cpp 589 (from the `dm_picker_popup_pending` flag set by `reopen_dm_picker`, app_state.cpp 651-655) and owned at 591.
- The settings panel is the only one with a constant, `kPanelId` (src/ui/settings_bar.cpp 36).
Table: opener passes literal L; owner expects L. Match: all five. A mismatch means the modal never opens, no error. Tests: tests/ui/uitest_library.cpp, uitest_details.cpp, uitest_batch_reports.cpp and uitest_harness.cpp each name these labels, so a one-sided rename fails those scripts. Only the two ids that cross a file boundary matter: "Scanning charts" (3 files) and "Song folders" (2 files).
The scan modal opened from the toolbar when `start_scan` made no job is safe: `render_scan_modal` is only called `if (app.scan_job)` (library_view.cpp 195), and the button is greyed by the same `can_scan`. The existing row for the scan gate lists `if (app.scan_job) ImGui::OpenPopup("Scanning charts");` as an allowed line (rules/ui.json 921-925).
Disagree on: none today.
Owner proposed: two `constexpr const char*` names in library_parts.h (`kScanModalId`, `kFoldersModalId`), read by the opener and the owner. Optional for the other three, which stay inside one function.
Evidence: by reading.
Found by: G11b-5; verified by V9.
Scan row or reason: a row on the two literals `"Scanning charts"` and `"Song folders"` outside library_parts.h and library_dialogs.cpp would flag a new bare copy; the owner would be the constants.

---

### Side notes

- `ScanJob::run` (src/ui/library_jobs.cpp 180-233) has no try/catch around `discover_charts` (only the cache read is wrapped); `BatchJob::run` and `ResultJobBase::run_guarded` do catch. An exception escaping `discover_charts` on that thread would end the process. I did not check whether `discover_charts` can throw.
- `render_batch_done` (library_dialogs.cpp 493) and `update_background_jobs` (app_state.cpp 593) read "stopped" from the cancel flag after the batch finished; see M77 for the edge where Stop lands on the last frame.
- `kRulesFile` (user_messages.cpp 48-50) is pinned by tests but has no GUI caller (see M82).
- `StoreOpenProgress::time_left_text` forward-declares `ByteRateClock` from the Preview's header (library_jobs.h 40); a tidy-up, not a bug.
- `start_batch` starts a `BatchJob` over an empty plan if called directly with nothing to run (app_state.cpp 556-575); only tests can reach it.
