# TA-D: audit of the Claude Code hook test suite

Scope: the live hooks in `C:\Users\Patrick\.claude\hooks\` (47 top-level `.ps1`/`.sh` hooks plus 10 files in `lib\`) and their tests in `hooks\tests\` (37 live `test_h*.ps1` files, 2 replay tools, 52 backups). Nothing was run. Everything below comes from reading the code. About 50 tool calls, inside the cap.

The short version: the suite is custom (no Pester), it is mostly strong, and the newer half is very well built. The weak spots are a set of older tests that touch the live hooks folder and live state, a handful of hooks with no behaviour test at all, and one real hole in how the two dispatchers handle a crashing gate. The `.bak-*` clutter is large but harmless: nothing loads it.

How the suite is run. There is no runner script and no README. The only instruction is one sentence in the memory file `fleet-control-hooks-2026-09.md`: a `Get-ChildItem tests\test_h*.ps1 | ForEach-Object { powershell ... }` loop. Each test prints `PASS`/`FAIL` lines and (mostly) sets its exit code. I checked every test's tail for exit-code handling; all 37 exit 1 on failure except as noted in (b).

---

## (a) Working well

**The harness fits the job.** Each test builds the JSON a hook would receive, pipes it to the hook in a real child `powershell.exe`, and reads the printed decision. That is how Claude Code calls the hooks, so the tests catch quoting, encoding and stdin problems that an in-process call would hide. `test_h19_utf8_input.ps1:38-65` goes further and feeds raw UTF-8 bytes through `System.Diagnostics.Process`, with a fixture check (line 86) that proves the feeder adds no byte-order mark. This is careful work.

**The newer tests are isolated.** Tests h15-h18, h24-h28, h30-h38 copy the hooks they test into a temp folder (for example `test_h17_git_ref_gate.ps1:34-40`, `test_h30_library_run_gate.ps1:12-13`, `test_h38_merge_precheck.ps1:78-79`), build throwaway git repos, and so cannot touch the real `hooks\state`. h17 even checks that its own review records "never touch the real hooks\state" (lines 9-14).

**The dispatcher test uses stub gates.** `test_h27_verdict_merge.ps1` copies each dispatcher next to stub gates that carry the real gates' file names, and takes the expected text by running each stub alone (lines 4-9, 97-100) instead of retyping it. This follows the preamble's "derive once" rule. It checks deny plus deny, ask plus deny, reminder plus deny, a lone reminder, nothing at all, and a missing merge helper. It also has a `trap` (line 18) so a check that throws counts as a failure, never a skip.

**Fail-closed on a missing input helper is tested hard.** `test_h19_utf8_input.ps1:193-306` runs every gate (25 of them) and every quiet hook (9) from a scratch copy of the hooks folder, once with `lib\hook_input.ps1` missing and once with it throwing. It checks that gates deny with a reason naming the helper, that hooks which cannot refuse warn on stderr and block nothing, and that `handoff_gate` denies on PreToolUse but only warns on UserPromptSubmit. Line 213 also fails if a new hook calls `Read-HookInput` and nobody adds it to the list, so the list cannot go stale unnoticed. The `[scan]` section (lines 309-373) uses the PowerShell parser, not regex, and fails when an allow-list entry no longer matches anything.

**Time and counters are made deterministic by seeding state, not by waiting.** `test_h2_progress.ps1:108-114, 221-266` writes a state file with `firstCall` set to 12, 19, 21 or 31 minutes ago and checks the 10-minute/15-call demand, the 20-minute warning (once only), the 30-minute stop, and which tools still pass after the budget (`git`, `StructuredOutput`, `SubagentHandback`, status writes). That is the right technique. Both sides of each rule are covered.

**Assertions mostly check the decision and the reason text.** `test_h5_git.ps1:43-55`, `test_h10_status_brief.ps1:24-29`, `test_h8_bash_gates.ps1:34-39` all compare the exact decision (`deny`/`ask`/none) and a needle in the reason. The newer tests also check the last line of a deny starts with `Instead:` (`test_h20`, `test_h30`, `test_h27:91-95`), which enforces the user's hook-message contract.

**Hang and race cases exist.** `test_h17_git_ref_gate.ps1:857` injects a 25-second sleep into the gate to test the hang path. `test_h29_state_file.ps1` tests the locked state helper with real concurrent writers.

**Retired things were retired cleanly.** `bash_listing_gate.ps1` and `test_h14_listing_gate.ps1` both carry the `.retired-gatechange` suffix; the dispatcher no longer lists the gate, and `test_h14` is not picked up by the runner pattern. No live test refers to a retired hook.

---

## (b) Broken or misleading

**1. Both dispatchers skip a gate that crashes or has been deleted, which is fail-open. Severity: high.**
`dispatch_bash_ps.ps1:66-78` (and the same loop in `dispatch_star.ps1:43-52`) does this for each gate: if the file is missing, `continue`; run it with errors silenced; if it printed nothing, `continue`. `Merge-GateVerdicts` (`lib\verdict_merge.ps1:33-41`) then also drops any output that is not valid JSON or has no `hookSpecificOutput`, calling it "no opinion". So a gate that throws before printing, is renamed by mistake, or prints garbage simply stops protecting, and the call goes through. Many gates have their own outer `catch` that denies (for example `git_attribution_gate.ps1:586`, `library_run_gate.ps1:323`, `merge_precheck_gate.ps1:100`), but `bash_risky_gate`, `bash_windows_path_gate`, `shell_read_gate`, `fanout_track` and `fable_delegate_gate` have no outer try/catch at all, and `rerun_gate.ps1:185` deliberately swallows to `{}`. No test feeds the dispatcher a crashing, empty, or garbage-printing gate. `test_h27` (the natural home) only has stubs that print well-formed JSON, and `test_h19` only covers the missing-helper failure. The memory note's promise "hook failure = deny with reason" is therefore only true for the one failure mode that is tested. Fix: make the dispatcher deny (naming the gate) when a listed gate is missing, exits non-zero, or prints nothing/unparseable output, and add those four cases to h27.

**2. Nothing checks that a hook is actually wired up. Severity: medium.** `test_h19:215-233` reads `settings.json` for `handoff_gate` only. A test that `settings.json` parses, that every live hook file is either registered with the right matcher or listed in a dispatcher's `$gates`, and that no `.bak` is registered, does not exist. If someone removes a registration or a gate from `$gates` (`dispatch_bash_ps.ps1:47-61`), every behaviour test still passes because each test runs the hook file directly. Only `suite_run_gate` gets a "is in the $gates array" check (`test_h20:219`), done with a regex over the dispatcher's source. Fix: one parser-based wiring test over `settings.json` and both `$gates` arrays.

**3. `suite_run_gate` blocks harmless read-only searches. Severity: low.** `suite_run_gate.ps1:153` denies any command that contains the literal text `test_h*` together with any of `powershell`, `-File`, `foreach`, `ForEach-Object`, `%\s*\{`. I hit it twice during this audit with `Select-String` and `Get-ChildItem ... | % { }` over the test folder; neither runs a test. It is also easy to walk around (`test_h[0-9]*`, `tests\*.ps1`), so it blocks honest searches and does not stop a determined run. The rule is a text match, not a parse of what runs. Fix: only deny when the wildcard is the target of `&`, `-File`, `.`, `Invoke-*`, or `Start-Process`, and add a "listing and searching the tests is allowed" case beside the existing one at `test_h20:125`.

**4. `test_h21_no_ask` has a check that passes on a crash. Severity: medium.** Lines 99 and 112 assert `(Get-Decision $out) -ne 'ask'` for ten gates. `Get-Decision` returns `'unparseable'` for a crash or empty output (lines 48-53), and `'unparseable' -ne 'ask'` is true, so a gate that dies passes "does not ask". The test's own header says the point is "a hook never asks", so asserting the positive (decision is `none`) is what the cases intend. Fix: assert `-eq 'none'` where that is the expected outcome.

**5. Some test names say "reviewer" or "implementer" while the state they seed has an empty role.** Severity: low. In `test_h2_progress.ps1` the cases at lines 119-136 ("reviewer at 150 calls...") seed `role = ''`. `agent_progress_gate.ps1:169-175` then derives the role from the agent's first message, so what these cases exercise depends on the fixture transcript, not on the label. The reviewer-only addendum at `agent_progress_gate.ps1:254` ("You are the reviewer (D107)... list in your report every finding you did not fix") has no matching string anywhere in the test file (I searched for two phrases from it). So the D107 text on the hard stop is untested. Fix: seed `role = 'reviewer'` explicitly and assert line 254's text for both the call-budget and the time-cap deny.

**6. `test_h20_suite_gate.ps1:220` asserts a backup file exists.** Severity: low. The check `dispatch_bash_ps.ps1.bak-p0-3a exists` tests a leftover file, not behaviour. If the clutter in (c) is cleaned up, this test fails for no real reason. Fix: delete the check.

**7. Memory notes disagree with the code.** Severity: low. `fleet-control-hooks-2026-09.md` says (H5) every `git commit` needs `Task:`/`Agent:`/`Session:` trailers in `git_attribution_gate`. The test at `test_h5_git.ps1:64-69` and the gate say the opposite since 2026-10-03: trailers are judged by the git-side hook, and this gate "leaves it alone". The note also says `test_h14_listing_gate` exists with 18 checks (it is retired), says the suite is "260 PASS across 14 files" (there are 37 live files), and says "A hook never asks" while `test_h8_bash_gates.ps1:114-124` correctly expects `ask` from the main session for media writes outside the scratchpad (subagents are denied instead). The code and tests are consistent with each other; the memory is stale.

---

## (c) Unnecessary / dead

**Backup clutter: 193 `.bak-*` files plus 2 `.retired-*`, about 2.7 MB. Severity: low.**
By folder: 132 files (1.0 MB) beside the hooks, 11 (0.47 MB) in `lib\`, 52 (1.25 MB) in `tests\`. The 30 tagged `2026-10-04`, 28 `d46` and 27 `d46b` are the oldest generation. I checked the three places they could do harm:
- `settings.json` registers hooks only by exact live file names (`dispatch_star.ps1`, `dispatch_bash_ps.ps1`, `workflow_script_gate.ps1`, and the rest). No `.bak` path appears anywhere in it.
- The dispatchers load gates by explicit list (`dispatch_bash_ps.ps1:47-61`, `dispatch_star.ps1:38`), not by folder scan.
- Test scans use `-Filter '*.ps1'` (`test_h19:206, 334`). A name ending `.ps1.bak-x` does not end in `.ps1`, so the scans skip them. The memory's run command (`test_h*.ps1`) also skips them.
So none of it is loaded or run by mistake. The only reference to a backup from live code is `test_h20:220` (see b6).
They are noise in every directory listing and in every audit grep (a plain Grep over `hooks\` returns hits from backups unless excluded), there is no git repo under `~/.claude` to make them redundant, and five backups belong to a gate that no longer exists (`bash_listing_gate.ps1.bak-*`, `.retired-gatechange`, `test_h14...retired-gatechange`). Fix: move everything older than the current day's edits into one `hooks\_archive\` folder that no scan touches, or put `~/.claude/hooks` under git and delete the copies.

**`hooks\state` holds 6,524 files, 40 MB, with no pruning. Severity: low.** 1,939 `agent_*.json`, 1,310 files under `shrink_guard\` (the oldest from 2026-08-15), 1,866 status files, 232 doc-review records, 215 derive-once records, 194 `fanout_*`, 151 `reruns_*`. Every gate that reads or writes state works on this growing folder, and the status files include leftover names such as `test-runner.md` from 2026-09-15. A nightly or on-start prune of files older than N days would stop this. I did not find any test-run leftovers in it (the older tests do clean up after themselves, see d2).

**`transcript_lib.ps1` is a live shared library with no test.** It is used by 7 hooks (`fable_delegate_gate`, `fanout_track`, `rerun_gate`, `posture_inject`, `executor_dispatch_brief_gate`, `no_recursive_delegation_gate`, `fanout_mark_delegated`) and appears in no test. It only gets indirect coverage through whatever the hook tests happen to feed. It is not dead; I list it here because it is the one lib file that has nothing of its own.

No test file tests something that no longer exists, apart from the correctly retired h14.

---

## (d) Poorly designed, with the fix

**1. Two generations of tests, and the older generation uses the live hooks and live state. Severity: medium.** h1-h13, h29 and parts of h2/h3/h8/h9/h10/h11/h21/h30 hard-code `C:\Users\Patrick\.claude\hooks\...` and `...\hooks\state` (for example `test_h2_progress.ps1:5-6`, `test_h8:4-8, 64`, `test_h9:3-7`, `test_h10:4`, `test_h13:6-7`, `test_h21:15, 24`). They run the production hook file and write `fanout_test-h8...json`, `delegate_turn_test-h8*.json`, `status\test_h19_*.md` and similar into the production state folder, cleaning up at the end. Consequences: (a) a test run cannot be pointed at a candidate copy of a hook, so a bad edit is tested only after it is live; (b) a test that dies half way leaves files behind in the folder the fleet reads; (c) two test runs at once, or a test run while a real fleet is running, share state. The collision risk is low because session ids are prefixed `test-`, but a hook change to a key scheme would make that unsafe. The newer tests prove the temp-copy pattern works. Fix: give the old tests a shared helper that copies hooks into a temp folder and points them there, as h15/h30/h38 do.

**2. No shared test library; the same helpers are pasted into every file. Severity: medium.** `function Check` is defined in 33 files, `Get-Decision` and `Invoke-Hook` in 20 each, with small differences (some use `2>&1`, some `2>$null`; some set `$failed = 1`, some count). The differences are real bugs waiting: tests that use `2>&1` (h4, h8, h10) turn any new stderr warning line into "unparseable", and tests that use `2>$null` (h21) hide it. Fix: one `tests\lib\harness.ps1` with Invoke-Hook (stdin bytes, stdout and stderr read apart), Get-Decision, Get-Reason, Check, and an exit-code summary; migrate files as they are touched.

**3. No runner. Severity: medium.** The one sentence in memory is the only way to run the suite, and the agents' rule (preamble: never run all hook tests) plus `suite_run_gate` make a runner awkward. The main session has to copy a one-liner that fails with "wildcard" blocks for agents (see b3). Because each test prints in its own style (`RESULT:`, `N passed, M failed`, bare PASS lines, `h31: ...`), a total is impossible without parsing. Fix: a `run_hook_tests.ps1` for the main session only that runs every live `test_h*.ps1` in sequence, collects exit codes, and prints one table (name, exit code, seconds). Keep it behind the same gate.

**4. Real transcript fixture. Severity: medium.** `test_h8_bash_gates.ps1:6`, `test_h9_rerun.ps1:4` and `test_h10_status_brief.ps1:5` all point to the user's real session transcript `e81b56eb-a783-...jsonl` (6.3 MB, last written 2026-09-05, models `claude-fable-5-1` 624 times, opus 7, sonnet 3). The fable gate keys on the model it reads from that file. The file is a copy of a live user transcript: if Claude Code's transcript clean-up ever removes it, or if it is ever resumed, h8, h9 and h10 change behaviour or fail with no source change. The newer tests (h21:28-35) write their own two-line fixtures. Fix: replace with a tiny fixture written at test start.

**5. Real checkout as working directory. Severity: low.** `test_h5_git.ps1:32` defaults the hook's `cwd` to the real hydra-test checkout, and `git_attribution_gate.ps1:323-327` runs `git -C <cwd> rev-parse ...` there, so these cases depend on the real repo's state when `hookCwd` is not overridden. h2, h3, h20, h21, h30 also name the real repo path. Fix: use a temp repo for those cases, as 25 other h5 cases already do.

**6. Wall-clock and process-spawn cost. Severity: low.** Real sleeps total about 20 s across `test_h1_stall.ps1:165, 251`, `test_h12:119-140`, `test_h22:189-201`, `test_h29:155`, plus h17's 25-second hang case. They are bounded polls with deadlines, not open-ended waits. The bigger cost is one `powershell.exe` start per case (about 150-400 ms each on Windows PowerShell 5.1): h5 has about 200 cases, h15 about 240, h16 about 185 (scanner, mostly in-process), h25 about 110, h31 about 147, h17 runs real git in many throwaway repos. A full run is therefore several minutes. I did not measure it. Nothing needs the network: the only `https://` strings are test inputs for `gh` and `git config` parsing (`test_h5:736-740`, `test_h34`), not requests.

**7. Replay tools: useful, different job, one stale list. Severity: low.** `replay_extract.py` pulls every past Bash/PowerShell call out of the transcripts with its original outcome; `replay_shell_corpus.ps1` then pushes them through named gates and buckets new denies against old outcomes (false alarms, lost protection, contract breaks). That is a "measure a gate change against real history" tool and does not overlap the unit tests. `test_h26_replay.ps1` tests the tool itself with six fixture lines and stub gates, so it is not a duplicate. Two small problems: `OLD_RULES` in `replay_extract.py` still lists `blocked_listing` ("Directory listing with a Windows path"), a rule from the retired listing gate (correct for old history, but a reader will think it is live); and `replay_shell_corpus.ps1:69` points to a `_common.md` contract file that I could not find under `~/.claude`.

---

## (e) Gaps: hooks with no or thin tests

Counting a hook as covered only when a test makes it run and checks what it decides (a name in `test_h19`'s fail-closed list does not count):

**No behaviour test at all (medium each):**
- `no_recursive_delegation_gate.ps1` denies a Sonnet or Haiku session from dispatching an Agent. Its model check reads the transcript from the hook input, which the memory says is the parent's transcript for a subagent call (`fleet-control-hooks-2026-09.md:27`). I did not verify that claim; if true, the gate looks at the wrong model. A test with a Sonnet transcript and an Opus transcript would settle it. Its deny text has no `Instead:` line either.
- `no_background_in_subagent_gate.ps1` (denies `run_in_background` and Monitor inside subagents; the preamble's "never use run_in_background" rule rests on it).
- `fanout_mark_delegated.ps1` and `fanout_reset.ps1` (the other two halves of the fan-out counter that `fanout_track` reads; `test_h21` seeds the state file by hand instead).
- `posture_inject.ps1` (injects the model-postures text every prompt; only fail-closed is checked).
- `transcript_lib.ps1` (see c).

**Thin (low to medium):**
- `executor_dispatch_brief_gate.ps1`: `test_h10` covers rule 0 (status sentence) with five cases. Its other rules (model per role, task table, the "no Haiku" change from 2026-10-09) have no test.
- `agent_watchdog.ps1`: `Stop-StaleFfmpeg` (`agent_watchdog.ps1:162-176`) kills any `ffmpeg` process older than the limit, whoever started it, and has no test. It can end the user's own long video job (the fcvideo workflow uses ffmpeg). Either test it with a stand-in process name and the limit, or restrict it to ffmpeg started by an agent. Severity: medium, because the action is destructive and unverified.
- `stop_orchestrator_gate.ps1` / `workflow_monitor_reminder.ps1`: `test_h3` has about 10 checks for the Stop-blocking rule, including the "never twice within 3 minutes" and "zero tool calls in the turn" rules; the catch-and-ignore blocks (`stop_orchestrator_gate.ps1:26-177`, eight `catch { }`) are not exercised, so a transcript it cannot read is not known to be safe.
- `workflow_script_gate.ps1` (`test_h4`, about 9 cases): its "break on review failure needs a followup agent" rule describes the pre-D107 fixer/checker design. It has a `.bak-onereview`, so it was touched, but the rule text should be checked against the current one-reviewer policy.
- `status_append.ps1`: used by every agent every few calls, only exercised indirectly (h2 line 102, h19 lines 150-175).
- `spec_preflight_gate.ps1` (`test_h6`, about 8 cases), `handoff_gate.ps1` (`test_h7`, about 11), `rerun_gate.ps1` (`test_h9`, about 12) are small suites for gates of 6-10 KB; the state-unreadable branches (memory says they warn, never ask) are not tested.
- The `SubagentStop` agent-type hook in `settings.json` (line 99) is a prompt, not a script, and cannot be unit tested; there is no regression check of any kind on it.

**Cross-cutting (medium):** no test of crash handling for the five gates with no outer try/catch (`bash_risky_gate`, `bash_windows_path_gate`, `shell_read_gate`, `fanout_track`, `fable_delegate_gate`), and none for the dispatcher behaviours in (b1) and (b2).

---

## (f) Not reached

- I did not read the bodies of h11, h12, h13, h22 (watchdog and stall injection) or h3, h6, h7, h9, h23, h24, h25, h31-h36, so I cannot say whether each one is clean on assertion strength; I judged them from grep of structure, counts, and the first 100-170 lines of h2, h5, h8, h10, h17, h19, h21, h26, h27.
- I did not check `git_ref_gate.sh` (14 KB) against the shim installed in `.git\hooks\` beyond confirming both exist and the shim says it refuses when the gate is missing, empty or crashing.
- I did not time a run or run any test (the preamble forbids it), so all speed statements are estimates from case counts and sleeps.
- I did not verify whether `no_recursive_delegation_gate` reads the right model (see e); I only reported the memory's claim and the gate's code.
- I did not check `cleanupPeriodDays` in settings, which decides whether the real transcript in (d4) can disappear.
- Duplication between test files other than the pasted helpers (for example overlap between h31/h33/h35 doc-review cases and h15 derive-once cases, which share a rules library) was not compared case by case.
