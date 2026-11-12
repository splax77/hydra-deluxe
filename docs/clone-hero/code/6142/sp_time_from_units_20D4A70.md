# Star Power units to time, RVA 0x20D4A70 (`#8265`, static, not virtual)

Listing: `sp_time_from_units_20D4A70.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. The inverse of `sp_units_from_time_20D1D20.asm`: takes units in `xmm0`, a timing segment (class `#8349`) in `rdx` and the same scale in `xmm2`.

- Lines 2-3: a missing segment goes to the null-reference stop at line 10.
- Lines 4-7: units times 60.0 times the segment's +0x40, divided by scale times the segment's +0x30 (its tempo).

The end-time function `0x20F37E0` calls it at lines 74 and 82 of `sp_end_time_20F37E0.asm` to turn the meter back into time.
