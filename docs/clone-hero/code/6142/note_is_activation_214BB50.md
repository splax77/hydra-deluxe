# Is this note an activation note, RVA 0x214BB50

Listing: `note_is_activation_214BB50.asm`, build 6142, made 2026-10-30 with `tools/ch_mcp/chdis.py`. `rcx` is a note object. The listing starts at the function's first instruction; whether the RVA is the start of a named method was not checked in `script.json` (the capstone listing shows a three-instruction function followed by padding).

- Line 1: test the note's flag word at +0x7c against 0x800.
- Line 2: `seta al`: the result is true when the flag bit is set.
- Line 3: return.

The drum hit handlers call this to decide whether the hit note starts Star Power (see SP-17 on page 5).
