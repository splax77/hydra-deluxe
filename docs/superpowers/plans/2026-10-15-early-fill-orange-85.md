# Early fills: orange timing up to 85 ms

Date: 2026-10-15. Status: waiting for the user's yes.

## What changes, in plain words

Today the search drops any early fill you would have to hit more than 60 ms
early. After this change it keeps every early fill up to 85 ms early. Those
are the "orange" ones, where your Star Power comes ready after the fill's
deadline and you must hit early to summon it.

The other side of the window stays at 60 ms. That's the "white" side: Star
Power comes ready before the deadline with less than 60 ms to spare, so the
activation gets the `E` mark but has nothing to time. Keeping it at 60 keeps
away the clutter you cut in 1.7.9 (D51 call 7: "lots of paths were getting
cluttered with early fills that will never matter so i tightened it from 85
to 60").

## The user's calls (2026-10-15)

1. Only the orange side goes to 85 ms. The white side stays at 60 ms.
2. The 85 is a fixed number of its own, not the hit window setting. The hit
   window stays display-only, so changing it never makes songs need
   re-analysis.
3. Every analyzed chart reads Stale. The results stamp becomes
   "2.8.0+efill85", a second bump inside the unreleased 2.8.0, the same way
   "2.1.0+allzero" was done.

## How the code handles it today

One constant, `kEarlyFillWindowMs = 60` in `src/core/model.h`, is read by two
rules. `fill_refuses` says a fill never spawns when Star Power comes ready
more than 60 ms after its deadline; that's the orange side, and it decides
what the search can find. `is_e0` says an activation is E-critical when Star
Power is ready less than 60 ms before the deadline; that's the white side.
`allzero_activation` (the E1 rule, D102) and `fill_within_limit` (the Path
limit) both go through `is_e0`.

The search reads these rules in two places in `src/search/engine.cpp`.
`branch_activate` refuses an activation whose fill won't spawn.
`ready_class` builds the search's group key from how many upcoming fills
refuse, and it stops early once a fill leaves too much slack. Its comment
says a fill that refuses always sits inside the E0 window. That stays true
after the split: anything refused (more than 85 ms late) is far inside the
60 ms white edge.

Nothing else reads the 60 directly. The display (`path_view.cpp`, the
badges, the report's Early fill column) works from `is_e_critical`,
`is_E0` and the stored difficulty, so it follows the rules on its own.

## The change

Split the one constant into two in `src/core/model.h`.
`kEarlyFillReachMs = 85.0` is the hardest early fill the search allows, and
`fill_refuses` reads it. `kEarlyFillSlackMs = 60.0` is the slack under which
a fill still gets the `E` mark, and `is_e0` reads it. The old name
`kEarlyFillWindowMs` goes away, because "window" would now be ambiguous. The
comment above each constant says only what it means and which rule reads it.
It carries no history: no old values, no dates, no versions. The history
lives in this plan, the decision record and `CONTEXT.md`.

Bump `kResultsStamp` in `src/store/stored_versions.h` to "2.8.0+efill85" and
accept only that, with a comment line saying why. The store tests that name
the stamp (`test_store_open.cpp`, `test_store_records.cpp`,
`test_perf_digest.cpp`, `test_rules_file.cpp`, `single_owner/special.cpp`)
follow it where they spell it out.

The tests move with the split. `test_model.cpp` checks each edge against its
own constant: a fill exactly 85 ms late still spawns and 85.1 ms late
refuses, and the E-critical edge stays at 60. `test_fills_and_skips.cpp`'s
"exact window edge" test moves to the 85 ms edge. A new case there builds a
fill that needs a 70 ms early hit and checks that the search now takes it
and shows it as a required early fill of 70 ms. The single-owner rows in
`tests/single_owner/rules/core.json` name the two new constants and their
owner lines. `test_docs_match_code.cpp` gets markers for both constants.

The docs follow. The "Early fill (E)" entry in `CONTEXT.md` describes both
sides and their history. In `docs/differences-from-public-hydra.md` the
"Early fill window" row reads "70 ms" against "85 ms early, 60 ms slack",
the version table gets a 2.8.0 line, and the paragraph that says the window
affects "only the display, never the path search" is corrected, because the
orange side has always steered the search. That page is public, so its
change goes through the doc-review gate.

## Checking it

The executor runs the corpus (`testdata/input`) before and after and lists
every chart whose best score, best path or stored early-fill figure changes,
with the early-fill ms that caused it. The 1.7.9 check found no best path in
the library needing 60 to 85 ms, so the expectation is few or no best-score
changes. Extra paths and tied variants with a 60 to 85 ms early fill may
appear. That list goes in the merge summary. No whole-library run happens
without your yes in chat.

`model.h` is included across the code base, so the merge runs the whole
test suite (agent rule 1), not just the touched tests.

## Who does it

One executor on Opus, in its own worktree: the constant split, the stamp,
the tests, the internal docs and the corpus diff. It's one small task, so
nothing runs in parallel. Then one fresh Sonnet reviewer reads the code and
the docs (derive-once review, plus doc-review for the public page), fixes
what it finds and signs off. Then the main session builds, runs the suite
and merges to `main`. Nothing is pushed or released without your say.
