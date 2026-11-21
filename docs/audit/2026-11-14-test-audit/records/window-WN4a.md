# Window round WN4a (agent a55ee54e17828beea)

Task T5-WN4a, session e633f7f6-f313-4787-b622-8d901b577b64. Code read at the worktree HEAD (`5ec682c7`'s code). Nothing was run. Every claim is "by reading" unless it names a run. This redoes the six files of `records/window-WN4.md` that the re-send page gives WN4a: `test_replay.cpp`, `test_replay_schedule.cpp`, `test_replay_windows.cpp`, `test_pathstring_read.cpp`, `test_preview_scene.cpp`, `test_preview_controller.cpp`. The WN4 file and its verifier (`verify/window-WN4.md`) were read first.

## The window, read in code

- The search's default is 60: `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`), read through `SearchSettings::early_fill_window_ms` (`src/search/pather.h:45`). `ScoreGraph`'s last constructor argument defaults to the same constant (`src/search/graph.h:198-201`; the graph does not read it, `graph.h:195-197`), so `ScoreGraph(song, 4, FillDeadlineRule::Ch10)` with `run_search` in `test_replay.cpp:890` and `test_preview_scene.cpp:293` is also a 60 ms search.
- The rule: `fill_refuses` and `is_e0` take the window (`src/core/model.h:331,335`); the engine reads them at `src/search/engine.cpp:859,862,1007`.
- Every setting in these six files reaches 60. `app::Settings().to_analysis_settings()` is the default; `test::scores_settings(cap)` starts from it (`tests/record_fixtures.h:582-589`); a bare `app::AnalysisSettings` (used by `tests/analyzed_chart.h:19` and `analysis_settings()` in `test_preview_controller.cpp:139`) takes the field's default, 60. No test in the six files sets `early_fill_window_ms`, so there is no A row.
- The filed rulings on the window, `docs/agents/rulings/code-and-engine.md` lines 55 to 57 under `## Confirmed`: "so we should use the hit window. and stop inventing our own numbers." / "make the early fill threshold an adjustable setting pinned to one number in both directions" / "analysis settings panel, default 60ms, supports 0ms to 85ms". Read in the ruling file.
- The engine dumps in `records-engine/` are at 85 ms (`326cd4e0`). No dump at 60 ms exists. Every question 3 below therefore says "claim not checked" for the 60 ms case and names the dump the main session must take: `hydra_replay dump` and `hydra_replay schedule` at `--efill 60` for the named chart.

## Where this file differs from the WN4 file

- Every test body in the six files was read line by line, except the ranges named under "Not read line by line".
- The WN4 verifier's disputes inside these files all hold, and two more rows move from B to C by their bodies: `test_replay.cpp:871` (it types 228710 and "1 2" from a corpus run; the old record `A24.md` heading 3 already faulted those lines) and `test_replay.cpp:888` (types `{19200}` from a search). Two C rows of the WN4 file are B here, each with its reason in the table: `test_replay.cpp:340` and the four `test_pathstring_read.cpp` C rows (90, 99, 148, 212). The other WN4 C rows in these files stay C and gain a full record. Two C rows that the WN4 verifier found only by a typed-number scan (preview controller 243, 869) are C here because the assertion reads search output (a best path with an activation), not a typed number.
- `test_preview_controller.cpp` has a row that WN4 called delta round (1). One case, `test_preview_controller.cpp:361`, is in the delta round (`records/delta-DL5.md`, record 20); it is not judged again.


## The tables

## tests/test_replay.cpp: A 0, B 42, C 6, delta round 0 (48 test cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 104 | replay reproduces the engine's score for every corpus path | C | record 1 |
| 150 | replay: combo_after is the combo once the chord is hit | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 179 | replay: a solo's bonus reaches the on-screen total on the section's last chord | B | chart text parsed and replayed with no windows (replay_path(song, {})); no search |
| 209 | targeted search reproduces every corpus path | C | record 2 |
| 311 | every tied variant stores what a search pricing it alone stores | C | record 3 |
| 340 | targeted search rejects a tick that is not a fill | B | search_target runs at 60, but the case asserts the result is empty for a real activation tick + 1; engine.cpp:574-581 finds a target by exact equality with a fill tick, which no window enters; the chart picked moves nothing asserted |
| 360 | replay without Star Power scores no doubling at all | B | first .mid chart parsed and replayed with no windows; no search |
| 386 | a squeezed-out chord past the leeway earns nothing | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 407 | paid_by_sp: yes exactly when the row's SP value is above zero | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 418 | replay: a squeezed-out chord SP pays nothing shows the plain multiplier | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 462 | replay: the squeeze-out warning ignores a chord only a zero-paying window reached | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 493 | shown_multiplier doubles the combo multiplier only under Star Power | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 508 | a path JSON names the squeezed-out chord by its tick, the offset only for older dumps | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 550 | windows read from a path JSON match the ones read from the record | B | both sides read from the same stored path (windows_for_path vs windows_from_json(paths_json)); `checked > 0` is a floor (see weak floors) |
| 611 | a dump carries each window's SqIn phrases back to the replay | C | record 4 |
| 649 | paths_json writes every field the dump readers use | B | asserts key names, and that the end-ratio fields are numbers, on the first chart with an activation; no value is typed |
| 736 | paths_json: a constant tempo gives end ratios of 1 and no change on any side | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 758 | paths_json: a tempo marker on the activation tick gives the earlier segment's ratio | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 779 | paths_json: a change on the tick bounds neither of its steady spans | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 798 | paths_json: a time signature change between the ticks gives the measure ratio | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 820 | paths_json: a clamped end is anchored on its collecting note | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 835 | paths_json writes the end ratio the search stored | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 854 | paths_json: an activation with no deact tick writes every end field null | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 871 | result block: an empty record writes a null score and an empty bestpath | C | record 5 |
| 888 | paths_json prints the stored passed-over fills, not a guess | C | record 6 |
| 917 | a window ending on a phrase note with no offset is flagged | B | first .mid chart, windows built from the replay rows themselves; no search |
| 994 | per-note sp points sum to the chord's sp points | B | first .mid chart, one window over the whole chart; no search |
| 1059 | fixtures: mark_phrase_end sets the flag and the phrase start together | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1072 | a typed squeeze-out offset resolves to the phrase chord | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1110 | a typed squeeze-out on a chord the engine never squeezes out is refused | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1139 | a typed squeeze-out offset halfway between two phrase chords is refused | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1165 | a typed squeeze-out on a phrase banked before the activation is refused | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1200 | a typed squeeze-out on the phrase after a banked one is the engine's | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1225 | the squeeze-out warning names the chord the graph would squeeze | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1272 | the squeeze-out warning quotes the whole drop under sqout_rule | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1316 | squeeze_window_phrases: the phrase chords strictly inside the window, in chart order | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1350 | offered_phrase: the newest step's phrase early, the first unsqueezed one late | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1375 | sqout_chords: a window's newest phrase at or before D, or the first after D | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1395 | category_scores reports the multiplier each note was paid at | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1420 | category_scores prices a dynamic cymbal's bonus at its own note | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1445 | replay reports the multipliers category_scores applied | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1477 | replay copies each note's dynamics bonus | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1489 | replay names each note's dynamic | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1517 | chord_score_table: literal rows on a hand-built chart | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1595 | replay multipliers agree with the combo on every corpus chord | B | corpus charts parsed and replayed with no windows; no search |
| 1624 | replay score fields: one list in schema order | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1635 | assign_score and score_of are each other's reverse | B | hand-built chart, hand-typed JSON or a plain function call: no search runs, so the early-fill rule is never asked |
| 1649 | replay: scores_only leaves chord_code and notes empty and every score the same | B | full replay against scores-only replay of the same stored path; both sides from one search result; `runs > 0` is a floor (see weak floors) |

## tests/test_replay_schedule.cpp: A 0, B 38, C 2, delta round 0 (40 test cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 106 | schedule: every end step names its anchor | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 153 | schedule: a squeeze carries its own end, anchor and stored ratio | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 208 | schedule: paid_by matches in_sp on every corpus path | B | stored path against its own replay (paid_by vs in_sp); `rows > 0` is a floor (weak floor) |
| 241 | schedule: declined chords are the skipped fill ticks | B | declined rows against the same path's skipped_fill_ticks, one result on both sides; `declined_rows > 0` is a floor (weak floor) |
| 267 | schedule: the window constants are written | B | constants written for a hand-built song; window_back_ms 85.0 is the Clone Hero hit window, not the early-fill window |
| 284 | schedule: sides come from the whole chart | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 317 | schedule: a missing --path refuses by name | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 325 | schedule: a chart with no path refuses by name | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 337 | schedule: nothing under src/ reads the schedule | B | reads source text; the 85.0 it looks for is kHitWindowMs (model.h:106), not the early-fill default |
| 419 | schedule: the equal-room split on Freak's clamped squeeze | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 473 | schedule: Freak 1144, Rescued 561 and 931, Ghost 756 | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 501 | schedule: a squeeze that needs no timing is pressed plain | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 526 | schedule: a squeeze the game cannot make is not feasible | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 548 | schedule: the fallback block | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 607 | schedule: last_send_ms is the side less one frame | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 620 | schedule: an activation that needs both directions is refused naming both chords | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 636 | schedule: a lead across a steady span is refused naming the span | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 653 | schedule: a press that reaches a neighbour on its pad is refused naming both chords | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 679 | schedule: a late gem that pays less than an inside gem is refused | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 697 | schedule: press groups | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 735 | schedule: the planned end counts one frame | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 773 | schedule: the equal-room split of a squeeze-in is the mirror of a squeeze-out's | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 793 | schedule: moving_side reads the anchor's press, not the squeeze's kind | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 822 | schedule: song_length_ms is app::song_length_ms | B | schedule song_length against app::song_length_ms for the same chart; parse and metadata only; `with_length > 0` a floor |
| 851 | schedule: sp_ready_tick is the second bank arrival | B | bank_rise_ticks read two ways off one stored activation; `acts_seen > 0` a floor (weak floor) |
| 905 | schedule: the late press holds sqout_late_notes | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 939 | schedule: a whole_chord chord with no inside group has an empty inside list and an empty join_gems | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 964 | schedule: first_note writes the same schedule as before | B | corpus loop asserts a property of the schedule writer; `late_rows > 0` is a WARN, not a check |
| 1005 | schedule: hydra_replay_build is written | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 1062 | schedule: one frame everywhere | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 1098 | schedule: far press follows TM-50 | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 1130 | schedule: every skip writes a refusal and a flag | B | property per corpus row (refused iff no press); `rows > 0` a floor |
| 1272 | schedule: a squeeze-out that is also a skipped fill | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 1345 | schedule: one decline rule | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 1378 | schedule: the first press group is the activation note | B | first press of an activation chord holds the activation note alone: a property of one stored path; `seen > 0` a floor |
| 1514 | schedule: Pitch Dark prices each read at its own step's scale | C | record 7 |
| 1621 | schedule: Pitch Dark's read range is its read end's steady span | C | record 8 |
| 1651 | schedule: a lead that moves the read end past a tempo change is refused naming its span | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 1680 | schedule: a tempo change on an end splits the conversion's sides | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |
| 1716 | schedule: read_at_own_ms runs the conversion backwards on its own side | B | hand-built Activation or geometry typed into the schedule functions (plain_song, overfill, sqout(...)); no search runs |

## tests/test_replay_windows.cpp: A 0, B 7, C 0, delta round 0 (7 test cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 226 | replay: the open-window walk pins every row on hand-built windows | B | hand-built windows on the squeeze chart, the longest corpus chart with hand-made windows, or RapidCheck windows; no search runs |
| 344 | replay: disjoint windows that squeeze out a one-note chord on D | B | hand-built windows on the squeeze chart, the longest corpus chart with hand-made windows, or RapidCheck windows; no search runs |
| 382 | replay: window order does not matter and SP points add up window by window | B | hand-built windows on the squeeze chart, the longest corpus chart with hand-made windows, or RapidCheck windows; no search runs |
| 426 | replay: window order does not matter and windows add up, shrunk by RapidCheck | B | hand-built windows on the squeeze chart, the longest corpus chart with hand-made windows, or RapidCheck windows; no search runs |
| 494 | replay: on every corpus path the disc follows SP's payment and windows add up | B | disc against payment and window sums for every stored path, one result on both sides; `runs > charts` is a floor on a path count (weak floor) |
| 548 | replay_path with a table handed in equals replay_path building its own | B | table handed in against a table built inside, same song and same stored windows; equality of two replays of one input |
| 574 | replay_path refuses a table of the wrong size or detail | B | refusal of a wrong-size table; the corpus chart only supplies a song and windows |

## tests/test_pathstring_read.cpp: A 0, B 10, C 0, delta round 0 (10 test cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 82 | pathstring read: every corpus best path round-trips | B | round trip: text written from a path the search found at window W is read back by a graph built at the same W (paths_from_pathstring takes cfg), so the two sides cannot differ because of W |
| 90 | pathstring read: a token no fill can place is refused with its position | B | first chart whose best path has two windows gets a second window "99"; the refusal text names position 2 and "99", typed input; which chart is first moves nothing asserted |
| 99 | pathstring read: every refusal is ErrorKind::PathTextRefused | B | refusals of "   " and "99" on the first chart with notes; parse-level refusals, no activation placed |
| 115 | pathstring read: \"(No activations.)\" gives a path with no activations | B | "(No activations.)" reads to no activations; no search value asserted |
| 126 | pathstring read: every corpus path round-trips from Hydra's long Copy path text | B | round trip: text written from a path the search found at window W is read back by a graph built at the same W (paths_from_pathstring takes cfg), so the two sides cannot differ because of W |
| 135 | pathstring read: long text with no activations gives a path with no activations | B | an empty path's long text reads to no activations; the chart only supplies multsqueezes |
| 148 | pathstring read: a long-text window no fill can place is refused with its position | B | as 90 for the long text; typed "99" and position 2 are input |
| 175 | pathstring read: the graph entry point reads the same path as the settings one | B | settings entry against graph entry on one graph window; both read the same stored text; `paths > 0` floor |
| 203 | pathstring read: the graph entry point refuses a token no fill can place | B | as 90 through the graph entry point |
| 212 | pathstring read: long text with the wrong number of sections is refused | B | section-count refusal on the first chart with notes; no search value asserted |

## tests/test_preview_scene.cpp: A 0, B 30, C 3, delta round 0 (33 test cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 37 | build_preview_scene: notes carry lane and drum attributes | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 70 | color_of: each lane maps back to the colour it was drawn from | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 80 | build_preview_scene: the song length is the one handed in, never a note's time | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 96 | build_preview_scene: SP phrase, solo, and fill spans | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 115 | build_preview_scene: a fill whose chord sits before its measure line spans chord to line | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 128 | build_preview_scene: a generated fill draws the half measure before its chord | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 174 | build_preview_scene: solo spans come from the song's solo sections | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 189 | build_preview_scene: each solo's notes for the solo box, path-free | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 209 | build_preview_scene: drum lanes come from the song's drum lanes | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 227 | build_preview_scene: an unanalyzed chart offers every candidate fill | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 234 | build_preview_scene: skips say which fills the path was offered | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 249 | build_preview_scene: a second activation with skips 0 hides what lies between | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 271 | build_preview_scene: a fill an activation took stays taken when a later one lists it | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 289 | build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged | C | record 9 |
| 306 | build_beat_events: bars, beats, and half-beats from the timing alone | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 330 | build_beat_events: a 3/4 section changes the beat count per bar | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 351 | build_beat_events: 6/8 steps by the eighth note | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 373 | build_beat_events: the fainter line is halfway back to the line before, at a restart too | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 389 | build_preview_scene fills beats, tempos and resolution | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 411 | build_preview_scene: the beat lines stop at the song's length (D266) | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 448 | build_preview_scene: no path means no overlay | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 454 | parser keeps the SP-phrase start on the flagged note | B | parser flags only (load_songpath); no search |
| 475 | build_preview_scene: an analyzed chart's overlay matches its path | B | analyzed chart's overlay checked against its own best path as properties (counts, window_end_tick, lane, measure); both sides the same path; no value typed |
| 690 | base + overlay: equals build_preview_scene on the hand-built fixtures | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 744 | base + overlay: equals build_preview_scene on every corpus chart and stored path | B | base plus overlay equals the whole scene for every stored path; both built from one path; `paths > charts` is a floor (weak floor) |
| 778 | path_overlay_key: no overlay, the same path, and a changed path | B | path_overlay_key of one stored path against copies of itself |
| 805 | build_preview_scene: passed-over fills on several activations, and a tick that is no fill | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 821 | build_preview_scene: each activation names the fill it took | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 863 | build_preview_scene: a ghost path lists its own taken fills and nothing else | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |
| 898 | build_preview_scene: a corpus best path as the ghost lists its taken fills | C | record 10 |
| 925 | preview scene: fills after the last activation read Offered when the engine lists them, Hidden other | C | record 11 |
| 967 | preview fills: every stored passed-over fill is a fill in the scene | B | property: every stored passed-over tick is a fill in the scene; `ticks_checked > 0` is a floor (weak floor) |
| 994 | preview path label: the notation and which list it came from | B | hand-built song (make_hand_song, make_fill_song, make_overfill_song, timing maps) and hand-built Path objects; no search runs |

## tests/test_preview_controller.cpp: A 0, B 23, C 4, delta round 1 (28 test cases)

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 171 | the Preview load builds the highway timeline on its worker | B | first chart with a path is analyzed only to give the load job a best path; the case compares the worker's timeline with the renderer's for one scene |
| 191 | the Preview load stops the beat lines at the song's length, not the audio's end | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 243 | the Preview overlay job builds the highway timeline on its worker | C | record 12 |
| 291 | the Preview base job builds the path-free scene and timeline | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 338 | track_options: one builder, compared whole | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 348 | a cancelled Preview load stops before decoding | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 361 | a cancelled click job stops before counting | delta round | records/delta-DL5.md record 20 |
| 379 | with no audio device the Preview still loads, muted, with a warning | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 403 | with an audio device that won't start the Preview still loads, muted, with a warning | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 434 | preview_config: asked before the first render it fails loudly | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 441 | a Preview asset failure reads Reinstall Hydra, with the raw text as its detail | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 475 | closing the Preview frees its render targets; reopening draws again (WARP) | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 502 | scrub marks: the Preview's scrubber ends at the song's length | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 518 | scrub marks: a stated length past the audio ends playback and the scrubber there | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 535 | the Preview's playback, time box and scrubber all end at the song's length | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 548 | switching paths builds the new overlay off the UI thread | B | asserts overlay keys for the best path of the first chart with paths; keys from one path on both sides |
| 575 | shows_path: the same path at another cap, and a different path | B | key comparisons for one best path at SP cap 4 and 1 and a key with its last character changed; needs only one path from analyze_with_paths |
| 614 | set_ghost_path: the ghost's fills land on the scene, and clearing it empties them | C | record 13 |
| 672 | set_ghost_path: a changed chart draws no ghost, as it draws no path | C | record 14 |
| 729 | a mode change on the open chart reloads only the notes and keeps the playhead | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 791 | a mode with no notes pauses the Preview in place; switching back clears it | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 826 | a second mode change during a notes reload wins | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 869 | the Preview hides the path when the chart file changed since its record | C | record 15 |
| 895 | the Preview trusts the scan's fingerprint and does not re-hash an unchanged chart | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 938 | busy covers the overlay and base jobs, not only the first load | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 955 | the Preview volume is the settings owner's: default and clamp | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 964 | the Preview's song key: another difficulty, Pro Drums or 2x Bass makes a different song | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |
| 992 | the Preview's song key: the 1.0 fills setting makes a different song | B | no search value asserted: hand-written chart or a corpus chart with no path, or controller plumbing (jobs, audio device, cancel, keys, volume) |

## C records

Each record starts with the delta-round line. Question 3 names the chart and the dump the main session must take at 60 ms (`hydra_replay dump` and `hydra_replay schedule` with `--efill 60`); until then each is "claim not checked" at 60. Question 8 names the bug as a change in a named function; all are "reasoned" (read, not planted). "Old" is the earlier verdict, read in the old records file.

### 1. replay reproduces the engine's score for every corpus path (tests/test_replay.cpp:104)

Delta round: window, replaces `records/A23.md` record 1.
Read: lines 55-146 (`check_every_corpus_path`, the three `Case` rows), `replay_stored_path`, `digest::loading_chart_count`; old record 1 (old verdict Rewrite: drop `min_paths`). Adding commit: not read.
1 Claim: every stored path of every corpus chart, under the defaults, under the CH 1.0 fill rule and on Hard, replays to the score the engine stored.
2 Owner: the engine stores `Path::totalscore`; `replay_stored_path` (`src/core/replay.h`) prices it again. The test is a two-owner agreement; both are engine/core, no display.
3 Real chart: Engine: every corpus path at 60 ms, none dumped. Claim: replay equals stored. Claim not checked at 60 (the 85 ms dumps cannot show it; the property is rule-free, so it should hold, but the search now returns different paths).
4 Direct: 131 `all_windows()` and 144 `mismatches == 0` are properties. 142 `run.paths >= c.min_paths` with `382, 382, 294` (lines 119-121) is a count typed from a run.
5 Reason: the file's own comment (106-108) says `min_paths` is "a floor pinned from one run". No rule gives 382 or 294. A different window changes the number of stored paths.
6 Display: not a display.
7 Failure: the old record names it (a replay that disagrees with the graph's pricing); not reopened.
8 Deleted: 144 (replay drops a doubling, `replay_stored_path` skips a window; reasoned). 142 (a change that drops paths; reasoned, but it also fails when a rule change adds or removes paths without any bug, which is the 60 ms case). 141 (the loader skipping a chart; reasoned).
9 Stand-in: 142 is a floor standing for "the loop walked everything". 141 already checks that, so 142 stands for nothing the other lines miss.
10 Same rule: `test_replay.cpp:209` (record 2) and `test_replay_windows.cpp:494` walk the same stored paths.
11 Real: yes, corpus charts.
Verdict: Rewrite (same as old). Drop `min_paths` (lines 112, 119-121, 142); keep 141 and require `paths > 0` per setting.
Typed numbers that a 60 ms run can move: 119, 120, 121 (`382`, `382`, `294`), asserted at 142.
Finding: none against `src/`.

### 2. targeted search reproduces every corpus path (tests/test_replay.cpp:209)

Delta round: window, replaces `records/A23.md` record 4.
Read: lines 203-254; `search_target` (`src/search/pather.cpp:280` sets `target_act_ticks`); old record 4 (old verdict Rewrite: remove the 300 floor).
1 Claim: handing a path's activation ticks back to `search_target` returns that same path, with the same score, deactivation nodes and squeezes.
2 Owner: the engine (`search_target`, the engine's `target_act_ticks` lookup at `engine.cpp:574-592`). No display.
3 Real chart: every corpus path at 60 ms; not dumped. Claim not checked at 60. Both sides run under the same window (`cfg` is passed to both the first search and `search_target`), so a window change moves both together; the test should still hold.
4 Direct: 253 `mismatches == 0` is a property. 251 `REQUIRE(run.paths >= 300)` is a typed floor.
5 Reason: 300 has no source; it is a count typed from a run on the corpus, so it moves with the corpus and with the window.
6 Display: no.
7 Failure: old record 4; not reopened.
8 Deleted: 253 (search_target prices a chosen path differently from the full search, e.g. `follows_pins` at `engine.cpp:592` stops following pins one fill early; reasoned). 251 (the loop silently walking fewer charts; reasoned, but a corpus change also trips it).
9 Stand-in: 251 stands for "the loop ran".
10 Same rule: record 3 (the tied-variant case) and `test_pathstring_read.cpp` round trips.
11 Real: yes.
Verdict: Rewrite (same as old). Replace 251 by `run.paths > 0` and `run.charts == loading_chart_count`.
Typed numbers that a 60 ms run can move: 251 (`300`).
Finding: none.

### 3. every tied variant stores what a search pricing it alone stores (tests/test_replay.cpp:311)

Delta round: window, replaces `records/A23.md` record 5.
Read: lines 256-336; `test::lone_pricing` (`tests/record_fixtures.h`, not opened beyond its use); old record 5 (old verdict Rewrite).
1 Claim: a tied variant (decision D3) stores its own facts, the ones a search pricing it alone stores.
2 Owner: the engine's tie folding. No display.
3 Real chart: tie variants over the whole corpus at cap 4/depth 4, cap 4/depth 40 and cap 2/depth 40, at 60 ms; not dumped. The file's comment (309-310, 316-317) says today these list 12, 295 and 303 variants: those counts are from the 85 ms runs and a 60 ms search lists a different number. Claim not checked at 60.
4 Direct: 330 `differing == 0` is a property. 328 `variants >= {10, 200, 200}` is a typed floor. 331 `skipped() == {0, 0, 4}` pins an exact count of variants with no lone root.
5 Reason: the comment (312-319) explains the intent (a regression that stops listing ties). No rule gives 10, 200, 200, 0, 0 or 4; they are counts from a run, at 85 ms. The skip count 4 at cap 2 is the number of variants the lone search folds under a root of its own; a window change changes which variants exist.
6 Display: no.
7 Failure: D3 and the fixtures in `test_tied_variants.cpp`; not reopened.
8 Deleted: 330 (a variant takes its leader's facts; reasoned). 331 (a compared variant turning skipped; reasoned, but at 60 it fails with no bug, which makes it a snapshot). 328 (the engine listing no ties; reasoned).
9 Stand-in: 328 and 331 stand for "the same variants were checked".
10 Same rule: `test_tied_variants.cpp` (WN3's file) holds the purpose-built fixtures for D3.
11 Real: yes.
Verdict: Rewrite (same as old). Print each skipped variant with its reason and require `compared > 0`; drop the three typed floors and the typed skip counts.
Typed numbers that a 60 ms run can move: 321 and 322 (`Setting{4, 4, 0, 10}`, `{4, 40, 0, 200}`, `{2, 40, 4, 200}`), read at 328 and 331; the "12, 295 and 303" in the comment at 316-317.
Finding: none.

### 4. a dump carries each window's SqIn phrases back to the replay (tests/test_replay.cpp:611)

Delta round: window, replaces `records/A23.md` record 15.
Read: lines 604-645; `windows_from_json`; `ambiguous_window_warnings`; old record 15 (old verdict Investigate, claim not checked only).
1 Claim: a dump keeps each window's squeezed-in phrase ticks, so `score --path` does not warn that a phrase might have been squeezed out when it was squeezed in; an old dump with no key reads as none.
2 Owner: `paths_json` writes, `windows_from_json` reads, `ambiguous_window_warnings` skips (`src/core/replay.cpp:413`, CLI only). Not a display.
3 Real chart: Engine: the old record read Pitch Dark path 0, activation 0, `sqin_ticks [128520]` at 85. The test's own chart is `test_fast_tempo/spent_then_next.chart` at cap 2; at 60 ms nothing was dumped. Claim not checked at 60 ms.
4 Direct: 626 `sqin_ticks` equal is direct. 635 no warning contains "17360" is direct once "0++" exists. 637 `with_sqins > 0` and 638 `saw_0pp` ask the search to find at least one window with a squeezed-in phrase and a path whose text is exactly "0++" on this chart at cap 2.
5 Reason: "17360" and "0++" come from the chart's own search at 85 ms (comment 606-610). Under 60 ms the path "0++" may not exist: if it does not, `CHECK(saw_0pp)` fails and 635 never runs.
6 Display: no.
7 Failure: D34; the CLI warned about a phrase already squeezed in. Still possible.
8 Deleted: 626 (dump drops `sqin_ticks`; reasoned). 635 (the warning stops skipping a squeezed-in phrase, `ambiguous_window_warnings`; reasoned). 638 (the test passing with no "0++" path, so 635 never running; reasoned). 644 (the reader rejecting an older dump; reasoned).
9 Stand-in: 637 and 638 stand for "the case reached the phrase".
10 Same rule: old records 14 and 11 (same file).
11 Real: yes, the chart is in the corpus.
Verdict: Investigate: claim not checked only (as old). Needs the dump of `spent_then_next.chart` at cap 2 at 60 ms, to see whether "0++" is still a path and still has a squeezed-in phrase.
Typed numbers that a 60 ms run can move: 629 (`"0++"`) and 635 (`"17360"`), asserted at 638 and 635 respectively.
Finding: none.

### 5. result block: an empty record writes a null score and an empty bestpath (tests/test_replay.cpp:871)

Delta round: window, replaces `records/A24.md` record 3.
Read: lines 868-883; `result_json`; old record 3 (old verdict Rewrite: keep 873-874, replace 881-882 with a property).
1 Claim: the result block writes null and "" for an empty record; on Allister - Overrated it writes the best score and best path text.
2 Owner: `result_json` writes the dump's block; the engine owns `optimal_score` and `best_path_text`. No display.
3 Real chart: Engine: Allister - Overrated best score and best path at 60 ms; not dumped. Claim: 228710 and "1 2" (lines 881-882). The same 228710 is typed in `test_pb_text.cpp` ("Best finish here=228,710", WN4b's file), so the claim is typed twice. Not checked at 60.
4 Direct: 873-874 direct. 881 and 882 are two numbers typed from a run.
5 Reason: none. They are the engine's result on that chart at the time. At 60 ms the best path on this chart may differ: the WN4 verifier read `test_pb_text.cpp:118` typing this chart's offered fill ticks.
6 Display: no.
7 Failure: audit finding 98 (comment 868-870): a no-path record printed a real-looking 0. The null half is still checked.
8 Deleted: 874 (empty `bestpath` text; reasoned). 873 (a null score for a no-path record; reasoned). 881/882 are the typed snapshot (any change in the best path or score trips them, which is the 60 ms case).
9 Stand-in: none.
10 Same rule: `test_pb_text.cpp:298` and 313 type the same chart's numbers.
11 Real: yes.
Verdict: Rewrite (same as old). Keep 873-874; replace 881-882 with: the block's score equals the record's `optimal_score` and its bestpath equals `best_path_text`, no number typed.
Typed numbers that a 60 ms run can move: 881 (`228710`) and 882 (`"1 2"`).
Finding: none.

### 6. paths_json prints the stored passed-over fills, not a guess (tests/test_replay.cpp:888)

Delta round: window, replaces `records/A24.md` record 4.
Read: lines 885-911; `make_ch10_fill_song` (`tests/record_fixtures.h:108-121`); `ScoreGraph` constructor (`src/search/graph.h:198-201`); old record 4 (old verdict Investigate, claim not checked only).
1 Claim: on the 1.0 fill rule's song, the stored passed-over fill (19200) is not the fill nearest the activation (24960), and the dump prints the stored one.
2 Owner: the engine stores `skipped_fill_ticks`; `paths_json` writes it. No display.
3 Real chart: none; the chart is hand-built. Engine: no real record. Claim not checked. The search runs at the graph's default, 60 ms (graph.h:201), with a target at tick 28800.
4 Direct: 904-906 compare the dump with the walk, a property. 909-910 types `{19200}` from the search.
5 Reason: comment 886-887 gives the reason (the stored fill is not the nearest). Whether the search stores 19200 depends on `fill_refuses` and `is_e0` for the fills at 19200 and 24960 (480 and 3456 ticks long); the window changes which of them the search can pass over.
6 Display: no.
7 Failure: the dump printing a guessed fill; still possible.
8 Deleted: 904 (the dump printing the nearest fill instead of the stored one; reasoned). 910 (same bug, direct).
9 Stand-in: none.
10 Same rule: record 9 (`test_preview_scene.cpp:289`) runs the same fixture and the same target.
11 Real: the fixture stands for a CH 1.0 chart with two fills; no corpus chart is named.
Verdict: Investigate: claim not checked only (as old). Needs a 1.0-rule dump at 60 ms of a real chart where the stored fill is not the nearest.
Typed numbers that a 60 ms run can move: 909-910 (`19200`); 892 (`28800`) is an input.
Finding: none.

### 7. schedule: Pitch Dark prices each read at its own step's scale (tests/test_replay_schedule.cpp:1514)

Delta round: window, replaces `records/B4.md` record "schedule: Pitch Dark prices each read at its own step's scale".
Read: lines 1427-1619, `pitch_dark()` (1448-1461), `three_step_overfill`; old record (old verdict Rewrite: keep 1558-1559, 1601-1602, 1613-1618; drop every pinned figure); `verify/window-WN4.md` dispute 1.
1 Claim: on Pitch Dark, the end the player reads after an anchor chord is priced at that end step's own scale, and every timed press's own end converts from the read through the ratio between the two steps.
2 Owner: `schedule::schedule_json` over `step_transfer_scale` (`src/core/squeeze_rating.h`) and `end_conversion`. Callers: the schedule tool only. No display.
3 Real chart: Pitch Dark (`testdata/input/test_pitch_dark_ratio`), path 0, at 60 ms; not dumped. The case's comment at 1519-1520 says "path 0 is the path the run played" and 1521-1532 require 3 activations, 4 steps for activation 1 and the ticks 178560, 182280, 198570, 194080, 202440, 202480, 206280. These come from the 85 ms search. The old record read `pitch_dark_ratio.schedule.json` (85). Claim not checked at 60; the first `REQUIRE`s may fail outright at 60, and every later line then does not run.
4 Direct: 1556-1559 and 1601-1602 are properties against the scale function. The rest are typed engine values: 1542 `158262.53`; 1543 `158254.77` (a made-up wrong value); 1561 `154250.379`; 1565 `24.124`; 1567 `64.571`; 1574 `-18.316`; 1575 `161386.46`; 1576-1577 `0.79467`; 1579 `161356.443`; 1580 `158224.757`; 1598 `164341.3`; 1608 `-40.919`; 1609 `kPitchDark243OwnEndMs = 100949.4546` (line 1437, comment "pinned from one run").
5 Reason: the file says the literals are "the plan's, read from Pitch Dark's chart and the run of 2026-11-12" (1429-1431). A live run, at 85 ms. No rule gives them.
6 Display: the schedule is a tool output, not a screen.
7 Failure: T2 of the schedule plan (removed); a read priced at the squeeze's ratio. Still possible.
8 Deleted: 1541-1542 (read priced at the step-1 scale: `schedule_json` using the squeeze's ratio for the read; reasoned). 1556-1559 (read after an activation chord priced at its own step 0). 1586-1588 (fallback threshold in read terms). 1601-1602 (conversion times read ratio equals the end ratio). The typed figures (1565, 1567, 1574-1580, 1598, 1608-1611) are snapshots: any change in the path or the ticks trips them.
9 Stand-in: 1521-1532 `REQUIRE`s stand for "the path is the one the live run played".
10 Same rule: the hand-built `three_step_overfill` case at 1680 states the conversion rule without a chart.
11 Real: yes, Pitch Dark.
Verdict: Rewrite (same as old). Express every planned end and aim from the stored step scales the way 1558 does, drop every pinned figure and the two made-up wrong values (1543, 1561), and take the path from a property (the first path with a timed squeeze-in) instead of `paths.front()` plus 3 and 4 and seven ticks.
Typed numbers that a 60 ms run can move: 1521 (`3`), 1523 (`4`), 1524-1532 (`178560`, `182280`, `198570`, `194080`, `202440`, `202480`, `206280`), 1542, 1543, 1561, 1565, 1567, 1574, 1575, 1576, 1577, 1579, 1580, 1598, 1608, 1437 (`100949.4546`) read at 1609. The chord indexes 466, 437, 144, 166, 644, 676, 243 are chart facts under path 0; if path 0 changes they name other chords.
Finding: none against `src/`.

### 8. schedule: Pitch Dark's read range is its read end's steady span (tests/test_replay_schedule.cpp:1621)

Delta round: window, replaces `records/B4.md` record "schedule: Pitch Dark's read range is its read end's steady span".
Read: lines 1621-1649; `pitch_dark()`; old record (old verdict Rewrite: state the range from the owner, no ms typed).
1 Claim: chord 644's read range on Pitch Dark is its read end's steady span, between the tempo changes at ticks 197760 and 198720.
2 Owner: `schedule_json`; `steady_spans` for the span. No display.
3 Real chart: Pitch Dark path 0 at 60 ms; not dumped. The case reads `presses_of(sch, 644, "squeeze_in")` and requires exactly one: with another path 0 there may be none (the `REQUIRE(p644.size() == 1)` at 1630 fails).
4 Direct: 1644-1647 property per press. 1638-1639 type `157545.2` and `158350.6` within 0.1 ms.
5 Reason: 0.1 comes from the plan's T2b acceptance; the two ms values come from the live run at 85.
6 Display: no.
7 Failure: the read range written wrong; still possible.
8 Deleted: 1638-1639 (the range moving off the step's span; reasoned). 1644-1647 (a press with an end carrying no range; reasoned).
9 Stand-in: none.
10 Same rule: record 7 (same fixture).
11 Real: yes.
Verdict: Rewrite (same as old). Compare the range to the read end's stored ms less `before_end_ms` and plus `after_end_ms` from the owner, item by item, no ms typed; pick the press by property.
Typed numbers that a 60 ms run can move: 1638 (`157545.2`), 1639 (`158350.6`), 1629 (chord 644, `"squeeze_in"`), 1626 (ticks 198570, 197760, 198720 in the comment).
Finding: none.

### 9. build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged (tests/test_preview_scene.cpp:289)

Delta round: window, replaces `records/A20.md` record 14.
Read: lines 289-304; `make_ch10_fill_song`; old record 14 (old verdict Investigate, claim not checked only).
1 Claim: under the CH 1.0 fill rule, fill A (19200) is shown and passed over because fill B's 1.0 deadline falls before A's; the scene lights A Offered, B Hidden and the activation's fill Taken.
2 Owner: the engine stamps `skipped_fill_ticks`; `build_preview_scene` reads it. The display reads the engine's list.
3 Real chart: none (hand-built). Claim not checked. At 60 ms the search (graph default window, target 28800) may list a different skipped set.
4 Direct: 300-303 type three fill states and `end_tick == 19200` from the search's output, in the display's words.
5 Reason: comment 290-291 gives "Finding 52"; not opened. The expected states come from one run on the fixture.
6 Display: the expected side is typed, not read from the record (question 6 fails).
7 Failure: finding 52, the Preview offering the wrong fill under 1.0. Still possible.
8 Deleted: 301 (the scene turning the passed-over fill Hidden; reasoned). 303 (the taken fill not Taken; reasoned).
9 Stand-in: none.
10 Same rule: record 6 (`test_replay.cpp:888`), same fixture and target.
11 Real: the fixture stands for a 1.0 chart; no corpus chart is named.
Verdict: Investigate: claim not checked only (as old). Needs a real 1.0-rule chart's dump at 60 ms; then rewrite to state, for each fill the record lists as skipped, Offered, and Taken for the activation's fill, no state typed.
Typed numbers that a 60 ms run can move: 299 (`3` fills), 300 (`19200`), 301-303 (the three states).
Finding: none.

### 10. build_preview_scene: a corpus best path as the ghost lists its taken fills (tests/test_preview_scene.cpp:898)

Delta round: window, replaces `records/A20.md` record 30.
Read: lines 896-919; `corpus::analyzed_by_ten_scores` (`tests/analyzed_chart.h:16-25`); old record 30 (old verdict Rewrite: no typed indices; name the chart).
1 Claim: the ghost path's list of taken fills, on the first chart with paths at depth 10, is fills 2 and 7, equal to the fills the same path takes when drawn.
2 Owner: the scene builder reads the path's activations. No display rule invented.
3 Real chart: the first chart in corpus order whose analysis at depth 10 and 10 ms yields a path; the test does not name it. At 60 ms nothing is dumped; the chart and its best path may both differ. Claim not checked.
4 Direct: 909 `ghost_fills == taken` and 917 `ghost_windows == windows` are properties. 903 `{2, 7}` is typed.
5 Reason: "pinned from one run" (comment 896-897).
6 Display: 903 types the display's expected indices; 909 compares with the display built another way, both from the same path, not from the record's ticks.
7 Failure: the ghost list drifting from the drawn path; still possible.
8 Deleted: 909 (ghost list built differently from the drawn one; reasoned). 903 (any change in the best path; a snapshot).
9 Stand-in: none.
10 Same rule: record 14 (`test_preview_controller.cpp:614`) types the same `{2, 7}`; two files, one typed value, both from 85.
11 Real: yes.
Verdict: Rewrite (same as old). For each activation of the ghost path, the fill whose chord is the activation's chord is in the list, resolved from the record's ticks; no indices typed; name the chart.
Typed numbers that a 60 ms run can move: 903 (`{2, 7}`).
Finding: none.

### 11. preview scene: fills after the last activation read Offered when the engine lists them, Hidden otherwise (tests/test_preview_scene.cpp:925)

Delta round: window, replaces `records/A20.md` record 31.
Read: lines 921-962; `test::scores_settings(4)`; old record 31 (old verdict Rewrite: state per fill by property, no typed ticks).
1 Claim: on Hail The Sun - Missed Injections (top 40 scores, cap 4), the path "3" activates once at 123838 and lists 244800 and 281759 as passed over; the fills after it read Offered when listed, Hidden otherwise.
2 Owner: the engine stores `trailing_skipped_fill_ticks`; the scene reads it. A display reading an engine list: right shape.
3 Real chart: Missed Injections at cap 4, depth 40, at 60 ms; not dumped. Claim: the path with text "3" exists with exactly those ticks. The comment's numbers come from the 85 ms search. Claim not checked at 60; 936-938 are `REQUIRE`s, so a different path list stops the case.
4 Direct: 942 and 961 are properties (the scene reads the list). 932 `"3"`, 937 `{123838}`, 938 `{244800, 281759}` and 953 `"157922:H 180960:H 244800:O 281759:O "` are typed.
5 Reason: comment 943 says "pinned from one run".
6 Display: partly. 942 compares with the record; 953 types the display's states.
7 Failure: the Preview not lighting fills the engine passed over after the last activation; still possible.
8 Deleted: 942 (scene drops the trailing list; reasoned). 961 (the same path without its list lights fills; reasoned). 953 (any change in the fills; a snapshot).
9 Stand-in: none.
10 Same rule: record 10 and the delta records for the Preview's fills.
11 Real: yes.
Verdict: Rewrite (same as old). Choose a path by the property "non-empty trailing list"; for every fill after the last activation, state is Offered iff its tick is in the list; no ticks or string typed.
Typed numbers that a 60 ms run can move: 932 (`"3"`), 937 (`123838`), 938 (`244800`, `281759`), 953 (`157922`, `180960`, `244800`, `281759` and the H/O letters).
Finding: none.

### 12. the Preview overlay job builds the highway timeline on its worker (tests/test_preview_controller.cpp:243)

Delta round: window, replaces `records/A21.md` record 3.
Read: lines 243-282; `analysis_settings()` (139-145), `first_chart_with_a_path`; old record 3 (old verdict Rewrite).
1 Claim: a scene job lays a path's overlay over a base and returns the timeline built from its own scene; the base is shared on the next job.
2 Owner: the job and `build_track_state`. No game rule.
3 Real chart: the first corpus chart with a path at depth 2, 10 ms, at 60 ms; the chart is not named. Claim not checked.
4 Direct: 263 `CHECK_FALSE(out.scene.activations.empty())` needs the first chart's best path to have an activation; that is a search output. 279 compares activation counts of two jobs from one path.
5 Reason: the test needs a path with an activation and gets it from the search at whatever window runs.
6 Display: no.
7 Failure: base rebuilt on every path change; still possible.
8 Deleted: 263 (overlay empty because the path is dropped; reasoned; but also red if the best path has no activation, which a window change can cause). 266-268 (base reuse; reasoned).
9 Stand-in: 263 stands for "the path's overlay is in".
10 Same rule: record 1 of the old file (the load job); same fixture.
11 Real: yes.
Verdict: Rewrite (same as old). Build the path with a hand-made activation instead of leaning on the search's best path, and compare activations item by item.
Typed numbers that a 60 ms run can move: none typed; 263 reads the search output.
Finding: none.

### 13. set_ghost_path: the ghost's fills land on the scene, and clearing it empties them (tests/test_preview_controller.cpp:614)

Delta round: window, replaces `records/A21.md` record 18.
Read: lines 610-668; `corpus::analyzed_by_ten_scores`; `first_chart_with_a_path`; old record 18 (old verdict Rewrite; "a snapshot of 2 and 7").
1 Claim: a ghost path on an open chart adds its taken fills to the scene beside the drawn path; clearing it brings the old overlay back.
2 Owner: the controller and the scene builder; no game rule.
3 Real chart: the first chart at depth 10 and the first chart at depth 2, both at 60 ms; not dumped. Line 620 `REQUIRE(path_identity(found.best) == path_identity(ghost))` assumes two different searches (depth 2 and depth 10) end on the same best path. Claim not checked at 60.
4 Direct: 631, 655, 666 empty-list checks are direct. 643 `{2, 7}` is typed from a run.
5 Reason: "The chart and its best path are the ones test_preview_scene.cpp pinned the ghost list on" (comment 612-613); the number is from 85.
6 Display: 643 types the display's list.
7 Failure: ghost lifecycle bugs; still possible.
8 Deleted: 643 (ghost list not built; reasoned). 620 (the two searches picking different paths; makes the case stop, not fail on a bug).
9 Stand-in: 620 stands for "the pinned chart is the one used".
10 Same rule: record 10 types the same `{2, 7}`.
11 Real: yes.
Verdict: Rewrite (same as old). Compare the scene's ghost fills with the fills the ghost path's own activations use, read from the ghost path; keep the lifecycle lines; drop 620 by using one chart and one path for both.
Typed numbers that a 60 ms run can move: 643 (`{2, 7}`).
Finding: none.

### 14. set_ghost_path: a changed chart draws no ghost, as it draws no path (tests/test_preview_controller.cpp:672)

Delta round: window, replaces `records/A21.md` record 19.
Read: lines 670-695; old record 19 (old verdict Keep).
1 Claim: when the chart file changed since its record, the Preview draws neither the path nor a ghost; when it did not, it draws both.
2 Owner: the controller (`drawn_path`); no game rule.
3 Real chart: the first chart with a path at depth 2, and the ghost from the first chart at depth 10, at 60 ms; not dumped. For `changed == false`, 692-693 require a non-empty ghost list and non-empty activations, which the search must supply. Claim not checked at 60.
4 Direct: 692 and 693 compare emptiness with `changed`, properties; they need the ghost path to take a fill and the drawn path to have an activation.
5 Reason: none typed. The needed facts are search output.
6 Display: no number.
7 Failure: finding 126; still possible.
8 Deleted: 692 (a changed chart still drawing the ghost; reasoned). 693 (same for the path; reasoned).
9 Stand-in: none.
10 Same rule: record 15 (`test_preview_controller.cpp:869`).
11 Real: yes.
Verdict: Investigate: claim not checked only. The old Keep rested on an 85 ms search; a Keep needs a 60 ms record showing the first chart's best path has an activation and the ghost takes at least one fill.
Typed numbers that a 60 ms run can move: none typed.
Finding: none.

### 15. the Preview hides the path when the chart file changed since its record (tests/test_preview_controller.cpp:869)

Delta round: window, replaces `records/A21.md` record 23.
Read: lines 862-889; `build_scrub_marks` (`src/app/preview_view.cpp:762-770`: one mark per scene activation); `chart_with_audio` (`tests/audio_chart_fixtures.h:43-48`, the first `.chart` in the corpus plus a sine); old record 23 (old verdict Investigate, claim not checked only).
1 Claim: a changed chart hides the path and the panel says so; an unchanged one shows it.
2 Owner: the controller and the scrub marks. No game rule.
3 Real chart: the corpus's first `.chart` at depth 2, 10 ms, at 60 ms; not named, not dumped. Claim not checked.
4 Direct: 886 `chart_changed() == changed` is direct. 887 `scrub_marks().empty() == changed` is true for `changed == false` only if the best path has an activation, because the marks are one per scene activation (`preview_view.cpp:767`).
5 Reason: the test needs that activation from the search.
6 Display: no number.
7 Failure: finding 126; still possible.
8 Deleted: 886 (changed chart not detected; reasoned). 887 (the changed chart still draws marks; reasoned).
9 Stand-in: none.
10 Same rule: record 14.
11 Real: yes.
Verdict: Investigate: claim not checked only (as old). It needs a capture of a changed chart in the Preview tab and a 60 ms record showing this chart's best path has an activation.
Typed numbers that a 60 ms run can move: none typed.
Finding: none.

## Weak floors and counts on search output (B rows, not disputed, listed so a 60 ms run can be watched)

- `test_replay.cpp:599` (`checked > 0`) and `1675` (`runs > 0`).
- `test_replay_schedule.cpp:237` (`rows > 0`), `264` (`declined_rows > 0`), `902` (`acts_seen > 0`), `1269` (`rows > 0`), `1424` (`seen > 0`): each counts rows from search output; none can reach zero unless a window removes every activation on the corpus, which I cannot show from the code.
- `test_replay_windows.cpp:520-521` (`charts > 0`, `runs > charts`), `test_preview_scene.cpp:775` (`paths > charts`), `991` (`ticks_checked > 0`), `test_pathstring_read.cpp:56, 197` (`paths > 0`).
- `test_pathstring_read.cpp:76, 168`: `FAIL("no corpus chart's best path has two windows")` and "exactly two windows": a window change that leaves no such chart fails the case.

## Every typed number that a 60 ms run can move (list)

`tests/test_replay.cpp`: 119, 120, 121 (`382`, `382`, `294`, read at 142); 251 (`300`); 321-322 (`{4, 4, 0, 10}`, `{4, 40, 0, 200}`, `{2, 40, 4, 200}`, read at 328 and 331); 316-317 (comment: "12, 295 and 303"); 629 and 635 (`"0++"`, `"17360"`); 881 (`228710`); 882 (`"1 2"`); 909-910 (`{19200}`).
`tests/test_replay_schedule.cpp`: 1521 (`3`), 1523 (`4`), 1524-1532 (`178560`, `182280`, `198570`, `194080`, `202440`, `202480`, `206280`), 1542 (`158262.53`), 1543 (`158254.77`), 1561 (`154250.379`), 1565 (`24.124`), 1567 (`64.571`), 1574 (`-18.316`), 1575 (`161386.46`), 1576-1577 (`0.79467`), 1579 (`161356.443`), 1580 (`158224.757`), 1598 (`164341.3`), 1608 (`-40.919`), 1437 (`100949.4546`, read at 1609), 1638-1639 (`157545.2`, `158350.6`).
`tests/test_preview_scene.cpp`: 299-303 (`3` fills, `19200`, three states), 903 (`{2, 7}`), 932 (`"3"`), 937 (`123838`), 938 (`244800`, `281759`), 953 (`"157922:H 180960:H 244800:O 281759:O "`).
`tests/test_preview_controller.cpp`: 643 (`{2, 7}`).
Not window-sensitive and left out: typed ms in hand-built geometry (`60545.26`, `53.59`, `-21.825` and the like in `test_replay_schedule.cpp:419-806`), which are inputs to `split_squeeze`; and `85.0` at `test_replay_schedule.cpp:281,355`, which is the Clone Hero hit window (`kHitWindowMs`), not the early-fill window.

## Splits

- `test_preview_scene.cpp:903` and `test_preview_controller.cpp:643` type the same ghost list `{2, 7}` for the same chart and path: one value in two files.
- `test_replay.cpp:882`/`881` (Allister - Overrated best score 228710, path "1 2") and the same score typed in `test_pb_text.cpp` (WN4b's file) type one chart's best result twice.
- `test_preview_controller.cpp:620` compares the best path of two different searches (`analyzed_by_ten_scores`, depth 10; `analysis_settings()`, depth 2) and stops the case if they differ: two settings answering one question.
- `test_replay_schedule.cpp:355-356` pins the text `kHitWindowMs = 85.0` in `src/core/model.h` as "Hydra's own two numbers" (plan rule 11), while the filed ruling at `code-and-engine.md` line 55 reads "so we should use the hit window. and stop inventing our own numbers." and the early-fill default is 60. I do not read which of these governs; this is two statements about 85 beside a 60 default, for the main session.

## Not read line by line

- `tests/test_replay_windows.cpp` lines 226-493 (the hand-built windows, RapidCheck and `longest_corpus_song` cases): read as comments and by a search for `analy`, `search`, `corpus`, `Settings`, `ScoreGraph`, `run_search`, `HydraRecord`; the only hits are the longest corpus chart's parse (line 203 and 153-ish comment) and comments. Case names: "replay: the open-window walk pins every row on hand-built windows" (226), "replay: disjoint windows that squeeze out a one-note chord on D" (344), "replay: window order does not matter and SP points add up window by window" (382), "replay: window order does not matter and windows add up, shrunk by RapidCheck" (426).
- `tests/test_preview_controller.cpp` lines 320-500 (track options, cancelled loads, audio device cases, asset failure, WARP) and 730-865 and 926-1002 (mode changes, busy, volume, song keys): by the same token search, which found no analysis, search or best-path token (the only `Settings` hits are `app::Settings` for the volume and the 1.0 fills key, and a `ViewJob` that is cancelled before it counts).
- `tests/test_pathstring_read.cpp`: all read. `tests/test_replay.cpp`: all read. `tests/test_replay_schedule.cpp`: all read. `tests/test_preview_scene.cpp`: all read.
- Not opened: `test::lone_pricing`, `windows_for_path`, `replay_json.cpp` internals, `docs/audit/2026-10-03-fix-decisions.md` D3, D34, D36, and the adding commit of every test (`git log -S`).
