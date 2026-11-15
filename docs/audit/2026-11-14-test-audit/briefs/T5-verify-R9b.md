# Brief: T5, verify round 9, slices A35 and A36

Read `docs/agents/brief-preamble.md` first, in the worktree named below. Its rules bind you. Where this brief and that page disagree, stop and report it.

## Who and where

- Task id: T5-V-R9b. Session id: 16892967-2cf8-4437-a7cd-e3dad250f621.
- Your worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records`, branch `claude/test-audit-records`. Read code there. Do every git command there.
- Your slices: A35 and A36. Their records are `docs/audit/2026-11-14-test-audit/records/A35.md` and `records/A36.md`, written by other agents. The tests they judge are named in `coverage.md` and in each records file's head. You audited neither.
- The files you own and write: `docs/audit/2026-11-14-test-audit/verify/A35.md` and `verify/A36.md`, one per slice. Put your agent id on the first line of each. Commit them by name, with the trailers the preamble gives (Task: T5-V-R9b). Other agents commit on this branch at the same time: stage only your files by name, and if git reports an `index.lock`, wait a few seconds and retry. Commit after every few records. You change no test, no code and no records file, and you write no other file.
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
- Count the records against the slice's test cases. A35 is `tests/test_tied_variants.cpp` (13 test cases). A36 is `tests/test_track_state.cpp` (23). A test case with no record is a finding.

Question 3 for a claim that is not about a chart. "Not an engine claim", "not a chart-play fact" and "no chart fact is claimed" are not answers; the rubric's verdict section rules Keep out unless the claim was checked on the real thing it is about. If the claim is about files, folders, the store or the settings, the real thing is real input: write which real input the claim was checked on (the corpus, a real chart file, a real settings file) and against which independent owner. A test whose only input is a hand-built fixture has not had its claim checked on real input. If the claim is about what a window, panel or status line shows, or where it sits, the real thing is a capture of that screen. Until one exists, write "claim not checked", name the capture needed, and the verdict cannot be Keep. Bad: "3 Real chart: not an engine or display claim; no data to read." Good: "3 Real chart: Engine: the `testdata/input` corpus scanned once and with its root given twice. Claim: the two path lists are equal; checked on real input, owner `walk_folders`." Dispute every Keep whose question 3 does not meet this.

A claim about threads, timing or a transport is checked on the real mechanism: the real thread pool, the real clock, the real WinHTTP stack against a loopback server. That checks how the code behaves, not what a real server sends; a claim about a real server's reply needs a saved real reply named by its source. Read the code behind every record, line by line. Agreeing with a verdict because of its shape, without opening the assertions, is not a check; a record you could not open is listed as not reached, by exact test name.



A test whose only fault is question 3 ("claim not checked", every other question holding) gets Investigate, not Rewrite, with the words "claim not checked only" and the capture or real input it needs. Rewrite is for a fault in the test itself. The capture pass after the last round finishes these, and a Keep may then follow. Dispute a Rewrite whose only fault is question 3.

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
