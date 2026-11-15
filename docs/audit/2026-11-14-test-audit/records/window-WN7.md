# Window round WN7 (agent a3c0072c80a89313b)

Task T5-WN7, session e633f7f6-f313-4787-b622-8d901b577b64. Group: `tests/test_path_list.cpp`, `tests/test_pb_files.cpp`, `tests/test_preview_boxes.cpp`, and every test case of `tests/test_app_library.cpp` with no record in `records/delta-DL3.md`. Code read at the worktree HEAD. Nothing was run. Every claim is "by reading".

## Which window these tests run under

The default is 60: `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`), read by `SearchSettings` (`src/search/pather.h:45`) and by `Settings::early_fill_window_ms` (`src/app/config.h:195`, passed on at `src/app/config.cpp:411`). Nothing in this group sets it. The rule is `fill_refuses(e_offset, window_ms)` (`model.h:331`) and `is_e0` (`model.h:335`); the search reads it at `src/search/engine.cpp:859-869, 1007, 1224`. A smaller window refuses more fills and marks fewer activations E0.

Search entry points in the group, found by a grep over the four files and the helpers they include:

- `corpus::analyzed_by_ten_scores()` (`tests/analyzed_chart.h:16-25`): `AnalysisSettings` with depth 10 scores and `ms_filter` 10, window not set (default 60). It calls `first_analyzed_with_paths` (`tests/corpus_util.h:132`), which analyzes corpus charts in order and returns the first with a non-empty `paths`. Used at `test_path_list.cpp` lines 95, 144, 170, 198, 220, 230 and `test_preview_boxes.cpp` lines 469 and 499.
- `song_details::burnout()` (`tests/song_details_fixtures.h:24-33`): `analyze_chart_file` on Green Day - Burnout with `scratch_settings().to_analysis_settings()`. `scratch_settings` (`tests/scratch_settings.h:18-24`) sets only the chart folder, `auto_open_report` and `depth_value = 2`, so the window is the default 60. Used at `test_path_list.cpp` lines 269, 328, 342, 399.
- `open_and_wait(s, corpus::burnout_notes())` through `open_burnout` (`tests/test_pb_files.cpp:33`, `tests/pb_open_util.h:16`): `PbState::open_with(path, scratch_settings())`, the same default 60. Every `test_pb_files.cpp` case opens it. `c.graph` and `c.settings` are the open's own search graph and settings, so `start_window`, `open_path_file` and `paste_path` run at the same window as the record.
- `test_app_library.cpp`: the only real search is `run_redo_batch_over` (`tests/app_state_fixtures.h:222-235`, which calls `analyze_chart_file` when the stored record is not empty) and a click's view job (`select(..., SelectionSource::Click)`). Every other batch in the file runs a fake `ChartAnalyzer`: `gated_scores` (`test_app_library.cpp:204-216`) returns `scored_record(...)` with no engine, and the other lambdas throw. `run_redo_batch_over` is called only from lines 83 and 107, both in `records/delta-DL3.md`.

No engine record at 60 exists: `records-engine/` is at 85, and none of its 16 files is Burnout.

Two things the window does not reach, read in code:

- `record.multsqueezes` come from the search graph's chart pass: `record.multsqueezes = graph.multsqueezes()` (`src/search/pather.cpp:27`), filled by `ScoreGraph::store_multsqueeze` (`src/search/graph.cpp:263`). `graph.cpp` stores the window (line 100) and never branches on it. So Burnout's one squeeze (`+15 pts`) and every label built from `rec.multsqueezes` cannot move with the window.
- The Preview's replay (`replay_stored_path`, `src/core/replay.h:304`) and `build_preview_scene` do not call `fill_refuses` or `is_e0`: a grep of `src` outside `src/search` finds those only in `model.h` and `model.cpp`. A hand-built `Activation` has the default `e_offset` and `e_window_ms = kDefaultEarlyFillWindowMs` (`model.h:606`).

## Counts

| File | Cases | A | B | C | delta round |
|---|---|---|---|---|---|
| `tests/test_path_list.cpp` | 15 | 0 | 13 | 2 | 0 |
| `tests/test_pb_files.cpp` | 8 | 0 | 6 | 2 | 0 |
| `tests/test_preview_boxes.cpp` | 53 | 0 | 52 | 1 | 0 |
| `tests/test_app_library.cpp` | 52 | 0 | 38 | 0 | 14 |

C verdicts: Keep 0, Rewrite 4 (records 1, 2, 3, 5), Remove 0, Investigate 1 (record 4).

## test_path_list.cpp

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 23 | build_score_breakdown: exact lines, rounded like the report | B | A hand-built `Path` (`notes_points = 3`, `score_combo = 2`); no search. |
| 42 | path buttons: each path's own hardest timing, warn past the difficult floor | B | Hand-built `Path`s and `Activation`s; `e_offset = 300.0` (line 57) is past either window, so not E-critical at 60 or 85. No search. |
| 80 | build_path_buttons: a 1-bar cap says why there are no activations | B | Hand-built `HydraRecord` with `sp_cap` 1 and 4. No search. |
| 94 | build_path_list: score groups and the all-0 dedupe rule | B | Reads `analyzed_by_ten_scores` (default 60). Every asserted value is computed from the same record: `total == flat.size()`, the label equals `group_thousands(first.totalscore())`, the 100-point and tick-plus-1 edits are the test's own. Residual: lines 131-132 REQUIRE the first path to have an activation with an SP end. A window that left the first corpus chart's best path with none would fail loudly, not give a wrong value. |
| 143 | build_path_list: one duplicate hides only itself | B | Same helper; relational (`path_identity` of the list row equals the variant's). Same residual REQUIREs at 147-148. |
| 169 | path_identity: same path, rescored path, trimmed path | B | Same helper; each check compares a copy with an edit of the same path. Same residual REQUIREs at 173 and 192. |
| 197 | build_path_list: the dedupe and the overlay key read one identity | B | Same helper; loops every path of the record, relational. |
| 219 | build_multsqueezes: one labeled entry per squeeze | B | `v.size() == rec.multsqueezes.size()`; `multsqueezes` come from the graph's chart pass (`pather.cpp:27`, `graph.cpp:263`), not the window. |
| 229 | PathsTabCache: views are built once and rebuilt only when their inputs move | B | Build counters, relational to the same record. Line 254 `if (all.size() > 1)` skips its fourth check when the record has one path, silently (old record PL9 in `records/A19.md` names it). |
| 268 | path buttons: Burnout's list, in the mockup's groups | C | Record 1. |
| 327 | multiplier squeeze: Burnout's one squeeze and the fold's summary | B | Reads only `burnout().record.multsqueezes`, which the graph pass fills (`pather.cpp:27`). Typed `+15` and `[Red snare - Yellow cymbal]` (lines 330-336) cannot move with the window. Its old verdict (Rewrite, `records/A19.md` PL11) is unchanged and is not this round's. |
| 339 | multiplier squeeze: the note names follow the Pro Drums setting | B | `cache.details(...).squeezes` is built from `rec.multsqueezes`; `rec.best_path()` is only the cache key. Old verdict (Investigate, claim not checked only, PL12) unchanged. |
| 362 | PathsTabUi: one row open at a time, expand and collapse all | B | `PathsTabUi` alone, no record. |
| 398 | PathsTabCache: folds reset for a new path, not for a display setting | C | Record 2. |
| 433 | path_item_id: the label, two hashes and the index | B | A string function on typed strings. |

## test_pb_files.cpp

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 50 | pb files: a saved path file opens back as the same path, as the player's | C | Record 3. |
| 84 | pb files: a path file's first path becomes the player's | C | Record 4. |
| 110 | pb files: a path file that cannot be read changes nothing and says why | B | A file that does not exist; asserts two sentences and unchanged history. The only search-derived requirement is `open_burnout`'s non-empty `paths`. |
| 123 | pb files: a path file that does not fit the chart changes nothing and says why | B | Two bad files: not JSON, and one window at ticks 1 and 2. Tick 1 holds no fill at any window (a smaller window only refuses more fills), so the refusal and its `path 1 of ...` detail do not depend on the window. |
| 152 | pb files: a path file that cannot be written says why | B | Saves `best_path()` into a folder that does not exist; asserts the error sentences. Any path would do. |
| 170 | pb files: Copy writes Hydra's long Copy path text | B | `copy_text` is `pathstring_verbose(multsqueezes)` (`pb_files.cpp:27-29`), which always adds a "Score:" section (`model.cpp:623`), so it differs from `pathstring()` for any path. Both sides read the same best path. |
| 179 | pb files: a copied path pastes back as the same path, long or short | B | Loops every path of the record, relational (`check_same_path` against the same path); the paste reads fills from the same `c.graph` at the same window. |
| 201 | pb files: a paste that does not fit the chart is refused and leaves the path as it was | B | `"99"` matches no window number; the `best.pathstring()` round trip is relational. |

## test_preview_boxes.cpp

The file builds every song and path by hand (`make_hand_song`, `make_fill_song`, `make_sp_song`, `meter_song`, `sp_act_at`, `priced_path` in `tests/preview_song_fixtures.h`) and prices them with the replay, which does not read the window. A grep over the whole file for `analy`, `search`, `Engine`, `ScoreGraph`, `fill_`, `early`, `window`, `is_E` and `e_offset` finds the search includes at lines 35-36 (unused), two `analyzed_by_ten_scores` calls (lines 469, 499), `e_offset = 300.0` at line 953 (past either window) and `"E0 (0.0 ms)"` at line 979 (a default `e_offset` of 0 is below either window). Bodies read: 425-466, 466-512, 823-884 (head), 940-985, and the helpers at 30-114. The other B rows rest on the grep and the helpers; see "Not read whole" at the end.

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 115 | build_time_box: timestamp, measure, tempo | B | Hand-built song and path, priced by the replay; no search, no window. |
| 137 | build_time_box: the timestamp rounds 59,999.6 ms to 1:00.000 | B | Hand-built song and path, priced by the replay; no search, no window. |
| 145 | build_time_box: the end measure runs past the last beat line | B | Hand-built song and path, priced by the replay; no search, no window. |
| 157 | build_time_box: the practice section in force | B | Hand-built song and path, priced by the replay; no search, no window. |
| 179 | build_time_box: a mid-measure meter change restarts the grid | B | Hand-built song and path, priced by the replay; no search, no window. |
| 205 | build_time_box: a change between two beats shows from the next beat | B | Hand-built song and path, priced by the replay; no search, no window. |
| 223 | build_time_box: the time signature in force, as the chart wrote it | B | Hand-built song and path, priced by the replay; no search, no window. |
| 247 | step_shown_ms: a step moves the shown moment by exactly its size | B | Hand-built song and path, priced by the replay; no search, no window. |
| 255 | step_shown_ms: never before 0 or past where playback ends | B | Hand-built song and path, priced by the replay; no search, no window. |
| 262 | step_shown_ms: a playhead outside the song steps from the moment shown | B | Hand-built song and path, priced by the replay; no search, no window. |
| 272 | score box: no path hides the box | B | Hand-built song and path, priced by the replay; no search, no window. |
| 280 | score box: a solo's bonus lands on its last note | B | Hand-built song and path, priced by the replay; no search, no window. |
| 311 | score box: before the first note nothing is hit yet | B | Hand-built song and path, priced by the replay; no search, no window. |
| 324 | struck_at: a chord exactly on the playhead counts as hit | B | Hand-built song and path, priced by the replay; no search, no window. |
| 332 | score box: the multiplier is the replay's, doubled on chords Star Power pays | B | Hand-built song and path, priced by the replay; no search, no window. |
| 371 | score box: past the length it reads the moment the time box shows | B | Hand-built song and path, priced by the replay; no search, no window. |
| 384 | score box: a chord exactly on the playhead counts as hit | B | Hand-built song and path, priced by the replay; no search, no window. |
| 401 | score box: a leeway chord past the SP end is paid doubled, the box stays plain | B | Hand-built song and path, priced by the replay; no search, no window. |
| 445 | score box: a path the replay can't reproduce says so | B | Hand-built song and path, priced by the replay; no search, no window. |
| 468 | score box: the analyzed chart ends on the path's total | B | Reads `analyzed_by_ten_scores` (default 60). `end.score == group_thousands(best.totalscore())` compares the replay with the engine score of the same path; the replay does not read the window and both sides take the same best path. |
| 485 | score box stars: before the first chord no star is earned | B | Hand-built song and path, priced by the replay; no search, no window. |
| 498 | score box stars: the analyzed chart ends on the path's own star count | C | Record 5. |
| 513 | score box stars: a solo's bonus raises the score but not the stars | B | Hand-built song and path, priced by the replay; no search, no window. |
| 552 | score box stars: no score, no stars | B | Hand-built song and path, priced by the replay; no search, no window. |
| 569 | drain box: idle on a chart with no Star Power | B | Hand-built song and path, priced by the replay; no search, no window. |
| 585 | drain box: idle at a steady tempo reads the rate and a full meter | B | Hand-built song and path, priced by the replay; no search, no window. |
| 598 | drain box: the rate switches exactly at a tempo change | B | Hand-built song and path, priced by the replay; no search, no window. |
| 613 | drain box: past the length it reads the moment the time box shows | B | Hand-built song and path, priced by the replay; no search, no window. |
| 627 | drain box: a 7/8 section drains faster at the same BPM | B | Hand-built song and path, priced by the replay; no search, no window. |
| 638 | drain box: active inside the stored SP window, idle outside it | B | Hand-built song and path, priced by the replay; no search, no window. |
| 662 | drain box: empties in reads the stored end, not a recount | B | Hand-built song and path, priced by the replay; no search, no window. |
| 677 | drain box: an activation with no stored end stays idle | B | Hand-built song and path, priced by the replay; no search, no window. |
| 691 | solo_percent: the game's float32 truncation | B | Hand-built song and path, priced by the replay; no search, no window. |
| 702 | solo box: from before the solo to its bonus, timed off the last chord | B | Hand-built song and path, priced by the replay; no search, no window. |
| 738 | solo box: the count is notes, the bonus thousands-grouped | B | Hand-built song and path, priced by the replay; no search, no window. |
| 753 | solo box: a solo that starts during the last one's results takes over | B | Hand-built song and path, priced by the replay; no search, no window. |
| 771 | solo box: the same with or without a path | B | Hand-built song and path, priced by the replay; no search, no window. |
| 787 | solo box: a bonus over a thousand has a separator; none without solos | B | Hand-built song and path, priced by the replay; no search, no window. |
| 800 | solo box: past the length it reads the moment the time box shows | B | Hand-built song and path, priced by the replay; no search, no window. |
| 808 | solo box samples: each solo's widest boxes | B | Hand-built song and path, priced by the replay; no search, no window. |
| 823 | scrub marks: each activation's onset over the scrubber's length | B | Hand-built song and path, priced by the replay; no search, no window. |
| 835 | song_fraction: a clamped share, none without a length | B | Hand-built song and path, priced by the replay; no search, no window. |
| 844 | song_fraction: has_song_length takes only a positive length | B | Hand-built song and path, priced by the replay; no search, no window. |
| 856 | scrub marks: an activation sits at the Paths timeline's fraction when the audio outlasts the notes | B | Hand-built song and path, priced by the replay; no search, no window. |
| 877 | scrub marks: a playhead past the last note parks the thumb at the right end | B | Hand-built song and path, priced by the replay; no search, no window. |
| 886 | activation jumps: nearest activation before or after the playhead | B | Hand-built song and path, priced by the replay; no search, no window. |
| 899 | next activation box: the activation at or after the playhead | B | Hand-built song and path, priced by the replay; no search, no window. |
| 912 | next activation box: past the length it reads the moment the time box shows | B | Hand-built song and path, priced by the replay; no search, no window. |
| 919 | next activation box: the note names follow the Pro Drums setting | B | Hand-built song and path, priced by the replay; no search, no window. |
| 929 | next activation box: on an activation within half a millisecond, and no chord | B | Hand-built song and path, priced by the replay; no search, no window. |
| 949 | next activation box: the notation line is the activation's own, header and detail unchanged | B | Hand-built `Activation`s: `e_offset = 300.0` (line 953), then `TwoActs`, whose first activation has the default `e_offset` 0 and reads `E0 (0.0 ms)` (line 979); 0 is below any window. No search. |
| 1027 | build_time_box: every lookup at its boundaries on a chart of many changes | B | Hand-built song and path, priced by the replay; no search, no window. |
| 1087 | build_time_box: inside the half tick before a tempo change every line shows the new values | B | Hand-built song and path, priced by the replay; no search, no window. |

## test_app_library.cpp

The 14 cases already in `records/delta-DL3.md` are marked "delta round" and not judged: lines 83, 107, 249, 270, 280, 303, 483, 514, 529, 1242, 1261, 1277, 1293, 1312. The other 38 are B. Bodies read: 135-165, 314-372, 412-425, 451-482, 544-610, 655-725, 778-809, 1181-1220. For the rest, the reason is the file-wide grep: the only path from this file to `analyze_chart_file` is `run_redo_batch_over` (called only at lines 95 and 121, in delta cases) and the click's view job, and the only asserted values near a click are selection and job state.

| Line | Test case | Kind | Reason or record |
|---|---|---|---|
| 54 | set_search narrows the library and the match count | B | Searches a seeded store's rows; no analysis. |
| 70 | set_search: a filter that does not parse leaves the query empty | B | Searches a seeded store's rows; no analysis. |
| 83 | a batch result for the open chart refreshes its row, not the panel | delta round | `records/delta-DL3.md` |
| 107 | a batch result for an open Ready chart changes its row and keeps the panel | delta round | `records/delta-DL3.md` |
| 135 | refresh_library_row picks up one chart's new result | B | `save_batch_record` stores an empty analysis for one row; no search. |
| 249 | a batch refresh fills scores in where the rows sit; the batch's end sorts them | delta round | `records/delta-DL3.md` |
| 270 | a batch whose last result was already read still sorts when it ends | delta round | `records/delta-DL3.md` |
| 280 | a batch's end sorts once, and the next batch sorts when it ends | delta round | `records/delta-DL3.md` |
| 303 | a batch dismissed before a tick saw it finish still sorts | delta round | `records/delta-DL3.md` |
| 314 | a batch that fails as a whole sorts a held order when it ends | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 346 | a tick during a batch does not wait for the store | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 360 | a tick during a batch re-reads no row the batch did not save | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 372 | a batch's end re-reads no row the batch did not save | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 393 | a batch that fails as a whole still re-reads every row when it ends | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 412 | a lookup the batch could not make shows the store's problem | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 451 | a chart in two folders gets both rows from one batch result | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 483 | a failed chart whose failure the batch stored shows on its row, and one it did not " | delta round | `records/delta-DL3.md` |
| 514 | a paused batch's stored results reach their rows | delta round | `records/delta-DL3.md` |
| 529 | a stopped batch's stored results reach their rows, then the list sorts | delta round | `records/delta-DL3.md` |
| 544 | one song's stored result keeps its row in place, with no batch running | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 555 | a settings change's re-read of the summaries sorts a held order at once | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 566 | a reload comes up sorted under Best path | B | Batch with a fake analyzer (`gated_scores` returns `scored_record(...)`, or a lambda that throws); row order and store reads are asserted, no engine runs. |
| 594 | is_selected_row: the selected row is the one with its notespath, not its md5 | B | Compares notespaths and md5; no analysis. |
| 608 | a scan cannot start during a batch, and the status line says so | B | `start_batch_until_stopped`: the analyzer waits for Stop and throws; asserts the scan refusal sentence. |
| 660 | a click right after a scan, before any frame, opens the chart the scan kept | B | A click starts a real view analysis at the default 60, but the asserts read only `selected`, `details_open()`, notespath and md5 (660) or their absence (672); no result value. |
| 672 | a click right after a scan on a row the scan removed does nothing | B | A click starts a real view analysis at the default 60, but the asserts read only `selected`, `details_open()`, notespath and md5 (660) or their absence (672); no result value. |
| 683 | the batch confirm right after a scan, before any frame, counts the library's N | B | Counts of library rows and store results, and a batch whose analyzer throws (778); no engine result. |
| 693 | any_job_running lists every background job | B | Asserts only `any_job_running()` around a stopped batch and a latched click job; no result value. |
| 723 | the confirm counts every library row, and rows with a result from the store | B | Counts of library rows and store results, and a batch whose analyzer throws (778); no engine result. |
| 746 | the batch confirm counts the rows on screen when a search and a chip narrow them | B | Counts of library rows and store results, and a batch whose analyzer throws (778); no engine result. |
| 778 | the batch runs the plan the confirm showed, with no second store read | B | Counts of library rows and store results, and a batch whose analyzer throws (778); no engine result. |
| 810 | an AppState whose database can't open fails its startup open with DatabaseOpen | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 827 | the startup open runs on its own thread and tick collects the store and library | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 898 | an AppState closed during its startup open stops the open | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 915 | a library read that fails after the startup open is the status line's problem | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 937 | an AppState on a pre-2.2.0 library sets it aside, leaves one notice, then asks for " | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 982 | the scan after the set-aside box says the library needs a scan when a chart folder " | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1015 | an AppState on a current library leaves no notice and asks for no scan | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1049 | an AppState on a library scanned by an older Hydra asks for a scan, marked as its own | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1061 | an AppState on an empty library asks for no scan | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1075 | a cancelled scan on open leaves the library out of date; a finished one clears it | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1106 | a refused start_scan drops the open's mark so a later scan the user starts has no notice | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1120 | an out-of-date library with a chart folder that cannot be read starts no scan | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1149 | start_scan with a chart folder that cannot be read starts no scan and keeps every " | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1181 | the startup screen's line names what the open is doing | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1203 | Analyze library on a database that fails shows the sentence and opens no confirm | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1220 | a library reload whose reads fail keeps the library and says so | B | Startup open, library or scan notices on scratch or junk databases, and status sentences; no analysis. |
| 1242 | opening a song does not wait for a save | delta round | `records/delta-DL3.md` |
| 1261 | select_chart on a hidden chart does not wait for a save | delta round | `records/delta-DL3.md` |
| 1277 | opening the batch confirm does not wait for a save | delta round | `records/delta-DL3.md` |
| 1293 | a settings change re-reads the rows without waiting for a save | delta round | `records/delta-DL3.md` |
| 1312 | a failed reader read shows the store's problem | delta round | `records/delta-DL3.md` |

## Typed numbers a run at 60 ms can move

Each is on an assertion line, from a run before the default became 60. All come from `burnout()` or `analyzed_by_ten_scores()`.

- `tests/test_path_list.cpp:272` four buttons; `:276` best path "3- 1 2"; `:278` fold "378,315"; `:281` timing "163.0 ms"; `:282` timing_warn; `:286, :287` fold "378,175", title "0 4 1"; `:291, :292` fold "378,075", title "2 1 2"; `:298` "375,955" and "0 0 0 0"; `:299` "2,360 below optimal". Record 1.
- `tests/test_path_list.cpp:406` `act_open.size() == 3` (Burnout's best path has three activations); `:426-430` button build counts that hold only while the best path has that many rows. Record 2.
- `tests/test_preview_boxes.cpp:509-510` `end.stars == 7` and `end.next_star.empty()`, comment "Pinned from one run". Record 5.
- `tests/test_pb_files.cpp:56, :88` `best_acts.size() >= 2` (a precondition typed from Burnout's best path); `:61, :93` the one-window path differs from every record path. Records 3 and 4.
- Not moved by the window, read in code: `test_path_list.cpp:330-336, :352-359` (`+15 pts`, `[Red snare - Yellow cymbal]`, `gain == 15`), because `multsqueezes` come from the graph's chart pass (`pather.cpp:27`). They are still snapshot numbers; the old Rewrite verdicts (`records/A19.md` PL11, PL12) stand.

## C records

### 1. path buttons: Burnout's list, in the mockup's groups (tests/test_path_list.cpp:268)
Delta round: window, replaces records/A19.md record PL10
Read: `test_path_list.cpp:268-325`; `song_details_fixtures.h:22-33`; `scratch_settings.h:18-24`; the old record PL10; `records/window-WN4c.md` record 1 (same chart, `test_activation_rows.cpp:582`). Not read: the body of `build_path_buttons` (`src/app/path_view.cpp`), taken from the old record.
Does the old verdict hold at 60: Rewrite stands. Its reason (every value is a snapshot of one run) now has a second cause: the run was before the default window became 60, and nothing in this tree shows Burnout's path list at 60.
1 Claim: On Burnout at depth 2 scores, the Paths tab lists four buttons: best "3- 1 2" (378,315, 163.0 ms, warn), "0 4 1" (378,175), "2 1 2" (378,075) and the all-0 path "0 0 0 0" (375,955, "2,360 below optimal"); a tied root and variant share one fold.
2 Owner: `build_path_buttons` for grouping and labels; the path list and scores are the search's (`Engine`, early-fill rule `fill_refuses`/`is_e0`, `model.h:331,335`).
3 Real chart: claim not checked. Burnout is a real chart; no engine dump of it exists (`records-engine/` has 16 files, none Burnout, all at 85). Needed: `hydra_replay dump --efill 60` of Green Day - Burnout at depth 2 scores, 10 ms, Pro Drums, and its path list beside the typed values.
4 Direct: 271 heading, 275-276 group and notation, 278-281 fold, count, title and timing, 285, 290, 295 group names, 287, 292, 298 titles, 299 shortfall; all typed, raw results of one run. 274 `buttons[0].path == &rec.best_path()` is the owner's answer. 302-305 heading strings are the display's words. 309-324 a hand-built tied record, window-free.
5 Reason: "3- 1 2", the four scores and 163.0 have no reason except that Burnout printed them. `2,360` is `378,315 - 375,955` worked by hand. The window can move which fills the search may take, so it can move the paths, their scores and the 163.0 ms squeeze-out timing.
6 Display: the buttons are the display. Lines 278 and 298-299 type engine values the display shows; they should be read from `rec.paths` (as record PL10 says).
7 Failure: a path list with a wrong fold, label or shortfall. Possible; this is the screen's headline list.
8 Deleted: line 320 `tv.buttons[0].fold == tv.buttons[1].fold` fails if `build_path_buttons` puts a tied root and its variant in different folds; line 299 fails if the all-0 shortfall is computed against the wrong score.
9 Stand-in: `REQUIRE(v.buttons.size() == 4)` stands for "Burnout's list has these four paths".
10 Same rule: `test_activation_rows.cpp:582` types the same "3- 1 2" and 163.0 (WN4c record 1); `test_ui` Paths-tab captures were not read.
11 Real: yes for the Burnout half, no for the tied half (hand-built).
Verdict: Rewrite. Read each button's fold, notation, title, timing and shortfall from `rec.paths` and `rec.optimal_score()`; keep 302-305 and the tied-variant half; REQUIRE the count from the record. Then the claim needs the 60 ms dump named in 3.

### 2. PathsTabCache: folds reset for a new path, not for a display setting (tests/test_path_list.cpp:398)
Delta round: window, replaces records/A19.md record PL14
Read: `test_path_list.cpp:398-431`; `song_details_fixtures.h:22-33`; the old record PL14.
Does the old verdict hold at 60: Rewrite stands. The typed 3 at 406 is Burnout's best-path activation count from one run; a run at 60 may give another count.
1 Claim: A display setting rebuilds the rows but keeps what is unfolded; opening another path folds every row again; the buttons rebuild only when the generation or the depth moves.
2 Owner: `PathsTabCache` and `PathsTabUi::reset`.
3 Real chart: claim not checked on a screen. Needed: Burnout's `hydra_replay dump --efill 60`, to read its best path's activation count.
4 Direct: 406 `act_open.size() == 3` typed; 407 all 0; 412, 414 stays open; 415 builds == 3; 420-422 the other path's size and folded; 426, 428, 430 button build counts 1, 2, 3.
5 Reason: 3 is the activation count of Burnout's best path from one run; the rest are the test's own inputs.
6 Display: n/a.
7 Failure: folds lost when the user moves a setting. Possible.
8 Deleted: line 412 `CHECK(cache.ui().all_open())` fails if a display-setting change resets the folds; line 422 fails if a new path keeps the old folds.
9 Stand-in: none.
10 Same rule: `PathsTabCache` buttons counters again at `test_path_list.cpp:236-239` (record PL9).
11 Real: yes.
Verdict: Rewrite. Replace the literal 3 with `best.walk_activations().size()` and drop the repeated button counts, as the old record says. `rec.all_paths()[1]` at 420 has no REQUIRE that a second path exists.

### 3. pb files: a saved path file opens back as the same path, as the player's (tests/test_pb_files.cpp:50)
Delta round: window, replaces records/C7.md record 20
Read: `test_pb_files.cpp:1-79`; `pb_open_util.h:16`; `pb_files.cpp:27-60` (by grep, via the old record); the old record 20; `start_window` and `repin` (`src/pathbuilder/pb_coach.cpp:180-186`).
Does the old verdict hold at 60: Rewrite stands. At 60 the test also rests on two Burnout facts from one run.
1 Claim: A one-window path the player builds on Burnout, saved and opened again, comes back as the same path (ticks, text, identity), as one undoable edit.
2 Owner: `save_path_file`, `open_path_file`, `path_file_json` (`pb_files.cpp`); "same path" is `path_identity` (`model.cpp:633`).
3 Real chart: the round trip runs on Burnout's real record and graph. Window-sensitive facts: line 56 `best_acts.size() >= 2` and line 59 `start_window` at the second activation's tick returns a path. Both are Burnout facts from before the default became 60; no dump at 60 exists. Needed: Burnout `hydra_replay dump --efill 60` (activation count of the best path) and a schedule showing a fill at that tick.
4 Direct: 56 REQUIRE count, 59 REQUIRE path, 61 distinct from every record path (precondition), 65-66 the file's "paths" equals `paths_json({&mine}, ...)`, 70 no refusal, 72 and 78 same path, 74-77 undo and redo.
5 Reason: no typed numbers on assertions; 2 at line 56 is a precondition.
6 Display: n/a.
7 Failure: a saved file that does not come back as the same path.
8 Deleted: line 72 (`check_same_path`) fails if `open_path_file` loses a window tick or its text; lines 65-66 pass either way (the old record's finding).
9 Stand-in: `mine` stands for any player path; it is the second fill of Burnout's best path.
10 Same rule: `test_pathstring_read.cpp:53` round trip of the reader on the corpus.
11 Real: yes.
Verdict: Rewrite. Delete lines 65-66 (a function compared with its own body). Choose the activation by a property (the first record path with two or more windows, REQUIRE it exists) so the test does not depend on Burnout's best path having three. Open a real `hydra_replay` dump too, as the old record asks.

### 4. pb files: a path file's first path becomes the player's (tests/test_pb_files.cpp:84)
Delta round: window, replaces records/C7.md record 21
Read: `test_pb_files.cpp:84-108`; the old record 21.
Does the old verdict hold at 60: Investigate stands, and has two reasons: the rule "the first path wins" has no filed ruling, and the same two Burnout preconditions as record 3.
1 Claim: A path file of two paths opens its first as the player's, as one edit, and leaves the second out.
2 Owner: `open_path_file` (`pb_files.cpp`, the first-path read at about line 101, by the old record).
3 Real chart: Burnout, with two real paths, one built by a click. The same preconditions as record 3 at lines 88 and 93 (`best_acts.size() >= 2`; `path_identity(best) != path_identity(*one.path)`). Claim about the rule: no `## Confirmed` line in `docs/agents/rulings/` for "first path" (old record 21 searched; the plan that held the user's answer is removed, `git show 307d0b5c:docs/superpowers/plans/2026-10-30-path-builder-redesign.md`). Not re-searched this round.
4 Direct: 88, 91, 93 REQUIREs; 101 no refusal; 103 current path equals `*one.path`; 105-107 one edit.
5 Reason: none typed.
6 Display: n/a.
7 Failure: the file's last or best path opens instead of the first.
8 Deleted: line 103 fails if `open_path_file` takes the best or last path.
9 Stand-in: the one-window path stands for "a path unlike Hydra's best".
10 Same rule: `tests/pb_ui/pbtest_files.cpp` opens the same file.
11 Real: yes.
Verdict: Investigate. The rule "a path file opens its first path" has no `## Confirmed` ruling; the user decides it. The test body reads sound; its precondition on Burnout's activation count is the same as record 3's.

### 5. score box stars: the analyzed chart ends on the path's own star count (tests/test_preview_boxes.cpp:498)
Delta round: window, replaces records/T3-run2.md record 22
Read: `test_preview_boxes.cpp:498-511`; `analyzed_chart.h:16-25`; `corpus_util.h:85-135`; the old record 22.
Does the old verdict hold at 60: Rewrite stands. Lines 509-510 pin "seven stars, no next star" for "the first corpus chart with paths", a chart the test never names; its best path at 60 can have another score and another star count.
1 Claim: At the end of the analyzed chart the box's stars equal the path's star count, and "next star" is empty exactly at the maximum.
2 Owner: `path_stars` (the number the details header shows) and `score_box_at`.
3 Real chart: claim not checked. The chart is whichever `first_chart_with_paths` returns; unnamed. Needed: the name of that chart and `hydra_replay dump --efill 60` of it, to read its best path's score and stars.
4 Direct: 502 REQUIRE state Ready; 505 `stars == path_stars(best)` (the owner's answer); 506 `next_star.empty() == (stars == kMaxStars)` (a rule); 509 `stars == 7` and 510 `next_star.empty()` (snapshots, "Pinned from one run", comment 507-508).
5 Reason: 7 is one run's value for an unnamed chart.
6 Display: 505 compares the box with the engine's `path_stars`: good.
7 Failure: the box's stars disagree with the details header.
8 Deleted: line 505 fails if the box computes stars from a different total than `path_stars(best)`; line 506 fails if the "next star" line stays at the maximum.
9 Stand-in: `corpus::analyzed_by_ten_scores()` stands for "a real chart".
10 Same rule: line 547 (the solo-bonus star case, record 23 in `T3-run2.md`).
11 Real: yes.
Verdict: Rewrite. Delete lines 509-510; keep 505-506 (the old verdict).

## Not read whole

- `tests/test_preview_boxes.cpp`, by exact name, bodies not read whole (sorted B from the whole-file grep, the helpers and the old records `records/T3-run2.md`): build_time_box (115, 137, 145, 157, 179, 205, 223, 1027, 1087); step_shown_ms (247, 255, 262); score box (272, 280, 311, 332, 371, 384, 401 from line 425 on); struck_at (324); score box stars (513, 552); drain box (569, 585, 598, 613, 627, 638, 662, 677); solo_percent (691); solo box (702, 738, 753, 771, 787, 800, 808); scrub marks (856, 877); activation jumps (886); next activation box (899, 912, 919, 929). The whole-file grep above would find a search call in any of them; it finds none.
- `tests/test_path_list.cpp`: every body read, except lines 404-436 (the rest of record 2's case and `path_item_id`), read from line 398 to 410 and the case at 433 by its grep line.
- `tests/test_app_library.cpp`: bodies not read whole: lines 54-134, 150-248, 373-411, 611-654, 726-777, 810-1180 (every case there), 1221-1241. Sorted B from the file-wide grep, the fixture `app_state_fixtures.h`, and the first lines of each.
- Not read: `tests/test_pb_files.cpp` helper bodies `check_same_path`'s `act_ticks` (`replay_json.h`), `temp_util.h`.
