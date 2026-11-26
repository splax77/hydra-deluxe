# endofsong_speed_restore_160260: an end-of-song method that puts the song speed back (build 6142)

Listing: `endofsong_speed_restore_160260.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 160260 600`.

Plain version: it sets the `song_speed` setting to the number the game saved in `GlobalVariables` at +0x5C.

Lines 41 to 43 fetch the `song_speed` setting (settings static +0x10). Lines 47 to 49 fetch the `GlobalVariables` instance (static +8). Line 56 reads the whole number at its +0x5C, and line 57 calls the int setter `0xFE5F50` with it. Lines 173 to 180 do the same on a second branch.

The setter ends in the clamp (`int_setting_clamp_FE5B10.md`), so a saved number of 0 leaves the setting at its minimum, 10.

Who calls it (byte scan of direct calls, 2026-11-25): `0x160260` from 0x161163, `0x160650` from 0x1423CF and 0x164196, `0x1637E0` from 0x161A5E. Those sit in `EndOfSong`'s methods at `0x160BA0`, `0x161480` and `Update` (`0x163BB0`), and in the method at `0x142330`, which finds an `EndOfSong` object first and does nothing when none exists (`endofsong_restore_caller_142330.md`). None of the four is inside `EndOfSong.Start` (`endofsong_start_speed_read_15DF80.md`), and `0x160BA0`, `0x161480`, `0x142330` and `0x163BB0` have no direct callers, so they run through method pointers. `GlobalVariables` +0x5C is the field `originalSongSpeed` (`dump.cs` line 55813). Not read: who writes it, and the pause menu's own calls of the setter.
