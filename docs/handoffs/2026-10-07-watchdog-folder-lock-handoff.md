# The stall watchdog locks worktree folders and never stops (handoff, 2026-10-07)

Worktree folders under `.claude\worktrees` keep getting left behind as empty folders that Windows won't delete. The cause is the stall-watchdog hook, not git or the worktrees. Nothing has been changed yet. The user asked for this note before choosing a fix, so get their go before editing any hook or stopping any process. Every file named here is under `C:\Users\Patrick\.claude\hooks\` unless the path says otherwise.

## What happens

1. The first time a session dispatches an agent, the PostToolUse hook `watchdog_start.ps1` (matcher `Agent|Workflow`) starts `agent_watchdog.ps1`. That's a hidden Windows PowerShell 5.1 process that checks agent transcripts once a minute.
2. It starts it with `Start-Process` and no `-WorkingDirectory` (`watchdog_start.ps1:60`). So the watchdog inherits the folder the session is in at that moment.
3. Windows won't delete a folder that is any running process's current folder. If the session was in a worktree, that worktree's folder is locked while the watchdog runs. `git worktree remove` then deletes everything inside but leaves the empty folder, and reports "Permission denied".
4. The watchdog has two ways to exit (`agent_watchdog.ps1`, lines 555-559): a stop file `state\watchdog_<session>.stop`, or the session transcript sitting idle for `MaxHours` (8 by default, line 7). Nothing writes the stop file. Only the watchdog itself and its test mention it, and at 23:15 there were 0 stop files in `state\`. So in practice every watchdog lives until its session has been idle for 8 hours.

## Evidence

- `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\allzero-e1` is empty, and every delete fails with "being used by another process". That includes deletes from a shell started after the session that used it had ended.
- Watchdog pid 2520 (session 1e3b55fb-7857-48ba-b1b7-102a0f588902) started at 22:55, while that session's working folder was the allzero-e1 worktree (made at 22:41).
- This is the likely holder but not proven: no handle-listing tool (Sysinternals `handle.exe`) is installed, and no other process started from that session is still alive. The cheapest proof is to stop pid 2520 and try the delete again.
- Three other empty folders (busy-solomon-f03007, eloquent-feynman-66bec2, keen-meninsky-c6d0f2) deleted fine on 2026-10-07. They are probably the same thing from earlier sessions whose watchdogs had already timed out. That is a guess; nothing recorded who held them.
- At about 23:15: 14 `agent_watchdog.ps1` processes alive, the oldest started at 7:22 that morning, and 79 `watchdog_*.pid` files in `state\`.
- `watchdog_start.ps1` is the only hook that starts a long-lived process. Every other `Start-Process` under `hooks\` is either in a test or is text a gate matches, so this one line is the whole leak.

## Proposed fix (offered to the user, not started)

1. **Start the watchdog outside any project.** Add `-WorkingDirectory $hookDir` to the `Start-Process` call in `watchdog_start.ps1`. Add a hook test: start a watchdog from a temporary folder, then check that the folder can be deleted while the watchdog is still running. The existing watchdog tests are `tests\test_h1_stall.ps1` and `tests\test_h22_watchdog_monitor.ps1`. Follow their fixture style (session ids that start with `test-`, and every state file removed at the end).
2. **Free the locked folder now.** Stop pid 2520, then delete the empty allzero-e1 folder. This loses nothing: the next agent dispatch finds the pid dead and starts a fresh watchdog, with the fixed folder.
3. **Optional: make watchdogs end with their session.** Stop the 13 other stale watchdogs. Then give the watchdog a real way to end, for example a SessionEnd hook that writes `state\watchdog_<session>.stop`. A shorter `MaxHours` would also work, but it can stop a watchdog in a session that is only paused. Clean out pid files whose process is gone. Ask the user which way they want before building it.

## Rules that apply

- Fix hooks, don't route around them, and every hook fix comes with a test (memory "fix-hooks-dont-bypass").
- A hook that fails must fail loudly: deny with a reason or show a visible warning, never hang and never block the user's prompt (memory "fail-loudly-never-freeze"). `watchdog_start.ps1` already swallows errors into `{}`. Keep it that way, or make a failure visible, but never let it block the Agent call.
- Hooks run under Windows PowerShell 5.1. State files are read and written through `lib\state_file.ps1` (memory "fleet-control-hooks").
- Stopping processes and editing `~/.claude/hooks` changes the user's own setup, so each step needs the user's yes in chat.
