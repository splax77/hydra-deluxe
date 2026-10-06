# Audit: step 1 execution session (ade9655b, 2026-10-03 23:02 to 2026-10-04 04:45 UTC)

Scope: the session that ran the 22 step-1 tasks in parallel worktrees, integrated them on `claude/s1-int`, and finished the git-side derive-once gate. 75 agents, one main transcript. All numbers come from `metrics/agents.csv`, `metrics/commands.csv`, `metrics/gaps.csv`, the coordinator's `timeline.py`, and my own per-agent split of the raw transcripts (`audit/step1exec_peragent.txt`, `audit/step1exec_cmds.txt`, `audit/step1exec_reviews.txt`). Agent ids are the 17-character ids from the transcript file names.

## The short version

The session ran 5 h 42 min and used 20.0 agent-hours (1,201 agent-minutes), so on average 3.5 agents were alive; the peak was 12 at 01:33. Inside the agents, 47% of the time was tools running, 38% was the model producing its next step (no extended thinking anywhere; this is call volume times context size), 13% was the model streaming long replies and edits, and only 0.5% was an agent parked waiting to be woken. The main session itself spent 53 minutes in user questions and lost 32 minutes to one stalled tool call during which no agent ran at all.

A third of all agent time (420 of 1,201 minutes, 14 agents) went to the derive-once gate hooks, not to step 1. That is where the user's "25 minutes" complaints came from, and it is where the biggest avoidable costs are: five agents polished a command-line-parsing design that one review then killed (111 agent-minutes), and the two long gate agents reran whole hook test suites eight and five times (about 85 agent-minutes that a "changed files only" rule would have removed).

Step 1 itself was efficient per task: 22 implementers averaged 12 minutes each. What stretched it was a review finding on Task 17 that opened a chain of ten "extra" agents (203 agent-minutes, 01:31 to 04:41 on the critical path) fixing pre-existing engine bugs that, by the agents' own library counts, change no score on any of the 19,343 charts at default settings.

## 1. Where the wall-clock went

### The session's shape

From 23:02 to 23:50 only gate agents ran (one to three alive) while the 22 step-1 tasks waited; this is what the user called insanity at 23:49. Step-1 wave 0 started at 23:50, within a minute of the pushback. Implementers were then dispatched over three hours as their dependencies cleared: wave 1 at 00:01-00:12, Task 5's three parts serially 00:06-00:43, wave 2 at 00:49, wave 3 at 01:05-01:27, Task 22 at 03:12. Integration of the approved lanes happened at 01:49-01:59 (a430239e) and 02:59-03:11 (ae263cc6); the final merge and proof ran 03:46-04:08 (adca81b4). Between 02:27 and 02:58 the fleet was empty (see the stall below). The user stopped new work at 04:37.

### Agent time, split into non-overlapping states (all 75 agents)

| State | Minutes | Share |
|---|---|---|
| A tool was running | 562 | 47% |
| Model working before its next output (prefill + generation of the next block) | 458 | 38% |
| Model streaming further blocks (long replies, big edits) | 152 | 13% |
| Attachments, hooks, other | 25 | 2% |
| Finished and parked until woken | 5.5 | 0.5% |

### What the tools were doing (agent time only, from the category labels)

Reading and searching 135 min, Hydra builds 116 min, tests 80 min, polling background jobs 65 min, other shell 49 min, edits 41 min, GUI tests 30 min, library/compare runs 23 min, file reads 25 min, status lines 9 min. The labels are keyword-based and misfile some things: for example the long "read/search" items in a1eb5012 (602 s, 497 s, 317 s) are whole hook-suite runs launched through a PowerShell script. Reading the actual commands, the 14 gate agents spent roughly 150 of their 213 tool-minutes running hook test suites and probe scripts, and the step-1 agents spent most of theirs on `build_cpp.ps1` (91.6 min across 56 agents), `hydra_tests` and `hydra_uitest --all`.

### Why model time is so large (the coordinator's question)

Model time is 610 of the 1,201 minutes. There is no extended thinking in any transcript (about 1k thinking characters across all 75 agents), so this is not "thinking per step". It is 8,922 API calls at an average context of 158k tokens. The wait per call rises with context: 3.5 s at 50-100k, 5.4 s at 100-150k, 6.0 s at 150-200k, 6.9 s at 200-250k, 7.7 s at 300-350k. Four things drive it:

Every agent's first call already carries 59-60k tokens (system prompt, CLAUDE.md, the skills listing, the brief). That fixed load is paid on every one of the 8,922 calls.

Every implementer and reviewer then reads the same 4,600-line plan from cold (header, Global Constraints, Parallel schedule, Appendices A and B, its task). Contexts jump to 100-150k within the first ten calls; 56 agents did this independently. Reviewers additionally re-read the implementer's branch.

Call counts are high because every edit, build, test and status line is its own call: 100-250 calls per implementer (a684ed30 made 81 Edit calls, a7088286 74, a3b0e44f 44), and each call pays the full context.

A few single calls are minutes long because the agent wrote a whole file in one Write: ac526ca8 spent 7.6 min on one call writing the 608-line `test_h17_git_ref_gate.ps1`; a45cc793's longest single wait was 294 s, a3aef9d4's 167 s, ac5dd18d's 230 s. The 152 min of "streaming" is mostly the 2-6k-character final reports (every agent wrote one) and multi-block turns.

So the lever is not "think less"; it is fewer calls per agent and smaller contexts per call.

### Parked time: on the coordinator or on the user?

Parked time is negligible in this slice: 5.5 minutes total. 2.6 min is a758cc0f being woken by system notifications about its own background job, about 2 min is three agents (ab88398d, adc6b681, a0a7bfa5) re-woken by Stop-hook feedback asking them to back a claim, and 0.9 min is a430239e waiting for the coordinator's "go with option 3". No agent ever waited on the user directly.

The user-wait lives in the main session: 14 AskUserQuestion calls totalling 52.9 min (the 03:46 one took 26.6 min, the 02:18 one 8.3 min, the 02:59 one 7.5 min), plus the 32-minute stall described below. That is 85 of the main session's 342 minutes during which nothing new could be dispatched.

### The five most expensive agents

1. a1eb5012 "Finish and install ref gate", 87.6 min. 60 min of tools, of which about 50 min is eight whole-hook-suite runs (602, 497, 433, 572, 340, 317, 170 and 114 s), first in a staging copy and then again on the live folder. One run hung on a child process in test_h5 and one failed on Get-FileHash under Start-Process, so runs were repeated. 20 min of model time over 256 calls at an average 314k context.
2. a45cc793 "Command-line hook cleanup", 84.7 min. Five polling loops (260, 179, 244, 292, 192 s) waiting on background whole-suite runs, plus 11 min of foreground suite runs, plus 23.5 min of model time over 248 calls at 346k average context, the largest in the session. Its brief covered four gates, the scanner, the dispatcher and four test files in one agent.
3. a3aef9d4 "Git-side ref gate fix round", 35.9 min. Three full test_h17 runs of 139-152 s each (real git on throwaway repos) and 18 min of model time over 164 calls.
4. ac526ca8 "Git-side ref gate (review+trailers)", 34.7 min. The 7.6-min single Write above, then suite runs of 108, 188 and 305 s; 15 min of model time.
5. adbf45f3 "Measure double-squeeze options", 30.3 min. 18 min of tools: a prototype built with 26 edits, then library runs (19,343 charts, three caps, two modes, 10-16 s each) run one after the other inside a 314-s poll loop, and 9 min of model time.

For comparison, the costliest step-1 implementers were af44823f (Tasks 9/10/13 lane, 19.7 min), a21065a9 (Task 20, 18.1 min) and a0a7bfa5 (Task 18, 17.5 min); the median implementer was about 12 minutes.

## 2. Which time was avoidable

**Polishing a design that was then abandoned (about 111 agent-minutes, 98 min of wall on the gate track).** The first reviewer of the attribution gate (aa6e52e6, 23:04-23:10) already reported that quoted and inner-shell commands slip past text parsing. The response was three more parsing agents: the shared scanner (ac5dd18d, 25 min), moving the derive-once gate onto it (ae9fcd5b, 21 min), and a redirect label (a46b8bdd, 23 min), plus fix round 2 (a2435f39, 13 min). The next review (aeddca2a, 00:22-00:42) found that `git checkout main && git merge` in one call skips the gate entirely and listed about thirty other bypasses, and the user moved the judge into a git `reference-transaction` hook at 00:42. The scanner survived for the command-line gates, so not all of this was waste, but the derive-once judge work before 00:42 was. A design check after the first review would have saved most of it.

**Whole hook-suite reruns (about 85 agent-minutes).** a1eb5012 ran the full suite eight times, a45cc793 five or more, a46b8bdd and ae9fcd5b several times each; the h5 suite alone spawns a PowerShell process per case and takes 5-10 min under load. Proving each change on its own test file and running the suite once before install would have needed about 25 minutes instead of about 110. The rule now exists as text (memory `no-repeat-suite-runs`, written later on 2026-10-04 from another session); in this session it did not yet exist, and nothing enforces it.

**The 32-minute stall with an empty fleet (02:27-02:58).** At 02:26:47 the main session ran `cat >> docs/audit/2026-10-03-fix-decisions.md <<'EOF' ... D31 ...` through Bash. The PreToolUse hooks passed at 02:26:47, the command "completed with no output" at 02:58:37, and the Agent call issued 15 s later (the extreme-tempo fix, a1599160) only launched at 02:58:38 because it queued behind it. Five task notifications piled up meanwhile. The transcript holds no deny and the command itself is instant, so the most likely cause is a permission dialog for appending to a repo file through Bash that the user, who had just answered a question at 02:26:26, was not there to click; I cannot prove that from the data. The effect is certain: zero agents alive for 31 minutes, and every later step (fast-tempo fix, its review, D34, the ready-key prototype) shifted right into the 04:37 cut-off.

**Scope growth from one review finding (203 agent-minutes, 01:31 to 04:41).** a99e8ef2's review of Task 17 found a "can't happen" throw that fires and an older early-squeeze-out bug. The user chose "fix it in step 1, count first". The count (a43cb9ed) found zero affected charts at defaults; the fix went ahead anyway (a264046c, 24 min), then its review (a1b5aef8, 18 min), its minors (ae037693, 10 min), then two extreme-tempo crashes at 2,000-4,000 BPM (a1599160, 28 min; review a12c6466, 18 min), then D34 (ab3ccce6, 27 min), a measurement (adbf45f3, 30 min) and the ready-key prototype (aa2ab841, 20 min). Every one of these reported 0 score changes on the library at defaults; the net visible effect was 7 path changes at cap 2. The merge of s1-fast then crashed the tests at extreme tempo and needed its own fix and review (adca81b4, a3381bff, af378356). Step 1's integration would otherwise have been final at about 03:11 instead of 04:08.

**Task 5 split three ways by the call budget (38 min on the critical path).** ac7919c3 hit the 100-call warning after only the fixture header and stopped; afbe874f hit it again at step 6; a684ed30 finished. Each restart re-read the plan from cold, and waves 2 and 3 waited on Task 5. A brief that said "commit at 110 calls and continue" or that split Task 5 into the fixture helpers (which could have run in wave 0) and the engine change would have saved 15-20 minutes of critical path. The budget rule is now in memory (`derive-once-gate-2026-10`) but only as brief text.

**Cold worktree builds (18-36 agent-minutes).** Every `git worktree add` starts without `build-cpp`, so the first `build_cpp.ps1` in 11 worktrees took 60-138 s (17.6 min total); 25 builds over 60 s cost 36 min in all. Builds that follow an edit are 5-40 s.

**Chasing a known flake (about 10 agent-minutes, plus noise in every review).** `batch-strip-drift` was flaky on main before step 1. At least seven agents reran `hydra_uitest --all` and then the single test to show it passes alone (a4d422a1, a2f96983, a0a7bfa5, a21065a9, a4cff2ae, afc96f7b, a8d3544e). It was fixed only at 03:46 (a3cc681e, 17 min).

**Blocked shell reads (about 15 agent-minutes).** About 60 Bash calls using `sed -n`, `head`, `tail` or `awk` were denied by the shell-text-tool hook across 35 agents, each costing a retry call of 10-20 s. The briefs never say the rule.

**Serial library runs (about 5 minutes).** adbf45f3 ran its six library passes one after another in a poll loop; this is the pattern the user corrected later the same day (memory `parallel-by-default`, "library runs too").

**Main-session context.** The main transcript took 110 monitor notifications and 75 agent reports and compacted twice (01:26 and 04:44). After the first compaction the progress ledger carries guessed timestamps ("some earlier lines carry guessed 22:xx times", 01:57). This cost accuracy rather than minutes.

## 3. Review rounds

"Cost" is wall time from the reviewer's start to the last fix or re-review that round caused; agent-minutes are the sum over the agents involved.

| Round | Reviewer (min) | What it found | Fix agents (min) | Wall cost | Agent-min | Preventable by the brief? |
|---|---|---|---|---|---|---|
| Wave 0: T1-T4, T15 | a131efe3 (16.1) | T1: "SqIn at offset 0 is free" written twice (rating and sentence); T4 printer printed 0 for a missing offset; T3 and T15 minors | a2f96983 (10.0), a1ec2b66 (1.7), a82e538f (10.1) | 00:08-00:37, 29 min | 38 | Partly. T1's brief said run the older plan "exactly as written" and that plan held the copy. T4's zero-for-missing was a plan gap. |
| Wave 1: T11, T12, T16 | a2274084 (8.8) | T11: replay still computed the offset by hand; scan pattern missed `hydra::` prefix; T12/T16 minors | ae5c9967 (6.8) | 00:20-00:37, 17 min | 16 | Yes: "grep every caller of the old rule, including qualified names" is one line. |
| Task 5 | a83bb1ae (12.8) | Fixture helper took a measure-relative end instead of a literal tick; three minors | aa26be71 (7.4), re-review inside a4b7ec4d | 00:45-01:22, 37 min | 27 | Yes: Appendix B literals were the plan's own rule; the brief could have named the helper's contract. |
| Tasks 6 and 17 | a99e8ef2 (22.4) | T6 test repeated the engine's max; T17's "unreachable" throw fires and kills the chart; found the older early-squeeze-out bug | a209a017 (6.0), a2848e52 (17.3), then the ten-agent chain above | 01:06-04:41 | 23 + 203 | The throw: yes, "never add a throw for a state the plan calls impossible; test it instead". The older bug: no, it predates the plan; the choice to fix it in step 1 was the cost. |
| Task 8 | a4b7ec4d (15.2) | Four minors, two wording decisions (D16, D17) | afc96f7b (8.6) | 01:06-01:40, 34 min | 24 | Mostly user wording; no. |
| Tasks 9, 10, 13 | a2ddf2db (8.0) | T13: a test that compared a number with itself; codec writer accepts an unreachable row | ac1a2699 (13.0) | 01:27-01:49, 22 min | 21 | Yes: "a corpus test must recompute the fact independently or it proves nothing" is the recurring finding kind. |
| Tasks 18, 19 | a5a41275 (11.5) | T19's guard skipped variants silently | aa63e510 (6.0) | 01:37-01:56, 19 min | 18 | Yes: "pin the skip count". |
| Tasks 20, 21 | adc6b681 (8.7), a5c72d5c (8.7) | Minors; a gauge rounding judgment call for the user (D15) | a4cff2ae (6.4) | 01:27-01:57, 30 min | 24 | No; the judgment call needed the user. |
| Task 7 | a625a823 (6.5) | Five minors, no fix dispatched | none | 6 min | 7 | n/a |
| Early-squeeze-out fix | a1b5aef8 (18.4) | Four minors; measured 0 effect on the library | ae037693 (9.9) | 01:58-02:27, 29 min | 28 | n/a (the fix itself was the question) |
| Extreme-tempo fixes | a12c6466 (18.2) | Approve; one item needing a decision (became D34) | ab3ccce6 (27.3) | 03:27-04:41 | 46 | n/a |
| Task 22 | a8bd9670 (16.8) | ADR named the wrong release; format-7 history comment incomplete | inside adca81b4 (22.8) | 03:27-04:08, 41 min | ~27 | Yes: "list every codec change between main and s1-int" could have been the brief's step. |
| Merge fix + uitest flake | a3381bff (8.6) | Approve; bank-tick test gap on fast charts | af378356 (12.7) | 04:14-04:36, 22 min | 21 | n/a |

Gate-side rounds: attribution-gate review (aa6e52e6) to redirect label, 23:04-00:21, 77 min wall, 111 agent-min; scanner-and-gates review (aeddca2a, 20 min) ended the design; git-side gate review (a758cc0f, 29.5 min: symbolic-ref hole, marker-removal hole, one more) to fix round (a3aef9d4, 35.9 min) to install (a1eb5012, 87.6 min), 01:21-04:35.

Thirteen step-1 reviews cost 144 agent-minutes and produced 103 minutes of fix agents. Every review found something, and about half the Important findings are the same four kinds the later memory names (a copied rule, a test that recomputes nothing, an unpinned skip, a number without a decision). Putting those four kinds in every implementer brief, not just the reviewer's, is the cheap fix.

## 4. The user's corrections, from the transcript

**23:23 "commit and push can be separate calls."** The gate design had treated commit-and-push as one gated shape. A detail the user had to correct; the rule is not to invent constraints the user did not ask for.

**23:49-23:51 "so you're saying you want to spend days running one agent at a time ... complete insanity" and "why do i have to constantly babysit you?"** Trigger: the user asked "what is left after round 3?" and the reply at 23:49:11 listed six gate steps in order and then "Then the step-1 plan runs, one agent per task." At that moment the session was 47 minutes old, one to three gate agents had ever been alive, and none of the 22 step-1 tasks had started. Within a minute of the pushback three worktrees and four agents were running, and a planner (ab6f4b58) produced the four-wave schedule at 00:00. This session is the origin of the `parallel-by-default` memory. The rule that would have prevented it: build the wave table and start wave 0 before any other track, in the same turn as the plan is accepted.

**01:10 "how are the git gate and command line agents doing? 25 minutes is a long time."** ac526ca8 and a45cc793 had started at 00:45. ac526ca8 was 25 minutes in because of the 7.6-minute single Write and three suite runs of 2-5 minutes; it finished at 01:20. a45cc793 was 25 minutes in and went on to 02:10, polling five whole-suite runs. The user asked again at 01:43 ("how is the command line agent doing?"); it was at 58 minutes, in the middle of those polls. Two rules would have prevented it: targeted test files only, suite once at the end; and a brief small enough to finish in about 45 minutes (that agent's brief covered four gates, the scanner, the dispatcher and four test files).

**04:37 "do not launch any new work. let the remaining work finish and create a handoff."** The session was 5.5 hours in, had compacted twice, and the assistant had just proposed merging the gate branch to main as the gate's first live test. The user ended it. The 32-minute stall and the review-chain scope growth are what pushed the end of step 1 past this point.

Two Stop-hook bounces (00:07 and 01:34) also hit the main session for bullet-heavy replies, each costing a rewrite turn.

## 5. Speedups for steps 3-5, with estimates and enforcement

**Targeted tests only; suite once at the merge point.** Saves about 85 agent-minutes per gate-sized change and 30-50 minutes of wall on the longest agent. Enforce with a PreToolUse hook on Bash/PowerShell: deny a command that runs more than two `test_h*.ps1` files, or `hydra_tests.exe` with no `-tc`/`-sf` filter, or `hydra_uitest --all`, unless the environment variable set only by the main session's merge step is present. The rule text already exists; this is the hook that makes it hold.

**Never block the fleet on a permission dialog.** Saves the 31-minute empty-fleet gap (once per session that it happens). Enforce in settings: allow `Edit`/`Write` under `docs/audit/`, `docs/handoffs/` and `.superpowers/` for the main session, and have the orchestrator write ledgers with Edit/Write instead of `cat >>` heredocs (the shell-text hook already pushes agents that way; the main session is the exception). Add a watchdog line to the status monitor: "no agent alive for 5 minutes while the task ledger has open tasks".

**One design review before any fix round on a new mechanism.** Would have saved about 100 agent-minutes and an hour of the gate track. Enforce in the workflow script: a "Needs fixes" verdict whose findings include a bypass of the mechanism itself (not a case) routes to a design question for the user, not to a fix agent.

**Pre-existing bugs found in review go to the ledger, not to a fix agent.** Would have returned about 2 hours of the session's tail and 200 agent-minutes. Enforce in the reviewer brief and the orchestrator's dispatch check: a finding outside the plan's task list is filed under `docs/audit` with the library count, and only the user's explicit "fix now" launches an agent; the count agent runs first and alone.

**Smaller contexts per call.** Contexts under 100k average 3.5 s per call; 150-200k average 6 s. If every implementer read a 200-line task excerpt instead of the plan's 1,500-line header-plus-appendices, average context would drop by roughly 50k and the model-wait bucket data says that is about 1.5-2 s per call, or 2-3 agent-hours across 8,900 calls (not wall time, since agents run in parallel). Enforce with the plan-writing step: emit one `tasks/<n>.md` per task with the global constraints inlined, and brief the agent on that file only. Also trim the per-agent fixed 59k: the skills listing and tool catalogue a subagent never uses.

**Fewer calls per agent.** Implementers made 100-250 calls because every edit was separate and every build re-ran after it. Briefs can say "batch related edits, build once per logical change" and "status line every 10 calls" is already the floor. Saves perhaps 20% of model time (about 2 agent-hours); enforcement is soft (the budget hook's 100-call warning is the existing lever; keep it, and brief "commit before 110 calls").

**Warm worktrees.** Copy the main checkout's `build-cpp` into a new worktree before the agent starts (a `New-Worktree.ps1` helper the orchestrator must use). Saves 15-35 agent-minutes per 20 worktrees, and the first proof in each lane lands 1-2 minutes sooner.

**Fix known flakes before the wave, not during.** `batch-strip-drift` cost about 10 agent-minutes of reruns and clouded every review's uitest line. Enforce with the wave-0 checklist: "any test on the run-alone list gets a fix agent in wave 0".

**Say the shell-read rule in the brief.** One sentence ("sed/head/tail/awk are blocked for reading; use Read with offset and limit") removes about 60 denied calls, about 15 agent-minutes. Enforce by putting it in the shared brief preamble the status rule already lives in.

**Thin the monitor.** 110 monitor notifications in 5.7 hours drove two compactions of the main session. Set the batch monitor to report only changed lines and at 10-minute intervals; the status files remain readable on demand. Saves context and the post-compaction errors, not minutes.

## 6. Data notes and unknowns

The command categories in `commands.csv` are keyword-based. Status-line calls whose text mentions tests get labelled "test" or "build", and whole-suite runs launched through a script got "read/search"; for the gate agents I read the actual command text and durations rather than the category totals. The agents.csv `model_min` column counts only the wait before the first output; the per-agent split above (`step1exec_peragent.txt`) uses the coordinator's state rules and adds the streaming share.

The cause of the 02:26:47 stall is inferred, not proven: the transcript shows hooks passing, a 1,910-second Bash call that completed normally, and queued notifications in between. A permission dialog is the only mechanism I know that fits; the Claude Code permission log, if one exists, would settle it.

Two user messages ("babysit", and the 04:37 instruction) are not in the user-message records: the first is a queued message typed while the assistant was running, the second was typed as the answer to an AskUserQuestion. Both are quoted from those records.
