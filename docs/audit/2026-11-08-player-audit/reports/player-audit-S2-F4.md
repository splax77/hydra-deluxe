# player-audit-S2-F4: family 5 (timing chain) and family 9 (focus and input isolation)

Main fd93c684, worktree read. All claims "by reading" unless marked "proven by <file>". Run folder R = `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m0-run1`.
My analysis scripts were read-only over R and were not kept.

## Read, and not read
Read in full or in the relevant parts: foreground.py (all), front.py (1-260 only; the rest is the proxy loop), bot_run.py 60-195 and 316-440, game_tools.py 690-720 and 895-1020, quiet.py 1-120 (plus a grep of the whole of it), live.py (all), walk_edges.py 56-280, clock.ts (all), kit.ts 440-662 (press) plus greps, kit_tools.py 560-760, input_driver.py 240-291, play_chart.py 1258-1345, replay_schedule.cpp 14-90, 240-420, 755-830, R's replies.txt, launch jsonl, press-1.csv, sp_watch-1.csv, scorestats head.
Not read: launch.ts / launch_core.ts bodies (only grepped for window words: none), menu.ts, menuhooks.ts, trace.ts, state.ts, hooks_common.ts, gamemanager.ts bodies, menu_tools.py bodies (grep only), server.py, sp_tools.py, trace_hits.py, frida_tools.py, frida_attach.py, machine_recorder/report, recorder_link beyond a grep, replay.cpp/replay_json.cpp, tests, kit_methods.json and the other data files, the rest of replay_schedule.cpp (420-755, 830-1003), engine.py, process.py. Unread files are not clean.

## FAMILY 9: focus and input

### F9-1. The 43.9 s event in R lines up with a 30 ms stall in the game and with the one late press in the run
Evidence in R (proven by R files, with one inferred offset):
- R/replies.txt line 4: "foreground went to the game 2 times, at 0.0 s and 43.9 s since the watch started". The sibling runs (ls-m1, ls-m2) show only the 0.0 s grab.
- The watch starts after start_run returns (game_tools.py:711-714), so its zero is just after the launch patch. R's launch jsonl says the patch landed at epoch 1794158082.327 s. That puts 43.9 s at about epoch 126.2 to 126.9 (inferred; the exact watch zero is not in any file).
- In R/press-1.csv, row 324 (chart 38737.4 ms) was read at epoch 126.856 and is the ONLY press in 1478 whose clock read was more than 6 ms off its aim: clock minus chart = +21.74 ms (every other press is between -2.0 and 0, median -0.5).
- In R/sp_watch-1.csv (reads about every 4 ms) the rows at epoch 126.822 and 126.858 are 36 ms apart, then normal again. So the game (or the reader) stalled about 30 ms right there. The watch's other gaps over 50 ms in the whole file: one 5.0 s gap at 173.3 s (end of song).
- So the likeliest reading: something activated the game window at about 126.8 s, Windows/Unity took about 30 ms to handle it, and press 324 was read late. The press still hit (hit=1, score 116015 to 116215), so the 21.7 ms lateness stayed inside the window.
- What it does NOT show: who brought the window forward. Nothing in R records it.

Code paths that could bring it forward, found by reading (none is a certain cause):
1. None of the kit-route code touches a window. The kit route (kit.ts press, kit_tools.kit_press, game_tools route kit with play.press set, game_tools.py:1448-1450) calls no FindWindow, SetForegroundWindow, ShowWindow or SendInput. play_chart.live_play with a `press` given never looks up the window (play_chart.py:1333 is inside `if press is None`).
2. The Python side's only window-front calls: `input_driver.focus_window` (SetForegroundWindow, input_driver.py:284-291), reached from play_chart.live_press:1274 (keys route, once per press), game_tools.press_keys:937 (ch_key, once per call, and ch_open_song's keys route), game_tools.ch_press:895-899 via live_press, walk_edges.py:240 (once per note), active_probe.py:197 and pad_flash_test.py:63. All need the keys route. R was a kit run, so none of these ran in R unless a separate ch_key/ch_press call was made during the run (not shown in R's files; open).
3. SendInput is only in input_driver.send_input (:259). Same callers as above.
4. The game is started with `SW_SHOWMINNOACTIVE` for the suspended form (bot_run.py:154-183), the form launch_patch.json selects ("launch": "suspended"). The spawn form (bot_run.py:323-330) has no show-window option and its own docstring says the window is not asked to open minimized. It is not used now but is a live path if launch_patch.json changes.
5. Known game behaviour: docs/clone-hero/07-internals.md IN-78 says the game takes the front 0.4 to 0.5 s after launch even when asked to start minimized (the 0.0 s event in all three runs). Earlier unexplained second grabs: sessions 2026-10-31-d1867340 (7.9 s) and 2026-11-07-70bfa33d (48.9 s). Three late grabs at different times (7.9, 48.9, 43.9 s) argue against a fixed timer in our code. Nothing in the files I read fires on a timer at those points.
6. The machine recorder, quiet check, front proxy and recorder_link start child processes with CREATE_NO_WINDOW (front.py:184, recorder_link.py:107, quiet.py:299), so none opens a window. quiet.py runs powershell.exe the same way (quiet.py:311). By reading; not run.
7. Anything outside the player: another program or the user's own window changes (a window closing hands the front to the next window in Z order, which can be the game's). Only a run with a full window-event log (ETW, not just GetForegroundWindow every 0.1 s) can settle it. OPEN. The watch records only which handle is in front (foreground.py events list, including when the game left), not what the previous window was or who asked. R's events list is not in any file of R (only the count line was kept).

### F9-2. The keys route can send keys to the user's window
- play_chart.live_press:1274, game_tools.press_keys:937, walk_edges.py:240 call `focus_window(hwnd)` and then SendInput, and never check that the game is in front afterwards. Windows can refuse SetForegroundWindow from a background process; the return value is dropped (input_driver.py:290-291 calls and discards it). If it refuses, the keys go to the user's current window.
- find_game_window returns 0 when the title "Clone Hero" is not found. play_chart.py:1334-1340 and walk_edges.py:224-226 only print WARNING and go on; focus_window(0) does nothing (input_driver.py:287-288); the keys go to whatever is in front. (S1 C7 has this for play_chart; walk_edges has the same.)
- game_tools.ch_press/press_keys refuse with "no game window found" when hwnd is 0 (game_tools.py:895-897, 933-935). The command-line players do not.
- The "allow key run" rule is not checked in any Python file I read. It lives in a hook outside the repo (server.py:148-150 says so). The command-line route `py tools/ch_probe/experiments/play_chart.py` runs live_play with no press, so it is the keys route, unguarded in Python (play_chart.py main, per S1 C7). Whether a shell hook stops it was not read.
- FindWindowW(None, title) matches by exact title only; a minimized game window still matches. SetForegroundWindow on a minimized window does not restore it, so keys may go to a minimized window or nowhere. OPEN (a keys-route run would settle it; not to be run).
Table: hwnd found, SetForegroundWindow allowed -> game focused each press; hwnd found, refused -> keys to user's window, no message; hwnd 0 on CLI -> keys to user's window, one WARNING.

### F9-3. Kit route: user's keyboard and mouse
- kit.ts setup turns Rewired's "ignore input when app not in focus" off (kit.ts:363-368) and restores it at teardown (:857-860). ch_kit_setup refuses unless settings.ini has pause_on_focus_lost at the run value (kit_tools.py:710-719). So the kit route needs no focus by design, and R's frames kept running while the game was behind (R: 330 fps, 3.033 ms per frame, from press-1.csv).
- If a teardown fails or the worker dies mid-run, the setting stays changed in the game process only (it is in-process; the game restarts clean). Registry/ini restore is by ch_stop (outside my read).
- The server says "Rewired's keyboard and mouse off for the run" (server.py instructions). That is a Rewired setting inside the game; it does not block the OS from giving the user's keys to the game window if the user clicks it. Not read further.

### F9-4. Smaller notes
- foreground.py only reads; `ForegroundWatch.poll` counts a change only when the game becomes front, so a game that stays in front after a grab until the user clicks away counts once (IN-78).
- Poll is every 0.1 s (foreground.py:22); a grab shorter than 0.1 s that ends between two polls is not counted. The kit press itself lasts less than that, so a flash could be missed. Open.
- The watch's `events` list (all handle changes with times) is kept in memory only; R keeps just the count line. A future run could save `watch.events` next to replies.txt (suggestion, not a change).

## FAMILY 5: the timing chain

### The chain (one press)
1. Chart ms in the dump JSON (`chords[].ms`, a float in ms) -> player Chord.ms (play_chart.py).
2. Aim: C++ replay_schedule.cpp computes `aim_ms` per press (offsets from the chord: lead, squeeze, +8 ms per later group), `room_ms`, `last_send_ms`. The Python player does NOT read that output: grep for `"schedule"`, `press_schedule`, and the `constants` object in all .py files under tools/ found nothing (searched tools/**/*.py, the only hit for "schedule" is replay.cpp:871, the command name). The Python player recomputes its own leads (play_chart.py press_plan:431, activation_leads:347, _timed_presses:558). Two full implementations of the same rules, no link between them. (The schedule plan may intend a later step to join them; I did not read the plan.)
3. Clock read: live.wait_until polls `clock.read()` (raw, est) until est >= target_ms - lead_ms (live.py:131-146). Returns raw (song seconds converted to ms). For play_chart the clock is `SongClock(engine.song_clock, max_fill_s=0.0)` so est == raw (play_chart.py:1344). walk_edges uses SongClock with C.CLOCK_MAX_FILL_S = 0.05 s and CLOCK_FRESH_S = 0.002 s (constants.py:315-318) and waits on est, so the two players read time differently (see T2).
4. Send: kit_tools.kit_press -> agent.request (frida RPC) under one request lock (kit_tools.py:601-630) with KIT_PRESS_CAP_S = 1 s (:98).
5. Agent: kit.ts press: down write on Frida's thread (on_main false unless set; KIT_PRESS_ON_MAIN in kit_tools), `frameCount()` read right after; hold = hold_frames frames (KIT_HOLD_FRAMES = 2, kit_tools.py:92); a later group goes down at `Date.now() >= first.downMs + after_ms`, polled every TICK_MS = 2 ms (kit.ts:61), landing on the first frame the poll sees at or after that; then release.
6. Game: Rewired reads the write one frame later (docs IN note, kit.ts comments), judged "between one and two frames after the clock the player read" (replay_schedule.cpp:37-40, citing TM-27). The score is read 5 ms after the reply (S1 C6).
Proven numbers from R/press-1.csv (1478 presses): read to down write -1.65..+0.8 ms (Date.now has 1 ms ticks, so about 0); read to reply 6.3..14.1 ms, median 8.0; hold 2 frames for most and 3 frames for some; 3.033 ms per frame; clock minus chart -2.0..+21.7 (one outlier, row 324); smallest chart gap 68.5 ms.

### Conversions
- ms <-> s: constants.py:208-218 `s_to_ms`, `ms_to_s`, MS_PER_S (one home in ch_probe). kit.ts uses its own ms (Date.now) and frames; clock.ts makes seconds. replay_schedule.cpp is all ms. No fractional-ms rounding rule: Date.now is integer ms, so down_ms/up_ms in the press log are 1 ms resolution while clock_ms_at_press is a float.
- Clock to game frames: no home. The only link is the press reply's down_frame/up_frame and the 3 ms constant.
- Audio time vs song clock: sp_watch logs audio_s, raw_s, corr_s (R/sp_watch-1.csv header). corr_s was constant 8.598 s in the rows around 126 s in R. clock_s = raw_s in rows read. Which of them the optimal press path uses is: raw (play_chart). Another home: clock.ts reads GameManager `songTime` (a different field again) at 250 ms (clock.ts:14). Three song-time sources (engine song_clock in memory, GameManager.songTime, audio_s) for different tools; I did not compare them.

### T1. Each fixed number has two or three homes
- Frame 3 ms: replay_schedule.cpp:35 `kFrameMs` (and kJudgeEarlyMs 3, kJudgeLateMs 6, :39-40) and play_chart.py:101 `FRAME_MS`; kit.ts has no frame constant. The C++ cites TM-49 and SP-47 (2.98 to 3.03 ms). R measured 3.033 ms per frame (proven by R/press-1.csv). So the 3.0 is a nominal; R's frames run 1 percent longer. Over a 6 ms judge delay that is 0.2 ms, small. But at the C++ edge: `press_room_ms` uses 2 frames (6 ms) as the worst judge delay (replay_schedule.cpp:249-251) while `last_send_ms` uses 1 frame (3 ms) (:329) and play_chart's drop check uses 1 frame (:1115). So the room accounting says "worst judge is 6 ms late" and the latest-send accounting says "judged 3 ms late". Two answers inside the same C++ file to "how late is a press judged". A press sent at last_send_ms can be judged 3 ms later than last_send_ms assumes. Split.
- PRESS_LEAD_MS 2: replay_schedule.cpp:43, play_chart.py:78, and kit.ts adds none. Edge: it only applies to plain presses; the round trip read to down is about 0 so the 2 ms is all lead.
- SP_END_LEAD_MS 15, SP_OUT_LAG_MS 15, CHORD_ORDER_GAP_MS 8: C++ :49, :54, :61 and Python play_chart.py:83, 88/95 (S1), 113. The C++ writes them into a `constants` object "where the tests pin it" (:16-17), nothing in Python reads it. If one side changes, the other does not follow. A test would need to compare the two homes (not read: tests/ for replay_schedule; I did not open them).
- Window half-gap clamps 37.5 / 85 and the formula: C++ :21-29, Python constants.py:222-267 and play_chart.py:304-332 (S1 C9).
- Room margin 3 ms (C++ :66) and Python ACT_LEAD_MARGIN_MS 6 (S1): different values for a similar "stay inside the window" margin; C++ requires room >= 3 against a worst case of 6 ms judge delay; Python leaves 6 ms of side. Not necessarily the same rule; they are two margins with no link.
- Order-group gap 8 ms: made three ways. C++ gives `after_ms` 8 per group (:784). The kit route makes it in the agent by `Date.now()` on a 2 ms tick, landing on the first frame at or after (kit.ts:523-534, 596-603). The keys route makes it by `time.sleep` in the player (play_chart.py:1277-1279). At 330 fps (3 ms frames) the kit lands the second group 8 to 10 ms after the first (2 ms tick plus 3 ms frame); if a frame stretches over 8 ms (TM-49/50 loaded frames), both groups land in the same frame and the order the gap exists to give is lost. The reply gives each group's down_frame so a run can show it; R had no split chord in its reply columns I read (no `groups`, no after_ms in R/kit-1.jsonl: 0 matches). OPEN: a run on a chart with order groups under load.
- `first.downMs` is read after the first write's promise resolves (kit.ts:578-584), so after_ms counts from slightly after the actual write, not from it. Magnitude not measured (R has no split press). By reading.

### T2. Two clocks read differently
- play_chart: raw only (est = raw, max_fill_s 0). walk_edges and other experiments: est with up to 50 ms fill when the last change was seen within 2 ms. A press timed by "est" in walk_edges is not timed the same way as in the player. Edge: a frame that stretches (raw unchanged for many ms) makes est run ahead by up to 50 ms in walk_edges; in play_chart the read stays at the old frame, so the press fires at the first read whose clock reached the target, which is a late frame, drop check sees it if more than side-3 late.
- wait_until: `ahead <= 0` returns the first read at or past the target; between reads it sleeps min(ahead-30 ms, 500 ms) when ahead > 40 ms, otherwise it spins (live.py:110-146). A clock that jumps forward (frame stretch, or a seek) returns late and the lateness is reported only via the drop check (play_chart.py:1115, late side only, S1 C2). A clock that jumps back more than 1 s raises ClockJumpedBack (live.py:65, 89); less than 1 s back is accepted silently. A clock that does not change for 5 s raises ClockFrozen (live.py:61, 95); a pause shorter than 5 s passes through, the presses then fire late as soon as the clock moves.
- R shows the clock stayed continuous (R/sp_watch-1.csv: only gap over 0.05 s is the 5.0 s at the end of the song, after the last chord).

### T3. Edge table
| Case | What happens | Source |
|---|---|---|
| Press exactly at last_send (side - 3) | C++ room formula says feasible if room >= 3; player sends if raw+3 <= chart+side (equal passes: `>` strict, play_chart.py:1115) | by reading |
| Press read exactly at window edge (chart+side) | load check `<=` passes (S1 C1); drop check refuses (raw+3 > chart+side) | by reading |
| Frame longer than 3 ms (e.g. 8 ms) | Judge delay becomes 1 to 2 of those frames; C++ worst case still 6 ms; both order groups may fall in one frame | by reading; R shows 3.033 ms steady |
| Clock jumps forward 20 ms (R row 324: read +21.74 late) | press sent late, hit anyway; drop check passes (21.74+3 < 85) | proven by R/press-1.csv row 324 |
| Chord at time 0 | first_note_index skips notes within 150 ms of the clock read at start (live.py:152); a first chord at ms < clock+150 is never pressed, no row, no count (S1 C5). Chord at ms 0 with a clock start at 0: skipped unless the clock starts negative | by reading |
| Two chords closer than hold + reply (~8 to 14 ms in R) | next press waits on the request lock until the first reply (kit_tools.py:613, 622); second goes late by up to the reply time; only the late-side drop check sees it. R's smallest gap was 68.5 ms, so R proves nothing for close chords | by reading; open for a dense chart |
| Two chords closer than the 8 ms gap | an order group of chord A at +8 and chord B at its own aim can cross or land in the same frame; far-press check in C++ (far_press_refusal, :392-420) covers windows, not the group gap | by reading |
| Hold 2 frames vs 3 frames | R shows both (hold column up_frame - down_frame is 2 or 3): the agent's hold ends at the first tick whose frame is >= down+2; a tick that misses a frame gives 3 | proven by R/press-1.csv |
| Score read 5 ms after the reply (S1 C6) with replies up to 14 ms | read happens after the reply, so 5 ms is after the game's own frame by then | by reading |
| settings: INPUT_SETTLE_MS 250 (constants.py:327) | used by walk_edges after a press; the player uses a 5 ms read. Two numbers for "when has the game scored" | by reading |

### T4. The built agent is older than its source (kind 1; found by reading file dates and grep)
- Main checkout `tools/ch_mcp/agent/kit.js` is dated 2026-11-06 21:31. `kit.ts` was last committed 2026-11-07 23:11 (commit 2709008f, "Player limits T3: split chords go in one kit message with the 8 ms gap made in the game"). grep of kit.js: `after_ms` 0 hits, `groups` 0 hits, so the built agent does not have the T3 split-chord code. grep of kit.js for `then` gives 3 hits (likely Promise.then), `rewrite` 3, `on_main` 2, so it does have the older press. Result: a split press (`then` list) sent to this kit.js would be answered by an agent that does not know `then`; nothing in Python checks the agent's build against its source (searched ch_mcp/*.py outside tests for mtime/version checks: none). Runs on 2026-11-08 (R included) therefore ran the older agent. R had no split chords in its replies, so R is unaffected.
- `launch.js` is dated 2026-10-31 13:17; `launch.ts`, `launch_core.ts` were committed/edited 2026-11-07 (b24d9b8f was a comment-only repoint; the file dates are 18:26). `menuhooks.js` 2026-10-31 vs `menuhooks.ts` 2026-11-07 18:26. Likely comment-only changes (b24d9b8f "Repoint tools comments"), content not diffed. Only kit.ts has a behaviour commit after its build.
- `menu.ts` has no built `menu.js` and no `build:menu` in package.json (package.json lists clock, trace, state, menuhooks, kit, launch). frida_tools.py:67 has a `MENU_BLOCK = "menu"` and tests (test_frida_attach.py:60, 840) treat `menu.ts` as an agent source. How menu.ts is loaded at run time I did not read (open: frida_tools.py).
- The built .js files are git-ignored (tools/ch_mcp/.gitignore line 2), as the common brief says; a new checkout or worktree has no agent at all.

### T5. Smaller timing notes
- clock.ts samples every 250 ms (SAMPLE_MS, clock.ts:14) with `Date.now()` as `t`: wall ms, JS side, not the python clock; fine for a coarse summary and not for press timing. Not on the press path.
- play_chart's `sleep`-based spin thresholds (_SPIN_WITHIN_S 40 ms, _WAKE_BEFORE_S 30 ms, live.py:113-115): a press needing a 2 ms lead is found by spinning for the last 40 ms; Python on Windows then reads the clock at its own rate (R: read-to-clock error median -0.5 ms, max early -2.0, proven by R/press-1.csv).
- down_ms/up_ms in the log are `Date.now()` integers; wall_s_at_read is a float. Subtracting them (as I did) has a 1 ms error.

## Splits (each its own line)
1. How late a press is judged: replay_schedule.cpp room uses 2 frames (:250), last_send_ms uses 1 frame (:329), play_chart drop check uses 1 frame (:1115). Same file, two answers.
2. Frame length: 3.0 ms in C++ (:35) and Python (:101); R measured 3.033 ms.
3. All seven press numbers (frame, lead, SP end lead, SP out lag, order gap, window clamps, margins) live in C++ and in Python with no link; Python does not read the C++ constants object.
4. Order-group gap made three ways: C++ plan 8 ms; kit.ts on a 2 ms tick plus frame; keys route by time.sleep.
5. Clock: raw (play_chart) vs est with 50 ms fill (walk_edges and the other probes).
6. Built kit.js (no split groups) vs kit.ts (has them).
7. Settle time: 5 ms score read (player) vs INPUT_SETTLE_MS 250 (probes).

## Open (a run would settle)
- Who brought the window forward at about 126.8 s in R. A run with a window-event trace (not run by me).
- Whether SetForegroundWindow succeeds from the MCP worker (a background process) on this machine; keys route only.
- Order groups under a long frame (kit.ts) on a dense chart.
- Close chords (< 15 ms apart) through the request lock.
- Whether the Python grabs of the keys route are ever reached while a kit run is going (ch_key/ch_press during a run); R's folder does not list its tool calls.

## Side notes outside my families
- game_tools.py:1448-1450: kit route passes `press` only; keys route passes `driver`. Right now `route` None with kit set up defaults to kit (:993-996). Fine.
- S1 C7's "default CLI run takes the keys route" is confirmed by play_chart.py:1333-1340.
- R/replies.txt line 1-2 "dll md5 before/after" identical: no change to the game file on disk.
