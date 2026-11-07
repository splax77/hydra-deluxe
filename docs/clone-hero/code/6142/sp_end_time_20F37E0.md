# SP end time, RVA 0x20F37E0 (build 6142)

Class `#8288` (the abstract base engine class, dump.cs line 529952). Name in script.json: `#8288.<obfuscated>`. Listing: `sp_end_time_20F37E0.asm`, made 2026-10-30 with `py -m tools.ch_mcp.chdis 20F37E0`. It starts at the function's first instruction (script.json lists 0x20F37E0 as a method start). The Ghidra decompile in `ghidra_decompile_all.txt` has no copy of this function, so the instructions are the evidence.

In plain words: it answers "if the player activates Star Power at song time S with M units of meter, at what song time does the meter run out?" It converts S to a tick position, adds M, and walks the song's tempo segments forward until the M units are used up. It returns that time as a double. The arguments are the engine (rcx), the meter as a signed 64-bit whole number (rdx) and the start time (xmm2).

Lines 1 to 3: save registers.
Lines 4 to 14: first call only: a one-time check that fills two generic-list method slots (the flag is a bss byte). It changes nothing the rule needs.
Lines 15 to 21: load the chart record at engine +0x230; if it is null, jump to line 94 (a null-reference throw).
Lines 22 to 24: load the tempo-segment list at engine +0x208 and the chart's double at +0xB0 into xmm8. That double is what the tick conversions below take as their "resolution".
Lines 25 to 30: ask the list for the segment that holds the start time (call to 0x18216A720); null result goes to line 94.
Lines 31 to 39: subtract the segment's start time (segment +0x18) from the start time and call 0x1820D1D20. Its formula (read at 0x20D1D20 on 2026-10-30: `ticks = dt * res * seg+0x30 / (seg+0x40 * 60)`) turns seconds into ticks inside that segment.
Lines 40 to 45: convert the meter (rbp) to a double and add the ticks from line 39. xmm6 is now "ticks still to burn, counted from the segment start".
Lines 44 to 48: xmm7 starts as the segment's start time; ebx is the segment index (segment +0x20); the loop at lines 47 to 77 stops when the index reaches the list count.
Lines 49 to 66: load segment i and segment i+1. If i is the last segment, go to line 78 (the rest lives in this segment).
Lines 66 to 70: the length of segment i in ticks is next+0x38 minus this+0x38 (the start-tick field of each). If the ticks still to burn are below that length, go to line 78.
Lines 71 to 77: otherwise subtract the whole segment from the ticks to burn, add that segment's duration in seconds (call 0x1820D4A70) to the running time, move to the next segment and loop.
Lines 78 to 83: the last, partial segment: convert the remaining ticks to seconds in this segment (0x1820D4A70) and add them.
Lines 84 to 93: return the running time in xmm0.
Line 94: the null-reference throw.

What it does not read: no time-signature list, no beat count, no fixed bar length. The end time depends on the meter amount, the activation time and the tempo map only.
