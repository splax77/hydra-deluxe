# section_loop_2150c00.asm (build 6142, RVA 0x2150C00)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (173 lines). Starts at the method's first instruction, the `.chart` section dispatcher the loader `0x2161A80` calls once per section.

Plain words: compare the section's name with `Song`, `SyncTrack` and `Events`, and hand the section to the reader for that name. An `[Events]` section goes to 0x2151830, the reader that finds the coda. It makes no decision about fills.

- Lines 17 to 23: the one-time loads of the three section names.
- Lines 26 to 50, 54 to 77 and 81 to 104: the three name compares (`MemoryExtensions.SequenceEqual<char>` at lines 50, 77 and 104). Line 81 loads `Events`.
- Line 116: `call 0x2151650`, a second section reader, not read here.
- Line 131: `call 0x2151830` (0x2150DFB), the `[Events]` reader.

Callers (direct callers only, byte scan on 2026-11-08 with a `call rel32`/`jmp rel32` scan of the `.text` and `il2cpp` sections): 0x2161CC4 in the `.chart` loader `0x2161A80` (`chart_loader_2161a80.asm` line 148).
