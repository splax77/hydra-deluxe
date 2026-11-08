# player-audit-S3-V4 (agent a2cd2259fd40a98f5)

Read only. I ran only small Python scripts over the LS-M2 run files (v4_a.py to v4_d.py in this folder) and a hand-rolled MIDI read of the chart's notes.mid. Nothing built or played.

## H1. Handoff claim: "chord 618 broke, so activation 2 ended at 163,483 ms, not 167,027 ms"
Verdict: PARTLY. The end times and the lost extension are right. Chord 618 was NOT a miss. The phrase was lost because of a miss at chord 609, the activation chord, which is inside the same phrase.
Proven by `ls-m2-run1\press-2.csv`: the CSV `index` column is the chord index. Row 618 (chart_ms 157924.97) has hit=1, score 278455 -> 278855 (+400). Rows 560 to 639 all have hit=1. Row 609 scored +520, which is the cymbal's 65 x 8 only. The kick's 50 x 8 = 400 is absent (Hydra expects 920).
Proven by the score JSON (`kick-prep\who\...-score.json`): activation 2 is act_ms 156354.292, deact_ms 167027.114. Chord 618 is flagged `is_sp_phrase_end` at 157925.0.
Proven by the chart (`notes.mid`, read by my script): the phrase 168960 to 170759 ticks starts exactly at the activation tick 168960. By tick, chords 609 to 618 are in that phrase. Chord 609 is the activating chord ('n...N', kick plus cymbal), and 618 is the phrase's last chord. Note 7 of the dump's activation record gives sp_meter 2 and nominal_deact_tick 176640 against deact_tick 180480, so Hydra plans this one phrase as the extension (3,840 ticks).
Proven by `sp_watch-2.csv`: at clock 156.356 s the meter is 7200 with sp_end_s 163.4769 and never rises. Between 150 s and 170 s the only changes are the activation at 156.356 and the drop to 0 at 163.483. At 157.921 to 157.933 (chord 618) the meter goes 5631 -> 5619, the normal drain, with no jump. So 618 hit and the meter did not rise, because the phrase's first chord (609) lost its kick.
Also in the watch: sp_mult_u32 goes 8 at 156.356, then 2 at 156.441 and stays low. Score gains for 610 to 618 are half or less of Hydra's figures. That is the streak breaking, a separate effect from the extension.
Open (not by reading): the rule "a missed note in the phrase cancels its award" is the game's behaviour I am inferring from this run. `docs/clone-hero/05-star-power.md` SP-04 says only that the last chord carries flag 0x100; I found no page statement that a missed phrase note cancels the award (searched the 05, 03, 04 pages for miss/phrase). A reading of the game's miss handler or a run with only the kick withheld would settle it. Whatever the rule, the handoff's wording "chord 618 broke" is wrong.

## C14 (which checkout's code a run loads)
Verdict: PARTLY. Python and the built JS always come from one checkout, so they cannot mix. Nothing stamps or compares which build it is.
Homes: `.mcp.json` (main checkout) starts `py tools/ch_mcp/front.py -- py tools/ch_mcp/server.py` as relative paths; `front.py:180` Popen has no cwd, so the path resolves against the cwd the MCP server was started in. `server.py:31-33` and `frida_attach.py:37-40` set `_REPO_ROOT` from the file's own location. `frida_tools.py:49` `AGENT_DIR` is `agent/` beside that file. `kit_tools.py:721`, `frida_tools.py:376`, `bot_run.py:631` only test `os.path.isfile` on the .js.
So a worker started from a worktree would find no .js (they are git-ignored), and says "not found; run npm run build:..." (`trace_hits.py:265`). I found no commit, hash or mtime compare in `tools/ch_mcp/*.py` (searched rev-parse, getmtime, st_mtime, stale: only unrelated hits at game_tools.py:1285, bot_run.py:430, quiet.py:278).
Evidence kind: by reading.

## NEW, serious: built kit.js predates the T3 split-chord commit (stale build)
In the main checkout (`C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\ch_mcp\agent`): `kit.js` mtime 2026-11-06 21:31:30. Commit 2709008f (2026-11-07 23:11, "Player limits T3: split chords go in one kit message") changed `kit.ts` (adds `groupsOf` and the `then`/`after_ms` groups). `grep -c "groupsOf\|after_ms" kit.js` = 0. The Python (`kit_tools.py:337`, `:620`) sends `then`. The built agent has no code that reads `then`, so a second group is silently ignored. LS-M2 ran 2026-11-08 12:20 (`launch-20261108-122036.jsonl`), after that commit and with kit.js unchanged since 11-06. This fits the nine activation kicks and the three kick-first chords that the handoff says missed (its "stale build very likely" line). By reading plus file times; not proven by a press row (the LS-M2 CSV has one down_frame per row, no per-group frames). Other agents: launch.js (Oct 31 13:17) was read whole by W2A as matching; I did not re-check. Python checks of built agent vs source: none found (above).
Split: Python `kit_tools.py` sends `then`; the installed `kit.js` ignores it. Input where they differ: any split chord.

## F8-9 (kit action ids 37/38/39/4/16/6, kick bound to "Orange")
Verdict: NOT CONFIRMED as a missing block. `docs/clone-hero/07-internals.md` IN-82 (line 837 to 839) states definition 1 "MIDI Drums": elements Snare 0, Tom1 1, Tom2 2, Tom3 3, Cymbal1 4, Cymbal2 5, Cymbal3 6, Kick 7, StartPad 18, SelectPad 19, "binds element 0 to action 1, 1 to 2, 2 to 3, 3 to 0, 4 to 37, 5 to 38, 6 to 39, 7 to 4, 18 to 16 and 19 to 6" and Kick to Orange. This matches `kit_tools.py:161-172` `KIT_BINDINGS` id for id. Label there: "Proven in the game's code", read from `level1` bytes, and "the read script was not kept". The names YellowCymbal/BlueCymbal/GreenCymbal are not individually listed (it says "the three cymbal actions"); the code says names are for the reply only. Action 4 in the drums input is also in `docs/clone-hero/code/6142/drums_input_source.asm` lines 183 to 187 per the kick-axis plan. By reading.

## W2A
- 7 CONFIRMED by reading. `launch_patch.json:3` "1.1.0.6142", `menu_fields.json:3` "6142", `start_candidates.json:3` "1.1.0.6142-final", `menu.ts:54` RVA 0x32F590; user path at `launch_patch.json:4` and `kit_methods.json:3`. `ch_attach` calls `build_facts` (version string vs facts file, `build_facts.py:116-125`); `menu_fields.json` build is read only in tests (`test_frida_tools.py:528,564,595`). `_engine_read_lines` and SP watch call `verify_targets` (`frida_tools.py:669`, `sp_tools.py:396`), which reads two rdata doubles only (`process.py:196-205`).
- 8 CONFIRMED by reading. `constants.py:211` `s_to_ms` and `sp_compare.py:212` `_ms` both x1000. `sp_compare.py:~114-122` has SP_MULTIPLIER 2, MEASURES_PER_SP_BAR 2, SP_CAP_BARS 4, SP_ACTIVATION_BARS 2; C++ owners `timing.h:48,51,254`, `model.h:37` have the same values. `at_ready_level` (`sp_compare.py:288-292`) is `amount >= 0.5 * sp_max` in floats; SP-16 (`05-star-power.md:167-169`) says the game tests that +0x1B8 is not above the meter and no listing of the test was made. sp_max in LS-M2 is 14400 (even), so no difference seen there.
- Baked `menu_fields` part of 1: baked copy exists (`state.js:3492 menu_fields_default`, used at :3716). I did not diff it field by field; W2A reported the same 15 fields. Not independently verified.
- Side notes, focus section, test-fake section: not verified (listed below).

## W2B
- A5 CONFIRMED. `replay_schedule.cpp:20-21` kWindowFrontMs 37.5 and kWindowBackMs 85; formula constants `:26-29` (0.0110924369747899, 2.628539e-05, 170, 20). `ch_probe/constants.py:222-224` EXPECT_NORMAL_BACK_S 0.085, FRONT_S 0.0375; `model.h:105` kHitWindowMs 85. `tests/test_replay_schedule.cpp:275,291` pin 37.5 for the C++ only. grep of src, tests, tools for kWindowBackMs/kWindowFrontMs finds only replay_schedule.cpp and its test, so no test ties the Python copy to the C++ one. Split: none differs today (all 37.5 and 85).
- B1 CONFIRMED by reading. `machine_recorder.py:366-373`: only OSError and ValueError give None; no range, monotonic or still-the-engine check.
- B2 PARTLY. The recorder does not call `verify_targets` (grep: no call in machine_recorder.py). But the engine address it reads comes from the SP watch: `game_tools.py:1080` sends `engine <addr>` after the watch found it, and `sp_tools.py:396` runs `verify_targets` before that search. So the clock offset is used after a two-doubles check, in another module. The check is only those two rdata doubles (`process.py:196-205`), and `build_facts.load_facts` (`build_facts.py:105-126`) compares version strings only. Same-version changed exe passes both.
- B5 not independently verified (I did not compare the offsets to the Windows layout).
- B6 NOT CONFIRMED as a problem. `recorder_link.start` re-raises (`:108-110`) but its only caller `game_tools.py:1221-1226` catches Exception and adds one reply line, so the run goes on.
- B7 CONFIRMED. `machine_recorder.py:~429-431` late sample skips ticks; `machine_report.py:280-283` prints median, worst and "over GAP_MS" gaps.
- C2 PARTLY. `menu_tools.py:113-119` and `menu_steps.json` (`verified: false`, `unproven: true` at line 13) say so. `docs/clone-hero/07-internals.md` IN-79 (:803-808) is the page block: "Proven in a live run", and says the profile screen and main menu both read active from the start, so the active read cannot tell them apart. Its evidence says no result file of that run was kept.
- E3 PARTLY: the C++ constants are as stated (replay_schedule.cpp:26-29). I did not open the Python `predicted_window_normal`.

## Not verified (no time or not reached)
W2A: side notes in item 10, focus section, test-fake section, baked menu_fields field-by-field diff.
W2B: B3-not mine; C3, D4, E1, E4, F3, F4, F5.
F2: S2, S3, S4, replies.txt figures.
S1-calibrate: C9, C10, C15, C16, C17.
F3/F4: F4-8 and F4-9 (START_LEAD).
