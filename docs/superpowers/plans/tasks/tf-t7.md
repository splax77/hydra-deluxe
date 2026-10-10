Read docs/superpowers/plans/tasks/_tf-preamble.md first; it holds the rules.

# Task TF-T7: the real-device test proves the device played

Task id: `tf-t7`. Worktree `.claude\worktrees\tf-t7`; make it. Your spec is the plan's "Task 7", option A (the user's choice): keep the test and strengthen it.

`PreviewAudioDevice` gains `bool started() const` (a read of the existing private flag; no behaviour change). The test asserts it after `start()`. Its source callback counts calls in an atomic, and the test waits for the count to pass zero, with a **2-second cap** (the user's number; it's a hang detector, not a timing test). On the cap it fails with a message saying the device thread never pulled audio. The old `set_headless` value is put back by a guard.

The shared capped-wait helper (`tests/wait_util.h`) lives on an unmerged branch, so your base doesn't have it. Write the wait as a short deadline loop in the test with a comment saying the merge switches it to `wait_util.h` once that lands. Name the loop's location in your report.

Don't change what `start()` does when `ma_device_start` fails; whether the user should see that failure is an open question for the user.

## Owned files

`tests/test_audio_device.cpp`, `src/audio/device.h`, `src/audio/device.cpp` (the accessor only, if it isn't inline).

## Failing first

Assert `started()` before calling `start()`; show red; restore.

## Tests

`-sf=*test_audio_device*` only. This case needs a real sound card; the machine has one. If it fails for a hardware reason, report the exact output and stop.
