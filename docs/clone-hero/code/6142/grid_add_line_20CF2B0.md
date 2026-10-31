# grid_add_line_20CF2B0.asm (build 6142, RVA 0x20CF2B0)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (183 lines). Support listing for the grid builder `0x20DACA0`, which calls it once per line (`grid_restart_20DACA0.asm` line 184). Method `#8265.ʷʿʽʽ...` family; its static arguments are `(grid, chart, tick, kind, out int)`. I read only the parts that decide which list gets what; the tempo walk in lines 36 to 82 was not traced in detail.

Plain words: add one line to the grid. Every line adds its time in seconds to the grid's time list (`grid+0x10`) and its kind to the kind list (`grid+0x18`). Only a line of kind 0 (a measure line) also adds its tick to the third list (`grid+0x20`, a `List<long>`), which is the list the fill generator walks.

- Lines 36 to 82: find the tempo entry in force at `tick` (the list at `[chart+0xe0]`), and convert the tick to seconds with `(tick - tempoTick) * 60.0 / (tempoBpm * resolution) + tempoTime` (constant 60.0 in the listing's comment, resolution at `[chart+0xb0]`).
- Lines 83 to 94 (second block): append the seconds to `[grid+0x10]`.
- Lines 95 to 106: append the kind (`r14d`) to `[grid+0x18]`.
- Lines 107 to 108: `test r14d, r14d; jne` skips the next block when the kind is not 0.
- Lines 109 to 119: for kind 0 only, append the tick (`rdx = rsi`) to `[grid+0x20]`.
