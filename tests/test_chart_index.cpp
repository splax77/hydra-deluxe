// Tests for store/chart_index.{h,cpp} and RecordStore::chart_index: which
// charts the library lists, how many copies each has and which copy names
// it, in scan order. Also the record types beside it in store/record_store.h:
// RecordRow::summary and the record comparison.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "app/analysis.h"  // discover_charts, hash_chart_file, save_scan_as_library
#include "core/winstr.h"   // join_folder, os_path, parent_folder
#include "parse/chart_files.h"  // find_song_ini
#include "store/chart_index.h"
#include "store/record_store.h"
#include "corpus_util.h"
#include "library_fixtures.h"  // library_row
#include "store_fixtures.h"    // fixture, fixture_chart, at4
#include "temp_util.h"

using namespace hydra::store;

namespace {

// The same chart (same md5) found again in another folder.
ChartLibraryEntry copy_in_other_folder(const ChartLibraryEntry& row) {
    ChartLibraryEntry e = row;
    e.title = row.title + " (copy)";
    e.notespath = "D:\\more charts\\" + row.md5 + "\\notes.chart";
    e.rootfolder = "D:\\more charts";
    return e;
}

}  // namespace

TEST_CASE("chart index: a chart in two folders has two copies and the first row names it") {
    const ChartLibraryEntry first = library_row("aa", "First Title", "First Artist", "First Charter");
    const ChartLibraryEntry second = copy_in_other_folder(first);
    const ChartIndex index = ChartIndex::from_rows({first, library_row("bb", "Other"), second});

    CHECK(index.lists("aa"));
    CHECK(index.copies("aa") == 2);
    CHECK(index.copies("bb") == 1);
    const ChartLibraryEntry* naming = index.naming("aa");
    REQUIRE(naming != nullptr);
    CHECK(naming->title == "First Title");
    CHECK(naming->artist == "First Artist");
    CHECK(naming->charter == "First Charter");
    CHECK(naming->notespath == first.notespath);
    CHECK(naming->rootfolder == first.rootfolder);
    CHECK(index.song_ids() == std::vector<std::string>{"aa", "bb"});
}

TEST_CASE("chart index: a chart the library does not list counts once and has no naming row") {
    const ChartIndex index = ChartIndex::from_rows({library_row("aa", "A")});

    CHECK_FALSE(index.lists("zz"));
    CHECK(index.copies("zz") == 1);
    CHECK(index.naming("zz") == nullptr);
}

TEST_CASE("chart index: count adds each id's copies once, however often it appears") {
    const ChartLibraryEntry a = library_row("aa", "A");
    const std::vector<ChartLibraryEntry> rows = {a, copy_in_other_folder(a), library_row("bb", "B")};
    const ChartIndex index = ChartIndex::from_rows(rows);
    auto rows_with = [&](const std::string& md5) {
        return static_cast<int>(std::count_if(rows.begin(), rows.end(),
                                              [&](const ChartLibraryEntry& e) { return e.md5 == md5; }));
    };
    const int aa_copies = rows_with("aa");
    const int bb_copies = rows_with("bb");
    REQUIRE(aa_copies == 2);  // the copy above is a second row for aa
    // zz is in no row; an unlisted chart's count is the index's own answer.
    REQUIRE_FALSE(index.lists("zz"));
    const int zz_copies = index.copies("zz");

    // Each id once, however often the list repeats it.
    CHECK(index.count({"aa", "bb", "aa", "zz", "bb"}) == aa_copies + bb_copies + zz_copies);
    CHECK(index.count({}) == 0);
}

// A real scan: the test corpus, plus a scratch folder holding a second copy of
// one corpus chart (its notes file and song.ini), so one chart is found in
// two folders. The expected order, copies and naming rows are read off the
// scan's own list, which the store must keep as it was found.
TEST_CASE("chart index: the store lists the library in scan order, not by name") {
    namespace fs = std::filesystem;
    std::string chart;
    std::string ini;
    for (const std::string& p : corpus::chart_paths()) {
        ini = hydra::find_song_ini(hydra::parent_folder(p));
        if (!ini.empty()) {
            chart = p;
            break;
        }
    }
    REQUIRE_FALSE(chart.empty());
    // Emptied first: a crashed run may have left its copy behind.
    const std::string copy_root = testtemp::temp_dir("chart_index_copy");
    testtemp::remove_dir(copy_root);
    const std::string copy_folder = hydra::join_folder(copy_root, "copy");
    fs::create_directories(hydra::os_path(copy_folder));
    fs::copy_file(hydra::os_path(chart), hydra::os_path(copy_folder) / hydra::os_path(chart).filename());
    fs::copy_file(hydra::os_path(ini), hydra::os_path(copy_folder) / hydra::os_path(ini).filename());

    auto [items, errors] = hydra::app::discover_charts({corpus::root(), copy_root});
    testtemp::remove_dir(copy_root);
    REQUIRE(errors.empty());
    const std::string copied_md5 = hydra::app::hash_chart_file(chart);
    REQUIRE_FALSE(copied_md5.empty());

    // What the scan found: each id once in first-seen order, its row count,
    // and its first row.
    std::vector<std::string> first_seen;
    std::map<std::string, int> rows_of;
    std::map<std::string, const ChartLibraryEntry*> first_row;
    for (const ChartLibraryEntry& e : items) {
        if (rows_of[e.md5]++ == 0) {
            first_seen.push_back(e.md5);
            first_row[e.md5] = &e;
        }
    }
    REQUIRE(rows_of[copied_md5] >= 2);  // the original and the scratch copy

    RecordStore store(":memory:");
    REQUIRE_FALSE(hydra::app::save_scan_as_library(store, items).has_value());
    const ChartIndex index = store.chart_index();
    CHECK(index.song_ids() == first_seen);
    for (const std::string& id : first_seen) {
        INFO("chart " << id);
        CHECK(index.copies(id) == rows_of[id]);
        const ChartLibraryEntry* naming = index.naming(id);
        REQUIRE(naming != nullptr);
        CHECK(naming->notespath == first_row[id]->notespath);
        CHECK(naming->title == first_row[id]->title);
    }

    // The stored rows, row by row, in scan order; the name-ordered listing
    // reads the same rows another way round.
    const std::vector<ChartLibraryEntry> stored =
        store.list_chart_library(0, -1, {}, LibraryOrder::Unsorted);
    REQUIRE(stored.size() == items.size());
    std::multiset<std::string> scanned;
    for (size_t i = 0; i < items.size(); ++i) {
        INFO("scan row " << i);
        CHECK(stored[i].notespath == items[i].notespath);
        scanned.insert(items[i].notespath);
    }
    std::multiset<std::string> listed_by_name;
    for (const ChartLibraryEntry& e : store.list_chart_library(0, -1)) listed_by_name.insert(e.notespath);
    CHECK(listed_by_name == scanned);
}

TEST_CASE("chart index: an empty library lists nothing") {
    RecordStore store(":memory:");
    const ChartIndex index = store.chart_index();
    CHECK(index.song_ids().empty());
    CHECK(index.count({"aa"}) == 1);
}

namespace {

// The record the store keeps for a real analysis: the store fixture's corpus
// chart analyzed at 4 bars, saved and read back.
RecordRow saved_record() {
    const ChartLibraryEntry entry = corpus::scanned_entry(hydra::test::fixture_chart());
    RecordStore store(":memory:");
    store.rebuild_chart_library({entry});
    store.save(prepare_record(entry.md5, hydra::test::at4(), hydra::test::fixture().record));
    const RecordLookup found = store.lookup_one(entry.md5, hydra::test::at4());
    REQUIRE(found.status == RecordStatus::Ready);
    REQUIRE_FALSE(found.record.paths.empty());
    return found.record;
}

// Changes `v` to some other value of its type.
void change(int& v) { v += 1; }
void change(int64_t& v) { v += 1; }
void change(double& v) { v += 1.0; }
void change(bool& v) { v = !v; }
void change(std::string& v) { v += "x"; }
template <typename T>
void change(std::optional<T>& v) {
    if (v) {
        v.reset();
    } else {
        v = T{};
    }
}

// One column of a record changed, named for a failure message.
struct ColumnChange {
    const char* column;
    std::function<void(RecordRow&)> apply;
};

// Every column of RecordRow and of its best PathRow, each changed alone.
std::vector<ColumnChange> every_column_change() {
    return {
        {"song_id", [](RecordRow& r) { change(r.song_id); }},
        {"pathcount", [](RecordRow& r) { change(r.pathcount); }},
        {"stars", [](RecordRow& r) { change(r.stars); }},
        {"notecount", [](RecordRow& r) { change(r.notecount); }},
        {"paths (one fewer)", [](RecordRow& r) { r.paths.pop_back(); }},
        {"failure_kind", [](RecordRow& r) { change(r.failure_kind); }},
        {"failure_text", [](RecordRow& r) { change(r.failure_text); }},
        {"path rank", [](RecordRow& r) { change(r.paths.front().rank); }},
        {"path path", [](RecordRow& r) { change(r.paths.front().path); }},
        {"path score", [](RecordRow& r) { change(r.paths.front().score); }},
        {"path actcount", [](RecordRow& r) { change(r.paths.front().actcount); }},
        {"path maxskip", [](RecordRow& r) { change(r.paths.front().maxskip); }},
        {"path hardest_ms", [](RecordRow& r) { change(r.paths.front().hardest_ms); }},
        {"path hardest_efill", [](RecordRow& r) { change(r.paths.front().hardest_efill); }},
        {"path avgmult", [](RecordRow& r) { change(r.paths.front().avgmult); }},
        {"path sqin_count", [](RecordRow& r) { change(r.paths.front().sqin_count); }},
        {"path sqout_count", [](RecordRow& r) { change(r.paths.front().sqout_count); }},
        {"path optimal", [](RecordRow& r) { change(r.paths.front().optimal); }},
    };
}

}  // namespace

// The summary follows the best path: with every column of the best path and
// of the record changed, each summary field still equals its column, so a
// summary read from any other path, or from the wrong column, fails.
TEST_CASE("record row: the summary reads rank 1 and the record's own columns") {
    RecordRow r = saved_record();
    auto check_follows = [](const RecordRow& rec) {
        const PathSummary s = rec.summary();
        const PathRow& best = rec.paths.front();
        CHECK(best.rank == 1);
        CHECK(s.has_scored_best_path());
        CHECK(s.score == best.score);
        CHECK(s.actcount == best.actcount);
        CHECK(s.maxskip == best.maxskip);
        CHECK(s.hardest_ms == best.hardest_ms);
        CHECK(s.avgmult == best.avgmult);
        CHECK(s.sqin_count == best.sqin_count);
        CHECK(s.sqout_count == best.sqout_count);
        CHECK(s.notecount == rec.notecount);
        CHECK(s.pathcount == rec.pathcount);
        CHECK(s.stars == rec.stars);
    };
    check_follows(r);
    PathRow& best = r.paths.front();
    change(best.score);
    change(best.actcount);
    change(best.maxskip);
    best.hardest_ms = best.hardest_ms.value_or(0.0) + 1.0;
    change(best.avgmult);
    change(best.sqin_count);
    change(best.sqout_count);
    change(r.notecount);
    change(r.pathcount);
    r.stars = r.stars.value_or(0) + 1;
    check_follows(r);
}

TEST_CASE("record row: a record with no paths has an empty summary") {
    RecordRow r;
    r.song_id = "aa";
    CHECK_FALSE(r.has_scored_best_path());
    CHECK(r.summary() == PathSummary{});
    CHECK_FALSE(r.summary().has_scored_best_path());
}

TEST_CASE("record row: two records are equal only when every column and path is") {
    const RecordRow base = saved_record();
    CHECK(base == saved_record());
    for (const ColumnChange& c : every_column_change()) {
        INFO("changed column: " << std::string(c.column));
        RecordRow other = base;
        c.apply(other);
        CHECK(other != base);
    }
}
