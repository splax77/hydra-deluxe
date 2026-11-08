# note_after_lookup_5DD8D0.asm (build 6142, RVA `0x5DD8D0`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (82 lines, 0x12D bytes, ending at the int3 padding after the null-reference exit). Starts at the method's first instruction. It is a generic helper on `ChordListSearch` (`#8670`), our name `NoteAfter<object>`; `dump.cs` lines 552885 to 552889 give its shape: static, `(List<T> list, long target, int startIndex = 0)`, returning an element. The per-note window routine `0x20F3A90` calls it at `note_window_20F3A90.asm` line 29, with the note list at `this+0x1e0` as the list, the note's tick (`note+0x40`) as the target and the note's index (`note+0x20`) as the start.

Plain words: walk the list from the start index and return the first element whose tick is strictly later than the target, or nothing when the list runs out. An element at the target's own tick is skipped, so another gem at the same tick is never returned.

- Lines 1 to 10: save registers. `rdi` = the method info (`r9`), `ebx` = startIndex (`r8d`), `rsi` = target (`rdx`), `rbp` = list (`rcx`).
- Lines 11 to 17: make sure the generic slots are loaded.
- Lines 18 to 32: build a walker over the list from startIndex, with 2147483647 (the largest int) as the end (line 23), by the `ListRange` call at line 32.
- Lines 33 to 45: copy the walker into the loop's state at `rsp+0x30`.
- Lines 47 to 52: the loop: step the walker (call at line 50); when it has no next element, go to line 73.
- Lines 53 to 59: fetch the current element into `rbx`; a null element goes to the throw at line 80.
- Lines 60 to 63: read the element's tick with the virtual call at 0x182C44210 (the same tick read as `nearest_chord_5DE030.md` line 16).
- Lines 64 and 65: `cmp rax, rsi; jle` back to line 47: while the element's tick is at or before the target, keep walking.
- Lines 66 to 72: the element's tick is later than the target: return it.
- Lines 73 to 79: the walk ran out: return null. The half-gap clamp then gives the back value for that side (TM-19 on page 3).
- Lines 80 to 82: null-reference throw and padding.

The walk direction is not read from the walker itself; it rests on the default start of 0 in the dump's signature and on the stop test, which only makes sense walking up a list sorted by tick.
