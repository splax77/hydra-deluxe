# Hook overhaul plan, 2026-10-10

Status: approved by the user on 2026-10-10 ("accept all"), with every recommendation below taken as the answer. Not started: the "Hook test hygiene" session (`2026-10-10-hook-test-hygiene.md`) is already building on the same hook files and test numbers, so the two plans must be reconciled before wave 0.

This plan carries out the hook audit (`docs/audit/2026-10-10-hook-audit.md`) under the user's six decisions of 2026-10-10. The main session stays on Opus. Four things are retired: the shell-read gate, the Stop gate, the fanout trio and the rerun gate. The status-line rule counts minutes only. Review gates skip text of 20 lines or fewer. Four Fable planners wrote the detailed sections, which sit beside this file in `2026-10-10-hook-overhaul/`. This page is the merged plan: what gets built, in what order, who owns which file, and what the user still has to answer. Each task's full detail (why, exact lines, test, size) is in the section named in its ID: F tasks are in p1, T tasks in p2, R tasks in p3 and C tasks in p4.

## What the overhaul does, in plain words

It starts by putting `~\.claude` under git. Today a saved hook file is live in every session at once, and the only history is 195 hand-made `.bak` files. After this, hook work happens in a git worktree, a fast-forward merge makes it live, and `git revert` undoes it in under a second.

Then it fixes the four broken things. The dispatchers will deny when a gate is missing, crashes or prints nothing. The watchdog process goes; a once-a-minute stall check inside `stall_inject` replaces it, and the machine-wide ffmpeg kill is dropped. The progress gate will read each agent's own first prompt, so reviewers finally get their D107 wording. Its status rule will count minutes only and will never block a hand-back.

It retires what the user chose, plus the two Fable-only gates. The media guard and the `sed -i` deny, the only rules in those files worth keeping, move into the risky gate first, so they never go dark.

It cuts the noise. Words inside quotes and here-docs stop counting as commands. The suite gate moves onto the shared scanner. Scratch helper files stop counting as patch scripts. Short review texts pass without a reviewer.

It consolidates. Each shell command is parsed once, not up to eight times. Agent, Workflow and Monitor share one dispatcher, and so do the per-prompt hooks. The 594-line attribution gate becomes three gates, one per job. Derive-once and doc review share one records library, one waiver hook and one submit gate. About 330 lines of dead doc-review code go.

Last, the docs catch up. CLAUDE.md, the preamble, the two review briefs and the per-prompt posture get corrected wording, which the user approves before anyone edits them.

## How the work runs

The procedure is in p1, "How the work runs", and becomes `~\.claude\hooks\README.md` (task F3). The short version follows. The main session makes a branch and a worktree under `~\.claude\worktrees\<task>`. One Opus executor builds the task there and runs only its own tests from the worktree copy. One fresh Sonnet reviewer fixes what it finds, commits and signs off (D107). The main session merges `main` into the branch, runs the full hook test suite once, then fast-forwards `main`, which makes the change live.

Three files are owned by the main session alone, never by a task. The first is `settings.json`. Registrations only take effect in new sessions anyway, so each task's report states its registration change in words and the main session applies it at merge. The second is the gate list in `dispatch_bash_ps.ps1`, for the same reason. The third is `tests\test_h19_utf8_input.ps1` until task F10 makes it derive its list on its own.

Retiring a hook takes two commits (p1, "One fact that shapes the design"). The first removes the registration and replaces the script with a one-line stub that prints `{}`. The second, after every running session has restarted, deletes the stub. Running sessions keep their old registrations, so deleting the file at once would make every matching call report a hook error.

## Order of work

Each wave starts when the tasks it depends on have merged. Tasks in one wave run in parallel; no two of them own the same file.

**Wave 0, foundation.** F1 puts `~\.claude` under git and installs the git-side gate there (if the user says yes to question A2). It also checks the claim that registrations load once per session. F5 deletes the stale project copies and the dead plan hooks in `settings.local.json`. It touches only hydra-test's ignored `.claude` folder, so it runs beside F1.

Progress on 2026-10-10: F5 is done. F1 is done too: `~\.claude` is a git repo, the first commit `9bdb26c` holds the live config (104 files), the gate shims are installed, and a trailer-less commit was refused. Two parts of F1 are still open. The first is checking that registrations load once per session; it moves into T5, which needs it first. The second is new, F1b. The derive-once gate's list of code paths (`Test-DeriveOnceGatedPath` in `lib\derive_once_rules.ps1`) is hard-coded for Hydra's `src/`, `tests/` and so on. So in `~\.claude` the gate checks trailers but doesn't yet require a reviewer before a hook change merges, which leaves answer A2 only half in force. F1b reads the gated folders from the repo's `.derive-once-gate` marker file, so `~\.claude` can list `hooks/`. It owns `lib\derive_once_rules.ps1` and test h15, so it runs before R4 and after nothing else. Until it lands, the one-reviewer-per-change rule is kept by hand. Rollback note: if `git_ref_gate.sh` itself breaks, every ref update from Claude Code is refused, so the rollback for that one file is `git checkout HEAD~1 -- hooks/git_ref_gate.sh`, a file checkout that moves no ref.

**Wave 1, make worktrees testable.** F2 makes the ten surviving tests and one hook find their files relative to themselves, so a test run from a worktree tests the worktree copy. F10 (new, merged from p4's overlap note) makes `test_h19` read its gate list from `settings.json` and the dispatchers' lists, so no later task has to edit it by hand. F3 writes the hooks README. Every hook task after this depends on F2.

**Wave 2, fixes, moves and first cleanups, all in parallel.**
- T1: dispatchers fail closed.
- T2: progress gate (reviewer role, minutes only, hand-back passes).
- T3: stall check inside `stall_inject` replaces the watchdog.
- T4: media guard and `sed -i` move into the risky gate, built on the shared scanner. T4 also moves `Expand-HookPath` and `Test-NullSink` into `lib\scratch_path.ps1`, which makes p4's task 5 unnecessary.
- F4: zip and delete the `.bak` files.
- F6: state pruning.
- F7: the new posture text.
- R-doc: p3's tasks 2, 1 and 3, in that order, given to one executor because they share six files. The dead code goes first, then the 20-line rule, then the gh gate's fail-open catch.
- R6b: the suite gate onto the scanner.
- R6c: the patch-script rule's scratch check.
- C6: the agent definitions and the spawn-depth setting.

**Wave 3.**
- T5: retirements, stubs first. It waits for T1, T3 and T4.
- R6a: quoted text blanked in the scanner. It waits for T4, because both edit `bash_risky_gate.ps1`.
- R4: one records library and one waiver hook. It waits for R-doc.

**Wave 4.**
- C1: parse once and a shared gate loop. It waits for T5, R6a and R6b.
- C7: `stall_inject` caching. It waits for T3.
- R5: one submit gate. It waits for R4.

**Wave 5.**
- C2: dispatcher for Agent, Workflow and Monitor. It waits for C1.
- C3: dispatcher for prompts. It waits for C1 and R4.
- C4: split the attribution gate. It waits for C1 and R6c.

**Wave 6, docs and final cleanup.** F9 corrects CLAUDE.md, the preamble, `doc-review.md` and `derive-once-review.md`, with F8's two sentences folded in. It waits for T2, R-doc and C6, so the docs are never ahead of the hooks. Then the stub-deletion commit for every retired hook goes in, once the user has restarted their sessions.

## Absorbed from the hook-test-hygiene plan

The user decided on 2026-10-10 that this plan leads all hook work. The "Hook test hygiene" session had already built three things, which stay: the shared test harness (`tests\lib\harness.ps1`, test h39), the runner (`run_hook_tests.ps1`, test h40) and the `.bak` archive (`hooks\_archive\`, 133 files). The main session uses the runner for its one full run at each merge. Its remaining tasks map onto this plan as follows.

Its task 3, hooks finding their own folder, merges into F2. F2 covers every surviving hook that names its folder by absolute path, not only the two P1 listed, and keeps the absolute path as a fallback as the hygiene plan specified. Its task 4, the wiring test, is F10, built to its design: it parses `settings.json` and the dispatchers' gate lists, checks every listed file exists, and keeps a helper list that fails when it names a missing file. Its task 9 is R6b. Its task 10 is done; F4 now deletes `_archive\` once F1 has the history. Its task 11 is F6, with its `-WhatIf` first run shown to the user and the 7-day age from answer A3.

Two of its tasks become new tasks here. H5 moves the old tests (h1 to h13, h29 and parts of h2, h3, h21 and h30) onto temp copies with the harness, in batches after F2 and F10. It skips every test T5 deletes. H6 writes allow and deny tests for the surviving hooks that have none: `no_background_in_subagent_gate`, `transcript_lib` and `workflow_monitor_reminder`. Each batch runs beside wave 2 and owns only the test files it moves, never a file a wave-2 task owns at the same time.

The two plans' approved answers disagreed in three places, and the user settled them on 2026-10-10 in this plan's favour. State files are pruned after 7 days, not 14. No subagent may dispatch agents (the spawn-depth setting), rather than only Sonnet and Haiku ones. The watchdog's ffmpeg kill is dropped, not kept and tested.

Two findings arrived after the planners finished, and they change two tasks. First, the settings schema of the installed Claude Code (2.1.275) already has `onFailure: "block"` on command hooks. The docs scout had dated it to 2.1.295, and the schema shows it's here now. So T1 also asks the main session to add `onFailure: "block"` to the dispatcher registrations, which closes the "dispatcher itself crashed" hole natively. The fail-closed gate loop is still needed, because `onFailure` judges only the dispatcher's own exit, not the gates inside it. The schema says it is ignored on Stop and SubagentStop hooks. Second, the hygiene session found that `suite_run_gate` doesn't stop a subagent from running `run_hook_tests.ps1`: rule 3 needs `test_h*` in the command text, and rule 4 counts only named files. R6b adds the runner to the gate.

Its tasks 7 and 8 shrink. The synthetic transcripts for h8, h9 and h10 aren't needed, because T5 deletes those tests. Its questions Q4 (the ffmpeg kill) and Q5 (`no_recursive_delegation_gate`) are settled by T3 and C6.

## Who owns what

Test numbers h41 to h49 are assigned here. The hygiene session already used h39 (the harness) and h40 (the runner).

| Task | Owns | New test |
|---|---|---|
| F1 | `~\.claude\.gitignore`, `.git`, git-side shims | none |
| F2 | tests h2, h4, h6, h11, h12, h13, h20, h21, h29, h30; `no_recursive_delegation_gate.ps1` (skipped if C6 retires it) | none |
| F10 | `tests\test_h19_utf8_input.ps1` | h50 (wiring) |
| F1b | `lib\derive_once_rules.ps1`, test h15 | none |
| F3 | `hooks\README.md` | none |
| F4 | the `.bak`/`.retired` files, the zip | none (edits h20 line 220 after F2) |
| F5 | `hydra-test\.claude\hooks\**`, project `model-postures.md`, `settings.local.json` | none |
| F6 | `hooks\state_prune.ps1` | h43 |
| F7 | `model-postures.md`, `posture_inject.ps1` | h44 |
| T1 | `dispatch_bash_ps.ps1`, `dispatch_star.ps1`, `lib\verdict_merge.ps1` | h41 |
| T2 | `agent_progress_gate.ps1`, tests h2 | none |
| T3 | `stall_inject.ps1`, tests h1, h22 | none |
| T4 | `bash_risky_gate.ps1`, `lib\scratch_path.ps1` | h42 |
| T5 | the retired scripts (stubbed), `cmd_writes_lib.ps1`, tests h3, h8, h9, h10, h21, h23, h27, h28, `replay_shell_corpus.ps1` | none |
| R-doc | `lib\doc_review_rules.ps1`, the doc-review tool, gh, submit and key hooks, tests h31 to h35 | none |
| R6a | `lib\shell_scan.ps1`, `bash_risky_gate.ps1`, tests h16, h24 | none |
| R6b | `suite_run_gate.ps1`, tests h20 | none |
| R6c | rule-3 block of `git_attribution_gate.ps1`, test h5 or a new h49 | h49 if needed |
| R4 | `lib\review_records.ps1` (new), both review libraries, `review_waiver.ps1` (new), both old waiver hooks, tests h15, h31, h36, h37 | none |
| R5 | `review_submit_gate.ps1` (new), both old submit gates, tests h15, h32, h37 | none |
| C1 | both dispatchers, `lib\hook_input.ps1`, `lib\dispatch_lib.ps1` (new), the nine shell gates' input lines, test h27 | h45 |
| C2 | `dispatch_launch.ps1` (new) | h46 |
| C3 | `dispatch_prompt.ps1` (new), `lib\verdict_merge.ps1` (new function) | h47 |
| C4 | `git_attribution_gate.ps1` and its three successors, test h5 | none |
| C6 | `~\.claude\agents\*.md` (four files), `no_recursive_delegation_gate.ps1` (retired) | h48 |
| C7 | `stall_inject.ps1`, test h11 | none |
| F9 | `CLAUDE.md`, `docs/agents/brief-preamble.md`, `doc-review.md`, `derive-once-review.md` | none |

Some files have more than one owner over the life of the plan, but never two at once:
- `bash_risky_gate.ps1`: T4, then R6a, then C1.
- `dispatch_bash_ps.ps1`: T1, then C1.
- `stall_inject.ps1`: T3, then C7.
- `lib\verdict_merge.ps1`: T1, then C3.
- test h5: R6c, then C4.
- test h20: F2, then F4, then R6b.
- tests h15 and h37: R4, then R5.

The waves above already respect these orders.

## What I changed while merging

The planners overlapped in four places, and the merge settles each one.

- **The media guard.** p2 builds it on the scanner in T4, and p4 planned a separate rewrite (its task 5). T4 does it once, and p4's task 5 is dropped.
- **`test_h8`.** p2 deletes it while p3 listed it as its own. It goes in T5, and R6a doesn't need it.
- **`settings.json`.** p2 had T5 owning it. Under p1's rule, the main session owns it.
- **Test numbers.** These are now unique, h41 to h49, after the hygiene session's h39 and h40.

## Questions for the user

Each question has a recommendation. Reply "accept all", or name the numbers you want changed. The planners' sections quote the code's actual rule for each one, with an example from both sides.

**Foundation (p1)**
- A1. Should `skills\` go in the `~\.claude` repo along with `hooks\`, `settings.json`, `CLAUDE.md` and `model-postures.md`? Recommend yes.
- A2. Should the derive-once git gate guard `~\.claude` too? That makes a hook change wait for its reviewer, and an emergency revert needs `waive derive-once <key>` first. Recommend yes.
- A3. Prune state files older than 7 days, leaving review records alone? Recommend 7.
- A4. Move the bullet-check Stop hook from hydra-test's `settings.local.json` into your user settings, so it runs in every project and lives in git? Recommend yes.
- A5. Approve the new posture text in p1, task F7. It's about 2,200 characters, sent with every prompt.

**Fixes (p2)**
- B1. Keep the `sed -i` deny after the shell-read gate goes? Recommend yes.
- B2. Status rule: nag at 5 silent minutes, deny at 10. Recommend keeping today's minute values.
- B3. At 10 silent minutes, only the hand-back passes, not `git commit`. Recommend this.
- B4. Stall threshold of 10 minutes, scanned at most once a minute. These are the watchdog's own numbers. Recommend keeping them.
- B5. Should the progress gate deny when it crashes itself, instead of letting the call through uncounted? Recommend yes, to match your fail-closed rule.
- B6. Keep the empty-fleet notice? It fired 6 times, every one real. Recommend keeping it.

**Review gates and noise (p3)**
- C1. Count the 20 lines as non-blank lines. Recommend non-blank.
- C2. Apply the 20-line rule to commits on the four public pages too? Recommend yes; one rule for all three gates.
- C3. Drop the `DROP TABLE` and `TRUNCATE` patterns? Recommend yes.
- C4. Blank quoted text for the PowerShell tool too? The planner left this open, because you kept PowerShell out of the scanner on 2026-10-05. I recommend yes. Most of the false denials are PowerShell `status_append` calls with the trigger words inside the quoted status text, and a real `Remove-Item -Recurse -Force C:\src` has its words unquoted and stays denied.
- C5. Should the suite gate count a loop over literal filter strings as filtered? Recommend yes, when every literal is a valid filter.

**Consolidation (p4)**
- D1. Replace `no_recursive_delegation_gate` with `CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH=1`? That's stricter: an Opus executor can no longer dispatch helpers either, which the preamble already forbids. Recommend yes.
- D2. Should the posture require every dispatch to name one of the four agent types (planner, executor, reviewer, scout)? Recommend yes; otherwise the agent files bind nothing.
- D3. Add `maxTurns: 200` as a safety net under the 150-call gate? Recommend no; the gate already stops agents, and turns don't map to calls.
- D4. Is a target of 0.5 s per shell call (down from 1.0 s) enough, with no resident hook server? Recommend yes.
- D5. Fold Monitor into the launch dispatcher? Recommend yes.

## Numbers and where they come from

Hook counts, line counts, timings and denial counts are from the audit and its scouts. The audit names each source. Line numbers in the sections are from the live hook files read on 2026-10-10. The installed Claude Code version (2.1.275, CLI and desktop app) is from p4's check of `claude --version` and `~\.local\share\claude\versions`. The `onFailure` and spawn-depth minimum versions (2.1.295 and 2.1.217) are from Anthropic's hooks and env-vars pages, cited in p4.
