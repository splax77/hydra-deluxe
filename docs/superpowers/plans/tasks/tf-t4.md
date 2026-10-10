Read docs/superpowers/plans/tasks/_tf-preamble.md first; it holds the rules.

# Task TF-T4: pinned literals instead of recomputed oracles

Task id: `tf-t4`. Worktree `.claude\worktrees\tf-t4`; make it. Your spec is the plan's "Task 4", plus one extra item below.

## From the plan

`tests/test_song.cpp` (the regex oracles, about lines 596 to 730 on main): the two disco cases read their expected answer from a table of byte ranges per marker shape, and the section-header case from a three-column table. Make the tables with a throwaway script in your scratch folder, run against the two regexes, then delete the script (never commit it). The 256-byte parse loop stays. The `std::regex` oracles and the `<regex>` include (if nothing else uses it) go.

`tests/test_fill_deadline.cpp`: replace the `3750.0 / bpm` check with a table of five tempos and their lead, pinned from one run of today's code (confirm the plan's numbers by running, not arithmetic). The two formula lines in the clamp cases go; their literals already pin the answer. The header's "worked out from the formula" sentence changes to say the literals came from one run.

## Extra item: `tests/test_sng.cpp`

The case "sng: unmasking into the caller's buffer matches the byte formula" (about line 110) recomputes the XOR unmask formula per byte and compares the decoder against it. Decide which it is. If the formula is the published `.sng` format spec (check `docs/` for the format write-up, and whether `src/` has one owner of the formula the test could call instead), it may stay as a spec check: say so with the reason, and change nothing. Otherwise replace the recomputation with pinned bytes from one run (for example the first 32 and the last 16 unmasked bytes as a hex literal), keeping the 1,000-byte payload and the mid-block end.

## Owned files

`tests/test_song.cpp` (the oracle region only; the malformed-line case at about line 731 and everything else stay), `tests/test_fill_deadline.cpp`, `tests/test_sng.cpp` (that one case only). If a `tests/test_single_owner.cpp` row names the regexes, stop and report the row instead of editing it.

## Failing first

Flip one table entry in each file, show red, flip back.

## Tests

`-sf=*test_song.cpp*`, `-sf=*test_fill_deadline*`, `-sf=*test_sng*`.
