# half_gap_clamp_20F7210 (build 6142, made 2026-10-30 with tools/ch_mcp/chdis.py)

Function: `#8288.ClampedHalfGap(noteA, noteB, double min, double max)`, protected method of the base engine class `#8288` (`dump.cs` line 530311), RVA `0x20F7210`; the first instruction is the function start. The decompile (see `code/README.md`) has it at lines 53871 to 53907. It returns half the time gap from noteA to noteB, held between `min` and `max` (seconds). Note time is the `double` at note+0x28. The fifth call argument (the max) is on the stack at [rsp+0x80] inside the function (line 40).

Lines 1 to 12: prologue; `min` (xmm3) is kept in xmm8; one-time `System.Math` class init.

Lines 13 to 16: if noteA is null or noteB is null, jump to line 72 (the neighbour is missing).

Lines 17 to 22: gap = noteB.time minus noteA.time; if the gap is exactly 0, jump to line 72 as well.

Lines 23 to 33: half = (noteB.time - noteA.time) * 0.5 (constant 0x3063FB0 is 0.5), kept in xmm6.

Lines 40 to 50: `max` is loaded into xmm7. If `min` is greater than `max`, Math.ThrowMinMaxException is called (this is the Math.Clamp argument check). Not reachable with the drums engine's constants.

Lines 51 to 52: if `min` is greater than the half-gap, the result is `min` (line 63 puts it in xmm6).

Lines 53 to 54: otherwise if the half-gap is greater than `max`, the result is `max` (lines 55 to 62 return it).

Lines 64 to 71: otherwise the half-gap itself is returned.

Lines 72 to 77: the missing-neighbour and zero-gap exit returns `max` unchanged (the value at [rsp+0x80]). So a note with no neighbour on a side, or a neighbour at the same time, gets the full back constant for that side.

Called twice per note by `note_window_20F3A90`, once per side, with min = `this+0x38` (front) and max = `this+0x30` (back).
