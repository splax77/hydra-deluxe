# player_menu_confirm_32E190.asm (build 6142, RVA `0x32E190`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32E190 3000` (316 lines). Starts at the method's first instruction: `PlayerSelectionMenu`'s public override void ʵˁʺʻʲʹˁʻʹʺʳ() (`dump.cs` line 72943). It overrides `BaseMenu`'s virtual method of the same name (`dump.cs` line 44958), which it calls first (line 14). `SongOptions` overrides the same method (0x297240). Page 7's IN-60 reads this as the confirm (Green) handler from a decompile; that reading is not checked here.

Plain words: it acts on the row the player confirms, by the page the screen is on. On the Difficulty page it sets the difficulty from the row's text and writes it into the profile. On the Instrument page it calls `0x32FB60`, which does the same for the instrument.

- Lines 9 and 10: on the Ready page (+0x120 = 4) it returns at once.
- Lines 15 to 23: switch on the page: Confirm 0 goes to line 137, Instrument 1 to line 133, Difficulty 2 to line 53, Modifiers 3 to line 24, anything else to the end.
- Lines 66 to 72: the Difficulty page reads the row name `+0x218[+0x40]`, indexed by the cursor (+0x40) alone, not by cursor plus scroll.
- Lines 73 to 119: that name equal to "Expert", "Hard", "Medium" or "Easy" sets +0x241 to 3, 2, 1 or 0 (lines 79, 103, 111 and 119).
- Lines 80 to 87: then copy +0x241 into the profile's difficulty byte (player +0x20, then +0x10, then +0x11) and move on (line 95).
- Lines 121 to 131: the name "Change Instrument" sets +0x1F0 to 1 and calls `0x330820` with 1. What `0x330820` does was not read.
- Lines 133 to 135: the Instrument page calls `0x32FB60` (`player_menu_set_instrument_32FB60.md`).
- Lines 188 to 208 and 287 to 308 read +0x240 and +0x241 and hand them on; not read closely.

Who calls it: not searched; it is a virtual method, so its callers go through the method table.
