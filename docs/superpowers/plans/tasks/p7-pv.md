Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task PV: the Preview (findings 73, 126, 72 callers, 139 Preview cap floors, 130 Preview half, 109 PreviewController::busy)

Task id: PV. Base: main after M_D and M7-1 (the main session names the hash at launch). Branch: claude/p7-pv (worktree `.claude\worktrees\p7-pv`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 2 table, "PV Preview". Decisions: D51 calls 17, 18, 14 and 16 in `docs/audit/2026-10-03-fix-decisions.md` (questions 17, 18, 14 and 16 of `docs/audit/2026-10-04-phase-7-questions.md`); finding 109 is a code-only call. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 73.`, `#### 126.`, `#### 72.`, `#### 139.`, `#### 130.` and `#### 109.`.

## Goal

Three questions the Preview asks itself today: is the audio playing, does the chart file match its record, and what are the volume and the cap. After this task `PreviewTransport` owns the play state so a jump back brings the sound back (D51 call 17), the load job notices a chart that changed since it was analyzed and the panel says so in D51's words (call 18), the volume and the cap come from the Settings owner SE1 built (calls 14 and 16), the panel reads the cap from the record's key (finding 130), and one `busy()` says whether any Preview job is still running (109). Nothing stored changes. On screen only two things change: sound after a jump back, and the one new line on a changed chart.

## What the code does today

Line numbers are from `claude/p3-d2` for the Preview files and `claude/p7-w1` for `config.h`; the merged base moves them a little.

**73.** `PreviewTransport::seek_ms` (preview_transport.cpp line 63) moves the clock and the playhead but never restarts the playhead. `Playhead::read_frames` (src/audio/player.cpp) pauses the playhead itself when the audio runs out. `jump_ms`, `jump_activation` and `seek_activation` in preview_controller.cpp (lines 275, 350, 359) seek while playing. On a chart whose notes outlast its audio, play past the audio's end and jump back: the clock says playing, the playhead says stopped, the highway scrolls in silence with the play button lit.

**126.** `PreviewLoadJob::run` (preview_load_job.cpp line 142) re-reads `entry_.notespath` and never compares the file with `entry_.md5`. That md5 is the record's hyhash: `AppState` fetches the viewed record with `settings.record_key(selected->md5)`. An edited chart that was not rescanned draws the stored path on the new notes.

**72.** The default 40 is typed in `preview_controller.h` (`volume_pct_`, line 303) as well as in `config.h` (`preview_volume`, the owner). `set_volume` (controller line 303) clamps 0 to 100 itself; percent-to-gain is typed twice (`poll` line 167, `set_volume` line 305). The slider in `render_preview_panel` (preview_tab.cpp line 288) is `SliderInt` 0 to 100 with no flags, so Ctrl+click typing escapes the range. `PreviewTransport::set_gain` (line 92) floors at 0, and so does `Playhead::set_gain`, which phase 6 J1-4 (merged as M6-J1a) made the owner of that floor. SE1 added the owner on `claude/p7-w1`: `app::Settings::clamp(&Settings::preview_volume, value)` and `app::Settings::volume_gain(percent)`, both over the key table in config.cpp.

**139.** The Preview floors the cap to 1 three times: `build_unanalyzed_sp_meter_curve` and `build_sp_meter_curve` in preview_view.cpp (lines 71 and 93) and the gauge in preview_tab.cpp (`std::max(1, pc->sp_meter_cap())`, line 552). The minimum lives in config.cpp's key table (`sp_cap`, floor 1, no ceiling), reached through `Settings::clamp`. D51 call 16 keeps a 1-bar cap allowed; the Paths tab line about it is task E3's.

**130.** `render_preview_panel` (preview_tab.cpp lines 195 to 197) takes a Ready record's cap from the blob (`record->sp_cap.value_or(kCloneHeroSpCap)`). The key columns own the cap, and the viewed record's key cap is `app.settings.sp_cap` (`settings.cap_query()`); `prepare_row` already refuses a record whose cap differs from its key.

**109.** `PreviewController::loading()` (controller.h line 109) is `job_` alone. `scene_job_`, `base_job_` and the retired overlay jobs are not covered. The GUI harness's `jobs_busy` reads `loading()`, so `wait-idle` can return while an overlay is still building.

## What changes

**73.** `PreviewTransport::seek_ms` makes the playhead match the clock: when the clock is playing, the playhead is seeked and set playing again; when paused, as today. The header comment says the clock is the master and a seek re-syncs the audio to it (CONTEXT.md, Transport). `tick`'s stop-at-end rule is untouched.

**126.** The load job hashes the file with `app::hash_chart_file(entry_.notespath)` (the scan's rule, so the two can never disagree) on its worker, and compares it with `entry_.md5`. A mismatch sets a `chart_changed` flag on `Result` and builds the scene with no path, as for an unanalyzed chart. An empty hash (unreadable file) is not a mismatch; the parse error speaks for that. `PreviewController` keeps the flag (`chart_changed()`), builds every later overlay job without a path while it is set, and clears it in `close()`. The panel shows one line, in the warning colour where the "No audio device" line sits: `This chart changed since it was analyzed. Analyze it again to see its path.` The Paths tab is untouched; it reads the record. `entry_for` in `tests/test_preview_controller.cpp` gives every entry the fake md5 "prevctl"; it switches to `hash_chart_file(notespath)` so the existing cases keep their overlays.

**72.** `volume_pct_` starts from `app::Settings{}.preview_volume`; `set_volume` stores `Settings::clamp(&Settings::preview_volume, percent)` and both gain sites call `Settings::volume_gain`. The slider gets `ImGuiSliderFlags_AlwaysClamp` and writes the setting through `Settings::clamp`. Its end stops are taken from the owner by clamping the smallest and largest `int`, so no 0 or 100 is typed in the tab. `PreviewTransport::set_gain` stops flooring and remembers the gain it is given; `Playhead::set_gain` owns the floor. That takes phase 6 J3-5's one transport line early; say so in your report.

**139.** One floor, a file-local helper beside the two curve builders in preview_view.cpp that both call, reading `Settings::clamp(&Settings::sp_cap, cap)` so the smallest cap is typed once, in the key table. The gauge in the tab divides by `sp_meter_cap()` with no `std::max`.

**130.** The panel passes `app.settings.sp_cap` as the meter's cap in both branches, Ready or not, with a comment that the key columns own the cap (finding 130) and `prepare_row` makes the record's equal its key's. The blob read and its `value_or(kCloneHeroSpCap)` go; that is phase 6 347's tab line, done early.

**109.** `PreviewController::busy()`: true while `job_`, `base_job_`, `scene_job_` or any retired overlay job is unfinished. `loading()` stays, because it means "the first load's progress bar". LB points `jobs_busy` and `docs/agents/ui-testing.md` at it in wave 3.

## Owned files (only these may change)

- `src/ui/preview_transport.cpp`, `src/ui/preview_transport.h`
- `src/ui/preview_load_job.cpp`, `src/ui/preview_load_job.h` (the `Result` flag)
- `src/ui/preview_controller.cpp`, `src/ui/preview_controller.h`
- `src/ui/preview_tab.cpp`
- `src/app/preview_view.cpp` (the cap floors only; `preview_view.h` stays untouched, so keep the helper file-local)
- `tests/test_preview_transport.cpp`, `tests/test_preview_controller.cpp`, `tests/test_preview_load_progress.cpp`, `tests/test_preview_view.cpp` (the `sp meter curve` cases only)
- `tests/test_single_owner.cpp`: your own scan rows at the end of the file only; the main session joins them at M7-2.

Not yours: `src/audio/player.*` (phase 6 J1-4), `src/app/config.*` (SE1, then phase 6 J2-1 this wave), `src/ui/paths_tab.cpp` (E3 and SE2), `tests/ui/*` (LB).

## Test cases to add

Write each red first, then green. Use the files' own helpers: `make_playhead`, `make_ramp`, `entry_for`, `chart_with_audio`, `wait_finished`, `corpus::first_chart_with_suffix`, `hash_chart_file`. No new helper.

In `tests/test_preview_transport.cpp`:
1. `a seek while playing brings the audio back after it ran out` (new). Load a raw-pointer `Playhead` of 1000 ms with `last_note_ms` 2000, play, then `read_frames` past 48000 frames so the playhead pauses itself while `transport.playing()` stays true. `seek_ms(500.0)`. Pin: `playhead->playing()` is true, its position is 500 ms, and `read_frames(out, 4)` returns 4 with frame 24000's ramp values. Red line: today playing is false and 0 frames come back.

In `tests/test_preview_controller.cpp`:
2. `the Preview hides the path when the chart file changed since its record` (new). Open `chart_with_audio()` with an entry whose md5 is a wrong literal and the record's best path, as `switching paths builds the new overlay off the UI thread` builds one. Pin: `chart_changed()` true and `scrub_marks()` empty; with the real hash, `chart_changed()` false and the marks present. Red line: `chart_changed` does not exist today.
3. `busy covers the overlay and base jobs, not only the first load` (new). After a finished load, open the same chart with another path. Pin: `busy()` true while `loading()` false; poll until `busy()` is false and the overlay key is the new one.
4. `the Preview volume is the settings owner's: default and clamp` (new, needs a `volume_percent()` accessor on the controller). Pin: a fresh controller reads `app::Settings{}.preview_volume`; `set_volume(150)` reads 100 and `set_volume(-5)` reads 0 (the range the key table holds).

In `tests/test_preview_view.cpp`:
5. `sp meter curve: the cap floor is the settings owner's` (new). Both curve builders at cap 0 give `cap` equal to `Settings::clamp(&Settings::sp_cap, 0)`, which is 1 today (the value today's floors print).

Existing cases that must pass unchanged: every case in the three `-sf` filters and every `sp meter curve:` case, in particular `play seeks the playhead to the clock and both run`, `tick pauses at the end and pins the time`, `gain set before load applies to the next playhead`, `switching paths builds the new overlay off the UI thread`, `with no audio device the Preview still loads, muted, with a warning`, `sp meter curve: the bank stops at the cap`.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_preview_transport*`
- `build-cpp\Release\hydra_tests.exe -sf=*test_preview_controller*`
- `build-cpp\Release\hydra_tests.exe -sf=*test_preview_load_progress*`
- `build-cpp\Release\hydra_tests.exe -tc="sp meter curve*"`

Nothing else. Never the full suite, never `hydra_uitest --all`; the main session runs the GUI scripts at M7-3.

## Stored results

None. No record, score or path changes. The results stamp stays "2.1.0".

## Not in this task

- The Paths tab's "A 1-bar cap can never activate Star Power." line (E3) and the settings boxes' clamps (SE2).
- `Playhead::set_gain` and the audio frame pair (phase 6 J1-4); the jump constants, overlay sizes and `kOnyxDefaults` (J3-5); the other preview_view folds (J3-4).
- `jobs_busy`, `wait-idle` and `docs/agents/ui-testing.md` (LB, wave 3).
- The error line's words (ER, wave 4).

## Done when

- A seek while playing restarts the playhead; a changed chart loads with no overlay and the D51 line; the default, clamp and gain come from `app::Settings`; one cap floor in preview_view.cpp over the key table; the tab reads the key's cap; `busy()` exists.
- The five cases above pass and every other case in the four filters passes.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

**Decided (D58):** the user took the recommended answer to every question below. Treat each recommendation as the decision.

1. **Where the changed-chart line sits** (needs the user: a display choice). Recommended: one warning-colour line above the highway, where the "No audio device" line sits, with the highway still drawn below it; the whole overlay hides (activations, score box, scrub marks), and the SP gauge draws the unanalyzed fill-only curve.
2. **Where the hash runs** (code-only). Recommended: at the start of the audio branch, hidden behind the stem open, which is the slow step. Report the blink-182 Discography .sng load time before and after.
3. **The slider's end stops** (code-only). Recommended: derive them from `Settings::clamp` at the int extremes; a range accessor on the key table would be cleaner but config.cpp is phase 6 J2-1's this wave. Note it for J2-1 in your report.
4. **`busy()` and retired overlay jobs** (code-only). Recommended: count them until they finish; `wait-idle` means every thread is done.
5. **The transport's gain floor** (code-only). Recommended: remove it now, since `Playhead::set_gain` owns it on main; the alternative leaves J3-5 to do it in wave 3.

Notes on the refs: the Preview files differ between `claude/p3-d2` and main. `p3-d2` carries phase 3's D wave: the scrubber ends at the last note, `audio_end_chart_ms` as the audio-end owner, the base and overlay jobs, the time-left clock. `claude/p7-w1` does not touch any Preview file, but brings `config.h`'s `clamp` and `volume_gain` (SE1) and `core/audio_sniff` (PS1). The merged base has both; build against it, not either branch.

## Commits

One commit, trailers `Task: PV` plus the preamble's others. Report as the preamble says, plus the load-time numbers from question 2.
