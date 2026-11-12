# Player stall fixes: why the auto-player dropped 41 chords on The Who, and how to stop it (2026-11-12)

This plan answers the user's ruling on the LS-A2 live run of 2026-11-11: "-76 is completely unacceptable and is a MASSIVE regression from prior runs. there will be no more runs until you figure this out." It changes no Hydra screen and no game file. It changes the auto-player (the Python tool that presses a chart's notes into Clone Hero) and its press log. No game run happens until the user says yes to the last step.

## The plain version

The auto-player stopped for a moment, seventeen times, while the song kept playing: ten long stalls, six short ones and one inside a press. Each time it woke up, the notes it had slept through were already too late to press, so it skipped them. The game did not freeze. The player's code did not change the notes it meant to press. It was simply not running when it needed to be.

**What is proven.** The game's song clock moved as far as the wall clock in every stall, so the game ran on. A second thread in the same Python process (the Star Power watch, which reads the game's memory every 4.5 ms) kept its pace through 10 of the 11 long gaps, so Python as a whole did not freeze. A separate recorder sampled the player's thread every 20 ms: inside the eleven long gaps the thread sat in a Windows kernel wait (a wait inside the operating system, not in Python's own sleep) in 7 of 12 samples, against 40 of 5,876 samples the rest of the time. So the player's thread was stuck inside a call into Windows.

**What is not proven.** Which call it was stuck in. There are two suspects, both file writes on the player's path. The first is the press log: after every press, the player opens the log file, adds one row and closes it, on the same thread that has to press the next note. The second is the kit's own log: the reply to each press is written and flushed to `kit-<n>.jsonl` by Frida's thread (Frida is the tool that reads and drives the game) before the player is allowed to see the reply, so a slow write there holds up the press call itself (row P25). That fits the one stall inside a press, chord 1181. But no run has caught the thread inside either call. It is also not known why the recorder, a separate program, froze at the same moments in most of the long stalls. That points at something machine-wide (Windows' storage optimizer was cancelled just after the first five), and a machine-wide pause can stall any call, not only a file write.

**What the plan does.** It takes the press-log write off the player's thread (task PS2), with a test that forces the stall first and shows the fix cures it (PS1), and it takes the kit log's write off the press reply's path (PS5). It adds timing to every press row, so the next stall names the step it happened in instead of leaving a guess (PS3). It builds an offline harness (PS4) that runs the real player loop with no game, to catch the real blocking call on this machine before any live run (steps LS-P0 to LS-P2). Then one live re-run of The Who (LS-P3), only on the user's yes.

**Where this plan departs from its brief.** Four places, each from reading the run's files:

1. The brief counts 11 stalls. Ten of those are stalls. The eleventh (before chord 1426) is a planned wait: that chord's press had been moved 3.5 s later by the retime rule, and the player waited for it as told. It is question 4.
2. Six more chords (196, 430, 839, 1769, 2013 and 2079) were each dropped alone, by a short overshoot of 48 to 119 ms. They are the same kind of stall, only smaller, so the plan counts 16 stalls between presses, not 11.
3. Chord 1181 was dropped because the press call for chord 1180 took 212 ms (the median press call took 7.6 ms, and 9 sent press rows took over 38 ms). That stall was inside the press, not after it. Moving the press-log write cannot fix it. The kit log's write (PS5) is the suspect from the code, and PS3's timing is what will name the step next time.
4. The offline hunt uses Python's own stack dump (`faulthandler`, part of Python, which writes every thread's current line at once) as its first tool, and `cdb` (the Windows debugger) second. `cdb` takes time to attach, and most stalls are over in under a second.

## What was checked, and how

Every number below was read from the run's own files for this plan, with short read-only scripts, not taken from the handoff. The run folder is `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1`.

**The drops.** `press-1.csv` holds 2,150 rows, and 41 say `dropped` in the `send` column. A stall is measured from one row's `wall_s_at_reply` (the wall time the press call returned) to the next row's `wall_s_at_read` (the wall time of the clock read that sends the next press), minus the wait the chart asked for (the next row's `chart_ms` minus this row's `clock_ms_at_press`).

| Stall before chord | Wall gap | Chart asked for | Over | Chords dropped |
|---|---|---|---|---|
| 128 | 1,103 ms | 222 ms | 881 ms | 128 to 131 |
| 150 | 828 ms | 225 ms | 603 ms | 150 to 152 |
| 165 | 530 ms | 222 ms | 309 ms | 165, 166 |
| 215 | 424 ms | 226 ms | 197 ms | 215 |
| 1453 | 484 ms | 226 ms | 258 ms | 1453 |
| 1900 | 959 ms | 112 ms | 848 ms | 1900 to 1907 |
| 1922 | 934 ms | 111 ms | 822 ms | 1922 to 1926 |
| 1941 | 740 ms | 110 ms | 629 ms | 1941 to 1944 |
| 1991 | 575 ms | 221 ms | 355 ms | 1991, 1992 |
| 2103 | 2,045 ms | 883 ms | 1,162 ms | 2103 to 2105 |

Those ten stalls dropped 33 chords. Six short overshoots dropped one chord each: 119 ms before chord 196, 95 ms before 430, 48 ms before 839, 94 ms before 1769, 95 ms before 2013 and 92 ms before 2079. Chord 1181 is the press-call stall (departure 3). Chord 1426 is the retime (question 4). That is 33 + 6 + 1 + 1 = 41.

**Why a late read drops a chord.** The loop in `play_chords` (`tools/ch_probe/experiments/play_chart.py`) reads the song clock, and the line `dropped = sends and raw_ms > chord.last_send_ms` skips any press whose latest send time has passed. That rule is right: a press sent that late would be judged a miss and could break the combo. The fault is the stall, not the rule.

**The game ran on.** In every stall the song clock (`clock_ms_at_press`) moved within 9 ms of the wall gap. The Star Power watch's log, `sp_watch-1.csv`, has a row every 4.5 ms; its longest gap inside nine of the ten stalls is under 45 ms (44.1 ms at most), and it has one 463 ms gap inside the stall before chord 165.

**The player's thread.** The recorder's `machine-2\threads.csv` samples each thread's state and wait reason. For thread 29080 of process 28784, between a press reply and the next clock read, 40 of 5,876 samples outside the long gaps are "waiting, reason 0" (Executive, a wait inside the kernel) and 7 of 12 samples inside them are. The eleven long gaps counted here are the ten stalls and the planned wait before chord 1426. The recorder's own samples have a gap over 100 ms at 8 of the eleven. Reason 6 (UserRequest) is what Python's sleep shows as. That thread 29080 is the player's thread is not written in any file (row P9, unchecked); it is the one thread of that process whose samples follow the press pattern.

**The suspect.** `_play_body` in `tools/ch_mcp/game_tools.py` gives `play_chords` an `on_press` that calls `play_chart.append_press_row` for every row. That function opens the file, writes one row and closes it. Between a reply and the next clock read, the thread also checks the reply, reads the Star Power end and the score from the game's memory, sleeps 5 ms (`SCORE_READ_DELAY_S`), and waits in `live.wait_until`. The watch thread reads the same memory and kept running, and it keeps its file open (`sp_tools.py`, the `open` and `f.flush()` in its body). The recorder keeps its files open too.

The press call is the other file write. In `LiveAgent._on_message` (`tools/ch_mcp/frida_attach.py`), Frida's thread writes and flushes each message to the kit log while holding the agent's lock, and only then hands the reply to the waiting press. `kit-1.jsonl` of this run holds 2,097 press replies, which is the 2,150 press rows less the 41 dropped and the 12 later groups of split chords. So every press that was sent waited on one write.

**The machine.** Windows' Application log has one storage optimizer event in the run: at 22:40:15 it reports the defragmentation of C: cancelled. The first press was at 22:39:13 and the first five stalls fall before 22:40:15. The log holds no optimizer event from 2026-11-07 to 2026-11-09, 5 on 11-10, 4 on 11-11 and 2 on 11-12. This is a match in time only. The later stalls have no optimizer event.

**The earlier run.** `2026-11-08\ls-m2-run1\press-2.csv` has 2,150 rows of the same chart and none dropped (its `hit` column holds only `1` and `declined`). That run has no recorder files, so it cannot say whether the machine was quieter. The player wrote its log row by row on that day too, so the write alone is not what changed between the two days (row P14).

**The code since.** `git diff 12b028d1..60e031c0` is empty for `tools/ch_probe` and `tools/ch_mcp`, so main still holds the code the run used.

## Rulings that bind this plan

- "there will be no more runs until you figure this out" (the user, on LS-A2; quoted in this plan's brief). No step below starts the game before the user's yes to LS-P3.
- No elevated window, so no `wpr` trace (`docs/agents/this-machine.md`). `cdb` attaches without one.
- Runs must not need focus, and no library is scanned. The offline steps use no game and read only this repo's own files as load.
- A new number needs the user's decision. Questions 2 and 5 hold this plan's new numbers.

## Questions

Each is for the user. Nothing that depends on one starts before its answer.

**Answers (D335 in `docs/audit/2026-10-03-fix-decisions.md`).** Question 1: "File and status", so the seven columns show in the file and in the `ch_play_status` rows. Question 2: "Yes, those numbers (Recommended)". Question 3: open. Question 4: "Trace it first", so a read-only trace of why `aim_end_ms` is early comes before any rule is chosen. The trace (row P20) found the schedule right: `aim_end_ms` is the end the game should store when it is read after chord 1363, and the game read 3.5 s more only because activation 4 never fired, its bar short after the stalls cost phrases. So question 4's line that a stall-free run would drop chord 1426 too is wrong: with no stall, the read should match and chord 1426 is pressed on time. Chord 2074 is the same kind of case in the other direction (read 3,527.19 ms earlier than its `aim_end_ms`). The rule for a press retimed past its own chord's window still matters whenever the game's meter strays from the plan, and it waits for the user. Question 5, with the go-ahead for PS1, PS2 and PS5: "Start them, 5 s cap (Recommended)".

1. **Seven new columns in the press log.** PS3 adds a time, in ms, for each step of a press: `wait_read_max_ms`, `wait_oversleep_max_ms`, `press_took_ms`, `reply_check_took_ms`, `end_read_took_ms`, `score_read_took_ms` and `prev_handoff_took_ms`. The user did not ask for new columns, so this is a question (rule 12). What you would see: `press-<n>.csv` today starts `index,chart_ms,clock_ms_at_press,score_before,score_after,send,down_frame,up_frame,down_ms,up_ms,wall_s_at_read,wall_s_at_reply,scored_between,end_read_ms,probe,retimed` (the header of LS-A2's `press-1.csv`); after, the same line ends with the seven new names, and each row ends with seven numbers. `ch_play_status` prints the rows under a line that lists the columns the rows carry (`game_tools.py`, the "rows, newest first" line); it would list seven more names and each row would be seven numbers longer. Real values exist only after the first run with the change. Recommended: yes to the columns in the file, and keep them out of the `ch_play_status` rows, which are already long. The other choice shows them in both.
2. **The offline hunt and its numbers.** LS-P2 runs the real player loop with no game for a fixed time, under a file-reading load, and dumps the stack when one step runs long. It needs three decisions. The threshold that counts as a stall: recommended 40 ms, because the narrowest press room in this run's schedule is 46.1 ms (chords 53, 71, 99 and 110 and others; `room_ms` of the schedule's `presses`; their `side_ms` is 48.1 ms), so a step that runs 40 ms long can already cost a chord, while the median press call took 7.6 ms and the 99th percentile 18.2 ms. The cap: recommended 10 minutes for each run, up to three runs (LS-P2 says which), 30 minutes at most. The load: a second process that reads the files under this checkout's `testdata` and `.git` folders in a loop; never the song library. It also needs your yes to run at all, because it loads the machine for up to 30 minutes.
3. **Should `ch_start` refuse while the storage optimizer runs?** Today the quiet check (`quiet_report` in `tools/ch_mcp/quiet.py`) refuses for a running compiler, CPU over 15 percent or a recent build folder, and only reports Spotify and Defender. The optimizer ran during the first five stalls, but nothing proves it caused them. What you would see on a yes: `ch_start` could answer "refused: the machine is not quiet: ...; storage optimizer: running; ..." and you would wait or stop the optimizer. On a no, the check stays as it is. How to tell that the optimizer is running is itself unchecked (row P16), so a yes adds a scout and a task; it is not in this plan. Recommended: report it on the quiet check's lines without refusing, until LS-P2 says whether it matters.
4. **Chord 1426: a retimed press pushed past its own chord's window.** The rule, in `retime_from_end`: `delta_ms = read_end_ms - p.aim_end_ms`, then the press moves by `delta_ms`. Then `play_chords` applies `dropped = sends and raw_ms > chord.last_send_ms` to the moved press. In this run the schedule planned the press of chord 1426 15 ms before the chord's own time, 325,091.5 ms, so at 325,076.5 ms, and gave it an `aim_end_ms` of 321,547.96 ms, the end the schedule expected the game to store after chord 1363. The game's end, read after chord 1363, was 325,094.43 ms, which is 3,546.47 ms later than that `aim_end_ms`. The press moved from 325,076.5 ms to 328,623.0 ms. The schedule disagrees with itself here: its own `paid_by` for chord 1426 says the activation that chord 1363 pays for ends at chord 1426 (offset 0), and chord 1363 sits 10,656.19 ms before that end, which puts the end at 314,435.31 + 10,656.19 = 325,091.50 ms, where the game's read end landed (row P27). Why the schedule's `aim_end_ms` is 3.5 s earlier than that is not traced (row P20, unchecked); an activation at chord 1218 that gave "no activation read" is one lead, not a finding. The chord's last send time is 325,174.22 ms, so it was dropped, and both its notes were missed. At the other end: a read 3 ms later than aimed moves the press 3 ms and it is sent; any read up to 97.7 ms later than aimed still sends it; anything beyond drops it. The choices: keep the rule (a press moved past its chord's window is dropped); or press such a chord at its own planned time, giving up the squeeze but keeping its notes; or have `hydra_replay schedule` write a fallback for it, as it does for squeeze-outs. No fix is folded into this plan. Recommended: decide before LS-P3, not after. If the read end matched the activation's true end, the retime moves a correct press away from it, and a stall-free run would drop chord 1426 too, so LS-P3 would fail its own pass line for a reason that is not a stall. No fix is in this plan, so LS-P3's go request carries your answer and, if it is "keep the rule", says chord 1426 is expected to drop.
5. **How long the run's end waits for a log writer.** PS2's press-log writer and PS5's kit-log writer are each stopped at the end (the run's end, and the agent's detach), and each wait needs a cap. Recommended 5 s, one number for both. If it passes, `ch_play_status` says the press log may lack its last rows (the rows stay in memory for the status reply), and the detach line says the kit log may lack its last lines.

## Tasks

PS1 and PS2 are one branch and one executor: PS1's test is committed first and red, PS2 turns it green. PS3 is cut from that branch's tip, because both change `play_chart.py` and its tests. PS4 is cut from PS3's tip. PS5 touches files no other task touches, so it is cut from main and runs beside them. All are Python; executors run on Opus and may run the named Python tests themselves, since none needs a build. Each task's branch gets one fresh reviewer.

### PS1: the forced repro, a log write that blocks 900 ms (Python; Opus)

Owns: `tools/ch_mcp/tests/test_game_tools.py` (a new class `BlockedPressLogTest`), and `tools/ch_probe/tests/fakes.py` if the fake clock has to move there to be shared.

The stall shows up only some of the time, so the test forces it. It runs the real `_play_body` with the real `play_chords` on a fake song clock and a fake press. It swaps `play_chart._press_log_writer` (the one function every press-log write goes through) for a fake whose `writerow` moves the fake clock forward 900 ms, the size of the run's first stall, but only when it is called on the thread that runs the player loop. That is exactly what a blocked write does: it costs the player time only if the player is the one writing.

Acceptance:
- `BlockedPressLogTest.test_a_log_write_that_blocks_900_ms_drops_no_chord`: the fake blocks on the first row; the run must end `done` with `result.dropped == 0` and every chord's press sent. Run with `py -m pytest -q -p no:cacheprovider tools/ch_mcp/tests/test_game_tools.py -k BlockedPressLogTest`.
- Red first: on the PS1 commit alone the test fails with at least one dropped chord, and the commit's body quotes the failing line. If it passes on today's code, the executor stops and reports, because then the test does not reproduce the stall.
- `BlockedPressLogTest.test_the_same_block_on_the_player_thread_still_drops`: the same fake wired straight into `on_press` on the player's thread drops chords. This keeps the first test honest: it proves the fake can cause a drop, so a green result means the write moved, not that the fake went soft.

### PS2: every press-log write moves to a writer thread (Python; Opus; same branch as PS1)

Owns: `tools/ch_probe/experiments/play_chart.py` (a new `PressLogWriter` beside `write_press_log`; `append_press_row` removed), `tools/ch_mcp/game_tools.py` (`_play_body` and the `_PlaySession` fields it needs), `tools/ch_probe/tests/test_play_chart.py` (the `append_press_row` tests become the writer's), `tools/ch_mcp/tests/test_game_tools.py` (the press-log tests). Uses rows P5, P6 and P7.

Who owns what, after reading both files: `play_chart` owns the press log's format and already holds the two functions that write it, so the writer lives there. `game_tools` owns the run's session, so `_play_body` starts the writer before the play, hands it each row, and stops it after.

The writer opens the log once, writes the header, and then takes rows from a queue on its own thread. It writes each row and flushes it (pushes it out of Python's buffer to Windows) before taking the next. That keeps the property `append_press_row`'s docstring promises today: a worker that dies keeps every row written so far. On the player's thread, `on_press` only appends the row to the session's list and puts it on the queue; it makes no file call. A write that fails is remembered by the writer; the log stops there, the run goes on, and the end of the run writes every row in one go, as today. At the end `_play_body` stops the writer and waits for it, up to question 5's cap.

Acceptance:
- PS1's `test_a_log_write_that_blocks_900_ms_drops_no_chord` passes, unchanged.
- `test_no_file_call_runs_on_the_player_thread`: every call of `_press_log_writer` and of the file's `write` and `flush` is recorded with its thread; none is the thread that ran `play_chords`. Fails if any write stays in `on_press`.
- `test_the_writer_builds_its_rows_through_press_log_writer`: the writer's rows match what `write_press_log` writes for the same rows, header first. Fails if the writer grows its own format.
- `test_every_row_is_on_disk_before_the_writer_closes`: after three rows are taken, a second handle reads the header and all three rows while the writer is still open. Fails if the flush per row is dropped, which is the "worker that dies" promise.
- `test_the_rows_reach_the_file_in_order_and_only_once`: 200 rows in, the file holds them in order, once each.
- `test_a_failed_write_goes_on_and_the_end_writes_every_row_once`: the existing failed-append test, with the failure raised inside the writer.
- `test_a_run_reusing_a_log_name_starts_the_file_over`: the existing test still passes.
- `test_the_end_waits_for_the_writer_and_says_so_when_the_cap_passes`: a writer held past the cap makes the status line say the log may lack rows; a normal end holds every row. The cap is question 5's number, named once, as `LOG_WRITER_CAP_S` in `tools/ch_probe/constants.py`, which the press-log writer and the kit-log writer both read.
- `git grep append_press_row` finds nothing outside this plan and the session cards; the reviewer runs it.

### PS3: each press row says how long each step took (Python and docs; Opus; cut from PS2's tip; after question 1's yes)

Owns: `tools/ch_probe/experiments/overlay_logs.py` (the one home of the press-log columns), `tools/ch_probe/experiments/play_chart.py` (`play_chords`), `tools/ch_probe/experiments/score_compare.py` (the list of older headers it still loads), `tools/ch_mcp/game_tools.py` (the status rows, if question 1 keeps the columns out of them), their four test files, and the press-log sentence in step 5 of `docs/agents/clone-hero-mcp.md`. Uses rows P4 and P8.

`play_chords` times each step with the timer it is already given (`now`), so the tests' fake time works. The wait loop lives in `live.wait_until`, which takes its clock and its sleep as arguments; `play_chords` wraps both, so `live.py` does not change. The seven values, in ms, go on the first row of each message, the way `scored_between` does:

`wait_read_max_ms` is the longest single clock read during the wait for this press. `wait_oversleep_max_ms` is the longest a sleep ran past what was asked. `press_took_ms` is the press call. `reply_check_took_ms` is the kit reply check. `end_read_took_ms` is the Star Power end read with its retime, blank when the row had none. `score_read_took_ms` is the two score reads added together: the one before the press (`score_before`, read between the clock read and the press, so it also delays the press) and the one after the 5 ms delay (`score_after`). The 5 ms sleep itself is not counted. `prev_handoff_took_ms` is how long `on_press` took for the message before; a row cannot hold its own hand-off, because it has already been handed over.

Acceptance:
- `test_a_stall_in_each_step_shows_in_that_steps_column`: for each of the seven steps in turn, a fake that takes 300 ms of fake time in that step alone gives a row whose matching column is at least 300 and whose other six are under 20. Fails if a step is untimed or timed under the wrong name.
- `test_a_dropped_row_carries_its_wait_times`: a dropped row holds the two wait columns and `prev_handoff_took_ms`, and leaves `press_took_ms` blank.
- `test_only_the_first_row_of_a_split_chord_carries_the_times`.
- `test_press_log_columns` in `test_overlay_logs.py` pins the new header as a literal, and `test_the_press_columns_before_the_timing_columns` pins today's header under its own name.
- `test_a_log_with_the_header_before_timing_still_loads`: `score_compare`'s loader accepts LS-A2's header, copied as a literal from `press-1.csv`.
- `test_the_status_rows_leave_the_timing_columns_out` (or, if question 1 says both, `test_the_status_rows_show_the_timing_columns`).
- The docs sentence names `overlay_logs.py` as the home of the columns and does not list them.

### PS4: the offline stall harness (Python; Opus; cut from PS3's tip; after question 2's yes)

Owns: `tools/ch_probe/experiments/stall_harness.py` (new), `tools/ch_probe/tests/test_stall_harness.py` (new), and a paragraph in `docs/agents/this-machine.md` on how to run it. Uses rows P10 and P11.

The harness runs the real `play_chords` on a real schedule file with no game. Its song clock is the wall clock. Its press goes through the checkout's real `LiveAgent` (`tools/ch_mcp/frida_attach.py`) with a fake Frida script that answers each post after 7.6 ms (the median kit press call in LS-A2, row P10), on another thread by calling `_on_message`, as Frida does, so the kit log's write is in the loop; it writes `kit-<n>.jsonl` into the harness's own folder. Its row sink is one of three: `writer` (PS2's `PressLogWriter`), `per-row` (the harness's own `per_row_append`: open, append one row, close, on the player's thread, the shape main had before PS2 removed `append_press_row`) or `none`. A watchdog thread watches which step the player is in. When a step runs past the threshold, it writes every thread's Python stack with `faulthandler` to a `stall-<n>.txt`, with the step's name and how long it has run. For the first stall of a run only, it then starts `cdb -pv` on its own process and saves that output too, with a line saying whether the stall was still on when `cdb` attached. `cdb` pauses the process while attached, so one attach per run keeps the rest of the run's timing clean. `--force-stall-ms` makes the sink block once, so the watchdog itself can be tested. `--minutes` caps the run, and `--threshold-ms` and `--hunt-run` are its other numbers. `--load` starts the file-reading process of question 2 and stops it at the end. `--no-cdb` skips the `cdb` attach, so the Python dump can be tried alone. The harness runs by its file path (`py -I tools/ch_probe/experiments/stall_harness.py`), because `-m` cannot find `tools` under `-I`; its docstring says how to run it. The step names the watchdog reports are the ones PS3's columns use, with the kit log's write falling inside `press`.

Acceptance:
- `test_a_forced_stall_writes_a_dump_that_names_the_sink`: with `--force-stall-ms` and the `per-row` sink, a `stall-1.txt` exists and its text holds the sink function's name and the step name `handoff`.
- `test_the_writer_sink_drops_nothing_under_a_forced_stall`: the same forced block with the `writer` sink ends with 0 dropped chords and no stall file for the player's thread.
- `test_the_cap_ends_the_run_and_says_so`: a run past `--minutes` stops, prints "cap reached", and exits 0.
- `test_a_clean_run_says_no_stall`: the summary line reads "no stall over N ms in M minutes" with the two numbers it was given.
- `test_a_missing_cdb_is_said_and_the_run_goes_on`: with no `cdb` found, the stall file says so and the Python dump is still written. The `cdb` path comes from the lookup in `docs/agents/this-machine.md`, never a typed path.
- `test_a_second_stall_in_one_run_starts_no_second_cdb`: two stalls reach the watchdog, with a fake `cdb` launcher; the launcher is called once and both Python dumps are written. This is the "one attach per run" promise.
- `test_a_blocked_agent_log_write_does_not_stall_the_press`: PS5 has merged, so the kit log is written on `LiveAgent`'s writer thread; the fake Frida script's log file blocks once, and the run has no stall file and no dropped chord.
- `test_the_load_process_is_stopped_at_the_end`, on every way out, the cap included.
- `test_the_defaults_are_the_users_numbers`: the threshold, the run's length and the hunt's run count default to `STALL_THRESHOLD_MS`, `STALL_RUN_MINUTES` and `STALL_HUNT_RUNS_MAX` in `tools/ch_probe/constants.py`, the user's numbers from D335 item 3 (40 ms, 10 minutes, three runs), and `test_a_hunt_run_past_the_most_is_refused`: `--hunt-run` above 3 exits with an error. The numbers have one home, `constants.py`.

### PS5: the kit log's write leaves the press reply's path (Python; Opus; cut from main)

Owns: `tools/ch_mcp/frida_attach.py` (`LiveAgent._on_message` and `detach`) and `tools/ch_mcp/tests/test_frida_attach.py`. Uses rows P25 and P26.

Today `_on_message` writes and flushes each message to the agent's log with the agent's lock held, and only then hands the reply to the waiting request. The fix: `_on_message` hands the reply over first and puts the message on a queue. One writer thread writes each line and flushes it, in order, so the file keeps its one-JSON-line-per-message form (run reviews read `kit-<n>.jsonl` as a source) and a crash keeps every line written so far. `detach` stops the writer and waits for it up to question 5's cap (`LOG_WRITER_CAP_S` in `tools/ch_probe/constants.py`), then closes the file. A message that arrives after the log is closed is still dropped, as today. A write that fails stops the log, not the reply: the agent's `out` callback says so, and the requests go on.

Acceptance:
- `test_a_blocked_log_write_does_not_hold_up_the_reply`: the log file's `write` blocks on an event; a request is answered while it is still blocked. Fails if the write runs before the reply is handed over.
- `test_every_message_reaches_the_log_in_order_once`: 200 messages in, the file holds 200 lines in order, once each, in the same form `_on_message` writes today.
- `test_every_line_is_on_disk_before_detach`: after three messages, a second handle reads three lines while the agent is still attached.
- `test_detach_waits_for_the_writer_and_says_so_when_the_cap_passes`: a writer held past the cap makes the detach text say the log may lack its last lines; a normal detach holds every line. The cap is question 5's number, defined once in a module both `play_chart` and `frida_attach` import; the reviewer checks it appears once.
- `test_a_failed_log_write_stops_the_log_and_not_the_reply`.
- `test_a_message_after_the_log_closed_is_dropped`: pins today's rule.

## Test limits

The harness's fake Frida script answers each kit press call after 7.6 ms, the median kit press call of LS-A2 (row P10): a stand-in for the real call, not a rule the player follows.

## The live steps

The main session runs these, in order, each after the task it names has merged. LS-P0 to LS-P2 start no game. Each is capped as its text says. No step scans the song library, and nobody focuses a window.

### LS-P0: the harness's first run, with a forced stall (no game; after PS4)

Calls: `py -I tools/ch_probe/experiments/stall_harness.py <The Who schedule> --sink per-row --load --force-stall-ms 900 --minutes 1 --threshold-ms <question 2> --no-cdb` (never tried)

The one new piece is the harness on this machine, with its load process. LS-P0 to LS-P2 run on PS4's branch tip, which holds PS1 to PS3 and PS5, so the kit log is written on `LiveAgent`'s writer thread, off the reply path. Pass: one `stall-1.txt` whose Python stack shows the player's thread inside the sink (`per_row_append`), a press log with dropped chords after the forced stall, and the load process gone when the run ends. Fail: no stall file, or the run does not end at its 1 minute cap; the output goes to the user. Cap: 2 runs.

### LS-P0b: the harness with the writer sink (no game; after LS-P0 passes)

Calls: `py -I tools/ch_probe/experiments/stall_harness.py <The Who schedule> --sink writer --load --force-stall-ms 900 --minutes 1 --threshold-ms <question 2> --no-cdb` (never tried)

The one new piece is PS2's writer on this machine. Pass: the run drops no chord and writes no stall file for the player's thread, and its press log holds every row. Fail: any dropped chord; the output goes to the user. Cap: 2 runs.

### LS-P1: one `cdb` attach to the harness (no game; after LS-P0b passes)

Calls: `py -I tools/ch_probe/experiments/stall_harness.py <The Who schedule> --sink per-row --force-stall-ms 2000 --minutes 1 --threshold-ms <question 2>` (seen: LS-P0's passing run), `cdb -pv -p <the harness's pid> -c "~*kc 40; qd"` started by the harness (never tried)

The one new piece is `cdb` on a Python process; it has been seen only on Hydra's own (`docs/agents/this-machine.md`). Uses row P11 (unchecked). The forced stall is 2,000 ms, the longest gap of the real run, to give the attach its best chance. Pass: the saved output holds a stack for every thread, the stall file says how long the attach took, and whether the stall was still on. Fail: `cdb` is refused or prints no stacks; then LS-P2 runs with the Python dump alone, and the report says so. Cap: 2 runs.

### LS-P2: the hunt for the real blocking call (no game; after LS-P1; needs question 2's yes)

Calls: `py -I tools/ch_probe/experiments/stall_harness.py <The Who schedule> --sink per-row --load --minutes <question 2> --threshold-ms <question 2> --hunt-run 1` (seen: LS-P0's passing run), the same with `--sink writer` and `--hunt-run 2` (seen: LS-P0b's passing run), `cdb -pv` started by the harness (seen: LS-P1's passing run)

Uses rows P12 and P13 (unchecked): this step is what checks them. The first run writes the log the way main does today; the second uses PS2's writer, under the same load for the same time; both on PS4's branch tip, which holds PS5, so the kit log is off the reply path in both. There are four ways it can end, and each has its report:

- The `per-row` run stalls in `handoff` and the `writer` run drops nothing: the dump's line (open, write or close) is the blocking call, and the fix is shown to hold. Go to LS-P3's go request.
- Neither run stalls within the cap: the report says "not reproduced in N minutes". The user decides whether LS-P3 runs on PS1 to PS5 alone.
- A run stalls in another step (the sleep, the clock read): the cause is wider than the file writes. Stop. The stall files go to the user before any live run, and this plan gains a task for what they show.
- A run fails to start or to stop at its cap: stop and report the output.

Cap: one run of each sink, and 2 tries if a run fails to start.

### Results of LS-P0 to LS-P2 (2026-11-12, main at 9c00e8d3)

Every run folder is under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\`.

LS-P0 (`ls-p0`) caught its forced stall: `stall-1.txt` shows the player's thread inside `per_row_append`, and the row after it has `prev_handoff_took_ms` 900.4. It dropped no chord, so that part of its pass line was wrong: the forced block lands right after the first chord, and the next chord is 2 s later. The load process was gone at the end. The same run also caught one real hand-off of 159.9 ms before chord 257, whose stack was dumped only after it ended.

LS-P0b (`ls-p0b`) passed: with the writer sink the forced 900 ms block cost the player nothing, with no stall file, no dropped chord, and all 292 rows in the log.

LS-P1 (`ls-p1`) failed. `cdb -pv` attached at the first stall and never quit; it was stopped by hand after more than 6 minutes. Every one of the harness's 12 threads stayed suspended after `cdb` was killed, so the harness's own cap could not end the run, and it was stopped too. So, as LS-P1's fail line says, LS-P2 ran with the Python dump alone (`--no-cdb`). The harness's `cdb` attach is not usable until that hang is fixed.

LS-P2 run 1 (`ls-p2-run1`, the `per-row` sink under load) dropped 6 chords (1447, 1451 to 1454, 2007), each right after a long hand-off; 11 hand-offs ran over 40 ms, the longest 427.5 ms. Eight of the 11 stall dumps were taken while the stall was still on, and each shows the player's thread on the `open(path, "a")` line of `per_row_append`, the line that both opens and closes the file. So on this machine the blocking call is the press log's open or close. The same run also had one press call of 763.5 ms and two oversleeps over 50 ms, inside a 25 s burst of stalls, so part of the stalling is machine-wide and not only file calls.

LS-P2 run 2 (`ls-p2-run2`, the `writer` sink under the same load) dropped no chord, and no step passed 40 ms: the longest hand-off was 0.4 ms, the longest press call 9.0 ms and the longest oversleep 1.8 ms. Run 2 saw no machine-wide burst like run 1's, so the two runs are not a perfect pair; what run 2 shows is that the writer took the slow file call off the player's thread.

### LS-P3: The Who again on the kit route (the game; only on the user's yes)

Calls: `ch_start(<The Who folder>, human=True, player="ProDrums,Expert")` (seen: LS-A2 run 1), `ch_run_chart(<folder>, <score>, kit_player="System", schedule_json=<schedule>, path_index=0)` (seen: LS-A2 run 1), `ch_play_status` (seen: LS-A2 run 1), `ch_wait_stats` (seen: LS-A2 run 1), `ch_compare` (seen: LS-M2 run 1), `ch_stop` (seen: LS-A2 run 1)

This is LS-A2 of `docs/superpowers/plans/2026-11-08-player-audit-fixes.md` again, on the same score and schedule files in `2026-11-11\ls-a2-prep`, on a tip that holds PS1 to PS5. The handoff says `ch_compare` ran in LS-A2 run 1, but that run's folder holds no file from it; LS-M2 run 1's `replies.txt` holds its output on the same chart (row P15), so that is the run this plan cites. The audit plan's run rules hold: hold other sessions, check the game dll's MD5 before and after, `ch_reload` after the merges, copy the stats and the watch out before `ch_stop`.

Pass: `press-<n>.csv` has no `dropped` row, every first row of a message carries its timing columns, and `ch_compare` shows `notes_hit` no lower than the 3,753 of LS-M2 run 1 (the `stats:` line of its `replies.txt`; the schedule played here is the LS-A2 one, so the declines can differ). Fail: any dropped row. The report then gives, for each one, the step its timing columns name and the recorder's samples around it, and no further run is asked for in the same message. Cap: 1 run. A second run is a new request to the user.

### Result of LS-P3 (2026-11-12, main at 95dfa6eb)

The run folder is `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-12\ls-p3-run1`. The game's dll MD5 was 106E15E5BF799EA62F916E8CEAF38BC6 before and after. The run ended "done". The game hit 3,766 of 3,767 gems (LS-A2: 3,691; LS-M2 run 1: 3,753), earned 31 of 31 phrases, made 9 of 9 activations and scored 1,139,680 against Hydra's 1,143,035. Every end read came within 2.7 ms of plan, and chord 1426 was sent on time, as row P20 predicted.

It fails its pass line: one row is `dropped`, chord 468 (chart 130,064.92 ms). Its timing columns name the step: `wait_read_max_ms` is 87.3 ms, one read of the game's song clock in the wait, where the other reads in the run take 0.06 to 0.2 ms. The press was read at 130,128.67 ms, 63.7 ms past the chord, beyond its window. The recorder's samples around it (`machine-1`) show the machine idle (no ready threads, 20 ms steps throughout) and the game's clock moving on. In the worker process, the thread with the most CPU over the run (tid 19260, the player by its spin) sat in wait reason 37 (a wait on a lock) for that span, while the thread that wakes most often (tid 37216, the Star Power watch by its 4 ms polls) ran for about 80 ms. So this stall is a lock wait inside the worker, most likely Python's interpreter lock held by the watch thread, not a file call and not a machine-wide pause. Which work the watch did for 80 ms is not traced. Matching each tid to its thread is by behaviour only: 37216 is the one worker thread with switches in nearly every 20 ms sample (25,637 rows), which fits only the watch's 4 ms poll; the foreground watch polls every 0.1 s and the log writers once per row.

The follow-up read found a second stall of the same kind. Only two first rows in the whole press log have a step over 15 ms, and both are `wait_read_max_ms`: chord 21 at 69.3 ms (clock 42.6 s, sent in time) and chord 468 at 87.3 ms. The watch's own log, `sp_watch-1.csv`, has exactly two gaps over 60 ms during the song, 74 ms ending at clock 42.69 s and 94 ms ending at clock 130.04 s, the same two moments. At both, the recorder shows tid 37216 running and tid 19260 waiting in reason 37. So twice in the song both Python threads stopped together while one thread of the worker ran, and the separate recorder process went on reading the game's clock every 20 ms, so the game's memory stayed readable. Ruled out: a tool call (the MCP log puts the `ch_wait_stats` call at 15:55:26.261 UTC, 0.8 s after the first stall, and no call near the second), the machine (idle, no ready threads), the file writes (both writers are off the player's thread), and Python's garbage collector: the worker's own imports and loads with the watch's rows, timed offline, gave a full collection of 20 to 36 ms, and an offline replica of the real player loop beside the real `sp_watch.watch` (fed this run's rows, no game) ran 2.6 minutes with no collection over 1.1 ms and no stall. What the watch thread ran for 70 to 90 ms is not known: the recorder sees threads from outside, and nothing in the live worker recorded a Python stack.

## Out of scope (needs the user's yes to leave out)

- The LS-A4 Star Power block and its listings (one missed gem inside a phrase strips the phrase's award): still to be written up on `docs/clone-hero/05-star-power.md`.
- Chord 1893 of The Who, where the schedule plans a squeeze-out 7,119.91 ms after the end: a possible schedule bug, its own question.
- The branch `claude/pl-freak-t2`, which waits for the user's word to be deleted.
- Any fix for chord 1426's retime (question 4) and any change to the quiet check (question 3).
- Raising the player thread's priority, or any other guard against a machine-wide pause: nothing is proposed until LS-P2 shows one is needed.
- Stalls inside the press call itself that the kit log's write (PS5) does not explain (chord 1181 may or may not be one): PS3 measures them; no further fix is planned until one is caught with its timing.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| A log write that blocks 900 ms drops no chord | PS1, PS2 | PS2 | `test_a_log_write_that_blocks_900_ms_drops_no_chord`, red on PS1's commit, green on PS2's |
| The forced block can still cause a drop, so the green result is real | PS1 | PS1 | `test_the_same_block_on_the_player_thread_still_drops` |
| No file call runs on the player's thread | PS2 | PS2 | `test_no_file_call_runs_on_the_player_thread` |
| The writer keeps one format, the one `write_press_log` writes | PS2 | PS2 | `test_the_writer_builds_its_rows_through_press_log_writer` |
| A worker that dies keeps every row so far | PS2 | PS2 | `test_every_row_is_on_disk_before_the_writer_closes` |
| Rows reach the file in order, once each | PS2 | PS2 | `test_the_rows_reach_the_file_in_order_and_only_once` |
| A failed write stops the log, not the run, and the end writes every row once | PS2 | PS2 | `test_a_failed_write_goes_on_and_the_end_writes_every_row_once` |
| A reused log name starts over | PS2 | PS2 | `test_a_run_reusing_a_log_name_starts_the_file_over` |
| The end's wait for the writer is capped and says when the cap passed | PS2, question 5 | PS2 | `test_the_end_waits_for_the_writer_and_says_so_when_the_cap_passes` |
| `append_press_row` is gone | PS2 | PS2 | the reviewer's `git grep append_press_row` |
| A kit log write that blocks does not hold up the press reply | PS5 | PS5 | `test_a_blocked_log_write_does_not_hold_up_the_reply` |
| The kit log keeps one line per message, in order, flushed per line | PS5 | PS5 | `test_every_message_reaches_the_log_in_order_once`, `test_every_line_is_on_disk_before_detach` |
| The detach's wait for the kit-log writer is capped and says when it passed | PS5, question 5 | PS5 | `test_detach_waits_for_the_writer_and_says_so_when_the_cap_passes` |
| A failed kit-log write stops the log, not the reply | PS5 | PS5 | `test_a_failed_log_write_stops_the_log_and_not_the_reply` |
| A block in the kit log's write leaves the `press` step on time in the harness | PS4, PS5 | PS4 | `test_a_blocked_agent_log_write_does_not_stall_the_press` |
| A stall in any of the seven steps shows in that step's column | PS3 | PS3 | `test_a_stall_in_each_step_shows_in_that_steps_column` |
| A dropped row still carries its wait times | PS3 | PS3 | `test_a_dropped_row_carries_its_wait_times` |
| Only a message's first row carries the times | PS3 | PS3 | `test_only_the_first_row_of_a_split_chord_carries_the_times` |
| The columns have one home, and older logs still load | PS3 | PS3 | `test_press_log_columns`, `test_the_press_columns_before_the_timing_columns`, `test_a_log_with_the_header_before_timing_still_loads` |
| The status rows show the timing columns only if question 1 says so | PS3, question 1 | PS3 | `test_the_status_rows_leave_the_timing_columns_out` or its opposite |
| `live.py` does not change | PS3 | PS3 | the reviewer's `git diff` of the branch shows no change to `live.py` |
| The watchdog's dump names the stalled step and function | PS4 | PS4 | `test_a_forced_stall_writes_a_dump_that_names_the_sink` |
| The harness shows the writer cures a forced stall | PS4 | PS4 | `test_the_writer_sink_drops_nothing_under_a_forced_stall` |
| Every harness run is capped and says how it ended | PS4 | PS4 | `test_the_cap_ends_the_run_and_says_so`, `test_a_clean_run_says_no_stall` |
| A missing `cdb` is said, never silent | PS4 | PS4 | `test_a_missing_cdb_is_said_and_the_run_goes_on` |
| The load process never outlives the harness | PS4 | PS4 | `test_the_load_process_is_stopped_at_the_end` |
| The harness's threshold, run length and run count are the user's numbers, from one home | PS4, question 2 | PS4 | `test_the_defaults_are_the_users_numbers`, `test_a_hunt_run_past_the_most_is_refused` |
| `cdb` attaches once per run at most | PS4 | PS4 | `test_a_second_stall_in_one_run_starts_no_second_cdb` |
| The real blocking call is looked for offline before any live run | LS-P2 | LS-P2 | the two run folders and their summary lines, shown in LS-P3's go request |
| The harness, its load and the writer sink are each tried once on their own before the hunt | LS-P0, LS-P0b | LS-P0, LS-P0b | each step's pass line |
| No game run before the user's yes | Rulings, LS-P3 | LS-P3 | the go request; LS-P0 to LS-P2 name no `ch_` call |
| Chord 1426's expected drop is settled before LS-P3 | question 4, LS-P3 | LS-P3 | the go request carries the answer |
| The re-run drops no chord and every message's first row is timed | LS-P3 | LS-P3 | `press-<n>.csv` of the run: no `dropped`, no blank timing on a first row |
| The song library is never scanned as load | question 2, PS4 | PS4 | `test_the_load_process_is_stopped_at_the_end` also asserts the load's folders are the two this plan names |
| No fix for chord 1426 or the quiet check rides in | Out of scope | PS2, PS3 | the reviewer's `git diff` shows no change to `retime_from_end` or `quiet.py` (PS5 changes `frida_attach.py` only, and no other task touches it) |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: LS-A2 run 1's press log has 2,150 rows, 41 dropped, and the sixteen stalls with the sizes in "What was checked" | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1\press-1.csv` | checked 2026-11-12, every row read |
| P2: the watch's longest gap is 44.1 ms or less inside nine of the ten long stalls and 463 ms inside the one before chord 165 | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1\sp_watch-1.csv` | checked 2026-11-12 |
| P3: thread 29080 shows "waiting, reason 0" in 7 of 12 samples inside the eleven long gaps (the ten stalls and the wait before chord 1426) and 40 of 5,876 outside; the recorder has a gap over 100 ms at 8 of the eleven | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1\machine-2` (`threads.csv`, `samples.csv`) | checked 2026-11-12 |
| P4: a press read past `last_send_ms` is dropped, and the loop's steps between a reply and the next read are the reply check, the end read, a 5 ms sleep, the score read, `on_press` and `live.wait_until` | code | `tools/ch_probe/experiments/play_chart.py` (`play_chords`, the `dropped =` line) and `tools/ch_probe/experiments/live.py` (`wait_until`) | checked 2026-11-12 |
| P5: `on_press` in `_play_body` calls `append_press_row` on the play thread for every row, and a failed append stops the log while the run goes on | code | `tools/ch_mcp/game_tools.py` (`_play_body`) | checked 2026-11-12 |
| P6: `append_press_row` opens, writes one row and closes, and its docstring promises a dying worker keeps every row; every press-log write goes through `_press_log_writer` | code | `tools/ch_probe/experiments/play_chart.py` (`append_press_row`, `write_press_log`, `_press_log_writer`) | checked 2026-11-12 |
| P7: the watch thread keeps its file open and flushes | code | `tools/ch_mcp/sp_tools.py` (the `open` and `f.flush()` in the watch body) | checked 2026-11-12 |
| P8: the press-log columns have one home, `score_compare` loads the older headers by name, and `ch_play_status` lists the columns the rows carry | code | `tools/ch_probe/experiments/overlay_logs.py` (`PRESS_LOG_COLUMNS`), `tools/ch_probe/experiments/score_compare.py` (the accepted headers), `tools/ch_mcp/game_tools.py` (the "rows, newest first" line) | checked 2026-11-12 |
| P9: thread 29080 of process 28784 is the player's thread | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1\machine-2` names thread ids only; `recorder.log` names the recorder's own thread and no other | PS3: its columns name the stalled step without thread ids, and no task or step uses this row as settled |
| P10: a kit press call (`wall_s_at_reply` minus `wall_s_at_read` of a sent row) had a median of 7.6 ms in LS-A2, a 99th percentile of 18.2 ms and 9 rows over 38 ms, the longest 212.4 ms (before chord 1181); the calls before the six single drops took 7.2, 7.1, 26.9, 7.4, 10.7 and 8.3 ms | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1\press-1.csv` (`wall_s_at_reply` minus `wall_s_at_read`) | checked 2026-11-12 |
| P11: `cdb -pv` attaches from this shell and prints every thread's stack; seen on Hydra's process, not on Python's | code | `docs/agents/this-machine.md` ("Debugger: WinDbg and cdb") | checked 2026-11-12 for what the page says; the Python target is LS-P1's one never-tried call |
| P12: an open, append and close per row can block the calling thread for hundreds of ms on this machine | unchecked | the main session's offline trials of the per-row write that `PressLogWriter` in `tools/ch_probe/experiments/play_chart.py` replaced, kept only in scratch files; no run folder holds them | PS4, then LS-P2's `per-row` run |
| P13: a file write on another thread does not hold up the player's thread (Python lets other threads run during file calls) | unchecked | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1\sp_watch-1.csv` shows the watch running through the stalls, which fits but does not prove it | PS4, then LS-P2's `writer` run |
| P14: LS-M2 run 1 played the same chart with no dropped row and left no recorder files | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1` (`press-2.csv`, the folder's file list) | checked 2026-11-12 |
| P15: LS-A2 run 1 made the `ch_start`, `ch_run_chart`, `ch_wait_stats` and `ch_stop` calls LS-P3 repeats; `ch_compare` left no file there, but LS-M2 run 1's `replies.txt` holds a `ch_compare` result for The Who (score FAIL, notes 3,767 pass, 9 chords declined) and a `stats:` line with `notes_hit 3753 of 3767` | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1` (the launch log, `press-1.csv`, `kit-1.jsonl`, the scorestats copy) and `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1\replies.txt` | checked 2026-11-12; `ch_compare` is marked seen: LS-M2 run 1 |
| P16: how to tell from a process list that the storage optimizer is running | unchecked | `tools/ch_mcp/quiet.py` has no such read | question 3 |
| P17: the Application log has one optimizer event in the run (22:40:15, cancelled), none from 11-07 to 11-09, and 5, 4 and 2 on 11-10, 11-11 and 11-12 | unchecked | Windows' `Application` event log, provider `Microsoft-Windows-Defrag`, read with `Get-WinEvent` on 2026-11-12; an event log is none of the five source kinds, so the row stays unchecked | question 3 |
| P18: chord 1426's press is timed from the end read after chord 1363, aimed at an end of 321,547.96 ms, with last send 325,174.22 ms; the read end was 325,094.43 ms | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-prep\The Who - Won't Get Fooled Again (O)-schedule.json` (`presses`, chord 1426) and `press-1.csv` (row 1363's `end_read_ms`) | checked 2026-11-12 |
| P19: `retime_from_end` moves a press by the read end minus its `aim_end_ms` and has no check against the chord's last send | code | `tools/ch_probe/experiments/play_chart.py` (`retime_from_end`) | checked 2026-11-12 |
| P20: chord 1426's `aim_end_ms` (321,547.96 ms) is activation 5's first stored end, the end the game should hold when it is read after chord 1363, before the phrase on chord 1380 adds 3,840 ticks and makes the final end 325,091.50 ms; the game read 325,094.43 ms instead because activation 4 never fired at chord 1218 (the bar held 3,600 of 14,400, below the 7,200 half), so its Star Power was carried into activation 5 | run | the schedule's `activations[5].sp_end_steps` and the `reads` of chord 1363, `sp_watch-1.csv` (`sp_amount_u32` 3600 and `sp_half_u64` 7200 at clock 285.9 s) and `press-1.csv` row 1218 ("no activation read"); the schedule writer's `in_read_terms` in `tools/replay_schedule.cpp` | checked 2026-11-12 |
| P21: the player's code on main is the code LS-A2 ran | code | `git diff 12b028d1..60e031c0 -- tools/ch_probe tools/ch_mcp` (empty) | checked 2026-11-12 |
| P22: the quiet check refuses for a compiler, CPU over 15 percent or a recent build folder, and only reports Spotify and Defender | code | `tools/ch_mcp/quiet.py` (`quiet_report`, `QUIET_CPU_PCT`, `REPORTED_NAMES`) | checked 2026-11-12 |
| P23: no game run may start before the user's yes | ruling | the user's words "there will be no more runs until you figure this out", quoted by the main session in this plan's brief and reported in the LS-A2 handoff on branch `claude/handoff-a828b0ca` | checked 2026-11-12; the reviewer's brief must quote it too |
| P24: across all 2,150 `presses` of LS-A2's schedule the narrowest `room_ms` is 46.1 ms and the narrowest chord `side_ms` is 48.1 ms (chords 53, 71, 99, 110 and others); chord 196's `side_ms` is 66.2 ms | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-prep\The Who - Won't Get Fooled Again (O)-schedule.json` (`presses`, `chords`) | checked 2026-11-12 over every entry; question 2 settles the threshold |
| P25: `LiveAgent._on_message` writes and flushes each message to the log, holding the agent's lock, and only then sets the waiting request's event; `_request` takes the same lock before it returns | code | `tools/ch_mcp/frida_attach.py` (`_on_message`, `_request`) | checked 2026-11-12 |
| P26: `kit-1.jsonl` holds 2,099 lines, 2,097 of them press replies, which is the 2,150 press rows less 41 dropped and 12 later groups of split chords | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1\kit-1.jsonl` (lines holding `down_frame`) | checked 2026-11-12 |
| P27: the schedule's chord 1426 is at 325,091.50 ms with `paid_by` activation 5 at offset 0, chord 1363 is at 314,435.31 ms with offset -10,656.19, and the end read after chord 1363 was 325,094.43 ms; the schedule's `reads` for chord 1363 holds `stored_end_ms` 321,547.96 | run | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-prep\The Who - Won't Get Fooled Again (O)-schedule.json` (`chords` 1363 and 1426) and `press-1.csv` (`end_read_ms` of chord 1363) | checked 2026-11-12 |
