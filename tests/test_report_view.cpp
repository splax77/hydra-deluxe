// The report windows' shared table (app/report_view.h) and each report's
// columns, cells and keep-rules (app/path_report_view.h,
// app/dm_report_view.h). The rows are Hydra's own: the path report over the
// corpus library, and a leaderboard player's scores joined to it by
// dm_report::collect_dm_rows. Each expected cell is the owner's formatter on
// the row's own fields.

#include "doctest.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "app/analysis.h"         // discover_charts, save_scan_as_library, ScanItem
#include "app/config.h"           // Settings::batch_run
#include "app/display_format.h"   // format_avg_mult, format_percent
#include "app/dm_report.h"        // collect_dm_rows, kStatusWords
#include "app/dm_report_view.h"
#include "app/path_report_view.h"
#include "app/report.h"           // generate_report, beyond_edge_text, hit_window_text
#include "app/report_view.h"
#include "core/model.h"           // counted, format_ms, group_thousands, kCloneHeroSpCap
#include "core/squeeze_rating.h"  // timing_tiers, beyond_tier, none_tier
#include "core/strutil.h"         // starts_with
#include "corpus_util.h"
#include "dm_fixture.h"           // testdm::settings, profile, save, make_score, kHash
#include "headless_imgui_fixture.h"
#include "imgui.h"
#include "net/dmbot_client.h"     // DmScore, kBaseSpeedPercent, is_base_speed
#include "parse/song.h"           // NoNotesError
#include "report_samples.h"       // every_row
#include "store/record_store.h"
#include "ui/column_widths.h"
#include "ui/fonts.h"
#include "ui/report_window.h"  // report_frame::column_spec
#include "ui/text_role.h"

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

// The corpus as a user's library: the library scan of testdata/input saved
// as the library, and every chart with notes in the comparison's chart mode
// analyzed at Clone Hero's cap (testdm::settings) and saved, so the path
// report and the leaderboard comparison both read it.
store::RecordStore& corpus_store() {
    static const std::unique_ptr<store::RecordStore> db = [] {
        auto out = std::make_unique<store::RecordStore>(":memory:");
        auto [items, errors] = discover_charts({corpus::root()});
        if (!errors.empty()) throw std::runtime_error("corpus discovery error: " + errors.front());
        if (const std::optional<std::string> problem = save_scan_as_library(*out, items))
            throw std::runtime_error("corpus library save: " + *problem);
        const Settings settings = testdm::settings();
        const BatchRun run = settings.batch_run();
        for (const ScanItem& item : items) {
            try {
                testdm::save(*out, item.md5, settings,
                             corpus::analyzed(item.notespath, run.settings));
            } catch (const NoNotesError&) {
                // No notes in this chart mode: the report lists no row for it.
            }
        }
        return out;
    }();
    return *db;
}

// Hydra's path report over the corpus library.
const report::GeneratedReport& corpus_page() {
    static const report::GeneratedReport page = [] {
        report::ReportOptions options;
        options.run = testdm::settings().batch_run();
        return report::generate_report(corpus_store(), options);
    }();
    return page;
}

// The corpus page's rows. Fails the test when the corpus gives none.
const std::vector<ReportRow>& corpus_report_rows() {
    const std::vector<ReportRow>& rows = corpus_page().paths;
    REQUIRE(!rows.empty());
    return rows;
}

// Where each comparison score sits in corpus_scores().
enum ScoreIndex : size_t { kUnder, kAbove, kAt, kOtherSpeed, kMissing, kScoreCount };

// A leaderboard player's scores joined to the corpus library by
// collect_dm_rows: one under, one over and one equal to a chart's best path,
// one over its best at another speed, and one on a chart the library does
// not list. The scores leave the song names empty, so each joined row is
// named from the library.
std::vector<DmReportRow> corpus_scores() {
    std::vector<const ReportRow*> best;  // each chart's rank-1 row, in page order
    for (const ReportRow& r : corpus_report_rows()) {
        if (r.rank != 1) continue;
        const bool seen = std::any_of(best.begin(), best.end(), [&](const ReportRow* b) {
            return b->hyhash == r.hyhash;
        });
        if (!seen) best.push_back(&r);
    }
    REQUIRE(best.size() >= 4);
    for (const ReportRow& r : corpus_report_rows()) REQUIRE(r.hyhash != testdm::kHash);

    const auto score = [](const std::string& id, int64_t points, int speed, const char* posted,
                          std::optional<int> rank) {
        net::DmScore s = testdm::make_score(id, points, speed);
        s.song_name.clear();
        s.artist.clear();
        s.charter.clear();
        s.posted = posted;
        s.rank = rank;
        return s;
    };
    std::vector<net::DmScore> scores(kScoreCount);
    scores[kUnder] = score(best[0]->hyhash, best[0]->score - 2000, net::kBaseSpeedPercent,
                           "2024-05-01T12:00:00Z", 3);
    scores[kAbove] = score(best[1]->hyhash, best[1]->score + 1234, net::kBaseSpeedPercent,
                           "2023-01-02T00:00:00Z", 1);
    scores[kAbove].is_fc = true;
    scores[kAt] = score(best[2]->hyhash, best[2]->score, net::kBaseSpeedPercent, "", 7);
    scores[kOtherSpeed] = score(best[3]->hyhash, best[3]->score + 50,
                                net::kBaseSpeedPercent + 50, "2025-12-31T23:59:59Z", 12);
    scores[kMissing] = score(testdm::kHash, 5000, net::kBaseSpeedPercent,
                             "2022-06-15T08:30:00Z", std::nullopt);
    return dm_report::collect_dm_rows(corpus_store(), scores, testdm::profile());
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
    return TableView<ReportRow>(std::make_shared<const std::vector<ReportRow>>(std::move(rows)),
                                std::move(texts), path_report_view::path_columns());
}

TableView<DmReportRow> dm_view(std::vector<DmReportRow> rows) {
    std::vector<std::string> texts = dm_texts(rows);
    return TableView<DmReportRow>(
        std::make_shared<const std::vector<DmReportRow>>(std::move(rows)), std::move(texts),
        dm_report_view::dm_columns());
}

// A path report result holding `rows`, as the window is handed one.
std::shared_ptr<const report::GeneratedReport> path_result(std::vector<ReportRow> rows) {
    auto result = std::make_shared<report::GeneratedReport>();
    result->paths = std::move(rows);
    return result;
}

// A view built from `result` the way the path window builds one.
TableView<ReportRow> path_view_of(const std::shared_ptr<const report::GeneratedReport>& result) {
    return TableView<ReportRow>(report_view::shared_rows(result, &report::GeneratedReport::paths),
                                path_texts(result->paths), path_report_view::path_columns());
}

template <class Row>
std::vector<std::string> ids(const std::vector<report_view::Column<Row>>& cols) {
    std::vector<std::string> out;
    for (const auto& c : cols) out.push_back(c.id);
    return out;
}

// True when no two columns share an id: the sort and the windows find a
// column by its id.
template <class Row>
bool ids_unique(const std::vector<report_view::Column<Row>>& cols) {
    std::vector<std::string> sorted = ids(cols);
    std::sort(sorted.begin(), sorted.end());
    return std::adjacent_find(sorted.begin(), sorted.end()) == sorted.end();
}

template <class Row>
const report_view::Column<Row>& column_named(const std::vector<report_view::Column<Row>>& cols,
                                             const std::string& id) {
    for (const auto& c : cols)
        if (c.id == id) return c;
    throw std::invalid_argument("no column " + id);
}

template <class Row>
std::string cell_of(const std::vector<report_view::Column<Row>>& cols, const std::string& id,
                    const Row& row) {
    return column_named(cols, id).cell(row);
}

bool holds(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// Whether `text` holds "name: " and then `gloss` as dm_report_view shows it.
// A gloss's placeholders (dm_report.h, StatusWords::gloss) are filled by the
// view, so each one stands for whatever the view put there; the gloss's own
// words around them must all follow, in order.
bool holds_gloss(const std::string& text, const std::string& name, const std::string& gloss) {
    size_t at = text.find(name + ": ");
    if (at == std::string::npos) return false;
    at += name.size() + 2;
    size_t from = 0;
    bool first = true;
    while (true) {
        const size_t open = gloss.find("__", from);
        const std::string piece =
            gloss.substr(from, open == std::string::npos ? std::string::npos : open - from);
        if (first) {
            if (text.compare(at, piece.size(), piece) != 0) return false;
        } else {
            const size_t found = text.find(piece, at);
            if (found == std::string::npos) return false;
            at = found;
        }
        at += piece.size();
        first = false;
        if (open == std::string::npos) return true;
        const size_t close = gloss.find("__", open + 2);
        if (close == std::string::npos) return false;
        from = close + 2;
    }
}

// The indices of the rows of `rows` a search for `typed` keeps, in row
// order: search_matches over each row's own search text.
std::vector<size_t> matching(const std::vector<ReportRow>& rows, const std::string& typed) {
    const std::vector<std::string> words = report_view::search_words(typed);
    std::vector<size_t> out;
    for (size_t i = 0; i < rows.size(); ++i)
        if (report_view::search_matches(words, path_report_view::path_search_text(rows[i])))
            out.push_back(i);
    return out;
}

// An artist some of `rows` name and some do not, so a search for it narrows
// the table.
std::string narrowing_artist(const std::vector<ReportRow>& rows) {
    for (const ReportRow& r : rows) {
        const size_t found = matching(rows, r.artist).size();
        if (found > 0 && found < rows.size()) return r.artist;
    }
    FAIL("every corpus row names the same artist");
    return {};
}

// A word from a song in `rows` that `text` does not hold.
std::string absent_from(const std::vector<ReportRow>& rows, const std::string& text) {
    for (const ReportRow& r : rows)
        for (const std::string& word : report_view::search_words(r.song))
            if (!report_view::search_matches({word}, text)) return word;
    FAIL("every corpus song word is in the search text");
    return {};
}

// Where row `a` falls against row `b` under `specs`, as the sort promises:
// each key in turn, in its direction, with a row that has no value for a key
// after every row that has one, whichever way the key runs.
template <class Row>
int sort_order(const std::vector<report_view::Column<Row>>& cols,
               const std::vector<SortSpec>& specs, const Row& a, const Row& b) {
    for (const SortSpec& s : specs) {
        const report_view::Column<Row>& col = column_named(cols, s.column);
        const SortKey ka = col.sort_key(a);
        const SortKey kb = col.sort_key(b);
        const bool a_empty = std::holds_alternative<std::monostate>(ka);
        const bool b_empty = std::holds_alternative<std::monostate>(kb);
        if (a_empty && b_empty) continue;
        if (a_empty != b_empty) return a_empty ? 1 : -1;
        int c = report_view::compare_keys(report_view::folded_key(ka), report_view::folded_key(kb));
        if (s.dir == SortDir::Descending) c = -c;
        if (c != 0) return c;
    }
    return 0;
}

// Sets `specs` on `view` and checks the rows it shows come in that order,
// rows that tie on every key in their row order.
template <class Row>
void check_sorts(TableView<Row>& view, const std::vector<report_view::Column<Row>>& cols,
                 const std::vector<SortSpec>& specs) {
    view.set_sort(specs);
    const std::vector<size_t>& shown = view.visible();
    for (size_t k = 1; k < shown.size(); ++k) {
        const int c = sort_order(cols, specs, view.rows()[shown[k - 1]], view.rows()[shown[k]]);
        CHECK(c <= 0);
        if (c == 0) CHECK(shown[k - 1] < shown[k]);
    }
}

// A row on tier `tier`, named after it, and the best path or not.
ReportRow tier_row(const std::string& tier, bool optimal) {
    ReportRow r;
    r.song = tier;
    r.tier = tier;
    r.optimal = optimal;
    return r;
}

// The visible rows' songs, in order.
template <class Row>
std::vector<std::string> songs(const TableView<Row>& view) {
    std::vector<std::string> out;
    for (size_t i : view.visible()) out.push_back(view.rows()[i].song);
    return out;
}

}  // namespace

TEST_CASE("report view search: folded words in any order, nothing more") {
    using report_view::search_matches;
    using report_view::search_words;
    CHECK(search_words("  Beyonc\xC3\xA9   HALO ") == std::vector<std::string>{"beyonce", "halo"});
    // Tabs and the no-break space split words like a space.
    CHECK(search_words("halo\tbeyonce") == std::vector<std::string>{"halo", "beyonce"});
    CHECK(search_words("halo\xC2\xA0" "beyonce") == std::vector<std::string>{"halo", "beyonce"});
    CHECK(search_words("").empty());
    CHECK(search_words("   ").empty());

    const std::vector<ReportRow>& rows = corpus_report_rows();
    // A corpus row whose song holds a plain "e", so a query can spell it "é".
    const auto with_e = std::find_if(rows.begin(), rows.end(), [](const ReportRow& r) {
        return holds(r.song, "e");
    });
    REQUIRE(with_e != rows.end());
    const ReportRow& row = *with_e;
    const std::string text = path_report_view::path_search_text(row);
    std::string shouted = row.song;
    for (char& ch : shouted)
        if (static_cast<unsigned char>(ch) < 0x80)
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    std::string accented = row.song;
    accented.replace(accented.find('e'), 1, "\xC3\xA9");

    CHECK(search_matches(search_words(""), text));
    CHECK(search_matches(search_words(row.artist + " " + shouted), text));
    CHECK(search_matches(search_words(accented), text));
    CHECK(search_matches(search_words(search_words(row.path).front()), text));
    CHECK(search_matches(search_words(row.charter), text));
    CHECK_FALSE(search_matches(search_words(row.song + " " + absent_from(rows, text)), text));
    // Quotes and the Library's field prefixes are ordinary words here.
    const std::string song_word = search_words(row.song).front();
    CHECK_FALSE(search_matches(search_words("\"" + song_word + "\""), text));
    CHECK_FALSE(search_matches(search_words("artist:" + search_words(row.artist).front()), text));

    // The comparison searches song, artist and charter, not the posted date.
    const std::vector<DmReportRow> dm = corpus_scores();
    const DmReportRow& joined = dm[kUnder];
    const std::string dm_text = dm_report_view::dm_search_text(joined);
    CHECK(search_matches(search_words(joined.charter + " " + joined.song + " " + joined.artist),
                         dm_text));
    CHECK_FALSE(search_matches(search_words(joined.posted.substr(0, 4)), dm_text));
}

TEST_CASE("report view: search, keep-rule, clear_search and the count line") {
    const std::vector<ReportRow>& rows = corpus_report_rows();
    const auto& noun = path_report_view::kNoun;
    TableView<ReportRow> view = path_view(rows);
    CHECK(view.visible() == report_samples::every_row(rows.size()));
    CHECK(view.count_line(noun) == report_view::count_line(rows.size(), rows.size(), noun));

    // A search shows the rows that hold every word, and the count line counts
    // them out of every row.
    const std::string typed = narrowing_artist(rows);
    const std::vector<size_t> found = matching(rows, typed);
    view.set_search(typed);
    CHECK(view.visible() == found);
    CHECK(view.count_line(noun) == report_view::count_line(found.size(), rows.size(), noun));

    // The keep-rule narrows what the search keeps: best paths on one tier.
    const std::string tier = rows[found.front()].tier;
    const auto kept = [&](const std::vector<size_t>& from) {
        std::vector<size_t> out;
        for (size_t i : from)
            if (rows[i].tier == tier && rows[i].optimal) out.push_back(i);
        return out;
    };
    view.set_keep(path_report_view::path_keep(tier, true));
    CHECK(view.visible() == kept(found));

    // Clearing the search keeps the keep-rule.
    view.clear_search();
    CHECK(view.search().empty());
    CHECK(view.visible() == kept(report_samples::every_row(rows.size())));
    view.set_keep(path_report_view::path_keep(std::nullopt, false));
    CHECK(view.count_line(noun) == report_view::count_line(rows.size(), rows.size(), noun));
}

// The rows live once, in the result; the view only points at them.
TEST_CASE("report view shares its result's rows instead of copying them") {
    const std::shared_ptr<const report::GeneratedReport> result = path_result(corpus_report_rows());
    const long owners_before = result.use_count();
    const TableView<ReportRow> view = path_view_of(result);
    CHECK(&view.rows() == &result->paths);
    // The view holds the result itself, one more owner than before.
    CHECK(result.use_count() == owners_before + 1);
}

// A Refresh, a new batch's report or a Cancel replaces or drops the slot's
// result while the window still holds its view.
TEST_CASE("report view still reads its rows after the result is replaced or dropped") {
    const std::vector<ReportRow>& rows = corpus_report_rows();
    std::shared_ptr<const report::GeneratedReport> result = path_result(rows);
    const std::weak_ptr<const report::GeneratedReport> held = result;
    TableView<ReportRow> view = path_view_of(result);
    result = path_result({rows.back()});
    CHECK_FALSE(held.expired());
    result.reset();
    const std::string typed = narrowing_artist(rows);
    std::vector<size_t> found = matching(rows, typed);
    view.set_sort({path_report_view::path_first_sort()});
    view.set_search(typed);
    REQUIRE(view.rows().size() == rows.size());
    std::vector<size_t> shown = view.visible();
    std::sort(shown.begin(), shown.end());
    CHECK(shown == found);
    for (size_t i : view.visible()) CHECK(view.rows()[i].song == rows[i].song);
    CHECK(view.count_line(path_report_view::kNoun) ==
          report_view::count_line(found.size(), rows.size(), path_report_view::kNoun));
}

TEST_CASE("report view count line: the noun follows counted on the total") {
    const std::vector<ReportRow>& rows = corpus_report_rows();
    const auto& noun = path_report_view::kNoun;
    const std::string one_path = counted(1, noun.one, noun.many);
    TableView<ReportRow> view = path_view({rows.front()});
    // One row in the table: the singular noun, never the plural.
    CHECK(holds(view.count_line(noun), one_path));
    CHECK_FALSE(holds(view.count_line(noun), noun.many));
    // No row shown: the noun still follows the one row in the table.
    view.set_search(absent_from(rows, path_report_view::path_search_text(rows.front())));
    REQUIRE(view.visible().empty());
    CHECK(holds(view.count_line(noun), one_path));
    CHECK_FALSE(holds(view.count_line(noun), noun.many));
    const auto& scores = dm_report_view::kNoun;
    CHECK(holds(report_view::count_line(4, 4, scores), counted(4, scores.one, scores.many)));
    CHECK(holds(report_view::count_line(1, 1, scores), counted(1, scores.one, scores.many)));
    CHECK_FALSE(holds(report_view::count_line(1, 1, scores), scores.many));
}

TEST_CASE("report view sort: first directions, empty values sink, two keys, stable ties") {
    const std::vector<ReportRow>& rows = corpus_report_rows();
    const auto cols = path_report_view::path_columns();
    TableView<ReportRow> view = path_view(rows);
    // A numeric column starts high to low, a text one A to Z.
    CHECK(view.first_direction("score") == SortDir::Descending);
    CHECK(view.first_direction("song") == SortDir::Ascending);
    CHECK(view.first_direction("tier") == SortDir::Ascending);

    for (const char* id : {"score", "ms", "efill", "song", "artist"}) {
        INFO(id);
        check_sorts(view, cols, {{id, SortDir::Descending}});
        check_sorts(view, cols, {{id, SortDir::Ascending}});
    }
    // The second key breaks the first's ties.
    check_sorts(view, cols, {{"artist", SortDir::Ascending}, {"score", SortDir::Ascending}});
    check_sorts(view, cols, {{"artist", SortDir::Descending}, {"score", SortDir::Descending}});

    std::vector<SortSpec> too_many(report_view::kMaxSortKeys + 1, SortSpec{"song", SortDir::Ascending});
    CHECK_THROWS_AS(view.set_sort(too_many), std::invalid_argument);
    CHECK_THROWS_AS(view.set_sort({{"nope", SortDir::Ascending}}), std::invalid_argument);

    using report_view::compare_keys;
    using report_view::folded_key;
    CHECK(compare_keys(folded_key(1.0), folded_key(2.0)) < 0);
    CHECK(compare_keys(folded_key(std::string("B")), folded_key(std::string("a"))) > 0);
    CHECK(compare_keys(folded_key(std::string("\xC3\x89t\xC3\xA9")),
                       folded_key(std::string("ete"))) == 0);
}

// D284 item 1.
TEST_CASE("report view sort: Timing sorts in ladder order, not by name") {
    const std::vector<TimingTier> tiers = timing_tiers();
    std::vector<std::string> ladder;
    for (const TimingTier& t : tiers) ladder.push_back(t.name);
    // Each row's song is its tier's name, so songs() reads the order back.
    // The rows start out of ladder order.
    std::vector<std::string> shuffled = ladder;
    std::rotate(shuffled.begin(), shuffled.begin() + static_cast<std::ptrdiff_t>(shuffled.size() / 2),
                shuffled.end());
    std::vector<ReportRow> rows;
    for (const std::string& tier : shuffled) rows.push_back(tier_row(tier, true));
    TableView<ReportRow> view = path_view(std::move(rows));
    view.set_sort({{"tier", view.first_direction("tier")}});
    CHECK(songs(view) == ladder);
    view.set_sort({{"tier", SortDir::Descending}});
    const std::vector<std::string> reversed(ladder.rbegin(), ladder.rend());
    CHECK(songs(view) == reversed);
}

TEST_CASE("path report columns: one id each and number columns drawn in figures") {
    const auto cols = path_report_view::path_columns();
    CHECK(ids_unique(cols));
    // A number column holds figures; a text column holds words or notation.
    for (const auto& c : cols) {
        INFO(c.id);
        CHECK((c.look.kind == report_view::CellKind::Figure) == c.numeric);
    }
    CHECK(column_named(cols, "path").look.kind == report_view::CellKind::Notation);
    CHECK(column_named(cols, "song").look.truncate);
    CHECK_FALSE(column_named(cols, "song").look.dim);
    CHECK(column_named(cols, "artist").look.dim);
    CHECK(column_named(cols, "tier").look.chip);
    CHECK_FALSE(column_named(cols, "score").look.truncate);
    // The Timing definition names the hit window the reports print.
    CHECK(holds(column_named(cols, "tier").definition, report::hit_window_text()));
    CHECK(column_named(cols, "mult").title == app::kAverageMultiplierLabel);

    // The window starts sorted on a number column, high to low.
    const SortSpec first = path_report_view::path_first_sort();
    CHECK(column_named(cols, first.column).numeric);
    CHECK(first.dir == report_view::first_direction(true));
}

TEST_CASE("path report cells read as the page wrote them") {
    const auto cols = path_report_view::path_columns();
    const std::vector<TimingTier> tiers = timing_tiers();
    const std::string dash = report::kDash;
    const auto ms_text = [&](const std::optional<double>& ms) {
        return ms ? format_ms(*ms) : dash;
    };
    const auto key = [&](const char* id, const ReportRow& r) {
        return column_named(cols, id).sort_key(r);
    };
    for (const ReportRow& r : corpus_report_rows()) {
        INFO(r.song << " path " << r.path);
        CHECK(cell_of(cols, "song", r) == r.song);
        CHECK(cell_of(cols, "artist", r) == r.artist);
        CHECK(cell_of(cols, "charter", r) == r.charter);
        CHECK(cell_of(cols, "path", r) == r.path);
        CHECK(cell_of(cols, "score", r) == group_thousands(r.score));
        CHECK(cell_of(cols, "acts", r) == std::to_string(r.acts));
        CHECK(cell_of(cols, "skip", r) == std::to_string(r.skip));
        CHECK(cell_of(cols, "ms", r) == ms_text(r.ms));
        CHECK(cell_of(cols, "tier", r) == path_report_view::tier_label(r.tier));
        CHECK(cell_of(cols, "efill", r) == ms_text(r.efill));
        CHECK(cell_of(cols, "mult", r) == format_avg_mult(r.mult));
        CHECK(cell_of(cols, "sqin", r) == std::to_string(r.sqin));
        CHECK(cell_of(cols, "sqout", r) == std::to_string(r.sqout));
        CHECK(cell_of(cols, "notes", r) == group_thousands(r.notes));

        // Each column sorts on its row's number or text, except Timing,
        // which sorts on its tier's place in the ladder (D284 item 1).
        CHECK(std::get<double>(key("score", r)) == static_cast<double>(r.score));
        if (r.ms) CHECK(std::get<double>(key("ms", r)) == *r.ms);
        else CHECK(std::holds_alternative<std::monostate>(key("ms", r)));
        if (r.efill) CHECK(std::get<double>(key("efill", r)) == *r.efill);
        else CHECK(std::holds_alternative<std::monostate>(key("efill", r)));
        CHECK(std::get<double>(key("mult", r)) == r.mult);
        CHECK(std::get<std::string>(key("path", r)) == r.path);
        const auto place = std::find_if(tiers.begin(), tiers.end(), [&](const TimingTier& t) {
            return r.tier == t.name;
        });
        REQUIRE(place != tiers.end());
        CHECK(std::get<double>(key("tier", r)) ==
              static_cast<double>(std::distance(tiers.begin(), place)));
    }
    // The Beyond chip names the Beyond edge every report prints.
    CHECK(holds(path_report_view::tier_label(beyond_tier(tiers).name), report::beyond_edge_text()));
    CHECK(path_report_view::tier_label(none_tier(tiers).name) == "No timing needed");
}

TEST_CASE("path report: Timing choices and the keep-rule") {
    const std::vector<TimingTier> tiers = timing_tiers();
    const auto choices = path_report_view::timing_choices();
    REQUIRE(choices.size() == tiers.size() + 1);
    // The window's start, new-result and Clear filters index keeps every tier.
    CHECK_FALSE(choices[report_view::kEveryRowChoice].tier.has_value());
    CHECK(choices[0].label == "All timing tiers");
    for (size_t i = 0; i < tiers.size(); ++i) {
        CHECK(choices[i + 1].tier == std::optional<std::string>(tiers[i].name));
        CHECK(choices[i + 1].label == path_report_view::tier_label(tiers[i].name));
    }
    CHECK(choices[1].label == tiers.front().name);
    CHECK(holds(choices[choices.size() - 2].label, report::beyond_edge_text()));
    CHECK(choices.back().label == "No timing needed");

    const std::string beyond_name = beyond_tier(tiers).name;
    const ReportRow best_beyond = tier_row(beyond_name, true);
    const ReportRow other_beyond = tier_row(beyond_name, false);
    const ReportRow best_none = tier_row(none_tier(tiers).name, true);
    auto all = path_report_view::path_keep(std::nullopt, false);
    auto best = path_report_view::path_keep(std::nullopt, true);
    auto beyond = path_report_view::path_keep(beyond_name, false);
    auto beyond_best = path_report_view::path_keep(beyond_name, true);
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

TEST_CASE("dm comparison columns: one id each and the definitions' game numbers") {
    const auto cols = dm_report_view::dm_columns();
    CHECK(ids_unique(cols));
    CHECK(column_named(cols, "song").look.truncate);
    CHECK(column_named(cols, "artist").look.dim);
    CHECK(column_named(cols, "charter").look.dim);
    CHECK(column_named(cols, "posted").look.dim);
    CHECK(column_named(cols, "status").look.chip);
    for (const char* id : {"actual", "optimal", "delta", "pct_h", "percent", "speed", "rank"}) {
        INFO(id);
        CHECK(column_named(cols, id).numeric);
        CHECK(column_named(cols, id).look.kind == report_view::CellKind::Figure);
    }
    // The definitions state Clone Hero's cap and the base speed from their
    // owners, and the Status definition glosses every status that has one.
    CHECK(holds(column_named(cols, "optimal").definition,
                "SP cap " + std::to_string(kCloneHeroSpCap)));
    CHECK(holds(column_named(cols, "speed").definition,
                std::to_string(net::kBaseSpeedPercent) + "%"));
    CHECK(holds(column_named(cols, "pct_h").definition,
                std::to_string(net::kBaseSpeedPercent) + "%"));
    const std::string status_definition = column_named(cols, "status").definition;
    for (const dm_report::StatusWords& w : dm_report::kStatusWords) {
        INFO(std::string(w.status));
        CHECK(holds(status_definition, w.name));
        if (*w.gloss) CHECK(holds_gloss(status_definition, w.name, w.gloss));
    }
    // The view filled every placeholder a gloss brought in.
    CHECK_FALSE(holds(status_definition, "__"));

    // The window starts sorted on a number column, high to low.
    const SortSpec first = dm_report_view::dm_first_sort();
    CHECK(column_named(cols, first.column).numeric);
    CHECK(first.dir == report_view::first_direction(true));
}

TEST_CASE("dm comparison cells read as the page wrote them") {
    const auto cols = dm_report_view::dm_columns();
    const std::vector<DmReportRow> rows = corpus_scores();
    REQUIRE(rows.size() == kScoreCount);
    // What collect_dm_rows made of each score.
    CHECK(rows[kUnder].status == dm_report::kStatusUnderOptimal);
    CHECK(rows[kAbove].status == dm_report::kStatusAboveOptimal);
    CHECK(rows[kAt].status == dm_report::kStatusAtOptimal);
    CHECK(rows[kOtherSpeed].status == dm_report::kStatusOtherSpeed);
    CHECK(rows[kMissing].status == dm_report::kStatusNotInLibrary);

    const std::string dash = report::kDash;
    const auto key = [&](const char* id, const DmReportRow& r) {
        return column_named(cols, id).sort_key(r);
    };
    for (const DmReportRow& r : rows) {
        INFO(r.identifier << " " << r.status);
        CHECK(cell_of(cols, "song", r) == r.song);
        CHECK(cell_of(cols, "artist", r) == r.artist);
        CHECK(cell_of(cols, "charter", r) == r.charter);
        CHECK(cell_of(cols, "actual", r) == group_thousands(r.actual));
        CHECK(cell_of(cols, "optimal", r) == (r.optimal ? group_thousands(*r.optimal) : dash));
        // Points left; a score over optimal reads by how much it is over,
        // at any speed (D64).
        const std::string delta = cell_of(cols, "delta", r);
        if (!r.delta) {
            CHECK(delta == dash);
        } else if (r.above_optimal) {
            CHECK(delta != group_thousands(*r.delta));
            CHECK(holds(delta, group_thousands(-*r.delta)));
        } else {
            CHECK(delta == group_thousands(*r.delta));
        }
        // % of opt only for a score at base speed beside an optimal.
        const bool compared = r.optimal.has_value() && net::is_base_speed(r.speed);
        CHECK(cell_of(cols, "pct_h", r) ==
              (compared ? format_percent(r.actual, *r.optimal, dm_report::kPercentDecimals) : dash));
        if (compared) {
            REQUIRE(r.pct_h.has_value());
            CHECK(std::get<double>(key("pct_h", r)) == static_cast<double>(*r.pct_h));
        } else {
            CHECK(std::holds_alternative<std::monostate>(key("pct_h", r)));
        }
        CHECK((cell_of(cols, "fc", r) == dash) == !r.is_fc);
        CHECK(std::get<double>(key("fc", r)) == (r.is_fc ? 1.0 : 0.0));
        CHECK(holds(cell_of(cols, "percent", r), std::to_string(r.percent)));
        CHECK(holds(cell_of(cols, "speed", r), std::to_string(r.speed)));
        if (r.rank) CHECK(holds(cell_of(cols, "rank", r), std::to_string(*r.rank)));
        else CHECK(cell_of(cols, "rank", r) == dash);
        // Posted shows the timestamp's date and sorts on the timestamp; an
        // empty date has no key (D103 item 17).
        const std::string posted = cell_of(cols, "posted", r);
        if (r.posted.empty()) {
            CHECK(posted == dash);
            CHECK(std::holds_alternative<std::monostate>(key("posted", r)));
        } else {
            CHECK(!posted.empty());
            CHECK(starts_with(r.posted, posted));
            CHECK(std::get<std::string>(key("posted", r)) == r.posted);
        }
        CHECK(cell_of(cols, "status", r) == r.status);
    }

    // Points left: red over optimal, dim with no value or at another speed.
    const auto& points_left = column_named(cols, "delta");
    CHECK(points_left.tone(rows[kUnder]) == Tone::Normal);
    CHECK(points_left.tone(rows[kAt]) == Tone::Normal);
    CHECK(points_left.tone(rows[kAbove]) == Tone::Alert);
    CHECK(points_left.tone(rows[kMissing]) == Tone::Dim);
    CHECK(points_left.tone(rows[kOtherSpeed]) == Tone::Dim);
}

TEST_CASE("dm comparison: Status choices, the keep-rule, sort and count line") {
    const auto choices = dm_report_view::status_choices();
    REQUIRE(choices.size() == std::size(dm_report::kStatusWords) + 1);
    CHECK(choices[0].label == "All charts");
    // The window's start, new-result and Clear filters index keeps every status.
    CHECK_FALSE(choices[report_view::kEveryRowChoice].status.has_value());
    for (size_t i = 0; i < std::size(dm_report::kStatusWords); ++i) {
        const dm_report::StatusWords& w = dm_report::kStatusWords[i];
        INFO(w.status);
        CHECK(choices[i + 1].status == std::optional<std::string>(w.status));
        CHECK(starts_with(choices[i + 1].label, w.name));
    }

    const auto cols = dm_report_view::dm_columns();
    const std::vector<DmReportRow> rows = corpus_scores();
    const auto& noun = dm_report_view::kNoun;
    TableView<DmReportRow> view = dm_view(rows);
    check_sorts(view, cols, {dm_report_view::dm_first_sort()});
    CHECK(view.count_line(noun) == report_view::count_line(rows.size(), rows.size(), noun));
    check_sorts(view, cols, {{"delta", SortDir::Ascending}});
    // The score off the library has no Points left and sinks.
    CHECK(view.visible().back() == kMissing);
    check_sorts(view, cols, {{"pct_h", view.first_direction("pct_h")}});
    // An empty Posted date sinks in both directions (D103 item 17).
    check_sorts(view, cols, {{"posted", SortDir::Ascending}});
    CHECK(view.visible().back() == kAt);
    check_sorts(view, cols, {{"posted", SortDir::Descending}});
    CHECK(view.visible().back() == kAt);

    view.set_keep(dm_report_view::dm_keep(std::string(dm_report::kStatusUnderOptimal)));
    CHECK(view.visible() == std::vector<size_t>{kUnder});
    CHECK(view.count_line(noun) == report_view::count_line(1, rows.size(), noun));
    view.set_keep(dm_report_view::dm_keep(std::nullopt));
    CHECK(view.count_line(noun) == report_view::count_line(rows.size(), rows.size(), noun));
}

// The header row draws in the default font, so a column's header is measured
// as a Label whatever its cells draw in.
TEST_CASE("report column_spec: a Notation column's header measures as Label, its cells in Notation") {
    // The longest path Hydra's report gives on the corpus.
    const std::vector<ReportRow>& rows = corpus_report_rows();
    const std::string cell = std::max_element(rows.begin(), rows.end(),
                                              [](const ReportRow& a, const ReportRow& b) {
                                                  return a.path.size() < b.path.size();
                                              })->path;
    test::HeadlessImGui imgui;
    REQUIRE(ui::g_mono_font != nullptr);
    ImGui::NewFrame();
    report_view::CellLook look;
    look.kind = report_view::CellKind::Notation;
    const std::vector<ui::ColumnSpec> specs{ui::report_frame::column_spec("Path", look)};
    const ui::MeasuredWidths m =
        ui::measure_widths(specs, 1, [&](size_t, size_t) { return cell; });
    CHECK(m.header_widths[0] == ui::text_size(ui::TextRole::Label, "Path").x);
    CHECK(m.widths[0] == std::ceil(ui::measure_in_role(ui::TextRole::Notation)(cell)));
    ImGui::EndFrame();
}
