# TOOL-AUDIT-T5-VS: seed verification (all claims by reading; nothing run)

## S1 CONFIRMED (changed in one detail)
Homes read:
- `gates\status_append.ps1` line 20 joins the agent's text as the line; line 21 writes `(Get-Date).ToString('HH:mm') + ' (no message)'` only when no text is given; line 46 appends. So the helper already has a clock and uses it only for the empty case.
- `gates\progress.py` line 58: `_helper_line` shows `"HH:MM done ... | next: ..."`. Preamble line 13 and the card writer's brief line 100 (`"HH:MM done <n> of <m> cards | next: <session>"`) tell the agent to type the time.
- Seen now in `gates\state\status`: the newest files, last written 21:11, end in `00:00 done ...`, `10:00 done ...`, `11:00 done ...`. That agrees with the seed (agents invent times).
- The 50-of-52 and 169 s figures were not recomputed (P5, P6, P24 left open; would need a run over the status folder and the hook records).
Split: typed time (agent) vs file write time (helper clock available, unused). Also a third home: the stall hook's evidence line. Not read here.
Ruling: none searched for in rulings (not asked); not cited.

## S2 CONFIRMED
- `progress.py` lines 13 to 16: the numbers 100/150 calls, 20/30 minutes, 5/10 status minutes, comment "The one home of these numbers".
- Restated: `agent-rules.md` rule 7 (5 minutes); preamble lines 13, 21 and 73 (5, 10, 100, 150, 20, 30); `model-postures.md` ("100 tool calls and 20 minutes"); `tools\session_cards\writer-brief.md` line 100 (5 minutes) and line 101 ("At 20 minutes or 100 tool calls, stop starting new cards").
- Situation where they differ: the card writer's line 101 turns the hook's warning into a rule the agent times itself. Agents have no clock (S1), so the stop is the hook's warning, not the agent's own count. Also the writer's brief says stop at the warning but nothing says the hook's 30-minute refusal; the preamble does.
Split lines: progress.py vs preamble vs rule 7 vs model-postures vs writer-brief. Each restates a number.

## S3 CONFIRMED-CHANGED
- Owner `model:` lines (`~\.claude\agents\*.md` line 4): card-writer haiku, executor opus, scout sonnet, reviewer sonnet, planner fable.
- Restated in `agent-rules.md` rule 2, `model-postures.md` (line 5, long line, matched), preamble line 67 (`Co-Authored-By: Claude Opus 5.5`).
- Differences found: (a) rule 2 and model-postures say scouts run on "Sonnet or Haiku"; `scout.md` carries only `model: sonnet`, so Haiku scouts are not what the owner says. (b) The preamble trailer is a fixed Opus name for every agent type; it says "Use the model line your brief gives if it differs". This session's own attribution reminder names a different model. No gate hit for `Co-Authored-By` in `gates\*.py`, so no hook checks it (searched gates\*.py only).
Split lines: scout model (file: sonnet; rule 2: sonnet or haiku); trailer model (preamble: Opus; real agent: varies).

## S4 CONFIRMED-CHANGED (the seed's premise is partly wrong)
- `gates\delete_rule.py` lines 17 and 18: WORKTREE_FOLDERS = `...\hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`. Preamble line 43 names the same two.
- `git worktree list` from the main checkout: 3 worktrees under `hydra-test/.claude/worktrees` and 96 under `hydra-test/worktrees` (including `tooling-audit-records` and `tooling-audit-plan`). Both folders exist.
- So the rule's list is missing the folder that holds almost all current worktrees. `model-postures.md` says the delete rule trusts "a `worktrees` folder"; the code names only the `.claude` one for Hydra. Effect: a recursive delete inside `hydra-test\worktrees\<name>` would not pass as an allowed root (by reading; a hook run would settle it, not run). The seed's wording "git's worktrees sit under hydra-test\worktrees" is true for most, not all.
Split lines: delete_rule.py + preamble (`.claude\worktrees`) vs actual locations (`worktrees`, 96 of 99 Hydra worktrees) vs model-postures wording ("a worktrees folder").

Not reached: none. Not settled by reading: S1 counts (P5, P6, P24).
