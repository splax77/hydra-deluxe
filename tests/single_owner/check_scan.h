// How a single-owner scan's test case ends. The row scan (scan.cpp) and the
// clone scan (clones.cpp) both end this way; it is the test side of
// listed.h, kept apart so the engine builds without doctest.
#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "doctest.h"
#include "listed.h"

namespace single_owner {

// Each entry no source line took is stale, and the case fails with one line
// per problem.
inline void check_scan(std::vector<std::string> problems, const std::vector<ListedLine>& listed,
                       const std::vector<bool>& used) {
    for (size_t i = 0; i < listed.size(); ++i) {
        if (used[i]) continue;
        problems.push_back(stale_message(listed[i]));
    }
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}

}  // namespace single_owner
