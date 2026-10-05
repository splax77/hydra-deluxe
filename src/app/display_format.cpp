#include "app/display_format.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace hydra::app {

double py_round3(double v) {
    // Python's round(x, 3) rounds the exact binary value to 3 decimal places,
    // ties-to-even. MSVC's printf does the same correctly-rounded conversion,
    // so format-and-reparse reproduces it.
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.3f", v);
    return std::strtod(buf, nullptr);
}

std::string format_avg_mult(double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.3f", py_round3(v));
    return buf;
}

std::string format_ms(double ms) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.1fms", ms);
    return buf;
}

std::string format_ms_spaced(double ms) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.1f ms", ms);
    return buf;
}

namespace {

int64_t ten_to(int decimals) {
    int64_t scale = 1;
    for (int i = 0; i < decimals; ++i) scale *= 10;
    return scale;
}

}  // namespace

int64_t percent_steps(int64_t part, int64_t total, int decimals) {
    // Count in steps of 10^-decimals percent, in whole numbers, so the
    // rounding is exact: part * 100 * 10^decimals / total, to the nearest.
    const int64_t num = part * 100 * ten_to(decimals);
    const bool negative = num < 0;
    const int64_t mag = negative ? -num : num;
    const int64_t steps = (2 * mag + total) / (2 * total);  // a half rounds up
    return negative ? -steps : steps;
}

std::string format_percent(int64_t part, int64_t total, int decimals) {
    const int64_t steps = percent_steps(part, total, decimals);
    const int64_t scale = ten_to(decimals);
    const int64_t mag = steps < 0 ? -steps : steps;
    const int64_t whole = mag / scale;
    const int64_t frac = mag % scale;
    char buf[64];
    if (decimals > 0)
        std::snprintf(buf, sizeof(buf), "%s%lld.%0*lld%%", steps < 0 ? "-" : "",
                      static_cast<long long>(whole), decimals, static_cast<long long>(frac));
    else
        std::snprintf(buf, sizeof(buf), "%s%lld%%", steps < 0 ? "-" : "",
                      static_cast<long long>(whole));
    return buf;
}

std::string clock_str(double ms) {
    const long long total = std::llround(ms);
    const long long mag = total < 0 ? -total : total;
    char buf[48];
    std::snprintf(buf, sizeof(buf), "%s%lld:%02lld.%03lld", total < 0 ? "-" : "", mag / 60000,
                  (mag / 1000) % 60, mag % 1000);
    return buf;
}

}  // namespace hydra::app
