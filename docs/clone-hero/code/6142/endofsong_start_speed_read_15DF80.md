# endofsong_start_speed_read_15DF80: where EndOfSong.Start reads the song speed for the stats file (build 6142)

Listing: `endofsong_start_speed_read_15DF80.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 15DF80 F0`. It is a slice of `EndOfSong.Start` (`0x15DC20`, `scorestats_writer_15DC20.md`), not the whole method; it starts on an instruction, not on the method's first byte.

Plain version: Start fetches the `song_speed` setting object and hands its current value to the stats object's fill method `0x252200`. This is the one place the stats file's speed comes from.

Line 28 loads the settings class's TypeInfo slot at 0x3C98320, line 29 its statics, and line 30 takes static +0x10, the `song_speed` object (`settings_load_374D50.asm` lines 2028 to 2030). Line 46 reads that object's +0x10, the current value, into `r8d`. Line 48 calls `0x252200`. So the speed written to the stats file is the value in the setting at the moment Start reaches line 46.

None of the four call sites of the restore methods `0x160260`, `0x160650` and `0x1637E0` lies inside Start. They are at 0x161163, 0x1423CF, 0x164196 and 0x161A5E (byte scan, 2026-11-25; `endofsong_speed_restore_160260.md`), and Start has no direct caller. Not shown: that no call through a method pointer from inside Start reaches a restore method before line 46.
