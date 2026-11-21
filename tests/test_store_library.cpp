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
std::vector<ChartLibraryEntry> different_corpus_rows(size_t n) {
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

}  // namespace

TEST_CASE("a library entry filled by position keeps its first seven fields in place") {
    // Tests and fixtures (test_cli.cpp, dm_fixture.h) fill the entry by
    // position with seven strings. A field added among them would shift the
    // path, root folder and fingerprint into the wrong slots with no compile
    // error, so a new field goes after these seven. An entry filled that way
    // from a real scan's row must work in the store as the row itself does:
    // the rescan cache finds it by its path with its fingerprint, and the
    // library names the chart by its title.
    const ChartLibraryEntry& scanned = scanned_corpus().front();
    const ChartLibraryEntry e{scanned.md5, scanned.title, scanned.artist, scanned.charter,
                              scanned.notespath, scanned.rootfolder, scanned.sig};
    RecordStore store(":memory:");
    store.rebuild_chart_library({e});
    const ChartLibraryCache cache = store.chart_library_cache();
    REQUIRE(cache.count(scanned.notespath) == 1);
    CHECK(cache.at(scanned.notespath).md5 == scanned.md5);
    CHECK(cache.at(scanned.notespath).sig == scanned.sig);
    CHECK(cache.at(scanned.notespath).rootfolder == scanned.rootfolder);
    CHECK(names_of(store, scanned.md5).title == scanned.title);
    CHECK(names_of(store, scanned.md5).artist == scanned.artist);
    CHECK(names_of(store, scanned.md5).charter == scanned.charter);
}

TEST_CASE("a failed library rebuild keeps the previous scan") {
    // The rebuild's transaction opens before anything changes, so an insert
    // that fails leaves the library and its rescan cache as they were. A
    // trigger that refuses one chart's md5 makes an insert fail partway. The
    // rows are the corpus scan's.
    const std::vector<ChartLibraryEntry> rows = different_corpus_rows(4);
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
    // library table. The index's name is the store's own (no header gives
    // it), so the check asks SQLite for any index on the library table whose
    // first column is md5, the column those lookups search.
    const std::string path = testtemp::temp_path("md5_index", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM pragma_index_list('charts') AS l,"
                               " pragma_index_info(l.name) AS i"
                               " WHERE i.seqno = 0 AND i.name = 'md5'") >= 1);
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

// ---- charts the library no longer lists (D87 items 3 and 4, task storage-T1) --

namespace {

// How many records one chart holds, read straight out of the file.
int64_t results_of(const std::string& path, const std::string& hash) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM records WHERE song_id = '" + hash + "'");
}

// The engine's record of the corpus chart `row` lists, at `cap` bars under
// `rules`, as corpus::analyzed reads it once per run.
const HydraRecord& analysis_of(const ChartLibraryEntry& row, int cap = 4,
                               const core::Rules& rules = core::default_rules()) {
    return corpus::analyzed(row.notespath,
                            hydra::test::store_fixture_settings(cap, std::nullopt, false, rules));
}

// The first `n` scanned_corpus rows whose charts the engine analyzes to at
// least one path at 4 bars: `n` different charts a click could save.
std::vector<ChartLibraryEntry> analyzable_corpus_rows(size_t n) {
    std::vector<ChartLibraryEntry> out;
    for (const ChartLibraryEntry& e : scanned_corpus()) {
        if (out.size() == n) break;
        try {
            if (!analysis_of(e).paths.empty()) out.push_back(e);
        } catch (const std::exception&) {
            // A chart with no notes for the fixture's chart mode saves nothing.
        }
    }
    REQUIRE(out.size() == n);
    return out;
}

// The chart `row` lists, analyzed by the engine and saved as the click saves
// it, under chart mode `mode`.
void save_chart(RecordStore& store, const ChartLibraryEntry& row,
                const std::string& mode = "mode") {
    store.save(prepare_record(row.md5, at4(mode), analysis_of(row)));
}

// The library rows the real scan writes for the fixture's chart copied into
// two song folders: one chart, two copies, in the scan's order.
std::vector<ChartLibraryEntry> two_copies(const std::string& tag) {
    const std::string root = write_song_folders(
        tag, {SongFolder{"copy_a", hydra::test::fixture_chart(),
                         ChartNames{"Song", "Artist", "Charter", "Album"}},
              SongFolder{"copy_b", hydra::test::fixture_chart(),
                         ChartNames{"Song Copy", "Artist", "Charter", "Album"}}});
    std::vector<ChartLibraryEntry> rows = scan_folder(root);
    testtemp::remove_dir(root);
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].md5 == rows[1].md5);
    REQUIRE(rows[0].notespath != rows[1].notespath);
    return rows;
}

// The fixture's chart, the one two_copies lists, saved as the click saves
// it: the engine's record of it at 4 bars.
void save_fixture_chart(RecordStore& store, const std::string& md5) {
    store.save(prepare_record(md5, at4(), hydra::test::fixture().record));
}

}  // namespace

// The rows are the corpus scan's and the records the engine's analyses of
// those charts.

TEST_CASE("a scan that drops a chart deletes its stored rows") {
    const std::vector<ChartLibraryEntry> rows = analyzable_corpus_rows(2);
    const ChartLibraryEntry& a = rows[0];
    const ChartLibraryEntry& b = rows[1];
    const std::string path = testtemp::temp_path("scan_drops_chart", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({a, b});
        save_chart(store, a);
        save_chart(store, b);
    }
    REQUIRE(results_of(path, a.md5) == 1);
    REQUIRE(results_of(path, b.md5) == 1);
    {
        RecordStore store(path);
        store.rebuild_chart_library({a});
    }
    CHECK(results_of(path, b.md5) == 0);
    CHECK(results_of(path, a.md5) == 1);
    std::remove(path.c_str());
}

TEST_CASE("a chart with two library copies keeps its rows when one copy leaves") {
    const std::vector<ChartLibraryEntry> copies = two_copies("scan_drops_copy_charts");
    const std::string id = copies[0].md5;
    const std::string path = testtemp::temp_path("scan_drops_copy", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library(copies);
        save_fixture_chart(store, id);
    }
    REQUIRE(results_of(path, id) == 1);
    {
        RecordStore store(path);
        store.rebuild_chart_library({copies[1]});
        CHECK(listed_copies(store) == std::map<std::string, int>{{id, 1}});
    }
    CHECK(results_of(path, id) == 1);
    std::remove(path.c_str());
}

TEST_CASE("a library chart keeps its rows for other chart modes, including Stale ones") {
    const std::vector<ChartLibraryEntry> rows = analyzable_corpus_rows(2);
    const ChartLibraryEntry& a = rows[0];
    const ChartLibraryEntry& b = rows[1];
    const std::string path = testtemp::temp_path("scan_keeps_modes", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({a, b});
        const Profile rules{"rules", CapQuery::at(8), Lens{}};
        const Profile build{"build", CapQuery::at(8), Lens{}};
        save_chart(store, a, "mode");
        save_chart(store, a, "other mode");
        save_chart(store, b);
        // The engine's analysis under rules other than the defaults, which
        // this store runs.
        store.save(prepare_record(a.md5, rules, analysis_of(a, 8, test::other_rules())));
        // A save purges the chart's records from other builds (RecordStore::save
        // says which), so the other build's record is the chart's last save.
        test::save_old_build_record(store, prepare_record(a.md5, build, analysis_of(a, 8)));
        REQUIRE(store.lookup_one(a.md5, rules).status == RecordStatus::Stale);
        REQUIRE(store.lookup_one(a.md5, rules).stale_rules);
        REQUIRE(store.lookup_one(a.md5, build).status == RecordStatus::Stale);
        REQUIRE(store.lookup_one(a.md5, build).stale_build);
    }
    REQUIRE(results_of(path, a.md5) == 4);
    {
        RecordStore store(path);
        store.rebuild_chart_library({a});
    }
    CHECK(results_of(path, a.md5) == 4);
    CHECK(results_of(path, b.md5) == 0);
    std::remove(path.c_str());
}

TEST_CASE("reidentify_chart moves the library row to the new md5 and deletes the old md5's rows") {
    // Chart a's file now holds chart c: reidentify_chart takes c's md5 and
    // fingerprint, as a click that finds the file changed passes them.
    const std::vector<ChartLibraryEntry> rows = analyzable_corpus_rows(3);
    const ChartLibraryEntry& a = rows[0];
    const ChartLibraryEntry& b = rows[1];
    const ChartLibraryEntry& c = rows[2];
    const std::string path = testtemp::temp_path("reidentify_moves", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({a, b});
        save_chart(store, a);
        save_chart(store, b);
    }
    REQUIRE(results_of(path, a.md5) == 1);
    REQUIRE(results_of(path, b.md5) == 1);
    {
        RecordStore store(path);
        store.reidentify_chart(a.notespath, c.md5, c.sig);
        CHECK(listed_copies(store) == std::map<std::string, int>{{c.md5, 1}, {b.md5, 1}});
        const ChartLibraryCache cache = store.chart_library_cache();
        REQUIRE(cache.count(a.notespath) == 1);
        CHECK(cache.at(a.notespath).md5 == c.md5);
        CHECK(cache.at(a.notespath).sig == c.sig);
    }
    CHECK(results_of(path, a.md5) == 0);
    CHECK(results_of(path, b.md5) == 1);
    std::remove(path.c_str());
}

TEST_CASE("reidentify_chart keeps the old md5's rows while another library row has it") {
    const std::vector<ChartLibraryEntry> copies = two_copies("reidentify_keeps_charts");
    const std::string id = copies[0].md5;
    const ChartLibraryEntry other = corpus_row_other_than(id);
    const std::string path = testtemp::temp_path("reidentify_keeps", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library(copies);
        save_fixture_chart(store, id);
    }
    REQUIRE(results_of(path, id) == 1);
    {
        RecordStore store(path);
        store.reidentify_chart(copies[1].notespath, other.md5, other.sig);
        CHECK(listed_copies(store) == std::map<std::string, int>{{id, 1}, {other.md5, 1}});
    }
    CHECK(results_of(path, id) == 1);
    std::remove(path.c_str());
}

TEST_CASE("a scan that fails partway through its deletes keeps the dropped chart's rows") {
    // The deletes run inside the scan's own transaction. A trigger that
    // refuses the records delete fails it after the library rows changed.
    const std::vector<ChartLibraryEntry> rows = analyzable_corpus_rows(2);
    const ChartLibraryEntry& a = rows[0];
    const ChartLibraryEntry& b = rows[1];
    const std::string path = testtemp::temp_path("scan_delete_fails", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({a, b});
        save_chart(store, a);
        save_chart(store, b);
    }
    REQUIRE(results_of(path, b.md5) == 1);
    exec_on_file(path,
                 "CREATE TRIGGER refuse_result_delete BEFORE DELETE ON records"
                 " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    {
        RecordStore store(path);
        CHECK_THROWS(store.rebuild_chart_library({a}));
        CHECK(store.chart_library_count() == 2);
    }
    CHECK(results_of(path, b.md5) == 1);
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
    // The rows are the corpus scan's.
    RecordStore store(":memory:");
    store.rebuild_chart_library(different_corpus_rows(4));
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

// The library is the corpus scan's rows for kThreadedRows different charts.
std::string threaded_library_file(const char* name) {
    const std::string path = testtemp::temp_path(name, ".db");
    std::remove(path.c_str());
    RecordStore store(path);
    store.rebuild_chart_library(different_corpus_rows(static_cast<size_t>(kThreadedRows)));
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
    // Four copies of one corpus chart and one other corpus chart, read by
    // the real scan from two charts folders. "Song" twice, the scan reaching
    // the later path first (its folder is listed first); "song" (lowercase
    // sorts after every uppercase letter byte for byte) and "Alpha".
    const std::string chart = hydra::test::fixture_chart();
    const std::string id = hydra::app::hash_chart_file(chart);
    const std::string other_chart = corpus_row_other_than(id).notespath;
    const std::string late_root = write_song_folders(
        "byte_order_z", {SongFolder{"z_song", chart, ChartNames{"Song", "A", "C", "Al"}}});
    const std::string early_root = write_song_folders(
        "byte_order_a",
        {SongFolder{"a_song", chart, ChartNames{"Song", "A", "C", "Al"}},
         SongFolder{"l_song", chart, ChartNames{"song", "A", "C", "Al"}},
         SongFolder{"q_alpha", chart, ChartNames{"Alpha", "A", "C", "Al"}},
         SongFolder{"o_other", other_chart, ChartNames{"Other", "A", "C", "Al"}}});
    REQUIRE(late_root > early_root);
    std::pair<std::vector<ChartLibraryEntry>, std::vector<std::string>> scan =
        hydra::app::discover_charts({late_root, early_root});
    REQUIRE(scan.second.empty());
    REQUIRE(scan.first.size() == 5);
    // The chart file the scan found in song folder `folder`.
    const auto path_in = [&](const std::string& folder) {
        for (const ChartLibraryEntry& e : scan.first) {
            const std::string dir = folder_of(e.notespath);
            if (dir.substr(dir.find_last_of("\\/") + 1) == folder) return e.notespath;
        }
        FAIL("the scan found no chart in " << folder);
        return std::string();
    };
    REQUIRE(scan.first.front().notespath == path_in("z_song"));
    RecordStore store(":memory:");
    store.rebuild_chart_library(scan.first);

    std::vector<std::string> copies;
    for (const ChartLibraryEntry& e : store.list_chart_library_copies(id))
        copies.push_back(e.notespath);
    CHECK(copies == std::vector<std::string>{path_in("q_alpha"), path_in("z_song"),
                                             path_in("a_song"), path_in("l_song")});
    std::vector<std::string> listed;
    for (const ChartLibraryEntry& e : store.list_chart_library(0, -1)) listed.push_back(e.notespath);
    CHECK(listed == std::vector<std::string>{path_in("q_alpha"), path_in("o_other"),
                                             path_in("z_song"), path_in("a_song"),
                                             path_in("l_song")});
    testtemp::remove_dir(late_root);
    testtemp::remove_dir(early_root);
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

// A library the real scan wrote: the fixture's chart in two song folders
// (two_copies) and one other corpus chart. Every row carries the timing the
// scan read, so a current stamp shows it on every row.
struct TimedLibrary {
    std::vector<ChartLibraryEntry> rows;
    std::string twice;  // the md5 of the chart in two folders
};
const TimedLibrary& timed_library() {
    static const TimedLibrary library = [] {
        TimedLibrary out;
        out.rows = two_copies("timed_library");
        out.twice = out.rows.front().md5;
        out.rows.push_back(corpus_row_other_than(out.twice));
        return out;
    }();
    return library;
}

}  // namespace

TEST_CASE("the reader lists a chart's copies as the writer does") {
    StoreFile file("reader_copies");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library().rows);

    const std::vector<ChartLibraryEntry> want = store.list_chart_library_copies(timed_library().twice);
    REQUIRE(want.size() == 2);
    check_same_list(store.reader().list_chart_library_copies(timed_library().twice), want);
}

TEST_CASE("the reader lists the library by name as the writer does") {
    StoreFile file("reader_by_name");
    {
        RecordStore store(file.path);
        store.rebuild_chart_library(timed_library().rows);
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
        check_same_list(store.reader().list_chart_library_copies(timed_library().twice),
                        store.list_chart_library_copies(timed_library().twice));
    }
}

TEST_CASE("a reader's chart list does not wait while a save group is open") {
    StoreFile file("reader_list_no_wait");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library().rows);
    const std::vector<ChartLibraryEntry> want_list = store.list_chart_library(0, -1);
    const std::vector<ChartLibraryEntry> want_copies = store.list_chart_library_copies(timed_library().twice);

    HeldSaveGroup group(store);
    const std::vector<ChartLibraryEntry> list = store.reader().list_chart_library(0, -1);
    const std::vector<ChartLibraryEntry> copies = store.reader().list_chart_library_copies(timed_library().twice);
    const bool returned_while_held = group.held();
    group.let_go();
    CHECK(returned_while_held);
    check_same_list(list, want_list);
    check_same_list(copies, want_copies);
}

TEST_CASE("a reader sees a rescan once it is saved") {
    StoreFile file("reader_rescan");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library().rows);
    const std::vector<ChartLibraryEntry> before = store.reader().list_chart_library(0, -1);

    // The rescan finds the chart in two folders gone and another corpus
    // chart added; the library's other chart stays.
    const ChartLibraryEntry& stays = timed_library().rows.back();
    ChartLibraryEntry added;
    for (const ChartLibraryEntry& e : scanned_corpus())
        if (e.md5 != timed_library().twice && e.md5 != stays.md5) {
            added = e;
            break;
        }
    REQUIRE_FALSE(added.md5.empty());
    store.rebuild_chart_library({added, stays});
    const std::vector<ChartLibraryEntry> want = store.list_chart_library(0, -1);
    REQUIRE(md5s_of(want) != md5s_of(before));
    check_same_list(store.reader().list_chart_library(0, -1), want);
    check_same_list(store.reader().list_chart_library_copies(added.md5),
                    store.list_chart_library_copies(added.md5));
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
        store.rebuild_chart_library(timed_library().rows);
        const ChartIndex want = store.chart_index();
        // A current stamp: the chart in two folders is listed once, with two
        // copies, and its naming row carries timing.
        REQUIRE(want.song_ids().size() == 2);
        REQUIRE(want.copies(timed_library().twice) == 2);
        REQUIRE(want.naming(timed_library().twice) != nullptr);
        REQUIRE(want.naming(timed_library().twice)->timing.has_value());
        check_same_index(store.reader().chart_index(), want);
    }
    // A stamp that is not current (chart_meta_current): the writer's naming
    // rows carry no timing, and neither do the reader's.
    exec_on_file(file.path, "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version'");
    {
        RecordStore store(file.path);
        const ChartIndex want = store.chart_index();
        REQUIRE(want.naming(timed_library().twice) != nullptr);
        REQUIRE_FALSE(want.naming(timed_library().twice)->timing.has_value());
        check_same_index(store.reader().chart_index(), want);
    }
}

TEST_CASE("a reader's chart index does not wait while a save group is open") {
    StoreFile file("reader_index_no_wait");
    RecordStore store(file.path);
    store.rebuild_chart_library(timed_library().rows);
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
    store.rebuild_chart_library(timed_library().rows);
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();

    const std::vector<ChartLibraryEntry> want_list = store.list_chart_library(0, -1);
    const std::vector<ChartLibraryEntry> want_copies = store.list_chart_library_copies(timed_library().twice);
    const ChartIndex want_index = store.chart_index();
    REQUIRE(want_copies.size() == 2);
    check_same_list(reader->reads().list_chart_library(0, -1), want_list);
    check_same_list(reader->reads().list_chart_library_copies(timed_library().twice), want_copies);
    check_same_index(reader->reads().chart_index(), want_index);
    CHECK(reader->reads().chart_library_count() == store.chart_library_count());

    // Under a hold the same reads give the same rows.
    const OwnReader::Hold hold = reader->hold();
    check_same_list(reader->reads().list_chart_library(0, -1), want_list);
    check_same_list(reader->reads().list_chart_library_copies(timed_library().twice), want_copies);
    check_same_index(reader->reads().chart_index(), want_index);
}
