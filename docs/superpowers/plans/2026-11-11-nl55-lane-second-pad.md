# A double drum lane's second pad comes from a later chord, as in the game (2026-11-11)

Task PLAN-NL55. This plan changes one rule in the chart loader and the tests, page block and decision note that go with it. It waits for the user's yes (agent rule 3).

## The short version

A drum lane is the coloured strip Clone Hero draws down a pad column during a roll or trill. A double lane lights two pads. The game picks the second pad from the next chord after the lane's first chord. Hydra picks it from the first chord itself when that chord has two or more pads. The fix is to always look at the next chord.

The change is small. One function in `src/parse/song.cpp`, `game_lane_pads`, loses its special case. One existing test pins the old behaviour and is rewritten, and five new cases are added.

Only the Preview's highway reads a lane's pads. The engine never reads them, so no best score and no path can move. I measured the corpus anyway: none of the 10 double lanes in `testdata/input` changes.

Two things need the user. The game's listing does not show what happens when the song has no later chord at all (question 1). And I found no real chart where the Preview changes, so the before and after below come from a test chart (question 2).

## One thing in the brief that the code contradicts

The brief for this plan said lit pads change what Hydra counts and scores. They do not. `Song::drum_lanes` has one reader outside the loader: `build_preview_scene` in `src/app/preview_view.cpp` lines 345 to 349, which hands the pads to the highway (`src/render/track_state.cpp` line 261, drawn at `src/render/highway_draw.cpp` line 317). Nothing under `src/search` or `src/store` names a drum lane. The decision log says the same: "Hydra's engine does not, and this decision changes no score" (D112, `docs/audit/2026-10-03-fix-decisions.md` line 628). So this plan changes a drawing and nothing else.

## The game's rule, checked against the listings

The page block NL-55 (`docs/clone-hero/02-note-list.md` line 567) says:

> a single lane lights the lowest pad of the first chord with a non-kick note, in the order red, yellow tom, blue tom, green tom, yellow cymbal, blue cymbal, green cymbal. A double lane adds the highest pad of a later chord: the search for the second chord starts from the first chord and always skips it, even when the first chord has two or more pads. Example: first chord red plus yellow cymbal, next chord blue tom; the game lights red and blue tom.

I opened the four listings the block cites. They agree with the block. Here is what each shows.

The builder, `docs/audit/drum-lanes/f20d12c0.asm`, finds the first chord at lines 126 to 133 and takes its lowest pad at lines 148 to 154. For a double lane (the test at lines 155 and 156) it calls the search a second time at lines 157 to 163. That call passes the first chord's own tick and its own index. The search, `docs/audit/drum-lanes/f5de790.asm` lines 69 to 78, takes a chord when its tick is later, or when the tick is equal and the index differs. So the first chord is never taken. Lines 79 to 84 then require a non-kick pad. The builder takes that chord's highest pad at lines 169 to 175 through `docs/audit/drum-lanes/f20d51d0.asm`. Nowhere does the builder count the first chord's pads.

The listings also show two things the block does not say yet.

First, the second search has no end. The builder passes it four values: the chord list, a tick, the pad mask and a start index (lines 157 to 162). The lane's end is not one of them. So the second chord can sit after the lane's end. Hydra already does this when the first chord has one pad: its loop runs to the end of the song (`src/parse/song.cpp` line 158).

Second, the builder does not check whether the second search found anything. The search returns -1 when it runs out of chords (`f5de790.asm` lines 54, 55 and 87). The builder hands that result straight to the list's item getter (`f20d12c0.asm` lines 163 to 167) with no test for -1. No listing in the repo shows what that getter does with -1. So what the game does when no later chord exists is not shown. That is question 1.

## Hydra's code today

The rule lives in one place: `game_lane_pads`, `src/parse/song.cpp` lines 143 to 169. Lines 155 and 156 are the difference. When the first chord has two or more pads, the function takes that chord's own highest pad and never looks further. Otherwise lines 158 to 164 take the next chord with a pad, anywhere later in the song.

Its helpers are `lights_lane_pad` (line 88, kicks never count), `lane_pads` (line 108, a chord's lowest and highest pad) and `lane_span` (line 131, which chords a lane starts from). `light_drum_lanes` (line 203) calls it for each lane when Note Shuffle is off. With Note Shuffle on it calls `every_lane_pad` instead (D254), which this plan does not touch.

I searched for other copies of the rule. In `src`, `tools` and `tests` there is none: every other mention of a lane's pads reads the finished `DrumLane::pads`. Two audit scripts under `docs/audit/drum-lanes/play/` hold a Python copy of the old rule, `lanelib.py` line 207 and `lanecount.py` line 207. They are dated records of the October lane audit, not code Hydra runs. They are on the out-of-scope list at the end.

The old rule is also written down in three documents. The decision log's D112 item 3 says the second pad comes from "the first chord itself when it has two or more pads" (`docs/audit/2026-10-03-fix-decisions.md` line 625). The test "`.chart: a double lane on a three-pad chord lights its lowest and highest`" pins it (`tests/test_song_parse.cpp` line 529). The page block NL-55 records it as a difference. Task T1 fixes all three. The User Guide's lane paragraph (`docs/UserGuide.md` line 315) says a trill "lights up to two, the pads it moves between", which stays true, so it needs no edit.

## What the user will see

One screen reads the changed value: the Preview's highway, in the lane strips. No text anywhere reads it: no table, no tooltip, no report line.

On every real chart I checked, nothing changes. `testdata/input` holds 115 chart files with 10 double lanes in six songs, and each of those lanes starts on a lone green cymbal. The six songs are Trivium - Feast of Fire, DOMi & JD Beck - Zildjian LIVE! Performance 2020 (two lanes), Avenged Sevenfold - Unbound (The Wild Ride), Rush - YYZ, Between the Buried and Me - (B) The Decade Of Statues, and The Agonist - Thank You, Pain (one lane on each of four difficulties). Rows P8 and P9 name the files and ticks.

So the before and after come from the test chart T1 adds, not from a real chart. The chart has a double lane from tick 0 to 192, a chord of red and yellow cymbal at tick 0, and a blue tom at tick 96.

| Screen | Before | After |
|---|---|---|
| Preview highway, the lane strips under that lane | red column and yellow column lit | red column and blue column lit |

Agent rule 11 asks for a real chart. I do not have one, and question 2 asks how the user wants that settled.

## Does any score or path move

No. Three facts back that.

The engine does not read lanes (see "One thing in the brief" above, rows P5 and P6).

No corpus chart reads differently. I read every double lane in `testdata/input` and worked out both rules for each: 10 double lanes, none with two or more pads on its first chord, none that changes, none without a later chord, and none whose later chord sits past the lane's end.

T1 proves it again after the build. The corpus parse digests hash every lane's pads (`tests/song_digest.h` lines 250 to 256). If any corpus lane changed, "the corpus's parse digest is pinned" would fail. T1 must leave those pins untouched.

## The results stamp

`kResultsStamp` in `src/store/stored_versions.h` line 55 stays as it is. Its rule (lines 42 to 54) asks for a bump when "analysis output changes": anything a fresh analysis would compute differently. A lane's pads are not analysis output. No table stores them, and the engine does not read them. A saved result computed before this change is the same result after it.

## Global constraints

The rule stays in one place, `game_lane_pads`. No test works the expected pads out again; each pins literal pads.

No new number is introduced.

Note Shuffle on is untouched: `every_lane_pad` and D254 stand.

Single lanes are untouched.

Agents do not build or run `hydra_tests.exe`. The main session builds and runs T1's tests.

## Questions

1. **A double lane whose first chord is the last chord with a pad in the whole song.** The game's listing does not show what happens. The second search returns -1 and the builder passes it on unchecked (`docs/audit/drum-lanes/f20d12c0.asm` lines 163 to 167); the getter it reaches has no listing. Hydra's real rule today is `if (second && *second != first.lowest) pads.push_back(*second);`, so with no second chord it lights the first chord's lowest pad alone. Example today: a lone green at tick 0 under a double lane, then only kicks, lights green (pinned at `tests/test_song_parse.cpp` line 586). The other end, after this change: red plus yellow cymbal at tick 0 under a double lane, then nothing. Side (a): light red alone, which keeps today's rule for "no second chord" and is what T1 is written to do. Side (b): keep red and yellow for that one case, which keeps a branch the game does not have. Side (c): drop the lane. I recommend (a). No chart I read has this case.

2. **A real chart for the before and after.** None of the 115 chart files in `testdata/input` changes. I also read the first 200 charts with a double lane named in `docs/audit/drum-lanes/play/library_all_charts_with_lanes.csv` (444 double lanes, read-only, by path), and none changes there either. That list names 1,887 library charts with a double lane. Side (a): accept the test chart as the example, since the case is rare. Side (b): allow a one-time, read-only read of those 1,887 named chart files to find real examples, and show the Preview before and after on one of them before merge. Reading all of them is close to a library scan, so it needs your yes in chat.

3. **Out of scope.** The list at the end needs the user's yes.

## Tasks

### T1: the second pad always comes from a later chord (C++ and docs; Opus; one task)

Uses rows P1 to P4 and P7. The no-later-chord case uses row P12 and waits on question 1; the text below assumes side (a).

**The code.** In `game_lane_pads` in `src/parse/song.cpp`, remove the branch that takes the first chord's own highest pad. A double lane then always walks forward from the chord after the first one, to the end of the song, and takes the highest pad of the first chord that has a pad. The line that adds the second pad only when it differs from the first stays. Update the comment above `game_lane_pads` if it no longer fits; it must say why, and point at NL-55, without restating the rule.

**The tests**, in `tests/test_song_parse.cpp`, each built with the file's own `lane_chart` helper:

- Rewrite "`.chart: a double lane on a three-pad chord lights its lowest and highest`". Its chart is red, yellow cymbal and blue tom at tick 0, then a green tom at tick 96. It must now expect red and green, with Pro Drums on and off, and its name and comments must say the second pad comes from the next chord.
- Add the block's example: red plus yellow cymbal at tick 0, blue tom at tick 96, double lane 0 to 192. Expect red and blue.
- Add a kick between them: red plus yellow cymbal at 0, a lone kick at 48, yellow tom plus blue tom at 96. Expect red and blue.
- Add a later chord past the lane's end: red plus yellow cymbal at 0, double lane 0 to 96, green tom at 192. Expect red and green.
- Add a repeat: red plus yellow cymbal at 0, red at 96. Expect red alone.
- Add the no-later-chord case: red plus yellow cymbal at 0 and nothing after. Expect red alone (question 1, side (a)).

Every one of these fails on today's code, because today's code answers red and yellow for each.

**The documents, in the same commit.** Apply the page block update below to `docs/clone-hero/02-note-list.md`. Add a dated correction under D112 in `docs/audit/2026-10-03-fix-decisions.md`, saying item 3's "the first chord itself when it has two or more pads" was wrong and citing NL-55. Add the user's choice to `docs/agents/rulings/clone-hero.md` in that file's own row shape, quoting "Match the game (Recommended)" with its card, `docs/sessions/2026-11-08-77057df2.md` line 19. The main session gives the D-number for the correction in the executor's brief.

**Acceptance criteria.**

1. The six test cases above exist and pin literal pads. The main session runs `hydra_tests.exe "-tc=*double lane*"` green after the build. The reviewer reads each case and confirms today's code would answer red and yellow.
2. These tests pass with no edit to them: "drum lanes: the testdata lanes, with the pads Clone Hero lights", "`.chart: a double lane on a one-pad chord takes the next chord with a pad`", "drum lanes take their pads from the shuffled notes with Note Shuffle on" and "`.chart: with Note Shuffle off, a lane over several pads keeps the game's pads`". That proves single lanes, one-pad first chords and Note Shuffle on are unchanged.
3. `hydra_tests.exe "-tc=the corpus's*digest is pinned*"` passes, and the diff does not touch `tests/test_perf_digest.cpp`. That proves no corpus chart reads differently and no prepared row moves.
4. The diff does not touch `src/store/stored_versions.h`, `src/search`, `src/core` or `every_lane_pad`.
5. The reviewer greps `src` for `first.highest` and for a pad count compared with 2, and finds neither. That proves the old branch is gone and no second copy was added.
6. `hydra_tests.exe "-tc=docs match code*"` passes with the page block, the decision note and the rulings row in place.

## The live steps

This plan has no live step. Nothing here runs the game.

## The page block's update

Once T1 lands, NL-55 on `docs/clone-hero/02-note-list.md` changes in four lines. The Rule and Label lines stay.

Evidence: add two sentences after the sentence that ends "so the first chord is never taken". "The second search is given no end: the builder passes it the chord list, a tick, the pad mask and a start index (f20d12c0.asm lines 157 to 162), so the second chord may sit after the lane's end. The builder does not test the search's result for -1 before it reads the chord (lines 163 to 167), and the getter it calls has no listing, so what the game does when no later chord exists is Unknown." Then replace the closing sentence about the decision log with: "The decision log's D112 item 3 said the second pad comes from the first chord itself when it has two or more pads; its correction of the merge date withdraws that."

Hydra: replace the whole line with "`light_drum_lanes` in src/parse/song.cpp and `lights_lane_pad`, with the pick in `game_lane_pads`: a double lane takes the highest pad of the next chord with a pad, anywhere later in the song. When the song has no such chord Hydra lights the first pad alone." The executor adds the line numbers as they stand after the change.

Ruling: replace with "D112: \"go with clone hero\" (card 2026-10-11-14ef4882, 00:36 UTC); for the second pad the user chose \"Match the game (Recommended)\" (docs/sessions/2026-11-08-77057df2.md, 20:51 UTC). The no-later-chord case follows question 1's answer", with the user's answer to question 1 quoted in place of that last sentence.

Found: add "; Hydra matched to the game" with the merge date and the executor's session.

## Prior art

The format's public notes say a double roll lane is for "swells that alternate between two different lanes", and a charting forum says the two pads must be different colours. Neither says which pads a game lights. Onyx lights lanes by its own rule, which D112 already set aside in favour of the game's. I found no other tool that documents Clone Hero's pick. Sources: the Guitar Game Chart Formats pages for drums (https://thenathannator.github.io/GuitarGame_ChartFormats/Chart-File-Formats/mid-format/Tracks/Drums) and the Rhythm Gaming World thread "Cymbal Swell question" (https://rhythmgamingworld.com/?p=397142).

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| A double lane's second pad is the highest pad of the next chord with a pad, even when the first chord has two or more pads | The short version; T1 the code | T1 | the rewritten three-pad test and the block's example test; both answer red and yellow on today's code |
| The second search skips chords that hold only kicks | The game's rule; T1 tests | T1 | the kick-between test |
| The second chord may sit after the lane's end | The game's rule; T1 tests | T1 | the past-the-end test |
| A second pad equal to the first is stored once | T1 the code | T1 | the repeat test |
| With no later chord, the lane lights its first pad alone | Question 1 side (a); T1 tests | T1 | the no-later-chord test; waits on question 1 |
| Single lanes, one-pad first chords and Note Shuffle on do not change | Global constraints | T1 | criterion 2: four named tests pass unedited |
| No best score and no path moves; no corpus chart reads differently | Does any score or path move | T1 | criterion 3: the corpus digest pins pass unedited; criterion 4: no edit under `src/search` or `src/core` |
| The results stamp does not change | The results stamp | T1 | criterion 4: the diff does not touch `src/store/stored_versions.h` |
| The rule is derived once, in `game_lane_pads` | Global constraints | T1 | criterion 5: the reviewer's grep |
| No test works the expected pads out again | Global constraints | T1 | criterion 1: each case pins literal pads; the reviewer reads them |
| No new number | Global constraints | T1 | the reviewer reads the diff for a new constant |
| The page block, the D112 correction and the rulings row land in the same merge as the code | T1 the documents | T1 | criterion 6: the docs check passes; the reviewer reads the three edits |
| Agents do not build or run `hydra_tests.exe` | Global constraints | T1 | the executor's report lists the commands it ran; the main session runs criteria 1, 2, 3 and 6 |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: for a double lane the builder runs a second search that starts from the first chord's own tick and index, and never counts the first chord's pads | code | `docs/audit/drum-lanes/f20d12c0.asm` lines 155 to 175 | checked 2026-11-11 (read whole) |
| P2: the search takes a chord only when its tick is later, or equal with a different index, and only when it has a pad in the mask | code | `docs/audit/drum-lanes/f5de790.asm` lines 69 to 86 | checked 2026-11-11 (read whole) |
| P3: the first pad is the chord's lowest set bit and the second is the chord's highest set bit, both under the mask that leaves the kick out | code | `docs/audit/drum-lanes/f20d5310.asm` lines 29 to 35 and `docs/audit/drum-lanes/f20d51d0.asm` lines 29 to 47 | checked 2026-11-11 |
| P4: Hydra takes the first chord's own highest pad when that chord has two or more pads, and otherwise the next chord with a pad up to the end of the song | code | `src/parse/song.cpp` lines 143 to 169 | checked 2026-11-11 |
| P5: the only reader of a lane's pads outside the loader is the Preview scene, which feeds the highway | code | `src/app/preview_view.cpp` lines 345 to 349, `src/render/track_state.cpp` line 261, `src/render/highway_draw.cpp` line 317; a search of `src` and `tools` for the lane type, its list and its fields found no other reader | checked 2026-11-11 |
| P6: the decision that added lanes says the engine does not read them and no score changes | code | `docs/audit/2026-10-03-fix-decisions.md` line 628 | checked 2026-11-11 |
| P7: the user chose to match the game for this difference, on top of "go with clone hero" | ruling | `docs/sessions/2026-11-08-77057df2.md` line 19, "Match the game (Recommended)"; `docs/sessions/2026-10-11-14ef4882.md` line 18, "go with clone hero" | checked 2026-11-11 |
| P8: the .mid double lanes in the corpus each start on a lone green cymbal: Feast of Fire at tick 11520, Zildjian LIVE! at 210480 and 444480, Unbound at 360000, YYZ at 318000, The Decade Of Statues at 315360 | chart | `testdata/input/common/IB24/T3/Trivium - Feast of Fire/notes.mid`, `testdata/input/common/IB24/T7/DOMi & JD Beck - Zildjian LIVE! Performance 2020/notes.mid`, `testdata/input/common/Summer Blast _25 Setlist/Tier 4/Avenged Sevenfold - Unbound (The Wild Ride)/notes.mid`, `testdata/input/common/Summer Blast _25 Setlist/Tier 4/Rush - YYZ/notes.mid`, `testdata/input/common/Summer Blast _25 Setlist/Tier 6/Between the Buried and Me - (B) The Decade Of Statues/notes.mid` | checked 2026-11-11 (read by a script through the parser in docs/audit/drum-lanes/play/lanelib.py; T1's criterion 3 proves it again) |
| P9: the one .chart double lane in the corpus, at tick 167280 on all four difficulties, starts on a lone green cymbal with a yellow cymbal next | chart | `testdata/input/common/Summer Blast _25 Setlist/Tier 6/The Agonist - Thank You, Pain/notes.chart` | checked 2026-11-11 |
| P10: the corpus parse digest hashes every lane's pads, so a changed corpus lane fails its pin | code | `tests/song_digest.h` lines 250 to 256 and `tests/test_perf_digest.cpp` line 149 | checked 2026-11-11 |
| P11: the results stamp is bumped only when analysis output changes | code | `src/store/stored_versions.h` lines 42 to 55 | checked 2026-11-11 |
| P12: what the game does when the second search finds no chord | unchecked | `docs/audit/drum-lanes/f20d12c0.asm` lines 163 to 167 pass the result on with no test for -1; the getter they call has no listing in the repo | question 1 |
| P13: the change is rare in the user's library: none of 444 double lanes in the first 200 double-lane charts named in the committed list changes | unchecked | `docs/audit/drum-lanes/play/library_all_charts_with_lanes.csv` names the charts; the script that read them was not kept | question 2 |
| P14: the existing three-pad test pins the old rule and the one-pad test pins "only the first pad" when no later chord has a pad | code | `tests/test_song_parse.cpp` lines 529 to 545 and 585 to 588 | checked 2026-11-11 |

## Out of scope (needs the user's yes to leave out)

The two audit scripts that copy the old rule, `docs/audit/drum-lanes/play/lanelib.py` and `docs/audit/drum-lanes/play/lanecount.py`, stay as they are. They are the record of the October audit, and their saved outputs were made with them.

The game's late-start cut-off (NL-58) stays a recorded difference, as the user already chose.

The User Guide's sentence that nobody has read how late a lane may start is out of date since NL-58, but it is not about the second pad, so this plan leaves it.

Whether the game's chord list can hold two chords at one tick stays Unknown on NL-55. Hydra's loader takes the next entry in its own list either way.

A release note for this change is written when the release is cut, not here.
