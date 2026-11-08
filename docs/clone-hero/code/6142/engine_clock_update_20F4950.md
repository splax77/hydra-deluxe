# engine_clock_update_20F4950.asm (build 6142, RVA `0x20F4950`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (115 lines). Starts at the method's first instruction. `dump.cs` line 530329 names it as a public instance method of the base engine class #8288 taking one `double`.

Plain words: move the engine's clock pair at `+0x100` (a time in seconds and its chart tick) to the given time, first stepping through up to three earlier times that `0x20F6FF0` returns.

- Line 22: save the old pair.
- Lines 25 to 60: up to three times, ask `0x20F6FF0` for a time; while it returns one, set the pair to (that time, its tick from `0x212BCC0`) at line 55 and call the engine's virtual method at vtable `+0x1B8` (line 57).
- Line 61: put the saved pair back.
- Lines 72 to 85: set the pair to (the given time, its tick from `0x212BCC0`), stored at line 85.
- Lines 92 to 97: the new tick at `+0x108` goes to `0x20D4AA0` with the tempo segment at `+0x148`, and the result is stored back at `+0x148`.

Every store to `+0x100` here builds the pair with the struct constructor `0x2117120` from a time and `0x212BCC0` of that time, or restores the saved pair.
