# track_dedupe_5DB0C0.asm (build 6142, RVA `0x5DB0C0`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (170 lines). Starts at the method's first instruction. `dump.cs` line 552085 names it as a generic instance method of the track class #8661 taking one `List<T>`; line 552088 gives this RVA for the shared `<object>` copy.

Plain words: among items that start on the same tick and give the same key, keep the longest and remove the rest. The key is an `int` from the one method of interface #8331 (`dump.cs` lines 533373 to 533382), called through `0x2C44210`.

- Lines 28 to 45: for each index i, read item i, its key (line 41) and its start tick `+0x10` (line 42).
- Lines 46 to 76: scan forward while the next item's tick is not above this tick (line 57). For each later item with the same tick and key, keep the index of the longest so far (`+0x18`, `cmovg` at line 74). An equal length keeps the earlier one.
- Lines 77 to 105: walk the same-tick run again and set every same-key item other than the kept one to null (`set_Item`, line 103).
- Lines 125 to 149: remove every null from the list.
- Lines 109 to 124 and 150 to 157: when the list is the chart-note list type (#8569), the first removed index goes to the track method `0x215EA80`.

The forward scan stops at the first later tick, so it assumes the list is sorted by start tick. Its only direct callers (byte scan) are five sites in the track finish `0x215C320` (`docs/audit/note-shuffle/step0/open2/f215c320.asm` lines 63, 70, 78, 82 and 86).

For the Star Power phrase class #8252 every implementing method returns a constant (`0x84220` returns 0 and `0x102040` returns 1; `dump.cs` lines 527234 to 527271), and no class derives from #8252, so every phrase gives the same key.
