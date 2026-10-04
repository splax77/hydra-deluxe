# Handoff: hooks read their input in the wrong encoding

This work is about the user-scope hooks in `C:\Users\Patrick\.claude\hooks`, not about Hydra itself. The last session surveyed the problem and wrote a fix plan. The user has not said yes to the plan yet, so **nothing has been edited**. The next session should get that yes first, then do the fix.

No agents or workflows are running. The handoff hook's journal check listed nothing in flight.

## The problem in plain terms

Claude Code sends each hook its input as UTF-8 text. Most hooks read it with `[Console]::In.ReadToEnd()`. Under Windows PowerShell 5.1 that decodes with the console code page (437), not UTF-8. So an accented letter arrives garbled: "café" becomes "caf├⌐". The user measured this on 2026-10-04.

The deletion guard (`deletion_shrink_gate.ps1`) is where it hurts most. When a file path has an accented letter, the garbled path doesn't exist. The guard then takes its "new file" exit and allows the deletion unchecked, even for a subagent whose brief never named the file.

The user's probe script shows this. An ASCII name is denied and an accented name is allowed. It sits in another session's scratchpad: `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test--claude-worktrees-objective-maxwell-56ed0c\def8b3e1-0508-44f9-b9dd-c1965068fd63\scratchpad\probe_utf8.ps1`. Set `PROBE_NONAME=1` for the control run.

The guard has a second problem. Its brief check (`Get-Brief`) reads the subagent transcript with `Get-Content` and no `-Encoding`. PS 5.1 then decodes UTF-8 as code page 1252. Once stdin is fixed, a brief that does name an accented file would stop matching it, and the gate would wrongly deny.

## What the survey found

**A second way past the deletion guard.** Found by reading the code; no test has shown it yet. When an Edit's `old_string` has an accented letter, the garbled text isn't found in the file (`deletion_shrink_gate.ps1` around line 309). The guard assumes the Edit tool will reject the call, and allows it. So the bypass works through the deleted text too, not only the file name.

**Hooks that likely make wrong decisions today.** These were judged by reading what each hook matches on; no test has proved it yet.

- `fable_delegate_gate` and `fanout_track` match on the file path when they run alone for Edit/Write.
- `spec_preflight_gate` reads spec file paths out of an Agent prompt.
- `workflow_script_gate` reads `scriptPath`.
- `dispatch_star` reads stdin garbled and passes that text to `agent_progress_gate` and `stall_inject`.

**Hooks that are already right.** `dispatch_bash_ps`, `derive_once_review_gate` and `git_attribution_gate` decode the raw bytes as UTF-8 themselves. The six gates that `dispatch_bash_ps` runs get its decoded text: `bash_risky_gate`, `bash_listing_gate`, `git_attribution_gate`, `rerun_gate`, `fanout_track` and `fable_delegate_gate`. The dispatcher sets `$global:HookInputDecoded` and points `[Console]::In` at a StringReader.

**Garbled but harmless today.** The rest only look at plain-ASCII fields:

- `posture_inject`, `fanout_reset` and `watchdog_start`
- `handoff_gate`, `stop_orchestrator_gate` and `workflow_monitor_reminder`
- `no_recursive_delegation_gate`, `no_background_in_subagent_gate` and `executor_dispatch_brief_gate`
- `monitor_batch_gate` and `derive_once_waiver`

**Inline hooks were not surveyed.** The grep covered only the `.ps1` files in `hooks\`. At least one hook is written inline in a `settings.json` command and also reads `[Console]::In.ReadToEnd()`: the Stop hook that counts bullet lines in `last_assistant_message`. Garbled text probably doesn't change its line count, but check every settings file for inline hooks before calling the survey complete.

**Transcript reads with no UTF-8 setting.** The deletion guard's `Get-Brief` is the one that matters. Four others only check record types or build an ASCII slug, so they're harmless today:

- `agent_progress_gate` (`Get-FirstUserText`)
- `git_attribution_gate` (the task-key slug, around line 113)
- `agent_watchdog`
- `stop_orchestrator_gate`

`handoff_gate` already passes `-Encoding UTF8`, and `transcript_lib.ps1` decodes UTF-8 bytes itself.

## The plan waiting for the user's yes

1. Add one small helper in `hooks\lib`. It reads the raw stdin bytes and decodes them as UTF-8, dropping a leading BOM. When `$global:HookInputDecoded` is set, it takes the dispatcher's text instead.
2. Make every hook call that helper instead of `[Console]::In.ReadToEnd()`. The two hooks with their own decoding (`derive_once_review_gate`, `git_attribution_gate`) switch to it too, so the rule lives in one place.
3. Make `dispatch_star` decode once and set `$global:HookInputDecoded`, the same way `dispatch_bash_ps` does.
4. Add `-Encoding UTF8` to the five transcript reads listed above.
5. Add red-then-green cases to `tests\test_h18_shrink_guard.ps1`. Run them against the current hook first to watch them fail, then against the fixed hook. Three cases:
   - an accented file name in a subagent whose brief doesn't name it → deny;
   - an accented `old_string` → deny;
   - a brief that does name the accented file → allow.
6. Add a raw-bytes case for one gate that runs under a dispatcher.
7. Add a source-scan test that fails if any hook reads `[Console]::In` directly, or reads a transcript without UTF-8. This is the same idea as the long-paths scan test.
8. Back up every hook before replacing it, as `<name>.bak-2026-10-04`. `deletion_shrink_gate` already has `.bak-2026-10-03` and `.bak-2026-10-03b`, so don't overwrite those.
9. Run every `tests\test_*.ps1` under Windows PowerShell and report the results. Don't commit anything without asking.

## Questions still open for the user

1. **Plan approval.** Get the user's go-ahead on the plan as a whole.
2. **Shared helper or inline copies.** The plan uses the shared helper. A hook that runs alone then loads one extra file, and if that file is missing the hook errors and lets the call through. The alternative is a four-line copy in each hook. The last session recommended the shared helper.
3. **Status files.** `stall_inject` and `status_batch_monitor` read agent status files without UTF-8, so relayed status text could be garbled. Status files aren't transcripts, so this is outside the brief. Check what encoding `status_append` writes before changing the readers. The user hasn't said whether to include this.

## Traps for whoever does the fix

**Feed tests raw bytes.** Feed stdin as raw UTF-8 bytes through `System.Diagnostics.Process`, the way Claude Code does. Piping a JSON string from PowerShell into `powershell.exe` turns é into `?`, so that isn't a faithful test. The existing h18 harness pipes (`Invoke-Gate`, around line 55: `$json | & powershell ...`). The new cases need their own raw-bytes feeder.

**`sed` is blocked.** A user-scope Bash hook blocks any command containing the word `sed`. Use the Read tool, `awk`, or `grep -n` to look at line ranges.

**Baseline before any change.** On 2026-10-04 all 18 test files exited 0. h17 printed "173 passed, 0 failed" and h18 printed "40 passed, 0 failed". The summary checks for the other files matched only PASS lines. A FAIL line in the middle of a file wasn't searched for, so grep for FAIL in the final run.
