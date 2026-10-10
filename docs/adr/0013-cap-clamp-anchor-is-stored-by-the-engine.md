# The cap-clamp anchor is stored by the engine, never re-derived

> **Superseded in part by [ADR 0026](0026-the-store-keeps-summaries-the-engine-gives-details.md),
> 2026-10-07.** "Stored" now means the engine stamps the fact on the record
> when it analyzes. The record is no longer saved, and the store's Ready rule
> no longer checks a path format (the "What this costs" section below is
> history).

An activation's Star Power window normally ends a fixed distance past the
activation. But when a phrase collected mid-Star-Power would overfill the
meter past the SP cap (the most bars of SP you can hold), the window's end
gets pinned to the cap measured from that phrase's note instead. We call that
note the clamp anchor. A phrase that only fills the meter exactly to the cap
is a tie, and a tie does not clamp. The end is pinned only while the meter is
full. As the meter drains, a later phrase that fits under the cap extends the
end again from where it was pinned, and the clamp anchor stays the earlier
note.
`ScoreGraph::extend_deacts` owns this rule, and the "a later unclamped
extension keeps the earlier clamp_tick" row of the test "SP cap overfill:
where the end lands and which note clamped it" pins it.

The search knows the clamp anchor exactly: it is the collecting note it
compared against the cap while extending the window. But at copy-out it
used to keep only the resulting end tick, not which note produced it. So the
record could say a window was capped without saying which note did the
capping — the one fact a frontend warning needs to point the player at the
right note.

## The decision

The engine writes the clamp anchor onto the record. Nothing outside the
search reconstructs it — not from the deact node, not from the backend rows,
not from the chart.

`Activation::clamp_tick` is the field. The search stamps it onto the
activation at copy-out: the collecting note that pinned the window's end,
when the cap bound. It is left unset when the window never hit the cap. Blob
format version 5 stores it. Path node payload version 3 carries the new
activation layout.

## No fallback

A record written before blob version 5 has no `clamp_tick`. A consumer that
finds it unset says nothing was clamped — it does not guess. The overfill
warning in Song Details simply does not appear until the chart is
re-analyzed.

## What this costs

Old records show no overfill warning until they are re-analyzed. The store's
Ready rule already checks the stored path format (from ADR 0011), so records
analyzed before this change read Stale right away — the library asks for
re-analysis immediately. No separate version bump is needed beyond the one
above.

## Amendment, 2026-10: the clamp note is read from the SP-end history

`clamp_tick` is no longer a stored field. Each time the cap pins the end, the
search records a `Clamped` step in the activation's SP-end history.
`Activation::clamp_tick()` is the note of the last such step (ADR 0021). The
rule above is unchanged: nothing outside the search reconstructs it.

The clamp note now does more than drive the warning. When the cap pinned the
end, the transfer scale is measured from that note, not from the activation,
because its timing is what moves the end (D1).
