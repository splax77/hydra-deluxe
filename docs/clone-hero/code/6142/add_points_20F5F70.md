# Add plain points, RVA 0x20F5F70 (build 6142)

Class `#8288`. Listing: `add_points_20F5F70.asm` (chdis, 2026-10-30). Decompiled lines: `ghidra_decompile_all.txt` 53118 to 53138. Signature from script.json: `(this, int points, note)`; the note argument is never read. The brief and il2cpp-fields.md call this one of the "score adders".

In plain words: it adds a ready-made number of points (not a note gem) to the score with the same multiplier and Star Power split as the note adder. Direct callers found by a byte scan of the DLL are three generic engine wrappers (RVAs 0xFAFEF0, 0xFBFA10 and 0xFBFC70); the drums class does not call it directly (the drums score function jumps to 0x20F6EB0 instead). A byte scan can miss calls made through a vtable, so "not used by drums" is not proven.

Lines 1 to 4: if the player kind (+0x190) is 1 (RemotePlayer) return (line 31). Same for points of zero or less (lines 5 to 6).
Lines 7 to 10: points times the multiplier (+0x48) added to the score (+0x94).
Lines 11 to 15: +0xC4 gets points added. (+0xC4 is a different bucket from the note adder's +0xC0.)
Line 14: branch on +0x42 (Star Power active).
Lines 16 to 21 (off): +0xC8 gets (multiplier minus 1) times points; return.
Lines 22 to 30 (on): half = multiplier divided by 2 (signed); +0xCC gets half times points (line 29); +0xC8 gets (half minus 1) times points (line 30).
Line 31: return.
