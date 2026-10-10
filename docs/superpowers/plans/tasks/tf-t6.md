Read docs/superpowers/plans/tasks/_tf-preamble.md first; it holds the rules.

# Task TF-T6: no more wall-clock assertions

Task id: `tf-t6`. Worktree `.claude\worktrees\tf-t6`; make it. Your spec is the plan's "Task 6", option A (the user's choice).

In `tests/test_library_model.cpp` (about line 393) and `tests/test_library_query.cpp` (about line 379), each "under 20 ms" check becomes a count check: pin the row count each query returns against the synthetic rows, and the hit count of the named search, as literals from one run. The timing stays as a `MESSAGE` only. In `tests/test_path_view.cpp`, delete the case that requires cached frames to be ten times faster than rebuilds (about line 718); first confirm the case the plan names (about line 650) really counts builds per input change, and say so in your report. Another branch deletes a different case near line 615 of that file; leave it.

## Owned files

`tests/test_library_model.cpp`, `tests/test_library_query.cpp`, `tests/test_path_view.cpp` (the ratio case only).

## Failing first

A wrong literal in each count check, shown red, restored.

## Tests

`-sf=*test_library_model*`, `-sf=*test_library_query*`, `-sf=*test_path_view*`.
