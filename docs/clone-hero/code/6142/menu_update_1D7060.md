# menu_update_1D7060.asm (build 6142, RVA `0x1D7060`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 1D7060` (275 lines), session 428cf079 (task CH-UNK-W9b). The method is `BaseMenu.Update`, virtual slot 47 (`dump.cs` line 45036).

Plain words: once a frame, a menu hands each player's input to its button method. Only the input part is annotated here.

- Lines 30 to 53: a fade check, then a flag at +0x9B is set from a player count and slot 48 (offset 0x438) is called when it changes.
- Lines 55 to 65: when the menu has its own player at +0x20, it asks slot 30 (offset 0x318) whether that player may drive it, then calls `0x1D6B10` for it (lines 151 to 153).
- Lines 66 to 124: otherwise, when a global has no single player at +0x40, it walks up to four players in a global list (+0x38, lines 85 to 90, the count test at lines 120 to 122). For each one that is ready and allowed (lines 95 to 115, the same tests as `menu_buttons_1D4BC0.asm` lines 17 to 30) it calls `0x1D6B10` (line 119).
- Lines 125 to 153: when that global does have a single player at +0x40, it does the same for that one player.

Who calls it: Unity calls `Update` on every enabled MonoBehaviour; `PlayerSelectionMenu.Update` `0x3314E0` overrides it. No byte scan was run.
