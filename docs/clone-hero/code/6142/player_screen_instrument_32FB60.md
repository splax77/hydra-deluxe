# player_screen_instrument_32FB60.asm (build 6142, RVA `0x32FB60`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32FB60` (233 lines), session 428cf079 (task CH-UNK-W9b). A private `PlayerSelectionMenu` method with no argument (`dump.cs` line 73010).

Plain words: Green on the Instrument page sets the instrument from the highlighted row's text, not from a number kept for the row. It saves the instrument on the profile and moves to the Difficulty page or the Confirm page.

- Lines 1 to 33: the one-time loads of the 13 instrument names.
- Lines 34 to 46: the row's text is the string at +0x218[cursor + scroll] (`0x1D5C00` returns cursor + scroll). Its hash picks the compare branch.
- Lines 55 to 183: the name sets +0x240: "Lead Guitar" 0, "Bass Guitar" 1, "Rhythm Guitar" 2, "Guitar Coop" 3, "6 Fret Lead Guitar" 4, "6 Fret Bass Guitar" 5, "Drums" 6, "Keys" 7, "Pro Drums" 9, "6 Fret Rhythm Guitar" 10, "6 Fret Guitar Coop" 11, "6 Fret Keys" 14 and "No Part" 15.
- Lines 185 to 191: copy +0x240 into the profile's byte +0x10.
- Lines 192 to 206: set +0x242 to whether the profile's controller type is 2 or 3 (Drums or FiveLaneDrums, IN-57).
- Lines 207 to 216: set +0x243 from the profile's method `0x2123150` (not read).
- Lines 215 to 218: for "No Part", clear +0x1F0 and +0x1F1 together.
- Lines 219 to 228: set the page with `0x330820` to 2, Difficulty, when +0x1F0 is set, else 0, Confirm.

Who calls it: `player_screen_green_32E190.asm` line 135, on the Instrument page. Found by reading that listing, not by a byte scan.
