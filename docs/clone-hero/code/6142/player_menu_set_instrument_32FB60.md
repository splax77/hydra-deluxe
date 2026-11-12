# player_menu_set_instrument_32FB60.asm (build 6142, RVA `0x32FB60`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32FB60 3000` (233 lines). Starts at the method's first instruction: `PlayerSelectionMenu`'s `private void ʹʻʽʷˀʿˀʿʸˁʼ()` (`dump.cs` line 73009).

Plain words: it sets the screen's instrument (+0x240) from the text of the highlighted row, then writes it into the profile.

- Lines 1 to 33: set-up and the one-time loads of 13 instrument names ("6 Fret Lead Guitar", "Keys", "Pro Drums", "Drums", "No Part" and others).
- Lines 34 to 46: the row name is `+0x218[0x1D5C00(this)]`. `0x1D5C00` is `BaseMenu`'s cursor (+0x40) plus scroll (+0x3C), read with `py -m tools.ch_mcp.chdis 1D5C00 80` (three instructions, not kept).
- Lines 47 to 183: a string switch (hash, then `String.op_Equality`) that stores one instrument number in +0x240 for each name it knows; for example "6 Fret Lead Guitar" stores 4 (line 61), "6 Fret Guitar Coop" stores 0xB (line 71) and "No Part" stores 0xF (line 111). A name it does not know leaves +0x240 as it was.
- Lines 184 to 191: copy +0x240 into the profile's instrument byte (player +0x20, then +0x10, then +0x10).
- Lines 192 to 206: +0x242 is set true when the profile's controller type is Drums 2 or FiveLaneDrums 3.
- Lines 207 to 216: +0x243 is set from the profile getter `0x2123150`.
- Lines 215 to 218: when the instrument is 0xF, the word at +0x1F0 is cleared.
- Lines 219 to 228: tail-call `0x330820` with 2 when the byte at +0x1F0 is set and 0 when it is not. That `0x330820` takes a page number was not read.

Who calls it, direct callers only (byte scan, `docs/audit/2026-11-08-clone-hero-unknowns/W9a/callers_240_241_writers.txt`): only the confirm method `0x32E190` at 0x32E3D1 (`player_menu_confirm_32E190.asm` line 135).
