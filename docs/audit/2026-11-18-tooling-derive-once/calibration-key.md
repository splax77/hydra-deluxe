# Calibration answer key (T1)

Written and committed before the calibration scout starts. No brief holds this text. The scout reads seven files: `gates\progress.py`, `gates\status_append.ps1`, `docs/agents/brief-preamble.md`, `tools\session_cards\writer-brief.md`, `agents\card-writer.md`, `docs/agents/agent-rules.md` and `model-postures.md`. It passes only if it finds all three items below on its own.

| Item | The problem it must find | Counts as found when the report |
|---|---|---|
| K-A | The status line's time is typed by the agent, although the helper that writes the line can read the clock. | names the agent-typed time (`HH:MM` in the preamble, the writer brief or progress.py's reply) and that `status_append.ps1` could stamp it, as a guessed value or a value with two owners |
| K-B | The card writer's brief restates the hook's budget numbers (20 minutes, 100 tool calls) that `progress.py` owns. | names writer-brief.md's numbers as copies of progress.py's constants |
| K-C | The card writer's brief turns the hook's warning at 20 minutes into a stop the writer times itself. | says the brief's rule does something different from the hook's (a stop, or self-timing, against a warning), not only that the numbers match |
