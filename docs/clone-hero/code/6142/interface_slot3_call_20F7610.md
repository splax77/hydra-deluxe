# Base engine counter hand-off, RVA 0x20F7610 (`#8288`)

Listing: `interface_slot3_call_20F7610.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` line 530197 gives this RVA, a public method of the base engine with no arguments. SP-43's scan (`docs/audit/2026-11-08-clone-hero-unknowns/sp43_immscan.py`) flagged its `mov r8d, 3` at line 77 and `add eax, 3` at line 112.

- Lines 11-17: do nothing unless the byte at +0x43 is set, then clear it.
- Lines 24-48: when +0x164 is at least 0x32 (50), work out a number from +0x168 and +0x16c (their ratio less 0.5, doubled, times +0x168 times 10, then through `0x2C49A60` and times 10), else zero; add it to +0xa0.
- Lines 50-88: call method 3 of an interface on the object at +0x1a0, passing that number, +0x164, whether +0x168 is +0x16c less one, +0x170 and more. Lines 62-76 search the object's interface table for the interface; lines 109-117 add 3 to the found offset to reach its method 3; lines 77-80 are the fallback lookup `0x2F65070` with 3 as the method number. The same lookup shape is in `drums_input_update_100DE20.asm` lines 28-44, with 0 there.
- Lines 91-104: step +0x160 up to the end of the list at +0x200, then clear +0x164 to +0x170.

Both 3s are an interface method number, not a time.
