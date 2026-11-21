# Brief: T5, recheck A4's Keeps on question 3

Read `docs/agents/brief-preamble.md` first, in the worktree named below. Its rules bind you. Where this brief and that page disagree, stop and report it.

## Who and where

- Task id: T5-RQ3-A4. Session id: 16892967-2cf8-4437-a7cd-e3dad250f621.
- Your worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records`, branch `claude/test-audit-records`. Read code there. Do every git command there.
- Your input: `docs/audit/2026-11-14-test-audit/records/A4.md` (an auditor's records for `tests/test_analysis.cpp`) and `verify/A4.md` (a verifier's check of them). You wrote neither.
- The one file you own and write: `docs/audit/2026-11-14-test-audit/verify/A4-q3.md`. Put your agent id on its first line. Commit it by name, with the trailers the preamble gives (Task: T5-RQ3-A4). Other agents commit on this branch at the same time: stage only your file by name, and if git reports an `index.lock`, wait a few seconds and retry. You change no test, no code and no other file.
- Tests you may run: none. You build nothing and run no `hydra_*` program (agent rule 14).
- Caps and status: as the preamble says.

## The job

The auditor gave 31 Keeps. The verifier disputed six (tests 7, 28, 29, 30, 31 and 41) and accepted the others while noting that most of their question 3 answers read "not an engine or display claim". The rubric (`docs/agents/test-rubric.md`, "The verdict") rules Keep out unless the claim was checked on a real chart. Re-judge question 3, and only question 3, for every A4 record whose verdict is Keep and that the verifier did not dispute, by this rule:

Question 3 for a claim that is not about a chart. "Not an engine claim", "not a chart-play fact" and "no chart fact is claimed" are not answers; the rubric's verdict section rules Keep out unless the claim was checked on the real thing it is about. If the claim is about files, folders, the store or the settings, the real thing is real input: write which real input the claim was checked on (the corpus, a real chart file, a real settings file) and against which independent owner. A test whose only input is a hand-built fixture has not had its claim checked on real input. If the claim is about what a window, panel or status line shows, or where it sits, the real thing is a capture of that screen. Until one exists, write "claim not checked", name the capture needed, and the verdict cannot be Keep. Bad: "3 Real chart: not an engine or display claim; no data to read." Good: "3 Real chart: Engine: the `testdata/input` corpus scanned once and with its root given twice. Claim: the two path lists are equal; checked on real input, owner `walk_folders`."

For each such record write one line: the test's number and name, its question 3 as it should read, and "Keep stands" or "Keep falls: claim not checked, needs <the real input or capture>". Where a test already runs on the real corpus and compares two answers of the code, say which owner the comparison rests on, by file and line.

## Rules every audit agent keeps

- Only the main session picks a finding's exit. You pick none, and you grade nothing as closed.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own.

## Your final report

Return: the commit hash; the number of Keeps you re-judged; how many stand and how many fall; and anything you stopped on.
