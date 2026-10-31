# chart_length_212E130 (build 6142, made 2026-10-30 with tools/ch_mcp/chdis.py)

Function: `#8538.LastGemEndTick(bool addMargin = True)`, the chart class's "last gem end" method (`dump.cs` line 543562), RVA `0x212E130`; the first instruction is the function start. The decompile (see `code/README.md`; not kept) has it at lines 15776 to 15838 (named `FUN_18212e130`). Fields on the chart: +0xD0 the `List<Track>` (`#8661`), +0x120 a `Nullable<long>` cache (flag byte at +0x120, value at +0x128), +0xB0 a `double` margin in ticks. The listing's second argument (dl) is the `addMargin` bool.

Lines 1 to 26: prologue and one-time generic setup.

Lines 27 to 32: if the cache flag byte at chart+0x120 is already set, jump to line 85 and use the cached value.

Lines 33 to 35: otherwise the track list at chart+0xD0 must exist; null throws (line 98).

Lines 36 to 47: a `List<Track>` enumerator is built (call at line 40); the running maximum is kept in rdi, starting at 0.

Lines 49 to 63: loop. `MoveNext` (line 51); a null track is skipped (line 55 to 56); for each track `0x215FED0` (the track's last gem end, see `docs/clone-hero/code` row for it, decompile lines 50458 to 50507) is called (line 58); if the result is greater than the maximum it replaces it (lines 59 to 62). Every instrument track in the list takes part, so every difficulty of every instrument counts, vocals excluded because `0x215FED0` reads only the gem list at track+0x60 (the vocal list is at track+0x68).

Lines 64 to 76: enumerator disposed; the maximum is read back.

Lines 77 to 84: the maximum is stored in the `Nullable<long>` at chart+0x120 (the cache), so later calls skip the loop.

Lines 85 to 87: the cached value is read.

Lines 88 to 91: when `addMargin` is true, the double at chart+0xB0 is truncated to an integer (`cvttsd2si`) and added to the result. Chart+0xB0 is written by `0x212C0B0` (decompile lines 14459 to 14471) as a value times the float at (chart+0xC8)+0x14, and its caller at decompile line 42845 passes the ushort at (header+0xF0)+0x12. Earlier scouts read those as the chart resolution (ticks per beat) and the song speed; this scout did not trace what those two fields are, so the identity is unchecked.

Lines 92 to 97: epilogue. Lines 98 to 102: throws.
