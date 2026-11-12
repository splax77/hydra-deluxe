# player_screen_down_32E710.asm and player_screen_up_330F30.asm (build 6142)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32E710` and `py -m tools.ch_mcp.chdis 330F30` (31 lines each), session 428cf079 (task CH-UNK-W9b). `PlayerSelectionMenu` (TypeDefIndex 1429) overrides of `BaseMenu` slot 10 (`dump.cs` line 72911) and slot 21 (line 72995).

Plain words: on the player screen, the down and up moves do what every menu does, except on the Ready page, where they do nothing.

`player_screen_down_32E710.asm`:
- Lines 9 to 10: when the page (+0x120) is 4, Ready, return at once (line 27).
- Lines 11 to 13: call the base slot 10 `0x1D5290`, which moves the highlight down (`menu_cursor_down_1D57F0.md`).
- Lines 14 to 25: when +0x245 is set, call `0x92910` on a static object with 3. That method was not read.

`player_screen_up_330F30.asm` is the same shape: lines 9 to 10 skip the Ready page, lines 11 to 13 call the base slot 21 `0x1D6AE0` (the up move, `menu_cursor_up_1D6820.md`), and lines 14 to 25 call `0x92910` with 2.

Who calls them: `menu_buttons_1D4BC0.asm` calls slot 10 for Blue down and slot 21 for Yellow down when the profile's drums test is true (lines 128 to 135, 160 to 167, 268 to 296), and for actions 5 and 14 (lines 357 to 386). Found by reading that listing, not by a byte scan.
