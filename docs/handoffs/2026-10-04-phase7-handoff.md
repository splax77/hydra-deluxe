# Handoff: run phase 7 of the derivation fixes (2026-10-04)

Written by the phase 7 planning session (b9839bd7). Phase 7 is planned and approved, but no code has started. The user said not to launch the work from this session. Your job is to launch it, merge it wave by wave, and keep in step with phase 6.

## What to read first

The plan is `docs/superpowers/plans/2026-10-04-phase-7.md`. It lists every task, the files each one owns, the tests it runs, and the merge steps.

The answers are D51 at the end of `docs/audit/2026-10-03-fix-decisions.md`, which answers `docs/audit/2026-10-04-phase-7-questions.md`. The user answered "all recommended", with two notes in their own words. On 4- and 5-note chords: they never appear in a real drums chart, but Clone Hero allows them, so Hydra must count them correctly. On the early-fill window: they tightened it from 85 to 60 ms on purpose, because early fills that never matter were cluttering paths.

Every brief starts from `docs/superpowers/plans/tasks/_phase7-preamble.md`.

All of the above is committed on main as 81a2519.

## Where things stand

Phase 7 is the audit's step 7: 75 findings. Four Fable scouts rechecked them at main. Six were already fixed: 30, 37, 304, 344, 222 and 77. Their rows in `docs/audit/2026-10-03-fix-triage.json` still say open. R7.5 and R7.6 are settled by D27 and D6, and their rows are stale too. **Correcting those eight rows' `status_at_head` is still to do.** The plan's "Decisions written before code" section says the main session does it; D51 itself is written.

The scouts' reports were not saved to the repo. Their transcripts were empty when this session tried to extract them. The facts they found are in the plan's task tables and in the brief writers' prompts below.

## Wave 1 briefs: all nine are written

Three Fable writers wrote the nine wave 1 briefs. Each writer checked its briefs against the code at 81a2519, and all three finished before this handoff was final. Nothing from this session is still running. The briefs are committed with this handoff, in `docs/superpowers/plans/tasks/`:
- **`p7-e1.md`**, the tie key (95, 330, 173, 179). It adds `Path::recount_tied_paths` in `model.cpp` to E1's owned files.
- **`p7-e2.md`**, multiplier squeezes (49, 50, 247, 335). The codec stores only the chord code and combo, so the points are worked out again on read. Stored bytes change only where a chart holds a chord the new rule lists.
- **`p7-st1.md`**, the store cache and other-rules rows (65, 343, 116, 129, 130, 132, 133).
- **`p7-se1.md`**, the settings owner. It adds `src/core/rules.h` beside `rules.cpp` for the field table.
- **`p7-t1.md`**, hydra_replay and hydra_bench. It adds `tools/replay_json.h`. It forks from SE1's commit.
- **`p7-au1.md`**, stem length and damage (R7.8, R7.9).
- **`p7-ps1.md`**, song stems (74, 101).
- **`p7-pr1.md`**, the probe helpers, which delete the six old scripts.
- **`p7-pr2.md`**, play_chart reads Hydra's dump. It forks from PR1's commit.

Read each brief's "Open questions" before launching. The writers raised the items below. The first three need the user, because they change a stored record or something on screen that D51 didn't spell out. The rest are code-only calls; take the recommendation unless you see a reason not to, and report it afterwards.

### Ask the user before launching the task

1. **ST1, the schema change (finding 65).** D51 keeps results made under other rules, but the scout was wrong about the table. The results table has a UNIQUE key (hyhash, chartmode, sp_cap, ms_enabled, ms_value, depth_mode, depth_value, legacy_fills) without the rules fingerprint. So rows from two rules setups can't sit side by side without changing the stored layout. The brief recommends schema 4, built like `add_fill_rule_column`: rebuild the table with a `rules_fp` column filled from `substr(structure,5,8)` and added to the key. Blobs and result ids stay untouched, so no re-analysis is needed. The user hasn't seen this layout change, so ask before ST1 starts. E1, E2, SE1, AU1, PS1, PR1 and PR2 don't depend on it.
2. **SE1, `depth_mode=2` in a hand-edited INI.** D51 Q14 says pull a value to the nearest edge. For depth_mode, that turns 2 into 1, which means points, but today a 2 searches by scores. The scout recommended "anything not 1 is scores". Ask which; I recommend scores, since that's what such a file has always searched.
3. **E2, the how-to line for a chord with several crossing notes.** D51 says the line names every note that crosses the step, but gives no sentence. The brief recommends "Hit [A] and [B] last." Ask for a yes, or the user's wording.

### Code-only calls (recommendation given)

- **SE1, the SP cap reading.** Keep the pinned rule that 0, junk and `auto` read as 4. That rule is an earlier user decision with its own test, and D51's clamp covers only values with a range. The minimum cap of 1 goes in the key table in `config.h`, not in `model.h`.
- **The single-owner scan rows.** The scan file, `tests/test_single_owner.cpp`, has no phase 7 owner. Let each wave 1 task add its own rows at the end of the file, and fix the joins on the integration branch.
- **T1:**
  - The shared result-block writer lives in `replay_json`, which `hydra_tests` links, so `test_replay.cpp` can pin it.
  - `--cap 0` keeps refusing, even though the INI reads 0 as 4, because a typed 0 is a mistake on the command line.
  - `hydra_bench` has no test target, so the brief has the implementer diff the two dumps by hand instead.
- **PS1, the link question.** `preview_source.cpp` is in `hydra_core`; `sniff_format` is in `hydra_audio`, which depends on `hydra_core`. The call links today only because the only code that pulls in `preview_source` lives in `src/ui` and `src/audio`. The cleaner fix is to move the pure byte check out of `decode.cpp` into `hydra_core`. But `decode.cpp` and `CMakeLists.txt` aren't PS1's files. So either add both files to PS1's owned list at launch (recommended, since no other wave 1 task owns them), or accept the hidden dependency and note it.
- **PR1:**
  - `poll_windows.window_verdict` moves into `watch_window.py`, because `tests/test_s2_window_constants.py` imports it.
  - `InputDriver.schedule_hit` is deleted, because only its own tests call it.
  - `watch_window` shares only the "has the song stopped" check with the runners, at 5 s.
  - A jump back raises its own error, so play_chart can re-sync where the edge runners stop.
- **PR2:**
  - The plan's file paths are wrong; the real ones are `tools/ch_probe/experiments/play_chart.py` and `tools/ch_probe/tests/test_play_chart.py`.
  - play_chart reads a dump JSON, because `hydra_replay` is an `EXCLUDE_FROM_ALL` exe that needs a database path to run. The dump's `ms` has no Offset or song.ini delay, just like play_chart today, so nothing shifts.
  - The 2x kick follows the dump's bass2x setting.
  - The mid-song start changes to D51's 150 ms rule.
- **E1, the cap for finding 95.** Finding 95's example ("Sugar/Tzu" keeping two paths at limit 1) came from hydra_replay's default cap. The brief assumes cap 4; if the two paths appear only at another cap, the implementer stops and reports.

Phase 3 is still running in another session ("Phase 3 continuation"). Wave C merged as 16c836c; wave D (O1 to K5, then S1) is on the `claude/p3-*` worktrees. Phase 7's wave 2 waits for phase 3's merge M_D.

Phase 6 is being planned in another session ("Phase 6 planning"), and it numbers its decisions from D52.

## The split with phase 6 (agreed by message with the phase 6 planner)

Phase 6 owns the step 6 leftovers (146, 149, 150, 177, 183, R7.11, R7.25, and the test-builder halves of 117, 277 and 286), plus 1, 32 and 128. Phase 7 took eleven of phase 6's folds because they answer a phase 7 task's question:
- E1 took 173 and 179.
- E2 took 247 and 335.
- E3 took 180, 245, 243, 324 and 261.
- SE1 took 199 and 200.

The ordering rule is one writer per file per wave across both phases, and an idle file goes to whichever phase is ready. In practice:
- **During phase 7's wave 1:** phase 6 folds `analysis.cpp`/`.h`. Phase 7 writes those files in both waves 2 and 3, so wave 1 is phase 6's only window.
- **During phase 7's wave 2:** phase 6 folds the wave 3 files: report pages, `library_*`, `app_state`, `uitest_harness` and `cli/batch.cpp`. Phase 7's wave 3 forks after that merge. To keep `app_state.cpp` free in wave 2, ST2 adds the per-difficulty song length beside the old store call, and LB switches `update_song_length` in wave 3.
- **During phase 7's wave 3:** phase 6 folds the wave 2 files: model, path_view, pather, preview_*, settings_bar, paths_tab, timing, record_store and engine.
- **After M7-1:** phase 6 folds the wave 1 files: `tools/ch_probe`, the audio readers, preview_source, scoring, config/strutil/rules_file, and tools/replay and bench.
- **Before wave 4:** all of phase 6 lands, so ER rebases onto the folds once.

Phase 6's plan is committed as `docs/superpowers/plans/2026-10-04-phase-6.md`. The user hasn't approved it yet. Its appendix, "Files per wave for phase 7", lists every file each phase 6 wave owns. Its waves map onto phase 7's like this:
- J1 runs alongside wave 1.
- J2 runs after M_D and M7-1, alongside wave 2. Phase 7's wave 3 forks after M6-J2.
- J3 runs after M7-2, alongside wave 3.
- J4 runs after M7-3, before ER.

The appendix was checked against the nine briefs and this plan. Four collisions came out of that check. I proposed the fixes below to the phase 6 planner, and its yes on the first three hadn't arrived when this was written. Check for its reply before launching T1, AU1 or SE2:
1. **`tests/test_replay.cpp`** is edited by both J1-1 and T1. T1 forks from J1-1's commit, as well as from SE1's.
2. **`tests/test_stream_mix.cpp`** is edited by both J1-4 and AU1. AU1 forks from J1-4's commit.
3. **`tests/test_app_state.cpp`** is edited by both J2-4 and SE2. SE2 forks from J2-4's commit. ST2 stays out of `tests/test_cli.cpp`, which is J2-3's; if it can't, it forks from J2-3's commit.
4. **Finding 88's flag.** The "has a scored best path" flag set in `summarize_record` moves from wave 3 into ST2, in wave 2, because phase 6's J3-6 owns `record_store.cpp` during wave 3. The plan's ST2 row includes it.

Phase 7's wave 2 tasks add no new test files, because J2-1 owns `CMakeLists.txt`. Decision numbers: D52 went to phase 3, so take the next free number when the user answers anything.

## How to run it (the user's global rules)

1. Agents never run the full test suite; they run only the tests their brief names. The main session runs the full suite once, when merging.
2. Planning agents run on Fable, code executors on Opus, and code reviewers on Sonnet.
3. Anything that changes a display or a stored record goes to the user first. D51 covers everything planned; a new display or record choice found mid-task stops the task and goes to the user. Code-only changes go ahead on your recommendation and get reported afterwards.
4. Agents never stall or go quiet. They write status lines and fail loudly when blocked.
5. Every merge to main needs a CLEAN derive-once review from a fresh Sonnet agent first (`docs/agents/derive-once-review.md`).
6. Each wave is one parallel Workflow, and the main session merges.

### Wave by wave

- **Wave 1** (E1, E2, ST1, SE1, AU1, PS1, PR1 from 81a2519, or main's head if only docs moved):
  - First, put the three user questions above to the user in one message. E1, PS1 and PR1 don't depend on them, so they can launch while the user answers. AU1 doesn't either, but it waits for phase 6's J1-4 commit; see collision 2 above. ST1, SE1 and E2 launch once their question is answered; write each answer into its brief and into D51 as an addendum.
  - Otherwise, launch them together in one Workflow.
  - T1 forks from SE1's first commit, and PR2 from PR1's; launch each as soon as that commit exists.
  - Cold builds go through phase 3's slot helper, which the preamble names, so at most three run at once across both phases.
- **M7-1:**
  - Join on `claude/p7-w1`. If the precheck script (`tools/derive_once_precheck.ps1`, from merge M0) has reached main, run it; otherwise the reviewer does that step by hand.
  - Then the Sonnet derive-once review, then fix rounds until it's CLEAN.
  - E1 and E2 change stored results. Before merging, measure corpus scores and tied or multiplier-squeeze counts against main, and show the user the counts and why, in game terms.
  - Every other task must leave corpus scores byte-identical.
  - The phase 3 scripts `scores.ps1` and `wave_c_tests.ps1` sit in phase 3's scratchpad beside `cold_build.ps1` and can be reused.
- **Wave 2** (E3, ST2, PV, SE2, TM): after phase 3's M_D and M7-1. Write the briefs first with Fable writers, the same way. E3 may change all-0 lists, and the user sees those counts before M7-2.
- **Wave 3:** RP and DOC in parallel; LB forks from RP's commit. M7-3 adds one `hydra_uitest` run by the main session.
- **Wave 4:** ER alone. It goes after M7-3 and after all of phase 6 has landed.

## Brief writer rules (what each brief must satisfy)

- No code listings: name the owners, files, test cases and pinned literals.
- Plain English.
- The owned files are the plan row's files, plus their tests.
- Pinned numbers come only from D51, the plan's Test limits, or values today's code prints.
- Display text uses D51's exact wording.
- Test filters must exist today.
- Each brief stays under about 120 lines, in the format of `tasks/C4c.md`.

## Loose ends from earlier handoffs, still open

- Three empty folders under `.claude/worktrees` (`admiring-fermi-4a140e`, `bold-wilson-6af6e3`, `objective-maxwell-56ed0c`). Delete them once nothing holds them open.
- Unmerged prototype branches (`claude/s2-proto-*`, `claude/s1-flake`). Ask the user before deleting them.
- `docs/handoffs/2026-09-29-public-timing-scripts/blink/results/blink-both.json` is untracked on purpose.
- The hook backup files (`*.bak-d46*`) in `C:\Users\Patrick\.claude\hooks`.
- Nothing from phase 7 is pushed. Ask the user before pushing.
