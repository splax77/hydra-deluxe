# chart_loader_2161a80.asm (build 6142, RVA `0x2161A80`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (295 lines). Starts at the method's first instruction, the `.chart` loader.

Plain words: read every section of the file first, then run each track's finish. The finish `0x215C320` is what calls the coda skip `0x215DA70`, so the coda from `[Events]` is already set when any fill is marked, wherever `[Events]` sits in the file.

- Lines 148 to 152: the section loop. Line 148 (0x2161CC4) calls the section dispatcher 0x2150C00 for one section, and the jumps at lines 150 and 152 go back to the loop head at 0x2161B50.
- Lines 215 to 222: the track loop after it. Line 218 tests a byte at `[rax + 0x2c]` and skips the finish when it is set; that byte was not traced (a normal full load takes the finish). Line 221 (0x2161DEE) calls the track finish `0x215C320`, and line 222 jumps back for the next track.

Callers (direct callers only, byte scan on 2026-11-08 with a `call rel32`/`jmp rel32` scan of the `.text` and `il2cpp` sections): 0x21616E2, not read here.
