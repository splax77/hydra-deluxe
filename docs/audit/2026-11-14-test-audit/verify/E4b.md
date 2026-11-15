Agent a1b5ee7da1fd07848 (task T5-V-R24-E4b, session 16892967-2cf8-4437-a7cd-e3dad250f621)

# T5 verify, round 24, slice E4 part b

Records checked: `test_status_append.ps1` (7), `test_h53_session_cards_start.ps1` (56), `test_h52_cleanup_sweep.ps1` (81), in `records/E4.md` lines 514-963. Hooks are in `C:\Users\Patrick\.claude\gates` (main 0586e15 as far as I can tell; I ran no git command there). I ran nothing, so every mark below is "by reading" unless it says otherwise. No existing run in `records-engine` or `records-screen` bears on these hook tests, so nothing here is "proven by".

## What I read

- Rubric, rubric examples (first 60 lines), the brief, the preamble.
- Tests in full: `test_status_append.ps1` (1-49), `test_h53_session_cards_start.ps1` (1-269), `test_h52_cleanup_sweep.ps1` (1-489), and `tests/lib/harness.ps1` in full (1-295).
- Hooks in full: `status_append.ps1` (1-27), `session_cards_start.ps1` (1-371), `cleanup_sweep.ps1` (1-743). The record admits it never opened `cleanup_sweep.ps1`; I did.
- Owners the hooks lean on: `common.py` 100-129 and 139 (`status_path`, `check_id`, the log append), `review_rules.py` 180-196 and 398-449 (`_latest_verdicts`, `_clean_keys`, `explain_main_move`), `progress.py` (grep: lines 22, 27, 38, 66, 100), `stall.py` (grep: 66, 77), `tools/session_cards/batch.py` and `prune_list.py` print lines and `--out`/`--cards-root` parsers (grep), one real registry record `C:\Users\Patrick\.claude\sessions\17280.json`, the real transcript layout `projects\C--Users-Patrick-Downloads-Hydra-hydra-test\<session id>.jsonl` (exists), and `settings.json`.
- Not read: `PUBLIC_DOCS` and `BUILTIN_CODE` in `review_rules.py`; real `prepass.py`, `prune_list.py` argument parsers beyond the grep; the adding commits 00d390a and 87e2252 (subject lines only, as in the records); `test_progress.py`.

## Findings that cut across records

1. **The record's Q2 for `status_append.ps1` missed the real owner of the file's path.** `gates/common.py:119` `status_path(agent_id)` is the Python owner of "where agent X's status file is": `STATE_ROOT/status/<check_id(id)>.md`. `progress.py:100` reads that file's mtime to decide whether the agent is overdue, `progress.py:66` names it in the refusal, and `stall.py:66` and `stall.py:77` read it. `status_append.ps1` lines 10 and 20 work out the same path again in PowerShell: a literal `C:\Users\Patrick\.claude\gates\state\status` plus `<target>.md`, with no `check_id` (a target containing `/`, `\` or `:` is accepted, and any target ending `.md` is written wherever it points). Two parts answer "where is the status file" and no test compares them. The test only ever passes a full `.md` path, so the writer and reader paths never meet. This is a split (S1 below).
2. **Other callers of `status_append.ps1` the record did not list** (grep of `C:\Users\Patrick\.claude`, files `settings.json`, `gates/*`): `settings.json:8` and `settings.fragment.json:4` (the allow rule that is the reason the script exists, per its own header), `progress.py:22` (`STATUS_PREFIX`) and `progress.py:38` (the call shape given in a refusal), `tests/test_progress.py:25`, `test_percall_rewrite.py:89`, `test_shell_output.py:111`, `tests/replay.py:304`. Also `cleanup_sweep.ps1:597` prunes `state\status\*.md`.
3. **The test and the real caller run different PowerShells.** The test runs `& powershell` (Windows PowerShell 5.1, test line 17). The agents' PowerShell tool is described as PowerShell 7+. `Add-Content -Encoding utf8` writes a byte-order mark in 5.1 and none in 7. `stall.py:77` reads with `utf-8-sig`, so it tolerates both, but the test never runs the form agents use. Unverified: which shell actually executes the allow-ruled call. Flagged for the main session.
4. **Silent success.** `status_append.ps1:6` sets `$ErrorActionPreference = 'SilentlyContinue'` and line 26 prints "appended to" whatever `Add-Content` (line 25) did. A failed write still exits 0 and says "appended". No test covers a failed write (read-only file, locked file, bad folder). Only check 3 (line 27) would notice a missing file, and only because it re-reads the file. Production finding against `status_append.ps1`, and a coverage gap.
5. **Every file has an emitted check with no record.** `Assert-LiveStateUntouched` (harness.ps1:280-295) runs `Check` once at the end of each file (lines 48, 268, 488). Each is a real emitted result and has no record in E4.md (the brief's counts of 7, 56 and 81 count only the literal `Check` lines, and those all have records). By reading, none of the three can fail: it fails only if a file appears under the live `gates\state` whose path contains the marker. `status_append.ps1` called with a full `.md` path writes in a temp folder; `session_cards_start.ps1` is given `-StateRoot` and writes only `session_cards\claim.txt` and `last_removal_reminder.txt`, whose paths hold no marker; `cleanup_sweep.ps1` only deletes and writes `cleanup_sweep\*` under the given `-StateRoot`. So the guard is true by construction. Three missing records, one per file, each with the verdict "cannot fail" (Q8). The record at E4.md:538 also misreads it: it says line 48 "checks only the live status folder"; it checks every path under `gates\state`.
6. **Real tools, checked against the fakes (h53).** The fake `batch.py` and `prune_list.py` print `sessions=N batches=N waves=N` and `paths=N held_back=N mb=X.X`. The real ones print exactly those shapes (`batch.py:75`, `prune_list.py:147`). The real `batch.py` writes `wave-{w}-batch-{n}.txt` (`batch.py:71`), the names the hook's regex and the test's fakes use. Real `check.py` takes `--out` and `--cards-root` (`check.py:289-290`); real `batch.py` takes only `--out` (`batch.py:46`), so the test's claim "prepass and batch do not get `--cards-root`" (check at 265) agrees with the real parser. So the fakes are faithful in the parts the hook reads. What stays unchecked is a real `prepass.py`/`batch.py` run on real transcripts and a recorded real SessionStart payload (the test types `session_id`, `cwd`, `hook_event_name`, `source`).
7. **Real registry and transcript shape (h52).** The hand-typed registry record in `test_h52` line 313 (`pid`, `sessionId`, `procStart`) uses exactly the fields `Get-RunningSessions` reads (`cleanup_sweep.ps1:374-378`), and a real record (`sessions\17280.json`) holds those three fields, with `procStart` in the same `FileTimeUtc` form the hook compares. The transcript layout `projects\<project>\<session id>.jsonl` matches the real one. The scratch layout `Temp\claude\<project>\<session id>` matches the real one named in this session's environment. So the fixtures have the real shape; the age rules (7 days, 24 hours, 30 days, 1 hour) are still only exercised on typed ages.
8. **Thresholds have no reason beyond a label.** The h53 hook's `$ClaimHours = 2`, `$RemovalReminderMb = 500`, `$ReminderGapHours = 24` carry the comment "The user's choice (task SC-9, 2026-10-14)" (`session_cards_start.ps1:41-47`). I searched Hydra's `docs/sessions/` and `docs/agents/rulings/` for `SC-9` and found nothing, so it is not a filed ruling and not a reason (rubric Q5). The record's Q5 said the reason for 2 hours "is not given in the file (not opened)"; it is given as that label, and that is all. The test's 3 h, 5 h, 25 h, 499.9 and 500.0 are copies of those constants placed either side.
9. **Rewrites that point at a constant that does not exist.** Records 137, 139, 144, 145, 146, 195, 214, 236 and 266 (h53) say "point at the owner's text / by its own constant". `Get-CardNote` (`session_cards_start.ps1:269-292`) builds the whole sentence inline from literals; there is no constant to read. The rewrite as written needs a production change (a new owner), which the audit never proposes, or the sentence checks stay as typed phrases. This needs the main session to decide whether tests of the model-facing wording are wanted at all; I did not pick.
10. **No record proposes a saved expected-output file.** Checked every Rewrite in my part; none does (ruling `docs/agents/rulings/code-and-engine.md` line 50 not breached).

## Splits (one line each)

- S1: Where an agent's status file lives: `common.py:119` (`status_path`, with `check_id`) read by `progress.py:100` and `stall.py:77`, against `status_append.ps1:10,20` (hard-coded folder, no id check, any `.md` path accepted). No test makes them meet.
- S2: The live state folder is spelled three ways: `status_append.ps1:10` (a literal absolute path, deliberate per its line 8 comment), `harness.ps1:30` (`$HOME\.claude\gates\state`), and `cleanup_sweep.ps1:61` and `session_cards_start.ps1:53` (`$PSScriptRoot\state`). They agree on this machine only.
- Copy, not a split today (agree by reading): the sweep's `reviews.jsonl` keep rule (`cleanup_sweep.ps1:567-583`, last verdict per key, exact compare) restates `review_rules._latest_verdicts` (`review_rules.py:188-190`, a dict keyed by `key`, last wins). They match. The test's expected text (line 443) is that rule applied by hand to named inputs, and never asks the reader. A property test (the reader's `_latest_verdicts` before and after the prune gives the same answer) would compare the two owners. I did not read the other places that read `path` in a verdict record (doc passes), so "same answer" is unproven for records that share a `path` but not a `key`.
- Copy, not a split today: the card note's sentence "A commit that changes only docs/sessions needs no reviewer" (`session_cards_start.ps1:289`, its comment at 267 names `review_rules.explain_main_move`) against `explain_main_move` (`review_rules.py:398-422`): a `docs/sessions` path is neither marked code nor (as far as I read) a public doc, so it agrees. `PUBLIC_DOCS` not opened.
- Copy, not a split: the note's docs-check command (`session_cards_start.ps1:285`) and Hydra's CLAUDE.md name the same command.
- Session liveness: only `cleanup_sweep.ps1` reads Claude Code's `sessions\` registry (grep of `gates` for `procStart` and `'sessions'` found no other reader), so no split there.

## Rule for my marks

Where a claim is about an OS mechanism (a real junction, a real held file handle, a real killed process tree, a real pipe, real git on a throwaway repo, real concurrent writer) I accept the real mechanism as the real thing, as the brief allows for threads, timing and transports and for "a real git repo". Where the claim is a decision on typed state (an age, a count, a message's wording, a registry value), the record's "claim not checked only" stands, because the input is the test's own. I apply this the same way across all three files; where the record did not, I dispute for consistency.

---

# test_status_append.ps1 (7 records, 7 checks, plus the emitted guard check with no record)

All seven: Q2 callers are in finding 2 above (record missed settings.json, progress.py, common.py, stall.py). Q3 by the brief's rule: the claim is about files; the input is a typed line and a temp folder; the real form agents use (bare id, live folder) is never run, and the test runs 5.1 where agents may run 7. The record's own Q3 says "real mechanism, typed input" and names no real input. By the brief this is "claim not checked", so a Keep is ruled out. The input needed: a recorded real status call through the real shape, or the helper made to take a state folder so the bare-id form can run against a temp folder (a production change; I propose none). Verdict for all seven: **Investigate, claim not checked only.**

1. **'a full .md path works and exits 0' (:25).** Read 22-25 and script 12-27 by reading. Q8: `exit 0` changed to `exit 1` turns it red. A failed write (finding 4) would not. Record Keep: **dispute, Investigate (claim not checked only).**
2. **'the output names the file written' (:26).** Q8: a changed confirmation text turns it red. Same Q3. **Dispute Keep, Investigate.**
3. **'the folder was made and holds the line' (:27).** Q8: removing the folder creation (script 23) leaves `Add-Content` failing silently, no file, red. Re-reads with .NET `ReadAllLines` (independent of `Add-Content`). Three conditions in one check (a failure prints only the path); agree with the record. Same Q3. **Dispute Keep, Investigate.**
4. **'a second call appends' (:32).** Q8: `Add-Content` turned into `Set-Content` turns it red. Same Q3. **Dispute Keep, Investigate.**
5. **'arguments after the target join into one line' (:37).** Q8: `-join ' '` turned into `-join ''` turns it red. The real call passes one quoted argument (`progress.py:38`), so this form is not the real one either. **Dispute Keep, Investigate.**
6. **'no message writes a placeholder line' (:42).** Q8: a changed placeholder text is red; `HH:mm` turned into `mm:ss` stays green (the regex is digits only), so the clock part is unchecked. The record's "a copy of line 17's format" holds. **Dispute Keep, Investigate.**
7. **'no target exits 1 and says so' (:46).** Record verdict Investigate: **agree on the verdict, dispute the reason.** The record says the comment "writes nothing" is unasserted. By reading, the script exits at line 13 before any write, so "writes nothing" is true by construction; and the record's belief that `Assert-LiveStateUntouched` checks it is wrong (finding 5). The real fault is the shared Q3. Q8: changing `exit 1` or the message text turns it red.

Missing record: the emitted `live state untouched (...)` check at line 48. Cannot fail (finding 5).

---

# test_h53_session_cards_start.ps1 (56 records, 56 checks, plus the emitted guard check with no record)

Shared Q2: the hook is registered at `settings.json:99` and `settings.fragment.json:91`, described at `gates/README.md:65`; no caller in `src/`. Shared Q3 for all 56: the hook runs for real in a real child `powershell.exe` and drives a real `py.exe`, but the tools are fakes (faithful in what the hook reads, finding 6), the payload is typed, and the state folder and claim file are throwaway. Real input missing: a recorded real SessionStart payload and the real `tools\session_cards` scripts on a copy of real transcripts. Unless a line says otherwise, "Investigate" below means the record's verdict "Investigate (claim not checked only)", and I **agree**.

**Scope**
1. 'another project''s session gets {}' (:110). Read; `Test-InScope` 178-181, hook 318. Q8: removing the scope test gives a note (sessions = 2), red. Agree.
2. 'and exits 0' (:111). Hook line 371 exits 0 always; `exit 1` there is red. Agree.
3. 'and runs none of the card scripts' (:112). Red if the scripts run out of scope. Agree.
4. 'and writes no claim' (:113). Red if `Enter-Claim` runs out of scope. Agree.
5. 'a folder whose name only starts like the repo is out of scope' (:115). `Test-PathUnder` adds `\` (hook 79); a plain `StartsWith(repo)` puts the sibling in scope, a note is printed, red. Agree.

**No session waiting**
6. 'with no session waiting the repo''s session gets {}' (:120). `$n -gt 0` (hook 336) turned `-ge` gives a note, red. Agree.
7. 'the scripts run in order: prepass, batch, prune_list' (:121). Swapping the order in the hook (330, 334, 346) is red. Agree.
8. 'with no session waiting no claim is written' (:122). Red if the claim is written before the `$n` test. Agree.
9. 'without -DataDir no script is passed --out' (:123). Red if `Get-RootArgs` always adds it. Agree.

**The note**
10. 'a worktree session with sessions waiting gets the card note' (:131). The 3 comes from the fake's own `sessions = 3`; the sentence start is hook 275. Red if the sentence changes. Agree.
11. 'the note is SessionStart additionalContext' (:132). Hook 363. The event name is Claude Code's contract, typed by hand in both places. Agree.
12. 'the note says nothing to the user (no systemMessage)' (:133). Red if a message is added when the note is given. Agree.
13. 'the note lists the batch files in wave and batch order' (:135). The test scrambles the order and expects wave then numeric batch; the sort is hook 262; a text-order sort gives `batch-1, batch-10, batch-2` and is red. The names match the real `batch.py:71`. Good check; still agree Investigate.
14. 'the note gives the batches folder in full, and the writer brief from the tools folder' (:136). Hook 270-272. Agree.
15. 'the note names the card-writer agent and never-run-check' (:137). Hook 278 has "never to run check.py" and 276 "card-writer". Phrase copy. Agree **Rewrite in kind**, but the stated rewrite has no constant to point at (finding 9).
16. 'the note says the agents write to staging and check.py moves passing cards to the cards root' (:139). Hook 279. Paths come from the test's inputs; the words 'write their cards to' and 'passes into' are the copied part. Agree Rewrite in kind (finding 9).
17. 'the note gives check, index, link_memory in that order, each by its full path' (:141). Hook 281-283. A property about order. Agree.
18. 'after link_memory the note gives the docs check in the main checkout, then the listing and the commit' (:143). Hook 285-288. The command string `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"` is also in Hydra's CLAUDE.md. Agree.
19. 'the commit step stages by name, on main, with the three trailers, and needs no reviewer' (:144). Hook 286-289. Four phrases copied. "needs no reviewer" is the rule `review_rules.explain_main_move` owns; by reading it agrees (a `docs/sessions` path is neither code nor, as far as read, a public doc; finding in "Splits"). Agree Rewrite; the rewrite should ask `explain_main_move` for a `docs/sessions`-only commit and compare, not retype the sentence.
20. 'the commit step confirms the branch and commits only docs/sessions' (:145). Hook 286-288. Agree Rewrite in kind.
21. 'the note says not to hold up the user''s request' (:146). Hook 290. Agree Rewrite in kind.
22. 'without -DataDir or -CardsDir the note''s commands carry no --out or --cards-root' (:147). Hook 136-144. Red if the note always adds them. Agree.
23. 'the claim names this session' (:150). `Enter-Claim` writes `id<TAB>time` (hook 236). Agree.

**The claim**
24. 'another session within the claim window gets no card note' (:155). Hook 327 and 215. Agree.
25. 'and the claim still names the first session' (:156). Agree.
26. 'and it leaves prepass and batch alone (only prune_list runs)' (:158). Red if hook 327 is dropped. The reason (batch.py rewrites files the first session's agents read) is in the hook's own comment 324-326; `batch.py:65` really deletes `wave-*-batch-*.txt` first. Agree.
27. 'a claim file held open by another process gives no note and runs no batch' (:163). The test opens `claim.txt` with `FileShare.None` (line 161); `Test-ClaimHeldByOther` (hook 204-208) gets a sharing-violation `IOException` and returns true. This is a real OS handle, the same kind of claim as h52 `:195`. Q8: removing the `IOException` branch makes the hook throw into the outer catch, the output is a systemMessage not `{}`, red. **Dispute Investigate, the real mechanism is the real thing: Keep** (consistency with h52 `:195`, which the record Keeps). The record's worry (one handle is not a race) is fair but the claim in the name is "held open", not "race".
28. 'and the claim still names the first session afterwards' (:164). Follows 27. **Dispute, Keep with it.**
29. 'the claiming session gets the note again when it resumes' (:167). Hook 215 (`$parts[0] -ne $sessionId`). Agree.
30. 'a claim three hours old does not hold the note back' (:172). 3 h against `$ClaimHours = 2`; the other side (seconds old) is checked by 24. The 2 is a label only (finding 8). Agree.
31. 'and the claim now names the new session' (:173). Agree.
32. 'a claim dated in the future does not hold the note back' (:176). Hook 196 (`$then -le $now`). Red if that term is dropped. Agree.

**Failing scripts**
33. 'a failing prepass gives a systemMessage naming it and its error' (:183). The text 'prepass.py failed (exit 1)' is built from the script name and exit code (hook 154), 'broke on purpose' is the test's own error text, so it is a property, not wording. The error shape is a fake traceback. Agree.
34. 'and exits 0' (:184). Agree.
35. 'and the batch still runs and the note is still given' (:185). Hook 330-331 own try. Red if the prepass failure stops batch. Agree.
36. 'a failing batch gives a systemMessage naming it, and no note' (:189). Hook 342. Agree.
37. 'and exits 0' (:190). Agree. Name repeats at 111, 184, 190, 213, 226 (record's side note holds).
38. 'and prune_list still runs' (:191). Hook 345 is outside the batch try. Agree.
39. 'sessions waiting but no batch file gives a systemMessage' (:195). Hook 339 throws 'wrote no batch file'. Pure wording. Agree Rewrite in kind (finding 9).
40. 'non-ASCII error text is escaped in the output and read back whole' (:201). Hook 365-368. Without the escape the console code page output has bytes of 0x80 or more either way (OEM or UTF-8), so the regex is red. Real child process, real pipe. **Agree Keep.** The accent is invented; real Windows error text may use other characters, immaterial to the escape.
41. 'a prepass past the budget is stopped and named in a systemMessage' (:211). Budget 3 s (the hook's own `-BudgetSec`, hook 50); the fake sleeps 120 s (test line 50). Hook 119-123. Real clock, real process. **Agree Keep.**
42. 'and the hook returns long before the script would have' (:212). `took -lt 30`: no reason for 30. The direct claim is "before the script would have", and the fake's own sleep (120, test line 50) is that number. **Agree Rewrite** (bound by the fake's sleep constant; the record's "budget plus slack" types another number).
43. 'and exits 0' (:213). Agree.
44. 'and batch.py, with the budget spent, is not started and no note is given' (:214). Hook 102 'did not start'. Wording copy. Agree Rewrite in kind.
45. 'and the stopped script''s process is gone' (:218). `taskkill /T` (hook 120) kills the `py.exe` and its python child; the pid is the python process's. Red if the kill is removed (the fake sleeps 120 s). Real mechanism. **Agree Keep.** A reused pid could fail it rarely, as the record says.
46. 'a broken common.py gives a systemMessage naming it' (:225). Hook 173 and 355-359. Agree.
47. 'and exits 0' (:226). Agree.

**The reminder**
48. 'just under the threshold: no reminder' (:232). 499.9 against `$RemovalReminderMb = 500` (hook 45, label only: finding 8). Agree.
49. 'at the threshold: the reminder sentence, with the list path' (:236). Hook 349-350. `-ge` turned `-gt` is red here. Whole sentence retyped. Agree Rewrite (size and path appear).
50. 'the reminder blocks nothing (no decision)' (:237). Agree.
51. 'above the threshold within a day: no second reminder' (:240). Hook 244-248. Agree.
52. 'a day later: the reminder again, with the new size' (:243). 25 h against 24 (label only). Agree.

**-DataDir and -CardsDir**
53. 'with -DataDir the scripts run on that folder' (:253). Red if `--out` is not passed (calls land in the default folder). Real parsers accept `--out`. Agree.
54. 'and the note points there, with --out on each command' (:254). Agree.
55. 'with -CardsDir prune_list gets --cards-root and prepass and batch do not' (:265). Real `batch.py:46` takes only `--out`, so a `--cards-root` passed to it would make argparse exit 2; the claim agrees with the real parser (finding 6). Agree Investigate.
56. 'and the note names that folder and passes it to check, index and link_memory' (:266). Retyped command shapes. Agree Rewrite in kind.

Missing record: the emitted guard check at line 268 (cannot fail, finding 5). Count: 56 of 56 literal `Check` lines have a record.

Tally for h53: 56 records; I agree with 54 verdicts and dispute 2 (27 and 28 in my numbering, at lines 163 and 164: Investigate to Keep). Agreed Keeps: lines 201, 211, 218.

---

# test_h52_cleanup_sweep.ps1 (81 records, 81 checks, plus the emitted guard check with no record)

Shared Q2: registered at `settings.json:87` and `settings.fragment.json:79`; `gates/README.md:65`; `shell_output.py:26` saves trimmed output in the session scratch folder this sweep clears (so the "which sessions run" answer protects it). No caller in `src/`. The record's H-52 says its "reasoned" lines rest on the test's text only; I read the hook, so the Q8 lines below are read against it. Constants (hook 49-58): `StartGapHours = 1`, `WorktreeIdleHours = 24`, `ScratchAgeDays = 7`, `InfoLogDays = 30`. The test's 2 days, 10 days, 40 days, 2 hours and "just now" sit either side of each, so the brackets are real, if loose.

**Setup guards**
1. 'the test could make the worktree junction' (:90). **Agree Keep.** It states "this machine can make junctions", checked by the real call. It is paired with a loud failure; the later checks 176 is skipped when it fails (a silent skip only after a red line).
2. 'the test could make junctions' (:151). Same. Agree Keep (guards 197-199).

**Run 1, worktrees**
3. 'a run with no earlier report prints an empty JSON object' (:167). Hook 713-721, 741-742. Agree Investigate.
4. 'the hook exits 0' (:168). Agree.
5. 'merged, clean and idle: removed' (:170). Hook 250-266: 2 days > 24 h, head is an ancestor of `main`, clean, no link, then `git worktree remove` (hook 293). Real throwaway git repo, read back by `Test-Path` (independent). The record's Investigate treats it differently from 174 for the same kind of fixture. **Dispute: Keep**, the same standard as 174.
6. 'merged, clean and idle with an ignored build folder: removed' (:171). `build/` is in the fixture `.gitignore` (test 34) so `status --porcelain` is empty; `Measure-Tree` counts bytes for the report only. Red if status read ignored files. **Dispute: Keep.**
7. 'a worktree under the hook repo''s worktrees folder: removed' (:172). Hook 174-180: `<root>\worktrees` only counts for the hook's own repo. Red if that prefix goes. **Dispute: Keep.**
8. 'kept: unmerged, untracked, modified, recent, locked, cwd, outside, link' (:174). Hook reasons: `commits not in main` (261), `uncommitted or untracked files` (265), `recent git activity` (258), `locked or prunable` (253), `holds the session folder` (255), `outside the sweep folders` (252), and the link kept by `Measure-Tree` `HasLink` (288). Real git; **Agree Keep**, but the record's Q8 is wrong for three of the eight: `git worktree remove` refuses a locked worktree and one with modified or untracked files by itself (the test's own comment 209-210 says so). If the sweep's own locked, untracked or modified check were removed, git's refusal would still keep the folder and this check would stay green; it is line 213 (the report names no kept worktree) that would go red. For unmerged, recent, cwd, outside and link, this line is the one that goes red. The record's "any one rule removed turns exactly one name red" is true for five names, not eight.
9. 'nothing behind the worktree''s junction is deleted' (:176). Real junction, `keep.txt` read back. Agree Keep. (The same bug turns 174 'link' red first.)
10. 'the sweep leaves the index of a kept worktree alone: unmerged/untracked/modified/link' (:178). `--no-optional-locks` (hook 263) is what stops git rewriting a racy index. Real git. Agree Keep. Q8 note: 'unmerged' returns at the ancestor test (hook 261), before any `git status`, so for that name the named bug cannot turn it red; the other three can.
11. 'the main checkout is untouched' (:180). The record says Rewrite (compare the file set). **Dispute: Remove or Investigate, not a Rewrite worth doing.** No one plausible change turns this red: `git worktree remove` refuses a main worktree itself, the hook's own two guards (251 and 252) both stop it, and the scratch sweep only works in the scratch folder. A file-set compare is just as unable to fail.
12. 'worktrees with a missing folder keep their git records (no prune)' (:182). Hook 253 (prunable) and the missing `git worktree prune`. Real git `worktree list`. Agree Keep.
13. 'the branch of a removed worktree still exists' (:184). Agree Keep.
14. 'git no longer lists the removed worktrees' (:186). Agree Keep.

**Run 1, scratch**
15. 'a stale top-level folder (with a read-only file) is deleted' (:188). Real read-only file; hook 339-342 clears the flag first, red if removed. The 7-day line is only tested by 10 d against fresh. Agree Investigate (the record's reason holds).
16. 'a fresh top-level folder loses its stale child' (:189). Hook 406-411. Agree Investigate.
17. 'and keeps its fresh child' (:190). Agree Investigate.
18. 'the current session''s folder survives at the top, though stale' (:191). Hook 400. Real folder named by the session id. Agree Keep.
19. 'a stale folder holding the current session''s folder survives' (:192). Hook 403-404 (`$holdsSession`). Agree Keep.
20. 'that folder''s other stale child still goes' (:193). Hook 406-411. Agree Investigate.
21. 'a stale loose file at the top is deleted' (:194). Agree Investigate.
22. 'a stale file held open stays, and did not stop the rest' (:195). Real handle with `FileShare.None`; `File.Delete` throws, hook 349-351 records it and goes on. The "did not stop the rest" half is carried by 188-194 (the entries sort after `held.txt` alphabetically: `loose.txt`, `stale-holder`, `stale-top`), as the record says. Agree Keep.
23. 'a stale folder holding a junction is left alone' (:197). Hook 334. Real junction. Agree Keep.
24. 'a top-level junction is not deleted' (:198). Hook 399. Agree Keep.
25. 'nothing behind a junction is deleted' (:199). Agree Keep.

**Run 1, report**
26. 'the report names each removed worktree' (:205). Hook 609-611. The "3 worktrees" is correct (merged, build, claude). Typed phrase. Agree Rewrite.
27. 'the report counts the scratch entries, folders and files together' (:206). I checked the 4: `stale-top`, `fresh-top\stale-child`, `stale-holder\other-old`, `loose.txt` (hook 404, 409). The record's working is right. Hook 615 says "scratch entries (folders and files)". Agree Rewrite (count derived from the fixture).
28. 'the report names the stuck file and the folder with a link' (:207). Hook 334 and 350. Agree Investigate.
29. 'the report gives errors without PowerShell''s wrapper text' (:208). The record calls it a weak negative that passes when no error is reported. **Dispute: Keep.** An error IS reported in this run: the held file's `File.Delete` throws a `MethodInvocationException` whose message begins "Exception calling ..." (hook 350 runs it through `Get-ErrorText`, hook 100-104). If that unwrapping broke, `held.txt`'s entry would carry the wrapper text and this goes red. The negative is exercised.
30. 'the report names no kept worktree (...)' (:213). Hook 282, 295. This is the line that goes red when a sweep's own locked, untracked or modified check is removed (item 8). Agree Keep.

**Run 2 and the start marker**
31. 'the next run prints the last report as a systemMessage' (:222). Hook 715-721. Agree Investigate.
32. 'the report file is gone once printed' (:223). Hook 718. Agree Investigate.
33. 'a sweep started under an hour ago holds back the next one (worktree)' (:224). Hook 688 against `$StartGapHours = 1`; seconds against 2 h at 235. Agree Investigate.
34. '... (scratch)' (:225). Agree Investigate.
35. 'the message blocks nothing (no decision, no hookSpecificOutput)' (:227). Agree Investigate.
36. 'a report is printed only once' (:229). Agree Investigate.
37. 'an old start marker lets the sweep run again' (:235). 2 h typed. Agree Investigate.

**Other repos, junction path, no session id**
38. 'a repo with no local main branch is left alone' (:244). Record Keep. **Dispute: Rewrite.** Without the guard at hook 270-271, `git merge-base --is-ancestor <head> main` on a repo with only `master` fails (exit 128), `Test-WorktreeRemovable` returns `FAIL could not check its commit against main` (hook 262) and the folder is still kept. So the line stays green with the guard deleted; the bug the record says it catches ("unmerged work lost") is stopped by a second check. The check could assert that no failure is reported for the repo (the guard keeps it quiet; without it the report names a FAIL).
39. 'the test could put a junction on the sweep folder' (:258). Setup guard. Agree Keep.
40. 'a worktree reached through a junction is not removed' (:261). Hook 254 and 236-245. Real junction and a real move. Without the check, `git worktree remove` through the link deletes it (red). Agree Keep.
41. 'with no session id no scratch entry is deleted' (:273). Hook 396. Agree Keep.
42. 'and the report says why' (:274). Hook 396 text contains 'no session id'. Wording. Agree Rewrite.

**As registered (background)**
43. 'part 1 returns {} at once and leaves no process holding its output' (:286). Record Keep. **Dispute: Rewrite.** The name promises "at once"; no line asserts it (the stopwatch is only an INFO line, 285). If `-Worker` launch (hook 727-732) were replaced by the foreground sweep, the output is still `{}` (first run, no earlier report), the report is written and the stale folder deleted, so 286, 291 and 292 would all stay green. Only the "no process holding its pipe" half can fail (the harness reports `HOOK OUTPUT NOT READ`).
44. 'the background worker finished within 60 s and wrote its report' (:291). 60 s has no reason; any wait needs a cap, and a detached worker leaves only the file to wait on. Agree Rewrite (state why 60).
45. 'the background worker deleted the stale scratch folder' (:292). Agree Investigate.

**State prune with a concurrent writer**
46. 'the test''s writer was still writing when the sweep ended, so the two overlapped' (:410). Guard on the test's own setup, fails loudly. The log prune is the last step before the report, so overlap at sweep end is overlap with the prune. Agree Keep.
47. 'the test''s writer finished within 120 s' (:412). No reason for 120. Agree Rewrite.
48. 'the sweep with state to prune exits 0' (:413). Agree Investigate.
49. 'a session whose transcript was written lately keeps its stale scratch folder' (:415). Hook 385. Layout matches the real one (finding 7). Agree Investigate.
50. 'a session the registry names, with its process alive, keeps its stale scratch folder' (:416). Hook 370-380. Real process, real `StartTime`, registry fields as in the real record (finding 7). Agree Investigate.
51. 'a registry record whose process is gone does not keep it' (:417). Pid 1073741823 is not a multiple of 4, so no Windows process. Agree Investigate.
52. 'a registry record whose start time does not match (a reused pid) does not keep it' (:418). Hook 377. Agree Investigate.
53. 'a session whose transcript is old loses its stale scratch folder' (:419). Agree Investigate.
54. 'a stale project folder holding a running session''s folder stays' (:420). Hook 403. Agree Keep.
55. 'and its other stale folder goes' (:421). Agree Investigate.
56. 'an old status file goes' (:423). Hook 597. Agree Investigate.
57. 'a new status file stays' (:424). Agree Investigate.
58. 'an ended session''s old agents folder goes' (:425). Hook 598-599, name split at `.` (427). Agree Investigate.
59. 'a running session''s old agents folder stays' (:426). Agree Investigate.
60. 'a fresh agents folder stays' (:427). Agree Investigate.
61. 'an ended session''s old stall records go' (:428). Agree Investigate.
62. 'a running session''s old stall record stays' (:429). Agree Investigate.
63. 'an old posture record goes, a new one stays' (:430). Two claims in one check; agree Investigate.
64. 'a subfolder the prune does not name is left alone' (:431). `session_cards\old.txt` is 10 days old; applying the prune to it turns this red. Agree Investigate.
65. 'refusals.jsonl loses its line older than 30 days' (:434). 40 days against `$InfoLogDays = 30`, with a 10-day line kept. Agree Investigate.
66. 'and keeps the newer, undated and unreadable lines, in order' (:435). Hook 565 (no time or unreadable always stays). I traced the order. Agree Investigate.
67. 'every line written during the sweep is still there' (:437). Hook 476-546 is built for exactly this. Real concurrent writer, real file sharing. One limit the record missed: the writer is a PowerShell loop with `FileShare.ReadWrite` (test 393), not the real appender (`common.py:139` uses `os.open(... O_APPEND ...)` through the C runtime, whose default sharing is read and write, no delete). They approximate each other; the test does not use the real appender. The writer also gives up on a line after 200 tries (about 1 s, test 392-395) and moves on, so a long refusal would show as a "missing" line (a flake, not a bug). **Agree Keep.**
68. 'and none is there twice' (:439). Agree Keep.
69. 'agent_stop.jsonl keeps only its line from the last 30 days' (:441). Agree Investigate.
70. 'reviews.jsonl keeps each key''s last verdict however old (...)' (:444). I traced it against hook 567-583: k1 v1 dropped (a later k1), h1 dropped (10 d, over 7), v2 kept, `K1` kept (exact compare), v3 kept, h2 kept, v4 dropped (later k3), the broken line kept, v5 kept. The test's expected text `v2+vU+v3+h2+rvBad+v5` equals that; it is the rule applied to named inputs, so it passes Q4. The remaining gap is that it never asks the reader (`review_rules._latest_verdicts`); see Splits (copy). Agree Investigate.
71. 'waivers.jsonl is never cut' (:445). Both lines are 100 days old; adding the file to the prune list would drop them, red. Agree Keep.
72. 'the prune leaves no temp file behind' (:447). Hook 508-509 names `.prune-<pid>.tmp` and `.old`. Agree Keep.
73. 'the report counts the pruned state records and log lines' (:451). I checked both numbers: 5 state records (status-old, agents-ended, stall-ended, launch-ended, posture-old) and 5 log lines (ref-old, stop-old, v1, h1, v4). Pinned counts with no working. Agree Rewrite.
74. 'and names no failure' (:452). The record calls it a weak negative. **Dispute: Keep.** The report prints "could not be removed or checked" (hook 623) for any failure, and in this run a failed carry-over or a locked log is a real way to see it (real concurrent writer, 100 retries, hook 455-459). The claim is "no failure"; a marker phrase is how the owner states one.

**Unknown sessions and broken helper**
75. 'a sweep that cannot tell which sessions run exits 0' (:464). Agree Investigate.
76. 'and removes no stale scratch folder' (:465). Hook 367-368, 654-655. Agree Keep.
77. 'and no old state record' (:466). Agree Keep.
78. 'and the report says which sessions still run could not be read' (:470). Hook 655. Wording. Agree Rewrite.
79. 'a broken input helper gives a systemMessage' (:479). Hook 698-705. Wording. Agree Rewrite.
80. 'and exits 0' (:480). Agree Investigate.
81. 'and starts no sweep' (:481). Hook 704 exits before `Enter-SweepStart` (723). Agree Investigate.

Missing record: the emitted guard check at line 488 (cannot fail, finding 5). Count: 81 of 81 literal `Check` lines have a record; lines 174 and 178 are loops that emit 8 and 4 results each from one record.

Tally for h52: 81 records; I agree with 73 verdicts and dispute 8: items 5, 6, 7 (Investigate to Keep), 11 (Rewrite to Remove or Investigate), 29 (Rewrite to Keep), 38 (Keep to Rewrite), 43 (Keep to Rewrite), 74 (Rewrite to Keep). The Keep verdict on item 8 (:174) stands, with the corrected Q8.

---

# Totals

| File | Records | Agree | Dispute |
|---|---|---|---|
| test_status_append.ps1 | 7 | 1 (the Investigate on :46, reason differs) | 6 (all Keeps, to Investigate) |
| test_h53_session_cards_start.ps1 | 56 | 54 | 2 (:163, :164, Investigate to Keep) |
| test_h52_cleanup_sweep.ps1 | 81 | 73 | 8 (above) |

Records not reached: none. Test cases with no record: the three emitted `Assert-LiveStateUntouched` checks (status_append :48, h53 :268, h52 :488), each of which cannot fail by reading. Production findings: `status_append.ps1` reports "appended" after a failed write (finding 4); `status_append.ps1` re-derives the status path that `common.status_path` owns (S1).
