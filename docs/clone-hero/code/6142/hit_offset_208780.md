# hit_offset_208780: the hit offset the accuracy text shows (build 6142)

Listing: `hit_offset_208780.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. A `BasePlayer` virtual method, slot 34, that takes one note object, `dump.cs` lines 48845 and 48846. It is the only direct caller of `accuracy_text_208070` (a byte scan finds one jump, line 34; direct callers only).

Lines 6 to 19 call the `BrutalFader` and `ComboColor` methods; what they do was not read. Lines 25 to 30 take the note's time (the double at the note's +0x28, the note class at `dump.cs` line 548368), subtract `GameManager.songTime` (+0x70 of `BasePlayer.gameManager`, `dump.cs` line 25304), turn the result into a float and store it at `BasePlayer`+0xC4. Line 34 then jumps to `accuracy_text_208070`.

So the stored value is the note time minus the song time at the moment this method runs, in seconds. The text flips its sign, so a positive shown number means the hit was handled after the note's time. `songTime` is a stored field, not a press time stamp: `GameManager.Update` stores it from the clock getter `0x16C480` (`gm_update_C05A0.asm` lines 151 to 153), and three other `GameManager` methods store it too. So the shown offset includes any time between the press and the moment this method runs.

Who calls slot 34 was not traced: a virtual call cannot be found by a direct-call scan.
