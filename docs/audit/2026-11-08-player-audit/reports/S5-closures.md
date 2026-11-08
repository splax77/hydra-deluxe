# S5 closure check, player audit

Checker: a05b088550a77d825. Read-only. Code read at main (fd93c684).

## Exit 3

L44: PASS. The quote is at docs/superpowers/plans/2026-11-08-replay-press-schedule.md line 29, word for word: "why is the python still doing its own thing? the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do", then "yes revise the plan". It names both sides (Python planning, Hydra's schedule) and picks Hydra. Code agrees: tools/replay_schedule.h says the player reads one file; no Python reads the schedule. Two cautions. (1) The words exist in the repo only inside that plan, quoted by the plan's author "from this revision's brief". No card or ruling file holds them (grepped docs/sessions, docs/agents/rulings, fix-decisions: nothing). The main session should still confirm them against its own session record. (2) "yes revise the plan" approves revising, not building TP1-TP4. The ruling decides direction only.

L07: PASS in part, REOPEN for one split. The quote covers "Python and C++ both plan the squeeze lead; Python's goes". It does not cover the second split in L07's own row: Python lead 65.28 vs C++ lead 78.5 for the same input. That is a different formula (minimal lead plus margin, vs equal room on both presses). The replay plan lists "how a squeeze is split between its two presses" as its own Question 1, with three sides (equal room, the user's "-45/+45", minimal lead). I found no quoted answer to it anywhere (plan, cards, rulings, fix-decisions). TC2 built the equal-room side and the merge says so, and plan line 192 says "questions 1 to 4 as answered", but no user words are in the repo. The user's one nearby quote ("get ur freak on is 84.3ms so you have tons of room if you just aim for -45/+45") leans to a third option. So the lead formula needs a question, or a quoted answer. The "Python raises when anchor is not the activation, C++ plans it" part is covered by line 29.

L45: PASS for the five equal constants (press lead, end lead, out lag, order gap, frame), with a flag on the margin. The quote covers "one owner for the Star Power timing numbers". The margin is not undecided as exits.md says (see below).

L47: REOPEN. The quote says "starpower planning". The far-press check (_refuse_far_presses, TM-50: a press reaching another note on its pad) and the chord order groups are about general press rules, not Star Power planning. Nothing the user typed names them. They reach the plan only through the plan author's owner table (line 133-134) and delete list (line 115). Also L47's own splits include behaviours the user would see: Python raises at the first bad far press, C++ writes one refusal per chord; C++ has fallback rows, Python none; Python raises when a late gem pays less than an inside gem, C++ writes a refusal. Those are choices of behaviour, and they fall under Q12 (silent skips) only in part. Exit 3 needs the quote to name both values; here it names neither. Keep L47's copies split as a question (or add a plan question the user answers).

Is the room margin, plus the late-judgement edges, the only part left undecided? No, in two directions.
- Too many: the room margin is already ruled. docs/superpowers/plans/2026-11-08-player-freak-fixes.md line 84-86, question 2 has three sides: (a) 3.0 ms, (b) 0, (c) keep 6, recommended (a). The replay plan (line 33 and premise row P38) records the user's "Yes, start it" approving every recommended answer, and C++ kRoomMarginMs is 3.0 for that reason. So Q9 "3 ms or 6 ms" asks something the user answered, at least by that second-hand quote. Either close the margin by exit 3 with that quote (after the main session confirms the words against its own record, since the repo holds them only in the plan), or keep Q9 and tell the user the history. Not a leak, but exits.md's reason for Q9 ("the ruling does not pick the numbers") is wrong for this number. Note the 6 in Python stood for two frames, and C++ keeps those two frames separately as kJudgeLateMs 6, so the real question is whether 3 plus 6 is the intended total, which the freak-fixes Q2 text does say.
- Too few: the lead formula (above) and L47's behaviours.
The late-judgement edges (Q8, 0/1/2 frames) are rightly a question: no block gives two frames (SP-47 says one), and the ruling does not touch them.

## Exit 4

L58: PASS. KIT_BINDINGS (tools/ch_mcp/kit_tools.py:161-172, not tools/ch_probe; the cited name is right, the folder in the ledger's short form is not) gives 0:1 Red, 1:2 Yellow, 2:3 Blue, 3:0 Green, 4:37, 5:38, 6:39, 7:4 Orange, 18:16 Start, 19:6 Select. IN-82 (07-internals.md lines 837-839) lists the same ten ids, kick 7 to action 4, and says so ("the action names are those of KIT_BINDINGS"). Both answer "which action id does each kit element drive", with the same ids. Agree.

L79: PASS. recorder_link.start (recorder_link.py:80-113) records the failure line and re-raises. Its only non-test caller is game_tools.py:1223-1226, which catches Exception, appends start_failed_line and goes on ("a run never fails for its recording"). Start raises so the caller can decide; the caller decides to carry on. Agree.

L63: PASS, with a gap. Code side holds: kit.ts sets and restores ignoreInputWhenAppNotInFocus (lines 179-183, 860); no kit route code touches a window. The 3.033 ms figure rests on the ls-m0-run1 press-1.csv, which I could not open (scratch run files; the repo cites them only in verifier reports). This row is "a check that passed", with no two sides, so exit 4's one-sentence form is thin: the kit route answers "does input need the game window in front", and it says no. The ledger row also lists two unread edges (teardown fails and leaves the setting changed; ch_stop restore not read). Those are not a split, but they are unchecked. Agree with PASS; the main session should not cite the run files as evidence in a permanent doc.

L47 top-pad: PASS on the order, with one edge exits.md misses. Python _TOP_PAD_ORDER (play_chart.py:131) is Green, Blue, Yellow, Red. Chord::activation_note (model.cpp:307-316) is Green, Blue, Yellow, Red, then Kick. The pad order agrees. They differ for a chord holding only a kick: Python _flagged_gem returns None, C++ returns the kick. A kick-only chord has one gem, so no ordering can differ, and Python handles kick-only declines on a separate branch (play_chart.py ~720-723). Agree it is not a value split. Wording: say "the same order; the kick-only edge has one gem". Having two copies is L44's matter, and L44 PASSes.

## Exit 2 and Open tables

Closure in disguise: none. One question already decided: Q9 (above). Nothing in the Exit 2 rows is closed by its own wording.

Other points:
- Every ledger id L01-L85 appears in exits.md. Good.
- "Open" rows L67-L73, L83, L84 have no verifier and so no exit. The method says an audit with a finding that has no exit is not finished. They are correctly not closed, but the audit cannot be called complete until a verifier gives each an exit.
- Open rows L10, L32, L40 also sit in Q4, Q14, Q1. Fine as long as the question text says the live run is pending.

## One line per closure

- L44: PASS (quote present at plan line 29, names Python planning vs Hydra's schedule; second-hand in the repo).
- L07: PASS for owner; REOPEN for the lead formula (65.28 vs 78.5 is plan Q1, no quoted answer found).
- L45: PASS for the five equal constants; margin is already ruled by freak-fixes Q2 (3.0 over 0 and 6), so Q9 is redundant or should cite it.
- L47: REOPEN (quote is about Star Power planning; far-press check, order groups and raise-vs-refusal behaviours are not named).
- L58: PASS. L79: PASS. L63: PASS (run data unverified by me).
- L47 top-pad: PASS (same order; kick-only chord has one gem).
