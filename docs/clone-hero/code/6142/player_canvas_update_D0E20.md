# player_canvas_update_D0E20.asm (build 6142, RVA `0xD0E20`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis D0E20` (223 lines). Starts at the method's first instruction: `PlayerSelectionCanvas.Update()` (`dump.cs` line 25774; class header at line 25710, TypeDefIndex 452). Unity calls a MonoBehaviour's `Update` once a frame while the component is enabled.

Plain words: each frame, while its bool at +0x48 is false, the canvas walks its four player screens. For each screen whose GameObject is active, it asks the screen "are you on the Ready page" (`0x32F590`) and counts the screens and the yes answers. When every active screen is ready, and every profile screen passes a check, it fades to the `Gameplay` scene and switches its own `Update` off. A slot whose screen is not active but whose player exists gets its screen switched on.

The fields, from `dump.cs` lines 25713 to 25722: +0x30 is `PlayerSelectionMenu[] players`, +0x40 is `PlayerProfileMenu[] playerMenus`, +0x48 is an unnamed private bool.

- Lines 1 to 12: stack set-up and the one-time loads of three type infos.
- Lines 13 and 18: if the bool at +0x48 is set, jump to line 170 (the other branch).
- Lines 19 to 22: zero the slot index (`ebx`), the active count (`r14d`) and the ready count (`esi`).
- Lines 25 to 42: `players[ebx]`'s GameObject; if it is not active in the hierarchy, go to line 59.
- Lines 43 to 58: for an active screen, add one to the active count (line 53), call `0x32F590` on it (line 54), and add one to the ready count when it returns true (lines 55 to 57).
- Lines 59 to 96: for an inactive screen, read `GlobalVariables.instance.playerList[ebx]` (+0x38, `dump.cs` line 55807); when that player's +0x10 is set, switch the screen's GameObject on (line 96).
- Lines 97 to 99: next slot, four slots in all.
- Lines 100 to 105: go on only when at least one screen is active and all active screens are ready.
- Lines 106 to 129: walk `playerMenus`; a menu that passes `0x1E1920` must also pass `PlayerProfileMenu` method `0x1E74C0`, or the method returns.
- Lines 130 to 162: stop a sound through `BassAudioManager` (line 150), fade to `Gameplay` (lines 156 to 158), and set this component's `enabled` to false (line 162).
- Lines 163 to 169: return.
- Lines 170 to 218: the +0x48 branch. When a test on type `ʳʷʴʾʾˁʾʻʲʹʺ` passes and player 0's Rewired button 0 is pressed down, it hands a new object to `0x322B0` with the value 2. It does not call `0x32F590`.
- Lines 219 to 222: null-reference and range exits.

Who calls it: Unity, as a MonoBehaviour message. No caller search was run for it.
