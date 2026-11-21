# The clean records, judged (session f9b4f64e, 2026-11-15)

The 397 clean Investigate records (`../CLEAN.md`) were each open for one reason: the test claims Hydra does something, and nobody had checked the claim. This folder holds the verdicts.

Two passes made them. First, three re-judges read the capture runs (`../rejudge/`). They found 13 records shown by a run and most of the rest not shown, because the runs had been planned by group, not by claim. Then the user said: "just proceed without captures for the remaining and use your best judgement with static code analysis". So nine fresh scouts (Sonnet, brief `../../briefs/T5c-by-reading.md`) took every record still open, in batches B1 to B10 (B7 was merged into B5). Each scout read the test, traced the test's own input through the production code, and used any capture output that bore on the claim. Each `B<n>-records.md` is the batch's rows from CLEAN.md section 3; each `B<n>-verdicts.md` is its table and side notes.

A verdict is about the claim only. It is not an exit: only the main session picks an exit (`docs/agents/audit-method.md`), in T8.

## Totals

Counted from the table rows of the verdict files. CLEAN.md section 3 has 385 rows for its 397 records (some rows hold several items); a few records are judged on two rows because their test is listed twice.

| Verdict | Rows | Where |
|---|---|---|
| Shown by a capture run | 13 | `../rejudge/J1-engine.md` 7, `J2-R4a.md` 3, `J4-store-hooks.md` 3 |
| Shown by a capture run (by-reading pass) | 21 | B2 19, B1 1, B3 1 |
| Holds by reading | 333 | B1 37, B2 26, B3 47, B4 43, B5 35, B6 37, B8 35, B9 35, B10 38 |
| Contradicted | 0 | none |
| Cannot tell | 20 | listed below |

No scout found a test whose claim the code contradicts. "Holds by reading" means the scout traced the test's input to the claimed result in the code; no run proves it. The main session checked four of these rows against the code (B1 A17 17, B4 C9 1.5, B9 test_a_broken_transcript_line_fails_loudly, and the R4b output B1 cites); all four held.

## Cannot tell (20)

Each needs a run, a file outside the repo, or the live game. The verdict files say what would settle each.

- Needs the test or a run: A12 10 (the docs check), A16 22 (prints a timing only when `HYDRA_TIMING_DB` is set), A20 14 (the 1.0-rule search on a built song), A31 SR1 and SR14 (the five decoders and libopus on a damaged packet), A38 11 (`expect-text` refusing), A44 7, A45 7 and A45 16 (pixel widths and wrapping), WN3 10 and C11 "CL folded variant Clamped to SqIn" (search outcomes at cap 3), DL1 456 (SQLite's checkpointing).
- Needs files outside the repo: D3 `test_frida_tools.py:588` and `:624`, D8 `test_method_watch.py:422`, `:426`, `:435` and `:453` (the real `dump.cs` and `facts.json`).
- Needs the live game and a person watching: D16 `key_delivery_test.py` and `pad_flash_test.py` (scripts with no assertion).

## Side notes that need an exit in T8

The verdict files hold every side note. These are the ones that say a test cannot do its job, beyond the ruled faults the recount already counted:

- **A test that cannot fail.** `tools/test_effort_test_faults.py:204`: no input can make a removed line look like a diff header, so the test passes whatever the code does (B9).
- **A title that promises what the code does not do.** `tests/test_precheck_git.cpp` near 297 (C8 4.10) says the code mask keeps every line break; it keeps `\n` and blanks the `\r` of `\r\n`, and the expected string relies on that (B3).
- **Tests that check no Hydra code.** A11 4 and 6 call only helpers in the test file (B1); C4 1 tests a CMake reader that lives in the test file (B3); D5 `test_game_tools.py:3169` and `:3174` test Python and Windows behaviour (B6); D12's nine PC and PS rows never give the chart to the engine (B5).
- **Passes that need nothing.** E3 RJ:147, 232 and 285 would pass with no gate installed (B5). D11 99 would pass if `presses[1]` had no `end_read_after` (B5).
- **Not real inputs where the record asked for real ones.** B8's whole batch builds hand-written handoffs and repos; DL2 1 builds old files from typed CREATE TABLE text; `make_sng` writes no file-data length field, which a real `.sng` may hold (unchecked, B3).
- **Misfiled or stale records.** The DL3 click records filed as batch screens (index.md); A7 #43 and #47 filed under the Path report; D10 63 said to need a live game; A19 PL13 filed under the Path Builder; many stale line numbers (B1, B2, B6).

## Rule slip

The B6 scout ran its batch's Python tests (98, 75 and 295 passed) though its brief named none. They need no build and change nothing, and its verdicts are by reading, but the brief preamble says an agent runs only the tests its brief names.
