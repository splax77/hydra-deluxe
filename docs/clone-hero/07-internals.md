# Internals [IN]

This page covers the game's classes and fields that the tools read, the per-note hooks, the bot, and how the two builds name things.
It describes build 1.1 (v1.1.0.6142), and says where 1.0 (v1.0.0.4045) differs.
Last write-back: 2026-10-31.

Most rows here are about the game's own layout, which no Hydra rule reads. Their Ruling line says so. The rules these fields feed (timing, Star Power, scoring) live on pages 03, 05 and 06. Paths that start `RUNS` mean `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\`; `GD` means Ghidra's full decompile `ghidra_decompile_all.txt`, which `code/README.md` describes (it is not in the repo); `dump.cs` is `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs`; `overlay-spike` means `tools/ch_probe/experiments/results/overlay-spike/`, whose result files are untracked today.

## The game manager

### IN-01 GameManager keeps 40 of its 60 field names readable

Rule: `GameManager` (TypeDefIndex 449, in CloneHero.dll) has 60 fields, and 40 of them keep plain names, among them the song time, song length, song-over flag and practice speed.
Label: Proven in the game's code
Evidence: `dump.cs` line 25284 (the class header) and the field lines after it: songLength at +0x50, songTime at +0x70, isSongOver at +0x91, practiceSpeed at +0xD8. The count of 60 fields, 40 with plain names, is `RUNS\l4\compare-4045-6142.txt` line 13803 (20 scrambled and 40 readable, IN-49). This is dump text, not a listing.
Game code (6142): `GameManager.songTime`, `GameManager.songLength`, `GameManager.isSongOver`, `GameManager.practiceSpeed`; fields only, no method.
Hydra: no owner: Hydra never reads the running game. The tools read the class by name in tools/ch_mcp/agent/gamemanager.ts.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study (`docs/research/2026-10-28-clone-hero-mcp-feasibility.md` line 54); recounted 2026-10-30, session b02ab3e6 (the reference build).

### IN-02 The game manager's fields can be read by name while a song plays

Rule: Frida with frida-il2cpp-bridge 0.14.0 reads `GameManager.songTime` and the other plain fields by name on metadata version 31, and there is one live game manager during a song.
Label: Proven in a live run
Evidence: `RUNS\l2\clock.jsonl` (2026-10-28): 371 samples, songTime from 3.0683 s to 63.5600 s, isSongOver true in the last sample, one instance throughout.
Game code (6142): `GameManager.songTime`, `GameManager.isSongOver`; read by field name, no method.
Hydra: no owner: Hydra never reads the running game. The tool readers are tools/ch_mcp/agent/state.ts and clock.ts.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study line 137; checked 2026-10-30, session b02ab3e6 (the reference build).

### IN-03 A finished song's game manager stays in memory, marked by a null native pointer

Rule: After a song, the destroyed game manager is still in memory. Unity's m_CachedPtr field (+0x10 of the base Object) is null on it, which tells it apart from a live one; at the main menu the tools read "menu (no live GameManager)" with one destroyed instance.
Label: Proven in a live run
Evidence: the field layout is `dump.cs` line 695824 (m_CachedPtr at 0x10). The live behaviour comes from two runs on 2026-10-29 in session e9b04efc, on the Window Map probe chart. First, after a song the user went back to the main menu, and two state reads both said "song over", with the game manager found and one instance, at a song time of 63.56 s. Then, once the tools skipped a game manager whose m_CachedPtr is null, a second run left the song for the main menu and both the attach and the state read said "menu (no live GameManager)", found false, with one destroyed instance. The session's card (docs/sessions/2026-10-29-e9b04efc.md) records both runs. No raw result file was kept.
Game code (6142): the base Object's native-pointer field; no method.
Hydra: no owner. The tool check is in tools/ch_mcp/agent/gamemanager.ts lines 39 to 60.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-29, session e9b04efc (`docs/agents/clone-hero-mcp.md` repeated it and now points here).

### IN-04 The game manager is where the timing settings enter a song

Rule: When the game starts, `GameManager.Awake` stores the audio calibration in seconds at +0x6C and the video calibration minus the audio calibration at +0x68. The song-load coroutine reads `song.ini` before the chart and copies practiceSpeed (+0xD8) into the song clock. What those values do is on page 03.
Label: Proven in the game's code
Evidence: `code/6142/gm_awake_B41A0.asm` lines 147 to 165 and the millisecond-to-second helper `code/6142/cal_ms_to_s_210C1F0.asm`; `code/6142/gm_load_coroutine_D8D80.asm` lines 272 to 291 (song.ini read, then chart load) and 472 to 473 (practice speed copied).
Game code (6142): `GameManager.Awake`, RVA `0xB41A0`; helper RVA `0x210C1F0`; the load coroutine's MoveNext (a scrambled nested class of GameManager), RVA `0xD8D80`.
Hydra: no owner: Hydra has no calibration and no practice-speed model (a grep for `calibrat` in src/ finds nothing).
Ruling: none needed here: the timing rules and their rulings are page 03's.
Found: 2026-10-28, overlay spike handoff (docs/handoffs/2026-10-28-overlay-spike-sitting-handoff.md, removed; `git show 444631e6:docs/handoffs/2026-10-28-overlay-spike-sitting-handoff.md`); listed 2026-10-30, session b02ab3e6 (the reference build).

### IN-05 The song clock is its own class with two getters

Rule: The song clock is class #707. Its constructor and per-frame tick build the raw clock; one getter returns the raw clock minus the audio calibration (the judged time, songTime) and the other returns the raw clock plus video minus audio (the drawn time). The clock keeps the speed ratio at +0x2C and the delay term at +0x58. The clock's rules are on page 03.
Label: Proven in the game's code
Evidence: `code/6142/clock_ctor_16C9A0.asm` lines 22 to 38; `code/6142/clock_tick_16C650.asm` lines 19 to 141 (audio pull at lines 73 to 75); `code/6142/clock_raw_minus_audio_16C480.asm` lines 2 to 4; `code/6142/clock_raw_plus_video_16C490.asm` lines 2 to 4.
Game code (6142): class #707 (scrambled methods): constructor RVA `0x16C9A0`, tick RVA `0x16C650`, judged-time getter RVA `0x16C480`, drawn-time getter RVA `0x16C490`.
Hydra: no owner of a game clock. The delay sign Hydra's Preview uses is `kOffsetDirection` in src/app/preview_source.cpp line 139.
Ruling: none needed here: the clock rules and their rulings are page 03's.
Found: 2026-10-28, overlay spike handoff ("The clock"); listed 2026-10-30, session b02ab3e6 (the reference build).

### IN-06 The song metadata object holds the delay in whole milliseconds at +0x24

Rule: A song metadata object (class #828) keeps the song's delay as whole milliseconds at +0x24. The `song.ini` reader fills it through an integer getter, a second getter hands it out in seconds, and the chart reader writes the chart Offset there only while it is still 0. The rules are on page 03.
Label: Proven in the game's code
Evidence: `code/6142/ini_song_reader_1EFC60.asm` lines 38 to 47; `code/6142/ini_int_getter_21647A0.asm` lines 32 to 66; `code/6142/delay_seconds_1EF1A0.asm` lines 2 to 4; `code/6142/chart_song_metadata_35C980.asm` lines 634 to 663 (the skip when +0x24 is not 0, then the store).
Game code (6142): class #828 reader RVA `0x1EFC60`, seconds getter RVA `0x1EF1A0`; ini getter RVA `0x21647A0`; chart metadata reader (a scrambled SongEntry method) RVA `0x35C980`.
Hydra: `preview_audio_offset_ms` in src/app/preview_source.cpp line 311 and `parse_delay_ms` at line 113 hold Hydra's side of the rule.
Ruling: none needed here: the delay and Offset rules, and their open splits, are page 03's.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-07 The settings loader builds the calibration and speed settings

Rule: The settings loader builds the audio calibration, video calibration and song_speed settings as integer setting objects; the integer setting's constructor hands its default to a base constructor that clamps it into range, and a getter turns song_speed into a ratio. The values and ranges are on page 03.
Label: Proven in the game's code
Evidence: `code/6142/settings_load_374D50.asm` lines 359 to 399 (audio, video) and 2017 to 2035 (song_speed); `code/6142/setting_int_ctor_210C470.asm`; `code/6142/setting_base_ctor_2116210.asm` (the clamp); `code/6142/speed_ratio_210C090.asm` lines 10 to 13.
Game code (6142): loader RVA `0x374D50`; integer setting constructor RVA `0x210C470`; base constructor RVA `0x2116210`; ratio getter RVA `0x210C090`. All names are scrambled.
Hydra: no owner: Hydra reads no calibration or speed setting (grep for "calibrat" and "song_speed" in src/).
Ruling: none needed here: the values and their rulings are page 03's.
Found: 2026-10-28, overlay spike handoff ("What the code says about calibration"); listed 2026-10-30, session b02ab3e6 (the reference build).

### IN-08 CalibrationWindow is a joystick screen, not the audio and video calibration

Rule: The class `CalibrationWindow` (TypeDefIndex 1166) is Rewired's joystick-axis window (deadzone, zero, sensitivity and invert sliders). The audio and video calibration screen is `CalibrationMenu` (TypeDefIndex 992) with `Calibrator` (TypeDefIndex 1281); how that screen picks its values was not read.
Label: Proven in the game's code
Evidence: `dump.cs` lines 62144 to 62270 (namespace Rewired.UI.ControlMapper, the four slider fields); `dump.cs` lines 66629 to 66713 (Calibrator) and 52940 (CalibrationMenu). Class metadata only; no method of either calibration class was read.
Game code (6142): `CalibrationWindow` (no method read); `Calibrator.Update`, RVA `0x2FB0B0`, not read.
Hydra: no owner (grep for `calibrat` in src/ finds nothing).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The drum engine

### IN-09 The drum engine is a sealed class over a generic base

Rule: The drum engine is class #8255, a sealed class whose base is the generic class #8253 instantiated on the drum input struct #8663. #8253 derives from the abstract engine class #8288, which holds nearly every field the tools read; #8255 itself adds a double at +0x2E0.
Label: Proven in the game's code
Evidence: `dump.cs` line 527421 (#8255 and its base) and lines 527421 to 527452 (its fields, the double at 0x2E0); `dump.cs` line 529952 onward (#8288's fields); `overlay-spike/il2cpp-fields.md` (tracked) gives the same layout. Layout and width only; meanings are in the rows below.
Game code (6142): `#8288+0x20`, `#8288+0x30`, `#8288+0x38`, `#8288+0x42`, `#8288+0x48`, `#8288+0x68`, `#8288+0x94`, `#8288+0x120`, `#8288+0x130`, `#8288+0x1B0`, `#8255+0x2E0`; dump text, no listing.
Hydra: no owner. The tool's names for these offsets are the `OFF_` constants in tools/ch_probe/constants.py lines 87 to 147 (for example `OFF_TOTAL_WINDOW`, `OFF_SCORE`).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build), from `tools/ch_probe/constants.py` and the overlay spike.

### IN-10 The engine constructor writes the window constants into the engine

Rule: The drum engine's constructor picks a back and a front window constant, the normal pair (`RVA_CONST_NORMAL_BACK`, `RVA_CONST_NORMAL_FRONT`) or the precision pair (`RVA_CONST_PRECISION_BACK`, `RVA_CONST_PRECISION_FRONT`) by bit 0x1000 of the flags at +0x198, stores them at +0x30 and +0x38, and sets the whole window at +0x20 to twice the back value. The window rules are on page 03.
Label: Proven in the game's code
Evidence: `code/6142/engine_ctor_20DF680.asm` lines 80 to 87 (the four loads and the stores to +0x30 and +0x38); GD lines 1807 to 1821 (the bit test and the +0x20 store).
Game code (6142): #8255 constructor, RVA `0x20DF680`; flags field `#8288+0x198`.
Hydra: no owner here: Hydra's fixed window and its difference from the game are on page 03.
Ruling: none needed here: the hit-window rulings and splits are page 03's.
Found: 2026-09-25, session 8157f2dc (milestone 1 run); re-read 2026-10-30, session b02ab3e6 (the reference build).

### IN-11 The song clock sits at +0x100 and the score at +0x94

Rule: The engine's song clock is the double at +0x100, in seconds, with the same time as whole milliseconds at +0x108. The score is the int at +0x94; it rises only on a hit and ends equal to the score in `scorestats.json`.
Label: Proven in a live run
Evidence: `overlay-spike/run1_cal0_sp_watch.csv` (2026-10-28): 40,922 rows, clock from 2.207 s to 181.160 s, always rising; `overlay-spike/run1_cal0_score_compare.txt` line 48 ("Final: game 365095") against `overlay-spike/run1_cal0_scorestats.json` (score 365095); `RUNS\l3\keys\presses.csv` (142 rows with score before and after each press). The clock's struct (#8496: a double, then a long) is `dump.cs` line 540408.
Game code (6142): `#8288+0x100`, `#8288+0x94`; data fields, no method.
Hydra: no owner: Hydra's score is its own sum. The tool's names are `OFF_SONG_CLOCK` and `OFF_SCORE` in tools/ch_probe/constants.py lines 98 and 101.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc; checked 2026-10-30, session b02ab3e6 (the reference build).

### IN-12 The double at +0x2E0 is the start of a double lane's round, not a hit time

Rule: The tools call the drum engine's double at +0x2E0 the hit time. It is not a per-hit time. The engine writes it, with the song clock, only while it handles a note flagged 0xC000 (a note in a double lane), at the hit that empties the round's pending-pad mask at +0x2C6; the mask is then refilled from +0x2C4 for the next round. The engine compares the song clock minus this value with 0.75 s (IN-15). So on a chart with no double lane the value is never written.
Label: Proven in the game's code
Evidence: `docs/audit/drum-lanes/play/f20deaa0.asm` has two copies of the code. Each tests flag 0xC000 (lines 131 and 179), clears the hit lane's bit in the pending mask at +0x2C6 (lines 137 and 185) and, only when that leaves no lane pending, writes the clock from `[rdi + 0x100]` into +0x2E0 (lines 140 and 188). The reads are in IN-15. The one run that logged the field read 0.0 in every row of `overlay-spike/run1_cal0_sp_watch.csv` (40,922 rows); that fits a chart without double lanes, but the chart was not checked for them. The early "hit time" columns of `tools/ch_probe/experiments/results/hit_detect.csv` were the song clock at +0x100, not this field (session 8157f2dc). Page 02's NL-59 describes the same timer from the game's side.
Game code (6142): `#8255+0x2E0`; writer in the engine method at RVA `0x20DEAA0`, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 140 and 188.
Hydra: no owner. The tool calls it `OFF_HIT_TIME` in tools/ch_probe/constants.py line 106, which is the wrong name.
Ruling: none needed: no Hydra rule reads it; the tool's name is Q-78
Found: 2026-09-25, session 8157f2dc; 2026-10-30, session b02ab3e6 (the reference build).

### IN-13 The combo count and the window note count have no result file

Rule: Session 8157f2dc read +0xB0 as the combo count (it resets on a miss) and +0x8C as the count of notes inside the active window (it goes up and down). No result file holds either field.
Label: Unknown
Evidence: not checked: the only record is the session's printout; no sp_watch file has either column, and the dump names neither field (`overlay-spike/il2cpp-fields.md` line 164 lists both as unnamed ints). Looked in the results folders and the committed listings.
Game code (6142): `#8288+0x8C` and the int at +0xB0; no writer read.
Hydra: no owner. The tool calls +0x8C `OFF_NOTE_COUNT` (tools/ch_probe/constants.py line 147).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc.

### IN-14 Old engine objects stay in memory after a restart

Rule: The game leaves old engine objects in memory, frozen at their last clock and score; a restarted song makes a new one, so the live engine is the one whose clock changes between two reads.
Label: Unknown
Evidence: looked in `overlay-spike/*_stdout.txt`, the results folder and the engine finder's tests; the multi-engine read was printed in session 8157f2dc and its output folder no longer exists.
Game code (6142): not found: no code that frees or keeps engines was read.
Hydra: no owner. The tool relies on it in `find_live_engine` (tools/ch_probe/engine_finder.py).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc.

### IN-15 The hit-threshold constant is a double lane's 0.75 s round timer

Rule: The constant the tools name `RVA_HITCHECK_THRESHOLD` (a double, 0.75, in the DLL's data) is not a hit grade. The drum engine compares the song clock minus the round start at +0x2E0 (IN-12) with it, and when the time is over 0.75 s while a round is pending it makes one virtual call and clears the round state.
Label: Proven in the game's code
Evidence: `RVA_HITCHECK_THRESHOLD = 0x31406E8` (tools/ch_probe/constants.py line 191); the listings annotate the address as `[0x31406e8] xmm dbl=(0.75, 12.0)`. Its two users are `docs/audit/drum-lanes/play/f20ddec0.asm` lines 30 to 45 (clock at +0x100 minus +0x2E0, compared with it; the break needs +0x2C4 and +0x2C8 non-zero and the byte at +0x2F1 zero, then clears +0x2EC, +0x2C4, +0x2C8 and +0x2F0) and its twin `f20de5b0.asm` lines 17 to 19. The virtual call's target was not read. Session 8157f2dc (2026-09-25) printed the value 0.75 earlier; that printout is not kept, and it showed only the value.
Game code (6142): engine methods at RVA `0x20DDEC0` and `0x20DE5B0`, the listings above.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc.

### IN-16 The tool's two window-formula tables carry each other's names

Rule: The window formula's normal branch (flags bit 0x1000 clear) reads the four constants that tools/ch_probe/constants.py files under `RVA_FORMULA_PRECISION`, and the precision branch reads the four filed under `RVA_FORMULA_NORMAL`. So the tool's normal-mode prediction runs on the precision constants.
Label: Proven in the game's code
Evidence: GD lines 767 to 781, read again for this page: the bit-clear branch (lines 772 to 774) multiplies by the addresses listed at constants.py lines 175 to 180, and the bit-set branch (lines 779 to 781) by those at lines 166 to 171. The arithmetic fits the live run: the comment above `WINDOW_CAP_MS` in tools/ch_probe/constants.py (lines 235 to 239) records that the formula with the DLL's constants gives 171.4313075 ms at an input of 170 and 96.2931672 ms at 75, and `tools/ch_probe/experiments/results/poll_windows.csv` holds 171.431308 and 96.293167 as its largest and smallest totals. Session 8157f2dc (2026-09-25) printed the constants' values and found these totals came from the bit-clear table, while the other table gives -242 and -78; that printout is not kept.
Game code (6142): window formula, RVA `0x20DDDA0`; flags field `#8288+0x198`.
Hydra: no owner in src/; the two tables are `RVA_FORMULA_NORMAL` (constants.py line 166) and `RVA_FORMULA_PRECISION` (line 175), and the tool's normal-window prediction (`predicted_window_normal` in tools/ch_probe/experiments/analysis.py, fed by `normal_formula_constants` at lines 312 to 316) reads the first.
Ruling: open split: see questions page Q-76
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The per-note hooks

### IN-17 Three methods run once per note

Rule: On a drums run, three methods run once for every note: `0x20DEEC0` on the drum engine (adds a hit's points; its arguments are the note and base points 50), `0x20DE770` on the drum engine (handles a hit's accent and ghost flags), and `0x100E6E0` on the base #8253 instantiated on #8663. No other method came within one of the note count.
Label: Proven in a live run
Evidence: `RUNS\l3\trace-wm-count.txt` (2026-10-28, bot, Window Map, 142 notes): "once per note (count 142)" lists the three, and of the 44 hooked addresses the only counts above 142 are 2402 and then 20,831 and up (the others read 0); `RUNS\l3\keys\trace-keys-count.txt` (key presses): 139 each, cut short by the trace's cap (see IN-18). The signatures are `dump.cs` lines 527454 to 527455, 527547 to 527548 and 527352 to 527353. Listings of the first two are on page 06 (`code/6142/note_score_add_20DEEC0.asm`, `code/6142/note_hit_dynamics_20DE770.asm`).
Game code (6142): #8255 methods RVA `0x20DEEC0` and `0x20DE770`; #8253 instantiation method RVA `0x100E6E0`.
Hydra: no owner. The tool's names are `RVA_NOTE_SCORE_ADD`, `RVA_NOTE_DYNAMICS` and `PER_NOTE_DEFAULT_RVAS` in tools/ch_probe/constants.py lines 70 to 77.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, session 6a549de0 (feasibility study lines 143 to 156).

### IN-18 A per-note call lands just after the note's time

Rule: On a bot run the per-note calls come a few milliseconds after the note: the first two calls on Window Map came at 3003.0 ms for a note at 3000 ms. A key-press trace stopped at 139 of 142 notes because its cap ran out while the song sat paused; a later run with the cap counted from song time got all 142.
Label: Proven in a live run
Evidence: `RUNS\l3\trace-wm-args.txt` first lines (+3.0 ms); `RUNS\l3\keys\trace-keys-count.txt` (139, "ended: done (agent: cap)"). The study's figures of 284 calls and a 1.75 ms median were not recomputed, the 142-of-142 rerun is L2 run 1 of 2026-10-29 in session e9b04efc (the presser hit 142 of 142, and the trace fired 142 times, one per note, all within 85 ms of the note, the largest gap 4.1 ms; card docs/sessions/2026-10-29-e9b04efc.md, no trace log kept), and "paused" as the cause is the study's inference.
Game code (6142): RVA `0x20DE770` and `0x20DEEC0`.
Hydra: no owner.
Ruling: user: "Song cap + 120 s (Recommended)" (card 12d82c30, 02:03 UTC, to "How long should the new wall-clock cap be?"); it sets the tool's cap, not a game rule.
Found: 2026-10-28, sessions 6a549de0 and 12d82c30 (feasibility study lines 154 to 156).

### IN-19 Whether the score method fires only for hit notes

Rule: A live run recorded that `0x20DEEC0` fired 77 times on a run where the game counted 77 hit notes of 142 and 10 overhits, with every section equal, so it would fire only for hit notes. The trace half of that record is not on disk.
Label: Unknown
Evidence: not checked: it needs the run's trace log. The game's side was the file `l2-run2-scorestats.json` of session e9b04efc, which was not kept (77 hit, 142 total, 10 excess, section hits 2, 8, 12, 15, 11, 10, 5, 8, 3, 3). The run was L2 run 2 on Window Map, 2026-10-29: the user let the opening notes pass and played the rest by hand, with no presser, and the trace stopped with "ended: done (agent: song over)" after 77 calls, on 77 different notes, the trace's count equal to the game's in each of the ten sections. The session's card (docs/sessions/2026-10-29-e9b04efc.md) records the 77 calls, the 77 hits and the ten matching sections.
Game code (6142): #8255 method RVA `0x20DEEC0`.
Hydra: no owner. The tool comment is at tools/ch_probe/constants.py lines 67 to 69.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-29, session e9b04efc (L2 run 2).

### IN-20 What the dynamics method does on a miss is not known

Rule: Only the score method has been traced on a run with misses, so whether `0x20DE770` fires for a missed note is not known.
Label: Unknown
Evidence: looked in `RUNS\l3` (counts only for clean runs and one key run with 3 late notes) and the 2026-10-29 live runs of session e9b04efc (IN-03, IN-18, IN-19, IN-36 and IN-43 carry what they found; none traced this method on a run with misses); GD lines 1112 to 1199 show no branch that names a miss.
Game code (6142): #8255 method RVA `0x20DE770`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-21 The dynamics method compares a per-lane byte with a threshold

Rule: `0x20DE770` picks a signed byte for the note's lane (engine bytes +0x309 to +0x310) and compares it with an unsigned threshold read through the engine; a note flagged one way needs the byte below the threshold and a note flagged the other way at or above it. Which flag is accent and which ghost, and where the lane bytes get their values, was not read.
Label: Proven in the game's code
Evidence: GD lines 1137 to 1162 (the lane byte chosen by the note's lane mask) and 1163 to 1181 (the two comparisons). The page 06 listing of the same method is `code/6142/note_hit_dynamics_20DE770.asm`.
Game code (6142): #8255 method RVA `0x20DE770`.
Hydra: no owner: Hydra reads accents and ghosts from the chart, not from an input value (page 06).
Ruling: none needed: the dynamics scoring rule and its ruling are page 06's.
Found: 2026-10-28, session 6a549de0 (feasibility study line 131); read 2026-10-30, session b02ab3e6 (the reference build).

### IN-22 The address the tools call the hit check belongs to the other instantiation

Rule: `0x100DAB0` is virtual slot 25 of the generic base #8253 instantiated on the 8-byte input struct #8533; the drum engine's own copy of that method is `0x100DAE0`, on the #8663 instantiation. Neither is shown to be a hit check: the drum copy runs every frame and compares the previous input struct with the current one, and `0x100DAB0` never runs in a drums song.
Label: Proven in the game's code
Evidence: `dump.cs` lines 527331 to 527338 (the slot 25 instantiation list: `0x100DAB0` on #8533, `0x100DC80` on #8332, `0x100DAE0` on #8663) and line 527421 (#8255 derives from the #8663 instantiation); `script.json` names agree. Live: `RUNS\l3\trace-wm-count.txt` gives `0x100DAE0` 20,831 calls in a 62 s song with 142 notes. GD lines 2247 to 2275 hold both bodies; the drum copy calls `0x2176020`, a method of #8663 (`dump.cs` lines 552242 to 552243), whose body was not read, so "input changed" is a reading of the compare, not a name.
Game code (6142): #8253 slot 25, RVA `0x100DAB0` (on #8533) and RVA `0x100DAE0` (on #8663); callee RVA `0x2176020`.
Hydra: no owner in src/. The tool names `0x100DAB0` `RVA_HIT_CHECK` (tools/ch_probe/constants.py line 55, comment lines 52 to 54), and tools/ch_probe/experiments/active_probe.py line 215 sets a breakpoint there.
Ruling: open split: see questions page Q-72
Found: 2026-10-28, session 6a549de0 (feasibility tests handoff line 41); dump checked 2026-10-30, session b02ab3e6 (the reference build).

### IN-23 The tool's drums base-constructor address is the other instantiation's

Rule: `0x100E960` is the constructor of #8253 on #8533 and on #8332; the constructor of the #8663 instantiation, the drum engine's base, is `0x100EAE0`.
Label: Proven in the game's code
Evidence: `dump.cs` lines 527295 to 527306 and `script.json` rows for both addresses; `RUNS\l3\trace-wm-count.txt` lists ".ctor (base 0x100eae0)" in the drum engine's hooked set.
Game code (6142): #8253 constructor RVA `0x100EAE0` (on #8663) and RVA `0x100E960` (on #8533 and #8332).
Hydra: no owner in src/. The tool's `RVA_BASE_ENGINE_CTOR_DRUMS` (tools/ch_probe/constants.py line 46) holds `0x100E960`; nothing uses it but a test fixture copy.
Ruling: open split: see questions page Q-77
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-24 A breakpoint written into the game's code crashed it once

Rule: On 2026-09-25, writing breakpoint bytes into the game's code with the debugger crashed Clone Hero, while a bare debugger attach did not. The cause (a code-integrity check) was a guess.
Label: Unknown
Evidence: looked in tools/ch_probe/ and the results folder: no crash record and no result file; the test scripts are not in the repo. The only record is session 8157f2dc's transcript.
Game code (6142): not found.
Hydra: no owner. The tool still has a breakpoint route (tools/ch_probe/debugger.py; active_probe.py line 215).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc.

## The Star Power fields

### IN-25 Five Star Power fields follow the meter live

Rule: The engine keeps the Star Power meter at +0x68 and the full meter at +0x1B0 (a long that never changes during a song), the active byte at +0x42, the multiplier at +0x48 (doubled while active) and the end time at +0x120 (a double in song seconds). The sizes and timings are page 05's.
Label: Proven in a live run
Evidence: `overlay-spike/run1_cal0_sp_compare.txt` (2026-10-28): +0x68 rising by a phrase, +0x1B0 constant, +0x42 and +0x48 flipping together, +0x120 ahead of the clock and moving later when a phrase is collected during Star Power (lines 2 to 40); `overlay-spike/run1_cal0_sp_watch.csv` (40,922 rows).
Game code (6142): `#8288+0x68`, `#8288+0x1B0`, `#8288+0x42`, `#8288+0x48`, `#8288+0x120`.
Hydra: no owner here. The tool's names are `OFF_SP_AMOUNT`, `OFF_SP_MAX`, `OFF_SP_ACTIVE`, `OFF_SP_MULTIPLIER` and `OFF_SP_END_TIME` in tools/ch_probe/constants.py lines 119 to 135.
Ruling: none needed here: the Star Power rules and their splits are page 05's.
Found: 2026-10-28, overlay spike handoff lines 45 to 49; 2026-10-30, session b02ab3e6 (the reference build).

### IN-26 The gain method adds to the meter and a running total, and tests the half mark

Rule: The Star Power gain method adds the amount to the meter at +0x68 and to a running total at +0x80, raises the phrase counter at +0x4C when the gain comes from a phrase, and announces "Star Power available" when the meter crosses the half mark, the long at +0x1B8, while Star Power is off.
Label: Proven in the game's code
Evidence: `code/6142/sp_gain_20F4FA0.asm` lines 19 to 22 (the two adds), 23 to 24 and 37 (the phrase counter), 25 to 36 (the half-mark test), as explained in `code/6142/sp_gain_20F4FA0.md`.
Game code (6142): #8288 gain method (scrambled name), RVA `0x20F4FA0`; fields `#8288+0x68`, `#8288+0x1B8`.
Hydra: no owner here: page 05 holds Hydra's Star Power model.
Ruling: none needed here: the Star Power rules are page 05's.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-27 The flag at +0x130 is set on every gain, not only on an overflow

Rule: The byte at +0x130 is set by the gain method on every gain and also by the per-update clamp when the meter overflows, so it is not only an overflow flag. While Star Power is on, the per-update method recomputes the end time at +0x120 when the byte is set and then clears it; no clear was found in these two methods for the Star-Power-off case, and no other method was searched.
Label: Proven in the game's code
Evidence: `code/6142/sp_gain_20F4FA0.asm` lines 44 to 57 (sets +0x130, then sends the new meter ratio); GD lines 53552 to 53572 (the clamp in the per-update method sets it at line 53556; the test at line 53558 recomputes the end time and the store at line 53572 clears it, all inside the Star-Power-on branch that starts at line 53512). The live data fits that: the flag is 0 in all 11,436 rows with Star Power on, and in `overlay-spike/run1_cal0_sp_watch.csv` it is 1 in 22,890 rows, all with Star Power off and 1440 or 2880 ticks in the meter, and first turns on at 5.715 s with a quarter meter and no clamp.
Game code (6142): `#8288+0x130`; gain method RVA `0x20F4FA0`; per-update method RVA `0x20F6800`.
Hydra: no owner in src/. The tool calls it `OFF_SP_OVERFLOW` (tools/ch_probe/constants.py line 140) and its comment at lines 137 to 140 says it is set only on an overfill.
Ruling: open split: see questions page Q-79
Found: 2026-10-30, session b02ab3e6 (the reference build); the clamp write first read 2026-10-03, session cebf3006.

## Song entries and the checksum

### IN-28 SongEntry holds the checksum and GlobalVariables holds the current song entry

Rule: `SongEntry` (TypeDefIndex 176) holds the song checksum at +0x38. `GlobalVariables` (TypeDefIndex 1048) has a static instance at static offset +0x8 and the current song entry at +0x28.
Label: Proven in the game's code
Evidence: `dump.cs` line 11073 (the class) and its checksum field at 0x38; `dump.cs` lines 55801 to 55830 (currentSongEntry at 0x28, static instance at 0x8). `overlay-spike/il2cpp-fields.md` lines 263 to 300 give the same, and say nothing there was read from a running game.
Game code (6142): `SongEntry.checksum`, `GlobalVariables.currentSongEntry`, `GlobalVariables.instance`; dump text, no listing.
Hydra: no owner of the route. Hydra's own chart id is `hash_chart_file` in src/app/analysis.h line 88; whether it equals this checksum is page 01's song-id row.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build), from the docs survey line 191.

### IN-29 Whether the current song entry is the song being played

Rule: Nothing yet shows that `GlobalVariables.currentSongEntry` holds the playing song during a song; the dump does not say when the game sets it.
Label: Unknown
Evidence: looked in `dump.cs` (layout only) and `overlay-spike/il2cpp-fields.md`; no run has read the route.
Game code (6142): `GlobalVariables.currentSongEntry`; no writer read.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The song-select menu

### IN-30 The song-select classes are located, not read

Rule: The song-select screen's classes are `SongSelect` (TypeDefIndex 347, a BaseMenu), `SongSelectSearch` (237) and `SongSelectPanel` (144). None of their methods was read.
Label: Unknown
Evidence: not checked: it needs a reading of the classes' methods. The class lines are `dump.cs` 20595, 15170 and 9120, checked for this page.
Game code (6142): class headers only; no method read.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The player screen

### IN-56 The player screen keeps its page in an enum field at +0x120

Rule: `PlayerSelectionMenu` (TypeDefIndex 1429, a BaseMenu) holds the page it shows at +0x120. The field's type is an enum nested in the class (TypeDefIndex 1427) with five words: Confirm 0, Instrument 1, Difficulty 2, Modifiers 3 and Ready 4.
Label: Proven in the game's code
Evidence: `dump.cs` line 72822 (the class header), line 72825 (the field at 0x120) and lines 72794 to 72803 (the enum and its five words). This is dump text, not a listing.
Game code (6142): `#1429+0x120`; field only, no method.
Hydra: no owner: Hydra never reads the running game. The tools read the field by name through tools/ch_mcp/menu_fields.json, and `menu_lines` in tools/ch_mcp/frida_tools.py prints it.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07 (task LS-K3-menu-fields), checked in the dump.

### IN-57 The player screen keeps the profile's controller type as a byte enum at +0x1E8

Rule: `PlayerSelectionMenu` holds a controller type at +0x1E8. Its type is a one-byte enum (TypeDefIndex 8309) with four words: Guitar 0, SixFretGuitar 1, Drums 2 and FiveLaneDrums 3.
Label: Proven in the game's code
Evidence: `dump.cs` line 72873 (the field at 0x1E8) and lines 532200 to 532208 (the enum, its byte value field and its four words). This is dump text, not a listing.
Game code (6142): `#1429+0x1E8`; field only, no method.
Hydra: no owner: Hydra never reads the running game. The tools read it through tools/ch_mcp/menu_fields.json.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07 (task LS-K3-menu-fields), checked in the dump.

### IN-58 The player screen has four ints on BaseMenu and two row arrays of its own

Rule: `BaseMenu` (TypeDefIndex 824), the parent of `PlayerSelectionMenu`, holds four ints in a row at +0x3C, +0x40, +0x44 and +0x48. The tools read three of them and call +0x3C the scroll, +0x40 the cursor and +0x48 the visible-row count; those three meanings come from a scout's decompile (IN-60), not from the dump. `PlayerSelectionMenu` adds an array of strings at +0x218 (the tools read it as the row names), an array of the instrument enum (TypeDefIndex 8326) at +0x228 (read as the row instrument ids) and an int at +0x230 (read as the row count); those readings are the decompile's too. It also holds the instrument at +0x240 and the difficulty at +0x241, the player index at `PlayerSelectionMenu.playerIndex`, and bools at +0x1F0, +0x242, +0x244 and +0x246.
Label: Proven in the game's code
Evidence: `dump.cs` lines 44726 and 44742 to 44745 (BaseMenu and its four ints, the one at 0x44 not read here), lines 72885, 72887 and 72888 (the two arrays and the count), lines 72890 and 72891 (instrument and difficulty), line 72828 (playerIndex) and lines 72875, 72892, 72894 and 72896 (the bools). The dump gives only the layout; what each field means beyond its type is IN-60. This is dump text, not a listing.
Game code (6142): `#824+0x3C`, `#824+0x40`, `#824+0x48`, `#1429+0x218`, `#1429+0x228`, `#1429+0x230`, `#1429+0x240`, `#1429+0x241`, `PlayerSelectionMenu.playerIndex`, `#1429+0x1F0`, `#1429+0x242`, `#1429+0x244`, `#1429+0x246`; fields only, no method.
Hydra: no owner: Hydra never reads the running game. The tools read these through tools/ch_mcp/menu_fields.json; which row is highlighted is worked out in `highlighted_row_text` in tools/ch_mcp/frida_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07 (task LS-K3-menu-fields), checked in the dump.

### IN-59 The player screen's instrument and difficulty are the profile's saved copy

Rule: The instrument at +0x240 and the difficulty at +0x241 of `PlayerSelectionMenu` are copies of the profile's saved values, made when the screen opens. They do not follow the cursor while the player picks.
Label: Unknown
Evidence: a read-only scout's decompile of build 6142 (session 5cc1cc07, 2026-10-31) reported it; no listing of it is committed. The dump shows only the two fields' types and offsets (`dump.cs` lines 72890 and 72891), not who writes them. Live run LS-K3 run 2 (2026-10-31) read Guitar and Expert in these two fields; that one read neither proves nor rules out a saved copy.
Game code (6142): `#1429+0x240`, `#1429+0x241`; the writer was not read from a listing.
Hydra: no owner: Hydra never reads the running game.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07, from a decompile of build 6142.

### IN-60 How the player screen moves, from a decompile

Rule: The pad Green confirms the highlighted row and moves to the next page. Yellow and Blue move the cursor, but only when the profile's controller type (IN-57) is Drums or FiveLaneDrums. The highlighted row is row cursor + scroll (IN-58). On the Instrument page, the highlighted row's instrument id is the instrument Green sets; what the ids mean on the other pages was not found. The code also sets the page field (IN-56) to 5, 6 and 7, which the enum does not name. The bool at +0x1F0 looks like a first-time flag, +0x242 like an is-drums flag, +0x244 like a network flag and +0x246 like a lock on going back; those names are guesses from what the code does with them.
Label: Unknown
Evidence: a read-only scout's decompile of build 6142 (session 5cc1cc07, 2026-10-31) reported all of it; no listing is committed, and none of it can be checked in the dump, which gives only names, types and offsets.
Game code (6142): `#1429+0x120`, `#1429+0x1E8`, `#824+0x40`, `#824+0x3C`, `#1429+0x228`, `#1429+0x1F0`, `#1429+0x242`, `#1429+0x244`, `#1429+0x246`; the input methods were not read from a listing.
Hydra: no owner: Hydra never reads the running game. The tools print these fields with `menu_lines` in tools/ch_mcp/frida_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07, from a decompile of build 6142.

## The bot

### IN-31 The bot never gains Star Power

Rule: A bot never gains Star Power meter: the engine's gain method returns before adding anything when the player kind is Bot (or a remote player). On a chart with 11 phrases the bot hit every note and earned 0 phrases, with 0 activations and 0 time in Star Power, so a bot run can never test a Star Power rule.
Label: Proven in the game's code
Evidence: `code/6142/sp_gain_20F4FA0.asm` lines 13 to 16 (the player kind at +0x190 compared with 2 and with 1, each jumping to the return at line 58). Live, 2026-10-28: `RUNS\l1\bot\scorestats-Trippin_SP_Spike-20261028-210052.json` (Drums) and `...-212754.json` (ProDrums): 11 phrases offered, 0 earned, 0 activations, 0 time in Star Power, 1,257 notes hit, is_bot true. The same chart played by keys earned 11 phrases and activated 4 times (`overlay-spike/run1_cal0_scorestats.json`). The listing shows only this route; no other method that adds meter was searched.
Game code (6142): #8288 gain method, RVA `0x20F4FA0`; field `#8288+0x190`.
Hydra: no owner: Hydra has no bot model (no "is_bot" in src/).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, session 6a549de0 (feasibility study line 129); code read 2026-10-30, session b02ab3e6 (the reference build).

### IN-32 The engine's player kind is set once by its constructor

Rule: The engine's player kind is a 4-byte enum at +0x190 (Player 0, RemotePlayer 1, Bot 2, Replay 3), written once in the base engine's constructor. Which caller passes Bot for a bot player was not followed.
Label: Proven in the game's code
Evidence: enum #8553 at `dump.cs` lines 546294 to 546301; the field at `dump.cs` line 530029; the store at 0x20F7BCB, inside `0x20F79E0`, is line 93 of `code/6142/engine_ctor_20F79E0.asm` (page 06's listing), checked for this page.
Game code (6142): `#8288+0x190`; base engine constructor RVA `0x20F79E0`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-33 A remote player's engine adds no score and drains no Star Power

Rule: When the player kind is RemotePlayer, the engine's two score adds return at once and its per-update Star Power method skips the drain step (it still runs the clamp after it); Player, Bot and Replay engines run all of them.
Label: Proven in the game's code
Evidence: `code/6142/add_note_score_20F6EB0.asm` lines 13 to 14, `code/6142/add_points_20F5F70.asm` line 1 and `code/6142/sp_update_20F6800.asm` lines 37 to 38 (each compares +0x190 with 1; the first two jump to their return, the third jumps past the drain to the clamp, listing line 69); the enum at `dump.cs` line 546294.
Game code (6142): RVA `0x20F6EB0`, RVA `0x20F5F70`, RVA `0x20F6800`; field `#8288+0x190`.
Hydra: no owner: Hydra has no network player.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-34 A bot run's total matches Hydra's

Rule: A bot run scores the same total as Hydra: 25,550 on Window Map (142 gems, no phrases) and 290,975 on Trippin SP Spike in ProDrums (1,257 gems, 11 phrases).
Label: Proven in a live run
Evidence: `RUNS\l1\bot\*.json` (game totals, 2026-10-28); `RUNS\l1\window-map.json` and `RUNS\l1\trippin.json` (`hydra_replay` score output, final totals 25550 and 290975). Hydra's gem and phrase counts were not recomputed.
Game code (6142): none read; a live total.
Hydra: the score command of `hydra_replay`.
Ruling: none needed: same rule.
Found: 2026-10-28, session 6a549de0 (feasibility study lines 121 to 127).

### IN-35 The bot hits every accent and ghost on ProDrums

Rule: On ProDrums the bot hits all accents and ghosts: 9 of 9 and 34 of 34 on Trippin SP Spike. One key-press run of the same chart hit 0 of 9 and 0 of 34.
Label: Proven in a live run
Evidence: `RUNS\l1\bot\scorestats-Trippin_SP_Spike-20261028-212754.json` (accent and ghost hits 9 and 34); `overlay-spike/run1_cal0_scorestats.json` (0 and 0, is_bot false; one run).
Game code (6142): none read; the comparison it may come from is IN-21.
Hydra: Hydra scores every chart dynamic as hit (page 06).
Ruling: user: "the code is the source of truth but from experience i can tell you dynamics only work with a real drum kit. keyboard inputs never register dynamics." (session 6a549de0, 01:11 UTC); it explains the key run, not the bot.
Found: 2026-10-28, session 6a549de0 (feasibility study line 131).

### IN-36 The results screen ignores keys after a command-line bot run

Rule: After a command-line bot run the results screen took no keys at all, with Frida detached too. Whether the command-line launch is the cause is not settled.
Label: Proven in a live run
Evidence: one run, L1 on 2026-10-29 in session e9b04efc: on the bot run's results screen the game took no keys, so the user could not go back to the menu, and it took none after the tools detached Frida either. Only that one bot run was tried, and no bot run was compared with a song started from the menu. The session's card (docs/sessions/2026-10-29-e9b04efc.md) records it; `docs/agents/clone-hero-mcp.md` once repeated it and now points here. No key log was kept.
Game code (6142): not found.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-29, session e9b04efc.

### IN-37 A bot run leaves the profile name empty in scorestats.json

Rule: The game writes `scorestats.json` at the end of a command-line bot song, with an empty profile name; the key-press run wrote "Guest". Each song replaces the same file: the game's code writes one fixed file name with File.WriteAllText (SC-35 on page 06). No live run looked for an earlier song's file after a later song, so the live side of the overwrite is not shown here.
Label: Proven in a live run
Evidence: the three `RUNS\l1\bot\*.json` files (profile name empty) and `overlay-spike/run1_cal0_scorestats.json` ("Guest"), 2026-10-28.
Game code (6142): writer not found here (page 06 has the scorestats rows).
Hydra: no owner. The tool backs the file up and restores it in tools/ch_mcp/game_files.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 31 and 119.

### IN-38 Nothing in the code names the "is_bot" string

Rule: The "is_bot" key of `scorestats.json` seems to come from a serializer reading a field name, because no code refers to the string.
Label: Unknown
Evidence: not checked: a scan of 838,088 direct calls and data references found no reference to the "is_bot" string's slot in `stringliteral.json`, while the same scan found the "combo: {0}" references, but a reference through a table the scan does not follow would also look like this. The scan's script and output were not kept.
Game code (6142): not found.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-39 Why scores are switched off is kept in GlobalVariables

Rule: `GlobalVariables.scoreDisabled` (+0x7C) is an enum (#727) with the values None 0, Modifier 1, Bot 2, Pause 3; who writes Bot in 1.1 was not traced.
Label: Unknown
Evidence: not checked: the enum and field are `dump.cs` lines 39726 to 39733 and 55827 (metadata only); in 1.0 the writes are in the 4045 decompile's GameManager.cs lines 465 and 2252 and EndOfSong.cs line 386, not copied.
Game code (6142): `GlobalVariables.scoreDisabled`; no writer read.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-40 Whether the 1.0 bot earns Star Power

Rule: In 1.0 the gain method skips a remote player and any player for which a scrambled test method returns true; what that method tests was not found, so whether the 1.0 bot earns Star Power is not known.
Label: Unknown
Evidence: not checked: the test method is called at the 4045 decompile's BasePlayer.cs lines 636, 755 and 1104 and GameManager.cs line 355, and its body was not located.
Game code (4045): the BasePlayer gain method, `code/4045/fill_deadline.cs` lines 34 to 57 (line 36 is the skip).
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

## Input and logs

### IN-41 Injected keys reach the game

Rule: Keys sent with SendInput reach Clone Hero for all eight drum keys, and a pad flash lasts about one video frame at 60 fps.
Label: Unknown
Evidence: not checked: the Notepad test and the 60 fps measurement are in session 8157f2dc's transcript only, and the video was not opened. Later key-press runs hit every chord with the same keys (`overlay-spike/run1_cal0_play_chart.txt`, not re-opened for this page).
Game code (6142): none (input arrives through the game's Rewired input system).
Hydra: no owner. The tool's key map is `DEFAULT_BINDINGS` (tools/ch_probe/experiments/key_delivery_test.py and the ch_mcp tests).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc.

### IN-42 Whether the kick key scores a 2x kick gem

Rule: The tools press the kick key for both kicks and assume it scores a 2x kick gem.
Label: Unknown
Evidence: not checked: `overlay-spike/run1_cal0_scorestats.json` shows the DoubleKick modifier and 1,257 of 1,257 notes hit, but whether that chart holds any 2x kick gem was not checked.
Game code (6142): not found.
Hydra: the 2x kick read is page 02's.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc.

### IN-43 A command-line song logs only its loading line at first

Rule: A song started from the command line writes only its "Loading Song" line to the session log at first, so waiting for "OnCountdownStart" never works; the tools attach about 5 s after launch.
Label: Unknown
Evidence: not checked: no session-log file was kept; the record is the study's Test 2 text and the L1 live run of 2026-10-29 (session e9b04efc), in which the attach waited 4.8 s so that it came 5 s after the launch, then attached in 1.2 s and read the state "playing".
Game code (6142): not found.
Hydra: no owner. The tool's delay is `ATTACH_DELAY_S` (tools/ch_mcp/frida_tools.py).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study line 139.

### IN-44 The empty hit-detection logs may have a switch

Rule: The game ships empty `HitDetection-Standard.log` and `HitDetection-Validation.log` files; a setting or flag that fills them was suspected and never found.
Label: Unknown
Evidence: looked in the feasibility study (lines 33 and 109) and docs/handoffs/2026-10-28-ch-mcp-results-handoff.md line 39 (removed; `git show 444631e6:docs/handoffs/2026-10-28-ch-mcp-results-handoff.md`); nothing was tested.
Game code (6142): not found.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 33 and 109.

### IN-45 The game's text strings are plain

Rule: The game's 23,972 text strings are not encrypted, the metadata lists 776 source file paths, and the "combo: {0}" string is used by a static BasePlayer method at 0x1168290 for drum inputs.
Label: Unknown
Evidence: not checked: the counts were not redone and the method was not read. The reference scan of IN-38 did find code references to the "combo: {0}" string at that method.
Game code (6142): not read.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 56 and 158.

## Naming across the two builds

### IN-46 Four assemblies are scrambled, by Beebyte Obfuscator 3.13.0

Rule: Only four of the game's assemblies have scrambled names: CloneHero.dll (985 of 1,451 types), StrikeCore.dll (427 of 449), CHNet.Core.dll (167 of 176) and CHNet.Server.dll (15 of 22). Each holds the marker class `__BB_OBFUSCATOR_VERSION_3_13_0`, so the scrambler is Beebyte Obfuscator 3.13.0.
Label: Proven in the game's code
Evidence: the type counts per assembly are the table in `docs/research/2026-10-28-clone-hero-mcp-feasibility.md` lines 45 to 50, a count of the type names with non-ASCII letters per image in `dump.cs` (image list in lines 1 to 100). The marker class is at `dump.cs` lines 74148, 553588, 813701 and 879625 (TypeDefIndex 1450, 8690, 13587 and 15020), checked for this page.
Game code (6142): class metadata only.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 41 to 50; recounted 2026-10-30, session b02ab3e6 (the reference build).

### IN-47 The scrambled method counts per assembly were not recounted

Rule: The study gives scrambled method counts of 7,941 of 11,926, 3,095 of 3,577, 1,426 of 1,614 and 288 of 303 for the four assemblies.
Label: Unknown
Evidence: not checked: it needs a recount of the method lines per image in `dump.cs`.
Game code (6142): method metadata, not counted.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 47 to 50.

### IN-48 Only 164 of 1.0's 539 scrambled class names survive into 1.1

Rule: 1.0 has 539 scrambled class names and 1.1 has 1,594. Only 164 keep both name and shape; 189 keep the name with another shape, 186 are only in 1.0 and 1,241 only in 1.1. So a scrambled name from one build almost never finds the same class in the other.
Label: Proven in the game's code
Evidence: `RUNS\l4\compare-4045-6142.txt` lines 13940 and 13941, the output of tools/ch_mcp/compare_dumps.py over the two builds' `facts.json` files. A comparison of the dumps, not a live run; not rerun for this page.
Game code (6142): both builds' class metadata.
Hydra: no owner. The comparing tool is tools/ch_mcp/compare_dumps.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study line 164.

### IN-49 GameManager went from all scrambled in 1.0 to mostly readable in 1.1

Rule: 1.0's `GameManager` has 63 fields, all scrambled; 1.1's has 60, of which 40 are readable.
Label: Proven in the game's code
Evidence: `RUNS\l4\compare-4045-6142.txt` line 13803 ("GameManager: fields scrambled 63 -> 20, readable 0 -> 40"), which agrees with IN-01.
Game code (6142): `GameManager.songTime` and the other readable fields (IN-01).
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study line 164.

### IN-50 The drum engine's 1.1 name and the shared GameManager field names

Rule: The study says 1.1's drum engine name does not appear in 1.0 at all, and that only 13 GameManager field names appear in both builds.
Label: Unknown
Evidence: not checked: it needs a search of the 4045 `facts.json` for the 1.1 engine name and a field-by-field compare of the two GameManager classes. The compare file's line 5 count of 13 is about classes that appear more than once in a file, not about these fields.
Game code (6142): not checked.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study line 164.

### IN-51 Build 1.0 is a Mono build with plain .NET code

Rule: 1.0 (v1.0.0.4045) is a Mono build: it has no `GameAssembly.dll`, its code is the .NET assembly `Assembly-CSharp.dll`, which ilspycmd turns back into C#, and its `facts.json` has no addresses.
Label: Proven in the game's code
Evidence: the 1.0 install folder holds `MonoBleedingEdge` and no `GameAssembly.dll` (listed for this page); the two decompiles under `code/4045/` were made from `Assembly-CSharp.dll` with ilspycmd on 2026-10-30, in session b02ab3e6 (the reference build). The all-null addresses in `facts.json` are the plan's count (question 6); the Unity version was not checked.
Game code (4045): `code/4045/fill_generator.cs` and `code/4045/fill_deadline.cs` (decompiled C#).
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study line 162; plan question 6.

### IN-52 Two scrambled method names are the same in both builds

Rule: The 1.1 fill generator `0x20D2B30` and the 1.1 Star Power gain method `0x20F4FA0` carry the same scrambled method names as the 1.0 fill generator and the 1.0 meter methods. A shared name is evidence of the same function, not proof of the same body.
Label: Proven in the game's code
Evidence: 6142 `script.json` (the generator's name at `0x20D2B30`) and `dump.cs` line 530117 (the gain method's name); the same names are in `code/4045/fill_generator.cs` line 30 and `code/4045/fill_deadline.cs` lines 7 and 34. Names only.
Game code (6142): generator RVA `0x20D2B30`; gain method RVA `0x20F4FA0`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build).

## The earlier probe and the launch line

### IN-53 The command-line launch flags and what the game does after a bot song

Rule: The game's manual lists the flags `--song <folder>`, `--player <Instrument>,<Difficulty>`, `--profile`, `-i` and `-v`, and a song started with `--song` and `--player` plays with a bot. After the bot song ended, the game was still running 30 s later, on a menu or the results screen.
Label: Unknown
Evidence: not checked: the manual page was not opened and the "still running 30 s later" check is one observation recorded only in the feasibility study (lines 29 and 119). What the tools send is `launch_command` in tools/ch_mcp/bot_run.py (lines 160 to 169): `--song`, `--player`, and `--profile` when a profile is given; the three bot runs behind IN-37 show that launch does play a song to its end.
Game code (6142): not found: the command-line parser was not read.
Hydra: no owner. The tool's command line is `launch_command` in tools/ch_mcp/bot_run.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 29 and 119.

### IN-54 The first auto-player counted a hit when the score rose

Rule: In September 2026 the probe's auto-player (`experiments/play_chart.py`) played the chart "Slipping" and counted a hit each time the score went up; the probe finds the drum engine by scanning memory for the two window constants stored side by side at +0x30 and +0x38.
Label: Proven in a live run
Evidence: `tools/ch_probe/experiments/results/hit_detect.csv` (2026-09-25 run, tracked by commit 283e0288): 60 rows, every `hit` True, the `score` column rising on every row from 1350 to 4850. The file does not name the script that wrote it, and its date is the commit's, not checked against a game log. That the auto-player counts a hit when the score rises is the feasibility study's line 19 (the code of `play_chart.py` was not read for this). The memory scan is the docstring and code of `tools/ch_probe/engine_finder.py`.
Game code (6142): none read; a live run and the tool's own memory scan.
Hydra: no owner. The tool's finder is `find_live_engine` in tools/ch_probe/engine_finder.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, feasibility study line 19; checked 2026-10-30, session b02ab3e6 (the reference build).

## Saved score files

### IN-55 scoredata.bin keeps one best score per chart and instrument

Rule: The player's saved score file, `scoredata.bin` in the game's LocalLow\srylain Inc_\Clone Hero folder (`LOCALLOW_DIR` in tools/ch_mcp/game_files.py line 37), holds one best score per chart per instrument and a play count per song. It stores the chart's hash next to each score, with no song title or artist. A human run changes it (and `scoresext.bin`); the tools restore both after a run.
Label: Unknown
Evidence: no reader of the file's format is committed and the game's writer was not read. Session c8856afd (2026-10-10) decoded the user's copy with a one-off script that was not kept; its finding that the file "stores only the chart hash next to each score, with no title or artist" comes from the session's own record, which is not kept. Session e8435f64 (2026-10-09) states the one-best-score rule from the dmleaderboards backend, which reads the uploaded file; its card (docs/sessions/2026-10-09-e8435f64.md) says dmleaderboards keeps only the best run per chart and copies play_count from the game's save file. That a human run changes both files is from session 6a549de0's own record, which is not kept. No repo file holds any of these.
Game code (6142): not found: the writer was not looked for.
Hydra: no owner. The chart hash is Clone Hero's song id (page 01, CR-10); the tools back the file up and restore it in tools/ch_mcp/game_files.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-10, session c8856afd; 2026-10-09, session e8435f64

## Rows waiting on a live run

IN-19, IN-20, IN-21, IN-27, IN-29, IN-36, IN-40, IN-44, IN-59, IN-60. See `live-checks.md`.

## Open splits

IN-16 (Q-76), IN-12 (Q-78), IN-22 (Q-72), IN-23 (Q-77), IN-27 (Q-79). See `questions.md`.
