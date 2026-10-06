# Handoff: phase 7 is done (2026-10-05)

Written by session 6fc5f64c ("Phase 7 Wave 2 handoff"). It follows `docs/handoffs/2026-10-05-phase7-wave3-done-handoff.md`, which ends at wave 3. Nothing from this session is still running.

## Where things stand

Every phase 7 task is on main. After wave 3, the user replaced "song length from the last note" with "song length from the audio" (D69, D70), which became tasks AL1 and AL2. Then wave 4, typed errors (finding 193), ran as ER1 and ER2. Main is at fe95e6a. GitHub has main up to 41f8791 (phase 6 pushed it with the user's yes); the two ER merges and this handoff are not pushed.

## What merged after wave 3

| Merge | Main commit | What it is |
|---|---|---|
| AL1 | 0711fd4 | a song's length is read from its audio headers at analysis (D69) |
| AL2 | 2c4b63e | that length is stored once per song with a stamp; the store, writers, backfill and Paths tab read it (D70). Phase 6 merged it. |
| ER1 | 65b6be5 | every lower-layer error carries a kind (`src/core/error_kind.h`), and one switch in `plain_error` picks the sentence |
| ER2 | fe95e6a | the batch and the screens carry kinds, a failed save no longer closes Hydra, and the old text matcher is deleted |

ER1 and ER2 each had a CLEAN derive-once review under D61. On each merged tree the full suite ran 1103/1103 and `hydra_uitest --all` ran 63/63. Both ER authors ran out of budget; fresh agents finished each one, and the reviews named the finisher as the author. The triage file marks finding 193 fixed and updates finding 62's evidence to point at AL.

## What the user will see change with ER2

The Preview and Dynamics tabs show a plain sentence ("Preview failed: ..." plus the sentence) with the raw error text dimmed underneath. A missing Preview file reads "Reinstall Hydra". A chart whose save fails during a batch now counts as a failed chart with the database sentence, where before it closed Hydra. Every other screen shows the same sentences as before. D71 took the briefs' recommendations for all of these.

## Open, waiting on the user

- **A database that can't open at startup still closes Hydra with no message.** Fixing it needs a new startup message, which is the user's call.
- **Merged worktrees on disk:** every `.claude\worktrees\p7-*` (al1, al2, au1, doc, e1-3, er1, er2, lb1-2, pr1-2, ps1, pv, rp, se1-2, st1-2, t1, tm, w1, w2a-c). All their branches are merged. Removing them was offered, not yet approved.
- **Pushing** the ER merges.

## Notes the ER reviewers left, not blocking

- `BatchJob::run` has no catch around `run_batch`. A database read before the work pool starts (`charts_with_result`) can still throw out of the job thread. It goes with the startup-database question above.
- In the scan stage, `ReadNote::error` still decides "did it fail" by a non-empty error text, the way `WorkResult` did before ER2. An exception with an empty message would be dropped there.
- `plain_error_detail` is meant to own "the raw text for a details line", but only the Preview controller calls it; three other places write `e.what()` directly. It's the same value today.
- The dimmed, wrapped details line is drawn the same way in four screens (details panel, Dynamics, Preview, library dialogs). A small helper in `widgets.h` could own it.
- Comments in `mapped_file.h` and `dmbot_client.h` still say "throws std::runtime_error". That's still true, because `KindedError` is one.

## Scratchpad

Session 6fc5f64c's scratchpad holds the review files and prechecks (`review-er1*.md`, `review-er2.md`, `precheck-er*.txt`) and a 9 MB exe copy (`w2c-exe-0850`). The database copies are gone.
