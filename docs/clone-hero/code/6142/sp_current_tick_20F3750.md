# Current tick, RVA 0x20F3750 (`#8288`, not virtual)

Listing: `sp_current_tick_20F3750.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Returns the song clock as a tick count in `rax`.

- Lines 1-4: save, clear a register, keep the engine in `rbx`.
- Lines 5-10: if the song clock (+0x100) is below zero, return 0.
- Lines 11-16: load the current tempo segment (+0x148) and the object at +0x230. A missing one goes to the null-reference stop at line 31.
- Lines 17-22: clock minus the segment's start time (+0x18), then the conversion `0x20D1D20` with the resolution at +0xb0.
- Lines 23-30: add the segment's start tick (+0x38), turn it into an integer and return.
- Lines 31-33: null-reference stop.
