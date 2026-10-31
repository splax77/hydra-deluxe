# sp_phrase_advance_20D0330 (build 6142, made 2026-10-30 with tools/ch_mcp/chdis.py)

Function: the compiler-generated cursor step `#8265.AdvancePhraseCursor(ref #8264)` (`dump.cs` line 528398), RVA `0x20D0330`; the first instruction is the function start. `#8264` is the phrase cursor struct built by `sp_phrase_assign_20D2440`: index at +0x00, phrase list at +0x08, current phrase start at +0x10, current phrase end at +0x18.

Lines 1 to 10: prologue and the one-time setup of generic method pointers.

Lines 11 to 13: the cursor's list pointer is read; a null list jumps to the null-reference throw (line 31).

Lines 14 to 17: the index is compared with count minus one (`cmp [cursor], count-1; jg`). If the index is past the last phrase, the function returns without touching start and end. So after the last phrase the cursor keeps the last phrase's start and end, and its index keeps counting.

Lines 18 to 23: otherwise the phrase at the index is fetched; a null phrase jumps to the throw.

Lines 24 to 27: the cursor's start becomes phrase+0x10 and its end becomes start plus phrase+0x18.

Lines 28 to 30: return. The caller incremented the index before the call, so one call moves the cursor one phrase on.

Lines 31 to 33: null-reference throw.
