# plain_drums_20EFEE0.asm (build 6142, RVA `0x20EFEE0`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (class `Image`, whole function, 35 lines). The listing starts at the function's first instruction (script.json says `0x20efee0` is a method start: `#8278.ʷʽʶʽˁʸʻʵʲʴʶ`, static, parameters `List<note>` and an `int8`).

Plain words: for every note in the list it clears two flag bits. Nothing is added, removed or moved.

- Lines 1 to 3: set up the stack. Line 5 moves the note list (`rcx`) into `rdi`.
- Lines 4 to 11: the first-call setup every IL2CPP method does (it loads two generic-method slots once, then sets a flag byte). It has no effect on the notes.
- Lines 12 to 15: counter `ebx` = 0. If the list is null (line 14 to 15) the method jumps to the null-reference exit at line 33.
- Lines 17 to 18: loop test. `cmp eax, [rdi+0x18]` compares the counter with the list's count; when the counter reaches the count it jumps to the exit at line 29.
- Lines 19 to 24: fetch note number `ebx` (a call to `List.get_Item`). A null note jumps to the null-reference exit (line 33).
- Line 25: the one write. `and dword ptr [note+0x7c], 0xfff3ffff` keeps every bit of the note's flag word except bits 18 and 19 (values 0x40000 and 0x80000). Those are the two bits the hit handler `0x20DE770` tests through the getters `0x214B6C0` (0x40000) and `0x214BE10` (0x80000), see `docs/audit/drum-lanes/f20de770.asm` lines 64 and 69, where the first sets the counter at engine `+0xD8` and the second the one at `+0xDC`.
- Lines 26 to 28: next note, loop.
- Lines 29 to 32: normal return. Line 33 is the null-reference exit.

The `int8` second parameter is never read in this listing (no use of `dl`/`sil`/`edx` as a value; line 20 reuses `edx` as the index argument).

Who calls it: only `0x20D2EA0` (the chart builder), at `0x20D3301` (found by scanning the `il2cpp` section for call and jmp instructions to this RVA, 2026-10-30), and only on the branch where the instrument byte is not 9 (`docs/audit/note-shuffle/step0/open2-refute/f20D2EA0.asm` lines 245 to 253: `cmp sil, 9; jne` to the call at `0x20D32F2` of `0x20F16E0`, then this call).
