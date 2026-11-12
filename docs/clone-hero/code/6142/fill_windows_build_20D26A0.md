# Flagged-fill window list builder, RVA 0x20D26A0 (`#8265`, static, not virtual)

Listing: `fill_windows_build_20D26A0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py` (cut at 0x290 bytes; it ends in null-reference stops). `dump.cs` line 528373 declares it as a static method of `#8265` that takes the chart (`rcx`, kept in `r15`) and a track (`rdx`, kept in `rbx`) and returns a `List` of `#8323` (`dump.cs` line 532864). The engine's +0x1F8 is that list type: it is the field at +0x18 of the struct `#8263` (`dump.cs` line 528231) that the base engine holds at +0x1E0 (`dump.cs` line 530043).

- Lines 36-42: make the new list.
- Lines 43-58: read the track's +0x88 list, a `List` of `#8292` (`dump.cs` line 551863). `#8292` is the fill class: its maker `0x20FB9A0` is the fill initialiser on page 4.
- Lines 60-69: walk the fills, and skip each one whose byte at +0x20 is zero (lines 68 and 69).
- Lines 70-77: keep the fill's +0x10 (its start tick) and +0x18 (its length), and make a new `#8323`.
- Lines 80-86: item +0x10 = the start tick turned into time by the chart's `0x212B9C0` (`dump.cs` line 543503, one `long` in, a `double` out).
- Lines 87-93: item +0x18 = the time from the start tick to start plus length, by `0x20CFCE0` (`dump.cs` line 543570), stored as a float.
- Lines 94-97: item +0x20 = the start tick, item +0x28 = the length in ticks (sign-extended from the fill's 32-bit +0x18).
- Lines 98-113: add the item to the list and go on.
