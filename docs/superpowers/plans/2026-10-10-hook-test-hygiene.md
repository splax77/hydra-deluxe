# Hook test hygiene: temp copies, one harness, a runner, and the untested hooks

Plan written 2026-10-10 by planning agent TP-3 (session c39f32f3). Source: the test-suite audit of 2026-10-10 (`docs/audit/2026-10-10-test-suite-audit.md`) and its hook scout report (D-hooks). Every claim below was checked against the files; where I could not check, I say so.

## What is wrong, and what this plan changes

The hook tests in `C:\Users\Patrick\.claude\hooks\tests` come in two generations. The newer tests (h15 onward, plus h27) copy the hooks they test into a temp folder and run them there, so nothing they do can touch the real `hooks\state` folder that every running agent depends on. The older tests (h1 to h13, h29, and parts of h2, h3, h21 and h30) run the live hook files and write their test state into the live folder. That has three costs: a hook edit can only be tested once it is already live, a test that dies halfway leaves files in the folder the fleet reads, and a test run during a real fleet shares state with it. Three of those tests (h8, h9, h10) also read a real 6 MB user transcript from 2026-09-05; if that file goes, they change behaviour with no code change.

Underneath that, the same four helper functions (`Invoke-Hook`, `Get-Decision`, `Get-Reason`, `Check`) are pasted into 20 to 33 files with small differences. Some read stderr into the output (`2>&1`), so any new warning line turns a decision into "unparseable"; others hide stderr (`2>$null`). There is no runner: the only way to run the suite is a one-line loop in a memory file, and each test prints its total in its own style, so there is no suite total. Six hooks have no behaviour test at all, three more are thin, and nothing checks that every gate file is actually wired into `settings.json` or a dispatcher. The `suite_run_gate` denies harmless searches that merely mention `test_h*` (I hit it myself during this planning, on a `Select-String` over the test folder). And the hooks folder carries 193 backup files and 6,565 state files that nothing prunes.

The plan: one shared harness file, one runner for the main session, hooks that find their own folder so a temp copy is fully self-contained, the old tests moved onto temp copies batch by batch, synthetic transcripts, a wiring test, tests for every untested hook (allow side and deny side), a narrower `suite_run_gate` rule 3, an archive folder for the backups, and a state-pruning script with a retention the user picks.

## When this lands, and what it does not touch

Another session is editing the dispatchers right now: `dispatch_bash_ps.ps1`, `dispatch_star.ps1` and `lib\verdict_merge.ps1` (the fail-open bug), the outer try/catch audit of the gates, and test_h21's `-ne 'ask'` assertions. This plan lands after that session's work is in. Nothing here edits those three files, adds or changes a try/catch in any gate, or touches test_h21's assertions. The wiring test in this plan reads the dispatchers' `$gates` lists, which is read-only and safe. Three gates this plan does edit (one-line path changes in `fanout_track`, `fable_delegate_gate`, `rerun_gate`) are also in that session's try/catch audit, so those edits wait until its files are final.

## How every step is kept safe

A broken hook blocks every agent on the machine, and hooks are read fresh on every tool call, so an edit is live the moment it is saved. Every task that edits a hook therefore works the same way: write the new version as a copy, run that hook's own test file against the copy in a temp folder, and only then move the copy over the live file. The harness makes this the default: its `New-HooksCopy` helper builds the temp folder, and a test takes a `-HookDir` parameter so the same test runs against a candidate copy or the live folder. The main session runs the whole suite once with the runner after each wave; subagents never do (the preamble forbids it and `suite_run_gate` enforces it, two test files per agent). `settings.json` is not edited by any task. Hook edits are done by Opus executors, because `fable_delegate_gate` stops the main session writing hook code.

There is no git repo under `~/.claude`, so the `.bak` files were the only history. Until the user decides on Q1, each executor keeps making one `.bak-<tag>` of a hook before changing it, and the archive step moves them all at the end.

## Tasks

Each task names the files it touches, the test that proves it, and whether it can run beside the others. Every executor runs at most two test files (the gate's rule). "Harness" means `tests\lib\harness.ps1` from task 1.

### Task 1: one harness file (first, alone)

Write `hooks\tests\lib\harness.ps1`, dot-sourced by tests. It holds: `Invoke-Hook` (feeds UTF-8 bytes with no byte-order mark on stdin, reads stdout and stderr apart, returns both), `Get-Decision`, `Get-Reason`, `Get-Context`, `Check`, `Check-Decision` (which also enforces that a deny's last line starts with `Instead: `, as h20 and h30 do today), `New-HooksCopy` (copies named hooks plus `lib\`, `transcript_lib.ps1` and `cmd_writes_lib.ps1` into a temp folder and returns the path), `New-Transcript` (writes a two-line transcript with a given model, no BOM), `Get-StateSnapshot` and `Assert-LiveStateUntouched` (name-list of the real `hooks\state` before and after a test; any new file is a failure), and `Write-Summary` (prints `N passed, M failed` and sets the exit code). The one place where the output format lives is this summary, so the runner can read every test the same way.

The test that proves it: a new `test_h39_harness.ps1` runs the harness against three stub scripts in a temp folder: one that denies, one that prints a decision on stdout and a warning on stderr, one that crashes before printing. It pins that the stderr line never makes the decision "unparseable" (the bug the `2>&1` tests have today), that a crash reads as `unparseable` and never as `none`, and that `Assert-LiveStateUntouched` catches a planted file. Nothing else can start until this lands, so it goes first, as one small agent.

### Task 2: a runner for the main session (parallel with task 1)

Write `hooks\run_hook_tests.ps1`. It runs every live `tests\test_h*.ps1` in sequence, each in its own `powershell.exe`, captures the exit code and the seconds, prints one table (file, result, seconds) and a total line, and exits 1 if any file failed. It also warns when a file printed a `FAIL` line but exited 0, as a backstop while the old tests still have their own summary styles. A `-Only h2,h8` switch runs named files, and a `-TestsDir` switch points it at another folder. It is for the main session only; a subagent running it is denied by `suite_run_gate` rule 3, which is right.

Proof: `test_h40_runner.ps1` points the runner at a temp folder holding two stub test files (one passes, one fails) and pins the table, the total and the exit code. The executor cannot run the real suite; the main session runs the runner once when this lands and reports the first real totals.

### Task 3: hooks find their own folder

Thirteen hooks name their own folder by the absolute path `C:\Users\Patrick\.claude\hooks`, for the state folder or to dot-source `transcript_lib.ps1`: `agent_progress_gate`, `executor_dispatch_brief_gate`, `fable_delegate_gate`, `fanout_track`, `fanout_mark_delegated`, `fanout_reset`, `rerun_gate`, `posture_inject`, `no_recursive_delegation_gate`, `workflow_monitor_reminder`, `stop_orchestrator_gate`, `status_append` and `deletion_shrink_gate`. A temp copy of any of these still reads the live library and writes the live state, which is why the old tests could not move. The change is one line per place: use `$PSScriptRoot`, keeping the absolute path as the fallback when `$PSScriptRoot` is empty, exactly as `suite_run_gate` already does. The two existing env-var overrides (`SHRINK_GUARD_STATE_DIR`, `DERIVE_ONCE_STATE_DIR`) stay as they are. Two strings that merely show the user the standard monitor command (in `monitor_batch_gate` and `workflow_monitor_reminder`) are not self-references and stay.

This task is split across the task 5 batches: each batch agent fixes the hooks its tests cover, then moves those tests, and proves both with the same two test files. The three gates in the other session's try/catch audit (`fanout_track`, `fable_delegate_gate`, `rerun_gate`) are done last, after that session lands. Proof across the whole set: the scan in task 6 lists which hooks still carry the absolute path and only ever shrinks.

### Task 4: the wiring and scan test (parallel, needs only task 1)

Write `test_h41_wiring.ps1`. It parses `settings.json` and both dispatchers' `$gates` arrays with the PowerShell parser (not regex), then checks four things: every registered or listed file exists; no registered path ends in `.bak-*` or `.retired-*`; every top-level hook `.ps1` is either registered, listed by a dispatcher, or on an explicit helper list in the test (`status_append`, `status_batch_monitor`, `agent_watchdog`, the submit and key helpers, `main_move_check`, `install_git_ref_gate`, the two shared libraries); and that helper list fails if it names a file that no longer exists, so a rename cannot turn it into a no-op. The same file carries the self-path scan from task 3: a known list of hooks still holding the absolute path, which must only shrink. Read-only over the dispatchers.

### Task 5: move the old tests to temp copies, in batches (parallel, after tasks 1 and 3 for its hooks)

Each batch is one agent with at most two test files. The pattern per file: dot-source the harness, build the temp copy with `New-HooksCopy`, point `$hook` and `$stateDir` at the copy, drop the pasted helpers, drop the hand-written cleanup of live state, and end with `Assert-LiveStateUntouched` and `Write-Summary`. Batches:

- h2 and h3 (progress gate; stop gate and monitor reminder). h2 also gets the D107 reviewer cases from task 7.
- h8 and h9 (fable gate and fanout counter; rerun gate), plus the synthetic transcripts of task 8. Waits for the other session.
- h10 and h4 (brief gate; workflow script gate). h10 also gets the extra brief-gate rules from task 7.
- h6 and h7 (preflight; handoff).
- h11 and h13 (stall_inject relay and context reminder).
- h12 and h29 (monitor gate; the state-file library). Check h29 first: it may already write only to temp, in which case only the pasted helpers go.
- h1 and h5 (stall watchdog; git attribution). h1 already builds a workspace; check before moving anything. h5's cases that default `cwd` to the real hydra-test checkout get a throwaway repo, as its other cases already have.
- h21 and h30: only their live-state lines move (h21's `fanout_<sid>.json` seed, h30's real-checkout path). h21's assertions are the other session's and are not touched.

Proof for every batch: the two migrated files pass against a temp copy, and the live-state snapshot shows no new file. The harness summary line replaces each file's own.

### Task 6: tests for the untested hooks (parallel, needs task 1; tasks 3 for the hooks it names)

One agent per line, each with one or two test files, allow side and deny side in every file:

- `test_h42_subagent_gates.ps1`: `no_recursive_delegation_gate` (main session passes; a subagent whose transcript ends in a Sonnet or Haiku record is denied naming the family; Opus and Fable pass; no model and a missing transcript pass) and `no_background_in_subagent_gate` (main session passes; subagent Monitor denied; subagent Agent with `run_in_background` true denied; false or absent passes; other tools pass). Both deny texts today have no `Instead:` line, so the harness check fails them; the executor adds the line. Q5 asks what the first gate is meant to read.
- `test_h43_fanout_counter.ps1`: the trio around the fan-out counter, in a temp copy. `fanout_reset` writes the empty state; `fanout_mark_delegated` with a Fable parent dispatching Sonnet sets `delegated`, with Opus as the target leaves it, from a Sonnet parent does nothing; then `fanout_track` reads it: after a mark the fifth file gives no nag, after a reset it does. No try/catch edits.
- `test_h44_posture_inject.ps1` and `test_h45_transcript_lib.ps1`: posture_inject with a temp cwd holding a `.claude\model-postures.md` with all four family blocks (the Fable transcript yields the FABLE block as additionalContext, an unknown model gives `{}`, a missing marker gives `{}`); and the library's `Get-AssistantModel` on fixtures: one assistant record, newest-assistant-wins when a user record is last, a file past 256 KB whose answer sits in the tail, a partial first line, a missing file, and a file held open by a writer.
- The watchdog's ffmpeg kill, in `test_h22`: give `Stop-StaleFfmpeg` and the script a `-StaleName` parameter that defaults to `ffmpeg`. The test copies `powershell.exe` to a temp name, starts it sleeping, runs the watchdog `-Once -FfmpegMin 0 -StaleName <that name>` (h22 already runs it `-Once`), and pins that the process is gone and the log names it; a fresh one with the usual limit survives. No real ffmpeg is ever touched. Q4 asks whether the kill itself should narrow.

### Task 7: the thin tests get their missing cases (inside the task 5 batches)

In h2: seed `role = 'reviewer'` explicitly and pin the D107 sentence on the call-budget deny, the time-cap deny, the 100-call warning and the 20-minute warning; pin that an implementer gets none of them. Today those cases seed an empty role and the label says "reviewer" while the fixture decides. In h10: one case per deny branch of `executor_dispatch_brief_gate` (the Plan task table, the missing model, and the branches at its lines 62, 70, 78 and 87, which the executor reads and names) with its allow twin each; today only rule 0 is covered.

### Task 8: synthetic transcripts for h8, h9, h10 (inside their task 5 batches)

Replace the real transcript path with `New-Transcript` from the harness, carrying the model each test's cases assume (the real file's newest record is Fable; the executor confirms each case's expectation from the gate it drives, not from the old file). The wiring test gains one line: no test under `tests\` names `.claude\projects`.

### Task 9: suite_run_gate rule 3 (parallel, needs task 1)

Rule 3 today denies any command that contains `test_h*` together with any of `powershell`, `pwsh`, `-File`, `Invoke-Expression`, `iex`, `Start-Process`, `ForEach-Object`, `foreach` or `% {`. The loop words are the problem: `Select-String -Path ...\test_h*.ps1 | ForEach-Object Line` runs nothing. The fix: drop `ForEach-Object`, `foreach` and `% {` from the list and add the call operators `& ` and `. ` when followed by a variable or a quote. Then `Get-ChildItem test_h*.ps1 | % { & $_.FullName }` and `foreach ($f in ...) { . $f }` are still denied through `& $` and `. $`, while listing, searching, measuring and `Get-Content` over the folder pass. The rule stays a text match; the known walk-arounds (`test_h[0-9]*`) are out of this plan's scope and noted as such. Red-then-green in `test_h20`: the three search commands above as new allow cases beside the existing "listing is not a run" case, and the two loop-with-call forms as deny cases. The executor keeps the exact red line for the reviewer. The same edit deletes h20's check that `dispatch_bash_ps.ps1.bak-p0-3a` exists, which tests a leftover file.

### Task 10: archive the backups (main session, one command, after Q1)

Move every `*.bak-*` and `*.retired-*` under `hooks\`, `hooks\lib` and `hooks\tests` into `hooks\_archive\` keeping the file names. Checked: `settings.json` names only live files, both dispatchers load gates by explicit name, and the scans in h19 filter on `*.ps1`, so nothing loads a backup today; the only reference is the h20 check that task 9 deletes. The wiring test then adds: no `.bak-*` or `.retired-*` file sits outside `_archive\`.

### Task 11: prune the state folder (parallel, needs task 1 and Q2, Q3)

Write `hooks\prune_state.ps1 -Days N [-WhatIf]`. It deletes files under `hooks\state` whose last write is older than N days, by kind: `agent_*.json`, `status\*.md`, `fanout_*`, `reruns_*`, `stalls_*`, `stalls_seen_*`, `status_seen_*`, `watchdog_*`, `suite_runs_*`, `wf_launch_*`, `stopgate_*`, `delegate_turn_*`, `handoff_*`, `ctx_seen_*`, `emptyfleet_*` and `shrink_guard\*`. The `derive_once\` and `doc_review\` record folders are excluded unless Q3 says otherwise. `watchdog_start.ps1` calls it once a day (a stamp file `state\prune_last.txt` makes repeats free), so it runs whenever a fleet starts; no settings edit. The watchdog loop itself is not edited, so no pid needs killing. First real run is by hand with `-WhatIf`, output shown to the user.

Today's numbers, from a count on 2026-10-10: 6,565 files, 39 MB; 5,710 older than one day, 1,589 older than 7 days, 759 older than 14 days, 256 older than 30 days; the oldest from 2026-08-15.

Proof: `test_h46_prune_state.ps1` builds a temp state tree, ages files by setting their last-write time, and pins that `-WhatIf` deletes nothing and lists the right ones, a real run deletes exactly the old ones, excluded folders are untouched, a file one minute inside the limit survives, and the stamp stops a second run the same day.

## Pester, honestly

Pester 3.4.0 is what Windows ships; Pester 5 or 6 would be a gallery install. Its draws are `Mock`, tags, and JUnit output. `Mock` does not reach into a child `powershell.exe`, and that is where every hook runs in these tests (and in Claude Code), so the main thing Pester adds is of no use here. A migration would rewrite 37 files and roughly 2,000 cases into `Describe`/`It`, for a runner and a summary format that task 1 and task 2 give in about 300 lines. Recommendation: the shared harness, no Pester. If JUnit output is ever wanted for CI, the runner can emit it in a later step.

## User decisions

**Q1. The 193 backup and 2 retired files.** Options: (A) move them into `hooks\_archive\`, nothing reads them, 2.7 MB stays on disk; (B) delete them; (C) put `~/.claude/hooks` under git and delete them, so hook edits get real history. Recommendation: A now. C is the better long-term answer, but it changes how every executor edits hooks and deserves its own decision.

**Q2. Retention for the state folder.** Options: 7 days (1,589 files go today), 14 days (759), 30 days (256). Recommendation: 14 days. The oldest agent any gate still reads is this session's; a two-week window is far past any resumed work.

**Q3. Do the derive-once and doc-review records get pruned?** They are review evidence the merge gate consults for a range; an old one is dead once its range is merged, but I have not traced every reader. Options: exclude them (recommended), or include them at 30 days.

**Q4. The watchdog kills any `ffmpeg` older than 15 minutes, whoever started it.** The fcvideo workflow runs ffmpeg by hand. Options: keep the rule and only test it (recommended, cheapest; the test uses a stand-in name and never touches real ffmpeg), or narrow the kill to ffmpeg whose parent is an agent's shell, which needs a parent-process walk and its own cases. If you run ffmpeg by hand while a fleet is up, pick the narrow one.

**Q5. What `no_recursive_delegation_gate` is meant to read.** It takes the model from `transcript_path`, and the memory note (unverified by me, verified live by the author on 2026-09-04) says that path is the parent session's transcript for a subagent call. So the gate denies when the parent is Sonnet, not when the subagent is. If the intent is "a Sonnet subagent may not dispatch", the gate needs a different source for the model and the test pins that instead. Please say which.

**Q6. Pester.** Recommendation above: no. Say so if you want it anyway, and the harness becomes a Pester shim instead.

## Risks

A hook edit that breaks blocks every agent: every edit goes through a temp copy and that hook's test first, the absolute-path fallback stays in every self-reference, test_h19's fail-closed scan runs as a canary, and the main session runs the full suite once after each wave. The dispatchers and settings.json are not edited. The archive move is safe because nothing loads a backup (checked above); the one test that did is fixed first. The prune never runs before a `-WhatIf` the user has seen, excludes the review records, and cannot touch a live session's files, whose last-write times are fresh by definition. The ffmpeg test uses a renamed copy of `powershell.exe`, never the `ffmpeg` name. Task 9 widens what `suite_run_gate` allows, so its red-then-green cases pin that every loop-and-run form stays denied.

## Size and order

Task 1 is about 200 lines plus a 150-line self-test; task 2 about 120 plus a test; task 3 is thirteen one-line edits; task 4 about 150 lines; task 5 touches sixteen test files, mostly their first 30 lines and their cleanup; task 6 is five new test files of 100 to 200 lines plus one parameter in the watchdog; task 9 is a ten-line hook change and ten cases; task 11 is an 80-line script and a 100-line test. Roughly 25 to 30 small executor runs in three waves.

Wave 1 (as soon as the dispatcher session lands): task 1 alone, with task 2 and task 10 beside it. Wave 2: tasks 4, 9, 11, and the task 6 lines whose hooks have no absolute self-path (`no_background`, `transcript_lib`, the watchdog). Wave 3: the task 5 batches, each carrying its task 3 edits and its task 7 and 8 cases, and the rest of task 6. One fresh Sonnet reviewer per change, then merge, per D107.

## What done looks like

The main session runs `run_hook_tests.ps1` and gets one table and one total. No test file defines `Invoke-Hook`, `Get-Decision` or `Check` of its own. No test names `hooks\state` or `.claude\projects` directly, and `Assert-LiveStateUntouched` passes in every file. No hook names its own folder by an absolute path outside the fallback line. Every hook file is either wired or on the helper list, and the wiring test fails when either side changes. The six untested hooks and the three thin ones have allow and deny cases. `Select-String` over the test folder is not denied. The backups sit in `_archive\`. The state folder holds only the last N days of files, and the next fleet start keeps it that way.

## Stale notes for the main session

`fleet-control-hooks-2026-09.md` says H5 needs trailers in `git_attribution_gate` (the gate leaves trailers to the git-side hook since 2026-10-03), that `test_h14_listing_gate` exists with 18 checks (retired), that the suite is 260 cases in 14 files (37 files now), and that a hook never asks (h8 correctly expects `ask` for the main session on a media write outside the scratchpad). The memory is the main session's to fix; no task here touches it.

## Open questions I could not settle

I did not read the bodies of h1, h5, h29 and h30 past their headers, so whether h1 and h29 already write only to temp is a check inside their batch, not a fact. I did not read `executor_dispatch_brief_gate` lines 30 to 85, so task 7 names its deny branches by line and leaves the case wording to the executor. I did not run anything, so the runner's first real totals come from the main session.
