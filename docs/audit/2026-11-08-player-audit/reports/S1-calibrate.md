# S1 calibrate: tools/ch_probe/experiments/play_chart.py (main fd93c684)

All claims are "by reading" unless said. I read the whole file (1400 lines), plus live.wait_until / first_note_index / StoppedCheck, engine.pressed_input_hit, input_driver.focus_window, the constants named below, and docs/clone-hero/03-timing.md TM-22, TM-25 to TM-27, TM-50. Not reached: tools/ch_mcp/kit_tools.py (pad_axis, the real press reply shape), tools/ch_mcp tests/fakes (kind 7), the C++ side of the dump JSON (tools/replay_json.cpp). Kind 7 is therefore mostly open.

## C1. Window assumed symmetric, but the reference shows it is not (kind 8, and kind 5)
Home: play_chart.py:315-325 (window_side_ms docstring: "the player assumes both sides are this one"), used at 335-344 (window_sides), 368/371 (activation_leads), 646-647 (_refuse_far_presses inside), 1115 (drop check).
What it does: one window per chord, same on both sides of the note, from the game's normal formula over neighbour gaps.
Reference: TM-26 says early reaches past 88.43 ms and late ends before 84.92 ms on an 85.72 ms side. TM-27 says "Not proven" for the lag and the edge on closer neighbours. So "both sides equal" is contradicted for the early/late reach, and the docstring says the page leaves it open.
Table:
- early press 88.43 ms at 85.72 side: game hit; load check `abs(at-chord.ms) <= side` refuses it (raises).
- late press 84.92 ms at 85.72 side: game missed; load check passes it; drop check (1115): raw_ms + 3 > chart + 85.72 is 87.92 > 85.72... evaluated on raw_ms, see C2.
- press exactly at +side (late) or -side (early): load check accepts (<=); TM-25 says the game accepts up to and including the end on the clock, but TM-27 says the effective edge is lagged.
Split: (a) per-note formula window here vs Hydra's fixed 85 ms (kHitWindowMs, TM-22/TM-29, open Q-32). (b) early side here = late side vs TM-26 measured early > late.

## C2. Drop check judges the late side only, with a fixed 3 ms frame (kinds 5, 6, 8)
Home: play_chart.py:101 (FRAME_MS = 3.0), 1115.
`dropped = sends and raw_ms + FRAME_MS > chart_ms + sides[idx]`. Only the late edge is tested. A message read too early (before chart - side) is not tested at all (wait_until fires at or after at_ms, so only if the clock read skipped; no early check exists). FRAME_MS is a fixed number; TM-49/TM-50 show frames stretch to hundreds of ms under load, and the game's rate is a `framerate` setting (330 in the runs). No reference page backs 3.0 as "the" frame period (TM-12 mentions 0.003 s for a different thing: the audio-clock spread). It is also one more fixed gap beside PRESS_LEAD_MS 2, SP_END_LEAD_MS 15, SP_OUT_LAG_MS 15, ACT_LEAD_MARGIN_MS 6, CHORD_ORDER_GAP_MS 8, SCORE_READ_DELAY_S 5 ms, with no table tying them together.
Table (side = 85.72):
- raw_ms = chart+82.72: 85.72 > 85.72 false -> sent. raw_ms = chart+82.73: dropped.
- later order group of a message: only the first press is judged; a later group sent 8 ms after goes past the check unjudged (its own at_ms is later by after_ms).
- chord.declined with no lanes: sends False, dropped False, no press.
- exact 0 side impossible (min 48.15).
Split: drop edge (side - 3, late only) vs load check edge (side, both sides) vs TM-26 measured late edge (< 84.92).

## C3. Dropped chords are silent misses in the headline numbers (kind 6)
Home: play_chart.py:1150-1152, 1391-1393 (main summary), 995-1224.
Dropped messages are counted in result.dropped, not in `sent`. main() prints "Notes: sent, Hits: hit, Misses: sent-hit, Hit rate: hit/sent". Dropped and declined notes are not in Notes or Misses, so a run that drops 50 of 150 chords prints Hit rate 100.0% with Misses 0. result.dropped is never printed in main (it is only in the per-row line tag and RunResult).
Table: sent=100, hit=100, dropped=50 -> "Hit rate: 100.0%", "Misses: 0", "Done.".
Edge: sent=0 -> max(sent,1) gives "0.0%" and Done.

## C4. main() returns 0 and prints "Done." for every stop reason (kind 6, 2)
Home: play_chart.py:1382-1396.
Only STOP_NO_ENGINE gives a non-zero exit. "cap", "frozen", "lost", "interrupted", "stopped", "past_end" all fall through to the summary, `Done.`, return 0. Unplayed chords are in no count (not sent, not missed). Table: run stops "cap" at chord 40 of 400 with all 40 hit -> Notes 40, Hits 40, 100.0%, exit 0.
Also: `except OSError` (1221) turns any OSError raised inside the loop (including from `press` or read_score) into stop="lost" with a printed line, and the run returns normally.

## C5. Songs joined mid-song silently skip notes within 150 ms, and a run's skipped start is not counted (kinds 4, 6)
Home: play_chart.py:1063, 1098; live.first_note_index (START_LEAD_MS = 150, live.py:152-163).
Notes (presses) before the cursor are in no count. Also the test is on press `at_ms` of each message head, not on the chord time. The 150 ms is a fixed lead with a second home in live.py. After a jump back (1089-1106), `result = RunResult()` throws away all earlier rows and counts (the press log written by main then holds only post-restart rows; on_press already got the earlier rows, so a caller log and result.rows differ).
Table: clock at chart_ms-100 of chord k -> chord k is skipped, no row, no count. Clock exactly chart-150 (+lead) -> at_ms >= clock+150 kept (the loop uses `<` so equal is kept).

## C6. "hit" is any score rise over the whole message (kind 6, 4)
Home: play_chart.py:1145-1148; engine.pressed_input_hit (engine.py:46).
`hit = score_after > score_before`, read 5 ms after the reply. A chord with several gems (or several order groups in one message) is "hit" if any gem scored; a chord where only one of three gems landed counts as a full hit, a miss on the other gems is not seen. Anything else that raises the score within the 5 ms (a sustain, a previous chord's late score, an earlier SP bonus) counts as this chord's hit. A hit that scores 0 points (cannot, by the docstring) would show as a miss.
Table: 3-gem chord, 1 gem lands -> hit=1. Previous message's gem scoring late inside the 5 ms -> next chord's hit=1 with nothing sent? (dropped path sets score_after=score_before, so safe there; declined chord is never `hit`.)
Edge: SCORE_READ_DELAY_S 0.005 is a fixed sleep; under the load of TM-49 (frames at 2.2 fps) a score read at +5 ms can predate the frame that judged the press, giving a false miss; this fixed gap has the home here and `constants.INPUT_SETTLE_MS = 250` (a second number for the same kind of wait).

## C7. Keys route focuses the game window (kind 9)
Home: play_chart.py:1263-1281 (live_press), 1333-1340 (live_play), main() at 1382 calls live_play with no press, so the CLI path always uses it.
`focus_window(hwnd)` calls SetForegroundWindow before every press (input_driver.py:284-291) and the keys go via SendInput to the focused window. The default CLI run (`main`) takes this route. The file's comment says it needs the user's `allow key run`, but nothing in play_chart.py checks it; the check is in the `ch` MCP hook, not here. If find_game_window returns 0, a warning is printed (1338), `ch_hwnd` 0 passed on; focus_window(0) does nothing, and the keys are then sent to whatever window has focus (the user's), the run goes on.
Table: hwnd found -> window to the front every press (every 3-300 ms). hwnd 0 -> WARNING line, keys go to the foreground window. kit press route (press given): no window touched.

## C8. A missing process or odd values logged or defaulted while the run goes on (kind 2)
Homes:
- 1338: "WARNING: could not find Clone Hero window" and the run continues (see C7).
- 1186-1189: a kit reply that is not a dict, or has no `groups` list for a split message, or has fewer entries than groups: rows get no reply values, no message; the run goes on. `replies[k]` missing for k >= len(replies) is silently skipped.
- 1165-1167: a reply `groups` that is not a list falls back to the single reply for group 0 only; later groups' rows get nothing.
- 683-686: `score.get("path_source", {}).get("index")`: a missing path_source is accepted silently (comment says so); a mismatch is checked only when the key exists.
- 761: `score.get("activations", [])`; 817 and 836 likewise: a score file with no activations key is treated as no activations, with no message; the dump check at 888 then compares `[]` against dump activations and raises only if the dump has some.
- 838: `act.get("deact_ms", ...)`: a missing deact_ms falls back to the deact chord's `ms` or None; ActEnd(None) then leads to ValueError only if a lead needs it (lines 409-412 check stored_end_ms; end_ms None is not checked in `_solve_lead`).
- 790-791: a score file whose gems carry no `multiplier` (any gem missing it) silently turns off the order split (multipliers None). `all(... for gem in gems)` is checked over all gems, not just kept ones; one gem with no key -> none for the chord.
- 782-783: `chord.get("tick")` None: a chord with no tick is accepted; `tick in act_ticks` False, `squeezes.get(None)` None.
- 1128: `read_sp_end` None or end_s None -> NO_ACTIVATION_READ and the plan stands (documented), run continues with times unmoved.
Table: score file with no "tick" on chords -> no squeezes, activations, end handling; load succeeds and plays plain presses; no message.

## C9. Rules Python works out that Hydra or the reference already states (kind 3, 8)
- 304-332: the whole window formula (half-gap clamps, linear/square/scale/offset) is re-derived in Python from constants.py; Hydra uses a fixed 85 (TM-22). TM-23/TM-24 say Hydra has "no owner in src/" for the per-note window. Two homes (here and constants.py: CAP_FROM_GAP_MS, FLOOR_UP_TO_GAP_MS at 267-268 are a third form of the same clamps).
- 595-599 and 596: "the gem Hydra's FirstNote rule pays outside is note 0 of Chord::note_list(true)": the Python relies on the score file's note order instead of a dumped field. Reads the order of the JSON list as the rule.
- 736-744, 131: `_TOP_PAD_ORDER` (flag choice, FL-09) is re-stated in Python; comment says "unchecked until a run shows it" (row P22). Assumption not backed by the page as proven.
- 108-113, 488-527: order groups by multiplier (SC-38) work out the game's score order in Python from the score file's multiplier.
- 765, 899: sqout_tick `>= 0` / `< 0` as "has a squeeze-out" mirrors the dump's sentinel.
- 117: SQUEEZE_IN / SQUEEZE_OUT strings mirror SPSqueeze::type_name (C++).
- 83, 88, 95, 113: SP_END_LEAD_MS 15, SP_OUT_LAG_MS 15, ACT_LEAD_MARGIN_MS 6, CHORD_ORDER_GAP_MS 8, PRESS_LEAD_MS 2 are user-set numbers per comments citing plans; plan pages 2026-11-06 and 2026-11-07 are cited by path, one marked "(removed; git show ...)". I did not open the plans to check the quoted answers.

## C10. Hit window for a squeeze press uses `sides[chord.index]` of the squeeze chord, the activation lead uses the activation chord's side, with the same margin (kind 5)
Home: 368-371 (need_ms), 415 (`lead_ms > side_ms - ACT_LEAD_MARGIN_MS`), 588.
need_ms for SqOut: end_ms + 15 - chord.ms - side; for SqIn: chord.ms + 15 - end_ms - side. Both use a fixed 15 and side, while _refuse_far_presses later tests the press `at_ms` against `side` with `<=` (no margin). A press landing exactly on the side passes the load check but the drop check uses side - 3.
Table (side 85, chord.ms=1000, end 1085-... SqOut at lead exactly where need_ms = 0): need_ms > 0 strict, so need 0 gives no lead; the late gem is then pressed at chord.ms - (end_ms + 15) lead... accepted at exactly the side by `<=` in 647.
Split: load check edge (<= side) vs lead margin (side - 6) vs drop edge (side - 3).

## C11. retime_from_end moves presses but the load-time check is not run again (kinds 4, 5)
Home: 449-459, 1134-1138, compare 632-673.
_refuse_far_presses runs once at load on the planned times. After the game's real stored end is read, retime_from_end changes every press timed against that end by delta_ms (any size, the game's frame stamp). The moved presses are not rechecked against their windows (only the drop check at send time catches the late side). A delta that moves a press out of the early side is sent anyway.
Table: delta = +20 ms -> the squeeze-in press moves 20 ms later with no check; delta = -20 -> 20 ms earlier, no check on the early edge. delta 0 -> unchanged. Also `p.end_ms + delta_ms` raises TypeError if a press with end_act_index set has end_ms None (all such presses set end_ms in _timed_presses; the 616-617 path and 591 path do; the 628 path does).

## C12. Press log and run row "diff_ms" and lead are measured at the clock read, not the send (kind 6)
Home: 1086-1088, 1174, 1194.
Rows keep `clock_ms_at_press = raw_ms`, the read that triggered the send, and the printed `diff_ms` is raw_ms - chart_ms, not the lead the press used and not the time the game saw (TM-27 says the effective time is a frame later). A fake clock/press in tests cannot tell the difference. For a later order group the row keeps the first group's clock (comment says so; down_ms from the reply is used if present).
Table: lead_ms = 2 -> diff_ms about -2 (any value from -2 to clock granularity), not zero. Under a 130 ms stall (TM-50) the printed diff shows the late read, correct, but score_after/hit is read 5 ms later.

## C13. Clock choices: raw clock with max_fill_s 0 and est ignored (kind 5)
Home: 1061-1063, 1086-1088, 1344.
SongClock(engine.song_clock, max_fill_s=0.0) so est == raw; the code reads `raw_ms` only. wait_until polls with est but returns raw; since fill is 0 both agree. The conversion C.s_to_ms(raw) is used at three places (1063, 1098, 1133) and ms_to_s in prints; `sleep` granularity and `_SPIN_WITHIN_S` / `_WAKE_BEFORE_S` / `_MAX_SLEEP_S` are in live.py (not read). Song clock vs chart: the docstring says `ms` has no Offset or song.ini delay; no check in this file that the score file's ms matches the loaded song's clock zero (no song id or chart hash in the checks; only path_index vs path_source).
Split: none found beyond that; open: whether the score JSON is of the song the game is playing is not verified anywhere in this file (kinds 1, 6). Table: a score JSON for song A played against song B: first_note_index starts by clock, presses go out, every chord is a miss or hit by chance; run reports its hit rate with no refusal.

## C14. Code that runs is not the code in git (kind 1)
Nothing in this file compares the running file to git, and the live path imports `tools.ch_probe.*` and `tools.ch_mcp.kit_tools` (with_axes, 946) from the repo path by `sys.path.insert(0, _REPO_ROOT)`. The `ch_reload` note in the server instructions says merged code goes live on reload; this file has no version stamp in its log or press rows. A run's press log does not record the commit or the constants (PRESS_LEAD_MS etc.) it ran with. By reading only; I did not read the worker loader.
Edge: a worker started from a different checkout (e.g. a worktree) imports that checkout's constants.

## C15. Name-keyed table `_KEY_FOR_PAD` (kind 4)
Home: 157-166, 778, 722-723.
Keys are (color, cymbal). Red/cymbal True, Kick/cymbal True, or any color string the dump may write that is not in the table raises KeyError at load (loud). A ghost/accent maps to the plain pad. `("Orange"...)`/5-lane colors are absent. A chord whose `notes` list is empty yields lanes [] and `[] or [every]`... `_order_groups` returns `[every]` = `[[]]` for an empty chord: one press with no keys, `checked` per rules, `sends` True (only declined+empty skips), so a non-declined chord with no gems calls `press([], axes=[])`.
Table: notes [] and not declined -> `press([], [])` sent, hit will be False -> counted as sent and MISS (if the reply is a no-op). Notes with two gems of the same pad (two Green) are fine for non-flagged chords; for an activation chord, _flagged_gem raises.
Also: kick_only declined chord with 2 kicks sends 1 (by `[:1]`); chart then has a note whose press is absent (by design of the mode), counted as declined, not miss.

## C16. Empty and edge inputs (kinds 2, 4)
- `chords == []`: main prints "has no chords to play." and returns 0 (1375-1377); live_play/play_chords: press_plan([]) -> [], first_note_index([]) -> 0, cursor >= 0 -> "past_end" return; window_sides([]) -> {}.
- One chord: window_sides gives half_gap None both sides: 2*85 = 170 -> TM-22's 85.72.
- Two chords at the same ms (gap 0): half_gap_ms(0) is treated as "no neighbour" (85 ms back clamp), per the docstring's TM-19 reference; I did not re-read TM-18/19 text fully (trimmed output). A negative gap (list not in time order) gives max(gap/2, 37.5) = 37.5, silently accepted; `chords` order is trusted ("in time order") with no check.
- `chord["ms"]` as a float from JSON; ints accepted.
- `_dump_path` (680): `json.load(f)["paths"][path_index]` raises KeyError/IndexError for a bad index (loud, not a friendly message).
- `declined_chord_indexes` (701): `act["skipped_fill_ticks"]` KeyError if the dump lacks it (loud).
- `load_dump_notes` (935) calls load_dump_chords(path) with no dump, so a squeeze-out score file raises there (documented at 925-931 for load_score_times only).

## C17. Test fake vs real agent (kind 7), not reached
Home: play_chords' `press` contract, 1019-1023, 1165-1189. The real kit reply shape is in tools/ch_mcp/kit_tools.py (not read). The code accepts a non-dict reply (rows then carry only the fields play_chords itself sets, so `down_ms`, `down_frame` etc. are blank in the press log). A fake that returns None would give a log that differs from the real agent's in every reply column, and nothing here would flag it. Open: read kit_tools.py and tests/test_play_chart*.py to compare.

## Splits (each its own line)
1. Window per note: formula here (304-332) vs Hydra's fixed 85 ms (TM-22, Q-32 open).
2. Window sides: equal here (321) vs early reaches farther than late (TM-26).
3. Late edge: load check `<= side` (647) vs drop check `raw+3 > side` (1115) vs lead margin `side - 6` (415) vs SP lead 15 / lag 15 (368, 371).
4. The frame period: FRAME_MS 3.0 (101) vs the game's `framerate` setting, 330 fps in the runs (TM-49), and the 5 ms score delay (147) vs INPUT_SETTLE_MS 250.
5. Start skip: START_LEAD_MS 150 in live.py vs nothing else here (single home, but the file inherits it without saying).
6. What "good" means: `hit` (any score rise) vs "all gems of the chord landed" vs the summary's Notes/Misses excluding dropped, declined, skipped-at-start and unplayed chords.

## Not reached
kit_tools.py, the test files for play_chart, live.py's polling constants, the plans quoted for the numbers, tools/replay_json.cpp field names (end_scale_*, steady_ms_*, nominal_deact_tick), the 03-timing TM-18/TM-19 full text, and any check of git vs running code.
