// The Library's shown order does not depend on the order its rows were read
// in. ui::read_library reads the chart list unsorted (LibraryOrder::Unsorted),
// and LibraryModel::set_charts sorts the rows itself; this case proves the
// table shows the same rows in the same order either way, on the corpus
// (testdata/input).

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "app/analysis.h"  // discover_charts
#include "store/record_store.h"
#include "corpus_util.h"      // corpus::analyzed
#include "store_fixtures.h"  // OldLibrary
#include "testdata_dirs.h"  // corpus::root
#include "ui/library_model.h"

using hydra::store::ChartLibraryEntry;
using hydra::store::LibraryOrder;
using hydra::store::RecordLookup;
using hydra::store::RecordStore;
using hydra::test::OldLibrary;
using hydra::ui::LibraryModel;
using hydra::ui::LibrarySort;

namespace {

// The rows' files in the order the table shows them.
std::vector<std::string> shown_paths(const LibraryModel& m) {
    std::vector<std::string> out;
    for (size_t i : m.order()) out.push_back(m.rows()[i].entry.notespath);
    return out;
}

}  // namespace

TEST_CASE("library: the shown order does not depend on the order the rows were read") {
    const auto [items, errors] = hydra::app::discover_charts({corpus::root()});
    REQUIRE(errors.empty());
    REQUIRE(items.size() > 1);
    RecordStore store(":memory:");
    store.rebuild_chart_library(items);

    // Three read orders: by name, unsorted (what read_library asks for) and
    // by name reversed.
    const std::vector<ChartLibraryEntry> by_name = store.list_chart_library(0, -1);
    const std::vector<ChartLibraryEntry> unsorted =
        store.list_chart_library(0, -1, {}, LibraryOrder::Unsorted);
    std::vector<ChartLibraryEntry> reversed = by_name;
    std::reverse(reversed.begin(), reversed.end());
    REQUIRE(unsorted.size() == by_name.size());

    // LibraryModel's sort (library_model.cpp, resort) ends its tie-breaks on
    // the file path, so the order is fixed only while no two rows share one.
    // Two rows with the same path, title and folder would keep the order they
    // were read in.
    std::set<std::string> paths;
    for (const ChartLibraryEntry& e : by_name) paths.insert(e.notespath);
    CHECK(paths.size() == by_name.size());

    LibraryModel named, plain, backwards;
    named.set_charts(by_name);
    plain.set_charts(unsorted);
    backwards.set_charts(reversed);

    // Real scores, so the Best path sort orders by score and not only by its
    // tie-break: three corpus charts (the ones testdata/store/README.md names)
    // analysed for real under the old library's settings and saved here. A
    // Stale lookup carries no record, so they are saved under the profile they
    // are read with and come back Ready.
    const hydra::store::Profile profile = OldLibrary::profile();
    const std::vector<std::string> ids = OldLibrary::song_ids();
    for (const std::string& id : ids) {
        const auto row = std::find_if(by_name.begin(), by_name.end(),
                                      [&](const ChartLibraryEntry& e) { return e.md5 == id; });
        REQUIRE_MESSAGE(row != by_name.end(), "no corpus row has the hash " << id);
        store.save(hydra::store::prepare_record(
            id, profile, corpus::analyzed(row->notespath, OldLibrary::settings())));
    }
    const std::vector<RecordLookup> lookups = store.lookup(ids, profile);
    REQUIRE(lookups.size() == ids.size());
    std::set<int64_t> scores;
    for (size_t i = 0; i < ids.size(); ++i) {
        CAPTURE(ids[i]);
        REQUIRE(lookups[i].status == hydra::store::RecordStatus::Ready);
        REQUIRE(lookups[i].record.summary().has_scored_best_path());
        scores.insert(*lookups[i].record.summary().score);
        // Each record's chart is a corpus row: a hash no row lists changes nothing.
        CHECK(named.set_summary_for(ids[i], lookups[i]) > 0);
        CHECK(plain.set_summary_for(ids[i], lookups[i]) > 0);
        CHECK(backwards.set_summary_for(ids[i], lookups[i]) > 0);
    }
    // Two or more scores that differ, or the score order is not exercised.
    REQUIRE(scores.size() > 1);
    for (LibrarySort column : {LibrarySort::Title, LibrarySort::Artist, LibrarySort::Album,
                               LibrarySort::Charter, LibrarySort::Folder, LibrarySort::BestPath}) {
        for (bool ascending : {true, false}) {
            CAPTURE(static_cast<int>(column));
            CAPTURE(ascending);
            named.set_sort(column, ascending);
            plain.set_sort(column, ascending);
            backwards.set_sort(column, ascending);
            const std::vector<std::string> want = shown_paths(named);
            REQUIRE(want.size() == by_name.size());
            CHECK(shown_paths(plain) == want);
            CHECK(shown_paths(backwards) == want);
        }
    }
}
