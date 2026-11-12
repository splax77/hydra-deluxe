# Time to Star Power units, RVA 0x20D1D20 (`#8265`, static, not virtual)

Listing: `sp_units_from_time_20D1D20.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Takes a time offset in `xmm0`, a timing segment (class `#8349`, `dump.cs` line 534464) in `rdx` and a scale in `xmm2` (the callers pass the +0xb0 field of the object at engine +0x230). Returns the offset in the segment's units.

- Lines 2-3: a missing segment goes to the null-reference stop at line 11.
- Lines 4-5: the segment's +0x40 (its units-per-beat factor, written by `0x20D2930`) times 60.0.
- Lines 6-8: offset times scale times the segment's +0x30 (its tempo, copied from the tempo event by `0x20D3F30`), divided by the value from line 5.

So the result is offset x scale x tempo / (60 x factor). `sp_time_from_units_20D4A70.asm` is the inverse.
