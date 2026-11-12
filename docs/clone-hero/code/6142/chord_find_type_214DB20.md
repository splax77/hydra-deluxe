# chord_find_type_214DB20 (build 6142, RVA 0x214DB20)

A method of the note class that finds another note of the same chord with a given type. Its arguments are the note (rcx), the track (rdx) and the type (r8d). Made 2026-11-12 with `tools/ch_mcp/chdis.py` (43 lines, the whole method).

Lines 14 and 15 read the index range of the note's chord, from `+0x2C` to `+0x30`. Lines 17 to 35 walk that range in the track's note list `+0x60`, skip the note's own index `+0x28` (line 17), and return the first note whose `+0x20` type equals the asked type (line 31). Line 36 returns null when none matches. The conversion step `five_lane_convert_215D450.asm` calls it at lines 80, 99, 168 and 175.
