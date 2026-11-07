# Handoff: cost measurement and the planned controlled test of agent effort

Session `de983c9b-0e3c-485a-90c5-06602204cf08`, written 2026-11-07.

## Goal

The user wanted to know whether the `squeez` token optimizer was worth using, then what actually drives token cost in this setup and where the biggest savings are. The open goal is a controlled test of whether agents at medium effort do the work as well as agents at high effort, for less money.

## What was done

**squeez was judged and rejected.** It was audited, then built from source in a throwaway home folder and run on real Hydra output. Nothing was installed. It saved 94.0% on 12 read-only Hydra commands, but mostly by deleting. In the tests it hid new test failures on reruns and hid one-line file changes. It marks most Bash commands "allow", which skips permission prompts. Do not install it.

**A full cost accounting was run** over every local Claude Code transcript, read-only. The total was $4,698 at list API prices, nearly all from Oct 14 to Nov 7. It excludes this session and 385 calls on older models. The bucket, role and model totals are exact sums of recorded usage; cache reads closed to within $1. Content shares and savings are estimates.

| Cost type | Dollars | Share |
|---|---|---|
| Cache reads | $2,175 | 46.3% |
| Cache writes | $1,447 | 30.8% |
| Output, including thinking | $1,075 | 22.9% |

| Role | Dollars | Share |
|---|---|---|
| Main sessions (Opus) | $1,640 | 34.9% |
| Executors (Opus) | $1,461 | 31.1% |
| Reviewers (Sonnet) | $669 | 14.2% |
| Planners (Fable) | $452 | 9.6% |
| Scouts (Sonnet or Haiku) | $386 | 8.2% |

Prices used, per million tokens as input / output / cache read: Fable 5.1 $10 / $50 / $0.25, Opus 5.5 $4 / $20 / $0.20, Sonnet 5.5 $2 / $10 / $0.20, Haiku 5.5 $0.10 / $0.50 / $0.01 (up to 100K tokens of context; $0.50 / $2.50 / $0.05 above that). Cache writes cost input times 1.25 (5 minutes) or times 2 (1 hour). These came from the `claude-api` skill, cached 2026-10-06. The Haiku cache read rate is the stated general 0.1 times input, not a listed figure.

**The savings found.** They overlap, so do not add them up.

| Change | Possible saving | Risk |
|---|---|---|
| Agents at medium effort, not high | 9–19% | Quality unproven |
| Sonnet scouts to Haiku | about 6% | Some; agent rule 2 allows it |
| Compact main sessions at 200–300k tokens | 3.3–3.6% | Lost context |
| Status lines sent alongside another call | up to 3.7% | Low |
| Trim starting context | 2–3% | Low |
| Group back-to-back Read and Grep calls | up to 2.8% | Low |
| Shell output hook | 2–3%, scaled from an earlier estimate | Medium |
| Inject the prompt hook less often | about 1% | Low |
| Split the preamble | about 0.4% | Low |

Three more moves would break agent rule 2, so only the user can choose them: reviewers to Haiku (about $563), executors to Sonnet (about $443), planners to Opus (about $256).

**The effort numbers.** These are medians per agent since Oct 19, same role and model:

| Role | High | Medium | Median calls, high to medium |
|---|---|---|---|
| Executor | $2.08 | $1.25 | 42 to 22 |
| Reviewer | $0.77 | $0.30 | 26 to 13 |
| Sonnet scout | $1.52 | $0.60 | 47 to 20 |
| Planner | $5.73 | $4.11 | 16 to 16 |

The 19% ceiling is that gap applied to every high-effort agent since Oct 19, taken against about $2,932 of spend in that period. The 9% assumes half of the gap is real.

**How effort really behaves.** Effort almost never changes inside a run. Of 218 main sessions, 4 ever held more than one effort value. Of 2,197 agents, 1 changed mid-run. Agents take their parent session's effort: 2,181 of 2,197 matched it, including every executor, reviewer and scout. None of the 2,165 Agent calls passed an `effort` argument, and no agent definition sets one.

So the high and medium agents above came from different sessions doing different work. That is why the gap is unproven: it mixes effort with task difficulty.

## What is next

Nothing here is approved to run. The user answered the question "Should I plan a controlled test of agent effort at high versus medium?" with "yes, create a handoff", so the next step is a plan, and the test itself still waits for the user's yes (agent rule 3). The test spends real money, so its budget needs that same yes.

1. **Write the test plan**, with a Premise map (`docs/agents/premise-map.md`), on a planner agent. The plan should compare the same work at both efforts, so task difficulty cancels out. Suggested shape, not yet agreed: take finished tasks from `main`, run each one from its base commit at high and at medium, per role, and score the results the same way. Reviewers can be scored on planted faults; executors on whether the task's tests pass and on what the fresh reviewer finds. Count rework, because a cheap run that needs redoing is not cheap. Give every step a cap.
2. **Settle how to set effort per agent.** The Agent tool has an `effort` parameter, and its description says agent definitions can set effort in frontmatter. That tool text also says to set the parameter only when the user or instructions ask. Neither route has been tried here. Claude Code's own docs on effort were not read this session, because the fetch tool was unavailable.
3. **Offer the low-risk changes separately.** Status lines and starting context need no quality test. Check first whether settings can remove unused connectors and skills, and whether agents can skip `MEMORY.md`; neither was checked.
4. **Look into the 18 agent cache rebuilds** that cost $69 with gaps under 5 minutes. Their cause is unknown.

The accounting scripts were scratch files and are gone with this session. To rebuild them: walk every `*.jsonl` under the transcript folder, including the `subagents` folders, whose `.meta.json` files give `agentType`. Take the last `message.usage` per `requestId`, and price it with `message.model`. The `effort` field sits on each assistant record. A `compact_boundary` system record marks a compaction.

## The user's words

From session `de983c9b-0e3c-485a-90c5-06602204cf08`:

- "give me a full detailed measurement of everything and point out where the biggest potential savings are"
- "yes, create a handoff. is claude able to dynamically decide which effort level is appropriate for a task? i have been operating under the assumption that everything runs at whatever level you set at the start of the session"
- "Yes, measure first"
- "Yes, look at it"

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| ruling | `de983c9b-0e3c-485a-90c5-06602204cf08` | "give me a full detailed measurement of everything and point out where the biggest potential savings are" | the session transcript holds those words |
| ruling | `de983c9b-0e3c-485a-90c5-06602204cf08` | "yes, create a handoff. is claude able to dynamically decide which effort level is appropriate for a task? i have been operating under the assumption that everything runs at whatever level you set at the start of the session" | the session transcript holds those words; it answers the question "Should I plan a controlled test of agent effort at high versus medium?" |
| ruling | `de983c9b-0e3c-485a-90c5-06602204cf08` | "Yes, measure first" | the session transcript holds those words |
| ruling | `de983c9b-0e3c-485a-90c5-06602204cf08` | "Yes, look at it" | the session transcript holds those words |
| path | `docs/agents/agent-rules.md` | Rule 2 sets each role's model; rule 3 says no work starts without the user's approval | read rules 2 and 3 |
| path | `docs/agents/brief-preamble.md` | Tells agents to write a status line "alone in its own call" | grep that phrase |
| path | `docs/agents/premise-map.md` | Says the shape of the Premise map a new plan carries | read it |
| path | `C:/Users/Patrick/.claude/gates/progress.py` | Its first-call note also says "alone in its own call" | grep that phrase |
| path | `C:/Users/Patrick/.claude/agents/executor.md` | Sets `model: opus` and no effort line | grep `^effort` in the agents folder finds nothing |
| path | `C:/Users/Patrick/.claude/agents/reviewer.md` | Sets `model: sonnet` and no effort line | same grep |
| path | `C:/Users/Patrick/.claude/agents/scout.md` | Sets `model: sonnet` and no effort line | same grep |
| path | `C:/Users/Patrick/.claude/agents/planner.md` | Sets `model: fable` and no effort line | same grep |
| path | `C:/Users/Patrick/.claude/projects/C--Users-Patrick-Downloads-Hydra-hydra-test` | The main transcript folder; other projects sit beside it | list it |
| state | `squeez is not installed` | No `~/.claude/squeez` folder exists and `settings.json` does not mention squeez | `ls ~/.claude/squeez`; grep squeez in `~/.claude/settings.json` |
| state | `total $4,698 at list price` | Cache reads $2,175, cache writes $1,447, output $1,075; main $1,640, agents $3,058. New sessions will move these totals | rebuild the sum as the prose says; expect a larger total |
| state | `effort is fixed per run` | 4 of 218 main sessions and 1 of 2,197 agents ever changed effort; 2,181 of 2,197 agents matched the parent session; 0 of 2,165 Agent calls passed `effort` | count `effort` values per transcript, and read Agent tool inputs |
| state | `the effort gap is unproven` | The per-agent gap since Oct 19 compares different sessions and tasks, so the 9–19% is an estimate, not a result | no controlled run exists; the plan must make one |
| state | `2,285 of 4,943 agent status calls were alone` | Those 2,285 were the only tool in their API call and cost about $176; the other 2,658 went out with other tools | count status_append calls per API request |
| state | `main sessions start near 96,000 tokens` | Median first-call context since Oct 19 was 96,036 for main sessions; agents ran 19,719 to 24,940 by role | sum the token fields of the first usage in each transcript |
| state | `agents load MEMORY.md` | 2,160 of 2,173 agent instruction records listed `MEMORY.md` | read the `instructions` attachment in agent transcripts |
| state | `compaction simulation` | Compacting main sessions nets $168 at 200k and $157 at 300k, and loses $32 at 150k; it assumed a reset to the start plus 12k and a 10k summary | a simulation, not a measurement; rebuild it to check |
| state | `no test has run and nothing changed` | The session changed no settings, agent definitions or Hydra code; its only commit holds this handoff | `git log main..handoff/cost-effort-test` |
| state | `claude code effort docs unread` | How Claude Code sets effort per agent was not checked against its docs; only the Agent tool's text and the `claude-api` skill were read | read the docs before the plan relies on it |

## Checker output

```
manual | ruling | de983c9b-0e3c-485a-90c5-06602204cf08 | "give me a full detailed measurement of everything and point out where the biggest potential savings are"
manual | ruling | de983c9b-0e3c-485a-90c5-06602204cf08 | "yes, create a handoff. is claude able to dynamically decide which effort level is appropriate for a task? i have been operating under the assumption that everything runs at whatever level you set at the start of the session"
manual | ruling | de983c9b-0e3c-485a-90c5-06602204cf08 | "Yes, measure first"
manual | ruling | de983c9b-0e3c-485a-90c5-06602204cf08 | "Yes, look at it"
ok | path | docs/agents/agent-rules.md | Rule 2 sets each role's model; rule 3 says no work starts without the user's approval
ok | path | docs/agents/brief-preamble.md | Tells agents to write a status line "alone in its own call"
ok | path | docs/agents/premise-map.md | Says the shape of the Premise map a new plan carries
ok | path | C:/Users/Patrick/.claude/gates/progress.py | Its first-call note also says "alone in its own call"
ok | path | C:/Users/Patrick/.claude/agents/executor.md | Sets `model: opus` and no effort line
ok | path | C:/Users/Patrick/.claude/agents/reviewer.md | Sets `model: sonnet` and no effort line
ok | path | C:/Users/Patrick/.claude/agents/scout.md | Sets `model: sonnet` and no effort line
ok | path | C:/Users/Patrick/.claude/agents/planner.md | Sets `model: fable` and no effort line
ok | path | C:/Users/Patrick/.claude/projects/C--Users-Patrick-Downloads-Hydra-hydra-test | The main transcript folder; other projects sit beside it
manual | state | squeez is not installed | No `~/.claude/squeez` folder exists and `settings.json` does not mention squeez
manual | state | total $4,698 at list price | Cache reads $2,175, cache writes $1,447, output $1,075; main $1,640, agents $3,058. New sessions will move these totals
manual | state | effort is fixed per run | 4 of 218 main sessions and 1 of 2,197 agents ever changed effort; 2,181 of 2,197 agents matched the parent session; 0 of 2,165 Agent calls passed `effort`
manual | state | the effort gap is unproven | The per-agent gap since Oct 19 compares different sessions and tasks, so the 9�19% is an estimate, not a result
manual | state | 2,285 of 4,943 agent status calls were alone | Those 2,285 were the only tool in their API call and cost about $176; the other 2,658 went out with other tools
manual | state | main sessions start near 96,000 tokens | Median first-call context since Oct 19 was 96,036 for main sessions; agents ran 19,719 to 24,940 by role
manual | state | agents load MEMORY.md | 2,160 of 2,173 agent instruction records listed `MEMORY.md`
manual | state | compaction simulation | Compacting main sessions nets $168 at 200k and $157 at 300k, and loses $32 at 150k; it assumed a reset to the start plus 12k and a 10k summary
manual | state | no test has run and nothing changed | The session changed no settings, agent definitions or Hydra code; its only commit holds this handoff
manual | state | claude code effort docs unread | How Claude Code sets effort per agent was not checked against its docs; only the Agent tool's text and the `claude-api` skill were read
checked 23 rows: 9 ok, 0 missing, 0 not-on-main, 0 not-pushed, 14 manual
```
