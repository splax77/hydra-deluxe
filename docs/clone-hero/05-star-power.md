# Star Power [SP]

This page covers Star Power phrases, the meter, activation, drain, the Star Power end and squeezes.
It describes build 1.1 (v1.1.0.6142).
Last write-back: 2026-11-08.

The live-run files named below are the `run1_cal0_*` files under tools/ch_probe/experiments/results/overlay-spike/. "Run 1" is the key-press auto-player run on the probe chart Trippin SP Spike (resolution 192), stamped 2026-10-29 00:39 UTC. The engine fields are those of the base engine class #8288, which the drum engine #8255 inherits. `ghidra_decompile_all.txt` and "the decompile" mean Ghidra's full decompile of build 6142, which `code/README.md` describes; it is not in the repo.

## Phrase membership

### SP-01 A note is inside a phrase when start <= tick < start + length

Rule: A note belongs to a Star Power phrase when the phrase start is at or before the note's tick and the note's tick is before start plus length. A note exactly on the end tick is outside the phrase.
Label: Proven in the game's code
Evidence: The start test is at lines 72 to 74 and the end test at lines 93 to 97 of the listing; the end is start plus length at lines 54 to 57. The 2026-10-03 reading of the same function (ch-evidence.md question 21, lines 41 to 66 at commit 5fbe71ae) said the same; its listing was lost.
Game code (6142): RVA `0x20D2440` (class #8265, scrambled name), `code/6142/sp_phrase_assign_20D2440.asm` lines 54 to 57, 72 to 74 and 93 to 97
Hydra: `close_sp_phrase` in src/parse/song.cpp line 400 (both parsers call it, lines 804 and 1413)
Ruling: D21. The user chose "Yes, one helper (Recommended)" to "SP phrase edges: should .chart use Clone Hero's rule ..." (card 2026-10-04-bbf41dc4, 01:53 UTC). Hydra and the game agree.
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-02 A zero-length phrase pays nothing

Rule: A phrase of length 0 marks no note, so it awards nothing, even with a chord on its tick.
Label: Proven in the game's code
Evidence: With length 0 the start test (lines 72 to 74) and the end test (lines 93 to 97) can never both pass.
Game code (6142): RVA `0x20D2440`, `code/6142/sp_phrase_assign_20D2440.asm` lines 72 to 74 and 93 to 97
Hydra: `close_sp_phrase` in src/parse/song.cpp line 400 (its test is never true at length 0)
Ruling: none needed: same rule (D21, as in SP-01)
Found: 2026-10-03, session cebf3006 (first reading); 2026-10-30, session b02ab3e6 (the reference build)

### SP-03 A phrase that runs past the last note pays on the last note

Rule: When a note inside a phrase has no next note, it counts as the phrase's last note, so a phrase that runs past the end of the chart still awards on the chart's last chord.
Label: Proven in the game's code
Evidence: Lines 75 to 84 leave the next note empty on the last note; lines 99 to 100 skip the "next note is still inside" exit when it is empty; lines 104 to 133 mark the last chord.
Game code (6142): RVA `0x20D2440`, `code/6142/sp_phrase_assign_20D2440.asm` lines 75 to 84 and 99 to 103
Hydra: `close_sp_phrase` in src/parse/song.cpp line 400, called once more after the last tick
Ruling: none needed: same rule (D21, as in SP-01)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-04 The phrase's last chord carries the award flag

Rule: Every note inside a phrase gets flag 0x80. Every note of the phrase's last chord also gets flag 0x100, which is the award. The last chord is the last note whose next note starts at or after the phrase end.
Label: Proven in the game's code
Evidence: Line 98 sets 0x80; lines 99 to 103 are the "not last" exit; lines 104 to 133 walk the chord and set 0x100 on each member at line 128.
Game code (6142): RVA `0x20D2440`, `code/6142/sp_phrase_assign_20D2440.asm` lines 98 to 133
Hydra: `close_sp_phrase` in src/parse/song.cpp line 400 marks the last chord's timestamp
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-05 The phrase cursor steps forward one phrase per note

Rule: The game walks the notes and the phrases together. When a note sits at or past the current phrase's end, the cursor moves forward by exactly one phrase (an if, not a loop). Past the last phrase it keeps the last phrase's start and end.
Label: Proven in the game's code
Evidence: One increment and one call to the advance helper, then one re-test, at lines 85 to 97; the helper leaves the cursor alone past the last phrase at its lines 14 to 17 and 24 to 27.
Game code (6142): RVA `0x20D2440` lines 85 to 97; helper RVA `0x20D0330`, `code/6142/sp_phrase_advance_20D0330.asm` lines 14 to 27
Hydra: no owner: `close_sp_phrase` (src/parse/song.cpp line 400) closes each phrase on its own and keeps no cursor
Ruling: none needed: no chart is shown to differ (see SP-06)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-06 Nested or overlapping phrases may never award in the game

Rule: Because the cursor steps one phrase per note (SP-05), a phrase that sits inside another, or starts before the previous one ends, may never be reached in the game and so may award nothing. Hydra awards every phrase.
Label: Unknown
Evidence: not checked: it needs the sort order of the track's phrase list, which the loader and the clean-up at `0x5DB320` would show; neither was opened.
Game code (6142): RVA `0x20D2440` (the step, SP-05); the sort order: not found
Hydra: `close_sp_phrase` in src/parse/song.cpp line 400 (awards every phrase independently)
Ruling: none yet: the game's rule is not settled, so no difference is shown
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-07 .chart and .mid read a phrase the same way

Rule: A .chart phrase (`S 2 <len>`) and a .mid phrase (note 116 on and off, length = off minus on) are stored the same way and go through the same membership rule.
Label: Unknown
Evidence: the 2026-10-03 listings were not kept (read at `0x213E0C0` (.chart) and `0x2155050` (.mid), session cebf3006, ch-evidence.md lines 55 to 58 at commit 5fbe71ae). The committed `.chart` special-event listing `docs/audit/drum-lanes/spec_213e0c0.asm` lines 20 to 59 shows type 2 building an item through the method at 0x20D5E60 (line 43) and adding it to a list (lines 47 to 59); what the item holds and that it is the phrase list were not read. One side is shown live: LS-D2d of `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md)`, 2026-11-06, earned all 25 note-116 phrases of a .mid chart (`C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-06\ls-d2d-run1\`). The .chart side and the sameness were not checked by that run.
Game code (6142): the `.chart` type-2 branch, RVA `0x213E0C0`, `docs/audit/drum-lanes/spec_213e0c0.asm` lines 20 to 59; no phrase read found in `0x2155050`; the reference build's working listings were not kept
Hydra: `close_sp_phrase` in src/parse/song.cpp line 400, called by the .mid reader (line 804) and the .chart reader (line 1413)
Ruling: none needed: D21 put both parsers on one helper (SP-01)
Found: 2026-10-03, session cebf3006; the .mid run, 2026-11-06, session 70bfa33d (LS-D2d); the type-2 listing 2026-11-08, session 77057df2 (docs-gaps search)

### SP-08 A clean-up step drops phrases that hold no note

Rule: A clean-up deletes phrases with no note in [start, start + length - 1] (or [start, start] at length 0). A zero-length phrase on a note survives it but still awards nothing (SP-02), so the step changes no score.
Label: Unknown
Evidence: the 2026-10-03 listing was not kept (session cebf3006, ch-evidence.md line 64 at commit 5fbe71ae). The committed listing of the caller, `docs/audit/note-shuffle/step0/open2/f215c320.asm`, has five references to `0x5DB320`, not three: calls at lines 101, 106 and 116 and jumps at lines 113 and 123. Nothing in it ties any of them to the phrase list. The decompile lines 47834 to 47839 show three of them. The body of `0x5DB320` is in no committed file.
Game code (6142): `0x5DB320`, called from `0x215C320`, `docs/audit/note-shuffle/step0/open2/f215c320.asm` lines 101 to 123; body not found
Hydra: no owner: Hydra never keeps a phrase with no note (looked in src/parse/song.cpp)
Ruling: none needed: no score effect
Found: 2026-10-03, session cebf3006; the caller's listing 2026-11-08, session 77057df2 (docs-gaps search)

## The meter and the full bar

### SP-09 The meter is a whole-number count with a fixed full bar

Rule: The Star Power meter is a signed 8-byte whole number in the engine. The full bar is a read-only 8-byte number beside it. Both belong to the base engine class.
Label: Proven in the game's code
Evidence: dump.cs lines 529974 and 530037 declare both fields of class #8288 (TypeDefIndex 8288, line 529952); the update reads both at lines 9 to 15 of its listing. The probe names them `OFF_SP_AMOUNT` and `OFF_SP_MAX` in tools/ch_probe/constants.py.
Game code (6142): `#8288+0x68` (meter), `#8288+0x1B0` (full bar); RVA `0x20F6800`, `code/6142/sp_update_20F6800.asm` lines 9 to 15
Hydra: Hydra counts bars, not units: `kCloneHeroSpCap` (4 bars) in src/core/model.h line 37
Ruling: none needed: same rule in different units (SP-11)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-10 A quarter bar is 7.5 times the chart's tick base

Rule: The engine constructor sets a quarter bar to the chart record's tick base times 7.5, rounded to a whole number. Half a bar is two quarters and the full bar is four.
Label: Proven in the game's code
Evidence: Line 158 loads the chart double, line 162 multiplies by the constant 7.5, line 164 rounds, line 166 stores the quarter at +0x1C0, line 168 stores the half at +0x1B8 and line 170 stores the full bar at +0x1B0. The rounding routine's tie rule was not read. The same lines are in the older committed docs/audit/note-shuffle/step0/open34/engine_ctor_20f79e0.asm lines 162 to 170.
Game code (6142): RVA `0x20F79E0` (the base engine constructor), `code/6142/engine_ctor_20F79E0.asm` lines 157 to 170
Hydra: no owner for a unit meter: looked in src/ for 7.5, bar_ticks and sp_bar; Hydra's bar is `kMeasuresPerSpBar` (2 measures) in src/core/timing.h line 254
Ruling: none needed: no score depends on the unit (SP-11)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-11 The full bar is 30 times the resolution; Hydra's is 32 in 4/4

Rule: The game's full bar read 5,760 on a resolution-192 chart and 14,400 on a resolution-480 chart, which is 30 times the resolution. Hydra's full meter is 4 bars of 2 measures, which is 32 times the resolution in 4/4. The two are different units for the same length of Star Power (SP-31).
Label: Proven in a live run
Evidence: Run 1: `run1_cal0_sp_compare.txt` ("PASS sp_max constant: 5760; Hydra full meter 6144 (resolution 192 x 32)"), `run1_cal0_scorestats.json` (sp_bar_ticks 5760) and `run1_cal0_sp_watch.csv` (5760 in all 40,922 rows). Bot run on Window Map (resolution 480, read from its notes.chart): sp_bar_ticks 14400 in C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\l1\bot\scorestats-Window_Map-20261028-205656.json.
Game code (6142): `#8288+0x1B0`, set at RVA `0x20F79E0` (SP-10)
Hydra: `kCloneHeroSpCap` in src/core/model.h line 37 and `kMeasuresPerSpBar` in src/core/timing.h line 254
Ruling: none needed: the units differ but the end times agree on the run (SP-31); the case where they could differ is SP-15
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-12 A phrase adds a quarter bar to the meter

Rule: Collecting a phrase adds one quarter of the full bar to the meter (1,440 at resolution 192), so four phrases fill it. The gain method adds the amount to the meter and to a running total, and sets a "meter changed" byte.
Label: Proven in the game's code
Evidence: The gain method adds the amount to +0x68 and +0x80 at lines 19 to 22 and sets +0x130 at lines 44 to 57. Live: run 1's `run1_cal0_sp_compare.txt` shows 11 of 11 phrase rises of 1,440 (1,438 or 1,439 for the three collected during Star Power, where drain overlaps the poll). Which call passes the quarter on a phrase hit was not traced (see SP-35).
Game code (6142): RVA `0x20F4FA0`, `code/6142/sp_gain_20F4FA0.asm` lines 19 to 22 and 44 to 57
Hydra: `kMeasuresPerSpBar` in src/core/timing.h line 254 (one phrase is one bar of two measures)
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-13 The "meter changed" byte stays set until Star Power uses it

Rule: The byte at +0x130 means "the meter changed, work the end time out again". Every gain sets it. Only the update that runs while Star Power is on clears it, after it has worked the end out again. So it stays set while Star Power is off and the meter holds phrases.
Label: Proven in the game's code
Evidence: Set at `code/6142/sp_gain_20F4FA0.asm` lines 44 to 57; read at `code/6142/sp_update_20F6800.asm` lines 77 and 78 and cleared at line 105, on the active path only. Live: in `run1_cal0_sp_watch.csv` the byte is 1 in 22,890 rows, every one with Star Power off and 1,440 or 2,880 in the meter, and 0 whenever Star Power is on or the meter is empty. This explains why the byte is set with no overfill. tools/ch_probe/constants.py names it `OFF_SP_OVERFLOW`, but it is not an overflow flag.
Game code (6142): RVA `0x20F4FA0` lines 44 to 57; RVA `0x20F6800`, `code/6142/sp_update_20F6800.asm` lines 69 to 106
Hydra: no owner: Hydra works out each end directly, not per frame
Ruling: none needed: an internal flag
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-14 One unexplained tick at activation 1

Rule: Run 1's first activation started from a meter of 2,879, not 2,880, and its game end sat 2.9 ms after Hydra's. The cause is not known.
Label: Unknown
Evidence: `run1_cal0_sp_watch.csv`, first active row (meter 2879; later activations 2880); `run1_cal0_sp_compare.txt`: game end 39,488.3 ms against Hydra's 39,485.4 ms. Decision log D15 accepts up to one tick of drain difference in the Preview gauge, not in the end time. Looked in the activation and update listings for the cause; not found.
Game code (6142): RVA `0x20F5640` (SP-19) uses the meter as it is, not whole bars
Hydra: `activation_initial_end_times` in src/search/graph.h line 167 (whole bars from `kSpActivationBars` to the cap)
Ruling: none needed: no rule is shown to differ
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-15 Whether Star Power length follows the time signature

Rule: Hydra makes one bar last two measures, counted continuously across a time signature change (a part measure counts as its share of the old measure). Whether the game's drain follows the time signature is not settled by the code read so far.
Label: Unknown
Evidence: Two readings. The end-time listing converts the meter to time through the tempo segments only, with no time-signature list, but its tick formula at `0x20D1D20` reads two segment fields, +0x30 and +0x40, whose meaning was not read, so a time signature could hide there. The 2026-10-12 session read a measure-based clock at `0x20D2930` with constants 4.0, 8.0 and 30.0 (listing lost; read 2026-10-12 at `0x20D2930`, session 28fad19c; docs/superpowers/plans/2026-10-12-one-record-per-chart/part-B2-timing.md line 23, removed; `git show ec91f8d1:docs/superpowers/plans/2026-10-12-one-record-per-chart/part-B2-timing.md`, says "Neither function is in the evidence folder"). The 30 times resolution full bar (SP-11) fits 8 measures of 3.75 times resolution. Run 1 is 4/4 under both compared activations. Hydra's own figure for a 36/4 section at 126 BPM (34.3 s per bar, card 18665965) is arithmetic on Hydra's rule.
Game code (6142): RVA `0x20F37E0`, `code/6142/sp_end_time_20F37E0.asm` lines 15 to 93; tick formula `0x20D1D20` read, not listed; `0x20D2930` listing lost
Hydra: `MeasureCount` in src/core/timing.h line 185; `SongTiming::plusmeasure` in src/core/timing.cpp line 320; `sp_bars_to_measures` in src/core/timing.h line 255
Ruling: D231: the user chose "Adopt CH's rule (Recommended)" to "Clone Hero measures Star Power's length continuously ..." (card 2026-10-12-28fad19c, 03:14 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

## Activation on drums

### SP-16 Activation needs Star Power off and at least half the meter

Rule: The player can activate only when Star Power is off and the meter holds at least half the full bar (two quarters). Hydra's 2 bars out of 4 is the same ratio.
Label: Proven in the game's code
Evidence: The can-activate test returns false when +0x42 is set and otherwise tests that +0x1B8 is not above the meter (ghidra_decompile_all.txt lines 52809 to 52819, Ghidra's 2026-09-17 decompile, not in the repo); +0x1B8 is half the bar at `code/6142/engine_ctor_20F79E0.asm` line 168. No listing of the test was made.
Game code (6142): RVA `0x20F57F0` (decompile lines 52809 to 52819); constructor RVA `0x20F79E0` line 168
Hydra: `kSpActivationBars` in src/core/timing.h line 51
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-17 Drums activate only by hitting a fill's activation note

Rule: On drums, Star Power activates only inside a hit. Five hit handlers each test that the player kind is not a remote player, that Star Power can activate, and that the hit note carries flag 0x800; then they call the activation. Flag 0x800 is the one the fill code puts on a fill's activation note.
Label: Proven in the game's code
Evidence: The flag test is lines 1 to 3 of the listing (test of note +0x7C against 0x800). The five call sites were found by a search for the activation's virtual slot and read in the decompile at lines 149 to 154, 347 to 352, 568 to 573, 1343 to 1348 and 1566 to 1571. The committed press handler docs/audit/drum-lanes/play/f20deaa0.asm lines 112 to 130 shows the same order. The dynamics hit handler `0x20DE770` does not test the flag itself: its ten calls in `code/6142/note_hit_dynamics_20DE770.asm` (lines 64 to 136) include none to `0x214BB50`.
Game code (6142): handlers RVA `0x20DCC90`, `0x20DD190`, `0x20DD700`, `0x20DEAA0` and `0x20DEFF0`; flag test RVA `0x214BB50`, `code/6142/note_is_activation_214BB50.asm` lines 1 to 3; the dynamics handler RVA `0x20DE770`, `code/6142/note_hit_dynamics_20DE770.asm`
Hydra: `Song::check_activations` in src/core/song.cpp line 106 places the fills; the search picks the activation chord
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build); the dynamics handler 2026-11-08, session 77057df2 (docs-gaps search)

### SP-18 No other way to activate on drums

Rule: Drums have no activation button or tilt; the fill note is the only route.
Label: Unknown
Evidence: not checked: a search found only the five sites of SP-17 inside the engine's address range, three direct calls to the activation in the guitar family, and no drum counterpart to the guitar's button activators `0xFBC950` and `0xFBC9C0`; about 100 other call sites of the same virtual slot in other classes were not triaged. dump.cs was searched by name only for an input enum.
Game code (6142): RVA `0x20F5640` (slot 14); guitar activators `0xFBC950` and `0xFBC9C0`
Hydra: no owner: Hydra places activation on a fill's note and has no button model
Ruling: none yet: the game's rule is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-19 What activation does

Rule: Activation sets the active flag, stores the clock as the start time and the start tick, works out the end time from the meter as it is, adds one to an activation counter and doubles the multiplier.
Label: Proven in the game's code
Evidence: Lines 10 and 12 (flag +0x42, start time +0x110), 15 to 33 (start tick +0x118), 34 (end time from the meter at +0x68), 35 to 40 (counter +0xB4, multiplier +0x48 doubled, end time +0x120). Live: run 1 shows "multiplier doubles: 4 -> 8" and an end time ahead of the clock on all four activations.
Game code (6142): RVA `0x20F5640`, `code/6142/sp_activate_20F5640.asm` lines 9 to 40
Hydra: `kStarPowerMultiplier` in src/core/timing.h line 48; `activation_initial_end_times` in src/search/graph.h line 167
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-20 A missed activation note is hit for the player

Rule: When Star Power can activate, a missed fill-end note is hit automatically (the first branch of the miss handler). A press on the wrong pad while a fill-end note is in the window also hits that note, without activating.
Label: Proven in the game's code
Evidence: The committed listing calls the can-activate test at line 7, the flag test `0x214BB50` at line 14 and branches at line 16. docs/audit/drum-lanes/FINDINGS-play-time.md line 181 says whether Hydra matches was not checked. The wrong-pad case is the press handler `docs/audit/drum-lanes/play/f20deaa0.asm`: lines 107 and 108 remember a window note with flag 0x800 whatever pad was pressed, and after the loop lines 163 to 200 hit it through slot 11 (line 199) when bit 0 of its flag byte at `+0x84` is clear (the getter at 0x214B610, line 167; that this bit means "already hit" was not read), Star Power can activate and a pressed pad was left over (lines 175 and 176), without the slot-14 activation call of line 130. The byte at +0x40 tested at line 177 also gates it; its meaning was not read.
Game code (6142): RVA `0x20DCAD0`, docs/audit/drum-lanes/f20dcad0.asm lines 7 to 16; the press handler RVA `0x20DEAA0`, docs/audit/drum-lanes/play/f20deaa0.asm lines 107 to 108 and 163 to 200
Hydra: no owner found: looked in src/ for fill_end and lenient; page 4 owns the fill rows
Ruling: none needed: no difference is shown
Found: 2026-10-30, session b02ab3e6 (the reference build); the wrong-pad case 2026-11-08, session 77057df2 (docs-gaps search)

### SP-21 Whether the activating note scores doubled

Rule: The note that activates Star Power is itself scored with the doubled multiplier. In the press handler the activation runs first and doubles the multiplier, then the hit handler runs for the same note, and the hit handler ends in the score add, which multiplies the points by the multiplier as it stands then.
Label: Proven in the game's code
Evidence: `docs/audit/drum-lanes/play/f20deaa0.asm` line 130 calls virtual slot 14 (vtable offset 0x218) and line 152 calls slot 11 (offset 0x1E8) on the same note. On the drum engine slot 14 is `0x20F5640`, inherited from class #8288 (`dump.cs` line 530185), and slot 11 is the drum override `0x20DE770` (`dump.cs` line 527547); the drum class #8255 and its parent #8253 override neither slot 14 nor slot 20 (`dump.cs` lines 527281 to 527560). The activation doubles the multiplier at `+0x48` (`code/6142/sp_activate_20F5640.asm` lines 35 to 40). The hit handler ends by jumping to slot 6 with base points 50 (`code/6142/note_hit_dynamics_20DE770.asm` lines 126 to 135), which on the drum engine is `0x20DEEC0` (`dump.cs` line 527454); that jumps to `0x20F6EB0` (`code/6142/note_score_add_20DEEC0.asm` line 31), which multiplies by `+0x48` at `code/6142/add_note_score_20F6EB0.asm` line 40. The slot-to-offset step (slot n at vtable offset 0x138 plus 16 n) is the same one TM-20 and SP-29 use. `dump.cs` is at C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs, outside the repo; the user ruled it may be cited (2026-11-08, "Yes, cite them (Recommended)"). Not watched in a run.
Game code (6142): the press handler, RVA `0x20DEAA0`, `docs/audit/drum-lanes/play/f20deaa0.asm` lines 130 and 152; activation RVA `0x20F5640`; hit handler RVA `0x20DE770`; score add RVAs `0x20DEEC0` and `0x20F6EB0`
Hydra: no owner searched
Ruling: none yet: the game's rule is now read, and Hydra's side was not searched
Found: 2026-10-30, session b02ab3e6 (the reference build); the order 2026-11-08, session 77057df2 (docs-gaps search)

## Drain

### SP-22 The meter drains by the ticks that passed since the last update

Rule: While Star Power is on, each update takes the ticks that passed since the activation tick, subtracts the previous total, and takes that difference off the meter, never going below zero. The drain is whole units of the game's tick scale; tempo only turns ticks into seconds.
Label: Proven in the game's code
Evidence: Lines 24 to 30 save the previous total and work out the new one (current tick minus the start tick), lines 39 to 56 subtract the difference, clamped to the meter. The current tick comes from the clock through the tempo segment at lines 5 to 30 of the tick helper. ch-evidence.md lines 76 to 81 at commit 5fbe71ae said the same from a lost listing.
Game code (6142): RVA `0x20F6800`, `code/6142/sp_update_20F6800.asm` lines 24 to 56; tick helper RVA `0x20F3750`, `code/6142/sp_current_tick_20F3750.asm` lines 5 to 30
Hydra: no per-frame owner: `ScoreGraph::extend_deacts` in src/search/graph.cpp and `ScoreGraph::add_deact_edge` work out one end per activation
Ruling: none needed: same end, computed differently
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-23 A remote player's engine does not drain

Rule: The drain is skipped when the engine's player kind is RemotePlayer (value 1). Player, Bot and Replay engines drain.
Label: Proven in the game's code
Evidence: Lines 37 and 38 compare the 4-byte field at +0x190 with 1 and skip the drain. The field's enum is Player 0, RemotePlayer 1, Bot 2, Replay 3 (dump.cs lines 546294 to 546302).
Game code (6142): `#8288+0x190`; RVA `0x20F6800`, `code/6142/sp_update_20F6800.asm` lines 37 and 38
Hydra: no owner: Hydra has no network player
Ruling: none needed: outside Hydra's scope
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-24 A phrase that overfills the meter is cut to a full bar

Rule: When a phrase pushes the meter above the full bar, the meter is cut to the full bar and the overflow is thrown away. During Star Power the end time is then worked out again from the ticks already drained plus the full bar, which puts the end one full bar after the collecting moment. The same cut runs when Star Power is off.
Label: Proven in the game's code
Evidence: Lines 9 to 15 cut the meter while off; lines 69 to 76 cut it while on and set +0x130; lines 77 to 86 work out the new end time from drained plus meter. ch-evidence.md question 37 (lines 70 to 88 at commit 5fbe71ae) said the same from a lost listing, and added that CHOpt (another tool that copies the game's rules) agrees: its `add_phrase` keeps the meter at the smaller of the sum and a full bar. Live: no run has overfilled during Star Power (run 1's in-SP rises reached 2,158, 3,600 and 2,161, all under 5,760).
Game code (6142): RVA `0x20F6800`, `code/6142/sp_update_20F6800.asm` lines 9 to 15 and 69 to 86
Hydra: `ScoreGraph::extend_deacts` in src/search/graph.cpp (the ceiling is the collecting note plus the cap, from `sp_meter_cap_`). The game's cap is always one full bar; a Hydra cap other than 4 bars is a what-if setting with no game equivalent
Ruling: none needed: same rule (D29 records that `extend_deacts` stops at a full meter as the game does)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-25 What deactivation does

Rule: At the end, the game clears the active flag, halves the multiplier, sets the meter to zero (unspent Star Power is dropped), adds the time spent to a total, stores the clock as the last end and zeroes the end time and the drain counters.
Label: Proven in the game's code
Evidence: Lines 9 to 26 of the listing (flag +0x42, multiplier +0x48 halved, meter +0x68 zero, time total +0xB8, last end +0x128, end +0x120 zero). The committed docs/audit/drum-lanes/play/f20f5490_deactivate_sp.asm holds the same function. Live: run 1 shows the flag clearing and the multiplier halving 8 to 4 at each of four ends.
Game code (6142): RVA `0x20F5490` (slot 23), `code/6142/sp_deactivate_20F5490.asm` lines 9 to 26
Hydra: `ScoreGraph::add_deact_edge` in src/search/graph.cpp
Ruling: none needed: same rule
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-26 The Preview gauge drains to the stored end

Rule: Hydra's Preview gauge fills one bar per phrase, drains to the stored deactivation tick, and uses the record's cap as full scale. This is Hydra's display, not a game rule.
Label: Hydra assumes it, never checked
Evidence: `build_sp_meter_curve` in src/app/preview_view.cpp line 125. No game check exists.
Game code (6142): none (a display)
Hydra: `build_sp_meter_curve` in src/app/preview_view.cpp
Ruling: none needed: a display that reads the engine's end (cards 13441c5f and a28ef5f2 hold the user's derive-from-the-engine rule)
Found: 2026-10-30, session b02ab3e6 (the reference build)

## The Star Power end

### SP-27 The end is one stored time, worked out through the tempo map

Rule: At activation the game works out the song time at (activation tick plus meter) by walking the tempo segments, and stores it as one decimal time. It works it out again only when the meter changes during Star Power (SP-13, SP-24).
Label: Proven in the game's code
Evidence: Lines 15 to 45 find the start segment and add the meter in ticks; lines 46 to 77 walk whole segments; lines 78 to 83 add the last part segment. Its callers are the activation (`code/6142/sp_activate_20F5640.asm` line 34) and the update (`code/6142/sp_update_20F6800.asm` line 84). It reads no time-signature list; see SP-15 for what that leaves open.
Game code (6142): RVA `0x20F37E0`, `code/6142/sp_end_time_20F37E0.asm` lines 15 to 93
Hydra: `SongTiming::sp_end_ms` in src/core/timing.cpp line 340; `activation_initial_end_times` in src/search/graph.h line 167
Ruling: none needed: one end in both
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-28 Star Power ends in the first frame whose clock reaches the end

Rule: Each frame, the end check runs: if Star Power is on and the clock is at or past the stored end time, it calls the deactivation. A clock exactly equal to the end already ends it. The end is a time, not "the meter reached zero". The engine's event step also calls a handler with the end time when the end falls inside its step, but that handler only records the time and does not end Star Power.
Label: Proven in the game's code
Evidence: The end check compares clock with end at lines 1 to 3, tests the flag at lines 4 and 5 and jumps to slot 23, the deactivation (SP-25), at line 8. The drum engine's own slot 15 at `0x20DE5A0` is two instructions that jump there (dump.cs line 527475). The event step calls a virtual handler (vtable offset 0x238, slot 16) with the end time when the end lies after the old clock and before the new time (`code/6142/advance_events_20F6A50.asm` lines 219 to 230). The reviewer of this page disassembled slot 16 (tools/ch_mcp/chdis.py on the 6142 DLL, 2026-10-30): dump.cs gives it `Slot: 16` at RVA `0x20F43A0` in the base class and none of the drum engine's classes overrides it. It adds the time to a list of doubles at +0x180 when the list does not hold it yet, and it never touches the flag at +0x42: the committed listing `docs/audit/drum-lanes/play/f20f43a0_add_event.asm` lines 31 to 73 shows the duplicate test and the add. The end check also runs at those queued times between frames: `docs/audit/drum-lanes/play/f20f6ff0_next_event.asm` line 103 calls the frame update `0x20F4550` at each queued time it takes out, and that update calls the end check at `code/6142/base_frame_update_20F4550.asm` line 37 (page 3, TM-52). ch-evidence.md line 197 said "checked once per frame" from a lost listing.
Game code (6142): RVA `0x20F63B0` (slot 15), `code/6142/sp_end_check_20F63B0.asm` lines 1 to 8; drum override RVA `0x20DE5A0`; event step RVA `0x20F6A50` lines 219 to 230; its handler RVA `0x20F43A0` (slot 16), `docs/audit/drum-lanes/play/f20f43a0_add_event.asm` lines 31 to 73; RVA `0x20F6FF0`, `docs/audit/drum-lanes/play/f20f6ff0_next_event.asm` line 103
Hydra: one end point: `offset_from_sp_end` in src/core/model.h line 115; `paid_by_sp_walk` in src/core/backend_value.h line 28
Ruling: D269 call 4: "there should never be two different answers for where star power ends. ever." (card 2026-10-30-61cee421)
Found: 2026-10-30, session b02ab3e6 (the reference build); the queued checks 2026-11-08, session 77057df2 (docs-gaps search)

### SP-29 Whether a hit is scored before the end check in each frame

Rule: In each frame the game first advances its events to the new time, then runs the end check, then the drain. If a hit is scored inside that first step, a hit handled in the frame whose clock first reaches the end is still scored with Star Power on, even when its clock is past the end. Whether hits are scored there, or in an input callback outside this function, was not read.
Label: Unknown
Evidence: The order of the calls is shown: the frame update calls `0x20F4950` at line 17, slot 9 at line 21, slot 21 at line 33, slot 15 (the end check) at line 37, slot 18 at line 41 and slot 12 (the drain) at line 45; slot n is at vtable offset 0x138 plus 16 n. The step `0x20F4950` is committed as `docs/audit/drum-lanes/play/f20f4950_step_loop.asm`: when the byte at +0x290 is set it calls `0x20F6FF0` and moves the clock to its result up to three times (lines 18 to 60), running slot 8 at each (line 57), then puts the clock back (line 61); it shows no press handler or score add. `0x20F6FF0` is `docs/audit/drum-lanes/play/f20f6ff0_next_event.asm` (page 3, TM-52). What sets +0x290 was not read. Its callers are `0x20A1B0` (three call sites) and `0xFEB4C0`. Where a press is turned into a hit, and with which clock, is not read (SP-30).
Game code (6142): RVA `0x20F4550`, `code/6142/base_frame_update_20F4550.asm` lines 17 to 45; RVA `0x20F4950`, `docs/audit/drum-lanes/play/f20f4950_step_loop.asm` lines 18 to 61
Hydra: no frame model: Hydra pays a note by its chart time against one end (`paid_by_sp_walk` in src/core/backend_value.h line 28)
Ruling: none yet: the game's rule is not settled; questions page Q-50 holds the question that follows if hits are scored before the end check
Found: 2026-10-30, session b02ab3e6 (the reference build); reviewed 2026-10-30; the step listing 2026-11-08, session 77057df2 (docs-gaps search)

### SP-30 Whether a hit is judged at a clock between frames

Rule: The clock a hit is judged at may be the frame clock or one of up to three sub-step clocks between frames.
Label: Unknown
Evidence: both functions are committed. `docs/audit/drum-lanes/play/f20f4950_step_loop.asm` lines 18 to 60 set the clock up to three times, when +0x290 is set, from the result of `0x20F6FF0`. `docs/audit/drum-lanes/play/f20f6ff0_next_event.asm` lines 44 to 106 sort the queued times, take out those inside the step and return the latest one taken (page 3, TM-52). So the clock is stepped to queued window edges and Star Power ends between frames. Whether a press is judged at those clocks is not shown, because neither listing reads input.
Game code (6142): RVA `0x20F4950` and `0x20F6FF0`, the listings above
Hydra: no owner
Ruling: none yet: the game's rule is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build); the listings 2026-11-08, session 77057df2 (docs-gaps search)

### SP-31 Hydra's end lands within 3 ms of the game's

Rule: On the compared activations, the game's stored end and Hydra's deactivation chord agree within 3 ms, and the flag clears within one poll of the stored end.
Label: Proven in a live run
Evidence: Run 1, `run1_cal0_sp_compare.txt`: activation 1 game end 39,488.3 ms against Hydra 39,485.4 ms (+2.9 ms), activation 2 80,099.2 against 80,098.0 ms (+1.2 ms). Activations 3 and 4 were not on Hydra's path. The flag cleared 2.8, 4.8, 0.9 and 2.3 ms after the four stored ends (at 39,491.1, 80,104.0, 125,203.8 and 170,916.6 ms; the stored ends are 39,488.3, 80,099.2, 125,202.9 and 170,914.3 ms), with the poller about 3 to 4 ms apart; the multiplier halved in the same row. Summary: 31 PASS, 3 FAIL, the FAILs being which chord the game activated on. One chart, 4/4.
Game code (6142): `#8288+0x120`, `#8288+0x42` and `#8288+0x48` read live
Hydra: `Activation::deact_tick` in src/core/model.h; `kMeasuresPerSpBar` in src/core/timing.h line 254
Ruling: none needed: same end
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-32 Hydra puts the end on a whole tick

Rule: Hydra adds whole measures in ticks and rounds down, so its SP end sits on a whole tick. The game stores the end as a decimal time.
Label: Hydra assumes it, never checked
Evidence: `SongTiming::plusmeasure` in src/core/timing.cpp line 320 and ADR 0014 line 162 ("plusmeasure rounds down"). On Shop Theme, public Hydra's rounding shows -2.7 ms where Hydra Deluxe shows 0 ms. That reading is in card 2026-10-29-07074f3d; the copy of the public source it read was not kept. The game side is the decimal store in SP-27. One tick is about 1.7 ms at resolution 192 and 186 BPM.
Game code (6142): RVA `0x20F37E0` returns a decimal time (SP-27)
Hydra: `SongTiming::plusmeasure` in src/core/timing.cpp line 320
Ruling: none needed: c0e1259d's choice "Exact count in both modes (Recommended)" covers Hydra's whole-tick count
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-33 A chord exactly on the end counts inside Star Power

Rule: Hydra pays a chord whose time equals the SP end as inside Star Power. In the game the answer depends on the hit's clock and the frame (SP-28, SP-29): the end check turns Star Power off once the clock is at or past the end, and whether a hit at a clock equal to the end is scored before or after that check is not settled.
Label: Hydra assumes it, never checked
Evidence: Hydra: `paid_by_sp_walk` (offset at or below 0) and `after_sp_end` (tick strictly after the end) in src/core/backend_value.h lines 28 and 34. D13 (decision log line 31) states the rule as a decision, not in the user's words. No run has placed a hit exactly on an end. LS-O3 run 1 of 2026-11-07 pressed Senri's 8 end chords 12.23 to 14.94 ms before their time and all 8 paid inside, where LS-D2d's on-time presses paid chords 2044 and 2320 outside. So a press early enough is paid inside, and the exact case stays unread (result files in hydra-data, ch-mcp-runs/2026-11-07/ls-o3-run1). The late side is shown at one offset: in LS-R1 run 2 of 2026-11-07, chord 740's second press came at clock 118,442.16 ms, 15.27 ms after the stored end of 118.42689 s, and paid 200, not doubled (`C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run2\press-1.csv`, the second row of chord 740, score 301,670 to 301,870). In `sp_watch-1.csv` there the multiplier reads 4 from 118.4270 s on, and the meter steps from 0 to 1,440 at 118.4452 s, so the phrase was credited after the end.
Game code (6142): RVA `0x20F63B0` and `0x20F4550` (SP-28, SP-29)
Hydra: `paid_by_sp_walk` in src/core/backend_value.h line 28; `after_sp_end` line 34
Ruling: none needed: no difference is shown
Found: 2026-10-30, session b02ab3e6 (the reference build); the LS-R1 late press 2026-11-08, session 77057df2 (docs-gaps search)

### SP-47 The end is stamped from the frame that reads the press

Rule: When an activation is pressed, the game stores the Star Power end as if the press came one frame after the song clock the player read just before it sent the press. On four live activations the stored end sat 2.98 to 3.03 ms later than that clock read gives, which is one frame at the game's about 330 frames a second (TM-49). So the game judges a press in the frame after the one whose clock the player read.
Label: Proven in a live run
Evidence: Two runs of 2026-11-07. Bomber, chord 147: `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-r2ab-run1\replies.txt` says "the end moved 50.8 ms for a 53.78 ms early press", 2.98 ms short of the lead the clock read gives (the reply rounds the move to 0.1 ms). Get Ur Freak On, chords 232, 633 and 1062: `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-07\ls-f1-run1\press-2.csv`. The time the game judged each press is its `end_read_ms`, less the stored end the path's dump gives (57,932.89, 98,444.63 and 141,656.25 ms, the time of `nominal_deact_tick` in the `ls-l0` dump), plus its `chart_ms`. That time is 3.03 ms after `clock_ms_at_press` on all three (3.030, 3.029 and 3.032). The end is read straight after the activation's own press, before a later phrase can extend it, so the chord the dump anchors the final end on (`end_anchor_tick`, chord 263 for chord 232) does not enter the sum. The arithmetic takes the end to move by the press's error times r, and r is 1.0 at all four activations (`end_scale_early` and `end_scale_late` in the `ls-l0` dump and in tools/ch_probe/tests/bomber_refine_dump.json), so these runs check nothing about r and SP-39 keeps its label. Not proven: other frame rates, and which code takes the press in that frame (SP-30).
Game code (6142): the stored end `#8288+0x120` (`OFF_SP_END_TIME` in tools/ch_probe/constants.py) read live; the frame step itself was not found in the code (SP-30)
Hydra: no frame model in `src/` (SP-29); the press schedule's frame model (`kFrameMs` and `kJudgeEarlyMs` in tools/replay_schedule.cpp) plans each end from a press judged one frame after it was sent, and the player's `retime_from_end` in tools/ch_probe/experiments/play_chart.py moves each press timed from an end read (after the activation, or after the chord the schedule names) by that read minus the end the press was aimed at
Ruling: none needed: no Hydra rule depends on it
Found: 2026-11-08, session 6341c618, from the runs of session d0a3868c

## Phrases collected during Star Power

### SP-34 A phrase collected during Star Power moves the end later

Rule: A phrase collected while Star Power is on adds a quarter bar to the meter and sets the "meter changed" byte. The next update works the end out again from the ticks drained so far plus the meter, which pushes the end one quarter bar later (two measures in Hydra's units).
Label: Proven in the game's code
Evidence: The gain at `code/6142/sp_gain_20F4FA0.asm` lines 19 to 22 and 44 to 57; the recompute at `code/6142/sp_update_20F6800.asm` lines 77 to 86. Live: run 1's end moved while active on activations 2, 3 and 4 (75,539.7 to 80,099.2 ms, 120,710.6 to 125,202.9 ms, 166,291.0 to 170,914.3 ms), and activation 2's end is 1.2 ms from Hydra's. An open exception, LS-M2 run 1 of 2026-11-08 on Won't Get Fooled Again: the phrase-end chord 618 (157,925 ms) was hit during Star Power for +400 and the end did not move. In `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-08\ls-m2-run1\sp_watch-2.csv`, `sp_end_s` holds 163.4769 on every row from the activation to past chord 618, and the meter keeps draining by a few units a row through 157.92 s with no quarter-bar rise. `replies.txt` there gives 30 of 31 phrases earned and the game's end 163,483 ms against Hydra's 167,027 ms. Two causes are possible, and neither is settled. First, in `press-2.csv` there chord 609, the activation chord, has a second row with no `down_frame`, so one part of it was never pressed; if 609 lies in that phrase, the phrase was not earned. Second, from chord 610 the game's score rises are small (130, 230, 230, then 200 for 613 to 615) against 400 for 616 to 618, so the multiplier after chord 609 is not what a plain reading gives. Settling it needs per-note data.
Game code (6142): RVA `0x20F4FA0` lines 19 to 57; RVA `0x20F6800` lines 77 to 86
Hydra: `ScoreGraph::extend_deacts` in src/search/graph.cpp; `kMeasuresPerSpBar` in src/core/timing.h line 254
Ruling: none needed: same rule. The LS-M2 exception is recorded open, as the user chose (2026-11-08, "Record as open exception (Recommended)").
Found: 2026-10-30, session b02ab3e6 (the reference build); the LS-M2 exception 2026-11-08, session 77057df2 (docs-gaps search)

### SP-35 Which call credits a phrase on a hit

Rule: A phrase is credited when its last chord is hit, using that hit's own Star Power state. The call that passes the quarter bar to the gain method on a phrase hit has not been read.
Label: Unknown
Evidence: The only decompiled caller of the gain method is the fill deadline `0x20DDB70` (ghidra_decompile_all.txt line 681). The label "Star Power phrase: 0x20F6719 to 0x20F6738" in docs/audit/drum-lanes/FINDINGS-play-time.md line 87 is unproven: the callee there, `0x20F7610`, writes +0xA0 from a hit ratio and never writes the meter in the 67 lines read. A virtual call with the engine's quarter bar after the 0x100 flag is cleared, in docs/audit/drum-lanes/play/f20f63f0_base_hit.asm lines 206 to 216, is the likely route; its target was not read.
Game code (6142): gain RVA `0x20F4FA0`; the crediting call: not found (looked in the base hit listing and the decompile)
Hydra: `close_sp_phrase` in src/parse/song.cpp line 400 marks the crediting chord
Ruling: none yet: the game's rule is not settled
Found: 2026-10-30, session b02ab3e6 (the reference build)

## Squeezes

### SP-36 The game's own squeeze tests compare note time with the stored ends

Rule: In the base hit routine the game bumps one counter when a note scheduled at or before the activation is hit at or after it, and the same counter when a note scheduled after the end is hit before the end, both while Star Power is on. It bumps a second counter when Star Power is off and the note lies within the field +0x10 of the last end. The first test is not strict (it includes a note exactly on the activation and a clock exactly at it). The second is strict on both sides. The third is strict on its upper edge and includes its lower edge, and it is the only one that adds the field +0x10 to the note time.
Label: Proven in the game's code
Evidence: docs/audit/drum-lanes/play/f20f63f0_base_hit.asm lines 78 to 90 (activation at +0x110 not below the note time and clock not below the activation, `jb` skips; counter +0xE8), 91 to 103 (note time above +0x120 and end above the clock, `jbe` skips; counter +0xE8) and 104 to 119 (note time plus +0x10 above +0x128 by `jbe`, and +0x128 not below note time minus +0x10 by `jb`, with Star Power off; counter +0xEC); ghidra_decompile_all.txt lines 53403 to 53424 read the same. The field +0x10 is the early-side half-window of the note last stored by the per-frame update, and 0 when the byte at +0x195 is set (docs/audit/drum-lanes/play/f20f5810_base_update.asm lines 74 to 84; page 3, TM-24). Which scorestats fields read the two counters is not traced (page 6, SC-37).
Game code (6142): RVA `0x20F63F0` (the base hit routine), docs/audit/drum-lanes/play/f20f63f0_base_hit.asm lines 78 to 119; the +0x10 store, RVA `0x20F5810`, docs/audit/drum-lanes/play/f20f5810_base_update.asm lines 74 to 84
Hydra: `within_squeeze_window` in src/core/model.h line 123; `sqout_chords` in src/core/sqout_chord.h
Ruling: none needed: these are counters; the SP end itself is SP-28
Found: 2026-10-30, session b02ab3e6 (the reference build); the +0x10 field 2026-11-08, session 77057df2 (docs-gaps search)

### SP-37 Squeeze-in and squeeze-out, and their timing budget

Rule: A squeeze-in hits a phrase's last note early so it is collected inside Star Power. A squeeze-out hits it late so Star Power has ended and the phrase stays for a later activation. Hydra's timing budget is the hit window times (1 + r), with r from SP-39.
Label: Hydra assumes it, never checked
Evidence: `squeeze_budget_ms` in src/core/squeeze_rating.cpp line 126 and `kHitWindowMs` (85 ms) in src/core/model.h line 105. The meaning of squeeze-in and squeeze-out was checked only against Hydra's code and full-combo videos, never against the game's code; one such video case is session 93d37c20 (2026-08-29, Enigma Machine, card only). Hydra shows a row's "effective" timing as the offset times 2 / (1 + r), which puts the real budget of 85 x (1 + r) ms onto the plain two-window scale (`effective_backend_ms` in src/core/squeeze_rating.cpp line 117). Card 700792ed (docs/sessions/2026-08-29-700792ed.md) gives a case: a free squeeze-out 331.3 ms past the end with late r = 4.45, about 122 ms effective. That the chart was Thick as a Brick comes from that session's own record, which is not kept. The game side is the frame rule of SP-29.
Game code (6142): RVA `0x20F63F0` (SP-36), `0x20F4550` (SP-29)
Hydra: `squeeze_budget_ms` in src/core/squeeze_rating.cpp line 126; `kHitWindowMs` in src/core/model.h line 105
Ruling: card c73697df: "the hit window is +/- 85ms, the 70ms is an outdated figure. we can change that." (02:08 UTC; the window's value only)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-38 Hydra looks for squeezes only within 500 ms of the end

Rule: Hydra's search and its stored backends reach strictly less than 500 ms either side of the SP end. Public Hydra stopped at 140 ms. The game has no such horizon.
Label: Hydra assumes it, never checked
Evidence: `kSqueezeWindowMs = 500.0` in src/core/model.h line 111 and `within_squeeze_window` (strict) at line 123; public `3b49012c:hydra/hypath.py` lines 87 and 177 (`< 140`). No 500 ms or 0.5 s constant was searched for in the engine. docs/differences-from-public-hydra.md lines 29 and 83 describe the widening.
Game code (6142): none (a search horizon, not a game rule)
Hydra: `kSqueezeWindowMs` in src/core/model.h line 111; `within_squeeze_window` line 123
Ruling: card 6e7764db: "timing and backend limit should not be limited to 200ms. they should reach the full 500 ms hydra supports" (2026-08-29 02:45 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-39 An early or late activation moves the end by r times the error

Rule: Because Star Power length is counted in measures, a timing error at activation moves the SP end by r times the error, where r is the end measure's length over the activation measure's. Early and late can differ at a tempo or time signature change. The scale's direction follows the sign of the difficulty, not the squeeze kind.
Label: Hydra assumes it, never checked
Evidence: `frontend_transfer_scales` in src/core/squeeze_rating.cpp line 24 and `transfer_scale_between` in src/core/squeeze_rating.h line 53; tests/test_squeeze_rating.cpp holds the Tom Sawyer shape and the Dumpweed fixture. The scale was checked against five figures by DragonDelgar (public Hydra's developer). The user pasted his Discord messages into session f1784fd5 (2026-08-16). His original messages were not kept, and no repo file holds them. The session's card (docs/sessions/2026-08-16-f1784fd5.md) names the five charts and says the computed scale matched his numbers on all five; the figures below were copied onto this page from the session's own record, which is not kept. Hydra reproduced each one: Tom Sawyer at measure 105.2.360, r = 0.5132, so +50.5 ms at the front is +25.9 ms at the end; Beast and the Harlot at measure 196.0, late r = 0.65376, so +22.0 ms becomes +14.38 ms; Bloom at measure 107.0, late r = 0.4444, giving 81.06 ms against his "81.05"; As I Am at measure 81.0, early r = 0.57888, giving 97.51 ms against his "~97.5" and a 72.53 ms budget against his 72.6; Dumpweed at measure 19.0, a 73.95 ms budget against his "±74". These are Hydra's numbers matching another tool's numbers, so they check Hydra's arithmetic, not the game. Hydra's Dumpweed scale is 0.935114 early and 0.965344 late (tests/test_squeeze_rating.cpp), because a phrase collected during Star Power moves the real end (SP-34). The on-screen timings of the video Dumpweed Pro Drums FC (YouTube id eny2KPSHFNg), front -74.1 ms and squeeze-out +75.1 ms on a squeeze that succeeded, fit that scale (card docs/sessions/2026-08-20-9f5a3914.md). The game premise is SP-15, which is open.
Game code (6142): none (rests on SP-15)
Hydra: `frontend_transfer_scales` in src/core/squeeze_rating.cpp line 24; `transfer_scale_between` in src/core/squeeze_rating.h line 53; `rate_activation` in src/core/squeeze_rating.cpp
Ruling: none needed: Hydra's model of a game rule still open in SP-15
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-40 When the cap clamps, the scale is measured from the collecting note

Rule: When the full-bar cap clamps a window, Hydra measures the transfer scale from the collecting note. The game's clamp anchors the end one full bar after the collecting moment (SP-24), which is the same anchor.
Label: Hydra assumes it, never checked
Evidence: The game's anchor is `code/6142/sp_update_20F6800.asm` lines 69 to 86 (SP-24). Hydra's scale model is its own. A Rolling in the Deep video case (card 23b23b12: collecting note 63.1 ms early, squeezed note 67.3 ms late) is unchecked: the video is not named and the diagnosis note is uncommitted.
Game code (6142): RVA `0x20F6800` lines 69 to 86
Hydra: `SpExtension` in src/search/graph.h line 103 (the stored clamp tick)
Ruling: D1: card cebf3006 holds the user's pick "Yes, collecting note (Recommended)" (decision log line 7)
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-41 No backends after a squeezed-out note

Rule: After a squeezed-out note, Star Power has ended, so the engine keeps no backend rows past it and the display agrees.
Label: Hydra assumes it, never checked
Evidence: cards 99e42ceb and 135630ac name the engine rebuild in src/search/engine.cpp; the code was not re-opened for this row.
Game code (6142): none (a Hydra record rule)
Hydra: src/search/engine.cpp (per the cards; not re-opened)
Ruling: card 99e42ceb: "hydra should not display any backends beyond a squeeze out, that doesnt make any sense. do not just change the display, find where the engine calculates these records and fix it there" (03:55 UTC)
Found: 2026-10-30, session b02ab3e6 (the reference build)

## The 3 ms leeway

### SP-42 The 3 ms backend leeway is DragonDelgar's scoring rule

Rule: A backend note less than 3 ms after the SP end still counts in the optimal score; exactly 3.0 ms does not. This rule decides only whether a backend row is counted. It does not move where Star Power ends, which is one point (SP-28).
Label: Hydra assumes it, never checked
Evidence: Public Hydra at 3b49012c, `git show 3b49012c:hydra/hypath.py` line 678: `is_leeway = be.offset_ms > 0 and be.offset_ms < 3`. Hydra: `counted_without_squeeze` in src/core/backend_value.h line 39 (true when `paid_by_sp_walk` is, or when `offset_ms < leeway_ms`, strict). The game has no such rule (SP-43).
Game code (6142): none (a scoring rule, not a game rule)
Hydra: `Rules::backend_leeway_ms` in src/core/rules.h line 32; `counted_without_squeeze` in src/core/backend_value.h line 39
Ruling: D269 call 4, the user's words (session 61cee421): "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted. it's an arbitrary number chosen by dragondelgar and i am fine with keeping it so our scores use the same methodology as delgar's hydra". The strict edge at exactly 3.0 ms is D29's record.
Found: 2026-10-30, session 61cee421

### SP-43 The game's engine holds no 3 ms constant

Rule: No 3 ms (0.003 s) constant is used by the drum engine or its base classes.
Label: Unknown
Evidence: A byte search of `GameAssembly.dll` found the double and the float 0.003 once each in .rdata; a scan of every instruction for references to them found six, in static methods of class #182 (`0x3738B0`, `0x373A40`, `0x373F30`, `0x374310`) and two compares in class #707 at `0x16C650`, none in #8288, #8253, #8255 or #8544. The two compares (`code/6142/clock_tick_16C650.asm` lines 105 and 116) test the gap between the audio clock and the raw clock, and line 107 stores the correction when it is over 0.003 s (page 3, TM-12 and TM-47), so they are the clock's drift threshold, not a scoring leeway. A scan of 18,340 operands in the engine address ranges for values near 0.003, or exactly 3.0 or 3000.0, found only scale factors. Constants held as immediate operands were not searched, so the absence is not shown. The 2026-10-03 claim over 126 engine methods rests on a lost listing.
Game code (6142): not found: looked as described in the Evidence line
Hydra: `Rules::backend_leeway_ms` in src/core/rules.h line 32 (Hydra's own rule, SP-42)
Ruling: none needed: SP-42's ruling covers the leeway
Found: 2026-10-30, session b02ab3e6 (the reference build); the two compares 2026-11-08, session 77057df2 (docs-gaps search)

## The bot

### SP-44 The bot never gains Star Power

Rule: The game's bot never gains Star Power meter, so it never collects a phrase and never activates. The engine's gain method returns before adding anything when the player kind is Bot (or RemotePlayer).
Label: Proven in the game's code
Evidence: Lines 13 and 14 compare the player kind at +0x190 with 2 (Bot), lines 15 and 16 with 1 (RemotePlayer), each jumping to the return. The kind is set to 2 for a profile whose "is_bot" setting is on by the drums player's kind method (page 7, IN-64), and the command-line loader switches that setting on (page 7, IN-61). Live: both Trippin SP Spike bot runs (Drums and ProDrums, 2026-10-28) have is_bot true, sp_phrases_earned 0 of 11, sp_activations 0 and time_in_sp 0 (C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\l1\bot\); docs/research/2026-10-28-clone-hero-mcp-feasibility.md (Test 1) says bot runs tell nothing about Star Power paths. Run 1, which does use Star Power, was the key-press auto-player (is_bot false in `run1_cal0_scorestats.json`), so it says nothing about the bot (SP-45).
Game code (6142): RVA `0x20F4FA0`, `code/6142/sp_gain_20F4FA0.asm` lines 13 to 16; `#8288+0x190`
Hydra: no owner: Hydra has no bot model
Ruling: none needed: a fact about the game's bot
Found: 2026-10-30, session b02ab3e6 (the reference build)

### SP-45 The key-press auto-player does activate Star Power

Rule: A tool that presses keys for the player (the project's auto-player) collects phrases and activates Star Power, so Star Power checks need it, not the bot.
Label: Proven in a live run
Evidence: Run 1, `run1_cal0_scorestats.json`: is_bot false, sp_phrases_earned 11 of 11, sp_activations 4; `run1_cal0_sp_compare.txt` shows four activations, with "FAIL path activation 5 ...: the game made no activation".
Game code (6142): `#8288+0x42` read live
Hydra: no owner: the player is a tool in tools/ch_probe
Ruling: none needed: a tool fact
Found: 2026-10-30, session b02ab3e6 (the reference build)

## On screen

### SP-46 The multiplier disc is the only Star Power sign on the 1.1 screen

Rule: In a Clone Hero 1.1 gameplay video, the one sign of Star Power is the multiplier disc, which shows x8 instead of x4 at a full combo. The purple bar under the score is the song's progress, not a Star Power meter, and no separate meter is drawn.
Label: Unknown
Evidence: session e61236dc (2026-08-17, the Swim to the Moon full-combo video). Its card (docs/sessions/2026-08-17-e61236dc.md, the first roadblock) records that the session took the purple progress bar for the Star Power meter and then found the 16 Star Power runs from the multiplier disc showing x8 at 60 fps. The session's own record, which is not kept, adds that it searched the frames for a real meter first (it correlated every pixel with the Star Power signal and diffed frames before and after an activation). Neither record states that no meter exists, and no game code was read for it. C:\Users\Patrick\Downloads\Hydra\video-tools\README.md lines 235 to 246 (outside the repo) find the disc by its changes at the combo steps, which fits but does not show the claim.
Game code (6142): none read
Hydra: no owner: Hydra does not read the game's screen; video-tools is a separate tool
Ruling: none needed: a fact about the game's screen
Found: 2026-08-17, session e61236dc

## Rows waiting on a live run

SP-06, SP-14, SP-15, SP-18, SP-24, SP-29, SP-30, SP-31, SP-33, SP-35, SP-36, SP-37, SP-46. Each is on the live-checks page.

## Open splits

SP-29 (Q-50).
