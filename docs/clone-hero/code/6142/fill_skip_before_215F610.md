# fill_skip_before_215F610.asm (build 6142, RVA `0x215F610`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (80 lines). Starts at the method's first instruction: a scrambled `public void` method with no parameters of the track class #8661 (`dump.cs` lines 551989 and 551990).

Plain words: the mirror of `fill_skip_from_coda_215DA70.asm`. If the chart has a coda event, every written fill that starts before the coda's tick is marked skipped (line 54 is `setl`), and every fill at or after it is not.

Callers: a byte scan for direct calls and jumps on 2026-11-08 found none. Delegates, vtable slots and reflection were not searched, so whether it ever runs is not known.
