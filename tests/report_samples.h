// The sample report rows the tests share: the six path rows and four
// leaderboard scores the samples page shows. tests/test_report.cpp pins the
// tiles on them, and the GUI tests (tests/ui/uitest_report_windows.cpp) open
// both report windows on them. Linked into hydra_tests and
// hydra_uitest_harness, so there is one copy. every_row, the shown list a
// tile test passes for a window with no filter, lives here too.

#ifndef HYDRA_TESTS_REPORT_SAMPLES_H
#define HYDRA_TESTS_REPORT_SAMPLES_H

#include <cstddef>
#include <vector>

#include "app/dm_report.h"
#include "app/report.h"

namespace report_samples {

// The hit window the sample path rows are labeled at.
inline constexpr double kSampleHitWindowMs = 85.0;

// The six path rows, labeled at kSampleHitWindowMs.
std::vector<hydra::app::report::ReportRow> sample_path_rows();

// The four scores.
std::vector<hydra::app::dm_report::DmReportRow> sample_dm_rows();

// Every row of `n`, in order: what a window shows with no filter set.
inline std::vector<size_t> every_row(size_t n) {
    std::vector<size_t> shown(n);
    for (size_t i = 0; i < n; ++i) shown[i] = i;
    return shown;
}

}  // namespace report_samples

#endif  // HYDRA_TESTS_REPORT_SAMPLES_H
