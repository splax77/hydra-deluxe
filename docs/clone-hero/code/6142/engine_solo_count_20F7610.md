# engine_solo_count_20F7610.asm  (build 6142, RVA 0x20F7610, a method of the engine base class #8288, the solo end and its bonus)
Made with chdis.py over the whole method (the next method starts at 0x20F7830), 2026-11-08, session 77057df2. rbx = the engine.
Lines 12 and 13: do nothing unless the in-solo byte (+0x43) is set; line 18 clears it.
Lines 25 to 49: when +0x164 is at least 50 (0x32), the bonus is ((hit / total) - 0.5) x 2 x hit x 10, rounded by 0x182C49A60 (a function the dump does not name) and multiplied by 10, where hit is +0x168 and total is +0x16C (the record's note count, copied by 0x20F62C0). Below 50 the bonus is 0.
Line 50: the bonus is added to +0xA0.
Lines 51 to 89: tell the listener at +0x1A0 through an interface slot, with the bonus, +0x164, the double at +0x170 and a flag that is true when +0x168 equals total minus 1 (lines 56 to 62); what that flag means was not worked out.
Lines 92 to 105: step the record index (+0x160) unless it is the last one, and zero +0x164, +0x168, +0x16C and +0x170.
Not read: what +0x164 and +0x168 hold and which code adds to them.
