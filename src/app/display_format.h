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

// Every one-decimal timing a screen shows: one decimal, a space, the unit
// ("163.5 ms", D57 item 1). The Paths tab reads it for the squeeze sentences,
// the "(eff. ...)" figures, the backend tooltip, the early-fill line and the
// path buttons; the path report reads it for its Hardest and Early fill
// columns. The caller decides the sign; the early fill passes
// Activation::e_difficulty(true), which is positive when the fill is hit
// early. A whole-ms timing ("163 ms") is format_ms_whole in core/model.h.
std::string format_ms(double ms);

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
