// Tests for the library table in store/record_store.{h,cpp}: a scan rebuilds
// it whole or not at all, a result takes its names from it, its rescan cache
// is read only under a current stamp, and a chart the scan no longer lists
// loses its stored rows.

#include "doctest.h"

#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/model.h"
#include "db_file_util.h"  // exec_on_file, scalar_on_file
#include "display_fixtures.h"  // add_stale_rows
#include "library_fixtures.h"  // library_row
#include "record_fixtures.h"
#include "store/record_store.h"
#include "store_fixtures.h"  // at_cap
#include "temp_util.h"

using namespace hydra;
using namespace hydra::store;

using hydra::test::at_cap;
using hydra::test::exec_on_file;
// One integer straight out of a database file: how these tests look at a
// table without the store growing an accessor for it.
using hydra::test::scalar_on_file;

TEST_CASE("a failed library rebuild keeps the previous scan") {
    // The rebuild used to drop the table before opening its transaction, so
    // an insert that failed left the library empty, and took the rescan cache
    // with it. A trigger that refuses one md5 makes an insert fail partway.
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
    // User decision 2026-09-26: fixing song.ini reaches the reports. A
    // result's names come from the library (kNamingCopiesSql), so a rescan
    // after song.ini changed renames it.
    RecordStore store(":memory:");
    const ChartLibraryEntry first = library_row("h", "Scanned Title");
    ChartLibraryEntry second = library_row("h", "Second Copy");
    second.notespath = "C:\\charts\\copy\\notes.chart";
    store.rebuild_chart_library({first, second});
    store.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4));
    auto listed = [&] {
        std::vector<RecordListing> rows =
            store.list_records(std::nullopt, CapQuery::at(4), Lens{}, SortColumn::Score, true);
        REQUIRE(rows.size() == 1);
        return rows[0];
    };
    // The scan found two copies of the chart; the first one it listed names
    // it.
    CHECK(listed().ref_name == "Scanned Title");

    ChartLibraryEntry renamed = library_row("h", "New Title");
    renamed.artist = "New Artist";
    renamed.charter = "New Charter";
    store.rebuild_chart_library({renamed, second});
    CHECK(listed().ref_name == "New Title");
    CHECK(listed().ref_artist == "New Artist");
    CHECK(listed().ref_charter == "New Charter");

    // A chart the scan found but nobody analyzed has no result to count.
    store.rebuild_chart_library({renamed, library_row("x", "Never Analyzed")});
    CHECK(store.counts().first == 1);
}

// ---- store speed (2026-09-26 audit, Task 9) --------------------------------

TEST_CASE("the library table's md5 index exists") {
    // D76: the scan's purge and list_records find a chart's library rows by
    // md5; without the index each lookup read the whole library table.
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
    // (kNamingCopiesSql), a single copy included.
    RecordStore store(":memory:");
    const ChartLibraryEntry first = library_row("h", "Scanned Title");
    ChartLibraryEntry second = library_row("h", "Second Copy");
    second.notespath = "C:\\charts\\copy\\notes.chart";
    store.rebuild_chart_library({first, second, library_row("s", "Single Copy")});

    store.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4));
    store.add_record(RecordKey{"s", "mode", CapQuery::at(4)}, at_cap(4));
    std::map<std::string, std::string> names;
    for (const RecordListing& row :
         store.list_records(std::nullopt, CapQuery::at(4), Lens{}, SortColumn::Score, true))
        names[row.hyhash] = row.ref_name;
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
    store.save_analysis(prepare_row(RecordKey{hash, mode, CapQuery::at(4)}, at_cap(4)));
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
        CHECK(store.library_copies() == std::unordered_map<std::string, int>{{"a", 1}});
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
        save_chart(store, "a", "mode");
        save_chart(store, "a", "other mode");
        test::add_stale_rows(store, at_cap(8), RecordKey{"a", "build", CapQuery::at(8)},
                             RecordKey{"a", "rules", CapQuery::at(8)},
                             RecordKey{"a", "both", CapQuery::at(8)});
        save_chart(store, "b");
        REQUIRE(store.get_summary(RecordKey{"a", "build", CapQuery::at(8)}).status ==
                RecordStatus::Stale);
    }
    REQUIRE(results_of(path, "a") == 5);
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A")});
    }
    CHECK(results_of(path, "a") == 5);
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
        CHECK(store.library_copies() == std::unordered_map<std::string, int>{{"a2", 1}, {"b", 1}});
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
        CHECK(store.library_copies() == std::unordered_map<std::string, int>{{"a", 1}, {"a2", 1}});
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
