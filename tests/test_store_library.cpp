// Tests for the library table in store/record_store.{h,cpp}: a scan rebuilds
// it whole or not at all, a result takes its names from it, its rescan cache
// is read only under a current stamp, a chart the scan no longer lists
// loses its stored rows, a chart list read reports its rows and can be
// stopped, and both read orders (LibraryOrder) give the rows they promise.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/error_kind.h"  // Cancelled
#include "core/model.h"
#include "db_file_util.h"  // exec_on_file, scalar_on_file
#include "display_fixtures.h"  // save_old_build_record, other_rules_record
#include "library_fixtures.h"  // library_row
#include "record_fixtures.h"
#include "store/chart_index.h"
#include "store/record_store.h"
#include "store_fixtures.h"  // at_cap, at4
#include "temp_util.h"

using namespace hydra;
using namespace hydra::store;

using hydra::test::at4;
using hydra::test::at_cap;
using hydra::test::exec_on_file;
// One integer straight out of a database file: how these tests look at a
// table without the store growing an accessor for it.
using hydra::test::scalar_on_file;

namespace {

// How many copies the library lists of each chart it lists, by song id, as
// the store's chart index reads them.
std::map<std::string, int> listed_copies(RecordStore& store) {
    const ChartIndex index = store.chart_index();
    std::map<std::string, int> out;
    for (const std::string& id : index.song_ids()) out[id] = index.copies(id);
    return out;
}

// The title, artist, charter and album of the library row that names chart
// `id`.
struct ChartNames {
    std::string title, artist, charter, album;
};
ChartNames names_of(RecordStore& store, const std::string& id) {
    const ChartIndex index = store.chart_index();
    const ChartLibraryEntry* naming = index.naming(id);
    REQUIRE(naming != nullptr);
    return ChartNames{naming->title, naming->artist, naming->charter, naming->album};
}

}  // namespace

TEST_CASE("a library entry filled by position keeps its first seven fields in place") {
    // Tests and fixtures (test_cli.cpp, dm_fixture.h) fill the entry by
    // position with seven strings. A field added among them would shift the
    // path, root folder and fingerprint into the wrong slots with no compile
    // error, so a new field goes after these seven.
    const ChartLibraryEntry e{"md5", "Title", "Artist", "Charter",
                              "C:\\charts\\a\\notes.chart", "C:\\charts", "sig"};
    CHECK(e.md5 == "md5");
    CHECK(e.title == "Title");
    CHECK(e.artist == "Artist");
    CHECK(e.charter == "Charter");
    CHECK(e.notespath == "C:\\charts\\a\\notes.chart");
    CHECK(e.rootfolder == "C:\\charts");
    CHECK(e.sig == "sig");
    CHECK(e.album.empty());
    CHECK(!e.timing.has_value());
}

TEST_CASE("a failed library rebuild keeps the previous scan") {
    // The rebuild's transaction opens before anything changes, so an insert
    // that fails leaves the library and its rescan cache as they were. A
    // trigger that refuses one md5 makes an insert fail partway.
    const std::string path = testtemp::temp_path("rebuild_fail", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B")});
    }
    exec_on_file(path,
                 "CREATE TRIGGER refuse_boom BEFORE INSERT ON charts WHEN NEW.md5 = 'boom'"
                 " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    {
        RecordStore store(path);
        CHECK_THROWS(
            store.rebuild_chart_library({library_row("c", "C"), library_row("boom", "Boom")}));
        CHECK(store.chart_library_count() == 2);
        const ChartLibraryCache cache = store.chart_library_cache();
        CHECK(cache.count("C:\\charts\\a\\notes.chart") == 1);
        CHECK(cache.count("C:\\charts\\b\\notes.chart") == 1);
        // No transaction was left open: the next rebuild goes through.
        store.rebuild_chart_library({library_row("c", "C")});
        CHECK(store.chart_library_count() == 1);
    }
    std::remove(path.c_str());
}

TEST_CASE("a result's names follow the latest scan") {
    // Library names win in the reports, so a scan brings a fixed song.ini in:
    // a result's names come from the library (ChartIndex), and a rescan after
    // song.ini changed renames it.
    RecordStore store(":memory:");
    ChartLibraryEntry first = library_row("h", "Scanned Title");
    first.album = "Scanned Album";
    ChartLibraryEntry second = library_row("h", "Second Copy");
    second.notespath = "C:\\charts\\copy\\notes.chart";
    store.rebuild_chart_library({first, second});
    store.save(prepare_record("h", at4(), at_cap(4)));
    // The scan found two copies of the chart; the first one it listed names
    // it.
    CHECK(names_of(store, "h").title == "Scanned Title");
    CHECK(names_of(store, "h").album == "Scanned Album");

    ChartLibraryEntry renamed = library_row("h", "New Title");
    renamed.artist = "New Artist";
    renamed.charter = "New Charter";
    renamed.album = "New Album";
    store.rebuild_chart_library({renamed, second});
    CHECK(names_of(store, "h").title == "New Title");
    CHECK(names_of(store, "h").artist == "New Artist");
    CHECK(names_of(store, "h").charter == "New Charter");
    CHECK(names_of(store, "h").album == "New Album");
    // The rescans kept the chart's record.
    CHECK(store.lookup_one("h", at4()).status == RecordStatus::Ready);

    // A chart the scan found but nobody analyzed has no result to count.
    store.rebuild_chart_library({renamed, library_row("x", "Never Analyzed")});
    CHECK(store.record_count() == 1);
}

// ---- store speed (2026-09-26 audit, Task 9) --------------------------------

TEST_CASE("the library table's md5 index exists") {
    // D76: the scan's purge and list_chart_library_copies find a chart's
    // library rows by md5; without the index each lookup read the whole
    // library table.
    const std::string path = testtemp::temp_path("md5_index", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM sqlite_master"
                       " WHERE type='index' AND name='charts_by_md5'") == 1);
    std::remove(path.c_str());
}

// ---- library summaries (interface redesign, Task 7) ------------------------

TEST_CASE("the scan's first copy names a chart whatever copy was analyzed") {
    // D51 call 10 (finding 63): one rule names a chart the scan found twice,
    // the first copy it listed. Every chart takes its names from the library
    // (ChartIndex), a single copy included.
    RecordStore store(":memory:");
    const ChartLibraryEntry first = library_row("h", "Scanned Title");
    ChartLibraryEntry second = library_row("h", "Second Copy");
    second.notespath = "C:\\charts\\copy\\notes.chart";
    store.rebuild_chart_library({first, second, library_row("s", "Single Copy")});

    store.save(prepare_record("h", at4(), at_cap(4)));
    store.save(prepare_record("s", at4(), at_cap(4)));
    std::map<std::string, std::string> names;
    for (const RecordRow& record : store.ready_records(at4()))
        names[record.song_id] = names_of(store, record.song_id).title;
    CHECK(names ==
          std::map<std::string, std::string>{{"h", "Scanned Title"}, {"s", "Single Copy"}});
}

TEST_CASE("the scan cache is dropped when its reader stamp is not current") {
    // D51 call 12 (finding 345): the rescan cache carries kChartMetaStamp. A
    // file without a current stamp hands back no cache, so the next scan
    // reads every chart again and stamps the table it writes.
    const std::string path = testtemp::temp_path("chart_meta_stamp", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry entry = library_row("a", "A");
    {
        RecordStore store(path);
        store.rebuild_chart_library({entry});
    }
    {
        RecordStore store(path);
        CHECK(store.chart_library_cache().count(entry.notespath) == 1);
    }
    exec_on_file(path, "DELETE FROM meta WHERE key = 'chart_meta_version'");
    {
        RecordStore store(path);
        CHECK(store.chart_library_cache().empty());
        store.rebuild_chart_library({entry});
        CHECK(store.chart_library_cache().count(entry.notespath) == 1);
    }
    // A file the scan stamped before it read albums: its rows hold no album,
    // so its cache is dropped too.
    exec_on_file(path, "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version'");
    {
        RecordStore store(path);
        CHECK(store.chart_library_cache().empty());
    }
    std::remove(path.c_str());
}

TEST_CASE("the library is out of date only when it has rows and its reader stamp is not current") {
    const std::string path = testtemp::temp_path("library_out_of_date", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry entry = library_row("a", "A");
    {
        RecordStore store(path);
        CHECK_FALSE(store.chart_library_out_of_date());
        store.rebuild_chart_library({entry});
        CHECK_FALSE(store.chart_library_out_of_date());
    }
    exec_on_file(path, "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version'");
    {
        RecordStore store(path);
        CHECK(store.chart_library_out_of_date());
    }
    exec_on_file(path, "DELETE FROM meta WHERE key = 'chart_meta_version'");
    {
        RecordStore store(path);
        CHECK(store.chart_library_out_of_date());
        store.rebuild_chart_library({entry});
        CHECK_FALSE(store.chart_library_out_of_date());
    }
    std::remove(path.c_str());
}

TEST_CASE("a library from before the album column is out of date after the open adds it") {
    // The charts table and scan stamp a build from before the album wrote, as
    // seed_without_album in test_store_open.cpp makes them.
    const std::string path = testtemp::temp_path("library_out_of_date_album", ".db");
    std::remove(path.c_str());
    {
        RecordStore seed(path);
        seed.rebuild_chart_library({library_row("a", "A")});
    }
    exec_on_file(path, "ALTER TABLE charts DROP COLUMN album;"
                       "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version';");
    {
        RecordStore store(path);
        CHECK(store.chart_library_count() == 1);
        CHECK(store.chart_library_out_of_date());
    }
    std::remove(path.c_str());
}

// ---- charts the library no longer lists (D87 items 3 and 4, task storage-T1) --

namespace {

// How many records one chart holds, read straight out of the file.
int64_t results_of(const std::string& path, const std::string& hash) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM records WHERE song_id = '" + hash + "'");
}

// One analyzed chart as the click saves it.
void save_chart(RecordStore& store, const std::string& hash, const std::string& mode = "mode") {
    store.save(prepare_record(hash, at4(mode), at_cap(4)));
}

// library_row's row for another folder holding the same chart.
ChartLibraryEntry second_copy(const char* md5) {
    ChartLibraryEntry copy = library_row(md5, "Copy");
    copy.notespath = std::string("C:\\other\\") + md5 + "\\notes.chart";
    copy.rootfolder = "C:\\other";
    return copy;
}

}  // namespace

TEST_CASE("a scan that drops a chart deletes its stored rows") {
    const std::string path = testtemp::temp_path("scan_drops_chart", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B")});
        save_chart(store, "a");
        save_chart(store, "b");
    }
    REQUIRE(results_of(path, "a") == 1);
    REQUIRE(results_of(path, "b") == 1);
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A")});
    }
    CHECK(results_of(path, "b") == 0);
    CHECK(results_of(path, "a") == 1);
    std::remove(path.c_str());
}

TEST_CASE("a chart with two library copies keeps its rows when one copy leaves") {
    const std::string path = testtemp::temp_path("scan_drops_copy", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), second_copy("a")});
        save_chart(store, "a");
    }
    REQUIRE(results_of(path, "a") == 1);
    {
        RecordStore store(path);
        store.rebuild_chart_library({second_copy("a")});
        CHECK(listed_copies(store) == std::map<std::string, int>{{"a", 1}});
    }
    CHECK(results_of(path, "a") == 1);
    std::remove(path.c_str());
}

TEST_CASE("a library chart keeps its rows for other chart modes, including Stale ones") {
    const std::string path = testtemp::temp_path("scan_keeps_modes", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B")});
        const Profile rules{"rules", CapQuery::at(8), Lens{}};
        const Profile build{"build", CapQuery::at(8), Lens{}};
        save_chart(store, "a", "mode");
        save_chart(store, "a", "other mode");
        save_chart(store, "b");
        store.save(prepare_record("a", rules, test::other_rules_record(at_cap(8))));
        // A save purges the chart's records from other builds (RecordStore::save
        // says which), so the other build's record is the chart's last save.
        test::save_old_build_record(store, prepare_record("a", build, at_cap(8)));
        REQUIRE(store.lookup_one("a", rules).status == RecordStatus::Stale);
        REQUIRE(store.lookup_one("a", rules).stale_rules);
        REQUIRE(store.lookup_one("a", build).status == RecordStatus::Stale);
        REQUIRE(store.lookup_one("a", build).stale_build);
    }
    REQUIRE(results_of(path, "a") == 4);
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A")});
    }
    CHECK(results_of(path, "a") == 4);
    CHECK(results_of(path, "b") == 0);
    std::remove(path.c_str());
}

TEST_CASE("reidentify_chart moves the library row to the new md5 and deletes the old md5's rows") {
    const std::string path = testtemp::temp_path("reidentify_moves", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry a = library_row("a", "A");
    {
        RecordStore store(path);
        store.rebuild_chart_library({a, library_row("b", "B")});
        save_chart(store, "a");
        save_chart(store, "b");
    }
    REQUIRE(results_of(path, "a") == 1);
    REQUIRE(results_of(path, "b") == 1);
    {
        RecordStore store(path);
        store.reidentify_chart(a.notespath, "a2", "sig-a2");
        CHECK(listed_copies(store) == std::map<std::string, int>{{"a2", 1}, {"b", 1}});
        const ChartLibraryCache cache = store.chart_library_cache();
        REQUIRE(cache.count(a.notespath) == 1);
        CHECK(cache.at(a.notespath).md5 == "a2");
        CHECK(cache.at(a.notespath).sig == "sig-a2");
    }
    CHECK(results_of(path, "a") == 0);
    CHECK(results_of(path, "b") == 1);
    std::remove(path.c_str());
}

TEST_CASE("reidentify_chart keeps the old md5's rows while another library row has it") {
    const std::string path = testtemp::temp_path("reidentify_keeps", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry copy = second_copy("a");
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), copy});
        save_chart(store, "a");
    }
    REQUIRE(results_of(path, "a") == 1);
    {
        RecordStore store(path);
        store.reidentify_chart(copy.notespath, "a2", "sig-a2");
        CHECK(listed_copies(store) == std::map<std::string, int>{{"a", 1}, {"a2", 1}});
    }
    CHECK(results_of(path, "a") == 1);
    std::remove(path.c_str());
}

TEST_CASE("a scan that fails partway through its deletes keeps the dropped chart's rows") {
    // The deletes run inside the scan's own transaction. A trigger that
    // refuses the records delete fails it after the library rows changed.
    const std::string path = testtemp::temp_path("scan_delete_fails", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B")});
        save_chart(store, "a");
        save_chart(store, "b");
    }
    REQUIRE(results_of(path, "b") == 1);
    exec_on_file(path,
                 "CREATE TRIGGER refuse_result_delete BEFORE DELETE ON records"
                 " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    {
        RecordStore store(path);
        CHECK_THROWS(store.rebuild_chart_library({library_row("a", "A")}));
        CHECK(store.chart_library_count() == 2);
    }
    CHECK(results_of(path, "b") == 1);
    std::remove(path.c_str());
}

namespace {

// The md5s of `rows`, in the order they came.
std::vector<std::string> md5s_of(const std::vector<ChartLibraryEntry>& rows) {
    std::vector<std::string> out;
    for (const ChartLibraryEntry& e : rows) out.push_back(e.md5);
    return out;
}

}  // namespace

TEST_CASE("store: the chart list reports each row it reads") {
    RecordStore store(":memory:");
    store.rebuild_chart_library({library_row("d", "D"), library_row("b", "B"),
                                 library_row("a", "A"), library_row("c", "C")});
    const std::vector<ChartLibraryEntry> plain = store.list_chart_library(0, -1);

    std::vector<int64_t> reported;
    const std::vector<ChartLibraryEntry> counted =
        store.list_chart_library(0, -1, [&](int64_t rows_read) {
            reported.push_back(rows_read);
            return true;
        });
    CHECK(reported == std::vector<int64_t>{1, 2, 3, 4});
    CHECK(static_cast<int64_t>(counted.size()) == store.chart_library_count());
    // The count changes nothing about the rows or their order.
    CHECK(md5s_of(counted) == md5s_of(plain));
    CHECK(md5s_of(plain) == std::vector<std::string>{"a", "b", "c", "d"});
}

TEST_CASE("store: a chart list read that is told to stop stops") {
    RecordStore store(":memory:");
    store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B"),
                                 library_row("c", "C"), library_row("d", "D")});

    std::vector<int64_t> reported;
    bool cancelled = false;
    try {
        store.list_chart_library(0, -1, [&](int64_t rows_read) {
            reported.push_back(rows_read);
            return rows_read < 2;
        });
    } catch (const Cancelled& e) {
        cancelled = e.kind() == ErrorKind::Cancelled;
    }
    CHECK(cancelled);
    CHECK(reported == std::vector<int64_t>{1, 2});
    // The stopped read left the store able to answer the next one.
    CHECK(store.list_chart_library(0, -1).size() == 4);
    CHECK(store.chart_library_count() == 4);
}

namespace {

// The md5s of `rows`, sorted: which rows came, whatever their order.
std::vector<std::string> md5_set_of(const std::vector<ChartLibraryEntry>& rows) {
    std::vector<std::string> out = md5s_of(rows);
    std::sort(out.begin(), out.end());
    return out;
}

// Checks two reads of one row gave the same row, field for field.
void check_same_entry(const ChartLibraryEntry& n, const ChartLibraryEntry& e) {
    CHECK(n.title == e.title);
    CHECK(n.artist == e.artist);
    CHECK(n.charter == e.charter);
    CHECK(n.album == e.album);
    CHECK(n.notespath == e.notespath);
    CHECK(n.rootfolder == e.rootfolder);
    CHECK(n.sig == e.sig);
    CHECK(n.timing == e.timing);
}

}  // namespace

TEST_CASE("store: the unsorted chart list read returns the rows the read by name does") {
    RecordStore store(":memory:");
    store.rebuild_chart_library({library_row("d", "D"), library_row("b", "B"),
                                 library_row("a", "A"), library_row("c", "C")});
    const std::vector<ChartLibraryEntry> by_name = store.list_chart_library(0, -1);

    std::vector<int64_t> reported;
    const std::vector<ChartLibraryEntry> unsorted = store.list_chart_library(
        0, -1,
        [&](int64_t rows_read) {
            reported.push_back(rows_read);
            return true;
        },
        LibraryOrder::Unsorted);
    CHECK(reported == std::vector<int64_t>{1, 2, 3, 4});
    CHECK(md5_set_of(unsorted) == md5_set_of(by_name));
    // The same rows, field for field, whichever order read them.
    for (const ChartLibraryEntry& e : unsorted) {
        CAPTURE(e.md5);
        bool found = false;
        for (const ChartLibraryEntry& n : by_name) {
            if (n.md5 != e.md5) continue;
            found = true;
            check_same_entry(n, e);
        }
        CHECK(found);
    }
}

TEST_CASE("store: an unsorted chart list read that is told to stop stops") {
    RecordStore store(":memory:");
    store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B"),
                                 library_row("c", "C"), library_row("d", "D")});

    std::vector<int64_t> reported;
    bool cancelled = false;
    try {
        store.list_chart_library(
            0, -1,
            [&](int64_t rows_read) {
                reported.push_back(rows_read);
                return rows_read < 2;
            },
            LibraryOrder::Unsorted);
    } catch (const Cancelled& e) {
        cancelled = e.kind() == ErrorKind::Cancelled;
    }
    CHECK(cancelled);
    CHECK(reported == std::vector<int64_t>{1, 2});
    CHECK(store.list_chart_library(0, -1, {}, LibraryOrder::Unsorted).size() == 4);
    CHECK(store.chart_library_count() == 4);
}

namespace {

// A library file with more rows than kLibraryOpenThreads, so the whole
// unsorted read of it reads on every thread. A ":memory:" store reads on one
// connection, since no other connection can open it.
constexpr int kThreadedRows = 40;
static_assert(kThreadedRows > static_cast<int>(kLibraryOpenThreads),
              "every thread needs a share of the rows");

std::string threaded_library_file(const char* name) {
    const std::string path = testtemp::temp_path(name, ".db");
    std::remove(path.c_str());
    std::vector<ChartLibraryEntry> rows;
    for (int i = 0; i < kThreadedRows; ++i) {
        const std::string id = "c" + std::to_string(i);
        rows.push_back(library_row(id, "Song " + std::to_string(kThreadedRows - i)));
    }
    RecordStore store(path);
    store.rebuild_chart_library(rows);
    return path;
}

}  // namespace

TEST_CASE("store: the chart list read on threads returns the rows the one-connection read does") {
    const std::string path = threaded_library_file("threaded_rows");
    {
        RecordStore store(path);
        // The read by name stays on the store's one connection.
        const std::vector<ChartLibraryEntry> one = store.list_chart_library(0, -1);
        const std::vector<ChartLibraryEntry> threaded =
            store.list_chart_library(0, -1, {}, LibraryOrder::Unsorted);
        REQUIRE(one.size() == static_cast<size_t>(kThreadedRows));
        CHECK(md5_set_of(threaded) == md5_set_of(one));
        // The same rows, field for field.
        for (const ChartLibraryEntry& e : threaded) {
            CAPTURE(e.md5);
            int found = 0;
            for (const ChartLibraryEntry& n : one) {
                if (n.md5 != e.md5) continue;
                ++found;
                check_same_entry(n, e);
            }
            CHECK(found == 1);
        }
    }
    std::remove(path.c_str());
}

TEST_CASE("store: the chart list read on threads counts up to the row count once") {
    const std::string path = threaded_library_file("threaded_count");
    {
        RecordStore store(path);
        std::vector<int64_t> reported;
        const std::vector<ChartLibraryEntry> rows = store.list_chart_library(
            0, -1,
            [&](int64_t rows_read) {
                reported.push_back(rows_read);
                return true;
            },
            LibraryOrder::Unsorted);
        CHECK(rows.size() == static_cast<size_t>(kThreadedRows));
        // Every count from 1 to the row count, once each and in order, as
        // the one-connection read reports them.
        std::vector<int64_t> every_row;
        for (int64_t i = 1; i <= store.chart_library_count(); ++i) every_row.push_back(i);
        CHECK(reported == every_row);
    }
    std::remove(path.c_str());
}

TEST_CASE("store: a chart list read on threads that is told to stop stops every thread") {
    const std::string path = threaded_library_file("threaded_stop");
    {
        RecordStore store(path);
        std::vector<int64_t> reported;
        bool cancelled = false;
        try {
            store.list_chart_library(
                0, -1,
                [&](int64_t rows_read) {
                    reported.push_back(rows_read);
                    return false;  // stop at the first report
                },
                LibraryOrder::Unsorted);
        } catch (const Cancelled& e) {
            cancelled = e.kind() == ErrorKind::Cancelled;
        }
        CHECK(cancelled);
        CHECK(reported.size() == 1);
        // The store answers the next calls.
        CHECK(store.chart_library_count() == kThreadedRows);
        CHECK(store.list_chart_library(0, -1, {}, LibraryOrder::Unsorted).size() ==
              static_cast<size_t>(kThreadedRows));
    }
    std::remove(path.c_str());
}

TEST_CASE("store: a chart list read on threads stops at whichever count asks") {
    const std::string path = threaded_library_file("threaded_stop_mid");
    {
        RecordStore store(path);
        // A stop at the second count, a middle one and the last one each end
        // the read as a cancel, with every count up to the stop told and
        // none past it.
        for (int64_t stop_at : {int64_t{2}, int64_t{kThreadedRows / 2}, int64_t{kThreadedRows}}) {
            CAPTURE(stop_at);
            std::vector<int64_t> reported;
            bool cancelled = false;
            try {
                store.list_chart_library(
                    0, -1,
                    [&](int64_t rows_read) {
                        reported.push_back(rows_read);
                        return rows_read < stop_at;
                    },
                    LibraryOrder::Unsorted);
            } catch (const Cancelled& e) {
                cancelled = e.kind() == ErrorKind::Cancelled;
            }
            CHECK(cancelled);
            std::vector<int64_t> up_to_stop;
            for (int64_t i = 1; i <= stop_at; ++i) up_to_stop.push_back(i);
            CHECK(reported == up_to_stop);
        }
    }
    std::remove(path.c_str());
}

TEST_CASE("store: a chart list read on threads whose connection fails to open fails whole") {
    const std::string path = threaded_library_file("threaded_open_fails");
    {
        RecordStore store(path);
        store.fail_chart_list_reader_open_for_test(2);
        std::vector<int64_t> reported;
        std::vector<ChartLibraryEntry> rows;
        bool failed_read = false;
        try {
            rows = store.list_chart_library(
                0, -1,
                [&](int64_t rows_read) {
                    reported.push_back(rows_read);
                    return true;
                },
                LibraryOrder::Unsorted);
        } catch (const KindedError& e) {
            failed_read = e.kind() == ErrorKind::DatabaseRead;
        }
        // The store's plain failed read, with no rows: nothing was read and
        // nothing was counted.
        CHECK(failed_read);
        CHECK(rows.empty());
        CHECK(reported.empty());
        // The forced failure was for one read only; the next one reads it all.
        CHECK(store.list_chart_library(0, -1, {}, LibraryOrder::Unsorted).size() ==
              static_cast<size_t>(kThreadedRows));
    }
    std::remove(path.c_str());
}

TEST_CASE("store: rows read by name compare byte for byte, and one name keeps scan order") {
    // Four copies of one chart and one other chart. "Song" twice, scanned
    // with the later path first; "song" (lowercase sorts after every
    // uppercase letter byte for byte) and "Alpha".
    std::vector<ChartLibraryEntry> rows = {library_row("x", "Song"), library_row("x", "song"),
                                           library_row("x", "Song"), library_row("x", "Alpha"),
                                           library_row("y", "Other")};
    const char* paths[] = {"C:\\z\\notes.chart", "C:\\l\\notes.chart", "C:\\a\\notes.chart",
                           "C:\\q\\notes.chart", "C:\\o\\notes.chart"};
    for (size_t i = 0; i < rows.size(); ++i) rows[i].notespath = paths[i];
    RecordStore store(":memory:");
    store.rebuild_chart_library(rows);

    std::vector<std::string> copies;
    for (const ChartLibraryEntry& e : store.list_chart_library_copies("x"))
        copies.push_back(e.notespath);
    CHECK(copies == std::vector<std::string>{"C:\\q\\notes.chart", "C:\\z\\notes.chart",
                                             "C:\\a\\notes.chart", "C:\\l\\notes.chart"});
    std::vector<std::string> listed;
    for (const ChartLibraryEntry& e : store.list_chart_library(0, -1)) listed.push_back(e.notespath);
    CHECK(listed == std::vector<std::string>{"C:\\q\\notes.chart", "C:\\o\\notes.chart",
                                             "C:\\z\\notes.chart", "C:\\a\\notes.chart",
                                             "C:\\l\\notes.chart"});
}
