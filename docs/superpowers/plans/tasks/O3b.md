Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task O3b: app and UI helper owners (wave D1)

**Orchestrator answers (2026-10-04, binding; they override the open questions below).** R7.4: no owner here. The sentence has one caller, the settings bar, so K3 writes it once in `settings_bar.cpp` ("Settings are locked while <cleaned title> analyzes."). Both-causes Stale sentence: keep today's text for that case. 0 of 0 Preview loader: as written, the loader's slice reads 0 and the bar stays at its 8% start; the "never moves backwards" test stands. Shared D2 files: noted; the integrator handles them.

Task id: O3b. Base: the `claude/p3-c` join commit (db97ea5 (claude/p3-c)). Branch: `claude/p3-<id lower case>`, so `claude/p3-o3b`.

## Goal

You add six small owners and re-point nothing but the one helper that lives in your own file. Each owner answers one question a screen asks today in two or three different ways. Wave D2 makes the screens call them. Nothing you do changes a score, a path or a stored record.

## Finding 15: a note exactly on the playhead (D48 Q27)

Today `build_score_box` in `src/app/preview_view.cpp` finds the last chord at or before the playhead, so a chord at `now` counts as hit. `build_highway_draws` in `src/render/highway_draw.cpp` flashes a gem only when its time is strictly before `now` and skips the target glow when it is not, so the same chord shows unlit.

D48 says: it counts as hit in both, so a jump lands on a struck gem.

You add `struck_at(now_ms, note_ms)` in `src/app/preview_view.h`: true when the note is at or before the playhead. Add the predicate only; leave `build_score_box` and the highway alone. K4a makes both call it in D2 (the highway works in seconds and converts). `render/track_state.h` already includes `app/preview_view.h`, so the highway can reach it.

Test case (in `tests/test_preview_view.cpp`): "struck_at: a chord exactly on the playhead counts as hit". Pin that 2500.0 against a note at 2500.0 is hit, 2499.9 is not, and 2500.1 is. The 2500 ms chord is the activation chord of `make_sp_song` in that file's existing score-box cases.

## Finding 69: a progress bar at 0 of 0 (D48 Q19)

Today `progress_bar_counted` in `src/ui/widgets.h` draws 0 of 0 as full. The batch strip in `library_dialogs.cpp` draws it empty. `PreviewLoadJob::Progress::fraction` in `preview_load_job.cpp` leaves its slice at the start.

D48 says: empty, meaning "nothing reported yet".

You add `progress_fraction(done, total)` in `src/ui/widgets.h`: the fraction done, held between 0 and 1, and 0 when the total is 0. Make `progress_bar_counted` in the same file read it. K3 points the batch strip at it and K4b points the Preview loader's slice at it. The Preview bar then still sits at its slice start (8%) while the audio total is unknown, because the owner only answers for the slice; see the open question.

Test case (in `tests/test_preview_load_progress.cpp`, which already covers the 0-of-0 question for the Preview): "progress_fraction: 0 of 0 is empty, and the fraction stays between 0 and 1". Pin (0, 0) gives 0, (1, 2) gives one half, and (200, 100) gives 1, the same more-than-total case the file's existing zero-total test uses.

## Finding 70: how a long label is cut (D48 Q18)

Today three routines cut text. `render::ellipsize` in `src/render/overlay_layout.cpp` trims the spaces before the "…" and allows an exact fit. `text_ellipsized` in `src/ui/widgets.h` hands the job to ImGui's `RenderTextEllipsis`, which keeps the space. `draw_title_ellipsized` in `src/ui/library_table.cpp` is a hand copy that also returns the kept width for the search highlight.

D48 says: the picker's rule everywhere (no trailing space, an exact fit allowed).

`render::ellipsize` stays the owner. You add an overload that also reports the kept width: the width of the text before the "…", the whole width when it all fits, and 0 when only "…" is shown. You then make `text_ellipsized` in `widgets.h` build its string through `ellipsize` with `ImGui::CalcTextSize` as the measurer and draw that string; the overflow tooltip stays. K3 replaces `draw_title_ellipsized` with the overload in D2. On screen, a cut label in the song panel and dialogs loses the space before its "…" and may keep one more character; that is the whole change.

Test cases (in `tests/test_overlay_layout.cpp`, using the file's `ten_per_char` measurer):
- "ellipsize: the kept width is the width of the text before the ellipsis". Pin, with the existing label and widths: a label that fits reports its full width; the cut at 80.0 that reads "3- 1 2…" reports 60; the cut at 15.0 that reads "…" alone reports 0; the accented cut at 40.0 that reads "Caf…" reports 30.
- The two existing `ellipsize` cases keep passing unchanged.

## Finding 13: why a result is out of date (D48 Q17)

Today the sentence is typed three ways: twice in `src/ui/details_panel.cpp` ("Out of date: this result came from another Hydra version or from different rules in hydra_rules.ini. Re-analyze to refresh it."), once as the chip hint and once as the row tooltip in `src/ui/library_table.cpp`. Each blames both causes. The store already knows the real one: `RecordLookup::stale_build` and `stale_rules` in `src/store/record_store.h`. "No paths found." is typed in `details_panel.cpp` twice and in `path_view.cpp`.

D48 says: one sentence that names the real cause, "analyzed by another Hydra version" or "different rules in hydra_rules.ini".

You add `stale_text(build, rules)` in `src/app/user_messages.h` and `.cpp`. Use today's details-panel sentence as the frame and keep only the true half: one sentence for another Hydra version, one for different rules. When both are true, return today's sentence unchanged, because that case already names both and D48 settles no new wording for it. Put "No paths found." beside it as a named constant. K2 and K3 make the panel, the chip, the tooltip and `build_record_status` call these in D2.

Test case (in `tests/test_user_messages.cpp`): "user_messages: stale_text names the real cause". Pin the build-only sentence, the rules-only sentence, the both-causes sentence (today's text), and the "No paths found." constant.

## Finding R7.20: two error messages (D48 Q21)

Today `plain_error_text` in `src/app/user_messages.cpp` has no entry for "cannot read file size: " (thrown in `core/winstr.cpp`), so an Analyze that hits it reads "Something went wrong.". It has none for "StreamMix: " (thrown in `audio/stream_mix.cpp`), so the Preview prints the raw text. It still matches "mix_stems:", but `mix_stems` has no caller in `src/` outside its own file; only tests use it.

D48 says: the file-size failure reads like the other missing-file errors, and the mixer failure reads as the audio-decode sentence.

You map "cannot read file size: " to the existing missing-file sentence and "StreamMix: " to the existing audio-decode sentence, and delete the dead "mix_stems:" match. K4b makes `PreviewController::poll` carry the plain text and K4a stops `preview_tab.cpp` printing the raw one.

Test cases (in `tests/test_user_messages.cpp`): extend "user_messages: a missing or unreadable song file" with the file-size text; add "user_messages: a Preview mixer failure reads as an audio-decode problem" pinning the audio-decode sentence for "StreamMix: invalid output format"; and pin that "mix_stems: data converter init failed" now falls back to the generic sentence.

## Finding 20: what the Preview calls the same song (D48 Q22)

Today `PreviewController::open` in `src/ui/preview_controller.cpp` compares `open_key_`, a string holding only the md5. A difficulty, Pro Drums or 2x Bass change therefore keeps the old notes on the highway while the path and score box come from the new mode.

D48 says: the Preview reloads with the new mode's notes, or says that difficulty has no notes.

You add the key type in `src/ui/preview_controller.h` only: a small value with the md5, the difficulty, the Pro Drums flag and the 2x Bass flag, with equality. Those are the four inputs `Settings::to_analysis_settings` in `src/app/config.h` already uses (`difficulty()`, `view_prodrums`, `effective_bass2x()`). Leave `open_key_` and `open` as they are; K4b switches them in D2 and adds the reload test.

Test case (in `tests/test_preview_controller.cpp`): "the Preview's song key: another difficulty, Pro Drums or 2x Bass makes a different song". Two keys with the same four inputs are equal; flipping any one of the last three makes them differ.

## Owned files (only these may change)

`src/app/preview_view.h` (the predicate only), `src/ui/widgets.h`, `src/render/overlay_layout.h`, `src/render/overlay_layout.cpp`, `src/app/user_messages.h`, `src/app/user_messages.cpp`, `src/ui/preview_controller.h` (the key type only), `tests/test_preview_view.cpp`, `tests/test_preview_load_progress.cpp`, `tests/test_overlay_layout.cpp`, `tests/test_user_messages.cpp`, `tests/test_preview_controller.cpp`, `tests/test_single_owner.cpp` (your two rows only).

Wave C's C3b touched `user_messages.cpp` (the no-notes match) and C4a touched `preview_view.h`; both are in your base.

## Tests you may run

`build-cpp\Release\hydra_tests.exe` with: `-tc="struck_at*"`, `-tc="score box: before the first note*"`, `-tc="progress_fraction*"`, `-tc="Preview load progress: a zero total*"`, `-tc="ellipsize*"`, `-sf=*test_user_messages*`, `-tc="the Preview's song key*"`, `-tc="single-owner*"`. No `hydra_uitest` script; this task has no screen.

## Scan rows (tests/test_single_owner.cpp)

Row 1. Question: "Why is a stored result out of date?" Owner: `stale_text` in `src/app/user_messages.cpp`. It must flag `hint("Analyzed by another Hydra version, or under different rules in "` (library_table.cpp). It must not flag `} else if (status == store::RecordStatus::Stale) {`. Known copies, with `removed_by`: the two `details_panel.cpp` sentences (K2), the chip hint and the row tooltip in `library_table.cpp` (K3).

Row 2. Question: "How is a long label cut to fit its space?" Owner: `ellipsize` in `src/render/overlay_layout.cpp`. It must flag `font->RenderChar(draw, size, ImVec2(IM_TRUNC(pos.x + kept_w), pos.y), col, font->EllipsisChar);` (library_table.cpp). It must not flag `text_ellipsized(text.c_str());`. Known copies: the `draw_title_ellipsized` lines in `library_table.cpp` (K3). The `RenderTextEllipsis` line in `widgets.h` goes in this task, so it is not a known copy.

The other four owners get no row: a playhead comparison, a division, an error prefix and a struct have no spelling a line scan can tell from ordinary code.

## Done when

Each owner has one test per answer, red first then green. `text_ellipsized` cuts through `render::ellipsize`. The two scan rows pass with their known copies listed. `git diff --stat` shows only the files above.

## Open questions

- R7.4 (the settings bar saying "this song analyzes" after you move to another song) is listed under D48 Q17 and under K3, but the plan's D1 table gives it no owner function. K3 owns `settings_bar.cpp` and K2 owns the `details_panel.cpp` tooltip that already names the song, so the sentence would end up typed in two D2 tasks. If one owner is wanted, it belongs beside `stale_text` in `user_messages` and should be added to this brief; I did not add it.
- The both-causes Stale sentence: D48 names the two single-cause phrases but not a sentence for a row that is stale for both reasons. This brief keeps today's text for that case. Say if you want it shortened.
- "Empty at 0 of 0" for the Preview loader: the owner makes the loader's slice read 0, so the Preview bar stays at its 8% slice start while the audio total is unknown, as today. If "empty" was meant to be the whole Preview bar at 0, that contradicts the "never moves backwards" test and needs a call.
- `tests/test_preview_view.cpp` is this task's file for the `struck_at` case in D1. O2's brief puts its clock case in a new `tests/test_display_format.cpp`, so there is no clash this wave.
- D2 files shared by hunk, from the D2 briefs as written: `src/ui/library_jobs.cpp` (K1a's throw, K3's rest), `src/app/user_messages.cpp` (K1a's no-records mapping, on top of this task's D1 work), `tests/test_cli.cpp` (K1a, K1b and K5, one group of cases each) and `tests/test_report.cpp` (K1a, and one case for K1b). Each brief names its hunk, but the plan's "each file owned once" promise does not hold for these four. The integrator should merge them in a fixed order.
