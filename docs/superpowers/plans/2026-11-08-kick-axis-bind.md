# How a ghost kick reaches the game: the kick axis controller (2026-11-08)

Task PL-KICK-PLAN, session 6341c618. This plan answers one held task: T5 of `docs/superpowers/plans/2026-11-07-player-limits.md` made the virtual kit's kick an axis, and its setup check wants an axis binding on the kick. The game binds the kick as a button, so T5 as committed (branch `claude/pl-t5-kick-axis`, commit 86578b15) would refuse every kit run at setup. The user chose "Plan an in-run axis bind". This plan says what the code allows, what it does not, and the one route that works inside the game.

## The short version

The game does not read the kick's strength from the kick element. It reads it from the action. The velocity helper (`docs/clone-hero/code/6142/lane_velocity_337E10.asm`) calls Rewired's `Player.GetAxis` on the lane's action id (4, "Orange", for the kick) and turns the answer into a velocity byte: round((GetAxis - 0.5) x 2 x 127). So what matters is what `GetAxis(4)` returns, not which element is pressed.

An axis binding on the kick element itself is impossible. The kit is built from the game's custom controller definition 6 ("SLInputDrums"), and that definition holds exactly ten elements: seven axes (Snare, Tom1, Tom2, Tom3, Cymbal1, Cymbal2, Cymbal3, ids 0 to 6) and three buttons (Start, Select, Kick, ids 20 to 22). A custom controller gets its axis and button counts from its definition when it is created, and nothing adds an element to it afterwards. There is no spare axis to bind, and element 22 is a button in the definition, so a binding that calls it an axis would point at an axis that does not exist.

That splits the kick question in two. An accent kick most likely needs no change at all: a pressed button bound to an action makes `GetAxis` read 1.0, which is velocity 127, the accent side. That is Rewired's documented behaviour (buttons contribute to axis values) but no run has read it in this game, so it is row P11 and live step LS-K0 settles it. A ghost kick needs a value below the threshold (0.89 for the threshold of 100 the game read live), and no button can give one.

The route for ghost kicks is a second custom controller on the same player. Setup makes one more controller from definition 6 after the kit, tagged `hydra-kick`, which takes id 7. It goes on the same Rewired player as the kit, gets an empty map at the drum map's category and layout (`AddEmptyMap`), and that map gets one binding made at runtime: its element 0 (the Snare axis) to action 4 (Orange), with `ControllerMap.CreateElementMap(4, Positive, 0, Axis, Full, false)`. A ghost kick is then an axis write of the ghost value on that controller's element 0. The game's velocity gate still passes, because the player still holds controller 6. Nothing on disk or in the registry is edited by the agent: the maps live on the player in memory, teardown removes and destroys the controller before the game quits, and `ch_stop`'s registry restore stands behind that. Input still comes from inside the game; no `SendInput`, no focus.

This narrows what the user asked. The user said "Plan an in-run axis bind", which reads as an axis on the kick. What this plan builds is an axis bound to Orange on a second controller, and the kick element itself stays a button. The plan does not decide that: question 1 puts it to the user first. The kick stays a button for plain and accent kicks. That is the route proven over 2,342 presses (IN-77 on page 07). T5's branch is not merged; its useful parts (the `ControllerElementType` numbers and the element-type check) move into T1 here with the kick's expected type set to button, which is what the game gives.

## What was checked and found

Each fact below is a row in the Premise map at the end. The handoff for this work is `docs/handoffs/2026-11-07-player-limits-handoff.md` in the `handoff-player-limits` worktree; `py -I tools/check_handoff.py` on it gave 11 ok, 0 missing, 2 not-on-main, 8 not-pushed, 8 manual. Two claims in it do not match what exists now. The lane branch `claude/pl-lane-earliest` was at 598a5748, not 5603d112, with a clean worktree: the "uncommitted edit to page 03" the handoff names was committed as 598a5748 ("Timing page TM-50"). That branch is now merged into main (02190724), and the merge changed `tools/ch_probe/experiments/play_chart.py` and `tools/ch_probe/tests/test_play_chart.py`, so every executor cuts its worktree from main at 02190724 or later, never from this plan's old branch point. And the handoff's reading that the kick must be an axis for dynamics to score is half right: it holds for ghosts, and most likely not for accents (P11).

T5's commit changes only `tools/ch_mcp/kit_tools.py`, `tools/ch_mcp/tests/test_kit_tools.py` and `tools/ch_probe/tests/test_play_chart.py`. It touches no agent code. Its `_type_problem` refuses the map the game gives (element 22 with `element_type` 1), which run `ls-d2b-run1` recorded on 2026-11-06, so the handoff's "every kit run would refuse at setup" is right.

Rewired's `ControllerElementType` is Axis = 0, Button = 1, CompoundElement = 100 (`dump.cs` lines 290899 to 290906). That is the write-back owed to page 07; T4 adds it.

Prior art. Rewired's own API reference lists `ControllerMap.CreateElementMap(Int32, Pole, Int32, ControllerElementType, AxisRange, Boolean)` as "controller button or axis assignment" and `ReplaceElementMap` with the same shape, and says a `ReplaceElementMap` whose map id is not found returns false and creates nothing ([ControllerMap methods](https://guavaman.com/projects/rewired/docs/api-reference/html/Methods_T_Rewired_ControllerMap.htm)). The Controller Maps page says maps belong to players and can be added, removed, loaded and saved at runtime by script, and that "all saving and loading of controller data is up to the developer to implement" ([Controller Maps](https://guavaman.com/projects/rewired/docs/ControllerMaps.html)). The Custom Controllers page says a custom controller's elements are defined in the Rewired Editor and instantiated at runtime from the Input Manager's definitions, with values pushed by `SetAxisValue` and `SetButtonValue` ([Custom Controllers](https://guavaman.com/projects/rewired/docs/CustomControllers.html)). The Input Behaviors page says digital axis settings let "buttons and keyboard keys be treated like axes and return values ranging from -1 to 1" ([Input Behaviors](https://guavaman.com/projects/rewired/docs/InputBehaviors.html)). None of these pages says what `GetAxis` returns for a pressed button with digital axis simulation off, which is why P11 is unchecked. The game's dump holds the same methods: `CreateElementMap` at `dump.cs` line 297289 on `ControllerMap` (line 297102), `AddEmptyMap` at 304238 and `GetMap(ControllerType, int, int, int)` at 304107 on the player's `MapHelper` (line 303824).

## Routes considered and not taken

Replacing the kick's binding with an axis binding on the same controller: no spare axis exists (P7), and every pad axis is in use for its own lane. Not possible.

Hooking the velocity helper at 0x337E10 to return the wanted byte for the kick lane: this changes what the game's judge computes, not what the player sends. Every change the tools make today is either a read, a trace hook that only records, or the one loader byte the user allowed by name on 2026-10-31. A hook that rewrites a judged value is a new kind of change and would need the user's own words. Not recommended; question 1 lists it as a side so the user can rule.

Registering a fake MIDI kit so the game's MIDI handler writes velocities: the handler (RVA 0x21E420) builds a controller from definition 1 and needs an entry in `MIDIInputManager`'s registered list. Definition 1's elements and maps were not read, and the registration path has more unknowns than the one binding call. Not recommended.

Sending all kicks through the new axis, with the kit's kick button unused: one path instead of two, but it moves the proven kick route onto a never-tried one for every chart. Question 2 asks; the recommendation keeps plain and accent kicks on the button.

## Global constraints

Runs never need window focus and never send keys through Windows: every press in this plan is a method call on a Rewired custom controller inside the game (`SetAxisValueById`, `SetButtonValueById`), as today. The agent writes no game file and no registry value; the only Rewired objects it makes are destroyed at teardown, and `ch_stop` restores the registry key whether or not the game saved anything. Every new game method the agent calls is on the kit's allowed list in `tools/ch_mcp/kit_methods.json` with its dump line, and `tools/ch_mcp/tests/test_frida_attach.py` holds the list equal to `kit.ts`'s. No agent builds C++ or runs `hydra_tests`, `hydra_replay` or `hydra_uitest`; the main session builds the kit agent (`npm run build:kit`) and runs the live steps. Every live step has a cap of 2 runs and 10 minutes. The user ruled the EULA question "Proceed, my call" (P25).

## The real rule, with examples from both ends

The game turns the answer of `GetAxis(4)` into a byte: round((value - 0.5) x 2 x 127) (`lane_velocity_337E10.asm`, P1). A hit marks a ghost when the byte is 0 or more and below the engine threshold, and an accent when it is at or above it (SC-13, P21). The threshold read 100 live. So:

| What presses | `GetAxis(4)` | Byte | Mark |
|---|---|---|---|
| a velocity-1 kick (the lowest ghost) on the new axis | 0.504 | 1 | ghost |
| a ghost kick on the new axis, as `pad_axis` gives it for threshold 100 | 0.890 | 99 | ghost, the last byte below 100 |
| a velocity-100 kick | 0.894 | 100 | accent, the first byte at the threshold |
| a plain or accent kick on the kit's button, if P11 holds | 1.000 | 127 | accent side |
| a press that does not pass the gate | not read | -1 | neither |

No screen text in Hydra changes. What changes is what a kit run scores on a chart with ghost kicks: today the player presses every kick at full strength, and with this plan LS-K2b counts the ghost kicks in `ghost_notes_hit`.

## Questions

1. **Build the kick axis controller, and accept that the kick element stays a button?** The user asked for an in-run axis bind. The code allows an axis only on a second controller bound to Orange (P7), not on the kick element, so this is narrower than the words: the kick keeps its button, and a ghost kick is a press on another controller. Sides: (a) build T1 to T4 and run LS-K0 to LS-K2, so a ghost kick can score as a ghost; (b) leave ghost kicks as they are: the player cannot match Hydra on a chart with velocity-1 kicks (Hydra prices them, SC-18), and T5's branch is dropped; (c) hook the velocity helper instead (the route above, not recommended, needs the user's words). Recommended: (a). The check is LS-K2's stats line: `ghost_notes_hit` counting the chart's kick ghosts. Examples from both ends: a chart whose kicks are all plain loses nothing under (b); a chart like the Who one, with 36 velocity-1 kicks that Hydra prices as ghosts (SC-18), has none of them scored as ghosts by the player under (b).
2. **Which kicks go through the axis?** Sides: (a) ghosts only, meaning kicks whose value is below the accent value 1.0 (the table above); plain and accent kicks keep the kit's button, the route IN-77 proved; (b) every kick through the axis, one path (a plain or accent kick presses the axis at 1.0, byte 127). Recommended: (a). If LS-K0 shows a button kick does not count as an accent (P11 false), the answer becomes (b), and T1's routing constant `KICK_AXIS_ALL` changes from `False` to `True`; the plan names that one line so the change is one edit. The press tool sees only a lane and a value, never the chart's dynamic, and a plain gem and an accent gem both press at 1.0 (P32), so the routing cannot tell an accent kick from a plain one; that is why the choice is ghosts-or-all and not by dynamic.
3. **The T5 branch.** Sides: (a) T1 starts from main and takes T5's `ELEMENT_TYPE_AXIS`, `ELEMENT_TYPE_BUTTON`, `KIT_TYPE_CHECKED` and `_type_problem` with the kick expected as a button, T3 takes T5's play-chart test, and `claude/pl-t5-kick-axis` is deleted after T1 merges; (b) merge T5 then fix on top. Recommended: (a); T5's kick-as-axis `KIT_ELEMENTS` line and its test are the thing that is wrong.
4. **The charts.** LS-K0 needs a `.mid` with accent kicks: I Am Ahab (33, `docs/clone-hero/live-checks.md` line 49; row P17, the main session confirms it in the library with a db query before the go). LS-K1 and LS-K2 need a `.mid` with ghost kicks: the Who chart, Won't Get Fooled Again (36 ghost kicks, SC-18; row P18, the same db query). Sides: (a) extend the allowance line in `docs/agents/clone-hero-mcp.md` line 31 to both charts from their library folders, on the user's 2026-10-31 ruling that network blockers are not wanted, as the two earlier plans did; (b) copy each into `Hydra Probe`. Recommended: (a). If the db query finds neither, the main session lists the `.mid` charts with velocity-1 kicks it does find and asks.
5. **Out of scope.** The list at the end needs the user's yes.

## The message shapes (so T1 and T2 can run in parallel)

The setup message gains one block: `"kick_axis": {"tag": "hydra-kick", "element_id": 0, "action_id": 4, "category_id": 1, "layout_id": 2}`. Without the block the agent builds no second controller (the old behaviour, kept for the menu walk). The setup reply gains `"kick_axis": {"id": <int>, "tag": "hydra-kick", "created": <bool>, "player": <name>, "maps": [<same shape as the kit's maps>]}` and, for both controllers, `"elements": [{"id", "name", "element_type"}]` under `kit` and under `kick_axis`, read from each controller's `ElementIdentifiers`. A press element gains an optional `"controller"`: `"kit"` (the default) or `"kick_axis"`. The teardown reply gains the checks "kick axis elements cleared", "kick axis removed from player", "kick axis destroyed", and "kick gone" is folded into "kit gone", which now also names a controller still tagged `hydra-kick`.

## Tasks

All Python tests: `py -m unittest tools.ch_mcp.tests.test_kit_tools tools.ch_mcp.tests.test_frida_attach` and `py -m pytest -q tools/ch_probe/tests/test_play_chart.py`. Executors run on Opus, each in its own worktree cut from main, one fresh reviewer each (D107). T1, T2 and T3 are independent and run in parallel, each cut from main at 02190724 or later (P30); T4 is the main session's after the runs.

### T1: kit_tools knows the kick axis controller (Python; Opus; independent)

Owns `tools/ch_mcp/kit_tools.py`, `tools/ch_mcp/tests/test_kit_tools.py`, and the `ch_kit_setup`, `ch_kit_press` and `ch_kit_teardown` descriptions in `tools/ch_mcp/server.py` (text only). From T5's commit it takes `ELEMENT_TYPE_AXIS = 0`, `ELEMENT_TYPE_BUTTON = 1`, `KIT_TYPE_CHECKED` and `_type_problem`, with the kick staying `KitElement(22, "Kick", True)` so the expected type is button. New constants, each with a comment naming this plan: `KICK_AXIS_TAG = "hydra-kick"`, `KICK_AXIS_ELEMENT = 0`, `KICK_AXIS_ACTION = (4, "Orange")`, `KICK_AXIS_REQUIRED_ID = KIT_REQUIRED_ID + 1`, `KICK_AXIS_ALL = False` (question 2: when true every kick that has a value goes to the axis; when false only a kick whose value is below `pad_axis(DYNAMIC_ACCENT)` does). `KitElement` gains `controller: str = "kit"`, and `KIT_ELEMENTS` gains `"kick_axis": KitElement(0, "Snare", False, controller="kick_axis")`, so `ch_kit_press("kick_axis", value=0.89)` presses it by hand. `setup_message` carries the `kick_axis` block. `setup_lines` prints the second controller's id, tag, map and bindings and both element lists. A new `kick_axis_problems(reply)` refuses, each by name: a missing `kick_axis` block, an id other than `KICK_AXIS_REQUIRED_ID`, no enabled map at the category and layout, a map with any binding other than exactly one on element 0 of type axis to action 4, and a kit element list that is not seven axes (0 to 6) and three buttons (20 to 22). `_setup` runs it after `binding_problems` and tears down on a refusal. `kit_press`'s `press` receives only lanes and values (P32), so it routes a kick lane by its value: a kick with a value below `pad_axis(DYNAMIC_ACCENT)`, or any kick with a value when `KICK_AXIS_ALL` is true, goes to the `kick_axis` element at that value, and every other kick (no value, or 1.0) to the button; `press_message` writes each element's `controller`, in every order group of a split chord (`then`) as well as the first; the press text line of `press_items` names the controller of each element. `teardown_lines` prints the new checks.

Acceptance (unit tests on canned replies): the `ls-d2b-run1` map with element 22 of type 1 passes `binding_problems` (the test says T5's branch refused it); a kick binding of type 0 is refused naming "button"; a reply with the kick axis block as the shapes section gives passes; each of the five refusals above fires on a reply built to break only it (an id of 8; a disabled map; a second binding; a binding on element 1; an element list with eight axes); a chord of red ghost plus kick ghost gives two elements, element 0 on `kit` at the ghost value and element 0 on `kick_axis` at the ghost value; a kick at `pad_axis("none")` or `pad_axis("accent")` (both 1.0) and a kick with no value give element 22 on `kit` with `true` and no value; with `KICK_AXIS_ALL` patched to true the kick at 1.0 goes to `kick_axis` at 1.0; `setup_message()` carries the block with these literals; `press_message` with a `kick_axis` element writes `"controller": "kick_axis"` and a kit element writes none or `"kit"`; `teardown_lines` prints the three new checks.

### T2: the kit agent builds, binds and tears down the kick axis (TypeScript; Opus; independent)

Owns `tools/ch_mcp/agent/kit.ts`, `tools/ch_mcp/kit_methods.json`, and `tools/ch_mcp/tests/test_frida_attach.py`, where `CALL_SITES[KIT_AGENT]` (43 `.invoke(` sites today) and `NATIVE_SITES[KIT_AGENT]` (1 `new NativeFunction` site) are pinned counts (P31): the executor sets each to the new count in `kit.ts`, and the commit message says what they are now. In `setup`, after the kit is added and its map loaded, when the message holds `kick_axis`: `createController(definitionId, tag)`, record it in `recorded` before anything else so a throw still tears it down, `AddController(controller, true)` on the same player, `maps.AddEmptyMap(Custom, id, categoryId, layoutId)`, `maps.GetMap(Custom, id, categoryId, layoutId)`, then on that map `CreateElementMap(actionId, 0, elementId, 0, 0, false)` (Pole.Positive, ControllerElementType.Axis, AxisRange.Full) and throw if it returns false, naming the arguments. Whether `AddEmptyMap` and `CreateElementMap` need the by-hand call with the trailing method pointer that `LoadMap` needs (IN-76): the executor checks each method's first instructions with `tools/ch_mcp/chdis.py` the way IN-76 describes, and uses the by-hand form for any that writes past its arguments; the commit message says which. `readMaps` runs for both controllers; a new `readElements(controller)` lists `ElementIdentifiers` with id, name and `elementType` number. `write` takes the `controller` word and writes to the kit or the kick axis. `teardown` clears the kick axis's touched elements, removes it from the player and destroys it, each as a check, and "kit gone" looks for its tag too. New allowed calls, each with its dump line: `AddEmptyMap` (304238), `GetMap` (304107), `CreateElementMap` (297289), `get_ElementIdentifiers` (285803), and `get_elementType` and `get_id` and `get_name` on `ControllerElementIdentifier` under `also` (lines 295129 to 295133 give the properties; the executor reads their getter lines).

Acceptance: `py -m unittest tools.ch_mcp.tests.test_frida_attach` passes with the new names and the new site counts (the test fails on any name in `kit.ts` not in the JSON, and the reverse, and on any `.invoke(` or `new NativeFunction` count that differs from the pinned one); the reviewer reads `setup` and confirms the record is written before the first create and that a false `CreateElementMap` throws; the main session builds with `npm run build:kit` and the live proof is LS-K1b (setup and read-back) and LS-K1c (the press).

### T3: the player hands a kick its strength, pinned by a test (Python; Opus; independent)

Owns `tools/ch_probe/tests/test_play_chart.py` only. `with_axes` in `tools/ch_probe/experiments/play_chart.py` already gives every gem a `pad_axis` value, kicks included, and `Chord.axes` is kept per lane (P32), so the player needs no change: T1's `kit_press` does the routing from the lane and the value. What is missing is a test that says so. T3 adds one test to `DynamicsTest`, carried from T5's commit 86578b15 (`test_a_ghost_or_accent_kick_gets_the_axis_a_pad_gets`, written against the pre-merge file): a chord of a ghost kick, and a chord of an accent kick with a red ghost, run through `with_axes` with the engine threshold 64. The press log has no column that names a gem's element (P33), so this plan adds none; which route a kick took is read from the kit press's own text line (T1 prints the controller) and from LS-K2b's stats.

Acceptance: the new test passes and would fail if `with_axes` stopped giving a kick lane its value (the kick's `axes` entry would be `None` or 1.0); the existing `DynamicsTest` cases pass unchanged; the two kept press logs (`ls-r1-run2\press-1.csv`, `ls-r2ab-run1\press-2.csv`) are not touched.

### T4: the pages and the write-back (main session, no code; after the runs)

Page 07 gets a new block: definition 6's ten elements, their ids and types, with the `level1` offsets and the live read from LS-K1b as evidence; and the `ControllerElementType` numbers (Axis 0, Button 1, CompoundElement 100, `dump.cs` lines 290899 to 290906, as an enum "not checked" by the docs test per the README). Page 06: SC-19 gets LS-K0's result (an accent kick on a button, pressed by the kit, counted or not), and SC-13 gets the line that the velocity comes from `GetAxis` on the action, so a button reads 127. `docs/agents/clone-hero-mcp.md`: the kit paragraph (line 27) names the kick axis controller and its teardown checks; line 31 names the two charts (question 4). `docs/clone-hero/live-checks.md` line 49 and the SC-19 row take the run. The README count table is recounted. This plan leaves the repo in the merge that finishes T4, and the docs check (`hydra_tests.exe "-tc=docs match code*"`, run by the main session) proves nothing still points at it.

## The live steps

The main session runs these through the `ch` MCP; a planning agent calls no `ch_*` tool. Before each run the user sees the step's `Calls:` line, each never-tried call and each unchecked row it uses, and says go. Every run: hold other sessions from before `ch_start` to after `ch_stop`; take the DLL's MD5 before and after; copy the stats file out before `ch_stop`; keep `replies.txt` in a dated folder under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\`. `ch_reload` after each merge, and `npm run build:kit` before the reload that follows T2. Every step has a cap of 2 runs and 10 minutes; a run at its cap stops and goes to the user. The order is LS-K1, then LS-K0, then LS-K2. LS-K1 needs T1 to T3 merged and the kit agent built, because T1's setup refuses a reply without the kick axis block. LS-K0 needs LS-K1c passed, so its setup is not also a first try of the kick axis. LS-K2 needs LS-K1c passed and LS-K0b resolved.

### LS-K0a: the accent-kick launch (I Am Ahab; 2 runs, 10 minutes)

Calls: `ch_start(<I Am Ahab's folder>, human=True, player="ProDrums,Expert")` (never tried)

Uses rows P17, P25. Starts only once LS-K1c has passed. The main session first confirms the chart is in the library with a db query (P17) and has made its dump and score files with `tools/ch_probe/prep_inputs.py`; if either fails the step ends with that report. Pass: `ch_start` returns patched. Fail: a refusal or a passed cap, to the user. LS-K0b starts at once.

### LS-K0b: a button kick as an accent (inside LS-K0a's run; 2 runs)

Calls: `ch_run_chart(<I Am Ahab's folder>, <score.json>, kit_player="System", decline_mode="all_but_top_pad", dump_json=<dump.json>)` (never tried), `ch_play_status` (seen: LS-R2ab run 1), `ch_wait_stats` (seen: LS-R2ab run 1), `ch_compare(<stats copy>, <score.json>)` (seen: LS-R2ab run 1), `ch_stop` (seen: LS-R2ab run 1)

Uses rows P1, P2, P3, P11, P21. The setup line must read "bindings: all 10 match KIT_BINDINGS" and the kick axis lines (LS-K1b has already shown them pass). Pass: every kick row hits, and the stats' `accent_notes_hit` counts the chart's 33 accent kicks (P11 true: a button reads 1.0, so SC-19's live answer is yes). Fail: `accent_notes_hit` leaves the kicks out with every kick row hit (P11 false: a button does not read 1.0; question 2's answer becomes (b) and `KICK_AXIS_ALL` becomes true before LS-K2), or kicks miss (a player bug, to the user). Run 2 only when the machine was not held.

### LS-K1a: the ghost-kick launch (the Who chart; 2 runs, 10 minutes)

Calls: `ch_start(<Won't Get Fooled Again's folder>, human=True, player="ProDrums,Expert")` (never tried)

Uses rows P18, P25. As LS-K0a: the db query and the prep files first. LS-K1b starts at once.

### LS-K1b: the kick axis setup and read-back (inside LS-K1a's run; 2 runs)

Calls: `ch_attach` (seen: LS-D2b run 1), `ch_kit_setup(player="System")` (never tried: the setup now builds the kick axis), `ch_kit_status` (seen: LS-D2b run 1)

Uses rows P7, P8, P9, P10, P14, P24, P26. Pass: the reply shows the kit on id 6 with ten elements (seven of type 0, ids 0 to 6; three of type 1, ids 20 to 22), the kick axis on id 7 with the same ten elements, its map at category 1 layout 2 enabled with exactly one binding (element 0, type 0, action 4), and the keyboard and mouse lines off. Fail: `CreateElementMap` returned false or threw (P10 false: the reply's fault lines to the user; no run 2 until the call's argument layout is read with `chdis.py` as T2 says), `AddEmptyMap` or `GetMap` faulted the way IN-76's `LoadMap` did (the by-hand form is needed: T2's executor missed it; a fresh executor, then run 2), or the kit's element list is not seven axes and three buttons (P7 wrong: page 07 and the user). A setup that reads a wrong id is refused by the tool itself and names the launch rule (P26).

### LS-K1c: a ghost-value press on the kick axis reads as Orange (inside LS-K1a's run; 2 runs)

Calls: `ch_kit_press("kick_axis", value=0.89, read="Orange")` (never tried), `ch_kit_press("kick", read="Orange")` (seen: LS-D1b run 1, a kick on System), `ch_stop` (seen: LS-D2b run 1)

Uses rows P12, P13. Pass: the first press's read lines show Orange on from the frame after the down write and off at release (the axis binding fires the button action, as the pads' axes do); the second shows the same for the button; `ch_stop`'s teardown prints every check ok including "kick axis destroyed", and the registry line reads unchanged or restored. Fail: Orange never reads on for the axis press (the runtime binding is not read by `GetButtonDown`: the binding's `enabled` flag and the map's are read back and go to the user), or a teardown check is not ok (the step names which, and the user hears before any further run).

### LS-K2a: the ghost-kick run's launch (2 runs, 10 minutes)

Calls: `ch_start(<Won't Get Fooled Again's folder>, human=True, player="ProDrums,Expert")` (seen: LS-K1a's passing run)

Uses row P18. Starts only once LS-K1c has passed and LS-K0b has passed or its fail line has been acted on. LS-K2b starts at once.

### LS-K2b: ghost kicks score as ghosts (inside LS-K2a's run; 2 runs)

Calls: `ch_run_chart(<Won't Get Fooled Again's folder>, <score.json>, kit_player="System", decline_mode="all_but_top_pad", dump_json=<dump.json>)` (never tried), `ch_play_status` (seen: LS-R2ab run 1), `ch_wait_stats` (seen: LS-R2ab run 1), `ch_compare(<stats copy>, <score.json>)` (seen: LS-R2ab run 1), `ch_stop` (seen: LS-K1c's passing run)

Uses rows P1, P11, P13, P21. Pass: every kick row hits, `ghost_notes_hit` counts the chart's 36 ghost kicks, and the score equals the file's. Fail: the ghosts hit but kicks counted as accents (P13 false: `GetAxis(4)` took the kit's button, not the axis; the design moves every kick to the axis, question 2 (b), and run 2 follows that one-line change), kicks miss (the press rows and the user), or the score differs with the counts right (a Hydra question, to the user as a report). SC-18 and SC-19 on page 06 take the result (T4).

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| The kick stays a button for plain and accent kicks; the setup check expects element 22 of type 1 | The short version; T1 | T1 | the `ls-d2b-run1` map passes `binding_problems`; a type-0 kick binding is refused naming "button" |
| T5's kick-as-axis line and its test never merge | Question 3 (a) | T1 | T1 starts from main; the reviewer confirms `KIT_ELEMENTS["kick"]` is a button |
| Setup refuses by name a missing block, a wrong id, no enabled map, any binding set other than one axis binding on element 0 to action 4, and a kit element list that is not 7 axes and 3 buttons | T1 | T1 | five canned-reply tests, one refusal each |
| A ghost kick is routed to the kick axis at its `pad_axis` value; plain and accent kicks to the button | T1; question 2 | T1 | the two routing tests |
| `KICK_AXIS_ALL` is the one line question 2's answer changes | Question 2 | T1 | the patched-`KICK_AXIS_ALL` test shows a kick at 1.0 flips to the axis |
| Every press element names its controller | The message shapes | T1, T2 | `press_message` test; the reviewer reads `write` |
| The agent records the kick axis before any other call on it, so a throw still tears it down | T2 | T2 | the reviewer reads `setup`; LS-K1b's fail line |
| A false `CreateElementMap` throws naming its arguments | T2 | T2 | the reviewer reads `setup`; LS-K1b's fail line |
| Every new game method is on the allowed list with its dump line | Global constraints; T2 | T2 | `test_frida_attach` passes; it fails on any name missing either way |
| Teardown clears, removes and destroys the kick axis and checks it is gone | The message shapes; T2 | T2 | LS-K1c's `ch_stop` lines all ok; `teardown_lines` test in T1 |
| The agent edits no file and no registry value; `ch_stop` restores the key | Global constraints | T2; LS-K1c | the reviewer greps `kit.ts` for `UserDataStore`, `Save`, `PlayerPrefs` and finds none; LS-K1c's registry line |
| Runs need no focus and send no keys | Global constraints | all | every call in every `Calls:` line is a kit or read tool; none is `ch_key`, `ch_press` or a keys route |
| A kick lane carries its `pad_axis` value out of the player, so T1 can route it | T3 | T3 | the new `DynamicsTest` case; it fails if a kick's `axes` entry is not the ghost value |
| The press log gains no column and the kept press logs are untouched | T3 | T3 | the reviewer reads the diff: only `test_play_chart.py` changes |
| Page 07 gets definition 6's elements and the `ControllerElementType` numbers; SC-13, SC-19, the mcp page and live-checks take the runs; the plan leaves the repo | T4 | T4 | the docs check run by the main session |
| Every live step has a cap of 2 runs and 10 minutes, one never-tried call at most, and a stop line | The live steps | main session | each `### LS` heading names its cap; the plan reviewer counts never-tried calls |
| A button kick counting as an accent, or not, is settled before LS-K2 | LS-K0b | main session | LS-K0b's pass and fail lines each name the next action |
| LS-K2 does not start before LS-K1c passes | LS-K2a | main session | LS-K2a's `Calls:` line cites LS-K1a's passing run |
| The charts come from the library only on the user's yes | Question 4 | T4 | line 31 of the mcp page names them in T4's merge |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: the velocity byte is round((GetAxis(action) - 0.5) x 2 x 127) when GetButtonDown(action) is true and the gate passes, else -1 | code | `docs/clone-hero/code/6142/lane_velocity_337E10.asm` lines 62 to 107; `lane_velocity_337E10.md` paragraph 3 | checked 2026-11-08 |
| P2: the drums input reads the kick lane through that helper with action id 4 | code | `docs/clone-hero/code/6142/drums_input_source.asm` lines 183 to 187 | checked 2026-11-08 |
| P3: the gate is ContainsController(Custom = 0x14, id 6) on the player, or a registered MIDI kit | code | `docs/clone-hero/code/6142/lane_velocity_337E10.asm` lines 51 to 54 and 39 to 41 | checked 2026-11-08 |
| P4: the game's drum map binds the kit's element 22 "Kick" with `element_type` 1 to action 4 "mapperOrange" | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2b-run1\kit-2.jsonl` line 2, the `maps` block | checked 2026-11-08 |
| P5: the pads' elements 0 to 6 bind with `element_type` 0 | run | the same `kit-2.jsonl` line 2 | checked 2026-11-08 |
| P6: Rewired's ControllerElementType is Axis = 0, Button = 1, CompoundElement = 100 | code | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs` lines 290899 to 290906 | checked 2026-11-08 |
| P7: definition 6 "SLInputDrums" holds ten element identifiers, seven axes (Snare, Tom1, Tom2, Tom3, Cymbal1, Cymbal2, Cymbal3) then three buttons (Start, Select, Kick), and no other | data | `C:\Clone Hero\1.1 june\Clone Hero\Clone Hero_Data\level1` bytes 990912 to 992484 (the definition's name, GUID, identifier list, axes list and buttons list, read as strings) | checked 2026-11-08 by a string scan; LS-K1b reads the live list |
| P8: a custom controller is created with its definition's axis and button counts | code | `docs/clone-hero/code/6142/create_custom_controller_1EE9800.asm` lines 67 to 71 | checked 2026-11-08 |
| P9: the game's Rewired has `ControllerMap.CreateElementMap(int, Pole, int, ControllerElementType, AxisRange, bool)`, `MapHelper.AddEmptyMap(ControllerType, int, int, int)` and `MapHelper.GetMap(ControllerType, int, int, int)` | code | `dump.cs` lines 297289 (class at 297102), 304238 and 304107 (class at 303824) | checked 2026-11-08 |
| P10: `CreateElementMap` with an axis element of a custom controller makes a binding the player reads | unchecked | Rewired's API reference calls the overload "controller button or axis assignment"; no run has made one in this game | LS-K1b |
| P11: a pressed button bound to action 4 makes `GetAxis(4)` read 1.0, so today's kick is velocity 127, an accent | unchecked | Rewired's Input Behaviors page says buttons return axis values; `kit_tools.py` line 242 says only that a plain gem's velocity is never read; Senri had no kick dynamics (P16) | LS-K0b |
| P12: an axis element at the ghost value fires the button action and earns the ghost mark | data | `docs/clone-hero/06-scoring.md` SC-14 (Proven in a live run): LS-O1d earned 726 of 726 ghosts on pads pressed as axes; stats in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-o1-run1\` | checked 2026-11-08 |
| P13: with the kit's kick button unpressed, `GetAxis(4)` reads the kick axis's value and not the button's 0 | unchecked | no page or listing reads how Rewired combines two bindings of one action across a player's controllers | LS-K2b |
| P14: controller ids count up from 0 in a launch and a destroy does not move the counter, so after six placeholders and the kit the next create gets id 7 | data | `docs/clone-hero/07-internals.md` IN-81 (Proven in the game's code); `docs/clone-hero/code/6142/create_custom_controller_1EE9800.asm` lines 42 to 47 | checked 2026-11-08 |
| P15: `ch_start` exports `HKCU\Software\srylain Inc.\Clone Hero` as `rewired.reg` and `ch_stop` restores it when it differs | code | `tools/ch_mcp/game_files.py` lines 70 to 77 and 259 to 268 | checked 2026-11-08 |
| P16: a kit run changed that key and the restore put it back | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2b-run1\replies.txt` line 31 ("rewired registry: restored") | checked 2026-11-08 |
| P17: I Am Ahab is a `.mid` with 33 accent kicks and is in the library | unchecked | `docs/clone-hero/live-checks.md` line 49 names it; the db copy was not queried | LS-K0a's go request (a db query by the main session) |
| P18: Won't Get Fooled Again (the Who chart) has 36 ghost kicks and is in the library | unchecked | `docs/clone-hero/06-scoring.md` SC-18 cites ADR 0012 line 23 for the 36 ghost kicks; the db copy was not queried | LS-K1a's go request (a db query by the main session) |
| P19: T5's commit changes `kit_tools.py`, `test_kit_tools.py` and `test_play_chart.py` only, making the kick an axis and the type check expect an axis | code | `git diff --stat main...claude/pl-t5-kick-axis`; the diff's `_type_problem` and `KIT_ELEMENTS` hunks | checked 2026-11-08 |
| P20: the kit agent's allowed calls must equal the names in `kit_methods.json` | code | `tools/ch_mcp/tests/test_frida_attach.py` lines 55 to 69 and 119 to 121 | checked 2026-11-08 |
| P21: the hit handler marks a ghost for a byte from 0 below the threshold and an accent at or above it; the threshold read 100 live | data | `docs/clone-hero/06-scoring.md` SC-13 (Proven in the game's code) | checked 2026-11-08 |
| P22: Senri holds 631 plain kicks and no ghost or accent kick, so LS-O1d says nothing about kick velocity | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-prep\senri-nosp.json`, counted by color and dynamic | checked 2026-11-08 |
| P23: on main the kick is `KitElement(22, "Kick", True)`; a boolean value writes `SetButtonValueById`, a number writes `SetAxisValueById` | code | `tools/ch_mcp/kit_tools.py` line 145; `tools/ch_mcp/agent/kit.ts` lines 442 to 450 | checked 2026-11-08 |
| P24: setup loads the kit's map with the five-argument `LoadMap` called by hand with the method pointer, because il2cpp's stub writes past the arguments | code | `tools/ch_mcp/agent/kit.ts` lines 386 to 408; `docs/clone-hero/07-internals.md` IN-76 | checked 2026-11-08 |
| P25: the user ruled "Proceed, my call" on the EULA | ruling | `docs/agents/rulings/clone-hero.md` line 13 | checked 2026-11-08 |
| P26: a second `ch_kit_setup` in one launch is refused because the kit would not be on id 6 | code | `tools/ch_mcp/kit_tools.py` lines 67 to 76 and 657 to 658 | checked 2026-11-08 |
| P27: the user chose "Plan an in-run axis bind" | ruling | the brief for PL-KICK-PLAN, as the main session quotes the user | checked 2026-11-08 |
| P28: the setup reply's `element_type` is `ActionElementMap.elementType` read as a number | code | `tools/ch_mcp/agent/kit.ts` line 305; `tools/ch_mcp/kit_methods.json` line 35 | checked 2026-11-08 |
| P29: the kit agents are built with `npm run build:kit` and the server never builds them | code | `tools/ch_mcp/README.md` line 79 | checked 2026-11-08 |
| P30: the lane branch `claude/pl-lane-earliest` (tip 598a5748) is merged into main as 02190724, and that merge changed `play_chart.py` and `test_play_chart.py` | code | `git log --oneline -1 main` and `git diff 55423e67 02190724 --stat` | checked 2026-11-08 by the plan reviewer |
| P31: `test_frida_attach.py` pins `kit.ts`'s count of `.invoke(` sites (43) and of `new NativeFunction` sites (1), and fails on any other count | code | `tools/ch_mcp/tests/test_frida_attach.py` lines 62, 67 and 110 to 115; `grep -c "\.invoke(" tools/ch_mcp/agent/kit.ts` gives 43 | checked 2026-11-08 by the plan reviewer |
| P32: `kit_press`'s `press` takes lanes and axis values only, `pad_axis` gives a plain and an accent gem the same 1.0, and `with_axes` fills `Chord.axes` for every gem including kicks | code | `tools/ch_mcp/kit_tools.py` lines 243 to 265 and 564 to 570; `tools/ch_probe/experiments/play_chart.py` lines 927 to 940 | checked 2026-11-08 by the plan reviewer |
| P33: the press log's columns are `PRESS_LOG_COLUMNS` (index, times, score, hit, frames, wall times, `scored_between`, `end_read_ms`, `probe`) and none names a gem's element | code | `tools/ch_probe/experiments/overlay_logs.py` lines 56 to 99 | checked 2026-11-08 by the plan reviewer |

## Out of scope (needs the user's yes to leave out)

1. Reading how Rewired combines two bindings of one action in its code (P13): the run answers it, and the code is Rewired's, not the game's.
2. Definition 1 (the MIDI kit's definition) and the MIDI registration route: not read, not needed if LS-K2 passes.
3. Plain Drums dynamics, by the user's words of 2026-11-07 ("except for the regular drums, that is an intentional deviation from the game. we dont need to test that").
4. A Hydra-side count of library charts with velocity-1 kicks (how much the ghost route is worth): the main session can run it on request; this plan does not.
