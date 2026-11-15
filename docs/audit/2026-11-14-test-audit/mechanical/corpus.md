# T6: change the corpus and watch what turns red (session 7a62df56, 2026-11-15)

This is the first mechanical check in the approved plan (`docs/superpowers/plans/2026-11-14-test-suite-audit.md`, "T6"). It changes which charts sit in `testdata/input` and runs the whole suite each time, to find tests that lean on which charts happen to be in the corpus. The user asked for it on 2026-11-15: "start the t8 plan now, run t6 and t7 in parallel, incorporate your findings into the plan when they finish".

## How it ran

The main session cut a throwaway worktree from main at 5d88062e279407e44ebd31f542b7e4793e4870d4 (branch `claude/test-audit-t6`, never merged) and built `hydra_tests` there through a build pass. The tests find the corpus by a path compiled from that worktree's own source folder (`HYDRA_INPUT_DIR` in `CMakeLists.txt`), so changing the worktree's `testdata/input` changes what the tests read, with no rebuild. Each run was the full suite, run by the main session, with doctest's JUnit reporter.

1. **Baseline**, corpus unchanged: 2,280 test cases, none failing.
2. **One chart added**: `testdata/input/test_pitch_dark_ratio` copied whole to a new folder, `testdata/input/t6_added_copy`. 3 test cases failed.
3. **One chart removed**: the copy deleted, then `testdata/input/test_path_reduction` removed. 1 test case failed.

The removed folder was picked as the plan says: the main session grepped `tests/` for each top-level corpus folder's name and its chart file names. `test_path_reduction` (one file, `test0.chart`) was the only one with no hit. Only the six top-level folders were tried, not their subfolders.

Every run, the baseline too, printed "Hydra couldn't read its database (hydra.db). Check that no other copy of Hydra is running, then try again." on its output. It is not tied to the change.

## What turned red

### With one chart added

| Test | File | Failure line |
|---|---|---|
| discover_charts output matches the checked-in scan snapshot | `tests/test_analysis.cpp` | line 100: `CHECK( items.size() == snapshot.size() )`, 99 == 98 |
| the scan's folder count is reported while the walk runs, from the calling thread | `tests/test_analysis.cpp` | line 1453: `CHECK( seen.back() == 121 )`, 122 == 121 |
| corpus: squeeze-ins at exactly 0 and what they change | `tests/test_engine_sweeps.cpp` | line 656: `CHECK( charts == 98 )`, 99 == 98; line 660: `saved_rows == 374`, got 379; line 664: `saved_negative == 90`, got 94; line 665: `still_empty_with_sqin == 79`, got 83; line 666: `allzero_charts == 69`, got 70; line 667: `allzero_paths == 70`, got 71 |

### With one chart removed

| Test | File | Failure line |
|---|---|---|
| the scan's folder count is reported while the walk runs, from the calling thread | `tests/test_analysis.cpp` | line 1453: `CHECK( seen.back() == 121 )`, 120 == 121 |

The scan-snapshot test and the squeeze-in sweep stayed green when `test_path_reduction` went. So the folder is counted among the 121 folders, but its chart is not among the 98 charts those two tests count. Why the scan skips `test0.chart` was not looked into here.

## What this means for the audit

Each test above is a finding: it leans on which charts are in the corpus. Three distinct tests went red in all; the folder-count test went red in both steps. Each needs its record matched and an exit in T8.

Each red test already has a record, and each record already judged the pinned number a count of the corpus rather than a rule. The run now confirms that claim, so these records move from "reasoned" to "confirmed" on this point:

- "discover_charts output matches the checked-in scan snapshot" is `records/A4.md` record 3, verdict Rewrite. Its question 3 says the snapshot "follows the corpus, not a rule".
- "the scan's folder count is reported while the walk runs, from the calling thread" is `records/A4.md` record 45, verdict Rewrite. It says "the 121 is a count of the corpus, not a rule".
- "corpus: squeeze-ins at exactly 0 and what they change" is `records/A13.md` record 10 and `records/delta-DL5.md` record 18, verdict Rewrite (DL5, "Agree").

A test that stayed green is not proven independent of the corpus. These two changes add a copy of one chart and remove one folder. A test that reads one named chart, or that counts something the copy or the removed folder does not change, can still lean on the corpus and stay green.

The raw JUnit files were scratch files and are not kept. The counts above were read from them.
