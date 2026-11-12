# Five-lane drum charts, read the way Clone Hero plays them

Plan written 2026-11-12, task PL-FIVE-LANE, session c61c1986. Nothing here is built yet. Every task and every live step waits for the user's yes (agent rule 3).

## The short version

Some drum charts are written for a five-pad kit: red, yellow, blue, orange and green, plus the kick. Guitar Hero World Tour charts are the usual case. Hydra reads only four pads, so on such a chart it throws away every note on the last pad and scores two of the others wrongly.

Clone Hero does not throw anything away. When it loads a five-pad chart it rewrites it into an ordinary four-lane Pro Drums chart, once, before any player touches it. The rewrite is one function in the game, and its listing has been sitting in this repo since October without any reference page citing it. I read it for this plan. It says: yellow becomes the yellow cymbal, blue becomes the blue tom, orange becomes the green cymbal, and green becomes the green tom. When orange and green fall on the same tick, green moves to the blue tom so the two do not collide.

That rule matches the one recorded run we have, to the point. So the plan is to copy that one rewrite into Hydra's chart readers, in one place, and leave everything after the readers alone. A converted chart is a plain four-lane Pro chart, so the engine, the path screens and the preview already know how to handle it.

Four things are still open in the game's code, and each one has a task or a live step below that closes it. Four questions are for the user.

## What the game does (build 1.1, v1.1.0.6142)

Every fact in this section was read today in a listing of build 6142, unless it says otherwise. The recorded run is from another build, 1.0, and is kept apart at the end of the section.

**The two pads have names in the game.** A drum chart's pads are note types 13 to 18: kick, red, yellow, blue, then GreenDrum (17) and OrangeDrum (18). In a .mid, the pitch five above the 2x kick (100 on Expert) is the orange type, and the pitch six above it (101 on Expert) is the green type. In a .chart, N 4 is orange and N 5 is green. So what our pages call "the fifth pad" is the game's green, and the pad Hydra calls green is the game's orange. This matters because the function that turns a note into a lane gives the orange type no lane at all. Something has to rewrite every orange note first, and that something is the conversion step.

**When the conversion runs.** It runs for every drum track, in both chart formats, at the end of the step that turns markers into note flags. It runs after the notes on one tick have been sorted by type. It is done on the track, not per player, so a four-lane player and a Pro Drums player get the same converted notes.

**How it picks a branch.** The track carries two flags. The first goes up when the chart holds a green-type note (pitch 65, 77, 89 or 101 in a .mid, on any difficulty; N 5 in a .chart). The second goes up when the chart holds Pro Drums marks (a tom marker in a .mid, N 66 to 68 in a .chart). The step takes the five-lane branch when the first flag is up and the second is down. Otherwise it reads one more byte, and takes the five-lane branch when that byte is set. What that byte is, and whether song.ini's five_lane_drums key sets it, is not read yet (gap G1).

**The five-lane branch.** Each note is rewritten by its type:

| Chart pad | .mid pitch (Expert) | .chart | Becomes |
|---|---|---|---|
| Red | 97 | N 1 | Red (snare) |
| Yellow | 98 | N 2 | Yellow cymbal |
| Blue | 99 | N 3 | Blue tom |
| Orange | 100 | N 4 | Green cymbal |
| Green | 101 | N 5 | Green tom, or blue tom when the same tick also holds an orange note |

Only the type and the tom and cymbal bits are written. A note's accent, ghost, Star Power, solo and flam marks stay on it.

**The four-lane branch.** This is the ordinary case, and it is where today's "green" comes from: every orange-type note is renamed green. One extra rule covers a chart that has both a green-type note and Pro Drums marks. There, the green-type note becomes a green tom. If the same tick also holds an orange note, it becomes blue instead, and takes the flags of a blue note already on that tick when there is one.

**Two pads on one lane.** The conversion avoids the common collision itself, by moving green to blue. It can still leave two blue toms on one tick, when a chord holds blue, orange and green together. The game has a step that merges two notes of one type on one tick into one, but whether that step runs before or after the conversion is not read yet (gap G2). Schism has no such chord, so its numbers do not depend on it.

**Accents, ghosts, 2x kick, fills, solos and Star Power.** None of these depend on which pad a note is. Accent and ghost are flags on the note and the conversion keeps them. The 2x kick, fills, solos and Star Power phrases are markers by tick, read before the conversion. So a green-type note is simply one more note in its chord for all of them. Which .chart numbers mark the accent and ghost of the orange and green pads (N 37 and N 43, N 38 and N 44) is on the page but I did not re-read the decoder (gap G3).

**Plain Drums.** With Pro Drums off, the game folds every cymbal onto its tom after the conversion. So a five-lane chart played without Pro Drums has no cymbals, the same as any other chart.

**A five-lane kit.** The game also has a FiveLaneDrums controller type. For that player a later step moves yellow toms and blue cymbals around (NL-73), and the lane list has a ninth lane, FiveLaneGreen. How that player's notes are laid out and scored is not read, and this plan does not model it unless the user asks (question 1).

**The recorded run (build 1.0, one results screen, arithmetic only).** A run of Tool's Schism on v1.0.0.4080 shows Notes 106,075 with one miss. Adding the missed note's 50 gives 106,125. The chart's Expert track holds 561 kicks, 198 red, 519 yellow, 177 blue, 266 orange and 166 green (my count today, with mido). At 50 for a kick, snare or tom and 65 for a cymbal, the 6142 rule gives (561 + 198 + 177 + 166) x 50 + (519 + 266) x 65 = 55,100 + 51,025 = 106,125. So the 1.0 run agrees with the 1.1 code. It is one screen and a sum. It cannot tell a green tom from a blue tom, and it is not proof of what 1.0's code says.

### What is still open in the game's code

- **G1.** What the extra byte in the branch test is, and what sets it. Task T1 reads it. Live step LS3 checks the song.ini key in the real game.
- **G2.** Whether two notes of one type on one tick are merged before or after the conversion. T1 reads the order. LS4 checks it in the real game.
- **G3.** The .chart accent and ghost numbers for the orange and green pads. T1 re-reads the decoder.
- **G4.** Whether build 1.0 holds the same step. T1 looks in the 1.0 dump. If it is not there, the 1.0 side stays "one results screen".

## What Hydra does today

Hydra's .mid reader takes the four pitches above the kick and nothing else, so pitch 101 is dropped before it becomes a note. The .chart reader knows N 1 to N 4 and drops N 5, N 38 and N 44. Neither reader looks at whether the chart is five-lane, so yellow, blue and orange are scored by the ordinary Pro Drums rule: a cymbal unless a tom marker says otherwise.

On Schism that gives Notes 100,480, which is (561 + 198) x 50 + (519 + 177 + 266) x 65. Blue is paid as a cymbal when the game plays it as a tom, and all 166 green notes are missing.

## Where Hydra changes

**The rule gets one home.** The conversion becomes one function in the parser, next to the disco flip, and both readers reach it through the shared chord emitter (`emit_chord_timestamp` in src/parse/song.cpp). It runs before the flam step and the disco flip, because the game converts at load and does those two later. Nothing else in Hydra may work out a five-lane mapping again.

**The readers take the two extra things.** The .mid reader accepts the pitch six above the 2x kick. Three places hold today's four-pad limit and all three move together: `MidiParser::optype`, `is_handled_note`, and the filter that the fast reader `parse_mid_lean` builds from `midi_note_is_read`. The .chart reader accepts N 5, and N 38 and N 44, through `chart_pad_color` and `ChartParser::note_optype`.

**The mode is decided once per chart, before the notes are read.** The game's two flags are facts about the whole file, so Hydra has to know them before it emits the first chord. In a .mid the first flag counts the green pitch of every difficulty, not just the one being read. The reader therefore looks through the drum track once for those two facts, then reads the notes. That look-through is the one place the mode is decided.

**Hydra's Pro Drums switch keeps its meaning.** With the switch on, a converted chart shows and scores its cymbals. With it off, the cymbals do not count, exactly as for a chart with cymbal marks today. That goes through the existing owner, `cymbal_counts`, so there is no second rule.

**Nothing after the readers changes.** A converted chord is made of the colours and cymbal marks Hydra already has. I checked every file that names a pad colour: src/core/model.cpp, src/parse/note_shuffle.cpp, src/app/preview_view.cpp, src/app/dynamics_breakdown.cpp and src/ui/dynamics_tab.cpp. They read chords the readers have already built, so they need no change. `NoteColor` gets no sixth value, and T3's acceptance holds it to that.

**Stored results.** A five-lane chart now reads differently, and the rule at the top of src/store/stored_versions.h says a reader change like this bumps `kResultsStamp`. The stamp has no way to single out five-lane charts, so every saved result reads Stale once and asks to be analysed again. That is visible, so it is question 3. The scan stamp `kChartMetaStamp` stays as it is unless T4 has to read five_lane_drums from song.ini during the scan.

## What the user will see (rule 11)

The chart is Tool's Schism, Expert, Pro Drums on. The "today" numbers are the main session's `hydra_replay dump` run, which I could not repeat because agents may not run that program; task M1 repeats it. The "after" Notes figure is arithmetic from my note count. Every other "after" figure needs the new build and is marked so.

| Screen | Text today | Text after |
|---|---|---|
| Paths tab, score breakdown, "Notes:" | 100,480 | 106,125 |
| Paths tab, "Combo Bonus:" | 297,990 | not known until built |
| Paths tab, "Star Power:" | 37,800 | not known until built |
| Paths tab, "Total Score:" | 436,270 | not known until built |
| Paths tab, the best path's code | 14 10 | not known until built; it may change |
| Paths tab, a chord's name in an activation row | a blue note reads "Blue cymbal"; a green-pad note is absent | the blue note reads "Blue tom"; the green-pad note reads "Green tom", or "Blue tom" in the one chord that also holds orange |
| Preview highway | blue gems drawn as cymbals; 166 notes missing | blue gems drawn as toms; the 166 notes drawn on the green lane as toms |
| Library list, the song's score and stars | from 436,270 | not known until built |
| Library list, every song | Ready | Stale once, until analysed again (question 3) |

Yellow and orange do not change on screen: they are cymbals today and cymbals after. A chart with no green-pad note and no five-lane switch reads exactly as it does today, apart from the Stale mark.

M3 fills the "not known" cells from the real build and shows the user the finished table before the merge.

## Prior art

One web search, 2026-11-12. The community chart-format notes give the same five mappings as the game's code: red to red, yellow to yellow cymbal, blue to blue tom, orange to green cymbal, green to green tom. They also warn that a chord holding orange and green needs a special case, and suggest that a chart with both five-lane notes and Pro Drums marks be read as Pro. Both points match what the listing does. Sources: https://thenathannator.github.io/GuitarGame_ChartFormats/Chart-File-Formats/mid-format/Tracks/Drums and https://thenathannator.github.io/GuitarGame_ChartFormats/Chart-File-Formats/chart-format/Tracks/Drums. Clone Hero's own drum guide (https://wiki.clonehero.net/link/41) describes the opposite direction, moving Pro notes onto a five-lane kit, which is NL-73. The search found nothing specific on how YARG, Moonscraper or CHOpt convert, so the plan leans on none of them.

## Questions for the user

1. **Which player does Hydra model on a five-lane chart?** The real rule: the game converts every five-lane chart to four-lane Pro Drums at load, for every player. A player whose controller type is FiveLaneDrums then gets a further remap onto five lanes, which nobody has read. One end: a player on a four-pad kit or a keyboard playing Schism sees four lanes, as in the recorded run, and this plan models that. Other end: a player on a Guitar Hero kit with controller type FiveLaneDrums sees five lanes, and Hydra's lanes, chord names and maybe its score would not match their screen. My recommendation is four-lane only, with the five-lane kit left out of scope and written down as such.
2. **Should Hydra follow the game on a chart that has both green-pad notes and Pro Drums marks?** The real rule: with both present and no five-lane switch, the game reads the chart as Pro, makes each green-pad note a green tom, and moves it to blue when orange shares its tick. One end: a World Tour chart with no tom markers converts fully, as above. Other end: a Pro chart where a charter also used pitch 101 keeps its own toms and cymbals and gains the pitch-101 notes as toms. The plan follows the game. Say so if you want such charts treated differently.
3. **May every saved result read Stale once?** The real rule: `kResultsStamp` is compared as a whole string, so a bump marks every saved result Stale, five-lane or not. One end: Schism's saved 436,270 is wrong and must be redone. Other end: a four-lane chart's saved number is still right, but it reads Stale and asks for analysis again. The stamp rule in src/store/stored_versions.h says to bump. The other choice is no bump, which keeps four-lane rows Ready and leaves five-lane rows wrong until each is analysed again by hand.
4. **May a scan count the five-lane charts in your library?** It would read every chart file once to count how many hold a green-pad note or the song.ini key, and run no analysis. It needs your yes because it touches the whole library. Without it the plan still works; we just do not know how many of your charts change.

## The tasks

Five executor tasks, each one Opus executor, under 100 calls and 20 minutes. T1, T2 and T5 share no files and start together. T3 follows T2 on the same branch. T4 follows T1 and the live steps. The main session's own steps are M1 to M4.

### T1: Read the rest of the game's rule and write it onto the reference pages

This task closes G1 to G4 by reading, and it does the write-back. It works on its own branch and touches only files under docs/clone-hero.

It gives the conversion step a proper listing and note under docs/clone-hero/code/6142, lists the helper that finds another note of the chord, and regenerates the 1.1 facts table with tools/ch_mcp/dump_facts.py. It rewrites NL-04 to state the conversion, adds new blocks for the branch test, the five-lane branch, the four-lane branch and the merge order, corrects CR-18 (which says only one reader of the two flags was found), and retires or rewrites Q-13 according to the user's answers. It adds the Schism results screen to NL-04 as a 1.0 fact, worded as one screen and a sum.

Acceptance criteria:
- G1: a block names the byte, the code that writes it, and whether song.ini's five_lane_drums reaches it, each with listing lines. If it cannot be read in the budget, the block is labelled Unknown and says where the search stopped.
- G2: a block states whether the same-type merge runs before or after the conversion, with the caller's listing lines.
- G3: the NL-05 wording for N 37, N 38, N 43 and N 44 is checked against the decoder listing and corrected if wrong.
- G4: a line says whether the 1.0 dump holds the step, and where it was looked for.
- The results-screen fact cites a kept file or a public video id, never the scratch-folder screenshot. The main session supplies the kept copy before T1 starts.
- The docs check passes on the branch: the main session runs `hydra_tests.exe "-tc=docs match code*"` at merge.
- T1 also opens the 2026-10-28 run folder and settles premise P17 in its report.

### T2: Fixtures and tests, written first and handed back red

This task writes the tests before any reader changes. It owns a new folder, testdata/five_lane, and a new test file, tests/test_five_lane.cpp, plus the line that adds that file to the test build.

The corpus has no five-lane drum chart today: I scanned every .chart and .mid under testdata and found none. So T2 adds small ones. Each is a few bars long and built so one test can fail for one reason. The .chart fixtures are text. The .mid tests build their tracks in memory with the helpers in tests/midi_util.h. T2 also writes four probe song folders for the live steps (A to D below), copying the shape of the existing probe at `C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair`. While it is there it settles premises P18 and P30, by opening that folder and the reference block that says what the bot hits, and it says in its report what it found. If either turns out false, the live steps do not run as written and the main session brings the user a changed step.

Every expected value is a literal, or comes from calling the production function. No test works the mapping out a second time.

Acceptance criteria, one test each, all red on today's code:
- A five-lane .mid (green pad, no tom markers) keeps every note: the chord count and the note count match the fixture's literals.
- In that chart yellow reads as a cymbal, blue as a tom, orange as a green cymbal and green as a green tom.
- A tick holding orange and green reads as a green cymbal and a blue tom.
- A green-pad note on Easy only, with the chart read on Expert, still converts Expert.
- The same four checks on a .chart fixture with N 5.
- An accent and a ghost on a green-pad note survive the conversion, in .mid (velocity 127 and 1 under the dynamics tag) and in .chart (N 38 and N 44).
- A green-pad note inside a Star Power phrase, inside a solo and next to a 2x kick: the phrase, the solo flag and the kick read as they do without the green-pad note.
- A chart with green-pad notes and tom markers follows the four-lane branch: green pad to green tom, or to blue when orange shares the tick.
- With Pro Drums off, the converted chart has no cymbal that counts.
- A disco section on a converted chart flips after the conversion, not before.
- A four-lane chart from the existing corpus reads exactly as before: its song digest is unchanged. This one is green today and must stay green.
- The fast reader `parse_mid_lean` and the full reader give equal songs on the five-lane .mid fixture.

The main session builds the branch and runs the new file red (M2). It passes the exact red lines to T3 and to the reviewer.

### T3: The readers and the one conversion

This task makes T2's tests pass. It owns src/parse/song.cpp and src/parse/song.h, and src/core/model.h and src/core/model.cpp only if the chord needs a new method beside `apply_disco_flip`. It works on T2's branch after M2.

It adds the look-through that sets the two flags, widens the three .mid pitch gates and the two .chart functions, and adds the conversion in the shared chord emitter. It implements both branches as the listing gives them. It does not implement the extra byte (G1) or a same-lane merge (G2); those are T4's.

Acceptance criteria:
- Every T2 test is green, and the unchanged-corpus test is still green.
- The mapping is written in one function. A grep for the five-lane rule finds no second copy in src/app, src/ui, src/search or tools.
- `NoteColor` has the same five values as before.
- Schism's Notes figure is 106,125 on the new build (M3 runs `hydra_replay dump` and checks it).
- No file outside the owned list is edited.

### T4: The five-lane switch and the same-lane merge, as the game has them

This task starts only after T1 has closed G1 and G2 and live steps LS3 and LS4 have run. It uses premises P11 and P12, which are unchecked until then. If either is still Unknown, T4 does not start and the gap goes to the user as a question.

It teaches the mode decision the extra byte: whatever T1 and LS3 show sets it. If that is song.ini's five_lane_drums, the chart reading gains that key, and the task checks whether the scan stamp `kChartMetaStamp` must move. It also makes a tick with two notes on one lane read as the game reads it, through the existing same-tick owner if there is one.

Acceptance criteria:
- A fixture with tom markers and the switch set converts by the five-lane branch; the same fixture without the switch does not. Red first.
- A fixture with blue, orange and green on one tick reads with the note count LS4 measured. Red first.
- If song.ini is read, a test shows a chart whose song.ini changes is read again, and the stamp is bumped by the rule in src/store/stored_versions.h.

### T5: The stamp, the decision record and the player-facing text

This task owns src/store/stored_versions.h, the stamp cases in tests/test_store_records.cpp, the decision log and any public page that says how Hydra reads drum pads. It starts with T2, on its own branch, and merges in the same merge as T3.

It bumps `kResultsStamp` if question 3 is answered yes, and shrinks the accepted list to the new stamp. It records the decision under the next free D-number with the user's answers quoted. It searches the four public pages for any line this change makes false and fixes it by docs/agents/public-doc-style.md.

Acceptance criteria:
- With the bump: a row stamped with today's stamp reads Stale, and a row with the new stamp reads Ready (the existing stamp tests, updated).
- Without the bump (question 3 answered no): the stamp file is untouched and the decision record says five-lane rows stay wrong until analysed again.
- The decision record quotes the user's words for questions 1 to 3.

### M1: The main session repeats today's numbers

Before anything is built, the main session runs `hydra_replay dump` on Schism from main and records the path code and the four breakdown figures. This settles premise P15. If they differ from this plan's table, the table is corrected before the user sees it.

### M2: The main session runs T2's tests red

It builds T2's branch through a build pass and runs tests/test_five_lane.cpp. Every new test but the unchanged-corpus one must fail, and each failure line goes to T3 and the reviewer. A new test that passes on today's code is sent back to a fresh executor as a test that proves nothing.

### M3: The main session fills the rule 11 table and shows the user

After T3 builds, it runs `hydra_replay dump` on Schism, checks Notes is 106,125, and reads each screen in the table with `hydra_uitest` or the hydra server on a snapshot of the library. It shows the user the finished before-and-after table and waits for a yes before merging.

### M4: One review, then one merge

One fresh reviewer reads the code branch and the two docs branches. The reviewer checks the Promise map and the Premise map line by line. The pages, the code, the stamp and this plan's removal land in one merge.

## The live steps

Each step is one bot run of a probe chart a few bars long, on build 6142, with the game minimized and nobody at the keyboard. The bot never uses Star Power (premise P29). If it also hits every note (premise P30, which T2 settles before any step runs), the end-of-song score is the sum of the notes' values times the multiplier. The pass line compares that score with the total `hydra_replay score` gives for the same chart on T3's build. A score can tell a tom from a cymbal and can tell a dropped note. It cannot tell a green tom from a blue tom, so lane identity rests on the code reading alone, and the pages say so.

Each step runs once. A step that fails is not repeated: the failure, with both totals, goes to the user.

### LS1 Probe A: green pad, no tom markers

This checks the five-lane branch itself. The chart has a different number of notes on each pad, one tick with orange and green together, and nothing else. Pass: the game's score equals Hydra's. Fail: any other score; report both. It uses premises P17, P18 and P30, and does not start until T1 and T2 have settled them.

Calls: `ch_start` (never tried), `ch_wait_stats` (seen: 2026-10-28 ProDrums bot run, premise P17), `ch_stop` (seen: 2026-10-28 ProDrums bot run, premise P17)

### LS2 Probe B: green pad with tom markers

This checks the four-lane branch's extra rule. The chart is probe A plus one tom marker. Pass: the game's score equals Hydra's. It starts only after LS1's run folder exists.

Calls: `ch_start` (never tried), `ch_wait_stats` (seen: LS1's passing run), `ch_stop` (seen: LS1's passing run)

### LS3 Probe C: probe B with five_lane_drums = True in song.ini

This answers G1 in the real game. The notes are probe B's. If the score equals probe A's kind of total, the song.ini key forces the five-lane branch. If it equals probe B's, the key does nothing here. Either result is a pass, and it is written onto the page; a third score is a fail. It uses premise P11.

Calls: `ch_start` (never tried), `ch_wait_stats` (seen: LS1's passing run), `ch_stop` (seen: LS1's passing run)

### LS4 Probe D: blue, orange and green on one tick

This answers G2 in the real game. The chart holds a few such ticks and nothing else on those ticks. If the two blue toms are kept, the score counts both; if they merge, it counts one. Either is a pass and is written onto the page; any other score is a fail. It uses premise P12.

Calls: `ch_start` (never tried), `ch_wait_stats` (seen: LS1's passing run), `ch_stop` (seen: LS1's passing run)

## Out of scope, for the user to approve

- The five-lane kit player (controller type FiveLaneDrums), unless question 1 says otherwise.
- Build 1.0's code. The plan records the one results screen and nothing more for 1.0, unless T1 finds the step in the 1.0 dump.
- A count of five-lane charts in the library, unless question 4 is answered yes.
- Something I found on the way and did not chase: the same step ends by making every pad a tom when a chart has neither flag up. If the second flag really starts down for a plain .mid with no tom markers at all, the game plays such a chart with no cymbals, while Hydra with Pro Drums on pays its yellow, blue and green as cymbals. I did not read what the flag starts as, so this may be nothing. It is a four-lane question and not part of what the user approved; it should get its own look.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| Every note of a five-lane chart is kept | "The readers take the two extra things" | T2, T3 | T2's note-count and chord-count test, red in M2, green after T3 |
| Yellow reads as a cymbal, blue as a tom, orange as a green cymbal, green as a green tom | the five-lane branch table | T2, T3 | T2's four-pad test on .mid and on .chart |
| Orange and green on one tick read as green cymbal and blue tom | the five-lane branch table | T2, T3 | T2's same-tick test |
| A green-pad note on any difficulty converts the difficulty being read | "The mode is decided once per chart" | T2, T3 | T2's Easy-only test |
| A chart with green-pad notes and Pro marks follows the four-lane branch | "The four-lane branch" | T2, T3 | T2's tom-marker test; LS2 in the game |
| Accents and ghosts survive on the green pad | "Accents, ghosts, 2x kick..." | T2, T3 | T2's dynamics test, both formats |
| Star Power, solos and the 2x kick read the same with a green-pad note present | "Accents, ghosts, 2x kick..." | T2, T3 | T2's phrase, solo and kick test |
| With Pro Drums off no converted cymbal counts | "Hydra's Pro Drums switch keeps its meaning" | T2, T3 | T2's Pro-off test |
| The conversion runs before the disco flip and the flam step | "The rule gets one home" | T2, T3 | T2's disco test |
| The mapping is written once | "The rule gets one home" | T3 | T3's grep criterion, read by the reviewer |
| `NoteColor` gains no value; nothing after the readers changes | "Nothing after the readers changes" | T3 | T3's criterion; the unchanged-corpus digest test |
| A four-lane chart reads exactly as before | "What the user will see" | T2, T3 | T2's unchanged-corpus test, green before and after |
| The fast reader and the full reader agree | "The readers take the two extra things" | T2, T3 | T2's lean-versus-full test |
| Schism's Notes figure becomes 106,125 | rule 11 table | T3, M3 | M3's `hydra_replay dump` line |
| The user sees the finished before-and-after table before the merge | "M3 fills..." | M3 | the main session's message and the user's yes |
| Tests come red first | T2 | M2 | M2's red lines, passed to T3 and the reviewer |
| Saved results are redone, or the user chose not to | "Stored results" | T5 | the stamp tests in tests/test_store_records.cpp, or the decision record |
| The five-lane switch and the same-lane merge follow the game | T4 | T4 | T4's two red-first fixtures; LS3 and LS4 |
| T4 does not start on an Unknown | T4 | T4 | T1's blocks carry a label other than Unknown, checked by the main session before launch |
| The game facts go onto the pages in the same merge | T1, M4 | T1 | the docs check at merge; the reviewer reads NL-04 and CR-18 |
| The 1.0 fact is labelled one results screen and cites a kept file | T1 | T1 | the reviewer reads the block's Evidence line |
| No live step runs on an unsettled premise | T2, LS1 | T1, T2 | T1's and T2's reports state P17, P18 and P30 as found; the main session's go request for LS1 quotes them |
| A failed live step is not repeated | "The live steps" | LS1 to LS4 | the run folders: one per step |
| Each open gap G1 to G4 is closed or labelled Unknown | "What is still open" | T1, LS3, LS4 | T1's four acceptance lines |
| The decision is recorded with the user's words | T5 | T5 | the reviewer reads the decision record |
| This plan leaves the repo in the merge that finishes it | M4 | M4 | the docs check at merge |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The game has a track step at 0x215D450 that tests the two flags and one more byte, then rewrites note types and the tom and cymbal bits as this plan's tables say. | code | `docs/audit/note-shuffle/step0/open2-refute/f215D450.asm` lines 26 to 37 (the test), 57 to 118 (five-lane branch), 130 to 186 (four-lane branch), 187 to 222 (the all-toms tail) | checked 2026-11-12, read line by line |
| P2: That step is called for a track after the disco step and the within-tick sort, by the step that turns markers into flags. | code | `docs/clone-hero/code/6142/note_flags_from_markers_215E280.asm` lines 84 to 90 and `docs/clone-hero/code/6142/note_flags_from_markers_215E280.md` | checked 2026-11-12 |
| P3: Note type 17 is GreenDrum and 18 is OrangeDrum. | code | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs` lines 546901 to 546906 | checked 2026-11-12 |
| P4: In a .mid, pitch 100 is type 18 and pitch 101 is type 17, and the same holds five and six above each difficulty's 2x kick. | code | `docs/audit/note-shuffle/step0/open2/midi_type_table.txt` and `docs/audit/note-shuffle/step0/open2/f210d7b0.asm` | checked 2026-11-12 |
| P5: In a .chart, N 4 is type 18 and N 5 is type 17, and N 5 raises the first flag. | code | `docs/clone-hero/code/6142/chart_n_handler_213D620.asm` lines 85 to 105 | checked 2026-11-12 |
| P6: The lane map gives type 17 the green tom or green cymbal lane and gives type 18 no lane. | code | `docs/audit/note-shuffle/step0/open2/f20d36a0_lanemap.asm` lines 24 to 78 | checked 2026-11-12 |
| P7: The helper the step calls returns another note of the same chord with a given type; the tom test reads flag bit 4, the cymbal test bit 5, and the kick test is type 13. | code | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\ghidra\ghidra_decompile_all.txt` lines 37124 to 37151 and 36230 to 36234; `docs/audit/note-shuffle/step0/open2/getters.asm` lines 7 to 11 and 31 to 35 | checked 2026-11-12 |
| P8: The .mid reader raises the first flag for pitch 65, 77, 89 or 101 and the second for a tom marker, and writes both to every track it fills. | code | `docs/clone-hero/code/6142/mid_drum_reader_2155050.asm` lines 87 to 99, 136 to 147, 270 and 552 to 557 | checked 2026-11-12, all four ranges read |
| P9: With Pro Drums off the game folds cymbals onto toms after the track is built, and the disco flip and flam are builder steps that run later than the conversion. | code | `docs/clone-hero/code/6142/note_builder_20D2EA0.asm` lines 228 to 253 | checked 2026-11-12 |
| P10: The game has a FiveLaneDrums controller type (3) and a FiveLaneGreen lane (512), and a builder step that runs only for type 3. | code | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs` lines 532207 and 548292; `docs/clone-hero/code/6142/note_builder_20D2EA0.asm` lines 237 to 242 | checked 2026-11-12 |
| P11: The extra byte in the branch test is set by song.ini's five_lane_drums key. | unchecked | `docs/audit/note-shuffle/step0/open2-refute/f215D450.asm` lines 30 to 37 read the byte but do not name it | T1 |
| P12: Two notes of one type on one tick are merged after the conversion. | unchecked | `docs/audit/note-shuffle/step0/open2/README.md` names the merge at 0x215B8C0 but not its place in the order | T1 |
| P13: Schism's Expert drum track holds 561, 198, 519, 177, 266 and 166 notes on pitches 96 to 101, in 1,289 chords, with no tom markers, one tick holding 100 and 101, and no tick holding three pads. | chart | `C:\Clone Hero\songs\synchotic\Sync Charts\Guitar Hero\(2008) Guitar Hero World Tour\Guitar Hero World Tour\Tool - Schism\notes.mid` (md5 9dd6ea4d589ef140b98b17c545a2b6e1) | checked 2026-11-12, counted with mido |
| P14: Schism's song.ini sets five_lane_drums and drum_fallback_blue to True. | chart | `C:\Clone Hero\songs\synchotic\Sync Charts\Guitar Hero\(2008) Guitar Hero World Tour\Guitar Hero World Tour\Tool - Schism\song.ini` lines 23 and 24 | checked 2026-11-12 |
| P15: Today's Hydra gives Schism the path 14 10 and 436,270 in total (Notes 100,480, Combo Bonus 297,990, Star Power 37,800). | unchecked | the main session's brief reports it; `src/parse/song.cpp` line 933 explains the Notes figure | M1 |
| P16: A v1.0.0.4080 run of Schism shows Notes 106,075 with one miss. | unchecked | the brief reports a screenshot in the session's scratch folder; no kept file under `docs/clone-hero/` holds it | T1 |
| P17: `ch_wait_stats` and `ch_stop` have run after a ProDrums bot launch. | unchecked | `docs/clone-hero/06-scoring.md` line 159 reports a ProDrums bot run with a scorestats copy dated 2026-10-28; the run folder was not opened | T1 |
| P18: A probe song folder shaped like the existing Hydra Probe loads in a bot launch. | unchecked | `docs/agents/clone-hero-mcp.md` line 51 names the folder; it was not opened | T2 |
| P19: Hydra's .mid reader takes only the four pitches above the kick, in three places. | code | `src/parse/song.cpp` lines 777 to 790, 933 to 937 and 1976 to 1984 | checked 2026-11-12 |
| P20: Hydra's .chart reader knows N 1 to N 4, and accents and ghosts 34 to 37 and 40 to 43. | code | `src/parse/song.cpp` lines 1731 to 1772 | checked 2026-11-12 |
| P21: Both readers emit chords through one function, which holds the disco flip. | code | `src/parse/song.cpp` lines 446 to 455, 1078 and 1827 | checked 2026-11-12 |
| P22: Whether a cymbal counts is decided by `cymbal_counts` with the Pro Drums setting. | code | `src/parse/song.cpp` lines 857 and 1755 | checked 2026-11-12 |
| P23: A chord's on-screen name comes from `note_label`, which writes "tom" and "cymbal" from the note's own mark. | code | `src/core/model.cpp` lines 61 to 67 | checked 2026-11-12 |
| P24: The score breakdown labels are "Notes:", "Combo Bonus:", "Star Power:" and "Total Score:". | code | `src/app/path_view.cpp` lines 426 to 436 | checked 2026-11-12 |
| P25: A reader change that makes a chart read differently bumps `kResultsStamp`, which is compared as a whole string and is "2.14.1+coda" today. | code | `src/store/stored_versions.h` lines 42 to 55 | checked 2026-11-12 |
| P26: The test corpus holds no drum chart with a green-pad note. | chart | `testdata/input` and the rest of `testdata`, every .chart drum section scanned for N 5, N 38 and N 44 and every .mid PART DRUMS for pitches 65, 77, 89 and 101; the same script finds Schism | checked 2026-11-12 |
| P27: The reference pages cite the conversion step nowhere, and CR-18 says one reader of the flags was found. | code | `docs/clone-hero/01-reading-a-chart.md` line 185; a grep of `docs/clone-hero/` pages and `questions.md` for 215D450 finds only the listing index in `docs/clone-hero/code/README.md` lines 190 and 191 | checked 2026-11-12 |
| P28: A gem pays 50 and a gem on a cymbal lane pays 65, before the multiplier. | data | `docs/clone-hero/06-scoring.md` blocks SC-01 and SC-02, lines 11 to 24, both labelled Proven in the game's code | checked 2026-11-12 |
| P29: The game's bot never gains Star Power, so it never activates. | data | `docs/clone-hero/05-star-power.md` block SP-44, lines 467 to 470, labelled Proven in the game's code | checked 2026-11-12 |
| P30: The game's bot hits every note of a chart, so its end score is the sum of every note's value times the multiplier. | unchecked | `docs/agents/clone-hero-mcp.md` line 45 says the bot plays the song; no block was opened that says it misses nothing | T2 |
