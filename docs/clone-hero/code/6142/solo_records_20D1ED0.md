# solo_records_20D1ED0.asm  (build 6142, RVA 0x20D1ED0, a static method of class #8265, the solo record builder)
Made with chdis.py over the whole method (the next method starts at 0x20D2440), 2026-11-08, session 77057df2. It takes a track (rbx) and the track's note list (r14) and returns a list of built solo records (class #8291), which goes into the note-list struct at +0x20.
Lines 53 to 62: read the track's instrument byte (+0x58) and its solo list (+0x78); the drum flag is set for instrument 6 or 9.
Lines 74 to 98: r15 = the first note's tick (+0x40); rdi = the last note's tick plus its length (+0x40 plus +0x48).
Lines 99 to 124: walk the track's solo list; skip a solo whose start (+0x10) plus length (+0x18) is below the first note's tick, or whose start is past the last note's end.
Lines 125 to 200: walk the notes from the solo's start. The first note for which 0x214B630 is true (the SoloBegin flag 0x200 at note +0x7C) becomes the first note (lines 167 to 174). Each note adds one to the count unless 0x214BCC0 is true for it (lines 177 to 184). The walk stops at the first note for which 0x214B7D0 is true (the SoloEnd flag 0x400), after that note was counted (lines 195 to 200). Lines 185 to 194 add a drum-only test (0x214BA20) that was not read.
Lines 201 to 234: a new record gets the count at +0x10, the first note's time at +0x18, the time span at +0x20, the first note's tick at +0x28, the last note's end tick at +0x30 and the tick span at +0x38, and is added to the list.
Not read: what 0x214BCC0 and 0x214BA20 test.
