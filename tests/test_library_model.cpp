// Unit tests for ui/library_model: the in-memory library behind the main
// window's table. It filters with app/library_query, counts the status chips,
// sorts, and turns each stored summary into the Best path cell's text.

#include "doctest.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include "app/user_messages.h"  // stale_text, stored_failure_text: the rows' tooltips
#include "core/error_kind.h"
#include "core/rules.h"
#include "display_fixtures.h"  // kTagOnlyTitle, add_stale_records, empty_analysis_under
#include "parse/song.h"        // NoNotesError
#include "store/record_store.h"
#include "ui/library_model.h"

using hydra::store::CapQuery;
using hydra::store::ChartLibraryEntry;
using hydra::store::PathRow;
using hydra::store::Profile;
using hydra::store::RecordLookup;
using hydra::store::RecordStatus;
using hydra::ui::LibraryModel;
using hydra::ui::LibrarySort;
using hydra::ui::StatusChip;

namespace {

ChartLibraryEntry chart(const char* md5, const char* title, const char* artist,
                        const char* charter, const char* folder, const char* album = "") {
    ChartLibraryEntry e;
    e.md5 = md5;
    e.title = title;
    e.artist = artist;
    e.album = album;
    e.charter = charter;
    e.rootfolder = folder;
    e.notespath = std::string("C:\\songs\\") + folder + "\\" + title + "\\notes.mid";
    return e;
}

// A Ready lookup whose record keeps one path: the best path's text, score and
// hardest timing, and the record's stars.
RecordLookup ready(int64_t score, const char* bestpath, int stars, double hardest_ms) {
    RecordLookup s;
    s.status = RecordStatus::Ready;
    PathRow best;
    best.path = bestpath;
    best.score = score;
    best.hardest_ms = hardest_ms;
    s.record.paths.push_back(best);
    s.record.stars = stars;
    return s;
}

RecordLookup stale() {
    RecordLookup s;
    s.status = RecordStatus::Stale;
    return s;
}

// Six charts from the scratch library's shapes: Burnout analyzed, Chair
// stale, the rest not analyzed. Row order here is scan order, not title order.
// Halo and YYZ have no album.
LibraryModel sample() {
    LibraryModel m;
    m.set_charts({
        chart("burnout", "Burnout", "Green Day", "Hoph2o", "common\\Summer Blast _25 Setlist\\Tier 4",
              "Dookie"),
        chart("yyz", "YYZ", "Rush", "Harmonix, Onyxite", "common\\Summer Blast _25 Setlist\\Tier 4"),
        chart("chair", "Chair", "Sufferer", "Satan", "common\\Summer Blast _25 Setlist\\Tier 4",
              "Sufferer"),
        chart("acid", "Acid Romance", "Some Band", "<color=#e02222>Blood</color>line", "common\\Other",
              "A Quiet Place To Die"),
        chart("halo", "Halo", "Beyoncé", "Someone", "common\\Other"),
        chart("other", "Other", "Thrice", "Someone", "IB24\\T4", "The Illusion of Safety"),
    });
    RecordLookup none;
    m.set_summaries({ready(378315, "3- 1 2", 7, 163.0), none, stale(), none, none, none});
    return m;
}

std::vector<std::string> titles(const LibraryModel& m) {
    std::vector<std::string> out;
    for (size_t i : m.order()) out.push_back(m.rows()[i].title);
    return out;
}

// A library of three charts whose stored results are Stale, one per cause,
// read through the store the way the app reads them (lookup). Chart
// "build" came from another Hydra version, "rules" from other rules in
// hydra_rules.ini, "both" from both. Returns the model's rows, in that order.
std::vector<hydra::ui::LibraryRow> stale_rows() {
    const Profile profile{"mode", CapQuery::at(8), hydra::store::Lens{}};
    hydra::store::RecordStore store(":memory:");
    hydra::test::add_stale_records(store, hydra::test::empty_analysis_under(profile), profile,
                                   "build", "rules", "both");

    LibraryModel m;
    m.set_charts({chart("build", "Build", "A", "C", "common"),
                  chart("rules", "Rules", "A", "C", "common"),
                  chart("both", "Both", "A", "C", "common")});
    m.set_summaries(store.lookup(m.hashes(), profile));
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
          "Out of date: this result came from another Hydra version. Click the song or run a "
          "batch to refresh it.");
}

TEST_CASE("library model: a row Stale from other rules names hydra_rules.ini in its tooltip") {
    const std::vector<hydra::ui::LibraryRow> rows = stale_rows();
    REQUIRE(rows.size() == 3);
    const hydra::ui::LibraryRow& row = rows[1];
    REQUIRE(row.status == RecordStatus::Stale);
    CHECK_FALSE(row.stale_build);
    CHECK(row.stale_rules);
    CHECK(row_tooltip(row) ==
          "Out of date: this result came from different rules in hydra_rules.ini. Click the "
          "song or run a batch to refresh it.");
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
          "in hydra_rules.ini. Click the song or run a batch to refresh it.");
}

TEST_CASE("library model: a Stale row whose cause changes takes the new cause") {
    LibraryModel m;
    m.set_charts({chart("h", "Song", "A", "C", "common")});
    RecordLookup by_build = stale();
    by_build.stale_build = true;
    REQUIRE(m.set_summaries({by_build}) == 1);
    RecordLookup by_rules = stale();
    by_rules.stale_rules = true;
    CHECK(m.set_summaries({by_rules}) == 1);
    CHECK_FALSE(m.rows()[0].stale_build);
    CHECK(m.rows()[0].stale_rules);
}

TEST_CASE("library model: every chart shows, sorted by title, with its Best path text") {
    const LibraryModel m = sample();
    CHECK(titles(m) == std::vector<std::string>{"Acid Romance", "Burnout", "Chair", "Halo",
                                                "Other", "YYZ"});
    CHECK(m.rows()[0].best_label() == "378,315  3- 1 2");
    CHECK(m.rows()[2].best_label() == "Stale");
    CHECK(m.rows()[1].best_label() == "Not analyzed");
    // Colour tags never reach the screen.
    CHECK(m.rows()[3].charter == "Bloodline");
    CHECK(m.counts().all == 6);
    CHECK(m.counts().not_analyzed == 4);
    CHECK(m.counts().stale == 1);
    CHECK(m.counts().analyzed == 1);
}

TEST_CASE("library model: a row keeps its chart's hash, file and folder, and a click reads the rest") {
    // One chart in two folders, under two names, plus another chart.
    ChartLibraryEntry first = chart("same", "First Name", "A", "C", "one");
    first.sig = "1:2:3:4";
    first.timing = hydra::store::ChartTimingMeta{180000.0, 50.0};
    ChartLibraryEntry second = chart("same", "Second Name", "B", "D", "two");
    second.sig = "5:6:7:8";
    second.timing = hydra::store::ChartTimingMeta{std::nullopt, std::nullopt};
    hydra::store::RecordStore store(":memory:");
    store.rebuild_chart_library({first, second, chart("other", "Other", "E", "F", "one")});

    LibraryModel m;
    m.set_charts(store.list_chart_library(0, -1));
    REQUIRE(m.rows().size() == 3);
    for (const hydra::ui::LibraryRow& row : m.rows()) {
        if (row.entry.md5 != "same") continue;
        const ChartLibraryEntry& scanned =
            row.entry.notespath == first.notespath ? first : second;
        CHECK(row.entry.rootfolder == scanned.rootfolder);
        CHECK(row.title == scanned.title);
    }

    // The click's read: every copy of the one chart, as the scan wrote it.
    const std::vector<ChartLibraryEntry> copies = store.list_chart_library_copies("same");
    REQUIRE(copies.size() == 2);
    for (const ChartLibraryEntry& copy : copies) {
        const ChartLibraryEntry& scanned = copy.notespath == first.notespath ? first : second;
        CHECK(copy.md5 == "same");
        CHECK(copy.title == scanned.title);
        CHECK(copy.artist == scanned.artist);
        CHECK(copy.charter == scanned.charter);
        CHECK(copy.rootfolder == scanned.rootfolder);
        CHECK(copy.sig == scanned.sig);
        CHECK(copy.timing == scanned.timing);
    }
    CHECK(store.list_chart_library_copies("missing").empty());
}

TEST_CASE("library model: a title made only of tags reads (unknown)") {
    LibraryModel m;
    m.set_charts({chart("tags", hydra::test::kTagOnlyTitle, "Artist", "Charter", "common")});
    REQUIRE(m.rows().size() == 1);
    CHECK(m.rows()[0].title == "(unknown)");
}

TEST_CASE("library model: an artist made only of tags reads (unknown), and search keeps the stored text") {
    // D50 item 5: the artist column follows the title's rule. The search still
    // reads the stored text, so "(unknown)" is not a search hit.
    LibraryModel m;
    m.set_charts({chart("tags", "Song", hydra::test::kTagOnlyTitle, "Charter", "common")});
    REQUIRE(m.rows().size() == 1);
    CHECK(m.rows()[0].artist == "(unknown)");
    CHECK(m.rows()[0].searchable.artist.find("unknown") == std::string::npos);
    m.set_query("unknown");
    CHECK(m.order().empty());
}

TEST_CASE("library model: an album draws without its tags, a blank one draws empty, and search folds it") {
    // display_album's text, with no placeholder for a blank album.
    LibraryModel m;
    m.set_charts({chart("a", "Song A", "Band", "C", "common", " <b>Dookie</b> "),
                  chart("b", "Song B", "Band", "C", "common", ""),
                  chart("c", "Song C", "Band", "C", "common", hydra::test::kTagOnlyTitle)});
    REQUIRE(m.rows().size() == 3);
    CHECK(m.rows()[0].album == "Dookie");
    CHECK(hydra::ui::library_cell_text(m.rows()[0], hydra::ui::kColumnAlbum) == "Dookie");
    CHECK(m.rows()[0].searchable.album == " dookie ");  // the stored text, folded
    CHECK(m.rows()[1].album.empty());
    CHECK(m.rows()[1].searchable.album.empty());
    CHECK(m.rows()[2].album.empty());
    m.set_query("album:dookie");
    CHECK(titles(m) == std::vector<std::string>{"Song A"});
    m.set_query("unknown");
    CHECK(m.order().empty());
}

TEST_CASE("library model: each column's position and its user ID are one mapping") {
    // The five shipped IDs keep their values and Album takes 5 while it is
    // drawn third (the user's call, 2026-10-24).
    using hydra::ui::column_of;
    using hydra::ui::sort_of;
    CHECK(hydra::ui::kLibraryColumnCount == 6);
    CHECK(hydra::ui::kColumnAlbum == 2);
    CHECK(hydra::ui::kColumnArtist < hydra::ui::kColumnAlbum);
    CHECK(hydra::ui::kColumnAlbum < hydra::ui::kColumnCharter);
    CHECK(static_cast<int>(LibrarySort::Title) == 0);
    CHECK(static_cast<int>(LibrarySort::Artist) == 1);
    CHECK(static_cast<int>(LibrarySort::Charter) == 2);
    CHECK(static_cast<int>(LibrarySort::Folder) == 3);
    CHECK(static_cast<int>(LibrarySort::BestPath) == 4);
    CHECK(static_cast<int>(LibrarySort::Album) == 5);
    CHECK(sort_of(0) == LibrarySort::Title);
    CHECK(sort_of(1) == LibrarySort::Artist);
    CHECK(sort_of(2) == LibrarySort::Album);
    CHECK(sort_of(3) == LibrarySort::Charter);
    CHECK(sort_of(4) == LibrarySort::Folder);
    CHECK(sort_of(5) == LibrarySort::BestPath);
    for (int c = 0; c < hydra::ui::kLibraryColumnCount; ++c) {
        INFO("column " << c);
        CHECK(column_of(sort_of(c)) == c);
    }
}

TEST_CASE("library model: Album sorts on the album its cell shows, blanks first") {
    LibraryModel m = sample();
    m.set_sort(LibrarySort::Album, true);
    // Halo and YYZ have no album; they tie and fall back to title.
    CHECK(titles(m) == std::vector<std::string>{"Halo", "YYZ", "Acid Romance", "Burnout", "Chair",
                                                "Other"});
    m.set_sort(LibrarySort::Album, false);
    CHECK(titles(m) == std::vector<std::string>{"Other", "Chair", "Burnout", "Acid Romance", "Halo",
                                                "YYZ"});
}

TEST_CASE("library model: an empty or placeholder artist or charter reads (unknown), and the "
          "charter is trimmed") {
    // D56 item 2 and D266 item 2: every missing artist or charter reads
    // "(unknown)". A present charter reads display_charter's text: tags gone,
    // the spaces at its ends trimmed.
    LibraryModel m;
    m.set_charts({chart("a", "Song A", "", " <b>Bob</b> ", "common"),
                  chart("b", "Song B", "<unknown artist>", "<unknown charter>", "common")});
    REQUIRE(m.rows().size() == 2);
    CHECK(m.rows()[0].artist == "(unknown)");
    CHECK(m.rows()[0].charter == "Bob");
    CHECK(m.rows()[1].artist == "(unknown)");
    CHECK(m.rows()[1].charter == "(unknown)");
}

TEST_CASE("library model: a name sorts on the text its cell shows, not the stored text") {
    // D114 item 3. A blank title and a placeholder artist show "(unknown)" and
    // sort there; a leading space the cell trims no longer moves the row.
    LibraryModel m;
    m.set_charts({chart("blank", "", "1 Band", "C", "common"),
                  chart("bang", "!!! Song", "<unknown artist>", "C", "common"),
                  chart("space", " Zebra", "Band", "C", "common"),
                  chart("apple", "Apple", "Cats", "C", "common")});
    CHECK(titles(m) == std::vector<std::string>{"!!! Song", "(unknown)", "Apple", "Zebra"});
    m.set_sort(LibrarySort::Artist, true);
    CHECK(titles(m) == std::vector<std::string>{"!!! Song", "(unknown)", "Zebra", "Apple"});
}

TEST_CASE("library model: a lookup that differs only in a field the library never reads changes nothing") {
    LibraryModel m = sample();
    const hydra::ui::LibraryRow& burnout = m.rows()[0];  // ready(378315, "3- 1 2", 7, 163.0)
    RecordLookup same = ready(378315, "3- 1 2", 7, 163.0);
    same.record.paths.front().actcount = 4;
    CHECK(burnout.shows_same_as(same));
    CHECK(m.set_summary_for("burnout", same) == 0);
    CHECK_FALSE(burnout.shows_same_as(ready(378315, "3- 1 2", 6, 163.0)));
    CHECK_FALSE(burnout.shows_same_as(stale()));
}

TEST_CASE("library model: status_label is the one source of the status words") {
    using hydra::ui::status_label;
    CHECK(std::string(status_label(RecordStatus::Ready)) == "Analyzed");
    CHECK(std::string(status_label(RecordStatus::Stale)) == "Stale");
    CHECK(std::string(status_label(RecordStatus::NotAnalyzed)) == "Not analyzed");
    CHECK(std::string(status_label(RecordStatus::CantAnalyze)) == "Can't analyze");
    // The Best path cell of a row with no current result shows the same word.
    const hydra::store::PathSummary none;
    CHECK(hydra::ui::best_path_label(RecordStatus::Stale, "", none) == "Stale");
    CHECK(hydra::ui::best_path_label(RecordStatus::NotAnalyzed, "", none) == "Not analyzed");
    CHECK(hydra::ui::best_path_label(RecordStatus::CantAnalyze, "", none) == "Can't analyze");
}

TEST_CASE("library model: status_of names the one status each chip groups") {
    using hydra::ui::StatusChip;
    using hydra::ui::status_of;
    CHECK(status_of(StatusChip::Analyzed) == RecordStatus::Ready);
    CHECK(status_of(StatusChip::Stale) == RecordStatus::Stale);
    CHECK(status_of(StatusChip::NotAnalyzed) == RecordStatus::NotAnalyzed);
    CHECK(status_of(StatusChip::CantAnalyze) == RecordStatus::CantAnalyze);
    CHECK_FALSE(status_of(StatusChip::All).has_value());
}

// One chart under each status, the Can't analyze one a failure the store kept
// (failure_record) and read back the way the app reads it (lookup).
TEST_CASE("library model: every status has its own chip, and All counts them all") {
    const Profile profile{"mode", CapQuery::at(8), hydra::store::Lens{}};
    const hydra::NoNotesError no_notes(hydra::Difficulty::Expert, true);
    hydra::store::RecordStore store(":memory:");
    store.save(hydra::store::failure_record("cant", profile, hydra::ErrorKind::NoNotes,
                                            no_notes.what()));
    const RecordLookup cant = store.lookup_one("cant", profile);
    REQUIRE(cant.status == RecordStatus::CantAnalyze);

    LibraryModel m;
    m.set_charts({chart("ready", "Ready", "A", "C", "common"),
                  chart("stale", "Stale", "A", "C", "common"),
                  chart("new", "New", "A", "C", "common"),
                  chart("cant", "Cant", "A", "C", "common")});
    m.set_summaries({ready(300000, "1 1", 6, 40.0), stale(), RecordLookup{}, cant});

    const hydra::ui::ChipCounts& c = m.counts();
    CHECK(c.analyzed == 1);
    CHECK(c.stale == 1);
    CHECK(c.not_analyzed == 1);
    CHECK(c.cant_analyze == 1);
    CHECK(c.all == 4);
    CHECK(c.of(StatusChip::CantAnalyze) == 1);
    CHECK(hydra::ui::chip_label(StatusChip::CantAnalyze, 1) == "Can't analyze (1)##chipcant");

    // The chip narrows the table to that one chart, which is what the
    // Analyze button counts and runs (D115).
    m.set_chip(StatusChip::CantAnalyze);
    CHECK(titles(m) == std::vector<std::string>{"Cant"});
    CHECK(m.narrowed());

    // The row keeps the stored kind and text, and its cell reads the word.
    const hydra::ui::LibraryRow& row = m.rows()[3];
    CHECK(row.failure_kind == hydra::ErrorKind::NoNotes);
    CHECK(row.failure_text == no_notes.what());
    CHECK(hydra::ui::library_cell_text(row, hydra::ui::kColumnBestPath) == "Can't analyze");

    // Re-analyzed after the file was fixed: the chip empties and falls back
    // to All.
    CHECK(m.set_summary_for("cant", ready(1000, "0", 1, 1.0)) == 1);
    CHECK(m.chip() == StatusChip::All);
    CHECK(m.counts().cant_analyze == 0);
    CHECK_FALSE(m.rows()[3].failure_kind.has_value());
}

// The hover of a Can't analyze cell words the stored failure as a live one of
// the same kind reads: a no-notes chart's sentence quotes its raw text, so
// there is no details line under it.
TEST_CASE("library model: a stored failure's hover reads as the live failure does") {
    const hydra::NoNotesError no_notes(hydra::Difficulty::Expert, true);
    const std::string hover =
        hydra::app::stored_failure_text(hydra::ErrorKind::NoNotes, no_notes.what());
    CHECK(hover == "No Expert Pro Drums notes in this chart.");
    CHECK(hover == hydra::app::plain_error_block(no_notes));

    // A kind whose sentence does not quote the raw text gets the details line
    // under it.
    const hydra::KindedError unreadable(hydra::ErrorKind::ChartUnreadable,
                                        "expected a [section] header");
    const std::string block =
        hydra::app::stored_failure_text(hydra::ErrorKind::ChartUnreadable, unreadable.what());
    CHECK(block == hydra::app::plain_error_block(unreadable));
    CHECK(block.find(hydra::app::plain_error(unreadable)) == 0);
    CHECK(block.find("expected a [section] header") != std::string::npos);
}

TEST_CASE("library model: a new failure text or kind changes the row") {
    LibraryModel m;
    m.set_charts({chart("cant", "Cant", "A", "C", "common")});
    RecordLookup first;
    first.status = RecordStatus::CantAnalyze;
    first.failure_kind = hydra::ErrorKind::ChartUnreadable;
    first.failure_text = "line 3";
    REQUIRE(m.set_summary_for("cant", first) == 1);
    CHECK(m.rows()[0].shows_same_as(first));
    RecordLookup other_text = first;
    other_text.failure_text = "line 4";
    CHECK_FALSE(m.rows()[0].shows_same_as(other_text));
    RecordLookup other_kind = first;
    other_kind.failure_kind = hydra::ErrorKind::ChartTimingRefused;
    CHECK_FALSE(m.rows()[0].shows_same_as(other_kind));
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
    m.set_query("album:quiet");
    CHECK(titles(m) == std::vector<std::string>{"Acid Romance"});
    CHECK(m.counts().all == 1);
    m.set_query("album:thrice");  // Thrice is an artist here, not an album
    CHECK(titles(m).empty());
    m.set_query("dookie");  // a plain word matches an album too
    CHECK(titles(m) == std::vector<std::string>{"Burnout"});
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
    // What the Analyze button runs: the rows on screen, the chip applied (D115).
    CHECK(m.order().size() == 1);

    // Chair is re-analyzed: the Stale group is empty, so the table goes back
    // to All instead of sitting empty.
    CHECK(m.set_summary_for("chair", ready(300000, "1 1", 6, 40.0)) == 1);
    CHECK(m.chip() == StatusChip::All);
    CHECK(m.order().size() == 6);
    CHECK(m.counts().analyzed == 2);
}

TEST_CASE("library model: the search or a chip narrows the table, and nothing else does") {
    LibraryModel m = sample();
    CHECK_FALSE(m.narrowed());                    // neither
    m.set_query("\"tier 4\"");
    CHECK(m.narrowed());                          // the search alone
    m.set_chip(StatusChip::NotAnalyzed);
    CHECK(m.narrowed());                          // both
    m.set_query("");
    CHECK(m.narrowed());                          // the chip alone
    CHECK(m.order().size() == 4);
    m.set_chip(StatusChip::All);
    m.set_query("stars:9");                       // a filter that does not parse
    CHECK_FALSE(m.narrowed());
    CHECK(m.order().size() == 6);
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

// A chart that can't be analyzed sorts after Not analyzed in both directions:
// it is the one row that never gains a score (unscored_rank).
TEST_CASE("library model: Best path sorts a Can't analyze row after Not analyzed both ways") {
    LibraryModel m = sample();
    RecordLookup cant;
    cant.status = RecordStatus::CantAnalyze;
    cant.failure_kind = hydra::ErrorKind::NoNotes;
    cant.failure_text = "x";
    m.set_summary_for("acid", cant);
    m.set_sort(LibrarySort::BestPath, false);
    CHECK(titles(m) == std::vector<std::string>{"Burnout", "Chair", "Halo", "Other", "YYZ",
                                                "Acid Romance"});
    m.set_sort(LibrarySort::BestPath, true);
    CHECK(titles(m) == std::vector<std::string>{"Burnout", "Chair", "Halo", "Other", "YYZ",
                                                "Acid Romance"});
}

// A result whose analysis kept no path is Analyzed, but it has no best path,
// so a stars: or squeeze filter has nothing to test on it (D51 call 11).
TEST_CASE("library model: a Ready row with no paths is Analyzed but has no facts") {
    LibraryModel m;
    m.set_charts({chart("empty", "Empty", "A", "C", "common"),
                  chart("scored", "Scored", "A", "C", "common")});
    RecordLookup no_paths;
    no_paths.status = RecordStatus::Ready;  // no score, no stars
    m.set_summaries({no_paths, ready(1000, "1", 7, 100.0)});
    m.set_query("stars:7");
    CHECK(titles(m) == std::vector<std::string>{"Scored"});
    m.set_query("squeeze<=200");
    CHECK(titles(m) == std::vector<std::string>{"Scored"});
    m.set_query("");
    CHECK(m.counts().analyzed == 2);
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

TEST_CASE("library model: library_cell_text is what each column shows") {
    using hydra::ui::library_cell_text;
    const LibraryModel m = sample();
    // Rows 0 (analyzed), 3 (tags in the charter) and 5 (no result) are in
    // scan order, as sample() lists them.
    CHECK(library_cell_text(m.rows()[0], hydra::ui::kColumnTitle) == "Burnout");
    CHECK(library_cell_text(m.rows()[0], hydra::ui::kColumnArtist) == "Green Day");
    CHECK(library_cell_text(m.rows()[0], hydra::ui::kColumnAlbum) == "Dookie");
    CHECK(library_cell_text(m.rows()[0], hydra::ui::kColumnCharter) == "Hoph2o");
    CHECK(library_cell_text(m.rows()[0], hydra::ui::kColumnFolder) ==
          "common\\Summer Blast _25 Setlist\\Tier 4");
    CHECK(library_cell_text(m.rows()[0], hydra::ui::kColumnBestPath) == "378,315  3- 1 2");
    CHECK(library_cell_text(m.rows()[3], hydra::ui::kColumnCharter) == "Bloodline");
    CHECK(library_cell_text(m.rows()[5], hydra::ui::kColumnBestPath) == "Not analyzed");
}

TEST_CASE("library model: only the Best path cell reads the row's summary") {
    // The table re-measures just the Best path column when a summary changes
    // (update_column_widths), so no other cell's text may depend on it.
    using hydra::ui::library_cell_text;
    LibraryModel m = sample();
    std::vector<std::string> before;
    for (int c = 0; c < hydra::ui::kLibraryColumnCount; ++c)
        before.push_back(library_cell_text(m.rows()[0], c));
    REQUIRE(m.set_summary_for("burnout", stale()) == 1);
    for (int c = 0; c < hydra::ui::kLibraryColumnCount; ++c) {
        INFO("column " << c);
        if (c == hydra::ui::kColumnBestPath)
            CHECK(library_cell_text(m.rows()[0], c) != before[static_cast<size_t>(c)]);
        else
            CHECK(library_cell_text(m.rows()[0], c) == before[static_cast<size_t>(c)]);
    }
}

TEST_CASE("library model: rows_version moves when the rows are replaced and at no other time") {
    LibraryModel m = sample();
    const std::uint64_t first = m.rows_version();
    m.set_summary_for("chair", ready(1000, "1", 3, 0.0));
    m.set_query("burnout");
    m.set_chip(StatusChip::Analyzed);
    m.set_sort(LibrarySort::Artist, false);
    CHECK(m.rows_version() == first);
    m.set_charts({chart("a", "A", "X", "Y", "common")});
    CHECK(m.rows_version() != first);
}

TEST_CASE("library model: query_version moves when the search changes and at no other time") {
    LibraryModel m = sample();
    const std::uint64_t first = m.query_version();
    m.set_summary_for("chair", ready(1000, "1", 3, 0.0));
    m.set_chip(StatusChip::Analyzed);
    m.set_sort(LibrarySort::Artist, false);
    m.set_charts({chart("a", "A", "X", "Y", "common")});
    CHECK(m.query_version() == first);
    m.set_query("burnout");
    const std::uint64_t searched = m.query_version();
    CHECK(searched != first);
    m.set_query("burnout");  // the same search again is no change
    CHECK(m.query_version() == searched);
    m.set_query("");
    CHECK(m.query_version() != searched);
}

TEST_CASE("library model: filtering 20,000 charts keeps the pinned row counts") {
    // query_matches runs for every row on every applied keystroke. The times
    // are printed for a person to read; a timing belongs in hydra_bench, not
    // in a pass/fail check that a busy machine can fail.
    std::vector<ChartLibraryEntry> charts;
    std::vector<RecordLookup> summaries;
    charts.reserve(20000);
    for (int i = 0; i < 20000; ++i) {
        char md5[16], title[32], artist[32], charter[64], folder[48];
        std::snprintf(md5, sizeof(md5), "h%05d", i);
        std::snprintf(title, sizeof(title), "Song %05d", i);
        std::snprintf(artist, sizeof(artist), "Artist %02d", i % 50);
        std::snprintf(charter, sizeof(charter), "<color=#e02222>Char</color>ter %d", i % 30);
        std::snprintf(folder, sizeof(folder), "Pack %03d\\Tier %d", i % 200, i % 7);
        charts.push_back(chart(md5, title, artist, charter, folder));
        // Only every third row has a summary, so its stars step on i / 3 to
        // give 5, 6 and 7 among those rows.
        summaries.push_back(i % 3 == 0 ? ready(300000 + i, "1 2 3", 5 + (i / 3) % 3, i % 90)
                                       : RecordLookup{});
    }
    LibraryModel m;
    auto t0 = std::chrono::steady_clock::now();
    m.set_charts(std::move(charts));
    m.set_summaries(summaries);
    MESSAGE("load + sort 20,000: "
            << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count()
            << " ms");

    // Each query's row count over the rows built above, pinned from one run
    // on 2026-10-10 at 3782c333 plus the stars change above.
    struct QueryRows {
        const char* query;
        size_t rows;
    };
    for (const QueryRows& qr : {QueryRows{"s", 20000}, QueryRows{"song 1", 18152},
                                QueryRows{"\"tier 4\"", 2857}, QueryRows{"artist 07 song", 2526},
                                QueryRows{"charter", 20000}, QueryRows{"stars:7", 2222},
                                QueryRows{"squeeze<=20 pack", 1561}, QueryRows{"zzqx", 0},
                                QueryRows{"", 20000}}) {
        t0 = std::chrono::steady_clock::now();
        m.set_query(qr.query);
        const double ms =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        MESSAGE("query \"" << std::string(qr.query) << "\": " << ms << " ms, " << m.order().size()
                           << " rows");
        CHECK_MESSAGE(m.order().size() == qr.rows, "query \"" << std::string(qr.query) << "\"");
    }
    t0 = std::chrono::steady_clock::now();
    m.set_sort(LibrarySort::BestPath, false);
    MESSAGE("sort by Best path: "
            << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count()
            << " ms");
}
