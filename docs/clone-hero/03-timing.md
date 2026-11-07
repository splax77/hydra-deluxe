# Timing [TM]

This page covers chart Offset and song.ini delay, the song clock, the hit window, calibration and playback speed.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-10-30.

## Offset and delay

### TM-01 The song.ini delay is read as whole milliseconds with no clamp

Rule: The game reads `delay` from the `[song]` section of `song.ini` as a whole number of milliseconds, default 0, with no clamp, into the song metadata object at `#828+0x24`.
Label: Proven in the game's code
Evidence: `code/6142/ini_song_reader_1EFC60.asm` lines 38 to 47; the class fields are in `dump.cs` lines 45110 to 45125.
Game code (6142): the song.ini reader of class #828 (scrambled name), RVA `0x1EFC60`, `code/6142/ini_song_reader_1EFC60.asm` lines 38 to 47.
Hydra: `ini_delay_ms` in src/app/preview_source.cpp, which reads the key as text and hands it to `parse_delay_ms` (line 113).
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-02 A delay with a decimal point is read as seconds

Rule: When the `delay` text contains a ".", the game reads it as a decimal number of seconds, times 1000 and cut toward zero, so `delay = 0.5` is 500 ms. Text without a "." is a plain integer, and unreadable text gives 0.
Label: Proven in the game's code
Evidence: `code/6142/ini_int_getter_21647A0.asm` lines 32 to 66: the `String.Contains(".")` test, Int32.TryParse at line 42, Single.TryParse at line 59, the multiply by 1000.0 at line 63, the truncating convert at line 64 and the default return at line 66. The same getter serves only the `delay` key in the reader at `0x1EFC60`.
Game code (6142): an ini getter (scrambled name), RVA `0x21647A0`, `code/6142/ini_int_getter_21647A0.asm` lines 32 to 66.
Hydra: `parse_delay_ms` in src/app/preview_source.cpp line 113 calls `parse_finite_number`, so "0.5" becomes 0.5 ms. No owner reads a decimal as seconds.
Ruling: open split: see questions page Q-30
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-03 The chart Offset counts only when the delay is still 0, rounded up to whole milliseconds

Rule: The load coroutine reads `song.ini` first and the chart second. The chart's `Offset` is used only when the metadata delay is still 0, and then the delay becomes ceil(Offset x 1000) whole milliseconds. A nonzero delay wins, and `delay = 0` does not block the Offset.
Label: Proven in the game's code
Evidence: `code/6142/chart_song_metadata_35C980.asm` lines 87, 103, 562 to 586 and 634 to 663 (the skip when `+0x24` is nonzero at line 636, Single.TryParse at line 651, the multiply at line 659, the call to the C runtime's ceiling helper at line 661, the store at line 663); that helper jumps to a one-instruction round toward plus infinity. The order is in `code/6142/gm_load_coroutine_D8D80.asm` lines 272 to 291. The three measured points of the 2026-09-24 recording (TM-05) all agree with this reading; their scripts and result files were not kept.
Game code (6142): `SongEntry` chart metadata reader (scrambled member), RVA `0x35C980`, and the load coroutine `GameManager` (scrambled `MoveNext`), RVA `0xD8D80`, listings as above.
Hydra: `preview_audio_offset_ms` in src/app/preview_source.cpp lines 311 to 316 makes the same choice (the delay wins when present and nonzero, else Offset x 1000), with the Offset parsed into `chart_offset_s` at src/parse/song.cpp line 1825. Hydra keeps fractional milliseconds and does not round up.
Ruling: The choice is ruled: D274 item 6, "Check the game, then honor (Recommended)" for the Preview's audio delay and Offset. The rounding up is not named in it: open split: see questions page Q-31
Found: 2026-10-30, session b02ab3e6 (the reference build); the measurements 2026-09-25, session 2a88c065.

### TM-04 A delay equal to the Offset makes no difference

Rule: A `song.ini` delay exactly equal to the chart Offset changes nothing, because both routes store the same milliseconds in the same field and the clock reads that one field.
Label: Proven in the game's code
Evidence: Follows from TM-01 and TM-03: one field, `#828+0x24`, one clock argument (`code/6142/ini_song_reader_1EFC60.asm` lines 38 to 47, `code/6142/chart_song_metadata_35C980.asm` lines 634 to 663). This edge is not the sharp case anyway: the readings "delay replaces Offset" and "the larger wins" agree there. This settles the reserved question Q-73 (the delay-equals-Offset edge) on the game's side.
Game code (6142): RVAs `0x1EFC60` and `0x35C980`, listings as above.
Hydra: `preview_audio_offset_ms` in src/app/preview_source.cpp line 311.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-05 A positive delay makes the notes come later against the audio

Rule: The clock reads audio position / ratio plus (-delay / ratio), so audio position `delay` is chart time 0. A positive delay makes the notes come later against the music. Delay and Offset change only this mapping, never the chart's own times.
Label: Proven in the game's code
Evidence: `code/6142/clock_ctor_16C9A0.asm` lines 22 to 32 (`+0x58` = -(b / c)), `code/6142/clock_tick_16C650.asm` lines 72 to 75, `code/6142/gm_load_coroutine_D8D80.asm` lines 435 to 464 and `code/6142/delay_seconds_1EF1A0.asm` lines 2 to 4. The 2026-09-24 recording (`delay = 1016` measured +1018 ms, `Offset = 0.25` measured +248 ms, and a copy with `Offset = 0.25` and `delay = 500` measured +496 ms) agrees; its scripts were not kept. The three numbers are written in the comments at src/app/preview_source.cpp line 138 and tests/test_preview_source.cpp lines 302 and 305. Session 2a88c065's card (docs/sessions/2026-09-25-2a88c065.md) records that a subagent measured the note shift against the audio in the user's video.
Game code (6142): the clock constructor of class #707, RVA `0x16C9A0`; the delay-in-seconds getter of class #828, RVA `0x1EF1A0`.
Hydra: `kOffsetDirection` in src/app/preview_source.cpp line 139.
Ruling: D274 item 6: "Check the game, then honor (Recommended)".
Found: 2026-10-30, session b02ab3e6 (the reference build); the recording 2026-09-25, session 2a88c065.

### TM-06 A .sng's delay follows the same rule; a .srb has none

Rule: A `.sng` file's metadata `delay` is read by a second reader and follows the same "delay wins when nonzero" rule. A `.srb` has no delay field, so only the chart Offset counts.
Label: Unknown
Evidence: not checked: it needs the body of the reader at `0x1EFF90`, of which only the key string `delay` was seen, at 0x1EFFDD and 0x1F0041, in a call scan of 2026-10-30 whose output was not kept; no listing of `0x1EFF90` is under `code/6142/`. The `.srb` half is Hydra's reading of its own parser: src/parse/srb.h has no delay.
Game code (6142): not found: the reader `0x1EFF90` and the branch at `0x1F0380` were seen in a call scan only.
Hydra: `sng_metadata_delay_ms` (src/app/preview_source.cpp line 182) and `chart_audio_offset_ms` (line 358).
Ruling: D275 item 4: the user chose "Yes, same rule (Recommended)" for `.sng` and `.srb` charts.
Found: 2026-10-30, session b02ab3e6 (the reference build); the ruling 2026-09-26, session 4908325e.

### TM-07 The delay and Offset move only the audio, never the chart's own times

Rule: Hydra converts chart time to audio time in one place, audio = chart + offset, and scoring and the search never read the offset. This matches the game, where the delay changes only the clock's audio mapping (TM-05).
Label: Hydra assumes it, never checked
Evidence: `preview_audio_offset_ms` and its callers `chart_audio_offset_ms`, `song_length_ms` and the Preview mix; a grep of `src/` for `delay_ms`, `chart_offset_s` and `offset_ms` finds no other user.
Game code (6142): see TM-05.
Hydra: `preview_audio_offset_ms` and `kOffsetDirection` in src/app/preview_source.cpp lines 311 and 139.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-08 A second delay getter divides by 1582

Rule: A getter of class #828 at `0x1F0830` divides the delay by 1582.0 instead of 1000.0. It looks unused.
Label: Unknown
Evidence: a 4-instruction read of the getter on 2026-10-30, whose output was not kept; no listing of `0x1F0830` is under `code/6142/`. A direct-call scan of the code section found no caller. Virtual and delegate calls were not searched.
Game code (6142): RVA `0x1F0830` (scrambled name); no committed listing.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The song clock

### TM-09 The judged clock is the raw clock minus the audio calibration

Rule: The clock the game judges against is the raw clock minus the audio calibration. The time used for drawing is the raw clock plus (video minus audio).
Label: Proven in the game's code
Evidence: `code/6142/clock_raw_minus_audio_16C480.asm` lines 2 to 4 (load `+0x38`, subtract `+0x48`, return) and `code/6142/clock_raw_plus_video_16C490.asm` lines 2 to 4 (load `+0x50`, add `+0x38`, return); the bytes match the overlay handoff.
Game code (6142): two getters of class #707 (scrambled names), RVAs `0x16C480` and `0x16C490`.
Hydra: no owner: Hydra compares in chart time (TM-24).
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-10 GameManager stores both clocks every frame

Rule: `GameManager.Update` stores the first getter's result as `GameManager.songTime` (+0x70) and the second as `GameManager.songTimeVideoTime` (+0x78) every frame, and the player passes `songTime` to the engine, which writes it to the engine clock.
Label: Unknown
Evidence: listing lost: read 2026-10-28 in session f756d547 and re-read from the DLL on 2026-10-30 (the calls at 0xC085B and 0xC0877, the stores at 0xC0867 and 0xC0880), but no listing was kept. The player hop was not re-checked.
Game code (6142): `GameManager.Update`, RVA `0xC05A0`; fields from `dump.cs` lines 25298 to 25305.
Hydra: no owner: the Preview and Paths use chart time and never read the game clock.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547.

### TM-11 The raw clock starts at least one second before chart time 0

Rule: Let d be the delay divided by the ratio, in seconds. The raw clock starts at -1.0 s when d is 1 s or less, and at -d - 1.0 s when d is more than 1 s. So it always starts at least one second before chart time 0, and a delay of more than 1 s moves the start back by the whole delay plus one second.
Label: Proven in the game's code
Evidence: `code/6142/clock_ctor_16C9A0.asm` lines 25 to 38: line 25 negates b / c (stored at `+0x58` on line 32), line 27 compares -1.0 with it, and the branch picks -1.0 (line 38) or `+0x58` minus 1.0 (line 34). A working note of 2026-10-30, not kept, wrote this as min(-1.0, -d - 1.0), which differs from the listing for d between 0 and 1 s; the listing's own note reads it the way this block does.
Game code (6142): the clock constructor of class #707, RVA `0x16C9A0`.
Hydra: no owner: Hydra has no count-in model.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-12 The raw clock runs on the frame timer and is pulled toward the audio

Rule: Each frame the raw clock advances by deltaTime x practice speed / ratio. While the audio clock is inside (lead-in, length - 1 s) and playing, a gap above 0.003 s between the two is spread over later frames.
Label: Proven in the game's code
Evidence: `code/6142/clock_tick_16C650.asm` lines 19 to 141, with the 0.003 constant at lines 105 and 116. `GameManager.Update` calls it at 0xC0839.
Game code (6142): the clock tick of class #707 (scrambled name), RVA `0x16C650`.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-13 The engine's song clock is the double at engine+0x100, in seconds

Rule: The drum engine holds the song clock as a double in seconds at `#8288+0x100`.
Label: Proven in a live run
Evidence: `tools/ch_probe/experiments/results/overlay-spike/run1_cal0_sp_watch.csv`, column `clock_s`: 40,922 rows rising from 2.207 s to 181.160 s; `tools/ch_probe/experiments/results/hit_detect.csv` and the 2026-10-28 key-press run's `presses.csv` read the same offset. The early name "hit time" for this field was wrong: it is the clock. What the long at `+0x108` holds is FL-27's question.
Game code (6142): field `#8288+0x100` (struct #8496, first member, `dump.cs` line 540408).
Hydra: no owner in `src/`; the tool name is `OFF_SONG_CLOCK` in tools/ch_probe/constants.py line 98.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-09-25, session 8157f2dc.

### TM-14 The double at engine+0x2E0 is not a hit time

Rule: The engine does not copy the song clock into `#8255+0x2E0` at each hit, as the tools assumed. It writes the clock there only when a hit completes a double lane's round of pads, so the field is the round's start. Page 07's IN-12 holds the rule and its evidence.
Label: Proven in the game's code
Evidence: the only two writes are at lines 140 and 188 of `docs/audit/drum-lanes/play/f20deaa0.asm`, each behind the double-lane flag test 0xC000 (lines 131 and 179), and the lane update reads the field as the round's start (NL-59, IN-15). The field read 0.0 in all 40,922 rows of the bot run `run1_cal0_sp_watch.csv`, which fits a chart with no double lane.
Game code (6142): field `#8255+0x2E0` (`dump.cs` lines 527435 to 527436); writer in the engine method at RVA `0x20DEAA0`, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 140 and 188.
Hydra: no owner in `src/`; the tool name is `OFF_HIT_TIME` in tools/ch_probe/constants.py line 106.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-15 Calibration does not move the engine clock against the chart

Rule: With a bot, neither a 30 ms audio calibration nor a 30 ms video calibration moved the engine clock against the chart's own times. Star Power phrase and end events came at the same engine times as the 0 ms run, within a few ms.
Label: Proven in a live run
Evidence: `tools/ch_probe/experiments/results/overlay-spike/run1_cal0_sp_compare.txt`, `run2_cal30_sp_compare.txt` and `run2b_vid30_sp_compare.txt` (the first 8 phrase rises PASS in all three files; the two 30 ms runs ended early, at 137.8 s and 118.0 s of the 0 ms run's 181.2 s, so only run 1 holds the later rises, and run 2's two "missing" rises fall after its end), and the `*_score_compare.txt` press medians -0.42, -0.45 and -0.45 ms. Trippin SP Spike, bot, 2026-10-28. The press medians alone prove nothing, because the bot presses from the same clock.
Game code (6142): see TM-09.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547.

### TM-16 About 3 ms of input polling delay

Rule: The game's on-screen average of +2.5 ms, against clock reads of about -0.5 ms, points to about 3 ms of input polling delay.
Label: Unknown
Evidence: the +2.5 ms is from a user screenshot that is not in the repo, and the 3 ms is the overlay handoff's own inference.
Game code (6142): not found: not looked for.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547.

## The hit window

### TM-17 The drums engine always uses the neighbour-gap window

Rule: The drums engine constructor sets the byte at `#8255+0x41` to 1 in both normal and precision mode, and the per-note window routine takes the neighbour-gap branch when that byte is nonzero.
Label: Proven in the game's code
Evidence: `code/6142/engine_ctor_20DF680.asm` line 78 (`mov byte ptr [rbx + 0x41], 1`) and `code/6142/note_window_20F3A90.asm` lines 4 and 9.
Game code (6142): the drums engine constructor, RVA `0x20DF680`; the per-note window routine of class #8288 (scrambled name), RVA `0x20F3A90`.
Hydra: no owner for a per-note window: Hydra holds one fixed number, `kHitWindowMs` in src/core/model.h line 104.
Ruling: open split: see questions page Q-32
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-18 The constructor sets the front and back clamps by the precision bit

Rule: With bit 0x1000 of the engine flags at `#8288+0x198` clear, the constructor stores a front of 0.0375 s at `+0x38` and a back of 0.085 s at `+0x30`. With the bit set, the front is 0.025 s and the back 0.04 s. The whole window at `+0x20` starts at twice the back value.
Label: Proven in the game's code
Evidence: `code/6142/engine_ctor_20DF680.asm` lines 78 to 91 (the four constants read at lines 80, 81, 83 and 84, the stores at lines 85, 87 and 91). The decompile `ghidra_decompile_all.txt` lines 1807 to 1822 agree. The four constants are `RVA_CONST_NORMAL_FRONT`, `RVA_CONST_NORMAL_BACK`, `RVA_CONST_PRECISION_FRONT` and `RVA_CONST_PRECISION_BACK` in tools/ch_probe/constants.py. `dump.cs` line 547692 names the bit `PrecisionMode = 4096`.
Game code (6142): the drums engine constructor, RVA `0x20DF680`, `code/6142/engine_ctor_20DF680.asm` lines 78 to 91.
Hydra: no owner. `kHitWindowMs` (src/core/model.h line 104) is a separate number that equals the normal back value.
Ruling: open split: see questions page Q-32
Found: 2026-09-25, session 8157f2dc; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-19 Each half-gap is held between the front and the back

Rule: For each side of a note, the game halves the gap to the neighbour and clamps it between the front and the back. A missing neighbour, or one at the same time, gives the back value.
Label: Proven in the game's code
Evidence: `code/6142/half_gap_clamp_20F7210.asm` lines 13 to 22 (null or zero gap exit), 23 to 33 (the half-gap), 51 to 62 (min and max) and 72 to 77 (a missing neighbour returns the back); `code/6142/note_window_20F3A90.asm` lines 38 to 53 pass the front as the minimum and the back as the maximum. The decompile lines 53871 to 53907 agree.
Game code (6142): the clamp of class #8288 (scrambled name), RVA `0x20F7210`, `code/6142/half_gap_clamp_20F7210.asm` lines 13 to 77.
Hydra: no owner (see TM-17).
Ruling: open split: see questions page Q-32
Found: 2026-10-03, session cebf3006; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-20 The two half-gaps feed the formula, and the note gets half its output

Rule: The per-note routine adds the two clamped half-gaps, passes the sum to the engine's virtual slot 19, which on the drums engine is the window formula, and returns half the formula's value as the note's half-window.
Label: Proven in the game's code
Evidence: `code/6142/note_window_20F3A90.asm` lines 38 to 65; `dump.cs` line 527484 names `0x20DDDA0` as `Slot: 19` of class #8255. An earlier pass of 2026-10-30 labelled this Unknown because the 2026-10-03 listing was lost; the committed listing above replaces it, so the code label holds. The slot-to-offset step (slot 19 at vtable offset 0x268) was not checked in this build, but the live run in TM-23 matches the formula exactly.
Game code (6142): RVA `0x20F3A90`, `code/6142/note_window_20F3A90.asm` lines 38 to 65.
Hydra: no owner (see TM-17).
Ruling: open split: see questions page Q-32
Found: 2026-10-03, session cebf3006; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-21 The normal-mode window formula

Rule: With x the summed gap in ms, the whole window in ms is (x x 0.0110924369747899 - x^2 x 2.628539e-05) x 170 - 20. The function has no clamp of its own.
Label: Proven in the game's code
Evidence: `code/6142/hit_window_formula_20DDDA0.asm` lines 11 and 12 (the bit test and x = gap x 1000.0) and 17 to 35 (the constants, decoded in the listing's comments). The decompile lines 757 to 782 agree. It gives 171.4313075 ms at x = 170, 96.2931672 ms at x = 75 and 102.2586 ms at x = 80.
Game code (6142): the formula of class #8255 (scrambled name), RVA `0x20DDDA0`, `code/6142/hit_window_formula_20DDDA0.asm` lines 11 to 35.
Hydra: no owner (see TM-17).
Ruling: open split: see questions page Q-32
Found: 2026-09-25, session 28e6f29d; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-22 A note's window runs from 48.15 to 85.72 ms a side; Hydra uses a fixed 85

Rule: In normal mode the formula's input is held between 75 and 170 ms, so the whole window runs from 96.2932 to 171.4313 ms, and each side gets half: from 48.15 ms when both neighbours are close to 85.72 ms when both are far. A note whose neighbours are both 80 ms away gets 102.26 ms, 51.13 ms a side. Hydra judges every note at a fixed 85 ms a side, which is the game's back clamp for one half-gap, not its per-side window.
Label: Proven in the game's code
Evidence: TM-19 to TM-21 by arithmetic. Hydra's side: `kHitWindowMs` (src/core/model.h line 104) is read by the badges, Ratings, timing tiers and the search (TM-29).
Game code (6142): RVAs `0x20F3A90`, `0x20F7210` and `0x20DDDA0`, the three listings above.
Hydra: `kHitWindowMs` in src/core/model.h line 104.
Ruling: open split: see questions page Q-32. D270 item 8 ("yes it should use one nuimbers eerywhere", and "Fixed 85, no setting (Recommended)") rules on Hydra's own two numbers, and D28 records the game's cap and floor with "Steps 4-6 are shelved"; neither names both the game's per-note window and Hydra's fixed 85.
Found: 2026-10-03, session cebf3006; re-read 2026-10-30 in session b02ab3e6 (the reference build).

### TM-23 A running engine's stored window takes exactly the formula's values

Rule: In a normal-mode song the stored whole window at `#8288+0x20` took 126 distinct values, from 96.293167 ms to 171.431308 ms, which are the formula's ends.
Label: Proven in a live run
Evidence: `tools/ch_probe/experiments/results/poll_windows.csv` (tracked): 126 rows, maximum 171.431308 on 6 rows, minimum 96.293167 on 22 rows. The user ran `poll_windows.py` while playing on 2026-09-25 at 22:27 UTC. The file has no date or song name inside it.
Game code (6142): the per-frame routine at `0x20F5810` stores twice the per-note result at `+0x20` (decompile lines 52946 to 52950).
Hydra: no owner in `src/`; the tool names are `OFF_TOTAL_WINDOW`, `OFF_BACK_WINDOW` and `OFF_FRONT_WINDOW` in tools/ch_probe/constants.py.
Ruling: open split: see questions page Q-32
Found: 2026-09-25, session 8157f2dc.

### TM-24 The per-frame routines store the half-window and the whole window

Rule: The per-frame routines store the half-window at `#8288+0x10` and twice that at `+0x20`, so the stored total is the formula's output. A second routine, `0x20F6A50`, also calls the per-note window routine.
Label: Proven in the game's code
Evidence: decompile `ghidra_decompile_all.txt` lines 52946 to 52950 (in `0x20F5810`) and line 53619 (in `0x20F6A50`); no listing was made.
Game code (6142): RVAs `0x20F5810` and `0x20F6A50`, decompiled lines as above.
Hydra: no owner (see TM-17).
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-25 A note is missed only when the clock is strictly past its window end

Rule: A note counts as missed only when the clock is strictly later than its window end, and a press is valid up to and including the window end.
Label: Proven in the game's code
Evidence: committed listing `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 94 to 99 (the compares at 0x20F59E7 and 0x20F59FB); decompile line 52955.
Game code (6142): the base engine update, RVA `0x20F5810`, `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 94 to 99.
Hydra: not compared: nobody has read how `timing_tiers` (src/core/squeeze_rating.cpp) treats a hit exactly on the edge.
Ruling: none found: Hydra's own edge was not read
Found: 2026-10-03, drum-lanes FINDINGS-play-time.md.

### TM-26 The early and the late side of a note's window

Rule: The engine compares (note time - clock) with `#8288+0x10` on one side and a mirrored copy at `+0x18` on the other, and a flag at `+0x195` sets the early side to 0 and the late side to the full window.
Label: Unknown
Evidence: not checked: it needs a listing of the compares in `0x20F5810` and `0x20F6A50` and a trace of what sets `+0x195`; only decompile lines 52945 to 52955 and 53619 to 53639 were read.
Game code (6142): RVAs `0x20F5810` and `0x20F6A50`, no listing.
Hydra: one number for both sides, `kHitWindowMs` in src/core/model.h line 104.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-27 Whether the hit check enforces the stored window

Rule: Whether a press is accepted up to half the stored window (about 48 to 86 ms a side), or by some other limit, is not known. One screenshot showed a late hit at +85.9 ms, 0.2 ms past the 85.72 ms the formula allows at most.
Label: Unknown
Evidence: no run ever walked presses across the edge: the edge scripts `walk_edges.py` and `watch_window.py` have no result file, and the hit-window plan's steps 4 to 6 were shelved by D28. The +85.9 ms screenshot is not in the repo. The hit-compare candidates `0x100DAE0` and `0x20DE770` have no annotated listing of the comparison.
Game code (6142): not found: looked at `0x100DAE0` and `0x20DE770`.
Hydra: `kHitWindowMs` in src/core/model.h line 104: every note at a fixed 85 ms a side.
Ruling: open split: see questions page Q-32
Found: 2026-09-25, session 28e6f29d.

### TM-28 The formula tables in constants.py carry each other's names

Rule: In tools/ch_probe/constants.py the table named `RVA_FORMULA_NORMAL` holds the constants the game uses when the precision bit is set, and `RVA_FORMULA_PRECISION` holds the ones it uses when the bit is clear. `predicted_window_normal` in the probe's analysis.py is fed from the first, so it predicts the wrong curve.
Label: Proven in the game's code
Evidence: the decompile lines 767 to 781 against constants.py lines 166 to 180: the bit-clear branch uses the four addresses that constants.py lines 175 to 180 call precision. With those constants the bit-clear shape gives 171.43 ms at 170 and 96.29 ms at 75, which matches `poll_windows.csv`; the other table gives -242 and -78. The cards for sessions 1ac71f52 and ea0b884c already said "labels are swapped".
Game code (6142): RVA `0x20DDDA0`, `code/6142/hit_window_formula_20DDDA0.asm` lines 11 to 54.
Hydra: no owner in `src/`; the tool names are `RVA_FORMULA_NORMAL` and `RVA_FORMULA_PRECISION` in tools/ch_probe/constants.py lines 166 and 175.
Ruling: open split: see questions page Q-34
Found: 2026-09-25, session 28e6f29d.

### TM-29 Hydra uses one fixed 85 ms for badges, ratings, tiers and the search

Rule: Hydra judges every note at a fixed 85 ms a side, and the badges, Rating words, the report's timing tiers and the early-fill reach all read that one constant.
Label: Hydra assumes it, never checked
Evidence: `kHitWindowMs = 85.0` at src/core/model.h line 104; `fill_refuses` at line 316 reads it as the reach; `timing_tiers` and `squeeze_budget_ms` in src/core/squeeze_rating.cpp. The single `kHitWindowMs` is what D270 item 8 asked for.
Game code (6142): see TM-22.
Hydra: `kHitWindowMs` in src/core/model.h line 104.
Ruling: D270 item 8: the user asked "why are there two different numbers for hit window? who approved that? did you ever tell me you were using two different numbers?", then "yes it should use one nuimbers eerywhere", and chose "Fixed 85, no setting (Recommended)".
Found: 2026-10-30, session 9a82ff65.

### TM-30 The 0.75 constant that constants.py calls the hit threshold, as a hit-grade fraction

Rule: The `.rdata` value read as 0.75 and called "hit threshold" in constants.py is a hit-grade fraction.
Label: Unknown
Evidence: not checked: it needs a search of the decompile for the code that reads the constant. The only source is a guess in a 2026-09-25 transcript ("probably the fraction") and the constants.py comment.
Game code (6142): not found: where the value is read was not searched.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-09-25, session 8157f2dc.

## Precision mode

### TM-31 The precision-mode formula

Rule: With the precision bit set, the formula is 13 + 12.4805915 x (0.04 x x + 0.0004 x x^2), with no clamp of its own. If the input is held between twice the precision front and twice the back, as in normal mode, the whole window runs from 50.44 ms (x = 50) to 84.89 ms (x = 80), about 25.2 to 42.4 ms a side.
Label: Proven in the game's code
Evidence: `code/6142/hit_window_formula_20DDDA0.asm` lines 36 to 54, with the constants in the listing's comments; the decompile lines 776 to 781 agree. The 50.44 and 84.89 ms are arithmetic from those constants. The range rests on the clamp in TM-19 taking the precision front and back, which was not traced; the listing of `0x20F3A90` shows the clamp only for the engine fields in general.
Game code (6142): RVA `0x20DDDA0`, `code/6142/hit_window_formula_20DDDA0.asm` lines 36 to 54.
Hydra: no owner: a grep of `src/` for `precision` finds no game-mode input.
Ruling: open split: see questions page Q-32
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-32 Whether the menu's Precision Mode sets that bit, and the window it gives

Rule: Whether the player's Precision Mode option sets bit 0x1000, and whether a live precision-mode window runs from 50.44 to 84.89 ms, is not known.
Label: Unknown
Evidence: the enum name `PrecisionMode = 4096` is in `dump.cs` line 547692, but no code that sets the field from a menu was traced, and `tools/ch_probe/experiments/results/` holds no precision-mode run. constants.py line 241 says "nobody has read precision mode's cap".
Game code (6142): field `#8288+0x198`; its writer was not read.
Hydra: no owner
Ruling: open split: see questions page Q-32
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-33 Hydra uses its fixed 85 ms whatever the precision setting

Rule: Hydra's fixed 85 ms window is used for every chart whatever the player's precision setting, while the game's precision windows are narrower (TM-31).
Label: Hydra assumes it, never checked
Evidence: `kHitWindowMs` at src/core/model.h line 104; a grep of `src/` finds no precision-mode input.
Game code (6142): see TM-18 and TM-31.
Hydra: `kHitWindowMs` in src/core/model.h line 104.
Ruling: open split: see questions page Q-32
Found: 2026-10-30, session b02ab3e6 (the reference build).

## Calibration

### TM-34 The calibration and speed settings and their limits

Rule: Audio and video calibration are whole-millisecond settings in the `[offsets]` section, keys `audio` and `video`, default 0, limited to -2000 to 2000. The song speed setting is a whole percent, default 100, limited to 10 to 5000. The setting's constructor clamps the default into the range.
Label: Proven in the game's code
Evidence: `code/6142/settings_load_374D50.asm` lines 359 to 399 (audio and video) and 2017 to 2035 (song speed); `code/6142/setting_base_ctor_2116210.asm` for the clamp.
Game code (6142): the settings loader (scrambled names), RVA `0x374D50`; the int setting constructor `0x210C470`; the base constructor `0x2116210`.
Hydra: no owner: a grep of `src/` for "calibrat" and "song_speed" finds nothing.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-35 GameManager keeps the calibrations in seconds

Rule: At game start `GameManager.Awake` stores the audio calibration in seconds at `GameManager.audioCal` (+0x6C) and the video calibration minus the audio one at `GameManager.videoCal` (+0x68), and passes both to the clock at song start.
Label: Proven in the game's code
Evidence: `code/6142/gm_awake_B41A0.asm` lines 147 to 165 and `code/6142/cal_ms_to_s_210C1F0.asm` (value / 1000.0).
Game code (6142): `GameManager.Awake`, RVA `0xB41A0`; the converter `0x210C1F0`.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-36 Video calibration moves only what is drawn

Rule: Nothing that judges a hit reads the video time; video calibration moves only what is drawn.
Label: Unknown
Evidence: not checked: it needs every reader of the getter at `0x16C490` traced. The overlay handoff's session found two drawing readers, `HighwayScroll.Update` and `DrumsNoteRenderer.Update` (this row is now their only home); about 30 readers were never all traced. TM-15's bot run shows the engine clock does not move, which is consistent.
Game code (6142): RVA `0x16C490`, readers not all traced.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547.

### TM-37 The calibration screen classes

Rule: The audio and video calibration screen is `CalibrationMenu` (#992) with `Calibrator` (#1281). `CalibrationWindow` (#1166) is the input library Rewired's joystick axis window (deadzone, zero, sensitivity and invert sliders), not the audio and video calibration. How the calibration screen picks its values, and whether both values go to the online leaderboard, were never read.
Label: Unknown
Evidence: class metadata only: `dump.cs` lines 62144 to 62270 (namespace `Rewired.UI.ControlMapper`), 52940 and 66629 to 66713; no method body was read. The `CalibrationWindow` half rests on the class fields alone; one label covers the block, so it stays Unknown until a method is read.
Game code (6142): `Calibrator.Update`, RVA `0x2FB0B0`, not read.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-38 Hydra models no calibration

Rule: Hydra compares and scores in chart time as if both calibrations were 0. That matches the engine, whose clock does not move against the chart's times when the calibration changes (TM-15).
Label: Hydra assumes it, never checked
Evidence: a case-insensitive grep for `calibrat` over `src/` finds nothing.
Game code (6142): see TM-09 and TM-15.
Hydra: no owner: looked in `src/`, `tools/` and `tests/` for `calibrat`.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

## Playback speed

### TM-39 The playback speed is a ratio passed to the clock

Rule: The playback speed is the `song_speed` setting (TM-34). The game uses it as ratio = value / 100, passes it to the clock constructor and keeps it at clock+0x2C.
Label: Proven in the game's code
Evidence: `code/6142/speed_ratio_210C090.asm` lines 10 to 13, `code/6142/gm_load_coroutine_D8D80.asm` lines 444 to 451 and 460, `code/6142/clock_ctor_16C9A0.asm` line 30.
Game code (6142): the ratio getter (scrambled name), RVA `0x210C090`; the clock constructor `0x16C9A0`.
Hydra: no owner: Hydra scores at 1x only (TM-44).
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-40 At another speed the clock divides the audio position and the delay

Rule: At a ratio other than 1 the clock divides both the audio position and the delay by the ratio. Whether the audio position is in file seconds or in stretched real seconds is not known.
Label: Unknown
Evidence: not checked: it needs the unit of the audio manager's position read at `0x365F10`. The division is in `code/6142/clock_tick_16C650.asm` lines 72 to 75 and `code/6142/clock_ctor_16C9A0.asm` lines 22 to 32.
Game code (6142): RVA `0x365F10` (scrambled `BassAudioManager` member), not read.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-41 The practice speed scales the raw clock

Rule: `GameManager`'s practice speed field (+0xD8) is copied into the clock's multiplier at song start and scales how fast the raw clock runs between audio reads.
Label: Proven in the game's code
Evidence: `code/6142/gm_load_coroutine_D8D80.asm` lines 472 and 473; `code/6142/clock_tick_16C650.asm` lines 19 to 26.
Game code (6142): the load coroutine (scrambled `MoveNext`), RVA `0xD8D80`.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-42 The score adder has no speed term

Rule: The score adder adds the points times the multiplier and nothing else, so the playback speed does not change a note's points.
Label: Proven in the game's code
Evidence: decompile `ghidra_decompile_all.txt` lines 53121 to 53138 (`+0x94 += param_2 * +0x48`, no other factor). The adder that pays each note, `0x20F6EB0`, does the same in `code/6142/add_note_score_20F6EB0.asm` lines 36 to 41 (the imul by `+0x48` at line 40 and the add into `+0x94` at line 41). The third adder, `0x20F6A50`, was not re-read for a speed term.
Game code (6142): RVA `0x20F5F70`, decompiled lines as above; the listing is `code/6142/add_points_20F5F70.asm`.
Hydra: no owner for a speed rule (TM-44).
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-43 The hit window at another speed

Rule: Whether the hit window and the song clock run in chart time or in real time at a playback speed other than 100% is not known. Separately, at half speed the game time-stretches the audio, so a gameplay video's sound cannot be lined up with the chart by its audio.
Label: Unknown
Evidence: the formula and the clamp hold no speed variable, but whether the note times are already scaled by the speed was not traced. The time-stretch half is a card's roadblock with no code or file read; it is not checked and needs a run at half speed.
Game code (6142): not found: looked in the decompile lines 757 to 782 and 53871 to 53907.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-44 Hydra scores every chart at 100 percent

Rule: Hydra scores every chart as if played at 100 percent. It only sets apart leaderboard scores played at another speed, and no score or timing rule uses a speed.
Label: Hydra assumes it, never checked
Evidence: `kBaseSpeedPercent` and `is_base_speed` in src/net/dmbot_client.h lines 35 and 36, read by src/app/dm_report.cpp line 110.
Game code (6142): see TM-39.
Hydra: `kBaseSpeedPercent` in src/net/dmbot_client.h line 35.
Ruling: D25: "Off-speed leaderboard scores get their own "other speed" status and leave the Matched, Above optimal and Points left on table totals." (the decision log's words; no user quote is in the record)
Found: 2026-10-03, D25.

## The lane-press window

### TM-45 A missed lane note is granted after a recent lane press

Rule: A missed lane note is granted a hit when fewer than 2 grants (single lane) or 4 (double lane) have happened since the last lane press, or when a lane pad was pressed in the last 0.075 s of engine time (song clock at +0x100 minus the last lane press at `+0x2D0`). That 0.075 s is a different timer from a double lane's 0.75 s round timer, which breaks the combo (NL-59). The grant lands at the window's end, to within a rounding step, so lanes open no new squeeze timing. Every press of a note that carries a lane flag zeroes both grant counters (at `+0x2E8` and `+0x2EC`) with one 8-byte write at 0x20DECF3 and stores the press time at `+0x2D0`, so during a roll a few dropped notes are still credited.
Label: Proven in the game's code
Evidence: committed listing `docs/audit/drum-lanes/f20dcad0.asm` lines 22 to 27 (the grant check), 57 (the grant counter increment) and 69 to 73 (clock minus the last lane press against 0.075; line 71 loads the double 0.075). The limits 2 and 4 themselves were not opened. The reset is `docs/audit/drum-lanes/play/f20deaa0.asm` lines 131 to 138: the test for the lane bits 0xC000 on the note (line 131), the 8-byte store of a zero register over `+0x2E8` (line 133, with the register zeroed at line 23) and the press-time store (line 138); the same three steps repeat at lines 179 to 186 (0x20DEDC3 to 0x20DEDF4). Read from the listing by the reviewer of this page's move out of the drum-lanes findings; no run has shown it.
Game code (6142): RVA `0x20DCAD0`, `docs/audit/drum-lanes/f20dcad0.asm` lines 22 to 73; the press handler RVA `0x20DEAA0`, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 131 to 138.
Hydra: no owner: a grep of `src/` for "0.075", "lane_press", "press_window" and "grant" finds nothing; Hydra's paths assume every note is hit.
Ruling: none needed: no Hydra rule depends on it
Found: drum-lanes FINDINGS-play-time.md.

## Overhits

### TM-46 Pressing both the tom and the cymbal key for one gem misses it

Rule: pressing the tom key and the cymbal key of one colour for a single gem counts as an overhit, and the gem is missed.
Label: Unknown
Evidence: session 8157f2dc (2026-09-25). Its card (docs/sessions/2026-09-25-8157f2dc.md, the roadblock "Cymbal notes missed from about note 40 on") records that the bot's cymbal notes missed while its parser fired two keys for each such gem, and that they hit once the parser pressed one key per gem. That the session read this as an overhit comes from its own record, which is not kept. No run isolated the two-keys cause, no result file is in the repo, and the overhit path was not read in the game's code.
Game code (6142): not found: the overhit path was not read.
Hydra: none: Hydra's paths press only the notes in the chart.
Ruling: none needed.
Found: 2026-09-25, session 8157f2dc.

## Rows waiting on a live run

TM-02, TM-03, TM-22, TM-26, TM-27, TM-32, TM-36, TM-40, TM-43, TM-46. See `live-checks.md`.

## Open splits

TM-02 (Q-30), TM-03 (Q-31), TM-17 to TM-23, TM-27, TM-31 to TM-33 (Q-32), TM-28 (Q-34). See `questions.md`.
