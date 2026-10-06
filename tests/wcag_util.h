// WCAG 2.2 contrast, the one formula the colour tests measure with. Each test
// converts its own colour type (an ImGui ImVec4, a CSS "#rrggbb") to three
// 0-to-1 channels and asks here. The linear-light threshold is 2.2's 0.04045
// (D54, audit finding 288).

#ifndef HYDRA_TESTS_WCAG_UTIL_H
#define HYDRA_TESTS_WCAG_UTIL_H

#include <algorithm>
#include <cmath>

namespace testwcag {

// Relative luminance of an sRGB colour whose channels run from 0 to 1.
inline double relative_luminance(double r, double g, double b) {
    auto linear = [](double c) {
        return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * linear(r) + 0.7152 * linear(g) + 0.0722 * linear(b);
}

// Contrast ratio of two relative luminances, lighter over darker:
// (L1 + 0.05) / (L2 + 0.05).
inline double contrast_ratio(double la, double lb) {
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

}  // namespace testwcag

#endif  // HYDRA_TESTS_WCAG_UTIL_H
