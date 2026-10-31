Listing: `star_progress_update_2DF7F0.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: `StarProgress` (#1169) method `ʺʽʸʺʾˀʲʾʹʲʻ`, RVA `0x2DF7F0`. It is the per-frame update of the HUD star bar; the argument is the current score. `ScoreManager.Update` (RVA `0x1286B0`) calls it at 0x128837. Four near-copies exist (`0x2DF200`, `0x2DFC20`, `0x2E0150`, `0x2E0540`); this is the only one with a direct caller found, and two of the others start at slot 1.

Lines 1 to 10: prologue and one-time setup.
Line 11: if the "needs rebuild" byte at `+0x88` is clear, jump to line 122 (skip the rebuild).
Lines 19 to 34: rebuild. Star index `+0x58` and the previous-cutoff field `+0x60` become 0, the last-score field `+0x7C` becomes -1, and the two star sprites are cleared.
Lines 35 to 38: load the engine list at `+0x80`. If it holds exactly one engine, jump to line 112 (copy that engine's own int array).
Lines 39 to 56: otherwise sum the star base score at `engine+0x1D0` over the list, from index 0.
Lines 57 to 101: walk the nine cutoff slots from slot 0: read table slot i, multiply the float base by it (`mulss`), widen, call the rounding helper 0x183031130, cut to int, and store in slot i of the int array at `+0x50`.
Lines 112 to 120: single-engine case: Array.Copy the engine's int array at `engine+0xF8` into `+0x50`.
Line 121: clear the rebuild byte.
Lines 122 to 145: while the star index `+0x58` is below the cap `+0x5C` (7, set in the constructor at `0x2E0990`) and the score is at or above cutoff slot `+0x58`, show the star sprite for that index, play the particles, and store the new index.
Lines 177 to 249: the bar's fill ratio between the previous and the next cutoff, written to the bar's scale and position.
