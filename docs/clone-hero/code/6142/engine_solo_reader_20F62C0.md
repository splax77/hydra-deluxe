# engine_solo_reader_20F62C0.asm  (build 6142, RVA 0x20F62C0, a method of the engine base class #8288, the solo start)
Made with chdis.py over the whole method (the next method starts at 0x20F6390), 2026-11-08, session 77057df2. rbx = the engine.
Lines 12 and 13: do nothing if the in-solo byte (+0x43) is already set.
Lines 14 to 23: take the built solo record at index +0x160 of the list at +0x200 (class #8291, made by 0x20D1ED0) and copy its note count (+0x10) to +0x16C.
Lines 24 to 36: call the record's getter 0x210DAE0 and store the double at +0x170.
Lines 35 to 41: call a listener at +0x1A0 through an interface slot. Line 42 sets the in-solo byte.
