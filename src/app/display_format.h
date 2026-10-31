// The display text for numbers and labels that more than one screen shows.
// Each rule lives here once, and every screen reads it from here.

#ifndef HYDRA_APP_DISPLAY_FORMAT_H
#define HYDRA_APP_DISPLAY_FORMAT_H

#include <algorithm>
#include <cstdint>
#include <string>

namespace hydra::app {

// The separator between two facts on one line of text: " · " (U+00B7 in UTF-8
// with a space each side). Every screen joins facts with this one spelling; a
// line that spells the dot another way says why beside it.
inline constexpr char kFactSeparator[] = " \xC2\xB7 ";

// round(v, 3), matching Python's correctly-rounded decimal rounding.
double py_round3(double v);

// The average multiplier as every screen shows it: py_round3, three places.
std::string format_avg_mult(double v);

// The average multiplier's name, wherever a screen labels it (D115).
inline constexpr char kAverageMultiplierLabel[] = "Average Multiplier";

// A timing in ms is written by format_ms in core/model.h, which the copied
// path shares; a whole-ms limit by format_ms_whole beside it.

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

// A points figure in the few characters under a fill on the Path Builder's
// timeline, the approved mockup's `short` rule
// (docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html):
// under 1,000 it is written whole ("640"), from 1,000 as thousands to one
// place with a "k" ("1.2k", "13.6k"). The tenth rounds to the nearest, a half
// away from zero.
std::string format_points_short(int64_t points);

// A song time as the Preview's clock shows it, "m:ss.mmm" ("1:04.000"). The
// time rounds to the nearest whole ms first (a half rounds away from zero),
// then splits into minutes and seconds, so 59,999.6 ms reads "1:00.000".
std::string clock_str(double ms);

// A duration as "0:42", "12:03" or "1:02:05". Negative reads as 0:00.
std::string format_duration(double seconds);

// How long a job has left, as the batch strip and the Preview loader say it:
// "about 1:30 left". The time itself is format_duration's.
inline std::string time_left_text(double seconds) {
    return "about " + format_duration(seconds) + " left";
}

// How full a progress bar is with `done` of `total` finished: held between 0
// and 1, and empty at a total of 0, which means nothing has been reported yet
// (D48, Q19). Every progress bar reads its fill from here.
inline float progress_fraction(double done, double total) {
    if (!(total > 0.0)) return 0.0f;
    return static_cast<float>(std::clamp(done / total, 0.0, 1.0));
}

}  // namespace hydra::app

#endif  // HYDRA_APP_DISPLAY_FORMAT_H
