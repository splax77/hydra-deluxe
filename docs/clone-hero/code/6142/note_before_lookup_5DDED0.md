# note_before_lookup_5DDED0.asm (build 6142, RVA `0x5DDED0`)

Made 2026-11-08 with `tools/ch_mcp/chdis.py` (95 lines, 0x160 bytes, ending at the int3 after the null-reference exit; the next instruction, at 0x5DE030, starts `nearest_chord_5DE030.asm`). Starts at the method's first instruction. It is a generic helper on `ChordListSearch` (`#8670`), our name `NoteBefore<object>`; `dump.cs` lines 552894 to 552898 give its shape: static, `(List<T> list, long target, int startIndex = 2147483647, bool inclusive = false)`, returning an element. The per-note window routine `0x20F3A90` calls it at `note_window_20F3A90.asm` line 37, with the note list at `this+0x1e0`, the note's tick (`note+0x40`) as the target, the note's index (`note+0x20`) as the start and `inclusive` false (`xor r9d, r9d` at line 31 of that listing, address 020f3b24).

Plain words: walk the list down from the start index and return the first element whose tick is strictly earlier than the target, or nothing when the list runs out. An element at the target's own tick is returned only when `inclusive` is true. The window routine passes false, so for it another gem at the same tick is never returned.

- Lines 1 to 11: save registers. Line 7 reads the method info from the stack into `rdi`. `ebx` = startIndex (`r8d`), `ebp` = the `inclusive` byte (`movzx ebp, r9b`, line 9), `rsi` = target (`rdx`), `r14` = list (`rcx`).
- Lines 12 to 19: make sure the generic slots are loaded.
- Lines 20 to 34: build a walker over the list from startIndex, with 0 as the other end (`xor r9d, r9d`, line 25), by the `ListRange` call at line 34.
- Lines 35 to 47: copy the walker into the loop's state at `rsp+0x30`.
- Lines 50 to 55: the loop: step the walker (call at line 53); when it has no next element, go to line 86.
- Lines 56 to 62: fetch the current element into `rbx`; a null element goes to the throw at line 94.
- Lines 63 to 68: read the element's tick (virtual call at 0x182C44210, line 66); `cmp rax, rsi; jl` to line 78: an element strictly earlier than the target is returned.
- Lines 69 to 77: read the tick again (line 72); `edx` starts 0 (line 73) and becomes the `inclusive` byte only when the tick is at or before the target (`cmovle edx, ebp`, line 75). With `edx` 0 the loop goes on (line 77 `je` to line 50). So an element at the target's tick is returned only when `inclusive` is true, and an element later than the target is always passed over.
- Lines 78 to 85: return the element.
- Lines 86 to 93: the walk ran out: return null. The half-gap clamp then gives the back value for that side (TM-19 on page 3).
- Lines 94 and 95: null-reference throw and padding.

The walk direction is not read from the walker itself; it rests on the default start of 2147483647 in the dump's signature (the end of the list) and on the stop test, which only makes sense walking down a list sorted by tick.
