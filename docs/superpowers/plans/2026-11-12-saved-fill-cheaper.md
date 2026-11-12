# The saved-figure fill: cheaper, or gone

Plan of 2026-11-12. Task id `plan-fill-cheaper`. Nothing here is built yet, and nothing starts before the user says yes.

## The plain version

The brief asked for a cheaper way to fill the "hardest" figure on old saved paths when a library opens. I looked for one and found two things.

First, a cheaper way does exist. The reader that turns saved path text back into a path builds the chart's search graph again for every fill it tries. (The search graph is the engine's map of the chart; building it is the costly part the stack samples named.) The engine already has a search that takes a graph built once. The reader just does not use it.

Second, and more important: on main today the fill has no row it can ever reach. It only touches saved results this build reads as current. The user ruled on 2026-11-08 to bump the results stamp for the coda fix ("Bump the stamp"), so every result saved before that reads as out of date. Every build that writes the new stamp already saves the figure. So no current row can be missing it. On the user's library the fill finds nothing and returns at once. The next release will not run it on first open.

That also explains the timings. The 22 seconds on 2026-11-11 came from a program built before the stamp change. The 0.7 seconds today came from main's code, where the fill does nothing.

So the best real option is to remove the fill, not to speed it up. That is the user's call, because the user chose the fill on 2026-10-31 ("Yes, first open"). Question 1 asks it. If the user keeps the fill, the plan makes it cheaper with the graph built once per chart. The reader change also speeds up pasting a path into the Path Builder, so Question 2 asks whether to do it either way.

## What I checked and how

**Can the figure come from stored data?** No. A saved path row holds its text, score, activation count, largest skip, the hardest figure, the hardest early fill, the average multiplier, two squeeze counts and the optimal flag (`record_paths` in `src/store/record_store.cpp`, lines 261 to 275). It holds no per-squeeze timing. The figure is the largest timing over every squeeze and required early fill, so it needs the activations back.

**Can one replay stand in for many searches?** Partly. The path text carries no ticks, so each window still has to be placed by trying fills in chart order (`place_next` in `src/search/pathstring_read.cpp`). Each try calls `search_target(song, settings, windows)`, which builds a new graph every time (`src/search/pather.cpp`, lines 401 to 404). The twin `search_target(graph, windows)` takes a graph already built, and its header comment says it is "for a caller that runs many searches on one chart" (`src/search/pather.h`, lines 155 to 158). Using it keeps every search the same and drops every graph build but one. I could not measure the saving, because agents do not build. Task T4 measures it.

**Running one full analysis per chart instead** would give the paths back too, but a full analysis costs far more than pinned searches. It is not cheaper.

**The null rows.** I copied `C:\Program Files\Hydra\hydra.db` with SQLite's backup, through a read-only connection, to `C:\Users\Patrick\AppData\Local\Temp\claude\b8c40ac9d5b73501\c8b70230-d47d-4a13-ba2e-f4c7048d9ab4\scratchpad\fillplan\installed-copy.db`, and read only the copy. It holds 68,743 path rows, 58,379 with no figure. Of those, 43,609 have no squeeze and no required early fill, so they have no figure to save. The fill skips every row with no squeeze-in on purpose (`rows_missing_hardest`, `sqin_count > 0`). Only 13,991 empty rows have a squeeze-in, across about 5,800 charts. So the count of 58,374 could never reach zero: at most 13,991 rows would move.

**Do the writes land?** Yes. Two snapshot folders left from 2026-11-11 (`C:\Users\Patrick\AppData\Local\Temp\hydra_test_uitest_25372` and `hydra_test_uitest_46364`) each hold a 4 MB write-ahead file beside the database. (The write-ahead file is where SQLite keeps new rows before folding them into the main file.) Read through private copies with that file, they show 6,032 and 3,352 figures written, among them 4,438 and 2,459 negative figures that no older build ever saved. Neither holds the "done" marker, so both fills were stopped part way. A count that leaves the write-ahead file out misses some of those writes: the first folder's main file alone shows 53,774 empty rows, where the full read shows 52,342. Three other folders from that night (`hydra_live_13964_1`, `hydra_live_34524_1`, `hydra_live_39080_1` under `Temp\claude`) show no write at all and the unchanged 58,374.

**The marker.** The installed database's `meta` table holds two rows, `auto_results_deleted` and `chart_meta_version`. It has no `hardest_ms_is_engine_figure` row. The marker is not set. The installed program is version 2.14.0 and has no fill, so nothing could have set it.

**Why 0.7 seconds today.** All 26 profile rows in the installed database carry the stamp `2.12.1+ch10grid`. Main accepts only `2.14.1+coda` (`src/store/stored_versions.h`, line 55), since merge `5fe26cb8` on 2026-11-08 at 22:11. So no profile row is Ready, `hardest_figure_filled` answers true ("nothing to fill"), and the fill returns before it reads a chart. The programs in `build-cpp\Release` that open a window (`Hydra.exe`, `hydra_uitest.exe`) are dated 2026-11-08 17:28, before that merge. A window started from them still accepts the old stamp and runs the full fill. That is the 22 seconds. Which program the 2026-11-11 run used is reported to me, not read by me, so it is premise P11.

## Prior art

I searched the web for lazy backfill and for removing one-time migration code.

The common advice is to keep a backfill out of startup and run it as a background job in bounded batches that can resume ([Schema changes and backfills, SQD docs](https://docs.sqd.dev/en/cloud/resources/schema-changes-and-backfills); [Zero-downtime schema migrations](https://alivedise.github.io/backend-engineering-essentials/distributed-systems/zero-downtime-schema-migrations)). That is the "run it off the open path" option. A lazy backfill should only fill missing values, which is what `set_hardest` already does ([Lazy backfill, DEV](https://dev.to/canceylan1988/lazy-backfill-roll-out-new-data-shapes-without-a-migration-script-16g5)).

On removal, projects delete one-time migration code once no data can need it. The strongest basis is a row-level check, not elapsed time ([Don't let old migrations haunt your codebase, Lincoln Loop](https://lincolnloop.com/blog/dont-let-old-migrations-haunt-your-codebase/); [Delete your old migrations, Julik Tarkhanov](https://blog.julik.nl/2025/10/delete-your-old-migrations)). Task T1 is that row-level check, written as a test. I found nothing specific to desktop apps with a results cache keyed by a version stamp.

## What the user will see

Nothing changes on screen but one startup line that can no longer appear.

Three places read the saved figure: the Library's Hardest ms value and its `squeeze` search (`src/ui/library_model.cpp` line 46, `src/app/library_query.cpp` lines 420 to 432), and the report's Hardest ms column and summary (`src/app/report.cpp` lines 61 and 264). All three read only Ready results. An out-of-date result carries no record at all (`lookup` in `src/store/record_store.cpp`, lines 1322 to 1325), so its figure is never shown.

Before and after, on a real chart from the corpus: "Smash Mouth - I'm A Believer", best path `1 0+ 0 0`, shows Hardest ms 0.0. That number is pinned in `tests/test_saved_figure_fill.cpp` line 107. A fresh analysis saves it directly, so it reads 0.0 with or without the fill.

Before and after, on the user's library: all 19,066 saved results read as out of date in the next release, because of the 2026-11-08 stamp ruling. They show no figure until they are analysed again. The fill never changed that, and removing it does not either.

The one visible difference under Question 1's first answer: the startup line `Reading saved paths...` is gone. Today it shows only when the fill has a chart to read, which on main is never.

Under Question 1's second answer (keep the fill, make it cheaper), nothing visible changes at all. Under Question 2, pasting a path into the Path Builder gets faster and reads the same path.

## Tasks

The tasks are one branch, in order, because T1, T2 and T3 share `CMakeLists.txt` and the fill's files. One executor does them. The main session builds and runs every test named here (agent rule 14).

### T1: Pin the two facts the decision stands on

Owns `tests/test_saved_figure_fill.cpp` and, only if it needs a new helper, `tests/store_fixtures.h`. No source file changes.

This task adds two test cases. Both should pass on main as it is, because they pin facts that are already true. They cannot be written failing-first for that reason. The reviewer reads each one and says what would make it fail.

The first case pins "a result this build saves is never missing its figure". It analyses the corpus charts the file already uses, saves each through the store's normal save, and checks two things through the store's own calls: every saved path with a squeeze-in holds a figure, and `rows_missing_hardest` returns none. It fails if `path_row_of` stops saving `engine_hardest_ms`, or saves it empty for a path with a squeeze-in. This case uses premises P3 and P4.

The second case pins "rows under an older stamp are left alone". It saves a result under a profile row whose stamp is not the current one, with an empty figure and a squeeze-in. Then it runs `fill_saved_hardest` with a counting progress callback and a counting replay. It checks the fill returns true, the callback and the replay are never called, and the row is unchanged. It fails if the fill ever reads a row that is not Ready. If the file already holds a case that proves exactly this, the executor names it in the commit message and adds nothing. This case uses premise P5. It also settles P10 and P11: it shows the mechanism behind the 0.7 seconds without leaning on the reported numbers.

The executor also reads `convert_results` (`src/store/record_store.cpp`, line 866 on) and says in its report whether a converted older file can end up with a current-stamp row that has an empty figure. That settles P6. If it can, the executor stops and reports; Question 1's first answer is then off the table.

Acceptance: both cases are in the file. The main session runs `hydra_tests.exe "-tc=*saved figure*,*older stamp*"` (the executor gives the exact names) and both pass.

### T2: Remove the fill (only if Question 1 is answered "Remove it")

Owns `src/app/saved_figure_fill.h`, `src/app/saved_figure_fill.cpp`, `tests/test_saved_figure_fill.cpp`, `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`, `tests/test_library_jobs.cpp`, `tests/test_store_open.cpp`, `tests/store_fixtures.h`, `src/store/record_store.h`, `src/store/record_store.cpp`, `src/core/model.h` (one comment), `CMakeLists.txt`, plus any file that shows the startup step's label, which the executor finds by grepping for `ReadingSavedPaths` and `kReadingSavedPathsLabel` and names in its report before it edits.

The fill's two files go. The open job stops calling it, and its `ReadingSavedPaths` step and label go with it. The store loses the four calls only the fill used (`rows_missing_hardest`, `set_hardest`, `hardest_figure_filled`, `mark_hardest_figure_filled`), the two small structs they pass, and `ready_profile_ids` if nothing else calls it. The comment on `engine_hardest_ms` in `model.h` says it has two callers; it then has one. The fill's tests go, and so does T1's second case, whose job is done once the main session has run it green. T1's first case stays, moved to a store test file if `test_saved_figure_fill.cpp` is deleted: it is the guard that makes removal safe.

`paths_from_pathstring` stays. The Path Builder reads pasted path text with it (`src/pathbuilder/pb_files.cpp`, line 35).

A database that already holds a `hardest_ms_is_engine_figure` marker row keeps it. Nothing reads it, and it does no harm. No code is added to delete it.

Acceptance, each with its check:

1. Nothing in `src/` or `tests/` still names the fill. The executor's grep for `fill_saved_hardest`, `saved_figure_fill`, `ReadingSavedPaths`, `rows_missing_hardest`, `hardest_figure_filled`, `set_hardest` and `MissingHardestRow` over both folders returns nothing, and the report shows the command and its empty result.
2. The open still works and still reports its other steps. The main session runs the open job's and store-open tests that remain: `hydra_tests.exe "-tc=*StoreOpenJob*,*store open*"` (the executor gives the exact names left in `test_library_jobs.cpp` and `test_store_open.cpp`).
3. T1's first case still passes after the move.
4. The merge's own test list comes from `hydra_precheck.exe --tests` on the branch.

### T3: Say so in the docs (only if T2 runs)

Owns `docs/adr/0018-stored-data-has-one-set-of-version-stamps.md`, `docs/audit/2026-10-03-fix-decisions.md` and `docs/agents/this-machine.md`.

ADR 0018 (line 94) and D302 item 6 in the decisions file (line 1687) both say `app::fill_saved_hardest` refills old rows. Each gets a dated note under the existing text: the fill was removed, why no row can need it, and the user's answer to Question 1 in the user's exact words. The existing words stay; D302 is a record. The main session gives the note its D-number. `this-machine.md` line 61 names the fill in its stack-sampling example; the executor rewords it only if the example no longer makes sense.

Acceptance: the docs check passes. The main session runs `hydra_tests.exe "-tc=docs match code*"`.

### T4: The reader builds the graph once (if Question 1 is "Keep it", or Question 2 is "Yes")

Owns `src/search/pathstring_read.h`, `src/search/pathstring_read.cpp` and `tests/test_pathstring_read.cpp`. If the fill is kept, it also owns `src/app/saved_figure_fill.h`, `src/app/saved_figure_fill.cpp` and `tests/test_saved_figure_fill.cpp`.

The reader gets a second entry point that takes a graph already built, beside the song. The entry point that exists today builds the graph once with `search_graph` and calls the new one. Inside, every search goes through `search_target(graph, windows)`. No search changes, no rule moves, and `engine_hardest_ms` stays the only place the figure is worked out. The graph is only read after it is built (`run_search` takes it `const`), so the fill's workers can share nothing and still each build one per chart.

If the fill is kept, it builds one graph per chart with `search_graph` and passes it to every row's replay. The `PathReplay` test seam takes the graph too.

Tests, written first. The executor cannot run them red; the reviewer reads them and says what would make each fail.

1. Same answers. Every existing case in `tests/test_pathstring_read.cpp` stays as it is and must still pass: they pin path text, scores and refusals from real runs. One new case reads each corpus path text through both entry points and checks `path_identity` and `engine_hardest_ms` agree. It fails if the graph entry point reads a different path.
2. One graph per chart (fill kept only). A new case in `tests/test_saved_figure_fill.cpp` passes a replay that records the address of the graph it is given. With several rows of one chart, every call must see the same graph. It fails if the fill builds a graph per row.
3. The fill's figures do not move (fill kept only). Every figure pinned in `tests/test_saved_figure_fill.cpp` today stays pinned and must still pass.

Acceptance: the main session runs `hydra_tests.exe "-tc=*path text*,*pathstring*,*saved*figure*"` (the executor gives the exact names) and all pass. Then it times the change: the fill's test over the corpus charts in `testdata/input`, before and after, with doctest's `-d` duration flag, one run each on a quiet machine, and writes both numbers in the merge message. That settles P9. No whole-library run.

## Out of scope, for the user to approve

Running the fill in the background after the library shows. It is the standard answer when a backfill must run, but here no row needs the backfill. If the user keeps the fill and T4's timing is still too slow, that becomes its own plan, with its own question: it would let the Library show an empty figure that fills in later.

Rebuilding the stale `Hydra.exe` and `hydra_uitest.exe` in `build-cpp\Release`. They predate the stamp change, so a window started from them does not show main's behaviour. That is the main session's build to make.

Deleting leftover marker rows from existing databases.

## Questions for the user

1. **Remove the fill, or keep it and make it cheaper?** The real rule is in `rows_missing_hardest`: the fill reads "every Ready path row with no saved figure and at least one squeeze-in, whose chart the library lists". Ready means the row's profile carries the stamp this build accepts, which is `2.14.1+coda` alone. From one end: the user's 13,991 rows with a squeeze-in and no figure all carry `2.12.1+ch10grid`. They are out of date, the fill skips them, and no screen shows them until they are analysed again. From the other end: a chart analysed by the next release is saved with its figure already in place (for example "I'm A Believer", `1 0+ 0 0`, 0.0), so the fill skips it too. No row sits between the two. I recommend **Remove it** (T1, T2, T3): about 500 lines and one startup step that can never do anything. This reverses what was built from the user's 2026-10-31 answer "Yes, first open", which the 2026-11-08 answer "Bump the stamp" has since made empty. The other answer is **Keep it, make it cheaper** (T1, T4): the fill stays as a safety net that today has nothing to catch.

2. **Make the path-text reader build its graph once, even if the fill is removed?** The same reader runs when a path is pasted into the Path Builder. Today it builds the graph once per fill it tries; after, once per paste. It reads the same path either way. I recommend **Yes** (T4, reader files only): the change is small and the tests pin the same answers. **No** leaves the Path Builder as it is.

## Promise map

| Promise | Where in the plan | Task | The check that proves it |
|---|---|---|---|
| A result this build saves is never missing its figure when it has a squeeze-in | T1, first case | T1 | The new case in `tests/test_saved_figure_fill.cpp`; fails if `path_row_of` stops saving `engine_hardest_ms` |
| The fill never reads a row that is not Ready | T1, second case | T1 | The new case with a counting callback and replay under an older stamp; fails on any call |
| A converted older file cannot hold a current-stamp row with an empty figure, or the executor stops and reports | T1, last paragraph | T1 | The executor's read of `convert_results`, stated in its report; the reviewer opens the same lines |
| After removal, nothing in `src/` or `tests/` names the fill | T2, acceptance 1 | T2 | The grep and its empty result in the executor's report |
| The open still works and reports its other steps | T2, acceptance 2 | T2 | `hydra_tests.exe "-tc=*StoreOpenJob*,*store open*"` |
| The guard case survives the removal | T2, acceptance 3 | T2 | T1's first case passes after the move |
| `paths_from_pathstring` stays for the Path Builder | T2, third paragraph | T2 | The existing cases in `tests/test_pathstring_read.cpp` still pass (T2's precheck list) |
| No code is added to delete marker rows | T2, fourth paragraph | T2 | The reviewer reads the diff: it holds no write to `meta` |
| The docs name the removal and keep D302's words | T3 | T3 | `hydra_tests.exe "-tc=docs match code*"`; the reviewer reads the two notes |
| The graph entry point reads the same path as today's | T4, test 1 | T4 | The new both-entry-points case and every existing reader case |
| The figure is worked out only by `engine_hardest_ms` | T4, second paragraph | T4 | The reviewer greps the diff for `difficulty()` and `e_difficulty()`: no new caller |
| The kept fill builds one graph per chart | T4, test 2 | T4 | The graph-address case; fails on a graph per row |
| The kept fill's figures do not move | T4, test 3 | T4 | Every figure already pinned in `tests/test_saved_figure_fill.cpp` |
| The saving is measured on the corpus, never the whole library | T4, acceptance | T4 | The two `-d` timings in the merge message |
| No visible change the user did not approve | "What the user will see"; Questions 1 and 2 | T2, T4 | The startup line goes only on the user's "Remove it"; the reviewer checks no other UI text changed |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The reader builds a new graph for every search, because it calls the song form of `search_target` | code | `src/search/pathstring_read.cpp:87` and `:185`; `src/search/pather.cpp:401` | checked 2026-11-12 |
| P2: A graph form of `search_target` exists, runs the same search, and the graph is only read after it is built | code | `src/search/pather.h:155`; `src/search/pather.cpp:406`; `src/search/graph.cpp:103`; `src/search/engine.h:71` | checked 2026-11-12 |
| P3: `path_row_of` is where a path's saved figure is set, from `engine_hardest_ms`, which returns a value whenever the path has any squeeze | code | `src/store/record_store.cpp:565`; `src/core/model.h:899` | checked 2026-11-12 |
| P4: The fill reads only rows with no figure and a squeeze-in, under Ready profile rows | code | `src/store/record_store.cpp:1436` and `:1460` | checked 2026-11-12 |
| P5: Ready needs the stamp this build accepts, and main accepts `2.14.1+coda` alone; with no Ready profile row the fill returns true at once | code | `src/store/stored_versions.h:55`; `src/store/record_store.cpp:431` and `:1509`; `src/app/saved_figure_fill.cpp:114` | checked 2026-11-12 |
| P6: No older file can be converted into a current-stamp row with an empty figure | unchecked | `src/store/record_store.cpp:866` keeps each old row's own stamp, but I did not read the whole function | T1 |
| P7: The user ruled the stamp bump | ruling | `docs/agents/rulings/clone-hero.md:14`: "Bump the stamp" | checked 2026-11-12 |
| P8: The user chose the first-open fill | ruling | `docs/sessions/2026-10-31-711858a7.md:36`: "Yes, first open"; D302 item 6 in `docs/audit/2026-10-03-fix-decisions.md:1687` records the same answer | checked 2026-11-12 |
| P9: Building the graph once makes the reader much faster | unchecked | `src/search/pather.cpp:261` shows what a build does; no timing exists | T4 |
| P10: Today's windows built from main started in 0.7 s | unchecked | the brief for `plan-fill-cheaper` reports it; I opened no run folder | T1 |
| P11: The 22 s on 2026-11-11 came from a program built before the stamp change | unchecked | `build-cpp/Release/Hydra.exe` is dated 2026-11-08 17:28 and merge `5fe26cb8` is 22:11; which program ran is only reported | T1 |
| P12: The installed database has no fill marker, 26 profile rows all stamped `2.12.1+ch10grid`, and 13,991 of its 58,379 empty rows have a squeeze-in | data | `C:\Program Files\Hydra\hydra.db`, read through a backup copy | checked 2026-11-12 |
| P13: The fill's writes land in the write-ahead file, and a stopped fill leaves no marker | data | `C:\Users\Patrick\AppData\Local\Temp\hydra_test_uitest_25372\hydra.db` with its `-wal`, read through a private copy | checked 2026-11-12 |
| P14: An out-of-date result carries no record, so no screen shows its figure | code | `src/store/record_store.cpp:1322` | checked 2026-11-12 |
| P15: A saved path row holds no per-squeeze timing | code | `src/store/record_store.cpp:261` | checked 2026-11-12 |
| P16: The Path Builder reads pasted path text with the same reader | code | `src/pathbuilder/pb_files.cpp:35` | checked 2026-11-12 |
| P17: The stamp change reached main before any build could write `2.14.1+coda` with an empty figure: the save of the engine's figure (`d43abafa`) is an ancestor of the bump (`cf3100ce`) | code | `git merge-base --is-ancestor d43abafa cf3100ce` in the repo at `src/store/stored_versions.h` | checked 2026-11-12 |

This plan has no live steps.
