# Player audit: questions for the user (S6)

Session 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e, 2026-11-08. Each question names the ledger rows it settles. Questions go out five or fewer at a time. The code was read on 2026-11-08 at main `fd93c684`.

## Batch 1: what gets pressed, and whether the run is playing what it thinks

### Q1. Refuse a built agent or exe that does not match its source (L01, L36 to L40)

The game runs the built `tools/ch_mcp/agent/*.js` files, not the `.ts` sources. The loaders' only test is whether the file exists: `if not os.path.isfile(path)` (`frida_tools.py:377`; the same test in `bot_run.py:631` and `trace_hits.py:300`). The kit's built file is from 2026-11-06 21:31, and its source changed on 2026-11-07 (commit `2709008f`). The tests run the `.ts` in Node, never the `.js` (`test_kit_tools.py:1247-1271`). Nothing records which build a run used. The same gap holds for `hydra_replay.exe`: the main checkout's exe is from 11-08 11:14, and `replay_schedule.cpp` changed at 12:26.

What it cost: LS-M2 lost 14 notes, and with them the extension of activation 2 (`ls-m2-run1`: 3,753 of 3,767 notes hit). Freak run 1 lost 4 of its 8 misses the same way.

What you would see: today a stale agent attaches with no message. After the fix, `ch_attach`, `ch_run_chart` and `ch_play_chart` would refuse with a line naming the stale file and the command that rebuilds it. Each run folder would also record which build ran.

- (a) The loaders refuse a built agent whose source is newer or differs, by a stamp the build writes. The run folder records the stamp and the `hydra_replay` build. The tests run the built file too.
- (b) Keep the loaders as they are, and add a build-all step before each live run.

Recommended: (a). A build-all step is a habit, and the last session skipped one. The case against (a): it adds a stamp file to the build, and a refusal can block a run while the rebuild is a minute away.

### Q2. The player refuses a reply or file that is missing what it needs (L01 player side, L14, L65)

When a split chord's reply has no `groups` list, the player keeps the first reply only and goes on: `if replies and later and isinstance(pressed.get("groups"), list):` (`play_chart.py:1166`). The later rows then get no reply values, and no line says so. The same "accept and go on" holds for score files: a missing `path_source`, `activations`, `tick` or `multiplier` changes what is pressed with no message (`play_chart.py:683-686`, `:761`, `:782-783`, `:790-791`). The tests' fake replies are `{}` or `None`, so no test catches a missing `groups` (`test_play_chart.py:505, 584, 1482-1496`).

What you would see: today LS-M2 printed nothing about the 12 chords whose second part never went down. After the fix, the run would stop at chord 3 (the first split chord) with a line naming the missing `groups`.

- (a) Stop the run on a reply that lacks what was asked for, and refuse a score file that lacks a key the play depends on. Make the test fakes send the real reply shape.
- (b) Keep going, but print one line per chord that lost its reply.

Recommended: (a). A run that has already lost notes cannot reach Hydra's score, so going on proves nothing. The case against: a stop ends a long run that might still have shown other faults.

### Q3. The three ways of leaving gems unpressed on a fill the path passes over (L08, L59)

When Hydra's path lets a fill pass, the game would activate Star Power if the fill's last chord were hit in full. The player has three modes for that chord (`play_chart.py:123-126`, `_kept_gems` at `:709-728`):

- `all_but_top_pad` presses every gem but the highest pad (the gem that activates, FL-09).
- `kick_only` presses only the first kick.
- `nothing` presses nothing.

With no mode given, every gem is pressed, and the game activates where Hydra's path does not.

What the game does with the gem left out: LS-M2 left one pad out at each of nine chords (239, 800, 827, 1004, 1029, 1328, 1617, 1662, 1989). The game credited every one as hit (`ls-m2-run1` score steps 460 or 600 each, equal to Hydra's), with 9 activations, the same as the plan. So `all_but_top_pad` cost no note and no streak on two songs (Senri chord 405 was the first). `kick_only` and `nothing` leave more gems out, and no run has tested them.

What the user said: the modes were approved inside two batch answers, "Yes, take all seven" (`docs/sessions/2026-11-07-5b92c813.md:21`) and "yes all create handoff" (`docs/superpowers/plans/2026-11-07-player-refinements.md:111`). Neither names `kick_only` or `nothing`. Later you said "you cant just choose to not hit notes!". That was about a wrong diagnosis of LS-M2's misses, but no ruling since says which mode, if any, you want.

- (a) Keep only `all_but_top_pad`, and remove `kick_only` and `nothing`.
- (b) Keep all three as they are.
- (c) Remove declining altogether, so every gem is pressed and the game activates at every fill. The player could then not follow any path that passes a fill.

Recommended: (a). It is the one mode two runs proved costs nothing, and the others were never chosen in your words. The case against: a fill that ends on a kick alone has no pad, so `all_but_top_pad` sends nothing there (`_kept_gems` returns an empty list when no pad gem exists). Whether the game credits that kick is unproven, and `kick_only` was the mode that would have pressed it.

### Q4. Which gem of a split chord goes first (L09, L10, L19)

Some chords are sent in two parts, 8 ms apart, so the game scores the gems in a set order (`play_chart.py:488-527`, `CHORD_ORDER_GAP_MS = 8` at `:113`). At an activation chord, the player sends the activating cymbal first and the kick 8 ms later. A session card holds your words: "in both cases you have to hit the kick first and cymbals are interchangeable so t doesnt matter which is hit first" (`docs/sessions/2026-09-29-56ac5b71.md:17`). The page block SC-38 rests on one run, and says which gem activates "was not read in the code". The 8 ms gap and "Only where it matters" are your words (`player-refinements.md:204`).

Two smaller facts sit beside it. The 8 ms is made three ways: the C++ plan, the kit's 2 ms tick, and a `time.sleep` on the keys route. A frame longer than 8 ms would put both parts in one frame. And a squeezed-out chord is sent as two messages 30 ms apart, a number with no named source (chord 1893 in `ls-m2-run1`).

No run has yet sent a later part with the current kit agent, so whether the order works is unproven.

- (a) Read the game's code for which gem activates and in what order a chord's gems score, before any change.
- (b) Switch the activation chord to kick first, cymbal 8 ms later, per the card.
- (c) Keep the order as it is.

Recommended: (a). Your card's words were about a squeeze, and this is an activation chord, so the card may not apply. The game's code settles it. The case against: it delays the rerun of LS-M2 by one code read.

### Q5. Check that the run plays the song, path and settings its files are for (L16, L17, L20, L21)

Nothing compares the score file with the song the game has loaded. In `game_tools.py:1026-1109`, the song folder is used only for the run cap (`:1055`) and the watch (`:1075`). A score for one song played into another would run and report hits. The judging scripts never read the stats file's `checksum`, `playback_speed`, `no_fail` or `modifiers` (`compare_scorestats.py:81-134`). `ch_open_song` can end with "done: a song is loaded, but its folder is X, not the target Y" as an ordinary reply (`menu_tools.py:285-288`). Its first two steps check words that already show before any press (IN-79), and no step checks the instrument or difficulty. `ch_run_chart` always uses dump path 0. It refuses a mismatched score that records its path, but accepts one with no `path_source` (`play_chart.py:683-686`).

What you would see: today a wrong song, speed or modifier runs and is judged as if right. After the fix, the run refuses before the first press, and the judge fails on a mismatched checksum or speed.

- (a) Refuse to start unless the loaded song's folder matches the score's chart. Have the judge compare checksum, speed and modifiers. Make `ch_open_song`'s wrong song a refusal, check instrument and difficulty, and give `ch_run_chart` a `path_index`.
- (b) Only add the song-folder check before the first press.

Recommended: (a). Each piece closes a way a run can be judged against the wrong thing. The case against: (a) touches four tools at once, and `ch_open_song` has no live proof yet (LS-K3 was struck), so its new checks need a live run.

## Batch 2: how a run is judged, and the late edge

Q5 is as written above.

### Q6. What counts as a hit, and when a run passes (L22, L23, L24, L28, L29, L30)

Today the player marks a chord hit when the score rose at all after the press, and copies that to every part of a split chord (`play_chart.py:1145-1148`). `score_compare` passes a run at 98 % of rows hit (`MIN_HIT_PERCENT = 98`, `score_compare.py:64`). It forgives any difference at a chord with a ghost or accent (`elif chord.ghost_or_accent or change_may_show: status = "ghost_accent"`, `:188`), a rule written for keyboard presses. `ch_compare` prints "pass" for `total_notes`, `sp_phrases_total` and `sp_activations`, which are chart facts, and never compares the game's `notes_hit` or `sp_phrases_earned` (`compare_scorestats.py:94-125`). The command line prints "Done." and exits 0 for every stop reason but one (`play_chart.py:1382-1396`).

LS-M2 today: "PASS hit rate 99.6% (2141 of 2150)", with 12 chords marked hit that lost a gem, 39 chords forgiven as ghost or accent, and three "pass" lines, while the game hit 3,753 of 3,767 notes. LS-M0 today: 1,478 chords equal, PASS.

- (a) Judge by the source facts. A chord passes when the game's score step equals Hydra's points for that chord (the score file). A run passes only when every chord passes, and the game's own `notes_hit` equals `total_notes`, `sp_phrases_earned` equals Hydra's phrases, and the score equals Hydra's total. The 98 % floor and the hit-by-any-rise go. The ghost and accent forgiveness stays only on the keys route. A stop other than the song's end is a failure. LS-M2 would fail at chord 3. LS-M0 would still pass.
- (b) Keep today's checks, and add `notes_hit` and `sp_phrases_earned` lines.

### Q7. The Star Power checks that cannot fail (L25, L26, L27)

In `sp_compare.py`, "activation N on the path's chord" calls `report.check(True, text)` whenever the activation lands on the right chord (`:523-534`). The end difference is only printed: LS-M2's activation 2 ended 3,544 ms early, on a PASS line. "Flag clears at the end" compares the game with its own end time. The multiplier check prints PASS for "0 of 0" chords compared. `sp_max` passes whenever it is the same on every row, whatever its value. `ch_compare` passes `total_notes` if it equals either the gem count or the chord count.

- (a) The end check fails when the last chord the game pays inside Star Power is not Hydra's deactivation chord. The flag check compares with Hydra's end. "0 of 0" is a failure. `sp_max` is checked against the game's value for the chart (30 times its resolution, SP-11). `total_notes` is compared with the gem count only (LS-M2: 3,767 gems).
- (b) Print the end difference as its own FAIL line past a fixed number of ms, and leave the rest.

### Q8. How late a press may be sent: 0, 1 or 2 frames (L48, L05)

The schedule gives four answers to one question. `press_room_ms` allows 2 frames of late judging (`replay_schedule.cpp:249-251`). `last_send_ms` allows 1 frame (`:329`). `far_press_refusal` allows none (`:392-402`). The two fallbacks use 2 and 1 (`:333-341`). The player's drop check uses 1 (`play_chart.py:1115`). Example, from the schedule's own numbers: a 50 ms side and a chord at 1,000 ms give a latest send of 1,047, a far-press limit of 1,050, and a room limit of 1,044. The game fact SP-47 (four end reads, 2.98 to 3.03 ms) says one frame. No page block says two. No run uses the schedule yet.

- (a) One frame everywhere, from SP-47, as one constant the schedule and the player both read.
- (b) Read the game's code for the judge lag first (TM-27 is not proven), then pick.
## Batch 3: the squeeze split, game facts, the schedule's skips, the keys route

### Q9. How a squeeze's timing is shared between its two presses (L07, L47)

Hydra rates a squeeze by the one number it needs, and gives the frontend transfer scale (`SPSqueeze`, `src/core/model.h:371-420`). It does not say how that need is shared between the anchor press and the squeeze press. The two player copies share it differently. The Python gives the anchor `need / ratio + 6` and the squeeze press none (`play_chart.py:347-428`). The C++ gives both presses equal room inside their windows (`equal_room_lead_ms`, `replay_schedule.cpp:224-327`). Example: equal 85.72 ms sides, a 130 ms chord-to-end gap and ratio 1 give a Python lead of 65.28 and a C++ lead of 78.5. The replay plan's question 1 offered (a) equal room, (b) your "-45/+45", and (c) the minimal lead, and recommended (a). TC2 built (a). In session 6341c618 you said "rebuild and go with recommendations", but no record says which plan's recommendations that took. On a refusal, the Python stops at the first bad press, and the C++ writes one refusal per chord and has fallback rows.

- (a) Equal room, as TC2 built it. The schedule lists every refusal, and under Q2 the run does not start on any.
- (b) Your -45/+45. The plan shows it fails on a 100 ms squeeze such as Rescued 931.
- (c) The minimal lead.

### Q11. Rules that rest on unproven or contradicting game facts (L11, L56, L57, L60)

The squeeze-out late gem is pressed 15 ms after the end. Its value is your batch yes ("yes all create handoff"), but the game facts behind it, SP-29 and SP-30, are Unknown. The far-press refusal applies to every pad (`replay_schedule.h:169-174`). TM-50 proves it only for a kick pair, and its Ruling line says "no Hydra rule depends on it". A comment in `kit_tools.py:258-261` says a plain gem's velocity is never read, which SC-13 does not say. Pages lag the code: FL-31 cites one run where LS-M2 adds nine more chords, TM-50 does not name Freak chord 803, and SP-47's Hydra line says there is no frame model.

- (a) Read the game's code for SP-29, SP-30 and the two-window rule on every pad. Fix the comment, and write the run results back to FL-31, TM-50 and SP-47.
- (b) Only write back and fix the comment, and leave the unproven rules as they are.

### Q12. The schedule's silent skips and untested branches, before the player reads it (L49 to L55, L66)

Several branches of `replay_schedule.cpp` drop a chord from the press list. Some write a refusal sentence (`:818-823`, `:850`) and some write none (`:638`, `:641`, `:725`). A refused far-press is still written as a press row (`:939-958`). A declined chord with nothing to send gives a row with no gems. A squeeze-out chord presses all its gems, not the declined chord's kept gems (`:848-860`); the Python refuses that pairing (`play_chart.py:785-788`). Two of the schedule's rules and about ten refusals have no test.

- (a) Every skip writes a refusal and a flag on its row. A squeeze-out chord that is also a passed fill uses Hydra's activation-note rule (Q3), or is refused. Each branch gets a test before TP1 lets the player read the schedule.
- (b) Fix the silent skips only, and leave the tests for later.

### Q13. The keys route, and the window coming to the front (L61, L62, L64)

The player's command line has a keys route that sends keys with `SendInput` after `SetForegroundWindow`, drops that call's result, and only warns when no game window is found (`input_driver.py:284-291`, `play_chart.py:1333-1340`). Your ruling: "yes we need to do autonomous runs. that waws the whole point of this" (`docs/agents/rulings/clone-hero.md:10`). The kit route needs no focus. LS-M0's window came to the front at 43.9 s with no cause found in code. The foreground watch polls every 0.1 s and keeps its events only in memory.

- (a) Remove the keys route from play runs, so play is kit only. Key tools that remain refuse when no window is found or focus fails. The watch saves its events to the run folder. Finding the 43.9 s cause waits for a run with a window-event log, on your go.
- (b) Keep the keys route, but have it refuse when the window is missing or focus fails.
## Batch 4: lines that say the wrong thing, and the small fixes

Three read-only scouts checked every row below against the code, the rulings, the D-records, the Clone Hero pages, the plans and the session cards on 2026-11-08. No ruling, card or plan covers any Q14 row. TP1 deletes none of this code.

### Q14. Report and launch lines that can say the wrong thing (L31, L32, L34, L35, L74, L41, L42, L43)

Each of these prints a line that is wrong, or says less than it seems to. No number the player acts on is wrong. Each fix changes only what a line says.

- L31. The report says "Clock at press minus chart ms" (`score_compare.py:304`). The number is the clock the player read just before it sent the press, not the moment the game judged it (TM-27 says the same). After: "Clock at the read before the send minus chart ms". The numbers stay the same.
- L32. Every `ch_*` reply is cut at 12,000 characters, keeping the start (`hydra_ui_mcp.py:45`, `server.py:46-51`). `ch_sp_compare` prints its "Summary: N PASS, M FAIL" last (`sp_compare.py:633`), so a long report loses its verdict. A cut has already happened in a run: "The reply cut at 12,000 characters, so rows for chords 0 to 16 were not read." (`ls-o2-run1\replies.txt` line 25, a status reply). The cut hint also points at `ch_notes`, which does not apply to `ch_sp_compare`. After: the summary is the first line. The Hydra GUI tools had the same bug and were fixed the same way (`docs/sessions/2026-10-24-dacea5a6.md`).
- L34. `trace_hits` prints "calls within 85 ms of their nearest note: N of M", where M counts calls, not notes (`trace_hits.py:224-246`, `frida_tools.py:553-570`). A missed note fires no call (`docs/agents/clone-hero-mcp.md:53`), so it never shows. A real trace says "284 of 284" for 142 notes (`2026-10-28\l3\trace-wm-args.txt`). After: one added line, "notes with no call within 85 ms: 0 of 142". The flat 85 ms stays. It is the widest side the game allows, and the line is true as worded.
- L35. The machine recorder writes whatever 8 bytes it reads at engine `+0x100` as the clock, even NaN or a negative value (`machine_recorder.py:356-373`). After: such a read becomes "no clock" for that sample. Real runs so far look sane (largest step +1.6 ms in `ls-t3-run-b\report-a.txt`).
- L74. With no song length for a folder, `ch_sp_watch_start` prints "run cap: 240 s (game_tools.run_cap_s for <folder>)", as if the 240 came from the song (`sp_tools.py:197-202`; `run_cap_s` falls back to `RUN_CAP_S` itself, `game_tools.py:171-181`). After: "run cap: 240 s (default; no song length for <folder>)".
- L41. A launch whose loader check reads "no" (the loader ran, and the bytes differ) prints that word and goes on (`bot_run.py:556-566, 701-716`). `ch_status` does re-read the line later (`bot_run.py:590-597`), so the ledger's "nothing re-reads it" is out of date. After: `ch_status` adds a plain warning line when the word is "no". "No" has never appeared in a saved run.
- L42 and L43. `docs/agents/clone-hero-mcp.md:74` says "If the game is updated, every Frida tool refuses until a dump of the new build exists". That holds for `ch_attach` and the trace tools (`build_facts.load_facts`). It does not hold for the launch agent, which never calls `load_facts` and rests on its byte checks (`launch_core.ts:160-182`). The recorder reads an address the Star Power watch has already checked. After: the sentence says the launch agent is guarded by its byte checks. No code changes.

Left out, because each would be a new rule that nothing in the repo states: failing a launch on "no", comparing `launch_patch.json`'s "build" with the installed version, and having the recorder catch a frozen or restarted clock.

- (a) Make all seven changes above.
- (b) Make only L31 and L32, the two that show in today's runs, and leave the rest.

Recommended: (a). Each is a text change, and each line would then say the true thing. The case against: five of the seven have never shown in a saved run.

### Q15. The smaller items, as one list of fixes (L12, L13, L15, L18, L33, L75, L76, L78, L80, L81, L82, L85)

Four rows need nothing:

- L12. The room check in `retime_from_end` already belongs to the replay plan's TP2, with tests at both edges (`2026-11-08-replay-press-schedule.md`, lines 146 and 230 to 238).
- L18. The player reads the raw clock, and the walk-edges probe reads a filled one. They are two tools with separate jobs, and the comments say so. A late press after a short stall is already caught by the drop check, and Q8 makes that one frame.
- L78. The recorder's report prints `settings.ini` lines from any section, and `game_files` reads only `[game]`. The comment at `machine_report.py:49-52` says why: framerate and vsync sit in `[video]`.
- L80. When the recorder falls behind, it skips ticks, and the report prints the gaps (`machine_report.py:280-284`).

Seven are small fixes, each with no new rule:

- L13. Chords less than 150 ms ahead of the clock at start are skipped, and nothing says so (`live.py:152-163`). The refinements plan lists this as out of scope pending your yes (`2026-11-07-player-refinements.md:197`). After: the start line adds "skipped 1 chord before 2.00 s", and the same after a restart. The skip itself stays.
- L15. An empty chord, or two gems on one kit element, slips through quietly (`play_chart.py:157-166`, `kit_tools.py:312-316`). The Q3 and Q4 ruling already removes the empty declined chord. After: the loader refuses a chord with no notes, and the kit refuses a repeated element, the way Q2 refuses a bad score file. No real `hydra_replay` file has either shape.
- L33. `find_activations` starts at row 1, so an activation already on when the watch starts is never reported (`sp_compare.py:229-245`). After: it is reported as "already on at the first row". Every run so far started the watch before activating.
- L75. `sp_compare._ms` repeats `constants.s_to_ms`. After: it calls `s_to_ms`. The numbers are the same.
- L76. `ch_open_items` reads four tools' private fields (`server.py:104-112`), so renaming any of them would break it. After: each tool gets a small public check that `ch_open_items` calls instead. You see nothing change. The row's other sub-items were never opened by a verifier, so they stay out.
- L82. `ch_install_chart` joins `--name` onto the probe folder unchecked, so a `..` writes outside it, and a failed install then deletes that outside folder (`install_chart.py:78-89`). It also copies the source's other audio files, which the game will play. After: a name that is not a plain folder name is refused with a "refused:" line, and the source's other audio files are left out of the copy.
- L85. Run by hand, `trace_hits` with an empty manifest crashes with a traceback (`trace_hits.py:119, 157`); the MCP route already guards this (`frida_tools.py:520`). After: one plain line, the same as the MCP route's.

One needs your word: L81, the song length. D255 holds your words: "there should one rule that every function uses. we already know how clone hero decides the end of the song, we don't need different places deriving different rules." (`docs/audit/2026-10-03-fix-decisions.md:1059`). The Python player does not follow it. Its run cap and `ch_install_chart`'s silent audio read `song.ini`'s `song_length` directly (`game_files.py:357-367`, `bot_run.py:387-403`, `install_chart.py:72`). Where `song.ini` has two `song_length` lines, the player takes the first and Hydra's reader the last (`analysis.cpp:328-365`). Hydra's one end, `app::song_length_ms` (`src/app/song_length.cpp:28`), never reaches the player, because `hydra_replay` does not write it. Which of two lines the game takes is not on the Clone Hero pages (CR-45 to CR-48 on page 01). No chart in `testdata/input` has two lines.

- (a) Make the seven fixes. For L81, `hydra_replay` writes Hydra's song end into its output, and the run cap and silent audio read it, per D255.
- (b) Make the seven fixes. Leave L81 as it is, and record the difference.

Recommended: (a). D255's words already ask for one end everywhere, and the player is the one place left that has its own. The case against: it adds a field to `hydra_replay`'s output, which means a build and a test, and probe songs today take their length from `tools/ch_probe/probe_songs.py:201-203` (last note plus 3,000 ms), which would also have to read Hydra's end.
## Batch 5: what the second closure check reopened

The fresh closure checker of 2026-11-08 (session 274bd9dd) passed L06, L46's window split, L56's page split, L18's two clocks, L78 and L80, and reopened three things. The main session read each one in the code before writing it here.

### Q16. The far-press refusal is stricter than the game (L56)

TM-50 on page 03 is now proven in the game's code: a press goes to the earliest note on its pad that is still unhit and whose window holds the press. `far_press_refusal` (`tools/replay_schedule.cpp:392-422`, the same rule as `_refuse_far_presses` in `play_chart.py:632-665`) refuses a press in two other cases:

- **An earlier note.** It refuses a press that lands inside the window of the nearest earlier chord on its pad (`replay_schedule.cpp:404-410`), whether or not that note is already hit. The game skips a hit note, so a press there is safe once the earlier note's own press has gone in. It matters only when that earlier note is still unhit when the press lands: it has no press (a skipped fill's left-out gem, Q3), or its own press is sent later than this one.
- **A later note.** It refuses a press that lands inside the window of the next chord on its pad when the player sends no press on that pad for it (`:412-420`). The game gives such a press to the press's own note, which is earlier and unhit, so the later note is never touched.

Example: two snares 100 ms apart, each with an 85 ms side, both pressed on time. A press of the first sent 20 ms late lands inside the second's window. Today the schedule lets it through, because the second is pressed. If the second were a gem the player leaves out, the schedule would refuse it, though the game gives the press to the first snare either way. No run reads the schedule yet, so no refusal has stopped a run.

- (a) The refusal follows TM-50 exactly: refuse a press only when an earlier note on its pad whose window holds it will still be unhit when it lands, because the player sends it no press or sends that press later. The later-note branch goes.
- (b) Keep the rule as it is, stricter than the game.

Recommended: (a). Every refusal left would be a press the game really gives to the wrong note, and no run stops on a press that would have worked. The case against: (b) leaves a margin if a press is dropped at run time. Under Q6 a dropped press already fails the run.

### Q17. A press moved earlier by the read end is never checked against its window (L12)

When the game's real Star Power end comes earlier than planned, `retime_from_end` moves every press timed from the end earlier by the same ms (`play_chart.py:449-459`). Nothing checks that the moved press is still inside its window. Freak-fixes question 3, which you approved with "Yes, start it", covers only the other direction: a late end that leaves no room. The replay plan's TP2 tests only that side and the drop past the latest send (`2026-11-08-replay-press-schedule.md` lines 230 to 238).

What the game does with a press sent before its note's window opens: by TM-50 it goes to an earlier unhit note on that pad if one is in its window. If none is, it is an overhit, which breaks the combo (`docs/audit/drum-lanes/FINDINGS-play-time.md` line 60).

The end reads so far came 3 to 4 ms later than planned (Freak run 1), never earlier, so this has not happened.

- (a) TP2's room check also tests the early edge. A press the shift would send before its window opens is sent at the window's opening instead, and its row says so. That is still after the real end, because the end moved earlier.
- (b) The run stops, with a line naming the chord, the way Q2 stops on missing input.
- (c) Leave it.

Recommended: (a). It keeps the run going, and the press stays both inside its window and after the end. The case against: it is a new rule, for a case no run has shown.

### Q18. The probes keep a third copy of the window formula (L46)

The window formula has three homes. The player's Python copy (`window_sides`, `note_window_ms`) goes in TP1. The C++ schedule's copy stays as the one the player reads. The third is `predicted_window_normal` in `tools/ch_probe/experiments/analysis.py:227`. It is read only by the probes `active_probe.py`, `passive_probe.py` and `walk_edges.py`, and no play run imports it. Your scope answer, "Yes, recommended scope", left out the probe scripts no play run imports. So this copy was never audited, and nothing has ruled that it may stay.

- (a) Fold it into the fix plan: the probes call the schedule's formula, with the constants they decode from the game's memory passed in.
- (b) Leave it out, as your scope answer did, and list it for a later probe audit.

Recommended: (b). Your scope answer drew this line, and the probes are not used in play runs. The case against: it is a copy of a rule that two homes already share, and your words "why are you still letting different areas do their own thing? this is why everything needs to be derived once" (2026-10-14, `docs/agents/rulings/code-and-engine.md`) ask for one home.
## Answers

- Q1: "Refuse stale (Recommended)" (2026-11-08): loaders refuse a stale built agent by a build stamp; run folders record the stamps and the `hydra_replay` build; tests run the built file.
- Q2: "Stop the run (Recommended)" (2026-11-08): stop on a reply missing what was asked for; refuse a score file missing a key the play depends on; fakes send the real reply shape.
- Q3: not answered as asked. The user: "why are there three modes? who came up with this rule? what is the exact text i was presented with? there is only one way to correctly handle a skip and hydra's engine already knows it".
- Q4: not answered as asked. The user: "that was about multiplier squeezes. hydra's engine already knows how to handle activations. you keep trying to reinvent new rules instead of reading what we already have".
- Q3 and Q4 ruling: the main session named Hydra's `Chord::activation_note()` (`src/core/model.cpp:307`: first of Green, Blue, Yellow, Red, Kick) as the one rule, for the gem a skipped fill leaves out and the gem an activation chord presses first, with every gem of an activation chord paid at the Star Power multiplier (`category_scores`, `src/core/scoring.cpp`; `add_act_edge(sg.sp, ...)`, `src/search/graph.cpp:207`). The user: "Yes, that rule" (2026-11-08). So the player reads that gem from Hydra; `kick_only`, `nothing` and the Python `_TOP_PAD_ORDER` go.
- Q5: "All of it (Recommended)" (2026-11-08).
- Q6: "Source facts (Recommended)" (2026-11-08).
- Q7: "Judge vs Hydra (Recommended)" (2026-11-08).
- Q8: "One frame (Recommended)" (2026-11-08).
- Q9: the user: "i provided 45 as an example. why do you keep carrying it forward as if it was a rule? hydra already knows what timing you need and it can be split equally" (2026-11-08). So the squeeze's need, as Hydra rates it, is split equally between the two presses (equal room, as TC2 built it); -45/+45 is not an option.
- Q12: "Flag and test all (Recommended)" (2026-11-08).
- Q13: the user: "i care about the runs working, not the window coming into focus" (2026-11-08). No change is made for focus; the 43.9 s window event is not pursued.
- Q11: the user: "are the facts actually unknown or did you never check what we already have documented?" (2026-11-08). The main session then read the pages: the squeeze-out press rests on SP-28 (Proven in the game's code), SP-31 (Proven in a live run), SP-36 (Proven in the game's code) and SP-47, not on SP-29 and SP-30, which only concern a hit judged in the exact end frame; the velocity path is IN-86 and NL-10 (Proven in the game's code); the far-press rule on other pads is the one gap, and the base hit routine it needs is already committed at `docs/audit/drum-lanes/play/f20f63f0_base_hit.asm`.
- Q10 was not sent. L04 and L05 follow the Q8 answer (one frame, which also fits TM-26's early and late edges on an 85.72 ms side). L06's 3.0 ms frame is freak-fixes question 1 (a) under "Yes, start it". L46's copies follow L44's ruling, and Hydra's fixed 85 ms is the user's "hydra does not currently account for the dynamic hit window, although that is a good idea for a new feature" (`docs/superpowers/plans/2026-11-08-replay-press-schedule.md:31`). L12 moves to Q15.
- Q11 follow-up: the user: "you said that "most" is already documented. what do you need that isnt documented? that looks like a lot we already have that you didnt check" (2026-11-08). The main session then read `docs/audit/drum-lanes/FINDINGS-play-time.md`: the step loop and next-event functions SP-29 and SP-30 call "not listed" are committed (`docs/audit/drum-lanes/play/f20f4950_step_loop.asm`, `f20f6ff0_next_event.asm`) and explained there; the hit routine counts a note in Star Power by its hit clock (line 98); a press is valid up to and including the window end. Two things stay undocumented: (1) which note a press goes to when two unhit same-pad notes are both in their windows, which the committed press handler `docs/audit/drum-lanes/play/f20deaa0.asm` can settle (real: Freak run 1 chord 154 missed this way, TM-50); (2) whether a press is processed at its own timestamp (the untraced caller of `0x100E0D0`, FINDINGS line 108), which the measured one-frame model (SP-47) already covers in practice and is optional. The user then asked for a handoff to search for other documentation missing from `docs/clone-hero/`.
- Q11 two-note press rule: the user: "Yes, read it" (2026-11-08). The main session read the press handler `docs/audit/drum-lanes/play/f20deaa0.asm` and the base update `f20f5810_base_update.asm` that fills its window array. A press goes to the earliest unjudged note on its pad whose window holds the clock, on every pad and for early and late presses alike; each pressed pad hits at most one note per update. Written back to TM-50 on page 03 (now Proven in the game's code), which also settles L56: the schedule's `far_press_refusal` rests on a rule the game's code holds for every pad. The second Q11 item (whether a press is judged at its own timestamp) stays optional, covered in practice by SP-47's one-frame model. This closure goes to the fresh closure checker with L06 and L46.
- Q14: "All seven (Recommended)" (2026-11-08): the seven line changes as written under Q14; the three new rules listed there stay out.
- Q15: "Follow D255 (Recommended)" (2026-11-08), to a question that said the seven small fixes go ahead either way: the seven fixes as written; L12, L18, L78 and L80 need nothing; for L81, `hydra_replay` writes Hydra's song end into its output and the run cap and the silent audio read it, per option (a)'s text. Option (a)'s case against names the probe songs' own length (`tools/ch_probe/probe_songs.py:201-203`) as a further cost; the answer does not say it is taken, so the fix plan lists it, together with Q18's removal of the old probe scripts.
- Q16: "Follow TM-50 (Recommended)" (2026-11-08): the far-press refusal refuses a press only when an earlier note on its pad, whose window holds it, will still be unhit when it lands (no press, or its press sent later); the later-note branch goes.
- Q17: not answered as asked. The user: "why is the real window different from what we planned? what is the flaw in our planning? how do we fix it?" (2026-11-08).
- Q18: not answered as asked. The user: "remove the old probe scripts, we dont need them anymore" (2026-11-08). Which scripts that covers is for the fix plan to list and the user to confirm; `analysis.py` is read by `active_probe.py`, `passive_probe.py`, `walk_edges.py` and `tools/ch_probe/tests/test_analysis.py`.
- Q17 settled: the main session answered the user's question from SP-47 (page 05): the game judges a press one frame after the clock read, and the Python player plans the end with no frame (SP-47's Hydra line), so every end read came about 3 ms late. The C++ schedule plans it with the frame (`planned_end_ms`, `tools/replay_schedule.cpp:258`), Q8 makes that one frame one constant, and `live.wait_until` never sends before the aim (`live.py:142`), so after TP1 an early end can come only from a frame shorter than 3.0 ms (2.98 ms at the shortest of SP-47's four reads). Offered "plan the end with the frame (TP1 + Q8), no early-edge rule", the user: "Yes, settle it" (2026-11-08).
