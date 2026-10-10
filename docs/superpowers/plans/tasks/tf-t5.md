Read docs/superpowers/plans/tasks/_tf-preamble.md first; it holds the rules.

# Task TF-T5: the engine digest at the settings that change the engine

Task id: `tf-t5`. Worktree `.claude\worktrees\tf-t5`; make it. Your spec is the plan's "Task 5".

The user approved three more engine-digest pins: Hard with no Pro and no 2x kick; Expert Pro 2x with the CH 1.0 fill rule (`legacy_fills`); and Expert Pro 2x with Note Shuffle on and the ms limit off. Everything else matches the existing pin. Find how each setting reaches the engine (the `AnalysisSettings` fields and how `batch_run` passes them) before you write them; don't guess field names.

Also count the charts that analysed inside `engine_digest` and `CHECK(analyzed == <corpus size>)` beside every pin, the existing one included. Read the corpus size from the same place the digest reads its chart list, never a typed count.

Each new digest literal is pinned from one run on your base, with a comment saying so. The plan expects about 0.1 s per pin; report the measured time per pin.

## Owned files

`tests/test_perf_digest.cpp` only.

## Failing first

Change one new literal's last digit, show the red message, restore.

## Tests

`-sf=*test_perf_digest*`.
