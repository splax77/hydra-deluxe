# Internals [IN]

This page covers the game's classes and fields that the tools read, the per-note hooks, the bot, the game's MIDI kit, and how the two builds name things.
It describes build 1.1 (v1.1.0.6142), and says where 1.0 (v1.0.0.4045) differs.
Last write-back: 2026-11-12.

Most rows here are about the game's own layout, which no Hydra rule reads. Their Ruling line says so. The rules these fields feed (timing, Star Power, scoring) live on pages 03, 05 and 06. Paths that start `RUNS` mean `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\`; `GD` means Ghidra's full decompile `ghidra_decompile_all.txt`, which `code/README.md` describes (it is not in the repo); `dump.cs` is `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs`; `overlay-spike` means `tools/ch_probe/experiments/results/overlay-spike/`, whose result files are in the repo.

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
Hydra: no owner. The tool's names for these offsets are the `OFF_` constants in tools/ch_probe/constants.py lines 87 to 151 (for example `OFF_TOTAL_WINDOW`, `OFF_SCORE`).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build), from `tools/ch_probe/constants.py` and the overlay spike.

### IN-10 The engine constructor writes the window constants into the engine

Rule: The drum engine's constructor picks a back and a front window constant, the normal pair (`RVA_CONST_NORMAL_BACK`, `RVA_CONST_NORMAL_FRONT`) or the precision pair (`RVA_CONST_PRECISION_BACK`, `RVA_CONST_PRECISION_FRONT`) by bit 0x1000 of the flags at +0x198, stores them at +0x30 and +0x38, and sets the whole window at +0x20 to twice the back value. The window rules are on page 03.
Label: Proven in the game's code
Evidence: `code/6142/engine_ctor_20DF680.asm` lines 80 to 87 (the four loads and the stores to +0x30 and +0x38); GD lines 1807 to 1821 (the bit test and the +0x20 store). A byte scan of the DLL found fifteen sites that test bit 0x1000 of +0x198 (`docs/audit/note-shuffle/scout-flag/scan_198_1000.txt`, "candidates 2899 hits 15"). Two of them are named on these pages: this constructor and the window formula (IN-16). The other thirteen, among them a BasePlayer method and copies in the generic engine instantiations, were not read.
Game code (6142): #8255 constructor, RVA `0x20DF680`; flags field `#8288+0x198`.
Hydra: no owner here: Hydra's fixed window and its difference from the game are on page 03.
Ruling: none needed here: the hit-window rulings and splits are page 03's.
Found: 2026-09-25, session 8157f2dc (milestone 1 run); re-read 2026-10-30, session b02ab3e6 (the reference build); the fifteen sites, 2026-11-08, session 77057df2 (docs-gaps search).

### IN-11 The song clock sits at +0x100 and the score at +0x94

Rule: The engine's song clock is the double at +0x100, in seconds, with the same time as whole milliseconds at +0x108. The score is the int at +0x94; it rises only on a hit and ends equal to the score in `scorestats.json`.
Label: Proven in a live run
Evidence: `overlay-spike/run1_cal0_sp_watch.csv` (2026-10-28): 40,922 rows, clock from 2.207 s to 181.160 s, always rising; `overlay-spike/run1_cal0_score_compare.txt` line 48 ("Final: game 365095") against `overlay-spike/run1_cal0_scorestats.json` (score 365095); `RUNS\l3\keys\presses.csv` (142 rows with score before and after each press). The clock's struct (#8496: a double, then a long) is `dump.cs` line 540408.
Game code (6142): `#8288+0x100`, `#8288+0x94`; data fields, no method.
Hydra: no owner: Hydra's score is its own sum. The tool's names are `OFF_SONG_CLOCK` and `OFF_SCORE` in tools/ch_probe/constants.py lines 98 and 101.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc; checked 2026-10-30, session b02ab3e6 (the reference build).

### IN-12 The double at +0x2E0 is the start of a double lane's round, not a hit time

Rule: The drum engine's double at +0x2E0 is the round start, not a per-hit time. The hit routine writes it, with the song clock, while it handles a note that carries either lane bit of 0xC000, at the hit that empties the round's pending-pad mask at +0x2C6; the mask is then refilled from +0x2C4 for the next round. A note in a single lane carries 0x4000 (page 02, NL-56, which is itself Unknown), so the test passes on single lanes too. That hit is one writer, not the only one: the Evidence lists three more listings that store the clock at +0x2E0. Two of them (`f20de420.asm` and `f20dcad0.asm`) do it right after they add a note's lanes to both masks (+0x2C4 and +0x2C6), and `f20de9a0.asm` repeats the hit routine's empty-mask step. What calls those three was not traced here. The engine compares the song clock minus this value with 0.75 s (IN-15).
Label: Proven in the game's code
Evidence: the writers of +0x2E0 are the constructor (`code/6142/engine_ctor_20DF680.asm` line 65, which zeroes it), `docs/audit/drum-lanes/f20de420.asm` line 25, `docs/audit/drum-lanes/play/f20de9a0.asm` line 53, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 140 and 188, and `docs/audit/drum-lanes/f20dcad0.asm` line 56 (a search of every committed listing for a store to +0x2E0, 2026-11-08). `docs/audit/drum-lanes/play/f20deaa0.asm` has two copies of the hit code. Each tests flag 0xC000 (lines 131 and 179), which passes when either bit is set, clears the hit lane's bit in the pending mask at +0x2C6 (lines 137 and 185) and, only when that leaves no lane pending, writes the clock from `[rdi + 0x100]` into +0x2E0 (lines 140 and 188). The reads are in IN-15. The one run that logged the field read 0.0 in every row of `overlay-spike/run1_cal0_sp_watch.csv` (40,922 rows); that fits a chart without double lanes, but the chart was not checked for them. The early "hit time" columns of `tools/ch_probe/experiments/results/hit_detect.csv` were the song clock at +0x100, not this field (session 8157f2dc). Page 02's NL-59 describes the same timer from the game's side.
Game code (6142): `#8255+0x2E0`; writer in the engine method at RVA `0x20DEAA0`, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 140 and 188.
Hydra: no owner in `src/`; the tool name is `OFF_ROUND_START` in tools/ch_probe/constants.py line 106.
Ruling: D317: the user chose "Rename to round start (Recommended)"; no Hydra rule reads it
Found: 2026-09-25, session 8157f2dc; 2026-10-30, session b02ab3e6 (the reference build); the single-lane case and the writers, 2026-11-08, session 77057df2 (docs-gaps search).

### IN-13 The streak is at +0x88, +0xB0 counts overhits, and +0x8C counts the window's notes

Rule: The int at +0x88 is the streak: the combo break zeroes it. The int at +0xB0 is not the combo count: it is bumped on the path for a press that matched no note, so it counts overhits. The int at +0x8C counts the notes in the active window: it is zeroed by a method the drum engine's update calls first, and bumped once for each note the window update adds.
Label: Proven in the game's code
Evidence: `docs/audit/drum-lanes/play/f20f4b60_combo_break.asm` line 28 stores 0 to +0x88 (IN-87). `docs/audit/drum-lanes/play/f20de9a0.asm` line 44 adds one to +0xB0, at the end of the path the hit routine takes for pressed pads that matched no note (`docs/audit/drum-lanes/play/f20deaa0.asm` lines 205 to 209). `docs/audit/drum-lanes/play/f20f5540_slot9_base_and_activate_sp.asm` line 7 zeroes the 8 bytes at +0x8C (so +0x90 with it); the update at `0x20DDEC0` calls that method at line 16 of `docs/audit/drum-lanes/play/f20ddec0.asm`. And `docs/audit/drum-lanes/play/f20f5810_base_update.asm` line 131 adds one to +0x8C right after a note is appended to the window list (lines 127 to 130). Session 8157f2dc had read +0xB0 as the combo count from a printout that is not kept; the listings contradict that reading. That excess_hits in `scorestats.json` is this counter was not traced.
Game code (6142): `#8288+0x8C`, the ints at +0x88 and +0xB0; combo break RVA `0x20F4B60`; window update RVA `0x20F5810`; RVA `0x20F5540`
Hydra: no owner. The tool calls +0x8C `OFF_NOTE_COUNT` (tools/ch_probe/constants.py line 151).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc; the listings, 2026-11-08, session 77057df2 (docs-gaps search).

### IN-14 Old engine objects stay in memory after a restart

Rule: The game leaves old engine objects in memory, frozen at their last clock and score; a restarted song makes a new one, so the live engine is the one whose clock changes between two reads.
Label: Unknown
Evidence: looked in `overlay-spike/*_stdout.txt`, the results folder and the engine finder's tests; the multi-engine read was printed in session 8157f2dc and its output folder no longer exists. The restart half is also stated in tools/ch_probe/README.md lines 42 to 43 ("A restart makes a new engine object rather than winding the old one back to zero"), which is tool prose, not a run record; it is the lead for a check, not proof. On 2026-11-08 every run folder under `RUNS` (2026-10-28 to 2026-11-08) was grepped for an engine address and for candidate counts. No run output shows two engines. Three files name one engine address each, in three different game processes (`RUNS\2026-10-28\l3\keys\play_chart.txt`, `RUNS\2026-11-07\ls-o2-run1\replies.txt` and `RUNS\2026-11-07\ls-r1-run1\replies.txt`). The engine finder prints no candidate count: it prints a dot per try and returns the first engine whose clock moved (tools/ch_probe/engine_finder.py lines 202 to 231), so its output cannot show a second engine. The `"instances"` field in the run replies counts `GameManager` objects (tools/ch_mcp/agent/clock.ts lines 16 to 49), not engines. No run restarted a song. Proof needs a finder that prints every candidate's clock, run across a restart.
Game code (6142): not found: no code that frees or keeps engines was read.
Hydra: no owner. The tool relies on it in `find_live_engine` (tools/ch_probe/engine_finder.py line 169).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc; the README lead, 2026-11-08, session 77057df2 (docs-gaps search); the run-folder search, 2026-11-08, session 428cf079 (unknowns audit).

### IN-15 The hit-threshold constant is a double lane's 0.75 s round timer

Rule: The constant the tools name `RVA_HITCHECK_THRESHOLD` (a double, 0.75, in the DLL's data) is not a hit grade. The drum engine compares the song clock minus the round start at +0x2E0 (IN-12) with it, and when the time is over 0.75 s while a round is pending it calls the combo break (virtual slot 20, IN-87) and clears the round state.
Label: Proven in the game's code
Evidence: `RVA_HITCHECK_THRESHOLD = 0x31406E8` (tools/ch_probe/constants.py line 195); the virtual call is `call qword ptr [rax + 0x278]` at `docs/audit/drum-lanes/play/f20ddec0.asm` line 45, which is slot 20 (the vtable starts at +0x138 and each slot takes 0x10, as the slot arithmetic at `docs/audit/drum-lanes/play/f20f4b60_combo_break.asm` lines 77 to 84 shows). `dump.cs` line 530260 gives slot 20 of #8288 as RVA `0x20F4B60`, and no override of slot 20 appears in the drum engine's or its generic base's method lines (dump.cs lines 527280 to 527700, searched 2026-11-08). The same slot is called on the overhit path (`docs/audit/drum-lanes/play/f20de9a0.asm` line 43), as page 02's NL-59 says. The listings annotate the address as `[0x31406e8] xmm dbl=(0.75, 12.0)`. Its two users are `docs/audit/drum-lanes/play/f20ddec0.asm` lines 30 to 45 (clock at +0x100 minus +0x2E0, compared with it; the break needs +0x2C4 and +0x2C8 non-zero and the byte at +0x2F1 zero, then clears +0x2EC, +0x2C4, +0x2C8 and +0x2F0) and its twin `f20de5b0.asm` lines 17 to 19, a copy with no known caller. That rests on a search of 2026-11-08: no committed listing under `docs/audit` or `docs/clone-hero/code` calls the virtual address 1820de5b0, and `dump.cs` line 527574 gives the method as private with no vtable slot, so only a direct call could reach it. The DLL itself was not searched for one; `docs/audit/drum-lanes/play/xref_callers.py` names it, but its output was not kept. Session 8157f2dc (2026-09-25) printed the value 0.75 earlier; that printout is not kept, and it showed only the value.
Game code (6142): engine methods at RVA `0x20DDEC0` and `0x20DE5B0`, the listings above; slot 20, RVA `0x20F4B60`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc; the slot-20 target, 2026-11-08, session 77057df2 (docs-gaps search).

### IN-16 The tool's two window-formula tables carry each other's names

Rule: The window formula's normal branch (flags bit 0x1000 clear) reads the four constants that tools/ch_probe/constants.py files under `RVA_FORMULA_PRECISION`, and the precision branch reads the four filed under `RVA_FORMULA_NORMAL`. So the tool's normal-mode prediction ran on the precision constants, until it was removed with the rest of the old probe scripts.
Label: Proven in the game's code
Evidence: GD lines 767 to 781, read again for this page: the bit-clear branch (lines 772 to 774) multiplies by the addresses listed at constants.py lines 179 to 184, and the bit-set branch (lines 779 to 781) by those at lines 170 to 175. The arithmetic fits the live run: the comment above `WINDOW_CAP_MS` in tools/ch_probe/constants.py (lines 240 to 243) records that the formula with the DLL's constants gives 171.4313075 ms at an input of 170 and 96.2931672 ms at 75, and `tools/ch_probe/experiments/results/poll_windows.csv` holds 171.431308 and 96.293167 as its largest and smallest totals. Session 8157f2dc (2026-09-25) printed the constants' values and found these totals came from the bit-clear table, while the other table gives -242 and -78; that printout is not kept.
Game code (6142): window formula, RVA `0x20DDDA0`; flags field `#8288+0x198`.
Hydra: no owner in src/; the two tables are `RVA_FORMULA_NORMAL` (constants.py line 170) and `RVA_FORMULA_PRECISION` (line 179), and the constants read in tools/ch_probe/engine.py (EngineModel's constants method) files each table's values under that table's name. The tool's normal-window prediction read the first; it lived in tools/ch_probe/experiments/analysis.py (removed; `git show a081f660:tools/ch_probe/experiments/analysis.py`).
Ruling: open split: see questions page Q-76
Found: 2026-10-30, session b02ab3e6 (the reference build).

### IN-87 The combo break zeroes the streak and tells the listener the old one

Rule: The combo break, virtual slot 20 of the base engine, keeps the old streak, zeroes the streak at +0x88, calls slot 10 and one more engine method, and then sends an event that carries the old streak to the engine's listener at +0x1A0. For a remote player (kind 1 at +0x190) it skips the zeroing and both calls, and still sends the event. Before the kind test, for every player kind, it also calls one more method when the byte at +0x46 is set, and clears that byte; what that method does was not read.
Label: Proven in the game's code
Evidence: `docs/audit/drum-lanes/play/f20f4b60_combo_break.asm`: line 15 reads the old streak, lines 17 to 23 make the +0x46 call (the vtable call at line 22), lines 24 and 25 test the kind and jump past the reset, line 28 zeroes +0x88, line 30 calls slot 10 (`[rax + 0x1d8]`), line 34 calls another engine method, lines 39 to 69 fetch the listener at +0x1A0 and call it with the old streak in `r9d` (line 63). Slot 10 of the base is the multiplier method `0x20F52E0` (page 06, SC-05); no override of it was found in the drum engine's dump lines, but which method runs there was not traced in a run. The method's signature, two bools, is `dump.cs` line 530260.
Game code (6142): #8288 slot 20, RVA `0x20F4B60`; fields `#8288+0x190`, the int at +0x88
Hydra: no owner: Hydra's paths never miss a note.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 77057df2 (docs-gaps search).

### IN-88 A flag at +0xF0 marks time inside a flagged fill's window, and overhits are ignored then

Rule: A method the drum update calls walks a list held at +0x1F8 from a saved index, and sets the byte at +0xF0 while the clock's tick (+0x108) is inside an item (from its +0x20 to its +0x20 plus +0x28). The overhit path returns at once while that byte is set. The list's items are fill windows (class #8323). There is one for each fill of the track whose byte at +0x20 is not zero. Each item holds the fill's start time at +0x10, its length in seconds at +0x18, its start tick at +0x20 and its length in ticks at +0x28. What the fill's +0x20 byte means is page 04's open question.
Label: Proven in the game's code
Evidence: `docs/audit/drum-lanes/play/f20f5540_slot9_base_and_activate_sp.asm` lines 14 to 57 (the list at +0x1F8, the index at +0x294, the bounds at lines 29 to 35, the old value copied to +0xF1 at line 23, the set at line 56 and the clear at line 53); `docs/audit/drum-lanes/play/f20de9a0.asm` line 3 (the overhit path's test of +0xF0). The items: `code/6142/fill_windows_build_20D26A0.asm`, explained in `code/6142/fill_windows_build_20D26A0.md`, walks the track's fill list (lines 43 to 58), skips each fill whose +0x20 byte is zero (lines 68 and 69), and fills a new #8323 (lines 86 to 97). The engine's +0x1F8 is field +0x18 of the struct #8263 that the base engine holds at +0x1E0 (`dump.cs` lines 530043 and 528231), a list of #8323. The builder's result is passed as the fifth argument of the struct's constructor `0x20E8D30` (`code/6142/note_builder_20D2EA0.asm` lines 338 and 339, 420 and 421, and 424). The clock at +0x108 is a tick, not whole milliseconds (page 04, FL-22).
Game code (6142): RVA `0x20F5540`; fields at +0xF0, +0x1F8 and +0x294; window builder RVA `0x20D26A0`
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 77057df2 (docs-gaps search); the items, 2026-11-08, session 428cf079 (unknowns audit).

### IN-89 Thirty-four one-line note getters test bits of three note fields

Rule: The game has 34 one-line getters on the note class that each test bits of one note field. Twenty-three test the flag word at +0x7C (bits 0x1, 0x2, 0x4, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000 and 0x800000 one at a time, and the masks 0x2A, 0x32 and 0xC0000). Eight test the byte at +0x84 (bits 0x1, 0x2, 0x4, 0x8, 0x10 and 0x20, and the mask 0x3). Three read the type word at +0x80 (the mask 0x1E, the cymbal mask 0xE0, and the value 1). Hopo (0x10) at +0x7C and IsSustaining (0x8) at +0x84 are each tested by two getters, which is why the counts are larger than the lists of bits. The two flag fields are typed by enums whose member names survive in the dump, so the 31 getters on +0x7C and +0x84 each test a named bit. On +0x7C (enum #8589): Chord 0x1, Disjoint 0x2, Child 0x4, Hopo 0x10, Strum 0x20, Tap 0x40, StarPower 0x80, StarPowerEnd 0x100, SoloBegin 0x200, SoloEnd 0x400, DrumActivation 0x800, DrumsDisco 0x1000, DrumsDiscoNoFlip 0x2000, DrumsGhostHit 0x40000, DrumsAccentHit 0x80000, DrumsDoubleKick 0x100000, DrumsCymbal 0x200000, DrumsTom 0x400000 and DrumsSnare 0x800000. The mask 0x32 is Disjoint, Hopo and Strum (that getter is true when any of the three is set: `and al,0x32; cmp al,1; seta`), 0x2A is Disjoint, ExtendedSustain and Strum, and 0xC0000 is ghost or accent. On +0x84 (enum #8590): Hit 0x1, Missed 0x2, DynamicsHit 0x4, IsSustaining 0x8, WasSqueezed 0x10 and WasSqueezeLoss 0x20, and the mask 0x3 is hit or missed. The word at +0x80 is a plain ushort that no enum names, so what the three getters on it mean is not known, beyond page 06's reading of 0xE0 as the cymbal mask.
Label: Proven in the game's code
Evidence: `docs/audit/note-shuffle/scout-flag/note_getters.txt` lines 1 to 34, one getter per line with its instructions, gives each getter's field and bit. The names are the member names of the field's enum in `dump.cs`: the note class #8597 at line 548368, its +0x7C typed #8589 (line 548385), +0x80 a ushort (line 548386) and +0x84 typed #8590 (line 548388), enum #8589 from line 548192 and enum #8590 from line 548224. These are names read from the dumped metadata, not from code that uses them. Every getter's direct callers are in `docs/audit/2026-11-08-clone-hero-unknowns/W10a/xref_note_getters.txt`, summed per getter by `W10a/getter_callers_summary.py` into `W10a/getter_callers_summary.txt` beside it (byte scan of il2cpp and .text). Four getters have no direct caller found (byte scan of il2cpp and .text): `0x214B620` (DrumsDiscoNoFlip), `0x214B860` (the 0x32 mask), `0x214B870` (the 0x2A mask) and `0x214BDF0` (IsSustaining). For the +0x80 word, enum #8591 (Open 1, Green 2, Red 4, Yellow 8, Blue 16, Orange 32, `dump.cs` line 548239) would read 0x1E as the four middle frets and 1 as open, but nothing ties that enum to +0x80. Named on other pages: `0x214BB40` (+0x84 bit 4, DynamicsHit, page 06 SC-12), `0x214BE30` (the cymbal mask, SC-02), `0x214BE10` (+0x7C bit 0x80000, accent) and `0x214B6C0` (bit 0x40000, ghost) (SC-13), and `0x214BB50` (+0x7C bit 0x800, called by the hit routine at `docs/audit/drum-lanes/play/f20deaa0.asm` line 124).
Game code (6142): the getters listed in `docs/audit/note-shuffle/scout-flag/note_getters.txt`
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 77057df2 (docs-gaps search); the names and callers, 2026-11-08, session 428cf079 (unknowns audit).

### IN-90 The drum engine's constructor sets starting values of fields no page names

Rule: Before the window constants (IN-10), the drum engine's constructor sets +0x2F2 to 1, copies the first value of its set-up argument to +0x318, takes +0x2F6 from the low byte of the argument's +0x18 and the dynamics threshold at +0x2C0 (page 06, SC-13) from the high half of the same 8 bytes, zeroes the round fields (+0x2C4, +0x2C8, +0x2D0, +0x2D8, +0x2E0, +0x2E8, +0x2F0), stores four times a whole number read from +0xB8 of the object at +0x230 into +0x300, and takes +0x308 from a byte of its argument. Proven in the game's code: +0x300 is four times the chart's +0xB8, and its only readers are the three fill clears, which add it to the clock's tick to make the fill deadline (page 04, FL-23). That +0xB8 is the chart's resolution is not shown, because the tick conversion reads the chart's +0xB0 and that +0xB0 equals +0xB8 was not shown. What the other fields mean is not known.
Label: Unknown
Evidence: `code/6142/engine_ctor_20DF680.asm` lines 22 (+0x2F2), 45 to 47 (+0x318), 49 to 53 (+0x2F6 and +0x2C0), 54 to 69 (the zeroes, and +0x300 from the +0x230 object's +0xB8, truncated at line 57 and shifted left by 2 at line 68) and 70 to 73 (+0x308). The readers of +0x300 are the gain override `0x20DDB70` (`code/6142/fill_deadline_20DDB70.asm` line 24) and `0x20DF3E0` and `0x20DE4C0` (line 13 of `code/6142/fill_clear_20DF3E0.asm` and of `code/6142/fill_clear_c1_20DE4C0.asm`), found by a disassembly pass over every method of #8255 that lists each read and write at these offsets (direct reads in #8255's own methods only). The same pass names the readers of the other fields, whose methods were not read: +0x2F2 is read only by slot 18 `0x20DDC80` (`docs/audit/drum-lanes/play/f20ddc80.asm` line 3); +0x318 by `0x20DE240` and `0x20DF4C0`, which pass it to a runtime helper at 0x2EFBCF0, which has no line in `dump.cs` (`code/6142/lane_flag_calls_20DF4C0.asm` lines 14 to 22); +0x2F6 only by the input poll `0x20DE070`; +0x308 by `0x20DE070`, `0x20DE680` and `0x20DE9A0`; +0x2D0 and +0x2E8 only by slot 4 `0x20DCAD0`; +0x2D8 and +0x2E0 by slot 9 `0x20DDEC0` and by `0x20DE5B0`. The base constructor's quarter bar reads +0xB8 of the same object (`code/6142/engine_ctor_20F79E0.asm` lines 120 to 125 and 158 to 166), and a live run read it as 192 on a resolution-192 chart (`code/6142/engine_ctor_20F79E0.md` line 21). `dump.cs` lines 543378 and 543379 give the chart's +0xB0 and +0xB8 as doubles.
Game code (6142): #8255 constructor, RVA `0x20DF680`; `#8255+0x2E0`; fill clears RVA `0x20DDB70`, RVA `0x20DF3E0` and RVA `0x20DE4C0`
Hydra: no owner, except +0x300: Hydra's fill lead is `kCh11FillLeadBeats` of 4 beats times the chart's resolution (src/search/graph.h line 71, src/search/graph.cpp lines 66 to 70). How each side uses it is page 04's FL-23.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 77057df2 (docs-gaps search); the readers, 2026-11-08, session 428cf079 (unknowns audit).

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
Hydra: no owner in src/. The tool names `0x100DAB0` `RVA_HIT_CHECK` (tools/ch_probe/constants.py line 55, comment lines 52 to 54). The runner that set a breakpoint there was tools/ch_probe/experiments/active_probe.py (removed; `git show a081f660:tools/ch_probe/experiments/active_probe.py`), line 215.
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
Hydra: no owner. The tool still has a breakpoint route (tools/ch_probe/debugger.py). The runner that wrote breakpoints into the game's code was tools/ch_probe/experiments/active_probe.py (removed; `git show a081f660:tools/ch_probe/experiments/active_probe.py`), line 215.
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
Hydra: no owner of the route. Hydra's own chart id is `hash_chart_file` in src/app/analysis.h line 103; whether it equals this checksum is page 01's song-id row.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build), from the docs survey line 191.

### IN-29 Whether the current song entry is the song being played

Rule: Proven in the game's code: the song-load method `0x235AD0` sets `GlobalVariables.currentSongEntry` to the song it loads, before it builds the chart. Whether the field still holds that song for the whole song is not known, because ten other stores to it were not read.
Label: Unknown
Evidence: looked in `dump.cs` (layout only) and `overlay-spike/il2cpp-fields.md`. The store: `code/6142/globals_set_song_235AD0.asm` line 47 stores the song argument at +0x28 with no test in front of it (`this` from line 18, the song from line 17); the chart builder `0x20D2EA0` is called later, at line 110. A field-write scan for +0x28 over every named method that loads GlobalVariables' type info (`docs/audit/2026-11-08-clone-hero-unknowns/W9a/globals_writes_28_7C.txt`) found 16 stores: this one, two in each of two `SongSelect` methods (`0x891D0` and `0x8D260`), five in `EndOfSong` methods, five in `PauseMenu` methods and one in `0x26A130`. The `PauseMenu` and `EndOfSong` stores were not read, and they are where to read next. The song-load method's direct callers (byte scan of il2cpp and .text, `W9a/callers_235AD0.txt`) are `SongOptions` method `0x297240` and a coroutine at `0x3BD710`; neither was read, so the menu route to it is not shown. One run has read the route: on a command-line start of Window Map, the entry's folder during the song was that song's folder (decision log D316 item 4). That compares the folder, not the checksum `live-checks.md` asks for, and no menu start has been read.
Game code (6142): `GlobalVariables.currentSongEntry`; song-load method RVA `0x235AD0`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build). 2026-11-07, session 28b91640 (the folder read). The writer, 2026-11-08, session 428cf079 (unknowns audit).

## The song-select menu

### IN-30 The song-select classes are located, not read

Rule: The song-select screen's classes are `SongSelect` (TypeDefIndex 347, a BaseMenu), `SongSelectSearch` (237) and `SongSelectPanel` (144). The method that starts a song from the list has not been found.
Label: Unknown
Evidence: the class lines are `dump.cs` 20595, 15170 and 9120. All 70 method RVAs of `SongSelect` (`dump.cs` lines 20595 to 20904) were disassembled with tools/ch_mcp/chdis.py, with no listing kept, and searched for `LoadScene`, `SceneManager`, `PlayerSelection`, string literals and reads of +0x28; none starts a song. `SongSelect` overrides the BaseMenu slots 10 (`0x8AC20`), 21 (`0x8EA10`), 36, 51 (`0x8A210`), 57 (Green held) and 47 (`Update`, `0x8EE20`, which only jumps to `BaseMenu.Update`); the slot-to-press map is `code/6142/menu_buttons_1D4BC0.md`. It does not override slot 31, the Green press, whose base method `0x1D5620` only sets the menu's +0x28 and calls `0x1A6EF0` (read with chdis, not kept). So a Green press reaches only the base slot 31, and the overrides of the held-Green and other slots were not read for a song start. `SongSelectSearch` and `SongSelectPanel` were not read. The lead for the next read is `SongOptions` method `0x297240`: its Green handler calls the song-load method `0x235AD0` (IN-29), found as a direct caller by byte scan.
Game code (6142): class headers; `SongSelect` methods read with chdis, no listing kept.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build); the method search, 2026-11-08, session 428cf079 (unknowns audit).

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

Rule: `PlayerSelectionMenu` holds a controller type at +0x1E8. Its type is a one-byte enum (TypeDefIndex 8309) with four words: Guitar 0, SixFretGuitar 1, Drums 2 and FiveLaneDrums 3. The screen copies it from its profile's "controller_type" setting (IN-65) in OnEnable and again in Update; the profile screen writes that setting when its string reads "Controller" (IN-67).
Label: Proven in the game's code
Evidence: `dump.cs` line 72873 (the field at 0x1E8) and lines 532200 to 532208 (the enum, its byte value field and its four words). The copies were read with tools/ch_mcp/chdis.py on 2026-10-31, with no listing: OnEnable `0x32BF80` (`dump.cs` line 72986) calls the profile's controller-type getter `0x2123320` at 0x32C2CE and stores its byte at 0x32C2D7, and Update `0x3314E0` (`dump.cs` line 72974) does the same at 0x3315CC and 0x3315D3. Update reaches the profile through the menu's +0x20 and then +0x10 (0x3315B8 and 0x3315C1); OnEnable's path to it was not read. The getter reads the setting at profile +0x60 (0x2123345) and returns it as enum #8309 (`dump.cs` line 541772).
Game code (6142): `#1429+0x1E8`; OnEnable RVA `0x32BF80`, Update RVA `0x3314E0`, getter RVA `0x2123320`.
Hydra: no owner: Hydra never reads the running game. The tools read it through tools/ch_mcp/menu_fields.json.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07 (task LS-K3-menu-fields), checked in the dump; the copies read 2026-10-31, session b15bb484 (task D1).

### IN-58 The player screen has four ints on BaseMenu and two row arrays of its own

Rule: `BaseMenu` (TypeDefIndex 824), the parent of `PlayerSelectionMenu`, holds four ints in a row at +0x3C, +0x40, +0x44 and +0x48. The tools read three of them and call +0x3C the scroll, +0x40 the cursor and +0x48 the visible-row count; those three meanings come from a scout's decompile (IN-60), not from the dump. `PlayerSelectionMenu` adds an array of strings at +0x218 (the tools read it as the row names), an array of the instrument enum (TypeDefIndex 8326) at +0x228 (read as the row instrument ids) and an int at +0x230 (read as the row count); those readings are the decompile's too. It also holds the instrument at +0x240 and the difficulty at +0x241, the player index at `PlayerSelectionMenu.playerIndex`, and bools at +0x1F0, +0x242, +0x244 and +0x246.
Label: Proven in the game's code
Evidence: `dump.cs` lines 44726 and 44742 to 44745 (BaseMenu and its four ints, the one at 0x44 not read here), lines 72885, 72887 and 72888 (the two arrays and the count), lines 72890 and 72891 (instrument and difficulty), line 72828 (playerIndex) and lines 72875, 72892, 72894 and 72896 (the bools). The dump gives only the layout; what each field means beyond its type is IN-60. This is dump text, not a listing.
Game code (6142): `#824+0x3C`, `#824+0x40`, `#824+0x48`, `#1429+0x218`, `#1429+0x228`, `#1429+0x230`, `#1429+0x240`, `#1429+0x241`, `PlayerSelectionMenu.playerIndex`, `#1429+0x1F0`, `#1429+0x242`, `#1429+0x244`, `#1429+0x246`; fields only, no method.
Hydra: no owner: Hydra never reads the running game. The tools read these through tools/ch_mcp/menu_fields.json; which row is highlighted is worked out in `highlighted_row_text` in tools/ch_mcp/frida_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07 (task LS-K3-menu-fields), checked in the dump.

### IN-59 The player screen's instrument and difficulty start as the profile's saved copy and change on a confirm

Rule: When the screen opens, `PlayerSelectionMenu` copies the profile's instrument into +0x240 and its difficulty into +0x241. Confirming a row on the Instrument or Difficulty page sets the field from the row's text and writes it back into the profile at once, so the screen and the profile stay equal. No cursor move writes either field in the methods read. Two more methods also write +0x241 and were not read.
Label: Proven in the game's code
Evidence: `code/6142/player_menu_on_enable_32BF80.asm`: OnEnable, which Unity runs each time the screen opens, copies the profile's byte +0x10 into +0x240 (lines 191 to 198) and +0x11 into +0x241 (lines 199 to 206), after it replaces an instrument of 0xF ("No Part") in the profile (lines 183 to 190). `code/6142/player_menu_confirm_32E190.asm`: on the Difficulty page the confirm sets +0x241 to 3, 2, 1 or 0 for "Expert", "Hard", "Medium" or "Easy" (lines 79, 103, 111 and 119) and copies it into the profile (lines 80 to 87); on the Instrument page it calls `0x32FB60` (line 135), which sets +0x240 from the highlighted row's name and copies it into the profile (`code/6142/player_menu_set_instrument_32FB60.asm` lines 34 to 183 and 184 to 191). A scan of all 51 named methods of `PlayerSelectionMenu` and its nested classes for byte stores to +0x240 and +0x241 (`docs/audit/2026-11-08-clone-hero-unknowns/W9a/player_menu_stores_240_241.txt`) found 27 stores in five methods. The two not read are `0x32C9C0` (writes 5, 8, 0 and 1) and `0x32D000` (writes 3, 2, 1 and 0); no direct caller of either was found (byte scan of il2cpp and .text, `W9a/callers_240_241_writers.txt`). Live run LS-K3 run 2 (2026-10-31) read Guitar and Expert in these two fields.
Game code (6142): `#1429+0x240`, `#1429+0x241`; OnEnable RVA `0x32BF80`, confirm RVA `0x32E190`, instrument setter RVA `0x32FB60`.
Hydra: no owner: Hydra never reads the running game.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07, from a decompile of build 6142; the listings, 2026-11-08, session 428cf079 (unknowns audit).

### IN-60 How the player screen moves

Rule: Every menu's update hands each player's presses to one button method, which calls one virtual slot per press. Blue moves the highlight down and Yellow moves it up, but only when the profile's drums test passes: the controller type (IN-57) is Drums or FiveLaneDrums and one global setting is 1, or the player has no profile. The moves change the cursor at +0x40, or the scroll at +0x3C at the scrolling edge, and do nothing on the Ready page. Green does not always go to the next page; it goes where the row or a flag says. On Confirm, the cursor alone picks the row: row 1 opens Instrument, row 2 Difficulty, row 3 Modifiers, and row 0 goes to Ready unless +0x244 is set. Row 0 saves the part first, except when +0x245 is clear and `CHNetManager` method `0x145FE0` returns true. On Instrument, the row at cursor + scroll sets the instrument from the row's name string at +0x218 (not from the id array at +0x228), then opens Difficulty when +0x1F0 is set, else returns to Confirm. On Difficulty, the cursor alone picks the row, which sets the difficulty by name and goes to Modifiers when +0x1F1 is set, else to Confirm; the row "Change Instrument" sets +0x1F0 and opens Instrument. On Modifiers, row 0 returns to Confirm and any other row toggles one modifier. On Ready, Green does nothing. The bool at +0x242 is "the controller type is Drums or FiveLaneDrums". +0x1F0 means "came here to change the instrument". +0x246 stops Red from leaving the Instrument page, and +0x244 stops Red from leaving the screen from Confirm; whether +0x244 is a network flag is not shown. The code writes the page field (IN-56) as 5 and 6, which the enum does not name; 7 is only compared, and no write of 7 was found in the class's own methods.
Label: Proven in the game's code
Evidence: the route of a press: `code/6142/menu_update_1D7060.asm` lines 76 to 153 and `code/6142/menu_player_input_1D6B10.asm` line 16 hand each player to the button method; `code/6142/menu_buttons_1D4BC0.asm` calls slot 31 for Green down (lines 86 to 93 and 318 to 321), slot 51 for Red down (lines 120 to 127 and 298 to 301), and slot 21 for Yellow down and slot 10 for Blue down only when the drums test passes (lines 128 to 135, 160 to 167 and 268 to 296), with no profile counting as a pass (line 80); otherwise Yellow and Blue call slots 20 and 36, which the player screen does not override and BaseMenu leaves empty (`dump.cs` lines 44886 and 44979). The drums test is `code/6142/menu_drums_test_D7F20.asm` lines 9 to 23; which global setting it reads was not found. Two more actions, ids 5 and 14 (names not read), also call slots 10 and 21 for every controller type when Green is not held (`menu_buttons_1D4BC0.asm` lines 322 to 398). The moves: `code/6142/player_screen_down_32E710.asm` and `code/6142/player_screen_up_330F30.asm` lines 9 to 13 (nothing on Ready, else the base move), `code/6142/menu_cursor_down_1D57F0.asm` lines 34 and 36 and `code/6142/menu_cursor_up_1D6820.asm` lines 21 and 35. Green per page: `code/6142/player_screen_green_32E190.asm` lines 9 and 10 (Ready), 24 to 52 (Modifiers), 53 to 132 (Difficulty) and 137 to 211 (Confirm; lines 171 to 181 skip the save when +0x245 is clear and `0x145FE0` returns true, and line 211 calls `0x32FF70`). In `code/6142/player_screen_confirm_ready_32FF70.asm`, lines 22 and 23 compare +0x244 with 0 (at 0x32FFD1) and jump past the write of 4, Ready, at line 25 (0x32FFE1) when it is set; `code/6142/player_screen_instrument_32FB60.asm` lines 34 to 228 (Instrument), with +0x242 at lines 192 to 206. Red: `code/6142/player_screen_red_32CBE0.asm` lines 44 and 45 (+0x246) and line 60 (+0x244). Page 6 is written by `code/6142/player_screen_page6_32D2D0.asm` line 5; 5 and 6 are passed to the second page setter `0x330350`, found by a search of every method of the class for page writes (`docs/audit/2026-11-08-clone-hero-unknowns/W9b/psm_page_values.sh`). The action ids are from the kit log `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-31\ls-d1a-run3\kit-2.jsonl`. A `CHNetManager` bool `0x145FE0` (not read) gates Red on Ready and Confirm and the save on Confirm row 0. A read-only scout's decompile (session 5cc1cc07, 2026-10-31) first reported this screen's moves; this block rests on the listings, which correct it in three places.
Game code (6142): `#1429+0x120`, `#1429+0x1E8`, `#824+0x40`, `#824+0x3C`, `#1429+0x228`, `#1429+0x1F0`, `#1429+0x242`, `#1429+0x244`, `#1429+0x246`; button method RVA `0x1D4BC0`, Green RVA `0x32E190`, instrument setter RVA `0x32FB60`.
Hydra: no owner: Hydra never reads the running game. The tools print these fields with `menu_lines` in tools/ch_mcp/frida_tools.py. The tools' `highlighted_row_text` there follows this block: the cursor alone on Confirm and Difficulty, cursor + scroll on Instrument and Modifiers, and no row on Ready. It also prints the id from +0x228, which none of these listings reads.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 5cc1cc07, from a decompile of build 6142; the listings, 2026-11-08, session 428cf079 (unknowns audit).

### IN-91 The player screen's method at 0x32F590 is a Ready-page test that the canvas calls once a frame per active screen

Rule: The method of `PlayerSelectionMenu` at RVA 0x32F590 tests whether the player screen is on its Ready page. `PlayerSelectionCanvas.Update` calls it once a frame on each active player screen, until every active screen is ready or the canvas's bool at +0x48 is set. So with one active screen it runs once a frame, and with two, twice. Five other methods call it too, and they were not read. The tools watch it as the screen's per-frame method, which holds for one player.
Label: Proven in the game's code
Evidence: `code/6142/player_menu_on_ready_page_32F590.asm` lines 1 to 3: it compares the page field +0x120 with 4 (Ready, IN-56) and returns the result. `dump.cs` lines 73000 and 73001 give it as a public instance method returning bool (class header at line 72822). Its direct callers (byte scan of il2cpp and .text, `docs/audit/2026-11-08-clone-hero-unknowns/W9a/callers_32F590.txt`) are `PlayerSelectionCanvas.Update` at 0xD0EFF, three unnamed `PlayerSelectionCanvas` methods and two `PlayerProfileMenu` methods. `code/6142/player_canvas_update_D0E20.asm` shows the per-frame call: while the canvas's +0x48 is false and its component is enabled (lines 13 and 18), it calls the test once on every player screen whose GameObject is active (lines 25 to 58 and 97 to 99), and when every active screen is ready it fades to Gameplay and switches itself off (lines 100 to 105 and 130 to 162). The canvas class and its fields are `dump.cs` lines 25710 to 25722. The tools' comment at tools/ch_mcp/agent/menu.ts lines 49 to 54 calls it the screen's per-frame method.
Game code (6142): `PlayerSelectionMenu` method RVA `0x32F590` (`dump.cs` lines 73000 and 73001); `PlayerSelectionCanvas.Update` RVA `0xD0E20`.
Hydra: no owner: Hydra never reads the running game. The tools use it as `PLAYER_SCREEN_FRAME_RVA` in tools/ch_mcp/agent/menu.ts line 54.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 77057df2 (docs-gaps search), from tool code; the listings, 2026-11-08, session 428cf079 (unknowns audit).

## The bot

### IN-31 The bot never gains Star Power

Rule: A bot never gains Star Power meter: the engine's gain method returns before adding anything when the player kind is Bot (or a remote player). On a chart with 11 phrases the bot hit every note and earned 0 phrases, with 0 activations and 0 time in Star Power, so a bot run can never test a Star Power rule. The kind is Bot because the drums player's kind method gives Bot to a slot whose profile has "is_bot" on (IN-64), and the command-line loader switches it on when no replay is given (IN-61).
Label: Proven in the game's code
Evidence: `code/6142/sp_gain_20F4FA0.asm` lines 13 to 16 (the player kind at +0x190 compared with 2 and with 1, each jumping to the return at line 58). The kind's source is `code/6142/player_kind_11676A0.asm` lines 110 to 132 and `code/6142/cli_loader_3968D0.asm` lines 324 to 329. Live, 2026-10-28: `RUNS\l1\bot\scorestats-Trippin_SP_Spike-20261028-210052.json` (Drums) and `...-212754.json` (ProDrums): 11 phrases offered, 0 earned, 0 activations, 0 time in Star Power, 1,257 notes hit, is_bot true. The same chart played by keys earned 11 phrases and activated 4 times (`overlay-spike/run1_cal0_scorestats.json`). The listing shows only this route; no other method that adds meter was searched.
Game code (6142): #8288 gain method, RVA `0x20F4FA0`; field `#8288+0x190`.
Hydra: no owner: Hydra has no bot model (no "is_bot" in src/).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, session 6a549de0 (feasibility study line 129); code read 2026-10-30, session b02ab3e6 (the reference build); the kind's source read 2026-10-31, session b15bb484 (task D1).

### IN-32 The engine's player kind is set once by its constructor

Rule: The engine's player kind is a 4-byte enum at +0x190 (Player 0, RemotePlayer 1, Bot 2, Replay 3), written once in the base engine's constructor from the +0x10 of its `ref` record parameter, a set-up record. For drums, the value Bot is written into that record by the drums player's kind method (IN-64): the `DrumsPlayer` method that calls it copies the record into a struct, passes the struct to the drums engine's constructor, and that constructor passes the record on to the base constructor through the engine-base constructor.
Label: Proven in the game's code
Evidence: enum #8553 at `dump.cs` lines 546294 to 546301; the field at `dump.cs` line 530029; the store at 0x20F7BCB, inside `0x20F79E0`, is line 93 of `code/6142/engine_ctor_20F79E0.asm` (page 06's listing), checked for this page; line 9 there keeps the record parameter (`r8`) in `r14`, and lines 83 to 94 read it at +8, +0xC, +0xD, +0xE and +0x10. The base constructor's signature is `dump.cs` line 530192 (a `ref` record as its second parameter). The kind method `code/6142/player_kind_11676A0.asm` fills a record with those fields. The path from there to the constructor is in `code/6142/player_kind_11676A0.md`, "Where the record goes": the caller `0x33C560` calls the kind method at 0x33C8D5, copies the record to `rsp + 0x48` (0x33C8DA to 0x33C8F8), passes `rsp + 0x20` in `r8` to the drums engine's constructor `0x20DF680` (0x33C933 and 0x33C9BA), which keeps `r8` in `rdi` and passes `rdi + 0x28` (the record) as the fifth argument of the engine-base constructor `0x100EAE0` (lines 8, 17, 28 and 44 of `code/6142/engine_ctor_20DF680.asm`; IN-23 names `0x100EAE0` the drum engine's base constructor), which loads it into `r8` at 0x100EAFD and calls `0x20F79E0` at 0x100EB63. The caller's lines and `0x100EAE0` were read with tools/ch_mcp/chdis.py on 2026-10-31 and have no listing of their own.
Game code (6142): `#8288+0x190`; base engine constructor RVA `0x20F79E0`; kind method RVA `0x11676A0`; its caller RVA `0x33C560`; drums engine constructor RVA `0x20DF680`; engine-base constructor RVA `0x100EAE0`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build); the kind method and the path to the constructor, 2026-10-31, session b15bb484 (task D1).

### IN-33 A remote player's engine adds no score and drains no Star Power

Rule: When the player kind is RemotePlayer, the engine's two score adds return at once and its per-update Star Power method skips the drain step (it still runs the clamp after it); Player, Bot and Replay engines run all of them.
Label: Proven in the game's code
Evidence: `code/6142/add_note_score_20F6EB0.asm` lines 13 to 14, `code/6142/add_points_20F5F70.asm` line 1 and `code/6142/sp_update_20F6800.asm` lines 37 to 38 (each compares +0x190 with 1; the first two jump to their return, the third jumps past the drain to the clamp, listing line 69); the enum at `dump.cs` line 546294. Two more sites test the same kind: the combo break skips its streak reset for a remote player (`docs/audit/drum-lanes/play/f20f4b60_combo_break.asm` line 24, IN-87), and the hit routine skips a call for one (`docs/audit/drum-lanes/play/f20deaa0.asm` line 115). The method at 0x20F3610, which its listing's file name calls the base miss, tests it too: after its slot-20 call (line 31, `call qword ptr [rax + 0x278]`), `cmp dword ptr [rbx + 0x190], 1` at `docs/audit/drum-lanes/play/f20f3610_base_miss.asm` line 32 jumps past lines 34 to 53 to line 54, so a remote player's miss skips the two calls there, to 0x20F62C0 (line 43) and `0x20F7610` (line 53). What those two calls do was not read.
Game code (6142): RVA `0x20F6EB0`, RVA `0x20F5F70`, RVA `0x20F6800`; the base miss method at 0x20F3610 (`docs/audit/drum-lanes/play/f20f3610_base_miss.asm` lines 32 to 54); field `#8288+0x190`.
Hydra: no owner: Hydra has no network player.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build); the base miss site, 2026-11-08, session 77057df2 (docs-gaps search).

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

### IN-38 A stats writer names the "is_bot" key in code

Rule: The "is_bot" key is written by code, not by a serializer reading a field name. A JSON stats writer passes the "is_bot" string to the JSON writer's bool call, with the bool from its record's +0xB6. The profile constructor uses the same string to name its "is_bot" setting (IN-65).
Label: Proven in the game's code
Evidence: read with tools/ch_mcp/chdis.py on 2026-10-31 in `0x1144C0` (`dump.cs` line 30475, a method of class #561 taking a `Utf8JsonWriter`); its listing is `code/6142/score_submit.asm` (lines 101 and 407 to 410). At 0x114703 the method's one-time set-up loads the "is_bot" string slot (`lea rcx, [rip + 0x3ba677e]`, the slot at 0x3CBAE88). At 0x114CC9 it loads the same slot as the property name (`mov rdx, qword ptr [rip + 0x3ba61b8]`), after reading the value byte at 0x114CC1 (`movzx r8d, byte ptr [r14 + 0xb6]`), and calls the JSON writer's WriteBoolean at 0x114CD3; the keys around it are "is_fc", "versus_winner" and "is_pfc". The profile constructor loads the same slot at 0x2123CE3. The writer's text goes into `scorestats.json` (IN-72).
Game code (6142): stats writer RVA `0x1144C0`; profile constructor RVA `0x2123890`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build); corrected 2026-10-31, session b15bb484 (task D1), from the disassembly.

### IN-39 Why scores are switched off is kept in GlobalVariables

Rule: `GlobalVariables.scoreDisabled` (+0x7C) is an enum (#727) with the values None 0, Modifier 1, Bot 2, Pause 3. Two writers of Bot were found: the GameManager load coroutine and `EndOfSong.Start`. Each writes Bot only when the `GlobalVariables` method `0x235A40` returns true, which needs some player in `playerList` to pass both `0x337540` (it has a Rewired player, or its remote byte is set) and the "is_bot" test `0x13F8B0`. The game also writes a fifth value, 4, which the enum does not name: the GameManager pause method writes it when scores are not already off, practice is not on, totalPauses (+0x120) is above 3 and the two GameManager times at +0x70 and +0x118 are less than 1185.0 apart.
Label: Proven in the game's code
Evidence: the enum and field are `dump.cs` lines 39726 to 39733 and 55827. The test: `code/6142/globals_any_bot_235A40.asm` lines 19 to 32 (the two tests `0x337540` and `0x13F8B0` were read with tools/ch_mcp/chdis.py and not kept; `code/6142/globals_any_bot_235A40.md` describes them). The first writer: `code/6142/gm_load_coroutine_D8D80.asm` line 547 (the call), line 549 (the false branch) and line 560 (the store of 2). The second: `code/6142/end_of_song_start_15DC20.asm` line 729 (the call) and line 742 (the store of 2). The value 4: `code/6142/gm_score_disabled_4_B5F90.asm` lines 126 to 196. A field-write scan for +0x7C over every named method that loads GlobalVariables' type info (`docs/audit/2026-11-08-clone-hero-unknowns/W9a/globals_writes_28_7C.txt`) found seven stores. The other four were not read: 0 and 1 in `BasePlayer` `0x209C70`, 3 in `GameManager` `0xB9680`, and a register value in `EndOfSong` `0x161C70`, which seems to write 0 when a bot is present (not shown). In 1.0 the writes are in the 4045 decompile's GameManager.cs lines 465 and 2252 and EndOfSong.cs line 386, not copied.
Game code (6142): `GlobalVariables.scoreDisabled`; test RVA `0x235A40`; load coroutine RVA `0xD8D80`; `EndOfSong.Start` RVA `0x15DC20`; pause method RVA `0xB5F90`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build); the writers, 2026-11-08, session 428cf079 (unknowns audit).

### IN-40 The 1.0 bot earns no Star Power

Rule: In 1.0 the gain method skips a remote player and the bot. Its test method returns the profile's `isBot` setting, and a command-line start sets that setting to 1, so the 1.0 bot never earns Star Power. In network play, BasePlayer sets `isBot` to 0.
Label: Proven in the game's code
Evidence: `code/4045/bot_test.cs`, explained in `code/4045/bot_test.md`, made with ilspycmd from the 1.0 `Assembly-CSharp.dll`. Section B1 is the whole test method: it returns the profile's `isBot`. Section B2 is the gain method's opening, BasePlayer.cs line 755: its whole body is inside `if (!isRemotePlayer && !IsBot())`. Section B3 is CliGameLauncher.cs line 684, which sets `isBot.CurrentValue = 1` on the profile it makes. Section B4 lists every write to the meter field in BasePlayer.cs: the only add is line 759, inside B2's test, and all three callers of the gain method go through it. The network-play write is BasePlayer.cs line 1108 of the same decompile, not copied into the listing. The test method's other call sites are BasePlayer.cs lines 636 and 1104 and GameManager.cs line 355. `isBot` is declared in `BasePlayerProfile`, outside Assembly-CSharp, which was not decompiled.
Game code (4045): the BasePlayer gain method, `code/4045/fill_deadline.cs` lines 34 to 57 (line 36 is the skip); the test, `code/4045/bot_test.cs` section B1.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-30, session b02ab3e6 (the reference build); the test method, 2026-11-08, session 428cf079 (unknowns audit).

## Input and logs

### IN-41 Injected keys reach the game

Rule: Keys sent with SendInput reach Clone Hero for all eight drum keys, and a pad flash lasts about one video frame at 60 fps.
Label: Unknown
Evidence: one part is proven in a live run. In `overlay-spike/run1_cal0_play_chart.txt` lines 83 to 87, a key-press run on a chart of 857 notes reads "Notes: 857, Hits: 857, Misses: 0" and a best streak of 857 (old output: play_chart's summary no longer prints those counts), so the injected keys reached the game for every note of that chart. That file does not say the keys were sent with SendInput, and it does not show which of the eight keys the chart used. The rest stays unchecked: the Notepad test and the 60 fps measurement are in session 8157f2dc's transcript only, and the video was not opened. The eight keys themselves are `DEFAULT_BINDINGS` at tools/ch_probe/input_driver.py line 69, whose comment says they were read off the game's Controller Remap screen on 2026-09-25; no kept file shows that screen, so the key map is Unknown too.
Game code (6142): none (input arrives through the game's Rewired input system).
Hydra: no owner. The tool's key map is `DEFAULT_BINDINGS` (tools/ch_probe/input_driver.py line 69, used by tools/ch_probe/experiments/key_delivery_test.py and the ch_mcp tests).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc; the 857-of-857 run file and the key map's source, 2026-11-08, session 77057df2 (docs-gaps search).

### IN-42 Whether the kick key scores a 2x kick gem

Rule: The tools press the kick key for both kicks and assume it scores a 2x kick gem.
Label: Unknown
Evidence: `overlay-spike/run1_cal0_scorestats.json` shows the DoubleKick modifier and 1,257 of 1,257 notes hit. That run cannot show it, because its chart holds no 2x kick gem: Trippin SP Spike's [ExpertDrums] has 402 notes N 0 (kick) and no N 32 (2x kick), and Hydra's replay output for it counts 402 Kick. A 2x kick would also be counted as Kick there, so the chart itself is the proof. The chart is C:\Clone Hero\songs\Hydra Probe\Trippin SP Spike\notes.chart, outside the repo: its [ExpertDrums] section holds 402 notes of N 0 and none of N 32. The replay output is C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\l1\trippin.json, outside the repo: its 857 chords hold 402 notes of colour Kick. Both counts were read again on 2026-11-11 (session 428cf079). The checker's verdict is the IN-42 line of `docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md` (line 108). A check needs a run on a chart with 2x kick gems.
Game code (6142): not found.
Hydra: the 2x kick read is page 02's.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, session 8157f2dc; the chart check, 2026-11-11, session 428cf079 (unknowns audit).

### IN-43 One run's session log held only its loading line while the song played

Rule: In one key-press run, the session log got only the "Loading Song" line and one warning in 300 s while the song played, and no "OnCountdownStart" line appeared in that time; so waiting for "OnCountdownStart" does not work, and the tools wait ATTACH_DELAY_S = 5.0 s instead (tools/ch_mcp/frida_tools.py line 43). That a command-line start is the cause was not checked, because no kept file records that run's launch arguments.
Label: Proven in a live run
Evidence: `tools/ch_probe/experiments/results/overlay-spike/run1_cal0_log_tail.txt` (the 2026-10-29 key-press run of `overlay-spike/run1_cal0_play_chart.txt`) watched `Session-2026-10-28_20-34-47.log` from its end for 300 s (lines 2 and 3). It got two lines, both at 20:36:15.244: "[Log] Loading Song: Hydra Probe - Trippin SP Spike" and a missing-text warning (lines 4 and 5), then stopped at the 300 s cap (line 6). The song played meanwhile: `overlay-spike/run1_cal0_sp_watch.csv` reads the clock rising from 2.207 s to 181.160 s. The two Unity `Player.log` files kept from runs (`hydra-data\ch-mcp-runs\2026-11-07\ls-t2-run-a\Player.log` and `hydra-data\ch-mcp-runs\2026-11-07\ls-t3-run-b\Player.log`) show "Loading Song" and no line holding "countdown" over a whole run; they have no time stamps, so they cannot show what came first. The tools' wait is `ATTACH_DELAY_S`, 5.0 s, at tools/ch_mcp/frida_tools.py line 43.
Game code (6142): not found.
Hydra: no owner. The tool's delay is `ATTACH_DELAY_S` (tools/ch_mcp/frida_tools.py).
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study line 139; the kept log tail, 2026-11-08, session 428cf079 (unknowns audit).

### IN-44 The hit-detection logs are made at start-up and keep only tagged entries

Rule: The game sets up two file sinks for `HitDetection-Standard.log` and `HitDetection-Validation.log` at start-up; where the file is opened is not shown. Each sink writes only log entries tagged "GuitarEngine" or "GuitarEngineValidator". No setting is read on the way. Whether the shipped build ever makes a tagged entry is not known: no direct caller of the log writers was found by a byte scan of il2cpp and .text, which cannot see calls from generic method instances.
Label: Proven in the game's code
Evidence: GameInit.PreUnityInit calls `0x213040` with no test in front of it (the call at 0x1646F0). `code/6142/fast_logger_setup_213040.asm` checks only whether it already ran (lines 55 to 63), makes the `Logs` folder (lines 196 to 218) and adds three log targets at level 4, Trace (lines 239, 259 and 279); two are file sinks, one for `HitDetection-Standard.log` tagged "GuitarEngine" (line 248) and one for `HitDetection-Validation.log` tagged "GuitarEngineValidator" (line 268). The sink's write method writes an entry only when the entry's tag string at +0x40 equals the sink's tag, compared ordinally (`code/6142/log_file_sink_write_2158DF0.asm` lines 27 to 42); the tag is the fifth argument of the logger's writer (`code/6142/log_writer_2128F10.asm` lines 93 and 94). Unity's Debug.Log does not reach these files: the setup's lines 166 to 184 set Unity's logger to class #981, which hands lines to Unity's own log. No direct caller found (byte scan of il2cpp and .text) for the writers `0x2128F10` and `0x2128B70` or the dispatch method `0x2128770`. A byte scan cannot see calls from generic method instances, and the generic Log instances have compiled bodies (for example `0x604210`), so this does not show that no entry is ever made. The empty files are reported in the feasibility study (lines 33 and 109) and docs/handoffs/2026-10-28-ch-mcp-results-handoff.md line 39 (removed; `git show 444631e6:docs/handoffs/2026-10-28-ch-mcp-results-handoff.md`); no run has tested whether play fills them.
Game code (6142): logger setup RVA `0x213040`; file sink write RVA `0x2158DF0`; logger writer RVA `0x2128F10`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 33 and 109; the listings, 2026-11-08, session 77057df2 (unknowns audit).

### IN-45 The game's text strings are plain

Rule: The game has 23,972 text strings, all distinct, which Il2CppDumper reads as plain text. The metadata holds 1,346 distinct `.cs` source paths, of which 776 start with `\Assets`. The literal "combo: {0}" plus a newline is one line of a player's results summary: the static `BasePlayer<T>` method at 0x1168290, one of four compiled copies, builds such lines with 34 String.Format calls and writes them to Unity's log. That 0x1168290 is the copy for drum inputs is not shown.
Label: Proven in the game's code
Evidence: the counts are from `docs/audit/2026-11-08-clone-hero-unknowns/W8c/count_strings_paths.py`, whose output is `W8c/count_strings_paths.out.txt` beside it; it reads `stringliteral.json`, `dump.cs` and the game's `global-metadata.dat`. That the strings are not encrypted rests on Il2CppDumper reading them as plain text; it was not checked another way. The literal is at `stringliteral.json` lines 42695 and 42696. The method: `code/6142/base_player_combo_text_1168290.asm` lines 23 to 94 load the formats (line 43 the combo one), lines 173 to 181 format the combo line and line 591 logs. `dump.cs` lines 53245 to 53258 list the four copies. All four use the string, and so do `PracticeSectionDisplay` `0x2E98A0` and method `0x3B0910`; no direct caller of 0x1168290 was found (byte scan of il2cpp and .text, `W8c/xref_IN-45.out.txt`). Its type argument is struct #8663, whose second field is eight sbytes (struct #8688, `dump.cs` lines 553483 to 553494); no name in `dump.cs` says drums.
Game code (6142): `BasePlayer<T>` method copy RVA `0x1168290`.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 56 and 158; the counts and the method, 2026-11-11, session 428cf079 (unknowns audit).

### IN-92 Where the game keeps its key bindings and settings

Rule: Rewired keeps the game's bindings in Unity's PlayerPrefs, which a Windows player keeps under the registry key `HKCU\Software\srylain Inc.\Clone Hero`, with a dot in "Inc." unlike the LocalLow folder. The game's own code writes PlayerPrefs too, so the key holds more than the bindings. `poll_rate` in `settings.ini` is Rewired's joystick refresh rate: a whole-number setting whose default is four times the screen's refresh rate, with min 60 and max 1000 passed to the setting's constructor 0x210C470 (not read); the schema file gives the same numbers. The game builds the paths of `settings.ini` and `settings_schema.json` at start-up; this machine's `settings_schema.json` gives `poll_rate` the default 656, minimum 60 and maximum 1000, and who writes that file was not read.
Label: Proven in the game's code
Evidence: the bindings: Rewired's `UserDataStore_PlayerPrefs` (`dump.cs` line 4278) is the only caller of `PlayerPrefs.SetString` and `PlayerPrefs.Save` inside Rewired's code (direct callers only, byte scan of il2cpp and .text, `docs/audit/2026-11-08-clone-hero-unknowns/W10a/xref_playerprefs.txt`), and the same scan finds callers in `MainMenu`, `News`, `BuildCheck`, `CliGameLauncher` and one scrambled class. Unity's "PlayerPrefs" Scripting API page (https://docs.unity3d.com/ScriptReference/PlayerPrefs.html) gives the registry path from the company and product names, which the game's `Clone Hero_Data\app.info` gives as "srylain Inc." and "Clone Hero"; this machine's key holds Rewired's controller maps. `poll_rate`: `code/6142/settings_load_374D50.asm` lines 1637 to 1671 read Unity's Screen.currentResolution (line 1639), divide its refresh-rate ratio (lines 1644 and 1658 to 1664), truncate and multiply by 4 (lines 1665 and 1666), with minimum 60 (line 1660) and maximum 1000 (line 1657), and store the setting at the settings statics' +0xC0 (line 1671). `code/6142/poll_rate_to_rewired_233198.asm` lines 1 to 12: `GlobalVariables.Start` hands the value to Rewired's `ReInput.ConfigHelper.set_joystickRefreshRate`; the unit is Rewired's, from documentation that was not fetched. Two more readers of `poll_rate` in `GameplaySettingsMenu` were not read. The paths: `code/6142/settings_ini_path_377FC0.asm` lines 12 to 28 and `code/6142/settings_schema_path_164940.asm` lines 4 to 38. The tools' key is `REGISTRY_KEY` at tools/ch_mcp/game_files.py line 70.
Game code (6142): settings loader RVA `0x374D50`; settings path RVA `0x377FC0`; the schema path, built inside the `GameInit` method RVA `0x164810`; the call in `GlobalVariables.Start`, `code/6142/poll_rate_to_rewired_233198.asm` lines 1 to 12.
Hydra: no owner: Hydra never reads the running game. The tools back the registry key up and restore it in tools/ch_mcp/game_files.py, so the non-Rewired values in it are backed up too.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 77057df2 (docs-gaps search), from tool code and the feasibility study; the game's code, 2026-11-08, session 428cf079 (unknowns audit).

## Naming across the two builds

### IN-46 Four assemblies are scrambled, by Beebyte Obfuscator 3.13.0

Rule: Only four of the game's assemblies have scrambled names: CloneHero.dll (985 of 1,451 types), StrikeCore.dll (427 of 449), CHNet.Core.dll (167 of 176) and CHNet.Server.dll (15 of 22). Each holds the marker class `__BB_OBFUSCATOR_VERSION_3_13_0`, so the scrambler is Beebyte Obfuscator 3.13.0.
Label: Proven in the game's code
Evidence: the type counts per assembly are the table in `docs/research/2026-10-28-clone-hero-mcp-feasibility.md` lines 45 to 50, a count of the type names with non-ASCII letters per image in `dump.cs` (image list in lines 1 to 100). The marker class is at `dump.cs` lines 74148, 553588, 813701 and 879625 (TypeDefIndex 1450, 8690, 13587 and 15020), checked for this page.
Game code (6142): class metadata only.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 41 to 50; recounted 2026-10-30, session b02ab3e6 (the reference build).

### IN-47 The scrambled method counts per assembly

Rule: Counting every method line in the dump, the four scrambled assemblies have 9,814 of 14,885 methods scrambled (CloneHero.dll), 3,700 of 4,209 (StrikeCore.dll), 1,701 of 1,906 (CHNet.Core.dll) and 304 of 320 (CHNet.Server.dll). Counting only methods with code of their own, the figures are 9,618 of 14,601, 3,019 of 3,338, 1,560 of 1,730 and 304 of 320. The feasibility study's counts (7,941 of 11,926, 3,095 of 3,577, 1,426 of 1,614 and 288 of 303) are not reproduced by either count.
Label: Proven in the game's code
Evidence: `docs/audit/2026-11-08-clone-hero-unknowns/W10b/count_methods.py` counted `dump.cs`, with its output in `W10b/method_counts.txt` beside it. It puts each type in an image by the image list at the top of `dump.cs`, counts each method line of a type's methods block, and calls a method scrambled when its name holds a non-ASCII letter, the same test IN-46 used for types. The second count keeps only methods whose RVA line is not -1. mscorlib.dll, counted as a control, has 0 scrambled. The study (docs/research/2026-10-28-clone-hero-mcp-feasibility.md line 52) says its counts came from a regex and are "close, not exact"; that regex is not kept, so the gap cannot be traced.
Game code (6142): method metadata, counted.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 47 to 50; recounted 2026-11-08, session 428cf079 (unknowns audit).

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

Rule: 1.1's drum engine name (class #8255) appears nowhere in 1.0's dump of its five assemblies. Of 1.1's 60 GameManager fields and 1.0's 63, exactly 13 field names appear in both, and all 13 are scrambled names; none of 1.1's readable field names is among them. The script compares names only, not types or offsets, so a shared name does not by itself show the same field (IN-48 counts the classes that keep their names).
Label: Proven in the game's code
Evidence: `docs/audit/2026-11-08-clone-hero-unknowns/W11b/in50_compare.py`, with its output in `W11b/in50_compare_output.txt` beside it. It reads the 6142 `facts.json` (the drum engine's name, the class at `dump.cs` line 527421), the field lines of 6142's `GameManager` (`dump.cs` line 25284 up to its methods), and the 4045 `facts.json`: every class, field and method name of its 1,020 classes, its one GameManager class and the file's raw text. The engine name is not any of 4045's 10,415 distinct names, and not in that file's text in plain or JSON-escaped spelling. The 4045 `facts.json` covers the five assemblies its `dump_file` names (Assembly-CSharp, Assembly-CSharp-firstpass, StrikeCore, CHNet.Core and CHNet.Server), so "nowhere in 1.0" means nowhere in those five. The compare file's line 5 count of 13 is about classes that appear more than once in a file, not about these fields. The study now holds the claim at docs/research/2026-10-28-clone-hero-mcp-feasibility.md line 139.
Game code (6142): class #8255 and `GameManager` field metadata, counted against 4045's.
Hydra: no owner.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study (line 164 at commit f1b568bd); counted 2026-11-08, session 428cf079 (unknowns audit).

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

Rule: A song started with `--song` and `--player` plays with a bot, because the command-line loader switches the profile's "is_bot" setting on when the launcher holds no replay (IN-61). The `--player` value is read as in IN-63.
Label: Proven in the game's code
Evidence: `code/6142/cli_loader_3968D0.asm` lines 322 to 329 (IN-61), and the parser read for IN-63. Not checked: the flag list from the game's manual (`--song <folder>`, `--player <Instrument>,<Difficulty>`, `--profile`, `-i` and `-v`), because the manual page was not opened and the flag reader was not read; and that the game was still running 30 s after the bot song ended, which is one observation recorded only in the feasibility study (lines 29 and 119). What the tools send is `launch_command` in tools/ch_mcp/bot_run.py: `--song`, `--player`, and `--profile` when a profile is given; the three bot runs behind IN-37 show that launch does play a song to its end.
Game code (6142): the loader RVA `0x3968D0` and the `--player` parser RVA `0x396110`; the flag reader was not read.
Hydra: no owner. The tool's command line is `launch_command` in tools/ch_mcp/bot_run.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-28, feasibility study lines 29 and 119; the loader and the parser read 2026-10-31, session b15bb484 (task D1).

### IN-54 The first auto-player counted a hit when the score rose

Rule: In September 2026 the probe's auto-player (`experiments/play_chart.py`) played the chart "Slipping" and counted a hit each time the score went up; the probe finds the drum engine by scanning memory for the two window constants stored side by side at +0x30 and +0x38.
Label: Proven in a live run
Evidence: `tools/ch_probe/experiments/results/hit_detect.csv` (2026-09-25 run, tracked by commit 283e0288): 60 rows, every `hit` True, the `score` column rising on every row from 1350 to 4850. The file does not name the script that wrote it, and its date is the commit's, not checked against a game log. That the auto-player counts a hit when the score rises is the feasibility study's line 19 (the code of `play_chart.py` was not read for this). The memory scan is the docstring and code of `tools/ch_probe/engine_finder.py`.
Game code (6142): none read; a live run and the tool's own memory scan.
Hydra: no owner. The tool's finder is `find_live_engine` in tools/ch_probe/engine_finder.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-09-25, feasibility study line 19; checked 2026-10-30, session b02ab3e6 (the reference build).

## The command-line loader

### IN-61 A command-line start with no replay switches the profile's is_bot setting on

Rule: When the game starts a song from the command line, its player loader writes the parsed instrument and difficulty into the slot's profile. Then, when the launcher holds no replay (its field at +0x60 is null), it sets the profile's "is_bot" setting to 1. With a replay it skips that write and takes the controller type and modifiers from the replay instead. The 1 is the immediate of one instruction, `mov edx, 1` at 0x396EC5, so the byte that makes the player a bot is 0x396EC6.
Label: Proven in the game's code
Evidence: `code/6142/cli_loader_3968D0.asm` lines 320 and 321 (instrument to profile +0x10, difficulty to +0x11), lines 322 and 323 (the replay field at +0x60 compared with null, jumping to the replay branch at line 358), lines 324 to 329 (the "is_bot" setting at +0x48, `mov edx, 1`, the int setter `0xFE5F50`, which stores the value at the setting's +0x10), and lines 358 to 405 (the replay branch, with no "is_bot" write). The listing holds no bytes, so the bytes were read from the DLL's file at that RVA on 2026-10-31 with tools/ch_mcp/chdis.py: `BA 01 00 00 00`, which is `mov edx, 1` with a 32-bit immediate and puts the `1` at 0x396EC6. How a replay gets into the +0x60 field was not traced. `code/6142/cli_loader_3968D0.md` explains every branch.
Game code (6142): `CliGameLauncher` loader, RVA `0x3968D0` (`dump.cs` line 17992); the replay field `#282+0x60`; the profile fields `#8526+0x10`, `#8526+0x11` and `#8526+0x48`; the setter RVA `0xFE5F50`.
Hydra: no owner: Hydra never reads the running game. The tools start the game this way in `launch_command` in tools/ch_mcp/bot_run.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D1), from the disassembly.

### IN-62 ProDrums takes the loader's drums branch: controller type Drums and modifiers DoubleKick

Rule: In the no-replay branch the loader first sets the profile's modifiers to None (1). Then Drums (6) and ProDrums (9) both take the drums branch, which sets the profile's "controller_type" setting to Drums (2) and the modifiers to DoubleKick (2). The six-fret instruments get controller type SixFretGuitar (1) and every other instrument gets Guitar (0), with the modifiers left at None.
Label: Proven in the game's code
Evidence: `code/6142/cli_loader_3968D0.asm` line 330 (modifiers 1), lines 331 to 334 (compare with 6 and 9, both jumping to line 350), lines 350 to 357 (the setter with 2 on +0x60, then 2 to +0x14), lines 335 to 349 with lines 406 and 407 (the mask `0x4C30` and the setter with 1 or 0). The enums are `dump.cs` lines 533194 and 533197 (Drums 6, ProDrums 9), lines 532200 to 532208 (controller type, Drums 2) and lines 547680 and 547703 (None 1, DoubleKick 2; AllStrums shares the value 2). The bot runs' DoubleKick modifier (IN-42) fits this branch.
Game code (6142): RVA `0x3968D0`; the profile fields `#8526+0x14` and `#8526+0x60`.
Hydra: no owner: Hydra never reads the running game. The tools' `launch_command` in tools/ch_mcp/bot_run.py sends the player it is given, and its default is `DEFAULT_PLAYER`, "Drums,Expert".
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D1), from the disassembly.

### IN-63 The --player value is two comma-split words, matched lower-case

Rule: The `--player` parser splits its text at every comma, lower-cases the first two parts and matches each against a fixed word list. The instrument words and values are guitar 0, bass 1, rhythm 2, guitarcoop 3, guitar6fret 4, bass6fret 5, drums 6, keys 7, prodrums 9, rhythm6fret 10, guitarcoop6fret 11 and keys6fret 14; any other word gives Guitar (0). The difficulty words are easy 0, medium 1, hard 2 and expert 3; any other word gives Expert (3). A value with no comma has no second part, and the parser throws.
Label: Proven in the game's code
Evidence: read with tools/ch_mcp/chdis.py on the 6142 DLL on 2026-10-31, from `0x396110` to the next method at 0x3964F0; no listing of it is committed. The split on `0x2c` (a comma) is at 0x396215, the two lower-case calls at 0x39623F and 0x396260, the string-hash switch from 0x39626D, and each word's string compare is followed by its value, for example `mov bl, 9` at 0x3962CD after "prodrums". Every failed compare lands on `xor bl, bl` at 0x3963D3, and the difficulty's last step at 0x3964A3 to 0x3964AB keeps 3 unless the word is "easy". The array-bounds throw is at 0x3964E0. The method is `dump.cs` line 17940, returning the pair of the instrument enum (#8326) and the difficulty enum (#8617).
Game code (6142): `CliGameLauncher` parser, RVA `0x396110`.
Hydra: no owner: Hydra never reads the running game. The tools' value is `launch_command` in tools/ch_mcp/bot_run.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D1), from the disassembly.

### IN-64 The drums player's kind is RemotePlayer, then Bot, then Replay, else Player

Rule: The method that fills the drums engine's set-up record decides the player kind with three tests on the player's slot, in this order. A slot whose profile marks it remote gets RemotePlayer (1). Otherwise a slot whose profile has "is_bot" other than 0 gets Bot (2). Otherwise a slot whose profile holds a replay gets Replay (3). Everything else gets Player (0). So with "is_bot" at 0 and no replay, a local drums player is Player.
Label: Proven in the game's code
Evidence: `code/6142/player_kind_11676A0.asm` lines 110 to 132 (the remote test, the bot test, the replay test and the three stores to the record's +0x10). The bot test `0x13F8B0` reads the slot's profile, its "is_bot" setting at +0x48, and the setting's bool conversion `0x210BF30`, which is `cmp dword ptr [rbx + 0x10], 0` then `seta al`; `seta` is an unsigned compare, so any int other than 0 reads as true. The replay test `0x13F8E0` is true when the profile's +0xE0 is not null. The remote test `0x337720` returns the profile's byte at +0xB1. The three were read with tools/ch_mcp/chdis.py on 2026-10-31 and have no listing of their own. The enum is `dump.cs` line 546294. `code/6142/player_kind_11676A0.md` explains the lines and says how far the record's hand-off to the engine was followed.
Game code (6142): `#996` method, RVA `0x11676A0`; tests RVA `0x337720`, RVA `0x13F8B0`, RVA `0x13F8E0`; bool conversion RVA `0x210BF30`; the slot field `#901.player`.
Hydra: no owner: Hydra has no bot model.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D1), from the disassembly; the plan for the direct start (docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)) named the method as a lead.

### IN-65 The profile keeps instrument, difficulty, modifiers, is_bot and controller type at fixed places

Rule: The profile's base class holds the instrument as a byte enum at +0x10, the difficulty as a byte enum at +0x11, the modifier flags as an int enum at +0x14, the "is_bot" setting at +0x48 and the "controller_type" setting at +0x60. Both settings are int settings; "is_bot" is built with the default false, and the game reads it as true when its int is anything but 0 (IN-64). A new profile starts as Guitar on Expert with the modifiers None.
Label: Proven in the game's code
Evidence: the fields are `dump.cs` lines 541668 to 541683 (class #8526, the base of the profile class #451 at line 25613), with the enums at lines 533184 (#8326), 550049 (#8617) and 547675 (#8576), and the setting class #8308 at line 532111. The constructor `0x2123890` stores the setting named "controller_type" at +0x60 (0x2123B0F to 0x2123B4B) and the one named "is_bot" at +0x48 (0x2123CE3 to 0x2123D0A, built by the bool constructor at 0x210C430, `dump.cs` line 532178, with `xor r9d, r9d`), and writes the word 0x300 to +0x10 and 1 to +0x14 (0x2123A49 and 0x2123A53). It was read with tools/ch_mcp/chdis.py on 2026-10-31; no listing of it is committed. The bool conversion is IN-64's.
Game code (6142): `#8526+0x10`, `#8526+0x11`, `#8526+0x14`, `#8526+0x48`, `#8526+0x60`; profile constructor RVA `0x2123890`.
Hydra: no owner: Hydra never reads the running game.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D1), from the dump and the disassembly.

### IN-66 The loader gives the slot the Rewired System Player

Rule: The loader stores a Rewired player in the slot's player field. It is the System Player, fetched with GetSystemPlayer, unless a bool of the launcher is set; then it takes an entry of Rewired's player list instead. The drums input reads this field at each button read (IN-68). What sets the bool was not traced.
Label: Proven in the game's code
Evidence: `code/6142/cli_loader_3968D0.asm` lines 195 to 213 (the slot from `GlobalVariables.playerList` and its index written to +0x28), line 215 (the launcher's bool at +0x58), line 244 (the GetSystemPlayer call, `dump.cs` line 310436), lines 217 to 233 (the other route, through a helper at 0x2C421D0 that was not read) and line 246 (the store to the slot's +0x18, whose type is Rewired's `Player`, `dump.cs` line 73341).
Game code (6142): RVA `0x3968D0`; the slot field `#1435+0x18`; the launcher's bool `#282+0x58`.
Hydra: no owner: Hydra never reads the running game. The kit's player is `KIT_PLAYER` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D1), from the disassembly.

### IN-67 The profile screen writes the profile's controller type and resets its modifiers when its string reads Controller

Rule: In the `PlayerProfileMenu` method `0x1E68D0`, when the menu's string at +0x30 equals "Controller" and its int at +0x40 differs from the profile's current controller type, the method writes that int into the profile's "controller_type" setting and sets the profile's modifiers to None (1). That the string is the name of the highlighted row and the int is the cursor was not checked.
Label: Proven in the game's code
Evidence: read with tools/ch_mcp/chdis.py on 2026-10-31 in the `PlayerProfileMenu` method `0x1E68D0` (`dump.cs` line 45883); no listing of it is committed. The compare of the menu's string at `rbx` +0x30 (`BaseMenu`, `dump.cs` line 44739) with the text "Controller" is at 0x1E6C48 to 0x1E6C55; the only jump to 0x1E6C48 is at 0x1E6BB3. Then 0x1E6C6B to 0x1E6C7E compare the setting at profile +0x60 with the int at +0x40 of the menu object in `rbx`, 0x1E6C92 calls the int setter `0xFE5F50` with that int, and 0x1E6C97 writes 1 to profile +0x14. The profile is reached through `rbx` +0x20 and then +0x10 (0x1E69A5 and 0x1E69D2). `rbx` is set from `rcx` at 0x1E68E1 and is reloaded only on another path (0x1E6B7E), so it still holds the menu at 0x1E6C7B. That the `BaseMenu` int at +0x40 (`dump.cs` line 44743) is the cursor rests on IN-58's scout reading, which was not checked.
Game code (6142): `PlayerProfileMenu` method RVA `0x1E68D0`; the profile fields `#8526+0x14` and `#8526+0x60`.
Hydra: no owner: Hydra never reads the running game.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D1), from the disassembly.

### IN-68 The drums input reads the slot's Rewired player again at each button read

Rule: The drums player's input method asks the Rewired player held in its slot (the field the loader fills, IN-66) which buttons are down. It loads the slot and that Rewired player afresh before nearly every button read, so it uses whatever player the slot holds at that moment; it keeps no copy from when the player was built. It reads nothing when the slot's profile is marked remote.
Label: Proven in the game's code
Evidence: `code/6142/drums_input_source.asm` lines 19 to 26 (the slot from +0x88, its profile from +0x10, the remote byte +0xB1 jumping to the return at line 204), lines 27 to 32 (the slot's +0x18 and the call to Rewired's GetButtonDown with action 4), lines 33 to 37 (the slot and +0x18 loaded again for the next read), and the same pattern to line 292: 17 calls into Rewired's `Player` (`dump.cs` lines 306991 and 306997) and 16 loads of the slot. `code/6142/drums_input_source.md` explains the lines. Not checked: how often the method runs, because it is an override with no direct caller. Rewired does read a controller added to the System Player: in LS-D1d of `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)`, 2026-11-06, a kit kick on System scored a hit (IN-77).
Game code (6142): `DrumsPlayer` input method RVA `0x33A880` (`dump.cs` line 10158); Rewired reads RVA `0x1F39540` and RVA `0x1F3B730`; the slot field `#901.player`; the slot's Rewired player `#1435+0x18`.
Hydra: no owner: Hydra never reads the running game. The kit's player is `KIT_PLAYER` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D2), from the disassembly; the System Player's hit, 2026-11-06, session 70bfa33d (LS-D1d).

### IN-69 A bot, or a remote player that is playing, is fed the chart's notes as input, and is_bot is read again on every call

Rule: In its input loop, `BasePlayer.Update` calls a method that asks, each time, whether the slot is remote and whether it is a bot. A remote slot whose `BasePlayer.isRemotePlayerPlaying` is clear gets nothing, whatever its "is_bot". Otherwise a bot (its profile's "is_bot" read through the bot test of IN-64) or a remote slot with that flag set gets the next chart note whose time has come, and the loop passes that note's time to the engine's per-frame update as a hit. A player that is neither gets nothing. So for a local player, turning "is_bot" off before a song stops the chart from playing itself, whenever it is turned off.
Label: Proven in the game's code
Evidence: read with tools/ch_mcp/chdis.py on 2026-10-31; no listing of it is committed. In `0x208F50` (`dump.cs` line 48922, a `BasePlayer` method taking the song time and an out double): the remote test `0x337720` on the slot at 0x208FD0; when it is true and the byte at +0x111 is 0, the method clears the player's field +0x130 and returns "nothing" (0x208FD9 to 0x208FFA). Otherwise the bot test `0x13F8B0` on the slot at 0x209013; when it is true the method goes to the note read at 0x209037; when it is false the remote test runs again (0x20902E) and a false answer returns "nothing" (0x209035), a true one goes to the note read. The note read gets the engine's list through the getter `0x20F4F90` (0x209071; the getter returns the engine's field +0x1E0, `dump.cs` line 530043), reads the index at the player's +0x128, takes the item (0x2090DB), compares the time with the item's +0x28 (0x2090EC), and on a due item stores the time in the out value and returns true (0x2090F7 to 0x209116). The player's +0x120 is `BasePlayer.engine` (`dump.cs` line 48770) and its +0x111 is `BasePlayer.isRemotePlayerPlaying` (line 48767). In `BasePlayer.Update` (`0x20A1B0`, `dump.cs` line 48872) the call is at 0x20A400 inside a loop, and a true answer leads to the call of the engine's update `0x20F4550` at 0x20A425 with that time. That the list holds the chart's notes rests on the compare with a time at +0x28; the list's type was not looked up. The dump's name for the getter (`0x20F4F90`) is a shared name and was not used. What sets `BasePlayer.isRemotePlayerPlaying` was not traced.
Game code (6142): RVA `0x208F50`; `BasePlayer.Update` RVA `0x20A1B0`; tests RVA `0x337720` and RVA `0x13F8B0`; `BasePlayer.isRemotePlayerPlaying`; engine update RVA `0x20F4550`.
Hydra: no owner: Hydra has no bot model.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D2), from the disassembly; the direct-start plan (docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)) named the method as a lead.

### IN-70 profiles.ini lives in Documents\Clone Hero; the LocalLow copy is only a source to copy from

Rule: The game keeps `profiles.ini` and `profiles.bin` in its Documents folder (`Documents\Clone Hero` on Windows, the folder that also holds `settings.ini` and `scorestats.json`). When a static `GlobalVariables` switch is on, it uses a "PlayerData" folder beside the game instead. The older place, Unity's data folder (LocalLow\srylain Inc_\Clone Hero), is only a source: when the Documents file is missing, the load copies the old file across, and it never writes or deletes the old one. When both new files exist, the load deletes `profiles.bin`. A profile numbered -1 is built during the load and dropped, never added to the list.
Label: Proven in the game's code
Evidence: the folder method `0x307B40` returns .NET's Environment GetFolderPath with folder 5, joined with "Clone Hero" (0x307BC1 to 0x307BFB), or, when the static bool tested at 0x307BBB is set, the parent of Unity's Application dataPath joined with "PlayerData" (0x307C17 to 0x307C6D). Folder 5 is .NET's SpecialFolder Personal, the user's Documents folder; on this machine that is `C:\Users\Patrick\Documents`, read from Windows on 2026-10-31, and the game folder holds no "PlayerData" folder. The profile list's constructor `0x1CB350` joins that folder with "profiles.bin" into +0x10 and "profiles.ini" into +0x18 (0x1CB3D6 to 0x1CB432) and registers the ini name "profiles" with that path (`0x2112BF0`, at 0x1CB46E). The load is `code/6142/profile_save.asm`: lines 51 to 67 (the old paths from Unity's Application persistentDataPath), lines 71 to 109 (the delete and the two copies), lines 190 to 207 (the -1 test and the add). `code/6142/profile_save.md` explains the lines. That persistentDataPath is the LocalLow folder is Unity's documented place, not read in code. On 2026-10-31 neither folder held a `profiles.ini` or a `profiles.bin`. What sets the static bool was not traced.
Game code (6142): folder method RVA `0x307B40`; profile list constructor RVA `0x1CB350`; load RVA `0x1C9CA0`; ini register RVA `0x2112BF0`; the paths `#792+0x10` and `#792+0x18`.
Hydra: no owner: Hydra never reads the running game. The tools back the two files up from `DOCUMENTS_DIR` (`DOCUMENTS_FILES` in tools/ch_mcp/game_files.py), where the game keeps them. The LocalLow copies are not backed up: the game only reads them, so a run cannot change them.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D2), from the disassembly.

### IN-71 Whether the loader's profile clone is ever saved to profiles.ini

Rule: Not settled. The command-line loader works on a clone of the profile (IN-61), and the clone is built with the number -1 and an empty ini name, where a loaded profile gets the ini name "profiles". Proven in the game's code: the ini save method `0x2112DB0` skips every key whose ini name is empty, so the clone's settings do not reach `profiles.ini` through it. Whether another writer saves them is not known, because five other ini writers were not read through.
Label: Unknown
Evidence: the clone `0xD8470`, called at line 253 of `code/6142/cli_loader_3968D0.asm`, builds its profile with `mov r8` holding the empty string and `mov edx, 0xffffffff` (0xD84C9 to 0xD84DE) through the profile constructor `0xD8A90`, which keeps the ini name and the section "profile{0}" in the profile's +0xC0 (0xD8BE5 to 0xD8C1C, through `0x2115820`). The load passes "profiles", or "" for -1 (`code/6142/profile_save.asm` lines 190 and 191). The save method: `code/6142/ini_save_2112DB0.asm` lines 100 to 105 compare each key's ini name with "" and jump to the next key, before the method looks at the name it was given (lines 134 to 141) or its path (lines 154 to 168). A file's path comes only from the name-to-path dictionary (`code/6142/ini_path_lookup_2115760.asm` lines 24 to 40), and of the 9 direct registrations found (byte scan of il2cpp and .text) only the one at 0x1CB46E pairs a name with the `profiles.ini` path, and that name is "profiles". Not read through: the two File.WriteAllText methods `0x2113710` and `0x2113C60` (read with tools/ch_mcp/chdis.py, not kept; neither takes an ini name) and the three methods that call the path lookup and then the document writer `0x2165C50` (`0x21134B0`, `0x21135B0` and `0x2114A80`, `code/6142/ini_path_lookup_2115760.md`). The runs: the `ch_stop` restore lines of all 15 human runs under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\` (2026-10-31 to 2026-11-07), which start through the command-line loader, say `profiles.ini` was unchanged or restored, and 11 say it was absent before and after (for example `2026-10-31\ls-d0-run1\replies.txt` line 36); they cannot tell "skipped" from "nothing to save". The Documents folder today holds no `profiles.ini`, `profiles.bin` or `.ini` other than `settings.ini`. Where we looked first is in `code/6142/profile_save.md`.
Game code (6142): clone RVA `0xD8470`; profile constructor RVA `0xD8A90`; ini key RVA `0x2115820`; ini writers RVA `0x2113710`, RVA `0x2113C60` and RVA `0x2112DB0`; path lookup RVA `0x2115760`.
Hydra: no owner: Hydra never reads the running game. The tools back up `profiles.ini` in `DOCUMENTS_FILES` in tools/ch_mcp/game_files.py, the folder IN-70 gives.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D2), from the disassembly; the save method and the runs, 2026-11-08, session 428cf079 (unknowns audit).

### IN-72 A player's stats record goes into scorestats.json on disk, with is_bot as one of its values

Rule: The stats writer of IN-38 writes one player's block of JSON, and its text is written to a local file. Its first caller builds `scorestats.json`, which `EndOfSong.Start` writes in `Documents\Clone Hero` (IN-70's folder). Its second caller builds another JSON text that another `EndOfSong` method writes to a file in the same folder, whose name is not known. "is_bot" is a value written into the block; the writer does not skip a bot.
Label: Proven in the game's code
Evidence: `code/6142/score_submit.asm` lines 407 to 410 (the byte at +0xB6 written as "is_bot"). A scan of the DLL's code for direct calls found two callers. The first is `0x252630` (line 230 of `code/6142/scorestats_json_252630.asm`), whose only direct caller is `EndOfSong.Start` (line 266 of `code/6142/scorestats_writer_15DC20.asm`), which joins the folder from `0x307B40` with "scorestats.json" (lines 246 to 260) and writes the text with File.WriteAllText (line 270). The second is `0x253EF0`, whose only direct caller is the `EndOfSong` method `0x161C70` (call at 0x1621B1, File.WriteAllText at 0x1621BF); the string labels in those two methods do not fit the code, so that file's name is not given. That `EndOfSong.Start` skips the file for a bot was not looked for; it picks the "botnosave" text when `GlobalVariables.instance` +0x7C is 2 (`code/6142/scorestats_writer_15DC20.asm` lines 1046 to 1071), and what that value means was not traced. `code/6142/score_submit.md` explains the lines.
Game code (6142): stats writer RVA `0x1144C0`; text writers RVA `0x252630` and RVA `0x253EF0`; `EndOfSong.Start` RVA `0x15DC20`; `EndOfSong` method RVA `0x161C70`.
Hydra: no owner. The tools back `scorestats.json` up in `DOCUMENTS_FILES` in tools/ch_mcp/game_files.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D2), from the disassembly.

### IN-73 Whether a finished song posts anything online when nobody is logged in

Rule: Not settled. The stats record reaches only local files through the direct calls found (IN-72). Whether the game also sends a score or stats online after a song, with or without a login, and whether "is_bot" decides it, was not traced.
Label: Unknown
Evidence: looked at the callers of the stats writer `0x1144C0` and of its text writers by a scan for direct calls only; calls through method tables, delegates and tasks were not followed, and no network code was read. The DLL does hold an online service: the text "Making session request to gateway: https://gateway-service.clonehero.net/api/session/request" is loaded in the method `0x285AD0`. No direct caller of it was found (byte scan of il2cpp and .text, `docs/audit/2026-11-08-clone-hero-unknowns/W9b/xref_scan_output.txt`); one 8-byte pointer to it sits in `.data` at RVA 0x3C55C68, most likely the method pointer table, which was not checked. That text rests on the string-slot map, which a rescan found right against the file: all 23,972 slots decode as the game's own init helper does to the text chdis shows. One oddity stays unexplained: the gateway request URL's slot is also loaded in `GHLNoteContainer` method `0x1F53B0` and passed to `UnityEngine.SortingLayer.NameToID` (at 0x1F544D), where a URL cannot fit; a live read of the slot would settle it. The texts that look like the stats feature, "Fetching gateway stats for version: " and the "session/stats?backendVersion=" URL, sit in an async state machine's method `0x2EE6A0` (`dump.cs` line 65492), which was not listed; "Fetching" reads as a download, not a post. The game logs in the run folders were not read. The game's log showing "Unauthorized" for analytics in bot runs is one observation recorded in the direct-start plan (docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md), section 3), not a code fact. See `code/6142/score_submit.md`.
Game code (6142): not found: looked from RVA `0x1144C0` through RVA `0x252630`, RVA `0x253EF0`, RVA `0x15DC20` and RVA `0x161C70`, and at RVA `0x285AD0`.
Hydra: no owner: Hydra never reads the running game.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session b15bb484 (task D2), from the disassembly; the caller and string scans, 2026-11-08, session 428cf079 (unknowns audit).

### IN-74 A paused command-line start takes the one-byte patch before the loader runs, and the song plays with no press

Rule: When the game is started paused with `--song` and `--player Drums,Expert`, GameAssembly.dll is not yet mapped when the paused game is let go. The DLL maps within about half a second of the resume. A patch written at that moment, turning the loader's `mov edx, 1` into `mov edx, 0` (IN-61), is in place before the loader's first run. The loader then runs once, reading the patched byte, and the song starts and plays with no Ready press. The game still came to the front once, at the launch. LS-D0 read no player kind; a later full song on this start wrote is_bot false in its stats and gained Star Power, so the player is a human (IN-64). The same start works with `--player ProDrums,Expert`.
Label: Proven in a live run
Evidence: LS-D2d run 1 of the same plan, 2026-11-06, wrote is_bot false, 25 phrases earned and 9 activations in its stats copy (`C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\`). LS-D2a run 1 of `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)`, 2026-11-06, on the Senri chart as ProDrums, gave the same bytes, 0.5 s to the patch, one loader run and a moving clock (`C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2a-run1\replies.txt`). LS-D0 run 1 of `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)`, 2026-10-31, on Window Map. Result files in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-31\ls-d0-run1\`: `replies.txt` (the ch server's replies) and `launch-20261031-141438.jsonl` (the launch agent's log). The arm reply listed 39 loaded modules, none of them GameAssembly.dll. The patch landed 0.5 s after the resume, with `BA 01 00 00 00` before and `BA 00 00 00 00` after, and the loader's first twelve bytes matching. The status then counted one loader run, which read `BA 00 00 00 00`. Two state reads gave songTime 6.32 s and 9.42 s with isSongPlaying true and isPaused false, and no press was sent. The foreground watch counted one time the game came to the front, at 0.0 s. The DLL's MD5 was `106E15E5BF799EA62F916E8CEAF38BC6` before the run and after it.
Game code (6142): the loader RVA `0x3968D0`; the patched instruction at 0x396EC5, inside it (IN-61).
Hydra: no owner: Hydra never reads the running game. The tools start the game this way in `launch_human` in tools/ch_mcp/bot_run.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 90d40bc9 (LS-D0 run 1), from a live run; the ProDrums start, 2026-11-06, session 70bfa33d (LS-D2a, LS-D2d).

### IN-75 Rewired's Controller class has an enabled property, and the keyboard switch on the controller helper is obsolete in its favour

Rule: Every Rewired controller, the keyboard and the mouse among them, has an "enabled" property with a getter and a setter. The controller helper's older "keyboardEnabled" property is marked obsolete, and its note says to switch the keyboard off through the keyboard controller's "enabled" instead.
Label: Proven in the game's code
Evidence: `dump.cs` line 285668 (the class Controller in the namespace Rewired, TypeDefIndex 3374), lines 285739 and 285740 (get_enabled), lines 285742 and 285743 (set_enabled); line 309787 (the class ControllerHelper inside ReInput, TypeDefIndex 3740), lines 309798 and 309799 (its Mouse and Keyboard properties) and lines 309800 and 309801 (the Obsolete attribute on keyboardEnabled, "Use Controller.enabled instead"). The dump holds signatures only, so what the setter does inside was not read. The kit switches the keyboard and mouse off through this setter; tools/ch_mcp/kit_methods.json lists the calls with these dump lines.
Game code (6142): the getter RVA `0x1D5C350` and the setter RVA `0x1D5D1F0`, on the class with TypeDefIndex 3374.
Hydra: no owner: Hydra never reads the running game. The kit's allowed calls are listed in tools/ch_mcp/kit_methods.json.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session f018b7c2 (the kit's keyboard and mouse switch-off), checked in the dump on 2026-10-31, session 90d40bc9.

### IN-76 Rewired gives a custom controller no default map on the System Player

Rule: A custom controller can be added to Rewired's System Player during a song. Asking that player to load its default maps for custom controllers then loads none, so the controller has no bindings there. The reason is in the game's Rewired setup: System's list of default maps for custom controllers is empty, while Player1 to Player4 each list map category 1 ("Gameplay") with layouts 0 ("Default") and 2 ("Drums"). The drum map for the kit's custom controller definition, 6 ("SLInputDrums"), is the one at category 1 and layout 2.
Label: Proven in a live run
Evidence: LS-D1 run 1 of `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)`, 2026-10-31, on Window Map after a direct start. Result files in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-31\ls-d1-run1\`: `replies.txt` and `kit-1.jsonl`. The setup reply says the kit was created and added to System, and its maps list for the kit is empty, so `ch_kit_setup` refused and tore the kit down with every check ok. The Rewired setup was read on 2026-10-31 from the game's own files: the Rewired InputManager MonoBehaviour in `Clone Hero_Data\level1` (object 9186) and its copy in `Clone Hero_Data\sharedassets0.assets` (object 452), which agree. The game has no type tree for them, so they were decoded by hand against the field lists in `dump.cs` (UserData at line 318932, Player_Editor at 316885 with its Mapping class at 316625, ControllerMap_Editor at 330885, CustomController_Editor at 316302); the decode script was not kept. The decoded map for definition 6 binds elements 0 to 6 and 20 to 22 to the same actions as `KIT_BINDINGS` in tools/ch_mcp/kit_tools.py, which checks the decode. That LoadDefaultMaps loads exactly a player's default list is inferred from this and the run; the dump holds signatures only. The map helper also has a LoadMap that takes a category and a layout (`dump.cs` lines 303867 and 303889), which this run did not try. LS-D1a run 1 of `docs/superpowers/plans/2026-10-31-ch-mcp-kit-map-and-network-block.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-kit-map-and-network-block.md)` tried it on 2026-10-31, on Window Map after a direct start: the five-argument LoadMap(Custom, kit id, 1, 2, true) on System's map helper threw "access violation accessing 0x58", so it loaded no map there. Result files in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-31\ls-d1a-run1\`: `replies.txt` and `kit-1.jsonl`, whose stack names the LoadMap call. LS-D1a run 2 made the same call on 2026-10-31 with the crash logger in place, and it faulted the same way. Result files in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-31\ls-d1a-run2\`: `replies.txt` and `kit-1.jsonl`. The fault's place is in Frida's own agent DLL, not in the game: offset 0x1E496C of the x86_64 frida-agent.dll that Frida unpacks into the user's Temp folder. Disassembled, that is the instruction right after libffi's Win64 call into the target function, the read that restores a saved register from the caller's frame pointer plus 0x58, and the logged frame pointer (rbp) is 0. So the LoadMap call came back, or was unwound back, with the caller's frame lost; the game read no field at offset 0x58. The game's Player.log holds no line from the call. The cause is the call's argument layout, read from the code on 2026-10-31. The five-argument LoadMap (RVA `0x1F508C0`, `dump.cs` line 303889) is an instance method that returns void. Compiled, it takes the player's map helper, the five arguments and one more pointer, the method's own description, which il2cpp-compiled code passes last. Its first instructions write 0 into that last stack slot before they jump on. frida-il2cpp-bridge 0.14.0 adds that last pointer only for a generic method (its dist/index.js under tools/ch_mcp/agent/node_modules, lines 2512 to 2513), so it passed six slots, and the seventh, which the stub cleared, was libffi's saved frame pointer. The bridge passes the `ControllerType` enum correctly, as a plain int (Custom is 20). The four-argument LoadMap (`dump.cs` line 303867) writes no slot past its own arguments. LS-D1a run 3, on 2026-10-31 with the user's go past the step's two runs, made the same call by hand with that last pointer passed, and it worked: on System the kit got the map at category 1 and layout 2, enabled, with 10 bindings that all match `KIT_BINDINGS`, and the teardown's checks were all ok. Result files in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-31\ls-d1a-run3\`: `replies.txt` and `kit-2.jsonl`. So LoadMap by category and layout loads the drum map onto System even though System's default list is empty. That run sent no press, so whether kit presses on System reach the drums input is not yet known.
Game code (6142): AddController RVA `0x1F2D2B0` (`dump.cs` line 306697) as listed in tools/ch_mcp/kit_methods.json, and LoadDefaultMaps RVA `0x1F50710` (`dump.cs` line 304761), which the kit no longer calls; it loads the map with LoadMap (RVA `0x1F508C0`, `dump.cs` line 303889).
Hydra: no owner: Hydra never reads the running game. The tools hand the kit to a player in `ch_kit_setup` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 90d40bc9 (LS-D1 run 1), from a live run; the access violation, 2026-10-31, session 6e9d8be4 (LS-D1a run 1); the fault's place and the map on System, 2026-10-31, session d1867340 (LS-D1a runs 2 and 3).

### IN-77 A press on the virtual kit reads one frame later, with the game minimized and unfocused

Rule: With Rewired's switch that drops input in an unfocused window set off, a virtual drum kit takes presses while the game is minimized and not in front. In these runs the kit was made from custom controller definition 6 ("SLInputDrums"), the definition the first probe picked, not the one the game makes a real MIDI kit from, which is definition 1 (IN-82, IN-83); it was given to Player1. Its Start press reads as Start one frame later. A pad press reads as its lane one frame later, then off on release. One write is enough for a whole press, and it need not come from the game's main thread. The same holds with the kit on the System Player after a human command-line start, and there a kick press scores a hit on a kick note. A press read back by name uses the action's plain name, such as Orange; the setup reply prints the action's descriptive name, such as mapperOrange.
Label: Proven in a live run
Evidence: the probe of 2026-10-31, recorded in docs/sessions/2026-10-31-a5f17ca5.md and in "The probe facts" in `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md)`. Start was written every 2 ms for 150 ms and read true one frame later. The Tom3 pad, bound to Green, was written at 1.0 for 100 ms, and Green read true one frame later and false on release. LS-K1 of the same plan, 2026-10-31, recorded in docs/sessions/2026-10-31-a379bc75.md, wrote each press once. Three presses of Green, held 1, 2 and 3 frames, each read on from the frame after the down write. The comment on `KIT_REWRITE` in tools/ch_mcp/kit_tools.py quotes the frames of the 2-frame hold and says all three ran off the main thread. The probe's scripts and the runs' raw replies were not kept. LS-D1b to LS-D1d of `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)`, 2026-11-06, on Window Map, result files in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d1b-run1\`: `replies.txt`, `kit-1.jsonl` and `trace-2.jsonl`. Orange read on during a kick held 2 frames on System, and a kick held 30 frames traced a hit at 49448.0 ms, 8 ms after note 132. Every other traced press sat 143 ms or more from any note and traced nothing, so that run shows no false hit and no missed one. In LS-D2d, 2026-11-06, on the Senri chart, the auto-player's 2,342 presses, each held 2 frames, all hit (result files in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\`). The action's two names were read on 2026-11-06 from the Rewired setup in `Clone Hero_Data\level1` and `Clone Hero_Data\sharedassets0.assets`, where each action stores its name and then its descriptive name; the read script was not kept. The read-back goes through GetButton by name (tools/ch_mcp/kit_methods.json).
Game code (6142): not found: no game method was read for this. The Rewired calls are listed in tools/ch_mcp/kit_methods.json.
Hydra: no owner: Hydra never reads the running game. The tools press the kit with `ch_kit_press` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session a5f17ca5 (the probe); 2026-10-31, session a379bc75 (LS-K1); 2026-11-06, session 70bfa33d (LS-D1b to LS-D1d, LS-D2d).

### IN-93 Inside a hook on GameManager.Update, a kit write reads one frame later, and the engine clock holds the frame before's songTime

Rule: A hook that runs as `GameManager.Update` returns can write the virtual kit and call the kit player's button reads on every frame without a fault. A press written there reads as pressed on the next frame, and its release reads as off on the frame after the release write. At that point the engine's song clock (TM-13) still holds the `GameManager.songTime` the game stored one frame earlier, so the engine clock trails `songTime` by exactly one frame's step. Which part of the frame copies `songTime` to the engine was not read.
Label: Proven in a live run
Evidence: LS-G1 of `docs/superpowers/plans/2026-11-12-in-game-press-timing.md`, 2026-11-12, on The Who - Won't Get Fooled Again (O), ProDrums Expert, the kit on the System Player after a human command-line start; result file `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\ls-g1-run1\frame_probe-1.json`. The hook logged 1,649 frames, 2733 to 4381, each one above the last, over 4,997 ms of wall time, with no fault and the hook out afterwards. Green went down on frame 2733 and up on frame 2735; Green and GetAnyButton first read pressed on frame 2734 and read off again on frame 2736. On all 1,648 frame pairs, the engine clock read on a frame equals the `songTime` read on the frame before. The engine clock minus `songTime` had median -3.030 ms and widest -4.373 ms, which is that frame's step. The rate was 329.8 frames a second with no compiler running.
Game code (6142): `GameManager.Update`, RVA `0xC05A0`, the method hooked; `GameManager.songTime`; field `#8288+0x100`, the engine clock (TM-13). The hop from `songTime` to the engine was not read.
Hydra: no owner: Hydra never reads the running game. The tools run the probe with `ch_kit_frame_probe` and judge its reply with `frame_probe_lines` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-12, session 1ac0d6b4 (LS-G1)
### IN-78 The game takes the front just after launch, and keeps running behind other windows

Rule: Asked to start minimized and not in front, the game still comes to the front 0.4 to 0.5 seconds after launch. In LS-K2 it kept the front until the user clicked another window. With `pause_on_focus_lost` at 0, as the tools set it for a run, the game keeps running when it is not in front.
Label: Proven in a live run
Evidence: LS-K0 and LS-K2 of `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md)`, 2026-10-31, recorded in docs/sessions/2026-10-31-a379bc75.md. LS-K0's foreground watch saw the game come forward about 0.5 seconds after launch. It did not time how long the game stayed there. In LS-K2 the game came forward about 0.4 seconds after launch and stayed until the user clicked away. During LS-K2's 20-second watch, with another window in front, the frame count rose from 2829 to 6127. No result file of these runs was kept. IN-74 saw the same single trip to the front on a paused start.
Game code (6142): not found: no game method was read for this.
Hydra: no owner: Hydra never reads the running game. The tools ask for the minimized start in tools/ch_mcp/bot_run.py, count trips to the front in tools/ch_mcp/foreground.py, and set the run's pause line in tools/ch_mcp/game_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session a379bc75 (LS-K0 and LS-K2), from live runs.

### IN-79 Four kit presses go from the title screen to the player screen, and the active read cannot tell the profile screen from the main menu

Rule: From the title screen, the kit's Start joins a player, and the game's player count goes from 0 to 1. The first Green picks the profile. The second Green opens the song list: the main menu goes inactive and the song list goes active. The third Green picks the highlighted song, sets the game's current-song folder and switches the player screen on. The profile screen and the main menu both read active from the moment the game opens, so that read cannot tell those two apart.
Label: Proven in a live run
Evidence: LS-K3 run 1 of `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md)`, 2026-10-31, recorded in docs/sessions/2026-10-31-597e0795.md. The active read is Unity's activeInHierarchy on the menu's game object, in tools/ch_mcp/agent/menu.ts. The first Green changed nothing the reads could see, so that it picked the profile is taken from the next press opening the song list. The run stopped on the player screen by the user's choice, before any Ready press, so this block says nothing about the song loading. No result file of this run was kept.
Game code (6142): not found: no game method was read for this.
Hydra: no owner: Hydra never reads the running game. The kit's menu steps are in tools/ch_mcp/menu_steps.json.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 597e0795 (LS-K3 run 1), from a live run.

### IN-80 What the keyboard does in the menus has not been tried on the game

Rule: Not settled by any run. The manual says Space opens the control mapper from the main menu. On this install Green and Blue are the keys A and K. A key sequence through the menus was written from the manual, and no run has confirmed it.
Label: Unknown
Evidence: the Space line is the manual's Keyboard and Gamepads page, read 2026-10-30, as `MENU_KEYS` in tools/ch_probe/input_driver.py and docs/agents/clone-hero-mcp.md record it; no tool sends Space. A and K are in `DEFAULT_BINDINGS` in the same file, whose comment says they were read off the game's Controller Remap screen on 2026-09-25, while docs/sessions/2026-10-31-4c48c8e5.md's write-back line gives them with the manual's facts. That card marks the key sequence unverified until live step LS1 of `docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md)`. No later block settles it. The first key-press step was paused before any press (docs/sessions/2026-10-31-a5f17ca5.md), and the walk was rebuilt on kit presses. So tools/ch_mcp/menu_steps.json now holds kit items only and is still marked not verified, and IN-79 shows kit presses, not keys.
Game code (6142): not found: no game method was read for this.
Hydra: no owner: Hydra never reads the running game. The tools' key table is `MENU_KEYS` in tools/ch_probe/input_driver.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-31, session 4c48c8e5, from the manual; no run.

### IN-81 A custom controller's id comes from a counter that destroying a controller does not move

Rule: Rewired gives each new custom controller the number in one counter on its controller manager, then adds one to the counter. Destroying a controller leaves the counter alone, so an id is not handed out twice until the counter is reset or wraps. The counter starts at 0, so with nothing made before it, the seventh custom controller of a launch gets id 6, and a kit set up again in the same launch gets a higher id. Two things can put the counter back to 0: the write at line 47 of the listing, which runs only when the new value reaches 0x7FFFFFFF (about 2.1 billion creates, so never in a launch), and the reset at 0x1EEB716, whose trigger is not known. So "only counts up" holds for a launch only if that reset does not run in it.
Label: Proven in the game's code
Evidence: `code/6142/create_custom_controller_1EE9800.asm` lines 42 to 47 read the counter and write it back one higher, and line 74 passes the old value on. `code/6142/create_custom_controller_1EE9800.md` follows that value through the three constructors to `Controller.id`, and gives the search of class #3552 that found no change to the counter on the destroy path. The reset to 0 is at 0x1EEB716 in `0x1EEB3F0`, read with tools/ch_mcp/chdis.py on 2026-11-06 with no listing of its own. The start at 0 fits the saved runs: the kit reads id 0 in `kit-2.jsonl` of ls-d1a-run3, ls-d2b-run1's `kit-2.jsonl` and ls-d2d-run1's `kit-3.jsonl`, under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`. LS-O1a of `docs/superpowers/plans/2026-11-06-optimal-score-run.md` (removed; `git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md`) showed the count past 0 on 2026-11-06: six placeholders took ids 0 to 5 and the kit took id 6 (`replies.txt` in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-o1-run1\`). So the reset did not run between that launch and that setup; what triggers it is still not known.
Game code (6142): #3552, the controller manager: the create method RVA `0x1EE9800`, the counter `#3552+0xD0`, and the public form RVA `0x1D6F9C0` (`dump.cs` line 309950); `code/6142/create_custom_controller_1EE9800.asm` lines 42 to 47 and 74.
Hydra: no owner: Hydra never reads the running game. The tools rest on this in `KIT_PLACEHOLDERS` and `KIT_REQUIRED_ID` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-06, session 5b92c813.

## The game's MIDI kit and the drum controller definitions

### IN-82 A real MIDI kit is custom controller definition 1, "MIDI Drums": ten axes and no buttons, bound by map 2

Rule: The game's Rewired setup ships custom controller definition 1, named "MIDI Drums". It has ten elements, every one an axis: Snare 0, Tom1 1, Tom2 2, Tom3 3, Cymbal1 4, Cymbal2 5, Cymbal3 6, Kick 7, StartPad 18 and SelectPad 19. It has no buttons. Its drum map, id 2 at category 1 ("Gameplay") and layout 2 ("Drums"), binds those elements to the same ten actions as definition 6's map: Snare to Red, Tom1 to Yellow, Tom2 to Blue, Tom3 to Green, the three cymbals to the three cymbal actions, Kick to Orange, StartPad to Start and SelectPad to Select. So on a real kit the kick is an axis.
Label: Proven in the game's code
Evidence: read on 2026-11-08 from the game's own `Clone Hero_Data\level1`, which has no type tree, by hand against the field lists in `dump.cs`, the same way IN-76 read it; the read script was not kept. The definition is bytes 987772 to 989720. Its name "MIDI Drums" is written twice from 987772, its `_id` 1 is at 987804, and the element identifier count 10 is at 987852. The ten identifier records start at 987856, each in the field order of `dump.cs` lines 295089 to 295116 (id, name, five empty strings, the element type, the compound type, the role), and every element type is 0, which is Axis (IN-84): Kick 7 is at 988180 and its type at 988212, StartPad 18 at 988224 and SelectPad 19 at 988272. The axis count 10 is at 988324, with its records from 988328, and the button count 0 is at 989716. The map is bytes 984680 to 985148: id 2 at 984680, category 1 at 984684, layout 2 at 984688, two empty strings, the custom controller uid 1 at 984700 and the binding count 10 at 984704. Ten 44-byte binding records follow from 984708, in the field order of `dump.cs` lines 294184 to 294214 (category, action, element type, element id, then seven more). They bind element 0 to action 1, 1 to 2, 2 to 3, 3 to 0, 4 to 37, 5 to 38, 6 to 39, 7 to 4, 18 to 16 and 19 to 6, every one with element type 0; the record that binds Kick 7 to action 4 starts at 985016. The action names are those of `KIT_BINDINGS` in tools/ch_mcp/kit_tools.py. The register method (IN-85) makes a real kit's controller from this definition.
Game code (6142): not found: the definition and the map are data in `level1`, not a method; their field lists are Rewired's classes at `dump.cs` lines 295089 and 294184.
Hydra: no owner: Hydra never reads the running game. The definition the virtual kit is built from is `KIT_DEFINITION_ID` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 6341c618 (the kick plan).

### IN-83 Definition 6, "SLInputDrums", has its kick as a button; the virtual kit used it because the first probe picked it

Rule: The game's Rewired setup also ships custom controller definition 6, named "SLInputDrums". It has seven axes, Snare 0 to Cymbal3 6, and three buttons, Start 20, Select 21 and Kick 22. Its drum map, id 6 at category 1 and layout 2, binds the same ten actions as definition 1's map 2, with Kick 22 to Orange as a button. No game method read so far makes a controller from definition 6. The virtual kit was built from it because the probe of 2026-10-31 picked it, not because the game uses it for a drum kit; the game's own MIDI kit uses definition 1 (IN-82, IN-85).
Label: Proven in the game's code
Evidence: read on 2026-11-08 from `Clone Hero_Data\level1` in the same way as IN-82; the read script was not kept. The definition is bytes 990908 to 992480: its name "SLInputDrums" twice from 990908, its `_id` 6 at 990940, the identifier count 10 at 990988 and its records from 990992, with Start 20 at 991316, Select 21 at 991364 and Kick 22 at 991412, each with element type 1 (Button), at 991352, 991400 and 991444. The axis count 7 is at 991456 with its records from 991460, and the button count 3 is at 992428 with its records from 992432. The map is bytes 986656 to 987124: id 6, category 1, layout 2, uid 6 at 986676 and the binding count 10 at 986680; its ten 44-byte binding records from 986684 give elements 0 to 6 type 0, and 22 (to action 4), 20 (to 16) and 21 (to 6) type 1, in the records that start at 986992, 987036 and 987080. That map is the one `KIT_BINDINGS` in tools/ch_mcp/kit_tools.py expects and the one LS-D2b run 1 read back live (`kit-2.jsonl` line 2 in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2b-run1\`). The probe's pick is recorded in "The probe facts" of `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md)`, which says `CreateCustomController(6, "hydra-probe")` built a kit from definition 6, "SLInputDrums"; it does not say why that definition was chosen. The session card docs/sessions/2026-10-31-a5f17ca5.md records the user's "Yes, probe it" (01:30 UTC), which approved the in-game probe, but names no definition; in these two records nothing shows the user being asked which definition to use. That no game method makes a controller from definition 6 rests only on the methods read here (IN-85), not on a search of the DLL.
Game code (6142): not found: the definition and the map are data in `level1`, not a method.
Hydra: no owner: Hydra never reads the running game. The virtual kit's definition is `KIT_DEFINITION_ID` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 6341c618 (the kick plan); the probe's pick, 2026-10-31, session a5f17ca5.

### IN-84 Rewired's element types: Axis is 0, Button is 1, CompoundElement is 100

Rule: Rewired's ControllerElementType, the element type a definition's identifier and a map's binding both store, is Axis = 0, Button = 1 and CompoundElement = 100.
Label: Proven in the game's code
Evidence: `dump.cs` lines 290899 to 290906 (the enum in the namespace Rewired, TypeDefIndex 3494). It is an enum, so the values are not checked (`docs/clone-hero/README.md`, "What stays unchecked"). The identifier's `_elementType` field is at `dump.cs` line 295110 and the binding's at line 294190.
Game code (6142): not found: an enum, not a method, and not checked.
Hydra: no owner: Hydra never reads the running game. The kit's setup checks the bindings it reads back in `binding_problems` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 6341c618 (the kick plan).

### IN-85 The game registers a MIDI kit only on a real MIDI port, makes its controller from definition 1, and loads no map itself

Rule: When the game registers a MIDI drum kit for a player, it first needs a drums profile whose flag at +0xB1 is clear (what that flag means was not read) and a MIDI port that exists; with no such port it leaves and makes nothing. Then it opens the port, makes a Rewired custom controller from definition 1 ("MIDI Drums"), adds it to the slot's Rewired player, and keeps a device record that holds the slot, the port and the controller. No instruction of its own calls a map method; the calls it makes that have no listing were not read for one (see Evidence). The velocity helper's MIDI check (method `0x21F790`) asks only whether some device record holds the player's slot, so a player passes that check only after this method has made a record for it.
Label: Proven in the game's code
Evidence: `code/6142/midi_register_21CDE0.asm`, explained in `code/6142/midi_register_21CDE0.md`. Lines 37 to 63 need the slot's profile, with its controller type Drums (2) or FiveLaneDrums (3). Lines 70 to 77 ask RtMidi for the port count at 0021cf30 and leave on `cmp ebx, eax; jg` at 0021cf37 when the port index is past the last port. Lines 109 to 115 call CreateCustomController with 1 (the one-argument form, `dump.cs` lines 309946 and 309947). Lines 117 to 126 call AddController on the slot's player with false. Lines 127 to 161 fill the device record: the slot at +0x10, the controller at +0x30, the enabled flag at +0x48. Lines 172 to 197 add the record to the manager's list, and lines 234 to 253 subscribe the pump (IN-86) to Rewired's InputSourceUpdateEvent. No line calls a map method; the five manager methods it calls that have no listing, and the call at RVA 0x212B040 on an object held by the profile (lines 206 and 270), were not read for one. Lines 42 and 43 leave when the profile's bool at +0xB1 is set. `code/6142/midi_slot_check_21F790.asm` lines 14 to 49 are the check: it walks the manager's list and returns true when a record's slot equals its argument (line 30). The velocity helper calls it at `code/6142/lane_velocity_337E10.asm` lines 38 to 41. A run on a machine with no MIDI port (whether this machine has one was not checked) can pass the velocity gate only by the other check, custom controller id 6 (SC-14).
Game code (6142): MIDIInputManager (TypeDefIndex 990): the register method RVA `0x21CDE0` (`dump.cs` line 52750) and the slot check RVA `0x21F790` (`dump.cs` line 52813); the device list `#990+0x38`; the device record (TypeDefIndex 987, `dump.cs` lines 52504 to 52513) `#987+0x10`, `#987+0x30` and `#987+0x48`; CreateCustomController RVA `0x1D6FA80`; AddController RVA `0x1F2D2B0`; `code/6142/midi_register_21CDE0.asm` lines 37 to 197 and 234 to 253, `code/6142/midi_slot_check_21F790.asm` lines 14 to 49.
Hydra: no owner: Hydra never reads the running game. The virtual kit's definition is `KIT_DEFINITION_ID` in tools/ch_mcp/kit_tools.py, and the id the gate needs is `KIT_REQUIRED_ID` there.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 6341c618 (the kick plan).

### IN-86 The game's MIDI pump writes each pad and the kick as an axis at 0.5 plus velocity over 254, and Start and Select at 1.0

Rule: On each of Rewired's input updates, the game turns each MIDI message a registered kit has queued into an axis write on that kit's custom controller, when the message matches one of ten pad names (the match, method 0x21F840, was not read). The four pads, the three cymbals and the kick are written by element name (Snare, Tom1, Tom2, Tom3, Cymbal1, Cymbal2, Cymbal3, Kick) at 0.5 + velocity / 254, so velocity 127 gives 1.0 and velocity 1 gives about 0.504. StartPad and SelectPad are written at 1.0 whatever the velocity. The velocity helper reads such a value back with the inverse, round((value - 0.5) x 2 x 127), so a real kit's velocity reaches the hit handler unchanged.
Label: Proven in the game's code
Evidence: `code/6142/midi_pump_21E420.asm`, explained in `code/6142/midi_pump_21E420.md`. Lines 86 to 88 load 127.0, 0.5 and 1.0. Lines 137 to 142 make the value from the message's byte at +0x9: over 127, times 0.5, plus 0.5. Lines 151 to 317 write the four pads, the three cymbals and the kick with CustomController.SetAxisValue on the record's controller (+0x30), each after the method at 0x21F840 matches the message to a pad name; the kick is lines 305 to 317 ("Kick Pad" to "Kick", 0021ea43 to 0021ea4a). Lines 327 to 339 and 349 to 361 write StartPad and SelectPad from xmm9, which is 1.0 (0021eaa8 to 0021eab3, 0021eb15 to 0021eb20). The pad names are the manager's string constants, `dump.cs` lines 52723 to 52732. That the byte is the note's velocity is inferred, because the method that fills a message was not read; the listing's note says why it fits. The inverse is `code/6142/lane_velocity_337E10.asm` lines 79 to 103. How often Rewired raises its input update event was not read.
Game code (6142): MIDIInputManager (TypeDefIndex 990): the pump RVA `0x21E420` (`dump.cs` lines 52818 and 52819); SetAxisValue by element name RVA `0x1D5DF10` (`dump.cs` line 286520); `code/6142/midi_pump_21E420.asm` lines 86 to 361.
Hydra: no owner: Hydra never reads the running game. The virtual kit writes the same value with `pad_axis` in tools/ch_mcp/kit_tools.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-11-08, session 6341c618 (the kick plan).

## Saved score files

### IN-55 scoredata.bin keeps one merged entry per chart and instrument, and one play count per chart

Rule: The player's saved score file is `scoredata.bin`; the game joins that name to the folder that 0x307590 returns, which was not read, and the tools use the LocalLow folder (`LOCALLOW_DIR` in tools/ch_mcp/game_files.py line 39). The file starts with the version 20211009 and a record count. Each record is a 16-byte chart hash, an entry count, a 2-byte play count, one more byte, then one 16-byte entry per instrument (instrument, difficulty, a byte, a flag, a short, a byte, the modifier flags and an int). No title or artist is stored. Each finished run adds one to the chart's play count whatever its instrument. It is then merged into that instrument's entry field by field: the larger int at +0x4 wins and brings the difficulty, modifiers and the short at +0xA with it, a tie takes the new run's, the larger bytes at +0xC and +0x14 are kept, and a flag set in either run stays set. That the int at +0x4 is the score was not read from a name.
Label: Proven in the game's code
Evidence: `code/6142/score_store_paths_2F4300.asm` lines 22 to 41 set the path. The writer `code/6142/score_store_write_2F57C0.asm` opens it (lines 30 to 46), writes the version (lines 55 to 57) and the record count (lines 63 to 66), then each record (lines 67 to 90). A record is `code/6142/score_chart_write_2167B0.asm`: the hash (lines 36 to 54), the entry count (lines 55 to 59), the play count (lines 60 to 64), one byte of unread meaning (lines 65 to 69) and the entries (lines 70 to 104); a record with no entries writes nothing (lines 29 to 35). An entry is `code/6142/score_entry_write_2BAB60.asm` lines 8 to 50. The entries sit in a dictionary keyed by instrument (`dump.cs` line 49307). Adding a run: `code/6142/score_chart_add_2161E0.asm` adds one to the record's play count, capped at 0xFFFF (lines 55 to 60), and merges with the instrument's old entry (lines 20 to 51) by `code/6142/score_entry_merge_2BA4F0.asm` lines 13 to 92. The end of a song reaches it through `code/6142/song_entry_add_score_358560.asm` (the add at line 34, the save at line 40); its callers (byte scan of il2cpp and .text) are `EndOfSong.Start` and the `EndOfSong` method `0x161C70`. The live file and a backup parse to their last byte in this layout, version 20211009, two records with one Pro Drums Expert entry each (`docs/audit/2026-11-08-clone-hero-unknowns/W8c/read_scoredata.py`, output `W8c/read_scoredata.out.txt`). Every tool backup equals the live file, and no after-run copy is kept, so no kept file shows a run changing it; that a human run changes `scoredata.bin` and `scoresext.bin` is from session 6a549de0's own record, which is not kept, and whether a bot run reaches the save was not read. Session e8435f64's card (docs/sessions/2026-10-09-e8435f64.md) says dmleaderboards keeps only the best run per chart and copies play_count from this file.
Game code (6142): score store paths RVA `0x2F4300`; writer RVA `0x2F57C0`; record writer RVA `0x2167B0`; entry writer RVA `0x2BAB60`; run add RVA `0x2161E0`; entry merge RVA `0x2BA4F0`; `SongEntry` method RVA `0x358560`.
Hydra: no owner. The chart hash is Clone Hero's song id (page 01, CR-10); the tools back the file up and restore it in tools/ch_mcp/game_files.py.
Ruling: none needed: no Hydra rule reads it.
Found: 2026-10-10, session c8856afd; 2026-10-09, session e8435f64; the open play-count question, 2026-11-08, session 77057df2 (docs-gaps search); the writer, 2026-11-11, session 428cf079 (unknowns audit).

## Rows waiting on a live run

IN-19, IN-20, IN-21, IN-27, IN-29, IN-36, IN-44, IN-80. See `live-checks.md`.

## Open splits

IN-16 (Q-76), IN-22 (Q-72), IN-23 (Q-77), IN-27 (Q-79). See `questions.md`.
