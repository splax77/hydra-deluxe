# Base engine three-step loop, RVA 0x20F5330 (`#8288`)

Listing: `three_step_loop_20F5330.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` line 530314 gives this RVA, a private method of the base engine that takes one double. SP-43's scan (`docs/audit/2026-11-08-clone-hero-unknowns/sp43_immscan.py`) flagged its `cmp edi, 3` at line 56.

- Lines 3-9: call the virtual slot at offset 0x1b8 with `r8b = 1`.
- Lines 10-11: when the byte at +0x290 is zero, go to lines 68-75, which jump to `0x20F6FF0` once with -1.0 and return.
- Lines 17-19: keep -1.0 and save the song clock at +0x100.
- Lines 22-57: a loop of at most three passes (`edi` counts them, line 56). Each pass calls `0x20F6FF0`; stops when it returns -1.0 or a time not past the last one; turns that time into a clock value through `0x212BCC0` and the constructor `0x2117120`, stores it at +0x100, and calls slot 0x1b8 again.
- Line 62: put the saved clock back at +0x100.

The 3 is a cap on passes, the same shape as the step loop `0x20F4950` (`docs/audit/drum-lanes/play/f20f4950_step_loop.asm`). It is not a time.
