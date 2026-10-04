# Audit slice "merge-gate": session a9a461da (2026-10-04 13:24 to 20:08 UTC)

Read-only audit of the session that made the derive-once review gate live and took step 1, step 2 and T10/T11 through it. Numbers come from `metrics/agents.csv`, `metrics/commands.csv`, the coordinator's `timeline.py` logic re-run per agent, and the agent transcripts. Times are UTC. Agent ids are the 17-character transcript ids.

## The short version

The three gates took 257 minutes of wall clock and 14 review rounds (step 1: 7 rounds, 157 min; step 2: 4 rounds, 60 min; T10/T11: 3 rounds, 40 min). The gate branch itself needed 6 more rounds (85 min) before that, and D36 is on its second round now. The 14 rounds found 51 things. About 29 of the 51 are kinds a script can find (copied test helpers, numbers with no decision, tests that re-do production arithmetic in a recognisable spelling, source scans outside the one scan file). Six were new copies the fix rounds themselves introduced, seven were findings a fix round said it had fixed but had only partly fixed, and four were old copies that earlier reviewers in the same chain had missed. Mid-review user rulings (D40-D43, D45 and its two addenda, D47, the depth-40 and 120-window limits) generated new code each time, and each piece of new code bought another round.

Model time, not tools, dominates the gate work. Fourteen reviewers spent 72 minutes waiting on or writing model output and 15 minutes in tools. Seven fix agents spent 60 minutes on the model and 34 in tools. The model time is not deep thinking: no transcript in this slice stores a single thinking block, and the per-turn latency is a steady 2.8-3.7 seconds. It is volume: 80-280 turns per agent with 100-220k tokens of context behind every turn, plus long outputs (the first step-1 fix agent wrote 163k characters of edits and reports). Fewer turns and shorter reports are the lever, not a different model.

Idle time is the third bucket: 96 minutes of agents parked after finishing a turn. Eleven minutes of that was the user answering a question. About 20-27 minutes was the coordinator losing track of a parked reviewer across a context compaction, and that was on the critical path. The remaining 55 minutes were fix agents kept warm while a re-review ran, which is not lost wall clock but twice pushed an agent over its 150-call budget so a fresh agent had to re-read everything.

What would get a branch to CLEAN in one or two rounds: run the mechanical checks before the first review, make the first fix round sweep the whole range by kind instead of fixing the listed instances, ask the user every pending number question in one batch before the review starts, and never let a reviewer park.

## 1. Where the wall clock went

The slice has 80 agent transcripts plus the main session. Agent wall time totals 19.0 hours: tools 9.5 h (50%), model waiting for first token 5.8 h (31%), model writing 1.8 h (10%), parked-then-resumed 1.6 h (8%), other 0.2 h. The main session ran 393 minutes with 431 tool calls; its biggest own costs were 14 user questions (16.2 min open), 15 builds (9.3 min), shell glue (6.8 min), 4 uitest runs (5.7 min) and 37 git calls (3.6 min).

The five most expensive agents, by wall time:

The UTF-8 hooks fixer (aa6df6a114121c641, 86.6 min) spent 70 minutes in tools: 26.6 min polling, 18.4 min in read/search shells and 16.8 min in tests, which were four full hook-suite runs the brief asked for. This is the known lesson in `no-repeat-suite-runs.md`; the user asked about it twice (14:56, 14:58).

The D46 hooks follow-up (a070469054a700803, 58.1 min) repeated the pattern on a smaller scale: 19.6 min polling and 16.7 min of shell runs of the hook suites, plus a pause for the user's "fail loudly" ruling at 16:27.

The D39 open-file size helper (a7f128014d94a397e, 56.3 min) was a 22-minute job kept alive for 56. It finished at 14:22, then got extra scope three more times (14:30, 14:46, 14:51) as each of the gate branch's reviews found one more thing in the scan test. It sat idle 22.5 min between those sends, waiting on reviews.

The first step-1 fix agent (ab15d1f9a7b3ca7ee, 50.8 min) used only 6.2 min of tools. It was idle 26.4 min (2 min on the user's D45 ruling, 24 min parked after 15:58 while the D42 agent and review round 2 finished), and spent 12.8 min waiting on the model plus 4.9 min writing, over 276 turns with an average of 202k tokens of context per turn. It ended at 16:25 past the 150-call budget with uncommitted, non-compiling edits.

The "fix three step-1 findings" agent (a2d45a3953c4eb65d, 50.0 min) built 17 times for 18.7 min, roughly one build per edit, and got a second job mid-run (diagnose four deterministic test failures, sent 14:05).

Review rounds as a group: the 14 reviewers cost 113 minutes of wall time, 27 of it the round-1 reviewer parked. Their four helper readers (a856a4c2d66e1c891, a7e5e20833a0b83c1, a407e2680c5207a23, a819e69b44b6fb2b4, a230e300f11ba91c2: 7-10 min each, 100-220k context each) added 36 minutes of model time to read the same tree in five slices.

## 2. Why so much model time

Three facts explain it. First, there is no extended thinking in these transcripts: every agent in the slice has zero thinking characters stored. Second, per-turn latency is flat and small: the step-2 reviewer (a0160824e895534bc) waited 7.1 min over 151 turns, 2.8 s each; the first step-1 fix agent waited 12.8 min over 276 turns, 2.8 s each; the round-1 step-1 reviewer waited 5.0 min over 82 turns, 3.7 s each. Third, every one of those turns carries a large prompt: average context per call is 125-210k tokens for reviewers and 115-225k for fix agents, peaking at 289-316k. So model time is (turns × ~3 s) + (output characters / ~550 per second). The fix agents each wrote 43-163k characters; the reviewers 15-66k.

Cold re-reading is part of the turn count. Each reviewer in a chain re-reads the same files from scratch (27-62 read/grep calls each, see §4 table), and the four round-1 helpers each re-read the core tree. The known lesson in `lean-agent-fanout.md` applies and still happened here: round 1 of step 1 used five readers instead of one reviewer with a scope list.

## 3. Avoidable time

Repeated builds: the "fix three findings" agent built 17 times (18.7 min). The main session built 15 times (9.3 min), once after each re-merge of s1-final into s2-final (16:13, 16:36, 16:54, 17:13, 17:35). Every re-merge rebuilt and re-ran the full suite in the main checkout; three of those full-suite runs (15:14, 16:15, 16:55) took 16-36 s each, so the suite itself is cheap here. The cost of the rule breach was CPU contention on a shared box, not minutes.

The full-suite rule failed after it was stated. The user said it at 14:16 ("running the suite 10 times is unnecessary"), again at 14:56-14:58 (UTF-8 agent), and the coordinator still wrote "then the full hydra_tests suite ONCE" into the step-1 fix briefs at 16:44 and 17:02 and the T10 fix brief at 18:46, and into the D36 brief, which drew the 19:19 correction "reiterating a rule you ignored". The rule failed because the coordinator read the 14:16 correction as "no loops" and the 19:19 one as "no full suite"; the briefs were templated and the template was not changed. Fix agents in the three gates ran the unfiltered suite 1-3 times each (s1-fix1 once, s2-fix1 twice, t10-fix1 three times, per `commands.csv`), 3-20 s each.

Parked agents: 96 minutes idle. The round-1 step-1 reviewer (aaaa6ecadb91b1094) launched five helper readers, ended its turn at about 15:02, and was not woken until the coordinator's SendMessage at 15:29; the helpers had all finished by 15:10. In between the main session hit its context limit (15:12 summary), committed docs, built, ran the suite, launched the T10 fix, and took the "why two fix agents" question. Those 19-27 minutes were on the critical path of step 1. The other parked time (ab15d1f9a7b3ca7ee 24 min, af62f119ffdef7589 7 min, a7f128014d94a397e 22.5 min) was agents kept warm while a review of their work ran; not lost wall clock, but two of them (ab15d1f9 at 16:25, af62f at 18:23) then ran out of calls on the next message and had to be replaced by a38f4edd1fa88e5b1 (8.4 min) and a8cbd4fd04634a4cf (4.0 min), each starting cold.

Double dispatch: the T10 fix agent a274d20e045de088c (15:19) duplicated the wave-2 workflow's own fix agent; the user caught it at 15:23 and it was stopped after 4 min of reads. Known lesson, recorded.

Workflow kept alive: cleanup-wave-1 was still running at 15:00 after every task had reported; the user said "stop that workflow. it's already finished and is burning time for nothing". Its Monitor also expired twice (14:04, 14:35) and had to be re-armed.

Stale base: T10 was built by the wave-2 workflow from the step-1 head of 14:27 (a0da9b07ee96cf8db), before the step-1 fix rounds added `last_clamp_tick` (round-2 fix, 16:25). Its `Path::clamp_tick`/`clamp_from` fields were a second copy of that owner by the time it was integrated at 17:37, and the integrator (afc3fa11653ba2279) was not asked to check which owners the base had gained since the fork. That one finding (T10 round 1, #6) drove the longest T10 fix item: engine change plus 7 minutes of before/after dumps on 48 chart sets (af664e418119097ff, run/compare 7.0 min).

## 4. The review rounds

The table lists every round for the three gates. "Interval" is minutes from this review's start to the next review's start (or to the CLEAN verdict for the last round). Kinds: A = a rule written twice in production code; B = a test that recomputes what production computes instead of pinning a literal; C = a copied test helper or fixture; D = a number or rule with no user decision, or a doc or comment stating the rule differently; E = a source scan outside `test_single_owner.cpp`, or a scan row that lets copies through.

| Round | Reviewer | Start | Interval | Verdict | Findings by kind | What filled the interval |
|---|---|---|---|---|---|---|
| S1 R1 | aaaa6ecadb91b1094 | 14:58 | 74.8 | 14 findings | A 7, B 3, C 1, D 2, E 1 | review 35.5 min (27 parked); fix ab15d1f9 15:34-15:58 incl. user ruling D45; D42 agent acf0215d5b74f34f9 15:53-16:11 in parallel; main merge+build 16:12 |
| S1 R2 | a5666a9135bd07c67 | 16:12 | 24.0 | 6 | A 2, B 1, D 1, E 2 | review 8.8; fix agent resumed 16:22, over budget 16:25; replacement a38f4edd1fa88e5b1 16:25-16:34; user question (120 windows) 16:34 |
| S1 R3 | a70d9cf0cd5471423 | 16:36 | 17.9 | 4 | A 1, B 1, C 1, D 1 | review 6.8; user question (variants, D45 addendum) 16:43; fix a4128874bc0c61b76 16:44-16:54 |
| S1 R4 | a59da2774108cfbf7 | 16:54 | 19.2 | 4 | A 1, B 2, C 1 | review 7.3; fix + sweep ac3593f17fc515ece 17:02-17:13 |
| S1 R5 | a271d755bc840421a | 17:13 | 7.6 | 2 | C 1, D 1 | review 5.2; user question (depth 40) 17:19; main fixed it itself |
| S1 R6 | a8555ecf63263d2b4 | 17:21 | 9.7 | 1 | A 1 | review 7.4; main fixed it itself |
| S1 R7 | a560606c96a1d97b9 | 17:31 | 3.5 | CLEAN | - | merged to main 17:35 |
| S2 R1 | a0160824e895534bc | 17:36 | 39.0 | 9 | A 3, B 1, C 4, E 1 | review 11.1; fix af62f119ffdef7589 17:48-18:03; user question (note order, D47) 18:03-18:12; fix resumed to 18:15 |
| S2 R2 | a66757aa5cf0feee4 | 18:15 | 12.8 | 2 | C 2 | review 6.1; fix agent resumed 18:22, over budget 18:23; replacement a8cbd4fd04634a4cf 18:23-18:28 |
| S2 R3 | ab98e3e9d19494b22 | 18:28 | 6.4 | 1 | A 1 | review 4.3; main fixed it itself |
| S2 R4 | a94b513cd6ed4db41 | 18:34 | 1.4 | CLEAN | - | merged 18:36 |
| T10 R1 | afb48d53aff791f3f | 18:38 | 30.3 | 7 | A 1, B 1, C 4, D 1 | review 7.6; fix af664e418119097ff 18:46-19:08 (7 min of dump comparisons) |
| T10 R2 | a983e37618d323aa5 | 19:08 | 6.3 | 1 | B 1 | review 4.8; main fixed it itself |
| T10 R3 | a1aaf4f32df82dc58 | 19:14 | 3.0 | CLEAN | - | merged 19:18 |

Totals: 51 findings. A 17, B 10, C 14, D 6, E 4. Review agents' own wall 113 min; fix agents about 100 min; user questions during the gates about 12 min of open wall; the rest is main-session merge, build and launch glue.

For context, the gate branch before it went live took six rounds of the same shape (a3ef16a51183d6635 13:31, ac877a31bb0f4c975 13:55, a0e13ef77bb521ba3 14:21, a947326fe09e46ff0 14:39, a9bfcc2eba8acb1d5 14:48, aacf0afa9b8cca7af 14:53 CLEAN): 5+1+2+1+1 findings, nearly all E-kind bugs in the scan test itself (copy of an owner line passes, a rule written twice in the scan loop). D36 (a2e6ddb70767d5389 19:46, 10 findings: C 4, A 3, B 1, D 2) is on round 2 (a90ea243359baf6a5, one comment finding).

### What each kind was, and what would have caught it before review

Kind C, copied test helpers (14 findings, in 9 of the 11 non-clean rounds): `collect_variants` in three files, `deact_edge_at` copied, the MIDI delta writer, the `.srb` fixture, the leaderboard fixture, the minimal `.chart` writer in four places, hand-written path walks four times, the "one step per SqIn" check three times, two tied-variant loops the sweep missed. Three of the step-2 ones were already audit findings (117, 277, 286). A script catches every one of these: list function names defined in more than one file under `tests/`, and normalise bodies to catch renamed copies. Nothing in the brief would have been as reliable, because three different reviewers each missed some of them and the round-4 sweep missed two more.

Kind D, numbers and rules with no decision (6): numbers in code comments, test floors, a test depth of 40 citing a decision that does not contain 40, `keep_losers()` with an unrecorded depth 50, ADR 0014 not updated for T10's fold exception. A script that lists every numeric literal added by the diff and checks the cited decision text contains it would have produced the D40/D43 question list before the first review instead of across three rounds.

Kind E, scans outside the one scan file and loose scan rows (4): three source scanners beside `test_single_owner.cpp`, a guard left outside after round 1, a `sqout_tick` row that let more through than the walker it replaced, a second test reading the source tree in step 2. The round-2 fix added the row that catches the first two; a rule that every new scan row ships with its must-match and must-not-match lines (the brief already asks reviewers for them) would have caught the loose row.

Kind B, tests that recompute (10): `timecode(A).ms() - timecode(B).ms()` for an offset, `== 1.0` instead of `is_scaled`, a typed 500, rebuilding the plain SP end (a known audit finding, 156), re-counting tied paths, re-running the targeted search's own filter. The spellings repeat across branches: the offset subtraction appeared in step 1 round 1, again in T10 round 1 (where the scan row was too narrow to see the `timecode(` spelling) and again in T10 round 2. A grep for those spellings in `tests/` plus the brief line "pin literals from one run; never compute the expected value" would have removed most. The two that would not be caught mechanically are design-level (the lone-pricing oracle accepting a promoted path), which came from new D45 code.

Kind A, production copies (17): seven in round 1 of step 1 (the graph's new "after the SP end" test, the n-th SqIn pairing, `squeezed_in` beside `sqin_phrase_ticks`, `is_e0` restated, a `sqout_position` copy, the codec's last kind, the band's second value), then one or two per round after that. Six of the later ones were introduced by the fix rounds themselves (the clamp lookup from the round-1 fix, the promotion code from the D45 ruling, the "shares nothing with its new parent" rule from the round-3 fix) and one (the early-fill offset, round 6) was old D38 code five reviewers had passed. This is the review's real job and a script cannot do it. What would have shrunk it: the implementer brief asking for the reviewer's own step 1 ("list the questions your diff answers, grep for each by inputs, not names") before committing, and the fix brief requiring a grep proof per finding that no third copy remains, since three findings (the SqIn count, the scan guard, the `.chart` writer) came back the next round as "still in N places".

### Why 14 rounds

Each round found a smaller set, but the sets did not shrink to zero for four reasons that stack.

The first review found 14 things and the fix was split across a user ruling (D45) and a 150-call budget, so round 2 reviewed a half-finished round and found the leftovers plus two new copies the fix had written. Rounds 2 to 4 were driven by D45 and its two addenda: each ruling added production code (filter, promotion, placement) and a test, and each addition carried a new copy or an unpinned count. Rounds 4 to 6 were fed by old copies no earlier reviewer had named (two test copies in round 4, two loops in round 5, one engine copy in round 6): fresh reviewers read different files first, so each pass had a different blind spot. The sweep that would have ended this came in round 4's brief ("then sweep for more of the same kind") and folded nine more copies, but still missed two loops.

Step 2 and T10 were shorter because their round-1 findings were mostly kind C and a script-shaped list, but each still needed a leftover round (an agent over budget; a scan row too narrow) and a one-finding round.

### What gets a branch to CLEAN in one or two rounds

Run the mechanical checks before the first review, on the branch, by the implementer or a 2-minute script agent: duplicate test helpers, new literals against decisions, the recompute spellings, scans outside the scan file, every new scan row with its two example lines. On this data that removes about 29 of 51 findings and three whole rounds (S1 R5, S2 R2, T10 R2: 27 min) outright, and shrinks S1 R1 from 14 findings to about 7, which one agent can fix under budget.

Collect every pending user question before the review, in one AskUserQuestion: the literal check produces the D40-D43 list; the plan's open behaviours (D45) are known from the branch's own TODO comments. Six questions were asked during these gates, one per round, and each answer spawned code that needed a round.

Make the first fix round a sweep by kind over the whole range, not a fix of the listed instances, with a grep proof per finding that no copy remains. Split it into two parallel agents (production findings; test findings) each capped at 100 calls and committing per finding, so no agent runs out mid-round.

Give the reviewer the recurring-kinds list and the scan output, so the reviewer's pass is spent on kind A. Memory `derive-once-gate-2026-10.md` records this lesson; `docs/agents/derive-once-review.md` (still at 3551b5e) does not yet contain it.

Reviewers never park: no helper readers, or helpers in the foreground. The 27 lost minutes in S1 R1 were a third of that round's interval.

## 5. The user's corrections

14:16 "running the suite 10 times is unnecessary and hogging CPU": the flake agent a0d00ecf0b9fa564d looped the full suite under 16 busy-loop shells. Known lesson; its briefs now say targeted test only.

14:56 and 14:58 "how close is utf 8? ... 80 minutes ... insane": the UTF-8 brief asked for a baseline of every hook test file and a final run of every file. Known lesson.

15:00 "stop that workflow. it's already finished and is burning time for nothing": cleanup-wave-1 kept running after its last result. Rule: the script exits on its last result, or the coordinator stops it the moment the last Monitor event lands.

15:23 "why did you launch two fix agents for t10?": double dispatch beside the wave-2 script's own fix step. Known lesson, recorded.

16:27 "agents should fail loudly. freezing is unacceptable": D46 had been implemented as fail-closed "ask". Known lesson.

17:44 "new rule: if anything changes a display in the app or a record, run it by me. if it only changes code you can use your recommendation": came after the fifth AskUserQuestion of the step-1 gate (numbers, T10 land, 120 windows, variants, depth 40). Rule recorded in memory `confirm-before-coding.md`; enforcing it is a brief line that classifies each question as display/record or code-only before asking.

18:42 "no other sessions are active. clean up those worktrees": the coordinator hesitated to remove worktrees. Rule: ask once at session start whether other sessions share the checkout, then act.

19:19 "reiterating a rule you ignored: agents do not run the full test suite": the rule existed from 14:16 and still failed because the fix-brief template kept "full suite ONCE". Enforce with a hook that denies an unfiltered `hydra_tests.exe` run from a subagent, or by editing the template once.

19:28-19:29 "how long until d36 is done? what is it doing?" and "run all the library runs in parallel": the D36 agent ab2932d4d4e6eeebe ran 24 library dumps serially (18.3 min of shell time). Known lesson in `parallel-by-default.md`; it failed here because the brief said "one job at a time" to protect timing numbers, and the agent applied that to score comparisons too. The brief should separate "timing runs alone" from "comparison runs together".

## 6. Speedups for the next phase, with estimates

Mechanical pre-review checks (duplicate test helpers, literals vs decisions, recompute spellings, scan placement, scan-row examples) as a script the implementer runs and the reviewer re-runs. Saves about 90-120 of the 257 gate minutes on this data (three rounds gone, the rest shorter, round-1 fixes under one agent's budget). Enforce: the script is a test case in `test_single_owner.cpp` or a `tools/` script named in the implementer brief and the review brief; the submit helper refuses a review whose file does not quote the script's output.

Batch user questions before the first review. Saves one round per question, about 10-20 min each (S1 R3, R5 and S2 R1's second half were single-question rounds). Enforce: the implementer brief ends with "list every number and behaviour without a decision; the coordinator asks them all in one question before launching the review".

Sweep-by-kind in fix round 1, two parallel agents, 100-call cap, commit per finding. Saves the leftover rounds (S1 R2's half-round, S2 R2, both replacement agents: about 25 min) and the cold re-reads after two budget exhaustions. Enforce: fix-brief template; the budget hook already exists.

Reviewers in the foreground, no helper readers. Saves 27 min on the critical path (S1 R1) and 36 min of helper model time. Enforce: the review brief says "no Agent tool use", or the dispatcher launches reviewers with a tool allowlist that omits Agent.

Fresh-base check for integrators: before integrating a branch built on an older head, list the owner helpers the base gained since the fork and grep the branch for their questions. Would have removed T10 R1 #6 and its 21-minute fix round. Enforce: an integrate-brief line plus running the scan test (with the rows added since the fork) on the branch first.

Unfiltered test runs denied in subagents. Saves little time here (3-36 s a run) but ends the rule failures (14:16, 14:58, 19:19). Enforce: a PreToolUse hook that denies `hydra_tests.exe` with no `-tc`/`-sf`/`-ts` when run by a subagent.

Shorter turns and reports for model time. Reviewers made 27-62 read/grep calls each; one script that prints the diff's functions with their call sites would replace most of them. A report format cap (one paragraph per finding, no "checked one by one" recap of previous rounds; S1 R2-R6 each re-verified every earlier finding in prose) would cut the 15-66k characters of output per review by half. Rough saving: a third of the 72 minutes of reviewer model time.
