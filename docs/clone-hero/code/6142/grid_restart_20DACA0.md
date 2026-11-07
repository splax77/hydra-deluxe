# grid_restart_20DACA0.asm (build 6142, RVA `0x20DACA0`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (226 lines). Starts at the method's first instruction (`#8257.BuildLines`, instance method of the grid class, parameter `#8538` chart). The name "restart" is the record's; the code builds the whole grid of lines in one pass.

Plain words: walk the chart from tick 0 to the song's end, one beat at a time. At each beat, look up the time signature in force. Whenever the numerator or the denominator differs from the previous beat's signature, begin a new measure on this very beat (the counters go back to zero). Each beat becomes a "line" with a kind: 0 is a measure line, 1 a beat, 2 a sub-beat marker. The step to the next beat is `trunc(resolution * 4.0 / denominator)` ticks. Only kind-0 lines are kept in the list the fill generator reads.

- Lines 1 to 19 (not shown above in the walkthrough) and 20 to 23: stack set-up; `r15` is the grid (this), `r13` the chart.
- Lines 24 to 29: `rax = 0x212E130(chart, 1, 0)`, the song end tick (the same function the song-end work calls, `#8538.LastGemEndTick`); stored at `[rsp+0xa8]`. Line 33: a negative end jumps to the final clean-up.
- Line 34: `xmm6 = 4.0` (a `.rdata` constant, no `constants.py` name). Lines 35 to 38: `rbp` (current tick) = 0, `rdi` (previous signature) = null, `r14d` (beat counter in measure) = 0, `r12d` (counter since restart) = 0.
- Lines 48 to 65: `rsi = [chart+0xe8]` is the time signature list; read its span; an empty list jumps to the exit (line 64).
- Lines 66 to 101: pick the signature for tick `rbp`. At tick 0 (lines 102 to 106) take the first entry. Otherwise a binary search (lines 68 to 98) finds the last entry whose tick (`[sig+0x10]`) is at or before `rbp`.
- Lines 107 to 119: `r8b` = "the signature changed": 1 when there is no previous signature, or when the numerator (`[sig+0x18]`) or the denominator (`[sig+0x1c]`) differs from the previous one's.
- Lines 120 to 126: when it changed, `esi` and `edx` become 0; otherwise they keep `r12d` and `r14d`.
- Lines 127 to 136: `r9d = denominator >> 2`; `r14d = edx`, but 0 when `edx` equals the numerator (the beat counter wraps at the numerator); when `r14d` is 0, `r8d` becomes 1 (a measure starts).
- Lines 137 to 149: not a measure start: kind is 1 if the denominator is at most 4 or the beat counter is a multiple of `denominator >> 2`, else 2 (lines 138 to 147).
- Lines 150 to 163: a measure start with numerator 1 and `esi > 0` takes the same kind choice.
- Lines 164 to 177: otherwise a measure start of kind 0; if the previous signature's denominator is above 4, the previous line's stored kind is first set to 2 (the `set_Item` at lines 173 to 176).
- Lines 178 to 184: call `0x20CF2B0(grid, chart, tick rbp, kind r9d, out)` to add the line (`grid_add_line_20CF2B0.md`).
- Lines 185 to 192: `rbp += trunc(chart.resolution * 4.0 / denominator)` (the resolution is `[chart+0xb8]`, line 187).
- Lines 193 to 198: stop when `rbp` exceeds the song end; else bump counters and loop (line 198 jumps back to line 40).
- Lines 199 to 221: finish: store the last index of two of the grid's lists and return.

Who calls it: only `0x20D2EA0` (the chart builder) at 0x20D312D, right before the loop that decides between written and generated fills (`f20D2EA0.asm` lines 118 to 134).
