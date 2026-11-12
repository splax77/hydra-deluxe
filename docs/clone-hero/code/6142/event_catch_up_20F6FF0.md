# Base engine event catch-up, RVA 0x20F6FF0 (`#8288`)

Listing: `event_catch_up_20F6FF0.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. `dump.cs` line 530251 gives this RVA, a protected method of the base engine that takes two doubles and returns a double. The step loop `0x20F4950` calls it at each sub-step (`docs/audit/drum-lanes/play/f20f4950_step_loop.asm` lines 29 and 67), and so does `0x20F5330` (`three_step_loop_20F5330.asm` lines 26 and 75).

Only one part is read here. Lines 95-104 walk a list backwards (`ebx` counts down, line 104) and, for each item whose time lies between the two arguments, call the frame update `0x20F4550` with that time and `r8d = 0` (lines 100-103). It is the one caller of the frame update found that passes `false` (byte scan of direct calls; the other eleven pass 1). The rest of the method was not read.
