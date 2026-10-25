// Check 4's own part, apart from the scan rows' E items (check_rows.h), so a
// test can run it on rows it builds by hand.
#pragma once

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "check_rows.h"
#include "checks.h"

namespace precheck {

// What check 4 needs from the scan rows. run_check_4 fills it from
// check_rows.h; tests fill it by hand.
struct Check4Rows {
    // The file and line of every kind-E row hit on an added line. Check 4's
    // own spellings skip these, so a line prints once.
    std::set<std::pair<std::string, int>> e_hit_lines;
    // The rule rows the range added (added_rows).
    std::vector<AddedRow> added_rows;
};

// Check 4 without the row items: the spellings and the new-row checks.
void run_check_4_spellings(Context& ctx, const Check4Rows& rows);

}  // namespace precheck
