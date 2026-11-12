# menu_player_input_1D6B10.asm (build 6142, RVA `0x1D6B10`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 1D6B10` (39 lines), session 428cf079 (task CH-UNK-W9b). A private `BaseMenu` method taking one menu player (`dump.cs` line 44874).

Plain words: it remembers which player is driving the menu, runs the button method for that player, and forgets the player again.

- Lines 4 to 8: store the player at menu +0xF8.
- Lines 12 to 16: call virtual slot 33 (offset 0x348), the button method `0x1D4BC0` (`menu_buttons_1D4BC0.md`).
- Lines 18 to 23: clear +0xF8. Lines 25 to 33 are the same clean-up on the exception path.

Who calls it: `menu_update_1D7060.asm` lines 119 and 153. Not byte-scanned for other callers.
