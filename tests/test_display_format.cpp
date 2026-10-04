// The display owners in src/app/display_format: a percentage and the
// Preview's clock. Each number here is one D48 settled (phase 3 task O2).

#include "doctest.h"

#include "app/display_format.h"

using hydra::app::clock_str;
using hydra::app::format_percent;

TEST_CASE("format_percent: nearest at the asked decimals") {
    // 12 of 453 is 2.65%: the Dynamics tab used to cut it to 2%.
    CHECK(format_percent(12, 453, 0) == "3%");
    CHECK(format_percent(999, 1000, 0) == "100%");
    // A score 10 points short of optimal, as the leaderboard page shows it.
    CHECK(format_percent(198010, 198020, 2) == "99.99%");
    CHECK(format_percent(198020, 198020, 2) == "100.00%");
}

TEST_CASE("clock_str: the total rounds to whole ms before it splits") {
    // Half a millisecond before the minute used to read 0:60.000.
    CHECK(clock_str(59999.6) == "1:00.000");
    // The values "build_time_box: timestamp, measure, tempo" pins through the
    // time box.
    CHECK(clock_str(1300.0) == "0:01.300");
    CHECK(clock_str(64000.0) == "1:04.000");
    CHECK(clock_str(0.0) == "0:00.000");
}
