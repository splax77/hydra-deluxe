# The path report covers the whole library

Status: plan, 2026-10-15. All eight questions are answered (see "The user's answers" below). The build waits for the user's yes. Nothing here is built yet.

## The user's answers (2026-10-15, in chat)

1. Remember every failure the chart's own content causes (option b): NoNotes, ChartTimingRefused, NoteShuffleFreezes, ChartUnreadable.
2. Two nullable columns on `records` and a fourth status, CantAnalyze (option a).
3. The library cell reads "Can't analyze", dim, with the reason in the hover (option a).
4. A fifth chip, "Can't analyze (N)" (option a).
5. The line names the reason when every chart shares one, otherwise the generic line, and the Show list groups by reason (option a).
6. A batch skips these charts as "already had a result", the confirm adds the dim line, and the redo box tries them again (option a).
7. The command-line tool gets nothing more (the recommendation; the main session took it).
8. The report runs an ordinary batch, with the strip, and the window shows "Analyzing n of N charts" over a real bar (the recommendation). Added by the user's answer: when more than 500 charts need analyzing, the report shows the normal batch confirm first ("Analyze N charts that have no result yet?"). At 500 or fewer it just runs. The 500 is the number offered in the question and chosen by the user; T5 owns it as one named constant, and the guide states it.

## The plain version

Today the path report lists only the charts that already have a Ready record under the current settings, and says nothing about the rest. On the user's library that is 19,356 charts shown out of 19,512 rows (the main session's count on a copy of the user's db, 2026-10-15). The 156 left out were never mentioned. The user never agreed to that.

After this change, opening the path report first analyzes every library chart that has no Ready record under the current settings, Stale ones included, then lists them all. The window shows "Analyzing n of N charts" with a real bar while that runs, and Cancel stops it the way Stop stops a batch. Charts that cannot be analyzed, because the file has no Expert drum part, get no table row. One line above the table counts them, with a Show list link, and the Charts shown tile reads "19,453 of 19,512" (shown of library). The failure is stored per chart and per profile, so the library row shows it, the report counts it, and no batch tries that chart again until its file changes. A chart's id is its file's hash, so a changed file is a new chart.

The numbers above come from the main session's check and from a scout that analyzed the 156 missing charts on a scratch copy (per-chart outcomes in the session scratchpad, `scout_a51/outcomes.tsv`): 97 saved fine the first time (52 .sng, 30 .srb, 8 .chart, 7 .mid; 0.3 s at most each) and 59 failed every time with "No Expert Pro Drums notes in this chart." (46 .mid, 13 .chart).

## Questions for the user

Each question quotes what the code does now, gives the options and names the recommendation. Rule 3: a change to a display or to a stored record takes its design to the user first.

### 1. Which failures are remembered?

The real rule today: `run_batch` (src/app/analysis.cpp) catches every `std::exception` from `analyze_chart_file` and counts the chart as failed, with `plain_error`'s sentence. Nothing is stored, so the next batch plans the chart again. The kinds that reach that catch (`ErrorKind` in src/core/error_kind.h, sentences in src/app/user_messages.cpp):

Caused by the chart's own content under this reading, so the same file fails the same way every time:
- `NoNotesError`, kind AlreadyPlain: "No Expert Pro Drums notes in this chart." (all 59 on the user's library).
- ChartTimingRefused: "Hydra can't analyze this chart because <line>. Fix that line in the chart file or download the song again."
- NoteShuffleFreezes: "Clone Hero freezes loading this chart with Note Shuffle on."
- ChartUnreadable (a parse error such as "expected a [section] header", a damaged container): "Hydra couldn't read this chart file. It may be damaged or in a format Hydra doesn't support; try downloading the song again."

Caused by something else, so a retry can succeed:
- SongFileMissing ("Hydra couldn't open the song file. It may have been moved or deleted; run Scan library..."), HashFailed, `std::bad_alloc` ("Hydra ran out of memory on this chart..."), DatabaseWrite (the save failed), SearchBroken ("The analysis failed on this chart because of a bug in Hydra...").

Options:
- (a) Remember only `NoNotesError`. Covers every case seen on the user's library and nothing else.
- (b) Remember every failure the chart's own content causes: the four kinds in the first list. A parse error on a damaged file is remembered too, with its reason, until the file changes (a re-download is a new id). **Recommended.** The predicate is one function over `ErrorKind` and lives in one place.
- (c) Remember every failure. Rejected: a file that was locked by another program, or an out-of-memory failure on a busy machine, would read "can't analyze" forever.

With (b), `NoNotesError` gets its own kind, `ErrorKind::NoNotes`, whose sentence is its own text (as AlreadyPlain does now), so the predicate reads kinds and never words.

### 2. How the failure is stored

Today a record (`records` table, ADR 0031) holds the path count, the star count and the note count, keyed by song id and profile id; its paths are rows of `record_paths`. `store::RecordStatus` has three values: NotAnalyzed, Stale, Ready. `RecordStore::lookup` is the only place that decides the status (CONTEXT.md "Result status").

Options:
- (a) **Recommended.** Two nullable columns on `records`: `failure_kind TEXT` (the `ErrorKind` name) and `failure_text TEXT` (what the thrower said). Both NULL for an ordinary record. A failed chart is a record with path count 0, no star count, note count 0, no path rows and the two columns set. `RecordStatus` gains a fourth value, `CantAnalyze`, and `RecordLookup` carries the kind and text. Every existing rule keeps working unchanged: the stamp and rules checks (a failure saved by this build reads Stale after a stamp bump and is tried again, which is right, because a reader change can make a chart readable), the purge of charts that left the library, the deletion of empty profiles, and the save's replace. The sentence a screen shows is derived at display time from the kind, through `plain_error`'s one switch (`kind_sentence`), so no stored wording goes out of date.
- (b) A separate `record_failures` table. Cleaner on paper, but the lookup, the purge, the empty-profile deletion, the candidate query and `record_count` would each need a second branch, and each is a second copy of a rule.

The upgrade: an existing 2.8.0 file lacks the two columns. `classify_library_file` (src/store/library_file.h) is the one place that reads a file's tables and columns (single-owner rule in tests/single_owner/rules/store.json), so it gains one kind for "current layout, records table without the failure columns", and the open adds the columns in place with `ALTER TABLE` inside one transaction, the way the earlier rebuilds ran. The results stamp does not move: the engine's output is unchanged, and an old record gains two NULL columns, which changes nothing it holds (ADR 0018's rule).

### 3. The library row of a can't-analyze chart

Today `best_path_label` (src/ui/library_model.cpp) shows `status_label`'s word for any row that is not Ready: "Not analyzed" or "Stale". The cell is drawn in `kNewSongColor` for Not analyzed and `kWarningColor` for Stale (src/ui/library_table.cpp), and the hover reads `kNotAnalyzedText` ("Not analyzed yet. Click the song or run a batch to analyze it.") or `stale_text`.

Options:
- (a) **Recommended.** The cell reads "Can't analyze", in the dim text colour. Hovering shows the stored reason as `plain_error` words it: "No Expert Pro Drums notes in this chart.", with the details line under it when there is one (`error_detail_line`, empty for a no-notes chart). A click still analyzes the chart, because a click is the user's own request; it fails the same way and the panel shows the same sentence it shows today.
- (b) The reason itself in the cell ("No Expert Pro Drums notes"). Too long for the column, and it varies by kind.
- (c) Keep "Not analyzed" in the cell and put the reason only in the hover. Rejected: the chips would then count these charts as Not analyzed, and the Analyze button would promise to analyze them.

### 4. Which chip counts it

Today `chip_of` maps the three statuses to the chips All, Not analyzed, Stale, Analyzed (`StatusChip`, src/ui/library_model.h), and `refilter` counts rows per chip.

Options:
- (a) **Recommended.** A fifth chip, "Can't analyze (N)", after Analyzed. It greys out at 0 like the others. The Analyze button follows the chip (D115), so selecting it and clicking "Analyze shown (59)..." opens the confirm, which says every chart here already has a result (question 6 below).
- (b) Count them under Analyzed. Rejected: the Analyzed count would no longer equal the report's Charts shown.
- (c) Count them under Not analyzed. Rejected: a batch would promise to analyze them.

### 5. The coverage line and the Show list

Today the path report has no such line. `path_tiles` (src/app/report.cpp) writes the Charts shown tile as one number, `library.count(ids_of(rows, shown))`, the rows that pass the filters counted in library rows (copies included) through `ChartIndex::count`.

After the change the report reads one lookup over every library chart (one call to `RecordStore::lookup` over `ChartIndex::song_ids`) and sorts the charts three ways: Ready (table rows), CantAnalyze (the line), and everything else (no result: a transient failure, or a stopped run). The tile reads "19,453 of 19,512": shown of library, both through `ChartIndex::count`. The shown number still moves with the filters, as it does today.

Options for the line:
- (a) **Recommended.** "59 charts can't be analyzed · Show list". Show list expands, under the line, one row per chart: Title · Artist · reason, in library order, the way the batch strip's "N charts failed" list opens. When every chart shares one reason, the line names it instead: "59 charts have no Expert Pro Drums notes" (the user's own wording, 2026-10-15). With several reasons the list groups by reason, each group headed by its sentence and count.
- (b) Always the generic line, and the reasons only in the list. One spelling, less helpful.
- (c) One line per reason. Can grow to several lines on a library with every kind.

A second line appears only when some charts have no result: "3 charts have no result under these settings". After a finished open-analysis it means those charts failed for a reason that is not remembered (question 1), and their reasons are in the batch strip's failed list. Every count on this page, the tile and both lines, counts library rows through `ChartIndex::count`, so with no filters, shown + can't analyze + no result = the library's count, and a test pins that sum.

When every chart is in the second or third group, the window shows the lines and "Nothing to show" rather than today's "Nothing is analyzed under these settings (...)". That sentence stays for a library with no results at all under the settings before the open-analysis has run (it cannot be reached after one finishes).

### 6. What the batch says about these charts

Today `plan_batch` skips a chart when `charts_with_result` lists it, which is `RecordStore::ready_song_ids`: the Ready charts. The confirm reads "Analyze N charts that have no result yet?", or "Every chart here already has a result." when nothing is left to run, and the redo box "Also re-analyze charts that already have a result (N charts)". The counts strip reads "21 analyzed · 0 failed · 1 skipped (already had a result)" (`batch_counts_text`, D114 item 10).

Options:
- (a) **Recommended.** The skip list is "Ready or CantAnalyze" (one store function, `settled_song_ids`). A can't-analyze chart counts as skipped; the strip's wording stays, because a stored failure is a result. The confirm gains one dim line when the scope holds such charts: "N of these can't be analyzed. Check the box to try them again." The redo box re-tries them, because checking it is the user's explicit request, and the redo plan already skips nothing. `BatchPlan` gains a `cant_analyze` row count (included in `skipped`) for the confirm's line.
- (b) The same, but the redo box never re-tries them. Then only a changed file or a stamp bump ever retries, and the confirm's line says so.
- (c) A fourth strip count, "59 can't be analyzed". More numbers on the strip, all from the same plan. Possible, but the strip is already the widest line in the window.

### 7. The command-line tool

There is no `hydra_report` tool in the tree (src/cli holds batch.cpp and fillcompare.cpp; the name survives only in two test files and old plan files). `hydra_batch` goes through `plan_batch` and `run_batch`, so it stores failures on its first run ("[i/N] FAILED Title: No Expert Pro Drums notes in this chart.", as today) and skips them on later runs, counted in its closing "skipped (already had a result)". Recommended: nothing more. Option: a closing line "N charts can't be analyzed" after the failures list. `hydra_fillcompare` lists Ready records on each side and is unchanged; a can't-analyze chart gets no row.

### 8. The progress shown while the report analyzes

Today `path_report_window.cpp` sets `frame.progress_bar = false` and the building subtitle "Building the report from your library..." with grey placeholder rows. Before commit b19db916 the window drew a real bar with the overlay "Analyzing n of N records" from the report job's progress (`git show b19db916^:src/ui/report_window.cpp`, `state_body`).

Recommended: opening the report runs an ordinary batch (`AppState::start_batch` over a plan of every library row, skip list as in question 6) and the window reads that `BatchJob`'s snapshot: the first line "Analyzing n of N charts" over a real bar (`progress_fraction(completed, total)`), where n and N are `run_batch`'s own `completed` and `total`, so they are the numbers the batch strip under the toolbar shows. The strip itself shows too, with Pause and Stop and the settings lock, as for any batch; the user asked for Stop to behave like a batch. When the batch ends, `update_background_jobs` launches the report as it does after any batch, with the window's filters kept (`PathReportCause::Open`). Cancel in the window stops the batch; a stopped run builds no report, so the window shows "Report cancelled." with Try again, as today. Alternatives: hide the strip for a report-open run (a second batch look to maintain), or run the analysis inside `ReportJob` as before b19db916 (a second batch runner, which D115 removed on purpose).

Two consequences to accept with this: the "Open path report" button today shows only once the library has an analyzed chart (`library_has_analyzed`); it should show whenever the library has rows and analysis is on, since opening is now how a fresh library gets analyzed. And a settings change marks the report out of date; Refresh now analyzes the library under the new settings before it lists, which on a never-analyzed combination is a run over the whole library. That is what decision 2 asks for; the confirm is not shown for it. If the user wants a confirm before a whole-library run from the report, say so.

## What changes, by question (one owner each)

**Is this chart Ready, Stale, Not analyzed or Can't analyze?** Owner stays `RecordStore::lookup`. It reads the two new columns with the record and answers CantAnalyze when the winning candidate carries a kind. `RecordLookup` gains `failure_kind` and `failure_text`. Every other home of the three-state rule only adds the fourth case: `chip_of`, `unscored_rank`, `status_label`, `status_of` and `refilter` in library_model.cpp; the cell colour and hover in library_table.cpp; `library_has_analyzed` and the save compare at app_state.cpp 467 and 772; `ready_song_ids`, `ready_records` and `record_count` in record_store.cpp. `hydra_uitest`'s state dump reads `status_label`, so it shows the new word with no change.

**Which charts does a batch skip?** One store function, `RecordStore::settled_song_ids(profile)`, returns each Ready or CantAnalyze chart with its status. `charts_with_result` returns both sets; `plan_batch` skips both and counts the CantAnalyze rows in a new `cant_analyze` field (inside `skipped`). `ready_song_ids` goes. The single-owner row "Does this chart have a current result under these settings?" in store.json is updated to name the new function beside `lookup`.

**How does a failure become a stored result?** One app function, `app::prepare_failed_chart(song_id, profile, const std::exception&)`, returns a `PreparedRecord` carrying the kind and text, or nothing when the kind is not remembered (question 1's predicate, `remembered_failure(ErrorKind)` in core/error_kind.h). `run_batch`'s worker calls it where it records a failure, and the click's `save_view_summary` path calls it when the analysis failed. Both still go through `RecordStore::save`, the one write. A click on a chart whose stored failure has the same kind and text saves nothing, as a click with an unchanged record saves nothing today.

**Which charts does the report cover?** `report::listed_records` becomes `report::library_lookup(store, profile, library)`: one `lookup` over the library's song ids, handed back as the Ready records by id, the CantAnalyze charts with their kind and text, and the ids with no result. `collect_rows`, `collect_dm_rows` and `collect_fill_rows` all read it, so the three pages agree on which charts are covered. `ready_records` goes with `listed_records`. The leaderboard page keeps its "no current result" status for a chart that is not Ready; a chart with no Expert Pro Drums part cannot have an Expert Pro Drums leaderboard score, so no row changes there.

**How many charts does the page count?** Still `ChartIndex::count`, for the tile's both numbers and both lines. `GeneratedReport` gains the library count, the can't-analyze list (id, kind, text) and the no-result count; `path_tiles` writes "shown of library".

**When does the report analyze?** `AppState::show_path_report` and `request_path_report`: with the store readable and analysis on, they plan a batch over every library row (`plan_batch(all rows, charts_with_result(...))`); when the plan has rows to run, `start_batch` runs it and the report launches on its finish, as after any batch; when it has none, the report job starts at once, as today. While a batch is already running, the window opens in its building state and reads that batch's snapshot. The window's Cancel calls `BatchJob::stop` on a report-open run, and `cancel_path_report` on a plain build.

## Public text

docs/UserGuide.md follows docs/agents/public-doc-style.md (second person, no first person, every label checked against the code). The library section's Best path table gains a "Can't analyze" row and the chips sentence gains the fifth chip. The batch section says a chart that can't be analyzed is skipped like one with a result, that the confirm says how many, and that the redo box tries them again. The path report section loses "A finished batch builds it... Building the report only reads the results Hydra Deluxe has saved. It analyzes nothing and opens no chart file." and "There is no progress bar", and gains: opening the report analyzes every song with no current result first, with "Analyzing n of N charts" and Cancel; the coverage line and Show list; the "19,453 of 19,512" tile; and that Refresh after a settings change analyzes under the new settings. The "Open path report" sentence changes from "once your library has analyzed songs" to whenever the library has songs. The line "If nothing is analyzed under the current settings but other results exist..." is reworded for the new empty state. README.md line 20 stays true. The task owner greps the four public pages for "path report", "Not analyzed" and "Open path report" and fixes each hit. The guide goes through the doc review as the hook requires.

Internal text: CONTEXT.md's "Result status" entry gains the fourth state and names `settled_song_ids`; "Record" says a record may be a stored failure; "Batch counts" says a can't-analyze chart is a skipped row; a new entry "Report coverage" names `library_lookup`. ADR 0031 gets a dated note on the two columns and the fourth status, and ADR 0018 a line saying the stamp did not move. The decision goes into docs/audit/2026-10-03-fix-decisions.md as the next free D number (D248 was the last written on 2026-10-15; the executor checks the file).

## Prior art

A brief web search (2026-10-15) found no scanner that documents a per-file failure record, only the problem it solves. Roon keeps a "Skipped Files" list under Settings, Library, and its forum has scans that abort and restart on the same bad files (community.roonlabs.com/t/roon-stuck-scanning-library-ref-bxufbb/321592, community.roonlabs.com/t/skipped-for-almost-10-years/321045). Lyrion's tracker has "Embedded scanner continuously retries the scan if some files can't be scanned" (bugs-archive.lyrion.org/bug-15923.html) and a report where files that failed one scan were picked up on later ones (bug-15895.html), which shows the risk of a cached failure hiding a file that became readable. Keying the failure by the file's hash, as Hydra's song id already does, is the standard answer to that risk: a changed file is retried on its own.

## Tasks

Every executor runs on Opus, reads docs/agents/brief-preamble.md first, writes a status line every 10 calls and every 5 minutes, stays under 100 tool calls and 20 minutes, commits by name and never pushes. Tests are `build-cpp\Release\hydra_tests.exe -sf=<file>` for the named files and `hydra_uitest --test <name>` for the named GUI tests only. Nothing runs over the user's library.

### Wave 1

**T1, the store.** Owns src/store/record_store.h, record_store.cpp, library_file.h, library_file.cpp, src/core/error_kind.h, tests/test_store_records.cpp, tests/test_store_open.cpp, tests/single_owner/rules/store.json. Adds the two columns and the fourth status, the upgrade kind and the in-place `ALTER TABLE`, `ErrorKind::NoNotes` with `remembered_failure` and a name pair for the kind, `settled_song_ids`, the lookup's new answer, `record_count` counting only real records, and the save writing a failure record (replacing paths) and a real record (clearing the failure). Tests: a failure saves and reads back CantAnalyze with its kind and text; a failure under another build's stamp reads Stale; a real save replaces a failure and the reverse; `settled_song_ids` lists exactly lookup's Ready and CantAnalyze charts; a 2.8.0 file without the columns opens and gains them in place; a profile whose only record is a failure survives `drop_empty_profiles`. Runs `-sf=test_store_records.cpp`, `-sf=test_store_open.cpp`, `-sf=test_store_results.cpp` and the single-owner scan.

T1 fixes the API the others build against. The exact signatures go in its brief and in the wave-2 briefs:
- `enum class RecordStatus { NotAnalyzed, Stale, Ready, CantAnalyze };`
- `RecordLookup { ...; std::optional<ErrorKind> failure_kind; std::string failure_text; }`
- `RecordRow { ...; std::optional<ErrorKind> failure_kind; std::string failure_text; }`
- `PreparedRecord failure_record(std::string song_id, const Profile&, ErrorKind, std::string text);` in store
- `std::unordered_map<std::string, RecordStatus> RecordStore::settled_song_ids(const Profile&);`
- `bool remembered_failure(ErrorKind);` in core/error_kind.h

### Wave 2, in parallel worktrees forked from T1's merge

**T2, the batch and the click.** Owns src/app/analysis.h, analysis.cpp, src/ui/app_state.cpp (the save and batch planning parts only), src/ui/library_dialogs.cpp (the confirm only), tests/test_analysis.cpp, tests/test_app_click.cpp. Adds `prepare_failed_chart`, `charts_with_result` returning both sets, `BatchPlan::cant_analyze`, the worker storing a remembered failure through the one save, the click storing it, and the confirm's dim line. Tests: a batch over a corpus chart with no Expert notes stores a failure and a second batch skips it as a skipped row; a transient failure (a test analyzer throwing SongFileMissing) is not stored; the confirm's line text. Runs `-sf=test_analysis.cpp`, `-sf=test_app_click.cpp`, `-sf=test_batch_text.cpp`.

**T3, the library.** Owns src/ui/library_model.h, library_model.cpp, library_table.cpp, src/app/user_messages.h and .cpp (the hover text only), tests/test_library_model.cpp, tests/ui/uitest_library.cpp. Adds the fifth chip, the cell word and colour, the hover with the derived sentence and details line, and the chip order. Tests: chip counts over a model with every status; `best_path_label` for CantAnalyze; a GUI test that a can't-analyze row shows the word and the chip counts 1. Runs `-sf=test_library_model.cpp` and `hydra_uitest --test` for the library tests it touches.

**T4, the report model.** Owns src/app/report.h, report.cpp, dm_report.cpp (the one call), fill_report.cpp (the one call), tests/test_report_rows.cpp, tests/test_report_page.cpp, tests/test_dm_report.cpp, tests/test_fill_report.cpp. Adds `library_lookup`, the three-way split, the coverage data on `GeneratedReport`, the "shown of library" tile, the line texts, and the new empty state. Tests: the sum shown + can't analyze + no result equals the library count on the corpus; a chart with a stored failure gets no row and one list entry; the tile's text; the fill and leaderboard pages unchanged for Ready charts. Runs `-sf=` for its four test files.

**T5, the window and the open flow.** Owns src/ui/app_state.h, app_state.cpp (the report functions only), path_report_window.cpp, report_window.h, report_window.cpp, library_toolbar.cpp, library_jobs.h and .cpp (ReportJob only), tests/test_app_reports.cpp, tests/test_library_jobs.cpp, tests/ui/uitest_report_windows.cpp, tests/ui/uitest_batch_reports.cpp. Adds the open-runs-a-batch flow, the window reading the batch snapshot for "Analyzing n of N charts" over a real bar, Cancel stopping it, the coverage line with Show list and the no-result line, and the toolbar button's new rule. T2 and T5 both touch app_state.cpp in different functions; the main session merges them and resolves the one file. Tests: a GUI test that opens the report on the corpus with nothing analyzed, sees "Analyzing", waits idle and finds the tile and the line; a test that a stored failure appears in the Show list; a unit test that the window's numbers are the snapshot's. Runs `-sf=test_app_reports.cpp`, `-sf=test_library_jobs.cpp` and `hydra_uitest --test` for the report-window tests it touches.

**T6, the documents.** Owns docs/UserGuide.md, README.md, docs/README.md, CONTEXT.md, docs/adr/0018 and 0031 (dated notes), docs/audit/2026-10-03-fix-decisions.md (the new D entry), tests/test_docs_match_code.cpp if a pinned string moves. Writes the public text from the user's answers, checks every label against T3's and T5's labels as the briefs state them, and runs `-sf=test_docs_match_code.cpp`.

### Wave 3, the main session

Merge T2 through T6 onto T1's merge, resolving app_state.cpp. Run the tests that touch the changed files only (CLAUDE.md rule 1): record_store.h is included widely, so the whole suite runs once for this merge. One fresh reviewer per the derive-once hook; the guide and the plan go through the doc review.

**The real-app check before merge** (memory: UI changes need a real-app check). On a snapshot copy of the user's library through `hydra_start('window')`, with the user's own settings (profile 7: Expert Pro Drums, 2x Bass, SP cap 4): open the path report. The plan skips the 19,356 Ready charts and runs the 156, about 30 s on the scout's timings; it is not a whole-library run. Check the window reads "Analyzing n of 156 charts" while it runs, then the tile "19,453 of 19,512", the line "59 charts have no Expert Pro Drums notes" with a Show list of 59 titles, the library's "Can't analyze (59)" chip, a row's "Can't analyze" cell and its hover, and that a second open of the report runs no batch. Then open the confirm from the Can't analyze chip and read its line. Do not change an analysis setting and press Refresh on the snapshot: that is a run over the whole library under new settings, and needs the user's yes in chat first (rule 8). If the user wants that path checked too, ask.

## Not in this change

The leaderboard comparison's footer still says charts with no current result are "in your library without a result for this mode at SP cap 4: analyze them, then compare again"; with the report now analyzing on open, that page could do the same. Its own question, later.

A chart that fails for a reason that is not remembered (question 1's second list) is still planned by every batch, as today. The coverage line now says how many such charts there are, which is what the user asked for.
