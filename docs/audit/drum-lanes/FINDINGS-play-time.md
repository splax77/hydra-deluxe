# What Clone Hero does with drum lanes during play (2026-10-10)

Question from D112: the game reads lane flags while you play, and Hydra's engine does not. Does Hydra need to copy that? This is a read-only investigation of Clone Hero 1.1.0.6142's `GameAssembly.dll`. No Hydra code changed. Listings are in `play/`, next to the earlier ones in this folder. All addresses are RVAs.

## The short answer

The lane path gives a lane note the same score, the same combo and the same Star Power credit as a normal hit. It only decides when a note you did not press in time still counts as hit. A player who hits every note never uses it. So Hydra's best score does not change.

The one place it could matter is how late a lane note can be hit. A normal hit must land inside the hit window. A lane hit that the game grants on its own happens after the window has closed. That could stretch a late squeeze (SqOut) by a small, unmeasured amount, on lane notes only. In the testdata corpus, one lane out of 49 touches a Star Power phrase boundary, so the exposure is small. I did not measure the real library.

I recommend: change nothing in the engine, say in one public sentence that Hydra ignores lane leniency, and measure the late gap in the game before anyone decides to model it. The decisions are listed at the end.

## How a lane note is marked

The note flag word at note+0x7C carries three lane bits. 0x4000 means the note is in a single lane. 0x8000 means a double lane. 0x10000 means it is the last note of the lane. The lane builder sets them (`f20d12c0.asm`, at 0x20D16BA, 0x20D16DB and 0x20D16F8).

I scanned the whole code section for instructions that test or set those bits on a note's flag word. Only the lane builder and the drum engine (0x20DC000 to 0x20DF700) use them. The base hit routine (0x20F63F0), the score function (0x20DEEC0), the Star Power code and the star-base code never read them. The scan sees `test`, `and`, `or` and `bt` with the bits written into the instruction. It cannot see a flag word loaded into a register and masked later, so "only these" is strong but not proven. The scripts are `play/flagscan_dword.py` and `play/flagscan_byte.py`.

## What the engine keeps for a lane

These are fields on the drum engine object. The roles come from how the code uses them. The game's names are scrambled, so these are my labels.

The pad mask at +0x2C4 holds the pads the current lane lights. +0x2C8 says whether it is a double lane. +0x2C6 holds the pads not yet pressed in the current round. +0x2D0 is the time of the last press on a lane pad. +0x2E0 is when the round started. +0x2D8 is when the last lane note was hit. +0x2E8 counts hits the game granted without a press. +0x2EC counts hits on notes outside the lane while it is open. +0x2F1 says the lane has finished. The constants are 0.075 s, 0.5 s and 0.75 s, plus the limits 2 and 4. All come from the class's constant list in the dump and from the compares below.

## The lane rule that matters: a miss becomes a hit

Each frame the engine walks the notes that are past the window (`f20f5810_base_update.asm`). A note counts as past when the engine time is more than the hit window after the note's time (the compare at 0x20F59E7). For each such note the engine calls the miss routine, vtable slot 4. The call is at 0x20F5C1C.

The drum engine overrides that slot at 0x20DCAD0 (`f20dcad0.asm`). A normal miss runs the base miss code at 0x20F3610 (`f20f3610_base_miss.asm`). The override can end in the hit routine instead. A tail jump through vtable slot 11 does it (0x20DCC11).

The note is turned into a hit when it is on a pad in the lane mask and one of two things holds:

- **The grant budget is not used up.** Fewer than 2 grants since the last press on a lane pad for a single lane, fewer than 4 for a double lane (0x20DCB1F to 0x20DCB39). Each grant adds one (0x20DCBF1).
- **A lane pad was pressed in the last 0.075 s.** The engine time minus the last lane press time must be under 0.075 s (0x20DCC28 to 0x20DCC44). This path does not spend the budget.

Every press on a lane pad sets the grant count back to zero and records the press time. That happens in the press handler (0x20DECF3 to 0x20DED12). So during a roll you can drop a couple of notes and the game still credits them.

A missed note on a double lane that is not yet in the mask joins the mask and is granted as well (0x20DCB3F to 0x20DCBF1). A real miss on a non-kick note that fails both tests clears the lane (0x20DCC46 to 0x20DCC6A).

## What the other lane-aware code does

The press handler (`f20deaa0.asm`, called from 0x20DDC80) does two lane-specific things. Pressing a lane pad registers the pad with the lane (the join routine 0x20DE420). Pressing the wrong pad while a lane note sits in the window still hits that note: the handler picks the window note flagged 0x800 and hits it when it has a lane bit (0x20DEDC3 to 0x20DEE29). That is, inside a lane the pad does not have to match. The window is the same as for normal hits, so this never hits a note earlier than a normal press could.

A press on a non-lane pad while a lane is open ends the lane (`f20de9a0.asm`). It also calls vtable slot 20 and adds one to a counter at +0xB0 (0x20DEA48 to 0x20DEA4E). I did not trace slot 20. It is probably the overhit penalty, and a perfect player never reaches it.

Hitting three notes outside the lane while it is open also ends the lane (0x20DE79C to 0x20DE7B8). The last lane note records its hit time (0x20DE7D7), and the lane clears 0.5 s later (`f20de5b0.asm`, `f20ddec0.asm`). A double lane that is not finished 0.75 s after the round started is cleared and calls slot 20 as well.

Four lookalike copies of the press handler exist at 0x20DCC90, 0x20DD190, 0x20DD700 and 0x20DEFF0, each with different flag masks. I found no call to any of them, so I treated them as dead code. That is not proven.

## Lane hit versus normal hit

The hit routine is the same for both. The press handler calls vtable slot 11 at 0x20DED52 and 0x20DEE29. The miss override tail-jumps to the same slot at 0x20DCC11. For the drum engine, slot 11 is 0x20DE770 (`f20de770.asm`).

That routine calls the base hit code at 0x20F63F0 (`f20f63f0_base_hit.asm`), then adds the score through slot 6 with a base of 50 (0x20DE964 to 0x20DE98E). The score function at 0x20DEEC0 adds 15 for a cymbal and doubles for the second flag, and has no lane branch. Here is what each part does.

- **Score:** identical. Same 50, same cymbal bonus, same ghost and accent doubling, same multiplier.
- **Combo:** identical. The base hit code adds one to the combo and updates the best combo (0x20F6524 and 0x20F660E).
- **Star Power phrase:** identical. A note that ends a phrase triggers the same phrase check (0x20F6719 to 0x20F6738).
- **Hit time:** this is the one difference. The base hit code reads the engine clock at +0x100 and compares it with three stored times at +0x110, +0x120 and +0x128 and with the note's time (0x20F654F to 0x20F65FB). I read these as the Star Power boundaries, but I did not name them. A pressed hit uses the clock at the press. A granted hit uses the clock at the moment the note was found to be past the window.
- **Pad:** the pad of the pressed key does not matter inside a lane, as above.

## Does it change anything Hydra reports?

**Best score and combo.** No. The grants only rescue notes the player did not hit in time. A path that hits every note gets the same score either way.

**Hit windows.** The lane path never hits earlier than the normal window. It can hit later than any press could. A granted hit lands once the engine time is past the note's time plus the window. A press must land inside that window. So a lane note has a later last-possible hit time than a normal note. Whether that is the same instant or up to one frame later depends on how the engine steps its clock. The step loop at 0x20F4950 sets the clock from a next-event time returned by 0x20F6FF0, so it may land exactly on the boundary. I did not trace that function. The size of the gap is therefore unknown.

**Squeezes.** Only a late squeeze on a lane note could be affected: a SqOut that needs the note hit after the Star Power end. The note would have to be a lane note on a lit pad. The player would leave it unpressed, with a lane press within 0.075 s before, or with grants left. The gain is at most that unmeasured gap. Early squeezes (SqIn) and activation timing before the note are not affected, because no lane rule hits anything early. The hit window itself is also not fixed in the game. It changes with the gaps between notes (see the dynamic input probe). Hydra's 85 ms is a setting, not the game's value, so the lane gap is small next to that existing approximation.

**Best all-0 path.** No. That path needs every timing at 0 ms or easier, and the lane rule only extends the late side.

## How many lanes could matter in the corpus

A scout counted the lanes in `testdata/input` (115 chart files, 44 .chart and 71 .mid) with its own script, checked against `lanediff.py`. The data is in `play/lanes_testdata.csv` and the script is `play/lanecount.py`.

There are 49 Expert lanes in 19 charts: 42 single and 7 double. The 4 Easy, Medium and Hard lanes are in 3 .chart files that also have Expert lanes. D112 and the README say 50, but `lanediff.py` prints 49. The 49 is the tool's count.

A lane holds between 3 and 108 non-kick notes. The median single lane has 6 and the median double lane 42. No lane has fewer than 3. Among the 579 notes in single lanes, 25 sit on a pad that the lane does not light. Those are never granted. All 312 notes in double lanes are on lit pads.

Only one Expert lane has a Star Power phrase boundary at its start or end or inside it: a single lane in the Zildjian LIVE! .mid, ticks 208320 to 209055. Three more single lanes sit fully inside a phrase with no boundary: CHON - Book, black midi - Sugar/Tzu and SoundHaven - Focus. So in the corpus, lane leniency could touch a Star Power squeeze in at most one lane. I did not check whether the squeeze note itself is a lane note.

The scout applied Hydra's lit-pad rule from `song.cpp` in its own script and did not run Hydra's parser, so the lit and unlit counts are unchecked against the real parser. I spot-checked only the lane total (49, matching `lanediff.py`).

I did not count the real library. That would need your yes in chat, and the answer would be a count of stored squeezes whose note falls in a lane.

## What I did not trace

- Slot 20 (0x20F4B60), the call the lane failure and the overhit use.
- The meaning of flag 0x800 on a note. It picks the note the wrong-pad rule hits. The lane builder does not set it.
- The hit window function (0x20F3A90) and the clock stepping (0x20F4950, 0x20F6FF0). The gap above depends on them.
- The .mid reader's lane case, as the folder README says.
- Nothing was run in the game. Every claim above rests on reading listings.

## Decisions if Hydra should model it

1. **Model it at all?** My recommendation is no for score and combo, because there is nothing to model. The open part is only the late gap.
2. **Measure the late gap first?** A short game test would settle it: a chart with a lane, the bot or a scripted press stopping, and the grant time read against the note time. Without it, any model would be a guess. This was not asked for here.
3. **Count the library?** Ask for permission to count stored squeezes whose note falls in a lane, so you can see whether the late gap could ever show up.
4. **If the gap is real and the count is above zero, what to change?** Either extend the late side of the squeeze budget for lane notes by the measured gap, or only mark such squeezes with a note in the details. The first changes stored results and bumps the results stamp.
5. **Public wording.** One sentence in the user guide and `docs/differences-from-public-hydra.md`: Hydra assumes every note is hit and does not model the game's lane leniency. It goes through the public-doc review.
6. **Fix the lane count.** D112 and this folder's README say 50 lanes, and the tool prints 49. Correct it or say which file made it 50.
