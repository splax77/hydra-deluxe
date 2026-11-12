# track_slice_2152B20.asm (build 6142, RVA 0x2152B20)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (106 lines). Starts at the first instruction; `dump.cs` has no method at this address. Its caller is the `.mid` drum reader at 0x2155836 (`mid_drum_reader_2155050.asm` line 453), which passes the chart (`rcx`), the instrument (`dl`) and a span to fill (`r8`).

Plain words: return the chart's tracks for one instrument, all difficulties, as a span, and say whether any of them exists.

- Lines 27 and 29: the chart's track list at `+0xD0` and the start index, instrument times a static count at `[static+0x60]`.
- Lines 39 to 69: slice the list from that index for that count and store the span in the caller's buffer.
- Lines 70 to 94: return true at the first non-null track in the slice; false (lines 95 to 103) when all are null.

The index rule is the same as the track lookup `track_lookup_20CFD40.asm` lines 31 to 44 (instrument times the static, plus the difficulty), so the count is the number of difficulties. The difficulty enum `dump.cs` lines 550049 to 550058 has Easy 0 to Expert 3 and `Count = 4`.
