# Timing [TM]

This page covers chart Offset and song.ini delay, the song clock, the hit window, calibration and playback speed.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-11-11.

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
Hydra: `preview_audio_offset_ms` in src/app/preview_source.cpp lines 311 to 316 makes the same choice (the delay wins when present and nonzero, else Offset x 1000), with the Offset parsed into `chart_offset_s` at src/parse/song.cpp line 1888. Hydra keeps fractional milliseconds and does not round up.
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

### TM-06 A .sng's delay is stored in the same field, with the same parse, as a song.ini delay

Rule: The `.sng` metadata reader asks for the key `delay` with a default of 0 and stores the whole milliseconds in `#828+0x24`, the field the song.ini reader fills (TM-01). Its getter parses the text the same way: a plain integer without a ".", or seconds times 1000, cut toward zero, with one (TM-02). Two steps are not proven: that TM-03's Offset step runs the same way for a `.sng` chart, and whether the getter's key lookup matches `Delay` as well as `delay`.
Label: Proven in the game's code
Evidence: `code/6142/sng_meta_reader_1EFF90.asm` lines 33 to 38 (the `delay` key at line 33, the zero default at line 35, the call to the getter at line 37 and the store at `+0x24` at line 38). `code/6142/sng_int_getter_262940.asm` lines 30 to 46 (the `String.Contains(".")` test and Int32.TryParse) and 47 to 69 (Single.TryParse, the multiply by 1000.0 at line 63 and the truncating convert at line 64). The getter's key comparer was not read. A `.srb` file has no delay field; that is Hydra's reading of its own parser (src/parse/srb.h has no delay), not the game's code.
Game code (6142): the `.sng` metadata reader of class #828 (scrambled name), RVA `0x1EFF90`, and its int getter (scrambled name), RVA `0x262940`, listings as above.
Hydra: `sng_metadata_delay_ms` (src/app/preview_source.cpp lines 182 to 188) matches the key case-blind, keeps the last `delay` pair and parses it with `parse_delay_ms`, which reads a decimal as milliseconds (TM-02's split, Q-30); `chart_audio_offset_ms` (line 358). Whether the game also matches `Delay` is not known, so whether the two agree on that key is not known.
Ruling: D275 item 4: the user chose "Yes, same rule (Recommended)" for `.sng` and `.srb` charts.
Found: 2026-10-30, session b02ab3e6 (the reference build); the ruling 2026-09-26, session 4908325e; the listings 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079.

### TM-07 The delay and Offset move only the audio, never the chart's own times

Rule: Hydra converts chart time to audio time in one place, audio = chart + offset, and scoring and the search never read the offset. This matches the game, where the delay changes only the clock's audio mapping (TM-05).
Label: Hydra assumes it, never checked
Evidence: `preview_audio_offset_ms` and its callers `chart_audio_offset_ms`, `song_length_ms` and the Preview mix; a grep of `src/` for `delay_ms`, `chart_offset_s` and `offset_ms` finds no other user.
Game code (6142): see TM-05.
Hydra: `preview_audio_offset_ms` and `kOffsetDirection` in src/app/preview_source.cpp lines 311 and 139.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-08 A second delay getter divides by 1582

Rule: A getter of class #828 at `0x1F0830` loads the delay at `#828+0x24` and divides it by 1582.0 instead of 1000.0. No caller was found by direct call or jump, pointer-table, slot or address reference, vtable, delegate, string lookup or inlined copy. A reflection walk that calls methods without naming them is not ruled out.
Label: Proven in the game's code
Evidence: `code/6142/delay_1582_1F0830.asm` lines 1 to 4 (the load of `+0x24`, the convert to a double, the divide by 1582.0 and the return). No direct caller found (byte scan of il2cpp and .text, for rel32 calls and jumps). `dump.cs` line 45148 declares the getter without `virtual`, and its metadata flags (0x886) hold no virtual bit, so it has no vtable slot. Its 64-bit address appears once, in the method-pointer table the runtime fills each method's code address from: the 19 words around it are #828's 19 methods in `dump.cs` order. None of the 26,183 method slots in the dump's `script.json` names it, so no delegate is built on it. Its scrambled name is not in `stringliteral.json` or the decompile, so no lookup by name finds it. The double 1582.0 has one reference in the image, the getter's own divide at 0x1F083D, so the body is not inlined elsewhere. The scans and the metadata reads are described in `docs/audit/2026-11-08-clone-hero-unknowns/W2.md`, "TM-08, third pass"; their scripts were not kept.
Game code (6142): the getter of class #828 (scrambled name), RVA `0x1F0830`, `code/6142/delay_1582_1F0830.asm` lines 1 to 4.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build); the listing and scans 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079.

## The song clock

### TM-09 The judged clock is the raw clock minus the audio calibration

Rule: The clock the game judges against is the raw clock minus the audio calibration. The time used for drawing is the raw clock plus (video minus audio).
Label: Proven in the game's code
Evidence: `code/6142/clock_raw_minus_audio_16C480.asm` lines 2 to 4 (load `+0x38`, subtract `+0x48`, return) and `code/6142/clock_raw_plus_video_16C490.asm` lines 2 to 4 (load `+0x50`, add `+0x38`, return); the bytes match the overlay handoff.
Game code (6142): two getters of class #707 (scrambled names), RVAs `0x16C480` and `0x16C490`.
Hydra: no owner: Hydra compares in chart time (TM-24).
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-10 GameManager stores both clocks on each Update

Rule: Each `GameManager.Update` call ticks the clock, copies the old `GameManager.songTime` into `GameManager.lastSongTime` (+0x80), then stores the first getter's result as `GameManager.songTime` (+0x70) and the second as `GameManager.songTimeVideoTime` (+0x78). Two cases skip this: the whole body is skipped when the manager's byte at `+0x158` is 0, and the clock block is skipped while paused when a CHNetManager test returns false. How the player hands `songTime` on to the engine is not covered by this block.
Label: Proven in the game's code
Evidence: `code/6142/gm_update_C05A0.asm` line 144 calls the clock tick `0x16C650`, lines 146 and 147 copy `+0x70` to `+0x80`, lines 151 and 153 call `0x16C480` and store the result at `+0x70`, and lines 157 and 159 call `0x16C490` and store it at `+0x78`. Lines 17 and 18 are the `+0x158` skip, and lines 68 to 79 are the pause test (`GameManager.isPaused` at `+0x90`, then CHNetManager's test at line 77, jumping past the block at line 79). The load coroutine makes the same two calls once at song start (`code/6142/gm_load_coroutine_D8D80.asm` lines 483 to 491). The player-to-engine hop was not read in this listing; TM-36 says what is known of it.
Game code (6142): `GameManager.Update`, RVA `0xC05A0`, `code/6142/gm_update_C05A0.asm` lines 17 to 159; the load coroutine, RVA `0xD8D80`, lines 483 to 491; fields from `dump.cs` lines 25298 to 25311.
Hydra: no owner: the Preview and Paths use chart time and never read the game clock.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547; 2026-11-08, session 77057df2 (docs-gaps search, and the listing); checked 2026-11-11, session 428cf079.

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

### TM-47 The clock object holds the raw clock, the audio clock and the correction

Rule: The clock object (class #707) keeps the raw clock as a double at `#707+0x38`, the audio clock as a double at `#707+0x20`, the correction as a float at `#707+0x18` and the applied total as a float at `#707+0x1C`, all in seconds. `GameManager.sync` holds the object. The tools' walk from the engine reaches the live object: in run LS-T2 of 2026-11-07 (result files in hydra-data, ch-mcp-runs/2026-11-07/ls-t2-run-a) the raw, audio and correction columns held a value on all 12,398 watch rows, the raw clock matched the song clock with a median difference of 0.0 ms, and the audio clock sat a median 0.66 ms behind it. The correction read -41.8 ms on the first row and about zero on the last.
Label: Proven in the game's code
Evidence: `code/6142/clock_tick_16C650.asm` line 75 stores the audio clock, lines 32 and 127 store the raw clock, line 107 stores the correction and line 125 stores the applied total; `code/6142/clock_tick_16C650.md` lines 5 to 11 explain them. The widths are in `dump.cs` lines 38467 to 38474, and `GameManager.sync` is line 25317. The walk was read on 2026-11-07 with tools/ch_mcp/chdis.py, and the listing was not kept. The engine stores its listener at `#8288+0x1A0` in its constructor at RVA `0x20F79E0`. The drum engine's constructor, RVA `0x20DF680`, has one direct caller, the `DrumsPlayer` method at RVA `0x33C560`, and that method passes the player itself as the listener (the helper at RVA `0x11676A0`, 0x11676B1 and 0x116771A). The player reaches the manager through `BasePlayer.gameManager`: `BasePlayer.Update`, RVA `0x20A1B0`, loads it and reads the manager's fields from it at 0x20A270 to 0x20A2BC. `GameManager.Update`, RVA `0xC05A0`, loads `GameManager.sync` at 0xC0827 and calls the clock tick at 0xC0839.
Game code (6142): the clock tick of class #707, RVA `0x16C650`; fields `#707+0x18`, `#707+0x1C`, `#707+0x20`, `#707+0x38`, `GameManager.sync`, `BasePlayer.gameManager` and `#8288+0x1A0`.
Hydra: no owner in `src/`; the tools read these through `clock_block` in tools/ch_probe/engine.py, with `OFF_ENGINE_LISTENER`, `PLAYER_OFF_GM`, `GM_OFF_CLOCK`, `CLOCK_OFF_AUDIO`, `CLOCK_OFF_RAW` and `CLOCK_OFF_CORR` in tools/ch_probe/constants.py.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-11-07, session 7d216054.

### TM-48 The song clock ran up to 127 ms ahead of wall time in one run

Rule: In LS-O2 run 1 the engine's song clock ran up to 127 ms ahead of wall time, measured against the median offset, at about 38.4 s of song time, and came back by about 39.5 s. In LS-T3 run 1 of 2026-11-07, with two builds started on purpose during the song, the same measure read from 137.3 ms ahead to 355.5 ms behind, and it stayed within 2 ms before the builds began (result files in hydra-data, ch-mcp-runs/2026-11-07/ls-t3-run-b). So heavy load on the machine can move it; whether that was also the cause in LS-O2 is not proven.
Label: Proven in a live run
Evidence: LS-O2 run 1, 2026-11-07, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-o2-run1\sp_watch-1.csv`, columns `clock_s` and `wall_s`. Taking clock minus wall on each of the 6,240 rows and subtracting the median gives +127.4 ms at clock 38.45 s, and within 3 ms again in the rows from 39.4 s to 39.6 s.
Game code (6142): field `#8288+0x100`, the clock the log reads; the cause was not looked for in the code. TM-12's pull toward the audio is one way a clock could do this, and nothing has checked it here.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-11-07, session 6c03736f

### TM-49 Builds during a song cut the game's frame rate from about 330 a second

Rule: In LS-T3 run 1 of 2026-11-07, two builds were started on purpose at song second 14.3. Before them the game drew about 330 frames a second, its `framerate = 330` cap. In the five-second bands from song second 15 to 25 it drew 53.4 and 60.5 frames a second, and its slowest gaps ran at 2.2 frames a second (two frames in about 0.9 s). The press round trip went from a median of about 8 ms to as much as 974.9 ms, and the run hit 101 of 142 notes. In Run A of the same day, with nothing compiling, every band stayed between 322.2 and 330.0. What starves the game (who held the foreground, or the compilers' priority) is not recorded.
Label: Proven in a live run
Evidence: `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-t3-run-b\fps.txt`, frames a second from the press frame counters per five song seconds, for both `ls-t2-run-a` and `ls-t3-run-b`; the hit count and the build start are in `docs/superpowers/plans/2026-11-07-ch-run-timing.md`, "What Run B showed".
Game code (6142): not looked for; the frame cap is `framerate` in `settings.ini`.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-11-07, session b132faae

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
Hydra: no owner in `src/`; the tool name is `OFF_ROUND_START` in tools/ch_probe/constants.py line 106.
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

Rule: The game's on-screen average of +2.5 ms, against clock reads of about -0.5 ms, points to about 3 ms of input polling delay. What the average is, is proven in the game's code: it is the mean, over the last 256 hits, of the value the hit method stores in the player's float at `+0xC4` (a scrambled `BasePlayer` field, `dump.cs` line 48752), which is the note's time minus `GameManager.songTime`, sign flipped and in ms, so positive means late. Nothing is added to it. How the 3 ms splits between polling and other causes is not in the code.
Label: Unknown
Evidence: the +2.5 ms is from a user screenshot that is not in the repo, and the 3 ms is the overlay handoff's own inference. The average: `code/6142/accuracy_text_208070.asm` keeps the last 256 values of `+0xC4` in a list (lines 46 to 77), sums them, divides by the count, flips the sign and multiplies by 1000 (lines 81 to 102 and 128 to 132); "Accuracy" is the latest value, sign flipped, times 1000 (lines 120 to 122). The hit method `code/6142/hit_offset_208780.asm` stores the note's `+0x28` time minus the manager's `+0x70` at `+0xC4` (lines 25 to 30) and jumps to the text method (line 34). Other writers of `+0xC4` were not searched. The value holds no press time stamp, so any time between the press and the frame that handles it shows in it. That gap was not measured: a run would need the accuracy text on, presses at known times from the virtual kit, and for each press the kit's time, the stored `+0xC4` and `songTime` in that frame.
Game code (6142): the accuracy text method of `BasePlayer` (scrambled name), RVA `0x208070`, `code/6142/accuracy_text_208070.asm` lines 46 to 132; the hit method (slot 34), RVA `0x208780`, `code/6142/hit_offset_208780.asm` lines 25 to 34.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547; the two listings 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079.

## The hit window

### TM-17 The drums engine always uses the neighbour-gap window

Rule: The drums engine constructor sets the byte at `#8255+0x41` to 1 in both normal and precision mode, and the per-note window routine takes the neighbour-gap branch when that byte is nonzero.
Label: Proven in the game's code
Evidence: `code/6142/engine_ctor_20DF680.asm` line 78 (`mov byte ptr [rbx + 0x41], 1`) and `code/6142/note_window_20F3A90.asm` lines 4 and 9.
Game code (6142): the drums engine constructor, RVA `0x20DF680`; the per-note window routine of class #8288 (scrambled name), RVA `0x20F3A90`.
Hydra: no owner for a per-note window: Hydra holds one fixed number, `kHitWindowMs` in src/core/model.h line 105.
Ruling: open split: see questions page Q-32
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-18 The constructor sets the front and back clamps by the precision bit

Rule: With bit 0x1000 of the engine flags at `#8288+0x198` clear, the constructor stores a front of 0.0375 s at `+0x38` and a back of 0.085 s at `+0x30`. With the bit set, the front is 0.025 s and the back 0.04 s. The whole window at `+0x20` starts at twice the back value.
Label: Proven in the game's code
Evidence: `code/6142/engine_ctor_20DF680.asm` lines 78 to 91 (the four constants read at lines 80, 81, 83 and 84, the stores at lines 85, 87 and 91). The decompile `ghidra_decompile_all.txt` lines 1807 to 1822 agree. The four constants are `RVA_CONST_NORMAL_FRONT`, `RVA_CONST_NORMAL_BACK`, `RVA_CONST_PRECISION_FRONT` and `RVA_CONST_PRECISION_BACK` in tools/ch_probe/constants.py. `dump.cs` line 547692 names the bit `PrecisionMode = 4096`.
Game code (6142): the drums engine constructor, RVA `0x20DF680`, `code/6142/engine_ctor_20DF680.asm` lines 78 to 91.
Hydra: no owner. `kHitWindowMs` (src/core/model.h line 105) is a separate number that equals the normal back value.
Ruling: open split: see questions page Q-32
Found: 2026-09-25, session 8157f2dc; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-19 Each half-gap is held between the front and the back

Rule: For each side of a note, the game halves the gap to the neighbour and clamps it between the front and the back. A missing neighbour, or one at the same time, gives the back value. The per-note window routine never hands the clamp a neighbour at the note's own time (TM-51), so the same-time exit is not reached for another gem of the same chord.
Label: Proven in the game's code
Evidence: `code/6142/half_gap_clamp_20F7210.asm` lines 13 to 22 (null or zero gap exit), 23 to 33 (the half-gap), 51 to 62 (min and max) and 72 to 77 (a missing neighbour returns the back); `code/6142/note_window_20F3A90.asm` lines 38 to 53 pass the front as the minimum and the back as the maximum. The decompile lines 53871 to 53907 agree.
Game code (6142): the clamp of class #8288 (scrambled name), RVA `0x20F7210`, `code/6142/half_gap_clamp_20F7210.asm` lines 13 to 77.
Hydra: no owner (see TM-17).
Ruling: open split: see questions page Q-32
Found: 2026-10-03, session cebf3006; listed 2026-10-30, session b02ab3e6 (the reference build).

### TM-51 A note's neighbours are the nearest notes at another time, so a chord's window is set by the chords either side

Rule: The per-note window routine finds a note's two neighbours by its tick: the first note strictly later, and the first note strictly earlier. A note at the same tick, such as another gem of the same chord, is never returned, so every gem of a chord gets the window that the gaps to the chords before and after it give. Freak's chord 1062 (kick and green cymbal at 133,520.37 ms, with chord 1061 84.27 ms before it and chord 1063 340.91 ms after it) gets half-gaps of 42.14 and 85 ms (the back clamp), a sum of 127.14 ms and 73.76 ms a side; it never gets the 85.72 ms that two back values would give.
Label: Proven in the game's code
Evidence: `code/6142/note_window_20F3A90.asm` lines 21 to 37 call both lookups on the note list at `this+0x1e0` with the note's tick (`note+0x40`) and its index (`note+0x20`), and line 31 (`xor r9d, r9d`) passes false for the backward lookup's same-tick flag. `code/6142/note_after_lookup_5DD8D0.asm` lines 60 to 65 read each note's tick and keep walking while it is at or before the target (`cmp rax, rsi; jle`), so lines 66 to 72 return the first note strictly later. `code/6142/note_before_lookup_5DDED0.asm` lines 63 to 68 return a note strictly earlier (`jl`), and lines 69 to 77 accept one at the same tick only when the flag byte is set (`cmovle edx, ebp`, with `ebp` the flag from line 9). `dump.cs` lines 552885 and 552894 give the two signatures, the second with its flag defaulting to false. What is inferred, not read: that the first lookup walks up the list and the second walks down it (so each returns the nearest note, not just some note on its side). That rests on the signatures' default start indexes, 0 and 2147483647 (the end of the list), and on the stop tests; the walker builders at RVAs `0x5FBC10` and `0x5FB640` and the walkers' step functions were not listed. The same-tick result does not depend on it: both stop tests exclude a note at the target's own tick whichever way the walk runs. That `note+0x40` is the tick is from `code/6142/sp_phrase_assign_20D2440.md` line 3 and the tick read 0x182C44210 from `code/6142/nearest_chord_5DE030.md` line 16. The 1062 example's times are the `ms` of chords 1061 to 1063 in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-l0\Eric Moore - Get Ur Freak On [Missy Elliot]-score.json` (the same `chart_ms` as rows 1061 to 1063 of `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-f1-run1\press-2.csv`), put through TM-19 to TM-21.
Game code (6142): the two list helpers of class #8670 (scrambled names), RVAs `0x5DD8D0` and `0x5DDED0`, `code/6142/note_after_lookup_5DD8D0.asm` lines 47 to 79 and `code/6142/note_before_lookup_5DDED0.asm` lines 50 to 93; their caller, RVA `0x20F3A90`, lines 21 to 37.
Hydra: the per-note window of the press schedule (`hydra_replay schedule`) in tools/replay_schedule.cpp takes the chords either side, the same rule; no screen reads it. Hydra's own judging uses no neighbours (see TM-17).
Ruling: none needed: same rule
Found: 2026-11-08, session 6341c618.

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
Evidence: TM-19 to TM-21 by arithmetic. Hydra's side: `kHitWindowMs` (src/core/model.h line 105) is read by the badges, Ratings, timing tiers and the search (TM-29).
Game code (6142): RVAs `0x20F3A90`, `0x20F7210` and `0x20DDDA0`, the three listings above.
Hydra: `kHitWindowMs` in src/core/model.h line 105.
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

Rule: The per-frame routine stores the half-window at `#8288+0x10`, minus the half-window at `+0x18` and twice the half-window at `+0x20`, so the stored total is the formula's output. When the byte at `+0x195` is set, it stores 0 at `+0x10` and minus the whole window at `+0x18` instead. A second routine, `0x20F6A50`, also calls the per-note window routine (TM-52).
Label: Proven in the game's code
Evidence: committed listing `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 74 to 85: line 74 calls the per-note window routine, line 78 stores the result at `+0x10`, lines 79 and 80 store its negative at `+0x18`, lines 76, 77 and 81 store twice it at `+0x20`, and lines 75, 82 to 85 are the `+0x195` branch. The decompile lines 52946 to 52950 agree. The call in `0x20F6A50` is `docs/audit/drum-lanes/play/f20f6a50_advance_schedule.asm` line 51. What sets `+0x195` was not read.
Game code (6142): the base engine update, RVA `0x20F5810`, `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 74 to 85; RVA `0x20F6A50`, `docs/audit/drum-lanes/play/f20f6a50_advance_schedule.asm` line 51.
Hydra: no owner (see TM-17).
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build); 2026-11-08, session 77057df2 (docs-gaps search).

### TM-52 The engine queues each note's window start and end and steps the clock to them

Rule: For each note, the event step works out the window's start (note time minus the half-window) and end (note time plus the half-window). It queues each one that falls after the current clock and before the step's limit, in a list of times that holds each time once. A third path hands the clock to the same helper when the window is already open (start before the clock, end after it) and queues the result if it is after the clock and before the limit. The next-event routine sorts that list largest first, so its end holds the earliest time, takes out the times inside the step and calls the frame update at each one. When the byte at `+0x290` is set, the step loop moves the clock to up to three of those queued times before the frame's own time, running the engine at each. So a window edge can be acted on between frames. When the start or end does not give back the half-window exactly, it goes through a helper whose effect was not read.
Label: Proven in the game's code
Evidence: committed listings under `docs/audit/drum-lanes/play/`. `f20f6a50_advance_schedule.asm` line 51 calls the per-note window routine, lines 57 to 60 and 72 to 75 work out the end and the start from the note time at `+0x28`, lines 61 to 70 and 76 to 85 pass either through the helper at 0x180F96AA0 (a VA, not listed) when the check fails, and lines 87 to 126 queue the start and the end through virtual slot 16 (vtable offset 0x238) when each is after the clock at `+0x100`; lines 97 to 117 hold the third path, which passes the clock held at entry (loaded at line 34) through the same helper. `f20f43a0_add_event.asm` lines 31 to 73 add a time to the list at `+0x180` only when no entry equals it. `f20f6ff0_next_event.asm` lines 44 to 76 sort the list largest first (a swap when an entry is larger than the one before it, lines 58 to 69), and lines 77 to 106 walk it from the end, so earliest first, and take out each time between the previous result and the limit and call `0x20F4550` with it (line 103). `f20f4950_step_loop.asm` line 18 tests `+0x290`, lines 29 to 60 call `0x20F6FF0` and set the clock at `+0x100` to its result up to three times, and line 61 puts the clock back. Not read: what sets `+0x290`, the helper at 0x180F96AA0, and what the step's limit is. IN-17 and IN-18 (page 7) count per-note calls and do not touch this queue.
Game code (6142): the event step, RVA `0x20F6A50`; slot 16, RVA `0x20F43A0` (`dump.cs` line 530212); the next-event routine, RVA `0x20F6FF0`; the step loop, RVA `0x20F4950`; listings as above.
Hydra: no owner: Hydra has no frame or event model (SP-29).
Ruling: none needed: no Hydra rule depends on it
Found: 2026-11-08, session 77057df2 (docs-gaps search).

### TM-25 A note is missed only when the clock is strictly past its window end

Rule: A note counts as missed only when the clock is strictly later than its window end, and a press is valid up to and including the window end.
Label: Proven in the game's code
Evidence: committed listing `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 94 to 99 (the compares at 0x20F59E7 and 0x20F59FB); decompile line 52955.
Game code (6142): the base engine update, RVA `0x20F5810`, `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 94 to 99.
Hydra: not compared: nobody has read how `timing_tiers` (src/core/squeeze_rating.cpp) treats a hit exactly on the edge.
Ruling: none found: Hydra's own edge was not read
Found: 2026-10-03, drum-lanes FINDINGS-play-time.md.

### TM-26 The early and the late side of a note's window

Rule: Measured at the song clock the player read just before it sent a press, the two sides are not the same: the early side reaches further than the late side. In LS-E run 1 of 2026-11-07, on notes 1,000 ms from both neighbours (so 85.72 ms a side by TM-22's formula), presses sent 80.91, 82.48, 87.85 and 88.43 ms early all hit, and presses sent 84.92, 85.46, 89.01 and 91.65 ms late all missed. So the early side reaches past 88.43 ms and the late side ends before 84.92 ms. Both fit one 85.72 ms side judged a few ms after the press is sent (TM-27). In the code, the per-frame update compares (note time - clock) with `#8288+0x10` on the early side and with its mirror at `+0x18` on the late side, and when the byte at `+0x195` is set the early side is 0 and the late side the whole window (TM-24). What sets `+0x195` was not read.
Label: Proven in a live run
Evidence: LS-E run 1, 2026-11-07, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-e-run1\press-1.csv`, the rows of chords 10, 20, 30, 40, 50, 60, 70 and 80 (the ones with a `probe` value): `chart_ms` minus `clock_ms_at_press` gives each press's lead, and `hit` gives the verdict. The asked leads are in `probes.json` there, and `replies.txt` there lists the eight results. The code part is the committed listing `docs/audit/drum-lanes/play/f20f5810_base_update.asm`: line 59 loads the clock at `+0x100`, lines 69 and 72 take the note time at `+0x28` minus the clock, lines 74 to 85 store the two sides, and lines 98 to 102 admit the note only when that difference is not above `+0x10` and not below `+0x18`.
Game code (6142): the base engine update, RVA `0x20F5810`, `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 59 to 102.
Hydra: one number for both sides, `kHitWindowMs` in src/core/model.h line 105.
Ruling: none found
Found: 2026-10-30, session b02ab3e6 (the reference build); the live result 2026-11-07, session d0a3868c; the listing 2026-11-08, session 77057df2 (docs-gaps search).

### TM-27 Whether the hit check enforces the stored window

Rule: Measured at the song clock the player read just before it sent a press, the hit check does not stop at the formula's side. On notes with an 85.72 ms side, a press sent 88.43 ms early hit, and one sent 84.92 ms late missed (TM-26 has all eight). That fits the check holding a press to the 85.72 ms side at a game frame that reads the press some time after it is sent: the early hit needs that lag to be at least 2.71 ms, and the late miss needs it to be more than 0.80 ms. Not proven: the lag itself (no row records the frame that judged a press); that the stored window held 85.72 ms on those notes (the run did not read it); and the edge on notes with closer neighbours, whose side is smaller. One screenshot showed a late hit at +85.9 ms, which is not in the repo.
Label: Proven in a live run
Evidence: LS-E run 1, 2026-11-07, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-e-run1\press-1.csv`: chord 60 (chart 63,000 ms, sent at clock 62,911.57, hit 1) and chord 70 (chart 73,000 ms, sent at clock 73,084.92, hit 0, score unchanged at 6,550). The chart is `C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair\notes.chart`, every note a kick 1,000 ms from the next except the last pair. In the code, the per-frame update admits a note to its window list only while (note time - clock) lies inside the stored sides (TM-26): `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 98 to 102 test the sides, lines 127 and 131 append the note to the list at `+0x58` and count it at `+0x8C`. That the press handler takes notes only from that list was not read.
Game code (6142): the base engine update, RVA `0x20F5810`, `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 98 to 131; the press-side compare not found: looked at `0x100DAE0` and `0x20DE770`.
Hydra: `kHitWindowMs` in src/core/model.h line 105: every note at a fixed 85 ms a side.
Ruling: open split: see questions page Q-32
Found: 2026-09-25, session 28e6f29d; the live result 2026-11-07, session d0a3868c; the listing 2026-11-08, session 77057df2 (docs-gaps search).

### TM-50 A press inside two unhit same-pad notes' windows goes to the earlier note

Rule: When a press lands inside the windows of two notes on its pad that are both still unhit, the game gives it to the earlier note, even when the later note is nearer. The handler's pad test is the same for every pad, and it does not ask whether the press is early or late. Each update the base update walks the chart from its cursor, in chart order, and appends to the engine's window array at `+0x58` (counted at `+0x8C`) each chord that is not yet judged and whose window holds the clock. The press handler then walks that array and the notes of each chord in turn. It skips a note for which a getter on the note's byte `+0x84` is true. A note takes the press when one of its pads is among the pads in the engine's word `+0x152` and not yet used in this walk. All of that note's pads are then marked used, so each pressed pad hits at most one note: the first unskipped one in the walk. The live runs below show that first one is the earlier note. Caveats, each a point the listings do not show: (1) the handler passes 0 and the count less one to an obfuscated generic call that builds its walk, so that it walks in index order rests on the live runs, not on the listing; (2) that the `+0x84` getter means "already hit or missed" is inferred from where it is used, since no listing names those bits; (3) `+0x152` is written on every input poll before the poll jumps into the handler, but whether it holds pads newly pressed or pads held down is not shown, so "two presses on one pad in one update count once" is not proven; (4) because a matched note marks all its pads used, a note with more than one pad bit blocks those pads for later notes in the same walk; (5) the handler has a second path: after the walk, when a pressed pad stayed unused, a note carrying flag 0x800 (the fill's activation note, SP-17 on page 05) that it saw in the walk can be hit with no pad match; (6) the base update's walk can stop or skip in ways this block does not rely on: a chord whose `+0x40` equals the previous chord's is not added, and a chord whose window has not yet opened ends the walk except for some flagged chords within three times the window. Not read: who empties the window array between updates. In LS-E run 1 of 2026-11-07, chord 119 (a kick at 122,000 ms) was pressed 60.30 ms late, 39.70 ms before chord 120 (a kick at 122,100 ms). That is inside 119's 76.57 ms side, and inside 120's window, because no side is smaller than 48.15 ms (TM-22). The press scored for 119, and 120's own press, 0.30 ms early, then hit as well. A second run, LS-F1 run 1 of 2026-11-07 on Get Ur Freak On, shows the next note taken by a press past its own note's window. First the game itself stalled for about 130 ms at song time 33.6 s: in the machine recorder's samples 1974 to 1978 its thread switches fell to 26 to 28 a sample, from 568 to 686 in samples 1968 to 1972, with only 8 or 9 of its 27 or 28 threads switching at all, and the press rows of chords 152 and 153 are 44 game frames apart over 262 ms of wall time, where about 3 ms a frame gives 87. The player's Star Power watch shows the same stall as a 131.5 ms gap with no row. After it, chord 153 (a yellow cymbal and a kick, 33,663.09 ms) was pressed 107.47 ms late, outside any side (85.72 ms at most, TM-22), and 61.07 ms before chord 154, a yellow cymbal at 33,831.63 ms. The press scored 65, and 154, whose own press came 0.83 ms early, missed (hit 0, score unchanged), so the late press took the next note on its pad. The same run's chord 803 (107,292.99 ms) is not a second case. In `press-2.csv` (rows 803 and 804, columns `chart_ms`, `clock_ms_at_press`, `score_before`, `score_after`, `hit`) 803 reads pressed at clock 107,355.12 ms, 62.13 ms late and 22.14 ms before chord 804 (107,377.26 ms); 803 scored 65 (262,485 to 262,550) and read hit 1, and 804's own press, 0.93 ms early, then also read hit 1 and scored 50 (262,550 to 262,600). But chords 802 and 804 are 84.27 ms either side of 803, so by TM-19 to TM-21 its side is 53.59 ms, and 62.13 ms late is past it, not inside it. The press clock is also in doubt: `sp_watch-1.csv` shows a 99.4 ms gap with no row in it, from wall 1794112614.9902 to 615.0896, and its clock reads either side, 107,255.114 and 107,261.175 ms, are 94 ms behind the 107,355.12 ms the press row gives for the same wall second (615.089). So the row does not show whether the press was early, inside both windows or late, and `replies.txt` there does not name chord 803. Not seen in a run: an early press inside two windows, or the two-window case on a pad other than the kick; the code above covers both. Not read in either run: which gem the 65 points were for (the run did not read what each press hit).
Label: Proven in the game's code
Evidence: The press handler `docs/audit/drum-lanes/play/f20deaa0.asm`: lines 37 to 53 build a walk over the window array at `+0x58` from index 0 to the count at `+0x8C` minus 1; lines 72 to 95 take each chord and walk its notes; lines 104 to 106 skip a note for which `0x214B6D0` is true (`test byte ptr [rcx + 0x84], 3`, `docs/audit/note-shuffle/scout-flag/note_getters.txt` line 9); lines 107 and 108 remember a note with flag 0x800, and lines 163 to 199 hit it after the walk (caveat 5); lines 109 to 113 test the note's pads at `+0x80` against the pressed pads at `+0x152` with the pads already used this press taken out; line 114 adds the note's pads to the used set; line 152 hits the note through slot 11. The window array is filled by the base update `docs/audit/drum-lanes/play/f20f5810_base_update.asm`: lines 57 to 66 start at the cursor `+0x50` in the chart's chord list at `+0x1E0`, lines 88 to 102 keep a chord whose time minus the clock lies inside its window and that is not judged (line 90), skipping one whose `+0x40` matches the previous chord's (line 96), lines 126 to 131 append it at the count and add one, lines 256 to 258 step to the next chord, and lines 235 to 251 end the walk at a chord whose window has not opened (caveat 6). The poll `docs/audit/drum-lanes/play/f20de070_poll_input.asm` writes `+0x152` (lines 60, 89 and 99) and jumps into the handler (line 114). The order of the walk is shown by the live cases (caveat 1). The live cases: LS-E run 1, 2026-11-07, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-e-run1\press-1.csv`: the row of chord 119 (sent at clock 122,060.30, hit 1, score 11,750 to 11,950) and of chord 120 (sent at clock 122,099.70, hit 1, score 11,950 to 12,150). Had the press gone to 120, 119 would have missed and 120's own press would have found no note. The stats file `scorestats-Edge_Walk_Pair-20261107-233205.json` there gives 117 notes hit of 121, and `replies.txt` there names the four misses as the four late probes of TM-26 and gives 119's side. The LS-F1 facts are in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-f1-run1\press-2.csv` (rows 153 and 154: `chart_ms`, `clock_ms_at_press`, `score_before`, `score_after`, `hit`; rows 152 and 153: `down_frame` 11,518 and 11,562, `down_ms` 1794112541224 and 1794112541486), `sp_watch-1.csv` there (the 131.5 ms gap, wall 1794112541.353 to 541.484), and `machine-2\threads.csv` there (the rows with `pid` 9604, the game's pid in `replies.txt` there, for samples 1968 to 1979; `machine-2\samples.csv` gives samples 1974 to 1978 the wall times 541.38 to 541.46), with `replies.txt` there. The chart is `C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair\notes.chart`; its last two notes are kicks 100 ms apart, and the note before them is 1,000 ms earlier.
Game code (6142): the press handler, RVA `0x20DEAA0` (`docs/audit/drum-lanes/play/f20deaa0.asm`), and the base update, RVA `0x20F5810` (`docs/audit/drum-lanes/play/f20f5810_base_update.asm`).
Hydra: no owner in `src/`; the press schedule's `far_press_refusal` in tools/replay_schedule.cpp follows this rule: it refuses a press only when an earlier same-pad chord whose window holds it gets no press on that pad, or gets it at the same clock or later. A refused press is written as a refusal and `refused: true` on the chord's row, not as a press. Caveat 3 leaves the same-clock case unproven, so the schedule takes the safe side. The player in tools/ch_probe/experiments/play_chart.py has no check of its own: it sends the presses the schedule wrote.
Ruling: D322 Q16: the refusal is to follow this rule, refusing a press only when an earlier note on its pad whose window holds it will still be unhit when it lands
Found: 2026-11-07, session d0a3868c; the code reading 2026-11-08, session 31e4f22f.

### TM-28 The formula tables in constants.py carry each other's names

Rule: In tools/ch_probe/constants.py the table named `RVA_FORMULA_NORMAL` holds the constants the game uses when the precision bit is set, and `RVA_FORMULA_PRECISION` holds the ones it uses when the bit is clear. The probe's normal-window prediction was fed from the first, so it predicted the wrong curve; it lived in tools/ch_probe/experiments/analysis.py (removed; `git show a081f660:tools/ch_probe/experiments/analysis.py`). No tool predicts a window now, but EngineModel.constants in tools/ch_probe/engine.py still files each table's values under that table's name.
Label: Proven in the game's code
Evidence: the decompile lines 767 to 781 against constants.py lines 170 to 184: the bit-clear branch uses the four addresses that constants.py lines 179 to 184 call precision. With those constants the bit-clear shape gives 171.43 ms at 170 and 96.29 ms at 75, which matches `poll_windows.csv`; the other table gives -242 and -78. The cards for sessions 1ac71f52 and ea0b884c already said "labels are swapped".
Game code (6142): RVA `0x20DDDA0`, `code/6142/hit_window_formula_20DDDA0.asm` lines 11 to 54.
Hydra: no owner in `src/`; the tool names are `RVA_FORMULA_NORMAL` and `RVA_FORMULA_PRECISION` in tools/ch_probe/constants.py lines 170 and 179.
Ruling: open split: see questions page Q-34
Found: 2026-09-25, session 28e6f29d; line cites re-pinned 2026-11-08, session 77057df2 (docs-gaps search).

### TM-29 Hydra uses one fixed 85 ms for badges, ratings, tiers and the search

Rule: Hydra judges every note at a fixed 85 ms a side, and the badges, Rating words, the report's timing tiers and the early-fill reach all read that one constant.
Label: Hydra assumes it, never checked
Evidence: `kHitWindowMs = 85.0` at src/core/model.h line 105; `fill_refuses` at line 318 reads it as the reach; `timing_tiers` and `squeeze_budget_ms` in src/core/squeeze_rating.cpp. The single `kHitWindowMs` is what D270 item 8 asked for.
Game code (6142): see TM-22.
Hydra: `kHitWindowMs` in src/core/model.h line 105.
Ruling: D270 item 8: the user asked "why are there two different numbers for hit window? who approved that? did you ever tell me you were using two different numbers?", then "yes it should use one nuimbers eerywhere", and chose "Fixed 85, no setting (Recommended)".
Found: 2026-10-30, session 9a82ff65.

### TM-30 The 0.75 constant that constants.py calls the hit threshold is a double lane's round timer

Rule: The `.rdata` value read as 0.75 and named `RVA_HITCHECK_THRESHOLD` in constants.py has one reader found, and that reader does not use it as a hit-grade fraction. The drum engine compares the song clock minus a double lane's round start with it: it is the 0.75 s round timer of NL-59 and IN-15. The earlier guess that it is a hit-grade fraction is wrong for this reader.
Label: Proven in the game's code
Evidence: committed listing `docs/audit/drum-lanes/play/f20ddec0.asm` lines 30 to 33: line 30 loads the clock at `+0x100`, line 31 subtracts the round start at `+0x2E0`, and line 32 compares the result with the double at the address constants.py names `RVA_HITCHECK_THRESHOLD` (the listing's comment reads it as 0.75). The old reading came from a guess in a 2026-09-25 transcript ("probably the fraction") and the constants.py comment at line 193. Other readers of the constant were not searched.
Game code (6142): the drum engine's slot 9, RVA `0x20DDEC0`, `docs/audit/drum-lanes/play/f20ddec0.asm` lines 30 to 33.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-09-25, session 8157f2dc; the reader 2026-11-08, session 77057df2 (docs-gaps search).

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

Rule: Whether the player's Precision Mode option sets bit 0x1000, and whether a live precision-mode window runs from 50.44 to 84.89 ms, is not known. The bit's path from the player menu to the engine is read in the code: a menu modifier entry ORs its value into the profile's modifiers word, that word is copied into the record the drums engine is built from, and the engine stores it at `#8288+0x198`, the word the window formula tests. Three links are not read, so the bit half is not proven: the leaderboard filter that can drop menu entries, whether the menu's profile is the profile the player uses at run time, and that the menu's "Precision Mode" text is the 4096 entry (tied to it by the enum name only). The window half needs a live run.
Label: Unknown
Evidence: `code/6142/psm_mods_list_32CE60.md` (the three arrays the player menu offers, each holding 4096, read from `global-metadata.dat`; the filter at listing lines 36 to 82 was not read); the menu method `0x32D360` passes an entry to the profile's toggle at 0x32D39E to 0x32D3A2; `code/6142/profile_mods_toggle_21236D0.asm` lines 32 to 34 OR the value into the profile's `+0x14` when it is off (lines 25 to 30 first clear the modifiers a helper names as clashing; what it clears was not read); `code/6142/player_kind_11676A0.asm` lines 26 to 33 copy the profile's `+0x14` into the record's `+8`; `code/6142/engine_ctor_20F79E0.asm` lines 83 to 85 store the record's `+8` at `+0x198`; `code/6142/hit_window_formula_20DDDA0.asm` line 11 tests bit 0x1000 of it. The enum name `PrecisionMode = 4096` is in `dump.cs` line 547692. No direct caller of the toggle outside `PlayerSelectionMenu` was found (byte scan of il2cpp and .text: three callers, all in that class). `tools/ch_probe/experiments/results/` holds no precision-mode run, and constants.py line 245 says "nobody has read precision mode's cap". A live run would set the profile's modifiers word to include 0x1000 before `0x33C560` builds the engine, or call the toggle `0x21236D0` with 4096, then read `#8288+0x198` and the half-window `0x20F3A90` returns for a note with no near neighbour.
Game code (6142): the `PlayerSelectionMenu` list method, RVA `0x32CE60`; the profile toggle (scrambled name), RVA `0x21236D0`; the record builder, RVA `0x11676A0`; the engine base constructor, RVA `0x20F79E0`; field `#8288+0x198`; listings as above.
Hydra: no owner
Ruling: open split: see questions page Q-32
Found: 2026-10-30, session b02ab3e6 (the reference build).

### TM-33 Hydra uses its fixed 85 ms whatever the precision setting

Rule: Hydra's fixed 85 ms window is used for every chart whatever the player's precision setting, while the game's precision windows are narrower (TM-31).
Label: Hydra assumes it, never checked
Evidence: `kHitWindowMs` at src/core/model.h line 105; a grep of `src/` finds no precision-mode input.
Game code (6142): see TM-18 and TM-31.
Hydra: `kHitWindowMs` in src/core/model.h line 105.
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
Evidence: The getter at `0x16C490` has four direct callers (byte scan of il2cpp and .text), and each stores the video time in one field, `GameManager.songTimeVideoTime` (+0x78): the `GameManager` method `0xBEB50` at `code/6142/gm_video_time_BEB50.asm` line 160, `GameManager.Update` at `code/6142/gm_update_C05A0.asm` line 159, the coroutine `0xD9B90` at `code/6142/gm_coroutine_D9B90.asm` line 150, and the load coroutine at `code/6142/gm_load_coroutine_D8D80.asm` line 491. The two drawing readers read that field, not the getter: `HighwayScroll.Update` through its `GameManager` field at `code/6142/highway_scroll_update_3818E0.asm` line 22, and `DrumsNoteRenderer.Update` reads `+0x78` of the object at its `+0x28` at `code/6142/drums_note_renderer_update_1370A0.asm` line 98 (that `+0x28` is its `GameManager` is likely but not shown). On the judging side, the window method `0x20F3A90`, its clamp `0x20F7210` and the formula `0x20DDDA0` read no clock (`code/6142/note_window_20F3A90.asm`, `code/6142/half_gap_clamp_20F7210.asm`, `code/6142/hit_window_formula_20DDDA0.asm`). The press method reads the engine's own clock at `+0x100` and stores it as the last lane press time (`docs/audit/drum-lanes/play/f20deaa0.asm` lines 135 to 138 and 183 to 186). `BasePlayer.Update` reads `GameManager.songTime` (+0x70) for the times it hands on (`code/6142/baseplayer_update_20A1B0.asm` lines 102 to 105, 114, 125 to 138 and 152 to 164), but not at every call: at the two calls of `0x20F4550` at 0x20A3BD and 0x20A425 the time is a value a player method writes to the stack after it is given `songTime` (a virtual call at 0x20A397, and the `BasePlayer` method at 0x20A400), and what those methods write was not read. A scan of the 321 engine methods from 0x20DD000 to 0x20F8000 found none that names `GameManager` or calls either clock getter; its one double read at `+0x78` of another object, at 0x20E766E, reads the replay record, not the manager. Not shown: what time the nine other direct callers of `0x20F4550` pass (at 0xFEB4C0, 0xFEB580, 0xFEB650, 0xFFC1C0, 0xFFC940, 0xFFD340, 0xFFDAB0, 0x20F5120 and 0x20F6FF0, not listed), and calls through a method table, which the scan cannot find. TM-15's bot run shows the engine clock does not move with the video calibration, which is consistent.
Game code (6142): RVA `0x16C490` and its four callers, listings as above; `BasePlayer.Update`, RVA `0x20A1B0`; the engine time method `0x20F4550`, its other callers not listed.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-28, session f756d547; 2026-11-08, session 77057df2 (docs-gaps search, and the listings); checked 2026-11-11, session 428cf079.

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
Evidence: The division is in `code/6142/clock_tick_16C650.asm` lines 72 to 75 and `code/6142/clock_ctor_16C9A0.asm` lines 22 to 32. The position getter `0x365F10` returns 0 unless a song is loaded, then asks BASS for the byte position of the channel whose handle is at the audio manager's `+0x58` and converts it to seconds (`code/6142/audio_position_365F10.asm` lines 17 to 45; the externs match BASS's position and bytes-to-seconds calls by shape and error text). Which channel that is: the mixer builder `0x368590` stores `+0x58` (`code/6142/mixer_create_368590.asm` line 31). When its int argument is not -1 it builds a tempo stream over the mixer (lines 21 to 28) and `+0x58` gets the tempo stream; when the argument is -1, `+0x58` gets the mixer. The tempo stream's starting tempo is set to that argument (`code/6142/tempo_create_366FC0.asm` lines 112 to 117). The song-load method passes the song speed's whole percent minus 100, but only when its own byte at `+0x68` is set, and -1 otherwise (`code/6142/audio_load_36B700.asm` lines 112 to 125); what writes that byte was not searched. So once that byte is set, any speed, 100 included, gets a tempo stream, and a speed of 99% gives 99 - 100 = -1, the "no tempo" value, so no tempo stream. One side fact: when a bool setting is on, the tempo builder also sets the pitch to 12 x log2(speed / 100) (`code/6142/tempo_create_366FC0.asm` lines 68 to 106). Not in the game's code: whether BASS reports a tempo stream's position in source seconds or in played seconds. That is the BASS_FX library's behaviour; a run at 50% speed comparing the getter's result with real time would settle it.
Game code (6142): the position getter (scrambled `BassAudioManager` member), RVA `0x365F10`; the mixer builder, RVA `0x368590`; the tempo builder, RVA `0x366FC0`; the song-load method, RVA `0x36B700`; listings as above.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build); the listings 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079.

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
Evidence: the formula and the clamp hold no speed variable (the decompile lines 757 to 782 and 53871 to 53907), but whether the note times are already scaled by the speed was not traced. Most times the player hands the engine are read from `GameManager.songTime` (TM-36), which comes from a clock that divides the audio position by the speed ratio (TM-40); whether that position is in file seconds is TM-40's open BASS_FX question. The time-stretch half is a card's roadblock with no code or file read; it is not checked and needs a run at half speed. For such a run, the song speed is an int setting (default 100, limits 10 to 5000, TM-34), and its value setter is the generic int setter `0xFE5F50`, which stores the value at the setting's `+0x10` and then calls the setting's changed method (`code/6142/int_setting_set_FE5F50.asm` lines 2 to 4). The run must set it before the song loads, because the song-load method reads it to build the tempo stream (TM-40).
Game code (6142): not found for the note-time scaling: looked in the decompile lines 757 to 782 and 53871 to 53907; the setter of class #8338 (scrambled name), RVA `0xFE5F50`, `code/6142/int_setting_set_FE5F50.asm` lines 2 to 4.
Hydra: no owner
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build); the setter 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079.

### TM-44 Hydra scores every chart at 100 percent

Rule: Hydra scores every chart as if played at 100 percent. It only sets apart leaderboard scores played at another speed, and no score or timing rule uses a speed.
Label: Hydra assumes it, never checked
Evidence: `kBaseSpeedPercent` and `is_base_speed` in src/net/dmbot_client.h lines 35 and 36, read by src/app/dm_report.cpp line 128.
Game code (6142): see TM-39.
Hydra: `kBaseSpeedPercent` in src/net/dmbot_client.h line 35.
Ruling: D25: "Off-speed leaderboard scores get their own "other speed" status and leave the Matched, Above optimal and Points left on table totals." (the decision log's words; no user quote is in the record)
Found: 2026-10-03, D25.

## The lane-press window

### TM-45 A missed lane note is granted after a recent lane press

Rule: A missed lane note whose pad is in the lane is granted a hit when fewer than 2 grants (single lane) or 4 (double lane) have happened since the last lane press, or when a lane pad was pressed in the last 0.075 s of engine time (song clock at +0x100 minus the last lane press at `+0x2D0`). That 0.075 s is a different timer from a double lane's 0.75 s round timer, which breaks the combo (NL-59). The grant lands at the window's end, to within a rounding step, so lanes open no new squeeze timing. Every press of a note that carries a lane flag zeroes both grant counters (at `+0x2E8` and `+0x2EC`) with one 8-byte write at 0x20DECF3 and stores the press time at `+0x2D0`, so during a roll a few dropped notes are still credited.
Label: Proven in the game's code
Evidence: committed listing `docs/audit/drum-lanes/f20dcad0.asm` lines 22 to 27 (the grant check: lines 19 to 21 skip it unless the note's pad is in the lane mask at `+0x2C4`; line 22 tests the double-lane byte at `+0x2C8`, lines 23 to 25 pick 4 when it is set and 2 when it is clear, and line 26 compares the grant counter at `+0x2E8` with that limit), 57 (the grant counter increment) and 66 to 73 (the 0.075 s grant: lines 66 to 68 skip it unless the note's pad is in the lane mask at `+0x2C4`, then lines 69 to 73 compare clock minus the last lane press with 0.075; line 71 loads the double 0.075). Lines 45 to 48 set the `+0x2C8` byte from the note's double-lane flag 0x8000. The reset is `docs/audit/drum-lanes/play/f20deaa0.asm` lines 131 to 138: the test for the lane bits 0xC000 on the note (line 131), the 8-byte store of a zero register over `+0x2E8` (line 133, with the register zeroed at line 23) and the press-time store (line 138); the same three steps repeat at lines 179 to 186 (0x20DEDC3 to 0x20DEDF4). Read from the listing by the reviewer of this page's move out of the drum-lanes findings; no run has shown it.
Game code (6142): RVA `0x20DCAD0`, `docs/audit/drum-lanes/f20dcad0.asm` lines 22 to 73; the press handler RVA `0x20DEAA0`, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 131 to 138.
Hydra: no owner: a grep of `src/` for "0.075", "lane_press", "press_window" and "grant" finds nothing; Hydra's paths assume every note is hit.
Ruling: none needed: no Hydra rule depends on it
Found: drum-lanes FINDINGS-play-time.md; the limits and the lane-mask test 2026-11-08, session 77057df2 (docs-gaps search).

## Overhits

### TM-46 Pressing both the tom and the cymbal key for one gem misses it

Rule: pressing the tom key and the cymbal key of one colour for a single gem counts as an overhit, and the gem is missed.
Label: Unknown
Evidence: session 8157f2dc (2026-09-25). Its card (docs/sessions/2026-09-25-8157f2dc.md, the roadblock "Cymbal notes missed from about note 40 on") records that the bot's cymbal notes missed while its parser fired two keys for each such gem, and that they hit once the parser pressed one key per gem. That the session read this as an overhit comes from its own record, which is not kept. No run isolated the two-keys cause, and no result file is in the repo. The overhit path itself is readable: in the press handler, the pressed pads left over after the matching notes are taken go to the overhit method (`docs/audit/drum-lanes/play/f20deaa0.asm` lines 201 to 209), and that method, unless one of its exits applies (the byte at `+0xF0` set, the lane tests, or the `+0x308` test), clears the lane state, makes one virtual call and adds one to the int at `+0xB0` (what that int counts is not settled, IN-13) (`docs/audit/drum-lanes/play/f20de9a0.asm` lines 24 to 44). The two keys of one colour set two different bits: for yellow, the input source hands Rewired action 2 to a setter that sets bit 2 of the inputs record (`code/6142/drums_input_source.asm` lines 78 to 83, `code/6142/pad_yellow_tom_2176900.asm`), and action 0x26 to one that sets bit 6 (lines 130 to 135, `code/6142/pad_yellow_cymbal_21766F0.asm`). That actions 2 and 0x26 are the yellow tom and the yellow cymbal is inferred from the order of the reads, not from action names. In the press handler a note takes the press when its mask at `+0x80` shares a bit with the pressed pads not yet taken (`docs/audit/drum-lanes/play/f20deaa0.asm` lines 109 to 114), and the pads left over go to the overhit method. So if a yellow cymbal gem's mask holds only the cymbal bit, the tom bit is left over. Three links are not shown: that the record's bits reach the engine's `+0x152` unchanged, what mask a cymbal gem holds at `+0x80`, and the action names. That the gem itself is missed is not shown either: it is taken by its matching bit before the overhit runs, and whether the overhit then breaks it is in the overhit method's virtual call, which was not followed. So the block stays Unknown.
Game code (6142): the press handler, RVA `0x20DEAA0`, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 109 to 114 and 201 to 209; the overhit method at 0x20DE9A0, `docs/audit/drum-lanes/play/f20de9a0.asm` lines 24 to 44; the two pad setters (scrambled names), RVAs `0x2176900` and `0x21766F0`, listings as above.
Hydra: none: Hydra's paths press only the notes in the chart.
Ruling: none needed.
Found: 2026-09-25, session 8157f2dc; the overhit path 2026-11-08, session 77057df2 (docs-gaps search); the pad setters 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079.

## Rows waiting on a live run

TM-02, TM-03, TM-22, TM-27, TM-32, TM-36, TM-40, TM-43, TM-46. See `live-checks.md`.

## Open splits

TM-02 (Q-30), TM-03 (Q-31), TM-17 to TM-23, TM-27, TM-31 to TM-33 (Q-32), TM-28 (Q-34). See `questions.md`.
