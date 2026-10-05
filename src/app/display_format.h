// The display text for numbers that more than one screen shows. Each rule
// lives here once, so Song Details and the report cannot drift apart.

#ifndef HYDRA_APP_DISPLAY_FORMAT_H
#define HYDRA_APP_DISPLAY_FORMAT_H

#include <cstdint>
#include <string>

namespace hydra::app {

// round(v, 3), matching Python's correctly-rounded decimal rounding.
double py_round3(double v);

// The average multiplier as every screen shows it: py_round3, three places.
std::string format_avg_mult(double v);

// A timing in ms, one decimal, with the unit: "12.3ms". The caller decides
// the sign; the early fill passes Activation::e_difficulty(true), which
// is positive when the fill is hit early.
std::string format_ms(double ms);

// A timing in ms for a sentence or a label: one decimal, a space, the unit
// ("163.0 ms"). The Paths and Preview text and the path report's timing
// columns use this; format_ms keeps the older "163.0ms" form of the Paths
// panel's early-fill line.
std::string format_ms_spaced(double ms);

// Part over total as a percentage with the sign, rounded to the nearest at
// `decimals` places: 12 of 453 at 0 reads "3%", 198,010 of 198,020 at 2 reads
// "99.99%". An exact half rounds away from zero. The total must be above 0;
// a screen with nothing to count says so itself.
std::string format_percent(int64_t part, int64_t total, int decimals);

// The number format_percent writes, in whole steps of 10^-decimals percent:
// 198,010 of 200,000 at 2 is 9901 (99.01%). A page that has to work a percent
// out itself, like the leaderboard page's average tile, is sent these whole
// numbers so it rounds nothing in floating point.
int64_t percent_steps(int64_t part, int64_t total, int decimals);

// A song time as the Preview's clock shows it, "m:ss.mmm" ("1:04.000"). The
// time rounds to the nearest whole ms first (a half rounds away from zero),
// then splits into minutes and seconds, so 59,999.6 ms reads "1:00.000".
std::string clock_str(double ms);

}  // namespace hydra::app

#endif  // HYDRA_APP_DISPLAY_FORMAT_H
