# note_build_20D4BE0.asm (build 6142, RVA `0x20D4BE0`)

Made 2026-11-23 with `tools/ch_mcp/chdis.py` (474 lines). Starts at the method's first instruction. `dump.cs` names it as a static method of the static class #8265 (scrambled name) that takes the chart (class #8538) and returns a `List` of engine notes (class #8597). A byte scan of call instructions finds it is the only caller of the engine note's full constructor `0x214BF10`.

Only lines 237 to 267 are annotated here: where each engine note's two times come from.

- Lines 240 to 244: the note's start time is the chart's tick-to-time conversion (`tick_to_time_212B9C0`) of the chart note's tick at `+0x10`; kept in xmm7.
- Lines 245 to 252: the end tick from `0x2109730`, through the same conversion; kept in xmm6.
- Lines 253 to 267: a new engine note is made and its constructor called with the start time in xmm1 and the end time in xmm2; `note_ctor_214BF10.asm` stores the first at `+0x28` and the second at `+0x30` (its lines 27 and 15).

So the note time that the hit window's half-gap reads (`half_gap_clamp_20F7210.md`, note+0x28) is the file's time divided by the song speed. What `0x2109730` returns (the end tick) was not read.
