// Unit tests for ui/library_model: the in-memory library behind the main
// window's table. It filters with app/library_query, counts the status chips,
// sorts, and turns each stored summary into the Best path cell's text.

#include "doctest.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

#include "app/user_messages.h"  // stale_text, what a Stale row's tooltip shows
#include "core/rules.h"
#include "display_fixtures.h"  // kTagOnlyTitle
#include "store/record_store.h"
#include "ui/library_model.h"

using hydra::store::CapQuery;
using hydra::store::ChartLibraryEntry;
using hydra::store::PreparedRow;
using hydra::store::RecordKey;
using hydra::store::RecordStatus;
using hydra::store::SummaryLookup;
using hydra::ui::LibraryModel;
using hydra::ui::LibrarySort;
using hydra::ui::StatusChip;

namespace {

ChartLibraryEntry chart(const char* md5, const char* title, const char* artist,
                        const char* charter, const char* folder) {
    ChartLibraryEntry e;
    e.md5 = md5;
    e.title = title;
    e.artist = artist;
    e.charter = charter;
    e.rootfolder = folder;
    e.notespath = std::string("C:\\songs\\") + folder + "\\" + title + "\\notes.mid";
    return e;
}

SummaryLookup ready(int64_t score, const char* bestpath, int stars, double hardest_ms) {
    SummaryLookup s;
    s.status = RecordStatus::Ready;
    s.bestpath = bestpath;
    s.summary.score = score;
    s.summary.stars = stars;
    s.summary.hardest_ms = hardest_ms;
    return s;
}

SummaryLookup stale() {
    SummaryLookup s;
    s.status = RecordStatus::Stale;
    return s;
}

// Six charts from the scratch library's shapes: Burnout analyzed, Chair
// stale, the rest not analyzed. Row order here is scan order, not title order.
LibraryModel sample() {
    LibraryModel m;
    m.set_charts({
        chart("burnout", "Burnout", "Green Day", "Hoph2o", "common\\Summer Blast _25 Setlist\\Tier 4"),
        chart("yyz", "YYZ", "Rush", "Harmonix, Onyxite", "common\\Summer Blast _25 Setlist\\Tier 4"),
        chart("chair", "Chair", "Sufferer", "Satan", "common\\Summer Blast _25 Setlist\\Tier 4"),
        chart("acid", "Acid Romance", "Some Band", "<color=#e02222>Blood</color>line", "common\\Other"),
        chart("halo", "Halo", "Beyoncé", "Someone", "common\\Other"),
        chart("other", "Other", "Thrice", "Someone", "IB24\\T4"),
    });
    SummaryLookup none;
    m.set_summaries({ready(378315, "3- 1 2", 7, 163.0), none, stale(), none, none, none});
    return m;
}

std::vector<std::string> titles(const LibraryModel& m) {
    std::vector<std::string> out;
    for (size_t i : m.order()) out.push_back(m.rows()[i].title);
    return out;
}

// A library of three charts whose stored results are Stale, one per cause,
// read through the store the way the app reads them (get_summaries). Chart
// "build" came from another Hydra version, "rules" from other rules in
// hydra_rules.ini, "both" from both. Returns the model's rows, in that order.
std::vector<hydra::ui::LibraryRow> stale_rows() {
    hydra::core::Rules other = hydra::core::default_rules();
    other.max_tied_paths = 2;
    hydra::HydraRecord here;
    here.sp_cap = 8;
    hydra::HydraRecord foreign = here;
    foreign.rules_fingerprint = other.fingerprint();
    auto key = [](const char* md5) { return RecordKey{md5, "mode", CapQuery::at(8)}; };

    hydra::store::RecordStore store(":memory:");
    PreparedRow build = hydra::store::prepare_row(key("build"), here);
    build.hyversion = "0.0.0";
    store.add_row(build);
    store.add_row(hydra::store::prepare_row(key("rules"), foreign));
    PreparedRow both = hydra::store::prepare_row(key("both"), foreign);
    both.hyversion = "0.0.0";
    store.add_row(both);

    LibraryModel m;
    m.set_charts({chart("build", "Build", "A", "C", "common"),
                  chart("rules", "Rules", "A", "C", "common"),
                  chart("both", "Both", "A", "C", "common")});
    m.set_summaries(
        store.get_summaries(m.hashes(), "mode", CapQuery::at(8), hydra::store::Lens{}));
    return m.rows();
}

// The sentence a Stale row's tooltip shows (library_table.cpp asks
// stale_text with the row's two flags).
std::string row_tooltip(const hydra::ui::LibraryRow& row) {
    return hydra::app::stale_text(row.stale_build, row.stale_rules);
}

}  // namespace

TEST_CASE("library model: a row Stale from another Hydra version says so in its tooltip") {
    const std::vector<hydra::ui::LibraryRow> rows = stale_rows();
    REQUIRE(rows.size() == 3);
    const hydra::ui::LibraryRow& row = rows[0];
    REQUIRE(row.status == RecordStatus::Stale);
    CHECK(row.stale_build);
    CHECK_FALSE(row.stale_rules);
    CHECK(row_tooltip(row) ==
          "Out of date: this result came from another Hydra version. Re-analyze to refresh it.");
}

TEST_CASE("library model: a row Stale from other rules names hydra_rules.ini in its tooltip") {
    const std::vector<hydra::ui::LibraryRow> rows = stale_rows();
    REQUIRE(rows.size() == 3);
    const hydra::ui::LibraryRow& row = rows[1];
    REQUIRE(row.status == RecordStatus::Stale);
    CHECK_FALSE(row.stale_build);
    CHECK(row.stale_rules);
    CHECK(row_tooltip(row) ==
          "Out of date: this result came from different rules in hydra_rules.ini. Re-analyze "
          "to refresh it.");
}

TEST_CASE("library model: a row Stale from both causes names both in its tooltip") {
    const std::vector<hydra::ui::LibraryRow> rows = stale_rows();
    REQUIRE(rows.size() == 3);
    const hydra::ui::LibraryRow& row = rows[2];
    REQUIRE(row.status == RecordStatus::Stale);
    CHECK(row.stale_build);
    CHECK(row.stale_rules);
    CHECK(row_tooltip(row) ==
          "Out of date: this result came from another Hydra version or from different rules "
          "in hydra_rules.ini. Re-analyze to refresh it.");
}

TEST_CASE("library model: a Stale row whose cause changes takes the new cause") {
    LibraryModel m;
    m.set_charts({chart("h", "Song", "A", "C", "common")});
    SummaryLookup by_build = stale();
    by_build.stale_build = true;
    REQUIRE(m.set_summaries({by_build}) == 1);
    SummaryLookup by_rules = stale();
    by_rules.stale_rules = true;
    CHECK(m.set_summaries({by_rules}) == 1);
    CHECK_FALSE(m.rows()[0].stale_build);
    CHECK(m.rows()[0].stale_rules);
}

TEST_CASE("library model: every chart shows, sorted by title, with its Best path text") {
    const LibraryModel m = sample();
    CHECK(titles(m) == std::vector<std::string>{"Acid Romance", "Burnout", "Chair", "Halo",
                                                "Other", "YYZ"});
    CHECK(m.rows()[0].best_label == "378,315  3- 1 2");
    CHECK(m.rows()[2].best_label == "Stale");
    CHECK(m.rows()[1].best_label == "Not analyzed");
    // Colour tags never reach the screen.
    CHECK(m.rows()[3].charter == "Bloodline");
    CHECK(m.counts().all == 6);
    CHECK(m.counts().not_analyzed == 4);
    CHECK(m.counts().stale == 1);
    CHECK(m.counts().analyzed == 1);
}

TEST_CASE("library model: a title made only of tags reads (unknown)") {
    LibraryModel m;
    m.set_charts({chart("tags", hydra::test::kTagOnlyTitle, "Artist", "Charter", "common")});
    REQUIRE(m.rows().size() == 1);
    CHECK(m.rows()[0].title == "(unknown)");
}

TEST_CASE("library model: an artist made only of tags reads (unknown), and search keeps the stored text") {
    // D50 item 5: the artist column follows the title's rule. Search and sort
    // still read the stored text, so "(unknown)" is not a search hit.
    LibraryModel m;
    m.set_charts({chart("tags", "Song", hydra::test::kTagOnlyTitle, "Charter", "common")});
    REQUIRE(m.rows().size() == 1);
    CHECK(m.rows()[0].artist == "(unknown)");
    CHECK(m.rows()[0].searchable.artist.find("unknown") == std::string::npos);
    m.set_query("unknown");
    CHECK(m.order().empty());
}

TEST_CASE("library model: status_label is the one source of the three status words") {
    using hydra::ui::status_label;
    CHECK(std::string(status_label(RecordStatus::Ready)) == "Analyzed");
    CHECK(std::string(status_label(RecordStatus::Stale)) == "Stale");
    CHECK(std::string(status_label(RecordStatus::NotAnalyzed)) == "Not analyzed");
    // The Best path cell of a row with no current result shows the same word.
    const hydra::store::PathSummary none;
    CHECK(hydra::ui::best_path_label(RecordStatus::Stale, "", none) == "Stale");
    CHECK(hydra::ui::best_path_label(RecordStatus::NotAnalyzed, "", none) == "Not analyzed");
}

TEST_CASE("library model: status_of names the one status each chip groups") {
    using hydra::ui::StatusChip;
    using hydra::ui::status_of;
    CHECK(status_of(StatusChip::Analyzed) == RecordStatus::Ready);
    CHECK(status_of(StatusChip::Stale) == RecordStatus::Stale);
    CHECK(status_of(StatusChip::NotAnalyzed) == RecordStatus::NotAnalyzed);
    CHECK_FALSE(status_of(StatusChip::All).has_value());
}

TEST_CASE("library model: the search narrows the rows and the chip counts follow it") {
    LibraryModel m = sample();
    m.set_query("\"tier 4\"");
    CHECK(titles(m) == std::vector<std::string>{"Burnout", "Chair", "YYZ"});
    CHECK(m.counts().all == 3);
    CHECK(m.counts().not_analyzed == 1);
    CHECK(m.counts().stale == 1);
    CHECK(m.counts().analyzed == 1);

    m.set_query("green burnout");
    CHECK(titles(m) == std::vector<std::string>{"Burnout"});
    m.set_query("bloodline");
    CHECK(titles(m) == std::vector<std::string>{"Acid Romance"});
    m.set_query("beyonce");
    CHECK(titles(m) == std::vector<std::string>{"Halo"});
    m.set_query("stars:7");
    CHECK(titles(m) == std::vector<std::string>{"Burnout"});
    m.set_query("squeeze<=200");
    CHECK(titles(m) == std::vector<std::string>{"Burnout"});
    m.set_query("squeeze<=20");
    CHECK(titles(m).empty());
    CHECK(m.counts().all == 0);
    m.set_query("stars:9");
    CHECK_FALSE(m.query().errors.empty());
    m.set_query("");
    CHECK(m.order().size() == 6);
}

TEST_CASE("library model: a status chip narrows the rows, and an emptied chip falls back to All") {
    LibraryModel m = sample();
    m.set_chip(StatusChip::NotAnalyzed);
    CHECK(titles(m) == std::vector<std::string>{"Acid Romance", "Halo", "Other", "YYZ"});
    CHECK(m.counts().all == 6);  // counts ignore the chip
    m.set_chip(StatusChip::Stale);
    CHECK(titles(m) == std::vector<std::string>{"Chair"});
    // "What would Analyze search analyze": the query's matches, whatever the chip.
    CHECK(m.matches().size() == 6);

    // Chair is re-analyzed: the Stale group is empty, so the table goes back
    // to All instead of sitting empty.
    CHECK(m.set_summary_for("chair", ready(300000, "1 1", 6, 40.0)) == 1);
    CHECK(m.chip() == StatusChip::All);
    CHECK(m.order().size() == 6);
    CHECK(m.counts().analyzed == 2);
}

TEST_CASE("library model: Best path sorts by score, with unscored rows last both ways") {
    LibraryModel m = sample();
    m.set_summary_for("yyz", ready(500000, "2 2", 7, 12.0));
    m.set_sort(LibrarySort::BestPath, false);
    CHECK(titles(m) == std::vector<std::string>{"YYZ", "Burnout", "Chair", "Acid Romance",
                                                "Halo", "Other"});
    m.set_sort(LibrarySort::BestPath, true);
    CHECK(titles(m) == std::vector<std::string>{"Burnout", "YYZ", "Chair", "Acid Romance",
                                                "Halo", "Other"});
    m.set_sort(LibrarySort::Title, false);
    CHECK(titles(m).front() == "YYZ");
    m.set_sort(LibrarySort::Artist, true);
    CHECK(titles(m).front() == "Halo");  // "beyonce" folds first
}

TEST_CASE("library model: one chart in two folders gets both rows updated") {
    LibraryModel m;
    m.set_charts({chart("same", "Song", "A", "C", "Pack 1"), chart("same", "Song", "A", "C", "Pack 2")});
    CHECK(m.counts().not_analyzed == 2);
    CHECK(m.set_summary_for("same", ready(1000, "1", 3, 0.0)) == 2);
    CHECK(m.counts().analyzed == 2);
    // Asking again with the same answer changes nothing.
    CHECK(m.set_summary_for("same", ready(1000, "1", 3, 0.0)) == 0);
}

TEST_CASE("library model: filtering 20,000 charts takes under 20 ms") {
    // query_matches runs for every row on every applied keystroke, so the
    // whole pass has to fit well inside a frame.
    std::vector<ChartLibraryEntry> charts;
    std::vector<SummaryLookup> summaries;
    charts.reserve(20000);
    for (int i = 0; i < 20000; ++i) {
        char md5[16], title[32], artist[32], charter[64], folder[48];
        std::snprintf(md5, sizeof(md5), "h%05d", i);
        std::snprintf(title, sizeof(title), "Song %05d", i);
        std::snprintf(artist, sizeof(artist), "Artist %02d", i % 50);
        std::snprintf(charter, sizeof(charter), "<color=#e02222>Char</color>ter %d", i % 30);
        std::snprintf(folder, sizeof(folder), "Pack %03d\\Tier %d", i % 200, i % 7);
        charts.push_back(chart(md5, title, artist, charter, folder));
        summaries.push_back(i % 3 == 0 ? ready(300000 + i, "1 2 3", 5 + i % 3, i % 90)
                                       : SummaryLookup{});
    }
    LibraryModel m;
    auto t0 = std::chrono::steady_clock::now();
    m.set_charts(std::move(charts));
    m.set_summaries(summaries);
    MESSAGE("load + sort 20,000: "
            << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count()
            << " ms");

    for (const char* q : {"s", "song 1", "\"tier 4\"", "artist 07 song", "charter", "stars:7",
                          "squeeze<=20 pack", "zzqx", ""}) {
        // Best of five: on a busy machine one run can lose its time slice
        // and read 50 ms for 2 ms of work. The fastest run is the filter's
        // own cost. set_query skips a repeat of the same text, so a
        // different query goes in (untimed) before each timed one.
        double ms = 1e9;
        for (int rep = 0; rep < 5; ++rep) {
            m.set_query("~reset~");
            t0 = std::chrono::steady_clock::now();
            m.set_query(q);
            ms = std::min(ms, std::chrono::duration<double, std::milli>(
                                  std::chrono::steady_clock::now() - t0).count());
        }
        MESSAGE("query \"" << std::string(q) << "\": " << ms << " ms, " << m.order().size() << " rows");
        CHECK(ms < 20.0);
    }
    t0 = std::chrono::steady_clock::now();
    m.set_sort(LibrarySort::BestPath, false);
    MESSAGE("sort by Best path: "
            << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count()
            << " ms");
}
