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
        score_at(kHash, optimal - 1000, 75),   // would read "matched"
        score_at("00ff00ff00ff00ff00ff00ff00ff00ff", 123, 150),  // no result at all
        score_at(kHash, optimal - 1000, 100),  // base speed: matched, as today
    };
    const std::vector<DmReportRow> rows =
        app::dm_report::collect_dm_rows(store, scores, kMode, store::Lens{});
    REQUIRE(rows.size() == 4);

    // These strings are load-bearing: the page's filter and chip classes key on them.
    CHECK(rows[0].status == "other speed");
    CHECK(rows[1].status == "other speed");
    CHECK(rows[2].status == "other speed");
    CHECK(rows[3].status == "matched");

    // The numbers still show; only the comparison is withheld.
    CHECK(rows[0].optimal == optimal);
    CHECK(rows[1].delta == 1000);
    CHECK_FALSE(rows[0].pct.has_value());
    CHECK_FALSE(rows[1].pct.has_value());
    REQUIRE(rows[3].pct.has_value());

    const app::dm_report::DmReportStats stats = app::dm_report::tally_dm_rows(rows);
    CHECK(stats.total == 4);
    CHECK(stats.matched == 1);
    CHECK(stats.above_optimal == 0);
    CHECK(stats.other_speed == 3);
    CHECK(stats.not_in_library == 0);
}

TEST_CASE("s2 offspeed: the counts sentence names other speeds only when there are some") {
    app::dm_report::DmReportStats st;
    st.total = 4;
    st.matched = 1;
    st.above_optimal = 1;
    st.not_in_library = 1;
    st.other_speed = 1;
    CHECK(app::dm_report::counts_phrase(st) ==
          "1 matched, 1 above optimal, 0 not analyzed, 1 not in your library, "
          "1 at another speed");
    st.other_speed = 0;
    CHECK(app::dm_report::counts_phrase(st) ==
          "1 matched, 1 above optimal, 0 not analyzed, 1 not in your library");
}

TEST_CASE("s2 offspeed: the page filters, colours and counts 'other speed'") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    const app::dm_report::GeneratedDmReport report = app::dm_report::generate_dm_report(
        store, {score_at(kHash, optimal - 1, 100), score_at(kHash, optimal + 5, 150),
                score_at(kHash, optimal - 9, 50)},
        kMode, store::Lens{}, "TestUser");
    CHECK(report.stats.other_speed == 2);
    CHECK(report.html.find("TestUser — 3 scores: 1 matched, 0 above optimal, 0 not analyzed, "
                           "0 not in your library, 2 at other speeds") != std::string::npos);
    CHECK(report.html.find("<option value=\"other speed\">Other speed</option>") !=
          std::string::npos);
    CHECK(report.html.find("'other speed':'s-otherspeed'") != std::string::npos);
    CHECK(report.html.find("['Other speed', otherSpeed.length.toLocaleString()]") !=
          std::string::npos);
    CHECK(report.html.find(".s-otherspeed{") != std::string::npos);
}

// The page's stat row counts statuses in JavaScript, and tally_dm_rows counts
// them in C++. The page has to count per filtered view, so it can't just print
// the C++ totals. Instead this case pins that both count the same statuses:
// every status the C++ assigns has its own tally field, its own count in the
// page's stats(), its own filter option and its own chip class, and the page
// counts no status the C++ doesn't know.
TEST_CASE("s2 offspeed: the page counts the same statuses tally_dm_rows counts") {
    const std::vector<std::string> statuses = {"matched", "above optimal", "not analyzed",
                                               "not in library", "other speed"};
    std::vector<DmReportRow> rows;
    for (const std::string& s : statuses) {
        DmReportRow r;
        r.status = s;
        rows.push_back(r);
    }
    const app::dm_report::DmReportStats st = app::dm_report::tally_dm_rows(rows);
    CHECK(st.total == 5);
    CHECK(st.matched == 1);
    CHECK(st.above_optimal == 1);
    CHECK(st.not_analyzed == 1);
    CHECK(st.not_in_library == 1);
    CHECK(st.other_speed == 1);

    // The page shell with no rows. (generate_dm_report returns no page at all
    // for an empty score list, so the shell is built directly.)
    const std::string html = app::dm_report::build_dm_html({}, "TestUser", "footer");
    for (const std::string& s : statuses) {
        CAPTURE(s);
        CHECK(html.find("rows.filter(r => r.status === '" + s + "')") != std::string::npos);
        CHECK(html.find("<option value=\"" + s + "\">") != std::string::npos);
        CHECK(html.find("'" + s + "':'s-") != std::string::npos);
    }
    // No status counted on the page that the C++ never assigns.
    size_t counted = 0;
    for (size_t at = html.find("rows.filter(r => r.status === '"); at != std::string::npos;
         at = html.find("rows.filter(r => r.status === '", at + 1))
        ++counted;
    CHECK(counted == statuses.size());

    // The help text reads the base speed from kBaseSpeedPercent, not a literal.
    CHECK(html.find("__BASE_SPEED__") == std::string::npos);
    CHECK(html.find("Other speed: played at a speed other than " +
                    std::to_string(net::kBaseSpeedPercent) + "%.") != std::string::npos);
}
