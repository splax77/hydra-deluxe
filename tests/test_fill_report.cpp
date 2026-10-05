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

#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/fill_report.h"
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

store::RecordKey key_for(const std::string& hash, bool legacy_fills) {
    store::Lens lens;
    lens.legacy_fills = legacy_fills ? 1 : 0;
    return store::RecordKey{hash, kMode, store::CapQuery::at(kCloneHeroSpCap), lens};
}

// One analyzed corpus chart, kept alive for the whole file: it supplies a real
// Song (songmeta rows, which list_records joins against) and a real record to
// build PreparedRows from. The scores themselves are overridden per row, so
// which chart it is doesn't matter.
const app::AnalysisResult& sample_chart() {
    static app::AnalysisResult result = [] {
        app::AnalysisSettings settings;
        settings.depth_value = 0;
        for (const std::string& path : corpus::chart_paths()) {
            try {
                app::AnalysisResult r = app::analyze_chart_file(path, settings);
                if (r.song.is_empty() || r.record.paths.empty()) continue;
                return r;
            } catch (const std::exception&) {
                continue;
            }
        }
        // Song has no default constructor, so there is no empty AnalysisResult
        // to fall back on. Throwing keeps the lambda's deduced return type as
        // the one `return r;` above gives it.
        throw std::runtime_error("no corpus chart analyzed");
    }();
    return result;
}

// Registers `hash` in `store` and files a result for it under the given fill
// rule with exactly the summary given. The structure/nodes come from a real
// record so the row is well-formed; only the numbers the report reads are
// overridden.
void put(store::RecordStore& store, bool legacy_fills, const std::string& hash,
         int64_t score, int acts, const std::string& bestpath) {
    const app::AnalysisResult& sample = sample_chart();
    store.add_song(hash, "Song " + hash.substr(0, 4), "Test Artist",
                   "Test Charter", sample.song);

    HydraRecord record = sample.record;
    record.legacy_fills = legacy_fills;
    store::PreparedRow row = store::prepare_row(key_for(hash, legacy_fills), record);
    row.hyhash = hash;
    row.bestpath = bestpath;
    row.summary.score = score;
    row.summary.actcount = acts;
    row.summary.notecount = 1234;
    store.add_row(row);
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

std::vector<FillCompareRow> compare(store::RecordStore& old_store,
                                    store::RecordStore& new_store) {
    return app::fill_report::collect_fill_rows(
        old_store, new_store, kMode, store::CapQuery::at(kCloneHeroSpCap),
        store::Lens{});
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
    old_store.add_song(kOldOnly, "Song bb22", "Test Artist", "Test Charter",
                       sample_chart().song);
    test::store_batch_result(old_store, key_for(kOldOnly, true));

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

    app::fill_report::FillCompareStats stats =
        app::fill_report::tally_fill_rows(rows);
    CHECK(stats.total == 8);
    CHECK(stats.same == 1);
    CHECK(stats.ch10_higher == 2);
    CHECK(stats.ch11_higher == 1);
    CHECK(stats.only_old == 1);
    CHECK(stats.only_new == 1);
    CHECK(stats.in_both == 1);
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
    std::string html = app::fill_report::build_fill_html(rows, subtitle, footer);

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
    // Counts and deltas go through the page's shared fmt.
    CHECK(html.find(".toLocaleString()]") == std::string::npos);
    CHECK(html.find("'+' + r.delta.toLocaleString()") == std::string::npos);
}

TEST_CASE("report payload: the search field is folded and tag-free") {
    FillCompareRow row;
    row.song = "Halo";
    row.artist = "Beyonc\xc3\xa9";  // Beyoncé
    row.charter = "<b>Bob</b>";
    row.status = "only 1.1";
    const std::string html = app::fill_report::build_fill_html({row}, "sub", "foot");
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
        app::fill_report::generate_fill_report(old_store, new_store, kMode,
                                               store::CapQuery::at(kCloneHeroSpCap),
                                               store::Lens{});
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
        app::fill_report::generate_fill_report(empty_old, empty_new, kMode,
                                               store::CapQuery::at(kCloneHeroSpCap),
                                               store::Lens{});
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
        app::fill_report::generate_fill_report(old_store, new_store, kMode,
                                               store::CapQuery::at(8), store::Lens{});
    CHECK(off.html.empty());
    CHECK(off.reason ==
          "Nothing is analyzed under these settings (SP cap 8, Expert Pro Drums, 2x Bass) "
          "in either database. Analyze with these settings, or change them.");
}

TEST_CASE("collect_fill_rows: a blank stored song name reads (unknown)") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");

    // A blank songmeta name from before the fallback. add_song keeps the latest
    // name it sees, so the blank name goes in after put()'s own "Song aa11".
    put_ch10(old_store,kBoth, 1000000, 3, "old-path");
    put_ch11(new_store,kBoth, 1000000, 3, "new-path");
    old_store.add_song(kBoth, "", "Test Artist", "Test Charter", sample_chart().song);
    new_store.add_song(kBoth, "", "Test Artist", "Test Charter", sample_chart().song);

    std::vector<FillCompareRow> rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].song == kUnknownTitle);

    // A title made only of tags reads the same fallback.
    new_store.add_song(kBoth, test::kTagOnlyTitle, "Test Artist", "Test Charter",
                       sample_chart().song);
    rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].song == kUnknownTitle);

    // A bold title, artist and charter read without their tags.
    new_store.add_song(kBoth, "<b>Bold Title</b>", "<i>Tagged Artist</i>",
                       "<color=#FF8000>Tagged Charter</color>", sample_chart().song);
    rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].song == "Bold Title");
    CHECK(rows[0].artist == "Tagged Artist");
    CHECK(rows[0].charter == "Tagged Charter");

    // An artist made only of tags reads "(unknown)" by the title's rule
    // (D50 item 5); a charter made only of tags keeps today's blank.
    new_store.add_song(kBoth, "Song", test::kTagOnlyTitle, test::kTagOnlyTitle,
                       sample_chart().song);
    rows = compare(old_store, new_store);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].artist == kUnknownTitle);
    CHECK(rows[0].charter == "");

    // An empty artist and the scan's placeholder read "(unknown)" too (D56
    // item 2); a charter loses the spaces at its ends (display_charter).
    for (const char* artist : {"", kUnknownArtist}) {
        new_store.add_song(kBoth, "Song", artist, " <b>Bob</b> ", sample_chart().song);
        rows = compare(old_store, new_store);
        REQUIRE(rows.size() == 1);
        CHECK(rows[0].artist == kUnknownTitle);
        CHECK(rows[0].charter == "Bob");
    }
}

TEST_CASE("generate_fill_report: one chart reads \"1 chart\" in the subtitle") {
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_ch10(old_store, kBoth, 1000000, 3, "old-path-one");
    put_ch11(new_store, kBoth, 1000000, 3, "new-path-one");
    const app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(old_store, new_store, kMode,
                                               store::CapQuery::at(kCloneHeroSpCap),
                                               store::Lens{});
    CHECK(result.html.find("1 chart in ") != std::string::npos);
    CHECK(result.html.find("1 charts") == std::string::npos);
}

TEST_CASE("collect_fill_rows: a record on both sides with a score on one is in both") {
    // D50 item 2: both databases hold a record for the chart, but the 1.0
    // record is a Ready record with no paths, so it has no score. The row is
    // listed under "in both", not under "only 1.1".
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    old_store.add_song(kBoth, "Song aa11", "Test Artist", "Test Charter",
                       sample_chart().song);
    test::store_batch_result(old_store, key_for(kBoth, true));
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
    new2.add_song(kBoth, "Song aa11", "Test Artist", "Test Charter", sample_chart().song);
    test::store_batch_result(new2, key_for(kBoth, false));
    rows = compare(old2, new2);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].status == "in both");

    // The tally counts it by name, and the page lists it under "In both" and
    // writes "no score" on the side that has none.
    CHECK(app::fill_report::tally_fill_rows(rows).in_both == 1);
    const std::string html = app::fill_report::build_fill_html(rows, "sub", "foot");
    CHECK(html.find("<option value=\"in both\">In both</option>") != std::string::npos);
    CHECK(html.find("'no score'") != std::string::npos);
}

TEST_CASE("generate_fill_report: a score on one side only is counted, and the parts add up") {
    // D52: the subtitle and the tiles count the "in both" rows as charts with
    // a score on one side only, so every chart lands in exactly one part.
    constexpr const char* kOneSided = "dd44ee55ff6677889900aa11bb22cc33";
    store::RecordStore old_store(":memory:");
    store::RecordStore new_store(":memory:");
    put_ch10(old_store, kBoth, 1000000, 3, "old-path-G");
    put_ch11(new_store, kBoth, 1050000, 4, "new-path-G");   // 1.1 higher
    put_ch10(old_store, kOldOnly, 900000, 2, "only-old");   // only 1.0
    put_ch11(new_store, kNewOnly, 800000, 5, "only-new");   // only 1.1
    // A record on both sides, but the 1.0 one has no paths and so no score.
    old_store.add_song(kOneSided, "Song dd44", "Test Artist", "Test Charter",
                       sample_chart().song);
    test::store_batch_result(old_store, key_for(kOneSided, true));
    put_ch11(new_store, kOneSided, 700000, 2, "one-sided");

    app::fill_report::GeneratedFillReport result =
        app::fill_report::generate_fill_report(old_store, new_store, kMode,
                                               store::CapQuery::at(kCloneHeroSpCap),
                                               store::Lens{});
    const app::fill_report::FillCompareStats& s = result.stats;
    CHECK(s.total == 4);
    CHECK(s.in_both == 1);
    CHECK(s.same + s.ch10_higher + s.ch11_higher + s.only_old + s.only_new + s.in_both ==
          s.total);

    // The subtitle names the part after "in one database only".
    CHECK(result.html.find(
              "4 charts in Expert Pro Drums, 2x Bass: 1 score higher under 1.1, "
              "0 higher under 1.0, 0 unchanged, 2 in one database only, "
              "1 with a score on one side only") != std::string::npos);
    // The tiles get a matching one after "Only one side", counted from the
    // rows the table shows, like the tiles before it.
    CHECK(result.html.find("['Only one side', fmt(n('only 1.0') + n('only 1.1'))],\n"
                           "      ['Score on one side only', fmt(n('in both'))],") !=
          std::string::npos);
}
