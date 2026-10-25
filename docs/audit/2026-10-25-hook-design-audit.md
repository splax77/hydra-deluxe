# Hook design audit (2026-10-25)

Why does almost every session hit a new hook bug? This audit answers that from three sources. The first is the 204-commit history of `C:\Users\Patrick\.claude`. The second is the session cards from August to October. The third is every transcript from 24 October, local time: 18 main sessions and their subagents. The hooks are about 30 registered gates, 13,340 lines of PowerShell (`wc -l hooks\*.ps1 hooks\lib\*.ps1`). Nothing was changed while writing this.

## The short answer

The hooks are not breaking at random. Four habits in how they are built produce new bugs at a steady rate, and each fix tends to add to the problem.

1. **The review gates rebuild who wrote what from git history after the fact.** They don't record it at the moment it happens. Every new way of working creates a history shape the rebuild has never seen. Past examples are a CRLF review file, a merge commit, a change in a second repo, and today's reviewer that fixed in two commits.
2. **New gates create new real-world shapes for old gates.** On 24 October the new reviewer dispatch gate started making doc reviewers commit their fixes on a branch. That is correct, but no reviewer had done it before. The old submit gate had only ever seen one fix commit, so it broke on the second one. Neither gate was wrong on its own; they had never been tried together.
3. **Most fixes add a special case instead of removing a cause.** About 35 of the 47 catalogued bugs were fixed with "handle one more shape". Every case added makes the gates bigger, so the next session has more ways to hit an untested combination.
4. **The tests use hand-made inputs, so real shapes are first seen live.** The one exception proves the point. The shell scanner was replayed against 3,696 real past commands and is now the quietest part of the system. No other gate gets that treatment.

## What happened on 24 October

The hooks refused 52 tool calls across today's sessions. Each one was counted from the transcripts with the extractor in the session scratchpad (`hookmine.py`; output `hook_events_1024.txt`). Most of those refusals were the hooks doing their job.

| What the hook refused | Count | Verdict |
|---|---|---|
| `sed -i` edits (11 by agents, 1 by a main session) | 12 | Right. The preamble forbids it. |
| Python patch scripts writing source files | 13 | Right. The preamble forbids it. |
| `python3`, `git merge --trailer`, `git checkout -B` | 5 | Right. Each is a documented rule. |
| `git -C $variable merge` or `cd x && git merge` by a main session | 7 | Safe refusal. The hook can't read a folder held in a variable. The same main-session habit came up in 6 different sessions. |
| Doc-review helper not run alone in its call | 3 | Safe refusal, same reason. |
| `hydra_batch` with no folder, or with folders in variables | 3 | Right, or safely cautious. |
| Recursive delete of a scratch folder held in a variable | 1 | Safe refusal. A real scratch path, but the hook can't see that. |
| Status line overdue; 30-minute hard stop (twice) | 3 | Right. |
| Merge the ref gate would refuse; reviewer dispatch with a "not committed" brief; two deliberate live tests of the dispatch gate | 4 | Right. |
| **Doc reviewer refused for fixing in two commits** | 1 | **Bug.** This is the handoff's bug. |

So one refusal out of 52 was a real bug. 30 were agents doing things the preamble already tells them not to do. About 14 were the hooks refusing text they can't read, to be safe. The cost of the bug was larger than its count suggests, though, because of what else went wrong around the review gates today:

- A derive-once reviewer wrote "SIGN-OFF: clean" and never ran the submit helper. The merge was refused later, and the user waived the review (`waive derive-once d8f8d8d83f`, session b53fbac5). The same thing happened in session 66725311, which had to send a second reviewer. Commit de4776c added a new gate to flag it.
- The reviewer dispatch gate could not judge a hook change in `~\.claude` from a Hydra session. A session stalled until the user ruled, then commit e6ef37e fixed it.
- The two-commit bug blocked the ASan plan, and the user waived its doc review (`waive doc-review b300fb39...`, session bc2849d9).
- The review-gate area got 9 hook commits today: two new gates and seven fixes, with one bug still open.

Both of today's waivers came from review-gate friction. Neither came from wanting to skip a review.

A reviewer in session 6fc3d247 hit the 30-minute hard stop at "95 minutes". That was not a fault: the computer was asleep between 19:31 and 20:58 UTC (the user's account). The progress gate counts wall-clock time, so a sleep counts against an agent's budget.

## The history in one table

The bug catalogue covers 47 significant bugs from mid-September to today. One scout built it from commit bodies and session cards, and the commit hashes are in its report. Each bug got one main cause.

| Cause | Bugs | What it looks like |
|---|---|---|
| Re-reading free text and getting the shape wrong | about 14 | Shell commands, review files, briefs, Workflow scripts. Each fix covers one more phrasing. |
| Something bad let through, or failing open | about 9 | Usually found by an audit, not by a test. |
| Windows, PowerShell, encoding or process starts | about 8 | Code page 437 input, BOMs, CRLF, slow process starts, the hook server. |
| Legitimate work refused | about 5 | Derive-once carry (refused 3 times in 5 days), doc-review submit (4 times). |
| Two hooks, or a hook and a doc, disagreeing | about 4 | Preamble vs progress gate, dispatch gate vs submit gate. |
| A test that didn't test the real thing | about 4 | A test wrote to live state; a test copied the gate list by hand. |
| Shared state and identity | about 3 | A counter reset by parallel calls; another session's agents relayed. |

Three areas account for most of it. First, the shell-text gates (risky, scratch, suite, precheck, patch script, media). Second, the progress and status gate, which has been quiet since 11 October. Third, the review gates (derive-once and doc-review, and now dispatch and handback), whose bug count is still going up.

The 10 October overhaul's own fixes held. Failing closed, putting the folder under git, retiring four gates and counting status in minutes have not come back. But that day shipped about 90 commits, and many of them fixed the overhaul's own new code. The overhaul also left the review gates alone, and they are where today's bugs are.

## Why the review gates keep breaking

`Get-DocReviewSubmitAuthorRefusalIn` (`hooks\lib\doc_review_rules.ps1`, line 388) shows the pattern. To decide whether a reviewer wrote the text it reviews, the gate looks up every commit in every branch that adds or removes that text's blob. It reads each commit's `Agent:` trailer. Then it excuses exactly one commit: the one that adds the final text. A reviewer that commits twice leaves a middle commit that adds and removes neither the handed blob nor the final one in the way the rule expects. The gate counts the reviewer as an author and refuses.

The real fact is simple: "the main session handed key K to reviewer R". That fact exists at dispatch time. The dispatch gate even reads the key out of the brief. But nothing records it, so the submit gate has to guess it later from git. Every change to how people commit gives that guess a new shape to get wrong. Derive-once carry has the same structure, and it got three such fixes between 10 and 15 October.

## Other weak points the design map found

- **One rule, several copies.** "Is this agent a reviewer?" is decided three ways: a 10-option regex on the first message (`agent_progress_gate.ps1`, line 77), `Test-DeriveOnceReviewer` (`review_handback_gate.ps1`, line 79) and `Test-DispatchIsReview` (`reviewer_dispatch_gate.ps1`, line 205). A doc reviewer counts as a reviewer to the first and not to the second.
- **Docs drift from the code.** The preamble says "commit before your 110th tool call", but the code warns at 100 and stops at 150. The preamble also says "every 10 tool calls or 5 minutes", but the gate counts minutes only.
- **Gates that predict other gates.** The dispatch gate and the merge precheck both predict what a later gate will say. They share libraries, which helps. Still, a new refusal reason has to be added to both, or the prediction is wrong.
- **Identity from folder layout.** "Which agent is this" comes from the `agent_id` field, which is solid. "Which transcript is this agent's" and "what is its brief" come from folder arithmetic on Claude Code's own transcript layout. Workflow agents are special-cased everywhere for that reason.
- **No test at all:** `no_background_in_subagent_gate.ps1` is registered and live, and no test file mentions it.
- **Live means everywhere at once.** A merged hook is live in every running session on the next tool call. On a busy day, several sessions build and merge hook changes while depending on the hooks.

## What would stop the stream

These are ordered by how much of the bug stream each one cuts. None of them is built. Each one needs the user's yes before it starts.

1. **Record facts when they happen, then check the record.** At dispatch, the dispatch gate writes "key K handed to agent R, base B, by session S". At submit, the gate checks the reviewer against that record and doesn't dig through git. Any commits the reviewer makes on its own branch after dispatch count as fixes by definition. That removes today's bug and the whole family behind it (multi-commit, CRLF, merges, second repo). The same idea fits derive-once carry.
2. **Shadow mode for every new or changed deny rule.** The rule first logs "would have refused" for a few days without blocking. Then someone reads the log against real sessions, and only after that may it deny. Today the dispatch gate's side effect reached the submit gate on its first live day. In shadow mode it would have reached a log.
3. **Replay real calls before merging a gate change.** The extractor used for this audit already pulls every real tool call out of the transcripts. A replay harness would run the last two weeks of real Agent, Workflow, Bash and PowerShell calls through the old and the new gate, and show every decision that changed. The shell scanner got stable this way. It should be the merge test for every gate, not just that one.
4. **Give the main session helpers instead of parsing its commands.** Today's 7 "can't read the folder" merge refusals and 3 helper refusals came from main sessions writing the same command shapes. A `merge_into.ps1 <repo> <branch>` helper, like `status_append.ps1` and `doc_review_submit.ps1`, gives the gate one fixed shape to read.
5. **One owner per rule, with a test that the docs agree.** Put the reviewer check in one function, and put the numbers (100, 150, 20, 30, 5, 10) in one file. Then add a docs-vs-code style check that the preamble and CLAUDE.md quote the same numbers.
6. **A two-week freeze on new gates, then prune.** Count refusals per gate over the last two weeks from the transcripts. Retire gates that never fire or only ever refuse by mistake. Measured today, most refusals come from a handful of well-understood rules, and the bugs come from the newest gates.

The handoff's narrow fix still makes sense on its own: excuse every reviewer commit on the path from the handed text to the final text. Point 1 would later replace it, along with the git digging it patches.

## Sources

- Bug catalogue (47 rows, commit hashes and card ids): HOOK-AUDIT-A scout report, this session.
- Design map (every registered hook, its parsers, chains and identity sources): HOOK-AUDIT-B scout report, this session. Three of its claims were checked against the code here: the missing test, the three reviewer checks, and the 110/100/150 numbers.
- Today's refusals and remarks: `hookmine.py` run over every transcript with activity between 2026-10-24T04:00Z and 2026-10-25T04:00Z, in this session's scratchpad.
- The two-commit bug: `docs/handoffs/2026-10-24-doc-review-multi-commit-fix-handoff.md`, and the code at `hooks\lib\doc_review_rules.ps1`, lines 388 to 446.
