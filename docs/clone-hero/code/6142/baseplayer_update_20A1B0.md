# baseplayer_update_20A1B0: BasePlayer.Update hands the engine the song time (build 6142)

Listing: `baseplayer_update_20A1B0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. The method is `BasePlayer.Update` (`dump.cs` line 48872, class `BasePlayer`, TypeDefIndex 901). `BasePlayer` keeps `gameManager` at +0x20 (`dump.cs` line 48720) and `engine` at +0x120 (line 48770).

Plain version: every time this method passes a time to the engine, the time is `GameManager.songTime` (+0x70, `dump.cs` line 25304). It never reads `songTimeVideoTime` (+0x78).

Line 71 reads `gameManager.songTime` to test it against 0. Lines 102 to 105 pass it to the player's own virtual method at +0x378, which fills a time at `[rsp + 0x40]`; line 114 hands that time to the engine method `0x20F4550` (a `#8288` method taking a double and a bool, `dump.cs` line 530128). Lines 125 to 129 pass `songTime` to `0x208F50`, and line 138 hands the time it fills to `0x20F4550` again. Lines 152 and 155 pass `songTime` itself to `0x20F4550`, and lines 162 and 164 pass it to the engine method `0x20F3500`. No line in the method reads +0x78 of any object.

What it does not show: what the player's virtual method at +0x378 and `0x208F50` compute from `songTime` (both return a time below or at it, by their use in a loop; not read).
