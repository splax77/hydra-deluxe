# Seed findings

These four were found before the audit started. Each is verified in T5 like any candidate. The figures are the plan's Premise map rows, named in brackets.

| Seed | What | Where |
|---|---|---|
| S1 | Agents are told to type a time (`HH:MM`) that the status helper could read itself; agents have no clock, so they make times up. 50 of the 52 newest timed status files end in a time more than 3 minutes from the file's write time (P6). Today's card writer ran 169 seconds by the hook's record (P5; 170,558 ms by the harness, P24), wrote times 20:19 to 20:42, and stopped "at the 20 minute limit". | `gates\status_append.ps1` lines 19 to 21 and 46; `docs/agents/brief-preamble.md` line 13; `tools\session_cards\writer-brief.md` line 100; `gates\progress.py` line 58 (P1 to P4) |
| S2 | The call, time and status budgets are owned by `progress.py` lines 14 to 16 and restated in agent rule 7, the preamble, `model-postures.md` and the card writer's brief; the brief turns the hook's 20-minute warning into an agent-timed stop. | `gates\progress.py` lines 13 to 16; `docs/agents/agent-rules.md` line 13; `tools\session_cards\writer-brief.md` line 101 (P3, P4, P7) |
| S3 | Which model runs each agent type is owned by the `model:` line of each agent file and restated in agent rule 2, `model-postures.md` and the preamble's commit form. | `agents\*.md` line 4; `docs/agents/agent-rules.md` line 8; `model-postures.md` line 5; `docs/agents/brief-preamble.md` line 67 (P8, P9) |
| S4 | `delete_rule.py` and the preamble name `hydra-test\.claude\worktrees` as Hydra's worktree folder, while git's worktrees, the audit's own among them, sit under `hydra-test\worktrees`. | `gates\delete_rule.py` lines 17 and 18; `docs/agents/brief-preamble.md` line 43 (P18) |
