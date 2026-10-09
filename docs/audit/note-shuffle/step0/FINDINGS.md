# Note Shuffle step 0 findings (2026-10-09)

This is step 0 of the Note Shuffle plan (`docs/superpowers/plans/2026-10-08-note-shuffle.md`). It covers the code reading for the plan's four open questions, a freeze count over `testdata/input`, and the songs for the user's game tests. Nothing in Hydra's source changed. The user's answers to the plan's seven questions are decision D104 in `docs/audit/2026-10-03-fix-decisions.md`.

## How it was done

Three Fable scouts read Clone Hero 1.1.0.6142's GameAssembly.dll, one per question: Open 1, Open 2, and Open 3 and 4 together. A second Fable agent then re-read the code for each answer and tried to refute it. None of the three answers was refuted. The skeptics corrected some details, and those corrections are folded in below. Each agent's full answer, with addresses, is in `agent-results.json`. Their listings are in `open1/`, `open2/` and `open34/`, and each skeptic's are in the matching `-refute/` folder.

An Opus agent ran the shuffle reference over the corpus (`corpus/`). A second Opus agent built the game-test songs (`../game-tests/`).

## Open 1: ticks are not rescaled

The seed uses each file's own ticks, as 64-bit integers, for both .chart and .mid. The .chart reader reads the number before "= N" straight into the note. The .mid reader adds up each track's delta times (the gaps between events) and stamps each note with the running total. The resolution (the .chart `Resolution` line, or the .mid header's ticks per beat) is stored separately. It's used for timing, HOPO and sustain thresholds, and never touches a note's tick. The skeptic also read the .mid header parser and the game's binary chart cache, and neither rescales.

So Hydra keeps the file's own ticks, as it does now. The seed needs unsigned 64-bit wraparound, because the game multiplies and adds in 64 bits.

## Open 2: the game sorts each chord kick, red, yellow, blue, green

When a track is read, the game sorts each run of same-tick notes by note type (kick, red, yellow, blue, green), with ties broken by flags. A cymbal is a flag, not a type, so a yellow cymbal sorts in the yellow slot. The shuffle walks that sorted list. That's exactly the order Hydra already walks, so the parser doesn't need to remember file order.

Kicks count toward the four-note seed, and so do flam copies. A flam only fires on a chord with a single pad (a kick may come with it). Its copy is inserted right after the source note. Because the copy always lands one pad above the source, that position is the same as colour order, so Hydra's lane walk still matches. Hydra's flam rule gives the same pads as the game in every case. For green, Hydra keeps green and copies blue, while the game moves the source to blue and copies green. The pair of pads is the same.

One difference from Hydra doesn't depend on the shuffle. When a 2x kick and a normal kick share a tick, both readers keep them as two notes. A later clean-up step then merges them into one kick that carries the 2x mark, combining any ghost or accent marks. With 2x Bass off, the game then removes every 2x kick, so that tick has no kick at all. With 2x Bass on, it has one kick. Hydra keeps whichever of the two comes first in the file ([song.cpp:927](../../../../src/parse/song.cpp)). That's the open question in the memory note `derivation-audit-2026-10-03`. With 2x Bass off and the normal kick first, Hydra scores a kick the game doesn't have. That changes stored results, so it needs the user's decision. The corpus can't show it: 55 charts have 2x kicks (37 .mid, 18 .chart), and none puts one on the same tick as a normal kick (`kick2x/scan.py`).

The skeptic found one edge case. The game only groups same-tick notes into a chord when they sit next to each other in the parsed list. The .chart reader also checks that events come in order. So this only matters for a malformed file.

## Open 3: the star base is taken after the shuffle

The star base function has exactly one caller: the drums engine's constructor (the code that sets up the engine). That runs after the note builder returns, and the shuffle runs inside the builder. The star base reads each note's lane field, which is the same field the shuffle writes. So the game's star cutoffs count the cymbals the shuffle creates. Hydra's plan already works that way, so there's no second base to store. On plain Drums (Pro Drums off), the shuffle only ever draws the four toms.

## Open 4: both freezes are real in the code

The redraw loop has no counter, no timer and no fallback. Only the copy rule can skip it. The mask of blocked colours folds each tom and its cymbal into one colour. For a chord's first note, the blocked colours are the whole previous chord's. For later notes, they're the earlier notes of the same chord. After a chord that uses all four colours, the next chord's first note has nothing left to draw and loops forever, unless the copy rule fires.

The copy rule compares a chord's first note with the chord two back, and its later notes with the chord one back. The capped Python reference already does both. The scout's own prose described them loosely, so the build should follow the reference code, not that prose.

A zero seed isn't special-cased anywhere. With four notes on tick 0, every draw lands on red. The second pad note can never avoid red, so it loops forever.

Nothing upstream catches either case: no exception, no chord-size cap, no timeout. The code doesn't show what the screen does. A frozen window and a loading screen that never ends are both possible, and only a game test will say which.

## Corpus count (`testdata/input`, 115 charts × 16 drum settings)

There were 1,840 runs (one per chart and drum setting). 1,288 had notes and all of them finished shuffling. The other 552 are the lower difficulties of the 46 charts that chart only Expert. No run froze, no chord ended with two notes on one colour, and no note ended up with no pad.

That doesn't mean the freeze is rare in real charts. No chord in the corpus has more than two pads, so the four-colour case can't happen here. The smallest seed is 1,152, so seed zero can't happen either.

Reversing the order of notes inside each chord changed the output in 1,188 of the 1,288 runs. So getting the order right matters for almost every chart, and the code reading says Hydra's order is the right one.

## What's left

Game tests by the user, from `../game-tests/README.md`. A1, A2 and B confirm Open 1 and 2. C1, C2, D1 and D2 confirm the freezes. E confirms Open 3. The code reading predicts one outcome for each, and the README lists them.

The 2x kick merge is decision D105. The user chose to match the game once songs F1 and F2 confirm it, and the change applies with Note Shuffle off too.

Not tested in the game: the flam copy's position. That's code reading only, and Hydra's lane walk gives the same list either way.
