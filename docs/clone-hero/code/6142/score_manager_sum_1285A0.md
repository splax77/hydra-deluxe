Listing: `score_manager_sum_1285A0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: `ScoreManager` (#592) method, RVA `0x1285A0`, the whole method. It sums the HUD's totals over the engines in its array at `+0x38` (type #8288[], `dump.cs` line 31538).

Lines 13 to 15: set `+0x40`, `+0x44`, `+0x48` and `+0x68` to 0.
Lines 30 to 36: for each engine, add its score `+0x94` to `+0x40`, its solo bonus `+0xA0` to `+0x44` and its `+0x88` to `+0x48`.
`score_manager_update_1286B0.asm` lines 91 to 93 then pass `+0x40` alone to the HUD star bar (`StarProgress`, `star_progress_update_2DF7F0.md`).
