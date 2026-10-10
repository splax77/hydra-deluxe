Read docs/superpowers/plans/tasks/_tf-preamble.md first; its rules apply, with these changes. Your base is the current `main`, not 013f4c66: make your worktree with `pwsh -NoProfile -File C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\new_worktree.ps1 -TaskId tf-leftovers`. Task id `tf-leftovers`. The user asked for these three on 2026-10-10 ("do the leftovers with an agent"); they are listed at the end of `docs/handoffs/2026-10-10-test-fidelity-fixes-handoff.md`. Commit each one separately.

## 1. The "stars:7" library query matches nothing

`tests/test_library_model.cpp` has a case that pins the row count of nine queries over 20,000 synthetic charts. "stars:7" pins 0, because the synthetic data only gives a summary to rows where `i % 3 == 0`, and those rows get `5 + i % 3` stars, which is always 5. So that query proves no more than "zzqx" does.

Fix the synthetic data so the stars values actually vary across the rows that have a summary (5, 6 and 7 should all occur), keeping everything else about the data the same. Then repin every query count in that case from one run, with the comment saying when and at which commit. Check whether any other case reads the same synthetic data and repin it too if its numbers move; report each count that changed, old and new. Don't change the query list. Failing first: show the old 0 going red against the new data before you repin.

## 2. Two copies of the results row count

`results_rows_in` in `tests/test_store.cpp` and `results_rows` in `tests/test_app_state.cpp` both wrap `SELECT COUNT(*) FROM results`, and the same query is typed inline in `tests/test_store.cpp` (near lines 848 and 904 on the reviewer's read). Grep `tests/` for every other copy. Keep one helper in a shared test header that both files already include, or the most fitting existing one (don't make a new header if one fits), and have every copy call it. If `tests/test_single_owner.cpp`'s clone scan or a scan row should guard it, say which and add it only if a row fits the scan's existing pattern.

## 3. `tools/build_slot.ps1` builds from the caller's folder

It runs `<Repo>\build_cpp.ps1` without changing directory. Older checkouts' `build_cpp.ps1` run `cmake --preset` against the current folder, so pointed at an old release from the main checkout it rebuilt the main checkout instead. Make it run the build from inside `-Repo` (Push-Location before, Pop-Location in the `finally`). Find out whether a test covers this script (grep `tools/` and `tests/` and the CI workflow for `build_slot`); if one exists, add a case that would have caught this, and run that test only. Don't run any real build through the slot for this beyond what the test does. In the script's header, add one sentence that a very long `-Repo` path can break MSBuild's 260-character limit (seen 2026-10-10 under the scratchpad), so old-release worktrees belong on a short path. Don't add code for that.

## Owned files

`tests/test_library_model.cpp` (and any other test file that reads the same synthetic data, if its counts move), `tests/test_store.cpp`, `tests/test_app_state.cpp`, the shared test header you pick, `tools/build_slot.ps1`, its test file if one exists, `tests/test_single_owner.cpp` only for a row item 2 needs.

## Tests

`-sf=*test_library_model*` (and any other file whose counts you repin), `-sf=*test_store*`, `-sf=*test_app_state*`, `-tc="single-owner*"`, and the build_slot test if one exists.
