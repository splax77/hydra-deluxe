# Timing segment list builder, RVA 0x20D3F30 (`#8265`, static, not virtual)

Listing: `sp_segments_build_20D3F30.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py` (cut at 0xB00 bytes; the end after line 276 is null-reference stops). Takes the chart (class `dump.cs` line 543384 area) and returns a new list of timing segments (class `#8349`, `dump.cs` line 534464).

- Lines 51-55: read the chart's +0xE0 list (tempo events, class `#8353`, `dump.cs` line 534609; its maker's default is 120) and its +0xE8 list (time signature events, class `#8344`, `dump.cs` line 533997; its maker's defaults are 4 and 4).
- Lines 56-65: start with a time signature of 4 over 4 and a tempo value of 0.5 until the first events are read.
- Lines 66-229: walk both lists in tick order. A tempo event sets the tempo (its +0x18) and the time (its +0x20). A time signature event sets the top number (its +0x18), the bottom number (its +0x1C) and the time (its +0x20). When both fall on one tick, both are taken (lines 205-226).
- Lines 230-257: make a segment for each step: +0x10 the tick, +0x18 the time, +0x30 the tempo, +0x20 its index, +0x24 the top number, +0x28 the bottom number, and add it to the list.
- Lines 258-262: call `0x20D2930` with the scale at +0xb0 of the object passed in, which fills +0x38 and +0x40 (`sp_segment_rates_20D2930.md`).
