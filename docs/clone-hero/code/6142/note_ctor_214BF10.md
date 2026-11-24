# note_ctor_214BF10.asm (build 6142, RVA `0x214BF10`)

Made 2026-11-23 with `tools/ch_mcp/chdis.py` (33 lines). The constructor of the engine note, class #8597, that takes two doubles, a ushort, a flags value and two longs (`dump.cs`). Its only direct caller is `note_build_20D4BE0`.

- Lines 8 and 10: keep the second double (xmm2) in xmm7 and the first (xmm1) in xmm6.
- Line 15: the second double goes to `+0x30`.
- Lines 19 and 24: the two longs go to `+0x40` and `+0x48`.
- Line 27: the first double goes to `+0x28`, the note time.

The first double's parameter has the same scrambled name as the time parameter of the chart's time-to-tick method `0x212BCC0` (`dump.cs`), which fits it being a time.
