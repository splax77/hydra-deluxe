# RapidCheck trial: the planted-bug proof (ct-t7b)

This is the proof step of task 7 in `docs/superpowers/plans/2026-10-10-ci-test-tooling.md`. A one-character bug went into `replay_path` in `src/core/replay.cpp`. Both window-order cases ran against it, then the bug came out again. Nothing under `src/` is committed. The two cases are the old one, "replay: window order does not matter and SP points add up window by window", and the new one, "replay: window order does not matter and windows add up, shrunk by RapidCheck". Both sit in `tests/test_replay.cpp`.

The command each time was `build-cpp\Release\hydra_tests.exe "-tc=replay: window order does not matter*"`, on the warm build in the ct-t7 worktree at commit 1a986eb4.

## First plant: caught by neither case

The first plant touched only the squeeze-out price. In the `core::paid_by_sp` call, `sg.sqout_sp` became `~sg.sqout_sp` (one added character). A squeezed-out chord that should keep part of its doubling then reads as unpaid. Its window pays it nothing and leaves.

Both cases passed: 2 test cases, 23 assertions, all green. The reason is what the cases check. They check that window order does not matter, that the xN disc agrees with the points, and that a row's SP points are the sum of what each window pays alone. A bug that changes what one window pays, and changes it the same way in every replay, keeps all three true. So these cases cannot see a wrong squeeze-out price. Catching one is the job of tests that compare against fixed numbers, such as pinned rows or "replay reproduces the engine's score for every corpus path". This trial did not check whether those catch this plant.

## Second plant: caught by both

The second plant was on the `core::backend_row_value` line. `sp_points += ...` became `sp_points = ...` (one deleted character). When two windows pay one chord, the chord now gets only the last window's points instead of the sum.

The old case's report, trimmed (53 lines, 3,600 bytes in full). The same two failures repeat for each leeway variant; whole-chord squeeze-out shows only the second.

```
tests\test_replay.cpp(1837): ERROR: CHECK( first_result_difference(r, replay_path(*longest, reversed, rules)).empty() ) is NOT correct!
  logged: chart: ...\Pathfinder - When The Sunrise Breaks The Darkness\notes.mid (3632 chords)
          leeway 3 ms

tests\test_replay.cpp(1621): ERROR: CHECK( bad.count == 0 ) is NOT correct!
  values: CHECK( 1097 == 0 )
          leeway 3 ms: first row that does not add up: 186
...
(10 failed checks in all: 1097 bad rows at leeway 3, 0 and -5 ms, 1184 at 250 ms, 1093 under whole-chord squeeze-out; first bad row 186 every time)
```

RapidCheck's report, trimmed (26 lines, 881 bytes in full):

```
Falsifiable after 9 tests and 122 shrinks

unsigned long long:
0

std::vector<hydra::ReplayWindow>:
[{act 11520, deact 11641}, {act 11520, deact 11640, sqout 11520}]

tests\test_replay.cpp:1899:
RC_ASSERT(first_result_difference(r, replay_path(song, reversed, rules)) == "")

Expands to:
"row 0: points" == ""

Log:
leeway 3 ms
```

The `0` is the leeway variant's index, and the log line names it. The old case ran over 2,000 windows. RapidCheck's answer is two windows that both start on the chart's first chord (tick 11520), so both pay it. One pays it in full; the other squeezes it out on that chord. With `=`, whichever window comes last wins. The sort by activation tick does not fix the order of two windows that start together, so reversing the list changes the row. That points straight at the line.

The old case did not print a wall of rows, as the plan expected. It prints a count and the first bad row. But "row 186 of 2,000 windows" still leaves you to find which windows meet there, and RapidCheck hands you the pair.

## Did shrinking earn its keep?

Yes, for the bug it can see: two windows and one row beat 1,097 bad rows across 2,000 windows.
The shrunk pair also showed a cause the old report hides: two windows starting on one chord.
The cost was small: both cases together took 4.2 s failing and 1.6 s passing, and the seed stays fixed.
It shares the old case's blind spot: a wrong squeeze-out price with consistent windows passed both.
So it is worth keeping beside the old case, but it does not replace pinned rows or the engine-agreement test.
