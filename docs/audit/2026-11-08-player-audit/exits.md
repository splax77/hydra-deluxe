# Player audit: each finding's exit (S4)

Session 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e, 2026-11-08. The main session picked every exit below, following `docs/agents/audit-method.md`. Row ids are those in `ledger.md`. The agent reports behind each row are in `reports/`.

The audit changed no code, so no finding leaves as exit 1 (fixed) here. A finding whose fix is plain still leaves as a question, because each fix changes what the user sees: a refusal, a failed check, or a different count. The questions are in `questions.md`, numbered Q1 onwards.

## Exit 3: covered by a ruling

**L44, the player still plans Star Power itself.** The user's words: "why is the python still doing its own thing? the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do", then "yes revise the plan" (`docs/superpowers/plans/2026-11-08-replay-press-schedule.md` line 29). Both sides are named: the Python's own planning, and Hydra's schedule. The ruling picks Hydra's schedule. The work is the replay plan's TP1 to TP4, which are not built.

**L45, the five timing constants that agree in both copies** (press lead, SP end lead, SP out lag, order gap, frame). The same ruling as L44 says Hydra's copy is the one the player reads. The values agree today, so no value is picked.

**L45's room margin, Python 6.0 against C++ 3.0.** Question 2 of `docs/superpowers/plans/2026-11-08-player-freak-fixes.md` (line 85) offered "(a) 3.0 ms (one slow frame); (b) 0 ...; (c) keep 6", recommended (a). The user approved every recommended answer of that plan with "Yes, start it", recorded as row P38 of `docs/superpowers/plans/2026-11-08-replay-press-schedule.md` (line 343, and line 33). The question names both values. The Python still has 6.0 only because that plan's T2 never merged (branch `claude/pl-freak-t2`, held), and under L44 the Python's copy goes away.

The closure checker reopened L07 and L47: the ruling picks who owns the planning, but not the lead formula (L07) or the press rules' behaviour (L47). Both are now Q9.

### Second round of closures (Q10 not sent, and Q11 and Q15 follow-ups)

**L06, the fixed 3 ms frame.** Question 1 of `docs/superpowers/plans/2026-11-08-player-freak-fixes.md` (line 84) offered "(a) 3.0 ms, one frame at TM-49's about 330 a second ...; (b) 3.03 (1000/330); (c) read it live from the kit agent's frame rate each run", recommended (a). The user approved every recommended answer of that plan with "Yes, start it", recorded in `docs/superpowers/plans/2026-11-08-replay-press-schedule.md` line 33. The question names the fixed value and the live read.

**L46, the hit-window copies.** The copies of the window follow L44's ruling: Hydra's schedule is the one the player reads. Hydra's fixed 85 ms against the game's dynamic window is the user's "hydra does not currently account for the dynamic hit window, although that is a good idea for a new feature" (`docs/superpowers/plans/2026-11-08-replay-press-schedule.md` line 31). The quote names both: Hydra's fixed window, and the game's dynamic one.

**L12, retiming with no recheck of the window.** Question 3 of the freak-fixes plan ruled what happens when the read end leaves a squeeze-out's late gem no room: "the gem joins the last inside group and the row says `retimed=inside`", under "Yes, start it" (`2026-11-08-replay-press-schedule.md` line 33). The replay plan's TP2 owns `retime_from_end` and re-makes that room check, with tests at both edges (lines 146 and 230 to 238). Not built yet.

**L75's Ready level.** `sp_compare.at_ready_level` decides from the game's live meter whether Star Power can be activated, with its own copy of Hydra's numbers (`READY_FRACTION = SP_ACTIVATION_BARS / SP_CAP_BARS`, `sp_compare.py:122` and `:288-292`), and it differs from the game's rounded half for an odd `sp_max` (SP-16). Hydra's engine already answers "is this path ready to activate here" (`kSpActivationBars` in `src/core/timing.h:51`, the path's `sp_ready_ms` in `src/search/engine.cpp`). L44's ruling names both sides: "why is the python still doing its own thing? the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do" (`docs/superpowers/plans/2026-11-08-replay-press-schedule.md` line 29), and D319 item 2 (Q7, "Judge vs Hydra (Recommended)") has the Star Power checks judge against Hydra. When the main session first wrote this up as a new question, the user wrote: "why are you still trying to invent your own rules? hydra already knows how starpower works" (2026-11-08, session 274bd9dd). So the Python test goes, and the judge compares the game's state with Hydra's ready point. `hydra_replay` does not write that point today, so the fix plan adds it.

The fresh closure checker reopened this: the Python test answers "was the game's meter at the activation level in this row", a reading of the game, and Hydra's `sp_ready_ms` answers "when does the path hold two bars", from the chart. Hydra never sees `sp_max`. L44's quote is about planning, and D319's Q7 list does not name the Ready level. So the closure waits for the user's word on one thing: the judge takes "when" from Hydra and "is the meter ready" from the game's own stored threshold (`+0x1B8`, SP-16), with no Python copy of either.

**L62, the 43.9 s window event.** Q13 asked about the keys route and the window coming to the front, and its option (a) said "Finding the 43.9 s cause waits for a run with a window-event log, on your go." The user: "i care about the runs working, not the window coming into focus" (D320 item 4). The words name the window event and say it is not pursued. Checked by a fresh agent: PASS. Its caveat: the event lined up with a 30 ms stall and one late press, and stalls are a run matter (L02), which stays open.

### Third round: the nine rows no verifier had opened (L67 to L73, L83, L84)

Two fresh verifiers (session 274bd9dd, task ids player-audit-verify-A and -B) read every claim. Their verdicts are summed up here; the rows themselves are unchanged in `ledger.md`. A fresh closure checker has since checked every exit below; "What the third closure check returned", after this section, gives its verdicts.

**Exit 3, covered by a ruling:**

- **L67's fakes.** The fake replies that never send `groups`, and the tests that never run a built agent, are D318 item 1 ("the tests also run the built file") and item 2 ("the test fakes send the real reply shape"). The recorder's fake, built from the parser's own offset constants (`test_machine_recorder.py:34-73`), is a test that cannot fail on a wrong offset; its fix changes nothing anyone sees, and it goes in the fix plan's list.
- **L68's copies of Hydra's rules.** `_TOP_PAD_ORDER` and the decline modes go by D318 item 3. The strings `SQUEEZE_IN` and `SQUEEZE_OUT` (`play_chart.py:117-118`) copy `SPSqueeze::type_name`, and the five timing constants copy the C++ ones; both go by L44's ruling and L45. The Python also hard-codes the squeeze-out's late gem as the chord's first note (`play_chart.py:595-599`), while Hydra's rules file can set `SqOutRule::WholeChord` (`src/core/rules.h:22`). That is Star Power planning the Python does itself, so L44's ruling ("hand off all the starpower planning to hydra which already knows what to do") covers it: `hydra_replay` writes which gems the late press holds, and the player reads it. `hydra_replay` does not write the rule today.
- **L70, the kit's reply is its own writes.** A reply that is not a dict, or lacks the down and up values, is D318 item 2 (stop on a reply missing what was asked). A press the game ignored shows up as a failed chord under D319 item 1 (each chord's score step against Hydra's).

**Exit 4, not a problem:**

- **L67's menu, bot-run and SP-watch fakes.** The menu fake matches `menu.ts`'s reply (`test_menu_tools.py:35-56`), the `PATCHED` fake has `launch_core.ts`'s keys (`test_bot_run.py:539-541`), and the drift check the SP-watch fake skips is tested in `test_process.py:126-180`.
- **L68's first-note gem.** `play_chart.py:595-599` takes gem 0, and Hydra's `Chord::note_list(true)` sorts so that gem 0 is the squeeze-out note `category_scores` uses (`src/core/model.cpp:156-174`, `src/core/scoring.cpp:58`). They agree under the default rule; the rules-file case is above.
- **L72's engine search.** `find_run_engine` (`game_tools.py:184-198`) answers "which normal-mode engine does the player play on"; `sp_watch.find_engine_live` answers "which engine is playing", over every window pattern. Inside `ch_run_chart` the watch is handed the player's engine (`game_tools.py:1075`). The row's blank-cell claim is struck (the watch writes "nan", which loads), and the lost press log is retried once at the end of the run (`game_tools.py:1485-1499`).
- **L73's timing notes.** The spin thresholds, the 250 ms clock sample and the three song-time reads each serve one tool, and no two are compared in code. That the engine's clock copies `GameManager.songTime` is TM-10, labelled Unknown.
- **L83, `edge_probe`.** Its file holds two leads that answer two questions, the asked lead (`probe`) and the measured one (`clock_ms_at_press` minus `chart_ms`); the third lives in memory only and equals the asked one by construction (`test_edge_probe.py:148`). The KeyError cannot happen: `press_plan` gives every chord at least one press (`play_chart.py:503-504`).
- **L84, `find_window_edge` and `overlay_logs`.** The edge finder's tie at a midpoint (`analysis.py:98-109`) matters only to the old probes that import it, and they go under D322 item 3 if the user confirms the list. `overlay_logs.py` is the live player's column owner, not an old script.

**For the Q18 list:** `edge_probe.py` and `overlay_logs.py` are live (`edge_probe` is imported by `game_tools.py`; `overlay_logs` by `game_tools.py`, `play_chart.py` and others). `analysis.py`, `active_probe.py` and `passive_probe.py`, with their tests, are old. `walk_edges.py` is old except for `SongClock`, which the live player imports from it (`play_chart.py:75`, pinned by `test_play_chart.py:126-130`), so `SongClock` moves before the file can go.

**Questions (exit 2):** L69's empty chord list (Q20), L71's trace lines (Q21), with L75's Ready level (Q19).

### What the third closure check returned

A fresh checker (session 0963d03d, task id player-audit-closures-3) read every exit above against the code and the cited rulings. The main session then grepped each exit 3 quote in its cited file, and every one is there.

It passed L67's reply fakes and built-file tests, L68's copies, L70, L67's menu, bot-run and SP-watch fakes, L68's first-note gem, L72, L73, L83 and L84.

It reopened L67's recorder fake. No ruling names it: D318 item 2 is about the agent's reply shape, and the recorder reads a Windows table, not a reply. "Its fix changes nothing anyone sees" is the code-only reason `audit-method.md` says is not an exit. It leaves by exit 1 once the fix plan's task lands.

It found one more split, inside Hydra. `tools/replay_schedule.cpp:861` sets the squeeze-out's late press to gem 0 and never reads the rules file, while `src/core/scoring.cpp:58` takes SP from every note of the chord under `SqOutRule::WholeChord`. L44's ruling settles who owns the choice. How the player presses a squeeze-out under `WholeChord` has no ruling, so it goes to the fix plan.

Two smaller notes: `sp_watch.py:9` says the writer leaves a blank cell, but it writes "nan"; and this section's Q18 note listed `server.py` as an `edge_probe` importer, which it is not (fixed above).

### Exit 4, second round

**L77, `machine_report` crashing on `statistics.quantiles`.** On this machine's Python 3.14.5, `statistics.quantiles` on one point returns a list, also in the form the report uses (`n=100, method="inclusive"`, 99 values), and only an empty list raises "must have at least one data point" (run by the main session and by the closure checker, 2026-11-08). The call at `machine_report.py:487` runs only after `if not pairs: return` at `:483-484`, so it never gets an empty list. The `_share` division (`:242-243`) divides by the window's span, which is zero only if two samples carry the same wall stamp; `window_of` (`:216-229`) does not forbid that, but real clock stamps do not repeat. The closure rests on the Python in use being 3.13 or later (3.14.5 for `py`, 3.14.7 for `python`). Checked by a fresh agent: PASS. Not yet checked by a fresh agent.

**L56, the far-press refusal on every pad.** TM-50 on `docs/clone-hero/03-timing.md` is now proven in the game's code (commit `fb1be3cd`): the press handler gives a press to the earliest unjudged note on its pad in the window, on every pad. `far_press_refusal` (`tools/replay_schedule.cpp:392`) answers "which earlier or later same-pad note would this press reach", and the game's handler answers "which note this press hits". They now agree on every pad. The second closure check reopened two branches of the refusal that are stricter than the handler (Q16, below).

**L18, two clocks.** The player reads the raw song clock to time its presses (`play_chart.py:1344`, `max_fill_s=0.0`). The walk-edges probe reads a filled estimate to step through window edges (`constants.py:315-318`). They answer different questions for different tools.

**L78, two settings.ini readers.** `game_files.get_game_setting` answers "what is this `[game]` setting"; `machine_report._settings_lines` answers "which lines in the file mention this key", so it can show framerate and vsync from `[video]` (`machine_report.py:49-52`).

**L80, the recorder falling behind.** A late sample skips ticks (`machine_recorder.py:430-432`), and the report prints the median gap, the worst gap and the count over `GAP_MS` (`machine_report.py:280-284`). The lag is reported, not silent.

L04 and L05 left by Q8's answer, not as closures.

### What the second closure check returned

A fresh checker (session 274bd9dd) passed L06, L46's fixed-against-dynamic window split, L56's page split, L18's two clocks, L78 and L80. It reopened three parts, now questions: L56's two refusal branches that are stricter than the game (Q16), L12's early edge (Q17; the late edge stays closed by freak-fixes question 3), and L46's probe copy in `analysis.py` (Q18). L12's other listed split, the frame model, left by Q8's answer (one frame, one constant both sides read). It also noted that "Yes, start it" is quoted only in plan text, because session 6341c618 has no card yet.

**L18's other half, the stop check.** `live.StoppedCheck` (`live.py:61-99`) answers "has the song stopped or gone back" (5 s with no clock change, 1 s back). Whether one press is late is answered per press by the drop check, which Q8 sets to one frame. Checked by a fresh agent (the doc reviewer of this range, 2026-11-08), who read `live.py:55-100` and the drop check at `play_chart.py:1115` (`raw_ms + FRAME_MS > chord.chart_ms + sides[chord.index]`) and agreed: a stall shorter than 5 s passes `StoppedCheck` and every chord it makes late is dropped by the drop check; a stall longer than 5 s raises `ClockFrozen`; a jump back of more than 1 s raises `ClockJumpedBack` and the run re-syncs.

## Exit 4: not a problem

**L58, the kit's action ids.** `kit_tools.py:161-172` `KIT_BINDINGS` and block IN-82 on `docs/clone-hero/07-internals.md` lines 837-839 give the same ids for the same actions. The code and the page answer one question the same way.

**L79, recorder_link re-raising.** `recorder_link.start` raises, and its one caller, `game_tools.py:1221-1226`, catches it and adds a reply line. The finder thought the raise went unhandled; the caller handles it.

**L63, the kit route needs no focus.** This is a check that passed. The kit route touches no window, and frames ran at 3.033 ms with the game behind (`ls-m0-run1` `press-1.csv`).

The closure checker (`reports/S5-closures.md`) passed L58, L79, L63 and this one.

**L47's top-pad part.** `play_chart.py:131` `_TOP_PAD_ORDER` and `src/core/model.cpp:307-316` `activation_note` both order Green, Blue, Yellow, Red. They are two copies that give the same answer. Having two copies is L44's matter.

## Exit 2: a question to the user

| Rows | Question |
|---|---|
| L01, L36, L37, L38, L39, L40 | Q1 refuse a built agent or exe that does not match its source |
| L01 (player side), L14, L65 | Q2 the player refuses a reply or file that is missing what it needs |
| L08, L59 | Q3 the three ways of declining a chord |
| L09, L10, L19 | Q4 which gem of a split chord goes first, and the gaps |
| L16, L17, L20, L21 | Q5 check that the run plays the song, path and settings the files are for |
| L22, L23, L28, L29, L30 | Q6 what counts as a hit, and when a run passes |
| L24, L25, L26, L27 | Q7 judging rules that cannot fail or excuse a miss |
| L48, L05, L04 | Q8 how late a press may be sent: 0, 1 or 2 frames (L04 joined from Q10) |
| L07, L47 | Q9 the activation lead formula (Python 65.28 ms against C++ 78.5 ms in one example), and how the press rules refuse or fall back |
| L11, L57, L60 | Q11 rules that rest on unproven or contradicting game facts (L56 left by exit 4; Q10 was never sent, its rows are above) |
| L49, L50, L51, L52, L53, L54, L55, L66 | Q12 the schedule's silent skips and untested branches, before TP1 reads it |
| L61, L62, L64 | Q13 the keys route and the 43.9 s window event |
| L31, L32, L34, L35, L74, L41, L42, L43 | Q14 report and launch lines that can say the wrong thing |
| L13, L15, L33, L75 (the `_ms` copy only), L76, L81, L82, L85 | Q15 the smaller items, as one list of fixes (L12 by exit 3; L18, L78, L80 by exit 4) |
| L56 (two branches) | Q16 the far-press refusal is stricter than the game |
| L12 (early edge) | Q17 a press moved earlier by the read end is never checked against its window |
| L46 (probe copy) | Q18 the probes keep a third copy of the window formula |

## Open: needs a verifier or a live run

These rows have no verifier verdict yet, or rest on a run nobody has done. None is closed. Each names what would settle it.

| Row | What it needs |
|---|---|
| L02 | the 4 Freak misses not caused by the stale agent (chords 153 and 154, 803, 1144): a run on a rebuilt `kit.js`, to see whether the stalls repeat |
| L03 | whether one missed gem in a Star Power phrase cancels its award (the rule is inferred from `ls-m2-run1`; no page block says it): a run with only the kick held back, or the game's miss handler read |
| L75 (Ready level) | Q19, answered (D323 item 1) |
| L69 (empty chord list) | Q20, answered (D323 item 2) |
| L71 (not-hooked lines) | Q21, answered (D323 item 3) |
| L10, L32, L40 | a run on the rebuilt `kit.js` with split chords under load; a real long `ch_sp_compare` reply; a `hydra_replay` run to diff its output against the 11:14 exe's |

Q16 to Q18 answered on 2026-11-08 (`questions.md` "## Answers"). Q17 leaves L12's early edge with no rule: the plan includes the judging frame once TP1 lands.
