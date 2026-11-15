# Brief: T5, the question-3 sweep, groups SW1 to SW4

Read `docs/agents/brief-preamble.md` first, in the worktree named below. Its rules bind you. Where this brief and that page disagree, stop and report it.

## Why this sweep exists

Partway through T5 a reading rule was set: a test whose only fault is question 3 of `docs/agents/test-rubric.md` ("claim not checked", every other question holding) gets Investigate, with the words "claim not checked only", not Rewrite. Records written before that rule may still say Rewrite for such a test. A9 and A14 are known cases. Your job is to find every one in your group, so the main session can move them to Investigate. You change no record.

## Who and where

- Task id: T5-SW1 to T5-SW4, as your launch prompt names. Session id: e633f7f6-f313-4787-b622-8d901b577b64.
- Your worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records`, branch `claude/test-audit-records`. Do every git command there.
- The one file you own and write: `docs/audit/2026-11-14-test-audit/sweep/<group>.md` (for example `sweep/SW1.md`). Commit it by name, with the trailers the preamble gives. Other agents commit on this branch at the same time: stage only your file by name, and if git reports an `index.lock`, wait a few seconds and retry. Commit after every few records files, not only at the end.
- Tests you may run: none. You build nothing and run no `hydra_*` program.
- Caps: the progress hook's (a warning at 100 calls or 20 minutes, a stop at 150 calls or 30 minutes). At the warning, commit and list every records file you did not reach.
- Status: one line every 5 minutes through `status_append.ps1`, as the preamble says.

## Your records files

| Group | Records files in `docs/audit/2026-11-14-test-audit/records/` |
|---|---|
| SW1 | A1 A2 A10 A11 A12 A13 A14 A15 A16 A17 A18 A19 A20 A21 A22 A23 A24 A25 A26 A27 |
| SW2 | A3 A4 A5 A6 A7 A8 A9 A28 A29 A30 A31 A32 A33 A34 A35 A36 A37 A40 A41 A42 A43 A44 A45 A46 A47 A48 A49 |
| SW3 | B1 B2 B3 B4 B5 B6 B7 C1 C2 C3 C4 C5 C6 C7 C8 C9 C10 C11 C12 C13 C14 D1 D10 D11 |
| SW4 | D2 D3 D4 D5 D6 D7 D8 D9 D12 D13 D14 D15 E1 E2 E3 E4 |

`sweep/rewrites.tsv` lists every record whose verdict is Rewrite, one row each: its records file, its heading, and a `prefilter` column. The prefilter marks a row `q3-unchecked` when the record's text holds "not checked", "hand-built" or "no real input". It is a text match, a pointer only. Judge every Rewrite row of your files, marked or not.

Each slice's verify files are in `verify/` with the same name, plus any `-resend`, `-reread` or `-q3` file for that slice (for example `verify/A26.md` and `verify/A26-resend.md`; the E slices have `E1a.md` and `E1b.md`). A verify file can dispute a record's answers or its verdict. Read the record as the verify file left it.

## The test for each Rewrite record

For each Rewrite row, read the record and its verify entry. Then answer: is question 3 the only fault?

- **Q3 only** when the record's reason for Rewrite is that the claim was not checked on the real thing (a hand-built fixture, no capture, no real input, no chart record), and every other question (1, 2 and 4 to 11) holds, as the record answers them and as the verify file leaves them.
- **Other fault** when any other question fails: a weak or missing assertion, a test that cannot fail, a wrong or copied rule, a duplicate, a name that does not match the test, and so on. Name the question and the fault in a few words.
- **Unclear** when the record does not answer the other questions well enough to tell. Say what is missing. Do not open the test code to fill a gap in the record; the main session decides those.

Write the record's own words that show it. A label is not a reason: "weak spot", "approved" or "fine" in a record proves nothing without the sentence behind it.

## Your file

One section per records file, then one line per Rewrite row in it, in the order of `rewrites.tsv`:

`<heading> | Q3 only / Other fault / Unclear | <the record's words, quoted, and the verify file's, if it disputes> | <for Q3 only: the capture or real input the record names, or "none named">`

At the end of the file, a count table: per records file, the number of Rewrite rows, Q3 only, Other fault and Unclear. The totals must equal your group's rows in `rewrites.tsv`.

## Rules every audit agent keeps

- Only the main session picks a finding's exit or changes a verdict. You list; you change nothing.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own.
- No whole-library run happens without the user's `allow library run` in chat. You run nothing anyway.

## Your final report

Return: the commit hash of your file; the count table's totals; every records file you did not reach; and anything you stopped on.
