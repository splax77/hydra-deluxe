# Track lookup, no dump name, RVA 0x20CFD40 (build 6142)

`script.json` has no method at this address and `dump.cs` has no `RVA: 0x20CFD40` line, so this is a compiler-outlined helper, not a named method. The listing `track_lookup_20CFD40.asm` (96 lines) starts at the first instruction I could identify as the helper's entry (it has a normal prologue and 20 direct call sites in the DLL target it, among them the builder at 0x20D2FBC and several in the 0x2151000-0x2158000 range; I did not identify the owner of each). Made with `tools/ch_mcp/chdis.py`, 2026-10-30. Label for any rule resting on its exact start: Unknown, because no table names this address as a function start.

What it does, line by line. Arguments: the chart (`rcx`), the instrument (`dl`), the difficulty (`r8b`).

Lines 1-21: prologue and one-time metadata set-up.

Line 23-24: a difficulty of `0xFF` jumps to line 77.

Lines 25-36: index = instrument x (a number held in the static data of the chart class `#8538`, at `+0x60`) + difficulty. -1 jumps to line 77.

Lines 37-44: read the chart's list of tracks (`[chart+0xd0]`, a `List<#8661>`) and fetch the slot.

Lines 45-64: if the slot holds nothing, take a track object from the pool at `[chart+0x20]`, initialise it with 0x18215E6E0 (the track's init method) and store it in the slot.

Lines 65-76: fetch the slot again and return it.

Lines 77-93: build and throw an `ArgumentException` with the string "Invalid difficulty" (line 81).
