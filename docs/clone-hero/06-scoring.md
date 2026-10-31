# Scoring [SC]

This page covers note values, the multiplier and combo, accents and ghosts, the solo bonus, star cutoffs and the score breakdown.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-10-30.

The run files named below are of two kinds. The three bot runs of 2026-10-28 are in C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\l1\bot\ (outside the repo): Window Map, and Trippin SP Spike on Drums and on ProDrums. "Run 1" is the key-press auto-player run on Trippin SP Spike, `run1_cal0_scorestats.json` under tools/ch_probe/experiments/results/overlay-spike/, untracked in git on 2026-10-30. Engine fields belong to the base engine class #8288; the drum engine is #8255. `ghidra_decompile_all.txt` is Ghidra's full decompile of build 6142, which `code/README.md` describes; it is not in the repo.

## Note values and the cymbal bonus

### SC-01 A gem is worth 50 points, and a chord pays per gem

Rule: Each gem pays a base of 50 points. A chord pays the base once for each gem in it. The gem count is the note's stored count when set, otherwise the number of set bits in its lane mask.
Label: Proven in the game's code
Evidence: The base adder works out the gem count at lines 17 to 38 (stored count at note +0x50, else a popcount of the lane mask). The drum hit routine passes 50 as the base at line 127 of its listing. The game also holds 50 as a named constant of class #8611 (dump.cs line 549143). Live: Window Map's base_score 7,100 is 142 gems times 50.
Game code (6142): adder RVA `0x20F6EB0`, `code/6142/add_note_score_20F6EB0.asm` lines 17 to 38; drum hit routine RVA `0x20DE770`, `code/6142/note_hit_dynamics_20DE770.asm` line 127
Hydra: `kNoteBasePoints` in src/core/model.h line 143; `ChordNote::basescore` in src/core/model.cpp line 84
Ruling: none needed: same rule. No decision names the 50 itself; D51 call 13 (decision log line 156, items in docs/audit/2026-10-04-phase-7-questions.md item 13) records the cymbal's +15, the solo's 100 and the multiplier steps "as they are", not the 50.
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-02 A cymbal gem pays 15 more

Rule: A gem on a cymbal lane pays 65 instead of 50. The test is any of three lane-mask bits (0xE0).
Label: Proven in the game's code
Evidence: The drum score method calls the cymbal test and adds 15 at lines 13 to 16 before tail-calling the base adder at line 31. The cymbal test (RVA `0x214BE30`) is `test word [note+0x80], 0xE0`, as the star base's note says too (`code/6142/star_base_20F4680.md` line 16); the reviewer of this page also read it with tools/ch_mcp/chdis.py on the 6142 DLL, 2026-10-30. The same +15 shape is in the star base (SC-27).
Game code (6142): RVA `0x20DEEC0` (drum override of slot 6), `code/6142/note_score_add_20DEEC0.asm` lines 13 to 16 and 31
Hydra: `kCymbalBonusPoints` in src/core/model.h line 144; `ChordNote::basescore` in src/core/model.cpp line 84
Ruling: none needed: same rule (D51 call 13 records the cymbal's +15 "as they are", decision log line 156)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-03 A hit adds points times the multiplier, with no speed term

Rule: A hit raises the score by its points times the current multiplier, and adds the plain points to a raw total. Nothing in the adder depends on playback speed. The adder returns at once, scoring nothing, when the engine's player kind is RemotePlayer (value 1, SP-23); a player, bot or replay engine scores.
Label: Proven in the game's code
Evidence: Lines 13 and 14 compare the player kind at +0x190 with 1 and skip the whole adder. Lines 36 to 41 add points to +0xC0 and points times +0x48 to the score at +0x94. ghidra_decompile_all.txt lines 53749 to 53751 read the same. A speed term applied to the points before the adder is not ruled out; a half-speed video matched Hydra's curve point for point (card e05522d2).
Game code (6142): RVA `0x20F6EB0`, `code/6142/add_note_score_20F6EB0.asm` lines 13 and 14 and 36 to 41; `#8288+0x94`, `#8288+0x190`
Hydra: `category_scores` in src/core/scoring.cpp line 26; no speed owner, only `is_base_speed` in src/net/dmbot_client.h line 36 for leaderboard comparisons
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
Hydra: `to_multiplier` in src/core/timing.cpp line 12; `kStarPowerMultiplier` in src/core/timing.h line 48
Ruling: none needed: same rule (D51 call 13 records the steps at combos 10, 20 and 30 "as they are", decision log line 156)
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

### SC-09 A fourth running total moves on two note flags

Rule: The adder also changes a fourth total at +0xD0: it takes away (multiplier minus 1) times points outside Star Power when one note flag is set, and adds half times points inside Star Power when another is set. What the total and the flags mean is not known.
Label: Unknown
Evidence: The instructions are lines 51 to 54 and 74 to 80 of the adder; the flag getters `0x214BBA0` and `0x214B600` each test one bit of note +0x84. The tie to the file's squeezed_score is a guess (SC-37).
Game code (6142): RVA `0x20F6EB0`, `code/6142/add_note_score_20F6EB0.asm` lines 51 to 54 and 74 to 80
Hydra: no owner: not searched
Ruling: none yet: the meaning is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-10 Hit order inside a chord at a multiplier step

Rule: When a chord crosses a multiplier step, each gem may pay at the multiplier of its own position, so the order of the hits changes the score. Hydra prices the best order (cheapest gem first).
Label: Hydra assumes it, never checked
Evidence: `multsqueeze_gain` in src/core/scoring.cpp line 134. The 15-point "hit-order loss" in The Violation FC report (C:\ytdl\videos\The Violation FC 2x60 [2MqPDMH4-MY].fcreport.md line 46) was declared by hand to explain a residual, not detected. The game's order rule was not read.
Game code (6142): none read
Hydra: `multsqueeze_gain` in src/core/scoring.cpp line 134
Ruling: card 56ac5b71: "in both cases you have to hit the kick first and cymbals are interchangeable so t doesnt matter which is hit first" (01:54 UTC; the advice wording only)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-11 Whether the combo carries into the next song of a setlist

Rule: In album or setlist play the combo may carry from one song to the next, so every note can pay 4x from the first. Hydra starts the combo at 0 on each chart.
Label: Unknown
Evidence: not checked: only card 2059d526 says "the album run starts with a combo over 12,000"; its video report was not opened.
Game code (6142): none read
Hydra: `chord_score_table` in src/core/scoring.cpp line 99 (one combo counter per chart)
Ruling: none yet: the game's rule is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build)

## Accents and ghosts

### SC-12 An accent or ghost pays double only when marked as performed

Rule: An accent or ghost gem pays double only when the hit handler has marked it as performed (the flag with value 4 in the word at note +0x84). Otherwise it pays its plain value.
Label: Proven in the game's code
Evidence: The drum score method calls the flag test and doubles at lines 19 to 24, then tail-calls the adder at line 31. The test (RVA `0x214BB40`) is `test byte [note+0x84], 4`, and the hit handler's setter (RVA `0x214B740`) sets that same flag; the reviewer of this page read both with tools/ch_mcp/chdis.py on the 6142 DLL, 2026-10-30, and neither is a committed listing.
Game code (6142): RVA `0x20DEEC0`, `code/6142/note_score_add_20DEEC0.asm` lines 19 to 31
Hydra: no owner for a performed mark: Hydra pays every accent and ghost as performed (src/core/scoring.cpp)
Ruling: open split: see questions page Q-51
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-13 The performed mark compares a lane value with a threshold

Rule: The hit handler picks a signed byte per lane and compares it with an engine threshold. A ghost is marked when the byte is at least 0 and below the threshold; an accent is marked when it is at or above it. A negative byte marks neither. Each mark also bumps a counter: +0xD8 for a ghost and +0xDC for an accent.
Label: Proven in the game's code
Evidence: Lines 29 to 61 pick the lane byte (one byte each at +0x309 to +0x310); lines 72 to 83 are the path for the flag the getter `0x214BE10` tests (note +0x7C bit 0x80000, `DrumsAccentHit` at dump.cs line 548216): mark and bump +0xDC, the accent counter. Lines 91 to 102 are the path for `0x214B6C0` (bit 0x40000, `DrumsGhostHit`, dump.cs line 548215): mark and bump +0xD8, the ghost counter. Both getters were read in the 6142 DLL with tools/ch_mcp/chdis.py on 2026-10-31 (each is one `test dword ptr [rcx + 0x7c]` against that bit). ghidra_decompile_all.txt lines 1163 to 1181 read the same. What the bytes and the threshold hold in play, and what writes them, was not found; which side is a hard hit is not known.
Game code (6142): RVA `0x20DE770` (slot 11), `code/6142/note_hit_dynamics_20DE770.asm` lines 29 to 102; threshold `#8255+0x2C0`
Hydra: no owner: Hydra has no velocity or threshold
Ruling: none needed for the comparison; the paid-or-not question is Q-51
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-14 A keyboard press never earns accent or ghost credit

Rule: On a key-press run the game counted no accent or ghost as hit, while the bot on the same chart hit them all.
Label: Proven in a live run
Evidence: Run 1 (keys, ProDrums): accent_notes_hit 0 of 9, ghost_notes_hit 0 of 34. ProDrums bot on the same chart: 9 of 9 and 34 of 34 (scorestats-Trippin_SP_Spike-20261028-212754.json). One key run only; the code does not show where a key press gets its lane value.
Game code (6142): none beyond SC-13
Hydra: no owner: Hydra scores a dynamic gem as always played right
Ruling: open split: see questions page Q-51 (the user said in session 6a549de0: "the code is the source of truth but from experience i can tell you dynamics only work with a real drum kit. keyboard inputs never register dynamics."; that states the game's behaviour, not what Hydra should price)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-15 Plain Drums drops accents and ghosts

Rule: On plain Drums (instrument 6) the game clears the accent and ghost bits of every note in the chart builder, so no dynamics score. On ProDrums (instrument 9) they stay.
Label: Proven in the game's code
Evidence: The chart builder compares the instrument with 9 and otherwise calls `0x20F16E0` then `0x20EFEE0` (docs/audit/note-shuffle/step0/open2/f20d2ea0_build.asm lines 245 to 253), which clears the bits (docs/audit/note-shuffle/step0/open2-refute/f20EFEE0.asm line 25). Live: on Trippin SP Spike the plain-Drums bot scored 248,550 against Hydra's 257,150, a gap of 8,600 = 43 dynamic gems times 50 times 4x.
Game code (6142): chart builder RVA `0x20D2EA0`; clearing RVA `0x20EFEE0`
Hydra: no owner: Hydra keeps dynamics on plain Drums
Ruling: the user, card 6a549de0: "there is no way this is intentional, it must be a bug in clone hero" (01:30 UTC) and "Keep Hydra as is" (01:44 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-16 An unperformed accent or ghost may stop scoring under one flag

Rule: When bit 0x10 of the engine flags word is set and an accent or ghost was not performed, the hit handler returns before it scores the note or steps the streak. What sets the bit is not known.
Label: Unknown
Evidence: Lines 84 to 90 and 103 to 104 test the flags word at +0x198 for 0x10 and return. The setter was not searched; tools/ch_probe/constants.py names only bit 0x1000 of `OFF_FLAGS`.
Game code (6142): RVA `0x20DE770`, `code/6142/note_hit_dynamics_20DE770.asm` lines 84 to 90; `#8288+0x198`
Hydra: no owner: Hydra scores every note
Ruling: none yet: the trigger is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build)

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
Evidence: docs/adr/0012-kick-dynamics-are-priced.md line 23: "Ghost kicks are proven by video on this chart." (the Who chart; 36 ghost kicks explain 1,142,235 against 1,143,035). The video is under C:\ytdl\videos and listed in C:\Users\Patrick\Downloads\Hydra\video-tools\refs.json; not re-run for this page.
Game code (6142): none
Hydra: `ChordNote::basescore` in src/core/model.cpp line 84
Ruling: card 528e137d: "velocity 1 is a ghost, 127 is an accent, on both the normal kick and the 2x kick pitch" (02:58 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-19 An accent kick pays double

Rule: An accent on the kick pays double like any accent.
Label: Hydra assumes it, never checked
Evidence: ADR 0012 lines 26 to 30: "Accent kicks are assumed, not proven." No saved run has a kick accent.
Game code (6142): none
Hydra: `ChordNote::basescore` in src/core/model.cpp line 84
Ruling: card 528e137d: "The symmetric rule is the sensible default, but it needs a video of a chart with velocity-127 kicks before we trust it." (02:58 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

## The solo bonus

### SC-20 The solo bonus at a full solo is 100 per note

Rule: A solo hit in full pays 100 points per note, once, with no multiplier and no Star Power. Below 50 percent it pays nothing, and between 50 and 100 percent it follows 10 times round(((h / N) minus 0.5) times 2 times h times 10) in 64-bit float, rounding half to even, paid at the solo-end note even when that note is missed.
Label: Unknown
Evidence: listing lost; read 2026-10-16 at the game's solo-end method, sessions 1aaf0ac4 and 5b35957b (no address recorded; card 1aaf0ac4 says the formula "stayed unconfirmed at the end"). The reference build looked in `SoloCounter` (TypeDefIndex 123; its methods hold only "Solo Move" and "frets"), its fields and every string literal containing "solo", and did not find the function. Every saved run has solo_bonus_total 0; run 1's "Guitar Solo" sections are [section] names, not solo markers. Public `3b49012c:hydra/hypath.py` line 65 pays 100 * timestamp.chord.count().
Game code (6142): not found: looked in `SoloCounter` methods `0x293A30`, `0x294070`, `0x294880`, `0x2946F0`, `0x295190` and `0x2956C0`
Hydra: `solo_bonus` in src/core/scoring.cpp line 95 with `kSoloBonusPerNote` in src/core/model.h line 146 (full solos only, since Hydra's paths hit every note)
Ruling: D251: "Like the game" (decision log line 878)
Found: 2026-10-16, session 5b35957b

### SC-21 The solo's note count counts every gem

Rule: Hydra counts every gem of a chord in a solo's note total. The game's count is not settled. Session 1aaf0ac4 (2026-10-16) first wrote that every gem counts, citing the Swim to the Moon video in which a 236-note solo paid 23,600 points (SC-22); a later agent in the same session reported that the game merges same-tick drum gems into one note.
Label: Unknown
Evidence: not checked: no listing. The two statements are in session 1aaf0ac4's condensed transcript, lines 97 and 214, and card 1aaf0ac4 line 29; no repo file holds either. The video's 236 may itself count gems or chords, so it does not settle the question.
Game code (6142): none
Hydra: `solo_note_count` in src/core/scoring.cpp line 91
Ruling: none yet: the game's rule is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-22 The bonus is paid as one lump at the solo's end

Rule: The game pays the solo bonus in one lump when the solo ends, so a score curve trails the per-note total during a solo.
Label: Proven in a live run
Evidence: Swim to the Moon video, 2026-08-17 (card e61236dc); C:\Users\Patrick\Downloads\Hydra\video-tools\README.md lines 457 and 458 record it; result file C:\ytdl\videos\Swim to the Moon Pro Drums FC [OoE7PMv-ekk].fcreport.json.
Game code (6142): none
Hydra: `chord_score_table` in src/core/scoring.cpp line 99 books it per chord (the totals agree)
Ruling: none needed: a display difference only
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-23 The solo box shows a whole percent and a message ladder

Rule: The box appears when the player hits the solo's first note, not at the chart's solo marker, and hitting the last note ends the solo. During a solo the game shows trunc(float(h) / float(N) times 100.0f) percent. After the last note it shows a message for 1.5 s and the bonus for 1.5 s. The messages run 100 Perfect, 95+ Awesome, 90+ Great, 80+ Good, 70+ Solid, 69 "nice", 60+ Okay, else Messy, and N minus 1 gives "Awesome Choke!".
Label: Unknown
Evidence: listing lost; read 2026-10-16, sessions 1aaf0ac4 and 5b35957b (D251's record, decision log line 876). The first-note start is in those sessions' condensed transcripts (line 77 of each) and in no repo file. Hydra's own box was checked in the real app (card 5b35957b: 44% = 53 / 119), which checks Hydra, not the game.
Game code (6142): not found (as SC-20)
Hydra: `solo_percent` in src/app/preview_view.cpp; `kSoloResultMs` in src/app/preview_view.h (Hydra shows only "Perfect Solo!")
Ruling: D251: "Game's look + count" and "Like the game" (decision log lines 877 and 878)
Found: 2026-10-30, session b02ab3e6 (the reference build)

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

Rule: The rounding after the float multiply is Math.Ceiling, so a cutoff moves by one point only when the base score is not a multiple of 5; drum bases always are.
Label: Unknown
Evidence: not checked in a listing: the helper has none. Session 9168b711 (2026-09-27) read it as roundsd mode 0xA; that listing is lost and no repo file holds it. An example of where 32-bit floats matter: a base of 786,437 times the 3.6 slot is 2,831,173.0 in 32-bit floats, so 6 stars need 2,831,173 points, where exact arithmetic would need 2,831,174 (worked out with Python's 32-bit float packing on 2026-10-31 from SC-24 and SC-25; it assumes the rounding is Math.Ceiling, which is this block's open question). The reviewer of this page opened the helper once (tools/ch_mcp/chdis.py on the 6142 DLL, 2026-10-30, no listing committed): it tests a stored CPU-feature value and, when it is 2 or more, jumps to `roundsd xmm0, xmm0, 0xa`, which rounds up; below 2 it runs a software routine that was not read. That fits "rounds up" on a machine with that instruction, and the software path is why the block stays Unknown. Multiples of 5 follow from 50, 65, 100 and 130 (SC-01, SC-02).
Game code (6142): call target in `code/6142/star_count_210DD00.asm` lines 46 to 48, not listed
Hydra: `star_cutoff` in src/core/stars.cpp line 9
Ruling: none needed: no drum chart reaches the difference
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-27 The star base is worked out once, when the engine is built

Rule: The drums star base is the sum over notes of (50, plus 15 for a cymbal) times the gem count, times 2 when the note carries either dynamic flag (once, even with both). There is no sustain term for drums. It runs when the instrument is Drums (6) or ProDrums (9), after the Note Shuffle, and is stored at +0x1D0. Other instruments take a sustain term instead: every note whose length is at least a third of a chart value (read as the resolution) adds its length in ticks, and the total is turned into points once, at 25 points per beat, rounded by the same helper as SC-26.
Label: Proven in the game's code
Evidence: Star base lines 68 (drums branch), 87 to 101 (cymbal and the two flag tests), 103 to 118 (gem count) and 120 to 123 (the sum); lines 68 to 80 and 147 to 149 leave the sustain term at 0. The non-drums term is lines 29 to 39 (the value divided by 3.0), 73 to 82 (the length test) and 139 to 149 (the total divided by the value over 25.0); that the value at chart record +0xB8 is the resolution is session 9168b711's reading (card line 24 says "sustains at 25 points per beat"), not shown by name in the dump. The constructor picks the drums branch for 6 or 9 at lines 183 to 189, calls it at line 196 and stores the result at line 202. That the two flags are accent and ghost is named by the dump: 0x40000 is `DrumsGhostHit` and 0x80000 is `DrumsAccentHit` (dump.cs lines 548215 and 548216). Live: the game's base_score is 73,520 on Trippin SP Spike ProDrums, 62,850 on its plain Drums run (1,257 gems times 50, no cymbals or dynamics) and 7,100 on Window Map (142 gems times 50). Hydra's totals for the same charts are in `trippin.json`, `trippin-normal.json` and `window-map.json`, one folder above the bot files. On ProDrums Hydra's notes, accent and ghost totals sum to the game's base (71,370 + 450 + 1,700 = 73,520). On Window Map Hydra's notes total is 7,100 with no dynamics. On plain Drums the game's 62,850 equals Hydra's notes total only, because Hydra still prices 450 + 1,700 for dynamics there and the game drops them (SC-15).
Game code (6142): RVA `0x20F4680`, `code/6142/star_base_20F4680.asm` lines 68 to 149; constructor RVA `0x20F79E0`, `code/6142/engine_ctor_20F79E0.asm` lines 183 to 202
Hydra: `Path::chart_base_score` in src/core/model.cpp line 833
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

### SC-29 The star count may leave out the solo bonus

Rule: Hydra compares a score without the solo bonus with the cutoffs. Whether the game's count does the same is not read.
Label: Unknown
Evidence: not checked: four other callers of the star counter (`0x210E2A0`, `0x210F140`, `0x21104F0`, `0x2110920`, all in #8351) were found and not opened. Card 9168b711 says which bonus field is "solo" came from CHOpt and the changelog, not from the code. That session read the count's score as leaving out two bonus totals that are added after the stars are counted; CHOpt (another optimizer) names them the solo bonus and the clean-play bonus, and the game's changelog says solos stopped counting toward stars in v0.20 (condensed transcript of session 9168b711, line 123; no repo file holds the listing). The same session found that the official wiki (wiki.clonehero.net, "How to play") and CHOpt agree with the game's cutoffs for 3 to 7 stars (card 9168b711 line 24). No saved run has a solo.
Game code (6142): callers of RVA `0x210DD00`, not opened
Hydra: `score_without_solo` in src/core/stars.h line 51; `path_stars` line 59
Ruling: none yet: the game's rule is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build)

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

Rule: In the game's file, score equals note_score plus combo_score plus sp_score plus 50 for each accent or ghost hit. squeezed_score is not part of the sum.
Label: Proven in a live run
Evidence: ProDrums bot: 71,370 + 217,455 + 0 + 50 x 43 = 290,975. Plain Drums bot: 62,850 + 185,700 = 248,550. Window Map: 7,100 + 18,450 = 25,550. Run 1: 71,235 + 210,600 + 83,260 = 365,095, with squeezed_score 1,385 left out. solo_bonus_total is 0 in all four, so the solo term is unchecked.
Game code (6142): none (the per-player writer is not traced, SC-35)
Hydra: `score_total` in src/core/model.h line 151
Ruling: none needed: same sum
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-32 The Notes line is the 1x total less 50 per dynamic hit

Rule: The results screen's Notes line is the plain-points total less 50 for each accent or ghost hit, so a dynamic cymbal's extra 15 stays in Notes. The Accent and Ghost lines are the hits times 50.
Label: Unknown
Evidence: listing lost; read 2026-10-11 at RVA `0x20F3DC0` (instructions 0x20F425C to 0x20F426F) and at 0x126FD6 and 0x127029, session 1bae448b (condensed transcript lines 108, 111 and 112). Both of those addresses lie inside the `PlayerStatsBox` method that starts at 0x126280 and ends before 0x1270D0 (its bounds from the 6142 dump's script.json, checked 2026-10-31); the dump names its class `PlayerStatsBox`. The same session read the base value 50 passed by the drum hit handler at 0x20DE96C (line 98), inside 0x20DE770. Live: the ProDrums bot's note_score 71,370 = base_score 73,520 less 50 x 43, and equals Hydra's Notes. Run 1 (no dynamic performed) has note_score 71,235, 2,285 below base_score; that would be 43 x 50 plus 9 x 15 if 9 of the dynamic gems are cymbals, which was not checked.
Game code (6142): RVA `0x20F3DC0` (listing lost)
Hydra: `category_scores` in src/core/scoring.cpp line 26
Ruling: D208: "we don't need to change anything, matching the game is correct." (decision log line 777)
Found: 2026-10-11, session 1bae448b

### SC-33 The average multiplier is score over base score

Rule: The game's avg_multiplier is score divided by base_score.
Label: Proven in a live run
Evidence: 248,550 / 62,850 = 3.95465394 (file 3.95465398); 25,550 / 7,100 = 3.59859155 (3.59859157); 290,975 / 73,520 = 3.95776659 (3.95776653); 365,095 / 73,520 = 4.96592764 (4.9659276). Agreement to float precision, on runs with no solo bonus. With a solo, whether the game's field keeps the bonus in is not measured; Hydra takes it out.
Game code (6142): none (the writer is not traced)
Hydra: `Path::avg_mult` in src/core/model.cpp line 837 (total less solo, over the base score)
Ruling: D222: "change it to Average Multiplier, that's a real stat you see in the game" (decision log line 805)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SC-34 Off-speed scores are never judged against Hydra's optimum

Rule: Hydra's optimum is for normal speed only, and a score above it is labelled "above optimal", never "impossible", because Hydra's optimum leaves out many backends and the old fill-spawn logic.
Label: Hydra assumes it, never checked
Evidence: src/app/dm_report.cpp lines 107 to 122. The dmleaderboards site (a community leaderboard for drum scores) ranks every speed on one board per chart. The user said so on 2026-10-10 (D206, card 35360d51 line 18: "dmleaderboards does not use separate leaderboards for different speeds."); the public DMBot source and a user screenshot backed it, neither committed. The Clone Hero v1.1 leaderboard announcement (https://clonehero.net/2024/11/10/clonehero-leaderboards.html) says the game's own leaderboards show only scores for the selected modifiers and speed (ch-evidence.md question 222 at commit 5fbe71ae; not re-read for this page).
Game code (6142): none (outside the game)
Hydra: `is_base_speed` in src/net/dmbot_client.h line 36
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

Rule: Each player entry holds 46 keys, one of them a list of sections with 3 keys each. Every key is a string literal in the game, and every run wrote them all. Which engine counter feeds each key is not traced: the per-player serialiser was not opened. The table below gives, per key, the observed values and what writes it.
Label: Unknown
Evidence: stringliteral.json holds every key name; the four saved files show the same key set. The per-player writer is another class: `0x251A70` only adds an object and `0x112490` copies another stats object.
Game code (6142): per-player serialiser: not found (looked in `0x251A70` and `0x112490`)
Hydra: no owner in src/
Ruling: none yet: the writers are not traced
Found: 2026-10-30, session b02ab3e6 (the reference build)

Per-player keys. "Unknown" means no code was read that writes the key; a named field is a candidate the value matches, never a traced writer.

| key | observed | what writes it |
|---|---|---|
| instrument | Drums or ProDrums | Unknown |
| difficulty | Expert | Unknown |
| profile_name | empty for bots, Guest for run 1 | Unknown |
| modifiers | ["DoubleKick"] | Unknown |
| remote_network_player | false | Unknown (the engine's player kind at #8288+0x190 is a candidate) |
| lefty_flip | false | Unknown |
| no_fail | true | Unknown |
| video_calibration | 0.03 on bots, 0 on run 1 | Unknown |
| audio_calibration | 0 | Unknown |
| controller_type | Drums | Unknown |
| score | the final score | Unknown (equals the engine score at #8288+0x94, SC-03, and band_score) |
| base_score | the star base | Unknown (equals the star base at #8288+0x1D0, SC-27) |
| solo_bonus_total | 0 in every run | Unknown |
| squeezed_score | 0, or 1,385 on run 1 | Unknown (the fourth total at #8288+0xD0, SC-09, is a candidate) |
| clean_play_bonus | 0 in every run | Unknown (no reader found; Hydra has no such category) |
| note_score | SC-32 | Unknown |
| sustain_score | 0 | Unknown (the second adder's total at #8288+0xC4, SC-04, is a candidate) |
| sp_score | 0, or 83,260 on run 1 | Unknown (the Star Power share at #8288+0xCC, SC-08) |
| combo_score | equals Hydra's Combo with no Star Power | Unknown (the Combo share at #8288+0xC8, SC-08) |
| notes_hit | gems hit (1,257 on Trippin SP Spike) | Unknown (the hit count at +0x9C, SC-06, is a candidate) |
| total_notes | gems in the chart | Unknown |
| max_streak | the gem count (1,257 on Trippin SP Spike, 142 on Window Map) | Unknown (the best streak at +0x98, SC-06) |
| end_streak | the gem count, as max_streak | Unknown |
| excess_hits | 0 in all four runs named above; 10 on one earlier key-press run with misses (card e9b04efc; that run's file was not kept) | Unknown |
| sp_phrases_earned | 0 on bots, 11 on run 1 | Unknown |
| sp_phrases_total | 11 | Unknown |
| sp_activations | 0 on bots, 4 on run 1 | Unknown (the activation counter at +0xB4, page 5 SP-19) |
| time_in_sp | 0, or 50.03 s on run 1 | Unknown (the time total at +0xB8, page 5 SP-25) |
| sp_ticks_accumulated | 0, or 15,840 on run 1 (11 x 1,440) | Unknown (the running gain total at +0x80, page 5 SP-12) |
| sp_bar_ticks | 5,760 at resolution 192, 14,400 at 480 | Unknown (equals the full bar at #8288+0x1B0, page 5 SP-11) |
| stars | 5 to 7 | Unknown (agrees with SC-28) |
| avg_multiplier | score over base_score | Unknown (SC-33) |
| squeezed_notes | 0, or 7 on run 1 | Unknown (the squeeze counter at +0xE8, page 5 SP-36, is a candidate) |
| squeeze_note_losses | 0, or 1 on run 1 | Unknown (the counter at +0xEC, page 5 SP-36, is a candidate) |
| frets_ghosted | 0 | Unknown |
| accent_notes_hit | 9 of 9 on the ProDrums bot, 0 on run 1 | Unknown (the accent counter at +0xDC, SC-13, is a candidate) |
| accent_notes_total | 9 | Unknown |
| ghost_notes_hit | 34 of 34 on the ProDrums bot, 0 on run 1 | Unknown (the ghost counter at +0xD8, SC-13, is a candidate) |
| ghost_notes_total | 34 | Unknown |
| is_fc | true | Unknown |
| is_pfc | true | Unknown |
| versus_winner | true | Unknown |
| is_bot | true on bots, false on run 1 | Unknown (no code reference to the literal was found) |
| failed_in_run_time | -1 | Unknown |
| section_count | 7, or 10 on Window Map (no [section] events, a game fallback of "0% - 10%" sections) | Unknown |
| section_stats | a list of section_name, notes_hit and notes_count | Unknown |

The top-level keys are written by the serialiser of SC-36; where each value comes from is Unknown. Observed: playback_speed 100, game_mode "Quickplay", band_score equal to the player's score and band_stars equal to stars in these one-player runs. The online score record `ScoreData` (dump.cs line 18833) is a different structure, the leaderboard upload.

## Rows waiting on a live run

SC-07, SC-08, SC-10, SC-11, SC-12, SC-14, SC-16, SC-19, SC-20, SC-21, SC-29, SC-30, SC-33. Each is on the live-checks page.

## Open splits

SC-12 and SC-14 (Q-51).
