# A tied variant's facts are its own

A tied variant is a path that scores exactly what another path scores. The
search keeps one of the two as the leader and stores the other as a branch of
it: the variant stores its own activations up to the fold (the point where
the search merged the two), and reads the rest from its leader on load
(`var_point` marks where). Until this change, every fact after the fold was
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

A guard test prices each corpus variant alone with a targeted search and
requires the same stored facts. It runs at the app's defaults and at score
range 40 with caps 4 and 2, because the defaults hold almost no ties. Only a
root of that search is an answer, because a root was never folded. A variant
whose activations match a root but whose squeeze kinds do not is a failure.
Sometimes the lone search ties the variant under another root too, so no root
can answer for it. The guard skips such a variant, prints it, and pins how
many there are at each setting. The best all-0 path's variants
(`allzero_paths`) are not covered.

Not decided here: a variant's skip state, early-fill offset and skipped fills
after a fold between windows (finding 97). Until that gets its own plan, a
variant still shows its leader's on that activation.

## What this costs

The record format moved to 7 with ADR 0021, and this change rides on the
same bump. On the corpus, no score, path string or path list changed.
