# Note Shuffle handoff (2026-10-08)

## Where things stand

The design plan for adding Clone Hero 1.1's Note Shuffle modifier to Hydra is written and committed: `docs/superpowers/plans/2026-10-08-note-shuffle.md` (commit bde3dcd6 on main). It holds no code, and no source file changed. Nothing has been dispatched.

The plan is waiting on the user. They haven't answered its seven questions (Q1 to Q7), and none of its four open questions (Open 1 to 4) is settled. The next session's first job is to ask the user for those answers. Don't start step 0 or any build work until the user says so.

## What the next session should do

First, read the plan end to end. Then put Q1 to Q7 to the user in chat, one at a time or as a short batch, quoting each rule exactly as the plan states it. Record each answer as part of decision D104 in `docs/audit/2026-10-03-fix-decisions.md`. D103 is the latest number in use.

Next, the user decides when to start step 0. Step 0 does three things before any code. It copies the references out of the temp folder into `docs/audit/note-shuffle/`. It settles Open 1 to 4: one read-only Fable scout reads the game code, and the user runs the game tests. And it counts each freeze case over `testdata/input` with the reference. That count is the corpus only, never the whole library.

## The facts the plan rests on

The game's shuffle, as later settled, now lives in docs/clone-hero/02-note-list.md (NL-40 to NL-49). This session worked from the memory `ch-note-shuffle-algorithm`.

This session ran four probes against the user's video chart, using the old references. The video can't tell within-chord note orders apart: three different orders all match 22/22. Raw .mid ticks match 22/22, while ticks scaled to 192 match only 11/22. Removing the "first note avoids the previous chord's colours" rule drops the match to 5/22, so that rule is real.

That rule predicted freezes: on made-up charts, the reference hung in 1,833 of 2,000, and on a chart whose first four notes all sit on tick 0. The two freeze cases, later confirmed, are NL-47 in docs/clone-hero/02-note-list.md. This was the biggest risk in the plan, and it needed a game test before anyone wrote code.

Whether the star cutoffs include the shuffle was still open here (Open 3). The answer now lives in docs/clone-hero/02-note-list.md (NL-51) and 06-scoring.md (SC-27).

## Files that only exist outside the repo

These live in temp folders and could disappear. Step 0 copies the first group into the repo.

The old session's references are in `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6c19d0ef-61bc-419c-82d8-18564e1f9d9b\scratchpad\`. That folder has the two shuffle references (`scout-strings\shuffle_ref.py`, `scout-flag\shuffle_ref.py`), the video check (`verify\check_video.py`) and the assembly listings (`scout-flag\*.asm`, `scout-strings\*.asm`).

This session's probes are in `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\cfb6f1fa-7c74-4514-a80f-e68dfb81a59f\scratchpad\`. That folder has `order_probe.py` (note order and tick scaling), `edge_probe.py` (freezes, clashes, empty lanes), `capped\shuffle_ref_capped.py` (a reference that raises an error instead of looping forever) and `nolag\` (the reference without the previous-chord rule).

If the old folder is gone, re-derive the algorithm from the addresses in the memory note, using the Il2CppDumper output named in `docs/audit/ch-evidence.md` line 7.

## Agents

This session ran three read-only Sonnet scouts. They mapped the drum pipeline, the results storage keys and the GUI settings. All three finished and handed back their reports, and those reports are already folded into the plan. The progress journal still lists them as unfinished because their last call was the hand-back. The stall watchdog flagged one of them (the GUI scout) for the same reason. None is running, and nothing needs stopping or re-dispatching.

## Related notes

A short version of this handoff is in the shared notes folder, `C:\Users\Patrick\AppData\Roaming\Claude\side-session-notes\local_c4a0a204-88f5-4c44-8add-8fa86865cebd\note-shuffle-plan.md`. The memory `ch-note-shuffle-algorithm` now points to the plan and lists the probe results.
