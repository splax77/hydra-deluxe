# Side notes from the scouts (not duplication questions)

Things a scout noticed in passing that are not part of this audit's question. They are unchecked claims. Each goes to the user at the end of phase 5 as a possible follow-up, after someone checks it in the code.

- G8: `Playhead::read_frames` returns `n`, not the frames its source really gave, while its header says it returns the real frames written.
- G8: several headers under `src/audio/` say "throws std::runtime_error" where the code throws a `KindedError`.
- G8 (also a G2-8 note): `MappedFile` opens with `FILE_SHARE_READ` only, while `file_byte_source` also shares write. The scout reads this as a stem that another program holds open for writing silently dropping from the Preview. Untested. Checked 2026-10-11: it holds, and it only happens while another program is still writing the file (a download, a sync, an editor's export), when the file is incomplete anyway. The user said leave it: no notice (2026-10-11).
- Process: most scouts got their agent id from the progress hook late (C2, G6b, G9) or never (G1b, G4b, G8, G10), so early status lines went under a guessed name. `docs/agents/brief-preamble.md` promises the id on the first reply. The hook or the preamble is wrong about when it arrives.
- G7a: in `src/parse/note_shuffle.cpp` around line 142, `prev_tick` starts at 0. The scout reads this as making a first non-kick note at tick 0 count as being in an earlier chord. It did not check whether the game's own shuffle code does the same.
