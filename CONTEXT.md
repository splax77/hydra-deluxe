# Hydra

Score optimizer and path viewer for Clone Hero drums: it scans a song library,
analyzes each chart, and shows the Star Power paths that give the best score.

## Language

### Library

**Chart**:
One playable song file (`.mid`, `.chart`, `.sng`, or `.srb`) plus its metadata.
A file's format is `chart_format_of`'s answer. Whether it is a container (a
`.sng` or `.srb` that packs the chart, its metadata and its audio into one
file) is `is_container`'s. Both live in src/parse/chart_files.h.
_Avoid_: song file, track

**Library**:
The set of charts found by the latest scan of the user's folders.

**Scan**:
The pass that discovers charts in the configured folders and registers them in
the library. Distinct from analysis.

**Scanned chart**:
One chart file as the last scan found it: its hash, names, file, folder,
fingerprint (`sig`) and stated timing. `store::ChartLibraryEntry` in
src/store/record_store.h is the shape, and `app::ScanItem` in
src/app/analysis.h is another name for it, so the scan saves its rows as they
are. The scan trims each name once in `discover_charts`, before
`title_or_unknown`, `artist_or_unknown` and `charter_or_unknown` fill in a
blank one (D114 item 15). `kChartMetaStamp` in src/store/stored_versions.h
marks which reading of the names a stored row has.

**Library row**:
One line of the library table: one copy of a chart in one folder. Rows are
told apart by `row_key` in src/ui/library_model.h, the chart's notes path,
never its hash, because one chart in two folders is two rows. The selection,
a click and a batch all compare it, and the open song panel re-reads its
chart by it after a rescan, so the panel follows its row (D114 item 7).

**Rescan cache**:
The last saved scan, handed to the next one so a chart whose fingerprint has
not changed is reused without reading its files. `load_scan_cache` in
src/app/analysis.h is the one read of it; a store that cannot be read gives an
empty cache, which only makes the scan slower. Whether one chart changed since
its row was made is `chart_changed_since`'s answer, and `ChartNow::differs_from`
compares the new hash with the stored one.

**Analyze**:
The search that computes a chart's paths as a record and saves its summary.
A click on a song analyzes it, and so does a batch; the record itself is never
saved (docs/adr/0026). Its progress bar moves in steps of at least half a percent of the chart
(src/search/engine.cpp), and the main search owns the first 90% of the bar,
the all-0 pass the rest (src/search/pather.cpp; D48, Q33).

**Record**:
The result of one analysis: the kept paths, their scores, and the settings the
analysis ran with. The engine builds it each time a chart is analyzed, and it
lives in memory only (docs/adr/0026).
_Avoid_: stored record

**Summary row**:
What the database keeps of a record: the score, stars, counts, hardest timing
and best path, one row per chart and analysis settings. Its key is the chart
and its analysis settings: the chart mode, the SP cap, the fill rule, the path
limit and the score range (the result identity, below), plus the rules it ran
under. A summary row is stale unless both hold: it carries a results stamp this build accepts (a stamp that
changes only when analysis output changes, not with every release; see
src/store/stored_versions.h), and it was made under the rules in force. A
lookup reports a chart as one of three statuses: not analyzed, stale, or
ready (the result status, below). A listing returns only ready rows, so a
stale row reads the same as no row at all.

**Result identity**:
What a stored result is filed under. `store::ResultSettings` in
src/store/record_store.h is the settings part: the chart mode, the SP cap
(`store::CapQuery`) and the lens. `store::RecordKey` adds the chart's hash to
it. A lookup over many charts asks with a `ResultSettings`, a lookup for one
chart with a `RecordKey`. `Settings::record_key` builds the key from the app's
settings, and `KeyPart` in src/app/config.h names the part each analysis
setting is filed under.

**Lens**:
The part of a result's identity beyond the chart mode and the cap: the path
limit, the score range (number and unit) and the fill rule. `store::Lens` in
src/store/record_store.h keeps it in one canonical form, so equal settings
compare equal: a path limit that is off is stored as 0, and the unit as the
search reads it. `Settings::lens` builds it. The word is the code's only;
nothing a player reads uses it.

**Result status**:
What the store says about a chart's result under the current settings: not
analyzed (no row), stale (a row another build or other rules made), or ready.
`store::RecordStatus` in src/store/record_store.h; only the store decides it.
The library writes the three as "Not analyzed", "Stale" and "Analyzed"
(`status_label` in src/ui/library_model.h), on its chips and in the Best path
cell. A ready result may have kept no paths.
_Avoid_: not analyzed (for the comparison page's wider state: that page says
"no current result")

**Library file upgrade**:
The one-time rewrite of an older hydra.db into this version's smaller layout,
on the first start after updating. Hydra copies the kept tables into a new
file beside the old one, then swaps the two. A crash or a closed window at any
point leaves either the whole old file or the whole new one, and the next
start tidies up (docs/adr/0026, "The upgrade"; D100). The copy lives in
src/store/record_store.cpp, the next start's tidy-up in
src/store/upgrade_files.cpp, and the failure sentence in
src/app/user_messages.cpp. The open runs off the window's thread, so the
window never freezes; a slow one shows the startup screen.

**SP cap**:
The Star Power meter ceiling an analysis runs under, in bars. 4 is Clone
Hero's rule and the default. Other values answer what-if questions; their
scores are not achievable in game.
_Avoid_: edition, uncapped, SP meter

**Analysis settings**:
The eight settings that make up a record's identity, shown together in the
analysis settings panel and applied to every song: difficulty, Pro Drums, 2x Bass and
Note Shuffle (together the chart mode, read off the settings as a
`ChartReading` by `Settings::reading` and spelled by `chartmode_key`),
the SP cap, 1.0 fills (the fill spawn deadline),
the score range, and the path limit.
Changing one shows the library's summary rows under the new combination, and
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
the eight settings, each with the part of the result identity it is filed
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
jobs, `resolve_preview_source`, `resolve_preview_song`, `chart_song_length_ms`,
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
artist, charter or folder, in any order; `"quotes"` match a phrase, and
`title:`, `artist:`, `charter:` and `folder:` limit a word or phrase to one
field. `stars:N` (exactly N stars) and `squeeze<=N` (hardest squeeze or
required early fill at most N ms; `squeeze<N` means the same) test the summary
row's best path and only ever match ready rows. Matching
folds case and accents away, so `beyonce` finds "Beyoncé", and ignores Clone
Hero's rich-text tags.

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
skipped (already had a result)" (D114 item 10).
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
(src/net/dmbot_client.cpp; D48, Q33).

### Reports and messages

**In the library**:
Whether the last scan's library lists a chart. `report::library_lists` in
src/app/report.h is the one answer for every page, over
`library_copies_by_hash`'s map (D87 item 4, D92). `records_by_hash` keeps
only the results whose chart it lists.

**Comparable chart mode**:
Whether the analysis settings can be held against dmleaderboards scores, which
are Clone Hero scores. `dm_report::why_not_comparable` in src/app/dm_report.h
answers it: empty when they can, otherwise the sentence naming the first rule
they break. The toolbar's button and `collect_dm_rows` (Refresh, Try again,
"Compare another player...") both ask it with the real difficulty (D114 item
6). Whether a cap is Clone Hero's is `is_clone_hero_cap` in src/core/model.h.

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
What `hydra_fillcompare`'s page says about one chart scored under both fill
rules: same, 1.0 higher, 1.1 higher, only 1.0, only 1.1, or in both. The six
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
(`kNothingMatches`) and the search box's hint (`kSearchChartsHint`) live in
src/app/html_page.h, because the fill page, built without the GUI, shows them
too.

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
the raw text. A sentence more than one screen shows is a constant in the same
header, such as `kPathReportFailed` and `kComparisonFailed` (D114 item 14).

**Rules file error**:
What a bad hydra_rules.ini does. In the app, analysis stays off, and the
toolbar, the path report window and the song panel all show
`kAnalysisOffSentence` (src/app/user_messages.h; D114 items 11 and 13). A
command-line tool stops instead: `app::load_tool_rules` (src/app/rules_file.h)
loads the rules for every tool and prints `rules_file_tool_text`, the tools'
sentence and then the loader's line. Numbers in the file are read through
`parse_finite_number` (src/core/strutil.h), so "nan", "inf" and hex are
errors (D114 item 8).

**Job outcome**:
How a finished background job ended: done, cancelled or failed
(`JobOutcome`). `ResultJobBase::outcome` in src/ui/job_base.h weighs the run's
return and the Cancel flag in one place: a run that returned whole is done
even when a Cancel came after its last step, so a report keeps it. The click's
analysis is the one exception, written beside its reader,
`AppState::update_view_job` (D114 item 21). A step asked to stop throws
`hydra::Cancelled` (src/core/error_kind.h) or a type derived from it, and a
report build stopped by Cancel is marked through `ReportSlot::mark_cancelled`.

### Paths

**Path**:
One way to play a chart's Star Power: which activations to take and what each
is worth. Every path that ties the record's top score is optimal, not only the
first one listed (D48). A path's average multiplier reads 0.000x when it has no
scoring notes, instead of dividing by zero (`Path::avg_mult`); real charts
never hit this (D48, Q33).

**Score categories**:
The six parts a path's score is made of: base (the notes), combo bonus, Star
Power, solo bonus, accents and ghosts. `score_total` in src/core/model.h is
the one sum, in the order `Path` stores them. `ReplayScore` in
src/core/replay.h holds the same six (`kReplayScoreFields` names each), and
`score_of` and `assign_score` are the only pairing between it and `Path`.

**Activation**:
One use of banked Star Power, written in path notation with its skip count and
squeeze symbols (e.g. `E2+-`). An activation needs
2<!-- default: kSpActivationBars --> banked bars of Star Power, a Clone Hero
rule. `kSpActivationBars` in src/core/timing.h owns that number (D54).

**Skip**:
A fill an activation deliberately passes over before activating.

**Authored fill**:
A fill written in the chart. Each one is placed on its own once every chord
is read: on the last chord at or before its end but not before its start, or
on the first chord after its end within the landing window
(`fill_land_slop_beats`, plus one tick). The closer wins, a tie goes to the
later chord, and a fill with neither is dropped (docs/adr/0023).

**SP phrase**:
A chart section that awards a bar of Star Power when hit fully. It covers
the chords with start <= tick < end, and pays on the last of them; a phrase
of length zero pays nothing, and one that runs past the last note pays on
that note (Clone Hero's rule, docs/adr/0023).
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
no Star Power) times 0.1, 0.5, 1.0, 2.0, 2.8, 3.6 or 4.4, rounded up. Clone
Hero compares it against your score without the solo bonus. The product is
kept as a 32-bit float before it is rounded up, as the decompiled game does,
which can move a large cutoff by one point (`star_cutoff` in
src/core/stars.cpp; D48, Q33).

**Early fill (E)**:
An activation timing (the `E` notation) where the fill must be summoned by
hitting early. Its window is 60 ms<!-- default: kEarlyFillWindowMs -->
(`kEarlyFillWindowMs`), fixed, not the hit-window setting. It was tightened
from 85 ms. The user kept 60 ms because paths were cluttered with early fills
that never matter (D51 call 7); the hardest real early fill on 18,773 charts
was 57.7 ms.

**Fill spawn deadline (CH 1.1)**:
The latest your SP meter can fill up and still have a fill appear. Clone Hero
1.1 puts it a flat 4 beats before the fill starts. Hydra's default rule.
`kCh11FillLeadBeats` in src/search/graph.h owns the 4, and
`fill_rule_description` writes it into the sentence the "1.0 fills" tooltip
and the fill comparison page show. A
beat here is a quarter note (the chart's resolution) in every meter, 6/8
included; that is not verified against Clone Hero (docs/adr/0023).

**Fill spawn deadline (CH 1.0)**:
The older rule: roughly one fill-length of lead time before the fill, clamped
to 250..10000 ms. Short fills got stricter in 1.1 and long fills got looser.
The "1.0 fills" analysis setting, or `hydra_batch --legacy-fills` into its own
database. Part of a summary row's key, so 1.0 and 1.1 rows sit side by side;
`hydra_fillcompare` diffs the two. See docs/adr/0010.

**Generated fill**:
A fill Hydra places itself on a chart with no authored fills: on the chord
nearest a downbeat, if one sits within half a beat of it, half a measure
long, and at least 4 measures after the last one (hydra_rules.ini can change
these). With two or more meter changes
between two chords, its length reads the earlier meter; that is not
verified against Clone Hero (docs/adr/0023).
When two chords sit equally near the downbeat, the later one gets the fill.
How far from the downbeat that chord may sit, and the fill's length, are cut
down to whole ticks.
Neither rule is verified against Clone Hero either; both live in
`Song::check_activations` (docs/adr/0023, D74).

**Hit window**:
The per-side ms window Clone Hero registers a hit in. A setting; feeds the
squeeze budgets, ratings, and report tiers, never the search. The report's
timing tiers are Normal below 2 ms (`kDifficultMs`), then Hard below half a
hit window, Extreme below one window, Insane below one and a half, Insane+
below two, and Beyond from two windows up (`timing_tiers` in
src/core/squeeze_rating.cpp; D48, Q33).

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
record as it runs, and nothing downstream re-derives it: every reader asks
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
(`rate_note` for each row). The Rating cell's word ("Easy", "Hard
(uncounted)" and so on) is `BackendSqueeze::summarystr`'s, in
src/core/model.cpp. That ladder differs from the report's timing tiers
(`timing_tiers`), and the report's footer says the two can differ.

**Needs no timing**:
A squeeze that is already done with no frontend timing: a SqIn whose note the
SP walk already pays, or a SqOut whose note it does not. `SPSqueeze::needs_no_timing`
in src/core/model.h is the one answer, and the rating and the details sentence
both read it. The word "Free" in the Rating cell is a different test,
`BackendSqueeze::summarystr`'s, on a backend row's offset.
_Avoid_: free (for this test)

**Backend leeway**:
How long after the SP end a note still scores under Star Power without a
squeeze: less than `backend_leeway_ms`
(3 ms<!-- default: Rules::backend_leeway_ms --> by default). A note exactly
3.0 ms<!-- default: Rules::backend_leeway_ms --> after the SP end does not
score under SP. Hydra's own rule: no such constant was found in the Clone
Hero engine methods read; the 3 ms<!-- default: Rules::backend_leeway_ms -->
is Hydra's own setting.

**Difficulty**:
A path's or activation's hardest squeeze or required early fill, in raw gap
ms — never scaled by the transfer scale.

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
A line across the note highway at a bar, a beat, or the half-beat before one,
drawn from the chart's timing.

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
at the playhead. With a path, every value is the record's. Between activations
it steps up one bar at each tick where the engine stamped a bar's arrival. In
an active window it shows the measures left until the SP end in force, two
measures to a bar, and the record lists every place that end moved. It empties
exactly at the deact node. The gauge counts no phrases and applies no cap. A
late squeeze-in refills at the old SP end, because the player hits that phrase
early and SP never stops. A squeezed-out phrase's bar arrives when the player
hits it: as SP ends for an early phrase, on its own note for a late one.
Without a path it fills one bar per phrase and pins at the cap, since nothing
spends it and there is no record to read.

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
How long the song runs, in chart time. It is the length the chart's own
metadata states (a song.ini `song_length`, a .sng `song_length` key or a .srb
`song_length_ms`), moved from audio time into chart time by the chart's delay
or Offset. A chart whose metadata states none gets the start of its last
Expert drum note instead (2x kick included). No audio is opened for it.
`app::song_length_ms` works it out (D75), and the store keeps one per song. The
Paths timeline measures against the length, so a song with none shows no marks
there. The Preview's gold marks on the scrubber are placed by it too when there
is one. With none, they use the transport's playback range instead (D48, D70
item 1), which is not a song length. The scrubber itself ends no further than
playback can reach: the shorter of the song length and the playback end (D114
item 17). `playhead_range` owns both choices, and `PreviewTransport::range()`
hands out the result.

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
