# The automatic run that matches Hydra's optimal score (2026-11-06)

## The plain version

The user ruled in session 70bfa33d: "our work is not finished until you can replicate hydra's optimal score in an automatic run. now that you know the gaps, you need to figure out how we can accomplish that." (Quoted by the main session from that session's transcript; the session has no card yet.)

The target is the Senri chart, `C:\Clone Hero\songs\Hydra Probe\Ladies Talk fill check`, played as `ProDrums,Expert` on the `ch` server's virtual kit. The run passes when the game's own stats file reads `score` 1,229,730, which is Hydra's top path for that chart.

The last run (LS-D2d, 2026-11-06) scored 900,870. The 325,300 gap splits into four known pieces, and the top path needs one more thing the run did not try. In the order of points:

1. 320,415 points: accents and ghosts. The kit's presses carried no velocity, because the game reads a hit's strength only from a registered MIDI kit or from a custom controller whose id is 6. Our kit is controller 0. Fix: make the kit controller 6, and press each gem at the strength its `dynamic` asks for.
2. 3,560 points: the top path declines the fill at chord 405 and takes 8 activations, not 9. Fix: do not press the gem that carries the activation flag at chord 405.
3. 2,245 points: chords 0 to 6 were never pressed, because four separate tool calls ran before the first press at 5.40 s. Fix: one chained call that sets the kit up and starts the player before the first note at 4.14 s.
4. 1,980 points: the first solo paid 76 of 83 notes, a consequence of item 3. The chained start fixes it.
5. 660 points: chords 2044 and 2320 sit exactly on a Star Power end and the game paid them outside. Hydra pays all 8 end chords inside (SP-33, an assumption Hydra holds and nobody has checked). Fix: press each end chord early enough to land in a frame before the end. If the game still pays them outside, Hydra's assumption is wrong for this case; that goes to the user and changes neither Hydra nor the target quietly.

The work goes in two waves of code, with one short probe launch between them. Wave 1 builds the id-6 kit, the per-gem strengths and two engine read-outs, and the probe launch checks the three things no code reading can settle: that the kit reaches id 6 and the gate accepts it, that a ghost-strength press still counts as a press, and that a dynamic then registers. Wave 2 builds the chained start, the fill decline, the end-chord lead and the exact score check, in parallel. Then the live runs: the chained start on Window Map (which is LS-K4), the decline and the lead on Senri, and the full run against 1,229,730 (which also holds LS-K5's read).

Every live run has a cap on tries. A run that fails stops and goes to the user with its lines. Nothing loops "until it passes".

## Rulings that bind this plan

- "our work is not finished until you can replicate hydra's optimal score in an automatic run. now that you know the gaps, you need to figure out how we can accomplish that." (session 70bfa33d, quoted by the main session). The goal.
- "Keep Player1" (session 70bfa33d, `docs/agents/rulings/clone-hero.md` line 5). `KIT_PLAYER` stays `Player1`; a human launch passes `player="System"` to the kit setup.
- "why does it matter if the game is focused briefly?" (2026-10-31, `docs/agents/clone-hero-mcp.md`, "Safety rules"). The foreground count is information, not a pass condition.
- "the code is the source of truth but from experience i can tell you dynamics only work with a real drum kit. keyboard inputs never register dynamics." (session 6a549de0, SC-14's Ruling line). This plan makes the virtual kit look like the thing the game grades; it does not change what Hydra prices (Q-51 stays open).
- The user allowed method calls and field writes from the Frida agents on 2026-10-30, and the one-byte loader patch on 2026-10-31 (`docs/agents/clone-hero-mcp.md`, "Play stays offline"). The kit changes here stay inside that: more `CreateCustomController` calls, the same setters.

## What was checked in the code, and how

The handoff `docs/handoffs/2026-11-06-optimal-score-plan-handoff.md` was run through `py -I tools/check_handoff.py`: 19 ok, 6 not-pushed as its Claim column says, 0 missing, 13 manual. This worktree's branch holds `main` at d8f94f72 merged in, which is the Q-51 fix the brief names. The manual rows were read as follows.

The gate. `docs/clone-hero/code/6142/lane_velocity_337E10.asm` lines 42 to 54 load the player's slot (`[rbx+0x88]`), the slot's Rewired player (`[rcx+0x18]`), that player's controller helper (`[rdx+0x50]`), and call `ContainsController` with type 0x14 (Custom) and id 6. Lines 33 to 41 are the MIDI list check on the same slot. Line 64 is `GetButtonDown`, and lines 79 to 103 compute round((GetAxis - 0.5) * 2 * 127). The slot's +0x18 is the Rewired player the command-line loader fills with the System Player (IN-66). So the gate asks the System player, which is the player the kit already presses through (LS-D1d).

The hit handler. `docs/clone-hero/code/6142/note_hit_dynamics_20DE770.md` lines 6 to 10: one signed byte per lane, the ghost path marks "performed" when the byte is at least 0 and below the threshold at engine +0x2C0, the accent path when it is at or above it, and a negative byte marks neither. Line 9: with bit 0x10 of the flags word at engine +0x198 set, an unperformed dynamic returns before scoring. The plain-gem path (lines 62 to 71 of the listing) never reads the byte, so a plain gem's strength does not matter.

Where the threshold comes from. `docs/clone-hero/code/6142/engine_ctor_20DF680.asm` lines 51 to 53: `mov rax, [rdi+0x18]; shr rax, 0x20; mov [rbx+0x2c0], eax`. The threshold is the int at +0x1C of the 0x48-byte struct the `DrumsPlayer` method `0x33C560` builds (`player_kind_11676A0.md`, "Where the record goes"). What fills that int was not read. The plan reads the value live rather than chasing it.

The game's own bot. SC-13's Evidence line: the bot writes 127 for an accent and the threshold minus 1 for a ghost, and the MIDI handler turns a velocity v into axis 0.5 + v / 254. This plan copies those two values, so it adds no number of its own for strengths.

The target. `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\senri-acts-true-ends.json` is Hydra's score of the 9-fill path, 1,226,170, with per-gem `dynamic` fields ("none", "ghost", "accent"), `is_fill` per chord and `sp_points` per gem. The brief's main session ran `hydra_replay dump` and `score --path ... --index 0 --prodrums 1 --bass2x 1` and got 1,229,730 with the 8 activations; that file is not in the run folder, so task T0 writes it. `tools/replay.cpp` lines 98 to 116 hold those options.

Is 1,229,730 the current engine's number? `build-cpp\Release\hydra_replay.exe` is dated 2026-10-31 13:20. `main` at that time was 853093cd (13:18). `git log 853093cd..main -- tools/replay.cpp tools/replay_json.cpp src/core src/search src/parse` is empty: no scoring code changed after the build. The number stands. T0 still rebuilds `hydra_replay` with `build_cpp.ps1` before it writes the target file, so the file carries a fresh stamp; that is one warm build and costs nothing.

The 8 end chords. Reading `senri-acts-true-ends.json` at the 8 target end ticks: chords 268 (tick 36960), 566 (74640), 819 (107040), 1019 (132960), 1649 (210000), 1843 (233520), 2044 (259440) and 2320 (294960). Each is a kick plus a cymbal at multiplier 4, except 1649 (kick plus an accented cymbal) and 2320 (one ghosted red). Their `sp_points` sum to 3,880. That is what rides on the end chords: 460 each for six of them, 720 for 1649 and 400 for 2320. The LS-D2d run lost 660 on two of them (460 for 2044 and 200 for 2320, the ghost unpaid), which matches the handoff's figure. T0 recomputes the 3,880 from the 8-activation file, because the multiplier at each end is the 9-fill file's.

Window Map. `C:\Clone Hero\songs\Hydra Probe\Window Map\notes.chart`: resolution 480, one tempo 125 BPM, offset 0, and `song.ini` delay 0, so one tick is one millisecond. Lines 22 to 163 hold 142 notes; the 13 at ticks 3000 to 4600 fall before 5,400 ms. So the handoff's "13 notes before 5.4 s" is right for Window Map. Senri has 12 gems in 8 chords before 5,400 ms (`senri-acts-true-ends.json`), and the run pressed chord 7 at 5,398 ms, so it lost the 11 gems of chords 0 to 6, as the run's replies say.

LS-K4 and LS-K5. Both stand in `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md` lines 299 to 309 with their calls, and in `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md` lines 220 to 226 with their pass lines. No `ls-k4` or `ls-k5` folder exists under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\`. The LS-K5 read, `ch_sp_between(114778.9, 166544.0)`, was answered from LS-D2d's rows after `ch_stop` (`replies.txt` lines 70 to 74): no activation inside, 11,542 rows, meter full at 14,400.

The kit agent. `tools/ch_mcp/agent/kit.ts` reads the kit's id from the object (`kitId`, line 237) and passes it to `GetMaps` (line 245) and `LoadMap` (line 346). No line compares it with 0. Setup finds the kit by tag (line 231), creates it from definition 6 (line 313), adds it to the named player (line 325) and loads map category 1 layout 2 (line 345). Teardown destroys the one kit (line 657). `ALLOWED_CALLS` (lines 30 to 46) already holds `CreateCustomController`, `DestroyCustomController` and `get_id`. `press` writes `SetAxisValueById(id, value)` per element with the value the message carries (lines 379 to 387), so a per-element strength needs no agent change; `press_message` in `tools/ch_mcp/kit_tools.py` line 213 is what writes `KIT_AXIS_VALUE` for every pad.

The auto-player. `tools/ch_probe/experiments/play_chart.py` line 99: `load_dump_chords` keeps each gem's `color` and `cymbal` only. Line 65: `PRESS_LEAD_MS = 2.0`. `tools/ch_probe/experiments/live.py` line 151: `START_LEAD_MS = 150.0`, and `first_note_index` skips any chord less than that ahead of the clock. `tools/ch_mcp/game_tools.py` line 526: `ch_play_chart` loads the chords and starts `live_play`, which runs its own engine search (`engine_finder.ENGINE_SEARCH_CAP_S` is 120 s, `tools/ch_probe/engine_finder.py` line 40). `tools/ch_mcp/sp_tools.py` line 325: the watch runs its own search too.

The compare. `tools/ch_mcp/compare_scorestats.py` passes or fails on `total_notes` and `sp_phrases_total` only; `base_score` and `note_score` are reported, and the game's `score` is never read.

## Prior art: how Rewired numbers a controller

The web gave nothing on it. Rewired's Custom Controllers page (`https://guavaman.com/projects/rewired/docs/CustomControllers.html`) says controllers are created at play time "from the pre-defined definitions in the Rewired Input Manager" and that a Tag tells instances apart; it says nothing about ids, destruction or limits. The API page for `ReInput.ControllerHelper.CreateCustomController` (`https://guavaman.com/projects/rewired/docs/api-reference/html/M_Rewired_ReInput_ControllerHelper_CreateCustomController.htm`) lists one parameter, `sourceControllerId`, and no remarks. Two web searches for Rewired controller id numbering returned nothing about Rewired; the only counter-based scheme they turned up was another project's. So how a custom controller gets its id is `unchecked` (row P9), and task T1 reads the game's own copy of `CreateCustomController` (Rewired_Core, `dump.cs` line 309950) with `tools/ch_mcp/chdis.py` before the probe runs. Whatever the reading says, the design below works for both numbering schemes.

## The design, point by point

### 1. Reaching id 6

Three routes were weighed. First, keep six placeholder controllers alive: create six custom controllers from definition 6 with their own tags, never hand them to a player, then create the kit, which gets id 6 whether Rewired counts up forever or hands out the lowest free id. Second, the handoff's route: create and destroy six, which works only if Rewired never reuses an id. Third, the MIDI list: add the slot to `MIDIInputManager`'s list with a hand-made entry. The handoff describes that entry (+0x48 byte 0, +0x30 a controller) from a scout's reading with no listing in the repo, so it is unchecked, and it writes a game field directly, which no agent does today. Hooking the helper at 0x337E10 to return a velocity is the last resort, because it changes what the game's code does for every hit. A virtual MIDI driver is a machine install and was recommended against; that was the session's advice, not a ruling.

The plan takes the first route. What the six idle controllers cost: Rewired holds six more objects and updates their (never written) element values each frame, which is the same work the one kit already causes; no player reads them, because they are never added to one; they are not saved, because runtime controllers are not part of the saved maps, and the registry restore puts the Rewired key back anyway. What they could break: a later game-made custom controller would take id 7 or higher, which only matters if the game makes one, and no MIDI device is present; and teardown must destroy all seven, else "kit gone" would pass while six placeholders stay. T1 makes teardown destroy each and check each. If the probe shows the kit at an id other than 6, setup refuses and tears everything down, and the reply says which ids the seven got; that reply is what the user sees.

### 2. The gate on the System player, and a kit at id 6

The gate reads the slot's own Rewired player (+0x18 of the slot, IN-66: the System Player on a command-line launch) and asks its controller helper `ContainsController(Custom, 6)`. The kit is added to that player today (`ch_kit_setup(player="System")`, LS-D1d passed). So the kit needs only the id. Whether `ContainsController(type, id)` matches by the controller's own id within its type is Rewired's semantics, which nothing in the repo proves (row P10, unchecked); the probe settles it, because a dynamic registers only if the gate passes.

Nothing in the kit code assumes id 0: `kit.ts` passes `kitId(kit)` wherever an id is needed, and `kit_tools.py` only prints the id. T1 adds a grep to its review step for a literal 0 against the kit's id anyway, and the probe's press read-back (LS-O1c) shows the pads still reach the player at id 6.

### 3. The Star Power ends

All 8 target ends sit exactly on a chord, and 3,880 points ride on them. SP-28 (proven) says the end check turns Star Power off in the first frame whose clock is at or past the end. SP-29 (unknown) is whether a hit in that same frame is scored before or after the check. The LS-D2d run paid chords 2044 and 2320 outside with presses 3.8 and 6.8 ms before the watch saw the flag clear, with the press lead at 2 ms and a press landing up to one frame late (about 3 ms at 327 fps). So a press 2 ms before the chord is not enough. Two ways to land the hit in an earlier frame:

- Press the end chord early. The front of the hit window is 37.5 ms in normal mode (TM-18), so a press 15 ms early is a hit, and at 327 fps it lands about four frames before the end. This changes only the one chord's press time. It is the plan's first try; the 15 ms is question 3.
- Press the fill chord late, so the activation and its end move later (SP-39's model; the LS-D2d run saw each end 9 ticks, about 8 ms, after Hydra's end with activations 2.7 to 6.7 ms late). This moves the whole window, so any chord just after the fill chord would fall outside. It is the fallback, and it is not built unless the first try fails.

If the end chords still pay outside with the early press, the game decides "inside" by something other than the hit's clock, Hydra's SP-33 assumption is wrong for this case, and the most the run can reach is 1,229,730 less 3,880, which is 1,225,850. That is question 5 for the user. The plan never lowers the target on its own and never edits Hydra's rule.

### 4. Declining chord 405

Chord 405 is a kick plus a green cymbal (`n...N`), both plain, at tick 54480 (46,981.7 ms), the fill the top path skips. FL-09 (proven) says a landed fill sets the activation flag 0x800 on one note of the chord, and that note is the highest pad by the drum-lanes reading. FL-31 (proven as a code reading, never run) says a missed flagged note is granted as a hit through the plain hit handler when Star Power can activate, so it scores and does not activate. Which gem carries the flag and whether the grant really pays are the two things no run has shown.

The live step (LS-O3) tries this first: press the kick, do not press the cymbal. If FL-09's reading holds, the kick is an ordinary hit and the cymbal is granted; the game's running score steps by the chord's full value (115 at multiplier 4 is 460, from the target file) and the watch shows no activation at 47.0 s. If the score steps by less, or an activation appears, the step fails and the second try sends nothing for the whole chord. Either way the player sends at most one gem for a declined chord, and the pass line says which. A declined chord whose gems are not all granted costs the streak, which is why this is settled before any full run relies on it.

### 5. A ghost's press strength

The byte must be at least 0 and below the threshold, and the axis must still read as `GetButtonDown`. The plan copies the game's bot: a ghost at velocity threshold minus 1, an accent at 127, both from SC-13's evidence line, written as axis 0.5 + v / 254 (the MIDI handler's formula, same line). So an accent is axis 1.0, the value the kit writes today, and a ghost is 0.5 + (T - 1) / 254 where T is read from the engine at +0x2C0 before the first press. No new number is chosen; the one home of the formula is a function the executor adds to `kit_tools.py`, and the player derives each gem's value from the gem's `dynamic` and the live threshold. Whether a low axis still reads as a press is row P12 (unchecked) and probe step LS-O1c proves it with the press read-back on the System player.

Mixed chords: the handler keeps one byte per lane (+0x309 to +0x310), so an accent, a ghost and a plain gem on three lanes each get their own strength, and a plain gem's byte is never read. Two gems on one lane can only be two kicks on the kick pad, and all 631 kicks on this chart are plain, so no chord here needs two strengths on one lane; a chart that does is out of scope (item 1 of that list).

### 6. Bit 0x10 of the flags word

The flags word is the engine's own, at +0x198, the same word whose bit 0x1000 is precision mode (TM-18). The snapshot already reads it (`tools/ch_probe/engine.py` line 107, `flags=u32(C.OFF_FLAGS)`), but `ch_snapshot` prints only the window and clock lines. T6 prints the word in hex and says whether bit 0x10 is set, and probe step LS-O1b reads it. The cost of a wrong strength follows from that read: with the bit clear, a wrong strength scores the gem plain, 50 times the multiplier short (SC-13); with the bit set, the gem scores nothing and the streak does not step (SC-16), which on this chart would cost the multiplier for the next notes too. The plan's pass lines do not depend on which it is; the write-back to SC-16 does.

### 7. The engine behind 1,229,730

Covered above: no scoring source changed after the executable's build, and T0 rebuilds before it writes the target file anyway.

### 8. Thirteen or twelve

Thirteen on Window Map, twelve on Senri, as the chart reads show. LS-K4's 142 is Window Map's note count, so its pass line stays 142 and needs the chained start to reach the 13 early notes.

### 9. LS-K5

Recommendation: accept it from the LS-D2d run. The step's question was whether the game activates anything in the window 114,778.9 to 166,544.0 ms when every fill chord is pressed, and the read said no activation inside, with the meter full the whole way. The target run (LS-O5) has no activation in that window either, and T5's compare repeats the read, so the user loses nothing by accepting now. This is question 1.

### 10. The handoff's open points

- Charts whose first note is under about 1.2 s, charts with kick dynamics, charts whose top path needs squeeze timing: out of scope (items 1 to 3), after Senri.
- The "end time" label beside "flag cleared" in `ch_sp_watch_status`: T5 renames it "end time at the flip" and its test pins the line. In scope, because the full run's report reads those lines.
- The stale comment above `KIT_PLAYER`: T1 touches `kit_tools.py` and fixes it, since the file is already open; its test is the existing one that reads `KIT_PLAYER`.
- `ch_status` showing a finished trace: out of scope (item 4); it misleads nobody in these steps, because no step here starts a trace.
- The second focus grab at 48.9 s: out of scope (item 5); the user ruled a brief grab does not matter, and the count is printed by every step here.
- `note_score` not matching Hydra's `notes` (SC-32 Unknown): out of scope (item 6); T5 compares the `score` total, which did match the running rebuild.
- `senri-acts.json` wrong in the run folder: T0 writes the target file beside it and `replies.txt` already says which file is right. No further task.
- Pushing: out of scope (item 7); the main session's call.

## Questions

1. **LS-K5.** Recommended: accept it from the LS-D2d run, as design point 9 says, and let LS-O5's repeat of `ch_sp_between` confirm it. The other side: run LS-K5 on its own, which costs one more 4.5-minute launch and tells us nothing the LS-D2d rows did not.
2. **The id-6 route.** Recommended: six placeholder controllers kept alive for the run and destroyed at teardown (design point 1). The other side: the handoff's create-and-destroy six, which depends on how Rewired numbers controllers.
3. **The end-chord lead.** Recommended: 15 ms early for a chord whose tick equals an activation's end tick, from the 37.5 ms front window and the 3 ms frame. The other side: a smaller lead such as 8 ms, closer to how the run missed, or the late fill press of design point 3. The number is the user's to set; the code's one home is a constant beside `PRESS_LEAD_MS`.
4. **Declining chord 405.** Recommended: first try pressing the kick and leaving the cymbal to the grant, as FL-09's highest-pad reading says; second try sends nothing. The other side: send nothing first.
5. **If the end chords pay outside even when pressed early.** Recommended: stop, and bring the user the running-score lines for the end chords beside Hydra's `in_sp` for the same chords, so the user rules on SP-33 and on what the pass line becomes (1,225,850 is the number if all 8 pay outside). The other side: none that this plan may take alone; changing Hydra or the target is the user's call.
6. **Per-gem strength in wave 1.** The user was shown the strengths as a wave-2 item. The probe cannot show "a dynamic registers" without them, so this plan builds T2 in wave 1. Recommended: yes. The other side: a probe that only checks the id and the threshold, and a second probe launch later for the dynamics.
7. **Out of scope.** The list at the end needs the user's yes.

## Tasks

Every task runs in its own worktree on its own branch, gets one fresh review, and runs only its own tests. Wave 1 is T0, T1, T2 and T6 together. The probe launch (LS-O1) runs once wave 1 is merged. Wave 2 is T3, T4, T5 and T7 together. Tasks in a wave are independent and launch together (agent rule 6).

### T0: the target score file (main session, no code)

Rebuild `hydra_replay` with `build_cpp.ps1`, then run `hydra_replay dump` on the Senri chart's `notes.mid` and `hydra_replay score --chart <notes.mid> --path <dump> --index 0 --prodrums 1 --bass2x 1 --out senri-top.json`, writing the file into `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\` beside `senri-acts-true-ends.json`. Acceptance: `final.total` is 1,229,730; the `activations` list has 8 entries with the act and deact ticks the handoff lists; chord 405 has `is_fill` true and is in no activation; the 8 chords whose tick equals a deact tick have `sp_points` summing to the number this plan quotes as what rides on the ends (3,880 from the 9-fill file; T0 reports the 8-fill file's sum and the plan's reviewer corrects the number here if it differs). The check is the command's printed total and a `py -I` one-liner over the JSON, pasted into the go request for LS-O1.

### T1: the id-6 kit (Opus)

Owns `tools/ch_mcp/agent/kit.ts`, `tools/ch_mcp/kit_tools.py`, `tools/ch_mcp/kit_methods.json` if a new method is needed, and their tests under `tools/ch_mcp/tests/`. Setup creates `KIT_PLACEHOLDERS` (6, from the gate's literal at `lane_velocity_337E10.asm` line 53, named there in the comment) placeholder controllers from the kit's definition with tags `KIT_PLACEHOLDER_TAG` plus an index, before it creates the kit, and never adds them to a player. The reply lists every id it got. `kit_tools.py` refuses a setup whose kit id is not `KIT_REQUIRED_ID` (6, same source) and tears down. Teardown destroys the kit and every placeholder and prints one check per controller; "kit gone" also checks that no placeholder tag remains. The stale comment above `KIT_PLAYER` is replaced with one that says the user kept `Player1` and a human launch passes `System`. Before the probe, T1 also reads the game's `CreateCustomController` (Rewired_Core, `dump.cs` line 309950) with `tools/ch_mcp/chdis.py` and writes what it finds about the id's source into the task report and, as an IN block, onto `docs/clone-hero/07-internals.md` (Unknown if the listing does not show it).

Acceptance (`py -m pytest tools/ch_mcp/tests/test_kit_tools.py tools/ch_mcp/tests/test_frida_attach.py -q`): a fake agent returning kit id 6 passes setup and the reply names six placeholder ids and the kit's id; a fake returning id 0 or 7 is refused with the ids listed and teardown is sent; teardown with seven controllers prints seven "destroyed" checks and fails "kit gone" when any tag remains; `ALLOWED_CALLS` still equals `kit_methods.json`; a grep test asserts no literal `0` is compared with the kit's id in `kit.ts` or `kit_tools.py`.

### T2: per-gem strength (Opus)

Owns `tools/ch_probe/experiments/play_chart.py`, `tools/ch_mcp/kit_tools.py` (the press message only; T1 and T2 touch different functions and the integrator merges), `tools/ch_probe/constants.py` and `tools/ch_probe/engine.py` for the threshold read, and their tests. `load_dump_chords` keeps each gem's `dynamic` beside its lane. `constants.py` gains `OFF_DYN_THRESHOLD = 0x2C0` (its facts row exists: SC-13 cites `#8255+0x2C0`), and the engine model reads it once when the player finds the engine. One function in `kit_tools.py`, say `pad_axis(dynamic, threshold)`, is the one home of the strengths: 1.0 for "accent" and for "none", and 0.5 + (threshold - 1) / 254 for "ghost", both from SC-13's evidence line, which the comment names. The press message carries one value per element from that function; buttons stay `True`.

Acceptance (`py -m pytest tools/ch_mcp/tests/test_kit_tools.py tools/ch_probe/tests/test_play_chart.py -q`, or the test files that exist for those modules): chords loaded from a fixture carry "ghost" and "accent"; `pad_axis("accent", T)` is 1.0 for any T; `pad_axis("ghost", T)` gives a byte of T - 1 when run back through round((axis - 0.5) * 254) for T in 1, 64 and 127; `pad_axis("none", T)` is 1.0; a press message for a chord of a ghost red, an accented yellow cymbal and a plain kick carries three different values with the kick `True`; a threshold of 0 raises, naming the field, since no ghost is possible then.

### T6: the probe's read-outs (Opus)

Owns `tools/ch_mcp/frida_tools.py` (`ch_snapshot`), `tools/ch_mcp/kit_tools.py` (`ch_kit_press`'s new `value` argument), and their tests. `ch_snapshot` prints the flags word in hex, says whether bit 0x10 is set, and prints the dynamics threshold at +0x2C0. `ch_kit_press` takes an optional `value` for the pads of that press, so the probe can press one pad at ghost strength by hand with the existing `read` list.

Acceptance (`py -m pytest tools/ch_mcp/tests/test_frida_tools.py tools/ch_mcp/tests/test_kit_tools.py -q`): a fake snapshot with flags 0x1010 prints "bit 0x10: set" and one with 0x1000 prints "clear"; the threshold line names the value and the offset's constant name; `ch_kit_press("red", value=0.6)` sends 0.6 for element 0 and `True` for a button; a value outside 0 to 1 is refused.

### T3: declining a fill (Opus)

Owns `tools/ch_probe/experiments/play_chart.py` and `tools/ch_mcp/game_tools.py` and their tests. The player takes a list of chord indexes to decline and, for each, which gems to send: `kick_only` or `nothing` (question 4). `load_dump_chords` gives a declined chord the lanes the mode keeps. The row for a declined chord says "declined" in its result column, and the hit count does not count it as a miss. The decline list for Senri is derived once from the target file: every chord with `is_fill` true whose tick is no activation's `act_tick` and whose fill the top path does not take; for Senri that is chord 405 and the other 23 fill chords the path passes over. Note the difference: a fill chord the path passes over while Star Power is not ready is just a chord, and pressing it activates nothing; the game grants a missed flagged note only while Star Power can activate (FL-31). So the player declines a fill chord only when the target file's `cum` shows the meter at or above half there, which on Senri is chord 405 alone. The function that decides is the one home of that rule and its test pins chord 405 from the fixture.

Acceptance (`py -m pytest` on the module's test files): the decline list from a fixture of the target file is [405]; a declined chord in `kick_only` mode sends the kick lane only; in `nothing` mode it sends no press and the row says declined; the run's `hit` count excludes declined chords; `ch_play_chart` and the chained tool refuse a mode other than the two names.

### T4: the chained start (Opus)

Owns `tools/ch_mcp/game_tools.py`, `tools/ch_mcp/server.py`, `tools/ch_mcp/sp_tools.py`, `tools/ch_mcp/kit_tools.py` (a function to run setup from another tool), and their tests. One new tool, `ch_run_chart(song_folder, score_json, kit_player, decline_mode, sp_end_lead_ms)`, does in one call what LS-D2d did in four: attach, kit setup on `kit_player`, the Star Power watch and the auto-player, with one engine search shared by the watch and the player. Every part keeps its owner: the schedule stays in `play_chords`, the kit checks in `kit_tools.py`, the watch in `sp_tools.py`. The reply has one line per part, and a part that fails tears the kit down and stops the call there, naming the part. `first_note_index` is unchanged: a chord less than 150 ms ahead of the first clock read is still skipped, and the reply says which chord was pressed first.

Acceptance (`py -m pytest tools/ch_mcp/tests/test_game_tools.py tools/ch_mcp/tests/test_sp_tools.py -q`, or the files that exist): with fakes, the tool calls setup, watch and play in that order and the player and watch receive the same engine pointer; a failed setup stops before the watch and the reply names setup; a failed watch tears the kit down; the reply's first-press line names the chord index; the tool refuses while a play session runs.

### T5: the exact score check (Opus)

Owns `tools/ch_mcp/compare_scorestats.py`, `tools/ch_mcp/sp_tools.py` (the label), and their tests. The compare gains three checked lines: `score` against Hydra's `final.total`, `accent_notes_hit` and `ghost_notes_hit` against the counts of gems with `dynamic` "accent" and "ghost" in the file, and `sp_activations` against the length of `activations`. It keeps the two lines it has. The exit code is 0 only when every checked line passes. The activation line of `ch_sp_watch_status` says "end time at the flip" where it now says "end time", so the two times cannot be read as one.

Acceptance (`py -m pytest tools/ch_mcp/tests/test_compare_scorestats.py tools/ch_mcp/tests/test_sp_tools.py -q`): a stats fixture with score 1229730, 368 accents, 726 ghosts and 8 activations against a target-file fixture passes all five lines and exits 0; the same with score 1229729 fails the score line and exits 1; 367 accents fails the accent line; 9 activations fails the activation line; the status fixture's activation line reads "end time at the flip".

### T7: the end-chord lead (Opus)

Owns `tools/ch_probe/experiments/play_chart.py` and its tests. A constant beside `PRESS_LEAD_MS`, `SP_END_LEAD_MS`, set to the number question 3 fixes (15 if the recommendation stands), with a comment naming the question. `load_dump_chords` marks a chord whose tick equals an activation's `deact_tick` in the same file, and `play_chords` uses `SP_END_LEAD_MS` for that chord's `lead_ms` and `PRESS_LEAD_MS` for every other. The press row records the lead it used.

Acceptance (`py -m pytest` on the module's tests): from the target-file fixture the 8 marked chords are 268, 566, 819, 1019, 1649, 1843, 2044 and 2320; a fake clock run shows those chords pressed `SP_END_LEAD_MS` early and the others 2 ms early; the row names the lead; a lead above the front window of 37.5 ms (TM-18) is refused at load.

## The live steps

The main session runs these; a planning agent calls no `ch_*` tool. Before each run the user sees the step's `Calls:` line, each never-tried call and each unchecked row the step uses, and says go. Every run takes the DLL's MD5 before `ch_start` and after `ch_stop`, copies the watch CSV and the stats file out before `ch_stop`, and keeps a `replies.txt` in a dated folder under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`. A run at its cap stops and goes to the user with its lines; no step repeats on its own.

The probe, LS-O1a to LS-O1e, is one launch of Senri. Its sub-steps run inside that one game, so one launch answers the probe's three questions, and each sub-step tries one new piece.

### LS-O1a: the kit at id 6 (2 runs, 10 minutes)

Calls: `ch_start(<Senri folder>, human=True, player="ProDrums,Expert")` (seen: LS-D2d run 1), `ch_attach` (seen: LS-D2d run 1), `ch_kit_setup(player="System")` with T1's placeholders (never tried)

Uses rows P9 and P10. Pass: the setup reply lists six placeholder ids and the kit's id as 6, map category 1 layout 2 with all 10 bindings matching, keyboard and mouse off. Fail: the kit's id is not 6 (the reply lists the ids; the run ends with `ch_stop` and the ids go to the user with T1's reading of `CreateCustomController`), or setup faults (the reply's fault lines go to the user). Run 2 only after a fault whose lines name a fix in T1's code.

### LS-O1b: the threshold and the flags word (inside LS-O1a's run)

Calls: `ch_snapshot` with T6's lines (never tried)

Uses row P13. Pass: the reply prints the dynamics threshold as a whole number from 1 to 127 and the flags word with bit 0x10 named set or clear. Both numbers are written back: the threshold to SC-13 and the bit to SC-16 (as Proven in a live run, with this run's folder). Fail: the threshold reads 0 or above 127 (no ghost strength is possible or the field is not the threshold); the run goes on to LS-O1c but LS-O1d is skipped, and the user gets the number.

### LS-O1c: a ghost-strength press reads as a press (inside LS-O1a's run)

Calls: `ch_kit_press("red", value=<pad_axis("ghost", T)>, read="Red")` with T6's value argument (never tried)

Uses row P12. The value is the one T2's function gives for the threshold LS-O1b read. Pass: the reply's `seen` list shows "Red" turning true on the down frame and false on the up frame, as LS-D1b showed at 1.0. Fail: no true read; then the game does not read that axis value as a button, the low end of the ghost range is unusable, and the user gets the value and the threshold before any dynamics run (a second press at a higher ghost value is allowed once, with the value named, to show whether any ghost strength reads as a press).

### LS-O1d: dynamics register (inside LS-O1a's run)

Calls: `ch_sp_watch_start(<Senri folder>)` (seen: LS-D2d run 1), `ch_play_chart(<senri-top.json>, song_folder=<Senri folder>, route="kit")` with T2's strengths (never tried), `ch_play_status` (seen: LS-D2d run 1), `ch_wait_stats` (seen: LS-D2d run 1)

Uses rows P10, P11, P12 and P14. Only the strengths are new here; the start is still the separate-call start, so chords 0 to 6 are lost as before and the fills are all pressed. Pass: the stats file's `accent_notes_hit` and `ghost_notes_hit` are both above 0, and each equals the count of accent and ghost gems from chord 7 on in `senri-top.json` (the pass message states both numbers from the file). Fail: both 0 (the gate did not pass: the user gets the setup reply's ids and the stats line); or ghosts 0 with accents counted (the ghost byte is wrong: the user gets the threshold and the value); or `notes_hit` below the LS-D2d run's 2,985 (a wrong strength cost hits: the rows with `hit` 0 and the flags bit from LS-O1b go to the user).

### LS-O1e: nothing left behind (inside LS-O1a's `ch_stop`)

Calls: `ch_stop` with T1's seven-controller teardown (never tried)

Pass: seven "destroyed" checks ok, "kit gone" ok with no placeholder tag left, every file line "unchanged" or "restored", the registry line one of the two, and the MD5 unchanged. Fail: any other line; it goes to the user with the backup folder.

### LS-O2: the chained start on Window Map, which is LS-K4 (2 runs, 10 minutes)

Calls: `ch_start(<Window Map folder>, human=True)` (seen: LS-D0 run 1), `ch_run_chart(<Window Map folder>, <windowmap.json>, kit_player="System")` (never tried), `ch_play_status` (seen: LS-D2d run 1), `ch_wait_stats` (seen: LS-D2d run 1), `ch_compare(<stats copy>, <windowmap.json>)` (seen: LS-D2d run 1), `ch_stop` (seen: LS-O1e's passing run)

Uses rows P15 and P16. Window Map is plain Drums with 142 plain notes, 13 of them before 5.4 s, so it tests the start and nothing else; `windowmap.json` is the Hydra score file in `ls-d2d-prep`. Pass: LS-K4's own line, 142 hits with the first press at chord 0, the compare's five lines passing (0 accents, 0 ghosts, 0 activations), the frame rate line recorded. Fail: the first-press line names a chord above 0 (the reply's per-part times say which part took the time; they go to the user before any change to the lead); or hits below 142 (the rows with `hit` 0 and their `down_frame`, as LS-K4 says). Run 2 only after a timing fail whose part times name one slow part.

### LS-O3: declining chord 405 and the early end chords, on Senri (2 runs, 20 minutes)

Calls: `ch_start(<Senri folder>, human=True, player="ProDrums,Expert")` (seen: LS-D2d run 1), `ch_run_chart(<Senri folder>, <senri-top.json>, kit_player="System", decline_mode="kick_only", sp_end_lead_ms=<question 3>)` (never tried), `ch_play_status` (seen: LS-D2d run 1), `ch_sp_watch_status` (seen: LS-D2d run 1), `ch_wait_stats` (seen: LS-D2d run 1), `ch_compare(<stats copy>, <senri-top.json>)` (seen: LS-D2d run 1), `ch_stop` (seen: LS-O1e's passing run)

Uses rows P17, P18, P19 and P20. One call carries both new behaviours, the decline and the lead, because they are two arguments of the same tool; they are observed at different chords, so a failure names one. Pass, the decline: `ch_sp_watch_status` lists no activation between 46.0 and 48.0 s, and the watch CSV's `score` steps across chord 405 by that chord's `points` total in `senri-top.json` (the message quotes the number). Pass, the lead: at each of the 8 end chords the CSV's `score` steps by the chord's doubled value, that is `cum.total` of the chord minus the one before it in the target file, which already counts Star Power. Pass overall: both, plus `sp_activations` 8 and the compare's activation line. Fail, decline: an activation at 47.0 s or a short step at chord 405; run 2 uses `decline_mode="nothing"`, and a second fail goes to the user with both runs' lines. Fail, lead: any end chord steps by the single value; this is question 5, and it goes to the user with the 8 lines and Hydra's `in_sp` for the same chords, and run 2 is not started for it. If this run's stats also read `score` 1,229,730, it counts as LS-O5 run 1.

### LS-O4: the Star Power read from LS-K5 (inside LS-O3's run, or LS-O5's)

Calls: `ch_sp_between(114778.9, 166544.0)` (seen: LS-D2d run 1, read from that run's rows after its `ch_stop`)

Uses row P21. If question 1 accepts LS-K5 from LS-D2d, this is a confirmation and its result is recorded in the same report. Pass: no activation inside and the meter at or above the ready level with the flag clear somewhere in the window, as the LS-D2d rows read. Fail: an activation inside; that would mean the target run activated where the top path does not, and the run's activation list goes to the user.

### LS-O5: the full run against 1,229,730 (3 runs, 30 minutes)

Calls: `ch_start(<Senri folder>, human=True, player="ProDrums,Expert")` (seen: LS-D2d run 1), `ch_run_chart(<Senri folder>, <senri-top.json>, kit_player="System", decline_mode=<LS-O3's passing mode>, sp_end_lead_ms=<question 3>)` (seen: LS-O3's passing run), `ch_play_status` (seen: LS-D2d run 1), `ch_sp_watch_status` (seen: LS-D2d run 1), `ch_wait_stats` (seen: LS-D2d run 1), `ch_compare(<stats copy>, <senri-top.json>)` (seen: LS-D2d run 1), `ch_sp_between(114778.9, 166544.0)` (seen: LS-D2d run 1), `ch_stop` (seen: LS-O1e's passing run)

Uses every row LS-O2 and LS-O3 used. Pass: the stats file reads `score` 1229730, `notes_hit` 2996 of 2996, `accent_notes_hit` 368, `ghost_notes_hit` 726, `sp_activations` 8, and `compare_scorestats.py` prints five passing lines and exits 0; the MD5 is unchanged and every restore line is "unchanged" or "restored". Fail: any other score. The report then subtracts the game's `score` from 1,229,730 and names which of the five mechanisms the CSV shows short (a chord whose step differs from the target file's `cum.total` step is the first suspect). Three runs at most; after the third the user gets the three gaps and the three suspects. A run that fails on a mechanism LS-O1 to LS-O3 passed is a new fact, not a retry: it stops the step.

## Out of scope (needs the user's yes to leave out)

1. A chart whose first note falls before the chained start can press (about 1.2 s; the exact floor is a measurement LS-O2's part times give).
2. A chart with kick dynamics. The kit's kick is a button (`KIT_ELEMENTS["kick"]`, element 22), so it has no velocity; definition 1's axis kick is unmapped.
3. A chart whose top path needs real squeeze timing. Senri's top path has none (handoff; T0's file shows no `sqoutMs` in its path).
4. `ch_status` printing a finished trace with none running.
5. The second focus grab at 48.9 s in LS-D2d.
6. The game's `note_score` against Hydra's `notes` (SC-32).
7. Pushing `main`.
8. The late fill press as a way to move the ends (design point 3's fallback); it is built only if question 5 sends it back.
9. Hooking the velocity helper, the MIDI list entry and a virtual MIDI driver as routes past the gate.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The kit gets id 6 and setup refuses any other id | design 1; T1 | T1 | `test_kit_tools`: fake id 6 passes, fake id 0 and 7 refused with ids listed; LS-O1a's reply |
| Six placeholders are created before the kit and never added to a player | design 1 | T1 | `test_kit_tools`: the fake agent's call order has six creates, then the kit's create, and no `AddController` for a placeholder |
| Teardown destroys all seven and checks each, and "kit gone" fails on a leftover tag | design 1 | T1 | `test_kit_tools`: seven destroyed checks; a leftover tag fails "kit gone"; LS-O1e's `ch_stop` lines |
| No kit code compares the kit's id with 0 | design 2 | T1 | the grep test over `kit.ts` and `kit_tools.py` |
| The stale `KIT_PLAYER` comment is fixed | design 10 | T1 | review reads the comment; `KIT_PLAYER` test unchanged |
| `CreateCustomController`'s id source is read before the probe | prior art | T1 | the task report and an IN block on page 07, Unknown if not shown |
| An accent and a plain gem press at 1.0, a ghost at 0.5 + (T - 1) / 254 | design 5; T2 | T2 | `pad_axis` tests at T in 1, 64, 127; the mixed-chord message test |
| The threshold is read once from the engine, never typed | design 5 | T2 | `OFF_DYN_THRESHOLD` in `constants.py`; the model's read in the fake-process test; a threshold of 0 raises |
| Each gem's strength follows its `dynamic` from the score file | design 5 | T2 | fixture chords carry "ghost" and "accent"; the mixed-chord message carries three values |
| `ch_snapshot` prints the flags word and bit 0x10, and the threshold | design 6; T6 | T6 | `test_frida_tools`: 0x1010 prints set, 0x1000 prints clear; the threshold line |
| `ch_kit_press` takes a pad value and refuses one outside 0 to 1 | T6 | T6 | `test_kit_tools`: value 0.6 sent; 1.5 refused |
| A declined chord sends the kick only or nothing, and counts as no miss | design 4; T3 | T3 | the two mode tests; the hit-count test; a third mode refused |
| The decline list is derived once from the target file and is [405] on Senri | T3 | T3 | the fixture test pins [405] |
| One call sets up, watches and plays with one shared engine search | design point 3 of the plain version; T4 | T4 | `test_game_tools`: call order; same engine pointer to watch and player |
| A failed part stops the chained call there, tears the kit down and names the part | T4 | T4 | the failed-setup and failed-watch tests |
| The first-press chord is in the reply | T4 | T4 | the reply-line test; LS-O2's pass line reads it |
| The compare checks score, accent hits, ghost hits and activations and exits 0 only when all five pass | T5 | T5 | the five fixture cases in `test_compare_scorestats` |
| The status line says "end time at the flip" | design 10; T5 | T5 | the status fixture test |
| End chords press `SP_END_LEAD_MS` early, others 2 ms; a lead over 37.5 ms is refused | design 3; T7 | T7 | the fake-clock run test; the 8 marked chords; the refusal test |
| The target file holds 1,229,730 with 8 activations and chord 405 declined | T0 | T0 | the printed total and the JSON one-liner in LS-O1's go request |
| 1,229,730 is the current engine's number | design 7 | T0 | the empty `git log 853093cd..main` over the scoring sources, repeated after the rebuild |
| The probe shows id 6 accepted, a ghost press registering and dynamics counted | plain version | LS-O1a to LS-O1d | each sub-step's pass line |
| Every live step caps its tries and a failed run goes to the user | plain version | all LS | each step's run count and fail line; LS-O5's three-run cap and report |
| A failed end-chord read never changes Hydra or the target | design 3 | LS-O3 | LS-O3's lead fail line stops run 2 and names question 5 |
| LS-K4's pass line (142 hits, compare pass) is kept | design 8 | LS-O2 | LS-O2's pass line |
| LS-K5's read is repeated in the target run | design 9 | LS-O4 | LS-O4's pass line inside LS-O3 or LS-O5 |
| MD5 before and after, CSV and stats copied before `ch_stop` | the live steps | all LS | each run's `replies.txt` header and file list |
| The write-backs: the threshold to SC-13, the bit to SC-16, the end-chord result to SP-33, the decline to FL-31 | LS-O1b, LS-O3 | main session | the pages' blocks cite the run folder; the docs test passes |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The user ruled the work unfinished until an automatic run matches Hydra's optimal score | ruling | the main session's brief quoting session 70bfa33d: "our work is not finished until you can replicate hydra's optimal score in an automatic run..." (the session has no card yet; `docs/handoffs/2026-11-06-optimal-score-plan-handoff.md` line 85 holds the same words) | checked 2026-11-06 |
| P2: `KIT_PLAYER` stays `Player1` and a human launch names `System` | ruling | `docs/agents/rulings/clone-hero.md` line 5, "Keep Player1" | checked 2026-11-06 |
| P3: The gate returns a velocity only for a slot in the MIDI list or a player holding custom controller id 6, and otherwise -1 | code | `docs/clone-hero/code/6142/lane_velocity_337E10.asm` lines 33 to 70 and 105 | checked 2026-11-06 |
| P4: The gate asks the slot's own Rewired player (+0x18), which is the System Player on a command-line launch | code | `lane_velocity_337E10.asm` lines 42 to 54; `docs/clone-hero/07-internals.md` IN-66 (Proven in the game's code) | checked 2026-11-06 |
| P5: The velocity is round((GetAxis - 0.5) * 2 * 127), only when GetButtonDown is true | code | `lane_velocity_337E10.asm` lines 64 to 103 | checked 2026-11-06 |
| P6: A ghost is marked at a byte of at least 0 and below the threshold at +0x2C0, an accent at or above it, a negative byte marks neither, and a plain gem's byte is never read | code | `docs/clone-hero/code/6142/note_hit_dynamics_20DE770.md` lines 6 to 10 | checked 2026-11-06 |
| P7: With bit 0x10 of engine +0x198 set, an unperformed dynamic returns before scoring | code | `note_hit_dynamics_20DE770.md` line 9; `docs/clone-hero/06-scoring.md` SC-16 | checked 2026-11-06 |
| P8: The threshold at +0x2C0 is the int at +0x1C of the struct the DrumsPlayer method passes to the constructor | code | `docs/clone-hero/code/6142/engine_ctor_20DF680.asm` lines 51 to 53; `docs/clone-hero/code/6142/player_kind_11676A0.md` "Where the record goes" | checked 2026-11-06 |
| P9: How Rewired assigns a custom controller's id (a counter that never reuses, or the lowest free id) | unchecked | the Rewired pages fetched (`https://guavaman.com/projects/rewired/docs/CustomControllers.html` and the `CreateCustomController` API page) say nothing about ids; no listing of `CreateCustomController` is in `docs/clone-hero/code/` | T1 |
| P10: `ContainsController(Custom, 6)` is true when a custom controller with id 6 is on the player | unchecked | Rewired's semantics; nothing under `docs/clone-hero/` proves the match is by the controller's id | LS-O1a |
| P11: The bot writes 127 for an accent and the threshold minus 1 for a ghost, and the MIDI handler writes axis 0.5 + v / 254 | code | `docs/clone-hero/06-scoring.md` SC-13 Evidence line (RVAs `0x33A880`, `0x21E420`) | checked 2026-11-06 |
| P12: An axis near 0.5 on the kit still reads as GetButtonDown on the System player | unchecked | `tools/ch_mcp/kit_tools.py` line 75 shows only 1.0 was ever pressed; no run pressed lower | LS-O1c |
| P13: The threshold read live is between 1 and 127, and bit 0x10 reads clear or set | unchecked | `docs/clone-hero/06-scoring.md` SC-13 ("Where the threshold is stored and its default were not found") and SC-16 (Unknown) | LS-O1b |
| P14: With the gate passed and the right strengths, the game counts the accent and ghost hits in its stats file | unchecked | SC-14 shows the zero case from LS-D2d; no run has shown the positive case on the kit | LS-O1d |
| P15: Window Map has 142 notes, 13 before 5,400 ms, one tempo at 125 BPM and resolution 480 with zero offset and delay | chart | `C:\Clone Hero\songs\Hydra Probe\Window Map\notes.chart` lines 7, 15, 22 to 163; `song.ini` `delay = 0` | checked 2026-11-06 |
| P16: `windowmap.json` is Hydra's score file for Window Map, and the attach lands before the song clock reaches 0 | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-prep\windowmap.json`; `...\ls-d2d-run1\replies.txt` line 8 (songTime -0.56 s at attach) | checked 2026-11-06 |
| P17: Chord 405 is a plain kick plus a plain green cymbal at tick 54480, the fill the top path declines, and `senri-acts-true-ends.json` carries `dynamic`, `is_fill`, `sp_points` and `cum` per chord | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\senri-acts-true-ends.json`, chord index 405 | checked 2026-11-06 |
| P18: A landed fill flags one note of its chord, the highest pad by the drum-lanes reading | data | `docs/clone-hero/04-fills.md` FL-09 (Proven in the game's code; the highest-pad part rests on `docs/audit/drum-lanes/FINDINGS-play-time.md` line 50) | checked 2026-11-06 |
| P19: A missed flagged note is granted as a hit through the plain hit handler when Star Power can activate, so it scores without activating | code | `docs/clone-hero/04-fills.md` FL-31; `docs/audit/drum-lanes/f20dcad0.asm` lines 6 to 16 (a code reading of the slot spacing, never run) | checked 2026-11-06 |
| P20: Star Power ends in the first frame whose clock is at or past the end; whether a hit in that frame is scored first is unknown; the front hit window is 37.5 ms | data | `docs/clone-hero/05-star-power.md` SP-28 (Proven), SP-29 (Unknown); `docs/clone-hero/03-timing.md` TM-18 (Proven) | checked 2026-11-06 |
| P21: The LS-D2d run pressed 2,342 chords for 2,342 hits, scored 900,870 with 0 of 368 accents and 0 of 726 ghosts, paid chords 2044 and 2320 outside Star Power, and read no activation between 114,778.9 and 166,544.0 ms | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\replies.txt` lines 22, 38 to 41, 64 to 74 | checked 2026-11-06 |
| P22: The 8 target end chords are 268, 566, 819, 1019, 1649, 1843, 2044 and 2320, and their `sp_points` sum to 3,880 in the 9-fill file | run | `senri-acts-true-ends.json`, the chords whose `tick` equals each target deact tick | checked 2026-11-06 |
| P23: The top path scores 1,229,730 with 8 activations and declines chord 405 | unchecked | the brief's main session ran `hydra_replay dump` and `score --path ... --index 0 --prodrums 1 --bass2x 1`; the output file is not in the run folder | T0 |
| P24: No scoring source changed after `hydra_replay.exe` was built on 2026-10-31 13:20 | code | `git log 853093cd..main -- tools/replay.cpp tools/replay_json.cpp src/core src/search src/parse` is empty; `main` was 853093cd at 13:18 | checked 2026-11-06 |
| P25: `hydra_replay score` takes `--path`, `--index`, `--prodrums`, `--bass2x` and `--out` | code | `tools/replay.cpp` lines 98 to 116 | checked 2026-11-06 |
| P26: `kit.ts` reads the kit's id from the object and passes it to `GetMaps` and `LoadMap`; `press` writes each element's own value; `ALLOWED_CALLS` holds the create and destroy calls | code | `tools/ch_mcp/agent/kit.ts` lines 30 to 46, 237 to 245, 313, 346, 379 to 387 | checked 2026-11-06 |
| P27: `press_message` writes `KIT_AXIS_VALUE` (1.0) for every pad, and `KIT_ELEMENTS` makes the kick a button | code | `tools/ch_mcp/kit_tools.py` lines 75, 117 to 128, 213 | checked 2026-11-06 |
| P28: `load_dump_chords` keeps only color and cymbal; the press lead is 2 ms; a chord under 150 ms ahead at the first read is skipped; the watch and the player each run their own engine search | code | `tools/ch_probe/experiments/play_chart.py` lines 65 and 99 to 106; `tools/ch_probe/experiments/live.py` lines 151 to 162; `tools/ch_mcp/sp_tools.py` line 325; `tools/ch_mcp/game_tools.py` line 572 | checked 2026-11-06 |
| P29: `compare_scorestats.py` passes or fails on note and phrase counts only and never reads `score` | code | `tools/ch_mcp/compare_scorestats.py` lines 71 to 95 | checked 2026-11-06 |
| P30: The engine snapshot already reads the flags word at `OFF_FLAGS` (0x198) | code | `tools/ch_probe/engine.py` line 107; `tools/ch_probe/constants.py` line 143 | checked 2026-11-06 |
| P31: LS-K4 and LS-K5 stand with their calls and pass lines and have not run | code | `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md` lines 299 to 309; `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md` lines 220 to 226; no `ls-k4` or `ls-k5` folder under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\` | checked 2026-11-06 |
| P32: The hit handler keeps one byte per lane at +0x309 to +0x310, so each gem of a chord is graded on its own lane's strength | code | `note_hit_dynamics_20DE770.md` line 6 | checked 2026-11-06 |
| P33: All 631 kick gems on Senri are plain, so no lane needs two strengths in one chord | run | `senri-acts-true-ends.json`, every gem with color "Kick" has `dynamic` "none" (handoff claim re-read from the file) | checked 2026-11-06 |
