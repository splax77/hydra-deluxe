# replay_header_write_3A5A20: the replay code that sets the song speed (build 6142)

Listing: `replay_header_write_3A5A20.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 3A5A20 700`. The listing stops at 0x700 bytes, before the function ends.

Plain version: this function sets the `song_speed` setting from a number its own replay object holds.

Lines 207 to 212 follow the object's +0x60 and then +0x20 to a second object. Lines 213 to 217 fetch the `song_speed` setting (settings static +0x10). Line 219 reads a whole number from the second object's +0x88, and line 220 calls the int setter `0xFE5F50` with it.

Not read: which class the function belongs to, what fills +0x88, and whether it is the speed a replay file was recorded at. The brief of session aa0ce9ac calls this the replay header's speed.
