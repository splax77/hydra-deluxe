# time_to_tick_212BCC0.asm (build 6142, RVA `0x212BCC0`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (101 lines). Starts at the method's first instruction. `dump.cs` line 543506 names it as an instance method of the chart class #8538 taking a `double` and returning a `long`.

Plain words: turn a song time in seconds into a chart tick, rounded toward zero.

- Lines 14 to 16 and 27 to 29: a negative time counts as 0.
- Lines 30 to 72: walk the chart's tempo list at `chart+0xE0` and keep the last entry whose time (`+0x20`) is below the given time (the walk stops at the first entry at or past it, line 64).
- Lines 74 to 79: that entry's tick `+0x10`, time `+0x20` and tempo `+0x18`, and the chart's resolution at `chart+0xB0`.
- Lines 83 to 92: tick = entry tick + truncate((time - entry time) * resolution * tempo / 60). The 60.0 is a constant in `.rdata` (line 89).
