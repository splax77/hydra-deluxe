# What Clone Hero does with drum lanes during play (2026-10-10, second pass)

Question from D112: the game reads lane flags while you play, and Hydra's engine does not. Does Hydra need to copy that? This is a read-only investigation of Clone Hero 1.1.0.6142's `GameAssembly.dll`. No Hydra code changed. Listings are in `play/`, next to the earlier ones in this folder. All addresses are RVAs. A first pass left several items untraced; this second pass traced them and changed two conclusions.

## The short answer

For a player who hits every note, lanes change nothing in Hydra's score, with one exception that can matter: **a double lane has a timer.** If a full round of both pads is not pressed within 0.75 s of the previous round, the game breaks the combo, even if you hit every note on time. Hydra does not model this. No double lane in `testdata` comes near it: the longest gap between lit notes is 0.1 s.

Everything else in the lane path only turns a note you did not press in time into a hit. It uses the same hit routine as a press, so score, combo and Star Power credit are identical. The first pass worried that such a hit could land later than any press could. The second pass shows it lands at the window's end, to within one rounding step. That is the latest a press could land too, so lanes open no new timing for squeezes.

My recommendation: do not model the lane leniency. Decide separately whether to model the double-lane timer, because it is the one rule that can lower a perfect player's score. The decisions are at the end.

## How a lane note is marked

The lane bits on a note's flag word, and who sets them, now live in docs/clone-hero/02-note-list.md (NL-56). The lane builder sets them at 0x20D16BA, 0x20D16DB and 0x20D16F8 (`f20d12c0.asm`).

The builder only flags notes whose pad is in the lane's pad mask (the test at 0x20D168F). A note on an unlit pad inside the lane gets no flag and is an ordinary note.

Which code reads those bits, and the limit of the scan that found it (`play/flagscan_dword.py`, `play/flagscan_byte.py`), is in docs/clone-hero/02-note-list.md (NL-56). The scan covered the whole code section: only the lane builder and the drum engine (0x20DC000 to 0x20DF700) use the bits, and the base hit routine (0x20F63F0), the score function (0x20DEEC0), the Star Power code and the star-base code never read them.

## What the engine keeps for a lane

These are fields on the drum engine object. The game's names are scrambled, so these are my labels.

- +0x2C4 is the pad mask of the open lane. +0x2C8 says whether it is double.
- +0x2C6 holds the pads not yet pressed in the current round.
- +0x2D0 is the time of the last press on a lane pad. +0x2E0 is when the current round started. +0x2D8 is when the last lane note was hit.
- +0x2E8 counts hits the game granted without a press. +0x2EC counts hits on non-lane notes while the lane is open.
- +0x2F1 says the lane has finished.

The constants 0.075 s, 0.5 s and 0.75 s are in the class's constant list and in the compares below. The limits 2 and 4 are in the grant check.

## A miss becomes a hit

Each update the engine walks the notes that are past the window (`f20f5810_base_update.asm`). A note is past when the engine time is more than the window after its time. That compare is at 0x20F59E7 and it is a strict "later than". For each such note the engine calls the miss routine, vtable slot 4, at 0x20F5C1C.

The drum engine overrides that slot at 0x20DCAD0 (`f20dcad0.asm`). A normal miss runs the base miss code at 0x20F3610 (`f20f3610_base_miss.asm`). The override can instead tail-jump to the hit routine (slot 11, at 0x20DCC11). The note becomes a hit when it is on a pad in the lane mask and one of these holds:

- **The grant budget is not used up.** This rule now lives in docs/clone-hero/03-timing.md (TM-45).
- **A lane pad was pressed a moment ago.** This rule now lives in docs/clone-hero/03-timing.md (TM-45). This path does not spend the budget.

The grant limits and the 0.075 s test are in docs/clone-hero/03-timing.md (TM-45). Every press on a lane pad zeroes both counters at +0x2E8 and +0x2EC with one 8-byte write (0x20DECF3), so during a roll you can drop a couple of notes and the game still credits them. TM-45 does not hold that reset.

A missed note on a double lane that is on a pad not yet in the mask joins the mask and is granted as well (0x20DCB3F to 0x20DCBF1). A real miss on a non-kick note that fails both tests clears the lane (0x20DCC46 to 0x20DCC6A).

There is a second grant that is not a lane rule: a missed fill-end note is hit for you when Star Power can be activated. It now lives in docs/clone-hero/04-fills.md (FL-31) and docs/clone-hero/05-star-power.md (SP-20).

## What the flag 0x800 is

0x800 marks the last note of a drum fill, the Star Power activation note. Two functions set it on the highest-pad note of each fill (0x20D015E in the .mid reader's fill code at 0x20CFF60, and 0x20D2D9D in 0x20D2B30). The note-class getter at 0x214BB50 tests it. It is not a lane flag, and the lane builder never sets it.

The press handler uses it in two ways (`f20deaa0.asm`). A correct press on a fill-end note while Star Power can be activated first calls ActivateStarPower (vtable slot 14, 0x20F5640), then hits the note (0x20DECC1 to 0x20DECE4). A wrong-pad press while such a note is in the window hits that note anyway, without activating (0x20DEDA3 to 0x20DEE29). If that note also has a lane bit, it joins the lane first.

## The press handler

The press handler is 0x20DEAA0, reached from 0x20DDC80 (vtable slot 18) and from the input poll at 0x20DE070 (slot 26, which builds the pressed-pad mask each update). A press that matches a window note hits it through slot 11 (0x20DED52). Before that, a lane note resets the counters, records the press time and clears its pad from the round mask. It then joins the lane through 0x20DE420 (0x20DECEA to 0x20DED39).

Four lookalike copies of the press handler exist (0x20DCC90, 0x20DD190, 0x20DD700, 0x20DEFF0). They use different flag masks. Every function in the DLL has exactly one pointer to it, the method-table entry. The four copies have no direct caller either (`play/xref_callers.py`), while the live handler has two. The live handler is the only one whose masks (0xC000 and 0x800) match the bits the builder sets. So the four copies are dead code. Their helper twins (0x20DE680, 0x20DD630, 0x20DE5B0) are called only by the dead cluster, or by nothing.

A press on a pad that matched no note is the overhit path (`f20de9a0.asm`, from 0x20DEE50). It clears the lane, breaks the combo and adds one to a counter at +0xB0. It applies with or without a lane, and a perfect player never reaches it. A kick press in a double lane that has not used its kick yet is allowed once instead (0x20DE9D3 to 0x20DE9F3).

## Slot 20 is the combo breaker

Vtable slot 20 is 0x20F4B60 (`f20f4b60_combo_break.asm`). It keeps the old combo, zeroes the combo at +0x88 (0x20F4BC7), resets the multiplier through slot 10, and fires an event carrying the old combo. It does nothing when the engine's role field at +0x190 is 1. That field is Player 0, RemotePlayer 1, Bot 2, Replay 3, from the enum in the dump. So the combo breaks for a local player, a bot and a replay, and not for the online opponent's mirrored engine. The base miss code uses the same slot (0x20F3683).

## The double-lane timer

This is the rule the first pass missed. Every update, `0x20DDEC0` (slot 9) and the twin at `f20de5b0.asm` check two timeouts:

- If the lane is finished (the last lit note was hit) and 0.5 s has passed since that hit, the lane clears.
- If the lane is **double**, not finished, and **more than 0.75 s has passed since the round started**, the engine calls the combo breaker and clears the lane (0x20DDF6B to 0x20DDFB9; the compare constant 0.75 is at 0x31406E8).

The round starts when a new pad joins the lane. It restarts each time every pad of the mask has been pressed (0x20DECFA to 0x20DED29). A single lane has no such timer. So in a double lane both pads must be pressed in every 0.75 s stretch. Alternating two pads at a steady 0.5 s each breaks the combo after the first full round, because a round takes 1.0 s. A fast trill passes.

A hand check of the rule's logic: with pad A at 0 s and pad B at 0.5 s, then A at 1.0 s and B at 1.5 s, the combo breaks at 1.25 s. With notes 0.1 s apart it never breaks. The simulation and its check are `play/double_lane_timer_sim.py` and `play/double_lane_timer_sim_check.py`.

The three-hit rule also clears a lane. Three hits on non-lane notes in a row while the lane is open (counted at 0x20DE7A5 to 0x20DE7B8) end it with no combo break.

## Lane hit versus normal hit

The hit routine is the same for both. The press handler calls vtable slot 11 at 0x20DED52 and 0x20DEE29. The miss override tail-jumps to the same slot at 0x20DCC11. For the drum engine slot 11 is 0x20DE770 (`f20de770.asm`).

That routine calls the base hit code at 0x20F63F0 (`f20f63f0_base_hit.asm`), then adds the score through slot 6 with a base of 50 (0x20DE964 to 0x20DE98E). The score function at 0x20DEEC0 adds 15 for a cymbal and doubles for the second note flag, and has no lane branch.

- **Score:** identical.
- **Combo:** identical. The base hit code adds one to the combo and updates the best combo (0x20F6524 and 0x20F660E).
- **Star Power phrase:** identical (0x20F6719 to 0x20F6738).
- **Hit time:** the only difference. The base hit code compares the engine clock with the note's time and with the Star Power times (below). A pressed hit has the clock at the press. A granted hit has the clock at the moment of the grant.

## The three stored times are Star Power times

Fields +0x110, +0x120 and +0x128 on the engine are:

- **+0x110** is the time Star Power was activated. ActivateStarPower stores the clock there (0x20F5677).
- **+0x120** is the scheduled end of the active Star Power (stored at 0x20F56FD) and zero otherwise.
- **+0x128** is the time Star Power last ended. DeactivateStarPower (0x20F5490) stores the clock there and zeroes +0x120.

The hit routine uses them to decide whether a note counts as in Star Power (0x20F654F to 0x20F65FB). A note scheduled before the activation but hit after it counts. A note scheduled after the scheduled end but hit before it also counts. These depend on the hit's clock, which is why timing matters to squeezes.

## When does a granted hit happen?

The engine does not wait for a frame. It schedules its own checks. For each upcoming note it computes the window start and the window end from the note's time and its window, then nudges each by one step of the last digit so rounding cannot put it inside the window. The helper at 0x180F96AA0 does the nudge. The scheduling is `f20f6a50_advance_schedule.asm` (0x20F6B70 to 0x20F6BF2). Each time is added to an event list by 0x20F43A0 (`f20f43a0_add_event.asm`). The step loop then runs the engine at each queued time in order (`f20f4950_step_loop.asm`, `f20f6ff0_next_event.asm`).

So the miss check runs at the note's window end, to within one rounding step, and a granted hit has that clock. Two cases follow. If the nudge puts the end strictly past the window, the grant happens right there. If rounding gives exactly the edge, the strict "later than" at 0x20F59E7 fails, and the grant happens at the next scheduled time instead: another note's window edge, a Star Power time, or the end of the update. That is at most one update later.

A press is valid up to and including the window end (the check at 0x20F59FB to 0x20F5A0F is inclusive). So the latest legal press and a granted hit coincide to within a rounding step in the common case. The grant never lands earlier than a press could, and not meaningfully later.

Two things I did not confirm. I did not trace who calls the input-processing method (`0x100E0D0`, the generic engine's shared version) to prove a press is processed at its own timestamp. The structure, a clock stepped to queued times and input stored at that clock, points that way. And the window itself is dynamic in the game (see the dynamic input probe), while Hydra's 85 ms is a setting. That gap is an existing approximation, not caused by lanes.

## Does it change anything Hydra reports?

**Best score and combo.** No, except through the double-lane timer. A path that hits every note gets the same score as long as no double lane has a 0.75 s stretch without both pads.

**Hit windows and squeezes.** No new reachable timing. A granted hit lands at the latest legal press time, and an early squeeze never gets a lane help. A late squeeze on a lane note gains at most one rounding step in the common case, and at most one update in a rounding tie. The first-pass worry is closed.

**Best all-0 path.** No change from the grant rule. The double-lane timer is a separate matter.

**The double-lane timer.** It can lower a perfect player's score in a chart whose double lane has slow notes or a long run on one pad. Hydra would then show a higher best score than the game allows. The corpus has no such lane.

## How many lanes could matter in the corpus

A scout counted the lanes in `testdata/input` (115 chart files, 44 .chart and 71 .mid) with its own script, checked against `lanediff.py`. The data is `play/lanes_testdata.csv`, the script is `play/lanecount.py`.

There are 49 Expert lanes in 19 charts: 42 single and 7 double. A lane holds between 3 and 108 non-kick notes. The 4 Easy, Medium and Hard lanes are in 3 .chart files that also have Expert lanes. D112 and the README say 50 lanes, and `lanediff.py` prints 49. The 49 is the tool's count.

Among the 579 notes in single lanes, 25 sit on a pad the lane does not light. Those are ordinary notes and are never granted. All 312 notes in double lanes are on lit pads.

I ran the double-lane timer rule on all 10 double lanes (7 Expert and 3 lower difficulties, all in the same three charts). The longest gap between lit notes is 0.1 s and none breaks the combo. The second-pad join, the three-hit clear and the 0.5 s clear were included.

One Expert lane has a Star Power phrase boundary at its start, end or inside: a single lane in the Zildjian LIVE! .mid, ticks 208320 to 209055. With the timing result above, that lane is not a risk either.

The scout applied Hydra's lit-pad rule from `song.cpp` in its own script and did not run Hydra's parser, so the lit and unlit counts are unchecked against the real parser. I spot-checked only the lane total (49, matching `lanediff.py`).

I did not count the real library.

## Library scan (2026-10-11)

With the user's yes in chat, I read every .mid and .chart in Hydra's library and ran the double-lane timer rule on every double lane. The library list is the `charts` table of the installed `hydra.db` (copied first): 19,499 charts. I scanned 19,415 of them, the 15,031 .mid and 4,384 .chart files. The 84 .sng and .srb containers were not scanned. 3 files failed to parse. It read files only, ran no Hydra analysis, and took 103 s. The script is `play/library_scan.py`.

3,801 charts have at least one lane. At Expert there are 12,683 lanes: 8,813 single and 3,870 double, in 3,799 charts. The timer rule would fire on **none** of the 3,870 Expert double lanes. Their median gap between lit notes is 0.077 s and the longest is 0.676 s.

It would fire on 19 double lanes at Easy, Medium and Hard, in 9 charts. They are listed in `play/library_double_lane_breaks.csv`. Many are tiny lanes of 2 to 20 notes with one lit pad and a gap over a second, such as the 2-note Medium lane in Dream Theater's *As I Am* and the 20-note Easy and Medium lanes in Architects' *Seeing Red*.

The stored results in the database are all Expert (37,692 rows, every chart mode starts with "Expert"), and the settings file has `view_difficulty=Expert`. So none of those 19 lanes has been analyzed by Hydra. Hydra can analyze the lower difficulties if the setting changes.

Here is every lane the timer would fire on, one row per lane. A chart appears more than once when several of its lanes qualify or when more than one difficulty does. The longest gap is between lit notes, and a combo break can come sooner than that gap suggests, because a round needs both pads. No Expert lane is in the list.

| Artist | Song | Charter | Difficulty | Lit pads | Notes in lane | Longest gap between lit notes (s) | First combo break (s into song) |
|---|---|---|---|---|---|---|---|
| An Endless Sporadic | Impulse | armchairsum65, YourDailyFill | Medium | GY | 4 | 0.6 | 65.55 |
| An Endless Sporadic | Impulse | armchairsum65, YourDailyFill | Medium | GY | 18 | 0.6 | 258.646 |
| An Endless Sporadic | Impulse | armchairsum65, YourDailyFill | Easy | GY | 4 | 0.6 | 65.55 |
| An Endless Sporadic | Impulse | armchairsum65, YourDailyFill | Easy | GY | 6 | 0.514 | 151.779 |
| An Endless Sporadic | Impulse | armchairsum65, YourDailyFill | Easy | GY | 5 | 0.834 | 195.379 |
| An Endless Sporadic | Impulse | armchairsum65, YourDailyFill | Easy | GY | 17 | 1.2 | 258.646 |
| Architects | Seeing Red | sylvexite | Hard | G | 37 | 6.402 | 9.713 |
| Architects | Seeing Red | sylvexite | Medium | G | 20 | 6.585 | 9.53 |
| Architects | Seeing Red | sylvexite | Easy | G | 20 | 6.585 | 9.53 |
| Counterparts | Heaven Let Them Die (Full EP) | FigNeutered, SirMonkfish | Hard | G | 11 | 1.477 | 46.066 |
| Dream Theater | As I Am | Onyxite, Grinnz | Medium | G | 2 | 0.923 | 443.715 |
| Halcyon Way | Desecration Day | armchairsum65, ThatAuthoringGroup | Easy | G | 3 | 0.968 | 255.266 |
| Leprous | Forced Entry | armchairsum65, Doxophobia | Medium | B | 11 | 1.107 | 28.52 |
| Leprous | Forced Entry | armchairsum65, Doxophobia | Easy | B | 11 | 1.107 | 28.52 |
| Mastodon | Stargasm | MMSRhino | Medium | GY | 19 | 0.513 | 270.475 |
| Mastodon | Stargasm | MMSRhino | Easy | GY | 19 | 0.513 | 270.475 |
| Opeth | The Baying of the Hounds | armchairsum65, Sideshow, JackDaMaster, Parama | Medium | GY | 12 | 0.625 | 7.875 |
| Opeth | The Baying of the Hounds | armchairsum65, Sideshow, JackDaMaster, Parama | Easy | GY | 12 | 0.625 | 7.875 |
| Smashing Pumpkins | Here Is No Why | armchairsum65, DemonUnicorns | Easy | G | 2 | 1.101 | 220.448 |

The full list of every chart with any lane (3,801 charts, with single and double lane counts per difficulty and how many double lanes would trip the timer) is `play/library_all_charts_with_lanes.csv`.

Caveats: the lit pads come from the census script's copy of Hydra's rule, not from Hydra's parser. The timer rule rests on my reading of the code and has not been seen in the game. Both could shift a count, but the Expert answer is zero by a wide margin.

## The .mid reader, checked

The .mid reader maps a lane marker's note-on velocity to a difficulty at 0x2155378 to 0x2155442 (`f2155200_mid_reader_lane_case.asm`), with a second copy at 0x2156E68. Velocity 21 to 30 gives Easy, 31 to 40 Medium, 41 to 50 Hard and 51 or more Expert. Velocity 20 or less also gives Expert, because the compare falls through to the same value. That matches D112 item 2. Pitch 126 makes a single lane and pitch 127 a double lane. The lane object is made with its start, its length and the kind (`lane_ctor_2116390.asm`). I did not see the note-off pairing, so "length comes from the note-off" rests on the reader receiving a start and a length.

## What is still not traced

- Who calls the generic input-processing method (0x100E0D0), which would prove each press runs at its own timestamp.
- The wrong-pad path in `f20deaa0.asm` and the first branch of `0x20DCAD0` both treat the fill-end note leniently when Star Power is available. Whether that matches what Hydra assumes about activation was not checked. It is not a lane rule.
- The semantics of the role-1 skip, beyond the enum names.
- Nothing was run in the game. Every claim rests on reading listings and on one small simulation.

## Decisions if Hydra should model it

1. **The grant rule: model it?** My recommendation is no. It changes no score and opens no new timing.
2. **The double-lane timer: model it?** It is the one lane rule that can lower a perfect score. In `testdata` it never fires. Options: leave it out and say so in one public sentence; or add a check that flags a double lane whose lit notes leave a 0.75 s stretch without both pads (no score change, a warning only); or apply the combo break in the engine, which changes stored results and bumps the results stamp.
3. **Library count: done.** No Expert double lane in the library trips the timer, so for the Expert difficulty Hydra analyzes today the timer does not matter. It would matter only for 19 lower-difficulty lanes in 9 charts that Hydra has never analyzed. That favors option 2's first choice, leave it out and say so.
4. **A game test for the timer.** A chart with a double lane whose pads alternate every 0.5 s, played by the bot or by hand, would show the break at once. It would confirm the rule before anyone changes the engine.
5. **Public wording.** One sentence in the user guide and `docs/differences-from-public-hydra.md`. It goes through the public-doc review.
6. **Fix the lane count.** D112 and this folder's README say 50 lanes, and the tool prints 49. Correct it or say which file made it 50.
