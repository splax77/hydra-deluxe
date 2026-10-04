# Phase 6 recheck, batch C

Parse, store, audio, winstr and strutil, the CLI, build identity, and the 17 `tools/ch_probe` folds. 55 findings. Checked against main at 9cec59c (that is c86ab6a plus the D49 record, no code change). Read-only scout; nothing in the repo was changed except this file.

## Summary

Of the 55 findings, one is fixed (269: `report_file_exists` now calls `file_exists_utf8`). Three are partly fixed (227, R7.19, 229). Three moved without being fixed (187, 188, R7.21): the code was rewritten by the ranged-reads merge or the audio loader rework, but the second copy is still there under a new name. The other 48 are open exactly as the audit describes them.

One finding needs a user call: 298, because dropping the `PRAGMA user_version` write changes a number stored in the database file. Everything else in this batch is code-only. A handful of developer-tool numbers (the probe cut-offs in 328, the probe sleeps in 273, the "hardware threads minus one" rule in R7.22, the miniaudio converter defaults in R7.28) have no decision behind them; the folds do not change them, so one D49-style "recorded as they are" line covers all of them. That is listed at the end, not as a question.

29 findings touch a file a phase 3 branch also changes (analysis.cpp, song.cpp, record_store.cpp, library_query.cpp, user_messages.cpp, report.cpp, dm_report.cpp, preview_view.cpp, preview_tab.cpp, preview_load_job.cpp, preview_transport.cpp, dynamics_breakdown.cpp, app_state.cpp, library_jobs.cpp, bench.cpp). Two of those are trivial joins (257 adds one CMake line next to O2's one line; 199 only shares `tests/test_rules.cpp`). The other 26 findings, all of src/audio, src/parse/chart_files and srb, src/core, preview_source.cpp, the installer and all of tools/ch_probe, are free of phase 3 and can start now. Phase 3 does not fix any finding in this batch; where it touches the same lines (201's helpers in library_query.cpp, 191's `notes.back().ms`, 347's `value_or`) the copies survive on the branches unchanged.

ADR 0020 and the C2 merge cover neither 269 (that was fixed earlier, by `report_file_exists` calling winstr) nor 251 nor 217. The ADR names the shell as the exception to `win32_path` and names `fits_shell` and `shell_path`; it says nothing about a parent-folder helper or the `> 32` success test. C2 left both `> 32` lines in place (`report_files.cpp:69`, `win32_dialogs.cpp:77`).

## Status table

Status is at main 9cec59c. "Phase 3" says which task's branch changes a file the fold needs, or "none".

| id | status | phase 3 overlap | evidence |
|---|---|---|---|
| 257 | open | O2 adds one line to CMakeLists.txt | `version.h:24-25` type "Hydra Deluxe" and "Hydra.Hydra"; `hydra.iss` types them at 34, 36, 68, 73, 93; CMake passes only HYDRA_VERSION (`CMakeLists.txt:191`) |
| 201 | open | O3a, K1a touch library_query.cpp (helpers kept); C3b, O3b touch user_messages.cpp (helpers kept); K1a owns report.cpp; C3b, O3a touch song.cpp | `library_query.cpp:31-57` is_ascii_space, ascii_lower, iequals_ascii, starts_with_ci; `user_messages.cpp:62-65` starts_with, ends_with; `report.cpp:196` and `song.cpp:286-287` std::tolower. New since the audit: C2 added a wide `starts_with` in `winstr.cpp:57` |
| 182 | open | K4b, C3b touch preview_load_job.cpp | `player.cpp:29, 33, 37` and `preview_load_job.cpp:199` each write the ms/frames formula |
| R7.21 | moved (as the audit says) | K4b | the pad formula is in `PreviewLoadJob::run`, `preview_load_job.cpp:199` |
| 228 | open | none | `constants.py:146` 0.085 and `:150` 85.0, neither derived; `:179` builds CAP_FROM_GAP_MS from the ms copy |
| 229 | partly | none | active_probe, passive_probe, walk_edges, watch_window call `all_patterns`; play_chart passes only `normal_pattern` (`:332`); find_clock3 `:34`, find_engine `:65`, hit_detect `:46`, poll_windows `:84` still type 0x4000000 |
| 230 | open | none | `live.py:49-52` struct.unpack_from; `live.py:43` and `find_engine.py:95-96` repeat the precision bit test (find_engine reads raw 0x198); `debugger.py:58`, `passive_probe.py:97`, `find_clock3.py:106-173`, `find_engine.py:46-47` decode by hand; `watch_window.py:261-262` reads fields by raw offset |
| 231 | open | none | `process.py:351, 377` and `debugger.py:572, 583` both bind Read/WriteProcessMemory |
| 232 | open | none | SETTLE_MS = 250 in `walk_edges.py:55` and `active_probe.py:66`; the "score rose" check is inline in three runners |
| 233 | open | none | `probe_songs.py:38` KICK = 0 beside `constants.py:206`; `probe_songs.py:135` truncates BPM, `probe_chart.py:87` rounds it |
| 234 | open | none | `probe_songs.py:40` DEFAULT_OUT and `live.py:30` PROBE_ROOT are the same literal; "Window Map"/"Edge Walk" at `probe_songs.py:215`, `watch_window.py:242`, `walk_edges.py:184`; "Hydra Probe - " at `probe_songs.py:200` and `active_probe.py:101` |
| 235 | open | none | `analysis.py:235` and `:259` both write `(t * c1 - (t ** exponent) * c2) * c3` |
| 236 | open | none | FindWindowW(None, "Clone Hero") at `active_probe.py:191`, `pad_flash_test.py:54`, `play_chart.py:342`, `walk_edges.py:209` |
| 237 | open | none | `milestone1.py:36-52` reads constants one by one; `EngineModel.constants` at `engine.py:148-174` |
| 238 | open | none | `test_probe_chart.py:35` parse_drum_ticks, `test_probe_songs.py:18` chart_ticks, `play_chart.py:89` parse_chart |
| 239 | open | none | `probe_chart.py:48` ms_to_ticks, no inverse; `probe_songs.py:36` comment "1 tick per ms" is the identity the others lean on |
| 271 | open | none | `engine.py:166` bare "hitcheck_threshold"; `process.py:194-196` back_ms/front_ms hold seconds |
| 272 | open | none | `resolve` fakes in `test_engine.py:62`, `test_engine_finder.py:38`, `test_runners.py:42`; owner `process.py:145` |
| 273 | open | none | hold 0.003 in `input_driver.py:163` and `interfaces.py:231`, pinned in `test_input_driver.py:179`; sleeps 0.12 and 0.4 at `engine_finder.py:171, 183` |
| 328 | open, no decision | none | `watch_window.py:224` 10.0; `walk_edges.py:95` 0.002 and 0.05; `analysis.py:157` 1.0; `engine_finder.py:165` 0.001. D49 covers the worktree and precheck scripts, not these |
| 342 | open | none | `engine_finder.py:68` backs up by OFF_BACK_WINDOW only; OFF_FRONT_WINDOW is never read there |
| 187 | moved | none | the suffix tests now sit in `is_container_path` (`preview_source.cpp:351`) and two `.sng` checks (`:374`, `:389`); R7.17 describes the current shape |
| R7.17 | open | none | as above; `chart_format_of` at `chart_files.cpp:9-14` is the owner |
| 192 | open | K1a owns report.cpp, K1b owns dm_report.cpp | `analysis.cpp:123` writes lowercase hex; re-lowered at `report.cpp:246`, `dm_report.cpp:173`, `dmbot_client.cpp:264` |
| 113 | open | O3a, K1a touch library_query.cpp; K1a, K3 touch library_jobs.cpp | `record_store.cpp:1684, 1701` bare LIKE; `library_jobs.cpp:137` the search-string BatchJob constructor still exists |
| 262 | open | K5 owns dynamics_breakdown.cpp; K3, C4b touch app_state.cpp | `dynamics_breakdown.cpp:169` path key, `:173` md5 key; `app_state.cpp:366, 386, 411`; `dynamics_load_job.cpp:11` |
| R7.26 | open | K2 touches record_store.cpp (6 lines) | "engine_mode" at `record_store.cpp:644, 705, 710`; the migration reads the key itself |
| 225 | open | K4b touches preview_transport.cpp/.h | `player.h:57` and `preview_transport.cpp:89` both clamp below 0 |
| R7.23 | open | none | `ma_reader.cpp:512-524` flac_behind_id3 parses one tag; `:529` sniffs a second time |
| R7.19 | partly | C3b, O3a touch song.cpp | `base_ = 96` is gone (`song.cpp:775` sets it from difficulty_base_pitch); the list at `:511-518`, the gate at `:623` and the two switches at `:646-674` remain |
| R7.24 | open | none | `ma_reader.cpp:274` kMaxReservoir = 511 beside the macro at `:52`; the formula at `:337` and `:433` |
| 186 | open | C3b, O1 touch analysis.cpp; C3b, O3a touch song.cpp | `analysis.cpp:346` found_mid ? found_mid : found_chart; `song.cpp:1395-1412` breaks on the first .mid, keeps the last .chart |
| 227 | partly | none | owner moved to `opus_reader.cpp`: kRate = 48000 is named once (`:50`) and used three times; kMaxFrame = 5760 (`:51`) and kPreRoll = 19200 (`:52`) are still worked out by hand |
| 251 | open | C3b, O1 touch analysis.cpp | `analysis.cpp:42` parent_of, `preview_source.cpp:36` dir_name, `config.cpp:16` exe_dir |
| R7.25 | open | K4b touches preview_load_job.cpp | `preview_load_job.cpp:96` file_size_bytes, then `:116` MappedFile::open sizes it again (`mapped_file.cpp:19`) |
| 347 | open | K4a changes the same lines (the else branch becomes app.settings.sp_cap; value_or stays) | `preview_tab.cpp` `record->sp_cap.value_or(kCloneHeroSpCap)` at main and on claude/p3-k4a |
| R7.11 | open | none | `midi.cpp:178-180` std::fseek/ftell; three known-copy lines at `test_single_owner.cpp:1087-1091` |
| 197 | open | K2 | `record_store.cpp:843-846` compares hyhash, chartmode, hyversion, sp_cap only |
| 199 | open | phase 3 adds 44 lines to tests/test_rules.cpp; rules.cpp and rules_file.cpp untouched | `rules.cpp:31-35` and `rules_file.cpp:41-61` each type the names and "first_note"/"whole_chord" |
| 256 | open | K1a, K3 touch library_jobs.cpp; C3a owns bench.cpp; phase 3 touches test_analysis.cpp | positional brace copies at `library_jobs.cpp:77-78`, `bench.cpp:137`, `test_analysis.cpp` (rescan cache case) |
| 204 | open | O3a, K1a touch library_query.cpp | `library_query.cpp:281-287` term_matches switch; `:406` match_spans' own field test |
| 217 | open (C2 did not touch it) | none | `report_files.cpp:69` and `win32_dialogs.cpp:77` both `> 32` |
| 269 | fixed | none | `report_files.cpp:134` calls file_exists_utf8; the `is_dir` helper in analysis.cpp is gone (`analysis.cpp:335` reads DirEntry::is_dir) |
| 189 | open | C3b, O1 touch analysis.cpp | `analysis.cpp:338` last match wins; `preview_source.cpp:122-124` first match wins |
| 191 | open | C4a, K4a, O3b touch preview_view.cpp; K2 touches record_store.cpp | `preview_view.cpp:279` notes.back().ms, unchanged on claude/p3-d1; owner `record_store.cpp:463` |
| 188 | moved | C3b, O1 touch analysis.cpp; song.cpp too | ranged reads reshaped it: `song.cpp:1428-1434` walks header then notes over a ByteSource; `preview_source.cpp:229-238` walks all three streams over a whole buffer; `analysis.cpp:194-203` composes "inflate at the header, parse metadata" again |
| 196 | open | K2 | `record_store.cpp:308` rank_row and `:323` stale_reasons are separate; both called at `:1188` and `:1203` |
| R7.28 | open | none | `stream_mix.cpp:196-201` and `mixer.cpp:26` each write the passthrough test and the converter config |
| 194 | open | K2 | `record_store.cpp:264` kStructureHeadBytes; `:280, :289` read_le at 0 and 4; substr(structure,…) at `:353-354, 1057, 1065-1066, 1112, 1491` |
| 195 | open | K2; K5 owns dynamics_breakdown.cpp | `record_store.cpp:266-271` read_le/write_le; `dynamics_breakdown.cpp:118-125` write_u32_le/read_u32_le; `serialize.cpp:9-58` is the declared owner |
| 298 | open, needs a user call | K2 | `record_store.cpp:637` PRAGMA user_version = 3; nothing in src reads it |
| 252 | open | K2 | `record_store.cpp:417-421` and `:443-446` both say sp_cap=?; both keep an unused CapQuery |
| 253 | open | K2 | `record_store.cpp:974` eleven plus ten typed placeholders; `:1426` reindex; `:1583` list_records reads slots 17 and 19 |
| 299 | open | K2 | `record_store.cpp:614` has_column + ALTER; `:1665` pragma_table_info probe |
| R7.22 | open | C3b, O1 touch analysis.cpp; K1a, K3 touch library_jobs.cpp | floor at `analysis.cpp:491`, `work_pool.h:52`, `library_jobs.cpp:155` |

## The folds, group by group

### Audio frames, gain, Opus rate (182, R7.21, 225, 227)

The question behind 182 and R7.21 is "how many audio frames is a span of milliseconds, and back". Four copies: three in `Playhead` and one in the Preview load job's front pad. The owner should be a small pair in `src/audio` (`frames_of_ms`, `ms_of_frames`) in a header the UI can include, since one caller now lives in `src/ui`. The Preview audio sits at the same frame either way, so nothing changes on screen. Tests: "seek clamps to the valid range in both frames and ms" and "read_frames while playing copies frames and advances the clock" in test_audio_player.cpp, "a Preview load turns a negative chart offset into front silence" in test_preview_load_progress.cpp, and "StreamMix's front pad adds silence before the first sample" in test_preview_transport.cpp. One new case pins the pair at 48000 and 44100 from a run. A scan row matches the `* sample_rate / 1000` and `* kOutRate / 1000` spellings. Size S for the owner and the three player callers; the load-job caller waits for K4b.

225 asks "what is the lowest legal output gain". `Playhead::set_gain` owns it; the transport should stop clamping and either store the raw value or read the clamped value back. "gain set before load applies to the next playhead" and "Playhead applies the output gain to served frames" cover it. Scan row: `gain < 0.0f ? 0.0f`. Size S, but the transport half waits for K4b.

227 is now mostly done. The Opus decoder moved to `opus_reader.cpp` and names `kRate` once. What is left is deriving `kMaxFrame` (120 ms) and `kPreRoll` (400 ms) from `kRate` instead of typing 5760 and 19200. "decode_audio: Ogg-Opus output is pinned bit for bit" and the Opus cases in test_stem_reader.cpp pin the output, so a wrong derivation fails loudly. Scan row: `\b(5760|19200)\b` in src/audio, with `preview_load_job.h`'s 48000 (the mixer's output rate) and `ma_reader.cpp:259` (the MP3 rate table) exempt as different facts. Size S.

### Stem format sniffing, the MP3 reservoir, the converter config (R7.23, R7.24, R7.28)

R7.23 asks "which format are these stem bytes". `sniff_format` says any ID3 tag is MP3; `open_ma_reader` then sniffs again and overrides with `flac_behind_id3`, which parses one tag. The fold moves the ID3-skip into `sniff_format` so it returns Flac for a tagged FLAC, and `open_ma_reader` stops sniffing. Audio output does not change: the stem already ended up in the FLAC decoder, only by a longer route. One new case feeds a tagged FLAC fixture to `sniff_format`. "sniff_format classifies audio containers by their magic bytes" is the existing test; "StemReader: straight-through read equals the full decode" covers the readers. The audit's two-stacked-tags gap stays as it is unless the owner loops over tags as dr_flac does; that is the implementer's call and changes no shipped audio. Scan row: `sniff_format(` outside decode.cpp and stem_reader.cpp. Size S.

R7.24 is one file. A `reservoir_after` helper owns the min-of-mins formula, and `kMaxReservoir` is defined from the miniaudio macro instead of retyping 511. "StemReader: MP3 seeks through the seek points on a long stream" and "an MP3 seek anywhere lands on the straight decode" pin the seek landings. Scan row: `std::min(kMaxReservoir` and a bare 511 in ma_reader.cpp outside the macro line. Size S.

R7.28 asks "how is a stem converted to the output format". `StreamMix` and `convert_stem` each write the passthrough test and the converter config. One `stem_converter_config(in_rate, in_channels, out_rate, out_channels)` in `src/audio` (returning the config and whether it is a passthrough) serves both. "StreamMix: reading start to end equals mix_stems of every decoded stem" is the test that would catch a split. The miniaudio defaults it bakes in (linear resampler, filter order 1, no dither) have no decision; the fold keeps them as they are. Scan row: `ma_data_converter_config_init(`. Size S.

### Preview sizes a stem twice (R7.25)

"How many compressed bytes does a loose stem have" is answered by `file_size_bytes` in `open_audio` and again by `MappedFile::open`. The fold maps first and takes the total from the mapped size, dropping the `file_size_bytes` call and the clamps that reconcile the two. The "Opening audio: X of Y MB" bar shows the same Y unless the file changes while it opens. Tests: "Preview load progress: labels name the step, in real units", "the bar never moves backwards", and "a Preview load cancelled while opening a 300 MB Opus stem stops promptly". Scan row: the existing "How many bytes does a file hold?" row already lists `const uint64_t n = hydra::file_size_bytes(path);` as a calls-owner line; after the fold that spelling leaves preview_load_job.cpp. Size S, but it waits for K4b (same file).

### Chart format, notes file, song.ini, .srb layout (186, 187, R7.17, 188, 189)

These four share one home, `src/parse/chart_files.cpp` and `srb.h`, and two callers, the scan in `analysis.cpp` and the Preview in `preview_source.cpp`.

187 and R7.17 ask "which chart format is this path". `chart_format_of` owns it. `is_container_path` and the two `.sng` tests in `preview_source.cpp` should call it once per load and switch on the answer. Scan row: `ends_with_ci(…, ".sng"|".srb"|".mid"|".chart")` outside chart_files.cpp. Tests: "chart_files: a path's format comes from its extension in any case" in test_song.cpp, "resolve_preview_source: a .srb uses its chart's Offset" and "container charts give the same Song, stems and offset as the chart inside" in test_preview_source.cpp. Size S, no phase 3 file.

186 asks "which notes file wins when a song has both". A `pick_notes_file` helper in chart_files.cpp takes a list of names and returns the chosen one (first .mid, else last .chart). `discover_charts` and `load_container_sng` call it. Both pick .mid today, so nothing changes. One new case pins the choice on a three-name list. Scan row: `found_mid ? found_mid : found_chart` and the `== ChartFormat::Mid` preference spelling. The owner can land now; both callers wait for phase 3 (analysis.cpp, song.cpp).

189 asks "which file is the folder's song.ini". The scan keeps the last match, the Preview the first. On a Windows folder there is only one, so either rule is fine, but one picker should own it. The simplest owner is a `find_song_ini(folder)` beside `is_song_ini` in chart_files.cpp that both call. Scan row: `is_song_ini(` outside chart_files.cpp. Tests: "discover_charts finds a folder whose notes and ini names are capitalized" and "resolve_preview_source reads delay from a song.ini in any case". Owner now; the scan caller waits for phase 3.

188 asks "where are an .srb's streams". The ranged-reads merge split the walk three ways: the loader walks header then notes over a ByteSource, the Preview walks all three streams over a whole buffer, and the scan composes the metadata read again. One function in srb.h over a ByteSource ("read the metadata and return it with the next stream's offset") lets all three ask once; the Preview's audio walk continues from the returned offset. Tests: "srb: the note loader reads the metadata and notes streams, not the rest", "srb: a stream inflates the same from ranged reads as from the whole buffer", "extract_srb_audio: trailing audio streams inflate; art is skipped", "srb: discovery surfaces the embedded metadata". Scan row: `srb_inflate_stream(_reading)?(` outside srb.cpp and the new owner. Size M. The srb.h owner and the Preview caller can land now; the loader and scan callers wait for phase 3.

### MIDI pitches (R7.19)

"Which MIDI pitches does the drum parser act on." One pitch table in song.cpp that `is_handled_note`, both `optype` switches and the note-off gate read, with the gate's "every note-off case is 103 or higher" derived from the table rather than typed. The `base_ = 96` copy is already gone. No parse changes: ".mid: each difficulty reads its own pitch base", "mid: a stray SP note-off flags nothing", ".mid: the note on the solo marker's note-off tick is outside the solo" and the corpus invariants pin it. Scan row: `case (95|103|109|110|111|112|116|120):` in song.cpp outside the table. Size M. Waits for phase 3 (song.cpp).

### Chart hash spelling (192)

"How is a chart hash spelled for matching." `stream_md5` writes lowercase hex and is the only producer. A `normalize_chart_hash` beside it is called at the DMBot boundary only; the two internal re-lowerings in report.cpp and dm_report.cpp go. "records_by_hash keys every listed record by its lower-case hash" and "collect_dm_rows tells not analyzed from not in library" cover it. Scan row: `to_lower_ascii(` applied to a name containing md5, hash, hyhash or identifier, outside the owner. Size S, but report.cpp and dm_report.cpp are K1a and K1b files, so it waits.

A note for another batch: the hex digit table itself is typed twice, `analysis.cpp:123` and `path_codec.cpp:373`. That is "how is a byte written as hex", not in this batch.

### ASCII text helpers (201)

"How is text compared ignoring ASCII case, and which bytes are whitespace." strutil owns it and needs to export its character tests plus `starts_with`, a case-blind equals and `starts_with_ci`. Then library_query.cpp's four private helpers, user_messages.cpp's two, and the two `std::tolower` loops in report.cpp and song.cpp call strutil. C2 added a wide-string `starts_with` in winstr.cpp; that is a `std::wstring` twin and can stay or move beside the new narrow one. Nothing visible changes (no locale is ever set). Tests: the four strutil cases, "library query: artist: charter: folder: and title: limit a term to one field", "library query: squeeze<=N keeps analyzed rows…", "user_messages: the no-notes message is already plain and passes through", and ".chart: each difficulty reads its own section" (difficulty names). New strutil cases pin `starts_with_ci` and the equals on a mixed-case pair. Scan row: `std::tolower(` anywhere, and `^bool (starts_with|ends_with|iequals_ascii|starts_with_ci)\(` outside strutil. Owner now (S); the four caller files all wait for phase 3.

### Search term applies, dead SQL search (204, 113)

204: one `term_applies_to(term_field, column)` in library_query.cpp, read by `term_matches` and `match_spans`. Scan row restricted to those two function bodies: `QueryField::Any`. Tests: the field-limiting case and "library query: highlight spans cover the displayed bytes". Size S.

113: the SQL `LIKE` search in `chart_library_count` and `list_chart_library` is dead; the fold deletes the search parameter and the search-string `BatchJob` constructor so `query_matches` is the only answer. Scan row: `LIKE ?` in src. Tests: "set_search narrows the library and the match count", the library_jobs cases, and the store maintenance case. Size S. Both wait for phase 3 (library_query.cpp, record_store.cpp, library_jobs.cpp).

### Parent folder, shell success, read a whole file (251, 217, R7.11)

251: one `parent_folder(path)` in winstr (or `std::filesystem::path::parent_path` through `os_path`) replaces `parent_of`, `dir_name` and `exe_dir`'s cut. Every real caller passes a path with a separator, so the answers match. New case pins the four audit rows (trailing slash, bare name, empty). Scan row: the `find_last_of` slash spelling the three helpers use. Owner and the preview_source and config callers now; the analysis.cpp caller waits.

217: `shell_execute_ok(result)` in winstr, called by `open_in_browser` and `show_in_folder`. Scan row: `reinterpret_cast<INT_PTR>(…) > 32`. The existing "Which code launches the Windows shell?" row already lists both files as owners, so the new row is about the success test, not the call. Tests: "a long report page is copied to a short temp path for the browser", "shell_path gives the shell a short name for a long path". Size S, no phase 3 file (C2 is merged).

R7.11: `MidiFile::from_file` becomes `MidiFile(read_file_bytes(path))`, and the three `midi.cpp` lines leave the known-copies list at `test_single_owner.cpp:1087-1091`. The error wording changes from "cannot open MIDI file:" to winstr's "cannot open file:", which `plain_error_text` already maps to the same sentence; "user_messages: a missing or unreadable song file" pins that. Tests: "midi: every corpus .mid reads with a sane structure", "midi: non-MIDI input is rejected". Size S.

### Store: blob header, byte codec, Stale reasons, row identity (194, 195, 196, 197)

194 asks "where do the format and fingerprint sit in a structure blob". `path_codec` writes them and should export the offsets and a read-header helper; record_store builds its C++ reads and its SQL `substr` text from those constants. Nothing stored changes; the layout is the same, only the name of it moves. Tests: "a row in an older path format is Stale even when this build stamped it", "a row analyzed under other rules reads Stale until the rules match again", "the first open deletes the results Auto saved, and their paths, once", "a database from before the stars column gets its stars filled on open", and "path codec: a rebuilt record flattens to the same bytes". Scan row: `substr(structure,` with a digit, and `read_le(structure_head, 0|4`.

195 asks "how is a little-endian number written byte by byte". serialize's BinaryWriter/BinaryReader (or one exported pair) replaces record_store's `read_le`/`write_le` and dynamics_breakdown's `write_u32_le`/`read_u32_le`. "dynamics encode/decode round-trip" and "dynamics decode rejects bad blobs" pin the dynamics blob; the store cases above pin the header. Scan row: `(read|write)_(u32_)?le(` outside serialize. The dynamics half is K5's file and waits.

196: `rank_row` returns the reasons with the verdict so `stale_reasons` goes. Only `hydra_replay dump` prints the reasons (K2 and O3b also plan a `stale_text` for the song panel from the same reasons). Test: "a Stale lookup says why: another build, other rules, or both". Scan row: `layout_is_current(` outside rank_row.

197: `reload_row` selects the five lens columns too and compares a `RecordKey`. Tests: "a rewritten row that lands on its own id is read fresh, not mixed up", "RecordKey compares on every part of the identity". Scan row: `!= meta.(hyhash|chartmode|hyversion|sp_cap)`.

All four live in record_store.cpp, which K2 touches (6 lines), so they wait for phase 3. Together M.

### Store SQL owners and schema (252, 253, 298, 299, R7.26)

252: one cap-match helper and binder; the two `[[maybe_unused]] CapQuery` parameters go. `write_row`'s purge at `:958` asks a different question (the row's own unique key) and stays, listed as an exempt line. Tests: "records at different caps coexist; each lookup sees only its own cap", "has_record and a lookup agree on which rows are readable", "analyzed_hashes names exactly the charts has_record would skip". Scan row: `sp_cap=?` outside the helper.

253: `kSummaryColumnList` gets a column-count constant; the INSERT placeholders, `reindex`'s UPDATE list and `list_records`' slots are built from it. Tests: "RecordStore maintenance: has_record, list_records, reindex", "get_summaries answers a page the same as get_summary row by row", "the listing and a lookup agree on which row is a chart's answer". Scan row: the typed placeholder string and `column_(text|blob|int)(s, 1[6-9])`.

299: the `pragma_table_info` probe in `chart_library_cache` is dead; delete it. Test: "a charts table from before the sig column still rebuilds". Scan row: `pragma_table_info(`.

R7.26: a named constant for "engine_mode" and the migration calls the getter. Tests: "a schema 2 database keeps its results, filed under Clone Hero 1.1", "hydra_batch stamps a new database with the rule it ran under". Scan row: `"engine_mode"` outside the constant line.

298 is the one user call in this batch; see below. Whichever answer, the change is a few lines in record_store.cpp and two checks in test_store.cpp.

All in record_store.cpp, waiting for K2. Together with the group above this is one L task.

### Song length (191)

"How long is a song" is `store::song_length_ms`, the last note's time. `build_preview_scene` should call it instead of `scene.notes.back().ms`. Same number for every chart (the parser only makes a timestamp for a chord with notes). Tests: "build_preview_scene: an analyzed chart's overlay matches its path", "a stored song keeps its length, and an old songmeta row reads none"; the test_store check that retypes the formula should compare against the owner instead. Scan row: `notes.back().ms`. Size S; waits for phase 3 (preview_view.cpp).

### Preview gauge cap fallback (347)

The store refuses to save a Ready record without a cap (`record_store.h:151, 217` hold a plain int), yet the Preview reads `record->sp_cap.value_or(kCloneHeroSpCap)`. The fold reads the cap directly for a Ready record and drops the default argument on `build_preview_scene`, so a capless record is an error rather than a silent 4 bars. Nothing shows differently today because no such record exists. K4a edits the same three lines (its else branch becomes the settings cap), so this waits and then changes only the Ready branch. Tests: "sp meter curve: the bank stops at the cap", "sp meter readout: bars banked over the cap", and the uitest preview script. Scan row: `sp_cap.value_or(`. Size S. Making `HydraRecord::sp_cap` non-optional is the thorough form; it touches path_codec and is best left to the store task if the user wants it.

### Dynamics key (262)

`dynamics_store_key` (md5) owns the identity; the in-memory cache compares that key instead of the path key, and `dynamics_cache_key` goes. "update_dynamics uses a stored row with the current count stamp", "close_details keeps a Dynamics count that finished on another tab", "dynamics keys come from one place". Scan row: `dynamics_cache_key(`. Size S; waits for K5 and K3.

### Scan row conversion, worker floor (256, R7.22)

256: one `to_library_entry(const ScanItem&)` in analysis.h replaces the three positional brace copies (library_jobs.cpp:77, bench.cpp:137, the rescan-cache test). "rescan cache reproduces the scan without reading chart files", "discover_charts output matches the checked-in scan snapshot". Scan row: `ChartLibraryEntry` followed by `{item.md5` or `{it.md5`. Size S.

R7.22: `batch_worker_count` owns the count; `run_work_pool` and `set_analyzer_for_test` trust what they are given and drop their `std::max(1, …)`. "run_work_pool hands every item to the consumer once", "run_work_pool: a cancel mid-run never strands the consumer". Scan row: `std::max(1,` next to a worker word. The "hardware threads minus one, guess 1 when Windows says 0, same count for hashing" parts have no decision; the fold keeps them. Size S. Both wait (analysis.cpp, library_jobs.cpp, bench.cpp are phase 3 files).

### Rules field table (199)

"Which rules fields exist and how is each spelled." One table in core/rules (name, member, reader, writer) that `fixed_cap_text` and `load_rules_file` both walk. The fingerprint text must stay byte-for-byte, or every stored result turns Stale: the task adds one case that pins the current `Rules{}.fingerprint()` value as a literal taken from a run before the change, next to "rules: the default stamp is built once and matches a fresh record" and "rules: every key in the file is read". Scan row: `key == "` in rules_file.cpp outside the table. Size S. Only `tests/test_rules.cpp` overlaps phase 3 (44 added lines on the D1 branches), so it can start now if the integrator joins that file, or wait.

### Build identity (257)

CMakeLists.txt owns "Hydra Deluxe" and "Hydra.Hydra" next to the version; it passes them to `version.h` as defines and `build_installer.ps1` passes them to Inno with `/D`, as it already does for the version. The installer's five "Hydra Deluxe" lines become `{#HYDRA_APP_NAME}`. Nothing a player sees changes. There is no test for the installer; done means the two strings appear once in CMakeLists.txt and a scan row (scope: src plus `installer/hydra.iss`) matches `Hydra\.Hydra|"Hydra Deluxe"`. O2 adds one unrelated line to CMakeLists.txt, a trivial join. Size S.

### The Clone Hero probe (228 to 239, 271 to 273, 328, 342)

Seventeen findings, all in `tools/ch_probe`, none touched by phase 3, none visible anywhere in Hydra. The tests run with `python -m pytest tools/ch_probe/tests -q` (README line 70); 229 tests in 13 files. `-k` picks files.

The engine and memory layer (228, 229, 230, 231, 237, 271, 272, 342, and the engine_finder half of 273) folds into `process.py`, `engine.py`, `engine_finder.py` and `constants.py`. Concretely: the ms back window is derived from the seconds value; `engine_finder` derives the 8-byte gap from the two offsets and the four old scripts call `normal_pattern`/`all_patterns` and `MODULE_SPAN`; `EngineModel` gains a one-read snapshot and a pure `is_precision(flags)` so `live.py`, `find_engine.py`, `watch_window.py` and `walk_edges.py` stop decoding fields themselves, and one `s_to_ms` replaces the scattered x1000; the debugger reuses `process.py`'s reader and writer and keeps only its cache flush; `milestone1` prints `EngineModel.constants()`; every constants key is a `CONST_KEY_*`; `verify_targets` names its seconds as seconds; the three test fakes build a real `Process` with fake callables as `test_process.py` already does; the 0.12 s and 0.4 s sleeps get names. Tests: `-k "engine or process or analysis or debugger"`. New cases pin `EXPECT_NORMAL_BACK_MS == 85.0` and the gap `OFF_FRONT_WINDOW - OFF_BACK_WINDOW == 8` from the constants (a guard, not a recompute). Size M.

The probe songs and runners (232, 233, 234, 235, 236, 238, 239, the rest of 273, 328) fold into `probe_chart.py`, `probe_songs.py`, `input_driver.py`, `analysis.py` and the runners. `probe_songs.chart_text` calls probe_chart's section builders and the kick constant, with one resolution/BPM pair; `probe_songs` owns the root, sub-folder names, name prefix and folder writer, and `live.PROBE_ROOT` and `active_probe.write_probe_song` go; one `pressed_input_hit` helper beside `EngineModel.score` with one settle constant; one window helper in `input_driver` seeded from `find_game_window`; the two predictors share one inner-term helper; one Python `.chart` note reader for the two test helpers and `play_chart`; `ticks_to_ms` beside `ms_to_ticks` and the identity callers use it; the 3 ms hold is one constant the protocol stub references; each cut-off in 328 becomes a named, commented constant with its value unchanged. Tests: `-k "probe_chart or probe_songs or play_chart or hit_window or input_driver or runners"`. Size M. This task edits `constants.py` too, so it runs after the engine task or the two share one branch.

The single-owner scan covers `tools/` by default, so rows can guard the Python copies if `tests/source_tree.h` walks `.py` files; I did not confirm that (see the unchecked list). If it does not, a small pytest that greps `tools/ch_probe` for `0x4000000`, `FindWindowW(None, "Clone Hero")`, `SETTLE_MS =` and `struct.unpack` outside `process.py` does the same job.

## Questions for the user

**298, the database's schema number.** Today every open writes `PRAGMA user_version = 3` into the hydra.db file. Nothing in Hydra reads it back; the upgrades decide what to do by asking which columns exist. Two tests check the number. Option A drops the write and the header's "schema user_version 3" wording: new databases carry 0 in that slot, existing files keep their 3, and nothing on any screen changes. Option B makes the number the real gate, moved into `stored_versions.h`, and the column probes go. Recommendation: A. The column probes already own the question and have run every upgrade so far; making a number the gate would skip probes on a file whose number lies, and nothing outside Hydra reads the slot.

No other finding in this batch changes a display, a report page, hydra_batch's text, or a stored field, stamp, score or path. The 199 fold keeps the fingerprint text byte-for-byte and pins it; the 194 and 195 folds keep the blob bytes and are pinned by the round-trip tests.

## Numbers with no decision (not questions; one record line covers them)

The folds name these but do not change them. A D49-style line, "recorded as they are, developer tools and pool sizing only", would let the precheck pass them: the probe cut-offs in 328 (10 ms hit-time gap, 2 ms fresh clock, 50 ms fill cap, 1.0 ms tolerance, 80 % majority, 0.001 window floor, 0.000001 clock step), the probe pacing in 273 (0.12 s and 0.4 s sleeps, 3 ms key hold, 80 to 92 ms walk), the 250 ms settle in 232, R7.22's "hardware threads minus one, guess 1 when Windows reports 0, same count for the hashing pool", and R7.28's miniaudio converter defaults (linear resampler, low-pass order 1, no dither). None touches a score, a display or a record.

## Proposed tasks

No two tasks own a file. Where a fold's owner and callers sit on both sides of phase 3, the owner lands in a now-task and the callers in a later one, as marked. Every task appends its scan rows to `tests/test_single_owner.cpp`; as in phase 3, the integrator joins that file in the merge commit. Each task runs only the filters listed.

| id | title | findings | owns | tests | done when | waits for phase 3? |
|---|---|---|---|---|---|---|
| C1 | Probe engine and memory layer | 228, 229, 230, 231, 237, 271, 272, 342, 273 (engine_finder half) | tools/ch_probe: constants.py, process.py, engine.py, engine_finder.py, debugger.py, experiments/live.py, milestone1.py, find_clock3.py, find_engine.py, hit_detect.py, poll_windows.py, passive_probe.py; tests test_engine.py, test_engine_finder.py, test_process.py, test_analysis.py, test_debugger.py | `python -m pytest tools/ch_probe/tests -q -k "engine or process or analysis or debugger"` | 0x4000000, struct.unpack and the precision test appear only in their owners; the ms window and the 8-byte gap are derived; the debugger has no Read/WriteProcessMemory binding | no (M) |
| C2 | Probe songs and runners | 232, 233, 234, 235, 236, 238, 239, 273 (rest), 328 | tools/ch_probe: probe_chart.py, probe_songs.py, input_driver.py, interfaces.py, experiments/analysis.py, active_probe.py, walk_edges.py, watch_window.py, play_chart.py, pad_flash_test.py; tests test_probe_chart.py, test_probe_songs.py, test_play_chart.py, test_hit_window_scripts.py, test_input_driver.py, test_runners.py; constants.py for the new named cut-offs (after C1) | `-k "probe_chart or probe_songs or play_chart or hit_window or input_driver or runners"` | one chart writer, one song-folder writer, one window helper, one hit check, one settle constant, one Python note reader; every cut-off is a named constant with its old value | no, but after C1 (M) |
| C3 | Audio owners | 182 and R7.21 (owner plus the three Playhead callers), 225 (owner), 227, R7.23, R7.24, R7.28 | src/audio/* (player.h/.cpp, opus_reader.cpp, decode.h/.cpp, stem_reader.cpp, ma_reader.cpp, stream_mix.h/.cpp, mixer.h/.cpp), tests test_audio_player.cpp, test_audio_decode.cpp, test_stem_reader.cpp, test_stream_mix.cpp, test_audio_mixer.cpp | `-sf=*test_audio_player*,*test_audio_decode*,*test_stem_reader*,*test_stream_mix*,*test_audio_mixer*` | frames_of_ms/ms_of_frames exist and the player calls them; kMaxFrame and kPreRoll derive from kRate; sniff_format says Flac for a tagged FLAC and open_ma_reader does not sniff; one reservoir_after; one converter config; a scan row for each | no (M) |
| C4 | Chart files, containers, path helpers | 187, R7.17, 186 (owner), 189 (owner plus Preview), 188 (owner plus Preview), 251 (owner plus preview_source and config), 217, R7.11 | src/parse/chart_files.h/.cpp, srb.h/.cpp, midi.cpp, src/app/preview_source.h/.cpp, src/app/config.cpp, src/core/winstr.h/.cpp, src/app/report_files.cpp, src/ui/win32_dialogs.cpp; tests test_srb.cpp, test_sng.cpp, test_preview_source.cpp, test_midi.cpp, test_winstr.cpp, test_long_paths.cpp, the chart_files cases in test_song.cpp | `-sf=*test_srb*,*test_sng*,*test_preview_source*,*test_midi*,*test_winstr*,*test_long_paths*`, `-tc="chart_files*"` | the Preview asks chart_format_of once per load; pick_notes_file, find_song_ini, the srb layout reader, parent_folder and shell_execute_ok exist with pinned cases; from_file calls read_file_bytes and the three midi.cpp known copies are gone | no (M to L) |
| C5 | strutil exports | 201 (owner) | src/core/strutil.h/.cpp, tests/test_strutil.cpp | `-sf=*test_strutil*` | starts_with, a case-blind equals, starts_with_ci and the character tests are exported and pinned | no (S) |
| C6 | Build identity | 257 | CMakeLists.txt (two defines), src/core/version.h, installer/hydra.iss, installer/build_installer.ps1 | none exist; the build and `build_installer.ps1` run once | "Hydra Deluxe" and "Hydra.Hydra" are typed once, in CMakeLists.txt; a scan row with scope src and installer/hydra.iss guards them | no (S); one-line join with O2 |
| C7 | Store owners | 194, 195 (store half), 196, 197, 252, 253, 298 (per the answer), 299, R7.26, 113 (store half), 191 (the test_store check compares the owner) | src/store/record_store.h/.cpp, path_codec.h/.cpp, serialize.h/.cpp, stored_versions.h if B is chosen; tests test_store.cpp, test_path_codec.cpp | `-sf=*test_store*,*test_path_codec*` | path_codec exports the header offsets and record_store builds every read and every substr from them; one little-endian codec; rank_row returns the reasons; reload_row compares a RecordKey; one cap filter; one column list with a count; no dead probe, no LIKE, one "engine_mode" | yes (K2 touches record_store.cpp) (L) |
| C8a | Parse and scan callers | 186 (scan and .sng callers), 188 (loader and scan callers), 189 (scan caller), 192 (owner), 251 (analysis caller), 256 (owner plus bench and test), R7.19, R7.22 (owner; work_pool and library_jobs callers), 201 (song.cpp caller) | src/app/analysis.h/.cpp, src/app/work_pool.h, src/parse/song.cpp, tools/bench.cpp; tests test_analysis.cpp, test_song.cpp, test_s2_parser_owners.cpp | `-sf=*test_analysis*,*test_song*,*test_s2_parser_owners*` | the scan and both container loaders call the C4 owners; one pitch table; normalize_chart_hash exists; to_library_entry exists and bench and the test use it; run_work_pool trusts its count | yes (C3b, O1, O3a touch analysis.cpp and song.cpp; C3a owns bench.cpp) (M) |
| C8b | Search and hash callers | 113 (search owner side), 204, 201 (library_query, user_messages, report callers), 192 (report, dm_report, dmbot callers) | src/app/library_query.h/.cpp, src/app/user_messages.cpp, src/app/report.cpp, src/app/dm_report.cpp, src/net/dmbot_client.cpp; tests test_library_query.cpp, test_user_messages.cpp, test_report.cpp, test_dm_report.cpp | `-sf=*test_library_query*,*test_user_messages*`, `-tc="records_by_hash*"`, `-tc="collect_dm_rows*"` | no private ASCII helpers outside strutil; one term_applies_to; hashes are lowered only at the DMBot boundary | yes (O3a, K1a, K1b, O3b, C3b) and after C5 and C8a (M) |
| C9a | Preview callers | 182 and R7.21 (load-job caller), 225 (transport caller), R7.25, 347, 191 | src/ui/preview_load_job.h/.cpp, preview_transport.h/.cpp, preview_tab.cpp, src/app/preview_view.h/.cpp; tests test_preview_load_progress.cpp, test_preview_transport.cpp, test_preview_view.cpp, tests/ui/uitest_preview.cpp | `-sf=*test_preview_load_progress*,*test_preview_transport*,*test_preview_view*`, `hydra_uitest preview` | the front pad calls frames_of_ms; the transport does not clamp; the job maps a stem before sizing it; the gauge reads the record's cap with no fallback; the scene asks store::song_length_ms | yes (K4a, K4b, C4a, O3b) and after C3 (M) |
| C9b | Library and Dynamics callers | 262, 195 (dynamics half), 256 (library_jobs caller), R7.22 (library_jobs floor), 113 (the BatchJob search constructor) | src/app/dynamics_breakdown.h/.cpp, src/ui/app_state.cpp, src/ui/dynamics_load_job.cpp, src/ui/library_jobs.h/.cpp; tests test_dynamics_store.cpp, test_dynamics_breakdown.cpp, test_app_state.cpp, test_library_jobs.cpp | `-sf=*test_dynamics_store*,*test_dynamics_breakdown*,*test_library_jobs*`, `-tc="update_dynamics*"`, `-tc="close_details*"` | one Dynamics key; the dynamics blob uses serialize's codec; ScanJob calls to_library_entry; no search-string BatchJob | yes (K5, K3, C4b, K1a) and after C7 and C8a (M) |
| C10 | Rules field table | 199 | src/core/rules.h/.cpp, src/app/rules_file.h/.cpp, tests/test_rules.cpp | `-sf=*test_rules*` | one table walked by the fingerprint and the reader; a pinned `Rules{}` fingerprint literal from before the change still matches | only tests/test_rules.cpp overlaps (S) |

Wave order: C1, C3, C4, C5, C6, C10 can start now in parallel; C2 after C1 (or on C1's branch). After phase 3 merges: C7, C8a in parallel; then C8b, C9a, C9b. 298's answer is needed before C7 starts.

## What the audit got wrong, or has changed since

- 227's owner is `opus_reader.cpp`, not `decode.cpp`; `decode_ogg_opus` is gone (9dc7151). The rate is already named once. Only the two derived constants remain hand-worked.
- 187's lines moved; R7.17 is the current description and 187 should close with it.
- 188 is reshaped by the ranged-reads merge (23a5d97). The loader walks over a ByteSource and the Preview over a whole buffer, so the shared owner must take a ByteSource.
- R7.19's `base_ = 96` copy is gone; `base_` is set from `difficulty_base_pitch` at parse.
- 229: the live runners already call `all_patterns`; `play_chart` passes only the normal pattern, and four old scripts still inline the span.
- 269 is fixed (the triage already said so); the `is_dir` helper the audit named is gone too.
- 201: C2 added a wide-string `starts_with` in winstr.cpp that the audit predates.
- D49 does not cover the probe cut-offs (328) or pacing (273); it is about the worktree and precheck scripts.
- 347: the store's own row types (`PreparedRow::sp_cap`) are plain ints; only `HydraRecord::sp_cap` is optional. The thorough fix is in core/model and path_codec, not the store.

## Left unchecked

- Whether `tests/source_tree.h` walks `.py` files, which decides whether C++ scan rows can guard the probe (grep of source_tree.h for an extension filter found none; I did not read the walker).
- The bodies of `bench.cpp:135-140` (256) and `parent_of`/`dir_name`/`exe_dir` (251): confirmed by grep and the audit's reading, not re-read.
- `kReloadRowSql`'s column list (197): only the compare at 843-846 was read.
- The exact slots `reindex` binds (253): only `list_records`' 17 and 19 were confirmed.
- Line numbers inside the ch_probe test files for 272 and 273 beyond the greps above.
- Nothing was built or run.
