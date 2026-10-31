# engine_ctor_20DF680.asm (build 6142, RVA `0x20DF680`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (101 lines). This is a support listing, kept because the brief's address `0x20DF7B7` sits inside it, not inside the engine update. Script.json: `0x20df680` is `#8255..ctor` (the drum engine class), and `0x20DF7B7` is 0x137 bytes into it.

Plain words: builds the drum engine, copies its settings, and sets the two hit-window constants. The part this reference needs: it reads the chart's resolution, shifts it left by two (times four) and stores it in the engine at `+0x300`.

- Lines 1 to 14: stack set-up and the one-time generic-slot load.
- Lines 15 to 44: copy the 160-byte settings struct (`rsi`) to the stack and call the base constructor `0x18100EAE0`.
- Lines 45 to 48: `[engine+0x318]` = a reference from the second argument.
- Lines 49 to 53: copy a byte and an int from the second argument into `+0x2f6` and `+0x2c0`.
- Lines 54 to 56: `rax = [engine+0x230]` (the chart object); null exits.
- Line 57: `cvttsd2si rax, qword ptr [rax+0xb8]`: truncate the chart's resolution (a double) to a whole number.
- Lines 58 to 67: zero a run of engine fields (`+0x2c4` to `+0x2f0`). Note `+0x2f8` (the fill-list index the deadline method uses) is not among them, so it keeps the zero the allocator gave it.
- Line 68 (`0x20DF7B7`): `shl rax, 2`. Line 69: `[engine+0x300] = rax`. So `+0x300 = trunc(resolution) * 4`, four quarter notes in ticks in every meter (the number of ticks in a quarter note is the resolution).
- Lines 70 to 77: more fields.
- Lines 75 and 79 to 84: `test [engine+0x198], 0x1000` (the precision-mode bit): normal mode stores 0.085 into `+0x30` (back window) and 0.0375 into `+0x38` (front window) (lines 80 to 81 and the comments' constants); precision mode stores 0.04 and 0.025 (lines 83 to 84).
- Lines 85 to 91: `[engine+0x30]`, `[engine+0x38]`, and `[engine+0x20] = 2 * back window`.
- Lines 92 to 98: a final init call and return.
