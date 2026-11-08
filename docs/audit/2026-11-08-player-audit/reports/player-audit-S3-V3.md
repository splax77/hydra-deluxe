# player-audit-S3-V3 (agent a43f55527e92a5233)

Read-only. Everything below is "by reading" unless a file is named. Nothing was run. Repo paths are in the worktree `...\hydra-test\.claude\worktrees\player-audit-plan` (same code as main fd93c684 for these files).
Calls used: about 50. Candidates I did not open are listed at the end; they are neither confirmed nor rejected.

## Priority group

### W2A-4 ch_run_chart cannot pick the dump's path -- PARTLY
Homes: game_tools.py:1026-1054 (ch_run_chart passes no path_index; chords_to_play defaults 0); game_tools.py:410-436 (chords_to_play -> play_chart.load_dump_chords); play_chart.py:747-808 (load_dump_chords) and :676-687 (_dump_path); tools/replay.cpp:485 and :651 (writes `path_source: {file, index}` into every score JSON made from a path file).
What holds: ch_run_chart has no path_index argument, so it always reads dump path 0.
What does not: load_dump_chords does refuse a path mismatch when the score file says which path made it. _dump_path (:683-686) raises ValueError("... was made from path k, but path_index is 0") when score `path_source.index` != path_index; chords_to_play turns it into a ChordsRefused reply. So a score made for path k plus its dump is refused by ch_run_chart, not played on path 0.
Gap that remains: a score file with no `path_source` (made with `--acts`, per the comment at :681-682) is accepted for any index, and test_play_chart.py:1050 pins that. :684 also does not compare `path_source.file` with the dump file name, so a score and a dump of different files only fail if their ticks fail to line up (declined_chord_indexes raises on a skipped-fill tick with no chord, :702-705; I did not read _squeezes for the squeeze case).
Split: ch_play_chart (server.py:362-387) and ch_sp_compare have path_index; ch_run_chart does not (a run of path k > 0 cannot be started through it at all, it can only be refused).
Open: none needed for the refusal; a `--acts` score + a path-k dump would settle the gap only by reading _squeezes.

### W2A-5 ch_open_song's first two steps prove nothing; no instrument/difficulty check; "done:" line -- CONFIRMED
Homes: tools/ch_mcp/menu_steps.json (read: 6 steps, "verified": false, no search step; step 1 expects profile_screen_active, step 2 main_menu_active, step 4 "pick the selected song" green, step 5 "ready" green expects game_manager_live; the `guess` text of step 4 says it picks "whatever song the list has selected, which may not be the target"); menu_tools.py:261-307 (_run_step: press, then _wait for the expect word), :280-288 (_done_line returns "done: a song is loaded, but its folder is X, not the target Y" as an ordinary reply); docs/clone-hero/07-internals.md:803-811 (IN-79, "Proven in a live run": the profile screen and main menu "both read active from the moment the game opens").
Because both words already hold before any press, step 1 and 2 cannot detect a lost press (by reading, plus IN-79's own text). No step expects an instrument or difficulty; the file says Ready is pressed "with the player screen's default instrument and difficulty". `verified: false` is read by no code (not re-grepped beyond menu_tools.py; the finder's claim and the comment at menu_tools.py:117).
Open: a live run (LS-K3 was struck) would show the real default instrument and difficulty.

### W2B-C1 same "done:" line with the wrong song -- CONFIRMED (same homes as W2A-5)
menu_tools.py:285-288. Caller check: ch_run_chart does not use ch_open_song and never looks at which song is loaded (see S1-C13).

### S1-C13 nothing checks the score JSON is of the song the game is playing -- CONFIRMED
Homes: game_tools.py:1026-1109 (`song_folder` is used for run_cap_s at :1055 and watch_for_run at :1075 only); sp_tools.py:212-232 (watch_for_run: `song_folder` only gives `_song_end_ms` for the gap line, docstring :220); play_chart.py:747-808 (load_dump_chords checks only path index); I grepped game_tools.py, sp_tools.py, sp_watch.py, play_chart.py, engine.py for currentSongFolder, folderPath, hyhash, md5, sha: no match in any of them (matches for song_folder only as above).
What a run does: it finds a moving engine clock (engine_finder) and presses the score file's chords by clock. Neither the chart's chords, the song folder nor a loaded-song read from the state agent is compared with the score JSON. A score JSON for song A played into song B runs and reports hits by chance. It will stop on "past_end"/cap. Nothing refuses.
Open: a live run with the wrong song would show the reply; not run.

### W2B-A1 three different "latest press still inside" limits -- CONFIRMED (latent: nothing reads the schedule)
Homes: replay_schedule.cpp:249-251 (press_room_ms: side - max(|aim|, |aim+6|), two frames late and early side judged at the unjudged aim); :329 (last_send_ms = chord + side - 3, one frame); :333-335 (sqout_fallback_after_ms: two frames); :339-341 (sqin_fallback_before_ms: one frame); :392-402 (far_press_refusal: |send - chord| <= side, zero frames). Constants :21-66 (kJudgeEarlyMs 3, kJudgeLateMs 6, kRoomMarginMs 3). .h :124-131 describes last_send_ms as "the latest song clock a press may still be sent at (TM-27: equal is inside)".
Input where they differ: side 50, chord 1000: last_send 1047; far_press accepts 1050; room-based (room >= 0) allows aim <= +44, with the 3 ms margin (:271) aim <= +41. A press sent at 1047 is judged at 1050..1053 under the 1-2 frame model (:36-40), and 1053 is outside 1050. So last_send_ms is the 1-frame limit while rooms use 2 frames.
Reference: docs/clone-hero/03-timing.md TM-27 (:313-321): a press sent 84.92 ms late on an 85.72 side missed, so the lag is more than 0.80 ms; "Not proven: the lag itself". So the 1-frame / 2-frame model is itself unproven.
Latent: grep of tools/**/*.py for last_send_ms, "refusals", if_end_after_ms, room_ms found no reader, so no run uses these numbers today.
Early side: press_room_ms uses |aim| for the early side and does not add the frame (aim -52, side 50 gives room -2 although a judge 1-2 frames later at -49..-46 is inside). Stricter than the model it states. By reading.

### W2B-A2 squeeze-out chord ignores the declined gem list -- CONFIRMED (latent)
Homes: replay_schedule.cpp:558-577 (`mults[i]` holds every gem's multiplier; `sent[i]` is the kept gems); :844-858 (squeeze-out inside groups from squeeze_out_inside_groups(ci, mults[i], ...) and the late gem as place 0, never from sent[i]); :881 and :815 (squeeze-in and plain use sent[i]).
Split: play_chart.py:785-788 (load_dump_chords raises "chord N is a fill the path passes over and also a squeeze or an activation") refuses this input; replay_schedule.cpp has no such refusal (grep of "also a squeeze", "passes over" there: only the lead refusal at :820), so for a declined chord that is also a squeeze-out chord the C++ schedule presses all gems. Whether a real path can have a skipped-fill tick on a squeeze tick I did not check; no test combines them (test grep).

### W2B-A3 / F2-S1b refusals drop chords from the press list -- CONFIRMED (latent)
Homes: replay_schedule.cpp:818-823 (lead chord that is also a squeeze or end: refusal + continue), :836 (`it == end || !split` continue, no sentence of its own), :841 (`!aim_end` continue), :850 (squeeze_out_inside_groups refused: continue, the sentence is at :382), :725 (`!valid(x)` continue, no sentence), :638 and :641 (squeeze whose chord/anchor is null or invalid: continue, no sentence), :651 (`!ratio` continue; the sentence is at :494-497). The squeeze rows get `feasible:false` (:691-693) in those cases. The press list then has no row for those chords and nothing in the file flags which.
Whether the Python player stops on non-empty `refusals`: no Python reads `refusals` (grep, tools/**/*.py), so today neither stops nor continues.
Gap I confirmed (F2-S1a): a squeeze whose `chord_tick` exists but lands on no chord: chords.at(...) gives null via or_null (:503, :507), the refusals at :489-490 fire only for a missing tick, and :638 skips it silently.
Declined chord with nothing to send: press_groups (:348-352) returns one group with empty gems; row has "gems": [] and skips the far-press check (:939 `!p.gems.empty()`). Confirmed.

### W2A-3 a reply cut by the 12,000-character cap loses the verdict line -- CONFIRMED (no measured long reply)
Homes: hydra_ui_mcp.py:45 (REPLY_CAP_CHARS = 12_000), :267-289 (cap_reply keeps the start, drops the end, with a note); server.py:46-51 (CAP_HINT is "ch_notes(since=...) reads a long trace in pieces", used for every tool); sp_compare.py:633 (the "Summary: N PASS, N FAIL" note is the last line of the report) and sp_tools.py:312 (ch_sp_compare returns report.text() whole). So a long ch_sp_compare loses the Summary line and some PASS/FAIL lines; the cut note tells the reader to use ch_notes(since=...), which does not apply to ch_sp_compare. sp_tools.py:251-254 puts the gap line first on purpose for this reason (docstring).
Open: how long a real report gets (not measured; no run file read). A 2000-chord chart yields one multiplier check line per run, not per chord, so I do not know it reaches the cap.

### W2A-6 launch reports success when "patched" -- CONFIRMED, with a correction
Homes: bot_run.py:701-716 (launch_human returns HumanLaunch as soon as the status says "patched"); bot_run.py:557-566 (loader_ran: "not yet" when loader_runs <= 0, "yes" when bytes_at_first_run equals the write, else "no"); :569-586 (launch_status_lines puts the loader line last); launch_core.ts:143-154 and :161-199 (loaderRan hook counts runs and reads the bytes at the first run; a read failure writes `reason` but state stays "patched").
Correction to the candidate: the loader line is not only in ch_status; launch_human prints it at launch (:714-715), but at that moment the loader has usually not run, so it reads "not yet". Nothing re-reads it later to fail the launch (grep loader_ran / LOADER_NO / LOADER_YES in tools/ch_mcp/*.py: only bot_run.py:557-586). A later "no" is visible only if someone asks ch_status. The 12-entry-byte and 5-patch-byte checks (launch_core.ts:169-182) refuse a differing build loudly. Open: a live launch.

## The rest of W2A

### W2A-1 built .js not compared with source -- CONFIRMED by reading
frida_tools.py:372-379 (state agent: `os.path.isfile` only); .gitignore of tools/ch_mcp (`agent/*.js`). No hash or stamp (I did not open bot_run.py:631 or trace_hits.py:300 myself; frida_tools.py only). The baked `menu_fields_default` vs menu_fields.json split I did not re-check.

### W2A-2 run cap message for a folder with no length -- CONFIRMED
game_tools.py:171-181 (run_cap_s catches SongLengthMissing itself and returns play_chart.RUN_CAP_S); sp_tools.py:197-202 (catches SongLengthMissing from the same call, which cannot be raised, and prints "run cap: N s (game_tools.run_cap_s for <folder>)"). Input: a folder with no song.ini length and no manifest: bot_run.py:387-403 raises SongLengthMissing, run_cap_s swallows it, the reply says the cap came from the folder. Also bot_run.py:401-403's message says "nothing was started", but the two cap callers continue. The fake in tests/test_sp_tools.py:78-80 I did not open.

### W2A-9 find_activations never sees an activation already on at row 0 -- CONFIRMED
sp_compare.py:230-245: `i = 1`, requires `not is_active(rows[i-1])`; row 0 is never an entry, and the numbering starts at the first detected one.

### W2A-10 sub-items
- server.py:105-115 reads private `kit._agent`, `sp._watch`, `method_watch._session`, `frida._trace` -- CONFIRMED by reading (rename gives AttributeError in ch_open_items).
- others not opened: sp_tools.py:431-440, :370, frida_tools.py:96, menu.ts, menuhooks.ts.

### W2A-7, W2A-8 -- NOT VERIFIED (only glimpsed at constants); W2A-8's sp_compare._ms / s_to_ms two homes of x1000 not opened.

## The rest of W2B

### W2B-A4 untested edges -- CONFIRMED
Grep of tests/test_replay_schedule.cpp: the rule strings "sp_end" (as a press rule) and "inside_moved_end" return no hit (the other "sp_end" hits are other words); the phrases "two squeezes", "also a squeeze", "no chord at skipped", "no SP-end steps", "names no chord", "has no end in the record", "SP-end step on chord", "no stored ratio", "cannot realize", "less than a plain press" each return 0 hits. "steady span" returns 1. So no test names any of these refusals by text. A test could still reach them without naming them; I did not trace every test.

### W2B-B3 machine_report crash -- NOT CONFIRMED for quantiles, PARTLY for division
`py --version` gives 3.14.5 and `python --version` 3.14.7. machine_report.py:487 `statistics.quantiles(steps, n=100, method="inclusive")`: Python 3.13+ no longer raises for one data point (per the 3.13 documentation I remember; I did not run it). On an older Python it would raise StatisticsError, not Refused. `_share` (:242-243) divides by span_ms; window_of (:218-229) sums wall-stamp gaps, and :541 requires two samples, so zero only if all stamps are equal. Not shown by any file.

### W2B-B4 two settings parsers -- PARTLY: machine_report.py:50-52 says framerate/vsync sit in [video] and game_files reads [game] only, so the difference is stated in a comment; the report not naming the section I did not check.

### W2B-D1 song length homes -- CONFIRMED as split
game_files.py:356-365 (read_song_ini: first line wins via setdefault) and its own docstring at game_files.py:~383 (set_song_title: "read_song_ini takes the first and Hydra's read_song_ini_keys the last"). bot_run.py:387-403 (song_length_ms uses game_files.song_ini_length_ms; the cap, the silent ogg and the install all use the first line). Input: a song.ini with two `song_length` lines: the run cap and the silent ogg use the first, Hydra the last. Did not check the game's real choice.

### W2B-D2 install_chart copies the whole folder -- CONFIRMED: install_chart.py:82 copytree, then write_ogg only overwrites song.ogg. D3 `--name` unchecked join at :78 -- CONFIRMED.

### W2B-E2 edge_probe KeyError -- PARTLY: edge_probe.py:124 `first[p.index].lead_ms` with `first = _first_presses(chords)`; I did not read _first_presses to see whether a chord with no press is missing from it.

### W2B-F1 trace_hits "within 85 ms" count -- CONFIRMED
tools/ch_mcp/trace_hits.py:151-166 (line_up pairs each call with its nearest note by min abs, any distance), :232-246 (count within a flat EXPECT_NORMAL_BACK_MS, "of {len(lined)}": the total is calls, not notes). No line gives notes with no call.
### W2B-F2 -- CONFIRMED by reading: trace_hits.py:157 min() over empty notes raises ValueError; :119 `notes[-1]` IndexError. Whether the manifest can be empty (the loader) I did not check.

## From F2, not-verified list

### F2-J6 sp_compare passes on its own consistency -- CONFIRMED
Homes: sp_compare.py:466-480 ("flag clears at the end" compares the flag drop with the game's own sp_end_s, 100 ms-style window EVENT_WINDOW_MS); :523-534 (the "on the path's chord" check is `report.check(True, text)` with the difference between game end and Hydra's deactivation only printed, never checked); :585-600 (multiplier check: `if fails: ... else check(True ... {matched} of {compared})`, so compared = 0 passes; accepts chords i-1, i, i+1); :358-385 (phrase rise search: any rise after `lo`, any size); :160-167 (a log without clock columns loads clock_s = NaN; `t < lo` and `t > hi` are both False for NaN, so the first unused rise passes). The ls-m2-run1 figure of 3,544 ms I did not re-read.
"No activations, no line": sp_compare.py:504 loops over max(len(game), len(path)); with both 0 it prints nothing about activations (header :615 still prints the count).
### F2-J8 clock at press, not the judging time -- CONFIRMED by reading: play_chart.py:1170 `diff_ms = raw_ms - chord.chart_ms`; rows keep the first group's clock. TM-27 (03-timing.md:315) says the check reads the press "some time after it is sent". Not opened: the report line in replies.txt.
### F2-S1c (far-press still emitted): CONFIRMED, replay_schedule.cpp:939-958: sentence pushed, far_refused set, the row is still written with no flag; only the first refusal per chord.
### F2-S1d (:725): CONFIRMED, `if (!valid(x)) continue;` no sentence.
### F2-S1e flagged(): CONFIRMED, :795-802 returns nullopt when notes empty or no gem of the activation note's colour. Python split: play_chart.py:793-799 sets activating_gem = 0 for a chord with no pad gem and raises for two gems of the top colour (:736-744); C++ returns the first gem of that colour (:799-800) and never raises.
### F2-S1f: CONFIRMED, :895-927: a final-end chord with no aim end falls to "plain" at -2 ms with no sentence; an inside-moved-end chord with no aim end `continue`s through the loop to the same fallback.
### F2-S1g: CONFIRMED (see W2B-A3).
### F2-S1h: CONFIRMED, :282 keeps the last SqOut/last SqIn in the pointer; :679 `geos.front().kind` decides the way; two same-kind squeezes on one anchor share the larger lead (:292-298, no sentence).
### F2-S1i: PARTLY. replay_json.cpp:26-39, :80-82, :87 read: sqin_ticks missing -> none; from_record missing -> false; sqout falls back to a bare offset. The "CLI prints the pick" claim in the file comment (:20-23) I did not check in replay.cpp. The warnings array and the claim that no script reads it: not re-grepped.
### F2-S1j: CONFIRMED for the C++ side (:21-66 constants are above), and the Python copies exist (play_chart.py FRAME_MS and others per S1-C2); not re-read in play_chart.

## From S1-calibrate
### S1-C12 diff_ms at the clock read: CONFIRMED (play_chart.py:1170).
### S1-C8 "silent skip" sub-items: V1 already confirmed; I saw :1186-1189 again (replies of a wrong shape add nothing).

## NOT VERIFIED (never assume confirmed or rejected)
W2A: 7, 8, parts of 1 (baked menu_fields vs file), 10 (all but the server.py line), the focus section, the test-fake section.
W2B: A5, B1 (clock from a stale engine: machine_recorder.py:360-370 only shows OSError/ValueError give None; nothing more read), B2, B5, B6, B7, C2, C3, D4, E1, E3, E4, F3, F4, F5.
F2: J8 report line, S2 (kit.ts), S3 (other agents' catch blocks), S4 (server and tool replies), except the pieces above.
S1-calibrate: C9, C10, C14, C15, C16, C17 not opened by me.
