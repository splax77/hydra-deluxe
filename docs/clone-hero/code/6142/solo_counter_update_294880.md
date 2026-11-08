Listing: `solo_counter_update_294880.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: `SoloCounter` (`dump.cs` line 7999) method, RVA `0x294880` (`dump.cs` line 8036), the whole method. It has the same parameters and the same shape as the end-message method `0x2960B0` (`solo_counter_result_2960B0.md`), but nothing found calls it.

Lines 21 to 28: it tests the first int against 8, -18, -48 and -11, which are not percents.
Line 39: one branch reads the message array at offset -0x188, which is outside any array.
Line 17: it sets the done byte `+0x50` to 0, where `0x2960B0` sets it to 1.

A byte scan for call instructions found no direct caller. Three more methods of the class with the same parameters, `0x2946F0`, `0x295AE0` and `0x295850`, also test the first int against values that include negative numbers, which no percent can be, and also have no direct caller.
