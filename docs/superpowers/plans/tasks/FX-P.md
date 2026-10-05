Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task FX-P: M_D review fixes, Preview

Task id: FX-P. Base: b6ec982 (the reviewed claude/p3-d2 tip). Branch `claude/p3-fx-p`. Worktree `.claude\worktrees\p3-fx-p`.

## Goal

The M_D derive-once review found copies in the Preview. Its full text, with every file and line, is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\review-M_D.md`, section "Part: preview" (findings 1 to 7), plus finding 4 of "Part: library" (the text-width lambda). Read those first. User decisions: D56 in `docs/audit/2026-10-03-fix-decisions.md` on main (57981dd); read it in the main checkout.

## What is left

1. **Song length (preview 1).** `PreviewScene::song_length_ms` re-derives the last note's onset that `store::song_length_ms` owns. Make the scene take the store's value (one derivation), so its three readers (the two SP-curve closes in `preview_view.cpp` and the controller) and the scrubber read the same number. The beat-line fallback keeps today's result. Pin one case where the two used to be computed apart.
2. **Frames to ms (preview 2, audit 182).** `audio_end_chart_ms` in `src/ui/preview_transport.h` repeats `Playhead::length_ms`'s frames-to-ms sum. Add the one helper audit finding 182 proposes in `src/audio/` (frames and sample rate to ms) and have `Playhead::length_ms`, `audio_end_chart_ms` and any other copy the review lists call it. Read finding 182 in `docs/audit/2026-10-03-derivation-audit.md` for the copies it names; fix the ones in your owned files and list the rest.
3. **`timing.h` comment (preview 3).** It names `tick_at`, which is gone, and lists the Paths timeline as a user. Make it say what the code does now. (`path_view.cpp`'s own tick rounding is FX-L's.)
4. **`running_activation` (preview 4).** Call `struck_at` instead of writing `a.ms <= now`.
5. **Two `length_ms` (preview 5).** `PreviewController::length_ms()` now answers "where does the scrubber end", while `PreviewTransport::length_ms()` answers "how long is the audio". Rename the controller's to say what it answers (for example `scrub_end_ms()`), update its callers and the header comment, and fix the uitest "5 s jumps, clamped to the song's ends" in `tests/ui/uitest_preview.cpp` so a jump past the end is checked against the audio end. Give that uitest a chart whose audio outlasts its notes if one exists in the fixtures, so the check means something.
6. **Clock (preview 6, D56 item 3).** The clock keeps the audio length. Correct `docs/UserGuide.md` (around line 160) so the clock's total is described as the end of the audio, not "the song's last measure". If phase 4's docs test pins that line, keep it green.
7. **Drain comment (preview 7).** The comment saying the drain text and the floor teal "can never differ" is wrong (the floor is darkened by design); make it say what the code does.
8. **Text-width lambda (library 4, first half).** `src/ui/preview_tab.cpp` (around lines 135-136) carries a lambda identical to `text_width` in `src/ui/widgets.h`; call `text_width`. Do not edit `widgets.h` (FX-L owns it).

Precheck line 3 (`preview_tab.cpp` 230 alpha) is carried over unchanged; leave it.

## Owned files

`src/app/preview_view.cpp`, `src/app/preview_view.h`, `src/ui/preview_controller.cpp`, `src/ui/preview_controller.h`, `src/ui/preview_load_job.cpp`, `src/ui/preview_load_job.h`, `src/ui/preview_tab.cpp`, `src/ui/preview_transport.cpp`, `src/ui/preview_transport.h`, `src/audio/player.cpp`, `src/audio/player.h`, `src/audio/stream_mix.h` (and a new `src/audio/` helper header if needed, added to CMake only if it has a .cpp), `src/core/timing.h`, `src/store/` file holding `song_length_ms` (only if the scene needs an overload), `docs/UserGuide.md` (the clock line only), `tests/test_preview_view.cpp`, `tests/test_preview_controller.cpp`, `tests/test_preview_transport.cpp`, `tests/test_preview_load_progress.cpp`, `tests/test_audio*.cpp` or `tests/test_player*.cpp` (whichever tests the player), `tests/ui/uitest_preview.cpp`, `tests/test_single_owner.cpp` (rows for these items only).

## Tests you may run

`-sf=*test_preview_view*`, `-sf=*test_preview_controller*`, `-sf=*test_preview_transport*`, `-sf=*test_preview_load_progress*`, the player/audio test file, `-tc="scrub marks*"`, `-tc="song_fraction*"`, `-tc="single-owner*"`, `-sf=*docs_match_code*`; build `hydra_uitest` and run `--test preview`, `--test preview-activation-jumps`, `--test preview-path-overlay`, `--test preview-mode-reload`.

## Done when

Each item has one owner and a test where it is code. No score, path, record or visible-number change; the only visible change is the guide's sentence.
