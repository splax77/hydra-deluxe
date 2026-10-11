# G6a: the Preview's app side and UI side

Scout: concept-audit-G6a. Read-only. Every file in the list was read in full: `app/preview_view.cpp/.h`, `app/preview_source.cpp/.h`, `app/preview_clock.h`, `app/song_length.cpp/.h`, `ui/preview_tab.cpp`, `ui/preview_controller.cpp/.h`, `ui/preview_transport.cpp/.h`. Outside the group I read only: the Preview job structs in `ui/preview_load_job.h` (lines 100-350), `read_preview_notes` and the load `run()` in `ui/preview_load_job.cpp`, `render/overlay_layout.h`, the sweep and the overlay-interval code in `render/track_state.cpp`, `Settings::chartmode_key`/`to_analysis_settings` in `app/config.cpp`, `ViewedSong` in `ui/app_state.h`, and the timeline-end lines in `app/path_view.cpp` (408-412).

All paths below are under `src/`.

## Short answers to the specific questions

**Does the controller hand its job fields over, or work them out again?** Handed over. By my reading of the declared members, `PreviewController` shares six names with `PreviewSceneJob` (`song_`, `path_`, `sp_cap_`, `rules_`, `audio_end_ms_`, `song_length_ms_`) and five with `PreviewNotesJob` (`pro_`, `path_`, `sp_cap_`, `rules_`, `audio_end_ms_`). The probe said five and four; it probably left one out each. In both jobs the values come from the controller as constructor arguments (`start_scene_job`, `start_notes_job`, `poll`, `open` in `ui/preview_controller.cpp`). The controller got them from `open()`'s arguments (path, cap, rules, mode) or from the first load's `Result` (`audio_end_ms`, `song_length_ms`, `song`), and keeps them so later jobs can be handed the same values. Nothing is recomputed from the chart. The one rule that sits between the copies, "a changed chart draws no path", is `drawn_path` in `ui/preview_load_job.h` (line 34), called from both the load job (`preview_load_job.cpp` 228) and the controller (`path_to_draw`, `preview_controller.cpp` 127). One owner. So these shared names are K3 in shape (same fields on each side of a hand-off), not a copy of a rule. They are listed under "Parallel on purpose".

**Do the two `State` enums answer one question?** No. `PreviewScore::State {None, Unavailable, Ready}` (`app/preview_view.h` 235) says whether the running score can be shown for the drawn path. `ViewedSong::State {None, Analyzing, Ready, Cancelled, Failed, FileMissing, RulesBroken}` (`ui/app_state.h` 160) says where the click's analysis job stands. They share the words None and Ready only. K0.

**Where are the song length, the playhead clamp and the "score so far" decided, and does the tab or transport decide them again?**
- Song length: one owner, `app::song_length_ms` (`app/song_length.cpp` 28). The Preview asks it through `chart_song_length_ms` in `read_preview_notes`; the Paths tab asks the same function through `analysis_song_length` (`app/analysis.cpp` 856). Neither the tab nor the transport works it out again. The tab only reads it through `PreviewController::scrub_end_ms`. See G6a-5 for a related split: the Preview has three different "ends", and the time box and the Paths timeline label different ones.
- Playhead clamp: not one place. The transport clamps seeks, the display clamps again, the scrub thumb clamps again, and a fraction clamps again. See G6a-1.
- "Score so far": decided once, in `build_score_box` (`app/preview_view.cpp` 552-590), off `PreviewScore::steps`, which `build_score` fills from the core replay. The tab only draws the finished strings and the star count. I found no second place that picks "the chord the counter reads at this time". The only neighbour is the SP-running test the score box and the drain box share (G6a-2).

## Candidates

### G6a-1. Where can the playhead be, and what does the screen show when it is outside?
Looks like: K1 with one edge that differs (a K2 on the empty chart and on a scrub range longer than the playback range).
Homes:
- `PreviewTransport::seek_ms` and `PreviewTransport::tick`, `ui/preview_transport.cpp` 77-107 (inputs: a requested ms, `length_ms_` = max of last drawn note and audio end).
- `shown_length` and `shown_ms`, `app/preview_view.cpp` 484-494 (inputs: the clock's `now_ms`, and `length_ms`, which the controller passes as `playback_end_ms()` for the time, score, drain and next-activation boxes and for `step_tick_ms`).
- `scrub_thumb_ms`, `app/preview_view.cpp` 660-664 (inputs: `now_ms`, the scrub edge).
- `song_fraction`, `app/preview_view.cpp` 640-643 (clamp 0..1 for the gold marks).
- The gauge fill clamp, `ui/preview_tab.cpp` 736-737 (bars over cap, clamp 0..1). This is a different quantity, so I list it only as a fourth spelling of "clamp to 0..1".
Table, home 1 (transport, L = `length_ms_`):
- ms < 0 -> 0.
- L > 0 and ms > L -> L.
- L = 0 (no notes, no audio) and ms > 0 -> ms unchanged; no upper clamp.
- tick() while playing: L > 0 and clock >= L -> pause and pin to L. With L = 0 the clock runs on without end.
- The clock itself can sit past L between two tick() calls; `now_ms()` does not clamp.
Table, home 2 (`shown_ms(now, len)`):
- len < 0 -> treated as 0.
- now < 0 -> 0.
- now > len -> len (so with len = 0 the display is pinned to 0).
- otherwise now.
Table, home 3 (`scrub_thumb_ms(now, edge)`):
- edge > 0 -> min(now, edge).
- edge <= 0 -> now, with no lower clamp.
Differ on:
1. A chart with no notes and no audio (L = 0): the transport lets the clock run and seek anywhere; the display pins every box to 0.
2. The slider range is the scrub edge (song length when the metadata states one), but the seek it triggers is clamped to the playback range. If the stated length is longer than both the last note and the audio, dragging the thumb to the right end seeks to the playback end, and the next frame the thumb is drawn at the playback end rather than the right end. I did not run this; it follows from `PreviewController::scrub_end_ms` (`preview_controller.cpp` 403) against `PreviewTransport::seek_ms` (transport 77-80).
3. `scrub_thumb_ms` has no lower clamp, the others do.
Note: the first two differences are on edge inputs only. The verifier should check difference 2 against a chart whose song.ini length exceeds its audio. The "known, line-level" part: the derivation audit already lists the 0..1 fraction formula (line 232 of `docs/audit/2026-10-03-derivation-audit.md`); that copy is now unified in `song_fraction`.

### G6a-2. Is Star Power running at this moment?
Looks like: K1 with an edge that differs at the exact instant (probably intended; G6b should confirm).
Homes:
- `running_activation`, `app/preview_view.cpp` 241-247 (inputs: the playhead ms, each activation's `sp_window()`; used by `build_score_box` and `build_drain_box`).
- `SpanSweep::at` over the `sp_active_` intervals, `render/track_state.cpp` 78-96, with the intervals filled from the same `sp_window()` at 262-266 (inputs: the time of each highway row, the same window). Not in my group; I read only these lines.
Table, home 1 (`running_activation`), window [s, e):
- now < s -> not running.
- now == s -> running (struck_at is `note_ms <= now_ms`).
- s < now < e -> running.
- now == e -> not running.
- activation with no stored SP end, or e <= s -> `sp_window()` is empty -> never running.
Table, home 2 (`SpanSweep::at`), same window:
- now < s -> Empty.
- now == s -> Toggle::Start (not "On").
- s < now < e -> On.
- now == e -> Toggle::End.
- empty window -> not an interval at all.
Differ on: the instants exactly at the window's start and end. Home 1 answers a yes or no for the playhead; home 2 answers a drawing question per row ("does a boundary mark go here"). They may be different questions that happen to read one input.
Note: both read `PreviewActivation::sp_window()` (`preview_view.h` 114), so the window itself is single-owner. The split is only in how an instant at the edge is classified. Jumping to an activation puts the playhead exactly on `s` (see `seek_activation`), so the edge case is normal use: the drain box says "empties in" and the floor starts its tint on the same frame. Phase 4 should decide if that is intended.

### G6a-3. Which settings make a Preview "another song", and which make only a new overlay?
Looks like: K1 (a list of fields written in several places) with a K2 risk on `rules`.
Homes:
- `PreviewSongKey`, `ui/preview_controller.h` 56-68 (md5, difficulty, pro, bass2x, noteshuffle) and its use in `PreviewController::open`.
- `Settings::chartmode_key` (`app/config.cpp` 266-279) and `Settings::to_analysis_settings` (315-328): the same four mode fields, spelled for the store key and for analysis. `Settings::with_chartmode` (297-313) is the inverse and loops over the same four.
- The loose parameter list `(pro, bass2x, difficulty, noteshuffle)` carried by `PreviewLoadJob`, `PreviewNotesJob`, `resolve_preview_source`, `resolve_preview_song`, `chart_song_length_ms`, `dynamics_for_settings`.
- The overlay comparison in `open()` (`preview_controller.cpp` 85-95): path key and `sp_cap`.
Table, home 1 (what `open()` does when only one input changes, same md5, chart already open):
- difficulty / pro / bass2x / noteshuffle changes -> notes reload job, audio and playhead kept (if first load done).
- path changes -> overlay job only.
- `sp_cap` changes -> overlay job only (the cap is inside the overlay key).
- `rules` changes -> `rules_` is overwritten at the top of `open()` (line 83) but no job starts: nothing rebuilds.
- md5 changes -> full close and reload.
Table, home 2 (`chartmode_key` / record key):
- difficulty / pro / bass2x / noteshuffle change -> different key.
- `sp_cap` change -> different `cap_query`.
- `legacy_fills`, depth, ms limit -> different `Lens`.
- `rules` -> not in the key (they sit in `to_analysis_settings` only).
Differ on: `rules`. The running score in the scene is priced from `rules` (`preview_view.h` 459-460), but a rules change alone does not rebuild the overlay while a chart is open. Whether that matters depends on whether a rules change also changes the viewed record's path key. I did not trace that.
Note: `PreviewSongKey`'s comment says it holds "the inputs `Settings::to_analysis_settings` reads to pick the notes". That is a comment pointing at a rule, not the rule itself, so a fifth mode setting must be added to `chartmode_key`, `to_analysis_settings`, `with_chartmode`, `PreviewSongKey`, `open()` and every job constructor by hand. Overlaps G3 (which settings a page depends on); G3 may want this.

### G6a-4. When does the last note start?
Looks like: K1, agrees today. Known, line-level (derivation audit line 232 names it); listed because a second owner now exists.
Homes:
- `last_drawn_note` / `last_note_ms`, `app/preview_view.cpp` 251-258 (inputs: `scene.notes`, built from `song.sequence` in tick order; takes `notes.back()`).
- `last_note_start_ms`, `app/song_length.cpp` 20-26 (inputs: `song.sequence` from the end; first timestamp whose `chord.count() > 0`).
Table, home 1 / home 2, same song:
- no notes -> nullptr and 0.0 / nullopt.
- last timestamp holds a chord -> its ms / its ms.
- trailing timestamps with empty chords -> skipped (the scene has no note for them) / skipped (`count() > 0`).
- chord with several lanes -> the shared ms / the shared ms.
Differ on: agree today. They split if the scene ever drops or merges a note the sequence keeps (for example a lane hidden by a mode), or if the sequence ever holds a chord whose notes are not drawn. The two inputs are also parsed differently by design: the scene is at the selected mode, the length backup is the Expert chart with 2x kick.
Note: `song_length.h` line 38 says "It agrees with last_note_ms of the Preview's scene" - a comment that names the agreement instead of one function calling the other. No test or shared call enforces it that I saw (I did not read tests).

### G6a-5. What is "the end of the song", and which end does each label show?
Looks like: K2 (same word, different rules), in front of a user.
Homes:
- `PreviewTransport::length_ms` = max(last drawn note, audio end in chart time), `ui/preview_transport.cpp` 26-32. The time box's `length` and the "m:ss / m:ss" clock text come from it (`build_time_box`, `preview_view.cpp` 498-512, with `length_ms` = `playback_end_ms()`).
- `app::song_length_ms`, the metadata length or the last Expert-2x note (`app/song_length.cpp` 28-40), shown by the scrubber's edge and gold marks (`scrub_end_ms`) and by the Paths tab's timeline end (`path_view.cpp` 408-412, `timeline_end`).
- Two stored copies of the same song length: the controller's `song_length_ms_` and `PreviewScene::song_length_ms`, and a third on `ViewedSong::song_length_ms` for the Paths tab. All three come from the same owner function, called separately by `read_preview_notes` and by the click's analysis.
Table, playback end (home 1) vs song length (home 2), by case:
- metadata states a length, audio runs longer -> audio end vs metadata length (moved into chart time).
- metadata states no length, audio missing -> last note vs last Expert-2x note start.
- metadata states a length longer than both the audio and the notes -> the longer of audio end and last note vs the metadata length; playback end is the shorter (see G6a-1 difference 2).
- no notes, no audio -> 0 vs nothing (unless the metadata states a length).
Differ on: any chart whose audio outlasts its stated or last-note length. The Preview's time box "of m96.3.240" and the Paths timeline's end measure are then different measures for the same chart.
Note: this is by design per the comments (D70, D75, D48 Q25), so it may be K3 on purpose; but the labels the user sees should be checked. I did not read the Paths tab's label text, only the code that builds `timeline_end`. The derivation audit (line 220-234) asked the user to choose chart end or audio end; D75 answered for the scrubber, and I did not find where the time box was decided.

### G6a-6. Is this length one the timeline can use?
Looks like: K1, agrees today. Known, line-level.
Homes: `has_song_length` (`preview_view.cpp` 638, "positive") with `stated_length_ms`, `song_fraction`, `scrub_end_ms`, `scrub_thumb_ms` and path_view all calling it; against `length_ms_ > 0.0` in `PreviewTransport::seek_ms` (line 79) and `tick` (line 102), and `len_s > 0.0f` in `ui/preview_tab.cpp` 497.
Table: length < 0, = 0, > 0 -> unusable, unusable, usable in all spellings. `shown_length` treats < 0 as 0 and keeps 0 as a length of 0.
Differ on: agree today. Split if "usable" ever needs a floor above 0 (for example a minimum of one second).
Note: the transport's test guards the playback range, not the song's length, so it can be argued to be a different question. The tab's `1.0f` fallback slider range (line 497) is a UI guard for ImGui.

### G6a-7. Does the drawn path have activations?
Looks like: K1, agrees today.
Homes: `no_acts = scrub_marks.empty()` in `ui/preview_tab.cpp` 428-429 (decides whether "< Act" and "Act >" are greyed); `scene_.activations` in `jump_activation` and `seek_activation` (`preview_controller.cpp` 489-502) and `activation_jump_ms` (`preview_view.cpp` 671).
Table:
- no path -> no marks, no activations: greyed / jump returns nothing.
- path with activations and a usable scrub edge -> marks present / activations present.
- path with activations and an unusable scrub edge (no song length and no playback length) -> no marks, so buttons greyed / activations exist, `[` and `]` still work (the keys are not gated).
Differ on: the last row, which needs an empty playback range and activations together. I could not construct it (activations come from notes, so the playback range would be non-zero). Agree today. They split if `song_fraction` ever drops a mark.

### G6a-8. Where does each text box sit on the Preview image?
Looks like: K1 inside one file (`ui/preview_tab.cpp`), with a restating comment in `render/overlay_layout.h`.
Homes:
- The fit pass, `preview_tab.cpp` 591-648: left column height = time box + gap + score box (if shown); drain box at `v_margin`; next box under the drain box via the `next_top_at` lambda.
- The draw pass, 660-812: score box at `box_max.y + sz.gap`; drain box at `origin.y + v_margin`; next box at `next_top_at(sz)`.
- `render::OverlayBoxes`, `render/overlay_layout.h` 52-63, whose comments describe the same stacking ("score box under it", "next-activation box, under the drain box").
Table, fit vs draw:
- score shown -> fit adds `gap + score_h`; draw places it `gap` under the time box. Agree.
- drain shown -> fit uses top `v_margin`; draw uses `origin.y + v_margin`. Agree (the number is typed in both spots).
- drain hidden -> `next_top_at` handles it for fit and draw alike (one lambda).
- gauge: its geometry is typed once.
Differ on: agree today. The left column and the drain box are described twice; the next box is described once. A change of order (score above time, say) needs edits in the fit and draw spots.
Note: `overlay_box_sizes` is the one home for sizes (the comment at 256-258 says so and the code agrees). Only positions are repeated. The time box's own lines ("position  of length", tempo, section) are also assembled in the tab (lines 567-572) rather than in `build_time_box`; the app side holds the fields, the tab the layout of the text.

## Parallel on purpose

**`PreviewController` against `PreviewSceneJob` and `PreviewNotesJob` (K3).**
- The controller knows: which job is in flight, the retired jobs, the keys (`requested_path_key_`, `path_key_`, `job_path_key_`, `scene_path_key_`), the current scene and its dirty flag, the audio device, and the volume.
- A job knows: the inputs it was started with and its own `Output`/`Result`.
- Why per-module: the controller owns live state across jobs; each job owns a snapshot, copied so it can outlive the caller's settings (comments on `rules_` say so). The copies are handed over, not re-derived. The one shared decision, "a changed chart draws no path", is `drawn_path` (one owner). The overlay key is written and read back in one pair of functions (`overlay_key`, `overlay_key_path_part`, `preview_controller.cpp` 29-39).

**`PreviewScene` (app) against `render::TrackState` (render): the highway.** Not in my files, but read at the seam. TrackState holds drawing intervals; the scene holds the facts and `sp_window()`. Per-module facts only, except the edge question in G6a-2.

**`PreviewClock` against `PreviewTransport`.** The clock knows play start and song ms (pure arithmetic, injectable time). The transport knows the audio playhead, the lock, the offset and the playback range. The clamp is in the transport, not the clock, and the clock has no clamp of its own. Clean split, apart from G6a-1.

## Dropped leads

- `State` in `preview_view.h` and `app_state.h`: K0, two different questions (score availability and analysis job state); only the words None and Ready overlap.
- "Score unavailable" and "star <n> in <points>": one home each, `app/preview_view.cpp` 558 and 587. The tab only draws the strings (its mentions are comments). K0.
- "SP drain (if activated)": one home, `preview_view.cpp` 620. K0.
- "about 0:40 left" and "Opening audio: 312 of 625 MB": the text is built once, in `PreviewLoadJob::Progress` (`preview_load_job.cpp` 288-308) with the words from `ui/library_parts.h`' `time_left_text`; the controller copies the three finished values (`preview_controller.cpp` 202-206). The same example strings appear only in comments in `preview_controller.h` (141-145). Comment-level echo, not a second home.
- "Show in Preview" across `paths_tab.cpp`, `preview_controller.h`, `preview_tab.cpp`: the activation index is meaningful in both tabs because both walk `Path::walk_activations()` (`preview_view.cpp` 366 and `path_view.cpp` 236). One walk. K0. A stale comment: `preview_view.h` line 449-450 says activations are "those carrying a timecode", but the code at 366 filters nothing.
- `PreviewController` shared names: see "Short answers" above; K3, listed above.
- Percent-to-gain typed twice (derivation audit 1541): now one call, `Settings::volume_gain`, used at `preview_controller.cpp` 298 and 444. Fixed; known, line-level.
- `struck_at` copies (derivation audit 360): unified; `render/highway_draw.cpp` 247 calls `app::struck_at`. Fixed.
- Offset / delay rule: `preview_audio_offset_ms` is the only home (`preview_source.cpp` 310); `chart_audio_offset_ms` and `song_length_ms` both call it. K0.
- `is_audio_filename`, `base_name`, `stem_of` in `preview_source.cpp`: grep found the extension list and the path-splitting helpers nowhere else in `src/`. Dropped (G8/G9 may know of others in `audio/` I did not search by another spelling).

## Not read

None of my files. Outside my group I did not read: `render/highway_draw.cpp` beyond line 247, `core/replay.cpp`, any test file, the Paths tab's label text, and the rest of `ui/preview_load_job.cpp` (only 40-110 and 195-310 were read).
