# Scoring [SC]

This page covers note values, the multiplier and combo, accents and ghosts, the solo bonus, star cutoffs and the score breakdown.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-11-22.

The run files named below are of three kinds. The three bot runs of 2026-10-28 are in C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\l1\bot\ (outside the repo): Window Map, and Trippin SP Spike on Drums and on ProDrums. "Run 1" is the key-press auto-player run on Trippin SP Spike, `run1_cal0_scorestats.json` under tools/ch_probe/experiments/results/overlay-spike/. The virtual kit runs of 2026-11-06 to 2026-11-08 each have a folder under C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\<date>\, named by its run (such as ls-o3-run1), holding the run's scorestats file. With the bot files that makes 16 scorestats files under that folder. Engine fields belong to the base engine class #8288; the drum engine is #8255. `ghidra_decompile_all.txt` is Ghidra's full decompile of build 6142, which `code/README.md` describes; it is not in the repo.

## Note values and the cymbal bonus

### SC-01 A gem is worth 50 points, and a chord pays per gem

Rule: Each gem pays a base of 50 points. A chord pays the base once for each gem in it. The gem count is the note's stored count when set, otherwise the number of set bits in its lane mask.
Label: Proven in the game's code
Evidence: The base adder works out the gem count at lines 17 to 38 (stored count at note +0x50, else a popcount of the lane mask). The drum hit routine passes 50 as the base at line 127 of its listing. The game also holds 50 as a named constant of class #8611 (dump.cs line 549143). Live: Window Map's base_score 7,100 is 142 gems times 50.
Game code (6142): adder RVA `0x20F6EB0`, `code/6142/add_note_score_20F6EB0.asm` lines 17 to 38; drum hit routine RVA `0x20DE770`, `code/6142/note_hit_dynamics_20DE770.asm` line 127
Hydra: `kNoteBasePoints` in src/core/model.h line 144; `ChordNote::basescore` in src/core/model.cpp line 84
Ruling: none needed: same rule. No decision names the 50 itself; D51 call 13 (items in docs/audit/2026-10-04-phase-7-questions.md item 13) records the cymbal's +15, the solo's 100 and the multiplier steps "as they are", not the 50.
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-02 A cymbal gem pays 15 more

Rule: A gem on a cymbal lane pays 65 instead of 50. The test is any of three lane-mask bits (0xE0).
Label: Proven in the game's code
Evidence: The drum score method calls the cymbal test and adds 15 at lines 13 to 16 before tail-calling the base adder at line 31. The cymbal test (RVA `0x214BE30`) is `test word [note+0x80], 0xE0`, as the star base's note says too (`code/6142/star_base_20F4680.md` line 16); the reviewer of this page also read it with tools/ch_mcp/chdis.py on the 6142 DLL, 2026-10-30. The same +15 shape is in the star base (SC-27).
Game code (6142): RVA `0x20DEEC0` (drum override of slot 6), `code/6142/note_score_add_20DEEC0.asm` lines 13 to 16 and 31
Hydra: `kCymbalBonusPoints` in src/core/model.h line 145; `ChordNote::basescore` in src/core/model.cpp line 84
Ruling: none needed: same rule (D51 call 13 records the cymbal's +15 "as they are")
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-03 A hit adds points times the multiplier, with no speed term

Rule: A hit raises the score by its points times the current multiplier, and adds the plain points to a raw total. Nothing in the adder depends on playback speed. The adder returns at once, scoring nothing, when the engine's player kind is RemotePlayer (value 1, SP-23); a player, bot or replay engine scores.
Label: Proven in the game's code
Evidence: Lines 13 and 14 compare the player kind at +0x190 with 1 and skip the whole adder. Lines 36 to 41 add points to +0xC0 and points times +0x48 to the score at +0x94. ghidra_decompile_all.txt lines 53749 to 53751 read the same. A speed term applied to the points before the adder is not ruled out; a half-speed video matched Hydra's curve point for point (card e05522d2).
Game code (6142): RVA `0x20F6EB0`, `code/6142/add_note_score_20F6EB0.asm` lines 13 and 14 and 36 to 41; `#8288+0x94`, `#8288+0x190`
Hydra: `category_scores` in src/core/scoring.cpp line 42, which reads no speed, so points are the same at every song speed (page 3, TM-42 and TM-44); `is_base_speed` in src/net/dmbot_client.h line 37 sets apart off-speed leaderboard scores
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-04 A second adder adds ready-made points into its own total

Rule: Another adder takes a ready-made number of points, applies the same multiplier and Star Power split, and keeps its raw total at +0xC4. No drums caller of it was found.
Label: Proven in the game's code
Evidence: Lines 7 to 30 of its listing. A byte scan of the DLL for direct calls found three generic engine wrappers only; a scan can miss virtual calls. The nearest scorestats field is sustain_score (0 on every drums run), not tied by code.
Game code (6142): RVA `0x20F5F70`, `code/6142/add_points_20F5F70.asm` lines 7 to 30; `#8288+0xC4`
Hydra: no owner: looked in src/core/scoring.cpp and model.h for a second raw total
Ruling: none needed: no drums score is shown to use it
Found: 2026-10-30, session b02ab3e6 (the reference build)

## Multiplier and combo

### SC-05 The multiplier is one plus a tenth of the streak, up to 4

Rule: The multiplier is 1 plus the streak divided by 10 (whole division), and 4 once the streak is 30 or more. So 1x at 0 to 9, 2x at 10 to 19, 3x at 20 to 29 and 4x from 30. Star Power doubles it.
Label: Proven in the game's code
Evidence: Lines 1 and 2 test the streak at +0x88 against 30; lines 3 to 9 divide by 10 and add 1; lines 11 and 12 store 4 above 30; lines 13 to 16 double it when +0x42 is set. At a streak of exactly 30 the division gives 4 too, so the step is at 30, not 31. Live: run 1's multiplier read only 1, 2, 3, 4 and 8.
Game code (6142): RVA `0x20F52E0` (slot 10), `code/6142/multiplier_from_streak_20F52E0.asm` lines 1 to 16
Hydra: `to_multiplier` in src/core/timing.cpp line 12; `kX2StartCombo`, `kX3StartCombo` and `kX4StartCombo` in src/core/timing.h lines 33 to 35; `kStarPowerMultiplier` in src/core/timing.h line 56
Ruling: none needed: same rule (D51 call 13 records the steps at combos 10, 20 and 30 "as they are")
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-06 The streak counts gems, and the multiplier is worked out after each hit

Rule: The multiplier starts at 1. Each gem hit adds one to the streak and to a hit count, then the multiplier is worked out again. The best streak is kept beside it.
Label: Proven in the game's code
Evidence: ghidra_decompile_all.txt line 54329 sets the multiplier to 1 in the constructor; lines 53399 to 53402 add one to +0x9C and +0x88 and call slot 10; lines 53425 and 53426 keep the best streak at +0x98. Live: on Trippin SP Spike notes_hit and max_streak are 1,257, the gem count, not the 857 chords; and the game's combo_score equals Hydra's Combo exactly on every bot run (SC-07), which a step on the wrong gem would move.
Game code (6142): base hit routine RVA `0x20F63F0` (decompile lines 53338 to 53426); constructor RVA `0x20F79E0`
Hydra: `to_multiplier` in src/core/timing.cpp line 12; `multiplier_step` in src/core/timing.cpp
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-07 With no Star Power, the game's combo_score equals Hydra's Combo

Rule: On a run with no Star Power, the game's combo_score equals Hydra's Combo category and its sp_score is 0.
Label: Proven in a live run
Evidence: Trippin SP Spike ProDrums bot: combo_score 217,455, Hydra 217,455. Window Map bot: 18,450 and 18,450. Plain Drums bot: 185,700, which is Hydra's 192,150 less the 6,450 that Hydra's plain-Drums run prices for dynamics (SC-15). Hydra's figures are the `hydra_replay score` outputs beside the bot files.
Game code (6142): `#8288+0xC8` (the Combo share, SC-08); the writer of combo_score is not traced
Hydra: `category_scores` in src/core/scoring.cpp line 26
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-08 Star Power's share of a hit

Rule: Outside Star Power a hit's combo share is (multiplier minus 1) times its points. Inside Star Power the stored multiplier is doubled, so the game halves it first: the combo share is (half minus 1) times points and the Star Power share is half times points, one extra copy of the points at the normal multiplier.
Label: Proven in the game's code
Evidence: Lines 45 to 50 (outside) and 62 to 73 (inside) of the adder, writing +0xC8 (combo) and +0xCC (Star Power). Hydra's split gives the same numbers for a combo multiplier c: (c minus 1) times points and c times points. So at a 4x combo inside Star Power a plain gem's Star Power share is 4 x 50 = 200 points and a cymbal's is 4 x 65 = 260 (SC-01, SC-02); the August 2026 video score work used the same two values. Run 1's whole-run sp_score 83,260 is not compared with Hydra here.
Game code (6142): RVA `0x20F6EB0`, `code/6142/add_note_score_20F6EB0.asm` lines 45 to 50 and 62 to 73
Hydra: `category_scores` in src/core/scoring.cpp line 26
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-09 A fourth running total moves on two note flags and is written as squeezed_score

Rule: The adder also changes a fourth total at +0xD0. Outside Star Power it takes away (multiplier minus 1) times the points when bit 0x20 of note +0x84 is set, and inside Star Power it adds half the multiplier times the points, the Star Power share that note earned, when bit 0x10 is set. The two setters are called only from the base hit routine, before the note is scored; two direct `or` sites elsewhere were not traced. The base hit routine sets bit 0x10 on a note hit during Star Power whose time is at or before the activation time or after the end time, and bit 0x20 on a note hit with Star Power off within the half-window of the last end. The total is what the stats file writes as squeezed_score. That this total and these bits are about squeezing is read from the names the game gives the file's keys and the dump gives the bits, not from what the code does with them.
Label: Proven in the game's code
Evidence: The adder `code/6142/add_note_score_20F6EB0.asm` lines 45 to 54 (outside Star Power: the combo share at line 50, the getter `0x214BBA0` at line 51, the take-away at line 54) and 62 to 80 (inside: the halved multiplier at lines 62 to 66, the getter `0x214B600` at line 75, the add at line 79). The two getters test bits 0x20 and 0x10 of note +0x84 (`docs/audit/note-shuffle/scout-flag/note_getters.txt`). The setters `0x214B6E0` (bit 0x10) and `0x214B710` (bit 0x20) are called only in the base hit routine `0x20F63F0` (no direct caller found elsewhere, byte scan of il2cpp and .text; a scan for each setter's mask constant found each mask once): `docs/audit/drum-lanes/play/f20f63f0_base_hit.asm` lines 78 to 103 set bit 0x10 and count `+0xE8`, and lines 104 to 119 set bit 0x20 and count `+0xEC` (page 05, SP-36). The drum hit handler calls the base hit before it scores the note (`code/6142/note_hit_dynamics_20DE770.md` lines 13 and 14). The total reaches the file in three copies: `code/6142/engine_stats_20F3DC0.asm` lines 281 and 282 copy `+0xD0` to the player's `+0x3C`, `code/6142/stats_fill_112490.asm` lines 125 and 126 copy that to the record's `+0x50`, and `code/6142/score_submit.asm` lines 266 to 270 write it under the key squeezed_score. The dump's enum for the word at note +0x84 (#8590, `dump.cs` lines 548224 to 548234; the field's type at line 548388) names 16 `WasSqueezed` and 32 `WasSqueezeLoss`; enum values are not checked. Not traced: an `or ax, 0x10` in `0x20F0520` and an `or ax, 0x20` in `0x20DE070`. Each builds a 16-bit word beside bits 0x8 and 0x40, and where that word is stored was not traced, so either may set the same word on another route or touch another field. Two live runs of CHON - Petal (B), 2026-11-22, give this total to the point, from the notes page 05 SP-36 lists for each run. A kick or pad is 50 points and a cymbal 65; the multiplier is 8 in Star Power and 4 after it. LS-P2 run 2 (`C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-p2-run2\scorestats-after-run.json`) wrote 975: its six bit-0x10 notes hold 330 points, giving 330 x 4 = 1,320, less 345. Run 3 (`...\ls-p2-run3\scorestats-CHON_-_Petal_B-20261122-132657.json`) wrote 55: its two bit-0x10 notes hold 100 points, giving 400, less 345. The 345 is the same in both runs: chord 730's kick (3 x 50 = 150) and chord 731's cymbal (3 x 65 = 195), the two notes each file counts in squeeze_note_losses. Both files have score 442,735, so the total moves with sub-millisecond press timing and leaves the score alone (SC-31).
Game code (6142): RVA `0x20F6EB0`, `code/6142/add_note_score_20F6EB0.asm` lines 45 to 80; the base hit routine, RVA `0x20F63F0`, `docs/audit/drum-lanes/play/f20f63f0_base_hit.asm` lines 78 to 119; the stats copies and writer, listings as above
Hydra: no owner: a case-blind grep of `src/` for "squeezed_score", "squeezed points", "squeezed_points" and "squeeze_points" finds nothing. Hydra keeps no running total of squeezed points; its own squeeze tests are compared in page 05, SP-36.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build); the writers 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079; the live totals 2026-11-22, session 6ad668f7 (LS-P2 runs 2 and 3)

### SC-10 Hit order inside a chord at a multiplier step

Rule: When a chord crosses a multiplier step, each gem may pay at the multiplier of its own position, so the order of the hits changes the score. Hydra prices the best order (cheapest gem first).
Label: Hydra assumes it, never checked
Evidence: `multsqueeze_gain` in src/core/scoring.cpp line 134. The 15-point "hit-order loss" in the report on the video The Violation FC 2x60 (YouTube id 2MqPDMH4-MY) was declared by hand to explain a residual, not detected; the report file was not kept. The game's order rule was not read.
Game code (6142): none read
Hydra: `multsqueeze_gain` in src/core/scoring.cpp line 134
Ruling: card 56ac5b71: "in both cases you have to hit the kick first and cymbals are interchangeable so t doesnt matter which is hit first" (01:54 UTC; the advice wording only)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-38 Gems pressed in one frame are scored in the game's order, not Hydra's

Rule: When every gem of a chord is pressed in the same frame, the game scores them in its own order, which put a green cymbal before a kick and a yellow cymbal before a green cymbal. At a multiplier step this moved the step onto the kick, and on an activation chord the gems scored before the activating gem stayed single. Hydra's top path assumes the player chooses the order (SC-10), so a one-frame chord can fall short of it. Whether the order is a fixed lane order, and which gem activates, was not read in the code.
Label: Proven in a live run
Evidence: LS-R1b run 1 on Mouths Like Sidewinder Missiles, 2026-11-07, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run1\press-1.csv` against `refine-prep\mouths-top-score.json` (record in `replies.txt` there). Chord 4 (kick, green cymbal, multiplier 1 to 2) stepped 165, green at x1 plus kick at x2, where Hydra prices 180. Chords 340, 704 and 1208 (kick, yellow cymbal, green cymbal, each an activation) stepped 1,180, yellow single plus green and kick doubled, where Hydra prices 1,440. Chord 927 (kick, green cymbal, an activation) stepped 920, both doubled, as Hydra prices. Together 795 points, the run's whole shortfall (551,975 against 552,770). The rerun, LS-R1 run 2 on the same chart (2026-11-07, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run2\scorestats-The_Fall_of_Troy_-_Mouths_Like_Sidewinder_Missiles_highfine-20261107-205346.json`), scored 552,770, equal to Hydra, closing the 795-point gap.
Game code (6142): not looked for
Hydra: `multsqueeze_gain` in src/core/scoring.cpp line 134 prices the order; the score file's per-gem `multiplier` and `sp_points` carry it
Ruling: none needed: card 56ac5b71 rules the player picks the order ("you have to hit the kick first"); the open point is the auto-player's press order
Found: 2026-11-07, session f3d8ce9f; the rerun, 2026-11-08, session 77057df2 (docs-gaps search)

### SC-39 A press judged before its gem's window opens is an excess hit and breaks the combo

Rule: A pad press the game judges just before the window of the gem it was meant for has opened does not hit that gem. The game counts it as an excess hit, and the combo breaks, so the multiplier falls to 1. On Petal this happened on an activation chord, so Star Power was not activated there; the gem itself still scored at 1x as its window closed, which fits FL-31's grant.
Label: Proven in a live run
Evidence: LS-P2 run 1, 2026-11-22, on CHON - Petal (B), Pro Drums, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-p2-run1\`. The schedule `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-p2-prep\schedule.json` gives chord 692 (a kick and a blue gem, Hydra's third activation) two press groups, each with `window_from_ms` 114,490.50. In `press-1.csv` the blue gem's group was judged at 114,490.03 ms (`judged_ms`), 0.47 ms before that, and the kick's group at 114,490.94 ms. In `sp_watch-1.csv` the score rises by 50, from 272,275 to 272,325, at clock 114,492.95 ms, and in the same row the multiplier (`sp_mult_byte`) falls from 4 to 1 while Star Power stays off; the next rise, of 65 with the multiplier still 1, comes at 114,662.07 ms, just after the window's close at 114,661.93 ms. `scorestats-CHON_-_Petal_B-20261122-110230.json` there reads `excess_hits` 1 and `max_streak` 881, and 881 is the number of gems before chord 692 in the schedule. LS-P2 run 2, with the same chord's blue group judged at 114,490.96 ms, inside the window, reads `excess_hits` 0 and `max_streak` 1,390 (`scorestats-after-run.json` in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-22\ls-p2-run2\`). The stats file's `excess_hits` is the engine's overhit count (SC-37; TM-46 has the overhit path).
Game code (6142): not read for this case; the overhit path is TM-46's, the press handler RVA `0x20DEAA0`, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 201 to 209.
Hydra: no owner: Hydra's paths press every gem inside its window. The press bands that keep a press inside it come from tools/replay_schedule.cpp.
Ruling: none needed: no Hydra rule depends on it
Found: 2026-11-22, session 1390758e (LS-P2 runs 1 and 2).

### SC-11 Whether the combo carries into the next song of a setlist

Rule: In album or setlist play the combo may carry from one song to the next, so every note can pay 4x from the first. Hydra starts the combo at 0 on each chart.
Label: Unknown
Evidence: not checked in the game: only card 2059d526 says "the album run starts with a combo over 12,000"; its video report was not opened. Public sources were searched on 2026-11-11 and say nothing either way: two web searches (one on a setlist combo or streak carrying to the next song, one on the combo resetting between setlist songs); the official wiki's "How to Play" page (wiki.clonehero.net), which covers the combo only within a song and says of setlists only that the song menu can "view setlist options"; its "Dictionary" page, which defines a full combo as every note "in a song"; and the v1.1.0.6135 release page and changelog (clonehero.net), which say nothing about setlists or the combo. Reddit was not searched, and no Reddit thread came up in the web results. The search is in `docs/audit/2026-11-08-clone-hero-unknowns/W10b.md`, "SC-11". No game code was read and no setlist was played.
Game code (6142): none read
Hydra: `chord_score_table` in src/core/scoring.cpp line 99 (one combo counter per chart)
Ruling: none yet: the game's rule is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build); the public-source search 2026-11-11, session 428cf079

## Accents and ghosts

### SC-12 An accent or ghost pays double only when its DynamicsHit flag is set

Rule: An accent or ghost gem pays double only when the hit handler has set the note's DynamicsHit flag (value 4 in the word at note +0x84). The handler sets it when the hit's velocity falls on the accent or ghost side (SC-13); this page calls such a hit a performed dynamic. Otherwise the gem pays its plain value.
Label: Proven in the game's code
Evidence: The drum score method calls the flag test and doubles at lines 19 to 24, then tail-calls the adder at line 31. The test (RVA `0x214BB40`) is `test byte [note+0x84], 4`, and the hit handler's setter (RVA `0x214B740`) sets that same flag; the reviewer of this page read both with tools/ch_mcp/chdis.py on the 6142 DLL, 2026-10-30, and neither is a committed listing. The name DynamicsHit is the dump's: the note's word at +0x84 has the enum type #8590 (`dump.cs` line 548388), and that enum names 4 `DynamicsHit` (line 548231; enum values are not checked).
Game code (6142): RVA `0x20DEEC0`, `code/6142/note_score_add_20DEEC0.asm` lines 19 to 31
Hydra: no owner for the DynamicsHit flag: Hydra pays every accent and ghost as performed (src/core/scoring.cpp)
Ruling: open split: see questions page Q-51
Found: 2026-10-30, session b02ab3e6 (the reference build); the enum name 2026-11-11, session 428cf079

### SC-13 The performed mark compares a lane value with a threshold

Rule: The hit handler picks a signed byte per lane and compares it with an engine threshold. A ghost is marked when the byte is at least 0 and below the threshold; an accent is marked when it is at or above it. A negative byte marks neither. Each mark also bumps a counter: +0xD8 for a ghost and +0xDC for an accent. The byte is the hit's velocity, so a hard hit is the accent side. It comes from a helper at RVA 0x337E10 (no dump line names it), which gives -1 unless the player is on a registered MIDI kit or holds custom controller id 6.
Label: Proven in the game's code
Evidence: Lines 29 to 61 pick the lane byte (one byte each at +0x309 to +0x310); lines 72 to 83 are the path for the flag the getter `0x214BE10` tests (note +0x7C bit 0x80000, `DrumsAccentHit` at dump.cs line 548216): mark and bump +0xDC, the accent counter. Lines 91 to 102 are the path for `0x214B6C0` (bit 0x40000, `DrumsGhostHit`, dump.cs line 548215): mark and bump +0xD8, the ghost counter. Both getters were read in the 6142 DLL with tools/ch_mcp/chdis.py on 2026-10-31 (each is one `test dword ptr [rcx + 0x7c]` against that bit). ghidra_decompile_all.txt lines 1163 to 1181 read the same. `code/6142/lane_velocity_337E10.asm` shows what fills the bytes: round((GetAxis - 0.5) * 2 * 127) on a pressed lane that passes the gate, else -1. The MIDI handler turns velocity v into axis 0.5 + v / 254 (RVA `0x21E420`), and the register method makes the controller from definition 1 (RVA `0x21CDE0`). The bot writes 127 for an accent and the threshold minus 1 for a ghost (RVA `0x33A880` is the live input; the bot branch sits near 0x33B64F). The threshold read 100 live, with the song playing, in LS-O1b of `docs/superpowers/plans/2026-11-06-optimal-score-run.md` (removed; `git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md`) on 2026-11-06 (`replies.txt` in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-o1-run1\`), and the same run's player read 100 again. That is one chart and one player. The drum engine's constructor writes the field from the high half of the 8 bytes at +0x18 of its set-up argument (`code/6142/engine_ctor_20DF680.asm` lines 51 to 53, page 07 IN-90); where the argument gets that value was not traced. In that run the gate passed with the kit on custom controller id 6 and no MIDI kit: see SC-14.
Game code (6142): RVA `0x20DE770` (slot 11), `code/6142/note_hit_dynamics_20DE770.asm` lines 29 to 102; threshold `#8255+0x2C0`
Hydra: no owner: Hydra has no velocity or threshold
Ruling: none needed for the comparison; the paid-or-not question is Q-51
Found: 2026-10-30, session b02ab3e6 (the reference build); what writes the bytes, 2026-11-06, session 70bfa33d; the threshold's live value, 2026-11-06, session f1fabefd

### SC-14 A keyboard press never earns accent or ghost credit

Rule: On a key-press run the game counted no accent or ghost as hit, while the bot on the same chart hit them all.
Label: Proven in a live run
Evidence: Run 1 (keys, ProDrums): accent_notes_hit 0 of 9, ghost_notes_hit 0 of 34. ProDrums bot on the same chart: 9 of 9 and 34 of 34 (scorestats-Trippin_SP_Spike-20261028-212754.json). One key run only; the code does not show where a key press gets its lane value. A virtual kit press at full value earns none either: LS-D2d of `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)`, 2026-11-06, hit 2,985 of 2,996 notes on the Senri chart with accent_notes_hit 0 of 368 and ghost_notes_hit 0 of 726 (stats copy in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\`). A rebuild of that run with every gem at its plain value matches the game's running score from the watch CSV on all but two of the 2,342 chords played, so the 1,094 dynamic gems earned nothing extra; Hydra's pricing of them is worth 320,415 points on that chart with those activations. The cause is the gate in SC-13: the kit is controller 0 and no MIDI kit is registered, so every hit carried -1. A virtual kit on custom controller id 6, pressing each gem at its own strength, does earn the credit: LS-O1d of `docs/superpowers/plans/2026-11-06-optimal-score-run.md` (removed; `git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md`), 2026-11-06, pressed chords 62 to 2348 of the Senri chart, which hold 2,925 gems, 354 accents and 726 ghosts, and the stats file reads notes_hit 2,925, accent_notes_hit 354 and ghost_notes_hit 726 (stats copy in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-o1-run1\`). So the limit is the gate, not the virtual kit.
Game code (6142): none beyond SC-13
Hydra: no owner: Hydra scores a dynamic gem as always played right
Ruling: open split: see questions page Q-51 (the user said in session 6a549de0: "the code is the source of truth but from experience i can tell you dynamics only work with a real drum kit. keyboard inputs never register dynamics."; that states the game's behaviour, not what Hydra should price)
Found: 2026-10-30, session b02ab3e6 (the reference build); the kit press, 2026-11-06, session 70bfa33d (LS-D2d); the kit at id 6, 2026-11-06, session f1fabefd (LS-O1d)

### SC-15 Plain Drums drops accents and ghosts

Rule: On plain Drums (instrument 6) the game clears the accent and ghost bits of every note in the chart builder, so no dynamics score. On ProDrums (instrument 9) they stay.
Label: Proven in the game's code
Evidence: The chart builder compares the instrument with 9 and otherwise calls `0x20F16E0` then `0x20EFEE0` (docs/audit/note-shuffle/step0/open2/f20d2ea0_build.asm lines 245 to 253), which clears the bits (docs/audit/note-shuffle/step0/open2-refute/f20EFEE0.asm line 25). Live: on Trippin SP Spike the plain-Drums bot scored 248,550 against Hydra's 257,150, a gap of 8,600 = 43 dynamic gems times 50 times 4x.
Game code (6142): chart builder RVA `0x20D2EA0`; clearing RVA `0x20EFEE0`
Hydra: no owner: Hydra keeps dynamics on plain Drums
Ruling: the user, card 6a549de0: "there is no way this is intentional, it must be a bug in clone hero" (01:30 UTC) and "Keep Hydra as is" (01:44 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-16 An unperformed accent or ghost may stop scoring under one flag

Rule: When bit 0x10 of the engine flags word is set and an accent or ghost was not performed, the hit handler returns before it scores the note or steps the streak. The bit was clear in a human Quickplay run. What sets it is not known.
Label: Proven in a live run
Evidence: LS-O1b of `docs/superpowers/plans/2026-11-06-optimal-score-run.md` (removed; `git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md`), 2026-11-06: `ch_snapshot` read the flags word as 0x2 with bit 0x10 clear, with ProDrums Expert playing on the virtual kit (`replies.txt` in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-o1-run1\`). One read, early in one song. Lines 84 to 90 and 103 to 104 test the flags word at +0x198 for 0x10 and return. The setter was not searched; tools/ch_probe/constants.py names only bit 0x1000 of `OFF_FLAGS`.
Game code (6142): RVA `0x20DE770`, `code/6142/note_hit_dynamics_20DE770.asm` lines 84 to 90; `#8288+0x198`
Hydra: no owner: Hydra scores every note
Ruling: none yet: the trigger is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build); the live read, 2026-11-06, session f1fabefd

### SC-17 An unperformed dynamic raises an event and nothing else

Rule: When an accent or ghost is hit without its mark, the handler calls a method that only sends event 4 to the engine's listener; it changes no score.
Label: Proven in the game's code
Evidence: Lines 107 to 117 of the hit handler; ghidra_decompile_all.txt lines 53145 to 53157 show the callee does nothing else.
Game code (6142): RVA `0x20DE770` lines 107 to 117; callee RVA `0x20F5FF0`
Hydra: no owner
Ruling: none needed: no score effect
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-18 A ghost kick pays double

Rule: A ghost on the kick pays double like any ghost.
Label: Proven in a live run
Evidence: docs/adr/0012-kick-dynamics-are-priced.md line 23: "Ghost kicks are proven by video on this chart." (the Who chart; 36 ghost kicks explain 1,142,235 against 1,143,035). The video is Won't Get Fooled Again Pro Drums FC (YouTube id T_HRmf0PlQQ), listed in C:\Users\Patrick\Downloads\Hydra\video-tools\refs.json; not re-run for this page.
Game code (6142): none
Hydra: `ChordNote::basescore` in src/core/model.cpp line 84
Ruling: card 528e137d: "velocity 1 is a ghost, 127 is an accent, on both the normal kick and the 2x kick pitch" (02:58 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-19 An accent kick pays double

Rule: An accent on the kick pays double like any accent.
Label: Proven in a live run
Evidence: LS-M0 run 1 on I Am Ahab, 2026-11-08, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m0-run1\scorestats-Mastodon_-_Leviathan_-_02_-_I_Am_Ahab-20261108-121738.json`: every note hit, accent_notes_hit 166 of 166, score 525,555 and base_score 132,295. Hydra's score file for the same chart, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\kick-prep\ahab\Mastodon - Leviathan - 02 - I Am Ahab-score.json`, gives a final total of 525,555 and a final accent total of 8,300 (166 times 50), and 33 of its 166 accents are on the kick. The game's score and base equal Hydra's, which pays every one of those kick accents double, so the game does too. One chart only. ADR 0012 lines 26 to 30 still say "Accent kicks are assumed, not proven"; that line predates this run.
Game code (6142): none read; the scorer doubles any note with the performed flag and has no kick test (SC-12, `code/6142/note_score_add_20DEEC0.asm` lines 19 to 24)
Hydra: `ChordNote::basescore` in src/core/model.cpp line 84
Ruling: card 528e137d: "The symmetric rule is the sensible default, but it needs a video of a chart with velocity-127 kicks before we trust it." (02:58 UTC); the run above stands in for that video
Found: 2026-10-30, session b02ab3e6 (the reference build); the live run, 2026-11-08, session 77057df2 (docs-gaps search)

## The solo bonus

### SC-20 The solo bonus at a full solo is 100 per note

Rule: Each solo is scored on its own, and the file's solo_bonus_total is the sum over the chart's solos. A solo of N notes with h of them hit pays nothing when h is below half of N. Otherwise it pays 10 times round(((h / N) minus 0.5) times 2 times h times 10), which is 100 per note when every note is hit. The bonus has no multiplier and no Star Power. Not yet seen in a run: the case of exactly half, the rounding of a value ending in .5 (the earlier reading said half to even, in 64-bit float), and whether the bonus is paid at the solo-end note even when that note is missed.
Label: Proven in a live run
Evidence: four virtual kit runs, their scorestats files in the run folders named here. The Ladies Talk chart (`C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check\notes.mid`, MD5 9569049A5F3100D1EB759359E6EAAEF5, equal to the `checksum` in all three of its runs' files, resolution 480) has two solo markers on note 103 of PART DRUMS: ticks 5,760 to 13,440 holding 83 gems, and ticks 140,400 to 186,480 holding 497 gems, each counted from the start tick up to but not including the end tick (read with a short Python MIDI reader on 2026-11-08). LS-O3 run 1 (2026-11-07, ls-o3-run1) hit every note: 8,300 plus 49,700 is its 58,000. LS-D2d run 1 (2026-11-06, ls-d2d-run1) missed 11 notes, all in the Intro section (section_stats Intro 76 of 87, every other section full): 76 of 83 gives 6,320, and 6,320 plus 49,700 is its 56,020; h = 75 would give 6,050 and h = 77 would give 6,590, so 76 is the only fit. LS-O1 run 1 (2026-11-06, ls-o1-run1) hit 16 of the Intro's 87, so at most 16 of the first solo's 83, under half: 0 plus 49,700 is its 49,700. LS-M2 run 1 on Won't Get Fooled Again (2026-11-08, ls-m2-run1; chart `C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\Onyxite\The Who\The Who - Won't Get Fooled Again (O)\notes.mid`, MD5 AF44D5C8D941BB33444D18E7871CDC77, equal to its file's `checksum`) has one solo of 121 gems, ticks 491,520 to 505,920, and its Drum solo section reads 121 of 121: 12,100. No press CSV was kept for LS-D2d or LS-O1, so their misses were not checked note by note. The formula was first read 2026-10-16 at the game's solo-end method, sessions 1aaf0ac4 and 5b35957b; that listing is lost and no address was recorded. The reference build looked in `SoloCounter` (TypeDefIndex 123; its methods hold only "Solo Move" and "frets"), its fields and every string literal containing "solo", and did not find the function. Public `3b49012c:hydra/hypath.py` line 65 pays 100 * timestamp.chord.count().
Game code (6142): not found: looked in `SoloCounter` methods `0x293A30`, `0x294070`, `0x294880`, `0x2946F0`, `0x295190` and `0x2956C0`
Hydra: `solo_bonus` in src/core/scoring.cpp line 95 with `kSoloBonusPerNote` in src/core/model.h line 147 (full solos only, since Hydra's paths hit every note)
Ruling: D251: "Like the game"
Found: 2026-10-16, session 5b35957b; the four runs, 2026-11-08, session 77057df2 (docs-gaps search)

### SC-21 The solo's note count counts every gem

Rule: The game counts every gem of a chord in a solo's note total, the same as Hydra, so a full solo pays 100 per gem, not per chord. With the DoubleKick modifier on, a 2x kick gem counts too. Same-tick gems are not merged into one note.
Label: Proven in a live run
Evidence: the runs of SC-20, all with the DoubleKick modifier. On Ladies Talk the two solos hold 83 and 497 gems but 74 and 389 chords, and 458 of the second solo's gems are left once the 2x kick pitch (95) is dropped. LS-O3 run 1 hit every note and its solo_bonus_total is 58,000, which is 100 times the 580 gems; the chord count would give 46,300, and leaving out the 2x kick would give 54,100. On Won't Get Fooled Again the solo holds 121 gems in 78 chords, and LS-M2 run 1's solo_bonus_total is 12,100. Session 1aaf0ac4 (2026-10-16) had first written that every gem counts; a later agent in that session reported that the game merges same-tick gems (card docs/sessions/2026-10-16-1aaf0ac4.md, open threads, not confirmed). These runs settle it against the merge.
Game code (6142): none
Hydra: `solo_note_count` in src/core/scoring.cpp line 91
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build); the runs, 2026-11-08, session 77057df2 (docs-gaps search)

### SC-22 The bonus is paid as one lump at the solo's end

Rule: The game pays the solo bonus in one lump when the solo ends, so a score curve trails the per-note total during a solo.
Label: Proven in a live run
Evidence: the video Swim to the Moon Pro Drums FC (YouTube id OoE7PMv-ekk), 2026-08-17 (card e61236dc); C:\Users\Patrick\Downloads\Hydra\video-tools\README.md lines 457 and 458 record it. The result file was not kept.
Game code (6142): none
Hydra: `chord_score_table` in src/core/scoring.cpp line 99 books it per chord (the totals agree)
Ruling: none needed: a display difference only
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-23 The solo box shows a whole percent and a message ladder

Rule: Proven in the game's code: the end message comes from a ladder on a whole percent: exactly 100 gives "Perfect Solo!", 95 or more "Awesome Solo!", 90 or more "Great Solo!", 80 or more "Good Solo!", 70 or more "Solid Solo!", exactly 69 "nice", 60 or more "Okay Solo...", and anything lower "Messy Solo...", which also zeroes the bonus the method stores. "Awesome Choke!" is not on the ladder: it replaces the message whenever a flag passed to the method is set. Still Unknown: the box appears when the player hits the solo's first note, not at the chart's solo marker, and hitting the last note ends the solo. During a solo the game shows trunc(float(h) / float(N) times 100.0f) percent. After the last note it shows a message for 1.5 s and the bonus for 1.5 s. N minus 1 sets the choke flag.
Label: Unknown
Evidence: The ladder: the `SoloCounter` end-message method `0x2960B0` compares its first int with 100, 95, 90, 80, 70, 69 and 60 and picks a text from the method's static array (`code/6142/solo_counter_result_2960B0.asm` lines 21 to 82). Line 40 zeroes its other int on the Messy branch, and line 101 stores that int at `+0x80`. Lines 87 to 99 replace the text with slot 7 when the bool argument is set. The array's ten texts, in slot order, are "Perfect Solo!", "Awesome Solo!", "Great Solo!", "Good Solo!", "Solid Solo!", "Okay Solo...", "Messy Solo...", "Awesome Choke!", soloPointsText and "nice" (`code/6142/solo_counter_message_293CF0.asm` lines 42 to 179). On the online route the zeroed int is the solo bonus: `code/6142/player_solo_remote_2096F0.asm` lines 61 to 73 pass one record field to `0x2960B0` and add it to the engine's `+0xA0`. Whether the box's shown bonus reads `+0x80` was not traced. Every other `SoloCounter` method was disassembled and searched for these compares; only `0x2960B0` has them. The rest is not read. The running percent method `code/6142/solo_counter_percent_294C60.asm` lines 11 to 33 writes an int it is given followed by "%" and does no division, so the formula sits in its callers, the two `BasePlayer` virtual methods `0x2085D0` (slot 26) and `0x2086C0` (slot 49). Neither has a direct caller (byte scan of il2cpp and .text), and a scan for calls through slots 26 and 49 in `BasePlayer` code and the engine's range found none; calls from a subclass, from `BasePlayer<TInputs>` or in another register form were not searched. The constructor makes a 1.5 s and a 0.032 s wait (`code/6142/solo_counter_ctor_2962C0.asm` lines 13 to 30), which fits the 1.5 s, but which coroutine uses each was not traced. What sets the choke flag was not reached. The first-note start and the percent formula come from the 2026-10-16 reads of sessions 1aaf0ac4 and 5b35957b, whose listing was lost: D251's record gives the percent formula, the message and bonus times of 1.5 s each, and "Perfect Solo!" at 100%, and card 1aaf0ac4 (docs/sessions/2026-10-16-1aaf0ac4.md) says the session checked the message list and percent ladder against the game's disassembly. Hydra's own box was checked in the real app (card 5b35957b, docs/sessions/2026-10-16-5b35957b.md: 44% = 53 / 119), which checks Hydra, not the game.
Game code (6142): the `SoloCounter` end-message method, RVA `0x2960B0`, `code/6142/solo_counter_result_2960B0.asm` lines 21 to 101; the text array method, RVA `0x293CF0`; the percent method, RVA `0x294C60`; the percent formula and the start: not found (looked in every `SoloCounter` method and the three `BasePlayer` callers `0x2085D0`, `0x2086C0` and `0x2096F0`)
Hydra: `solo_percent` in src/app/preview_view.cpp; `kSoloResultMs` in src/app/preview_view.h (Hydra shows the percent during the solo, then "Perfect Solo!", then the bonus; no other message, since a Hydra path hits every note)
Ruling: D251: "Game's look + count" and "Like the game"
Found: 2026-10-30, session b02ab3e6 (the reference build); the ladder 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079

## Star cutoffs

### SC-24 The star table is nine floats

Rule: The game's star table holds nine 32-bit floats: 0.1, 0.5, 1.0, 2.0, 2.8, 3.6, 4.4, 5.2 and 6.0. A static constructor copies them from the metadata into a static array.
Label: Proven in the game's code
Evidence: The constructor builds an array of 9 and fills it at lines 12 to 22. dump.cs line 541526 names the data's metadata offset 0x891AC0, and the nine floats occur once in the game's `global-metadata.dat` (under `Clone Hero_Data\il2cpp_data\Metadata\` in the 1.1 install), at that offset (a byte search, 2026-10-31).
Game code (6142): RVA `0x214C320` (class #8611 static constructor), `code/6142/star_table_cctor_214C320.asm` lines 12 to 22
Hydra: `kStarMultipliers` in src/core/stars.h line 25 (the first seven)
Ruling: none needed: same values
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-25 Stars count the cutoffs the score reaches, up to 7

Rule: The star count is the number of table slots 0 to 6 whose cutoff the score reaches, counted from slot 0 and stopping at the first miss. The cutoff of slot i is the base score as a 32-bit float times slot i, rounded and cut to a whole number. Slots 7 and 8 are never read.
Label: Proven in the game's code
Evidence: Lines 15 (count starts at 0), 30 (table load), 35 to 37 (float multiply), 46 to 48 (rounding call and cut), 49 and 50 (stop at the first miss), 51 to 53 (stop at 7), 55 (return). Whether the rounding helper 0x183031130 rounds up is SC-26.
Game code (6142): RVA `0x210DD00` (class #8351), `code/6142/star_count_210DD00.asm` lines 15 to 55
Hydra: `star_cutoff`, `kMaxStars` and `stars_for_score` in src/core/stars.h lines 29, 21 and 47
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-26 The cutoff rounds up

Rule: The rounding after the float multiply is Math.Ceiling. Drum bases are always multiples of 5. For slots 2 to 6, a multiple-of-5 base gives a whole product, so Ceiling changes nothing. For slots 0 and 1 (0.1 and 0.5), a base whose fifth is odd gives a product ending in .5, and Ceiling rounds it up: a base of 15 gives 1.5 and 7.5, so the cutoffs are 2 and 8.
Label: Proven in the game's code
Evidence: The star counter widens the 32-bit product to a double (`code/6142/star_count_210DD00.asm` line 46), calls the helper 0x3031130 (line 47) and cuts the result to a 32-bit int (line 48); lines 39 to 45 set up the System.Math class just before the call. The helper is Math.Ceiling by its address: System.Math.Ceiling at `0x1B518E0` is a single jump to it (`code/6142/math_ceiling_1B518E0.asm` line 1; `dump.cs` line 90193). Both of the helper's routes round up. It tests a stored CPU-feature value and picks one (`code/6142/ceil_helper_3031130.asm` lines 1 to 3). The fast route is one `roundsd xmm0, xmm0, 0xa`, and mode 0xA rounds toward plus infinity (`code/6142/ceil_sse41_304BA10.asm` line 1). The software route also returns the ceiling (`code/6142/ceil_soft_3031150.asm`): it cuts off the fraction bits (lines 30 to 37) and adds 1.0 when the number is positive and the cut changed it (lines 38 to 42); a positive number below 1 gives 1.0 (lines 19 to 29), and a finite number of 2 to the 52 or more comes back as it is (lines 12 to 16). An example of where 32-bit floats matter: a base of 786,437 times the 3.6 slot is 2,831,173.0 in 32-bit floats, so 6 stars need 2,831,173 points, where exact arithmetic would need 2,831,174 (worked out with Python's 32-bit float packing on 2026-10-31 from SC-24 and SC-25). Multiples of 5 follow from 50, 65, 100 and 130 (SC-01, SC-02).
Game code (6142): the rounding helper, RVA 0x3031130, and its two routes, RVAs 0x304BA10 and 0x3031150, listings as above; System.Math.Ceiling, RVA `0x1B518E0`; the call in `code/6142/star_count_210DD00.asm` lines 46 to 48
Hydra: `star_cutoff` in src/core/stars.cpp lines 9 to 15 takes the same steps: a 32-bit float product, widened to a double and rounded up. It then keeps a 64-bit int where the game cuts to a 32-bit int; the two part only above 2,147,483,647 points, far beyond any drum chart's cutoff.
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build); the listings 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079

### SC-27 The star base is worked out once, when the engine is built

Rule: The drums star base is the sum over notes of (50, plus 15 for a cymbal) times the gem count, times 2 when the note carries either dynamic flag (once, even with both). There is no sustain term for drums. It runs when the instrument is Drums (6) or ProDrums (9), after the Note Shuffle, and is stored at +0x1D0. Other instruments take a sustain term instead: every note whose length is at least a third of a chart value (read as the resolution) adds its length in ticks, and the total is turned into points once, at 25 points per beat, rounded by the same helper as SC-26.
Label: Proven in the game's code
Evidence: Star base lines 68 (drums branch), 87 to 101 (cymbal and the two flag tests), 103 to 118 (gem count) and 120 to 123 (the sum); lines 68 to 80 and 147 to 149 leave the sustain term at 0. The non-drums term is lines 29 to 39 (the value divided by 3.0), 73 to 82 (the length test) and 139 to 149 (the total divided by the value over 25.0); that the value at chart record +0xB8 is the resolution is session 9168b711's reading (card line 24 says "sustains at 25 points per beat"), not shown by name in the dump. The constructor picks the drums branch for 6 or 9 at lines 183 to 189, calls it at line 196 and stores the result at line 202. That the two flags are accent and ghost is named by the dump: 0x40000 is `DrumsGhostHit` and 0x80000 is `DrumsAccentHit` (dump.cs lines 548215 and 548216). Live: the game's base_score is 73,520 on Trippin SP Spike ProDrums, 62,850 on its plain Drums run (1,257 gems times 50, no cymbals or dynamics) and 7,100 on Window Map (142 gems times 50). Hydra's totals for the same charts are in `trippin.json`, `trippin-normal.json` and `window-map.json`, one folder above the bot files. On ProDrums Hydra's notes, accent and ghost totals sum to the game's base (71,370 + 450 + 1,700 = 73,520). On Window Map Hydra's notes total is 7,100 with no dynamics. On plain Drums the game's 62,850 equals Hydra's notes total only, because Hydra still prices 450 + 1,700 for dynamics there and the game drops them (SC-15).
Game code (6142): RVA `0x20F4680`, `code/6142/star_base_20F4680.asm` lines 68 to 149; constructor RVA `0x20F79E0`, `code/6142/engine_ctor_20F79E0.asm` lines 183 to 202
Hydra: `Path::chart_base_score` in src/core/model.cpp line 839
Ruling: none needed: same rule (a Hydra note holds one dynamic type, so the both-flags case does not arise)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-28 The game's stars agree with Hydra's at the 3.6 and 4.4 edges

Rule: On the saved runs the game's star count agrees with Hydra's table at the 3.6 and 4.4 cutoffs.
Label: Proven in a live run
Evidence: Window Map 25,550 against 3.6 times 7,100 = 25,560: 5 stars. Trippin ProDrums 290,975 between 264,672 and 323,488: 6 stars. Run 1 365,095 above 323,488: 7 stars. Plain Drums 248,550 between 226,260 and 276,540: 6 stars. The lower cutoffs are at no edge in any run.
Game code (6142): RVA `0x210DD00` (SC-25)
Hydra: `star_cutoff` in src/core/stars.cpp line 9
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-29 The star count leaves out the solo bonus and the clean-play bonus

Rule: The game counts stars on the score before the solo bonus (engine +0xA0) is added, and before the clean-play bonus (engine +0xA4) is added too. Hydra compares a score without the solo bonus with the cutoffs, so the two agree whenever the clean-play bonus is 0. That bonus has no traced writer, read 0 in every saved stats file (SC-37), and has no Hydra counterpart.
Label: Proven in the game's code
Evidence: The star counter `0x210DD00` has four direct callers, all in #8351 (byte scan of il2cpp and .text), and all four take the same three steps. In `code/6142/band_stars_210E2A0.asm`, lines 111 to 118 add each player's `+0x14`, `+0x10`, `+0x18` and `+0x1C` into the band's `+0x18`, `+0x10`, `+0x14` and `+0x1C`. Lines 171 to 175 then call the counter with the band's `+0x18` as the score and `+0x10` as the star base and keep the count at `+0x20`. Only after that do lines 176 to 181 add `+0x14` and then `+0x1C` into the score. `band_stars_210F140.asm`, `band_stars_21104F0.asm` and `band_stars_2110920.asm` show the same steps (their `.md` files give the lines). Which player field is which comes from the engine and the stats file: `code/6142/engine_stats_20F3DC0.asm` lines 219 and 220 copy the engine score `+0x94` into the player's `+0x14`, lines 234 and 235 the solo bonus `+0xA0` into `+0x18`, and lines 236 and 237 `+0xA4` into `+0x1C`. `code/6142/stats_fill_112490.asm` lines 115 and 116 and 127 and 128 copy `+0x18` and `+0x1C` into the record fields that `code/6142/score_submit.asm` lines 262 and 272 write as solo_bonus_total and clean_play_bonus. The file's score is the getter `0x2130490`, `+0x1C` plus `+0x18` plus `+0x14` (`code/6142/player_total_score_2130490.asm`), so the stars are counted on score less solo_bonus_total less clean_play_bonus. The engine's own star count and the HUD star bar (SC-30) also count on the engine score `+0x94`, without `+0xA0` (`code/6142/engine_stats_20F3DC0.asm` lines 199 to 207; `code/6142/score_manager_sum_1285A0.asm` lines 30 to 34). Where the band count at `+0x20` is shown was not traced. Four saved runs have a solo bonus (SC-20); in none of them does it carry the score across a cutoff (worked from SC-24's table on 2026-11-08).
Game code (6142): the four callers of RVA `0x210DD00` in #8351, RVAs `0x210E2A0`, `0x210F140`, `0x21104F0` and `0x2110920`, `code/6142/band_stars_210E2A0.asm` lines 111 to 181; the engine stats method, RVA `0x20F3DC0`; the total-score getter, RVA `0x2130490`
Hydra: `score_without_solo` in src/core/stars.h line 51; `path_stars` line 59. Hydra has no clean-play bonus.
Ruling: none needed: same rule while the clean-play bonus is 0
Found: 2026-10-30, session b02ab3e6 (the reference build); the solo runs, 2026-11-08, session 77057df2 (docs-gaps search); the four callers 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079

### SC-30 The HUD star bar keeps its own cutoffs

Rule: The HUD star bar rebuilds its own nine cutoffs as ceil(float(base) times slot i) from slot 0 up. Its base is the sum of the engines' star bases; with one engine it copies that engine's own cutoff array. Its star cap is 7.
Label: Proven in the game's code
Evidence: Lines 19 and 20, 38, 54 (adds +0x1D0), 96 to 101, 119 and 120 (the copy from engine +0xF8), 122, 123 and 145. The only direct caller found is `ScoreManager.Update`. Four near-copies (`0x2DF200`, `0x2DFC20`, `0x2E0150`, `0x2E0540`) exist; two start at slot 1, and which copies run in play is unchecked.
Game code (6142): `StarProgress` method RVA `0x2DF7F0`, `code/6142/star_progress_update_2DF7F0.asm` lines 19 to 145
Hydra: `stars_for_score` in src/core/stars.h line 47 (no HUD bar)
Ruling: none needed: the copy that is known to run starts at slot 0
Found: 2026-10-30, session b02ab3e6 (the reference build)

## The score breakdown lines

### SC-31 The score is the sum of the breakdown fields

Rule: In the game's file, score equals note_score plus combo_score plus sp_score plus solo_bonus_total plus 50 for each accent or ghost hit. squeezed_score is not part of the sum.
Label: Proven in a live run
Evidence: ProDrums bot: 71,370 + 217,455 + 0 + 50 x 43 = 290,975. Plain Drums bot: 62,850 + 185,700 = 248,550. Window Map: 7,100 + 18,450 = 25,550. Run 1: 71,235 + 210,600 + 83,260 = 365,095, with squeezed_score 1,385 left out. The sum holds in all 16 scorestats files under the run folder (checked with a short Python script on 2026-11-08). Four of them have a solo bonus, so the solo term is checked: LS-O3 run 1, 168,235 + 665,075 + 283,720 + 58,000 + 50 x 1,094 = 1,229,730. squeezed_score is nonzero in eight of the 16 (from 380 on LS-F1 run 1 to 3,940 on LS-O3 run 1) and is in none of the sums.
Game code (6142): none (the per-player writer is not traced, SC-35)
Hydra: `score_total` in src/core/model.h line 152
Ruling: none needed: same sum
Found: 2026-10-30, session b02ab3e6 (the reference build); the 16 files, 2026-11-08, session 77057df2 (docs-gaps search)

### SC-32 The Notes line is the 1x total less 50 per dynamic hit

Rule: The file's note_score, and the results screen's Notes line, is the plain-points total less 50 for each accent or ghost hit, so a dynamic cymbal's extra 15 stays in Notes. The Accent and Ghost lines are the hits times 50. The runs prove the file's field; that the screen's line shows the same total rests on the lost listing below.
Label: Proven in a live run
Evidence: in every one of the 16 scorestats files under the run folder that hit every note, base_score less note_score is exactly 50 per accent or ghost hit (nine files, checked with a short Python script on 2026-11-08). For example LS-M0 run 1 (2026-11-08): 132,295 less 115,645 is 16,650, which is 50 times its 333 accent and ghost hits; LS-O3 run 1 (2026-11-07): 222,935 less 168,235 is 54,700, 50 times 1,094. The screen's side: listing lost; read 2026-10-11 at RVA `0x20F3DC0` (instructions 0x20F425C to 0x20F426F) and at 0x126FD6 and 0x127029, session 1bae448b. The session's card (docs/sessions/2026-10-12-1bae448b.md) records the finding, the Notes line as the 1x total less 50 per ghost or accent hit; the addresses come from the session's own record, which is not kept, and its listings were swept with a scratch folder. Both of those addresses lie inside the `PlayerStatsBox` method that starts at 0x126280 and ends before 0x1270D0 (its bounds from the 6142 dump's script.json, checked 2026-10-31); the dump names its class `PlayerStatsBox`. The same session read the base value 50 passed by the drum hit handler at 0x20DE96C, inside 0x20DE770 (the same record, not kept). Live: the ProDrums bot's note_score 71,370 = base_score 73,520 less 50 x 43, and equals Hydra's Notes. Run 1 (no dynamic performed) has note_score 71,235, 2,285 below base_score; that would be 43 x 50 plus 9 x 15 if 9 of the dynamic gems are cymbals, which was not checked.
Game code (6142): RVA `0x20F3DC0` (listing lost)
Hydra: `category_scores` in src/core/scoring.cpp line 26
Ruling: D208: "we don't need to change anything, matching the game is correct."
Found: 2026-10-11, session 1bae448b; the 16 files, 2026-11-08, session 77057df2 (docs-gaps search)

### SC-33 The average multiplier is score less the solo bonus, over base score

Rule: The game's avg_multiplier is score less solo_bonus_total, divided by base_score. On a chart with no solo that is score over base_score.
Label: Proven in a live run
Evidence: with no solo: 248,550 / 62,850 = 3.95465394 (file 3.95465398); 25,550 / 7,100 = 3.59859155 (3.59859157); 290,975 / 73,520 = 3.95776659 (3.95776653); 365,095 / 73,520 = 4.96592764 (4.9659276). With a solo, the four runs of SC-20: LS-D2d run 1 (900,870 less 56,020) / 222,935 = 3.78966963 (file 3.78966975, where score over base would be 4.04095364); LS-O1 run 1 (1,198,410 less 49,700) / 222,935 = 5.15266782 (5.152668); LS-O3 run 1 (1,229,730 less 58,000) / 222,935 = 5.25592662 (5.25592661); LS-M2 run 1 (1,063,755 less 12,100) / 214,640 = 4.89962262 (4.89962244). Each agrees to the file's float precision, and the same holds in all 16 files under the run folder (checked with a short Python script on 2026-11-08).
Game code (6142): none (the writer is not traced)
Hydra: `Path::avg_mult` in src/core/model.cpp lines 843 to 847 (total less solo, over the base score)
Ruling: D222: "change it to Average Multiplier, that's a real stat you see in the game"
Found: 2026-10-30, session b02ab3e6 (the reference build); the solo runs, 2026-11-08, session 77057df2 (docs-gaps search)

### SC-34 Off-speed scores are never judged against Hydra's optimum

Rule: Hydra's optimum is for normal speed only, and a score above it is labelled "above optimal", never "impossible", because Hydra's optimum leaves out many backends and the old fill-spawn logic.
Label: Hydra assumes it, never checked
Evidence: src/app/dm_report.cpp lines 128 to 152. The dmleaderboards site (a community leaderboard for drum scores) ranks every speed on one board per chart. The user said so on 2026-10-10 (D206, card 35360d51 line 18: "dmleaderboards does not use separate leaderboards for different speeds."); the public DMBot source and a user screenshot backed it, neither committed. The Clone Hero v1.1 leaderboard announcement (https://clonehero.net/2024/11/10/clonehero-leaderboards.html) says the game's own leaderboards show only scores for the selected modifiers and speed (ch-evidence.md question 222 at commit 5fbe71ae; not re-read for this page).
Game code (6142): none (outside the game)
Hydra: `is_base_speed` in src/net/dmbot_client.h line 37; the report compares with the 100% result whatever the Song speed setting holds (`Settings::profile_at_clone_hero_cap`, src/app/config.cpp line 458)
Ruling: card e3a57d11: "do not label above optimal as impossible." and "just note them as above optimal." (02:24 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

## The scorestats file

### SC-35 The game writes scorestats.json when the results screen starts

Rule: When the end-of-song screen starts, the game builds a stats object, fills it, joins a folder path with the literal file name, writes the serialised text with File.WriteAllText, and logs "Failed to write scorestats.json: " on an error. Every song writes the same file name, and File.WriteAllText replaces whatever the file held, so each song overwrites the last one's file.
Label: Proven in the game's code
Evidence: Listing lines 87 (the literal), 171 to 181 (new object), 207 (fill), 242 (per player), 251 to 270 (folder, Path.Combine, serialiser, write) and 274 to 300 (the error text and Debug.LogError). The overwrite is read from the code only. The bot runs of 2026-10-28 copied the file after each run because the feasibility study (docs/research/2026-10-28-clone-hero-mcp-feasibility.md line 31) expected it to be overwritten; no run checked that an earlier file was gone (IN-37).
Game code (6142): `EndOfSong.Start`, RVA `0x15DC20`, `code/6142/scorestats_writer_15DC20.asm` lines 171 to 300
Hydra: no owner in src/; the tools read it (`SCORESTATS` in tools/ch_mcp/game_files.py; tools/ch_mcp/compare_scorestats.py)
Ruling: none needed: a game file
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-36 One serialiser writes the twelve top-level keys

Rule: The twelve top-level keys are written by one serialiser: game_version, checksum, song_name, artist_name, charter_name, playback_speed, game_mode, score_timestamp, band_score, band_stars, player_count and players.
Label: Proven in the game's code
Evidence: Lines 29 to 60 of the serialiser load the twelve key literals.
Game code (6142): RVA `0x252630` (class #1037), `code/6142/scorestats_json_252630.asm` lines 29 to 60
Hydra: no owner in src/
Ruling: none needed: a game file
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-37 The per-player keys and what writes each

Rule: Each player entry holds 46 keys, one of them a list of sections with 3 keys each. Every key is a string literal in the game, and every run wrote them all. The per-player serialiser `0x1144C0` writes each key from one field of a record, the filler `0x112490` fills that record from the per-player stats object, and the engine's stats method `0x20F3DC0` builds that object, so most keys trace back to one engine field. About a dozen keys stay untraced, because they come from settings and player objects that were not followed: instrument, difficulty, profile_name, modifiers, remote_network_player, lefty_flip, no_fail, video_calibration, audio_calibration, controller_type, versus_winner and is_bot, and the engine byte `+0xA8` inside is_pfc. The table below gives, per key, the observed values and what writes it.
Label: Proven in the game's code
Evidence: stringliteral.json holds every key name. All 16 scorestats files under C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs show 46 per-player keys and 3 section keys each, and clean_play_bonus 0 in all. The 16 are 13 in the dated folders 2026-11-06 to 2026-11-08 and the 3 under 2026-10-28\l1\bot. The kept script is docs/audit/2026-11-08-clone-hero-unknowns/main/sc37_scorestats_keys.py, and its output is sc37_scorestats_keys_output.txt beside it (run 2026-11-11). That run 1's file and every file share the same top-level keys was checked on 2026-11-08 by a script that was not kept. The writer is `code/6142/score_submit.asm` lines 130 to 475, the filler `code/6142/stats_fill_112490.asm` lines 13 to 192, and the engine stats method `code/6142/engine_stats_20F3DC0.asm` lines 180 to 300. Each key's three hops (the writer line that names the key, the filler line that sets its record field, and the engine line that sets the stats field) are tabled in `docs/audit/2026-11-08-clone-hero-unknowns/W8.md`, "SC-37". The two getters the filler calls are `code/6142/player_avg_multiplier_2130570.asm` and `code/6142/player_is_pfc_212F4F0.asm`.
Game code (6142): the per-player serialiser, RVA `0x1144C0`, `code/6142/score_submit.asm` lines 130 to 475; the filler, RVA `0x112490`; the engine stats method, RVA `0x20F3DC0`; listings as above
Hydra: no owner in src/
Ruling: none needed: no Hydra rule depends on it
Found: 2026-10-30, session b02ab3e6 (the reference build); the observed values, 2026-11-08, session 77057df2 (docs-gaps search); the writers 2026-11-08, session 77057df2; checked 2026-11-11, session 428cf079

Per-player keys. In the "what writes it" column an engine field is traced through the writer, the filler and the engine stats method (W8.md gives the lines); "not traced" means the hop stops at a settings or player object that was not followed. Engine fields are on #8288. The observed column covers the 16 files under the run folder, read on 2026-11-08: three bot runs and 13 virtual kit runs, each with one player. Run 1's file is named where it differs.

| key | observed | what writes it |
|---|---|---|
| instrument | Drums in five files, ProDrums in eleven | not traced (a byte of an object the filler is handed) |
| difficulty | Expert in every file | not traced (the next byte of that object) |
| profile_name | empty in every file; Guest on run 1 | not traced (that object's +0x18, then +0x38) |
| modifiers | ["DoubleKick"] in every file | not traced (a list from `0x116D00`) |
| remote_network_player | false in every file | not traced (a byte at +0xB1 of the filler's object, not the engine's player kind at #8288+0x190; whether the two agree was not checked) |
| lefty_flip | false in every file | not traced (a setting getter) |
| no_fail | true in every file | not traced (a static setting) |
| video_calibration | 0.03 in every file (written 0.0299999993); 0 on run 1 | not traced (a static setting) |
| audio_calibration | 0 in every file | not traced (a static setting) |
| controller_type | Drums in every file | not traced (a value 0 to 3 from `0x337CE0`, mapped one to one) |
| score | the final score, 9,400 to 1,229,730 | engine +0x94 plus +0xA0 plus +0xA4, through the getter `0x2130490` (SC-29) |
| base_score | the star base, 6,050 to 222,935 | engine +0x1D0 (SC-27) |
| solo_bonus_total | 0 in twelve files; 56,020, 49,700, 58,000 and 12,100 on the four runs of charts with solos (SC-20) | engine +0xA0 |
| squeezed_score | 0 in eight files, 380 to 3,940 in the other eight; 1,385 on run 1 | engine +0xD0 (SC-09) |
| clean_play_bonus | 0 in every file | engine +0xA4; no writer of +0xA4 traced, and Hydra has no such category |
| note_score | SC-32 | engine +0xC0 less 50 times (+0xD8 plus +0xDC) |
| sustain_score | 0 in every file | engine +0xC4 (SC-04) |
| sp_score | 0 in the eight files with no activation, 96,070 to 283,720 in the other eight; 83,260 on run 1 | engine +0xCC (SC-08) |
| combo_score | 4,350 to 665,075; equals Hydra's Combo with no Star Power (SC-07) | engine +0xC8 (SC-08) |
| notes_hit | gems hit, 101 to 3,753 (1,257 on Trippin SP Spike) | engine +0x9C (SC-06) |
| total_notes | gems in the chart, 121 to 3,767 | a sum the engine stats method's section loop builds (the loop was not read whole) |
| max_streak | the longest streak in gems: the gem count on the nine full-combo files, 38 to 2,985 on the seven with misses | engine +0x98 (SC-06) |
| end_streak | the streak at the song's end: equal to max_streak on the full-combo files, 12 to 2,985 on the others | engine +0x88 |
| excess_hits | 0 in eleven files; 1 on LS-O1 run 1, 4 on LS-E and LS-F1, 11 on LS-O2 and 21 on LS-T3 run b | engine +0xB0 |
| sp_phrases_earned | 0 on the two Trippin SP Spike bot runs (of 11) and on the charts with no phrases; 14 to 30 on the others (15 of 16 on LS-F1, 30 of 31 on LS-M2); 11 on run 1 | engine +0x4C |
| sp_phrases_total | 0, 11, 14, 16, 25 or 31, by chart | the count of a list at engine +0x228, then +0x70 |
| sp_activations | 0, 4, 5, 8 or 9; 4 on run 1 | engine +0xB4 (page 5 SP-19) |
| time_in_sp | 0, or 37.81 to 106.66 s; 50.03 s on run 1 | engine +0xB8, a double cut to a float (page 5 SP-25) |
| sp_ticks_accumulated | 0; 20,160 (14 x 1,440) and 23,040 (16 x 1,440) on the 5,760-bar charts; 90,000 (25 x 3,600) and 108,000 (30 x 3,600) on the 14,400-bar charts; 53,925 on LS-F1 run 1 (15 phrases earned, 75 short of 15 x 3,600); 15,840 on run 1 (11 x 1,440) | engine +0x80 (page 5 SP-12) |
| sp_bar_ticks | 5,760 in five files, 14,400 in eleven; the two charts of SC-20, both at resolution 480, read 14,400 | engine +0x1B0 (page 5 SP-11) |
| stars | 3 to 7 | the engine's own count on its score +0x94 (agrees with SC-28) |
| avg_multiplier | score less solo_bonus_total, over base_score (SC-33), 1.32 to 5.26 | engine +0x94 over +0x1D0 in 32-bit floats, 0 when the score is 0 or less, through the getter `0x2130570` (SC-33) |
| squeezed_notes | 0 to 18; 7 on run 1 | engine +0xE8 (page 5 SP-36) |
| squeeze_note_losses | 0, 1, 3, 6 or 8; 1 on run 1 | engine +0xEC (page 5 SP-36) |
| frets_ghosted | 0 in every file | engine +0xD4 |
| accent_notes_hit | up to 368 of 368 (LS-O3 run 1); 0 of 368 on LS-D2d run 1 and 0 of 9 on run 1 (SC-14) | engine +0xDC (SC-13) |
| accent_notes_total | 0 to 368, by chart | engine +0xE4 |
| ghost_notes_hit | up to 726 of 726 (LS-O3 run 1); 0 of 726 on LS-D2d run 1 and 0 of 34 on run 1 (SC-14) | engine +0xD8 (SC-13) |
| ghost_notes_total | 0 to 726, by chart | engine +0xE0 |
| is_fc | true in the nine files that hit every note, false in the seven with misses | set when the record's max_streak equals its total_notes |
| is_pfc | equal to is_fc in every file | the getter `0x212F4F0`: max_streak equals total_notes, above 0, and the engine byte +0xA8 is 0; that byte was not traced |
| versus_winner | true in every file | not traced (a player byte at +0xA9; the engine stats method does not set it) |
| is_bot | true on the three bot runs, false on the 13 virtual kit runs and on run 1 | not traced (the return of `0x13F8B0` into the record byte at +0xB6, page 7 IN-38) |
| failed_in_run_time | -1 in fourteen files; 5,991 on LS-O1 run 1 and 28,076 on LS-T3 run b, both with no_fail on | engine +0xAC |
| section_count | 7 to 35; 10 on the charts with no [section] events (Window Map, Edge Walk Pair, Get Ur Freak On), a game fallback of "0% - 10%" sections | the count of the section list |
| section_stats | a list of section_name, notes_hit and notes_count | the list the engine stats method builds in its section loop |

The top-level keys are written by the serialiser of SC-36; where each value comes from is Unknown. Observed in all 16 files: game_version "v1.1.0.6142-final", playback_speed 100, game_mode "Quickplay", player_count 1, band_score equal to the player's score and band_stars equal to stars. The online score record `ScoreData` (dump.cs line 18833) is a different structure, the leaderboard upload.

## Rows waiting on a live run

SC-07, SC-08, SC-10, SC-11, SC-12, SC-14, SC-20 (the exactly-half case, the .5 rounding and a missed solo-end note), SC-29, SC-30. Each is on the live-checks page.

## Open splits

SC-12 and SC-14 (Q-51).
