Listing: `score_entry_write_2BAB60.asm`, build 6142, made 2026-11-11 with `tools/ch_mcp/chdis.py` (`py -m tools.ch_mcp.chdis 2BAB60`). Function: the method at RVA 0x2BAB60 of the score entry struct TypeDefIndex 1150 (`dump.cs` lines 61014 to 61101), which takes a `BinaryWriter`. The struct's fields: instrument (enum 8326) at +0x0, difficulty (enum 8617, `dump.cs` lines 550049 to 550059: Easy 0 to Expert 3) at +0x1, int at +0x4, bool at +0x8, short at +0xA, byte at +0xC, modifiers (flags enum 8576, `dump.cs` lines 547675 onward) at +0x10, byte at +0x14, bool at +0x15, ints at +0x18 and +0x1C.

Plain version: it writes one instrument's entry in 16 bytes: instrument, difficulty, a byte, a flag, a short, a byte, the modifiers and an int. Three fields are not written: the bool at +0x15 and the ints at +0x18 and +0x1C.

`BinaryWriter` slots as in `score_chart_write_2167B0.md`, plus 0x208 slot 13 `Write(bool)`, 0x288 slot 21 `Write(short)`, 0x2a8 slot 23 `Write(int)` and 0x2b8 slot 24 `Write(uint)`.

In order: lines 8 to 12 write the instrument, sign-extended, with `Write(ushort)`, so 2 bytes. Lines 15 to 17 write the difficulty byte at +0x1. Lines 20 to 22 write the byte at +0x14. Lines 25 to 27 write the bool at +0x8. Lines 30 to 32 write the short at +0xA. Lines 35 to 37 write the byte at +0xC. Lines 40 to 42 write the modifiers at +0x10 as a uint. Lines 45 to 50 write the int at +0x4 and return through the tail jump.

The int at +0x4 is the one the merge keeps the larger of and uses to pick which run's difficulty and modifiers stay (`score_entry_merge_2BA4F0.asm` lines 47 to 68). The getter at 0x2BA7A0 returns the larger of +0x4 and +0x18. The names of the other fields were not read.

Who calls it, direct callers only (byte scan with `docs/audit/2026-11-08-clone-hero-unknowns/W8c/xref.py`): the three chart record writers 0x215F80, 0x2167B0 and 0x216FA0 of class 907.
