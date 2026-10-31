# Advance the engine's event clock, RVA 0x20F6A50 (build 6142), vtable slot 8

Class `#8288`. Listing: `advance_events_20F6A50.asm`, lines 1 to 243 (chdis printed on into the next function; lines after 243 were cut). Decompiled lines: `ghidra_decompile_all.txt` 53581 to 53712. Signature from script.json: `(this, double time, bool = True)`.

This is not a score adder. The decompile shows no write to the score (+0x94) or to any score bucket. `docs/audit/ch-evidence.md` question 222 (line 177) and il2cpp-fields.md line 26 group it with the adders; the code does not support that.

In plain words: it moves the engine's event clock forward to `time`. For every kind of timed event whose time falls between the old clock (+0x100) and the new time, it calls the same virtual method (vtable offset 0x238, `call qword ptr [rax + 0x238]`) with that event's time. The kinds are the note windows, the next items in the engine's lists, and, last of all, the Star Power end.

Only lines 219 to 230 below were checked against the listing line by line. The other line ranges are approximate and follow the decompile's order; treat them as a guide, not as proof.

Lines 1 to 28 (about): prologue, one-time init check.
Lines 29 to 115 (third argument non-zero, the default): walk the note list starting at the current index (+0x50) and call the virtual method for the early and late edges of each note's hit window that fall inside (old clock, new time) (decompile 53608 to 53660). It stops at the first note whose edges are both at or after the new time.
Lines 116 to 195: the next time-signature or sync-track item, the next fill or section item, and a time derived from the current section, each tested the same way: strictly after the old clock and strictly before the new time (decompile 53666 to 53704).
Lines 196 to 218: one more time, derived from the next list item minus a clamped constant (call 0x180F96990), tested the same way.
Lines 219 to 230 (the Star Power end): if the active flag (+0x42, line 219) is set, load the end time at +0x120 (line 221) and compare it with the old clock (+0x100, line 222) and the new time (xmm8, line 224). Only when old clock is below the end time and the end time is below the new time does it call the virtual method with the end time as the event time (lines 226 to 230). So the end is a one-off event the first update after the clock passes +0x120. It is not a per-frame compare of the meter with zero.
Lines 231 to 241: restore registers and return.
Lines 242 to 243: null-reference throw.

Not proven here: that the virtual method at offset 0x238 turns Star Power off. It is implemented in the derived classes (the drums engine is class 8255 per il2cpp-fields.md line 51), which this scout did not read.

Reviewer note, 2026-10-30: the handler at vtable offset 0x238 (slot 16) is RVA 0x20F43A0 in the base class. It adds the time to a list of doubles at +0x180 and never touches +0x42, so this call does not end Star Power. See page 5, SP-28.
