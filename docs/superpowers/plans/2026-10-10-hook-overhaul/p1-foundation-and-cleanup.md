# P1: Foundation and cleanup

Planner P1, agent a0c925a5491acf650, 2026-10-10. Nothing here is built. Every task waits for the user's yes, and the doc and posture wording in sections F7 and F9 goes to the user before anyone edits a file.

## The short answer

Today the hooks are live the moment a file is saved, and nothing records what they looked like a minute ago. Several Claude sessions share them. So an executor that edits `C:\Users\Patrick\.claude\hooks\x.ps1` is editing every running session at once, and a typo denies every tool call on the machine until someone notices.

The fix is to make `C:\Users\Patrick\.claude` a git repo and do all hook work in git worktrees. A worktree is a second checkout of the same repo in another folder. The live folder stays on `main` and keeps serving every session. Executors edit and test their worktree copy. Making a change live is one fast-forward merge on `main`, which rewrites only the files that changed. Rolling back is `git revert`, one command, under a second.

I checked whether a worktree copy can be tested in place. The two dispatchers find their gates with `$PSScriptRoot` (the folder the running script sits in), and every library is dot-sourced the same way, so a worktree copy of a dispatcher runs the worktree's gates and libraries, not the live ones (`dispatch_bash_ps.ps1:45`, `dispatch_star.ps1:36`, `lib\*.ps1` headers). The exceptions are seven hooks that dot-source `transcript_lib.ps1` or `cmd_writes_lib.ps1` by the absolute live path, and sixteen tests that hard-code `C:\Users\Patrick\.claude\hooks`. Those tests would test the live hook even when run from a worktree, so a failing-first test could never go red. Most of the seven hooks and six of the sixteen tests belong to hooks P2 retires. The survivors are a small task (F2) that has to land before any other hook task starts.

State files (`hooks\state\...`) are a different matter. They are runtime data shared by every session, and the hooks name them by absolute path (`agent_progress_gate.ps1:26-27`, `status_append.ps1:16`). A worktree copy writes to the same state as the live copy. That is what we want: a hook under test should see the real counters. Tests already use fake session and agent ids so they do not collide.

## One fact that shapes the design

Claude Code reads the hook scripts fresh on every call, but it reads the registrations (the `hooks` key in `settings.json`) once, when a session starts. A change to `settings.json` does nothing in a running session until the user reviews it in `/hooks` or starts a new session. Several secondary write-ups agree on this ([gend.co](https://www.gend.co/blog/configure-claude-code-hooks-automation), [mintlify guide](https://www.mintlify.com/VineeTagarwaL-code/claude-code/guides/hooks), [smartscope](https://smartscope.blog/en/generative-ai/claude/claude-code-hooks-revolution-2025/)); I did not find the official page in the search, so treat it as likely rather than proven, and F1's executor confirms it with one test (below).

Two rules follow. First, a script fix is live at once for every session, so a bad merge hurts fast and a revert heals fast. Second, a retired hook's script must not be deleted in the same commit that removes its registration: running sessions still hold the old registration and would call a missing file on every matching tool call, which Claude Code reports as a non-blocking error (W1.md, output contract). So a retirement is two commits. The first removes the registration and replaces the script body with one line that prints `{}` and exits 0. The second, in the next cleanup after every session has restarted, deletes the stub. P2 owns the retirements and should follow this.

Also: every task below that touches `settings.json` does not own it. `settings.json` is one file that P1, P2, P3 and P4 would all edit, and two tasks never own one file at the same time. So the main session owns `settings.json`. Each task's report states the exact registration change it needs, in words, and the main session applies it when it merges. Nothing is lost by the delay, because registrations only take effect in new sessions anyway.

## How the work runs once the foundation exists

This is the procedure F3 writes down. I say it out loud here so the user can approve the procedure, not just the file.

The main session makes a branch and a worktree for a task: `git -C C:\Users\Patrick\.claude worktree add C:\Users\Patrick\.claude\worktrees\<task> -b <task>`. The worktree lives under `~\.claude\worktrees\`, which the `.gitignore` leaves untracked. The brief names that folder as the executor's checkout.

The executor edits only files inside its worktree and runs only its own test, from the worktree: `powershell -NoProfile -ExecutionPolicy Bypass -File C:\Users\Patrick\.claude\worktrees\<task>\hooks\tests\test_hNN.ps1`. Because the test and the hook find each other through `$PSScriptRoot`, this exercises the worktree copy and leaves the live hooks alone. The executor commits in the worktree with the usual trailers. The live sessions never see any of it.

The reviewer works in the same worktree, fixes its findings, commits and signs off (D107).

The main session then updates the branch with `git merge main` inside the worktree, runs the task's tests there once more, and makes it live with `git -C C:\Users\Patrick\.claude merge --ff-only <task>`. Fast-forward only, so there is never a conflict marker in a live hook. Git rewrites each changed file in place; a hook process that starts during that write could read a half-written file. The window is milliseconds per file and the next call reads the finished file, so I propose accepting it rather than building an atomic swap.

Rollback: `git -C C:\Users\Patrick\.claude revert --no-edit HEAD` puts the previous scripts back on the next tool call. For one file, `git -C C:\Users\Patrick\.claude checkout HEAD~1 -- hooks/x.ps1` and a follow-up commit. Either way the fix is live before anyone has to think.

The one thing a worktree cannot do is register itself. `settings.json` names the live folder. For a change to a dispatcher, where the question is "does Claude Code accept what this prints", the executor can run `claude -p "Reply ok" --model haiku --settings <scratch settings file>` with a settings file whose hook commands point at the worktree, in a scratch folder, and read the transcript. The user has run this shape before (`hydra-test\.claude\settings.local.json` line 32 keeps such a command in its allow list). Not verified: whether `--settings` adds to or replaces the user hooks. If it adds, both copies run, and the test still shows whether the worktree copy's verdict arrives. P2 and P4 decide whether they need this; the hook tests cover everything else.

## Tasks

### F1. Put `~\.claude` under git

Why: the 195 `.bak` and `.retired` files (scout H, directory listing) are hand-made version control with no record of which backup matches which change. Git gives real history, diffs, worktrees and a one-command rollback.

What changes. The executor runs `git init` in `C:\Users\Patrick\.claude`, sets `core.autocrlf=false` in that repo so git never rewrites a live hook's line endings on checkout, and writes a whitelist `.gitignore`: ignore everything (`*`), then un-ignore `hooks/` (but not `hooks/state/` and not any `*.bak-*` or `*.retired-*` name), `settings.json`, `CLAUDE.md`, `model-postures.md` and `skills/`. Everything else stays out: `projects\` (2,571 MB), `plugins\` (80 MB), `file-history\` (51 MB), `backups\`, `cache\`, `shell-snapshots\`, `sessions\`, `session-env\`, `telemetry\`, `worktrees\`, and the top-level `.credentials.json`, `.claude.json`, `history.jsonl`, `mcp-needs-auth-cache.json` and `.last-*` files. Sizes are from my `Get-ChildItem` pass over `~\.claude` today. `.credentials.json` holds tokens; the whitelist shape exists so a new file never lands in git by accident. The executor checks `git status --ignored` shows it ignored before the first commit.

The first commit is the live state, exactly as it is, so `main` equals what every session is running. Then the executor runs `install_git_ref_gate.ps1 -Repo C:\Users\Patrick\.claude` so the derive-once gate guards `main` here as it does in hydra-test. The installer takes any repo path and refuses nothing about this one (it only refuses a `core.hooksPath` or a foreign shim, `install_git_ref_gate.ps1:64-81`). The gate asks `origin` only to exempt commits origin already has (`git_ref_gate.sh:220-227`); with no origin there is simply no exemption. Then it confirms the snapshot behaviour: start a `claude -p` session with a scratch settings file, edit the file's hooks while it runs, and check the edit does nothing until a new session. That one observation decides whether the two-commit retirement rule above is needed.

Files owned: `C:\Users\Patrick\.claude\.gitignore` (new), the new `.git` folder, and the two shims it installs in `C:\Users\Patrick\.claude\.git\hooks\`. No hook script changes.

Failing-first test: there is no behaviour to go red here. "Done" is: `git -C C:\Users\Patrick\.claude status --porcelain` prints nothing; `git ls-files` lists no `.bak`, no `state\`, no `.credentials.json`; `git worktree add` into `worktrees\probe` works and `git worktree remove` cleans it; the git gate refuses a trailer-less commit on `main` from inside Claude Code, as it does in hydra-test.

Depends on: nothing. Everything else in the overhaul depends on it.

Size: small, about 25 tool calls. The main session could do this itself in fewer.

### F2. Tests and hooks find their files relative to themselves

Why: a test that hard-codes `C:\Users\Patrick\.claude\hooks\agent_progress_gate.ps1` tests the live hook even when it runs from a worktree (`test_h2_progress.ps1:5`). Every failing-first test in the overhaul depends on the test seeing the worktree copy.

What changes. Ten surviving tests get the line other tests already use: `$hookDir = Split-Path -Parent $PSScriptRoot` (the pattern in `test_h16_shell_scan.ps1:12` and `test_h25_windows_path_gate.ps1:8`). They are h2, h4, h6, h11, h12, h13, h20, h21, h29 and h30 (from my grep of the tests folder). Six more hard-code the path but test hooks P2 retires (h3, h8, h9, h10, h22) or keep a fallback only (h18); those go with their hooks. Two surviving hooks dot-source by absolute path: `no_recursive_delegation_gate.ps1:13` loads `transcript_lib.ps1` from the live folder; `posture_inject.ps1:12` does the same, but F7 rewrites that file, so F7 owns it. Five retired hooks have the same line and die with P2.

Files owned: the ten tests named above, and `no_recursive_delegation_gate.ps1`.

Failing-first test: copy the hooks folder to a scratch folder, change one word in a deny message in the scratch copy, run each converted test from the scratch copy; before the change the test reports the live wording (red), after it the scratch wording (green). "Done" is all ten tests pass from the live folder and from a scratch copy. The executor runs only those ten.

Depends on: F1 (so the change is a commit on a branch). Everything in P2, P3 and P4 depends on F2.

Size: small, about 30 tool calls.

### F3. Write the hook-work procedure down

Why: executors and reviewers read their brief and the preamble; neither says where the hooks' worktrees live or how a change goes live. Memory `stage-by-name-in-shared-checkout` shows what happens when the shared-checkout rule is only in someone's head.

What changes. A new `C:\Users\Patrick\.claude\hooks\README.md` says, in the words of the section "How the work runs" above: hooks are a git repo at `~\.claude`; `main` is live; work happens in `~\.claude\worktrees\<task>`; tests run from the worktree; the main session fast-forwards `main`; rollback is `git revert`; retirements are two commits; `settings.json` is the main session's. The preamble gets one sentence pointing at it (F9 owns the preamble, so F3 hands F9 the sentence). This README is not a doc-review-gated page (the library gates four public pages, `lib\doc_review_rules.ps1:53`), and it is new, so the user approves its content here.

Files owned: `C:\Users\Patrick\.claude\hooks\README.md` (new).

Test: none; "done" is the file matches the approved procedure.

Depends on: F1. Size: small, about 10 calls.

### F4. Delete the backup files

Why: 195 `.bak` and `.retired` files are 74 percent of the hooks folder (scout H). Once F1 holds the live versions in git, they are noise, and one test even asserts a backup exists (`test_h20_suite_gate.ps1:220` checks `dispatch_bash_ps.ps1.bak-p0-3a`).

What changes. The executor zips every `*.bak-*` and `*.retired-*` under `hooks\`, `hooks\lib\` and `hooks\tests\`, plus the top-level `settings.json.bak-*` and `model-postures.md.bak-onereview`, into `C:\Users\Patrick\.claude\backups\hooks-bak-2026-10-10.zip`, then deletes them. The zip is the only history older than F1's first commit; the user can delete it later. The `test_h20` line 220 check goes, since git now answers "is there a backup". Deleting by name pattern without `-Recurse -Force` keeps the risky gate quiet (`bash_risky_gate` denies `Remove-Item -Recurse -Force`, audit "Noise"); the executor passes an explicit file list.

Files owned: the backup files themselves, the zip, and `hooks\tests\test_h20_suite_gate.ps1`.

Failing-first test: run `test_h20_suite_gate.ps1` after deleting; its line 220 goes red; remove the check; green. "Done" is no `*.bak-*` or `*.retired-*` under `~\.claude`, and the zip opens.

Depends on: F1 committed first. Overlaps F2 on `test_h20` only if they run at once, so F4 runs after F2. Size: small, about 15 calls.

### F5. Remove the stale project copies and the dead plan hooks

Why: `hydra-test\.claude\hooks\` holds seven hook copies dated 3 September that nothing registers (scout H; `settings.local.json` has no entries for them). `posture_inject.ps1:22-23` reads `hydra-test\.claude\model-postures.md` before the global copy, so two identical 10,499-character copies will drift (scout I). The two `prompt`-type hooks in `settings.local.json` lines 52-73 fire only on writes under `C:\Users\Patrick\.claude\plans\`, which does not exist (`Test-Path` False today), so they never run.

What changes. Delete `hydra-test\.claude\hooks\` (seven scripts and its `state\` folder), `hydra-test\.claude\model-postures.md` and its `.bak-onereview`, and `settings.local.json.bak-d46` and `.bak-gatechange`. In `settings.local.json`, remove the two `PreToolUse` entries and keep the `Stop` bullet check, which works (audit "What's working"). `hydra-test\.gitignore:29` ignores `.claude/*`, so these are plain file deletions with no commit in hydra-test. F7 removes the project-first lookup from `posture_inject.ps1` so there is one home for postures.

Open point: the `Stop` bullet check enforces the global `CLAUDE.md` style rule but lives in the uncommitted project file, so a fresh clone loses it (scout H). It could move into `~\.claude\settings.json`, where F1 puts it under git. That is a registration change, so the main session would apply it. I recommend the move; the user decides.

Files owned: `hydra-test\.claude\hooks\**`, `hydra-test\.claude\model-postures.md`, `hydra-test\.claude\model-postures.md.bak-onereview`, `hydra-test\.claude\settings.local.json` and its two `.bak-*` files.

Test: none to go red; "done" is the files are gone, `settings.local.json` still parses (`ConvertFrom-Json`), and the Stop check still fires on a bullet-wall reply in a new session.

Depends on: nothing in git terms; run it alongside F1. Size: small, about 10 calls.

### F6. Prune `hooks\state`

Why: nothing prunes the state folder except `shrink_guard` (scout E). My count today: 2,982 top-level files (1,982 `agent_*.json`, 223 `watchdog_*`, 204 `fanout_*`, 157 `reruns_*`, 128 `stalls_*`, 109 `status_seen_*`, 55 `stopgate_*`, 48 `suite_*`, 30 `wf_*`, 23 `handoff_*`, 17 `delegate_*`), `status\` 1,896 files oldest 2026-09-04, `shrink_guard\` 1,348 files and 31.7 MB with a file from 2026-08-15 even though no agent folder is older than 3 days, so something in it escapes its own pruning. `stall_inject` lists folders on every main-session call, so growth costs time, not just disk.

What changes. A new `hooks\state_prune.ps1` deletes top-level state files and `status\*.md` whose last write is older than 7 days, and whatever in `shrink_guard\` the executor finds unpruned, by the same age. It leaves `derive_once\` and `doc_review\` alone: those are review records the git gate reads, and I do not know how long a range can wait between review and merge. The retired hooks' families (`fanout_`, `reruns_`, `stopgate_`, `watchdog_`, `stalls_`, `emptyfleet_`, `delegate_`) need no special case; they age out in a week. The script runs once per session start. That is a new `SessionStart` registration (the event exists, W1.md line 27) that the main session adds; it takes effect in new sessions. The 7 days is a new number with no source but the ages above; the user confirms it (question 3).

Files owned: `hooks\state_prune.ps1` (new), `hooks\tests\test_h39_state_prune.ps1` (new).

Failing-first test: the test builds a scratch state folder with files dated 8 days and 6 days old, runs the script against it (the script takes the state root as a parameter so the test never touches live state), and checks only the 8-day files are gone and `derive_once\` and `doc_review\` are untouched. Red before the script exists.

Depends on: F1, F2. Size: small, about 25 calls.

### F7. One posture for the main session

Why: `posture_inject.ps1` picks a block by the model of the last assistant turn (lines 14-19). The main session runs on Opus, so it receives the OPUS block, which tells it "You are executing one task that someone else scoped" (`model-postures.md:49`). That is executor text for the orchestrator, and it tells executors to run Sonnet workers (line 51) while the preamble says "You have no helper agents" (`brief-preamble.md:63`). The FABLE block, 7,694 characters (scout I), is never injected.

The design point: `UserPromptSubmit` fires when the user sends a prompt. Subagents have no user prompts, so this hook only ever runs in the main session, whatever model it is on. The model switch is therefore the wrong axis. One block, always the orchestrator, no transcript read. That also removes `posture_inject`'s dependency on `transcript_lib.ps1` and the project-first lookup (lines 22-23), so the hook shrinks to: read `~\.claude\model-postures.md`, print it as `additionalContext`.

The contradiction is resolved by dropping the Delegation rule, not by softening the preamble. Executors do their own work (the preamble wins, and P4's agent definitions can take the `Agent` tool away from them). The orchestrator fans out; executors do not.

Proposed new `model-postures.md`, whole file:

> You are the main session. You orchestrate: you own the requirements, the judgment, the integration and the final check. Agents build. The rules below are the ones that keep going wrong; CLAUDE.md has the full set.
>
> Nothing starts without the user's yes. Every plan and every build waits for it. A code-only change needs no approval of its design, but its build still waits. When the user asks for one specific edit, the request is the approval.
>
> Independent tasks run at the same time, each in its own worktree, never one after another. Each task is one executor on Opus, briefed with the preamble (`docs/agents/brief-preamble.md`) first, then the task id, the session id, the files it owns, the tests it may run and the status rule. Executors have no helper agents; if a task is too big for one executor inside 100 tool calls and 20 minutes, it is two tasks. Read-only scouts and reviewers run on Sonnet. One fresh reviewer per change fixes its own findings and signs off; then you merge. Nothing goes back to the executor and no second round (D107).
>
> Do small edits yourself. Anything that needs a build, a test run or more than a few files goes to an executor. Hook changes go through a worktree of `~\.claude` (`~\.claude\hooks\README.md`).
>
> While agents run, read their status lines (the hook relays them to you) and scout the next step. Act on a stall at once. Relay progress to the user in one plain sentence per agent. When agents return, check their claims against the code, build once, run the full suite once, then merge.
>
> Never trust another agent's summary. Never run or brief a full library run without the user's permission in chat. Every number you report says where it came from. Write plain English: lead with the plain version, one idea per sentence, no bullet walls of file:line references.
>
> Keep this conversation under 500K tokens; the context hook warns you above that. End the phase and let compaction happen before the next dispatch rather than carrying a huge context into it.

That is about 2,200 characters against 1,926 today (scout I's count of the OPUS block), so the per-prompt cost stays level. The 500K figure is the threshold `stall_inject.ps1` warns at (scout E, job three). The 100-call and 20-minute figures are `agent_progress_gate.ps1`'s warning points (audit; rule 7). Nothing else in the text is a number.

Files owned: `C:\Users\Patrick\.claude\model-postures.md`, `hooks\posture_inject.ps1`, `hooks\tests\test_h40_posture.ps1` (new; today only the UTF-8 smoke test touches this hook, scout H).

Failing-first test: feed the hook a `UserPromptSubmit` payload whose transcript's last assistant model is Opus; red when the output contains "executing one task"; green when it contains "You are the main session". A second case: a payload with no transcript still gets the posture (today it prints `{}`, line 20).

Depends on: F1, F2, and the user approving the text above. P2's retirement of the Fable-only gates is independent, but the FABLE block's claims about them ("a hook denies...") disappear with this rewrite either way. Size: small, about 25 calls.

### F8. Retire the SubagentStop LLM hook

Why: `settings.json` lines 94-103 run a model-judged hook after every subagent finish to check two phrasings (a flat "does not exist" claim after a narrow search, and a "someone else changed this" claim without reflog evidence). The audit found no recorded catch in the 60 newest sessions. Each run is a model call per subagent.

What changes. The registration goes (main session applies it). Its two rules move into the preamble as two sentences, where every agent reads them (F9 owns the preamble, so F8 hands F9 the text): "Say where you looked. 'Not found where I checked' is a different claim from 'does not exist'; make the weaker one unless you searched everywhere it could be." and "Before you say someone else changed a file or a commit, run `git reflog -10` and `git log --format='%h %ad %an'`, or say origin unknown. Your own earlier Write, Edit or commit counts as evidence that the claim is false." The first already exists in the OPUS posture (line 57), which F7 drops, so this is a move, not a loss.

Files owned: none beyond the preamble text handed to F9; the `settings.json` change is the main session's.

Test: none; "done" is the registration is gone and the two sentences are in the preamble.

Depends on: F9. Size: tiny, folded into the main session's merge and F9.

### F9. Correct CLAUDE.md, the preamble, doc-review.md and the duplicated rules

Why: scout I checked each claim against the code. CLAUDE.md's "Doc review" section describes a scope the library no longer has. The preamble says "Commit before your 110th tool call" when the hook warns at 100 (`brief-preamble.md:57`; `agent_progress_gate.ps1`, scout I), and its `.new`-file advice sidesteps the shrink guard on exactly the text the guard protects (`brief-preamble.md:29`; scout I item b). D107, the status rule and the call limits are written in four to six places each.

What changes, with the proposed wording. These are hydra-test files, so they go through the usual branch, derive-once review and merge; none of them is a doc-review-gated page (`lib\doc_review_rules.ps1:53`).

**CLAUDE.md, "Doc review" section**, replaced by:

> Four public pages get a document review before they change on `main`: `README.md`, `docs/README.md`, `docs/UserGuide.md` and `docs/differences-from-public-hydra.md`. So do Artifact publishes, plans shown with ExitPlanMode, and text posted with `gh`, when the text is over 20 lines; 20 lines or fewer passes without a reviewer. A hook enforces it. Dispatch the reviewer with `docs/agents/doc-review.md`, the key and the review kind the hook's message gives. Only the user can skip a review, by typing `waive doc-review <key>` in chat.

The 20-line rule is the user's decision 5 and P3 builds it; this wording waits for P3's exact line-counting rule, and the user confirms whether the four pages also get the 20-line pass (P3's question).

**CLAUDE.md, "Agent rules"**, rule 7 becomes: "Agents write a status line every 5 minutes. At 100 tool calls or 20 minutes an agent starts wrapping up: it commits what it has and reports what is left. 150 calls or 30 minutes is a hard stop. A reviewer hands nothing on; it signs off what it fixed and lists the rest in its review's Notes." Rule 5 drops its second half (the time-out text), since rule 7 now says it once. The closing sentence "For rule 2, a hook checks only that a Fable session's dispatches name a model" becomes "No hook checks rules 2, 3 or 9" until P4's agent definitions land, when P4 rewrites it.

**Preamble.** "Status lines": "every 10 tool calls or every 5 minutes, whichever comes first" becomes "every 5 minutes", and "A hook warns you when a line is due" stays. "Editing files": the sentence "To replace most of a file, Write `<file>.new` and move it over the old one, because the shrink guard blocks large deletions made through Edit" becomes "The shrink guard blocks only deletions of text git cannot restore; Write and Edit over a committed file pass. If it blocks you, stop and report; do not move a file over the old one to get past it." "Committing": "Commit before your 110th tool call" becomes "Commit after each finished step, and when the hook warns you at 100 calls or 20 minutes." The trailer example's model line gets "your brief gives the model line" moved in front of the example, so a Sonnet reviewer does not copy "Opus". "Helpers and waiting" keeps "You have no helper agents; do the work yourself" and adds the reason: "the main session fans work out; an agent that fans out again cannot be budgeted." "Review findings" shrinks to two sentences and points at CLAUDE.md rule 5. F8's two sentences go under "The code is the source of truth". F3's sentence goes under "Editing files": "Hooks under `~\.claude\hooks` are a git repo; never edit them in place. `~\.claude\hooks\README.md` says how."

**docs/agents/doc-review.md.** Line 19 loses "The kind of review is full or edit" and the edit-review sentences after it. The "Decision questions" paragraph (line 51) goes, since the gate no longer reviews questions (`doc_review_tool_gate.ps1` header, scout I). The whole "The edit review" section (lines 130-136) goes, and line 124's reason for the third submit word ("so the next edit of that document can get an edit review") becomes "so the gate keeps the text you signed off". The pointers at "plan section 3.7, 3.8, 4, 5, Q2" of the 2026-10-08 plan point at the library, `lib\doc_review_rules.ps1`, which is the code. The status-line paragraph (line 9) becomes "The preamble's status rule applies." The paragraphs "A finding you should not fix yourself" and "If you run out of time or tool calls" (lines 87-89) shrink to one sentence each pointing at the preamble's "Review findings", and `derive-once-review.md` lines 78-80, which are the same paragraphs with other nouns, do the same.

Files owned: `hydra-test\CLAUDE.md`, `docs\agents\brief-preamble.md`, `docs\agents\doc-review.md`, `docs\agents\derive-once-review.md`.

Test: none. "Done" is: every number in the four files matches the hook that enforces it (the executor greps `agent_progress_gate.ps1` for 100, 150, 20, 30 and 5), and no sentence describes edit reviews, question reviews, the 110th call or the `.new` trick.

Depends on: the user approving the wording; P2's minutes-only status rule (so the preamble is not ahead of the hook); P3's dead-code removal and 20-line rule (so doc-review.md and CLAUDE.md are not ahead of the library). So F9 is the last P1 task to merge. Size: medium, about 40 calls, mostly reading the four files in full.

## Order and overlaps

F1 first, alone; it is the ground everything stands on. F5 can run beside it, since it touches only hydra-test's ignored `.claude` folder. Then F2, which every other hook task in P2, P3 and P4 waits for. F3, F4, F6 and F7 are independent of each other after F2 and can run as one wave, with F4 after F2 because both touch `test_h20`. F9 (with F8 folded in) goes last, after P2's status change and P3's library cuts.

Overlaps with other planners: P2's retirements delete five hooks that F2 would otherwise fix (`fanout_track`, `fanout_mark_delegated`, `fable_delegate_gate`, `executor_dispatch_brief_gate`, `rerun_gate`) and six tests (h3, h8, h9, h10, h22, and h18's fallback); F2 leaves them alone. `settings.json` belongs to the main session for every planner's tasks. P4's agent definitions will own the rule-2 sentence in CLAUDE.md after F9.

## Open questions for the user

1. **What goes in the repo?** F1 tracks `hooks\` (minus `state\`), `settings.json`, `CLAUDE.md`, `model-postures.md` and `skills\` (231 files, 3.9 MB, your two skills). The alternative is hooks only. Example for tracking `skills\`: a change to `analyzing-sp-squeezes` becomes a diff you can read. Example against: if any skill folder holds something you would not want in git history, it is in forever. I recommend tracking them.

2. **Should the derive-once git gate guard `~\.claude` too?** CLAUDE.md's rule reads "Merges and commits on `main` that touch code need a review from a fresh agent first. A hook enforces it." Installing the gate makes a hook change wait for its reviewer's record before it can go live, the same as hydra-test. Example for: a bad dispatcher merge is the most expensive mistake on the machine, and a reviewer is the only check between the executor and every session. Example against: a one-line revert of a broken hook is also a move of `main`, so in an emergency you type `waive derive-once <key>` first, which costs a minute while every tool call is being denied. I recommend installing it and accepting the waiver step; the alternative is to exempt `git revert` commits in the gate, which is P3's library and a bigger change.

3. **How old is old in `hooks\state`?** F6 proposes 7 days, from the ages above (the oldest `status\` file is 2026-09-04; `agent_*.json` holds 1,982 files for agents that are finished). Example at the short end: 3 days would already prune everything from before this week, but a session resumed after a long weekend would find its agents' status files gone, and `stall_inject` relays from them. Example at the long end: 30 days keeps about four times today's file count and the folder scans grow with it. The script never touches `derive_once\` or `doc_review\`; say if you want those aged too and at what age.

4. **Move the Stop bullet check into the user settings?** It lives in `hydra-test\.claude\settings.local.json`, which is not in git (`hydra-test\.gitignore:29`). Moving it into `~\.claude\settings.json` puts it under F1's git and makes it run in every project. Example for: the rule it enforces is the global `CLAUDE.md`'s, not hydra-test's. Example against: it would also fire in `C:\ytdl` or any other folder you open. I recommend the move.

5. **Does the posture text in F7 say what you want the orchestrator told?** It is the one text injected on every prompt, so every sentence costs context every turn. Anything you would rather keep in CLAUDE.md (read once per session) should move there instead.

## Numbers and where they come from

Live scripts, lines and the 195 backups: the audit's table and scout H's listing. Folder sizes under `~\.claude` and the state file counts: my own `Get-ChildItem` pass on 2026-10-10 (projects 2,571 MB, plugins 80 MB, file-history 51 MB, hooks 42.5 MB, skills 3.9 MB; state top level 2,982 files and 2.3 MB; `shrink_guard` 1,348 files and 31.7 MB; `status` 1,896 files and 1.1 MB). Which tests hard-code the live path: my grep of `hooks\tests` for `C:\Users\Patrick\.claude\hooks`. Which hooks dot-source by absolute path: my grep of `hooks\*.ps1` for `. 'C:\Users\Patrick\.claude\hooks`. Posture block sizes (7,694 and 1,926 characters): scout I. Warning and stop points (100/150 calls, 20/30 minutes, 5 minutes): the audit and scout I's check of `agent_progress_gate.ps1`. The 500K context warning: scout E's reading of `stall_inject.ps1`. Hook snapshot at session start: the three secondary web sources above, unverified against Anthropic's page; F1 verifies it.
