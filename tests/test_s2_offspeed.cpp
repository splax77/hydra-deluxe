// Step 2, task 7: off-speed leaderboard scores get their own status (D25,
// finding 222). Clone Hero keeps a leaderboard per speed and Hydra's optimal
// is computed at base speed, so an off-speed score is shown but not compared.

#include "doctest.h"

#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/dm_report.h"
#include "core/model.h"
#include "corpus_util.h"
#include "dm_fixture.h"
#include "net/dmbot_client.h"
#include "store/record_store.h"

using namespace hydra;
using app::dm_report::DmReportRow;

// The store and score builders are the shared leaderboard fixture.
using testdm::fill_store;
using testdm::kHash;
using testdm::kMode;
const auto score_at = testdm::make_score;

TEST_CASE("s2 offspeed: one base speed, read everywhere") {
    CHECK(net::kBaseSpeedPercent == 100);
    CHECK(net::is_base_speed(100));
    CHECK_FALSE(net::is_base_speed(150));
    CHECK_FALSE(net::is_base_speed(75));
    CHECK(net::DmScore{}.speed == net::kBaseSpeedPercent);
    CHECK(DmReportRow{}.speed == net::kBaseSpeedPercent);
}

TEST_CASE("s2 offspeed: an off-speed score is 'other speed', with or without a result") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    const std::vector<net::DmScore> scores = {
        score_at(kHash, optimal + 5, 150),     // would read "above optimal"
        score_at(kHash, optimal - 1000, 75),   // would read "under optimal"
        score_at("00ff00ff00ff00ff00ff00ff00ff00ff", 123, 150),  // no result at all
        score_at(kHash, optimal - 1000, 100),  // base speed: under optimal
    };
    const std::vector<DmReportRow> rows =
        app::dm_report::collect_dm_rows(store, scores, kMode, store::Lens{});
    REQUIRE(rows.size() == 4);

    // These strings are load-bearing: the window's Status filter and chips key on them.
    CHECK(rows[0].status == "other speed");
    CHECK(rows[1].status == "other speed");
    CHECK(rows[2].status == "other speed");
    CHECK(rows[3].status == "under optimal");

    // The numbers still show; only the comparison is withheld.
    CHECK(rows[0].optimal == optimal);
    CHECK(rows[1].delta == 1000);
    CHECK_FALSE(rows[0].pct_h.has_value());
    CHECK_FALSE(rows[1].pct_h.has_value());
    REQUIRE(rows[3].pct_h.has_value());

    const app::dm_report::DmReportStats stats = app::dm_report::tally_dm_rows(rows);
    CHECK(stats.total == 4);
    CHECK(stats.under_optimal == 1);
    CHECK(stats.at_optimal == 0);
    CHECK(stats.above_optimal == 0);
    CHECK(stats.other_speed == 3);
    CHECK(stats.not_in_library == 0);
}

TEST_CASE("s2 offspeed: the counts sentence names other speeds only when there are some") {
    app::dm_report::DmReportStats st;
    st.total = 4;
    st.under_optimal = 1;
    st.above_optimal = 1;
    st.not_in_library = 1;
    st.other_speed = 1;
    CHECK(app::dm_report::counts_phrase(st) ==
          "1 under optimal, 0 at optimal, 1 above optimal, 0 not analyzed, "
          "1 not in your library, 1 at another speed");
    st.other_speed = 0;
    CHECK(app::dm_report::counts_phrase(st) ==
          "1 under optimal, 0 at optimal, 1 above optimal, 0 not analyzed, "
          "1 not in your library");
}

TEST_CASE("s2 offspeed: the subtitle and the Other speed tile count 'other speed'") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    const app::dm_report::GeneratedDmReport report = app::dm_report::generate_dm_report(
        store, {score_at(kHash, optimal - 1, 100), score_at(kHash, optimal + 5, 150),
                score_at(kHash, optimal - 9, 50)},
        kMode, store::Lens{}, "TestUser");
    CHECK(report.stats.other_speed == 2);
    CHECK(report.subtitle ==
          "TestUser — 3 scores: 1 under optimal, 0 at optimal, "
          "0 above optimal, 0 not analyzed, 0 not in your library, "
          "2 at other speeds");

    CHECK(testdm::dm_tile(report.rows, "Other speed") == "2");
}
