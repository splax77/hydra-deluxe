Listing: `player_solo_percent_2086C0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: `BasePlayer` virtual method, slot 49, RVA `0x2086C0` (`dump.cs` line 48937), the whole method. It takes one int, the percent. A byte scan found no direct call to it; it is reached through the class's virtual table, and no call through that slot was found.

Lines 11 to 16: pass the int to the `SoloCounter` (`+0x58`) percent method `0x294C60` (`solo_counter_percent_294C60.md`).
Lines 17 to 45: in an online game, pass it on through `+0x118`; not traced.
