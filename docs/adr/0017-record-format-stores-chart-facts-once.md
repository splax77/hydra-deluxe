# The record format stores each chart fact once

> **Superseded in part by [ADR 0026](0026-the-store-keeps-summaries-the-engine-gives-details.md),
> 2026-10-07.** The structure blob and the path nodes are gone, so every
> statement below about the stored layout is history. The record in memory
> still holds the multiplier squeezes once and each root's totals beside it.

A stored record is a structure blob (the path tree's shape) plus
content-addressed path nodes (ADR 0009). Until 1.8.1 each node carried its
own multiplier squeezes, six score totals, note count, leftover SP and two
skipped-note counts, beside its activations. Most of that was never read.

## The decision

A node holds a path's activations and nothing else.

The multiplier squeezes depend on the combo alone, and a full-combo path
never breaks combo. So they are one fact about the chart, not about a path.
The search graph finds them once, the record holds one list
(`HydraRecord::multsqueezes`), and the structure blob stores it once, right
after the record's header.

A root path's totals (the six score categories, the note count and the
leftover SP) are stored next to that root in the structure blob. A variant's
score totals and note count are not stored: `Path::prepare_variants` copies
them from the parent on every load. Its banked bars are its own and are
stored with it (amended 2026-10, ADR 0022).

The two skipped-note counts are gone. Nothing ever set them to anything but
zero, so the two warnings that read them could never show.

The six activation fields the search always sets (skips, timecode, chord,
SP meter, frontend points, early-fill offset) are plain values, with
no presence byte. The deactivation node, the cap-clamp tick and the
squeeze-out tick can legitimately be missing, so they keep theirs.

The whole-record blob format that `write_record` and `read_record` spoke is
deleted. Only tests used it, and it could not read a real old blob since
ADR 0015 anyway.

## What this costs

The path node format and the structure format both go from 5 to 6. We call
the result record format v7, since it follows blob format 6; there is no
constant named 7. Every result analyzed before this change reads Stale, and
the library needs one re-analysis. Every number, label and path string it
shows afterwards is unchanged.

## Amendment, 2026-10: format 7

The path format stamp is now 7 itself (kPathFormatStamp, ADR 0021). So
"record format v7" above means stamp 6, the 1.8.2 to 2.0.0 layout.

Format 7 changed the activation fields above. A presence byte is one byte
that says whether a value follows; a field without one is always there.

- The skip count and the SP meter are now the sizes of two stored lists: the
  passed-over fills and the bank arrivals.
- The deactivation node, the cap-clamp tick and the collected phrases are no
  longer stored. All three are read from the stored SP-end history (ADR 0021).
- The single `transfer_pre` pair is gone. Each SqIn stores its own scale.
- A squeeze entry lost its kind byte. Only SqIns are stored now; the
  squeeze-out is stored once, as its tick.
- The squeeze-out tick keeps its presence byte. It now comes before the
  history, where it used to follow the deact and clamp ticks.
- Each backend row lost its `is_sp` flag.
- A transfer scale gained a presence byte, because it may be unknown.

Two changes sit outside the node. A root's leftover SP changed from one
number to its list of bank ticks. Each tied variant stores its own such list
in its tree entry (ADR 0022).
