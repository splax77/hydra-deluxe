# tick_to_time_212B9C0.asm (build 6142, RVA `0x212B9C0`)

Made 2026-11-23 with `tools/ch_mcp/chdis.py` (152 lines). Starts at the method's first instruction. `dump.cs` names it as an instance method of the chart class #8538 taking a `long` and returning a `double`.

Plain words: turn a chart tick into a song time in seconds, through the tempo map, using the chart's working resolution at `chart+0xB0`.

- Lines 20 to 47: find the tempo entry in force at the tick, in the tempo list at `chart+0xE0`; keep its tempo `+0x18` and its time `+0x20`.
- Lines 48 to 72: time = entry time + (tick - entry tick) x 60 / (tempo x `chart+0xB0`). The 60.0 is a `.rdata` constant (line 58).
- Lines 74 to 131: a round-trip check. The time is turned back into a tick with the same resolution (lines 77 to 88); while that tick is not the one asked for, the double is stepped by one unit in the last place (lines 104 to 131) and the check runs again.
- Lines 132 onward: return the time.

`chart+0xB0` is the file's resolution times the song speed ratio (`song_resolution_set_212C0B0.md`), so the time this returns is the file's time divided by the speed.
