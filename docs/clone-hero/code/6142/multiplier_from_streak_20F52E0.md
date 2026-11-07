Listing: `multiplier_from_streak_20F52E0.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: base engine (#8288) virtual slot 10, RVA `0x20F52E0`. It sets the score multiplier at `engine+0x48` from the streak at `engine+0x88`.

Lines 1 to 2: if the streak is above 30 (compare with 0x1e), jump to line 11.
Lines 3 to 9: otherwise divide the streak by 10 (the compiler's multiply-by-reciprocal trick, 0x66666667 then shifts) and add 1. That is 1 for streaks 0 to 9, 2 for 10 to 19, 3 for 20 to 29, and 4 for exactly 30.
Line 11: a streak above 30 gives 4.
Line 12: store the result in the multiplier field `engine+0x48`.
Lines 13 to 16: if the Star Power flag at `engine+0x42` is set, double it and store it again.
Line 17: return.
