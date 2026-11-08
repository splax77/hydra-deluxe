# player-audit-S2-W2B (second-wave finder, agent a4382c0b82b3bd1d6)

Read-only. All claims "by reading" unless marked. Nothing run.

## Read in full
tools/ch_mcp: machine_recorder.py, machine_report.py, recorder_link.py, game_files.py, menu_tools.py, install_chart.py, build_facts.py.
tools/ch_probe: process.py, interfaces.py, probe_songs.py, probe_chart.py, experiments/overlay_logs.py, edge_probe.py, analysis.py.
C++: tests/test_replay_schedule.cpp (all 793 lines) and tools/replay_schedule.cpp (all 1003 lines, to judge test gaps).
Partly read: test_menu_tools.py (1-200), test_sp_tools.py (20-150), test_bot_run.py (486-640), test_frida_attach.py (fakes at 150-250), test_trace_hits.py (1-80), test_machine_recorder.py and test_machine_report.py (grep of builders and class list only), test_server.py (class list only).

## A. Candidates in the schedule (tools/replay_schedule.cpp) and its test gaps

A1. Three different "latest press that is still inside" limits (kind 5, 4).
- press_room_ms (cpp:249) judges a press 2 frames late: room = side - max(|aim|, |aim + 6|).
- last_send_ms (cpp:329) = chord + side - 3 (one frame). The test pins it ("the side less one frame", test:616).
- far_press_refusal (cpp:392) tests the raw send time against the side with no frame added; test:627 pins "sent exactly on the edge is inside".
- sqout_fallback_after_ms uses 2 frames (cpp:333); sqin_fallback_before_ms uses 1 frame (cpp:339).
- Split: one question ("how late may a press be sent and still be judged inside the note's window") has answers of 0, 1 and 2 frames. Inputs: side 50, chord 1000: last_send 1047; far_press accepts 1050; room-based safe send is 1044 (aim +44 gives room 0). A press sent at 1047 can be judged at 1053 and miss.
- Also room uses |aim| (the unjudged send) on the early side while judging happens 1 to 2 frames later and so is closer to the note: aim -52, side 50 gives room -2 although judged at -49..-46 is inside. Stricter than the model it states.
- Test gap: no test puts a press between last_send_ms and the 2-frame limit; the 1-frame vs 2-frame choice is pinned only as constants (judge_early 3, judge_late 6).

A2. A squeeze-out chord ignores the declined-chord gem list (kind 4).
- cpp:848-860 builds the inside groups from mults[i] (all gems) and the late gem as place 0, never from sent[i]. The squeeze-in branch (cpp:881) and plain branch do use sent[i].
- If a squeeze chord's tick is also a skipped fill tick, the schedule presses gems the decline mode would not send. Not tested (no test combines a squeeze with a declined chord). By reading.

A3. Chords can drop out of the press list when a refusal is raised (kind 2, 6).
- cpp:818-823 (a lead chord that is also a squeeze or an end), cpp:836-841 (no split, no read), cpp:850 (late gem pays less) `continue` with no press row for the chord. The file still carries `refusals`. Whether the player refuses to run on a non-empty `refusals` is outside my files; the file itself shows a chord list with fewer press rows than chords. Open: check play_chart reads `refusals` and stops.
- Declined chord with nothing to send: press_groups returns one group with empty gems; the row has "gems": [] and skips the far-press check (cpp:939 `!p.gems.empty()`). An empty press row is a press that presses nothing. Tested only at press_groups level (test:719), not in the file.

A4. Edges of replay_schedule.cpp with no test (by grep of the test file):
- rule "sp_end" (cpp:897-905) and rule "inside_moved_end" (cpp:908-926): no test names either rule string. No test sets a final-end chord pressed 15 ms early, and none pins a chord paid inside before a moved end.
- Refusals with no test: "chord N is two squeezes" (cpp:643); "leads a squeeze and is also a squeeze or an end" (cpp:820); "no chord at skipped fill" (cpp:616); "has no SP-end steps" (cpp:470); "names no chord in the record" (cpp:489); "has no end in the record" (cpp:490); SqIn missing `transfer` (only the SqOut `transfer_post` missing case is tested, test:165); "has no SP-end step on chord" (cpp:731); "has no stored ratio ... end read" (cpp:743).
- schedule_for_windows (cpp:990) throws "the engine cannot realize this path": no test.
- Empty chart or no chords (replay.chords empty, acts with no chords): untested.
- replay.chords[i] is assumed to be song.sequence[i] (cpp:572, 798): the test comment says the list is never pruned, but nothing fails if it is. Test at 241 uses c.index for the same lookup, so the two disagree on which index names the sequence entry.
- Lead below kPressLeadMs refusal (cpp:303): untested. Lead steady-span refusal tested for the early side only (test:654); the late side (after_anchor_ms, after_end_ms) untested.
- KickOnly with a chord that has no kick (test only uses chords that have one): returns empty, untested. AllButTopPad with notes empty: untested.
- The both-directions refusal (test:638) is tested only through anchor_lead_ms, not through the whole file.
- Empty kept gems: covered only for "kick alone under AllButTopPad" (test:222) and the corpus walk; no test that the press list for such a chord has one row with gems [].

A5. Constants with several homes (kind 5). cpp:21-22 repeats 37.5 and 85 ms; ch_probe/constants.py:222-223 (EXPECT_NORMAL_BACK_S, FRONT_S) has them again; src/core/model.h has kHitWindowMs = 85 (test:372 pins it). The test checks the model.h literal but nothing ties the C++ and Python copies together. Tick rate: probe_chart._ticks_per_ms has a test against Hydra (test_tick_rate_vs_hydra, named in its docstring); frame time and the 15/2/8 ms numbers exist in C++ only, and play_chart.py has its own (not mine to read).

## B. Machine recorder and report

B1. Clock read from a stale or wrong engine pointer is recorded as a clock (kind 2, 6). machine_recorder.py:366-373 reads engine + OFF_SONG_CLOCK from an address the worker sent once. Only OSError/ValueError give None. A freed or moved engine object returns some other double, written as clock_s; machine_report then judges "the game's clock kept up with wall time" from it. No range or monotonic check; no check the engine object is still the engine. Open: a live run that restarts a song under the recorder would settle it.

B2. Recorder never checks the game build (kind 1). It uses C.OFF_SONG_CLOCK (0x100) with no verify_targets call (process.py:196 is the only build-drift guard and it reads two rdata doubles only). The worker's build_facts check compares the game's version.json string with a facts file, not the exe bytes. build_facts.py:97-127 refuses only on version mismatch. A changed exe with the same version string passes. DUMPS_ROOT (build_facts.py:28) and GAME_DIR (game_files.py:32) are hard-coded user paths.

B3. machine_report may crash instead of refusing (kind 2). machine_report.py:487 `statistics.quantiles(steps, n=100)` with exactly one clock step raises StatisticsError on Python versions that require two points; the code only catches Refused. Also `_share` (line 243) divides by span_ms, zero if all kept samples share one wall stamp. Open: the Python version in use decides the quantiles case.

B4. settings.ini read by two parsers that disagree (kind 3, 5). machine_report._settings_lines (line 250-263) matches `framerate`/`vsync` lines in any section, game_files._game_setting_line reads only the [game] section. The report prints framerate lines without saying which section.

B5. Recorder fake is built from the parser's own constants (kind 7). test_machine_recorder.py:43-69 packs the fake table with M.PROC_*, M.THREAD_* offsets, the same names parse_table reads. A wrong offset would pass. By reading, I compared the offsets with the documented SYSTEM_PROCESS_INFORMATION / SYSTEM_THREAD_INFORMATION x64 layouts from memory (all agree); live proof is LS-T1 per the plan, which I did not read.

B6. recorder_link.start (recorder_link.py:108-110) re-raises after setting _said, and its docstring says a recorder never harms a run; the reply line says "the run goes on with no recording". That is a logged-and-continue path (kind 2) by design in the code comment; the caller is not in my files.

B7. Recorder samples fall behind silently: record() (line 431) skips ticks when late. The report prints gap stats (machine_report.py:280-284) so this is reported, not hidden.

## C. Menu driver

C1. ch_open_song's last line says "done:" even when the wrong song is loaded (menu_tools.py:286-289 "done: a song is loaded, but its folder is X, not the target Y"). A reply starting "done" with a wrong song is a run reported as fine (kind 6). The checked-in menu_steps.json has no search step ("no step searches"), so the song loaded is whatever the list had selected; SEARCH_NEXT_ITEM = kit:blue is a guess (menu_tools.py:44-50). menu_steps.json says verified false. Caller (ch_run_chart / server) behaviour not in my files.

C2. Expect words rest on "unproven" reads (menu_tools.py:113-119): `active` on menu GameObjects was never settled (LS-K3 struck). Kind 8: assumptions the Clone Hero reference may not back. I did not open docs/clone-hero page 07 IN-79.

C3. Fake vs real: test_menu_tools state() (line 35-56) matches menu.ts readClass (classes: found/live/destroyed/active, onlineMenuActive only for MainMenu and SongSelect, selectedSong.folderPath, globals.currentSongFolder). No difference found. Not matching: the fake never returns an `entry.fields` block or {found: false, reason} for a class; menu_tools `_class_entry` handles them (returns {}), untested here (not read past line 200).

## D. Install and probe songs

D1. Song length has four homes (kind 5, 3). (a) probe_songs.song_length_ms = last note + 3000 (probe_songs.py:201), used by trace_hits.trace_cap_s; (b) game_files.song_ini_length_ms (first `song_length` line wins, game_files.py:357-367 "setdefault") used by install_chart for the silent ogg; (c) Hydra's read_song_ini_keys (last line wins; game_files.py:386 says so); (d) the game ends the song by GameManager songLength (docs/clone-hero/01-reading-a-chart.md:465), not song.ini song_length. Split: a song.ini with two song_length lines gives an ogg of the first and a Hydra length of the last. D255 says one song end everywhere; I did not check whether the player run uses (b) or (c).

D2. install_chart copies the whole source folder (shutil.copytree, install_chart.py:82) then overwrites song.ogg only. Other audio files in the source (drums.ogg, guitar.ogg, song.mp3) stay and the game will play them; the docstring says only "a silent song.ogg". Testdata folders may have none. By reading.

D3. install_chart `--name` is joined onto dest_root unchecked (install_chart.py:78); a name with `..` writes outside the probe folder, and the failure path rmtree's that dest (line 89). Low risk, hand-run tool.

D4. probe charts: Offset 0 and `delay = 0` written; 1 tick = 1 ms at 480/125. ms_to_ticks rounds (probe_chart.py:70); at these constants no rounding. Fine. interfaces.py describes the SendInput InputDriver (interfaces.py:209-235); the focused-window route. I did not trace whether any play run imports input_driver (play_chart.py not in my list). Report as a path into focus use if play_chart imports it.

## E. Experiments

E1. edge_probe.mark_row (edge_probe.py:55-65) edits the log row after the fact: chart_ms and lead_ms are moved back by the shift, so the log shows the chord's real time with the lead the press "really used". The log has two numbers for the lead: PROBE_COLUMN (asked) and the adjusted lead_ms. probe_row_lines recomputes the lead as chart_ms - clock_ms_at_press, a third form. Number with source in the code but three homes for "lead".

E2. edge_probe.probe_chords (line 124) takes first[p.index].lead_ms for the shift. A chord with no press in press_plan (e.g. declined empty) would KeyError (not caught, not a ValueError). By reading.

E3. analysis.find_window_edge treats a hit with magnitude exactly at threshold as inside (`mag > threshold`), and misses `mag <= threshold` as errors; candidate edges are midpoints, so the exact-edge sample is never at a threshold. Fine. predicted_window_normal reproduces the game formula (a second copy of replay_schedule.cpp whole_window_ms; kind 3/5). Constants differ in form: python takes them live from the engine; C++ hard-codes 0.0110924369747899, 2.628539e-05, 170, 20 (cpp:26-29). Two homes of one formula.

E4. overlay_logs.py is column lists only. No problem found. The column-list home is single.

## F. Trace and report numbers

F1. trace_hits.args_report (trace_hits.py:240-243) counts "calls within 85 ms of their nearest note", with 85 = EXPECT_NORMAL_BACK_MS. Three problems (kind 3, 6): the limit is a flat 85 ms, not the note's own side (TM-19 to TM-22; the schedule file has side_ms); line_up pairs a call with its nearest note even at 500 ms (test shows +500 ms); and no line says how many notes got no call. A run with half the notes unpressed and double calls on others can report "N of N within 85 ms".

F2. trace_hits.line_up (line 157) `min(notes, ...)` raises ValueError when the manifest has no notes; trace_cap_s (line 119) raises IndexError on `notes[-1]`. Not caught. By reading.

F3. Fake vs real, test_frida_attach: FakeScript.post (line 169-172) delivers every canned message synchronously inside post(), and FakeSession.detach (192-198) calls the detached handler synchronously. Real Frida delivers both on its own thread, later. frida_attach.py:147-153 takes a lock for that reason. The tests cannot show a race or a lock-order fault. Kind 7. Message shapes: trace.ts call message {call, names, songTime, args} equals test call() (test_trace_hits.py:49-51); done message {done, mode, calls, not_hooked, ...} not checked against the fake beyond that.

F4. test_bot_run FakeLaunchAgent PATCHED (test_bot_run.py:539-541) has the same keys as launch_core.ts status (state, before, after, entry_before, module, loader_runs, bytes_at_first_run, patched_at_ms, reason). No difference found. The `module` value {base, size} I did not compare with ModuleSeen's type (launch_core.ts:15 shows `module: string` in the Patch type, separate from ModuleSeen).

F5. test_sp_tools snap_at (test_sp_tools.py:48) builds an all-zero snapshot with only clock_s set; every field the SP watch compares is zero. Real snapshots carry nonzero sp fields; tests that need them set them through **fields. FakeProc.verify_targets does nothing (line 87); the build-drift guard is never exercised in that file.

## G. Items 9 (focus)
Searched the 14 files in my list for foreground, SendInput, ShowWindow, keybd_event, mouse_event, SetFocus, AppActivate. Only hit: interfaces.py:211 and :229 (a Protocol comment naming SendInput). menu_tools says nothing it does touches keys or the window; the code presses the kit only. machine_recorder opens the game with VM_READ and QUERY rights only (process.access_mask(read_only=True)). Recorder uses CREATE_NO_WINDOW (recorder_link.py:107).

## Open (needs a live run)
- B1: does a song restart under the recorder give a stale clock? Settle with a recorded run that reloads a song.
- B3: Python version and a one-step recording.
- A3: does the player stop on non-empty `refusals`? (read play_chart.py.)
- F1: which note count a real trace reports; compare a run's calls to notes.

## Not reached
Whole files unread: test_server.py (class list only), test_machine_recorder.py bodies, test_machine_report.py bodies, test_frida_attach.py bodies past the fakes, test_bot_run.py bodies outside 486-640, test_sp_tools.py bodies past line 150, test_trace_hits.py past line 80, test_menu_tools.py past line 200. docs/clone-hero page 07 not opened. Unread files are not assumed clean.
