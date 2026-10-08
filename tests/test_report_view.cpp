// The report windows' shared table (app/report_view.h) and each report's
// columns, cells and keep-rules (app/path_report_view.h,
// app/dm_report_view.h). The literals are what the HTML pages showed for the
// same rows.

#include "doctest.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/display_format.h"  // percent_steps
#include "app/dm_report_view.h"
#include "app/path_report_view.h"
#include "app/report.h"           // tier_for
#include "app/report_view.h"
#include "core/squeeze_rating.h"  // timing_tiers

using namespace hydra;
using namespace hydra::app;
using report::ReportRow;
using dm_report::DmReportRow;
using report_view::SortDir;
using report_view::SortKey;
using report_view::SortSpec;
using report_view::TableView;
using report_view::Tone;

namespace {

constexpr double kWindow = 85.0;  // the default hit window; Beyond starts at 170 ms

ReportRow path_row(std::string song, std::string artist, std::string charter,
                   std::string path, int64_t score, std::optional<double> ms,
                   bool optimal) {
    ReportRow r;
    r.song = std::move(song);
    r.artist = std::move(artist);
    r.charter = std::move(charter);
    r.mode = "Expert Pro Drums";
    r.path = std::move(path);
    r.score = score;
    r.optimal = optimal;
    r.ms = ms;
    std::tie(r.tier, r.tok) = report::tier_for(ms, kWindow);
    return r;
}

std::vector<std::string> path_texts(const std::vector<ReportRow>& rows) {
    std::vector<std::string> out;
    for (const ReportRow& r : rows) out.push_back(path_report_view::path_search_text(r));
    return out;
}

std::vector<std::string> dm_texts(const std::vector<DmReportRow>& rows) {
    std::vector<std::string> out;
    for (const DmReportRow& r : rows) out.push_back(dm_report_view::dm_search_text(r));
    return out;
}

TableView<ReportRow> path_view(std::vector<ReportRow> rows) {
    std::vector<std::string> texts = path_texts(rows);
    return TableView<ReportRow>(std::move(rows), std::move(texts),
                                path_report_view::path_columns(kWindow));
}

TableView<DmReportRow> dm_view(std::vector<DmReportRow> rows) {
    std::vector<std::string> texts = dm_texts(rows);
    return TableView<DmReportRow>(std::move(rows), std::move(texts),
                                  dm_report_view::dm_columns());
}

// The visible rows' songs, in order.
template <class Row>
std::vector<std::string> songs(const TableView<Row>& view) {
    std::vector<std::string> out;
    for (size_t i : view.visible()) out.push_back(view.rows()[i].song);
    return out;
}

template <class Row>
std::vector<std::string> ids(const std::vector<report_view::Column<Row>>& cols) {
    std::vector<std::string> out;
    for (const auto& c : cols) out.push_back(c.id);
    return out;
}

template <class Row>
std::vector<std::string> titles(const std::vector<report_view::Column<Row>>& cols) {
    std::vector<std::string> out;
    for (const auto& c : cols) out.push_back(c.title);
    return out;
}

template <class Row>
std::vector<std::string> cells(const std::vector<report_view::Column<Row>>& cols,
                               const Row& row) {
    std::vector<std::string> out;
    for (const auto& c : cols) out.push_back(c.cell(row));
    return out;
}

// Four scores, one of each kind of cell the comparison draws.
std::vector<DmReportRow> dm_sample() {
    std::vector<DmReportRow> rows(4);
    DmReportRow& under = rows[0];
    under.song = "Under";
    under.artist = "Band A";
    under.charter = "Charter A";
    under.actual = 98000;
    under.optimal = 100000;
    under.delta = 2000;
    under.pct_h = percent_steps(98000, 100000, dm_report::kPercentDecimals);
    under.percent = 99;
    under.speed = 100;
    under.rank = 3;
    under.posted = "2024-05-01T12:00:00Z";
    under.status = "under optimal";

    DmReportRow& above = rows[1];
    above.song = "Above";
    above.artist = "Band B";
    above.charter = "Charter B";
    above.actual = 101234;
    above.optimal = 100000;
    above.delta = -1234;
    above.pct_h = percent_steps(101234, 100000, dm_report::kPercentDecimals);
    above.is_fc = true;
    above.percent = 100;
    above.speed = 100;
    above.rank = 1;
    above.posted = "2023-01-02T00:00:00Z";
    above.status = "above optimal";
    above.above_optimal = true;

    DmReportRow& missing = rows[2];
    missing.song = "Missing";
    missing.artist = "Band C";
    missing.actual = 5000;
    missing.percent = 80;
    missing.speed = 100;
    missing.status = "not in library";

    DmReportRow& slow = rows[3];
    slow.song = "Fast";
    slow.artist = "Band D";
    slow.charter = "Charter D";
    slow.actual = 100050;
    slow.optimal = 100000;
    slow.delta = -50;
    slow.percent = 100;
    slow.speed = 150;
    slow.rank = 12;
    slow.posted = "2025-12-31T23:59:59Z";
    slow.status = "other speed";
    slow.above_optimal = true;
    return rows;
}

}  // namespace

TEST_CASE("report view search: folded words in any order, nothing more (D56 item 1)") {
    using report_view::search_matches;
    using report_view::search_words;
    CHECK(search_words("  Beyonc\xC3\xA9   HALO ") == std::vector<std::string>{"beyonce", "halo"});
    // Tabs and the no-break space split words like a space.
    CHECK(search_words("halo\tbeyonce") == std::vector<std::string>{"halo", "beyonce"});
    CHECK(search_words("halo\xC2\xA0" "beyonce") == std::vector<std::string>{"halo", "beyonce"});
    CHECK(search_words("").empty());
    CHECK(search_words("   ").empty());

    const std::string text = path_report_view::path_search_text(
        path_row("Halo", "Beyonc\xC3\xA9", "Someone", "2(+) E0 1", 100, std::nullopt, true));
    CHECK(search_matches(search_words(""), text));
    CHECK(search_matches(search_words("BEYONCE halo"), text));
    CHECK(search_matches(search_words("halo beyonc\xC3\xA9"), text));
    CHECK(search_matches(search_words("2(+)"), text));
    CHECK(search_matches(search_words("someone"), text));
    CHECK_FALSE(search_matches(search_words("halo nope"), text));
    // Quotes and the Library's field prefixes are ordinary words here.
    CHECK_FALSE(search_matches(search_words("\"halo\""), text));
    CHECK_FALSE(search_matches(search_words("artist:beyonce"), text));
    CHECK_FALSE(search_matches(search_words("stars:5"), text));

    // The comparison searches song, artist and charter.
    std::vector<DmReportRow> dm = dm_sample();
    const std::string dm_text = dm_report_view::dm_search_text(dm[0]);
    CHECK(search_matches(search_words("charter a under"), dm_text));
    CHECK_FALSE(search_matches(search_words("2024"), dm_text));
}

TEST_CASE("report view: search, keep-rule, clear_search and the count line") {
    TableView<ReportRow> view = path_view({
        path_row("Halo", "Beyonce", "A", "1", 300, 1.0, true),
        path_row("Halo", "Beyonce", "A", "2", 200, 500.0, false),
        path_row("Toxicity", "System", "B", "1", 400, std::nullopt, true),
        path_row("Aerials", "System", "B", "1", 100, 500.0, true),
    });
    CHECK(view.visible() == std::vector<size_t>{0, 1, 2, 3});
    CHECK(view.count_line(path_report_view::kNoun) == "4 of 4 paths");

    view.set_search("system");
    CHECK(songs(view) == std::vector<std::string>{"Toxicity", "Aerials"});
    CHECK(view.count_line(path_report_view::kNoun) == "2 of 4 paths");

    view.set_keep(path_report_view::path_keep(std::string("Beyond"), true));
    CHECK(songs(view) == std::vector<std::string>{"Aerials"});

    view.clear_search();
    CHECK(view.search().empty());
    CHECK(songs(view) == std::vector<std::string>{"Aerials"});
    view.set_keep(path_report_view::path_keep(std::nullopt, false));
    CHECK(view.count_line(path_report_view::kNoun) == "4 of 4 paths");

    CHECK(report_view::count_line(1234, 56789, "paths") == "1,234 of 56,789 paths");
}

TEST_CASE("report view sort: first directions, empty values sink, two keys, stable ties") {
    TableView<ReportRow> view = path_view({
        path_row("b song", "X", "A", "1", 300, 10.0, true),
        path_row("Alpha", "X", "A", "2", 300, std::nullopt, false),
        path_row("Zed", "Y", "B", "1", 100, 30.0, true),
        path_row("alpha", "Y", "B", "1", 500, 20.0, true),
    });
    // A numeric column starts high to low, a text one A to Z.
    CHECK(view.first_direction("score") == SortDir::Descending);
    CHECK(view.first_direction("song") == SortDir::Ascending);
    CHECK(view.first_direction("tier") == SortDir::Ascending);

    view.set_sort({{"score", SortDir::Descending}});
    CHECK(songs(view) == std::vector<std::string>{"alpha", "b song", "Alpha", "Zed"});

    // Hardest ms: the row with none sinks both ways.
    view.set_sort({{"ms", SortDir::Descending}});
    CHECK(songs(view) == std::vector<std::string>{"Zed", "alpha", "b song", "Alpha"});
    view.set_sort({{"ms", SortDir::Ascending}});
    CHECK(songs(view) == std::vector<std::string>{"b song", "alpha", "Zed", "Alpha"});

    // Text ignores case and accents; a tie keeps row order.
    view.set_sort({{"song", SortDir::Ascending}});
    CHECK(songs(view) == std::vector<std::string>{"Alpha", "alpha", "b song", "Zed"});
    view.set_sort({{"song", SortDir::Descending}});
    CHECK(songs(view) == std::vector<std::string>{"Zed", "b song", "Alpha", "alpha"});

    // The second key breaks the first's ties.
    view.set_sort({{"artist", SortDir::Ascending}, {"score", SortDir::Ascending}});
    CHECK(songs(view) == std::vector<std::string>{"b song", "Alpha", "Zed", "alpha"});
    view.set_sort({{"artist", SortDir::Descending}, {"score", SortDir::Descending}});
    CHECK(songs(view) == std::vector<std::string>{"alpha", "Zed", "b song", "Alpha"});

    CHECK_THROWS_AS(view.set_sort({{"song", SortDir::Ascending},
                                   {"artist", SortDir::Ascending},
                                   {"score", SortDir::Ascending}}),
                    std::invalid_argument);
    CHECK_THROWS_AS(view.set_sort({{"nope", SortDir::Ascending}}), std::invalid_argument);

    using report_view::compare_keys;
    using report_view::folded_key;
    CHECK(compare_keys(folded_key(1.0), folded_key(2.0)) < 0);
    CHECK(compare_keys(folded_key(std::string("B")), folded_key(std::string("a"))) > 0);
    CHECK(compare_keys(folded_key(std::string("\xC3\x89t\xC3\xA9")),
                       folded_key(std::string("ete"))) == 0);
}

TEST_CASE("path report columns: the page's fifteen, in order, word for word") {
    const auto cols = path_report_view::path_columns(kWindow);
    CHECK(ids(cols) == std::vector<std::string>{"song", "artist", "charter", "mode", "path",
                                                "score", "acts", "skip", "ms", "tier", "efill",
                                                "mult", "sqin", "sqout", "notes"});
    CHECK(titles(cols) == std::vector<std::string>{
                              "Song", "Artist", "Charter", "Mode", "Path", "Score", "Acts",
                              "Max skip", "Hardest ms", "Timing", "Early fill (ms)",
                              "Avg multiplier", "SqIn", "SqOut", "Notes"});
    std::vector<bool> numeric;
    for (const auto& c : cols) numeric.push_back(c.numeric);
    CHECK(numeric == std::vector<bool>{false, false, false, false, false, true, true, true,
                                       true, false, true, true, true, true, true});
    std::vector<std::string> defs;
    for (const auto& c : cols) defs.push_back(c.definition);
    CHECK(defs == std::vector<std::string>{
        "",
        "",
        "",
        "The difficulty and drum options the path was found for.",
        "The path in path notation: one entry per activation, with its skip count and squeeze symbols.",
        "The total score the path reaches.",
        "Activations: how many times the path uses Star Power.",
        "The most fills any one activation passes over before activating.",
        "The hardest squeeze or required early fill the path needs, in raw ms. A dash means it needs none.",
        "How hard Hardest ms is, in bands of your hit window. Beyond means more than twice the hit window.",
        "The hardest early fill (E0) on the path: how many ms early you must hit to summon the fill. Negative means slack. A dash means the path has none.",
        "Average multiplier: the score without solo bonuses divided by the base score (every note at 1x).",
        "SP phrase notes squeezed into an active Star Power window (+ in the path).",
        "SP phrase notes squeezed out of an active Star Power window (- in the path).",
        "Notes in the chart.",
    });
    // The page's cell classes.
    CHECK(cols[0].look.truncate);
    CHECK_FALSE(cols[0].look.dim);
    CHECK(cols[1].look.dim);
    CHECK(cols[3].look.dim);
    CHECK(cols[4].look.mono);
    CHECK(cols[9].look.chip);
    CHECK_FALSE(cols[5].look.truncate);

    const SortSpec first = path_report_view::path_first_sort();
    CHECK(first.column == "score");
    CHECK(first.dir == SortDir::Descending);
}

TEST_CASE("path report cells read as the page wrote them") {
    const auto cols = path_report_view::path_columns(kWindow);
    ReportRow r = path_row("Halo", "Beyonce", "Someone", "2(+) E0 1", 1234567, 12.34, true);
    r.acts = 3;
    r.skip = 2;
    r.efill = 4.0;
    r.mult = 2.5;
    r.sqin = 1;
    r.sqout = 0;
    r.notes = 1500;
    CHECK(cells(cols, r) == std::vector<std::string>{
                                "Halo", "Beyonce", "Someone", "Expert Pro Drums", "2(+) E0 1",
                                "1,234,567", "3", "2", "12.3 ms", "Hard", "4.0 ms", "2.500",
                                "1", "0", "1,500"});

    // No timing and no early fill: dashes, and the None tier's chip.
    ReportRow none = path_row("Halo", "Beyonce", "Someone", "1", 100, std::nullopt, false);
    CHECK(cols[8].cell(none) == report::kDash);
    CHECK(cols[10].cell(none) == report::kDash);
    CHECK(cols[9].cell(none) == "No squeezes");
    CHECK(std::holds_alternative<std::monostate>(cols[8].sort_key(none)));
    CHECK(std::holds_alternative<std::monostate>(cols[10].sort_key(none)));

    ReportRow far = path_row("Halo", "Beyonce", "Someone", "1", 100, 500.0, false);
    CHECK(cols[9].cell(far) == "Beyond 170 ms");

    // Each column sorts on the number or text the page sorted on.
    CHECK(std::get<double>(cols[5].sort_key(r)) == 1234567.0);
    CHECK(std::get<double>(cols[8].sort_key(r)) == 12.34);
    CHECK(std::get<std::string>(cols[9].sort_key(r)) == "Hard");
    CHECK(std::get<double>(cols[10].sort_key(r)) == 4.0);
    CHECK(std::get<double>(cols[11].sort_key(r)) == 2.5);
    CHECK(std::get<std::string>(cols[4].sort_key(r)) == "2(+) E0 1");
}

TEST_CASE("path report: Timing choices and the keep-rule") {
    const std::vector<TimingTier> tiers = timing_tiers(kWindow);
    const auto choices = path_report_view::timing_choices(kWindow);
    REQUIRE(choices.size() == tiers.size() + 1);
    CHECK_FALSE(choices[0].tier.has_value());
    CHECK(choices[0].label == "All timing tiers");
    for (size_t i = 0; i < tiers.size(); ++i) {
        CHECK(choices[i + 1].tier == std::optional<std::string>(tiers[i].name));
        CHECK(choices[i + 1].label == path_report_view::tier_label(tiers[i].name, kWindow));
    }
    CHECK(choices[1].label == "Normal");
    CHECK(choices[choices.size() - 2].label == "Beyond 170 ms");
    CHECK(choices.back().label == "No squeezes");

    const ReportRow best_beyond = path_row("A", "", "", "1", 1, 500.0, true);
    const ReportRow other_beyond = path_row("B", "", "", "2", 1, 500.0, false);
    const ReportRow best_none = path_row("C", "", "", "1", 1, std::nullopt, true);
    auto all = path_report_view::path_keep(std::nullopt, false);
    auto best = path_report_view::path_keep(std::nullopt, true);
    auto beyond = path_report_view::path_keep(std::string("Beyond"), false);
    auto beyond_best = path_report_view::path_keep(std::string("Beyond"), true);
    CHECK(all(best_beyond));
    CHECK(all(other_beyond));
    CHECK(all(best_none));
    CHECK(best(best_beyond));
    CHECK_FALSE(best(other_beyond));
    CHECK(best(best_none));
    CHECK(beyond(best_beyond));
    CHECK(beyond(other_beyond));
    CHECK_FALSE(beyond(best_none));
    CHECK(beyond_best(best_beyond));
    CHECK_FALSE(beyond_best(other_beyond));
}

TEST_CASE("dm comparison columns: the page's thirteen, in order, word for word") {
    const auto cols = dm_report_view::dm_columns();
    CHECK(ids(cols) == std::vector<std::string>{"song", "artist", "charter", "actual",
                                                "optimal", "delta", "pct_h", "fc", "percent",
                                                "speed", "rank", "posted", "status"});
    CHECK(titles(cols) == std::vector<std::string>{
                              "Song", "Artist", "Charter", "Actual", "Hydra opt",
                              "Points left", "% of opt", "FC", "Percent", "Speed", "Rank",
                              "Posted", "Status"});
    std::vector<bool> numeric;
    for (const auto& c : cols) numeric.push_back(c.numeric);
    CHECK(numeric == std::vector<bool>{false, false, false, true, true, true, true, true, true,
                                       true, true, false, false});
    std::vector<std::string> defs;
    for (const auto& c : cols) defs.push_back(c.definition);
    CHECK(defs == std::vector<std::string>{
        "",
        "",
        "",
        "The score the player posted.",
        "The optimal score Hydra found for the chart at SP cap 4, the Clone Hero rule.",
        "Hydra opt minus Actual. Marked over when the posted score is higher.",
        "Actual as a percent of Hydra opt. Only for scores played at 100% speed.",
        "Full combo: every note hit.",
        "The percent the leaderboard lists for this score.",
        "The playback speed the score was set at. 100% is normal speed.",
        "The score rank on this chart leaderboard.",
        "The date the score was posted.",
        "Under optimal, At optimal or Above optimal when Hydra has a result. Not analyzed: the chart is in your library but has no current result for this mode at SP cap 4. No paths: analyzed, but the analysis kept no path. Not in your library: the last scan did not find it. Other speed: played at a speed other than 100%. Clone Hero keeps a separate leaderboard per speed, so it is shown but not compared.",
    });
    CHECK(cols[0].look.truncate);
    CHECK(cols[1].look.dim);
    CHECK(cols[2].look.dim);
    CHECK(cols[11].look.dim);
    CHECK(cols[12].look.chip);

    const SortSpec first = dm_report_view::dm_first_sort();
    CHECK(first.column == "delta");
    CHECK(first.dir == SortDir::Descending);
}

TEST_CASE("dm comparison cells read as the page wrote them") {
    const auto cols = dm_report_view::dm_columns();
    const std::vector<DmReportRow> rows = dm_sample();
    CHECK(cells(cols, rows[0]) == std::vector<std::string>{
                                      "Under", "Band A", "Charter A", "98,000", "100,000",
                                      "2,000", "98.00%", report::kDash, "99%", "100%",
                                      "#3", "2024-05-01", "under optimal"});
    CHECK(cells(cols, rows[1]) == std::vector<std::string>{
                                      "Above", "Band B", "Charter B", "101,234", "100,000",
                                      "+1,234 over", "101.23%", "\xE2\x9C\x93", "100%", "100%",
                                      "#1", "2023-01-02", "above optimal"});
    const std::string d = report::kDash;
    CHECK(cells(cols, rows[2]) == std::vector<std::string>{
                                      "Missing", "Band C", "", "5,000", d, d, d, d, "80%",
                                      "100%", d, d, "not in library"});
    CHECK(cells(cols, rows[3]) == std::vector<std::string>{
                                      "Fast", "Band D", "Charter D", "100,050", "100,000",
                                      "+50 over", d, d, "100%", "150%", "#12", "2025-12-31",
                                      "other speed"});

    // Points left: red over optimal, dim with no value or at another speed.
    const auto& delta = cols[5];
    CHECK(delta.tone(rows[0]) == Tone::Normal);
    CHECK(delta.tone(rows[1]) == Tone::Alert);
    CHECK(delta.tone(rows[2]) == Tone::Dim);
    CHECK(delta.tone(rows[3]) == Tone::Dim);

    // % of opt sorts on the shown hundredths.
    CHECK(std::get<double>(cols[6].sort_key(rows[1])) == 10123.0);
    CHECK(std::holds_alternative<std::monostate>(cols[6].sort_key(rows[3])));
    CHECK(std::get<double>(cols[7].sort_key(rows[1])) == 1.0);
    CHECK(std::get<double>(cols[7].sort_key(rows[0])) == 0.0);
    // Posted sorts as text; an empty date has no key (D103 item 17).
    CHECK(std::get<std::string>(cols[11].sort_key(rows[0])) == "2024-05-01T12:00:00Z");
    CHECK(std::holds_alternative<std::monostate>(cols[11].sort_key(rows[2])));
}

TEST_CASE("dm comparison: Status choices, the keep-rule, sort and count line") {
    const auto choices = dm_report_view::status_choices();
    std::vector<std::string> labels;
    std::vector<std::optional<std::string>> statuses;
    for (const auto& c : choices) {
        labels.push_back(c.label);
        statuses.push_back(c.status);
    }
    CHECK(labels == std::vector<std::string>{
                        "All charts", "Under optimal", "At optimal", "Above optimal",
                        "Not analyzed (in your library)", "No paths (analyzed, none kept)",
                        "Not in your library", "Other speed"});
    CHECK(statuses == std::vector<std::optional<std::string>>{
                          std::nullopt, std::string("under optimal"),
                          std::string("at optimal"), std::string("above optimal"),
                          std::string("not analyzed"), std::string("no paths"),
                          std::string("not in library"), std::string("other speed")});

    TableView<DmReportRow> view = dm_view(dm_sample());
    view.set_sort({dm_report_view::dm_first_sort()});
    CHECK(songs(view) == std::vector<std::string>{"Under", "Fast", "Above", "Missing"});
    CHECK(view.count_line(dm_report_view::kNoun) == "4 of 4 scores");
    view.set_sort({{"delta", SortDir::Ascending}});
    CHECK(songs(view) == std::vector<std::string>{"Above", "Fast", "Under", "Missing"});
    view.set_sort({{"pct_h", view.first_direction("pct_h")}});
    CHECK(songs(view) == std::vector<std::string>{"Above", "Under", "Missing", "Fast"});
    // An empty Posted date sinks in both directions (D103 item 17).
    view.set_sort({{"posted", SortDir::Ascending}});
    CHECK(songs(view) == std::vector<std::string>{"Above", "Under", "Fast", "Missing"});
    view.set_sort({{"posted", SortDir::Descending}});
    CHECK(songs(view) == std::vector<std::string>{"Fast", "Under", "Above", "Missing"});

    view.set_keep(dm_report_view::dm_keep(std::string("under optimal")));
    CHECK(songs(view) == std::vector<std::string>{"Under"});
    CHECK(view.count_line(dm_report_view::kNoun) == "1 of 4 scores");
    view.set_keep(dm_report_view::dm_keep(std::nullopt));
    CHECK(view.count_line(dm_report_view::kNoun) == "4 of 4 scores");
}
