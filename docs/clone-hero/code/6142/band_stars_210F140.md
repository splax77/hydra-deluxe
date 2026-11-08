Listing: `band_stars_210F140.asm`, build 6142, made 2026-11-08 with `tools/ch_mcp/chdis.py`. Function: #8351 static method, RVA `0x210F140`, the whole method. It is a near-copy of `band_stars_210E2A0.asm`; that file's `.md` explains the steps. A byte scan found no direct caller of this method.

Lines 110 to 117: for each player, add its `+0x14` to `+0x18`, its `+0x10` to `+0x10`, its `+0x18` to `+0x14` and its `+0x1C` to `+0x1C`.
Line 172: call the star counter `0x210DD00` with the score argument `ecx` = `+0x18` and the base argument `edx` = `+0x10`; the count goes to `+0x20` on the next line.
Lines 174 to 179: only after that, add `+0x14` (the solo bonus) and then `+0x1C` (the clean-play bonus) into `+0x18`
