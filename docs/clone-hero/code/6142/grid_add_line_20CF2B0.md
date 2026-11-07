# grid_add_line_20CF2B0.asm (build 6142, RVA 0x20CF2B0)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (183 lines). Support listing for the grid builder `0x20DACA0`, which calls it once per line (`grid_restart_20DACA0.asm` line 184). The record put it in the `#8265` family, but its address is in neither `dump.cs` nor `script.json`, so it has no name to map; its static arguments are `(grid, chart, tick, kind, out int)`. I read only the parts that decide which list gets what; the tempo walk in lines 34 to 79 was not traced in detail.

Plain words: add one line to the grid. Every line adds its time in seconds to the grid's time list (`grid+0x10`) and its kind to the kind list (`grid+0x18`). Only a line of kind 0 (a measure line) also adds its tick to the third list (`grid+0x20`, a `List<long>`), which is the list the fill generator walks, and its seconds to a fourth list (`grid+0x28`, a `List<double>`).

- Lines 34 to 79, and line 88: find the tempo entry in force at `tick` (the list at `[chart+0xe0]`), and convert the tick to seconds with `(tick - tempoTick) * 60.0 / (tempoBpm * resolution) + tempoTime` (constant 60.0 in the listing's comment, resolution at `[chart+0xb0]`).
- Lines 70 and 82 to 101: append the seconds to `[grid+0x10]`.
- Lines 102 to 123: append the kind (`r14d`) to `[grid+0x18]`.
- Lines 124 to 125: `test r14d, r14d; jne` jumps to the epilogue at line 164 when the kind is not 0, past the next two blocks.
- Lines 126 to 147: for kind 0 only, append the tick (`rdx = rsi`) to `[grid+0x20]`.
- Lines 148 to 163 and 173 to 178: for kind 0 only, append the seconds to `[grid+0x28]`.
