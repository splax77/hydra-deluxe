# Brief: the repeated-values pass, RP1 to RP3 and their verifiers

Read `docs/agents/brief-preamble.md` first, in the worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records` (branch `claude/test-audit-records`). Its rules bind you. Session id: e633f7f6-f313-4787-b622-8d901b577b64. Task ids: T5-RP1 to T5-RP3 for a sorter, T5-V-RP1 to T5-V-RP3 for a verifier.

## Why this pass exists

The audit judged one test at a time. A value copied by hand into many test files is a fault no single record sees whole. `cross-findings.md` holds the first two found that way: X1 (Burnout's "3- 1 2" and 378315 typed in ten files) and X2 (the Path Builder's fill ticks). Read it first; it shows what a finding of this kind is and how it is written.

The main session listed every literal typed on an assertion line or in a test constant, in three or more test files: `repeats/repeats.tsv`, 207 rows, made by a script that reads `tests/` for CHECK, REQUIRE, IM_CHECK, assert, `==`, `!=` and `constexpr` lines. Each row is a value, its file count, its use count and every `file:line`. The script sees only those lines. A value set in an array and asserted later, or typed in a comment, is not in it; when you see one beside a row, add it.

## Your rows

Rows are numbered from 1 after the header, in the file's order.

| Sorter | Rows |
|---|---|
| RP1 | 1 to 69 |
| RP2 | 70 to 138 |
| RP3 | 139 to 207 |

## The sorter's job

For each row, open enough of its places to say what the value is in each, and sort the row into one kind. A row can hold different things in different files (1000 can be a tick in one and a score in another): split such a row into its parts and sort each part.

- **K1, an engine answer copied.** The value is what the engine, the store or the app computes from a real chart or a real input (a score, a path, a tick an activation lands on, an E mark, a count of paths, a ms figure), typed instead of read from the engine's record. Name the chart and the owner that computes it (file and function in `src/`). This is X1's kind.
- **K2, a product constant or text copied.** The value is a constant or a string `src/` owns (a window, a cap, a limit, a message, a label), typed instead of read from that constant. Name the owner (file, line, name).
- **K3, the test's own input.** The value is something the test or its fixture builds and owns: a resolution, a tick on a hand-built chart, a tempo, a path a test writes. Typing it is not a copy. Say where the test builds it.
- **K4, a fact of a real chart file.** The value is read from the chart file itself, not computed (a note count, a resolution, a song name), and the engine does not decide it.
- **Unclear.** Say what you could not tell.

For each K1 and K2 part, also write whether a change to its owner would turn every copy red at once (the X1 harm), and list every copy's `file:line`.

Group K1 and K2 parts that are one fact (for example 378315 and "378,315" and "3- 1 2" are one fact, Burnout's best path) into one candidate finding, numbered `RP<n>-<k>`, with: the fact in game words, the owner, every copy, and whether X1 or X2 already covers it. Do not choose an exit; only the main session does.

## The verifier's job

Your part is one sorter's file. Another agent wrote it. Open the places of every K1 and K2 candidate and of a sample of at least one in five K3 and K4 rows, chosen across the whole range, and check the kind and the owner against the code. A K3 or K4 that is really K1 or K2 is a finding. Say for each candidate whether you agree. Add any copy the sorter missed.

## Files

A sorter writes `repeats/RP<n>.md`, a verifier `repeats/V-RP<n>.md`, with its agent id on the first line. Stage only your file by name, check `git diff --cached --name-only` shows only it, and commit with the preamble's trailers; retry on `index.lock` or a moved HEAD. Commit every 20 rows or so.

The file opens with a table, one row per sorted row or part: row number, value, kind, one-line reason, and `file:line` of each place you opened. Then the candidates section. Then counts per kind.

## Rules

- Only the main session picks an exit. "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own. Widen it, and say where you looked.
- You run no `hydra_*` program and build nothing. The engine dumps in `records-engine/` (including `burnout.efill60.dump.json`) are there to read.
- Caps: the progress hook's (a warning at 100 calls or 20 minutes, a stop at 150 calls or 30 minutes). At the warning, commit and list the rows you did not reach. Status lines every 5 minutes with the preamble's one call.

## Report

A sorter returns: the commit hash, counts per kind, the candidate list (id and one line each), and rows not reached. A verifier returns: the commit hash, candidates agreed and disputed, K3 and K4 rows checked and moved, copies added, and rows not reached.
