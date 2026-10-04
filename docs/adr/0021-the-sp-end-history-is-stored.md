# The SP-end history is stored, and the displays read it

Think of a Star Power window as a parking meter. The engine sees every coin
go in: the activation, each phrase collected while SP runs, each time the cap
pins the meter, each squeeze-in. Until 2.1.0 the record kept only the time the
meter ran out (ADR 0011), the last cap note (ADR 0013) and the collected
phrases (ADR 0014). Every screen that needed the story in between had to
rebuild it, and the rebuilds went wrong in the corners.

The transfer scale stepped back two measures to guess the SP end before a
SqIn (finding 27). It measured from the activation even when the cap had
pinned the end to a later note (finding 28). It stored x1.00 when it could
not work a scale out (finding 332). The Preview gauge re-ran the meter and
showed a full bar after a late SqIn where the engine had 0.875 (finding 5).

## The decision

Each activation stores one history, `sp_end_steps`. A step is a note and the
SP end in force after it. Its kind says why the end moved:

- `Activation`: the activation itself. The end its banked bars give.
- `Collected`: a phrase collected while SP runs. Two measures more.
- `Clamped`: a phrase collected with the meter full. The end is pinned to
  the cap's length past that phrase (ADR 0013).
- `SqIn`: the squeeze-in phrase, early or late.

The first step is always the activation. A squeezed-out phrase has no step,
because its bar goes to the bank, not to the window. When SP outlasts the
chart, the last end is the end the search tracked.

The old stored fields are now read-only views of the history, with their old
names, so every reader kept compiling:

- `deact_tick()` is the last step's end (the deact node, ADR 0011).
- `clamp_tick()` is the note of the last `Clamped` step (ADR 0013).
- `collected_phrase_ticks()` is the note of every step after the first
  (ADR 0014).

New views answer the questions the displays used to guess. `nominal_end()` is
the end before any phrase. `squeeze_end_tick(k)` is the end squeeze k was
measured from: for a SqIn, the end before its step; for the SqOut, the deact
node. `end_anchor_tick(i)` is the note whose timing moves the end of step i.
`refill_tick(i)` is where the gauge gains a step's bar. None of these is
stored. Each is read from the one list.

## A list replaces the count it counts

Where a list holds exactly the items a stored count counts, the list is
stored and the count becomes its size. Three counts went this way:

- `bank_rise_ticks` replaces `sp_meter`: the tick where each spent bar
  arrived.
- `trailing_bank_ticks` replaces `leftover_sp`: the bars banked after the
  last window. Each tied variant stores its own (ADR 0022).
- `skipped_fill_ticks` replaces `skips`: the fills the path was shown and
  passed over. The Preview lights exactly these.

## Transfer scales read the history

A transfer scale says how much a frontend timing error moves the SP end. SP
length is counted in measures, so a tempo or meter change between two notes
makes the error grow or shrink.

The scale is measured from the note whose timing moves the SP end (D1). That
is the activation, or the cap's collecting note when the cap pinned the end.
The Rolling in the Deep FC video proved it (docs/cap-clamped-squeeze-frontend-anchor.md).

Each SqIn now stores its own scale, measured to the end that SqIn was
measured from. The old single `transfer_pre` is gone, so two SqIns in one
window are each exact. The scale at the deact node stays in `transfer_post`.
The engine stamps both at copy-out, the step where the search turns its
finished paths into the stored record. It uses `frontend_transfer_scales`,
which reads only the stored history.

## "Unknown" is a guard, not a state

A stored scale may be unknown. The record keeps a presence byte for it: one
byte that says whether the scale follows. The details view shows "Transfer scale unknown." in orange. But no fresh
record should ever hold it (D4). Every way to reach it is closed where it
starts, and a test analyzes the corpus and proves no record stores it. If the
note ever shows, it is a bug.

## Timing that can't measure time is refused at load

The scales, the squeeze window and every tick-to-ms lookup assume time rises
with the tick. So a chart whose timing breaks that is refused when it loads,
with a plain error that names the tick (`check_timing_maps`). That covers a
resolution of 0 or less, a measure 0 ticks long or shorter, and a tempo that
is 0, negative, or infinite. A time signature with a top number of 0 names no
meter, so it is ignored in both formats, as `.chart` already did. Every
corpus chart loads and scores as before.

## What this costs

The record format moved to 7 and the results stamp to "2.1.0" (ADR 0018).
Every saved result reads Stale once and needs one re-analysis.

Scores, path strings and the list of paths did not change on the corpus. The
scale line and the eff. figures change only where a tempo or meter change
sits between the activation and the note that moves the end.

A record from before format 7 is never read: it reads Stale first. Only a
hand-built activation has an empty history, and then its views say they
cannot tell, the same "no fallback" rule as ADR 0011.
