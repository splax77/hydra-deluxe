# Drum lanes: the game-code listings behind D112

These are disassembly listings of Clone Hero 1.1.0.6142's `GameAssembly.dll`, made with the Note Shuffle helpers (`docs/audit/note-shuffle/scout-strings/chd.py`) on 2026-10-10. Each file is named after the function's address (an RVA). D112 in `docs/audit/2026-10-03-fix-decisions.md` is the decision they back.

`spec_213e0c0.asm` is the .chart reader's special-phrase handling, where `S 65` and `S 66` become lanes. `lane_ctor_2116390.asm` is the lane object's constructor (start, length, difficulty, kind). `f20d12c0.asm` builds each lane's pads from the notes under it; `f20d5310.asm` takes a pad mask's lowest pad and `f20d51d0.asm` its highest. `f20dcad0.asm`, `f20de420.asm` and `f20de770.asm` are the drum engine reading the lane flags during play. The rest are helpers those call.

Two scripts from the same day check the facts against `testdata`. `midlanes.py <notes.mid>` prints a .mid's lane notes (pitch 126 or 127) and the Expert pads after the first one. `lanediff.py <folder>` lists every lane where the game's pad rule and Onyx's differ (4 of 50 in `testdata`). Run them with `py -I`.

The listing of the .mid reader's lane case (around RVA 0x2156990) was not kept.
