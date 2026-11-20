// Tests for app/fill_report: the CH 1.0 vs CH 1.1 join (collect_fill_rows),
// the tally, and the comparison page (build_fill_html).
//
// Every case here uses one in-memory store holding results under each fill
// rule. Each result carries its rule in its profile (docs/adr/0010), and the
// join reads the 1.0 results and the 1.1 results from that one store. The
// cases file real corpus analyses under each rule, listed as the library scan
// lists them; the "in both" cases add a record that kept no path.

#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings::profile_with_fills
#include "app/fill_report.h"
#include "app/html_page.h"  // json_escape_into, html_escape, charts_search_hint
#include "app/library_query.h"  // search_field
#include "app/report.h"  // kNoChartLibrary, nothing_under_settings
#include "app/report_view.h"  // folded_key
#include "core/model.h"
#include "corpus_util.h"
#include "dm_fixture.h"  // testdm::settings, kMode
#include "parse/song.h"
#include "search/graph.h"  // fill_rule_name
#include "store/record_store.h"
#include "temp_util.h"  // temp_path, temp_dir, remove_dir

using namespace hydra;
using app::fill_report::FillCompareRow;

namespace {

// Files `record`, analyzed under the given fill rule, for `hash` through the
// one save, under the profile the page reads for that rule.
void save(store::RecordStore& store, bool legacy_fills, const std::string& hash,
          HydraRecord record) {
    record.legacy_fills = legacy_fills;
    store::PreparedRecord prepared = store::prepare_record(
        hash, testdm::settings().profile_with_fills(fill_rule_for(legacy_fills)), record);
    store.save(prepared);
}

// A Ready result with no paths for `hash` under the given fill rule: an
// analysis that kept no path. No corpus chart is known to keep no path, so
// this record is built empty. It does not list the chart.
void put_no_paths(store::RecordStore& store, bool legacy_fills, const std::string& hash) {
    HydraRecord empty;
    empty.sp_cap = kCloneHeroSpCap;
    save(store, legacy_fills, hash, empty);
}

std::vector<FillCompareRow> compare(store::RecordStore& store) {
    return app::fill_report::collect_fill_rows(store, testdm::settings());
}

const FillCompareRow* find(const std::vector<FillCompareRow>& rows,
                           const std::string& hash) {
    for (const FillCompareRow& r : rows)
        if (r.hyhash == hash) return &r;
    return nullptr;
}


// ---- real corpus records ---------------------------------------------------

// The settings a corpus chart is analyzed under here: the fixture's settings
// under `rule`, at depth 0 as testdm::sample_chart is analyzed.
app::AnalysisSettings analysis_under(FillDeadlineRule rule) {
    app::Settings s = testdm::settings();
    s.legacy_fills = legacy_fills_for(rule);
    app::AnalysisSettings a = s.to_analysis_settings();
    a.depth_value = 0;
    return a;
}

// The library row the scan writes for the corpus chart at `notespath`: its id
// and fingerprint from corpus::scanned_entry, and its names from
// app::read_chart_meta, the reader the scan names a chart with.
store::ChartLibraryEntry scan_row(const std::string& notespath) {
    store::ChartLibraryEntry e = corpus::scanned_entry(notespath);
    const app::ChartMeta meta = app::read_chart_meta(notespath);
    e.title = meta.title;
    e.artist = meta.artist;
    e.charter = meta.charter;
    e.album = meta.album;
    return e;
}

// One corpus chart as the scan lists it, analyzed under each fill rule.
struct ScoredChart {
    store::ChartLibraryEntry entry;
    HydraRecord ch10;
    HydraRecord ch11;
};

// The first three corpus charts with paths under 1.1
// (corpus::charts_with_paths), each analyzed under 1.0 too. Throws, naming
// the corpus, when fewer than three have paths, and naming the chart when one
// fails or keeps no path under 1.0.
const std::vector<ScoredChart>& scored_charts() {
    static const std::vector<ScoredChart> charts = [] {
        std::vector<corpus::ChartWithPaths> found =
            corpus::charts_with_paths(analysis_under(FillDeadlineRule::Ch11), 3);
        if (found.size() < 3)
            throw std::runtime_error("fewer than three charts under " + corpus::root() +
                                     " analyze to a path under 1.1");
        std::vector<ScoredChart> out;
        for (corpus::ChartWithPaths& f : found) {
            ScoredChart c;
            c.entry = scan_row(f.chart);
            c.ch10 =
                app::analyze_chart_file(f.chart, analysis_under(FillDeadlineRule::Ch10)).record;
            if (c.ch10.paths.empty())
                throw std::runtime_error(f.chart + " has a path under 1.1 and none under 1.0");
            c.ch11 = std::move(f.result.record);
            out.push_back(std::move(c));
        }
        return out;
    }();
    return charts;
}

// Files `record`, an analysis under `rule`, for chart `md5`.
void file_under(store::RecordStore& store, FillDeadlineRule rule, const std::string& md5,
                const HydraRecord& record) {
    save(store, legacy_fills_for(rule), md5, record);
}

// Lists `c` alone as the store's library and files both its analyses.
void file_both(store::RecordStore& store, const ScoredChart& c) {
    store.rebuild_chart_library({c.entry});
    file_under(store, FillDeadlineRule::Ch10, c.entry.md5, c.ch10);
    file_under(store, FillDeadlineRule::Ch11, c.entry.md5, c.ch11);
}

// The status a chart scored under both rules carries: the sign of its 1.1
// score minus its 1.0 score, in fill_report.h's words.
std::string status_of_delta(int64_t delta) {
    if (delta > 0) return app::fill_report::kStatusNewHigher;
    if (delta < 0) return app::fill_report::kStatusOldHigher;
    return app::fill_report::kStatusSame;
}

// One side of a row against the record the store holds for that side
// (store::record_of is the save's own step from analysis to stored row).
void check_side(const std::optional<int64_t>& score, const std::string& path,
                const std::optional<int>& acts, const store::RecordRow& want) {
    CHECK(score == want.summary().score);
    CHECK(path == want.best_path_text());
    CHECK(acts == want.summary().actcount);
}

// A side that holds no record: no score, path or act count.
void check_empty_side(const std::optional<int64_t>& score, const std::string& path,
                      const std::optional<int>& acts) {
    CHECK_FALSE(score.has_value());
    CHECK(path.empty());
    CHECK_FALSE(acts.has_value());
}

// How many of `library`'s rows list chart `md5`.
int copies_in(const std::vector<store::ChartLibraryEntry>& library, const std::string& md5) {
    return static_cast<int>(
        std::count_if(library.begin(), library.end(),
                      [&](const store::ChartLibraryEntry& e) { return e.md5 == md5; }));
}

// A library holding the three scored charts in several folders, as a scan
// finds it: scored chart i in i + 1 folders, each copy its notes file (and
// song.ini, when it has one) in a scratch folder of its own, scanned by
// app::discover_charts as the app scans a library. Each chart's copy count
// differs, so a tally that counts one status in another's place is seen. The
// first chart is filed under both rules, the second under 1.0 only and the
// third under 1.1 only: batch runs under each rule that covered different
// charts. Returns the scan's rows.
std::vector<store::ChartLibraryEntry> file_copies(store::RecordStore& store) {
    namespace fs = std::filesystem;
    const std::vector<ScoredChart>& charts = scored_charts();
    const std::string root = testtemp::temp_path("fill_copies", "");
    testtemp::remove_dir(root);
    testtemp::temp_dir("fill_copies");
    for (size_t i = 0; i < charts.size(); ++i) {
        const fs::path from = hydra::os_path(charts[i].entry.notespath);
        const fs::path ini = from.parent_path() / "song.ini";
        for (size_t k = 0; k <= i; ++k) {
            const fs::path dir =
                hydra::os_path(root) / ("c" + std::to_string(i) + "_" + std::to_string(k));
            fs::create_directories(dir);
            fs::copy_file(from, dir / from.filename());
            if (fs::exists(ini)) fs::copy_file(ini, dir / "song.ini");
        }
    }
    auto [library, errors] = app::discover_charts({root});
    testtemp::remove_dir(root);
    if (!errors.empty()) throw std::runtime_error("scratch library scan error: " + errors.front());
    store.rebuild_chart_library(library);
    file_under(store, FillDeadlineRule::Ch10, charts[0].entry.md5, charts[0].ch10);
    file_under(store, FillDeadlineRule::Ch11, charts[0].entry.md5, charts[0].ch11);
    file_under(store, FillDeadlineRule::Ch10, charts[1].entry.md5, charts[1].ch10);
    file_under(store, FillDeadlineRule::Ch11, charts[2].entry.md5, charts[2].ch11);
    return library;
}

// Each status's count against the library rows of the charts `rows` give that
// status, and the total against every row the library holds.
void check_fill_tally(const app::fill_report::FillCompareStats& s,
                 const std::vector<FillCompareRow>& rows,
                 const std::vector<store::ChartLibraryEntry>& library) {
    auto rows_of = [&](const char* status) {
        int n = 0;
        for (const FillCompareRow& r : rows)
            if (r.status == status) n += copies_in(library, r.hyhash);
        return n;
    };
    CHECK(s.same == rows_of(app::fill_report::kStatusSame));
    CHECK(s.ch10_higher == rows_of(app::fill_report::kStatusOldHigher));
    CHECK(s.ch11_higher == rows_of(app::fill_report::kStatusNewHigher));
    CHECK(s.only_old == rows_of(app::fill_report::kStatusOnlyOld));
    CHECK(s.only_new == rows_of(app::fill_report::kStatusOnlyNew));
    CHECK(s.in_both == rows_of(app::fill_report::kStatusInBoth));
    CHECK(s.total == static_cast<int>(library.size()));
}

// `s` as the page's data writes a string: quoted and escaped.
std::string json_of(const std::string& s) {
    std::string out;
    app::html::json_escape_into(out, s);
    return out;
}

// A scored chart's analysis under `rule`.
const HydraRecord& record_under(const ScoredChart& c, FillDeadlineRule rule) {
    return rule == FillDeadlineRule::Ch10 ? c.ch10 : c.ch11;
}

// The sentence report::nothing_under_settings gives for `s`'s chart mode and
// cap, the profile generate_fill_report reads under.
std::string nothing_reason(const app::Settings& s) {
    const store::Profile p = s.profile_with_fills(FillDeadlineRule::Ch11);
    return app::report::nothing_under_settings(p.chartmode, p.cap.exact);
}

// file_copies' library, the rows collect_fill_rows joins from it, and the
// page generate_fill_report makes of them. Throws when there is no page.
struct CopiesPage {
    std::vector<store::ChartLibraryEntry> library;
    std::vector<FillCompareRow> rows;
    std::string html;
};
CopiesPage copies_page() {
    store::RecordStore store(":memory:");
    CopiesPage p;
    p.library = file_copies(store);
    p.rows = compare(store);
    p.html = app::fill_report::generate_fill_report(store, testdm::settings()).html;
    if (p.html.empty()) throw std::runtime_error("file_copies' library made no fill page");
    return p;
}

}  // namespace

// Each corpus chart's row joins the store's two real records for it: the 1.0
// and 1.1 scores, best paths and act counts, the note count, the 1.1 score
// minus the 1.0 score, and the status that difference's sign names.
TEST_CASE("collect_fill_rows: each row's delta and status follow its two real scores") {
    store::RecordStore store(":memory:");
    const std::vector<ScoredChart>& charts = scored_charts();
    std::vector<store::ChartLibraryEntry> library;
    for (const ScoredChart& c : charts) library.push_back(c.entry);
    store.rebuild_chart_library(library);
    for (const ScoredChart& c : charts) {
        file_under(store, FillDeadlineRule::Ch10, c.entry.md5, c.ch10);
        file_under(store, FillDeadlineRule::Ch11, c.entry.md5, c.ch11);
    }

    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == charts.size());
    for (const ScoredChart& c : charts) {
        CAPTURE(c.entry.notespath);
        const FillCompareRow* r = find(rows, c.entry.md5);
        REQUIRE(r != nullptr);
        const store::RecordRow old_rec = store::record_of(c.entry.md5, c.ch10);
        const store::RecordRow new_rec = store::record_of(c.entry.md5, c.ch11);
        check_side(r->old_score, r->old_path, r->old_acts, old_rec);
        check_side(r->new_score, r->new_path, r->new_acts, new_rec);
        CHECK(r->notes == new_rec.summary().notecount);
        REQUIRE(r->old_score.has_value());
        REQUIRE(r->new_score.has_value());
        REQUIRE(r->delta.has_value());
        CHECK(*r->delta == *r->new_score - *r->old_score);
        CHECK(r->status == status_of_delta(*r->delta));
    }
}

TEST_CASE("collect_fill_rows: a chart with a record under one rule only still gets a row") {
    store::RecordStore store(":memory:");
    const ScoredChart& old_only = scored_charts()[0];
    const ScoredChart& new_only = scored_charts()[1];
    store.rebuild_chart_library({old_only.entry, new_only.entry});
    // A 1.0 batch run that covered one chart and a 1.1 run that covered the
    // other.
    file_under(store, FillDeadlineRule::Ch10, old_only.entry.md5, old_only.ch10);
    file_under(store, FillDeadlineRule::Ch11, new_only.entry.md5, new_only.ch11);

    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 2);

    const FillCompareRow* o = find(rows, old_only.entry.md5);
    REQUIRE(o != nullptr);
    CHECK(o->status == app::fill_report::kStatusOnlyOld);
    check_side(o->old_score, o->old_path, o->old_acts,
               store::record_of(old_only.entry.md5, old_only.ch10));
    check_empty_side(o->new_score, o->new_path, o->new_acts);
    CHECK_FALSE(o->delta.has_value());

    const FillCompareRow* n = find(rows, new_only.entry.md5);
    REQUIRE(n != nullptr);
    CHECK(n->status == app::fill_report::kStatusOnlyNew);
    check_side(n->new_score, n->new_path, n->new_acts,
               store::record_of(new_only.entry.md5, new_only.ch11));
    check_empty_side(n->old_score, n->old_path, n->old_acts);
    CHECK_FALSE(n->delta.has_value());
}

// The page reads report::library_lookup's Ready records, so a stored failure
// is no record. Every chart in testdata/input has Expert Pro Drums notes, so
// the failing chart is the checked-in edge chart whose Expert drums section
// never closes: testdata/parse_edge/expected_expert.tsv pins it as a
// NoNotesError, and the fill rule never changes what the chart's notes are.
// Its failures under both rules are filed as the batch files them
// (app::prepare_failed_chart), and it gets no row.
TEST_CASE("collect_fill_rows: a chart that failed under both rules gets no row") {
    store::RecordStore store(":memory:");
    const ScoredChart& scored = scored_charts()[0];
    const store::ChartLibraryEntry failing =
        scan_row(corpus::testdata_root() + "\\parse_edge\\c15_unterminated_drums.chart");
    store.rebuild_chart_library({scored.entry, failing});
    file_under(store, FillDeadlineRule::Ch10, scored.entry.md5, scored.ch10);
    for (FillDeadlineRule rule : {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11}) {
        std::optional<store::PreparedRecord> failure;
        try {
            (void)app::analyze_chart_file(failing.notespath, analysis_under(rule));
        } catch (const std::exception& e) {
            failure = app::prepare_failed_chart(
                failing.md5, testdm::settings().profile_with_fills(rule), e);
        }
        REQUIRE(failure.has_value());
        store.save(*failure);
    }

    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].hyhash == scored.entry.md5);
    CHECK(rows[0].status == app::fill_report::kStatusOnlyOld);
}

// One store holding both rules' results for a chart, as the app's own
// database does once "1.0 fills" has been used, gives one row, named from the
// names the library scan read for the chart.
TEST_CASE("collect_fill_rows: both rules' records in one database give one row named from its "
          "library") {
    store::RecordStore store(":memory:");
    const ScoredChart& c = scored_charts()[0];
    file_both(store, c);

    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].hyhash == c.entry.md5);
    CHECK(rows[0].song == display_title(c.entry.title));
    CHECK(rows[0].artist == display_artist(c.entry.artist));
    CHECK(rows[0].charter == display_charter(c.entry.charter));
    CHECK(rows[0].copies == 1);
}

// D79 item 3: each status counts its charts' library rows.
TEST_CASE("tally_fill_rows counts every status by its charts' library rows") {
    store::RecordStore store(":memory:");
    const std::vector<store::ChartLibraryEntry> library = file_copies(store);
    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == scored_charts().size());
    check_fill_tally(app::fill_report::tally_fill_rows(rows, store.chart_index()), rows, library);
}

// D79 item 3 and D115: a chart the scan finds in several folders counts each
// copy, in its row and in the report's tally.
TEST_CASE("collect_fill_rows and generate_fill_report count each chart's library copies") {
    store::RecordStore store(":memory:");
    const std::vector<store::ChartLibraryEntry> library = file_copies(store);
    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == scored_charts().size());
    for (const FillCompareRow& r : rows) {
        CAPTURE(r.hyhash);
        CHECK(r.copies == copies_in(library, r.hyhash));
    }
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    CHECK(result.reason.empty());
    check_fill_tally(result.stats, rows, library);
}

// The page's chart tiles add up each row's "k", so each row's data carries its
// chart's library copies, and the script's STATUS object carries every
// status word fill_report.h gives a row.
TEST_CASE("fill page: each row's data carries its library copies and the script every status") {
    store::RecordStore store(":memory:");
    const std::vector<store::ChartLibraryEntry> library = file_copies(store);
    const std::vector<FillCompareRow> rows = compare(store);
    const std::string html = app::fill_report::generate_fill_report(store, testdm::settings()).html;
    REQUIRE_FALSE(html.empty());
    for (const FillCompareRow& r : rows) {
        CAPTURE(r.hyhash);
        std::string song;
        app::html::json_escape_into(song, r.song);
        CHECK(html.find("{\"k\":" + std::to_string(copies_in(library, r.hyhash)) +
                        ",\"song\":" + song) != std::string::npos);
    }
    for (const char* status :
         {app::fill_report::kStatusSame, app::fill_report::kStatusOldHigher,
          app::fill_report::kStatusNewHigher, app::fill_report::kStatusOnlyOld,
          app::fill_report::kStatusOnlyNew, app::fill_report::kStatusInBoth}) {
        CAPTURE(status);
        std::string word;
        app::html::json_escape_into(word, status);
        CHECK(html.find(word) != std::string::npos);
    }
}

// The page made from a real chart's two records leaves no __NAME__
// placeholder, and carries both best paths, both rules' short names and the
// report windows' search hint.
TEST_CASE("generate_fill_report fills every placeholder of a real chart's page") {
    store::RecordStore store(":memory:");
    file_both(store, scored_charts()[0]);
    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 1);
    const std::string html = app::fill_report::generate_fill_report(store, testdm::settings()).html;
    REQUIRE_FALSE(html.empty());

    CHECK_FALSE(std::regex_search(html, std::regex("__[A-Z][A-Z_]*__")));
    for (const std::string& path : {rows[0].old_path, rows[0].new_path}) {
        std::string escaped;
        app::html::json_escape_into(escaped, path);
        CHECK(html.find(escaped) != std::string::npos);
    }
    for (FillDeadlineRule rule : {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11})
        CHECK(html.find(fill_rule_name(rule, FillRuleNameStyle::Short)) != std::string::npos);
    CHECK(html.find(app::html::html_escape(app::html::charts_search_hint())) != std::string::npos);
}

// D114 item 3: each text column's data carries its text as the windows fold
// it (report_view::folded_key), so the page sorts the way the windows do.
TEST_CASE("build_fill_html: text columns sort on the windows' folded text") {
    const CopiesPage page = copies_page();
    REQUIRE(page.rows.size() == scored_charts().size());
    bool folds_something = false;
    for (const FillCompareRow& r : page.rows) {
        CAPTURE(r.hyhash);
        for (const auto& [field, text] :
             {std::pair<std::string, const std::string*>{"song_s", &r.song},
              {"artist_s", &r.artist}, {"charter_s", &r.charter}, {"p10_s", &r.old_path},
              {"p11_s", &r.new_path}, {"status_s", &r.status}}) {
            const std::string column = field;
            CAPTURE(column);
            const std::string folded = std::get<std::string>(
                app::report_view::folded_key(app::report_view::SortKey{*text}));
            if (folded != *text) folds_something = true;
            CHECK(page.html.find(",\"" + field + "\":" + json_of(folded)) != std::string::npos);
        }
    }
    // A corpus name has a capital the fold lowers, so a column left on its
    // raw text would not match its folded field.
    CHECK(folds_something);
}

// D114 item 4: the count line's total is counted's wording of the library's
// copies, so one chart reads "1 of 1 chart", as the windows' count lines do.
TEST_CASE("build_fill_html: the count line's total is singular at one chart") {
    store::RecordStore store(":memory:");
    file_both(store, scored_charts()[0]);
    const std::string one =
        app::fill_report::generate_fill_report(store, testdm::settings()).html;
    CHECK(one.find(json_of(counted(1, "chart", "charts"))) != std::string::npos);

    // file_copies' library holds each scored chart in several folders.
    const CopiesPage many = copies_page();
    CHECK(many.html.find(json_of(counted(static_cast<int64_t>(many.library.size()), "chart",
                                         "charts"))) != std::string::npos);
}

// The search text of each row is the Library's (search_field) over the
// row's song, artist and charter, so the page searches as the Library does.
TEST_CASE("report payload: each row's search field is the Library's search_field") {
    const CopiesPage page = copies_page();
    REQUIRE_FALSE(page.rows.empty());
    for (const FillCompareRow& r : page.rows) {
        CAPTURE(r.hyhash);
        CHECK(page.html.find(",\"search\":" +
                             json_of(app::search_field(r.song, r.artist, r.charter))) !=
              std::string::npos);
    }
}

TEST_CASE("generate_fill_report: tally and framing behind one seam") {
    store::RecordStore store(":memory:");
    const std::vector<store::ChartLibraryEntry> library = file_copies(store);
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    check_fill_tally(result.stats, compare(store), library);
    CHECK_FALSE(result.html.empty());
    CHECK(result.reason.empty());

    // An empty database: zero stats, no page, and the settings sentence
    // (finding 105, D50 item 3), so hydra_fillcompare prints what the seam
    // says instead of deciding it again.
    store::RecordStore empty(":memory:");
    const app::fill_report::GeneratedFillReport none =
        app::fill_report::generate_fill_report(empty, testdm::settings());
    CHECK(none.stats.total == 0);
    CHECK(none.html.empty());
    CHECK(none.reason == nothing_reason(testdm::settings()));

    // Records stored at Clone Hero's cap, asked at cap 8: no page, and the
    // sentence names the cap asked for.
    const app::Settings at8 = testdm::settings(testdm::kMode, 8);
    const app::fill_report::GeneratedFillReport off =
        app::fill_report::generate_fill_report(store, at8);
    CHECK(off.html.empty());
    CHECK(off.reason == nothing_reason(at8));
    CHECK(off.reason != none.reason);
}

// The page's song, artist and charter are the three name owners' answers for
// the names the scan read. The corpus chart is one whose song.ini names its
// artist and charter with Clone Hero colour tags.
TEST_CASE("collect_fill_rows: the names go through display_title, display_artist and "
          "display_charter") {
    const std::string chart = corpus::chart_named("SoundHaven - Focus");
    REQUIRE_FALSE(chart.empty());
    const store::ChartLibraryEntry entry = scan_row(chart);
    store::RecordStore store(":memory:");
    store.rebuild_chart_library({entry});
    for (FillDeadlineRule rule : {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11})
        file_under(store, rule, entry.md5,
                   app::analyze_chart_file(chart, analysis_under(rule)).record);

    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].song == display_title(entry.title));
    CHECK(rows[0].artist == display_artist(entry.artist));
    CHECK(rows[0].charter == display_charter(entry.charter));
    // The owners strip the tags, so a raw name on the row would not match.
    CHECK(display_artist(entry.artist) != entry.artist);
    CHECK(display_charter(entry.charter) != entry.charter);
}

// The subtitle words one chart with counted's singular noun.
TEST_CASE("generate_fill_report: one chart reads \"1 chart\" in the subtitle") {
    store::RecordStore store(":memory:");
    file_both(store, scored_charts()[0]);
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    REQUIRE(result.stats.total == 1);
    const std::string chartmode =
        testdm::settings().profile_with_fills(FillDeadlineRule::Ch11).chartmode;
    CHECK(result.html.find(app::html::html_escape(counted(1, "chart", "charts") + " in " +
                                                  chartmode + ": ")) != std::string::npos);
}

// D50 item 2: a chart with a record under each rule, one of which kept no
// path, is "in both" whichever side lacks the path, and the tally counts it
// there.
TEST_CASE("collect_fill_rows: a record under both rules with a score under one is in both") {
    const ScoredChart& c = scored_charts()[0];
    for (FillDeadlineRule scored : {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11}) {
        const std::string scored_name = fill_rule_name(scored, FillRuleNameStyle::Short);
        CAPTURE(scored_name);
        const FillDeadlineRule unscored =
            scored == FillDeadlineRule::Ch10 ? FillDeadlineRule::Ch11 : FillDeadlineRule::Ch10;
        store::RecordStore store(":memory:");
        store.rebuild_chart_library({c.entry});
        file_under(store, scored, c.entry.md5, record_under(c, scored));
        put_no_paths(store, legacy_fills_for(unscored), c.entry.md5);

        const std::vector<FillCompareRow> rows = compare(store);
        REQUIRE(rows.size() == 1);
        const FillCompareRow& r = rows[0];
        CHECK(r.status == app::fill_report::kStatusInBoth);
        CHECK_FALSE(r.delta.has_value());
        const store::RecordRow want = store::record_of(c.entry.md5, record_under(c, scored));
        if (scored == FillDeadlineRule::Ch10) {
            check_side(r.old_score, r.old_path, r.old_acts, want);
            CHECK_FALSE(r.new_score.has_value());
        } else {
            check_side(r.new_score, r.new_path, r.new_acts, want);
            CHECK_FALSE(r.old_score.has_value());
        }
        CHECK(app::fill_report::tally_fill_rows(rows, store.chart_index()).in_both ==
              copies_in(store.list_chart_library(0, -1), c.entry.md5));
    }
}

// Every chart lands in exactly one part, the "in both" charts included, so
// the parts add up to the total. D114 item 2 words that part kInBothPhrase.
TEST_CASE("generate_fill_report: a score on one side only is counted, and the parts add up") {
    store::RecordStore store(":memory:");
    const std::vector<store::ChartLibraryEntry> library = file_copies(store);
    // file_copies filed the second chart under 1.0 only; a 1.1 record that
    // kept no path puts it in both, with a score on one side only.
    const std::string& one_sided = scored_charts()[1].entry.md5;
    put_no_paths(store, legacy_fills_for(FillDeadlineRule::Ch11), one_sided);

    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    const app::fill_report::FillCompareStats& s = result.stats;
    check_fill_tally(s, compare(store), library);
    CHECK(s.in_both == copies_in(library, one_sided));
    CHECK(s.same + s.ch10_higher + s.ch11_higher + s.only_old + s.only_new + s.in_both ==
          s.total);
    // The subtitle names that part's count with its phrase.
    CHECK(result.html.find(app::html::html_escape(group_thousands(s.in_both) + " " +
                                                  app::fill_report::kInBothPhrase)) !=
          std::string::npos);
}

// ---- results the library doesn't list (D92) ---------------------------------

TEST_CASE("generate_fill_report: a database with no chart library stops with the path "
          "report's sentence") {
    // A real chart's analyses under both rules, as hydra_batch with folder
    // arguments files them, and no library.
    store::RecordStore store(":memory:");
    const ScoredChart& c = scored_charts()[0];
    for (FillDeadlineRule rule : {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11})
        file_under(store, rule, c.entry.md5, record_under(c, rule));
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    CHECK(result.html.empty());
    CHECK(result.reason == app::report::kNoChartLibrary);
}

TEST_CASE("collect_fill_rows: a result whose chart the library doesn't list is left out") {
    // The library lists the first scored chart only; every scored chart's
    // analyses under both rules are filed.
    store::RecordStore store(":memory:");
    const std::vector<ScoredChart>& charts = scored_charts();
    store.rebuild_chart_library({charts[0].entry});
    for (const ScoredChart& c : charts)
        for (FillDeadlineRule rule : {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11})
            file_under(store, rule, c.entry.md5, record_under(c, rule));

    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].hyhash == charts[0].entry.md5);
    for (size_t i = 1; i < charts.size(); ++i) CHECK(find(rows, charts[i].entry.md5) == nullptr);
}
