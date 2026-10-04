Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task PR2: play_chart reads Hydra (finding 59, and play_chart's share of 79, 80 and 85)

Task id: PR2. Base: **PR1's commit, given at launch** (not main). Branch: claude/p7-pr2 (worktree `.claude\worktrees\p7-pr2`, made as the preamble says, from that commit). Python only: no build.

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 1 table, "PR2 play_chart reads Hydra". Decisions: the "Probe scripts" and "play_chart.py" code-only calls at the end of `docs/audit/2026-10-04-phase-7-questions.md`, approved by D51. Test limits (plan): the 150 ms start lead and the 2 ms play_chart lead. Finding text: `docs/audit/2026-10-03-derivation-audit.md`, heading `#### 59.`, with 79, 80 and 85 for the shared loops.

The plan row writes the files as `tools/ch_probe/play_chart.py` and `tests/test_play_chart.py`. They live at `tools/ch_probe/experiments/play_chart.py` and `tools/ch_probe/tests/test_play_chart.py`; those are the owned paths.

## Goal

"Which notes does this chart hold, in which lane, and when" has one owner in Hydra: the parser and timing behind `hydra_replay dump`. The live auto-player answers it again with its own .chart parser, MIDI reader and tempo walk, and finding 59 shows them disagreeing on tom markers, disco flip, flams, sysex bytes and more (282 of 772 ticks on one chart). After this task play_chart plays exactly the chords Hydra analyzed, from a dump file, and its waiting, start cursor and hit press come from PR1's owners. Its timing at the game does not change (D51).

## What the code does today

`play_chart.py` defines `parse_chart` (around line 89) with `chart_notes_to_lanes` (line 50) and the `.chart` note tables, `parse_midi` (line 182) with `_read_vlq` (line 173), `midi_notes_to_lanes` (line 153) and the MIDI note tables, `ticks_to_seconds` (line 290), the `TempoEvent`/`NoteEvent` dataclasses and a default `SYNOVIAL_DIR`. `main` (line 306) takes a chart folder, prefers notes.chart over notes.mid, then: builds its start cursor by keeping notes from 50 ms *behind* the clock (line 362, repeated in the jump-back re-sync at line 399); polls the raw engine clock and fires at `target - 0.002` (line 409); treats a clock move of 1 ms or less as frozen and stops after 5 s (lines 420 to 425); re-syncs when the clock falls by more than a second (line 396); presses with `InputDriver.press_chord`, reads the score 5 ms later, and prints the row.

`hydra_replay dump --chart <file> --db <path>` (tools/replay.cpp, usage around line 99; the JSON around lines 359 to 392) writes one object per chord with `tick`, `ms`, `chord_code`, `notes` and more. `ms` is the chart's own time: `src/core/replay.cpp` line 105 takes it from the song's tempo map (`timecode.ms()`), and Hydra keeps the `.chart` Offset apart in `Song::chart_offset_s`, applied only by the Preview (`preview_source.cpp`), never to note times; song.ini's delay is read only there too. So the dump's `ms` carries no delay and no Offset, the same as play_chart's own times today, and the playing does not shift.

## What changes

**play_chart reads a dump file.** Its one positional argument becomes the path of a `hydra_replay dump` JSON, replacing the chart folder and the `SYNOVIAL_DIR` default. It does not run `hydra_replay` itself, because that needs a built `hydra_replay.exe` (an `EXCLUDE_FROM_ALL` target) and a database path, while a JSON file is a plain input the test can fabricate and the user already makes for the FC-video work. One reader, `load_dump_notes(path)`, returns the chords in order as (ms, lanes) from `chords[].ms` and `chords[].chord_code`. The parsers, the VLQ reader, the two lane tables, `ticks_to_seconds`, the dataclasses and `SYNOVIAL_DIR` are deleted. The module docstring and usage line say the new input.

**One code-to-lane table.** `lanes_for_code(code)` reads ADR 0015's five-character code, one lane per position in kick, red, yellow, blue, green order: "." is empty; any letter is a note, upper case meaning a cymbal on yellow, blue or green and a 2x kick on the kick. Kick in either case presses `Lane.KICK` (L; D51, 85). Red is `Lane.RED`. Yellow, blue and green press their `*_CYMBAL` lane when upper case and the pad lane otherwise. The letter itself (n, g, a: normal, ghost, accent) never changes the lane. This is the only lane rule left in the file.

**PR1's owners replace the loops.** `main` builds `SongClock(engine.song_clock, max_fill_s=0.0)` so the clock stays the raw reading play_chart fires on today (a zero fill-in makes the estimate equal the raw), and calls PR1's `live.wait_until` with `lead_ms=PRESS_LEAD_MS`, a named module constant of 2.0 (D51: play_chart keeps its 2 ms early press, so its playing doesn't change). The frozen-clock stop is the helper's: an unchanged value for `live.STALL_S` seconds; play_chart's "moved 1 ms or less counts as frozen" goes. On the helper's jump-back error play_chart keeps its re-sync: it rebuilds the cursor and carries on, where the edge runners stop. The start cursor at both places becomes `live.first_note_index(notes_ms, clock_ms)`. The press stays `press_chord`, the 5 ms score wait and the row printing stay as they are.

## Owned files (only these may change)

- `tools/ch_probe/experiments/play_chart.py`
- `tools/ch_probe/tests/test_play_chart.py`

Not yours: `live.py`, `input_driver.py`, `README.md` and everything else under `tools/ch_probe` (PR1's, already committed when you fork), `tools/replay.cpp` (T1's).

## Test cases (in `tools/ch_probe/tests/test_play_chart.py`)

Write each red first. The file's `_CHART` fixture and `_parse` go with the parser.

1. New `test_dump_chords_become_timed_lanes` replaces `test_each_tick_gets_one_lane_per_gem` and `test_times_follow_the_tempo`. It writes a small dump JSON to a temp dir (only the keys the reader uses: a `chords` list with `ms` and `chord_code`) and pins the (ms, lanes) list the reader returns. Mirror the old rows: a yellow tom, a yellow cymbal, a blue cymbal, a green cymbal, a 2x kick (upper-case kick letter) as `[Lane.KICK]`, red plus kick, and a green accent as the plain green lane; times as written in the JSON, unchanged.
2. `test_lane_helper` stays and calls `lanes_for_code`. Pins: ".nN.N" gives red, yellow cymbal and green cymbal (ADR 0015's own example); "N...." and "n...." both give `[Lane.KICK]`; "...n." gives blue; "....." gives nothing.
3. `test_no_private_copies_are_left` stays and its name list grows: `parse_chart`, `parse_midi`, `_read_vlq`, `midi_notes_to_lanes`, `chart_notes_to_lanes`, `ticks_to_seconds`, `TempoEvent`, `CHART_DRUM_NOTES`, `DRUM_NOTES`, `SYNOVIAL_DIR`, plus `wait_until`, `first_note_index` and `STALL_S` (play_chart must not define its own).
4. New `test_press_lead_is_two_ms`: `play_chart.PRESS_LEAD_MS == 2.0`.
5. New `test_waiting_and_the_start_cursor_come_from_live`: the names play_chart binds for the wait and the start cursor are the very objects in `live` (`is`), the pattern of `test_lane_names_come_from_the_key_table`, which also stays.

The only numbers you may type are 2.0 and the JSON times and lanes in test 1. No new tolerance, depth or band.

## Tests you may run

From the worktree root: `python -m pytest tools/ch_probe/tests/test_play_chart.py -q`. Nothing else; PR1 already ran the other probe files on the commit you fork from.

## Not in this task

- `hydra_replay`'s settings and output (103, 198, 68, 38, 98) are T1's. The dump's `ms` stays what it is.
- The shared helpers themselves are PR1's; if one is missing a seam you need (for example the jump-back error is not told apart from the stall), stop and report rather than adding a copy.
- `README.md` is PR1's in this wave; see Open questions.

## Done when

- play_chart has no parser, MIDI reader, lane-number table or tempo math; it reads a dump and maps `chord_code` through one table.
- It fires through `live.wait_until` with `PRESS_LEAD_MS = 2.0`, stops through `live.STALL_S`, starts through `live.first_note_index`, and presses the 2x kick on L.
- The five cases are green. `git diff --stat <PR1 commit>..HEAD` lists only the two owned files.

## Open questions

- Mid-song start changes: today play_chart keeps notes up to 50 ms behind the clock; with the one rule it skips notes less than 150 ms ahead. The main session confirmed this as D51's code-only call (one start-cursor rule). Its full-song playing is unchanged, since the first note is always more than 150 ms ahead when the run starts before the song.
- Stall detection changes in a small way: a clock creeping in steps of 1 ms or less counted as frozen before and counts as moving now, under the shared rule. Code-only; no decision needed unless the main session objects.
- The dump's 2x kick depends on the dump's bass2x setting (T1 makes `hydra_replay` read the app's settings): when 2x is off, Hydra drops the 2x kick and play_chart will not press it, which is "exactly the notes Hydra analyzed".
- `README.md`'s play_chart paragraph (lines 21 to 34) should say it reads a `hydra_replay dump`. README is PR1's; recommended: the main session adds that sentence at M7-1.

## Commits

One commit, trailers `Task: PR2` plus the preamble's others. Report as the preamble says: owner, file, one sentence per change, each case's red line and green result, the diff file list against PR1's commit.
