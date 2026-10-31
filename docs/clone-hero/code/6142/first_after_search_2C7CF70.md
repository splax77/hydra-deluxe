# first_after_search_2C7CF70.asm (build 6142, RVA `0x2C7CF70`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` with `maxbytes 0x200` (84 lines; `0x2C7CF70` is past the last method address in script.json, `0x2BF4C30`, so the listing stops at the padding the tool finds, not at a next-method address). Support listing for `0x5DE030` and `0x5DDB50`. It is a generic helper (`ʿʲˁʽʹʺʶʳˁʾʼ`-family, shared by every generic instantiation).

Plain words: walk a list forward, starting at index `r8d`, and return the index of the first element whose tick is strictly greater than the target. Return -1 when the list runs out.

- Lines 1 to 17: set-up. `rsi = target` (`rdx`), `edi = start index` (`r8d`), `rbp = list` (`rcx`).
- Lines 18 to 32: build a forward range enumerator over the list from `start index` to 0x7fffffff (line 23, clamped to the list's last index by the constructor `0x5FBC10`; its `MoveNext` at `0xFD0660` increments the index and continues while it is at most the end index).
- Lines 47 to 52: `MoveNext`; false jumps to the "not found" return at lines 74 to 76 (`-1`).
- Lines 53 to 64: fetch the element and its tick through the virtual call 0x182C44210.
- Lines 65 to 66: `cmp rax, rsi; jle` loops while the tick is less than or equal to the target.
- Lines 67 to 73: otherwise return the current index (`edi`).
