Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task FX2-P: M_D review round 2 fixes, Preview

Task id: FX2-P. Base: fc55f8b. Branch `claude/p3-fx2-p`. Worktree `.claude\worktrees\p3-fx2-p`.

## Goal

Round 2 of the M_D review: `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\06ef47ec-7105-4611-b41f-4db00557e4a1\scratchpad\p3\review-M_D-r2.md`, section "Part: preview" (findings A to F), plus findings 3 and 8 of "Part: library". Read them first. User decisions: D56 and D57 in `docs/audit/2026-10-03-fix-decisions.md` on main.

## What is left

1. **`timing.h` comment (preview A; reports 4a).** It says `path_view.cpp` still rounds the tick itself; it now calls `display_tick_at_ms`. List the Paths timeline end as a caller.
2. **"Song's end" comments (preview B).** `preview_view.h` (around 251, 257), `preview_tab.cpp` (around 375-376) and `preview_view.cpp` (around 455) call the time box's end the song's end; say where playback ends (D56 item 3).
3. **Transport length reads (preview C).** `preview_controller.cpp` (around 270, 286, 312) reads `transport_.length_ms()` directly beside `playback_end_ms()`; route them through the accessor.
4. **Two chart ends in `build_preview_base` (preview D).** The SP curve uses the store's length and the beat grid the last drawn note's tick. Leave the code; add a comment saying why the grid needs a tick and that the two agree on every parsed song, and point at the test that pins it.
5. **Gauge alpha (preview E).** Name the 230 alpha in `preview_tab.cpp` (around line 561) as a constant beside `kStarPowerColor` in `src/ui/theme.h`, value unchanged.
6. **uitest thresholds (preview F).** Replace 12000.0 and 16000.0 in `tests/ui/uitest_preview.cpp` (around 239 and 377) with one named constant for the fixture chart's length floor, at least as large as the jump target the test uses.
7. **Progress percent (library 3).** The "%.0f%%" progress overlay is typed in `src/ui/details_panel.cpp` (around 199) and `src/ui/preview_tab.cpp` (around 227). Add one `progress_bar_percent` (or the name the review proposes) in `src/ui/widgets.h` beside `progress_bar_counted`; both call it. Text on screen unchanged.
8. **Cymbal lane (library 8).** `src/render/track_state.cpp` (around line 43) writes `lane != PreviewLane::Red` where `allows_cymbals` answers it; call `allows_cymbals`.

## Owned files

`src/core/timing.h`, `src/app/preview_view.h`, `src/app/preview_view.cpp`, `src/ui/preview_tab.cpp`, `src/ui/preview_controller.cpp`, `src/ui/preview_controller.h`, `src/ui/theme.h`, `src/ui/widgets.h` (the progress helper only), `src/ui/details_panel.cpp` (the progress overlay only), `src/render/track_state.cpp`, `tests/ui/uitest_preview.cpp`, `tests/test_preview_view.cpp`, `tests/test_preview_controller.cpp`, `tests/test_track_state.cpp`, `tests/test_single_owner.cpp` (your rows only).

## Tests you may run

`-sf=*test_preview_view*`, `-sf=*test_preview_controller*`, `-sf=*test_track_state*`, `-tc="single-owner*"`; build `hydra_uitest` and run `--test preview`, `--test preview-activation-jumps`, `--test preview-load-bar`, `--test panel-headline`.

## Done when

Each item has one owner and a test where it is code. No score, path, record or visible change.
