# sp_phrase_add_20CF0B0.asm (build 6142, RVA `0x20CF0B0`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (63 lines). Starts at the first instruction; `dump.cs` has no method at this address. Arguments: the track (`rcx`), a start tick (`rdx`) and a length (`r8`).

Plain words: rent a Star Power phrase object (class #8252) from the track's pool, store the start and length, and append it to the track's phrase list at `track+0x70`.

- Lines 15 to 23: the pool at `track+0x48` (the pool for #8252, `dump.cs` line 551848) and its rent method `0xFE7380`; the list at `track+0x70` is loaded at line 19.
- Lines 24 and 25: start tick to `+0x10`, length to `+0x18`.
- Lines 26 to 58: `List<#8252>.Add`, inlined: store in the spare slot, or `AddWithResize` (line 45).

Its only direct callers (byte scan) are the two `.mid` readers. In the drum reader `0x2155050` it is the pitch 116 case: the pitch jump table at RVA `0x2155B2C` sends pitch 0x74 to `0x215557C` (read from the DLL's bytes), which calls this with the Expert track, the note-on tick and note-off minus note-on (`mid_drum_reader_2155050.asm` lines 148 to 150 and 295 to 301). In the other `.mid` reader `0x2156990` it is the call at `0x2157291`, for the pitch that reader picked as its phrase pitch (116, or 103 under the rule at its `0x2156B85` to `0x2156C9B`).

The `.chart` reader builds the same object through `0x20D5E60`, which stores the same two fields (`+0x10` and `+0x18`), and appends it to the same `track+0x70` list (`docs/audit/drum-lanes/spec_213e0c0.asm` lines 31 to 63).
