Status: DONE_WITH_CONCERNS. One acceptance criterion needs a user answer; see the open question below.

Branch `worktree-agent-a3f10bd3b212c6a3d`, tip `47a6a4e`. The worktree had been cut at `bc58282`, 59 commits behind main. I fast-forwarded it to `814caf0`, the base you named, before any edit.

**The report file was not written.** Writing to `C:\Users\Patrick\Downloads\Hydra\hydra-test\.superpowers\sdd\2026-10-07-summary-only-storage\task-3-report.md` was refused: "This agent is isolated in the worktree ... Edit the worktree copy of this file instead of the shared-checkout path." The full report is below. Please save it there.

Tests: test_report 38/38 (1780 corpus rows equal field by field in two chart modes), test_fill_report 16/16, single-owner* 10/10, plus the 2 ReportJob tests. A full build of every target was clean.

---

# Task 3 (storage-T3) report: the path report from the engine

## What changed

The path report no longer reads stored paths. `collect_rows` asks `list_records` which charts have a Ready row at the report's cap and lens. That covers every chart mode, and it also gives each chart's name. The report then analyzes each one that has a library row, with `analyze_chart_file`, through `run_work_pool` on `batch_worker_count()` threads. Each chart's rows come from one new function, `chart_rows`. It makes the same `summarize_path`, `pathstring`, `is_optimal` and `e_difficulty` calls as before. The batch's seed and the report's own pass both build rows there.

The batch hands over what it already worked out. `BatchCallbacks` has a new `report_seed` pointer. When it is set, each worker builds the chart's `chart_rows` after the analysis. The calling thread files them in the seed by md5, at the moment that chart's `on_result` fires, so only charts that were saved get in. I did not change `on_result`'s own signature. hydra_batch (T4's file) and `test_analysis.cpp` both use that signature.

`BatchJob` owns the seed. `AppState` passes the seed and the batch's run to `ReportJob` at the start site. `hydra_report` passes an empty seed, with `settings.batch_run()` as the run.

The analysis settings come from a `BatchRun` in `ReportOptions::run`. The report throws if that run is missing, or if its cap or lens differs from `options.cap` and `options.lens`. A seed filed under other settings also throws, and so does one holding fewer rows per chart than the report asks for. `run_batch` refuses a seed for another run.

Chart modes other than the run's own keep the page as it was. The old page listed every mode at the current cap, and the subtitle still says so. Each other mode is analyzed with the run's search settings (the `SearchSettings` base). Its parse choices (difficulty, pro drums, 2x bass) come from whichever `Settings` makes `chartmode_key()` spell that mode. That keeps `chartmode_key` as the one owner of the key. A mode that no settings spell throws.

A small new helper, `stop_on_cancel`, builds the cancel-checking progress callback. `run_batch` and the report pass both use it, instead of each keeping its own copy.

The old reader stays as `collect_stored_rows`. Only the equivalence test calls it. T5 deletes both.

## Files changed

The production files are `src/app/report.h`, `src/app/report.cpp`, `src/app/analysis.h`, `src/app/analysis.cpp`, `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`, `src/ui/app_state.cpp` (the start site only) and `src/cli/report.cpp`. The tests are in `tests/test_report.cpp`.

`library_jobs.h` had to change along with the .cpp. BatchJob gained three small pieces: a `seed_` member, the line `callbacks.report_seed = &seed_`, and `take_report_seed()`. T2 also edits `library_jobs.cpp`, so expect a small merge there.

`ReportJob`'s constructor keeps its old leading arguments and adds `run` and `seed` as trailing defaults. That keeps `tests/test_library_jobs.cpp:288` compiling without me editing a file I don't own. A job built without a run fails loudly when it runs.

## Acceptance criteria

1. The engine's rows equal the stored records' rows, field by field: **PASS.** The test scans `testdata/input` and saves it as the library. It batches two chart modes, "Expert Pro Drums, 2x Bass" and "Hard Drums, 1x Bass", at cap 4 with top 100 paths. It then compares `collect_rows` against `collect_stored_rows`. 1780 rows matched on all 20 fields, with no failures. Nothing differed, so there is no finding to report.
2. A seeded chart is not analyzed again: **PASS.** The test counts calls through a `BatchCallbacks::analyze` hook and through `ReportOptions::analyze`. A batch covers 2 of 3 charts. The report then analyzes only the third, and its page matches a fresh full analysis byte for byte.
3. Cancelling stops the pass and writes no report: **PASS** at the `generate_report` level. The first analysis sets the flag. Each worker then stops after at most the chart it had started. The result is `Cancelled`, with no rows and no html. `ReportJob` keeps its existing early return when cancelled, so it writes no file. I did not add a GUI-level test.
4. A chart whose file fails to load is skipped and counted: **PASS as written, but see the open question.** The chart is left off the page. Its line, "<notespath>: <error>", goes into `GeneratedReport::failures`.

## Open question (the brief said to stop and ask)

Before this change, the report had no "failed" count. A stale record was skipped without being counted, and a record that failed to decode threw and failed the whole report. So the brief's condition "if no failed count exists today" holds. My provisional behaviour skips the chart and records it in `GeneratedReport::failures`. Neither the GUI nor `hydra_report` shows that list yet, because no new user-visible words are allowed. The user needs to decide whether, and how, the page or the finished strip should mention charts that failed to load.

## Verify

`.\build_cpp.ps1 -Target hydra_tests` succeeded. A full `.\build_cpp.ps1` also built every target cleanly, including Hydra, hydra_report and hydra_uitest.

- `hydra_tests.exe -sf=*test_report*` gave 38 passed out of 38, with 36776 assertions, in about 0.4 s.
- `hydra_tests.exe -sf=*test_fill_report*` gave 16 passed out of 16, unchanged.
- `hydra_tests.exe -tc="single-owner*"` gave 10 passed out of 10. The first run flagged two hand-built `RecordKey{...}` in my test fixtures. I moved those to `Settings::record_key`, and the rerun passed.
- `hydra_tests.exe -tc="jobs: a report job carries*,the post-batch report lists*"` gave 2 passed out of 2. I ran these because I changed `ReportJob`.

Red before green: with the seed fill disabled (`if (false && seed && ...)`), the seed test failed on the line `tests\test_report.cpp(849): ERROR: CHECK( seed.rows.size() == 2 ) is NOT correct! values: CHECK( 0 == 2 )`. The other test cases were written together with the new API, so their red would only have been a compile error.

## Migrated tests

The existing `test_report.cpp` tests stored fixture records under a chart mode called "mode", and some had no library rows. Both are now impossible, so the fixtures changed. They file results under `fixture_settings(cap).record_key(...)` and list each chart in the library. Real corpus charts keep their real file. The tied-variant fixture uses the notespath `"fixture:tied"`, which `fixture_analyzer` turns into the H1 record. Names now come from the library rows, which `rebuild_chart_library` also writes to songmeta. So the names test still holds after T5 moves `list_records` onto `kNamingCopiesSql`.

One test changed meaning: "an unlisted chart counts once" (D77 item 2) became "a chart the library dropped is left out", because D87 item 4 replaces D77 item 2.

## Self-review

Each question the change answers has one owner. Rows come from `chart_rows`, called by both the batch and the report. Which charts are Ready, and their names, come from `list_records`. Copies come from `RecordStore::copies_of`. Settings come from `Settings::batch_run`, `chartmode_key` and `to_analysis_settings`. The tier comes from `tier_for`. The worker count comes from `batch_worker_count`, and the cancel check from `stop_on_cancel`. No new user-visible text was added. No change touches the store or `stored_versions.h`.

## Concerns

- **Charts with a result but no library row are no longer on the page** (D87 item 4). `hydra_batch` run with folder arguments leaves the library alone. On such a database, `hydra_report` now prints "Nothing is analyzed under these settings (...)" instead of a page. That sentence is misleading there. It needs a decision, or T4's batch change may settle it.
- **Row order changed.** The page's JSON rows now follow `list_records`, best score first. Before, they followed insertion order, which in a threaded batch is effectively random. The table sorts in the browser, so only the order of tied rows can differ.
- **A copy is picked arbitrarily.** For an md5 with several copies, the report analyzes the first copy `list_chart_library` lists (it sorts by name), not the naming copy. The copies hold the same notes file, so this only matters if their song.ini files differ.
- **No changed-file check.** The report does not check `chart_files_unchanged`. A chart edited since the scan shows its new paths under its old hash, which is the same gap D87 item 3 fixes for clicks.
- **ReportJob's defaulted arguments.** `run` and `seed` are trailing defaults only to keep `test_library_jobs.cpp` compiling. Once someone may edit that test, the constructor could require them.
- **Tool calls.** I used about 130 calls, past the 100-call wrap-up mark, without splitting the work. Everything above is committed.

---

# Fix round (agent a56cbdb1d56533982), items 1-7 of task-3-fix-plan.md

Status: DONE. Branch `worktree-agent-a3f10bd3b212c6a3d`, tip `4eb7d5f` (one commit on top of `47a6a4e`). Main is not merged yet, and the reviewer has not been messaged, as the dispatch said.

## What changed

The report now analyzes the same copy of a chart the batch does. The store has one new read-only function, `RecordStore::naming_copy_paths()`. It returns, for each md5, the notes file of the copy `kNamingCopiesSql` picks. It joins that query's own pick back to its `charts` row (`p.rowid = c."MIN(rowid)"`), written the way `library_copies()` is. No other store code changed. `collect_rows` reads its files from it. The `files` map built from `list_chart_library`, its `emplace` line and the "Copies of one md5 hold the same notes" comment are gone. `collect_rows` no longer takes a `library` argument, so `generate_report` and the six test call sites stopped passing one. A hash the function doesn't return is a chart that left the library, and stays off the page (D87 item 4).

The page says which charts it left out (D89 item 1). A failure is now a `ReportFailure` with the notes path and the error. There is one per file, even when the file failed in two chart modes. The new `left_out_line()` builds "Left out: N charts whose file couldn't be read." with `hydra::counted`, and returns an empty string when nothing failed. `build_html` puts the line and the file paths in the payload as `left_out` and `left_out_files`. The page script adds them under the subtitle, with `textContent`, only when the line is not empty. I did not touch the page body's header markup, so the known clone entries for `<div class="sub">__SUBTITLE__</div>` still hold. After "Wrote ...", `hydra_report` prints the same line and then each path.

A database with results but no chart library now says so (D89 item 2). `generate_report` has a new reason, `NoLibrary`, between `NothingStored` and `NothingUnderSettings`. It is used when the store holds results but `chart_library_count()` is 0. Its `why_empty` is the constant `kNoChartLibrary`, which holds D89's sentence word for word. The `NothingStored` branch line is unchanged. `hydra_report` prints `why_empty` whenever it is set, so it prints both sentences.

There is a new scan row in `tests/test_single_owner.cpp`, "Which library copy of a chart does a pass analyze?". Its owner is `kNamingCopiesSql`, read through `RecordStore::naming_copy_paths`. It uses the reviewer's pattern over `src`, and its must-match examples include the removed `files.emplace(...)` line. The new store line names `MIN(rowid)`, which the existing "Which copy names a chart the scan found twice?" rule flags. So I added that line to the rule's owner lines, with the reason "naming_copy_paths joins kNamingCopiesSql's own pick back to its row".

## Covering tests (tests/test_report.cpp)

- "the report analyzes the copy that names a chart the scan found twice" sets up one md5 with two copies. The scan lists the real file first, titled "Zed", then a missing file titled "Alpha". The counting analyzer sees only the real file. There are no failures, and the page counts 2 copies.
- "a report on results with no chart library says the library is missing (D89)" checks that `rows == 0`, that the reason is `NoLibrary`, and that `why_empty` equals D89's sentence, pinned as a literal.
- "a chart whose file fails to load is left off the page and listed" is extended. With no failures, the html has no "Left out:". With one failure, it has "Left out: 1 chart whose file couldn" and the missing file's name. It also checks the failure's `notespath` and that its error is not empty.

## Commands and output

The first `.\build_cpp.ps1 -Target hydra_tests` hit the known C1001/LNK1000 stale-state failure. I deleted `build-cpp\hydra_tests.dir\Release\hydra_tests.iobj` and `.ipdb`, and the rebuild succeeded. A full `.\build_cpp.ps1` then built every target cleanly.

- `hydra_tests.exe -sf=*test_report*`: 40 passed out of 40, with 36788 assertions. The engine-vs-stored test still compares 1780 rows.
- `hydra_tests.exe -sf=*test_fill_report*`: 16 passed out of 16.
- `hydra_tests.exe -tc="single-owner*"`: 10 passed out of 10.
- `hydra_tests.exe -tc="jobs: a report job carries*,the post-batch report lists*"`: 2 passed out of 2. I ran these because `GeneratedReport::failures` changed type.

No store test covers `naming_copy_paths` on its own. The two-copies report test covers it.

## Red before green

I built once with three temporary changes. The first put back the old pick: a map filled by `old.emplace(normalize_chart_hash(e.md5), &e)` over `list_chart_library`. The second passed `{}` as the page's failures. The third disabled the `NoLibrary` branch with `false &&`. Then I restored all three. The red lines were:

- `tests\test_report.cpp(916): ERROR: CHECK( pass.paths == std::vector<std::string>{files[0]} ) is NOT correct!` The same test also failed at lines 917 (`page.failures.empty()`) and 918 (`page.records == 2`).
- `tests\test_report.cpp(930): ERROR: CHECK( page.empty_reason == report::EmptyReason::NoLibrary ) is NOT correct!` Line 933 (`why_empty`) also failed.
- `tests\test_report.cpp(955): ERROR: CHECK( page.html.find("Left out: 1 chart whose file couldn") != std::string::npos ) is NOT correct!` Line 957 (the file name) also failed.
- `src/app/report.cpp:393: answers "Which library copy of a chart does a pass analyze?", which belongs to kNamingCopiesSql in src/store/record_store.h (the naming copy), read through RecordStore::naming_copy_paths: for (const store::ChartLibraryEntry& e : library) old.emplace(normalize_chart_hash(e.md5), &e);` This came from the single-owner run, which had 9 passed and 1 failed.

## Concerns

- In the GUI, a database with results but no library still shows the app's "no records stored yet" message. D89 names hydra_report only, so I left `ReportJob` alone.
- `naming_copy_paths` sits right after `library_copies` in record_store.{h,cpp}, and T1 is changing those files. The merge with main may conflict there, but only by position.

## Delta: merge of main (T1, T4)

I ran `git merge main` once, with main at `78bef9d`. Git merged it with no conflicts, and `naming_copy_paths` sits beside T1's new store functions. The merge commit is `43a5c70`, which is the new tip. The ref gate refused git's default merge message because it had no trailers, so I committed the merge again with them.

One test had to change. "a report on results with no chart library says the library is missing (D89)" set up its database by saving a scan with no charts in it. T1's `rebuild_chart_library` now deletes every result the library doesn't list, so that setup left an empty database, and the report gave `NothingStored`. The test now stores a song and its record without ever saving a scan. That is the state `hydra_batch` with folder arguments leaves behind. The production code did not change.

The first build after the merge reported success, but the seed test crashed with SIGSEGV inside `CHECK_THROWS_AS(run_batch(...))`. After I deleted `hydra_tests.iobj` and `.ipdb` and relinked, the same source passed three runs in a row. The crash came from the known stale incremental link, not from the code.

Every test below ran on a clean relink and a full `.\build_cpp.ps1`:

- `-sf=*test_report*`: 40 passed out of 40. The engine-vs-stored test still compares 1780 rows. The test the dispatch named, which this branch rewrote as "a chart the library dropped is left out", is among them.
- `-sf=*test_fill_report*`: 16 passed out of 16.
- `-sf=*test_store*`: 77 passed out of 77.
- `-tc="single-owner*"`: 10 passed out of 10.

## Fix note: two hydra_report CLI tests (test_cli.cpp)

The "hydra_report writes a page..." and "hydra_report reports a --legacy-fills database..." tests built their database with folder arguments, which never saves a chart library (D79 item 1), so the report now says "This database has no chart library" (D87 item 4, D89 item 2). Both now build the database through a new CliSandbox::library_db helper: it writes hydra_settings.ini with `Settings::chartfolders` = the sandbox songs folder, then runs hydra_batch with no folder arguments. All page-content assertions are kept. The first test also gets one new check: a folder-argument database exits 1 with the exact D89 sentence.

Tests: `-sf=*test_cli*` 13/13 cases, 177 assertions pass; `-tc=single-owner*` 10/10 cases, 2441 assertions pass.
