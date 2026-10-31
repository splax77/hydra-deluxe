# nearest_chord_loop_5DDB50.asm (build 6142, RVA `0x5DDB50`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (104 lines). Support listing: the generator `0x20D2B30` calls it (line 103 of that listing). Starts at the method's first instruction (a generic helper on `ʽʹʽʽˁʹʷʺˀʺˁ`, name `ʶʲˁʿʹʵˀʺʲˁʿ<object>`, static: `(List<chord> list, long target, int startIndex, MethodInfo)`, returns an index).

Plain words: the same nearest-chord search as `0x5DE030` but with no window and returning an index. A is the first chord strictly after the target, searched forward from `startIndex`. B is the nearest chord at or before the target, searched backward from A (or from `startIndex` when there is no A). If only one exists it wins. If both exist, B wins only when the target is strictly nearer to B than to A, so a tie goes to A, the later chord.

- Lines 1 to 23: set-up; `r14d = startIndex` (`r8d`), `rbp = target` (`rdx`).
- Lines 24 and 25: call `0x2C7CF70(list, target, startIndex)`: index A or -1.
- Lines 26 to 29: `ebx = A`; `r8d = A`, or `startIndex` when A is -1 (`cmove`, line 29).
- Lines 30 to 36: call `0x2C7CE00(list, target, r8d, true)`: index B or -1.
- Lines 37 to 42: B is -1 and A is -1: return -1 (line 42). B is -1 and A is not: return A (line 41 `jne` to line 84 `mov eax, ebx`).
- Lines 51 and 52: A is -1 and B is not: return B (line 52 `je` to line 93 `mov eax, r15d`).
- Lines 53 to 78: fetch both chords and their ticks (virtual call 0x182C44210, lines 71 and 78).
- Lines 79 to 84: `(tick(A) - target) > (target - tick(B))` returns B (line 93), else A (line 84).

Note for the claims table: the backward search starts at `startIndex` when there is no A, so with no chord after the target it returns the chord at or below `startIndex`, which is not necessarily the last chord of the song.
