# fold_20F16E0.asm (build 6142, RVA `0x20F16E0`)

Made 2026-10-30 with `tools/ch_mcp/chdis.py` (45 lines). Starts at the method's first instruction (`#8278.FoldCymbalsToToms`, static, one `List<note>` parameter).

Plain words: for every note whose "colour" word has any of the three top bits of its low byte set, shift the whole 16-bit word right by 3. The shift moves the cymbal bits down onto the tom bits, so a cymbal becomes the tom of the same lane. Notes with none of those bits are left alone.

- Lines 1 to 11: stack set-up and the one-time generic-method slot load, same as `plain_drums_20EFEE0`.
- Lines 12 to 15: counter `ebx` = 0; null list jumps to the null exit at line 43.
- Lines 17 to 18: loop until the counter reaches the list count.
- Lines 19 to 24: fetch note `ebx`; a null note jumps to line 43.
- Lines 25 to 26: `test byte ptr [note+0x80], 0xe0; je` skips the note when bits 5, 6 and 7 of that byte are all clear. These three bits are the cymbal markers (the getter `0x214BE30` tests exactly `0xE0` on the word at `+0x80`, `docs/audit/note-shuffle/scout-flag/note_getters.txt` line 32).
- Lines 27 to 32: fetch the same note again.
- Lines 33 to 35: `movzx ecx, word [note+0x80]; shr cx, 3; mov word [note+0x80], cx`. This is the fold.
- Lines 36 to 38: next note, loop. Lines 39 to 42: return. Line 43: null exit.

Who calls it: only `0x20D2EA0`, at 0x20D32F2, on the branch where the instrument byte is not 9 (`f20D2EA0.asm` lines 245 to 251). On ProDrums (byte 9) `0x20F1420` runs instead.
