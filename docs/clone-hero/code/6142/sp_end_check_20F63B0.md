# Star Power end check, RVA 0x20F63B0 (`#8288` slot 15)

Listing: `sp_end_check_20F63B0.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. First instruction is the function start (dump.cs: `Slot: 15` for the base class). Engine object in `rcx`.

- Lines 1-3: load the song clock (+0x100) and compare it with the end time (+0x120). If the clock is below the end (`jb`), jump to the return at line 9. So a clock equal to the end continues.
- Lines 4-5: if the active flag (+0x42) is 0, return.
- Lines 6-8: clock at least end and SP on: jump through virtual slot 23 (`0x20F5490`, the deactivation, offset 0x2a8).
- Line 9: return.

The drum engine's own slot 15 is RVA `0x20DE5A0` (dump.cs line 527475). Its whole listing, read with `chdis`:

```
020de5a0: xor    edx, edx
020de5a2: jmp    0x1820f63b0
```

So drums use this check unchanged.
