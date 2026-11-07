# Plan: a controlled test of agent effort (high, medium, and low where it fits)

Written 2026-11-07 by a planner agent (task `EFFORT-PLAN`, session `43eecb1c-c046-4595-b33c-4b483bdcbdc8`). Nothing in this plan has run. The test spends real money, so it waits for the user's yes, and that yes must name a budget (agent rule 3; the handoff's "What is next").

## The question in one paragraph

Agents here run at whatever effort their session has, which has mostly been high. The handoff found that medium-effort agents cost about half as much as high-effort ones, but those agents did different work in different sessions, so the gap mixes effort with task difficulty. This plan runs the same work at each level, scores it blind, and counts rework, so the user can decide per role whether to lower effort. The honest limit up front: an affordable test can show the cost gap is real and can catch a large quality drop. It cannot see a small one. Section "How big the test is, and what it can and cannot show" says the numbers.

## What I checked in the handoff

The handoff is `docs/handoffs/2026-11-07-Cost measurement and the planned controlled test of agent effort (high vs medium)-handoff.md`. The checker printed 9 `ok`, 0 `missing`, 14 `manual`. I checked the manual rows against the machine. The agent files set `model: opus` (executor), `sonnet` (reviewer, scout), `fable` (planner), `haiku` (card-writer), and no file has an `effort:` line. `settings.json` has no effort key, and `CLAUDE_CODE_EFFORT_LEVEL` is empty in this shell. `claude --version` prints `2.1.310`.

I rebuilt the effort numbers from the transcripts myself, read-only, following the recipe in the handoff's last paragraph (walk every `*.jsonl` under the project's `subagents` folders, `.meta.json` gives `agentType`, last `message.usage` per `requestId`, price by `message.model`, the `effort` key on each assistant record). I scanned only the Hydra project folder, 1,604 agents in 126 sessions, not every project as the handoff did, so my counts are smaller than its 2,197. What I found agrees with the handoff where it overlaps. 1,591 of 1,604 agents matched their parent session's effort. 1 agent changed effort mid-run. 0 of 1,619 `Agent` tool calls carried an `effort` key. Medians since 2026-10-19, high then medium: executor $2.09 and $1.17 (handoff: $2.08 and $1.25), reviewer $0.77 and $0.33 (handoff: $0.77 and $0.30), planner $5.93 and $3.90 (handoff: $5.73 and $4.11). The scout medians differ: I got $0.13 high and $0.41 medium, against the handoff's $1.52 and $0.60. The scout means are $0.55 and $0.44, so the high-scout cost is skewed by a few large runs and the median hides them. My call counts are higher than the handoff's (executor 63 and 46 calls against 42 and 22) because I counted every `tool_use` block; the handoff probably counted API requests. Neither difference changes the plan.

Two handoff claims are wrong or stale and the plan does not stand on them. First, "What is next" item 2 says the Agent tool's `effort` route and the frontmatter route have "neither been tried here" and that the docs were unread. The docs are now read (below); the routes are still untried, which is what LS-0 settles. Second, the handoff's "agents take their parent session's effort" is contradicted by Claude Code's own docs, which say an agent on a different model from the session does not get the session's level, and that Opus 5.5, Sonnet 5.5 and Haiku 5.5 default to medium. The transcripts show Sonnet reviewers under Opus sessions carrying the session's `high`. One of three things is true: the docs describe a newer build than 2.1.310, the transcript's `effort` field records the session's setting rather than what the API received, or the docs are wrong for this host. The test cannot start until that is settled, because every result hangs on knowing which level each agent really ran at. That is LS-0.

## What the docs say about setting effort (read 2026-11-07, both pages cut off at 100,000 characters)

From `https://code.claude.com/docs/en/sub-agents`: the frontmatter `effort` field "Overrides the session effort level, but not the `CLAUDE_CODE_EFFORT_LEVEL` environment variable. Options: low, medium, high, xhigh, max". The per-call parameter: "When you ask Claude to run a non-fork subagent at a specific effort level, it can also pass an `effort` parameter for that invocation. The parameter overrides the `effort` field ... The per-invocation parameter requires Claude Code v2.1.292 or later." And: "On the main conversation's model, the subagent runs at the level you pick in the session ... On another model, that level doesn't apply. To choose its level, set the `effort` field."

From `https://code.claude.com/docs/en/model-config`: the session's level resolves in this order: the environment variable, a level picked in the session, `--effort`, saved settings, then "The model's default effort: `high` on every model that supports effort, except that Opus 5.5, Sonnet 5.5, and Haiku 5.5 default to `medium`". Fable 5.1 and the 5.5 models all support `low`, `medium`, `high`, `xhigh`, `max`.

So the priority the main session relayed is right as far as the docs go: environment variable, then per-call parameter, then frontmatter, then session level (on the same model) or the model's default (on another model). The test uses the per-call parameter, because it changes no file and a user-approved plan naming each agent's level counts as the instruction the Agent tool's text asks for. If the user later adopts a level for a role, the permanent form is the `effort:` frontmatter line in that role's agent file, which is a gated change to `C:\Users\Patrick\.claude\agents\` and needs its own review.

One web result (a blog, unchecked) says Claude Code 2.1.316 added `CLAUDE_CODE_SUBAGENT_EFFORT` and stopped subagents inheriting the session level. This machine runs 2.1.310. If Claude Code updates during the test the ground moves, so every live step records `claude --version` and stops if it differs from LS-0's.

## Prior art, and what I take from it

I searched the web for measurements of reasoning-effort levels in coding agents. The usable sources are an arXiv preprint, "Prompt-Induced Waste in Coding Agents: Reasoning, Effort, Harness Design, and End-to-End Cost" (`https://arxiv.org/pdf/2608.01347`), which reports that going from low to xhigh raised billed cost 2.4 to 4.5 times while success on Opus 5 barely moved, and that every timeout and empty patch happened at high or xhigh; a Latitude blog post, "reasoning effort only matters without tests" (`https://latitude.so/blog/reasoning-effort-only-matters-without-tests`), which found effort made no difference when the agent could run tests and a large difference when it could not; and a single-run Sonnet 5.5 bug-fix comparison (`https://wmedia.es/en/tips/claude-code-subagents-cheaper-low-effort`) where medium was cheapest ($0.076) because low took 12 turns instead of 4 and cost more ($0.094) than medium, with every level passing all hidden tests. All three are `unchecked` premises in the map; none is load-bearing for a result, only for the design.

Three design choices follow. Our executors can run their task's tests, so the prior art predicts a small quality gap there; the test must be able to see a gap of a few failures or it proves nothing new, and the size section is honest about that. Low effort can cost more than medium through extra turns, so the test measures cost, not just the level, and never assumes lower is cheaper. Single runs are noise, so every cell has repeats.

## Global constraints

Every test agent stays on its role's model (agent rule 2): executors on Opus, reviewers and scouts on Sonnet, planners on Fable. No run touches the full test suite, `hydra_uitest --all` or any library run (rules 1 and 8); an executor runs only the test file its brief names, on `testdata/input`. Every test branch is cut by hand from one pinned base commit into a worktree directly under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`, never with `isolation: worktree`, because that cuts from `origin/main`, which is hundreds of commits behind (memory rule "Agent worktrees cut from origin/main"; `.claude/worktrees/` is also the one place the delete rule trusts). No test branch is ever merged into `main`; the check in T4 proves it. The scorer never learns an agent's level. Every step has a dollar cap, a run cap and a time cap, and says what to report when a cap is hit. The whole test has a hard stop cap. `CLAUDE_CODE_EFFORT_LEVEL` must be empty for the whole test, because it would override every per-call level.

## The work before any run

### T1: rebuild the accounting as a committed script

The handoff's scripts were scratch and are gone. T1 writes `tools/agent_cost.py` in the Hydra repo, on its own branch, following the handoff's recipe: walk every `*.jsonl` under `C:\Users\Patrick\.claude\projects\<project>\<session>\subagents\`, read `agentType` from the `.meta.json` beside each, take the last `message.usage` per `requestId`, price it by `message.model` with the price table from the handoff (Fable 5.1 $10 / $50 / $0.25 per million input / output / cache read, Opus 5.5 $4 / $20 / $0.20, Sonnet 5.5 $2 / $10 / $0.20, Haiku 5.5 $0.10 / $0.50 / $0.01; cache writes at 1.25 times input), read the `effort` key on each assistant record, and count `tool_use` blocks. It takes `--session <id>` to report only one session's agents, `--agent <id>` for one agent, and `--since <date>`. It prints one row per agent (agent id, type, model, effort values seen, cost, calls, first timestamp) and a summary per type and effort, and writes the rows as CSV to a path given with `--out`. It also prints the main session's own cost for `--session`, so the orchestration overhead is measured, not guessed. The main session's cost is read from `<project>\<session>.jsonl` the same way.

Acceptance: a test `tools/test_agent_cost.py` builds a two-record fake transcript with known usage and asserts the priced total to the cent for each of the four model families, asserts that the last usage per `requestId` wins, and asserts the `effort` values are listed. Run with `py -I -m unittest tools.test_agent_cost`. The script run on this session's folder with `--since 2026-10-19` reproduces the executor medians above within $0.10 (the rebuild in this plan is the pin; if the committed script disagrees by more, the script is wrong, not the plan). The results of every live step are written by this script to `docs/superpowers/specs/2026-11-07-agent-effort-test/runs.csv` on the test's results branch, one row per agent, with the step, task, role and level columns added by the main session from its launch log.

### T2: pick the tasks and recover their briefs

Executor tasks come from merges on `main` whose subject ends `(reviewed CLEAN)` and whose change is not under `src/ui/` (a UI change would need the real-app check, which has no place in a test). I recommend four, chosen because each is small, has its own test file and is recent: the quiet-machine check in `ch_start` (merge `32973b96`), the gap line in the Star Power watch status (`735d2daa`), the audio, raw clock and correction in the Star Power watch (`8ad1fc65`), and the ETW recording script (`60f7a5b3`). For each, T2 finds the executor's transcript under the launching session's `subagents` folder, takes its first message as the brief, verbatim, and records the branch's base commit (`git merge-base main <branch tip>`), the test command the brief names, and the merged diff as the reference. It writes a checklist per task: each behaviour the merged diff added, one line each, in plain words, with no mention of how it was done. The blind scorer uses that checklist. T2 also writes the reviewer fixtures: for each of the four branches it cuts a fresh branch from the same base, applies the merged diff, then plants three faults of the kinds the project's review rules target (an off-by-one at a boundary the tests do not reach, a comment that restates a rule, a second copy of a rule that another function already owns), commits with trailers, and writes the fault key (file, line, what was planted) to a file under the test's spec folder that only the scoring script reads. It writes the four scout questions with exact-answer keys: each is a read-only question about the repo whose answer is a file path and a quoted line, such as "which file owns the agent call budget numbers, quote the line". Planner tasks are not prepared unless question 2 says yes.

Acceptance: four executor briefs, four fault branches each with exactly three planted faults listed in the key, and four scout questions with keys, all committed on the test's results branch; `git log main..<fault branch>` shows one commit per fault branch; the fault key's line numbers match `git show` of each fault commit (a test in T2's own check script, run once).

### T3: the blind scorer's brief and the scoring script

The scorer is a Sonnet scout at high effort, launched once per executor task after that task's runs finish. It gets the task's checklist, the reference diff, and the six runs' diffs and test output labelled R1 to R6 in an order a script shuffled and recorded in a key file the scorer never reads. It fills one line per run per checklist item (done, partly, missing) and lists anything a run changed outside the task. It never sees transcripts, costs, levels or agent ids. Scoring for reviewers needs no scorer: a script reads each reviewer's commits after the handed tip and checks each planted fault's file and line against the fault key (fixed, named in the review but not fixed, or missed), and counts edits to lines with no planted fault as false findings. Scout answers are checked against the key by exact match of the path and the quoted line, by script.

Rework is counted in dollars. A run that fails (an executor whose named tests do not pass, or that hit the progress cap or reported unfinished; a reviewer that missed a planted fault; a scout whose answer does not match) is charged its own cost plus the median high-effort cost of its role for a redo. That is the "rework-adjusted cost" the decision rule reads.

Acceptance: the shuffle script writes a key file and the scorer's brief holds no level, cost or agent id (a grep of the brief for `high`, `medium`, `low`, `$` and `agent-` finds nothing). The fault-check script is run on one fault branch with a hand-made fix of one fault and reports exactly one fixed, two missed.

### T4: the harm guard

Before any run, T4 writes `tools/effort_test_guard.ps1` (committed on the results branch) that checks four things and prints one line each: `CLAUDE_CODE_EFFORT_LEVEL` is empty; `claude --version` equals the version LS-0 recorded; no branch named `claude/eff-*` is contained in `main` (`git branch --contains` on each tip, filtered for `main`, prints nothing); and every `claude/eff-*` worktree sits directly under `.claude\worktrees\`. The main session runs it before and after every live step. Hooks and gates, each with its owner: the ref gate (`ref_judge.py` in `C:\Users\Patrick\.claude\gates\`) refuses a commit without `Task:`, `Agent:` and `Session:` trailers on any branch, so test briefs carry them as the preamble says, and it refuses any move of `main` without a fresh review, which is the second wall between a test branch and `main`. Review verdicts (`review_dispatch.py` and `agent_stop.py`, writing `state\reviews.jsonl`) will record a CLEAN or BLOCKED verdict for each fault branch's tip when a test reviewer finishes; that record would let that tip onto `main` if a session merged it, so the fault branches are kept out of `main` by T4's check and deleted by the user after the results are committed (question 6). The progress budget (`progress.py`, 100 and 150 calls, 20 and 30 minutes, 5 and 10 minute status deadlines) applies to every test agent alike and is part of what is measured: a run stopped by it is a failure with rework. The status rule costs each level the same. The delete rule (`delete_rule.py`) is why worktrees go under `.claude\worktrees\`. The cleanup sweep (`cleanup_sweep.ps1`) removes only worktrees `main` already holds, so test worktrees stay until deleted by hand. The whole-library guard and the key-route guard never fire, because no chosen task needs them.

Acceptance: the guard script run on a fixture repo with one `claude/eff-x` branch merged into `main` prints a failing line and exits 1; run on the real repo before LS-0 it prints four passing lines and exits 0.

## How big the test is, and what it can and cannot show

Each cell (one role, one task, one level) gets 3 repeats. Runs of the same task at both levels launch in the same wave, alternating levels, so time of day and machine load fall on both sides alike (rule 6 says to run in parallel anyway).

Executors: 4 tasks x 2 levels x 3 repeats = 24 runs. Reviewers: 4 fault branches x 2 levels x 3 repeats = 24 runs. Scouts: 4 questions x 3 levels (high, medium, low) x 3 repeats = 36 runs. Planners: 0 unless question 2 says yes (then 2 tasks x 2 levels x 2 repeats = 8 runs, scored by a plan reviewer on a fixed checklist).

What this can show. The cost gap: the rebuilt data puts the executor interquartile range at $1.47 to $3.15 at high and $0.79 to $1.68 at medium. Within one task the spread is narrower than across tasks, and a paired comparison of 12 runs per level per role can tell a 40% cost gap from noise; the observed gap is about 45%, so if it is real the test will see it. A large quality drop: 12 of 12 passing at high against 7 of 12 at medium is a difference a two-sided Fisher test calls real at about p = 0.04. A small quality drop: 12 of 12 against 10 of 12 is p about 0.48, which is no evidence either way. Said plainly: if medium fails one or two more runs than high out of 12, this test cannot tell whether that is real, and the decision rule below treats that as "keep high", not as "medium is fine". Doubling the repeats to 6 per cell would roughly halve the smallest detectable gap and double the cost; that is question 5.

## The live steps

Each step is a live step because it spends money and launches agents. Each `Calls:` line lists the main session's tool calls; "never tried" means never tried with a per-call `effort` on this host. A `Workflow` launch counts as the Agent call it makes.

### LS-0: does a per-call effort land on an agent of another model?

One small, cheap run that settles the docs-versus-transcripts conflict and proves the mechanism. The main session launches two Sonnet scouts in one wave on the same trivial read-only question (T2's first scout question), one with `effort: "low"` and one with `effort: "high"` on the Agent call. After both hand back, the main session runs `tools/agent_cost.py --session <this session>` and reads each agent's `effort` values and output tokens from the transcript. Pass: each agent's every assistant record carries exactly the requested level, and the high run's output tokens exceed the low run's. Fail: either agent carries the session's level or `medium` instead of the requested one, or both runs show the same `effort`. On fail the step reports both agents' `effort` values and the version, and the test stops here; the next move is the user's (question 7). This step also records `claude --version` for T4's guard, and the main session's own cost for the two launches, which becomes the measured orchestration overhead per launch used in every later cap check.

Caps: 2 runs, $3, 15 minutes wall clock from the first launch. When a cap is hit, report the agents' ids, what each transcript shows, and the cost so far.

Calls: `PowerShell` running `tools/effort_test_guard.ps1` (never tried), `Agent` with `subagent_type: scout` and `effort` set (never tried), `Agent` with `subagent_type: scout` and `effort` set (seen: the first launch of this same step), `PowerShell` running `tools/agent_cost.py --session` (seen: T1's acceptance run).

Two never-tried calls sit in this step, which the premise-map page forbids. So LS-0 is split: LS-0a runs the guard script alone, and LS-0b runs the two launches and the accounting. Both are written above as one story; the go request names LS-0a first and LS-0b only after LS-0a prints four passing lines. LS-0b then holds one never-tried call (the first Agent launch with `effort`).

### LS-1: executors, high against medium

For each of the four tasks, one wave of six executors (three at `high`, three at `medium`, alternating) in six worktrees cut by hand from the task's base commit: `git worktree add .claude\worktrees\eff-<task>-<n> -b claude/eff-<task>-<n> <base>`. Each brief is the recovered brief verbatim, with the worktree path, branch, task id (`EFF-<task>-<n>`) and the test command it already names. After the wave, the main session runs the guard and the accounting, then launches the blind scorer for that task (T3). Then the next task's wave. A run that hits the progress cap or reports unfinished is a failure; nothing is retried.

Caps: 24 executor runs plus 4 scorer runs; $75 for the step (24 runs at the rebuilt high mean of $2.31 is $55, plus 4 scorers at about $1 each, plus orchestration at LS-0's measured per-launch cost; the cap holds a margin for a few $7 runs, which the rebuilt max says happen); 3 hours wall clock. At any cap the step stops after the current wave, and the report gives the runs finished, their costs and scores so far, and which tasks did not run.

Calls: `PowerShell` (guard, seen: LS-0a), `PowerShell` (`git worktree add` by hand, seen: this repo's everyday use), `Agent` x6 with `subagent_type: executor` and `effort` (never tried for executor; seen for scout in LS-0b), `PowerShell` (accounting, seen: LS-0b), `Agent` with `subagent_type: scout` for the scorer (seen: LS-0b).

### LS-2: reviewers on planted faults, high against medium

For each fault branch, one wave of six reviewers (three `high`, three `medium`), each dispatched with `subagent_type: reviewer` and a brief naming the fault branch's 40-character tip, as `docs/agents/derive-once-review.md` requires and `review_dispatch.py` checks. Six reviewers on one tip would each try to commit fixes on the same branch, so T2 cuts six copies of each fault branch (`claude/eff-rev-<task>-<n>`), all at the same tip commit, one per reviewer, each in its own worktree. After the wave, the fault-check script scores each reviewer's commits, and the accounting runs.

Caps: 24 runs; $35 (24 at the rebuilt high mean of $1.14 is $27, plus orchestration); 2 hours. At a cap, stop after the current wave and report as LS-1.

Calls: `PowerShell` (guard, seen: LS-0a), `Agent` x6 with `subagent_type: reviewer` and `effort` (never tried for reviewer; the mechanism seen in LS-0b and LS-1), `PowerShell` (fault-check script, seen: T3's acceptance run), `PowerShell` (accounting, seen: LS-0b).

### LS-3: scouts at three levels

For each of the four questions, one wave of nine scouts, three per level (`high`, `medium`, `low`). Each gets the question and nothing else. The answer-check script scores each, and the accounting runs.

Caps: 36 runs; $25 (36 at the rebuilt high mean of $0.55 is $20); 90 minutes. At a cap, stop after the current wave and report as LS-1.

Calls: `PowerShell` (guard, seen: LS-0a), `Agent` x9 with `subagent_type: scout` and `effort` (seen: LS-0b for high and low; `medium` never tried, so the first medium scout of the first wave is the one never-tried call), `PowerShell` (answer-check, seen: T3's acceptance run), `PowerShell` (accounting, seen: LS-0b).

### LS-4: planners (only if question 2 says yes)

Two planning briefs with a fixed checklist of what a good plan must contain (every promise mapped, every premise sourced, a cap on every step), 2 levels x 2 repeats, scored by one Sonnet plan reviewer per plan on the checklist, blind to level. Caps: 8 planner runs and 8 reviewer runs; $75 (8 at the rebuilt high mean of $5.99 is $48, plus reviewers at $1.14, plus orchestration); 2 hours.

Calls: `PowerShell` (guard, seen: LS-0a), `Agent` x4 with `subagent_type: planner` and `effort` (never tried for planner), `Agent` x4 with `subagent_type: reviewer` (seen: LS-2), `PowerShell` (accounting, seen: LS-0b).

### LS-5: the results document

The main session writes `docs/superpowers/specs/2026-11-07-agent-effort-test/results.md` from `runs.csv`: per role and level, the pass count, the rework-adjusted cost, the median and mean cost, the median calls, and the decision rule's verdict for each role. It commits on the results branch, and the branch goes through the usual doc review before anything is shown. Calls: `PowerShell` (accounting, seen: LS-0b), `PowerShell` (`git add` by name and `git commit` with trailers, seen: everyday use).

## The budget

All per-run numbers are the means from my rebuild of this project's transcripts since 2026-10-19 (T1's committed script re-derives them; until it does they are premise P9, unchecked). Means, not medians, because the caps must cover the skew. Orchestration (the main session's own tokens per launch) is unknown until LS-0b measures it; the estimate below assumes 25% of agent cost, which is a guess and is replaced by LS-0b's number before LS-1 starts.

| Step | Runs | Estimate | Hard stop cap |
|---|---|---|---|
| LS-0a, LS-0b | 2 | $1.50 | $3 |
| LS-1 executors and scorers | 24 + 4 | $60 | $75 |
| LS-2 reviewers | 24 | $28 | $35 |
| LS-3 scouts | 36 | $20 | $25 |
| Orchestration (25% guess, replaced by LS-0b) | | $27 | included above |
| Core total | 90 | about $137 | $140 |
| LS-4 planners (question 2) | 8 + 8 | $57 | $75 |
| Grand total with planners | 106 | about $194 | $215 |

The hard stop for the whole test is $140 without planners and $215 with them. The main session checks the running total after every wave with `tools/agent_cost.py --session`, and stops the test when the total crosses the cap, reporting what ran and what did not. A step's own cap stops that step the same way.

## The decision rule, fixed before any run

For each role, compare the 12 high runs with the 12 medium runs (or the 12 low runs for scouts).

Lower the role's effort to the cheaper level when both hold: the cheaper level has at most one more failure than high out of 12 (failures as T3 defines them), and its rework-adjusted cost is at least 25% below high's. Keep the role at high when either holds: the cheaper level has three or more failures than high out of 12, or its rework-adjusted saving is under 15%. Anything between is inconclusive, and inconclusive means keep high; the results note says what a larger test would cost to settle it. For scouts, low is chosen over medium only if low passes the same two tests against medium. A role whose step was stopped by a cap before 12 runs per level finished gets no verdict.

If LS-0b fails, no verdict exists for any role and nothing changes.

## What the user will see change

Nothing changes until the user approves a verdict. If a verdict lowers a role's effort, the change is one `effort:` line in that role's file under `C:\Users\Patrick\.claude\agents\`, on a branch, with a review, as the gates require. The user would then see cheaper agents in the accounting and, if the test was wrong, more rework in later sessions; the results note says which failures to watch for.

## Questions

1. **Budget.** Approve a core test at about $137 with a hard stop of $140, or with planners about $194 and a hard stop of $215. Recommendation: the core test without planners. You will see about 90 agent launches over roughly a day of wall clock, and nothing else changes.

2. **Planners in or out.** Planners are 9.6% of spend and the hardest role to score (a plan's quality is a judgement, not a test result), and the handoff's own numbers show the smallest effort gap for them. Recommendation: out for now, as a second test if the core test lowers any role. You will see no planner runs and no planner verdict.

3. **Low effort for scouts.** Adding `low` to the scout step costs about $7 more. The prior art warns low can take more turns and cost more than medium, which is exactly what the measurement would show. Recommendation: include it; scouts are read-only and the user asked whether some agents need only low. Reviewers do not get a low arm, because they are the safety gate and a missed fault there costs more than the test saves.

4. **The four executor tasks.** The four named in T2 are my pick for being small, tested and non-UI. Recommendation: accept them; or name others, with the same three properties. You will see those four tasks redone 6 times each on throwaway branches.

5. **Repeats.** Three per cell is the smallest size that can see the cost gap and a large quality drop. Six per cell would see a gap of about two failures in 12 and double the cost (about $275 core). Recommendation: three, and read the inconclusive verdict as keep high. You will see the limits written into the results note.

6. **Fault branches and review verdicts.** Test reviewers leave CLEAN or BLOCKED records for fault-branch tips in `reviews.jsonl`. Those branches are kept out of `main` by the guard, and the records only matter if someone merges them. Recommendation: after the results are committed, you delete the `claude/eff-*` branches and worktrees yourself, or type a yes for the main session to do it. You will see about 60 extra branches and worktrees until then.

7. **If LS-0b fails.** If the per-call level does not land, the only other route the docs give is the `effort:` frontmatter line in each agent file, which is a gated change to the agents folder for the test's duration. Recommendation: stop and ask before trying it. You would see one reviewed commit in `C:\Users\Patrick\.claude` adding and later removing those lines.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Same work at each level, so task difficulty cancels out | "The question in one paragraph"; "How big the test is" | T2, LS-1 to LS-3 | T2 acceptance: four briefs verbatim, four fault branches with three faults each, four questions with keys; the launch log pairs every high run with a same-task cheaper run in one wave |
| Every test agent stays on its role's model | Global constraints | T2, LS-1 to LS-4 | the accounting rows for each step show `model` per role matching rule 2; a row with another model fails the step |
| No full test suite, no `--all`, no library run | Global constraints | T2 | the recovered briefs name one test command each; T2's check greps every brief for `--whole-library`, `--all` and a bare `hydra_tests.exe` with no filter and finds nothing |
| Worktrees cut by hand from a pinned base under `.claude\worktrees\` | Global constraints | LS-1, LS-2, T4 | the guard's fourth line; the launch log holds each `git worktree add ... <base>` line |
| No test branch ever lands on `main` | Global constraints, T4 | T4 | guard line three fails on a fixture with a merged `claude/eff-x`; passes on the real repo before and after every step |
| The scorer never learns the level | T3 | T3 | grep of the scorer brief for level words, `$` and `agent-` finds nothing; the shuffle key is read only by the unblinding script |
| Rework is counted | T3, decision rule | T3, LS-5 | `runs.csv` has a `rework_cost` column; the results table reads it |
| Every step has dollar, run and time caps and says what to report | each LS "Caps:" line | LS-0 to LS-4 | the go request for each step quotes its caps; the accounting check after each wave |
| A hard stop cap for the whole test | The budget | LS-1 to LS-4 | `tools/agent_cost.py --session` total checked after every wave against $140 or $215 |
| No later step starts until LS-0b passes | LS-0 | LS-0b | LS-0b's pass line (both agents' `effort` values equal the requested levels, high output tokens exceed low) is quoted in LS-1's go request |
| `CLAUDE_CODE_EFFORT_LEVEL` stays empty, `claude --version` stays fixed | Global constraints, T4 | T4 | guard lines one and two before and after every step |
| The accounting is rebuilt and tested | T1 | T1 | `py -I -m unittest tools.test_agent_cost` passes; the script reproduces the executor medians within $0.10 |
| Results are written to one place | T1, LS-5 | LS-5 | `runs.csv` and `results.md` exist under the spec folder on the results branch; the doc review passes |
| The decision rule is fixed before any run | The decision rule | LS-5 | the results note applies the thresholds above verbatim; a verdict using other thresholds is a review finding |
| Nothing changes without the user's approval | What the user will see change | none (a rule on the main session) | no edit to `C:\Users\Patrick\.claude\agents\` appears in any test commit; `git log` of that repo over the test window shows none |
| Cheaper levels are measured, never assumed cheaper | Prior art; LS-3 | LS-3, LS-5 | `runs.csv` holds measured cost per run; the verdict uses it |

## Out of scope

The low-risk savings in the handoff's "What is next" item 3 (status lines alongside other calls, trimming starting context, skipping `MEMORY.md` for agents) and item 4 (the 18 cache rebuilds) are not in this plan; they need no quality test and should be offered separately. Changing any agent file's effort is out of scope until a verdict exists and the user approves it.

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: No agent file sets an `effort:` line; executor is `model: opus`, reviewer and scout `model: sonnet`, planner `model: fable`, card-writer `model: haiku` | code | `C:\Users\Patrick\.claude\agents\executor.md`, `reviewer.md`, `scout.md`, `planner.md`, `card-writer.md`, frontmatter lines 1-5 of each | checked 2026-11-07 |
| P2: `settings.json` has no effort key and `CLAUDE_CODE_EFFORT_LEVEL` is empty in this shell | code | `C:\Users\Patrick\.claude\settings.json` (grep for `effort`, no match); the shell's `$env:CLAUDE_CODE_EFFORT_LEVEL` printed empty | checked 2026-11-07 |
| P3: Claude Code here is 2.1.310 | code | `claude --version` output | checked 2026-11-07 |
| P4: The progress budget is 100/150 calls, 20/30 minutes, 5/10 minute status deadlines, and progress.py is the one home | code | `C:\Users\Patrick\.claude\gates\progress.py` lines 14-16 | checked 2026-11-07 |
| P5: The ref gate refuses untrailered commits on any branch and any move of `main` without a review; review verdicts are written per tip into `reviews.jsonl`; the cleanup sweep removes only worktrees `main` holds; the delete rule trusts only `.claude\worktrees\` | code | `C:\Users\Patrick\.claude\gates\README.md` lines 47, 61-75, 85 | checked 2026-11-07 |
| P6: `isolation: worktree` cuts from `origin/main`, which was 361 commits behind on 2026-11-07; executors' worktrees are cut by hand with `git worktree add ... main` | code | `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\agent-worktree-base-is-origin-main.md` lines 11-15 | checked 2026-11-07 |
| P7: Agent transcripts sit at `<project>\<session>\subagents\agent-<id>.jsonl` with a `.meta.json` holding `agentType`; assistant records carry `requestId`, `message.model`, `message.usage` and an `effort` key | code | the Hydra project folder `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test`, 126 `subagents` folders, 1,604 agent transcripts, one `.meta.json` read in full | checked 2026-11-07 |
| P8: 1,591 of 1,604 agents in this project matched their parent session's `effort`; 0 of 1,619 Agent calls carried an `effort` key; 1 agent changed effort mid-run | unchecked | re-derived this session from the transcripts in P7 with a scratch script that no permanent doc may cite; the handoff `docs/handoffs/2026-11-07-Cost measurement and the planned controlled test of agent effort (high vs medium)-handoff.md` reports the same shape for 2,197 agents | T1 |
| P9: Per-agent means and medians since 2026-10-19 (executor $2.31 mean / $2.09 median high, $1.30 / $1.17 medium; reviewer $1.14 / $0.77 and $0.45 / $0.33; scout $0.55 / $0.13 and $0.44 / $0.41; planner $5.99 / $5.93 and $4.33 / $3.90; executor max $6.98) | unchecked | same scratch rebuild as P8; the handoff's effort table gives medians within $0.21 for executors, reviewers and planners | T1 |
| P10: The price table (Fable $10/$50/$0.25, Opus $4/$20/$0.20, Sonnet $2/$10/$0.20, Haiku $0.10/$0.50/$0.01; cache writes 1.25 times input) | unchecked | the handoff's "Prices used" paragraph, which cites the `claude-api` skill cached 2026-10-06; the skill was not opened this session | T1 (the script's comment names the skill path once opened) |
| P11: The frontmatter `effort` field overrides the session level but not `CLAUDE_CODE_EFFORT_LEVEL`; the per-call `effort` parameter overrides the field and needs v2.1.292 or later; forks do not take the parameter | unchecked | `https://code.claude.com/docs/en/sub-agents`, "Choose an effort level" and the frontmatter table, read 2026-11-07, page cut at 100,000 characters | LS-0b |
| P12: An agent on another model than the session does not get the session's level; Opus 5.5, Sonnet 5.5 and Haiku 5.5 default to `medium`; the resolution order is env var, session pick, `--effort`, settings, model default | unchecked | `https://code.claude.com/docs/en/model-config`, "Adjust effort level", read 2026-11-07 | LS-0b |
| P13: Fable 5.1 and the 5.5 models support `low`, `medium`, `high`, `xhigh`, `max` | unchecked | `https://code.claude.com/docs/en/model-config`, the levels table | LS-3 (a `low` scout runs) |
| P14: Claude Code 2.1.316 adds `CLAUDE_CODE_SUBAGENT_EFFORT` and stops session inheritance | unchecked | web search result, a blog at `https://pasqualepillitteri.it/en/news/28714/claude-code-subagent-effort-variable-sub-agents` | T4 (the version check stops the test if the build changes) |
| P15: With tests available, effort made no difference in one study; cost rose 2.4 to 4.5 times from low to xhigh in another; low took 12 turns and cost more than medium in a single-run blog test | unchecked | `https://latitude.so/blog/reasoning-effort-only-matters-without-tests`, `https://arxiv.org/pdf/2608.01347`, `https://wmedia.es/en/tips/claude-code-subagents-cheaper-low-effort` | question 5 (they shape the size only) |
| P16: The Agent tool's text says the parameter is ignored for forks and should be set only when the user or instructions ask | unchecked | relayed in this task's brief by the main session; this planner has no Agent tool to read | LS-0b (the main session reads the tool text when it launches) |
| P17: The four executor tasks named in T2 merged with `(reviewed CLEAN)` and are the merges `32973b96`, `735d2daa`, `8ad1fc65`, `60f7a5b3` | code | `git log --merges --oneline -25 main` in `C:\Users\Patrick\Downloads\Hydra\hydra-test` | checked 2026-11-07 (that they are non-UI and have their own test file is unchecked, settled by T2) |
| P18: `testdata/input` holds the checked-in corpus for test runs (5 folders) | code | `C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input` listing | checked 2026-11-07 |
| P19: Agent rules 1, 2 and 8 say no full suite, fixed models per role, no library run without the user's chat permission | code | `docs/agents/agent-rules.md` lines 7-8 and 14; `docs/agents/brief-preamble.md` lines 41-43 | checked 2026-11-07 |
| P20: The handoff checker printed 9 ok, 0 missing, 14 manual on the handoff | code | `py -I tools/check_handoff.py` output in this worktree, 2026-11-07 | checked 2026-11-07 |
| P21: The user said "Yes, write the plan" | ruling | this task's brief, as the main session quotes it; not yet on a session card | checked 2026-11-07 (the card writer adds it) |
