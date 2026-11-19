// Unit tests for ui/library_model: the in-memory library behind the main
// window's table. It filters with app/library_query, counts the status chips,
// sorts, and turns each stored summary into the Best path cell's text.

#include "doctest.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/user_messages.h"  // stored_failure_text: a Can't analyze row's tooltip
#include "core/error_kind.h"
#include "core/rules.h"
#include "corpus_util.h"       // charts_with_paths, scanned_entry: corpus charts and their results
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

// Three corpus charts (testdata/input) analysed under the default settings,
// each listed under the scan's own hash and fingerprint (corpus::scanned_entry),
// with its own result saved Stale for one cause: the first as another Hydra
// version's, the second under other rules in hydra_rules.ini, the third both.
// The rows are read through the store the way the app reads them (lookup), in
// that order.
std::vector<hydra::ui::LibraryRow> stale_rows() {
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    const std::vector<corpus::ChartWithPaths> charts = corpus::charts_with_paths(run.settings, 3);
    REQUIRE_MESSAGE(charts.size() == 3, "the corpus needs three charts with paths");
    hydra::store::RecordStore store(":memory:");
    std::vector<ChartLibraryEntry> entries;
    for (const corpus::ChartWithPaths& c : charts) entries.push_back(corpus::scanned_entry(c.chart));
    hydra::test::save_old_build_record(
        store, hydra::store::prepare_record(entries[0].md5, run.profile, charts[0].result.record));
    store.save(hydra::store::prepare_record(
        entries[1].md5, run.profile, hydra::test::other_rules_record(charts[1].result.record)));
    hydra::test::save_old_build_record(
        store, hydra::store::prepare_record(entries[2].md5, run.profile,
                                            hydra::test::other_rules_record(charts[2].result.record)));

    LibraryModel m;
    m.set_charts(entries);
    m.set_summaries(store.lookup(m.hashes(), run.profile));
    return m.rows();
}

}  // namespace

// The tooltip's sentences are stale_text's, and test_user_messages.cpp checks
// them; these check that each cause the store read reaches the row, which is
// what the tooltip is worded from (library_table.cpp).
TEST_CASE("library model: a row Stale from another Hydra version carries that cause") {
    const std::vector<hydra::ui::LibraryRow> rows = stale_rows();
    REQUIRE(rows.size() == 3);
    const hydra::ui::LibraryRow& row = rows[0];
    REQUIRE(row.status == RecordStatus::Stale);
    CHECK(row.stale_build);
    CHECK_FALSE(row.stale_rules);
}

TEST_CASE("library model: a row Stale from other rules carries that cause") {
    const std::vector<hydra::ui::LibraryRow> rows = stale_rows();
    REQUIRE(rows.size() == 3);
    const hydra::ui::LibraryRow& row = rows[1];
    REQUIRE(row.status == RecordStatus::Stale);
    CHECK_FALSE(row.stale_build);
    CHECK(row.stale_rules);
}

TEST_CASE("library model: a row Stale from both causes carries both") {
    const std::vector<hydra::ui::LibraryRow> rows = stale_rows();
    REQUIRE(rows.size() == 3);
    const hydra::ui::LibraryRow& row = rows[2];
    REQUIRE(row.status == RecordStatus::Stale);
    CHECK(row.stale_build);
    CHECK(row.stale_rules);
}

TEST_CASE("library model: a Stale row whose cause changes takes the new cause") {
    // One corpus chart's own result, saved as another Hydra version's, then
    // saved again under other rules in hydra_rules.ini: each re-read goes
    // through the store as the app's refresh does.
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    const corpus::ChartWithPaths c = corpus::first_chart_with_paths(run.settings);
    const ChartLibraryEntry entry = corpus::scanned_entry(c.chart);
    hydra::store::RecordStore store(":memory:");
    hydra::test::save_old_build_record(
        store, hydra::store::prepare_record(entry.md5, run.profile, c.result.record));

    LibraryModel m;
    m.set_charts({entry});
    REQUIRE(m.set_summaries(store.lookup(m.hashes(), run.profile)) == 1);
    REQUIRE(m.rows()[0].stale_build);
    REQUIRE_FALSE(m.rows()[0].stale_rules);

    store.save(hydra::store::prepare_record(entry.md5, run.profile,
                                            hydra::test::other_rules_record(c.result.record)));
    CHECK(m.set_summaries(store.lookup(m.hashes(), run.profile)) == 1);
    CHECK_FALSE(m.rows()[0].stale_build);
    CHECK(m.rows()[0].stale_rules);
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

TEST_CASE("library model: a missing charter sorts and searches as a missing artist does (D266 item 2)") {
    // Sort reads the shown text, so a placeholder or tags-only charter sorts
    // under "(unknown)". Search reads the stored text: the scan's placeholder
    // is a hit for charter:unknown, as <unknown artist> is for artist:unknown,
    // and a tags-only charter is not, as a tags-only artist is not.
    LibraryModel m;
    m.set_charts({chart("placeholder", "Song P", "<unknown artist>", "<unknown charter>", "common"),
                  chart("tags", "Song T", hydra::test::kTagOnlyTitle, hydra::test::kTagOnlyTitle,
                        "common"),
                  chart("named", "Song N", "Cats", "Bob", "common"),
                  chart("late", "Song L", "Dogs", "Zed", "common")});
    m.set_sort(LibrarySort::Charter, true);
    CHECK(titles(m) == std::vector<std::string>{"Song P", "Song T", "Song N", "Song L"});
    m.set_sort(LibrarySort::Artist, true);
    CHECK(titles(m) == std::vector<std::string>{"Song P", "Song T", "Song N", "Song L"});

    m.set_query("charter:unknown");
    CHECK(titles(m) == std::vector<std::string>{"Song P"});
    m.set_query("artist:unknown");
    CHECK(titles(m) == std::vector<std::string>{"Song P"});
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
    // drawn third.
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
    // sort there; a leading space the cell trims does not move the row.
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

// The rows of sample() under Best path, highest first, once YYZ has a new score
// that its row is still holding back from the sort.
namespace {

const std::vector<std::string> kHeldOrder{"Burnout", "Chair", "Acid Romance", "Halo", "Other",
                                          "YYZ"};
const std::vector<std::string> kScoreOrder{"YYZ", "Burnout", "Chair", "Acid Romance", "Halo",
                                           "Other"};

LibraryModel sample_with_held_yyz() {
    LibraryModel m = sample();
    m.set_sort(LibrarySort::BestPath, false);
    REQUIRE(m.set_summary_for("yyz", ready(500000, "2 2", 7, 12.0)) == 1);
    return m;
}

}  // namespace

// The user's words: "Yes, hold them still". New scores fill in where each row
// already sits, and the list re-sorts when it is asked to.
TEST_CASE("library model: a new score fills in where the row sits, and one call sorts it") {
    LibraryModel m = sample_with_held_yyz();
    CHECK(titles(m) == kHeldOrder);
    CHECK(m.rows()[m.order().back()].best_label() == "500,000  2 2");
    CHECK(m.counts().analyzed == 2);

    CHECK(m.sort_if_out_of_date());
    CHECK(titles(m) == kScoreOrder);
    CHECK_FALSE(m.sort_if_out_of_date());  // nothing is out of date now
}

TEST_CASE("library model: a whole re-read of the summaries keeps the rows in place too") {
    LibraryModel m = sample();
    m.set_sort(LibrarySort::BestPath, false);
    const RecordLookup none;
    REQUIRE(m.set_summaries({ready(378315, "3- 1 2", 7, 163.0), ready(500000, "2 2", 7, 12.0),
                             stale(), none, none, none}) == 1);
    CHECK(titles(m) == kHeldOrder);
    CHECK(m.sort_if_out_of_date());
    CHECK(titles(m) == kScoreOrder);
}

TEST_CASE("library model: answers that change nothing leave the order up to date") {
    LibraryModel m = sample();
    m.set_sort(LibrarySort::BestPath, false);
    REQUIRE(m.set_summary_for("burnout", ready(378315, "3- 1 2", 7, 163.0)) == 0);
    CHECK_FALSE(m.sort_if_out_of_date());
}

// Only the Best path sort reads a row's summary, so no other sort has an order
// a new summary can put out of date.
TEST_CASE("library model: a new score under another sort leaves nothing to re-sort") {
    LibraryModel m = sample();
    const std::vector<std::string> by_title = titles(m);
    REQUIRE(m.set_summary_for("yyz", ready(500000, "2 2", 7, 12.0)) == 1);
    CHECK(titles(m) == by_title);
    CHECK_FALSE(m.sort_if_out_of_date());
}

TEST_CASE("library model: a held row still leaves a chip it no longer belongs to") {
    LibraryModel m = sample();
    m.set_sort(LibrarySort::BestPath, false);
    m.set_chip(StatusChip::NotAnalyzed);
    REQUIRE(titles(m) == std::vector<std::string>{"Acid Romance", "Halo", "Other", "YYZ"});
    REQUIRE(m.set_summary_for("halo", ready(500000, "2 2", 7, 12.0)) == 1);
    CHECK(titles(m) == std::vector<std::string>{"Acid Romance", "Other", "YYZ"});
    CHECK(m.counts().not_analyzed == 3);
    CHECK(m.counts().analyzed == 2);
}

TEST_CASE("library model: a search, a chip or a header click sorts a held order at once") {
    LibraryModel m = sample_with_held_yyz();
    SUBCASE("a search") {
        m.set_query("e");
        CHECK(titles(m).front() == "YYZ");
    }
    SUBCASE("a chip") {
        m.set_chip(StatusChip::Analyzed);
        CHECK(titles(m) == std::vector<std::string>{"YYZ", "Burnout"});
    }
    SUBCASE("a header click") {
        m.set_sort(LibrarySort::BestPath, true);
        CHECK(titles(m) == std::vector<std::string>{"Burnout", "YYZ", "Chair", "Acid Romance",
                                                    "Halo", "Other"});
    }
    SUBCASE("a click on another column's header") {
        m.set_sort(LibrarySort::Title, true);
        CHECK(titles(m).back() == "YYZ");
    }
    CHECK_FALSE(m.sort_if_out_of_date());
}

// The table hands over its sort on its first frame whether or not it changed,
// and the search box and chips can repeat themselves: only a change counts.
TEST_CASE("library model: asking for the search, chip and sort already in place holds the order") {
    LibraryModel m = sample_with_held_yyz();
    m.set_query("");
    m.set_chip(StatusChip::All);
    m.set_sort(LibrarySort::BestPath, false);
    CHECK(titles(m) == kHeldOrder);
    CHECK(m.sort_if_out_of_date());
}

TEST_CASE("library model: new rows leave no held order behind") {
    LibraryModel m = sample_with_held_yyz();
    m.set_charts({chart("a", "A", "X", "Y", "common")});
    CHECK_FALSE(m.sort_if_out_of_date());
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

// The saved figure is the engine's, sign included; the squeeze filters read it
// through shown_timing (facts_of), so a figure below 0 needs no timing.
TEST_CASE("library model: the squeeze filters read the saved figure as shown") {
    LibraryModel m;
    m.set_charts({chart("inside", "Inside", "A", "C", "common"),
                  chart("onend", "On End", "A", "C", "common")});
    m.set_summaries({ready(1000, "1", 7, -315.8), ready(1000, "1", 7, 0.0)});
    // -315.8 needs no timing, which reaches no floor; the raw figure would match.
    m.set_query("squeeze>-400");
    CHECK(titles(m) == std::vector<std::string>{"On End"});
    // 0.0 is a timing to hit: it matches >=0 and <=0 and not the strict <0.
    m.set_query("squeeze>=0");
    CHECK(titles(m) == std::vector<std::string>{"On End"});
    m.set_query("squeeze<=0");
    CHECK(titles(m) == std::vector<std::string>{"Inside", "On End"});
    m.set_query("squeeze<0");
    CHECK(titles(m) == std::vector<std::string>{"Inside"});
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

namespace {

using Answers = std::vector<std::pair<std::string, RecordLookup>>;

// One chart in two folders and two other charts, all Not analyzed.
LibraryModel two_folders_and_two_more() {
    LibraryModel m;
    m.set_charts({chart("same", "Song", "A", "C", "Pack 1"), chart("same", "Song", "A", "C", "Pack 2"),
                  chart("first", "First", "B", "C", "Pack 1"),
                  chart("second", "Second", "D", "C", "Pack 2")});
    return m;
}

// The answer `answers` gives for `md5`.
const RecordLookup& answer_for(const Answers& answers, const std::string& md5) {
    const auto found = std::find_if(answers.begin(), answers.end(),
                                    [&](const auto& a) { return a.first == md5; });
    REQUIRE(found != answers.end());
    return found->second;
}

}  // namespace

TEST_CASE("library model: set_summaries_for gives every row the answer set_summary_for would") {
    RecordLookup cant;
    cant.status = RecordStatus::CantAnalyze;
    cant.failure_kind = hydra::ErrorKind::NoNotes;
    cant.failure_text = "x";
    const Answers answers{{"same", ready(1000, "1", 3, 0.0)},
                          {"first", stale()},
                          {"second", cant}};

    LibraryModel at_once = two_folders_and_two_more();
    const size_t changed_at_once = at_once.set_summaries_for(answers);

    LibraryModel one_by_one = two_folders_and_two_more();
    size_t changed_one_by_one = 0;
    for (const auto& [md5, lookup] : answers)
        changed_one_by_one += one_by_one.set_summary_for(md5, lookup);

    CHECK(changed_at_once == changed_one_by_one);
    CHECK(changed_at_once == at_once.rows().size());  // every row was Not analyzed
    REQUIRE(at_once.rows().size() == one_by_one.rows().size());
    for (size_t i = 0; i < at_once.rows().size(); ++i) {
        const RecordLookup& expected = answer_for(answers, at_once.rows()[i].entry.md5);
        CHECK(at_once.rows()[i].shows_same_as(expected));
        CHECK(one_by_one.rows()[i].shows_same_as(expected));
    }
    CHECK(at_once.counts().all == one_by_one.counts().all);
    CHECK(at_once.counts().not_analyzed == one_by_one.counts().not_analyzed);
    CHECK(at_once.counts().stale == one_by_one.counts().stale);
    CHECK(at_once.counts().analyzed == one_by_one.counts().analyzed);
    CHECK(at_once.counts().cant_analyze == one_by_one.counts().cant_analyze);
}

TEST_CASE("library model: set_summaries_for applies a chart's later answer when it is listed twice") {
    LibraryModel m = two_folders_and_two_more();
    const Answers answers{{"same", ready(1000, "1", 3, 0.0)}, {"same", ready(2000, "2", 4, 0.0)}};
    CHECK(m.set_summaries_for(answers) == 2);
    size_t shown = 0;
    for (const hydra::ui::LibraryRow& row : m.rows()) {
        if (row.entry.md5 != "same") continue;
        CHECK(row.shows_same_as(answers.back().second));
        ++shown;
    }
    CHECK(shown == 2);
}

TEST_CASE("library model: set_summaries_for updates both rows of a chart in two folders") {
    LibraryModel m = two_folders_and_two_more();
    const Answers answers{{"same", ready(1000, "1", 3, 0.0)}};
    CHECK(m.set_summaries_for(answers) == 2);
    size_t shown = 0;
    for (const hydra::ui::LibraryRow& row : m.rows()) {
        if (row.entry.md5 != "same") continue;
        CHECK(row.shows_same_as(answers.front().second));
        ++shown;
    }
    CHECK(shown == 2);
    CHECK(m.counts().analyzed == 2);
    CHECK(m.counts().not_analyzed == 2);  // first and second are untouched
}

// Many new scores at once fill in where their rows sit, as one does through
// set_summary_for.
TEST_CASE("library model: set_summaries_for holds the row order under the Best path sort until "
          "sort_if_out_of_date") {
    LibraryModel m = sample();
    m.set_sort(LibrarySort::BestPath, false);
    const std::vector<std::string> before = titles(m);
    REQUIRE(m.set_summaries_for({{"yyz", ready(500000, "2 2", 7, 12.0)},
                                 {"halo", ready(400000, "1 1", 6, 10.0)}}) == 2);
    CHECK(titles(m) == before);
    CHECK(m.counts().analyzed == 3);

    CHECK(m.sort_if_out_of_date());
    // The scored rows come first, each score no higher than the one above it,
    // then the unscored rows in the order they held before.
    std::vector<std::string> unscored_before;
    for (const std::string& title : before)
        for (const hydra::ui::LibraryRow& row : m.rows())
            if (row.title == title && row.status != RecordStatus::Ready)
                unscored_before.push_back(title);
    std::vector<std::string> unscored_after;
    std::optional<int64_t> last_score;
    for (size_t i : m.order()) {
        const hydra::ui::LibraryRow& row = m.rows()[i];
        if (row.status != RecordStatus::Ready) {
            unscored_after.push_back(row.title);
            continue;
        }
        INFO("row " << row.title);
        CHECK(unscored_after.empty());  // no scored row below an unscored one
        REQUIRE(row.summary.score.has_value());
        if (last_score) CHECK(*row.summary.score <= *last_score);
        last_score = row.summary.score;
    }
    CHECK(unscored_after == unscored_before);
    CHECK_FALSE(m.sort_if_out_of_date());
}

TEST_CASE("library model: set_summaries_for with answers that change nothing rebuilds nothing") {
    LibraryModel m = sample();
    m.set_sort(LibrarySort::BestPath, false);
    const hydra::ui::ChipCounts before = m.counts();
    // Each answer is the one sample() already gave its row.
    CHECK(m.set_summaries_for({{"burnout", ready(378315, "3- 1 2", 7, 163.0)},
                               {"chair", stale()},
                               {"yyz", RecordLookup{}}}) == 0);
    CHECK(m.set_summaries_for({}) == 0);
    CHECK(m.counts().all == before.all);
    CHECK(m.counts().not_analyzed == before.not_analyzed);
    CHECK(m.counts().stale == before.stale);
    CHECK(m.counts().analyzed == before.analyzed);
    CHECK(m.counts().cant_analyze == before.cant_analyze);
    // A rebuild under the Best path sort would have left the order out of
    // date (summaries_changed); nothing did.
    CHECK_FALSE(m.sort_if_out_of_date());
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

namespace {

// Rows whose shown text differs from their tag-stripped stored text (spaces
// trimmed, a placeholder shown for an empty name, a folder shown with its
// tags) and rows where the two are the same (tags, accents, full-width
// letters). Made-up rows. The ideographic space is U+3000.
std::vector<ChartLibraryEntry> pinned_charts() {
    return {
        chart("tags", "<color=#ff0000>Red</color> Song", "<b>The</b> Band", "<i>Hoph2o</i>",
              "common\\<b>Tagged</b>", "<size=20>Big</size> Album"),
        chart("spaces", "  Spaced Out ", " Lead Artist", "Charter\t", "pack", " Album  "),
        chart("empty", "", "<unknown artist>", "<unknown charter>", "pack", ""),
        chart("accents", "Beyoncé Æther", "Motörhead", "Łukasz", "common\\Café", "Straße"),
        chart("wide", "ＢＵＲＮＯＵＴ！", "Ｇｒｅｅｎ" "\xE3\x80\x80" "Ｄａｙ", "東京事変", "ｗｉｄｅ",
              "Ａｌｂｕｍ" "\xE3\x80\x80"),
    };
}

// The row `entry` makes in a library of that one chart. set_charts builds a
// library of one row on the calling thread, so this is the one-thread build.
hydra::ui::LibraryRow one_thread_row(const ChartLibraryEntry& entry) {
    LibraryModel m;
    m.set_charts({entry});
    REQUIRE(m.rows().size() == 1);
    return m.rows()[0];
}

// Every field set_charts writes: the kept chart, the shown names, the search
// text and the sort keys.
void check_same_row(const hydra::ui::LibraryRow& got, const hydra::ui::LibraryRow& want) {
    CHECK(got.entry.md5 == want.entry.md5);
    CHECK(got.entry.notespath == want.entry.notespath);
    CHECK(got.entry.rootfolder == want.entry.rootfolder);
    CHECK(got.title == want.title);
    CHECK(got.artist == want.artist);
    CHECK(got.album == want.album);
    CHECK(got.charter == want.charter);
    CHECK(got.searchable.title == want.searchable.title);
    CHECK(got.searchable.artist == want.searchable.artist);
    CHECK(got.searchable.album == want.searchable.album);
    CHECK(got.searchable.charter == want.searchable.charter);
    CHECK(got.searchable.folder == want.searchable.folder);
    CHECK(got.sort_keys == want.sort_keys);
}

}  // namespace

TEST_CASE("library model: each row's search text and sort keys are pinned") {
    // Every literal is the fold of the text the field names, written out by
    // hand; a sort key taken from the wrong text fails.
    LibraryModel m;
    m.set_charts(pinned_charts());
    struct Want {
        const char* md5;
        // By column: title, artist, album, charter, folder.
        std::array<const char*, hydra::ui::kColumnBestPath> keys;
        // make_searchable's fields: title, artist, album, charter, folder.
        std::array<const char*, 5> search;
    };
    const Want want[] = {
        {"tags",
         {"red song", "the band", "big album", "hoph2o", "common\\<b>tagged</b>"},
         {"red song", "the band", "big album", "hoph2o", "common\\tagged"}},
        {"spaces",
         {"spaced out", "lead artist", "album", "charter", "pack"},
         {" spaced out ", " lead artist", " album ", "charter ", "pack"}},
        {"empty",
         {"(unknown)", "(unknown)", "", "(unknown)", "pack"},
         {"", "<unknown artist>", "", "<unknown charter>", "pack"}},
        {"accents",
         {"beyonce aether", "motorhead", "strasse", "lukasz", "common\\cafe"},
         {"beyonce aether", "motorhead", "strasse", "lukasz", "common\\cafe"}},
        {"wide",
         {"burnout!", "green day", "album ", "東京事変", "wide"},
         {"burnout!", "green day", "album ", "東京事変", "wide"}},
    };
    REQUIRE(m.rows().size() == std::size(want));
    for (size_t i = 0; i < std::size(want); ++i) {
        // set_charts keeps its rows in the order it was given them.
        const hydra::ui::LibraryRow& row = m.rows()[i];
        INFO("row " << want[i].md5);
        REQUIRE(row.entry.md5 == want[i].md5);
        for (size_t c = 0; c < want[i].keys.size(); ++c) {
            INFO("column " << c);
            CHECK(row.sort_keys[c] == want[i].keys[c]);
        }
        CHECK(row.searchable.title == want[i].search[0]);
        CHECK(row.searchable.artist == want[i].search[1]);
        CHECK(row.searchable.album == want[i].search[2]);
        CHECK(row.searchable.charter == want[i].search[3]);
        CHECK(row.searchable.folder == want[i].search[4]);
    }
    // The default sort, Title ascending, on those keys.
    CHECK(titles(m) == std::vector<std::string>{"(unknown)", "Beyoncé Æther", "ＢＵＲＮＯＵＴ！",
                                                "Red Song", "Spaced Out"});
}

TEST_CASE("library model: rows built on threads match the one-thread rows, in the same order") {
    // Enough copies of the pinned rows for more rows than set_charts has
    // threads, and a row count that is not a multiple of the threads, so the
    // shares are uneven. Each copy gets its own hash.
    static constexpr int kThreads = static_cast<int>(hydra::store::kLibraryOpenThreads);
    static constexpr int kPinned = 5;  // pinned_charts().size(), checked below
    constexpr int kCopies = [] {
        int copies = kThreads / kPinned + 1;
        while ((copies * kPinned) % kThreads == 0) ++copies;
        return copies;
    }();
    static_assert(kCopies * kPinned > kThreads);
    static_assert((kCopies * kPinned) % kThreads != 0);
    REQUIRE(pinned_charts().size() == static_cast<size_t>(kPinned));
    std::vector<ChartLibraryEntry> charts;
    for (int copy = 0; copy < kCopies; ++copy)
        for (ChartLibraryEntry e : pinned_charts()) {
            e.md5 += std::to_string(copy);
            charts.push_back(std::move(e));
        }
    const std::vector<ChartLibraryEntry> given = charts;

    LibraryModel m;
    m.set_charts(std::move(charts));
    REQUIRE(m.rows().size() == given.size());
    for (size_t i = 0; i < given.size(); ++i) {
        // set_charts keeps its rows in the order it was given them.
        INFO("row " << i << " " << given[i].md5);
        check_same_row(m.rows()[i], one_thread_row(given[i]));
    }
    // The default sort, Title ascending: the pinned order, each title once per copy.
    std::vector<std::string> want;
    for (const char* title :
         {"(unknown)", "Beyoncé Æther", "ＢＵＲＮＯＵＴ！", "Red Song", "Spaced Out"})
        want.insert(want.end(), static_cast<size_t>(kCopies), std::string(title));
    CHECK(titles(m) == want);
}

TEST_CASE("library model: a library of 0, 1 or 3 charts loads, fewer than the threads") {
    const std::vector<ChartLibraryEntry> pinned = pinned_charts();
    REQUIRE(pinned.size() >= 3);
    // Every size here is below the thread count, which is what the name says.
    static_assert(size_t{3} < hydra::store::kLibraryOpenThreads);

    LibraryModel none;
    none.set_charts({});
    CHECK(none.rows().empty());
    CHECK(none.order().empty());
    CHECK(none.counts().all == 0);

    LibraryModel one;
    one.set_charts({pinned[0]});
    REQUIRE(one.rows().size() == 1);
    CHECK(titles(one) == std::vector<std::string>{"Red Song"});
    CHECK(one.counts().all == 1);

    LibraryModel three;
    three.set_charts({pinned[0], pinned[1], pinned[2]});
    REQUIRE(three.rows().size() == 3);
    for (size_t i = 0; i < 3; ++i) {
        INFO("row " << pinned[i].md5);
        check_same_row(three.rows()[i], one_thread_row(pinned[i]));
    }
    CHECK(titles(three) == std::vector<std::string>{"(unknown)", "Red Song", "Spaced Out"});
    CHECK(three.counts().all == 3);
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

    // Each query's row count, worked from the generator above: which of its
    // rows have the asked-for text or fact.
    const auto count_where = [](auto&& keep) {
        size_t n = 0;
        for (int i = 0; i < 20000; ++i)
            if (keep(i)) ++n;
        return n;
    };
    const auto has = [](const std::string& text, const char* word) {
        return text.find(word) != std::string::npos;
    };
    // The folded text of row i's title and artist; every row's folder holds
    // "pack" and its charter "charter", and neither holds "song" or "artist".
    const auto title_of = [](int i) {
        char t[32];
        std::snprintf(t, sizeof(t), "song %05d", i);
        return std::string(t);
    };
    const auto artist_of = [](int i) {
        char a[32];
        std::snprintf(a, sizeof(a), "artist %02d", i % 50);
        return std::string(a);
    };
    const auto folder_of = [](int i) {
        char f[48];
        std::snprintf(f, sizeof(f), "pack %03d\\tier %d", i % 200, i % 7);
        return std::string(f);
    };
    const auto charter_of = [](int i) { return "charter " + std::to_string(i % 30); };
    // Every word of a plain search must be in some field of the row.
    const auto words_match = [&](int i, std::initializer_list<const char*> words) {
        const std::string fields[] = {title_of(i), artist_of(i), charter_of(i), folder_of(i)};
        for (const char* w : words) {
            bool found = false;
            for (const std::string& f : fields) found = found || has(f, w);
            if (!found) return false;
        }
        return true;
    };
    // stars: and squeeze filters match only the rows given a summary, i % 3 == 0.
    struct QueryRows {
        const char* query;
        size_t rows;
    };
    const QueryRows queries[] = {
        {"s", count_where([&](int i) { return words_match(i, {"s"}); })},
        {"song 1", count_where([&](int i) { return words_match(i, {"song", "1"}); })},
        {"\"tier 4\"", count_where([](int i) { return i % 7 == 4; })},
        {"artist 07 song", count_where([&](int i) { return words_match(i, {"artist", "07", "song"}); })},
        {"charter", count_where([&](int i) { return words_match(i, {"charter"}); })},
        {"stars:7", count_where([](int i) { return i % 3 == 0 && 5 + (i / 3) % 3 == 7; })},
        {"squeeze<=20 pack", count_where([&](int i) {
             return i % 3 == 0 && i % 90 <= 20 && words_match(i, {"pack"});
         })},
        {"zzqx", count_where([&](int i) { return words_match(i, {"zzqx"}); })},
        {"", 20000},
    };
    for (const QueryRows& qr : queries) {
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
