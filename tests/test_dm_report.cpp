// Tests for app/dm_report: the score-vs-optimal join (collect_dm_rows) and
// generate_dm_report's subtitle and footer. Pins the status strings the
// comparison window's Status filter and chips key on, and the counts the app
// shows. The library side is a real store: an analyzed corpus chart, or a
// corpus chart folder scanned and analyzed the way the app does it.

#include "doctest.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"          // Settings
#include "app/display_format.h"  // format_percent, percent_steps
#include "app/library_query.h"   // parse_library_query
#include "core/model.h"          // counted, group_thousands, kCloneHeroSpCap
#include "app/dm_report.h"
#include "app/dm_report_view.h"  // dm_search_text
#include "app/report.h"          // Tile, ChipToken, kNoChartLibrary, kDash
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "core/strutil.h"        // ends_with, starts_with, to_lower_ascii
#include "corpus_util.h"
#include "display_fixtures.h"
#include "dm_fixture.h"
#include "net/dmbot_client.h"
#include "parse/song.h"
#include "store/chart_index.h"
#include "store/record_store.h"

using namespace hydra;
using app::dm_report::DmReportRow;
namespace dmr = hydra::app::dm_report;

// The store and score builders are the shared leaderboard fixture.
using testdm::fill_store;
using testdm::kHash;
using testdm::kMode;
using testdm::make_score;
using testdm::dm_tile;

namespace {

// A song id no chart in any test store has: a score on a song the library
// never listed.
const std::string kUnlistedHash = "00ff00ff00ff00ff00ff00ff00ff00ff";

// A corpus chart folder whose song.ini names carry Clone Hero rich-text tags,
// as a real charter writes them: its artist and its charter are tagged.
const std::string kTaggedChartFolder =
    "/common/Summer Blast _25 Setlist/Tier 7/SoundHaven - Focus [Ariana Grande]";

// Lists the one corpus chart in `folder` (under corpus::root()) as `store`'s
// library through the app's own scan (discover_charts, save_scan_as_library),
// and saves its analysis at depth 0 under testdm::settings(), the profile the
// comparison lists. Returns the library row the scan wrote.
store::ChartLibraryEntry scan_and_analyze(store::RecordStore& store, const std::string& folder) {
    auto [items, errors] = app::discover_charts({corpus::root() + folder});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    REQUIRE_FALSE(app::save_scan_as_library(store, items).has_value());
    app::AnalysisSettings analysis = testdm::settings().to_analysis_settings();
    analysis.depth_value = 0;
    testdm::save(store, items[0].md5, testdm::settings(),
                 corpus::analyzed(items[0].notespath, analysis));
    return items[0];
}

// The names `store`'s library lists chart `song_id` under (its naming copy).
store::ChartLibraryEntry listed_names(store::RecordStore& store, const std::string& song_id) {
    const store::ChartIndex library = store.chart_index();
    const store::ChartLibraryEntry* named = library.naming(song_id);
    REQUIRE(named != nullptr);
    return *named;
}

// A score on `song_id` whose leaderboard entry is an "unknown song" one, with
// no usable metadata, so the row takes the library's names.
net::DmScore unknown_song_score(const std::string& song_id) {
    net::DmScore s = make_score(song_id, 0);
    s.known = false;
    return s;
}

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

TEST_CASE("collect_dm_rows joins scores to records and labels them") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);
    REQUIRE_FALSE(store.chart_index().lists(kUnlistedHash));

    // Three scores around the record's own best score, and one on a song the
    // library never listed.
    const int64_t under = optimal - 1000;
    std::vector<net::DmScore> scores;
    scores.push_back(make_score(kHash, under));
    scores.push_back(make_score(kHash, optimal));
    scores.push_back(make_score(kHash, optimal + 5));
    scores.push_back(make_score(kUnlistedHash, under));

    std::vector<DmReportRow> rows =
        dmr::collect_dm_rows(store, scores, testdm::profile());
    REQUIRE(rows.size() == scores.size());

    CHECK(rows[0].status == dmr::kStatusUnderOptimal);
    CHECK(rows[1].status == dmr::kStatusAtOptimal);
    CHECK(rows[2].status == dmr::kStatusAboveOptimal);
    CHECK(rows[3].status == dmr::kStatusNotInLibrary);

    // A joined row carries the record's best score and what the score left.
    for (size_t i = 0; i < 3; ++i) {
        CAPTURE(i);
        CHECK(rows[i].optimal == optimal);
        CHECK(rows[i].delta == optimal - scores[i].score);
    }
    // The row's percent is percent_steps' whole hundredths, the number the
    // cell's text is written from.
    REQUIRE(rows[0].pct_h.has_value());
    CHECK(*rows[0].pct_h == app::percent_steps(under, optimal, dmr::kPercentDecimals));

    CHECK_FALSE(rows[3].optimal.has_value());
    CHECK_FALSE(rows[3].delta.has_value());

    // The leaderboard's own name wins over the library's when it has one.
    const store::ChartLibraryEntry listed = listed_names(store, kHash);
    REQUIRE(display_title(scores[0].song_name) != display_title(listed.title));
    CHECK(rows[0].song == display_title(scores[0].song_name));
}

TEST_CASE("collect_dm_rows: no percent off 100% speed; store identity fallback") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    // Any speed but the base one is off speed (net::is_base_speed).
    const int off_speed = net::kBaseSpeedPercent + 50;
    REQUIRE_FALSE(net::is_base_speed(off_speed));
    const net::DmScore fast = make_score(kHash, optimal - 10, off_speed);
    const net::DmScore unknown_meta = unknown_song_score(kHash);

    std::vector<DmReportRow> rows =
        dmr::collect_dm_rows(store, {fast, unknown_meta}, testdm::profile());
    REQUIRE(rows.size() == 2);

    // An off-speed score keeps Hydra's numbers but no percent of them.
    CHECK(rows[0].optimal == optimal);
    CHECK_FALSE(rows[0].pct_h.has_value());
    // An "unknown song" entry takes the library's name for its chart.
    const store::ChartLibraryEntry listed = listed_names(store, kHash);
    REQUIRE(display_title(listed.title) != display_title(unknown_meta.song_name));
    CHECK(rows[1].song == display_title(listed.title));
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
    // Every entry with an id comes through and no other, counted from the body
    // by a plain JSON read.
    const corpus::json users_json = corpus::json::parse(users_body);
    size_t with_id = 0;
    for (const corpus::json& u : users_json)
        if (u.contains("id")) ++with_id;
    REQUIRE(users.size() == with_id);
    CHECK(users[0].id == users_json[0]["id"].get<std::string>());
    CHECK(users[0].username == users_json[0]["username"].get<std::string>());
    CHECK(users[0].elo == users_json[0]["elo"].get<int>());
    CHECK(users[0].total_scores == users_json[0]["stats"]["total_scores"].get<int64_t>());
    CHECK(users[0].total_score == users_json[0]["stats"]["total_score"].get<int64_t>());
    CHECK_FALSE(users[1].elo.has_value());

    // The scores endpoint: two arrays, flattened and told apart by `known`;
    // identifiers are lowercased; charter_refs join with ", "; missing speed
    // reads the base speed.
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
    const corpus::json scores_json = corpus::json::parse(scores_body);
    const corpus::json& known_json = scores_json["scores"][0];
    REQUIRE(scores.size() == scores_json["scores"].size() + scores_json["unknown_scores"].size());
    CHECK(scores[0].identifier == to_lower_ascii(known_json["identifier"].get<std::string>()));
    CHECK(scores[0].identifier != known_json["identifier"].get<std::string>());
    CHECK(scores[0].charter == known_json["charter_refs"][0].get<std::string>() + ", " +
                                   known_json["charter_refs"][1].get<std::string>());
    CHECK(scores[0].score == known_json["score"].get<int64_t>());
    CHECK(scores[0].is_fc);
    CHECK(scores[0].speed == known_json["speed"].get<int>());
    CHECK(scores[0].rank == known_json["rank"].get<int>());
    CHECK(scores[0].known);
    CHECK(scores[1].known == false);
    CHECK(scores[1].speed == net::kBaseSpeedPercent);

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
    const int off_speed = net::kBaseSpeedPercent + 50;
    REQUIRE_FALSE(net::is_base_speed(off_speed));
    const std::vector<net::DmScore> scores = {make_score(kHash, optimal + 5),
                                              make_score(kHash, optimal + 5, off_speed),
                                              make_score(kHash, optimal - 5, off_speed)};
    const std::vector<DmReportRow> rows =
        dmr::collect_dm_rows(store, scores, testdm::profile());
    REQUIRE(rows.size() == scores.size());
    CHECK(rows[0].status == dmr::kStatusAboveOptimal);
    CHECK(rows[1].status == dmr::kStatusOtherSpeed);
    CHECK(rows[2].status == dmr::kStatusOtherSpeed);
    for (size_t i = 0; i < rows.size(); ++i) {
        CAPTURE(i);
        CHECK(rows[i].delta == optimal - scores[i].score);
        CHECK(rows[i].above_optimal == (scores[i].score > optimal));
    }

    // The Delta cell: the off-speed score over the optimal reads exactly as
    // the base-speed one does, naming how far over it is, and not as its
    // delta alone would read. The one under reads its delta.
    const std::string over_cell = dm_cell("delta", rows[0]);
    CHECK(dm_cell("delta", rows[1]) == over_cell);
    CHECK(over_cell != group_thousands(*rows[0].delta));
    CHECK(over_cell.find(group_thousands(scores[0].score - optimal)) != std::string::npos);
    CHECK(dm_cell("delta", rows[2]) == group_thousands(*rows[2].delta));
}

TEST_CASE("dm comparison: a score's search text is folded and tag-free") {
    // A real chart's names, as the scan read them from its song.ini, reach
    // the row through collect_dm_rows.
    store::RecordStore store(":memory:");
    const store::ChartLibraryEntry chart = scan_and_analyze(store, kTaggedChartFolder);
    REQUIRE(chart.charter.find('<') != std::string::npos);

    const std::vector<DmReportRow> rows =
        dmr::collect_dm_rows(store, {unknown_song_score(chart.md5)}, testdm::profile());
    REQUIRE(rows.size() == 1);
    const std::string text = app::dm_report_view::dm_search_text(rows[0]);

    // None of the names' markup is searchable.
    CHECK(text.find('<') == std::string::npos);
    // Every word of every shown name, read as the search box reads a typed
    // query, is found.
    for (const std::string& name : {rows[0].song, rows[0].artist, rows[0].charter}) {
        CAPTURE(name);
        const app::LibraryQuery query = app::parse_library_query(name);
        REQUIRE_FALSE(query.terms.empty());
        for (const app::QueryTerm& term : query.terms) {
            CAPTURE(term.folded);
            CHECK(text.find(term.folded) != std::string::npos);
        }
    }
}

TEST_CASE("generate_dm_report: tally and framing behind one seam") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 1000);

    // Each score beside the status its offset from the record's best score
    // gives.
    std::vector<net::DmScore> scores;
    std::vector<std::string> expected;
    scores.push_back(make_score(kHash, optimal - 1000));
    expected.push_back(dmr::kStatusUnderOptimal);
    scores.push_back(make_score(kHash, optimal - 10));
    expected.push_back(dmr::kStatusUnderOptimal);
    scores.push_back(make_score(kHash, optimal));
    expected.push_back(dmr::kStatusAtOptimal);
    scores.push_back(make_score(kHash, optimal + 5));
    expected.push_back(dmr::kStatusAboveOptimal);
    scores.push_back(make_score(kUnlistedHash, optimal));
    expected.push_back(dmr::kStatusNotInLibrary);

    const std::string player = "TestUser";
    const dmr::GeneratedDmReport result =
        dmr::generate_dm_report(store, scores, testdm::profile(), player);
    REQUIRE(result.rows.size() == scores.size());
    for (size_t i = 0; i < scores.size(); ++i) {
        CAPTURE(i);
        CHECK(result.rows[i].status == expected[i]);
    }

    // The tally against a recount of the rows' own statuses.
    auto count = [&](const char* status) {
        return static_cast<int>(std::count_if(result.rows.begin(), result.rows.end(),
                                              [&](const DmReportRow& r) { return r.status == status; }));
    };
    CHECK(result.stats.total == static_cast<int>(scores.size()));
    CHECK(result.stats.under_optimal == count(dmr::kStatusUnderOptimal));
    CHECK(result.stats.at_optimal == count(dmr::kStatusAtOptimal));
    CHECK(result.stats.above_optimal == count(dmr::kStatusAboveOptimal));
    CHECK(result.stats.not_analyzed == count(dmr::kStatusNoCurrentResult));
    CHECK(result.stats.not_in_library == count(dmr::kStatusNotInLibrary));
    CHECK(result.stats.other_speed == count(dmr::kStatusOtherSpeed));
    CHECK(result.stats.no_paths == count(dmr::kStatusNoPaths));

    // The subtitle names the player, counts every score and nothing by status
    // (the tiles count the scores that pass the filters, D103 item 27), and
    // ends with the cap the comparison runs at.
    CHECK(starts_with(result.subtitle, player));
    CHECK(result.subtitle.find(counted(result.stats.total, "score", "scores")) != std::string::npos);
    CHECK(ends_with(result.subtitle, dmr::clone_hero_cap_phrase()));
    // The footer names the chart mode it compared.
    CHECK(result.footer.find(kMode) != std::string::npos);

    // No scores: zero stats, no rows.
    const dmr::GeneratedDmReport none =
        dmr::generate_dm_report(store, {}, testdm::profile(), player);
    CHECK(none.stats.total == 0);
    CHECK(none.rows.empty());
}

// What a window draws from: the rows, the subtitle and footer the page shows,
// and who it compared. The footer names the mode it compared.
TEST_CASE("generate_dm_report hands over its rows, subtitle, footer, player and mode") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 1000);
    std::vector<net::DmScore> scores;
    scores.push_back(make_score(kHash, optimal - 1000));
    scores.push_back(make_score(kUnlistedHash, optimal));

    const std::string player = "TestUser";
    const dmr::GeneratedDmReport result =
        dmr::generate_dm_report(store, scores, testdm::profile(), player);
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0].status == dmr::kStatusUnderOptimal);
    CHECK(result.rows[1].status == dmr::kStatusNotInLibrary);
    CHECK(result.username == player);
    // The line ends with the cap the comparison runs at (D276 item 7), and
    // counts with the count line's noun, singular for one score.
    CHECK(result.subtitle.find(counted(2, "score", "scores")) != std::string::npos);
    CHECK(ends_with(result.subtitle, dmr::clone_hero_cap_phrase()));
    const dmr::GeneratedDmReport one =
        dmr::generate_dm_report(store, {scores[0]}, testdm::profile(), player);
    CHECK(one.subtitle.find(counted(1, "score", "scores")) != std::string::npos);
    CHECK(ends_with(one.subtitle, dmr::clone_hero_cap_phrase()));

    // The footer names the mode it compared, and its last sentence, in the
    // "no current result" words (D114 item 1), tells the player to set the
    // cap the comparison reads (D281), read from kCloneHeroSpCap.
    const std::string cap = std::to_string(kCloneHeroSpCap);
    CHECK(result.footer.find(std::string(kMode) + ".") != std::string::npos);
    const size_t last = result.footer.find(
        dmr::status_words(dmr::kStatusNoCurrentResult).count_one);
    REQUIRE(last != std::string::npos);
    const std::string last_sentence = result.footer.substr(last);
    CHECK(last_sentence.find("at SP cap " + cap) != std::string::npos);
    CHECK(last_sentence.find("set SP cap to " + cap) != std::string::npos);

    // No scores: the player is still named, and nothing else is.
    const dmr::GeneratedDmReport none =
        dmr::generate_dm_report(store, {}, testdm::profile(), player);
    CHECK(none.rows.empty());
    CHECK(none.username == player);
    CHECK(none.subtitle.empty());
    CHECK(none.footer.empty());
}

TEST_CASE("dm_tiles: the tiles count only the scores shown") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 2000);
    const int off_speed = net::kBaseSpeedPercent + 50;
    REQUIRE_FALSE(net::is_base_speed(off_speed));
    const std::vector<DmReportRow> rows = dmr::collect_dm_rows(
        store,
        {make_score(kHash, optimal - 2000), make_score(kHash, optimal - 1500),
         make_score(kHash, optimal), make_score(kHash, optimal + 5),
         make_score(kHash, optimal - 500, off_speed), make_score(kUnlistedHash, optimal)},
        testdm::profile());
    REQUIRE(rows.size() == 6);

    // The status tiles, in kStatusWords' order: every status but the ones
    // with no tile of their own (D62 item 1).
    size_t status_tiles = 0;
    for (const dmr::StatusWords& w : dmr::kStatusWords)
        if (w.tile) ++status_tiles;

    const std::vector<std::vector<size_t>> subsets = {
        {0, 1, 2, 3, 4, 5}, {0}, {0, 1}, {2, 3}, {4, 5}, {}};
    for (size_t s = 0; s < subsets.size(); ++s) {
        CAPTURE(s);
        const std::vector<size_t>& shown = subsets[s];
        const std::vector<app::report::Tile> tiles = dmr::dm_tiles(rows, shown);
        // Scores shown, the status tiles, the average and the points left.
        REQUIRE(tiles.size() == status_tiles + 3);

        // The first tile counts every shown score (D103 item 27).
        CHECK(tiles[0].value == group_thousands(static_cast<int64_t>(shown.size())));
        // Each status tile counts the shown scores with its status.
        size_t t = 1;
        for (const dmr::StatusWords& w : dmr::kStatusWords) {
            if (!w.tile) continue;
            const auto n = std::count_if(shown.begin(), shown.end(),
                                         [&](size_t i) { return rows[i].status == w.status; });
            CHECK(tiles[t].label == w.name);
            CHECK(tiles[t].value == group_thousands(static_cast<int64_t>(n)));
            ++t;
        }

        // The average is the mean of the shown rows' percents in the cells'
        // own steps (percent_steps(1, 1, ...) is 100% in them), written as a
        // cell is; with no percent shown it reads the dash. Only a score under
        // optimal leaves points on the table: the sum of those deltas.
        int64_t pct_sum = 0;
        int64_t pct_rows = 0;
        int64_t points_left = 0;
        for (size_t i : shown) {
            if (rows[i].pct_h) {
                pct_sum += *rows[i].pct_h;
                ++pct_rows;
            }
            if (rows[i].status == dmr::kStatusUnderOptimal) points_left += rows[i].delta.value();
        }
        const std::string average =
            pct_rows == 0
                ? std::string(app::report::kDash)
                : app::format_percent(pct_sum,
                                      pct_rows * app::percent_steps(1, 1, dmr::kPercentDecimals),
                                      dmr::kPercentDecimals);
        CHECK(tiles[t].value == average);
        CHECK(tiles[t + 1].value == group_thousands(points_left));
        // One shown row's average reads its own cell.
        if (shown.size() == 1 && rows[shown[0]].pct_h)
            CHECK(tiles[t].value == dm_cell("pct_h", rows[shown[0]]));
    }
}

TEST_CASE("status_token: each status's chip colour") {
    using app::report::ChipToken;
    using dmr::status_token;
    // Not in library takes the default chip, and no other status the page
    // writes does.
    for (const dmr::StatusWords& w : dmr::kStatusWords) {
        CAPTURE(w.status);
        CHECK((status_token(w.status) == status_token(dmr::kStatusNotInLibrary)) ==
              (std::string(w.status) == dmr::kStatusNotInLibrary));
    }

    // On real rows: a score compared with Hydra's optimal is coloured, under
    // and at alike, above apart from them; a score shown but not compared
    // (played off speed) is muted.
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);
    const int off_speed = net::kBaseSpeedPercent + 50;
    const std::vector<DmReportRow> rows = dmr::collect_dm_rows(
        store,
        {make_score(kHash, optimal - 5), make_score(kHash, optimal),
         make_score(kHash, optimal + 5), make_score(kHash, optimal, off_speed)},
        testdm::profile());
    REQUIRE(rows.size() == 4);
    for (size_t i = 0; i < 3; ++i) {
        CAPTURE(i);
        REQUIRE(rows[i].pct_h.has_value());
        CHECK(status_token(rows[i].status) != ChipToken::muted);
    }
    CHECK(status_token(rows[0].status) == status_token(rows[1].status));
    CHECK(status_token(rows[2].status) != status_token(rows[0].status));
    REQUIRE_FALSE(rows[3].pct_h.has_value());
    CHECK(status_token(rows[3].status) == ChipToken::muted);
}

TEST_CASE("collect_dm_rows: the stored names go through display_title, display_artist and "
          "display_charter") {
    // A real chart's names as the scan stored them from its song.ini. The
    // artist and the charter carry tags, so a row that skipped an owner shows.
    store::RecordStore store(":memory:");
    const store::ChartLibraryEntry chart = scan_and_analyze(store, kTaggedChartFolder);
    REQUIRE(display_artist(chart.artist) != chart.artist);
    REQUIRE(display_charter(chart.charter) != chart.charter);

    // The leaderboard has no metadata for it, so the row falls back to the
    // matched record's stored names.
    const std::vector<DmReportRow> rows =
        dmr::collect_dm_rows(store, {unknown_song_score(chart.md5)}, testdm::profile());
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].song == display_title(chart.title));
    CHECK(rows[0].artist == display_artist(chart.artist));
    CHECK(rows[0].charter == display_charter(chart.charter));
}

// D74 item 2: the leaderboard's own names go through the same display owners
// as the stored ones, so Clone Hero tags in DMBot's text never reach the page.
TEST_CASE("collect_dm_rows: a leaderboard row's DMBot names lose their Clone Hero tags") {
    // The board names a real chart the way its song.ini does, tags and all.
    store::RecordStore store(":memory:");
    const store::ChartLibraryEntry chart = scan_and_analyze(store, kTaggedChartFolder);
    REQUIRE(display_artist(chart.artist) != chart.artist);
    REQUIRE(display_charter(chart.charter) != chart.charter);

    net::DmScore tagged = make_score(chart.md5, 0);
    tagged.song_name = chart.title;
    tagged.artist = chart.artist;
    tagged.charter = chart.charter;

    // The same names on a score the library doesn't have, so no stored record
    // can stand in for them.
    net::DmScore tagged_unmatched = tagged;
    tagged_unmatched.identifier = kUnlistedHash;
    REQUIRE_FALSE(store.chart_index().lists(kUnlistedHash));

    std::vector<DmReportRow> rows =
        dmr::collect_dm_rows(store, {tagged, tagged_unmatched}, testdm::profile());
    REQUIRE(rows.size() == 2);
    for (const DmReportRow& r : rows) {
        CHECK(r.song == display_title(chart.title));
        CHECK(r.artist == display_artist(chart.artist));
        CHECK(r.charter == display_charter(chart.charter));
    }
}

TEST_CASE("collect_dm_rows: a leaderboard row with no charter shows the library's, else "
          "(unknown) (D266 item 2)") {
    store::RecordStore store(":memory:");
    const store::ChartLibraryEntry chart = scan_and_analyze(store, kTaggedChartFolder);
    REQUIRE(display_charter(chart.charter) != kUnknownTitle);

    // DMBot knows the song but sent no charter; the library's chart lends its.
    net::DmScore no_charter = make_score(chart.md5, 0);
    no_charter.charter.clear();

    // The same score on a song the library doesn't have: nothing to lend.
    net::DmScore no_charter_unmatched = no_charter;
    no_charter_unmatched.identifier = kUnlistedHash;
    REQUIRE_FALSE(store.chart_index().lists(kUnlistedHash));

    const std::vector<DmReportRow> rows = dmr::collect_dm_rows(
        store, {no_charter, no_charter_unmatched}, testdm::profile());
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].charter == display_charter(chart.charter));
    CHECK(rows[1].charter == kUnknownTitle);
}

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
    // up, so the cell reads 99.01%.
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
