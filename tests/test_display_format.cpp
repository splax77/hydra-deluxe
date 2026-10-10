// The display owners in src/app/display_format (a percentage and the
// Preview's clock) and the number-formatting owners in src/core/model.h
// (thousands commas, counted nouns, whole milliseconds). D48 settled the
// rounding and plural rules; the percentage and clock cases came with phase 3
// task O2.

#include "doctest.h"

#include <string>

#include "app/display_format.h"
#include "core/model.h"

using hydra::app::clock_str;
using hydra::app::format_percent;
using hydra::counted;
using hydra::format_ms_whole;
using hydra::group_thousands;
using hydra::has_have;

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

TEST_CASE("group_thousands matches Python {:,}") {
    CHECK(group_thousands(0) == "0");
    CHECK(group_thousands(999) == "999");
    CHECK(group_thousands(1000) == "1,000");
    CHECK(group_thousands(228710) == "228,710");
    CHECK(group_thousands(1234567) == "1,234,567");
    CHECK(group_thousands(-1234567) == "-1,234,567");
}

// D48 Q12: one rule everywhere. Caps 1 and 1000 are the plan's examples.
TEST_CASE("counted: singular at 1, commas from 1,000") {
    CHECK(counted(1, "bar", "bars") == "1 bar");
    CHECK(counted(2, "bar", "bars") == "2 bars");
    CHECK(counted(1000, "bar", "bars") == "1,000 bars");
    CHECK(std::string(has_have(1)) == "has");
    CHECK(std::string(has_have(2)) == "have");
}

// D48 Q2: whole-ms text rounds to nearest. No exact half is pinned.
TEST_CASE("format_ms_whole: nearest whole millisecond with the unit") {
    CHECK(format_ms_whole(12.4) == "12 ms");
    CHECK(format_ms_whole(12.6) == "13 ms");
    CHECK(format_ms_whole(12.25) == "12 ms");
    CHECK(format_ms_whole(163.0) == "163 ms");
}
