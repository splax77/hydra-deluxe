# The early fill live check (FL-24, with an offline read for FL-25)

Written 2026-11-12 by planning task efl-plan, session f7baa4ed. Nothing in this plan has started. Every task and every live run waits for the user's yes.

## What this plan answers

Hydra believes that hitting the note that makes Star Power ready a little early can summon a fill that would otherwise be just too late. Nobody has checked that in the game. This plan checks it in Clone Hero 1.1 with one small made-up chart and two short runs.

The question is the FL-24 row on `docs/clone-hero/live-checks.md`: how far early can the hit be and still summon the fill, when Star Power comes ready just after the fill's deadline? The row's pass line says "offered up to 85 ms". This plan can prove it up to 78 ms, and says below why the last few ms cannot be tested.

## Background

The handoff is `docs/handoffs/2026-11-12-early-fill-live-check-handoff.md`. Its checker run gave 9 ok, 2 not-pushed and 3 manual rows. I read every row against the code and the pages. Two things differ from what the handoff says.

First, one manual row is stale. The handoff says question 1 of the Dynamic Hit Window plan was unanswered. It has since been answered "Yes, ship it now" (commit 9965ca83, `docs/agents/rulings/ui-and-display.md`). So the early fill row ships without waiting for this check. This plan does not reopen that.

Second, the handoff calls FL-25's runs "bot runs" and suggests a repeat "with the fill watched". No tool in the repo reads a fill's flag today (see "Reading whether the fill was offered"). So FL-25 gets an offline read of the logs that were already saved, not a new run.

Everything else in the handoff matches. `fill_refuses` and `is_e0` are in `src/core/model.h` at lines 318 and 322. The IG6a merge 8901af45 is on this branch. FL-24 is still labelled "Hydra assumes it, never checked".

## What the code already settles, and what it does not

The game's deadline runs inside the method that adds Star Power (FL-19, SP-35). When a hit completes a phrase, the hit handler calls that method. If the addition carries the meter to half, the method clears every fill that starts at or before "current tick plus four beats".

FL-22 says what "current tick" is. It is the tick half of the engine's clock pair at engine `+0x100`. Every writer of that pair builds it from a time and that time's tick. So the deadline is judged at a time the engine holds, not at the note's place in the chart. That is good news for Hydra's model: an early press is processed at an earlier engine time than the note's own time.

Three things remain open, and only a run settles them.

1. Which time is in the pair when the hit handler runs. One of the pair's four writers, the three-step loop (`docs/clone-hero/code/6142/three_step_loop_20F5330.md`), stores a stepped event time, calls an engine slot, then puts the saved clock back. Whether a hit is ever handled inside that loop was not read. If it is, the deadline could see a time other than the press's frame.
2. How far the press's own frame trails the moment the press is sent (TM-27 calls this the lag, and it was never measured).
3. How early a press can be and still hit the note at all. TM-26 and TM-27 measured it on notes a full second from their neighbours: presses up to 88.43 ms early hit. The stored side there is 85.72 ms.

Point 3 is why 85 ms exactly cannot be tested. To beat a deadline that is 85 ms before the note, the press must land more than 85 ms early. To hit the note, it must land less than about 85.72 ms early plus the lag. That leaves under 1 ms, and a press on the game's frames lands up to 3 ms after its asked time (the user's 2026-11-12 ruling text on in-game timing gives the 3 ms). So the top case here is 78 ms.

## The four questions the brief asked

### Which chart

A new probe chart, "Early Fill Walk". No existing probe chart has a Star Power phrase ending a known number of ms after a fill's deadline. Trippin SP Spike is a real song's chart with one such spot of unknown size, so it cannot walk the distance.

The chart is built the way the other probe charts are: Resolution 480 at 125 BPM, so one tick is exactly one ms (`tools/ch_probe/probe_songs.py` lines 13 to 15 and 55 to 56). Four beats are then 1,920 ms.

It holds five cases, one after another, each about 23 s long. Each case has the same shape:

- a lone kick inside its own Star Power phrase, which fills a quarter of the meter;
- 2,000 ms later, a second lone kick inside its own phrase. This is the SP-ready note: hitting it carries the meter to half;
- the case's fill, 480 ms long, whose start is placed so that its deadline (start minus 1,920 ms) sits a chosen number of ms before the SP-ready note, with a lone kick exactly at the fill's end tick. That kick is the fill's last note. A written fill lands on a chord at or just after its end (FL-03), so the fill needs that chord to exist;
- a second fill 6,000 ms after the case fill's last note, the flush fill, with its own last-note kick. Its deadline is seconds after Star Power is ready, so the game always offers it;
- 12,000 ms of rest, so Star Power has run out before the next case.

The chosen distances are -30 ms for case A (Star Power ready 30 ms before the deadline, a control), then 30, 60, 70 and 78 ms after the deadline for cases B, C, D and E. The first note sits at 20,000 ms, so the run's setup finishes before it. The SP-ready note is at least 1,000 ms from both neighbours, so its window side is the 85.72 ms TM-26 measured.

The auto-player hits every note. A correct press on a fill's last note while Star Power is ready and the fill is still flagged activates Star Power (FL-31). So each case has two outcomes. If the game offered the case's fill, Star Power turns on at that fill's last note. If not, it turns on 6 s later at the flush fill.

Star Power lasts 7,680 ms on half a meter here. The bar is eight measures in any signature (SP-15), half is four, and the probe charts write one 4/4 time signature (`tools/ch_probe/probe_chart.py` lines 104 to 117), so four measures are 4 x 1,920 ms. That is shorter than the 12,000 ms rest, so each case starts with an empty meter.

Task T1 builds the chart. The main session then installs it with `ch_install_chart` in live step LS-F0.

### How the SP-ready note gets pressed early, without focus

The tool already exists. `ch_edge_probe` plays a schedule on the kit and presses each chord named in a probes file at a lead the file gives (`tools/ch_mcp/game_tools.py` line 1264, `tools/ch_probe/experiments/edge_probe.py`). It moves the chord's time in the schedule, so the kit agent fires it on the game's own frames like any other press. No key is sent and the window is never focused. It refuses a chord that is timed from a Star Power end, which the SP-ready notes here are not expected to be (task T3 checks).

Two gaps keep it from being used as it is. It has never run live on in-game timing (`docs/agents/clone-hero-mcp.md` says so). And it needs `ch_attach`, `ch_kit_setup` and `ch_sp_watch_start` called one by one first, which would put two never-tried calls in one live step.

The smallest change is to let `ch_run_chart` take an optional `probes_json`. `ch_run_chart` already does the attach, the kit setup, the Star Power watch and the play in one call. With the new argument it moves the probe chords with `edge_probe.probe_chords` and marks their rows, exactly as `ch_edge_probe` does. That is task T2.

Every probed note is asked for 84 ms early. TM-26's presses at 80.91 and 82.48 ms early hit. A frame-timed press lands at or after its asked time, so the real lead will be 84 ms or a little less. The press row records the real lead.

### Reading whether the fill was offered

No tool reads the fill's flag. I searched `tools/ch_mcp` and `tools/ch_probe` for the fill record, its active byte, the chord flag 0x800 and the deadline method's address, in the Python files, the agents and `constants.py`. The watcher's columns (`WATCH_COLUMNS` in `tools/ch_probe/experiments/overlay_logs.py`) hold the meter, its half, the active byte of Star Power itself, and the clocks. `ch_trace_start` hooks note hits. Neither sees a fill.

So this plan reads the fill's fate from where Star Power turns on. The Star Power watch logs every change of the active byte with the song clock. `ch_sp_between(start_ms, end_ms)` answers whether an activation fell in a stretch of the song. An activation at the case's fill means it was offered. An activation at the flush fill means it was not.

Reading the flag directly would need new agent code that walks the engine's fill list. That is on the Out of scope list.

For FL-25 the same idea works on logs that already exist. The three runs' watch logs are saved beside their compare files. They show the clock at which the meter reached half. Task T4 compares that with the deadline of the fill at tick 34944.

### Which early amounts, in how few runs

Two runs of the same chart answer the pass line.

The first run (LS-F2) presses everything on time. Case A should get its fill. Cases B to E should not, because Star Power comes ready after the deadline. This proves the chart does what it was built to do, and it is the "not early" control for every distance at once.

The second run (LS-F3) presses the SP-ready notes of B to E 84 ms early. If Hydra's model is right, all four get their fill.

## The tasks

Tasks T1 and T2 are independent and go to executors at the same time. T3 is the main session's, after T1 and T2 are merged. T4 needs one file from T3 (step 6), so its executor starts after T3 has written it.

### T1: Build the Early Fill Walk chart

An executor adds a third probe song to `tools/ch_probe/probe_songs.py`, named "Early Fill Walk", with the layout in "Which chart" above. The layout's numbers live once, in a small table in that file: the five distances, the 2,000 ms between the two phrase notes, the 480 ms fill length, the 6,000 ms to the flush fill, the 12,000 ms rest and the 20,000 ms lead-in. The song folder comes from `write_song_folder`, like the other two. Its manifest lists, for each case, the SP-ready note's time, the case fill's start, deadline and last-note time, and the flush fill's last-note time.

`tools/ch_probe/probe_chart.py` cannot write a Star Power phrase or a drum fill today: `chart_text` writes only `[ExpertDrums]` note lines (premise P14). The executor adds both there as optional arguments, so Window Map and Edge Walk come out byte for byte as before. A phrase is a line `<tick> = S 2 <length>` and a fill is `<tick> = S 64 <length>` in the same section (SP-01, FL-01).

The executor reads FL-01 onward on `docs/clone-hero/04-fills.md` for how a written fill and its last note are laid out in a `.chart`, and SP-01 to SP-04 on page 05 for the phrase. It cites the blocks it used in the code's comments by id.

The executor runs only the Python tests of `tools/ch_probe/tests/test_probe_songs.py`. It does not run `probe_songs` itself, because that runs `hydra_replay` (agent rule 14).

Acceptance criteria:

- A test builds the chart text and checks, for each of the five cases, that the case fill's start minus 1,920 equals the SP-ready note's tick minus that case's distance. It fails if any distance is off by one tick.
- A test checks that every SP-ready note is at least 1,000 ticks from the note before it and the note after it.
- A test checks that the first note is at tick 20,000 and that each case's first note comes at least 12,000 ticks after the previous flush fill's last note.
- A test checks the manifest holds the four times per case named above, read from the same table the chart is built from.
- The existing tests for Window Map and Edge Walk still pass unchanged.

### T2: `ch_run_chart` takes a probes file

An executor adds an optional `probes_json` argument to `ch_run_chart` in `tools/ch_mcp/game_tools.py` and its registration in `tools/ch_mcp/server.py`. With the argument, the call loads the probes with `edge_probe.load_probes`, moves the chords with `edge_probe.probe_chords`, and gives the play session `ProbeRun.mark_row`, the same three steps `ch_edge_probe` takes. Without it, the call behaves as today. The move has one owner, `edge_probe.py`; T2 adds no second copy of it. The executor updates the `ch_run_chart` paragraph on `docs/agents/clone-hero-mcp.md` in the same commit.

The executor runs only `tools/ch_mcp/tests/test_game_tools.py` and `tools/ch_mcp/tests/test_server.py`.

Acceptance criteria:

- A test calls `ch_run_chart` with a probes file on the fake game and checks the probe chord's press is planned at chord time minus the asked lead, and that its press row carries the asked lead in the `probe` column.
- A test checks a probes file that names a chord not in the schedule is refused before the attach, with `edge_probe`'s own words, and that nothing was started.
- A test checks a probe on a chord timed from a Star Power end is refused the same way.
- A test checks that a call with no `probes_json` plans the same presses as before the change.
- A test checks the reply holds the `probes:` line from `edge_probe.probe_lines`.

### T3: Prepare the run's inputs (main session)

The main session does this, because it needs `hydra_replay` and a build. It works in a new folder, `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\efl-prep` (or the day's date).

1. Run `py -m tools.ch_probe.probe_songs --out <prep folder>` to write the Early Fill Walk folder there. Nothing is written into the game's song folder in this step.
2. Run `hydra_replay` dump, score and schedule on that chart. Keep the three files in the prep folder.
3. Read the dump and the score file and check Hydra sees the chart as built: exactly ten chords marked `is_fill` (five case fills, five flush fills) and no other, each with a `fill_start` equal to the manifest's, ten phrase-end chords, and the SP-ready note of each case a phrase end. This settles premise P28 and half of P11 on Hydra's side. It does not show what the game does, which LS-F2 shows.
4. Pick a path index whose schedule holds no refusal, no `declined` chord and no press timed from a Star Power end on the five SP-ready chords. A declined chord would leave a fill's last note unpressed and make an offered fill look not offered. If no path qualifies, stop and report; do not start a run.
5. Write `probes.json` in the prep folder: the SP-ready chords of cases B, C, D and E, each with `lead_ms` 84.
6. Run `hydra_replay score` on the Trippin SP Spike chart (`C:\Clone Hero\songs\Hydra Probe\Trippin SP Spike\notes.chart`) and keep the file in the prep folder as `trippin-score.json`. Its chord at tick 34944 carries `fill_start` (`tools/replay.cpp` lines 136 and 461). T4 reads it from there, because that chart writes no fill of its own.

Acceptance criteria:

- The prep folder holds the chart folder, the dump, the score file, the schedule, `probes.json` and `trippin-score.json`.
- A short `prep-notes.txt` there records the path index, the count of `declined` chords (which must be 0), the five SP-ready chord indexes with their chart times, and the Trippin fill start tick.
- If step 3, 4 or 6 fails, the main session stops and tells the user what failed. No live step starts.

### T4: Read FL-25's saved logs

An executor reads the three saved runs under `tools/ch_probe/experiments/results/overlay-spike/`: `run1_cal0_sp_watch.csv`, `run2_cal30_sp_watch.csv` and `run2b_vid30_sp_watch.csv`. These hold `clock_s`, `sp_amount_u32`, `sp_max_u32` and `sp_active_byte` (premise P20). It needs no game and no build.

For each run it finds the first row before the activation at tick 38016 where `sp_amount_u32` had reached half of `sp_max_u32` with `sp_active_byte` 0. It takes the start tick of the fill that ends at tick 34944 from `trippin-score.json` in T3's prep folder (the chord at tick 34944, field `fill_start`). The chart writes no fill itself, so its file in the game's folder cannot give it. It works out the deadline as that start minus four times the chart's Resolution (192), turned into ms with the tempo map in `C:\Clone Hero\songs\Hydra Probe\Trippin SP Spike\notes.chart`. It then says, per run, whether the game's meter reached half before or after that deadline, and by how many ms. The fill's start is Hydra's. Whether the game placed its own fill at the same tick was not read, and the Rule says so (premise P29).

It writes the result onto FL-25's block on `docs/clone-hero/04-fills.md` in the same commit. If the meter reached half after the deadline in all three runs, FL-25's rule says so: the fill was cleared by the plain deadline (FL-19), because the presses were on time and Hydra's path counted on an early one. If the meter reached half before the deadline in any run, the rule says the fill was not offered although Star Power was ready in time, and the cause stays unknown. The FL-25 row on `docs/clone-hero/live-checks.md` is updated to match.

Acceptance criteria:

- FL-25's Evidence line names the three CSV files, the row used in each, `trippin-score.json`, the fill's start tick, the deadline in ms, and the three differences.
- The block's Rule states which of the two outcomes was found, and claims no more than the three logs show.
- If a log lacks the columns needed, or `trippin-score.json` is missing or holds no `fill_start` at tick 34944, the executor stops and reports which file and what was missing. It does not guess.

### T5: Write the result back

After LS-F3 (or after the step that ended the plan early), an executor updates the pages in one commit, with the run folders as evidence.

- FL-24 on `docs/clone-hero/04-fills.md`: the Rule, Label and Evidence lines take the result. On a pass the label becomes "Proven in a live run" and the rule says the distances at which an early hit summoned the fill, the measured lead of each, and that distances from 78 to 85 ms were not tested and why. On a fail the rule says the largest distance that worked and the smallest that did not, and its Ruling line reads "open split: see questions page" with a new Q number, added to `docs/clone-hero/questions.md` in the same commit. An inconclusive run leaves the label as it is and adds what was seen to the Evidence line.
- The FL-24 row on `docs/clone-hero/live-checks.md` is removed on a pass or a fail and reworded on an inconclusive run. Its stale words "menu start, key-played" go either way: the run is a direct human start on the kit.
- The block counts and the "Last write-back" dates on `docs/clone-hero/README.md`, page 04 and the live-checks page are brought up to date.
- Any other game fact the runs showed (for example the real lead of a frame-timed probe press) goes to its own page's block, not into FL-24.
- This plan and the handoff leave the repo in the same merge, and lines that still name them say how to read them with `git show`.

Acceptance criteria:

- Every number in FL-24's new text can be found in a named file of the run folders.
- No Hydra source file changes in this plan. A fail is reported to the user as a found split, whole, and waits for their ruling.

## The live steps

Each step waits for the user's go. The go request shows the step's `Calls:` line as written here and names its never-tried call. Each step runs at most twice. A second run happens only when the first ended for a reason outside the question (a refused quiet check, a dropped press). After two runs without a verdict the main session stops and puts the design to the user. Every step ends with `ch_stop`, and its restore lines and kit teardown lines must all read ok.

No step uses the key route. Nobody needs to type "allow key run". The game is launched minimized and driven through the virtual kit, so the user keeps using the machine.

### LS-F0: Install the chart

Calls: `ch_install_chart` (never tried)

The main session calls `ch_install_chart` with the prep folder's chart folder and the name "Early Fill Walk". The game is not running.

Pass: the reply starts "installed" and names the chart file under `C:\Clone Hero\songs\Hydra Probe\Early Fill Walk`. Fail: "refused", "install failed" or "not finished". On a fail, stop and report the reply.

### LS-F1: The game loads the chart for a human player

Calls: `ch_start` (never tried), `ch_attach` (seen: LS-E run 1), `ch_state` (seen: LS-D0 run 1), `ch_stop` (seen: LS-E run 1)

The main session starts the song with `human=True`, attaches, reads the state once, and stops. Nothing is pressed. The three seen markers were checked against those runs' `replies.txt` files (premise P15).

Pass: `ch_start` says patched, `ch_state` shows a live song with a moving clock, and `ch_stop` restores everything. Fail: anything else. On a fail, stop and report.

### LS-F2: Everything on time

Calls: `ch_start` (seen: LS-F1's passing run), `ch_run_chart` (never tried), `ch_play_status` (seen: LS-D2d run 1), `ch_sp_watch_status` (seen: LS-D2d run 1), `ch_sp_between` (seen: LS-D2d run 1), `ch_stop` (seen: LS-D2d run 1)

The main session starts the song with `human=True` and calls `ch_run_chart` with the prep folder's score file and schedule and `kit_player="System"`. When the play session has ended, it reads the status, then asks `ch_sp_between` once per case for the stretch from 200 ms before the case fill's last note to 200 ms after it. It copies the session folder to `ch-mcp-runs\<date>\ls-f2-run1` before `ch_stop`.

This step uses premise P11, which is unchecked until it runs.

Pass, all of:

- no press was dropped, and every SP-ready note's press row shows its score step;
- the watch shows the meter reaching half at each SP-ready note's press;
- case A has an activation at its case fill;
- cases B, C, D and E have no activation at their case fill and one at their flush fill.

Fail, and what each failure means:

- Case A has no activation at its fill. The chart or the fill's layout is wrong. Stop; LS-F3 does not run.
- Any of B to E activates at its case fill on an on-time press. The game is more lenient than FL-19 reads. Stop and report it; that is a finding of its own, and T5 writes it up.
- A missed SP-ready note or a dropped press. The run is void; this is the one case for a second run.

### LS-F3: The SP-ready notes pressed 84 ms early

Calls: `ch_reload` (seen: LS-E run 1), `ch_start` (seen: LS-F2's passing run), `ch_call` running `ch_run_chart` with `probes_json` (never tried), `ch_play_status` (seen: LS-F2's passing run), `ch_sp_watch_status` (seen: LS-F2's passing run), `ch_sp_between` (seen: LS-F2's passing run), `ch_stop` (seen: LS-F2's passing run)

This step starts only when T2 is merged and LS-F2's passing run folder exists. The main session reloads the worker so T2's code is live, starts the song as in LS-F2, and calls `ch_run_chart` through `ch_call` with the same files plus the prep folder's `probes.json`. `ch_call` is used because this session's tool list does not show a new argument after a reload. It reads the run the same way as LS-F2 and copies the session folder to `ls-f3-run1`.

This step uses premise P13, which is unchecked until it runs.

For each of cases B to E, read two things. The measured lead is the press row's `chart_ms` minus `clock_ms_at_press`, which `ch_play_status` prints on its probe lines. The outcome is where Star Power turned on.

Each case then gets one of three verdicts. The 6 ms below is the planner's own margin, two frames of 3 ms: one for the press to land and one for the engine to judge it. Nobody measured it (premise P27). The offered verdict does not use it. Only the not-offered verdict and the no-verdict line do, and a margin that is too small could only make a false fail, which is reported to the user as a found split and changes no code until they rule.

- Offered: Star Power turned on at the case fill.
- Not offered: Star Power turned on at the flush fill, the note was hit, and the measured lead was more than the case's distance plus 6 ms.
- No verdict: the note was missed, or the measured lead was not more than the distance plus 6 ms.

Pass for FL-24: cases B, C and D (30, 60 and 70 ms) are all "offered". Case E (78 ms) is reported with its verdict and does not decide the pass, because its margin is one frame.

Fail for FL-24: any of B, C or D is "not offered". The result is the largest distance offered and the smallest not offered.

Inconclusive: any of B, C or D has no verdict and none is "not offered". This is the one case for a second run.

Case A is not probed. It must activate at its case fill again; if it does not, the run is void.

## Questions

1. **May a human-player run play the new probe chart?** `docs/agents/clone-hero-mcp.md` says a human run plays only the test chart until page 07 says whether a human score is posted anywhere, and it lists the library charts you allowed by name. Early Fill Walk is a made-up, silent chart in the `Hydra Probe` folder, of the same kind as Edge Walk Pair, which LS-E run 1 played with a human player on 2026-11-07. The game's network stays as it is, per your 2026-10-31 ruling. Recommended: yes.

2. **May the runs start?** There are four steps: one install with the game off, one load check of a few seconds, and two plays of about 2.5 minutes each. Each is capped at two runs. Each go is still asked on its own, with the step's call list. Recommended: yes.

3. **Do you accept the Out of scope list below?** The item that narrows the live-checks row is the first one: the plan proves the early fill up to 78 ms, not 85.

## Out of scope

These are left out. Each needs the user's approval to stay out.

- Distances between 78 and 85 ms. The note's own window ends near 85.72 ms early, so a press cannot both beat an 85 ms deadline and hit the note with a frame to spare. FL-24's new text will say the range was not tested.
- Reading a fill's flag (the record's active byte or the chord's 0x800 bit) directly. It needs new agent code that walks the engine's fill list. The runs read where Star Power turns on instead.
- A new live run for FL-25. T4 reads the saved logs. If they show Star Power was ready in time and the fill still was not offered, a flag-reading tool and a new run become their own plan.
- Any change to Hydra's early fill reach, the E0 badge or the early fill row. A fail here is reported as a found split and waits for a ruling.
- The 60 ms slack (`kEarlyFillSlackMs`). It is a display choice of Hydra's, not a game rule.
- Reading which clock value the three-step loop leaves in place during a hit. The run answers the question that matters without it.
- Early hits at other tempos, on notes with close neighbours (whose window is smaller), or on a `.mid` chart.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| Each case fill's deadline sits the chosen distance from its SP-ready note | "Which chart", T1 | T1 | T1's first test, which fails on a one-tick error |
| Every SP-ready note is at least 1,000 ms from its neighbours | "Which chart", T1 | T1 | T1's second test |
| The first note is at 20,000 ms and each case starts at least 12,000 ms after the last flush fill | "Which chart", T1 | T1 | T1's third test |
| The layout's numbers live once and the manifest reads the same table | T1 | T1 | T1's fourth test |
| The existing probe songs do not change | T1 | T1 | T1's fifth criterion: their tests pass unchanged |
| A probed chord is pressed at its asked lead and its row is marked | T2 | T2 | T2's first test |
| A bad probes file is refused before anything starts | T2 | T2 | T2's second and third tests, one per refusal |
| A call without probes behaves as before | T2 | T2 | T2's fourth test |
| The probe move has one owner | T2 | T2 | The reviewer reads that `ch_run_chart` calls `edge_probe.probe_chords`; T2's first test fails if it does not |
| No run starts on a schedule with a declined chord, a refusal or a Star Power timed probe chord | T3 step 4 | T3 | `prep-notes.txt` records a declined count of 0; T2's third test covers the timed chord |
| No go is asked on a "seen" marker nobody opened | T3 step 6, LS-F1 | T3 | `prep-notes.txt` lists each call found in the two `replies.txt` files |
| The run never needs the game window focused and uses no key route | "The live steps" | LS-F1 to LS-F3 | Every `Calls:` line holds kit-route calls only; `ch_stop` prints the foreground count |
| Each live step runs at most twice, then the design goes to the user | "The live steps" | LS-F0 to LS-F3 | The run folders: no step has a `run3` |
| LS-F3 does not start before LS-F2 passes and T2 is merged | LS-F3 | LS-F3 | Its go request names LS-F2's passing run folder |
| A case counts as "not offered" only when the note was hit with a lead more than the distance plus 6 ms | LS-F3 | LS-F3 | The press rows in `ls-f3-run1`, quoted in FL-24's Evidence |
| FL-25's block claims no more than the three logs show | T4 | T4 | T4's first two criteria; the reviewer opens the three CSV rows |
| T4 stops when a log or the chart cannot be read | T4 | T4 | T4's third criterion |
| The result goes onto FL-24 in the same commit as the work | T5 | T5 | The commit holds page 04 and the run evidence lines together |
| A fail opens a question and changes no Hydra code | T5, Out of scope | T5 | The commit's file list holds no file under `src/` |
| The plan and the handoff leave the repo when the work is done | T5 | T5 | `git grep` for their paths finds nothing |
| The chart does what it was built to do before any early press is read | "Which early amounts" | LS-F2 | LS-F2's pass list: case A offered, B to E not offered on time |
| Hydra reads the built chart as ten fills and ten phrase ends with the fill starts the chart intended | T3 step 3 | T3 | `prep-notes.txt` and the dump |
| FL-25's fill start comes from Hydra's score file, not from a chart that holds no fill | T3 step 6, T4 | T3, T4 | T4's Evidence line names `trippin-score.json` |
| The early fill row is not reopened | Background | none needed | This plan has no task that touches the Dynamic Hit Window plan |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The game clears a fill when a Star Power gain reaches half and the fill starts at or before the current tick plus four beats | data | `docs/clone-hero/04-fills.md` block FL-19, and `docs/clone-hero/code/6142/fill_deadline_20DDB70.md` lines 5 to 17 | checked 2026-11-12 |
| P2: The "current tick" is the tick half of the engine's clock pair at +0x100, always written from a time | data | `docs/clone-hero/04-fills.md` block FL-22 | checked 2026-11-12 |
| P3: One writer of that pair, the three-step loop, stores a stepped time, calls an engine slot and restores the clock | code | `docs/clone-hero/code/6142/three_step_loop_20F5330.md` lines 7 to 9 | checked 2026-11-12 |
| P4: A phrase is credited when its last chord is hit, through the drum engine's deadline method | data | `docs/clone-hero/05-star-power.md` block SP-35 | checked 2026-11-12 |
| P5: A correct press on a flagged fill's last note while Star Power is ready activates it | data | `docs/clone-hero/04-fills.md` block FL-31 | checked 2026-11-12 |
| P6: On notes 1,000 ms from both neighbours, presses 80.91 and 82.48 ms early hit, and the latest early hit seen was 88.43 ms | data | `docs/clone-hero/03-timing.md` blocks TM-26 and TM-27 | checked 2026-11-12 |
| P7: Hydra refuses a fill only when Star Power is ready more than `kHitWindowMs` after the deadline, and E0 reads `kEarlyFillSlackMs` | code | `src/core/model.h` lines 316 to 324 | checked 2026-11-12 |
| P8: `ch_edge_probe` moves the named chords by the asked lead and plays them on the kit route with no key press | code | `tools/ch_mcp/game_tools.py` lines 1264 to 1314 and `tools/ch_probe/experiments/edge_probe.py` lines 111 to 162 | checked 2026-11-12 |
| P9: No tool reads a fill's active byte or the chord's 0x800 flag; the watch's columns hold only the meter, Star Power's own active byte and the clocks | code | `tools/ch_probe/experiments/overlay_logs.py` lines 50 to 69; a grep of `tools/ch_mcp` and `tools/ch_probe` for the fill record, the flag and `20DDB70` found nothing | checked 2026-11-12 |
| P10: A lone note inside its own phrase earns a quarter of the meter, so two of them reach half | data | `docs/clone-hero/05-star-power.md` blocks SP-04 (the phrase's last chord gets the award), SP-12 (a phrase adds a quarter bar) and the guard in `docs/clone-hero/code/6142/fill_deadline_20DDB70.md` lines 11 to 14 (an addition that reaches half counts) | checked 2026-11-12 |
| P11: The game offers the case A fill and every flush fill as the chart writes them, and the auto-player's hit on their last note activates | unchecked | `docs/clone-hero/04-fills.md` FL-01 and FL-03 say a written fill lands on a chord at or just after its end, but nothing says the game offers it untouched | LS-F2 |
| P12: Half a meter lasts 7,680 ms at 125 BPM in 4/4, shorter than the 12,000 ms rest, so each case starts with an empty meter | data | `docs/clone-hero/05-star-power.md` block SP-15 (eight measures per bar in any signature) and `tools/ch_probe/probe_chart.py` lines 104 to 117 (one 4/4 signature) | checked 2026-11-12 |
| P13: On in-game timing a probe press lands within 3 ms after its asked time and its row holds `clock_ms_at_press` | unchecked | `docs/agents/clone-hero-mcp.md` says the edge probe "has not yet run that way in a live game" | LS-F3 |
| P14: `tools/ch_probe/probe_chart.py` cannot write a Star Power phrase or a drum fill: `chart_text` writes only `N` lines of `[ExpertDrums]` | code | `tools/ch_probe/probe_chart.py` lines 126 to 156 | checked 2026-11-12 |
| P15: LS-E run 1 used `ch_attach` and `ch_stop` on a human direct start and `ch_reload`; LS-D0 run 1 used `ch_state` on a human direct start; LS-D2d run 1 used `ch_play_status`, `ch_sp_watch_status`, `ch_sp_between` and `ch_stop` on a human direct start with kit player System | run | `ls-e-run1\replies.txt` (2026-11-07), `ls-d0-run1\replies.txt` (2026-10-31) and `ls-d2d-run1\replies.txt` (2026-11-06), each under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\` | checked 2026-11-12, by opening each file. `ls-g4-run1` holds no `replies.txt`, and LS-E run 1's replies hold no `ch_state`, so the earlier marks were wrong. In LS-D2d `ch_sp_between` ran after `ch_stop`; the plan calls it before |
| P16: The probe charts use Resolution 480 at 125 BPM, one tick per ms | code | `tools/ch_probe/probe_songs.py` lines 13 to 15 and 55 to 56 | checked 2026-11-12 |
| P17: `ch_install_chart` copies only song.ini and the chart, adds silent audio as long as Hydra's song end, and never overwrites a folder | code | `tools/ch_mcp/install_chart.py` lines 1 to 18 | checked 2026-11-12 |
| P18: `ch_run_chart` does the attach, the kit setup, the Star Power watch and the play in one call, and `ch_call` reaches a tool whose schema changed at a reload | code | `docs/agents/clone-hero-mcp.md`, the `ch_run_chart` paragraph and the `ch_call` paragraph | checked 2026-11-12 |
| P19: The three FL-25 runs' watch logs are saved beside their compare files | code | `tools/ch_probe/experiments/results/overlay-spike/` holds `run1_cal0_sp_watch.csv`, `run2_cal30_sp_watch.csv` and `run2b_vid30_sp_watch.csv` | checked 2026-11-12, by listing the folder; the rows were not read |
| P20: The three saved watch logs hold `clock_s`, `sp_amount_u32`, `sp_max_u32` and `sp_active_byte` | run | the header of `tools/ch_probe/experiments/results/overlay-spike/run1_cal0_sp_watch.csv` (the other two have the same header) | checked 2026-11-12 |
| P20b: Trippin SP Spike's `notes.chart` writes no fill (no `S 64` line), so the fill ending at tick 34944 is Hydra's, and `hydra_replay score` writes its `fill_start` | chart | `C:\Clone Hero\songs\Hydra Probe\Trippin SP Spike\notes.chart` (only `S 2` lines) and `tools/replay.cpp` lines 136 and 461 | checked 2026-11-12 |
| P21: Runs must go on while the user uses the machine | ruling | `docs/agents/rulings/clone-hero.md`, 2026-10-30: "yes we need to do autonomous runs. that waws the whole point of this" | checked 2026-11-12 |
| P22: The key route is locked behind the user's typed "allow key run", and this plan does not use it | code | `docs/agents/clone-hero-mcp.md`, the paragraph "The key route is locked" | checked 2026-11-12 |
| P23: A frame-timed press fires on the first frame at or past its time and waits up to 3 ms for it | ruling | `docs/agents/rulings/clone-hero.md`, 2026-11-12: "Fire at the schedule time (Recommended)", whose context gives the 3 ms | checked 2026-11-12 |
| P24: Every kit run is timed by the game since the IG6a merge | code | `docs/agents/clone-hero-mcp.md`, "The route says who times the presses, and there is no switch"; commit `8901af45` is an ancestor of this branch | checked 2026-11-12 |
| P25: The early fill row ships without waiting for this check | ruling | `docs/agents/rulings/ui-and-display.md` line 5, 2026-11-12: "Yes, ship it now", whose context says "(1) the early fill row ships while FL-24 is unproven" | checked 2026-11-12 |
| P26: A human run may play only the test chart and the library charts the user named | code | `docs/agents/clone-hero-mcp.md`, the paragraph "Play stays offline" | checked 2026-11-12 |
| P27: The 6 ms margin in LS-F3's not-offered verdict is two frames of 3 ms and was not measured | unchecked | `docs/agents/rulings/clone-hero.md`, 2026-11-12, gives the 3 ms frame; nothing gives the judge delay | LS-F3 |
| P28: Hydra's own fill finder adds no fill to the Early Fill Walk beyond the ten written ones | unchecked | `docs/clone-hero/04-fills.md` FL-11 to FL-18 describe the generator; none was applied to this chart | T3 |
| P29: The game places the Trippin fill at the same start tick as Hydra | unchecked | `tools/replay.cpp` line 461 is Hydra's own `fill_start`; no game read of that fill exists | T4 |
