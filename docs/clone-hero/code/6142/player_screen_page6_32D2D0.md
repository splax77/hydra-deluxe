# player_screen_page6_32D2D0.asm (build 6142, RVA `0x32D2D0`)

Made 2026-11-11 with `py -m tools.ch_mcp.chdis 32D2D0` (33 lines), session 428cf079 (task CH-UNK-W9b). A public `PlayerSelectionMenu` method with no argument (`dump.cs` line 73028).

Plain words: this is one place that puts the page field at a number the page enum does not name. It sets the page to 6, saves the instrument and difficulty to the profile, and shows or hides two objects by whether the page is 5 or 7.

- Line 5: write 6 to the page (+0x120).
- Lines 6 to 14: write +0x240 and +0x241 to the profile with `0xD86D0`.
- Lines 15 to 21: `buttonsGroup` (+0x158) is shown when the page is 5.
- Lines 22 to 28: `ReadyHeader` (+0x160) is shown when the page is 7.

Page values outside the enum, found by listing every method RVA in `dump.cs` lines 72905 to 73100 with chdis (the class ends at line 73053) and searching for writes and compares of +0x120 and for the arguments passed to the two page setters; the script is `docs/audit/2026-11-08-clone-hero-unknowns/W9b/psm_page_values.sh`. 6 is written here; 5 and 6 are passed to the second page setter `0x330350` (from `0x32CD20`, `0x32D190` and `0x32E550`); 7 is only compared (here and at 0x3301A3, inside `0x330170`). No write of 7 was found among those methods. A write from outside the class was not searched for.

Who calls it: not searched.
