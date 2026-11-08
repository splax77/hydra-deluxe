# Player audit ledger (S4 compile)

Compiled from S1-calibrate, F1 to F4, W2A, W2B (finders) and V1 to V4 (verifiers). No exit is picked here. A row's Verdict is the verifier's wording. Paths: R-M2 = `hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1`, R-M0 = `...\2026-11-08\ls-m0-run1`, R-F1 = `...\2026-11-07\ls-f1-run1`. "main" = main checkout at fd93c684.

Id key. Finder ids are `<report>-<item>`: `S1-C1` is S1-calibrate item C1. `F1-C1` is F1 item C1. `F2-J1`, `F2-S1a` (S1 sub-letter a), `F2-S2`. `F3-1` to `F3-5d` are F3's family 3 items. `F3.F4-n` is F3's family-4 item n. `F3.F8-n` is F3's family-8 table row n. `F4-F9-n` is F4's focus item; `F4-T1` to `F4-T5` its timing items. `W2A-n`, `W2A-F` (focus section), `W2A-K` (test-fake section). `W2B-A1` and so on. Handoff ids `H1`, `H2` (V2/V4). Verifier-found extras are `V1-N1`, `V1-N2`, `V2-N1` to `V2-N3`, `V4-NEW`. "(proven)" below always names a run file.

Verifier abbreviations: V1 = player-audit-S3-V1, V2 = S3-V2, V3 = S3-V3, V4 = S3-V4.

---

# Section A: affects what is pressed or whether notes are hit

### L01 The built kit.js is older than kit.ts, so split chords press only their first group (LS-M2: 14 gems lost)
- **Source ids:** F1-C1, F2-J5, F4-T4 (the kit.js part; its launch.js and menuhooks.js parts are in L37), S1-C8 (the reply-column part only; the rest is L14), H1, V4-NEW, V1 section 1, V2 H1. Also F1-C6 note on the stale-kit case (tests, see L65) and F4-T1 split 6.
- **Verdict:** V1 CONFIRMED (F1-C1 / F2-J5 / S1-C8 reply columns). V2 CONFIRMED (H1; T4 CONFIRMED). V4 states it as NEW, serious, by reading plus file times, "not proven by a press row" (V1 and V2 later proved it from score steps).
- **Homes:** `tools/ch_mcp/agent/kit.js` (main, git-ignored, built 2026-11-06 21:31) against `tools/ch_mcp/agent/kit.ts` (commit 2709008f, 11-07 23:11, "Player limits T3"). `tools/ch_mcp/kit_tools.py:318-337, 601-630` (sends `then`, returns the agent reply unchanged). `tools/ch_probe/experiments/play_chart.py:1120-1122` (one message with `then=`), `:1165-1167` (uses `groups` only if the reply has a list), `:1186-1189` (missing entry skipped silently), `:1145-1148` (one score read, one `hit` for all rows of the chord). kit.js lines 3748-3877 read `m.elements` only.
- **Evidence:** Counts of `after_ms`, `groupsOf`, `"then"`, `groups`: kit.js 0, 0, 0, 0; kit.ts 6, 2, 3, 19 (V1 count). Proven by R-M2 `press-2.csv`: 2150 rows for 2137 chords. 12 chord indexes (3, 7, 10, 292, 609, 866, 1064, 1218, 1363, 1497, 1710, 2017) have a second row with blank down_frame, up_frame, down_ms, up_ms, scored_between, same score_before/after and hit=1. Game score step vs Hydra: chord 3 +50 vs +250; chord 7 +50 vs +490; chord 10 +100 vs +500; the nine activation chords +520 vs +920 (V1, V2 scripts v2_e/f). Missing gems 2+2+1+9 = 14 = total_notes 3767 minus notes_hit 3753 (proven by R-M2 scorestats json). Proven by R-M2 `sp_watch-2.csv`: at the nine activation chords the score takes one +520 jump 4 to 7 ms after the read and nothing more until the next chord (V2). Chord 1893 is NOT a `then` split: two separate messages with their own frames 131754 and 131764 (V1 corrects F2-J5's "row 1893 shows the second group did get a reply entry"). Nothing refuses or logs which kit.js ran (searched kit_tools.py, frida_attach.py, bot_run.py, trace_hits.py, method_watch.py, front.py).
- **Latent:** no, every LS-M2 and LS-F1 run on this machine ran it.
- **Splits:**
  - kit.ts (`then` presses every group, replies `groups`) vs kit.js (first group only, no `groups`). Input: any message with a non-empty `then`. Input with empty `then`: same in both builds.
  - Hydra's score for the 12 chords vs the game's (+920 vs +520 at chord 292 and the others).
  - The player's `hit` (score rose, copied to every group row) vs the game's notes: 12 chords read hit=1 and lost gems.
- **Ruling quotes found:** none found for the kit.js freshness question. (Handoff 6341c618 lists "stale build very likely"; no user quote.)
- **Open:** a run with a rebuilt kit.js. (Not run.)

### L02 Freak run 1 (ls-f1-run1): 8 lost gems, 4 from the same stale kit.js, 4 from stalls and a late squeeze press
- **Source ids:** H2.
- **Verdict:** V2 PARTLY (exactly 4 of the 8 are the stale-agent cause).
- **Homes:** R-F1 `press-2.csv`, `sp_watch-1.csv`, stats json, Freak score file `limits-prep-v2\freak-score.json`; docs/clone-hero/03-timing.md TM-50.
- **Evidence:** Proven by R-F1 files (V2 v2_i.py). Stats: 1854 of 1862, misses in buckets 0-10 %:1, 10-20 %:2, 20-30 %:1, 40-50 %:1, 50-60 %:1, 60-70 %:1, 70-80 %:1. Four chords have two rows with the second blank (28, 232, 633, 1062): chord 28 game +150 vs Hydra +350 (-200, the green 4 x 50); chords 232, 633, 1062 game +520 vs +920 (the kick). Other four: chord 153 pressed 107.47 ms late after a 131.5 ms stall; chord 154 hit=0; chord 1144 (squeeze-out late press, +0); chord 803 clock read 62.13 ms late with a 99.4 ms watch gap at clock 107.255 s (not named in the 11-07 handoff or in F1 replies.txt).
- **Latent:** no.
- **Splits:** F1 replies.txt and TM-50 (the page names chord 154 as the only second miss) vs the data (153's kick, 154, 803, and four later groups).
- **Ruling quotes found:** none found.
- **Open:** none for the count; a rebuilt-kit run would show whether the 4 stall cases repeat.

### L03 LS-M2 chord 618 phrase: the meter did not rise because chord 609 (the activation) lost its kick, not because 618 missed
- **Source ids:** H1 (the chord-618 sub-claim), V4 H1, V2-N1 (chord 1894 note).
- **Verdict:** V4 PARTLY (end times 163,483 vs 167,027 ms and the lost extension are right; "chord 618 broke" is wrong). V2 did not verify the 618 part ("Open (not read)").
- **Homes:** R-M2 `press-2.csv`, `sp_watch-2.csv`, `kick-prep\who\...-score.json`, chart `notes.mid`; docs/clone-hero/05-star-power.md SP-04.
- **Evidence:** Proven by R-M2 `press-2.csv`: row 618 hit=1, 278455 to 278855 (+400); row 609 +520 (cymbal only, kick's 400 absent). Proven by the score json: activation 2 act_ms 156354.292, deact_ms 167027.114; chord 618 is `is_sp_phrase_end` at 157925.0. Proven by `sp_watch-2.csv`: meter 7200 at 156.356 s, sp_end_s 163.4769, no rise at 157.921-157.933 (5631 to 5619, normal drain); drop to 0 at 163.483. Chart: phrase tick 168960 to 170759 begins at the activation tick; chords 609 to 618 are in it. V2 note: chord 1894 shows game +400 vs Hydra +600 (not one of the 14; not explained).
- **Latent:** no.
- **Splits:** handoff wording "chord 618 broke" vs press-2.csv (618 hit, 609 lost a gem).
- **Ruling quotes found:** none found. SP-04 says only the last chord carries flag 0x100; no page says a missed phrase note cancels the award (V4 searched 05, 03, 04).
- **Open:** a run with only the kick withheld, or the game's miss handler read.

### L04 Hit-window sides assumed equal (early = late), against TM-26
- **Source ids:** S1-C1, F3-1's split B (F3-1 itself is in L46), F3.F8-2/3 (early = late half; the "2 frames" half is discussed in L48), F3 splits list item 2.
- **Verdict:** V1 PARTLY (S1-C1: TM-26 says "Both fit one 85.72 ms side judged a few ms after the press is sent", so "contradicted" is too strong). V2 CONFIRMED (F8-2/3: unchanged in code; TM-26 and TM-27 as the finder says).
- **Homes:** `play_chart.py:315-325` (docstring "assumes both sides are this one"), `:335-344`, `:368/371`, `:646-647`, `:1115`; `tools/replay_schedule.cpp:137`; docs/clone-hero/03-timing.md TM-26 (:305), TM-27 (:313-321), Q-32.
- **Evidence:** By reading. TM-26 (Proven in a live run) early reaches past 88.43, late ends before 84.92 on an 85.72 side. TM-27 bounds the lag only at >0.80 ms and >= 2.71 ms, "Not proven" for the rest. The Hydra line of TM-26 says "Ruling: none found".
- **Latent:** no for Python; the C++ copy is latent (L46).
- **Splits:**
  - Per-note formula window here vs Hydra's fixed 85 ms (see L46 for the user's words).
  - Early press 88.43 ms at 85.72 side: game hit; load check `abs(at-ms) <= side` refuses it. Late press 84.92 ms: game missed; load check passes it.
- **Ruling quotes found:** none found for equal sides.
- **Open:** Q-32 (TM-27 not proven).

### L05 The drop check judges the late side only, with a fixed 3 ms frame; three more edges for the same question
- **Source ids:** S1-C2, S1 splits list item 3, F4-T3 (edge rows: "press exactly at last_send", "press read at window edge", "two chords closer than the 8 ms gap", "chord at time 0" which is also discussed in L13).
- **Verdict:** V1 CONFIRMED (S1-C2). V2 / V3 CONFIRMED the C++ side in L48. F4-T3 UNVERIFIED (finder's table; the proven row 324 is in L62).
- **Homes:** `play_chart.py:101` (FRAME_MS 3.0), `:1115` (`dropped = sends and raw_ms + FRAME_MS > chart_ms + sides[idx]`), `:647` (load check `<=`), `:415` (lead margin), `:368/371`.
- **Evidence:** By reading. raw_ms = chart+82.72 sends, chart+82.73 drops (side 85.72). A message read too early is not tested. A later order group of a message goes past the check unjudged (its own at_ms is later by after_ms).
- **Latent:** no.
- **Splits:** drop edge (side - 3, late only) vs load check edge (side, both sides) vs lead margin (side - 6) vs TM-26 measured late edge (< 84.92). Input: a press read exactly at chart+side: load check passes, drop check refuses.
- **Ruling quotes found:** none found for the 3 ms in `FRAME_MS` beyond the plan citations (freak-fixes Q1 (a), approved by "Yes, start it", plan P38 at replay-press-schedule.md:343).
- **Open:** none.

### L06 The frame is fixed at 3.0 ms; the game ran 3.033 ms per frame and frames stretch under load
- **Source ids:** F4-T1 (frame), F3.F8-4, S1 splits list item 4 (frame vs the `framerate` setting).
- **Verdict:** V2 CONFIRMED (F4-T1, with the 3.033 ms evidence from R-M0).
- **Homes:** `play_chart.py:101`; `replay_schedule.cpp:35` (kFrameMs); docs/clone-hero/03-timing.md TM-49 (about 330 a second; builds cut it), SP-47 (2.98 to 3.03 ms).
- **Evidence:** Proven by R-M0 `press-1.csv` (1478 presses): 3.033 ms per frame (down_frame to up_frame). Over a 6 ms judge delay that is 0.2 ms. TM-49/50 show frames stretching to hundreds of ms under load; a fixed 3.0 is a model, ruled by the user (freak-fixes Q1(a)).
- **Latent:** no.
- **Splits:** 3.0 ms (C++ and Python) vs 3.033 ms measured. FRAME_MS vs the game's `framerate` setting (330 in the runs).
- **Ruling quotes found:** plan quote "Yes, start it" (replay-press-schedule.md:343 P38) covers freak-fixes Q1(a). No verbatim wording of the frame question.
- **Open:** none.

### L07 Activation lead for a squeeze: Python and C++ work it out two ways, with different answers (and different margins)
- **Source ids:** F3-3, S1-C10.
- **Verdict:** V2 CONFIRMED (F3-3, by reading). S1-C10 was not verified by any verifier.
- **Homes:** `play_chart.py:347-428` (`activation_leads`, `_solve_lead`, margin `ACT_LEAD_MARGIN_MS` 6 at :95; refuses when lead > side - 6; raises when `end.anchor_index != act.index` at :399-404), `:415`, `:588`; `replay_schedule.cpp:224-327` (`equal_room_lead_ms`, `anchor_lead_ms`; margin `kRoomMarginMs` 3 at :66).
- **Evidence:** By reading. V2 worked example: squeeze-out, side_a = side_s = 85.72, chord-to-end gap 130 ms, r = 1: Python lead 65.28, C++ lead 78.5. Plan table (freak-fixes lines 60-66): Bomber chord 147, Python 51.27 vs C++ 60.27; Freak chords 1062-1144 Python fails (room -6.00). freak-fixes T2 is NOT merged: `git log main..claude/pl-freak-t2` lists 3+ commits, and the branch is "held, superseded" (V2).
- **Latent:** Python half is live; C++ half latent (L44).
- **Splits:**
  - Python need/ratio + 6, anchored on the activation chord vs C++ equal room on both presses, anchored on the overfill chord.
  - Anchor not the activation (Freak 263 style): Python raises, C++ plans it.
  - Margin 6.0 (Python) vs 3.0 (+ kJudgeLateMs 6) (C++).
  - S1-C10 edge: need_ms > 0 strict, so need 0 gives no lead; the late gem is accepted at exactly the side by `<=`.
- **Ruling quotes found:** "Yes, start it" (replay-press-schedule.md:343, P38) for the margin 3.0; "yes revise the plan" (replay plan line 29).
- **Open:** none.

### L08 Decline modes: which gems are left unpressed on a fill chord the path passes over
- **Source ids:** F3.F4-1.
- **Verdict:** V2 PARTLY (kept_gems in both homes give the same sets; rulings found only as blanket yeses).
- **Homes:** `play_chart.py:120-134`, `:709-728`, `:690-702`, `:1114`; `replay_schedule.cpp:157-205` (`kept_gems`), `:571-577`, `:596-607`; `game_tools.py:224-228`; docs/clone-hero/04-fills.md FL-31 (:326), 05-star-power.md SP-20.
- **Evidence:** Proven by R-M2 (V2 v2_l.py): in all_but_top_pad all nine declined chords (239, 800, 827, 1004, 1029, 1328, 1617, 1662, 1989) were credited in full (steps 460/460/460/600/600/460/460/460/460) and sp_activations 9 = 9. Not proven: the `nothing` result for a kick-only top pad (no such chord in M2). V2 side note: FL-31 cites only LS-O3 run 1.
- **Latent:** no.
- **Splits:** Python `_flagged_gem` picks the top pad from score-file order vs C++ `Chord::activation_note().colortype`: V2 says same answer (both Green, Blue, Yellow, Red; model.cpp:307-316), two copies of the table, and a tom and a cymbal of one colour cannot share a tick (so the finder's split does not hold). Empty `notes`: Python `press([], [])`? vs C++ one empty group (:352).
- **Ruling quotes found:** "yes all create handoff" (session 490d6781, answering "Take all eight recommendations and start the build?"; quoted in docs/superpowers/plans/2026-11-07-player-refinements.md:111) for all_but_top_pad. "Yes, take all seven" (docs/sessions/2026-11-07-5b92c813.md:21) for the original plan (git show 2a3b47f8:docs/superpowers/plans/2026-11-06-optimal-score-run.md "4. Declining chord 405"). No quote names `kick_only` or `nothing` (searched docs/agents/rulings/*, docs/audit/2026-10-03-fix-decisions.md, docs/sessions/*). User's reaction, per the 6341c618 handoff's ruling list (verbatim there): "what do you mean decline on purpose??? you cant just choose to not hit notes!" and "there are no kick skips in this song. so why are you trying to invent a new rule that chooses not to hit kicks? who came up with that rule?" (said about the Won't Get Fooled run; V2: the data show the rule did not cause those misses).
- **Open:** a question to the user is not covered by a quoted ruling for all three modes (V2); `nothing` mode live result.

### L09 Order groups (activating gem first, lowest multiplier first, 8 ms apart) rest on one run whose cause is unread
- **Source ids:** F3.F4-2, F3.F8-5.
- **Verdict:** V2 PARTLY (8 ms and "only where it matters" are quoted; SC-38 is one run; V2 found a user-words split).
- **Homes:** `play_chart.py:488-527` (`_order_groups`), `:113` (CHORD_ORDER_GAP_MS), `kit_tools.py:286-330`, `kit.ts:490-540`; `replay_schedule.cpp:343-366` (`press_groups`), `:783-789`; docs/clone-hero/06-scoring.md SC-38 (:113-121), SC-10, SC-21.
- **Evidence:** SC-38 "Proven in a live run" (LS-R1b run 1, ls-r1-run1 press-1.csv); the block says "Whether the order is a fixed lane order, and which gem activates, was not read in the code." SC-10 Hydra assumes it, never checked.
- **Latent:** no (Python live).
- **Splits:**
  - SC-10 (Hydra assumes the player chooses the order) vs the player forcing it with an 8 ms gap that frames may collapse (see L10).
  - V2: the user's card says kick first on a squeeze ("in both cases you have to hit the kick first...") vs the activation chord pressing the activating cymbal first and the kick 8 ms later. Different chords, page leaves "which gem activates" open.
- **Ruling quotes found:** "Only where it matters" and "8 ms" (player-refinements.md:204, "The user answered two questions in chat on 2026-11-07"). "in both cases you have to hit the kick first and cymbals are interchangeable so t doesnt matter which is hit first" (docs/sessions/2026-09-29-56ac5b71.md:17, quoted in SC-38's Ruling line).
- **Open:** none for the ordering; live run after a kit.js rebuild (L10).

### L10 The 8 ms group gap is made three ways and may collapse into one frame
- **Source ids:** F4-T1 (order-group gap; and the `first.downMs` read after the first write).
- **Verdict:** V2 CONFIRMED in outline (by reading). Whether it breaks in practice: open, no run has delivered a later group.
- **Homes:** `replay_schedule.cpp:784` (after_ms 8 per group); `kit.ts:523-534, 578-584, 596-603` (`Date.now() >= first.downMs + afterMs` on a 2 ms tick, `TICK_MS` kit.ts:61); `play_chart.py:1277-1279` (keys route, `time.sleep`), `live_press` also times later groups itself (V1 S1-C7 note).
- **Evidence:** By reading. At 330 fps (3 ms frames) kit lands the second group 8 to 10 ms after the first; a frame stretched over 8 ms puts both in one frame. `first.downMs` is read after the first write's promise resolves, so after_ms counts from slightly after the write (magnitude not measured). R-M0 had no split chord; no `after_ms` in R-M0 `kit-1.jsonl` (0 matches).
- **Latent:** no.
- **Splits:** C++ plan 8 ms vs kit.ts tick-plus-frame vs keys-route sleep.
- **Ruling quotes found:** "8 ms" (player-refinements.md:204).
- **Open:** a run on a chart with order groups on the rebuilt kit.js under load.

### L11 Squeeze-out: the late gem is pressed alone 15 ms after the end; the 15 ms numbers rest on questions the pages leave Unknown
- **Source ids:** F3.F4-3, F3.F8-14.
- **Verdict:** V2 CONFIRMED as described (F4-3; F8-14: SP-29/SP-30 Unknown, page 05 lines 299-317).
- **Homes:** `play_chart.py:588-607` (`_timed_presses`), `replay_schedule.cpp:844-878`, `:51-53`; docs/clone-hero/05-star-power.md SP-28 (Proven in the game's code), SP-29 and SP-30 (Unknown).
- **Evidence:** By reading. The "FirstNote rule pays outside" claim is Hydra's, from the score file's gem order (S1-C9 in L68). The finder searched the CH pages for FirstNote: no block under that name (page 02 not opened).
- **Latent:** Python live; C++ copy latent.
- **Splits:** none beyond Python/C++ (L44).
- **Ruling quotes found:** SP_OUT_LAG_MS is "question 2 of docs/superpowers/plans/2026-11-07-player-refinements.md", answered by "yes all create handoff" (session 490d6781; plan line 111). SP_END_LEAD_MS cites "question 3 of the optimal-score run plan" (file removed); no verbatim quote.
- **Open:** SP-29, SP-30.

### L12 retime_from_end moves presses by any delta and never rechecks the window
- **Source ids:** S1-C11, F3.F4-6.
- **Verdict:** V1 CONFIRMED (S1-C11: `_refuse_far_presses` is called once at load, `:807`, grep of call sites: only :807 and a test). V2 CONFIRMED (F4-6).
- **Homes:** `play_chart.py:449-459` (`retime_from_end`), `:1134-1138`, `:632-673`, `:807`.
- **Evidence:** By reading. A delta of +20 ms moves the press later with no check; -20 moves it earlier, no early-edge check. Only the late-side drop check at send time guards. End reads were +3 to +4 ms in F1 ("+4.06 ms", replies.txt). `p.end_ms + delta_ms` raises TypeError if a press with end_act_index has end_ms None (finder: all paths set it).
- **Latent:** no.
- **Splits:** SP-47 Hydra line "no frame model in `src/`; the player's `retime_from_end`" vs `replay_schedule.cpp` kJudge* (see L60).
- **Ruling quotes found:** none found specific to retiming (player-limits plan design point 1 is cited at play_chart.py:455 and engine.py:243; its question list not opened).
- **Open:** none.

### L13 Chords before the start cursor, within 150 ms of the first clock read, are skipped and counted nowhere; a restart discards earlier counts
- **Source ids:** S1-C5, F3.F4-9.
- **Verdict:** V1 CONFIRMED (S1-C5; `live.START_LEAD_MS` not re-opened). V2 UNVERIFIED for F3.F4-9 ("live.py 152 per finder, I did not open live.py").
- **Homes:** `play_chart.py:1063, 1098-1100` (`result = RunResult()` after a jump back), `live.py:152-163` (`first_note_index`, START_LEAD_MS 150).
- **Evidence:** By reading. Clock at chart_ms-100 of chord k: chord k skipped, no row, no count. A chord at ms 0 with the clock starting at 0 is skipped unless the clock starts negative. After a restart `result.rows` and the on_press log differ.
- **Latent:** no.
- **Splits:** none (single home inherited from live.py).
- **Ruling quotes found:** none found (player-limits plan not searched for START_LEAD).
- **Open:** none.

### L14 Silent defaults in the score and dump files: missing keys are accepted and change what is pressed
- **Source ids:** S1-C8 (all sub-items except the reply-column one, which is in L01).
- **Verdict:** V1 CONFIRMED for the sub-items it opened (`:1166`, `:1186-1189`, `:782`, `:790-791`). The others (`:683-686`, `:761`, `:817/836`, `:838`, `:1128`) not verified.
- **Homes:** `play_chart.py:683-686` (missing `path_source` accepted), `:761/817/836` (no `activations` key = none), `:838` (`deact_ms` fallback), `:790-791` (any gem with no `multiplier` silently turns off the order split for the chord), `:782-783` (`chord.get("tick")` None accepted), `:1128` (`read_sp_end` None keeps the plan), `:1338` (no window, see L61).
- **Evidence:** By reading. Score file with no `tick` on chords: no squeezes, activations or end handling; load succeeds with plain presses and no message.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L15 An empty chord is "pressed" as a miss; gems with no lane in the kit; two gems of one element collapse silently
- **Source ids:** S1-C15, F3.F4-8, F3.F4-7.
- **Verdict:** V2 CONFIRMED (F4-7: kit_tools.py:312-316 `seen.setdefault`; unreachable from hydra_replay files since a chord holds one note per colour). F3.F4-8 and S1-C15 UNVERIFIED (V2 did not re-verify F4-8; V3/V4 not opened).
- **Homes:** `play_chart.py:157-166` (`_KEY_FOR_PAD`), `:778`, `:722-723`; `kit_tools.py:144-155`, `:312-316`; `game_tools.py:207-212` (`_LANE_WORDS`).
- **Evidence:** By reading. No Orange / five-lane / double-kick entry: KeyError at load (loud). Non-declined chord with no gems calls `press([], axes=[])`, counted as sent and (if the reply is a no-op) a miss. A kick_only declined chord with two kicks sends one.
- **Latent:** partly; F4-7 not reachable from real data (V2).
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L16 Nothing checks that the score JSON belongs to the song the game is playing (and clock choices in the same item)
- **Source ids:** S1-C13.
- **Verdict:** V3 CONFIRMED (the wrong-song part). The clock half (raw clock, `max_fill_s=0`) is covered with F4-T2 in L18; no verifier verdict beyond that.
- **Homes:** `game_tools.py:1026-1109` (`song_folder` used only at :1055 run_cap_s and :1075 watch_for_run), `sp_tools.py:212-232`, `play_chart.py:747-808` (`load_dump_chords` checks only path index).
- **Evidence:** V3 grepped game_tools.py, sp_tools.py, sp_watch.py, play_chart.py, engine.py for currentSongFolder, folderPath, hyhash, md5, sha: no match. A score JSON for song A played into song B runs, reports hits by chance, stops on past_end/cap.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** a live run with the wrong song (not run).

### L17 ch_run_chart always reads dump path 0; a score with no `path_source` is accepted for any path
- **Source ids:** W2A-4.
- **Verdict:** V3 PARTLY (a score made for path k plus its dump is refused by `_dump_path`; the remaining gap is a score without `path_source`).
- **Homes:** `game_tools.py:1026-1054` (no path_index), `:410-436` (`chords_to_play`), `play_chart.py:747-808`, `:676-687` (`_dump_path`, ValueError "...was made from path k, but path_index is 0"), `tools/replay.cpp:485, 651` (writes `path_source`); `server.py:362-387` and `sp_tools.py:286-312` (those have `path_index`).
- **Evidence:** By reading. `:684` does not compare `path_source.file` with the dump's file name. test_play_chart.py:1050 pins acceptance of a missing `path_source`.
- **Latent:** a path-k > 0 run cannot be started through ch_run_chart at all.
- **Splits:** ch_play_chart and ch_sp_compare have `path_index`; ch_run_chart has none.
- **Ruling quotes found:** none found.
- **Open:** a `--acts` score plus a path-k dump (reading `_squeezes`).

### L18 Two clocks read differently, and what wait_until does when the clock jumps or pauses
- **Source ids:** F4-T2 (and S1-C13's clock half).
- **Verdict:** UNVERIFIED (no verifier opened it; V2 `live.py` not opened). R-M0 sp_watch showed the clock stayed continuous (finder, proven by R-M0 `sp_watch-1.csv`).
- **Homes:** `play_chart.py:1344` (`SongClock(engine.song_clock, max_fill_s=0.0)`), `constants.py:315-318` (CLOCK_MAX_FILL_S 0.05, CLOCK_FRESH_S 0.002 for walk_edges), `live.py:110-146`, `:61, 65, 89, 95` (ClockFrozen after 5 s, ClockJumpedBack past 1 s).
- **Evidence:** By reading. A clock jump back under 1 s is accepted silently; a pause shorter than 5 s passes, presses fire late once the clock moves; late reads are reported only through the late-side drop check (L05).
- **Latent:** no for play_chart; walk_edges is a probe.
- **Splits:** raw clock (play_chart) vs est with up to 50 ms fill (walk_edges and probes).
- **Ruling quotes found:** none found.
- **Open:** none.

### L19 Squeeze-out is sent as two messages 30 ms apart, not the 8 ms gap
- **Source ids:** V1-N1.
- **Verdict:** V1 observed it as a side note; "not investigated".
- **Homes:** `play_chart.py:1081-1082` ("one part of a squeezed-out chord"); R-M2 `press-2.csv` chord 1893 (frames 131754 and 131764).
- **Evidence:** Proven by R-M2 `press-2.csv`: chord 1893 two rows, +520 then +200 = Hydra's +720 (V1, V2 both confirm).
- **Latent:** no.
- **Splits:** the 30 ms comes from the plan, not CHORD_ORDER_GAP_MS.
- **Ruling quotes found:** none found.
- **Open:** why 30 ms.

### L20 ch_open_song: first two steps prove nothing, no instrument/difficulty check, "done:" is printed with the wrong song
- **Source ids:** W2A-5, W2B-C1, W2B-C2.
- **Verdict:** V3 CONFIRMED (W2A-5, W2B-C1). V4 PARTLY (W2B-C2: IN-79 is the page block, "Proven in a live run", and says the active read cannot tell the screens apart; no result file of that run was kept).
- **Homes:** `tools/ch_mcp/menu_steps.json` (6 steps, `"verified": false`, `unproven: true` at line 13, no search step), `menu_tools.py:261-307, 280-288, 113-119, 117, 44-50, 151-162`, `server.py:466-475`; docs/clone-hero/07-internals.md IN-79 (:803-811).
- **Evidence:** By reading. Step 1 expects profile_screen_active and step 2 main_menu_active; both already read true at game start (IN-79), so a lost press at step 1 or 2 shows only when step 3 times out. No step expects ProDrums or Expert. `_done_line` returns "done: a song is loaded, but its folder is X, not the target Y" as an ordinary reply. `verified: false` is read by no code. `online_refusal` reads an unfound menu class as closed. ch_run_chart does not use ch_open_song and never looks at the loaded song.
- **Latent:** no (when ch_open_song is used).
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** a live run (LS-K3 was struck).

---

# Section B: how a run is judged or reported

### L21 No judging script checks it is judging the same song, speed or run
- **Source ids:** F2-J1.
- **Verdict:** V1 PARTLY (proven: stats carry checksum, playback_speed 100, no_fail true, modifiers; no judge compares them). Live part (early quit) not verified.
- **Homes:** `compare_scorestats.py:81-134`, `game_tools.py:860-871` (ch_compare), `score_compare.py:313-322`, `sp_tools.py:286-312`, `bot_run.py:446-471` (`_poll_stats` takes any change of scorestats.json after launch, names the copy by launch slug).
- **Evidence:** Proven by R-M2 `scorestats-...json` (`"checksum": "AF44D5C8..."`, `"playback_speed": 100`, `"no_fail": true`, `"modifiers": ["DoubleKick"]`). V1 searched tools/**/*.py for checksum, song_name, playback_speed, no_fail, chartmode, warnings: only `game_tools.py` / `test_game_tools.py:498-503` prints modifiers and no_fail, none compares.
- **Latent:** no.
- **Splits:** stats of song A vs score of song B: FAIL only by coincidence of counts.
- **Ruling quotes found:** none found.
- **Open:** does the game write scorestats.json when a song is quit early (live quit-early run).

### L22 ch_compare "pass" lines are chart facts, not play results
- **Source ids:** F2-J2.
- **Verdict:** V1 PARTLY (the lines say what the finder says; the overall result still fails on a miss because `score` is compared).
- **Homes:** `compare_scorestats.py:94-120` (and returns `notes_ok and sp_ok and score_ok and dynamics_ok and activations_ok`).
- **Evidence:** Proven by R-M2 scorestats json and replies.txt line 2: notes_hit 3753 of total_notes 3767, sp_phrases_earned 30 of 31, sp_activations 9; the lines total_notes, sp_phrases_total, sp_activations print pass; only `score` FAILs. `notes_hit`, `sp_phrases_earned`, `excess_hits`, `end_streak`, `max_streak` are never compared.
- **Latent:** no.
- **Splits:** the score: score_compare last `score_after` 1,051,655 vs compare_scorestats `score` 1,063,755 vs Hydra `final.total` 1,143,035, same run; 12,100 end-of-solo bonus (solo_bonus_total, proven in the stats file) is the gap between the first two (also in L23).
- **Ruling quotes found:** none found.
- **Open:** none.

### L23 score_compare cannot pass a run with order-split or declined chords; hit rate counts rows, not chords
- **Source ids:** F2-J3, F2 splits list items 1, 2, 3, 4.
- **Verdict:** V1 CONFIRMED (V1 re-ran score_compare).
- **Homes:** `score_compare.py:167-197, 216-231, 234-251, 64, 120, 186, 247`, `play_chart.py:1145-1146, 1163-1204, 1178`.
- **Evidence:** Proven by running score_compare.py over R-M2 `press-2.csv` with `kick-prep\who` score JSON: "1834 equal, 0 lagged, 39 changed at a ghost or accent chord, 277 failed. FAIL score at chord index 3: game 495, Hydra 695 (difference -200...) Final: game 1051655, Hydra 1143035. PASS hit rate 99.6% (2141 of 2150)", exit 1. Same script over R-M0 `press-1.csv`: 1478 equal, PASS 100.0 %, exit 0. 98 % floor: 49 of 50 passes, 48 of 50 fails. n counts rows (2150) not chords (2137). Declined/dropped rows count as non-hits. The end-of-solo bonus (12,100) is never in the last row's `score_after`.
- **Latent:** no.
- **Splits:**
  - "The score of the run": 3 numbers (see L22).
  - "Did this chord hit": press-log `hit` vs compare_scorestats `notes_hit` vs `accent_notes_hit/ghost_notes_hit` vs credited_hits.
  - "How many chords": rows 2150 vs chords sent vs 2137 in the score JSON vs 3767 gems.
  - Hit-rate floor: MIN_HIT_PERCENT 98 vs "misses = sent - hit" with no floor vs play_chart.py main "Hit rate".
- **Ruling quotes found:** none found for the 98 % (cited to "the overlay spike plan, Task B step 3"; plan not opened).
- **Open:** none (the second-group cause is settled by L01).

### L24 The score check absorbs a miss on any chord with a ghost or accent gem (rule written for keyboard presses)
- **Source ids:** F2-J4, F3-5b.
- **Verdict:** V1 CONFIRMED, V2 CONFIRMED (by reading).
- **Homes:** `score_compare.py:186-194` (also :150-197, docstring lines 16-19), `:64`.
- **Evidence:** By reading. `elif chord.ghost_or_accent or change_may_show: status = "ghost_accent"; running = difference`. A missed accent chord (-500) gives no FAIL. 39 chords were absorbed this way in LS-M2 (proven by the score_compare report line in L23). The kit route sends axis values (`pad_axis`, SC-13, SC-18 live), so the keyboard reason no longer holds there.
- **Latent:** no.
- **Splits:** rule written for the keys route (SC-14, Proven in a live run, keyboard only) vs kit runs.
- **Ruling quotes found:** none found.
- **Open:** whether LS runs ran score_compare (V2: not checked).

### L25 sp_compare passes on the game's own consistency; several checks cannot fail
- **Source ids:** F2-J6, F2 splits list item 6.
- **Verdict:** V1 CONFIRMED by reading for the claims it opened (`:523-534`, `:470-478`, `:594-596`, `:578`). V3 CONFIRMED (all sub-claims including NaN clock and "no activations, no line"). Not run against the 17 MB watch file.
- **Homes:** `sp_compare.py:439-546, 554-601, 404-421, 466-480, 523-534, 585-600, 358-390, 396-401, 160-167, 504`.
- **Evidence:** By reading. The "activation N on the path's chord" check is `report.check(True, ...)` whenever the flip lands within 150 ms of the chord; the end difference is only printed. R-M2 activation 2 ended 3,544 ms before Hydra's deactivation (replies.txt) with a PASS line carrying that number in text. "flag clears at the end" compares to the game's own `sp_end_s` within 100 ms. Multiplier check accepts chords i-1, i, i+1 and prints PASS for "0 of 0". NaN clock: `t < lo` and `t > hi` both False, so the first unused rise passes. With both counts 0 no activation line prints.
- **Latent:** NaN-clock path: no run has such a log.
- **Splits:** sp_compare's printed `difference` in a PASS line vs compare_scorestats `sp_activations` (count only) vs replies.txt text.
- **Ruling quotes found:** none found.
- **Open:** none.

### L26 sp_compare re-states Hydra's Star Power constants and passes any constant sp_max (Hydra 32 x vs game 30 x)
- **Source ids:** F3-5a.
- **Verdict:** V2 CONFIRMED (`_check_sp_max` passes whenever sp_max is the same in every row; Hydra's number is printed, never compared).
- **Homes:** `sp_compare.py:109-122, 176-178, 404-421, 414, 549-590`; docs/clone-hero/05-star-power.md SP-09, SP-10, SP-11 (SP-11: the game's full bar is 30 x the resolution; Hydra's is 32 in 4/4; the page records the split).
- **Evidence:** By reading. R-M2 `sp_watch-2.csv` sp_max_u32 = 14,400 on every row V2 read (Who chart resolution not read, so not checked against 30 x or 32 x). Comment says the constants are "copied because this script cannot read C++".
- **Latent:** no.
- **Splits:** Hydra 32 x resolution vs game 30 x (SP-11).
- **Ruling quotes found:** none found.
- **Open:** none.

### L27 compare_scorestats passes `total_notes` on gems OR chords, and decides on keys the page marks Unknown
- **Source ids:** F3-5c, F3.F8-20.
- **Verdict:** V2 CONFIRMED (compare_scorestats.py:94-99; R-M2 total_notes 3,767 equals the score file's gem count, not its 2,137 chords). V2 did not open page 06 line 427.
- **Homes:** `compare_scorestats.py:94-99`; docs/clone-hero/06-scoring.md :427 (total_notes "Unknown"), :432, :442 (SC-37), SC-35.
- **Evidence:** By reading. `sp_phrases_total`, `accent_notes_hit`, `ghost_notes_hit` Unknown on SC-37's table; `score` vs `final.total` is backed by SC-31.
- **Latent:** no.
- **Splits:** gems vs chords as the meaning of `total_notes`.
- **Ruling quotes found:** none found.
- **Open:** none.

### L28 "hit" means any score rise within 5 ms of the reply, copied to every group row
- **Source ids:** S1-C6, F3-5d, F3.F8-19, F3 splits list item 9, S1 splits list item 6.
- **Verdict:** V1 CONFIRMED, now proven (S1-C6). V2 CONFIRMED by reading and by data (F8-19 / F3-5d).
- **Homes:** `play_chart.py:1145-1148`, `:1178`, `engine.py:46-49` (`pressed_input_hit`, docstring "rises only on a hit"), `engine.py:234-237`, `constants.py:100` (OFF_SCORE comment), `constants.INPUT_SETTLE_MS = 250`.
- **Evidence:** Proven by R-M2 `press-2.csv`: the 12 `then` chords print hit=1 while the game scored only the first group (L01). By reading: a 3-gem chord with one gem landing is hit=1; a missed fill-end note is granted without a press (FL-31, SP-20), TM-45 a missed lane note is granted after a recent lane press; the plan's P7 says a declined chord's grant lands in the next row. SCORE_READ_DELAY_S 0.005 is a fixed sleep; a read at +5 ms can predate the judging frame under load (TM-49).
- **Latent:** no.
- **Splits:** engine.py:46 docstring ("rises only on a hit") vs SP-20 / FL-31 / TM-45; "any gem scored" vs "all gems landed" vs the summary's Notes/Misses; 5 ms score read (player) vs INPUT_SETTLE_MS 250 (probes).
- **Ruling quotes found:** none found. The plan row P7 is plan text, not the user's words.
- **Open:** none.

### L29 The sent/hit/misses summary hides dropped, declined and unplayed chords; the CLI and the MCP text differ
- **Source ids:** F2-J7, S1-C3, V1-N2 (CLI main() summary vs MCP status text).
- **Verdict:** V1 CONFIRMED (F2-J7); V1 PARTLY (S1-C3: the CLI summary reads 100 % with drops, but the MCP path names them).
- **Homes:** `play_chart.py:1150-1156, 1391-1393`, `game_tools.py:1491-1554` (esp. :1506-1507 `running:` counts rows with hit == 1, :1534-1537 prints ended, misses, declined, dropped).
- **Evidence:** By reading. Dropped chords go to `result.dropped`, never printed by `main`. MCP path prints `misses {sent - hit}` and names `declined N` and `dropped:` beside it; chords never reached appear only through `ended:`. `running:` counts a split chord twice. Proven by R-M0 `replies.txt`: "1478 chords sent, 1478 hit, 0 misses, none dropped" matches `press-1.csv`.
- **Latent:** CLI path no; MCP path is what runs use.
- **Splits:** sent=100, hit=100, dropped=50 prints "Hit rate: 100.0%", "Misses: 0", "Done." (CLI).
- **Ruling quotes found:** none found. Plan text: freak-fixes Q4 (a) "`hit` holds `dropped`, as it holds `declined` today" under "Yes, start it".
- **Open:** none.

### L30 play_chart main() returns 0 and prints "Done." for every stop reason
- **Source ids:** S1-C4.
- **Verdict:** V1 PARTLY (CLI only; the MCP path shows `ended: <stop reason>`).
- **Homes:** `play_chart.py:1382-1396`, `:1221` (`except OSError` becomes stop="lost").
- **Evidence:** By reading. Only STOP_NO_ENGINE returns non-zero (2). A run stopping on "cap" at chord 40 of 400 with all 40 hit prints 100.0 % and exit 0.
- **Latent:** no for CLI.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L31 The timing numbers in the report are the clock at the Python read, not the time the game judged
- **Source ids:** F2-J8, S1-C12.
- **Verdict:** V3 CONFIRMED by reading (`play_chart.py:1170`); the replies.txt report line not opened.
- **Homes:** `play_chart.py:1086-1088, 1174, 1194, 1170`; `game_tools` report text; docs/clone-hero/03-timing.md TM-27 (:315).
- **Evidence:** By reading. `Clock at press minus chart ms: median -0.49...` (R-M2) is the clock at the Python read minus the chord ms. down_ms/down_frame are the agent's own wall times. Nothing converts `down_frame` to the song clock. For a later order group the row keeps the first group's clock.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L32 A reply cut at 12,000 characters drops its end, and several tools put the verdict last
- **Source ids:** W2A-3.
- **Verdict:** V3 CONFIRMED (no measured long reply).
- **Homes:** `hydra_ui_mcp.py:45, 267-289` (`cap_reply`), `server.py:46-51` (CAP_HINT), `sp_compare.py:633` (Summary last line), `sp_tools.py:312, 251-254`.
- **Evidence:** By reading. ch_sp_compare, ch_compare, ch_notes, ch_run_chart, ch_stop can lose the part that matters; the cut note's hint (`ch_notes(since=...)`) does not apply to ch_sp_compare.
- **Latent:** unknown: how long a real report gets was not measured.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** a real long ch_sp_compare report.

### L33 sp_compare.find_activations never sees an activation that is already on at row 0
- **Source ids:** W2A-9.
- **Verdict:** V3 CONFIRMED.
- **Homes:** `sp_compare.py:230-245`; `sp_tools.py:471-472`.
- **Evidence:** By reading. `i = 1`, requires `not is_active(rows[i-1])`.
- **Latent:** yes unless a watch starts with Star Power on.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L34 trace_hits counts "calls within 85 ms of their nearest note" with a flat 85 ms and no count of notes with no call
- **Source ids:** W2B-F1.
- **Verdict:** V3 CONFIRMED.
- **Homes:** `tools/ch_mcp/trace_hits.py:151-166` (`line_up`), `:232-246` (`args_report`, EXPECT_NORMAL_BACK_MS).
- **Evidence:** By reading. The total in "N of N" is calls, not notes; a call is paired with its nearest note even at 500 ms.
- **Latent:** unknown.
- **Splits:** flat 85 ms vs the note's own side (TM-19 to TM-22; the schedule's `side_ms`).
- **Ruling quotes found:** none found.
- **Open:** compare a real trace's calls to notes.

### L35 The machine recorder reads the clock from a stale or wrong engine pointer and records it as a clock
- **Source ids:** W2B-B1.
- **Verdict:** V4 CONFIRMED by reading. V3 did not verify.
- **Homes:** `machine_recorder.py:366-373`, `machine_report.py` (judges "the game's clock kept up with wall time").
- **Evidence:** By reading. Only OSError/ValueError give None; no range or monotonic check; no check the object is still the engine.
- **Latent:** unknown.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** a recorded run that restarts a song.

---

# Section C: built against source, and loading

### L36 Tests for kit.ts run the .ts, never the built kit.js
- **Source ids:** F1-C2.
- **Verdict:** V1 CONFIRMED by reading.
- **Homes:** `tools/ch_mcp/tests/test_kit_tools.py:1130-1136, 1247-1271, 1252 (skipIf NODE is None), 1432`.
- **Evidence:** By reading. Reads `kit.ts` and runs it in Node against a fake il2cpp; a green T3 test and a stale kit.js are compatible, and are what happened (L01). The frida-compile step is not covered; a machine without node skips quietly.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L37 package.json has one build script per agent, no build-all, and nothing lists which built .js is stale against shared sources
- **Source ids:** F1-C3, F4-T4 (menu.ts has no menu.js, how it loads: answered by W2A, bundled into state.js).
- **Verdict:** V1 PARTLY (kit.js is the only stale one that matters; others differ by comments only). V2: T4 CONFIRMED.
- **Homes:** `tools/ch_mcp/agent/package.json` (build:clock, trace, state, menuhooks, kit, launch), `tools/ch_mcp/.gitignore` line 2 (`agent/*.js`).
- **Evidence:** File times: kit.js 11-06 21:31 (stale, L01); launch.js 10-31 13:17 (launch.ts and launch_core.ts changed 11-07, comments only); menuhooks.js 10-31 10:14 (menuhooks.ts, menu.ts 11-07, comments only); state.js 10-31 11:06 (current); trace.js 11-07 17:11 (current for the D317 rename); clock.js 10-30 21:08 (current). `git diff a41101fc HEAD` read by V1. W2A read launch.js whole against launch.ts + launch_core.ts and found the same functions and messages; shared files gamemanager.ts, hooks_common.ts, menu.ts, launch_core.ts are bundled into several agents.
- **Latent:** yes for the other agents today (built code matches source in behaviour); nothing guarantees tomorrow.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L38 Nothing refuses a built agent that no longer matches its source; baked menu_fields vs the live file
- **Source ids:** W2A-1.
- **Verdict:** V3 CONFIRMED by reading (frida_tools.py only). The baked `menu_fields_default` vs menu_fields.json split not re-checked by V3 or V4 (V4: "reported the same 15 fields").
- **Homes:** `frida_tools.py:372-379, 70, 137`, `bot_run.py:631`, `trace_hits.py:300`, `state.js:3492` (`menu_fields_default`), `menu_fields.json`, `tools/ch_mcp/.gitignore` line 2.
- **Evidence:** By reading. Loaders test only `os.path.isfile`; no hash, mtime compare or build stamp in any reply. Fresh worktree has no .js (fails loudly). Edit to menu_fields.json: Python prints the new enum words at once while the agent keeps the baked names until `build:state` is rerun.
- **Latent:** the baked-vs-file split matters only after an edit.
- **Splits:** menu_fields.json live (`frida_tools.menu_fields`) vs baked (`state.js:3492`).
- **Ruling quotes found:** none found.
- **Open:** `npm run build:state` into a scratch copy and diff (builds not allowed).

### L39 Which checkout's code a run loads; no commit or constants stamp in the run
- **Source ids:** S1-C14.
- **Verdict:** V4 PARTLY (Python and the built JS always come from one checkout so cannot mix; nothing stamps or compares which build it is).
- **Homes:** `.mcp.json` (relative `tools/ch_mcp/front.py`), `front.py:180` (Popen, no cwd), `server.py:31-33`, `frida_attach.py:37-40`, `frida_tools.py:49, 376`, `kit_tools.py:721`, `bot_run.py:631`, `trace_hits.py:265`.
- **Evidence:** By reading. A press log does not record the commit or the constants (PRESS_LEAD_MS etc.) it ran with. V4 searched for rev-parse, getmtime, st_mtime, stale: only unrelated hits (game_tools.py:1285, bot_run.py:430, quiet.py:278).
- **Latent:** a worktree-started worker finds no .js and says so.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L40 The hydra_replay exe is not tied to the source it wrote JSON from; the JSON carries no exe stamp
- **Source ids:** F1-C4.
- **Verdict:** V1 PARTLY (exe dated 11-08 11:14 predates replay_schedule.cpp changes at 11:32 to 12:26; whether the JSON content differs not checked).
- **Homes:** `tools/ch_probe/prep_inputs.py:45, 85`; `tools/replay.cpp` (3b23d8f1 11:32), `tools/replay_schedule.cpp` (12:26); the score/dump JSON `path_source` (file, index).
- **Evidence:** Proven by `Get-Item` on `build-cpp\Release\hydra_replay.exe` (V1): 11-08 11:14. Kick-prep JSONs dated 11:16. A worktree has its own build-cpp; `--exe` default and `_REPO_ROOT` change with where the script runs.
- **Latent:** unknown.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** needs `hydra_replay` run (not allowed to the verifier).

### L41 The launch agent's "patched" is accepted before the loader has run on the patched byte
- **Source ids:** W2A-6.
- **Verdict:** V3 CONFIRMED, with a correction (the loader line is printed by launch_human too, but usually reads "not yet" at that moment; nothing re-reads it later).
- **Homes:** `bot_run.py:701-716, 557-566, 569-586`, `launch_core.ts:143-154, 161-199, 169-182, 151-153`, `launch_patch.json`.
- **Evidence:** By reading. A "no" (loader ran, bytes differ) does not fail the launch; a failed first-run read is a `reason` on a patched status. Build identity rests on 12 entry bytes and 5 patch bytes; `launch_patch.json` `"build": "1.1.0.6142"` is read by nothing.
- **Latent:** unknown.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** a live launch.

### L42 One game build in three spellings, never compared to the installed build; a user path in checked-in files
- **Source ids:** W2A-7.
- **Verdict:** V4 CONFIRMED by reading.
- **Homes:** `launch_patch.json:3` ("1.1.0.6142"), `menu_fields.json:3` ("6142"), `start_candidates.json:3` ("1.1.0.6142-final"), `menu.ts:54` (RVA 0x32F590), `launch_patch.json:4` and `kit_methods.json:3` (absolute `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs`), `frida_tools.py:371-375, 669`, `build_facts.py:105-126`, `test_frida_tools.py:528, 564, 595`, `process.py:196-205`, `sp_tools.py:396`.
- **Evidence:** By reading. `ch_attach` checks only that dumped facts exist for the installed build; a changed exe with the same version string passes; failures after a game update are loud in the agent but `ch_menu` then shows every field as an error.
- **Latent:** yes until the game updates.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L43 The machine recorder never verifies the game build; paths are hard-coded
- **Source ids:** W2B-B2.
- **Verdict:** V4 PARTLY (the recorder itself calls no `verify_targets`, but its engine address comes from the SP watch, which ran `verify_targets` first via `game_tools.py:1080` and `sp_tools.py:396`; the check is two rdata doubles only).
- **Homes:** `machine_recorder.py` (C.OFF_SONG_CLOCK 0x100), `process.py:196`, `build_facts.py:97-127, 28` (DUMPS_ROOT), `game_files.py:32` (GAME_DIR).
- **Evidence:** By reading.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

---

# Section D: the C++ schedule (latent)

All rows in D: nothing in tools/**/*.py reads `hydra_replay schedule` output today (V1 grep of play_chart.py, prep_inputs.py, game_tools.py; V2 grep of tools/**/*.py for replay_schedule, schedule_json, `"presses"`, `--schedule`; V3 grep for last_send_ms, "refusals", if_end_after_ms, room_ms). So no run uses any of this yet.

### L44 The player does not read the schedule: two full copies of the Star Power planning exist
- **Source ids:** F1-C5, F3 "big finding" (and F3 splits list item 13), F4 chain step 2, F2-S1 (intro: refusals are free text, no flag on press/chord/squeeze rows).
- **Verdict:** V1 CONFIRMED (F1-C5). V2 CONFIRMED (header claim; TP1-TP4 not built).
- **Homes:** `tools/replay_schedule.h:1-3` ("The player reads one file and works no Star Power rule out itself"), `tools/replay_schedule.cpp:962-966` (writes `decline`, `chords`, `presses`, `refusals`), `tools/replay.cpp:618-648, 871`, `play_chart.py` (`press_plan:431`, `activation_leads:347`, `_timed_presses:558`, `_order_groups`, `_refuse_far_presses`).
- **Evidence:** By reading. Plan: docs/superpowers/plans/2026-11-08-replay-press-schedule.md, "Rulings that bind this plan" line 29.
- **Latent:** yes: no Python reads the schedule.
- **Splits:** replay_schedule.h:1-3 vs play_chart.py (Python still plans). Two plans for one question; nothing compares them. A future consumer must parse refusal sentences.
- **Ruling quotes found:** "why is the python still doing its own thing? the whole point of this plan is to hand off all the starpower planning to hydra which already knows what to do" then "yes revise the plan" (docs/superpowers/plans/2026-11-08-replay-press-schedule.md:29).
- **Open:** plan tasks TP1-TP4 (Python reads the schedule) not built.

### L45 Six timing constants live in both Python and C++, and the room margin differs
- **Source ids:** F3-2, F2-S1j (constants half), F4-T1 (PRESS_LEAD_MS, SP_END_LEAD_MS, SP_OUT_LAG_MS, CHORD_ORDER_GAP_MS, room margin), F3 splits list item 3, F2 splits list item 5.
- **Verdict:** V2 CONFIRMED (F3-2 and F4-T1 constants). V3 CONFIRMED (F2-S1j, C++ side).
- **Homes:** `play_chart.py:78` (PRESS_LEAD_MS 2.0), `:83` (SP_END_LEAD_MS 15), `:88` (SP_OUT_LAG_MS 15), `:95` (ACT_LEAD_MARGIN_MS 6.0), `:101` (FRAME_MS 3.0), `:113` (CHORD_ORDER_GAP_MS 8); `replay_schedule.cpp:35, 39-40, 43, 49, 54, 61, 66` (kFrameMs, kJudgeEarlyMs 3, kJudgeLateMs 6, kPressLeadMs, kSpEndLeadMs, kSpOutLagMs, kChordOrderGapMs, kRoomMarginMs 3); the C++ writes them into a `constants` object (:16-17) that Python never reads; `constants.py`.
- **Evidence:** By reading. Values agree for five; margin Python 6.0 vs C++ 3.0 (plus kJudgeLateMs 6). Two margins: C++ requires room >= 3 against a 6 ms worst case; Python leaves 6 ms of side. kRoomMarginMs 3.0 and kFrameMs 3.0 are two numbers that happen to be equal. A one-sided edit fails a test only on that side (V2: test_replay_schedule.cpp:264-291 pins the C++ literals; test_play_chart.py:813-815 pins the Python four; no test compares them).
- **Latent:** Python half live, C++ half latent.
- **Splits:** margin 6.0 vs 3.0 (+6); all other five agree today.
- **Ruling quotes found:** per constant, from the finders' reading of comments: kPressLeadMs cites D51; kSpEndLeadMs cites "question 3 of the optimal-score run plan" (file removed); kSpOutLagMs cites player-refinements question 2 and "yes all create handoff" (session 490d6781; replay plan line 111); kChordOrderGapMs "8 ms" and "Only where it matters" (player-refinements.md:204); kFrameMs and margin 3.0 freak-fixes Q1(a)/Q2(a), "Yes, start it" (replay-press-schedule.md:343, P38).
- **Open:** none.

### L46 The per-note hit-window formula has three homes (Python, C++, a Python probe copy), plus Hydra's fixed 85 ms
- **Source ids:** F3-1, S1-C9 (window formula bullet; the other bullets are in L68), F3 splits list item 1, S1 splits list item 1, W2B-A5, W2B-E3.
- **Verdict:** V2 CONFIRMED (F3-1, by reading). V4 CONFIRMED (W2B-A5: grep of src, tests, tools for kWindowBackMs/kWindowFrontMs finds only replay_schedule.cpp and its test; tests pin 37.5 for C++ only). V4 PARTLY (W2B-E3: C++ constants as stated; Python `predicted_window_normal` not opened).
- **Homes:** `play_chart.py:304-344`, `constants.py:222-224, 259-272` (EXPECT_NORMAL_BACK_S 0.085, FRONT_S 0.0375, CAP_FROM_GAP_MS, FLOOR_UP_TO_GAP_MS, `WINDOW_FORMULA_NORMAL_*`), `replay_schedule.cpp:19-29, 121-155` (0.0110924369747899, 2.628539e-05, 170, 20; clamps 37.5 and 85), `src/core/model.h:104/105` (`kHitWindowMs` 85, pinned by test:372), `analysis.py` (`predicted_window_normal`, takes the constants live from the engine), docs/clone-hero/03-timing.md TM-17 to TM-22, TM-51, TM-23/24, TM-29.
- **Evidence:** By reading. Same side for the cases in the finder's table: neighbours 1000 ms both sides 85.72; one chord 85.72; both gaps <= 75 ms 48.15; gap 0 treated as no neighbour; negative gap gives 37.5, no error. Digits found in constants.py, replay_schedule.cpp, test_play_chart.py, docs, SPEC.md; test_replay_schedule.cpp:364 only checks the header names kFrameMs and kRoomMarginMs.
- **Latent:** Python live; the C++ copy latent.
- **Splits:** Python and C++ per-note window vs Hydra's fixed 85 (TM-22, TM-29). Python takes constants live; C++ hard-codes them.
- **Ruling quotes found:** "hydra does not currently account for the dynamic hit window, although that is a good idea for a new feature" (replay-press-schedule.md:31, "plan line 31"). Covers the screens, not the two copies of the formula.
- **Open:** whether test_replay_schedule.cpp pins `formula_*` from the schedule's "constants" object (V2: read the grep hits only).

### L47 Order groups, inside groups, far-press check, kept gems: C++ says "moved" but Python still has all of them, and they answer differently
- **Source ids:** F3-4 (and F3 splits list items 5, 6, 7, 8).
- **Verdict:** V2 PARTLY (both copies exist; Python raises at the first bad far press, C++ writes a refusal per chord; C++ has fallback rows, Python none; but the finder's top-pad split does NOT hold).
- **Homes:** C++ comments "moved" at `replay_schedule.cpp:343, 368, 391, 845`; `play_chart.py:488-527, 632-673, 700-728, 736-744, 131 (_TOP_PAD_ORDER = ("Green","Blue","Yellow","Red"))`; `replay_schedule.cpp:183-205, 343-422, 934-946`; `src/core/model.cpp:297-316` (`activation_note`).
- **Evidence:** By reading. Python `_kept_gems` raises ValueError for two gems of the top colour (dead for hydra_replay dumps since one note per colour); C++ drops a press into `refusals` when the late gem pays less than an inside gem, Python raises; no `retimed`/`join_group` in play_chart.py (grep `retim|fallback|join_group`).
- **Latent:** Python live; C++ copy latent.
- **Splits:** Python raises at first bad press vs C++ one refusal per chord; Python raises when a late gem pays less vs C++ writes a refusal; C++ fallbacks vs none (L53). `_TOP_PAD_ORDER` and `activation_note` order: same answer, two copies of the table (V2: no value split).
- **Ruling quotes found:** none found for the two copies; see L45 for the numbers.
- **Open:** none.

### L48 Three different answers to "how late may a press be sent and still be judged inside"
- **Source ids:** W2B-A1, F4-T1 (split 1: 2 frames vs 1 frame), S1 splits list item 3 (the "2 frames" half of F3.F8-2/3, whose id sits in L04) (C++ judge late edge 6 ms vs Python drop 3 ms), V1 section on F1-C5 (kJudgeLateMs 6 vs drop check 3).
- **Verdict:** V2 CONFIRMED (F4-T1 split, inside the C++ file). V3 CONFIRMED, latent (W2B-A1). V2 CONFIRMED (F8-2/3, kJudgeLateMs = 6 vs SP-47 "one frame").
- **Homes:** `replay_schedule.cpp:249-251` (`press_room_ms`, two frames late, early side judged at the unjudged aim), `:329` (`last_send_ms` = chord + side - 3), `:333-335` (`sqout_fallback_after_ms` two frames), `:339-341` (`sqin_fallback_before_ms` one frame, +18), `:392-402` (`far_press_refusal`, zero frames), `:36-40, 271`; `.h:124-131`; `play_chart.py:1115` (one frame); docs/clone-hero/03-timing.md TM-27, SP-47.
- **Evidence:** By reading, with a worked input (V3): side 50, chord 1000: last_send 1047; far_press accepts 1050; room-based allows aim <= +44, with the 3 ms margin aim <= +41; a press sent at 1047 is judged at 1050..1053 under the 1-2 frame model, and 1053 is outside 1050. Early side: aim -52, side 50 gives room -2 although judged at -49..-46 is inside. No test puts a press between last_send_ms and the 2-frame limit. SP-47 gives one frame (four activations 2.98 to 3.03 ms); TM-27 bounds the lag at >0.80 ms and >= 2.71 ms; no block gives two frames.
- **Latent:** yes (C++); the Python 3 ms drop is live (L05).
- **Splits:**
  - 0 frames (far_press_refusal) vs 1 frame (last_send_ms, sqin fallback, Python drop) vs 2 frames (press_room_ms, sqout fallback).
  - replay_schedule.cpp kJudgeLateMs = 6 vs SP-47 "one frame".
- **Ruling quotes found:** none found.
- **Open:** a live run that sends a press at last_send_ms; Q-32.

### L49 A squeeze-out chord ignores the declined gem list
- **Source ids:** W2B-A2.
- **Verdict:** V3 CONFIRMED (latent).
- **Homes:** `replay_schedule.cpp:558-577, 844-860` (inside groups from `mults[i]`, late gem as place 0, never from `sent[i]`), `:815, 881` (plain and squeeze-in use `sent[i]`); `play_chart.py:785-788` (raises "chord N is a fill the path passes over and also a squeeze or an activation").
- **Evidence:** By reading. No test combines a squeeze with a declined chord; replay_schedule.cpp has no refusal for it (only the lead refusal at :820).
- **Latent:** yes.
- **Splits:** Python refuses the input; C++ presses all gems. Input: a skipped-fill tick that is also a squeeze-out tick. Whether a real path can have that: not checked.
- **Ruling quotes found:** none found.
- **Open:** none.

### L50 Chords drop out of the press list when a refusal fires; a declined chord leaves a press row with no gems
- **Source ids:** F2-S1b, F2-S1g, W2B-A3.
- **Verdict:** V3 CONFIRMED (both, latent).
- **Homes:** `replay_schedule.cpp:818-823, 836, 841, 850, 725, 638, 641, 651, 691-693` (feasible:false), `:348-352` (`press_groups` returns one empty group), `:939` (`!p.gems.empty()` skips the far-press check), `:966` (refusals array).
- **Evidence:** By reading. A lead chord that is also a squeeze or an end gets a refusal then `continue` with no press row; no-split, no-read and late-pays-less cases `continue` ("refused above"). The press list is shorter than chords minus declined with no row saying which. The Python loader treats an empty press as a no-key message (S1-C15 / L15). Tested only at `press_groups` level (test:719), not in the file.
- **Latent:** yes.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** whether the player stops on a non-empty `refusals` (no Python reads it).

### L51 A squeeze whose tick lands on no chord gets feasible:false with no refusal sentence; other reads skip silently
- **Source ids:** F2-S1a, F2-S1d.
- **Verdict:** V3 CONFIRMED (both).
- **Homes:** `replay_schedule.cpp:638, 641, 489-490, 503, 507, 692` (S1a); `:725` `if (!valid(x)) continue;` (S1d); sentences exist at :730-735 and :743-746.
- **Evidence:** By reading. The refusals at 489-490 fire only for a missing tick, not a tick that lands on no chord (`chords.at(...)` returns null via or_null).
- **Latent:** yes.
- **Splits:** chord_tick present but not a chord: feasible false, refusals empty.
- **Ruling quotes found:** none found.
- **Open:** none.

### L52 A refused far-press is still emitted with no flag; only the first refusal per chord is recorded
- **Source ids:** F2-S1c.
- **Verdict:** V3 CONFIRMED.
- **Homes:** `replay_schedule.cpp:939-958` (far_refused set; row still written).
- **Evidence:** By reading.
- **Latent:** yes.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L53 No flagged gem, no aim end, and fallback paths press plain with no sentence
- **Source ids:** F2-S1e, F2-S1f, F3.F4-4 (fallbacks that change what is pressed: late gem joins the last inside group; squeeze-in chord pressed plain).
- **Verdict:** V3 CONFIRMED (S1e, S1f). V2 CONFIRMED as described (F4-4).
- **Homes:** `replay_schedule.cpp:795-802` (`flagged()` returns nullopt silently), `:895-927` (final-end chord with no aim end falls to "plain" at -2 ms; inside-moved-end `continue`s), `:868-891` (`fallback` rows), `:303-308`; `play_chart.py:793-799` (Python: `activating_gem = 0` for a chord with no pad gem; raises for two gems of the top colour, `:736-744`).
- **Evidence:** By reading. Python has no fallback rows (grep).
- **Latent:** yes.
- **Splits:** C++ returns the first gem of that colour and never raises vs Python raising for two gems of the top colour.
- **Ruling quotes found:** freak-fixes Q3 (a), approved by "Yes, start it" (replay-press-schedule.md:343, P38). Question text freak-fixes.md:86 ("When the read end leaves the late gem no room ... Sides: (a) press the late gem wit...") was read trimmed.
- **Open:** none.

### L54 Two squeezes of one kind on one anchor share one lead with no sentence
- **Source ids:** F2-S1h.
- **Verdict:** V3 CONFIRMED.
- **Homes:** `replay_schedule.cpp:282, 292-298, 679` (`way_of` uses `geos.front().kind`).
- **Evidence:** By reading. Keeps the last SqOut/SqIn in the pointer.
- **Latent:** yes.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L55 Missing `sqin_ticks`, `from_record` or sqout offset in a dump replay a phrase as played; Hydra's `warnings` array is read by no script
- **Source ids:** F2-S1i.
- **Verdict:** V3 PARTLY (replay_json.cpp:26-39, :80-82, :87 read; "CLI prints the pick" and the warnings-unread claim not re-grepped).
- **Homes:** `replay_json.cpp:26-39, 80-82, 87`, `replay.cpp:418-421, 481`.
- **Evidence:** Proven by kick-prep score JSONs: `warnings: []` in both, so no run has had one. A dump without `sqin_ticks` scores a squeezed-in phrase as played, high with no sentence.
- **Latent:** yes (no run has had a warning).
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

---

# Section E: game facts

### L56 The far-press refusal for every pad rests on TM-50, which proves only the kick pair
- **Source ids:** F3.F4-5.
- **Verdict:** V2 CONFIRMED.
- **Homes:** `replay_schedule.h:169-174` ("the game gives a press inside two unhit same-pad windows to the earlier note"), `replay_schedule.cpp:391-422, 934-946`; `play_chart.py:632-673, 1115`; docs/clone-hero/03-timing.md TM-50 (:323-331).
- **Evidence:** TM-50 "Proven in a live run" but "Not proven: whether an early press inside two windows also goes to the earlier note; any pad but the kick for the two-window case; any pair but these"; its Ruling line "none needed: no Hydra rule depends on it". V2: the Freak 153/154 case is a yellow cymbal pair, but it is a press outside its own note's window taken by the next note, not the two-window earlier-note case.
- **Latent:** C++ yes; Python load check live.
- **Splits:** TM-50 Ruling line ("no Hydra rule depends on it") vs replay_schedule.h:169-174 (depends on it for every pad).
- **Ruling quotes found:** none found for the matching rule. (The refinements plan line 111 "the load check takes the game's window and lanes", under the blanket yes "yes all create handoff".)
- **Open:** a live run for other pads.

### L57 "A plain gem's velocity is never read, so it presses like an accent" (kit_tools comment) is not what SC-13 says
- **Source ids:** F3.F8-6.
- **Verdict:** V2 CONFIRMED as unsupported by the cited block (code behaviour supported live).
- **Homes:** `kit_tools.py:258-261, 262-283` (`pad_axis`: axis = 0.5 + v / 254, accent and plain at 127, ghost at threshold - 1); docs/clone-hero/06-scoring.md SC-13 (:145-153), SC-18; 07-internals.md IN-86.
- **Evidence:** By reading. SC-13: the byte is the hit's velocity; ghost below the threshold, accent at or above, so a plain gem pressed at 127 is marked accent in the game's compare. Proven by R-M2 stats: accent_notes_hit 82 = accent_notes_total 82, ghost 160 = 160 (V2 did not recount against the score file); R-M0 166/166 accents, 167/167 ghosts per replies (not recounted).
- **Latent:** no.
- **Splits:** comment vs SC-13 text.
- **Ruling quotes found:** none found.
- **Open:** none.

### L58 Kit action ids and the kick bound to the action "Orange" (F8-9)
- **Source ids:** F3.F8-9.
- **Verdict:** V4 NOT CONFIRMED as a missing block. IN-82 (07-internals.md :837-839) lists definition 1 "MIDI Drums" with the same bindings id for id as `kit_tools.py:161-172` `KIT_BINDINGS`. V2 did not verify.
- **Homes:** `kit_tools.py:144-172`; docs/clone-hero/07-internals.md IN-82; `docs/clone-hero/code/6142/drums_input_source.asm:183-187` (per the kick-axis plan).
- **Evidence:** By reading. IN-82 "Proven in the game's code"; "the read script was not kept". The cymbal action names are not individually listed (the code says names are for the reply only).
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L59 Declining works because a missed fill-end note is granted without activating: evidence is slot spacing plus one run (now a second song)
- **Source ids:** F3.F8-16.
- **Verdict:** V2 CONFIRMED (FL-31 text "read from the slot spacing" at page 04 line 326; M2 adds nine more granted chords).
- **Homes:** docs/clone-hero/04-fills.md FL-31; 05-star-power.md SP-20; LS-O3 run 1 (Senri chord 405).
- **Evidence:** Proven by R-M2 (see L08): nine declined chords credited in full, activations 9 = 9. V2 side note: FL-31 should gain this result when written back.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L60 Pages and comments disagree with the code or each other (page behind code; labels vs plan rows)
- **Source ids:** F3.F8-15, F3.F8-21, F3 splits list items 11 (TM-50 vs schedule .h; also in L56), 12, 14, V2-N2 (FL-31 write-back), V2-N3 (TM-50 does not know chord 803).
- **Verdict:** V2 notes the FL-31 and TM-50 write-backs as side notes; F8-15 and F8-21 UNVERIFIED by any verifier.
- **Homes:** docs/clone-hero/04-fills.md FL-09 vs the refinements plan P22 ("unchecked", FINDINGS line 50 only; `replay_schedule.cpp:795-802` comment "row P27"); `engine.py:65-69` and `constants.py:108` ("not yet confirmed live" for SP offsets) vs pages SP-09/SP-13/SP-19 labelled Proven in the game's code; SP-47's Hydra line ("no frame model in `src/`") vs `replay_schedule.cpp` kJudge*; 03-timing.md TM-50 (chord 803 not recorded).
- **Evidence:** By reading. FL-09 page label is stronger than the plan row (`_TOP_PAD_ORDER`, play_chart.py:131, 736-744). The engine.py comment is stale or the offsets unconfirmed live, "I cannot tell".
- **Latent:** no.
- **Splits:** each pair above is one split.
- **Ruling quotes found:** none found.
- **Open:** none.

---

# Section F: focus and keyboard

### L61 The keys route focuses the game window and can send keys to the user's window
- **Source ids:** S1-C7, F4-F9-2, W2A-F, W2B-G, W2B-D4.
- **Verdict:** V1 CONFIRMED (S1-C7). V2 CONFIRMED (F9-2). W2A-F: V3 and V4 list it as not verified; V2's wider search (non-test Python and ps1 under tools/ for SetForegroundWindow, ShowWindow, SendInput, FindWindow, BringWindow, SwitchToThisWindow, SetFocus, AllowSetForeground) found hits only in the keys-route files `input_driver.py`, `play_chart.py`, `walk_edges.py`, `pad_flash_test.py`, `topmost_probe.py`, `capture_window.ps1`, `capture/steps.ps1`, `bot_run.py` (SW_SHOWMINNOACTIVE); none runs on a kit route. W2B-G, W2B-D4 UNVERIFIED.
- **Homes:** `play_chart.py:1263-1281` (`live_press`, `time.sleep` for later groups), `:1333-1340` (`live_play`; main at :1382 always uses it), `input_driver.py:259, 284-291` (`focus_window` drops the return of SetForegroundWindow), `game_tools.py:895-899, 933-937, 993-996` (refuse with "no game window found" when hwnd is 0), `walk_edges.py:224-226, 240`, `active_probe.py:197`, `pad_flash_test.py:63`, `server.py:148-150` and the hook `C:\Users\Patrick\.claude\gates` (`allow key run`), `interfaces.py:209-235, 211, 229` (Protocol naming SendInput).
- **Evidence:** By reading. If `find_game_window` returns 0: WARNING, the run goes on, keys go to whatever is in front. SetForegroundWindow from a background process can be refused, return dropped. The `allow key run` guard is not checked in any Python file; V1/V2 did not read the hook. FindWindowW matches by exact title; a minimized window still matches and SetForegroundWindow does not restore it.
- **Latent:** no for the CLI (`py tools/ch_probe/experiments/play_chart.py` runs live_play with no press); kit routes do not touch windows.
- **Splits:** the command-line players (warn and go on) vs `game_tools.ch_press/press_keys` (refuse).
- **Ruling quotes found:** none found in this audit's reports.
- **Open:** whether SetForegroundWindow succeeds from the MCP worker; minimized-window behaviour (a keys-route run; not to be run).

### L62 The 43.9 s foreground event in LS-M0 lines up with a 30 ms stall and the one late press, but who brought the window forward is unknown
- **Source ids:** F4-F9-1, V2-N4 (tools/capture_window.ps1 calls SetForegroundWindow and ShowWindow, lines 81, 84).
- **Verdict:** V2 PARTLY (facts right; the causal reading is only a guess).
- **Homes:** R-M0 `replies.txt` line 4, `press-1.csv` row 324, `sp_watch-1.csv`; `foreground.py` (poll every 0.1 s, line 22); `bot_run.py:154-183, 323-330` (SW_SHOWMINNOACTIVE; spawn form has no show-window option); docs/clone-hero/07-internals.md IN-78; sessions 2026-10-31-d1867340 (7.9 s) and 2026-11-07-70bfa33d (48.9 s).
- **Evidence:** Proven by R-M0: replies.txt "foreground went to the game 2 times, at 0.0 s and 43.9 s since the watch started"; press-1.csv row 324 (chord at 38737.4 ms) read 21.74 ms late at epoch 1794158126.856 (all others -2.0 to 0); sp_watch-1.csv has a 35.8 ms clock gap at clock 38.72 s. The press still hit (116015 to 116215). Counter-evidence (V2): M2 sp_watch-2.csv has gaps of 24.9 ms and 33.4 ms with only the 0.0 s foreground event; Freak run 1 has 9 stalls of 30 to 131 ms and 5 foreground events. The foreground watch's zero is inferred (patch time 82.3 + 43.9 = 126.2 vs stall at 126.82). The watch's `events` list is kept in memory only; R keeps the count line. No kit-route code calls FindWindow / SetForegroundWindow / ShowWindow / SendInput. A window closing hands the front to the next window in Z order. Child processes use CREATE_NO_WINDOW.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** a run with a window-event log (ETW).

### L63 Kit route needs no focus; Rewired's "ignore input when app not in focus" is switched off for the run
- **Source ids:** F4-F9-3.
- **Verdict:** V2 CONFIRMED by data (M0/M2 ran at 330 fps while behind, 3.033 ms per frame).
- **Homes:** `kit.ts:363-368, 857-860`, `kit_tools.py:710-719`, `server.py` (instructions line "Rewired's keyboard and mouse off for the run").
- **Evidence:** If teardown fails or the worker dies mid-run the setting stays changed in the game process only; registry/ini restore is by ch_stop (not read). The Rewired setting does not block the OS from giving the user's keys to the game window if the user clicks it.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L64 The foreground watch polls every 0.1 s and keeps its events only in memory
- **Source ids:** F4-F9-4.
- **Verdict:** UNVERIFIED.
- **Homes:** `foreground.py:22`, `ForegroundWatch.poll`.
- **Evidence:** By reading. A grab shorter than 0.1 s between two polls is not counted; a game that stays in front counts once (IN-78). Suggestion: save `watch.events` next to replies.txt.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

---

# Section G: tests

### L65 Test fakes for the press reply hide the real shape; the stale-kit case has no test
- **Source ids:** F1-C6, F2-S5, S1-C17.
- **Verdict:** V1 CONFIRMED (F1-C6). F2-S5 and S1-C17 covered by V1 F1-C6 (V3: S1-C17 not opened).
- **Homes:** `tools/ch_probe/tests/test_play_chart.py:1482-1496` (`_SplitReplyPress`), `:675-678`, `:177`, `:505` (`return {}`), `:584, 2089, 1576`, `:1522` (pins `hit` 1 and 1 with one `score_after` for both rows of a split chord, i.e. the copied `hit`), `:2264-2270`; `tools/ch_mcp/tests/test_game_tools.py:1145, 1203, 1230, 2239, 2662`; `kit.ts:647-656`; fixtures `tools/ch_probe/tests/*_score.json`, `*_dump.json`, `senri_end_chords.json`.
- **Evidence:** By reading. The fake's group entries lack `after_ms`; its reply lacks `rewrite` and `on_main`; fake `down_ms` is 9000+ vs epoch ms. No test sends `then` and gets back a reply without `groups`; a `groups` list shorter than 1+len(later) (`play_chart.py:1186`) has no test; a missing kit.js at the ch_play_chart level is covered only for launch.js in bot_run. Fixtures are hand-trimmed (only path_source, activations, chords); play_chart reads none of the dropped keys.
- **Latent:** no.
- **Splits:** `senri_end_chords.json` dated 11-06 22:04, no path_source; whether any test needs it was not checked. Real run `launch-*.jsonl` vs the fakes in test_bot_run.py: not compared by F1.
- **Ruling quotes found:** none found.
- **Open:** none.

### L66 The C++ schedule tests have untested rules and refusals
- **Source ids:** W2B-A4.
- **Verdict:** V3 CONFIRMED (grep of tests/test_replay_schedule.cpp: no test names "sp_end" as a press rule or "inside_moved_end", nor the refusal texts "two squeezes", "also a squeeze", "no chord at skipped", "no SP-end steps", "names no chord", "has no end in the record", "SP-end step on chord", "no stored ratio", "cannot realize", "less than a plain press"; "steady span" 1 hit). A test could still reach them without naming them; V3 did not trace every test.
- **Homes:** `tests/test_replay_schedule.cpp` against `tools/replay_schedule.cpp:897-926, 643, 820, 616, 470, 489-490, 731, 743, 990, 303, 572, 798, 939`; test:241 uses `c.index`, test:165 (SqOut `transfer_post` missing only), test:222, test:616, test:627, test:638, test:654, test:719.
- **Evidence:** By reading. Also: empty chart / no chords untested; `replay.chords[i]` assumed equal to `song.sequence[i]` and nothing fails if not; lead below kPressLeadMs untested; lead steady-span tested only for the early side; KickOnly with no kick and AllButTopPad with empty notes untested; both-directions refusal tested only through `anchor_lead_ms`.
- **Latent:** yes (C++ schedule).
- **Splits:** test:241 uses `c.index` and :572/:798 use `replay.chords[i]` for the same lookup.
- **Ruling quotes found:** none found.
- **Open:** none.

### L67 Test fakes for the recorder, menu, frida, bot-run and SP watch tests
- **Source ids:** W2B-B5, W2B-C3, W2B-F3, W2B-F4, W2B-F5, W2A-K (test_sp_tools.py:78-80 run-cap fake is in L74; no test compares a built .js to its source or the baked menu_fields to the file).
- **Verdict:** UNVERIFIED (V3 and V4 list B5, C3, F3, F4, F5 as not verified).
- **Homes:** `test_machine_recorder.py:43-69` (fake table packed with the parser's own M.PROC_* / M.THREAD_* constants), `test_menu_tools.py:35-56` (fake `state()` vs `menu.ts readClass`), `test_frida_attach.py:169-172, 192-198` (FakeScript.post and FakeSession.detach deliver synchronously; real Frida delivers on its own thread; `frida_attach.py:147-153` takes a lock for that), `test_bot_run.py:539-541` (FakeLaunchAgent PATCHED keys vs launch_core.ts status), `test_sp_tools.py:48, 87` (all-zero snapshot; `FakeProc.verify_targets` does nothing), `test_trace_hits.py:49-51`.
- **Evidence:** By reading. W2B compared the recorder offsets with the documented layouts from memory (all agree). No difference found in C3 or F4; F4: the `module` value {base, size} not compared with ModuleSeen's type (`launch_core.ts:15`). The fake menu state never returns `entry.fields` or `{found: false, reason}`; `_class_entry` handles them, untested here.
- **Latent:** no.
- **Splits:** none found.
- **Ruling quotes found:** none found.
- **Open:** LS-T1 per the plan (live proof of the recorder offsets; plan not read).

---

# Section H: other

### L68 Rules Python works out that Hydra or the reference already states
- **Source ids:** S1-C9 (bullets other than the window formula in L46 and the FirstNote note in L11).
- **Verdict:** UNVERIFIED (V3 and V4 list S1-C9 as not opened).
- **Homes:** `play_chart.py:595-599` (FirstNote rule read from the score file's note order), `:736-744, 131` (`_TOP_PAD_ORDER`, FL-09, comment "unchecked until a run shows it", row P22), `:108-113, 488-527` (order groups by multiplier, SC-38), `:765, 899` (sqout_tick >= 0 / < 0 sentinel), `:117` (SQUEEZE_IN / SQUEEZE_OUT strings mirror SPSqueeze::type_name), `:83, 88, 95, 113` (user-set numbers cite plan pages; one marked "(removed; git show ...)").
- **Evidence:** By reading. The finder did not open the plans to check the quoted answers.
- **Latent:** no.
- **Splits:** none stated beyond L46 and L47.
- **Ruling quotes found:** none found.
- **Open:** none.

### L69 Empty and edge inputs to the player
- **Source ids:** S1-C16.
- **Verdict:** UNVERIFIED.
- **Homes:** `play_chart.py:1375-1377` (no chords), `:680` (`_dump_path` KeyError/IndexError), `:701` (`declined_chord_indexes`: `act["skipped_fill_ticks"]` KeyError), `:935` (`load_dump_notes`), TM-18/TM-19.
- **Evidence:** By reading. Chords not in time order: a negative gap gives max(gap/2, 37.5) = 37.5, silently accepted, no order check. One chord: 85.72 side. Two chords at the same ms: treated as no neighbour. `load_dump_notes` calls `load_dump_chords` with no dump, so a squeeze-out score file raises there (documented at 925-931 for `load_score_times` only).
- **Latent:** yes for most.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none (the finder did not re-read TM-18/19 fully).

### L70 The kit press reply reports the agent's own writes, not what the game read
- **Source ids:** F2-S2.
- **Verdict:** UNVERIFIED (V3: not verified).
- **Homes:** `kit.ts:440-662, 860-968, 573-575, 61 (TICK_MS)`, `frida_attach.py:399-402`, `kit_tools.py:623-629, 843-851`.
- **Evidence:** By reading. A press that Rewired ignored (kit not on the player, wrong map) still returns ok; only the score rise shows it. `readBack` exists only for ch_kit_press diagnostics. A group's down time is checked on a tick (TICK_MS value not read by F2; F4 says 2 ms). `kit_tools.py:843-851` prints "?" for a missing down/up frame instead of failing (text tool only). Failed writes and releases are loud (throw with `error` and `fault`).
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L71 Other agents (.ts): quiet paths in state, trace, menu
- **Source ids:** F2-S3.
- **Verdict:** UNVERIFIED (V3: not verified).
- **Homes:** `state.ts:24-32` (menuBlock turns a failed menu read into `{error}` inside an ok reply), `trace.ts:160-162` (not_hooked listed, trace goes on; a trace with `hooked: 0` still runs to the cap), `trace_hits.py:208-213`, `menu.ts:233` (bare `catch {`), `:347` (`live.get(SONG_SELECT) ?? []`).
- **Evidence:** By reading. Not read in full: clock.ts, menu.ts (352 lines), menuhooks.ts, launch.ts, launch_core.ts, hooks_common.ts, gamemanager.ts (W2A read these later; see L38, L41).
- **Latent:** unknown.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L72 Server and tool reply quiet paths
- **Source ids:** F2-S4.
- **Verdict:** UNVERIFIED (V3: not verified).
- **Homes:** `frida_attach.py:405-410`, `sp_watch.py:141-143, 120`, `engine.py:273-290`, `sp_compare.py:166`, `engine_finder.py:135-136, 216-217, 183-186, 226`, `game_tools.py:195-198, 1058, 1477-1478, 1455-1464, 1485, 1499, 867-869`, `quiet.py:216-223, 164`, `compare_scorestats.py:85`, `machine_report.py`, `install_chart.py:85`, `machine_recorder.py:332-343, 372`.
- **Evidence:** By reading. Seven items: a reply after detach is dropped; a failed SP watch read ends the watch with a reason and the CSV is short (sp_compare then FAILs per missing item); clock columns blank and `load_watch_rows` raises on a blank cell; engine_finder picks the first engine whose clock changed, scan order decides ties; play-thread errors shown only by ch_play_status; a failed press-log write sets `log_note` and the run goes on; quiet.py read failures do not set `quiet = False`; compare_scorestats raises KeyError caught as "compare failed: KeyError".
- **Latent:** unknown.
- **Splits:** two engine searches in one run are shared only in ch_run_chart (capped normal-mode search vs sp_watch's all_patterns search).
- **Ruling quotes found:** none found.
- **Open:** none.

### L73 Smaller timing notes (clock.ts, ms integers, spin thresholds, three song-time sources, conversions)
- **Source ids:** F4-T5.
- **Verdict:** UNVERIFIED.
- **Homes:** `clock.ts:14` (SAMPLE_MS 250, `Date.now()`), `live.py:113-115` (_SPIN_WITHIN_S 40 ms, _WAKE_BEFORE_S 30 ms), `constants.py:208-218` (`s_to_ms`, `ms_to_s`).
- **Evidence:** Proven by R-M0 `press-1.csv`: read-to-clock error median -0.5 ms, max early -2.0; `down_ms`/`up_ms` are `Date.now()` integers so differences carry a 1 ms error. Three song-time sources (engine song_clock in memory, GameManager.songTime in clock.ts, audio_s in sp_watch) for different tools, not compared. Smallest chart gap in R-M0 68.5 ms.
- **Latent:** no.
- **Splits:** three song-time sources.
- **Ruling quotes found:** none found.
- **Open:** none.

### L74 ch_sp_watch_start reports a song-based run cap that may be the default cap
- **Source ids:** W2A-2.
- **Verdict:** V3 CONFIRMED.
- **Homes:** `sp_tools.py:197-202, 353-362`, `game_tools.py:171-181` (`run_cap_s`), `bot_run.py:387-403`, `tests/test_sp_tools.py:78-80` (FakeGame.run_cap_s returns 240.0, never raises or falls back).
- **Evidence:** By reading. `run_cap_s` catches SongLengthMissing itself and returns `play_chart.RUN_CAP_S`, so the `except` in sp_tools is unreachable and the line "run cap: N s (game_tools.run_cap_s for <folder>)" is wrong for a folder with no length. bot_run's message says "nothing was started" but the callers continue. `_song_end_ms` returns None and the gap line says "song length not known".
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L75 Time conversions, Ready level, gap constants, three clocks, attach delay
- **Source ids:** W2A-8.
- **Verdict:** V4 CONFIRMED by reading.
- **Homes:** `constants.py:211` (`s_to_ms`), `sp_compare.py:212-213` (`_ms`), `sp_compare.py:114-122, 288-292` (`at_ready_level`: `amount >= 0.5 * sp_max` in floats), `timing.h:48, 51, 254`, `model.h:37`, docs/clone-hero/05-star-power.md SP-16 (:167-169), `sp_tools.py:50` (GAP_MS = 50), `server.py:75-77`, `frida_tools.py:41` (ATTACH_DELAY_S = 5.0).
- **Evidence:** By reading. Both conversions multiply by 1000, so no number differs today. For an odd sp_max the float half and the game's rounded half (+0x1B8) can differ by one; R-M2 sp_max 14400 (even). GAP_MS was "the number LS-O2's hand count used", user-answered question 7 per its comment; ATTACH_DELAY_S user-approved per comment.
- **Latent:** yes for the odd-sp_max difference.
- **Splits:** `C.s_to_ms` vs `sp_compare._ms` (same arithmetic); float half vs the game's rounded half.
- **Ruling quotes found:** none found (comments only).
- **Open:** none.

### L76 Small items in the server, SP watch, menu and menuhooks
- **Source ids:** W2A-10.
- **Verdict:** V3 PARTLY (confirmed: `server.py:105-115` reads private fields `kit._agent`, `sp._watch`, `method_watch._session`, `frida._trace`, so a rename raises AttributeError in ch_open_items; the others not opened by V3 or V4).
- **Homes:** `server.py:105-115`, `sp_tools.py:431-440, 370`, `frida_tools.py:96`, `menu.ts:230-236, 242-258`, `menuhooks.ts`, `start_candidates.json`, `kit_methods.json`.
- **Evidence:** By reading. A watch thread alive after READ_CAP_S keeps its handle and CSV; a new ch_sp_watch_start is refused until it ends. `out` as an absolute path writes anywhere. A state read with no `songLoaded` key (an older state.js) is shown as "loading". `isLivePlayerScreen` may read reused memory. menuhooks.ts hooks "all" of PlayerSelectionMenu's methods including the per-frame one menu.ts already hooks (two Interceptor hooks on one address). kit_methods.json names match kit.ts ALLOWED_CALLS exactly (40 and 40, set difference empty, checked with a script).
- **Latent:** unknown.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L77 machine_report may crash instead of refusing
- **Source ids:** W2B-B3.
- **Verdict:** V3 NOT CONFIRMED for `statistics.quantiles` (Python 3.14 in use; 3.13+ no longer raises for one point, from memory of the docs, not run); V3 PARTLY for `_share` division (zero only if all stamps are equal; `:541` requires two samples).
- **Homes:** `machine_report.py:487, 243, 242-243, 218-229, 541`.
- **Evidence:** By reading; `py --version` 3.14.5, `python --version` 3.14.7 (V3).
- **Latent:** yes on this Python.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** Python version in use decides (V3).

### L78 settings.ini is read by two parsers that disagree
- **Source ids:** W2B-B4.
- **Verdict:** V3 PARTLY (`machine_report.py:50-52` says framerate/vsync sit in [video] and game_files reads [game] only, so the difference is stated in a comment; whether the report names the section not checked).
- **Homes:** `machine_report.py:250-263`, `game_files.py` (`_game_setting_line`).
- **Evidence:** By reading.
- **Latent:** no.
- **Splits:** any-section match vs [game] only.
- **Ruling quotes found:** none found.
- **Open:** none.

### L79 recorder_link.start re-raises, but the caller continues
- **Source ids:** W2B-B6.
- **Verdict:** V4 NOT CONFIRMED as a problem (caller `game_tools.py:1221-1226` catches Exception and adds one reply line).
- **Homes:** `recorder_link.py:108-110, 107`.
- **Evidence:** By reading.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L80 Recorder samples fall behind silently (reported in gap stats)
- **Source ids:** W2B-B7.
- **Verdict:** V4 CONFIRMED.
- **Homes:** `machine_recorder.py:~429-431`, `machine_report.py:280-284`.
- **Evidence:** By reading. The report prints median, worst and "over GAP_MS" gaps.
- **Latent:** no.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L81 Song length has four homes; two song.ini lines give different answers
- **Source ids:** W2B-D1.
- **Verdict:** V3 CONFIRMED as split.
- **Homes:** `probe_songs.py:201` (last note + 3000, used by `trace_hits.trace_cap_s`), `game_files.py:357-367` (first `song_length` line wins, setdefault; `song_ini_length_ms`), `game_files.py:386` / `:~383` (Hydra's `read_song_ini_keys`: last line wins), `bot_run.py:387-403`, docs/clone-hero/01-reading-a-chart.md:465 (the game ends the song by GameManager songLength).
- **Evidence:** By reading. A song.ini with two `song_length` lines: run cap and silent ogg use the first, Hydra the last. D255 says one song end everywhere; the game's real choice not checked.
- **Latent:** yes for charts with two lines.
- **Splits:** song.ini first line vs last line vs last note + 3000 vs GameManager songLength. Input with two `song_length` lines: game_files first, Hydra last.
- **Ruling quotes found:** none found in these reports (D255 is named, "one song end everywhere"; memory index entry, not a quote).
- **Open:** none.

### L82 install_chart copies the whole source folder and joins `--name` unchecked
- **Source ids:** W2B-D2, W2B-D3.
- **Verdict:** V3 CONFIRMED (both).
- **Homes:** `install_chart.py:82` (copytree, then only song.ogg overwritten), `:78` (`--name` joined onto dest_root), `:89` (rmtree on failure), `:85`.
- **Evidence:** By reading. Other audio in the source (drums.ogg, guitar.ogg, song.mp3) stays and the game will play them; a name with `..` writes outside the probe folder. Low risk, hand-run tool.
- **Latent:** yes unless a source folder has other audio.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L83 edge_probe has three homes for "lead" and an uncaught KeyError
- **Source ids:** W2B-E1, W2B-E2.
- **Verdict:** V3 PARTLY for E2 (`edge_probe.py:124`; `_first_presses` not read); E1 UNVERIFIED.
- **Homes:** `edge_probe.py:55-65` (`mark_row` moves chart_ms and lead_ms back), `:124` (`first[p.index].lead_ms`), `probe_row_lines`.
- **Evidence:** By reading. PROBE_COLUMN (asked), the adjusted lead_ms, and `chart_ms - clock_ms_at_press` are three forms. A chord with no press in `press_plan` (e.g. declined empty) would raise KeyError (not caught).
- **Latent:** yes (probe tool).
- **Splits:** three forms of "lead".
- **Ruling quotes found:** none found.
- **Open:** none.

### L84 analysis.find_window_edge and overlay_logs: no problem found (W2B-E4)
- **Source ids:** W2B-E4.
- **Verdict:** UNVERIFIED (V3 and V4 list E4 as not verified).
- **Homes:** `analysis.py` (`find_window_edge`), `overlay_logs.py`.
- **Evidence:** By reading. An exact-threshold hit counts as inside (`mag > threshold`), misses are `mag <= threshold`; candidate edges are midpoints so an exact-edge sample is never at a threshold. overlay_logs.py is column lists only. The second formula copy is in L46.
- **Latent:** yes.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

### L85 trace_hits raises uncaught on an empty manifest
- **Source ids:** W2B-F2.
- **Verdict:** V3 CONFIRMED by reading (whether the manifest can be empty not checked).
- **Homes:** `trace_hits.py:157` (`min(notes, ...)` ValueError), `:119` (`notes[-1]` IndexError).
- **Evidence:** By reading.
- **Latent:** yes unless a loader can produce an empty manifest.
- **Splits:** none.
- **Ruling quotes found:** none found.
- **Open:** none.

---

# Unverified (no verifier verdict at all; each is also carried in a row above)

Verifiers listed these as "not verified" or never opened them. The L-id is the row that carries the content.

- S1-C9 (L68), S1-C10 (L7), S1-C15 (L15), S1-C16 (L69), S1-C17 (L65; V1 F1-C6 answers it in part).
- F2-S2 (L70), F2-S3 (L71), F2-S4 (L72).
- F3.F4-8 (L15), F3.F8-15 and F3.F8-21 (L60).
- F4-F9-4 (L64), F4-T2 and S1-C13 clock half (L18), F4-T3 (L5, L13; the finder's edge table), F4-T5 (L73).
- W2A-F (L61; V2's wider search supports it but V3 and V4 did not verify), W2A-K (L67).
- W2B-B5, W2B-C3, W2B-F3, W2B-F4, W2B-F5 (L67), W2B-D4 and W2B-G (L61), W2B-E1 (L83), W2B-E4 (L84).
- Parts of otherwise-verified rows that no verifier opened: S1-C8 sub-items `:683-686, :761, :817/836, :838, :1128` (L14); F2-S1i warnings claim (L55); W2A-1 baked menu_fields diff (L38); W2A-10 items other than `server.py:105-115` (L76); V2-N1 chord 1894 (L3); F3-1's open question on `formula_*` pinning (L46).

# Coverage

Candidates in (finder items counted as listed): S1-calibrate 17 (C1-C17); F1 6 (C1-C6); F2 22 (J1-J8, S1a-S1j with the S1 intro folded into S1b, S2-S5); F3 28 (F3-1, F3-2, F3-3, F3-4, F3-5a, 5b, 5c, 5d; F3.F4-1 to F4-9; F3.F8-2/3, 4, 5, 6, 9, 14, 15, 16, 19, 20, 21); F4 9 (F9-1 to F9-4, T1 to T5); W2A 12 (items 1-10, focus section, test-fake section); W2B 29 (A1-A5, B1-B7, C1-C3, D1-D4, E1-E4, F1-F5, G) = 123 finder candidates. Plus 2 handoff claims (H1, H2) and 6 verifier-found extras (V1-N1, V1-N2, V2-N1, V2-N2, V2-N3, V2-N4; V4-NEW is the same finding as L01). Total in: 131.

Rows out: 85 (L01 to L85), in sections A 20, B 15, C 8, D 12, E 5, F 4, G 3, H 18.

Unverified: 26 candidate ids had no verifier verdict (S1-C9, C10, C15, C16, C17; F2-S2, S3, S4; F3.F4-8, F3.F8-15, F3.F8-21; F4-F9-4, F4-T2, F4-T3, F4-T5; W2A-F, W2A-K; W2B-B5, C3, D4, E1, E4, F3, F4, F5, G), plus the partial sub-claims listed above. Each also sits in a row in A-H with Verdict UNVERIFIED, so nothing is dropped.

Counting note: a few candidates are split across rows by sub-item, because their parts are separate findings: F4-T1 (L06, L10, L45, L48), S1-C8 (L01, L14), S1-C9 (L11, L46, L68), S1-C13 (L16, L18), F2-S1 intro (L44, L50), F2-S1j (L45). Their ids are listed once in the row that holds most of the content and mentioned in the others.

Reports not finished: none. All eleven reports were read in full. Not read by this compile: the run files, source files and pages themselves; every claim above is copied from a report and carries that report's evidence label.
