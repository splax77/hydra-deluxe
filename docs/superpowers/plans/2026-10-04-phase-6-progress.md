# Phase 6 progress (session 4f777343)

This is the running ledger for phase 6. The plan is `2026-10-04-phase-6.md`, and the decisions are D53 and D54.

## Wave J1

The user answered "use recommended answers to everything" on 2026-10-04. D53 and D54 were recorded in b1d596e, and the J1 briefs and the phase 6 preamble in 3eac153.

Phase 7's wave 1 launched first, so the agreed swap applies. J1-4 forked from AU1's commit 9a53477, because AU1 also edits `tests/test_stream_mix.cpp`. J1-1 waits for T1's own commit on `claude/p7-t1`, because T1 also edits `tests/test_replay.cpp`. J1-2, J1-3 and J1-5 forked from 3eac153, and J1-6 forks from J1-5's commit.

Before the fork, phase 7's nine wave 1 branches were diffed against J1's owned files. The only shared file was `tests/test_stream_mix.cpp`, which the swap handles. Re-run that check before merging M6-J1.

The workflow is wf_9effe137-183. Each task gets an Opus implementer, then a Sonnet answer check, then one Opus fix round and a recheck if the check finds something.

### M6-J1a merged (7a4e59e)

J1-2, J1-3, J1-5 and J1-6 merged to main as 7a4e59e. J1-1 and J1-4 wait, because they sit on phase 7's T1 and AU1 commits, which reach main with M7-1. The first derive-once review (key f937a5b) found four small things. There was a folder-and-name join written twice, so `join_folder` now lives in winstr. A known-copy row named no task. One test recomputed the .srb offset. ADR 0008 named `kSpanEndTicks`' old file. One sweep round of two Opus agents fixed all four, and round 2 (key 4b4fba0) came back CLEAN. The full suite passed 876 of 876 and the GUI suite 62 of 62. Corpus scores on all 97 charts were byte-identical to main at b555828.

### M6-J1b merged (8fe0356): J1-1

M7-1 merged first, as 6c65208. J1-1 then merged alone as 8fe0356. At the join, its stray 1536 tick moved to 192, and its temporary scan-row block went into `rules()` and `known_copies()`. The precheck found 0 items, and the review came back CLEAN on the first round (key 9f8bbea). The full suite passed 904 of 904 and the GUI suite 62 of 62. Corpus scores matched main at 5a95e8f, which already includes M7-1's intended score changes, on all 97 charts. The replay self-check passed 382 of 382.

### Still to do in J1: J1-4 (M6-J1c, after M_D)

J1-4 is at c87899b on `claude/p6-j1-4`, and its answer check was CLEAN. Phase 3's FX-P (inside M_D) also adds `src/audio/frames.h` with `frames_of_ms` and `ms_of_frames`, and it also edits `player.cpp` and `test_audio_player.cpp`. Agreed with the phase 3 session: M_D goes first. Then J1-4 merges main in, takes phase 3's `frames.h` (with its rate guard) and its `player.cpp` changes as they are, and keeps only J1-4's other folds: `stem_converter_config`, `tests/audio_util.h` and the mixer oracle pin. After that come the precheck, a review, the full suite and the scores, then the merge as M6-J1c.

## Wave J2

Phase 7 launched its wave 2 from phase 3's join branch, `claude/p3-d2` at 8b5a99d, without waiting for M_D. That commit already holds M7-1, M6-J1a, J1-1 and M0, so phase 6 did the same. The workflow is wf_82ef1b6e-5e5. J2-1, J2-2, J2-7 and J2-8 fork from 8b5a99d. J2-3 and J2-5 fork from J2-1's commit, because they need strutil's new exports. J2-6 forks from J2-1 and merges the J1-4 port, which brings `tests/audio_util.h`. The J1-4 port is `claude/p6-j1-4c`, on 8b5a99d, and it takes phase 3's `frames.h` and `player.cpp`. M6-J2 cannot merge before M_D does, and it merges main in first.

J2-4 forks from phase 7's SE2 commit. SE2 started first, so the agreed `test_app_state.cpp` order is swapped, and J2-4 waits for that hash.

Phase 7's PV takes J3-5's transport gain-floor line early, so J3-5 drops it. PV's volume slider reads `Settings::clamp` without editing `config.cpp`, which stays J2-1's.

## Phase 6 finished (session 9ce5e1bb, 2026-10-05)

Every wave is on main, and nothing is pushed. Each merge had a CLEAN derive-once review, one full suite run and one GUI suite run. Corpus scores matched on all 97 charts every time.

- **M6-J1c** merged J1-4 once phase 3's M_D was in.
- **M6-J2 (a1d4c28)** merged J2-1, J2-2, J2-3, J2-5, J2-6, J2-7 and J2-8 as one join. **M6-J2b (1967e93)** merged J2-4, which waited for phase 7's SE2. D64 kept the leaderboard's "+N over".
- **The GUI suite crash (a39f3c7).** `hydra_uitest --all` died about half the time with an access violation. The settings bar asked "is a batch running?" twice in one frame. When the batch finished between the two reads, it read the song title through an analysis job that was already gone. `AppState::settings_lock()` now answers once. The runner also prints each result as its test ends, and names a test that crashes.
- **M6-J3 (e9e2eeb)** merged J3-1 to J3-8, plus the join leftovers J3-9 and J3-9d. J3-9d puts one owner for reading and writing little-endian numbers in `src/core/little_endian.h`. D68 is the one visible change: the Preview's overlay check compares the whole path key.
- **M6-J4 (e4833e0)** merged J4-1 to J4-7. J4-7 folded the remaining test scratch paths onto `tests/temp_util.h`, which J4-6 added. The join also gave `ScopedFile` one home and wrote CONTEXT.md's sentences for `kDefaultDepthValue` and `kSpActivationBars` (D54).
- **AL2 (2c4b63e)** is phase 7's audio song length (D69 and D70). Phase 7 handed it over for phase 6 to merge after M6-J4.

Still open:
- J2-7's analysis timing check needs a quiet machine.
- A corrupted structure blob shorter than 12 bytes now reads Stale with "another build" instead of "other rules". No writer can produce one, so it stays as it is unless the user says otherwise.
- The old phase 6 worktrees are still on disk. They come out with the user's yes.

## Notes for later waves (from the J1 brief writers)

- **J2-8 and J3-4:** J1-3 exports the "is a toggle on after this instant" helper as `toggle_on_after`, not `toggle_on`. The plan's name would clash with the private `toggle_on(Toggle)` in `highway_draw.cpp`. J2-8 adopts the new name.
- **J3-4:** four `flag_sp` assignments in `tests/test_preview_view.cpp` are known copies of the SP-edge rule. No task names them yet; J3-4 owns that file in J3 and should fold them.
- **J2-1:** once J1-5 lands, the "cannot open MIDI file: " prefix in `user_messages.cpp` is dead, and so is its hand-built test. J2-1 owns that file.
- **J2-4:** `set_analyzer_for_test` lives in `src/ui/library_jobs.cpp`, so its `std::max(1, ...)` floor goes in J2-4, not J1-6.
- **J2-6** (from M7-1's review, handed over by phase 7): `tests/test_stem_reader.cpp` has eight inline max-abs-difference loops, including the one AU1 added. Fold every one onto `max_diff` in `tests/audio_util.h`, which J1-4 created. J2-6 owns that test file in J2.
- **M6-J1 merge:** phase 7's round 2 fix on `claude/p7-w1` edits one test and adds one case in `tests/test_replay.cpp`. It also adds a count-and-seek helper beside `stem_reader.h`. J1-1 rebuilds its scan rows and fixture edits on top of that when main is merged into the J1 branch.
- **J2-1:** `exe_dir()` plus a fixed name is joined by hand in `config.cpp`, `app_shell.cpp`, `main.cpp` and `icons.cpp`. `app_shell.cpp` also adds a trailing backslash by hand (around line 247). J2-1 points those at `join_folder` in winstr, which M6-J1 added, and widens its scan row, which today only sees a variable named `folder`. `config.cpp`'s parent cut returns "." where `parent_folder` returns "", so J2-1 keeps that edge visible. J2-1 also removes the dead "cannot open MIDI file: " prefix in `user_messages.cpp` and the hand-built `MidiError` in `test_user_messages.cpp`.
- **J2-3:** delete the duplicate "Truncated SRB file." guard in `song.cpp` when the .srb loader switches to `srb_read_metadata`.
- **J2-4:** `library_jobs.cpp` around line 248 copies a library entry back into a `ScanItem` by position, the reverse of `to_library_entry`. No scan row flags it yet. J2-4 folds it, along with the worker floor.
- **J2-5:** the two bare `folder + "\\" + name` joins in `preview_source.cpp` call `join_folder`, and their known-copy rows go.
- **J2-8:** the five `asset_dir + "\\..."` joins in `preview_renderer.cpp` answer the same question as `join_folder`. J2-8 owns that file in J2.
- **J3-5:** the load job's front pad calls `frames_of_ms`, and the transport's `set_gain` drops its own floor. J3-5 also deletes those two known-copy rows.
- **J1-6 deletes** the `analysis.cpp` known-copy rows J1-5 adds to `test_single_owner.cpp`.
- **J1-2:** there is no `hydra_uitest library` script; the GUI check is `hydra_uitest --test scan`.
- **J1-1:** the plan's `-tc="fixtures:*"` filter matched no case on main. J1-1's new case is named so that it matches.
