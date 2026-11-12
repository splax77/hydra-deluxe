# menu_cursor_down_1D57F0.asm (build 6142, RVA `0x1D57F0`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 1D57F0` (81 lines), session 428cf079 (task CH-UNK-W9b). `BaseMenu` virtual slot 42, taking one bool (`dump.cs` line 45015).

Plain words: move the menu's highlight one row down. It adds one to the cursor (+0x40), or to the scroll (+0x3C) once the cursor is at the scrolling edge (+0xDC). On the last row it wraps to the top when the menu allows wrapping.

- Lines 4 to 7: read the cursor +0x40 and the scroll +0x3C.
- Lines 8 to 9 and 45 to 47: when `scrollWholeMenu` (+0x9A) is set, add one to the scroll while it is below +0x44.
- Lines 10 to 15: the last row is +0x44 - 1 + +0x48 (+0x48 is the visible-row count, IN-58). When cursor + scroll is that row, go to the wrap test.
- Lines 16 to 37: compare the cursor with the edge +0xDC and the scroll with +0x44; then add one to the scroll (line 34) or to the cursor (line 36).
- Lines 38 to 44: the wrap. When +0x2B is clear and `allowWrapping` (+0x98) is set, cursor and scroll both go to 0 in one 8-byte write (line 42).
- Line 48: set +0x2E, which `menu_update_1D7060.asm` lines 154 to 160 turn into a call of slot 43 on the next frame.
- Lines 49 to 58: bookkeeping at +0xE4.
- Lines 59 to 76: when the bool argument is set and the row changed, call `0x1A6EF0` with 15 and a float of 0.3, 0.5 or 1.0 picked by +0x11C and +0x11D. That method was not read.

So the highlighted row is cursor + scroll, which `BaseMenu` method `0x1D5C00` returns (read with chdis, not kept: `mov eax, [rcx+0x40]; add eax, [rcx+0x3c]; ret`).

Who calls it: `BaseMenu` slot 10 `0x1D5290` calls slot 42 (offset 0x3D8) with `true` and then sets +0x2B (read with chdis, not kept). Not byte-scanned.
