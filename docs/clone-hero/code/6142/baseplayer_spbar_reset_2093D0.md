Listing: `baseplayer_spbar_reset_2093D0.asm`, build 6142, made 2026-11-11 in task CH-UNK-W10b with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 2093D0`). Function: a BasePlayer method that resets the player's on-screen Star Power and combo displays.

Plain version: it sets the SPBar back to empty, along with the combo colour and the Star Power neck.

Lines 4 to 8 call a ComboColor method on the object at +0x38 with 0. Lines 9 to 15 make a virtual call on the object at +0x28 with 0. Lines 18 to 20 skip the rest when the player is not active and enabled. Otherwise lines 21 to 27 call the SPBar method at 0x14D0D0 (`spbar_set_fill_14D0D0.asm`) on the SPBar at +0x90 with an amount of 0.0 and false. Lines 28 to 34 then jump to a SpNeckRenderer method on the object at +0xD8 with 0. Each null object jumps to the failure routine at line 38.

Who calls it: see the W10b result (`docs/audit/2026-11-08-clone-hero-unknowns/W10b.md`, SP-46) and its kept scan output `W10b/spbar_callers.txt`.
