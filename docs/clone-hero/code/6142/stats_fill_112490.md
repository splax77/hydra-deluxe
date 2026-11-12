Listing: `stats_fill_112490.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: #561 method, RVA `0x112490`, the whole method. It fills one per-player block of the stats record (#561) from a per-player stats object (#8548, in `rsi`). `EndOfSong.Start` reaches it through `0x251A70`, which builds the block and adds it to the record's player list (`scorestats_writer_15DC20.md`, lines 208 to 245 of that listing). The JSON writer `score_submit.asm` then writes the block's fields by name: `score` from `+0x34` (its line 252), `base_score` from `+0x38` (line 257), `solo_bonus_total` from `+0x3C` (line 262), `squeezed_score` from `+0x50` (line 267) and `clean_play_bonus` from `+0x54` (line 272).

Lines 107 to 110: `score` (`+0x34`) is the return of `0x2130490` on the #8548 object, which is its `+0x14` plus `+0x18` plus `+0x1C` (`player_total_score_2130490.asm`).
Lines 112 and 114: `base_score` (`+0x38`) is the object's `+0x10`.
Lines 115 and 116: `solo_bonus_total` (`+0x3C`) is the object's `+0x18`.
Lines 125 and 126: `squeezed_score` (`+0x50`) is the object's `+0x3C`.
Lines 127 and 128: `clean_play_bonus` (`+0x54`) is the object's `+0x1C`.
Lines 117 to 124 and 129 on copy the object's other ints into the block's other fields; which JSON key each feeds is in `score_submit.asm`.
