# player_screen_confirm_ready_32FF70.asm (build 6142, RVA `0x32FF70`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32FF70` (71 lines), session 428cf079 (task CH-UNK-PAGES-FIX). The listing ends at the `int3` padding after the method's last call. `PlayerSelectionMenu` method with no arguments besides the screen (`dump.cs`, the method at RVA 0x32FF70).

Plain words: this is the step Green's Confirm row 0 ends in. It goes to the Ready page only when the screen's byte +0x244 is clear. When +0x244 is set, it hands the chosen instrument and difficulty on instead, and never writes Ready.

- Lines 9 and 10: when +0x245 is clear, skip to line 22.
- Lines 11 to 21: when +0x245 is set, call `0x90D70` on the screen's +0x20 object (not read).
- Lines 22 and 23: compare +0x244 with 0 (at 0x32FFD1); when it is not 0, jump to line 52 and skip the page write.
- Line 25: write 4, Ready, to the page field +0x120 (at 0x32FFE1).
- Lines 24 to 37: switch two child objects (+0x158 and +0x160) on or off with `UnityEngine.GameObject.SetActive`; the second one is on when the page is 4.
- Lines 38 to 48: when +0x245 is set, end with a jump to `0x92F80` (not read).
- Lines 52 to 68, the +0x244 branch: call `0xD86D0` with the instrument (+0x240) and difficulty (+0x241), then end with a jump to a `PlayerStatsBox` method `0x123240` on +0x1D0 (neither read).

Who calls it: `player_screen_green_32E190.asm` line 211, on Confirm row 0. Found by reading that listing, not by a byte scan.
