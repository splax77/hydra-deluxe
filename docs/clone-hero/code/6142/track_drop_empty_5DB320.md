# track_drop_empty_5DB320.asm (build 6142, RVA `0x5DB320`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (103 lines). Starts at the method's first instruction. `dump.cs` line 552119 names it as a generic instance method of the track class #8661 with two type parameters, `TEvent` and `TEventWithin`, taking `List<TEvent>` and `List<TEventWithin>`; line 552122 gives this RVA for the shared `<object, object>` copy.

Plain words: walk the first list from its last item to its first, and remove every item that has no item of the second list starting inside it.

- Lines 19 to 26: take the second list as a span (`AsSpan`, line 26).
- Lines 29 to 32: start at the last index of the first list (`count - 1`); an empty list exits.
- Lines 37 to 41: fetch the item; `rdi` = its start tick at `+0x10`.
- Lines 42 to 53: call `0x2109730`, the event base class's "start plus length" (`+0x10 + +0x18`, base class #8290 at `dump.cs` lines 530349 to 530360). If the end equals the start (length 0), the search end is the start (line 50); otherwise it is end minus 1 (line 53).
- Lines 63 to 67: call the tick-range search `0x5FAD90` (`tick_range_search_5FAD90.asm`) on the second list with (start, search end).
- Lines 68 to 77: when both halves of the result are -1 (no item in range), `RemoveAt` the item (line 77) and retake the span (lines 78 to 89).
- Lines 90 and 91: step to the previous index until index 0 is done.

Its only direct callers (byte scan) are five sites in the track finish `0x215C320` (`docs/audit/note-shuffle/step0/open2/f215c320.asm` lines 101, 106, 113, 116 and 123).
