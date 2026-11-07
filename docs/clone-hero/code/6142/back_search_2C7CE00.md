# back_search_2C7CE00.asm (build 6142, RVA 0x2C7CE00)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` with `maxbytes 0x200`. The listing runs 134 lines because the tool did not find a padding break before the next function: lines 1 to 98 are this function and lines 99 to 134 are the start of 0x2C7CF70 (see `first_after_search_2C7CF70.asm`). Support listing for `0x5DE030` and `0x5DDB50`.

Plain words: walk a list BACKWARD, starting at index `r8d` and ending at index 0, and return the index of the first element met whose tick is below the target, or equal to it when the flag is true. Return -1 when the walk ends without a match.

- Lines 1 to 19: set-up. `rsi = target`, `ebx = start index` (`r8d`), `r14d = flag` (`r9b`), `rbp = list`.
- Lines 20 to 34: build the range enumerator with start `ebx` and end 0 (`r9d` is cleared at line 25). The constructor `0x5FB640` clamps the start to the list's last index; the enumerator's `MoveNext` at `0xFEC570` decrements the index and continues while it is at least the end index.
- Lines 50 to 55: `MoveNext`; false returns -1 (line 88, `ebp = 0xffffffff`).
- Lines 56 to 68: fetch the element and its tick.
- Lines 69 to 70: `cmp rax, rsi; jl` returns the index when the tick is below the target (line 89).
- Lines 71 to 79: otherwise `edx = flag if tick <= target else 0`; a non-zero `edx` returns the index (line 80). With the tick not below the target, "at most" means equal, so the flag makes an element exactly on the target count.
- Line 79: `je` back to line 50 continues the walk toward index 0.
