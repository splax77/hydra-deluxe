# Add one note's score, RVA 0x20F6EB0 (build 6142), vtable slot 6

Class `#8288`. Listing: `add_note_score_20F6EB0.asm` (chdis, 2026-10-30). Decompiled lines: `ghidra_decompile_all.txt` 53716 to 53771. The drums engine's score function 0x20DEEC0 ends in `jmp 0x1820f6eb0` (committed: `docs/audit/drum-lanes/play/f20deec0.asm` line 31). Fields: +0x42 SP active, +0x48 multiplier (int), +0x94 score, +0xC0 raw points, +0xC8 multiplier share, +0xCC Star Power share, +0xD0 a fourth running int whose meaning this scout did not settle, +0x190 player kind.

In plain words: this is where a note's points go into the score. The caller passes the note and a base value (the drums override passes 50, plus 15 for a cymbal, doubled for a ghost or accent, as session 1bae448b read it on 2026-10-11 (pages 06 SC-01, SC-02 and SC-12); the override is committed as `f20deec0.asm` lines 13 to 25). The function multiplies by the number of gems in the note, adds the plain points to a "raw" total, adds points times the multiplier to the score, and splits the multiplier's extra into two buckets: the combo multiplier outside Star Power, and, inside Star Power, half the multiplier goes to the Star Power bucket.

Lines 1 to 12: prologue and one-time init.
Lines 13 to 14: if the player kind (+0x190) is 1 (RemotePlayer), return with no score added (line 57 onward).
Lines 15 to 16: null note throws (line 81).
Lines 17 to 35: gem count. If the note's int at +0x50 is -1, count the set bits of the note's 16-bit lane mask at +0x80 (loop at lines 29 to 33); otherwise the count is the int at +0x50.
Lines 36 to 38: points = gem count times the base value (r8d, the third argument); +0xC0 gets that added (line 38).
Lines 39 to 41: +0x94 (the score) gets points times the multiplier (+0x48).
Lines 42 to 44: branch on +0x42. Zero (Star Power off) falls through to line 45; non-zero jumps to line 62.
Lines 45 to 50 (off): +0xC8 gets (multiplier minus 1) times points.
Lines 51 to 54 (off): call the note's flag test 0x18214BBA0 (tests bit 0x20 of the note's dword at +0x84, read 2026-10-30); if set, +0xD0 loses (multiplier minus 1) times points.
Lines 55 to 61: restore registers and return.
Lines 62 to 65 (on): half = multiplier divided by 2 (signed). Star Power doubled the multiplier at activation, so half is the multiplier the player would have had without Star Power.
Lines 66 to 70 (on): +0xC8 gets (half minus 1) times points.
Lines 71 to 73 (on): +0xCC gets half times points. So inside Star Power the Star Power share is one extra copy of the points at the normal multiplier.
Lines 74 to 80 (on): call the note's flag test 0x18214B600 (bit 0x10 of the dword at +0x84); if set, +0xD0 gains half times points; return.
Line 81 to 82: null-reference throw.

Unsettled: what +0xD0 and the two note flags mean. This scout did not read the readers of +0xD0.
