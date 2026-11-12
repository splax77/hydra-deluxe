# gm_video_time_BEB50: a GameManager method that stores the video time (build 6142)

Listing: `gm_video_time_BEB50.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. A `GameManager` method, `dump.cs` lines 25584 and 25585.

Line 158 calls the video-time getter `0x16C490`. Line 160 stores the result at +0x78, which is `GameManager.songTimeVideoTime` (`dump.cs` line 25305). The rest of the method was not read.
