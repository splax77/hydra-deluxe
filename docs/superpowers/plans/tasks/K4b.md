Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task K4b: Preview loading

**Orchestrator answers (2026-10-04, binding; they override the open questions below).** K4a owns `src/app/preview_view.h` and the two call lines whose signatures it changes: the `build_preview_base` call in `preview_load_job.cpp` and the `build_scrub_marks` call in `preview_controller.cpp`. Do not edit those two lines; everything else in your files is yours. 0 of 0: the bytes slice reads 0 and the bar keeps its 8% read share (O3b's owner); the "never moves backwards" test stands. R7.20: the mapping is O3b's and tested there; you only route the poll error through `plain_error_text`. The no-notes sentence comes from C3b's `NoNotesError` ("No Hard Pro Drums notes in this chart." style); pin whatever chart the test finds.

Base: the claude/p3-d1 join commit (47eafff (claude/p3-d1)). Branch: `claude/p3-k4b`. Worktree `.claude\worktrees\p3-k4b`.

## Goal

The Preview's loader decides three things by hand: whether a chart is "the same song", how full its progress bar is, and how it words time left and errors. Wave D1 owns each of those now. This task points the controller, the load job and the transport at the owners, and makes the Preview reload when the chart mode changes (D48 Q22). Nothing here changes a score, a path or a stored record.

Sources: plan `docs/superpowers/plans/2026-10-04-phases-3-5.md` (D2 table, row K4b), drafts `docs/audit/2026-10-04-session-audit/step3a.md` (findings 20 and 69) and `step3b.md` (R7.1, R7.20, 9), D48 in `docs/audit/2026-10-03-fix-decisions.md`. Read with offset and limit.

## The findings, one by one

**20, the Preview keeps the old mode's notes.** Today `PreviewController::open` in `src/ui/preview_controller.cpp` asks `active_ && open_key_ == entry.md5`. Difficulty, Pro Drums and 2x Bass reach the load job only on a fresh open, so with a song open on the Preview tab a mode change keeps the old notes on the highway while the path, gauge and score box already come from the new mode. D48 (Q22): the Preview reloads with the new mode's notes, or says "No Hard Pro Drums notes in this chart." (the owner's no-notes sentence, which C3b made a typed error). Owner: O3b's parsed-song key type in `src/ui/preview_controller.h`, built from the same inputs `Settings::to_analysis_settings` uses (md5, difficulty, pro, bass2x). `open` compares that key instead of the md5. On screen: switching difficulty, Pro Drums or 2x Bass reloads the highway; a chart without that difficulty shows the owner's sentence where the Preview would be.

**69, a 0-of-0 progress bar.** Today `PreviewLoadJob::Progress::fraction` in `src/ui/preview_load_job.cpp` guards the zero total itself and lands on the read share (8%) when the audio total is 0. The scan dialog says full and the batch strip says empty for the same moment. D48 (Q19): empty, meaning "nothing reported yet". Owner: O3b's `progress_fraction(done, total)` in `src/ui/widgets.h`. `fraction` builds the Opening slice from it. On screen: a Preview with zero audio bytes shows an empty bar until something is reported. Keep "the bar never moves backwards" green.

**R7.1, time left.** Today `Progress::time_left_text` in `preview_load_job.cpp` words it "about N s left" or "about N min left", while the batch strip says "about m:ss left". D48 (Q20): the batch strip's form for both, and the Preview keeps its 3-second wait. Owner: O2's `time_left_text(seconds)` in `src/ui/library_parts.h`. The job's method keeps only its gate (Opening step, 3 s elapsed, a known rate) and calls the owner for the words. On screen: the loader says "about 0:40 left" where it said "about 40 s left".

**R7.20, the raw mixer error.** Today `PreviewController::poll` copies the job's error text as is, and the panel prints "Preview failed: StreamMix: ...". D48 (Q21): the audio-decode sentence. Owner: O3b's two new mappings in `src/app/user_messages.h` ("cannot read file size" and "StreamMix:"). `poll` runs the error through `plain_error_text` once, so the panel (K4a's file) prints plain text. Nothing else in the panel changes.

**9, the transport's length (no change).** `PreviewTransport::load` in `src/ui/preview_transport.cpp` takes the later of the last note and the audio end as the scrub range. D48 (Q25) keeps the audio tail playable, so this stays as it is; the position fix is K4a's. You own the file so the two tasks do not collide, and so the comment can say the length is the scrub range, not where the song ends. The test "load takes the later of last note and audio end as the length" stays green.

**The no-notes line** at `preview_load_job.cpp` ~174 is C3b's work and already on your base. Do not touch it.

## Owned files (only these may change)

`src/ui/preview_controller.cpp`, `src/ui/preview_transport.cpp`, `src/ui/preview_transport.h`, `src/ui/preview_load_job.cpp`, and the tests `tests/test_preview_controller.cpp`, `tests/test_preview_load_progress.cpp`, `tests/test_preview_transport.cpp`, `tests/ui/uitest_preview.cpp` (one new case and its table entry). Nothing else. `preview_controller.h` is O3b's (the key type); `preview_load_job.h` is nobody's in D2, so report if you need it.

## Tests to add (test first, red then green)

In `tests/test_preview_controller.cpp`:
- "a difficulty change on the open chart starts a new Preview load" — same md5, new difficulty: a new load job starts. Cover Pro Drums and 2x Bass the same way inside the case. Re-use the file's existing controller setup; the file already builds loads on a worker.

In `tests/test_preview_load_progress.cpp`:
- Re-pin "Preview load progress: a zero total gives a finite fraction": the Opening step with 0 of 0 bytes reads 0 (empty), and the rest stays finite and in [0, 1].
- Re-pin "Preview load progress: time left waits 3 s, then reads seconds or minutes" to the m:ss form on its existing inputs (for example 40 s reads "about 0:40 left", 185 s reads "about 3:05 left"); the 2.9 s and unknown-rate cases still read "". Rename the case to say m:ss.

In `tests/ui/uitest_preview.cpp`, one new case, `preview-mode-reload`: open a test chart that has Expert but not Hard on the Preview tab (the drafts used the chart they call "Beg"; check `testdata/input` for one), pick Hard in the settings bar, and the panel shows the no-notes sentence for Hard; pick Expert again and the highway comes back. Register it in the file's table.

Pin only the words and numbers above; the loader's shares (0.08 and the rest) are R7.47's and stay as they are.

## Test filters and uitest scripts you may run

`build-cpp\Release\hydra_tests.exe -sf=*test_preview_controller*`, `-sf=*test_preview_load_progress*`, `-sf=*test_preview_transport*`, `-tc="single-owner*"`. Build `hydra_uitest` and run only `--test preview`, `--test preview-load-bar`, `--test preview-error-wraps`, `--test difficulty`, `--test preview-mode-reload`.

## Scan rows

None of your own. If a D1 row (whole-ms timings, time-left words) flags a line in your files, make the line call the owner.

## Done when

A difficulty change starts a new load job. A chart without that difficulty says so in the Preview (`preview-mode-reload`). The loader's time left reads "about m:ss left". `git diff --stat` lists only the files above.

## Open questions (stop and report, do not pick)

1. **K4a may need your files for 9 and 329.** `build_scrub_marks` is called from `preview_controller.cpp` with the transport length, and `build_preview_base` is called from `build_scene_base` in `preview_load_job.cpp` before the audio opens. If K4a's fix for the marks or the beat-grid end needs a different argument at either call, that line is yours; the orchestrator decides who edits it. Do not change those calls on your own.
2. **69 and the loader's 8%.** D48 says 0 of 0 reads empty. For the Preview that means the Opening step with a zero audio total shows 0 even though the chart has been read. If the owner's `progress_fraction` is meant only for the bytes slice (so the bar still shows the read share), the two readings differ; report which the orchestrator wants before re-pinning.
3. **R7.20 unit coverage.** A StreamMix failure is hard to raise in `test_preview_controller.cpp`. The sentence itself is pinned in O3b's `test_user_messages.cpp`; your proof that `poll` maps it is the `preview-error-wraps` uitest plus a read of the text. If that is not enough for the reviewer, report rather than building a fake mixer.
4. **The no-notes sentence's difficulty.** The owner's sentence names the mode ("No Hard Pro Drums notes in this chart."). If the test chart you find has no Pro Drums markers, the wording differs; quote what the owner prints, never retype it.
