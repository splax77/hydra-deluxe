# Hydra

Score optimizer and path viewer for Clone Hero drums: it scans a song library,
analyzes each chart, and shows the Star Power paths that give the best score.

## Language

### Library

**Chart**:
One playable song file (`.mid`, `.chart`, `.sng`, or `.srb`) plus its metadata.
A chart is known by its song id, the lowercase hex hash `song_id_read` gives it
(docs/adr/0028). Two files with the same song id are one chart, and every
comparison of song ids inside Hydra is exact; `normalize_chart_hash` is kept
for a hash that comes in from outside, the leaderboard reply's (D115).
A file's format is `chart_format_of`'s answer. Whether it is a container (a
`.sng` or `.srb` that packs the chart, its metadata and its audio into one
file) is `is_container`'s. Both live in src/parse/chart_files.h.
_Avoid_: song file, track

**Drum track**:
The MIDI track the song parser reads a `.mid` chart's drums from. Its owner is
`MidiFile::drums_track` (src/parse/midi.h), which never takes the first track,
as Clone Hero 1.1 never does (D253). The name test is `is_drums_track_name`,
which the reader's track-name pick asks too. Its names are "part drums" and
"part drum" in any ASCII case, as the game spells them (docs/adr/0023, D252).
The events track is the other track the parser looks for by name. Its owner is
`MidiFile::events_track`, and its name test is `is_events_track_name` (D253).
_Avoid_: PART DRUMS track

**Library**:
The set of charts found by the latest scan of the user's folders.

**Scan**:
The pass that discovers charts in the configured folders and registers them in
the library. Distinct from analysis.

**Scanned chart**:
One chart file as the last scan found it: its hash, names (title, artist,
charter and album), file, folder, fingerprint (`sig`) and stated timing.
`store::ChartLibraryEntry` in src/store/record_store.h is the shape, and
`app::ScanItem` in src/app/analysis.h is another name for it, so the scan
saves its rows as they are. The names come from the chart's song.ini, a
`.sng`'s metadata or a `.srb`'s header; a `.chart` file's own `[Song]` names
are not read (D263). The scan trims each name once in `discover_charts`,
before `title_or_unknown`, `artist_or_unknown` and `charter_or_unknown` fill in
a blank one (D114 item 15). The album has no such fallback: a blank album stays
blank, and `display_album` in src/parse/song.h cleans it for the screen.
`kChartMetaStamp` in src/store/stored_versions.h marks which reading of the
names a stored row has.

**Library row**:
One copy of a chart in one folder: a row of the `charts` table, and one line
of the library table. Rows are
told apart by `row_key` in src/ui/library_model.h, the chart's notes path,
never its hash, because one chart in two folders is two rows. The selection,
a click and a batch all compare it, and the open song panel re-reads its
chart by it after a rescan, so the panel follows its row (D114 item 7).
Every count of charts a player sees counts library rows, so a chart with two
copies counts twice (D76, D77, D79). `ChartIndex` in src/store/chart_index.h,
built by `RecordStore::chart_index` from the rows in scan order, is the one
answer to which charts the library lists (`ChartIndex::lists`), how many
copies each has (`ChartIndex::copies`; `ChartIndex::count` adds them up for a
set of charts) and which copy names it (`ChartIndex::naming`, the first row)
(D115). The adding-up rule itself is `store::count_charts`, which
`ChartIndex::count` calls; the path report keeps no index once it is built,
so its Charts shown tile asks that rule over the copies each row kept (D282).

**Selection scroll**:
Whether a list moves when its selected row changes, and how far. The owner is
`selection_scroll` in src/ui/list_scroll.h, which reads why the selection
changed (`SelectionSource`, recorded by `AppState::select`); `scroll_last_row`
is the one place that asks ImGui to scroll. A click on a row never scrolls;
a step or a pick from another window brings the row in at the nearest edge,
and moves nothing if it is already on screen. The same page owns the other
case that moves a list: when every row changes height under a selected row
(the Library's song panel hides Album, which makes rows two lines tall),
`row_keeps_place` says the selected row goes back to the same height on
screen, and `keep_last_row_in_place` is the one place that asks ImGui to do
it. A row that was off screen stays off screen. It leaves out where a column
sits and how a table is sized.

**Rescan cache**:
The last saved scan, handed to the next one so a chart whose fingerprint has
not changed is reused without reading its files. `load_scan_cache` in
src/app/analysis.h is the one read of it; a store that cannot be read gives an
empty cache, which only makes the scan slower. Whether one chart changed since
its row was made is `chart_changed_since`'s answer, and `ChartNow::differs_from`
compares the new hash with the stored one.

**Analyze**:
The search that computes a chart's analysis and saves it as a record. A click
on a song analyzes it, and so does a batch, and both save through
`prepare_record` and `RecordStore::save`. A click saves only when the stored
record is missing, Stale or different from the fresh one
(`AppState::save_view_summary`), and a click that saves marks both open
reports out of date, as a finished batch does (D115). Its progress bar moves in steps of at least half a percent of the chart
(src/search/engine.cpp), and the main search owns the first 90% of the bar,
the all-0 pass the rest (src/search/pather.cpp; D273).

**Analysis**:
The engine's full output for one chart under one profile: every kept path
with its activations, squeeze rows and SP-end history, built each time a
chart is analyzed and kept in memory only (docs/adr/0026). The type is
`HydraRecord`; the name is historical, from before records were stored the
way they are now. A record is what the store keeps of an analysis.
_Avoid_: record (for the in-memory output)

**Record**:
One chart's saved result under one profile: its path count, its best path's
star count, its note count and its top paths, best first. A record that kept
no paths has a path count of 0 and no path rows. `RecordRow` and `PathRow` in
src/store/record_store.h are its shape, held in the `records` and
`record_paths` tables (docs/adr/0031). `prepare_record` is the one place an
analysis becomes a record: it ranks `HydraRecord::all_paths` from 1 and keeps
at most `kPathsPerRecord`. Rank 1 is the record's best path, which the library
shows (`RecordRow::best_path_text`); nothing else picks a best path again.
`RecordStore::save` writes a record, and `RecordStore::lookup` and
`RecordStore::lookup_one` read one with its status. A record may instead be
a stored failure: a chart whose own content stops the analysis
(`remembered_failure` in src/core/error_kind.h says which kinds) is saved as
a record with no paths and its `failure_kind` and `failure_text` set, built
by `store::failure_record` and written by the same `RecordStore::save`. Its
song id is its file's hash, so a changed file is a new chart and is tried
again (D249). "Record" is a code and
glossary word only: public text says "chart" (user decision, 2026-10-11).
_Avoid_: summary row, stored record, result row

**Profile**:
One set of settings a chart can be analyzed under: the chart mode, the SP cap,
the lens, the results stamp and the rules fingerprint. A record is filed under
its chart's song id and its profile, so each profile keeps its own records and
switching back to settings analyzed before shows them Ready at once.
`store::Profile` in src/store/record_store.h holds the first three; the store
adds the stamp and the fingerprint itself when it files or looks up a record,
in the `profiles` table. `Settings::profile` is the one builder of the current
profile. A page that needs another asks for it by name:
`Settings::profile_at_clone_hero_cap` for the leaderboard comparison and
`Settings::profile_with_fills` for the fill comparison. `KeyPart` in
src/app/config.h names the part each analysis setting is filed under. There is
no limit on profiles; a save deletes a profile left with no records (D115).
A player never sees the word: the library and the reports always show the
current profile.
_Avoid_: result identity, record key

**Lens**:
The part of a profile beyond the chart mode and the cap: the path
limit, the score range (number and unit) and the fill rule. `store::Lens` in
src/store/record_store.h keeps it in one canonical form, so equal settings
compare equal: a path limit that is off is stored as 0, and the unit as the
search reads it. `Settings::lens` builds it. The word is the code's only;
nothing a player reads uses it.

**Result status**:
What the store says about a chart's result under the current settings. Ready:
a record under the current profile, with a results stamp this build accepts
and the rules in force. Stale: no Ready record, but one under a sibling
profile, the same chart mode, cap and lens under another results stamp or
other rules (`RecordLookup::stale_build` and `RecordLookup::stale_rules` say
which). Can't analyze: the winning record under the current profile is a
stored failure (see Record), so the chart's own content stops the analysis.
Not analyzed: none of these, so a record under another mode, cap or lens
never makes a chart Stale. `store::RecordStatus` in src/store/record_store.h
(its fourth value is `CantAnalyze`); only the store decides it, in
`RecordStore::lookup` (D115, D249). A stored failure follows the same stamp
and rules checks as any record, so after a results stamp bump it reads Stale
and is tried again. `RecordStore::settled_song_ids` lists the Ready and
Can't analyze charts under a profile from that same lookup; a batch skips
both.
The library writes the four as "Not analyzed", "Stale", "Analyzed" and
"Can't analyze" (`status_label` in src/ui/library_model.h), on its chips and
in the Best path cell. A ready result may have kept no paths.
_Avoid_: not analyzed (for the comparison page's wider state: that page says
"no current result")

**Library file upgrade**:
What the first open after an update does with an older hydra.db.
`classify_library_file` in src/store/library_file.h is the one answer to which
generation a file is. A file from 2.2.0 to 2.7.0 is converted in place, in one
transaction (`RecordStore::convert_results`): each old summary row becomes a
record under its profile, keeping its stamp, so it reads Stale until the chart
is analyzed again. A row with a score and no stars is left behind, and song
ids are lowercased. A file from before 2.2.0 (`kOldestConvertibleVersion`) is
not converted. The app sets it aside as hydra.db.pre-2.2.0, replacing an
earlier one, starts a fresh library, says so once and scans
(`set_aside_old_library` in src/store/upgrade_files.cpp). The command-line
tools refuse it and leave it alone. Each open in shipped code names which it
wants with `OldFilePolicy` (docs/adr/0031). The notice and the failure sentences live in
src/app/user_messages.cpp. The open runs off the window's thread, so the
window never freezes; a slow one shows the startup screen.

**SP cap**:
The Star Power meter ceiling an analysis runs under, in bars. 4 is Clone
Hero's rule and the default. Other values answer what-if questions; their
scores are not achievable in game.
_Avoid_: edition, uncapped, SP meter

**Analysis settings**:
The eight settings that make up a record's profile, shown together in the
analysis settings panel and applied to every song: difficulty, Pro Drums, 2x Bass and
Note Shuffle (together the chart mode, read off the settings as a
`ChartReading` by `Settings::reading` and spelled by `chartmode_key`),
the SP cap, 1.0 fills (the fill spawn deadline),
the score range, and the path limit.
Changing one shows the library's records under the new combination, and
analyzes the open song again under it (D90 item 1). Changing it back brings
the library's old rows back with no batch. The backend limit is
not one of them: it only hides backend rows on screen and never re-analyzes.
The path limit starts on<!-- default: Settings::mslimit_enabled -->
at 10 ms<!-- default: Settings::mslimit_value -->. The backend limit ("Hide
backend rows beyond") starts off<!-- default: Settings::backendlimit_enabled -->
with 50 ms<!-- default: Settings::backendlimit_value --> in the box. The user
confirmed both defaults (D48, Q33); `Settings` in src/app/config.h owns them.
The score range starts at 4<!-- default: kDefaultDepthValue --> scores below
the best. `kDefaultDepthValue` in src/search/pather.h owns that number, and
the search and `Settings` both start from it (D54).
`kAnalysisFields` in src/app/config.h lists the ten `Settings` fields behind
the eight settings, each with the part of the profile it is filed
under. The panel's Reset (`Settings::with_analysis_defaults`) and the settings
button's summary (`Settings::analysis_changes`) read that list, so a new
analysis setting is one new row there (concept audit M13).
_Avoid_: view options

**Chart reading**:
The four choices that pick which notes a chart is read as: the difficulty, Pro
Drums, 2x Bass and Note Shuffle. `ChartReading` in src/parse/chart_reading.h
holds them, and `Settings::reading` reads them off the settings, 2x Bass through
`effective_bass2x`; `Settings::with_reading` writes one back. Its stored
spelling is the chart mode, such as "Expert Pro Drums, 2x Bass", written by
`chartmode_key` and read back by `Settings::with_chartmode`.
`AnalysisSettings` holds a `ChartReading` as a base. The `load_songpath` and
`load_songbytes_*` loaders take one, and so do the Preview's song key and load
jobs, `resolve_preview_source`, `resolve_preview_song`,
`load_dynamics_song` and `dynamics_for_settings`. So a fifth choice is added
to `ChartReading`, `reading` and `with_reading`, and the code that only passes
a reading along needs no change. Only `load_songbytes_mid` and
`load_songbytes_chart` in src/parse/song.cpp take it apart, where the parsers
need the pieces.

**Score range**:
How far below the best score a kept path may fall: a number and a unit, scores
or points. The unit is `DepthMode` in src/search/engine.h. The settings keep it
as the ini's int, and `Settings::search_depth_mode` is the one reading of that
int (1 is points, anything else scores). `score_range_text` in
src/app/config.h writes it in words: "4 scores", "1 point".
_Avoid_: depth (the code's word; nothing a player reads says it)

**Settings in words**:
How the search settings are written for the user. `describe_settings` in
src/app/config.h gives the score range (`score_range_text`), the SP cap
(`bars_text`: "4 bars", "1 bar") and the path limit ("10 ms" or "off") as
`SettingsText`. The batch confirm, the settings button and `hydra_batch`'s
header all read it, each with its own labels (D114 item 9). A fill rule's
name is `fill_rule_name`'s, in src/search/graph.h.
_Avoid_: Depth, Timing cap, none (for a path limit that is off)

**Library search**:
What the user types to narrow the library. Every word must match the title,
artist, album, charter or folder, in any order; `"quotes"` match a phrase, and
`title:`, `artist:`, `album:`, `charter:` and `folder:` limit a word or phrase
to one field. A chart with no album matches nothing through it. `stars:N`
(exactly N stars) and `squeeze<=N` (hardest squeeze or required early fill at
most N ms; `squeeze<N` is strict, under N ms, D266) test the
record's best path and only ever match ready rows. `squeeze>N` and
`squeeze>=N` are the floor side, above N or at least N; a floor and a limit
together give a range, and a path that needs no timing reaches no floor. Any
other operator after `squeeze` is an error, not a word. Matching
folds case and accents away, so `beyonce` finds "Beyoncé", and ignores Clone
Hero's rich-text tags. The keys, an example of each and what each does are
listed once, in `search_tips()` in src/app/library_query.h; the library's
Search tips popup draws that list.

**Batch time left**:
The batch strip's estimate of how long the rest of a batch will take: the
average time per finished chart so far, times the charts still to go. It
shows only once three charts have finished (`kEtaMinFinished` in
src/ui/library_jobs.h). Decided in the approved
docs/superpowers/specs/2026-09-27-ui-redesign-design.md ("The estimate appears
once three charts have finished").

**Batch counts**:
A batch's three counts: analyzed, failed, and skipped (rows that already had a
result). `run_batch` is the only writer of them (`BatchProgress` in
src/app/analysis.h). `batch_counts_text` words them for both the app's
finished strip and `hydra_batch`'s closing line, "21 analyzed · 0 failed · 1
skipped (already had a result)" (D114 item 10). A Can't analyze chart is a
skipped row too, because its stored failure is its result (D249); the batch
confirm counts those rows on their own dim line.
_Avoid_: already stored

**Batch running**:
While a batch runs, a scan or a second batch is turned away with one
sentence, `AppState::kBatchRunningStatus` ("A batch is running.") in
src/ui/app_state.h, on the status line and as the greyed buttons' tooltip
(D114 item 12). `AppState::can_analyze` (the rules file is good and no batch
runs) and `AppState::can_scan` are the gates the buttons and the starts read.
_Avoid_: busy

**UI timings**:
How long the app's short-lived messages and refreshes wait. A click's
"Analyzing chart..." box shows only once its analysis has run 0.15 s, so a
fast chart shows its paths with no box and no "Done!" (D87 items 6 and 10).
Opening the library file at startup waits the same 0.15 s, through the same
constant (`kViewProgressDelaySeconds` in src/ui/app_state.h): a fast open
draws an empty window, and the startup screen appears only once the open has
run that long (D100 item 2). The startup screen's time-left line follows the
Song Preview's rule (`gated_time_left_text` in src/ui/preview_load_job.h, D100 item 4).
The "Copied!" flash after Copy path
lasts 2 seconds, the library search refilters at most every 0.15 s while
typing, and a running batch refreshes the library at most once a second
(src/ui/app_state.cpp, src/ui/paths_tab.cpp, src/ui/library_table.cpp;
confirmed as they are, D48, Q33).

**Leaderboard fetch**:
The download of real scores for the leaderboard report. It comes from the
DMBot API, the backend behind dmleaderboards.com, not from the site itself. It waits at most 15 s to resolve the host, 20 s to connect, 30 s to
send and 120 s to receive, because the backend cold-starts after idle; it
checks Cancel every 50 ms; and it accepts only HTTP status 200
(src/net/http.cpp; D273).

### Reports and messages

**In the library**:
Whether the last scan's library lists a chart. `ChartIndex::lists` in
src/store/chart_index.h is the one answer for every page (D87 item 4, D92).
`report::library_lookup` reads only the charts it lists.

**Report coverage**:
Which library charts a report page covers and how it accounts for the rest.
`report::library_lookup` runs one `RecordStore::lookup` over the library's
song ids and splits them three ways: Ready (table rows), Can't analyze (the
line above the path report's table, with its Show list), and no result (a
failure that is not remembered, or a stopped run). The path report, the
leaderboard comparison and the fill comparison all read it, so they agree on
which charts are covered. Every count of the three groups goes through
`ChartIndex::count`, so with no filters shown, can't analyze and no result
add up to the library's count. Opening the path report first runs a batch
over every chart with no current result, so the report covers the whole
library (D249).

**Comparable chart mode**:
Whether the analysis settings can be held against dmleaderboards scores, which
are Clone Hero scores. `dm_report::why_not_comparable` in src/app/dm_report.h
answers it: empty when they can, otherwise the sentence naming the first rule
they break. The toolbar's button and `collect_dm_rows` (Refresh, Try again,
"Compare another player...") both ask it about the profile the page lists, at
Clone Hero's cap whatever the cap setting, so only the difficulty, fill rule
and Note Shuffle can refuse a click (D114 item 6, D271 item 8). Whether a cap
is Clone Hero's is `is_clone_hero_cap` in src/core/model.h. The words that name
that cap on the page, the tooltip and the gate's sentence come from
`clone_hero_cap_phrase` in the same file.

**Comparison status**:
What the comparison page says about one leaderboard score: under, at or above
optimal, no paths, no current result, not in library, or other speed. The
`kStatus*` words in src/app/dm_report.h are the Status cell's text, and
`kStatusWords` beside them holds every other spelling: the tile, the dropdown
label, the counts phrase and the hover. "No current result" means the library
lists the chart but has no ready result for this mode at SP cap 4: never
analyzed there, or stale (D114 item 1). Written as a sentence, a chart the
scan never found is "not in your library" (D114 item 2).
_Avoid_: not analyzed (the library's word for a narrower state)

**Fill comparison status**:
What `hydra_fillcompare`'s page says about one chart's records under the two
fill rules, read from one database: same, 1.0 higher, 1.1 higher, only 1.0,
only 1.1, or in both. A chart with a record under one rule only still gets a
row, labelled by that rule; the page's filter and tile call those "Only under
1.0", "Only under 1.1" and "Under one rule only" (user's choice, 2026-10-15). The six
are the `kStatus*` words in src/app/fill_report.h. `collect_fill_rows` writes
them, `tally_fill_rows` counts them, and `build_fill_html` writes them into
the page's script. "In both" means a result on each side but not a score on
each; the subtitle and tile say `kInBothPhrase`, "in both, not both scored"
(D114 item 2).

**Report table**:
A report window's sortable, filterable table. `report_view::TableView` and
`report_view::Column` in src/app/report_view.h hold its rules: `text_column`
builds a text column, `optional_key` sorts a value a row may lack, and empty
values sink to the bottom whichever way a column sorts. `count_line` writes
the "X of Y" line. Each filter dropdown keeps every row at `kEveryRowChoice`,
which Clear filters goes back to. The empty table's sentence
(`kNothingMatches`) and `search_hint`, which writes every chart search box's
hint from the fields that box searches, live in src/app/html_page.h, because
the fill page, built without the GUI, shows them too. The Library reads the
same sentence and builds its hint with the same function (D276 item 3).

**Text sort order**:
How a text column sorts, in the library and in every report: on the text the
cell shows, folded by `fold_for_search` (src/app/library_query.h), so case
and accents don't split names apart. The library keeps one folded key per
column (`LibraryRow::sort_keys`, from `library_cell_text`; D114 item 3). The
report windows fold through `report_view::folded_key` and order with
`compare_keys`. The fill page carries a sort field per text column made by the
same `folded_key`, and its script only compares them.

**Fact separator**:
The " · " between facts on one line: a byline, a star count, a path's acts and
score. `kFactSeparator` in src/app/display_format.h is the one spelling. Two
dots spaced differently on purpose, in src/ui/library_dialogs.cpp and
src/ui/paths_tab.cpp, say why beside them.

**Plain error**:
What a failure reads as: one short sentence picked by the error's kind
(`ErrorKind` in src/core/error_kind.h), never by its words, with the raw text
kept for a small details line. `plain_error` in src/app/user_messages.h gives
the sentence through the one switch, `kind_sentence`, and `plain_error_detail`
the raw text, which is empty when the sentence already quotes it, so no screen
shows one failure twice (the user's rule of 2026-10-15). `plain_error_line` is
the one line for a place with room for a single line per failure,
`plain_error_block` the sentence and raw text as one block, and `error_detail_line`
(src/ui/widgets.h) draws a details line and nothing when it is empty. No other
code in src shows an exception's `what()`; a scan row (err-once.json) holds
that. A sentence more than one screen shows is a constant in the same
header, such as `kPathReportFailed` and `kComparisonFailed` (D114 item 14).

**Rules file error**:
What a bad hydra_rules.ini does. In the app, analysis stays off, and the
toolbar, the path report window and the song panel all show
`kAnalysisOffSentence` (src/app/user_messages.h; D114 items 11 and 13). A
command-line tool stops instead: `app::load_tool_rules` (src/app/rules_file.h)
loads the rules for every tool and prints `ini_file_tool_text`, the tools'
sentence and then the loader's line. Numbers in the file are read through
`parse_finite_number` (src/core/strutil.h), so "nan", "inf" and hex are
errors (D114 item 8). What the loader refuses, and the wording it shares with
hydra_settings.ini, is under Settings files.

**Settings files**:
How hydra_settings.ini and hydra_rules.ini are read, and what a bad
hydra_settings.ini does (D270). Both files follow one rule: anything the
loader can't read refuses the whole file, with an error that names the file,
the key and the value. `Settings::load_file` (src/app/config.h) refuses what
the rules loader, `load_rules_file` (src/app/rules_file.h), refuses: a line
with no `=`, an unknown key, and a value its key can't take. In the settings
file that is a value that is not a whole number, an on/off that is not `0` or
`1`, or a `view_difficulty` that names no difficulty. Both loaders word those
lines through `ini_not_key_value_text`, `ini_unknown_key_text` and
`ini_bad_value_text`, and read whole numbers through `parse_whole_number`, all
in src/app/ini_errors.h (D270 call 4); a scan row (ini-errors.json) holds
that. A number outside its range loads at the nearest edge in the settings
file, so `sp_cap=0` reads 1, where `hydra_rules.ini` refuses a number below
one of its five floors (D270 calls 1 to 3). A key Hydra used to write is
skipped, not refused: `kRetiredSettingsKeys` (src/app/config.h) and
`kRetiredRulesKeys` (src/app/rules_file.cpp) list them, and
`is_retired_key` asks. A live key of the same name wins. A retired settings
key is left out at the next save (D270 call 7). `hit_window_ms` is one of
them, because the hit window is no longer a setting (Hit window).
A refused hydra_settings.ini throws `SettingsFileError` (kind `SettingsFile`,
whose app sentence is `kSettingsOffSentence`). In the app, Hydra opens on a
default `Settings`, analysis is off as for the rules file, no scan starts and
`save_settings` writes nothing, so the file is never touched
(`AppState::settings_error`, D276 item 1). `AppState::analysis_off_lines` gives
the lines for both files, the settings file's first, and the toolbar, the song
panel and the path report window draw them through `analysis_off_notice`
(src/ui/widgets.h). A command-line tool stops instead:
`app::load_tool_settings` (src/app/config.h) loads the settings for every tool
and prints the error through `ini_file_tool_text`, exit code 2. What a refused
hydra_rules.ini does is its own entry (Rules file error).

**Job outcome**:
How a finished background job ended: done, cancelled or failed
(`JobOutcome`). `ResultJobBase::outcome` in src/app/job_base.h weighs the run's
return and the Cancel flag in one place: a run that returned whole is done
even when a Cancel came after its last step, so a report keeps it. The click's
analysis is the one exception, written beside its reader,
`AppState::update_view_job` (D114 item 21). A step asked to stop throws
`hydra::Cancelled` (src/core/error_kind.h) or a type derived from it, and a
report build stopped by Cancel is marked through `ReportSlot::mark_cancelled`.

**Update check**:
Whether Hydra offers a newer release in the version bar, and whether it may
install one (D259, D260). Three questions, each with one owner. Is this
version newer: `parse_version` in src/core/app_version.h reads both versions,
and only a strictly greater one counts. Is this the installed exe:
`installed_by_setup` in src/app/config.cpp, the only gate on the start-up
check, so a dev, worktree or copied build never checks. Is the download safe
to run: `verify_update` in src/app/update_verify.h checks the digest and the
release signature, and any failure refuses the install with one sentence
through `plain_error`. Hydra starts the installer with the plain open verb,
never as administrator; installer/hydra.iss says why. The text of a version
the user reads ("Hydra Deluxe 2.11.1") comes only from `named_version` in
src/core/app_version.h. What the version bar says is answered once, by
`AppState::version_bar_status`, and drawn in one place, `render_version_bar`
in src/ui/version_bar.cpp.

### Paths

**Path**:
One way to play a chart's Star Power: which activations to take and what each
is worth. Every path that ties the analysis's top score is optimal, not only the
first one listed (D48). A path's average multiplier reads 0.000x when it has no
scoring notes, instead of dividing by zero (`Path::avg_mult`); real charts
never hit this (D48, Q33).

**Optimal path**:
A path whose score ties the analysis's top score, so a tied variant is optimal
too. `HydraRecord::is_optimal` is the one answer, and it reads the top score
from `HydraRecord::optimal_score`, which has no value for an analysis with no
paths. The report's flag (a record's `PathRow::optimal`), the Paths tab's gold
group and the Preview's "(optimal)" label all come from it. "Best path" means
only the single top path, rank 1 of a record. The report's "Optimal paths
only" box keeps every optimal path among the ones the record kept (D115).

**Score categories**:
The six parts a path's score is made of: notes (the notes' own points), combo bonus, Star
Power, solo bonus, accents and ghosts. The Notes line is `Path::notes_points`.
D115 renamed it, so the chart's 1x total, `Path::chart_base_score`, is the one
"base score", and a note's own 1x value is `ChordNote::basescore`, the name
Clone Hero's code uses. `score_total` in src/core/model.h is
the one sum, in the order `Path` stores them. `ReplayScore` in
src/core/replay.h holds the same six (`kReplayScoreFields` names each), and
`score_of` and `assign_score` are the only pairing between it and `Path`.

**Activation**:
One use of banked Star Power, written in path notation with its skip count and
squeeze symbols (e.g. `E2+-`). An activation needs
2<!-- default: kSpActivationBars --> banked bars of Star Power.
`kSpActivationBars` in src/core/timing.h owns that number (D54). The game's
rule is docs/clone-hero/05-star-power.md (SP-16).

**Skip**:
A fill an activation deliberately passes over before activating.

**Authored fill**:
A fill written in the chart. Each one is placed on its own once every chord
is read, on a chord near its end within the landing window
(`fill_land_slop_beats`, plus one tick), by `place_authored_fills`. The
game's rule it copies is docs/clone-hero/04-fills.md (FL-03 to FL-07), and
the one side where Hydra differs is in docs/adr/0023.

**SP phrase**:
A chart section that awards a bar of Star Power when hit fully. Which
chords it covers and which one pays is the game's rule, in
docs/clone-hero/05-star-power.md (SP-01 to SP-04); `close_sp_phrase` is
Hydra's one owner of it.
_Avoid_: star power section

**All-0 path**:
The best path whose activations all fit the all-0 rule (`allzero_activation`
in src/core/model.h: no skips, or an E1, D102) and whose every squeeze and
early fill is 0 ms or easier (`allzero_options` in src/search/pather.cpp;
D85).

### Squeezes

**Squeeze**:
A deliberately early or late hit that moves points across a scoring boundary.

**Frontend squeeze**:
Hitting the activation chord's activation note first, so the chord's other
notes score under Star Power.

**Backend squeeze**:
Hitting a note near the SP end early, so it lands inside Star Power.

**Backend row**:
A chord within the squeeze window of an SP end, as the Backend timings table
lists it: one `BackendSqueeze` in src/core/model.h. The window is
`within_squeeze_window`'s answer. What the row is worth and the backend leeway
come from src/core/backend_value.h (`backend_row_value`). In public text
"backend" names a scoring effect, a table row and a squeeze; the rows are
backend rows (D115).

**SqIn / SqOut**:
Squeezing an SP phrase's note into (+) or out of (-) an active Star Power
window, written as the `+`/`-` symbols in path notation. Only a phrase after
the activation chord can be squeezed: one at or before it was banked before
SP started (core/sqout_chord.h, `activation_can_squeeze`). A phrase is
squeezed in only once (D34). An SP end offers a window at most one phrase
(`offered_phrase`, D36): its newest phrase, when collecting it is what moved
the window's end away from this SP end; or, when the window's end is this SP
end, the first phrase after it that the window has not squeezed in. An older
phrase can't be squeezed out while a newer one stays in: the newer one is hit
later, so it would be hit after Star Power ran out too.

**Multiplier squeeze**:
Ordering the hits of a multi-note chord on a combo-multiplier boundary so the
more valuable notes score on the higher multiplier.

**Star cutoff**:
The score a chart needs for 1 to 7 stars: its base score (every note at 1x,
no Star Power) times 0.1<!-- default: kStarMultipliers[1] -->, 0.5<!-- default: kStarMultipliers[2] -->, 1.0<!-- default: kStarMultipliers[3] -->, 2.0<!-- default: kStarMultipliers[4] -->, 2.8<!-- default: kStarMultipliers[5] -->, 3.6<!-- default: kStarMultipliers[6] --> or 4.4<!-- default: kStarMultipliers[7] -->, rounded up. Clone
Hero compares it against your score without the solo bonus. The product is
kept as a 32-bit float before it is rounded up, as the decompiled game does,
which can move a large cutoff by one point (`star_cutoff` in
src/core/stars.cpp; D48, Q33).

**Early fill (E)**:
An activation timing (the `E` notation) near a fill's spawn deadline. The
window has two sides, each a fixed number and never a setting, because the
search reads them (src/core/model.h).
On the orange side, Star Power comes ready after the deadline, so you must
hit early to summon the fill. It reaches 85 ms<!-- default: kHitWindowMs -->,
the hit window (`kHitWindowMs`, read by `fill_refuses`): later than that the fill
never spawns, and the search drops the activation.
On the white side, Star Power comes ready before the deadline with less than
60 ms<!-- default: kEarlyFillSlackMs --> to spare (`kEarlyFillSlackMs`, read
by `is_e0`). The activation still gets the `E` mark, but has nothing to time.
The white side stays at 60 ms because paths were cluttered with early fills
that never matter (D51 call 7); D250 widened only the orange side.

**Fill spawn deadline (CH 1.1)**:
The latest your SP meter can fill up and still have a fill appear. Clone Hero
1.1 puts it a flat 4 beats before the fill starts. Hydra's default rule.
`kCh11FillLeadBeats` in src/search/graph.h owns the 4, and
`fill_rule_description` writes it into the sentence the "1.0 fills" tooltip
and the fill comparison page show. A
beat here is a quarter note (the chart's resolution) in every meter, 6/8
included. The game's rule is docs/clone-hero/04-fills.md (FL-19 and FL-20);
Hydra adopted it in D115 (docs/adr/0010, note of 2026-10-15).

**Fill spawn deadline (CH 1.0)**:
The older rule: roughly one fill-length of lead time before the fill, clamped
to 250..10000 ms. Short fills got stricter in 1.1 and long fills got looser.
The "1.0 fills" analysis setting, or `hydra_batch --legacy-fills`. Part of a
record's profile, so a chart's 1.0 and 1.1 records sit side by side in one
database; `hydra_fillcompare` diffs the two. See docs/adr/0010. The 1.0
game's own deadline is docs/clone-hero/04-fills.md (FL-41 and FL-42).

**Generated fill**:
A fill Hydra places itself on a chart with no authored fills. The "1.0 fills"
setting picks which game version's generator runs, in one place:
`Song::check_activations`.

Clone Hero 1.1 (the default) follows the game's own generator, whose rules
are docs/clone-hero/04-fills.md (FL-13 to FL-18). hydra_rules.ini can change
its 4-line step and its half-beat distance (`fill_cooldown_measures` and
`fill_max_distance_beats`).

Clone Hero 1.0 mode keeps Hydra's own model of 1.0's fill rule (the user's
call, 2026-10-14; docs/adr/0023), and counts measures on the same grid as 1.1
(`MeasureIndex`; the user's call, 2026-10-24). It puts the fill on the chord
nearest a measure line, within half a beat (no extra tick), half a grid
measure long at the chord (`kCh10FillLengthMeasures`, fixed; the old
`fill_length_measures` key is read and ignored), and at least 4 lines after
the last one. When two chords sit equally near the line, the later one gets
the fill. The distance and the length are cut down to whole ticks.
`Song::place_fills_ch10` holds it, and `Song::place_fills_ch11` the 1.1
generator. How the game's own 1.0 generator differs is
docs/clone-hero/04-fills.md (FL-36).

`Song::generated_fills` records which generator ran (empty for authored
fills): the one answer to "was this fill generated". The Preview reads it to
draw a 1.1 generated fill as the half measure before its chord
(`drawn_fill_start_tick`, the user's call, 2026-10-19), because the fill's
engine length is often 0. That is display only: it changes no length the
deadline math reads, and authored and 1.0 fills are drawn from their own
length.

**Fill rule**:
Which Clone Hero version's fill rules a run follows, 1.1 or 1.0:
`FillDeadlineRule` in src/core/song.h. It picks both the fill spawn deadline
(`activation_fill_deadline_ms`) and the generated-fill generator
(`Song::check_activations`). A flag that says "1.0 fills" (the setting, a
`store::Lens`, or `hydra_batch --legacy-fills`) becomes a rule only through
`fill_rule_for` in src/search/graph.h, and `legacy_fills_for` reads a rule back
into the flag. The two are the only crossings. `fill_rule_name` names a rule
wherever it is shown. A record carries its rule in its profile's lens, so one
database holds both rules' records; the old label on the whole file
(`engine_mode`) is retired (D115, docs/adr/0010).

**Hit window**:
The per-side ms window Clone Hero registers a hit in, Clone Hero's Pro Drums
window: 85 ms<!-- default: kHitWindowMs --> (`kHitWindowMs` in
src/core/model.h). It is one fixed number, not a setting (D270 call 8). The
search's early fill reach (`fill_refuses`), the squeeze budgets
(`squeeze_budget_ms`, `nominal_budget_ms`), the Rating words and the report's
timing tiers all read it. The old `hit_window_ms` key is retired and skipped
(Settings files). The report's timing tiers are Normal up to 2 ms
(`kDifficultMs`), then Hard up to half a hit window, Extreme up to one window,
Insane up to one and a half, Insane+ up to two, and Beyond past two windows;
each edge belongs to the band below it (`timing_tiers` and `tier_for` in
src/core/squeeze_rating.cpp; D48, Q3 and Q33). The report's footer and its
Timing column's tooltip call it "the 85 ms hit window", printed by
`hit_window_text` (src/app/report.h; D278 item 4).

**Transfer scale**:
How frontend timing error carries to the SP end. SP length is measured in
measures, so a hit `d` ms off moves the SP end `r*d` ms; early and late hits
can scale differently on a signature or tempo change. It is measured from the
note whose timing moves the SP end: the activation, or the cap's collecting
note. Each SqIn stores its own scale, at the SP end it was measured from
(docs/adr/0021).

**SP-end history**:
Every place an activation's SP end moved, in order: the activation, each
phrase collected, each cap clamp and each SqIn, with the end in force after
it. The engine stores it once per activation. The deact node, the clamp note
and the collected phrases are read from it (docs/adr/0021).

**Deact node**:
The exact chart position where an activation's Star Power ends; backend
squeeze timings are measured against it. The search stamps it onto the
analysis as it runs, and nothing downstream re-derives it: every reader asks
`Activation::deact_tick` (src/core/model.h), which reads the last step of the
SP-end history.
_Avoid_: SP end tick (when the anchored search position is meant)

**Cap-clamped window**:
Normally an activation's Star Power window ends a fixed distance (measures)
past the activation. But if a phrase collected partway through Star Power
would overfill the meter past the SP cap (the max bars of SP you can hold),
the window's end gets pinned to the cap measured from that phrase's note
instead. A phrase that only fills the meter exactly to the cap is a tie, and a
tie does not clamp. The end is pinned only while the meter is full: as it
drains, a later phrase that fits under the cap extends the end again from
where it was pinned (`ScoreGraph::extend_deacts`, docs/adr/0013).
`clamp_tick()` names the note that pinned it, so later code doesn't have to
guess; a later unclamped extension keeps the earlier note.
The overfill warning in Song Details needs two things: the window is clamped,
and the activation lists a SqIn/SqOut or an uncounted or squeezed-out backend
row (`rate_activation` in squeeze_rating.cpp). A clamp with neither shows no
warning.

**Squeeze rating**:
The displayed difficulty judgement of a squeeze: its rating label, its
effective ms once the transfer scale is applied, and whether the scale is
material enough to warn about. `rate_activation` in
src/core/squeeze_rating.cpp works out the scales and effective ms
(`rate_note` for each row). The Rating cell's word ("Normal", "Hard
(uncounted)" and so on) is `BackendSqueeze::summarystr`'s, in
src/core/model.cpp, one rule for every row (D269). The figure banded is the
row's needed timing, `backend_row_timing` (src/core/model.h): a squeezed-out
row needs its SqOut's `squeeze_difficulty`, a plain row the engine counts
(`counted_without_squeeze`) needs nothing, and an uncounted plain row needs
its offset. A row that needs nothing reads Normal whatever its scale; any
other row with an eff. figure is banded on that figure (D276 item 2). The
bands are the report's timing tiers (`timing_tiers`, through `tier_for`), so
the cell and the report share one ladder. "(uncounted)" is added wherever
`counted_without_squeeze` says the engine pays nothing. The badges and the
report stay on raw ms (D269 call 5), so a row under a scale can read a
different word from its path's report tier, and the report's footer says the
two can differ.

**Needs no timing**:
A squeeze that is already done with no frontend timing: a SqIn whose note the
SP walk already pays, or a SqOut whose note it does not. `SPSqueeze::needs_no_timing`
in src/core/model.h is the one answer, and the rating and the details sentence
both read it.
_Avoid_: free (for this test)

**Backend leeway**:
How long after the SP end a note still scores under Star Power without a
squeeze: less than `backend_leeway_ms`
(3 ms<!-- default: Rules::backend_leeway_ms --> by default). A note exactly
3.0 ms<!-- default: Rules::backend_leeway_ms --> after the SP end does not
score under SP. The leeway is not a Clone Hero rule: no such constant was
found in the Clone Hero engine methods read. The 3 ms<!-- default: Rules::backend_leeway_ms -->
is DragonDelgar's number, from public Hydra (`hydra/hypath.py`, line 678 at
his commit 3b49012c), and Hydra Deluxe keeps it so its scores use the same
method (D269 call 4). It only decides whether a backend counts in the
optimal score or is uncounted. Star Power still ends at one point.

**Difficulty**:
A path's or activation's hardest squeeze or required early fill, in raw gap
ms — never scaled by the transfer scale.

### Chart timing

These words have one meaning each in code and in the guide (D115). Clone Hero
1.1.0.6142's code is the source of each rule (docs/adr/0010, note of
2026-10-15).

**Tick**:
One step of the chart's own resolution, its smallest unit of position. Its
length in time depends on the chart's resolution and tempo. `MsIndex` turns
ticks into ms and back (`MsIndex::at`, `MsIndex::tick_at_ms`), and
`SongTiming::display_tick_at_ms` gives the tick a screen shows at a time.

**Tempo**:
The BPM in force at a tick, `SongTiming::tempo_at`. A tick exactly on a tempo
change reads the new tempo.

**Time signature**:
The meter in force at a tick, as the chart wrote it: `apply_timesig` in
src/core/song.h stores the numerator as written and the bottom number from
`timesig_denominator`, and `SongTiming::time_signature_at` reads it back. A
signature whose beat would be shorter than one tick is refused, naming its
tick (user decision, 2026-10-14).

**Measure**:
Two questions with one owner each. Where the measure lines are is Clone
Hero's grid, `MeasureIndex` in src/core/timing.h: it starts again at the first
beat at or after a time signature change, and the measure lines, the
generated fills and the position text all read it. How many measures lie
between two ticks is Clone Hero's continuous count, `MeasureCount`: a part
measure before a mid-measure change counts as its share of the old measure.
Star Power's end, the phrase extension and the cap ceiling read it, through
`SongTiming::plusmeasure`, which walks it in exact whole numbers. The
position text (`format_measure` in src/core/timing.h, such as "m32.1.0")
counts measures and beats from 1 and the tick from 0.
_Avoid_: bar (for a measure)

**Beat**:
One step of Clone Hero's grid, so a quarter note in 4/4 and an eighth note in
6/8. The beat lines and the position text's beat number count these, so they
always agree. The game's grid step and where the grid restarts are
docs/clone-hero/04-fills.md (FL-26).
Two rules count quarter notes in every meter instead: the fill spawn deadline
(CH 1.1) and the generated fill's half-beat distance. The game's side of both
is docs/clone-hero/04-fills.md (FL-19 and FL-20 for the deadline, FL-14 for
the distance). No run of a 6/8 chart in Clone Hero is on record for either
rule (FL-20 says so for the deadline).

**Section**:
A named part of the song from the chart's section markers, such as "Chorus
1a" (`Song::practice_sections`). Its shown name is `section_display_name`'s,
from Clone Hero 1.1.0.6142's own list of 2,718 names, which Hydra ships in
data/clone-hero-1.1.0.6142-section-names.tsv; a name missing from the list
only has its underscores turned into spaces. Solo sections keep their full
name (`Song::solo_sections`).
_Avoid_: section (for a stretch of one tempo or time signature: that is a span)

**Span**:
One stretch of a single tempo or time signature inside `MsIndex`,
`MeasureIndex` or `MeasureCount`; each answers which span a tick sits in
(`MeasureIndex::span_at` and the others). `last_at_or_before` in
src/core/timing.h is the one "in force at this tick" search they share.
_Avoid_: section

**SP bar**:
One bar of the Star Power meter: two measures of Star Power
(`kMeasuresPerSpBar`, through `sp_bars_to_measures`), counted on the
continuous count. "Bar" means this and nothing else; the line drawn at a
measure's start is a measure line (`PreviewBeatKind::MeasureLine`).

### Preview

**Preview**:
The rendered, playable view of a chart: its notes as a scrolling 3D highway,
synced to the song audio. Distinct from a Path (the scoring plan) and a Chart
(the song file).
_Avoid_: player, viewer

**Note highway**:
The 3D fretboard the Preview draws, with the drum notes scrolling toward the
strike line as the song plays.
_Avoid_: fretboard, track

**Ghost path**:
A second Path the Preview draws faintly beside the one it shows: only the fills
it activates on, as edge-only outlines on the note highway
(`PreviewController::set_ghost_path`; the scene's `ghost_fills` come from
`app::apply_preview_overlay`). It sets no fill state, and a changed chart
draws none, as it draws no path (`drawn_path`). It has nothing to do with a
ghost note (a dynamics velocity, see Gem).
_Avoid_: shadow path, second path

**Gem**:
One drawn note on the note highway. Its model shows the drum type (tom, cymbal,
or kick), its texture shows the lane, the note's dynamics, and whether it sits
in an SP phrase. Dynamics are a velocity rule that applies to every lane, kick
included: velocity 1 is a ghost, velocity 127 an accent, and both score double
(see docs/adr/0012). A MIDI chart turns them on with the exact text
`ENABLE_CHART_DYNAMICS` or `[ENABLE_CHART_DYNAMICS]` in its drum track, and
only notes after the tag count; a `.chart` always has them on.
_Avoid_: note (the chart datum), block

**Lane**:
One column of the note highway, in KRYBG order. The four playable lanes
(Red..Green) spread across the highway; Kick is the full-width bar.

**Disco flip**:
A section where the chart swaps the red and yellow lanes, marked by text
events such as `[mix N drums0d]` (on) and `[mix N drums0]` (off), where N names the
difficulty: 0 Easy, 1 Medium, 2 Hard, 3 Expert. Each difficulty reads only
its own markers, and the flip happens only with Pro Drums on.
`drums0dnoflip` reads as off, unlike Clone Hero (docs/adr/0023).

**2x kick**:
A kick written for a double bass pedal: MIDI 59 Easy, 71 Medium, 83 Hard,
95 Expert, or `N 32` in a `.chart` difficulty section. Each difficulty reads
its own. The 2x Bass setting turns them on at every difficulty, like Clone
Hero's Double Kick modifier.

**Strike line**:
The fixed line on the note highway where a note is due to be hit. Notes scroll
down to it as the song plays; a gem flashes there when it passes.
_Avoid_: hit line, target line, target (the Onyx name)

**Beat line**:
A line across the note highway at a measure line, a beat, or the half-beat
before one, drawn from Clone Hero's grid (`MeasureIndex`; see Beat and
Measure).

**Active SP window**:
The stretch of the note highway from an activation to its deact node, where
Star Power is being spent. Comes from the path, not the chart; drawn as a
tinted floor, with a bright edge and a triangle beside each rail where it
ends (D81).

**Path overlay**:
Hydra's own analysis drawn on the note highway: the active SP windows and the
fills as a player following the path would see them. A fill the path
activates on is *taken* (all four lanes lit, the activation note's lane
highlighted); a fill the path had enough SP for but passed over is *offered*
(lanes lit dimly); every other candidate fill is hidden, because the game would
not have shown it. Offered fills are the ones the engine marked on each
activation as passed over; nothing guesses them from a count. A tied variant's
activations carry its own passed-over fills and early-fill offset, the next one
after a fold between windows included (D38), and a fill the path took always
stays taken. Fills
after the last activation are hidden because the engine records nothing about
them.

**SP meter gauge**:
The vertical gauge on the note highway's right edge showing banked Star Power
at the playhead. With a path, every value is the analysis's. Between activations
it steps up one bar at each tick where the engine stamped a bar's arrival. In
an active window it shows the measures left until the SP end in force, two
measures to a bar, and the analysis lists every place that end moved. It empties
exactly at the deact node. The gauge counts no phrases and applies no cap. A
late squeeze-in refills at the old SP end, because the player hits that phrase
early and SP never stops. A squeezed-out phrase's bar arrives when the player
hits it: as SP ends for an early phrase, on its own note for a late one.
Without a path it fills one bar per phrase and pins at the cap, since nothing
spends it and there is no analysis to read. Every scene built from a song has
the gauge (`PreviewScene::has_sp_gauge`), so a chart with no Star Power phrase
shows it empty, with the drain box beside it.

**Preview text box**:
One of the four boxes of text over the Preview image: the time, score, SP
drain and next-activation boxes. `render::TextBox` in
src/render/overlay_layout.h describes one as its lines (`BoxLine`: a line of
text, or the score box's star row). `measure_text_box` gives its size and
`text_box_line_origins` where each line starts, so every box's text is
left-aligned with one inset on all four sides (D114 item 27). `overlay_tops`
is the one stacking order of the boxes, read by both the fit pass and the
draw pass. The SP meter gauge is not a text box.

**Stem**:
One of the several audio files a chart may ship instead of a single mix (e.g.
`drums`, `guitar`, `song`). The Preview mixes all of a chart's stems into one
output, unpacking each from its compressed file as it plays (docs/adr/0019).

**Song length**:
How long the song runs, in chart time: where Clone Hero ends it. Three facts
feed it. The stated length is what the chart's own metadata says (a song.ini
`song_length`, a .sng `song_length` key or a .srb `song_length_ms`), moved
from audio time into chart time by the chart's delay or Offset. The end event
is the chart's `[end]` (or `end`) text event, which counts only when it sits
at least one beat past the last note. The last note is the latest note end in
any instrument at any difficulty. A stated length that ends before the last
note is dropped. The end event caps the stated length; with no stated length
left, the end event stands in, then the last note. One
second is then added, as Clone Hero adds it. No audio is opened for it.
`app::song_length_ms` works it out (D75, amended by D255), and nothing stores
it: a click works it out again. It is the one end of a song (D255, the user's
answer 5). The Paths timeline measures against it, so a song with none shows no
marks there. In the Preview, playback stops at it, and the time box's total,
the scrubber's edge and the gold marks all end there too
(`PreviewTransport::length_ms`). The audio's own length is never an input:
audio past the end does not play. With no length at all, nothing stops the
Preview's clock.

**Mixer**:
The step that reads a chart's stems at one shared position, resamples them to
one common format, and sums them into a single signal to play. It reads a few
milliseconds at a time from each stem's compressed bytes; nothing is unpacked
up front. A stem it cannot open is skipped, so one broken stem does not
silence the rest. A stem that hits a decode error part way through goes
silent from that point, and a seek to before the damage plays it again (D51
call 20). A stem whose header gives no length, a FLAC whose total is 0, is
counted by decoding it once on open (D51 call 19).

**Transport**:
The Preview's play, pause, and seek control together with its clock. The clock
is the master: while playing it is the time at play plus the time since; the
note highway reads it to place the notes, and the audio follows it.
The audio is shifted by the chart's audio offset, so the notes land on the
music as they do in Clone Hero. A nonzero song.ini `delay` replaces the
.chart `Offset`; a delay of 0 counts as unset. A positive value makes the
notes come later than the music. `preview_audio_offset_ms` in
src/app/preview_source.cpp is the one owner. This was decided in
docs/superpowers/plans/2026-09-24-derivation-fixes.md (decision 9; Task 17's
gate was measured at the game) and extended to .sng and .srb by
docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md (decision 4).
_Avoid_: player (the whole Preview), scrubber (the UI control only), playhead
(the audio follower only)

### Code layout

**Code library**:
One of the twelve static libraries the code under src/ is built as (apart
from the programs' own source files, src/ui/main.cpp and src/cli/), such as
`hydra_core`, `hydra_app` or `hydra_ui`. Hydra's own programs link them; no
outside project does. The GUI test runner and two tools have small libraries
of their own beside them. Which library a file is in is the source list in
`CMakeLists.txt` that names it, or that names the .cpp file of the same name
for a header. The folder is not the answer: src/ui/ holds files of
`hydra_preview`, `hydra_uibase` and `hydra_ui`, and src/app/ holds files of
`hydra_app` and `hydra_dm`. docs/development.md lists them in order, and
docs/adr/0032 records why.
_Avoid_: library (that is the user's charts), module, component

**Layering rule**:
A file includes only headers from its own code library and the libraries that
library links, following PUBLIC links onward but PRIVATE links one step only.
The link lines in `CMakeLists.txt` say which; tests/test_layering.cpp reads
them and fails on any include that reaches further. A third-party header
counts as the link line its row in tests/layering/externals.json names. A
wrong-way include is fixed, never parked: tests/layering/known_gaps.json stays
empty, and the case "layering: no known gap is left" fails if it is not.
_Avoid_: dependency rule
