Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task PR1: probe helpers (findings 78, 79, 80, 81, 82, 83, 84, 85, 174)

Task id: PR1. Base: main at 81a2519. Branch: claude/p7-pr1 (worktree `.claude\worktrees\p7-pr1`, made as the preamble says). Python only: no build.

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 1 table, "PR1 Probe helpers". Decisions: the "Probe scripts" code-only calls at the end of `docs/audit/2026-10-04-phase-7-questions.md`, approved by D51. Test limits (plan): the 5 s stall, the 150 ms start lead, the 2 ms play_chart lead, the 1e-6 ms window tolerance. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 78.` to `#### 85.` and `#### 174.`.

## Goal

The Clone Hero probe runners each answer five small questions on their own: when has the song clock reached a target, has the song stopped, which notes are already too late when a run starts mid-song, how far from the note did a hit land, and has the engine's window field changed. This task gives each one owner in `tools/ch_probe/experiments/live.py`, points `walk_edges.py`, `active_probe.py` and `watch_window.py` at them, and deletes the six old diagnostics whose only remaining job was to answer those questions differently. PR2 then points `play_chart.py` at the same owners, so name them clearly in your report. Nothing here touches Hydra's app or any stored result.

## What the code does today

`wait_until` inside `main` in `walk_edges.py` (around lines 224 to 243) and `wait_until` inside `drive_inputs` in `active_probe.py` (around lines 141 to 159) are the same loop: read `SongClock`, stop if the clock jumped back more than a second, stop if it has not changed for more than 5.0 s, fire when the estimate reaches the target. `watch_window.py`'s `main` (around lines 273 to 303) is a sampler, not a wait loop, but it carries the same two "has the song stopped" checks with its own `STALL_S = 8.0` (line 45). `InputDriver.schedule_hit` in `input_driver.py` (lines 210 to 229) is a fourth loop with a 30 s timeout and a `tap`, and only `tests/test_input_driver.py` calls it. (79)

`walk_edges.main` (line 216) skips notes under clock + 150 ms; `active_probe.drive_inputs` (line 161) keeps pairs strictly above clock + 150 ms. (80)

`Row.measured_ms` in `walk_edges.py` (lines 128 to 131) with the lines that fill `engine_ms` (257 to 259), and `drive_inputs` (176 to 179), both say: the engine's stored hit time minus the note when that field changed, else the estimated send time minus the note. (81)

`summarize` in `walk_edges.py` (lines 158 to 175, the mixed hits-and-misses branch) brackets the edge between the widest hit and the narrowest miss and gives up on overlap. `analysis.find_window_edge` (line 66) owns the edge: fewest-errors threshold, median on ties, with an error count. (82)

`changes` in `watch_window.py` (line 64) and its live loop (lines 293 to 297) test a step over `1e-6` ms three times by hand. (84)

`hit_detect.py` presses the 2x kick on O (`0x4F`, line 80) and holds chords 5 ms; `DEFAULT_BINDINGS` has no 2x entry and `press_chord` holds 3 ms. (85) `find_engine.py`, `find_clock3.py`, `poll_windows.py` and `hit_detect.py` each pick the live engine their own way and type `0x4000000` and raw field offsets. (78, 174) `test_attach.py` attaches without turning kill-on-exit off. (83) `tests/test_engine_finder.py` line 58 subtracts `0x30` for `C.OFF_BACK_WINDOW`; `tests/test_hit_window_scripts.py` line 44 packs `0x1000` for `C.PRECISION_MODE_BIT`. (174)

## What changes

**Delete six scripts**, all under `tools/ch_probe/experiments/`: `hit_detect.py`, `find_clock3.py`, `poll_windows.py`, `find_engine.py`, `milestone2.py`, `test_attach.py`. That closes 78, 83 and 174's script half, and hit_detect's halves of 81, 84 and 85. Nothing in `tools/`, `tests/`, `docs/agents/` or `CONTEXT.md` imports them except one: `tests/test_s2_window_constants.py` imports `poll_windows.window_verdict` (three cases in `PollVerdictTest`). Move `window_verdict` as it is into `watch_window.py`, the script that replaced poll_windows, and repoint that test. The two CSVs in `experiments/results/` stay: `poll_windows.csv` is the measurement `test_s2_window_constants.py` reads, and `hit_detect.csv` is the evidence finding 81 cites. Fix the two prose references: `watch_window.py`'s docstring line 3 ("This is poll_windows.py plus the song clock") and `README.md`'s `experiments/` bullet (lines 60 to 61), which should now name the runners that exist. `engine_finder.py`'s "moved here from experiments/find_engine.py" is history and may stay.

**One wait loop in `live.py`.** A `wait_until` that takes a `SongClock` (anything with `read()` returning raw and estimate seconds), a target in ms, and a `lead_ms` that defaults to 0 (the edge runners fire at the target; play_chart will pass its 2 ms). It fires when the estimate reaches target minus lead and returns the raw and estimated times in ms, as walk_edges' does. It stops with an error when the clock has not changed for more than `STALL_S = 5.0` (D51), and with a *different* error when the clock jumps back by more than the one-second figure every loop uses today, kept as a named constant, because PR2's play_chart re-syncs on a jump back where the edge runners stop. It takes `now` and `sleep` callables with `time.perf_counter` and `time.sleep` defaults, like `SongClock` and `find_live_engine` already do, so the test needs no real waiting. It also takes an optional `should_stop` callable for active_probe's thread event. `walk_edges` and `active_probe` delete their nested loops and call it. `watch_window` is a sampler, so it does not call `wait_until`; it calls the helper's "has the song stopped" check (the frozen and jump-back tests, split out so both can use it) and drops its own `STALL_S`. Note this under your report: watch_window's 8 s becomes 5 s, per D51.

**`schedule_hit` goes.** Nothing live calls it, it fires by `tap` rather than `press_chord`, and keeping it leaves a second wait loop. Remove it from `input_driver.py` with its `poll_interval_s` and `timeout_s` knobs and docstring lines, from the `InputDriver` Protocol in `interfaces.py`, and remove `TestScheduleHit` from `tests/test_input_driver.py`. Every other test there stays.

**One start cursor in `live.py`**: `first_note_index(notes_ms, clock_ms)` returns the index of the first note at least `START_LEAD_MS = 150.0` ms ahead of the clock (D51: skip notes less than 150 ms ahead). `walk_edges` and `active_probe` call it; active_probe passes its pairs' first-kick times.

**One hit offset in `live.py`**: `hit_offset_ms(note_ms, sent_ms, hit_time_before_s, hit_time_after_s)` returns the offset and whether it came from the engine. `walk_edges` fills `Row` from it (the `Row` columns stay for the CSV) and `drive_inputs` calls it in place of its inline copy.

**`summarize` reads the owner.** Its mixed branch calls `analysis.find_window_edge` on each side's (measured, hit) rows and prints the edge with its error count. The overlap sentence stays as a note when the narrowest miss is at or below the widest hit. The all-hit and all-miss replay hints stay as they are.

**One window tolerance.** `constants.py` gains `WINDOW_CHANGE_TOLERANCE_MS = 1e-6` (D51: watch_window's), with a comment saying what it is for. `live.py` gains `window_changed(a, b)`; `watch_window.changes` and the three hand tests in its live loop call it (the hit-time comparison asks the same question of another double, so it calls it too). After this, constants.py's claim that nothing else hard-codes an address (lines 6 to 7) is true; `MODULE_SPAN` in `engine_finder.py` is the one home of the span.

**The 2x kick stays on L** (D51: either pedal hits either kick). With hit_detect gone nothing sends O. `DEFAULT_BINDINGS` gains no 2x entry; its comment (line 57) records the game's bind screen and may add one clause saying ch_probe presses L for the 2x kick.

## Owned files

Everything under `tools/ch_probe/` except `experiments/play_chart.py` and `tests/test_play_chart.py` (PR2's). Expected to change: `experiments/live.py`, `walk_edges.py`, `active_probe.py`, `watch_window.py`, `constants.py`, `input_driver.py`, `interfaces.py`, `README.md`, `tests/test_hit_window_scripts.py`, `tests/test_engine_finder.py`, `tests/test_input_driver.py`, `tests/test_s2_window_constants.py`; deleted: the six scripts.

## Test cases to add (in `tests/test_hit_window_scripts.py`, beside `LiveSnapshotTest`)

Write each red first. Use fake `now`/`sleep` and the fake-clock pattern of `test_song_clock_fills_in_after_a_fresh_change`; no new helper that exists elsewhere under `tests/`.

1. `test_wait_until_fires_at_the_target_or_its_lead`: with lead 0 it returns on the first read whose estimate reaches the target; with `lead_ms=2.0` it returns 2 ms earlier. Pins 0 and 2.0 only.
2. `test_wait_until_stops_after_five_seconds_frozen`: a clock that never changes, with `now` stepping past `live.STALL_S`, raises; one that steps 4.9 s does not. Assert `live.STALL_S == 5.0`.
3. `test_wait_until_stops_differently_on_a_jump_back`: a clock that falls by more than the named jump-back constant raises the jump-back error, not the stall error. Read the constant from `live`; type no number.
4. `test_first_note_index_skips_notes_less_than_150ms_ahead`: notes at 100, 149, 150 and 200 ms with the clock at 0 give index 2; assert `live.START_LEAD_MS == 150.0`.
5. `test_hit_offset_prefers_the_engine_hit_time`: finding 81's example, note 10000 ms, hit time 10.000 to 10.010 s, gives +10.0 from the engine; an unchanged hit time gives send minus note.
6. `test_window_changed_uses_the_one_tolerance`: 85.0 to 85.0005 ms is a change, 85.0 to 85.0 is not; assert `C.WINDOW_CHANGE_TOLERANCE_MS == 1e-6`.
7. `test_old_diagnostics_are_gone`: the six paths do not exist, and `walk_edges`, `active_probe`, `watch_window` and `input_driver` have no `wait_until`, `schedule_hit` or `STALL_S` of their own (`hasattr`, like `test_live_keeps_no_offsets_or_finder_of_its_own`).

Re-pin: `test_summary_brackets_the_edge` asserts the edge line now carries the figure `find_window_edge` gives for those rows with 0 errors (run once, pin the printed line); `test_summary_flags_overlap_and_all_miss` asserts the overlap note still appears and the all-miss hint still reads "early edge is below 80 ms". Fix line 44 to `C.PRECISION_MODE_BIT` and `tests/test_engine_finder.py` line 58 to `C.OFF_BACK_WINDOW`. Repoint `PollVerdictTest` at `watch_window.window_verdict`.

## Tests you may run

From the worktree root: `python -m pytest tools/ch_probe/tests/test_hit_window_scripts.py tools/ch_probe/tests/test_engine_finder.py tools/ch_probe/tests/test_input_driver.py tools/ch_probe/tests/test_runners.py tools/ch_probe/tests/test_analysis.py tools/ch_probe/tests/test_debugger.py tools/ch_probe/tests/test_s2_window_constants.py -q`. Run `tools/ch_probe/tests/test_play_chart.py` once at the end as a smoke check that nothing you removed was play_chart's; do not edit it. Nothing else.

## Not in this task

- `play_chart.py` and its test are PR2's; it forks from your commit. Do not edit them even where they repeat a rule you just owned.
- `engine_finder.find_live_engine`, `analysis.find_window_edge`, `SongClock` and `press_chord` are the owners and stay as they are.
- `passive_probe.py`, `milestone1.py`, `pad_flash_test.py`, `key_delivery_test.py` stay; none repeats these rules.

## Done when

- No `wait_until`, start-cursor comparison, hit-offset formula, `5.0`/`8.0` stall, `150` lead or `1e-6` literal remains outside `live.py` and `constants.py` in `tools/ch_probe` (play_chart excepted until PR2). The six scripts are gone and nothing imports them.
- The cases above are green and every other case in the files named passes.
- `git diff --stat 81a2519..HEAD` lists only files under `tools/ch_probe/`, none of them play_chart's.

## Open questions

- `schedule_hit` is deleted rather than repointed, on this brief's call (nothing live uses it). Say at launch if it should instead call `live.wait_until` and keep its tests.
- `watch_window` is a sampler, not a wait loop; it shares only the "has the song stopped" half. If the main session wants it left on its own 8 s, say so; D51 reads as one 5 s limit.
- The one-second jump-back figure is today's value in all four loops, named but not newly pinned as a number; test 3 reads it from `live`.
- `README.md` describes play_chart's route (lines 21 to 34); PR2 changes how play_chart reads notes, and README is yours in this wave. Leave the paragraph true as it stands; the main session adds PR2's sentence at M7-1 (see PR2's brief).

## Commits

One commit, trailers `Task: PR1` plus the preamble's others. Report as the preamble says, and name `wait_until`, the stopped-check, the two error types, `first_note_index`, `hit_offset_ms`, `window_changed` and the two constants by their exact names, because PR2's brief refers to them as "PR1's".
