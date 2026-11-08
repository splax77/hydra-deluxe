# player-audit-S3-V5 (agent a305b1b3774854f96)

Candidates: L67, L68, L69, L70, L71, L72, L73, L83, L84. Extra: the L77 quantiles check. Everything is checked against the main checkout `C:\Users\Patrick\Downloads\Hydra\hydra-test`. "tools/ch_probe/experiments/" is shortened to "exp/" below. Note: `play_chart.py`, `edge_probe.py`, `analysis.py`, `overlay_logs.py`, `live.py`, `sp_watch.py`, `sp_compare.py` live under `tools/ch_probe/experiments/`, and `engine.py`, `engine_finder.py`, `constants.py` under `tools/ch_probe/`; the ledger names them without the folder.

## L77 extra check: statistics.quantiles with one data point on Python 3.14

I ran two `py -c` calls (one more than "exactly one", because the first used the default method and `machine_report.py:487` passes `method="inclusive"`):
1. `py -c "import statistics,sys;print(sys.version);print(statistics.quantiles([5.0],n=100))"` printed `3.14.5 ...` and a list of 99 values, all 5.0. No raise.
2. `py -c "import statistics;print(statistics.quantiles([5.0],n=100,method='inclusive')[94])"` printed `5.0`. No raise.

So the call at `machine_report.py:487` (`statistics.quantiles(steps, n=100, method="inclusive")[STEP_PERCENTILE - 1]`) does not raise on 3.14.5 for one data point. Proven by running. (V3's NOT CONFIRMED stands, now by run instead of by memory.) Whether it raises on an older Python (3.12 and earlier) was not run.

## L83 edge_probe: PARTLY (E2 KeyError NOT CONFIRMED, E1 three forms of "lead" PARTLY)

KeyError claim: NOT CONFIRMED by reading.
- `exp/edge_probe.py:115-120` builds `by_index` from the chords and raises ValueError ("probe chords ... are not in the score file") for any probe index not in `chords` before `first` is used. `:124` `first[p.index].lead_ms` runs only for indexes that are in `chords`.
- `_first_presses` (`:92-97`) takes the first press press_plan gives each chord. press_plan (`exp/play_chart.py:431-446`) calls `_chord_presses` -> `_timed_presses` for every chord, and every branch of `_timed_presses` (`:564-629`) returns a non-empty list: `in_order` goes through `_order_groups` (`:488-504`), whose last line `... or [every]` is commented "A declined chord that sends nothing is still one press, of no keys". The squeeze-out branch returns `[late]` or `... + [late]`. So a declined-empty chord still has a press and `first[p.index]` exists. The ledger's example ("a chord with no press in press_plan, e.g. declined empty") is wrong.
- What can still stop `probe_chords` is a ValueError from press_plan itself (`play_chart.py:582-584`, `_solve_lead`), which is a loud ValueError, not a KeyError. By reading. No test in `tools/ch_probe/tests/test_edge_probe.py` pins this (grep for KeyError/first[/declined found nothing).

Three forms of lead: the arithmetic says two of the three are the same number for the first press of a chord.
- `ProbeRun.shifts[i] = asked - first.lead_ms` (`:124`). `mark_row` (`:55-65`) adds the shift to `row["lead_ms"]` only `if "lead_ms" in row`. `play_chords` always writes `lead_ms` in its row (`play_chart.py:1175`). So the row's adjusted `lead_ms` = `first.lead_ms + shift` = the asked lead for the first order group. For a later order group of the same chord (`lead - k*CHORD_ORDER_GAP_MS`, `play_chart.py:576`) the shift is the same, so its adjusted lead is the asked lead minus k*8 ms, not the asked lead.
- `PROBE_COLUMN` holds the asked lead on every row of the chord, later groups too (`:65`).
- The third form, `chart_ms - clock_ms_at_press` (`probe_row_lines`, `:163`), is the lead as the clock read it, so it differs by the read-to-clock error (see L73). That is the point of the line ("asked X, pressed Y").
- `lead_ms` is not in `PRESS_LOG_COLUMNS` (`exp/overlay_logs.py:99`), so the CSV press log never holds the adjusted `lead_ms`; it exists only in the in-memory rows. Verified by reading `overlay_logs.py` (all columns listed at :68-99) and by the header of `ls-m0-run1/press-1.csv`, which has no `lead_ms` column.
Splits: asked lead (PROBE_COLUMN) vs adjusted row `lead_ms` on a later order group: they differ by k*8 ms. Asked vs read-off lead differ by the read-to-clock error (0 to about 3 ms in ls-m0-run1, see L73). By reading.
Evidence kind: by reading. Open: none (a live probe run would show the same).

## L84 find_window_edge and overlay_logs: PARTLY (the "never at a threshold" sentence is wrong)

- `exp/analysis.py:66-123`: error rule is `hit and mag > threshold` or `(not hit) and mag <= threshold` (`:107-110`), so a hit exactly at the threshold counts as inside, as the ledger says. Candidates (`:97-100`) are `mags[0]-1`, midpoints of neighbours, `mags[-1]+1`. By reading.
- Wrong part: "an exact-edge sample is never at a threshold". When two samples have the same magnitude, their midpoint equals that magnitude, so a candidate threshold equals a sample's magnitude. A miss at that magnitude then counts inside (`mag <= threshold`) and costs an error; a hit does not. It is consistent, not a bug in the rule. By reading; no run.
- Side note (new): `candidates` also holds that value once per duplicate pair, and `best_thresholds` (`:113-115`) then holds it more than once, so `statistics.median(best_thresholds)` (`:117`) weights duplicate-magnitude cuts more than single ones. Only matters when samples repeat a magnitude and several thresholds tie. Whether callers produce repeated magnitudes was not checked (callers: `exp/walk_edges.py:187`, `analysis.py:337`).
- `exp/overlay_logs.py` is column lists plus `watch_header` (a small rename function, `:63-66`), as the ledger says ("column lists only" is slightly short: `watch_header` is code). By reading.
Evidence kind: by reading. Open: none.

## L69 Empty and edge inputs to the player: CONFIRMED

All by reading `exp/play_chart.py`.
- `:1375-1377` no chords: prints "has no chords to play." and returns 0 (not an error exit).
- `:680` `json.load(f)["paths"][path_index]` raises KeyError/IndexError on a dump with no such path (ValueError only for a path_source mismatch, :683-686).
- `:701` `act["skipped_fill_ticks"]` is a direct index: KeyError for a dump without the key. (`:869` elsewhere uses `.get("sqin_ticks", [])`, so the copies of the access style differ.)
- `:933-935` `load_dump_notes` calls `load_dump_chords(path)` with no dump, so a score file with `sqout_tick >= 0` raises ValueError there (`:764-767`). The docstring at `:925-931` states the no-dump limit only for `load_score_times`.
- No order check on chords (grep for sorted/ascending found nothing that checks chord order; `window_sides` docstring `:337` says "in time order"). `half_gap_ms` (`:304-312`): a negative gap gives `max(gap/2, 37.5)` = 37.5 then `min(.., 85)`: silent. `None` or 0 gives 85 (`EXPECT_NORMAL_BACK_MS`, `constants.py:227`). One chord: both gaps None -> 85+85 -> `window_side_ms(170)`; by my arithmetic from the constants (`constants.py:259-262`) = ((170*0.0110924 - 170^2*2.6285e-5)*170 - 20)/2 = 85.7, matches 85.72.
- TM-19 (`docs/clone-hero/03-timing.md:225`) says "A missing neighbour, or one at the same time, gives the back value", which is what `half_gap_ms` does for 0. TM-18 (`:213-216`) is about the clamps; the ledger's "TM-18/TM-19" pair is right. Nothing to add.
Splits: none.
Open: none.

## L68 Rules Python works out that Hydra or the reference already states: CONFIRMED (homes exist; one doc basis is thin)

By reading `exp/play_chart.py`.
- `:595-599` the FirstNote note: "the gem Hydra's FirstNote rule pays outside is note 0 of Chord::note_list(true) (src/core/model.cpp)". `src/core/model.cpp:156` defines `Chord::note_list(bool basesorted)`, and `src/core/replay.cpp:149` uses `ts.chord.note_list(true)` to order the score file's notes, so "the file's first gem" has a C++ basis. (I did not open replay.cpp past :149 to see the JSON write order.)
- `:731-744` and `:131`: `_TOP_PAD_ORDER = ("Green","Blue","Yellow","Red")` with the comment "Row P22 ...: unchecked until a run shows it". It is a Python-only copy; grep of `src/` for highest/top pad/flagged/activating gem finds no C++ owner of "which pad the game flags" (only `src/parse/song.cpp:723 highest_pad_pitch` and `note_shuffle.h:22`, which are about MIDI pitches). FL-09 (`docs/clone-hero/04-fills.md:90-94`) says the flagged note is picked by `0x20D51D0` and that "highest-pad" comes "from docs/audit/drum-lanes/FINDINGS-play-time.md line 50 only", so the order green>blue>yellow>red rests on one secondhand line. Real thin basis, as the ledger hints.
- `:107-113, 488-527` `CHORD_ORDER_GAP_MS = 8.0` and order groups by multiplier; SC-38 exists (`docs/clone-hero/06-scoring.md:113`, label "Proven in a live run"). The comment cites "the amendment after LS-R1b run 1" of `docs/superpowers/plans/2026-11-07-player-refinements.md` (file exists).
- `:765, :899`: `sqout_tick >= 0` at :765 and `act["sqout_tick"] < 0` at :899 are the same sentinel in two forms (also :904 `>= 0`). Confirmed.
- `:115-118` `SQUEEZE_IN = "SqIn"`, `SQUEEZE_OUT = "SqOut"` match `src/core/model.h:412-414` `type_name()` ("SqIn"/"SqOut"). Same strings in two languages with only a comment tying them (the dump parser has no list check; a mismatch would show as an unknown kind).
- `:83, 88, 95, 113`: each comment names a plan question. The 2026-11-06 optimal-score-run plan is not in the tree (Glob of `docs/superpowers/plans/2026-11-0*` lists 8 files, none dated 11-06); the comment at :82 says "(removed; git show d7e47a20:...)". I did not run `git show` to check the commit holds the file. The 2026-11-07 and 2026-11-08 plan files named at :87, :93, :99, :111 exist; I did not read them to check the quoted answers.
Splits: none beyond L46 and L47.
Open: whether the green>blue>yellow>red order is right needs a live run or the game code (FL-09 says so itself).

## L70 Kit press reply: CONFIRMED

By reading.
- `agent/kit.ts:442-450` `write` sets button/axis values by id; `:535-662` `press` returns `down_frame`, `up_frame`, `down_ms`, `up_ms` (Date.now() ints, :584, :633) and echoes `hold_frames`, `rewrite`, `on_main`; nothing in the reply says the game read a hit. `readBack` (:477-486) runs only when the message carries a `read` list (`readNames` :458-467, `look` :559-563), so a press the game's Rewired player ignores returns normally.
- `TICK_MS = 2` (`kit.ts:61`); the hold loop sleeps `TICK_MS` between steps (:592). A later group goes down on the first tick at or after `after_ms` (:597), so its down time is quantized to ticks.
- Failed writes are loud: the down write sits in the `try` (:577-583), the release runs on every path (:624-643), and errors are thrown (`faultOf`/error replies at :893, :962-964). `frida_attach.py:399-402` turns an `error` reply into `ReadFailed(END_AGENT_ERROR)`.
- `kit_tools.py:843-851`: missing `down_frame`/`up_frame` prints "?" for frames and "? ms" for time instead of failing; this is the text tool (`ch_kit_press`) only. The player's press at `kit_tools.py:615-630` returns the agent's reply dict and re-raises with fault lines on `ReadFailed`.
Splits: none.
Open: whether a press the game ignores shows in a real run needs a live run (the score rise shows it; the ledger says so).

## L71 state/trace/menu quiet paths: CONFIRMED

By reading.
- `agent/state.ts:24-32` `menuBlock` catches any exception from `readMenu()` and returns `{error: ...}`; the GameManager read beside it still succeeds (comment at :24-25 says so).
- `agent/trace.ts:145-163` `Interceptor.attach` failures are pushed to `notHooked` and the trace goes on; `:140` sends `hooked: targets.length`; no check refuses 0 targets in count mode (the only refusals are the shape check :111-114, the GameManager lookup :117-119 and missing named methods :130-137, which applies only when `wanted !== null`). The trace then runs to its cap or the song end. `trace_hits.py:208-213` prints "hooked: N code addresses ..., not hooked: M" and one "not hooked:" line per failure, so the failure is shown in the count report.
- `agent/menu.ts:230-236` `isLivePlayerScreen` has the bare `catch { return false; }` (the comment at :225-229 says a throw means "gone"). `:347` `live.get(SONG_SELECT) ?? []`: an absent or not-found class gives no selected song (null), indistinguishable in `selectedSong` from an empty SongSelect. (The class entry itself carries `found:false, reason` at :286, :290.)
Splits: none. Open: none.

## L72 Server and tool reply quiet paths: CONFIRMED for the seven listed items; two named homes hold none of them

By reading.
1. Reply after detach dropped: `frida_attach.py:405-410` `_on_message` returns when `self._log is None or self._log.closed`. Confirmed.
2. SP watch read failure: `exp/sp_watch.py:139-143` catches OSError and ends the run with reason "lost the engine (...)" (the loop, `:131-154`). The CSV is then short. Confirmed. (`sp_watch.py:120` in the ledger is the docstring line naming the stop reasons.)
3. Blank clock cells: `engine.py:273-290` `clock_block` returns None when a pointer is null or a read raises OSError (comment: "losing these three columns must not end it"), so clock columns go NaN/blank. `exp/sp_compare.py:155-167` `load_watch_rows` -> `_number` (:150-152): `int(text)`, then `float(text)`, which raises ValueError on a blank cell. Confirmed: blank written, read raises ValueError. Only for rows where clock columns are in the header; an older header is filled with NaN (:166).
4. First engine whose clock changed: `engine_finder.py:202-227`, `candidates` dict keeps scan order ("ordered, so scan order decides ties", :203), the first `abs(c1-c0) > C.ENGINE_CLOCK_MOVED_S` returns (:226-227). The docstring says it (:183-186). Confirmed.
5. Play-thread errors: `game_tools.py:1472-1478` catches `Exception`, stores `play.error = "Type: msg"`, "reported by ch_play_status" (comment). `_stop_reason` (:1491-1494) shows it as "failed: ...". Confirmed.
6. Press-log write failure: `game_tools.py:1452-1464, 1466-1471, 1479-1487` sets `play.log_note` and the run goes on; `_press_log_line` (:1497-1499) shows it. Confirmed.
7. `quiet.py:216-223` free-memory and power-scheme read failures append a "could not read it" line and do not set `quiet = False` (the CPU, process list and folder reads do, :184-186, :198-200, :208-210). Confirmed by reading; whether these two lines are meant to gate the launch is not stated in the code.
8. `compare_scorestats.py:85-94`: `stats.get("players")`, `int(p["total_notes"])`, `hydra_counts` direct indexes can raise KeyError. `game_tools.py:867-869` (`ch_compare`) catches `(OSError, ValueError, KeyError, TypeError)` and prints "compare failed: KeyError: 'x'" and `sp_tools.py:301, 311` print the same shape. Confirmed.
Not confirmed as quiet: `install_chart.py:85` is inside `except BaseException:` that removes the half-made folder and re-raises (`:85-90`), loud. `machine_recorder.py:332-334` prints "ignored stdin line" to stderr; `:372` `ClockReader.read` returns None on OSError/ValueError by design ("A failed read is None and the next one tries again", :359). These homes are in the ledger's list but no listed item relies on them; I did not find any item for them.
The split ("two engine searches in one run are shared only in ch_run_chart") was not verified: I read `game_tools.py:186-198` (the capped normal-mode search, with a message saying precision engines are not found) and `engine_finder.py:159-164` (`all_patterns` = normal + precision pair in both orders), so the two searches differ in pattern list as stated. Not checked which callers share one result.
Open: none.

## L73 Smaller timing notes: CONFIRMED homes; the R-M0 numbers hold in magnitude

- `agent/clock.ts:14` `const SAMPLE_MS = 250;` and `:18, :39, :44` use `Date.now()`. Confirmed.
- `exp/live.py:113-115` `_SPIN_WITHIN_S = 0.04`, `_WAKE_BEFORE_S = 0.03`, `_MAX_SLEEP_S = 0.5`. Confirmed (the ledger says 40 ms and 30 ms).
- `constants.py:208-218` `MS_PER_S`, `s_to_ms`, `ms_to_s`. Confirmed.
- R-M0 numbers, proven by a script I ran (two scripts, not kept, over `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m0-run1\press-1.csv`, 1478 rows): smallest chart-time gap between consecutive rows 68.488 ms (ledger: 68.5, matches); every `down_ms` is an integer (matches "Date.now() integers"). The CSV has no `lead_ms` column, so the "read-to-clock error" can only be rebuilt by assuming the 2.0 ms plain lead (`PRESS_LEAD_MS`, `play_chart.py:78`): `chart_ms - clock_ms_at_press` then has max 2.0 (0.0009 ms after the plan) and min -1.03; `clock_ms_at_press - (chart_ms - 2)` has median 1.505 and range 0.0009 to 3.03 over 1477 of 1478 rows (one row at 23.7 ms, likely an activation press with another lead). The finder's "median -0.5, max early -2.0" matches this in size (median 0.5 ms, up to 2.0 ms) under the sign convention chart minus clock; the finder's exact definition was not available, so I call it PARTLY proven (magnitudes agree, definition assumed).
- Three song-time sources (engine song_clock, GameManager.songTime in clock.ts, audio_s in sp_watch): all three names exist (`engine.py:292` `song_clock`, `clock.ts`, `overlay_logs.py:45` `audio_s`); "not compared" is a claim about absence that I did not search for.
Latent: no. Open: none.

## L67 Test fakes: CONFIRMED (each home exists and does what the row says); no mismatch found in C3 or F4

By reading.
- B5 `tests/test_machine_recorder.py:43-69`: the table is packed with the parser's own `M.PROC_*` and `M.THREAD_*` constants (lines 51-69). Confirmed. A wrong offset constant in the parser would pass this test; only LS-T1 (live) settles it, as the row says.
- C3 `tests/test_menu_tools.py:35-56` `state()` builds each class entry with keys found/live/destroyed/active and (for MainMenu/SongSelect) onlineMenuActive. `agent/menu.ts:283-313` `readClass` returns those same keys plus optional `fields`, or `{found:false, reason}`. `ONLINE_HOLDERS` in the test = `ONLINE_MENU_HOLDERS` in `menu.ts:56`; `MENUS` (:31) = `MENU_CLASSES` (`menu.ts:48`). The fake never returns `entry.fields` or `found:false` (confirmed); `menu_tools._class_entry` (`:81-83`) handles non-dict entries. No difference in shape.
- F3 `tests/test_frida_attach.py:169-172` `FakeScript.post` calls the message handler synchronously on the calling thread; `:192-198` `FakeSession.detach` calls the "detached" handler synchronously. Real Frida delivers on its own thread, and `frida_attach.py:147-153` (the run function's `handle`) takes `lock` with the comment "Frida calls this on its own thread; the lock keeps a late message from writing while the log closes." (A second handler, `_on_message` at :405-419, is a different class with its own lock.) So the lock paths are not exercised across threads by these fakes. Confirmed.
- F4 `tests/test_bot_run.py:539-541` `PATCHED` has exactly the keys of `launch_core.ts` `Status` (`:43-53`): state, before, after, entry_before, module, loader_runs, bytes_at_first_run, patched_at_ms, reason. `module={"base": "0x7ff800000000", "size": 1}` matches `ModuleSeen {base: string; size: number}` (`launch_core.ts:32-35`). So the "module value not compared with ModuleSeen's type" question is answered: the shapes agree.
- F5 `tests/test_sp_tools.py:48-51` `snap_at` decodes an all-zero snapshot; `:87-88` `FakeProc.verify_targets` only appends "verify". Confirmed. `FakeGame.run_cap_s` (:78-80) returns 240.0 (this is L74's item).
- `tests/test_trace_hits.py:46-48` `call()` builds `{call, names, songTime, args}`; `agent/trace.ts:152-154` sends `{call, names, songTime, args}`. Same keys.
- W2A-K: "no test compares a built .js to its source or the baked menu_fields to the file": I did not search for that. Not verified.
Splits: none found.
Open: LS-T1 (live proof of the recorder offsets), as the row says.

## Side notes (outside the candidates)
- `exp/analysis.py:117`: duplicate-magnitude thresholds are counted more than once in the median (see L84).
- `exp/play_chart.py:701` vs `:869`: one dump key is read with a direct index and another with `.get(..., [])` in the same file (see L69).
