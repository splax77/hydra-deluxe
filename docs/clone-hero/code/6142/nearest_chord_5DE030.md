# nearest_chord_5DE030.asm (build 6142, RVA `0x5DE030`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (122 lines). Starts at the method's first instruction (script.json: a generic method on `#8597`-family helper class `ChordListSearch`, name `NearestChordInRange<object>`; static, `(List<chord> list, long target, long low, long high, int startIndex, MethodInfo)`).

Plain words: given a sorted chord list, a target tick, a lower bound and an upper bound, return the chord that is nearest the target, or null. The "after" candidate is the first chord strictly after the target. The "before" candidate is the nearest chord at or before the target. A candidate outside [low, high] is thrown away, and when both survive the nearer wins, with a tie going to the "after" chord.

- Lines 1 to 13: save registers. `r12 = low` (`r8`), `r15 = high` (`r9`), `r13 = target` (`rdx`), `rsi = list` (`rcx`). Line 9 reads the method-info argument from the stack.
- Lines 14 to 27: make sure the generic-slot is loaded; set up the call.
- Line 28: call `0x2C7CF70(list, target, startIndex, method)`. That helper walks forward from `startIndex` and returns the index of the first chord whose tick is greater than `target` (`first_after_search_2C7CF70.asm` lines 46 to 66), or -1. Call it "A".
- Lines 29 to 32: `ebp = A`. `r8d = A`, except that when A is -1 `r8d` becomes `startIndex` (line 32, `cmove`).
- Lines 33 to 39: call `0x2C7CE00(list, target, r8d, true, method)`. That helper walks backward from index `r8d` down to index 0 and returns the first chord whose tick is below the target, or equal to it when the flag is true (`back_search_2C7CE00.asm` lines 50 to 79). Call it "B".
- Lines 40 to 43: if B is -1 and A is also -1, return null (line 118). If B is -1 and A is not, fall to lines 44 to 60.
- Lines 44 to 60: B is -1, A is not: return the chord at A with no window check at all. So a fill that has no chord at or before its end takes the first chord after it however far away (this is the "unbounded" fallback in ADR 0023 line 84 to 87).
- Lines 61 to 67: B is not -1: fetch the chord at B.
- Lines 68 to 69: if A is -1, return the chord at B with no window check (line 69 `je` to the return at line 51). Note that in that case B was searched backward from `startIndex`, not from the end of the list, so it is the nearest chord at or below `startIndex`, which is chord 0 when `startIndex` is 0 (see FL-07 on page 4).
- Lines 70 to 89: both exist: fetch both chords and read their ticks with the virtual call at 0x182C44210 (lines 82 and 89).
- Lines 90 to 95: `high < tick(A)` (A is past the upper bound) and `low > tick(B)` (B is before the lower bound): if both, return null (line 118).
- Lines 96 to 97: B is before `low`: return A (line 116). A is not checked against `high` here because the previous test already ruled out "both bad".
- Lines 98 to 99: A is past `high`: return B (line 114).
- Lines 100 to 113: both are in bounds: `rbx = tick(A) - target`, `rcx = target - tick(B)`; `cmp rbx, rcx; jle` returns A when `tick(A) - target <= target - tick(B)` (line 113), else B. A tie therefore goes to A, the later chord.
- Lines 114 to 117: the two return paths. Line 118 to 119: return null. Lines 120 to 121: null-reference exit.

Who calls it: only `0x20CFF60` at 0x20D013C (callers scan of the `il2cpp` section, 2026-10-30).
