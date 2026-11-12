# tick_range_search_5FAD90.asm (build 6142, RVA `0x5FAD90`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (140 lines). Starts at the method's first instruction. `dump.cs` lines 533562 to 533566 name it as a static generic method taking a read-only span and two `long`s, returning a pair of `int`s; this RVA is the shared `<object>` copy.

Plain words: binary-search a tick-sorted span for the first and last items whose start tick (`+0x10`) lies in [low, high]. Return (-1, -1) when none does.

- Lines 18 to 21: low greater than high, or an empty span, returns (-1, -1) (lines 123 to 135).
- Lines 32 to 54: a lower-bound search for the first item with tick at or above low.
- Lines 55 to 64: if that item's tick is still below low, return (-1, -1) (line 118).
- Lines 70 to 92: an upper-bound search for the last item with tick at or below high.
- Lines 93 to 102: if that item's tick is above high, return (-1, -1).
- Lines 103 to 117: return (first, last).

The search assumes the span is sorted by start tick; it does not sort it.
