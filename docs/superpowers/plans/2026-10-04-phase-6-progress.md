# Phase 6 progress (session 4f777343)

This is the running ledger for phase 6. The plan is `2026-10-04-phase-6.md`, and the decisions are D53 and D54.

## Wave J1

The user answered "use recommended answers to everything" on 2026-10-04. D53 and D54 were recorded in b1d596e, and the J1 briefs and the phase 6 preamble in 3eac153.

Phase 7's wave 1 launched first, so the agreed swap applies. J1-4 forked from AU1's commit 9a53477, because AU1 also edits `tests/test_stream_mix.cpp`. J1-1 waits for T1's own commit on `claude/p7-t1`, because T1 also edits `tests/test_replay.cpp`. J1-2, J1-3 and J1-5 forked from 3eac153, and J1-6 forks from J1-5's commit.

Before the fork, phase 7's nine wave 1 branches were diffed against J1's owned files. The only shared file was `tests/test_stream_mix.cpp`, which the swap handles. Re-run that check before merging M6-J1.

The workflow is wf_9effe137-183. Each task gets an Opus implementer, then a Sonnet answer check, then one Opus fix round and a recheck if the check finds something.

### M6-J1a merged (7a4e59e)

J1-2, J1-3, J1-5 and J1-6 merged to main as 7a4e59e. J1-1 and J1-4 wait, because they sit on phase 7's T1 and AU1 commits, which reach main with M7-1. The first derive-once review (key f937a5b) found four small things. There was a folder-and-name join written twice, so `join_folder` now lives in winstr. A known-copy row named no task. One test recomputed the .srb offset. ADR 0008 named `kSpanEndTicks`' old file. One sweep round of two Opus agents fixed all four, and round 2 (key 4b4fba0) came back CLEAN. The full suite passed 876 of 876 and the GUI suite 62 of 62. Corpus scores on all 97 charts were byte-identical to main at b555828.

### Still to do in J1

J1-1 is at 0370712 on `claude/p6-j1-1`. Its answer check found one stray tick, 1536, in its new fixtures case; move that chord to an allowed tick. Its scan rows sit in a temporary block that `rules()` does not read yet, so move them in at the join. J1-4 is at c87899b on `claude/p6-j1-4`, and its answer check was CLEAN. Both merge as M6-J1b after M7-1. The steps: merge main into a fresh `claude/p6-j1b`, merge both tasks, fix the J1-1 leftovers, run the precheck, get a review, run one sweep round, run the full suite once, compare the scores, then merge.

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
