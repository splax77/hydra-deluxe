# Drums input update, RVA 0x100DE20 (`#8253` for the drums input type, slot 21)

Listing: `drums_input_update_100DE20.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` lines 527371 to 527383 declare slot 21 on `#8253` and give this RVA for its drums instance. The frame update `0x20F4550` calls slot 21 at line 33 of `base_frame_update_20F4550.asm`, when the bool at +0x290 is set, before the Star Power end check (slot 15, line 37).

- Lines 5-13: keep last frame's input state (+0x2a4, +0x2ac) in +0x298 and +0x2a0, then clear +0x2a4 and +0x2ac.
- Lines 14-16: the input source at +0x2b0; a missing one goes to the null-reference stop at line 78.
- Line 20: load the song clock at +0x100 into `xmm6`.
- Lines 21-49: find the input source's interface method and call it with the clock in `xmm1` and +0x2a4 as the place to write (lines 46 to 49).
- Lines 50-53: virtual slot 25 (offset 0x2c8).
- Lines 56-62: if slot 25 returned true, call `0x100E6E0`.
- Lines 63-70: jump to virtual slot 26 (offset 0x2d8), which the drum engine overrides with the input poll `0x20DE070` (`dump.cs` line 527511). The poll reaches the press handler `0x20DEAA0` by the jump at 0x20DE236.
- Lines 71-78: interface slot lookup and the null-reference stop.
