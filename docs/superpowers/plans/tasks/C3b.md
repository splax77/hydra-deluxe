Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task C3b: the typed no-notes error and the state dump's status words

Task id `C3b`. Base `dc7e582` (main). Branch `claude/p3-c3b`, worktree `.claude\worktrees\p3-c3b`.

Plan: `docs/superpowers/plans/2026-10-04-phases-3-5.md`, wave C row C3b. Handoff: `docs/handoffs/2026-10-04-phase3-handoff.md`. Evidence: `docs/audit/2026-10-04-session-audit/step3a.md` (finding 71, the paragraph "71, the no-notes sentence") and `step3b.md` (finding 87, "Status words"). Read them with offset and limit.

## Goal

Two sentences each exist in two places today. The "no notes" error has one owner for its wording but a second, different sentence typed in the search; after this task one typed error carries the owner's sentence and the stray sentence is gone. The library's three status words ("Analyzed", "Stale", "Not analyzed") are spelled out in the UI, while `hydra_uitest state` prints its own words ("current", "stale", "new"); after this task one function owns the words and the dump prints the chip words. Both are code-only: nothing the GUI shows changes, no score, path or record moves, and the results stamp stays "2.1.0".

## Finding 71: the no-notes sentence

**Today.** `no_notes_message(difficulty, prodrums)` in `src/parse/song.cpp` is the owner of the sentence ("No Hard Pro Drums notes in this chart."). `analyze_chart_file` in `src/app/analysis.cpp` loads the song with `load_songpath`, then throws a plain `ChartFileError` with that sentence when the song is empty. `PreviewLoadJob` in `src/ui/preview_load_job.cpp` does the same one-line check on the song `resolve_preview_song` hands it. `analyze_chart` in `src/search/pather.cpp` has its own backstop that throws `ChartFileError("No drum notes in this chart.")`, a second sentence. `is_no_notes_message` in `src/app/user_messages.cpp` recognises the owner's sentence by its shape ("No " ... " notes in this chart.") so `plain_error_text` passes it through unchanged.

**Owner.** A typed no-notes error in `src/parse/song.h`/`.cpp`, a `ChartFileError` subclass, whose message is `no_notes_message`'s sentence, plus the load-and-check entry point beside `load_songpath` that loads a chart and throws that error when the asked difficulty has no notes. The check must also be reachable from a song that is already loaded, because the Preview loader has a song in hand, not a path.

**What changes.** `analyze_chart_file` calls the owner's load-and-check instead of loading and checking by hand. The Preview loader's one line throws the owner's typed error. The pather's own sentence goes; see the open question below for what the backstop becomes. In `user_messages.cpp` only the no-notes match changes: it either becomes a check on the typed error where `plain_error` catches the exception, or it stays and its comment names the owner it mirrors. The `hydra_replay` and `hydra_bench` sources are C3a's and are not touched here; both already check for an empty song before calling `analyze_chart`.

## Finding 87: the status words in the state dump

**Today.** `render_chips` in `src/ui/library_table.cpp` spells the chip labels ("Not analyzed", "Stale", "Analyzed") in its own table. `best_path_label` in `src/ui/library_model.cpp` spells "Stale" and "Not analyzed" again for the Best path cell. `dump_state` in `tests/ui/uitest_harness.cpp` prints a third set, "current", "stale" and "new", after `status=` on each row line. `docs/agents/ui-testing.md` does not quote the dump words, and no checked-in uitest script greps them, so only the harness changes.

**Owner.** One `status_label(RecordStatus)` in `src/ui/library_model.cpp`, beside `chip_of`, declared in `src/ui/library_model.h`, returning the chip words: Ready reads "Analyzed", Stale reads "Stale", NotAnalyzed reads "Not analyzed".

**What changes.** `best_path_label` reads its two words from `status_label`. `dump_state` prints `status_label`'s word instead of its own three. `render_chips` in `library_table.cpp` is K3's file and stays as it is in this task; the "GUI text unchanged" rule holds because the words are identical.

## Owned files

Only these may change:

- `src/parse/song.h`, `src/parse/song.cpp`
- `src/app/analysis.cpp`
- `src/ui/preview_load_job.cpp` (one line)
- `src/search/pather.cpp`
- `src/app/user_messages.cpp` (the no-notes match only)
- `src/ui/library_model.cpp`, `src/ui/library_model.h` (the `status_label` declaration; the plan names only the .cpp, but the harness needs the declaration)
- `tests/ui/uitest_harness.cpp`
- `tests/test_song.cpp`, `tests/test_library_model.cpp`, `tests/test_user_messages.cpp` (added beyond the plan's row so the preamble's test-first rule can be met; flagged to the orchestrator)

If a fix needs any other file, stop and report which and why.

## Test cases to add

Each pins only words; no numbers, floors, seeds, depths or bands.

- `tests/test_song.cpp`: "the no-notes error carries no_notes_message's sentence" pins that the typed error for Hard, Pro Drums reads "No Hard Pro Drums notes in this chart." and that it is caught as a `ChartFileError`.
- `tests/test_song.cpp`: "load-and-check throws the no-notes error for a missing difficulty" uses a corpus chart with no Hard charting (the corpus helper and the no-Hard search `tests/test_corpus_cache.cpp` already uses) and expects the typed error with the owner's sentence; it also loads a chart that has the difficulty and expects no throw. If no corpus chart lacks Hard, stop and report.
- `tests/test_user_messages.cpp`: extend the existing case "user_messages: the no-notes message is already plain and passes through" so the typed error, not only a plain string, passes through `plain_error` unchanged.
- `tests/test_library_model.cpp`: "library model: status_label is the one source of the three status words" pins "Analyzed", "Stale" and "Not analyzed", and that `best_path_label` for a Stale and a Not analyzed row returns the same words.

Existing cases that must stay green: `tests/test_model.cpp` "no_notes_message names the difficulty and the drum mode"; `tests/test_library_model.cpp` "library model: every chart shows, sorted by title, with its Best path text" and "library model: a status chip narrows the rows, and an emptied chip falls back to All".

## Test filters you may run

`build-cpp\Release\hydra_tests.exe` with:

- `-tc="*no-notes*"` and `-tc="*no_notes*"`
- `-tc="*load-and-check*"`
- `-tc="user_messages: the no-notes*"`
- `-sf=*library_model*`
- `-sf=*test_song*` (if the new cases need the whole file to compile and run)

GUI check, by label and text only: build `hydra_uitest` and run one command file from your scratchpad with the lines `click Scan library`, `wait-idle`, `click //Scanning charts/Continue`, `state`. The row lines must end `status=Not analyzed` (or "Stale"/"Analyzed" for a row with a record). Never run `hydra_uitest --all`.

## Done when

- The literal "No drum notes in this chart." is gone from the source (grep `src/`, `tests/`, `tools/`).
- One typed no-notes error exists, and its sentence comes from `no_notes_message`.
- `hydra_uitest state` prints the chip words "Analyzed", "Stale", "Not analyzed".
- The GUI's text is unchanged: `best_path_label` still returns "Stale" and "Not analyzed", and the chip labels still read as `docs/agents/ui-testing.md` line 99 lists them.
- `git diff --stat dc7e582..HEAD` lists only the owned files.
- The results stamp stays "2.1.0"; no score, path or stored record changes.

## Open questions

1. **The pather's backstop.** The draft has `pather.cpp` "call" the load-and-check owner, but `analyze_chart` receives a loaded `Song` and a `SearchSettings`, and `SearchSettings` carries no difficulty or drum mode (those live on `app::AnalysisSettings`, and `Song` does not record them). So the pather cannot build the owner's sentence on its own. Either the backstop drops, since every caller at base checks for an empty song first (`analysis.cpp`, `preview_load_job.cpp`, `tools/replay.cpp` at its two call sites, `tools/bench.cpp`), or the typed error has to reach the pather with the difficulty from somewhere. The draft's proposed new case, "`analyze_chart` on an empty song throws the owner's sentence", only makes sense under the second shape. This brief does not pick. Ask the orchestrator before touching `pather.cpp`; until then, make the owner, the analysis and Preview callers, `user_messages.cpp` and finding 87, and report.
2. **Test files.** The plan's C3b row names no unit-test file. The three test files above are added so the new cases have a home. If the orchestrator wants them elsewhere, it says so before you start.
