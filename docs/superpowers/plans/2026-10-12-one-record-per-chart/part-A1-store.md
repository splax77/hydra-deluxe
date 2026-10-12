# D115 plan, part A1: the store

Planner: agent a93a2eb6af1162a72 (Fable), task D115-plan-A1, 2026-10-12. Read-only; nothing here is built yet. Every file and function name below was checked against the tree in `C:\Users\Patrick\Downloads\Hydra\hydra-test` on 2026-10-12. Where the spec and the code disagree, the code wins and the difference is called out.

## 1. The store's interface, fixed first

This is what every other part builds against. Names the spec gives are used as given. Names I chose are marked "(mine)". It is a sketch, not the final header: an executor writes the real comments.

```cpp
namespace hydra::store {

// ---- what a record keeps ---------------------------------------------------

// How many paths a record keeps, and the most the report shows. Replaces
// app::report::kDefaultReportPaths (src/app/report.h:152). (name mine)
inline constexpr int kPathsPerRecord = 5;

// ---- the profile ----------------------------------------------------------

// The settings part of a profile: chart mode, SP cap and lens. This is today's
// ResultSettings (record_store.h:151), renamed. Settings::profile() is its one
// builder in the app. The store adds the results stamp and the rules
// fingerprint itself when it files or looks up a record.
struct Profile {
    std::string chartmode;
    CapQuery cap;      // CapQuery and Lens stay as they are
    Lens lens;
    bool operator==(const Profile&) const; bool operator!=(const Profile&) const;
};
using ResultSettings = Profile;   // interim alias only; A1-5 deletes it

// ---- rows ------------------------------------------------------------------

// One kept path: a record_paths row. (name mine)
struct PathRow {
    int rank = 1;                           // 1 = best
    std::string path;                       // the pathstring
    int64_t score = 0;
    int actcount = 0;
    int maxskip = 0;
    std::optional<double> hardest_ms;       // Path::difficulty()
    std::optional<double> hardest_efill;    // new: hardest early fill (chart_rows' efill today)
    double avgmult = 0.0;                   // unrounded; format_avg_mult rounds it
    int sqin_count = 0;
    int sqout_count = 0;
    bool optimal = false;                   // new: HydraRecord::is_optimal
};

// One record: a chart's saved result under one profile. (name mine)
struct RecordRow {
    std::string song_id;                    // lowercase hex (ADR 0028)
    int pathcount = 0;                      // every kept path, roots and variants (all_paths().size())
    std::optional<int> stars;               // rank 1's star count; unset when no paths
    int notecount = 0;                      // Song::note_count, one per record
    bool from_summary = false;              // section 6
    std::vector<PathRow> paths;             // rank order; at most kPathsPerRecord; empty = analyzed, nothing kept

    // "Does this record still need its paths": the one reader of from_summary.
    // The report's fill pass and the tests ask this and nothing else.
    bool needs_paths() const { return from_summary; }
    // The library's view of the record (score, stars, counts, best path),
    // read off rank 1 and the record. Keeps today's PathSummary shape so the
    // library model and the pages change little; one function, no recompute.
    PathSummary summary() const;
    bool has_scored_best_path() const { return !paths.empty(); }
};

// What an analysis becomes before it is saved, built off the store so a
// worker thread can make it (today's PreparedRow, record_store.h:183).
struct PreparedRecord {                     // (name mine)
    Profile profile;
    uint64_t rules_fp = 0;                  // HydraRecord::rules_fingerprint
    RecordRow record;
};

// The one place an analysis becomes a record and its path rows: walks
// HydraRecord::all_paths(), ranks from 1, rank 1 is the summary. Folds in
// summarize_path, summarize_record, best_path_text and report::chart_rows'
// walk. Throws std::invalid_argument on a profile that does not match the
// analysis (today's prepare_row checks: cap, ms limit, fill rule).
PreparedRecord prepare_record(std::string song_id, const Profile& profile,
                              const HydraRecord& analysis);

// ---- lookups ---------------------------------------------------------------

// What a lookup found: today's SummaryLookup (record_store.h:229), widened
// with the record itself. `record` is filled only when status is Ready.
struct RecordLookup {                       // (name mine)
    RecordStatus status = RecordStatus::NotAnalyzed;
    bool stale_build = false;
    bool stale_rules = false;
    RecordRow record;
};
using SummaryLookup = RecordLookup;         // interim alias; A1-5 deletes it

// Ready-record listing entry for pages: the record plus its names from
// ChartIndex is the page's job; the store hands back records only.

class RecordStore {
public:
    // ---- writing (one save) ----
    // The one save (section 5). One transaction: replaces the chart's record
    // under the same profile, deletes the chart's records under every profile
    // whose stamp this build cannot read, deletes any profile left with no
    // records. Inside the open save group when there is one, as a SAVEPOINT.
    void save(const PreparedRecord& prepared);

    // ---- reading (one status lookup) ----
    // One answer per song id, in order. Resolves the wanted profile and its
    // siblings once (same mode, cap, lens; any stamp or rules), then answers
    // Ready / Stale (with why) / NotAnalyzed per chart (section 4).
    std::vector<RecordLookup> lookup(const std::vector<std::string>& song_ids,
                                     const Profile& want);
    RecordLookup lookup_one(const std::string& song_id, const Profile& want);
    // Every chart `lookup` would call Ready under `want`: the batch's skip
    // list (today's analyzed_hashes).
    std::unordered_set<std::string> ready_song_ids(const Profile& want);
    // Every Ready record under `want`, paths included, best score first: what
    // the report, the leaderboard page and the fill page read. Names come
    // from ChartIndex, not from here (today's list_records joined charts).
    std::vector<RecordRow> ready_records(const Profile& want);

    // ---- counts ----
    // How many records the file holds, any profile. Read only by
    // report::holds_results ("does it hold any").
    int64_t record_count();                 // replaces counts()

    // ---- the chart index ----
    // The library's charts, copies and naming copies, built from the charts
    // table in scan order (rowid order). Section 8's ChartIndex.
    ChartIndex chart_index();
};

// ---- ChartIndex (src/store/chart_index.h, new) ------------------------------

// The one owner of "which charts does the library list, how many copies does
// each have, and which copy names it", built from the library rows in scan
// order. A chart the index doesn't list counts once (D77 item 2).
class ChartIndex {
public:
    static ChartIndex from_rows(const std::vector<ChartLibraryEntry>& rows_in_scan_order);
    bool lists(const std::string& song_id) const;
    int copies(const std::string& song_id) const;          // 1 when unlisted (D77)
    const ChartLibraryEntry* naming(const std::string& song_id) const;  // first row; null when unlisted
    // How many charts a set of ids counts as: each id's copies added once,
    // however often it appears. The header, the tile, the building bar,
    // "Left out", the fill page and the batch's numbers all call this.
    int64_t count(const std::vector<std::string>& song_ids) const;
    const std::vector<std::string>& song_ids() const;      // distinct, in scan order
};

}  // namespace hydra::store

// src/app/config.h
struct Settings {
    // The one builder of the profile the current settings analyze under
    // (section 8). Replaces record_key({}).settings(), BatchRun::filed_under,
    // PageSettings::of's cap and lens, dm_report's forced cap and
    // fill_report's lens edits.
    store::Profile profile() const;
    // The two named variants the pages ask for by name (names mine):
    store::Profile profile_at_clone_hero_cap() const;      // the leaderboard page's
    store::Profile profile_with_fills(FillDeadlineRule rule) const;  // the fill page's two
};
```

Where I chose a name: `kPathsPerRecord`, `PathRow`, `RecordRow`, `PreparedRecord`, `prepare_record`, `RecordLookup`, `lookup`, `lookup_one`, `ready_song_ids`, `ready_records`, `record_count`, `ChartIndex::lists/copies/naming/song_ids`, `Settings::profile_at_clone_hero_cap`, `Settings::profile_with_fills`, and the `OpenStep::ConvertingRecords` step below. The spec's own names kept: `Settings::profile()`, `ChartIndex`, `ChartIndex::count`, `Profile`, `records`, `record_paths`, `profiles`, `from_summary`.

Two notes on the shape. First, the status lookup carries the whole record (paths too) because the report lists Ready records and wants their rows in the same read; the library model reads `.record.summary()` and ignores the paths. Second, "does this record still need its paths" is `RecordRow::needs_paths()`, an inline member in `store/`, because the only fact it reads is the record's own column and the report already holds the record. If the main session would rather have a `RecordStore` method, it is one line more; nothing else changes.

## 2. What the tables look like

Three tables replace `results` (section 3). Column names below are the plan's; an executor may not rename them without telling the main session, because A2 reads them through the structs above, and `tools/compare_db.py` (A2's) learns them by name.

`profiles`: `id INTEGER PRIMARY KEY, chartmode TEXT NOT NULL, sp_cap INTEGER NOT NULL, ms_enabled, ms_value, depth_mode, depth_value, legacy_fills INTEGER NOT NULL, hyversion TEXT NOT NULL, rules_fp BLOB NOT NULL, UNIQUE (chartmode, sp_cap, ms_enabled, ms_value, depth_mode, depth_value, legacy_fills, hyversion, rules_fp)`. The lens column names are `kLensColumnList` (record_store.cpp:245), reused, so `bind_lens` and `lens_match` keep working on this table.

`records`: `song_id TEXT NOT NULL, profile_id INTEGER NOT NULL, pathcount INTEGER NOT NULL, stars INTEGER, notecount INTEGER NOT NULL, from_summary INTEGER NOT NULL, PRIMARY KEY (song_id, profile_id)) WITHOUT ROWID`.

`record_paths`: `song_id TEXT NOT NULL, profile_id INTEGER NOT NULL, rank INTEGER NOT NULL, path TEXT NOT NULL, score INTEGER NOT NULL, actcount, maxskip INTEGER NOT NULL, hardest_ms REAL, hardest_efill REAL, avgmult REAL NOT NULL, sqin_count, sqout_count INTEGER NOT NULL, optimal INTEGER NOT NULL, PRIMARY KEY (song_id, profile_id, rank)) WITHOUT ROWID`.

Prior art for the rebuild and the key form, searched 2026-10-12: SQLite's own page on WITHOUT ROWID tables (https://www.sqlite.org/withoutrowid.html) says the clause needs an explicit PRIMARY KEY, stores the row inside the key's index (so there is no second copy of the key, which is section 3's reason for it), and is worth it for tables whose key is text or composite and looked up often, which both `records` and `record_paths` are. There is no ALTER that changes a table's key, so the move is the create-insert-drop rebuild the store already does in `upgrade_results_key` (record_store.cpp:771). The common migration recipes (for example the Turso migration notes, https://instagit.com/tursodatabase/turso/pitfalls-migrating-sqlite-to-turso) all come down to: one transaction, create the new table, `INSERT ... SELECT` with named columns, drop the old one, recreate indexes. One caveat from the docs applies to this repo: a WITHOUT ROWID table has no `rowid`, so `copy_table` (record_store.cpp:943), which copies `rowid` for any table without an INTEGER PRIMARY KEY, must learn to skip it. Section 5 covers it.

## 3. Where the upgrade runs, settled from the code

The spec says the copy upgrade "runs earlier in the constructor". The code says the opposite: the constructor calls `set_up_schema()` first and `copy_upgrade(dbpath)` second (record_store.cpp:682-683), and `set_up_schema` runs `create_result_tables`, the stars column, `upgrade_results_key` and `delete_auto_results` in that order (record_store.cpp:755-766). So the open sequence today is: journal settings, charts and meta tables, charts columns, results table, stars column, results-key rebuild, Auto delete, then the summary-only copy upgrade if the `paths` table exists, which makes a fresh file by building a second `RecordStore` on it (record_store.cpp:861) and copies `kKeptTables` (record_store.cpp:812) into it.

The three-table conversion goes at the end of `set_up_schema`, as its last step, right after `delete_auto_results` and before the constructor's `copy_upgrade`. Three reasons. First, it reads `results` in schema 4 form, which `upgrade_results_key` guarantees by then. Second, `delete_auto_results` deletes by `rules_fp` on `results` and marks itself done in `meta`; leaving it before the conversion means it keeps its SQL unchanged for the one open that still has a `results` table. On a file already converted, it must not run its DELETE at all (there is no `results` table), so it gains one guard: it runs only when `results` exists, and otherwise marks itself done. Third, the copy upgrade then copies the new tables, not `results`: `kKeptTables` becomes `{"charts", "profiles", "records", "record_paths", "meta"}`, and the fresh file the copy makes (a `RecordStore` built on an empty file) has those tables and no `results`, so `copy_table` finds them. The kept-rows filter `kNoStarsResultSql` (record_store.cpp:651, "stars IS NULL AND score IS NOT NULL") moves into the conversion: a no-stars row is dropped there, so the copy upgrade copies every row of every kept table and its filter goes. Every real no-stars row also carries a stamp this build does not accept (the v1.8.4 fixtures, `testdata/store/README.md` "Schema and stamps"), so in practice the readable-stamp rule already drops them; keeping the explicit rule costs one `OR` and keeps the test "the no-stars row is gone" honest.

The conversion runs once: it is gated on `has_table("results")`. A file with no `results` table is done (section 6). On a fresh file, `create_result_tables` no longer creates `results`; it creates the three new tables instead.

The step reports progress as `OpenStep::ConvertingRecords` (name mine), one report per row converted, with `rows_total` counted from `results` first, the way `copy_upgrade` counts (record_store.cpp:873-881). `library_jobs.cpp:127` maps `OpenStep` to the startup screen's steps in a `switch` with a fallback return, so the new value compiles without touching that file (MSVC /W4 does not warn on an unhandled enumerator); the screen shows it as "Opening" until A2 maps it. That is A2's one-line change in `src/ui/library_jobs.cpp` and `library_jobs.h` (`StoreOpenProgress::Step`).

## 4. How the interim stays green

The handoff asks whether main still builds and passes after the store tasks merge and before A2's. It would not, if the store tasks deleted the old API: `src/app/analysis.cpp` (run_batch, `save_analysis`, `prepare_row`, `analyzed_hashes`), `src/app/report.cpp` (`list_records`, `library_copies`, `naming_copy_paths`, `copies_of`, `counts()`), `src/app/dm_report.cpp` and `src/app/fill_report.cpp` (`records_by_hash`, `library_copies_by_hash`), `src/ui/app_state.cpp` (`record_key`, `get_summary`, `prepare_row`, `save_analysis`, `summarize_record`), `src/ui/library_jobs.cpp` (`get_summaries`, `record_key`), `src/cli/batch.cpp` (`PreparedRow` in `on_result`), `tools/bench.cpp` (`add_record`, `prepare_row`, `record_key`, `summarize_record`) and `tools/replay_json.cpp` (`summarize_record`, `best_path_text`; the handoff does not list this file under "callers", so A2 should add it) all call it.

Recommended way: the store tasks keep the old entry points as thin shims over the new tables until A2 has moved every caller, then one cleanup task (A1-5) deletes the shims, the old tests and the old single-owner rows together. Every merge lands on main and the suite stays green at each one, which is what the derive-once hook and rule 1 want, and the old store tests keep running against the new tables for free, which is the best check of the conversion there is. The shims are small because `Profile` is `ResultSettings` renamed: `get_summaries(ids, settings)` is `lookup(ids, settings)` with `.record.summary()` copied across; `has_record`, `get_summary` and `analyzed_hashes` already sit on top of it; `list_records` becomes `ready_records` plus names from `chart_index()`; `library_copies`, `naming_copy_paths` and `copies_of` read `chart_index()`; `counts()` returns `{distinct song ids, record_count()}`. The one shim that is not a one-liner is the write side: `save_analysis(PreparedRow)` and `add_row(PreparedRow)` have only a summary, no paths, so the shim files a record with one rank-1 row built from the summary and `from_summary = 1`, which is exactly what the conversion does to an old row, through the same internal writer. In the interim a batch's results therefore read as "converted, paths to fill", and the report still analyzes them as today; nothing a user sees changes until A2 lands.

The other way is an integration branch that main only takes when A1 and A2 are both done. It avoids the shims but leaves the tests target unbuildable between A1-2 and A2's caller tasks, so no task on the branch could run its own tests, and each merge's review would read code it cannot build. I do not recommend it.

## 5. The tasks

Each task is one Opus executor in its own worktree, inside 100 tool calls and 20 minutes, with one cold build. Within a wave no two tasks own one file; a later wave forks from the earlier wave's join tip. Files other parts may also want are listed under "shared". Every task writes its status line to `C:\Users\Patrick\.claude\hooks\state\status\<agent id>.md` with `status_append.ps1`, and commits with the Task, Agent and Session trailers.

The order: A1-0 alone first (everyone else builds against it), then A1-1 and A1-4 together, then A1-2, then A2's caller tasks, then A1-5 last.

### A1-0: the interface and ChartIndex (wave 0, alone)

Goal: put the types above into real headers, build `ChartIndex`, and give the store its scan-order read, all additive, so main builds and every later task has the names.

Depends on: nothing. Everyone forks from its join tip.

Owns: `src/store/record_store.h` (the new types, `kPathsPerRecord`, `Profile` with the `ResultSettings` alias, `RecordLookup` with the `SummaryLookup` alias, the `chart_index()` declaration), `src/store/record_store.cpp` (only two bodies: `chart_index()`, at the end of the chart-library section, reading `library_entry_select() + " ORDER BY rowid"`, and `RecordRow::summary()`), `src/store/chart_index.h` and `src/store/chart_index.cpp` (new), `tests/test_chart_index.cpp` (new; CMake globs `tests/test_*.cpp` at CMakeLists.txt:528, so no CMake edit for the test), `CMakeLists.txt` (add `chart_index.cpp` to the store library's sources), `src/app/report.h` (one line: `kDefaultReportPaths` becomes an alias of `store::kPathsPerRecord` until A2 removes it). Shared: `src/app/report.h` is A2's file; this is the only line A1 touches in it, and A2 forks after A1-0.

What it builds, in plain words. `ResultSettings` is renamed `Profile` and the old name becomes an alias, so nothing else changes yet. `RecordRow`, `PathRow`, `PreparedRecord` and `RecordLookup` are declared, with `RecordRow::summary()` defined (it fills a `PathSummary` from rank 1 and the record, and leaves every field unset when there are no paths, so `has_scored_best_path` keeps its D51 call 11 meaning). `prepare_record` is declared but not yet defined (A1-2 defines it; a declaration alone links fine because nothing calls it). `ChartIndex::from_rows` walks the rows once in the order given: the first row of each id is its naming copy, and each later row adds one to its copies; `count` adds each distinct id's copies once, with 1 for an unlisted id (D77 item 2, `copies_of`'s fallback at record_store.cpp:1445). `RecordStore::chart_index()` reads the charts table in rowid order, which is scan order (rebuild_chart_library inserts in `items` order, record_store.cpp:1574), and hands the rows to `from_rows`. It does not delete `kNamingCopiesSql`, `library_copies`, `naming_copy_paths` or `copies_of`; A1-5 does, once A2 has moved their callers.

Tests, in `tests/test_chart_index.cpp`: an index from rows with one chart in two folders gives copies 2 and the first row as naming copy; an unlisted id gives copies 1, `lists` false, `naming` null; `count` over ids with repeats adds each id once; `count` over an empty set is 0; `chart_index()` on a store whose library was built from three rows in a non-alphabetical order lists them in that order, not by name (pins scan order against `list_chart_library`'s ORDER BY name). One single-owner row, in a new `tests/single_owner/rules/store/d115-chart-index.json`: "Which charts does the library list, in what order, with what copies and naming copy?" owner `ChartIndex::from_rows`, pattern `ORDER BY rowid` over `src/`, so no second scan-order read appears. (The rows for `kNamingCopiesSql` in `store.json` stay until A1-5 deletes the constant.)

Commands it may run: `build-cpp\Release\hydra_tests.exe -sf=tests/test_chart_index.cpp` and `-sf=tests/single_owner/scan.cpp`.

Stop conditions: if `kDefaultReportPaths` has a reader that does arithmetic on its type (`int64_t`) and the alias changes a signature, keep `int64_t` for the alias and report it. If the cold build exceeds 8 minutes twice, report and stop.

### A1-1: the three tables and the conversion (wave 1, with A1-4)

Goal: create `profiles`, `records` and `record_paths` on open, convert an old file's `results` into them once, lowercase stored ids, and keep every old read and write working through the new tables.

Depends on: A1-0.

Owns: `src/store/record_store.cpp`, `src/store/record_store.h`, `src/store/stored_versions.h` (comment only: the results stamp now lives on the profile row), `tests/test_store_open.cpp`, `tests/test_store_results.cpp`, `tests/test_store_library.cpp`, `tests/store_fixtures.h`, `tests/display_fixtures.h` (`add_stale_rows` at line 114 and `old_build_row`, which write old-stamp rows through `add_row`; they keep working through the shim, so likely no edit, but the file is owned in case), `tests/db_file_sql.h` and `tests/db_file_util.h` (`kResultsRowCountSql` at db_file_sql.h:20 becomes a count of `records`, and `results_rows_in` is renamed `record_rows_in`; `tests/single_owner/rules/tests.json:1125` is its rule row and is updated with it), `tests/old_layout_fixture.h` (unchanged content; owned because its `detail_layout_sql` is what `seed_old_layout` uses and the task may need to extend it), `tests/single_owner/rules/store.json` (rows whose `owner_lines` quote `results` SQL: `not_in_library` at line 539, `rules_fp_of` at line 111, the summary-slot rows at 332 and 352, and the "Does this chart have a current result" row at 474; each gets its owner line and samples re-pointed, since an owner line that no longer matches fails the scan). Shared: `tests/test_app_click.cpp` (A2's) calls `results_rows_in` at lines 95-446 and reads `result_id FROM results` at line 62; the rename of the helper breaks its build, so A1-1 keeps `results_rows_in` as an alias of the new helper, and A2 fixes line 62 when it moves the click. `tests/ui/uitest_library.cpp` has the `startup-screen` GUI test (line 1007), whose seed `write_old_layout_library` (line 901) runs `DROP TABLE results;` on a file this build wrote (line 921) and counts rows through `kResultsRowCountSql` (line 983); after this task a fresh file has no `results` table, so the DROP fails. A1-1 changes that one statement to `DROP TABLE IF EXISTS results;` and keeps the constant's name, so the count reads `records` and the expected 2 still holds. The Analyze-button task (spec section 13) may also edit `tests/ui/uitest_library.cpp`, so the main session merges that one line by hand if both land.

What it builds. `create_result_tables` makes the three tables of section 2 (and no `results`). A new private step, the conversion, runs last in `set_up_schema` as section 3 says: when `results` exists, it opens one transaction, reports `OpenStep::ConvertingRecords`, reads every `results` row that passes `row_readable_sql()` and is not a no-stars row (`kNoStarsResultSql`), and writes each through the same internal writer the save will use: a profile row found or inserted for `(chartmode, sp_cap, lens, hyversion, rules_fp)`, a `records` row with `pathcount`, `stars`, `notecount` and `from_summary = 1`, and, when the old row has a score, one rank-1 `record_paths` row from the old summary columns (`bestpath`, `score`, `actcount`, `maxskip`, `hardest_ms`, `avgmult`, `sqin_count`, `sqout_count`), with `hardest_efill` NULL and `optimal` 1 (rank 1 always ties the top score). An old row with no score (a Ready record that kept no paths) becomes a record with no path rows and `pathcount` 0. `song_id` is `lower(hyhash)`; the same transaction runs `UPDATE charts SET md5 = lower(md5)`. Then `DROP TABLE results`. Rows an unreadable stamp drops are counted and reported in the task's final report (the user's file has 3, spec section 1). The conversion goes through C++ rather than one `INSERT ... SELECT` so that the converter and the save share one writer, and so each row can report progress; 37,696 rows in one transaction through cached statements is well under a second on SQLite.

`delete_auto_results` gains the `has_table("results")` guard (section 3). `stamped_fill_rule` (record_store.cpp:1138) probes "does the file hold any result" with `SELECT 1 FROM results LIMIT 1`; after the conversion the table is `records`. Its one caller before the conversion is `upgrade_results_key` on a schema 2 file, when `results` still exists, so the probe takes the table name from a private helper that answers from `results` while it exists and `records` after; one function, two tables, chosen by `has_table`. `counts()` becomes a shim over `record_count()` and a `COUNT(DISTINCT song_id)`.

The copy upgrade: `kKeptTables` becomes the five tables, the `kept_rows` filter goes, and `copy_table` learns WITHOUT ROWID: it asks `pragma_table_list` (the vendored SQLite is 3.46.0, `third_party/sqlite/sqlite3.h:149`; `table_list` arrived in 3.37) for the table's `wr` flag and inserts `rowid` into the column list only for a rowid table whose key is not an INTEGER PRIMARY KEY. The seed test `seed_old_layout` (test_store_open.cpp:158) builds the old layout by writing through today's store and then running `detail_layout_sql`, which renames `results`; after this task a store no longer has a `results` table to rename, so the seed changes to write the old `results` table directly with `kDetailLayoutResultsTableSql` and plain INSERTs of the same six rows (the fixture already holds that literal SQL at old_layout_fixture.h:19). The real-release fixture test (test_store_open.cpp:507) keeps its four files and its step lists gain `ConvertingRecords` after the key step; its SQL checks on `results` become checks on `records` joined to `profiles`.

The old API, re-pointed. `get_summaries` is rewritten over the new tables: resolve the sibling profiles for the wanted mode, cap and lens in one query over `profiles`, rank each with `rank_row` (record_store.cpp:396; `result_id` becomes the profile id as the newest-write tiebreak), then one query over `records` for the ids, and `WinnerPicker` (record_store.cpp:461) picks per chart as today, with the profile's rank as the candidate's. `list_records` reads rank-1 rows joined to `records` and `profiles`, keeps its signature, and names rows from `chart_index()` instead of `kNamingCopiesSql`. `analyzed_hashes` and `has_record` stay on top of `get_summaries`. `write_row` becomes the shim described in section 4: a `PreparedRow` becomes one record with `from_summary = 1` through the internal writer; `delete_results_where` goes with the table. `purge_charts_not_in_library` deletes from `records` and `record_paths` with `not_in_library("records.song_id")` and then any emptied profile; `not_in_library` keeps its one SQL spelling.

This task does not add the new public `save`, `lookup`, `ready_records` or `prepare_record`; that is A1-2. It does add the private pieces they share: `profile_id_for(profile, hyversion, rules_fp)` (find or insert), `write_record(song_id, profile_id, const RecordRow&)` (delete the chart's rows under that profile, insert the record and its paths) and `drop_empty_profiles()`.

Tests. The whole of `test_store_open.cpp`, `test_store_results.cpp` and `test_store_library.cpp` must pass unchanged in meaning; that is the point of the shims. New cases in `test_store_open.cpp`: a converted file holds one `profiles` row per distinct old key and stamp (the seed's rows give a known number); a converted Ready row reads back the same `PathSummary` as before (the existing `check_upgraded_rows` does this); a converted row's `from_summary` is 1 and a shim-written row's is 1; an old row with no score converts to a record with `pathcount` 0 and no path rows; an uppercase `hyhash` and `md5` in the seed come out lowercase; a second open reports `Opening` alone and converts nothing; the conversion inside a file that also has the `paths` table runs before the copy and the copied file holds the five tables. A failing-first test for the WITHOUT ROWID copy: seed a converted file with a `paths` table so the copy upgrade runs; before the `copy_table` fix it fails with "no such column: rowid" (keep the exact red line for the report). In `test_store_results.cpp` the one raw-SQL check at line 499 (`hex(rules_fp)` on `results`) moves to `profiles`; `test_store_library.cpp:181` counts a chart's rows on the file and moves to `records`.

Commands: `build-cpp\Release\hydra_tests.exe -sf=tests/test_store_open.cpp`, `-sf=tests/test_store_results.cpp`, `-sf=tests/test_store_library.cpp`, `-sf=tests/single_owner/scan.cpp`, `-sf=tests/test_app_click.cpp` once, because it writes through the click and counts rows, and `build-cpp\Release\hydra_uitest.exe --test startup-screen` once (build it with `.\build_cpp.ps1 -Target hydra_uitest`).

Single-owner rows: update the five `store.json` rows named above; add, in `tests/single_owner/rules/store/d115-records.json`, "Which rows does a record's profile match?" owner `profile_id_for` (pattern: `FROM profiles WHERE` outside the owner), and "Which stored rows belong to a chart no library row lists?" keeps its owner `not_in_library` with the new owner line.

Stop conditions. If the conversion of the `seed_old_layout` file cannot be made to pass through the old `check_upgraded_rows` without changing what a status means, stop and report which row and why: that is a design question, not a code one. If `pragma_table_list` is missing from the vendored build (compile error on the probe), fall back to reading `sql FROM sqlite_master` for the words `WITHOUT ROWID` and say so. If the task reaches 80 tool calls with the conversion passing but the shims not, commit the conversion with `results` kept (do not drop it) and report that A1-2 must drop it; the file then holds both for one wave, which only dev machines ever see.

### A1-4: Settings::profile() (wave 1, with A1-1)

Goal: one builder of the current profile in the app, plus its two named variants.

Depends on: A1-0 (for `store::Profile`).

Owns: `src/app/config.h`, `src/app/config.cpp`, `tests/test_config.cpp`. Shared: none; `record_key` stays for now (A1-5 deletes it after A2 moves `app_state.cpp:157,321,878` and `library_jobs.cpp:68`).

What it builds. `Settings::profile()` returns `{chartmode_key(), cap_query(), lens()}` (config.cpp:363-376 already own the three parts). `record_key(hyhash)` becomes `{hyhash, profile()}` so the two cannot drift while both exist. `profile_at_clone_hero_cap()` is `profile()` with `cap = CapQuery::at(kCloneHeroSpCap)`, which is what `dm_report.cpp:73` forces by hand today. `profile_with_fills(rule)` is `profile()` with the lens's `legacy_fills` set through `Lens::from` (the owner of that encoding, store.json:133), which is what `fill_report.cpp:221-223` edits by hand. `KeyPart` (config.h:278-287) stays as the table of which setting lands in which part.

Tests in `test_config.cpp`: `profile()` equals `record_key({}).settings()` for the default settings and for one with every key part changed (pins the interim equality); `profile_at_clone_hero_cap()` differs from `profile()` only in the cap when the cap is 8; `profile_with_fills(Ch10)` differs only in `legacy_fills`. Single-owner row, `tests/single_owner/rules/app/d115-profile.json`: "Under which settings is a chart analyzed (its profile)?" owner `Settings::profile`, pattern `\{\s*chartmode_key\(\)\s*,\s*cap_query\(\)\s*,\s*lens\(\)\s*\}` and `filed_under\(\)|PageSettings::of\(|old_lens\.legacy_fills\s*=|CapQuery::at\(kCloneHeroSpCap\)` scoped to `src/app` and `src/ui`, with the four current callers listed in `tests/single_owner/baseline.json` under `known_copies` (fields `question, file, line_text, removed_by, date, added_by, remove_by`, scan.cpp:271) with `removed_by` naming the A2 task and `remove_by` at most 14 days after `date` (`kBaselineMaxDays`, scan.cpp:154; the scan fails an entry past its date, scan.cpp:807), so the row is live now and A2's moves shrink the baseline. The main session sets the date; if A2 may take longer than 14 days, the row goes in with A1-5 instead.

Commands: `build-cpp\Release\hydra_tests.exe -sf=tests/test_config.cpp` and `-sf=tests/single_owner/scan.cpp`.

Stop conditions: if the baseline mechanism refuses an entry for a reason the executor cannot read off `scan.cpp`, drop the row to its narrow form (the `{chartmode_key(), cap_query(), lens()}` pattern alone) and report that A1-5 widens it.

### A1-2: the one save and the one lookup (wave 2)

Goal: the new public write and read API of section 1, the widened dead-row purge, emptied profiles, `needs_paths`, and the size measurement.

Depends on: A1-1 (the tables and the private writer) and A1-0.

Owns: `src/store/record_store.cpp`, `src/store/record_store.h`, `tests/test_store_records.cpp` (new: every test of the new API goes here, so the old test files stay A1-1's shape until A1-5 deletes them), `tests/store_fixtures.h` (may add a fixture that returns a `PreparedRecord`). Shared: none in this wave. A2's caller tasks fork from this task's join tip.

What it builds. `prepare_record(song_id, profile, analysis)` does today's `prepare_row` checks (record_store.cpp:579-607), then walks `analysis.all_paths()` (which `report::chart_rows` at report.cpp:78 and `summarize_record` at record_store.cpp:568 both walk today) and makes a `PathRow` per path up to `kPathsPerRecord`, ranked from 1, with `summarize_path`'s facts (record_store.cpp:537), `hardest_efill` from the activations' `e_difficulty()` as `chart_rows` computes it (report.cpp:97-101), `optimal` from `analysis.is_optimal(*path)`, and `avgmult` unrounded. The record's `pathcount` is `all_paths().size()`, `stars` is rank 1's `path_stars`, `notecount` is rank 1's `Path::notecount` (model.h:740; the spec names `Song::note_count`, but the code the summary reads today is `path.notecount` at record_store.cpp:556, and every path of one analysis carries the same count, which the test below pins) when there are paths and 0 otherwise (today's summary leaves it unset when there are no paths; the record column is NOT NULL so it stores 0, and `summary()` leaves the field unset when there are no paths, so nothing shown changes). After this `summarize_record`, `summarize_path` and `best_path_text` still exist as shims (`summarize_record(rec)` is `prepare_record(...).record.summary()` is not possible without a profile; so they keep their bodies until A1-5, and the new code does not call them). `RecordStore::save(prepared)` takes the lock, opens a SAVEPOINT inside a save group or a transaction outside one (today's `save_analysis` frame, record_store.cpp:1148-1175), and in it: deletes the chart's records and paths under every profile whose `hyversion` fails `row_readable_sql()` (the widened first purge: this chart, any mode), finds or inserts the profile for `(prepared.profile, current_record_version(), prepared.rules_fp)`, writes the record through `write_record`, and drops any profile with no records left. `lookup` and `lookup_one` return `RecordLookup` over the same resolution `get_summaries` got in A1-1, reading the record and its paths for Ready winners in one more query over `record_paths` for those ids. `ready_song_ids` and `ready_records` sit on top. `get_summaries`, `has_record`, `analyzed_hashes` and `list_records` become one-line shims over them. `save_analysis(PreparedRow)` and `add_row` stay as the from-summary shim.

The size measurement (spec section 3, "the plan's first task measures"). The path rows can only be written once `save` exists, so it lives here. A doctest case in `test_store_records.cpp`, `"store: path rows measured on the corpus"`, analyzes every chart under `testdata/input` (the way `test_store_results.cpp:78` already walks the corpus and config matrix) under the default profile, saves each through `save` into a temp file store, closes it, and then: copies it with `VACUUM INTO` and takes that file's size (A); runs `DELETE FROM record_paths` on a second copy, `VACUUM INTO` again, and takes its size (B); and does the same with `DELETE FROM records` for a third (C). The test pins only that every record's path rows read back equal to `prepare_record`'s, row by row, and prints bytes per path row ((A - B) / path rows), bytes per record ((B - C) / records) and the counts with `MESSAGE`, which doctest shows in the run's output. The vendored SQLite is built without `dbstat` (CMakeLists.txt:194 lists only `SQLITE_DEFAULT_MEMSTATUS=0` and `SQLITE_OMIT_SHARED_CACHE`), which is why the measurement subtracts compacted files, the same way the spec's 1,884,160-byte figure was taken. The executor reports the three numbers, and scales them in its report as: `bytes per path row x 4.08 x 18,848 + bytes per record x 18,848`, where 4.08 paths per record and 18,848 charts are the user's file's figures (spec section 3, read 2026-10-11), with the caveat that the corpus's 115 charts may have longer or shorter path strings than the library's 12-character average. No number in the test is pinned, so a corpus change never breaks it.

Tests in `test_store_records.cpp`: a saved analysis reads back Ready with the same paths, in rank order, rank 1 equal to the old `summarize_record`; every path of the corpus fixture carries the same `notecount` (so storing it once per record loses nothing); a save under the same profile replaces the record and its paths (five old rows never linger when the new record keeps two); a save deletes the chart's records under an old-stamp profile in another mode (the widened purge; write the old-stamp row with `add_row` first, then save, then check the other mode is NotAnalyzed, not Stale); a save keeps a readable record under other rules (D51 call 8; it reads Stale with `stale_rules`); a profile with no records left is gone after the purge (count `profiles` on the file); the three statuses against sibling profiles: Ready under the current profile, Stale with `stale_build` under another stamp, Stale with `stale_rules` under other rules, NotAnalyzed under another mode or cap or lens even when a record exists elsewhere; `needs_paths()` is true for a record `add_row` wrote and false for one `save` wrote; `ready_song_ids` equals the set `lookup` calls Ready; `ready_records` lists every Ready record best score first and none Stale; a save inside a save group is one SAVEPOINT and a failed one undoes only its chart (mirror of test_store_open.cpp:615); the corpus measurement case above.

Commands: `build-cpp\Release\hydra_tests.exe -sf=tests/test_store_records.cpp`, `-sf=tests/test_store_results.cpp`, `-sf=tests/single_owner/scan.cpp`. The corpus case may take a minute; run it once.

Single-owner rows, in `tests/single_owner/rules/store/d115-records.json`: "How does an analysis become a record and its path rows?" owner `prepare_record` (pattern `all_paths\(\)` in `src/store` and `src/app` outside the owner, with `chart_rows` and `summarize_record` listed in the baseline for A2/A1-5); "Which record does a save replace, and which rows does it purge?" owner `RecordStore::save` (pattern `DELETE FROM records` outside the owner's helper); "Does this record still need its paths?" owner `RecordRow::needs_paths` (pattern `from_summary` in `src/` outside `src/store`).

Stop conditions. If the corpus measurement runs past 3 minutes, cap the case at the first 40 charts and say so in the report (the per-row figures still hold). If `save` inside a group cannot reuse `save_analysis`'s frame without changing `BatchWrites` or the group flags, stop and report; those are D86's and not this change's.

### A1-5: the cleanup (last, after A2's caller tasks)

Goal: delete the old API, the old tests that only tested it, and the old single-owner rows, so each question has its one owner and the scan proves it.

Depends on: every A2 caller task merged (the batch, the click, the report, both comparison pages, `tools/bench.cpp`, `tools/replay_json.cpp`, `tools/compare_db.py`).

Owns: `src/store/record_store.h`, `src/store/record_store.cpp`, `tests/test_store_results.cpp`, `tests/test_store_library.cpp`, `tests/display_fixtures.h`, `tests/store_fixtures.h`, `tests/db_file_util.h`, `tests/single_owner/rules/store.json`, `tests/single_owner/rules/store/*.json`, `tests/single_owner/baseline.json`, `CONTEXT.md` (the store entries only: "Record", "Summary row", "Result identity", "Lens", and new "Profile", "Analysis", "Library row" per spec section 2; A2 and the docs task may also edit `CONTEXT.md`, so this task edits only those entries and the main session merges). Shared: `CONTEXT.md`.

What it deletes: `ResultSettings` and `SummaryLookup` aliases, `RecordKey`, `PreparedRow`, `prepare_row`, `add_record`, `add_row`, `save_analysis(PreparedRow)`, `get_summary`, `get_summaries`, `has_record`, `analyzed_hashes`, `list_records`, `RecordListing`, `SortColumn`, `counts()`, `library_copies`, `naming_copy_paths`, `copies_of`, `kNamingCopiesSql`, `summarize_record`, `summarize_path`, `best_path_text`, `write_row`, and `Settings::record_key`. Each gets a grep first; a caller still standing stops the task (report it, do not shim again). Tests that tested only the deleted functions go; tests that pin a status rule move to `test_store_records.cpp` if A1-2 did not already cover them (the executor compares case names). Rules: delete the rows for `kNamingCopiesSql` (store.json:33 and 62), `best_path_text` (85), `copies_of` (505), "How many charts does a page or its tool count?" (566, replaced by a row for `ChartIndex::count` with pattern `charts\s*\+=\s*\w+\.copies|\+=\s*\w+\.copies` over `src/app` and `src/cli`), the "Which PathSummary fields must match" and schema-2 rows if their owners went, and the baseline entries A1-4 and A1-2 added. The identity row: "How is a chart hash spelled for matching?" already exists in `app.json:649` for `normalize_chart_hash`, with a pattern that catches `to_lower_ascii(...hash)` outside the owner. It gains a second pattern, `normalize_chart_hash\(`, with `src/net/dmbot_client.cpp` the one allowed caller (an `exempt` entry, the form store.json:171 uses), so a new lowercase compare inside Hydra fails the scan, as section 8 says.

Commands: the three store test files, `tests/test_chart_index.cpp`, `tests/test_config.cpp` and `tests/single_owner/scan.cpp`.

Stop conditions: a caller of a deleted symbol outside the store means an A2 task is not merged; stop and name the file and line.

## 6. Testing the upgrade on a copy of an old-layout database

There are two kinds of fixture already. `seed_old_layout` (tests/test_store_open.cpp:158) builds a file in the summary-only-plus-paths layout from literal SQL (`tests/old_layout_fixture.h`), with one row of every status; A1-1 reshapes it so the `results` rows are inserted directly instead of written through the store. And `testdata/store/` holds four databases real releases wrote (v1.8.4 schema 2 twice, v2.0.0 schema 3, v2.1.0 schema 4; `testdata/store/README.md` pins their contents), which the test at test_store_open.cpp:507 opens from a temp copy and checks step lists and surviving rows. Both keep running; A1-1 extends their expectations to the three tables. The README's note that the v1.8.4 files keep no rows (score, no stars) stays true under the conversion's `kNoStarsResultSql` rule and under the stamp rule alike.

The real check on the user's own file is spec section 12's: the user opens a copy of `hydra.db` in the real app before merge, which fills every converted record once (about 18,848 analyses, a full library run that needs the user's yes in chat, rule 8). A1-1's executor can also run its conversion on a copy of the user's file through `hydra_batch --db <copy>` with one folder argument of a single corpus chart, which opens the store and converts without a library run; the handoff's size figures were taken on such copies.

## 7. What breaks outside the store, for A2

With the shims, nothing breaks at A1-1 or A1-2. What A2 moves, and the store function each lands on:

- `src/app/analysis.cpp:942` (`prepare_row` on the worker) and `:803` (`save_analysis`): `prepare_record` and `save`. `BatchCallbacks::on_result` (analysis.h:292) hands `src/cli/batch.cpp:233` a `PreparedRow`; it becomes a `PreparedRecord`, whose rank-1 row gives the score and path the CLI prints.
- `src/app/analysis.cpp:815` (`analyzed_hashes`) and `:245` (`BatchRun::filed_under`): `ready_song_ids(settings.profile())`.
- `src/app/report.cpp:169,184,214,234,236,260` and `src/app/report.h:134-149`: `ready_records` plus `chart_index()`; `kDefaultReportPaths` goes for `kPathsPerRecord`; `holds_results` reads `record_count()`.
- `src/app/dm_report.cpp:73,78,124` and `src/app/fill_report.cpp:221-247`: `ready_records(settings.profile_at_clone_hero_cap())` and `profile_with_fills`, with `chart_index()`.
- `src/ui/app_state.cpp:157,321,397,434,449-454,517-519,878` and `src/ui/library_jobs.cpp:68`: `lookup`, `prepare_record`, `save`, `profile()`, and exact id compares in place of `normalize_chart_hash`.
- `src/ui/library_jobs.cpp:127` and `library_jobs.h` (`StoreOpenProgress::Step`): map `OpenStep::ConvertingRecords`.
- `tools/bench.cpp:100,301`, `tools/replay_json.cpp:149-151`, `tools/compare_db.py:39-42`.
- `tests/test_app_click.cpp:62` (raw `result_id FROM results`), `tests/ui/uitest_library.cpp` (one raw count).

## 8. Questions for the user

1. The conversion keeps the explicit no-stars rule (`kNoStarsResultSql`) beside the stamp rule. Every real no-stars row also fails the stamp rule, so the explicit rule only guards the seed test. Keep it, or drop it and the seed row with it? (My pick: keep; it is one `OR`.)
2. `notecount` on a record with no paths: the old summary leaves it unset; the new column is NOT NULL and the plan stores 0 while `summary()` still reports it unset. Fine, or make the column nullable? (My pick: 0, since nothing reads it for a pathless record.)
3. The interim shims (section 4) mean that between A1-2 and A2 a batch's results read as "converted, paths to fill". No release ships from that state. Confirm that is acceptable, or ask for the integration branch instead.
4. `needs_paths` as `RecordRow::needs_paths()` (a member) rather than a `RecordStore` method. Either is one owner; say if you want the method.
5. No new threshold or number is introduced. `kPathsPerRecord` is `kDefaultReportPaths`'s 5 moved and renamed.
