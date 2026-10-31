# note_window_20F3A90 (build 6142, made 2026-10-30 with tools/ch_mcp/chdis.py)

Function: `#8288.ʴʼʶʿʴʲʶʵʼʳˁ(note)`, a protected method of the abstract base engine class `#8288` (`dump.cs` line 530284), RVA `0x20F3A90`; the first instruction is the function start. It returns half of a note's hit window, in seconds. Not in the committed decompile `ghidra_decompile_all.txt` (only its two call sites are, at lines 52946 and 53619). Fields used: `this+0x20` total window, `this+0x30` back constant, `this+0x38` front constant, `this+0x41` dynamic-window flag, `this+0x1e0` the note list, `note+0x20` index into it, `note+0x40` the neighbour-lookup argument.

Lines 1 to 9: prologue; line 4 tests the flag byte `this+0x41`; line 9 jumps to the fixed branch (line 64) when it is 0.

Lines 10 to 18: the dynamic branch loads front (`this+0x38`, into xmm7) and back (`this+0x30`, into xmm8) and does the one-time generic setup. Line 19 to 20: a null note throws (line 72).

Lines 21 to 29: the neighbour note on one side is looked up in the note list (call at line 29). Lines 30 to 37: the neighbour on the other side is looked up (call at line 37).

Lines 38 to 45: `0x20F7210` (half-gap clamp) is called for the note and the first neighbour, with front as the minimum and back as the maximum. Lines 46 to 53: it is called again for the other neighbour. Line 55 adds the two results.

Lines 56 to 59: the sum is passed to the virtual method at vtable offset 0x268 (slot 19), which on the drums engine is `hit_window_formula_20DDDA0`; its result (seconds) is left in xmm0.

Lines 60 to 63: restore registers and jump to the common exit.

Line 64, the fixed branch (flag 0): the result is `this+0x20` as is.

Line 65, common exit: the result is multiplied by 0.5 (constant at 0x3063FB0 is 0.5), so the function returns half a window. Lines 66 to 71: epilogue. Lines 72 to 73: null-reference throw.

What calls it: the per-frame functions `0x20F5810` and `0x20F6A50` (decompile lines 52946 and 53619). At 52946 the result is stored at engine+0x10 and doubled into engine+0x20 (lines 52946 to 52950), which is how the stored total window equals the formula's output.
