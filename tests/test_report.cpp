// Structural tests for app/report.{h,cpp}: collect_rows must surface one row
// per stored record, and build_html must substitute every template
// placeholder and embed the rows and the subtitle/footer strings.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <atomic>
#include <algorithm>
#include <cmath>
#include <map>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/display_format.h"
#include "app/dm_report.h"
#include "env_util.h"
#include "app/fill_report.h"
#include "app/html_page.h"
#include "app/library_query.h"
#include "app/report.h"
#include "app/report_files.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/squeeze_rating.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "record_fixtures.h"
#include "search/graph.h"
#include "store/record_store.h"
#include "wcag_util.h"

using namespace hydra;
using namespace hydra::app;

namespace {

size_t occurrences(const std::string& text, const std::string& what) {
    size_t n = 0;
    for (size_t at = text.find(what); at != std::string::npos; at = text.find(what, at + 1)) ++n;
    return n;
}

// Analyze the first `want` non-empty corpus charts into a fresh in-memory
// store and return how many records landed.
int fill_store(store::RecordStore& store, int cap, int want) {
    AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.sp_cap = cap;

    int added = 0;
    for (const AnalysisResult& result :
         corpus::analyzed_with_paths(settings, static_cast<size_t>(want))) {
        const std::string hyhash = "h" + std::to_string(added);
        store.add_song(hyhash, "Title " + std::to_string(added), "Artist", "Charter",
                       result.song);
        store.add_record(store::RecordKey{hyhash, "mode", store::CapQuery::at(cap)},
                         result.record);
        ++added;
    }
    return added;
}

void check_cap(int cap) {
    store::RecordStore store(":memory:");
    const int added = fill_store(store, cap, 5);
    REQUIRE(added > 0);

    // One row per shown path, so a record surfaces exactly one rank-1 row.
    const store::CapQuery query = store::CapQuery::at(cap);
    std::vector<report::ReportRow> rows =
        report::collect_rows(store, /*max_paths=*/100, query, store::Lens{});
    int rank1 = 0;
    for (const report::ReportRow& row : rows)
        if (row.rank == 1) ++rank1;
    CHECK(rank1 == added);
    CHECK(static_cast<int>(rows.size()) >= added);

    const std::string subtitle = "Subtitle marker 4242";
    const std::string footer = "Footer marker 2424";
    std::string html = report::build_html(rows, subtitle, footer);

    // Every placeholder is substituted, and the substituted content is there.
    CHECK(html.find("__SUBTITLE__") == std::string::npos);
    CHECK(html.find("__FOOTER__") == std::string::npos);
    CHECK(html.find("__DATA__") == std::string::npos);
    CHECK(html.find(subtitle) != std::string::npos);
    CHECK(html.find(footer) != std::string::npos);
    for (int i = 0; i < added; ++i)
        CHECK(html.find("Title " + std::to_string(i)) != std::string::npos);

    MESSAGE(std::to_string(cap) << " bars: " << rows.size()
            << " rows, " << html.size() << " bytes");
}

// Stores H1's tied-variant record for chart `hyhash` at SP cap `cap` under
// `lens`, with its song, so the report has three paths to list: two tied at
// the top score and a lower one.
void store_tied(store::RecordStore& store, const std::string& hyhash, int cap,
                store::Lens lens = {}) {
    HydraRecord record = test::tied_variant_record();
    record.sp_cap = cap;
    record.legacy_fills = lens.legacy_fills;
    store.add_song(hyhash, "Tied " + hyhash, "Artist", "Charter", test::beat_song({}, {}, 13440));
    store.add_record(store::RecordKey{hyhash, "mode", store::CapQuery::at(cap), lens}, record);
}

}  // namespace

TEST_CASE("report rows: a tied top-score variant is optimal too") {
    store::RecordStore store(":memory:");
    store_tied(store, "tied", 4);

    const std::vector<report::ReportRow> rows =
        report::collect_rows(store, /*max_paths=*/100, store::CapQuery::at(4), store::Lens{});
    REQUIRE(rows.size() == 3);
    // Best score first: the root and its tied variant, then the lower root.
    CHECK(rows[0].score == rows[1].score);
    CHECK(rows[2].score < rows[1].score);
    CHECK(rows[0].optimal);
    CHECK(rows[1].optimal);
    CHECK_FALSE(rows[2].optimal);

    report::ReportOptions options;
    options.cap = store::CapQuery::at(4);
    const report::GeneratedReport page = report::generate_report(store, options);
    // The subtitle still counts one record, however many paths tie.
    CHECK(page.records == 1);
    CHECK(occurrences(page.html, "\"opt\":true") == 2);
    CHECK(occurrences(page.html, "\"opt\":false") == 1);
    // "Best path only" and the bold row read the flag, not the rank.
    CHECK(page.html.find("if (bestOnly && !r.opt) return false;") != std::string::npos);
    CHECK(page.html.find("rowClass: r => r.opt ? 'best' : '',") != std::string::npos);
    CHECK(page.html.find("r.rank !== 1") == std::string::npos);
    CHECK(page.html.find("r.rank === 1") == std::string::npos);
}

TEST_CASE("report page embeds every stored record (4 bars)") { check_cap(4); }

// collect_rows lists a record's paths in all_paths() order and numbers them
// from 1, so that order has to be best first already. This guards it at the
// report's own settings (fill_store's), where variants appear.
TEST_CASE("all_paths lists paths best first on every corpus chart") {
    AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.sp_cap = 4;
    int charts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, settings.prodrums, settings.bass2x,
                                        settings.difficulty, settings.rules);
        if (song.is_empty()) continue;
        ++charts;
        const std::vector<const Path*> all = corpus::analyzed(chart, settings).all_paths();
        for (size_t i = 1; i < all.size(); ++i) {
            INFO(chart << " path " << i);
            CHECK(all[i]->totalscore() <= all[i - 1]->totalscore());
        }
    }
    CHECK(charts > 0);
}

TEST_CASE("report lists only the wanted cap and names it") {
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store, 4, 1) == 1);
    // The same chart again at 8 bars, under the same key.
    AnalysisSettings settings;
    settings.depth_value = 10;
    settings.sp_cap = 8;
    store.add_record(store::RecordKey{"h0", "mode", store::CapQuery::at(8)},
                     corpus::first_analyzed_with_paths(settings).record);
    REQUIRE(store.counts().second == 2);

    report::ReportOptions options;
    options.cap = store::CapQuery::at(4);
    report::GeneratedReport four = report::generate_report(store, options);
    CHECK(four.html.find("SP cap 4 bars") != std::string::npos);
    // The subtitle counts what the page lists: the one record at 4 bars, not
    // the 8-bar record the database also holds for the same chart.
    CHECK(four.records == 1);
    CHECK(four.songs == 1);
    CHECK(four.html.find("1 record across 1 chart") != std::string::npos);
    int rank1 = 0;
    for (const report::ReportRow& row : report::collect_rows(store, 100, options.cap, options.lens))
        if (row.rank == 1) ++rank1;
    CHECK(rank1 == 1);

    // The cap reads through the house count rule: one bar, and commas from
    // 1,000 (D48 Q12).
    store_tied(store, "one", 1);
    store_tied(store, "thousand", 1000);
    options.cap = store::CapQuery::at(1);
    CHECK(report::generate_report(store, options).html.find("SP cap 1 bar<") !=
          std::string::npos);
    options.cap = store::CapQuery::at(1000);
    CHECK(report::generate_report(store, options).html.find("SP cap 1,000 bars") !=
          std::string::npos);

    // A 1.0 page names its rule by the fill rule's one long name.
    store::Lens legacy;
    legacy.legacy_fills = true;
    store_tied(store, "legacy", 4, legacy);
    options.cap = store::CapQuery::at(4);
    options.lens = legacy;
    CHECK(report::generate_report(store, options)
              .html.find(std::string("SP cap 4 bars — ") +
                         fill_rule_name(FillDeadlineRule::Ch10, FillRuleNameStyle::Long) +
                         " fills") != std::string::npos);
}

TEST_CASE("collect_rows: a blank or old-placeholder song name reads (unknown)") {
    store::RecordStore store(":memory:");
    AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.sp_cap = 4;

    // songmeta names written before the fallback existed, a title with a
    // bold tag, and H1's title made only of tags. Each pairs with the name
    // the report shows.
    const std::vector<std::pair<std::string, std::string>> names = {
        {"", kUnknownTitle},
        {"<unknown title>", kUnknownTitle},
        {"<b>Bold</b> Song", "Bold Song"},
        {test::kTagOnlyTitle, kUnknownTitle},
    };
    size_t added = 0;
    for (const AnalysisResult& result : corpus::analyzed_with_paths(settings, names.size())) {
        const std::string hyhash = "u" + std::to_string(added);
        store.add_song(hyhash, names[added].first, "<i>Artist</i>", "<b>Charter</b>",
                       result.song);
        store.add_record(store::RecordKey{hyhash, "mode", store::CapQuery::at(settings.sp_cap)},
                         result.record);
        ++added;
    }
    REQUIRE(added == names.size());

    std::vector<report::ReportRow> rows =
        report::collect_rows(store, /*max_paths=*/100, store::CapQuery::at(4), store::Lens{});
    REQUIRE(!rows.empty());
    for (const report::ReportRow& row : rows) {
        INFO(row.hyhash);
        const size_t i = static_cast<size_t>(std::stoi(row.hyhash.substr(1)));
        CHECK(row.song == names[i].second);
        // Artist and charter lose their tags too.
        CHECK(row.artist == "Artist");
        CHECK(row.charter == "Charter");
    }

    // An artist made only of tags reads "(unknown)" by the title's rule
    // (D50 item 5); a charter made only of tags keeps today's blank.
    // add_song keeps the latest names it is given.
    store.add_song("u0", "Song", test::kTagOnlyTitle, test::kTagOnlyTitle,
                   test::beat_song({}, {}, 13440));
    rows = report::collect_rows(store, /*max_paths=*/100, store::CapQuery::at(4), store::Lens{});
    bool saw_u0 = false;
    for (const report::ReportRow& row : rows) {
        if (row.hyhash != "u0") continue;
        saw_u0 = true;
        CHECK(row.artist == kUnknownTitle);
        CHECK(row.charter == "");
    }
    CHECK(saw_u0);

    // The scan's artist placeholder reads "(unknown)" too (D56 item 2), and a
    // charter loses its tags and the spaces at its ends (display_charter).
    store.add_song("u0", "Song", kUnknownArtist, " <b>Bob</b> ", test::beat_song({}, {}, 13440));
    rows = report::collect_rows(store, /*max_paths=*/100, store::CapQuery::at(4), store::Lens{});
    saw_u0 = false;
    for (const report::ReportRow& row : rows) {
        if (row.hyhash != "u0") continue;
        saw_u0 = true;
        CHECK(row.artist == kUnknownTitle);
        CHECK(row.charter == "Bob");
    }
    CHECK(saw_u0);
}

TEST_CASE("fill_rule_for: the legacy_fills flag and the file stamp each name one fill rule") {
    // legacy_fills on is the Clone Hero 1.0 rule; off is the normal 1.1 rule.
    CHECK(fill_rule_for(true) == FillDeadlineRule::Ch10);
    CHECK(fill_rule_for(false) == FillDeadlineRule::Ch11);
    // A stamp reads back as the rule that wrote it; any other text is no rule.
    CHECK(fill_rule_from_stamp("ch10") == FillDeadlineRule::Ch10);
    CHECK(fill_rule_from_stamp("ch11") == FillDeadlineRule::Ch11);
    CHECK_FALSE(fill_rule_from_stamp("ch12").has_value());
    CHECK_FALSE(fill_rule_from_stamp("").has_value());
}

TEST_CASE("tier_for: raw-ms bands derived from the two-hit budget") {
    using report::tier_for;

    // Default window 85 -> budget 170: bands 2 / 42.5 / 85 / 127.5 / 170.
    // Each edge belongs to the band below it (D48 Q3): a timing on the
    // difficult floor is not past it, so 2.0 ms reads Normal, and 170.0 ms is
    // still inside the two-hit budget, so it reads Insane+.
    CHECK(tier_for(std::nullopt).first == "None");
    CHECK(tier_for(1.9).first == "Normal");
    CHECK_FALSE(past_difficult_floor(2.0));
    CHECK(tier_for(2.0).first == "Normal");
    CHECK(past_difficult_floor(2.1));
    CHECK(tier_for(2.1).first == "Hard");
    CHECK(tier_for(42.5).first == "Hard");
    CHECK(tier_for(42.6).first == "Extreme");
    CHECK(tier_for(85.0).first == "Extreme");
    CHECK(tier_for(85.1).first == "Insane");
    CHECK(tier_for(127.5).first == "Insane");
    CHECK(tier_for(127.6).first == "Insane+");
    CHECK(tier_for(170.0).first == "Insane+");
    CHECK(tier_for(170.0).second == "t4");
    CHECK(tier_for(170.1).first == "Beyond");
    CHECK(tier_for(170.1).second == "t5");

    // At the historical 70 ms window the original 2/35/70/105/140 ladder
    // reproduces, with the same edge rule.
    CHECK(tier_for(2.0, 70.0).first == "Normal");
    CHECK(tier_for(35.0, 70.0).first == "Hard");
    CHECK(tier_for(35.1, 70.0).first == "Extreme");
    CHECK(tier_for(70.1, 70.0).first == "Insane");
    CHECK(tier_for(105.1, 70.0).first == "Insane+");
    CHECK(tier_for(140.0, 70.0).first == "Insane+");
    CHECK(tier_for(140.1, 70.0).first == "Beyond");
}

TEST_CASE("tier_for walks the timing_tiers table edge by edge") {
    // tier_for and the page's embedded tier table read the one ladder in
    // core/squeeze_rating.h, so every banded entry's cutoff is exactly where
    // the label flips to the next entry's: the cutoff itself still reads the
    // entry's own name (D48 Q3), and just past it reads the next one.
    std::vector<TimingTier> tiers = timing_tiers(85.0);
    REQUIRE(tiers.size() >= 2);
    for (size_t i = 0; i + 1 < tiers.size(); ++i) {
        if (!tiers[i].cutoff) continue;
        const double cutoff = *tiers[i].cutoff;
        CHECK(report::tier_for(cutoff, 85.0).first == tiers[i].name);
        CHECK(report::tier_for(cutoff + 0.01, 85.0).first == tiers[i + 1].name);
    }
}

TEST_CASE("report payload carries the hit window and the tier table") {
    // No store needed: an empty row list still embeds the metadata.
    std::string html =
        report::build_html({}, "sub", "foot", /*hit_window_ms=*/85.0);
    CHECK(html.find("\"hit_window\":85.0") != std::string::npos);
    CHECK(html.find("\"tiers\":[") != std::string::npos);
    CHECK(html.find("{\"name\":\"Normal\",\"tok\":\"t0\",\"cutoff\":2.0}") !=
          std::string::npos);
    CHECK(html.find("{\"name\":\"Insane+\",\"tok\":\"t4\",\"cutoff\":170.0}") !=
          std::string::npos);
    CHECK(html.find("{\"name\":\"Beyond\",\"tok\":\"t5\",\"cutoff\":null}") !=
          std::string::npos);
    CHECK(html.find("{\"name\":\"None\",\"tok\":\"tn\",\"cutoff\":null}") !=
          std::string::npos);
    CHECK(html.find("\"rows\":[]") != std::string::npos);

    // The dropdown is payload-built; no hardcoded band strings remain.
    CHECK(html.find("Beyond 140ms") == std::string::npos);

    // The Hardest ms and Early fill cells print the app's own one-decimal
    // text, so an exact half rounds the way the app rounds it (D48 Q2, Q5).
    // The numbers stay beside the text for sorting and the tiles.
    report::ReportRow timed;
    timed.song = "Through The Fire";
    timed.artist = "DragonForce";
    timed.charter = "Some Charter";
    timed.path = "1";
    timed.tier = "Hard";
    timed.tok = "t1";
    timed.ms = 12.25;
    timed.efill = -3.25;
    report::ReportRow untimed = timed;
    untimed.ms.reset();
    untimed.efill.reset();
    const std::string rows = report::build_html({timed, untimed}, "sub", "foot", 85.0);
    CHECK(rows.find("\"ms\":12.25,\"ms_text\":\"" + format_ms(12.25) + "\"") !=
          std::string::npos);
    CHECK(rows.find("\"efill\":-3.25,\"efill_text\":\"" + format_ms(-3.25) + "\"") !=
          std::string::npos);
    CHECK(rows.find("\"ms\":null,\"ms_text\":null") != std::string::npos);
    CHECK(rows.find("\"efill\":null,\"efill_text\":null") != std::string::npos);
    CHECK(rows.find("['num', r.ms_text === null ? DASH : r.ms_text],") != std::string::npos);
    CHECK(rows.find("['num', r.efill_text === null ? DASH : r.efill_text],") !=
          std::string::npos);
    CHECK(rows.find("fmtMs") == std::string::npos);
    CHECK(rows.find("toFixed(1)") == std::string::npos);

    // Each row carries its search text: the library's fold of the shown song,
    // artist, charter and path (D48 Q31).
    const std::string search = fold_for_search(timed.song + " " + timed.artist + " " +
                                               timed.charter + " " + timed.path);
    CHECK(search == "through the fire dragonforce some charter 1");
    CHECK(occurrences(rows, "\"search\":\"" + search + "\"") == 2);
}

TEST_CASE("report payload: the average multiplier is C++ text from format_avg_mult") {
    // The cell prints the payload's mult_text; the number beside it stays
    // for sorting. The page never formats the multiplier itself.
    report::ReportRow row;
    row.song = "Song";
    row.path = "1";
    row.tier = "None";
    row.tok = "tn";
    row.mult = 2.5;
    const std::string html = report::build_html({row}, "sub", "foot", 85.0);
    CHECK(html.find("\"mult\":2.5,\"mult_text\":\"2.500\"") != std::string::npos);
    CHECK(html.find("['num', r.mult_text],") != std::string::npos);
    CHECK(html.find("r.mult.toFixed(") == std::string::npos);
}

TEST_CASE("report payload: the search field is folded and tag-free") {
    report::ReportRow row;
    row.song = "Halo";
    row.artist = "Beyonc\xc3\xa9";  // Beyoncé
    row.charter = "<b>Bob</b>";
    row.path = "1";
    row.tier = "None";
    row.tok = "tn";
    const std::string html = report::build_html({row}, "sub", "foot", 85.0);
    CHECK(html.find("\"search\":\"halo beyonce bob 1\"") != std::string::npos);
}

TEST_CASE("report page reads the Beyond edge from the payload") {
    // The edge travels in the page's data as the whole number the footer
    // prints, and the "Past N ms" tile counts the rows tier_for already put
    // in Beyond, so the page never works out either fact again.
    std::string html = report::build_html({}, "sub", "foot", /*hit_window_ms=*/85.0);
    CHECK(html.find("\"beyond_edge_ms\":170") != std::string::npos);
    CHECK(html.find("DATA.beyond_edge_ms") != std::string::npos);
    CHECK(html.find("Math.max(...DATA.tiers") == std::string::npos);
    CHECK(html.find("HIT_WINDOW * 2") == std::string::npos);
    CHECK(html.find("const beyond = rows.filter(r => r.tier === 'Beyond').length;") !=
          std::string::npos);
    CHECK(html.find("r.ms > BEYOND") == std::string::npos);
}


TEST_CASE("generate_report: one seam frames the page for every entry point") {
    store::RecordStore store(":memory:");
    const int added = fill_store(store, 4, 2);
    REQUIRE(added > 0);

    report::ReportOptions options;
    options.max_paths = report::kDefaultReportPaths;
    options.db_path = "C:/somewhere/hydra.db";
    report::GeneratedReport result = report::generate_report(store, options);
    CHECK(result.songs == added);
    CHECK(result.records == added);
    CHECK(result.rows >= added);

    // The framing strings are part of the interface: the CLI and the GUI's
    // ReportJob both ship exactly this subtitle and footer. The cut is per
    // chart and mode, and the page lists every mode at the current cap.
    CHECK(result.empty_reason == report::EmptyReason::None);
    std::string subtitle = counted(result.records, "record", "records") + " across " +
                           counted(result.songs, "chart", "charts") +
                           " — every mode at the current cap, top 5 paths per chart and mode";
    CHECK(result.html.find(subtitle) != std::string::npos);
    // D50 item 1. The default 85 ms hit window puts Beyond past 170 ms, the
    // same number the Beyond chip and the "Past 170 ms" tile print. The page
    // escapes the apostrophes.
    CHECK(result.html.find(
              "<p>Generated from hydra.db. Timing tiers measure how big each "
              "squeeze is, in steps of your hit window. The Paths tab&#x27;s row "
              "labels measure how far a hit lands from the Star Power end, so the "
              "two can differ. &#x27;Beyond&#x27; means past the 170 ms window.</p>") !=
          std::string::npos);
    CHECK(result.html.find("Timing tiers match") == std::string::npos);

    // --all-paths wording.
    options.max_paths = report::kEveryPathSentinel;
    CHECK(report::generate_report(store, options)
              .html.find(counted(result.songs, "chart", "charts") + " — every path") !=
          std::string::npos);

    // An empty store yields no page, and says nothing is stored.
    store::RecordStore empty(":memory:");
    report::GeneratedReport none = report::generate_report(empty, options);
    CHECK(none.rows == 0);
    CHECK(none.html.empty());
    CHECK(none.empty_reason == report::EmptyReason::NothingStored);

    // Records stored at cap 4, asked at cap 8: the page is empty because of
    // the settings, and the reason names them (finding 105, D48 Q28).
    options.cap = store::CapQuery::at(8);
    report::GeneratedReport off = report::generate_report(store, options);
    CHECK(off.rows == 0);
    CHECK(off.html.empty());
    CHECK(off.empty_reason == report::EmptyReason::NothingUnderSettings);
    const std::string sentence =
        "Nothing is analyzed under these settings (SP cap 8, Clone Hero 1.1 fills). "
        "Analyze with these settings, or change them.";
    CHECK(off.why_empty == sentence);
    // The app's ReportJob throws that sentence, and the strip shows it as it is.
    CHECK(plain_error(std::runtime_error(off.why_empty)) == sentence);
}

TEST_CASE("nothing_under_settings frames the cap, the middle words and the ending") {
    // One frame for every empty page (finding 105, D50 item 3): the path
    // report passes its fill rule, the fill comparison its chart mode and
    // " in either database".
    CHECK(report::nothing_under_settings(8, "Clone Hero 1.1 fills") ==
          "Nothing is analyzed under these settings (SP cap 8, Clone Hero 1.1 fills). "
          "Analyze with these settings, or change them.");
    CHECK(report::nothing_under_settings(4, "Expert Pro Drums, 2x Bass",
                                         " in either database") ==
          "Nothing is analyzed under these settings (SP cap 4, Expert Pro Drums, 2x Bass) "
          "in either database. Analyze with these settings, or change them.");
    CHECK(report::nothing_under_settings(1200, "Clone Hero 1.0 fills") ==
          "Nothing is analyzed under these settings (SP cap 1,200, Clone Hero 1.0 fills). "
          "Analyze with these settings, or change them.");
}

TEST_CASE("generate_report hands back nothing when its cancel flag is set") {
    // Closing Hydra while the report builds. The walk stops between records,
    // and no page is framed from the part of the library it managed to read.
    store::RecordStore store(":memory:");
    REQUIRE(fill_store(store, 4, 2) > 0);

    std::atomic<bool> cancel{true};
    report::ReportOptions options;
    options.max_paths = 5;
    options.db_path = "C:/somewhere/hydra.db";
    options.cancel = &cancel;

    report::GeneratedReport result = report::generate_report(store, options);
    CHECK(result.rows == 0);
    CHECK(result.html.empty());
    CHECK(result.empty_reason == report::EmptyReason::Cancelled);
}

TEST_CASE("write_report_file swaps the page in and leaves no .tmp behind") {
    // Same temp-path recipe as tests/test_store.cpp's temp_db: the Windows
    // temp directory, tagged and pid-suffixed so parallel test runs don't
    // collide.
    wchar_t tmp_dir[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp_dir);
    const std::string path = hydra::wide_to_utf8(tmp_dir) + "hydra_test_report_file_" +
                              std::to_string(GetCurrentProcessId()) + ".html";

    hydra::app::write_report_file(path, "<html>old</html>");
    hydra::app::write_report_file(path, "<html>new</html>");

    std::ifstream in(path, std::ios::binary);
    std::ostringstream contents;
    contents << in.rdbuf();
    CHECK(contents.str() == "<html>new</html>");

    CHECK_FALSE(std::filesystem::exists(path + ".tmp"));

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST_CASE("path report shows each row's chart mode in its own column") {
    report::ReportRow a;
    a.song = "Song A";
    a.mode = "Expert Pro Drums, 2x Bass";
    a.path = "1";
    a.tier = "None";
    a.tok = "tn";
    report::ReportRow b = a;
    b.mode = "Hard Drums, 1x Bass";
    const std::string html = report::build_html({a, b}, "sub", "foot", 85.0);

    CHECK(html.find("{k:'mode',") != std::string::npos);
    CHECK(html.find("t:'Mode'") != std::string::npos);
    CHECK(html.find("['dim trunc mode', r.mode]") != std::string::npos);
    CHECK(html.find("\"mode\":\"Expert Pro Drums, 2x Bass\"") != std::string::npos);
    CHECK(html.find("\"mode\":\"Hard Drums, 1x Bass\"") != std::string::npos);
    // The page never read each row's gap to the best path, so the payload no
    // longer carries it.
    CHECK(html.find("\"delta\":") == std::string::npos);
}

TEST_CASE("the three report pages share one stylesheet and one script") {
    const std::string paths = report::build_html({}, "sub", "foot", 85.0);
    const std::string dm = dm_report::build_dm_html({}, "sub", "foot");
    const std::string fill = fill_report::build_fill_html({}, "sub", "foot");
    for (const std::string* page : {&paths, &dm, &fill}) {
        CHECK(page->find(html::kReportCss) != std::string::npos);
        CHECK(page->find(html::kReportJs) != std::string::npos);
        // Rules no page used, and the theme switch nothing ever sets, are gone.
        CHECK(page->find(".delta {") == std::string::npos);
        CHECK(page->find(".rank {") == std::string::npos);
        CHECK(page->find("data-theme") == std::string::npos);
        // The pages search words only (D56 item 1, D57 item 3): accents fold,
        // and every typed word must appear in the text the page shows; quotes
        // and field prefixes are ordinary words. No page joins its fields and
        // looks for the query as one lowercased run, and every page carries
        // the fold table Hydra built from the library's fold.
        CHECK(page->find("toLowerCase().includes(") == std::string::npos);
        CHECK(page->find("const FOLD = {\"A\":\"a\",") != std::string::npos);
    }
    // The shared script folds the query through that table, splits it into
    // words and keeps a row when every word is in its search text. It holds no
    // fold of its own: no lowercasing and no accented letter, raw or escaped.
    const std::string script = html::kReportJs;
    CHECK(script.find("FOLD[ch]") != std::string::npos);
    CHECK(script.find(".split(/\\s+/)") != std::string::npos);
    CHECK(script.find("words.every(w => r.search.includes(w))") != std::string::npos);
    CHECK(script.find("toLowerCase") == std::string::npos);
    CHECK(std::none_of(script.begin(), script.end(),
                       [](char c) { return static_cast<unsigned char>(c) >= 0x80; }));
    for (const char* range : {"\\u00", "\\u01", "\\uff", "\\uFF"})
        CHECK(script.find(range) == std::string::npos);
    CHECK(paths.find("<div class=\"wrap\">") != std::string::npos);
    CHECK(dm.find("<div class=\"wrap dm\">") != std::string::npos);
    CHECK(fill.find("<div class=\"wrap fill\">") != std::string::npos);

    // Every number on the path report groups through the shared fmt, which
    // uses one fixed rule (1,234), whatever language the browser is set to
    // (D48 Q12). The single-owner scan keeps it that way.
    CHECK(std::string(html::kReportJsHead).find("n.toLocaleString('en-US')") !=
          std::string::npos);
    CHECK(paths.find(".toLocaleString()") == std::string::npos);
}

TEST_CASE("tier_for over a built table matches the window form") {
    const std::vector<TimingTier> tiers = timing_tiers(85.0);
    const std::vector<std::optional<double>> samples = {
        std::nullopt, 0.0, 1.9, 2.0, 42.5, 127.4, 169.9, 170.0, 500.0};
    for (const std::optional<double>& ms : samples)
        CHECK(report::tier_for(ms, tiers) == report::tier_for(ms, 85.0));
}

TEST_CASE("records_by_hash keys every listed record by its lower-case hash") {
    store::RecordStore store(":memory:");
    AnalysisSettings settings;
    settings.depth_value = 0;
    const AnalysisResult result = corpus::first_analyzed_with_paths(settings);
    store.add_song("ABCDEF0123", "Title", "Artist", "Charter", result.song);
    store.add_record(store::RecordKey{"ABCDEF0123", "mode", store::CapQuery::at(4)},
                     result.record);

    const std::unordered_map<std::string, store::RecordListing> by_hash =
        report::records_by_hash(store, "mode", store::CapQuery::at(4), store::Lens{});
    REQUIRE(by_hash.size() == 1);
    REQUIRE(by_hash.count("abcdef0123") == 1);
    CHECK(by_hash.at("abcdef0123").hyhash == "ABCDEF0123");
    CHECK(report::records_by_hash(store, "other mode", store::CapQuery::at(4),
                                  store::Lens{})
              .empty());
}

// Not an invariant: writes one small page of each kind into the folder named
// by HYDRA_PAGE_SAMPLES, built from fixed rows, so a page change can be
// checked in a real browser before and after (docs/adr/0016). Run it with
//   hydra_tests.exe --no-skip -tc="report pages: write samples*"
TEST_CASE("report pages: write samples for the browser check" * doctest::skip()) {
    const std::optional<std::string> dir = read_env("HYDRA_PAGE_SAMPLES");
    REQUIRE(dir.has_value());
    const std::filesystem::path out = std::filesystem::u8path(*dir);
    std::filesystem::create_directories(out);

    std::vector<report::ReportRow> paths;
    auto add_path = [&](const std::string& song, const char* mode, int rank,
                        const std::string& path, int64_t score, std::optional<double> ms,
                        std::optional<double> efill) {
        report::ReportRow r;
        r.song = song;
        r.artist = "Artist of " + song;
        r.charter = "Charter & Co";
        r.mode = mode;
        r.rank = rank;
        r.path = path;
        r.score = score;
        r.acts = 3 + rank;
        r.skip = rank - 1;
        r.ms = ms;
        auto [tier, tok] = report::tier_for(ms, 85.0);
        r.tier = tier;
        r.tok = tok;
        r.efill = efill;
        r.mult = 2.345 + rank;
        r.sqin = rank;
        r.sqout = 2 - rank % 2;
        r.notes = 1200 + rank;
        paths.push_back(r);
    };
    std::string long_path;
    for (int i = 0; i < 80; ++i) long_path += "1-E2+ ";
    add_path("Song A", "Expert Pro Drums, 2x Bass", 1, "1-E2+ 0-E1", 123456, 12.5, -3.25);
    add_path("Song A", "Expert Pro Drums, 2x Bass", 2, "1-E2 0-E1-", 123000, 48.0, std::nullopt);
    add_path("Song A", "Hard Drums, 1x Bass", 1, "0 0 1", 98000, std::nullopt, std::nullopt);
    add_path("Song B", "Expert Pro Drums, 2x Bass", 1, long_path, 250000, 171.0, 4.5);
    add_path("Song B", "Expert Pro Drums, 2x Bass", 2, "2 1-E3", 249500, 1.5, 0.0);
    add_path("Song C", "Expert Drums, 1x Bass", 1, "1 1 1", 77000, 90.0, std::nullopt);

    std::vector<dm_report::DmReportRow> dm;
    // The delta, the percent (in hundredths, as the payload carries it) and
    // whether the score is above optimal are typed, not worked out again from
    // the two scores.
    auto add_dm = [&](const char* song, int64_t actual, std::optional<int64_t> optimal,
                      std::optional<int64_t> delta, std::optional<int64_t> pct_h,
                      const char* status, bool above, bool fc, std::optional<int> rank) {
        dm_report::DmReportRow r;
        r.song = song;
        r.artist = "Artist";
        r.charter = "Charter";
        r.identifier = "hash";
        r.actual = actual;
        r.optimal = optimal;
        r.delta = delta;
        r.pct_h = pct_h;
        r.is_fc = fc;
        r.percent = fc ? 100 : 97;
        r.speed = 100;
        r.rank = rank;
        r.posted = "2026-09-20T12:34:56Z";
        r.status = status;
        r.above_optimal = above;
        dm.push_back(r);
    };
    add_dm("Song A", 120000, 123456, 3456, 9720, "under optimal", false, true, 3);
    add_dm("Song B", 251000, 250000, -1000, 10040, "above optimal", true, false, 1);
    add_dm("Song C", 90000, std::nullopt, std::nullopt, std::nullopt, "not in library", false,
           false, std::nullopt);
    add_dm("Song D", 80000, std::nullopt, std::nullopt, std::nullopt, "not analyzed", false,
           false, std::nullopt);

    std::vector<fill_report::FillCompareRow> fill;
    auto add_fill = [&](const char* song, std::optional<int64_t> old_score,
                        std::optional<int64_t> new_score, std::optional<int64_t> delta,
                        const char* status) {
        fill_report::FillCompareRow r;
        r.song = song;
        r.artist = "Artist";
        r.charter = "Charter";
        r.hyhash = song;
        r.old_score = old_score;
        r.new_score = new_score;
        r.delta = delta;
        if (old_score) { r.old_path = "1-E2 0"; r.old_acts = 2; }
        if (new_score) { r.new_path = "1-E2 0-E1"; r.new_acts = 3; }
        r.notes = 900;
        r.status = status;
        fill.push_back(r);
    };
    add_fill("Song A", 100000, 100500, 500, "1.1 higher");
    add_fill("Song B", 100000, 99000, -1000, "1.0 higher");
    add_fill("Song C", 100000, 100000, 0, "same");
    add_fill("Song D", 100000, std::nullopt, std::nullopt, "only 1.0");
    add_fill("Song E", std::nullopt, 100000, std::nullopt, "only 1.1");

    write_report_file(out / "paths.html",
                      report::build_html(paths, "Sample subtitle", "Sample footer", 85.0));
    write_report_file(out / "dm.html",
                      dm_report::build_dm_html(dm, "Sample subtitle", "Sample footer"));
    write_report_file(out / "fill.html",
                      fill_report::build_fill_html(fill, "Sample subtitle", "Sample footer"));
}

// ---- where reports are saved ------------------------------------------------

namespace {

namespace fs = std::filesystem;

// One test's view of the Documents folder. It clears the database override
// (a harness override keeps reports next to its database, which would hide
// the Documents rule), and afterwards puts the overrides and the lookup back
// and deletes its scratch folder.
struct DocumentsSandbox {
    PathOverrides previous = path_overrides();
    fs::path root;

    explicit DocumentsSandbox(const char* tag) {
        PathOverrides cleared = previous;
        cleared.db_path.clear();
        set_path_overrides(cleared);
        wchar_t tmp[MAX_PATH];
        GetTempPathW(MAX_PATH, tmp);
        root = fs::path(tmp) / ("hydra_test_docs_" + std::to_string(GetCurrentProcessId())) /
               fs::u8path(tag);
        std::error_code ec;
        fs::remove_all(root, ec);
        fs::create_directories(root);
    }

    ~DocumentsSandbox() {
        set_documents_dir_lookup({});
        set_path_overrides(previous);
        std::error_code ec;
        fs::remove_all(root, ec);
    }
};

}  // namespace

TEST_CASE("reports_dir is Documents\\Hydra, made on first use") {
    DocumentsSandbox box("made");
    set_documents_dir_lookup([&box] { return std::optional<fs::path>(box.root); });

    const fs::path dir = reports_dir();
    CHECK(dir == box.root / "Hydra");
    CHECK(fs::is_directory(dir));
    // Every report path helper lives in it.
    CHECK(fs::path(report_html_path()) == dir / "hydra_paths.html");
    CHECK(fs::path(dm_report_html_path()) == dir / "hydra_dmcompare.html");
}

TEST_CASE("reports_dir falls back to the database folder without Documents") {
    DocumentsSandbox box("fallback");
    const fs::path db_folder = fs::u8path(db_path()).parent_path();

    // No Documents folder at all.
    set_documents_dir_lookup([] { return std::optional<fs::path>(); });
    CHECK(reports_dir() == db_folder);

    // A Documents folder where "Hydra" can't be made: a plain file is in the way.
    { std::ofstream(box.root / "Hydra") << "not a folder"; }
    set_documents_dir_lookup([&box] { return std::optional<fs::path>(box.root); });
    CHECK(reports_dir() == db_folder);
}

TEST_CASE("reports_dir keeps a harness's reports next to its database") {
    DocumentsSandbox box("override");
    set_documents_dir_lookup([&box] { return std::optional<fs::path>(box.root); });
    PathOverrides scratch = path_overrides();
    scratch.db_path = (box.root / "scratch" / "hydra.db").u8string();
    set_path_overrides(scratch);

    CHECK(reports_dir() == box.root / "scratch");
    CHECK_FALSE(fs::exists(box.root / "Hydra"));  // Documents was never touched
}

// ---- the page shell and the path report -------------------------------------

namespace {

// The one COLS line of a page's script that declares column `key`.
std::string col_line(const std::string& page, const std::string& key) {
    const size_t at = page.find("{k:'" + key + "',");
    if (at == std::string::npos) return {};
    return page.substr(at, page.find('\n', at) - at);
}

// WCAG 2.2 relative luminance of "#rrggbb" (testwcag owns the formula).
double luminance(const std::string& hex) {
    auto channel = [&hex](size_t at) {
        return std::stoi(hex.substr(at, 2), nullptr, 16) / 255.0;
    };
    return testwcag::relative_luminance(channel(1), channel(3), channel(5));
}

double contrast(const std::string& a, const std::string& b) {
    return testwcag::contrast_ratio(luminance(a), luminance(b));
}

// The "--name: #rrggbb" tokens of the first ":root {" block at or after `from`.
std::map<std::string, std::string> css_tokens(const std::string& css, size_t from) {
    std::map<std::string, std::string> out;
    const size_t open = css.find(":root {", from);
    REQUIRE(open != std::string::npos);
    const std::string block = css.substr(open, css.find('}', open) - open);
    size_t at = 0;
    while ((at = block.find("--", at)) != std::string::npos) {
        const size_t colon = block.find(':', at);
        const size_t semi = block.find(';', colon);
        const size_t hash = block.find('#', colon);
        if (hash != std::string::npos && hash < semi && semi - hash == 7)
            out[block.substr(at + 2, colon - at - 2)] = block.substr(hash, 7);
        if (semi == std::string::npos) break;
        at = semi;
    }
    return out;
}

}  // namespace

TEST_CASE("report pages are standards-mode documents") {
    const std::string paths = report::build_html({}, "sub", "foot", 85.0);
    const std::string dm = dm_report::build_dm_html({}, "sub", "foot");
    const std::string fill = fill_report::build_fill_html({}, "sub", "foot");
    for (const std::string* page : {&paths, &dm, &fill}) {
        CHECK(page->rfind("<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n", 0) == 0);
        CHECK(page->find("</style>\n</head>\n<body>\n") != std::string::npos);
        REQUIRE(page->size() > 16);
        CHECK(page->substr(page->size() - 16) == "</body>\n</html>\n");
        CHECK(occurrences(*page, "<title>") == 1);
    }
}

TEST_CASE("report colours meet WCAG contrast in both themes") {
    const std::string css = html::kReportCss;
    const std::map<std::string, std::string> light = css_tokens(css, 0);
    const std::map<std::string, std::string> dark =
        css_tokens(css, css.find("prefers-color-scheme: dark"));
    for (const auto* theme : {&light, &dark}) {
        // Body text, dim text, header text and every chip colour, on the page,
        // on a cell, and on a hovered row or the header band.
        for (const char* text : {"ink", "muted", "t0", "t1", "t2", "t3", "t4", "t5", "tn"}) {
            for (const char* ground : {"paper", "surface", "raised"}) {
                INFO(text << " on " << ground);
                CHECK(contrast(theme->at(text), theme->at(ground)) >= 4.5);
            }
        }
        // The accent is the large bold heading word and the focus ring: 3:1.
        CHECK(contrast(theme->at("sp"), theme->at("surface")) >= 3.0);
        CHECK(contrast(theme->at("sp"), theme->at("raised")) >= 3.0);
    }
}

TEST_CASE("report table header sticks and the page prints") {
    const std::string css = html::kReportCss;
    const size_t at = css.find(".tablewrap {");
    REQUIRE(at != std::string::npos);
    const std::string rule = css.substr(at, css.find('}', at) - at);
    // Scrolling both ways makes the wrapper the header's sticky container.
    CHECK(rule.find("overflow: auto;") != std::string::npos);
    CHECK(rule.find("max-height:") != std::string::npos);
    CHECK(rule.find("overflow-x") == std::string::npos);

    CHECK(css.find("@media print {") != std::string::npos);
    CHECK(css.find("thead { display: table-header-group; }") != std::string::npos);
    CHECK(css.find(".controls { display: none; }") != std::string::npos);
}

TEST_CASE("report pages number their rows in a # column") {
    // The shared script adds the column, so all three pages get it.
    const std::string js = html::kReportJs;
    CHECK(js.find("th.textContent = '#';") != std::string::npos);
    CHECK(js.find("idx.textContent = fmt(++n);") != std::string::npos);
    // The sort control and the arrows walk the real columns, not the # one.
    CHECK(js.find("document.querySelectorAll('#head th.sortable')") != std::string::npos);
    CHECK(std::string(html::kReportCss).find("th.idx, td.idx {") != std::string::npos);
}

TEST_CASE("path report explains and renames its columns") {
    const std::string html = report::build_html({}, "sub", "foot", 85.0);
    CHECK(html.find("t:'Early fill (ms)'") != std::string::npos);
    CHECK(html.find("t:'Avg multiplier'") != std::string::npos);
    CHECK(html.find("t:'Early fill',") == std::string::npos);
    CHECK(html.find("t:'Avg mult',") == std::string::npos);
    for (const char* key : {"mode", "path", "score", "acts", "skip", "ms", "tier", "efill",
                            "mult", "sqin", "sqout", "notes"}) {
        INFO(key);
        CHECK(col_line(html, key).find("d:'") != std::string::npos);
    }
    // Hover text and the footer legend both read the definitions.
    CHECK(html.find("<dl class=\"legend\" id=\"legend\"></dl>") != std::string::npos);
    CHECK(html.find("const legend = document.getElementById('legend');") != std::string::npos);
    // One name per tier for the dropdown and the chip ("No squeezes" in both).
    CHECK(html.find("function tierLabel(name)") != std::string::npos);
    CHECK(html.find("o.textContent = tierLabel(t.name);") != std::string::npos);
    CHECK(html.find("['chip ' + r.tok, tierLabel(r.tier), 'chip']") != std::string::npos);
    // The search box and the tier dropdown have names a screen reader reads.
    CHECK(html.find("id=\"q\" aria-label=\"Search paths\"") != std::string::npos);
    CHECK(html.find("id=\"tier\" aria-label=\"Timing tier\"") != std::string::npos);
    // The tile over the table carries the column's name, since it can show an
    // early fill as well as a squeeze (D48 Q7), and the text of the row with
    // the largest Hardest ms.
    CHECK(html.find("['Hardest ms', hardest === null ? DASH : hardest.ms_text],") !=
          std::string::npos);
    CHECK(html.find("Tightest squeeze") == std::string::npos);
    // The "Past N ms" tile counts the rows tier_for put in Beyond: past the
    // edge, not on it.
    CHECK(html.find("const beyond = rows.filter(r => r.tier === 'Beyond').length;") !=
          std::string::npos);
    CHECK(col_line(html, "tier").find("Beyond means more than twice the hit window.") !=
          std::string::npos);
    // The average multiplier's definition opens with its own name.
    CHECK(col_line(html, "mult").find(
              "d:'Average multiplier: the score without solo bonuses divided by the base score "
              "(every note at 1x).'") != std::string::npos);
    CHECK(html.find("Points per note on average") == std::string::npos);
}

TEST_CASE("path report counts charts by hash in the tile and the subtitle") {
    report::ReportRow a;
    a.song = "Same";
    a.artist = "Name";
    a.path = "1";
    a.tier = "None";
    a.tok = "tn";
    a.hyhash = "h1";
    report::ReportRow a2 = a;
    a2.rank = 2;
    report::ReportRow b = a;  // another chart with the same title and artist
    b.hyhash = "h2";
    const std::string html = report::build_html({a, a2, b}, "sub", "foot", 85.0);

    // Each chart gets a small id in order of first appearance.
    CHECK(occurrences(html, "\"c\":0") == 2);
    CHECK(occurrences(html, "\"c\":1") == 1);
    CHECK(html.find("['Charts', fmt(new Set(rows.map(r => r.c)).size)]") != std::string::npos);
    CHECK(html.find("r.song + r.artist))") == std::string::npos);
}

TEST_CASE("comparison page explains its columns and splits the missing scores") {
    const std::string html = dm_report::build_dm_html({}, "sub", "foot");
    for (const char* key : {"actual", "optimal", "delta", "pct_h", "fc", "speed", "rank",
                            "posted", "status"}) {
        INFO(key);
        CHECK(col_line(html, key).find("d:'") != std::string::npos);
    }
    CHECK(html.find("<option value=\"not analyzed\">") != std::string::npos);
    CHECK(html.find("<option value=\"not in library\">") != std::string::npos);
    CHECK(html.find("value=\"unmatched\"") == std::string::npos);
    CHECK(html.find("'not analyzed':'s-notanalyzed'") != std::string::npos);
    // A score with a result reads under, at or above optimal; none is "matched".
    CHECK(html.find("<option value=\"under optimal\">Under optimal</option>") !=
          std::string::npos);
    CHECK(html.find("<option value=\"at optimal\">At optimal</option>") != std::string::npos);
    CHECK(html.find("<option value=\"above optimal\">Above optimal</option>") !=
          std::string::npos);
    for (const char* status : {"under optimal", "at optimal", "above optimal"}) {
        INFO(status);
        CHECK(html.find(std::string("'") + status + "':'s-") != std::string::npos);
    }
    CHECK(html.find("value=\"matched\"") == std::string::npos);
    CHECK(html.find("'matched'") == std::string::npos);
    CHECK(html.find("Matched") == std::string::npos);
    CHECK(col_line(html, "status").find(
              "d:'Under optimal, At optimal or Above optimal when Hydra has a result.") !=
          std::string::npos);
    CHECK(html.find("id=\"q\" aria-label=\"Search scores\"") != std::string::npos);
    CHECK(html.find("id=\"status\" aria-label=\"Status\"") != std::string::npos);
    CHECK(html.find("<dl class=\"legend\" id=\"legend\"></dl>") != std::string::npos);
}
