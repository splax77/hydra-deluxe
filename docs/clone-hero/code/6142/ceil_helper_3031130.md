Listing: `ceil_helper_3031130.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: the rounding helper at RVA 0x3031130 that the star counter calls (`star_count_210DD00.asm` line 47). It has no name in `script.json`. `System.Math.Ceiling` (RVA `0x1B518E0`, `dump.cs` line 90193) is a single jump to it (`math_ceiling_1B518E0.asm` line 1), so the star counter's call is `Math.Ceiling` with the jump skipped.

Lines 1 and 2: when a stored CPU-feature value is 2 or more, jump to 0x304BA10 (`ceil_sse41_304BA10.md`).
Line 3: otherwise jump to the software routine 0x3031150 (`ceil_soft_3031150.md`).

Both routes round up.
