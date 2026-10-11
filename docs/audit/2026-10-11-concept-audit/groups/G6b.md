# G6b: the renderer and the Preview load job

Scout: concept-audit-G6b (agent id `a8d115d91058ff843` per the progress hook; my first few status lines went to `G6b.md`, the fallback id the brief gave). Read in full: all 14 files in the list (3,196 non-blank lines). I also read the lines I name in app/preview_view.{h,cpp}, ui/preview_controller.cpp, ui/preview_tab.cpp, ui/library_jobs.{h,cpp}, ui/job_base.h, app/analysis.cpp and tests/single_owner/rules/render.json.

The short answer to the group question: the renderer and the load job do not keep a second copy of what the app side stores. They take the stored facts (the SP window, the spans, the path key) and use them. The two wave A leads both check out as "agrees". The one real gap is the rules, which are in no key. It is dormant today because rules never change while the app runs.

## Candidates

### Also seen in G6b: G6a-2 (is Star Power running at this moment?)

Looks like: K1. The two tables agree at every instant, including the exact start and the exact end of the window.

Homes (same as wave A, now read from the renderer side):
- `running_activation`, app/preview_view.cpp 241-247 (playhead ms; each activation's `sp_window()`; test is `struck_at(now, start) && now < end`, where `struck_at` is `start <= now`, preview_view.h 348).
- The highway: `SpanSweep::at` (track_state.cpp 78-96) over the `sp_active_` intervals filled from the same `sp_window()` (track_state.cpp 262-266), read through `toggle_on_after` (track_state.h 31-33) in `make_toggle_bounds` / `walk_window` (track_state.cpp 188-209, 434-446) and drawn in highway_draw.cpp 249-277 (floor) and 364-404 (end marks).

What is new: wave A saw that `SpanSweep::at` says Start, not On, at the window's first instant. The highway never reads that word on its own. The only reader of a Toggle outside track_state.cpp is `toggle_on_after` (grep of `Toggle::Start|On|Restart|End` outside track_state.cpp finds only track_state.h 32). `toggle_on_after` folds Start, Restart and On into "on after this instant" and End and Empty into "off after". So the span is on over [start, end) on the highway, the same half-open stretch the boxes use.

Table, boxes (window [s, e), `sp_window()` present):
- now < s -> not running
- now == s -> running (`s <= now`)
- s < now < e -> running
- now == e -> not running (`now < e` is false)
- now > e -> not running
- window with no stored end, or e <= s -> no window, never running

Table, highway (same instants, as the floor tint the strike line sits on):
- t < s -> Empty (off after)
- t == s -> Start (on after): the floor from s toward the future is tinted. The tint's edge is exactly at the strike line when now == s.
- s < t < e -> On (on after)
- t == e -> End (off after): the floor from e on is plain, and an end mark is drawn at e (`sp_active_ends` takes End or Restart).
- t > e -> Empty
- back-to-back windows A [a, b), B [b, c): at b the sweep says Restart (on after, one end mark at b). The boxes at now == b: A fails (`now < b` false), B matches (`b <= now`), so B runs. Agree.
- no stored end, or e <= s -> `sp_window()` is empty, so no interval, nothing tinted. Agree.

Differ on: nothing today. It would split if someone changed `struck_at` to a strict `<` for the boxes (then now == s would be "not running" in the boxes and "tinted" on the highway), or if `toggle_on_after` stopped counting Start as on. Both sides share `sp_window()` as the one owner of the edges, and `struck_at` is shared with the highway's gem flash (highway_draw.cpp 247), so the "chord on the playhead" rule is one function.

Note for the verifier: overlapping windows are the only input where I did not trace both sides. `SpanSweep` calls an edge inside another window "On" and draws no end mark there; `running_activation` returns the first match in list order. A real path cannot overlap two activations, so I treated it as unreachable. I read that from code, not from a test.

### Also seen in G6b: G6a-3 (which settings make a Preview another song, and which only a new overlay?)

Looks like: K1 with a gap on `rules`. The gap is dormant (not live), because nothing changes `settings.rules` after startup.

Trace from the load job outward:
- `PreviewController::open` (preview_controller.cpp 79-115) builds `PreviewSongKey{md5, difficulty, pro, bass2x, noteshuffle}` and the overlay key `path_key + "|cap" + sp_cap` (lines 29-31, 122). `rules` is in neither.
- The path key comes from `path_overlay_key(selected_path)` (preview_view.cpp 719-722), recomputed in preview_tab.cpp 350-355 only when the selected path pointer or `record_generation` moves.
- `open` writes `rules_ = rules` on line 83 and then returns early on line 87 when the song key, `sp_cap` and path key are all unchanged. So a rules-only change stores the new rules and builds nothing.
- Where the rules are used: the parse (`resolve_preview_song`, `check_activations`), the score box's replay (`build_score` -> `replay_stored_path(song, path, rules, ...)`, preview_view.cpp 214-235) and `apply_preview_overlay`. `PreviewLoadJob`, `PreviewNotesJob` and `PreviewSceneJob` each carry their own copy (`rules_`), copied from the controller's `rules_`.
- After a rules-only change, the next overlay or notes job would build with the new rules over a song that was parsed with the old rules (the parse is not redone, because `PreviewSongKey` has no rules). That is a mixed state.
- Why it is dormant: `settings.rules` is set in exactly one place at runtime, `load_startup_settings` (app_state.cpp 26), and the store is opened with that rules stamp (app_state.cpp 42-45). config.h 126-128 says the app only reads the rules file. `apply_settings` (app_state.cpp 853-872) does not look at rules. The only other writers are the two CLI tools.

Table, what the controller rebuilds, by what moved:
- md5 moved -> close, full load.
- same md5, mode moved (difficulty, pro, bass2x, noteshuffle), first load done, no load job running -> notes job only (audio and playhead kept).
- same md5, mode moved, first load still running -> close, full load again.
- path key moved or `sp_cap` moved -> overlay job only.
- `rules` moved alone -> nothing rebuilt; `rules_` updated.
- chart file changed since analysis -> path dropped (`drawn_path`), scene built with no path.

Differ on: `rules`. It is held by four objects and keyed by none. It would become live the day a rules file reload is added. Then the song key and the overlay key would both need the rules stamp, or `open` would need a third comparison. A K1 gap with a write-up for a later phase, not a bug today.

Also: the four mode fields travel as a loose list `(pro, bass2x, difficulty, noteshuffle)` through `read_preview_notes`, `PreviewLoadJob`, `PreviewNotesJob` and `resolve_preview_song` (preview_load_job.h 63-66, 111-112, 221-223). That is wave A's "loose list" point; nothing in my files adds a rule to it.

### Also seen in G6b: G7a-2 (when is a drum note a cymbal?)

What is new from my files: `gem_of`, track_state.cpp 37-48, line 42, `g.cymbal = pro && n.cymbal && allows_cymbals(app::color_of(n.lane))`. This is the third spelling of the cymbal gate after the two parsers (song.cpp 903-911, 1714-1722). It re-applies both halves of the gate to a note that the parser has already gated: `PreviewNote::cymbal = n.is_cymbal()` (preview_view.cpp 46) is only set when the song was parsed with Pro Drums on, and `track_options(pro)` (preview_load_job.h 42-46) passes the same `pro` the parse was given. Same `pro` on both sides, so the tables agree: Pro on + cymbal pad + flagged -> cymbal; anything else -> tom. It would differ only if the controller's `pro_` and the song's parse mode ever disagreed. The controller updates `pro_` when a notes job lands (preview_controller.cpp 157), so they move together. The single-owner rule list does not list this copy.

### Also seen in G6b: G6a-8 (where does each text box sit?)

What is new: `render/overlay_layout` does not decide the stacking. `OverlayBoxes` (overlay_layout.h 52-63) takes the sizes and the two tops (`right_top`, `next_top`) as inputs and only does the fit math (`overlay_scale`, `right_room`, `wrap_right_box`). The stacking order appears only in comments there (file header lines 1-3, and the field comments "score box under it", "under the drain box"). So the second home is a restating comment, as wave A said; there is no second rule in this file.

### G6b-1. Has the chart file changed since its record was analyzed?
Looks like: K2 at two edges (two jobs, one shared test function, different answers for an unreadable file). Possibly K3.

Homes:
- `PreviewLoadJob::run`, ui/preview_load_job.cpp 176-190 (inputs: `entry_.notespath`, `entry_.sig`, `entry_.md5`; calls `chart_changed_since`, then `now && !now->md5.empty() && now->md5 != entry_.md5`). The answer is `chart_changed`, which drops the drawn path.
- `ViewJob::run`, ui/library_jobs.cpp 496-513 (inputs: `song_.notespath`, `song_.sig`, `song_.md5`; calls the same `chart_changed_since`, then `now->md5.empty()` throws, and `now->md5 != song_.md5 || store::sig_can_show_unchanged(now->sig)` sets `files_changed`).

Table, home 1 (Preview): `chart_changed_since` gives nothing (fingerprint matches) -> not changed; file hashed, hash equals the entry's md5 -> not changed; hash differs -> changed; hash empty (unreadable) -> not changed (the parse error will speak); same hash but a fingerprint that cannot show unchanged -> not changed.
Table, home 2 (click): nothing -> no change; hash differs -> files_changed, new md5 and sig saved; hash empty -> the job fails with "could not read <path>"; same hash but `sig_can_show_unchanged(now->sig)` -> files_changed (only to save the new fingerprint, D96).

Differ on: an unreadable file (not changed vs the click job fails), and a rewritten file with the same content (not changed vs files_changed, which is bookkeeping, not "stale path"). The two jobs ask related but different questions, and the comment at library_jobs.cpp 498 says the test function is "the one answer, shared with the preview load". So the shared part is one function; the surrounding decision is per job. The thing the verifier should look at: a changed chart is hashed twice, once by the click job and once by the Preview's `std::async` thread, because the Preview opens from the library entry whose md5 the click job has not necessarily saved yet. I did not check the order in which the row's md5 is updated, so that is a read, not a proof.

### G6b-2. Two ways to assemble a Preview scene and its highway timeline
Looks like: K3-with-a-rule-inside. The rule inside is "what part of the timeline is path-free and what part is overlay".

Homes:
- Whole build: `build_scene_and_highway`, preview_load_job.cpp 54-66 -> `app::build_preview_scene` (= `build_preview_base` + `apply_preview_overlay`, preview_view.cpp 260-265) then `render::build_track_state`. Used by `PreviewLoadJob` and `PreviewNotesJob`.
- Split build: `build_scene_base` (preview_load_job.cpp 348-358) for the path-free half, then `PreviewSceneJob::run` (398-419): `apply_preview_overlay` on a copy of the base scene and `render::rebuild_overlay_fields` on a copy of the base timeline.
- Third entry: `PreviewRenderer::set_scene(scene, opts)` (preview_renderer.cpp 416-418) calls `build_track_state` again on the render thread when no worker-built timeline fits the options (preview_controller.cpp 359-362).

Table (same song, path, cap, rules, pro): all three give the same `TrackState`. The equality is pinned by tests/test_track_state.cpp 845 ("equals a full build on random overlays") and 907 (every 8th corpus chart's stored paths), which call the production functions. The rule "which fields are overlay" lives once, in `TrackState::set_overlay_intervals` (track_state.cpp 249-277) and `merge_overlay_edges`; the single-owner list already pins `merge_overlay_edges` as the one merge (render.json 107).

Differ on: agree today. It would split if a new span field were added to `build_track_state`'s path-free half but not to `set_overlay_intervals`' clear list, or the other way round. The tests above would catch it on corpus charts.

### G6b-3. Is `PreviewNote::solo` a second home of "is this note in a solo"?
Looks like: K1, or a field nobody reads (lead for another group, since the field is in app/preview_view).
Homes: `PreviewNote::solo = ts.flag_solo`, preview_view.cpp 50; and `PreviewScene::solos` from `song.solo_sections` (preview_view.cpp 292-294), which is `find_solo_sections` over the same `flag_solo` (song.cpp 88-95), then `TrackState::solo_` spans (track_state.cpp 289).
Table: a note with `flag_solo` -> `PreviewNote::solo` true and its tick lies inside its section's span (first chord to last chord, plus kSpanEndTicks past the last). A note without the flag -> false and outside every span. The two agree by construction because the spans are computed from the flags.
Differ on: agree today. My grep (`(note|n|pn|it)\.solo`, `->solo`) finds no reader of `PreviewNote::solo` in src or tests; only the track state's own `TrackInstant::solo` is read. Verifier: confirm the field is unread. If so it is a dead second home, not a rule copy.

### G6b-4. Do the two load jobs hand-roll job machinery that `ui/job_base.h` could own?
Looks like: K3 (parallel on purpose), with small per-job rules inside. I list it because the brief asked; see "Parallel on purpose" for what each side knows.

What `job_base.h` holds (read in full, 109 lines): `JobBase` (thread, cancel flag, finished flag, `spawn`, `shutdown`, `throw_if_cancelled`) and `ResultJobBase` (ok, error, message, `run_guarded`, `fail`). Nothing about progress, elapsed time, counts or a time-left clock.

What `PreviewLoadJob` (preview_load_job.h 109-209) and `StoreOpenJob` (library_jobs.h 100-155) each write for themselves:
- A start time and "seconds since start": `started_` set in `start()`, and the same expression `duration<double>(steady_clock::now() - started_).count()` at preview_load_job.cpp 258-259 and library_jobs.cpp 90-92. Same four lines, two homes.
- A mutex plus a `ByteRateClock` fed from `progress()` on the render thread (`clock_mutex_`, `clock_`; preview_load_job.cpp 260-263, library_jobs.cpp 104-109). The clock class itself and the gate and words (`ByteRateClock`, `kTimeLeftAfterSeconds`, `gated_time_left_text`) are already shared, defined in preview_load_job.{h,cpp} and used by `StoreOpenProgress::time_left_text` (library_jobs.cpp 43-46). So the shared part has an owner, but the owner sits in the Preview's header, and library_jobs.h forward-declares it (library_jobs.h 40).
- A done/total pair of atomics (`bytes_done_`/`bytes_total_`; `rows_done_`/`rows_total_`).
- A `start()` that is `spawn([this]{ run(); })`. This line is written in eleven places in src/ui (DmFetchUsersJob, DmReportJob, ScanJob, PreviewNotesJob, PreviewBaseJob, PreviewSceneJob verbatim; PreviewLoadJob and StoreOpenJob add the start time; BatchJob, ViewJob and ReportJob add their own steps; found by grep of `::start()` in src/ui, and the three bodies in library_jobs.cpp were not read). The destructor line `~X() { shutdown(); }` appears in a dozen. job_base.h's comment says derived destructors must call `shutdown()` on purpose, so that one is by design.

Where the two differ, and each difference is a per-job fact: the Preview derives its step from four atomic bools (`reading_done_`, `audio_done_`, `scene_done_`, `highway_done_`; progress() lines 248-255), the store open stores one `int` step and publishes counts before the step on purpose (library_jobs.cpp 113-119: "a frame that sees the new step sees its counts too"). The Preview clamps `bytes_done` to `bytes_total` (preview_load_job.cpp 257); the store open does not clamp rows, and relies on the bar helper (`progress_bar_counted`, widgets.h 75). Whether `progress_fraction` clamps was not read (widgets.h 68-80 not opened).

Table: none needed (no rule differs between the two homes beyond the clamp above). Differ on: `bytes_done > bytes_total` (Preview clamps in `progress()`; store open leaves it to the bar). Agree on every other input I could name.

Judgement for phase 4, not a grade: a "progress and elapsed" base class would own `started_`, `elapsed_s()`, the clock mutex and the clock. It would remove about 12 lines in two jobs and move the owner of `ByteRateClock` out of the Preview's header. Group G11a reads job_base.h and StoreOpenJob in full and should say whether the other jobs (BatchJob has its own `BatchClock` and `batch_eta_s`) belong under the same base.

## Parallel on purpose

### The three pad enums: `NoteColor` (core/model.h 51), `PreviewLane` (app/preview_view.h 37), `Pad` (render/track_state.h 43)
All three name the same five drum lanes (kick plus four pads). Converters: `lane_of` / `color_of` (preview_view.cpp 19-35) and `pad_of` (track_state.cpp 26-35).
- `NoteColor` knows the game's numbering (values start at 1, Kick first) and the cymbal rule (`allows_cymbals`, model.cpp 17-20).
- `PreviewLane` knows that the renderer's side must not include the parser's enum (preview_view.h 35-36). It keeps Kick = 0.
- `Pad` knows only the four pads left to right (value indexes per-pad arrays; `kPads` is the one loop list, pinned in render.json 424), and that a kick is not a pad (`pad_of` gives none).
Why each entry is a per-module fact: each enum is a naming and numbering choice for one layer; each converter is a switch with no logic beyond the one fact "kick has no pad". The cymbal gate is the one rule that sits in the middle (see G7a-2 above). One oddity for phase 4: `color_of` turns a lane back into a core colour "so a renderer holding only the lane can still ask the core's colour rules" (preview_view.h 41-43), so the layer split is not clean. That is design, not duplicated logic.

### `Step` in `PreviewLoadJob::Progress` and in `StoreOpenProgress`
`PreviewLoadJob::Progress::Step { Reading, Opening, Building, Highway }` (preview_load_job.h 156) and `StoreOpenProgress::Step { Opening, UpdatingResultsKey, Copying, Finishing, LoadingLibrary }` (library_jobs.h 49) are two nested enums with different meanings. Both have an `Opening` member and both are shown as a label. Each side knows its own list of steps, its own label words (preview_load_job.cpp 282-297; library_jobs.cpp 27-39) and which single step has a rate (Opening audio for the Preview, Copying for the store). The time-left gate and words are shared (`gated_time_left_text`). Per-module facts all. The bar's fill rule is shared too (`progress_fraction`). The Preview also has its own step shares of the bar (`kReadShare`, `kOpenShare`, `kSceneShare`, preview_load_job.cpp 32-34), a timing fact from one measured chart.

### Time left: `ByteRateClock` against `batch_eta_s`
Three estimators exist: the Preview's and store open's byte/row-rate clock (gate: job at least 3 s old), and the batch's average per finished chart (gate: at least 3 charts finished, `kEtaMinFinished`, library_jobs.h 221). They share the words (`time_left_text`, library_parts.h 63; used by library_dialogs.cpp 456 and `gated_time_left_text`). The gates and the estimators are different on purpose (a rate over a second versus a per-chart average), so these are per-job facts. I did not read `batch_eta_s` itself.

### Highway geometry: `track_rect`, `time_to_z`, `railing_x`
Each is written once in highway_draw.cpp and used by the renderer (preview_renderer.cpp 406), the overlay layout (overlay_layout.cpp 19, 41-42) and the draw list. Grep of `height_width_ratio`, `track_rect`, `time_to_z`, `far_time` outside highway_draw finds only these. Cleared, and already pinned by single-owner rules (render.json 169, 204-219, 290).

### Asset error messages ("PreviewRenderer: missing ...", "obj: ...", "3d-config.json: missing key ...")
Three loaders in render/ throw `KindedError(ErrorKind::PreviewAssets, ...)` with their own prefix (preview_renderer.cpp 165, 169, 198, 306, 315; obj_loader.cpp 76, 131; preview_config.cpp 63, 98). Each prefix names what it read. No shared rule; the error kind is the shared fact. Also `PreviewRenderer` throws plain `std::runtime_error` from `check` for D3D failures (line 36), a different class from the asset errors. Phase 4 could ask whether that split is intended; it is not a duplicate.

## Dropped leads

- `DepthMode` in render/highway_draw.h 59 (`Less`, `Always`: whether a draw tests the depth buffer) and in search/engine.h 20 (`Scores`, `Points`: the score-range unit). K0. Different namespaces (`hydra::render` and `hydra`), different questions. The only risk is a file that is in `hydra::` scope and includes both headers; no such file in my group does (preview_renderer.cpp is inside `hydra::render` and gets its own). A naming note, nothing more.
- `Step` in ui/preview_load_job.h and ui/library_jobs.h: see "Parallel on purpose" (K3, per-job facts).
- "Building highway", "Building scene", "Reading chart" in preview_load_job: each string appears only in `Progress::label` (preview_load_job.cpp 282-297) and in comments. Other screens say "Reading chart..." (dynamics_tab.cpp 130) and "Analyzing chart..." (details_panel.cpp 162) for other jobs, so they are different jobs' words, not the Preview's step. Not a duplicate; maybe a note for G1/G3 on wording of "Reading chart" (two screens use it for two different reads).
- "3d-config.json: missing key " in preview_config: one home (preview_config.cpp 63); its other mentions are in the header comment and tests. K0.
- "On before this instant" (walk_window, track_state.cpp 195) beside `toggle_on_after`: the single-owner rule at render.json 59-84 already decides it is a different question. Known, not written again.
- `StoreOpenJob` against `PreviewLoadJob` (four shared fields): written up as G6b-4 above.
- Ellipsis cut: `render::ellipsize` is the one home; all other users go through it (grep of `ellipsize(` and `text_ellipsized(` across src; library_table.cpp 377, preview_tab.cpp 162, settings_bar.cpp 270, widgets.h 210). Pinned by render.json 29. Cleared.
- Word wrap: `wrap_words` / `wrap_right_box` are used only for the Preview's right-hand boxes. Pinned by render.json 315. Cleared.
- `build_track_state` / `rebuild_overlay_fields` equality: see G6b-2 (tests pin it).

## Not read

None. All 14 files read in full. Lines outside my group were read only where named above. I did not read `batch_eta_s`, `progress_fraction` (widgets.h 68-80) or the tests behind the single-owner rule patterns.
