# stats_song_fields_252200: the stats object's song-field fill (build 6142)

Listing: `stats_song_fields_252200.asm` beside this file, made 2026-11-25 (session aa0ce9ac) with `py -m tools.ch_mcp.chdis 252200 160`. It is the first 85 lines of the method, up to its call that reads the current time; the method goes on past that.

Plain version: it fills the stats object's song fields, and its first int argument becomes the stats object's speed field.

Line 8 keeps the third argument `r8d` in `ebp`, and line 77 stores it at the stats object's +0x38 (`rbx` is the stats object). `scorestats_json_252630.asm` lines 138 to 140 write the JSON key `playback_speed` from that same +0x38. The caller `EndOfSong.Start` passes the `song_speed` setting's value as that argument (`endofsong_start_speed_read_15DF80.md`). Line 78 stores the fourth argument at +0x48, and line 76 stores a stack argument at +0x4C.

The method's other direct caller is at 0x1620C5. Its twin `0x253060` has no direct caller (byte scan, 2026-11-25) and was not read.
