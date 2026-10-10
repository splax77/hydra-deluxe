// Tests for how store/record_store.{h,cpp} opens its file and fails: an
// older file upgrades by copy-and-swap with real progress (D87, DBUP),
// whatever point a stop leaves it at; the tables and results older releases
// left are dropped once; every open sets the same journal settings; and a
// file, write or read that fails says so instead of answering wrong.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <sqlite3.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/winstr.h"
#include "db_file_util.h"  // exec_on_file, scalar_on_file, results_rows_in, write_junk_db
#include "display_fixtures.h"  // add_stale_rows
#include "library_fixtures.h"  // library_row
#include "old_layout_fixture.h"  // detail_layout_sql and the old layout's counts
#include "record_fixtures.h"
#include "store/record_store.h"
#include "store/stored_versions.h"
#include "store/upgrade_files.h"
#include "store_fixtures.h"  // at_cap, no_paths_at_cap
#include "temp_util.h"
#include "testdata_dirs.h"  // testdata_root

using namespace hydra;
using namespace hydra::store;

using hydra::test::at_cap;
using hydra::test::exec_on_file;
using hydra::test::no_paths_at_cap;
using hydra::test::write_junk_db;
// One integer straight out of a database file: how these tests look at a
// table without the store growing an accessor for it.
using hydra::test::scalar_on_file;
using hydra::test::results_rows_in;

// ---- store correctness (2026-09-26 audit, Task 1) --------------------------

TEST_CASE("a database from Hydra 1.6 or older opens with nothing to show") {
    // User decision 2026-09-26: the pre-1.7 migrations are gone. The old
    // `records` table is left where it is and never read, so its charts read
    // Not analyzed until they are analyzed again. Those rows could not be
    // read since 1.8.1 anyway.
    const std::string path = testtemp::temp_path("old_records", ".db");
    std::remove(path.c_str());
    { RecordStore seed(path); }
    exec_on_file(path,
                 "DROP TABLE results;"
                 "PRAGMA user_version = 1;"
                 "CREATE TABLE records (hyhash TEXT NOT NULL, chartmode TEXT NOT NULL,"
                 " hyversion TEXT NOT NULL, sp_cap INTEGER NOT NULL, bestpath TEXT NOT NULL,"
                 " blob BLOB NOT NULL, score INTEGER, actcount INTEGER, maxskip INTEGER,"
                 " hardest_ms REAL, avgmult REAL, notecount INTEGER, sqin_count INTEGER,"
                 " sqout_count INTEGER, pathcount INTEGER,"
                 " PRIMARY KEY (hyhash, chartmode, sp_cap));"
                 "INSERT INTO records (hyhash, chartmode, hyversion, sp_cap, bestpath, blob,"
                 " score) VALUES ('old', 'mode', '1.6.0', 8, '1 2 3', x'00', 100);");

    const RecordKey key{"old", "mode", CapQuery::at(8)};
    {
        RecordStore store(path);
        CHECK(store.counts().second == 0);
        CHECK(store.get_summary(key).status == RecordStatus::NotAnalyzed);
        CHECK_FALSE(store.has_record(key));
        // A fresh analysis lands as usual.
        store.add_record(key, at_cap(8));
        CHECK(store.get_summary(key).status == RecordStatus::Ready);
    }
    // The old table is left alone: nothing read it and nothing rewrote it.
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM records") == 1);
    std::remove(path.c_str());
}

TEST_CASE("store: a new database carries 0 in user_version and reads its own rows") {
    // D53 item 1: Hydra never writes the user_version slot. The column checks
    // in the constructor are the one upgrade gate, so a new file keeps the 0
    // SQLite gives a file nobody wrote it on.
    const std::string path = testtemp::temp_path("user_version", ".db");
    std::remove(path.c_str());
    const RecordKey key{"h", "mode", CapQuery::at(4)};
    {
        RecordStore store(path);
        store.add_record(key, at_cap(4));
    }
    CHECK(scalar_on_file(path,"PRAGMA user_version") == 0);
    {
        RecordStore reopened(path);
        CHECK(reopened.get_summary(key).status == RecordStatus::Ready);
    }
    std::remove(path.c_str());
}

// ---- the summary-only upgrade (D87 items 1 and 7, task storage-T5) ---------

namespace {

// The old layout's SQL lives in old_layout_fixture.h; these run it here.
void to_detail_layout(const std::string& path) {
    exec_on_file(path, test::detail_layout_sql().c_str());
}

int64_t detail_tables_in(const std::string& path) {
    return scalar_on_file(path, test::kDetailTablesCountSql);
}

int64_t structure_columns_in(const std::string& path) {
    return scalar_on_file(path, test::kStructureColumnCountSql);
}

}  // namespace

namespace {

// The keys the old-layout seed writes, all at an 8-bar cap.
const CapQuery kAt8 = CapQuery::at(8);
const RecordKey kReady{"ready", "mode", kAt8};
const RecordKey kOtherMode{"ready", "other mode", kAt8};
const RecordKey kNoPaths{"empty", "mode", kAt8};
const RecordKey kBuild{"build", "mode", kAt8};
const RecordKey kRules{"rules", "mode", kAt8};
const RecordKey kBoth{"both", "mode", kAt8};
const RecordKey kNoStars{"nostars", "mode", kAt8};

// What the seed holds, counted once by hand: one library row per chart (six
// charts), seven results, and three meta rows (the library's reader stamp,
// the fill-rule stamp and the Auto-results mark).
constexpr int64_t kSeedCharts = 6;
constexpr int64_t kSeedResults = 7;
constexpr int64_t kSeedMeta = 3;
// The results the upgrade keeps: all but the no-stars row.
constexpr int64_t kKeptResults = 6;
// The fill-rule stamp the seed writes, so the test can see meta come across.
constexpr const char* kSeedEngineMode = "seeded-engine-mode";

// A database at `path` in the layout before summary-only storage, with one
// row of every kind the upgrade treats differently, its files closed.
void seed_old_layout(const std::string& path) {
    for (const std::string& f : with_side_files(path)) std::remove(f.c_str());
    {
        RecordStore seed(path);
        seed.rebuild_chart_library({library_row("ready", "Ready"), library_row("empty", "Empty"),
                                    library_row("build", "Build"), library_row("rules", "Rules"),
                                    library_row("both", "Both"),
                                    library_row("nostars", "No stars")});
        seed.set_engine_mode(kSeedEngineMode);
        seed.add_record(kReady, at_cap(8));
        seed.add_record(kOtherMode, at_cap(8));
        seed.add_record(kNoPaths, no_paths_at_cap(8));
        test::add_stale_rows(seed, at_cap(8), kBuild, kRules, kBoth);
        seed.add_record(kNoStars, at_cap(8));
    }
    to_detail_layout(path);
    // A row from before the stars column: a score and no stars. Its stars
    // needed the stored paths to fill, so the upgrade deletes it. The
    // no-paths row has neither, by design, and stays (R17).
    exec_on_file(path, "UPDATE results SET stars = NULL WHERE hyhash = 'nostars'");
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM charts") == kSeedCharts);
    REQUIRE(results_rows_in(path) == kSeedResults);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM meta") == kSeedMeta);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE hyhash = 'empty'"
                                 " AND stars IS NULL AND score IS NULL") == 1);
    REQUIRE(detail_tables_in(path) == 5);
    REQUIRE(structure_columns_in(path) == 1);
}

// What every row reads after the upgrade: Ready stays Ready with its
// numbers, Stale stays Stale for the same reason, the no-stars row is gone,
// and the library and meta came across whole.
void check_upgraded_rows(RecordStore& store) {
    const SummaryLookup got_ready = store.get_summary(kReady);
    CHECK(got_ready.status == RecordStatus::Ready);
    CHECK(got_ready.summary == summarize_record(at_cap(8)));
    CHECK(got_ready.bestpath == best_path_text(at_cap(8)));
    CHECK(store.get_summary(kOtherMode).status == RecordStatus::Ready);
    const SummaryLookup got_empty = store.get_summary(kNoPaths);
    CHECK(got_empty.status == RecordStatus::Ready);
    CHECK_FALSE(got_empty.summary.has_scored_best_path());
    const SummaryLookup got_build = store.get_summary(kBuild);
    CHECK(got_build.status == RecordStatus::Stale);
    CHECK(got_build.stale_build);
    CHECK_FALSE(got_build.stale_rules);
    const SummaryLookup got_rules = store.get_summary(kRules);
    CHECK(got_rules.status == RecordStatus::Stale);
    CHECK_FALSE(got_rules.stale_build);
    CHECK(got_rules.stale_rules);
    const SummaryLookup got_both = store.get_summary(kBoth);
    CHECK(got_both.status == RecordStatus::Stale);
    CHECK(got_both.stale_build);
    CHECK(got_both.stale_rules);
    CHECK(store.get_summary(kNoStars).status == RecordStatus::NotAnalyzed);
    CHECK(store.counts().second == kKeptResults);
    CHECK(store.chart_library_count() == kSeedCharts);
    // The scan cache reads only under a current reader stamp, so a full one
    // shows the stamp came across with meta.
    CHECK(store.chart_library_cache().size() == static_cast<size_t>(kSeedCharts));
    CHECK(store.engine_mode() == std::optional<std::string>(kSeedEngineMode));
}

// The file is in this build's layout and nothing of the upgrade is left
// beside it.
void check_upgraded_files(const std::string& path) {
    CHECK(detail_tables_in(path) == 0);
    CHECK(structure_columns_in(path) == 0);
    CHECK_FALSE(file_exists_utf8(upgrading_path(path)));
    CHECK_FALSE(file_exists_utf8(old_path(path)));
}

// The file is still the old layout with every seeded row: an upgrade that
// failed or stopped changed nothing that matters, and left no fresh file.
void check_old_file_whole(const std::string& path) {
    CHECK(detail_tables_in(path) == 5);
    CHECK(structure_columns_in(path) == 1);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM charts") == kSeedCharts);
    CHECK(results_rows_in(path) == kSeedResults);
    CHECK_FALSE(file_exists_utf8(upgrading_path(path)));
    CHECK_FALSE(file_exists_utf8(old_path(path)));
}

void remove_db(const std::string& path) {
    for (const std::string& f : with_side_files(path)) std::remove(f.c_str());
    for (const std::string& f : with_side_files(upgrading_path(path))) std::remove(f.c_str());
    for (const std::string& f : with_side_files(old_path(path))) std::remove(f.c_str());
}

// Every report an open made, in order.
struct ProgressLog {
    std::vector<OpenProgress> reports;
    OpenProgressFn fn() {
        return [this](const OpenProgress& p) {
            reports.push_back(p);
            return true;
        };
    }
    // The steps in the order they began, each once.
    std::vector<OpenStep> steps() const {
        std::vector<OpenStep> out;
        for (const OpenProgress& p : reports)
            if (out.empty() || out.back() != p.step) out.push_back(p.step);
        return out;
    }
};

}  // namespace

TEST_CASE("the first open of a file with stored path details drops them and keeps every status") {
    const std::string path = testtemp::temp_path("summary_only_upgrade", ".db");
    seed_old_layout(path);
    const int64_t ids_before = scalar_on_file(path, "SELECT SUM(result_id) FROM results"
                                                    " WHERE hyhash <> 'nostars'");
    const int64_t chart_rowids_before = scalar_on_file(path, "SELECT SUM(rowid) FROM charts");

    ProgressLog log;
    {
        RecordStore store(path, core::default_stamp(), log.fn());
        check_upgraded_rows(store);
    }
    check_upgraded_files(path);
    // A freshly written file has no free pages from the dropped tables.
    CHECK(scalar_on_file(path, "PRAGMA freelist_count") == 0);
    // Every kept row keeps its id, and every library row its rowid (the
    // naming copy is the smallest one, kNamingCopiesSql).
    CHECK(scalar_on_file(path, "SELECT SUM(result_id) FROM results") == ids_before);
    CHECK(scalar_on_file(path, "SELECT SUM(rowid) FROM charts") == chart_rowids_before);

    // The bar's numbers: the steps in order, a total of every row the copy
    // copies, counted one by one up to that total.
    CHECK(log.steps() ==
          std::vector<OpenStep>{OpenStep::Opening, OpenStep::Copying, OpenStep::Finishing});
    const int64_t total = kSeedCharts + kKeptResults + kSeedMeta;
    int64_t last_done = -1;
    for (const OpenProgress& p : log.reports) {
        if (p.step == OpenStep::Opening) continue;
        CHECK(p.rows_total == total);
        CHECK(p.rows_done >= last_done);
        last_done = p.rows_done;
    }
    CHECK(last_done == total);
    CHECK(log.reports.back().step == OpenStep::Finishing);
    CHECK(log.reports.back().rows_done == total);

    // A second open finds no detail tables and does nothing: it doesn't even
    // apply the delete rule, which a row like this one would fail.
    exec_on_file(path, "UPDATE results SET stars = NULL WHERE hyhash = 'ready'"
                       " AND chartmode = 'other mode'");
    ProgressLog again;
    {
        RecordStore store(path, core::default_stamp(), again.fn());
        CHECK(store.counts().second == kKeptResults);
    }
    CHECK(again.steps() == std::vector<OpenStep>{OpenStep::Opening});
    CHECK(scalar_on_file(path, "SELECT SUM(result_id) FROM results") == ids_before);
    CHECK(detail_tables_in(path) == 0);
    remove_db(path);
}

TEST_CASE("a fresh file left by an earlier run is removed and the upgrade runs") {
    const std::string path = testtemp::temp_path("upgrade_stale_temp", ".db");
    seed_old_layout(path);
    write_junk_db(upgrading_path(path));
    write_junk_db(upgrading_path(path) + "-wal");
    {
        RecordStore store(path);
        check_upgraded_rows(store);
    }
    check_upgraded_files(path);
    CHECK_FALSE(file_exists_utf8(upgrading_path(path) + "-wal"));
    remove_db(path);
}

TEST_CASE("a run stopped between the two renames opens as the upgraded file") {
    const std::string path = testtemp::temp_path("upgrade_between_renames", ".db");
    seed_old_layout(path);
    stop_copy_upgrade_after_step_aside_for_test(true);
    CHECK_THROWS(RecordStore(path));
    stop_copy_upgrade_after_step_aside_for_test(false);
    // The state a kill there leaves: the original stepped aside, the fresh
    // file complete, nothing under the database's name.
    REQUIRE_FALSE(file_exists_utf8(path));
    REQUIRE(file_exists_utf8(old_path(path)));
    REQUIRE(file_exists_utf8(upgrading_path(path)));
    {
        RecordStore store(path);
        check_upgraded_rows(store);
    }
    check_upgraded_files(path);
    remove_db(path);
}

TEST_CASE("a stepped-aside original with no fresh file comes back and upgrades") {
    const std::string path = testtemp::temp_path("upgrade_old_alone", ".db");
    seed_old_layout(path);
    REQUIRE(MoveFileW(win32_path(path).c_str(), win32_path(old_path(path)).c_str()));
    {
        RecordStore store(path);
        check_upgraded_rows(store);
    }
    check_upgraded_files(path);
    remove_db(path);
}

TEST_CASE("a stop asked for mid-copy leaves the old file whole and throws Cancelled") {
    const std::string path = testtemp::temp_path("upgrade_cancelled", ".db");
    seed_old_layout(path);
    const OpenProgressFn stop_at_two = [](const OpenProgress& p) {
        return !(p.step == OpenStep::Copying && p.rows_done == 2);
    };
    try {
        RecordStore store(path, core::default_stamp(), stop_at_two);
        FAIL("the open finished although the callback asked it to stop");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::Cancelled);
    }
    check_old_file_whole(path);
    remove_db(path);
}

TEST_CASE("a fresh file that cannot be made fails as an upgrade error with the old file whole") {
    const std::string path = testtemp::temp_path("upgrade_no_temp", ".db");
    seed_old_layout(path);
    // A folder where the fresh file goes: nothing can create it.
    std::filesystem::create_directories(os_path(upgrading_path(path)));
    try {
        RecordStore store(path);
        FAIL("the open finished with no fresh file to copy into");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseUpgrade);
        CHECK(app::plain_error(e) ==
              "Hydra couldn't update its library file (hydra.db) for this version. Your charts "
              "and results were not changed. Check that no other copy of Hydra or hydra_batch "
              "is running and that the disk isn't full, then start Hydra again.");
    }
    std::filesystem::remove(os_path(upgrading_path(path)));
    check_old_file_whole(path);
    remove_db(path);
}

TEST_CASE("another connection on the old file stops the swap as an upgrade error") {
    const std::string path = testtemp::temp_path("upgrade_held_open", ".db");
    seed_old_layout(path);
    sqlite3* holder = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &holder) == SQLITE_OK);
    REQUIRE(sqlite3_exec(holder, "SELECT COUNT(*) FROM charts", nullptr, nullptr, nullptr) ==
            SQLITE_OK);
    try {
        RecordStore store(path);
        FAIL("the upgrade swapped a file another connection holds");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseUpgrade);
    }
    sqlite3_close(holder);
    check_old_file_whole(path);
    remove_db(path);
}

TEST_CASE("every open caps the WAL file the log keeps after a checkpoint") {
    // journal_size_limit is per connection, so the store sets it on every
    // open. 4194304 bytes is the value D93 chose.
    const std::string path = testtemp::temp_path("journal_size_limit", ".db");
    std::remove(path.c_str());
    for (int open = 0; open < 2; ++open) {
        RecordStore store(path);
        CHECK(store.journal_size_limit_for_test() == 4194304);
    }
    std::remove(path.c_str());
}

// ---- real old databases (testdata/store, test fidelity plan task 1) --------

namespace {

// One file in testdata/store and what today's open makes of it. The fields
// before the open are the README's "Facts the tests pin"; the fields after it
// were pinned from one run on 2026-10-10 at a5935954 (v2.0.0, v2.1.0) and at
// d6c48705 (the two v1.8.4 files).
struct OldDatabase {
    const char* file;
    int64_t user_version;
    bool has_legacy_fills_column;
    bool has_rules_fp_column;
    // The results rows the first open keeps: all 3, or none.
    int64_t rows_after_open;
    // The fill rule every kept row is filed under (0 where none is kept).
    int legacy_fills;
    // The steps the first open reported, each once.
    std::vector<OpenStep> steps;
};

// Every fixture row's chart mode (README "Schema and stamps").
const char* const kOldDatabaseChartMode = "Expert Pro Drums, 2x Bass";

// The three charts every fixture analysed (README "The three charts").
const char* const kOldDatabaseHashes[] = {"0b647b1570475f18d463466f2c4d72a7",
                                          "0d4fd734fe5f2bf6ff1fdbc4ff55be8f",
                                          "bef0759746740f9c1045f64e9a5a2938"};

// The rules fingerprint every fixture row holds, its bytes in file order as
// SQLite's lower(hex()) spells them (README "Rules fingerprint").
const char* const kOldDatabaseRulesFpHex = "f24c606966e9d270";

// The settings every fixture row was analysed under (README "Results rows by
// chart"): a 4-bar cap, the ms limit on at 10, depth "scores 4".
RecordKey old_database_key(const char* hyhash, int legacy_fills) {
    Lens lens;
    lens.ms_enabled = 1;
    lens.ms_value = 10;
    lens.depth_mode = 0;
    lens.depth_value = 4;
    lens.legacy_fills = legacy_fills;
    return RecordKey{hyhash, kOldDatabaseChartMode, CapQuery::at(4), lens};
}

using ChartPairs = std::set<std::pair<std::string, std::string>>;

// The (hyhash, chartmode) pairs the fixtures' results tables hold. Rows are
// matched by chart, since result_ids differ between the files.
ChartPairs old_database_pairs() {
    ChartPairs out;
    for (const char* h : kOldDatabaseHashes) out.insert({h, kOldDatabaseChartMode});
    return out;
}

// The (hyhash, chartmode) pairs the results table of the file at `path` holds.
ChartPairs chart_pairs_on_file(const std::string& path) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, "SELECT hyhash, chartmode FROM results", -1, &s, nullptr) ==
            SQLITE_OK);
    ChartPairs out;
    while (sqlite3_step(s) == SQLITE_ROW)
        out.insert({reinterpret_cast<const char*>(sqlite3_column_text(s, 0)),
                    reinterpret_cast<const char*>(sqlite3_column_text(s, 1))});
    sqlite3_finalize(s);
    sqlite3_close(db);
    return out;
}

int64_t results_columns_named(const std::string& path, const std::string& column) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM pragma_table_info('results') WHERE name = '" +
                                    column + "'");
}

}  // namespace

TEST_CASE("a database a real old release wrote opens with no path details, its results Stale or,"
          " before the stars column, left for a re-run") {
    using S = OpenStep;
    const std::vector<OldDatabase> fixtures = {
        // v1.8.4 had no stars column, so the copy keeps none of its rows
        // (docs/adr/0026-the-store-keeps-summaries-the-engine-gives-details.md).
        {"v1.8.4-schema2.db", 2, false, false, 0, 0,
         {S::Opening, S::UpdatingResultsKey, S::Copying, S::Finishing}},
        {"v1.8.4-schema2-legacy-fills.db", 2, false, false, 0, 0,
         {S::Opening, S::UpdatingResultsKey, S::Copying, S::Finishing}},
        {"v2.0.0-schema3.db", 3, true, false, 3, 0,
         {S::Opening, S::UpdatingResultsKey, S::Copying, S::Finishing}},
        // v2.1.0 stopped setting user_version (README "Schema and stamps"),
        // so its rules_fp column is what tells this layout apart.
        {"v2.1.0-schema4.db", 0, true, true, 3, 0, {S::Opening, S::Copying, S::Finishing}},
    };
    for (const OldDatabase& f : fixtures) {
        INFO(std::string(f.file));
        const std::string source = corpus::testdata_root() + "/store/" + f.file;
        REQUIRE(std::filesystem::exists(source));
        // The store opens in WAL mode, which would write beside the checked-in
        // file, so it opens a copy.
        const std::string path = testtemp::temp_path("old_database", ".db");
        remove_db(path);
        std::filesystem::copy_file(source, path, std::filesystem::copy_options::overwrite_existing);

        // The file is the layout the README says, before anything opens it.
        CHECK(scalar_on_file(path, "PRAGMA user_version") == f.user_version);
        CHECK(results_columns_named(path, "legacy_fills") == (f.has_legacy_fills_column ? 1 : 0));
        CHECK(results_columns_named(path, "rules_fp") == (f.has_rules_fp_column ? 1 : 0));
        REQUIRE(chart_pairs_on_file(path) == old_database_pairs());

        const bool kept = f.rows_after_open > 0;
        const ChartPairs pairs_after = kept ? old_database_pairs() : ChartPairs{};
        ProgressLog log;
        {
            RecordStore store(path, core::default_stamp(), log.fn());
            CHECK(store.counts().second == f.rows_after_open);
            for (const char* h : kOldDatabaseHashes) {
                INFO(std::string(h));
                if (kept) {
                    // Each was written by an older build, so the store's own
                    // status call reads it Stale, never Ready.
                    const SummaryLookup got =
                        store.get_summary(old_database_key(h, f.legacy_fills));
                    CHECK(got.status == RecordStatus::Stale);
                    CHECK(got.stale_build);
                } else {
                    // No row is left under either fill rule.
                    for (int legacy_fills : {0, 1})
                        CHECK(store.get_summary(old_database_key(h, legacy_fills)).status ==
                              RecordStatus::NotAnalyzed);
                }
            }
        }
        CHECK(log.steps() == f.steps);
        CHECK(results_rows_in(path) == f.rows_after_open);
        CHECK(chart_pairs_on_file(path) == pairs_after);
        CHECK(scalar_on_file(path, std::string("SELECT COUNT(*) FROM results WHERE lower(hex(rules_fp))"
                                               " <> '") +
                                       kOldDatabaseRulesFpHex + "'") == 0);
        CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE legacy_fills <> " +
                                       std::to_string(f.legacy_fills)) == 0);
        check_upgraded_files(path);

        // A second open finds nothing to upgrade and keeps what the first kept.
        ProgressLog again;
        {
            RecordStore store(path, core::default_stamp(), again.fn());
            CHECK(store.counts().second == f.rows_after_open);
        }
        CHECK(again.steps() == std::vector<OpenStep>{S::Opening});
        CHECK(results_rows_in(path) == f.rows_after_open);
        CHECK(chart_pairs_on_file(path) == pairs_after);
        remove_db(path);
    }
}

// SQLite switches a feature on when its SQLITE_ENABLE_* macro is defined at
// all, whatever its value (sqlite.org/compile.html), so the old
// "SQLITE_ENABLE_FTS5=0" compiled full-text search in. Hydra never uses it.
TEST_CASE("the vendored SQLite is built without FTS5") {
    CHECK(sqlite3_compileoption_used("ENABLE_FTS5") == 0);
}

// ---- store speed (2026-09-26 audit, Task 9) --------------------------------

TEST_CASE("a file store runs in WAL mode with an index on chart names") {
    // WAL: a commit appends to a log instead of rewriting the file in place,
    // so it costs far fewer disk syncs (https://www.sqlite.org/wal.html).
    // The journal mode is stored in the file, so a fresh connection sees it.
    const std::string path = testtemp::temp_path("wal", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, "PRAGMA journal_mode", -1, &s, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(s) == SQLITE_ROW);
    const std::string mode = reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
    sqlite3_finalize(s);
    sqlite3_close(db);
    CHECK(mode == "wal");
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM sqlite_master"
                       " WHERE type='index' AND name='charts_by_name'") == 1);
    std::remove(path.c_str());
}

TEST_CASE("a save_analysis that fails leaves nothing behind and reads as a save error") {
    // A trigger that refuses one chart's result makes the save fail.
    const std::string path = testtemp::temp_path("save_fail", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    exec_on_file(path,
                 "CREATE TRIGGER refuse_boom BEFORE INSERT ON results WHEN NEW.hyhash = 'boom'"
                 " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    {
        RecordStore store(path);
        const RecordKey boom{"boom", "mode", CapQuery::at(4)};
        try {
            store.save_analysis(prepare_row(boom, at_cap(4)));
            FAIL("the refused save went through");
        } catch (const std::exception& e) {
            CHECK(hydra::app::plain_error(e) ==
                  "Hydra couldn't save to its database (hydra.db). Check that the disk isn't "
                  "full and that no other copy of Hydra is running, then try again.");
        }
        CHECK(store.counts().second == 0);
        // The store is still usable: no transaction was left open.
        const RecordKey ok{"ok", "mode", CapQuery::at(4)};
        store.save_analysis(prepare_row(ok, at_cap(4)));
        CHECK(store.get_summary(ok).status == RecordStatus::Ready);
    }
    std::remove(path.c_str());
}

// D72 item 2: SQLite notices a file that isn't a database only at its first
// statement, after sqlite3_open_v2 said yes. It still reads "couldn't open".
TEST_CASE("a database file that isn't a database fails to open as DatabaseOpen") {
    const std::string path = testtemp::temp_path("junk_db", ".db");
    write_junk_db(path);
    try {
        RecordStore store(path);
        FAIL("a file of junk bytes opened as a database");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseOpen);
        CHECK(std::string(e.what()) == "sqlite exec failed: file is not a database");
    }
    CHECK(std::remove(path.c_str()) == 0);
}

// D72 items 2 and 3: a file another connection has locked fails at once, as
// "couldn't open", and the failed open lets go of the file.
TEST_CASE("a database another connection has locked fails to open as DatabaseOpen, and lets "
          "go of the file") {
    const std::string path = testtemp::temp_path("locked_db", ".db");
    std::remove(path.c_str());
    // A rollback-journal file (not WAL), so an exclusive lock keeps readers out.
    sqlite3* holder = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &holder) == SQLITE_OK);
    REQUIRE(sqlite3_exec(holder, "CREATE TABLE t (x); BEGIN EXCLUSIVE; INSERT INTO t VALUES (1);",
                         nullptr, nullptr, nullptr) == SQLITE_OK);
    try {
        RecordStore store(path);
        FAIL("a locked file opened");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseOpen);
        CHECK(std::string(e.what()) == "sqlite exec failed: database is locked");
    }
    REQUIRE(sqlite3_exec(holder, "ROLLBACK", nullptr, nullptr, nullptr) == SQLITE_OK);
    sqlite3_close(holder);

    { RecordStore store(path); }
    // Nothing holds the file now: SQLite opens it without delete sharing, so a
    // handle the failed open left behind would make this fail.
    std::remove((path + "-wal").c_str());
    std::remove((path + "-shm").c_str());
    CHECK(std::remove(path.c_str()) == 0);
}

namespace {

// Every table the store makes, dropped by a second connection while a store
// has the file open. The store's connection still holds the old schema, so
// its next read compiles and then fails when it steps.
constexpr const char* kDropEveryTable = "DROP TABLE results; DROP TABLE charts; DROP TABLE meta;";

// The read must throw the database read error, never answer.
void check_read_fails(const std::function<void()>& read) {
    try {
        read();
        FAIL_CHECK("a read on a failing database answered");
    } catch (const hydra::KindedError& e) {
        INFO(e.what());
        CHECK(e.kind() == hydra::ErrorKind::DatabaseRead);
        CHECK(hydra::app::plain_error(e) == hydra::app::kDatabaseReadSentence);
    }
}

}  // namespace

// D72's DB1 found this one: analyzed_hashes on a failing database answered
// with no charts, so a batch treated every chart as not analyzed.
TEST_CASE("a read on a failing database throws instead of answering empty") {
    const std::string path = testtemp::temp_path("read_fail", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4));
        REQUIRE(store.analyzed_hashes("mode", CapQuery::at(4), Lens{}).count("h") == 1);
        exec_on_file(path, "DROP TABLE results;");
        // The first read fails when it steps; by then the connection has
        // reloaded the schema, so the second fails when it compiles.
        for (const char* when : {"step", "compile"}) {
            INFO(std::string(when));
            check_read_fails([&] { store.analyzed_hashes("mode", CapQuery::at(4), Lens{}); });
        }
    }
    std::remove(path.c_str());
}

TEST_CASE("every store read throws a database read error when its step fails") {
    const std::string path = testtemp::temp_path("every_read_fail", ".db");
    std::remove(path.c_str());
    const RecordKey key{"h", "mode", CapQuery::at(4)};
    const CapQuery cap = CapQuery::at(4);
    const std::pair<const char*, std::function<void(RecordStore&)>> reads[] = {
        {"engine_mode", [](RecordStore& s) { s.engine_mode(); }},
        {"stamped_fill_rule", [](RecordStore& s) { s.stamped_fill_rule(); }},
        {"get_summaries", [&](RecordStore& s) { s.get_summaries({"h"}, "mode", cap, Lens{}); }},
        {"get_summary", [&](RecordStore& s) { s.get_summary(key); }},
        {"has_record", [&](RecordStore& s) { s.has_record(key); }},
        {"analyzed_hashes", [&](RecordStore& s) { s.analyzed_hashes("mode", cap, Lens{}); }},
        {"list_records",
         [&](RecordStore& s) {
             s.list_records(std::nullopt, cap, Lens{}, SortColumn::Score, false, std::nullopt);
         }},
        {"counts", [](RecordStore& s) { s.counts(); }},
        {"library_copies", [](RecordStore& s) { s.library_copies(); }},
        {"naming_copy_paths", [](RecordStore& s) { s.naming_copy_paths(); }},
        {"chart_library_cache", [](RecordStore& s) { s.chart_library_cache(); }},
        {"chart_library_count", [](RecordStore& s) { s.chart_library_count(); }},
        {"list_chart_library", [](RecordStore& s) { s.list_chart_library(0, 10); }},
    };
    for (const auto& [name, read] : reads) {
        INFO(std::string(name));
        // A fresh open makes the tables again, so each read fails at its own
        // first step.
        RecordStore store(path);
        exec_on_file(path, kDropEveryTable);
        check_read_fails([&] { read(store); });
    }
    std::remove(path.c_str());
}

// ---- Auto removed (2026-09-27, interface redesign Task 6) ------------------

namespace {

// A result the way Hydra 1.8.4's Auto stamped it: the fixture's paths at
// `cap`, under the default rules' Auto fingerprint.
HydraRecord auto_run_at(int cap) {
    HydraRecord r = at_cap(cap);
    r.rules_fingerprint = core::default_rules().retired_auto_fingerprint();
    return r;
}

}  // namespace

TEST_CASE("the first open deletes the results Auto saved, once") {
    const std::string path = testtemp::temp_path("auto_delete", ".db");
    std::remove(path.c_str());
    const RecordKey kept{"h", "mode", CapQuery::at(4)};
    const RecordKey whatif{"h", "mode", CapQuery::at(8)};
    const RecordKey auto_same_chart{"h", "mode", CapQuery::at(16)};
    const RecordKey auto_only{"a", "mode", CapQuery::at(32)};

    {
        RecordStore store(path);
        store.add_record(kept, at_cap(4));
        store.add_record(whatif, at_cap(8));
        store.add_record(auto_same_chart, auto_run_at(16));
        store.add_record(auto_only, auto_run_at(32));
        // This build accepts only the fixed-cap fingerprint, so an Auto row
        // reads Stale even before anything deletes it.
        CHECK(store.get_summary(auto_only).status == RecordStatus::Stale);
        CHECK(store.counts().second == 4);
    }
    // A database Hydra 1.8.4 wrote has no mark yet.
    exec_on_file(path, "DELETE FROM meta WHERE key='auto_results_deleted'");

    {
        RecordStore store(path);
        CHECK(store.counts().second == 2);
        CHECK(store.get_summary(auto_only).status == RecordStatus::NotAnalyzed);
        CHECK(store.get_summary(auto_same_chart).status == RecordStatus::NotAnalyzed);
        // A fixed-cap what-if above 4 bars is not an Auto result and stays.
        CHECK(store.get_summary(whatif).status == RecordStatus::Ready);
        const SummaryLookup left = store.get_summary(kept);
        REQUIRE(left.status == RecordStatus::Ready);
        CHECK(left.summary == summarize_record(at_cap(4)));
        CHECK(left.bestpath == best_path_text(at_cap(4)));
    }
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM meta WHERE key='auto_results_deleted'") == 1);

    // Marked done: a later open never deletes again.
    {
        RecordStore store(path);
        store.add_record(auto_only, auto_run_at(32));
    }
    {
        RecordStore store(path);
        CHECK(store.counts().second == 3);
        CHECK(store.get_summary(auto_only).status == RecordStatus::Stale);
    }
    std::error_code ec;
    std::filesystem::remove(std::filesystem::u8path(path), ec);
}

TEST_CASE("a start with a bad rules file leaves the Auto results for the next good start") {
    const std::string path = testtemp::temp_path("auto_delete_none", ".db");
    std::remove(path.c_str());
    const RecordKey auto_only{"a", "mode", CapQuery::at(32)};
    {
        RecordStore store(path);
        store.add_record(auto_only, auto_run_at(32));
    }
    exec_on_file(path, "DELETE FROM meta WHERE key='auto_results_deleted'");
    {
        // A bad hydra_rules.ini: there is no fingerprint to look for, so
        // nothing is deleted and nothing is marked done.
        RecordStore store(path, core::RulesStamp::none());
        CHECK(store.counts().second == 1);
    }
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM meta WHERE key='auto_results_deleted'") == 0);
    {
        RecordStore store(path);
        CHECK(store.counts().second == 0);
    }
    std::error_code ec;
    std::filesystem::remove(std::filesystem::u8path(path), ec);
}

// Plan task AL (p7-al2), which removed the songlength table.
TEST_CASE("opening a database that still has the old songlength table drops it") {
    // D69 replaced D58 item 5: the per-difficulty length table goes.
    const std::string path = testtemp::temp_path("song_length_old", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    exec_on_file(path,
                 "CREATE TABLE IF NOT EXISTS songlength (hyhash TEXT NOT NULL,"
                 " chartmode TEXT NOT NULL, length_ms REAL NOT NULL,"
                 " PRIMARY KEY (hyhash, chartmode));"
                 "INSERT OR REPLACE INTO songlength VALUES ('h', 'mode', 1234.0);");
    { RecordStore store(path); }
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM sqlite_master WHERE name = 'songlength'") == 0);
    std::remove(path.c_str());
}

// D86 (build settings): CMakeLists.txt builds the vendored SQLite without its
// memory-use counters and without shared cache, two options that trim its own
// CPU. A sibling of the FTS5 case above.
TEST_CASE("the vendored SQLite is built without memory counters or shared cache") {
    CHECK(sqlite3_compileoption_used("DEFAULT_MEMSTATUS=0") == 1);
    CHECK(sqlite3_compileoption_used("OMIT_SHARED_CACHE") == 1);
}

// ---- the batch writer (D86 item 3, task W1) --------------------------------

TEST_CASE("the batch guard sets the checkpoint threshold and truncates the log at the end") {
    // The threshold is per connection, so only the store's own getter can
    // read it. 1,000 is SQLite's own default, read off one run.
    const std::string path = testtemp::temp_path("batch_guard", ".db");
    std::remove(path.c_str());
    const std::filesystem::path wal = std::filesystem::u8path(path + "-wal");
    {
        RecordStore store(path);
        CHECK(store.wal_autocheckpoint_for_test() == 1000);
        {
            const RecordStore::BatchWrites batch(store);
            CHECK(store.wal_autocheckpoint_for_test() == kBatchWalAutocheckpointPages);
            const RecordKey key{"h", "mode", CapQuery::at(4)};
            store.save_analysis(prepare_row(key, at_cap(4)));
            CHECK(std::filesystem::file_size(wal) > 0);
        }
        CHECK(std::filesystem::file_size(wal) == 0);
        CHECK(store.wal_autocheckpoint_for_test() == 1000);
        CHECK(store.counts().second == 1);
    }
    std::remove(path.c_str());
}
