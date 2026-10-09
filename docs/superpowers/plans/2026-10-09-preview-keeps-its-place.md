# The Preview keeps its place when the notes change (plan, 2026-10-09)

**Status:** this is a plan for the user's review. No code is written yet, and the build waits for the user's yes (rule 3 in `CLAUDE.md`).

**Goal:** with the Preview open, changing Difficulty, Pro Drums, 2x Bass or Note Shuffle swaps the notes under the playhead. The playhead stays where it was. If the song was playing, it keeps playing. If it was paused, it stays paused at the same spot. The audio never stops.

**Where this came from:** the user's request on 2026-10-09: "currently if you change any analysis settings while the preview is open, it resets back to the start of the song. i want the preview to stay where it is while the song changes under it."

---

## What happens today

The Preview draws through one `PreviewController`. The Preview tab calls its `open()` every frame (`src/ui/preview_tab.cpp` line 286). `open()` first decides whether it is still showing "the same song". The rule is `PreviewSongKey::operator==` in `src/ui/preview_controller.h` (lines 60-63):

```cpp
return md5 == o.md5 && difficulty == o.difficulty && pro == o.pro &&
       bass2x == o.bass2x && noteshuffle == o.noteshuffle;
```

So the same chart at a different difficulty, or with Pro Drums, 2x Bass or Note Shuffle flipped, is a different song. For a different song, `open()` calls `close()` and starts a fresh `PreviewLoadJob` (`src/ui/preview_controller.cpp` lines 75-90). `close()` unloads the transport, which is the play/pause clock and the audio that follows it. `PreviewTransport::unload()` puts the clock back to 0 and pauses it (`src/ui/preview_transport.cpp` lines 30-37). When the new load lands, `PreviewTransport::load()` seeks to 0 and pauses again (lines 26-27). That is the reset the user sees. The audio files are opened again from scratch, and the panel shows the loading bar instead of the highway while that happens.

The other analysis settings take a different path, judging by the code. `AppState::apply_settings()` (`src/ui/app_state.cpp` lines 838-857) re-runs the song's analysis when the chart mode, SP cap or lens changes. That re-run changes the selected path, so `open()` sees the same song with a new path key. It then only rebuilds the gold path overlay on a background job and leaves the playhead alone (`src/ui/preview_controller.cpp` lines 62-73). So SP cap, Score range, Path limit and 1.0 fills should already keep the playhead. **This has not been tried in the app.** It comes from reading the code. Step 1 of the build checks it.

The comment on `PreviewSongKey` cites the decision behind this rule as "D48, Q22; D104 item 1". That decision stays: a mode change still loads that mode's notes. This plan changes only what happens to the playhead and the audio while it does.

## The idea

Nothing about the audio depends on these four settings. The stems come from `app::resolve_preview_stems(notespath, container, ...)`, which takes no difficulty or mode (`src/ui/preview_load_job.cpp` line 88). The audio offset is where chart time 0 sits in the audio. It comes from `chart_audio_offset_ms`, which reads the song.ini delay or the chart's Offset, again with no difficulty or mode (`src/app/preview_source.cpp` lines 358-374). So when only these settings change, the audio can keep playing and only the notes need rebuilding.

A path change already works this way. A background job builds the new scene, the old one stays on screen until it lands, and `poll()` swaps it in. The notes change will copy that pattern one level up: re-read the notes, rebuild the scene and highway, swap them in, and leave the transport loaded.

## The design

### When the notes reload, and when the whole Preview reloads

`open()` keeps comparing `PreviewSongKey`. What changes is what it does when the key differs:

- **Different chart (md5).** Full reload, as today. There is no position to keep.
- **Same chart, and the first load is still running** (`job_` is set). Full reload, as today. Nothing has played yet, so nothing is lost.
- **Same chart, and the first load failed** (no song or no transport loaded). Full reload, as today, so the audio gets a chance to load.
- **Same chart, first load finished.** Notes-only reload. This is the new case.

### The notes-only reload

A new background job, `PreviewNotesJob`, lives in `src/ui/preview_load_job.{h,cpp}` next to the other Preview jobs. It runs on the same job base (`ResultJobBase`), so it can be cancelled and its errors get the same plain sentence. It does what the first load's notes branch does, and nothing on the audio side:

1. Read the chart container (only for .sng and .srb charts) and parse the notes with the new settings, through `app::resolve_preview_song`.
2. Work out the new song length with `app::chart_song_length_ms`. The length depends on difficulty and 2x Bass, since it takes both.
3. Check that the chart has notes in this mode, with `require_notes`, which throws the "No Hard Pro Drums notes in this chart." error.
4. Build the scene with the path the controller asked for, using the audio end it already has (`audio_end_ms_`). Then build the highway timeline with `track_options` for the new Pro Drums setting.

Steps 1-3 are exactly the first load's code: the container read at `src/ui/preview_load_job.cpp` line 123, and the parse, length and notes check at lines 158-177. **Move them into one function that both jobs call**, so the rule for "read the notes for this mode" lives in one place. Don't copy it.

The job doesn't open audio and doesn't hash the chart file. The "chart changed since it was analyzed" answer is about the file, not the mode, so the controller keeps the answer from the first load (`chart_changed_`).

### What the controller does

When the notes reload **starts**:

- Record the new key in `open_key_`, so the next frame's `open()` sees nothing new.
- Take in the path and cap `open()` was given, the same way the full reload does (`src/ui/preview_controller.cpp` lines 82-87): set `path_`, `sp_cap_`, `requested_path_key_` and `path_key_`, and set `job_path_key_` to the key the notes job builds its scene for. The re-analysis usually clears the selected path at this moment, so the new notes start with no gold path.
- Clear `error_` and `error_detail_`. An earlier "no notes in this mode" message must go away when the user switches back.
- Cancel the overlay job if one is running, and move it to `retired_scene_jobs_` as `start_scene_job()` already does. Its result belongs to the old notes and must never be swapped in.
- Cancel the base job (`base_job_`) if one is running, and retire it so the UI thread never waits on its thread. `retired_scene_jobs_` holds only `PreviewSceneJob`s (`src/ui/preview_controller.h` line 345), so the build adds a retired list for base jobs, and `busy()` counts it. Otherwise `poll()` would take the old song's base when it finishes, because it takes any finished base while `scene_base_` is empty (lines 175-178). `PreviewSceneJob` checks only a base's timeline options, not which song it came from (`src/ui/preview_load_job.cpp` lines 333-336). So the next path change would draw the new path over the old notes.
- Leave the transport, the audio device and the drawn scene alone. The old highway stays up.

While it **runs**, a path change must not start an overlay job on the old song. The guard in `open()` (`if (!job_ && song_ && !song_->is_empty()) start_scene_job();`) also has to wait for the notes job. A second mode change cancels the running notes job, retires it the same way, and starts a new one.

When it **lands** (in `poll()`):

- Take the new song, song length, scene and timeline, and set `scene_dirty_` so `render()` uploads them. Set `scene_path_key_` to `job_path_key_` and clear `job_path_key_`, as the first load does (lines 198 and 231). `shows_path()` and Show in Preview read `scene_path_key_`.
- Set `pro_` to the new Pro Drums setting. `track_opts()` and the next-activation box both read it.
- Drop `scene_base_` and reset `base_started_`. The base is the path-free scene that later path changes build on, and it belongs to the old notes. `poll()` then builds a new one as it does after a first load. The old base job was already retired at the start, so nothing from the old song can land here.
- Tell the transport where the new last note is (see below).
- If the path was changed while the job ran, start an overlay job. This is the same catch-up the first load already does at `src/ui/preview_controller.cpp` line 235.

If it **fails** (no notes in this mode, or a parse error):

- Set `error_` and `error_detail_` from the job, as the first load does. The panel then shows "Preview failed: …" exactly as today.
- Pause the transport. While `error_` is set, the panel returns early and draws no Play button (`src/ui/preview_tab.cpp` lines 291-302). Audio that kept playing behind the error would have no way to stop.
- Keep the transport loaded and the playhead where it is. Switching back to a mode that has notes runs another notes-only reload, and the user lands on the same spot, paused.

`loading()` stays tied to the first load only (`job_`), so the panel keeps the highway on screen during a notes reload instead of the loading bar. `busy()` must also count the notes job. Finding 109 made `busy()` mean "every Preview thread is done", and tests wait on it.

### The transport's end

`PreviewTransport::length_ms()` is where playback stops: the later of the last note and the end of the audio. It is set only in `load()` (`src/ui/preview_transport.cpp` line 20). The last note moves with the mode, so the transport needs a way to change it without reloading the audio. Add one method, for example `set_last_note_ms(double)`. It works out the length again by the same rule as `load()`, sharing that line rather than copying it. If the clock is now past the new end, it moves the clock back to the end, the way `seek_ms` clamps.

The scrubber's length is the song length (`scrub_end_ms()`), and it moves with the new `song_length_ms_`. The scrub marks and next-activation boxes are cached per scene, and the existing `scene_dirty_` and generation rule rebuilds them (`src/ui/preview_controller.h` lines 258-283).

### What the user sees

1. The Preview is at 1:23 on Expert, playing.
2. The user picks Hard. The music keeps playing. The Expert notes stay on the highway for the moment it takes to read Hard's notes. Then Hard's notes appear in place, still at the same moment of the song.
3. Changing the mode re-analyzes the song, as it does today. Until that finishes, Hard's notes draw with no gold path. When the analysis lands, the path appears. This part is not new: a path change already swaps in that way.
4. If the chart has no Hard notes, the Preview pauses and shows "Preview failed: No Hard … notes in this chart." Picking Expert again brings the highway back at the same spot, paused.

## Edge cases

**A big .sng chart.** A .sng file holds its audio inside it, so reading the container means reading the whole file. The notes job still reads it to get the notes and the song length, the way a full reload does today. So this is never slower than now, and it skips opening the stems. Opening the stems is most of a load's time. On the blink-182 Discography chart (a loose 625 MB song.opus, not a .sng) it took 569 ms against 55 ms for reading the notes. That was measured with a scratch harness on 2026-10-03, under load with the file already in the OS cache. The comment at `src/ui/preview_load_job.cpp` lines 22-31 gives no run count. Keeping a .sng's bytes around from the first load would save the read, but it would hold the whole file, audio included, in memory for the life of the Preview. This plan doesn't do that.

**Changing settings with another tab open.** The Preview pauses when its tab is hidden (`src/ui/details_panel.cpp` line 268), and `open()` runs only while the tab is shown. So a change made on the Paths tab reloads the notes when the user comes back to the Preview, and the paused playhead is where they left it.

**Show in Preview.** That button waits for `shows_path()` on the selected path's key before it jumps (`src/ui/preview_tab.cpp` lines 333-337). It works the same after a notes reload, because the overlay key is set the same way.

## Tests

The builder runs only these tests, by filter. Never the whole suite, and never a library run.

**Unit tests in `tests/test_preview_controller.cpp`.** The test "a difficulty change on the open chart starts a new Preview load" (lines 529-558) checks `pc.loading()` after each mode change. Under this plan `loading()` stays false for a notes reload, so that test changes. Rewrite it to check, for each of Difficulty, Pro Drums, 2x Bass and Note Shuffle:

- the position is unchanged (seek to a spot first);
- a playing transport is still playing;
- the audio was not reopened. One way to see this is to count calls to the audio device factory, which the test can install with `set_audio_device_factory`. It should be called once in all.

Add one test for the failure case: switch to a difficulty the chart lacks, and check that the error is set and the transport is paused with the same position. Then switch back, and check that the error clears and the position is the same. Add one test that a second mode change during a running notes job wins, and that the first job's result is never drawn.

**Transport test.** Add one test that `set_last_note_ms` moves `length_ms()` and pulls a clock past the new end back to it. The transport's tests live in `tests/test_preview_transport.cpp`.

**GUI test in `tests/ui/uitest_preview.cpp`.** `test_preview_mode_reload` (lines 659-694) already picks Hard and Expert on "Beg". It waits with `!pc.loading()`, which won't cover a notes reload any more, so wait on `!pc.busy()` instead. Extend it so it seeks to a spot before picking Hard and checks the position after picking Expert again. Add a check that toggling Pro Drums while playing keeps it playing near the same spot. Run it through `hydra_uitest` (see `docs/agents/ui-testing.md`).

## Docs to update

`docs/UserGuide.md` line 175 says the Preview "reloads with that mode's notes". Change it to say the notes change in place and the playhead stays. Add a line to `docs/handoffs/release-next-notes.md`. The comment on `PreviewSongKey` (`src/ui/preview_controller.h` lines 48-52) and the comment on `open()` (lines 75-89) both describe the full reload, so they change too.

## How to run it

1. **Check the claim.** Before any code, run `hydra_uitest` with a small check that seeks the Preview, changes SP cap, and reads the position back. If it resets too, stop and tell the user: something besides the song key is resetting the playhead, and this plan doesn't cover it.
2. **Build.** One Opus agent in its own worktree does all the code, tests and docs above, because the changes all touch the same few files. Its brief points at `docs/agents/brief-preamble.md` and names the status rule.
3. **Review.** A fresh Sonnet agent does the derive-once review (`docs/agents/derive-once-review.md`). Two places are most likely to end up with a second copy of a rule: the shared notes-reading function and the transport's length rule.
4. **Look.** Before merging, run the GUI test and tell the user what it showed.
5. **Merge.** Run the full suite once, then merge. The doc changes go through the doc-review gate when `main` moves.
