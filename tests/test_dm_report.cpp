// Tests for app/dm_report: the score-vs-optimal join (collect_dm_rows) and
// generate_dm_report's subtitle and footer. Pins the status strings the
// comparison window's Status filter and chips key on, and the counts the app
// shows. Also drives the real WinHTTP transport against a loopback server.

// winsock2.h has to come before anything that pulls in windows.h.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"          // Settings
#include "app/display_format.h"  // format_percent, percent_steps
#include "core/model.h"
#include "app/dm_report.h"
#include "app/dm_report_view.h"  // dm_search_text
#include "app/report.h"          // Tile, ChipToken, kNoChartLibrary
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "core/strutil.h"
#include "corpus_util.h"
#include "display_fixtures.h"  // kTagOnlyTitle
#include "dm_fixture.h"
#include "net/dmbot_client.h"
#include "net/http.h"
#include "parse/song.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "wait_util.h"

using namespace hydra;
using app::dm_report::DmReportRow;

// The store and score builders are the shared leaderboard fixture.
using testdm::fill_store;
using testdm::kHash;
using testdm::kMode;
using testdm::make_score;
using testdm::dm_tile;

TEST_CASE("collect_dm_rows joins scores to records and labels them") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    std::vector<net::DmScore> scores;
    scores.push_back(make_score(kHash, optimal - 1000));       // under optimal
    scores.push_back(make_score(kHash, optimal));              // at optimal
    scores.push_back(make_score(kHash, optimal + 5));          // above optimal
    scores.push_back(make_score("00ff00ff00ff00ff00ff00ff00ff00ff",
                                123456));                      // not in library

    std::vector<DmReportRow> rows =
        app::dm_report::collect_dm_rows(store, scores, testdm::profile());
    REQUIRE(rows.size() == 4);

    // These exact strings are load-bearing: the page's status filter and chip
    // classes key on them. A score under optimal is never called "matched".
    CHECK(rows[0].status == "under optimal");
    CHECK(rows[1].status == "at optimal");
    CHECK(rows[2].status == "above optimal");
    CHECK(rows[3].status == "not in library");
    for (const DmReportRow& r : rows) CHECK(r.status != "matched");

    CHECK(rows[0].optimal == optimal);
    CHECK(rows[0].delta == 1000);
    // The row's percent is percent_steps' whole hundredths, the number the
    // cell's text is written from: 99.56% for 1,000 under the first corpus
    // chart's optimal, read from one run.
    REQUIRE(rows[0].pct_h.has_value());
    CHECK(*rows[0].pct_h == 9956);

    CHECK(rows[1].delta == 0);
    CHECK(rows[2].delta == -5);
    CHECK_FALSE(rows[3].optimal.has_value());
    CHECK_FALSE(rows[3].delta.has_value());

    // The leaderboard's own metadata wins when it has it.
    CHECK(rows[0].song == "Board Title");
}

TEST_CASE("collect_dm_rows: no percent off 100% speed; store identity fallback") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);

    net::DmScore fast = make_score(kHash, optimal - 10);
    fast.speed = 150;
    net::DmScore unknown_meta = make_score(kHash, optimal - 10);
    unknown_meta.known = false;  // "unknown song" entry: no usable metadata

    std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store, {fast, unknown_meta}, testdm::profile());
    REQUIRE(rows.size() == 2);

    CHECK_FALSE(rows[0].pct_h.has_value());  // speed != 100
    CHECK(rows[1].song == "Stored Title");  // fell back to the matched record
}

TEST_CASE("dmbot JSON parsers handle canned payloads") {
    // The users endpoint: elo/stats may be null or missing; entries without
    // an id are dropped.
    const std::string users_body = R"([
        {"id":"111","username":"alice","elo":1500,
         "stats":{"total_scores":10,"total_score":123456}},
        {"id":"222","username":"bob","elo":null},
        {"username":"no_id_dropped"}
    ])";
    std::vector<net::DmUser> users = net::parse_users_json(users_body);
    REQUIRE(users.size() == 2);
    CHECK(users[0].id == "111");
    CHECK(users[0].username == "alice");
    CHECK(users[0].elo == 1500);
    CHECK(users[0].total_scores == 10);
    CHECK(users[0].total_score == 123456);
    CHECK_FALSE(users[1].elo.has_value());

    // The scores endpoint: two arrays, flattened and told apart by `known`;
    // identifiers are lowercased; charter_refs join with ", "; missing speed
    // defaults to 100.
    const std::string scores_body = R"({
        "scores":[
            {"identifier":"AABB01","song_name":"Song","artist":"Artist",
             "charter_refs":["c1","c2"],"score":1000,"is_fc":1,"percent":98,
             "speed":150,"rank":3,"posted":"2026-01-01"}
        ],
        "unknown_scores":[
            {"identifier":"ccdd02","score":500}
        ]
    })";
    std::vector<net::DmScore> scores = net::parse_scores_json(scores_body);
    REQUIRE(scores.size() == 2);
    CHECK(scores[0].identifier == "aabb01");
    CHECK(scores[0].charter == "c1, c2");
    CHECK(scores[0].score == 1000);
    CHECK(scores[0].is_fc);
    CHECK(scores[0].speed == 150);
    CHECK(scores[0].rank == 3);
    CHECK(scores[0].known);
    CHECK(scores[1].known == false);
    CHECK(scores[1].speed == 100);

    // Unparseable bodies throw the user-facing error, not a JSON exception.
    CHECK_THROWS_AS(net::parse_users_json("not json"), std::runtime_error);
    CHECK_THROWS_AS(net::parse_scores_json("<html>503</html>"), std::runtime_error);

    // A non-array user body is rejected.
    CHECK_THROWS_AS(net::parse_users_json("{\"a\":1}"), std::runtime_error);
}

// D64.
TEST_CASE("collect_dm_rows answers above-optimal beside the status") {
    // A score at another speed that beats the optimal keeps reading "+N
    // over". Its status is "other speed", so the row carries the
    // above-optimal answer collect_dm_rows already gave, next to the delta;
    // the Delta cell (dm_report_view) reads it instead of the delta's sign.
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);
    const std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store,
        {make_score(kHash, optimal + 5), make_score(kHash, optimal + 5, 150),
         make_score(kHash, optimal - 5, 150)},
        testdm::profile());
    REQUIRE(rows.size() == 3);
    CHECK(rows[0].status == "above optimal");
    CHECK(rows[1].status == "other speed");
    CHECK(rows[2].status == "other speed");
    CHECK(rows[0].delta == -5);
    CHECK(rows[0].above_optimal);
    CHECK(rows[1].delta == -5);
    CHECK(rows[1].above_optimal);
    CHECK(rows[2].delta == 5);
    CHECK_FALSE(rows[2].above_optimal);
}

TEST_CASE("dm comparison: a score's search text is folded and tag-free") {
    DmReportRow row;
    row.song = "Halo";
    row.artist = "Beyonc\xc3\xa9";  // Beyoncé
    row.charter = "<b>Bob</b>";
    row.status = "not in library";
    CHECK(app::dm_report_view::dm_search_text(row) == "halo beyonce bob");
}

TEST_CASE("generate_dm_report: tally and framing behind one seam") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    std::vector<net::DmScore> scores;
    scores.push_back(make_score(kHash, optimal - 1000));  // under optimal
    scores.push_back(make_score(kHash, optimal - 10));    // under optimal
    scores.push_back(make_score(kHash, optimal));         // at optimal
    scores.push_back(make_score(kHash, optimal + 5));     // above optimal
    scores.push_back(make_score("00ff00ff00ff00ff00ff00ff00ff00ff",
                                123456));                 // not in library

    app::dm_report::GeneratedDmReport result =
        app::dm_report::generate_dm_report(store, scores, testdm::profile(), "TestUser");
    CHECK(result.stats.total == 5);
    CHECK(result.stats.under_optimal == 2);
    CHECK(result.stats.at_optimal == 1);
    CHECK(result.stats.above_optimal == 1);
    CHECK(result.stats.not_analyzed == 0);
    CHECK(result.stats.not_in_library == 1);

    // The subtitle counts every score and nothing by status: the tiles count
    // the scores that pass the filters (D103 item 27).
    CHECK(result.subtitle ==
          "TestUser — 5 scores — " + app::dm_report::clone_hero_cap_phrase());
    CHECK(result.footer.rfind("Actual scores from dmleaderboards.com against Hydra's", 0) == 0);

    // No scores: zero stats, no rows.
    app::dm_report::GeneratedDmReport none =
        app::dm_report::generate_dm_report(store, {}, testdm::profile(), "TestUser");
    CHECK(none.stats.total == 0);
    CHECK(none.rows.empty());
}

// What a window draws from: the rows, the subtitle and footer the page shows,
// and who and which mode it compared.
TEST_CASE("generate_dm_report hands over its rows, subtitle, footer, player and mode") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);
    std::vector<net::DmScore> scores;
    scores.push_back(make_score(kHash, optimal - 1000));
    scores.push_back(make_score("00ff00ff00ff00ff00ff00ff00ff00ff", 123456));

    const app::dm_report::GeneratedDmReport result =
        app::dm_report::generate_dm_report(store, scores, testdm::profile(), "TestUser");
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0].status == "under optimal");
    CHECK(result.rows[1].status == "not in library");
    CHECK(result.username == "TestUser");
    CHECK(result.chartmode == kMode);
    // The line ends with the cap the comparison runs at (D276 item 7).
    CHECK(result.subtitle ==
          "TestUser — 2 scores — " + app::dm_report::clone_hero_cap_phrase());
    // One score reads singular, as the count line writes its noun.
    CHECK(app::dm_report::generate_dm_report(store, {scores[0]}, testdm::profile(), "TestUser")
              .subtitle == "TestUser — 1 score — " + app::dm_report::clone_hero_cap_phrase());
    CHECK(result.footer.rfind("Actual scores from dmleaderboards.com against Hydra's optimal for " +
                                  std::string(kMode) + ".",
                              0) == 0);
    // The last sentence, in the "no current result" words (D114 item 1), telling
    // the player to set the cap the comparison reads (D281).
    const std::string last_sentence =
        " Charts with no current result are in your library without a result for this mode at "
        "SP cap 4: set SP cap to 4, analyze them, then compare again.";
    REQUIRE(result.footer.size() > last_sentence.size());
    CHECK(result.footer.substr(result.footer.size() - last_sentence.size()) == last_sentence);

    // No scores: the player and mode are still named, and nothing else is.
    const app::dm_report::GeneratedDmReport none =
        app::dm_report::generate_dm_report(store, {}, testdm::profile(), "TestUser");
    CHECK(none.rows.empty());
    CHECK(none.username == "TestUser");
    CHECK(none.chartmode == kMode);
    CHECK(none.subtitle.empty());
    CHECK(none.footer.empty());
}

TEST_CASE("dm_tiles: the tiles count only the scores shown") {
    auto row = [](const char* status, std::optional<int64_t> delta,
                  std::optional<int64_t> pct_h) {
        DmReportRow r;
        r.song = "Song";
        r.status = status;
        r.delta = delta;
        r.pct_h = pct_h;
        return r;
    };
    const std::vector<DmReportRow> rows = {
        row("under optimal", 2000, 9901),
        row("under optimal", 1500, 9900),
        row("at optimal", 0, 10000),
        // Played off base speed: its delta leaves no points on the table.
        row("other speed", 500, std::nullopt),
        row("no paths", std::nullopt, std::nullopt),
    };
    auto values = [&](const std::vector<size_t>& shown) {
        std::vector<std::string> out;
        for (const app::report::Tile& t : app::dm_report::dm_tiles(rows, shown))
            out.push_back(t.value);
        return out;
    };
    // The first tile says it counts the scores shown, like the path report's
    // Charts shown (D103 item 27).
    std::vector<std::string> labels;
    for (const app::report::Tile& t : app::dm_report::dm_tiles(rows, {0}))
        labels.push_back(t.label);
    CHECK(labels == std::vector<std::string>{"Scores shown", "Under optimal", "At optimal",
                                             "Above optimal", "No current result", "Not in your library",
                                             "Other speed", "Avg % of optimal",
                                             "Points left on table"});
    // A "no paths" score has no tile of its own; Scores shown counts it (D62
    // item 1).
    CHECK(values({0, 1, 2, 3, 4}) ==
          std::vector<std::string>{"5", "2", "1", "0", "0", "0", "1", "99.34%", "3,500"});
    // One row's average reads its own cell.
    CHECK(values({0}) ==
          std::vector<std::string>{"1", "1", "0", "0", "0", "0", "0", "99.01%", "2,000"});
    // 99.005% on average: the half rounds up, as format_percent rounds it.
    CHECK(values({0, 1})[7] == "99.01%");
    // No row with a percent, and nothing under optimal.
    CHECK(values({3, 4}) ==
          std::vector<std::string>{"2", "0", "0", "0", "0", "0", "1", "—", "0"});
    CHECK(values({}) ==
          std::vector<std::string>{"0", "0", "0", "0", "0", "0", "0", "—", "0"});
}

TEST_CASE("status_token: each status's chip colour, as the page's STATUS_CLASS gives it") {
    using app::report::ChipToken;
    using app::dm_report::status_token;
    // s-matched is drawn in --t0, s-above in --t1, s-notanalyzed and
    // s-otherspeed in --muted, and s-unmatched in --tn.
    CHECK(status_token("under optimal") == ChipToken::t0);
    CHECK(status_token("at optimal") == ChipToken::t0);
    CHECK(status_token("above optimal") == ChipToken::t1);
    CHECK(status_token("no current result") == ChipToken::muted);
    CHECK(status_token("no paths") == ChipToken::muted);
    CHECK(status_token("other speed") == ChipToken::muted);
    CHECK(status_token("not in library") == ChipToken::tn);
    // The page draws a status it doesn't know as s-unmatched.
    CHECK(status_token("something else") == ChipToken::tn);
}

TEST_CASE("collect_dm_rows: the stored names go through display_title, display_artist and "
          "display_charter") {
    // test_song_input.cpp pins what those three do. This checks only that the
    // page calls them: one tagged name each.
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    test::name_chart(store, kHash, "<b>Bold Title</b>", test::kTagOnlyTitle, " <b>Bob</b> ");

    // The leaderboard has no metadata for it, so the row falls back to the
    // matched record's stored names.
    net::DmScore unknown_meta = make_score(kHash, optimal - 10);
    unknown_meta.known = false;

    const std::vector<DmReportRow> rows =
        app::dm_report::collect_dm_rows(store, {unknown_meta}, testdm::profile());
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].song == "Bold Title");
    CHECK(rows[0].artist == kUnknownTitle);
    CHECK(rows[0].charter == "Bob");
}

// D74 item 2: the leaderboard's own names go through the same display owners
// as the stored ones, so Clone Hero tags in DMBot's text never reach the page.
TEST_CASE("collect_dm_rows: a leaderboard row's DMBot names lose their Clone Hero tags") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);

    net::DmScore tagged = make_score(kHash, optimal - 10);
    tagged.song_name = "<color=#e02222>Blood</color>line";
    tagged.artist = " <i>Tagged</i> Artist ";
    tagged.charter = " <b>Bob</b> ";

    // The same names on a score the library doesn't have, so no stored record
    // can stand in for them.
    net::DmScore tagged_unmatched = tagged;
    tagged_unmatched.identifier = "00ff00ff00ff00ff00ff00ff00ff00ff";

    std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store, {tagged, tagged_unmatched}, testdm::profile());
    REQUIRE(rows.size() == 2);
    for (const DmReportRow& r : rows) {
        CHECK(r.song == "Bloodline");
        CHECK(r.artist == "Tagged Artist");
        CHECK(r.charter == "Bob");
    }
}

TEST_CASE("collect_dm_rows: a leaderboard row with no charter shows the library's, else "
          "(unknown) (D266 item 2)") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);

    // DMBot knows the song but sent no charter; the library's chart lends its.
    net::DmScore no_charter = make_score(kHash, optimal - 10);
    no_charter.charter.clear();

    // The same score on a song the library doesn't have: nothing to lend.
    net::DmScore no_charter_unmatched = no_charter;
    no_charter_unmatched.identifier = "00ff00ff00ff00ff00ff00ff00ff00ff";

    const std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store, {no_charter, no_charter_unmatched}, testdm::profile());
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].charter == "Stored Charter");
    CHECK(rows[1].charter == kUnknownTitle);
}

namespace {

// The text the comparison window's column `id` shows for `row`
// (dm_report_view::dm_columns).
std::string dm_cell(const std::string& id, const DmReportRow& row) {
    for (const auto& c : app::dm_report_view::dm_columns())
        if (c.id == id) return c.cell(row);
    FAIL("no column " << id);
    return {};
}

// The sort key the comparison window's column `id` gives `row`.
app::report_view::SortKey dm_sort_key(const std::string& id, const DmReportRow& row) {
    for (const auto& c : app::dm_report_view::dm_columns())
        if (c.id == id) return c.sort_key(row);
    FAIL("no column " << id);
    return {};
}

}  // namespace

TEST_CASE("dm comparison: a percent rounds once") {
    // 198,010 of 198,020 is 99.99495%: rounded once it reads 99.99%, where
    // rounding to four places first and then to two read 100.00%. The row is
    // the one collect_dm_rows makes for that score at base speed.
    DmReportRow row;
    row.song = "Percent Song";
    row.actual = 198010;
    row.optimal = 198020;
    row.delta = 10;
    row.pct_h = 9999;
    row.status = "under optimal";

    // The % of opt cell writes the row's whole hundredths as text, and sorts
    // on those same hundredths.
    CHECK(dm_cell("pct_h", row) == "99.99%");
    CHECK(std::get<double>(dm_sort_key("pct_h", row)) == 9999.0);
}

TEST_CASE("dm comparison: the average tile reads a percent the way the cells do") {
    // 198,010 of 200,000 is exactly 99.005%. format_percent rounds the half
    // up, so the cell reads 99.01%. The page's old tile rounded the float
    // 99.00499999... with toFixed(2) and read 99.00%.
    CHECK(app::format_percent(198010, 200000, 2) == "99.01%");
    CHECK(app::percent_steps(198010, 200000, 2) == 9901);

    DmReportRow row;
    row.song = "Half Song";
    row.actual = 198010;
    row.optimal = 200000;
    row.delta = 1990;
    row.pct_h = 9901;
    row.status = "under optimal";

    // The cell's text and the tile's input both come from the row's whole
    // hundredths, so one score's tile reads the cell's own text.
    CHECK(dm_cell("pct_h", row) == "99.01%");
    CHECK(dm_tile({row}, "Avg % of optimal") == "99.01%");
}

TEST_CASE("collect_dm_rows: the % of opt column sorts on the shown hundredths") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    // The first two scores are one point apart. Their percents differ only
    // past the second decimal, so both read 100.00% and carry the same whole
    // hundredths. The third, half the optimal, reads 50.00%: lower at the
    // shown precision.
    std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store,
        {make_score(kHash, optimal), make_score(kHash, optimal - 1),
         make_score(kHash, optimal / 2)},
        testdm::profile());
    REQUIRE(rows.size() == 3);
    REQUIRE(rows[0].pct_h.has_value());
    REQUIRE(rows[1].pct_h.has_value());
    REQUIRE(rows[2].pct_h.has_value());
    CHECK(*rows[0].pct_h == 10000);
    CHECK(*rows[1].pct_h == 10000);
    CHECK(*rows[2].pct_h == 5000);
    CHECK(app::format_percent(rows[1].actual, optimal, 2) == "100.00%");

    // The "% of opt" column sorts on that one value, so the two 100.00% rows
    // tie (the window's sort keeps ties in order) and the 50.00% row sorts
    // below both.
    CHECK(std::get<double>(dm_sort_key("pct_h", rows[0])) == 10000.0);
    CHECK(std::get<double>(dm_sort_key("pct_h", rows[1])) == 10000.0);
    CHECK(std::get<double>(dm_sort_key("pct_h", rows[2])) == 5000.0);
}

namespace {

// A one-shot HTTP server on 127.0.0.1 for the real WinHTTP transport (the
// Fetcher seam swaps http_get out wholesale, so it can't reach that code).
// One thread accepts one connection, reads the request headers and sends
// `reply`. With `stall` set it then sends nothing more and keeps the
// connection open, like a render.com cold start (empty `reply`) or a body
// that stops arriving (a reply of headers and a short body). A stall ends
// when the server is destroyed, or at testwait's cap, so a cancel that never
// works fails the test instead of hanging it.
class LoopbackServer {
public:
    explicit LoopbackServer(std::string reply, bool stall = false)
        : reply_(std::move(reply)), stall_(stall) {
        WSADATA wsa;
        REQUIRE(WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
        listener_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        REQUIRE(listener_ != INVALID_SOCKET);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;  // any free port
        REQUIRE(bind(listener_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0);
        REQUIRE(listen(listener_, 4) == 0);
        int len = sizeof(addr);
        REQUIRE(getsockname(listener_, reinterpret_cast<sockaddr*>(&addr), &len) == 0);
        port_ = ntohs(addr.sin_port);
        thread_ = std::thread([this] { serve_one(); });
    }
    ~LoopbackServer() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            released_ = true;
        }
        release_.notify_all();   // ends a stall
        closesocket(listener_);  // also unblocks an accept() still waiting
        if (thread_.joinable()) thread_.join();
        WSACleanup();
    }
    LoopbackServer(const LoopbackServer&) = delete;
    LoopbackServer& operator=(const LoopbackServer&) = delete;

    std::string url() const { return "http://127.0.0.1:" + std::to_string(port_); }

    // True once the whole reply is sent (none, for an empty reply) and the
    // connection is stalled: the client's request is in, and it now waits on
    // a step only a cancel can end.
    bool stalled() const { return stalled_.load(); }

private:
    void serve_one() {
        SOCKET s = accept(listener_, nullptr, nullptr);
        if (s == INVALID_SOCKET) return;
        std::string request;
        char buf[4096];
        while (request.find("\r\n\r\n") == std::string::npos) {
            int n = recv(s, buf, sizeof(buf), 0);
            if (n <= 0) break;
            request.append(buf, static_cast<size_t>(n));
        }
        size_t sent = 0;
        while (sent < reply_.size()) {
            int n = send(s, reply_.data() + sent,
                         static_cast<int>(std::min<size_t>(reply_.size() - sent, 1 << 16)), 0);
            if (n <= 0) break;
            sent += static_cast<size_t>(n);
        }
        if (stall_) {
            stalled_ = true;
            std::unique_lock<std::mutex> lock(mutex_);
            release_.wait_for(lock, testwait::kWaitCap, [this] { return released_; });
            closesocket(s);
            return;
        }
        // Graceful close: FIN, then wait for the client to hang up, so no
        // unread bytes turn the close into a reset that drops the reply.
        shutdown(s, SD_SEND);
        while (recv(s, buf, sizeof(buf), 0) > 0) {}
        closesocket(s);
    }

    std::string reply_;
    bool stall_ = false;
    std::atomic<bool> stalled_{false};
    std::mutex mutex_;
    std::condition_variable release_;
    bool released_ = false;  // guarded by mutex_
    SOCKET listener_ = INVALID_SOCKET;
    unsigned short port_ = 0;
    std::thread thread_;
};

std::string http_response(const std::string& status_line, const std::string& body) {
    return "HTTP/1.1 " + status_line +
           "\r\nContent-Type: application/json\r\nContent-Length: " +
           std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
}

// The response headers of a 200 whose body is promised but never comes in
// full: Content-Length is larger than what follows.
std::string stalled_body_response(const std::string& body_start) {
    return "HTTP/1.1 200 OK\r\nContent-Length: 1000\r\nConnection: close\r\n\r\n" + body_start;
}

// What one cancelled call ended with. `error` is its exception text.
// `cancel_to_return_ms` is how long it ran on after the cancel was set; it is
// empty when the call ended before `ready` came true, so no cancel was set.
struct CancelResult {
    std::string error;
    std::optional<long long> cancel_to_return_ms;
};

// Runs `call`, and sets `cancel` from a second thread once `ready()` says the
// call is parked on a step only a cancel can end. Timing starts at the cancel,
// so WinHTTP's own setup before that is not counted.
template <class Ready, class Call>
CancelResult cancel_when(std::atomic<bool>& cancel, Ready ready, Call call) {
    using clock = std::chrono::steady_clock;
    std::atomic<bool> call_ended{false};
    std::optional<clock::time_point> cancel_at;
    std::string wait_error;
    std::thread setter([&] {
        try {
            testwait::wait_until([&] { return call_ended.load() || ready(); },
                                 "the loopback server to stall the request");
        } catch (const testwait::WaitTimeout& e) {
            wait_error = e.what();
            cancel = true;  // so the call cannot run on to its own 120 s timeout
            return;
        }
        if (call_ended) return;
        cancel_at = clock::now();
        cancel = true;
    });
    CancelResult result;
    try {
        call();
    } catch (const std::exception& e) {
        result.error = e.what();
    }
    const clock::time_point returned = clock::now();
    call_ended = true;
    setter.join();
    REQUIRE_MESSAGE(wait_error.empty(), wait_error);
    if (cancel_at)
        result.cancel_to_return_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(returned - *cancel_at).count();
    return result;
}

// The checks every cancel case shares: the cancel was set while the call was
// stalled, the call ended with the transport's "cancelled" (a receive timeout
// or a dropped connection reads differently), and it did so within 500 ms of
// the cancel (the user's limit).
void check_cancelled_promptly(const CancelResult& r) {
    REQUIRE_MESSAGE(r.cancel_to_return_ms.has_value(),
                    "the call ended before the cancel was set, with: " << r.error);
    MESSAGE("returned " << *r.cancel_to_return_ms << " ms after the cancel flag was set");
    CHECK(r.error == "cancelled");
    CHECK(*r.cancel_to_return_ms < 500);
}

}  // namespace

TEST_CASE("http transport: cancel aborts a request the server never answers") {
    net::set_fetcher({});  // the real WinHTTP transport
    LoopbackServer server("", /*stall=*/true);

    // Set before the call: fails at once, before any connection is made.
    std::atomic<bool> cancel{true};
    CHECK_THROWS_WITH_AS((void)net::fetch_users(server.url(), &cancel), "cancelled",
                         std::runtime_error);
    CHECK_FALSE(server.stalled());

    // Set once the server holds the request and sends nothing back (the
    // receive timeout is 120 s, so only the cancel can end this promptly).
    cancel = false;
    check_cancelled_promptly(cancel_when(cancel, [&] { return server.stalled(); },
                                         [&] { (void)net::fetch_users(server.url(), &cancel); }));
}

TEST_CASE("http transport: cancel aborts a body that stops after the headers") {
    net::set_fetcher({});  // the real WinHTTP transport
    net::set_downloader({});

    // The server cannot see when the client has read the headers, so here the
    // cancel may land just before the body read starts. The download subcase
    // below waits until body bytes reach the file, so it cancels inside it.
    SUBCASE("http_get: headers, then nothing") {
        LoopbackServer server(stalled_body_response(""), /*stall=*/true);
        std::atomic<bool> cancel{false};
        check_cancelled_promptly(cancel_when(cancel, [&] { return server.stalled(); },
                                             [&] { (void)net::fetch_users(server.url(), &cancel); }));
    }

    SUBCASE("http_download: part of the body, then nothing") {
        LoopbackServer server(stalled_body_response("[{\"id\":"), /*stall=*/true);
        testtemp::ScopedFile file("dm_http_stall", ".bin");
        const std::string part = file.path + ".part";
        std::atomic<uint64_t> received{0};
        std::atomic<bool> cancel{false};
        // The first body bytes reaching the file prove the call is in its body
        // read, past the headers.
        check_cancelled_promptly(cancel_when(
            cancel, [&] { return server.stalled() && received.load() > 0; },
            [&] { net::http_download(server.url(), file.path, 1000, &received, &cancel); }));
        CHECK(received.load() == 7);
        CHECK_FALSE(std::filesystem::exists(hydra::os_path(file.path)));
        CHECK_FALSE(std::filesystem::exists(hydra::os_path(part)));
    }
}

TEST_CASE("http transport: a 200 reply is read in full, a 404 keeps its message") {
    net::set_fetcher({});  // the real WinHTTP transport

    // Big enough (~250 KB) to arrive over several read chunks.
    std::string body = "[";
    for (int i = 0; i < 5000; ++i) {
        if (i) body += ",";
        body += R"({"id":")" + std::to_string(i) + R"(","username":"user)" +
                std::to_string(i) + R"(","elo":1500})";
    }
    body += "]";
    {
        LoopbackServer server(http_response("200 OK", body));
        std::atomic<bool> cancel{false};  // present but never set
        std::vector<net::DmUser> users = net::fetch_users(server.url(), &cancel);
        REQUIRE(users.size() == 5000);
        CHECK(users.front().id == "0");
        CHECK(users.back().username == "user4999");
    }
    {
        LoopbackServer server(http_response("404 Not Found", ""));
        CHECK_THROWS_WITH_AS((void)net::fetch_users(server.url()),
                             "leaderboard returned HTTP 404", std::runtime_error);
    }
}

TEST_CASE("collect_dm_rows tells no current result from not in library") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    // The last scan found kHash and one more chart nobody has analyzed.
    constexpr const char* kScanned = "abcdef00112233445566778899aabbcc";
    store::ChartLibraryEntry analyzed;
    analyzed.md5 = kHash;
    analyzed.title = "Stored Title";
    store::ChartLibraryEntry scanned;
    scanned.md5 = kScanned;
    scanned.title = "Scanned Only";
    store.rebuild_chart_library({analyzed, scanned});

    std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store,
        {make_score(kHash, optimal - 1000),                        // under optimal
         make_score(kScanned, 5000),                               // in the library, no result
         make_score("00ff00ff00ff00ff00ff00ff00ff00ff", 123456)},  // never scanned
        testdm::profile());
    REQUIRE(rows.size() == 3);
    CHECK(rows[0].status == "under optimal");
    CHECK(rows[1].status == "no current result");
    CHECK(rows[2].status == "not in library");
    CHECK_FALSE(rows[1].optimal.has_value());

    const app::dm_report::DmReportStats stats = app::dm_report::tally_dm_rows(rows);
    CHECK(stats.total == 3);
    CHECK(stats.under_optimal == 1);
    CHECK(stats.at_optimal == 0);
    CHECK(stats.above_optimal == 0);
    CHECK(stats.not_analyzed == 1);
    CHECK(stats.not_in_library == 1);
}

// D51 call 11.
TEST_CASE("collect_dm_rows: a Ready record with no paths reads \"no paths\"") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    // A second chart the last scan found, analyzed at Clone Hero's cap, whose
    // analysis kept no path: a Ready record with no score.
    constexpr const char* kEmpty = "abcdef00112233445566778899aabbcc";
    testdm::save_no_paths(store, kEmpty);
    store::ChartLibraryEntry analyzed;
    analyzed.md5 = kHash;
    analyzed.title = "Stored Title";
    store::ChartLibraryEntry empty;
    empty.md5 = kEmpty;
    empty.title = "Empty Title";
    store.rebuild_chart_library({analyzed, empty});

    std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store, {make_score(kHash, optimal - 1000), make_score(kEmpty, 5000)}, testdm::profile());
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].status == "under optimal");
    CHECK(rows[1].status == "no paths");
    CHECK_FALSE(rows[1].optimal.has_value());

    const app::dm_report::DmReportStats stats = app::dm_report::tally_dm_rows(rows);
    CHECK(stats.no_paths == 1);
    CHECK(stats.not_analyzed == 0);
    const std::string phrase = app::dm_report::counts_phrase(stats);
    const std::string clause = ", 1 with no paths";
    REQUIRE(phrase.size() > clause.size());
    CHECK(hydra::ends_with(phrase, clause));

    // With no such row, the phrase has no clause (D62 item 1).
    CHECK(app::dm_report::counts_phrase(app::dm_report::tally_dm_rows({rows[0]})).find(
              "with no paths") == std::string::npos);

    // The window offers the status in its Status filter, and the Status
    // column's definition explains it.
    bool offered = false;
    for (const auto& c : app::dm_report_view::status_choices())
        if (c.status == std::optional<std::string>("no paths"))
            offered = c.label == "No paths (analyzed, none kept)";
    CHECK(offered);
    bool defined = false;
    for (const auto& c : app::dm_report_view::dm_columns())
        if (c.id == "status")
            defined = c.definition.find("No paths: analyzed, but the analysis kept no path.") !=
                      std::string::npos;
    CHECK(defined);
}

// Finding 170.
TEST_CASE("why_not_comparable names the missing Clone Hero rule") {
    using app::dm_report::why_not_comparable;
    const std::string expert = "Needs Expert: the leaderboard only has Expert scores.";
    const std::string cap =
        "Needs SP cap 4, Clone Hero's rule: the leaderboard's scores were played under it.";
    const std::string fills =
        "Needs Clone Hero 1.1 fills: untick \"1.0 fills\". The leaderboard is played on "
        "current Clone Hero.";
    CHECK(why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, false, false) == "");
    CHECK(why_not_comparable(Difficulty::Hard, kCloneHeroSpCap, false, false) == expert);
    CHECK(why_not_comparable(Difficulty::Expert, 8, false, false) == cap);
    CHECK(why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, true, false) == fills);
    // The first rule that fails names the reason.
    CHECK(why_not_comparable(Difficulty::Easy, 8, true, false) == expert);
    CHECK(why_not_comparable(Difficulty::Expert, 8, true, false) == cap);

    // The join refuses a 1.0-fills profile with the same sentence, as an
    // AlreadyPlain error, so a screen shows the sentence as it is.
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store) > 0);
    app::Settings legacy = testdm::settings();
    legacy.legacy_fills = true;
    bool refused = false;
    try {
        app::dm_report::collect_dm_rows(store, {make_score(kHash, 1)},
                                        legacy.profile_at_clone_hero_cap());
    } catch (const KindedError& e) {
        refused = true;
        CHECK(e.kind() == ErrorKind::AlreadyPlain);
        CHECK(app::plain_error(e) == fills);
    }
    CHECK(refused);
}

// D271 item 8: the toolbar asks the gate about the profile the page is built
// at, so the cap setting never refuses; the other three rules still do.
TEST_CASE("why_not_comparable for settings asks about the Clone Hero cap profile") {
    using app::dm_report::why_not_comparable;
    for (const int cap : {kCloneHeroSpCap, 6, 8}) {
        CAPTURE(cap);
        app::Settings s = testdm::settings(kMode, cap);
        CHECK(why_not_comparable(s) == "");
        s.view_difficulty = "Hard";
        CHECK(why_not_comparable(s) == "Needs Expert: the leaderboard only has Expert scores.");
        s.view_difficulty = "Expert";
        s.legacy_fills = true;
        CHECK(why_not_comparable(s) ==
              "Needs Clone Hero 1.1 fills: untick \"1.0 fills\". The leaderboard is played on "
              "current Clone Hero.");
        s.legacy_fills = false;
        s.view_noteshuffle = true;
        CHECK(why_not_comparable(s) ==
              "dmleaderboards scores don't say whether Note Shuffle was on, so they can't be "
              "compared with a shuffled path. Untick Note Shuffle to compare.");
    }
}

// D276 item 7: the cap words, and the Compare button's tooltip built from them.
TEST_CASE("clone_hero_cap_phrase and compare_tooltip name SP cap 4") {
    CHECK(app::dm_report::clone_hero_cap_phrase() == "SP cap 4, Clone Hero's rule");
    CHECK(app::dm_report::compare_tooltip() ==
          "Compare a dmleaderboards player's scores against your library at SP cap 4, Clone "
          "Hero's rule.");
}

// D104 item 6.
TEST_CASE("why_not_comparable refuses Note Shuffle, and only Note Shuffle changes") {
    using app::dm_report::why_not_comparable;
    const std::string shuffle =
        "dmleaderboards scores don't say whether Note Shuffle was on, so they can't be "
        "compared with a shuffled path. Untick Note Shuffle to compare.";
    CHECK(why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, false, true) == shuffle);
    // An earlier rule still names the reason first.
    CHECK(why_not_comparable(Difficulty::Hard, kCloneHeroSpCap, false, true) ==
          "Needs Expert: the leaderboard only has Expert scores.");
    CHECK(why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, true, true) ==
          "Needs Clone Hero 1.1 fills: untick \"1.0 fills\". The leaderboard is played on "
          "current Clone Hero.");

    // The join reads the switch back from the mode string and refuses a
    // shuffled one with the same sentence. The unshuffled mode still joins.
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store) > 0);
    bool refused = false;
    try {
        app::dm_report::collect_dm_rows(store, {make_score(kHash, 1)},
                                        testdm::profile("Expert Pro Drums, 2x Bass, Note Shuffle"));
    } catch (const KindedError& e) {
        refused = true;
        CHECK(e.kind() == ErrorKind::AlreadyPlain);
        CHECK(app::plain_error(e) == shuffle);
    }
    CHECK(refused);
    CHECK(app::dm_report::collect_dm_rows(store, {make_score(kHash, 1)}, testdm::profile())
              .size() == 1);
}

// D114 item 6 (M17): Refresh, Try again and "Compare another player..." reach
// the join without the toolbar's gate, so the join reads the difficulty back
// from the mode text and refuses a non-Expert one with the toolbar's sentence.
TEST_CASE("collect_dm_rows refuses a non-Expert chart mode with the toolbar's sentence") {
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store) > 0);
    const std::string expert = "Needs Expert: the leaderboard only has Expert scores.";
    for (const std::string mode : {"Hard Pro Drums, 2x Bass", "Easy Drums, 1x Bass"}) {
        CAPTURE(mode);
        bool refused = false;
        try {
            app::dm_report::collect_dm_rows(store, {make_score(kHash, 1)}, testdm::profile(mode));
        } catch (const KindedError& e) {
            refused = true;
            CHECK(e.kind() == ErrorKind::AlreadyPlain);
            CHECK(app::plain_error(e) == expert);
        }
        CHECK(refused);
    }
}

// M17: a mode text the parser can't read is refused, not treated as comparable.
TEST_CASE("collect_dm_rows refuses a chart mode it cannot read") {
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store) > 0);
    store::Profile unreadable = testdm::profile();
    unreadable.chartmode = "not a chart mode";
    CHECK_THROWS_AS(app::dm_report::collect_dm_rows(store, {make_score(kHash, 1)}, unreadable),
                    std::exception);
}

// ---- results the library doesn't list (D92) ---------------------------------

TEST_CASE("collect_dm_rows: a database with no chart library stops with the path report's "
          "sentence") {
    // What hydra_batch with folder arguments leaves: a result, and no library.
    store::RecordStore store(":memory:");
    testdm::save_no_paths(store, kHash);
    bool stopped = false;
    try {
        app::dm_report::collect_dm_rows(store, {make_score(kHash, 1000)}, testdm::profile());
    } catch (const KindedError& e) {
        stopped = true;
        CHECK(e.kind() == ErrorKind::AlreadyPlain);
        CHECK(app::plain_error(e) == std::string(app::report::kNoChartLibrary));
    }
    CHECK(stopped);
}

TEST_CASE("collect_dm_rows: a result whose chart the library doesn't list is left out") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    // A second chart's result, saved without a library row for it.
    constexpr const char* kOutside = "0123456789abcdef0123456789abcdef";
    testdm::save_no_paths(store, kOutside);

    const std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store, {make_score(kHash, optimal - 1), make_score(kOutside, 5000)}, testdm::profile());
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].status == "under optimal");
    // The leaderboard's score stays on the page; Hydra's result for it does
    // not join it.
    CHECK(rows[1].status == "not in library");
    CHECK_FALSE(rows[1].optimal.has_value());
}

// D115: the page lists the one profile Settings::profile_at_clone_hero_cap
// names, whatever cap the settings hold.
TEST_CASE("collect_dm_rows reads the Clone Hero cap profile by name") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    // A second listed chart whose only record is a what-if one at 8 bars.
    constexpr const char* kWhatIfOnly = "0123456789abcdef0123456789abcdef";
    const app::Settings at8 = testdm::settings(kMode, 8);
    HydraRecord whatif = testdm::sample_chart().record;
    whatif.sp_cap = 8;
    test::name_chart(store, kWhatIfOnly, "What-if Title");
    testdm::save(store, kWhatIfOnly, at8, whatif);

    // Settings at the what-if cap still list Clone Hero's cap: kHash joins its
    // cap-4 record, and the chart analyzed only at 8 bars has no current
    // result.
    const std::vector<DmReportRow> rows = app::dm_report::collect_dm_rows(
        store, {make_score(kHash, optimal - 1), make_score(kWhatIfOnly, 5000)},
        at8.profile_at_clone_hero_cap());
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].status == "under optimal");
    CHECK(rows[0].optimal == optimal);
    CHECK(rows[1].status == "no current result");

    // A profile at another cap is refused with the gate's cap sentence.
    bool refused = false;
    try {
        app::dm_report::collect_dm_rows(store, {make_score(kHash, 1)}, at8.profile());
    } catch (const KindedError& e) {
        refused = true;
        CHECK(app::plain_error(e) ==
              "Needs SP cap 4, Clone Hero's rule: the leaderboard's scores were played under it.");
    }
    CHECK(refused);
}

// D103 item 22: a settings change marks the comparison out of date only when
// it moves a setting the comparison reads.
TEST_CASE("settings_change_touches: the comparison reads its chart mode and the lens, not the cap") {
    const app::Settings before;
    CHECK_FALSE(app::dm_report::settings_change_touches(before, before));

    app::Settings mode_only = before;
    mode_only.view_difficulty = "Hard";
    mode_only.view_prodrums = false;
    REQUIRE(mode_only.chartmode_key() != before.chartmode_key());
    CHECK(app::dm_report::settings_change_touches(before, mode_only));

    // The SP cap alone: the comparison is at Clone Hero's cap.
    app::Settings cap = before;
    cap.sp_cap = 5;
    CHECK_FALSE(app::dm_report::settings_change_touches(before, cap));

    app::Settings ms_limit = before;
    ms_limit.mslimit_enabled = !before.mslimit_enabled;
    REQUIRE(ms_limit.lens() != before.lens());
    CHECK(app::dm_report::settings_change_touches(before, ms_limit));

    app::Settings fills = before;
    fills.legacy_fills = !before.legacy_fills;
    REQUIRE(fills.lens() != before.lens());
    CHECK(app::dm_report::settings_change_touches(before, fills));

    // A setting no report reads.
    app::Settings auto_open = before;
    auto_open.auto_open_report = !before.auto_open_report;
    CHECK_FALSE(app::dm_report::settings_change_touches(before, auto_open));
}
