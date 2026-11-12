Listing: `engine_stats_20F3DC0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: engine (#8288) instance method, RVA `0x20F3DC0` (`dump.cs` line 530173), the whole method. It builds a new per-player stats object (#8548, in `rdi`) from the engine (`rbp`) and returns it. A byte scan for code that loads the engine's `+0xD0` and stores it at `+0x3C` found only this method.

Lines 199 to 207: count stars on the engine's own cutoff array: step the index while the score at `+0x94` is at or above the slot, up to 7. The count goes to the stats object's `+0x94` at line 233.
Lines 219 and 220: the engine's score `+0x94` goes to `+0x14`.
Lines 221 and 222: the star base `+0x1D0` goes to `+0x10`.
Lines 234 to 237: `+0xA0` goes to `+0x18` (the solo bonus) and `+0xA4` to `+0x1C` (the clean-play bonus).
Lines 240 to 248: `+0x98` (best streak) to `+0x20`, `+0x88` to `+0x24` (written as `end_streak`, `stats_fill_112490.asm` lines 135 and 136, `score_submit.asm` line 312), `+0x9C` to `+0x40`, `r12d` to `+0x44`, `+0x4C` to `+0x48`.
Lines 252 to 269: `+0x80` over `+0x1B0` as a float to `+0x50`; the two longs to `+0x58` and `+0x60`; `+0xB8` to `+0x70`; `+0xB0` to `+0x28`; `+0xB4` to `+0x68`.
Lines 209 to 211 and 270 to 275: `+0x2C` is the points total `+0xC0` less 50 times (`+0xD8` plus `+0xDC`).
Lines 276 to 300: `+0xC4` to `+0x30`, `+0xCC` to `+0x38`, `+0xD0` to `+0x3C`, `+0xC8` to `+0x34`, `+0xD4` to `+0x78`, `+0xD8` to `+0x7C`, `+0xDC` to `+0x84`, `+0xE4` to `+0x88`, `+0xE8` to `+0x8C`, `+0xEC` to `+0x90`, `+0xE0` to `+0x80`, `+0xAC` to `+0x98`.

Which JSON key each stats field becomes is in `stats_fill_112490.md` and `score_submit.asm`.
