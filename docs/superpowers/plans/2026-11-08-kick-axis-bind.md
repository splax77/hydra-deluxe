# How a ghost kick reaches the game: build the kit from the real kit's definition (2026-11-08, redone)

Task PL-KICK-PLAN-2, session 6341c618. This plan replaces the first kick plan of the same date (task PL-KICK-PLAN), which built a second custom controller because it believed no axis kick existed in the game. The user asked: "kick dynamics work on a real drum kit. why can't they work for you? ... did you actually dig into the code to find out how to support them, or did you make an assumption that it's not possible?" and ruled "yes redo it". This plan reads the code a real kit goes through and finds that the first plan's premise was wrong.

## The short version

A real drum kit is a MIDI device. The game's `MIDIInputManager` registers it (RVA `0x21CDE0`): it opens the MIDI port, then calls Rewired's `CreateCustomController(1, null)` and `AddController` on the player's slot. Definition 1 is "MIDI Drums". Every frame the manager's pump (RVA `0x21E420`) turns each MIDI note into `CustomController.SetAxisValue(controller, "Kick", 0.5 + velocity / 254)`, and the same for the seven pads; Start and Select are written as axes at 1.0. So on a real kit the kick is an axis, and its strength goes in exactly the way the virtual kit's pads already go in.

The game ships the definition and its map in `Clone Hero_Data\level1`. Definition 1 holds ten element identifiers, all of type Axis: Snare 0, Tom1 1, Tom2 2, Tom3 3, Cymbal1 4, Cymbal2 5, Cymbal3 6, Kick 7, StartPad 18, SelectPad 19; it has ten axes and no buttons. Map id 2, at category 1 ("Gameplay") and layout 2 ("Drums"), binds those ten elements to the same ten actions the kit's map binds today: Kick 7 to action 4 (Orange), StartPad 18 to 16 (Start), SelectPad 19 to 6 (Select), and the pads to Red, Yellow, Blue, Green and the three cymbals.

So the route closest to a real kit is to build the virtual kit from definition 1 instead of definition 6 ("SLInputDrums"). The kick becomes element 7, an axis, pressed at `pad_axis(dynamic)` like a pad. Nothing else about the kit changes: the same `CreateCustomController` call with a different number, the same `AddController`, the same by-hand `LoadMap(Custom, id, 1, 2, true)`, the same six placeholders so the kit lands on id 6, the same `SetAxisValueById` writes, the same teardown. The kit agent (`kit.ts`) does not change at all; only `kit_tools.py`'s constants, checks and texts do.

Which gate it passes. The velocity helper (`lane_velocity_337E10.asm`) reads a lane's strength when `GetButtonDown(action)` is true and either of two checks holds: the player's slot is in `MIDIInputManager`'s list of registered devices (method `0x21F790` walks that list and compares each record's slot with the argument), or the player holds a custom controller of type Custom (20) with controller id 6 (`ContainsController(ControllerType, int controllerId)`, so 6 is a controller id, not a definition id). The virtual kit passes the second gate today and keeps passing it. It does not pass the first, and does not need to: after the gate, both kinds of player go through the same `GetAxis(4)` read and the same round((v - 0.5) x 2 x 127). That is the one difference from a real kit, and question 1 puts it to the user.

Why not feed `MIDIInputManager` itself. Its register method needs a real MIDI port: at `0021cf30` it calls `RtMidi.MidiBase._GetPortCount`, and `cmp ebx, eax; jg` at `0021cf37` leaves the method with nothing made when the device index is past the last port. The machine has no MIDI port, and making the manager's device record by hand means writing game fields directly, which no agent does today (`docs/agents/clone-hero-mcp.md`, "No agent sets a game field directly"). That route is dropped; the exact instruction that stops it is the port-count compare.

## What the first plan got wrong

Each line names the claim and what the code says instead.

"An axis binding on the kick element itself is impossible" and "there is no spare axis to bind": wrong. Definition 1's Kick is an axis, and map 2 binds it to Orange. The first plan only read definition 6.

"Definition 1's elements and maps were not read ... the registration path has more unknowns than the one binding call": the elements and the map are now read from `level1` (rows P7 to P10). The registration path is not needed; only the definition is.

"The handler (RVA 0x21E420) builds a controller from definition 1": wrong method. `0x21E420` is the per-frame pump that writes axis values. The register method `0x21CDE0` builds the controller. SC-13 on page 06 already had this right.

"A ghost kick is a press on another controller ... the kick stays a button for plain and accent kicks": dropped. Every kick is an axis press at its `pad_axis` value, like a pad. There is no second controller, no `AddEmptyMap`, no `CreateElementMap`, no routing by value, and no `KICK_AXIS_ALL` switch.

"A pressed button bound to action 4 makes GetAxis read 1.0" (its P11) and "with the kit's kick button unpressed, GetAxis(4) reads the kick axis's value" (its P13): both moot. The kit has no button.

What still holds from it: the velocity formula and the two gates (P1 to P3), the drum map bindings for definition 6 (P10, now read from `level1` as well as the live run), the `ControllerElementType` numbers (P11), the two charts, the T5 branch question, and the fact that the press tool sees only lanes and values (P17).

## What T5 did and what happens to it

T5 of `docs/superpowers/plans/2026-11-07-player-limits.md` (branch `claude/pl-t5-kick-axis`, commit 86578b15) changed only `tools/ch_mcp/kit_tools.py`, `tools/ch_mcp/tests/test_kit_tools.py` and `tools/ch_probe/tests/test_play_chart.py`. It made the kick `KitElement(22, "Kick", False)`, an axis at element 22 on definition 6, and added `ELEMENT_TYPE_AXIS`, `ELEMENT_TYPE_BUTTON`, `KIT_TYPE_CHECKED` and `_type_problem`. Element 22 is a button in definition 6, so its setup check refuses the map the game gives (run `ls-d2b-run1`). The idea was right and the definition was wrong. T1 here takes its constants and its type check, with the kick at element 7 of definition 1, and T2 takes its play-chart test. Question 2 asks to drop the branch.

## Global constraints

Runs never need window focus and never send keys through Windows: every press is `SetAxisValueById` on a Rewired custom controller inside the game, as today. The agent writes no game file and no registry value; the kit is destroyed at teardown, and `ch_stop` restores the Rewired registry key whether or not the game saved anything (P19). No new game method is called, so `tools/ch_mcp/kit_methods.json` and `kit.ts`'s allowed list do not change, and `tools/ch_mcp/tests/test_frida_attach.py`'s pinned counts stay as they are (P20). No agent builds C++ or runs `hydra_tests`, `hydra_replay` or `hydra_uitest`; the main session runs the live steps. Every live step has a cap of 2 runs and 10 minutes. The user ruled the EULA question "Proceed, my call" (P21).

## The real rule, with examples from both ends

The game turns `GetAxis(4)` into a byte: round((value - 0.5) x 2 x 127) (P1). A hit marks a ghost when the byte is 0 or more and below the engine threshold, and an accent when it is at or above it; the threshold read 100 live (P16). The kit's `pad_axis` writes 0.5 + velocity / 254 (P15), the inverse of the read and the same formula the MIDI pump uses (P6). So:

| What presses | Axis value written | Byte the game reads | Mark |
|---|---|---|---|
| a velocity-1 kick on element 7 (the lowest ghost) | 0.504 | 1 | ghost |
| a ghost kick, as `pad_axis("ghost", 100)` gives it | 0.890 | 99 | ghost, the last byte below 100 |
| a velocity-100 kick | 0.894 | 100 | accent, the first byte at the threshold |
| a plain or accent kick, `pad_axis("none")` or `pad_axis("accent")` | 1.000 | 127 | accent side |
| a kick on a controller that is not id 6 and not a registered MIDI device | not read | -1 | neither |

No screen text in Hydra changes. What changes is what a kit run scores on a chart with ghost or accent kicks: today every kick is a button press whose strength the game never reads below 127, and after this plan LS-M2b counts the ghost kicks in `ghost_notes_hit`.

## Questions

1. **Build the kit from definition 1 (MIDI Drums) and pass the id-6 gate, not the MIDI-device gate?** The user's goal is that "ghost and accent kicks reach the game the way a real kit's do". This route uses the real kit's controller definition, its shipped map and its axis write, so the value reaches the judge through the same `GetAxis(4)` read a real kit's does. The one difference is the gate: a real kit passes because its slot is in `MIDIInputManager`'s device list; the virtual kit passes because it is custom controller id 6. The code reads the velocity the same way after either check (`lane_velocity_337E10.asm` lines 67 to 79: `test dil` for the MIDI check, `test sil` for the id check, then one `GetAxis`). Sides: (a) take this route, T1 to T4 and LS-M0 to LS-M2; (b) insist on the MIDI-device gate, which needs a virtual MIDI port driver on the machine (not installed; `RtMidi.MidiBase._GetPortCount` at `0021cf30` ends the register with nothing when no port exists) or by-hand game field writes, each a new permission; (c) leave kicks as they are. Recommended: (a). Examples from both ends: under (a) a chart with 36 velocity-1 kicks (the Who chart, SC-18) scores them as ghosts, and the Who chart's score matches Hydra's; under (c) none of them is a ghost and the run cannot match Hydra; under (b) nothing runs until a MIDI port exists.
2. **The T5 branch.** Sides: (a) T1 starts from main and takes T5's `ELEMENT_TYPE_AXIS`, `ELEMENT_TYPE_BUTTON` and `_type_problem` with every element expected as an axis, T2 takes T5's play-chart test, and `claude/pl-t5-kick-axis` is deleted after T1 merges; (b) merge T5 then fix on top. Recommended: (a); T5's `KitElement(22, "Kick", False)` and its kick-only type check are the parts that are wrong.
3. **The charts.** LS-M0 needs a `.mid` with accent kicks: I Am Ahab (33, `docs/clone-hero/live-checks.md` line 49; row P23, the main session confirms it in the library with a db query before the go). LS-M1 and LS-M2 need a `.mid` with ghost kicks: the Who chart, Won't Get Fooled Again (36 ghost kicks, SC-18; row P24, the same db query). Sides: (a) extend the allowance line in `docs/agents/clone-hero-mcp.md` line 31 to both charts from their library folders, on the user's 2026-10-31 ruling that network blockers are not wanted, as the two earlier plans did; (b) copy each into `Hydra Probe`. Recommended: (a). If the db query finds neither, the main session lists the `.mid` charts with velocity-1 kicks it does find and asks.
4. **Re-prove the menu walk on definition 1?** The menu walk (`ch_open_song`, kept for a day the direct route breaks) presses `kit:start` and `kit:green` on Player1. On definition 1, Start is an axis written at 1.0, which is what the MIDI pump writes for a real kit's Start pad (P6), but no run has pressed it on this kit. Sides: (a) run LS-M3 (two short steps at the title screen) and record IN-79 as re-proven; (b) skip it and mark the menu walk "not re-proven on definition 1" on the mcp page until a day it is needed. Recommended: (b); the direct route is the one in use, and LS-M3 costs a launch. Examples: under (a) IN-79's player count going 0 to 1 on a Start press is read again; under (b) a future menu-walk run starts with a never-tried Start press.
5. **Out of scope.** The list at the end needs the user's yes.

## Tasks

All Python tests: `py -m unittest tools.ch_mcp.tests.test_kit_tools tools.ch_mcp.tests.test_frida_attach` and `py -m pytest -q tools/ch_probe/tests/test_play_chart.py`. Executors run on Opus, each in its own worktree cut from current main (0fa446de or later; this branch holds main as of 0fa446de, P26), one fresh reviewer each (D107). T1, T2 and T3 are independent and run in parallel. Each is under 100 calls and 20 minutes. T4 is the main session's, after the runs.

### T1: the kit is built from definition 1 (Python; Opus; independent)

Owns `tools/ch_mcp/kit_tools.py`, `tools/ch_mcp/tests/test_kit_tools.py`, and the `ch_kit_setup` and `ch_kit_press` descriptions in `tools/ch_mcp/server.py` (text only). It changes no agent code: `tools/ch_mcp/agent/kit.ts`, `tools/ch_mcp/kit_methods.json` and `tools/ch_mcp/tests/test_frida_attach.py` stay as they are.

`KIT_DEFINITION_ID` becomes 1, with a new `KIT_DEFINITION_NAME = "MIDI Drums"` and a comment naming this plan and the `level1` facts (P7, P9). `KitElement` loses `button` and gains `strength: Optional[float]`: None means the element takes the press's value (the pads and the kick), and 1.0 means it is always written at full, as the game's MIDI pump writes StartPad and SelectPad (P6). `KIT_ELEMENTS` keeps its ten keys (so `Lane` names and `menu_steps.json`'s `kit:` items still resolve) with the new ids and names: `"kick": KitElement(7, "Kick", None)`, `"start": KitElement(18, "StartPad", 1.0)`, `"select": KitElement(19, "SelectPad", 1.0)`; the pads keep 0 to 6. `KIT_BINDINGS` keys become 7, 18 and 19 for actions 4, 16 and 6. From T5's commit it takes `ELEMENT_TYPE_AXIS = 0` and `ELEMENT_TYPE_BUTTON = 1` (P11) and `_type_problem`, now applied to every element: `binding_problems` refuses any binding whose `element_type` is not `ELEMENT_TYPE_AXIS`, naming the element and saying the kit is built from definition 1, whose elements are all axes, so a button binding is another definition's map. `press_message`'s `element_list` writes `strength` when it is set and the given value otherwise; a press never writes `True`. `setup_lines` prints the definition id and name on the kit line. The press text says "pads and kick pressed at value" when a value is given. The `ch_kit_press` description in `server.py` says `value` sets the pads and the kick, and that Start and Select always press at full.

Acceptance (unit tests on canned replies): the new ten-binding map (elements 0 to 7, 18, 19, all type 0, the actions above) passes `map_problems` and `binding_problems`; the `ls-d2b-run1` map (definition 6's: elements 20 to 22 of type 1) is refused, naming element 22 as not in `KIT_BINDINGS` and element 7 as expected action 4 with none given; a map with element 7 of type 1 is refused naming "button" and definition 1; `setup_message()` carries `"definition_id": 1`; `lane_element(Lane.KICK)` is element 7; a chord of red ghost plus kick ghost gives two elements, 0 and 7, each at the ghost value; a kick with no value gives element 7 at `pad_axis("none")` (1.0); `start` gives element 18 at 1.0 even when a value of 0.6 is given; no element in any `press_message` has the value `True`; the tests that named element 22 or `True` for the kick are rewritten to the new numbers, and the test file says why in one comment naming this plan.

### T2: the player hands a kick its strength, pinned by a test (Python; Opus; independent)

Owns `tools/ch_probe/tests/test_play_chart.py` only. `with_axes` in `tools/ch_probe/experiments/play_chart.py` already gives every gem a `pad_axis` value, kicks included, and `Chord.axes` is kept per lane (P17), so the player needs no change. What is missing is a test that says so. T2 adds one test to `DynamicsTest`, carried from T5's commit 86578b15 (`test_a_ghost_or_accent_kick_gets_the_axis_a_pad_gets`, written against the current file): a chord of a ghost kick, and a chord of an accent kick with a red ghost, run through `with_axes` with the engine threshold 64. The press log gains no column (P18), and the kept press logs are not touched.

Acceptance: the new test passes and would fail if `with_axes` stopped giving a kick lane its value (the kick's `axes` entry would be `None` or 1.0); the existing `DynamicsTest` cases pass unchanged; the diff touches only `test_play_chart.py`.

### T3: the game facts go on page 07 with their listings (docs; Opus; independent)

Owns `docs/clone-hero/07-internals.md`, `docs/clone-hero/code/6142/lane_velocity_337E10.md`, `docs/clone-hero/code/facts-6142.tsv`, `docs/clone-hero/README.md` (the count table only), and three new listings under `docs/clone-hero/code/6142/`, each with its `.md` beside it: `midi_register_21CDE0.asm` (the register method, made with `py -m tools.ch_mcp.chdis 21CDE0 4c0`), `midi_pump_21E420.asm` (`py -m tools.ch_mcp.chdis 21E420 830`) and `midi_slot_check_21F790.asm` (`py -m tools.ch_mcp.chdis 21F790 b0`). `chdis.py` needs `capstone`, which `py -m` finds and `py -I` does not.

Page 07 gets five blocks, each labelled "Proven in the game's code" with its listing lines, or "Proven in the game's code" with the `level1` byte offsets for the data file facts: definition 1 "MIDI Drums" (its ten identifiers, ids and types, ten axes, no buttons; `level1` bytes 987772 to 989720, P7) and its map 2 at category 1, layout 2 (bytes 984680 to 985148, P9); definition 6 "SLInputDrums" (seven axes 0 to 6, buttons Start 20, Select 21, Kick 22; bytes 990908 to 992480, P8) and its map 6 (bytes 986656 to 987124, P10); `ControllerElementType` Axis 0, Button 1, CompoundElement 100 (`dump.cs` lines 290899 to 290906, written as an enum "not checked" per the README, P11); the register method (creates from definition 1 and adds the controller to the slot's player with no `LoadMap`; refuses without a MIDI port, P5); and the pump (writes Kick as an axis at 0.5 + velocity / 254 and Start and Select at 1.0, P6). The method `0x21F790` gets its meaning in the register block's text, and `lane_velocity_337E10.md` paragraph 3 says "a slot in MIDIInputManager's device list" in place of "registered kits", with the id check named as a controller id. The facts table is rebuilt with `tools/ch_mcp/dump_facts.py --tsv` (the executor reads that file for its exact flags) so every new RVA has a row, and `README.md`'s count table is recounted. The docs check (`hydra_tests.exe "-tc=docs match code*"`) is the main session's; T3 hands the branch back uncompiled and says so.

Acceptance: the reviewer opens each new block's listing lines and `level1` offsets and confirms they say what the block says; `dump_facts.py --check` reports no difference; the docs check passes when the main session runs it.

### T4: the runs' write-back (main session, no code; after the runs)

Page 06: SC-19 takes LS-M0b's result (an accent kick on the axis kit, counted or not); SC-14's evidence gains the kick-as-axis run; SC-18 gains LS-M2b. Page 07: IN-77 gains the definition-1 presses from LS-M1c and LS-M2b, and IN-76 gains that `LoadMap` by category and layout found map 2 for a definition-1 controller (LS-M1b). `docs/agents/clone-hero-mcp.md`: the kit paragraph (line 27) says the kit is built from definition 1 with every element an axis, and line 31 names the two charts (question 3); if question 4 took (b), the menu-walk paragraph says the walk is not re-proven on definition 1. `docs/clone-hero/live-checks.md` line 49 and the SC-19 row take the run. The README count table is recounted. This plan leaves the repo in the merge that finishes T4, and the docs check proves nothing still points at it.

## The live steps

The main session runs these through the `ch` MCP; a planning agent calls no `ch_*` tool. Before each run the user sees the step's `Calls:` line, each never-tried call and each unchecked row it uses, and says go. Every run: hold other sessions from before `ch_start` to after `ch_stop`; take the DLL's MD5 before and after; copy the stats file out before `ch_stop`; keep `replies.txt` in a dated folder under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`. `ch_reload` after the merge of T1. Every step has a cap of 2 runs and 10 minutes; a run at its cap stops and goes to the user. The order is LS-M1, then LS-M0, then LS-M2, then LS-M3 if question 4 took (a). LS-M1 needs T1 merged and reloaded, because T1's setup refuses definition 6's map. LS-M0 and LS-M2 need LS-M1c passed.

### LS-M1a: the ghost-kick launch (the Who chart; 2 runs, 10 minutes)

Calls: `ch_start(<Won't Get Fooled Again's folder>, human=True, player="ProDrums,Expert")` (never tried)

Uses rows P21, P24. The main session first confirms the chart is in the library with a db query (P24) and has made its dump and score files with `tools/ch_probe/prep_inputs.py`; if either fails the step ends with that report. Pass: `ch_start` returns patched. Fail: a refusal or a passed cap, to the user. LS-M1b starts at once.

### LS-M1b: the definition-1 kit's setup and read-back (inside LS-M1a's run; 2 runs)

Calls: `ch_attach` (seen: LS-D2b run 1), `ch_kit_setup(player="System")` (never tried), `ch_kit_status` (seen: LS-D1b run 1)

The one never-tried piece is that the kit is built from definition 1. Uses rows P7, P9, P12, P13, P14, P27. Pass: the reply shows placeholders 0 to 5, the kit on id 6 with definition 1, one enabled map at category 1 layout 2 with ten bindings all of type 0 matching the new `KIT_BINDINGS` (element 7 to action 4, 18 to 16, 19 to 6, the pads as before), "bindings: all 10 match KIT_BINDINGS", and the keyboard and mouse lines off. Fail: the map list is empty (P27 false: `LoadMap` did not find map 2 for a definition-1 controller; the reply's maps lines go to the user, and the fallback is `LoadDefaultMaps` on Player1 as a diagnostic in run 2, since Player1 lists category 1 layout 2 for custom controllers, IN-76), a binding differs from `KIT_BINDINGS` (the tool refuses and tears down; the lines go to page 07 and the user), or `CreateCustomController(1, tag)` returned nothing (the fault lines to the user; no run 2).

### LS-M1c: a ghost-value kick press reads as Orange (inside LS-M1a's run; 2 runs)

Calls: `ch_kit_press("kick", value=0.89, read="Orange")` (never tried), `ch_stop` (seen: LS-D2b run 1)

The one never-tried piece is element 7 pressed as an axis. Uses rows P15, P28. Pass: the read lines show Orange on from the frame after the down write and off at release, the way the pads read (IN-77); `ch_stop`'s teardown prints every check ok and the registry line reads unchanged or restored. Fail: Orange never reads on (P28 false: an axis at 0.89 on element 7 does not fire the Orange action; the map's binding is read back and goes to the user before any further run), or a teardown check is not ok (the step names which, and the user hears before any further run).

### LS-M0a: the accent-kick launch (I Am Ahab; 2 runs, 10 minutes)

Calls: `ch_start(<I Am Ahab's folder>, human=True, player="ProDrums,Expert")` (never tried)

Uses rows P21, P23. Starts only once LS-M1c has passed. As LS-M1a: the db query and the prep files first. LS-M0b starts at once.

### LS-M0b: accent kicks score as accents (inside LS-M0a's run; 2 runs)

Calls: `ch_run_chart(<I Am Ahab's folder>, <score.json>, kit_player="System", decline_mode="all_but_top_pad", dump_json=<dump.json>)` (never tried), `ch_play_status` (seen: LS-R1 run 1), `ch_wait_stats` (seen: LS-R2ab run 1), `ch_compare(<stats copy>, <score.json>)` (seen: LS-R2ab run 1), `ch_stop` (seen: LS-M1c's passing run)

Uses rows P1, P2, P16, P17, P32, P33. Pass: every kick the reply does not list as declined hits, and the stats' `accent_notes_hit` counts the chart's 33 accent kicks less the accent kicks the reply lists as declined (SC-19's live answer is yes). Fail: `accent_notes_hit` leaves the kicks out with every kick row hit (the kick's byte did not reach 100: the press rows' values and the user), kicks miss (a player bug, to the user), or the score differs with the counts right (a Hydra question, to the user as a report). Run 2 only when the machine was not held.

### LS-M2a: the ghost-kick run's launch (2 runs, 10 minutes)

Calls: `ch_start(<Won't Get Fooled Again's folder>, human=True, player="ProDrums,Expert")` (seen: LS-M1a's passing run)

Uses row P24. Starts only once LS-M1c has passed. LS-M2b starts at once.

### LS-M2b: ghost kicks score as ghosts (inside LS-M2a's run; 2 runs)

Calls: `ch_run_chart(<Won't Get Fooled Again's folder>, <score.json>, kit_player="System", decline_mode="all_but_top_pad", dump_json=<dump.json>)` (never tried), `ch_play_status` (seen: LS-R1 run 1), `ch_wait_stats` (seen: LS-R2ab run 1), `ch_compare(<stats copy>, <score.json>)` (seen: LS-R2ab run 1), `ch_stop` (seen: LS-M1c's passing run)

Uses rows P1, P15, P16, P17, P24, P32, P33. Pass: every kick the reply does not list as declined hits, `ghost_notes_hit` counts the chart's 36 ghost kicks less the ghost kicks the reply lists as declined, and the score equals the file's. Fail: the ghosts hit but kicks counted as accents (the ghost value did not reach the judge: the press rows and the user), kicks miss (the press rows and the user), or the score differs with the counts right (a Hydra question, to the user as a report). SC-18 and SC-14 on page 06 take the result (T4).

### LS-M3a: the definition-1 kit on Player1 at the title screen (only if question 4 took (a); 2 runs, 10 minutes)

Calls: `ch_start()` (seen: LS-K3 run 1 of the in-game-input plan, a menu launch), `ch_attach` (seen: LS-K3 run 1), `ch_kit_setup()` (never tried)

The one never-tried piece is definition 1 on Player1. Uses rows P7, P9, P27. Pass: the kit on id 6 with the same map and bindings as LS-M1b, on Player1. Fail: as LS-M1b.

### LS-M3b: a Start press as an axis joins a player (inside LS-M3a's run; 2 runs)

Calls: `ch_kit_press("start", read="Start")` (never tried), `ch_menu` (seen: LS-K3 run 1), `ch_stop` (seen: LS-M1c's passing run)

The one never-tried piece is StartPad 18 pressed as an axis at 1.0. Uses rows P6, P29. Pass: Start reads on during the hold, and `ch_menu` shows the player count gone from 0 to 1 (IN-79). Fail: Start never reads on or the count stays 0 (P29 false: the menu walk needs a button Start; the mcp page says so and the walk stays on hold).

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The kit is built from definition 1, and `KIT_DEFINITION_ID` is the one place that says so | The short version; T1 | T1 | `setup_message()` test carries `"definition_id": 1`; the reviewer greps `kit_tools.py` for `6` next to "definition" and finds none |
| Every kit element is an axis: the kick at 7, Start at 18, Select at 19, pads 0 to 6; no press writes `True` | T1 | T1 | the `lane_element(Lane.KICK)` test; the "no `True` in any `press_message`" test |
| Start and Select always press at 1.0, as the MIDI pump writes them | T1 | T1 | the `start` at 0.6 test gives element 18 at 1.0 |
| Setup refuses by name a map whose bindings are not all axes, and definition 6's map | T1 | T1 | the `ls-d2b-run1` map refused naming element 22 and element 7; the type-1 element 7 test naming "button" and definition 1 |
| The new ten-binding map passes setup's checks | T1 | T1 | the canned new-map test passes `map_problems` and `binding_problems` |
| The kit agent, its allowed list and its pinned call counts do not change | Global constraints; T1 | T1 | the reviewer reads the diff: `kit.ts`, `kit_methods.json` and `test_frida_attach.py` untouched; `test_frida_attach` passes |
| The agent edits no file and no registry value; `ch_stop` restores the key | Global constraints | LS-M1c | LS-M1c's registry line |
| Runs need no focus and send no keys | Global constraints | all | every call in every `Calls:` line is a kit or read tool; none is `ch_key`, `ch_press` or a keys route |
| A kick lane carries its `pad_axis` value out of the player | T2 | T2 | the new `DynamicsTest` case; it fails if a kick's `axes` entry is not the ghost value |
| The press log gains no column and the kept press logs are untouched | T2 | T2 | the reviewer reads the diff: only `test_play_chart.py` changes |
| Page 07 gets definition 1 and its map, definition 6 and its map, the enum numbers, the register method and the pump, each with its listing or `level1` offsets, and the facts table has every new RVA | T3 | T3 | the reviewer opens each block's source; `dump_facts.py --check` clean; the docs check run by the main session |
| `lane_velocity_337E10.md` names the MIDI check as the slot in the device list and 6 as a controller id | T3 | T3 | the reviewer reads paragraph 3 against `0x21F790`'s listing and `dump.cs` line 306725 |
| SC-19, SC-14, SC-18, IN-76, IN-77, the mcp page, live-checks and the README take the runs; the plan leaves the repo | T4 | T4 | the docs check run by the main session |
| Every live step has a cap of 2 runs and 10 minutes, one never-tried call at most, and a stop line | The live steps | main session | each `### LS` heading names its cap; the docs check counts never-tried calls |
| LS-M0 and LS-M2 do not start before LS-M1c passes | LS-M0a; LS-M2a | main session | LS-M2a's `Calls:` line cites LS-M1a's passing run; both steps' first line |
| The charts come from the library only on the user's yes | Question 3 | T4 | line 31 of the mcp page names them in T4's merge |
| The menu walk is either re-proven on definition 1 or marked not re-proven | Question 4 | T4; LS-M3 | LS-M3b's pass line, or the mcp page's menu-walk paragraph |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: the velocity byte is round((GetAxis(action) - 0.5) x 2 x 127) when GetButtonDown(action) is true and a gate passes, else -1 | code | `docs/clone-hero/code/6142/lane_velocity_337E10.asm` lines 62 to 107 | checked 2026-11-08 |
| P2: the drums input reads the kick lane through that helper with action id 4 | code | `docs/clone-hero/code/6142/drums_input_source.asm` lines 183 to 187 | checked 2026-11-08 |
| P3: the gate is the MIDI check (method `0x21F790` on the slot) or `ContainsController(Custom = 0x14, 6)`, either one; `ContainsController(ControllerType, int controllerId)` takes a controller id | code | `lane_velocity_337E10.asm` lines 33 to 41, 51 to 54 and 67 to 70; `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs` line 306725 | checked 2026-11-08 |
| P4: method `0x21F790` walks `MIDIInputManager`'s device list (field +0x38) and returns true when a record's slot (+0x10) is the argument | code | the installed `GameAssembly.dll` (MD5 in `docs/clone-hero/code/facts-6142.tsv` line 3) at RVA `0x21F790`, read with `py -m tools.ch_mcp.chdis 21F790 b0`: the loop at 0021f7e0 to 0021f814 and `cmp qword ptr [rax + 0x10], rsi`; `dump.cs` lines 52504 to 52513 (the record's fields) and 52813 | checked 2026-11-08; T3 commits the listing |
| P5: the register method `0x21CDE0` needs a MIDI port (`_GetPortCount` at 0021cf30, `cmp ebx, eax; jg` at 0021cf37), then calls `CreateCustomController(1, null)` at 0021cfcc to 0021cfd4 and `AddController(controller, false)` on the slot's player at 0021cfff, stores the controller at the record's +0x30, and calls no `LoadMap` | code | the same DLL at RVA `0x21CDE0`, read with `py -m tools.ch_mcp.chdis 21CDE0 4c0`; `dump.cs` line 52750 | checked 2026-11-08; T3 commits the listing |
| P6: the pump `0x21E420` writes each hit pad with `CustomController.SetAxisValue(controller, <name>, velocity / 127 x 0.5 + 0.5)`, the kick as "Kick" (0021ea43 to 0021ea4a), and "StartPad" and "SelectPad" at 1.0 (0021eaa8 to 0021eab3, 0021eb15 to 0021eb20) | code | the same DLL at RVA `0x21E420`, read with `py -m tools.ch_mcp.chdis 21E420 830`: lines 0021e6fd to 0021e712 for the formula; `dump.cs` line 286520 for `SetAxisValue(string, float)` | checked 2026-11-08; T3 commits the listing |
| P7: definition 1 is "MIDI Drums" with identifiers Snare 0, Tom1 1, Tom2 2, Tom3 3, Cymbal1 4, Cymbal2 5, Cymbal3 6, Kick 7, StartPad 18, SelectPad 19, each with `_elementType` 0, ten axes and no buttons | data | `C:\Clone Hero\1.1 june\Clone Hero\Clone Hero_Data\level1` bytes 987772 to 989720: `_id` 1 at 987804, the identifier count 10 at 987852 and its records from 987856 (field order from `dump.cs` lines 295089 to 295116: id, name, five strings, `_elementType`), the axis count 10 at 988324 and its records from 988328, the buttons count 0 at 989716 | checked 2026-11-08 |
| P8: definition 6 is "SLInputDrums" with identifiers 0 to 6 of type 0 and Start 20, Select 21, Kick 22 of type 1, seven axes and three buttons | data | `level1` bytes 990908 to 992480: `_id` 6 at 990940, the identifier records from 990992, the axis count 7 at 991456 and its records from 991460, the button count 3 at 992428 and its records from 992432 | checked 2026-11-08 |
| P9: `level1` ships map id 2 at category 1, layout 2, `customControllerUid` 1, binding 0 to 1, 1 to 2, 2 to 3, 3 to 0, 4 to 37, 5 to 38, 6 to 39, 7 to 4, 18 to 16, 19 to 6, every binding `_elementType` 0 | data | `level1` bytes 984680 to 985148 (header words id 2, category 1, layout 2, two empty strings, uid 1, count 10; ten 44-byte records in the field order of `dump.cs` lines 294184 to 294214) | checked 2026-11-08 |
| P10: `level1` ships map id 6 at category 1, layout 2, uid 6, with the same ten actions on elements 0 to 6 (type 0) and 22, 20, 21 (type 1), which is `KIT_BINDINGS` and the live read of `ls-d2b-run1` | data | `level1` bytes 986656 to 987124; `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2b-run1\kit-2.jsonl` line 2 | checked 2026-11-08 |
| P11: Rewired's `ControllerElementType` is Axis = 0, Button = 1, CompoundElement = 100 | code | `dump.cs` lines 290899 to 290906 | checked 2026-11-08 |
| P12: a custom controller is created with its definition's axis and button counts, and `CreateCustomController(int sourceControllerId, string tag)` takes the definition id, which the kit passes as `definition_id` | code | `docs/clone-hero/code/6142/create_custom_controller_1EE9800.asm` lines 65 to 72; `tools/ch_mcp/kit_methods.json` line 13; `tools/ch_mcp/agent/kit.ts` lines 254 to 261 and 353 | checked 2026-11-08 |
| P13: controller ids count up from 0 in a launch and a destroy does not move the counter, so six placeholders put the kit on id 6 | data | `docs/clone-hero/07-internals.md` IN-81 (Proven in the game's code); `tools/ch_mcp/kit_tools.py` lines 59 to 66 | checked 2026-11-08 |
| P14: setup loads the kit's map with the five-argument `LoadMap(Custom, kit id, category, layout, true)` called by hand with the method pointer, and reads the maps back with `GetMaps` | code | `tools/ch_mcp/agent/kit.ts` lines 386 to 408 and 294 to 311; `docs/clone-hero/07-internals.md` IN-76 | checked 2026-11-08 |
| P15: `pad_axis` gives 0.5 + velocity / 254, with 127 for a plain or accent gem and threshold - 1 for a ghost; `press_message` writes a button as `True` and ignores its value; `kit.ts` writes a number with `SetAxisValueById` and a boolean with `SetButtonValueById` | code | `tools/ch_mcp/kit_tools.py` lines 243 to 264 and 287 to 294; `tools/ch_mcp/agent/kit.ts` lines 442 to 450 | checked 2026-11-08 |
| P16: the hit handler marks a ghost for a byte from 0 below the threshold and an accent at or above it; the threshold read 100 live | data | `docs/clone-hero/06-scoring.md` SC-13 (Proven in the game's code; the live 100 from `ls-o1-run1`) | checked 2026-11-08 |
| P17: `with_axes` gives every gem, kicks included, `pad_axis(dynamic, threshold)` in `Chord.axes`, and `kit_press`'s `press` takes lanes and values only | code | `tools/ch_probe/experiments/play_chart.py` lines 938 to 951; `tools/ch_mcp/kit_tools.py` lines 564 to 570 | checked 2026-11-08 |
| P18: the press log's columns name no gem's element | code | `tools/ch_probe/experiments/overlay_logs.py` lines 56 to 99 (`PRESS_LOG_COLUMNS`) | checked 2026-11-08 by the first plan's reviewer; this plan does not change the log |
| P19: the backup exports `HKCU\Software\srylain Inc.\Clone Hero` as `rewired.reg` and the restore re-imports it when it differs | code | `tools/ch_mcp/game_files.py` lines 70 to 77 and 255 to 270 | checked 2026-11-08 |
| P20: `test_frida_attach.py` pins `kit.ts`'s `.invoke(` sites at 43 and `new NativeFunction` sites at 1, and holds its allowed list equal to `kit_methods.json` | code | `tools/ch_mcp/tests/test_frida_attach.py` lines 62, 67 and 119 to 124 | checked 2026-11-08 |
| P21: the user ruled "Proceed, my call" on the EULA, and "Keep Player1" for the kit's default player | ruling | `docs/agents/rulings/clone-hero.md` lines 13 and 5 | checked 2026-11-08 |
| P22: the user ruled "yes redo it" for a plan around the real kit's MIDI Drums controller, after asking "did you actually dig into the code to find out how to support them, or did you make an assumption that it's not possible?" | ruling | the brief for `PL-KICK-PLAN-2`, as the main session quotes the user | checked 2026-11-08 |
| P23: I Am Ahab is a `.mid` with 33 accent kicks and is in the library | unchecked | `docs/clone-hero/live-checks.md` line 49 names it; the db copy was not queried | LS-M0a's go request (a db query by the main session) |
| P24: Won't Get Fooled Again (the Who chart) has 36 ghost kicks and is in the library | unchecked | `docs/clone-hero/06-scoring.md` SC-18 cites ADR 0012 line 23 for the 36 ghost kicks; the db copy was not queried | LS-M1a's go request (a db query by the main session) |
| P25: T5's commit changes `kit_tools.py`, `test_kit_tools.py` and `test_play_chart.py` only, making the kick `KitElement(22, "Kick", False)` and adding the element-type constants and `_type_problem` | code | `git diff --stat main...claude/pl-t5-kick-axis` and the diff's `KIT_ELEMENTS` and `_type_problem` hunks | checked 2026-11-08 |
| P26: this plan's branch is not behind main (`git rev-list --count HEAD..main` gives 0 at main 0fa446de, after the reviewer merged main into the branch) | code | `git log --oneline -3 main` and `git rev-list --count HEAD..main` in the `plan-pl-kick` worktree | checked 2026-11-08 |
| P27: `LoadMap(Custom, id, 1, 2, true)` on a controller made from definition 1 finds map 2, the way it found map 6 for definition 6 | unchecked | no listing reads how Rewired picks a custom controller's map by its definition; IN-76 in `docs/clone-hero/07-internals.md` proves it only for definition 6 | LS-M1b |
| P28: an axis element of a definition-1 controller written at 0.89 fires its bound button action, as the pads of definition 6 do (IN-77, SC-14's 726 ghosts) | unchecked | IN-77 and SC-14 prove it for the pads of definition 6; no run has pressed element 7 of definition 1 | LS-M1c |
| P29: a Start pad written as an axis at 1.0 joins a player at the title screen, as a button Start did (IN-79) | unchecked | IN-79 proves it for the button Start of definition 6; the MIDI pump writes 1.0 for a real kit (P6), but no run has pressed it on the virtual kit | question 4 |
| P30: the mcp page's rules: no agent sets a game field directly, every game method an agent calls is on its file's allowed list, and the key route is locked | code | `docs/agents/clone-hero-mcp.md` lines 29 and 33 | checked 2026-11-08 |
| P31: `MIDIInputManager`'s record holds the slot at +0x10, the `MidiIn` at +0x18, the `CustomController` at +0x30, the message list at +0x38 and the enabled flag at +0x48; a mapping holds NoteNumber, Velocity and OverHitThreshold; the pad names are "Kick Pad" and the rest | code | `dump.cs` lines 52504 to 52513, 52618 to 52636 and 52723 to 52732 | checked 2026-11-08 |
| P32: in decline mode `all_but_top_pad` a passed-over fill chord with no pad gem sends nothing, so a kick-only chord in a passed-over fill is not pressed | code | `tools/ch_probe/experiments/play_chart.py` lines 709 to 727 (`_kept_gems`) | checked 2026-11-08 |
| P33: no accent kick of I Am Ahab and no ghost kick of the Who chart sits in a fill chord the decline mode passes over | unchecked | `tools/ch_probe/experiments/play_chart.py` lines 709 to 727 says what the mode sends; neither chart's score file was read for it | LS-M0b and LS-M2b read the reply's declined list against the kick counts |

## Out of scope (needs the user's yes to leave out)

1. Passing the MIDI-device gate itself (a virtual MIDI port, or a by-hand `MIDIInputManager` record): question 1 (b) names what it would take; this plan does not build it.
2. Reading how Rewired picks a custom controller's map by its definition (P27): the run answers it, and the code is Rewired's, not the game's.
3. Plain Drums dynamics, by the user's words of 2026-11-07 ("except for the regular drums, that is an intentional deviation from the game. we dont need to test that").
4. A Hydra-side count of library charts with velocity-1 or velocity-127 kicks (how much the route is worth): the main session can run it on request; this plan does not.
5. What the pump's reset method `0x220660` writes to each axis before the frame's messages: read as ten `SetAxisValue` calls, value not read; it does not change this plan.
