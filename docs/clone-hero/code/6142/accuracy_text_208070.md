# accuracy_text_208070: the on-screen "Accuracy / Average" text (build 6142)

Listing: `accuracy_text_208070.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. A protected `BasePlayer` method with no arguments, `dump.cs` lines 48815 and 48816. It was found by a byte scan for references to the string literal "Accuracy: {0:0.0} ms\nAverage: {1:0.0} ms" (slot `0x3C9D928`), which only this method uses (two references, lines 28 and 140).

The method runs only when a setting read through the player object is on: lines 35 to 45 read `BasePlayer.player` (+0x88), its +0x10, that object's +0x90, and call `0x210BF30`; a false result skips to the end. Which setting that is was not read.

Lines 46 to 77 add the float at `BasePlayer`+0xC4 to the float list at +0xC8. When the list already holds 256 values (`0x100`, line 49), the oldest (index 0) is removed first (lines 51 to 54). So the list keeps the last 256 values.

Lines 81 to 102 add up every value in the list as doubles. Lines 128 to 132 divide the sum by the count, flip its sign and multiply by 1000. That is the "Average" number. Lines 120 to 122 flip the sign of the latest value at +0xC4 and multiply by 1000. That is the "Accuracy" number. Line 141 formats both with one decimal, and lines 144 to 148 hand the text to `BasePlayer.AccuracyText` (+0x78, a `TextMeshProUGUI`) through a virtual call.

Nothing in the method adds a constant or a delay. The average is a plain mean of the last 256 hit offsets.
