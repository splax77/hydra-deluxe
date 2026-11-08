# Player audit: each finding's exit (S4)

Session 31e4f22f-bf4a-4106-bbf3-bcd268d0d75e, 2026-11-08. The main session picked every exit below, following `docs/agents/audit-method.md`. Row ids are those in `ledger.md`. The agent reports behind each row are in `reports/`.

The audit changed no code, so no finding leaves as exit 1 (fixed) here. A finding whose fix is plain still leaves as a question, because each fix changes what the user sees: a refusal, a failed check, or a different count. The questions are in `questions.md`, numbered Q1 onwards.

## Exit 3: covered by a ruling

**L44, the player still plans Star Power itself.** The user's words: "why is the python still doing its own thing? the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do", then "yes revise the plan" (`docs/superpowers/plans/2026-11-08-replay-press-schedule.md` line 29). Both sides are named: the Python's own planning, and Hydra's schedule. The ruling picks Hydra's schedule. The work is the replay plan's TP1 to TP4, which are not built.

**L45, the five timing constants that agree in both copies** (press lead, SP end lead, SP out lag, order gap, frame). The same ruling as L44 says Hydra's copy is the one the player reads. The values agree today, so no value is picked.

**L45's room margin, Python 6.0 against C++ 3.0.** Question 2 of `docs/superpowers/plans/2026-11-08-player-freak-fixes.md` (line 85) offered "(a) 3.0 ms (one slow frame); (b) 0 ...; (c) keep 6", recommended (a). The user approved every recommended answer of that plan with "Yes, start it", recorded as row P38 of `docs/superpowers/plans/2026-11-08-replay-press-schedule.md` (line 343, and line 33). The question names both values. The Python still has 6.0 only because that plan's T2 never merged (branch `claude/pl-freak-t2`, held), and under L44 the Python's copy goes away.

The closure checker reopened L07 and L47: the ruling picks who owns the planning, but not the lead formula (L07) or the press rules' behaviour (L47). Both are now Q9.

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
| L48, L05 | Q8 how late a press may be sent: 0, 1 or 2 frames |
| L07, L47 | Q9 the activation lead formula (Python 65.28 ms against C++ 78.5 ms in one example), and how the press rules refuse or fall back |
| L04, L06, L12, L46 | Q10 the hit window's two sides, the fixed 3 ms frame, and retiming |
| L11, L56, L57, L60 | Q11 rules that rest on unproven or contradicting game facts |
| L49, L50, L51, L52, L53, L54, L55, L66 | Q12 the schedule's silent skips and untested branches, before TP1 reads it |
| L61, L62, L64 | Q13 the keys route and the 43.9 s window event |
| L31, L32, L34, L35, L74, L41, L42, L43 | Q14 report and launch lines that can say the wrong thing |
| L13, L15, L18, L33, L75, L76, L78, L80, L81, L82, L85 | Q15 the smaller items, as one list of fixes |

## Open: needs a verifier or a live run

These rows have no verifier verdict yet, or rest on a run nobody has done. None is closed. Each names what would settle it.

| Row | What it needs |
|---|---|
| L02 | the 4 Freak misses not caused by the stale agent (chords 153 and 154, 803, 1144): a run on a rebuilt `kit.js`, to see whether the stalls repeat |
| L03 | whether one missed gem in a Star Power phrase cancels its award (the rule is inferred from `ls-m2-run1`; no page block says it): a run with only the kick held back, or the game's miss handler read |
| L62 | who brought the window forward at 43.9 s: a run with a window-event log |
| L67, L68, L69, L70, L71, L72, L73, L83, L84 | no verifier opened them (each row's Source ids) |
| L77 | the `statistics.quantiles` part rests on the Python docs from memory, not a run: one call in Python 3.14 |
| L10, L32, L40 | a run on the rebuilt `kit.js` with split chords under load; a real long `ch_sp_compare` reply; a `hydra_replay` run to diff its output against the 11:14 exe's |
