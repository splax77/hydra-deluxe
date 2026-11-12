# engine_set_clock_20F4310.asm (build 6142, RVA `0x20F4310`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (36 lines). Starts at the method's first instruction. `dump.cs` line 530161 names it as a private instance method of the base engine class #8288 taking one `double`.

Plain words: set the engine's clock pair at `+0x100` to (the given time, the chart tick of that time).

- Lines 6 to 11: the chart at `engine+0x230` turns the time into a tick through `0x212BCC0` (`time_to_tick_212BCC0.asm`).
- Lines 12 to 22: build the 16-byte struct #8496 (`dump.cs` lines 540408 to 540417: a `double` at `+0x0` and a `long` at `+0x8`) from (time, tick) with its constructor at line 19, and store it at `engine+0x100` (line 22). So the `long` lands at `engine+0x108`.
- Lines 21 and 23 to 28: pass the tick and the time on to a method of the object at `engine+0x1D8`.

`engine_clock_update_20F4950.asm` writes the same pair the same way.
