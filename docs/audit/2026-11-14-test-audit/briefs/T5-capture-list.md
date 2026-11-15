# Brief: the capture pass's list, scouts CL1 to CL4

Read `docs/agents/brief-preamble.md` first, in the worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records` (branch `claude/test-audit-records`). Its rules bind you. Session id: e633f7f6-f313-4787-b622-8d901b577b64. Task id: T5-CL1 to T5-CL4, as your launch prompt names.

## The job

Many audit records say "claim not checked" and name what would check the claim: an engine dump of a real chart, a screen capture, or a real input (a real file, store, settings file, or recorded hook call). The main session will now take them. Your job is to list every one your records files and their verify files name, so none is missed. You judge no test and change no record.

Your files are in `docs/audit/2026-11-14-test-audit/records/` and the matching files in `verify/` (same slice name, plus any `-resend`, `-reread`, `-q3`, `a` or `b` file):

| Scout | Records files |
|---|---|
| CL1 | A1 to A27 |
| CL2 | A28 to A50, B1 to B7 |
| CL3 | C1 to C15, C14-C15-fuzz, D1 to D16 |
| CL4 | E1 to E4, every `delta-*.md`, every `window-*.md` |

Skip `T3.md` and `T3-run2.md` (calibration).

Read each file whole. For each record that says "claim not checked" (or names a capture, dump, real input or recorded call it needs), write one line per thing it needs. A grep for "not checked" finds most; read around each hit, and also look for "needs a", "capture", "dump", "real input", "--efill", "screenshot".

## Your file

`docs/audit/2026-11-14-test-audit/capture/<your id>.md`. Stage only it, by name, and commit with the preamble's trailers. Other agents commit on the branch at the same time; check `git diff --cached --name-only` shows only your file, and retry on `index.lock` or a moved HEAD.

Four sections, one table each:

1. **Engine dumps.** Columns: chart (its path under `testdata/input`, or its name if the record gives only a name), the settings asked (early-fill window, score depth, cap, legacy fills, difficulty), what is read from it (a path, a total, an activation tick, an E mark), and the records that need it (`<file> <record id or test line>`).
2. **Screen captures.** Columns: screen (tab, window, box), chart and path, the moment or state, what is read, and the records.
3. **Real inputs for files, store and settings.** Columns: what (for example a real `hydra_settings.ini` with a `#` path, a real `hydra.db` from release 2.8.0), what is compared against what, and the records.
4. **Real hook inputs.** Columns: hook, the recorded call or state needed, and the records.

Then one more table: per records file, the number of "claim not checked" records and the number of lines you wrote for them. Merge lines that ask for the same thing, listing every record on the one line.

Name a chart only by what the record says or by a path you found in `testdata/input`. Do not guess a chart. Where a record says "the first corpus chart with paths" or similar, write it so.

## Caps and status

The progress hook's caps (a warning at 100 calls or 20 minutes, a stop at 150 calls or 30 minutes). At the warning, commit and list the files you did not reach. Status lines every 5 minutes with the preamble's one call. You run no `hydra_*` program and build nothing.

## Report

Return: the commit hash, the four table row counts, and any file not reached.
