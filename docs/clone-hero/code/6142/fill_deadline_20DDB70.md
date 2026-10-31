# fill_deadline_20DDB70.asm (build 6142, RVA `0x20DDB70`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (72 lines). Starts at the method's first instruction (`#8255.AddStarPower`, instance method of the drum engine class, parameters `(long amount, bool)`). It is an override: its last instruction (line 69) tail-jumps to the base class method with the same name at `0x20F4FA0`, passing the same amount, `r8b = 1`, `r9d = 0`.

Plain words: just before the engine adds `amount` ticks of Star Power, if that addition would carry the meter from below half to half or more, and Star Power is not already active, then clear every written or generated fill whose start tick is at or before (current tick + four quarter notes). A cleared fill loses its activation flag, so it can no longer be used to activate. After the clean-up the base method does the adding.

- Lines 1 to 8: save registers. `rax = [engine+0x1b8]`, `r14 = amount` (`rdx`), `rbx = engine`.
- Lines 9 to 10: `[engine+0x68] >= [engine+0x1b8]` (the meter already at or above the limit) jumps to the tail (line 60). `+0x68` is the Star Power amount in ticks and `+0x1b8` is twice `round(7.5 * resolution)` (base engine constructor `base_engine_ctor_20F79E0.asm` lines 156 to 170: `+0x1c0 = x`, `+0x1b8 = 2x`, `+0x1b0 = 4x`), which is half of the full meter at `+0x1b0`.
- Lines 11 to 14: `amount_now + amount < [engine+0x1b8]` also jumps to the tail: the addition does not reach half.
- Lines 15 to 16: `[engine+0x42] != 0` (Star Power active) jumps to the tail.
- Lines 17 to 23: one-time generic-slot load.
- Lines 24 to 26: `rsi = [engine+0x300] + [engine+0x108]`. `+0x300` is `resolution << 2`, written by the constructor (`engine_ctor_20DF680.asm` lines 57 and 68 to 69). `+0x108` is added to it; the record reads `+0x108` as the current chart tick, but its field meaning and writer were not checked here (see FL-22 on page 4).
- Line 28: `edi = [engine+0x2f8]`: a running index into the fill list, so each call continues where the previous one stopped.
- Lines 29 to 31, 34 to 38: fetch the wrapper list through `0x20F4650(engine, 0)` and test that the index is below its count; if not, go to the tail.
- Lines 39 to 44: fetch wrapper number `edi`.
- Line 45: `cmp [wrapper+0x28], rsi; jg` stops the loop (stores the index, line 59) when the wrapper's fill start tick is greater than the deadline. `+0x28` is the fill start tick that `20CFF60` and `20D2B30` write. So the rule is `start <= current + 4 resolution` clears.
- Lines 47 to 51: `[wrapper+0x38] = 0` (line 48), then `and dword [wrapper.note+0x7c], 0xfffff7ff` (line 51): clear the `0x800` activation flag on the wrapper's chord.
- Lines 53 to 58: next index, re-fetch the list, loop (the compare at line 32).
- Lines 59: store the index; lines 60 to 69: the tail jump to the base method. Lines 70 to 71: null exit.

The 0x20DF7B7 instruction is not in this function. It is inside the drum engine constructor `0x20DF680` (class `#8255`, `.ctor`), at line 68 of `engine_ctor_20DF680.asm`: `shl rax, 2` just after `cvttsd2si rax, [chart+0xb8]` (line 57). The next instruction (line 69) stores the result into `[engine+0x300]`.

Who calls it: no direct call found (scan of the `il2cpp` section, 2026-10-30); it runs as a virtual override, so it is reached through the engine's vtable.
