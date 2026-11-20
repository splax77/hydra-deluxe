# Player audit fixes: the rulings D318 to D323 as tasks

Date: 2026-11-08. Planner: Fable, task player-audit-fix-plan, session 0963d03d. Turns the player audit's rulings (D318 to D323 in `docs/audit/2026-10-03-fix-decisions.md`, the exits in `docs/audit/2026-11-08-player-audit/exits.md`) into tasks. Follows `docs/superpowers/plans/2026-11-08-replay-press-schedule.md` (TC1 and TC2 merged; TP1 to TP4 not built; TD held on `claude/replay-td`).

## The plain version

The user asked for "a full audit of the playing code before making any changes. i dont trust any of this anymore." The audit is done and changed nothing. This plan is the change. It has twelve tasks, six live steps and four questions.

The shape is simple. Everything that is a rule about Star Power, timing or scoring moves into Hydra's `hydra_replay`, which already owns the engine's answers; the Python player keeps only sending, reading the game and logging. Three things the player works out for itself today go into `hydra_replay`'s output: the song's end (Hydra's one end, `app::song_length_ms`), the moment the meter reaches the activation level (Hydra's `bank_rise_ticks`), and which gems of a squeezed-out chord land after the end (Hydra's `sqout_rule`). Everything that judges a run then compares the game with Hydra, not with itself. Everything that can be stale, missing or for the wrong song refuses instead of going on. Lines that say the wrong thing are reworded. The old probe scripts go, once the user has seen the list.

One thing the audit itself got wrong, found while checking: the C++ schedule has the same hard-coded squeeze-out gem the Python has. `tools/replay_schedule.cpp` line 861 sets the late press's gems to place 0, and nothing in `schedule_json` or `schedule_for_windows` takes `Rules`, so `hydra_rules.ini`'s `sqout_rule = whole_chord` never reaches the schedule, while `src/core/scoring.cpp` line 58 pays every note outside under that rule. The exits file (L68) says only the Python hard-codes it. Task F1 fixes both from the one owner.

## What was checked, and how

Every claim below was read in the code on branch `claude/player-audit-fix-plan` at `e9657b17` (main's tip), not taken from the handoff. The handoff (`git show claude/handoff-274bd9dd:docs/handoffs/2026-11-08-player-audit-fix-plan-handoff.md`) was read whole; its Claims table held on every row this plan uses, with two exceptions noted in the Premise map (P30 and P31). `py -I tools/check_handoff.py` was not run on it, because the handoff lives on a branch, not a path; its own checker output is in the file.

`hydra_replay` writes none of the three fields today. `git grep -i -E 'song_length|ready|sqout|SqOut|sp_ready|kSpActivationBars' -- tools/replay*.cpp` finds `song_length` nowhere, `ready` nowhere, and `sqout` only as the squeeze's tick and offset (`sqout_tick`, `sqout_offset_ms`), never as the rule or the gem list.

Hydra's owners for the three fields exist. The song end is `app::song_length_ms(meta, song)` in `src/app/song_length.cpp` line 28, declared at `src/app/song_length.h` line 61, fed by `chart_timing_meta(...)` and `read_chart_timing_meta(notespath)` (`src/app/analysis.h` lines 56 to 61). The activation level is `kSpActivationBars = 2` (`src/core/timing.h` line 51), and each activation stores where every bar it spends arrived in `Activation::bank_rise_ticks` (`src/core/model.h` lines 617 to 625: "Where each bar this activation spends arrived ... in order"). So the tick at which the meter reaches the activation level is the `kSpActivationBars`th arrival. The squeeze-out rule is `Rules::sqout_rule` (`src/core/rules.h` line 33) and the one place that applies it is `category_scores` in `src/core/scoring.cpp`, line 58: `loses_sp = i == 0 || sqout_rule == WholeChord` over `chord.note_list(true)`.

The grep rule the brief sets was run before every design choice below: `src/core`, `src/search` and `tools/replay*.cpp`, then `docs/clone-hero`, `docs/adr` and `docs/agents/rulings` when the first three were silent. Each design point names what the grep found.

## Rulings that bind this plan

- "do a full audit of the playing code before making any changes. i dont trust any of this anymore." (D318 intro). No task here changes what a Hydra screen shows; rule 11's list is in design point 12.
- "why is the python still doing its own thing? the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do" (L44, `2026-11-08-replay-press-schedule.md` line 29). Every Star Power value the player or the judge uses comes through `hydra_replay`.
- "why are you still trying to invent your own rules? hydra already knows how starpower works" (D323 intro). No new number appears in this plan without a quoted ruling.
- "Yes, that rule" (D318 item 3): `Chord::activation_note()` is the one rule for the gem a skipped fill leaves out and the gem an activation chord presses first.
- "there should one rule that every function uses. we already know how clone hero decides the end of the song" (D255, quoted in D321 item 2). `app::song_length_ms` is the one end.
- "remove the old probe scripts, we dont need them anymore" (D322 item 3). The list is question 1.
- "i care about the runs working, not the window coming into focus" (D320 item 4). No focus work.
- "Yes, settle it" (D322 item 2): the end is planned with the frame; no early-edge rule.

## The design, point by point

### 1. The three new `hydra_replay` fields, one owner each (task F1)

**The song end.** `cmd_schedule` and `cmd_dump` in `tools/replay.cpp` gain one top-level field, `song_length_ms`, from `app::song_length_ms(app::chart_timing_meta(std::nullopt, a.chart), song)`. That is the same call `analysis_song_length` makes in `src/app/analysis.cpp` lines 855 to 859 with an empty scan, so the value is the one the Library shows for that chart. A chart for which the function returns empty writes `null`, and every Python reader refuses on `null` (design point 9).

**The ready point.** Each activation in the schedule and the dump gains `sp_ready_tick` and `sp_ready_ms`: the tick of `bank_rise_ticks.at(kSpActivationBars - 1)`, and its ms through the song's tempo map. The accessor is new on `Activation` in `src/core/model.cpp`, beside the other `sp_end_steps` readers, so `hydra_replay` reads it and does not rebuild it. An activation with fewer arrivals than `kSpActivationBars` is a hand-built one; it writes `null` and a refusal line, never a guess. Grep: `sp_ready_ms` exists only inside the search (`src/search/engine.cpp` lines 276, 316, 848 to 855) and is not stored; `bank_rise_ticks` is what the search stamps. The accessor reads the stamped list. Whether the second arrival equals the search's `sp_ready_ms` on real paths is F1's corpus test.

**The squeeze-out's late gems.** The late press of a squeeze-out (`tools/replay_schedule.cpp` lines 859 to 878) today holds `{0}`. It will hold the places `category_scores` pays outside under the rules the command loaded (`s.rules.sqout_rule`, `tools/replay.cpp` line 94). A new function in `src/core/scoring.cpp`, `sqout_late_notes(chord, rule)`, returns those places, and `category_scores` line 58 calls it, so the rule has one body. `schedule_for_windows` and `schedule_json` take `const core::Rules&`. The grep rule: `src/core/scoring.cpp` line 58 answers which notes pay outside under each rule, and the player's job is to make the game pay what Hydra pays, so the late press holds exactly those notes. Under `first_note` (the default) nothing changes: the late press is still place 0. Under `whole_chord` every gem of the chord is in the late press and there is no inside group. That is Hydra's answer, read from the owner, so it is not a question. The closure checker's note that no ruling names the `whole_chord` press is right, and none is needed: the user's ruling is that Hydra owns the Star Power planning (L44), and Hydra's scoring code states which notes pay outside. A press that put any gem of a `whole_chord` chord inside would make the game pay a different score than Hydra's, which is the thing the player exists to prevent. A `whole_chord` chord with no inside group has the same shape a one-gem squeeze-out chord has today (inside list empty, `join_gems` empty), so the fallback needs no new rule. The places are indexes into `note_list(true)`, which is also the order of `ReplayChord::notes` (P41). Who sees it: only a player whose `hydra_rules.ini` says `sqout_rule = whole_chord`. The default `first_note` writes the same schedule as before, and F1 pins that.

**The build.** Each output gains `hydra_replay_build`, the exe's compile date and time (`__DATE__ __TIME__`) and the git hash CMake stamps if the build already has one (P33). The Python run folder copies it (design point 5).

### 2. One frame, TM-50, flagged skips, one decline rule (task F2)

D319 item 3: `kJudgeLateMs` goes. `press_room_ms`, `last_send_ms`, `far_press_refusal` and both fallbacks read `kJudgeEarlyMs` (one frame). The schedule's `constants` block already writes the constants; the Python drop check reads `last_send_ms` from the file after TP2, so there is no Python copy to keep in step.

D322 item 1: `far_press_refusal` (`tools/replay_schedule.cpp` line 392) refuses only when an earlier same-pad note whose window holds the press will still be unhit when the press lands: the player sends it no press, or sends its press later. The later-note branch goes.

D320 item 3: every branch that drops a chord from the press list writes a refusal sentence and sets `refused: true` on the chord's row. A squeeze-out chord that is also a skipped fill sends the kept gems `activation_note` leaves (the one rule), or is refused if the late gem is the one left out. Each branch gets a test.

D318 item 3: `DeclineMode::KickOnly` and `DeclineMode::Nothing` go, and with them `--decline` on `hydra_replay schedule` and `decline_mode` on `ch_run_chart` and `ch_play_chart` (question 2). The one rule is `kept_gems`: every gem but `Chord::activation_note()`. Grep: `tools/replay_schedule.cpp` line 200 already reads `chord.activation_note()` for `all_but_top_pad`, and line 798 for the first press group, so the C++ already has the rule; F2 removes the other two modes and the flag.

### 3. Stale builds refuse (task F3)

D318 item 1. The agent build (`tools/ch_mcp/agent/package.json`, six `build:*` scripts) writes beside each `.js` a stamp file holding the SHA-256 of every `.ts` the compile read. The loaders (`frida_tools.py` line 377, `bot_run.py` line 631, `trace_hits.py` line 299) recompute the hash and refuse with a line naming the stale file and its build command. The run folder gets `builds.json`: each agent's stamp and the `hydra_replay_build` field of the schedule the run used. F3 writes the function that builds that record (`build_stamp.builds_record`) and tests it on fake inputs; F7 owns `game_tools.py`, which makes the run folder, so F7 calls it there and tests the file. The kit tests run the built `kit.js` in Node where today they read `kit.ts` (`test_kit_tools.py` lines 1247 to 1271).

### 4. The player refuses what is missing (task F4, after TP2)

D318 item 2, D321 item 3 (L13, L15), D323 item 2 (Q20), D319 item 1's "a stop before the song's end is a failure". The reply check in `play_chords` stops the run when a reply is not a dict, lacks `down`/`up`, or lacks `groups` for a press that sent groups. `load_schedule` refuses a file missing `song_length_ms`, `chords`, a chord's `tick` or `notes`, or a chord with no notes. The kit refuses a repeated element (`kit_tools.py` lines 312 to 316). `main` exits non-zero and prints the stop reason for every stop but the song's end; an empty chord list refuses with the same words as `ch_play_chart`. The start line says how many chords the run skipped and the song time it starts from; the words are the executor's, and F4 pins the sentence it writes (`live.py` `START_LEAD_MS` and `first_note_index` decide the skip). The test fakes send the real reply shape (D318 item 2). The `kit_tools.py` velocity comment (L57) says what SC-13 says.

### 5. The run plays the right song, path and settings (task F7, after TP3)

D318 item 4. `ch_run_chart` refuses before the first press unless the game's loaded song folder matches the schedule's `chart`. `ch_run_chart` and `ch_play_chart` take `path_index` and refuse a schedule whose `path_source.index` differs. `ch_open_song` turns "done: a song is loaded, but its folder is X" (`menu_tools.py` line 287) into a refusal and checks instrument and difficulty from the menu. `ch_open_items` calls public checks (L76, `server.py` lines 104 to 112). The run folder gets `builds.json` (design 3) written here.

### 6. The judge compares the game with Hydra (task F5, after TP4)

D319 items 1 and 2, D321 items 1 and 3, D323 item 3. In `score_compare.py`: a chord passes when the game's score step equals Hydra's points for it; `MIN_HIT_PERCENT` and the any-rise rule go; `ghost_accent` forgiveness applies only when the run's route is keys; the report's clock line reads "Clock at the read before the send minus chart ms". In `sp_compare.py`: the summary line is first; the end check fails when the last chord the game pays inside Star Power is not Hydra's deactivation chord; the flag check compares with Hydra's end; "0 of 0" fails; `sp_max` is checked against 30 times the chart's resolution (SP-11), with the resolution from the `--resolution` argument the module already takes and a refusal when none is given (P42); a run that stopped before the song's end fails the verdict, read from the stop reason F4 writes into the run's result (D319 item 1); `find_activations` reports an activation already on at row 0; `_ms` calls `constants.s_to_ms`; `at_ready_level`, `READY_FRACTION`, `SP_ACTIVATION_BARS`, `SP_CAP_BARS` and `MEASURES_PER_SP_BAR` go (F6 reads the game and Hydra instead); the "Hydra full meter" line that `hydra_full_meter_ticks` prints beside `sp_max` is built from those same bar counts, so it is question 4's. In `compare_scorestats.py`: `notes_hit` equals Hydra's gem count, `sp_phrases_earned` equals Hydra's phrases, the score equals Hydra's total, `total_notes` is compared with the gem count only, and `checksum`, `playback_speed`, `no_fail` and `modifiers` are compared with the run's. In `trace_hits.py`: the args report prints "not hooked: N" and each failure; a line "notes with no call within <EXPECT_NORMAL_BACK_MS> ms: N of M" is added, with the limit read from `constants.EXPECT_NORMAL_BACK_MS` (0.085 s, the limit the existing "calls within" line uses; P43), not a number typed again; an empty manifest run by hand prints one line.

### 7. The Star Power watch's ready test (task F6, cut from F5's branch)

D323 item 1. `sp_tools.py` takes "when" from the schedule's `sp_ready_ms` and "whether" from the game's own half at engine `+0x1B8` (SP-16, `docs/clone-hero/05-star-power.md` line 107), read as one more column of the watch. The watch's status line shows the three values the ruling names, whether (yes or no), the game's half and the meter, and Hydra's ready point in ms; the exact words are the executor's, because the ruling fixes the sources and not the sentence. `L74`: with no song length the run-cap line reads "run cap: 240 s (default; no song length)". `sp_watch.py` line 9 says the writer writes "nan", not "blank (nan)".

### 8. The recorder (tasks F8 and F9)

D321 item 1 (L35): a non-finite or negative clock read is written as no clock (`machine_recorder.py` lines 356 to 373). L42 and L43: the `docs/agents/clone-hero-mcp.md` sentence says the launch agent rests on its byte checks. L41: `ch_status` warns when the loader check reads "no" (`bot_run.py` lines 590 to 597; F3's file, so F3 carries it).

L67's recorder fake (reopened by the third-round check): `test_machine_recorder.py` lines 34 to 73 build the fixture table from the parser's own `M.PROC_*` offsets, so a wrong offset passes. F9 replaces it with a byte literal captured once from a real `NtQuerySystemInformation` buffer on this machine, trimmed to two processes, with the expected fields pinned as literals from that capture.

### 9. The song end reaches every reader (task F10, after F3, F6 and TP3)

D321 item 2. One Python function, `hydra_end.song_length_ms(json_path)`, reads `song_length_ms` from a schedule or dump and refuses on a missing or `null` field. The readers switch: `bot_run.song_length_ms` (run cap; the bot route runs `hydra_replay dump` on the folder's chart first, question 3), `game_tools.run_cap_s` (reads the run's schedule), `sp_tools`' two length readers, `install_chart.install` (the silent audio). Deleted: `game_files.song_ini_length_ms`, `probe_songs.song_length_ms` (last note plus 3,000 ms), the `song.ini` first-line read in `bot_run.song_length_ms`. L82: `install_chart` refuses a `--name` that is not a plain folder name and copies no other audio file.

### 10. The old probe scripts (task F11, after the user's yes on question 1)

D322 item 3. `SongClock` moves from `walk_edges.py` to `live.py` first, because the live player imports it (`play_chart.py` line 75) and a test pins the identity (`test_play_chart.py` lines 126 to 130). Then `analysis.py`, `active_probe.py`, `passive_probe.py`, `walk_edges.py` and their tests go, with every citation question 1 lists.

### 11. The pages (task F12, after TD merges)

L60: FL-31 on `docs/clone-hero/04-fills.md` cites LS-M2's nine declined chords; TM-50 names Freak chord 803; SP-47's Hydra line names the schedule's frame constant. TD (`claude/replay-td`) already rewrites TM-50's and SP-47's Hydra lines to name the schedule code, so F12 edits only what TD leaves.

### 12. What a Hydra user sees (rule 11)

No screen changes. F1 adds one accessor to `Activation` and one function to `scoring.cpp` that `category_scores` calls with the same inputs and the same result; TC1's scan test ("nothing under src/ reads the schedule") still holds, and F1 adds a test that `category_scores` gives the same `CategoryScores` before and after on every corpus chord under both rules. The Paths tab, Preview, report and badges read nothing new.

## Questions

Answered 2026-11-08, every one (a): "All four (Recommended)", "Remove the flag (Recommended)", "Tool runs dump (Recommended)", "Drop that half (Recommended)". Recorded as D324 in `docs/audit/2026-10-03-fix-decisions.md`.

1. **Which files Q18 removes.** The user said "remove the old probe scripts, we dont need them anymore". The four candidates and what names each, from `git grep` across the repo (docs included) on 2026-11-08:

   - `tools/ch_probe/experiments/analysis.py` (holds `predicted_window_normal`, `find_window_edge`). Imported by `active_probe.py` line 50, `passive_probe.py` line 50, `walk_edges.py` line 56, `tests/test_analysis.py` line 22. Cited by `tools/ch_probe/README.md`, `tools/ch_probe/constants.py` (comments), `tests/single_owner/rules/probe.json` (a single-owner scan rule that names the probe files; the rule must drop them), `docs/clone-hero/03-timing.md`, `07-internals.md`, `docs/clone-hero/questions.md`, and past audits under `docs/audit/` (history, not edited).
   - `tools/ch_probe/experiments/active_probe.py`. Imported by `tests/test_runners.py` line 27, `tests/test_hit_window_scripts.py` line 125. Cited by the same docs.
   - `tools/ch_probe/experiments/passive_probe.py`. Imported by `tests/test_runners.py` line 27, `tests/test_s2_window_constants.py` line 21. Cited by the same docs.
   - `tools/ch_probe/experiments/walk_edges.py`. **Not purely old.** `play_chart.py` line 75 imports `SongClock` from it, `test_play_chart.py` lines 126 to 130 assert `play_chart.SongClock is walk_edges.SongClock`, and `active_probe.py` line 51 imports it. F11 moves `SongClock` to `live.py` before the file goes. Also imported by `tests/test_hit_window_scripts.py` line 22. `live.py` and `edge_probe.py` name it in comments.
   - Tests that go whole: `tests/test_analysis.py`, `tests/test_runners.py`, `tests/test_hit_window_scripts.py`. `tests/test_s2_window_constants.py` imports `passive_probe` for one constant; F11 reads it and either repoints it at `constants.py` or deletes it, and says which in its report.
   - Also found by a repo-wide `git grep` for each module name and each function it owns (`predicted_window_normal`, `find_window_edge`, `SongClock`, `run_active_probe`, `run_passive_probe`, `clamp_edges`), docs and plans included, on 2026-11-08: `tools/ch_probe/experiments/live.py` and `edge_probe.py` (comments naming `walk_edges` or `active_probe`); `tools/ch_probe/constants.py` lines 287 to 322 (comments naming `analysis.py`'s clamp checks and `walk_edges.SongClock`, plus the clamp-tolerance constants only `analysis.py` reads); `tools/ch_probe/experiments/active_probe.py` line 99, which calls `probe_songs.song_length_ms`, the function D321 item 2 deletes (so F10 waits for F11); and three older plans that name the files, `docs/superpowers/plans/2026-11-07-player-limits.md`, `2026-11-08-player-freak-fixes.md` and `2026-11-08-player-audit.md`, which F11 leaves alone unless a line cites a file F11 removes, and then fixes it in the removed-file form. Past audits and handoffs under `docs/audit/`, `docs/handoffs/` and `docs/sessions/` are history and are not edited. F11 also lists every constant in `constants.py` that nothing reads once the files are gone; it deletes none of them, because the `RVA_FORMULA_*` tables are game facts that pages 03 and 07 cite.
   - Stay: `edge_probe.py` (imported by `tools/ch_mcp/game_tools.py` line 36, not `server.py` as exits.md L84 says) and `overlay_logs.py` (the press-log column owner).

   Sides: (a) remove the four files and the three tests, repoint the single-owner rule, the README, the two page lines and `questions.md`, and move `SongClock`; (b) remove only `analysis.py` and the two probes that cannot run, keep `walk_edges.py`. Recommended: (a), with the move; the user's words name "the old probe scripts" whole.

2. **The decline flag goes with the two modes.** D318 item 3 removes `kick_only` and `nothing`. A flag with one value is a copy of the rule, so F2 removes `--decline` from `hydra_replay schedule` and `decline_mode` from `ch_run_chart` and `ch_play_chart`; a call that still passes it fails with an unknown-argument error. Grep: `tools/replay_schedule.cpp` lines 157 to 200 hold the three modes and `decline_flag_refusal`; nothing under `src/` reads them. Sides: (a) remove the flag; (b) keep a one-value flag. Recommended: (a). The replay plan's LS-S2 call then loses its `decline_mode` word.

3. **How a bot run gets Hydra's song end.** `ch_start` with a song folder and `ch_install_chart` have a chart folder and no schedule. Hydra's value is `app::song_length_ms`; the question is plumbing. Sides: (a) the tool runs `hydra_replay dump --chart <folder's chart>` into the run or scratch folder and reads `song_length_ms` from it, through `prep_inputs.py`'s runner; (b) the tool takes a `--dump` path and refuses without one. Recommended: (a); one owner, no new argument. Either way `song.ini` is never read for the length again.

4. **The "Hydra full meter" line in `sp_compare`.** Today the Star Power report prints `sp_max` and, beside it, "Hydra full meter N (resolution R x 32)", built from `MEASURES_PER_SP_BAR` and `SP_CAP_BARS` in the Python (`sp_compare.py` lines 176 to 178 and 405 to 414). D323 item 1 says the Python's bar counts go, and D319 item 2 says `sp_max` is checked against the game's value, 30 times the resolution (SP-11: Hydra's own full bar is 32 in 4/4, and SP-11's ruling line says no score depends on the difference). `hydra_replay` does not write a full-meter figure (P1, grep for `resolution` in `tools/replay*.cpp` finds nothing). Sides: (a) drop the "Hydra full meter" half of the line, so the report shows `sp_max` and the game's 30 x resolution it was checked against; (b) F1 also writes Hydra's full-meter ticks into the dump, and the line stays as it is. Recommended: (a), because (b) adds a field to carry a number no check uses. The user sees the report line change in (a).

## Tasks

Each task is one Opus executor, one worktree, under 100 tool calls and 20 minutes, cut from the `main` that holds its blockers. No task builds; the main session builds `hydra_replay` and `hydra_tests` and runs `hydra_tests.exe "-tc=schedule*"`, `"-tc=replay*"` and `"-tc=scoring*"`. Python tasks run only the unittest modules their files touch, plus `hydra_precheck.exe` from the main checkout. A test pins a literal from one run or calls the production function; it never recomputes the expected value.

Order and fit with the replay plan. F1, F3, F8, F9 and F12 start now, in parallel (F12 after TD merges). F2 follows F1 (same file; `earlyStart: F1` once F1's branch is pushed to review). TP1 is cut after F1 and F2 merge, because `load_schedule` must read `song_length_ms`, `sp_ready_ms` and `late_gems`, and TP1's `test_no_deleted_name_remains` list gains `DECLINE_MODES`. TP3, TP2 and TP4 follow as the replay plan says. F4 follows TP2 and F7 follows TP3, because they edit `play_chart.py`, `game_tools.py` and `server.py`. F5 follows TP4 (`sp_compare.py`), F3 (`trace_hits.py`) and F4 (the stop reason it reads); F6 is cut from F5's branch and merges with it. F10 follows F3, F6, F7 (`game_tools.py`), TP3 and F11 (`active_probe.py` still calls the function F10 deletes until F11 removes it). F11 follows question 1's yes, F4 (`play_chart.py`, `live.py`, `test_play_chart.py`) and F12 (`03-timing.md`). F12 follows TD. No two tasks run at the same time on one file. `claude/replay-td` merges after F2, re-read for the one-frame and TM-50 words.

### F1: the three fields and the build stamp in `hydra_replay` (C++; Opus)

Owns: `tools/replay.cpp` (`cmd_schedule`, `cmd_dump`), `tools/replay_schedule.h`/`.cpp` (the `Rules` parameter, the late press, the new per-activation fields), `tools/replay_json.cpp` (the dump's activation block), `src/core/model.h`/`.cpp` (`Activation::sp_ready_tick()`), `src/core/scoring.h`/`.cpp` (`sqout_late_notes`), `tests/test_replay_schedule.cpp`, `tests/test_scoring.cpp` (new cases at the end), `CMakeLists.txt` if a build stamp define is added. Design points 1 and 12. Blockers: none. Uses rows P1 to P9, P33.

Acceptance, each a `TEST_CASE`:
- "schedule: song_length_ms is app::song_length_ms": on every chart in `testdata/input` the field equals `app::song_length_ms(chart_timing_meta(nullopt, path), song)`; on a hand-built song with no candidate it is `null`.
- "schedule: sp_ready_tick is the second bank arrival": on every corpus path each activation's `sp_ready_tick` equals `bank_rise_ticks.at(kSpActivationBars - 1)` and its ms equals the tempo map's; an activation with one arrival writes `null` and a refusal naming it. Fails if the value is taken from `sp_end_steps` or computed from phrase ticks.
- "scoring: sqout_late_notes is what category_scores pays outside": for every corpus chord and both rules, the places returned are exactly those with `loses_sp` true; `category_scores` gives the same `CategoryScores` as before the change (pinned from one run on the Freak and Senri fixtures).
- "schedule: the late press holds sqout_late_notes": on Freak 387 under `first_note` the late press holds place 0 and the inside groups the rest; under `whole_chord` it holds every place and there is no inside group. Fails if `{0}` is hard-coded.
- "schedule: a whole_chord chord with no inside group has an empty inside list and an empty `join_gems` in its fallback, like a one-gem chord".
- "schedule: first_note writes the same schedule as before": the Freak and Senri schedules under the default rule equal the pinned run's late presses and inside groups.
- "schedule: hydra_replay_build is written": the field is non-empty and equals the exe's stamp.
- "schedule: nothing under src/ reads the schedule" still passes.

### F2: one frame, TM-50, flagged skips, one decline rule (C++; Opus; after F1; earlyStart: F1)

Owns: `tools/replay_schedule.h`/`.cpp` (the rules), `tools/replay.cpp` (the `--decline` flag), `tests/test_replay_schedule.cpp`. Design point 2. Blockers: F1. Uses rows P10 to P14.

Acceptance:
- "schedule: one frame everywhere": `press_room_ms`, `last_send_ms`, `far_press_refusal` and both fallbacks give the same latest judged time for a 50 ms side at 1,000 ms (1,047 from `last_send_ms`; room measured against the same frame); `kJudgeLateMs` is absent from the file (a text scan).
- "schedule: far press follows TM-50": two snares 100 ms apart, both pressed on time, first sent 20 ms late: no refusal. The same with the second snare's press sent before the first's: refused naming both. A press inside a later same-pad note's window when that note gets no press: no refusal (the later-note branch is gone). Fails if either old branch remains.
- "schedule: every skip writes a refusal and a flag": for each branch Q12 names (`:638`, `:641`, `:725` on main), a hand-built input hits it and the output has a refusal line and `refused: true` on the row; a refused far press is not written as a press row.
- "schedule: a squeeze-out that is also a skipped fill": the late gem is the activation note: refused naming the chord; otherwise the kept gems are every gem but the activation note, pressed as the inside groups, and the late gem is from F1's field.
- "schedule: one decline rule": `DeclineMode` has one value or is gone; `kept_gems` returns every gem but `activation_note()`; `cmd_schedule` with `--decline` prints an unknown-flag error. `test_replay_schedule.cpp` calls that take a mode are rewritten.
- "schedule: the first press group is the activation note": the existing rule at line 798, pinned on Freak 232.

### F3: stale builds refuse, and the run records its builds (Python and package.json; Opus)

Owns: `tools/ch_mcp/agent/package.json`, a new `tools/ch_mcp/agent/stamp.py` (or `.js`) the build scripts call, `tools/ch_mcp/build_stamp.py` (the one reader), the loader lines in `frida_tools.py` (377), `bot_run.py` (631, and `ch_status`'s "no" warning at 590 to 597), `trace_hits.py` (299, 432), `tools/ch_mcp/tests/test_kit_tools.py` (lines 1247 to 1271 run `kit.js`), `test_bot_run.py`, `test_frida_tools.py`. Design points 3 and 8 (L41). Blockers: none. Uses rows P15 to P18.

Acceptance (unittest):
- `test_a_stale_stamp_refuses_naming_the_file_and_the_build_command`: a `.js` whose stamp hash differs from its `.ts` hash refuses in each of the three loaders with the file and `npm run build:<name>` in the line; a matching stamp loads; a missing stamp refuses.
- `test_the_kit_tests_run_the_built_file`: the Node test opens `kit.js`, not `kit.ts`, and skips with a named reason when the built file is absent or stale.
- `test_ch_status_warns_when_the_loader_read_no`: the status text holds the warning line; "yes" holds none.
- `test_builds_record_holds_each_stamp_and_the_replay_build`: `build_stamp.builds_record` given the loaded agents and a schedule dict returns one entry per agent and the schedule's `hydra_replay_build`; a schedule with no such field makes the record say so, never an empty string. (F7 writes it into the run folder and tests the file.)

### F4: the player refuses what is missing and reports every stop (Python; Opus; after TP2)

Owns: `tools/ch_probe/experiments/play_chart.py` (`play_chords`' reply check, `load_schedule`'s key checks, `main`'s exit codes and the empty-list refusal, the hit rule's removal), `tools/ch_probe/experiments/live.py` (the start line), `tools/ch_mcp/kit_tools.py` (repeated element; the velocity comment), `tools/ch_probe/tests/test_play_chart.py`, `tools/ch_mcp/tests/test_kit_tools.py`'s fakes (after F3 merges), `tools/ch_probe/tests/fakes.py`. Design point 4. Blockers: TP2, F3. Uses rows P19 to P22.

Acceptance:
- `test_a_reply_without_groups_stops_the_run_at_that_chord`: a fake reply `{}` for a press that sent groups raises with the chord index and the word `groups`; the run's result names the stop. Fails if the run goes on.
- `test_a_reply_that_is_not_a_dict_or_lacks_down_up_stops_the_run` (L70).
- `test_load_schedule_refuses_a_missing_key`: for each of `song_length_ms`, `chords`, `tick`, `notes`, and for `song_length_ms: null`, the load refuses naming the key.
- `test_a_chord_with_no_notes_is_refused` and `test_the_kit_refuses_a_repeated_element`.
- `test_main_exits_nonzero_on_every_stop_but_the_song_end`: each stop reason (`RunCapReached`, `StopAsked`, `ClockFrozen`, `ClockJumpedBack`, the reply stop, the empty list) exits 1 and prints its reason; a run that reaches the song end exits 0 and prints "Done.".
- `test_the_start_line_counts_skipped_chords`: a clock that skips one chord prints a line holding the count 1 and the start time, and one that skips none prints 0 (the restart form too); the sentence is pinned as a literal from one run.
- `test_fakes_send_the_real_reply_shape`: every fake press reply in the tests has `down`, `up` and, when groups were sent, `groups`.

### F5: the judge compares the game with Hydra (Python; Opus; after TP4)

Owns: `tools/ch_probe/experiments/score_compare.py`, `tools/ch_probe/experiments/sp_compare.py`, `tools/ch_mcp/compare_scorestats.py`, `tools/ch_mcp/trace_hits.py`, their tests. Design point 6. Blockers: TP4, F1 (the fields), F3 (`trace_hits.py`), F4 (the stop reason the verdict reads). Uses rows P23 to P26, P42, P43.

Acceptance:
- `test_a_chord_passes_only_when_the_step_equals_hydras_points`: a step one point off fails; `MIN_HIT_PERCENT` is absent (text scan); the any-rise rule is absent.
- `test_ghost_accent_forgiveness_applies_only_on_the_keys_route`: the same row passes on keys and fails on kit.
- `test_summary_is_the_first_line` and `test_the_end_check_fails_when_the_last_paid_chord_is_not_hydras_deact_chord` (pinned on LS-M2's activation 2: 3,544 ms early, from `ls-m2-run1`), `test_zero_of_zero_fails`, `test_sp_max_is_checked_against_30_times_resolution` (Freak 14,400 at resolution 480), `test_an_activation_on_at_row_0_is_reported`, `test_ms_calls_s_to_ms`, `test_no_ready_copy_remains` (text scan for `at_ready_level`, `READY_FRACTION`, `SP_ACTIVATION_BARS`, `SP_CAP_BARS`).
- `test_compare_scorestats_fails_on_notes_hit_phrases_score_checksum_speed_or_modifiers`: each one off fails; `total_notes` equal to the chord count but not the gem count fails.
- `test_trace_args_report_prints_not_hooked`, `test_notes_with_no_call_line`, `test_empty_manifest_prints_one_line`.
- `test_clock_line_wording`: "Clock at the read before the send minus chart ms".
- `test_a_run_that_stopped_before_the_song_end_fails`: a result whose stop reason is anything but the song's end fails the overall verdict, and one that reached the end passes it (D319 item 1).
- `test_the_hydra_full_meter_line_follows_question_4`: under (a) the report holds no "Hydra full meter" text; under (b) it holds the figure from the dump.
- `test_sp_max_check_refuses_without_a_resolution`.

### F6: the watch's ready test reads Hydra and the game (Python and agent; Opus; cut from F5's branch)

Owns: `tools/ch_mcp/sp_tools.py`, `tools/ch_probe/experiments/sp_watch.py` (the `+0x1B8` column and the line 9 comment), the agent file that reads the watch's Star Power fields (P27), `tools/ch_mcp/tests/test_sp_tools.py`, `test_sp_watch.py`. Design point 7. Blockers: F5 (same merge), F1. Uses rows P27 to P29.

Acceptance:
- `test_ready_when_comes_from_the_schedule_and_whether_from_the_games_half`: a row with meter 7,201 and game half 7,200 is ready; with half 7,201 and meter 7,200 it is not; the "when" line quotes the schedule's `sp_ready_ms`. No Python half is computed (text scan).
- `test_run_cap_line_says_default_when_no_song_length`.
- `test_the_watch_writes_the_half_column` from a fixture row.

### F7: the right song, path and settings (Python; Opus; after TP3)

Owns: `tools/ch_mcp/game_tools.py` (the song-folder check, `path_index`, `builds.json`), `tools/ch_mcp/menu_tools.py` (`ch_open_song`'s refusal and the instrument and difficulty checks), `tools/ch_mcp/server.py` (the arguments; `ch_open_items`' public checks), `tools/ch_mcp/tests/test_game_tools.py`, `test_menu_tools.py`, `test_server.py`. Design point 5. Blockers: TP3, F3. Uses rows P30, P34.

The run folder's `builds.json` is written here from `build_stamp.builds_record` (F3), and `test_builds_json_in_the_run_folder` reads the file back and checks one entry per loaded agent and the schedule's `hydra_replay_build`.

Acceptance:
- `test_run_chart_refuses_when_the_loaded_folder_is_not_the_schedules_chart` (before any press) and `test_path_index_must_match_path_source`.
- `test_open_song_wrong_folder_is_a_refusal` and `test_open_song_checks_instrument_and_difficulty`.
- `test_open_items_uses_public_checks`: no private attribute of another tool is read (text scan for `._` on the four tools).
- A signature test: `ch_run_chart` and `ch_play_chart` take `path_index` and no `decline_mode`.

### F8: the recorder's clock and the docs sentence (Python and docs; Opus)

Owns: `tools/ch_mcp/machine_recorder.py` (lines 356 to 373), `tools/ch_mcp/tests/test_machine_recorder.py` only for the new clock case (F9 owns the fixture; F8 adds one test function at the end, F9 is cut after F8), `docs/agents/clone-hero-mcp.md` line 74. Design point 8. Blockers: none.

Acceptance: `test_a_nonfinite_or_negative_clock_is_no_clock` (NaN, -1.0 and +inf each write the no-clock marker; 12.5 writes 12.5); the docs sentence names the launch agent's byte checks.

### F9: the recorder fake from real bytes (Python; Opus; after F8)

Owns: `tools/ch_mcp/tests/test_machine_recorder.py` (`fixture_table` and every test that calls it), a new `tools/ch_mcp/tests/recorder_sample.bin`. Design point 8. Blockers: F8. The main session captures the sample once with the recorder's own read, trimmed to two processes, and commits it on F9's branch before F9 starts (a live step with no game: LS-A0).

Acceptance: `test_the_parser_reads_the_captured_table`: the pids, names, thread counts and times pinned as literals from the capture; `fixture_table` is gone (text scan for `M.PROC_`); a test that shifts one offset by 4 fails.

### F10: the song end reaches every reader (Python; Opus; after F3, F6, TP3)

Owns: new `tools/ch_mcp/hydra_end.py`, `tools/ch_mcp/bot_run.py` (`song_length_ms` only), `tools/ch_mcp/game_tools.py` (`run_cap_s` only), `tools/ch_mcp/sp_tools.py` (the two length readers only), `tools/ch_mcp/install_chart.py`, `tools/ch_mcp/game_files.py` (`song_ini_length_ms` deleted), `tools/ch_probe/probe_songs.py` (`song_length_ms` deleted), `tools/ch_probe/prep_inputs.py` (the runner F10 calls), their tests. Design point 9. Blockers: F3, F6, F7, TP3, F11, question 3. Uses rows P6, P31, P44.

Acceptance:
- `test_every_length_reader_calls_hydra_end`: `bot_run`, `game_tools`, `sp_tools`, `install_chart` read the length from `hydra_end.song_length_ms`; `song_ini_length_ms`, `probe_songs.song_length_ms` and the `song_length` key read are absent (text scan of the five files).
- `test_hydra_end_refuses_null_or_missing`.
- `test_install_refuses_a_name_that_is_not_a_plain_folder_name` (`..`, a slash, an empty string) and `test_install_copies_no_other_audio`.
- `test_bot_run_cap_is_run_cap_s_of_hydras_end`: the fake dump's `song_length_ms` 260,504 gives the cap the existing test pins.

### F11: the old probe scripts go (Python and docs; Opus; after question 1's yes and TP2)

Owns: `tools/ch_probe/experiments/live.py` (`SongClock` moves in), `walk_edges.py`, `analysis.py`, `active_probe.py`, `passive_probe.py` (deleted), `tests/test_analysis.py`, `test_runners.py`, `test_hit_window_scripts.py` (deleted), `test_s2_window_constants.py`, `test_play_chart.py` lines 126 to 130, `play_chart.py` line 75, `edge_probe.py` and `constants.py` comments, `tools/ch_probe/README.md`, `tests/single_owner/rules/probe.json`, `docs/clone-hero/03-timing.md`, `07-internals.md`, `docs/clone-hero/questions.md` (the citing lines, each rewritten to say how to read the file now: `<path>` (removed; `git show <commit>:<path>`)). Design point 10. Blockers: question 1, TP2, F4, F12.

Acceptance: `test_songclock_lives_in_live`: `play_chart.SongClock is live.SongClock`; the report lists every `constants.py` name nothing reads once the files are gone, and deletes none; `git grep` for the four module names finds nothing under `tools/` and `tests/`; `hydra_precheck.exe` passes.

### F12: the pages catch up (docs; Sonnet; after TD merges)

Owns: `docs/clone-hero/04-fills.md` (FL-31), the TM-50 and SP-47 lines TD leaves. Design point 11. Blockers: TD. Acceptance: each changed sentence quoted with its source (the `ls-m2-run1` rows, the Freak 803 row, F2's constant).

## Coverage table

| Item | Task | Note |
|---|---|---|
| D318 1 (Q1) | F3 (stamps, loaders, the record function), F7 (writes `builds.json`), F1 (build field) | |
| D318 2 (Q2) | F4 | fakes in F4 |
| D318 3 (Q3, Q4) | F2 | question 2 for the flag |
| D318 4 (Q5) | F7, F5 (judge side) | |
| D319 1 (Q6) | F5 (step rule, `notes_hit`, phrases, score, a stop before the end fails the verdict), F4 (writes the stop reason, exit codes) | |
| D319 2 (Q7) | F5 | question 4 for the full-meter line |
| D319 3 (Q8) | F2; TP1/TP2 delete the Python frame | |
| D320 1 (Q9) | none: TC2 built equal room; pinned by TC2's tests | |
| D320 2 (Q11) | F4 (L57 comment), F12 (L60) | TM-50 already proven |
| D320 3 (Q12) | F2 | |
| D320 4 (Q13) | none, by the ruling | |
| D321 1 (Q14) | F5 (L31, L32, L34), F8 (L35, L42, L43), F6 (L74), F3 (L41) | |
| D321 2 (Q15 L81) | F1, F10 | question 3 |
| D321 3 (Q15 seven) | F4 (L13, L15), F5 (L33, L75 `_ms`, L85), F7 (L76), F10 (L82) | |
| D322 1 (Q16) | F2 | |
| D322 2 (Q17) | none: TP1 + F2 plan the end with the frame | |
| D322 3 (Q18) | F11, F10 (probe_songs length) | question 1 |
| D323 1 (Q19) | F1 (field), F6, F5 (copies go) | |
| D323 2 (Q20) | F4 | |
| D323 3 (Q21) | F5 | |
| L01, L36, L37, L38, L39 | F3 | |
| L02, L10 | LS-A3 | |
| L03 | LS-A4 | |
| L04, L05, L48 | F2, TP2 | |
| L06, L18, L58, L61, L62, L63, L64, L72, L73, L77, L78, L79, L80, L83 | none: exit 3 or 4, no work | |
| L07, L47 | none (Q9); TP1 deletes the Python copies | |
| L08, L09, L19, L59 | F2 (and TP1 reads it) | |
| L11 | none: the 15 ms constants stay, Q11 | |
| L12 | TP2 | |
| L13, L15, L28, L29, L30, L65, L69, L70 | F4 | L29 also F5 |
| L14 | F4 | |
| L16, L17, L20, L76 | F7 | |
| L21, L22, L23, L24, L25, L26, L27, L31, L32, L33, L34, L71, L85 | F5 | |
| L35, L42, L43 | F8 | |
| L40 | F1, F3, LS-A5 | |
| L41 | F3 | |
| L44, L45, L46 | TP1 to TP4; F11 (the probe copy) | |
| L49 to L55, L56, L66 | F2 | |
| L57 | F4 | |
| L60 | F12 | |
| L67 | F9 (recorder), F3 (built file), F4 (fakes) | |
| L68 | F1 (late gems), F2 (modes), TP1 (strings, constants) | |
| L74 | F6 | |
| L75 | F6 (ready), F5 (`_ms`), F1 (field) | |
| L81 | F1, F10 | |
| L82 | F10 | |
| L84 | F11 | |

## The live steps

The main session runs these after the tasks they name have merged and `hydra_replay` is rebuilt. The replay plan's run rules hold (hold other sessions, MD5 before and after, copy stats and watch out before `ch_stop`, `ch_reload` after the merge, the framerate line checked). Each step caps at 2 runs and 10 minutes. No library run. Nobody focuses the window.

### LS-A0: capture the recorder sample (no game; after F8)

Calls: `py -I <scratch script that calls the recorder's process-table read once and writes the raw bytes trimmed to two processes>` (never tried)

The recorder has no one-shot flag (`git grep sample tools/ch_mcp/machine_recorder.py` finds only the samples file and columns), so the main session writes the script in the scratch folder; nothing in the repo changes for it. Pass: a `.bin` under 64 KB with two processes, committed on F9's branch. Fail: the read raises; the error to the user.

### LS-A1: rebuild `kit.js` and attach with the stamp check (after F3)

Calls: `npm run build:kit` in `tools/ch_mcp/agent` (seen: the 2026-11-06 build that made the current `kit.js`), `ch_start(<Freak folder>, human=True, player="ProDrums,Expert")` (seen: LS-F1a run 1), `ch_attach` (never tried), `ch_stop` (seen: LS-F1b run 1)

The attach is the one new piece: the first attach with the stamp check. Pass: the attach reply names each agent's stamp and no refusal. Fail: a refusal naming a stale file (rebuild that agent, run 2) or a stamp that fails on a fresh build (stop, F3's reader to the user).

### LS-A2: the LS-M2 chart again on the rebuilt kit (after F4, F5, F7 and the replay plan's TP tasks)

Calls: `ch_start(<the LS-M2 chart's folder, read from ls-m2-run1\replies.txt>, human=True, player="ProDrums,Expert")` (seen: LS-M2 run 1), `ch_run_chart(<folder>, <score>, kit_player="System", schedule_json=<schedule>, path_index=0)` (never tried), `ch_play_status` (seen: ls-r1-run1), `ch_wait_stats` (seen: LS-F1b run 1), `ch_compare` (seen: LS-F1b run 1), `ch_stop` (seen: LS-F1b run 1)

The new piece is the schedule route on this chart. Uses P32 (unchecked: the chart the folder names). Pass: every split chord's second group lands (`press-2.csv` has no row with `groups` missing), `notes_hit` equals Hydra's gem count 3,767, and `ch_compare` passes under F5's rules. Fail: any refusal before the first press (its line to the user), or a chord whose step differs from Hydra's (the row to the user). This settles L10 (split chords under load) and gives the kick plan's T4 its write-back run.

### LS-A3: Freak against 531,685 (the replay plan's LS-S1 and LS-S2, unchanged but for the arguments)

Calls: `ch_start(<Get Ur Freak On folder>, human=True, player="ProDrums,Expert")` (seen: LS-F1a run 1), `ch_run_chart(<Freak folder>, <freak score>, kit_player="System", schedule_json=<freak schedule>, path_index=0)` (never tried), `ch_play_status` (seen: ls-r1-run1), `ch_sp_watch_status` (seen: ls-r1-run1), `ch_wait_stats` (seen: LS-F1b run 1), `ch_compare(<stats copy>, <freak score>)` (seen: LS-F1b run 1), `ch_stop` (seen: LS-F1b run 1)

These are LS-S1 and LS-S2 of `2026-11-08-replay-press-schedule.md` with `decline_mode` dropped (question 2) and `path_index=0` added.

Pass as LS-S2's seven reads. This settles L02: the four misses at chords 153 and 154, 803 and 1144 either repeat (their rows and the recorder's stall report to the user) or do not.

### LS-A4: whether one missed gem cancels a phrase's award (L03; a code read, no game)

Calls: `rea` decompile of the game's note-miss handler for drums (never tried)

The handler is found from the miss path that TM-50's listing names.

Pass: the listing shows whether a missed note inside a phrase clears the phrase flag; the fact goes to `docs/clone-hero/05-star-power.md` as a new block with its label. Fail: the handler is not found from TM-50's listing in 10 minutes; stop and offer the kick-held-back run as a second step with its own call list.

### LS-A5: `hydra_replay` against the 11:14 exe's files (L40; no game)

Calls: `.\build_cpp.ps1 -Target hydra_replay` (seen: every build), `hydra_replay dump` and `hydra_replay score` on the `ls-l0` Freak chart (seen: the `ls-l0` folder), a text diff against the folder's files (never tried)

Pass: the only differences are the new fields of F1 and the one-frame change of F2 (`last_send_ms` rows). Fail: any other field differs; the diff to the user before any run uses the new exe.

Where the waiting work fits: `kit.js` is rebuilt in LS-A1; LS-M2 reruns as LS-A2; `claude/replay-td` merges after F2 (re-read for TM-50 and one frame); the kick plan's T4 (its write-back) runs after LS-A2.

## Out of scope (needs the user's yes to leave out)

- Reading the game's code for SP-29 and SP-30 (the 15 ms squeeze-out numbers): Q11's answer left them; the constants stay as the user's batch yes set them.
- Failing a launch on a loader "no", comparing `launch_patch.json`'s build with the installed one, catching a frozen recorder clock (D321 item 1 left them out).
- The 43.9 s window event (D320 item 4).
- The hit-window formula inside Hydra's screens (the user's "new feature").

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| `song_length_ms` in the schedule and dump equals `app::song_length_ms`; a chart with none writes `null` | design 1 | F1 | "song_length_ms is app::song_length_ms" |
| `sp_ready_tick` is the `kSpActivationBars`th bank arrival, never rebuilt; too few arrivals write `null` and a refusal | design 1 | F1 | "sp_ready_tick is the second bank arrival" |
| The late press holds exactly the notes `category_scores` pays outside under the loaded rules; `{0}` is never hard-coded | design 1 | F1 | "the late press holds sqout_late_notes"; "sqout_late_notes is what category_scores pays outside" |
| `category_scores` gives the same result before and after F1 on every corpus chord under both rules | design 12 | F1 | the pinned `CategoryScores` test |
| Every output carries `hydra_replay_build` | design 1 | F1 | "hydra_replay_build is written" |
| Nothing under `src/` reads the schedule code | design 12 | F1, F2 | TC1's scan test at merge |
| One frame everywhere; `kJudgeLateMs` gone | design 2 | F2 | "one frame everywhere" |
| The far-press refusal follows TM-50; the later-note branch is gone | design 2 | F2 | "far press follows TM-50" |
| Every dropped chord writes a refusal and `refused: true`; a refused far press is no press row | design 2 | F2 | "every skip writes a refusal and a flag" |
| A squeeze-out that is a skipped fill uses the activation-note rule or is refused | design 2 | F2 | "a squeeze-out that is also a skipped fill" |
| One decline rule; `--decline` and `decline_mode` gone | design 2, question 2 | F2, F7 | "one decline rule"; F7's signature test |
| A stale built agent refuses in every loader, naming the file and build command | design 3 | F3 | `test_a_stale_stamp_refuses...` |
| The kit tests run the built file | design 3 | F3 | `test_the_kit_tests_run_the_built_file` |
| `ch_status` warns on a loader "no" | design 8 | F3 | `test_ch_status_warns...` |
| The run folder records every stamp and the replay build | design 3, 5 | F3, F7 | `test_builds_json...` |
| A reply missing what was asked stops the run at that chord | design 4 | F4 | `test_a_reply_without_groups...`, `..._not_a_dict...` |
| A schedule missing a key the play depends on, or with a `null` song end, is refused | design 4 | F4 | `test_load_schedule_refuses_a_missing_key` |
| An empty chord, a repeated kit element and an empty chord list are refused; `main` exits non-zero on every stop but the song end | design 4 | F4 | the four tests named |
| The start line counts skipped chords | design 4 | F4 | `test_the_start_line_counts_skipped_chords` |
| Test fakes send the real reply shape | design 4 | F4 | `test_fakes_send_the_real_reply_shape` |
| A chord passes only when the step equals Hydra's points; no 98 % floor; no any-rise rule | design 6 | F5 | the step test and text scan |
| Ghost/accent forgiveness only on keys | design 6 | F5 | `test_ghost_accent_forgiveness...` |
| Summary first; end check against Hydra's deact chord; "0 of 0" fails; `sp_max` against 30 × resolution; row-0 activation reported; `_ms` calls `s_to_ms`; no ready copy | design 6 | F5 | the eight named tests |
| `notes_hit`, phrases, score, checksum, speed, modifiers compared; `total_notes` against gems only | design 6 | F5 | `test_compare_scorestats_fails_on...` |
| The trace's args report prints not-hooked lines; the no-call line; the empty manifest line; the clock wording | design 6 | F5 | the four named tests |
| Ready "when" from the schedule, "whether" from `+0x1B8`; no Python half | design 7 | F6 | `test_ready_when_comes_from...` |
| The run-cap line says "default" with no length; `sp_watch.py` line 9 says "nan" | design 7 | F6 | `test_run_cap_line_says_default...`; the reviewer reads line 9 |
| Wrong folder, wrong path index, wrong song in `ch_open_song` refuse; instrument and difficulty checked; `ch_open_items` public | design 5 | F7 | the four named tests |
| A non-finite or negative clock read is no clock | design 8 | F8 | `test_a_nonfinite_or_negative_clock_is_no_clock` |
| The docs sentence names the launch agent's byte checks | design 8 | F8 | the reviewer reads the sentence against the code |
| The recorder fixture is a captured literal, not built from the parser's offsets; a shifted offset fails | design 8 | F9 | `test_the_parser_reads_the_captured_table` |
| Every length reader reads Hydra's end; `song.ini` and the probe formula are never read for it | design 9 | F10 | `test_every_length_reader_calls_hydra_end` |
| `install_chart` refuses a non-plain name and copies no other audio | design 9 | F10 | the two named tests |
| `SongClock` moves before `walk_edges.py` goes; no module name remains under `tools/` | design 10 | F11 | `test_songclock_lives_in_live`; the grep |
| Each citing doc line says how to read the removed file | design 10 | F11 | `git grep` for the removed file's name finds it only in the removed form |
| FL-31, TM-50 and SP-47 name their current sources | design 11 | F12 | the quoted sentences |
| A `whole_chord` squeeze-out's late press holds every gem; the default rule's schedule is unchanged | design 1 | F1 | "the late press holds sqout_late_notes"; "first_note writes the same schedule as before" |
| A run that stopped before the song's end fails the verdict | design 6, 4 | F5, F4 | `test_a_run_that_stopped_before_the_song_end_fails` |
| The "Hydra full meter" line follows the user's answer to question 4 | question 4 | F5 | `test_the_hydra_full_meter_line_follows_question_4` |
| Nothing in the probe-removal list is deleted before the user's yes; unread constants are listed, not deleted | question 1 | F11 | the grep and the report |
| No Hydra screen changes | design 12 | F1 | the `CategoryScores` pin; TC1's scan test |
| No new number without a quoted ruling | rulings | all | the reviewer checks every constant in the diff against this plan's quotes |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `hydra_replay` writes no song end, no ready point and no squeeze-out rule or gem list today | code | `tools/replay.cpp`, `tools/replay_json.cpp`, `tools/replay_schedule.cpp`, grep for `song_length`, `ready`, `sqout`, `kSpActivationBars` (only `sqout_tick`/`sqout_offset_ms` found) | checked 2026-11-08 |
| P2: `app::song_length_ms(meta, song)` is Hydra's one end (D255) | code | `src/app/song_length.h` line 61, `song_length.cpp` line 28, `analysis.cpp` lines 855 to 859 | checked 2026-11-08 |
| P3: `chart_timing_meta(scanned, notespath)` and `read_chart_timing_meta(notespath)` give the meta from a chart path alone | code | `src/app/analysis.h` lines 56 to 61 | checked 2026-11-08 |
| P4: `kSpActivationBars = 2` | code | `src/core/timing.h` line 51 | checked 2026-11-08 |
| P5: `Activation::bank_rise_ticks` holds where each bar arrived, in order, stamped by the search; `sp_meter()` is its size | code | `src/core/model.h` lines 617 to 625 | checked 2026-11-08 |
| P6: the search's `sp_ready_ms` is not stored on `Activation` | code | `src/search/engine.cpp` lines 276, 316, 848 to 855; grep of `src/core/model.h`, `replay.h` for `sp_ready` finds nothing | checked 2026-11-08 |
| P7: `Rules::sqout_rule` with `FirstNote`/`WholeChord`; `category_scores` applies it at `loses_sp = i == 0 \|\| rule == WholeChord` over `note_list(true)` | code | `src/core/rules.h` lines 19 to 33; `src/core/scoring.cpp` lines 28 to 58 | checked 2026-11-08 |
| P8: the schedule's late press is `gems = {0}` and `schedule_json`/`schedule_for_windows` take no `Rules` | code | `tools/replay_schedule.cpp` lines 844 to 878; `tools/replay.cpp` line 648; `tests/test_replay_schedule.cpp` line 245 | checked 2026-11-08 |
| P9: `cmd_schedule` has the loaded rules at hand (`s.rules`) | code | `tools/replay.cpp` lines 93 to 94, 641 | checked 2026-11-08 |
| P10: `kFrameMs` 3.0, `kJudgeEarlyMs` 1 frame, `kJudgeLateMs` 2 frames | code | `tools/replay_schedule.cpp` lines 35 to 40 | checked 2026-11-08 |
| P11: `far_press_refusal` is at line 392 with the two branches Q16 names | code | `tools/replay_schedule.cpp` line 392; Q16 text in `questions.md` lines 184 to 196 | checked 2026-11-08 (the function's head; the two branch bodies were read by the audit at lines 404 to 420 and are F2's first read) |
| P12: the C++ already reads `Chord::activation_note()` for the kept gems and the first press group | code | `tools/replay_schedule.cpp` lines 183 to 200, 794 to 798 | checked 2026-11-08 |
| P13: `DeclineMode` has `KickOnly`, `Nothing`, `AllButTopPad` and the `--decline` flag is required | code | `tools/replay_schedule.cpp` lines 157 to 200, 983 to 987; `tools/replay.cpp` lines 624 to 626 | checked 2026-11-08 |
| P14: TM-50 is proven in the game's code | data | `docs/clone-hero/03-timing.md`, block TM-50 (commit `fb1be3cd`) | checked 2026-11-08 via D320 item 2; the block itself not reopened: F2's executor opens it |
| P15: the three loaders test only `os.path.isfile` | code | `tools/ch_mcp/frida_tools.py` line 377; `bot_run.py` line 631; `trace_hits.py` line 299 | checked 2026-11-08 |
| P16: the agent build is six `frida-compile` scripts | code | `tools/ch_mcp/agent/package.json` lines 7 to 13 | checked 2026-11-08 |
| P17: the kit tests open `kit.ts` for Node | code | `tools/ch_mcp/tests/test_kit_tools.py` lines 1247, 1259 | checked 2026-11-08 |
| P18: `kit.js` is untracked (built locally); the main checkout's copy predates `kit.ts`'s last commit (2026-11-07 23:11) | code | `git log -1 -- tools/ch_mcp/agent/kit.ts`; the worktree has no `kit.js` | checked 2026-11-08 (the main checkout's `kit.js` time is from Q1's text, 2026-11-06 21:31: unchecked, LS-A1 reads it) |
| P19: `play_chart.py` still holds `FRAME_MS`, `DECLINE_MODES`, `_TOP_PAD_ORDER`, `press_plan`, `retime_from_end`, `_refuse_far_presses`, `RUN_CAP_S`; TP1 is not built | code | `tools/ch_probe/experiments/play_chart.py` lines 101 to 151, 431, 449, 632 | checked 2026-11-08 |
| P20: `main` prints "Done." | code | `play_chart.py` line 1395 | checked 2026-11-08 |
| P21: the Python late gem is note 0 | code | `play_chart.py` line 595 | checked 2026-11-08 |
| P22: `kit.ts` groups are `{elements, after_ms}` with a 2 ms tick | code | `tools/ch_mcp/agent/kit.ts` lines 61, 500 to 523 | checked 2026-11-08 |
| P23: `MIN_HIT_PERCENT = 98`, `ghost_accent` status | code | `tools/ch_probe/experiments/score_compare.py` lines 64, 189 | checked 2026-11-08 |
| P24: `sp_compare` has `report.check(True, ...)` lines and the ready copies | code | `sp_compare.py` lines 114 to 122, 288 to 292, 388, 418, 448, 458 | checked 2026-11-08 |
| P25: `compare_scorestats.compare` reads `accent_notes_hit`/`ghost_notes_hit` and no checksum | code | `tools/ch_mcp/compare_scorestats.py` lines 81, 111 to 113; `git grep checksum` finds no line in it | checked 2026-11-08 |
| P26: the count report prints `not_hooked` | code | `tools/ch_mcp/trace_hits.py` lines 211 to 212 | checked 2026-11-08 |
| P27: which agent file reads the watch's Star Power columns (`sp_amount_u64`, `sp_max_u64`) | code | none does: a search of `tools/` for `sp_amount`, `sp_max`, `0x1B0` and `0x68` in `.ts`, `.js` and `.py` files finds only Python (`sp_tools.py`, `engine.py`, `constants.py`, `overlay_logs.py`, `sp_compare.py`), so F6 owns no agent file | checked 2026-11-11 |
| P28: the game stores its half at engine `+0x1B8` (SP-16) | data | `docs/clone-hero/05-star-power.md` lines 107, 169 | checked 2026-11-08 |
| P29: `sp_tools` calls `at_ready_level` and prints `READY_FRACTION` | code | `tools/ch_mcp/sp_tools.py` lines 71 to 83; `tests/test_sp_tools.py` lines 293, 302 | checked 2026-11-08 |
| P30: `edge_probe` is imported by `game_tools.py`, not `server.py` (exits.md L84 is wrong on this) | code | `tools/ch_mcp/game_tools.py` line 36; `git grep edge_probe tools/ch_mcp/server.py` finds nothing | checked 2026-11-08 |
| P31: the Python length readers are `bot_run.song_length_ms`, `game_files.song_ini_length_ms`, `game_tools.run_cap_s`, `sp_tools` (two), `install_chart.install` line 72, `probe_songs.song_length_ms` lines 201 to 203 | code | `tools/ch_mcp/bot_run.py`, `tools/ch_mcp/game_files.py`, `tools/ch_mcp/game_tools.py`, `tools/ch_mcp/sp_tools.py`, `tools/ch_mcp/install_chart.py` line 72, `tools/ch_probe/probe_songs.py` lines 201 to 203 | checked 2026-11-08 |
| P32: which chart LS-M2 run 1 played | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1\` exists; `replies.txt` not opened | LS-A2's go request |
| P33: whether the C++ build already stamps a git hash the exe can print | unchecked | `CMakeLists.txt` not read for it | F1 |
| P34: `ch_open_song`'s wrong-song line is an ordinary reply at `menu_tools.py` line 287 | code | `tools/ch_mcp/menu_tools.py` line 287 | checked 2026-11-08 |
| P35: the Freak run folder is `2026-11-07\ls-f1-run1` | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-f1-run1` | checked 2026-11-08 (exists; its rows not reopened: LS-A3 reads them) |
| P36: `walk_edges.SongClock` is imported by the live player and pinned by a test | code | `play_chart.py` line 75; `test_play_chart.py` lines 126 to 130 (the main session's read, confirmed by `git grep` for `walk_edges import SongClock`) | checked 2026-11-08 |
| P37: the Q18 citation counts per file | code | `git grep -c` over the repo for the four module names and `predicted_window_normal`, `find_window_edge`, 2026-11-08 | checked 2026-11-08 |
| P38: TC1 and TC2 are merged; TD is on `claude/replay-td` unmerged; TP1 to TP4 are not built | code | `git log -3 -- tools/replay_schedule.cpp`; `git log main..claude/replay-td`; P19 | checked 2026-11-08 |
| P39: the user's rulings D318 to D323 as quoted above | ruling | `docs/audit/2026-10-03-fix-decisions.md` lines 1826 to 1862 | checked 2026-11-08 |
| P40: the `sp_watch.py` line 9 comment says "blank (nan)" | code | `tools/ch_probe/experiments/sp_watch.py` line 9 | checked 2026-11-08 |
| P41: `ReplayChord::notes` is filled from `note_list(true)`, the order `scoring.cpp` line 58 counts `i == 0` in | code | `src/core/replay.cpp` line 149; `src/core/scoring.cpp` line 28 | checked 2026-11-08 |
| P42: `sp_compare` already takes `--resolution` and prints Hydra's full meter from `MEASURES_PER_SP_BAR` and `SP_CAP_BARS` | code | `tools/ch_probe/experiments/sp_compare.py` lines 34, 176 to 178, 405 to 414, 605 | checked 2026-11-08 |
| P43: `EXPECT_NORMAL_BACK_MS` is 85 ms and the count report's "calls within" line already reads it | code | `tools/ch_probe/constants.py` lines 222 and 227; `tools/ch_mcp/trace_hits.py` lines 241 to 243 | checked 2026-11-08 |
| P44: `active_probe.py` and `bot_run.py` call `probe_songs.song_length_ms` | code | `tools/ch_probe/experiments/active_probe.py` line 99; `tools/ch_mcp/bot_run.py` line 399 | checked 2026-11-08 |
| P45: `machine_recorder.py` has no one-shot sample flag | code | `tools/ch_mcp/machine_recorder.py` (grep for `sample` finds only the samples file and its columns) | checked 2026-11-08 |
