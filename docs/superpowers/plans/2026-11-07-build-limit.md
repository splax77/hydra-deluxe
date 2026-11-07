# Only main sessions build, at most three at once (2026-11-07)

## The plain version

On 2026-11-07 the machine sat at 100 percent CPU because parallel agents each started a full-width build. The user ruled what happens next: three builds at once at most, strictly enforced. Agents never start a build. Only a main session builds, one build at a time per session. Any test that needs a build is run by the main session too.

The old limit failed because it was opt-in. `tools/build_slot.ps1` holds three slots, but only `tools/new_worktree.ps1` and `tools/mutation_probe.ps1` ask for one. A plain `build_cpp.ps1` run never asks.

The new limit works like a ticket booth. A main session asks a new tool, `hydra_build_pass`, for a pass. The tool hands one out only if a slot is free and that session has no build running. `build_cpp.ps1` refuses to run from Claude Code without a valid pass. A hook refuses the tool to every agent, so an agent can never get a pass.

The hook reads two structured fields, the agent id and the tool name, so nothing parses shell text. That matters, because agents and main sessions look identical from inside a shell. An agent's environment listing matched the main session's on every line, so `build_cpp.ps1` cannot tell them apart by itself. Only the hook input can.

Nothing changes in Hydra itself. No screen, score or stored record moves.

## How a build runs

The main session calls `hydra_build_pass`. It gets back either a pass, or a line saying who holds the slots, and it never blocks. With a pass, it runs its normal command with the pass in the environment:

`$env:HYDRA_BUILD_PASS='<pass>'; .\build_cpp.ps1 -Target hydra_tests`

`build_cpp.ps1` claims the slot with its own process id, builds, and gives the slot back pass or fail. A slot whose owner process has died frees itself, as it does today. A pass nobody claims runs out, so a session that forgets one cannot hold a slot.

A second lock stops anything that skips `build_cpp.ps1`. A `Directory.Build.targets` file at the repo root makes every Visual Studio project fail to build under Claude Code without a live pass. That covers `cmake --build` or MSBuild run by their full paths.

Both locks apply only when `CLAUDECODE` is 1, the same test the git ref gate uses. So CI and the user's own terminal are not gated.

## What this costs

The times below come from the file times of today's build folders under `.claude\worktrees`. Each span runs from the first object file to the last exe, for a cold `hydra_tests` build, and leaves out configure. They are single readings.

| Builds at once | Seen today | Average per build |
|---|---|---|
| One | 108 to 127 s each (eff-clock-1 to 6, fx-45, fx-55) | about 110 s |
| Three | 263 s and 269 s per group of three (fx-1 to 3; fx-4, 5 and 8) | about 89 s |
| Six, no limit | 646 s for six, 14:05:08 to 14:15:54 | about 108 s |

The six-build row is the weakest, because a later small rebuild could have stretched it.

The first cost is speed. One session now builds one branch at a time. Ten branches take about 18 minutes (10 times 110 s, worked out, not seen). Today nine builds through the slots took 15.8 minutes. Three sessions building together still use all three slots.

The second cost is slower feedback, and it is the larger one. An agent can no longer compile its own work, so it hands back code nobody has compiled. Every compile error and test failure now costs a trip through the main session and a fresh agent. Questions 1 to 3 cover what happens then.

The main session also carries more of the work. It builds every branch and runs every build-dependent test, which `C:\Users\Patrick\.claude\model-postures.md` today gives to executors.

## What this does not stop

An agent that deliberately forges a pass or a lock file could still build, because the pass lives in plain files. The locks stop habit and accident, not tampering. The `Directory.Build.targets` lock covers Visual Studio presets only, so a hand-run Ninja build skips it, though `build_cpp.ps1` still guards the Ninja presets. A branch cut before this merges has neither lock, and neither do old release checkouts. Rule 6 keeps its text: agents still run in parallel, and only building moves to the main session.

## Rulings that bind this plan

The user, 2026-11-07: "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session."

Asked what one build per session means, the user chose "One at a time (Recommended)". Asked how agents get a build, the user wrote: "any tests that require a build are done by the main session, not agents."

## Questions

1. A build or test fails before review. Who fixes it? Recommended: a fresh executor on the same branch, given the failure output. The main session then builds again. Rule 5 says nothing goes back to the executor, and this keeps to that, because the fixer is a new agent.
2. A reviewer can no longer compile its own fixes. If its C++ fix then fails the main session's build, who fixes that? Recommended: a fresh executor fixes it and a fresh reviewer reads only that fix. This is a narrow exception to D107's "no second round", so it needs the user's yes.
3. Failing-first tests need a build, so agents can no longer run them red. Recommended: the main session runs a test red only when a brief asks for it. Otherwise the reviewer reads the test and says what would make it fail. Running every test red would double the builds.
4. Should builds run at low priority, through MSBuild's `-lowPriority` switch? Recommended: yes. Three full-width builds can still fill the CPU, and a permission hook timed out under load. Builds take longer while the machine is busy with something else.
5. Should the user's own terminal builds count toward the three? Recommended: no, the same way the git gate leaves the user's terminal alone. The cost is that a terminal build on top of three Claude builds makes four.
6. How long may a pass sit unclaimed? Recommended: 2 minutes. This is a new number, so it needs the user's yes.

## Tasks

T1 and T2 touch different repos and need nothing from each other, so they run at the same time. Each gets one executor on Opus and one reviewer. Under the new rule neither can build, so the main session builds T1's branch and runs its tests.

### T1: The pass, both locks and the Hydra docs (Opus)

Goal: a build under Claude Code needs a pass, and a session gets at most one live pass out of three.

Files: `tools/build_slot.ps1` keeps the slot rule and gains the pass, the session id and the expiry. `tools/hydra_ui_mcp.py` gains `hydra_build_pass`. `build_cpp.ps1` checks and claims the pass. A new `Directory.Build.targets` holds the second lock. `tools/new_worktree.ps1` stops building and only makes the worktree. `tools/mutation_probe.ps1` and `installer/build_installer.ps1` run on the pass they are given. `docs/agents/agent-rules.md` and `CLAUDE.md` gain a rule 14. `docs/agents/brief-preamble.md`, `docs/agents/derive-once-review.md`, `docs/agents/integrate.md`, `docs/agents/ui-testing.md`, `docs/development.md` and `docs/agents/rulings/agents-and-workflow.md` say the new rule, with the user's words quoted.

Acceptance criteria, each with a test using `-SlotDir` fixtures:

- A fourth pass is refused while three are live, and the refusal names the holders.
- A second pass for the same session is refused while its first is live.
- A build with no pass stops before configure when `CLAUDECODE` is 1, and runs when it is not.
- `cmake --build` without a pass fails under `CLAUDECODE=1`, seen on a small target.
- A pass past its limit frees the slot, and so does a dead owner.
- `tools/mutation_probe.ps1` holds one pass across its rebuilds without taking a second slot.
- If question 4 is yes, a running cl.exe reads below normal priority.

### T2: The hook and the hook-side pages (Opus)

Goal: no agent can get a pass.

Files, in a worktree of `C:\Users\Patrick\.claude`: `gates/percall.py` refuses `mcp__hydra__hydra_build_pass` in `_subagent`, and the refusal says agents hand back and the main session builds. `gates/README.md`, `agents/executor.md`, `agents/reviewer.md` and `model-postures.md` say that agents neither build nor run tests that need a build.

Acceptance criteria: a test proves an agent's call is refused, a main-session call passes, and the refusal lands in the refusal log.

### T3: Check it on the next real fan-out (main session)

After both merge, the main session reads the file times of the next group of builds. No more than three may overlap, and none from one session may overlap. It also reads peak memory and paging during that group. It then updates the `parallel-by-default` memory note with the new rule.

## Promise map

| Promise | Task | Check |
|---|---|---|
| At most three builds at once | T1, T3 | fourth-pass test; file times of the next fan-out |
| One build at a time per session | T1, T3 | second-pass test; file times |
| Agents never start a build | T2 | hook test |
| No build under Claude Code without a pass | T1 | no-pass tests on `build_cpp.ps1` and `cmake --build` |
| A dead or forgotten pass never holds a slot | T1 | expiry and dead-owner tests |
| Tests needing a build run in the main session | T1, T2 | the reviewer reads the changed pages |
| Nothing changes in Hydra | T1 | the reviewer reads the diff: no file under `src/` changes |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `build_cpp.ps1` asks for no slot. | code | `build_cpp.ps1:88` to `build_cpp.ps1:102` | checked 2026-11-07 |
| P2: Only two scripts ask for a slot, and `tools/compare_db.py` only tells a person to use the wrapper. | code | `tools/new_worktree.ps1:47`, `tools/mutation_probe.ps1:314`, `tools/compare_db.py:41` | checked 2026-11-07, `git grep` of the scripts |
| P3: The slot limit is three, recorded in D49. | code | `tools/build_slot.ps1:35`, `docs/audit/2026-10-03-fix-decisions.md:150` | checked 2026-11-07 |
| P4: The installer and the mutation probe build through `build_cpp.ps1`, and CI does too. | code | `installer/build_installer.ps1:54`, `tools/mutation_probe.ps1:235`, `.github/workflows/ci.yml:85` | checked 2026-11-07 |
| P5: A dead owner's slot frees itself today. | code | `tools/build_slot.ps1:47` to `tools/build_slot.ps1:50` | checked 2026-11-07 |
| P6: In a subagent `percall.py` routes to `_subagent`, and hook input carries `agent_id` and the tool's name. | code | `C:\Users\Patrick\.claude\gates\percall.py:95`, `C:\Users\Patrick\.claude\gates\README.md` "What the probes found" | checked 2026-11-07 |
| P7: The git ref gate acts only when `CLAUDECODE` is 1. | code | `C:\Users\Patrick\.claude\gates\README.md` "Making a change live" | checked 2026-11-07 |
| P8: Hooks must not parse free text where a chokepoint exists. | ruling | `docs/agents/rulings/` hooks rulings, and the user's 2026-10-31 words in `C:\Users\Patrick\.claude\gates\README.md`: "trying to ban specific command patterns is a losing battle because there will always be another pattern nobody thought to check." | checked 2026-11-07 |
| P9: Three builds at once, agents never build, one per session. | ruling | the user's message of 2026-11-07, quoted in `## Rulings that bind this plan` and in the reviewer's brief | checked 2026-11-07 |
| P10: Rule 5 says nothing goes back to the executor, and rule 6 says never to queue independent tasks. | code | `docs/agents/agent-rules.md:11`, `docs/agents/agent-rules.md:12` | checked 2026-11-07 |
| P11: Executors today run tests and may build, and reviewers may build. | code | `docs/agents/brief-preamble.md` "Builds", `docs/agents/derive-once-review.md:93`, `C:\Users\Patrick\.claude\model-postures.md:9` | checked 2026-11-07 |
| P12: The hydra server is started from the project's `.mcp.json`. | code | `.mcp.json` | checked 2026-11-07 |
| P13: MSBuild has a low-priority switch. | code | `MSBuild.exe -help` of the 2026 Build Tools | checked 2026-11-07 |
| P14: Build times are about 110 s alone, 89 s per build at three and 108 s at six. | unchecked | object and exe file times under `.claude\worktrees`, read 2026-11-07 | T3 |
| P15: The slot folder last changed at 12:01:07 while builds ran until 14:15. | unchecked | folder time of `C:\Users\Patrick\.claude\hooks\state\build_slots` | T3 |
| P16: Four builds gave 100 percent CPU, 84 percent memory and about 33,000 pages per second, and a hook timed out. | unchecked | `side-session brief`, another session's report; the machine was idle when this plan was written | T3 |
| P17: An agent's shell environment matched the main session's on every variable that names Claude, an agent or a session. | unchecked | one Haiku scout's listing compared with the main session's on 2026-11-07, no file holds it | T2 |
| P18: The hydra server sees the session id in `CLAUDE_CODE_SESSION_ID`, and each session has its own server. | unchecked | `.mcp.json` starts it per client; the variable was seen in a shell only | T1 |
| P19: Generated `.vcxproj` files read a `Directory.Build.targets` at the repo root. | unchecked | `Microsoft.Common.targets` names the import; the chain from `build-cpp\hydra_tests.vcxproj` to it was not traced | T1 |
| P20: Low priority reaches cl.exe through reused MSBuild helpers and costs nothing on an idle machine. | unchecked | `MSBuild.exe -help` says only that MSBuild runs low | question 4 |
