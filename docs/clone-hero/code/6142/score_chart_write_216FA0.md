Listing: `score_chart_write_216FA0.asm`, build 6142, made 2026-11-11 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 216FA0`). Function: the method at RVA 0x216FA0 of the chart record class TypeDefIndex 907 (`dump.cs` line 49328), which takes a `BinaryWriter`.

Plain version: a second copy of the chart record writer. Its body matches `score_chart_write_2167B0.asm` instruction for instruction, at the same listing lines: the empty check at lines 29 to 35, the hash at 36 to 54, the byte at +0x2A at 55 to 59, the ushort play count at +0x28 at 60 to 64, the byte at +0x2B at 65 to 69 and the entry loop calling 0x2BAB60 at 70 to 104. See that note for what each field is.

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W8c/xref.py`, output in `xref_IN-55.out.txt` there): none found by a scan for `call`/`jmp` rel32 and RIP-relative references to 0x216FA0. The `scoredata.bin` writer at 0x2F57C0 calls 0x2167B0, not this copy.
