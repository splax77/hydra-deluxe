# One record per chart and profile: stored report paths, one answer per question

Status: design, approved in chat on 2026-10-11 (decision D115, written into the decisions file when the build lands). The build waits for a plan and the user's yes on it.

The user's choices, all made in chat on 2026-10-11:

- One record per chart and profile, the "lean profiles" layout.
- Paths are kept in a separate table, not as JSON.
- The upgrade keeps existing rows and fills their paths later.
- An open report shows the out-of-date strip.
- The player sees "charts", never "records".
- "Left out" counts copies.
- No limit on profiles.
- The chip reads "No timing needed".
- The checkbox reads "Optimal paths only".
- The count columns read "Note count".
- "Average Multiplier" is used everywhere, the Stars tab included.
- The Path limit tooltip is reworded.
- The meter walk waits on Clone Hero's rule and a corpus count.
- Section names read as Clone Hero shows them.
- The time signature label and the position share one owner.
- `hydra_replay`'s dump prints positions as the screen does.

One earlier answer was given for an approach the user then set aside. Under one-record-per-chart, a chart under other settings would have read Stale. Under profiles, a chart has a record per profile, so settings never analyzed read Not analyzed, as today (section 4).

## Why

The path report is the one slow thing Hydra does. On 2026-10-11 the main session profiled the real app on the user's library (`docs/handoffs/2026-10-11-path-report-speed-handoff.md`). Building the report took 41.6 of the session's 62 CPU-seconds and about 17 s of wall time. It ran 37,691 analyses of 18,907 chart files, because D87 keeps only summaries and the report analyzes every chart again to get its top 5 paths. It analyzed two modes per chart because the database holds results with Note Shuffle on and off.

The user asked for two things. The database stores only what it needs, with one record per chart. And the questions "what is a chart" and "what is a record" are each answered in exactly one place, because the report's building bar drifted from its header earlier the same day (D103 item 25: the bar read "of 37,691 records" while the header read "38,883 records across 19,444 charts").

A follow-up message widened the second ask: optimal, squeezes, backends, what a note is worth, how a score is computed, the average multiplier and the score breakdowns must each have one owner too. A third added the numbers in the Preview's info box: ticks, tempos, sections, time signatures and measures. Section 9 covers them all.

## 1. What the database holds today

These counts come from a copy of the user's installed `hydra.db` (`C:\Program Files\Hydra`, 17,993,728 bytes, copied 2026-10-11 21:48), read with Python's sqlite3.

The file has three tables. `charts` has 19,503 rows for 18,907 distinct chart hashes; the 596 extra rows are second copies of a chart in another folder. `results` has 37,696 rows for 18,848 charts. `meta` has three labels.

A result is filed under the chart and every setting it ran under: chart mode, SP cap, the five lens numbers and the rules fingerprint (the UNIQUE line in `src/store/record_store.cpp`). The user's file holds 18,848 rows under "Expert Pro Drums, 2x Bass", 18,842 under the same mode with Note Shuffle, and 6 leftovers. Three of the leftovers carry results stamps this build can never read (1.8.2 and 2.1.0+allzero). Nothing deletes them, because a save only clears unreadable rows of its own chart and mode.

Most of a results row is its label. Every row spells out a 32-character hash, a mode name of about 32 characters, the version text, five lens numbers and an 8-byte rules fingerprint, and the UNIQUE index keeps a second copy of all of it. Copied alone into a scratch file and compacted, today's results table with its index took 8,847,360 bytes, about 235 bytes per result.

## 2. The words, each with one meaning

These meanings hold in code, in `CONTEXT.md` and in public text. Section 8 names the one owner of each.

A **chart** is one playable chart, identified by its song id: the lowercase hex hash `song_id_read` gives it (ADR 0028). Two files with the same song id are one chart.

A **library row** is one copy of a chart in one folder, a row of the `charts` table. Library rows are told apart by their notes path (`row_key`). Every count of charts the player sees counts library rows, so a chart with two copies counts twice (D76, D77, D79; unchanged).

A **profile** is one set of settings a chart can be analyzed under: chart mode (difficulty, Pro Drums, 2x Bass, Note Shuffle), SP cap, the lens (ms limit, score range, fill rule), the rules fingerprint and the results stamp. It replaces "result identity" as the stored key. A player never sees the word; the library and the report always show the profile of the current settings.

A **record** is one chart's saved result under one profile: whether it kept any paths, its path count, its star count, and its top paths. "Record" is a code and `CONTEXT.md` word only. Public text says "chart" (user decision, 2026-10-11).

An **analysis** is the engine's full in-memory output for one chart under one profile, `HydraRecord` in the code. The type keeps its name; `CONTEXT.md` calls the thing an analysis, and its glossary entry says the type name is historical. A record is what the store keeps of an analysis.

Today the word "record" has four meanings: the in-memory result (`CONTEXT.md` "Record"), a chart in one mode counted per copy (the report header, the building bar, `docs/UserGuide.md` "A record is one song in one mode"), the stored summary row (`RecordStore`, `RecordKey`, `has_record` and the rest), and public Hydra's `records.json` (`docs/differences-from-public-hydra.md`, which keeps it: it names another program's file). After this change only the third survives, as defined above.

## 3. The new layout

Three tables replace `results`. Think of a profile as a label printed once: each record carries a small number pointing at its profile instead of spelling the settings out.

**`profiles`**: one row per profile ever written: an integer id, the chart mode text, the SP cap, the five lens numbers, the results stamp text and the rules fingerprint, unique over all of them. The user's file gives 7 profiles.

**`records`**: one row per chart and profile. Key: the chart's song id and the profile id, as a `WITHOUT ROWID` primary key (SQLite then stores the row inside its key index, so there is no second copy of the key). Columns: the path count, the best path's star count, the chart's note count and `from_summary` (section 6). The note count lives here, not on each path, because it is a fact of the chart under that profile (`Song::note_count`, set once per analysis) and every path shares it. A record with no paths has a path count of 0 and no path rows.

**`record_paths`**: one row per kept path. Key: song id, profile id and rank (1 = best). Columns: the path text, score, activation count, max skip, hardest ms, hardest early fill, average multiplier, squeezes in, squeezes out and the optimal flag. Plain columns, no JSON and no blob, so any SQLite tool can read them and no decoder can drift from the engine (ADR 0026's objection to stored paths).

Rank 1 is the best path. Its row is the summary the library sorts and filters on, so the summary is stored once, not as a copy beside the paths. Today's `bestpath`, `score`, `actcount`, `maxskip`, `hardest_ms`, `avgmult`, `sqin_count` and `sqout_count` columns become the rank-1 row's columns. `pathcount`, `stars` and `notecount` stay on the record, because they describe the record, not one path.

How many paths a record keeps: `kDefaultReportPaths` (5), renamed to say it is the stored count too. The report never shows more than it. The command-line `--all-paths` option still analyzes, because 5 stored paths cannot answer it.

Size. Copying the user's 37,696 results into a lean layout (profile ids, a two-part key, no separate index) took 1,884,160 bytes in a scratch test, about 50 bytes per result, against 8,847,360 today. That test keyed charts by a small integer; keying by the 32-character song id adds about 33 bytes a row, an estimate of about 3 MB. The path rows are estimated, not measured: the stored best path averages 12 characters and a record keeps 4.08 paths on average (both read from the user's file), so about 150 to 200 bytes per record and 3 to 4 MB per profile across 18,848 charts. The plan's first task measures the real size on `testdata/input` and scales it.

## 4. Ready, Stale and Not analyzed

The three statuses keep their meaning (`store::RecordStatus`, decided only by the store).

A chart is **Ready** under the current settings when it has a record under the current profile: the profile whose mode, cap and lens match the settings, whose results stamp this build accepts, and whose rules fingerprint is the rules in force.

A chart is **Stale** when it has no Ready record but has one under a sibling profile: same mode, cap and lens, but another results stamp or other rules. The tooltip names which, as today (`stale_build`, `stale_rules`).

A chart is **Not analyzed** otherwise. A record under another mode, cap or lens does not make a chart Stale; switching to settings never analyzed shows Not analyzed, as today.

Switching back to settings analyzed before shows those records Ready at once, because each profile keeps its records. No limit on profiles (user decision, 2026-10-11).

## 5. Writes and clean-up

The batch and the click both save a record through one function. It takes the analysis and the profile, writes the record and its top paths in one transaction, and replaces the chart's earlier record under the same profile.

The same transaction deletes the chart's records under any profile whose results stamp this build cannot read. That is today's first purge in `write_row`, widened from "this chart and mode" to "this chart", so a leftover like the user's 1.8.2 row goes the next time its chart is saved. Records under other rules but a readable stamp stay, because they read Ready again if the rules come back (D51 call 8).

A profile with no records left is deleted in the same transaction. The scan's purge of charts that left the library (D87 item 4) deletes their records and path rows, and then any emptied profiles.

The click's save also marks an open path report out of date, the same way a finished batch does. The window shows its existing strip, "Your library changed since this report was built", with Refresh.

## 6. The upgrade

The first open by this build turns the old `results` table into the three new tables, as one more step of the store's open sequence, after the existing results-key upgrades (`upgrade_results_key`, `delete_auto_results`); the plan fixes its exact place relative to the summary-only copy upgrade, which runs earlier in the constructor. It runs once: a file that has no `results` table is already done.

Every row whose results stamp this build accepts becomes a record under its profile, rules mismatch included. Rows with an unreadable stamp are dropped; on the user's file that is the 3 rows at 1.8.2 and 2.1.0+allzero. The old summary columns become each record's rank-1 path row, so the library shows the same numbers at once. Ranks 2 to 5 are empty, and the rank-1 row lacks two facts the old summary never held: the hardest early fill and the optimal flag. So each record carries one more column, `from_summary`: 1 when the upgrade converted it from an old summary row, 0 when a save wrote it from an analysis. It can't be worked out from the rows, because a chart with exactly one path looks complete either way. The report fills every record with `from_summary` set, and the fill clears it. One store function answers "does this record still need its paths", and the report and the tests ask that function.

The report then fills those records in (section 7). The work is a table rebuild in one transaction, like the schema 3 and 4 rebuilds; with 37,696 rows it reports progress through the existing `OpenStep` steps if it runs past the startup screen's 0.15 s.

## 7. The path report

The report lists the current profile only (user decision, 2026-10-11). For each library chart with a Ready record it reads the stored path rows. Opening it is a database read.

A record the upgrade converted (every record right after the upgrade, section 6) is filled in: the chart is analyzed once, its top paths saved, and its rows shown. The building bar counts only those charts, in library rows, through the one chart-count owner (section 8), and reads "Saving paths: n of N charts". On the user's library the first open after the upgrade fills about 18,848 charts once (one profile, not today's two modes); later opens fill nothing and show no bar.

The header reads "N charts". The word "records" leaves the page (user decision). The Charts shown tile and the header count through the same owner.

"Left out: N charts whose file couldn't be read" counts library rows like every other chart count (user decision); Show files still lists each file once.

A report where every chart failed to fill now says so, instead of today's wrong "Nothing is analyzed under these settings" (found by the record scout, `generate_report`'s empty branch).

What goes away: the batch's row hand-over (`ReportSeed`, D87 item 5), the per-mode settings rebuild (`settings_for_mode`), and the handoff's part 2 (read each file once per mode). Closing the window still frees its rows (D103 item 28); reopening is cheap now.

Part 4 of the handoff stays: the report window remembers each cell's fitted text until its column width or the rows change, instead of calling `text_ellipsized` for every cell every frame.

## 8. One owner per question: chart and record

Two read-only scouts mapped every place that answers these questions on 2026-10-11 (tasks PRS-scout-chart and PRS-scout-record). The main session checked their central claims in the code: `plan_batch`'s own copy counter, `copies_of`'s fallback of 1, the lowercasing in `normalize_chart_hash`, and the guide's and `CONTEXT.md`'s clashing uses of "record". Each question below names today's homes, the one owner after this change, and what happens to the rest. Each owner gets a single-owner scan rule (ADR 0029) so a second home fails the tests.

**What identifies a chart?** Today the song id has one maker, `song_id_read`, and that stays the owner. But hashes are compared two ways: exactly in SQL, `plan_batch`, `get_summaries`, the library model and the Preview, and lowercased in `library_copies_by_hash`, the report's listing, the leaderboard join, the batch seed and `select_chart`. They agree only because the scan happens to write lowercase. After the change a song id is lowercase wherever Hydra makes one, the upgrade lowercases any stored id that isn't, and every comparison inside Hydra is exact. `normalize_chart_hash` stays only where an outside hash comes in: the leaderboard reply's identifier (`dmbot_client.cpp`). Its other calls go.

**Which charts does the library list, how many copies does each have, and which copy names it?** Today these three facts are worked out in several places. The SQL view `kNamingCopiesSql` groups the charts table. `library_copies`, `naming_copy_paths`, `library_copies_by_hash` and `copies_of` read that view. `plan_batch` keeps its own per-chart counter. `library_lists` answers membership in C++, while the scan's purge (`not_in_library`) answers it in SQL with a different comparison. After the change one owner, a `ChartIndex` built by the store from the library rows in scan order, answers all three. Copies are the number of rows for a song id. The naming copy is the first row. A chart the index doesn't list counts once (D77 item 2, still live on the fill page). The batch plan, every page, the report and the fill comparison ask it. `kNamingCopiesSql`, `library_copies_by_hash`, `library_lists` and `plan_batch`'s counter go. The purge keeps its SQL `NOT EXISTS`, because the database must delete orphaned records itself. It is the one SQL spelling, and it is the only one allowed outside `ChartIndex`.

**How many charts does a set of rows count as?** Today the report header and the Charts shown tile use `charts_counted`. The building bar sums slot copies. The records count sums rank-1 rows times copies. "Left out" counts files. The fill page has `tally_fill_rows` plus a second sum in its page script. After the change one function, `ChartIndex::count`, takes a set of song ids and adds each id's copies once. The header, the tile, the building bar, "Left out", the fill page and the batch's numbers all call it. The fill page script's sum stays pinned to `tally_fill_rows` by its existing test. The library table counts its own rows, which is the same rule, since each row is one copy. The report's bar and its header then count the same thing by construction, so D103 item 25's mismatch can't come back.

**Under which settings is a chart analyzed (its profile)?** Today a result's key is built in five ways:

- `Settings::record_key({}).settings()`, an empty-hash trick
- `BatchRun::filed_under`
- `PageSettings::of`
- a cap forced to 4 by hand in `dm_report.cpp`
- fill rules set by hand on lens copies in `fill_report.cpp`

After the change `Settings::profile()` is the one builder. A page that needs another profile asks for it by name: the leaderboard comparison's Clone Hero cap, and the fill comparison's two fill rules. Nobody edits a cap or lens field by hand. `settings_for_mode` goes, since the report lists one profile.

**Does a chart have a current record (Ready, Stale or Not analyzed)?** Today `get_summaries` decides, and `analyzed_hashes` and `list_records` repeat its candidate query. They all share `WinnerPicker`, but each builds its own SQL. After the change one store function resolves a profile and its siblings and answers the status. The library chips, the batch's skip list, the report's listing and both comparison pages read it. The two spellings of the stamp test, `rank_row` in C++ and `row_readable_sql` in SQL, are built from one list today. They move to the profile table: a profile's stamp is checked once, where profiles are resolved.

**What is a record, and how many are there?** One owner: the `records` table, through the store's save and lookup functions. Nothing a player reads counts records any more. Inside the code, `counts()` becomes "how many records does the file hold", read only by `holds_results` ("does it hold any").

**Which paths does a record keep, and in what order?** Today `HydraRecord::all_paths()` gives the order, `chart_rows` numbers the ranks, and `summarize_record` reads `best_path()` (`paths.at(0)`), which is the same path by traversal rather than by a stated rule. After the change one function in the store turns an analysis into a record and its path rows, walking `all_paths()` and ranking from 1. The summary is rank 1 of that walk, not a second reading through `best_path()`. `chart_rows` and `summarize_record` fold into it. The report reads rows back; it never ranks.

**Does this record still need its paths?** One store function reads the record's `from_summary` column (section 6).

## 9. One owner per question: optimal, squeezes, scoring

Four more read-only scouts swept these questions on 2026-10-11 (tasks DRIFT-optimal, DRIFT-squeeze, DRIFT-score and DRIFT-breakdown). They started from the concept audit's findings (`docs/audit/2026-10-11-concept-audit/`) and checked that D114's fixes landed: M30, M34, M41 and M43 did. The main session checked each remaining claim below in the code.

The short answer is good news. Each of these questions already has one owner that computes it, and every screen calls it. What is left falls into three kinds: code that repeats an owner's work, one word with several meanings, and labels or text that say something different from the number beside them. This change fixes all three. Each owner below gets a single-owner scan rule where it has none.

### What is optimal?

The owner is `HydraRecord::is_optimal`: a path is optimal when it ties the top score, so tied variants are optimal too. The report's flag, the Paths tab's gold group and the Preview's "(optimal)" label all call it. The all-0 rule's owners are `allzero_activation` and `allzero_options`, and nothing else spells them.

The one repeat: the Paths tab works out the top score itself (`path_view.cpp`, `record.paths.empty() ? 0 : record.best_path().totalscore()`). It moves to a new `HydraRecord::optimal_score()`, empty for a record with no paths, which `is_optimal` also reads. The all-0 button's "N above optimal" branch can never fire, because the engine sweep test pins that no all-0 path beats the optimum, so it goes.

"Best path" means the single top path everywhere except one checkbox. The report's "Best path only" box keeps every optimal path, so it is renamed "Optimal paths only" (user decision, 2026-10-11). The guide's line about it gains "of the paths shown", because the report keeps 5 paths per chart. `CONTEXT.md` gets an "Optimal path" entry. A stale comment in `report_window.cpp` that points at a stylesheet rule no longer in `html_page.cpp` is fixed.

### What is a squeeze, and what is a backend?

Every number here has one owner, and all callers call it:

- squeeze kinds: `SqueezeKind`
- the 500 ms window: `within_squeeze_window`
- which phrase can be squeezed: `core/sqout_chord.h`
- late or early: `after_sp_end`
- a backend row's value and the leeway: `core/backend_value.h`
- "needs no timing": `SPSqueeze::needs_no_timing`
- a squeeze's difficulty: `squeeze_difficulty` and `hardest_part`
- the timing tiers: `timing_tiers`
- the backend row ladder: `BackendSqueeze::summarystr`
- the backend limit filter: `build_activations`

The engine's path-limit walk (`Engine::act_within_limit`) repeats `Activation::within_ms_limit`'s loop over the engine's flat arrays. Both call the same two leaf tests. It stays in its own form, because the engine has no `Activation` to hand over. A new test feeds both the same activations and pins that they agree.

The report's Timing chip said "No squeezes" for a path whose squeezes all need no timing, next to SqIn and SqOut columns that count every squeeze in the path text. The chip and its filter entry now read "No timing needed" (user decision). The columns keep counting every `+` and `-`.

The Path limit tooltip and the guide said the limit applies to "the hardest squeeze or required early fill". The search tests every squeeze by its own figure, including ones that need no timing, so the two differ for a negative limit. The tooltip and the guide are reworded to say what the search does (user decision). No behaviour changes.

Code comments that say "free squeeze" for "needs no timing" are reworded. On screen, "Free" stays the Rating ladder's word. In public text, "backend" means a scoring effect, a table row and a squeeze. `CONTEXT.md` gets a "Backend row" entry: a chord within the squeeze window of an SP end, as the Backend timings table lists it. The guide uses that term for rows. The guide's "double squeeze", a nickname nothing in the code uses, is replaced with the glossary's terms.

### What is a note worth, and how is a score computed?

Each fact has one owner, and all callers call it:

- a note's 1x value: `ChordNote::basescore`
- the multiplier steps: `to_multiplier`
- Star Power's doubling: `kStarPowerMultiplier`
- the six-category split: `category_scores`
- the solo bonus: `solo_bonus`
- the one chart walk: `chord_score_table`
- the six-category total: `score_total`
- the chart's 1x total: `Path::chart_base_score`
- the star cutoffs: `star_cutoff` and `path_stars`

The engine, the replay, the Preview, the Stars tab, the store, `hydra_replay` and the leaderboard comparison all read these.

There are four repeats. First, the engine adds Star Power points to the SP category and to the running score by hand, in two places (`engine.cpp`: the frontend points and `sp_delta`). One engine helper now adds to both. Second, "does this chord cross a multiplier step" is written in `MultSqueeze::applies` and again in `category_scores`, which reports the same step pair. One function in `core/timing` answers it, and both call it. Third, "cymbals count only under Pro Drums" is written once in each chart reader (`song.cpp`, the MIDI and .chart paths). One parse function answers it, and both readers call it. Fourth, comments that restate "50 a gem, 65 a cymbal, doubled" (`model.h`) are reworded to point at `ChordNote::basescore`.

The word "base score" names three numbers in the code: a note's 1x value, the Notes category, and the chart's 1x total. "Points" and "sp" each name two or three. The code is renamed so each name has one meaning:

- the Notes category becomes `notes_points`
- the chart's 1x total stays `chart_base_score`, the Stars tab's "Base score"
- a note's value stays `basescore`, the name Clone Hero's code uses

The plan lists each rename.

The guide and `CONTEXT.md` copy the star factors, "10 notes" and "15 points" as bare numbers. Each copy gets a docs-match marker that `tests/test_docs_match_code.cpp` checks against its owner (`kStarMultipliers`, `to_multiplier`, `kCymbalBonusPoints`), or is removed from the prose.

### The average multiplier and the score breakdowns

The owners are `Path::avg_mult` (the path's score without solo over the chart's 1x total), `category_scores` for the six breakdown lines, and `build_score_breakdown` for the Paths tab's text. Nothing computes them a second time.

The stored average multiplier had no reader, because the report worked it out again from the analysis. In the new layout the report reads it from `record_paths`, so the stored value is the one the page shows. It is stored unrounded. `format_avg_mult` is the one place it is rounded, and the report stops calling `py_round3` itself. Two comments that count its callers or name "the Python row" are reworded to point at `format_avg_mult`. The unused sort keys `SortColumn::AvgMult` and `SortColumn::NoteCount` go.

The label is spelled three ways today: "Avg. Multiplier:" on the Paths tab, "Avg multiplier" in the report, and "Multiplier" on the Stars tab, where the column holds the star cutoff factors. A star's cutoff is the chart's 1x total times its factor, so the factor is the average multiplier that star needs. All three now read "Average Multiplier", from one label constant (user decision). The Stars column's tooltip says it is the average multiplier that star needs.

"Notes" meant points on the Paths tab and a note count in the report and the fill page. The count columns become "Note count" (user decision). The Paths tab keeps "Notes", the word Clone Hero's results screen uses for that line. The guide's Score breakdown paragraph gains one sentence: Notes + Accent Notes + Ghost Notes is the Base score on the Stars tab, the Accent and Ghost lines show only their extra 50, and a cymbal's extra 15 stays in Notes. That fulfils what D114 item 16 assumed the guide already said.

### Ticks, tempos, sections, time signatures and measures

A sixth scout (task DRIFT-timing, 2026-10-11) traced the Preview's info box, which in the user's screenshot read "m28.32.109 of m88.2.103", "BPM 126.000 · 36/4" and "Section chorus_1a". The main session checked its central claims in the code.

All three lines come from one function, `build_time_box`. The "36/4" is what the chart states. Both readers store a time signature through `apply_timesig`, the numerator as written and the bottom number from `timesig_denominator`. A misread can't produce a numerator of 36. Charts in `testdata/input` use 15/8 and 14/8 too. In "m28.32.109", the measure and beat count from 1 and the tick from 0 (`format_measure`). The beat is a quarter note in every meter (`Timecode` divides by the chart's resolution). So beat 32 fits a 36/4 measure. The tick is in the chart's own resolution.

These owners stand, and every caller calls them:

- ticks to ms and back: `MsIndex` (`at`, `tick_at_ms`)
- the screen's rounded tick: `SongTiming::display_tick_at_ms`
- a tick's measure, beat and tick: `Timecode`, over `MeasureIndex`
- position text: `format_measure` and `measure_label`
- fractional measures: `measures_at_tick_f` and `tick_at_measures_f`
- the engine's measure steps: `plusmeasure`
- an SP bar's length: `kMeasuresPerSpBar` and `sp_bars_to_measures`
- storing a time signature: `apply_timesig`

What changes:

- **"In force at this tick" gets one owner.** The info box finds the tempo, time signature and section with three hand-written searches over its own copies of `Song`'s lists. `SongTiming` gains one "in force at tick" lookup for each, and the info box and the drain box call them. The time signature lookup goes through the same `MeasureIndex` answer the position text uses. So at the exact tick of a mid-measure change the label and the position can't disagree (user decision, 2026-10-11). Which side of the change that tick belongs to is decided once, with the meter-walk answer below.
- **The bar-line grid and the position text share one walk.** `build_beat_events` and `Timecode` both walk `MeasureIndex`'s arrays to the same grid. One grid walker serves both. The comment that says a mid-measure change "restarts the measure" is wrong (the code finishes the measure in the new length), and it is reworded.
- **The meter walk in `check_activations`.** It finds the time signature for fill placement by its own walk, one change per chord, not by `MeasureIndex`. The two differ when two or more changes fall between two chords. That moves an auto-generated fill, so scores can move. D26 kept the walk to match the old Python Hydra, not Clone Hero. The user chose to check Clone Hero's own rule first (ADR 0023) and count the `testdata/input` charts where the two differ (scout CH-meter-sections). Whichever rule Clone Hero uses becomes the one owner; the other goes. If scores move, the results stamp is bumped (ADR 0018), and the user sees the count and the charts first. The same scout checks whether Clone Hero's fill "beat" is a quarter note in 6/8, which `CONTEXT.md` marks unverified.
- **Section names read as Clone Hero shows them.** A MIDI chart's `prc_chorus_1a` shows as Clone Hero's own title for it, from the table or rule the scout finds in Clone Hero's code. A .chart's `[section Chorus 1A]` stays as written (user decision). One function turns a section event into its shown name, and the parsers stop stripping prefixes on their own (`section_name_of` and the .chart reader each do it today).
- **`hydra_replay`'s dump prints positions as the screen does.** It printed raw measure and beat counts from 0, so it said measure 27 where the screen said m28. It now prints through `format_measure` (user decision). `check_activations`, which adds 1 by hand, also goes through the owner. Pinned measure numbers in `tools/check_refs.py` cases and the FC video workflow are updated.
- **Words.** "Section" means a song part: the info box line, `Song::practice_sections`. Solo sections keep their full name. The tempo and meter segments in `MeasureIndex`, `MsIndex` and `PreviewMeter`, which code calls "sections", are renamed "spans". "Bar" means an SP bar (two measures, `kMeasuresPerSpBar`). The line drawn at a measure's start is a "measure line" in code and in the guide (`PreviewBeatKind::Bar` is renamed). The guide gains one sentence each saying what a tick is (a step of the chart's own resolution, so its length depends on the chart), that a beat is a quarter note, and that one SP bar is two measures. "Tick the box" becomes "check the box". `CONTEXT.md` gets Tick, Tempo, Time signature, Measure, Beat, Section, Span and SP bar entries, each naming its owner.
- **Outside the app.** `tools/ch_probe/probe_chart.py` writes its own tick-to-ms formula in Python, because it can't call the C++ owner. It gets a comment naming `MsIndex` as the rule it mirrors, plus a test that checks one value against `hydra_replay`.

### Left as they are

The Preview's two answers to "is SP running" are intentional and documented: the playhead drives the meter, and each chord's own flag drives its doubled value. The fill comparison's "same, higher or lower" is a different question from optimal. The guide's prose definition of the average multiplier matches the report's tooltip word for word. The tooltip is its owner in the app, and the guide is the public copy.

"When does the last note start" (concept audit M49) stays answered twice: the Preview and the song length ask it of different songs on purpose in some modes. "Is SP running at this moment" (M48) is different. It is one rule, start included and end left out, spelled twice: by `running_activation` and by the highway's span sweep. The audit left it alone because nothing touched it; this change gives it one owner, and both call it. "of m88.2.103" is where playback ends, the later of the last note and the audio's end, not the song's stated length. D114 item 18 kept that, and the guide explains it.

## 10. Other pages and tools

The leaderboard comparison and the fill comparison read one profile each through the same lookup the library uses. `tools/compare_db.py` learns the three tables. `tools/bench.cpp` saves through the one save function.

## 11. Docs and decisions

A new ADR, 0030, records the stored records and paths and supersedes ADR 0026 in part. ADR 0018's results stamp is not bumped: the engine's output does not change, and a converted record that still needs its paths is told apart by its `from_summary` column (section 6), not by a stamp. The decision is D115 in `docs/audit/2026-10-03-fix-decisions.md`.

`CONTEXT.md` gets the five words from section 2 and drops "Summary row" and "Result identity". `docs/UserGuide.md`'s path report section loses its record wording, its building-bar sentence and "Hydra Deluxe keeps only each song's summary". Its "no records are stored yet" sentence is checked against the real text (`kNoRecords` reads differently today). Public pages go through the doc review.

## 12. Testing

Everything runs on `testdata/input` or a few charts. No library runs.

Report rows read from the store equal rows from a fresh analysis, compared row by row, with the D87 rule that tied-score rows may change order. Store tests cover the upgrade (which rows survive, rank-1 rows equal the old summary columns), the three statuses with sibling profiles, the widened dead-row purge, emptied profiles and the scan purge. The single-owner scan (ADR 0029) gets one rule per question in sections 8 and 9, so a second home fails the build's tests.

Before merge the user sees the upgraded library and the report in the real app on a copy of their database (memory `ui-changes-need-real-app-check`). That check fills every record once, about 18,848 analyses: a full library run, asked for in chat first (CLAUDE.md rule 8).

## 13. Not in this change

Each item below needs either a separate decision or a measurement first. Each is proposed as its own session, so this change doesn't wait for them.

The library heading and the Analyze button used to count the same search two ways. The heading's "S of N charts" applies the status chip; the "Analyze search (N)" button and the batch it starts did not. The user has now decided this, in its own session ("Library heading vs Analyze button count", 2026-10-11): the button follows the chip. `LibraryModel::order()` becomes the one owner of the rows on screen. The heading, the button's count and the batch's rows all read it, and `matches()` and `library_match_count()` go. The button reads "Analyze shown (N)..." whenever a search or a chip narrows the table, and "Analyze library..." otherwise. That label is recommended and waits on the user's yes. The guide's two lines about it go through the doc review. It touches only the library's model, toolbar and their tests, none of the files the rest of this change touches, so the plan builds it as its own task alongside the others. The design is in that session's note, `analyze-button-follows-chip.md`.

The squeeze-out cost was checked and closed in its own session ("Measure the squeeze-out cost's two figures", 2026-10-11). "It scores N fewer points" describes the squeezed note itself, which loses exactly N. `hydra_replay`'s larger figure answers a different question and never shows beside it. The user ruled it not an issue, and nothing changes. The measurement is in `sqout-cost-gap-measurement.md` in the shared notes folder.

The Dynamics tab counts from its own parse, with 2x Bass always on. So with 2x Bass off, its totals can differ from the ghost and accent counts the score implies. That is the open user question in the 2026-10-03 derivation audit memory.

An all-0 path that ties the optimal score, but is a different path from the listed ones, shows under "Best all-0 path" without gold. It is rare, and it is not a wrong number, but it is optimal. Whether it should say so is a display question.
