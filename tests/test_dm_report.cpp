// Tests for app/dm_report: the score-vs-optimal join (collect_dm_rows) and
// generate_dm_report's subtitle and footer. Pins the status strings the
// comparison window's Status filter and chips key on, and the counts the app
// shows. The library side is a real store: an analyzed corpus chart, or a
// corpus chart folder scanned and analyzed the way the app does it.

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <filesystem>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>
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
#include "net/http.h"
#include "parse/song.h"
#include "search/graph.h"     // fill_rule_for, fill_rule_name
#include "store/chart_index.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "wait_util.h"

using namespace hydra;
using app::dm_report::DmReportRow;
namespace dmr = hydra::app::dm_report;

// The store and score builders are the shared leaderboard fixture.
using testdm::fill_store;
using testdm::kHash;
using testdm::kMode;
using testdm::make_score;

namespace {

// A song id no chart in any test store has: a score on a song the library
// never listed.
const std::string kUnlistedHash = "00ff00ff00ff00ff00ff00ff00ff00ff";

// A corpus chart folder whose song.ini names carry Clone Hero rich-text tags,
// as a real charter writes them: its artist and its charter are tagged.
const std::string kTaggedChartFolder =
    "/common/Summer Blast _25 Setlist/Tier 7/SoundHaven - Focus [Ariana Grande]";

// A second corpus chart from the same setlist, for a library of two.
const std::string kSecondChartFolder =
    "/common/Summer Blast _25 Setlist/Tier 7/Kinglet - Blister";

// The one corpus chart in `folder` (under corpus::root()), as the app's own
// scan reads it (discover_charts).
store::ChartLibraryEntry discover_one(const std::string& folder) {
    auto [items, errors] = app::discover_charts({corpus::root() + folder});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    return items[0];
}

// Saves `charts` as `store`'s library, the way a finished scan does.
void list_as_library(store::RecordStore& store,
                     const std::vector<store::ChartLibraryEntry>& charts) {
    REQUIRE_FALSE(app::save_scan_as_library(store, charts).has_value());
}

// Analyzes `chart` at depth 0 under `s` and saves the record under
// s.profile(), as the corpus analysis does.
void save_analysis(store::RecordStore& store, const store::ChartLibraryEntry& chart,
                   const app::Settings& s) {
    app::AnalysisSettings analysis = s.to_analysis_settings();
    analysis.depth_value = 0;
    testdm::save(store, chart.md5, s, corpus::analyzed(chart.notespath, analysis));
}

// Lists the one corpus chart in `folder` as `store`'s library through the
// app's own scan, and saves its analysis under testdm::settings(), the profile
// the comparison lists. Returns the library row the scan wrote.
store::ChartLibraryEntry scan_and_analyze(store::RecordStore& store, const std::string& folder) {
    const store::ChartLibraryEntry chart = discover_one(folder);
    list_as_library(store, {chart});
    save_analysis(store, chart, testdm::settings());
    return chart;
}

// The status a base-speed score should read, decided from two reads apart
// from collect_dm_rows' joined one: the library's own list
// (ChartIndex::lists) and the store's read of that one chart under `profile`
// (lookup_one).
std::string expected_status(store::RecordStore& store, const store::Profile& profile,
                            const net::DmScore& score) {
    if (!store.chart_index().lists(score.identifier)) return dmr::kStatusNotInLibrary;
    const store::RecordLookup found = store.lookup_one(score.identifier, profile);
    if (found.status != store::RecordStatus::Ready) return dmr::kStatusNoCurrentResult;
    if (!found.record.has_scored_best_path()) return dmr::kStatusNoPaths;
    const int64_t best = *found.record.summary().score;
    if (score.score > best) return dmr::kStatusAboveOptimal;
    return score.score == best ? dmr::kStatusAtOptimal : dmr::kStatusUnderOptimal;
}

// Each row reads expected_status for its score.
void check_expected_statuses(store::RecordStore& store, const store::Profile& profile,
                             const std::vector<net::DmScore>& scores,
                             const std::vector<DmReportRow>& rows) {
    REQUIRE(rows.size() == scores.size());
    for (size_t i = 0; i < rows.size(); ++i) {
        CAPTURE(i);
        CHECK(rows[i].status == expected_status(store, profile, scores[i]));
    }
}

// The tally counts every row once, in its own status's bucket.
void check_tally(const std::vector<DmReportRow>& rows) {
    const dmr::DmReportStats stats = dmr::tally_dm_rows(rows);
    CHECK(stats.total == static_cast<int>(rows.size()));
    for (const dmr::StatusWords& w : dmr::kStatusWords) {
        CAPTURE(w.status);
        const auto n = std::count_if(rows.begin(), rows.end(),
                                     [&](const DmReportRow& r) { return r.status == w.status; });
        CHECK(stats.*w.count == static_cast<int>(n));
    }
}

// The sentence the join refuses with: the plain text of the AlreadyPlain
// error collect_dm_rows throws for `profile`, or empty when it throws none.
std::string join_refusal(store::RecordStore& store, const store::Profile& profile) {
    try {
        dmr::collect_dm_rows(store, {make_score(kHash, 1)}, profile);
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::AlreadyPlain);
        return app::plain_error(e);
    }
    return {};
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
    // Both arrays' entries, each one row.
    const size_t entries = scores_json["scores"].size() + scores_json["unknown_scores"].size();
    REQUIRE(scores.size() == entries);
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
    REQUIRE(optimal > 0);
    const int off_speed = net::kBaseSpeedPercent + 50;
    REQUIRE_FALSE(net::is_base_speed(off_speed));
    // Two scores under the optimal by different amounts, one at it, one over
    // it, one off speed and one on a song the library never listed.
    const std::vector<DmReportRow> rows = dmr::collect_dm_rows(
        store,
        {make_score(kHash, optimal - optimal / 100), make_score(kHash, optimal - optimal / 200),
         make_score(kHash, optimal), make_score(kHash, optimal + 5),
         make_score(kHash, optimal - optimal / 400, off_speed), make_score(kUnlistedHash, optimal)},
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

// Scores at fractions of `optimal` in thousandths: at it, close under it,
// at half and near zero, so the percents run from 100% down.
std::vector<net::DmScore> scores_across(int64_t optimal) {
    std::vector<net::DmScore> scores;
    scores.push_back(make_score(kHash, optimal));
    scores.push_back(make_score(kHash, optimal - 1));
    for (const int64_t thousandths : {999, 995, 990, 500, 1})
        scores.push_back(make_score(kHash, optimal * thousandths / 1000));
    return scores;
}

TEST_CASE("dm comparison: the average tile reads a percent the way the cells do") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);
    const std::vector<DmReportRow> rows =
        dmr::collect_dm_rows(store, scores_across(optimal), testdm::profile());

    // The status tiles sit between the scores-shown tile and the average
    // (dm_tiles' order), so the average is the tile after them.
    size_t status_tiles = 0;
    for (const dmr::StatusWords& w : dmr::kStatusWords)
        if (w.tile) ++status_tiles;

    // The cell's text and the tile's input both come from the row's whole
    // hundredths, so each score shown alone has an average tile that reads
    // its own cell.
    for (size_t i = 0; i < rows.size(); ++i) {
        CAPTURE(i);
        REQUIRE(rows[i].pct_h.has_value());
        const std::vector<app::report::Tile> tiles = dmr::dm_tiles(rows, {i});
        REQUIRE(tiles.size() == status_tiles + 3);
        CHECK(tiles[status_tiles + 1].value == dm_cell("pct_h", rows[i]));
    }
}

TEST_CASE("collect_dm_rows: the % of opt column sorts on the shown hundredths") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);
    const std::vector<DmReportRow> rows =
        dmr::collect_dm_rows(store, scores_across(optimal), testdm::profile());

    // The case the rule is for: the score at optimal and the one a point under
    // it read the same text, and the score at half reads another.
    REQUIRE(rows.size() > 2);
    REQUIRE(rows[0].actual != rows[1].actual);
    REQUIRE(dm_cell("pct_h", rows[0]) == dm_cell("pct_h", rows[1]));
    REQUIRE(dm_cell("pct_h", rows[0]) != dm_cell("pct_h", rows.back()));

    // Over every pair of rows: the same text sorts as a tie, and the row with
    // fewer shown hundredths sorts below.
    for (size_t i = 0; i < rows.size(); ++i) {
        for (size_t j = 0; j < rows.size(); ++j) {
            CAPTURE(i);
            CAPTURE(j);
            REQUIRE(rows[i].pct_h.has_value());
            REQUIRE(rows[j].pct_h.has_value());
            const double key_i = std::get<double>(dm_sort_key("pct_h", rows[i]));
            const double key_j = std::get<double>(dm_sort_key("pct_h", rows[j]));
            CHECK((dm_cell("pct_h", rows[i]) == dm_cell("pct_h", rows[j])) == (key_i == key_j));
            CHECK((*rows[i].pct_h < *rows[j].pct_h) == (key_i < key_j));
        }
    }
}

TEST_CASE("collect_dm_rows tells no current result from not in library") {
    // A real library of two scanned corpus charts, one of them analyzed, and a
    // song id the scan never found.
    store::RecordStore store(":memory:");
    const store::ChartLibraryEntry analyzed = discover_one(kTaggedChartFolder);
    const store::ChartLibraryEntry scanned = discover_one(kSecondChartFolder);
    list_as_library(store, {analyzed, scanned});
    save_analysis(store, analyzed, testdm::settings());
    const store::Profile profile = testdm::profile();

    const std::vector<net::DmScore> scores = {make_score(analyzed.md5, 1),
                                              make_score(scanned.md5, 1),
                                              make_score(kUnlistedHash, 1)};
    // Both buckets the test tells apart are reached.
    REQUIRE(expected_status(store, profile, scores[1]) == dmr::kStatusNoCurrentResult);
    REQUIRE(expected_status(store, profile, scores[2]) == dmr::kStatusNotInLibrary);

    const std::vector<DmReportRow> rows = dmr::collect_dm_rows(store, scores, profile);
    check_expected_statuses(store, profile, scores, rows);
    CHECK_FALSE(rows[1].optimal.has_value());
    CHECK_FALSE(rows[2].optimal.has_value());
    check_tally(rows);
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

    std::vector<DmReportRow> rows = dmr::collect_dm_rows(
        store, {make_score(kHash, optimal - 1000), make_score(kEmpty, 5000)}, testdm::profile());
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].status == dmr::kStatusUnderOptimal);
    CHECK(rows[1].status == dmr::kStatusNoPaths);
    CHECK_FALSE(rows[1].optimal.has_value());

    const dmr::DmReportStats stats = dmr::tally_dm_rows(rows);
    CHECK(stats.no_paths == 1);
    CHECK(stats.not_analyzed == 0);
    // The counts phrase ends with the status's own count words for one.
    const dmr::StatusWords& no_paths = dmr::status_words(dmr::kStatusNoPaths);
    REQUIRE(no_paths.count_when_any);
    const std::string clause =
        std::string(", ") + counted(stats.no_paths, no_paths.count_one, no_paths.count_one);
    const std::string phrase = dmr::counts_phrase(stats);
    REQUIRE(phrase.size() > clause.size());
    CHECK(hydra::ends_with(phrase, clause));

    // With no such row, the phrase has no clause (D62 item 1).
    CHECK(dmr::counts_phrase(dmr::tally_dm_rows({rows[0]})).find(no_paths.count_one) ==
          std::string::npos);

    // Every status the table lists, this one among them, is offered in the
    // window's Status filter under its name, and the Status column's
    // definition explains each one that has a gloss. A gloss's text before
    // its first placeholder is the part the definition shows as written.
    std::string definition;
    for (const auto& c : app::dm_report_view::dm_columns())
        if (c.id == "status") definition = c.definition;
    REQUIRE_FALSE(definition.empty());
    const std::vector<app::dm_report_view::StatusChoice> choices =
        app::dm_report_view::status_choices();
    for (const dmr::StatusWords& w : dmr::kStatusWords) {
        CAPTURE(w.status);
        const auto offered = std::find_if(choices.begin(), choices.end(), [&](const auto& c) {
            return c.status == std::optional<std::string>(w.status);
        });
        REQUIRE(offered != choices.end());
        CHECK(starts_with(offered->label, w.name));
        const std::string gloss = w.gloss;
        if (gloss.empty()) continue;
        const std::string shown = gloss.substr(0, gloss.find("__"));
        CHECK(definition.find(std::string(w.name) + ": " + shown) != std::string::npos);
    }
}

// The gate's answer for each rule moved off the value the leaderboard plays
// at, one at a time, from the four-argument owner.
struct GateAnswers {
    std::string non_expert;
    std::string other_cap;
    std::string legacy_fills;
    std::string shuffle;
};
GateAnswers gate_answers() {
    using dmr::why_not_comparable;
    GateAnswers a;
    a.non_expert = why_not_comparable(Difficulty::Hard, kCloneHeroSpCap, false, false);
    a.other_cap = why_not_comparable(Difficulty::Expert, kCloneHeroSpCap + 1, false, false);
    a.legacy_fills = why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, true, false);
    a.shuffle = why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, false, true);
    return a;
}

TEST_CASE("why_not_comparable names the missing Clone Hero rule") {
    using dmr::why_not_comparable;
    CHECK(why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, false, false).empty());

    // Each rule broken alone refuses, each with its own sentence, which names
    // the value the leaderboard needs.
    const GateAnswers a = gate_answers();
    const std::vector<std::string> sentences = {a.non_expert, a.other_cap, a.legacy_fills,
                                                a.shuffle};
    for (size_t i = 0; i < sentences.size(); ++i) {
        CAPTURE(i);
        CHECK_FALSE(sentences[i].empty());
        for (size_t j = i + 1; j < sentences.size(); ++j) CHECK(sentences[i] != sentences[j]);
    }
    CHECK(a.non_expert.find(difficulty_name(Difficulty::Expert)) != std::string::npos);
    CHECK(a.other_cap.find(dmr::clone_hero_cap_phrase()) != std::string::npos);
    CHECK(a.legacy_fills.find(fill_rule_name(fill_rule_for(false), FillRuleNameStyle::Long)) !=
          std::string::npos);

    // Every difficulty but Expert, and every cap but Clone Hero's, refuses
    // with the same sentence.
    for (const Difficulty d : kAllDifficulties) {
        CAPTURE(difficulty_name(d));
        CHECK(why_not_comparable(d, kCloneHeroSpCap, false, false) ==
              (d == Difficulty::Expert ? std::string() : a.non_expert));
    }
    for (const int cap : {kCloneHeroSpCap - 1, kCloneHeroSpCap + 1, kCloneHeroSpCap + 4}) {
        CAPTURE(cap);
        CHECK(why_not_comparable(Difficulty::Expert, cap, false, false) == a.other_cap);
    }

    // The first rule that fails names the reason, in the order Expert, cap,
    // fills, Note Shuffle.
    CHECK(why_not_comparable(Difficulty::Easy, kCloneHeroSpCap + 1, true, true) == a.non_expert);
    CHECK(why_not_comparable(Difficulty::Expert, kCloneHeroSpCap + 1, true, true) == a.other_cap);
    CHECK(why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, true, true) == a.legacy_fills);

    // The join refuses a 1.0-fills profile with the same sentence, as an
    // AlreadyPlain error, so a screen shows the sentence as it is.
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store) > 0);
    app::Settings legacy = testdm::settings();
    legacy.legacy_fills = true;
    CHECK(join_refusal(store, legacy.profile_at_clone_hero_cap()) == a.legacy_fills);
}

// D271 item 8: the toolbar asks the gate about the profile the page is built
// at, so the cap setting never refuses; the other three rules still do.
TEST_CASE("why_not_comparable for settings asks about the Clone Hero cap profile") {
    using dmr::why_not_comparable;
    const GateAnswers a = gate_answers();
    for (const int cap : {kCloneHeroSpCap - 1, kCloneHeroSpCap, kCloneHeroSpCap + 2,
                          kCloneHeroSpCap + 4}) {
        CAPTURE(cap);
        // At any cap the settings form answers as the owner does at Clone
        // Hero's cap, for each of the other three rules.
        app::Settings s = testdm::settings(kMode, cap);
        CHECK(why_not_comparable(s).empty());
        s.view_difficulty = difficulty_name(Difficulty::Hard);
        CHECK(why_not_comparable(s) == a.non_expert);
        s.view_difficulty = difficulty_name(Difficulty::Expert);
        s.legacy_fills = true;
        CHECK(why_not_comparable(s) == a.legacy_fills);
        s.legacy_fills = false;
        s.view_noteshuffle = true;
        CHECK(why_not_comparable(s) == a.shuffle);
    }
}

// D276 item 7: the cap words, and the Compare button's tooltip built from them.
TEST_CASE("clone_hero_cap_phrase and compare_tooltip name Clone Hero's SP cap") {
    const std::string phrase = dmr::clone_hero_cap_phrase();
    CHECK(phrase.find("SP cap " + std::to_string(kCloneHeroSpCap)) != std::string::npos);
    // The tooltip ends with the phrase, and the gate's cap sentence says it.
    CHECK(ends_with(dmr::compare_tooltip(), phrase + "."));
    CHECK(gate_answers().other_cap.find(phrase) != std::string::npos);
}

// D104 item 6.
TEST_CASE("why_not_comparable refuses Note Shuffle, and only Note Shuffle changes") {
    using dmr::why_not_comparable;
    const GateAnswers a = gate_answers();
    // An earlier rule still names the reason first.
    CHECK(why_not_comparable(Difficulty::Hard, kCloneHeroSpCap, false, true) == a.non_expert);
    CHECK(why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, true, true) == a.legacy_fills);

    // The settings with Note Shuffle ticked and nothing else changed: the
    // toolbar's gate and the join both refuse them with the shuffle sentence,
    // the join reading the switch back from the profile's mode text. The
    // unshuffled settings still join.
    app::Settings shuffled = testdm::settings();
    shuffled.view_noteshuffle = true;
    REQUIRE(shuffled.profile_at_clone_hero_cap().chartmode != testdm::profile().chartmode);
    CHECK(why_not_comparable(shuffled) == a.shuffle);
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store) > 0);
    CHECK(join_refusal(store, shuffled.profile_at_clone_hero_cap()) == a.shuffle);
    CHECK(dmr::collect_dm_rows(store, {make_score(kHash, 1)}, testdm::profile()).size() == 1);
}

// D114 item 6 (M17): Refresh, Try again and "Compare another player..." reach
// the join without the toolbar's gate, so the join reads the difficulty back
// from the mode text and refuses a non-Expert one with the toolbar's sentence.
TEST_CASE("collect_dm_rows refuses a non-Expert chart mode with the toolbar's sentence") {
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store) > 0);
    for (const Difficulty d : kAllDifficulties) {
        if (d == Difficulty::Expert) continue;
        for (const bool prodrums : {true, false}) {
            CAPTURE(difficulty_name(d));
            CAPTURE(prodrums);
            app::Settings s = testdm::settings();
            s.view_difficulty = difficulty_name(d);
            s.view_prodrums = prodrums;
            REQUIRE(s.difficulty() == d);
            const std::string sentence =
                dmr::why_not_comparable(d, kCloneHeroSpCap, s.legacy_fills, s.view_noteshuffle);
            REQUIRE_FALSE(sentence.empty());
            CHECK(join_refusal(store, s.profile_at_clone_hero_cap()) == sentence);
        }
    }
}

// M17: a mode text the parser can't read is refused, not treated as comparable.
TEST_CASE("collect_dm_rows refuses a chart mode it cannot read") {
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store) > 0);
    store::Profile unreadable = testdm::profile();
    unreadable.chartmode = "not a chart mode";
    // The refusal is the mode reader's own (Settings::with_known_chartmode).
    std::string owner_says;
    try {
        (void)app::Settings{}.with_known_chartmode(unreadable.chartmode);
    } catch (const std::runtime_error& e) {
        owner_says = e.what();
    }
    REQUIRE_FALSE(owner_says.empty());
    CHECK(owner_says.find(unreadable.chartmode) != std::string::npos);
    bool refused = false;
    try {
        dmr::collect_dm_rows(store, {make_score(kHash, 1)}, unreadable);
    } catch (const std::runtime_error& e) {
        refused = true;
        CHECK(std::string(e.what()) == owner_says);
    }
    CHECK(refused);
}

// ---- results the library doesn't list (D92) ---------------------------------

TEST_CASE("collect_dm_rows: a database with no chart library stops with the path report's "
          "sentence") {
    // What hydra_batch with folder arguments leaves: a corpus chart's result,
    // and no library.
    store::RecordStore store(":memory:");
    const store::ChartLibraryEntry chart = discover_one(kTaggedChartFolder);
    save_analysis(store, chart, testdm::settings());
    REQUIRE(store.lookup_one(chart.md5, testdm::profile()).status == store::RecordStatus::Ready);
    bool stopped = false;
    try {
        dmr::collect_dm_rows(store, {make_score(chart.md5, 1000)}, testdm::profile());
    } catch (const KindedError& e) {
        stopped = true;
        CHECK(e.kind() == ErrorKind::AlreadyPlain);
        CHECK(app::plain_error(e) == std::string(app::report::kNoChartLibrary));
    }
    CHECK(stopped);
}

TEST_CASE("collect_dm_rows: a result whose chart the library doesn't list is left out") {
    // A library of one scanned corpus chart, and a second chart's result saved
    // with no library row for it, as when its folder left the library.
    store::RecordStore store(":memory:");
    const store::ChartLibraryEntry listed = discover_one(kTaggedChartFolder);
    const store::ChartLibraryEntry outside = discover_one(kSecondChartFolder);
    list_as_library(store, {listed});
    save_analysis(store, listed, testdm::settings());
    save_analysis(store, outside, testdm::settings());
    const store::Profile profile = testdm::profile();
    REQUIRE(store.lookup_one(outside.md5, profile).status == store::RecordStatus::Ready);

    const std::vector<net::DmScore> scores = {make_score(listed.md5, 1),
                                              make_score(outside.md5, 1)};
    REQUIRE(expected_status(store, profile, scores[1]) == dmr::kStatusNotInLibrary);
    const std::vector<DmReportRow> rows = dmr::collect_dm_rows(store, scores, profile);
    // The leaderboard's score stays on the page; Hydra's result for it does
    // not join it.
    check_expected_statuses(store, profile, scores, rows);
    CHECK_FALSE(rows[1].optimal.has_value());
}

// D115: the page lists the one profile Settings::profile_at_clone_hero_cap
// names, whatever cap the settings hold.
TEST_CASE("collect_dm_rows reads the Clone Hero cap profile by name") {
    // Two scanned corpus charts: the first analyzed at Clone Hero's cap and
    // at a what-if cap, the second only at the what-if cap.
    store::RecordStore store(":memory:");
    const store::ChartLibraryEntry both = discover_one(kTaggedChartFolder);
    const store::ChartLibraryEntry whatif_only = discover_one(kSecondChartFolder);
    list_as_library(store, {both, whatif_only});
    const app::Settings whatif = testdm::settings(kMode, kCloneHeroSpCap * 2);
    save_analysis(store, both, testdm::settings());
    save_analysis(store, both, whatif);
    save_analysis(store, whatif_only, whatif);
    REQUIRE(store.lookup_one(whatif_only.md5, whatif.profile()).status ==
            store::RecordStatus::Ready);

    // Settings at the what-if cap still list Clone Hero's cap: each row reads
    // what that profile's records give it, so the chart analyzed only at the
    // what-if cap has no current result.
    const store::Profile listed = whatif.profile_at_clone_hero_cap();
    const std::vector<net::DmScore> scores = {make_score(both.md5, 1),
                                              make_score(whatif_only.md5, 1)};
    REQUIRE(expected_status(store, listed, scores[1]) == dmr::kStatusNoCurrentResult);
    const std::vector<DmReportRow> rows = dmr::collect_dm_rows(store, scores, listed);
    check_expected_statuses(store, listed, scores, rows);
    const store::RecordLookup at_ch_cap = store.lookup_one(both.md5, listed);
    REQUIRE(at_ch_cap.status == store::RecordStatus::Ready);
    CHECK(rows[0].optimal == at_ch_cap.record.summary().score);

    // A profile at another cap is refused with the gate's cap sentence.
    const std::string sentence = dmr::why_not_comparable(
        whatif.difficulty(), whatif.sp_cap, whatif.legacy_fills, whatif.view_noteshuffle);
    REQUIRE_FALSE(sentence.empty());
    CHECK(join_refusal(store, whatif.profile()) == sentence);
}

// D103 item 22: a settings change marks the comparison out of date only when
// it moves a setting the comparison reads. Each analysis setting
// (kAnalysisFields) is changed alone, the way the settings reset test does.
TEST_CASE("settings_change_touches: every analysis setting but the SP cap marks the comparison "
          "out of date") {
    using app::KeyPart;
    const app::Settings before;
    CHECK_FALSE(dmr::settings_change_touches(before, before));

    struct Change {
        app::Settings& t;
        void operator()(bool app::Settings::* m) const { t.*m = !(t.*m); }
        void operator()(int app::Settings::* m) const { t.*m += 1; }
        void operator()(std::string app::Settings::* m) const {
            t.*m = t.*m == difficulty_name(Difficulty::Hard) ? difficulty_name(Difficulty::Expert)
                                                             : difficulty_name(Difficulty::Hard);
        }
    };
    for (const app::AnalysisField& f : app::kAnalysisFields) {
        app::Settings after = before;
        std::visit(Change{after}, f.member);
        CAPTURE(static_cast<int>(f.setting));
        REQUIRE(after.profile() != before.profile());
        CHECK(dmr::settings_change_touches(before, after) == (f.part != KeyPart::Cap));
    }

    // A setting no report reads.
    app::Settings auto_open = before;
    auto_open.auto_open_report = !before.auto_open_report;
    CHECK_FALSE(dmr::settings_change_touches(before, auto_open));
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
    CHECK(r.error == Cancelled().what());
    CHECK(*r.cancel_to_return_ms < 500);
}

}  // namespace

TEST_CASE("http transport: cancel aborts a request the server never answers") {
    net::set_fetcher({});  // the real WinHTTP transport
    LoopbackServer server("", /*stall=*/true);

    // Set before the call: fails at once, before any connection is made.
    std::atomic<bool> cancel{true};
    const std::string cancelled_text = Cancelled().what();
    CHECK_THROWS_WITH_AS((void)net::fetch_users(server.url(), &cancel), cancelled_text.c_str(),
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
        const std::string body_start = "[{\"id\":";
        LoopbackServer server(stalled_body_response(body_start), /*stall=*/true);
        testtemp::ScopedFile file("dm_http_stall", ".bin");
        const std::string part = file.path + ".part";
        std::atomic<uint64_t> received{0};
        std::atomic<bool> cancel{false};
        // The first body bytes reaching the file prove the call is in its body
        // read, past the headers.
        check_cancelled_promptly(cancel_when(
            cancel, [&] { return server.stalled() && received.load() > 0; },
            [&] { net::http_download(server.url(), file.path, 1000, &received, &cancel); }));
        CHECK(received.load() == body_start.size());
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
