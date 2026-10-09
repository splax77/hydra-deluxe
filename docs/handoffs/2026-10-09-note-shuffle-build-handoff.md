# Note Shuffle handoff: ready to build (2026-10-09)

## Where things stand

Step 0 of the Note Shuffle plan (`docs/superpowers/plans/2026-10-08-note-shuffle.md`) is done. The plan's four open questions are settled by reading the game's code and by game tests the user ran in Clone Hero 1.1. Every game test matched what the code reading predicted. The full write-up is `docs/audit/note-shuffle/step0/FINDINGS.md`, which a fresh agent reviewed before it was committed.

No Hydra source file has changed, and no build work is running. The commits through 541fef96 are on origin/main. c4b6b214 and this handoff are not pushed. Check with `git log origin/main..main`.

The next step is the build. Don't start it until the user says go.

## What was decided

D104 in `docs/audit/2026-10-03-fix-decisions.md` holds the user's answers to the plan's seven questions. They took the recommended option every time. In short: a "Note Shuffle" checkbox beside "2x Bass", and a mode string that ends in ", Note Shuffle" only when the switch is on. Pro Drums off still stores its own row, and there's one results stamp. A chart the game freezes on gets no path and a sentence. dmleaderboards Compare is greyed out while the switch is on. And the feature waits until the order of notes inside a chord is settled. It now is: by a game test for .mid, and by code reading alone for .chart (see "One gap is left" below).

D105 is new from step 0 and doesn't depend on the shuffle. When a 2x kick and a normal kick share a tick, the game merges them into one kick that carries the 2x mark. With 2x Bass off, it then removes that kick, so the tick has no kick at all. Hydra can differ in both cases. With 2x Bass off, Hydra never reads the 2x kick, so it keeps the normal kick. With 2x Bass on, it keeps whichever kick comes first in the file. Songs F1 and F2 confirmed the game's behaviour on video. So Hydra changes to match, and that change bumps the results stamp. Every saved row will then read Stale until it's re-analyzed.

## What step 0 found

The game uses each file's own ticks for the seed, as Hydra already does. The seed needs unsigned 64-bit wraparound.

The game sorts each chord kick, red, yellow, blue, green, and a cymbal sorts into its pad's slot. That's the order Hydra already walks, so the .mid and .chart parsers don't need to remember file order. Kicks and flam copies count toward the four-note seed. Hydra's flam rule gives the same pads as the game.

The star base is taken after the shuffle, so cymbals the shuffle adds raise the cutoffs. Hydra's plan already works that way, so no second base is stored. Song E confirmed it: a full combo scored 19,925 points with 5 stars.

Both freezes are real. A chord using all four colours, followed by a chord whose first note can't copy, hangs the game. So do four notes on tick 0. Songs C1 and D1 both left the game at "Clone Hero (Not Responding)" while loading. So D104's "no path, with a sentence" outcome is needed.

One gap is left. Song B tested the note order inside a chord for .mid only. For .chart, the order rests on the code reading, which found both readers ending in the same sort.

## Notes for whoever plans the build

The plan's build section still holds: one wave of three Opus executors (T1 the shuffle, T2 the setting and key, T3 the GUI text), then the join. Two things change, and one is added.

First, build the shuffle from the capped reference's code, `docs/audit/note-shuffle/probes/capped/shuffle_ref_capped.py`, not from any agent's prose. The skeptic found the scout's prose loose on two points. The colours blocked for a chord's later notes come from earlier notes in the same chord. And a chord's later notes copy from the chord one back, not two back.

Second, Hydra has to recognise a freeze. D104 chose "no path" and passed over the capped-loop option, a loop that carries on after the cap. A cap used only to recognise a freeze isn't covered by D104, and it would be a new number.

Here's how a freeze arises. For each note, the game first checks its copy rule. If the chord has the same pad shape as an earlier chord (two back for its first note, one back for later notes), the note copies that chord's pad and nothing is drawn, so nothing can hang. Otherwise the game redraws: it keeps drawing a random pad until it lands on a colour that isn't blocked. A redraw can't finish in two cases: when every pad the setting can draw is blocked, or when the seed is zero and red is blocked. A zero seed makes every draw land on red. It may be possible to detect exactly those two cases, after the copy rule has had its chance, instead of counting draws. That rests on the generator reaching every pad from any non-zero state, which nobody has checked yet. The only evidence so far is the corpus run: 1,288 runs finished, but no chord there has more than two pads. Check it first. If it holds, no new number is needed. If it doesn't, the cap is a new number, and that goes to the user first.

D105 needs an owner. It touches the kick handling in `src/parse/song.cpp` (lines 836 and 1509) and `Chord::add_note` in `src/core/model.cpp` (line 243). T1 and T2 also edit `song.cpp`, so give D105 to one of them, or run it first as its own small task. Two writers should never touch the same file. The results stamp bump belongs to whoever builds D105. `testdata/input` has no chart with a shared-tick 2x kick, so its test needs a fixture modelled on song F1.

## Files

Everything step 0 made is in the repo under `docs/audit/note-shuffle/`. That covers the two scout references, the capped reference, the video check and the probes. `step0/` has the findings, every agent's answer (`agent-results.json`), the assembly listings and the corpus scripts. `game-tests/` has the ten test songs, their README and the scripts that built them.

The user's game-test videos are `C:\Users\Patrick\Videos\group 1.mp4` to `group 4.mp4`. The frames the video-checking agents used are in this session's scratchpad (`video-g1` to `video-g4`). They could disappear, but the results they back are written into FINDINGS.md.

The test songs are also copied into `C:\Clone Hero\songs\Hydra Note Shuffle tests`. The user can delete that folder.

## Commits

The commits are 70369989 (D104), 65467c40 (step 0's code reading, corpus count and test songs), 41da3211 (D105 and songs F1 and F2), 541fef96 (the songs copied into the game's folder) and c4b6b214 (the game-test results). The memory note `ch-note-shuffle-algorithm` has the short version.
