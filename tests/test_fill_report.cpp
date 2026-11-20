// Tests for app/fill_report: the CH 1.0 vs CH 1.1 join (collect_fill_rows),
// the tally, and the comparison page (build_fill_html).
//
// Every case here uses one in-memory store holding results under each fill
// rule. Each result carries its rule in its profile (docs/adr/0010), and the
// join reads the 1.0 results and the 1.1 results from that one store. The
// cases that read scored_charts file real corpus analyses under both rules,
// listed as the library scan lists them; the rest file typed scores.

#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings::profile_with_fills
#include "app/fill_report.h"
#include "app/html_page.h"  // json_escape_into, html_escape, charts_search_hint
#include "app/report.h"  // kNoChartLibrary
#include "core/model.h"
#include "corpus_util.h"
#include "display_fixtures.h"  // kTagOnlyTitle
#include "dm_fixture.h"  // testdm::settings, sample_chart
#include "parse/song.h"
#include "search/graph.h"  // fill_rule_name
#include "store/record_store.h"
#include "temp_util.h"  // temp_path, temp_dir, remove_dir

using namespace hydra;
using app::fill_report::FillCompareRow;

namespace {

constexpr const char* kBoth = "aa11bb22cc33dd44ee55ff6677889900";
constexpr const char* kOldOnly = "bb22cc33dd44ee55ff6677889900aa11";
constexpr const char* kNewOnly = "cc33dd44ee55ff6677889900aa11bb22";

// Lists `hash` in `store`'s library as "Song <first four>", unless the
// library already lists it (a test that set up its own copies keeps them).
void name(store::RecordStore& store, const std::string& hash) {
    if (store.chart_index().lists(hash)) return;
    test::name_chart(store, hash, "Song " + hash.substr(0, 4), "Test Artist", "Test Charter");
}

// Files `record`, analyzed under the given fill rule, for `hash` through the
// one save, under the profile the page reads for that rule.
void save(store::RecordStore& store, bool legacy_fills, const std::string& hash,
          HydraRecord record, const std::function<void(store::PreparedRecord&)>& edit = {}) {
    record.legacy_fills = legacy_fills;
    store::PreparedRecord prepared = store::prepare_record(
        hash, testdm::settings().profile_with_fills(fill_rule_for(legacy_fills)), record);
    if (edit) edit(prepared);
    store.save(prepared);
}

// Lists `hash` in `store`'s library and files a result for it under the given
// fill rule with exactly the best path given. The record comes from a real
// analysis so it is well-formed; only the numbers the report reads are
// overridden.
void put(store::RecordStore& store, bool legacy_fills, const std::string& hash,
         int64_t score, int acts, const std::string& bestpath) {
    name(store, hash);
    save(store, legacy_fills, hash, testdm::sample_chart().record, [&](store::PreparedRecord& p) {
        store::PathRow& best = p.record.paths.front();
        best.path = bestpath;
        best.score = score;
        best.actcount = acts;
        p.record.notecount = 1234;
    });
}
// A Clone Hero 1.0 result, and a 1.1 one.
void put_ch10(store::RecordStore& store, const std::string& hash, int64_t score, int acts,
              const std::string& bestpath) {
    put(store, true, hash, score, acts, bestpath);
}
void put_ch11(store::RecordStore& store, const std::string& hash, int64_t score, int acts,
              const std::string& bestpath) {
    put(store, false, hash, score, acts, bestpath);
}

// A Ready result with no paths for `hash` under the given fill rule: an
// analysis that kept no path. It does not list the chart.
void put_no_paths(store::RecordStore& store, bool legacy_fills, const std::string& hash) {
    HydraRecord empty;
    empty.sp_cap = kCloneHeroSpCap;
    save(store, legacy_fills, hash, empty);
}

std::vector<FillCompareRow> compare(store::RecordStore& store) {
    return app::fill_report::collect_fill_rows(store, testdm::settings());
}

// A library index listing each id with the given number of copies.
store::ChartIndex index_of(const std::vector<std::pair<std::string, int>>& charts) {
    std::vector<store::ChartLibraryEntry> rows;
    for (const auto& [id, copies] : charts)
        for (int i = 0; i < copies; ++i) {
            store::ChartLibraryEntry e;
            e.md5 = id;
            e.notespath = "C:\\songs" + std::to_string(i) + "\\" + id + "\\notes.chart";
            rows.push_back(e);
        }
    return store::ChartIndex::from_rows(rows);
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

// D114 item 3: each text column carries the windows' folded sort text, and
// the script compares those fields as they are, with no browser collation.
TEST_CASE("build_fill_html: text columns sort on the windows' folded text") {
    FillCompareRow r;
    r.song = "\xC3\x89tude";  // Étude
    r.artist = "ABBA";
    r.charter = "Me";
    r.old_path = "0 1 3 0+";
    r.new_path = "0 1 3 0- 0";
    r.status = app::fill_report::kStatusSame;
    const std::string html = app::fill_report::build_fill_html({r}, store::ChartIndex{}, "sub", "foot");
    CHECK(html.find(",\"song_s\":\"etude\",\"artist_s\":\"abba\",\"charter_s\":\"me\","
                    "\"p10_s\":\"0 1 3 0+\",\"p11_s\":\"0 1 3 0- 0\",\"status_s\":\"same\"}") !=
          std::string::npos);
    for (const char* sort : {"sort:'song_s'", "sort:'artist_s'", "sort:'charter_s'",
                             "sort:'p10_s'", "sort:'p11_s'", "sort:'status_s'"}) {
        CAPTURE(sort);
        CHECK(html.find(sort) != std::string::npos);
    }
    CHECK(html.find("localeCompare") == std::string::npos);
    CHECK(html.find("const field = COLS.find(c => c.k === sortKey).sort || sortKey;") !=
          std::string::npos);
}

// D114 item 4: one chart reads "1 of 1 chart", as the windows' count lines do.
TEST_CASE("build_fill_html: the count line's total is singular at one chart") {
    FillCompareRow r;
    r.status = app::fill_report::kStatusSame;
    r.hyhash = "h0";
    r.copies = 1;
    const std::string one =
        app::fill_report::build_fill_html({r}, index_of({{"h0", 1}}), "sub", "foot");
    CHECK(one.find("  total: \"1 chart\",\n") != std::string::npos);
    CHECK(one.find("__COUNT_TOTAL__") == std::string::npos);
    CHECK(one.find("PAGE.noun") == std::string::npos);
    r.copies = 1000;
    const std::string many =
        app::fill_report::build_fill_html({r}, index_of({{"h0", 1000}}), "sub", "foot");
    CHECK(many.find("  total: \"1,000 charts\",\n") != std::string::npos);
}

TEST_CASE("build_fill_html colours and sums the delta from the status") {
    // collect_fill_rows already decided which side is higher when it set the
    // row's status; the cell's colour, its "+" and the gains and losses tiles
    // read that answer instead of testing the delta's sign again.
    const std::string html =
        app::fill_report::build_fill_html({}, store::ChartIndex{}, "sub", "foot");
    CHECK(html.find("r.status === STATUS.newHigher") != std::string::npos);
    CHECK(html.find("r.status === STATUS.oldHigher") != std::string::npos);
    CHECK(html.find("(r.status === STATUS.newHigher ? '+' : '') + fmt(r.delta)") !=
          std::string::npos);
    CHECK(html.find("const gains = rows.filter(r => r.status === STATUS.newHigher)") !=
          std::string::npos);
    CHECK(html.find("const losses = rows.filter(r => r.status === STATUS.oldHigher)") !=
          std::string::npos);
    CHECK(html.find("r.delta > 0 ?") == std::string::npos);
    CHECK(html.find("r.delta < 0 ?") == std::string::npos);
}

TEST_CASE("report payload: the search field is folded and tag-free") {
    FillCompareRow row;
    row.song = "Halo";
    row.artist = "Beyonc\xc3\xa9";  // Beyoncé
    row.charter = "<b>Bob</b>";
    row.status = "only 1.1";
    const std::string html =
        app::fill_report::build_fill_html({row}, store::ChartIndex{}, "sub", "foot");
    CHECK(html.find("\"search\":\"halo beyonce bob\"") != std::string::npos);
}

TEST_CASE("generate_fill_report: tally and framing behind one seam") {
    store::RecordStore store(":memory:");
    put_ch10(store, kBoth, 1000000, 3, "old-path-D");
    put_ch11(store, kBoth, 1050000, 4, "new-path-D");   // 1.1 higher
    put_ch10(store, kOldOnly, 900000, 2, "only-old");   // only 1.0
    put_ch11(store, kNewOnly, 800000, 5, "only-new");   // only 1.1

    app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    CHECK(result.stats.total == 3);
    CHECK(result.stats.ch11_higher == 1);
    CHECK(result.stats.only_old == 1);
    CHECK(result.stats.only_new == 1);
    CHECK(result.stats.same == 0);
    CHECK(result.stats.ch10_higher == 0);
    CHECK_FALSE(result.html.empty());
    CHECK(result.html.find("score higher under 1.1") != std::string::npos);

    // An empty database: zero stats, no page.
    store::RecordStore empty(":memory:");
    app::fill_report::GeneratedFillReport none =
        app::fill_report::generate_fill_report(empty, testdm::settings());
    CHECK(none.stats.total == 0);
    CHECK(none.html.empty());
    // The empty page's reason comes from here, so hydra_fillcompare prints
    // what the seam says instead of deciding it again. A truly empty database
    // gets the settings sentence too (finding 105, D50 item 3).
    CHECK(none.reason ==
          "Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, SP cap 4). "
          "Analyze with these settings, or change them.");
    CHECK(result.reason.empty());

    // Records stored at cap 4, asked at cap 8: the sentence names cap 8.
    app::fill_report::GeneratedFillReport off =
        app::fill_report::generate_fill_report(store, testdm::settings(testdm::kMode, 8));
    CHECK(off.html.empty());
    CHECK(off.reason ==
          "Nothing is analyzed under these settings (Expert Pro Drums, 2x Bass, SP cap 8). "
          "Analyze with these settings, or change them.");
}

TEST_CASE("collect_fill_rows: the names go through display_title, display_artist and "
          "display_charter") {
    // test_song_input.cpp pins what those three do. This checks only that the
    // page calls them: one tagged name each.
    store::RecordStore store(":memory:");
    put_ch10(store, kBoth, 1000000, 3, "old-path");
    put_ch11(store, kBoth, 1000000, 3, "new-path");
    test::name_chart(store, kBoth, "<b>Bold Title</b>", test::kTagOnlyTitle, " <b>Bob</b> ");

    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].song == "Bold Title");
    CHECK(rows[0].artist == kUnknownTitle);
    CHECK(rows[0].charter == "Bob");
}

TEST_CASE("generate_fill_report: one chart reads \"1 chart\" in the subtitle") {
    store::RecordStore store(":memory:");
    put_ch10(store, kBoth, 1000000, 3, "old-path-one");
    put_ch11(store, kBoth, 1000000, 3, "new-path-one");
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    CHECK(result.html.find("1 chart in ") != std::string::npos);
    CHECK(result.html.find("1 charts") == std::string::npos);
}

TEST_CASE("collect_fill_rows: a record under both rules with a score under one is in both") {
    // D50 item 2: the store holds a record for the chart under each rule, but
    // the 1.0 record is a Ready record with no paths, so it has no score. The
    // row is listed under "in both", not under "only 1.1".
    store::RecordStore store(":memory:");
    name(store, kBoth);
    put_no_paths(store, true, kBoth);
    put_ch11(store, kBoth, 1050000, 4, "new-path-F");

    std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 1);
    CHECK_FALSE(rows[0].old_score.has_value());
    CHECK(rows[0].new_score == 1050000);
    CHECK_FALSE(rows[0].delta.has_value());
    CHECK(rows[0].status == "in both");

    // The same the other way round: the 1.1 record has no score.
    store::RecordStore other(":memory:");
    put_ch10(other, kBoth, 1000000, 3, "old-path-F");
    put_no_paths(other, false, kBoth);
    rows = compare(other);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].status == "in both");

    // The tally counts it by name, and the page lists it under "In both" and
    // writes "no paths" on the side whose analysis kept none (D51 call 11).
    const store::ChartIndex library = other.chart_index();
    CHECK(app::fill_report::tally_fill_rows(rows, library).in_both == 1);
    const std::string html = app::fill_report::build_fill_html(rows, library, "sub", "foot");
    CHECK(html.find("<option value=\"in both\">In both</option>") != std::string::npos);
    CHECK(html.find("__ST_") == std::string::npos);
    CHECK(html.find("'no paths'") != std::string::npos);
    CHECK(html.find("'no score'") == std::string::npos);
}

TEST_CASE("generate_fill_report: a score on one side only is counted, and the parts add up") {
    // The subtitle and the tiles count the "in both" rows as one part, so
    // every chart lands in exactly one part. D114 item 2 words that part "in
    // both, not both scored".
    constexpr const char* kOneSided = "dd44ee55ff6677889900aa11bb22cc33";
    store::RecordStore store(":memory:");
    put_ch10(store, kBoth, 1000000, 3, "old-path-G");
    put_ch11(store, kBoth, 1050000, 4, "new-path-G");   // 1.1 higher
    put_ch10(store, kOldOnly, 900000, 2, "only-old");   // only 1.0
    put_ch11(store, kNewOnly, 800000, 5, "only-new");   // only 1.1
    // A record under each rule, but the 1.0 one has no paths and so no score.
    name(store, kOneSided);
    put_no_paths(store, true, kOneSided);
    put_ch11(store, kOneSided, 700000, 2, "one-sided");

    app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    const app::fill_report::FillCompareStats& s = result.stats;
    CHECK(s.total == 4);
    CHECK(s.in_both == 1);
    CHECK(s.same + s.ch10_higher + s.ch11_higher + s.only_old + s.only_new + s.in_both ==
          s.total);

    // The subtitle names the part after "under one rule only".
    CHECK(result.html.find(
              "4 charts in Expert Pro Drums, 2x Bass: 1 score higher under 1.1, "
              "0 higher under 1.0, 0 unchanged, 2 under one rule only, "
              "1 in both, not both scored") != std::string::npos);
    // The tiles get a matching one after "Under one rule only", counted from
    // the rows the table shows, like the tiles before it.
    CHECK(result.html.find("['Under one rule only', fmt(n(STATUS.onlyOld) + n(STATUS.onlyNew))],\n"
                           "      [\"In both, not both scored\", fmt(n(STATUS.inBoth))],") !=
          std::string::npos);
    CHECK(result.html.find("Score on one side only") == std::string::npos);
}

// ---- results the library doesn't list (D92) ---------------------------------

TEST_CASE("generate_fill_report: a database with no chart library stops with the path "
          "report's sentence") {
    // Results under both rules, as hydra_batch with folder arguments leaves
    // them, and no library.
    store::RecordStore store(":memory:");
    put_no_paths(store, true, kBoth);
    put_no_paths(store, false, kBoth);
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(store, testdm::settings());
    CHECK(result.html.empty());
    CHECK(result.reason == app::report::kNoChartLibrary);
}

TEST_CASE("collect_fill_rows: a result whose chart the library doesn't list is left out") {
    store::RecordStore store(":memory:");
    put_ch10(store, kBoth, 1000000, 3, "old-path-M");
    put_ch11(store, kBoth, 1050000, 4, "new-path-M");
    // A result under each rule for charts the library doesn't list.
    put_no_paths(store, true, kOldOnly);
    put_no_paths(store, false, kNewOnly);

    const std::vector<FillCompareRow> rows = compare(store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].hyhash == kBoth);
    CHECK(find(rows, kOldOnly) == nullptr);
    CHECK(find(rows, kNewOnly) == nullptr);
}
