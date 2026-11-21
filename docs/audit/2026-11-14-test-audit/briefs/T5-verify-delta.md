# Brief: T5, delta verify round, slices DL1 to DL5

Read `docs/agents/brief-preamble.md` first, in the worktree named below. Its rules bind you. Where this brief and that page disagree, stop and report it.

## Who and where

- Task id: T5-V-<slice>, as your launch prompt names (for example T5-V-DL1). Session id: e633f7f6-f313-4787-b622-8d901b577b64.
- Your worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records`, branch `claude/test-audit-records`. Read code there. Do every git command there.
- Your part: every record in `docs/audit/2026-11-14-test-audit/records/delta-<slice>.md`. Another agent wrote them; you did not audit them. That agent's brief, `briefs/T5-delta.md`, says which files your slice covers and which test cases are in scope: new ones, ones whose body or called helper changed, and removed ones, between commit `c10c7f5485cb34ab77e15c7298403223771b5ccc` and `5ec682c7`.
- The file you own and write: `docs/audit/2026-11-14-test-audit/verify/delta-<slice>.md`. Put your agent id on its first line. Commit it by name, with the trailers the preamble gives. Other agents commit on this branch at the same time: stage only your file by name, and if git reports an `index.lock`, wait a few seconds and retry. Commit after every few records. You change no test, no code and no records file, and you write no other file.
- Tests you may run: none. You build nothing and run no `hydra_*` program (agent rule 14).
- Caps: the progress hook's (a warning at 100 calls or 20 minutes, a stop at 150 calls or 30 minutes). At the warning, commit and list, by file and exact test name, every record you did not reach.
- Status: one line every 5 minutes through `status_append.ps1`, as the preamble says.

## The job

Read `docs/agents/test-rubric.md` and `docs/agents/test-rubric-examples.md` first. Then:

1. **Scope.** Run `git diff c10c7f5485cb34ab77e15c7298403223771b5ccc 5ec682c7 -- <file>` for each of your slice's files yourself, and list every test case that is new, changed (body or a called helper in those files) or removed. A test case in scope with no record is a finding. A record for a test that did not change is a note, not a fault.
2. **Each record.** Check every record against the code at `5ec682c7`, the Keeps as well as the rest: a wrong Keep is the miss this audit exists to stop.
   - Check each answer's claims against the files and lines it names. Mark each claim "by reading" (you read the code and it holds) or "proven by" a named run that already exists in the records folders. You run nothing, so most marks are "by reading".
   - Check the record's delta line. For a changed test, open the old record it names and say whether the old verdict should still stand.
   - Say whether you agree with the verdict. Where you do not, say what the verdict should be and why, with file and line.
   - Check question 2 against your own search of `src/` for callers. A caller the record missed is a finding.
   - Check question 8's named bug: would it really turn this test red, and at which line? Say so per assertion.
   - List every split the record's tables or answers show, one line each, including any the auditor missed.
   - Flag any rewrite that proposes a saved expected-output file (the filed ruling at `docs/agents/rulings/code-and-engine.md` line 50: "we don't need golden anymore, remove it all").

Question 3 for a claim that is not about a chart. "Not an engine claim", "not a chart-play fact" and "no chart fact is claimed" are not answers; the rubric's verdict section rules Keep out unless the claim was checked on the real thing it is about. If the claim is about files, folders, the store or the settings, the real thing is real input: which real input the claim was checked on (the corpus, a real chart file, a real settings file) and against which independent owner. A test whose only input is a hand-built fixture has not had its claim checked on real input. If the claim is about what a window, panel or status line shows, or where it sits, the real thing is a capture of that screen; until one exists, "claim not checked", and the verdict cannot be Keep. A claim about threads, timing, locks or a transport is checked on the real mechanism: the real thread pool, the real clock, a real second process or connection holding the real lock, the real WinHTTP stack against a loopback server. Dispute every Keep whose question 3 does not meet this.

A test whose only fault is question 3 gets Investigate, not Rewrite, with the words "claim not checked only" and the capture or real input it needs. Dispute a Rewrite whose only fault is question 3.

Read the code behind every record, line by line. Agreeing with a verdict because of its shape, without opening the assertions, is not a check; a record you could not open is listed as not reached, by exact test name.

Question 3's sources: `docs/audit/2026-11-14-test-audit/records-engine/index.md` and `records-screen/index.md`. Read a screenshot with the Read tool. The screen records were made at `c10c7f54`, and their SP-end screenshots land just before the end, so they are no evidence of the screen at or after an SP end.

## Rules every audit agent keeps

- A finding leaves the audit by one of four exits: fixed, a question to the user, covered by a ruling, not a problem (`docs/agents/audit-method.md`). Only the main session picks an exit. You pick none, and you grade nothing as closed.
- Every split, where two parts of Hydra answer one question differently, is its own line.
- "On purpose", "by design", "deliberately" and "kept" are claims, not evidence.
- A search that finds nothing proves nothing on its own. Widen it, and say where you looked.
- No whole-library run happens without the user's `allow library run` in chat. You run nothing anyway.
- The user's words can be in the decision log (`docs/audit/2026-10-03-fix-decisions.md`), the question files under `docs/audit/`, the plans under `docs/superpowers/plans/`, and the session cards in `docs/sessions/`. Only a line under `## Confirmed` in `docs/agents/rulings/` is a filed ruling. Open every D-number a record cites and quote what it says.

## Your final report

Return: the commit hash of your verify file; the number of in-scope test cases you found and the number of records; how many verdicts you agree with and how many you dispute; any in-scope test case with no record; the exact test names of any record you did not reach; and anything you stopped on. The verify file is the answer; do not restate it.
