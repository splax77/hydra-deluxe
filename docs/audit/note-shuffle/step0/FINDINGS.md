# Note Shuffle step 0 findings (2026-10-09)

This is step 0 of the Note Shuffle plan (`docs/superpowers/plans/2026-10-08-note-shuffle.md`). It covers the code reading for the plan's four open questions, a freeze count over `testdata/input`, and the songs for the user's game tests. Nothing in Hydra's source changed. The user's answers to the plan's seven questions are decision D104 in `docs/audit/2026-10-03-fix-decisions.md`.

## How it was done

Three Fable scouts read Clone Hero 1.1.0.6142's GameAssembly.dll, one per question: Open 1, Open 2, and Open 3 and 4 together. A second Fable agent then re-read the code for each answer and tried to refute it. None of the three answers was refuted. The skeptics corrected some details, and those corrections are folded in below. Each agent's full answer, with addresses, is in `agent-results.json`. Their listings are in `open1/`, `open2/` and `open34/`, and each skeptic's are in the matching `-refute/` folder.

An Opus agent ran the shuffle reference over the corpus (`corpus/`). A second Opus agent built the game-test songs (`../game-tests/`).

## Open 1: ticks are not rescaled

The seed uses each file's own ticks, as 64-bit integers, for both .chart and .mid. The .chart reader reads the number before "= N" straight into the note. The .mid reader adds up each track's delta times (the gaps between events) and stamps each note with the running total. The resolution (the .chart `Resolution` line, or the .mid header's ticks per beat) is stored separately. It's used for timing, HOPO and sustain thresholds, and never touches a note's tick. The skeptic also read the .mid header parser and a second reader class that builds notes from a byte stream. That class's role is unknown. Neither rescales a tick.

So Hydra keeps the file's own ticks, as it does now. The seed needs unsigned 64-bit wraparound, because the game multiplies and adds in 64 bits.

## Open 2: the game sorts each chord kick, red, yellow, blue, green

When a track is read, the game sorts each run of same-tick notes by note type (kick, red, yellow, blue, green), with ties broken by flags. A cymbal is a flag, not a type, so a yellow cymbal sorts in the yellow slot. The shuffle walks that sorted list. That's exactly the order Hydra already walks, so the parser doesn't need to remember file order.

Kicks count toward the four-note seed, and so do flam copies. A flam only fires on a chord with a single pad (a kick may come with it). Its copy is inserted right after the source note. Because the copy always lands one pad above the source, that position is the same as colour order, so Hydra's lane walk still matches. Hydra's flam rule gives the same pads as the game in every case. For green, Hydra keeps green and copies blue, while the game moves the source to blue and copies green. The pair of pads is the same.

One difference from Hydra doesn't depend on the shuffle. When a 2x kick and a normal kick share a tick, both readers keep them as two notes. A later clean-up step then merges them into one kick that carries the 2x mark, combining any ghost or accent marks. With 2x Bass off, the game then removes every 2x kick, so that tick has no kick at all. With 2x Bass on, it has one kick. Hydra can differ in both cases. With 2x Bass off, Hydra never reads the 2x kick (song.cpp lines 836 and 1509). It keeps the normal kick whatever the file order, so it scores a kick the game doesn't have. With 2x Bass on, Hydra keeps whichever kick comes first in the file, because a second kick on the same tick is refused as a duplicate (model.cpp line 243). That means it marks the kick 2x only when the 2x kick comes first. This is the open question in the memory note `derivation-audit-2026-10-03`. It changes stored results, so it went to the user as D105. The corpus can't show it: 55 charts have 2x kicks (37 .mid, 18 .chart), and none puts one on the same tick as a normal kick (`kick2x/scan.py`).

The skeptic found one edge case. The game only groups same-tick notes into a chord when they sit next to each other in the parsed list. The .chart reader also checks that events come in order. So this only matters for a malformed file.

## Open 3: the star base is taken after the shuffle

A scan of direct calls found one caller of the star base function: the engine's base constructor (the code that sets up an engine), which the drums engine's constructor calls. The function is private and not virtual, so no indirect caller is expected. The constructor runs after the note builder returns, and the shuffle runs inside the builder. The star base reads each note's lane field, which is the same field the shuffle writes. So the game's star cutoffs count the cymbals the shuffle creates. Hydra's plan already works that way, so there's no second base to store. On plain Drums (Pro Drums off), the shuffle only ever draws the four toms.

## Open 4: both freezes are real in the code

The redraw loop has no counter, no timer and no fallback. Only the copy rule can skip it. The mask of blocked colours folds each tom and its cymbal into one colour. For a chord's first note, the blocked colours are the whole previous chord's. For later notes, they're the earlier notes of the same chord. After a chord that uses all four colours, the next chord's first note has nothing left to draw and loops forever, unless the copy rule fires.

The copy rule compares a chord's first note with the chord two back, and its later notes with the chord one back. The capped Python reference already does both. The scout's own prose described them loosely, so the build should follow the reference code, not that prose.

Inside the shuffle, a zero seed isn't special-cased. With four notes on tick 0, every draw lands on red. The second pad note can never avoid red, so it loops forever.

The code reading found nothing that catches either case, but it didn't look everywhere. The shuffle and the note builder are plain calls with no exception or timeout in the code that was read. The note builder has four callers. Two were read (the player set-up and the online score path), and two (0xFEBB30 and 0xFEBCB0) were not. Neither can stop the loop, since it sits inside the shuffle itself. The steps that run before the shuffle were not checked for a cap on chord size. Which thread runs all this isn't shown, so the code can't say whether the window freezes or a loading screen hangs. The game tests below settle what actually happens.

## Corpus count (`testdata/input`, 115 charts × 16 drum settings)

There were 1,840 runs (one per chart and drum setting). 1,288 had notes and all of them finished shuffling. The other 552 are the lower difficulties of the 46 charts that chart only Expert. No run froze, no chord ended with two notes on one colour, and no note ended up with no pad.

That doesn't mean the freeze is rare in real charts. No chord in the corpus has more than two pads, so the four-colour case can't happen here. The smallest seed is 1,152, so seed zero can't happen either.

Reversing the order of notes inside each chord changed the output in 1,188 of the 1,288 runs. So getting the order right matters for almost every chart, and the code reading says Hydra's order is the right one.

## Game tests (2026-10-09): every prediction held

The user ran all ten songs in Clone Hero 1.1 with the game's bot playing, and recorded four videos, `C:\Users\Patrick\Videos\group 1.mp4` to `group 4.mp4`. Four Sonnet agents read the videos, one per video, and the main session re-checked the F1 kick frames and E's results screen itself.

**Note Shuffle off (groups 1 and 2).** C1 and D1 play to the end, so the files are fine. With 2x kick off (the results screens read "No Modifiers"), F1 and F2 show a kick on chords 1 and 9 only. The results screens count 11 notes (9 snares and 2 kicks). So a 2x kick on a normal kick's tick takes that kick away, as D105 expected. Hydra today would draw a kick on chords 3 and 5 as well. With 2x kick on, both songs show exactly one kick on chords 1, 3, 5, 7 and 9 (14 notes each).

**Note Shuffle on, Pro Drums on (group 3).** A1 matches its 192 row and A2 its 960 row, so each file's own ticks are used. B matches the colour-order row. C2 and D2 play and match their predicted chords. E was a full combo of 106 notes at 19,925 points with 5 stars, so the star base is taken after the shuffle. 2x Kick was also on for this group, which changes nothing, since none of these songs has 2x kicks.

**The freezes (group 4).** C1 and D1 both hang the game the moment the song starts loading. The screen goes black with no highway, then Windows shows "Clone Hero (Not Responding)", and the game had to be closed. C1 was played with 2x Kick on too, and its Pro Drums choice happened before the recording started. The prediction is a freeze either way.

## What's left

All four open questions are settled by the code and the game, with one gap. Song B tested the note order inside a chord for .mid only. For .chart, the order rests on the code reading, which found both readers ending in the same sort. Building can start, with the plan's three executors, when the user says so. The 2x kick merge (D105) is confirmed by F1 and F2 and can go into the same build. It applies with Note Shuffle off too, and it bumps the results stamp.

Not tested in the game: the note order inside a .chart chord, and the flam copy's position. Both are code reading only. For flams, Hydra's lane walk gives the same list either way.
