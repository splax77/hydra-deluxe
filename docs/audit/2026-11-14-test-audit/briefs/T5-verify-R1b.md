# Brief: T5, verify round 1, slices A3 and A4

Read `docs/agents/brief-preamble.md` first, in the worktree named below. Its rules bind you. Where this brief and that page disagree, stop and report it.

## Who and where

- Task id: T5-V-R1b. Session id: 16892967-2cf8-4437-a7cd-e3dad250f621.
- Your worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records`, branch `claude/test-audit-records`. Read code there. Do every git command there.
- Your slices: A3 and A4. Their records are `docs/audit/2026-11-14-test-audit/records/A3.md` and `records/A4.md`, written by other agents. The tests they judge are named in `coverage.md` and in each records file's head. You audited neither.
- The files you own and write: `docs/audit/2026-11-14-test-audit/verify/A3.md` and `verify/A4.md`, one per slice. Put your agent id on the first line of each. Commit them by name, with the trailers the preamble gives (Task: T5-V-R1b). Other agents commit on this branch at the same time: stage only your files by name, and if git reports an `index.lock`, wait a few seconds and retry. Commit after every few records. You change no test, no code and no records file, and you write no other file.
- Tests you may run: none. You build nothing and run no `hydra_*` program (agent rule 14).
- Caps: the progress hook's, as the preamble says (a warning at 100 calls or 20 minutes, a stop at 150 calls or 30 minutes). At the warning, commit and list, by slice and exact test name, every record you did not reach.
- Status: one line every 5 minutes through `status_append.ps1`, as the preamble says.

## The job

Read `docs/agents/test-rubric.md` and `docs/agents/test-rubric-examples.md` first. Then check every record in both slices against the code, the Keeps as well as the rest: a wrong Keep is the miss this audit exists to stop. For each record:

- Check each answer's claims against the files and lines it names. Mark each claim "by reading" (you read the code and it holds) or "proven by" a named run that already exists in the records folders. You run nothing, so most marks are "by reading".
- Say whether you agree with the verdict. Where you do not, say what the verdict should be and why, with file and line.
- Check question 2 against your own search of `src/` for callers. A caller the record missed is a finding.
- Check question 3. It must name a file in `records-engine/` or `records-screen/` and quote its "Engine:" and "Claim:" lines, or say "claim not checked". If a record says "claim not checked" and the chart it needs is now in one of those folders, finish it there.
- Check question 8's named bug: would it really turn this test red, and at which line? Say so per assertion.
- List every split the record's tables or answers show, one line each, including any the auditor missed.
- Flag any rewrite that proposes a saved expected-output file (the filed ruling at `docs/agents/rulings/code-and-engine.md` line 50: "we don't need golden anymore, remove it all").
- Count the records against the slice's test cases. A3 is `tests/test_activation_rows.cpp`, the test cases that start at lines 841 to 1695 (22). A4 is `tests/test_analysis.cpp`, all 48. A test case with no record is a finding.

Questions 3's sources: `docs/audit/2026-11-14-test-audit/records-engine/index.md` and `records-screen/index.md`. Read a screenshot with the Read tool.

## Rules every audit agent keeps

- A finding leaves the audit by one of four exits: fixed, a question to the user, covered by a ruling, not a problem (`docs/agents/audit-method.md`). Only the main session picks an exit. You pick none, and you grade nothing as closed.
- Every split, where two parts of Hydra answer one question differently, is its own line.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own. Widen it, and say where you looked.
- No whole-library run happens without the user's `allow library run` in chat. You run nothing anyway.
- The user's words can be in the decision log (`docs/audit/2026-10-03-fix-decisions.md`), the question files under `docs/audit/`, the plans under `docs/superpowers/plans/`, and the session cards in `docs/sessions/`. Only a line under `## Confirmed` in `docs/agents/rulings/` is a filed ruling. Open every D-number a record cites and quote what it says.

## Your final report

Return: the commit hash of your verify files; per slice, the number of records checked, how many verdicts you agree with and how many you dispute; the exact test names of any record you did not reach; any test case with no record; and anything you stopped on. The verify files are the answer; do not restate them.
