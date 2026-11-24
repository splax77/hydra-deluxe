// Off-speed leaderboard scores get their own status (D25, finding 222).
// Hydra's optimal is computed for normal speed only, so an off-speed score is
// shown but not compared.

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

// Speeds either side of the base one; any speed but the base is off speed.
const int kFaster = net::kBaseSpeedPercent + 50;
const int kSlower = net::kBaseSpeedPercent - 25;

TEST_CASE("s2 offspeed: one base speed, read everywhere") {
    CHECK(net::is_base_speed(net::kBaseSpeedPercent));
    CHECK_FALSE(net::is_base_speed(kFaster));
    CHECK_FALSE(net::is_base_speed(kSlower));
    // A score or row that names no speed counts as played at the base speed.
    CHECK(net::DmScore{}.speed == net::kBaseSpeedPercent);
    CHECK(DmReportRow{}.speed == net::kBaseSpeedPercent);
}

TEST_CASE("s2 offspeed: an off-speed score on a listed chart is 'other speed'; on an unlisted one, 'not in library'") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    // fill_store lists kHash only, so the third hash is a chart the library
    // doesn't list.
    const std::vector<net::DmScore> scores = {
        score_at(kHash, optimal + 5, kFaster),     // would read "above optimal"
        score_at(kHash, optimal - 1000, kSlower),  // would read "under optimal"
        score_at("00ff00ff00ff00ff00ff00ff00ff00ff", 123, kFaster),  // not listed (D284 item 2)
        score_at(kHash, optimal - 1000, net::kBaseSpeedPercent),  // base speed: under optimal
    };
    const std::vector<DmReportRow> rows =
        app::dm_report::collect_dm_rows(store, scores, testdm::profile());
    REQUIRE(rows.size() == scores.size());

    CHECK(rows[0].status == app::dm_report::kStatusOtherSpeed);
    CHECK(rows[1].status == app::dm_report::kStatusOtherSpeed);
    CHECK(rows[2].status == app::dm_report::kStatusNotInLibrary);
    CHECK(rows[2].speed == kFaster);  // the speed column still shows it
    CHECK(rows[3].status == app::dm_report::kStatusUnderOptimal);

    // The numbers still show; only the comparison is withheld.
    CHECK(rows[0].optimal == optimal);
    CHECK(rows[1].delta == optimal - scores[1].score);
    // Timing required is the best path's, whatever speed the score was
    // played at: each off-speed row carries the base-speed row's figure.
    CHECK(rows[0].timing_required == rows[3].timing_required);
    CHECK(rows[1].timing_required == rows[3].timing_required);
    CHECK_FALSE(rows[2].timing_required.has_value());

    const app::dm_report::DmReportStats stats = app::dm_report::tally_dm_rows(rows);
    CHECK(stats.total == static_cast<int>(scores.size()));
    // Each tally field counts the rows whose status is its word.
    auto rows_with = [&](const char* status) {
        int n = 0;
        for (const DmReportRow& r : rows)
            if (r.status == status) ++n;
        return n;
    };
    CHECK(stats.under_optimal == rows_with(app::dm_report::kStatusUnderOptimal));
    CHECK(stats.at_optimal == rows_with(app::dm_report::kStatusAtOptimal));
    CHECK(stats.above_optimal == rows_with(app::dm_report::kStatusAboveOptimal));
    CHECK(stats.other_speed == rows_with(app::dm_report::kStatusOtherSpeed));
    CHECK(stats.not_in_library == rows_with(app::dm_report::kStatusNotInLibrary));
    // The rows hold two off-speed scores, so the loop is not counting zeros.
    CHECK(stats.other_speed > 0);
    CHECK(stats.not_in_library > 0);
}

TEST_CASE("s2 offspeed: the counts sentence names other speeds only when there are some") {
    app::dm_report::DmReportStats st;
    st.total = 4;
    st.under_optimal = 1;
    st.above_optimal = 1;
    st.not_in_library = 1;
    const app::dm_report::StatusWords& other =
        app::dm_report::status_words(app::dm_report::kStatusOtherSpeed);
    REQUIRE(other.count_many != nullptr);
    st.other_speed = 0;
    const std::string without = app::dm_report::counts_phrase(st);
    CHECK(without.find(other.count_one) == std::string::npos);
    CHECK(without.find(other.count_many) == std::string::npos);
    // With some, the phrase gains the other-speed count at its end, in the
    // status's own count words, and reads as before up to there.
    for (int n : {1, 2}) {
        st.other_speed = n;
        INFO("other speed scores: " << n);
        CHECK(app::dm_report::counts_phrase(st) ==
              without + ", " + hydra::counted(n, other.count_one, other.count_many));
    }
}

TEST_CASE("s2 offspeed: the subtitle counts every score; the Other speed tile counts 'other speed'") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    const std::vector<net::DmScore> scores = {score_at(kHash, optimal - 1, net::kBaseSpeedPercent),
                                              score_at(kHash, optimal + 5, kFaster),
                                              score_at(kHash, optimal - 9, kSlower)};
    int off_speed = 0;
    for (const net::DmScore& s : scores)
        if (!net::is_base_speed(s.speed)) ++off_speed;
    const std::string user = "TestUser";
    const app::dm_report::GeneratedDmReport report =
        app::dm_report::generate_dm_report(store, scores, testdm::profile(), user);
    CHECK(report.stats.other_speed == off_speed);
    // Off-speed scores count in the subtitle like any other (D103 item 27):
    // its count is every score given, not the base-speed ones.
    CHECK(report.stats.total == static_cast<int>(scores.size()));
    CHECK(report.subtitle.find(hydra::counted(static_cast<int64_t>(scores.size()), "score",
                                              "scores")) != std::string::npos);
    CHECK(report.subtitle.rfind(user, 0) == 0);
    CHECK(report.subtitle.find(app::dm_report::clone_hero_cap_phrase()) != std::string::npos);

    const char* tile = app::dm_report::status_words(app::dm_report::kStatusOtherSpeed).name;
    CHECK(testdm::dm_tile(report.rows, tile) == hydra::group_thousands(report.stats.other_speed));
}
