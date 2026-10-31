# Building the note list [NL]

This page covers how Clone Hero turns the chart's drum notes into the note list it scores.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-10-30.

## Cymbals and toms

### NL-01 In a .mid, red is a tom and yellow, blue and green are cymbals unless a tom marker covers them

Rule: the .mid drum reader gives red the Tom flag, the kick neither flag, and yellow, blue and green the Cymbal flag; tom markers on pitches 110, 111 and 112 then turn yellow, blue and green into toms.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 334 to 340 (red gets 0x10, the kick is skipped, any other pad gets 0x20) and lines 244 to 294 (pitches 110 to 112 build Tom modifier records on yellow, blue and green). How far a marker reaches (Hydra holds it from note-on to note-off) was not read in the game, and the 2026-09-25 play run that settled "a marked gem is a tom" left no result file.
Game code (6142): the .mid drum reader, a method of class #8626, RVA `0x2155050`, the listing above.
Hydra: `kYellowTomMarkerPitch`, `kBlueTomMarkerPitch` and `kGreenTomMarkerPitch` in src/parse/song.cpp lines 680 to 682; `flag_cymbals_` set at lines 1092 to 1097.
Ruling: none needed: same rule.
Found: 2026-09-25, session 8157f2dc; listed by wave 2 scout R1.

### NL-02 The .mid marker pitches the drum reader dispatches

Rule: pitch 109 makes a Flam modifier, pitches 110 to 112 a Tom modifier (raising track flag +0xA2), and pitches 113 to 115 fall through and are skipped; 103, 116, 120, 126 and 127 go to four other sinks.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 151 to 243 (the compares and the jump table at 0x2155B2C, decoded by the scout) and 244 to 294 (the flam and tom records). The four sinks' bodies were not read.
Game code (6142): RVA `0x2155050`, lines 151 to 294.
Hydra: `kFlamMarkerPitch` to `kDoubleLaneMarkerPitch` in src/parse/song.cpp lines 678 to 689; the `MidiParser::optype` switch at lines 913 to 937.
Ruling: none needed: same pitches.
Found: 2026-10-30, wave 2 scout R1.

### NL-03 What each .chart N number is on drums

Rule: N 0 is the kick; N 1 to 4 are red, yellow, blue and the fourth pad (Hydra's green), each a tom; N 32 is a kick with the 2x flag; any other N value is looked up as a modifier.
Label: Proven in the game's code
Evidence: `code/6142/chart_n_handler_213D620.asm` lines 71 to 109 (the drum table, decoded through the jump table at 0x213DB10) and lines 95 to 107 (N 32); decompile lines 25098 to 25110 agree.
Game code (6142): the .chart N handler of class #8615, RVA `0x213D620`, the listing above.
Hydra: `ChartParser::note_optype` in src/parse/song.cpp lines 1683 to 1699; `chart_pad_color` at line 1668.
Ruling: none needed: same rule for N 0 to 4 and N 32.
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### NL-04 The game reads a fifth drum pad; Hydra drops it

Rule: the game reads .chart N 5 and .mid pitches 65, 77, 89 and 101 as a fifth pad (note type 17) and raises track flag +0xA1, and reads N 38 and N 44 as the accent and ghost of that pad; Hydra knows four pads and drops all of these. What the game does with a type-17 note in four-lane Pro Drums is unread.
Label: Proven in the game's code
Evidence: `code/6142/chart_n_handler_213D620.asm` lines 99 to 105 (type 0x11 raises +0xA1); `code/6142/pitch_difficulty_map_210D9C0.asm` lines 1 to 24 with the type map in docs/audit/note-shuffle/step0/open2/f210d7b0.asm; `code/6142/chart_modifier_decode_213D160.asm` lines 11 to 80 (N 38 and N 44 on that lane). How many library charts hold such lines was not counted (no library run).
Game code (6142): RVAs `0x213D620`, `0x210D9C0`, `0x210D7B0` and `0x213D160`, the listings above.
Hydra: `chart_pad_color` in src/parse/song.cpp lines 1668 to 1676 (pads 1 to 4) and `ChartParser::note_optype` lines 1700 to 1708 (34 to 37 and 40 to 43 only).
Ruling: open split: see questions page Q-13.
Found: 2026-10-30, wave 1 scout H2b and wave 2 scout R1.

### NL-05 The .chart modifiers: accents, ghosts and cymbals

Rule: N 34 to 38 make an accent and N 40 to 44 a ghost on red, yellow, blue, the fourth and the fifth pad; N 66, 67 and 68 make a cymbal on yellow, blue and the fourth pad and raise track flag +0xA2; N 39, N 45 to 65 and N 69 up give nothing. Each modifier starts and ends at its own tick, and no tag gates it.
Label: Proven in the game's code
Evidence: `code/6142/chart_modifier_decode_213D160.asm` lines 11 to 80; `code/6142/chart_n_handler_213D620.asm` lines 212 to 278 (the call, the cymbal kind raising +0xA2, the record written with the tick twice); decompile lines 24747 to 24840.
Game code (6142): RVAs `0x213D160` and `0x213D620`, the listings above.
Hydra: `ChartParser::note_optype` in src/parse/song.cpp lines 1690 to 1709, with `kChartCymbalMarkerBase`; `allows_cymbals` in src/core/model.cpp line 17.
Ruling: none needed: same rule for the four pads (the fifth is NL-04).
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### NL-06 A .chart applies its ghost and accent marks with no tag

Rule: the .chart finish step applies the cymbal, tom, flam, ghost and accent modifiers with no condition, so a .chart always has dynamics on.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f215e280.asm lines 23 to 45: five unconditional calls to the apply routine with kinds 7, 6, 8, 10 and 9. That the apply routine itself, docs/audit/note-shuffle/step0/open2/f215dbd0.asm, reads no dynamics flag was read on 2026-10-03 and not re-read since.
Game code (6142): RVA `0x215E280` calling `0x215DBD0`, the listings above.
Hydra: the ghost and accent cases of `ChartParser::note_optype` in src/parse/song.cpp lines 1700 to 1707, and the comment at lines 1889 to 1892.
Ruling: none needed: same rule.
Found: 2026-10-03, session cebf3006; re-read by wave 1 scout H2b.

### NL-07 A .chart marker with no note at its tick is skipped

Rule: a cymbal, ghost or accent marker whose tick holds no note is skipped, and the chart still loads.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f215dbd0.asm lines 104 to 110: the range lookup returns -1 and jumps to the exit.
Game code (6142): RVA `0x215DBD0`, lines 104 to 110.
Hydra: `Chord::apply_cymbal` and the ghost and accent appliers throw `ChartFileError`, which the per-op catch drops (src/core/model.cpp lines 287 to 304; src/parse/song.cpp line 1746).
Ruling: none needed: same rule.
Found: 2026-10-03, session cebf3006; re-read by wave 1 scout H2b.

### NL-08 A .chart marker on a chord with no note of its colour

Rule: Hydra drops a marker whose chord has notes but none of its colour; whether the game does the same is not known.
Label: Unknown
Evidence: only docs/audit/note-shuffle/step0/open2/f215dbd0.asm lines 84 to 130 were read; the per-colour check after the range lookup was not.
Game code (6142): RVA `0x215DBD0`; the colour check was not read.
Hydra: `Chord::apply_cymbal` and its siblings in src/core/model.cpp lines 291 to 304.
Ruling: none found.
Found: this plan's wave 1 scout H2b.

### NL-09 A .chart kick has no ghost or accent marker

Rule: the .chart modifier map has no case that targets the kick, so a .chart kick is always a normal kick.
Label: Proven in the game's code
Evidence: decompile lines 24750 to 24825 (only note numbers 34 to 38, 40 to 44 and 66 to 68 have cases; every other value returns lane 0); `code/6142/chart_modifier_decode_213D160.asm` lines 11 to 80.
Game code (6142): RVA `0x213D160`, the listing above.
Hydra: `ChartParser::note_optype` has no kick modifier case (src/parse/song.cpp lines 1695 to 1709); `op_kick` passes a normal kick at line 1419.
Ruling: none needed: same rule.
Found: 2026-10-04, session 9f4f8eb5 (ADR 0012); re-read by wave 1 scout H2b.

## Dynamics

### NL-10 A .mid note gets its ghost or accent mark from velocity 1 or 127, kicks included

Rule: once the dynamics tag is on, the .mid reader sets the ghost bit for velocity 1 and the accent bit for velocity 127 on every pitch it reads, kicks and 2x kicks included; before the tag the note is plain.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 325 to 333 (the tag byte test, then velocity 1 sets bit 9 and velocity 127 sets bit 10); docs/audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm lines 245 to 253 show the lane code is tested only after the velocity test.
Game code (6142): RVA `0x2155050`, the listing above.
Hydra: `vel_dyn` in src/parse/song.cpp lines 896 to 903; `op_note` and `op_kick` lines 823 to 841; `mop_kick` line 620.
Ruling: D24 (record text): "The dynamics tag counts only in Clone Hero's two exact spellings".
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### NL-11 The .mid dynamics tag is one of two exact spellings, set per track pass

Rule: dynamics turn on only at a drum-track text event equal to `ENABLE_CHART_DYNAMICS` or `[ENABLE_CHART_DYNAMICS]`, compared exactly with no trim inside the reader; the flag starts at 0 for each pass over a drum track, and notes before the event stay plain.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` line 100 (the flag starts 0 at 0x215524D), lines 363 to 368 (the event text loaded straight from the event) and 420 to 433 (two exact-equality compares at 0x21557A5 and 0x21557BB, then the flag set at 0x21557CD). Public Hydra accepted one-bracket forms (3b49012c, hydra/hysong.py line 273).
Game code (6142): RVA `0x2155050`, the listing above.
Hydra: `is_dynamics_marker` in src/parse/song.cpp lines 302 to 304; `dynamics_enabled_` reset at line 1098.
Ruling: D24: chose "Exact spellings (Recommended)" (card 2026-10-04-bbf41dc4, 01:57 UTC).
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### NL-12 Whether the drum track's event text is trimmed before it reaches the reader

Rule: the reader does not trim the tag text (NL-11), so a tag with a trailing space should not count; whether the text is trimmed earlier, where the event is built, is not known.
Label: Unknown
Evidence: no trim call between the text load and the compare in the reader; mtrk_read_event_2150010.asm and mtrk_read_214F6D0.asm under docs/audit/note-shuffle/step0/open1/ hold no trim either. The events-track reader does trim (midi_events_track_21544E0.asm, at 0x21546E3), but that is a different track. Where the drum track's text is built was not found.
Game code (6142): not found: looked in the listings named above.
Hydra: `is_dynamics_marker` in src/parse/song.cpp line 302 compares the raw text.
Ruling: D24 (record text): "caveat: assumes Clone Hero does not trim the text first".
Found: this plan's wave 1 scout H2b.

### NL-13 At one tick, a note before the dynamics tag stays plain

Rule: Hydra lets file order decide at a shared tick, so a note written before the tag on the tag's own tick stays plain; the game walks its event list in order, but how that list is built and sorted is unread.
Label: Unknown
Evidence: unchecked by the scout: it would need the code that builds and sorts the drum track's event list. The reader walks a passed-in list in order (`code/6142/mid_drum_reader_2155050.asm` lines 110 to 123).
Game code (6142): RVA `0x2155050`, lines 110 to 123; the list builder was not read.
Hydra: `kick_mark_before_tag_` and the note pass in src/parse/song.cpp lines 877 and 973 to 976.
Ruling: D24 (record text): "at a shared tick file order decides".
Found: 2026-10-03, session cebf3006.

### NL-14 A ghost kick scores double in the game

Rule: a velocity-1 kick under the dynamics tag scores double, as a ghost pad does.
Label: Unknown
Evidence: unchecked by the scout: ADR 0012 lines 14 to 24 call it proven by video (Won't Get Fooled Again (O), 36 ghost kicks, a full combo of 1,142,235), but no video or result file for that run was found. The reader side is NL-10.
Game code (6142): not found: the scoring side was not read.
Hydra: `mop_kick` and `op_kick` in src/parse/song.cpp lines 620 and 835.
Ruling: none found.
Found: ADR 0012.

### NL-15 An accent kick scores double in the game

Rule: a velocity-127 kick under the dynamics tag scores double, as an accent pad does.
Label: Hydra assumes it, never checked
Evidence: ADR 0012 lines 26 to 30 call it "assumed, not proven". The reader sets the accent bit on a kick (NL-10); the scorer was not read.
Game code (6142): not found: looked in the reader listing only.
Hydra: `mop_kick` in src/parse/song.cpp lines 620, 904 and 912.
Ruling: none found.
Found: ADR 0012.

### NL-16 A note with both a ghost and an accent mark plays as an accent

Rule: when one note carries both marks (as a merged kick can, NL-29), the game plays it as an accent.
Label: Unknown
Evidence: docs/audit/note-shuffle/step0/open2/f20d4be0_notelist.asm lines 141 to 153 test one flag getter first and skip the other when it is set; which getter is accent and which ghost was read in session 27b9bbc4's scratch folder, which is gone. The merge itself decides nothing (it or-s both bits, NL-29).
Game code (6142): RVA `0x20D4BE0`; listing lost; read 2026-10-09 in session 27b9bbc4.
Hydra: `add_dynamic_mark` in src/core/model.cpp lines 250 to 258.
Ruling: "go with the games rule" (card 2026-10-09-27b9bbc4).
Found: 2026-10-09, session 27b9bbc4.

## Disco flip

### NL-17 Disco flip is per difficulty, in both formats

Rule: a mix event goes only to the track of the difficulty it names, in a .mid and in a .chart, so each difficulty reads only its own markers.
Label: Proven in the game's code
Evidence: .chart: `code/6142/chart_e_handler_213CAF0.asm` lines 333 to 385 (the difficulty compare with the track's byte at +0x59, then the record). .mid: `code/6142/mid_drum_reader_2155050.asm` lines 363 to 418 (two starts-with tests on the raw text, the mix parser call, the track lookup by the parsed difficulty, the Add).
Game code (6142): RVAs `0x213CAF0` and `0x2155050`, the listings above.
Hydra: `is_disco_on_marker` and `is_disco_off_marker` in src/parse/song.cpp lines 307 and 318, given the difficulty's `mix_digit`.
Ruling: D19: chose "Per difficulty (Recommended)" (card 2026-10-04-bbf41dc4, 01:50 UTC).
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### NL-18 The mix parser accepts spaces or underscores and any integer spelling

Rule: the game's mix parser trims the text, drops one leading [ and a trailing ] if present, turns each underscore into a space, needs the first word to be exactly "mix", reads the next word as an integer (0 Easy to 3 Expert, anything else rejected), then needs the rest to start with "drums". Hydra needs exactly one separator byte of any kind and one digit byte at fixed places.
Label: Proven in the game's code
Evidence: `code/6142/mix_parser_215C750.asm` lines 59 (trim), 64 to 84 (brackets), 98 to 174 (underscore to space at lines 167 to 169), 140 to 184 (first word), 204 to 248 (integer parse and the map) and 263 to 290 (starts with "drums"); `code/6142/mix_digit_map_210D990.asm` lines 1 to 18.
Game code (6142): RVAs `0x215C750` and `0x210D990`, the listings above.
Hydra: `disco_head` in src/parse/song.cpp lines 293 to 296; `peel_brackets` lines 283 to 287.
Ruling: open split: see questions page Q-12.
Found: 2026-10-30, wave 2 scout R1.

### NL-19 A spaced .chart mix event is a disco marker in the game, not in Hydra

Rule: a .chart E text that starts with `[mix` or `mix` goes whole to the mix parser, so `E [mix 3 drums0d]` with ordinary spaces is a marker in the game; Hydra reads an E line as a marker only when it is one word.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 116 to 168 (the two starts-with tests) and 319 to 330 (the parser call); NL-18 for the parser. Hydra's check is `t.count == 2` in src/parse/song.cpp lines 1252 to 1259. The corpus file testdata/parse_edge/c20_disco.chart holds one spaced form; the library was not searched.
Game code (6142): RVA `0x213CAF0`, the listing above.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1252 to 1259.
Ruling: open split: see questions page Q-10.
Found: 2026-10-30, wave 1 scout H1 and wave 2 scout R1.

### NL-20 A disco span runs from a drumsNd event to the next six-letter drumsN event

Rule: a mix event whose text after the difficulty is 7 or more characters with d as its 7th starts a span; the span ends at the next mix event of the same difficulty whose text is exactly 6 characters and starts with "drums" (such as drums0), or two ticks after the last note. Only red and yellow notes in the span get the Disco flag.
Label: Proven in the game's code
Evidence: `code/6142/disco_apply_215CD50.asm` lines 98 to 116 (length and 7th character), 152 to 161 (same difficulty, next event), 220 to 223 (starts with drums, length 6), 243 to 255 (the range search) and 272 to 289 (types 14 and 15, the flag edit). The exact edge of the tick range lives in a helper that was not read.
Game code (6142): RVA `0x215CD50`, the listing above.
Hydra: `is_disco_on_marker` and `is_disco_off_marker` in src/parse/song.cpp lines 307 to 324; `emit_chord_timestamp` lines 419 to 421.
Ruling: D19 (record text): "Clone Hero's one-letter on/off rule (A+) is not adopted."
Found: 2026-10-30, wave 2 scout R1.

### NL-21 drums0dnoflip does not turn flip off in the game

Rule: in the game `drums0dnoflip` counts as a span start (its 7th character is d), adds only a no-flip flag to notes that are not already Disco, and is too long to end an open drums0d span, so an earlier drums0d keeps flipping through it. Hydra reads it as flip off on purpose. This replaces the older reading "it looks only at the 7th character".
Label: Proven in the game's code
Evidence: `code/6142/disco_apply_215CD50.asm` lines 105 and 107 (the 7th character and the length 13), 137 to 148 (the compare with "dnoflip"), 223 (the end needs length 6) and 282 to 289 (Disco set, or the 0x40 flag only when Disco is clear); decompile lines 48364 to 48476. The 2026-09-27 Marigold run (693,780 in the game against Hydra's 664,700) left no result file.
Game code (6142): RVA `0x215CD50`, the listing above.
Hydra: `is_disco_off_marker` in src/parse/song.cpp lines 318 to 324.
Ruling: deliberate difference: "this is a bug in clone hero, not an intentional feature. it will be fixed in a future update." (card 2026-09-28-1b0a7cd8, 03:43 UTC); D19: "`drums0dnoflip` stays off (Hydra does not copy Clone Hero's bug)."
Found: 2026-09-27, session 1b0a7cd8; re-read by wave 1 scout H2b and wave 2 scout R1.

### NL-22 Disco flip swaps red and yellow only with Pro Drums on

Rule: Hydra flips a disco section only under Pro Drums; which game function performs the flip, and whether plain Drums skips it, has not been found.
Label: Hydra assumes it, never checked
Evidence: the builder listing docs/audit/note-shuffle/step0/open2/f20d2ea0_build.asm lines 150 to 270 was read and no disco-bit test was found in it.
Game code (6142): not found: looked in the builder listing; candidates `0x20EF450` and `0x20F1420` were not read for the flip.
Hydra: `emit_chord_timestamp` in src/parse/song.cpp lines 418 to 427; `Chord::apply_disco_flip` in src/core/model.cpp line 211.
Ruling: D19 (record text): "The 'flip only with Pro Drums' gate moves to one place."
Found: this plan's wave 1 scout H2b.

### NL-23 How many charts carry drums0dnoflip

Rule: ADR 0023 says 48 library charts carry the marker; the memory note says 22 files contain it and 5 change score.
Label: Unknown
Evidence: unchecked by the scout: it would need a whole-library scan, which needs the user's permission. The two counts disagree.
Game code (6142): not found: data, not code.
Hydra: none.
Ruling: none found.
Found: 2026-09-27, session 1b0a7cd8.

## 2x kick and the Double Kick setting

### NL-24 In a .mid each difficulty's 2x kick is the pitch below its kick

Rule: the 2x kick is pitch 59 on Easy, 71 Medium, 83 Hard and 95 Expert; it gets the 2x flag and the kick's note type.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 316 to 324: the pitch minus 59 is tested against the mask 0x1001001001, whose bits 0, 12, 24 and 36 are those four pitches; docs/audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm lines 236 to 243 agree.
Game code (6142): RVA `0x2155050`, the listing above.
Hydra: `DifficultyChartCodes::kick2x_pitch` in src/parse/song.h line 61; `kDifficultyChartCodes` in src/parse/song.cpp lines 654 to 659.
Ruling: D20: chose "A: own pitch, box everywhere" (card 2026-10-04-bbf41dc4, 01:53 UTC).
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### NL-25 The .mid pitch-to-difficulty and pitch-to-pad maps

Rule: pitches 58 to 66 are Easy, 70 to 78 Medium, 82 to 90 Hard and 94 to 102 Expert; within a difficulty, counted from its 2x kick, +0 and +1 are kicks, +2 red, +3 yellow, +4 blue, +5 the fourth pad and +6 the fifth; other pitches in range give no note.
Label: Proven in the game's code
Evidence: `code/6142/pitch_difficulty_map_210D9C0.asm` lines 1 to 24; the type map in docs/audit/note-shuffle/step0/open2/f210d7b0.asm (its byte table decoded by the scout).
Game code (6142): RVAs `0x210D9C0` and `0x210D7B0`.
Hydra: `kDifficultyChartCodes` in src/parse/song.cpp lines 654 to 659; `MidiParser::optype` lines 905 to 909.
Ruling: none needed for the four pads; the fifth pad is NL-04.
Found: 2026-10-30, wave 2 scout R1.

### NL-26 A .chart N 32 is read on every difficulty with no gate

Rule: every difficulty section that holds an N 32 gets a 2x kick at read time; the Double Kick setting is applied later (NL-27).
Label: Proven in the game's code
Evidence: `code/6142/chart_n_handler_213D620.asm` lines 95 to 109 branch on the lane value only; decompile lines 25098 to 25110.
Game code (6142): RVA `0x213D620`, the listing above.
Hydra: `ChartParser::note_optype` case 32 in src/parse/song.cpp line 1699.
Ruling: D20 (as NL-24).
Found: 2026-10-03, session cebf3006; listed by wave 2 scout R1.

### NL-27 With Double Kick off, the game removes every 2x kick

Rule: the builder's only gate on 2x kicks is bit 2 of its modifier byte, at every difficulty; with the bit clear it calls a step that unlinks every note carrying the 2x-kick flag (0x100000) from its chord and removes it.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f20d2ea0_build.asm lines 231 to 236 (the bit test and the call); `code/6142/two_x_kick_drop_20EFF60.asm` lines 37 to 140 (the test call, the unlink at lines 46 to 93, the blanked slot at line 98, the RemoveAll at line 140); `code/6142/double_kick_test_214B850.asm` line 1 tests 0x100000 (DrumsDoubleKick in dump.cs line 548217). That bit 2 is the player's Double Kick setting comes from the live run in NL-28, not a listing.
Game code (6142): builder RVA `0x20D2EA0`; removal step RVA `0x20EFF60` of class #8278; test RVA `0x214B850`.
Hydra: `Chord::apply_2x_bass` in src/core/model.cpp line 280, called with `mode_bass2x_` in src/parse/song.cpp line 847.
Ruling: D20 (as NL-24); D105: chose "Match the game, test first (Recommended)" (card 2026-10-09-30beeb6e).
Found: 2026-10-09, session 30beeb6e; listed by wave 2 scout R9.

### NL-28 Live: a 2x kick on a normal kick's tick leaves no kick with Double Kick off

Rule: on songs F1 (.mid) and F2 (.chart), with 2x kick off the game showed a kick on chords 1 and 9 only and counted 11 notes; with it on, one kick on chords 1, 3, 5, 7 and 9 and 14 notes.
Label: Proven in a live run
Evidence: docs/audit/note-shuffle/step0/FINDINGS.md line 53 records the run of 2026-10-09; the recordings are C:\Users\Patrick\Videos\group 1.mp4 and group 2.mp4 (they exist today; the main session re-checked the F1 frames, card 30beeb6e). No scout of this plan viewed them. Hydra pins it in tests/test_kick2x.cpp line 131.
Game code (6142): builder RVA `0x20D2EA0` (NL-27).
Hydra: `Chord::add_kick` in src/core/model.cpp lines 263 to 276; `Chord::apply_2x_bass` line 280.
Ruling: D105: chose "Match the game, test first (Recommended)" (card 2026-10-09-30beeb6e).
Found: 2026-10-09, session 30beeb6e.

### NL-29 A 2x kick and a normal kick on one tick merge into one kick with every mark

Rule: both readers keep each kick as its own note; the track finalizer then merges notes of one tick and one type into the longest one (the first on a tie), or-ing every flag bit except the low three, so the merged kick carries the 2x bit and both kicks' ghost and accent bits.
Label: Proven in the game's code
Evidence: the readers: `code/6142/mid_drum_reader_2155050.asm` lines 341 to 354 and `code/6142/chart_n_handler_213D620.asm` lines 119 to 174 (one note per event). The merge: `code/6142/kick_merge_215B8C0.asm` lines 54 and 94 (same tick), 57 and 97 (same type), 68 and 69 (longest kept), 102 to 123 (the flag merge, low three bits at line 110). The finalizer docs/audit/note-shuffle/step0/open2/f215c320.asm lines 59 to 63 calls it.
Game code (6142): a method of the track class #8661, RVA `0x215B8C0`, the listing above.
Hydra: `Chord::add_kick` in src/core/model.cpp line 263.
Ruling: D105: chose "Match the game, test first (Recommended)" (card 2026-10-09-30beeb6e); "go with the games rule" and "that is the source of truth here" (card 2026-10-09-27b9bbc4).
Found: 2026-10-09, session 30beeb6e; listed by wave 2 scout R9.

### NL-30 The merged kick does not depend on which kick comes first in the file

Rule: Hydra merges a 2x kick and a normal kick on one tick in either file order; the game's finalizer or-s flags whatever the order, but whether songs F1 and F2 held both orders was not checked.
Label: Unknown
Evidence: unchecked by the scout: it would need a run with both file orders. Hydra's side is src/core/model.cpp lines 272 to 275.
Game code (6142): RVA `0x215B8C0` (NL-29).
Hydra: `Chord::add_kick` in src/core/model.cpp lines 263 to 276.
Ruling: D105 (as NL-29).
Found: this plan's wave 1 scout H2b.

### NL-31 The kick key scores a 2x-kick gem, and the Slipping layout

Rule: pressing the kick key scores a 2x-kick gem; in Synovial's Slipping `notes.mid` the 2x kick is its own pitch (302 gems on 95 against 915 on 96, never on one tick).
Label: Unknown
Evidence: unchecked by the scout: the 2026-10-29 run's `scorestats.json` (tools/ch_probe/experiments/results/overlay-spike/run1_cal0_scorestats.json) shows DoubleKick on and 1,257 of 1,257 notes hit, but the press log has no kick-type column, and the Slipping counts come from a transcript, not a recount.
Game code (6142): not found: not read.
Hydra: `mop_kick` in src/parse/song.cpp line 912.
Ruling: none found.
Found: 2026-09-25, session 8157f2dc.

## Chord order and merged notes

### NL-32 Two pads of one colour on one tick merge in the game; Hydra drops the second

Rule: the merge in NL-29 applies to every note type, so two red notes on one tick become one red note holding both notes' flags; Hydra's second `Chord::add_note` throws "Duplicate note." and the per-op catch drops that note with its marks.
Label: Proven in the game's code
Evidence: `code/6142/kick_merge_215B8C0.asm` lines 54 to 128 compare tick and type only, with no kick test; docs/audit/note-shuffle/step0/open2/f215b8c0.asm lines 38 to 128 agree. Hydra: src/core/model.cpp line 244 throws, and the catches in src/parse/song.cpp lines 1019 and 1746 drop the op (checked 2026-10-30).
Game code (6142): RVA `0x215B8C0`, the listing above.
Hydra: `Chord::add_note` in src/core/model.cpp line 242.
Ruling: open split: see questions page Q-14.
Found: this plan's wave 1 scout H2a; Hydra side checked by writer W1.

### NL-33 In a .mid, a tick's notes are ordered kick, red, yellow, blue, green

Rule: before any builder step the game orders each tick's notes by note type (kick, red, yellow, blue, green); a cymbal is a flag, not a slot, so a yellow cymbal sits in the yellow slot, and file order is discarded.
Label: Proven in a live run
Evidence: docs/audit/note-shuffle/step0/FINDINGS.md line 55 records song B (.mid) matching the colour-order row with Note Shuffle on, 2026-10-09 (C:\Users\Patrick\Videos\group 3.mp4, not viewed by this plan's scouts). The sort's compares are in docs/audit/note-shuffle/step0/open2/f215c040.asm lines 64, 65 and 143 to 149; its direction was not traced.
Game code (6142): RVAs `0x215C040`, `0x215E020` and `0x215C320` (the sort was not traced in full).
Hydra: `NoteColor` order in src/core/model.h line 55; `Chord::note_list` in src/core/model.cpp lines 155 to 158.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e.

### NL-34 A .chart's tick order is the same as a .mid's

Rule: both parsers end in the same track finalizer, so a .chart's within-tick order should equal the .mid's; no .chart was played to check it.
Label: Unknown
Evidence: unchecked by the scout: it would need the two call sites of the finalizer `0x215C320` opened (from `0x2155050` and `0x2161A80`) and a .chart chord-order run. FINDINGS.md lines 61 to 63 say "code reading only".
Game code (6142): RVAs `0x215C320`, `0x2155050` and `0x2161A80`; call sites not opened.
Hydra: `Chord::note_list` in src/core/model.cpp line 155.
Ruling: none needed.
Found: 2026-10-09, session 30beeb6e.

### NL-35 Every chord a chart holds is stored and scored, three-pad chords included

Rule: Hydra never decides a chord is unplayable; the game's handling of three pads at once was not read.
Label: Hydra assumes it, never checked
Evidence: `Chord::code` in src/core/model.cpp line 107; ADR 0015.
Game code (6142): not found: not read.
Hydra: `Chord::code` in src/core/model.cpp line 107.
Ruling: "if it exists hydra should score it" (card 9405caaa, 03:07 UTC).
Found: session 9405caaa (Hot Sexy Girls, 21 three-pad chords).

### NL-36 The .chart note record and its length field

Rule: the .chart N handler stores tick, length, type, flags and list index, and keeps the line's length only when it exceeds an int read from another object (else 0); what that object is was not found.
Label: Unknown
Evidence: `code/6142/chart_n_handler_213D620.asm` lines 119 to 148 (the conditional move); the object at track +0x50 and its +0x114 field were not identified in dump.cs.
Game code (6142): RVA `0x213D620`, lines 119 to 148.
Hydra: no owner: Hydra's drum chart keeps no note length this feeds.
Ruling: none found.
Found: 2026-10-30, wave 2 scout R1.

## Flams

### NL-37 A flam copies a single-pad chord's note one pad up

Rule: a .mid flam marker (pitch 109) on a chord with exactly one pad (a kick may be with it) adds a copy one pad up, right after it, and the copy counts toward the shuffle seed; for green, Hydra keeps green and copies blue, while the game moves the source to blue and copies green, which gives the same pair.
Label: Unknown
Evidence: unchecked by the scout: only the first 50 lines of the flam step in docs/audit/note-shuffle/step0/open2/drum_pretransforms_20ef450_20eff60_20f0320_20f1420_20f16e0_20efee0.asm were read, not the single-pad test or the insert. FINDINGS.md line 21 says the pads agree.
Game code (6142): RVA `0x20EF450` (or `0x20F0320`, which FINDINGS names); the insert was not read.
Hydra: `Chord::apply_flam_conversion` in src/core/model.cpp lines 228 to 239; `kFlamMarkerPitch` in src/parse/song.cpp line 679.
Ruling: none found.
Found: 2026-10-09, session 30beeb6e.

### NL-38 What a flam copy does with a cymbal flag or a ghost or accent mark

Rule: Hydra copies the source note whole, so a green cymbal flam gives a blue cymbal and a green cymbal; the game's flag handling of the pair is unread.
Label: Unknown
Evidence: looked in FINDINGS.md line 21 and docs/audit/note-shuffle/step0/open2/README.md line 20; the flam step's flag handling was not read.
Game code (6142): RVA `0x20EF450`; flag handling not read.
Hydra: `Chord::apply_flam_conversion` in src/core/model.cpp lines 228 to 239.
Ruling: none found.
Found: this plan's wave 1 scout H2a.

### NL-39 Whether the game can flam a .chart note

Rule: Hydra never flams a .chart, which has no flam marker; the game's flam step runs on the builder's note list for any format, and where its trigger flag comes from in a .chart is unread.
Label: Unknown
Evidence: the step is called for any format (docs/audit/note-shuffle/step0/open34/builder_20d2ea0.asm lines 228 to 230); the .chart modifier map has no flam note number (decompile lines 24750 to 24825), but a flag source in `0x215E280` was not traced.
Game code (6142): RVAs `0x20EF450` and `0x215E280`; the flag source was not read.
Hydra: `emit_chord_timestamp` in src/parse/song.cpp lines 418 to 420 flams only on the .mid path.
Ruling: none found.
Found: this plan's wave 1 scouts H2a and H2b.

## Note Shuffle

The algorithm and its tests are written up in docs/audit/note-shuffle/step0/FINDINGS.md; the rows below state each rule once with its listing.

### NL-40 The shuffle's seed is the first four notes' lane masks times their ticks

Rule: the seed is the sum, over the first min(4, count) notes of the track's note list, of lane mask times tick, wrapping at 64 bits; kicks count and no lane is filtered. The generator starts with s0 = seed shifted left 3 and s1 = seed shifted right 3.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 46 to 99: the count cap of 4 at lines 61 to 76 and 87 to 89, the multiply at lines 84 and 85, the two shifts at lines 96 to 99.
Game code (6142): RVA `0x20F0C30`, the listing above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 134 to 137; the `LaneGenerator` constructor line 67.
Ruling: none needed: same rule.
Found: 2026-10-09, session 6c19d0ef.

### NL-41 Each lane is drawn with one xorshift128+ step

Rule: one xorshift128+ step gives s0 + s1; its low 31 bits are scaled by 2^-31 times (N - 1), cut to an integer, plus 1, and that index picks the pad bit. Hydra's integer form (r times (N - 1)) shifted right 31 gives the same index, because the product stays below 2^34 and is exact in a double.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/lanedraw_20f0b30.asm lines 12 to 52 (the shifts 23, 17 and 26, the 31-bit mask at line 31, the constant at line 10, the increment and shift at lines 49 to 51). The integer equivalence is the scout's reasoning, not a test.
Game code (6142): RVA `0x20F0B30`, the listing above.
Hydra: `LaneGenerator::next_lane_index` in src/parse/note_shuffle.cpp lines 69 to 78.
Ruling: none needed: same rule.
Found: 2026-10-09, session 6c19d0ef.

### NL-42 Which pads the shuffle draws from

Rule: the shuffle draws from pads 1 to 7 (N = 8) with Pro Drums and from pads 1 to 4 (N = 5) on plain Drums, and never draws the kick.
Label: Unknown
Evidence: the six N targets are in docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 117 to 135, but the two jump tables that pick a target by instrument (at 0x20F11CC and 0x20F1208) show only as raw bytes at lines 350 and 351; their decode survives only as prose in docs/audit/note-shuffle/step0/agent-results.json line 300. Pro Drums matches the filmed runs; plain Drums was never filmed (tests/test_note_shuffle.cpp lines 98 to 100).
Game code (6142): RVA `0x20F0C30`; the jump-table decode is lost (read 2026-10-09, session 30beeb6e).
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 131 and 132.
Ruling: none found.
Found: 2026-10-09, session 30beeb6e.

### NL-43 The shuffle runs last among the drum steps, only with Note Shuffle on

Rule: the shuffle runs only when modifier bit 0x40 is set, after the flam step, the 2x-kick removal (when bit 2 is clear), the cymbal fold or Pro step, and the dynamics clear; so each drum setting gets its own shuffled chart.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/builder_20d2ea0.asm lines 153 to 162 (instruments 6 and 9 take the drum path) and 228 to 259 (the step calls in order, then the 0x40 test and the call at 0x20D3315). The names of the steps before the shuffle are the 2026-10-09 scouts' (open2/README.md lines 9 and 24).
Game code (6142): builder RVA `0x20D2EA0`, the listing above.
Hydra: `apply_shuffle_switch` in src/parse/song.cpp lines 1931 to 1935 and `finish_loaded_song` lines 1943 to 1946, after `emit_chord_timestamp` and `Chord::apply_2x_bass`.
Ruling: D104 item 3 (record text): "Pro Drums off gets its own row too."
Found: 2026-10-09, session 6c19d0ef.

### NL-44 Kicks never move and are skipped by the draw

Rule: a note whose mask holds the kick bit loops straight to the next note; the shuffle writes no kick.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 141 to 149 (the excluded bit) and 181 to 185 (the skip).
Game code (6142): RVA `0x20F0C30`, the listing above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 145 and 185.
Ruling: none needed: same rule.
Found: 2026-10-09, session 6c19d0ef.

### NL-45 A drawn pad is blocked when its colour is already used

Rule: a drawn pad is redrawn when its colour, as tom or cymbal, is already used: for the first note of a chord by the previous chord's output, for later notes by the earlier notes of the same chord.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 298 to 311 build the blocked mask from the used colours and test the drawn bit. The reset of that mask at each new chord (lines 270 to 279 and 312 to 316) was read but not traced variable by variable, so the "previous chord's output" half is unchecked.
Game code (6142): RVA `0x20F0C30`, lines 270 to 316.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 161 and 170 to 177.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e.

### NL-46 A chord that repeats the pattern two chords back copies its pads

Rule: when a chord's pad pattern equals the pattern two chords back, its first note copies the pad of the same rank in that chord's output; a later note in the chord compares with the chord one back.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 209 to 284 (the same-tick test, the two pattern slots, the rank search through two bit helpers, the slot rotation). The helper bodies were not opened. A probe that drops the rule matches 5 of 22 filmed ticks (card 2026-10-09-cfb6f1fa).
Game code (6142): RVA `0x20F0C30`; helpers `0x20FC610` and `0x20FC5F0`.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 154 to 159 and 173 to 176.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e.

### NL-47 The redraw loop has no cap, so two cases freeze the game

Rule: the redraw loop repeats until the drawn pad is free, with no counter, timer or fallback. So a four-colour chord followed by a chord whose first note has no copy loops forever, and so does a zero generator state (the first four notes all on tick 0), which draws red every time.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 285 to 315 (the loop and its only exits) and docs/audit/note-shuffle/step0/open34/lanedraw_20f0b30.asm lines 12 to 52 (a zero state stays zero and gives index 1). The 2026-10-09 runs C1 and D1 hung as the song loaded (FINDINGS.md line 57; video group 4.mp4, not viewed by this plan's scouts).
Game code (6142): RVAs `0x20F0C30` and `0x20F0B30`, the listings above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 162 and 163 return the freeze, shown as `ErrorKind::NoteShuffleFreezes`; `LaneGenerator::state_is_zero` line 81.
Ruling: D104 item 5 (record text): "A chart the game can't shuffle gets no path."
Found: 2026-10-09, session 30beeb6e.

### NL-48 From any non-zero state the redraw loop ends

Rule: the generator reaches every pad from any non-zero state, so the loop ends whenever a pad is free.
Label: Unknown
Evidence: unchecked by the scout: it would need docs/audit/note-shuffle/probes/reach/reach_check.py run (its header says it proves the full period and measured a worst case of 94 draws).
Game code (6142): RVA `0x20F0B30`.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 164 to 168 (no cap, by design).
Ruling: none found.
Found: 2026-10-09, session 30beeb6e.

### NL-49 The shuffle writes only the lane mask, so marks stay with the note

Rule: the only store into a note is its lane mask; the Star Power, solo, accent and ghost flags stay put, so a moved note keeps its mark.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm line 317 is the only store through the note pointer in the whole listing.
Game code (6142): RVA `0x20F0C30`, line 317.
Hydra: `moved_to` in src/parse/note_shuffle.cpp lines 41 to 55.
Ruling: none needed: same rule.
Found: 2026-10-09, session 6c19d0ef.

### NL-50 The drum lane bits of a play note

Rule: the lane mask bits are kick 0x1, red 0x2, yellow tom 0x4, blue tom 0x8, green tom 0x10, yellow cymbal 0x20, blue cymbal 0x40 and green cymbal 0x80.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f20d36a0_lanemap.asm lines 45 to 76 load the eight values in note-type order; the type-to-case mapping at lines 10 and 35 was not traced.
Game code (6142): RVA `0x20D36A0`, the listing above.
Hydra: `lane_bit` in src/parse/note_shuffle.cpp lines 26 to 35.
Ruling: none needed: same rule.
Found: 2026-10-09, session 6c19d0ef.

### NL-51 The star base is taken after the shuffle

Rule: the star cutoffs are computed from the shuffled lane masks, so they count the cymbals the shuffle creates.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/starbase_20f4680.asm line 105 reads the lane mask; engine_ctor_20f79e0.asm line 196 calls it, and baseplayer_208870.asm lines 55 and 103 call the builder before the engine. "Only direct caller" rests on a 2026-10-09 scan not re-run. The cymbal bonus inside the star base is page 6's.
Game code (6142): RVAs `0x20F4680`, `0x20F79E0` and `0x208870`, the listings above.
Hydra: `finish_loaded_song` in src/parse/song.cpp line 1944; `Path::chart_base_score` in src/core/model.h line 846.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e.

### NL-52 The filmed Note Shuffle runs of 2026-10-09

Rule: songs A1 (.chart, 192 ticks per beat) and A2 (.mid, 960) showed their own resolution's row; songs C2 and D2 played and matched; song E scored 19,925 with 5 stars on 106 notes.
Label: Unknown
Evidence: unchecked by the scout: the videos C:\Users\Patrick\Videos\group 3.mp4 and group 4.mp4 exist but were read only through FINDINGS.md lines 55 to 57. Hydra pins the predicted rows in tests/test_note_shuffle.cpp lines 102 to 125.
Game code (6142): not found: live runs, not code.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp.
Ruling: none found.
Found: 2026-10-09, session 30beeb6e.

## Lanes

### NL-53 A .mid lane is pitch 126 or 127, and its velocity picks the lowest difficulty

Rule: a .mid lane is a note-on of pitch 126 (single) or 127 (double) on the drum track; its velocity names the lowest difficulty that shows it, by compares at 21, 31, 41 and 51.
Label: Proven in the game's code
Evidence: docs/audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm lines 85 and 118 (the two pitches) and 93 to 99 and 126 to 132 (the four velocity compares). Which compare is Easy, Medium or Hard is D112's text; the branch targets were not traced.
Game code (6142): RVA `0x2155050`, the lane case.
Hydra: `kSingleLaneMarkerPitch` and `kDoubleLaneMarkerPitch` in src/parse/song.cpp lines 688 and 689.
Ruling: D112: "go with clone hero" (card 2026-10-11-14ef4882, 00:36 UTC).
Found: 2026-10-10, D112.

### NL-54 A .chart lane is S 65 or S 66

Rule: a .chart lane is special phrase S 65 (single) or S 66 (double) in a difficulty's section.
Label: Unknown
Evidence: unchecked by the scout: docs/audit/drum-lanes/spec_213e0c0.asm holds no compare with 0x41 or 0x42 (probably a jump table) and was not read.
Game code (6142): RVA `0x213E0C0`, not read.
Hydra: the lane ops in src/parse/song.cpp line 589.
Ruling: D112 (as NL-53).
Found: 2026-10-10, D112.

### NL-55 Which pads a lane lights

Rule: a single lane lights the lowest pad of the first chord with a non-kick note, from one tick before the lane to its end, in the order red, yellow tom, blue tom, green tom, yellow cymbal, blue cymbal, green cymbal; a double lane adds the highest pad of a second chord.
Label: Unknown
Evidence: unchecked by the scout: it would need docs/audit/drum-lanes/f20d12c0.asm, f20d5310.asm and f20d51d0.asm read; the rule is D112 item 3's text.
Game code (6142): RVAs `0x20D12C0`, `0x20D5310` and `0x20D51D0`, listings not read.
Hydra: `light_drum_lanes` in src/parse/song.cpp line 192; `lights_lane_pad` line 77.
Ruling: D112: "go with clone hero" (card 2026-10-11-14ef4882, 00:36 UTC).
Found: 2026-10-10, D112.

### NL-56 Only the lane builder and the drum engine read the lane flags

Rule: the flag bits 0x4000 (in a lane), 0x8000 (double) and 0x10000 (last note) are set by the lane builder and read only by the drum engine.
Label: Unknown
Evidence: docs/audit/drum-lanes/FINDINGS-play-time.md line 15 says its instruction scan "cannot see a flag word loaded into a register and masked later, so 'only these' is strong but not proven".
Game code (6142): RVA `0x20D12C0` sets them; exclusivity not proven.
Hydra: none: Hydra reads no lane flag in scoring.
Ruling: "lanes only affect how lenient the engine is, it doesnt change the underlying notes." (card 2026-10-11-27bb17bc, 03:52 UTC).
Found: 2026-10-10, docs/audit/drum-lanes.

### NL-57 With Note Shuffle on, how the game draws a lane

Rule: the user saw the game draw only the blue column for a lane on Glide with Note Shuffle on; Hydra's Preview lights every pad the lane's notes land on, on purpose. The game's lane-draw routine has never been read.
Label: Unknown
Evidence: the user's report only; D112's record says "The routine that draws a lane was not read".
Game code (6142): not found: looked in docs/audit/drum-lanes (none of its listings is the draw routine).
Hydra: `light_drum_lanes` in src/parse/song.cpp lines 194 to 202, called at lines 1945 and 1946.
Ruling: deliberate difference, D254: "clone hero only draws the blue. this will be an intentional deviation from the game. it looks better with both."; items "Every pad its notes use." and "Only with Note Shuffle on."
Found: 2026-10-17, session 65b16299.

### NL-58 The game drops a lane whose first note starts too late

Rule: the game drops a lane whose first note lands more than resolution times 12 divided by a value set at run time after the lane's start; Hydra counts any note inside the lane.
Label: Unknown
Evidence: the run-time value was never read (D112 item 4).
Game code (6142): not found: the divisor is a run-time static with no listing.
Hydra: `light_drum_lanes` in src/parse/song.cpp line 192.
Ruling: D112: chose "Any note inside the lane (Recommended)" (card 2026-10-11-14ef4882, 00:34 UTC).
Found: 2026-10-10, D112.

### NL-59 A double lane's 0.75-second timer

Rule: for a player who hits every note, lanes change no score, except a double lane's timer (a round of both pads within 0.75 s), which Hydra does not model; no Expert double lane in the library trips it.
Label: Unknown
Evidence: unchecked by the scout: docs/audit/drum-lanes/FINDINGS-play-time.md lines 5 to 9 state it; the play-time listings and docs/audit/drum-lanes/play/library_double_lane_breaks.csv were not opened.
Game code (6142): RVAs `0x20DCAD0` and `0x20DEAA0`, listings not read.
Hydra: no owner: lanes are display-only in Hydra.
Ruling: "lanes only affect how lenient the engine is, it doesnt change the underlying notes." and "if not then it doesnt matter" (card 2026-10-11-27bb17bc, 03:52 and 03:55 UTC).
Found: 2026-10-10, docs/audit/drum-lanes.

### NL-60 An activated fill lights only its activation note's lane

Rule: Hydra's Preview lights only the lane of a fill's activation note, as the user's screenshot of the game showed; no code read shows the game's lighting.
Label: Hydra assumes it, never checked
Evidence: docs/audit/drum-lanes/FINDINGS-play-time.md line 50 says the activation flag "is not a lane flag"; the screenshot was not saved.
Game code (6142): not found.
Hydra: the activation lane in src/app/preview_view.cpp line 428, from `Chord::activation_note`.
Ruling: "match the clone hero style of highlighting only the lane the activation note is on" (card ee38a528, 20:32 UTC).
Found: session ee38a528.

## What plain Drums drops

### NL-61 On plain Drums the builder folds cymbals and clears dynamics

Rule: when the instrument is not 9 (Pro Drums) the builder runs the cymbal fold and then the dynamics clear; Pro Drums runs a different step instead.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2-refute/f20D2EA0.asm lines 245 to 253 (the compare with 9 and the two calls); the scout's caller scan finds no other call site for either step.
Game code (6142): builder RVA `0x20D2EA0`; fold `0x20F16E0`; clear `0x20EFEE0`; Pro step `0x20F1420`.
Hydra: `ChartReading::prodrums` in src/parse/chart_reading.h line 24, read in src/parse/song.cpp line 1955.
Ruling: none needed for the trigger; the clear is NL-62.
Found: 2026-10-28, session 6a549de0; listed by wave 2 scout R2.

### NL-62 The plain-Drums clear removes every ghost and accent mark

Rule: the clear ands every note's flag word with 0xFFF3FFFF, which clears DrumsGhostHit (0x40000) and DrumsAccentHit (0x80000), and does nothing else; Hydra keeps scoring dynamics on plain Drums.
Label: Proven in the game's code
Evidence: `code/6142/plain_drums_20EFEE0.asm` lines 17 to 28 (the loop) and line 25 (the and); the flag names are dump.cs lines 548215 and 548216. This settles wave 2 scout R2's unchecked row on which bit is which.
Game code (6142): a static method of class #8278, RVA `0x20EFEE0`, the listing above.
Hydra: no owner for the drop: nothing in src/parse/song.cpp or src/core/model.cpp clears dynamics by instrument.
Ruling: deliberate difference: "there is no way this is intentional, it must be a bug in clone hero" (01:30 UTC) and "Keep Hydra as is" (01:44 UTC), card 2026-10-29-6a549de0.
Found: 2026-10-28, session 6a549de0; listed by wave 2 scouts R2 and R9.

### NL-63 The cleared bits are the ones the hit handler counts

Rule: the drum hit handler tests the ghost bit and then the accent bit and bumps one of two engine counters, so a cleared mark is never counted.
Label: Proven in the game's code
Evidence: docs/audit/drum-lanes/f20de770.asm lines 62 to 102 (the two getter calls at lines 64 to 69, the counter bumps at lines 82 and 101); docs/audit/note-shuffle/scout-flag/note_getters.txt lines 8 and 29 (the getters test 0x40000 and 0x80000).
Game code (6142): RVA `0x20DE770` (`RVA_NOTE_DYNAMICS` in `constants.py`), the listing above.
Hydra: none.
Ruling: none needed.
Found: 2026-10-30, wave 2 scout R2.

### NL-64 Live: plain Drums scored no accents or ghosts on Trippin SP Spike

Rule: a bot on plain Drums over Trippin SP Spike (9 accents, 34 ghosts) counted 0 of each and scored 248,550; on Pro Drums it hit 9 and 34 and scored 290,975, Hydra's Pro Drums score.
Label: Proven in a live run
Evidence: the result files hydra-data/ch-mcp-runs/2026-10-28/l1/bot/scorestats-Trippin_SP_Spike-20261028-210052.json (Drums) and -212754.json (ProDrums), both outside the repo, run 2026-10-28.
Game code (6142): RVA `0x20EFEE0` (NL-62).
Hydra: `ChartReading::prodrums` in src/parse/chart_reading.h line 24.
Ruling: as NL-62.
Found: 2026-10-28, session 6a549de0.

### NL-65 The same clear applies to a .mid on plain Drums

Rule: both readers feed the same builder, so a .mid should lose its dynamics on plain Drums too; this was read from the code, not run.
Label: Unknown
Evidence: unchecked by the scout: it would need the builder's callers traced for the source format, or a .mid bot run on plain Drums.
Game code (6142): RVAs `0x20D2EA0` and `0x215C320`.
Hydra: as NL-62.
Ruling: as NL-62.
Found: 2026-10-28, session 6a549de0.

### NL-66 On plain Drums every cymbal becomes its tom

Rule: the fold shifts the lane mask right by 3 for any note with a cymbal bit, so each cymbal lands on its tom; Hydra counts no cymbal unless Pro Drums is on.
Label: Proven in the game's code
Evidence: `code/6142/fold_20F16E0.asm` lines 25 and 26 (the cymbal-bit test) and 33 to 35 (the shift and store).
Game code (6142): RVA `0x20F16E0`, the listing above.
Hydra: `cymbal_counts` in src/core/song.cpp line 73 returns true only when Pro Drums is on.
Ruling: none needed: same rule.
Found: 2026-10-30, wave 2 scout R2; Hydra side checked by writer W1.

### NL-67 Keyboard presses never register accents or ghosts

Rule: a key press scores accents and ghosts as plain notes; the bot's presses register them.
Label: Proven in a live run
Evidence: tools/ch_probe/experiments/results/overlay-spike/run1_cal0_scorestats.json (2026-10-29, Pro Drums, keyboard): accents 0 of 9, ghosts 0 of 34, notes 1,257 of 1,257. One run. The mechanism (a per-lane velocity compared with a threshold) is unchecked: docs/audit/drum-lanes/f20de770.asm was not read for it.
Game code (6142): not found: the press-velocity path was not read.
Hydra: none: Hydra scores a perfect player.
Ruling: none found.
Found: 2026-10-28, session 6a549de0.

## Unmodelled modifiers

### NL-68 The drum build calls three more modifier steps after the shuffle

Rule: after the shuffle the builder calls `0x20F1900` when modifier bit 0x20 is set, `0x20F1250` for bit 4 and `0x20F06A0` for bit 8; Hydra models only Double Kick and Note Shuffle.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/builder_20d2ea0.asm lines 260 to 277; docs/audit/note-shuffle/step0/open2/f20d2ea0_build.asm lines 163 to 168 show a second call to `0x20F1900` on the non-drum path.
Game code (6142): builder RVA `0x20D2EA0`, the listings above.
Hydra: no owner: src/parse and src/core hold only `mode_bass2x_` and the Note Shuffle switch.
Ruling: none found.
Found: 2026-10-08, the Note Shuffle plan (line 51).

### NL-69 What those three modifier steps do

Rule: which modifier each bit is and what each step does to the notes is not known.
Label: Unknown
Evidence: no committed listing holds a body of `0x20F1900`, `0x20F1250` or `0x20F06A0`, and none is in the 2026-09-17 decompile.
Game code (6142): not found: looked in docs/audit/note-shuffle, docs/audit/drum-lanes and the decompile.
Hydra: no owner.
Ruling: none found.
Found: this plan's wave 1 scouts H2a and H2b.

## Rows waiting on a live run

NL-12, NL-13, NL-14, NL-15, NL-16, NL-19, NL-22, NL-30, NL-32, NL-34, NL-37, NL-38, NL-39, NL-42, NL-57, NL-58, NL-65, NL-69. Each is listed on the live-checks page.

## Open splits

NL-04 (Q-13), NL-18 (Q-12), NL-19 (Q-10), NL-32 (Q-14). Each is written up on the questions page.
