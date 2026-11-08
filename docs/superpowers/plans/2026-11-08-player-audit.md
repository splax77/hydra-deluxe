# Plan: a full audit of the playing code (2026-11-08)

Session 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e. The user's request, in their words: "do a full audit of the playing code before making any changes. i dont trust any of this anymore."

## What this audit is for

The auto-player plays Clone Hero on a virtual drum kit and aims for Hydra's top-path score. Session 6341c618 found that one run missed 14 notes because the game ran an old build of the kit agent, and nothing failed loudly. The same session gave the user several false causes before it found that one. So the user no longer trusts any part of the chain.

This audit reads every piece of code between Hydra's top path and a press landing in the game, plus every piece that judges a run afterwards. It looks for the kinds of problem that caused the last session's misses and false reports. It changes no code. Fixes come later, in their own plan, after the user rules on the findings.

The audit follows `docs/agents/audit-method.md`: plan, calibrate, find, verify, pick each finding's exit, check every closure, ask the user, record. Every finding leaves by one of that page's four exits: fixed, a question to the user, covered by a ruling quoted in the user's words, or not a problem. Only the main session picks an exit. Since this audit changes no code, nothing leaves as "fixed" during it.

## What the audit reads

"The playing code" is read here as everything a play run executes or reports from. The plain version: how the plan is made, how it is sent, how the game receives it, and how the result is judged.

The plan is made by `hydra_replay`'s schedule (`tools/replay_schedule.cpp`, `tools/replay_schedule.h`, `tools/replay_json.cpp`, `tools/replay_json.h`, `tools/replay.cpp`), and read into Python by `tools/ch_mcp/dump_facts.py`.

The plan is sent by the player, `tools/ch_probe/experiments/play_chart.py`, and the modules it imports: `tools/ch_probe/constants.py`, `engine_finder.py`, `engine.py`, `process.py`, `input_driver.py`, and the experiments `live.py`, `overlay_logs.py` and `walk_edges.py`. `engine.py` is a Python model of the game's engine, so it gets a close read for rules that Hydra already owns.

The game receives it through the `ch` server in `tools/ch_mcp/`: `server.py`, `kit_tools.py`, `bot_run.py`, `game_tools.py`, `sp_tools.py`, `trace_hits.py`, `frida_tools.py`, `frida_attach.py`, `front.py`, `quiet.py`, `game_files.py`, `menu_tools.py`, `method_watch.py`, and the agents in `tools/ch_mcp/agent/` (every `.ts` source, the built `.js` files in the main checkout, and `package.json`, which builds them).

The result is judged by `tools/ch_probe/experiments/score_compare.py`, `sp_compare.py` and `sp_watch.py`, `tools/ch_mcp/compare_scorestats.py`, `machine_recorder.py` and `machine_report.py`, and the run folders under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`.

The tests for all of the above, under `tools/ch_mcp/tests/` and `tools/ch_probe/tests/`, and the C++ schedule tests, are read too. They are read, not run.

## Questions before the audit starts

Two exclusions narrow the user's words, so each is a question before the audit starts (memory rule: any narrowing of the user's words is its own question).

1. **Does "the playing code" include Hydra's path search?** Hydra's engine picks the top path and its score. The player starts from that output. The plan reads how the player and the schedule read the engine's output, but not how the engine finds the path. Option (a): leave the engine out; it has had its own audits (`docs/audit/2026-10-03-derivation-audit.md`, `docs/audit/2026-10-11-concept-audit/`). Option (b): include it. Recommended: (a), because every miss and false report so far came from the player side. The case against: if the engine's path or score were wrong, the player would look wrong, and this audit would not find out why.

2. **Are the one-off probe experiments in scope?** `tools/ch_probe/experiments/` holds scripts no play run imports: `active_probe.py`, `analysis.py`, `edge_probe.py`, `find_checksum.py`, `key_delivery_test.py`, `log_tail.py`, `milestone1.py`, `pad_flash_test.py`, `passive_probe.py`, `poll_cost.py`, `topmost_probe.py`, `watch_checks.py` and `watch_window.py`. Option (a): leave them out. Option (b): include them. Recommended: (a), since they do not run in a play. The case against: a fact the player relies on may have been first measured by one of them, and a wrong measurement would carry over. The game-facts family below checks those facts against the Clone Hero reference anyway.

## What the audit reads first

Before any finder starts, the main session and every brief point at what is already decided, so nothing the user ruled on shows up as undecided:

- the player plans under `docs/superpowers/plans/`: `2026-11-07-player-limits.md`, `2026-11-07-player-refinements.md`, `2026-11-07-ch-run-timing.md`, `2026-11-08-kick-axis-bind.md`, `2026-11-08-player-freak-fixes.md`, `2026-11-08-replay-press-schedule.md`, and the removed `2026-11-06-optimal-score-run.md` (`git show 2a3b47f8abf9c2621721fa8d59fbad7373ec56f2:docs/superpowers/plans/2026-11-06-optimal-score-run.md`);
- the rulings in `docs/agents/rulings/`, most of all `clone-hero.md` and `tests-and-runs.md`;
- `docs/audit/2026-10-03-fix-decisions.md` for D-records;
- the Clone Hero reference under `docs/clone-hero/`, and `docs/agents/clone-hero-mcp.md`;
- the session cards in `docs/sessions/` tagged `clone-hero`, found by grep of `docs/sessions/index.md`.

The handoff from session 6341c618 (`docs/handoffs/2026-11-08-session-6341c618-findings-handoff.md` on branch `claude/handoff-6341c618-findings`) is another agent's summary. Its findings go to the verifiers as candidates to check, not to the finders as facts, so the finders do not anchor on it.

## The kinds of problem the audit looks for

Finders split by family of questions, not by folder, as the method says. There are nine families.

1. **Built is not source.** Is the code that runs the code in git? This covers the built agents against their `.ts` sources, the worker that `ch_reload` swaps in, Python modules cached in a running server, and the `hydra_replay` exe the player calls against its source. For each, does anything refuse a mismatch?
2. **Silent paths.** Every place a missing field, an odd reply, an exception, a timeout or a refusal is logged and the run goes on, or filled in with a default (`.get(key, default)` and the like). The last session's 14 misses went through one of these.
3. **Rules worked out twice.** Anything the Python works out that Hydra's engine, the schedule or the Clone Hero reference already states: hit windows, leads, Star Power ends, frames, fills, squeezes, scores. Each copy is a home, with a table of inputs and answers.
4. **Notes left out or moved.** Every code path that presses fewer or other gems than the chart has, or presses away from a note's time. Each one is traced to a ruling in the user's words or a proven game fact, or it becomes a question.
5. **The timing chain.** Every conversion between chart milliseconds, the game's clock and frames, and every fixed gap, lead or latency. Does each have one home, and what happens at its edges?
6. **How a run is judged.** How a run is called a pass or a fail, and where each reported number comes from. Can a run report a pass while it missed notes? Can a reported count disagree with the game's own stats?
7. **Tests that test fakes.** Does a test's fake reply match what the real agent sends? A test that fakes a field the built agent never sends passes while the run fails.
8. **Game facts.** Each assumption about Clone Hero in the playing code, checked against its block in `docs/clone-hero/` and that block's label.
9. **Focus and input isolation.** Can anything in a run bring the game window to the front, or need the user's keyboard, mouse or focus? The I Am Ahab run's window came to the front at 43.9 s with no cause found.

## The stages, with caps

Every agent below is a read-only scout on Sonnet (agent rule 2), started with the `Agent` tool. Every brief starts with `docs/agents/brief-preamble.md`, then carries the items in "What every audit brief carries" in `docs/agents/audit-method.md`, the status rule, and its caps. No agent builds, runs `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`, starts the game, or runs over the library. The audit needs no live run. A claim that only a run can settle goes on the open list with the run that would settle it, and that run waits for the user's go.

Each agent's cap is 120 tool calls or 30 minutes, whichever comes first. At the cap it stops and reports what it read, what it found, and what it did not reach. Unread files go on the open list; they are never assumed clean.

### S1: Calibrate (1 agent)

 One scout gets family 2 (silent paths) and `tools/ch_probe/experiments/play_chart.py` at `fd93c6846dd6599d0a7c63e8d0e1799fa0566a0b`, and nothing else. The known problem it must find is one the last session confirmed; the brief does not describe it, even as an example. If it misses it, the main session stops, tells the user, and fixes the brief before anyone else starts.

### S2: Find (4 agents, at the same time)

 Each takes two or three families across all the files in scope:

- Finder 1: families 1 and 7 (built against source, and tests that test fakes).
- Finder 2: families 2 and 6 (silent paths, and how a run is judged).
- Finder 3: families 3, 4 and 8 (rules worked out twice, notes left out, game facts).
- Finder 4: families 5 and 9 (the timing chain, and focus).

A finder reports candidates. Each candidate names its homes with file and line, gives a table of inputs and answers with the edges in, and writes every split in its table as its own line. A finder does not grade its own candidates.

### S3: Verify (2 fresh agents, at the same time)

 Verifier 1 takes finders 1 and 2's candidates. Verifier 2 takes finders 3 and 4's, plus the 6341c618 handoff's findings. Each checks every candidate against the code, marks each claim "by reading" or "proven by" a named run file, and lists every split as its own line. Side notes outside its job are findings too.

### Stop rule

 One find round and one verify round. There are no completeness rounds. If the verifiers' side notes turn up a new family of problem, that is a question to the user, not a second round started by the main session.

### S4: Pick each finding's exit (main session)

 For each finding, and each split inside it, the main session picks one exit and writes its evidence as the method says. A finding with no ruling that names both sides becomes a question.

### S5: Check every closure (1 fresh agent)

 One scout that found or verified nothing in this audit checks every exit 3 and exit 4, and returns PASS, REOPEN or GONE for each. The main session then greps every quoted ruling in its cited file; a quote that is not there turns into REOPEN.

### S6: Ask the user

 Questions go out five or fewer per message, each written as the method's "How to write a question" says: the code's real predicate, both sides' values, an example from each end and one from the gap, what the user will see, the options, a recommendation and the case against it.

### S7: Record

 The audit's files go in `docs/audit/2026-11-08-player-audit/`: the candidates, the verified findings, each finding's exit and its evidence, the open list, and the questions. Each answer becomes a D-record in `docs/audit/2026-10-03-fix-decisions.md`. The folder is committed on its own branch and goes through the doc review before it merges.

That is eight agents in all: one calibrator, four finders, two verifiers and one closure checker. Agent rule 7's status line applies to each.

## What comes after the audit

Nothing in the handoff's "What is next" list starts during the audit. That list is: rebuild `kit.js`, the guards, LS-M2 again, the TD merge, the kick plan's T4, and the replay plan's TP1 to TP4. The fix plan comes after the user has ruled on the findings, and goes through its own review and the user's yes.

## Live steps

None. The audit is read-only and starts no game, build or run.

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P0: The user asked for "do a full audit of the playing code before making any changes. i dont trust any of this anymore." | ruling | the user's message in session `31e4f22f-bf4a-4106-bbf3-bcd268d0d75e`, quoted in the reviewer's brief | checked 2026-11-08 |
| P1: The built kit agent the game runs dates from 2026-11-06 21:31 and has no `after_ms` or `groupsOf`. | code | `C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\ch_mcp\agent\kit.js` | checked 2026-11-08 by Get-Item and Select-String (0 matches) |
| P2: The kit agent's source has `after_ms` and `groupsOf`, on 8 lines. | code | `tools/ch_mcp/agent/kit.ts` | checked 2026-11-08 by Select-String |
| P3: `kit.ts` last changed in commit `2709008f`, on 2026-11-07. | code | `tools/ch_mcp/agent/kit.ts` (`git log`) | checked 2026-11-08 |
| P4: Main's tip is `fd93c6846dd6599d0a7c63e8d0e1799fa0566a0b`. | code | `git log -1 main` in `C:\Users\Patrick\Downloads\Hydra\hydra-test` | checked 2026-11-08 |
| P5: The player imports `constants`, `engine_finder`, `engine`, `process`, `input_driver`, `live`, `overlay_logs`, `walk_edges` and `kit_tools`. | code | `tools/ch_probe/experiments/play_chart.py` import lines | checked 2026-11-08 |
| P6: The player imports a Python engine model, `EngineModel` and `pressed_input_hit`, from `tools/ch_probe/engine.py`. | code | `tools/ch_probe/experiments/play_chart.py` import lines | checked 2026-11-08 |
| P7: No play run imports the thirteen experiments listed under question 2. | code | `tools/ch_probe/experiments/play_chart.py` import lines | checked 2026-11-08 |
| P8: The four run folders the 6341c618 handoff names exist. | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m0-run1`, `ls-m1-run1`, `ls-m2-run1` and `...\2026-11-07\ls-f1-run1` | checked 2026-11-08 by Test-Path |
| P9: The audit's stages, exits and brief contents are those of the audit method page. | code | `docs/agents/audit-method.md` | checked 2026-11-08 |
| P10: Every family can be judged by reading code and the existing run folders, with no new run. | unchecked | `docs/handoffs/2026-11-08-session-6341c618-findings-handoff.md` only reports that the last session found its misses from run files | S3 |
| P11: Hydra's path search had its own audits. | unchecked | `docs/audit/2026-10-03-derivation-audit.md` and `docs/audit/2026-10-11-concept-audit/` are named, not opened for this | question 1 |