# hit_window_formula_20DDDA0 (build 6142, made 2026-10-30 with tools/ch_mcp/chdis.py)

Function: `#8255.HitWindowFromGap(double)`, an override in vtable slot 19 of the sealed drums engine class `#8255` (`dump.cs` line 527484, "Slot: 19"), RVA `0x20DDDA0`; the first instruction is the function start. Decompiled at `hydra-data\scratch_ms\ghidra_decompile_all.txt` lines 757 to 782. The argument is a gap in seconds (the sum of two clamped half-gaps from `half_gap_clamp_20F7210`); the result is a whole-window width in seconds. `this+0x198 & 0x1000` is the PrecisionMode bit.

Lines 1 to 10: prologue and the one-time class-init setup for `System.Math`.

Line 11: the PrecisionMode bit is tested. Line 12: x = gap * 1000 (the constant at 0x3064C28 is 1000.0), so x is the gap in milliseconds. Line 16: `ja` jumps to the precision branch (line 36) when the bit is set.

Lines 17 to 35, the normal branch (bit clear): `Math.Pow(x, 2.0)` (line 20 loads 2.0, call at line 25). Result = ((x * 0.0110924369747899 - Pow(x,2) * 2.628539e-05) * 170 - 20) / 1000 seconds (lines 22 to 30). There is no clamp. At x = 170 it gives 171.4313 ms; at x = 75 it gives 96.2932 ms (computed from the constants printed in this listing).

Lines 36 to 54, the precision branch (bit set): Result = (13 + 12.4805915 * (0.04 * x + 0.0004 * Pow(x,2))) / 1000 seconds (lines 41 to 52: `x * -0.04`, `Pow(x,2) * 0.0004`, subtract, times 12.4805915, subtract from 13, divide by 1000). No clamp here either. With the precision-mode half-gaps held to 25 to 40 ms (see `drums_engine_ctor_20DF680`), x runs from 50 to 80 ms and the result from 50.44 to 84.89 ms (arithmetic on the listed constants, never run live).
