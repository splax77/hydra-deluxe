# Outlined add-fill copy, no dump name, RVA 0x20CEDC0 (build 6142)

Listing: `mid_fill_reader_20CEDC0.asm`, 71 lines, starts at the first instruction I could identify (`script.json` and `dump.cs` have no method at this address; it is a compiler-outlined copy, so its start is Unknown by the table rule, but the code begins with a normal prologue and both callers target this address exactly). Made with `tools/ch_mcp/chdis.py`, 2026-10-30. Callers (my scan): 0x215536E inside `0x2155050` and 0x2156E5E inside `0x2156990`, both in the static class `#8626`.

What it is: the same add-fill instructions as `add_fill_215BD50.asm`. It takes the track from `rcx` (the pool is at `[track+0x30]`, the list at `[track+0x88]`), the tick, the length and the flag, makes a fill with `0x20FB9A0` (line 17 of this listing, `call 0x1820fb9a0` at 0x20CEE3D) and appends it.

How the `.mid` reader uses it: at `0x2155346-0x215536E` (in `0x2155050`) the reader compares the event byte `[rsi+0x28]` with `0x74` and then with `0x78` (decimal 120, the MIDI note the survey and ADR 0023 call the fill note). On 120 it calls this helper with the track `r13`, the start tick `r14`, the length `rdi - r14` (end minus start) and the flag 0. Which track `r13` is for each difficulty was not followed here (see FL-29 on page 4).
