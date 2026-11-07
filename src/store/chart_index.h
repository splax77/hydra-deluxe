// The library's charts in one index: which charts the library lists, how
// many copies of each the scan found, and which copy names each chart. It is
// built once from the charts table in scan order (RecordStore::chart_index)
// and then answers every page's question about the library without going
// back to the database.

#ifndef HYDRA_STORE_CHART_INDEX_H
#define HYDRA_STORE_CHART_INDEX_H

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "store/record_store.h"

namespace hydra::store {

// The one owner of "which charts does the library list, how many copies does
// each have, and which copy names it" (D51 call 10, D77 item 2). from_rows
// is where those answers are worked out; the other members only read them.
class ChartIndex {
public:
    // Builds the index from the library rows, in the order the scan listed
    // them. The first row of a chart is its naming copy.
    static ChartIndex from_rows(const std::vector<ChartLibraryEntry>& rows_in_scan_order);

    // Does the library list this chart?
    bool lists(const std::string& song_id) const;
    // How many library rows the chart counts as. A chart the library does
    // not list still counts once: a page only shows charts it holds a
    // result for (D77 item 2).
    int copies(const std::string& song_id) const;
    // The library row that names the chart: its title, artist, charter and
    // notes path. Null when the library does not list the chart.
    const ChartLibraryEntry* naming(const std::string& song_id) const;
    // How many charts a set of ids counts as, over this index's copies
    // (count_charts).
    int64_t count(const std::vector<std::string>& song_ids) const;
    // Every chart the library lists, once each, in scan order.
    const std::vector<std::string>& song_ids() const { return song_ids_; }

private:
    struct Chart {
        ChartLibraryEntry naming;
        int copies = 0;
    };
    std::unordered_map<std::string, Chart> charts_;
    std::vector<std::string> song_ids_;
};

// How many charts a set of ids counts as, given each chart's copies: each
// distinct id's copies, added once however often the id appears. Zero for an
// empty set. The rule's one home: ChartIndex::count asks it over the index,
// and the path report's tiles ask it over the copies its rows kept once the
// index was dropped (D282).
int64_t count_charts(const std::vector<std::string>& song_ids,
                     const std::function<int(const std::string&)>& copies);

}  // namespace hydra::store

#endif  // HYDRA_STORE_CHART_INDEX_H
