# add_fill_215BD50.asm (build 6142, RVA `0x215BD50`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (71 lines). Starts at the method's first instruction (`#8661.ʳˁʾʵˀʿʲʲʼʸʽ`, instance method of the track class, parameters `long`, `long`, `bool`).

Plain words: take a fill object from the object pool, fill it with (tick, length, flag) and append it to the track's fill list at `track+0x88`. It makes no decision about fills.

- Lines 1 to 16: stack set-up and the one-time generic-slot load.
- Lines 17 to 19: `rcx = [track+0x30]`: the pool. A null pool jumps to the null exit (line 67).
- Lines 20 to 24: `rbx = [track+0x88]` (the fill list) and a call to the pool's rent method (`0x180FE7380`); a null result exits.
- Lines 25 to 30: call the fill constructor-like method `0x1820FB9A0` with (the rented object, `rdx` = first `long`, `r8` = second `long`, `r9b` = the bool, 0).
- Lines 31 to 66: append the result to the list: bump the version field (`inc [list+0x1c]`, line 35), then either store into the spare slot (lines 53 to 59) or call `List.AddWithResize` (line 52).

Callers: none found. A scan of the `il2cpp` section for `call rel32` and `jmp rel32` instructions to `0x215BD50` found no site on 2026-10-30 (scan script `callers.py` in this folder); `docs/audit/ch-evidence.md` line 100 says the same and guesses a delegate. This method is therefore not shown to run; the fills that exist come from the readers that rent fills themselves (`0x20CEDC0`, `0x213E0C0`, `0x20E5E70`, `0x215B7C0` per ch-evidence line 97 to 99, not re-read here) or from the generator's own wrapper objects.
