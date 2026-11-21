Agent a23bb20a801877a01 (task T5-V-RESEND-D13). Re-send verify file for slice D13. Records are in `docs/audit/2026-11-14-test-audit/records/D13.md`; the first verifier's file is `verify/D13.md`, whose "Not reached" section this file finishes.

# How to read this file

Every mark is "by reading". I ran nothing and found no existing run that proves a claim here. Paths below are in the worktree. The tools live under `tools/ch_probe/` and `tools/ch_probe/experiments/`. Record numbers are the records file's numbers.

The records I was sent: 122, 123, 124 (the three stall tests the first verifier did not open) and 46, 47, 48, 49, 50, 66, 68, 69, 70 (the typed figures and offsets it did not check). 12 records in all.

# Totals

- Records checked: 12.
- Agreed: 12.
- Disputed: 0.
- Not reached: none.
- Two additions to the records are listed under 122 and 66 below. Neither changes a verdict.

# The three stall tests, records 122 to 124

What I read: `tools/ch_probe/tests/test_stall_harness.py` lines 255 to 372 in full. In `tools/ch_probe/experiments/stall_harness.py`: `_forced_block` (489), `_play_pass` and `_game_pass` (602 to 773), `play_once` (786 to 811). In `tools/ch_probe/experiments/play_chart.py`: `play_in_game`'s event loop (1100 to 1200), `load_schedule`'s read handling (318 to 330), `Press.at_ms` (182). In `tools/ch_probe/experiments/stall_game.mjs`: the fire counter and clock lines (31 to 102).

What the three share. The input is a cut of the real LS-P3 schedule, a saved `hydra_replay schedule` output. It sits outside the repo at `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-prep\The Who - Won't Get Fooled Again (O)-schedule.json` (test line 256). I confirmed that file exists on this machine, so these tests run here. On any machine without it, or without node, the class is skipped and the skip reads as a pass (lines 309 to 312). The mechanism is real: a real node subprocess for the fake game, `time.perf_counter`, real threads, a real press-log writer. The game itself is fake (`stall_game.mjs` counts fires in its own timers). That checks how the Python player behaves under a block. It does not check what Clone Hero does, and these tests do not claim that.

I checked the schedule's own data against what the tests say about it, because the tests rely on it:
- Chords 285 to 300 are 16 chords from 92612 ms to 95528 ms. Their gaps run from 57 ms to 230 ms, so the "about 220 ms apart" comment fits the first stretch and the last ones are tighter. A 900 ms block therefore outlasts several of them.
- Chord 292 carries one end read (`planned_end_ms` 104833.30). No other chord in 285 to 300 has a read. So the cut has exactly one end read. `play_in_game` posts a retime on every end read, even one that moves nothing (`play_chart.py:1160-1163`), which is why line 344 can hold. The comment at 263 to 265 ("The cut holds chord 292's end read") is true.
- Chord 1218 (286015.7 ms) holds the end read of activation 4. The only press it moves is chord 1261's, at 293105.75 ms. The gap is 7,090 ms, matching "7.09 s" in the comment at 266 to 268. Chord 1261 itself holds no read.
- Chords within 1500 ms either side of 1261 (18 of them, 291680 ms to the moved press) hold no read except through 1218's. So the late-retime cut has one end read and one retime post, as lines 367 and 368 say.

122. test_a_900_ms_block_drops_nothing_when_the_game_times_the_presses. Agree, Investigate. Line by line: 334 stop is done; 335 and 337 nothing dropped, from the result and from the rows (two views of one fact); 338 `retime_missed` 0 (counted from rows at `stall_harness.py:809`); 339 `sent == len(chords)`; 340 and `assert_every_seq_once` (324 to 328) check that each press seq was reported once, fired once, and that the fires are a subset of the planned seqs; 343 the block was long enough to be felt; 344 the retime kind was posted. Each of these would go red on the fault the test names: a block that drops a press, a press fired twice, a retime that never goes out. I agree with the record's reason for Investigate and want to state it sharper: the fault is that the input is an outside-the-repo file, so the test silently does not run on a fresh checkout. Copying the 16 chords of the cut (as real input, not an expected-output file) into the repo would fix that. One more point the record did not make: line 343 types `_PS1_BLOCK_MS / 2` as its floor, with no reason for half. It is a guard that the block really landed in the run; say so beside it or name the quantity it compares with.

123. test_the_same_900_ms_block_drops_chords_from_the_outside_loop. Agree, Investigate, same reasons. `dropped >= 1` (349) is the claim. By reading it holds only if the 900 ms block on the player's own thread outlasts the next chords' last sends, which the schedule gaps (57 to 230 ms) allow. It is a real-time test: on a quiet machine it fails only if the old loop stopped dropping, which is its purpose. Its worth is as the control that 122 can go red.

124. test_a_retime_900_ms_late_fires_no_press_twice_and_says_retime_missed. Agree, Investigate, same reasons. Checked: `_late_retime_cut` moves chord 1218 to go down 400 ms before 1261's press. With `end_read_block_ms` of 900 the retime is sent after the press at 1261 has fired, so exactly chord 1261 is "retime missed" (360), the count is the one press planned from that read (361 to 362; I confirmed one press has `end_read_after` 1218), the message names it (363), one end read, one retime (367 to 368). All numbers in the test follow from the file and the two named constants; I found no typed figure that is a raw result. `_PASS_CAP_S` 60 (278) is a cap, not a claim.

Split noted: none new. The first verifier's S5 (the stall seconds typed in more than one place) does not touch these tests.

# The typed ms in records 46 to 50

I opened `mouths_refine_schedule.json`, `bomber_refine_schedule.json`, `senri_schedule.json` and the two `_score.json` files, and computed each figure from the file's own fields. `press_reach` takes each press's `chart_ms + aim_ms` (`Press.at_ms`, `play_chart.py:182`) and keeps the smallest and largest per chord (`sp_compare.py:100-102`, in `tools/ch_probe/experiments/`).

46. Agree, Rewrite. Mouths chord 740: ms 118425.9508; presses at aim -22.6060 (gems 1 and 2, rule `squeeze_out_inside`) and +7.3940 (gem 0, `squeeze_out_late`). 118425.9508 - 22.6060 = 118403.34 and + 7.3940 = 118433.34. The test's (118403.34, 118433.34) matches. The first verifier's caution about the proposed rewrite stands: "the reach contains each press's own time" can fail only if the min and max in `press_reach` are turned around, so say what it then protects.

47. Agree, Investigate (claim not checked only; the rows are hand-written, so the game is not the source). Chord 740 in the score file is tick 48480 and ms 118425.95, as the PASS line says ("tick 48480, 118426.0 ms"). Reach is 118403.34 to 118433.34, and the rise row is at 118441.0, 7.7 ms after the last press (I did not read the report's credit window itself). The row at 118.412 has amount 0, so the first rise is at 118441.0, as asserted.

48. Agree, Rewrite. Bomber chord 147 at ms 27333.3333 has two presses: aim -62.4294 (anchor, gem 1) and -54.4294 (gem 0, `after_ms` 8.0). 27333.3333 - 62.4294 = 27270.90 and - 54.4294 = 27278.90. Matches (27270.90, 27278.90). Chord 146 is at 27250.0, so the flip at 27285.0 ms is 35 ms after chord 146's own time and after both of 147's presses; the record says "83 ms before it". The 83 is chord 146's gap to the flip against chord 147's ms (27333.3 - 27250.0 = 83.3), which the test comment at 750 states as "chord 146 83 ms before it". That wording is the comment's own and slightly off: 146 is 83 ms before 147, and the flip lands 48 ms before 147's ms. It does not change the verdict.

49. Agree, Rewrite. Bomber chord 180 at ms 32583.3333, one press, aim +38.9040. Sum 32622.2373, rounded 32622.24, equal on both ends. Matches. The score file's chord 180 is tick 18768 and ms 32583.33, as the PASS line says. The row at 32.640 is 17.8 ms after the press, inside the window.

50. Agree, Rewrite. Senri chord 268 at 31860.680, one press, aim -15.0 (rule `sp_end`), gives 31845.68. Chord 267 at 31759.329, one press, aim -2.0 (rule `plain`), gives 31757.329, rounded 31757.33. Both match. The record's property ("one press means earliest equals latest") is the right one to keep.

# Offsets in records 66, 68, 69, 70

I read `tools/ch_probe/engine.py` 40 to 147 and 276 to 293, `tools/ch_probe/constants.py` 100 to 150 and 322 to 345, and the pages the constants name (`docs/clone-hero/05-star-power.md` SP-09, SP-11, SP-13, SP-16, SP-19; `03-timing.md` TM-13 and TM-47).

66. Agree, Rewrite. Every offset in `sp_block` (test lines 63 to 70) matches `constants.py`: 0x68 amount (`OFF_SP_AMOUNT`), 0x1B0 maximum, 0x42 active, 0x48 multiplier, 0x120 end time, 0x130 overflow byte, 0x100 song clock. The page lines I found back each: SP-11 gives +0x68 and +0x1B0 (page line 98), SP-19 gives +0x42, +0x48 and +0x120 (lines 199 and 324), SP-13 gives +0x130 (line 135), TM-13 gives +0x100. So the record's "I did not check 0x42, 0x48, 0x120, 0x130, 0x100 against a page" is now closed: each agrees with its page. Expected values follow the packing: 7680 + 1 x 2^32 = 4294974976, 6144 + 2 x 2^32 = 8589940736, 0x0101 = 257 with low byte 1, 0x0208 = 520 with low byte 8. The first verifier's check of the two big numbers stands. The pages are the game's code read offline, not a live memory read, so the claim stays "not checked on a real engine block" (C-MEM); the verdict is unchanged.

68. Agree, Investigate (claim not checked only; C-MEM). The half at +0x1B8 agrees with `OFF_SP_HALF` and with SP-16's evidence line "stores the half at +0x1B8" (page line 107). 7200 is a free input, as the record says, and `sp_max_u64` 8589940736 is the block's own raw result.

69. Agree, Investigate (claim not checked only; C-MEM). The fake memory's offsets match `constants.py`: engine listener 0x1A0, player-to-manager 0x20 (`PLAYER_OFF_GM`), manager-to-clock 0xB0 (`GM_OFF_CLOCK`), clock fields 0x18 (float correction), 0x20 (double audio), 0x38 (double raw). TM-47 states the three clock fields (0x18, 0x20, 0x38) and names the other three fields (`BasePlayer.gameManager`, `GameManager.sync`, `#8288+0x1A0`) without their offsets on the line I read (page line 136); so 0x20 and 0xB0 agree with `constants.py` only, and I did not check them against `dump.cs` (not opened). The expected tuple (12.75, 12.5, 0.25) follows the packed values and the order in `decode_clock_block` (`engine.py:101-110`).

70. Agree, Rewrite. The expected read list matches the code: the engine block read is `SNAPSHOT_SIZE` = `OFF_ROUND_START` 0x2E0 + 8 = 0x2E8 (`engine.py:55`); the player read is 8 bytes at player+0x20; the manager read is 8 bytes at manager+0xB0; the clock read is `CLOCK_BLOCK_SIZE` = 0x38 + 8 = 0x40 (`engine.py:59`, `clock_block` 284 to 291). So the typed list is the code's own constants again, a copy, as the record says. The record's rewrite ("four reads, the last three no more than 0x40 bytes") is right. A fifth read would turn the current test red at line 150.

# Splits

None new from these records. The first verifier's S1 to S6 stand and I found nothing in these twelve records that changes them.

# Question 2 and question 3 for these records

No caller the records missed: these twelve tests call tool code only (`sp_compare.press_reach`, `engine.decode_snapshot`, `engine.EngineModel.snapshot`, `stall_harness.play_once`), none in `src/`. `records-engine/` and `records-screen/` hold nothing that finishes a "claim not checked" here: the Mouths, Bomber and Senri schedules in `records-engine/` are not what records 47 and 68 to 69 need (a watcher CSV and a real engine block), as the first verifier found.

No rewrite here proposes a saved expected-output file. The one thing I suggest saving (record 122, the 16-chord cut) is test input, not an expected output.
