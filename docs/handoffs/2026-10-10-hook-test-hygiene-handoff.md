# Handoff: hook test suite hygiene (2026-10-10)

**Superseded 2026-10-10.** The Hook setup audit session (local_0d39a830) reported that the user put all hook work under `docs/superpowers/plans/2026-10-10-hook-overhaul.md`. That plan keeps wave 1 (harness, runner, h39, h40, `_archive`) and absorbs the rest. The same session reported the user's rulings where the two plans conflicted: prune state at 7 days, no subagent dispatches at all (spawn depth 1 replaces no_recursive_delegation_gate, so the Q5 answer below is moot), and no ffmpeg kill. Start nothing from this note.

**For the next session: build this plan after the dispatcher session merges, and ask the user one question first.** On 2026-10-10 the user read a summary of this plan and replied "approve all recommendations". That yes covers the design under repo rule 3. One question in it has no recommendation (Q5, below), so it is still open. Nothing has been built yet.

The plan is `docs/superpowers/plans/2026-10-10-hook-test-hygiene.md`. Read it first; this note only records what was decided and how it fits with the other work running now.

## Why this exists

A read-only audit on 2026-10-10 (`docs/audit/2026-10-10-test-suite-audit.md`; the hook scout's evidence is `docs/handoffs/2026-10-10-test-audit/D-hooks.md`) found the hook test suite split into two generations. The newer tests (h15 to h38) copy the hooks to a temp folder and are well built. The older ones (h1 to h13, h29, and parts of h2, h3, h21 and h30) run the live hooks against the live `hooks\state` folder. h8, h9 and h10 depend on a real 35-day-old transcript. The helpers are pasted into 20 to 33 files, and there is no runner. Six hooks have no test at all. `suite_run_gate.ps1` blocks harmless read-only searches. 193 backup files and 6,524 state files have piled up.

The planner found why the old tests could never be isolated. Thirteen hooks name their own folder by absolute path, so a temp copy still reads and writes the live folders.

## What the user decided

One shared harness, `tests\lib\harness.ps1`, replaces the pasted helpers. One runner, `run_hook_tests.ps1`, prints a single table and total, and only the main session runs it. Pester is not used: Windows ships only 3.4.0, and its mocks can't reach the child PowerShell process a hook runs in.

The 193 `.bak-*` and 2 `.retired-*` files move into `hooks\_archive\`. They are not deleted. Putting `~/.claude/hooks` under git was named as the better long-term answer, but it deserves its own decision later.

State files older than 14 days get pruned by a new `prune_state.ps1`, run once a day from `watchdog_start`. The prune must run once with `-WhatIf` and its output shown to the user before it deletes anything. The derive-once and doc-review record folders are excluded from pruning.

The watchdog keeps its rule of killing any ffmpeg older than 15 minutes, and that rule only gets a test. The test uses a renamed copy of `powershell.exe` as a stand-in, never the real ffmpeg name. The user's yes accepted this; if they run ffmpeg by hand while a fleet is up, the narrower parent-process rule is the alternative the plan describes.

## How to run it

Wave 1 starts only after the dispatcher session merges. It holds task 1 (the harness), with task 2 and task 10 beside it. Wave 2 holds tasks 4, 9 and 11, plus the task 6 tests for hooks with no absolute self-path. Wave 3 holds the task 5 batches, each carrying its task 3 path edits and task 7 and 8 cases, plus the rest of task 6. Every hook edit goes through a temp copy and that hook's own test first, because a broken hook blocks every agent. The absolute-path fallback line stays in every self-reference. test_h19's fail-closed scan runs as a canary. The main session runs the full hook suite once after each wave. Executors run on Opus, and each change gets one fresh Sonnet reviewer under D107.

## Fits with other work

The "Hook dispatchers: fail closed on a broken gate" session owns `dispatch_bash_ps.ps1`, `dispatch_star.ps1`, `lib\verdict_merge.ps1`, the outer try/catch audit of the gates and test_h21's weak assertions. This plan must not touch any of those. Three gates in that session's try/catch audit get this plan's one-line path edit last.

The memory file `fleet-control-hooks-2026-09.md` has stale claims, which the plan lists under "Stale notes for the main session". The main session fixes that memory; no task here does.

## Answered 2026-10-10 (session d40295e5)

The dispatcher fix was still unbuilt: its session wrote a design and stopped, waiting for the user's yes. The user chose to start wave 1 anyway, since tasks 1, 2 and 10 touch none of the dispatcher files. Later waves still wait where they overlap with that fix.

Q5: the user chose "block Sonnet/Haiku subagents". The gate must read the subagent's own model, from `agent_transcript_path` when present, else `<session>\subagents\agent-<agent_id>.jsonl` found from `transcript_path` (deletion_shrink_gate's `Get-Brief` already does this). Any Sonnet or Haiku subagent is denied an Agent call, whoever launched it. The task 6 test pins that.

Task 10 left one backup in place: `dispatch_bash_ps.ps1.bak-p0-3a`, because test_h20 checks it exists. Task 9 deletes that check and moves the file.

For task 9: the HT-2 executor found that `suite_run_gate` does not stop a subagent running `run_hook_tests.ps1`. Rule 3 needs `test_h*` in the command text, and rule 4 counts only named test files. Task 9 should make the gate name the runner too. Check this against the gate before building.

## Q5 as first asked

Q5: what is `no_recursive_delegation_gate` meant to check? It reads the model from `transcript_path`. A memory note says that path is the parent session's transcript when a subagent calls a tool. The planner didn't verify that note; its author says they verified it live on 2026-09-04. If the note is right, the gate denies when the parent is Sonnet, not when the subagent is. If the intent is "a Sonnet subagent may not dispatch", the gate needs a different source for the model. Verify the note's claim against a real transcript first, then ask the user which behaviour they want. Only after that should the test pin it.
