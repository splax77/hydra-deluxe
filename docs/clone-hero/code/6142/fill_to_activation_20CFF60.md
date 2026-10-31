# fill_to_activation_20CFF60.asm (build 6142, RVA `0x20CFF60`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (180 lines). Starts at the method's first instruction (`#8265.ʳʷˁʸʵʾʼʷˀʲˁ`, static: `(List<chord> notes, #8538 chart, #8661 track)`, returns `List<wrapper>`). 0x20D0061 to 0x20D008D (the slop stretch) is inside it: lines 55 to 62.

Plain words: for every written fill on the track that is not marked "skip", find the chord the fill ends on (using `0x5DE030`), put the activation flag `0x800` on that chord's note, and record a small wrapper object (fill start tick, the chord, an "active" byte). It returns the list of wrappers. The deadline method `0x20DDB70` later clears wrappers and flags that are too close.

- Lines 1 to 35: stack set-up and the one-time generic-slot loads.
- Lines 36 to 39: clear the enumerator's stack slots.
- Lines 40 to 45: `r13` is the track. A null track exits (line 170). `rbx = [track+0x88]`: the fill list. A null list exits. `ebx = list.Count` (line 45), used only to size the result list.
- Lines 46 to 52: make the result list (`List<wrapper>` with capacity `ebx`).
- Lines 53 to 54: `r12` is the chart; null exits.
- Lines 55 to 56: `xmm0 = [chart+0xb8] * 0.03125`. `[chart+0xb8]` is the chart's resolution as a double: the engine constructor reads the same field at `engine_ctor_20DF680.asm` line 57 and turns it into `resolution << 2`. 0.03125 is 1/32 (the listing's own comment names the constant).
- Lines 61 to 62: `cvttsd2si rdi, xmm0` truncates toward zero, then `inc rdi`. So the slop is `trunc(resolution / 32) + 1` ticks. Line 63 saves it.
- Lines 64 to 74: start walking the track's fill list with `List.GetEnumerator`.
- Lines 78 to 82: `MoveNext`; when it returns false jump to the clean-up at line 148.
- Lines 83 to 87: `rsi` = the current fill. Line 86 to 87: `cmp byte [fill+0x20], 0; jne` skips a fill whose byte `+0x20` is non-zero (the record calls this the skip flag; what sets it is not in this function).
- Lines 88 to 96: the call to `0x5DE030`. Arguments: notes list (`r14`), `rdx = [fill+0x18] + [fill+0x10]` = fill end (the fill's tick plus its length, so `+0x10` is the start and `+0x18` the length), `r8 = [fill+0x10]` = fill start, `r9 = end + slop`, then a stack integer 0 (the start index, line 93) and the generic method info (line 92). So the nearest-chord search always starts at index 0 for every written fill.
- Lines 97 to 98: a null result (no chord) skips the fill: the loop continues.
- Lines 99 to 105: call `0x20D51D0(chord, -1, 0)` which returns the note that will carry the flag (`docs/audit/drum-lanes/FINDINGS-play-time.md` line 50 says it is the highest-pad note of the chord; that was not re-read here). A null note continues the loop.
- Line 106: `or dword [note+0x7c], 0x800`: the activation flag. This is the instruction the drum-lanes findings call 0x20D015E.
- Lines 107 to 119: make a wrapper object; line 116 converts the fill start tick to seconds with `0x212B9C0`, stored at wrapper `+0x10` (line 119).
- Lines 120 to 128: wrapper `+0x18` = a float from `0x20CFCE0(chart+0x50 object, fill start, note.[+0x40], ...)`.
- Lines 129 to 132: wrapper `+0x20` = the note's chord (the object that got the flag).
- Lines 133 to 134: wrapper `+0x28` = `[fill+0x10]`, the fill's start tick. The deadline method compares this field.
- Lines 135 to 138: wrapper `+0x30` = chord tick (`[chord+0x40]`) minus fill start. It may be zero or negative.
- Line 139: wrapper `+0x38` = 1 (active).
- Lines 140 to 147: add the wrapper to the result list; next fill.
- Lines 148 to 169: dispose the enumerator, return the result list. Lines 170 to 178: null-reference and range exits.

Who calls it: only `0x20D2EA0` (the chart builder) at 0x20D3381, taken when the track's fill list `[track+0x88]` has a count different from zero (`f20D2EA0.asm` lines 281 to 289, with `r13d` zeroed at line 134).
