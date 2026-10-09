# Note Shuffle game tests

These are ten tiny Clone Hero songs. Songs A to E each answer one open question from the Note Shuffle plan (`docs/superpowers/plans/2026-10-08-note-shuffle.md`, Part 4) by playing them in the real game with Note Shuffle on. Songs F1 and F2 check the 2x kick rule from decision D105, with Note Shuffle off.

For every song, the two decoded copies of the game's shuffle (the "references", one per scout) were run on the exact notes in the file. Both copies agree on every prediction below. Hydra loaded every file and read back exactly the chords that were meant to be written, with Pro Drums on and off (16 of 16 checks; the output is in `build-scripts/hydra_check_output.txt`).

## What the code reading expects (added 2026-10-09)

These songs were built before the game-code reading finished. That reading (`../step0/FINDINGS.md`) now predicts one row per song. A1 should show its 192 row and A2 its 960 row, because the game uses each file's own ticks. B should show the colour order row, because the game sorts each chord kick, red, yellow, blue, green. C1 and D1 should freeze, and C2 and D2 should play. E should show 5 stars on a full combo, because the star base is taken after the shuffle. F1 and F2 (added later, Note Shuffle off) should show no kick on chords 3, 5 and 7 with 2x kick off. Any other result means the code reading is wrong somewhere, so please note exactly what you saw.

You don't need to hit any notes, except in song E. A short screen recording of the first few seconds is enough. A screenshot of the highway works too, as long as the first chords are on screen.

## How to install them

1. The ten folders inside `songs/` are already copied into `C:\Clone Hero\songs\Hydra Note Shuffle tests`. That's the songs folder Clone Hero's settings name (`[directories] path0` in `Documents\Clone Hero\settings.ini`). Delete that subfolder when the tests are done.
2. Start the game and rescan your songs.
3. Search for "NS". Every song name starts with "NS" and says what it tests.

## Settings for every song

Pick Drums on Expert. That is the only difficulty the songs have. Turn the Note Shuffle modifier on, unless a step says otherwise. Turn Pro Drums on: that's the main run, because cymbals show far more of the shuffle than toms do. 2x kick doesn't matter for songs A to E, because none of them has 2x kicks. F has its own settings.

A Pro Drums off run is optional. The tables give its predictions too, if you want a second check.

**Play C1 and D1 last.** They are expected to freeze the game. If the game stops responding, close it from Task Manager. The game shuffles the notes while it builds each player's notes, before play starts, so a freeze most likely shows up as a song that never finishes loading.

## How to read the tables

Each cell is one chord, left to right in the order they come down the highway. "Kick + red snare" means a kick and a red snare together. A "yellow cymbal" is the round gem, and a "yellow tom" is the normal gem.

Times are from the start of the song. Every song is quiet (the audio file is silence).

---

## A1 and A2: does the game rescale ticks? (Open 1)

**What it settles.** The shuffle's seed is built from each of the first four notes' lane times its tick. A tick is the chart's unit of time. If the game converts a file's ticks to another scale before taking the seed, every shuffle changes. The gameplay video only proved that a 480-tick .mid isn't rescaled.

**The songs.** A1 and A2 are the same 16-chord groove: a hi-hat beat for one bar, then crash, toms and snare. A1 is a .chart at 192 ticks per beat. A2 is a .mid at 960 ticks per beat. The first chord comes at 2.5 seconds, then one chord every quarter second.

What you'd see without Note Shuffle:

| | 1 | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|
| Written | kick + yellow cymbal | yellow cymbal | red snare | yellow cymbal | kick + yellow cymbal | kick |

Predicted with Note Shuffle and Pro Drums on. The same three rows apply to both songs. The first chord is kick + red snare under every guess, so look from chord 2 on.

| If the game uses ticks at... | 1 | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|
| 192 per beat | kick + red snare | yellow tom | blue tom | yellow tom | kick + blue tom | kick |
| 480 per beat | kick + red snare | blue cymbal | yellow cymbal | blue cymbal | kick + green cymbal | kick |
| 960 per beat | kick + red snare | green tom | yellow cymbal | green tom | kick + blue cymbal | kick |

With Pro Drums off (optional):

| If the game uses ticks at... | 1 | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|
| 192 per beat | kick + red snare | blue tom | red snare | blue tom | kick + yellow tom | kick |
| 480 per beat | kick + red snare | green tom | yellow tom | green tom | kick + yellow tom | kick |
| 960 per beat | kick + red snare | blue tom | yellow tom | blue tom | kick + yellow tom | kick |

**What each outcome means.**

If A1 shows the 192 row and A2 shows the 960 row, the game uses each file's own ticks. That's what Hydra already keeps, so nothing changes.

If both songs show the 480 row, the game converts every file to 480 ticks per beat first. Hydra's shuffle would then scale ticks to 480 before taking the seed.

If both show the 192 row, the game converts everything to 192.

Any other mix means the .chart and .mid readers treat ticks differently. Note which row each song showed.

If a song matches no row at all, stop. The plan's rule is not to guess a third scale.

## B: what order are the notes in inside a chord? (Open 2)

**What it settles.** When several notes share a tick, the game walks them in some order. The order decides which note counts as the chord's "first" note, and here it also decides the seed. Hydra doesn't keep the file's order, so the answer decides whether Hydra's .mid reader has to change.

**The song.** A .mid at 480 ticks per beat (the scale the video already proved). The first chord comes at 2.0 seconds, then one every quarter second. Inside each tick, the notes are written in an order that is none of the guesses below. The seed takes the first four notes, which here ends in the middle of chord 2, so different orders give different seeds.

The four guesses are these. "File order" is the order the notes are written in the file. "Colour order" is kick, red, yellow, blue, green, which is what Hydra uses today. "Tom-before-cymbal order" sorts by the game's internal lane number, which puts every tom before every cymbal. "Reverse colour order" is green back to kick.

What you'd see without Note Shuffle, with each chord's notes in file order:

| | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|
| Written | green tom, kick, yellow cymbal | yellow cymbal, kick, green tom | red snare, green cymbal | yellow cymbal, kick, blue tom | blue cymbal, yellow tom |

Predicted with Note Shuffle and Pro Drums on:

| If the game walks notes in... | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|
| File order | kick + red snare + blue cymbal | kick + red snare + yellow tom | red snare + blue cymbal | kick + yellow cymbal + green tom | yellow tom + blue tom |
| Colour order (Hydra today) | kick + red snare + green tom | kick + red snare + yellow tom | blue tom + green cymbal | kick + yellow tom + green tom | blue cymbal + green tom |
| Tom-before-cymbal order | kick + red snare + green tom | kick + yellow tom + green tom | red snare + blue tom | kick + blue cymbal + green cymbal | yellow tom + green tom |
| Reverse colour order | kick + red snare + green cymbal | kick + blue cymbal + green cymbal | yellow cymbal + green cymbal | kick + red snare + green tom | yellow cymbal + blue tom |

Chord 1 already separates file, colour and reverse order. Colour order and tom-before-cymbal order first split at chord 2.

With Pro Drums off (optional). There are no cymbals then, so colour order and tom-before-cymbal order are the same thing:

| If the game walks notes in... | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|
| File order | kick + red snare + green tom | kick + blue tom + green tom | red snare + blue tom | kick + yellow tom + green tom | yellow tom + blue tom |
| Colour order (Hydra today) | kick + yellow tom + blue tom | kick + red snare + blue tom | blue tom + green tom | kick + red snare + green tom | blue tom + green tom |
| Reverse colour order | kick + red snare + blue tom | kick + red snare + yellow tom | blue tom + green tom | kick + red snare + yellow tom | red snare + blue tom |

**What each outcome means.**

Colour order means Hydra can rebuild the order from the chord, with no parser change.

Tom-before-cymbal order also needs no parser change. The shuffle just sorts each chord's notes by lane number instead of by colour.

File order means Hydra's .mid reader has to remember where each note sat inside its tick. That's a parser change. The .chart reader may need the same, and this song doesn't test .chart.

Reverse order would also need no parser change.

If the screen matches no row, stop. The plan says one reading pass and one game test, then report if they disagree.

## C1 and C2: a four-colour chord, then a snare (Open 4)

**What it settles.** The shuffle won't put a chord's first note on any colour the previous chord used. After a chord that uses all four colours, no pad is left, so the game should loop forever. The only escape is the copy rule: a chord with the same shape as the chord two back copies that chord's pads instead of drawing. In C1 the chord two back from the snare is a lone yellow cymbal, so the snare can't copy it.

**C1.** One bar of hi-hat groove from 2.0 seconds. Then red snare + yellow cymbal + blue tom + green cymbal at 4.0 seconds. Then a lone red snare at 4.5 seconds. Both references never finish shuffling it, with Pro Drums on or off. The capped copy of the reference (`../probes/capped/`, which stops with an error instead of looping) stops on C1 and D1 and on no other song. With Pro Drums off the chord becomes four toms, which still uses all four colours.

**C2 (control).** The same song with the final snare removed. It should load and play normally.

C2 predicted with Note Shuffle and Pro Drums on:

| | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 (at 4.0 s) |
|---|---|---|---|---|---|---|---|---|---|
| Written | kick + yellow cymbal | yellow cymbal | red snare | yellow cymbal | kick + yellow cymbal | yellow cymbal | red snare | yellow cymbal | red snare + yellow cymbal + blue tom + green cymbal |
| Shuffled | kick + red snare | yellow tom | green tom | yellow tom | kick + blue tom | yellow tom | red snare | yellow tom | red snare + yellow cymbal + blue cymbal + green cymbal |

With Pro Drums off, C2's chords come out as: kick + red snare, yellow tom, blue tom, yellow tom, kick + blue tom, yellow tom, green tom, yellow tom, then all four toms.

**How to run it.** First play C1 with Note Shuffle **off**. It should play normally; that proves the file itself is fine. Then play C2 with Note Shuffle on. It should play and match the table. Last, play C1 with Note Shuffle on.

**What each outcome means.**

If C1 freezes (or crashes) with Note Shuffle on, and C2 plays, the decode is right. Hydra then needs the "game freezes on this chart" outcome from question Q5.

If C1 plays normally with Note Shuffle on, the decode is wrong somewhere. Record what the last two chords turned into. The plan says to stop there and go back to reading the code before anything else.

## D1 and D2: four notes on tick 0 (Open 4)

**What it settles.** If the first four notes all sit on tick 0, the seed is 0. The generator then returns the same number forever, so every draw lands on red. The second pad note at tick 0 must avoid red, never can, and the shuffle never ends.

**D1.** One chord at the very start of the song (0 seconds): kick + red snare + yellow cymbal + blue tom. Then a short groove from 0.5 seconds. Both references never finish shuffling it, with Pro Drums on or off.

**D2 (control).** The same notes moved two bars later, so the first chord is at 4.0 seconds and the seed isn't 0. It should play normally.

D2 predicted with Note Shuffle on:

| | 1 (at 4.0 s) | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|
| Written | kick + red snare + yellow cymbal + blue tom | yellow cymbal | red snare | yellow cymbal | kick + red snare | yellow cymbal |
| Pro Drums on | kick + red snare + yellow cymbal + blue tom | green tom | red snare | green tom | kick + red snare | green tom |
| Pro Drums off | kick + red snare + blue tom + green tom | yellow tom | green tom | yellow tom | kick + green tom | yellow tom |

With Pro Drums on, the first chord comes out looking exactly as written. That is the shuffle's real output here, not a sign that it's off. The second chord shows whether it ran: a green tom instead of the yellow cymbal.

**How to run it.** Same as C. Play D1 with Note Shuffle off first (it should play), then D2 with it on, then D1 with it on, last.

**What each outcome means.**

If D1 freezes with Note Shuffle on and D2 plays, the seed-zero case is real.

If D1 plays with Note Shuffle on, the decode is wrong somewhere. Record the first few chords and stop, as for C.

## E: are star cutoffs taken before or after the shuffle? (Open 3)

**What it settles.** Star cutoffs are a fixed share of a "base score": 50 per gem, plus 15 more for each cymbal. If the game works out the base before shuffling, the cymbals the shuffle adds raise your score but not the cutoffs. You'd then get more stars than the shuffled chart deserves.

**The song.** 82 chords of single toms and snares with kicks, at 110 BPM, about 25 seconds. It has no cymbals as written, no Star Power and no solos. With Pro Drums on, the shuffle turns 32 of its 106 gems into cymbals. The first chord comes at about 2.2 seconds.

Its first chords, predicted with Note Shuffle and Pro Drums on: red snare, green tom, red snare, yellow tom, blue cymbal, yellow cymbal, kick + blue cymbal, yellow tom. As written they are yellow tom, blue tom, green tom, red snare, red snare, blue tom, kick + yellow tom, green tom.

**How to run it.** Pro Drums on and Note Shuffle on. Play it for a full combo, meaning no missed notes, and don't use Star Power (there is none to use). Record the results screen: the star count and the score.

Hydra scored the shuffled chart. A full combo is worth 19,925 points.

The six-star cutoff depends on when the base is taken. Taken before the shuffle, the base is 5,300 (106 gems at 50). Six stars then need 19,080, so a full combo shows **6 stars**. Taken after the shuffle, the base is 5,780 (5,300 plus 32 cymbals at 15). Six stars then need 20,808, so a full combo shows **5 stars**.

| Stars | Cutoff if base is taken before | Cutoff if base is taken after |
|---|---|---|
| 5 | 14,840 | 16,184 |
| 6 | 19,080 | 20,808 |
| 7 | 23,320 | 25,432 |

**What each outcome means.**

Five stars on a full combo means the base is taken after the shuffle. Hydra already does that, so there's nothing extra to build.

Six stars means the base is taken before the shuffle. The Stars tab would then need the unshuffled base next to the shuffled score. The plan says that design goes back to you first.

If you miss a note, the test can still work. Any score from 19,080 to 20,807 separates the two the same way (6 against 5 stars). So does any score from 14,840 to 16,183 (5 against 4 stars).

If a full combo scores something other than 19,925 before any bonus, the shuffle put a different number of cymbals on the highway than the reference says. Please note the score; that is a finding of its own. The results screen may add bonus points on top, but the star count doesn't include them.

With Pro Drums off there are no cymbals either way, so E can't tell anything apart. It scores 18,350 and shows 5 stars under both answers.

## F1 and F2: a 2x kick and a normal kick on one tick (decision D105)

**What it settles.** This one doesn't need Note Shuffle. The code reading says the game merges a 2x kick and a normal kick on the same tick into one kick marked 2x. With 2x kick off, the game then removes every 2x kick, so that tick shows no kick at all. Hydra today keeps the normal kick there. If the game agrees with the code, Hydra changes to match (D105).

**The songs.** F1 is a .mid and F2 is the same song as a .chart. Nine chords, one every half second from 2.0 seconds. Every chord has a red snare. The kicks are written like this: chord 1 a normal kick; chord 3 a normal kick and a 2x kick, normal written first; chord 5 the same pair, 2x written first; chord 7 a 2x kick alone; chord 9 a normal kick. Chords 2, 4, 6 and 8 are a lone snare.

**How to run it.** Note Shuffle off. Drums on Expert, Pro Drums either way. Play each song twice, once with 2x kick off and once with it on, and note which chords show a kick.

| | 1 | 3 | 5 | 7 | 9 |
|---|---|---|---|---|---|
| Code reading, 2x kick off | kick | no kick | no kick | no kick | kick |
| Code reading, 2x kick on | kick | kick | kick | kick | kick |
| Hydra today, 2x kick off | kick | kick | kick | no kick | kick |
| Hydra today, 2x kick on | kick | kick | kick | kick | kick |

**What each outcome means.** If chords 3 and 5 show no kick with 2x kick off, the code reading is right, and Hydra will merge the two kicks the way the game does. If they show a kick, Hydra is already right and nothing changes. With 2x kick on, every row should show a kick on all five chords; anything else is a finding of its own.

`build-scripts/build_f.py` builds both songs and prints Hydra's rows.

---

## What's in this folder

`songs/` holds the ten song folders. Each has a song.ini, a notes.chart or notes.mid, and a silent song.ogg.

`predictions.json` holds every song's written notes and every prediction, including the rows the tables leave out.

`build-scripts/` holds the scripts that made all of this. `build.py` writes the songs and predictions, and `ns_lib.py` holds the writers. `run_refs.py` runs each scout's reference on a note list. `design.py` and `design_e.py` searched for layouts where the guesses differ. `hydra_check.py` loads each song in Hydra and compares the chords. `mid_order.py` reads B's file back byte by byte to confirm the written order (`b_file_order.txt`). The references they run live in `../scout-flag/` and `../scout-strings/`.
