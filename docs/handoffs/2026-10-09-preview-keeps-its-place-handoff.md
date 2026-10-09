# The Preview keeps its place when the notes change (handoff, 2026-10-09)

With the Preview open, changing Difficulty, Pro Drums, 2x Bass or Note Shuffle sends the playhead back to the start of the song. The user wants the playhead to stay where it is while the notes change under it. The full design is in `docs/superpowers/plans/2026-10-09-preview-keeps-its-place.md`. Read it first; this page only says where things stand and what to do next.

## Where it stands

The user asked for the change on 2026-10-09 and asked for a plan before any code. They read the plan's outline in chat and then asked for the plan document and this handoff. **They have not yet said yes to the build.** Rule 3 in `CLAUDE.md` says no work starts without that yes. So the next session's first job is to show the user the plan and wait for their answer.

No code has changed. The plan and this handoff are committed together.

## What the plan decides, in short

The reset happens because the Preview treats a mode change as a different song. It throws everything away, including the audio and the play/pause clock, and loads from scratch at 0:00. The audio doesn't depend on any of those four settings, so the fix keeps the audio and clock running and reloads only the notes on a background job. The old highway stays up until the new notes land, the way a path change already works. The plan also covers how playback's end moves with the new last note, what happens when the new mode has no notes, and which tests change.

## Two things to check before building

**Do other settings reset the playhead too?** The user said "any analysis settings". From the code, only the four settings above should reset it. SP cap, Score range, Path limit and 1.0 fills only rebuild the gold path, which leaves the playhead alone. Nobody has tried this in the app. Step 1 of the plan's "How to run it" checks it with `hydra_uitest` before any code is written. If another setting resets the playhead too, stop and tell the user, because the plan doesn't cover that.

**An existing test will change.** "a difficulty change on the open chart starts a new Preview load" in `tests/test_preview_controller.cpp` checks that a mode change starts a full load. Under the plan it won't, so the test gets rewritten to check that the position holds. That is expected, not a regression.

## How to run it

The plan's last section has the steps: check the claim, build with one Opus agent in a worktree, a fresh Sonnet derive-once review, run the GUI test and report it to the user, then the full suite once and merge.
