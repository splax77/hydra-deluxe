#include "store/chart_index.h"

#include <unordered_set>

namespace hydra::store {

ChartIndex ChartIndex::from_rows(const std::vector<ChartLibraryEntry>& rows_in_scan_order) {
    ChartIndex index;
    for (const ChartLibraryEntry& row : rows_in_scan_order) {
        const auto [it, first_row] = index.charts_.try_emplace(row.md5);
        if (first_row) {
            it->second.naming = row;
            index.song_ids_.push_back(row.md5);
        }
        ++it->second.copies;
    }
    return index;
}

bool ChartIndex::lists(const std::string& song_id) const {
    return charts_.count(song_id) != 0;
}

int ChartIndex::copies(const std::string& song_id) const {
    const auto it = charts_.find(song_id);
    return it == charts_.end() ? 1 : it->second.copies;
}

const ChartLibraryEntry* ChartIndex::naming(const std::string& song_id) const {
    const auto it = charts_.find(song_id);
    return it == charts_.end() ? nullptr : &it->second.naming;
}

int64_t ChartIndex::count(const std::vector<std::string>& song_ids) const {
    std::unordered_set<std::string> seen;
    int64_t total = 0;
    for (const std::string& id : song_ids)
        if (seen.insert(id).second) total += copies(id);
    return total;
}

}  // namespace hydra::store
