# G4a: is a chord's score broken down in more than one place?

Scout: G4a, session ec019463. Files read in full: core/scoring.{h,cpp}, core/replay.{h,cpp}, core/backend_value.h, core/stars.{h,cpp}, search/graph.h, app/dynamics_breakdown.{h,cpp}, ui/dynamics_tab.cpp, ui/stars_tab.cpp. I also read the lines I needed in core/model.{h,cpp}, search/graph.cpp, search/engine.cpp and app/path_view.cpp to find the other end of each pair.

## Short answer

A chord's score is broken down in one place: `category_scores` in core/scoring.cpp. Everything else in the group is computed from its output. The three struct pairs from the probe are all "computed from", not "each adds up the same categories itself".

- `CategoryScores` -> `ChordScoreRow`: `chord_score_table` copies the fields across one by one (scoring.cpp 108-125). The row adds `solo` (from `solo_bonus`), `combo_before` and the note range.
- `ChordScoreRow` -> `ReplayChord`: `replay_path` copies `combo_before`, `multiplier`, `multiplier_after`, `combo_after` straight off the row (replay.cpp 117-119, 187). It then fills `ReplayChord::points` (a `ReplayScore`) from the row's base, combo, solo, accent and ghost, and prices `sp` itself from the row's `sp` and `sqout_sp` through `core::backend_row_value` (replay.cpp 180-185).
- `CategoryScores` -> `ReplayScore`: there is no direct link. `ReplayScore` is fed from the row, so it is two steps downstream.

Nothing in the group re-adds a note's base, combo, SP, ghost or accent. The one real "same word, different number" is `sp`: the row's `sp` is the chord's full doubling if Star Power pays it, while `ReplayScore::sp` is what the windows actually paid (0 outside a window, `sqout_sp` on the squeezed-out chord). That is by design and the replay header says so, so I list it only as a note under G4a-1.

The candidates below are the second lists and second sums of the categories. None is a second breakdown of a chord.

## Candidates

### G4a-1. Which score categories does a path have, and in what order?
Looks like: K3-with-a-rule-inside (parallel on purpose, mostly known). One of the parallel parts holds a rule: the engine's running total.
Homes (all spell the same six categories base, combo, sp, solo, accent, ghost):
- `Path::score_base .. score_ghosts`, core/model.h 745-750. Inputs: the stored record.
- `ReplayScore`, core/replay.h 105-126. Inputs: the six values; owns `total()`, `add()`, `operator==`.
- `kReplayScoreFields`, core/replay.h 135-139. Inputs: the six JSON names, used by tools/replay.cpp 650 and tools/replay_json.cpp 93.
- `score_of` and `assign_score`, core/replay.cpp 214-232. Inputs: the Path/ReplayScore pairing, both directions.
- `ScoreGraphEdge::basescore .. ghostscore`, search/graph.h 144-149, with six `store_*score` methods (graph.h 232-237, graph.cpp 235-256 and the callers at graph.cpp 135-144).
- `EdgeView::basescore .. ghostscore` (engine.cpp 64-66), `OutPath::score_base .. score_ghosts` (engine.cpp 313-315), and the positional array `sc[6]` (engine.cpp 288), read as `sc[0]`..`sc[5]` at engine.cpp 820-825 and 1722-1727.
- `Path::prepare_variants` copies the six fields to each variant (model.cpp 664-669); engine.cpp 1909-1914 copies `OutPath` into `Path`.
- `build_score_breakdown`, app/path_view.cpp 429-434. Inputs: the six fields; owns the six labels.
- The total: `score_total` (model.h 149) is called by `Path::totalscore`, `ReplayScore::total` and the check in `Engine::emit_path` (engine.cpp 1715). But the engine's per-edge step adds the six again as its own sum (engine.cpp 826-827, `p.score += ...`).
Table, home 1 (score_total): (b, c, s, o, a, g) -> b+c+s+o+a+g. Always all six, no edge case.
Table, home 2 (engine.cpp 826): (e.basescore, comboscore, spscore, soloscore, accentscore, ghostscore) -> the same sum, added to `p.score` one edge at a time. Also `c.score += sp_delta` at 1076 and the frontend points at 962, which add to `sc[2]` and `score` together.
Differ on: nothing today. `emit_path` throws if `p.score` and `score_total(sc)` ever disagree (engine.cpp 1715-1720), so a seventh category added to one and not the other fails loudly at the first path.
Note: derivation-audit finding at line 3359 (the six-field sum) and finding 282 (the Path/ReplayScore pairing) are the line-level versions of this. Both were fixed by `score_total` and `assign_score`, so the leftover is the list itself (about ten spellings of six names). Test `test_replay.cpp` 1396-1415 pins `kReplayScoreFields` against the field order. I did not check whether a test pins the engine's `sc[]` order against `Path`; the verifier should. Phase 4 should decide if this is a finding or only "adding a category touches ten places", which is the nature of a six-field record. A cheap check: is there any place that would add a seventh category and compile without error but leave one list out? The engine's `sc[6]` fixed size and `emit_path`'s check would catch two of them; the labels in path_view would not.

### G4a-2. How is a chord's chart base score related to the stored "base" category?
Looks like: K2 (same word "base", two quantities). Probably harmless; one question for the verifier.
Homes: `category_scores`, core/scoring.cpp 49-51 (inputs: a note's 1x value `ChordNote::basescore`, its ghost/accent flags); `Path::chart_base_score`, core/model.cpp 848-850 (inputs: the three stored categories `score_base`, `score_ghosts`, `score_accents`).
Table, home 1: base category = 1x value - accent share - ghost share, where the accent or ghost share is the pad's 50 (`kNoteBasePoints`). Plain note: base = 1x value. Ghost or accent note: base = 1x value - 50. Example: a ghost yellow cymbal is worth (50+15)*2 = 130 at 1x (model.cpp 83-87); its ghost category is 50 and its base category is 80.
Table, home 2: chart base score = base + ghosts + accents, which gives back the sum of the 1x values of all notes. Same example: 80 + 50 = 130.
Differ on: nothing. The two are inverses and a test ties them (test_stars.cpp 89, `n.base + n.accent + n.ghost == row.note.basescore()`; derivation-audit line 3279 says the same). The word "base" is used for both the category (path view "Notes:", graph `basescore`, `ReplayScore::base`) and the whole 1x value (`ChordNote::basescore`, `StarCutoffs::base`, stars tab "Base score:"). A reader of the Stars tab and the Paths score breakdown sees "Base score" and "Notes" for different numbers on the same path.
Note: the user sees both numbers. Verify on a chart with ghosts or accents: Stars tab "Base score" should be Paths "Notes" + "Accent Notes" + "Ghost Notes". The code says so. I did not run it.

### G4a-3. What does an accent or ghost earn?
Looks like: K2 (two numbers, documented), possibly K0.
Homes: `CategoryScores::accent` / `::ghost`, scoring.cpp 49-50 (always the pad's 50, before the multiplier); `CategoryScores::dynamics_bonus`, scoring.cpp 80-84 (paid value less a plain note's paid value on the same pad, multiplier included).
Table, home 1 (accent category): accent note on any pad -> 50 at 1x, times nothing; the multiplier lives in the combo category.
Table, home 2 (dynamics_bonus): accent note on a tom or snare -> 50 * multiplier. Accent note on a cymbal -> 65 * multiplier (the plain cymbal is 65, the accent 130). Normal note -> 0.
Differ on: cymbal ghosts and accents. The category says 50, the bonus says 65 at 1x. The header comment at scoring.h 39-43 states the split on purpose: `accent` and `ghost` hold only the pad's 50, `dynamics_bonus` is what a mis-hit loses. The Preview reads `dynamics_bonus` (replay.cpp 136) and the Paths view reads the category total.
Note: the user sees "Accent Notes" in the Paths score breakdown, which sums the 50s, not the true gain on cymbals. I found no place that is wrong; the question for phase 4 is whether the label "Accent Notes" should be read as "the pad's 50" and whether that is what the user expects.

### G4a-4. Is this chord after the SP end?
Looks like: K1 (two tests of one question, with a documented equivalence).
Homes: `core::paid_by_sp_walk`, core/backend_value.h 30 (input: offset in ms from the SP end; answer: `offset_ms <= 0`); `core::after_sp_end`, backend_value.h 36 (inputs: chord tick, end tick; answer: `chord_tick > end_tick`).
Table, home 1 (inside SP when offset <= 0): offset < 0 -> inside; offset exactly 0 -> inside; offset > 0 -> outside.
Table, home 2 (after when tick > end tick): tick < end tick -> not after; tick == end tick -> not after; tick > end tick -> after.
Differ on: agree today. They split if two different ticks ever share one ms value, or a later tick maps to an earlier ms. The header comment (backend_value.h 32-35) says parse refuses timing that does not rise (D6), so they agree "wherever time runs forwards".
Note: callers: `paid_by_sp_walk` is used by `counted_without_squeeze`, engine.cpp 1067, model.h 360, squeeze_rating.cpp 119. `after_sp_end` is used by replay.cpp 161 and 416, graph.cpp 430, engine.cpp 1033, sqout_chord.h 92 and 152. The replay uses both in one loop (replay.cpp 161-165): the tick test to decide when a window leaves the open list, the offset test (via `paid_by_sp`) to price. This is close to the G4b / G5 squeeze leads; I report it because the two functions live in a group file. The verifier should check whether D6 really excludes equal-ms ticks (floating-point ms from two close ticks at a very high tempo).

## Parallel on purpose

### The SP score: graph edges against replay_path
Graph side: `ScoreGraphEdge::spscore` summed along the SP track, plus `frontend_points` on the activation edge (graph.cpp 207), plus the deactivation edge's backend delta (engine.cpp 1056-1077). Replay side: `replay_path` prices every chord per window (replay.cpp 150-176).
What each side knows. Graph: which edges the SP track crosses, the cap and fill rules, and the rows kept near each SP end. Replay: a list of windows and a per-chord loop. Both read the same `ChordScoreRow` fields (`sp`, `sqout_sp`) through the same functions (`backend_row_value`, `paid_by_sp`, `after_sp_end`, `sqout_position`). Neither holds a pricing rule; replay.h 9-20 says so, and hydra_replay's selfcheck proves they agree. I checked the claim in the code: the engine's `backend_row_value` call (engine.cpp 1068) and the replay's (replay.cpp 169) are the only two users, with the details table (path_view.cpp 367) a third. Verdict for the verifier: K3, every entry is a per-module fact.

### ChordScoreRow / ReplayChord / ReplayNote
What `ReplayChord` knows beyond the row: tick, ms, measure/beat position, chord code, per-note colour and cymbal flag, in_sp, multiplier_shown, running totals and the on-screen total with the solo bonus withheld. `ReplayNote` copies `sp`, `multiplier`, `dynamics_bonus` off `CategoryScores`. The solo-withholding rule (replay.cpp 192-204) is the one rule inside the replay that is not a copy: when the on-screen solo bonus is paid. It has no second home I could find (grep for `solo_pending`, `cum_onscreen` finds only replay.cpp/h, tools/replay.cpp 424 and preview_view.cpp 211-226, which reads the field). Verdict: K3-adjacent but the rule has one home, so not a candidate.

### Dynamics tab counts against score categories
`DynamicsCounts` (ghost, accent, normal) in dynamics_breakdown.h counts notes per pad, not points. `count_dynamics` (dynamics_breakdown.cpp 113-131) switches on `NoteDynamicType`; `category_scores` reads `is_ghost()` and `is_accent()`. Both read the one `ChordNote::dynamictype`. They answer different questions (how many versus how many points) and share no code. Verdict: K0 for the breakdown question. The dynamic words are a separate small list: `DynamicsCounts` has three named fields mirroring the three enum values, and the tab spells "Ghosts:", "Accents:" and the "Normal" column header by hand (dynamics_tab.cpp 78-80, 214-215) while `dynamic_label` (model.cpp 42) supplies only "Ghost" and "Accent". Normal has no label there (`dynamic_label(Normal)` is empty), so the tab must write it. Not a rule, a word list: a fourth dynamic type would need edits in the enum, `count_dynamics`, `DynamicsCounts` and the tab.

### Dynamics row table
The nine rows in dynamics_breakdown.cpp 43-53 are the single table: row, lane colour, cymbal flag, 2x flag. `dynamics_row_for`, `is_pad_row`, `dynamics_row_note`, `dynamics_row_label` and the tab's `pad_color` and cymbal-skip all read it (dynamics_tab.cpp 36-38, 172). The tab's `lane_color` (dynamics_tab.cpp 24-33) holds the RGB per lane; the Preview's lane colours are in G6; I did not read them. What the table knows: row identity and the lane/flag it matches. Every entry is a per-module fact. Verdict: no candidate.

## Dropped leads

- "With full solo bonus" in core/stars.h and ui/stars_tab.cpp: stars.h 37 only quotes the label in a comment; the label itself lives once, in stars_tab.cpp 40, and the values it shows come from `StarCutoffs::with_solo` (stars.cpp 22). K0 (a comment naming a UI label; the preamble asks comments to name the owner, so the comment could be reworded, but there is no second rule).
- "Yellow cymbal" in dynamics_breakdown.h and core/model.h: both are comment examples (model.h 73, dynamics_breakdown.h 114) of `note_label`'s output. The string is built once in `note_label`; `dynamics_row_label` calls it (dynamics_breakdown.cpp 96-99). K0.
- ChordScoreRow against ReplayScore and ReplayChord, CategoryScores against ReplayScore: not duplicate breakdowns; see Short answer. The six-name list they belong to is G4a-1.
- Known, line-level: the six-field sum and the Path/ReplayScore pairing (derivation-audit lines 3359 and 5242-5254). Both already have an owner (`score_total`, `score_of`/`assign_score`).
- Two small label notes, not candidates: "Solo Bonus:" in path_view.cpp 432 against "Solo bonus:" in stars_tab.cpp 29 (same quantity, `score_solo`, different capitalisation and a different page style), and `StarCutoffs::base` against `ReplayScore::base` (different quantities, see G4a-2).

## Not read

None. All twelve files were read in full.

## Things I checked and found fine

- `ChordNote::basescore` (model.cpp 83) is the one place a note's 1x value is worked out; `category_scores`, `multsqueeze_gain` and `MultSqueeze::applies` and `squeeze_advice` all call it. `MultSqueeze::applies` (model.cpp 373-381) decides whether a squeeze exists with its own `to_multiplier` comparison, and `category_scores` pays by `to_multiplier` per note; both call `to_multiplier`, so they cannot drift.
- `kStarPowerMultiplier` is read by `category_scores` (scoring.cpp 55) and `shown_multiplier` (timing.cpp 17). Same constant, two different questions (SP share versus the disc's number).
- `score_without_solo` (stars.cpp 35-37) is the one "total minus solo"; `Path::avg_mult` and the Preview call it. grep found no second subtraction.
- `sqout_sp()` is the one place the lost doubling is subtracted (scoring.h 28); graph.cpp and replay.cpp read it.
