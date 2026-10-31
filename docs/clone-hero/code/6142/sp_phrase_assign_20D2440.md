# sp_phrase_assign_20D2440 (build 6142, made 2026-10-30 with tools/ch_mcp/chdis.py)

Function: `#8265.AssignStarPowerPhrases` (obfuscated static method on `#8265`, RVA `0x20D2440`; `script.json` and `dump.cs` line 528339 name it; the first instruction is the function start). It takes the track's note list and the track's Star Power phrase list and marks which notes are in a phrase and which note ends one. Class names: note = `#8597` (`dump.cs` line 548368), phrase = `#8252` (`dump.cs` line 527218). Note tick is the `long` at note+0x40, flags are the `int` at note+0x7c; phrase start is the `long` at phrase+0x10 and phrase length the `long` at phrase+0x18.

Lines 1 to 14: prologue and the one-time "first call" setup of generic method pointers. Nothing about phrases.

Lines 15 to 39: a phrase-list cursor is built on the stack. It holds an index (at rbp+0x1f, starts 0), the list (rbp+0x27), the current phrase start (rbp+0x2f) and current phrase end (rbp+0x37). Both start at 0.

Lines 40 to 57: if the phrase list exists and index 0 is in range, the first phrase is read: start = phrase+0x10, end = start + phrase+0x18 (line 54 to 57). With no phrase the start and end stay 0.

Lines 58 to 63: the note list must exist; its count is read (line 60) and the loop counter edi starts at 0 (line 61). An empty list skips to the exit (line 63).

Lines 64 to 71: the note at index edi is fetched (call at line 68). A null note jumps to the null-reference throw at line 145.

Lines 72 to 74: if the note's tick is less than the current phrase start (`cmp [rax+0x40], start; jl`), the note is before the phrase and the loop goes to the next note (line 134). So the start edge is inclusive.

Lines 75 to 84: the next note (index edi+1) is fetched into rsi, or rsi is 0 when edi is the last note.

Lines 85 to 91: if the note's tick is at or past the current phrase end, the cursor moves on by one phrase: the index is incremented (line 88) and `0x20D0330` reloads start and end (line 91). This is one step per note, an `if` and not a loop. Line 92 reloads the end after the step.

Lines 93 to 97: re-test after the step. If the note is before the (possibly new) start, or at or past the (possibly new) end, the note is skipped (line 95 and line 97 both jump to line 134). So the end edge is exclusive: a note exactly on the end tick is outside, and a phrase of length 0 matches no note.

Line 98: the note is inside the phrase: flag 0x80 is set on the note (`or [note+0x7c], 0x80`).

Lines 99 to 103: if a next note exists and its tick is before the phrase end, this is not the last note of the phrase and the loop moves on (line 103 jumps to line 134).

Lines 104 to 133: this is the last note of the phrase (no next note, or the next note's tick is at or past the end). The note's chord (the enumerator of `#8597`, calls at lines 107 and 122) is walked, and flag 0x100 is set on every note of the chord (line 128). Flag 0x100 is what the score code reads as "this chord awards the phrase".

Lines 134 to 136: next note; loop until the list is done.

Lines 137 to 144: epilogue.

Lines 145 to 146: null-reference throw.
