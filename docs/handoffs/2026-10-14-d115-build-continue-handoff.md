# Handoff: continue building D115 (one record per chart and profile)

Written 2026-10-14 by session c0e1259d-b09f-41ee-b7ed-dd7ce0f2ff5e, when the user said "stop and create a handoff". It replaces `docs/handoffs/2026-10-12-one-record-per-chart-build-handoff.md` as the place to start. The plan is still `docs/superpowers/plans/2026-10-12-one-record-per-chart.md`, with its part files in `docs/superpowers/plans/2026-10-12-one-record-per-chart/`. The plan wins over the parts, and this file adds the decisions made since.

## Where things stand

Main is at f3629d74. The full suite passes there: 1433 of 1433 test cases, counted by doctest after a double build. Nothing is pushed. No agent is running: every agent this session started has handed back its report.

Merged to main so far, in order: B1-5, B1-2, B1-3, B1-1, A1-0, A2-5, A2-1, B1-4, A1-4, A1-1 (332fc1bb), a docs-match fix (8484998c), A1-2 (5b5dea8e), F1 (e9c272cf), A1-6a (3e921582), an ADR 0026 wording fix (55d69ed7), A1-6b (a0e34bf4), A1-6c with A1-6d (a0e4ea84), and the whole timing branch (f3629d74: B2-1 to B2-4, TJ-1 to TJ-4). Every one had its one fresh reviewer and passed the derive-once gate. The video-tools repo has one commit, 4e95395: `replay.py`'s `measure_text` now prints hydra_replay's label, such as "m3.4.0", where it printed "3.4.0".

Still to build, as the plan lists them: A2-3, then A2-4, then A2-2, then A1-5 (waves 4 to 7); B1-6, B1-7, B1-8 one at a time (wave 8); D1, every document (wave 9); then the real-app check and the push (wave 10). A1-5 must leave `tests/test_perf_digest.cpp` unchanged, as the plan says; its six digests were re-pinned at the timing merge and are now the baseline.

## What happened this session that the plan does not say

The user asked, mid-build: "make sure the question of 'what is an old version and how do i handle it' is only derived once everywhere in the project. i am starting to sense some drift here". A scout found the question answered in many places. The user approved a plan to give each part one owner, and these tasks built it. They are not in the plan file.

- F1: `legacy_fills_for` sits beside `fill_rule_for` in `src/search/graph.h`, and every flag-to-rule mapping goes through the pair, with a scan row. `AnalysisSettings::legacy_fill_deadline` is now `legacy_fills`. Retired settings keys live in one list per reader (`kRetiredSettingsKeys` in `src/app/config.h`, `kRetiredRulesKeys` in `src/app/rules_file.cpp`), each with the release that retired it.
- A1-6a: `src/store/library_file.h/.cpp` holds `classify_library_file` (Fresh, Current, Convertible, TooOld, Unreadable) and `kOldestConvertibleVersion = "2.2.0"`. `OldFilePolicy { SetAside, Refuse }` is named by every shipped open. `set_aside_old_library` in `src/store/upgrade_files.cpp` is the only thing that renames a library file. The copy upgrade, upgrade_results_key, the Auto cleanup and `retired_auto` are deleted. Scan rows enforce all three owners.
- A1-6b: the app shows the set-aside notice once in an information box, then starts a scan. `AppState::startup_notice()` and `startup_notice_closed()` are the seam; the GUI test is `old-library-set-aside`.
- A1-6c and A1-6d: the file-level fill-rule label (`engine_mode`) is gone, along with hydra_batch's refusal and hydra_fillcompare's warning. A conversion deletes the old meta row, and a scan row flags any new reader. hydra_batch may write `--legacy-fills` results into the app's own hydra.db. The command-line refusal of an old file prints one sentence. `set_aside_old_library` keeps an earlier backup until the new move works, and never deletes it after a failed put-back.
- TJ-1 to TJ-4 (timing branch): Clone Hero 1.0 mode keeps main's old fill generator exactly. `Song::check_activations` is the one place that picks `place_fills_ch10` or `place_fills_ch11` from a `FillDeadlineRule`, and the loaders have no default rule (tests use `tests/load_util.h`). The Preview carries the rule in `PreviewSongKey`. A fill whose chord sits before its measure line draws from the earlier tick. `fill_length_measures` is in `kRetiredRulesKeys` as retired in 2.8.0.

The owner list behind all this is in the session scratchpad, `d115-oldversion-owners.md`. It is summarised above because the scratchpad is temporary.

## User decisions made 2026-10-14

These come on top of the ten answers in the plan.

1. A time signature whose beat is under one tick is refused with Hydra's existing "time signature at tick N" sentence. That now reads "bottom number that is out of range" for "0 = TS 1 12" at resolution 192.
2. The batch's save-failure detail stays "saving a record failed:".
3. Files older than 2.2.0 are cut off. The app renames hydra.db to hydra.db.pre-2.2.0, replacing an earlier backup, starts a fresh library and rescans. The notice reads: "This library file is from a Hydra version older than 2.2.0, so Hydra set it aside as hydra.db.pre-2.2.0 and started a fresh library." If the rename fails, the error reads: "This library file is from a Hydra version older than 2.2.0, and Hydra could not set it aside: <reason>. Close any program using hydra.db and start Hydra again."
4. hydra_batch and hydra_fillcompare refuse a pre-2.2.0 file and never rename it. The sentence (proposed by the executor, matching the approved description) is: "The library file '<path>' is from a Hydra version older than 2.2.0, and opening it in Hydra sets it aside and starts a fresh library."
5. Clone Hero 1.0 mode keeps its old fill generator exactly: the old chord choice and the half-measure length.
6. The file-level fill-rule label is retired; one file holds both rules.
7. hydra_batch's refusal of `--legacy-fills` into hydra.db is dropped.
8. Star Power length uses the exact whole-number measure count in both modes, 1.0 included. "Keep 1.0 exactly as before" is about fills only.

## What the score check found

`testdata/input` (115 charts) was run through main's hydra_replay and the timing branch's. In 1.1 mode two charts change. Stone Temple Pilots "Trippin on a Hole in a Paper Heart" gains 1920 (377735 to 379655, path `2 1 0` to `0 0 E2 E0`). Every generated fill now starts on its measure line instead of half a measure earlier, so its deadline is 384 ticks later. Two Star Power phrase ends sit exactly on the new deadlines. TEXTURES "Laments Of An Icarus" has its 40 fills moved onto the restarted grid after its mid-measure 6/4 change. In 1.0 mode, no best score, best path or fill chord changes. Star Power ends move 1 or 2 ticks later on 9 of 97 charts, from the exact measure count; a scout traced every one.

## Open items for the next session

Documents for D1 (wave 9), in addition to the plan's list:

- `docs/UserGuide.md` lines ~489 and ~502 still show `fill_length_measures` as a live setting. This is a public page, so it gets the doc review.
- CONTEXT.md needs entries for `FillDeadlineRule`, `fill_rule_for` and `legacy_fills_for`, and an update to its "Library file upgrade" text (lines ~110-118) for the set-aside. Line ~433 still says `--legacy-fills` needs its own database.
- `docs/development.md` lines ~20, ~39-43 and ~53-56 describe the stamp refusal and "needs its own --db".
- ADR 0010 lines ~13-135 describe the guard, the stamp and the refusal; it needs the dated note for the fill-generator change and for the label's retirement. ADR 0014 line ~84 names `retired_auto_fingerprint`. ADR 0026's "The upgrade" section (lines ~95-165) describes the deleted copy-and-swap; a short ADR or amendment should say files older than 2.2.0 are set aside.
- Also in the docs: `README.md` line ~55 and the UserGuide's update section (lines ~17-35) describe the upgrade screens.

Smaller follow-ups:

- `tests/test_store_records.cpp` line ~166 calls `store.counts()`, which A1-5 deletes; A1-5 repoints it to `record_count`.
- The PurgeScope split in `file_record` (EveryMode for `save`, ThisMode for `write_row`) is interim. A1-5 deletes it together with `write_row`.
- `ready_records`' tie order (records with no paths last, then by song id) was the implementer's choice. A2-2 re-sorts anyway.
- `tools/bench.cpp` `replay_mode` now loads with `g_rules`, a small dev-tool change.
- The path to the `v2.1.0-schema4.db` fixture is spelled in four tests. A one-line helper in `tests/testdata_dirs.h` would give it one owner.
- `testdata/input/.../test_folded_sqin/twin_end_nodes.chart` is no longer read by any test; the twin-end tests read `testdata/squeeze_clamp/twin_end_clamped.chart`.
- A Current file can keep a stray `engine_mode` meta row from an unreleased build. Nothing reads it.

Promise to another session: the column-shares session ("Column shares scroll left align build", pipe `cc-msg-e59368f7ee2bff4184684a1097b22866`) wants the merge hash when A2-2 lands on main and again when B1-7 does. It was building CS-T4 (report windows) and CS-T5b (score breakdown) from main; CS-T5b and CS-SEL are merged. B1-6 and B1-7 rename `path.score_base` and kin, which CS-T5b's `build_score_breakdown` in `src/app/path_view.cpp` still reads.

A separate session is fixing the build that drops new test files (the suggested task "Fix build dropping new test files"). Until it lands, build main twice after a merge that adds a test file, and check the test-case count rises.

## How this session ran things, for whoever continues

Each task got one Opus executor in its own worktree, briefed with the shared page `d115-exec-common.md`. Each change got one Sonnet reviewer, briefed with `d115-review-common.md`. Those two pages and `d115-orchestrator-notes.md` live in the session scratchpad (`C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\c0e1259d-b09f-41ee-b7ed-dd7ce0f2ff5e\scratchpad\`), which is temporary. Copy them first if they still exist. The review files are in `C:\Users\Patrick\AppData\Local\Temp\claude\d115-review\`.

Review keys come from `pwsh -NoProfile -File C:/Users/Patrick/.claude/hooks/main_move_check.ps1 <branch> -NoFF`. A merge on main needs the trailers Task, Agent and Session plus Co-Authored-By, and `git -C` with the folder written out in full. The gate asks for a short resolution review of any merge of main into a branch that was resolved by hand.

Old worktrees under `.claude/worktrees/d115-*` (24 of them) and the base worktree `C:\Users\Patrick\AppData\Local\Temp\claude\d115sc-base` are all merged or done with. The cleanup sweep removes merged idle worktrees after 24 hours; `d115sc-base` holds main's scoring at 70d82797 and is useful only for another before-and-after score check.
