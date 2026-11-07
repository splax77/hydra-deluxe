# Star Power deactivation, RVA 0x20F5490 (`#8288` slot 23)

Listing: `sp_deactivate_20F5490.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. First instruction is the function start (dump.cs line 530317, `Slot: 23`). Engine object in `rbx`.

- Lines 1-8: first-use setup of a type token.
- Lines 9-16: multiplier (+0x48) divided by 2 (rounding toward zero), active flag (+0x42) set to 0.
- Line 17: amount (+0x68) set to 0.
- Lines 18-21: total SP duration (+0xb8) += clock (+0x100) minus activation time (+0x110).
- Lines 22-23: store the clock as the end clock at +0x128.
- Lines 24-26: zero the drain counters (+0x138, +0x140) and the end time (+0x120).
- Lines 27-33: if a listener exists, tell it with event code 14.
- Lines 34-36: otherwise return.
