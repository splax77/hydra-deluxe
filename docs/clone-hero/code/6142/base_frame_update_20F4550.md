# Engine update to a time, RVA 0x20F4550 (`#8288`, not virtual)

Listing: `base_frame_update_20F4550.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Arguments: `rcx` engine, `xmm1` the new time (double), `r8b` a bool kept in the engine at +0x290. Dump entry: dump.cs line 530128 for `FrameUpdate(double, bool)`, RVA `0x20F4550`.

- Lines 1-13: save registers and first-use setup.
- Line 14: store the bool at +0x290.
- Lines 15-17: call `0x20F4950` with the new time. That method moves the song clock and runs the note and input step (it sets the clock at +0x100 up to three times from sub-step times and then to the frame time; not listed here; SP-30 on page 5 holds what is known of it).
- Lines 18-21: virtual slot 9 (offset 0x1c8, RVA `0x20F5540` in the base class, overridden by drums at `0x20DDEC0`).
- Lines 22-33: if the flag at +0x290 is set, call a debug delegate at +0x288 and virtual slot 21 (offset 0x288).
- Lines 34-37: virtual slot 15 (offset 0x228): the Star Power end check `0x20F63B0`.
- Lines 38-41: virtual slot 18 (offset 0x258).
- Lines 42-45: virtual slot 12 (offset 0x1f8): the Star Power update and drain `0x20F6800`.
- Lines 46-52: if the flag was set, bump a counter on the object at +0x180 and clear its length.
- Lines 53-57: restore registers and return.
- Lines 58-60: null-reference stop.

So within one frame: input and notes, then the end check, then the drain.
