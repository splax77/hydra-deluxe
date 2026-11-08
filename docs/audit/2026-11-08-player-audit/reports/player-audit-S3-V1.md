# player-audit-S3-V1: verifier report (F1, F2, S1-calibrate candidates)

Main checkout fd93c684 for the built files and run folders; worktree player-audit-plan for source (same commit as main for these files). Nothing built, no test run, no game. What I ran: `git diff a41101fc HEAD` on the agent folder; `py -I` of three small scripts of mine over one run folder (`v1_dups.py`, `v1_split.py` in this folder) reading `ls-m2-run1/press-2.csv` and `kick-prep/who/...-score.json`; `score_compare.py` over those two files (a judging script, no game, no build). Evidence marks: "reading" = from code; "proven" = from the named run file.

## 1. Stale built agents and the LS-M2 run

### F1-C1 / F2-J5 / S1-C8 (reply columns) / F1-item-6: CONFIRMED, and the effect on the run is proven
Verdict: the built `kit.js` in the main checkout is the old build and the LS-M2 run pressed with it. The blank reply columns come from the agent side (old kit.js sends no `groups`); the Python side passes the reply through and accepts a missing `groups` without a word. The LS-M2 run's 14 "not hit" gems are the later groups that the old kit never pressed.

Homes:
- `tools/ch_mcp/agent/kit.js` (main checkout, git-ignored) file time 2026-11-06 21:31. `kit.ts` file time 11-07 23:20, last commit 2709008f 11-07 23:11 ("Player limits T3"). Counts of `after_ms`, `groupsOf`, `"then"`, `groups`: kit.js 0, 0, 0, 0; kit.ts 6, 2, 3, 19 (proven by my Select-String counts). `git diff a41101fc HEAD -- tools/ch_mcp/agent/kit.ts` is 129 lines: the T3 groups code (`Group`, `groupsOf`, per-group release, `reply.groups` when more than one group) and one plan-path comment. So kit.js lacks all of T3.
- `kit_tools.py:318-337, 601-630`: `press_message` puts `then:[{elements, after_ms}]` in the message; `kit_press` returns `agent.request(...)` unchanged. No stripping of `groups` there.
- `play_chart.py:1120-1122` sends a split chord as one message with `then=`; `:1165-1167` uses `pressed["groups"]` only if it is a list; `:1186` skips a row's reply values when the entry is missing. With an old kit.js the reply has no `groups`, so every later-group row gets no frame/ms columns, and nothing is said.
- `kit.ts` (new) always writes one `groups` entry per group when there is more than one (diff above).

Proven by `ls-m2-run1/press-2.csv` (written 12:29, after T3 merged at 23:11 the night before; the worker loaded the 11-06 kit.js):
- 2150 rows for 2137 chords: 13 chord indexes appear twice. 12 of them (3, 7, 10, 292, 609, 866, 1064, 1218, 1363, 1497, 1710, 2017) have a second row with blank down_frame, up_frame, down_ms, up_ms, scored_between, and the same score_before/score_after/hit=1 as the first row. All 12 are `then` splits.
- The 13th (chord 1893) is NOT a `then` split: its two rows have different wall_s_at_read, different score_before (933325 then 933845) and its own frames 131754 and 131764 (30 ms apart). It is two separate messages (a squeeze-out part; `play_chart.py:1081-1082` "one part of a squeezed-out chord"). So finder F2's note that row 1893 "shows the second group did get a reply entry" is wrong: it shows nothing about `groups`.
- Game score gain at each of the 12 `then` chords is short of Hydra's: chord 3 game +50, Hydra +250; chord 7 +50 vs +490; chord 10 +100 vs +500; chords 292, 609, 866, 1064, 1218, 1363, 1497, 1710, 2017 each +520 vs +920 (the SP-doubled second gem, 400 points, missing). Chord 1893 (separate messages) game +720 = Hydra +720 exactly.
- Missing gems: chord 3 later group has 2 gems (multipliers [1,2,2]), chord 7 has 2 ([2,3,3]), chord 10 has 1 ([3,3,4]), and each of the 9 SP-activation chords (the 9 `then` chords at 292...2017; the run had 9 activations) has 1 (the second gem of two). 2+2+1+9 = 14 = `total_notes 3767 - notes_hit 3753` in `scorestats-...json` (proven). Reading of why those 9 split: the activation chord presses the flagged gem first (play_chart.py:793-799). The count matches exactly, so the missed notes in LS-M2 are explained by the stale kit.js without needing any other cause.
- Not explained by it: replies.txt says activation 2 ended 163483 ms vs Hydra 167027 ms and phrase-end chord 618 "hit (+400) but the meter gained nothing" (sp_phrases_earned 30 of 31). Chord 618 is not a split chord (one row), so that is a separate cause. Open: needs a live run with a fresh kit.js (or the sp_watch rows around 157925 ms in `ls-m2-run1/sp_watch-2.csv`, 17 MB, which I did not open).

Nothing refuses or logs which kit.js was loaded. Searched `kit_tools.py`, `frida_attach.py`, `bot_run.py`, `trace_hits.py`, `method_watch.py`, `front.py` for hash/mtime/version/stale checks: found only the game build number and "agent missing" refusals. So there is no check that a built .js is as new as its .ts. (Finder said the same; I confirmed with the same kind of search, not a new one.)

Splits:
- kit.ts (T3) vs kit.js built 11-06: for a message with `then` the .ts presses every group and replies `groups`; the .js presses the first group only and replies no `groups`.
- Hydra's score for the 12 chords vs the game's: +920 vs +520 at chord 292 etc.

### F1-C3 (other built agents): PARTLY CONFIRMED (kit.js is the only stale one that matters)
By file time and `git diff a41101fc HEAD --stat`: launch.ts, launch_core.ts, menu.ts, menuhooks.ts changed after their .js builds (launch.js 10-31, menuhooks.js 10-31), but I read the diff lines: comments only (plan paths marked removed). trace.js 11-07 17:11 is as new as trace.ts 17:11 (trace.ts also in that diff, 19 lines; not read in full, same-time build). state.js and clock.js are newer than their sources. `package.json` has one script per agent and no build-all (finder; I did not re-open it).

### F1-C2 (tests run the .ts, not the .js): CONFIRMED by reading
`tools/ch_mcp/tests/test_kit_tools.py:1130-1136, 1247-1271`: reads `agent/kit.ts` and runs it in Node with a fake il2cpp; `@unittest.skipIf(NODE is None ...)` at 1252. Also 1432 reads kit.ts and kit_tools.py. Nothing opens kit.js. So a green T3 test and a stale kit.js are compatible, and are what happened.

### F1-C4 (hydra_replay exe not tied to source): PARTLY
`build-cpp\Release\hydra_replay.exe` in main is dated 11-08 11:14 (proven by Get-Item). `git log` shows `tools/replay.cpp`/`replay_schedule.cpp`/`src/core/replay.*` changes at 11:32, 11:35, 11:44, 11:59, 12:10, 12:26, so the exe predates the schedule code and `paid_by`. The score/dump JSONs in `kick-prep` were written from that exe (reported by finder from prep_inputs; file times 11:16 not rechecked by me). Whether the JSON content differs between 11:14 and now I did not check; the 3b23d8f1 diff touches `src/core/replay.cpp` (+9) and `replay.h` (+19), not read. No stamp of exe or commit in the score JSON or the run folder (finder; I saw `path_source` only). Open: needs `hydra_replay` run (not allowed to me).

### F1-C5 (player does not read `hydra_replay schedule`): CONFIRMED by reading
Searched `play_chart.py`, `prep_inputs.py`, `game_tools.py` for `"schedule"`, `replay_schedule`, `hydra_replay.*schedule`, `presses"]`: no match. Splits (each its own line): kFrameMs 3.0 (`replay_schedule.cpp:35`) vs FRAME_MS 3.0 (`play_chart.py:101`); kPressLeadMs 2 vs PRESS_LEAD_MS 2; kSpEndLeadMs 15 vs SP_END_LEAD_MS 15; kSpOutLagMs 15 vs SP_OUT_LAG_MS 15; kChordOrderGapMs 8 vs CHORD_ORDER_GAP_MS 8; window formula constants `replay_schedule.cpp:21-29` vs `constants.WINDOW_FORMULA_NORMAL_*` used at `play_chart.py:322-324`. Different on purpose or not: the C++ judges the late edge at `kJudgeLateMs = 2 * kFrameMs` = 6 ms (`:39-40`) and early at 1 frame = 3, while `play_chart.py:1115` drops a message at `raw_ms + 3.0 > chart + side` (3 ms late only) and `ACT_LEAD_MARGIN_MS` 6. Two answers to "how late can a press go" for the same input (6 ms in C++, 3 ms in the dropper). Kept as a split.

### F1-C6 (fakes vs real shapes): CONFIRMED
`test_play_chart.py:1482-1496`: `_SplitReplyPress` returns `groups` only when `then` is given and has no `after_ms`, `rewrite`, `on_main`. `:177` base fake `press(...)` returns None; `:1576` lambda returns None; finder's `{}` fakes at 505 are `return {}`. No test sends `then` and gets a reply with no `groups` (grep of "groups" in the file shows only the lines above): so the stale-kit case has no test. Test at `:1522` pins `hit` 1 and 1 with one `score_after` for both rows of a split chord, i.e. it pins the copied `hit`. Fixture details (score JSONs trimmed, `senri_end_chords.json` older) not re-checked.

## 2. Judging that can pass a run with missed notes

Short answer: no judge looked at in this pass reports PASS overall for the LS-M2 run (score_compare exits 1, ch_compare's score line FAILs), but each of the lesser lines can print PASS beside missed gems, and the hit-rate PASS is wrong about gems.

### F2-J2: PARTLY (the lines say what the finder says; the overall result still fails on a miss)
`compare_scorestats.py:94-120`: total_notes passes if Hydra's gem or chord count equals the game's `total_notes` (chart size); `sp_phrases_total` equals Hydra's phrase-end chord count (chart size); `sp_activations` compares a count (no times); `accent_notes_hit`/`ghost_notes_hit` are real play results. `notes_hit`, `sp_phrases_earned`, `excess_hits`, `end_streak`, `max_streak` are never read (reading of lines 94-134; the function returns `notes_ok and sp_ok and score_ok and dynamics_ok and activations_ok`). Proven by `scorestats-...json` of ls-m2-run1: notes_hit 3753, total_notes 3767, sp_phrases_earned 30, sp_phrases_total 31, sp_activations 9, score 1063755, solo_bonus_total 12100. And `replies.txt` line 2: score FAIL, notes/phrases/activations pass. So a run with missed notes cannot pass `compare()` unless the score is equal to the point; the misleading part is wording and the lines that print "pass" for things that are chart facts. The score equality is the only play-result gate. Whether a miss can leave the score equal: no, any lost gem changes it by reading (not tested).
Splits: three numbers for "the score" in one run: game `score` 1,063,755, score_compare's last `score_after` 1,051,655, Hydra `final.total` 1,143,035. 12,100 (solo_bonus_total, proven in the stats file) is the difference between the first two; score_compare never sees it.

### F2-J3: CONFIRMED (I re-ran score_compare)
Ran `py -I tools\ch_probe\experiments\score_compare.py ls-m2-run1\press-2.csv kick-prep\who\...-score.json`: "Score at 2150 presses: 1834 equal, 0 lagged, 39 changed at a ghost or accent chord, 277 failed"; "FAIL score at chord index 3: game 495, Hydra 695 (difference -200, was +0)"; "Final: game 1051655, Hydra 1143035"; "PASS hit rate 99.6% (2141 of 2150)"; exit 1. So the score check caught the problem; the hit-rate check did not (hit=1 on every real press row, 9 `declined` rows count against it; 9 of 2150 = 0.4%).
Causes by reading: `play_chart.py:1145-1148` reads the score once per message and `hit = score_after > score_before` (any rise, 5 ms later); `:1178` gives every row of a split chord the same `hit`; `score_compare.py:120` counts `row["hit"] == "1"` and `:247` passes at 98% of rows. `n` counts rows (2150), not chords (2137). 98% floor: 49 of 50 passes, 48 of 50 fails (`n_hit*100 >= 98*n`). Declined/dropped rows count as non-hits against the floor (a run declining over 2% fails the rate by design).
Open: why the second group "gets no points" is now settled for LS-M2 by section 1 (never pressed by the old kit).

### F2-J4: CONFIRMED by reading
`score_compare.py:188-190`: `elif chord.ghost_or_accent or change_may_show: status = "ghost_accent"; running = difference`. Any size of change at a chord with a ghost/accent gem (or the chord after a lagged one) is accepted and becomes the new baseline. The module docstring (lines 16-19) gives the reason: "A keyboard press carries no strike velocity, so the difference may change on a chord with a ghost or accent note". The kit route sends axis values (`play_chart.with_axes`, `kit_tools.pad_axis`; not opened by me), so the stated reason is about the keys route; the rule applies to kit runs too. 39 chords were absorbed this way in LS-M2 (proven by the report line). Table: a missed accent chord (-500) gives status ghost_accent, no FAIL.

### S1-C3 (dropped chords are silent misses in the headline): PARTLY
`play_chart.py:1150-1156`: dropped chords go to `result.dropped`, not `sent`; `:1391-1393` (CLI `main`) prints Notes `sent`, Misses `sent - hit`, Hit rate `hit/max(sent,1)`, and never prints `result.dropped`. So the CLI summary can read 100% with chords dropped. But the MCP path (`game_tools.py:1534-1537`) prints `ended: <stop>`, `misses {sent-hit}`, `declined N` when non-zero and `dropped_line(rows)` ("dropped: chords ..." or "dropped: none"), so on the route the runs use the drops are named. Reading only.

### S1-C4 (main returns 0 and prints Done for every stop): PARTLY
`play_chart.py:1384-1396`: only STOP_NO_ENGINE returns non-zero (2); cap/frozen/lost/interrupted/stopped/past_end all print the summary, "Done.", return 0. `:1221` turns any OSError in the loop into stop="lost". CLI only; the MCP path shows `ended: <stop reason>` (`game_tools.py:1491-1509`). Unplayed chords are in no count.

### S1-C6 ("hit" is any rise over the whole message): CONFIRMED, now proven
`play_chart.py:1145-1148`; `engine.pressed_input_hit` not re-opened. Proven: the 12 `then` chords above print `hit=1` while the game scored only the first group's gems.

### F2-J6 (sp_compare passes on game's own consistency): CONFIRMED by reading
`sp_compare.py:523-534`: "activation N on the path's chord" is `report.check(True, ...)` whenever the landing chord matches; the end difference is only printed in the text (`:529-531`), never checked (nothing else in the file mentions `deact`: grep). `:470-478`: "flag clears at the end" compares the flag drop to the game's own `sp_end_s` within EVENT_WINDOW_MS 100. `:594-596`: multiplier check prints PASS with `fails == 0` even for 0 of 0 compared. `:578`: accepts expected multipliers of chords i-1, i, i+1. Phrase-rise (360-390), clock NaN (166), "no path activation, no game activation = no line": not opened by me (listed under "not verified" below). Relation to the run: LS-M2 activation 2 ended 3,544 ms before Hydra's deact (replies.txt); a PASS line would have carried that number inside the text. I did not run sp_compare (needs the 17 MB watch file; not necessary for the code claim).

### F2-J7 (sent/hit/misses lines): CONFIRMED by reading
`game_tools.py:1501-1554`: `running:` counts `r['hit'] == 1` rows (a split chord counts twice); `misses {sent - hit}` excludes declined and dropped; they are printed beside it. LS-M0 (`ls-m0-run1/replies.txt`: 1478 chords sent, 1478 hit, 0 misses) matches the stats compare passing (replies.txt) and finder's CSV facts (not recounted by me).

### F2-J1 (no check that files belong to the same song/run): PARTLY
Proven: `scorestats-...json` carries checksum, playback_speed 100, no_fail true, modifiers (grep of the file). Searched `tools/**/*.py` for `checksum|song_name|playback_speed|no_fail|chartmode|["warnings"]|get("warnings"`: matches only `test_compare_db.py`, `test_probe_songs.py`, `find_checksum.py`+test, `il2cpp_strings.py`, `test_machine_report.py`, `test_game_tools.py:498-503` (a stats text print that shows `modifiers`, `no_fail`; it prints them, it does not compare). No judge script compares them. `bot_run.py:446-471` takes any change of scorestats.json after launch and names the copy from the song folder slug (read). Open: does the game write scorestats.json on an early quit (live quit-early run).

### S1-C1 (window sides equal): PARTLY
`play_chart.py:315-325` states the assumption ("assumes both sides are this one"). But TM-26 (`docs/clone-hero/03-timing.md:305`) says: "Both fit one 85.72 ms side judged a few ms after the press is sent (TM-27)". So the data do not contradict a symmetric window judged late; the finder's "contradicted" is too strong. The page does say the measured sides at the clock read differ (early past 88.43, late before 84.92). Split: load-check edge `abs(at - ms) <= side` (`:647`, both sides) vs drop edge `raw_ms + 3 > chart + side` (`:1115`, late only) vs C++ judge late edge 6 ms; Hydra fixed 85 (`kHitWindowMs`, per TM-26 "Hydra:" line) vs the per-note formula here (TM-23/24 not re-read).

### S1-C2 (drop check judges late side only, fixed 3 ms): CONFIRMED by reading
`play_chart.py:101` FRAME_MS = 3.0; `:1115`. No early-side check. (TM-49/50 frame stretch figures not re-read.)

## 3. The rest

CONFIRMED by reading (homes opened):
- S1-C7: keys route focuses the window (`play_chart.py:1263-1281`; `live_press` also times later groups itself with `time.sleep` after `after_ms`, a third home of group timing beside kit.ts and the C++ plan); `main` always uses it (`:1382`); warning-only when no window (`:1338`). The `allow key run` check is in the hook, not here (docstring says so).
- S1-C11: `retime_from_end` (`:449-459`) moves presses by any delta and `_refuse_far_presses` is called once, at load (`:807`; grep of call sites in tools/**/*.py: only `:807` and a test). The send-time drop check is the only later guard.
- S1-C5 (start skip, restart drops earlier counts): `:1063`, `:1098-1100` (`result = RunResult()` throws away earlier rows). `START_LEAD_MS` in live.py not re-opened by me.
- S1-C8 sub-items opened: `:1166` and `:1186-1189` (silent skip), `:782` (`chord.get("tick")` None accepted), `:790-791` (no `multiplier` key turns off splitting silently).
- S1-C10, C12, C13, C14, C15, C16: only partly looked at (C12 `:1174, 1194`; C15 `_KEY_FOR_PAD` at `:778`). Not verified individually: see list below.
- F2-S1a/b (silent skips in the C++ schedule): `replay_schedule.cpp:489-490` refusals fire only for a missing tick (not a tick that lands on no chord); `:638` and `:641` `continue` with no sentence; `:836`, `:841` `continue` ("refused above"/"the read's refusal names it"); `:818-822` refusal then `continue` leaves the chord with no press row; `:795-802` `flagged()` returns nullopt silently. Today nothing reads the schedule (C5), so no run is affected yet.
- F2-S5: fakes returning None/`{}` at `test_play_chart.py:177, 1576`; see F1-C6.

## 4. Side notes from this pass
- `play_chart.py` row for chord 1893 in LS-M2 shows a squeeze-out is sent as two separate messages 30 ms apart (frames 131754 and 131764), not the 8 ms `after_ms` group gap. `play_chart.py:1081-1082` calls that "one part of a squeezed-out chord". The 30 ms comes from the plan, not the gap constant; not investigated.
- `game_tools.py:1507` `running:` line counts rows with `hit == 1`; after a `DROPPED` string in `hit`, the comparison to int 1 is fine, but it counts split rows twice (J7).
- The CLI `main()` summary and the MCP status text disagree on what is shown (dropped, ended); runs use the MCP path.

## 5. Not verified (never assume confirmed or rejected)
F2-J1 live part (early quit stats); F2-J6 phrase-rise, NaN clock and "no activations, no line" claims; F2-J8 (clock at press vs judging time; matches S1-C12, reading only, TM-27 page claim not re-read); F2-S1c through S1j (C++ lines 725, 897-927, 939-958, 282, replay_json.cpp:80-87); F2-S2 (kit.ts residual quiet spots, TICK_MS), S3 (other agents' catch blocks), S4 (server/tool replies); S1-C9, C10, C12, C13, C14, C15, C16, C17 (kind-7 against kit_tools reply shape is partly answered by F1-C6); F1-C6 fixtures (`*_score.json` trimming, `senri_end_chords.json`); `ls-m1-run1/kit-1.jsonl` was grepped: no `groups`, `after_ms`, `"then"` (it is an M1 setup/kick log from 12:07, before any split press, so it says nothing about the reply shape).
