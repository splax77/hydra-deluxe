# five_lane_convert_215D450 (build 6142, RVA 0x215D450)

The track step that turns a five-pad drum chart into a four-lane Pro Drums chart. It is a no-argument method of the track class #8661 (`dump.cs` line 552109). Its one direct caller is `note_flags_from_markers_215E280.asm` line 90, after the disco step and the within-tick sort; a byte scan of the DLL's call and jump instructions on 2026-11-12 found no other. Made 2026-11-12 with `tools/ch_mcp/chdis.py` (248 lines, the whole method). It is byte for byte the same as docs/audit/note-shuffle/step0/open2-refute/f215D450.asm.

A note's type is its `+0x20` int: 13 kick, 14 red, 15 yellow, 16 blue, 17 the game's GreenDrum (the chart's fifth pad) and 18 the game's OrangeDrum (the chart's fourth pad, which Hydra calls green). Its flag word is `+0x24`: bit 4 (16) is Tom and bit 5 (32) is Cymbal. The four helpers it calls are the chord search `chord_find_type_214DB20.asm`, the Tom-bit test 0x214C5D0, the Cymbal-bit test 0x214DCA0 and the kick test 0x214C8A0 (docs/audit/note-shuffle/step0/open2/getters.asm).

Lines 26 to 37 are the branch test. With track `+0xA1` up and `+0xA2` down, line 29 jumps straight to the five-lane branch. Otherwise line 30 loads the track's song (`+0x50`, class #8538), line 33 the song's settings object (`+0xC8`, class #8267), and line 36 tests that object's `+0x10` byte. Set, it falls through to the five-lane branch; clear, it jumps to the four-lane branch at line 130. `chart_settings_ctor_20E73E0.md` and `songentry_load_35C110.md` say where that byte comes from.

Lines 38 to 118 are the five-lane branch, one pass over the note list `+0x60`. Lines 60 to 70 switch on the type:

- Red (14) and blue (16) go to lines 114 to 117: Cymbal cleared, Tom set.
- Yellow (15) goes to lines 109 to 112: Tom cleared, Cymbal set.
- The fifth pad (17) goes to lines 91 to 108: Cymbal cleared, Tom set, so it is the green tom. Lines 95 to 107 then look for another type-17 note on the chord; if that note is a cymbal, this note becomes type 16, the blue tom (line 107).
- The fourth pad (18) goes to lines 71 to 90: Tom cleared, Cymbal set and type 17, so it is the green cymbal. Lines 76 to 88 then look for another type-17 note on the chord; if that note is a tom, that other note becomes type 16, the blue tom (line 89).
- The kick (13) and any other type are left alone (line 70).

The within-tick sort has already put type 17 before type 18 (NL-33), so on a tick holding both, the fifth pad becomes a green tom first, then the fourth pad becomes the green cymbal and moves that green tom to blue. Either order ends the same way: green cymbal and blue tom.

Lines 130 to 186 are the four-lane branch, an indexed pass over the same list. Types 13 to 15 and 16 are left alone (lines 147 to 157). The fourth pad (18) becomes type 17 with its flags kept (line 158): this is the ordinary four-pad rename. The fifth pad (17, lines 160 to 180) gets Cymbal cleared and Tom set, then looks for a type-18 note on the chord (line 165). If there is one, it looks for a type-16 note (line 172), copies that note's flag word when one is found (lines 178 and 179), and becomes type 16 (line 180).

Lines 187 to 222 run after either branch, and only when both track flags are down: every note that is not a kick gets Cymbal cleared and Tom set.

Only the type and bits 4 and 5 are written, apart from the flag copy at lines 178 and 179. Accent, ghost, Star Power, solo and flam bits stay on the note.
