# Handoff: plan the next phase of the derivation fixes (2026-10-04)

Written by session a9a461da while it finishes D36. Your job is to plan what comes after steps 1 and 2. Don't start code until the user approves the plan.

## Where things stand

Main has steps 1 and 2 of the derivation fixes, plus T10 and T11. Everything was merged locally; nothing is pushed. Each merge went through the derive-once review gate and came back CLEAN first.

- Step 1 (the engine stores what the display used to guess) merged as 3ee32df after seven review rounds.
- Step 2 (the chart readers follow Clone Hero's rules) merged as 4aabca6 after four rounds.
- T10 and T11 (a cap-clamped SP end is offered its squeeze; a backend at exactly +3.0 ms stays uncounted) merged as 497770f after three rounds.
- The results stamp is "2.1.0", the path format is 7 and both Dynamics stamps are 2. No release tag carries 2.1.0 yet, so everything above ships in one re-analysis.

D36 (the extreme-tempo gaps) is merged too, as 7266643. The user approved it after seeing the counts (D36 addendum, 2497837). It took three review rounds: 10 code-only findings, then one stale comment, then CLEAN at a213d3d. The merge check: 825 tests pass, the UI tests 62 of 62, the selfcheck 382 of 382, and corpus scores identical to main on all 97 charts. Its worktree and branch are removed.

Journal state at handoff, pasted as the hook gave it:

```
Workflow journals:
  wf_b468db9f-6a4: started=9 result=9  (cleanup-wave-2, completed)
  wf_d28b6e6b-359: started=14 result=13 (cleanup-wave-1, killed earlier by the user's request; nothing pending from it)
Subagents active in the last 30 minutes and not finished:
  agent-a2e6ddb70767d5389: the D36 derive-once reviewer, last tool call 15:53
```

(That reviewer, its fix agent and two more review rounds all finished before D36 merged. Nothing from this session is still running.)

Other sessions own the worktrees `busy-gagarin-49ebe6`, `modest-tharp-559205` and `C:\Users\Patrick\AppData\Local\Temp\claude\mm`. Leave them alone.

A separate session, started from a task chip, is investigating a one-off "bad allocation" for the bundled track `Good Grief Retreat.srb` during a whole-library batch run. Alone, the chart analyzes in 0.1 s with under 10 MB, so the batch path misbehaves now and then.

## How much of the audit is done

The audit has 396 findings (`docs/audit/2026-10-03-derivation-audit.md`). The triage (`docs/audit/2026-10-03-fix-triage.json`) sorted them into seven steps. Steps 1 and 2 were assigned 45 of them: 24 for step 1 and 21 for step 2.

The steps actually closed more than that, because each review round folded in copies it found nearby. Counting the finding numbers the two plans name, plus the ones the reviews and decisions D39 to D47 closed, about 60 findings are fixed. That's roughly 15% of the audit.

- Step 1's plan names 1, 5, 24 to 30, 32, 48, 52, 89, 90, 91 (its doc half), 100, 127, 128, 137, 144, 145, 147, 149, 156, 159, 259, 303, 332, R7.6 and R7.10.
- Step 1's review rounds also closed 138 (D41), R7.29 (one keep-every-path band), R7.30 (D42, no test reference copies), 246 (`search_target` builds its graph like the main search) and half of R7.25 (D39).
- Step 2 closed 10, 11, 12, 21, 53 and 314 (recorded as unverified, D26), 60, 61, 64, 77, 222, 250, 255, 258, 315, 319, 320 and 141 (both recorded only), 331, 344, R7.5 and R7.7. It also closed part of 117, 277 and 286 (shared test builders).
- T10 and T11 closed 37 and 304.
- D47 (a `.chart` drum section is read in tick order) came from step 2's review, not the audit.

The triage counts are a good guide to what's left, but they were taken before any fixes landed. Re-check status before planning; the fastest way is to grep the decisions file and the merged commits for each finding number.

| Step | Findings in triage | State |
|---|---|---|
| 1, engine facts | 24 | done |
| 2, parser | 21 | done |
| 3, display format | 49 | not started |
| 4, docs | 44 | not started |
| 5, shell path | 1 | not started (long paths, ADR 0020, may already cover it) |
| 6, fold duplicates | 182 | not started, except copies the reviews folded |
| 7, other | 75 | not started |

## Already known to be open

The reviews kept listing findings that the steps touched but left for their own fix. These are the first candidates for the next plan:

- 146, the trim that moved into `set_sqout`, and `display_backends` mirroring it.
- 149, the replay's own "at or before D counts as inside" clamp.
- 1 and 32, the replay's `past_deact` tick test. It's listed as a known copy in the single-owner scan.
- 128, `hydra_replay`'s JSON writing and reading `sqout_tick` and the offset separately.
- 150, the squeeze-out cost worked out in both `squeeze_sentences` and `build_activations` in `path_view.cpp`.
- 177, the legacy fill-rule mapping in `search_target`.
- 183, rounding ms to a tick by hand in tests.
- R7.11, the three `midi.cpp` file-size entries still on the scan's known-copies list.
- R7.25's other half: the Preview loader sizes each loose stem twice per load.
- 117, 277 and 286's leftovers: the `.sng` half of the test builders, `test_song.cpp`'s own MIDI writers, and one corpus loop.
- D26's unverified Clone Hero rules (generated-fill length, 6/8 beats). These need a live Clone Hero check, which the user deferred.
- The round-7 pending user calls in the audit memory: negative `.chart` tempo, Offset "500ms" read as 500 s, "Open automatically" scope, FLAC length-0 silence, Opus seek-after-damage, and the 248-character cut-off.

The two review notes that weren't findings: two copies of "where does the SP track start" (test_search.cpp, test_fast_tempo.cpp), and `deact_edge_at` as a possible shared helper for them.

## Rules the user set this session

These are in memory, but planning should follow them from the start:

- Anything that changes an app display or a stored record goes to the user first, with the reason in game terms. Code-only changes go ahead on your recommendation and get reported afterwards (memory `confirm-before-coding`).
- Agents never run the full test suite. They run only the tests relevant to their change. The main session runs the full suite once, when merging (memory `no-repeat-suite-runs`).
- Library comparison runs go in parallel, each with its own scratch database. Only a timing run stays alone on a quiet machine (memory `parallel-by-default`).
- Hooks and agents fail loudly: deny with a reason, never "ask" where nobody can answer (memory `fail-loudly-never-freeze`).
- Every code merge to main needs a CLEAN derive-once review from a fresh agent (`docs/agents/derive-once-review.md`).
- Reviewers run helpers in the foreground and never end a turn to wait. Fix agents commit before about 110 tool calls (memory `derive-once-gate-2026-10`).
- Don't dispatch a follow-up a running workflow already handles. Read the script's control flow first (memory `fleet-orchestration-lessons`).

## Suggested way to plan

1. Re-check each open finding's status at main's head, and update the triage counts.
2. Group what's left by the question each finding answers, not by file. Step 6's 182 duplicates are the bulk. Many collapse into one owner per question, and the single-owner scan can guard each one.
3. Split out anything that changes a display or a record. Those need the user's calls before the plan is final, so ask them as one round, with the reason in game terms.
4. Plan tasks so they run in parallel worktrees, each with the tests relevant to it, and one integration branch per wave.
5. Cite prior art from GitHub or Stack Overflow where a fix pattern exists (memory `search-web-before-finalizing-plans`).

## Loose ends

- Three empty folders under `.claude/worktrees` (`admiring-fermi-4a140e`, `bold-wilson-6af6e3`, `objective-maxwell-56ed0c`) were held open by another process. Delete them once nothing holds them.
- Branches kept because they never merged: `claude/s2-proto-disco`, `claude/s2-proto-fillb`, `claude/s2-proto-kick2x`, `claude/s2-proto-phrasefill` (measurement prototypes) and `claude/s1-flake` (duplicates of a fix already on main). Ask the user before deleting them.
- `docs/handoffs/2026-09-29-public-timing-scripts/blink/results/blink-both.json` is 15 MB and untracked on purpose.
- The UTF-8 hook fix left `.bak-d46` and `.bak-d46b` backups beside every changed hook in `C:\Users\Patrick\.claude\hooks`, plus `settings.json.bak-d46` and the project's `settings.local.json.bak-d46`. They can go once the user is happy with the hooks.
- Nothing is pushed. Ask the user before pushing.
