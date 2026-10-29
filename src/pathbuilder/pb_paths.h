// The Path Builder's left list: the paths the player can pick and copy. Hydra's
// best path, the near-best paths its search kept, the best all-0 path, and the
// player's saved branches, each with its total and its gap from the best.

#ifndef HYDRA_PATHBUILDER_PB_PATHS_H
#define HYDRA_PATHBUILDER_PB_PATHS_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core/model.h"

namespace hydra::pathbuilder {

// One row of the list.
struct PathRow {
    std::string label;  // "Best path", "Path 2", "Best all-0 path", a branch's name
    const Path* path = nullptr;  // into the record or the branches; never null
    int64_t total = 0;           // Path::totalscore
    std::string text;            // Path::pathstring
    // HydraRecord::optimal_score minus this row's total: 0 or more for a
    // record path, since none outscores the optimal.
    int64_t gap = 0;
};

// The rows for `record`, in the order the list draws them: every root path of
// the record (HydraRecord::paths, the best first), then every root all-0 path
// (HydraRecord::allzero_paths), then `branches`. Empty when the record has no
// paths. The pointers hold while the record and branches stay as they are.
std::vector<PathRow> path_rows(const HydraRecord& record, const std::vector<Path>& branches);

// A row's total and gap as the list writes them: "706,180" and "-12,340"
// (a gap of 0 reads "0"). Thousands are grouped by group_thousands.
std::string total_text(const PathRow& row);
std::string gap_text(const PathRow& row);

// What the list's own state holds between frames.
struct PathListUi {
    // The row picked last, an index into path_rows; none until the player picks.
    std::optional<size_t> picked;
};

// Draws the list into the current ImGui window. Returns the row whose "Edit a
// copy" button was pressed this frame, if any.
std::optional<size_t> draw_path_list(const std::vector<PathRow>& rows, PathListUi& ui);

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_PATHS_H
