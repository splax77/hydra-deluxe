# Handoff: the derive-once review gate (mid-build) and audit round 7

This picks up from `docs/handoffs/2026-10-03-derivation-fixes-planning-handoff.md`. That session asked for a quick pass over the day's new code and more searching for what the audit missed. Both are done. Along the way the user asked for a review gate so the same mistakes stop reaching `main`. Its plan is written and is half built. Nothing from this session is on `main` yet.

## Where to start

Two jobs are open, and they don't depend on each other.

The first is finishing the review gate. It is stopped partway through Task 3's fix loop. The section below gives the exact next step.

The second is fix planning for the audit. The user asked whether a round 8 is needed. The recommendation given was to go to planning, for four reasons. Round 7's finds came mostly from that day's 34 new commits, not from code the first six rounds missed. The fix plans will re-read every area they touch. The gate will catch new copies going forward. And a round costs about 1.5 million tokens. The user hasn't answered that yet, so ask. If they want round 8, aim it at what round 7 only spot-checked: `src/audio`, `src/cli`, `src/net` and `tools/`.

## Audit round 7: what's in the report now

The audit report `docs/audit/2026-10-03-derivation-audit.md` has a new section, "Round 7 and the 2026-10-03 code (added after delivery)", just before the refuted-candidates list. It holds R7.1 to R7.51. 44 are findings: 11 drifts, 19 duplicates that agree today, 6 undecided assumptions and 8 docs that disagree with the code. The other seven are status notes on what that day's commits did to the old findings. Section 3 and "The short version" each gained one paragraph or sentence about it. Section 4 gained a fix-list entry, which the user approved: one owner for "does the Windows shell take this path".

Five finders and four verifiers produced it, all against HEAD 02976c1. Every candidate was checked by a different agent. The stop rule is still not met, because round 7 found new items. Don't describe the audit as complete.

Several round-7 findings need the user's call before anyone plans a fix:

- **Negative `.chart` tempos (R7 drift).** The parser accepts `B -60000`. The engine and the new replay walk then disagree, and the Preview shows "Score unavailable". Should such a tempo be rejected?
- **Offset parsing.** A `.chart` Offset written as `500ms` is read as 500 *seconds*. song.ini's delay drops the same text instead. Both parsers accept `nan`.
- **"Open automatically".** It also opens every dmleaderboards comparison, though it's documented for batches only.
- **Two new audio behaviours from the streaming change.** A FLAC whose header says "length unknown" is now silent in the Preview. A damaged Opus stem stays silent after a seek back.
- **The 248-character long-path cut-off.** ADR 0020 only names 260.

The finders' and verifiers' working files are copied into `.superpowers/sdd/2026-10-03-derive-once-review-gate/keep/`. That covers the candidates, the four verdict files, the literal list and the probe scripts.

## The review gate: state and next step

The plan is `docs/superpowers/plans/2026-10-03-derive-once-review-gate.md`, with its task file next to it. The user decided four things on 2026-10-03, recorded in the plan header. The gate covers merges and commits on `main`. A FINDINGS verdict blocks until fixed or waived. Old findings that a change only moves get listed, not blocked. It is executed subagent-driven in this session's style.

The ledger is `.superpowers/sdd/2026-10-03-derive-once-review-gate/progress.md`. Read it first. It names every dispatch, review, ruling and minor. Note that `.superpowers/` is **not** git-ignored in this repo, despite what the skill says, so never stage it.

**Task 1 (single-owner scan test)** is done and reviewed, commit d6e1d8b. **Task 2 (reviewer brief, marker file, CLAUDE.md pointer)** is done and reviewed, commit 3551b5e. Both sit on branch `claude/derive-once-gate` in the worktree `.claude/worktrees/derive-once-gate`. They are **not merged**.

**Task 3 (the three hooks and their tests)** is in its fix loop. Round 1 fixed seven of eight review findings, and its tests pass 56 of 56. But the re-review found one finding still open and some new breakage. Fix round 2, sent to the same implementer or a fresh one, must cover:

- **I3, still open.** A `$(...)` inside the submit helper's path or file argument records a fake review and runs a merge in the same call. The reviewer proved it in a throwaway repo. The anchored submit match must reject `$`, backtick, `(`, `)`, `;`, `&` and `|` in its arguments, with a test for both forms.
- **Redirects now block legitimate work.** `git merge feat 2>&1`, `> out.txt`, `2>$null` and `2>/dev/null` read the redirect as a second branch name. `git commit -m "x" 2>&1` and the `-F - <<'EOF'` form read it as a file name. Strip redirect tokens before both token walks, and add regression tests.
- **Other ways to stage in the same call.** `git checkout x -- src; git commit`, `git restore --staged ...; git commit` and `--pathspec-from-file` still slip past. Deny a commit on `main` that shares its command with any non-read-only git call.
- **Minors.** Deny a merge target that doesn't resolve. Read `--cleanup` as a flag that takes a value. Add to the plan's Known limits: a merge inside a quoted `$(...)` is invisible.

The re-review's full text is the last Task 3 entry in the ledger, and the reviewer's probe scripts are in `keep/`. Then do a scoped re-review, then the rest of the plan.

There is one ruling the user should know about. The plan said "merge Tasks 1 and 2, then register the gate". The executor reordered it, because a merge to `main` needs the user's yes. The order now is: register the hooks (Task 4, done by the main session, since subagents can't edit `settings.json`). Then **ask the user** before merging the branch. That merge becomes the gate's first real test: it should be denied, a fresh reviewer reviews the branch, and the merge goes through. Then Task 5's synthetic probe, the final whole-branch review, and finishing the branch.

The hooks are written but **not registered**. Today they do nothing. They live in `C:\Users\Patrick\.claude\hooks\`, which is not a git repo: `derive_once_review_gate.ps1`, `derive_once_submit.ps1`, `derive_once_waiver.ps1`, and `tests\test_h15_derive_once.ps1`.

The plan text is now behind the code in three places. Update it before the final review. First, its Task 3 gate code lacks the `return ,@($out)` fix. (PowerShell flattens a one-line result, so the branch check read `"m"` instead of `"main"`.) Second, its Known limits are missing the implementer's and reviewers' additions. Third, the scan test's rules are narrower than their questions. For example, "how many bytes does a file hold" doesn't catch `GetFileSizeEx`. That is logged as a minor for the final review.

## The commit-attribution hook fix (side job, done, not reviewed)

The user said "instead of trying to bypass the hook fix the hook". `git_attribution_gate.ps1` used to treat any command containing the words "git commit" as a commit, even inside quoted text or a here-doc body. It now finds commits, merges and rewrites on a copy of the command with quoted strings and here-doc bodies replaced by a placeholder. It still reads the trailers from the original text. 17 tests were added to `test_h5_git.ps1`, and 8 of them failed on the old gate. All 15 hook test files pass. The original is saved as `keep/git_attribution_gate.orig.ps1`.

It hasn't had an independent review yet. Give it one. The implementer flagged two concerns. A real commit hidden inside `bash -c "git commit ..."` is no longer seen. And a double-quoted string ending in a backslash can pair quotes wrongly.

## Rules for the next session

Explain how a thing works today and what would change, and get a yes before editing code. Any visible change is a blocking question up front. Write plans and reports in plain English per CLAUDE.md. Stage by file name, never `git add -A`.

Every agent brief carries the status-line sentence with `hooks\state\status`. Every brief should also hand the agent the session id for its commit trailers. Agents without it wrote `Session: unknown` (d6e1d8b) or put the worktree name in `Agent:` (3551b5e). Neither was rewritten, by ruling.

When a hook blocks something it shouldn't, fix the hook. Don't route around it.

Keep fan-out lean: three or four agents per discovery round, given the previous round's "what I searched" notes. Batch small jobs. Run one CPU-heavy tool at a time.

## Journal state at handoff

The handoff hook supplied no journal lines. No workflows ran in this session. Every subagent dispatched in this session finished and reported. Nothing is in flight. The status monitor expired and was not re-armed.

## Files this session changed

On `main`'s working tree, uncommitted:
- `docs/audit/2026-10-03-derivation-audit.md`, still untracked: the round 7 section and the section 4 addendum.
- `docs/superpowers/plans/2026-10-03-derive-once-review-gate.md` and its `.tasks.json`.
- This handoff.
- The `.superpowers/` workspace.

On branch `claude/derive-once-gate`: d6e1d8b and 3551b5e. In `C:\Users\Patrick\.claude\hooks\`: the three new hooks, `test_h15_derive_once.ps1`, the fixed `git_attribution_gate.ps1` and the extended `test_h5_git.ps1`. Release binaries in `build-cpp\Release` were rebuilt from 02976c1 on 2026-10-03.
