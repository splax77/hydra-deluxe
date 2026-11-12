# player_screen_red_32CBE0.asm (build 6142, RVA `0x32CBE0`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32CBE0` (84 lines), session 428cf079 (task CH-UNK-W9b). `PlayerSelectionMenu` override of `BaseMenu` slot 51 (`dump.cs` line 72965), which `menu_buttons_1D4BC0.asm` calls on Red down.

Plain words: Red goes back. From Instrument, Difficulty, Modifiers or Ready it returns to the Confirm page. From Confirm it leaves the player screen. Three flags can stop it.

- Lines 11 to 21: on the Ready page (4), do nothing when `CHNetManager` method `0x145FE0` returns true (call at line 19).
- Lines 22 to 24: call the base slot 51 `0x1D5250`.
- Lines 25 to 35: switch on the page.
- Lines 36 to 43: Ready (4) sets the page to 0 with `0x330820` and calls `0x3308D0`.
- Lines 44 to 50: Instrument (1) sets the page to 0 only when +0x246 is clear (lines 44 and 45); Difficulty (2) and Modifiers (3) jump straight to the set at lines 46 to 49.
- Lines 51 to 66: Confirm (0) calls `PlayerSelectionCanvas` method `0xD09F0` on the canvas at +0x1C8 (line 66), unless `0x145FE0` returns true or +0x244 is set (line 60). That canvas method was not read.
- Lines 67 to 78: when +0x245 is set, call `0x92910` with 1 (not read).

Who calls it: `menu_buttons_1D4BC0.asm` lines 298 to 301 (virtual slot 51). Found by reading that listing, not by a byte scan.
