Listing: `band_stars_210E2A0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: #8351 static method, RVA `0x210E2A0`, the whole method. It takes one #8351 object (the band's totals) and fills it from its list of per-player stats objects (#8548). It is one of the four direct callers of the star counter `0x210DD00` (`star_count_210DD00.asm`); the other three are `band_stars_210F140.asm`, `band_stars_21104F0.asm` and `band_stars_2110920.asm`, which do the same steps at the lines given below. A byte scan found one direct caller of this method, a GameManager method at `0xBC750` (call at 0xBC835), and no direct caller of the other three.

Lines 40 and 72 to 79: take the player list at `+0x50` and keep the players that pass a filter (a `Where` with a lambda, then `ToList`).
Lines 81 to 84: set the totals at `+0x10` and `+0x18` to 0 (two 8-byte stores, so `+0x14` and `+0x1C` too) and the two flags at `+0x24` and `+0x25` to 1.
Lines 108 to 118: for each player, add its `+0x14` to `+0x18`, its `+0x10` to `+0x10`, its `+0x18` to `+0x14` and its `+0x1C` to `+0x1C`.
Lines 167 to 170: an average of a count over the player count, stored as a byte at `+0x4C`.
Lines 171 to 175: call the star counter with the score argument `ecx` = `+0x18` (the sum of the players' `+0x14`) and the base argument `edx` = `+0x10` (the sum of the players' star base); store the count at `+0x20`.
Lines 176 to 181: only after that, add `+0x14` (the players' `+0x18`, the solo bonus) and then `+0x1C` (the players' `+0x1C`, the clean-play bonus) into `+0x18`.

In the other three listings the per-player sums are lines 110 to 117 (`band_stars_210F140.asm`, `band_stars_21104F0.asm`) or 111 to 118 (`band_stars_2110920.asm`), the star counter call is line 172, 174 or 175, and the two additions after it are lines 174 to 179, 176 to 181 or 177 to 182.

Which player field is which comes from `stats_fill_112490.asm` and `player_total_score_2130490.asm`.
