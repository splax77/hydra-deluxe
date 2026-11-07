# Star base score, RVA 0x20F4680 (build 6142)

Class `#8288`, a private method (dump.cs line 530104: `private int ...(bool, double)`). Listing: `star_base_20F4680.asm` (chdis, 2026-10-30), 169 lines. The Ghidra decompile has no copy of this function (a search for `FUNC @ RVA 0x20f4680` found nothing), so the instructions are the evidence. The only direct caller found by a byte scan is the base constructor 0x20F79E0 at 0x20F7DCC (`engine_ctor_20F79E0.asm` line 196), which stores the result at engine +0x1D0 (line 202).

In plain words: it adds up what a perfect run of the chart would score before any multiplier, and that sum is what the star cutoffs are percentages of. For drums, every note counts 50 points per gem, plus 15 for a cymbal, doubled when the note is a ghost or an accent. For other instruments it adds sustain length instead.

Arguments: rcx the engine, dl a flag the constructor sets when the engine's kind byte (+0x218) is 6 or 9 (constructor lines 183 to 189; the flag is the drums case, not proven by name here), xmm2 the chart record's double at +0xB8 (constructor line 193).

Lines 1 to 28: prologue and one-time init.
Lines 29 to 39: xmm0 = xmm2 divided by 3.0 and rounded by 0x183031130 (a thin wrapper that jumps to one of two rounding routines); the result truncated to a whole number at line 49 and kept at [rsp+0x28]. It is the shortest sustain, in ticks, that counts (used only on the non-drums path).
Lines 40 to 44: load the engine's note list from +0x1E0; null goes to line 161 (null throw).
Lines 45 to 60: r15 (the points total) and r14 (the sustain tick total) start at zero; esi (the dynamics multiplier) starts at 1; get the list enumerator.
Lines 62 to 66: loop: MoveNext; false jumps to line 126 (the end).
Line 68: test the drums flag (r12b). Non-zero jumps to line 83.
Lines 67 to 82 (not drums): if the note's long at +0x48 is at least the threshold, add it to r14, otherwise add 0 (lines 73 to 82); jump to line 103.
Lines 83 to 90 (drums): call 0x18214BE30, which is `test word [note+0x80], 0xE0` (three lane-mask bits). r13 becomes 0xF (15) when it is true, 0 when false.
Lines 91 to 102 (drums): esi becomes 2 when 0x18214BE10 (`test dword [note+0x7C], 0x80000`) or 0x18214B6C0 (`test dword [note+0x7C], 0x40000`) is true; otherwise 1.
Lines 103 to 119: gem count. If the note's int at +0x50 is -1, count the set bits of the note's 16-bit mask at +0x80; otherwise use +0x50.
Lines 120 to 125: r15 += (50 + r13) times gem count times esi. The 50 is the literal 0x32 at line 120.
Lines 126 to 138: end of loop, dispose the enumerator; exception cleanup.
Lines 139 to 149: r14 as a double, divided by (xmm2 divided by 25.0), rounded by 0x183031130, truncated, added to r15. For drums r14 is 0, so the result is r15 alone.
Lines 150 to 160: restore registers and return the sum in eax.
Lines 161 to 168: null throws and an unwind trap.

The two dynamics bits are named in the dump: 0x40000 is `DrumsGhostHit` and 0x80000 is `DrumsAccentHit` (dump.cs lines 548215 and 548216). The cymbal test is "any of three lane-mask bits"; the committed drum listings use the same getter 0x18214BE30 (`docs/audit/drum-lanes/play/f20deec0.asm` line 13), where the same +15 is added.
