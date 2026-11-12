# Building the note list [NL]

This page covers how Clone Hero turns the chart's drum notes into the note list it scores.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-11-11.

## Cymbals and toms

### NL-01 In a .mid, red is a tom and yellow, blue and green are cymbals unless a tom marker covers them

Rule: the .mid drum reader gives red the Tom flag, the kick neither flag, and yellow, blue and green the Cymbal flag; tom markers on pitches 110, 111 and 112 then turn yellow, blue and green into toms.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 334 to 340 (red gets 0x10, the kick is skipped, any other pad gets 0x20) and lines 244 to 294 (pitches 110 to 112 build Tom modifier records on yellow, blue and green). How far a marker reaches (Hydra holds it from note-on to note-off) was not read in the game, and the 2026-09-25 play run that settled "a marked gem is a tom" left no result file.
Game code (6142): the .mid drum reader, a method of class #8626, RVA `0x2155050`, the listing above.
Hydra: `kYellowTomMarkerPitch`, `kBlueTomMarkerPitch` and `kGreenTomMarkerPitch` in src/parse/song.cpp lines 708 to 710; `flag_cymbals_` set at lines 1140 to 1145.
Ruling: none needed: same rule.
Found: 2026-09-25, session 8157f2dc; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-02 The .mid marker pitches the drum reader dispatches

Rule: pitch 109 makes a Flam modifier, pitches 110 to 112 a Tom modifier (raising track flag +0xA2), and pitches 113 to 115 fall through and are skipped; 103, 116, 120, 126 and 127 go to four other sinks.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 151 to 243 (the compares and the jump table at 0x2155B2C) and 244 to 294 (the flam and tom records). The four sinks' bodies were not read.
Game code (6142): RVA `0x2155050`, lines 151 to 294.
Hydra: `kFlamMarkerPitch` to `kDoubleLaneMarkerPitch` in src/parse/song.cpp lines 707 to 718; the `MidiParser::optype` switch at lines 941 to 965.
Ruling: none needed: same pitches.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### NL-03 What each .chart N number is on drums

Rule: N 0 is the kick; N 1 to 4 are red, yellow, blue and the fourth pad (Hydra's green), each a tom; N 32 is a kick with the 2x flag; any other N value is looked up as a modifier.
Label: Proven in the game's code
Evidence: `code/6142/chart_n_handler_213D620.asm` lines 71 to 109 (the drum table, decoded through the jump table at 0x213DB10) and lines 95 to 107 (N 32); decompile lines 25098 to 25110 agree.
Game code (6142): the .chart N handler of class #8615, RVA `0x213D620`, the listing above.
Hydra: `ChartParser::note_optype` in src/parse/song.cpp lines 1744 to 1771; `chart_pad_color` at line 1729.
Ruling: none needed: same rule for N 0 to 4 and N 32.
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-04 The game reads a fifth drum pad; Hydra drops it

Rule: the game reads .chart N 5 and .mid pitches 65, 77, 89 and 101 as a fifth pad (note type 17) and raises track flag +0xA1, and reads N 38 and N 44 as the accent and ghost of that pad; Hydra knows four pads and drops all of these. What the game does with a type-17 note in four-lane Pro Drums is unread.
Label: Proven in the game's code
Evidence: `code/6142/chart_n_handler_213D620.asm` lines 99 to 105 (type 0x11 raises +0xA1); `code/6142/pitch_difficulty_map_210D9C0.asm` lines 1 to 24 with the type map in docs/audit/note-shuffle/step0/open2/f210d7b0.asm; `code/6142/chart_modifier_decode_213D160.asm` lines 11 to 80 (N 38 and N 44 on that lane). How many library charts hold such lines was not counted (no library run).
Game code (6142): RVAs `0x213D620`, `0x210D9C0`, `0x210D7B0` and `0x213D160`, the listings above.
Hydra: `chart_pad_color` in src/parse/song.cpp lines 1729 to 1737 (pads 1 to 4) and `ChartParser::note_optype` lines 1761 to 1768 (34 to 37 and 40 to 43 only).
Ruling: open split: see questions page Q-13.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### NL-05 The .chart modifiers: accents, ghosts and cymbals

Rule: N 34 to 38 make an accent and N 40 to 44 a ghost on red, yellow, blue, the fourth and the fifth pad; N 66, 67 and 68 make a cymbal on yellow, blue and the fourth pad and raise track flag +0xA2; N 39, N 45 to 65 and N 69 up give nothing. Each modifier starts and ends at its own tick, and no tag gates it.
Label: Proven in the game's code
Evidence: `code/6142/chart_modifier_decode_213D160.asm` lines 11 to 80; `code/6142/chart_n_handler_213D620.asm` lines 212 to 278 (the call, the cymbal kind raising +0xA2, the record written with the tick twice); decompile lines 24747 to 24840.
Game code (6142): RVAs `0x213D160` and `0x213D620`, the listings above.
Hydra: `ChartParser::note_optype` in src/parse/song.cpp lines 1744 to 1770, with `kChartCymbalMarkerBase`; `allows_cymbals` in src/core/model.cpp line 18.
Ruling: none needed: same rule for the four pads (the fifth is NL-04).
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-06 A .chart applies its ghost and accent marks with no tag

Rule: the .chart finish step applies the cymbal, tom, flam, ghost and accent modifiers with no condition, so a .chart always has dynamics on.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f215e280.asm lines 23 to 45: five unconditional calls to the apply routine with kinds 7, 6, 8, 10 and 9. That the apply routine itself, docs/audit/note-shuffle/step0/open2/f215dbd0.asm, reads no dynamics flag was read on 2026-10-03 and not re-read since. Three other functions hold the string `ENABLE_CHART_DYNAMICS` (`0x211BAD0`, `0x211C270` and `0x211E4C0`); they take a whole string and look like a writer, not the .chart reader, and nobody has opened them (ch-evidence.md question 320 at commit 5fbe71ae).
Game code (6142): RVA `0x215E280` calling `0x215DBD0`, the listings above.
Hydra: the ghost and accent cases of `ChartParser::note_optype` in src/parse/song.cpp lines 1761 to 1768, and the comment at lines 1950 to 1954.
Ruling: none needed: same rule.
Found: 2026-10-03, session cebf3006; re-read 2026-10-30 in session b02ab3e6 (the reference build).

### NL-07 A .chart marker with no note at its tick is skipped

Rule: a cymbal, ghost or accent marker whose tick holds no note is skipped, and the chart still loads.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f215dbd0.asm lines 104 to 110: the range lookup returns -1 and jumps to the exit.
Game code (6142): RVA `0x215DBD0`, lines 104 to 110.
Hydra: `Chord::apply_cymbal` and the ghost and accent appliers throw `ChartFileError`, which the per-op catch drops (src/core/model.cpp lines 288 to 305; src/parse/song.cpp line 1807).
Ruling: none needed: same rule.
Found: 2026-10-03, session cebf3006; re-read 2026-10-30 in session b02ab3e6 (the reference build).

### NL-08 A .chart marker on a chord with no note of its colour

Rule: a marker on a tick whose notes are all of other colours sets nothing, in the game and in Hydra. The game works in two steps. First it adds the marker to every note in the marker's tick range, whatever the note's colour. Then a second pass keeps the marker's flag on a note only when the marker's +0x14 field equals the note's +0x20 field.
Label: Proven in the game's code
Evidence: a .chart N line makes a one-tick marker: docs/audit/note-shuffle/step0/open2/f213d620_chart_N_line.asm lines 253 and 254 store one register as both ends. The apply routine adds the marker to every note in the range it finds, with no colour test, in docs/audit/note-shuffle/step0/open2/f215dbd0.asm lines 117 to 135. The flag pass is docs/audit/note-shuffle/step0/open2/f214cee0.asm, called from docs/audit/note-shuffle/step0/open2/f215e280.asm line 69. It switches on the marker's kind byte at +0x18 (lines 35 to 45). For kinds 10, 9, 7 and 6 it compares the marker's +0x14 with the note's +0x20 and skips the note when they differ (lines 46 to 48, 53 to 55, 62 to 64 and 69 to 71). Kind 8 sets flag 0x100 with no such test (line 60). Three points stay Unknown. That +0x14 and +0x20 are the colour is inferred, not shown. Which kind number is cymbal, ghost or accent is not shown in this listing. The range bounds come from the helper 0x18213E310 (called at f215dbd0.asm line 86) and the lookup 0x1805FAD90 (line 103), and neither body was read.
Game code (6142): the apply routine, RVA `0x215DBD0`, and the flag pass at 0x214CEE0, the listings above.
Hydra: `Chord::apply_cymbal`, `Chord::apply_ghost` and `Chord::apply_accent` in src/core/model.cpp lines 292 to 305 throw `ChartFileError` when the chord has no note of the marker's colour. The .chart parser's per-op catch at src/parse/song.cpp lines 1805 to 1808 drops the marker.
Ruling: none needed: same rule.
Found: 2026-10-30, session b02ab3e6 (the reference build); the colour test read 2026-11-08, session 77057df2 (docs-gaps search).

### NL-09 A .chart kick has no ghost or accent marker

Rule: the .chart modifier map has no case that targets the kick, so a .chart kick is always a normal kick.
Label: Proven in the game's code
Evidence: decompile lines 24750 to 24825 (only note numbers 34 to 38, 40 to 44 and 66 to 68 have cases; every other value returns lane 0); `code/6142/chart_modifier_decode_213D160.asm` lines 11 to 80.
Game code (6142): RVA `0x213D160`, the listing above.
Hydra: `ChartParser::note_optype` has no kick modifier case (src/parse/song.cpp lines 1756 to 1770); `op_kick` passes a normal kick at line 1467.
Ruling: none needed: same rule.
Found: 2026-10-04, session 9f4f8eb5 (ADR 0012); re-read 2026-10-30 in session b02ab3e6 (the reference build).

## Dynamics

### NL-10 A .mid note gets its ghost or accent mark from velocity 1 or 127, kicks included

Rule: once the dynamics tag is on, the .mid reader sets the ghost bit for velocity 1 and the accent bit for velocity 127 on every pitch it reads, kicks and 2x kicks included; before the tag the note is plain.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 325 to 333 (the tag byte test, then velocity 1 sets bit 9 and velocity 127 sets bit 10); docs/audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm lines 245 to 253 show the lane code is tested only after the velocity test.
Game code (6142): RVA `0x2155050`, the listing above.
Hydra: `vel_dyn` in src/parse/song.cpp lines 928 to 932; `op_note` and `op_kick` lines 851 to 869; `mop_kick` line 648.
Ruling: D24 (record text): "The dynamics tag counts only in Clone Hero's two exact spellings".
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-11 The .mid dynamics tag is one of two exact spellings, set per track pass

Rule: dynamics turn on only at a drum-track text event equal to `ENABLE_CHART_DYNAMICS` or `[ENABLE_CHART_DYNAMICS]`, compared exactly with no trim inside the reader; the flag starts at 0 for each pass over a drum track, and notes before the event stay plain.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` line 100 (the flag starts 0 at 0x215524D), lines 363 to 368 (the event text loaded straight from the event) and 420 to 433 (two exact-equality compares at 0x21557A5 and 0x21557BB, then the flag set at 0x21557CD). Public Hydra accepted one-bracket forms (3b49012c, hydra/hysong.py line 273).
Game code (6142): RVA `0x2155050`, the listing above.
Hydra: `is_dynamics_marker` in src/parse/song.cpp lines 311 to 313; `dynamics_enabled_` reset at line 1146.
Ruling: D24: chose "Exact spellings (Recommended)" (card 2026-10-04-bbf41dc4, 01:57 UTC).
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-12 Whether the drum track's event text is trimmed before it reaches the reader

Rule: the drum track's text event is not trimmed where it is built, and the reader compares it with no trim (NL-11), so a tag with a trailing space does not count. One gap is recorded, not fixed: Hydra drops a byte-order mark at the start of a text event, and the game keeps it. That matters only for a text that starts with a byte-order mark. The user chose to record it.
Label: Proven in the game's code
Evidence: the drum reader takes the text from field +0x20 of the text event (`code/6142/mid_drum_reader_2155050.asm` lines 356 to 363) and compares it as is (lines 420 to 433). That event's class derives from a text base class whose only field is that string, and whose read method, RVA 0x2121340, the event class does not override (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs lines 543325 to 543337 and 550220 to 550235). That method decodes the event's bytes with System.Text.Encoding's GetString and stores the result straight into +0x20 (`code/6142/mid_text_event_read_2121340.asm` lines 95 and 96); its other branch stores the empty string (lines 99 to 102). No trim or other edit runs between the read and the store. Not searched: every other writer of the +0x20 field in the whole game. The events-track reader does trim (docs/audit/note-shuffle/step0/open1/midi_events_track_21544E0.asm, at 0x21546E3), but that is a different track.
Game code (6142): the text read, RVA 0x2121340, and the drum reader, RVA `0x2155050`, the listings above.
Hydra: `is_dynamics_marker` in src/parse/song.cpp line 311 compares the text exactly. The text comes from `decode_chart_text` (src/parse/midi.cpp lines 131 and 138), which drops a leading byte-order mark (src/parse/text_decode.cpp lines 120 to 135).
Ruling: D24 (record text): "caveat: assumes Clone Hero does not trim the text first". The byte-order mark gap: the user chose to record it (session 428cf079, 2026-11-11: "Approve the moves").
Found: 2026-10-30, session b02ab3e6 (the reference build); the text read listed 2026-11-08, session 77057df2.

### NL-13 At one tick, a note before the dynamics tag stays plain

Rule: the game meets a drum track's events in file order, and so does Hydra, so a note written before the tag on the tag's own tick stays plain in both. The track reader appends each event to the track's list as it reads it, and no direct or metadata route in the game sorts that list or inserts into it.
Label: Proven in the game's code
Evidence: the track reader, docs/audit/note-shuffle/step0/open1/mtrk_read_214F6D0.asm, reads one event at a time (lines 168 to 201) and appends each to the track's +0x18 list with List.Add (lines 405 to 411). It pairs a note-off with its note-on in place, through a stack per channel and pitch (lines 238 to 260 and 300 to 327). When the track ends, `code/6142/mtrk_unpaired_cleanup_213ACB0.asm` sets to null each note-on that never got a note-off and removes the null slots with RemoveAt, walking from the end, which keeps the order (lines 19 to 150). The MIDI reader hands the drum reader that +0x18 list (docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 518 to 526), and the drum reader walks it with its enumerator (`code/6142/mid_drum_reader_2155050.asm` lines 102 to 123). The tag sets the dynamics flag when the walk reaches it (line 432), and each note reads the flag when the walk reaches it (lines 325 to 333). Searched: every metadata entry for a method of the event list's type in C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\script.json lists no Sort, Insert or Reverse, and Add is loaded only in the track reader; a byte scan of the DLL's code found no other direct reference (direct callers only). Not ruled out: a sort or insert reached through an interface call (IList), and a write into the list's backing array through a span. A side fact: the game drops a note-on that has no matching note-off before any reader sees it; Hydra's handling of that case was not checked.
Game code (6142): the track reader, RVA `0x214F6D0`, the cleanup, RVA 0x213ACB0, and the drum reader, RVA `0x2155050`, the listings above.
Hydra: `kick_mark_before_tag_` and the note pass in src/parse/song.cpp lines 905 and 1001 to 1004.
Ruling: D24 (record text): "at a shared tick file order decides".
Found: 2026-10-03, session cebf3006; the track reader and its cleanup read 2026-11-08, session 77057df2.

### NL-14 A ghost kick scores double in the game

Rule: a velocity-1 kick under the dynamics tag is a ghost, and a ghost kick hit softly enough is paid double by the same code as a ghost pad. "Softly enough" means the kick's lane value is below the threshold at engine +0x2C0. Proven in the game's code: the engine doubles the points for a hit note that carries the ghost flag, kick or pad, when it is hit softly enough. Not shown: that a velocity-1 kick under the dynamics tag gets that flag, because the step that turns the reader's chart flag 512 into the note's 0x40000 was not read.
Label: Unknown
Evidence: the pay half is read in the drum engine's hit handler, `code/6142/note_hit_dynamics_20DE770.asm`. For lane mask 1, the kick, it loads the lane value from engine +0x30D (lines 38 and 53). It then asks whether the note is a ghost (lines 64 to 66). For a ghost whose lane value is below the threshold at engine +0x2C0 and not negative, it sets the performed mark, mask 4 of note +0x84, through 0x214B740, and counts a ghost (lines 91 to 101). The handler's only kick test (line 8) guards the out-of-lane hit counter at lines 8 to 23 (NL-59), not the mark. The scorer doubles the points when that mark is set, with no kick test (`code/6142/note_score_add_20DEEC0.asm` lines 17 to 24). The .mid reader gives a kick its ghost bit the same way as a pad (NL-10). Not read: the step that turns the reader's chart flag 512 (Ghost) into the note's 0x40000 when the playable note is built. NL-15's live run shows that step for the accent bit only. A live run gives one total. In run hydra-data/ch-mcp-runs/2026-11-08/ls-m2-run1, outside the repo, the scorestats file for Won't Get Fooled Again (O) gives ghost_notes_total 160. Hydra's score file for that chart, hydra-data/ch-mcp-runs/2026-11-08/kick-prep/who/The Who - Won't Get Fooled Again (O)-score.json, holds 124 pad ghosts and 36 kick ghosts, which sum to 160. The 160 is a total only: it does not show which notes the game flagged, and it does not test the doubling. That run's score does not match Hydra's for other reasons. ADR 0012 lines 14 to 24 call the doubling proven by video (a full combo of 1,142,235), and no video or result file for that run was found.
Game code (6142): the hit handler, RVA `0x20DE770`, and the scorer, RVA `0x20DEEC0`, the listings above.
Hydra: `mop_kick` and `op_kick` in src/parse/song.cpp lines 648 and 863 keep the dynamics on the kick, as `mop_note` does for a pad (line 641).
Ruling: none found.
Found: ADR 0012; the ghost count and the scorer read 2026-11-08, session 77057df2 (docs-gaps search); the hit handler read 2026-11-08, session 77057df2.

### NL-15 An accent kick scores double in the game

Rule: a velocity-127 kick under the dynamics tag scores double, as an accent pad does.
Label: Proven in a live run
Evidence: run hydra-data/ch-mcp-runs/2026-11-08/ls-m0-run1, outside the repo, 2026-11-08, on I Am Ahab (Mastodon). Its scorestats file gives band_score 525,555, with 166 of 166 accents and 167 of 167 ghosts hit. Its base_score (132,295) less its note_score (115,645) is 16,650, which is 50 for each of the 333 ghost and accent hits. Hydra's score file for that chart, hydra-data/ch-mcp-runs/2026-11-08/kick-prep/ahab/Mastodon - Leviathan - 02 - I Am Ahab-score.json, gives a final total of 525,555 and a final accent bonus of 8,300 (166 times 50), and 33 of its 166 accents are on the kick. The game's score equals Hydra's, so the game paid the 33 accent kicks double. This is one chart. The scorer doubles any note whose +0x84 mark bit is set, with no kick test (`code/6142/note_score_add_20DEEC0.asm` lines 17 to 24); what sets that bit on a kick is unread.
Game code (6142): the scorer, RVA `0x20DEEC0`, the listing above.
Hydra: `mop_kick` in src/parse/song.cpp lines 648, 932 and 940.
Ruling: none found.
Found: ADR 0012; the live result read 2026-11-08, session 77057df2 (docs-gaps search).

### NL-16 A note with both a ghost and an accent mark plays as an accent

Rule: when one note carries both marks (as a merged kick can, NL-29), the game plays it as an accent.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f20d4be0_notelist.asm lines 141 to 153. The builder first calls the getter 0x214DC90; when it returns true, it sets play-note bit 0x13 and skips the second test. Only when it returns false does it call the getter 0x214C8B0, and then it sets bit 0x12. The getters are in docs/audit/note-shuffle/step0/open2/getters.asm: 0x214DC90 reads bit 10 of the note's +0x24 flags (lines 19 to 23), and 0x214C8B0 reads bit 9 (lines 25 to 29). The .mid reader sets bit 10 for velocity 127 and bit 9 for velocity 1 (NL-10), so bit 10 is the accent and wins. The merge itself decides nothing (it ORs both bits, NL-29).
Game code (6142): the note-list builder, RVA `0x20D4BE0`, the listing above.
Hydra: `add_dynamic_mark` in src/core/model.cpp lines 256 to 259.
Ruling: "go with the games rule" (card 2026-10-09-27b9bbc4).
Found: 2026-10-09, session 27b9bbc4; the listing cited 2026-11-08, session 77057df2 (docs-gaps search).

## Disco flip

### NL-17 Disco flip is per difficulty, in both formats

Rule: a mix event goes only to the track of the difficulty it names, in a .mid and in a .chart, so each difficulty reads only its own markers.
Label: Proven in the game's code
Evidence: .chart: `code/6142/chart_e_handler_213CAF0.asm` lines 333 to 385 (the difficulty compare with the track's byte at +0x59, then the record). .mid: `code/6142/mid_drum_reader_2155050.asm` lines 363 to 418 (two starts-with tests on the raw text, the mix parser call, the track lookup by the parsed difficulty, the Add).
Game code (6142): RVAs `0x213CAF0` and `0x2155050`, the listings above.
Hydra: `is_disco_on_marker` and `is_disco_off_marker` in src/parse/song.cpp lines 316 and 328, given the difficulty's `mix_digit`.
Ruling: D19: chose "Per difficulty (Recommended)" (card 2026-10-04-bbf41dc4, 01:50 UTC).
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-18 The mix parser accepts spaces or underscores and any integer spelling

Rule: the game's mix parser trims the text, drops one leading [ and a trailing ] if present, turns each underscore into a space, needs the first word to be exactly "mix", reads the next word as an integer (0 Easy to 3 Expert, anything else rejected), then needs the rest to start with "drums". Hydra needs exactly one separator byte of any kind and one digit byte at fixed places.
Label: Proven in the game's code
Evidence: `code/6142/mix_parser_215C750.asm` lines 59 (trim), 64 to 84 (brackets), 98 to 174 (underscore to space at lines 167 to 169), 140 to 184 (first word), 204 to 248 (integer parse and the map) and 263 to 290 (starts with "drums"); `code/6142/mix_digit_map_210D990.asm` lines 1 to 18.
Game code (6142): RVAs `0x215C750` and `0x210D990`, the listings above.
Hydra: `disco_head` in src/parse/song.cpp lines 302 to 305; `peel_brackets` lines 292 to 296.
Ruling: open split: see questions page Q-12.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### NL-19 A spaced .chart mix event is a disco marker in the game, not in Hydra

Rule: a .chart E text that starts with `[mix` or `mix` goes whole to the mix parser, so `E [mix 3 drums0d]` with ordinary spaces is a marker in the game; Hydra reads an E line as a marker only when it is one word.
Label: Proven in the game's code
Evidence: `code/6142/chart_e_handler_213CAF0.asm` lines 116 to 168 (the two starts-with tests) and 319 to 330 (the parser call); NL-18 for the parser. Hydra's check is `t.count == 2` in src/parse/song.cpp lines 1304 to 1307. The corpus file testdata/parse_edge/c20_disco.chart holds one spaced form; the library was not searched.
Game code (6142): RVA `0x213CAF0`, the listing above.
Hydra: `classify_chart_line` in src/parse/song.cpp lines 1304 to 1307.
Ruling: open split: see questions page Q-10.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### NL-20 A disco span runs from a drumsNd event to the next six-letter drumsN event

Rule: a mix event whose text after the difficulty is 7 or more characters with d as its 7th starts a span; the span ends at the next mix event of the same difficulty whose text is exactly 6 characters and starts with "drums" (such as drums0), or two ticks after the last note. Only red and yellow notes in the span get the Disco flag.
Label: Proven in the game's code
Evidence: `code/6142/disco_apply_215CD50.asm` lines 98 to 116 (length and 7th character), 152 to 161 (same difficulty, next event), 220 to 223 (starts with drums, length 6), 243 to 255 (the range search) and 272 to 289 (types 14 and 15, the flag edit). The exact edge of the tick range lives in a helper that was not read.
Game code (6142): RVA `0x215CD50`, the listing above.
Hydra: `is_disco_on_marker` and `is_disco_off_marker` in src/parse/song.cpp lines 316 to 334; `emit_chord_timestamp` lines 446 to 449.
Ruling: D19 (record text): "Clone Hero's one-letter on/off rule (A+) is not adopted."
Found: 2026-10-30, session b02ab3e6 (the reference build).

### NL-21 drums0dnoflip does not turn flip off in the game

Rule: in the game `drums0dnoflip` counts as a span start (its 7th character is d), adds only a no-flip flag to notes that are not already Disco, and is too long to end an open drums0d span, so an earlier drums0d keeps flipping through it. Hydra reads it as flip off on purpose. This replaces the older reading "it looks only at the 7th character".
Label: Proven in the game's code
Evidence: `code/6142/disco_apply_215CD50.asm` lines 105 and 107 (the 7th character and the length 13), 137 to 148 (the compare with "dnoflip"), 223 (the end needs length 6) and 282 to 289 (Disco set, or the 0x40 flag only when Disco is clear); decompile lines 48364 to 48476. A video of a human full-combo run backs it: on 2026-09-27 a Pro Drums run of Periphery's Marigold scored 693,780 against Hydra's optimal of 664,700. A copy of the chart with the marker renamed (the copy was not kept), so flip stays on, matched the on-screen score at 101 readings, and its optimal is 693,980, 200 above the run (card 2026-09-28-1b0a7cd8). The run left no result file.
Game code (6142): RVA `0x215CD50`, the listing above.
Hydra: `is_disco_off_marker` in src/parse/song.cpp lines 328 to 334.
Ruling: deliberate difference: "this is a bug in clone hero, not an intentional feature. it will be fixed in a future update." (card 2026-09-28-1b0a7cd8, 03:43 UTC); D19: "`drums0dnoflip` stays off (Hydra does not copy Clone Hero's bug)."
Found: 2026-09-27, session 1b0a7cd8; re-read 2026-10-30 in session b02ab3e6 (the reference build).

### NL-22 Disco flip swaps red and yellow only with Pro Drums on

Rule: the game flips a disco note only on Pro Drums, in the builder's Pro Drums step `0x20F1420`. For each note with the Disco flag, red becomes the yellow cymbal, and the yellow tom or yellow cymbal becomes red. Plain Drums runs the cymbal fold and the dynamics clear instead (NL-61), so it never flips. Hydra does the same on that gate: it flips only under Pro Drums. Which notes get the Disco flag is a separate question, and there Hydra differs on purpose for `drums0dnoflip` (NL-21).
Label: Proven in the game's code
Evidence: the builder `code/6142/note_builder_20D2EA0.asm` lines 245 to 249 call 0x20F1420 only when the instrument byte is 9 (Pro Drums), and call the fold otherwise. In docs/audit/note-shuffle/step0/open2-refute/f20F1420.asm, line 29 calls the getter 0x214B780 for each note and lines 30 and 31 skip the note when it returns false. That getter is `test dword ptr [rcx + 0x7c], 0x1000` (docs/audit/note-shuffle/scout-flag/note_getters.txt line 11), and 0x1000 is DrumsDisco (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs line 548209). Lines 38 to 61 then rewrite the lane mask at +0x80: 2 (red) becomes 0x20 (line 48), and 4 or 0x20 becomes 2 (line 61). The lane bits are NL-50's. Which notes carry the Disco flag is NL-20. The builder listing docs/audit/note-shuffle/step0/open2/f20d2ea0_build.asm lines 150 to 270 holds no disco test of its own, because the test sits inside this step. No run has checked it live.
Game code (6142): the Pro Drums step, RVA `0x20F1420`, called from the builder `0x20D2EA0`, the listings above.
Hydra: `emit_chord_timestamp` in src/parse/song.cpp lines 446 to 449 calls the flip only when `pro` is set; `Chord::apply_disco_flip` in src/core/model.cpp lines 212 to 227 swaps red and yellow, red becoming a cymbal and yellow a tom.
Ruling: D19 (record text): "The 'flip only with Pro Drums' gate moves to one place."
Found: 2026-10-30, session b02ab3e6 (the reference build); the Pro Drums step read as the flip 2026-11-08, session 77057df2 (docs-gaps search).

### NL-23 How many charts carry drums0dnoflip

Rule: ADR 0023 says 48 library charts carry the marker. A scan in session 1b0a7cd8 found it in 22 of 19,883 chart files, and in 5 of them the game's reading (NL-21) raises the best score over Hydra's: Periphery Marigold by 29,280, Periphery Lune by 11,520, Vadrum Drum Intro Medley by 12,000, Northlane Corruption by 2,280 and Bearstronaut Shadow by 1,320. The best path is the same under both readings in all five.
Label: Unknown
Evidence: the session's card (docs/sessions/2026-09-28-1b0a7cd8.md) names the same five charts in a library of 19,883, and says each has a different best score under the game's reading and the same recommended path. The 22 and the five deltas come from the session's own record, which is not kept. The scan scripts were not kept either, so no repo file holds the result. The two counts (48 and 22) disagree, and settling them needs a whole-library scan, which needs the user's permission.
Game code (6142): not found: data, not code.
Hydra: none.
Ruling: none found.
Found: 2026-09-27, session 1b0a7cd8.

## 2x kick and the Double Kick setting

### NL-24 In a .mid each difficulty's 2x kick is the pitch below its kick

Rule: the 2x kick is pitch 59 on Easy, 71 Medium, 83 Hard and 95 Expert; it gets the 2x flag and the kick's note type.
Label: Proven in the game's code
Evidence: `code/6142/mid_drum_reader_2155050.asm` lines 316 to 324: the pitch minus 59 is tested against the mask 0x1001001001, whose bits 0, 12, 24 and 36 are those four pitches; docs/audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm lines 236 to 243 agree. The pitch test starts at instruction 0x21555CD (line 316), and the dynamics-tag test of NL-10 is at 0x21555F1 (line 325); Hydra's comments in tests/test_kick2x.cpp line 25, tests/test_dynamics_tag.cpp line 4 and src/parse/song.cpp lines 776 and 1003 name those two addresses.
Game code (6142): RVA `0x2155050`, the listing above.
Hydra: `DifficultyChartCodes::kick2x_pitch` in src/parse/song.h line 61; `kDifficultyChartCodes` in src/parse/song.cpp lines 682 to 687.
Ruling: D20: chose "A: own pitch, box everywhere" (card 2026-10-04-bbf41dc4, 01:53 UTC).
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build); the two instruction addresses named 2026-11-08, session 77057df2 (docs-gaps search).

### NL-25 The .mid pitch-to-difficulty and pitch-to-pad maps

Rule: pitches 58 to 66 are Easy, 70 to 78 Medium, 82 to 90 Hard and 94 to 102 Expert; within a difficulty, counted from its 2x kick, +0 and +1 are kicks, +2 red, +3 yellow, +4 blue, +5 the fourth pad and +6 the fifth; other pitches in range give no note.
Label: Proven in the game's code
Evidence: `code/6142/pitch_difficulty_map_210D9C0.asm` lines 1 to 24; the type map in docs/audit/note-shuffle/step0/open2/f210d7b0.asm.
Game code (6142): RVAs `0x210D9C0` and `0x210D7B0`.
Hydra: `kDifficultyChartCodes` in src/parse/song.cpp lines 682 to 687; `MidiParser::optype` lines 933 to 937.
Ruling: none needed for the four pads; the fifth pad is NL-04.
Found: 2026-10-30, session b02ab3e6 (the reference build).

### NL-26 A .chart N 32 is read on every difficulty with no gate

Rule: every difficulty section that holds an N 32 gets a 2x kick at read time; the Double Kick setting is applied later (NL-27).
Label: Proven in the game's code
Evidence: `code/6142/chart_n_handler_213D620.asm` lines 95 to 109 branch on the lane value only; decompile lines 25098 to 25110.
Game code (6142): RVA `0x213D620`, the listing above.
Hydra: `ChartParser::note_optype` case 32 in src/parse/song.cpp line 1760.
Ruling: D20 (as NL-24).
Found: 2026-10-03, session cebf3006; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-27 With Double Kick off, the game removes every 2x kick

Rule: the builder's only gate on 2x kicks is bit 2 of its modifier byte, at every difficulty; with the bit clear it calls a step that unlinks every note carrying the 2x-kick flag (0x100000) from its chord and removes it.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2/f20d2ea0_build.asm lines 231 to 236 (the bit test and the call); `code/6142/two_x_kick_drop_20EFF60.asm` lines 37 to 140 (the test call, the unlink at lines 46 to 93, the blanked slot at line 98, the RemoveAll at line 140); `code/6142/double_kick_test_214B850.asm` line 1 tests 0x100000 (DrumsDoubleKick in dump.cs line 548217). That bit 2 is the player's Double Kick setting comes from the live run in NL-28, not a listing.
Game code (6142): builder RVA `0x20D2EA0`; removal step RVA `0x20EFF60` of class #8278; test RVA `0x214B850`.
Hydra: `Chord::apply_2x_bass` in src/core/model.cpp line 281, called with `mode_bass2x_` in src/parse/song.cpp lines 875 and 1813.
Ruling: D20 (as NL-24); D105: chose "Match the game, test first (Recommended)" (card 2026-10-09-30beeb6e).
Found: 2026-10-09, session 30beeb6e; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-28 Live: a 2x kick on a normal kick's tick leaves no kick with Double Kick off

Rule: on songs F1 (.mid) and F2 (.chart), with 2x kick off the game showed a kick on chords 1 and 9 only and counted 11 notes; with it on, one kick on chords 1, 3, 5, 7 and 9 and 14 notes.
Label: Unknown
Evidence: the mechanism is in the game's code. The finalizer merges the two kicks into one that carries the 2x flag (NL-29), and the builder removes every note with that flag when its bit 2 is clear (NL-27). That bit 2 is the Double Kick setting, and the chord and note counts above, are recorded only in prose: docs/audit/note-shuffle/step0/FINDINGS.md line 53, for the run of 2026-10-09. Its recordings, C:\Users\Patrick\Videos\group 1.mp4 and group 2.mp4, are not committed, and no data reading of them is kept, so the live result is not a permanent record. Hydra pins the expected counts in tests/test_kick2x.cpp lines 130 to 136.
Game code (6142): builder RVA `0x20D2EA0` (NL-27) and the merge, RVA `0x215B8C0` (NL-29).
Hydra: `Chord::add_kick` in src/core/model.cpp lines 264 to 277; `Chord::apply_2x_bass` line 281.
Ruling: D105: chose "Match the game, test first (Recommended)" (card 2026-10-09-30beeb6e).
Found: 2026-10-09, session 30beeb6e; relabelled 2026-11-08, session 77057df2 (docs-gaps search), because the video results live only in prose and uncommitted video.

### NL-29 A 2x kick and a normal kick on one tick merge into one kick with every mark

Rule: both readers keep each kick as its own note; the track finalizer then merges notes of one tick and one type into the longest one (the first on a tie), or-ing every flag bit except the low three, so the merged kick carries the 2x bit and both kicks' ghost and accent bits.
Label: Proven in the game's code
Evidence: the readers: `code/6142/mid_drum_reader_2155050.asm` lines 341 to 354 and `code/6142/chart_n_handler_213D620.asm` lines 119 to 174 (one note per event). The merge: `code/6142/kick_merge_215B8C0.asm` lines 54 and 94 (same tick), 57 and 97 (same type), 68 and 69 (longest kept), 102 to 123 (the flag merge, low three bits at line 110). The finalizer docs/audit/note-shuffle/step0/open2/f215c320.asm lines 59 to 63 calls it.
Game code (6142): a method of the track class #8661, RVA `0x215B8C0`, the listing above.
Hydra: `Chord::add_kick` in src/core/model.cpp line 264.
Ruling: D105: chose "Match the game, test first (Recommended)" (card 2026-10-09-30beeb6e); "go with the games rule" and "that is the source of truth here" (card 2026-10-09-27b9bbc4).
Found: 2026-10-09, session 30beeb6e; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-30 The merged kick does not depend on which kick comes first in the file

Rule: Hydra merges a 2x kick and a normal kick on one tick in either file order; the game's finalizer or-s flags whatever the order. Songs F1 and F2 hold both orders; no kept run has played them.
Label: Unknown
Evidence: the merge is read in full, but no run with both file orders was kept. In `code/6142/kick_merge_215B8C0.asm`, lines 54 to 70 keep the longest note of the tick and type, and a later note replaces it only when strictly longer (the conditional move at line 70), so on a length tie the lower list index is kept. Lines 102 to 123 OR the high flag bits of both notes and keep the highest of the low three, which gives the same flags in either order. Line 128 appends the other note's +0x38 list to the kept one. So the flags do not depend on file order, but which object survives, and the order of its +0x38 list, can. The 2026-10-09 run of songs F1 and F2 (chord 3 with the normal kick first, chord 5 the other way round) is recorded only in prose, docs/audit/note-shuffle/step0/FINDINGS.md line 53, with uncommitted video. The two song files were read on 2026-11-08 (docs/audit/2026-11-08-clone-hero-unknowns/W5.md, section NL-30): C:\Clone Hero\songs\Hydra Note Shuffle tests\NS F2 2x kick shared tick chart\notes.chart and the F1 notes.mid each hold nine chords at 480 ticks per beat. Chord 3 (tick 2880) has the normal kick first (notes.chart lines 25 and 26), and chord 5 (tick 3840) has the 2x kick first (lines 29 and 30); the .mid lists pitches 96 then 95, and 95 then 96, at those ticks. So both orders are in both files, and the live check still has to be run. Hydra's side is src/core/model.cpp lines 273 to 276.
Game code (6142): RVA `0x215B8C0` (NL-29), the listing above.
Hydra: `Chord::add_kick` in src/core/model.cpp lines 264 to 277.
Ruling: D105 (as NL-29).
Found: 2026-10-30, session b02ab3e6 (the reference build); the tie rule read 2026-11-08, session 77057df2 (docs-gaps search).

### NL-31 The kick key scores a 2x-kick gem, and the Slipping layout

Rule: pressing the kick key scores a 2x-kick gem; in Synovial's Slipping `notes.mid` the 2x kick is its own pitch (302 gems on 95 against 915 on 96, never on one tick).
Label: Unknown
Evidence: not checked: the 2026-10-29 run's `scorestats.json` (tools/ch_probe/experiments/results/overlay-spike/run1_cal0_scorestats.json) shows DoubleKick on and 1,257 of 1,257 notes hit, but the press log has no kick-type column, and the Slipping counts come from a transcript, not a recount.
Game code (6142): not found: not read.
Hydra: `mop_kick` in src/parse/song.cpp line 940.
Ruling: none found.
Found: 2026-09-25, session 8157f2dc.

## Chord order and merged notes

### NL-32 Two pads of one colour on one tick merge in the game; Hydra drops the second

Rule: the merge in NL-29 applies to every note type, so two red notes on one tick become one red note holding both notes' flags; Hydra's second `Chord::add_note` throws "Duplicate note." and the per-op catch drops that note with its marks.
Label: Proven in the game's code
Evidence: `code/6142/kick_merge_215B8C0.asm` lines 54 to 128 compare tick and type only, with no kick test; docs/audit/note-shuffle/step0/open2/f215b8c0.asm lines 38 to 128 agree. Hydra: src/core/model.cpp line 244 throws, and the catches in src/parse/song.cpp lines 1047 and 1807 drop the op (checked 2026-10-30).
Game code (6142): RVA `0x215B8C0`, the listing above.
Hydra: `Chord::add_note` in src/core/model.cpp line 242.
Ruling: open split: see questions page Q-14.
Found: 2026-10-30, session b02ab3e6 (the reference build); Hydra side checked by writer W1.

### NL-33 In a .mid, a tick's notes are ordered kick, red, yellow, blue, green

Rule: before any builder step the game orders each tick's notes by note type, lowest first, and breaks a tie by the flags word, lowest first. The type numbers put the kick first, then red, yellow and blue, then the fourth pad (Hydra's green). A cymbal is a flag, not a slot, so a yellow cymbal sits in the yellow slot, and file order is discarded.
Label: Proven in the game's code
Evidence: the track finalizer docs/audit/note-shuffle/step0/open2/f215c320.asm calls the sort 0x215C040 at line 56. In docs/audit/note-shuffle/step0/open2/f215c040.asm, lines 64 and 65 compare the notes' +0x10 tick, and lines 144 to 155 are the sort's compare: when two notes have the same +0x20 type it swaps them only if the earlier one's +0x24 flags are greater, and otherwise only if the earlier one's type is greater. The other path calls 0x215E020 (line 95), whose listing docs/audit/note-shuffle/step0/open2/f215e020.asm makes the same compare at lines 37 to 48. The type numbers (kick 13, red 14, yellow 15, blue 16, the fourth pad 18, the fifth pad 17) are in `code/6142/chart_n_handler_213D620.md`. A live check is recorded only in prose: docs/audit/note-shuffle/step0/FINDINGS.md line 55 says song B (.mid) matched the colour-order row with Note Shuffle on, 2026-10-09, but its video, C:\Users\Patrick\Videos\group 3.mp4, is not committed and no data reading of it is kept.
Game code (6142): the sort, RVA `0x215C040`, with `0x215E020`, called from the finalizer `0x215C320`, the listings above.
Hydra: `NoteColor` order in src/core/model.h line 56; `Chord::note_list` in src/core/model.cpp lines 156 to 159.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e; the sort direction read and the label moved to code 2026-11-08, session 77057df2 (docs-gaps search).

### NL-34 A .chart's tick order is the same as a .mid's

Rule: both parsers end in the same track finalizer, so a .chart's within-tick order equals the .mid's (NL-33). No .chart was played to check it.
Label: Proven in the game's code
Evidence: all three parser listings call the finalizer 0x215C320: docs/audit/note-shuffle/step0/open2/f2161a80.asm line 221 (the .chart parser), f2155050.asm line 557 (the .mid parser) and f2156990.asm line 655, all in docs/audit/note-shuffle/step0/open2/. The finalizer calls the sort (NL-33). Only the live .chart test is missing: docs/audit/note-shuffle/step0/FINDINGS.md lines 61 to 63 say the .chart order is "code reading only".
Game code (6142): the finalizer, RVA `0x215C320`, called from `0x2161A80`, `0x2155050` and `0x2156990`, the listings above.
Hydra: `Chord::note_list` in src/core/model.cpp line 156.
Ruling: none needed.
Found: 2026-10-09, session 30beeb6e; the call sites read 2026-11-08, session 77057df2 (docs-gaps search).

### NL-35 Every chord a chart holds is stored and scored, three-pad chords included

Rule: Hydra never decides a chord is unplayable; the game's handling of three pads at once was not read.
Label: Hydra assumes it, never checked
Evidence: `Chord::code` in src/core/model.cpp line 108; ADR 0015.
Game code (6142): not found: not read.
Hydra: `Chord::code` in src/core/model.cpp line 108.
Ruling: "if it exists hydra should score it" (card 9405caaa, 03:07 UTC).
Found: session 9405caaa (Hot Sexy Girls, 21 three-pad chords).

### NL-36 The .chart note record and its length field

Rule: the .chart N handler stores tick, length, type, flags and list index, and keeps the line's length only when it is greater than the song's int at +0x114 (else 0). That int works as a sustain cutoff; "sustain cutoff" is our reading of the compare, not a name in the code. The .chart loader sets it from a settings int when that int is 0 or more, else 0; the .mid loader uses the same settings int when it is 0 or more, else the resolution divided by 3, plus 1.
Label: Proven in the game's code
Evidence: `code/6142/chart_n_handler_213D620.asm` lines 119 to 126 load +0x114 from the object at track +0x50 and keep the length only when it is greater (the conditional move at line 126); lines 119 to 148 store the note. The track is class #8661, whose +0x50 field has the song's type, class #8538 (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs line 551849). The song's +0x114 is a private int (dump.cs line 543395). The .chart loader writes it in docs/audit/note-shuffle/step0/open1/chart_resolution_2164100.asm lines 310 to 319, and the MIDI loader in docs/audit/note-shuffle/step0/open1/midi_reader_2155C00.asm lines 132 to 141. Not read: which settings field the loaders' +0x24 is. The song.ini reader keeps sustain_cutoff_threshold at +0x1C of its own settings object (`code/6142/ini_song_reader_1EFC60.md` line 8), and whether that value reaches the loaders' +0x24 is unread.
Game code (6142): the N handler, RVA `0x213D620`, and the two loaders, RVAs 0x2164100 and `0x2155C00`, the listings above.
Hydra: no owner: Hydra's drum chart keeps no note length this feeds.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build); the song field read 2026-11-08, session 77057df2.

## Flams

### NL-37 A flam copies a single-pad chord's note one pad up

Rule: a .mid flam marker (pitch 109) on a chord with exactly one pad (a kick may be with it) adds a copy one pad up, right after it, and the copy counts toward the shuffle seed; for green, Hydra keeps green and copies blue, while the game moves the source to blue and copies green, which gives the same pair.
Label: Proven in the game's code
Evidence: the flam step is read in full in docs/audit/note-shuffle/step0/open2/drum_pretransforms_20ef450_20eff60_20f0320_20f1420_20f16e0_20efee0.asm. Line 29 loads the mask 0xFFFE, which leaves the kick bit out. Lines 61 to 70 count the chord's other pads and skip the chord when there are two or more. Lines 77 and 78 go on only when the note's flag word at +0x7C holds 0x20000, which is DrumsFlam (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs line 548214). Lines 104 to 157 pick the pads from the lane mask at +0x80: red gives a yellow copy, yellow a blue copy, blue a green copy, and the cymbals step up the same way. For green the source moves down to blue and the copy gets green: lines 111 to 122 do it for the green cymbal (0x80 becomes 0x40, the copy 0x80), and lines 124 to 133 with 154 to 156 for the green tom (0x10 becomes 8, the copy 0x10). Lines 175 to 180 insert the copy into the note list at the source's index plus 1, so it comes right after the source. The pads match FINDINGS.md line 21. That the copy counts toward the shuffle seed follows from the seed reading the note list (NL-40).
Game code (6142): the flam step, RVA `0x20EF450`, the listing above. The address 0x20F0320 in the same listing is not the flam: it is the type rewrite in NL-73.
Hydra: `Chord::apply_flam_conversion` in src/core/model.cpp lines 229 to 240; `kFlamMarkerPitch` in src/parse/song.cpp line 707.
Ruling: none found.
Found: 2026-10-09, session 30beeb6e; the flam step read in full 2026-11-08, session 77057df2 (docs-gaps search).

### NL-38 What a flam copy does with a cymbal flag or a ghost or accent mark

Rule: both the game and Hydra copy the source note whole, so the copy keeps the source's cymbal state and its ghost or accent mark. A green cymbal flam gives a blue cymbal and a green cymbal in both.
Label: Proven in the game's code
Evidence: in docs/audit/note-shuffle/step0/open2/drum_pretransforms_20ef450_20eff60_20f0320_20f1420_20f16e0_20efee0.asm, line 85 ORs 3 into the source's flag word at +0x7C, and then line 97 builds the copy by calling 0x214BE70 with the source note. That call's listing, docs/audit/note-shuffle/step0/open1-refute/gamenote_clone_214BE70.asm, copies the lane mask at +0x80 (lines 18 and 19) and the whole flag word at +0x7C (lines 20 and 21). The ghost and accent bits live in that word (DrumsGhostHit 0x40000 and DrumsAccentHit 0x80000, dump.cs lines 548215 and 548216, cleared from +0x7C by NL-62's listing). The flam step then sets only the lane mask (NL-37), and a cymbal is a lane-mask bit (NL-50), so the green cymbal pair is blue cymbal plus green cymbal.
Game code (6142): the flam step, RVA `0x20EF450`, and the note copy, RVA 0x214BE70, the listings above.
Hydra: `Chord::apply_flam_conversion` in src/core/model.cpp lines 229 to 240.
Ruling: none needed: same rule.
Found: 2026-10-30, session b02ab3e6 (the reference build); the copy read 2026-11-08, session 77057df2 (docs-gaps search).

### NL-39 Whether the game can flam a .chart note

Rule: Hydra never flams a .chart, which has no flam marker; the game's flam step runs on the builder's note list for any format, and where its trigger flag comes from in a .chart is unread.
Label: Unknown
Evidence: the step is called for any format (docs/audit/note-shuffle/step0/open34/builder_20d2ea0.asm lines 228 to 230). Half of the .chart side is read. The .chart modifier decoder, `code/6142/chart_modifier_decode_213D160.asm`, handles only N 5 and 6 (lines 68 to 77), N 34 to 44 through a jump table (lines 13 to 51) and N 66 to 68 (lines 52 to 67); every other value gets kind 0 at line 78. None of its kinds is a flam, so no .chart N number sets the flam flag through it. The marker route is now closed in the game's code. `0x215E280` sets the chart flag 256 (Flam) only from a kind-8 marker: it applies the track's markers (`code/6142/note_flags_from_markers_215E280.asm` lines 21 to 45) and then runs the flag pass 0x214CEE0, which sets 256 only for kind 8 (docs/audit/note-shuffle/step0/open2/f214cee0.asm lines 40, 41 and 60). The .chart N handler's own note flags are 0, 16 or 8, never 256 (`code/6142/chart_n_handler_213D620.asm` lines 63, 85, 99 and 106). Every method that makes or adds a marker is .mid code or the .chart N handler, Five marker methods have no direct caller: 0x20E6F00, 0x20E71A0, 0x215BB20, 0x215CC80 and 0x215E420 (`code/6142/marker_make_from_event_20E6F00.asm`, `code/6142/marker_copy_to_difficulties_20E71A0.asm`, `code/6142/marker_add_215BB20.asm`, `code/6142/marker_add_215CC80.asm` and `code/6142/marker_add_215E420.asm`). None has a route in: no vtable slot in dump.cs, no metadata entry in script.json, no string literal with its name, and its one pointer sits in the runtime's method table. Searched: byte scans of the DLL for calls, jumps, RIP-relative loads and 8-byte pointers to those methods, to the marker constructor 0x213E320, to the marker dictionary's add 0x2160210 and to the marker list's Add (direct callers only, byte scan of il2cpp and .text). The NL-39 sections of docs/audit/2026-11-08-clone-hero-unknowns/W3.md keep the caller lists. Still not read, so the block stays Unknown: every other writer of chart flag 256 (the chart note's flag word at +0x24) and of bit 0x20000 of the playable note's flag word at +0x7C, and the step that copies one into the other.
Game code (6142): the decoder, RVA `0x213D160`, the listing above; the flam step `0x20EF450`; the marker step `0x215E280` and its flag pass 0x214CEE0.
Hydra: `emit_chord_timestamp` in src/parse/song.cpp lines 446 to 448 flams only on the .mid path.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build); the decoder cited 2026-11-08, session 77057df2 (docs-gaps search); the marker route traced 2026-11-08, session 77057df2.

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
Evidence: docs/audit/note-shuffle/step0/open34/lanedraw_20f0b30.asm lines 12 to 52 (the shifts 23, 17 and 26, the 31-bit mask at line 31, the constant at line 10, the increment and shift at lines 49 to 51). The integer equivalence is reasoning, not a test.
Game code (6142): RVA `0x20F0B30`, the listing above.
Hydra: `LaneGenerator::next_lane_index` in src/parse/note_shuffle.cpp lines 69 to 78.
Ruling: none needed: same rule.
Found: 2026-10-09, session 6c19d0ef.

### NL-42 Which pads the shuffle draws from

Rule: the shuffle draws from pads 1 to 7 (N = 8) for instrument 9 and from pads 1 to 4 (N = 5) for instrument 6, and never draws the kick. Instrument 9 is Pro Drums and 6 is plain Drums (NL-43).
Label: Proven in the game's code
Evidence: the two jump tables that pick a target by instrument (at 0x20F11CC and 0x20F1208) show only as raw bytes in docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 350 and 351. The committed script docs/audit/note-shuffle/step0/open34-refute/jumptables.py reads both tables from the game's DLL; its output is kept as docs/audit/2026-11-08-clone-hero-unknowns/W5-jumptables-output.txt, whose header gives the exact command. Lines 12 and 15 of that output send instrument 6 to 0x20F0E2E and instrument 9 to 0x20F0E48. In the listing, 0x20F0E2E sets 4 and adds 1, so N = 5 (lines 117 to 119), and 0x20F0E48 sets 7 and adds 1, so N = 8 (lines 125 to 127). The draw scales a random fraction by N - 1, truncates and adds 1 (docs/audit/note-shuffle/step0/open34/lanedraw_20f0b30.asm lines 42 to 49), so the drawn index runs from 1 to N - 1 and never gives index 0, the kick. Pro Drums matches the filmed runs; plain Drums was never filmed (tests/test_note_shuffle.cpp lines 98 to 100).
Game code (6142): RVA `0x20F0C30` with the draw `0x20F0B30`; the jump tables at 0x20F11CC and 0x20F1208, decoded by the script above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 131 and 132.
Ruling: none found.
Found: 2026-10-09, session 30beeb6e; the decoder script cited 2026-11-08, session 77057df2 (docs-gaps search); its output kept 2026-11-08, session 77057df2.

### NL-43 The shuffle runs last among the drum steps, only with Note Shuffle on

Rule: the shuffle runs only when modifier bit 0x40 is set, after the flam step, the 2x-kick removal (when bit 2 is clear), the cymbal fold or Pro step, and the dynamics clear; so each drum setting gets its own shuffled chart.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/builder_20d2ea0.asm lines 153 to 162 (instruments 6 and 9 take the drum path) and 228 to 259 (the step calls in order, then the 0x40 test and the call at 0x20D3315). The names of the steps before the shuffle come from docs/audit/note-shuffle/step0/open2/README.md lines 9 and 24 (2026-10-09). The disco flip runs before the shuffle on Pro Drums, because it is the Pro step (NL-22). The shuffle is also called for instruments other than drums: in docs/audit/note-shuffle/scout-flag/build_20d2ea0.asm, lines 153 to 156 send instruments 6 and 9 to the drum path, and for any other instrument lines 157 to 162 call the shuffle at 0x20D31B1 when bit 0x40 is set. docs/audit/note-shuffle/step0/open34/callers.txt lines 5 and 6 list both call sites, 0x20D31B1 and 0x20D3315. What the shuffle does to a non-drum chart was not read.
Game code (6142): builder RVA `0x20D2EA0`, the listing above.
Hydra: `apply_shuffle_switch` in src/parse/song.cpp lines 1992 to 1996 and `finish_loaded_song` lines 2004 to 2009, after `emit_chord_timestamp` and `Chord::apply_2x_bass`.
Ruling: D104 item 3 (record text): "Pro Drums off gets its own row too."
Found: 2026-10-09, session 6c19d0ef; the non-drum call added 2026-11-08, session 77057df2 (docs-gaps search).

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
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 209 to 284 (the same-tick test, the two pattern slots, the rank search through two bit helpers, the slot rotation). The helper bodies are in docs/audit/note-shuffle/scout-flag/helpers.asm. The chord's pattern comes from 0x214BCD0 (called at line 215), which ORs the +0x80 lane mask of every note in the chord (helpers.asm line 73). The rank search calls `0x20FC610` (line 242), which saves the mask and clears its lowest set bit (helpers.asm lines 85 to 94), and then `0x20FC5F0` (line 247), which returns the old mask XOR the new one, that is the bit just cleared (helpers.asm lines 97 to 99). So the set bits are walked lowest first. A probe that drops the rule matches 5 of 22 filmed ticks (card 2026-10-09-cfb6f1fa).
Game code (6142): RVA `0x20F0C30`; helpers `0x20FC610` and `0x20FC5F0`, and the pattern helper 0x214BCD0, the listings above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 154 to 159 and 173 to 176.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e; the helper bodies read 2026-11-08, session 77057df2 (docs-gaps search).

### NL-47 The redraw loop has no cap, so two cases freeze the game

Rule: the redraw loop repeats until the drawn pad is free, with no counter, timer or fallback. So a four-colour chord followed by a chord whose first note has no copy loops forever, and so does a zero generator state (the first four notes all on tick 0), which draws red every time.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm lines 285 to 315 (the loop and its only exits) and docs/audit/note-shuffle/step0/open34/lanedraw_20f0b30.asm lines 12 to 52 (a zero state stays zero and gives index 1). The 2026-10-09 runs C1 and D1 hung as the song loaded (FINDINGS.md line 57; video group 4.mp4, not viewed in the reference build).
Game code (6142): RVAs `0x20F0C30` and `0x20F0B30`, the listings above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 162 and 163 return the freeze, shown as `ErrorKind::NoteShuffleFreezes`; `LaneGenerator::state_is_zero` line 81.
Ruling: D104 item 5 (record text): "A chart the game can't shuffle gets no path."
Found: 2026-10-09, session 30beeb6e.

### NL-48 From any non-zero state the redraw loop ends

Rule: the generator reaches every pad from any non-zero state, so the loop ends whenever a pad is free.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/probes/reach/reach_check.py steps the generator as the game does (its lines 10 to 14). The game's step is docs/audit/note-shuffle/step0/open34/lanedraw_20f0b30.asm lines 17 to 31: lines 21 and 22 make x = s0 ^ (s0 << 23), lines 20 and 23 to 26 make the new state x ^ s1 ^ (x >> 17) ^ (s1 >> 26), and lines 27 to 29 shift the pair. The script's output is kept as docs/audit/2026-11-08-clone-hero-unknowns/W5-reach_check-output.txt. Part 1 there (lines 4 to 14) shows the step's matrix to the power 2^128 - 1 is the identity and no power below it at the nine prime factors is, so every non-zero state lies on one cycle: from any non-zero state the generator walks every non-zero state, and so reaches every pad. Part 2 (lines 15 and 16) is a sample, not a proof: over 700,068 seeds every pad came up within 94 draws with Pro Drums and 49 on plain Drums. The game's draw clears bit 31 of the sum with a 32-bit instruction, which also clears the upper half (line 31), so it uses the low 31 bits, as part 2 and Hydra do.
Game code (6142): RVA `0x20F0B30`, the listing above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 164 to 168 redraw with no counter.
Ruling: none found.
Found: 2026-10-09, session 30beeb6e; the script read 2026-11-08, session 77057df2 (docs-gaps search); its output kept 2026-11-08, session 77057df2.

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
Evidence: docs/audit/note-shuffle/step0/open34/starbase_20f4680.asm line 105 reads the lane mask; engine_ctor_20f79e0.asm line 196 calls it, and baseplayer_208870.asm lines 55 and 103 call the builder before the engine. "Only direct caller" rests on a 2026-10-09 scan not re-run. That scan's output is docs/audit/note-shuffle/step0/open34/callers.txt: lines 1 to 4 list four callers of the builder `0x20D2EA0` (in `0x208870`, in 0x235AD0, and in two generic constructors at 0xFEBB30 and 0xFEBCB0), and line 8 lists one caller of the star base, in the engine constructor `0x20F79E0`. The cymbal bonus inside the star base is page 6's.
Game code (6142): RVAs `0x20F4680`, `0x20F79E0` and `0x208870`, the listings above.
Hydra: `finish_loaded_song` in src/parse/song.cpp line 2005; `Path::chart_base_score` in src/core/model.h line 878.
Ruling: none needed: same rule.
Found: 2026-10-09, session 30beeb6e; the builder's four callers added 2026-11-08, session 77057df2 (docs-gaps search).

### NL-52 The filmed Note Shuffle runs of 2026-10-09

Rule: songs A1 (.chart, 192 ticks per beat) and A2 (.mid, 960) showed their own resolution's row; songs C2 and D2 played and matched; song E scored 19,925 with 5 stars on 106 notes.
Label: Unknown
Evidence: not checked: the videos C:\Users\Patrick\Videos\group 3.mp4 and group 4.mp4 exist but were read only through FINDINGS.md lines 55 to 57. Hydra pins the predicted rows in tests/test_note_shuffle.cpp lines 102 to 125.
Game code (6142): not found: live runs, not code.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp.
Ruling: none found.
Found: 2026-10-09, session 30beeb6e.

## Lanes

### NL-53 A .mid lane is pitch 126 or 127, and its velocity picks the lowest difficulty

Rule: a .mid lane is a note-on of pitch 126 (single) or 127 (double) on the drum track; its velocity names the lowest difficulty that shows it, by compares at 21, 31, 41 and 51.
Label: Proven in the game's code
Evidence: docs/audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm lines 85 and 118 (the two pitches). Lines 93 to 107 map the velocity to a value: 51 and up gives 3, 41 to 50 gives 2, 31 to 40 gives 1, 21 to 30 gives 0, and under 21 gives 3 (the borrow at line 100 masked with 3). Lines 126 to 140 make the same mapping for pitch 127. That the value names the lowest difficulty (0 Easy to 3 Expert) is D112's reading; the listing does not show where the value is used.
Game code (6142): RVA `0x2155050`, the lane case.
Hydra: `kSingleLaneMarkerPitch` and `kDoubleLaneMarkerPitch` in src/parse/song.cpp lines 716 and 717.
Ruling: D112: "go with clone hero" (card 2026-10-11-14ef4882, 00:36 UTC).
Found: 2026-10-10, D112; the velocity mapping read 2026-11-08, session 77057df2 (docs-gaps search).

### NL-54 A .chart lane is S 65 or S 66

Rule: in a .chart, special phrase type 65 gives a single lane and type 66 a double lane. Hydra reads them the same way.
Label: Proven in the game's code
Evidence: docs/audit/drum-lanes/spec_213e0c0.asm tests the type with a subtract chain, not a compare with 0x41 or 0x42. Line 65 subtracts 0x40, and type 64 goes to a different call (line 66). Type 66 leaves 1 in ecx and passes it as the fourth argument (lines 67 to 69 and 74). Type 65 zeroes that argument at line 89 and jumps at line 91 back to line 76, past line 74, so it passes 0. Both then call 0x20CEEC0 at line 79. That method keeps its fourth argument (`code/6142/lane_add_20CEEC0.asm` line 7) and hands it to the lane constructor 0x2116390 as its fourth argument (lines 31 and 37). The constructor stores it at +0x21 (docs/audit/drum-lanes/lane_ctor_2116390.asm line 6), and the lane builder adds the double flag 0x8000 only when that byte is set (docs/audit/drum-lanes/f20d12c0.asm lines 207, 208 and 215). So S 66 gives a lane with +0x21 = 1 and S 65 one with +0x21 = 0. Direct callers of 0x20CEEC0 (byte scan of il2cpp and .text): the S-line handler at 0x213E205, the .mid drum reader at 0x21553DF and the MIDI reader method at 0x2156ED1. Not shown: that the phrase must sit in a difficulty's section.
Game code (6142): the S-line handler, RVA `0x213E0C0`, the lane add 0x20CEEC0 and the lane constructor 0x2116390, the listings above.
Hydra: `kChartSingleLanePhrase` and `kChartDoubleLanePhrase` in src/parse/song.cpp lines 1504 and 1505; the S line case that sets the double flag, lines 1714 to 1716; `op_lane` line 1454 records the lane.
Ruling: D112 (as NL-53).
Found: 2026-10-10, D112; the type test read 2026-11-08, session 77057df2 (docs-gaps search); the lane add read 2026-11-08, session 77057df2.

### NL-55 Which pads a lane lights

Rule: a single lane lights the lowest pad of the first chord with a non-kick note, in the order red, yellow tom, blue tom, green tom, yellow cymbal, blue cymbal, green cymbal. A double lane adds the highest pad of a later chord: the search for the second chord starts from the first chord and always skips it, even when the first chord has two or more pads. Example: first chord red plus yellow cymbal, next chord blue tom; the game lights red and blue tom.
Label: Proven in the game's code
Evidence: docs/audit/drum-lanes/f20d5310.asm lines 29 to 35 AND the chord's lane mask with the wanted pads and keep its lowest set bit. The builder docs/audit/drum-lanes/f20d12c0.asm passes the mask 0xFE, which leaves the kick out, to the second search at lines 157 to 163, starting from the first chord's own tick and index. That search, docs/audit/drum-lanes/f5de790.asm lines 69 to 78, accepts a chord only when its tick is later, or the tick is equal and the index differs, so the first chord is never taken. The second search is given no end: the builder passes it the chord list, a tick, the pad mask and a start index (f20d12c0.asm lines 157 to 162), so the second chord may sit after the lane's end. The builder does not test the search's result for -1 before it reads the chord (lines 163 to 167), and the getter it calls has no listing, so what the game does when no later chord exists is Unknown. The highest-pad pick is docs/audit/drum-lanes/f20d51d0.asm. The flagged range runs to the lane's start tick plus its length plus 1 (f20d12c0.asm lines 183 to 186, calling 0x5DEB40, docs/audit/drum-lanes/f5deb40.asm lines 61 to 73). Two points stay Unknown. First, whether the game's chord list can hold two chords at one tick is not shown; the merge at `code/6142/kick_merge_215B8C0.asm` lines 33 to 98 suggests not. Second, what the game does when no later chord exists, as above. The decision log's D112 item 3 said the second pad comes from the first chord itself when it has two or more pads; its correction of 2026-11-11 (D333 in docs/audit/2026-10-03-fix-decisions.md) withdraws that.
Game code (6142): the lane builder, RVA `0x20D12C0`, with the pad helpers `0x20D5310` and `0x20D51D0` and the search 0x5DE790, the listings above.
Hydra: `light_drum_lanes` in src/parse/song.cpp line 203 and `lights_lane_pad` line 88, with the pick in `game_lane_pads` line 147: a double lane takes the highest pad of the next chord with a pad, anywhere later in the song (lines 159 to 165). When the song has no such chord Hydra lights the first pad alone (line 166).
Ruling: D112: "go with clone hero" (card 2026-10-11-14ef4882, 00:36 UTC); for the second pad the user chose "Match the game (Recommended)" (docs/sessions/2026-11-08-77057df2.md, 20:51 UTC). For the case with no later chord the user answered "go with recommended" (2026-11-11, session 428cf079, D333), which lights the first pad alone.
Found: 2026-10-10, D112; the listings read 2026-11-08, session 77057df2 (docs-gaps search); Hydra matched to the game 2026-11-11, session 428cf079.

### NL-56 Only the lane builder and the drum engine read the lane flags

Rule: the flag bits 0x4000 (in a lane), 0x8000 (double) and 0x10000 (last note) are set by the lane builder, and every reader a direct-constant sweep finds is in the drum engine. Proven in the game's code: the builder sets them only on notes whose lane mask overlaps the lane's lit pads, and 0x10000 only on the last such note. A note in a double lane carries 0xC000, because 0x8000 is added to 0x4000. The reader half rests on a sweep whose script was not kept, so it is not a kept count.
Label: Unknown
Evidence: how they are set is read: in docs/audit/drum-lanes/f20d12c0.asm, line 205 tests the note's +0x80 lane mask against the lit pads and line 206 skips the note when they do not overlap. Lines 207 and 208 test the lane's double byte at +0x21, line 215 ORs in 0x8000 and line 222 ORs in 0x4000. Line 223 remembers the note's index, and line 230 ORs 0x10000 into the note at the last remembered index. The names are DrumsLane, DrumsLaneDouble and DrumsLaneEnd (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs lines 548211 to 548213). Who reads them: a sweep on 2026-11-08 disassembled every method that script.json places in the DLL's il2cpp section (132,238 methods) and kept each instruction that tests, ands, bit-tests, ors or moves a constant touching a lane bit on a memory operand at +0x7C, +0x7D or +0x7E, plus each load from those offsets that the next 12 instructions test, and, shift or bit-test against a lane bit and found 38 hits. docs/audit/2026-11-08-clone-hero-unknowns/W5.md, section NL-56, names about 32 of them and describes six only as float or colour stores in unrelated classes. The tests by name are all in the drum engine #8255: in methods 0x20DCAD0, 0x20DE420 (the lane join), `0x20DE770` and 0x20DEAA0 (the press handler). Seven wide-mask tests in the same class also see a lane flag. The only writers that set a lane bit are the builder's three or lines at 0x20D16BA, 0x20D16DB and 0x20D16F8; every other hit keeps the lane bits or is a float or colour store in an unrelated class. The sweep script was not kept. Routes the sweep cannot see: a mask held in a register, a flag word copied out (to the stack, a struct, a movups copy or another call) and masked later or elsewhere, a read through a shifted base pointer, reflection, code in the native .text section, and patching at run time.
Game code (6142): RVA `0x20D12C0` sets them; the readers found are in class #8255, the methods above.
Hydra: none: Hydra reads no lane flag in scoring.
Ruling: "lanes only affect how lenient the engine is, it doesnt change the underlying notes." (card 2026-10-11-27bb17bc, 03:52 UTC).
Found: 2026-10-10, docs/audit/drum-lanes; the setting lines read 2026-11-08, session 77057df2 (docs-gaps search); the reader sweep run 2026-11-08, session 77057df2.

### NL-57 With Note Shuffle on, how the game draws a lane

Rule: the user saw the game draw only the blue column for a lane on Glide with Note Shuffle on; Hydra's Preview lights every pad the lane's notes land on, as D254 chose. The listings read so far say the game draws one strip for each pad in the pad words of the one or two notes the lane references, read at draw time, so after Note Shuffle it shows those notes' shuffled pads only. Two links in that path are read from how values are used, not from listed code, and no fresh check has passed the reading.
Label: Unknown
Evidence: the user's report, and D112's record says "The routine that draws a lane was not read". The listings show the lane stores references: `docs/audit/drum-lanes/f20d12c0.asm` writes the first chord's reference into the lane at +0x20 (line 268) and, for a double lane, the second chord's at +0x28 (line 272; a single lane stores null there), and the shuffle rewrites each note's 16-bit lane word in place (`docs/audit/note-shuffle/step0/open2/f20f0c30_shuffle.asm` line 317). So the lane's notes are recoloured after the lane is built; that the drawn strip follows them is unread (decision log, D112's correction of 2026-10-17, line 539). The two lane objects are read. The lane as parsed is built by docs/audit/drum-lanes/lane_ctor_2116390.asm lines 1 to 7: two references at +0x10 and +0x18, a byte at +0x20 and the double byte at +0x21. The lane the builder makes (f20d12c0.asm lines 262 to 290) holds a float at +0x18, the two chord references at +0x20 and +0x28, the parsed lane's +0x10 value at +0x30 (its start tick: line 284 subtracts it from a chord's tick), a tick length at +0x38 and the double byte, copied from +0x21, at +0x40. When a lane is added to the list, docs/audit/drum-lanes/f215e4f0.asm lines 41 to 86 put a single lane in front of the double lanes that share its +0x10 value, so a single lane goes before a double at one tick. The draw path was followed on 2026-11-08 (docs/audit/2026-11-08-clone-hero-unknowns/W5.md, section "NL-57 (second pass: the lane renderer)"). The lane's slot-3 interface method returns the +0x80 pad word of the note at +0x20, ORed with the word of the note at +0x28 when the lane's +0x40 byte is set (`code/6142/lane_pad_mask_216A440.asm` lines 2 to 15); it stores no colour of its own. The chart build calls the lane builder before Note Shuffle (`code/6142/chart_build_20D2EA0.asm` line 150, then line 259 on the drum branch). DrumsNoteRenderer.Update calls 0x1166890 (`code/6142/drums_note_renderer_update_1370A0.asm` line 267), which takes the engine's lane list (`code/6142/lane_draw_1166890.asm` line 64) and hands it to the sprite loop 0x41BE70 (line 78). For each lane, the loop calls interface slot 3 to get the pad word (`code/6142/lane_sprite_spawn_41BE70.asm` lines 69 to 74), then puts one sprite per pad with alpha 0.75 (lines 78 to 159). Two links are read from use: that 0x2C44210 is the runtime's interface-call helper whose result is the method's return, and that 0x20FE7E0 with 0x20FC610 and 0x20FC5F0 walks the word's set bits. Searched: every dump.cs line and script.json entry that names the lane class #8641; byte scans for direct calls to the lane's two pad-mask methods (none), to the lane builder (one, 0x20D317E), to the engine's lane-list getter 0x20F4E20 (eight, all in BaseNoteRenderer methods) and to the sprite loop 0x41BE70 (10 call sites in 5 methods, by the rescan in docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md; only the draw path above was read) (direct callers only, byte scan).
Game code (6142): the lane pad word 0x216A440, DrumsNoteRenderer.Update, RVA `0x1370A0`, the lane draw 0x1166890 and the sprite loop 0x41BE70, the listings above. The lane constructor is 0x2116390 and the list insert 0x215E4F0.
Hydra: `light_drum_lanes` in src/parse/song.cpp lines 203 to 213, called at line 2007; with Note Shuffle on it lights `every_lane_pad` (line 213).
Ruling: deliberate difference, D254: "clone hero only draws the blue. this will be an intentional deviation from the game. it looks better with both."; items "Every pad its notes use." and "Only with Note Shuffle on."
Found: 2026-10-17, session 65b16299; the lane layouts and lane order read 2026-11-08, session 77057df2 (docs-gaps search); the draw path read 2026-11-08, session 77057df2.

### NL-58 The game drops a lane whose first note starts too late

Rule: the game drops a lane whose first chord with a non-kick pad, searched from one tick before the lane's start, lands more than a cut-off after the start. The cut-off is the file's resolution times 12 divided by 192, rounded half to even: 30 ticks at resolution 480, a 64th note. At exactly the start plus the cut-off the lane is kept. The cut-off also widens the lane object the builder makes: its start time is taken at the lane's start minus the cut-off. Hydra applies no cut-off and lights any note inside the lane. The user chose to record this difference and keep Hydra as it is.
Label: Unknown
Evidence: the test is in the lane builder, docs/audit/drum-lanes/f20d12c0.asm. Lines 70 to 88 load the song's +0xB8 double, multiply it by 12.0 and divide by the static double at +0x68 of the song class #8538. Lines 98 to 103 round the result through 0x2C49A60 and make it a tick count; `code/6142/lane_cutoff_round_2C49A60.asm` lines 8 to 25 round to the nearest whole number with halves to the even neighbour. The class's static constructor stores 192.0 at +0x68 (`code/6142/lane_divisor_cctor_212E3D0.asm` lines 266 to 269), and dump.cs line 543369 declares that field static readonly. The search at lines 126 to 133 starts from the lane's start minus one, with the mask 0xFE, which leaves the kick out; lines 145 to 147 skip the lane only when that chord's +0x40 tick is strictly greater than the start plus the cut-off. Lines 241 to 247 convert the start minus the cut-off to the lane object's start time, and lines 283 to 286 make its tick length the chord's distance from the start plus twice the cut-off. +0xB8 is the resolution the file states: the song's setter writes it (`code/6142/song_resolution_set_212C0B0.asm` lines 1 to 13), the .chart reader calls the setter with its Resolution value (`code/6142/chart_resolution_read_2164100.asm` lines 139 and 201 to 272), and the .mid reader calls it with the header's ticks per quarter note (`code/6142/midi_resolution_read_2155C00.asm` lines 82 to 94, `code/6142/midi_header_read_211AF60.asm` lines 82 to 86). Still not searched, so the label stays Unknown: writers of the public +0xB8 field outside the setter. A read-only search of the user's library on 2026-11-08 found 75 such lanes in 50 charts, for example Job for a Cowboy - Embedded, whose double lane at tick 273,840 has its first pad chord 240 ticks in (docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md line 21).
Game code (6142): the lane builder, RVA `0x20D12C0`, with the rounding 0x2C49A60, the static constructor 0x212E3D0 and the setter `0x212C0B0`, the listings above.
Hydra: `light_drum_lanes` in src/parse/song.cpp lines 203 to 213 applies no cut-off; its comment at lines 200 and 201 still calls the game's cut-off unknown.
Ruling: D112: chose "Any note inside the lane (Recommended)" (card 2026-10-11-14ef4882, 00:34 UTC). For the cut-off found since, the user chose "Record it only" (docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md line 21); questions page Q-81 records it.
Found: 2026-10-10, D112; the listing lines cited 2026-11-08, session 77057df2 (docs-gaps search); the divisor and the resolution read 2026-11-08, session 77057df2.

### NL-59 A double lane's 0.75-second timer

Rule: for a player who hits every note, lanes change no score, except a double lane's round timer, which Hydra does not model. A round is one press on every pad of the lane. When more than 0.75 s of song clock passes since the current round started, and the lane is double and not finished, the engine breaks the combo and clears the lane. A press on a lane pad crosses that pad off the round; when the last pad is crossed off, the round restarts at that press. A single lane has no such timer. This is a different timer from the 0.075 s grant window of TM-45. The engine keeps the open lane's pad mask at +0x2C4, the pads still owed this round at +0x2C6, a double-lane byte at +0x2C8 and the round's start time at +0x2E0.
Label: Proven in the game's code
Evidence: docs/audit/drum-lanes/play/f20ddec0.asm lines 30 to 50: the song clock at +0x100 minus the round start at +0x2E0 is compared with 0.75 (line 32); the call goes ahead only when the pad mask at +0x2C4 is set (line 34), the double byte at +0x2C8 is set (line 36) and the finished byte at +0x2F1 is clear (line 38); it then makes the virtual call at line 45 and zeroes the lane fields (lines 46 to 50). The twin update `docs/audit/drum-lanes/play/f20de5b0.asm` line 19 makes the same 0.75 compare. The round restart is `docs/audit/drum-lanes/play/f20deaa0.asm` lines 134 to 142: the pressed pad's bit is cleared from +0x2C6, and when nothing is left the clock goes to +0x2E0 and +0x2C6 is refilled from +0x2C4. docs/audit/drum-lanes/FINDINGS-play-time.md lines 25 to 27 name the fields and lines 66 to 73 read the virtual call as the combo breaker (vtable slot 20, `0x20F4B60`). The il2cpp dump confirms slot 20 is that method: C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs line 530260 reads "RVA: 0x20F4B60 ... Slot: 20". The 0.75 sits in the same .rdata slot the probe calls `RVA_HITCHECK_THRESHOLD`. That the game breaks the combo has not been seen in a run; docs/audit/drum-lanes/FINDINGS-play-time.md line 140 says no Expert double lane in the library trips the timer. How a lane fills and clears is read too. The join, docs/audit/drum-lanes/f20de420.asm, takes a lane note whose pads are not yet in +0x2C4, sets the double byte from the note's 0x8000 flag, ORs its pads into +0x2C4 and +0x2C6 and restarts the round clock at +0x2E0; a finished lane is first cleared. A press on lane pads that matches no note crosses them off the round the same way (docs/audit/drum-lanes/play/f20de9a0.asm lines 6 to 9 and 48 to 57). Three hits in a row on notes outside the lane, while it is open, clear it (docs/audit/drum-lanes/f20de770.asm lines 8 to 23: a non-kick note without 0xC000 bumps +0x2EC, and at 3 the lane fields are zeroed). A missed note that is not a kick clears it too (docs/audit/drum-lanes/f20dcad0.asm lines 74 to 80). A finished lane clears once the song clock at +0x100 is more than the constant at 0x3063fb0 past the time at +0x2D8 (docs/audit/drum-lanes/play/f20ddec0.asm lines 17 to 29; the listing's comment on line 19 shows that constant as the double 0.5, so 0.5 s). What +0x2D8 holds comes from FINDINGS-play-time.md line 70, which calls it the last lit note's hit time; the listing alone does not show that.
Game code (6142): the lane update RVA `0x20DDEC0` (`docs/audit/drum-lanes/play/f20ddec0.asm`) and its twin; the press handler RVA `0x20DEAA0`.
Hydra: no owner: lanes are display-only in Hydra.
Ruling: "lanes only affect how lenient the engine is, it doesnt change the underlying notes." and "if not then it doesnt matter" (card 2026-10-11-27bb17bc, 03:52 and 03:55 UTC).
Found: 2026-10-10, docs/audit/drum-lanes; the slot 20 link to dump.cs, the join and the clears added 2026-11-08, session 77057df2 (docs-gaps search).

### NL-60 An activated fill lights only its activation note's lane

Rule: Hydra's Preview lights only the lane of a fill's activation note, as the user's screenshot of the game showed; no code read shows the game's lighting.
Label: Hydra assumes it, never checked
Evidence: docs/audit/drum-lanes/FINDINGS-play-time.md line 50 says the activation flag "is not a lane flag"; the screenshot was not saved.
Game code (6142): not found.
Hydra: the activation lane in src/app/preview_view.cpp lines 435 and 436, from `Chord::activation_note`.
Ruling: "match the clone hero style of highlighting only the lane the activation note is on" (card ee38a528, 20:32 UTC).
Found: session ee38a528.

## What plain Drums drops

### NL-61 On plain Drums the builder folds cymbals and clears dynamics

Rule: when the instrument is not 9 (Pro Drums) the builder runs the cymbal fold and then the dynamics clear; Pro Drums runs a different step instead.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open2-refute/f20D2EA0.asm lines 245 to 253 (the compare with 9 and the two calls); a caller scan on 2026-10-30 found no other call site for either step; its output was not kept.
Game code (6142): builder RVA `0x20D2EA0`; fold `0x20F16E0`; clear `0x20EFEE0`; Pro step `0x20F1420`, which is the disco flip (NL-22).
Hydra: `ChartReading::prodrums` in src/parse/chart_reading.h line 24, read in src/parse/song.cpp lines 2016 and 2025.
Ruling: none needed for the trigger; the clear is NL-62.
Found: 2026-10-28, session 6a549de0; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-62 The plain-Drums clear removes every ghost and accent mark

Rule: the clear ands every note's flag word with 0xFFF3FFFF, which clears DrumsGhostHit (0x40000) and DrumsAccentHit (0x80000), and does nothing else; Hydra keeps scoring dynamics on plain Drums.
Label: Proven in the game's code
Evidence: `code/6142/plain_drums_20EFEE0.asm` lines 17 to 28 (the loop) and line 25 (the and); the flag names are dump.cs lines 548215 and 548216. This settles which bit is which.
Game code (6142): a static method of class #8278, RVA `0x20EFEE0`, the listing above.
Hydra: no owner for the drop: nothing in src/parse/song.cpp or src/core/model.cpp clears dynamics by instrument.
Ruling: deliberate difference: "there is no way this is intentional, it must be a bug in clone hero" (01:30 UTC) and "Keep Hydra as is" (01:44 UTC), card 2026-10-29-6a549de0.
Found: 2026-10-28, session 6a549de0; listed 2026-10-30 in session b02ab3e6 (the reference build).

### NL-63 The cleared bits are the ones the hit handler counts

Rule: the drum hit handler tests the ghost bit and then the accent bit and bumps one of two engine counters, so a cleared mark is never counted.
Label: Proven in the game's code
Evidence: docs/audit/drum-lanes/f20de770.asm lines 62 to 102 (the two getter calls at lines 64 to 69, the counter bumps at lines 82 and 101); docs/audit/note-shuffle/scout-flag/note_getters.txt lines 8 and 29 (the getters test 0x40000 and 0x80000).
Game code (6142): RVA `0x20DE770` (`RVA_NOTE_DYNAMICS` in `constants.py`), the listing above.
Hydra: none.
Ruling: none needed.
Found: 2026-10-30, session b02ab3e6 (the reference build).

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
Evidence: in the builder, `code/6142/note_builder_20D2EA0.asm` lines 245 to 253, the clear is chosen by the instrument byte alone: anything but 9 runs the fold and then the clear. No test of the source format is in those lines, and both parsers end in the same finalizer (NL-34). Still not checked: the builder's callers were not traced for the source format, and no .mid bot run on plain Drums was made.
Game code (6142): RVAs `0x20D2EA0` and `0x215C320`.
Hydra: as NL-62.
Ruling: as NL-62.
Found: 2026-10-28, session 6a549de0; the builder lines cited 2026-11-08, session 77057df2 (docs-gaps search).

### NL-66 On plain Drums every cymbal becomes its tom

Rule: the fold shifts the lane mask right by 3 for any note with a cymbal bit, so each cymbal lands on its tom; Hydra counts no cymbal unless Pro Drums is on.
Label: Proven in the game's code
Evidence: `code/6142/fold_20F16E0.asm` lines 25 and 26 (the cymbal-bit test) and 33 to 35 (the shift and store).
Game code (6142): RVA `0x20F16E0`, the listing above.
Hydra: `cymbal_counts` in src/core/song.cpp line 74 returns true only when Pro Drums is on.
Ruling: none needed: same rule.
Found: 2026-10-30, session b02ab3e6 (the reference build); Hydra side checked by writer W1.

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

Rule: bit 0x20 mirrors the notes, bit 4 removes every note whose lane word is exactly 1 (the kick, on drums), and bit 8 turns each tick into one lane-1 note (one kick, on drums). On drums the mirror moves red to the green tom, the yellow tom to the blue tom, the blue tom to the yellow tom and the green tom to red, leaves the kick, and skips a note whose lane word is exactly 0xF; it then moves the cymbals by the chord's pattern. Hydra models none of the three. The modifiers' names in the game's menu were not looked up.
Label: Proven in the game's code
Evidence: `code/6142/modifier_step_20F1900.asm` takes the instrument and jumps through a 15-entry table; instruments 6 and 9 take the drum branch (its note beside it gives the table read from the DLL). Lines 182 to 196 move the toms and skip the 0xF word (lines 183 and 184), and lines 231 to 259 move the cymbals: a yellow cymbal becomes green, or blue when the chord has a green tom; a blue cymbal stays, or becomes yellow when the chord has a blue tom; a green cymbal becomes yellow, or blue when the chord has a yellow tom. `code/6142/modifier_step_20F1250.asm` lines 36 to 56 null every note whose lane word is 1, and line 98 calls RemoveAll. `code/6142/modifier_step_20F06A0.asm` lines 41 to 68 set every note's lane word to 1 and keep one note per tick, and line 113 calls RemoveAll. The RemoveAll predicate, 0x6D5CB0, tests that the item is null (read by a fresh check, docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md line 6; its listing is not kept). The tie from each bit to its step is in `code/6142/note_builder_20D2EA0.asm`: line 260 tests the register bl against 0x20 before the call to `0x20F1900` at line 265, line 266 tests 4 before the call to `0x20F1250` at line 271, and line 272 tests 8 before the call to `0x20F06A0` at line 277. That bl holds the modifier word is not shown. Lines 163 to 168 call `0x20F1900` under the same 0x20 test on another branch, which was not examined.
Game code (6142): the three steps, RVAs `0x20F1900`, `0x20F1250` and `0x20F06A0`, the listings above.
Hydra: no owner: src/parse and src/core hold only `mode_bass2x_` and the Note Shuffle switch, so a chart played with any of the three scores differently in the game.
Ruling: none found.
Found: 2026-10-30, session b02ab3e6 (the reference build); the three steps read 2026-11-08, session 77057df2.

### NL-73 A builder step rewrites yellow toms and blue cymbals when one byte is 3

Rule: when a byte the builder reads equals 3, it runs 0x20F0320 after the 2x-kick removal. That step moves a yellow tom to the blue tom, or to red when the chord already holds a blue tom or blue cymbal. It moves a blue cymbal to the green cymbal, or to the yellow cymbal when the chord already holds a green tom or green cymbal. It is not the 2x-kick drop, despite its file name. The byte is the chart kind, and 3 is FiveLaneDrums, so the step runs only when a drum chart is built for the game's five-lane drums mode. Hydra has no five-lane mode, so four-lane play never meets this step.
Label: Proven in the game's code
Evidence: the step's listing, `code/6142/two_x_kick_drop_20F0320.asm`, reads each note's +0x80 lane mask. Line 29 tests for 4 (the yellow tom); lines 39 to 51 read the chord's pattern through 0x214BCD0 (the OR of its notes' lane masks, NL-46) and test bits 8 and 0x40; line 60 writes 8 and line 70 writes 2. Line 73 tests for 0x40 (the blue cymbal); lines 83 to 95 test the pattern for 0x10 and, by a sign test, 0x80; line 104 writes 0x80 and line 113 writes 0x20. The builder `code/6142/note_builder_20D2EA0.asm` loads the byte at offset 2 of the object in r14 at line 50, and lines 237 to 242 call the step only when that byte is 3. The lane bits are NL-50's. The builder is a static method whose first argument, a struct (TypeDefIndex 8636) passed by reference, lands in rdx and then r14 (builder lines 3 and 11). That struct's +0x2 field has the byte enum TypeDefIndex 8309, whose names are not scrambled: Guitar = 0, SixFretGuitar = 1, Drums = 2, FiveLaneDrums = 3 (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs line 550659 and the enum it names; enum values are not checked by the docs test).
Game code (6142): the step 0x20F0320 and the builder `0x20D2EA0`, the listings above.
Hydra: no owner: Hydra has no five-lane drum mode and no step like it.
Ruling: none needed: Hydra plays four-lane drums only. Whether Hydra should model five-lane play is a question only if the user raises it.
Found: 2026-11-08, session 77057df2 (docs-gaps search); the byte's enum read 2026-11-08, session 77057df2.

## Side files in a song folder

### NL-70 An expert+.mid beside notes.mid folds the 2x kicks into normal kicks

Rule: some song folders carry an `expert+.mid` next to `notes.mid`. In Synovial's Slipping the `expert+.mid` holds no gem on pitch 95 and 1,217 on pitch 96, which is the 915 normal kicks plus the 302 2x kicks of `notes.mid` (NL-31), with every pad pitch unchanged. So that file puts every 2x kick on the normal kick pitch. Whether the game ever opens `expert+.mid`, or plays `notes.mid` there, is not known.
Label: Unknown
Evidence: the counts are proven from the game's own files, by the user's answer that a fact counted from them counts as code proof (docs/audit/2026-11-08-clone-hero-unknowns/user-answers.md line 4). The files are C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\Xane60\Synovial\Synovial - Slipping\notes.mid (150,822 bytes, MD5 c0576de352a83439b7fe2657e021a9d0, the same file as the Icebreaker 2023 setlist's copy) and expert+.mid in the same folder (150,822 bytes, MD5 54eaeda21db590fad600e4e63c8f05e7). Both are format 1 with 480 ticks per beat, and track 1 is PART DRUMS. In `notes.mid` it has 302 note-ons on pitch 95, 915 on 96, 712 on 97, 235 on 98, 466 on 99 and 409 on 100, and no tick holds both 95 and 96; in `expert+.mid` it has 0 on 95, 1,217 on 96 and the same pad counts. The counts and how they were made are kept in docs/audit/2026-11-08-clone-hero-unknowns/W3.md, section NL-70; the counting script was not kept, and a fresh check recounted both files and checked the ticks (docs/audit/2026-11-08-clone-hero-unknowns/checker-verdicts.md line 42). The opening half is not read: no string literal holds "expert+" (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\stringliteral.json, searched for expert+, expert_plus and expertplus), and the Ghidra decompile holds none either, but the reader of the ".mid" literal (stringliteral.json line 70855) was not read, and a name built at run time would not show in that search. That the game plays `notes.mid` there is session 8157f2dc's reading from the 2x kicks it registered (docs/sessions/2026-09-25-8157f2dc.md).
Game code (6142): not found: the reader of the ".mid" literal was not read.
Hydra: none: Hydra reads only the notes file.
Ruling: none needed.
Found: 2026-09-25, session 8157f2dc; both files counted 2026-11-08, session 77057df2.

## The first Note Shuffle video

### NL-71 The 2026-10-08 video matched the decoded shuffle on 22 of 22 notes

Rule: a bot run on a 480-resolution .mid with Note Shuffle on scored 180 points more than Hydra's unshuffled number, all from toms that became cymbals. The decoded shuffle (NL-40 to NL-46), fed the file's own ticks, put all 22 of the video chart's notes in their filmed positions. Scaling the ticks to 192 per beat matched 11 of 22, and dropping the rule that a new chord's first note avoids the previous chord's colours matched 5 of 22.
Label: Unknown
Evidence: two scouts in session 6c19d0ef decoded the shuffle separately, and both references put all 22 notes of the video's chart ("turkish man yelling meow at an egg", Expert Pro Drums with 2x Kick and Note Shuffle) where the video shows them; its card (docs/sessions/2026-10-09-6c19d0ef.md) says the run was a bot run and all 22 matched. The same session's own record, which is not kept, gives the score gap: the video scored 2,725 against Hydra's 2,545 for the unshuffled chart, because the shuffle turned seven toms into cymbals, two hit at 1x and five at 2x, which is exactly 180. Session cfb6f1fa (2026-10-08) then ran the three probe checks against the same video: the file's own ticks match 22 of 22, ticks scaled to 192 per beat match 11 of 22, and dropping the previous-colours rule matches 5 of 22. Its card (docs/sessions/2026-10-09-cfb6f1fa.md) records all three results. The video is C:/Users/Patrick/Videos/2026-10-08 21-52-37.mp4 (it exists today; the reference build did not watch it). One probe is committed: docs/audit/note-shuffle/probes/order_probe.py runs the reference on the video chart with each chord's notes in lane order and in reverse, and with the ticks rescaled from 480 to 192 per beat (its header, lines 1 to 9). Its header asks whether the 22-tick check can tell chord orders apart. Its output is kept as docs/audit/2026-11-08-clone-hero-unknowns/W5-order_probe-output.txt (2026-11-08): every chord order it tries (lane order, reverse, and kick first with the pads reversed) matches 22 of 22 with the file's own ticks and 11 of 22 with the ticks scaled to 192 per beat. So the check cannot tell chord orders apart. That output confirms the 22 and 11 counts against the filmed positions typed into the probe itself (lines 17 to 23), not against the video. No committed script makes the 5-of-22 check. A local video is not a record (README, "What a block may cite"), so the live half needs a run with kept output.
Game code (6142): not found: a filmed run, not code; the algorithm is NL-40 to NL-46.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp pins rows from later runs (NL-52); no test uses this video.
Ruling: none needed.
Found: 2026-10-08, session 6c19d0ef; the order probe cited 2026-11-08, session 77057df2 (docs-gaps search).

### NL-72 The previous chord's tick starts at 0, so a first note on tick 0 joins a chord

Rule: the shuffle's "previous tick" starts at 0 and is set to each shuffled note's tick after it, so the first non-kick note reads as part of the same chord when it sits on tick 0.
Label: Proven in the game's code
Evidence: docs/audit/note-shuffle/step0/open34/shuffle_20f0c30.asm line 40 zeroes r13, and no line between it and line 209 writes r13. Line 209 compares r13 with the note's tick, the same-chord test. Line 211 reuses r13 for the pattern, and lines 318 and 319 load the note's tick into it for the next note. The reference script docs/audit/note-shuffle/probes/capped/shuffle_ref_capped.py, which an earlier check leaned on, says on line 5 that it was not verified against the game.
Game code (6142): RVA `0x20F0C30`, the listing above.
Hydra: `apply_note_shuffle` in src/parse/note_shuffle.cpp lines 140 (starts at 0), 146 (the test) and 179 (set after each note).
Ruling: none needed: same rule.
Found: 2026-10-31, session b478542c.

## Rows waiting on a live run

NL-12, NL-13, NL-16, NL-19, NL-30, NL-32, NL-34, NL-39, NL-57, NL-58, NL-65, NL-69. Each is listed on the live-checks page.

## Open splits

NL-04 (Q-13), NL-18 (Q-12), NL-19 (Q-10), NL-32 (Q-14). Each is written up on the questions page.
