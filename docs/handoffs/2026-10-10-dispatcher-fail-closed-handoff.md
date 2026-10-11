# Handoff: make the hook dispatchers fail closed (2026-10-10)

Nothing is built yet. This note holds the findings, the rule the user chose and the design waiting for approval. The hooks live in `C:\Users\Patrick\.claude\hooks`, outside this repo.

## The bug

The two dispatchers each run several gates in one process and print one merged verdict. `dispatch_bash_ps.ps1` runs 13 gates for Bash and PowerShell calls. `dispatch_star.ps1` runs two gates on every tool call. A broken gate silently lets the call through, in three ways.

If a gate's file is missing, the dispatcher skips it (`dispatch_bash_ps.ps1` line 68, `dispatch_star.ps1` line 45). If a gate throws, the dispatcher catches the error, sets the output to nothing and skips it (lines 75 and 49). If a gate prints something that is not JSON, `Merge-GateVerdicts` in `lib\verdict_merge.ps1` treats it as "no opinion" (line 38).

A fourth hole sits one level up. If the dispatcher itself crashes, for example inside `Merge-GateVerdicts`, it exits non-zero and prints nothing. Claude Code treats a non-zero exit other than 2 as a non-blocking error, so the call goes ahead.

## The rule the user chose

The user chose that a gate that prints nothing at all is broken, whatever its exit code ("Deny always").

So the full rule is this. A gate's turn counts only if it printed a JSON object. `{}` counts and means "no opinion". If the object has a `permissionDecision`, the value must be `allow`, `ask` or `deny`. Every other case turns that gate's turn into a deny:

1. The gate file is missing.
2. The gate threw an error.
3. The gate printed nothing, whatever its exit code.
4. The gate printed text that is not a JSON object, such as a stray word or a bare number.
5. The gate printed a decision word other than allow, ask or deny.

Why empty output is safe to deny: I parsed all 15 gates with the PowerShell parser (`System.Management.Automation.Language.Parser`). Every `exit` statement in every gate comes right after a statement that prints, and each gate's last top-level statement prints too. So no healthy gate prints nothing.

Why the exit code is never read: the exit code can be left over from a native command the gate ran, such as a `git` call that returned 1. If the dispatcher read it, a healthy gate could be denied by mistake. Because empty output denies anyway, the dispatcher has no need for the exit code. It only quotes the code in the reason.

## The proposed design (not yet approved)

The user has not said yes to building this. Rule 3 in `CLAUDE.md` says the build waits for that yes.

**Where the rule lives.** One new function in `lib\verdict_merge.ps1` decides whether a gate's printed line is a valid verdict, and says what is wrong when it is not. Both dispatchers call it, so the rule has one owner. Missing files and thrown errors are caught in each dispatcher's own gate loop. Those two loops already match line for line, and test_h27 runs both, so a drift between them would fail a test.

**What the deny says.** It reuses the wording of `Write-FailClosedDeny`, the helper each dispatcher already uses when a helper library will not load. That helper prints straight to stdout today. It needs a sibling that returns the JSON line instead, so a broken gate's deny joins the list before merging. Then if a real gate also denies, `Merge-GateVerdicts` lists both reasons and Claude fixes them in one retry. Example reason: "dispatch_bash_ps cannot check this call: the gate C:\Users\Patrick\.claude\hooks\rerun_gate.ps1 is missing or broken (it printed nothing, exit code 1). Restore that file; until then this gate denies every call." The next line reads "Instead: restore C:\Users\Patrick\.claude\hooks\rerun_gate.ps1, then run the same command again."

**The catch-all.** Each dispatcher's whole gate loop and merge go inside one try/catch that calls `Write-FailClosedDeny`. That closes the fourth hole above.

**The merge helper's comment.** `lib\verdict_merge.ps1` says on line 14 that empty or unreadable lines count as "no opinion". That stops being true for gate output, so the comment changes to point at the new check.

One question to test before relying on it: does a gate's runtime error reach the dispatcher's `catch`? A gate runs with `&` under the dispatcher's `$ErrorActionPreference = 'SilentlyContinue'`. A `throw` will reach it. An error that stops a single statement, such as calling a method on `$null`, may be swallowed and let the gate carry on. Check this with a stub gate before writing the "gate threw" test.

## Gates with no outer try/catch

I count seven gates whose main logic runs outside any try/catch, not the audit's five. The audit did not name its five, so I could not compare lists.

Four have no try around their main logic at all: `bash_risky_gate`, `bash_windows_path_gate`, `shell_read_gate` and `agent_progress_gate`. Three do have a large `try`, but with only a `finally` and no `catch`, and much of their logic runs before it: `fanout_track`, `fable_delegate_gate` and `suite_run_gate`.

Once the dispatcher fix lands, a crash in any of these seven becomes a deny, so none of them needs its own outer catch for safety.

The other eight gates have an outer catch. Most of them deny in that catch only when the command is one they guard, and print `{}` otherwise; that is a per-gate choice. Two of them always print `{}` in their outer catch, which lets the call through and hides the error from the dispatcher. They are `rerun_gate` (line 70 to 188) and `stall_inject` (line 301 to 353). `stall_inject` only adds reminders and decides nothing, so this is harmless there. For `rerun_gate` it is fail-open inside the gate. That is a follow-up for the user to decide and is not part of this fix.

## Tests to write

**New test file.** It goes in `C:\Users\Patrick\.claude\hooks\tests` and is built like `test_h27_verdict_merge.ps1`. It copies each dispatcher into a temp folder next to stub gates that carry real gate names, so it never runs the live hooks and never touches the live `hooks\state`. One check per broken case, on both dispatchers: a missing gate, a gate that throws, a gate that prints nothing with exit 0, one that prints nothing with exit 1, one that prints a non-JSON word, and one that prints an unknown decision word. Each check asserts the decision is exactly `deny` and that the reason names the gate's file. One more check should pair a broken gate with a stub that denies on its own, and confirm the merged reason holds both.

**Fix test_h21.** Lines 99 and 112 of `test_h21_no_ask.ps1` assert only that the decision is not `ask`. A crashed gate gives no decision, so these checks pass even when the gate is broken. Each should assert the exact decision. The expected decisions were not measured yet. Run each gate on test_h21's inputs once, take its decision from that run, and pin that literal. Note that test_h21 runs the live hooks and writes `hooks\state\fanout_test-h21-no-ask.json`. That is how it already works, and changing it is outside this fix.

Run only these two test files. Do not run the whole hook test set.

## Where the survey scripts are

The four parser scripts used for the survey are in this session's scratchpad, outside the repo, under `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\58a990f4-15f7-41de-a07f-f9fba21f8912\scratchpad\` (`survey.ps1` to `survey4.ps1`). They only read the gate files. `survey4.ps1` is the one behind the "every exit prints first" claim. Its check is a pattern match on the statement before each `exit`, so read it before trusting the claim.
