# V06: hand-back text, saved unchanged by the main session (the verifier wrote no file)

Report path: I wrote no report file, because the harness note forbids report .md files and my return value is the report. The full report is below. No file was changed in either repo.

SUMMARY (five lines)
1. Candidates checked: all F06 groups A to J (46 rows; the finder's split list S1 to S9 and its K6 code constants were opened line by line). Not reached: none, but see "Checked lightly" at the end.
2. CONFIRMED: 9 groups. Group A, B (S3 included), C (S1 included), D (S2 included), F (S5 included), G, H (S6 included), I (S7, S8 included) and J, with the narrowings noted below.
3. CONFIRMED-CHANGED: 2. These are S4 (not a split, see below) and S9 (weaker than the finder says). Group E also needs one narrowing.
4. REFUTED: 0 whole groups. S4 is refuted as a split. The pages agree with the code, and the finder had left the merge path untraced.
5. Splits added: 1 (S10, below). The finder's S4 is dropped from the split list, so the net is S1-S3, S5-S10.

Everything below is "by reading". Nothing was run.

CHANGED OR REFUTED FINDINGS

S4 is REFUTED as a split. The finder's two sides are:
- model-postures.md:5 says "a reviewer from a Workflow script leaves no verdict". derive-once-review.md:200 says such a reviewer "gets no such record, so its sign-off counts for nothing".
- gates README:180 says SubagentStop fires with agent_type reviewer for a Workflow-launched agent. agent_stop.py:238 does call the verdict path for any agent_type reviewer.

The finder left the merge path untraced. I traced it.
- agent_stop.py:200-203 looks for a handover record whose prompt_sha256 matches the brief. If none matches, it raises `NoVerdict("no handover record matches its brief (was it dispatched through the Agent tool?)")`.
- Only the PreToolUse "Agent" hook in review_dispatch.py:170 writes a handover. A Workflow launch fires no Agent call.
- README:188 says so in words: a review counts only through the `Agent` tool, and "the user approved that (plan question Q2)".

So the hook fires, finds no handover and records nothing. The pages and the code agree. This stays a K1 copy at most. The only open point is whether Q2 is in the user's words. I did not look it up.

S9 is CONFIRMED-CHANGED, a weaker split. The sides are:
- model-postures.md:11 orders the steps: "nudge it once with SendMessage ... If it does not recover, stop it with TaskStop and re-dispatch".
- stall.py:124-125 and :143-144 tell the main session only "stop it with TaskStop and re-dispatch its remaining work", with no nudge step.
- stall.py:460-461 (the check-in text) says "open its transcript, nudge it or stop it".

So the hook text contains both words, in two different messages. The stuck-agent notice skips the nudge step. The situation where the homes differ is a stuck-agent notice, not the periodic check-in. Kind K3 (the same advice with a changed order). No Confirmed ruling covers it. I found none for "supervis" or "nudge", and the finder searched the same words.

Group E (the main-session test) is CONFIRMED with a narrowing. The finder's "percall, stall and wake" is right, but the tests differ in form:
- percall.py:163 tests `not data.get("agent_id")` only.
- stall.py:421, :433 and :469 test `agent_id or not session_id`.
- wake.py:97 tests `agent_id or not session_id`.

So there are two forms of the same question, one without the session_id test. K6 stands. The edge is a hook input that has neither agent_id nor session_id. percall treats it as the main session, and stall and wake treat it as "not watched". Whether such input exists is open. A run would settle it, so the claim stays open.

NEW SPLIT S10 (missed by the finder)
- The gate's refusal text (ref_judge.py:230-231) says commits "origin already has on one of its branches" are exempt. The code in missing_trailers (:188) is `--not --branches ...known`. So commits already on any local branch are also not judged, which is a third exemption that the text and the preamble do not name.
- This extends S2 rather than contradicting it. The preamble (:65) names one exemption (merges), the refusal text names two (merges and origin-held commits), and the code reaches a third (commits already on a local branch).

CONFIRMED GROUPS, WITH THE HOMES I OPENED

A. Key tools and the key route (I1-001, I1-002). K1, plus K6 on the string "keys".
- percall.py:16 `KEY_TOOLS` and :18 `KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}`.
- game_tools.py:71 `ROUTE_KEYS = "keys"`.
- percall.py imports neither. test_progress.py:628 pins the percall copy to a typed literal.
- No split.

B. Shell tools and the delete rule (I1-018, I1-008, I1-048). K6.
- progress.py:20 `SHELL_TOOLS` is imported by percall.py at lines 47, 112, 133 and 147.
- delete_rule.py:168 has its own literal `("Bash", "PowerShell")`.
- tests\shell_output_sizes.py:20 has a typed copy.
- S3 is CONFIRMED. delete_rule.py:17-18 holds two exact folders, `...hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`.
  - This audit's own worktree is at `...hydra-test\worktrees\tooling-audit-records` (my environment's working directory), outside both.
  - Model-postures:5 ("under a `worktrees` folder (the delete rule trusts only those)") reads as any worktrees folder.
  - The ruling does not settle it. review-and-hooks.md:17 (Confirmed, 2026-10-25) chose "Keep rule, move worktrees" to a question saying "a worktree that sits in a `worktrees\` folder". It names the folder generically and does not name the two exact paths. It names one side, not both.

C. Status, ids and the first-call note (I1-027, I5-062, I5-066, I1-092).
- S1 is CONFIRMED. model-postures.md:11 says "A brief names no status file and no id; it carries only the preamble's status rule." The memory note every-brief-carries-status-rule.md:3 says "the brief must carry the status-file sentence". MEMORY.md says "every brief ... asks for a status line ... via gates\status_append.ps1".
- The Confirmed lines agents-and-workflow.md:25 ("i don't care if an agent is read only, no status updates is unacceptable") and :26 (plain English) say status is required. They do not say how a brief carries it, so they do not name both sides.
- K8 on the memory note is CONFIRMED, with a reading caveat.
  - The note says "the dispatch gate now denies briefs that lack it".
  - review_dispatch.py has no "status" string at all (count 0).
  - settings.fragment.json's only Agent matcher is :158, which routes to review_dispatch, which judges reviewer keys only.
  - test_docs_numbers.py:31 lists `reviewer_dispatch_gate` as RETIRED.
  - The note says "the dispatch gate", not that exact name, so I read it as the same thing. A reader could take it differently.

D. Trailers.
- S2 is CONFIRMED, extended by S10. The preamble names the merge exemption only. See above.
- ref_judge.py:186 reads trailers with `unfold,separator=%x2C`. agent_stop.py:117 reads the Agent trailer with `separator=%x09`. Two code readers of the same trailers (K6 partial). The jobs are different: one checks presence, the other reads identity.
- The preamble's Co-Authored-By line is "Claude Opus 5.5". This session's attribution line names a different model. No hook checks that line (K4), by reading ref_judge's TRAILER_KEYS use.

F. Reviewer type and review key.
- REVIEWER_TYPE is declared in agent_stop.py:20 and review_dispatch.py:16, each separately (K6).
- S5 is CONFIRMED.
  - README:39 says "The hook refuses a brief with no commit".
  - derive-once-review.md:17 and :200 say "A hook refuses a reviewer dispatch with no key".
  - review_dispatch.py:155-159 returns None (allows) when no key is found and `_gated(...)` is false, and denies only for a gated repo.
  - The situation where they differ is a repo with no review marker on main.
- Rulings: review-and-hooks.md:18 and :19 (Confirmed) concern the hand-back format, not the dispatch refusal. They name neither side of S5.

G. Card writers.
- card-writer.md:5 has `tools: Read, Glob, Grep, Write, PowerShell`.
- writer-brief.md:33 limits PowerShell to one command, which only the prose enforces.
- writer-brief.md:101 states "20 minutes or 100 tool calls". It is a typed copy of progress.py's numbers (K1).
- The two pins I read are test_docs_numbers.py (Hydra docs only) and test_agent_files.py. Neither opens writer-brief.md.

H. Models and tools of the agent files.
- S6 is CONFIRMED. scout.md:4 is `model: sonnet`, and rule 2 (CLAUDE.md) and model-postures:5 both say "Sonnet or Haiku".
- scout.md:5 lists Edit and Write in `tools:`, while scout.md:3 says "read-only", and :9 says "Change no files except your status line and files in your own scratch folder." The prose is the only enforcement of "read-only" (K5/K7).
- The executor, planner, reviewer and scout files carry the identical ten-tool line. The card-writer has a five-tool line. All five have `disallowedTools: Agent`. Typed four times (K1).

I. Parallel work, worktrees, supervision.
- S7 is CONFIRMED.
  - lean-agent-fanout.md:21 says "use 3-4 agents" for repeated discovery rounds. MEMORY.md:39 says "3-4 agents per round" and drops the "discovery" qualifier.
  - agents-and-workflow.md:21 (Confirmed, 2026-10-03) says "you dont need to stay limited to 10. run as many as you can."
  - The memory note's scope is discovery rounds, a narrower thing. The index line is looser. Whether the Confirmed line covers discovery rounds is for the main session. Memory notes are not the user's words.
- S8 is CONFIRMED.
  - model-postures:5 names the task id, session id, owned files, tests and the status rule.
  - Preamble :3 names the task id, session id, owned files, worktree or checkout, scratch folder and tests.
  - Each omits what the other names. S1 and S8 overlap: model-postures:11 also bars status-file and id text.

J. Reviewer guides and the handoff sentence. CONFIRMED as K1. I did not count the four copies of the handoff sentence myself. I saw it in model-postures:13, the preamble :95 and CLAUDE.md rule 9, which are three. I did not open agent-rules.md for the fourth.

Checked lightly, so these claims stay open
- Group C's three id rules (progress.py, common.check_id, status_append.ps1): I did not open the code for them.
- Group G's three transcript-location homes (stall.py:175-185, agent_cost.py:160, prepass.py:250-253): I opened none of them. The `_TASK_ENDED` regex in agent_stop.py:51 is confirmed.
- Group I's I5-107 cross-repo text and the "Claim, plus disagreement" quote in a memory note: not opened. The finder's own search found no matching Confirmed line.
- The finder's test claims (test_docs_numbers.py, test_agent_files.py coverage) rest on its read. I opened only the RETIRED line.

Hook gap, in general words only: nothing checks that a page's wording matches a hook's rule. S5 and S4's earlier wording show the gap.

Confirmed rulings, with file and line:
- review-and-hooks.md:17 (delete rule).
- review-and-hooks.md:18-19 (hand-back).
- review-and-hooks.md:30 (git-side gate).
- agents-and-workflow.md:12 (Haiku for transcript readers).
- agents-and-workflow.md:17, :20, :21, :23 (parallel work).
- agents-and-workflow.md:25-26 (status).
Both headers are "## Confirmed" (agents-and-workflow.md:7, review-and-hooks.md:7). None names both sides of any split S1-S3 or S5-S10.