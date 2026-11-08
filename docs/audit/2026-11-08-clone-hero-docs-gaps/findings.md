# Clone Hero facts the reference pages lack: findings

Date: 2026-11-08. Session 77057df2. Plan: `docs/superpowers/plans/2026-11-08-clone-hero-docs-gaps-search.md`.

## What this is

This is the result of a read-only search for game facts that are proven somewhere but are missing from the Clone Hero pages, labelled weaker than their evidence, or contradicted. Nothing on the pages has changed yet. The write-backs wait for the user's yes.

The user's rule for the search: "plans are another agent's summary and should not be treated as authoritative facts. the code is the source of truth. approve but every fact must be checked against the real code." So every row below names a real source: a line of a committed game listing, a row of a run's own data file, or a line of Hydra's code. Prose was followed only as a lead.

How it ran. One scout checked TM-50 first. One calibration scout found the SP-29/SP-30 gap with no hint. Eight finder scouts read the sources and the Unknown blocks, and four fresh verifiers reopened every row's named source. The main session spot-checked at least two rows per finder against the source itself.

Paths. Listing paths are in the repo. `RUNS` is `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs`. Two sources sit outside the repo and are flagged where used: the Ghidra decompile `RUNS\2026-10-28\ch-dynamics-2\out26.c`, and the il2cpp dump `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs`.

Exits. Each row closes by one of four: **write-back** (fixed later on the page, after the user's yes), **question** (to the user), **ruling** (covered by the user's quoted words), **not a problem** (one sentence why).

## Questions for the user

Each question has both sides quoted. Q1 to Q5 come first; Q6 to Q10 follow in a second message.

**Q1. A double lane's second pad (NL-55). Hydra differs from the game when the lane's first chord has two or more pads.**
- The game: the builder's second search (`docs/audit/drum-lanes/f20d12c0.asm` lines 157 to 163) starts from the first chord's own tick and index. The search (`docs/audit/drum-lanes/f5de790.asm` lines 69 to 78) accepts a chord only when its tick is later, or the tick is equal and the index differs. So the first chord is always skipped, and the second pad is the highest pad of a later chord.
- Hydra: `src/parse/song.cpp` lines 144 to 145, "if (first.count >= 2) { second = first.highest; }".
- The decision log, D112 item 3 (`docs/audit/2026-10-03-fix-decisions.md` line 625), describes the game's rule as "the first chord itself when it has two or more pads". The listing does not show that.
- The user's ruling on the pad rule was "go with clone hero" (card 2026-10-11-14ef4882 line 18), which names the game's rule as the target. It does not name this difference.
- Example: first chord red plus yellow cymbal, next chord blue tom. The game lights red and blue tom. Hydra lights red and yellow cymbal.
- Open: whether the game's chord list can hold two chords at one tick (the merge at `docs/clone-hero/code/6142/kick_merge_215B8C0.asm` lines 33 to 98 suggests not).

**Q2. Do decompiles outside the repo count as the real code?** Two kinds of rows rest on them. The `.chart` events reader rules (CR-32, CR-40) rest on `out26.c`, a Ghidra decompile of the game. The virtual slot numbers (slot 20 is `0x20F4B60` at `dump.cs` line 530260, slot 14 is `0x20F5640` at line 530185, and others) rest on `dump.cs`. Both are the game's own code, but neither is committed. If yes, rows S1-11, S1-13, T1-12/N2 (SP-21), T1-16 and S5-4 earn "Proven in the game's code". If no, they stay leads.

**Q3. A `.chart` event that starts with "[" but has no closing "]" (CR-40).** The game's decompile (`out26.c` lines 750 to 757) drops a leading "[" always, and the last character only when it is "]". Hydra (`src/parse/song.cpp` lines 1585 to 1586) strips the brackets only when both are there. This matters only for a malformed event like "[end". Fix Hydra, or leave it as a recorded edge case?

**Q4. Hydra's comment on markers with no note (NL-08).** `src/core/model.cpp` lines 288 to 291 say: "A cymbal, ghost or accent marker with no note of its colour under it is a malformed line ... Clone Hero skips such a marker too (0x215DDB0)." The game's check at `0x215DDB0` (`docs/audit/note-shuffle/step0/open2/f215dbd0.asm` lines 103 to 110) only tests that a range lookup came back empty. The listing does not show what the lookup keys on, so it proves neither "no note at the tick" nor "no note of its colour". NL-08 is Unknown. Proposed: reword the comment to say the colour case is not proven in the game. Hydra's behaviour does not change.

**Q5. Two public sentences say more than the evidence.**
- `docs/UserGuide.md` line 396: "the score that earns the star, rounded up". SC-26 is Unknown: the rounding helper called at `docs/clone-hero/code/6142/star_count_210DD00.asm` line 47 has no listing. The test at `tests/test_stars.cpp` lines 226 to 230 pins Hydra's own `std::ceil`, not the game.
- `docs/UserGuide.md` line 388: "The solo bonus does not count toward stars, because Clone Hero compares your score without it." SC-29 is Unknown: four callers of `0x210DD00` are unread. No saved run lands on a star cutoff.
- Soften both sentences, or leave them until a listing or run settles them?

**Q6. Two "Proven in a live run" labels rest on prose and uncommitted video (NL-28, NL-33).** Their only record is `docs/audit/note-shuffle/step0/FINDINGS.md` lines 53 and 55. The videos `C:\Users\Patrick\Videos\group 1.mp4` to `group 4.mp4` exist but are not committed, and `docs/audit/note-shuffle/game-tests/predictions.json` holds predictions only. Under the code-is-truth rule, relabel them (NL-33's sort direction is now proven from code, S2-1), or commit a data reading of the videos?

**Q7. The solo bonus formula does not fit two runs (SC-20).** `RUNS\2026-11-06\ls-d2d-run1` scored a solo bonus of 56,020 and `RUNS\2026-11-06\ls-o1-run1` 49,700 (their scorestats files). With 580 solo notes (58,000 / 100), the page's formula gives no whole number of hits for either. Either the note count or the formula is wrong. This stays open until a chart's solo note count and the press CSV are read; nothing will be claimed either way. Send a scout to settle it?

**Q8. Star Power did not extend in LS-M2 (SP-34).** In `RUNS\2026-11-08\ls-m2-run1`, the phrase-end chord 618 was hit during Star Power and the end did not move (`sp_watch-2.csv`, `sp_end_s` flat at 163.4769). Chord 609, the phrase's first chord and the activation chord, had one part never pressed. That would leave the phrase unearned (30 of 31 earned). But from chord 609 on the game's score rises are about half of Hydra's (`press-2.csv` rows 609 to 611), so a rival cause exists. Record it on SP-34 as an open exception, or settle it with per-note data first?

**Q9. The rulings file's tags.** `docs/agents/rulings/clone-hero.md` has seven rows ending "backs: no page yet". Six are carried by blocks (line 14 by SC-15; line 15 by CR-15 and CR-19; line 16 by SC-32; line 17 by NL-16 and NL-29; line 18 by NL-21; line 9 by `docs/clone-hero/code/README.md` line 237, which states the result but quotes no ruling). Line 17 quotes "go with the game's rule" where the user typed "go with the games rule". Line 19, "85 ms both sides (Recommended)", is a setting of Hydra's fill-comparison tool, and no block backs it. Repoint the six, fix line 17's quote to the typed words, and mark line 19 as a tool setting with no game block?

**Q10. A game marker spans a range of notes (NL-07).** When the lookup at `0x215DDB0` finds a range, the game adds the marker to every note in it (`f215dbd0.asm` lines 117 to 136). Hydra's handling was not checked. Send a scout to read Hydra's marker code against this before any claim?

## Covered by a ruling

- **NL-58, the lane drop rule (S1-4).** The game drops a lane whose first non-kick chord lands later than start plus a run-time tolerance (`docs/audit/drum-lanes/f20d12c0.asm` lines 74 to 104, 145 to 147). Hydra counts any note inside the lane. The user chose "Any note inside the lane (Recommended)" for "Clone Hero drops a lane when its first note lands too late after the lane's start" (card `docs/sessions/2026-10-11-14ef4882.md` line 17; page 02 line 602). The block's "no listing" sentence is still a write-back.

## Not a problem

- **S4-8, the swapped formula tables in `tools/ch_probe/constants.py`.** TM-28 already says it.
- **S3-9, NL-07's skip.** The block already cites `f215dbd0.asm` lines 104 to 110 (its small "jumps to the exit" wording is a write-back below).
- **S6-10, TM-08.** The only source is a prose note; the block already says no listing exists.
- **T1-14, SP-16.** No listing of `0x20F57F0` exists; the block says so.
- **S8-9, IN-61.** The launch patch address in `tools/ch_mcp/launch_patch.json` matches `docs/clone-hero/code/6142/cli_loader_3968D0.asm` line 328 (`mov edx, 1`).

A fresh closure checker (task CHDG-T5) returned PASS for the ruling row and for each row above.

## Write-backs

Each row: the block, what to write, the source, and the label the evidence earns. These change no Hydra behaviour. Rows marked (Q2) depend on question 2.

### Page 01, reading a chart

- **CR-10.** The game's scorestats `checksum` for a folder chart equals the MD5 of its notes file: `RUNS\2026-11-08\ls-m2-run1` scorestats `checksum` AF44D5C8D941BB33444D18E7871CDC77 equals the MD5 of the Who `notes.mid`. Proven in a live run for the folder half.
- **CR-32.** The events reader takes the text between the first and the last double quote (`out26.c` lines 687 and 727). Proven in the game's code (Q2).
- **CR-45.** Soften the Rule: CR-45 states flatly that `song_length` is not read, while CR-48 says neither is proven absent. Wording only.
- **CR-46.** The song-end chain is read: `docs/clone-hero/code/6142/gm_load_coroutine_D8D80.asm` line 351 (Math.Max), 357, 380 (Math.Min), 388 (adds 1.0). Proven in the game's code for that branch. Also record a factor from `0x210C090` multiplied in at lines 316 to 342 and 376 to 377 (unread), and that the other branch (lines 300 to 309) reuses the stored length.

### Page 02, the note list

- **NL-14 and NL-15.** The scorer `docs/clone-hero/code/6142/note_score_add_20DEEC0.asm` lines 17 to 24 doubles any note whose `+0x84` mask-4 flag is set, with no kick test. `RUNS\2026-11-08\ls-m0-run1` (scorestats band_score 525,555, accents 166 of 166) has all 166 accents hit, and base score less note score is exactly 50 per ghost or accent hit. Hydra's own score file for that chart, `RUNS\2026-11-08\kick-prep\ahab\Mastodon - Leviathan - 02 - I Am Ahab-score.json`, gives `final.total` 525,555 and `final.accent` 8,300 (166 times 50), with 33 of the 166 accents on the kick. The game's score equals it, so NL-15 earns Proven in a live run on that one chart. NL-14 earns the count half only: `ls-m2-run1` counts 160 ghosts including 36 kick ghosts; that a ghost kick pays double is still unproven. What sets the mask-4 flag on a kick is unread.
- **NL-16.** A note with both marks gets only the accent flag `0x13`, never the ghost flag `0x12` (`docs/audit/note-shuffle/step0/open2/f20d4be0_notelist.asm` lines 143 to 153; getters at `getters.asm` lines 19 to 29). Proven in the game's code; the block's "listing lost" is stale.
- **NL-22.** The listings of `0x20EF450` and `0x20F1420` exist (`docs/audit/note-shuffle/step0/open2-refute/`); `f20F1420.asm` lines 29 to 61 rewrite type words behind a flag test. Stays Unknown for the flip; "not read" is stale. Page 02 lines 228 and 632 disagree about `0x20F1420`.
- **NL-30.** The merge keeps the longest note and ORs the high flag bits (`kick_merge_215B8C0.asm` lines 33 to 128); on a length tie the lower index is kept, so file order can still change which object survives. Stays Unknown.
- **NL-33.** The sort inside a tick is type ascending, ties by flags ascending (`docs/audit/note-shuffle/step0/open2/f215c040.asm` lines 144 to 155). Proven in the game's code for the direction (see Q6 for the live label).
- **NL-34.** The `.chart` and `.mid` parsers end in the same finalizer (`f2161a80.asm` line 221, `f2155050.asm` line 557, `f2156990.asm` line 655, all in `step0/open2/`). Proven in the game's code; Unknown only for a live `.chart` test.
- **NL-37 and NL-38.** The flam step is fully read (`docs/audit/note-shuffle/step0/open2/drum_pretransforms_20ef450_...asm` lines 29, 61 to 70, 77 to 78, 85, 104 to 157, 175 to 180): kick left out of the pad count, flag `0x20000` gates it, the copy goes in right after the source, one pad up, green moves the source to blue, and flags are copied whole after `or 3` on the source. Proven in the game's code.
- **NL-39.** The `.chart` modifier decoder handles 5, 6, 34 to 44 and 66 to 68 only (`chart_modifier_decode_213D160.asm`). Half proven; cite the listing.
- **NL-42.** The jump-table decoder script `docs/audit/note-shuffle/step0/open34-refute/jumptables.py` is committed (needs the DLL, hand labels, no saved output). Stays Unknown; "only prose" is stale.
- **NL-43 (new beside it).** The shuffle is also called for non-drum instruments when bit `0x40` is set (`docs/audit/note-shuffle/scout-flag/build_20d2ea0.asm` lines 153 to 162; `step0/open34/callers.txt` lines 5 to 6). Proven in the game's code that the call is made.
- **NL-45 and NL-46.** The chord pattern is the OR of every note's lane mask, and the rank search lists set bits lowest first (`docs/audit/note-shuffle/scout-flag/helpers.asm`; `step0/open34/shuffle_20f0c30.asm` lines 215, 242, 247). Proven in the game's code.
- **NL-48.** `docs/audit/note-shuffle/probes/reach/reach_check.py` matches `lanedraw_20f0b30.asm` lines 12 to 52 step for step, and its run shows the full period. Proven in the game's code for "full period"; worst cases are sampled.
- **NL-51 or NL-47 (new line).** The note builder `0x20D2EA0` has four callers (`step0/open34/callers.txt` lines 1 to 4).
- **NL-53.** The velocity mapping is read (`docs/audit/drum-lanes/play/f2155200_mid_reader_lane_case.asm` lines 93 to 107, 126 to 140): 51 and up gives 3, 41 to 50 gives 2, 31 to 40 gives 1, 21 to 30 gives 0, under 21 gives 3.
- **NL-54.** `.chart` special type 0x41 (S 65) makes a single lane and 0x42 (S 66) a double lane, through a subtract chain (`docs/audit/drum-lanes/spec_213e0c0.asm` lines 65 to 79 and 86 to 91: type 65 passes 0 and type 66 passes 1 to `0x1820CEEC0`; type 64 goes to a different list). Proven in the game's code that the two types differ. This listing does not show that the last argument means single or double, so cite the lane constructor for that.
- **NL-55.** The pad rule is read (`f20d12c0.asm`, `f20d5310.asm`, `f20d51d0.asm`, `f5de790.asm`): kicks masked out, lowest pad of the first chord, and for a double lane the highest pad of a later chord. Proven in the game's code. See Q1 for Hydra. Also: the flagged range runs to tick start plus length plus 1 (`f20d12c0.asm` lines 183 to 186, `f5deb40.asm` lines 61 to 73).
- **NL-56 and NL-57.** The builder sets `0x4000`, `0x8000` and `0x10000` only on notes overlapping the lit pads, `0x10000` on the last flagged note (`f20d12c0.asm` lines 205 to 230). Record both lane object layouts (`lane_ctor_2116390.asm` lines 1 to 7; `f20d12c0.asm` lines 267 to 290) and that a single lane goes before a double at the same tick (`f215e4f0.asm` lines 41 to 86).
- **NL-58.** Replace "no listing" with the listing lines (see "Covered by a ruling").
- **NL-59.** Add the lane join (`docs/audit/drum-lanes/f20de420.asm`), the round press with no note (`play/f20de9a0.asm` lines 6 to 23, 48 to 57), the three-hit and non-kick-miss clears (`f20de770.asm` lines 8 to 23; `f20dcad0.asm` lines 74 to 80), and the 0.5 s timeout (`play/f20ddec0.asm` lines 17 to 29). Say the combo-breaker call is slot 20, `0x20F4B60` (Q2). The second listing `play/f20de5b0.asm` is a copy with no known caller.
- **NL-65.** In the builder the plain-drums clear is chosen by the instrument byte only (`note_builder_20D2EA0.asm` lines 245 to 253). Stays Unknown.
- **NL-71.** Both video scripts are committed; all three chord orders match 22 of 22, so the video cannot tell them apart. The 5-of-22 figure has no script. Stays Unknown.
- **New block.** `docs/clone-hero/code/6142/two_x_kick_drop_20F0320.asm` is not the 2x-kick drop. It rewrites the `+0x80` type word (4 becomes 8, or 2 when flag bit 8 or 0x40 is set; 0x40 becomes 0x80, or 0x20 when bit 0x10 or 0x80 is set). The builder calls it only when a byte of its second argument equals 3 (`note_builder_20D2EA0.asm` lines 50, 237 to 242). Proven in the game's code; the byte's meaning is Unknown. Fix `code/README.md` line 333's "five-lane" wording.
- **New line.** Name `0x21555CD` and `0x21555F1` in the 2x-kick pitch block (`mid_drum_reader_2155050.asm` lines 316 to 324), so the test comments' addresses resolve.

### Page 03, timing

- **TM-10.** The load coroutine's listing exists (`gm_load_coroutine_D8D80.asm` lines 483 to 491); it is not `Update`. Stays Unknown; "listing lost" is stale.
- **TM-24, TM-26, TM-27.** `docs/audit/drum-lanes/play/f20f5810_base_update.asm` lines 74 to 103 show the half window at `+0x10`, minus it at `+0x18`, the whole at `+0x20`, the `+0x195` mirror, and the update admitting only notes inside the stored window (append at lines 127 to 131). The code halves earn Proven in the game's code; drop each "no listing" line. What sets `+0x195` is unread.
- **TM-28.** Its `constants.py` line cites have drifted: `RVA_FORMULA_NORMAL` is at line 170 and `RVA_FORMULA_PRECISION` at line 179 (the block says 166 and 175).
- **TM-30.** Contradicted. The 0.75 constant is the double-lane round timer (`play/f20ddec0.asm` lines 31 to 32), as NL-59 and IN-15 already say, not a hit-grade fraction.
- **TM-36.** A third reader of `0x16C490` exists and does not draw (`gm_load_coroutine_D8D80.asm` line 489), so "only drawn" is not shown.
- **TM-45.** Drop "limits not opened": `docs/audit/drum-lanes/f20dcad0.asm` lines 22 to 27 give 2 on a single lane and 4 on a double. Add that the 0.075 s grant needs the pad in the lane mask (lines 66 to 73).
- **TM-46.** The overhit path is readable (`play/f20deaa0.asm` lines 201 to 209; `play/f20de9a0.asm` lines 24 to 44). The tom-plus-cymbal question stays Unknown.
- **New block.** The engine queues each note's window start and end, nudged outward, and runs its update at each queued time (`play/f20f6a50_advance_schedule.asm` lines 51 to 126; `play/f20f43a0_add_event.asm` lines 31 to 73; `play/f20f6ff0_next_event.asm` lines 44 to 106; `play/f20f4950_step_loop.asm` lines 29 to 60). Proven in the game's code for the shape; the nudge helper `0x180F96AA0` is unlisted. Check IN-17 and IN-18 first.

### Page 04, fills

- **FL-09.** The listing shows the highest set bit (`docs/audit/drum-lanes/f20d51d0.asm` lines 29 to 47, 71 to 74), and the fill passes an all-ones mask (`fill_to_activation_20CFF60.asm` lines 99 to 102). Drop "FINDINGS only".
- **FL-31.** Add the live grants: `RUNS\2026-11-07\ls-r1-run2\press-1.csv` chords 221 and 845, and `RUNS\2026-11-07\ls-r2ab-run1\press-2.csv` chord 854. Nothing was pressed, the next row rose by the gem's 200 on top of its own 200, the 200 appears in the `sp_watch` CSV 64 to 88 ms after the gem's chart time, the meter was at least half, and no activation followed. Chord 801 (two of three gems pressed) and LS-M2's nine declined chords support it. Proven in a live run for a half-or-more meter and a single-gem fill end. Say the press-side call is slot 14, ActivateStarPower (Q2).

### Page 05, Star Power

- **SP-07.** `docs/audit/drum-lanes/spec_213e0c0.asm` lines 20 to 59 builds a range for `.chart` type 2 through `0x20D5E60`. Stays Unknown; "listing lost" is stale.
- **SP-08.** `docs/audit/note-shuffle/step0/open2/f215c320.asm` lines 101 to 123 has five references to `0x5DB320`, not three; nothing ties it to phrases. Stays Unknown.
- **SP-17.** The dynamics handler `0x20DE770` never calls `0x214BB50` (`docs/clone-hero/code/6142/note_hit_dynamics_20DE770.asm`, ten calls, none to it). Proven in the game's code for this listing. Clears card 70bddae3's write-back tag.
- **SP-20.** A wrong-pad press while a `0x800` note is in the window hits it without activating (`play/f20deaa0.asm` lines 107 to 108, 163 to 200).
- **SP-21.** The order is activation (`f20deaa0.asm` line 130, slot 14), hit (line 152, slot 11), then the score add with the doubled multiplier (`add_note_score_20F6EB0.asm` line 40; `sp_activate_20F5640.asm` lines 36 to 40). Proven in the game's code (Q2); not watched in a run.
- **SP-28.** Cite `play/f20f43a0_add_event.asm`; the end check also runs at queued times between frames (`play/f20f6ff0_next_event.asm` line 103, `base_frame_update_20F4550.asm` line 37).
- **SP-29 and SP-30.** Both listings are committed (`play/f20f4950_step_loop.asm`, `play/f20f6ff0_next_event.asm`). The clock is stepped to up to three queued times when `+0x290` is set. Whether a hit is judged at those clocks is not shown, so both stay Unknown; their "not listed" lines are stale.
- **SP-33.** `RUNS\2026-11-07\ls-r1-run2` chord 740: a gem hit 15.27 ms after `sp_end_s` paid 200, not doubled, and a phrase was credited after the end (`sp_watch-1.csv`, the meter steps 0 to 1,440 at 118.4452). Proven in a live run for that offset; exactly-at-end stays open.
- **SP-36.** `+0x10` is the early-side half window, 0 when `+0x195` is set (`f20f5810_base_update.asm` lines 74 to 84).
- **SP-43.** The two 0.003 compares (`docs/clone-hero/code/6142/clock_tick_16C650.asm` lines 105 and 116) are the audio-versus-raw drift threshold, not a scoring leeway.

### Page 06, scoring

- **SC-19.** As NL-15: `ls-m0-run1` is a saved run with kick accents. Drop "no saved run has a kick accent". ADR 0012 lines 24 to 32 are stale too.
- **SC-20 and the SC-37 table.** Contradicted. Four runs have a nonzero `solo_bonus_total` (`ls-d2d-run1` 56,020, `ls-o1-run1` 49,700, `ls-o3-run1` 58,000, `ls-m2-run1` 12,100), and `squeezed_score` is nonzero in eight of the 16. Fix SC-20's Evidence and table lines 419 and 420. The formula stays Unknown (Q7).
- **SC-31 and SC-33.** In all 16 scorestats files under `RUNS`, the score equals notes plus combo plus Star Power plus solo bonus plus 50 per ghost or accent hit, and `squeezed_score` is never in the sum. The game's `avg_multiplier` equals the score less the solo bonus over the base score, to the file's float precision, in all four solo runs, the same as Hydra (`src/core/model.cpp` lines 841 to 845). Proven in a live run; the solo term is now checked.
- **SC-32.** Base score less note score equals 50 per ghost or accent hit in every all-notes-hit run (for example `ls-m0-run1` 132,295 less 115,645 is 16,650). Proven in a live run for the file's field.
- **SC-38.** Add the rerun: `ls-r1-run2` scored 552,770, equal to Hydra, closing the 795-point gap.

### Page 07, internals

- **IN-10.** Fifteen sites test bit `0x1000` of `+0x198` (`docs/audit/note-shuffle/scout-flag/scan_198_1000.txt`); two are named.
- **IN-12.** Contradicted. The `0xC000` test (`play/f20deaa0.asm` line 131) passes a single-lane `0x4000` note, so `+0x2E0` is written on single lanes too. Its writers are `f20de420.asm` line 25, `play/f20de9a0.asm` line 53, `play/f20deaa0.asm` lines 140 and 188, `f20dcad0.asm` line 56 and the constructor `engine_ctor_20DF680.asm` line 65. Cut "(a note in a double lane)" and "never written with no double lane".
- **IN-13.** Contradicted. `+0x88` is zeroed by the combo break (`play/f20f4b60_combo_break.asm` line 28), `+0xB0` is the overhit counter (`play/f20de9a0.asm` line 44), and `+0x8C` counts window notes, zeroed each update (`play/f20f5540_slot9_base_and_activate_sp.asm` line 7) and recounted (`f20f5810_base_update.asm` line 131).
- **IN-15.** The combo-breaker call is slot 20, `0x20F4B60` (Q2); align with NL-59. `play/f20de5b0.asm` is a copy with no known caller.
- **IN-33.** Three more role `+0x190 == 1` sites (`f20f4b60_combo_break.asm` line 24, `f20f3610_base_miss.asm` line 32, `f20deaa0.asm` line 115).
- **IN-41.** The result file was opened: injected keys hit 857 of 857 (`tools/ch_probe/experiments/results/overlay-spike/run1_cal0_play_chart.txt` lines 83 to 87). Proven in a live run for that; the rest stays open.
- **IN-55.** Card e8435f64's question (does `play_count` count other instruments) is not answered by any data.
- **New block, the combo break.** `0x20F4B60` zeroes the streak, fires an event with the old streak, and does nothing for role `+0x190 == 1` (`play/f20f4b60_combo_break.asm` lines 15, 24 to 28, 58 to 69). The slot-10 call at line 31 is unresolved.
- **New block, a region flag.** `+0xF0` is set while the clock is inside an item of the `+0x1F8` list, and overhits are ignored then (`play/f20f5540_slot9_base_and_activate_sp.asm` lines 14 to 57; `play/f20de9a0.asm` line 3). What the list items are is Unknown.
- **New block, the note getters.** `docs/audit/note-shuffle/scout-flag/note_getters.txt` lists 34 getters over `+0x7C`, `+0x80` and `+0x84`; record each bit tested. Meanings Unknown.
- **New block, engine starting values.** `docs/clone-hero/code/6142/engine_ctor_20DF680.asm` lines 22 to 73 store the starting values of fields no page names. Meanings Unknown.
- **Unknown leads.** Each of these comes from tool code or prose only, so it goes in as Unknown: the default key map (`tools/ch_probe/input_driver.py` line 69), the Rewired registry key (`tools/ch_mcp/game_files.py` line 70), "a restart makes a new engine object" (`tools/ch_probe/README.md` lines 42 to 43), the per-frame menu method `0x32F590` (`tools/ch_mcp/agent/menu.ts` line 54), and `poll_rate` and `settings_schema.json` (`docs/research/2026-10-28-clone-hero-mcp-feasibility.md` lines 35 and 84).
- **Line fix.** Page 07 line 157 cites `constants.py` line 191; it is 195.

### Across pages

- **Hydra line numbers have drifted.** Examples: `src/core/model.h` shifted by one from about line 56 (`kHitWindowMs` 105, `kSqueezeWindowMs` 111, `kSoloBonusPerNote` 147); `src/core/squeeze_rating.cpp` `squeeze_budget_ms` 126; `src/core/timing.cpp` `plusmeasure` 320; `src/app/dm_report.cpp` 128; `src/core/model.cpp` `Path::avg_mult` 841; `src/parse/song.cpp` `close_sp_phrase` 400. Re-pin every Hydra line cite on the pages and run the docs check.
- **`docs/sessions/index.md`.** Clear the stale `[write-back]` tags on b132faae and d0a3868c (TM-49 and SP-47 hold them), and on 70bddae3 once SP-17 is written.
- **The handoffs.** `docs/handoffs/2026-10-31-audit-of-audits-rest-handoff.md` line 31 says a live run is needed for `prev_tick = 0`; NL-72 already proves it in code.

## Open, needing a run or a read not in the repo

These go on the pages as Unknown with the read named, never as proven: what `0x1805fad90` and its byte key are (NL-08); whether the game's chord list can hold two chords at one tick (Q1); the rounding helper `0x183031130` and the four `0x210DD00` callers (SC-26, SC-29); the solo note count for SC-20; the per-note multiplier after chord 609 in LS-M2 (SP-34); whether a ghost kick pays double (NL-14); what sets `+0x195`, `+0x290`, `+0x308`, `+0x2F6` and the kick's mask-4 flag; the slot-10 target in the combo break; the `0x210C090` factor in the song-end chain; whether `0x20DE5B0` is ever called; chord 803 in LS-F1 (S3-7, a candidate only); the game's rich-text tag set (S3-10: no block holds it, and the only game data is the one tag pair `<b><i>` the score writer emits, `docs/clone-hero/code/6142/scorestats_writer_15DC20.asm` lines 93 and 383).

## Sources read and not read

Read in full or by every row: the drum-lanes audit and its listings; the Note Shuffle audit's notes files, game tests, probes and verify scripts; `docs/clone-hero/code/6142/` by name; every scorestats file under `RUNS`; the press and Star Power watch CSVs of ls-r1-run2, ls-r2ab-run1, ls-m0-run1, ls-m2-run1 and ls-f1-run1; the 137 Unknown and assumed blocks; the rulings file; the three write-back cards.

Read in part: about 170 of 179 Note Shuffle listings were not opened (73 have no citing page; that list was not kept); the 80 `clone-hero` cards by one grep; about 30 memory notes of 120; 27 run folders under `RUNS` hold no scorestats file and were not opened (only the five runs named above had their CSVs read); the decision log by address only; ADRs 0004, 0006 and 0010 by grep; `docs/UserGuide.md` lines 160 to 730 by grep.

Not read: the 237 removed plans (the user agreed to leave them out); the derivation audit and ledger beyond a grep (agreed).
