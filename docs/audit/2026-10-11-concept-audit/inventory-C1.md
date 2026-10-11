# Concept inventory C1 (CONTEXT.md terms, grepped through src/)

Scout: C1 (task concept-audit-C1). Phase 3, read-only. I started from the 45-odd terms in the Language section of `CONTEXT.md` and grepped `src/` for each term's identifiers, its "Avoid" words and its user-visible strings. I read only the lines around each hit. Line numbers are from the tree at commit 43045f32.

Most rules in this code base already have one owner, and the comments name it. The table says so for those. The seven candidates below are the places where I found a second home that the owner's comment does not mention, or a word set spelled twice.

## Table

| Concept | Owner CONTEXT.md names | Other homes found | Candidate? | Note |
|---|---|---|---|---|
| Chart (formats) | none named; `parse/chart_files.h` (`chart_format_of`, `chart_name_format`) | `ChartKind` in `app/analysis.cpp` 229-263 (Folder/Sng/Srb, built from `ChartFormat`); `is_container_path` in `app/preview_source.cpp` 339-342 | yes (small) | C1-8. Two derived "is it a container" tests. Both are built from the one extension rule, so they agree today. |
| Library | none named | `report::library_lists`, `library_copies_by_hash`, `lacks_chart_library` in `app/report.cpp` 171-194; `list_chart_library` in the store | no | The reports and the DM page share one membership helper. |
| Scan | none named | `scan_folder`, `save_scan_as_library` (`app/analysis.cpp`), `ScanJob` (`ui/library_jobs`), `cli/batch.cpp` calls the same saver | no | One pipeline, three callers. |
| Analyze | engine.cpp / pather.cpp | `plan_batch` is shared by the GUI batch and `cli/batch.cpp`; click path reads `RecordStatus::Ready` (`ui/app_state.cpp` 431, 744) | no | Progress shares: `kMainProgressShare` (pather), `kReadShare`/`kOpenShare` (preview load). Different jobs, no overlap. |
| Record | none | in memory only | no | |
| Summary row, its three statuses | `store::RecordStatus` (`store/record_store.h` 199) | `StatusChip` (`ui/library_model.h` 26) with `chip_of`, `status_of`, `status_label`; `kStatus*` strings in `app/dm_report.h` 34-45; the "Not analyzed" and "Stale" words in `ui/library_table.cpp`, `user_messages.h` | yes | C1-1. The DM page says "not analyzed" for a chart the library calls "Stale". |
| Library file upgrade | `store/record_store.cpp`, `store/upgrade_files.cpp`, `app/user_messages.cpp` | three files, named in CONTEXT | no | K3 on purpose, as CONTEXT says. I did not read it further. |
| SP cap | none (default `kCloneHeroSpCap`, `core/model.h` 36) | `max_sp_bars` (`search/graph.cpp` 287), `graph_build_cap` (`search/pather.cpp` 445, wraps it), Preview bank clamp (`app/preview_view.cpp` 93) | no | The two ceiling helpers share one call. The Preview clamp is a different question (display of a bank). |
| Analysis settings (identity) | `Settings` (`app/config.h`) | `store::Lens`, `store::RecordKey`, `CapQuery`, `AnalysisSettings`, `BatchRun`, all built from `Settings` | no | Built in one place (`Settings::lens`, `record_key`, `batch_run`). |
| Analysis settings (spelled in words) | `describe_settings` (`app/config.cpp` 354), called "the one place" in `config.h` 219 | `batch_settings_summary` and `settings_change_phrases` (`ui/library_dialogs.cpp` 62-123); `within_label` (`app/path_view.cpp` 476); `cap_label` (`app/report.cpp` 408); row labels in `ui/settings_bar.cpp` | yes | C1-3. |
| Library search | `app/library_query.h` | report windows: `report_view::search_words` / `search_matches` (`app/report_view.cpp` 8-27); HTML report pages: `visible()` in `app/html_page.cpp` 321-327 | yes | C1-2. The report boxes say on purpose that they are simpler. The rule "every word appears" exists three times. |
| Batch time left | `batch_eta_s` (`ui/library_jobs.cpp` 275), `kEtaMinFinished` | `ByteRateClock` and `gated_time_left_text` (`ui/preview_load_job`), used by the Preview load and the startup box; words in `time_left_text` (`ui/library_parts.h` 63) | yes | C1-6. Two estimators and two gates, one shared wording. CONTEXT documents the split. |
| UI timings | none single | `kViewProgressDelaySeconds`, `kCopiedSeconds`, `kSearchThrottleSeconds` (`ui/app_state.h` 386-397) | no | `kViewProgressDelaySeconds` and `kSearchThrottleSeconds` are both 0.15, but they are separate named constants answering different questions. |
| Leaderboard fetch | `net/dmbot_client.cpp` 171 | none | no | One call site. |
| Path | `core/model.h` `Path` | a second `Path` struct inside `search/engine.cpp` (leads.md probe 5) | not read | Leads said G4b/G5 own it. I did not open it. |
| Activation / notation | `Activation::notationstr`, `notationstr_verbose` (`core/model.cpp` 554-573) | `path_view`, `preview_view` copy the string whole | no | Callers copy, they do not rebuild. |
| Skip | `Activation::skipped_fill_ticks` / `skips()` | Preview reads the stored ticks (`preview_view.cpp` 418) | no | |
| Authored fill | `place_authored_fills` (`parse/song.cpp` 359), `rules.fill_land_slop_beats` | both parsers call it (song.cpp 1199, 1866) | no | |
| Generated fill | `Song::check_activations` (`parse/song.cpp` 577) | none | no | |
| SP phrase | `close_sp_phrase` (`parse/song.cpp` 397) | `Song::sp_phrase_count`; `core/sqout_chord.h` squeeze helpers | no | |
| All-0 path (rule) | `allzero_activation` (`core/model.h` 317), `Path::is_allzero` | engine and pather ask it | no | |
| All-0 / Optimal group words | none named | `paths_tab.cpp` 174-176 (headings) and `preview_view.cpp` 710-717 (path labels) | yes | C1-5. |
| Squeeze (SqIn/SqOut phrase rule) | `core/sqout_chord.h` (`offered_phrase`, `activation_can_squeeze`) | engine, graph, replay all call it | no | |
| Squeeze window 500 ms | `kSqueezeWindowMs`, `within_squeeze_window` (`core/model.h` 110-123) | all callers use it; `config.cpp` 97 derives the setting range from it | no | |
| Frontend / Backend squeeze, Backend leeway | `Rules::backend_leeway_ms`, `core::counted_without_squeeze` | `squeeze_rating.cpp`, `path_view.cpp`, `replay.cpp` all pass it through | no | Display filter `fabs(offset) > backend_limit` (`path_view.cpp` 338) is a different question. |
| Multiplier squeeze | `MultSqueeze` (`core/model`) | | not reached | |
| Star cutoff | `star_cutoff`, `kStarMultipliers` (`core/stars`) | Preview and Stars tab call `star_cutoffs`, `stars_for_score` | no | |
| Early fill (E) | `kEarlyFillWindowMs`, `fill_refuses`, `early_fill_difficulty` (`core/model.h`) | | no | |
| Fill spawn deadline 1.0 / 1.1 | `FillDeadlineRule` (`search/graph.h` 39) | bool `legacy_fills` (`Settings`, `Lens`), `legacy_fill_deadline` (`AnalysisSettings`), DB text stamp "ch10"/"ch11" and "0"/"1" column | no | All convert through `fill_rule_for`, `engine_mode_stamp`, `fill_rule_from_stamp`. Names and descriptions come from `fill_rule_name` / `fill_rule_description` at five call sites. |
| Hit window and report tiers | `timing_tiers` (`core/squeeze_rating.cpp` 165) | `BackendSqueeze::summarystr` (`core/model.cpp` 339), `Path::is_difficult` | yes | C1-4. |
| Transfer scale, SP-end history, Deact node | `core/squeeze_rating`, `Activation::sp_end_steps`, `Activation::deact_tick` | `activation_deact_tick` (`squeeze_rating.cpp` 22) only forwards | no | Forwarder, derives nothing. |
| Cap-clamped window | `ScoreGraph::extend_deacts`, `clamp_tick()` | | not reached | |
| Squeeze rating | `rate_activation` (`squeeze_rating.cpp`) | label text lives in `BackendSqueeze::summarystr`, not in `squeeze_rating` | yes | Same as C1-4. CONTEXT says the rating label belongs to the squeeze rating; the code puts it in model.cpp. |
| Difficulty (hardest ms) | `Path::difficulty`, `squeeze_difficulty`, `early_fill_difficulty` | stored once as `hardest_ms` (`store/record_store.cpp` 521) | no | |
| Preview / Note highway | `app/preview_view`, `render/` | | no | |
| Gem, Lane | none named | `NoteColor` (`core/model.h` 51), `PreviewLane` (`app/preview_view.h` 37), `Pad`+`TrackGem` (`render/track_state.h` 43, 51), `DynamicsRow` table (`app/dynamics_breakdown.cpp` 43), `lane_color` (`ui/dynamics_tab.cpp` 24) | yes | C1-7. |
| Disco flip | `Chord::apply_disco_flip`, `emit_chord_timestamp` (`parse/song.cpp` 411) | two parser classes keep parallel state (`op_disco`, `flag_disco_`, `mode_pro_`) | K3 only | Listed under Parallel on purpose. |
| 2x kick | `lane_flag`, `set_lane_flag` (`core/model.h` 57-63) | `DynamicsRow::Kick2x`, `PreviewNote::double_kick`, MIDI note numbers in `parse/song.cpp` | no | I did not read the MIDI number table. |
| Strike line | `render/highway_draw` | | not reached | |
| Beat line | `build_beat_events` (`app/preview_view.cpp` 446) | `MeasureIndex` iteration also builds `scene.meters` (336-340) | no | Both read `MeasureIndex`. |
| Active SP window | `PreviewActivation::sp_window` (`preview_view.h` 108-116) | highway, boxes read it | no | |
| Path overlay | `apply_preview_overlay` (`preview_view.cpp` 352) | | no | One builder; rule is read off the record. |
| SP meter gauge | `build_sp_meter_curve`, `build_unanalyzed_sp_meter_curve` (`preview_view.cpp`) | | no | Two builders, same file. I did not read them. |
| Stem | `is_audio_filename`, `is_stem_filename` (`app/preview_source.cpp` 142-150) | | no | |
| Song length | `app::stated_length_ms`, `chart_song_length_ms`, `has_song_length`, `scrub_end_ms` | store column `stated_length_ms`; no second rule found | no | `has_song_length` is called by every consumer I found. |
| Mixer | `audio::mix_song_stems` | | not reached | |
| Transport / audio offset | `preview_audio_offset_ms` (`app/preview_source.cpp`), `audio_ms_of_chart_ms` (`audio/frames.h` 35) | `PreviewTransport` (ui) | no | |

## Candidates

### C1-1. What state is a chart's analysis in?
Looks like: K2 (same word, different rule), with a K1 side (a one-to-one copy of the store enum in the UI).
Homes: `store::RecordStatus` { NotAnalyzed, Stale, Ready }, `src/store/record_store.h` 195-215 (inputs: a stored summary row, its results stamp, its rules stamp);
       `ui::StatusChip` { All, NotAnalyzed, Stale, Analyzed }, `src/ui/library_model.h` 26, with `chip_of` (library_model.cpp 14-21), `status_of` (78-83), `status_label` (67-74) (inputs: a `RecordStatus`);
       `kStatus*` strings in `src/app/dm_report.h` 34-45, set in `src/app/dm_report.cpp` 109-128 (inputs: a score, the record found by hash, whether the library lists the chart, the score's speed).
Table, library (`RecordStatus` -> word, with chart in the library):
  no row -> NotAnalyzed, "Not analyzed", chip NotAnalyzed;
  row with an old stamp or other rules -> Stale, "Stale", chip Stale;
  Ready, scored best path -> "Analyzed", best path shown;
  Ready, no path kept -> "Analyzed", no score (sorts first among unscored, `unscored_rank`).
Table, DM page (`DmReportRow::status`):
  no current result and the library lists the chart -> "not analyzed" (this covers no row and, per CONTEXT, a Stale row, because the listing returns only Ready rows);
  not in the library -> "not in library";
  Ready, no path kept -> "no paths";
  Ready, scored, base speed -> "under optimal" / "at optimal" / "above optimal";
  any status, off-speed score -> "other speed" (overwrites the rest).
Differ on: a Stale row. The library calls it "Stale" and the DM page counts it as "not analyzed". The DM page's own tooltip says "without a current result", so the sense differs on purpose, but the same two words name different sets. A new status (for example a row that is Ready under another rules stamp but still shown) would have to be added to the store enum, the chip enum, and the DM strings by hand.
Note: The Stale-row claim for the DM page is from CONTEXT ("A listing returns only ready rows") and from reading `dm_report.cpp` 93-126, which looks the record up in `records_by_hash`; the verifier should confirm `records_by_hash` lists Ready rows only. The DM status is a free `std::string` compared by `==` in `dm_report.cpp` 22-27 and 149-153, and `dm_report_view.cpp` 186-192 repeats each word with a display label; those two files are G1a's.

### C1-2. Which rows does the search box keep, and how are text columns ordered?
Looks like: K2 (the report boxes are a deliberately smaller language), with a K1 inside it (the word rule exists in C++ and in JavaScript).
Homes: `query_matches`, `parse_library_query`, `term_matches`, `src/app/library_query.cpp` (inputs: typed text, a `SearchableRow` of folded title/artist/charter/folder, optional stars/squeeze facts);
       `report_view::search_words` and `search_matches`, `src/app/report_view.cpp` 8-27 (inputs: typed text, one folded search string per row);
       `visible()` in the report page script, `src/app/html_page.cpp` 321-327 (inputs: the typed text folded through a `FOLD` table, a per-row `search` string).
Table, home 1 (library): `beyonce` finds "Beyoncé"; `"two words"` is a phrase; `artist:x` limits to one field; `stars:3` and `squeeze<=5` test the best path and match only Ready rows; empty query keeps every row.
Table, home 2 (report window): the typed text is folded, split on spaces, and a row stays when every word is a substring of its search text; a quote or `artist:` is an ordinary word; empty text keeps every row.
Table, home 3 (HTML page): the same as home 2, with the fold done by a table and `split(/\s+/)`.
Differ on: homes 2 and 3 agree today on the word rule, and the comment in html_page.cpp 314-320 says the fold table is built from the library fold so accents behave the same. Homes 1 and 2 differ on purpose (quotes, field prefixes). The sort is a separate pair: the window sorts text columns by `fold_for_search` (report_view.cpp 29-34) and sinks empty cells (report_view.h 196-199); the page sorts `r[sortKey]` with `localeCompare` and sinks nulls (html_page.cpp 332-339). A name with an accent or a mixed case may sort differently in the window and on the page.
Note: I did not check what `sortKey` holds in the page script (the raw display value or a folded key), so the sort difference is a read-only guess. G1a/G1b read the report code in full and should confirm.

### C1-3. How are the analysis settings written in words?
Looks like: K2 (same setting, different spellings in different tools), claim contradicted: `config.h` 219 calls `describe_settings` "the one place they are spelled".
Homes: `describe_settings`, `src/app/config.cpp` 354-361, used by `hydra_batch`'s header lines `src/cli/batch.cpp` 181-190 (inputs: `AnalysisSettings` depth mode and value, SP cap, ms filter);
       `batch_settings_summary`, `sp_cap_value`, `score_range_value`, `path_limit_ms_value`, `settings_change_phrases`, `src/ui/library_dialogs.cpp` 62-123 (inputs: `Settings`);
       `within_label`, `src/app/path_view.cpp` 476-480 (inputs: depth mode and value);
       `cap_label`, `src/app/report.cpp` 408 (inputs: cap).
Table, score range 4 in scores mode / 1 in points mode / SP cap 4 and 1 / path limit 10 ms and off:
  `describe_settings`: "scores 4" / "points 1"; "4 bars" / "1 bar"; "10 ms" / "none".
  `library_dialogs`: "4 scores" / "1 point"; "4 bars (Clone Hero's rule)" / "1 bar (a what-if)"; "10 ms" / "off".
  `within_label`: "Within 4 scores" / "Within 1 point".
  `report.cpp`: "SP cap 4 bars" / "SP cap 1 bar".
Differ on: the score range ("scores 4" against "4 scores"), the off path limit ("none" against "off"), and the names: the CLI prints "Depth" and "Timing cap" (batch.cpp 184, 186) where the panel and CONTEXT say "Score range" and "Path limit". The CLI is a developer tool, so the split may be intended, but the `config.h` comment says otherwise.
Note: The unit-plus-count form for bars goes through `counted` everywhere; only the depth form differs. The fill-rule line is shared already (`fill_rule_name`).

### C1-4. What word rates a squeeze?
Looks like: K2 (the same words, "Hard" and "Insane", on two ladders with different edges). The report footer admits it.
Homes: `BackendSqueeze::summarystr`, `src/core/model.cpp` 339-361 (inputs: signed offset from the SP end, squeezed-out flag, hit window W, leeway, the fixed `kBackendInnerBandMs` = 10 ms);
       `timing_tiers`, `src/core/squeeze_rating.cpp` 165-176 (inputs: raw squeeze ms, hit window W; edges 2 ms, W/2, W, 1.5W, 2W).
Table, home 1 (plain backend row, W = 85, leeway 3): offset below -85 "Free"; -85 to below -10 "Easy"; -10 to below 3 "Standard"; 3 to below 85 "Hard (uncounted)"; 85 or more "Insane (uncounted)". Squeezed-out rows: "Insane / Hard / Standard / Easy / Free SqOut" at -W, -10, +10, +W, with " (uncounted)" appended when the engine would not count the note.
Table, home 2 (W = 85): up to and including 2 ms "Normal"; up to 42.5 "Hard"; up to 85 "Extreme"; up to 127.5 "Insane"; up to 170 "Insane+"; past 170 "Beyond"; no squeeze "None".
Differ on: every shared word. A row 40 ms past the SP end is "Hard (uncounted)" in home 1 and (as a 40 ms squeeze) "Hard" in home 2, but 85 ms past is "Insane (uncounted)" in home 1 and "Extreme" in home 2. The report footer (`app/report.cpp` 419-424) says the two "can differ", so this is known, but the names overlap and the ladders are in different files. CONTEXT puts the rating label under "Squeeze rating" with `squeeze_rating`, yet `summarystr` is in `core/model.cpp`.
Note: `summarystr` takes the raw offset, not the transfer-scaled one that `rate_note` computes, so the label ignores the scale while the ms figure beside it uses it. Whether that is intended is a question for the verifier. The activation badge words ("squeeze in", "early fill", `path_view.cpp` 77-81) are a third vocabulary but name the part, not a grade, so they are K0.

### C1-5. What are the path groups called?
Looks like: K1 (the same three-way split, with two word tables).
Homes: heading text in `render_path_list`, `src/ui/paths_tab.cpp` 174-176 ("Optimal", `list.within_label`, "Best all-0 path") and the all-0 hint at 178-181 (inputs: `PathButtonView::Group`);
       `preview_path_label`, `src/app/preview_view.cpp` 710-717 ("  (optimal)", "  (best all-0)", nothing for Within) (inputs: the same `Group`).
Table, home 1: Optimal -> "Optimal"; Within -> "Within N scores/points"; AllZero -> "Best all-0 path".
Table, home 2: Optimal -> notation + "  (optimal)"; AllZero -> notation + "  (best all-0)"; Within -> notation alone.
Differ on: agree today in meaning; the spellings differ ("Best all-0 path" against "best all-0"). A rename of the all-0 path would have to be made in two files, one in `ui/` and one in `app/`. The Within heading is already derived once in `path_view.cpp` `within_label`.
Note: Only a small word table; the verifier may decide it is K3 (two views of one enum, each with per-view text).

### C1-6. How much time is left?
Looks like: K2, documented. Two estimators and two gates, one wording.
Homes: `batch_eta_s`, `src/ui/library_jobs.cpp` 275-278 (inputs: elapsed seconds, charts completed, total; gate `kEtaMinFinished` = 3 charts);
       `ByteRateClock::update`, `src/ui/preview_load_job.h` 72-77 and `gated_time_left_text` (`preview_load_job.cpp` 299-301) (inputs: elapsed seconds, bytes done, bytes total; gate `kTimeLeftAfterSeconds` = 3 s and a known rate), also used by `StoreOpenProgress::time_left_text` (`library_jobs.cpp` 43-45);
       wording `time_left_text` and `format_duration`, `src/ui/library_parts.h` 63 and `library_dialogs.cpp` 75-84 (shared).
Table, home 1: fewer than 3 charts done -> none; 3 of 10 done in 30 s -> 70 s; completed greater than total -> none.
Table, home 2: under 3 s elapsed -> none; rate unknown (under 1 s of measured progress, or no bytes in the last second) -> none; otherwise (total - done) / rate, changing at most once a second.
Differ on: the estimate itself (mean rate since start against a trailing rate) and the gate (3 charts against 3 seconds). CONTEXT states both rules, so this is the "agree on the words, differ on the rule, by design" case.
Note: Listed so phase 4 can mark it K3 if it agrees. The only shared piece that could split is the wording, and that is already one function.

### C1-7. What is one drum note, as each layer sees it?
Looks like: K3-with-a-rule-inside. A chain of lane/dynamics types, each layer with its own enum, and the "Pro Drums off" rule applied at four layers.
Homes: `NoteColor` {Kick=1..Green=5}, `NoteDynamicType` {Normal, Ghost, Accent}, `NoteCymbalType`, `lane_flag`, `allows_cymbals`, `note_label`, `src/core/model.h` 49-76 (the parser's and the engine's view);
       `PreviewLane` {Kick=0..Green=4} and `PreviewNote` with bools `cymbal`, `ghost`, `accent`, `double_kick`, `lane_of` / `color_of`, `src/app/preview_view.h` 35-57;
       `Pad` {Red..Green}, `Velocity` {Ghost, Normal, Accent}, `TrackGem` {kick, pad, cymbal, velocity}, `gem_of`, `src/render/track_state.h` 43-56 and `track_state.cpp` 24-47 (inputs: one `PreviewNote`, the `pro` option);
       `DynamicsRow` table `kDynamicsRows`, `src/app/dynamics_breakdown.cpp` 43-53 (a lane plus a cymbal or 2x flag);
       `lane_color`, `src/ui/dynamics_tab.cpp` 24-33 (inputs: `NoteColor`, returns a colour).
Table, Pro Drums off and a note written as a Yellow cymbal:
  parse (`parse/song.cpp` 909 and 1715-1721): the cymbal flag is only set when `mode_pro_` is on, so the Song holds a plain Yellow note;
  `note_label(note, pro)` (`core/model.cpp` 61-63): with pro off a Yellow note reads "Yellow", a Red one "Red";
  `gem_of` (`track_state.cpp` 42): `g.cymbal = pro && n.cymbal && allows_cymbals(...)`, a second gate on a flag the parser already cleared;
  Dynamics tab (`ui/dynamics_tab.cpp` 172): `if (!pro && dynamics_row_info(r).cymbal) continue;` hides the three cymbal rows.
Table, Pro Drums on: all four layers keep the cymbal.
Differ on: agree today. The rule "with Pro Drums off a pad is a plain pad" is applied in the parser, in the label function, in the highway and in the Dynamics tab. The conversion enums are K3 by design (the comment at `preview_view.h` 35-36 says the renderer is kept off `NoteColor`); the four `pro` gates are the rule inside.
Note: Whether the Song for a non-Pro analysis truly carries no cymbal flag I read from `song.cpp` 909 only; the Dynamics tab skips the rows by setting rather than by data, so the dynamics breakdown may be built from a Pro parse. The verifier should check which Song the Dynamics tab reads.

### C1-8. Is this chart a container (.sng or .srb) or a loose folder?
Looks like: K1, tiny, agrees today.
Homes: `ChartKind` {Folder, Sng, Srb} and `chart_kind_of`, `src/app/analysis.cpp` 229-263 (inputs: a path's extension via `ChartFormat`);
       `is_container_path`, `src/app/preview_source.cpp` 339-342, plus direct `ChartFormat::Sng` tests at 368 and 380 (inputs: a path's `ChartFormat`).
Table, home 1: ".sng" -> Sng; ".srb" -> Srb; anything else, including ".mid", ".chart" and unknown -> Folder.
Table, home 2: ".sng" or ".srb" -> container; anything else -> not.
Differ on: agree today; a fifth format (a new container) would need an edit in both. Both start from `chart_format_of`.
Note: Read by grep and a 4-line read only. Probably a K0 for phase 4: `ChartKind` selects how to hash the song's identity, `is_container_path` how to find its audio.

## Parallel on purpose

The two chart parsers in `parse/song.cpp` (the MIDI reader and the `.chart` reader) keep parallel state: `op_disco`/`flag_disco_` (867, 952 and 1471, 1514), `mode_pro_` (934, 1500), `fills_` (954, 1517), `sp_start_tick_`/`sp_end_tick_`. Each side knows only its own file grammar (a MIDI note number or a text line kind) and holds the same few flags. The rules they feed are shared: `close_sp_phrase` (397), `place_authored_fills` (359), `emit_chord_timestamp` (415) with its `pro && in_disco` flip test. I read the rule helpers, not the two classes in full; G7a owns that read.

## Dropped leads

None of leads.md was my group, so I have nothing to drop. Three of its items came up in my path and I checked them. "Not analyzed" is C1-1. "Clone Hero 1.0/1.1" in `search/graph.h` and `ui/library_parts.h`: the words come from `fill_rule_name` and `fill_rule_description`, called from five places (settings_bar, fill_report, dm_report, library_dialogs, cli/batch), so it is one owner. The `Path` struct in `search/engine.cpp` I did not open.

## Not reached

Terms I did not get to, or read only by grep: Multiplier squeeze, Cap-clamped window (the overfill warning's two-part test in `rate_activation` was read, the clamp builder was not), Strike line, Mixer, the second `Path` struct in `search/engine.cpp`, the 2x-kick MIDI note table in `parse/song.cpp`, the `ByteRateClock` internals, `build_sp_meter_curve` against `build_unanalyzed_sp_meter_curve`, and the Library file upgrade's three files. I also did not check duration formatting: `format_duration` (`ui/library_dialogs.cpp` 75) and `format_song_clock` style code in `app/display_format.cpp` 70 look like two formatters for seconds, but they print different shapes (h:mm:ss against m:ss.mmm), so I left them.
