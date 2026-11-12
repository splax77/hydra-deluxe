Listing: `solo_counter_show_294070.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: `SoloCounter` (`dump.cs` line 7999) method, RVA `0x294070` (`dump.cs` line 8030), the whole method. It places the box on screen. It holds no percent or message logic.

Lines 10 and 25 to 28: find the sibling object named `Solo Move`.
Line 41: read its anchored position.
Lines 54 to 58: take 1.9 when a `GlobalVariables` count at `+0x54` is above 1, else 1.0.
Lines 72 to 75: divide the position's x by that count and multiply it by the 1.9 or 1.0.
Line 95: set the box's anchored position.
