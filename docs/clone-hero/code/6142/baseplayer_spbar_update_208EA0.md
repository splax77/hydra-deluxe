Listing: `baseplayer_spbar_update_208EA0.asm`, build 6142, made 2026-11-11 in task CH-UNK-W10b with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 208EA0`). Function: a BasePlayer method, called by the player with the meter amount.

Plain version: the player hands its Star Power meter amount to its SPBar.

Line 2 loads the player's SPBar at +0x90 (the `spBar` field). Lines 3 and 4 jump to the failure routine at line 8 when it is null. Line 5 clears the fourth argument, and line 7 jumps to the SPBar method at 0x14D0D0 (`spbar_set_fill_14D0D0.asm`), which sets the bar's fill.

Who calls it: see the W10b result (`docs/audit/2026-11-08-clone-hero-unknowns/W10b.md`, SP-46) and its kept scan output `W10b/spbar_callers.txt`, a byte scan of il2cpp and .text for direct calls.
