# Side notes from the scouts (not duplication questions)

Things a scout noticed in passing that are not part of this audit's question. They are unchecked claims. Each goes to the user at the end of phase 5 as a possible follow-up, after someone checks it in the code.

- G7a: in `src/parse/note_shuffle.cpp` around line 142, `prev_tick` starts at 0. The scout reads this as making a first non-kick note at tick 0 count as being in an earlier chord. It did not check whether the game's own shuffle code does the same.
