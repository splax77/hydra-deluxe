// Tests for app/fill_report: the CH 1.0 vs CH 1.1 join (collect_fill_rows),
// the tally, and the comparison page (build_fill_html). Pins the status
// strings as raw literals -- tally_fill_rows and the page's chip-color map
// both compare them.
//
// Every case here uses two in-memory stores holding the same chart hashes at
// different scores: 1.0 results in the old one, 1.1 results in the new one.
// Each result carries its rule in its key (docs/adr/0010), and the join reads
// only 1.0 results from the old side and 1.1 results from the new.

#include "doctest.h"

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings::profile_with_fills
#include "app/fill_report.h"
#include "app/report.h"  // kNoChartLibrary
#include "core/model.h"
#include "corpus_util.h"
#include "display_fixtures.h"  // kTagOnlyTitle
#include "parse/song.h"
#include "search/graph.h"  // fill_rule_name
#include "store/record_store.h"

using namespace hydra;
using app::fill_report::FillCompareRow;

namespace {

constexpr const char* kMode = "Expert Pro Drums, 2x Bass";
constexpr const char* kBoth = "aa11bb22cc33dd44ee55ff6677889900";
constexpr const char* kOldOnly = "bb22cc33dd44ee55ff6677889900aa11";
constexpr const char* kNewOnly = "cc33dd44ee55ff6677889900aa11bb22";

// The settings every case compares under: kMode at Clone Hero's cap, with the
// ms limit off, as the corpus analysis ran. `cap` moves the SP cap.
app::Settings settings(int cap = kCloneHeroSpCap) {
    app::Settings s = app::Settings{}.with_known_chartmode(kMode);
    s.mslimit_enabled = false;
    s.sp_cap = cap;
    return s;
}

// One analyzed corpus chart, kept alive for the whole file: it supplies a real
// record to build records from. The scores themselves are overridden per
// row, so which chart it is doesn't matter.
const app::AnalysisResult& sample_chart() {
    static const app::AnalysisResult result = [] {
        app::AnalysisSettings settings;
        settings.depth_value = 0;
        return corpus::first_analyzed_with_paths(settings);
    }();
    return result;
}

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
        hash, settings().profile_with_fills(fill_rule_for(legacy_fills)), record);
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
    save(store, legacy_fills, hash, sample_chart().record, [&](store::PreparedRecord& p) {
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

std::vector<FillCompareRow> compare(store::RecordStore& old_store,
                                    store::RecordStore& new_store) {
    return app::fill_report::collect_fill_rows(old_store, new_store, settings());
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

}  // namespace

TEST_CASE("collect_fill_rows: delta sign and status literals") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");

    // 1.1 scores higher: the rare long-fill gain.
    put_ch10(old_store,kBoth, 1000000, 3, "old-path-A");
    put_ch11(new_store,kBoth, 1050000, 4, "new-path-A");

    std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    const FillCompareRow& r = rows[0];

    CHECK(r.old_score == 1000000);
    CHECK(r.new_score == 1050000);
    REQUIRE(r.delta.has_value());
    CHECK(*r.delta == 50000);  // new - old
    CHECK(r.status == "1.1 higher");

    // Both sides' paths and act counts survive the join.
    CHECK(r.old_path == "old-path-A");
    CHECK(r.new_path == "new-path-A");
    CHECK(r.old_acts == 3);
    CHECK(r.new_acts == 4);
    CHECK(r.notes == 1234);
}

TEST_CASE("collect_fill_rows: 1.0 higher and same") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");

    // The common case: 4 beats is the stricter deadline, so 1.1 drops.
    put_ch10(old_store,kBoth, 1050000, 4, "old-path-B");
    put_ch11(new_store,kBoth, 1000000, 3, "new-path-B");
    // A chart where the rule change made no difference at all.
    put_ch10(old_store,kOldOnly, 777000, 2, "tie-path");
    put_ch11(new_store,kOldOnly, 777000, 2, "tie-path");

    std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 2);

    const FillCompareRow* dropped = find(rows, kBoth);
    REQUIRE(dropped != nullptr);
    CHECK(*dropped->delta == -50000);
    CHECK(dropped->status == "1.0 higher");

    const FillCompareRow* tied = find(rows, kOldOnly);
    REQUIRE(tied != nullptr);
    CHECK(*tied->delta == 0);
    CHECK(tied->status == "same");
}

TEST_CASE("collect_fill_rows: a chart in one database only still gets a row") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");

    put_ch10(old_store,kOldOnly, 900000, 2, "only-old-path");
    put_ch11(new_store,kNewOnly, 800000, 5, "only-new-path");

    std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 2);  // the union of both key sets

    const FillCompareRow* only_old = find(rows, kOldOnly);
    REQUIRE(only_old != nullptr);
    CHECK(only_old->status == "only 1.0");
    CHECK(only_old->old_score == 900000);
    CHECK_FALSE(only_old->new_score.has_value());
    CHECK_FALSE(only_old->delta.has_value());
    CHECK(only_old->new_path.empty());

    const FillCompareRow* only_new = find(rows, kNewOnly);
    REQUIRE(only_new != nullptr);
    CHECK(only_new->status == "only 1.1");
    CHECK(only_new->new_score == 800000);
    CHECK_FALSE(only_new->old_score.has_value());
    CHECK_FALSE(only_new->delta.has_value());
    CHECK(only_new->old_path.empty());
}

TEST_CASE("collect_fill_rows: an old record with no score and no new record") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");

    // The 1.0 database holds a Ready record with no paths, so its summary has
    // no score, and the 1.1 database holds nothing for the chart. The record
    // is filed under the 1.0 key: the 1.0 side never reads a 1.1 result.
    name(old_store, kOldOnly);
    put_no_paths(old_store, true, kOldOnly);

    std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK_FALSE(rows[0].old_score.has_value());
    CHECK_FALSE(rows[0].new_score.has_value());
    // Labelled by the database that holds the record, not by the score.
    CHECK(rows[0].status == "only 1.0");
}

TEST_CASE("collect_fill_rows: each side reads only its own rule, even from one store") {
    // One store holding both rules' results for a chart, as the app's own
    // database does once "1.0 fills" has been used.
    store::RecordStore both(":memory:");
    put_ch10(both, kBoth, 1000000, 3, "old-path-E");
    put_ch11(both, kBoth, 1050000, 4, "new-path-E");

    std::vector<FillCompareRow> rows = compare(both, both);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].old_score == 1000000);
    CHECK(rows[0].new_score == 1050000);
    CHECK(rows[0].old_path == "old-path-E");
    CHECK(rows[0].new_path == "new-path-E");

    // A 1.1 result on the old side is not a 1.0 answer.
    store::RecordStore ch11_only(":memory:");
    put_ch11(ch11_only, kBoth, 1050000, 4, "new-path-E");
    rows = compare(ch11_only, ch11_only);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].status == "only 1.1");
}

TEST_CASE("tally_fill_rows counts every status") {
    std::vector<FillCompareRow> rows(8);
    rows[0].status = "same";
    rows[1].status = "1.0 higher";
    rows[2].status = "1.0 higher";
    rows[3].status = "1.1 higher";
    rows[4].status = "only 1.0";
    rows[5].status = "only 1.1";
    // Every status is counted by its own name: a status the page does not
    // know lands in no bucket, not in "only 1.1".
    rows[6].status = "no such status";
    rows[7].status = "in both";
    // Each row counts its chart's library copies (D79 item 3).
    std::vector<std::pair<std::string, int>> library;
    for (size_t i = 0; i < rows.size(); ++i) {
        rows[i].hyhash = "h" + std::to_string(i);
        library.push_back({rows[i].hyhash, i == 1 ? 3 : (i == 4 ? 2 : 1)});
    }

    app::fill_report::FillCompareStats stats =
        app::fill_report::tally_fill_rows(rows, index_of(library));
    CHECK(stats.total == 11);
    CHECK(stats.same == 1);
    CHECK(stats.ch10_higher == 4);
    CHECK(stats.ch11_higher == 1);
    CHECK(stats.only_old == 2);
    CHECK(stats.only_new == 1);
    CHECK(stats.in_both == 1);
}

TEST_CASE("collect_fill_rows: copies come from the 1.1 library, an unlisted chart counts once") {
    // D79 item 3. The 1.1 database's library lists kBoth in two folders. The
    // 1.0 database's library lists kOldOnly three times, which the page never
    // reads. kOldOnly is not in the 1.1 library, so it counts once
    // (store::RecordStore::copies_of); put() lists kNewOnly there once.
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");

    auto library = [](const std::string& md5, int copies) {
        std::vector<store::ChartLibraryEntry> out;
        for (int i = 0; i < copies; ++i) {
            store::ChartLibraryEntry e;
            e.md5 = md5;
            e.title = "Song";
            e.rootfolder = "C:\\songs" + std::to_string(i);
            e.notespath = e.rootfolder + "\\notes.chart";
            out.push_back(e);
        }
        return out;
    };
    // The libraries first: a scan deletes what its library doesn't list
    // (D87 item 4), and hydra_batch with folder arguments saves the rest
    // without touching the library.
    new_store.rebuild_chart_library(library(kBoth, 2));
    old_store.rebuild_chart_library(library(kOldOnly, 3));
    put_ch10(old_store, kBoth, 1000000, 3, "old-path-K");
    put_ch11(new_store, kBoth, 1050000, 4, "new-path-K");  // 1.1 higher
    put_ch10(old_store, kOldOnly, 900000, 2, "only-old");  // only 1.0
    put_ch11(new_store, kNewOnly, 800000, 5, "only-new");  // only 1.1

    const std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 3);
    CHECK(find(rows, kBoth)->copies == 2);
    CHECK(find(rows, kOldOnly)->copies == 1);
    CHECK(find(rows, kNewOnly)->copies == 1);

    // The tally, the subtitle and the payload all count the copies.
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(old_store, new_store, settings());
    const app::fill_report::FillCompareStats& s = result.stats;
    CHECK(s.total == 4);
    CHECK(s.ch11_higher == 2);
    CHECK(s.only_old == 1);
    CHECK(s.only_new == 1);
    CHECK(s.same + s.ch10_higher + s.ch11_higher + s.only_old + s.only_new + s.in_both ==
          s.total);
    CHECK(result.html.find(
              "4 charts in Expert Pro Drums, 2x Bass: 2 score higher under 1.1, "
              "0 higher under 1.0, 0 unchanged, 2 in one database only, "
              "0 in both, not both scored") != std::string::npos);
    CHECK(result.html.find("{\"k\":2,") != std::string::npos);
}

// D115: every count on the page goes through the 1.1 library's
// ChartIndex::count, so a chart with two library copies counts 2.
TEST_CASE("collect_fill_rows counts each status through ChartIndex::count") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    std::vector<store::ChartLibraryEntry> copies(2);
    for (size_t i = 0; i < copies.size(); ++i) {
        copies[i].md5 = kBoth;
        copies[i].title = "Song";
        copies[i].notespath = "C:\\songs" + std::to_string(i) + "\\notes.chart";
    }
    new_store.rebuild_chart_library(copies);
    REQUIRE(new_store.chart_index().copies(kBoth) == 2);
    put_ch10(old_store, kBoth, 777000, 2, "tie-path");
    put_ch11(new_store, kBoth, 777000, 2, "tie-path");

    const std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].status == "same");
    const app::fill_report::FillCompareStats s =
        app::fill_report::tally_fill_rows(rows, new_store.chart_index());
    CHECK(s.same == 2);
    CHECK(s.total == 2);
    CHECK(app::fill_report::generate_fill_report(old_store, new_store, settings()).stats.same ==
          2);
}

// The page's chart tiles and the count beside its filters add up each row's
// "k" in JavaScript, and tally_fill_rows adds up each row's copies by status
// in C++. The page has to count per filtered view, so it can't print the C++
// totals. This pins that all of them count the same thing: every row carries
// its copies as "k", one page function sums "k", the counter and every chart
// tile read that function, every status the C++ counts has its own count in
// the page's stats(), and the page counts no status the C++ doesn't know.
TEST_CASE("fill page: the tiles add up the copies tally_fill_rows adds up") {
    const std::vector<std::string> statuses = {"same",     "1.0 higher", "1.1 higher",
                                               "only 1.0", "only 1.1",   "in both"};
    std::vector<FillCompareRow> rows;
    std::vector<std::pair<std::string, int>> charts;
    for (size_t i = 0; i < statuses.size(); ++i) {
        FillCompareRow r;
        r.status = statuses[i];
        r.hyhash = "h" + std::to_string(i);
        r.copies = static_cast<int>(i) + 1;
        charts.push_back({r.hyhash, r.copies});
        rows.push_back(r);
    }
    const store::ChartIndex library = index_of(charts);
    const app::fill_report::FillCompareStats st = app::fill_report::tally_fill_rows(rows, library);
    CHECK(st.total == 21);
    CHECK(st.same == 1);
    CHECK(st.ch10_higher == 2);
    CHECK(st.ch11_higher == 3);
    CHECK(st.only_old == 4);
    CHECK(st.only_new == 5);
    CHECK(st.in_both == 6);

    const std::string html = app::fill_report::build_fill_html(rows, library, "sub", "foot");
    // The script's STATUS object carries fill_report.h's six words, and the
    // tiles count by its names.
    CHECK(html.find("const STATUS = {sameScore: \"same\", oldHigher: \"1.0 higher\", "
                    "newHigher: \"1.1 higher\", onlyOld: \"only 1.0\", onlyNew: \"only 1.1\", "
                    "inBoth: \"in both\"};") != std::string::npos);
    for (size_t i = 0; i < statuses.size(); ++i) {
        CAPTURE(statuses[i]);
        CHECK(html.find("{\"k\":" + std::to_string(i + 1) + ",") != std::string::npos);
    }
    for (const char* name : {"sameScore", "oldHigher", "newHigher", "onlyOld", "onlyNew", "inBoth"}) {
        CAPTURE(name);
        CHECK(html.find(std::string("n(STATUS.") + name + ")") != std::string::npos);
    }
    CHECK(html.find("const charts = rs => rs.reduce((a, r) => a + r.k, 0);") !=
          std::string::npos);
    CHECK(html.find("const n = s => charts(rows.filter(r => r.status === s));") !=
          std::string::npos);
    CHECK(html.find("['Charts', fmt(charts(rows))],") != std::string::npos);
    CHECK(html.find("['Charts', fmt(rows.length)]") == std::string::npos);
    // The one summing function, defined once, and the counter beside the
    // filters reads it through the shared script's PAGE.count.
    const size_t defined = html.find("const charts = ");
    REQUIRE(defined != std::string::npos);
    CHECK(html.find("const charts = ", defined + 1) == std::string::npos);
    CHECK(html.find("  count: charts,\n") != std::string::npos);
    CHECK(html.find("const countOf = PAGE.count || (rs => rs.length);") != std::string::npos);
    CHECK(html.find("fmt(countOf(rows)) + ' of ' + PAGE.total;") != std::string::npos);
    CHECK(html.find("  total: \"21 charts\",\n") != std::string::npos);
    // No status counted on the page that the C++ never assigns.
    size_t counted = 0;
    for (const char* call : {"(n(STATUS.", " n(STATUS."})
        for (size_t at = html.find(call); at != std::string::npos; at = html.find(call, at + 1))
            ++counted;
    CHECK(counted == statuses.size());
}

TEST_CASE("build_fill_html substitutes every placeholder") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_ch10(old_store,kBoth, 1000000, 3, "old-path-C");
    put_ch11(new_store,kBoth, 1050000, 4, "new-path-C");

    std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);

    const std::string subtitle = "Subtitle marker 5151";
    const std::string footer = "Footer marker 1515";
    std::string html =
        app::fill_report::build_fill_html(rows, new_store.chart_index(), subtitle, footer);

    CHECK(html.find("__SUBTITLE__") == std::string::npos);
    CHECK(html.find("__FOOTER__") == std::string::npos);
    CHECK(html.find("__DATA__") == std::string::npos);
    CHECK(html.find(subtitle) != std::string::npos);
    CHECK(html.find(footer) != std::string::npos);
    // Both sides' paths reach the page, and the default sort is delta-first.
    CHECK(html.find("old-path-C") != std::string::npos);
    CHECK(html.find("new-path-C") != std::string::npos);
    CHECK(html.find("sortKey: 'delta',") != std::string::npos);

    // The title, heading and score and path columns name each rule by its
    // short name.
    const std::string old_rule = fill_rule_name(FillDeadlineRule::Ch10, FillRuleNameStyle::Short);
    const std::string new_rule = fill_rule_name(FillDeadlineRule::Ch11, FillRuleNameStyle::Short);
    CHECK(html.find("<title>Fill spawn comparison &mdash; " + old_rule + " vs " + new_rule +
                    "</title>") != std::string::npos);
    CHECK(html.find("<span class=\"accent\">" + old_rule + " vs " + new_rule + "</span>") !=
          std::string::npos);
    CHECK(html.find("{k:'s10',     t:'" + old_rule + "',") != std::string::npos);
    CHECK(html.find("{k:'s11',     t:'" + new_rule + "',") != std::string::npos);
    CHECK(html.find("{k:'p10',     t:'" + old_rule + " path',") != std::string::npos);
    CHECK(html.find("{k:'p11',     t:'" + new_rule + " path',") != std::string::npos);
    CHECK(html.find("__OLD_RULE__") == std::string::npos);
    CHECK(html.find("__NEW_RULE__") == std::string::npos);
    // The search box keeps its hint, now written in from html_page.h (M07).
    CHECK(html.find("placeholder=\"Search song, artist, or charter\">") != std::string::npos);
    CHECK(html.find("__SEARCH_HINT__") == std::string::npos);
    // Counts and deltas go through the page's shared fmt.
    CHECK(html.find(".toLocaleString()]") == std::string::npos);
    CHECK(html.find("'+' + r.delta.toLocaleString()") == std::string::npos);
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
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_ch10(old_store,kBoth, 1000000, 3, "old-path-D");
    put_ch11(new_store,kBoth, 1050000, 4, "new-path-D");   // 1.1 higher
    put_ch10(old_store,kOldOnly, 900000, 2, "only-old");   // only 1.0
    put_ch11(new_store,kNewOnly, 800000, 5, "only-new");   // only 1.1

    app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(old_store, new_store, settings());
    CHECK(result.stats.total == 3);
    CHECK(result.stats.ch11_higher == 1);
    CHECK(result.stats.only_old == 1);
    CHECK(result.stats.only_new == 1);
    CHECK(result.stats.same == 0);
    CHECK(result.stats.ch10_higher == 0);
    CHECK_FALSE(result.html.empty());
    CHECK(result.html.find("score higher under 1.1") != std::string::npos);

    // Two empty databases: zero stats, no page.
    store::RecordStore empty_old(":memory:");
    store::RecordStore empty_new(":memory:");
    app::fill_report::GeneratedFillReport none =
        app::fill_report::generate_fill_report(empty_old, empty_new, settings());
    CHECK(none.stats.total == 0);
    CHECK(none.html.empty());
    // The empty page's reason comes from here, so hydra_fillcompare prints
    // what the seam says instead of deciding it again. Two truly empty
    // databases get the settings sentence too (finding 105, D50 item 3).
    CHECK(none.reason ==
          "Nothing is analyzed under these settings (SP cap 4, Expert Pro Drums, 2x Bass) "
          "in either database. Analyze with these settings, or change them.");
    CHECK(result.reason.empty());

    // Records stored at cap 4, asked at cap 8: the sentence names cap 8.
    app::fill_report::GeneratedFillReport off =
        app::fill_report::generate_fill_report(old_store, new_store, settings(8));
    CHECK(off.html.empty());
    CHECK(off.reason ==
          "Nothing is analyzed under these settings (SP cap 8, Expert Pro Drums, 2x Bass) "
          "in either database. Analyze with these settings, or change them.");
}

TEST_CASE("collect_fill_rows: the names go through display_title, display_artist and "
          "display_charter") {
    // test_song_input.cpp pins what those three do. This checks only that the
    // page calls them: one tagged name each, in both databases.
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_ch10(old_store, kBoth, 1000000, 3, "old-path");
    put_ch11(new_store, kBoth, 1000000, 3, "new-path");
    for (store::RecordStore* s : {&old_store, &new_store})
        test::name_chart(*s, kBoth, "<b>Bold Title</b>", test::kTagOnlyTitle, " <b>Bob</b> ");

    const std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].song == "Bold Title");
    CHECK(rows[0].artist == kUnknownTitle);
    CHECK(rows[0].charter == "Bob");
}

TEST_CASE("generate_fill_report: one chart reads \"1 chart\" in the subtitle") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_ch10(old_store, kBoth, 1000000, 3, "old-path-one");
    put_ch11(new_store, kBoth, 1000000, 3, "new-path-one");
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(old_store, new_store, settings());
    CHECK(result.html.find("1 chart in ") != std::string::npos);
    CHECK(result.html.find("1 charts") == std::string::npos);
}

TEST_CASE("collect_fill_rows: a record on both sides with a score on one is in both") {
    // D50 item 2: both databases hold a record for the chart, but the 1.0
    // record is a Ready record with no paths, so it has no score. The row is
    // listed under "in both", not under "only 1.1".
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    name(old_store, kBoth);
    put_no_paths(old_store, true, kBoth);
    put_ch11(new_store, kBoth, 1050000, 4, "new-path-F");

    std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK_FALSE(rows[0].old_score.has_value());
    CHECK(rows[0].new_score == 1050000);
    CHECK_FALSE(rows[0].delta.has_value());
    CHECK(rows[0].status == "in both");

    // The same the other way round: the 1.1 record has no score.
    store::RecordStore old2(":memory:");
    store::RecordStore new2(":memory:");
    put_ch10(old2, kBoth, 1000000, 3, "old-path-F");
    name(new2, kBoth);
    put_no_paths(new2, false, kBoth);
    rows = compare(old2, new2);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].status == "in both");

    // The tally counts it by name, and the page lists it under "In both" and
    // writes "no paths" on the side whose analysis kept none (D51 call 11).
    const store::ChartIndex library = new2.chart_index();
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
    // both, not both scored" (it was D52's "a score on one side only", which
    // a chart with no paths on either side made false).
    constexpr const char* kOneSided = "dd44ee55ff6677889900aa11bb22cc33";
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_ch10(old_store, kBoth, 1000000, 3, "old-path-G");
    put_ch11(new_store, kBoth, 1050000, 4, "new-path-G");   // 1.1 higher
    put_ch10(old_store, kOldOnly, 900000, 2, "only-old");   // only 1.0
    put_ch11(new_store, kNewOnly, 800000, 5, "only-new");   // only 1.1
    // A record on both sides, but the 1.0 one has no paths and so no score.
    name(old_store, kOneSided);
    put_no_paths(old_store, true, kOneSided);
    put_ch11(new_store, kOneSided, 700000, 2, "one-sided");

    app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(old_store, new_store, settings());
    const app::fill_report::FillCompareStats& s = result.stats;
    CHECK(s.total == 4);
    CHECK(s.in_both == 1);
    CHECK(s.same + s.ch10_higher + s.ch11_higher + s.only_old + s.only_new + s.in_both ==
          s.total);

    // The subtitle names the part after "in one database only".
    CHECK(result.html.find(
              "4 charts in Expert Pro Drums, 2x Bass: 1 score higher under 1.1, "
              "0 higher under 1.0, 0 unchanged, 2 in one database only, "
              "1 in both, not both scored") != std::string::npos);
    // The tiles get a matching one after "Only one side", counted from the
    // rows the table shows, like the tiles before it.
    CHECK(result.html.find("['Only one side', fmt(n(STATUS.onlyOld) + n(STATUS.onlyNew))],\n"
                           "      [\"In both, not both scored\", fmt(n(STATUS.inBoth))],") !=
          std::string::npos);
    CHECK(result.html.find("Score on one side only") == std::string::npos);
}

// ---- results the library doesn't list (D92) ---------------------------------

TEST_CASE("generate_fill_report: a database with no chart library stops with the path "
          "report's sentence") {
    // Results on both sides, as hydra_batch with folder arguments leaves
    // them, and a library on one side only: either side without one stops.
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_no_paths(old_store, true, kBoth);
    put_ch11(new_store, kBoth, 1050000, 4, "new-path-L");
    const std::string sentence = app::report::kNoChartLibrary;
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(old_store, new_store, settings());
    CHECK(result.html.empty());
    CHECK(result.reason == sentence);
    // The other way round too.
    const app::fill_report::GeneratedFillReport flipped =
        app::fill_report::generate_fill_report(new_store, old_store, settings());
    CHECK(flipped.html.empty());
    CHECK(flipped.reason == sentence);
}

TEST_CASE("collect_fill_rows: a result whose chart the library doesn't list is left out") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_ch10(old_store, kBoth, 1000000, 3, "old-path-M");
    put_ch11(new_store, kBoth, 1050000, 4, "new-path-M");
    // A result on each side for charts neither library lists.
    put_no_paths(old_store, true, kOldOnly);
    put_no_paths(new_store, false, kNewOnly);

    const std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].hyhash == kBoth);
    CHECK(find(rows, kOldOnly) == nullptr);
    CHECK(find(rows, kNewOnly) == nullptr);
}
