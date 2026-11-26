# endofsong_speed_restore_160260: an end-of-song method that puts the song speed back (build 6142)

Listing: `endofsong_speed_restore_160260.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 160260 600`.

Plain version: it sets the `song_speed` setting to the number the game saved in `GlobalVariables` at +0x5C.

Lines 41 to 43 fetch the `song_speed` setting (settings static +0x10). Lines 47 to 49 fetch the `GlobalVariables` instance (static +8). Line 56 reads the whole number at its +0x5C, and line 57 calls the int setter `0xFE5F50` with it. Lines 173 to 180 do the same on a second branch.

The setter ends in the clamp (`int_setting_clamp_FE5B10.md`), so a saved number of 0 leaves the setting at its minimum, 10.

Not read here: the class this method belongs to, its two sibling methods, the pause menu's methods, and who writes +0x5C. The brief of session aa0ce9ac names them (EndOfSong `0x160260`, `0x160650`, `0x1637E0`; song select writes +0x5C).
