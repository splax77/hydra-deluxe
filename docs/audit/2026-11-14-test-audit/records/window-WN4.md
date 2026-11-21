# Window round WN4 (agent afc4fab45f196993b)

Task T5-WN4, session e633f7f6-f313-4787-b622-8d901b577b64. Code read at the worktree HEAD (`5ec682c7`'s code).

## What I found about the window (read in code)

- The default is 60: `kDefaultEarlyFillWindowMs = 60.0` (`src/core/model.h:112`); `app::Settings::early_fill_window_ms` starts at it (`src/app/config.h:195`); `SearchSettings::early_fill_window_ms` starts at it (`src/search/pather.h:45`); a hand-built `Activation` carries `e_window_ms = kDefaultEarlyFillWindowMs` (`src/core/model.h:606`).
- Every default-built setting in this group reaches 60: `corpus::default_cfg()` (`tests/corpus_default_cfg.h:17`) is `app::Settings().to_analysis_settings()`; `scores_settings` (`tests/record_fixtures.h:582`) starts from it; `scratch_settings()` (`tests/scratch_settings.h`) changes only folders, depth 2 and auto_open_report, so `burnout()` (`tests/song_details_fixtures.h`), the GUI harness and the app fixtures (`tests/app_state_fixtures.h:174,231`, which pass the caller's settings to `analyze_chart_file`) are all 60 unless a test sets the field.
- Where the window is set in this group: `tests/song_digest.h:332` (`digest_settings`, 85) and `tests/test_activation_rows.cpp:1228` (`analyzed_like_the_gui`, 85). `test_cli.cpp:636-668` passes `--efill` text only to the argument parser. `test_report_rows.cpp:326` reads `hardest_efill`, a different thing.
- Only the search and the Activation E tests read the window: `fill_refuses` at `src/search/engine.cpp:862,1007`, `is_e0` at `engine.cpp:859` and `src/core/model.cpp:473,475`. Nothing under `src/` outside those and `src/app/path_view.cpp:84` (a comment) reads it.

## How far I read (honest limit)

The group holds 470 test cases. I stayed under the call and time caps by scanning every case's body for search entry points (`corpus::analyzed`, `analyzed_by_ten_scores`, `analyze_chart(_file)`, `search_target`, `search_graph`, `ScoreGraph`, `batch_run`, `open_and_wait`, `best_finish`, `burnout()`), then reading assertion lines for the cases that had one. A B row marked with a per-file reason was judged from the assertion lines and token scan, not a line-by-line read of the body. The verifier should treat every B row in a file whose reason says "token scan" as unproven. The C records below are short: each names the old record and verdict, the typed numbers the assertion lines hold, and the engine dump at 60 ms that question 3 now needs. None of the 60 ms dumps exists yet; the ones in `records-engine/` are at 85.

The filed rulings on the window (`docs/agents/rulings/code-and-engine.md` lines 55 to 57) were not re-quoted by me; see the file for their text. No test needs a saved expected-output file (line 50).


## tests/test_replay.cpp: A 0, B 42, C 6, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 104 | replay reproduces the engine's score for every corpus path | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 150 | replay: combo_after is the combo once the chord is hit | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 179 | replay: a solo's bonus reaches the on-screen total on the section's last chord | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 209 | targeted search reproduces every corpus path | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 311 | every tied variant stores what a search pricing it alone stores | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 340 | targeted search rejects a tick that is not a fill | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 360 | replay without Star Power scores no doubling at all | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 386 | a squeezed-out chord past the leeway earns nothing | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 407 | paid_by_sp: yes exactly when the row's SP value is above zero | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 418 | replay: a squeezed-out chord SP pays nothing shows the plain multiplier | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 462 | replay: the squeeze-out warning ignores a chord only a zero-paying window reached | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 493 | shown_multiplier doubles the combo multiplier only under Star Power | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 508 | a path JSON names the squeezed-out chord by its tick, the offset only for older dumps | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 550 | windows read from a path JSON match the ones read from the record | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 611 | a dump carries each window's SqIn phrases back to the replay | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 649 | paths_json writes every field the dump readers use | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 736 | paths_json: a constant tempo gives end ratios of 1 and no change on any side | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 758 | paths_json: a tempo marker on the activation tick gives the earlier segment's ratio | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 779 | paths_json: a change on the tick bounds neither of its steady spans | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 798 | paths_json: a time signature change between the ticks gives the measure ratio | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 820 | paths_json: a clamped end is anchored on its collecting note | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 835 | paths_json writes the end ratio the search stored | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 854 | paths_json: an activation with no deact tick writes every end field null | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 871 | result block: an empty record writes a null score and an empty bestpath | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 888 | paths_json prints the stored passed-over fills, not a guess | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 917 | a window ending on a phrase note with no offset is flagged | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 994 | per-note sp points sum to the chord's sp points | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1059 | fixtures: mark_phrase_end sets the flag and the phrase start together | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1072 | a typed squeeze-out offset resolves to the phrase chord | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1110 | a typed squeeze-out on a chord the engine never squeezes out is refused | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1139 | a typed squeeze-out offset halfway between two phrase chords is refused | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1165 | a typed squeeze-out on a phrase banked before the activation is refused | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1200 | a typed squeeze-out on the phrase after a banked one is the engine's | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1225 | the squeeze-out warning names the chord the graph would squeeze | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1272 | the squeeze-out warning quotes the whole drop under sqout_rule | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1316 | squeeze_window_phrases: the phrase chords strictly inside the window, in chart order | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1350 | offered_phrase: the newest step's phrase early, the first unsqueezed one late | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1375 | sqout_chords: a window's newest phrase at or before D, or the first after D | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1395 | category_scores reports the multiplier each note was paid at | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1420 | category_scores prices a dynamic cymbal's bonus at its own note | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1445 | replay reports the multipliers category_scores applied | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1477 | replay copies each note's dynamics bonus | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1489 | replay names each note's dynamic | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1517 | chord_score_table: literal rows on a hand-built chart | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1595 | replay multipliers agree with the combo on every corpus chord | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1624 | replay score fields: one list in schema order | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1635 | assign_score and score_of are each other's reverse | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |
| 1649 | replay: scores_only leaves chord_code and notes empty and every score the same | B | replay, JSON and score-table functions over hand-built charts or over one stored path; no search runs, or the two sides read the same path |

## tests/test_pb_coach.cpp: A 0, B 0, C 19, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 38 | coach rows: a two-click path fills the ladder, the rows, the ends and Finish the rest for  | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 99 | coach click: an impossible click leaves the path unchanged and says why | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 112 | coach click: a refusal is current only while the player's path stays as it was | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 133 | coach rows: a window over the Path limit carries the tight-window note | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 178 | coach rows: a squeeze with time to spare under a lower limit names the limit alone | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 202 | coach click: a fill that rules out a later window is refused in the later-window sentence | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 242 | coach end: choosing the last window's end pins it | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 276 | coach end: the kept end is the window's own end | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 299 | coach end: an earlier window's endings are the same list | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 368 | coach remove: removing a window drops it and every later one | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 397 | coach remove: removing every window leaves the empty path | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 427 | coach remove: a tick with no window changes nothing | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 439 | coach finish: Finish the rest for me makes BF(n)'s path the player's path as one edit | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 469 | coach click: a result that arrives after the player's path changed is dropped | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 503 | coach rows: a path with no window has no after-your-last-window line | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 516 | coach worker: a newer request supersedes the older | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 572 | candidate costs: the best next fill costs 0 and every other cost is BF(n) minus its best f | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 619 | candidate queue: it prices only the ticks it is given, in order | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |
| 645 | candidate queue: a newer request supersedes the older | C | opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60 |

## tests/test_perf_digest.cpp: A 0, B 0, C 0, delta round 8

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 128 | the corpus's prepared-row digest is pinned | delta round | records/delta-DL*.md has a record |
| 132 | the corpus's prepared-row digest is pinned at Hard, Pro Drums off, 2x Bass off | delta round | records/delta-DL*.md has a record |
| 136 | the corpus's prepared-row digest is pinned with the CH 1.0 fill rule | delta round | records/delta-DL*.md has a record |
| 142 | the corpus's prepared-row digest is pinned with Note Shuffle on and the ms limit off | delta round | records/delta-DL*.md has a record |
| 149 | the corpus's parse digest is pinned | delta round | records/delta-DL*.md has a record |
| 154 | the corpus's parse digest is pinned at Hard, Pro Drums off, 2x Bass off | delta round | records/delta-DL*.md has a record |
| 160 | the corpus's parse digest is pinned at Medium, Pro Drums off, 2x Bass off | delta round | records/delta-DL*.md has a record |
| 164 | the corpus's parse digest is pinned at Easy, Pro Drums off, 2x Bass off | delta round | records/delta-DL*.md has a record |

## tests/test_pb_text.cpp: A 0, B 4, C 6, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 52 | choices hint: a row is clicked, and a hover promises nothing | B | wording functions over hand-made rows (no corpus::analyzed in the body) |
| 56 | candidate row: the clock, phrases, end and squeeze of a priced fill | C | default window 60 via default_cfg(); asserted value can depend on the search |
| 112 | choice order: cheapest first, ties in chart order, pending last | B | wording functions over hand-made rows (no corpus::analyzed in the body) |
| 131 | step short: start, end and phrases | C | default window 60 via default_cfg(); asserted value can depend on the search |
| 146 | step short: the row carries the Paths tab's badge words | C | default window 60 via default_cfg(); asserted value can depend on the search |
| 165 | end choice text: the end measure and the Paths tab's badge words | C | default window 60 via default_cfg(); asserted value can depend on the search |
| 203 | "fill refusal: the five kinds in fill_reason's order, and fill_reason's sentence | C | default window 60 via default_cfg(); asserted value can depend on the search |
| 298 | fill card: an offered fill reads the heading, three rows and two grey lines | B | wording functions over hand-made rows (no corpus::analyzed in the body) |
| 313 | fill card: a costless fill reads best and \"This is the best choice now.\" | B | wording functions over hand-made rows (no corpus::analyzed in the body) |
| 326 | "fill card: taken, locked, refused and pending fills read the heading and their  | C | default window 60 via default_cfg(); asserted value can depend on the search |

## tests/test_analysis.cpp: A 0, B 48, C 0, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 54 | discover_charts walks the corpus without throwing | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 68 | discover_charts returns nothing for an empty root list | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 89 | discover_charts output matches the checked-in scan snapshot | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 122 | rescan cache reproduces the scan without reading chart files | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 173 | run_batch saves a record with its top paths through the one save | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 198 | charts_with_result counts a Ready record and not a Stale or sibling-profile one | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 223 | run_work_pool hands every item to the consumer once | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 230 | run_work_pool: a slow consumer holds the workers back | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 270 | run_work_pool: a consumer that throws while workers wait for room | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 291 | run_work_pool: a cancel mid-run never strands the consumer | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 345 | run_work_pool refuses a worker count below 1 | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 358 | save_scan_as_library stores every ScanItem field | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 389 | normalize_chart_hash lowers a hash and leaves a lowercase one alone | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 418 | run_batch: cancel stops running searches within seconds | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 470 | run_batch: a cancelled real search is neither a result nor a failure | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 504 | run_batch analyzes a chart found in two folders once, and counts both rows | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 545 | plan_batch: a second copy runs with its first and counts as a row | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 560 | plan_batch: every copy of a stored chart counts as skipped | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 572 | plan_batch: a stored failure is skipped and counted as can't analyze, every copy | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 586 | plan_batch takes its copies from ChartIndex | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 617 | run_batch reports analyzed, skipped and failed itself | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 653 | run_batch reports a failed chart once per copy, under its first copy's name | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 718 | run_batch counts a failed save as a failed chart and goes on | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 753 | run_batch hands on_error the sentence its exception's kind names | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 776 | run_batch hands on_error no details line for a chart with no notes | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 801 | run_batch stores a chart with no notes, and the next batch skips it | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 846 | run_batch stores nothing for a song file it can't open | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 889 | run_batch hands on_failure_saved the chart whose failure it stored | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 914 | run_batch hands on_failure_saved nothing for a failure the store does not keep | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 934 | run_batch hands on_failure_saved nothing when the failure record's save fails | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 973 | run_batch hands on_failure_saved nothing after a run fails as a whole | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 999 | prepare_failed_chart: a remembered kind becomes a failure record, anything else nothing | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1025 | run_batch: a run whose profile asks for another cap than its search fails each chart | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1051 | discover_charts: a song with no usable name reads (unknown) | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1095 | read_song_ini_keys reads a Windows-1252 song.ini as UTF-8 | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1112 | rescan cache: an old placeholder or blank title reads (unknown) | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1136 | "discover_charts: a blank or missing artist or charter reads the placeholder, " | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1195 | "discover_charts reads the album from song.ini, a .sng and an .srb, trimmed, and | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1242 | read_chart_meta reads the names the scan reads from song.ini, a .sng and an .srb | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1303 | discover_charts finds a folder whose notes and ini names are capitalized | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1322 | analyze_chart_file reads a drum track whose first name is unrecognized | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1338 | run_batch: a group commits before any of its charts is reported | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1365 | run_batch: cancel during a group writes nothing more and leaves no transaction open | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1391 | "run_batch: a failed group COMMIT saves each chart alone, and only a chart whose | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1436 | the scan's folder count is reported while the walk runs, from the calling thread | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1460 | the scan's items come out in walk order whatever order they were hashed in | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1498 | a folder two roots reach takes its rootfolder from the root the walk reaches it by | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |
| 1528 | discover_charts walks a folder given twice in two spellings once | B | run_batch tests stub the analyze callback or assert chart counts, status flags, store rows and sentences; no assertion reads a path, score or E mark (assertion lines read; bodies not read line by line) |

## tests/test_app_reports.cpp: A 0, B 20, C 0, delta round 10

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 86 | a path report request builds it in memory, one build at a time | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 114 | showing the path report opens its window and builds it only the first time | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 128 | --open-report opens the path report once the button shows, and only once | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 155 | the path report stays shut at a tick without --open-report | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 166 | closing the path report keeps its rows, and reopening shows them without a build | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 190 | a path report built on opening its window says so; Refresh's does not | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 213 | closing the path report mid-build lets the build finish and land | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 245 | reopening the path report after a cancelled build shows it cancelled | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 272 | the path report goes out of date on a batch or a setting it was built from | delta round | records/delta-DL*.md has a record |
| 332 | a setting changed during a build leaves that build out of date | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 349 | Cancel ends a path report build as cancelled and keeps the last report | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 370 | select_chart selects the first copy in the bar's mode | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 398 | select_chart opens the first copy by name when the search hides every copy | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 433 | a comparison row click selects the chart and keeps the bar's mode | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 467 | picking a player opens the comparison; Refresh reuses the player | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 569 | opening the path report runs a batch over the charts with no result, then builds | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 613 | the window's Cancel stops the report's own batch, and no report follows | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 636 | more rows to run than the threshold open the batch confirm first | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 741 | closing the path report while it asks ends the question, and reopening asks again | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 766 | a batch already running is waited on, and its finish builds the report | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 782 | the toolbar offers the path report whenever the library has rows | B | asserts report window state, build counts, out-of-date flags and store calls; a path report is built from stored rows and the assertions never read a path or score (assertion lines read) |
| 793 | the post-batch report lists the batch's cap and lens, not the live settings | delta round | records/delta-DL*.md has a record |
| 816 | the strip's report_started holds through a cancel, a refresh and a close | delta round | records/delta-DL*.md has a record |
| 952 | a path report builds while a save group is open | delta round | records/delta-DL*.md has a record |
| 972 | a DM report builds while a save group is open | delta round | records/delta-DL*.md has a record |
| 994 | a path report reads one moment | delta round | records/delta-DL*.md has a record |
| 1022 | a path report's separate store calls read one moment | delta round | records/delta-DL*.md has a record |
| 1056 | a report whose connection cannot open fails with the store's sentence | delta round | records/delta-DL*.md has a record |
| 1084 | a finished report holds no connection | delta round | records/delta-DL*.md has a record |
| 1097 | a cancelled report closes its connection | delta round | records/delta-DL*.md has a record |

## tests/test_preview_scene.cpp: A 0, B 31, C 2, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 37 | build_preview_scene: notes carry lane and drum attributes | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 70 | color_of: each lane maps back to the colour it was drawn from | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 80 | build_preview_scene: the song length is the one handed in, never a note's time | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 96 | build_preview_scene: SP phrase, solo, and fill spans | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 115 | build_preview_scene: a fill whose chord sits before its measure line spans chord to line | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 128 | build_preview_scene: a generated fill draws the half measure before its chord | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 174 | build_preview_scene: solo spans come from the song's solo sections | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 189 | build_preview_scene: each solo's notes for the solo box, path-free | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 209 | build_preview_scene: drum lanes come from the song's drum lanes | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 227 | build_preview_scene: an unanalyzed chart offers every candidate fill | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 234 | build_preview_scene: skips say which fills the path was offered | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 249 | build_preview_scene: a second activation with skips 0 hides what lies between | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 271 | build_preview_scene: a fill an activation took stays taken when a later one lists it | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 289 | build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 306 | build_beat_events: bars, beats, and half-beats from the timing alone | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 330 | build_beat_events: a 3/4 section changes the beat count per bar | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 351 | build_beat_events: 6/8 steps by the eighth note | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 373 | build_beat_events: the fainter line is halfway back to the line before, at a restart too | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 389 | build_preview_scene fills beats, tempos and resolution | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 411 | build_preview_scene: the beat lines stop at the song's length (D266) | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 448 | build_preview_scene: no path means no overlay | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 454 | parser keeps the SP-phrase start on the flagged note | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 475 | build_preview_scene: an analyzed chart's overlay matches its path | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 690 | base + overlay: equals build_preview_scene on the hand-built fixtures | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 744 | base + overlay: equals build_preview_scene on every corpus chart and stored path | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 778 | path_overlay_key: no overlay, the same path, and a changed path | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 805 | build_preview_scene: passed-over fills on several activations, and a tick that is no fill | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 821 | build_preview_scene: each activation names the fill it took | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 863 | build_preview_scene: a ghost path lists its own taken fills and nothing else | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 898 | build_preview_scene: a corpus best path as the ghost lists its taken fills | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 925 | preview scene: fills after the last activation read Offered when the engine lists them, Hi | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 967 | preview fills: every stored passed-over fill is a fill in the scene | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |
| 994 | preview path label: the notation and which list it came from | B | scene builders read hand-built songs and paths, or compare the scene with the same stored path; no typed value taken from a search |

## tests/test_pathstring_read.cpp: A 0, B 6, C 4, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 82 | pathstring read: every corpus best path round-trips | B | round trip of one stored path through its own text, or a refusal sentence |
| 90 | pathstring read: a token no fill can place is refused with its position | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 99 | pathstring read: every refusal is ErrorKind::PathTextRefused | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 115 | pathstring read: \"(No activations.)\" gives a path with no activations | B | round trip of one stored path through its own text, or a refusal sentence |
| 126 | pathstring read: every corpus path round-trips from Hydra's long Copy path text | B | round trip of one stored path through its own text, or a refusal sentence |
| 135 | pathstring read: long text with no activations gives a path with no activations | B | round trip of one stored path through its own text, or a refusal sentence |
| 148 | pathstring read: a long-text window no fill can place is refused with its position | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 175 | pathstring read: the graph entry point reads the same path as the settings one | B | round trip of one stored path through its own text, or a refusal sentence |
| 203 | pathstring read: the graph entry point refuses a token no fill can place | B | round trip of one stored path through its own text, or a refusal sentence |
| 212 | pathstring read: long text with the wrong number of sections is refused | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |

## tests/test_report_rows.cpp: A 0, B 30, C 0, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 204 | report rows: a tied top-score variant is optimal too | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 230 | report rows list every stored record (4 bars) | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 235 | collect_rows: the row list is sized to its rows | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 253 | path report: a batch's records read back as a fresh analysis's rows | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 340 | path report: a saved figure below 0 shows no timing and the none tier | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 376 | all_paths lists paths best first on every corpus chart | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 395 | report counts every library copy of a chart, and leaves out a chart the library dropped | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 430 | path report: the Charts shown tile counts what generate_report counts | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 451 | generate_report hands over its rows, subtitle and footer | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 494 | path report coverage: shown + can't analyze + no result is the library count (corpus) | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 548 | path report coverage: a chart with a stored failure gets no row and one list entry | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 579 | path report coverage: the lines' words | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 628 | path report coverage: no Ready chart but a stored failure reads Nothing to show | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 645 | report lists only the wanted cap and names it | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 689 | generate_report reports a 1.0-fills database under the 1.0 rule | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 713 | "collect_rows: the library names go through display_title, display_artist and " | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 737 | the report names a chart the scan found twice by its first copy | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 755 | fill_rule_for: the legacy_fills flag names one fill rule | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 761 | tier_for: raw-ms bands derived from the two-hit budget | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 786 | tier_for walks the timing_tiers table edge by edge | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 801 | path report: a row's search text is folded and tag-free | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 812 | generate_report: one seam frames the report for every entry point | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 870 | generate_report: \"every path\" is the sentinel's, a huge --paths stays a count | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 887 | nothing_under_settings frames the chart mode, the cap and the fills | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 902 | generate_report hands back nothing when its cancel flag is set | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 919 | a report with no analysis settings is refused | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 930 | a report on results with no chart library says the library is missing | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 951 | tier_for over a built table matches the one-argument form | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 962 | PageSettings::of differs when only the chart mode changes | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |
| 976 | settings_change_touches: the path report reads the SP cap and the lens | B | asserts row fields against the same record, counts of charts (shown, can't analyze, no result) and wording; both sides of each comparison come from one stored record, and no typed value reads a path (assertion lines of the batch cases read) |

## tests/test_app_click.cpp: A 0, B 28, C 1, delta round 11

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 98 | clicking a not-analyzed song shows its paths and turns its row Ready | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 118 | clicking a Ready song shows its paths and stores nothing new | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 140 | the click's save marks both open reports out of date | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 162 | a Stale converted record is replaced on click | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 195 | clicking a Stale song turns its row Ready | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 220 | clicking a chart edited since the scan re-identifies it and shows the new file | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 254 | a click whose re-identify fails shows the error and saves nothing | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 277 | clicking a chart touched since the scan saves its new fingerprint | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 306 | a touched chart whose fingerprint save fails shows the error and saves nothing | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 340 | clicking a touched chart that lost its song.ini leaves its library row alone | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 362 | clicking an edited chart that lost its song.ini still re-identifies it | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 382 | clicking A then B in one frame shows B, and A is never shown or saved | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 403 | Cancel shows Cancelled and saves nothing; Try again shows the paths | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 424 | closing the panel cancels the click's analysis and saves nothing | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 439 | a song whose file is gone shows FileMissing and starts no job | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 450 | the settings lock is the batch's alone | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 461 | a click whose save fails shows the paths and says so | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 549 | a click's save does not stop the window while a save group is open | delta round | records/delta-DL*.md has a record |
| 578 | a click's worker writes a changed chart's hash before it saves | delta round | records/delta-DL*.md has a record |
| 607 | a click cancelled before its save saves nothing | delta round | records/delta-DL*.md has a record |
| 621 | a click closed before its save saves nothing | delta round | records/delta-DL*.md has a record |
| 636 | a click replaced before its save saves nothing | delta round | records/delta-DL*.md has a record |
| 658 | a click whose job was let go after it wrote still gives the row the store's answer | delta round | records/delta-DL*.md has a record |
| 697 | a click cancelled during its save keeps the panel cancelled and gives the row the store's  | delta round | records/delta-DL*.md has a record |
| 708 | a click closed during its save gives the row the store's answer | delta round | records/delta-DL*.md has a record |
| 720 | a click replaced during its save gives the row the store's answer | delta round | records/delta-DL*.md has a record |
| 739 | a dropped click that wrote a new hash still reloads the library | delta round | records/delta-DL*.md has a record |
| 758 | a dropped click's save marks both reports out of date | delta round | records/delta-DL*.md has a record |
| 792 | a click on a chart with no notes stores the failure once | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 819 | a click whose song file can't be opened stores nothing | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 840 | the click's Dynamics count keeps the 2x kicks with 2x Bass off | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 866 | a bad hydra_rules.ini names the key and keeps analysis off | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 903 | a bad hydra_settings.ini keeps analysis off and saves nothing | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 957 | no hydra_rules.ini leaves analysis on | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 966 | under a bad hydra_rules.ini no stored record reads Ready | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 998 | the click reads a chart's stated length | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 1015 | a chart with no stated length and no audio reads its last note | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 1034 | the chart-file check runs on open and then every two seconds | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 1056 | UI timings: Copied!, search re-filter, batch refresh and the progress box delay | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |
| 1067 | the toolbar's narrowed label comes from one function | B | asserts row status (Ready, Stale), what the store holds and which sentence shows; compares a stored record with the fresh one from the same settings; no typed path or score |

## tests/test_saved_figure_fill.cpp: A 0, B 6, C 4, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 93 | a saved path with a squeeze-in on the SP end gets its figure back | B | fill markers, stops and stale stamps on hand-made rows |
| 121 | a saved path whose chart is gone keeps its empty figure | B | fill markers, stops and stale stamps on hand-made rows |
| 133 | a row that disagrees with its replay keeps its empty figure | B | fill markers, stops and stale stamps on hand-made rows |
| 147 | a stop leaves the marker unset | B | fill markers, stops and stale stamps on hand-made rows |
| 168 | a chart whose replay throws keeps its rows empty, and the fill goes on | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 212 | the fill on several workers writes the same figures as on one | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 286 | an inside-Star-Power squeeze-in fills a negative figure that shows as none | B | fill markers, stops and stale stamps on hand-made rows |
| 308 | the fill hands every row of one chart the same graph | B | fill markers, stops and stale stamps on hand-made rows |
| 341 | a result this build saves holds a saved figure on every path with a squeeze-in | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 371 | a row under an older stamp is left alone by the fill | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |

## tests/test_replay_schedule.cpp: A 0, B 40, C 0, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 106 | schedule: every end step names its anchor | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 153 | schedule: a squeeze carries its own end, anchor and stored ratio | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 208 | schedule: paid_by matches in_sp on every corpus path | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 241 | schedule: declined chords are the skipped fill ticks | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 267 | schedule: the window constants are written | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 284 | schedule: sides come from the whole chart | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 317 | schedule: a missing --path refuses by name | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 325 | schedule: a chart with no path refuses by name | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 337 | schedule: nothing under src/ reads the schedule | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 419 | schedule: the equal-room split on Freak's clamped squeeze | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 473 | schedule: Freak 1144, Rescued 561 and 931, Ghost 756 | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 501 | schedule: a squeeze that needs no timing is pressed plain | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 526 | schedule: a squeeze the game cannot make is not feasible | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 548 | schedule: the fallback block | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 607 | schedule: last_send_ms is the side less one frame | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 620 | schedule: an activation that needs both directions is refused naming both chords | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 636 | schedule: a lead across a steady span is refused naming the span | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 653 | schedule: a press that reaches a neighbour on its pad is refused naming both chords | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 679 | schedule: a late gem that pays less than an inside gem is refused | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 697 | schedule: press groups | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 735 | schedule: the planned end counts one frame | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 773 | schedule: the equal-room split of a squeeze-in is the mirror of a squeeze-out's | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 793 | schedule: moving_side reads the anchor's press, not the squeeze's kind | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 822 | schedule: song_length_ms is app::song_length_ms | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 851 | schedule: sp_ready_tick is the second bank arrival | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 905 | schedule: the late press holds sqout_late_notes | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 939 | schedule: a whole_chord chord with no inside group has an empty inside list and an empty j | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 964 | schedule: first_note writes the same schedule as before | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1005 | schedule: hydra_replay_build is written | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1062 | schedule: one frame everywhere | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1098 | schedule: far press follows TM-50 | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1130 | schedule: every skip writes a refusal and a flag | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1272 | schedule: a squeeze-out that is also a skipped fill | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1345 | schedule: one decline rule | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1378 | schedule: the first press group is the activation note | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1514 | schedule: Pitch Dark prices each read at its own step's scale | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1621 | schedule: Pitch Dark's read range is its read end's steady span | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1651 | schedule: a lead that moves the read end past a tempo change is refused naming its span | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1680 | schedule: a tempo change on an end splits the conversion's sides | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |
| 1716 | schedule: read_at_own_ms runs the conversion backwards on its own side | B | schedule builder reads hand-built paths or compares two readings of one stored path; asserted ticks are from hand-built charts |

## tests/test_replay_windows.cpp: A 0, B 7, C 0, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 226 | replay: the open-window walk pins every row on hand-built windows | B | replay_path reads hand-built windows, or one stored path against itself; the 500 floors are fuzz size, not path counts |
| 344 | replay: disjoint windows that squeeze out a one-note chord on D | B | replay_path reads hand-built windows, or one stored path against itself; the 500 floors are fuzz size, not path counts |
| 382 | replay: window order does not matter and SP points add up window by window | B | replay_path reads hand-built windows, or one stored path against itself; the 500 floors are fuzz size, not path counts |
| 426 | replay: window order does not matter and windows add up, shrunk by RapidCheck | B | replay_path reads hand-built windows, or one stored path against itself; the 500 floors are fuzz size, not path counts |
| 494 | replay: on every corpus path the disc follows SP's payment and windows add up | B | replay_path reads hand-built windows, or one stored path against itself; the 500 floors are fuzz size, not path counts |
| 548 | replay_path with a table handed in equals replay_path building its own | B | replay_path reads hand-built windows, or one stored path against itself; the 500 floors are fuzz size, not path counts |
| 574 | replay_path refuses a table of the wrong size or detail | B | replay_path reads hand-built windows, or one stored path against itself; the 500 floors are fuzz size, not path counts |

## tests/test_cli.cpp: A 0, B 18, C 1, delta round 2

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 183 | hydra_batch fills a new database under the rule it ran under | delta round | records/delta-DL*.md has a record |
| 223 | hydra_batch and hydra_fillcompare say why a database won't open | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 242 | hydra_batch --legacy-fills writes the tool's own hydra.db under the 1.0 rule | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 268 | hydra_batch refuses a library file too old to convert in one sentence | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 285 | hydra_batch refuses an unknown option | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 297 | hydra_batch and hydra_fillcompare print the one unknown-option line | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 313 | hydra_batch refuses a run with no folder and no --whole-library, and a mix of both | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 334 | hydra_batch --whole-library refuses a chart folder that cannot be read | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 378 | hydra_batch names a bad hydra_rules.ini and stops | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 396 | hydra_batch names a bad hydra_settings.ini and stops | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 412 | hydra_fillcompare names a bad hydra_settings.ini and stops | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 429 | hydra_batch writes a 1.0 run and a 1.1 run into one database | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 468 | hydra_batch reuses the GUI's scan cache | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 490 | hydra_batch prints an artist made only of tags as (unknown) | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 511 | hydra_batch names the cap with the one count rule | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 529 | hydra_fillcompare reads the tool's own hydra.db without --db | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 568 | hydra_fillcompare refuses --old and --new and names --db | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 581 | hydra_fillcompare compares both rules out of one database | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 636 | hydra_replay refuses --ms, --efill and --depth values that are not whole numbers | delta round | records/delta-DL*.md has a record |
| 682 | hydra_replay score writes act_ms and deact_ms for each window | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |
| 714 | hydra_batch and hydra_fillcompare import no winhttp.dll | B | tool exit codes, option sentences and store rows; the only search is a one-chart batch whose assertions are keys and counts of rows |

## tests/test_activation_rows.cpp: A 1, B 27, C 2, delta round 7

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 60 | display format: the average multiplier rounds the exact double | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 68 | display format: ms text is one decimal, a space and the unit | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 76 | build_activations: the early fill reads positive = early on both lines | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 118 | build_activations: rows and backend rows line up | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 142 | build_activations: the scale line shows the multipliers its rows read, early first | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 301 | kTransferScaleHint explains the measure scaling | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 310 | build_activations: overfill warning text | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 357 | "build_activations: the backend limit hides far rows but never " | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 419 | build_activations: the backend limit says how many rows it hides on each side | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 477 | build_activations: a squeezed-out row past the leeway is worth 0 | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 525 | build_activations: plain rows past the leeway show 0 | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 570 | format_measure: one form for both tabs | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 582 | activation rows: Burnout's three activations | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |
| 638 | activation timeline: onset over the song's length, and the end measure | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 740 | activation badges: one per timing, the fill first, then each squeeze | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 841 | activation badges: an early fill with time to spare shows, grey (D261) | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 854 | activation badges: the row is difficult exactly when a badge is orange | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 871 | activation badges: the user's row agrees with the copied path | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 897 | activation badge and copied path agree | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 925 | measure labels: one numbering rule for the rows, the timeline and the first measure | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 936 | longest_activation_badge: the widest wording with the window's figure | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 945 | squeeze sentences: SqIn, SqOut, and what a squeeze-out costs | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 1025 | squeeze sentences at the SP end: the squeeze-in earns, the squeeze-out earns | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 1080 | backend table: a counted row inside SP shows its early-scale eff. figure | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 1096 | build_activations: a near-1 multiplier prints its decimals and its row's figure | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 1167 | build_activations: an unknown scale says so and shows no eff. | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 1186 | activation rows: the chord names its notes in the Pro Drums setting's words | A | 85 ms (kHitWindowMs), set at tests/test_activation_rows.cpp:1228 in analyzed_like_the_gui |
| 1304 | dynamic hit window: the label is one constant | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 1308 | dynamic hit window: Tapped Out's squeeze out is out of reach | delta round | records/delta-DL*.md has a record |
| 1331 | dynamic hit window: Burnout's squeeze out has 5.4 ms safe | B | build_activations over hand-built Activations whose e_offset values (-30, -12.3, 10, 20, 56.2, 300) lie on the same side of 60 and 85 (e_window_ms defaults to kDefaultEarlyFillWindowMs = 60, src/core/model.h:606); 56.2 sits 3.8 ms inside 60 |
| 1354 | dynamic hit window: a squeeze in with a timing names its anchor the frontend | delta round | records/delta-DL*.md has a record |
| 1396 | dynamic hit window: an early fill gets one window, required or optional | delta round | records/delta-DL*.md has a record |
| 1439 | dynamic hit window: a squeeze in that needs no timing still gets its row | delta round | records/delta-DL*.md has a record |
| 1469 | dynamic hit window: a row for every timing badge, edited figures too | delta round | records/delta-DL*.md has a record |
| 1543 | dynamic hit window: no chord times, no row anywhere | delta round | records/delta-DL*.md has a record |
| 1556 | dynamic hit window: the row sits alongside and changes no other field | delta round | records/delta-DL*.md has a record |
| 1607 | dynamic hit window: every corpus row | C | default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search |

## tests/test_preview_golden.cpp: A 0, B 2, C 0, delta round 0

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 258 | preview golden: over_share counts one bad tile against golden.json's limits | B | image comparison of a drawn frame against a screenshot; no search call in the body (token scan) |
| 304 | preview golden: a Hydra frame matches the Onyx screenshot | B | image comparison of a drawn frame against a screenshot; no search call in the body (token scan) |

## tests/test_song_parse.cpp: A 1, B 50, C 0, delta round 1

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 78 | song parse holds its invariants over the corpus | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 92 | a parsed song keeps no spare room in its chord list | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 106 | song parse holds its invariants at Hard too | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 143 | cymbal_counts: a cymbal marker counts only under Pro Drums | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 167 | .chart: each difficulty reads its own section | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 208 | .mid: each difficulty reads its own pitch base | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 234 | .chart: [Events] section markers become practice sections | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 273 | .chart: E \"end\" and E \"[end]\" in [Events] are the end event | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 319 | .chart: the last note end is the latest N line's tick plus sustain in any section | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 343 | corpus: the last note end spans every instrument, through the tempo map | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 397 | .chart: the note on the solo end tick is in the solo | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 406 | .chart: time signatures are kept as written | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 427 | .mid: the note on the solo marker's note-off tick is outside the solo | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 435 | song: solo_sections lists each run of solo chords once | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 491 | drum lanes: the testdata lanes, with the pads Clone Hero lights | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 542 | .chart: a double lane on a three-pad chord takes its second pad from the next chord | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 561 | .chart: a double lane on a two-pad chord lights the next chord's highest pad | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 570 | .chart: a double lane on a two-pad chord skips a lone kick | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 579 | .chart: a double lane takes its second pad from a chord past the lane's end | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 589 | .chart: a double lane whose next chord repeats its first pad lights it once | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 598 | .chart: a double lane on the song's last chord lights its first pad alone | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 606 | .chart: N 66, 67 and 68 mark the yellow, blue and green cymbals; N 65 marks none | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 631 | .chart: a double lane on a one-pad chord takes the next chord with a pad | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 650 | .chart: a lane over kicks only, or over nothing, is dropped | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 660 | .chart: a lane starts on the first pad from one tick before it to its end | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 673 | .chart: a lane shows on each difficulty whose section holds it | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 683 | drum lanes take their pads from the shuffled notes with Note Shuffle on | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 725 | .chart: with Note Shuffle off, a lane over several pads keeps the game's pads | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 739 | .chart: with Note Shuffle on, a single lane over a red roll lights every pad | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 750 | Song::note_count: the chart's note total under each 2x Bass setting | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 769 | .mid: EVENTS text metas become practice sections | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 808 | .chart: a timing line that can't measure time is refused | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 841 | .chart: a TS 0 timing line is still ignored | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 852 | .mid: a timing line that can't measure time | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 878 | a resolution that is 0 or negative is refused in both formats | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 891 | mid: kick velocity is read as ghost/accent, like a pad's | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 943 | mid: Won't Get Fooled Again (O) has 36 ghost kicks | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 969 | mid: the marker pitch table is the one list the parser reads | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 981 | chart_files: loose-folder notes names match in any case | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 992 | chart_files: a path's format comes from its extension in any case | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1001 | mid: a stray SP note-off flags nothing | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1017 | .chart: [Song] Offset is read in seconds | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1090 | .chart: disco markers match the pinned tables | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1117 | .mid: disco markers match the pinned tables | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1147 | .chart: section headers are found as the pinned table says | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1200 | the no-notes error carries no_notes_message's sentence | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1207 | load-and-check throws the no-notes error for a missing difficulty | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1232 | Note Shuffle switch: a game-confirmed song loads shuffled on and as written off | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1253 | Note Shuffle switch: a chart the game freezes on gives the freeze error | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1284 | Note Shuffle switch: the Preview's song key differs when only the switch differs | B | parser output (lanes, sections, times, errors); fill_refuses and is_e0 are never reached from the asserted values (token scan: no search entry) |
| 1292 | Note Shuffle switch: the Dynamics pads follow the analysis's shuffle | A | 85 ms (kMaxEarlyFillWindowMs = kHitWindowMs), set at tests/song_digest.h:332 digest_settings |
| 1385 | the crafted edge files parse to their pinned lines | delta round | records/delta-DL*.md has a record |

## tests/test_preview_controller.cpp: A 0, B 27, C 0, delta round 1

| Line | Test case | Kind | Value and place, reason, or record |
|---|---|---|---|
| 171 | the Preview load builds the highway timeline on its worker | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 191 | the Preview load stops the beat lines at the song's length, not the audio's end | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 243 | the Preview overlay job builds the highway timeline on its worker | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 291 | the Preview base job builds the path-free scene and timeline | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 338 | track_options: one builder, compared whole | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 348 | a cancelled Preview load stops before decoding | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 361 | a cancelled click job stops before counting | delta round | records/delta-DL*.md has a record |
| 379 | with no audio device the Preview still loads, muted, with a warning | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 403 | with an audio device that won't start the Preview still loads, muted, with a warning | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 434 | preview_config: asked before the first render it fails loudly | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 441 | a Preview asset failure reads Reinstall Hydra, with the raw text as its detail | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 475 | closing the Preview frees its render targets; reopening draws again (WARP) | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 502 | scrub marks: the Preview's scrubber ends at the song's length | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 518 | scrub marks: a stated length past the audio ends playback and the scrubber there | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 535 | the Preview's playback, time box and scrubber all end at the song's length | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 548 | switching paths builds the new overlay off the UI thread | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 575 | shows_path: the same path at another cap, and a different path | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 614 | set_ghost_path: the ghost's fills land on the scene, and clearing it empties them | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 672 | set_ghost_path: a changed chart draws no ghost, as it draws no path | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 729 | a mode change on the open chart reloads only the notes and keeps the playhead | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 791 | a mode with no notes pauses the Preview in place; switching back clears it | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 826 | a second mode change during a notes reload wins | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 869 | the Preview hides the path when the chart file changed since its record | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 895 | the Preview trusts the scan's fingerprint and does not re-hash an unchanged chart | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 938 | busy covers the overlay and base jobs, not only the first load | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 955 | the Preview volume is the settings owner's: default and clamp | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 964 | the Preview's song key: another difficulty, Pro Drums or 2x Bass makes a different song | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |
| 992 | the Preview's song key: the 1.0 fills setting makes a different song | B | asserts the Preview's job order, cancel, audio, length and key behaviour, or compares a ghost's fills with the same record's; no typed path value |

## Helpers

| Helper | Kind | Reason |
|---|---|---|
| tests/song_digest.h | A | `digest_settings` sets `early_fill_window_ms = kMaxEarlyFillWindowMs` (85) at line 332; used by test_song_parse.cpp:1292 and the perf digests (delta round). |
| tests/app_state_fixtures.h | B | passes the caller's settings to `analyze_chart_file` (lines 174, 231); sets no window; each test's row says what it passes. |
| tests/song_details_fixtures.h | B | `burnout()` analyzes under `scratch_settings()`, window 60 (default); users of it are rated per test. |
| tests/dm_fixture.h | B | no search entry point (grep: no hit). |

## C records

Each record: the question 3 evidence now needed is a dump at 60 ms (`hydra_replay ... --efill 60`) of the chart named; the old engine records (`records-engine/`) were dumped at `326cd4e0` under the fixed 85 ms rule. Where a verdict below says "holds", that is by reading the old record's claim, not a re-run. Typed numbers are the numeric literals on the test's assertion lines; each can move if the stored path set changes with the window.

### 1. replay reproduces the engine's score for every corpus path (tests/test_replay.cpp:104)
Delta round: window, replaces records/A23.md record 1
Old verdict: Verdict: Rewrite. Drop `min_paths`; keep the rest. Count the charts that load and require each walked (already there), and require paths > 0 per setting.
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 2. targeted search reproduces every corpus path (tests/test_replay.cpp:209)
Delta round: window, replaces records/A23.md record 4
Old verdict: Verdict: Rewrite. Remove the 300 floor; require paths > 0.
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: 300.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 3. every tied variant stores what a search pricing it alone stores (tests/test_replay.cpp:311)
Delta round: window, replaces records/A23.md record 5
Old verdict: Verdict: Rewrite. Replace the floor and the exact skip count by the printed list: name each skipped variant by chart and path with why it has no oracle, require
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 4. targeted search rejects a tick that is not a fill (tests/test_replay.cpp:340)
Delta round: window, replaces records/A23.md record 6
Old verdict: Verdict: Rewrite. Take the non-fill ticks from the chart's own flags (a tick with no `has_activation` chord), over every chart that loads, and say which chart f
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 5. a dump carries each window's SqIn phrases back to the replay (tests/test_replay.cpp:611)
Delta round: window, replaces records/A23.md record 15
Old verdict: Verdict: Investigate. Claim not checked only: needs the dump and a `hydra_replay score --path` capture for `spent_then_next.chart` at cap 2.
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 6. paths_json prints the stored passed-over fills, not a guess (tests/test_replay.cpp:888)
Delta round: window, replaces records/A24.md record 4
Old verdict: Verdict:    Investigate, claim not checked only: needs a `hydra_replay dump` of a real chart under the CH 1.0 fill rule, to put the stored passed-over fill besi
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 7. coach rows: a two-click path fills the ladder, the rows, the ends and Finish the rest for me (tests/test_pb_coach.cpp:38)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: 364675, 13640, 128640, 364675.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 8. coach click: an impossible click leaves the path unchanged and says why (tests/test_pb_coach.cpp:99)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 9. coach click: a refusal is current only while the player's path stays as it was (tests/test_pb_coach.cpp:112)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 10. coach rows: a window over the Path limit carries the tight-window note (tests/test_pb_coach.cpp:133)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 11. coach rows: a squeeze with time to spare under a lower limit names the limit alone (tests/test_pb_coach.cpp:178)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 12. coach click: a fill that rules out a later window is refused in the later-window sentence (tests/test_pb_coach.cpp:202)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 13. coach end: choosing the last window's end pins it (tests/test_pb_coach.cpp:242)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: 257280, 253440, 257280.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 14. coach end: the kept end is the window's own end (tests/test_pb_coach.cpp:276)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 15. coach end: an earlier window's endings are the same list (tests/test_pb_coach.cpp:299)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: 215040, 706180, 257280, 692360, 253440, 706180.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 16. coach remove: removing a window drops it and every later one (tests/test_pb_coach.cpp:368)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: 334495.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 17. coach remove: removing every window leaves the empty path (tests/test_pb_coach.cpp:397)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: 358235.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 18. coach remove: a tick with no window changes nothing (tests/test_pb_coach.cpp:427)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 19. coach finish: Finish the rest for me makes BF(n)'s path the player's path as one edit (tests/test_pb_coach.cpp:439)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 20. coach click: a result that arrives after the player's path changed is dropped (tests/test_pb_coach.cpp:469)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 21. coach rows: a path with no window has no after-your-last-window line (tests/test_pb_coach.cpp:503)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 22. coach worker: a newer request supersedes the older (tests/test_pb_coach.cpp:516)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 23. candidate costs: the best next fill costs 0 and every other cost is BF(n) minus its best finish (tests/test_pb_coach.cpp:572)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: 199680.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 24. candidate queue: it prices only the ticks it is given, in order (tests/test_pb_coach.cpp:619)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: 165120, 199680.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 25. candidate queue: a newer request supersedes the older (tests/test_pb_coach.cpp:645)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (opens Burnout or White Washed (or a graph) at scratch_settings() / default window 60).
Typed numbers on assertion lines: 199680.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 26. candidate row: the clock, phrases, end and squeeze of a priced fill (tests/test_pb_text.cpp:56)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via default_cfg(); asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 27. step short: start, end and phrases (tests/test_pb_text.cpp:131)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via default_cfg(); asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 28. step short: the row carries the Paths tab's badge words (tests/test_pb_text.cpp:146)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via default_cfg(); asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 29. end choice text: the end measure and the Paths tab's badge words (tests/test_pb_text.cpp:165)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via default_cfg(); asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 30. "fill refusal: the five kinds in fill_reason's order, and fill_reason's sentence (tests/test_pb_text.cpp:203)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via default_cfg(); asserted value can depend on the search).
Typed numbers on assertion lines: 72480, 195360.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 31. "fill card: taken, locked, refused and pending fills read the heading and their  (tests/test_pb_text.cpp:326)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via default_cfg(); asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 32. build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged (tests/test_preview_scene.cpp:289)
Delta round: window, replaces records/A20.md record 14
Old verdict: Verdict: Investigate, claim not checked only (a real 1.0 chart and its record).
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: 19200.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 33. preview scene: fills after the last activation read Offered when the engine lists them, Hidden otherwise (tests/test_preview_scene.cpp:925)
Delta round: window, replaces records/A20.md record 31
Old verdict: Verdict: Rewrite. For every fill after the last activation, state is Offered iff its tick is in `trailing_skipped_fill_ticks`, else Hidden; choose a path by tha
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 34. pathstring read: a token no fill can place is refused with its position (tests/test_pathstring_read.cpp:90)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 35. pathstring read: every refusal is ErrorKind::PathTextRefused (tests/test_pathstring_read.cpp:99)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 36. pathstring read: a long-text window no fill can place is refused with its position (tests/test_pathstring_read.cpp:148)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 37. pathstring read: long text with the wrong number of sections is refused (tests/test_pathstring_read.cpp:212)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 38. a Stale converted record is replaced on click (tests/test_app_click.cpp:162)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 39. a chart whose replay throws keeps its rows empty, and the fill goes on (tests/test_saved_figure_fill.cpp:168)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 40. the fill on several workers writes the same figures as on one (tests/test_saved_figure_fill.cpp:212)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 41. a result this build saves holds a saved figure on every path with a squeeze-in (tests/test_saved_figure_fill.cpp:341)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 42. a row under an older stamp is left alone by the fill (tests/test_saved_figure_fill.cpp:371)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 43. hydra_fillcompare compares both rules out of one database (tests/test_cli.cpp:581)
Delta round: window, replaces records/B1.md record 18
Old verdict: Verdict: Rewrite. State the sum as a property (the six parts add to the total) read from the tool's counts, in place of the typed line.
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 44. activation rows: Burnout's three activations (tests/test_activation_rows.cpp:582)
Delta round: window, replaces the old record for this test (heading not found by title match in records/A*-D*.md; the verifier should locate it via coverage.md)
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: 378315.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.

### 45. dynamic hit window: every corpus row (tests/test_activation_rows.cpp:1607)
Delta round: window, replaces records/A3.md record 22
Old verdict: Verdict: Rewrite. Keep the shape (view against the owner, chord by chord, every chart). Count the skips and require zero, or name each chart allowed to skip; re
Window: default 60 (default window 60 via app::Settings().to_analysis_settings()/fixture; asserted value can depend on the search).
Typed numbers on assertion lines: none found by the scan.
Does the old verdict still hold at 60 ms: not re-judged beyond reading; the claim's wording does not name 85, so the verdict stands where it rested on the test's own lines. Anything the old record took from an engine dump at 85 (a path, count, E mark or activation of a named chart) is unproven at 60.
Question 3 now needs: a `hydra_replay --efill 60` dump of the chart the test uses (named in the test); claim not checked only.
Verdict: Investigate (claim not checked only), unless the old verdict was Rewrite or Remove for a fault in the test itself; then that verdict stays.
