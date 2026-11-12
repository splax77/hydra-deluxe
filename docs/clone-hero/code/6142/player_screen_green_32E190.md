# player_screen_green_32E190.asm (build 6142, RVA `0x32E190`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32E190 3C0` (229 lines), session 428cf079 (task CH-UNK-W9b). The byte count stops the listing at 0x32E550, where the next method (`dump.cs` line 72907) starts. `PlayerSelectionMenu` override of `BaseMenu` slot 31 (`dump.cs` line 72944), which `menu_buttons_1D4BC0.asm` calls on Green down.

Plain words: Green does a different thing on each page of the player screen. On Confirm it opens the page the highlighted row names, or readies the player. On Difficulty it sets the difficulty the row's text names. On Instrument it hands over to the instrument picker. On Modifiers it toggles one modifier, or goes back to Confirm from the first row. On Ready it does nothing. `0x330820` is the method that sets the page (+0x120); it takes the new page as its first argument.

- Lines 9 to 10: page 4, Ready: return.
- Lines 12 to 14: call the base slot 31 `0x1D5620` first.
- Lines 15 to 23: switch on the page: 0 to line 137, 1 to line 133, 2 to line 53, 3 to line 24; any other page goes to the end (line 212).
- Lines 24 to 52, Modifiers (3): the row is cursor + scroll (lines 24 to 27). Row 0 sets the page to 0, Confirm (lines 49 to 51). Any other row takes entry row - 1 of the list from `0x32CE60` (lines 28 to 42), passes it to the profile's method `0x21236D0` (line 44), redraws with `0x32D400` (line 47) and calls slot 48 (lines 166 to 169).
- Lines 53 to 132, Difficulty (2): the row's text is the string at +0x218[cursor] (lines 66 to 72); this uses the cursor alone, not cursor + scroll. "Expert", "Hard", "Medium" and "Easy" set +0x241 to 3, 2, 1 and 0 (lines 74 to 120), and the value is copied to the profile's byte +0x11 (lines 80 to 87). Then the page becomes 3, Modifiers, when +0x1F1 is set, else 0, Confirm (lines 88 to 95). "Change Instrument" sets +0x1F0 and the page to 1, Instrument (lines 121 to 131).
- Lines 133 to 136, Instrument (1): call `0x32FB60` (`player_screen_instrument_32FB60.md`).
- Lines 137 to 211, Confirm (0): switch on the cursor +0x40 alone. Cursor 3 sets page 3 (lines 151 to 154), cursor 2 page 2 (lines 156 to 159), cursor 1 clears +0x246 and sets page 1 (lines 161 to 165). Cursor 0 readies the player. When +0x245 is clear and `CHNetManager` method `0x145FE0` returns true, it skips straight to line 209 (lines 171 to 181). Otherwise it checks the song (+0x238) for the chosen part with a bit test of `1 << (instrument * 4 + difficulty)` on the song's +0x20 (lines 182 to 197), and writes instrument and difficulty to the profile with `0xD86D0` (line 208), or instrument 15 and difficulty 3 when the bit is clear (lines 205 to 206). Both paths call `0x32FF70` (line 211), which holds a write of 4, Ready, to the page (`0x32FFE1`, read with chdis, not kept; whether that write is conditional was not read).
- Lines 212 to 221: when +0x245 is set, call `0x92910` with 0 (not read).

Who calls it: `menu_buttons_1D4BC0.asm` lines 318 to 321 (virtual slot 31). Found by reading that listing, not by a byte scan.
