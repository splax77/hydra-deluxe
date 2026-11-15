# Brief: T5, verify re-send for one slice's unreached records

Read `docs/agents/brief-preamble.md` first, in the worktree named below. Its rules bind you. Where this brief and that page disagree, stop and report it.

## Who and where

- Task id: T5-V-RESEND-<slice>, where <slice> is the slice the launch prompt names (for example A26, A27 or C3). Session id: 16892967-2cf8-4437-a7cd-e3dad250f621.
- Your worktree: `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\test-audit-records`, branch `claude/test-audit-records`. Read code there. Do every git command there.
- Your input: `docs/audit/2026-11-14-test-audit/records/<slice>.md` (the auditor's records) and `verify/<slice>.md` (a first verifier's partial check). The first verifier ran out of time; its section "Not reached" lists, by exact name, the records whose test code it did not open. You audited and verified none of them.
- The one file you own and write: `docs/audit/2026-11-14-test-audit/verify/<slice>-resend.md`. Put your agent id on its first line. Commit it by name, with the trailers the preamble gives. Other agents commit on this branch at the same time: stage only your file by name, and if git reports an `index.lock`, wait a few seconds and retry. Commit after every few records. You change no test, no code and no other file.
- Tests you may run: none. You build nothing and run no `hydra_*` program (agent rule 14).
- Caps and status: as the preamble says. This is the slice's one re-send: any record you do not reach is listed in your file as "not audited", by exact name.

## The job

Do for every record in the "Not reached" list exactly what `briefs/T5-verify-R7a.md`, section "The job", asks of a verifier, with the reading rules it carries (question 3 for a claim not about a chart; a claim about threads, timing or a transport; "claim not checked only" is Investigate, not Rewrite). Read the test's code line by line and the production code it calls before you agree or dispute.

## Your final report

Return: the commit hash; the number of records checked, agreed and disputed; the exact names of any record you did not reach; and anything you stopped on.
