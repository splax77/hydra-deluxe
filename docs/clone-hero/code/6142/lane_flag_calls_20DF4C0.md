# Drum engine lane flag calls, RVA 0x20DF4C0 (`#8255`)

Listing: `lane_flag_calls_20DF4C0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` line 527550 gives this RVA, a private method of the drum engine with no arguments. SP-43's scan (`docs/audit/2026-11-08-clone-hero-unknowns/sp43_immscan.py`) flagged its `mov r9d, 3` at line 53.

- Lines 9-10: do nothing unless the byte at +0x290 is set.
- Lines 11-91: seven blocks, one per bit group of the word at +0x2f4. Each, when its bits are set and the object at +0x318 exists, calls the runtime helper 0x2EFBCF0 (not in `dump.cs`) with one of the bytes +0x309 to +0x310 and a fixed number in `r9d`. The seven numbers are 0x2c, 8, 8, 3, 0x3b, 0xffc2 and 0xff82. The 3 at line 53 is one of these per-lane arguments, beside values like 0x3b and 0xffc2 that are no time, so it is not a 3 ms compare. What the helper does with them was not read.
