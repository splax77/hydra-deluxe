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
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // open_store
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/winstr.h"
#include "corpus_util.h"  // first_chart_with_paths, analyzed
#include "db_file_util.h"  // exec_on_file, scalar_on_file, record_rows_in, write_junk_db
#include "display_fixtures.h"  // add_stale_records
#include "library_fixtures.h"  // library_row
#include "old_layout_fixture.h"  // v220_layout_sql, kV220ResultsTableSql
#include "record_fixtures.h"
#include "store/chart_index.h"
#include "store/library_file.h"
#include "store/record_store.h"
#include "store/stored_versions.h"  // kResultsStamp
#include "store/upgrade_files.h"
#include "store_fixtures.h"  // at_cap, at4, fixture, saved_figures
#include "temp_util.h"
#include "testdata_dirs.h"  // testdata_root

using namespace hydra;
using namespace hydra::store;

using hydra::test::at4;
using hydra::test::at_cap;
using hydra::test::exec_on_file;
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

// Whether the profiles table in the file at `path` has the early-fill window
// column.
bool profiles_have_early_fill(const std::string& path) {
    return scalar_on_file(path, std::string("SELECT COUNT(*) FROM pragma_table_info('profiles')"
                                            " WHERE name = '") +
                                    kEarlyFillWindowColumn + "'") == 1;
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
        CHECK(store.record_count() == 0);
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

// One corpus chart as a scan and an analysis give it: the library row
// discover_charts writes for the first chart it names an album for, and that
// chart's record at 4 bars (store_fixture_settings). A chart the analysis
// cannot read is passed over.
struct ScannedChart {
    ChartLibraryEntry row;
    HydraRecord record;
};

const ScannedChart& scanned_chart() {
    static const ScannedChart found = [] {
        const auto scan = hydra::app::discover_charts({corpus::root()});
        for (const ChartLibraryEntry& item : scan.first) {
            if (item.album.empty()) continue;
            try {
                return ScannedChart{item,
                                    corpus::analyzed(item.notespath, test::store_fixture_settings())};
            } catch (const std::exception&) {
                continue;
            }
        }
        throw std::runtime_error("no corpus chart the scan names an album for can be analyzed");
    }();
    return found;
}

// The scanned chart's record, filed under its own song id and `profile`.
PreparedRecord scanned_record(const Profile& profile) {
    return prepare_record(scanned_chart().row.md5, profile, scanned_chart().record);
}

}  // namespace

// ---- store correctness (2026-09-26 audit, Task 1) --------------------------

TEST_CASE("store: a new database carries 0 in user_version and reads its own rows") {
    // D53 item 1: Hydra never writes the user_version slot. classify_library_file
    // reads a file's tables, not that slot, so a new file keeps the 0 SQLite
    // gives a file nobody wrote it on.
    const std::string path = testtemp::temp_path("user_version", ".db");
    std::remove(path.c_str());
    const Profile profile = at4();
    const PreparedRecord saved = scanned_record(profile);
    {
        RecordStore store(path);
        store.save(saved);
    }
    CHECK(scalar_on_file(path,"PRAGMA user_version") == 0);
    {
        RecordStore reopened(path);
        const RecordLookup got = reopened.lookup_one(scanned_chart().row.md5, profile);
        REQUIRE(got.status == RecordStatus::Ready);
        CHECK(got.record == saved.record);
    }
    std::remove(path.c_str());
}

// ---- which generation a file is (store/library_file.h) ---------------------

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
    SUBCASE("LacksFailureColumns: 2.8.0's records table, without the failure columns") {
        file_of("CREATE TABLE records (song_id TEXT, profile_id INTEGER, pathcount INTEGER)");
        CHECK(classify_library_file(path) == LibraryFileKind::LacksFailureColumns);
        file_of("CREATE TABLE records (song_id TEXT, profile_id INTEGER, failure_kind TEXT)");
        CHECK(classify_library_file(path) == LibraryFileKind::LacksFailureColumns);
        file_of("CREATE TABLE records (song_id TEXT, profile_id INTEGER, failure_kind TEXT,"
                " failure_text TEXT)");
        CHECK(classify_library_file(path) == LibraryFileKind::Current);
    }
    SUBCASE("LacksAlbumColumn: a charts table without the album column") {
        file_of("CREATE TABLE charts (md5 TEXT, name TEXT, artist TEXT, charter TEXT)");
        CHECK(classify_library_file(path) == LibraryFileKind::LacksAlbumColumn);
        file_of("CREATE TABLE charts (md5 TEXT, name TEXT, album TEXT)");
        CHECK(classify_library_file(path) == LibraryFileKind::Current);
        // The older generations are named first: they gain the album too.
        file_of("CREATE TABLE charts (md5 TEXT);"
                "CREATE TABLE records (song_id TEXT, profile_id INTEGER, pathcount INTEGER)");
        CHECK(classify_library_file(path) == LibraryFileKind::LacksFailureColumns);
    }
    SUBCASE("LacksEarlyFillColumn: a profiles table without the early-fill window column") {
        file_of("CREATE TABLE profiles (id INTEGER PRIMARY KEY, chartmode TEXT, legacy_fills "
                "INTEGER)");
        CHECK(classify_library_file(path) == LibraryFileKind::LacksEarlyFillColumn);
        file_of((std::string("CREATE TABLE profiles (id INTEGER PRIMARY KEY, chartmode TEXT, ") +
                 kEarlyFillWindowColumn + " INTEGER)")
                    .c_str());
        CHECK(classify_library_file(path) == LibraryFileKind::Current);
        // The older generations are named first: they gain the column too.
        file_of("CREATE TABLE charts (md5 TEXT);"
                "CREATE TABLE profiles (id INTEGER PRIMARY KEY, chartmode TEXT)");
        CHECK(classify_library_file(path) == LibraryFileKind::LacksAlbumColumn);
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

// ---- a 2.8.0 file gains the failure columns --------------------------------

TEST_CASE("a 2.8.0 file without the failure columns opens and gains them in place") {
    const std::string path = testtemp::temp_path("lacks_failure_columns", ".db");
    const Profile profile = at4();
    remove_db(path);
    // A store this build writes from a real analysis. No file a 2.8.0 release
    // wrote is checked in, so the two columns it lacked are taken out below.
    const PreparedRecord real = scanned_record(profile);
    const std::string real_id = scanned_chart().row.md5;
    {
        RecordStore seed(path);
        seed.save(real);
    }
    // 2.8.0's records table: the same table without the two columns.
    exec_on_file(path, "ALTER TABLE records DROP COLUMN failure_text;"
                       "ALTER TABLE records DROP COLUMN failure_kind;");
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksFailureColumns);
    const int64_t paths_before = scalar_on_file(path, "SELECT COUNT(*) FROM record_paths");
    {
        RecordStore store(path);
        // The old record reads as it did.
        const RecordLookup got = store.lookup_one(real_id, profile);
        REQUIRE(got.status == RecordStatus::Ready);
        CHECK(got.record == real.record);
        CHECK(store.record_count() == 1);
        // And a failure can be stored beside it.
        store.save(failure_record("failed", profile, ErrorKind::NoNotes, "No notes."));
        CHECK(store.lookup_one("failed", profile).status == RecordStatus::CantAnalyze);
    }
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM records WHERE failure_kind IS NULL"
                               " AND failure_text IS NULL") == 1);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM record_paths") == paths_before);
    // A second open finds a Current file and changes nothing.
    {
        RecordStore reopened(path);
        CHECK(reopened.lookup_one(real_id, profile).status == RecordStatus::Ready);
        CHECK(reopened.lookup_one("failed", profile).status == RecordStatus::CantAnalyze);
    }
    remove_db(path);
}

TEST_CASE("an open that cannot add the failure columns fails as DatabaseUpgrade and changes "
          "nothing") {
    // A records table that has the second failure column but not the first:
    // the open adds failure_kind, then fails adding failure_text again, so
    // the column it did add must be undone with the rest of the transaction.
    const std::string path = testtemp::temp_path("half_failure_columns", ".db");
    remove_db(path);
    { RecordStore seed(path); }
    exec_on_file(path, "ALTER TABLE records DROP COLUMN failure_kind;");
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksFailureColumns);
    try {
        RecordStore store(path);
        FAIL("the open added a column the table already has");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseUpgrade);
    }
    CHECK(classify_library_file(path) == LibraryFileKind::LacksFailureColumns);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM pragma_table_info('records')"
                               " WHERE name = 'failure_kind'") == 0);
    remove_db(path);
}

// ---- a file from before the album gains the column (ALB-1) ------------------

namespace {

// Whether the charts table in the file at `path` has an album column.
bool charts_have_album(const std::string& path) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM pragma_table_info('charts')"
                                " WHERE name = 'album'") == 1;
}

// A file at `path` with one library row and one record, the scanned chart's
// (scanned_chart), written by this build and closed, then given back the
// charts table and a scan stamp this build does not accept, as a build from
// before the album wrote them. Returns the chart's song id.
std::string seed_without_album(const std::string& path, const Profile& profile) {
    remove_db(path);
    {
        RecordStore seed(path);
        seed.rebuild_chart_library({scanned_chart().row});
        seed.save(scanned_record(profile));
    }
    const int older_stamp = kChartMetaStamp.written - 1;
    REQUIRE_FALSE(kChartMetaStamp.is_current(older_stamp));
    exec_on_file(path, ("ALTER TABLE charts DROP COLUMN album;"
                        "UPDATE meta SET value = '" +
                        std::to_string(older_stamp) + "' WHERE key = 'chart_meta_version';")
                           .c_str());
    return scanned_chart().row.md5;
}

}  // namespace

TEST_CASE("a file whose charts table has no album column opens and gains it in place") {
    const std::string path = testtemp::temp_path("lacks_album_column", ".db");
    const Profile profile = at4();
    const std::string id = seed_without_album(path, profile);
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksAlbumColumn);
    // The row a scan of the chart writes again, album and all.
    const ChartLibraryEntry& rescanned = scanned_chart().row;
    REQUIRE_FALSE(rescanned.album.empty());
    {
        RecordStore store(path);
        // The old rows read as they did, with an empty album, and the old
        // stamp hands back no cache, so the next scan reads every chart.
        CHECK(store.chart_library_count() == 1);
        CHECK(store.chart_index().naming(id)->album.empty());
        CHECK(store.chart_library_cache().empty());
        CHECK(store.lookup_one(id, profile).status == RecordStatus::Ready);
        store.rebuild_chart_library({rescanned});
        CHECK(store.chart_index().naming(id)->album == rescanned.album);
    }
    CHECK(charts_have_album(path));
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    // A second open finds a Current file and reads the album the scan wrote.
    {
        RecordStore reopened(path);
        CHECK(reopened.chart_index().naming(id)->album == rescanned.album);
        CHECK(reopened.chart_library_cache().count(rescanned.notespath) == 1);
    }
    remove_db(path);
}

TEST_CASE("a 2.8.0 file gains the failure columns and the album column in one open") {
    const std::string path = testtemp::temp_path("lacks_failure_and_album", ".db");
    const Profile profile = at4();
    const std::string id = seed_without_album(path, profile);
    exec_on_file(path, "ALTER TABLE records DROP COLUMN failure_text;"
                       "ALTER TABLE records DROP COLUMN failure_kind;");
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksFailureColumns);
    {
        RecordStore store(path);
        CHECK(store.lookup_one(id, profile).status == RecordStatus::Ready);
        CHECK(store.chart_library_count() == 1);
    }
    CHECK(charts_have_album(path));
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM pragma_table_info('records')"
                               " WHERE name IN ('failure_kind', 'failure_text')") == 2);
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    remove_db(path);
}

TEST_CASE("an open that cannot add the album column fails as DatabaseUpgrade and changes "
          "nothing") {
    const std::string path = testtemp::temp_path("album_column_locked", ".db");
    const Profile profile{"mode", CapQuery::at(4), Lens{}};
    seed_without_album(path, profile);
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksAlbumColumn);
    // Another connection holds the file's write lock, so the add cannot
    // write; reading the file still works, as WAL allows.
    sqlite3* holder = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &holder) == SQLITE_OK);
    REQUIRE(sqlite3_exec(holder, "BEGIN IMMEDIATE; DELETE FROM meta WHERE key = 'none';", nullptr,
                         nullptr, nullptr) == SQLITE_OK);
    try {
        RecordStore store(path);
        FAIL("the open added a column while another connection held the write lock");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseUpgrade);
    }
    sqlite3_exec(holder, "ROLLBACK", nullptr, nullptr, nullptr);
    sqlite3_close(holder);
    CHECK_FALSE(charts_have_album(path));
    CHECK(classify_library_file(path) == LibraryFileKind::LacksAlbumColumn);
    remove_db(path);
}

// ---- converting a 2.2.0 to 2.7.0 file (D115 task A1-6a) --------------------

namespace {

// The profiles the 2.2.0 seed writes under, both at an 8-bar cap, and its
// charts. Every chart is under kMode8; "ready" is under kOtherMode8 as well.
const Profile kMode8{"mode", CapQuery::at(8), Lens{}};
const Profile kOtherMode8{"other mode", CapQuery::at(8), Lens{}};
constexpr const char* kReady = "ready";
constexpr const char* kNoPaths = "empty";
constexpr const char* kBuild = "build";
constexpr const char* kRules = "rules";
constexpr const char* kBoth = "both";
constexpr const char* kNoStars = "nostars";

// What the seed file holds, read from it before any open. The conversion
// keeps every result but the one the seed left a score and no stars
// (kNoStars); each kept result becomes one record, filed under one profile
// per distinct mode, cap, lens, stamp and rules among them, with a rank-1
// path row when it has a score.
struct V220Seed {
    int64_t charts = 0;    // library rows
    int64_t results = 0;   // results rows
    int64_t kept = 0;      // results the conversion keeps
    int64_t profiles = 0;  // profiles the store filed the kept records under
    int64_t scored = 0;    // kept results with a score
};

// The fixture chart analyzed at the seed's 8-bar cap: the record every seeded
// result is made from.
const HydraRecord& analysis_at8() {
    return ::corpus::analyzed(test::fixture_chart(), test::store_fixture_settings(8));
}

// The fill-rule label 2.2.0 to 2.7.0 kept for a whole file, as the seed
// writes it. The conversion deletes it (D115 task A1-6c).
constexpr const char* kSeedLabelSql = "INSERT INTO meta (key, value) VALUES ('engine_mode', 'ch10')";
constexpr const char* kSeedLabelCountSql = "SELECT COUNT(*) FROM meta WHERE key = 'engine_mode'";

// A database at `path` in the layout 2.2.0 to 2.7.0 wrote, with one row of
// every kind the conversion treats differently, its files closed. Its
// library rows are the rows a scan writes for the first corpus charts, filed
// under the seed's ids; its records are the fixture chart's 8-bar analysis.
// Returns what the file holds (V220Seed).
V220Seed seed_v220_layout(const std::string& path) {
    remove_db(path);
    const std::vector<std::string> ids{kReady, kRules, kBoth, kBuild, kNoPaths, kNoStars};
    const HydraRecord& analysis = analysis_at8();
    // The same analysis with every path taken out: analyzed, and nothing
    // survived.
    HydraRecord no_paths = analysis;
    no_paths.paths.clear();
    no_paths.allzero_paths.clear();
    {
        RecordStore seed(path);
        const std::vector<std::string>& charts = ::corpus::chart_paths();
        REQUIRE(charts.size() >= ids.size());
        std::vector<ChartLibraryEntry> rows;
        for (size_t i = 0; i < ids.size(); ++i) {
            ChartLibraryEntry row = ::corpus::scanned_entry(charts[i]);
            row.md5 = ids[i];
            rows.push_back(row);
        }
        seed.rebuild_chart_library(rows);
        seed.save(prepare_record(kReady, kMode8, analysis));
        seed.save(prepare_record(kReady, kOtherMode8, analysis));
        seed.save(prepare_record(kNoPaths, kMode8, no_paths));
        test::add_stale_records(seed, analysis, kMode8, kBuild, kRules, kBoth);
        seed.save(prepare_record(kNoStars, kMode8, analysis));
    }
    // What the store this build wrote holds before it is turned back into the
    // old layout: one profile per distinct settings, stamp and rules, as the
    // store itself filed them, and the rank-1 path rows the old results kept.
    // The no-stars row shares the "ready" profile, so it adds none.
    const int64_t profiles_filed = scalar_on_file(path, "SELECT COUNT(*) FROM profiles");
    const int64_t scored_kept = scalar_on_file(
        path, "SELECT COUNT(*) FROM record_paths WHERE rank = 1 AND song_id <> 'nostars'");
    exec_on_file(path, test::v220_layout_sql().c_str());
    // 2.2.0 to 2.7.0 wrote the charts table without the album.
    exec_on_file(path, "ALTER TABLE charts DROP COLUMN album");
    exec_on_file(path, kSeedLabelSql);
    // A row from before the stars column: a score and no stars. Its stars
    // needed the stored paths to fill, so the conversion leaves it behind.
    // The no-paths row has neither, by design, and stays.
    exec_on_file(path, "UPDATE results SET stars = NULL WHERE hyhash = 'nostars'");
    // An id an older build stored in capitals, in both tables: the conversion
    // lowercases them (ADR 0031), so the chart still reads under "ready".
    exec_on_file(path, "UPDATE results SET hyhash = 'READY' WHERE hyhash = 'ready';"
                       "UPDATE charts SET md5 = 'READY' WHERE md5 = 'ready';");
    // The seed's own facts: every library row it wrote is there, the
    // fill-rule label is in meta, the no-stars row kept its score, and the
    // no-paths row has neither score nor stars.
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM charts") ==
            static_cast<int64_t>(ids.size()));
    REQUIRE(scalar_on_file(path, kSeedLabelCountSql) == 1);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE hyhash = 'nostars'"
                                 " AND stars IS NULL AND score IS NOT NULL") == 1);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE hyhash = 'empty'"
                                 " AND stars IS NULL AND score IS NULL") == 1);
    REQUIRE(classify_library_file(path) == LibraryFileKind::Convertible);

    V220Seed out;
    out.charts = scalar_on_file(path, "SELECT COUNT(*) FROM charts");
    out.results = scalar_on_file(path, "SELECT COUNT(*) FROM results");
    out.kept = scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE hyhash <> 'nostars'");
    out.profiles = profiles_filed;
    out.scored = scored_kept;
    return out;
}

// What every row reads after the conversion: Ready stays Ready with its
// numbers, Stale stays Stale for the same reason, the no-stars row is gone,
// and the library and its reader stamp stayed whole.
void check_converted_rows(RecordStore& store, const V220Seed& seed) {
    // The old results kept rank 1 only, so the converted record reads the
    // summary and best path the analysis's own record gives.
    const RecordRow analysis_record = record_of(kReady, analysis_at8());
    const RecordLookup got_ready = store.lookup_one(kReady, kMode8);
    CHECK(got_ready.status == RecordStatus::Ready);
    CHECK(got_ready.record.summary() == analysis_record.summary());
    CHECK(got_ready.record.best_path_text() == analysis_record.best_path_text());
    CHECK(store.lookup_one(kReady, kOtherMode8).status == RecordStatus::Ready);
    const RecordLookup got_empty = store.lookup_one(kNoPaths, kMode8);
    CHECK(got_empty.status == RecordStatus::Ready);
    CHECK_FALSE(got_empty.record.has_scored_best_path());
    const RecordLookup got_build = store.lookup_one(kBuild, kMode8);
    CHECK(got_build.status == RecordStatus::Stale);
    CHECK(got_build.stale_build);
    CHECK_FALSE(got_build.stale_rules);
    const RecordLookup got_rules = store.lookup_one(kRules, kMode8);
    CHECK(got_rules.status == RecordStatus::Stale);
    CHECK_FALSE(got_rules.stale_build);
    CHECK(got_rules.stale_rules);
    const RecordLookup got_both = store.lookup_one(kBoth, kMode8);
    CHECK(got_both.status == RecordStatus::Stale);
    CHECK(got_both.stale_build);
    CHECK(got_both.stale_rules);
    CHECK(store.lookup_one(kNoStars, kMode8).status == RecordStatus::NotAnalyzed);
    CHECK(store.record_count() == seed.kept);
    CHECK(store.chart_library_count() == seed.charts);
    // The library row stored in capitals reads lowercase, as its record does.
    CHECK(store.list_chart_library_copies("ready").size() == 1);
    // The scan cache reads only under a current reader stamp, so a full one
    // shows the stamp stayed in meta.
    CHECK(store.chart_library_cache().size() == static_cast<size_t>(seed.charts));
}

// The file is in this build's layout, with no results table left and
// nothing set aside beside it.
void check_converted_file(const std::string& path) {
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM sqlite_master WHERE name = 'results'") == 0);
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    CHECK(charts_have_album(path));
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
    const V220Seed seed = seed_v220_layout(path);

    ProgressLog log;
    {
        RecordStore store(path, core::default_stamp(), log.fn());
        CHECK_FALSE(store.set_aside_path().has_value());
        check_converted_rows(store, seed);
    }
    check_converted_file(path);
    // The results became records: one profile per distinct settings, stamp
    // and rules, one record per kept row, a rank-1 path row for each with a
    // score, and every id lowercase.
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == seed.profiles);
    CHECK(record_rows_in(path) == seed.kept);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM record_paths") == seed.scored);
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
        CHECK(p.rows_total == seed.kept);
        CHECK(p.rows_done >= last_converted);
        last_converted = p.rows_done;
    }
    CHECK(last_converted == seed.kept);

    // A second open finds no results and does nothing.
    ProgressLog again;
    {
        RecordStore store(path, core::default_stamp(), again.fn());
        CHECK(store.record_count() == seed.kept);
    }
    CHECK(again.steps() == std::vector<OpenStep>{OpenStep::Opening});
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == seed.profiles);
    remove_db(path);
}

TEST_CASE("a stop asked mid-conversion leaves the results table whole and throws Cancelled") {
    const std::string path = testtemp::temp_path("convert_cancelled", ".db");
    const V220Seed seed = seed_v220_layout(path);
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
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM results") == seed.results);
    CHECK(record_rows_in(path) == 0);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == 0);
    CHECK(classify_library_file(path) == LibraryFileKind::Convertible);
    {
        RecordStore store(path);
        check_converted_rows(store, seed);
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
        // The error carries Windows' own reason for the rename that failed:
        // the same rename, tried again here through the store's renamer,
        // fails for the same reason and leaves the file where it is.
        const std::optional<std::string> reason = rename_file(path, set_aside_path(path));
        REQUIRE(reason.has_value());
        CHECK(std::string(e.what()) == *reason);
        // The sentence the user reads quotes that reason's first sentence and
        // names the oldest release this build converts, read from its owner.
        // The sentence's other words are test_user_messages' to pin.
        const std::string said = app::plain_error(e);
        const std::string reason_words = reason->substr(0, reason->find_first_of(".\r\n"));
        INFO(said);
        REQUIRE_FALSE(reason_words.empty());
        CHECK(said.find(reason_words) != std::string::npos);
        CHECK(said.find(kOldestConvertibleVersion) != std::string::npos);
        // The sentence already shows the reason, so no details line repeats it.
        CHECK(app::plain_error_detail(e).empty());
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
    // open, the first on a new file and the second on the file it left.
    const std::string path = testtemp::temp_path("journal_size_limit", ".db");
    std::remove(path.c_str());
    for (int open = 0; open < 2; ++open) {
        RecordStore store(path);
        CHECK(store.journal_size_limit_for_test() == kJournalSizeLimitBytes);
    }
    std::remove(path.c_str());
}

// ---- real old databases (testdata/store, test fidelity plan task 1) --------

TEST_CASE("a database any release before 2.2.0 wrote is too old: refused, or set aside for a "
          "fresh library") {
    // The four pre-2.2.0 files in testdata/store; its README says which
    // release wrote each.
    const char* const files[] = {"v1.8.4-schema2.db", "v1.8.4-schema2-legacy-fills.db",
                                 "v2.0.0-schema3.db", "v2.1.0-schema4.db"};
    for (const char* file : files) {
        INFO(std::string(file));
        const std::string path = copy_of_old_database(file, "old_database");
        check_too_old_file(path);
        remove_db(path);
    }
}

// hydra_batch and hydra_fillcompare open their library through app::open_store.
TEST_CASE("the command-line tools' open refuses a file too old to convert and never renames it") {
    const std::string path = copy_of_old_database("v2.1.0-schema4.db", "cli_refuse");
    const std::string before = file_bytes(path);
    try {
        app::open_store(path);
        FAIL("a command-line tool opened a file too old to convert");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::OldLibraryRefused);
        // The error names the file it refused, and the sentence the tool
        // prints names that file and the oldest release this build converts,
        // read from its owner. The sentence's other words are
        // test_user_messages' to pin.
        CHECK(std::string(e.what()) == path);
        const std::string said = app::plain_error(e);
        INFO(said);
        CHECK(said.find(path) != std::string::npos);
        CHECK(said.find(kOldestConvertibleVersion) != std::string::npos);
    }
    CHECK(file_bytes(path) == before);
    CHECK_FALSE(file_exists_utf8(set_aside_path(path)));
    CHECK_FALSE(side_files_present(path));
    remove_db(path);
}

// ---- a real v2.12.1 library (testdata/store/v2.12.1_library.db) -----------

namespace {

// Every row of `table` in the file at `path`, each column's type and bytes
// joined into one string, so two reads compare whole rows. The rows are a
// set, so the comparison does not lean on the order SQLite reads them in.
std::multiset<std::string> table_rows(const std::string& path, const std::string& table) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    const std::string sql = "SELECT * FROM " + table;
    REQUIRE(sqlite3_prepare_v2(db, sql.c_str(), -1, &s, nullptr) == SQLITE_OK);
    std::multiset<std::string> out;
    while (sqlite3_step(s) == SQLITE_ROW) {
        std::string row;
        for (int c = 0; c < sqlite3_column_count(s); ++c) {
            const int type = sqlite3_column_type(s, c);
            row += std::to_string(type) + ":";
            if (type != SQLITE_NULL) {
                const void* bytes = sqlite3_column_blob(s, c);
                const int n = sqlite3_column_bytes(s, c);
                row += std::to_string(n) + ":";
                if (n > 0) row.append(static_cast<const char*>(bytes), static_cast<size_t>(n));
            }
            row += "|";
        }
        out.insert(row);
    }
    sqlite3_finalize(s);
    sqlite3_close(db);
    return out;
}

// The first column of every row `sql` reads from the file at `path`, as
// text, in the order the query gives.
std::vector<std::string> texts_on_file(const std::string& path, const char* sql) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK);
    std::vector<std::string> out;
    while (sqlite3_step(s) == SQLITE_ROW)
        out.push_back(reinterpret_cast<const char*>(sqlite3_column_text(s, 0)));
    sqlite3_finalize(s);
    sqlite3_close(db);
    return out;
}

// The song ids of every record in the file at `path`, in id order.
std::vector<std::string> record_song_ids(const std::string& path) {
    return texts_on_file(path, "SELECT song_id FROM records ORDER BY song_id");
}

// Every table in the file at `path`, by name, with its rows (table_rows):
// what a write that is undone must leave as it found it.
std::map<std::string, std::multiset<std::string>> every_table_rows(const std::string& path) {
    std::map<std::string, std::multiset<std::string>> out;
    for (const std::string& name :
         texts_on_file(path, "SELECT name FROM sqlite_master WHERE type = 'table'"))
        out[name] = table_rows(path, name);
    return out;
}

// The one text value `sql` reads from the file at `path`.
std::string text_on_file(const std::string& path, const std::string& sql) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, sql.c_str(), -1, &s, nullptr) == SQLITE_OK);
    std::string out;
    if (sqlite3_step(s) == SQLITE_ROW && sqlite3_column_text(s, 0))
        out = reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
    sqlite3_finalize(s);
    sqlite3_close(db);
    return out;
}

// The run settings of the one profile row the file at `path` holds, read
// from that row, so a lookup asks for exactly what the batch stored.
Profile file_profile(const std::string& path) {
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == 1);
    Profile p{text_on_file(path, "SELECT chartmode FROM profiles"),
              CapQuery::at(static_cast<int>(scalar_on_file(path, "SELECT sp_cap FROM profiles"))),
              Lens{}};
    p.lens.ms_enabled = static_cast<int>(scalar_on_file(path, "SELECT ms_enabled FROM profiles"));
    p.lens.ms_value = static_cast<int>(scalar_on_file(path, "SELECT ms_value FROM profiles"));
    p.lens.depth_mode = static_cast<int>(scalar_on_file(path, "SELECT depth_mode FROM profiles"));
    p.lens.depth_value =
        static_cast<int>(scalar_on_file(path, "SELECT depth_value FROM profiles"));
    p.lens.legacy_fills =
        static_cast<int>(scalar_on_file(path, "SELECT legacy_fills FROM profiles"));
    return p;
}

}  // namespace

// The file a v2.12.1 batch wrote. Its charts table predates the album column
// and its results stamp predates kResultsStamp, so this is the upgrade a
// v2.12.1 user's first open of this build makes.
TEST_CASE("a library file v2.12.1 wrote opens, gains the album column, and keeps its records") {
    const std::string path = copy_of_old_database("v2.12.1_library.db", "v2121_library");
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksAlbumColumn);
    REQUIRE_FALSE(charts_have_album(path));
    // What the file holds before anything opens it.
    const std::multiset<std::string> records_before = table_rows(path, "records");
    const std::multiset<std::string> paths_before = table_rows(path, "record_paths");
    const int64_t library_rows_before = scalar_on_file(path, "SELECT COUNT(*) FROM charts");
    const std::vector<std::string> ids = record_song_ids(path);
    REQUIRE_FALSE(ids.empty());
    const Profile batch_profile = file_profile(path);
    // The file's stamp against this build's: a stamp this build does not
    // accept reads Stale, one it accepts reads Ready.
    const std::string file_stamp = text_on_file(path, "SELECT hyversion FROM profiles");
    const bool stamp_current = kResultsStamp.is_current(std::string_view(file_stamp));
    {
        // The app's own open (ui::StoreOpenJob::run).
        RecordStore store(path, OldFilePolicy::SetAside, core::default_stamp());
        CHECK_FALSE(store.set_aside_path().has_value());
        CHECK(store.record_count() == static_cast<int64_t>(ids.size()));
        CHECK(store.chart_library_count() == library_rows_before);
        const std::vector<RecordLookup> got = store.lookup(ids, batch_profile);
        REQUIRE(got.size() == ids.size());
        for (size_t i = 0; i < got.size(); ++i) {
            INFO(ids[i]);
            CHECK(got[i].status == (stamp_current ? RecordStatus::Ready : RecordStatus::Stale));
            CHECK(got[i].stale_build == !stamp_current);
        }
        CHECK(store.settled_song_ids(batch_profile).empty() == !stamp_current);
    }
    CHECK(charts_have_album(path));
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    // The open rewrote no record and no path row.
    CHECK(table_rows(path, "records") == records_before);
    CHECK(table_rows(path, "record_paths") == paths_before);
    remove_db(path);
}

namespace {

// Every profile row in the file at `path`, every column the older layout
// held, id first, in id order: what the early-fill rebuild must keep.
std::string older_profile_columns(const std::string& path) {
    return text_on_file(path,
                        "SELECT group_concat(row, ';') FROM (SELECT id || '|' || chartmode || '|' ||"
                        " sp_cap || '|' || ms_enabled || '|' || ms_value || '|' || depth_mode ||"
                        " '|' || depth_value || '|' || legacy_fills || '|' || hyversion || '|' ||"
                        " hex(rules_fp) AS row FROM profiles ORDER BY id)");
}

// How many records in the file at `path` are filed under a profile row it
// holds.
int64_t records_under_profiles(const std::string& path) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM records r JOIN profiles p"
                                " ON p.id = r.profile_id");
}

// Whether the file at `path` still holds the table the early-fill rebuild
// renames the old profiles table to.
bool rebuild_table_left(const std::string& path) {
    return scalar_on_file(path, std::string("SELECT COUNT(*) FROM sqlite_master WHERE name = '") +
                                    kProfilesBeforeEarlyFillTable + "'") != 0;
}

// How many profile rows in the file at `path` carry the default early-fill
// window.
int64_t profiles_at_default_window(const std::string& path) {
    return scalar_on_file(path, std::string("SELECT COUNT(*) FROM profiles WHERE ") +
                                    kEarlyFillWindowColumn + " = " +
                                    std::to_string(Lens{}.efill_window_ms));
}

// The profiles table a build before results were saved per early-fill window
// wrote: the same columns and key without the window. The file at `path`
// keeps every row it held, id included. With `keep_key` false the table
// has no unique key, so it can hold two rows a real file never could.
void drop_early_fill_column(const std::string& path, bool keep_key = true) {
    const std::string key = keep_key ? ", UNIQUE (chartmode, sp_cap, ms_enabled, ms_value,"
                                       " depth_mode, depth_value, legacy_fills, hyversion,"
                                       " rules_fp)"
                                     : "";
    const std::string sql =
        std::string("CREATE TABLE p5 (id INTEGER PRIMARY KEY, chartmode TEXT NOT NULL,"
                    " sp_cap INTEGER NOT NULL, ms_enabled INTEGER NOT NULL,"
                    " ms_value INTEGER NOT NULL, depth_mode INTEGER NOT NULL,"
                    " depth_value INTEGER NOT NULL, legacy_fills INTEGER NOT NULL,"
                    " hyversion TEXT NOT NULL, rules_fp BLOB NOT NULL") +
        key +
        ");"
        "INSERT INTO p5 SELECT id, chartmode, sp_cap, ms_enabled, ms_value, depth_mode,"
        " depth_value, legacy_fills, hyversion, rules_fp FROM profiles;"
        "DROP TABLE profiles;"
        "ALTER TABLE p5 RENAME TO profiles;";
    exec_on_file(path, sql.c_str());
}

// The search settings fixture() runs (store_fixtures.h), with the SP cap at
// `cap` and the early-fill window at `window_ms`: what at4() and its
// neighbours name as a profile.
app::AnalysisSettings fixture_search(int cap, int window_ms) {
    app::AnalysisSettings settings;
    settings.sp_cap = cap;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 0;
    settings.ms_filter = std::nullopt;
    settings.early_fill_window_ms = window_ms;
    return settings;
}

// The real analysis of fixture()'s chart (the first corpus chart with
// paths) at `cap` bars and the early-fill window `window_ms`.
const HydraRecord& fixture_chart_at(int cap, int window_ms) {
    static const std::string chart =
        corpus::first_chart_with_paths(fixture_search(4, static_cast<int>(kDefaultEarlyFillWindowMs)))
            .chart;
    return corpus::analyzed(chart, fixture_search(cap, window_ms));
}

// A file at `path` this build wrote, with two real records under two
// profiles (4 and 8 bars, both at the default window), its profiles table
// then put back to the layout before the early-fill window.
void seed_without_early_fill(const std::string& path) {
    remove_db(path);
    const int window = static_cast<int>(kDefaultEarlyFillWindowMs);
    {
        RecordStore seed(path);
        seed.save(prepare_record("a", at4(), fixture_chart_at(4, window)));
        seed.save(prepare_record("b", Profile{"mode", CapQuery::at(8), Lens{}},
                                 fixture_chart_at(8, window)));
    }
    drop_early_fill_column(path);
}

}  // namespace

TEST_CASE("the v2.12.1 file's profiles table is rebuilt with the early-fill window, keeping "
          "every row") {
    const std::string path = copy_of_old_database("v2.12.1_library.db", "v2121_efill");
    REQUIRE_FALSE(profiles_have_early_fill(path));
    const std::string before = older_profile_columns(path);
    REQUIRE_FALSE(before.empty());
    const int64_t records_before = record_rows_in(path);
    REQUIRE(records_before > 0);
    REQUIRE(records_under_profiles(path) == records_before);
    const int64_t profiles_before = scalar_on_file(path, "SELECT COUNT(*) FROM profiles");
    const Profile batch_profile = file_profile(path);
    const std::string song = record_song_ids(path).front();
    const bool stamp_current = kResultsStamp.is_current(
        std::string_view(text_on_file(path, "SELECT hyversion FROM profiles")));
    { RecordStore store(path, OldFilePolicy::SetAside, core::default_stamp()); }
    REQUIRE(profiles_have_early_fill(path));
    // Every row keeps its id and its other columns, and is filed under the
    // default window, since no older file recorded one.
    CHECK(older_profile_columns(path) == before);
    CHECK(profiles_at_default_window(path) == profiles_before);
    CHECK(records_under_profiles(path) == records_before);
    // At the default window no old result vanishes: it reads as the stamp
    // says, not Not analyzed. One chart is enough to show the lookup reaches
    // the rebuilt row; the earlier case reads them all.
    {
        RecordStore store(path);
        CHECK(store.lookup_one(song, batch_profile).status ==
              (stamp_current ? RecordStatus::Ready : RecordStatus::Stale));
        // At another window the chart has no result yet.
        Profile at_85 = batch_profile;
        at_85.lens.efill_window_ms = kMaxEarlyFillWindowMs;
        CHECK(store.lookup_one(song, at_85).status == RecordStatus::NotAnalyzed);

        // Two profiles that differ only in the window can now both be saved:
        // one chart's real analyses at the default window and at the top one.
        Profile mode_at_85 = at4();
        mode_at_85.lens.efill_window_ms = kMaxEarlyFillWindowMs;
        store.save(prepare_record(
            "h", at4(), fixture_chart_at(4, static_cast<int>(kDefaultEarlyFillWindowMs))));
        store.save(prepare_record("h", mode_at_85, fixture_chart_at(4, kMaxEarlyFillWindowMs)));
        CHECK(store.lookup_one("h", at4()).status == RecordStatus::Ready);
        CHECK(store.lookup_one("h", mode_at_85).status == RecordStatus::Ready);
    }
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles WHERE chartmode = 'mode'") == 2);
    remove_db(path);
}

TEST_CASE("a file whose profiles table lacks the early-fill window opens and gains it in "
          "place") {
    const std::string path = testtemp::temp_path("lacks_early_fill_column", ".db");
    seed_without_early_fill(path);
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksEarlyFillColumn);
    const std::string before = older_profile_columns(path);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == 2);
    const int64_t paths_before = scalar_on_file(path, "SELECT COUNT(*) FROM record_paths");
    {
        RecordStore store(path);
        // Both records read as they did, each under its own profile.
        CHECK(store.lookup_one("a", at4()).status == RecordStatus::Ready);
        CHECK(store.lookup_one("b", Profile{"mode", CapQuery::at(8), Lens{}}).status ==
              RecordStatus::Ready);
    }
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    CHECK(profiles_have_early_fill(path));
    CHECK(older_profile_columns(path) == before);
    CHECK(profiles_at_default_window(path) == 2);
    CHECK(records_under_profiles(path) == 2);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM record_paths") == paths_before);
    CHECK_FALSE(rebuild_table_left(path));
    remove_db(path);
}

namespace {

// How many profile rows in the file at `path` carry the default song speed.
int64_t profiles_at_default_speed(const std::string& path) {
    return scalar_on_file(path, std::string("SELECT COUNT(*) FROM profiles WHERE ") +
                                    kSpeedColumn + " = " + std::to_string(Lens{}.speed_percent));
}

// The profiles table a build before results were saved per song speed wrote:
// the same columns and key, the early-fill window included, without the
// speed. The file at `path` keeps every row it held, id included.
void drop_speed_column(const std::string& path) {
    exec_on_file(path,
                 "CREATE TABLE p6 (id INTEGER PRIMARY KEY, chartmode TEXT NOT NULL,"
                 " sp_cap INTEGER NOT NULL, ms_enabled INTEGER NOT NULL,"
                 " ms_value INTEGER NOT NULL, depth_mode INTEGER NOT NULL,"
                 " depth_value INTEGER NOT NULL, legacy_fills INTEGER NOT NULL,"
                 " efill_window_ms INTEGER NOT NULL,"
                 " hyversion TEXT NOT NULL, rules_fp BLOB NOT NULL,"
                 " UNIQUE (chartmode, sp_cap, ms_enabled, ms_value, depth_mode, depth_value,"
                 " legacy_fills, efill_window_ms, hyversion, rules_fp));"
                 "INSERT INTO p6 SELECT id, chartmode, sp_cap, ms_enabled, ms_value, depth_mode,"
                 " depth_value, legacy_fills, efill_window_ms, hyversion, rules_fp FROM profiles;"
                 "DROP TABLE profiles;"
                 "ALTER TABLE p6 RENAME TO profiles;");
}

}  // namespace

TEST_CASE("a file whose profiles table lacks the song speed opens, gains it, and reads every old "
          "row at 100") {
    const std::string path = testtemp::temp_path("lacks_speed_column", ".db");
    remove_db(path);
    const int window = static_cast<int>(kDefaultEarlyFillWindowMs);
    {
        RecordStore seed(path);
        seed.save(prepare_record("a", at4(), fixture_chart_at(4, window)));
        seed.save(prepare_record("b", Profile{"mode", CapQuery::at(8), Lens{}},
                                 fixture_chart_at(8, window)));
    }
    drop_speed_column(path);
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksSpeedColumn);
    const std::string before = older_profile_columns(path);
    const int64_t paths_before = scalar_on_file(path, "SELECT COUNT(*) FROM record_paths");
    {
        RecordStore store(path);
        // Both records read as they did, each under its own profile at 100.
        CHECK(store.lookup_one("a", at4()).status == RecordStatus::Ready);
        CHECK(store.lookup_one("b", Profile{"mode", CapQuery::at(8), Lens{}}).status ==
              RecordStatus::Ready);
        // At another speed the chart has no result.
        Profile at150 = at4();
        at150.lens.speed_percent = 150;
        CHECK(store.lookup_one("a", at150).status == RecordStatus::NotAnalyzed);
    }
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    CHECK(older_profile_columns(path) == before);
    CHECK(profiles_at_default_speed(path) == 2);
    CHECK(profiles_at_default_window(path) == 2);
    CHECK(records_under_profiles(path) == 2);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM record_paths") == paths_before);
    CHECK(scalar_on_file(path, std::string("SELECT COUNT(*) FROM sqlite_master WHERE name = '") +
                                   kProfilesBeforeSpeedTable + "'") == 0);
    remove_db(path);
}

TEST_CASE("a file older than both the early-fill window and the song speed reads every old row "
          "at 100") {
    const std::string path = testtemp::temp_path("lacks_both_columns", ".db");
    seed_without_early_fill(path);
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksEarlyFillColumn);
    {
        RecordStore store(path);
        CHECK(store.lookup_one("a", at4()).status == RecordStatus::Ready);
        Profile at150 = at4();
        at150.lens.speed_percent = 150;
        CHECK(store.lookup_one("a", at150).status == RecordStatus::NotAnalyzed);
    }
    CHECK(classify_library_file(path) == LibraryFileKind::Current);
    CHECK(profiles_at_default_speed(path) == 2);
    CHECK(records_under_profiles(path) == 2);
    remove_db(path);
}

TEST_CASE("an open that cannot rebuild the profiles table fails as DatabaseUpgrade and changes "
          "nothing") {
    // The old profiles table loses its unique key and gains a copy of one
    // row under a new id. The rebuild's first steps go through; copying the
    // rows into the new table, whose key the two copies share, fails. So the
    // file can only end as it was if the whole rebuild is undone.
    const std::string path = testtemp::temp_path("efill_rebuild_blocked", ".db");
    seed_without_early_fill(path);
    drop_early_fill_column(path, false);
    exec_on_file(path, "INSERT INTO profiles SELECT (SELECT MAX(id) + 1 FROM profiles), chartmode,"
                       " sp_cap, ms_enabled, ms_value, depth_mode, depth_value, legacy_fills,"
                       " hyversion, rules_fp FROM profiles WHERE id = (SELECT MIN(id) FROM"
                       " profiles)");
    REQUIRE(classify_library_file(path) == LibraryFileKind::LacksEarlyFillColumn);
    const std::string profiles_before = older_profile_columns(path);
    const std::string profiles_sql_before =
        text_on_file(path, "SELECT sql FROM sqlite_master WHERE name = 'profiles'");
    const int64_t records_before = record_rows_in(path);
    const int64_t paths_before = scalar_on_file(path, "SELECT COUNT(*) FROM record_paths");
    try {
        RecordStore store(path);
        FAIL("the open copied two rows that share the new table's key");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseUpgrade);
    }
    // The renamed old table is gone and no new table took its place: the old
    // table, every row and every record, as they were.
    CHECK_FALSE(rebuild_table_left(path));
    CHECK(classify_library_file(path) == LibraryFileKind::LacksEarlyFillColumn);
    CHECK_FALSE(profiles_have_early_fill(path));
    CHECK(text_on_file(path, "SELECT sql FROM sqlite_master WHERE name = 'profiles'") ==
          profiles_sql_before);
    CHECK(older_profile_columns(path) == profiles_before);
    CHECK(record_rows_in(path) == records_before);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM record_paths") == paths_before);
    CHECK(records_under_profiles(path) == records_before);
    remove_db(path);
}

// SQLite switches a feature on when its SQLITE_ENABLE_* macro is defined at
// all, whatever its value (sqlite.org/compile.html), so even
// "SQLITE_ENABLE_FTS5=0" compiles full-text search in. Hydra never uses it,
// so the build leaves that macro out.
TEST_CASE("the vendored SQLite is built without FTS5") {
    // Without FTS5, SQLite has no fts5 module: making a full-text table fails
    // where a plain table on the same connection does not.
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(":memory:", &db) == SQLITE_OK);
    CHECK(sqlite3_exec(db, "CREATE TABLE plain (x)", nullptr, nullptr, nullptr) == SQLITE_OK);
    CHECK(sqlite3_exec(db, "CREATE VIRTUAL TABLE full_text USING fts5(x)", nullptr, nullptr,
                       nullptr) == SQLITE_ERROR);
    sqlite3_close(db);
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
    // A read of the library in name order (LibraryOrder::ByName: by name,
    // then scan order, which is rowid) walks an index, whatever it is called,
    // instead of sorting the whole table first.
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    REQUIRE(sqlite3_prepare_v2(db, "EXPLAIN QUERY PLAN SELECT * FROM charts ORDER BY name, rowid",
                               -1, &s, nullptr) == SQLITE_OK);
    std::string plan;
    while (sqlite3_step(s) == SQLITE_ROW)
        plan += std::string(reinterpret_cast<const char*>(sqlite3_column_text(s, 3))) + "\n";
    sqlite3_finalize(s);
    sqlite3_close(db);
    INFO(plan);
    CHECK(plan.find("USING INDEX") != std::string::npos);
    CHECK(plan.find("TEMP B-TREE") == std::string::npos);
    std::remove(path.c_str());
}

TEST_CASE("a save that fails leaves nothing behind and reads as a save error") {
    // A trigger that refuses one chart's result makes the save fail.
    const std::string path = testtemp::temp_path("save_fail", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    exec_on_file(path,
                 "CREATE TRIGGER refuse_boom BEFORE INSERT ON records WHEN NEW.song_id = 'boom'"
                 " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    {
        RecordStore store(path);
        const Profile profile{"mode", CapQuery::at(4), Lens{}};
        const auto tables_before = every_table_rows(path);
        try {
            store.save(prepare_record("boom", profile, at_cap(4)));
            FAIL("the refused save went through");
        } catch (const KindedError& e) {
            // A save error, read as the sentence its kind owns.
            CHECK(e.kind() == ErrorKind::DatabaseWrite);
            CHECK(hydra::app::plain_error(e) ==
                  hydra::app::plain_error(KindedError(ErrorKind::DatabaseWrite, "")));
        }
        // Not one row of the chart, its paths or its profile is left in any
        // table.
        CHECK(every_table_rows(path) == tables_before);
        // The store is still usable: no transaction was left open.
        store.save(prepare_record("ok", profile, at_cap(4)));
        CHECK(store.lookup_one("ok", profile).status == RecordStatus::Ready);
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
        // The error carries SQLite's own reason for a file that is not a
        // database, read from SQLite, and the user reads the couldn't-open
        // sentence from its owner.
        const std::string what = e.what();
        INFO(what);
        CHECK(what.find(sqlite3_errstr(SQLITE_NOTADB)) != std::string::npos);
        CHECK(app::plain_error(e) == app::kDatabaseOpenSentence);
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
        // The error carries SQLite's own reason for a busy file, read from
        // SQLite, so the message names the lock and not some other failure.
        INFO(e.what());
        CHECK(std::string(e.what()).find(sqlite3_errstr(SQLITE_BUSY)) != std::string::npos);
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

// D73: settled_song_ids is the batch's list of analyzed charts. Read on a
// failing database, an empty answer would make a batch treat every chart as
// not analyzed.
TEST_CASE("a read on a failing database throws instead of answering empty") {
    const std::string path = testtemp::temp_path("read_fail", ".db");
    std::remove(path.c_str());
    const Profile profile{"mode", CapQuery::at(4), Lens{}};
    {
        RecordStore store(path);
        store.save(prepare_record("h", profile, at_cap(4)));
        REQUIRE(store.settled_song_ids(profile) ==
                std::unordered_map<std::string, RecordStatus>{{"h", RecordStatus::Ready}});
        exec_on_file(path, "DROP TABLE records;");
        // The first read fails when it steps; by then the connection has
        // reloaded the schema, so the second fails when it compiles.
        for (const char* when : {"step", "compile"}) {
            INFO(std::string(when));
            check_read_fails([&] { store.settled_song_ids(profile); });
        }
    }
    std::remove(path.c_str());
}

// The list is the public reads RecordStore declares in store/record_store.h;
// a read added there is not covered until it is added here, so the name says
// the list is by name.
TEST_CASE("each store read named in the test throws a database read error when its step fails") {
    const std::string path = testtemp::temp_path("every_read_fail", ".db");
    std::remove(path.c_str());
    const Profile profile{"mode", CapQuery::at(4), Lens{}};
    const std::pair<const char*, std::function<void(RecordStore&)>> reads[] = {
        {"lookup", [&](RecordStore& s) { s.lookup({"h"}, profile); }},
        {"lookup_one", [&](RecordStore& s) { s.lookup_one("h", profile); }},
        {"settled_song_ids", [&](RecordStore& s) { s.settled_song_ids(profile); }},
        {"rows_missing_hardest", [&](RecordStore& s) { s.rows_missing_hardest(profile); }},
        {"hardest_figure_filled", [&](RecordStore& s) { s.hardest_figure_filled(profile); }},
        {"record_count", [](RecordStore& s) { s.record_count(); }},
        {"chart_index", [](RecordStore& s) { s.chart_index(); }},
        {"chart_library_cache", [](RecordStore& s) { s.chart_library_cache(); }},
        {"chart_library_count", [](RecordStore& s) { s.chart_library_count(); }},
        {"chart_library_out_of_date", [](RecordStore& s) { s.chart_library_out_of_date(); }},
        {"list_chart_library", [](RecordStore& s) { s.list_chart_library(0, 10); }},
        {"list_chart_library_copies",
         [](RecordStore& s) { s.list_chart_library_copies("h"); }},
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

// CMakeLists.txt builds the vendored SQLite without its memory-use counters
// and without shared cache, two options that trim its own CPU. A sibling of
// the FTS5 case above.
TEST_CASE("the vendored SQLite is built without memory counters or shared cache") {
    const std::string path = testtemp::temp_path("sqlite_build", ".db");
    std::remove(path.c_str());
    // Both connections ask for a shared cache; a build without it gives each
    // its own.
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_SHAREDCACHE;
    sqlite3* writer = nullptr;
    REQUIRE(sqlite3_open_v2(path.c_str(), &writer, flags, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_exec(writer, "CREATE TABLE t (x); BEGIN; INSERT INTO t VALUES (1);", nullptr,
                         nullptr, nullptr) == SQLITE_OK);
    // No memory counters: SQLite counted none of the allocations above.
    CHECK(sqlite3_memory_used() == 0);
    // No shared cache: a reader allowed to read uncommitted rows would see
    // the writer's open insert through a shared cache. With its own cache it
    // reads the file, which does not hold the insert yet.
    sqlite3* reader = nullptr;
    REQUIRE(sqlite3_open_v2(path.c_str(), &reader, flags, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_exec(reader, "PRAGMA read_uncommitted = 1", nullptr, nullptr, nullptr) ==
            SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(reader, "SELECT COUNT(*) FROM t", -1, &s, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(s) == SQLITE_ROW);
    CHECK(sqlite3_column_int64(s, 0) == 0);
    sqlite3_finalize(s);
    sqlite3_close(reader);
    sqlite3_exec(writer, "ROLLBACK", nullptr, nullptr, nullptr);
    sqlite3_close(writer);
    std::remove(path.c_str());
}

// ---- the batch writer (D86 item 3, task W1) --------------------------------

TEST_CASE("the batch guard sets the checkpoint threshold and truncates the log at the end") {
    // The threshold is per connection, so only the store's own getter can
    // read the store's. D86 item 3: the batch's end puts back SQLite's own
    // threshold, which a second plain connection on the same file reads.
    const std::string path = testtemp::temp_path("batch_guard", ".db");
    std::remove(path.c_str());
    const std::filesystem::path wal = std::filesystem::u8path(path + "-wal");
    {
        RecordStore store(path);
        const int64_t sqlite_default = scalar_on_file(path, "PRAGMA wal_autocheckpoint");
        REQUIRE(sqlite_default != kBatchWalAutocheckpointPages);
        CHECK(store.wal_autocheckpoint_for_test() == sqlite_default);
        {
            const RecordStore::BatchWrites batch(store);
            CHECK(store.wal_autocheckpoint_for_test() == kBatchWalAutocheckpointPages);
            store.save(prepare_record("h", Profile{"mode", CapQuery::at(4), Lens{}}, at_cap(4)));
            CHECK(std::filesystem::file_size(wal) > 0);
        }
        CHECK(std::filesystem::file_size(wal) == 0);
        CHECK(store.wal_autocheckpoint_for_test() == sqlite_default);
        CHECK(store.record_count() == 1);
    }
    std::remove(path.c_str());
}

// ---- the saved hardest figure (rows_missing_hardest, set_hardest) ----------

namespace {

// A path row saved by hand at `rank`, with the figure and squeeze-in count
// given. The other columns are any values a record may hold.
PathRow hand_path(int rank, std::optional<double> hardest_ms, int sqin_count) {
    PathRow p;
    p.rank = rank;
    p.path = "1 0+ 0 0";
    p.score = 100000 - rank;
    p.actcount = 2;
    p.hardest_ms = hardest_ms;
    p.sqin_count = sqin_count;
    p.sqout_count = 1;
    p.optimal = rank == 1;
    return p;
}

// A record for chart `id` under `profile` that holds `paths`, ready to save.
PreparedRecord hand_record(const std::string& id, const Profile& profile,
                           std::vector<PathRow> paths) {
    PreparedRecord prepared =
        prepare_record(id, profile, hydra::test::empty_analysis_under(profile));
    prepared.record.pathcount = static_cast<int>(paths.size());
    prepared.record.paths = std::move(paths);
    return prepared;
}

// The settings the hardest-figure cases save under: the app's defaults.
const app::Settings& hardest_settings() {
    static const app::Settings s;
    return s;
}

// A corpus chart with two saved paths or more that hold a squeeze-in, under
// hardest_settings(): the library row a scan writes for it, its analysis,
// and the record a fresh save of that analysis writes.
struct SqueezedChart {
    ChartLibraryEntry entry;
    const HydraRecord* analysis = nullptr;
    PreparedRecord fresh;
};

// The ranks of `prepared`'s paths that hold a squeeze-in, in rank order.
std::vector<int> squeeze_in_ranks(const PreparedRecord& prepared) {
    std::vector<int> out;
    for (const PathRow& p : prepared.record.paths)
        if (p.sqin_count > 0) out.push_back(p.rank);
    return out;
}

// The ranks of `prepared`'s paths that hold a hardest figure, in rank order:
// the figure the engine worked out when the record was built
// (engine_hardest_ms), which a squeeze-out or a required early fill gives as
// a squeeze-in does.
std::vector<int> figure_ranks(const PreparedRecord& prepared) {
    std::vector<int> out;
    for (const PathRow& p : prepared.record.paths)
        if (p.hardest_ms) out.push_back(p.rank);
    return out;
}

// The first corpus chart, walking every one, whose fresh record holds two
// squeeze-in paths or more. A chart the engine refuses (no notes at the
// difficulty, or a file it cannot read) is passed over; any other exception
// fails the case. test_saved_figure_fill.cpp's "a row that disagrees with its
// replay keeps its empty figure" needs Smash Mouth to hold two under the
// same settings, so the walk finds one.
const SqueezedChart& squeezed_chart() {
    static const SqueezedChart found = [] {
        const app::AnalysisSettings settings = hardest_settings().to_analysis_settings();
        for (const std::string& file : ::corpus::chart_paths()) {
            SqueezedChart c;
            try {
                c.analysis = &::corpus::analyzed(file, settings);
            } catch (const ChartFileError&) {
                continue;
            }
            c.entry = ::corpus::scanned_entry(file);
            c.fresh = prepare_record(c.entry.md5, hardest_settings().profile(), *c.analysis);
            if (squeeze_in_ranks(c.fresh).size() >= 2) return c;
        }
        throw std::runtime_error("no corpus chart saves two paths with a squeeze-in");
    }();
    return found;
}

// `prepared` with every saved figure emptied, as an older build left it.
PreparedRecord with_figures_emptied(PreparedRecord prepared) {
    for (PathRow& p : prepared.record.paths) p.hardest_ms.reset();
    return prepared;
}

// Every rank of `prepared`'s paths but `held`, in rank order.
std::vector<int> ranks_but(const PreparedRecord& prepared, int held) {
    std::vector<int> out;
    for (const PathRow& p : prepared.record.paths)
        if (p.rank != held) out.push_back(p.rank);
    return out;
}

// Lists `chart` as the library and saves its analysis under `profile` as an
// older build left it, with the figure of every rank in `emptied` empty
// (hydra::test::save_as_old_build).
void save_squeezed(RecordStore& store, const SqueezedChart& chart, const Profile& profile,
                   const std::vector<int>& emptied) {
    hydra::test::save_as_old_build(store, chart.entry.notespath, chart.entry, hardest_settings(),
                                   emptied, [&profile](PreparedRecord& p) { p.profile = profile; });
}

// `profile` under another chart mode name, so the store files it under a
// profile row of its own.
Profile in_other_mode(const Profile& profile, const std::string& name) {
    Profile out = profile;
    out.chartmode = profile.chartmode + " (" + name + ")";
    return out;
}

}  // namespace

TEST_CASE("rows missing the hardest figure: a Ready row with an empty figure the engine "
          "gives its path is a candidate") {
    const SqueezedChart& chart = squeezed_chart();
    const Profile profile = hardest_settings().profile();
    const std::vector<int> figured = figure_ranks(chart.fresh);
    REQUIRE(figured.size() >= 2);
    // Every figure is emptied but the last figured rank's, which keeps the
    // figure a fresh save holds. The candidates are the other figured ranks;
    // an emptied rank whose path has no figure is not one.
    const int held = figured.back();
    const std::vector<int> candidates(figured.begin(), figured.end() - 1);
    RecordStore store(":memory:");
    save_squeezed(store, chart, profile, ranks_but(chart.fresh, held));
    const RecordLookup found = store.lookup_one(chart.entry.md5, profile);
    REQUIRE(found.status == RecordStatus::Ready);

    const std::vector<MissingHardestRow> rows = store.rows_missing_hardest(profile);
    std::vector<int> ranks;
    for (const MissingHardestRow& row : rows) ranks.push_back(row.rank);
    CHECK(ranks == candidates);
    // Each row is the saved path the fill replays and checks: its pathstring,
    // score and squeeze counts as the record reads them back, and the chart
    // file and signature of the library row that lists it.
    for (const MissingHardestRow& row : rows) {
        CAPTURE(row.rank);
        REQUIRE(row.rank >= 1);
        REQUIRE(static_cast<size_t>(row.rank) <= found.record.paths.size());
        const PathRow& saved = found.record.paths[static_cast<size_t>(row.rank - 1)];
        CHECK(row.song_id == chart.entry.md5);
        CHECK(row.path == saved.path);
        CHECK(row.score == saved.score);
        CHECK(row.sqin_count == saved.sqin_count);
        CHECK(row.sqout_count == saved.sqout_count);
        CHECK(row.notespath == chart.entry.notespath);
        CHECK(row.sig == chart.entry.sig);
    }

    // The same emptied record is no candidate as another build's, ...
    {
        RecordStore old_build(":memory:");
        old_build.rebuild_chart_library({chart.entry});
        hydra::test::save_old_build_record(old_build, with_figures_emptied(chart.fresh));
        REQUIRE(old_build.lookup_one(chart.entry.md5, profile).status == RecordStatus::Stale);
        CHECK(old_build.rows_missing_hardest(profile).empty());
    }
    // ... nor is the chart's own analysis under other rules, ...
    {
        app::AnalysisSettings under_other = hardest_settings().to_analysis_settings();
        under_other.rules = hydra::test::other_rules();
        const PreparedRecord other = with_figures_emptied(prepare_record(
            chart.entry.md5, profile, ::corpus::analyzed(chart.entry.notespath, under_other)));
        REQUIRE_FALSE(squeeze_in_ranks(other).empty());
        RecordStore rules_store(":memory:");
        rules_store.rebuild_chart_library({chart.entry});
        rules_store.save(other);
        REQUIRE(rules_store.lookup_one(chart.entry.md5, profile).status == RecordStatus::Stale);
        CHECK(rules_store.rows_missing_hardest(profile).empty());
    }
    // ... nor a chart the library does not list.
    {
        RecordStore unlisted(":memory:");
        unlisted.save(with_figures_emptied(chart.fresh));
        REQUIRE(unlisted.lookup_one(chart.entry.md5, profile).status == RecordStatus::Ready);
        CHECK(unlisted.rows_missing_hardest(profile).empty());
    }
}

TEST_CASE("set_hardest fills only a record's empty figures and ignores a rank it does not have") {
    // Every figure here is any value a figure may hold, each one different,
    // so a figure written to the wrong rank shows. A figure of 0 is a real
    // one: a squeeze at exactly 0 needs 0.0 ms (D302).
    const double held = 12.5;
    const double for_rank_1 = 0.0;
    const double for_rank_2 = -315.8;
    const double for_held_rank = 99.0;
    const double for_missing_rank = 7.0;
    RecordStore store(":memory:");
    const Profile profile = hydra::test::at4();
    store.rebuild_chart_library({library_row("a", "A")});
    store.save(hand_record("a", profile,
                           {hand_path(1, std::nullopt, 1), hand_path(2, std::nullopt, 2),
                            hand_path(3, held, 1)}));
    const std::vector<MissingHardestRow> before = store.rows_missing_hardest(profile);
    REQUIRE(before.size() == 2);
    CHECK(before[0].rank == 1);
    CHECK(before[1].rank == 2);

    // Rank 3 already holds a figure, which the write leaves alone, and the
    // record has no rank 9.
    store.set_hardest("a", before[0].profile_id,
                      {{1, for_rank_1}, {2, for_rank_2}, {3, for_held_rank}, {9, for_missing_rank}});
    CHECK(store.rows_missing_hardest(profile).empty());
    CHECK(hydra::test::saved_figures(store, "a", profile) ==
          std::vector<std::optional<double>>{for_rank_1, for_rank_2, held});
}

TEST_CASE("the hardest-figure marker is per profile: unset at first, set for one profile after "
          "it is recorded, still unset for another, and it survives a reopen") {
    const std::string path = testtemp::temp_path("hardest_marker", ".db");
    remove_db(path);
    const SqueezedChart& chart = squeezed_chart();
    const Profile filled = hardest_settings().profile();
    const Profile other = in_other_mode(filled, "other");
    // No record is filed under this one, so it has nothing to fill.
    const Profile empty = in_other_mode(filled, "empty");
    {
        RecordStore store(path);
        save_squeezed(store, chart, filled, squeeze_in_ranks(chart.fresh));
        save_squeezed(store, chart, other, squeeze_in_ranks(chart.fresh));
        CHECK_FALSE(store.hardest_figure_filled(filled));
        CHECK_FALSE(store.hardest_figure_filled(other));
        CHECK(store.hardest_figure_filled(empty));

        store.mark_hardest_figure_filled(filled);
        CHECK(store.hardest_figure_filled(filled));
        CHECK_FALSE(store.hardest_figure_filled(other));
    }
    {
        RecordStore reopened(path);
        CHECK(reopened.hardest_figure_filled(filled));
        CHECK_FALSE(reopened.hardest_figure_filled(other));
    }
    remove_db(path);
}

TEST_CASE("rows_missing_hardest for a profile with other rows returns none of theirs") {
    const std::string path = testtemp::temp_path("hardest_profiles", ".db");
    remove_db(path);
    const SqueezedChart& chart = squeezed_chart();
    const std::vector<int> squeezed = squeeze_in_ranks(chart.fresh);
    REQUIRE_FALSE(squeezed.empty());
    const Profile mine = hardest_settings().profile();
    const Profile theirs = in_other_mode(mine, "theirs");
    {
        RecordStore store(path);
        // The chart's record under each profile, every squeeze-in figure
        // emptied, so each profile has a candidate at every squeeze-in rank.
        save_squeezed(store, chart, mine, squeezed);
        save_squeezed(store, chart, theirs, squeezed);

        // Each profile reads its own rows, all filed under its own profile row.
        std::set<int64_t> my_profile_rows;
        std::set<int64_t> their_profile_rows;
        const std::vector<MissingHardestRow> rows = store.rows_missing_hardest(mine);
        CHECK(rows.size() == squeezed.size());
        for (const MissingHardestRow& row : rows) my_profile_rows.insert(row.profile_id);
        const std::vector<MissingHardestRow> their_rows = store.rows_missing_hardest(theirs);
        CHECK(their_rows.size() == squeezed.size());
        for (const MissingHardestRow& row : their_rows) their_profile_rows.insert(row.profile_id);
        CHECK(my_profile_rows.size() == 1);
        CHECK(their_profile_rows.size() == 1);
        CHECK(my_profile_rows != their_profile_rows);

        // A profile the store holds no row for has no candidates, and asking
        // files no profile row for it.
        const int64_t profile_rows = scalar_on_file(path, "SELECT COUNT(*) FROM profiles");
        const Profile none = in_other_mode(mine, "none");
        CHECK(store.rows_missing_hardest(none).empty());
        CHECK(store.hardest_figure_filled(none));
        CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM profiles") == profile_rows);
    }
    remove_db(path);
}
