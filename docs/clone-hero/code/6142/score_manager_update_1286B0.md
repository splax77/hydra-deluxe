Listing: `score_manager_update_1286B0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: `ScoreManager.Update` (#592), RVA `0x1286B0`, the whole method.

Line 15: call `0x1285A0`, which sums the engines' totals (`score_manager_sum_1285A0.md`).
Lines 76 to 85: when the total at `+0x48` changed, keep it at `+0x54` and pass it to the combo font at `+0x80`.
Lines 88 to 93: pass the summed score at `+0x40` to the HUD star bar at `+0x90` (`StarProgress`, RVA `0x2DF7F0`).
