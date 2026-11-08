# gm_update_C05A0: GameManager.Update (build 6142)

Listing: `gm_update_C05A0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. `GameManager.Update`, `dump.cs` lines 25386 and 25387.

Line 157 calls the video-time getter `0x16C490`. Line 159 stores the result at +0x78, which is `GameManager.songTimeVideoTime` (`dump.cs` line 25305). The rest of the method was not read.
