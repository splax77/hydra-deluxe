// The Library's shown order does not depend on the order its rows were read
// in. ui::read_library reads the chart list unsorted (LibraryOrder::Unsorted),
// and LibraryModel::set_charts sorts the rows itself; this case proves the
// table shows the same rows in the same order either way, on the corpus
// (testdata/input).

#include "doctest.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "app/analysis.h"  // discover_charts
#include "store/record_store.h"
#include "testdata_dirs.h"  // corpus::root
#include "ui/library_model.h"

using hydra::store::ChartLibraryEntry;
using hydra::store::LibraryOrder;
using hydra::store::RecordStore;
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
