# two_x_kick_drop_20EFF60 (build 6142, RVA 0x20EFF60), with double_kick_test_214B850
Static method of class #8278; rcx = the play-note list. double_kick_test_214B850 is the whole predicate: test [note+0x7C], 0x100000 (DrumsDoubleKick), true when set.
Lines 1 to 22: one-time setup (a RemoveAll predicate and generic pointers).
Lines 23 to 38: loop over the list; line 37 calls the predicate; line 38 skips notes without the flag.
Lines 39 to 65: if the note has a previous note (+0x10), set that note's next pointer (+0x18) to this note's next.
Lines 66 to 93: if the note has a next note, set that note's previous pointer to this note's previous.
Lines 94 to 98: blank the list slot (set_Item with null). Lines 99 to 117: loop end.
Lines 118 to 140: build the cached "is null" predicate once, then tail-call List.RemoveAll with it (line 140). Net effect: every 2x kick leaves the list and the chord chain.
The caller skips this whole call when bit 2 of its options byte is set (f20d2ea0_build.asm lines 231 to 236).
