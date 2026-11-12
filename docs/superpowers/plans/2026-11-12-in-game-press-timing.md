# Plan: the game times the auto-player's presses (2026-11-12)

Status: approved 2026-11-12 (D339): questions 1, 2, 3, 5 and 6 are answered, each with its recommended side, and the "Out of scope" list is approved. Question 4 is still open. IG0 is done; its readings are in the Premise map. The ruling "there will be no more runs until you figure this out" still stands; every live step below is its own request.

## The plain version

Today a Python loop outside the game decides when each press goes down. It reads the game's song clock over and over, and when the clock reaches a press's time it sends one message to the kit agent, the script that lives inside Clone Hero and owns the virtual drum kit. Every dropped chord in this series came from that loop being late: a slow file write, then an 87 ms pause while another Python thread held the interpreter.

The new design hands the whole schedule to the kit agent before the first note. The agent hooks one method the game runs once per frame. On each frame it reads the same song clock Python reads today, puts down every press that is due, and lifts every press whose hold has passed. Python no longer sits between the clock and the press. It sets the run up, listens for the agent's report of each press, writes the press log, runs the Star Power watch and does the end-of-run checks.

A Python pause then costs a late log row, never a late press. A pause of the game itself still costs presses, and the log will say so by name.

The user asked for this: "why are we timing presses separately if using the game has been an option this whole time?" and "create a handoff to plan". Those words come from the handoff, and session a09f66ea's card does not hold them yet (row P26, unchecked, settled by question 1).

## What was read, and what it showed

**The handoff.** `py -I tools/check_handoff.py` on the handoff printed 23 ok, 0 missing, 2 not-pushed and 13 manual. The two not-pushed commits are the ones the handoff itself says are not pushed. No claim in it contradicts the code read below. Its facts about LS-P3's result files were not opened here; they are rows P17 to P19, unchecked, and task IG0 opens them.

**How the kit agent presses today.** `press()` in `tools/ch_mcp/agent/kit.ts` is an async function on Frida's own thread. It writes the pad values once, then wakes every 2 ms (`TICK_MS`) to read `Time.frameCount` through an il2cpp call, and lifts the pads once the frame count has moved on by the hold. Each write and read goes through `Il2Cpp.perform`, which runs on the game's main thread only when the message says `on_main`. `KIT_PRESS_ON_MAIN` in `tools/ch_mcp/kit_tools.py` is `False`, and the hold is `KIT_HOLD_FRAMES = 2`. The agent answers one message at a time and has no hook on any game method.

**Whether a press reaches the game on the same frame.** It does not today. Block IN-77 of `docs/clone-hero/07-internals.md` is proven in live runs: a write from Frida's thread reads as pressed one frame later. Nothing in the repo has tried a write from the main thread or from inside a hooked method, so nobody knows if that lands on the same frame (row P12). Rewired's own documentation says the supported moment is its `InputSourceUpdateEvent`, which "fires before all input sources are updated" ([Custom Controllers](https://guavaman.com/projects/rewired/docs/CustomControllers.html), [InputSourceUpdateEvent](https://guavaman.com/projects/rewired/docs/api-reference/html/E_Rewired_ReInput_InputSourceUpdateEvent.htm)). This plan does not depend on a same-frame landing. It keeps today's one-frame lag. The schedule builder already carries a one-frame allowance (`kFrameMs = 3.0` and `kJudgeEarlyMs` in `tools/replay_schedule.cpp`, row P14). Live step LS-G1 measures the real lag.

**Which hook, which clock.** The hook goes on `GameManager.Update`. Block TM-10 of `docs/clone-hero/03-timing.md`, proven in the game's code, says that method ticks the clock once per call and stores `songTime`. The clock the agent compares against is the one Python reads today: the double at engine+0x100 (TM-13, `EngineModel.song_clock()` in `tools/ch_probe/engine.py`). Python finds the engine as it does now and passes the agent the finished address, so the agent holds no offset of its own. The agent reads it inside the game's process, which is a plain memory read and no game call. What is not known is whether the engine's copy is already this frame's value when `GameManager.Update` returns (TM-10 says the hop from player to engine "was not read"). If it is last frame's, every press is one more frame late. Row P13 holds that, and LS-G1 measures it by logging both clocks on every frame.

**What the outside loop does that must survive.** `play_chords()` in `tools/ch_probe/experiments/play_chart.py` does eight things per message: waits for the clock, reads the score, drops the message if the clock is past `last_send_ms`, sends the press with its later order groups, checks the reply, reads the Star Power end and retimes later presses, reads the score again 5 ms later (`SCORE_READ_DELAY_S`), and writes one log row per press. The design section says where each goes.

**Why the report needs a new path.** `LiveAgent._hand_over` in `tools/ch_mcp/frida_attach.py` gives a waiting request only the message that carries its id. Anything else the agent sends is written to the kit log and dropped. So the agent's per-press reports need a listener that does not exist yet (task IG3).

**Past sessions and plans.** The search of `docs/sessions/`, `docs/superpowers/plans/`, `docs/clone-hero/` and `docs/agents/rulings/` used these phrases: in-game timing, agent-side, inside the agent, per-frame hook, frame-timed, schedule inside the game, own process, and `Interceptor.attach`. No card or plan proposes, tries or drops timing presses inside the game, or giving the player its own process. The hits are the kit moving inside the game on 2026-10-31 (`docs/sessions/2026-10-31-a5f17ca5.md`), the machine recorder running as its own process, and the freak-fixes plan's stall sentence. A search that finds nothing proves little, so this is stated as where the search looked, not as proof that nobody ever discussed it.

**Prior art.** Frida's `Interceptor.attach` runs its callback on the thread that called the hooked function, which here is the game's main thread; this repo already does it in `tools/ch_mcp/agent/menu.ts`, `menuhooks.ts` and `trace.ts`, but only to read. `frida-il2cpp-bridge` 0.14.0, the version in `tools/ch_mcp/agent/package.json`, lists "trace, intercept and replace method calls" as a feature ([npm page](https://npmjs.org/package/frida-il2cpp-bridge)). Unity runs every `Update` once per frame ([Unity manual](https://docs.unity3d.com/Manual/events-per-frame-optimization.html)). Rewired's pages above say when a custom controller's values are read. No source found shows a game method being called from inside such a hook on this game, so that is row P11, unchecked, and LS-G1 is the one step that tries it.

## The design

**1. The schedule goes in once.** Python builds the same list it builds today (`with_axes` adds the pad strengths to the chords, then `press_plan` makes the presses), and sends it in one `play` message. Each entry carries what `Press` carries and the agent needs: a sequence number, the chord index, the fire time `at_ms`, `last_send_ms`, the kit elements with their values, the order-group gap `after_ms` with the sequence number of the press it follows, and `end_read_after`. The message also carries the hold in frames, the clock's address, the score's address, the score delay and the start lead (`live.START_LEAD_MS`, 150 ms). Every number stays in Python; the agent holds none, as its header comment already promises.

**2. The agent fires on frames.** On every `GameManager.Update` the hook reads the clock once. An entry goes down on the first frame whose clock is at or past its `at_ms`. That is today's predicate without Python's between-frame estimate. An entry whose clock at that moment is past its `last_send_ms` is not written and is reported `dropped`. A later order group goes down on the first frame whose clock is at or past its head's real fire clock plus `after_ms`, which is what "after the first group's down" means today. A press comes up on the first frame at least `hold_frames` after its own down. An entry whose pad is still held waits for the release and goes down on the next frame. On load, the agent skips entries less than the start lead ahead of the clock, as `first_note_index` does today, and reports how many.

**3. The agent reports, Python logs.** For each entry the agent sends one event: sequence number, chord index, `sent` or `dropped`, the clock it fired at, down and up frame, down and up time, the score at the fire frame, the score on the first frame at least the score delay after the release, and the wall time since the previous frame. Python turns each event into the same row `play_chords` builds today, so `PRESS_LOG_COLUMNS` keeps every column and `score_compare` and `ch_compare` read the file unchanged. The seven timing columns of PS3 describe steps that no longer exist, so they stay empty on in-game rows; question 3 covers that. Events queue inside Frida, so a slow Python loses none.

**4. Retiming stays in Python, in one place.** `retime_from_end()` is not copied into the agent. When the agent reports a press whose entry is marked as an end-read point, Python reads the Star Power end with `EngineModel.sp_end` as today, runs `retime_from_end` on the entries not yet reported, and posts a `retime` message that replaces them. The stored end does not move once the game has stamped it, so a late read gives the same answer. The risk is a retimed press coming due before the replacement arrives. The agent guards it: an entry marked `end_read_after` whose replacement has not arrived fires at its planned time and is reported `retime_missed`, loudly, never silently. Row P16 is how much time there really is between a read and the first press it moves; IG0 measures it on the schedule on file.

**5. Stops.** A `stop_play` message, the run cap, a clock that jumps back, and a clock that stops moving each end the agent's play: it lifts every pad, removes the hook and reports why. Python maps each onto today's stop words (`stopped`, `cap`, `frozen`, and a re-sync after a jump back by sending a fresh `play`). Teardown removes the hook too.

**6. What stays in Python.** Setup, finding the engine, the Star Power watch, the press log and its writer thread, the kit log, `ch_play_status`, the end-of-run checks and both compare tools. `play_chords` stays as it is for the keys route and for the offline harness.

**7. The fallback.** The user was also offered keeping the Python loop and giving the player its own process, so no other thread can hold its interpreter. This plan falls back to it in two cases. First, if LS-G1 fails because of the hook itself: a fault, skipped frames, or the frame rate under its floor. Second, by the standing rule that two failed runs on one goal put the design back to the user: after the second failed live run of any step here, the next message is a question that sets this design beside the fallback, not a third run. The fallback gets its own plan then; this one only names it.

## Rulings that bind this plan

- "there will be no more runs until you figure this out" (session a828b0ca). Every live step waits for its own yes.
- "why are we timing presses separately if using the game has been an option this whole time? insane that only now you decide to mention it" and "create a handoff to plan" (session a09f66ea).
- D272 rulings 2 and 5 (the decision log, `docs/audit/2026-10-03-fix-decisions.md`, row P20) let the kit agent call game methods ("Agents may call game methods") and set fields ("agents may set game fields where a feature needs it"). Ruling 5 also keeps one rule: "agents read fields by name, never by raw offset". Question 6 asks about that rule, because the clock read in design point 2 is a read at an address.
- Runs must not need focus, and no input goes through the focused window. The hook writes the kit inside the game, as today.

## Questions

1. **Go ahead with in-game timing, or build the fallback first?** Answered: "In-game timing (Recommended)" (D339). Recommended: in-game timing. It removes the outside loop that every failure came from; the fallback keeps the loop and removes one of its causes. The case against it: it is the larger change, it puts timing code inside the game's process where a fault could crash the game, and nothing has yet shown a write from a hook lands. LS-G1 shows that first, before any song is played through. What you will see: no screen changes until a run; a run's press log then has no `dropped` row caused by a Python pause, where LS-P3 had chord 468 dropped after an 87.3 ms read.

2. **When a press fires.** Answered after IG0: "Fire at the schedule time (Recommended)" (D339): side (a), so a press lands up to one frame later than today, about 1.5 ms on average (row P25). Today `live.wait_until` polls the song clock from Python until its estimate reaches the press's own time, `chart_ms + aim_ms` (`Press.at_ms`); the first row of LS-P3's `press-1.csv` (index 0) has `chart_ms` 33,746.56 and a clock read of 33,745.66 ms at the press. Then Python sends the message, and the press goes down a moment later inside the game. In the new design the press goes down on the first frame whose clock is at or past that same time. The two designs differ by Python's polling and the crossing of the message, against the up to one frame (about 3 ms at 330 frames a second) the new design can wait for the next frame; which way the difference leans is not known (row P25). IG0 measures it from LS-P3's press log before any code is written, and this question is asked again with the number and three real rows. The two sides: (a) fire at the schedule's own time, so the schedule means what it says (recommended); (b) add the measured difference back, so presses land where they land today. What you will see: the press log's `clock_ms_at_press`, which today is the Python read, becomes the clock of the frame the press went down on; and the game's hit offsets move by the measured difference under (a), or stay under (b).

3. **The press log and `ch_play_status`.** Answered: "Yes, change them (Recommended)" (D339). On in-game rows the seven step-timing columns (`wait_read_max_ms` and the rest) go empty, because those steps are gone. Two columns are added: `frame_gap_ms`, the wall time since the frame before the press, and `agent_note`, empty or `retime_missed`. Before, chord 468 of LS-P3 read `dropped` with `wait_read_max_ms` 87.3. After, a stall of the game shows as a large `frame_gap_ms` on the row, and a Python stall shows on no row. `ch_play_status` prints the same two values where it prints the seven today. Recommended: yes.

4. **After the full-song run passes.** Recommended: in-game timing becomes the kit route's only timing and the kit route stops calling `play_chords`, in task IG6. The keys route and the harness keep it. The other side keeps both behind a switch, which task IG3 adds in any case as a `timing` argument on `ch_play_chart` and `ch_run_chart` that defaults to today's Python loop until IG6; the choice here is only whether IG6 removes it. What you will see: nothing different on a screen; `ch_run_chart` stops offering the Python loop for the kit route, and every kit run's press log carries the in-game columns of question 3.

5. **Numbers this plan introduces.** Answered: "Accept all four (Recommended)" (D339). Each needs your yes. The frame probe of LS-G1 runs 5 s. Its frame-rate floor is 322.2 frames a second, the lowest five-second band of Run A in TM-49. The agent's events are sent once per frame that has any, with no cap on batch size. IG0's gap of 100 ms between an end read and the first press it moves: a smaller gap sends design point 4 back to you before IG2 starts, and a larger one lets it stand. These are all the numbers; the 150 ms start lead, the hold of 2 frames and the 5 ms score delay already exist in the code.

6. **The clock and score reads inside the game.** Answered: "Allow the two reads (Recommended)" (D339). D272 ruling 5 allowed field sets but kept one rule: "agents read fields by name, never by raw offset" (the check in `tools/ch_mcp/tests/test_frida_attach.py` matches only `add(0x`). Design point 2 has the agent read the engine's clock and the score at addresses Python hands it, which is a read at an address, not by field name. The engine's class has a scrambled name in the dump, so a read by name is not available for it (TM-48 names it `#8288+0x100`). The two sides: (a) allow it for these two reads, because the clock is the one every other tool in the player uses (recommended); (b) read `GameManager.songTime` by field name in the hook, which `clock.ts` already does and TM-10 says `GameManager.Update` stores every frame, and leave the score read to Python as today, a little after the report; this changes which clock the schedule is judged on and loses the score at the fire frame, both by amounts nothing here has measured. The case against (a) is that it breaks a rule you kept on purpose. What you will see: nothing different on a screen under either side; under (b) the `clock_ms_at_press` column would read a different clock than every earlier log.

## Tasks

Executors run on Opus and the reviewer of each on Sonnet. IG0 is a read-only scout on Sonnet. IG1 and IG3 are independent and start together after IG0; IG2 needs IG1; IG4 needs IG2 and IG3. No task needs a C++ build. Compiling `kit.js` from `kit.ts` (`npm run build:kit` in `tools/ch_mcp/agent`) is assumed to be the main session's step, because `kit.js` is not checked in (`tools/ch_mcp/.gitignore` line 2); row P22 is unchecked and IG0 settles who compiles it. Question 6 decides whether design point 2 reads the clock and score at addresses; IG2 waits for its answer.

### IG0: six offline readings (read-only; Sonnet)

It opens existing files only and changes nothing. It reports, each with the file and line it came from: (a) from `press-1.csv` in LS-P3's run folder, the median and the largest of `down_ms` minus `wall_s_at_read` times 1,000 on the sent rows, which is row P25's difference for question 2 (the reviewer's one look at row 0 gave 0.04 ms; the median is what counts); (b) from the schedule file LS-P3 played (the stall plan puts it in `2026-11-11\ls-a2-prep`), the smallest gap in ms between an end-read chord and the first press that read moves, which is row P16; (c) the exact tool calls LS-P3 run 1 made, to confirm each `seen` mark below (row P17); that run's folder holds no `replies.txt`, so the source is the stall plan's Calls line, `kit-1.jsonl` and `state-1.jsonl`, and a call that cannot be confirmed there is marked `never tried`; (d) what `tools/ch_mcp/tests/test_frida_attach.py` pins for a hook in `kit.ts` beyond the `CALL_SITES` count of 43 and the `add(0x` pattern (row P21); (e) that `node` runs in the executor's shell, and its version (row P19); (f) who compiles `kit.js` today (row P22; it is not checked in), and the game dll's MD5 against row P18.

Acceptance: a report with all six answers, each naming its source. Rows P16 to P19, P21, P22 and P25 are then updated in this plan by the main session. If (b) is under 100 ms, design point 4 goes back to the user before IG2 starts (question 5).

### IG1: the frame scheduler, as plain code with no game in it (TypeScript and Python tests; Opus)

A new file `tools/ch_mcp/agent/press_schedule.ts` holds design points 2 and 5 as one class with no Frida and no il2cpp import. It takes the entries and is called once per frame with the frame number, the clock and the wall time. It returns what to write, what to release and what to report. A new test file `tools/ch_mcp/tests/test_press_schedule.py` drives it through `node` with frame traces and compares against literals. This task uses row P19 (node); if IG0 finds no `node`, the task stops and reports, and the plan goes back to the user.

Acceptance, one named test each: an entry fires on the first frame at or past `at_ms` and not the frame before; an entry past `last_send_ms` on that frame is `dropped` and nothing is written; a later group fires `after_ms` after its head's real fire clock; a press is released exactly `hold_frames` frames after its down; a held pad delays its next press to the frame after the release; entries inside the start lead are skipped and counted; a `retime` replaces only entries not yet fired; an `end_read_after` entry with no replacement fires at its planned time and is reported `retime_missed`; a clock that jumps back, a clock that stops, `stop_play` and the cap each release every pad and report their reason; a 900 ms gap between two frames drops exactly the entries whose `last_send_ms` passed and fires the rest.

### IG2: the kit agent runs the scheduler on the game's frames (TypeScript; Opus; after IG1)

`kit.ts` gains three messages, `play`, `retime` and `stop_play`, and one hook on `GameManager.Update` that calls the IG1 class each frame and does its writes with the existing `write()`. `play` answers at once, so the agent keeps answering `status` and `watch` during a song. The hook is removed on every stop and in teardown. The task also adds a `frame_probe` message for LS-G1: for the seconds the message gives, it logs each frame's number, engine clock, `songTime` and wall time, presses one pad once, and reads it back each frame. This task uses rows P11 to P13 as open, never as settled: nothing in it assumes a same-frame landing or a fresh engine clock. `test_frida_attach.py`'s pins (the `CALL_SITES` count of 43, and the `ALLOWED_CALLS` and `kit_methods.json` lists) are updated for what the hook adds, as IG0 (d) finds (row P21). The clock and score reads follow question 6's answer.

Acceptance: `test_frida_attach.py` passes with the hook listed and every new game call in `ALLOWED_CALLS` and `kit_methods.json`; a test reads `kit.ts` and fails if the agent holds a number the `play` message should carry; a test reads `kit.ts` and fails if its `play` case waits for the schedule to finish before replying; a test reads `kit.ts` and fails if any place the scheduler reports a stop does not call the one function that detaches the hook and lifts every pad (the scheduler's own release is IG1's four stop tests).

### IG3: Python listens to the agent and writes the same log (Python; Opus; with IG1)

`LiveAgent` gains an event listener for messages that carry no request id. A new `play_in_game()` in `play_chart.py` builds the `play` message from the same `Press` list, turns events into rows, reads the Star Power end and posts `retime` (design point 4), and maps the agent's stop reasons onto `RunResult`. `kit_tools.py` builds the messages and owns their numbers. It also registers the `ch_kit_frame_probe` worker tool that LS-G1 calls with `ch_call`, which sends IG2's `frame_probe` message. `game_tools.py` gains a `timing` argument on `ch_play_chart` and `ch_run_chart`, default today's Python loop, with `"game"` choosing `play_in_game`; LS-G2 to LS-G4 pass it, and IG6 later removes the default. The two columns of question 3 are added to `overlay_logs.py`, and printed by `ch_play_status` in `game_tools.py`, only after the user's yes.

Acceptance, one named test each, all with a fake agent: every column `score_compare` and `ch_compare` read is filled on a sent row and a dropped row; a split chord's rows match today's rows from `play_chords` on the same schedule, column for column, except the timing columns; an end-read event leads to one `retime` message whose entries equal `retime_from_end`'s output, called and not recomputed; a `retime_missed` event reaches the row and the run's message; an event with an unknown sequence number stops the run as "reply"; each agent stop reason gives the stop word in design point 5; a fake agent whose events arrive 900 ms late still yields every row, in order, with no row marked dropped; `ch_run_chart` with `timing="game"` reaches `play_in_game` and without it reaches `play_chords`; after the user's yes to question 3, a sent row carries `frame_gap_ms`, a `retime_missed` row carries `agent_note`, and `ch_play_status` prints both.

### IG4: the offline proof that a Python stall drops nothing (Python; Opus; after IG2 and IG3)

`stall_harness.py`'s fake Frida script gains a fake game: a `node` process running the real IG1 class on a 330-a-second wall-clock frame ticker, fed by the real `play` message and answering with real events. The harness then runs `play_in_game` on the schedule file with a forced 900 ms block of Python, the block PS1 forced on the old loop. The harness's `cdb` step is not used; LS-P1 showed it hangs.

Acceptance: with the block forced on the listener thread, the run reports zero `dropped` and zero `retime_missed`; the same harness on `play_chords` with the same block reports at least one `dropped`, so the test fails if the stall stops mattering for the wrong reason.

### IG5: the docs (docs; Opus; after each live step, in that step's merge)

`docs/agents/clone-hero-mcp.md` describes the new messages and the changed `ch_run_chart`. After LS-G1, a new block on `docs/clone-hero/07-internals.md` records what the probe measured: the frame a hook write reads on, and whether the engine clock is fresh in the hook. The decision log gets the user's answers.

Acceptance: `hydra_tests.exe "-tc=docs match code*"` passes, run by the main session; the new block carries the label "Proven in a live run" and LS-G1's run folder.

### IG6: the kit route stops using the outside loop (Python; Opus; after LS-G4 passes and question 4's yes)

The kit route calls `play_in_game` only. `play_chords` loses its `kit` branch and keeps the keys route. This plan leaves the repo in this task's merge.

Acceptance: a test fails if `game_tools.py`'s kit route reaches `play_chords`; the docs check passes with the plan removed.

## Test limits

Agents run only `py -m pytest -q -p no:cacheprovider` on the single test files their task names. Nobody runs a full library test. The main session compiles `kit.js` and runs the docs check.

## The live steps

Each step is one run, asked for on its own, with its call list shown first. Before each: hold other sessions, check the game dll's MD5 against 106E15E5BF799EA62F916E8CEAF38BC6 (row P18), `ch_reload` after the merges. A failed step is reported with its evidence and no further run is asked for in the same message. Every `seen` mark below that names LS-P3 rests on row P17 until IG0 confirms it. Every game run waits for `kit.js` to be compiled by the main session (row P22) and, from LS-G2 on, for question 6's answer.

### LS-G1: the frame probe, 5 s inside a song (the game; after IG2 and IG3; only on the user's yes)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-P3 run 1), `ch_attach` (seen: LS-D2d run 1), `ch_kit_setup(player="System")` (seen: LS-D2d run 1), `ch_call("ch_kit_frame_probe", seconds=5)` (never tried), `ch_stop` (seen: LS-P3 run 1)

LS-P3 run 1 called `ch_run_chart`, which attaches and sets the kit up inside itself, so it never called `ch_attach` or `ch_kit_setup` on their own; `ch_attach` and `ch_kit_setup(player="System")` ran on their own after a human start with `ProDrums,Expert` in LS-D2d run 1 (on the Ladies Talk fill check chart; `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\replies.txt`) and in LS-E run 1. This step uses rows P17 and P18 for its marks, P22 for `kit.js`, and tries rows P11, P12 and P13. Pass: the hook ran on every frame (the frame number rises by exactly 1 between calls), no fault, the frame rate over the 5 s is at least 322.2, the one press read back as pressed, and the hook is gone after the probe. The report states the frame the press read on, and the gap between the engine clock and `songTime` per frame. Fail: anything else; design point 7's first case then applies. Cap: 1 run.

### LS-G2: the first chords, before any activation (the game; after IG4 and LS-G1's pass)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-P3 run 1), `ch_run_chart(<folder>, <score>, kit_player="System", schedule_json=<schedule cut before the first activation>, path_index=0, timing="game")` (never tried), `ch_play_status` (seen: LS-P3 run 1), `ch_stop` (seen: LS-P3 run 1)

The schedule file is the LS-P3 one cut offline at the chord before its first activation, so the run ends by itself. Pass: the run ends "done", no row is `dropped`, every sent row has its frames and scores, and every chord's score rose. Fail: any dropped row; the report gives its `frame_gap_ms`. Cap: 1 run.

### LS-G3: through the first activation and its retimed presses (the game; after LS-G2's pass)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-P3 run 1), `ch_run_chart(<folder>, <score>, kit_player="System", schedule_json=<schedule cut after the first activation's last retimed press>, path_index=0, timing="game")` (seen: LS-G2's passing run), `ch_play_status` (seen: LS-P3 run 1), `ch_stop` (seen: LS-P3 run 1)

Pass: as LS-G2, plus the end-read row holds `end_read_ms`, no row says `retime_missed`, and each retimed press fired at its retimed time. Cap: 1 run.

### LS-G4: the whole song, LS-P3 again (the game; after LS-G3's pass)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-P3 run 1), `ch_run_chart(<folder>, <score>, kit_player="System", schedule_json=<the LS-P3 schedule>, path_index=0, timing="game")` (seen: LS-G3's passing run), `ch_play_status` (seen: LS-P3 run 1), `ch_wait_stats` (seen: LS-P3 run 1), `ch_compare` (seen: LS-M2 run 1), `ch_stop` (seen: LS-P3 run 1)

Pass: no `dropped` row, no `retime_missed`, and `ch_compare` shows `notes_hit` no lower than LS-P3's 3,766 (row P17). Cap: 1 run. A second run is a new request.

## Out of scope (approved: "Leave all five out (Recommended)", D339)

- The fallback's own plan (design point 7). It is written only if a fallback case is hit.
- What the Star Power watch's thread ran for 70 to 90 ms in LS-P3. This design makes it harmless to presses; it does not explain it.
- The stall plan's question 3, the storage optimizer and the quiet check, which is still unanswered.
- Writing the kit from Rewired's `InputSourceUpdateEvent` to land on the same frame. It is considered only if LS-G1 shows the one-frame lag costs hits.
- The keys route. It keeps `play_chords`.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| An entry goes down on the first frame at or past its time | Design 2 | IG1 | first-frame test, with the frame before |
| An entry past `last_send_ms` is not written and is reported dropped | Design 2 | IG1 | dropped test; 900 ms frame-gap test |
| A later group fires `after_ms` after its head's real fire clock | Design 2 | IG1 | later-group test |
| A press comes up `hold_frames` frames after its down | Design 2 | IG1 | release test |
| A held pad delays its next press to the frame after release | Design 2 | IG1 | held-pad test |
| Entries inside the start lead are skipped and counted | Design 2 | IG1 | start-lead test |
| The agent holds no number of its own | Design 1 | IG2 | the test that reads `kit.ts` |
| Every stop lifts every pad and removes the hook | Design 5 | IG1, IG2 | IG1's four stop tests; IG2's test that every stop report in `kit.ts` calls the one detach function |
| `play` answers at once, so status still answers | IG2 | IG2 | the test that fails if the `play` case waits for the schedule's end; LS-G2's `ch_play_status` during the song |
| The kit route can choose in-game timing before IG6 | IG3, Question 4 | IG3 | the `timing` argument test |
| The two new columns exist and `ch_play_status` prints them, after the user's yes | Question 3 | IG3 | the `frame_gap_ms` and `agent_note` test |
| The frame probe can be called from the live step | IG2, LS-G1 | IG2, IG3 | `ch_kit_frame_probe` registered by IG3 and sending IG2's `frame_probe`; LS-G1's call list |
| The press log keeps every column the compare tools read | Design 3 | IG3 | filled-columns test; row-for-row test against `play_chords` |
| Events are never lost to a slow Python | Design 3 | IG3, IG4 | 900 ms late-events test; the harness run |
| `retime_from_end` has one owner | Design 4 | IG3 | the retime test calls it and compares |
| A missing retime is loud | Design 4 | IG1, IG3 | `retime_missed` tests on both sides |
| An unknown event stops the run | IG3 | IG3 | unknown-sequence test |
| A Python stall drops no press | Plain version | IG4 | forced 900 ms block: zero dropped, and the old loop drops one |
| The plan does not depend on a same-frame landing | What was read | IG2 | LS-G1 reports the frame; IG2's review reads for the assumption |
| The hook does not slow the game | Design 7 | IG2 | LS-G1's floor of 322.2 frames a second |
| No live step chains two new calls | The live steps | all | each `Calls:` line holds one `never tried` at most |
| Each live step is capped at one run | The live steps | main session | each step's Cap line |
| Two failed runs go back to the user | Design 7 | main session | the message after a second failed run is a question |
| No code or run before the user's yes | Status | main session | the go request for each step |
| New Clone Hero facts go on their page | IG5 | IG5 | the new block with its run folder; docs check |
| The plan leaves the repo when its work ends | IG6 | IG6 | docs check after removal |
| The kit route stops using the outside loop | Question 4 | IG6 | the test that fails if it reaches `play_chords` |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `press()` runs on Frida's thread, ticks every 2 ms and counts the hold in frames | code | `tools/ch_mcp/agent/kit.ts` lines 61 and 535 to 662 | checked 2026-11-12 |
| P2: a press runs off the main thread and holds 2 frames by default | code | `tools/ch_mcp/kit_tools.py` lines 93 and 108 | checked 2026-11-12 |
| P3: the agent answers one message at a time and has no hook | code | `tools/ch_mcp/agent/kit.ts` lines 929 to 969 | checked 2026-11-12 |
| P4: a kit write from Frida's thread reads as pressed one frame later | data | `docs/clone-hero/07-internals.md` block IN-77 | checked 2026-11-12 |
| P5: `GameManager.Update` ticks the clock and stores `songTime` once per call | data | `docs/clone-hero/03-timing.md` block TM-10 | checked 2026-11-12 |
| P6: the engine's song clock is the double at engine+0x100 | data | `docs/clone-hero/03-timing.md` block TM-13 | checked 2026-11-12 |
| P7: Python reads that double as its song clock | code | `tools/ch_probe/engine.py` lines 295 to 299 | checked 2026-11-12 |
| P8: `play_chords` waits, reads the score, drops past `last_send_ms`, presses, reads the end, retimes and logs | code | `tools/ch_probe/experiments/play_chart.py` lines 541 to 825 | checked 2026-11-12 |
| P9: `retime_from_end` moves presses by the read end and applies fallbacks | code | `tools/ch_probe/experiments/play_chart.py` lines 322 to 389 | checked 2026-11-12 |
| P10: `LiveAgent` hands over only the message with the waiting request's id | code | `tools/ch_mcp/frida_attach.py` lines 466 to 488 | checked 2026-11-12 |
| P11: a game method can be called from inside an `Interceptor` callback on `GameManager.Update` without a fault | unchecked | `tools/ch_mcp/agent/menu.ts` line 207 attaches a hook but was not read for a call inside it | IG2 builds the probe, LS-G1 runs it |
| P12: the frame on which a write made inside the hook reads as pressed | unchecked | `docs/clone-hero/07-internals.md` block IN-77 covers only writes from Frida's thread | IG2 builds the probe, LS-G1 runs it |
| P13: the engine clock already holds this frame's value when the hook runs | unchecked | `docs/clone-hero/03-timing.md` block TM-10 says the hop to the engine was not read | IG2 builds the probe, LS-G1 runs it |
| P14: the schedule builder holds a 3 ms frame constant and a judge-early allowance of one frame | code | `tools/replay_schedule.cpp` lines 37 and 42 | checked 2026-11-12 |
| P15: the press log's columns are `PRESS_LOG_COLUMNS`, with seven timing columns last | code | `tools/ch_probe/experiments/overlay_logs.py` lines 93 to 145 | checked 2026-11-12 |
| P16: there is enough time between an end read and the first press it moves for one message to arrive | data | IG0 (b): the LS-P3 schedule `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-prep\The Who - Won't Get Fooled Again (O)-schedule.json`, loaded with `load_schedule` and `press_messages`: 9 end reads; the smallest gap is 7,074.7 ms (read chord 1497, first moved chord 1539), 7,066.7 ms from the read message's last press, against the 100 ms threshold | checked 2026-11-12 |
| P17: LS-P3 made the calls its plan lists, hit 3,766 of 3,767 gems and dropped chord 468 after a 87.3 ms read | data | IG0 (c): the ch server's MCP log for session a09f66ea records `ch_reload`, `ch_start`, `ch_call` (which ran `ch_run_chart`), `ch_play_status`, `ch_wait_stats`, `ch_compare`, `ch_play_status` and `ch_stop`, with no arguments; `kit-1.jsonl` line 2 shows kit player System, the scorestats file shows ProDrums Expert and notes_hit 3,766 of 3,767, and `press-1.csv` line 474 shows chord 468 dropped with `wait_read_max_ms` 87.305; `human=True`, `path_index` and the schedule argument are in no file | checked 2026-11-12 |
| P18: the game dll's MD5 is 106E15E5BF799EA62F916E8CEAF38BC6 | data | `Get-FileHash -Algorithm MD5` of `C:\Clone Hero\1.1 june\Clone Hero\GameAssembly.dll`, run by IG0 and by the main session | checked 2026-11-12 |
| P19: `node` runs in the shell an executor will use, so the scheduler can be tested offline | data | IG0 (e): `node --version` printed v26.8.1, at `C:\Program Files\nodejs\node.exe`, in PowerShell; `frida-compile` 19.1.0 is in `tools/ch_mcp/agent/node_modules` | checked 2026-11-12 |
| P20: D272 rulings 2 and 5 allow the agent's method calls and field sets, and keep the by-name rule | code | `docs/audit/2026-10-03-fix-decisions.md` D272 items 2 and 5 (line 1236 and the numbered items after it) | checked 2026-11-12 |
| P21: `test_frida_attach.py` allows a hook in `kit.ts` | code | IG0 (d): `tools/ch_mcp/tests/test_frida_attach.py` has no hook rule and accepts `Interceptor.attach` (line 981); every `*.ts` in the agent folder is scanned (line 147), so a new `press_schedule.ts` may hold no `add(0x`, no `.invoke(`, no `.method(` lookup, no `ALLOWED_CALLS` and no `NativeFunction`; kit.ts's `ALLOWED_CALLS` must equal `kit_methods.json` both ways (lines 122 to 139); the LoadMap lines are pinned (lines 1059 to 1082) | checked 2026-11-12 |
| P22: the main session compiles `kit.js` from `kit.ts` | code | `tools/ch_mcp/agent/package.json` line 12 holds `build:kit`, `tools/ch_mcp/.gitignore` lines 1 to 3 keep `agent/*.js` and the stamp out of git, and the loaders refuse a stale stamp (`build_stamp.check`); no doc names who runs it, so the main session does, as it does every build (agent rule 14); a fresh worktree has no `node_modules`, so executors test the scheduler with `node` alone | checked 2026-11-12 |
| P23: the user barred game runs until the stall is understood: "there will be no more runs until you figure this out" | ruling | `docs/sessions/2026-11-12-a828b0ca.md` line 34, which holds the whole message | checked 2026-11-12 |
| P26: the user asked for this plan: "create a handoff to plan", and asked "why are we timing presses separately if using the game has been an option this whole time?" | unchecked | `.claude/worktrees/handoff-a09f66ea/docs/handoffs/2026-11-12-in-game-press-timing-handoff.md` lines 52 and 53 quote them; `docs/sessions/2026-11-12-a09f66ea.md` does not hold them | question 1 |
| P25: the press goes down later in the new design than in the old, by up to one frame | data | IG0 (a): on the 2,128 first-group sent rows of LS-P3's `press-1.csv`, `down_ms` minus `wall_s_at_read` x 1,000 has median -0.47 ms and largest +0.73 ms, both stamped in epoch time (`Date.now()` in `kit.ts` line 584, `time.time()` in `play_chart.py` line 687); the new design waits for the next frame, up to about 3 ms at 330 frames a second | checked 2026-11-12 |
| P27: Frida queues the agent's `send` messages in order when Python's handler is slow, so no event is lost | unchecked | `tools/ch_mcp/frida_attach.py` lines 466 to 475 show the handler queueing log lines; Frida's own message queue was not read | LS-G2 |
| P28: one `play` message holding the whole schedule of 2,137 chords and 2,150 presses goes through Frida's `post` | unchecked | `tools/ch_mcp/frida_attach.py` posts today's one-press messages only; no larger message was found | LS-G4 |
| P24: Run A's frame rate stayed between 322.2 and 330.0 with nothing compiling | data | `docs/clone-hero/03-timing.md` block TM-49 | checked 2026-11-12 |
