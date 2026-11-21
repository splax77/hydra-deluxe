// every_row, the shown list a tile or view test passes for a window with no
// filter set. tests/test_report_page.cpp, tests/test_report_rows.cpp and
// tests/test_report_view.cpp share it.

#ifndef HYDRA_TESTS_REPORT_SAMPLES_H
#define HYDRA_TESTS_REPORT_SAMPLES_H

#include <cstddef>
#include <vector>

namespace report_samples {

// Every row of `n`, in order: what a window shows with no filter set.
inline std::vector<size_t> every_row(size_t n) {
    std::vector<size_t> shown(n);
    for (size_t i = 0; i < n; ++i) shown[i] = i;
    return shown;
}

}  // namespace report_samples

#endif  // HYDRA_TESTS_REPORT_SAMPLES_H
