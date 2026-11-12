# player_menu_on_enable_32BF80.asm (build 6142, RVA `0x32BF80`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32BF80 3000` (478 lines). Starts at the method's first instruction: `PlayerSelectionMenu.OnEnable()` (`dump.cs` line 72986; class header at line 72822). Unity calls `OnEnable` each time the screen's component is switched on, so this is the screen's open method.

Only lines 152 to 206 were read closely, for the writes to the instrument at +0x240 and the difficulty at +0x241 (`dump.cs` lines 72890 and 72891). `rdi` is the screen. The menu's +0x20 is its player (class #1435), and the player's +0x10 is the profile, whose instrument byte is +0x10 and difficulty byte +0x11 (page 7, IN-65).

- Lines 152 to 167: the "is_bot" test `0x13F8B0` on the player picks one of three colours for the object at +0x128.
- Lines 168 to 177: the profile's controller-type getter `0x2123320` (line 175) is stored at +0x1E8 (line 177). This is the copy IN-57 names.
- Lines 178 to 190: when the profile's instrument is 0xF, it is replaced in the profile by `0xD75F0(controller type)` (lines 183 to 190).
- Lines 191 to 198: copy the profile's instrument byte (+0x10) to the screen's +0x240.
- Lines 199 to 206: copy the profile's difficulty byte (+0x11) to the screen's +0x241.
- Lines 207 onward: scene and UI set-up, not read.

Who calls it: Unity, as a MonoBehaviour message. No caller search was run.
