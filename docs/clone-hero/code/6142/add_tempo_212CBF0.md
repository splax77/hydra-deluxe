# add_tempo_212CBF0.asm (build 6142, RVA `0x212CBF0`)

Made 2026-11-23 with `tools/ch_mcp/chdis.py` (158 lines). Starts at the method's first instruction. `dump.cs` names it as an instance method of the chart class #8538 taking a `long` tick (default 0) and a `double` tempo (default 120).

Plain words: append a tempo to the chart's tempo list and store the song time it starts at, worked out from the tempo before it with the chart's working resolution at `chart+0xB0`.

- Lines 37 to 58: take the last tempo entry already in the list: its tick `+0x10`, tempo `+0x18` and time `+0x20`.
- Lines 60 to 73: time = last time + (tick - last tick) x 60 / (last tempo x `chart+0xB0`).
- Lines 81 to 106: add the new entry to the list.
- Lines 115 to 119: the new entry's tempo goes to `+0x18`, its time to `+0x20` and its tick to `+0x10`.
- Lines 127 onward: the error path, a formatted exception.

So every tempo's stored time is built with the same speed-scaled resolution that `tick_to_time_212B9C0` reads. Not checked: whether any reader calls the resolution setter after it has added tempos, which would leave earlier entries on the old resolution.
