// Tests for store/chart_index.{h,cpp} and RecordStore::chart_index: which
// charts the library lists, how many copies each has and which copy names
// it, in scan order. Also the record types beside it in store/record_store.h:
// RecordRow::summary and the record comparison.

#include "doctest.h"

#include <string>
#include <vector>

#include "library_fixtures.h"  // library_row
#include "store/chart_index.h"
#include "store/record_store.h"

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
    const ChartIndex index =
        ChartIndex::from_rows({a, copy_in_other_folder(a), library_row("bb", "B")});

    // aa (2 copies) + bb (1) + zz (unlisted, 1), with aa and bb repeated.
    CHECK(index.count({"aa", "bb", "aa", "zz", "bb"}) == 4);
    CHECK(index.count({}) == 0);
}

TEST_CASE("chart index: the store lists the library in scan order, not by name") {
    RecordStore store(":memory:");
    const ChartLibraryEntry charlie = library_row("cc", "Charlie");
    store.rebuild_chart_library(
        {charlie, library_row("aa", "Alpha"), copy_in_other_folder(charlie), library_row("bb", "Bravo")});

    const ChartIndex index = store.chart_index();
    CHECK(index.song_ids() == std::vector<std::string>{"cc", "aa", "bb"});
    CHECK(index.copies("cc") == 2);
    const ChartLibraryEntry* naming = index.naming("cc");
    REQUIRE(naming != nullptr);
    CHECK(naming->title == "Charlie");
    CHECK(naming->notespath == charlie.notespath);
    // The name-ordered listing reads the same rows another way round.
    REQUIRE(store.list_chart_library(0, -1).size() == 4);
    CHECK(store.list_chart_library(0, -1).front().md5 == "aa");
}

TEST_CASE("chart index: an empty library lists nothing") {
    RecordStore store(":memory:");
    const ChartIndex index = store.chart_index();
    CHECK(index.song_ids().empty());
    CHECK(index.count({"aa"}) == 1);
}

namespace {

PathRow path_row(int rank, int64_t score) {
    PathRow p;
    p.rank = rank;
    p.path = "1-2-3";
    p.score = score;
    p.actcount = 3;
    p.maxskip = 1;
    p.hardest_ms = 12.5;
    p.hardest_efill = 40.0;
    p.avgmult = 3.875;
    p.sqin_count = 2;
    p.sqout_count = 1;
    p.optimal = rank == 1;
    return p;
}

RecordRow record_with_paths() {
    RecordRow r;
    r.song_id = "aa";
    r.pathcount = 7;
    r.stars = 6;
    r.notecount = 812;
    r.paths = {path_row(1, 250000), path_row(2, 249950)};
    return r;
}

}  // namespace

TEST_CASE("record row: the summary reads rank 1 and the record's own columns") {
    const PathSummary s = record_with_paths().summary();
    CHECK(s.has_scored_best_path());
    CHECK(s.score == 250000);
    CHECK(s.actcount == 3);
    CHECK(s.maxskip == 1);
    CHECK(s.hardest_ms == 12.5);
    CHECK(s.avgmult == 3.875);
    CHECK(s.notecount == 812);
    CHECK(s.sqin_count == 2);
    CHECK(s.sqout_count == 1);
    CHECK(s.pathcount == 7);
    CHECK(s.stars == 6);
}

TEST_CASE("record row: a record with no paths has an empty summary") {
    RecordRow r;
    r.song_id = "aa";
    CHECK_FALSE(r.has_scored_best_path());
    CHECK(r.summary() == PathSummary{});
    CHECK_FALSE(r.summary().has_scored_best_path());
}

TEST_CASE("record row: two records are equal only when every column and path is") {
    const RecordRow base = record_with_paths();
    CHECK(base == record_with_paths());

    RecordRow other = base;
    other.notecount = 811;
    CHECK(other != base);

    other = base;
    other.stars.reset();
    CHECK(other != base);

    other = base;
    other.paths[1].hardest_efill.reset();
    CHECK(other != base);

    other = base;
    other.paths[1].optimal = true;
    CHECK(other != base);

    other = base;
    other.paths.pop_back();
    CHECK(other != base);
}
