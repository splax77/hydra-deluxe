# Album in the library: scanned, stored, shown, searched (tasks ALB-1 to ALB-5)

Session dcae32ba, 2026-10-24. Planner aca57e517e3d92f29 (task ALB-PLAN) read every file named below on main at 5f357402. Nothing is built. The build waits for the user's yes, and for the three questions at the end.

## What the player gets

Every chart's album comes into Hydra with the scan, the way its artist does. The full-size library gets an `Album` column between `Artist` and `Charter`. The search box learns `album:`, and a plain word matches an album the same way it matches a title or an artist. When the song panel is open and the table is narrow, a search that matched an album shows the album on the row's second line, under the Artist column, with the match in gold, and the footer counts it ("Matched on album.").

A chart with no album shows an empty cell and never matches. There is no placeholder like the artist's `(unknown)`.

The first scan after the update reads every chart once more, because the stored rows do not hold an album yet. No result is re-analyzed; only the names are read again. The scan window shows its counted "Reading charts..." bar while it runs, as it does for any scan whose cache was dropped.

Out of scope, by the user's decision 8: the report windows, `hydra_batch`, the song panel, and sorting or grouping by album.

## The user's decisions (chat, 2026-10-24)

These are fixed; the plan does not reopen them.

1. The full-size library gets an `Album` column between `Artist` and `Charter`.
2. Album's width comes out of the Folder column only. Start at Album 14 and Folder 25 (today Title 17, Artist 10, Charter 9, Folder 39, Best path 25, in `library_column_specs`, `src/ui/library_table.cpp` line 246). The final split is settled by a real-app check on the user's library before the merge.
3. In the narrow view, album joins the second line: under the Artist column, matches highlighted, whenever the second line is drawn and Album is hidden. The footer counts album. Rows stay one line tall without a search.
4. A new `album:` key beside `artist:`, `charter:` and `folder:`. Plain words match album too. The keys are listed in the Search tips popup (decision 10), not in a hint line.
5. A blank album shows an empty cell and never matches. Album text is cleaned the way an artist is (tags stripped, ends trimmed), by one owner beside `display_artist` and `display_charter` in `src/parse/song.h`.
6. The scan reads `album` from song.ini and from a `.sng`'s metadata, and takes the album the `.srb` parser already reads (`SrbMetadata::album`). A `.chart` file's own `[Song] Album` is out of scope. Confirmed: the scan reads no `.chart` metadata at all today. A folder chart's names come only from its song.ini (`discover_charts`, `src/app/analysis.cpp` line 663: `pc.kind == ChartKind::Folder ? read_metadata_ini(pc.ini_path) : ...`).
7. A new `album` column in the `charts` table. `kChartMetaStamp` goes from 5 to 6, so the next scan drops its cache and reads every chart's metadata once, with no re-analysis. The upgrade follows the store's existing add-a-column pattern.
8. Out of scope: report windows, batch CLI, details panel, sorting and grouping by album.
9. The User Guide's search section gets the `album:` key and the `Album` column, in the public doc style, and the docs-vs-code test keeps passing.
10. The long hint line under the search box goes away (option B from the mock, chosen in chat 2026-10-24). In its place is one short line with a "Search tips" link. Clicking the link opens a small popup with one key per line: the key with an example, then what it does in plain words. The box's placeholder becomes "Search songs". New keys go into the popup, so the line under the box never grows again.

## What the planner found that the decisions did not cover

### A saved hydra_ui.ini keys column state by column ID, and the IDs are the sort values

**Correction (2026-10-24, found by ALB-3 and confirmed in the code):** this section is wrong. `TableSetupColumn` sets a column's ID to `ImHashStr(label)` (`third_party/imgui/imgui_tables.cpp` line 1752); the user ID only travels to the sort spec. Saved hidden state and sort follow the header labels, so inserting Album could not shift them whatever IDs it used. The IDs were kept as planned, for a clean sort round trip. D263 item 1 records the true reason.

The ini question in the brief has a clear answer in the vendored ImGui. A table's saved state is one block per table ID, with one line per column, and each column line carries the column's ID (`TableSaveSettings`, `third_party/imgui/imgui_tables.cpp` line 3954: `column_settings->ID = column->ID`). That ID is the `user_id` Hydra passes to `TableSetupColumn` (line 1729), which `setup_column` (`src/ui/column_widths.cpp` line 278) takes from the library's call: `static_cast<ImGuiID>(LibrarySort::Artist)` and so on (`library_table.cpp` lines 291 to 303). On load, ImGui matches saved lines to columns by that ID first (`TableLoadSettingsForColumns`, lines 4028 to 4052). A changed column count does not discard the block; it only marks it dirty (line 3999).

So the saved state follows the IDs, not the positions. Today the IDs are Title 0, Artist 1, Charter 2, Folder 3, Best path 4 (`LibrarySort`, `src/ui/library_model.h` line 32). If Album took value 2 and the others shifted up, a saved "Charter hidden" would land on Album, "Folder hidden" on Charter, and a saved Best path sort on Folder. The User Guide promises "Hydra Deluxe remembers which columns are hidden and the sort" (line 158), and the user's memory rule says a user-facing change is a blocking question up front.

The plan keeps every existing ID where it is and gives Album the next free one: `LibrarySort { Title = 0, Artist = 1, Charter = 2, Folder = 3, BestPath = 4, Album = 5 }`. The column positions become their own list, since a column no longer sits at its sort value: `kColumnTitle 0, kColumnArtist 1, kColumnAlbum 2, kColumnCharter 3, kColumnFolder 4, kColumnBestPath 5`, with `kLibraryColumnCount` still `kColumnBestPath + 1`, and two small functions in `library_model.h` that are the one place the two lists meet: `LibrarySort sort_of(int column)` and `int column_of(LibrarySort)`. `library_cell_text` switches on `sort_of(column)` instead of a cast; `setup_column` passes `sort_of(c)` as the user ID; the sort spec still maps `ColumnUserID` straight back to a `LibrarySort`. The single-owner row "Which column is Best path, and how many columns are there?" (`tests/single_owner/rules/ui.json` line 1447) keeps `library_model.h` as the owner; its pattern `constexpr int kColumn\w+ = \d` already forbids a column number written anywhere else. The comment at line 34 ("each column sits at its LibrarySort value") is rewritten to say the two lists and the two functions. Question 1 at the end offers the cheaper alternative (a new table ID string, which forgets the saved state once) in case the user prefers it.

With that, Album has no saved line in anyone's ini, so it starts visible, and the existing five lines land where they always did. The GUI test `library-columns-at-start` (`tests/ui/uitest_library.cpp` line 1063) already writes an ini by column ID and checks the hidden state lands on Charter and Folder; it gains one check that the same ini, written with today's five IDs and a `ColumnsCount` of 5, still hides Charter and Folder and leaves Album visible. That test is the proof the saved state survives the insert.

### Album hides with the song panel

The brief's decision 3 says what happens "whenever that second line is drawn and Album is hidden", and the second line is drawn in the narrow view, which is the panel-open view. Today the panel hides Charter and Folder (`library_table.cpp` lines 316 to 320) so the narrow table is Title, Artist, Best path. The plan has Album hide and show with them, for the same reason (room), so the narrow view keeps its three columns and the album moves to the second line. This is question 2; the plan builds it this way unless the user says otherwise.

### The second line when Album is the only hidden column

Today `two_lines` is `searching_words && (!folder_shown || !charter_shown)` (line 342), and the title's second line shows the folder by default, or "charted by X" when the charter matched (`second_line`, lines 221 to 241). Every row has the same height because the clipper needs it, so the choice is table-wide, not per row.

The choice: `two_lines` becomes `searching_words && (!folder_shown || !charter_shown || !album_shown)`. The title's second line keeps today's rule exactly, but only when Folder or Charter is hidden; when Album is the only hidden column, the title's second line is empty, so the folder is not written twice (it is right there in its own column). Under the Artist column, the second line shows the row's album whenever Album is hidden, with `match_spans(q, QueryField::Album, album)` over it, blank for a blank album. Reasons: a search that matched an album the user cannot see still says where it matched, which is the whole point of the second line; and it is the same rule the folder follows (the folder shows on every row once Folder is hidden, matched or not), so there is one behaviour to learn. The cost is a two-line table whenever a word search is on and Album is hidden, which is already true for Folder and Charter. One gap, stated so nobody is surprised: if the user hides both Album and Artist from the header menu, the album line has no column to sit under and is not drawn; the footer still says "Matched on album." That is the same gap Title's second line would have if Title could be hidden, and it cannot.

The footer (`render_footer`, lines 475 to 486) today picks from four fixed strings. With three columns there are seven combinations, so it builds the sentence once: "Matched on " plus the names in the fixed order folder, charter, album, joined with ", " and a final " and ", plus ".". The two existing sentences the GUI test looks for ("Matched on folder." at `uitest_library.cpp` line 478, and "Matched on folder and charter.") come out letter for letter.

### Every other copy of the column list or the field list

Grepped on main. The column list lives in `library_model.h` (the constants and `LibrarySort`), `library_table.cpp` (`library_column_specs`, the `setup_column` calls, the `searched_cell` calls, the panel's hide pair) and `library_model.cpp` (`library_cell_text`, `set_charts`). The GUI tests read the constants (`uitest_library.cpp`; `uitest_details.cpp` only reads `library_hidden`). The unit test `tests/test_column_widths.cpp` has its own hand-made five-column fixture (line 57) that is a `place_columns` example, not the production list, so it stays. `tests/single_owner/rules/ui/column-widths.json` line 59 quotes a specs line as a `must_match` example; it still matches. `tools/hydra_ui_mcp.py` holds no column names and no search keys (grepped for Artist, Charter, Folder, `artist:`). `docs/agents/ui-testing.md` line 113 lists the table's columns; it is an internal page and ALB-3 updates it.

The search field list lives in `library_query.h` (`QueryField`, `SearchableRow`), `library_query.cpp` (`field_named` line 206, `term_matches` line 268, `make_searchable` line 362, `search_field` line 372), the two hint strings in `library_table.cpp` (lines 106 and 129), and the public pages: `docs/UserGuide.md` lines 156, 188 to 193 and 201 to 203, `docs/differences-from-public-hydra.md` line 45 (lists every filter), `README.md` line 18 ("such as", so it need not change). `CONTEXT.md` lines 39 to 46 (Scanned chart) and 246 to 254 (Library search) describe both.

The report pages call `search_field(song, artist, charter, path)` (`src/app/path_report_view.cpp` line 165, `dm_report_view.cpp` line 194, `fill_report.cpp` line 346). They have no album and are out of scope, so `search_field` keeps its signature and hands `make_searchable` an empty album.

### The precheck will print F lines for this change

`hydra_precheck`'s check 5 (`tools/precheck/check_5.cpp` line 28, `kMinSharedFields = 4`) reports a record that shares four or more field names with a record in another file when the range added one of the shared fields. `ChartMeta` (`analysis.cpp` line 128) and `store::ChartLibraryEntry` share title, artist, charter and timing today, and `album` makes five; `LibraryRow` and `SearchableRow` reach four with it. These are the one scanned-chart shape carried through the scan, the store and the table, each holding only what its stage needs (`CONTEXT.md` line 39 says so). The reviewer lists them under "Precheck lines dropped" with that sentence (`docs/agents/derive-once-review.md` line 118). Each executor's report says the same so the reviewer does not hunt.

### The first rescan and the UI

Dropping the cache means the next scan hashes every notes file and reads every song.ini again, which on the user's library is minutes. The scan already runs on `ScanJob`'s own thread and reports `on_charts(done, total, cached)` from the calling thread (`analysis.cpp` line 686, `src/ui/library_jobs.cpp` line 172), and the scan window draws "Reading charts..." with a counted bar (`src/ui/library_dialogs.cpp` lines 289 to 291). Nothing in this plan touches that path, so no code change is needed; the real-app gate watches the bar move on the user's library, and that rescan over the whole library needs the user's yes in chat first (CLAUDE.md rule 8).

## How the code changes, by layer

**Cleaning (one owner).** `src/parse/song.h` gains `display_album`: tags stripped and the ends trimmed, no fallback, a blank in gives a blank out. In `song.cpp` it is `clean_name(album)`, the same helper `display_charter` is (line 626). The comment says it has no placeholder because the scan stores none for an album (decision 5).

**Scan.** `ChartMeta` gains `album`. `read_metadata_ini` reads the `album` key (line 146 pattern), `parse_sng_metadata` the lower-cased `album` key (line 169 pattern), `srb_chart_meta` copies `md.album` (line 189 pattern). The final loop in `discover_charts` (line 701) sets `r->album = trim(r->album)` beside the three names, with no `_or_unknown`. A cached row from an older scan never reaches that loop with a missing album, because the stamp bump drops the whole cache.

**Store.** `ChartLibraryEntry` gains `std::string album` after `charter` (`record_store.h` line 327). The `charts` table's `CREATE TABLE` gains `album TEXT` (`record_store.cpp` line 799), `kLibraryColumnList` gains `album` (line 1408; the `static_assert` count goes to 10 and its message with it), `read_library_entries` and `rebuild_chart_library` take the new slot (lines 1430 and 1460; the slot numbers after it move up by one). For a file that already exists, `library_file.cpp`'s `kind_of` gains a generation after the failure-columns check: a `charts` table without an `album` column is `LibraryFileKind::LacksAlbumColumn`; `Current` now means the column is there. `set_up_schema` (line 816) adds the column for every kind older than Current: after `convert_results` for Convertible, after `add_failure_columns` for LacksFailureColumns, and alone for LacksAlbumColumn, through one `add_album_column` shaped like `add_failure_columns` (lines 820 to 833): one `ALTER TABLE charts ADD COLUMN album TEXT` in a transaction, failure rethrown as `DatabaseUpgrade`. `ALTER TABLE ADD COLUMN` rewrites no rows in SQLite, so the open stays quick and needs no progress step. `kChartMetaStamp` becomes `{6, {6}}` with the line "6 = the scan also reads each chart's album" (`stored_versions.h` line 97).

**Search.** `QueryField` gains `Album`; `field_named` maps "album"; `SearchableRow` gains `album`; `make_searchable` takes a fifth argument, the album, folded and stripped like the others; `term_matches` lists the Album column; `search_field` passes `{}`. `match_spans` needs no change: it already asks `term_applies_to`. The header comment at line 65 names `album:x`.

**Table.** `library_column_specs` lists six columns: Title 17, Artist 10, Album 14, Charter 9, Folder 25, Best path 25, which sum to 100 (the debug assert in `place_columns` and `text_shares_sum_to_100` in the GUI test both check that). `render_table` sets up the Album column at `kColumnAlbum` with the user ID `sort_of(kColumnAlbum)` and `ImGuiTableColumnFlags_NoSort` (decision 8; question 3 asks whether to drop the flag), hides and shows it with the panel beside Charter and Folder, and draws its cell through `searched_cell(kColumnAlbum, QueryField::Album)`. `LibraryRow` gains `album` (display text) and its `searchable.album`; `set_charts` fills both; `sort_keys` keeps one slot per column before Best path, so it grows by one. `SecondLineUse` gains `album`; `two_lines`, the second line and the footer change as described above. The table task leaves the search box and the lines under it alone; ALB-5 owns them.

**Search tips (decision 10).** The list of keys and what each one does is a fact about the query language, so its one home is the search owner, not the table. `library_query.h` gains `search_tips()`, which returns one entry per key in display order: the key with its example, and a plain meaning. The entries are `title:actors`, `artist:green`, `album:dookie`, `charter:boo`, `folder:ib24`, `"tier 4"` (exact phrase), `stars:7`, `squeeze<=20` and `squeeze>20`. Each example word comes from a chart in `testdata/input`. A unit test parses every example through `parse_library_query` and checks it produces the filter or field term its entry names. So an entry can never list a key the parser doesn't know, and the parser test fails if a key is renamed without its tip. The meanings follow the code, not the old spec. `stars:N` keeps rows whose stored best path gets exactly N stars (`query_matches`, `*facts.stars != *q.stars`). `squeeze<=N` keeps rows whose hardest squeeze is N ms or less, and a path that needs no timing passes it. `squeeze>N` keeps rows whose hardest squeeze is over N ms, and a path that needs no timing never matches it. A row with no current result matches no number filter (`facts_of`), and the popup says so in one footer line.

The library draws the line under the box as "Type any words, or narrow it down." followed by a "Search tips" link, then the query's error lines exactly as today. The link opens an ImGui popup that lists `search_tips()` in two columns: the key in the Notation role, the meaning in the Label role, each through `text_role.h` (D258, the one owner of font picks). Escape or a click outside closes the popup and leaves the search text alone. The placeholder (`library_table.cpp` line 106) becomes "Search songs". The link reuses the existing link or small-button style in `src/ui/widgets.h`; the executor picks the one that's already used for in-line links and names it in its report.

**Docs.** `docs/UserGuide.md`: the columns sentence (line 156) names `Album`; the panel sentence (line 158) says `Album`, `Charter` and `Folder` hide; the words row (line 188) says "title, artist, album, charter or folder"; a new `album:` row with an example from the test library (the executor picks a chart whose song.ini has an `album =` line, such as `Sufferer - Chair` under Tier 4, and checks the count the search gives); the second-line paragraph (line 201) gains the album sentence; the footer sentence (line 203) stays true as written. `docs/differences-from-public-hydra.md` line 45 adds `album:` to its filter list. `CONTEXT.md`'s Scanned chart and Library search entries name album. D263 in `docs/audit/2026-10-03-fix-decisions.md` records the nine decisions and the three answers from the questions below.

## Tasks

Every task is sized for one Opus executor inside 100 tool calls and 20 minutes, in its own worktree, forked from the tip its "after" line names. Each merge gets one fresh Sonnet reviewer who fixes its own findings and signs off (D107), with the derive-once review the hook asks for; ALB-4's reviewer also does the doc review. Executors run only the tests their task lists. No executor runs anything over the user's library.

### Wave 1, two tasks together

**ALB-1, the scanned fact and its store.** Owns `src/parse/song.h`, `src/parse/song.cpp` (`display_album` only), `src/app/analysis.cpp` (`ChartMeta`, the three readers, the trim loop), `src/store/record_store.h`, `src/store/record_store.cpp` (the `charts` table, the column list, the two slot walkers, `add_album_column`, `set_up_schema`), `src/store/library_file.h`, `src/store/library_file.cpp`, `src/store/stored_versions.h`, `tests/test_analysis.cpp`, `tests/test_store_library.cpp`, `tests/test_store_open.cpp`, and the test file that pins `display_charter` (grep `tests/` for `display_charter`; `tests/test_library_model.cpp` line 236 is one such case, and the new `display_album` case sits beside whichever one is the owner's).

It commits the store half first (schema, upgrade, stamp, store tests), then the scan half, so a budget stop leaves a building tree.

Tests it writes or changes:

- `test_store_open.cpp`: "classify_library_file names each generation" (line 176) gains the LacksAlbumColumn case; a new case shaped like line 239 ("a file whose charts table has no album column opens and gains it in place"): seed a file, `ALTER TABLE charts DROP COLUMN album`, check the kind, open, check the column is there, the rows read back, a rebuild writes an album, and a second open reads Current. A second new case: a 2.8.0-shaped file (failure columns and album both missing) opens and gains all three. A third: an open that cannot add the column (the table already has it while the kind says it lacks it is not makeable, so instead make `ALTER TABLE` fail by locking the file from another connection, as line 771 does) throws `DatabaseUpgrade` and the kind is unchanged.
- `test_store_library.cpp`: `library_row` sets an album; "a result's names follow the latest scan" (line 87) checks the album follows too; "the scan cache is dropped when its reader stamp is not current" (line 151) stays as the proof that a stale stamp drops the cache, plus one new check that a file stamped 5 by hand (`UPDATE meta SET value='5'`) hands back no cache.
- `test_analysis.cpp`: "save_scan_as_library stores every ScanItem field" (line 359) sets and reads `album`; a new case beside line 1001 reads an album from a temp song.ini, from a `.sng` made with `testsng::make_sng({{"album", " X "}})` (trimmed to "X"), and from a `.srb` (the corpus has one if `testdata/input` holds a `.srb`; otherwise the builder in `tests/test_srb.cpp` line 257 moves to a shared header the task owns), and a blank album stays blank with no placeholder. "discover_charts output matches the checked-in scan snapshot" (line 89) is untouched: the snapshot holds no album key and the test checks only the keys it holds.
- The `display_album` case: tags stripped, ends trimmed, blank stays blank, `<unknown artist>` is not special.

Tests it runs: `hydra_tests.exe "-sf=*test_store_open*"`, `"-sf=*test_store_library*"`, `"-sf=*test_analysis*"`, and `-sf=` the display test's file.

**ALB-2, the search owner.** Owns `src/app/library_query.h`, `src/app/library_query.cpp`, `tests/test_library_query.cpp`, and the Library search entry in `CONTEXT.md` (lines 246 to 254). No other file.

Tests it writes or changes: every `make_searchable` call in `test_library_query.cpp` passes an album (the three fixture rows at lines 36 to 45 get real ones from the corpus's song.ini files, the made-up ones get made-up albums); "artist: charter: folder: and title: limit a term to one field" (line 206) gains `album:` and checks `album:` does not match the same word in a title; "words match in any order and across fields" (line 174) gains a plain word that matches only in the album; "a term applies to its own column" (line 229) gains Album; "highlight spans cover the displayed bytes" (line 320) gains an Album span; a new case: a row with an empty album matches no `album:` term and no plain word through its album, and `match_spans(q, Album, "")` is empty; `search_field` (the report pages) is unchanged for the same inputs, pinned by one literal. It also adds `search_tips()` (decision 10) with a new case: every entry's example parses through `parse_library_query` into the term or filter the entry names, with no parse errors, and the entries come back in the order the plan lists.

Tests it runs: `hydra_tests.exe "-sf=*test_library_query*"` and `"-sf=*test_report_rows*"` (it calls `search_field`).

### Wave 2, three tasks together, forked from the tip holding ALB-1 and ALB-2

**ALB-3, the table.** Owns `src/ui/library_model.h`, `src/ui/library_model.cpp`, `src/ui/library_table.cpp`, the `columns_for_panel` comment in `src/ui/app_state.h` (line 124), `tests/test_library_model.cpp`, `tests/ui/uitest_library.cpp`, the column row in `tests/single_owner/rules/ui.json` (line 1447: its note and `must_not_match` examples), and the column line in `docs/agents/ui-testing.md` (line 113).

Tests it writes or changes:

- `test_library_model.cpp`: "library_cell_text is what each column shows" (line 501) gains `kColumnAlbum`; a new case pins `sort_of` and `column_of` for all six columns as literals, round trip, and that `kColumnAlbum == 2` sits between Artist and Charter while `LibrarySort::Album == 5`; "an artist made only of tags reads (unknown), and search keeps the stored text" (line 224) gains the album twin: a tagged album draws without tags and an empty one draws empty, with `searchable.album` folded; "the search narrows the rows" (line 380) gains an `album:` search with a pinned count over `sample()`.
- `uitest_library.cpp`: `library-search` (line 451) gains, after the Tier 4 block, an `album:` search whose count is pinned from the corpus (the executor runs it once and pins the number); then with a song open (Album, Charter and Folder hidden) it checks the album text and "Matched on album." appear; then clears the panel, hides only Album from the header menu (`ctx->TableSetColumnEnabled(table->ID, "Album", false)`), searches the album word again, and checks the row height is two lines, the album text is drawn under Artist and the folder is not drawn under Title; then with no search, the row height is one line. `library-column-widths` (line 769) keeps comparing with the production rule and its comments say six; the `TableSetColumnEnabled` block re-enables Album too. `library-columns-at-start` (line 1063) gains the five-ID ini check described above.
- `ui.json` line 1447's row: the examples name six columns and the new functions.

Tests it runs: `hydra_tests.exe "-sf=*test_library_model*"`, `"-sf=*single_owner*"`, and `hydra_uitest.exe --test library-search --test library-column-widths --test library-columns-at-start --test library-column-order --test library-layout`.

ALB-3 doesn't touch the search box, its placeholder or the lines under it (`library_table.cpp`, the box function around lines 100 to 135). ALB-5 owns that region. Both tasks edit `library_table.cpp`, in hunks that don't overlap, so the merge of the second one is a plain text merge. Whichever merges second runs both tasks' GUI tests after the merge.

**ALB-5, the Search tips line and popup.** Owns the search box function in `src/ui/library_table.cpp` (the placeholder, the line under the box, the popup) and a new GUI test in `tests/ui/uitest_library.cpp`, added as its own function at the end of the file so it doesn't collide with ALB-3's edits there. It reads `search_tips()` from ALB-2 and never writes a key or a meaning itself. The test `library-search-tips` checks that the old "Narrow with" line is gone, the line under the box reads "Type any words, or narrow it down.", clicking "Search tips" opens the popup, and every `search_tips()` entry's key and meaning shows in it. Then it checks that Escape closes the popup and leaves the typed search as it was, and that a bad filter's error line still shows under the box. It also updates the existing `library-search` test where it looks for the old hint text.

Tests it runs: `hydra_uitest.exe --test library-search-tips --test library-search`.

**ALB-4, the public pages and the record.** Owns `docs/UserGuide.md`, `docs/differences-from-public-hydra.md`, the Scanned chart entry in `CONTEXT.md` (lines 39 to 46), and the new D263 entry at the end of `docs/audit/2026-10-03-fix-decisions.md`. It forks from the same tip as ALB-3, writes the text against the key ALB-2 adds and the Search tips line and popup this plan fixes, and merges after ALB-3 and ALB-5 so no page describes a column or a popup that isn't drawn yet. The User Guide's search section drops any quote of the old hint line, says the keys are listed under "Search tips", and says the placeholder is "Search songs". Its example counts come from running the built `hydra_uitest` or `hydra_tests` on the corpus, never guessed; each number in the guide names its source as the page already does ("The examples below come from Hydra Deluxe's test library").

Tests it runs: `hydra_tests.exe "-tc=docs match code*"`. The main session declares the sources (`doc_review_sources.ps1`) before dispatching the reviewer, per the user's memory rule.

### The real-app gate (before ALB-3 merges)

Tests against the rule's own output proved nothing last time (the user's memory, column widths 2026-10-10), so the main session builds ALB-3's branch and runs the real `Hydra.exe` from its `build-cpp` on a snapshot copy of the user's installed `hydra.db` (with `-wal` and `-shm`) and the user's `hydra_ui.ini`. First it asks the user, in chat, for the rescan over the whole library that the stamp bump forces (rule 8). Then:

1. Before the rescan: the saved hidden columns and sort from the user's ini landed on the same columns as before, and Album is visible (question 1's proof on real data).
2. During the rescan: the "Reading charts..." bar moves and the window stays responsive (the memory rule on long work).
3. After it: the table with the panel closed, Album between Artist and Charter at 14 and Folder at 25, and a screenshot for the user. If the user wants a different split, the two shares change in `library_column_specs` on ALB-3's branch and the shot is retaken, up to three rounds; after three the main session stops and asks the user to name the numbers.
4. A search for an album the user knows, with the panel closed and then open: the gold match in the column, then on the second line under Artist, with "Matched on album." in the footer.
5. A blank-album chart shows an empty cell.
6. The Search tips link and popup at the narrow width (song open) and the full width, with a screenshot for the user. ALB-5 merges into the gate's build first, so this step sees both changes together.

Then it runs ALB-3's tests plus the three repo-wide tests (rule 1) and merges.

## Questions for the user

**Answered in chat, 2026-10-24: "approve with recommended."** Question 1 keeps the IDs (Album is ID 5 at position 2). Question 2 hides Album with the panel. Question 3 lets Album sort, so the header has no `NoSort` flag and ALB-3 pins a sort case for Album in place of the flag check. The `stars:N` meaning was checked in the code (`query_matches`: `*facts.stars != *q.stars`, so it keeps an exact match). The user's own search in the app found about 12.5k charts at `stars:7` and 7k at `stars:6`, which they judged right, since charts with no Star Power can't reach 7 stars.

**1. Saved column state (the ini).** The code's rule: ImGui matches a saved column line to a column by the column's ID (`imgui_tables.cpp` line 4032, `column_settings[n].ID != table->Columns[n].ID`), and Hydra's IDs are the `LibrarySort` values 0 to 4. The plan keeps those IDs and gives Album ID 5 at position 2, with `sort_of` and `column_of` as the one mapping, so your hidden columns and sort survive the update. The cheaper alternative is to rename the table's ID string (`"##librarytable"`), which makes ImGui forget the library's saved state once: every column visible and the sort back to Title on the first start after the update, and remembered again from then on. Recommended: keep the IDs. Say if you would rather take the one-time reset and the smaller change.

**2. Album with the song panel open.** The code's rule today: `TableSetColumnEnabled(kColumnCharter, !panel_open)` and the same for Folder (`library_table.cpp` lines 317 and 318), so the narrow table is Title, Artist, Best path. The plan hides Album with them, so the narrow view keeps three columns and the album shows on the second line as decision 3 describes. The alternative keeps Album in the narrow view (Title 17, Artist 10, Album 14, Best path 25 split the room) and the second line carries album only when you hide it by hand. Recommended: hide it with the panel.

**3. Sorting by Album.** Decision 8 puts sorting by album out of scope, so the plan gives the Album header `ImGuiTableColumnFlags_NoSort`. Every other name column sorts on the text it shows (`sort_keys`, D114 item 3), and Album would get that for free by leaving the flag off, since `set_charts` already folds every name column into `sort_keys`. Recommended: leave the flag off and let the header sort, because a column that alone refuses to sort looks broken. Say which.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| Album column sits between Artist and Charter | Decision 1 | ALB-3 | `test_library_model.cpp` new case: `kColumnAlbum == 2`, `column_of(LibrarySort::Album) == 2`; `library-column-widths` reads the six headers in order |
| Shares are Title 17, Artist 10, Album 14, Charter 9, Folder 25, Best path 25 and sum to 100 | Decision 2 | ALB-3 | `library-column-widths`: `text_shares_sum_to_100(cache.specs)` and every `WidthGiven` equals `place_columns` |
| Final split settled on the user's library before merge | Decision 2 | Gate | Gate step 3, screenshot shown, capped at three rounds |
| Album on the second line under Artist, highlighted, when the line is drawn and Album hidden | Decision 3 | ALB-3 | `library-search`: panel open, album text and gold span under Artist |
| Footer counts album | Decision 3 | ALB-3 | `library-search`: "Matched on album." |
| Rows one line tall without a search | Decision 3 | ALB-3 | `library-search`: row height equals one line with the box empty |
| Two lines when Album is the only hidden column; folder not repeated under Title then | Found: second line | ALB-3 | `library-search`: Album hidden by menu, height two lines, album under Artist, no folder under Title |
| Existing footer sentences unchanged | Found: second line | ALB-3 | `library-search` still finds "Matched on folder." |
| `album:` key; plain words match album | Decision 4 | ALB-2 | `test_library_query.cpp` lines 174 and 206 extended |
| `album:` does not match the word in a title | Decision 4 | ALB-2 | `test_library_query.cpp` line 206 extension |
| The long hint line is gone; one short line and a Search tips link replace it | Decision 10 | ALB-5 | `library-search-tips`: no "Narrow with" text, the short line and the link are shown |
| The popup lists every key, `album:` included, with its meaning | Decision 10 | ALB-2, ALB-5 | `search_tips()` examples parse in `test_library_query.cpp`; `library-search-tips` finds each entry in the popup |
| Escape closes the popup and keeps the search | Decision 10 | ALB-5 | `library-search-tips` |
| Placeholder reads "Search songs" | Decision 10 | ALB-5 | `library-search-tips` |
| Blank album shows empty, never matches, no placeholder | Decision 5 | ALB-1, ALB-2, ALB-3 | `display_album("")` is ""; ALB-2 empty-album case; `test_analysis.cpp` blank case; gate step 5 |
| Album cleaned like artist by one owner | Decision 5 | ALB-1 | `display_album` case: tags stripped, trimmed |
| Scan reads album from song.ini, .sng and .srb | Decision 6 | ALB-1 | `test_analysis.cpp` new case, three sources |
| .chart `[Song] Album` out of scope; scan reads no .chart metadata | Decision 6 | none (confirmed) | `analysis.cpp` line 663 reads song.ini only; listed under Out of scope |
| `album` column stored; every field round-trips | Decision 7 | ALB-1 | "save_scan_as_library stores every ScanItem field" with album |
| Stamp 5 to 6 drops the cache, no re-analysis | Decision 7 | ALB-1 | `test_store_library.cpp` line 151 plus the stamped-5 check; `kResultsStamp` untouched (the diff) |
| Existing db gains the column in place, including 2.8.0 and 2.2.0-to-2.7.0 files | Decision 7 | ALB-1 | `test_store_open.cpp` new cases: LacksAlbumColumn, 2.8.0-shaped, Convertible path through `convert_results` then the add |
| A failed add throws DatabaseUpgrade and changes nothing | Decision 7 | ALB-1 | `test_store_open.cpp` locked-file case |
| Long rescan shows progress, never freezes | Decision 7 | Gate | Gate step 2 on the user's library, after the rule 8 yes |
| Saved hidden columns and sort survive the insert | Found: ini | ALB-3 | `library-columns-at-start` five-ID ini check; gate step 1 |
| Album hides and shows with the panel | Found: panel | ALB-3 | `library-search`: with a song open, `**/Album` absent; after close, present |
| Album header does not sort (unless question 3 says otherwise) | Decision 8 | ALB-3 | `test_library_model.cpp`: the flag pinned through `library_column_specs` or the setup call's literal; swapped for a sort case if the user says yes |
| Report pages' search text unchanged | Decision 8 | ALB-2 | `search_field` literal in `test_library_query.cpp`; `test_report_rows` runs |
| User Guide names `album:` and `Album`; docs-vs-code passes | Decision 9 | ALB-4 | `hydra_tests.exe "-tc=docs match code*"`; the doc reviewer |
| No other copy of the column or field list left behind | Brief | ALB-2, ALB-3 | `-sf=*single_owner*`; the grep inventory above is the reviewer's checklist |
| Precheck F lines are the one shape, not a second home | Found: precheck | each executor's report | The reviewer's "Precheck lines dropped" names them |

## Out of scope (for the user to approve)

The report windows, `hydra_batch`'s output and the song panel show no album. A `.chart` file's own `[Song] Album` is not read. `README.md`'s "such as" list of filters is left as it is. Grouping by album is not built; sorting by album is question 3.
