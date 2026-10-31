# clock_ctor_16C9A0.asm  (build 6142, RVA 0x16C9A0, #707 .ctor, the song clock)
Made with chdis.py, 2026-10-30, session b02ab3e6, scout R7. Method start per script.json. Arguments: xmm1 = length a (double), xmm2 = lead-in b (double), xmm3 = speed ratio c (float), then two floats on the stack, d (audio calibration) and e (video minus audio).
Lines 2-14: prologue; the flag at +0x28 is set to 1 (line 11) and the float at +0x30 to 1.0f (0x3F800000, line 13); line 14 calls the base object constructor.
Lines 15-20: +0x48 = (double) d (line 20, the audio calibration in seconds); c is widened to double.
Lines 21-24: xmm8 = b / c, xmm6 = a / c (two divides).
Line 25: xmm8 = -(b / c) (sign flip with the sign-bit mask at 0x3064930).
Lines 26-32: +0x10 = a / c (line 29, the clock-time length); +0x2C = c (line 30, the speed ratio, readonly); +0x50 = (double) e (line 31, video minus audio); +0x58 = -(b / c) (line 32, the clock time at audio position 0). Because the clock reads audio position / c + (+0x58) (see clock_tick_16C650.md), audio position b maps to clock time 0: a positive lead-in puts the notes later in the audio.
Lines 27, 33-38: the starting raw clock. If -1.0 is greater than +0x58, the raw time (+0x38 and +0x40) starts at +0x58 - 1.0; otherwise it starts at -1.0. So the clock starts at least one second before chart time 0.
Lines 39-57: epilogue.
