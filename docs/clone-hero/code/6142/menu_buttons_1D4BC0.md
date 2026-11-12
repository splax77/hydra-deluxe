# menu_buttons_1D4BC0.asm (build 6142, RVA `0x1D4BC0`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 1D4BC0` (407 lines), session 428cf079 (task CH-UNK-W9b). The method is `BaseMenu` virtual slot 33 (`dump.cs` line 44963, TypeDefIndex 824), taking the menu's player object (class #1435, `dump.cs` line 73330 onward: +0x10 the profile, class #451; +0x18 the Rewired `Player`).

Plain words: this is how every menu turns one player's buttons into menu actions. It asks Rewired which buttons went down, came up or were held long, by action id, and calls one virtual method of the menu for each. A menu class reacts to a button by overriding that slot. Virtual calls here read `[vtable + 0x138 + 16 * slot]`, the same layout as `base_frame_update_20F4550.md`.

The action ids are the game's Rewired action table. 0 is Green, 1 Red, 2 Yellow, 3 Blue, 4 Orange, 6 Select and 16 Start: run `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-31\ls-d1a-run3\kit-2.jsonl` names them `mapperGreen` and so on, read from the running game. The names of ids 5, 14, 31 and 32 were not read.

- Lines 13 to 44: leave when the player is not ready (`0x337540`), when this menu ignores that player (+0x2C with +0x9C), when controls are off (+0x9B), when the profile's byte +0xB1 is set, or when one of three editor screens is open.
- Lines 47 to 71: action 32 held with repeat calls slot 18 (offset 0x258); else action 31 held with repeat calls slot 29 (0x308).
- Lines 72 to 85: `bpl` = Green held now. `sil` = the profile's drums test `0xD7F20` (`menu_drums_test_D7F20.md`), or 1 when there is no profile.
- Lines 86 to 93 and 318 to 321: Green down calls slot 31 (0x328).
- Lines 94 to 102 and 313 to 316: Green held long calls slot 57 (0x4C8). Lines 103 to 111 and 308 to 311: Red held long calls slot 26 (0x2D8).
- Lines 112 to 119 and 303 to 306: Green up calls slot 24 (0x2B8).
- Lines 120 to 127 and 298 to 301: Red down calls slot 51 (0x468).
- Lines 128 to 135 and 288 to 296: Yellow down calls slot 21 (0x288) when `sil` is set, else slot 20 (0x278).
- Lines 136 to 143 and 278 to 286: Yellow up calls slot 14 (0x218) when `sil` is set, else slot 45 (0x408).
- Lines 144 to 159: Yellow held long, only when `sil` is clear, calls slot 12 (0x1F8).
- Lines 160 to 167 and 268 to 276: Blue down calls slot 10 (0x1D8) when `sil` is set, else slot 36 (0x378).
- Lines 168 to 175 and 258 to 266: Blue up calls slot 4 (0x178) when `sil` is set, else slot 35 (0x368).
- Lines 176 to 191: Blue held long, only when `sil` is clear, calls slot 55 (0x4A8).
- Lines 192 to 216 and 238 to 257: Orange down calls slot 19 (0x268), up slot 22 (0x298), held long slot 25 (0x2C8).
- Lines 217 to 237: Select (6) down calls slot 38 (0x398); Start (16) down calls slot 39 (0x3A8).
- Lines 322 to 398: when Green is not held and the profile's controller passes `0x210BF30`, two more actions move the cursor through the helper `0x1D6BC0` (not read). Slot 10 gets action 14 and slot 21 gets action 5 when the profile's byte +0x10 is 6 or 9; otherwise slot 10 gets 5 and slot 21 gets 14 (lines 341 to 353). The profile's +0x10 is the instrument byte the player screen writes (`player_screen_instrument_32FB60.md`), and 6 and 9 are Drums and Pro Drums there.

Who calls it: `menu_player_input_1D6B10.asm` line 16 calls slot 33 (offset 0x348) for one player, and `menu_update_1D7060.asm` lines 119 and 153 call that method from `BaseMenu.Update` (slot 47) for each of up to four players (lines 76 to 124) or for one (lines 125 to 153). `PlayerSelectionMenu` does not override slot 33 (its overrides are slots 7, 10, 21, 31, 34, 47, 48, 51 and 53, `dump.cs` lines 72907 to 73019). Found by reading those two listings, not by a byte scan.
