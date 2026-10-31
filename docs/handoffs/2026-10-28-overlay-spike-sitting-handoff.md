# Handoff: the live overlay spike, mid-sitting (2026-10-28)

Session `f756d547-af2c-46f3-b891-96dea4efb402`. Plan: `docs/superpowers/plans/2026-10-28-overlay-spike.md` (on main).

## Where it stands

The user wants a live overlay that sits on top of Clone Hero while they drum. It would show the next activation, a countdown, squeeze reminders and a running "points vs optimal" figure. Phase 0 is a spike: one sitting at the real game that checks whether the needed live data can be read. The toolkit for it is built, reviewed and merged on main. Main is not pushed.

The sitting is half done. Runs 1, 2, 2b and 2c are finished. Run 3 (the checksum finder) and step 5 (fullscreen and poll cost) are still to do. Then Task E writes up the results. Nothing about the overlay itself has been built.

**Clone Hero's settings are not back to normal.** `settings.ini` has `video = 30` and `audio = 0` from the last run. Ask the user to set video calibration back to 0 before Run 3. Run 1 also left the probe song installed at `C:\Clone Hero\songs\Hydra Probe\Trippin SP Spike`. The user asked the session to copy it there.

## What is on main (not pushed)

Each task had one fresh reviewer and signed off clean. The merges, oldest first:

- `0037c9dc` the reviewed plan.
- `c1fa5611` Task P: `process.open_process(read_only=True)`, `engine_finder.find_live_engine_capped` (120 s cap), `experiments/overlay_logs.py` (the two CSV column lists).
- `c46eab85` Task D: `log_tail.py`, `topmost_probe.py`, `poll_cost.py`, and the write-up template `docs/research/2026-10-28-overlay-spike-results.md`. Then `9b7101ed` fixed a test fixture that used `0.003`, the key-hold value the single-owner scan reserves.
- `e706a7bf` Task A: the Star Power offsets in `constants.py`, the new `Snapshot` fields, `sp_watch.py`, `watch_checks.py`.
- `a356dc3c` Task S: `sp_compare.py`.
- `ff739d0b` Task B: `play_chart.py --log` and `score_compare.py`.
- `d34c3b5f` Task I: `tools/ch_probe/experiments/results/overlay-spike/il2cpp-fields.md`, the class-dump notes.
- `924be297` Task C plus a decode fix: `find_checksum.py` and `il2cpp_strings.py`.

Another session merged its own Clone Hero work (`ch_mcp`, Frida, a bot launcher) on top of these in parallel. Main's tip was `bd3da716` when this handoff was written. That session also touched `engine_finder.py` (`d288741b`). Check `git log` before you edit anything under `tools/ch_probe`.

## The sitting's materials

All of these are in the session scratch folder `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\f756d547-af2c-46f3-b891-96dea4efb402\scratchpad\`. That folder does not outlive the machine's temp cleanup, so Task E should copy what it needs into the results folder.

- `sitting\Hydra Probe\Trippin SP Spike\` is the probe song: Trippin from `testdata/input/common/IB24/T2`, with a silent `song.ogg`. Its `song.ini` and the `[Song]` section of `notes.chart` were both renamed to "Trippin SP Spike" by Hydra Probe, charter Hydra Probe. The first install showed under the chart's original name, so the user could not find it. Its MD5 is now `ddcd74c4b2fea312695257379de1b66d`.
- `sitting\trippin_dump_renamed.json` and `sitting\trippin_path3_score_renamed.json` are the `hydra_replay` dump and the score of path 3 (Pro Drums, 2x Bass, Expert, default rules). Path 3 is `0 0 E0 0 E0` at 378,855 points: the best path that passes over no fill, as the plan's Choice 2 asked. Renaming the chart changed none of the 857 chords.
- `il2cpp-dump\dump.cs` (36 MB) and `script.json` are the Il2CppDumper output for v1.1.0.6142.
- `owner_prescan.py` applies the single-owner rule patterns to a branch's changed files. See "Lessons" below.

The run files are in `tools/ch_probe/experiments/results/overlay-spike/` and are **not committed**. Task E commits them.

## What the runs showed

Every number below comes from the result file named with it.

**Run 1** (calibration 0, auto-player, Trippin path 3). The auto-player hit all 857 notes (`run1_cal0_play_chart.txt`). Its presses landed a median of 0.42 ms before the chart time, with 90% between -1.86 and +0.94 ms (`run1_cal0_score_compare.txt`).

- **Star Power fields: go** (`run1_cal0_sp_compare.txt`).
  - All 11 phrase ends raised `+0x68` by 1,438 to 1,440.
  - `+0x1B0` held at 5,760, and four phrases fill the meter.
  - Each activation set the flag at `+0x42`, doubled `+0x48` from 4 to 8, and set the end time at `+0x120`. The flag cleared at that end time.
  - The two activations that stayed on the path ended within 2.9 and 1.2 ms of Hydra's deactivation.
- **The meter's unit is not Hydra's chart ticks.** The game's full meter is 5,760 units on this chart (resolution 192), and Hydra's is 6,144 ticks. The NS E chart (resolution 480) showed 14,400 in `scorestats.json`. In both charts the game's full meter is 30 times the resolution, where Hydra's is 32 times. The end times still match Hydra, so this looks like a difference of unit, not of timing. Why it is 30 is not yet known.
- **The path split at the third activation.** Path 3's third activation is an E0 fill at tick 34944. The game did not offer it and activated at the next fill, tick 38016. Every comparison after about 102.7 s is off the path. Most of the 13,760-point gap (game 365,095 against Hydra 378,855) comes from here.
- **Ghost and accent notes pay less from a keyboard.** Before the split, the score fell behind Hydra only on ghost and accent chords, 200 to 260 points each and double in Star Power. `run1_cal0_scorestats.json` says ghost notes hit 0 of 34 and accent notes hit 0 of 9. A keyboard press carries no strike strength.
- **One unexplained step.** Chord 420, the last chord of activation 2 and a plain chord, lost 260 points. Task E should look at it.
- **The checksum is plain MD5 for a `.chart` file too.** `scorestats.json` says `DDCD74C4...`, the MD5 of the renamed `notes.chart`. Its score, 365,095, equals the live read; `sp_bar_ticks` equals the live `+0x1B0` (5,760); and `sp_activations` is 4.
- **The log is written live.** "Loading Song: Hydra Probe - Trippin SP Spike" arrived 4.3 s after "go", which is when the user pressed Play (`run1_cal0_log_tail.txt`).

**Run 2** (audio calibration 30 ms). It stopped at 137.8 s because the game was paused there; the log shows a seek to 137.86 s. All 704 presses hit. Their median was -0.45 ms, and activation end times matched Run 1 within 0.2 ms (`run2_cal30_*`). The game wrote no `scorestats.json` because the song did not finish.

**Runs 2b and 2c** (video calibration 30 ms, audio 0). 2b ran 583 notes, all hits, median -0.45 ms, activation ends within about 1 ms of Run 1. 2c ran 111 notes, all hits, median -0.53 ms. During 2c the game's own pause screen read "Accuracy: 1.1 ms, Average: 2.5 ms" (from the user's screenshot).

**What the calibration runs mean.** Neither setting moves the engine clock at `+0x100` against the chart's own times, so hits land the same against chart times. The code trace below shows that audio calibration does move that clock against the music you hear. The auto-player presses from the clock, so it is blind to both settings by design.

The user pointed out that video calibration clearly matters in real play. That fits a picture-only setting: a human hits when a note reaches the strike line, so moving the picture moves the human's hits. The overlay can compare with Hydra in the engine clock and ignore calibration. Anything it draws that must line up with notes on screen needs the video offset, applied the way the game applies it.

The game's own average of +2.5 ms, against our clock reads of about -0.5 ms, suggests about 3 ms of input polling delay. That is an inference.

## What the code says about calibration

A read-only scout traced both settings through `GameAssembly.dll` (v1.1.0.6142, image base 0x180000000). Its scripts are in the scratch folder under `calibration-trace\`. Each claim below is marked by how it was checked.

**Audio calibration shifts the clock the game judges against. Video calibration shifts only where notes are drawn.**

- **Loading.** The settings loader (RVA 0x374D50) reads `[offsets] audio` and `video`, each limited to -2000..2000 ms. `GameManager.Awake` (RVA 0xB41A0) turns them into seconds. It stores `audioCal` = audio/1000 at GameManager+0x6C and `videoCal` = (video − audio)/1000 at +0x68. _Scout's disassembly; the field names and offsets match dump.cs line 25302 onward._
- **The clock.** At song start both values go to a clock class (TypeDefIndex 707): audio at clock+0x48, video minus audio at +0x50. The clock's raw time at +0x38 follows the audio playback position. _Scout's disassembly._
  - The getter at RVA 0x16C480 returns raw time minus audio.
  - The getter at RVA 0x16C490 returns raw time plus (video − audio), which is songTime plus video.
  - _Both getters checked by the main session from the file's bytes:_ `f2 0f 10 41 38 f2 0f 5c 41 48 c3` and `f2 0f 10 41 50 f2 0f 58 41 38 c3`.
- **Per frame.** Every frame, `GameManager.Update` (RVA 0xC05A0) stores the first getter's result as `songTime` (+0x70) and the second's as `songTimeVideoTime` (+0x78). `BasePlayer.Update` passes `songTime` to the engine, which writes it to engine+0x100. So the engine clock the probe reads is songTime. _Scout's disassembly; the field offsets match dump.cs._
- **The readers.** The readers of `songTimeVideoTime` that the scout found are drawing code: `HighwayScroll.Update` and `DrumsNoteRenderer.Update`, among others. It did not trace all of the roughly 30 readers. _Scout's disassembly; "drawing only" is not fully proven._
- **Sign.** With a positive audio value, the judged clock trails the music by that many ms, so a note is judged that much later in the audio. With a positive video value, notes are drawn as if the song were that much later. _The arithmetic is checked. How the in-game Calibration menu picks the values was not traced._
- **Other uses.** The song-end check (GameManager+0x50) compares against songTime, so it moves with audio too. Both values are also sent to the online leaderboard when a session starts. _Scout's disassembly._

**What this means for the overlay.**
- **Comparing with Hydra:** use engine+0x100 against Hydra's chart times directly, with no correction for either setting. Runs 2, 2b and 2c measured exactly this.
- **Drawing anything that must line up with the notes on screen:** use songTime + video/1000, or read GameManager+0x78 directly. The overlay will need a route to GameManager, which `il2cpp-fields.md` gives through the GameManager static instance.
- **Matching what the player hears:** add audio/1000 to the clock.

**Where the trace stopped.** The scout did not decompile the Calibration menu, what the raw clock is anchored to at song start (so how the chart Offset or song.ini delay enters it), or where button presses get their timestamps.

## What is left

1. **Run 3.** Video back to 0 and audio 0. Bot on. Chart "NS E star cutoffs" (28 s). Run `find_checksum.py --hash 234C917654CC7257E2BAFCF7D761304B` and `sp_watch.py --out ...run3_bot_sp_watch.csv` from the main checkout.
   - Ask the user to say "ready" with the song highlighted, start the scripts, then tell them to press Play. Each script gives up after 120 s with no song playing, and two rounds timed out that way during this session.
   - The pause must be about 3 s, not 5, because `sp_watch` stops after 5 s of still clock.
   - The finder's report should show the raw-bytes hit and the `SongEntry` holder route (`GlobalVariables.instance` -> `currentSongEntry` +0x28 -> `checksum` +0x38, from `il2cpp-fields.md`).
2. **Step 5.** Set `fullscreen = 1`, start a song on the bot, and run `poll_cost.py` and `topmost_probe.py` while it plays. The user says whether the probe window shows over the game. Then set `fullscreen = 0` back.
3. **Task E.** One executor writes `docs/research/2026-10-28-overlay-spike-results.md` from the result files and commits them, then one reviewer. Add to the plan's verdicts:
   - the meter-unit finding;
   - the E0 split and whether Hydra's E0 needs an earlier hit on the phrase note;
   - the ghost and accent pay;
   - chord 420;
   - the two calibration runs, and the scout's code trace;
   - the game's 2.5 ms average.
4. Afterwards, the user decides what Phase 1 looks like.

## Lessons from this session

- **The repo scans read main, not the branch.** `hydra_tests.exe "-sf=*single_owner*"` and `"-tc=docs match code*"` read the tree the exe was built from. Run in a worktree, they check main. A Task D fixture got onto main that way and broke the scan until a fix branch merged. Before a Python-only merge, run `py -I <scratchpad>\owner_prescan.py <worktree> <base>`, then the real scans on main right after the merge. This is saved to memory as `repo-scans-read-main-checkout`.
- **Merge commits need the three trailers.** The ref gate refused a merge commit without `Task:`, `Agent:` and `Session:` trailers. Put them in the last `-m` paragraph, and use `Agent: orchestrator` for the main session.
- **Two scripts have no `--help`.** `topmost_probe.py` shows its window for 60 s, and `poll_cost.py` tries to open the game.
- **Rename the chart, not just `song.ini`.** For a copied probe chart, rename the `.chart` `[Song]` names too, or the game may list it under the chart's own name.
