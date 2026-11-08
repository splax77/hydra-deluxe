Listing: `player_is_pfc_212F4F0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: per-player stats (#8548) instance method, RVA `0x212F4F0`, the whole method. The stats filler calls it and stores the result as the record's `is_pfc` (`stats_fill_112490.asm` lines 176 and 177; `score_submit.asm` lines 412 and 413).

Lines 1 to 3: false unless the best streak `+0x20` equals the note total `+0x44`.
Lines 4 and 5: false unless the best streak is above 0.
Lines 6 and 7: then true when the byte `+0xA8` is 0. The engine stats method copies that byte from the engine's `+0xA8` (`engine_stats_20F3DC0.asm` lines 238 and 239); what sets the engine's byte was not traced.
