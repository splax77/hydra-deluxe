Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task K4a: the Preview view

**Orchestrator answers (2026-10-04, binding; they override the open questions below).** Q1 (finding 9, the scrubber's tail): PENDING the user's answer. Leave the scrubber's range and its marks exactly as they are today (do not touch `build_scrub_marks` or its call) and do everything else; a small follow-up applies the answer. Q2 (finding 329): you own `src/app/preview_view.h` and the one `build_preview_base` call line in `src/ui/preview_load_job.cpp` (K4b owns the rest of that file and was told so); pass the audio length in there so the beat lines reach the audio end. Q3: the drain box text reads the floor's own teal, `cfg.hydra.sp_active_color` from `src/render/preview_config`, so the two can never differ; no new colour constant. Q4: in `tests/ui/uitest_details.cpp` touch only `cap-switch`. Note: the next-activation box now pins "[Red snare]" (the D1 join re-pinned it); when you pass the Pro Drums flag through `note_label`, re-pin to whatever the flag gives.

Base: the claude/p3-d1 join commit (47eafff (claude/p3-d1)). Branch: `claude/p3-k4a`. Worktree `.claude\worktrees\p3-k4a`.

## Goal

The Preview tab's time box, scrubber marks, beat grid, colours, struck-chord rule and gauge cap each work out a fact by hand that wave D1 or wave C now owns. This task points the Preview at those owners and lands the Preview answers D48 settled (Q22 to Q27). Nothing here changes a score, a path or a stored record.

Sources: plan `docs/superpowers/plans/2026-10-04-phases-3-5.md` (D2 table, row K4a), drafts `docs/audit/2026-10-04-session-audit/step3a.md` and `step3b.md`, D48 in `docs/audit/2026-10-03-fix-decisions.md`, the questions file, and `step45.md` for the one comment (115). Read with offset and limit.

## The findings, one by one

**57, the time box half a tick before a tempo change.** Today `tick_at` in `src/app/preview_view.cpp` rounds the ms index's tick itself, and `build_time_box` looks the tempo up by ms while it looks the meter and section up by that rounded tick. For up to half a tick before a change the BPM line shows the old tempo while the other lines show the new values. D48 (Q23): all four switch together. Owner: O3a's `SongTiming::display_tick_at_ms` in `src/core/timing.h`. `tick_at` calls it, and the tempo lookup searches `scene.tempos` by tick the way the signature lookup does. On screen: one tempo with its meter in that half-tick window; nothing else moves.

**318, 0:60.000.** Today the local `clock_str` takes the minute first and prints the remainder with `%06.3f`, so 59,999.6 ms reads 0:60.000. D48 (Q20): it reads 1:00.000. Owner: O2's clock formatter in `src/app/display_format.h` (O2 lands it under the name the D1 table gives, `clock_str`; use that and delete the local copy). On screen: the clock never shows 60 seconds.

**15, a chord exactly on the playhead.** Today `build_score_box` counts a chord at `now_ms` as hit (`upper_bound` on `v < s.ms`), while `build_highway_draws` in `src/render/highway_draw.cpp` lights the flash only when `it->t < now_s` and skips the glow when `it->t >= now_s`. After "< Act" or "Act >" the box counts the chord while the highway shows it unlit. D48 (Q27): hit in both. Owner: O3b's `struck_at(now, note_ms)` predicate in `src/app/preview_view.h` (inclusive). The score box and both highway tests call it. The highway works in seconds; convert at the call, never add a second predicate. On screen: a jump lands on a lit gem.

**9, where the song ends.** Today `build_scrub_marks` divides each activation's ms by the length its caller passes, which is the transport length (last note or audio end, whichever is later). The Paths timeline divides by the last note. So an activation sits further right on the Paths timeline than on the scrubber whenever the music outlasts the notes. D48 (Q25): both bars measure position against the last note; the scrubber still plays the audio tail. Owner: C4a's song-fraction helper (the one `path_view.cpp` uses) over `scene.song_length_ms`. See the open question on the scrubber's tail and on the function's signature. On screen: an activation lines up on both bars (H1's audio-tail fixture).

**329, where the beat lines end.** Today `build_preview_base` builds beats to `last_tick + 2 * tpm`, two measures past the last note, while the transport keeps running. D48 (Q25): the beat lines run to the end of the audio. The base is built before this file knows the audio length; see the open question. On screen: beat lines keep scrolling through the audio tail.

**308, the gauge cap with no result.** Today `render_preview_panel` in `src/ui/preview_tab.cpp` falls back to `kCloneHeroSpCap` when the viewed record is not Ready. D48 (Q24): it uses `Settings::sp_cap`, the cap the next analysis will run at. Owner: the setting itself; no new function. On screen: an unanalyzed song's gauge pins at your cap, not at 4.

**16 and 136, colours.** Today `preview_tab.cpp` types the gold `IM_COL32(255, 204, 51, ...)` four times: the next-activation header, the "SP" label, the gauge fill and the drain box accent. The highway floor tints teal (`sp_active_color`) while SP runs, so the floor and the drain text disagree. The scrubber marks already use `kBestPathColor`. D48 (Q26): teal means "SP running" everywhere, so the drain box text turns teal like the floor; activation marks and the next-activation header both use the best-path gold. Owner: `theme.h` names from C4b (the SP gold's name) plus `kBestPathColor`. The header reads `kBestPathColor`; the "SP" label and gauge fill read C4b's SP gold name, values unchanged; the drain accent reads the teal (see the open question on which named teal). On screen: the drain box text goes teal while SP runs; the header goes best-path gold.

**17, note names in the box.** Today the next-activation box's chord text comes from `Chord::rowstr`, which spells "GreenTom". D48 (Q11): the Dynamics wording ("Green tom", "Yellow cymbal", "2x kick", plain "Yellow" with Pro Drums off). Owner: O1's `note_label` in `src/core/model.h`, which O1 already routes `rowstr` through. Your work here is the test pins in `test_preview_view.cpp` that quote the old spelling. On screen: the box reads "Green tom".

**R7.20, the raw error.** Today `preview_tab.cpp` prints `"Preview failed: %s"` with the controller's raw text, so a mixer failure shows "StreamMix: ...". D48 (Q21): the audio-decode sentence. K4b makes `PreviewController::poll` carry the plain text through O3b's `plain_error_text`. Your side: the panel prints what the controller carries and adds nothing of its own. If K4b's shape needs nothing here, leave the line.

**115, a comment (phase 4 finding, yours because you own the file).** The comment above the section loop in `build_preview_base` ("the ms index does not reach") is wrong. Per `step45.md`: the ms index covers every tick (`MsIndex::at` extrapolates) and `timecode(t).ms()` is the same call. Verify both claims in `src/core/timing.*` before rewording, then say that in two short sentences.

## Owned files (only these may change)

`src/app/preview_view.cpp`, `src/ui/preview_tab.cpp`, `src/render/highway_draw.cpp`, `src/ui/theme.h` (SP colour use only), and the tests `tests/test_preview_view.cpp`, `tests/test_highway_draw.cpp`, `tests/ui/uitest_details.cpp` (the `cap-switch` test only). Nothing else. `src/app/preview_view.h` is O3b's (the predicate) and C4a's (the stale comment); report if you need it.

## Tests to add (test first, red then green)

In `tests/test_preview_view.cpp`:
- "build_time_box: inside the half tick before a tempo change every line shows the new values" — a playhead less than half a tick before an existing fixture's tempo change; tempo and meter agree. Use the fixture's own change; invent no new ms.
- "build_time_box: the timestamp rounds 59,999.6 ms to 1:00.000" — the plan's number.
- "scrub marks: an activation sits at the Paths timeline's fraction when the audio outlasts the notes" — H1's chart whose audio runs 5 s past its last note; the mark equals `build_activations`' `song_fraction` for the same activation.
- "build_preview_scene: the beat lines run to the end of the audio" — re-pin the 4320 case in "build_preview_scene fills beats, tempos and resolution" to the audio end (see open question 2 first).
- "score box: a chord exactly on the playhead counts as hit".
- Re-pin the Burnout chord strings that quote "GreenTom"-style names to the Dynamics wording.

In `tests/test_highway_draw.cpp`:
- "build_highway_draws: a chord exactly on the playhead is lit" — flash and glow both present at `t == now`.

In `tests/ui/uitest_details.cpp`, `cap-switch` gains one step: set the SP cap to a value other than 4 in the settings bar, open an unanalyzed song on the Preview tab, and the gauge readout names that cap.

Pin only the numbers above. 59,999.6 and the 5 s tail are the plan's; everything else comes from existing fixtures.

## Test filters and uitest scripts you may run

`build-cpp\Release\hydra_tests.exe -sf=*test_preview_view*`, `-sf=*test_highway_draw*`, `-sf=*test_theme*`, `-tc="single-owner*"`. Build `hydra_uitest` and run only `--test preview`, `--test preview-drain-box`, `--test preview-activation-jumps`, `--test preview-error-wraps`, `--test preview-path-overlay`, `--test cap-switch`. Not `test_preview_golden`: the floor tint does not change.

## Scan rows

None of your own. The "what colour means SP running" row lands with C4b/O-owners; your files must pass it (no gold or teal literal left in `preview_tab.cpp`). Fix a flagged line by reading the named colour; never add an exemption.

## Done when

The half-tick case shows one tempo. The beat lines reach the audio end. Activations sit at the same fraction on both bars (H1's audio-tail fixture). The jumped-to chord is lit. `git diff --stat` lists only the files above.

## Open questions (stop and report, do not pick)

1. **9, the scrubber's tail.** If marks sit at `ms / last note` while the slider's range stays the audio length, the playhead no longer meets a mark at the activation. D48 says both bars measure against the last note and the tail stays playable, but not how the slider shows the tail (a second, unmarked region past the last note, or the marks mapped onto the longer range). Report before drawing anything. Also, `build_scrub_marks(scene, length_ms)` is declared in `preview_view.h` (not yours) and called from `preview_controller.cpp` (K4b) with the transport length; dropping the parameter touches both.
2. **329 needs the audio length inside `build_preview_base(song)`.** That function is declared in `preview_view.h` and called from `build_scene_base` in `src/ui/preview_load_job.cpp` (K4b) before the audio is opened. Reaching the audio end means a signature change in a file you do not own and a call-site change in K4b's file, or building the beats later in K4b's code. The plan's ownership leaves this gap; the orchestrator should assign it.
3. **Which teal for the drain box.** The floor's teal is `cfg.hydra.sp_active_color` from `src/render/preview_config.*` and `assets/preview/3d-config.json`; `theme.h` has the button teals. D48 says "teal like the floor" but not which named value the drain text reads. If C4b named one for this, use it; otherwise report.
4. **Shared test file.** `tests/ui/uitest_details.cpp` also holds the Stale-state check K2 may touch and the Preview tests K4b adds; K4b's new case goes in `uitest_preview.cpp` to avoid a clash. Say so in your report if you touch anything in that file beyond `cap-switch`.
