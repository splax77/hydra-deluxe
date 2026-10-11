# B1 audit: storage, library, app state and file-handling tests

Scout: TA-B1. Read only; nothing was edited, built or run. About 70 tool calls.

## The short version

The storage and app-state tests are, on the whole, good. They exercise real databases, real failure modes (a locked file, a junk file, a dropped table, a stop in the middle of an upgrade) and they pin literal values rather than re-deriving them. They are also fast: nothing in this area passes 0.7 s, and the slowest two (the store matrix at 0.64 s and the search invariants at 0.51 s) are corpus sweeps that earn their time.

The real problems are not in what the tests assert. They are in how the tests wait, what they pretend to protect, and a few places where the old-file fixtures share code with the thing they test. The seven findings that matter most:

1. About 25 polling loops wait for a background job with no cap. If a job ever stops finishing, the whole suite hangs instead of failing (high).
2. The old-database fixtures for schema 2 and 3 are built from constants that production exports, so they cannot disagree with production. There is no frozen real old file anywhere (medium to high).
3. Two tests assert "under 20 ms" on a wall clock (medium).
4. `leak_checked` does nothing in the Release build the suite runs, so the one store test that uses it checks nothing (medium).
5. `RecordStore::delete_results_without_chart` is public, has no caller in `src/` and no test. Either dead code or an untested path (medium).
6. The same user-facing sentences are pinned in four test files, so rewording one sentence breaks four tests (medium).
7. Helper duplication: `ScratchPaths` and the library-entry builder are copied between `test_app_state.cpp` and `test_song_panel_state.cpp` (low to medium).

---

## (a) Working well, keep

**Crash-safe upgrade tests in `test_store.cpp` (lines 938 to 1086).** Each one seeds a real old-layout file, makes the upgrade stop at a named point (between the two renames, mid-copy, fresh file unmakeable, file held open by another connection), then checks that the file is whole and nothing is left beside it. A real bug, such as "a stop mid-copy corrupts the old file", would fail `check_old_file_whole`. The "detail layout" SQL in `old_layout_fixture.h` is a literal copy of the old table text, as its comment says, so this one is tested against a real old shape.

**Error-path tests for the store.** Line 1638 loops over 13 read functions, drops every table under each, and checks every one throws the `DatabaseRead` kind and not an empty answer. Line 1523 uses a trigger to force a failed save and checks the store is still usable afterwards (no transaction left open). Lines 1553 and 1568 check a junk file and a locked file fail as `DatabaseOpen` and let go of the file. These protect the exact bug the comments name: a failing read that quietly answers "no charts" and makes a batch re-analyze everything.

**Agreement tests between two readers of the same fact.** `get_summaries` against `analyzed_hashes` against `has_record` (lines 1451, 1672, 1696, 1717). The page test at line 1717 spells out every expected status as a literal, and its comment correctly says why agreeing with `get_summary` alone would prove nothing. That is the house rule done right.

**`test_winstr.cpp` large-file tests (lines 56 to 82, 233).** Real sparse files at 2.5 GB and 5 GB catch the exact old bug (a 32-bit `ftell`). A truncated helper would answer 705,032,704, and the test says so.

**`test_long_paths.cpp`.** It makes real folders past 300 characters and runs the scan, the preview source, settings, reports, SQLite and ImGui's file calls through them. Line 214 pins the 259/260 edge as a literal "so the test doesn't share the owner's arithmetic". Good.

**`test_library_query.cpp` fold tests (lines 132 to 172).** Line 154 walks all 65,000 code points below U+10000 and checks the report pages' fold table lists every character the library fold changes. This is a real exhaustive check, not a sample.

**App-state tests under `ScratchPaths`.** Every test that touches settings or the database points the app at scratch files first and puts the process back afterwards (`test_app_state.cpp:81`). Nothing here reaches the developer's real `hydra.db` or INI. The held-job latch (`ViewLatch`, line 233) makes the "burst of setting changes" test independent of machine speed.

**`test_user_messages.cpp` line 63.** A real refused chart load is run through `plain_error`, so rewording the throw in the parser cannot quietly drop the user back to a generic message. Good use of a real producer.

**Table-driven config tests.** `kModeCases` (`test_config.cpp:371`) pins all 16 difficulty/prodrums/bass2x chart-mode keys with and without Note Shuffle as literals, and the `with_chartmode` test round-trips the same table. These keys are filed in users' databases, so pinning them byte for byte is right.

---

## (b) Broken or misleading

**1. Unbounded waits on background jobs. Severity: high.**
These loops spin until a job reports done, with no time cap, so one stuck job hangs the whole test run instead of failing a test. The house rule ("every step has a stop condition") says each wait needs a cap. In `test_app_state.cpp` the uncapped loops are at lines 242, 982, 990, 1000, 1034, 1037, 1126, 1313, 1350, 1361, 1405, 1506, 1660, 1716, 1720, 1832 and 1854. In `test_song_panel_state.cpp` they are at 153 and 164. In `test_library_jobs.cpp`, lines 90, 433 and 445 are uncapped (but its `wait_until` helper at line 46 does have a 10 s cap, and a few tests use it well). Meanwhile other loops in the same file are capped (`settle` at line 206 gives up after about 60 s; `wait_entered` at line 254). So the capped helpers exist and the uncapped loops simply did not use them. A plausible bug these would turn into a hang: a report job that never sets `finished()` after a cancel. Fix: one shared `wait_until(pred, limit)` in a test header (library_jobs already has the right one) and replace every bare `while (!x) sleep`. Also `test_cli.cpp:80` waits on a child process with `INFINITE` and reads its pipe with no timeout, so a hung `hydra_batch` hangs the suite.

**2. Schema 2 and schema 3 fixtures share constants with the code under test. Severity: medium-high.**
`downgrade_to_schema2` and `downgrade_to_schema3` (`test_store.cpp:1168 to 1202`) build the old table from `kSchema2ResultsTableSql` and `kSchema2ResultsColumns`. The header comment on those constants (`record_store.h:184`) says one of them is "read only by the store test, to build a schema 2 file". The other, `kSchema2ResultsColumns`, is what `upgrade_results_key` itself uses to copy rows across (`record_store.cpp:770`). So if that column list were wrong or missed a real column, the fixture would be built from the same wrong list and the upgrade test would still pass, while a real user's schema 2 file would lose data. These tests protect users who are upgrading from 1.8.x to now, which is the most expensive thing to get wrong. The summary-only fixture (`old_layout_fixture.h`) is better: its table text is a literal. The fix is to freeze the schema 2 and schema 3 table text as literals the same way, or, better, check in one tiny real old-version `.db` file under `testdata/` (there is none: `testdata/` holds no `.db` at all).

**3. `leak_checked` checks nothing in Release. Severity: medium.**
`leak_check.h:66 to 72` makes `leak_checked` run the body once and check nothing unless the build is MSVC Debug. The suite runs `build-cpp\Release\hydra_tests.exe`. The only store test using it is `test_store.cpp:747` ("a new database carries 0 in user_version"), so its leak check never runs in the build everyone tests with. The `user_version` assertions inside it do run, but the "leak" part of its promise is empty. Also only two other tests use it (`test_analysis.cpp:787`, `test_midi.cpp:60`). Either a Debug run with `MIMALLOC_DISABLE_REDIRECT=1` is a known step somewhere (nothing in the tests or `docs/agents` says so), or this is a feature that never fires. Fix: say where it runs, or drop it.

**4. Wall-clock thresholds. Severity: medium.**
`test_library_model.cpp:393` asserts each filter pass takes under 20 ms, and `test_library_query.cpp:379` asserts a 20,000-row match takes under 20 ms. Both take best-of-3 or best-of-5 to dodge a busy machine, which shows the author has already been bitten. They still fail on a loaded CI box, and they say nothing about why. At the time of the timing run they took 0.067 s and 0.016 s in total, so there is a wide margin today. The failure mode is a false red on a slow machine, not a missed regression. The two tests also overlap (both match 20,000 synthetic rows against the same query code, one inside `LibraryModel`). Fix: keep one, make it count work done rather than time (e.g. assert the hit count equals a pinned literal), and leave the timing as a `MESSAGE` the way `test_store.cpp:1895` already does for the 20,000-hash lookup.

**5. `upgrade timing` can never fail. Severity: low.**
`test_store.cpp:1091` returns immediately unless `HYDRA_UPGRADE_TIMING_DB` is set, and when set it contains no `CHECK` at all, only `MESSAGE` lines. It always shows as a passing test, which inflates the count and tells a reader nothing. It belongs in `tools/`, not the suite.

**6. A 4-times-pinned sentence produces 4 broken tests per rewording. Severity: medium.**
The exact "Hydra couldn't read its database (hydra.db)..." and "couldn't open..." sentences are spelled out as literals in `test_user_messages.cpp:135/143`, `test_store.cpp:1610/1538/1060`, `test_app_state.cpp:73` (`kDatabaseReadSentence`) and `test_cli.cpp:205`. `test_user_messages.cpp` owns that text and tests it properly. The others only need to prove the right *kind* of error was raised (which `e.kind() == ...` already does in each of them). Pinning the sentence again in each copy is the house rule's "second copy of the rule". One reword of one sentence turns red in four files. Fix: in the other files keep the kind check and drop the sentence, or compare against the exported constant.

**7. Constants tested against themselves. Severity: low.**
`test_app_state.cpp:954` ("UI timings") asserts `AppState::kCopiedSeconds == 2.0` and five more. It cannot catch a bug, it only forces a test edit whenever a value changes. The values were user-confirmed (D48, D87), which is a reason to write the decision somewhere, but a test that restates the constant is a change detector. Same pattern: `test_user_messages.cpp:117/123` (`kNoPathsFound == "No paths found."`, `kNotAnalyzedText`), `test_winstr.cpp:242` (`kFirstPieceRead == 65536`), `test_store.cpp:1475/2041` (SQLite compile flags). The compile-flag ones are defensible because the flag silently depends on a build setting. The UI-constant one is not.

**8. `hydra_tests` naming dependency. Severity: low.**
`test_winstr.cpp:280` asserts `args[0]` contains "hydra_tests". It fails if the test exe is renamed or run under another harness. It tests `utf8_argv`, which could be checked against the Windows command line directly.

**9. Misplaced/misleading comment in `test_search.cpp:185 to 191`.**
A comment block about the legacy Clone Hero 1.0 fill rule sits directly above the multiplier-squeeze test, which has nothing to do with fills. The comment describes a test that is somewhere else (or gone). Also the SP-overfill test at line 936 to 941 contains a half-finished sentence in the comment ("4608 + 1536 = ... no --"). Low severity, but a reader will believe the wrong test is explained.

---

## (c) Unnecessary: delete or merge

**Three tests protect `chartmode_key` for the same four Expert combinations.** `test_config.cpp:316` (four Expert keys), `:329` (walks Hard/Medium/Easy by hand) and `:392` (the 16-row table with and without Note Shuffle). The table test covers every literal the other two check. The two older ones add only `effective_bass2x()` and `difficulty()` assertions. Merge those two extra assertions into the table test and delete the other two. (Medium value, low risk.)

**Three tests for "the Ready rule is stated once".** `test_store.cpp:1451`, `:1672` and `:1696` (`has_record` against `get_summary`; `analyzed_hashes` against `has_record`; `analyzed_hashes` against `get_summaries`), plus line 474 ("the listing and a lookup agree"). They share the same fixture rows (Ready / old build / other rules / both / mixed). One table-driven test over a single seeded store, looping the four readers, would cover all of them and make adding a fifth reader cheap. (Medium value.)

**`test_obj_loader.cpp:104` repeats line 93.** Line 93 already checks "no faces" throws with the exact message; line 104's first `CHECK_THROWS` is the same case with a weaker check (no type, no message). Its second line (`f 1 2 3` with a missing vertex) is the only new thing and checks no exception type either. Merge into one test with the message checked on both. Low.

**`test_utf8_paths.cpp:49 and :62` overlap `test_long_paths.cpp:167`.** All three save and load the settings INI and read a text file through an unusual path. They protect different path problems (non-ASCII vs over-260), so they are not duplicates, but a single table over "odd folders" (non-ASCII, long, both) would be one test. Low value to change.

**The 2.5 GB sparse test (`test_winstr.cpp:56`) versus the 5 GB one (`:64`).** The 5 GB test already catches a 32-bit truncation. The 2.5 GB one catches a signed-32-bit helper that is otherwise 64-bit capable, which is distinct, so keep both.

**`delete_results_without_chart` (`src/store/record_store.cpp:1266`).** No caller in `src/`, `tools/` or `tests/` (checked by grep over all three). It is described in a header comment as the model for two other functions. If nothing calls it, the function is dead and the comment is the only thing keeping it. Either delete it and fix the comments or give it a caller and a test. (See Gaps.)

---

## (d) Poorly designed

- **`test_search.cpp` is not about search in the user's sense.** It is 2,165 lines of engine tests (SP squeeze, tied variants, clamp rules, SP end history) filed under a name that looks like it belongs with the library search. A reader looking for "how does library search work" will go to `test_search.cpp` and find squeeze maths; the library search lives in `test_library_query.cpp`. Fix: rename to `test_engine_search.cpp` or split by topic (squeeze, tied variants, overfill). The engine scout may also list this file.
- **Giant files.** `test_store.cpp` (2,225 lines), `test_search.cpp` (2,165) and `test_app_state.cpp` (1,966) each hold 60 or more cases. Fix: split `test_store.cpp` into at least three (results and lookups; library table and scan; open, upgrade and crash safety) so the upgrade section can be read on its own. `test_app_state.cpp` splits the same way (the click; settings; reports; startup open).
- **Duplicated helpers across files.** `ScratchPaths` is defined twice (`test_app_state.cpp:81`, `test_song_panel_state.cpp:40`), the second version is a cut-down copy without the rules path. `test_config.cpp:139` and `test_preview_controller.cpp:370/411` do the same set/restore by hand with no guard, so an exception between the two calls leaves the process pointing at a deleted scratch file for every later test. Two near-identical "library entry for index i" builders exist (`test_app_state.cpp:112`, `test_song_panel_state.cpp:61`), plus `chart_entry` (`test_store.cpp:88`) and `chart` (`test_library_model.cpp:32`). Fix: one `scratch_paths.h` with the RAII struct, and one builder header for `ChartLibraryEntry`.
- **The first-analyzable-corpus-chart loop is copied.** It appears at `test_store.cpp:148 to 163`, `:209 to 222` and `:254 to 273` (the third is a cached static; the first two redo the work). Fix: the first two should use `fixture()`.
- **Opaque names.** Tests are named for the decision, not the behaviour: "a file from before AL loses its songlength table" (`test_store.cpp:1911`), "summarize_record counts every kept path once" with "Finding 172" in a comment, "(D51 Q11)" and similar in a dozen test names. A reader cannot tell what bug is protected without opening `docs/audit`. Fix: lead the name with the behaviour; keep the decision tag in the comment.
- **Misplaced tests.** `test_winstr.cpp:286` tests the `temp_util.h` test helper itself, and `test_batch_text.cpp:139` ("settings reset") tests `Settings::with_analysis_defaults`, not batch text. `test_utf8_paths.cpp:77` is a renderer test inside a paths file. Fix: move each to the file named for its subject.
- **Hand-listed field lists that can silently go stale.** `check_same_settings` (`test_config.cpp:38`) lists each Settings field by hand, and `settings reset` (`test_batch_text.cpp:142`) lists each panel setting's change by hand. A new setting added to `Settings` is not covered unless someone remembers to add it. The key-name test at `test_config.cpp:89` does catch a new INI key, which is the right guard for the first; the reset test has no equivalent guard. Fix: add a scan test that fails when `Settings` gains a field the table does not name.
- **Cleanup on failure.** Many store tests end with `std::remove(path)` after a `REQUIRE`/`CHECK`, so a failing `REQUIRE` leaves a `hydra_test_*.db` behind. Next run removes it at the start (`std::remove` at the top of each), so no wrong result follows, but the temp folder fills on red runs. `testtemp::ScopedFile` exists for exactly this; the store tests do not use it. The ImGui tests in `test_app_shell.cpp` (lines 217, 224) have `REQUIRE`s between `setup_imgui` and `shutdown_imgui`; a failure leaves a live ImGui context for every later ImGui test. Low.
- **Re-computed expected values.** `test_app_shell.cpp:103` checks `doubled.FramePadding.x == base.FramePadding.x * 2.0f`, which restates the scale formula. A literal would be safer. `test_search.cpp:199 to 208` rebuilds the expected multiplier-squeeze list with the same constructor and the same score table as the production graph, so it checks the graph's loop against a copy of the loop. It at least calls production functions rather than a copy of a formula, so this is borderline, not a clear violation. Medium to low.
- **Near-copy tests that want a table.** The four SP-overfill cases (`test_search.cpp:933 to 1070`) build the same eight-note skeleton four times and differ by one tick and one expected number. One table of (phrase ticks, expected deact, expected clamp) would be shorter and make the next case a one-liner. Same for the five "SP end history" cases at `:1144 to 1214` and the four "tied variants" cases. Low.

---

## (e) Gaps

- **`delete_results_without_chart`** has no caller and no test (see above). Medium.
- **No frozen real old database anywhere in `testdata/`.** Every upgrade test starts from a file a current build wrote and then rewrote by hand. A real file from Hydra 1.8.4 and one from 2.1.0 would be the only way to know a real user file upgrades. Medium-high.
- **Pre-1.7 upgrade.** `test_store.cpp:712` pins that a 1.6 file opens empty and is left alone. It does not check that opening it twice stays stable, or that a second, current-version `RecordStore` can write into the same file. The `records` table is left in place forever. Low.
- **No test for a power cut mid-write after the swap.** The recovery matrix (`test_upgrade_files.cpp`) covers every file combination after a kill between steps, which is excellent. It does not cover the `-wal`/`-shm` side files of the stepped-aside original surviving into the next run. Low (SQLite handles it, but the code owns the rename).
- **`Settings::load_file` with a file it cannot read** (permission denied, directory instead of file). The tests cover missing and malformed content (`test_config.cpp:164, 188`), not an unreadable path. Low.
- **No test of INI save atomicity.** `save_file` returns a bool; the long-path test checks a successful save. There is no test of what happens to the previous good INI when the write fails halfway. Medium if `save_file` writes in place (not checked).
- **`ViewJob::run` re-identify decisions** are tested well by app-state clicks, but only on chart folders with a `song.ini`. Charts with a `.sng` or `.mid` and no ini are not clicked anywhere in this area. Low.
- **Rules file.** `test_app_state.cpp:633` covers a bad `max_tied_paths`. Only one bad key is ever tried, and `test_config.cpp` does not cover `rules_file.cpp` at all. This may live in the engine scout's files (rules tests), so I did not chase it.
- **Mid-run store replacement.** Nothing tests `AppState` when the database file is replaced or deleted while the app is open (only dropped tables). Low.

---

## (f) Files not reached

Read in depth: `test_store.cpp` (about 75 percent), `test_app_state.cpp` (lines 1 to 340, 625 to 1020, 1730 to 1966; the middle report and scan sections 1020 to 1730 only by name list), `test_library_jobs.cpp` (lines 30 to 270 and 285 to 375), `test_library_model.cpp` (lines 1 to 130, 240 to 400), `test_library_query.cpp` (lines 125 to 185, 310 to 380), `test_config.cpp` (lines 1 to 165, 300 to 435, 595 to 663), `test_cli.cpp` (lines 1 to 240), `test_app_shell.cpp` (all), `test_song_panel_state.cpp` (all), `test_winstr.cpp` (lines 20 to 310 mostly), `test_long_paths.cpp` (all), `test_user_messages.cpp` (lines 40 to 233), `test_obj_loader.cpp` (lines 100 to 216), `test_image_decode.cpp`, `test_utf8_paths.cpp`, `test_search.cpp` (lines 1 to 230 and 795 to 1085), `old_layout_fixture.h`, `db_file_util.h`, `temp_util.h`, `scratch_settings.h`, `leak_check.h`, `source_tree.h`.

Test-case names only (not read): `test_upgrade_files.cpp` bodies, `test_batch_text.cpp` lines 1 to 60 and 200 to 243, `test_library_jobs.cpp` lines 375 to 518 (leaderboard and failed-job tests), `test_library_model.cpp` lines 130 to 240, `test_library_query.cpp` lines 1 to 125 and 185 to 310, `test_config.cpp` lines 165 to 300 and 435 to 595, `test_cli.cpp` lines 240 to 447 (the fill-compare tests), `test_obj_loader.cpp` lines 1 to 100, `test_strutil.cpp`, `test_app_state.cpp` lines 340 to 625 and 1020 to 1730, `test_search.cpp` lines 230 to 795 and 1085 to 2165, `test_store.cpp` lines 340 to 700.

Helpers not opened: `record_fixtures.h` (header comments and first 60 definitions only), `env_util.h`, `byte_source_util.h`, `db_file_sql.h`.

Not checked: whether any of the 25 unbounded loops has a watchdog outside the test (the user-scope hooks mention a watchdog for agents, not for the test exe), and whether a Debug run with `MIMALLOC_DISABLE_REDIRECT=1` is part of any routine.

## Timings (from suite-durations.txt)

Nothing in this area passes 0.7 s. The slowest are `a saved result reads back its best path ... across the corpus and config matrix` (0.64 s), `search invariants hold across the corpus and config knobs` (0.51 s), `note total ... on every corpus chart` (0.30 s) and `the graph finds the chart's multiplier squeezes once` (0.27 s). All four are corpus sweeps that run the real analyzer on every checked-in chart and are not worth trading away. `jobs: pause lets the chart in flight ...` (0.26 s) and `jobs: stop while paused ...` (0.22 s) are slow only because of two fixed 200 ms sleeps used to prove "nothing started", a negative check that can only be proved by waiting. The remaining hydra_batch CLI tests (0.07 to 0.2 s each) copy the exes into a temp folder per test; a shared sandbox would trim them, but the gain is small.
