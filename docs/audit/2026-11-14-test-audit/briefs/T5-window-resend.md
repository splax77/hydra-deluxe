# Brief: T5, window round re-sends WN4a, WN4b, WN4c and WN6, with their verifiers

Your first brief is `briefs/T5-window.md`. Read it and follow it: its reason, its kinds A, B and C, its record shape, its rules, its caps, its commit rule and its report. This page changes only your files and adds the points below. Task ids: T5-WN4a, T5-WN4b, T5-WN4c, T5-WN6 for an auditor, T5-V-WN4a and so on for a verifier. Session id: e633f7f6-f313-4787-b622-8d901b577b64.

## Why a re-send

The WN4 auditor (`records/window-WN4.md`, commit 7c07da7a) hit its caps on 470 test cases. It sorted them by a text scan, read few bodies, and wrote 45 short C records that do not answer the twelve questions, name no bug at a line, and give a hedged verdict ("Investigate unless the old verdict was Rewrite or Remove"), which is not a verdict. Its verifier (`verify/window-WN4.md`, commit 1c7c6c0c) moved 13 B rows to C, moved the A row `test_activation_rows.cpp:1186` to B, and found typed numbers the scan missed. Read both files first. Your job is to redo WN4's work for your files properly: read every test body that the WN4 file sorted, confirm or change its kind, and write every C as a full record with one verdict. Use `coverage.md` to find each old record; the WN4 verifier names where several are (pb_coach in `records/C7.md`, pb_text in `A19.md`, preview_scene in `A20.md`, pathstring_read in `B2.md`, app_click in `A5.md`, saved_figure_fill in `A27.md`).

The WN5 verifier (`verify/window-WN5.md`) found test files in no group. WN6 takes them.

## The groups

| Group | Files | Writes |
|---|---|---|
| WN4a | `tests/test_replay.cpp`, `test_replay_schedule.cpp`, `test_replay_windows.cpp`, `test_pathstring_read.cpp`, `test_preview_scene.cpp`, `test_preview_controller.cpp` | `records/window-WN4a.md` |
| WN4b | `tests/test_pb_coach.cpp`, `test_pb_text.cpp`, `test_saved_figure_fill.cpp`, and the helper `tests/dm_fixture.h` | `records/window-WN4b.md` |
| WN4c | `tests/test_activation_rows.cpp`, `test_app_click.cpp`, `test_app_reports.cpp`, `test_cli.cpp`, `test_analysis.cpp`, `test_report_rows.cpp`, `test_song_parse.cpp`, `test_preview_golden.cpp`, `test_perf_digest.cpp`, and the helpers `tests/app_state_fixtures.h`, `song_digest.h`, `song_details_fixtures.h` | `records/window-WN4c.md` |
| WN6 | `tests/ui/uitest_batch_reports.cpp`, `tests/ui/uitest_tests.cpp` (including `register_script_test` in `tests/ui/uitest_script.cpp:57`), `tests/pb_ui/pbtest_build.cpp`, `pbtest_coach.cpp`, `pbtest_files.cpp`, `pbtest_window.cpp`, and the typed fill ticks `kFirstFillTick` and `kSecondFillTick` in `tests/pb_ui/pbtest_harness.h:60-61` | `records/window-WN6.md` |

A verifier writes `verify/window-<group>.md` and does "The verifier's job" of the first brief for its group's file.

## Points that bind this re-send

- Read every test body you sort. A row sorted by a token scan alone is not done. If you hit the warning cap, commit, and list every test case whose body you did not read, by exact name; do not sort it by scan.
- Every C record has one verdict: Keep, Rewrite, Remove or Investigate. "Investigate unless" or "Keep under" is not a verdict.
- Every C record answers questions 1 to 11 and names question 8's bug at a line, as `docs/agents/test-rubric.md` shows.
- List every typed number on an assertion line that a run at 60 ms can move, with its line. The WN4 verifier's list is a start, not the whole.
- Write status lines with the preamble's one call, from the PowerShell tool. If it is refused, say so in your report with the error text; do not write the status file another way.
