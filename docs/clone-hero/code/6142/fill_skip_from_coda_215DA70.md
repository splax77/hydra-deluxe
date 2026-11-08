# fill_skip_from_coda_215DA70.asm (build 6142, RVA `0x215DA70`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (80 lines). Starts at the method's first instruction: a scrambled `public void` method with no parameters of the track class #8661 (`dump.cs` lines 551937 and 551938).

Plain words: if the chart has a coda event, mark every written fill on this track that starts at or after the coda's tick as skipped, and every fill before it as not skipped. With no coda event it changes nothing.

- Lines 20 to 24: `rax = [track+0x50]` (the chart, class #8538); if the chart's `+0x100` is null the method returns (line 24 jumps to line 67).
- Lines 25 to 30: enumerate the track's fill list at `+0x88` (List of #8292).
- Lines 46 to 55: for each fill, `[fill+0x20] = (fill start [fill+0x10] >= [[chart+0x100]+0x10])`. Line 54 is `setge`, so a fill that starts exactly on the coda tick is skipped.

What `chart+0x100` holds: `dump.cs` line 543389 types it as class #8500, a text event (a string at `+0x18`, built from a string and a tick by the constructor `0x211BA30`, `dump.cs` lines 540573 to 540581). The `.mid` events reader writes it: `mid_events_coda_21544E0.asm` lines 245 and 272 load the strings `[coda]` and `coda`, line 300 tests `chart+0x100` for null, line 310 builds the event named `coda` at the event's tick, and line 312 stores it, so only the first coda event is kept. The `.chart` reader `0x2151650` has the same null test and store at 0x21523A7 to 0x21523E0 (seen in a scratch listing, not committed).

Caller (byte scan, 2026-11-08): one, the track finish `0x215C320`, at its first call (`docs/audit/note-shuffle/step0/open2/f215c320.asm` line 37), before any of the list clean-ups. Delegates, vtable slots and reflection were not searched.

The mirror method `0x215F610` (`fill_skip_before_215F610.asm`) is the same except line 54 is `setl`: it skips fills that start before the coda. The byte scan found no direct caller of it.
