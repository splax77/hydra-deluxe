Read docs/superpowers/plans/tasks/_tf-preamble.md first; it holds the rules.

# Task TF-T3: stems through the real functions

Task id: `tf-t3`. Worktree `.claude\worktrees\tf-t3`; make it. Your spec is the plan's "Task 3", all four cases plus the deletion of the copy-loop case. Byte sizes and frame counts are pinned from one run (`Get-Item` for file sizes), with a comment saying which file each number belongs to. The 12,000-frame figure is already pinned by the load-job test; reuse its meaning, but check the number in that test before you rely on it.

The fixture audio is the five `testdata/audio/sine220.*` files plus junk bytes typed in the test. No new binary fixtures. Build the case-four chart folder with the test's existing `audiochart` helpers (find them with Grep first).

`tests/test_preview_load_progress.cpp` is also edited by the unmerged wait-cap branch, so put the load-job case in your new file, not there.

## Owned files

`tests/test_song_audio.cpp` (new), `tests/test_audio_mixer.cpp` (delete the copy-loop case only), `CMakeLists.txt` (one line adding the new file). If `tests/test_single_owner.cpp` must change because a scan row names the deleted case, stop and report the row instead.

## Failing first

Case two red by making the junk stem a `REQUIRE`d third reader; keep the line, restore.

## Tests

`-sf=*test_song_audio*` and `-sf=*test_audio_mixer*`.
