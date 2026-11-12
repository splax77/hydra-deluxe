# highway_scroll_update_3818E0: HighwayScroll.Update (build 6142)

Listing: `highway_scroll_update_3818E0.asm` beside this file, made 2026-11-08 (session 77057df2) with `tools/ch_mcp/chdis.py` from `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`. `HighwayScroll.Update`, `dump.cs` line 14620.

Line 22 reads the double at +0x78 of the object at +0x48. The `HighwayScroll` field at +0x48 is typed `GameManager` (`dump.cs`, the class's fields from line 14587), so the double is `GameManager.songTimeVideoTime`. Line 62 reads +0x78 of the same object again. The method makes no call to `0x16C490`.
