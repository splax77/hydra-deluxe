// Tests for how store/record_store.{h,cpp} opens its file and fails: which
// generation a file is (store/library_file.h); a file from before 2.2.0 is
// refused or set aside, as the caller's OldFilePolicy says; a 2.2.0 to 2.7.0
// file's results become records, whatever point a stop leaves it at; every
// open sets the same journal settings; and a file, write or read that fails
// says so instead of answering wrong.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <sqlite3.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
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
#include "db_file_util.h"  // exec_on_file, scalar_on_file, record_rows_in, write_junk_db
#include "display_fixtures.h"  // add_stale_rows
#include "library_fixtures.h"  // library_row
#include "old_layout_fixture.h"  // v220_layout_sql, kV220ResultsTableSql
#include "record_fixtures.h"
#include "store/library_file.h"
#include "store/record_store.h"
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
using hydra::test::record_rows_in;

namespace {

// The bytes of the file at `path`, to show an open left it alone.
std::string file_bytes(const std::string& path) {
    std::ifstream in(std::filesystem::u8path(path), std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

// Removes the database at `path`, its side files and anything set aside
// beside it.
void remove_db(const std::string& path) {
    for (const std::string& f : with_side_files(path)) std::remove(f.c_str());
    for (const std::string& f : with_side_files(set_aside_path(path))) std::remove(f.c_str());
}

// What both policies do with the file too old to convert at `path`. Refuse
// throws OldLibraryRefused naming the file and leaves it byte for byte as it
// was. SetAside moves it to set_aside_path, replacing a file already there,
// and opens a fresh library in its place; the next open sets nothing aside.
void check_too_old_file(const std::string& path) {
    REQUIRE(classify_library_file(path) == LibraryFileKind::TooOld);
    const std::string before = file_bytes(path);
    REQUIRE_FALSE(before.empty());
    const std::string aside = set_aside_path(path);

    try {
        RecordStore store(path, OldFilePolicy::Refuse, core::default_stamp());
        FAIL("a file too old to convert opened under Refuse");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::OldLibraryRefused);
        CHECK(std::string(e.what()) == path);
    }
    CHECK(file_bytes(path) == before);
    CHECK_FALSE(file_exists_utf8(aside));
    CHECK_FALSE(side_files_present(path));

    // An earlier set-aside file waits at the name, to be replaced.
    write_junk_db(aside);
    {
        RecordStore store(path, OldFilePolicy::SetAside, core::default_stamp());
        CHECK(store.set_aside_path() == std::optional<std::string>(aside));
        CHECK(store.counts().second == 0);
        CHECK(store.chart_library_count() == 0);
    }
    CHECK(file_bytes(aside) == before);
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    CHECK(record_rows_in(path) == 0);
    {
        RecordStore store(path, OldFilePolicy::SetAside, core::default_stamp());
        CHECK_FALSE(store.set_aside_path().has_value());
    }
    CHECK(file_bytes(aside) == before);
}

// A copy of one file in testdata/store at a fresh temp path. The store opens
// in WAL mode and a set-aside renames, so neither may touch the checked-in
// file.
std::string copy_of_old_database(const std::string& file, const std::string& tag) {
    const std::string source = corpus::testdata_root() + "/store/" + file;
    REQUIRE(std::filesystem::exists(source));
    const std::string path = testtemp::temp_path(tag, ".db");
    remove_db(path);
    std::filesystem::copy_file(source, path, std::filesystem::copy_options::overwrite_existing);
    return path;
}

}  // namespace

// ---- store correctness (2026-09-26 audit, Task 1) --------------------------

TEST_CASE("a database from Hydra 1.6 or older is too old: refused, or set aside for a fresh "
          "library") {
    // Hydra 1.6's own records table, keyed by chart with no profile.
    const std::string path = testtemp::temp_path("old_records", ".db");
    remove_db(path);
    { RecordStore seed(path); }
    exec_on_file(path,
                 "DROP TABLE record_paths; DROP TABLE records; DROP TABLE profiles;"
                 "PRAGMA user_version = 1;"
                 "CREATE TABLE records (hyhash TEXT NOT NULL, chartmode TEXT NOT NULL,"
                 " hyversion TEXT NOT NULL, sp_cap INTEGER NOT NULL, bestpath TEXT NOT NULL,"
                 " blob BLOB NOT NULL, score INTEGER, actcount INTEGER, maxskip INTEGER,"
                 " hardest_ms REAL, avgmult REAL, notecount INTEGER, sqin_count INTEGER,"
                 " sqout_count INTEGER, pathcount INTEGER,"
                 " PRIMARY KEY (hyhash, chartmode, sp_cap));"
                 "INSERT INTO records (hyhash, chartmode, hyversion, sp_cap, bestpath, blob,"
                 " score) VALUES ('old', 'mode', '1.6.0', 8, '1 2 3', x'00', 100);");
    check_too_old_file(path);
    remove_db(path);
}

TEST_CASE("store: a new database carries 0 in user_version and reads its own rows") {
    // D53 item 1: Hydra never writes the user_version slot. classify_library_file
    // reads a file's tables, not that slot, so a new file keeps the 0 SQLite
    // gives a file nobody wrote it on.
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

// ---- which generation a file is (store/library_file.h) ---------------------

// Written with classify_library_file already in place (task D115-A1-6a), so
// none of these could run red first.
TEST_CASE("classify_library_file names each generation of library file") {
    const std::string path = testtemp::temp_path("classify", ".db");
    // A file made fresh by `sql`, run on an empty database.
    const auto file_of = [&](const char* sql) {
        remove_db(path);
        exec_on_file(path, sql);
    };

    SUBCASE("Fresh: no file, an empty file, or SQLite's in-memory name") {
        remove_db(path);
        CHECK(classify_library_file(path) == LibraryFileKind::Fresh);
        CHECK(classify_library_file("") == LibraryFileKind::Fresh);
        CHECK(classify_library_file(":memory:") == LibraryFileKind::Fresh);
        { std::ofstream(std::filesystem::u8path(path), std::ios::binary); }
        CHECK(classify_library_file(path) == LibraryFileKind::Fresh);
    }
    SUBCASE("Current: a file this build wrote, or one with none of the layouts' tables") {
        remove_db(path);
        { RecordStore store(path); }
        CHECK(classify_library_file(path) == LibraryFileKind::Current);
        file_of("CREATE TABLE t (x)");
        CHECK(classify_library_file(path) == LibraryFileKind::Current);
    }
    SUBCASE("Convertible: the results table 2.2.0 to 2.7.0 wrote") {
        file_of(test::kV220ResultsTableSql);
        CHECK(classify_library_file(path) == LibraryFileKind::Convertible);
    }
    SUBCASE("TooOld: each older layout's own evidence") {
        file_of("CREATE TABLE paths (x)");
        CHECK(classify_library_file(path) == LibraryFileKind::TooOld);
        file_of("CREATE TABLE records (hyhash TEXT)");
        CHECK(classify_library_file(path) == LibraryFileKind::TooOld);
        file_of((std::string(test::kV220ResultsTableSql) +
                 "; ALTER TABLE results ADD COLUMN structure BLOB")
                    .c_str());
        CHECK(classify_library_file(path) == LibraryFileKind::TooOld);
        // A results table missing any one of the three columns 2.2.0 wrote.
        file_of("CREATE TABLE results (hyhash TEXT, legacy_fills INTEGER, rules_fp BLOB)");
        CHECK(classify_library_file(path) == LibraryFileKind::TooOld);
        file_of("CREATE TABLE results (hyhash TEXT, stars INTEGER, rules_fp BLOB)");
        CHECK(classify_library_file(path) == LibraryFileKind::TooOld);
        file_of("CREATE TABLE results (hyhash TEXT, stars INTEGER, legacy_fills INTEGER)");
        CHECK(classify_library_file(path) == LibraryFileKind::TooOld);
    }
    SUBCASE("Unreadable: a file SQLite cannot read") {
        remove_db(path);
        write_junk_db(path);
        CHECK(classify_library_file(path) == LibraryFileKind::Unreadable);
    }
    remove_db(path);
}

// ---- converting a 2.2.0 to 2.7.0 file (D115 task A1-6a) --------------------

namespace {

// The keys the 2.2.0 seed writes, all at an 8-bar cap.
const CapQuery kAt8 = CapQuery::at(8);
const RecordKey kReady{"ready", "mode", kAt8};
const RecordKey kOtherMode{"ready", "other mode", kAt8};
const RecordKey kNoPaths{"empty", "mode", kAt8};
const RecordKey kBuild{"build", "mode", kAt8};
const RecordKey kRules{"rules", "mode", kAt8};
const RecordKey kBoth{"both", "mode", kAt8};
const RecordKey kNoStars{"nostars", "mode", kAt8};

// What the seed holds, counted once by hand: one library row per chart (six
// charts), seven results, and two meta rows (the library's reader stamp and
// the fill-rule stamp).
constexpr int64_t kSeedCharts = 6;
constexpr int64_t kSeedResults = 7;
constexpr int64_t kSeedMeta = 2;
// The results the conversion keeps: all but the no-stars row. Each becomes
// one record.
constexpr int64_t kKeptResults = 6;
// The profiles those records are filed under, counted once by hand: one per
// distinct mode, cap, lens, stamp and rules among the kept rows. "ready" and
// "empty" share one; "other mode", "build", "rules" and "both" have one each.
constexpr int64_t kSeedProfiles = 5;
// The path rows the kept records hold: one rank-1 row for each kept result
// with a score, so every one but "empty".
constexpr int64_t kSeedPaths = 5;
// The fill-rule label 2.2.0 to 2.7.0 kept for a whole file, as the seed
// writes it. The conversion deletes it (D115 task A1-6c).
constexpr const char* kSeedLabelSql = "INSERT INTO meta (key, value) VALUES ('engine_mode', 'ch10')";
constexpr const char* kSeedLabelCountSql = "SELECT COUNT(*) FROM meta WHERE key = 'engine_mode'";

// A database at `path` in the layout 2.2.0 to 2.7.0 wrote, with one row of
// every kind the conversion treats differently, its files closed.
void seed_v220_layout(const std::string& path) {
    remove_db(path);
    {
        RecordStore seed(path);
        seed.rebuild_chart_library({library_row("ready", "Ready"), library_row("empty", "Empty"),
                                    library_row("build", "Build"), library_row("rules", "Rules"),
                                    library_row("both", "Both"),
                                    library_row("nostars", "No stars")});
        seed.add_record(kReady, at_cap(8));
        seed.add_record(kOtherMode, at_cap(8));
        seed.add_record(kNoPaths, no_paths_at_cap(8));
        test::add_stale_rows(seed, at_cap(8), kBuild, kRules, kBoth);
        seed.add_record(kNoStars, at_cap(8));
    }
    exec_on_file(path, test::v220_layout_sql().c_str());
    exec_on_file(path, kSeedLabelSql);
    // A row from before the stars column: a score and no stars. Its stars
    // needed the stored paths to fill, so the conversion leaves it behind.
    // The no-paths row has neither, by design, and stays (R17).
    exec_on_file(path, "UPDATE results SET stars = NULL WHERE hyhash = 'nostars'");
    // An id an older build stored in capitals, in both tables: the conversion
    // lowercases them (ADR 0028), so the chart still reads under "ready".
    exec_on_file(path, "UPDATE results SET hyhash = 'READY' WHERE hyhash = 'ready';"
                       "UPDATE charts SET md5 = 'READY' WHERE md5 = 'ready';");
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM charts") == kSeedCharts);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM results") == kSeedResults);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM meta") == kSeedMeta);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE hyhash = 'empty'"
                                 " AND stars IS NULL AND score IS NULL") == 1);
    REQUIRE(classify_library_file(path) == LibraryFileKind::Convertible);
}

// What every row reads after the conversion: Ready stays Ready with its
// numbers, Stale stays Stale for the same reason, the no-stars row is gone,
// and the library and its reader stamp stayed whole.
void check_converted_rows(RecordStore& store) {
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
    // The library row stored in capitals reads lowercase, as its record does.
    CHECK(store.list_chart_library_copies("ready").size() == 1);
    // The scan cache reads only under a current reader stamp, so a full one
    // shows the stamp stayed in meta.
    CHECK(store.chart_library_cache().size() == static_cast<size_t>(kSeedCharts));
}

// The file is in this build's layout, with no results table left and
// nothing set aside beside it.
void check_converted_file(const std::string& path) {
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM sqlite_master WHERE name = 'results'") == 0);
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    CHECK_FALSE(file_exists_utf8(set_aside_path(path)));
    // The whole-file fill-rule label is gone; each record's profile names
    // its rule.
    CHECK(scalar_on_file(path, kSeedLabelCountSql) == 0);
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

TEST_CASE("the first open of a 2.2.0 to 2.7.0 file turns its results into records and keeps "
          "every status") {
    const std::string path = testtemp::temp_path("convert_v220", ".db");
    seed_v220_layout(path);

    ProgressLog log;
    {
        RecordStore store(path, core::default_stamp(), log.fn());
        CHECK_FALSE(store.set_aside_path().has_value());
        check_converted_rows(store);
    }
    check_converted_file(path);
    // The results became records: one profile per distinct settings, stamp
    // and rules, one record per kept row, a rank-1 path row for each with a
    // score, and every id lowercase.
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == kSeedProfiles);
    CHECK(record_rows_in(path) == kKeptResults);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM record_paths") == kSeedPaths);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM record_paths WHERE rank <> 1") == 0);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM records WHERE song_id <> lower(song_id)") ==
          0);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM charts WHERE md5 <> lower(md5)") == 0);

    // The bar's numbers: the conversion counts every row it converts up to
    // its total.
    CHECK(log.steps() == std::vector<OpenStep>{OpenStep::Opening, OpenStep::ConvertingRecords});
    int64_t last_converted = -1;
    for (const OpenProgress& p : log.reports) {
        if (p.step == OpenStep::Opening) continue;
        CHECK(p.rows_total == kKeptResults);
        CHECK(p.rows_done >= last_converted);
        last_converted = p.rows_done;
    }
    CHECK(last_converted == kKeptResults);

    // A second open finds no results and does nothing.
    ProgressLog again;
    {
        RecordStore store(path, core::default_stamp(), again.fn());
        CHECK(store.counts().second == kKeptResults);
    }
    CHECK(again.steps() == std::vector<OpenStep>{OpenStep::Opening});
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == kSeedProfiles);
    remove_db(path);
}

// An old row's rules fingerprint can be the empty blob. The conversion files
// it under a profile whose rules are that empty blob, not NULL, which the
// profiles table refuses: before bind_blob bound an empty blob as one, the
// whole open failed on "NOT NULL constraint failed: profiles.rules_fp".
TEST_CASE("the conversion keeps a row whose rules fingerprint is the empty blob") {
    const std::string path = testtemp::temp_path("convert_empty_rules_fp", ".db");
    seed_v220_layout(path);
    exec_on_file(path, "UPDATE results SET rules_fp = x'' WHERE hyhash = 'rules'");
    {
        RecordStore store(path);
        const SummaryLookup got_rules = store.get_summary(kRules);
        CHECK(got_rules.status == RecordStatus::Stale);
        CHECK(got_rules.stale_rules);
    }
    CHECK(record_rows_in(path) == kKeptResults);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles WHERE typeof(rules_fp) = 'blob'"
                               " AND length(rules_fp) = 0") == 1);
    remove_db(path);
}

TEST_CASE("an old result with no score converts to a record that kept no paths") {
    // The seed's "empty" row: analyzed, no paths kept, so no score.
    const std::string path = testtemp::temp_path("convert_no_score", ".db");
    seed_v220_layout(path);
    { RecordStore store(path); }
    CHECK(scalar_on_file(path, "SELECT pathcount FROM records WHERE song_id = 'empty'") == 0);
    CHECK(scalar_on_file(path, "SELECT notecount FROM records WHERE song_id = 'empty'") == 0);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM records WHERE song_id = 'empty'"
                               " AND stars IS NULL") == 1);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM record_paths WHERE song_id = 'empty'") == 0);
    remove_db(path);
}

TEST_CASE("a stop asked mid-conversion leaves the results table whole and throws Cancelled") {
    const std::string path = testtemp::temp_path("convert_cancelled", ".db");
    seed_v220_layout(path);
    const OpenProgressFn stop_at_two = [](const OpenProgress& p) {
        return !(p.step == OpenStep::ConvertingRecords && p.rows_done == 2);
    };
    try {
        RecordStore store(path, core::default_stamp(), stop_at_two);
        FAIL("the open finished although the callback asked it to stop");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::Cancelled);
    }
    // The conversion is one transaction: the two records it had filed are
    // gone with it, and the next open converts every row.
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM results") == kSeedResults);
    CHECK(record_rows_in(path) == 0);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == 0);
    CHECK(classify_library_file(path) == LibraryFileKind::Convertible);
    {
        RecordStore store(path);
        check_converted_rows(store);
    }
    check_converted_file(path);
    remove_db(path);
}

// ---- setting a file too old to convert aside (D115 task A1-6a) -------------

TEST_CASE("a set-aside that cannot be made fails with its sentence and leaves the old file "
          "whole") {
    const std::string path = copy_of_old_database("v2.1.0-schema4.db", "set_aside_blocked");
    const std::string before = file_bytes(path);
    // A folder at the set-aside name: remove_with_side_files cannot clear it,
    // so the rename fails.
    std::filesystem::create_directories(os_path(set_aside_path(path)));
    try {
        RecordStore store(path, OldFilePolicy::SetAside, core::default_stamp());
        FAIL("the open finished with nowhere to set the old file aside");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::OldLibrarySetAsideFailed);
        // The middle is Windows' own text for the failure, so only the words
        // around it are pinned here (test_user_messages pins it whole).
        const std::string said = app::plain_error(e);
        const std::string head = "This library file is from a Hydra version older than 2.2.0, "
                                 "and Hydra could not set it aside: ";
        const std::string tail = ". Close any program using hydra.db and start Hydra again.";
        CHECK(said.rfind(head, 0) == 0);
        REQUIRE(said.size() > head.size() + tail.size());
        CHECK(said.find(tail, head.size()) != std::string::npos);
    }
    std::filesystem::remove(os_path(set_aside_path(path)));
    CHECK(file_bytes(path) == before);
    remove_db(path);
}

TEST_CASE("another connection on an old file stops the set-aside, and the file stays whole") {
    const std::string path = copy_of_old_database("v2.1.0-schema4.db", "set_aside_held_open");
    const std::string before = file_bytes(path);
    sqlite3* holder = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &holder) == SQLITE_OK);
    REQUIRE(sqlite3_exec(holder, "SELECT COUNT(*) FROM results", nullptr, nullptr, nullptr) ==
            SQLITE_OK);
    try {
        RecordStore store(path, OldFilePolicy::SetAside, core::default_stamp());
        FAIL("the open set aside a file another connection holds");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::OldLibrarySetAsideFailed);
    }
    sqlite3_close(holder);
    CHECK(file_bytes(path) == before);
    CHECK_FALSE(file_exists_utf8(set_aside_path(path)));
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

// One file in testdata/store and its layout. The fields are the README's
// "Facts the tests pin".
struct OldDatabase {
    const char* file;
    int64_t user_version;
    bool has_legacy_fills_column;
    bool has_rules_fp_column;
};

// Every fixture row's chart mode (README "Schema and stamps").
const char* const kOldDatabaseChartMode = "Expert Pro Drums, 2x Bass";

// The three charts every fixture analysed (README "The three charts").
const char* const kOldDatabaseHashes[] = {"0b647b1570475f18d463466f2c4d72a7",
                                          "0d4fd734fe5f2bf6ff1fdbc4ff55be8f",
                                          "bef0759746740f9c1045f64e9a5a2938"};

using ChartPairs = std::set<std::pair<std::string, std::string>>;

// The (hyhash, chartmode) pairs the fixtures' results tables hold. Rows are
// matched by chart, since result_ids differ between the files.
ChartPairs old_database_pairs() {
    ChartPairs out;
    for (const char* h : kOldDatabaseHashes) out.insert({h, kOldDatabaseChartMode});
    return out;
}

// The (chart, chartmode) pairs an old file's results table holds.
ChartPairs result_pairs_on_file(const std::string& path) {
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

TEST_CASE("a database any release before 2.2.0 wrote is too old: refused, or set aside for a "
          "fresh library") {
    const std::vector<OldDatabase> fixtures = {
        {"v1.8.4-schema2.db", 2, false, false},
        {"v1.8.4-schema2-legacy-fills.db", 2, false, false},
        {"v2.0.0-schema3.db", 3, true, false},
        // v2.1.0 stopped setting user_version (README "Schema and stamps").
        {"v2.1.0-schema4.db", 0, true, true},
    };
    for (const OldDatabase& f : fixtures) {
        INFO(std::string(f.file));
        const std::string path = copy_of_old_database(f.file, "old_database");

        // The file is the layout the README says, before anything opens it.
        CHECK(scalar_on_file(path, "PRAGMA user_version") == f.user_version);
        CHECK(results_columns_named(path, "legacy_fills") == (f.has_legacy_fills_column ? 1 : 0));
        CHECK(results_columns_named(path, "rules_fp") == (f.has_rules_fp_column ? 1 : 0));
        REQUIRE(result_pairs_on_file(path) == old_database_pairs());

        check_too_old_file(path);
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
                 "CREATE TRIGGER refuse_boom BEFORE INSERT ON records WHEN NEW.song_id = 'boom'"
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
constexpr const char* kDropEveryTable =
    "DROP TABLE profiles; DROP TABLE records; DROP TABLE record_paths; DROP TABLE charts;"
    " DROP TABLE meta;";

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
        REQUIRE(store.analyzed_hashes(ResultSettings{"mode", CapQuery::at(4), Lens{}}).count("h") ==
                1);
        exec_on_file(path, "DROP TABLE records;");
        // The first read fails when it steps; by then the connection has
        // reloaded the schema, so the second fails when it compiles.
        for (const char* when : {"step", "compile"}) {
            INFO(std::string(when));
            check_read_fails(
                [&] { store.analyzed_hashes(ResultSettings{"mode", CapQuery::at(4), Lens{}}); });
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
        {"get_summaries",
         [&](RecordStore& s) { s.get_summaries({"h"}, ResultSettings{"mode", cap, Lens{}}); }},
        {"get_summary", [&](RecordStore& s) { s.get_summary(key); }},
        {"has_record", [&](RecordStore& s) { s.has_record(key); }},
        {"analyzed_hashes",
         [&](RecordStore& s) { s.analyzed_hashes(ResultSettings{"mode", cap, Lens{}}); }},
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
