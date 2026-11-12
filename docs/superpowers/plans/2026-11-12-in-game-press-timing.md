# Plan: the game times the auto-player's presses (2026-11-12)

Status: for the user's approval. No code changes and no game run until the user says yes (agent rule 3). The ruling "there will be no more runs until you figure this out" still stands; every live step below is its own request.

## The plain version

Today a Python loop outside the game decides when each press goes down. It reads the game's song clock over and over, and when the clock reaches a press's time it sends one message to the kit agent, the script that lives inside Clone Hero and owns the virtual drum kit. Every dropped chord in this series came from that loop being late: a slow file write, then an 87 ms pause while another Python thread held the interpreter.

The new design hands the whole schedule to the kit agent before the first note. The agent hooks one method the game runs once per frame. On each frame it reads the same song clock Python reads today, puts down every press that is due, and lifts every press whose hold has passed. Python no longer sits between the clock and the press. It sets the run up, listens for the agent's report of each press, writes the press log, runs the Star Power watch and does the end-of-run checks.

A Python pause then costs a late log row, never a late press. A pause of the game itself still costs presses, and the log will say so by name.

The user asked for this: "why are we timing presses separately if using the game has been an option this whole time?" and "create a handoff to plan".

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

**1. The schedule goes in once.** Python builds the same list it builds today (`press_plan`, then `with_axes`), and sends it in one `play` message. Each entry carries what `Press` carries and the agent needs: a sequence number, the chord index, the fire time `at_ms`, `last_send_ms`, the kit elements with their values, the order-group gap `after_ms` with the sequence number of the press it follows, and `end_read_after`. The message also carries the hold in frames, the clock's address, the score's address, the score delay and the start lead (`live.START_LEAD_MS`, 150 ms). Every number stays in Python; the agent holds none, as its header comment already promises.

**2. The agent fires on frames.** On every `GameManager.Update` the hook reads the clock once. An entry goes down on the first frame whose clock is at or past its `at_ms`. That is today's predicate without Python's between-frame estimate. An entry whose clock at that moment is past its `last_send_ms` is not written and is reported `dropped`. A later order group goes down on the first frame whose clock is at or past its head's real fire clock plus `after_ms`, which is what "after the first group's down" means today. A press comes up on the first frame at least `hold_frames` after its own down. An entry whose pad is still held waits for the release and goes down on the next frame. On load, the agent skips entries less than the start lead ahead of the clock, as `first_note_index` does today, and reports how many.

**3. The agent reports, Python logs.** For each entry the agent sends one event: sequence number, chord index, `sent` or `dropped`, the clock it fired at, down and up frame, down and up time, the score at the fire frame, the score on the first frame at least the score delay after the release, and the wall time since the previous frame. Python turns each event into the same row `play_chords` builds today, so `PRESS_LOG_COLUMNS` keeps every column and `score_compare` and `ch_compare` read the file unchanged. The seven timing columns of PS3 describe steps that no longer exist, so they stay empty on in-game rows; question 3 covers that. Events queue inside Frida, so a slow Python loses none.

**4. Retiming stays in Python, in one place.** `retime_from_end()` is not copied into the agent. When the agent reports a press whose entry is marked as an end-read point, Python reads the Star Power end with `EngineModel.sp_end` as today, runs `retime_from_end` on the entries not yet reported, and posts a `retime` message that replaces them. The stored end does not move once the game has stamped it, so a late read gives the same answer. The risk is a retimed press coming due before the replacement arrives. The agent guards it: an entry marked `end_read_after` whose replacement has not arrived fires at its planned time and is reported `retime_missed`, loudly, never silently. Row P16 is how much time there really is between a read and the first press it moves; IG0 measures it on the schedule on file.

**5. Stops.** A `stop_play` message, the run cap, a clock that jumps back, and a clock that stops moving each end the agent's play: it lifts every pad, removes the hook and reports why. Python maps each onto today's stop words (`stopped`, `cap`, `frozen`, and a re-sync after a jump back by sending a fresh `play`). Teardown removes the hook too.

**6. What stays in Python.** Setup, finding the engine, the Star Power watch, the press log and its writer thread, the kit log, `ch_play_status`, the end-of-run checks and both compare tools. `play_chords` stays as it is for the keys route and for the offline harness.

**7. The fallback.** The user was also offered keeping the Python loop and giving the player its own process, so no other thread can hold its interpreter. This plan falls back to it in two cases. First, if LS-G1 fails because of the hook itself: a fault, skipped frames, or the frame rate under its floor. Second, by the standing rule that two failed runs on one goal put the design back to the user: after the second failed live run of any step here, the next message is a question that sets this design beside the fallback, not a third run. The fallback gets its own plan then; this one only names it.

## Rulings that bind this plan

- "there will be no more runs until you figure this out" (session a828b0ca). Every live step waits for its own yes.
- "why are we timing presses separately if using the game has been an option this whole time? insane that only now you decide to mention it" and "create a handoff to plan" (session a09f66ea).
- D272 rulings 2 and 5 let the kit agent call methods and set fields where a feature needs them (quoted in `kit.ts`'s header; row P20, unchecked, IG0 opens D272).
- Runs must not need focus, and no input goes through the focused window. The hook writes the kit inside the game, as today.

## Questions

1. **Go ahead with in-game timing, or build the fallback first?** Recommended: in-game timing. It removes the outside loop that every failure came from; the fallback keeps the loop and removes one of its causes.

2. **When a press fires.** Today a press is sent when Python's clock estimate reaches its time, and it goes down some ms later, after the message has crossed into the game. In the new design it goes down on the first frame at or past its time, so every press lands earlier than today by that crossing time. IG0 measures the crossing time from LS-P3's press log before any code is written, and this question is asked again with the number and three real rows. The two sides: (a) fire at the schedule's own time, so the schedule means what it says (recommended); (b) add the measured delay back, so presses land where they land today. What you will see: the press log's `clock_ms_at_press` minus the press time, which today is the Python read, becomes the frame the press went down on; and the game's hit offsets move earlier by the crossing time under (a), or stay under (b).

3. **The press log and `ch_play_status`.** On in-game rows the seven step-timing columns (`wait_read_max_ms` and the rest) go empty, because those steps are gone. Two columns are added: `frame_gap_ms`, the wall time since the frame before the press, and `agent_note`, empty or `retime_missed`. Before, chord 468 of LS-P3 read `dropped` with `wait_read_max_ms` 87.3. After, a stall of the game shows as a large `frame_gap_ms` on the row, and a Python stall shows on no row. `ch_play_status` prints the same two values where it prints the seven today. Recommended: yes.

4. **After the full-song run passes.** Recommended: in-game timing becomes the kit route's only timing and the kit route stops calling `play_chords`, in task IG6. The keys route and the harness keep it. The other side keeps both behind a switch.

5. **Numbers this plan introduces.** Each needs your yes. The frame probe of LS-G1 runs 5 s. Its frame-rate floor is 322.2 frames a second, the lowest five-second band of Run A in TM-49. The agent's events are sent once per frame that has any, with no cap on batch size.

## Tasks

Executors run on Opus and the reviewer of each on Sonnet. IG0 is a read-only scout on Sonnet. IG1 and IG3 are independent and start together after IG0; IG2 needs IG1; IG4 needs IG2 and IG3. No task needs a C++ build. Compiling `kit.js` from `kit.ts` (`npm run build:kit` in `tools/ch_mcp/agent`) is assumed to be the main session's step; row P22 is unchecked and IG0 settles it.

### IG0: four offline readings (read-only; Sonnet)

It opens existing files only and changes nothing. It reports, each with the file and line it came from: (a) from LS-P3's press log, the median and the largest gap between a row's `wall_s_at_read` and its `down_ms`, the crossing time of question 2; (b) from the schedule file LS-P3 played, the smallest gap in ms between an end-read chord and the first press that read moves; (c) from LS-P3's `replies.txt`, the exact tool calls that run made, to confirm every `seen` mark below; (d) whether `tools/ch_mcp/tests/test_frida_attach.py` forbids a hook in `kit.ts`, and what its `CALL_SITES` count pins; (e) whether `node` runs on this machine and which version; (f) D272's rulings 2 and 5 in the decision log, quoted; (g) who compiles `kit.js` today and whether the file is checked in.

Acceptance: a report with all seven answers, each naming its source. Rows P16 to P22 are then updated in this plan by the main session. If (b) is under 100 ms, design point 4 goes back to the user before IG2 starts.

### IG1: the frame scheduler, as plain code with no game in it (TypeScript and Python tests; Opus)

A new file `tools/ch_mcp/agent/press_schedule.ts` holds design points 2 and 5 as one class with no Frida and no il2cpp import. It takes the entries and is called once per frame with the frame number, the clock and the wall time. It returns what to write, what to release and what to report. A new test file `tools/ch_mcp/tests/test_press_schedule.py` drives it through `node` with frame traces and compares against literals. This task uses row P19 (node); if IG0 finds no `node`, the task stops and reports, and the plan goes back to the user.

Acceptance, one named test each: an entry fires on the first frame at or past `at_ms` and not the frame before; an entry past `last_send_ms` on that frame is `dropped` and nothing is written; a later group fires `after_ms` after its head's real fire clock; a press is released exactly `hold_frames` frames after its down; a held pad delays its next press to the frame after the release; entries inside the start lead are skipped and counted; a `retime` replaces only entries not yet fired; an `end_read_after` entry with no replacement fires at its planned time and is reported `retime_missed`; a clock that jumps back, a clock that stops, `stop_play` and the cap each release every pad and report their reason; a 900 ms gap between two frames drops exactly the entries whose `last_send_ms` passed and fires the rest.

### IG2: the kit agent runs the scheduler on the game's frames (TypeScript; Opus; after IG1)

`kit.ts` gains three messages, `play`, `retime` and `stop_play`, and one hook on `GameManager.Update` that calls the IG1 class each frame and does its writes with the existing `write()`. `play` answers at once, so the agent keeps answering `status` and `watch` during a song. The hook is removed on every stop and in teardown. The task also adds a `frame_probe` message for LS-G1: for the seconds the message gives, it logs each frame's number, engine clock, `songTime` and wall time, presses one pad once, and reads it back each frame. This task uses rows P11 to P13 as open, never as settled: nothing in it assumes a same-frame landing or a fresh engine clock. `test_frida_attach.py`'s pins are updated to what IG0 found.

Acceptance: `test_frida_attach.py` passes with the hook listed and every new game call in `ALLOWED_CALLS` and `kit_methods.json`; a test reads `kit.ts` and fails if the agent holds a number the `play` message should carry; a test fails if any stop path leaves the hook attached or a pad down.

### IG3: Python listens to the agent and writes the same log (Python; Opus; with IG1)

`LiveAgent` gains an event listener for messages that carry no request id. A new `play_in_game()` in `play_chart.py` builds the `play` message from the same `Press` list, turns events into rows, reads the Star Power end and posts `retime` (design point 4), and maps the agent's stop reasons onto `RunResult`. `kit_tools.py` builds the messages and owns their numbers. The two columns of question 3 are added to `overlay_logs.py` only after the user's yes.

Acceptance, one named test each, all with a fake agent: every column `score_compare` and `ch_compare` read is filled on a sent row and a dropped row; a split chord's rows match today's rows from `play_chords` on the same schedule, column for column, except the timing columns; an end-read event leads to one `retime` message whose entries equal `retime_from_end`'s output, called and not recomputed; a `retime_missed` event reaches the row and the run's message; an event with an unknown sequence number stops the run as "reply"; each agent stop reason gives the stop word in design point 5; a fake agent whose events arrive 900 ms late still yields every row, in order, with no row marked dropped.

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

Each step is one run, asked for on its own, with its call list shown first. Before each: hold other sessions, check the game dll's MD5 against 106E15E5BF799EA62F916E8CEAF38BC6 (row P18), `ch_reload` after the merges. A failed step is reported with its evidence and no further run is asked for in the same message. Every `seen` mark below rests on row P17 until IG0 confirms it.

### LS-G1: the frame probe, 5 s inside a song (the game; after IG2; only on the user's yes)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-P3 run 1), `ch_attach` (seen: LS-P3 run 1), `ch_kit_setup(player="System")` (seen: LS-P3 run 1), `ch_call("ch_kit_frame_probe", seconds=5)` (never tried), `ch_stop` (seen: LS-P3 run 1)

This is the one step that tries rows P11, P12 and P13. Pass: the hook ran on every frame (the frame number rises by exactly 1 between calls), no fault, the frame rate over the 5 s is at least 322.2, the one press read back as pressed, and the hook is gone after the probe. The report states the frame the press read on, and the gap between the engine clock and `songTime` per frame. Fail: anything else; design point 7's first case then applies. Cap: 1 run.

### LS-G2: the first chords, before any activation (the game; after IG4 and LS-G1's pass)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-P3 run 1), `ch_run_chart(<folder>, <score>, kit_player="System", schedule_json=<schedule cut before the first activation>, path_index=0)` (never tried), `ch_play_status` (seen: LS-P3 run 1), `ch_stop` (seen: LS-P3 run 1)

The schedule file is the LS-P3 one cut offline at the chord before its first activation, so the run ends by itself. Pass: the run ends "done", no row is `dropped`, every sent row has its frames and scores, and every chord's score rose. Fail: any dropped row; the report gives its `frame_gap_ms`. Cap: 1 run.

### LS-G3: through the first activation and its retimed presses (the game; after LS-G2's pass)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-P3 run 1), `ch_run_chart(<folder>, <score>, kit_player="System", schedule_json=<schedule cut after the first activation's last retimed press>, path_index=0)` (seen: LS-G2's passing run), `ch_play_status` (seen: LS-P3 run 1), `ch_stop` (seen: LS-P3 run 1)

Pass: as LS-G2, plus the end-read row holds `end_read_ms`, no row says `retime_missed`, and each retimed press fired at its retimed time. Cap: 1 run.

### LS-G4: the whole song, LS-P3 again (the game; after LS-G3's pass)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-P3 run 1), `ch_run_chart(<folder>, <score>, kit_player="System", schedule_json=<the LS-P3 schedule>, path_index=0)` (seen: LS-G3's passing run), `ch_play_status` (seen: LS-P3 run 1), `ch_wait_stats` (seen: LS-P3 run 1), `ch_compare` (seen: LS-M2 run 1), `ch_stop` (seen: LS-P3 run 1)

Pass: no `dropped` row, no `retime_missed`, and `ch_compare` shows `notes_hit` no lower than LS-P3's 3,766 (row P17). Cap: 1 run. A second run is a new request.

## Out of scope (needs the user's yes to leave out)

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
| Every stop lifts every pad and removes the hook | Design 5 | IG1, IG2 | four stop tests; the hook-left-attached test |
| `play` answers at once, so status still answers | IG2 | IG2 | LS-G2's `ch_play_status` during the song |
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
| P11: a game method can be called from inside an `Interceptor` callback on `GameManager.Update` without a fault | unchecked | `tools/ch_mcp/agent/menu.ts` line 207 attaches a hook but was not read for a call inside it | LS-G1 |
| P12: the frame on which a write made inside the hook reads as pressed | unchecked | `docs/clone-hero/07-internals.md` block IN-77 covers only writes from Frida's thread | LS-G1 |
| P13: the engine clock already holds this frame's value when the hook runs | unchecked | `docs/clone-hero/03-timing.md` block TM-10 says the hop to the engine was not read | LS-G1 |
| P14: the schedule builder holds a 3 ms frame constant and a judge-early allowance of one frame | code | `tools/replay_schedule.cpp` lines 37 and 42 | checked 2026-11-12 |
| P15: the press log's columns are `PRESS_LOG_COLUMNS`, with seven timing columns last | code | `tools/ch_probe/experiments/overlay_logs.py` lines 93 to 145 | checked 2026-11-12 |
| P16: there is enough time between an end read and the first press it moves for one message to arrive | unchecked | `docs/superpowers/plans/2026-11-12-player-stall-fixes.md` only reports end reads; the schedule file was not opened | IG0 |
| P17: LS-P3 made the calls its plan lists, hit 3,766 of 3,767 and dropped chord 468 after a 87.3 ms read | unchecked | `docs/superpowers/plans/2026-11-12-player-stall-fixes.md` lines 215 to 227 report it; the run folder was not opened | IG0 |
| P18: the game dll's MD5 is 106E15E5BF799EA62F916E8CEAF38BC6 | unchecked | `docs/superpowers/plans/2026-11-12-player-stall-fixes.md` line 223 reports it; the file was not hashed | IG0 |
| P19: `node` runs on this machine, so the scheduler can be tested offline | unchecked | `tools/ch_mcp/agent/package.json` names `frida-compile`, which needs it; not run | IG0 |
| P20: D272 rulings 2 and 5 allow the agent's method calls and field sets | unchecked | `tools/ch_mcp/agent/kit.ts` lines 15 and 16 only cite it | IG0 |
| P21: `test_frida_attach.py` allows a hook in `kit.ts` | unchecked | `tools/ch_mcp/tests/test_frida_attach.py` lines 61 to 71 pin call counts; the hook rule was not read | IG0 |
| P22: the main session may compile `kit.js` from `kit.ts` | unchecked | `tools/ch_mcp/agent/package.json` line 12 holds the command; who runs it was not read | IG0 |
| P23: the user asked for this plan and barred runs until it is approved | ruling | the main session's brief for task `IGP-plan`, which quotes "create a handoff to plan" and "there will be no more runs until you figure this out" | checked 2026-11-12 |
| P24: Run A's frame rate stayed between 322.2 and 330.0 with nothing compiling | data | `docs/clone-hero/03-timing.md` block TM-49 | checked 2026-11-12 |
