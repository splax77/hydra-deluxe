# track_trim_overlaps_5DB4C0.asm (build 6142, RVA `0x5DB4C0`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (183 lines). Starts at the method's first instruction. `dump.cs` line 551982 names it as a generic instance method of the track class #8661 taking one `List<T>`; line 551985 gives this RVA for the shared `<object>` copy.

Plain words: walk the list in order and stop items with the same key from overlapping. When an item starts before the furthest end seen so far for its key, the earlier item is cut to end where this one starts, and this one is stretched to that furthest end.

- Lines 34 to 42: a 512-byte table on the stack, zeroed: 32 entries of (index, end tick), one per key.
- Lines 47 to 59: for each item, read its length `+0x18` and start `+0x10`.
- Lines 61 to 80 and 130 to 136: get the key through interface #8331 (`dump.cs` lines 533373 to 533382). A key of 32 or more throws (line 83).
- Lines 84 to 90: this item's end is start plus length; compare its start with the table's end for the key.
- Lines 91 to 101: on overlap, the item the table names gets length = this start minus its start.
- Lines 102 to 128: this item's length becomes the larger of the two ends (`Max`, line 109) minus its start, and the table entry becomes (this index, that end).
- Lines 137 to 148: with no overlap, the table entry becomes (this index, this end) when this end is the larger.
- Lines 154 to 166: when the list is the chart-note list type (#8569), the track method `0x215BE50` is called afterwards.

It reads the list in its stored order and does not sort it. Its only direct callers (byte scan) are three sites in the track finish `0x215C320` (`docs/audit/note-shuffle/step0/open2/f215c320.asm` lines 74, 90 and 94).
