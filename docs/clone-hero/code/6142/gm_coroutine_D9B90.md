# gm_coroutine_D9B90: a GameManager coroutine step (build 6142)

Listing: `gm_coroutine_D9B90.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. A `MoveNext` at `dump.cs` lines 25220 and 25221.

Line 148 calls the video-time getter `0x16C490`. Line 150 stores the result at +0x78 of the object in rbx, and line 153 passes that object to the `GameManager` method `0xBEA90`. Lines 118 to 121 also read and rewrite the same +0x78. That rbx is the `GameManager` is read from the call at line 153, not from the coroutine's fields. The rest was not read.
