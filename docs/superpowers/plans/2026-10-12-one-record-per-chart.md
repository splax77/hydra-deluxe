# Plan: one record per chart and profile (D115)

Main session 08261223, 2026-10-12. Nothing here is built. The build waits for the user's yes (CLAUDE.md rule 3).

The design is the spec, `docs/superpowers/specs/2026-10-11-one-record-per-chart-and-profile-design.md`. Four Fable planners wrote one part each on 2026-10-12. Their files sit next to this one in `2026-10-12-one-record-per-chart/` and hold each task's full detail: the store (`part-A1-store.md`), the report, its callers and the Analyze button (`part-A2-report-callers.md`), the scoring owners (`part-B1-scoring.md`) and the timing rules (`part-B2-timing.md`). `analyze-button-design.md` is the side session's design note for the button. This file puts the parts in one order, settles where they disagree, and lists what the user still has to decide. **Where this file and a part disagree, this file wins.** Each executor's brief names its part file and the overrides below that touch it.

Every line number in the parts was read at main 47dd4706. An executor greps again before it edits.

## The short version

The work is 22 tasks for Opus executors, plus one documents task and a few steps the main session does by hand. Two chains run side by side.

The **store chain** is serial, because each step builds on the one before. The store's interface comes first. Then the three new tables with the open-time upgrade, then the one save and the one lookup. After that the callers move: the batch and the click, then the two comparison pages, then the report. Last comes a clean-up that deletes the old store functions. Until that clean-up, the old functions stay as thin wrappers over the new tables. So every merge leaves `main` building and passing, and the old store tests keep checking the conversion for free.

The **timing chain** moves scores, so it works on its own branch, `d115-timing`. It merges as one change, after the user has seen the list of charts whose score or fills changed on `testdata/input`.

The scoring owners and the Analyze button fit beside both chains in the first two waves. The three big renames (what "base", "points" and "sp" mean) run last, after everything else. They touch twenty-odd files that the other tasks are still editing.

## One finding the spec missed, and the decision it needs

The timing change makes every saved result read Stale, two ways at once. It bumps the results stamp (ADR 0018). It also removes `fill_length_measures` from the rules, which changes the rules fingerprint.

The store has no "Stale but readable" stamp. `row_readable_sql()` in `src/store/record_store.cpp` (line 431) builds "this build can read the row" from `kResultsStamp.accepted`, and a bump shrinks that list to the new stamp alone (`src/store/stored_versions.h`, the comment above line 62). Spec section 6 says the upgrade drops rows whose stamp this build cannot read. After the bump, that would be all 37,696 of the user's rows (the count is from spec section 1). Every chart would then read Not analyzed, not Stale.

Even without that, no converted record could ever read Ready in this release. Each one carries the old rules fingerprint. So the report's "fill converted records" pass (spec sections 6 and 7) would never run on real data. Every Ready record would come from a batch or a click, and both write all of the record's paths.

The recommended answer (question 1 below):

- The upgrade keeps every old row, whatever its stamp, as a record under its profile. It reads Stale with its reason, as after any stamp bump today. A chart's old records are deleted the next time the chart is saved, by the widened purge (spec section 5).
- The `from_summary` column, the "does this record still need its paths" function and the report's fill pass are dropped. The report lists Ready records only, and those always have their paths.
- So after the update the library reads Stale everywhere, as it does after any bump. The user re-analyzes, and from then on the report opens as a plain database read.

This drops the "Saving paths" bar and most of A2-2's fill half. The tasks below assume this answer. If the user wants to keep the fill pass anyway, the parts' original text stands for A1-1, A1-2 and A2-2.

## The order

Each wave forks from the tip where the earlier wave merged. Within a wave, no two tasks own one file, except the named regions listed under "Shared files". There, git merges non-overlapping changes and the main session settles anything left.

| Wave | Tasks (main) | Tasks (`d115-timing`) |
|---|---|---|
| 1 | A1-0, A2-1, A2-5, B1-1, B1-2, B1-3, B1-5 | B2-1 |
| 2 | A1-1, A1-4, B1-4 | B2-2, B2-3, B2-4 (after main is merged into the branch) |
| 3 | A1-2 | the score check, the user's look, the stamp bump, the merge to main |
| 4 | A2-3 (needs A1-2, A1-4 and the timing merge) | |
| 5 | A2-4 | |
| 6 | A2-2 | |
| 7 | A1-5 | |
| 8 | B1-6, then B1-7, then B1-8 (one at a time) | |
| 9 | D1, the documents | |
| 10 | the real-app check on a copy of the user's database, then the push | |

Wave 1 runs eight executors, so up to eight cold builds compete for the CPU at once. If the first status lines show builds past 8 minutes, the main session holds back the last two dispatches of the wave until the others are warm.

A2-3 waits for the timing merge for one reason: the report must never list a record converted under the current stamp. Once the timing merge has bumped the stamp, no converted record can read Ready, so A2-2's plain read is correct.

## The tasks

One line each says what the task does. The part file has its owned files, tests, commands, single-owner rows and stop conditions. The overrides after each line are this file's changes to the part.

### Wave 1

**A1-0, the store's interface and `ChartIndex`** (part A1). This task adds the new types (`Profile`, `RecordRow`, `PathRow`, `PreparedRecord`, `RecordLookup`), `ChartIndex` with its `count`, and the scan-order read. Everything it adds sits beside the old code, so nothing breaks. Override: `RecordRow` drops `from_summary` and `needs_paths()` (question 1). It gains an `operator==` over every column and every path, because A2-3's click compares a fresh record with the stored one. `ChartIndex::naming` returns the whole first row, so the pages get the title, artist, charter and notes path from it.

**A2-1, the Analyze button follows the status chip** (part A2, plus `analyze-button-design.md`). The button counts and runs `LibraryModel::order()`, the rows on screen. It reads "Analyze shown (N)..." whenever a search or a chip narrows the table. Override: the guide's two lines move to D1, so this task owns no part of `docs/UserGuide.md`.

**A2-5, the report's labels and the window's fitted-text cache** (part A2). This task makes four label changes: "No timing needed", "Optimal paths only", "Note count", and "Average Multiplier" from one constant. It also fixes the stale `tr.best` comment and adds a `FittedTextCache`, so the window stops re-fitting every cell on every frame. Override: A2-5 defines `kAverageMultiplierLabel` in `src/app/display_format.h`, beside `format_avg_mult`. It is the only task that defines it. A2-5 also adds that constant's single-owner row, with baseline entries (`tests/single_owner/baseline.json`, `known_copies`) for the Paths tab's and the Stars tab's old labels. B1-4 removes both entries.

**B1-1, one optimal score and one multiplier-step test** (part B1). This task adds `HydraRecord::optimal_score()`, which `is_optimal` and the Paths tab both read, and deletes the all-0 button's dead "N above optimal" branch. It adds one "crosses a multiplier step" function beside `to_multiplier` in `src/core/timing.*`, called by `MultSqueeze::applies` and `category_scores`. And it rewords the comments that copy note values. Override: B1-1 does not touch `src/app/display_format.h`, the Paths tab's label or `baseline.json`. In `src/core/timing.h` and `timing.cpp` it adds only the new function next to `to_multiplier`, and B2-1 owns the rest of both files.

**B1-2, one place adds Star Power points in the engine; the two limit walks agree** (part B1). One engine helper replaces the two hand-written pairs of adds. A corpus test pins `Engine::act_within_limit` to `Activation::within_ms_limit`. Override: the `Path::sp` → `sp_bars` rename moves to B1-8, with the other "sp" renames.

**B1-3, one parse rule for "cymbals count only under Pro Drums"** (part B1). One function in `src/parse/song.h` answers it. The MIDI reader, the .chart reader and the Preview's `gem_of` (`src/render/track_state.cpp`, a third copy the spec missed) all call it. Shared region: B2-1 edits only `Song::build_timing` in `song.h`.

**B1-5, one owner for "is SP running at this moment"** (part B1, spec M48). `PreviewActivation::sp_running_at` becomes the owner, and `running_activation` calls it. A test pins the highway's span sweep to it at six edges.

**B2-1, Clone Hero's measure grid and continuous measure count** (part B2). `MeasureIndex` becomes Clone Hero's grid, which restarts at the first beat at or after a time signature change. A new `MeasureCount` gives the continuous count that `plusmeasure`, `measures_at_tick_f`, `tick_at_measures_f` and `sp_end_ms` read. This task also adds `tempo_at`, `time_signature_at` and the shared `last_at_or_before`, and renames "sections" to "spans". It works on branch `d115-timing`. Its failing-first test is the 4/4-to-7/8 example from spec section 9: `plusmeasure` returns tick 5280 today and must return 5400.

### Wave 2

**A1-1, the three tables and the conversion** (part A1). On open, the store converts `results` into `profiles`, `records` and `record_paths` once and lowercases stored ids. The old entry points keep working as wrappers over the new tables. Overrides from question 1: the conversion keeps rows of every stamp, and no `from_summary` column is written. The conversion still drops the no-stars rows (`kNoStarsResultSql`), as part A1 recommends. A record with no paths stores a note count of 0 (part A1 question 2; the library shows nothing for it either way). Part A1's interim wrappers are the chosen way to keep `main` green (its question 3). This task owns the one `DROP TABLE IF EXISTS results;` line in `tests/ui/uitest_library.cpp`. A2-1 has merged by then, so nobody else holds that file.

**A1-4, `Settings::profile()`** (part A1). This is the one builder of the current profile, plus the two named variants, `profile_at_clone_hero_cap()` and `profile_with_fills(rule)`. It also writes the profile's single-owner row, with baseline entries for the four places that build a key by hand today. The main session sets each baseline entry's date so it falls inside the scan's 14-day limit (`kBaselineMaxDays`). If the A chain will take longer, the row lands with A1-5 instead.

**B1-4, the Stars column, the Path limit tooltip and the docs-match markers** (part B1). The Stars tab's column title becomes the shared constant, with a header tooltip. The Path limit tooltip is reworded, with the words from question 5. New `known_defaults` entries and their markers cover the star factors, "10 notes" and "15 points". The docs-match test makes the markers and entries land together. Override: B1-4 also switches the Paths tab's "Avg. Multiplier:" in `src/app/path_view.cpp` to the constant, and removes A2-5's two baseline entries. Shared regions: `tests/test_docs_match_code.cpp`, where B2-3 deletes lines 428 to 429 on the timing branch, and the three marker lines in `docs/UserGuide.md` and `CONTEXT.md`.

**B2-2, the Preview reads the two measure owners** (part B2, on `d115-timing`). `build_beat_events` goes through the grid walker. `PreviewBeatKind::Bar` becomes `MeasureLine`, and `PreviewMeter` becomes `PreviewSpan`. The time box reads the three in-force lookups. The SP curve cuts at the continuous count's spans.

**B2-3, generated fills on the measure line, and Clone Hero's section names** (part B2, on `d115-timing`). `check_activations` reads the grid's measure lines and places fills the way Clone Hero's generator does (0x20D2B30). `fill_length_measures` is retired and read-and-ignored in old rules files. `section_display_name` reads the shipped 2,718-row list. The fill count of the report page, "Notes" in `fill_report.cpp`, is not this task's: A2-4 owns that file.

**B2-4, `hydra_replay` prints positions as the screen does** (part B2, on `d115-timing`). The dump's chord position becomes `{"label", "decimal"}` through `format_measure`, plus `fill_start` on fill chords so the score check can see fills move. `probe_chart.py` names `MsIndex` as its source, and a new Python test checks it against `hydra_replay`.

### Wave 3

**A1-2, the one save and the one lookup** (part A1). This task adds `prepare_record`, `RecordStore::save` (with the widened dead-row purge and the emptied-profile delete), `lookup`, `lookup_one`, `ready_song_ids` and `ready_records`. It also measures path-row size on `testdata/input` and scales it to the user's 18,848 charts. Override: the "needs paths" tests go (question 1). The measurement prints its numbers and pins none.

**The timing merge** (main session; the score check is part B2's recipe). The main session merges main into `d115-timing` and builds `hydra_replay` at the branch's fork point and at its tip. It runs both exes over the 115 charts in `testdata/input` (no library run) and writes `changed.md`: every chart whose best score, best path or generated fills differ. The user sees that list in chat. Then the main session makes three edits by hand. It bumps `kResultsStamp` to the release the user names (question 9). It re-pins `tests/test_perf_digest.cpp`, whose four digests fold the engine's output and are expected to change. And it re-pins the rules fingerprints `test_rules.cpp` pins. One fresh reviewer then reviews the joined branch, and the branch merges. The scout predicted that fills change only on Haken's "The Good Doctor" (10 of 29 generated fills) and scores only where Star Power crosses a mid-measure change. Anything else on the list gets explained before merge.

### Waves 4 to 7: the callers, then the clean-up

**A2-3, the batch, the click, the CLI and `tools/bench.cpp` save through the one save; `ReportSeed` goes** (part A2). The click's save also marks an open report out of date (question 4). Override: the names come from A1's interface, not A2's guesses. That means `prepare_record(song_id, profile, analysis)`, `RecordStore::save`, `lookup` and `lookup_one` (the click compares `lookup_one(...).record` with the fresh record when it reads Ready), and `ready_song_ids` for the skip list. `tools/replay_json.cpp`'s calls to `summarize_record` and `best_path_text` move to A1-5.

**A2-4, the two comparison pages and `tools/compare_db.py`** (part A2). Each page reads one profile by name through `ready_records`, and `ChartIndex` answers copies and "in library". `tally_fill_rows` counts through `ChartIndex::count`. Override: the fill page's count column also becomes "Note count", because the user's decision covers every count column.

**A2-2, the report reads stored rows** (part A2). Opening the report becomes a database read of the current profile's Ready records. The header reads "N charts", and "Left out" counts library rows through `ChartIndex::count`. `settings_for_mode`, `chart_rows`, `charts_counted` and `ReportRow::copies` go, and so does the report's own `py_round3`. Override (question 1): no fill pass, no "Saving paths" bar and no `AllFailed`. The report no longer analyzes anything, so the chart files are not opened when it opens. Its row-by-row test is the batch-then-read form only. The empty-database sentence uses the words from question 3.

**A1-5, the clean-up** (part A1). This task deletes every old store entry point, `kNamingCopiesSql`, `summarize_record`, `best_path_text`, `Settings::record_key` and the two interim aliases. Old tests and single-owner rows go with them, and the identity row narrows `normalize_chart_hash` to the leaderboard reply. Overrides: A1-5 also moves `tools/replay_json.cpp` off `summarize_record` and `best_path_text`. It moves `tests/test_perf_digest.cpp` from `PreparedRow` to `PreparedRecord` while folding the same facts in the same order, and **the four digests must not change**. That is the proof that the store's move changed no engine output. If a digest moves, the task stops and reports. It does not re-pin. A1-5's `CONTEXT.md` entries move to D1.

### Wave 8: the renames

These run last and one at a time. Each one touches files the earlier tasks own, so starting them sooner would mean merge conflicts across twenty-odd files. Before each one starts, the main session greps its rename list against the files any open task owns. A hit means it waits.

**B1-6**: the Notes category stops being "base" on the path side (`score_base` → `notes_points` and its kin, listed in part B1). Override from question 7: only these renames, the spec's breadth, not all six `Path` fields.

**B1-7**: `CategoryScores::base` and its kin become `notes`, and `StarCutoffs::base` becomes `chart_base_score`. `hydra_replay`'s JSON key `"base"` becomes `"notes"`. Part B1 found that no tool reads it by that key, and one word should keep one meaning.

**B1-8**: "points" and "sp" mean one thing each. This task makes `sp_points`, `sqout_sp_points`, `frontend_sp_points` and `gain`, plus `Path::sp` → `sp_bars`, which moved here from B1-2.

### Wave 9: the documents (D1)

One executor writes every document, because every part touches the same three files. That means `docs/UserGuide.md` (all the guide sentences in the four parts, A2-1's button lines included), `CONTEXT.md` (the five words from spec section 2, "Optimal path", "Backend row", Tick, Tempo, Time signature, Measure, Beat, Section, Span and SP bar, and the two fill entries without their "not verified" notes), ADR 0030 (it supersedes ADR 0026 in part), decision D115 in `docs/audit/2026-10-03-fix-decisions.md`, and ADR 0010's dated note. The guide is a public page, so it gets one fresh doc review (`docs/agents/doc-review.md`) together with the change's derive-once review. Every label and number in the guide is checked against the code, as `docs/agents/public-doc-style.md` asks.

### Wave 10: the real-app check, then the push

The main session copies the user's `hydra.db`, `-wal` and `-shm` from `C:\Program Files\Hydra` beside the built `Hydra.exe` and opens it. The startup box converts the results. Every chart then reads Stale, and the user sees the tooltips. Seeing the report and the Preview with real records needs a batch over the copy's library, about 18,848 analyses (18,848 charts hold results in the user's file, spec section 1). That is a full library run, so the main session asks for it in chat first (rule 8). The time to quote comes from the 2026-10-11 profile, which ran 37,691 analyses in about 17 s of wall time. One profile is about half of that, and Defender may double it. The user also opens a chart with a 6/8 or 7/8 stretch in the Preview to see the beat lines, the beat number and the measure lines (B2-2). Nothing is pushed before the user has seen this.

## Shared files

These are the only places two tasks of one wave touch one file. Each task's brief names the exact region it may change.

- `src/core/timing.h` and `timing.cpp`. In wave 1, B1-1 adds one function beside `to_multiplier`, and B2-1 owns the rest.
- `src/parse/song.h`. In wave 1, B1-3 adds one declaration, and B2-1 changes only `build_timing`.
- `docs/agents/ui-testing.md`. In wave 1, A2-1 edits line 96 and A2-5 edits line 106.
- `tests/test_docs_match_code.cpp`. In wave 2, B1-4 adds entries, and B2-3 deletes lines 428 to 429.
- `tests/single_owner/rules/`. New rows go in a new file per task under `rules/<area>/`, which the scan already loads (`tests/single_owner/scan.cpp`, lines 233 to 238). Edits to an existing row in `app.json`, `store.json` or `core.json` are owned by one task per wave, as the parts list them.

## How each task runs

Every executor is briefed with `docs/agents/brief-preamble.md` first. Then come its task id, session id 08261223-f3e8-41dc-bce8-512c18b2bbe9, its part file and this file's overrides, its owned files, the tests it may run and the status rule. It works in its own worktree, inside 100 tool calls and 20 minutes. Each merged change gets one fresh Sonnet reviewer, who fixes its own findings and signs off (D107), and the derive-once review that the hook asks for on `main`. The main session builds once, runs the full suite once per merge, and stages files by name. No agent runs a library test.

## Decisions the main session made (code-only, no display, score or stored-record change)

These come from the parts' open questions. Each one changes no screen, score or saved record, so the plan picks one and says so here.

- The interim wrappers keep `main` green, rather than an integration branch for the whole store (part A1, question 3).
- The conversion keeps the no-stars rule (part A1, question 1).
- A record with no paths stores a note count of 0 (part A1, question 2).
- Only the spec's renames, not every `Path` field (part B1, question 3).
- `hydra_replay`'s `"base"` key becomes `"notes"` (part B1, question 4).
- The "points" names from part B1, question 5 are taken as proposed.
- M48 gets its owner plus a pin test. The highway's generic six-kind sweep keeps its own walk (part B1, question 6).
- The star factors get docs-match markers in both the guide and `CONTEXT.md` (part B1, question 7).
- The grid copies Clone Hero's truncated beat step exactly. No chart in `testdata/input` has a resolution where that differs (part B2, question 6).

## Questions for the user

Each of these changes a display, a score or a saved record, so the build waits for the answers. Where a part recommends an answer, the recommendation is given.

All ten were answered in chat on 2026-10-12, each as recommended:

1. The upgrade keeps old rows as Stale records, and the fill pass goes.
2. The Mode column and the row click's mode switch go.
3. The empty database reads "Nothing is analyzed yet. Click a song or run a batch to analyze it."
4. A click marks both reports out of date.
5. Both tooltips take the words above.
6. A generated fill copies Clone Hero, so its length may be 0 or less.
7. `plusmeasure` walks in exact whole numbers.
8. Weak beats are drawn as ordinary beat lines.
9. The stamp becomes "2.8.0".
10. The dump prints only the label and the decimal. The main session edits `video-tools\replay.py` once the timing change merges.

1. **The upgrade and the stamp** (section above). Recommended: keep old rows as Stale records, and drop `from_summary` and the report's fill pass.
2. **The report's Mode column.** The report lists one profile now, so every row's Mode cell reads the same. Recommended: drop the column, and with it the row click's mode switch (`select_chart`'s `with_chartmode` branch). As a result, switching difficulty, Pro Drums, 2x Bass or Note Shuffle marks an open report out of date, which it does not do today.
3. **The empty-database sentence.** `kNoRecords` reads "None of the songs in this run could be analyzed, so there is no report to show." That is wrong for a file that holds nothing yet. Proposed: "Nothing is analyzed yet. Click a song or run a batch to analyze it."
4. **The click and the leaderboard comparison.** A batch marks both open reports out of date (`app_state.cpp` line 607). Should a click's save mark the comparison out of date too, not only the path report? Recommended: yes, both.
5. **Two tooltips.** Part B1 proposes these words. Path limit: "Keep extra paths only when every squeeze and required early fill on them is within this many ms. A path that ties the best score is kept whatever its timing. Lower or negative values demand more slack." Stars column: "The average multiplier a path needs for this many stars. The cutoff is the Base score times this factor."
6. **A chord just before its measure line.** Clone Hero's generator starts the fill on the line, so its length can be 0 or negative. Recommended: copy that. The alternative is to refuse such chords, a rule the disassembly does not show.
7. **`plusmeasure` arithmetic.** Recommended: an exact whole-number walk. Inside one time signature, Star Power then ends at exactly start + measures × ticks per measure. Today's doubles can land one tick early, and the score check lists any chart where that moves a score.
8. **Clone Hero's weak beats in 6/8.** Recommended: draw them like ordinary beat lines, which needs no new asset. The alternative is a third, fainter kind.
9. **The release that ships this.** The stamp is set to that version (for example "2.8.0").
10. **`video-tools\replay.py`** line 213 reads the dump's old measure, beat and tick numbers. Recommended: the dump prints only the label and the decimal, and that one line in the video-tools repo changes to read the label. Should the main session make that edit, or will you?

## Not verified

These are the facts the plan relies on that nobody has proven from Clone Hero's code. Each part names the place it depends on them.

- Clone Hero's Star Power drain functions (0x20D3FC0 and 0x20D2930) are not in the evidence folder. The continuous-count rule comes from the CH-sp-grid scout, whose post-pass constants the main session read on 2026-10-11.
- The grid's constant K (read as 4.0) and the generator's 0.5 are read off data slots, not re-read from the DLL.
- The direction and tie rule of the nearest-chord function 0x5DDB50 are unread. Hydra keeps its own rule there (absolute distance, ties to the later chord), and the scout's re-implementation found no fill that moves to another chord on `testdata/input`.
