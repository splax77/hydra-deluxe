# menu_cursor_up_1D6820.asm (build 6142, RVA `0x1D6820`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 1D6820` (66 lines), session 428cf079 (task CH-UNK-W9b). `BaseMenu` virtual slot 23, taking one bool (`dump.cs` line 44907).

Plain words: move the menu's highlight one row up, the mirror of `menu_cursor_down_1D57F0.md`.

- Lines 6 to 12 and 33 to 35: read the scroll +0x3C and the cursor +0x40; when `scrollWholeMenu` (+0x9A) is set, take one from the scroll while it is above 0.
- Lines 13 to 22: take one from the cursor (line 21), unless the cursor is within the top edge +0xE0 and the scroll is above 0, when the scroll goes down by one instead (line 35).
- Lines 23 to 31: at the very top, when +0x2A is clear and `allowWrapping` (+0x98) is set, call slot 54 (offset 0x498) to wrap.
- Line 37: set +0x2E, the same redraw flag as the down move.
- Lines 42 to 59: when the bool argument is set and the row changed, call `0x1A6EF0` with 14 and a float of 0.3, 0.5 or 1.0. That method was not read.

Who calls it: `BaseMenu` slot 21 `0x1D6AE0` calls slot 23 (offset 0x2A8) with `true` and then sets +0x2A (read with chdis, not kept). Not byte-scanned.
