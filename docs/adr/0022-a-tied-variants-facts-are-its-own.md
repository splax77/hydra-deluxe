# A tied variant's facts are its own

A tied variant is a path that scores exactly what another path scores. The
search keeps one of the two as the leader. It stores the other, the variant,
as a branch of the leader. The variant stores its own activations up to the
fold, the point where the search merged the two. On load it reads the rest
from its leader, and `var_point` marks where. Until this change, every fact after the fold was
the leader's, even where the variant's real fact differed. A variant folded
while SP was running showed its SP end frozen at the fold (finding 90). Every
variant showed its leader's leftover SP (finding 89).

## The decision

Before the fold, every fact is the variant's own. After the fold, the
leader's facts are the variant's, because the search folds only paths whose
futures are identical. The search itself does not change.

A variant folded while SP runs keeps its own SP-end steps up to the fold and
takes its leader's after it (ADR 0021). That gives it its leader's
deactivation, backend rows, and the SqIn or SqOut the leader made when it
closed the window. A closing SqIn or SqOut can add a + or - to the variant's
path string. Its activation note, the bank and skips it activated with, its
early-fill offset and the squeezes it made before the fold stay its own. Its
transfer scales are worked out from its own activation note and steps, never
copied.

A variant keeps its own banked bars. If it folded after the song ended, its
whole trailing list is its own. If it folded between windows, its own banked
bars come first in its next bank, then the leader's. That next bank is its
next activation's if the leader activates again. That activation is then
stored as the variant's own, and `var_point` points past it. If the leader
never activates again, the next bank is the variant's trailing list instead.
Either way its path string, score and place in the list do not change.

The trailing list sits next to `var_point` in the structure blob, because a
node holds activations only (ADR 0017).

A guard test checks each corpus variant against a lone search. That is a
targeted search told to activate exactly where the variant does, so it works
out the variant's score and facts on its own. The guard then compares the
trailing bank, the squeeze kinds, and each activation's SP-end steps, bank
arrivals, squeeze-out tick and backend rows. It does not compare transfer
scales, SqIn offsets, the early-fill offset or skipped fills. Transfer scales
come from the activation note and the steps, so they match when those match.
The last three are finding 97, below. The guard runs at the app's defaults and at score
range 40 with caps 4 and 2, because the defaults hold almost no ties. Only a
root of that search is an answer, because a root was never folded. A variant
whose activations match a root but whose squeeze kinds do not is a failure.
Sometimes the lone search ties the variant under another root too, so no root
can answer for it. The guard skips such a variant, prints it, and pins how
many there are at each setting. The best all-0 path's variants
(`allzero_paths`) are not covered.

Not decided here: a variant's skip state, early-fill offset and skipped fills
after a fold between windows (finding 97). D38 (2026-10-04) settled it. At the
fold the search keeps the variant's own SP-ready time, its passed fills and
the e_offset at the first of them. The activation its leader takes next is
stored with the variant's own facts: its own passed fills, then the ones both
passed after the fold, and the e_offset the early-fill rule gives its own
ready time. The display reads those stored facts. The tied-variant
lone-pricing tests in test_search.cpp and test_fast_tempo.cpp now compare the
early-fill offset and passed fills too. No score moved. On the corpus, one
listed path changes (The Agonist - Thank You, Pain, cap 4, score range 40,
10 ms limit: '2 0 2' becomes '2 0 5'), and no "Early fill" line changes.

## What this costs

The record format moved to 7 with ADR 0021, and this change rides on the
same bump. On the corpus, no score, path string or path list changed.
