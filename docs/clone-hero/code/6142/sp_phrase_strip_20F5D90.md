# sp_phrase_strip_20F5D90.asm (build 6142, RVA `0x20F5D90`)

Made 2026-11-12 with `py -m tools.ch_mcp.chdis 20F5D90 0x500` (115 lines). Starts at the method's first instruction. The method is slot 22 of the base engine class #8288, a protected virtual method with no parameters and a scrambled name (dump.cs lines 530287 and 530288). Neither drum class, #8253 or #8255, overrides slot 22 (dump.cs lines 527281 to 527585).

Plain words: walk the notes forward from the engine's note cursor and take the Star Power marks off them, up to and including the phrase's last chord. A note that is not the phrase's last chord loses its phrase flag 0x80. On the first note that carries the award flag 0x100 (SP-04), the walk goes through that note's chord and takes both 0x80 and 0x100 off every note in it, then returns. With 0x100 gone, the hit handler has nothing to credit when that chord is hit (page 5, SP-48).

- Lines 1 to 16: stack set-up and the one-time load of four generic method slots.
- Lines 17 to 34: build an enumerator over the engine's note list at +0x1E0, starting at the index held in +0x50 (the note cursor, read as a 32-bit int at line 17), with a count of 0x7FFFFFFF, so it runs to the end of the list.
- Lines 49 to 51: step to the first note; an empty walk returns (line 75).
- Lines 53 to 58: take the current note; a null note goes to the null exit (line 113).
- Lines 59 to 63: call the note getter 0x214B5F0, which tests flag 0x100 on the note's +0x7C (`docs/audit/note-shuffle/scout-flag/note_getters.txt` line 2). If it is set, jump to line 79.
- Lines 64 to 69: call the note getter 0x214BE20, which tests flag 0x80 on +0x7C (same file, line 30), and when it is set clear it (`and [note+0x7c], 0xffffff7f`, line 69).
- Lines 70 to 74: step to the next note and loop back to line 53; at the end of the list, return (lines 75 to 78).
- Lines 79 to 99: for the award note, call the note method 0x214B9C0 and build a second walk from what it returns, then step it with 0x2141DF0, a method of a class nested in the note class. The flow fits a walk over the members of that note's chord; the two callees were not read.
- Lines 100 to 108: for each member, clear both flags at once (`and [note+0x7c], 0xfffffe7f`, line 103) and step to the next.
- Lines 109 to 112: return once the chord's walk ends. So the outer walk stops at the first award chord and never reaches a later phrase.

Callers: the combo break, slot 20 at `0x20F4B60`, calls slot 22 through `[vtable+0x298]` when the engine's byte +0x46 is set, and then clears that byte (`docs/audit/drum-lanes/play/f20f4b60_combo_break.asm` lines 17 to 23; (0x298 - 0x138) / 16 = 22). No other caller was searched for.

Not read: where the cursor at +0x50 sits when the method runs (if it were already past the phrase's first note, the walk would start mid-phrase; if it were past the award chord, it would strip the next phrase instead); the two callees at lines 82 and 97; and the meaning of the four generic slots. That the outer walk runs forward in list order, and that the inner walk covers one chord, rests on the code's shape, not on a read of those callees.
