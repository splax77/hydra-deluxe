// Tests for the library table in store/record_store.{h,cpp}: a scan rebuilds
// it whole or not at all, a result takes its names from it, its rescan cache
// is read only under a current stamp, a chart the scan no longer lists
// loses its stored rows, a chart list read reports its rows and can be
// stopped, and both read orders (LibraryOrder) give the rows they promise.

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "app/analysis.h"  // discover_charts, hash_chart_file
#include "audio_chart_fixtures.h"  // copy_file_utf8, write_text_file
#include "core/error_kind.h"  // Cancelled
#include "core/model.h"
#include "db_file_util.h"  // exec_on_file, scalar_on_file
#include "display_fixtures.h"  // save_old_build_record, other_rules_record
#include "library_fixtures.h"  // library_row
#include "record_fixtures.h"
#include "store/chart_index.h"
#include "store/record_store.h"
#include "store/stored_versions.h"  // kChartMetaStamp
#include "store_file_fixtures.h"  // StoreFile, HeldSaveGroup
#include "store_fixtures.h"  // at_cap, at4, fixture, fixture_chart, store_fixture_settings
#include "temp_util.h"
#include "wait_util.h"  // wait_until, WaitTimeout, kWaitCap

using namespace hydra;
using namespace hydra::store;

using hydra::test::HeldSaveGroup;
using hydra::test::StoreFile;
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

// The library rows the real scan (app::discover_charts) writes for the
// corpus, scanned once per run: each row a rescan could match
// (sig_can_show_unchanged), the first row of each md5 only.
const std::vector<ChartLibraryEntry>& scanned_corpus() {
    static const std::vector<ChartLibraryEntry> rows = [] {
        std::pair<std::vector<ChartLibraryEntry>, std::vector<std::string>> scan =
            hydra::app::discover_charts({corpus::root()});
        if (!scan.second.empty())
            throw std::runtime_error("corpus scan error: " + scan.second.front());
        std::vector<ChartLibraryEntry> out;
        std::set<std::string> md5s;
        for (ChartLibraryEntry& e : scan.first)
            if (sig_can_show_unchanged(e.sig) && md5s.insert(e.md5).second)
                out.push_back(std::move(e));
        return out;
    }();
    return rows;
}

// The first `n` rows of scanned_corpus: `n` different charts.
std::vector<ChartLibraryEntry> corpus_rows(size_t n) {
    const std::vector<ChartLibraryEntry>& all = scanned_corpus();
    REQUIRE(all.size() >= n);
    return std::vector<ChartLibraryEntry>(all.begin(),
                                          all.begin() + static_cast<std::ptrdiff_t>(n));
}

// The first scanned_corpus row of a chart other than `md5`.
ChartLibraryEntry corpus_row_other_than(const std::string& md5) {
    for (const ChartLibraryEntry& e : scanned_corpus())
        if (e.md5 != md5) return e;
    FAIL("the corpus scan lists no chart but " << md5);
    return ChartLibraryEntry{};
}

// The folder that holds the chart file `notespath`.
std::string folder_of(const std::string& notespath) {
    const size_t slash = notespath.find_last_of("\\/");
    REQUIRE(slash != std::string::npos);
    return notespath.substr(0, slash);
}

// Writes a song.ini into `dir` that names its chart `names`, as a chart
// editor saves one.
void write_song_ini(const std::string& dir, const ChartNames& names) {
    audiochart::write_text_file(dir + "\\song.ini",
                                "[song]\nname = " + names.title + "\nartist = " + names.artist +
                                    "\ncharter = " + names.charter + "\nalbum = " + names.album +
                                    "\n");
}

// One song folder of a scratch charts folder: the folder's name, the corpus
// chart file copied into it, and the names its song.ini gives.
struct SongFolder {
    std::string folder;
    std::string chart;
    ChartNames names;
};

// Writes each of `songs` under the scratch folder testtemp::temp_dir(tag),
// emptied first: its corpus chart file, copied byte for byte under its own
// file name, and its song.ini. Returns the scratch folder.
std::string write_song_folders(const std::string& tag, const std::vector<SongFolder>& songs) {
    const std::string root = testtemp::temp_dir(tag);
    testtemp::remove_dir(root);
    for (const SongFolder& song : songs) {
        const std::string dir = root + "\\" + song.folder;
        std::filesystem::create_directories(hydra::os_path(dir));
        audiochart::copy_file_utf8(song.chart,
                                   dir + song.chart.substr(folder_of(song.chart).size()));
        write_song_ini(dir, song.names);
    }
    return root;
}

// The library rows the real scan writes for the charts folder `root`, in the
// order it lists them. Fails when the scan reports an error.
std::vector<ChartLibraryEntry> scan_folder(const std::string& root) {
    std::pair<std::vector<ChartLibraryEntry>, std::vector<std::string>> scan =
        hydra::app::discover_charts({root});
    const std::string first_error = scan.second.empty() ? std::string() : scan.second.front();
    INFO(first_error);
    REQUIRE(scan.second.empty());
    return scan.first;
}

// The first of `rows` that lists chart `md5`: the copy the scan listed first.
const ChartLibraryEntry& first_listed(const std::vector<ChartLibraryEntry>& rows,
                                      const std::string& md5) {
    for (const ChartLibraryEntry& e : rows)
        if (e.md5 == md5) return e;
    FAIL("the scan lists no copy of " << md5);
    return rows.front();
}

// A scan stamp this build does not read as current (kChartMetaStamp): one
// below the stamp it writes, as an older build wrote it.
int stale_chart_meta_stamp() {
    const int stale = kChartMetaStamp.written - 1;
    REQUIRE_FALSE(kChartMetaStamp.is_current(stale));
    return stale;
}

// The SQL that sets the store file's scan stamp to `stamp`.
std::string set_chart_meta_stamp_sql(int stamp) {
    return "UPDATE meta SET value = '" + std::to_string(stamp) +
           "' WHERE key = 'chart_meta_version';";
}

// Takes the scan stamp out of the store file at `path`, as a file from
// before the stamp holds none.
void drop_chart_meta_stamp(const std::string& path) {
    exec_on_file(path, "DELETE FROM meta WHERE key = 'chart_meta_version'");
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
    // trigger that refuses one chart's md5 makes an insert fail partway. The
    // rows are the corpus scan's.
    const std::vector<ChartLibraryEntry> rows = corpus_rows(4);
    const std::string path = testtemp::temp_path("rebuild_fail", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({rows[0], rows[1]});
    }
    const std::string trigger = "CREATE TRIGGER refuse_one BEFORE INSERT ON charts WHEN NEW.md5 = '" +
                                rows[3].md5 + "' BEGIN SELECT RAISE(ABORT, 'refused'); END;";
    exec_on_file(path, trigger.c_str());
    {
        RecordStore store(path);
        CHECK_THROWS(store.rebuild_chart_library({rows[2], rows[3]}));
        CHECK(store.chart_library_count() == 2);
        const ChartLibraryCache cache = store.chart_library_cache();
        CHECK(cache.size() == 2);
        CHECK(cache.count(rows[0].notespath) == 1);
        CHECK(cache.count(rows[1].notespath) == 1);
        // No transaction was left open: the next rebuild goes through.
        store.rebuild_chart_library({rows[2]});
        CHECK(store.chart_library_count() == 1);
    }
    std::remove(path.c_str());
}

TEST_CASE("a result's names follow the latest scan") {
    // Library names win in the reports, so a scan brings a fixed song.ini in:
    // a result's names come from the library (ChartIndex), and a rescan after
    // song.ini changed renames it. Two folders hold copies of one corpus
    // chart; the real scan reads them, and the chart's record is the
    // engine's analysis of it.
    const std::string root = write_song_folders(
        "names_follow_scan",
        {SongFolder{"first", hydra::test::fixture_chart(),
                    ChartNames{"Scanned Title", "Scanned Artist", "Scanned Charter",
                               "Scanned Album"}},
         SongFolder{"second", hydra::test::fixture_chart(),
                    ChartNames{"Second Copy", "Second Artist", "Second Charter",
                               "Second Album"}}});
    const std::vector<ChartLibraryEntry> scanned = scan_folder(root);
    REQUIRE(scanned.size() == 2);
    const std::string id = scanned[0].md5;
    REQUIRE(scanned[1].md5 == id);  // one chart, two copies
    REQUIRE(scanned[0].title != scanned[1].title);
    RecordStore store(":memory:");
    store.rebuild_chart_library(scanned);
    store.save(prepare_record(id, at4(), hydra::test::fixture().record));
    // The first copy the scan listed names it.
    CHECK(names_of(store, id).title == scanned[0].title);
    CHECK(names_of(store, id).album == scanned[0].album);

    // The naming copy's song.ini changes, and the next scan reads it.
    write_song_ini(folder_of(scanned[0].notespath),
                   ChartNames{"New Title", "New Artist", "New Charter", "New Album"});
    const std::vector<ChartLibraryEntry> rescanned = scan_folder(root);
    REQUIRE(rescanned.size() == 2);
    REQUIRE(rescanned[0].notespath == scanned[0].notespath);
    REQUIRE(rescanned[0].title != scanned[0].title);
    store.rebuild_chart_library(rescanned);
    CHECK(names_of(store, id).title == rescanned[0].title);
    CHECK(names_of(store, id).artist == rescanned[0].artist);
    CHECK(names_of(store, id).charter == rescanned[0].charter);
    CHECK(names_of(store, id).album == rescanned[0].album);
    // The rescans kept the chart's record.
    CHECK(store.lookup_one(id, at4()).status == RecordStatus::Ready);

    // A chart the scan found but nobody analyzed has no result to count.
    store.rebuild_chart_library({rescanned[0], corpus_row_other_than(id)});
    CHECK(store.record_count() == 1);
    testtemp::remove_dir(root);
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
    // (ChartIndex), a single copy included. Two corpus charts, the first in
    // two folders, are read by the real scan and analyzed by the engine.
    const std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(hydra::test::store_fixture_settings(), 2);
    REQUIRE(charts.size() == 2);
    const std::string twice = hydra::app::hash_chart_file(charts[0].chart);
    const std::string once = hydra::app::hash_chart_file(charts[1].chart);
    REQUIRE(twice != once);
    const std::string root = write_song_folders(
        "first_copy_names",
        {SongFolder{"copy_a", charts[0].chart,
                    ChartNames{"Scanned Title", "Artist", "Charter", "Album"}},
         SongFolder{"copy_b", charts[0].chart,
                    ChartNames{"Second Copy", "Artist", "Charter", "Album"}},
         SongFolder{"single", charts[1].chart,
                    ChartNames{"Single Copy", "Artist", "Charter", "Album"}}});
    const std::vector<ChartLibraryEntry> scanned = scan_folder(root);
    REQUIRE(scanned.size() == 3);
    RecordStore store(":memory:");
    store.rebuild_chart_library(scanned);
    REQUIRE(listed_copies(store) == std::map<std::string, int>{{twice, 2}, {once, 1}});

    store.save(prepare_record(twice, at4(), charts[0].result.record));
    store.save(prepare_record(once, at4(), charts[1].result.record));
    std::map<std::string, std::string> names;
    for (const RecordRow& record : store.ready_records(at4()))
        names[record.song_id] = names_of(store, record.song_id).title;
    CHECK(names == std::map<std::string, std::string>{{twice, first_listed(scanned, twice).title},
                                                      {once, first_listed(scanned, once).title}});
    testtemp::remove_dir(root);
}

TEST_CASE("the scan cache is dropped when its reader stamp is not current") {
    // D51 call 12 (finding 345): the rescan cache carries kChartMetaStamp. A
    // file without a current stamp hands back no cache, so the next scan
    // reads every chart again and stamps the table it writes.
    const std::string path = testtemp::temp_path("chart_meta_stamp", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry entry = corpus_rows(1).front();
    {
        RecordStore store(path);
        store.rebuild_chart_library({entry});
    }
    {
        RecordStore store(path);
        CHECK(store.chart_library_cache().count(entry.notespath) == 1);
    }
    drop_chart_meta_stamp(path);
    {
        RecordStore store(path);
        CHECK(store.chart_library_cache().empty());
        store.rebuild_chart_library({entry});
        CHECK(store.chart_library_cache().count(entry.notespath) == 1);
    }
    // A file an older build's scan stamped: its rows may not hold what this
    // build reads, so its cache is dropped too.
    exec_on_file(path, set_chart_meta_stamp_sql(stale_chart_meta_stamp()).c_str());
    {
        RecordStore store(path);
        CHECK(store.chart_library_cache().empty());
    }
    std::remove(path.c_str());
}

TEST_CASE("the library is out of date only when it has rows and its reader stamp is not current") {
    const std::string path = testtemp::temp_path("library_out_of_date", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry entry = corpus_rows(1).front();
    {
        RecordStore store(path);
        CHECK_FALSE(store.chart_library_out_of_date());
        store.rebuild_chart_library({entry});
        CHECK_FALSE(store.chart_library_out_of_date());
    }
    exec_on_file(path, set_chart_meta_stamp_sql(stale_chart_meta_stamp()).c_str());
    {
        RecordStore store(path);
        CHECK(store.chart_library_out_of_date());
    }
    drop_chart_meta_stamp(path);
    {
        RecordStore store(path);
        CHECK(store.chart_library_out_of_date());
        store.rebuild_chart_library({entry});
        CHECK_FALSE(store.chart_library_out_of_date());
    }
    std::remove(path.c_str());
}

TEST_CASE("a library from before the album column is out of date after the open adds it") {
    // The charts table and scan stamp a build from before the album wrote:
    // no album column, and a stamp this build does not read as current.
    const std::string path = testtemp::temp_path("library_out_of_date_album", ".db");
    std::remove(path.c_str());
    {
        RecordStore seed(path);
        seed.rebuild_chart_library({corpus_rows(1).front()});
    }
    const std::string before_album =
        "ALTER TABLE charts DROP COLUMN album;" + set_chart_meta_stamp_sql(stale_chart_meta_stamp());
    exec_on_file(path, before_album.c_str());
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

// ---- the chart list read on the store's reader (StoreReader) ----------------

namespace {

// Checks two reads of the chart list gave the same rows in the same order,
// field for field.
void check_same_list(const std::vector<ChartLibraryEntry>& got,
                     const std::vector<ChartLibraryEntry>& want) {
    REQUIRE(got.size() == want.size());
    CHECK(md5s_of(got) == md5s_of(want));
    for (size_t i = 0; i < got.size(); ++i) {
        CAPTURE(want[i].md5);
        CHECK(got[i].md5 == want[i].md5);
        check_same_entry(got[i], want[i]);
    }
}

// A library with a chart in two folders and one other chart, every row
// carrying timing, so a current stamp shows it on every row.
std::vector<ChartLibraryEntry> timed_library() {
    ChartLibraryEntry first = library_row("x", "Song");
    ChartLibraryEntry second = library_row("x", "Song Copy");
    second.notespath = "C:\\charts\\copy\\notes.chart";
    ChartLibraryEntry other = library_row("y", "Other");
    std::vector<ChartLibraryEntry> rows = {first, second, other};
    for (size_t i = 0; i < rows.size(); ++i)
        rows[i].timing = ChartTimingMeta{1000.0 * static_cast<double>(i + 1), 0.0};
    return rows;
}

}  // namespace

TEST_CASE("the reader lists a chart's copies as the writer does") {
    StoreFile file("reader_copies");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library());

    const std::vector<ChartLibraryEntry> want = store.list_chart_library_copies("x");
    REQUIRE(want.size() == 2);
    check_same_list(store.reader().list_chart_library_copies("x"), want);
}

TEST_CASE("the reader lists the library by name as the writer does") {
    StoreFile file("reader_by_name");
    {
        RecordStore store(file.path);
        store.rebuild_chart_library(timed_library());
        const std::vector<ChartLibraryEntry> want = store.list_chart_library(0, -1);
        // A current stamp: the writer's rows carry their timing.
        REQUIRE(want.size() == 3);
        REQUIRE(want[0].timing.has_value());
        check_same_list(store.reader().list_chart_library(0, -1), want);
        // The whole list unsorted reads on the reader's one connection, and
        // gives the same rows.
        const std::vector<ChartLibraryEntry> unsorted =
            store.reader().list_chart_library(0, -1, {}, LibraryOrder::Unsorted);
        CHECK(md5_set_of(unsorted) == md5_set_of(want));
    }
    // A stamp that is not current (chart_meta_current): the writer's rows
    // carry no timing, and neither do the reader's.
    exec_on_file(file.path, "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version'");
    {
        RecordStore store(file.path);
        const std::vector<ChartLibraryEntry> want = store.list_chart_library(0, -1);
        REQUIRE(want.size() == 3);
        REQUIRE_FALSE(want[0].timing.has_value());
        check_same_list(store.reader().list_chart_library(0, -1), want);
        check_same_list(store.reader().list_chart_library_copies("x"),
                        store.list_chart_library_copies("x"));
    }
}

TEST_CASE("a reader's chart list does not wait while a save group is open") {
    StoreFile file("reader_list_no_wait");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library());
    const std::vector<ChartLibraryEntry> want_list = store.list_chart_library(0, -1);
    const std::vector<ChartLibraryEntry> want_copies = store.list_chart_library_copies("x");

    HeldSaveGroup group(store);
    const std::vector<ChartLibraryEntry> list = store.reader().list_chart_library(0, -1);
    const std::vector<ChartLibraryEntry> copies = store.reader().list_chart_library_copies("x");
    const bool returned_while_held = group.held();
    group.let_go();
    CHECK(returned_while_held);
    check_same_list(list, want_list);
    check_same_list(copies, want_copies);
}

TEST_CASE("a reader sees a rescan once it is saved") {
    StoreFile file("reader_rescan");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library());
    const std::vector<ChartLibraryEntry> before = store.reader().list_chart_library(0, -1);

    store.rebuild_chart_library({library_row("z", "New Song"), library_row("y", "Other")});
    const std::vector<ChartLibraryEntry> want = store.list_chart_library(0, -1);
    REQUIRE(md5s_of(want) != md5s_of(before));
    check_same_list(store.reader().list_chart_library(0, -1), want);
    check_same_list(store.reader().list_chart_library_copies("z"),
                    store.list_chart_library_copies("z"));
}

// ---- the chart index read on the store's reader (StoreReader) ---------------

namespace {

// Checks two chart indexes list the same song ids in the same order, with the
// same copies and the same naming row for each.
void check_same_index(const ChartIndex& got, const ChartIndex& want) {
    CHECK(got.song_ids() == want.song_ids());
    for (const std::string& id : want.song_ids()) {
        CAPTURE(id);
        CHECK(got.lists(id));
        CHECK(got.copies(id) == want.copies(id));
        const ChartLibraryEntry* got_name = got.naming(id);
        const ChartLibraryEntry* want_name = want.naming(id);
        REQUIRE((got_name != nullptr) == (want_name != nullptr));
        if (!want_name) continue;
        CHECK(got_name->md5 == want_name->md5);
        check_same_entry(*got_name, *want_name);
    }
}

}  // namespace

TEST_CASE("the reader builds the chart index the writer builds") {
    StoreFile file("reader_chart_index");
    {
        RecordStore store(file.path);
        store.rebuild_chart_library(timed_library());
        const ChartIndex want = store.chart_index();
        // A current stamp: the chart in two folders is listed once, with two
        // copies, and its naming row carries timing.
        REQUIRE(want.song_ids().size() == 2);
        REQUIRE(want.copies("x") == 2);
        REQUIRE(want.naming("x") != nullptr);
        REQUIRE(want.naming("x")->timing.has_value());
        check_same_index(store.reader().chart_index(), want);
    }
    // A stamp that is not current (chart_meta_current): the writer's naming
    // rows carry no timing, and neither do the reader's.
    exec_on_file(file.path, "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version'");
    {
        RecordStore store(file.path);
        const ChartIndex want = store.chart_index();
        REQUIRE(want.naming("x") != nullptr);
        REQUIRE_FALSE(want.naming("x")->timing.has_value());
        check_same_index(store.reader().chart_index(), want);
    }
}

TEST_CASE("a reader's chart index does not wait while a save group is open") {
    StoreFile file("reader_index_no_wait");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library());
    const ChartIndex want = store.chart_index();
    const int64_t want_count = store.chart_library_count();

    HeldSaveGroup group(store);
    const ChartIndex got = store.reader().chart_index();
    const int64_t got_count = store.reader().chart_library_count();
    const bool returned_while_held = group.held();
    group.let_go();
    CHECK(returned_while_held);
    check_same_index(got, want);
    CHECK(got_count == want_count);
}

// ---- the chart list read on a reader of the caller's own (OwnReader) --------

TEST_CASE("an own reader lists and indexes the library as the writer does") {
    StoreFile file("own_reader_library");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library());
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();

    const std::vector<ChartLibraryEntry> want_list = store.list_chart_library(0, -1);
    const std::vector<ChartLibraryEntry> want_copies = store.list_chart_library_copies("x");
    const ChartIndex want_index = store.chart_index();
    REQUIRE(want_copies.size() == 2);
    check_same_list(reader->reads().list_chart_library(0, -1), want_list);
    check_same_list(reader->reads().list_chart_library_copies("x"), want_copies);
    check_same_index(reader->reads().chart_index(), want_index);
    CHECK(reader->reads().chart_library_count() == store.chart_library_count());

    // Under a hold the same reads give the same rows.
    const OwnReader::Hold hold = reader->hold();
    check_same_list(reader->reads().list_chart_library(0, -1), want_list);
    check_same_list(reader->reads().list_chart_library_copies("x"), want_copies);
    check_same_index(reader->reads().chart_index(), want_index);
}
