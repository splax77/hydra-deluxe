Listing: `star_count_210DD00.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. Function: #8351 static method, RVA `0x210DD00`. Arguments: the score (`ecx`, kept in `esi`) and the star base score (`edx`, kept in `edi`). It returns the star count.

Lines 1 to 13: prologue and one-time class setup.
Line 15: the star index `ebx` starts at 0.
Lines 18 to 30: load the star table (`#8611` static field) and read table slot `ebx`; a bounds check (line 27) stops on an index past the end.
Lines 35 to 37: turn the base score into a 32-bit float and multiply it by the table slot (`mulss`).
Lines 46 to 48: widen to a double, call the rounding helper at 0x183031130, and cut to an int: the cutoff.
Lines 49 to 50: if the score is below the cutoff, stop (the count is the current index).
Lines 51 to 53: otherwise add 1 to the index; if the index is still below 7, go round again. So the count never exceeds 7, and the table slots 7 and 8 are never read.
Lines 55 to 61: return the index.
