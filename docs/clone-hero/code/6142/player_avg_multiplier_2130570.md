Listing: `player_avg_multiplier_2130570.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: per-player stats (#8548) instance method, RVA `0x2130570`, the whole method. The stats filler calls it and stores the result as the record's `avg_multiplier` (`stats_fill_112490.asm` lines 154 and 155; `score_submit.asm` lines 357 and 358).

Lines 1 and 2: when `+0x14` (the score without the two bonuses, `engine_stats_20F3DC0.md`) is 0 or less, return 0 (lines 9 and 10).
Lines 3 to 7: otherwise return `+0x14` over `+0x10` (the star base), both turned into 32-bit floats first.
