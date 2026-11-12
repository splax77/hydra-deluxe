# Timing segment rates, RVA 0x20D2930 (`#8265`, static, not virtual)

Listing: `sp_segment_rates_20D2930.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Takes the list of timing segments (class `#8349`) in `rcx` and the scale in `xmm1`. Fills two fields of every segment. Its one direct caller found is `0x20D3F30` (`sp_segments_build_20D3F30.asm` line 262).

- Lines 45-48: load the constants 4.0, 8.0, 30.0 and 60.0.
- Lines 57-68: the segment's +0x40 = (4.0 / +0x28) x +0x24 x 8.0 / 30.0. The fields +0x24 and +0x28 are the time signature's top and bottom numbers (see `sp_segments_build_20D3F30.md`), so 4 / bottom x top is the measure's length in quarter notes, and +0x40 is that length times 8 / 30.
- Lines 69-79: the segment's +0x38 = a running sum. For each segment after the first, add (this segment's +0x18 minus the previous one's +0x18) x scale x previous +0x30 / (previous +0x40 x 60), the same formula as `0x20D1D20`. The first segment gets 0.
- Lines 80-82: keep this segment as the previous one and go on.

So +0x38 is where each segment starts in the units `0x20D1D20` returns, counted without a break across tempo and time signature changes.
