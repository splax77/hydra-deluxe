Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task TM: the meter-change side (finding 58)

Task id: TM. Base: main after M_D and M7-1 (the main session names the hash at launch). Branch: claude/p7-tm (worktree `.claude\worktrees\p7-tm`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 2 table, "TM Meter-change side". Decision: D51 call 22 in `docs/audit/2026-10-03-fix-decisions.md` (question 22 of `docs/audit/2026-10-04-phase-7-questions.md`). Finding text: `docs/audit/2026-10-03-derivation-audit.md`, heading `#### 58.`.

## Goal

One question: which time signature rules on the exact tick of a meter change? `MeasureIndex::section_at` in `src/core/timing.cpp` already answers it for the engine: a tick sitting on a change belongs to the section before it. `SongTiming::measures_at_tick_f` answers it again on its own and picks the other side. After this task `measures_at_tick_f` reads the section through `section_at`, so the SP gauge in the Preview measures that tick the way the engine does and its drain line loses its kink (D51 call 22). Nothing stored moves.

## What the code does today

`MeasureIndex::section_at` (timing.cpp, about line 109) is a `lower_bound` minus one: on the change tick it returns the earlier section. `Timecode::Timecode` (about line 118) reads it, and the engine measures every Star Power length through `Timecode` and `SongTiming::plusmeasure`, so the engine's side is "earlier section".

`SongTiming::measures_at_tick_f` (about line 206) scans the meter sections itself with `keys_at(i) > ticks` and so reads the change tick as the new section; its comment says "right-continuous" on purpose. With the meter map {0: 1920, 2400: 1440} from the finding, tick 2400 reads 1.3333 here and 1.25 through `Timecode`; 2399 and 2401 agree either way. `tick_at_measures_f` (about line 217) is its inverse: the last section whose entry position, measured in that section's own meter, sits at or before the wanted measure position.

Readers, checked at `claude/p3-d2`: `bars_left` in `build_sp_meter_curve` (`src/app/preview_view.cpp`, about line 112) takes the difference of two `measures_at_tick_f` calls on whole ticks, which is the gauge's drain line. `SongTiming::sp_end_ms` calls both functions and, as the header comment on that branch says, only tests call it. `frontend_transfer_scales` (`src/core/squeeze_rating.cpp`) does not call either, so stored transfer scales cannot move. `ms_per_measure_at` (about line 197) already calls `section_at`, on `ticks + 1`, and says why in its comment; it is not part of this finding.

## What changes

Owner: `MeasureIndex::section_at`, `src/core/timing.cpp` and `.h`.

`measures_at_tick_f` stops scanning and asks `section_at` for its section, so a position exactly on a change belongs to the section before it, the engine's side. For a fractional position between two whole ticks the position is past the change, so the whole tick at or after the position is the one to ask about; say so in one comment, and note that this replaces the "right-continuous" sentence with D51 call 22. The section's start and ticks-per-measure then give the same slope formula as today, so every tick off a change keeps its value.

`tick_at_measures_f` stays the inverse. Its entry rule (each section's entry position in its own meter) already maps both 1.25 and 1.3333 back to tick 2400 on the finding's map, so it needs no change; check the round trip at the change tick and one tick either side, and say in your report whether you changed it. The `timing.h` comment above the two functions says which side the change tick falls on and names `section_at` as the owner.

The gauge's own test, `sp meter curve*`, is PV's filter in this wave, and `preview_view.cpp` is PV's file; you do not run or edit it. The main session runs the full suite at M7-2.

## Owned files (only these may change)

- `src/core/timing.cpp`
- `src/core/timing.h` (comments only)
- `tests/test_timing.cpp`
- `tests/test_single_owner.cpp`: your own scan rows at the end of the file only (a meter-section scan by `keys_at` outside `section_at`).

Notes on the base. `timing.cpp`, `timing.h` and `test_timing.cpp` changed only on `claude/p3-d2`, which added `SongTiming::display_tick_at_ms` and its case `timing: display_tick_at_ms rounds to the nearest tick and never goes below 0`; `claude/p7-w1` left all three alone. The two measure-position functions are the same text on both branches and on main.

## Test cases to add (in `tests/test_timing.cpp`)

Build the maps inline the way `timing: a meter change off a barline carries a partial measure` does; no new helper.

1. `timing: a tick on a mid-measure meter change reads the engine's side (D51 Q22)`. Meter map {0: 1920, 2400: 1440}, any flat tempo. Pin `measures_at_tick_f(2400.0) == 1.25` exactly, and that it equals `timecode(2400).measures_decimal()` (the engine's reading; comparing against production is allowed, retyping its formula is not). Pin 2399.0 and 2401.0 to the values your red run prints for them (today's code; the finding's table has them rounded as 1.24948 and 1.33403), since those do not move. Red line: 1.3333... at 2400 today.
2. `timing: tick_at_measures_f stays the inverse across a mid-measure change`. Same map. Pin `tick_at_measures_f(measures_at_tick_f(t)) == t` for t in 2399.0, 2400.0, 2401.0, with doctest's `Approx` at the epsilon the file already uses (1e-9). Red first only if case 1's change breaks it; otherwise record that it was green from the start.

Existing cases that must pass unchanged: `timing: a meter change off a barline carries a partial measure` (it pins `section_at(2880) == 0`, the owner's side), `timing: ms_per_measure_at reads local measure durations`, `timing: continuous helpers are exact, inverse, and monotone` (its `sp_end_ms` continuity check across 57600 has no meter change there, so it holds), `timing: display_tick_at_ms rounds to the nearest tick and never goes below 0`.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_timing*`
- `build-cpp\Release\hydra_tests.exe -tc="single-owner*"`

Nothing else. Never the full suite.

## Stored results

None. The engine never calls `measures_at_tick_f`, `frontend_transfer_scales` does not either, and `sp_end_ms` is test-only, so scores, paths, transfer scales and records stay byte-identical. The stamp stays "2.1.0". On screen only the Preview's SP gauge changes, and only during the one tick of a mid-measure meter change inside an SP window.

## Not in this task

- The gauge code and its test (`preview_view.cpp`, `sp meter curve*`: PV now, J3-4 later).
- `ms_per_measure_at`'s `ticks + 1` read (not a copy; it documents its own reason).
- Finding 57's ms-versus-tick split in the Preview's time box (phase 3 D, done).

## Done when

- `measures_at_tick_f` has no section scan of its own and reads `section_at`; the header comment names the side and the owner.
- The two new cases pass, with case 1's red line recorded; every other `-sf=*test_timing*` case passes unchanged.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

1. **A fractional position just past the change (code-only).** `section_at` takes a whole tick, and the continuous function takes a double. Recommended: a position strictly between tick 2400 and 2401 is past the change and reads the new section, so the whole tick at or after the position is the one asked about; exactly 2400.0 reads the earlier section. This keeps every value off the change tick as it is today. The only caller passes whole ticks, so no screen can show the difference.
2. **Does `tick_at_measures_f` change (code-only)?** Recommended: no, unless case 2 goes red; its entry rule already returns 2400 for both readings of the change tick. Say which happened.

## Commits

One commit, trailers `Task: TM` plus the preamble's others. Report as the preamble says.
