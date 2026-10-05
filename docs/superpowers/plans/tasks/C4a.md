Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task C4a: one path identity and one song-fraction helper

Task id: C4a. Base: main at dc7e582. Branch: `claude/p3-c4a`. Worktree: `.claude\worktrees\p3-c4a`.

## Goal

Two display facts are each written twice today. "Are these two paths the same path?" is answered by the Paths tab's all-0 dedupe with one rule and by the Preview's overlay key with another. "How far into the song is this moment?" is a clamped division typed out once in the Paths timeline and once in the Preview scrub marks. This task gives each fact one owner function and points both readers at it. Nothing on screen moves: every activation sits at the same fraction, the same paths dedupe, and no score, path or stored record changes.

## Finding 249: two "same path" rules

What the code does today. `build_path_list` in `src/app/path_view.cpp` (the all-0 block near line 431) hides the all-0 section when some listed path has the same score and the same short notation (`pathstring`). `path_overlay_key` in `src/app/preview_view.cpp` (line 658) builds the Preview's cache key from the verbose notation (`pathstring_verbose` with no multiplier squeezes) plus the score. The two rules disagree when only the printed millisecond values differ, and neither looks at ticks. `Path` has no equality operator, which is why both grew their own.

The owner. One `path_identity(const Path&)` beside `Path` in `src/core/model.h`, with its body in `src/core/model.cpp`. It returns a string that names the path by its score and, for every activation the path walks (the same walk `pathstring` uses, so the variant tail is included), that activation's tick and its deact tick. Two paths with the same identity are the same path; the string is opaque, compared and never parsed.

What changes. The dedupe in `build_path_list` compares `path_identity` of the all-0 path against `path_identity` of each listed path. `path_overlay_key` returns `path_identity` of the path (still empty for a null path) instead of building its own string. The comment above `path_overlay_key` about multiplier squeezes goes, since the identity never read them. `PreviewController::open` keeps comparing the key with the SP cap exactly as today; this task does not touch `src/ui`.

## Finding 9, refactor half: one song-fraction helper

What the code does today. `build_activations` in `src/app/path_view.cpp` (line 222) sets each row's `song_fraction` to the activation's ms over `song_length_ms`, clamped to 0..1, and skips it when there is no timing or the length is not positive. `build_scrub_marks` in `src/app/preview_view.cpp` (line 599) does the same clamped division for each Preview activation, returning no marks when the length is not positive. `preview_view.h` line 234 calls `PreviewScene::song_length_ms` "the scrubber's right edge", but the scrubber's right edge is the transport's length (`PreviewTransport::load` takes the larger of the last note and the audio end); that field is the last note's onset only.

The owner. One `song_fraction(double ms, double length_ms)` in `src/app/preview_view.h` (body in `preview_view.cpp`). It gives no value when the length is not positive and otherwise the share of the song, clamped to 0..1. `preview_view.h` already includes `path_view.h`, so `path_view.cpp` includes `preview_view.h` to reach it.

What changes. `build_activations` keeps its timing and length checks and asks the helper for the fraction. `build_scrub_marks` keeps its early return for a non-positive length and asks the helper for each mark. The comment on line 234 is rewritten to say what the field is: the last note's onset, which the SP curve closes on and the Preview's own song end; the scrubber's range is the transport's length. Both callers still pass the lengths they pass today (the Paths tab the last note, the Preview the transport length), so positions do not move. Which end both views should measure against is D48 Q25 and belongs to the wave D Preview task, not here. The ms-to-tick rounding at `path_view.cpp` line 386 also stays; another task owns it.

## Owned files (only these may change)

- `src/core/model.h`
- `src/core/model.cpp`
- `src/app/path_view.cpp`
- `src/app/preview_view.h`
- `src/app/preview_view.cpp`
- `tests/test_path_view.cpp`
- `tests/test_preview_view.cpp`

`git diff --stat dc7e582..HEAD` must list only these. If a test needs `tests/test_model.cpp` or another file, stop and report.

## Tests

Existing cases that must keep passing unchanged:

- `build_path_list: score groups and the all-0 dedupe rule` (test_path_view.cpp). Pins: an all-0 copy of the first path stays hidden; the same path 100 points worse shows with label delta "(-100)".
- `activation timeline: onset over the song's length, and the end measure` (test_path_view.cpp). Pins: 2000 ms of 10,000 ms reads 0.2; end measure "m6"; no timing means no fraction.
- `scrub marks: each activation's onset over the scrubber's length` (test_preview_view.cpp). Pins: 0.2 and 0.8 over 10,000 ms; 1.0 when the length is 4000 ms; no marks at length 0.
- `path_overlay_key: no overlay, the same path, and a changed path` (test_preview_view.cpp). Pins: null gives empty; a copy matches; one more score point differs; one fewer activation differs.

New cases to add, test-first (red, then green):

- `path_identity: same path, rescored path, trimmed path` in test_path_view.cpp. A copy has the same identity; a path with one extra score point differs; a path with its last activation removed differs; two paths that differ only in a deact tick differ. No new numbers beyond those the overlay-key case already uses.
- `build_path_list: the dedupe and the overlay key read one identity` in test_path_view.cpp. For every path of the analyzed record, `path_overlay_key` equals `path_identity`, and the all-0 copy is hidden exactly when its identity matches a listed path's.
- `song_fraction: a clamped share, none without a length` in test_preview_view.cpp. Reuses the scrub-mark numbers: 2000 of 10,000 is 0.2, 8000 of 4000 clamps to 1.0, length 0 gives no value.

Use `analyzed()` and the helpers already in these two test files. Define no new helper; grep `tests/` first.

Filters the implementer may run, on `build-cpp\Release\hydra_tests.exe`:

- `-tc="build_path_list*"`
- `-tc="path_identity*"`
- `-tc="activation timeline*"`
- `-tc="scrub marks*"`
- `-tc="song_fraction*"`
- `-tc="path_overlay_key*"`

Nothing else. No `hydra_uitest` script is named for this task.

## Done when (from the plan)

- One `path_identity` exists in core model, and both the all-0 dedupe and `path_overlay_key` read it. No second "same path" rule remains in `path_view.cpp` or `preview_view.cpp`.
- One `song_fraction` helper exists, and both the timeline and the scrub marks call it. Neither file types the clamped division any more.
- The `song_length_ms` comment in `preview_view.h` says what the field is.
- Positions on screen are identical: the four existing cases pass with no edits to their pins.
- The results stamp is still "2.1.0"; `git diff --stat` shows only the owned files.
- If the new dedupe ever hides or shows the all-0 row for a record where the old rule did the opposite, stop and report the chart; do not widen or narrow the identity to fit.

## Open questions

- The audit's text for 249 and the step3a draft both say the Preview key "also folds in the rules" (hydra_rules.ini). Neither the plan's C4a row nor D48 decides this, and it would change when the Preview rebuilds its scene after a rules reload. This brief leaves the rules out of the key. If the orchestrator wants them in, say so; it also needs `src/ui/preview_controller.cpp`, which this task does not own.
- The plan's "Owns" column names only source files. This brief adds the two test files that hold the named cases, since the plan's own rule is test-first. If a different split is wanted, say so.
