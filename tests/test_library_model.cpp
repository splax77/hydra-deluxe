// Unit tests for ui/library_model: the in-memory library behind the main
// window's table. It filters with app/library_query, counts the status chips,
// sorts, and turns each stored summary into the Best path cell's text.

#include "doctest.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"       // discover_charts, save_scan_as_library, prepare_failed_chart
#include "app/library_query.h"  // fold_for_search, strip_rich_tags: what search reads
#include "app/user_messages.h"  // stored_failure_text: a Can't analyze row's tooltip
#include "audio_chart_fixtures.h"  // write_text_file: the song folders' files
#include "chart_text.h"            // chart_text: the song folders' short drum charts
#include "core/error_kind.h"
#include "core/model.h"  // shown_timing, timing_within_limit: what the squeeze filter reads
#include "core/rules.h"
#include "core/stars.h"   // kMaxStars: the top of the stars: filter
#include "core/winstr.h"  // parent_folder, join_folder: a chart's song folder
#include "corpus_util.h"      // charts_with_paths, scanned_entry: corpus charts and their results
#include "display_fixtures.h"  // kTagOnlyTitle, add_stale_records, empty_analysis_under
#include "temp_util.h"         // temp_dir, remove_dir: the scan's scratch root
#include "store/record_store.h"
#include "ui/library_model.h"

using hydra::store::CapQuery;
using hydra::store::ChartLibraryEntry;
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

// One song folder a test writes for the library scan: the folder's name, the
// lines of its song.ini under [song], and which short drum chart it holds
// (own_drum_chart). A chart of -1 is the folder's own, numbered by its place
// in the list, so two folders share a hash only when a test says so. A folder
// with a `copy_of` holds a copy of that corpus chart file instead, beside its
// corpus song.ini with the folder's own lines in place of its names.
struct SongFolder {
    std::string name;
    std::string ini;
    int chart = -1;
    std::string copy_of;
};

// The song.ini key of one line, its spaces dropped and lower-cased; empty for
// a line with no '='.
std::string ini_key(const std::string& line) {
    const size_t eq = line.find('=');
    if (eq == std::string::npos) return {};
    std::string key;
    for (size_t i = 0; i < eq; ++i) {
        const unsigned char c = static_cast<unsigned char>(line[i]);
        if (!std::isspace(c)) key.push_back(static_cast<char>(std::tolower(c)));
    }
    return key;
}

// The corpus song.ini beside `chart` with its name, artist, album and charter
// lines (and frets, the charter's other key) left out, then the lines `own`.
std::string copied_song_ini(const std::string& chart, const std::string& own) {
    const std::string text =
        corpus::read_bytes(hydra::join_folder(hydra::parent_folder(chart), "song.ini"));
    std::string out;
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(start, end - start);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const std::string key = ini_key(line);
        if (key != "name" && key != "artist" && key != "album" && key != "charter" &&
            key != "frets")
            out += line + "\n";
        start = end + 1;
    }
    return out + own;
}

// A two-note drum chart whose second note sits on a tick of its own, so the
// k-th chart hashes differently from every other.
std::string own_drum_chart(int k) {
    using namespace testchart;
    return chart_text(section("ExpertDrums", line(0, "N 0 0") + line(192 * (k + 1), "N 1 0")));
}

// Writes each folder under a fresh scratch root named by `tag`, then runs the
// app's library scan on that root (discover_charts). Fails on any scan error,
// or when the scan finds a different number of charts than were written.
std::vector<hydra::app::ScanItem> scan_song_folders(const std::string& tag,
                                                    const std::vector<SongFolder>& folders) {
    testtemp::remove_dir(testtemp::temp_dir(tag));
    const std::string root = testtemp::temp_dir(tag);
    for (size_t k = 0; k < folders.size(); ++k) {
        const std::string dir = hydra::join_folder(root, folders[k].name);
        std::filesystem::create_directories(hydra::os_path(dir));
        if (!folders[k].copy_of.empty()) {
            const std::string& from = folders[k].copy_of;
            const std::string file = from.substr(from.find_last_of("\\/") + 1);
            std::filesystem::copy_file(hydra::os_path(from),
                                       hydra::os_path(hydra::join_folder(dir, file)));
            audiochart::write_text_file(hydra::join_folder(dir, "song.ini"),
                                        copied_song_ini(from, folders[k].ini));
            continue;
        }
        audiochart::write_text_file(hydra::join_folder(dir, "song.ini"),
                                    "[song]\n" + folders[k].ini);
        const int chart_no = folders[k].chart < 0 ? static_cast<int>(k) : folders[k].chart;
        audiochart::write_text_file(hydra::join_folder(dir, "notes.chart"),
                                    own_drum_chart(chart_no));
    }
    std::pair<std::vector<hydra::app::ScanItem>, std::vector<std::string>> scan =
        hydra::app::discover_charts({root});
    REQUIRE_MESSAGE(scan.second.empty(), "scan error: " << scan.second.front());
    REQUIRE(scan.first.size() == folders.size());
    return std::move(scan.first);
}

// The library's charts as the app loads them: the scan saved as the library
// (save_scan_as_library) and listed back from the store.
std::vector<ChartLibraryEntry> scanned_entries(const std::string& tag,
                                               const std::vector<SongFolder>& folders) {
    hydra::store::RecordStore store(":memory:");
    const std::optional<std::string> error =
        hydra::app::save_scan_as_library(store, scan_song_folders(tag, folders));
    REQUIRE_MESSAGE(!error, "saving the scan failed: " << *error);
    return store.list_chart_library(0, -1);
}

// The library as the app loads it, from scanned_entries.
LibraryModel scanned_library(const std::string& tag, const std::vector<SongFolder>& folders) {
    LibraryModel m;
    m.set_charts(scanned_entries(tag, folders));
    REQUIRE(m.rows().size() == folders.size());
    return m;
}

// True when `notespath` is a chart in the song folder named `name`.
bool in_folder(const std::string& notespath, const std::string& name) {
    const std::string folder = hydra::parent_folder(notespath);
    return hydra::join_folder(hydra::parent_folder(folder), name) == folder;
}

// The row of the song folder named `name`.
const hydra::ui::LibraryRow& row_in(const LibraryModel& m, const std::string& name) {
    for (const hydra::ui::LibraryRow& row : m.rows())
        if (in_folder(row.entry.notespath, name)) return row;
    throw std::runtime_error("no library row for the song folder " + name);
}

// The scan's item for the song folder named `name`.
const hydra::app::ScanItem& item_in(const std::vector<hydra::app::ScanItem>& items,
                                    const std::string& name) {
    for (const hydra::app::ScanItem& item : items)
        if (in_folder(item.notespath, name)) return item;
    throw std::runtime_error("the scan found no chart in the song folder " + name);
}

// What a person types to find a song shown as kUnknownTitle: the word inside
// its brackets.
std::string unknown_word() {
    const std::string shown = hydra::kUnknownTitle;
    REQUIRE(shown.size() > 2);
    REQUIRE(shown.front() == '(');
    REQUIRE(shown.back() == ')');
    return shown.substr(1, shown.size() - 2);
}

// Six scanned song folders, in the shapes of the scratch library: three under
// "Tier 4" hold copies of three corpus charts (testdata/input) with paths, and
// three hold short drum charts of their own. Burnout's chart is analysed and
// saved, Chair's saved as another Hydra version's (Stale), and the rest have
// nothing saved. Halo and YYZ have no album. Every answer is read back from
// the store the way the app reads it (lookup), and the answers kept here are
// the store's later ones: YYZ's once its chart is analysed and saved (it
// holds the highest-scoring of the three charts), and Chair's once it is
// analysed again.
struct ScannedSample {
    LibraryModel model;
    std::vector<ChartLibraryEntry> entries;  // the charts the store listed, in row order
    std::string burnout, yyz, chair;  // the three analysed charts' hashes
    RecordLookup burnout_ready;       // Burnout's answer, as the model holds it
    RecordLookup yyz_ready;           // YYZ's answer once its result is saved
    RecordLookup chair_ready;         // Chair's answer once it is analysed again
    std::vector<RecordLookup> answers;   // every row's answer, as the model holds it
    std::vector<RecordLookup> with_yyz;  // every row's answer once YYZ's is saved
};

ScannedSample build_scanned_sample() {
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    std::vector<corpus::ChartWithPaths> charts = corpus::charts_with_paths(run.settings, 3);
    REQUIRE_MESSAGE(charts.size() == 3, "the corpus needs three charts with paths");
    // Lowest best path score first: Burnout gets the lowest, YYZ the highest.
    std::sort(charts.begin(), charts.end(),
              [](const corpus::ChartWithPaths& a, const corpus::ChartWithPaths& b) {
                  return a.result.record.paths.front().totalscore() <
                         b.result.record.paths.front().totalscore();
              });
    const std::vector<SongFolder> folders{
        {"Tier 4\\burnout", "name = Burnout\nartist = Green Day\ncharter = Hoph2o\nalbum = Dookie\n",
         -1, charts[0].chart},
        {"Tier 4\\yyz", "name = YYZ\nartist = Rush\ncharter = Harmonix, Onyxite\n", -1,
         charts[2].chart},
        {"Tier 4\\chair", "name = Chair\nartist = Sufferer\ncharter = Satan\nalbum = Sufferer\n", -1,
         charts[1].chart},
        {"Other\\acid",
         "name = Acid Romance\nartist = Some Band\ncharter = <color=#e02222>Blood</color>line\n"
         "album = A Quiet Place To Die\n"},
        {"Other\\halo", "name = Halo\nartist = Beyoncé\ncharter = Someone\n"},
        {"IB24\\T4\\other",
         "name = Other\nartist = Thrice\ncharter = Someone\nalbum = The Illusion of Safety\n"}};
    hydra::store::RecordStore store(":memory:");
    const std::optional<std::string> error =
        hydra::app::save_scan_as_library(store, scan_song_folders("lm_sample", folders));
    REQUIRE_MESSAGE(!error, "saving the scan failed: " << *error);

    ScannedSample s;
    s.entries = store.list_chart_library(0, -1);
    s.model.set_charts(s.entries);
    REQUIRE(s.model.rows().size() == folders.size());
    const std::string burnout_path = row_in(s.model, "burnout").entry.notespath;
    const std::string yyz_path = row_in(s.model, "yyz").entry.notespath;
    const std::string chair_path = row_in(s.model, "chair").entry.notespath;
    s.burnout = row_in(s.model, "burnout").entry.md5;
    s.yyz = row_in(s.model, "yyz").entry.md5;
    s.chair = row_in(s.model, "chair").entry.md5;

    store.save(hydra::store::prepare_record(
        s.burnout, run.profile, hydra::app::analyze_chart_file(burnout_path, run.settings).record));
    const hydra::HydraRecord chair_record =
        hydra::app::analyze_chart_file(chair_path, run.settings).record;
    hydra::test::save_old_build_record(store,
                                       hydra::store::prepare_record(s.chair, run.profile, chair_record));
    s.answers = store.lookup(s.model.hashes(), run.profile);
    s.model.set_summaries(s.answers);
    s.burnout_ready = store.lookup_one(s.burnout, run.profile);

    store.save(hydra::store::prepare_record(
        s.yyz, run.profile, hydra::app::analyze_chart_file(yyz_path, run.settings).record));
    s.yyz_ready = store.lookup_one(s.yyz, run.profile);
    s.with_yyz = store.lookup(s.model.hashes(), run.profile);

    store.save(hydra::store::prepare_record(s.chair, run.profile, chair_record));
    s.chair_ready = store.lookup_one(s.chair, run.profile);

    REQUIRE(row_in(s.model, "burnout").status == RecordStatus::Ready);
    REQUIRE(row_in(s.model, "chair").status == RecordStatus::Stale);
    REQUIRE(s.yyz_ready.status == RecordStatus::Ready);
    REQUIRE(s.chair_ready.status == RecordStatus::Ready);
    const std::optional<int64_t> low = s.burnout_ready.record.summary().score;
    const std::optional<int64_t> high = s.yyz_ready.record.summary().score;
    REQUIRE(low.has_value());
    REQUIRE(high.has_value());
    REQUIRE_MESSAGE(*high > *low, "YYZ's chart must outscore Burnout's");
    return s;
}

// Built once: the scan and the three analyses are the slow part.
const ScannedSample& scanned_sample() {
    static const ScannedSample lib = build_scanned_sample();
    return lib;
}

// A library of one scanned song folder of its own, under a scratch root named
// by `tag`: what a fresh scan hands set_charts.
std::vector<ChartLibraryEntry> one_scanned_chart(const std::string& tag) {
    const std::vector<ChartLibraryEntry> entries =
        scanned_entries(tag, {{"a", "name = A\nartist = X\ncharter = Y\n"}});
    REQUIRE(entries.size() == 1);
    return entries;
}

// Four scanned song folders: one corpus chart (testdata/input) copied into two
// folders, so both rows list one hash, and two folders of their own. Every row
// starts Not analyzed. `ready` is the copied chart's own result, saved and read
// back from the store the way the app reads it (lookup_one).
struct CopiedChart {
    LibraryModel model;
    std::string md5;     // the copied chart's hash, which both copies share
    RecordLookup ready;  // its answer once its result is saved
};

CopiedChart build_copied_chart() {
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    const corpus::ChartWithPaths c = corpus::first_chart_with_paths(run.settings);
    const std::vector<SongFolder> folders{
        {"Pack 1\\same_a", "name = Song\nartist = A\ncharter = C\n", -1, c.chart},
        {"Pack 2\\same_b", "name = Song\nartist = A\ncharter = C\n", -1, c.chart},
        {"Pack 1\\first", "name = First\nartist = B\ncharter = C\n"},
        {"Pack 2\\second", "name = Second\nartist = D\ncharter = C\n"}};
    CopiedChart out;
    out.model = scanned_library("lm_copied", folders);
    out.md5 = row_in(out.model, "same_a").entry.md5;
    REQUIRE(row_in(out.model, "same_b").entry.md5 == out.md5);
    REQUIRE(row_in(out.model, "first").entry.md5 != out.md5);
    REQUIRE(row_in(out.model, "second").entry.md5 != out.md5);
    hydra::store::RecordStore store(":memory:");
    store.save(hydra::store::prepare_record(out.md5, run.profile, c.result.record));
    out.ready = store.lookup_one(out.md5, run.profile);
    REQUIRE(out.ready.status == RecordStatus::Ready);
    return out;
}

// Built once: the corpus chart's analysis is the slow part.
const CopiedChart& copied_chart() {
    static const CopiedChart lib = build_copied_chart();
    return lib;
}

// A chart file that fails to analyze, as the app keeps it: `text` written as
// `file_name` under a fresh scratch folder named by `tag`, beside a song.ini,
// analysed as the batch does (analyze_chart_file), its failure kept through
// the batch's own step (prepare_failed_chart) and read back from `store`.
// The live failure's words are kept beside it, from their owners.
struct FailedChart {
    ChartLibraryEntry entry;
    RecordLookup lookup;        // the store's answer for the chart
    std::string live_block;     // plain_error_block of the live failure
    std::string live_sentence;  // plain_error of it
    std::string live_detail;    // plain_error_detail of it
    std::string live_what;      // its raw text
};

FailedChart kept_failure(hydra::store::RecordStore& store, const std::string& tag,
                         const std::string& file_name, const std::string& text) {
    testtemp::remove_dir(testtemp::temp_dir(tag));
    const std::string dir = testtemp::temp_dir(tag);
    audiochart::write_text_file(hydra::join_folder(dir, "song.ini"), "[song]\nname = " + tag + "\n");
    const std::string path = hydra::join_folder(dir, file_name);
    audiochart::write_text_file(path, text);
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    FailedChart out;
    out.entry = corpus::scanned_entry(path, tag);
    std::optional<hydra::store::PreparedRecord> kept;
    try {
        hydra::app::analyze_chart_file(path, run.settings);
    } catch (const std::exception& e) {
        out.live_block = hydra::app::plain_error_block(e);
        out.live_sentence = hydra::app::plain_error(e);
        out.live_detail = hydra::app::plain_error_detail(e);
        out.live_what = e.what();
        kept = hydra::app::prepare_failed_chart(out.entry.md5, run.profile, e);
    }
    REQUIRE_MESSAGE(kept.has_value(), "the chart in " << tag << " must fail as a kept failure");
    store.save(*kept);
    out.lookup = store.lookup_one(out.entry.md5, run.profile);
    REQUIRE(out.lookup.status == RecordStatus::CantAnalyze);
    REQUIRE(out.lookup.failure_kind.has_value());
    return out;
}

// A chart with notes only on guitar, so it has no drum notes to analyze.
std::string guitar_only_chart() {
    return testchart::chart_text(testchart::section("ExpertSingle", testchart::line(0, "N 0 0")));
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
    // One chart in two song folders under two names (the same notes.chart in
    // both), plus another chart. The scan reads them and the store keeps what
    // it read; the rows and the click's read must give back the scan's items.
    const std::vector<hydra::app::ScanItem> scanned = scan_song_folders(
        "lm_copies", {{"one", "name = First Name\nartist = A\ncharter = C\nsong_length = 180000\n"
                              "delay = 50\n", 0},
                      {"two", "name = Second Name\nartist = B\ncharter = D\n", 0},
                      {"other", "name = Other\nartist = E\ncharter = F\n"}});
    const hydra::app::ScanItem& one = item_in(scanned, "one");
    const hydra::app::ScanItem& two = item_in(scanned, "two");
    REQUIRE(two.md5 == one.md5);
    REQUIRE(item_in(scanned, "other").md5 != one.md5);
    // The one folder's song.ini states a length and delay and the other's does
    // not, so the two copies' timings differ as their files do.
    REQUIRE_FALSE(one.timing == two.timing);
    hydra::store::RecordStore store(":memory:");
    REQUIRE_FALSE(hydra::app::save_scan_as_library(store, scanned).has_value());

    LibraryModel m;
    m.set_charts(store.list_chart_library(0, -1));
    REQUIRE(m.rows().size() == scanned.size());
    for (const hydra::ui::LibraryRow& row : m.rows()) {
        const auto item = std::find_if(
            scanned.begin(), scanned.end(),
            [&](const hydra::app::ScanItem& i) { return i.notespath == row.entry.notespath; });
        REQUIRE(item != scanned.end());
        CHECK(row.entry.md5 == item->md5);
        CHECK(row.entry.rootfolder == item->rootfolder);
        CHECK(row.title == item->title);
    }

    // The click's read: every copy of the one chart, as the scan read it.
    const std::vector<ChartLibraryEntry> copies = store.list_chart_library_copies(one.md5);
    REQUIRE(copies.size() == 2);
    for (const ChartLibraryEntry& copy : copies) {
        const hydra::app::ScanItem& scan = in_folder(copy.notespath, "one") ? one : two;
        CHECK(copy.notespath == scan.notespath);
        CHECK(copy.md5 == one.md5);
        CHECK(copy.title == scan.title);
        CHECK(copy.artist == scan.artist);
        CHECK(copy.charter == scan.charter);
        CHECK(copy.rootfolder == scan.rootfolder);
        CHECK(copy.sig == scan.sig);
        CHECK(copy.timing == scan.timing);
    }
    CHECK(store.list_chart_library_copies(item_in(scanned, "other").md5).size() == 1);
}

TEST_CASE("library model: a title made only of tags reads (unknown)") {
    const LibraryModel m = scanned_library(
        "lm_tag_title",
        {{"tags", std::string("name = ") + hydra::test::kTagOnlyTitle + "\nartist = Artist\n"}});
    CHECK(row_in(m, "tags").title == hydra::kUnknownTitle);
}

TEST_CASE("library model: an artist made only of tags reads (unknown), and search keeps the stored text") {
    // D50 item 5: the artist column follows the title's rule. The search still
    // reads the stored text, so the shown word is not a search hit.
    LibraryModel m = scanned_library(
        "lm_tag_artist",
        {{"tags", std::string("name = Song\nartist = ") + hydra::test::kTagOnlyTitle +
                      "\ncharter = Charter\n"}});
    const hydra::ui::LibraryRow& row = row_in(m, "tags");
    CHECK(row.artist == hydra::kUnknownTitle);
    CHECK(row.searchable.artist.find(unknown_word()) == std::string::npos);
    m.set_query(unknown_word());
    CHECK(m.order().empty());
}

TEST_CASE("library model: a missing charter sorts and searches as a missing artist does (D266 item 2)") {
    // Sort reads the shown text, so a placeholder or tags-only charter sorts
    // under kUnknownTitle. Search reads the stored text: the placeholder the
    // scan stores for a song.ini with no charter (kUnknownCharter) is a hit for
    // charter:unknown, as kUnknownArtist is for artist:unknown, and a
    // tags-only charter is not, as a tags-only artist is not.
    const std::string tags = hydra::test::kTagOnlyTitle;
    LibraryModel m = scanned_library(
        "lm_missing_charter",
        {{"placeholder", "name = Song P\n"},
         {"tags", "name = Song T\nartist = " + tags + "\ncharter = " + tags + "\n"},
         {"named", "name = Song N\nartist = Cats\ncharter = Bob\n"},
         {"late", "name = Song L\nartist = Dogs\ncharter = Zed\n"}});
    m.set_sort(LibrarySort::Charter, true);
    CHECK(titles(m) == std::vector<std::string>{"Song P", "Song T", "Song N", "Song L"});
    m.set_sort(LibrarySort::Artist, true);
    CHECK(titles(m) == std::vector<std::string>{"Song P", "Song T", "Song N", "Song L"});

    m.set_query("charter:" + unknown_word());
    CHECK(titles(m) == std::vector<std::string>{"Song P"});
    m.set_query("artist:" + unknown_word());
    CHECK(titles(m) == std::vector<std::string>{"Song P"});
}

TEST_CASE("library model: an album draws without its tags, a blank one draws empty, and search folds it") {
    // display_album's text, with no placeholder for a blank album. The spaces
    // inside the tags reach the stored text, since song.ini's reader trims
    // only the value's ends.
    const std::string dookie = "<b> Dookie </b>";
    LibraryModel m = scanned_library(
        "lm_album",
        {{"a", "name = Song A\nartist = Band\ncharter = C\nalbum = " + dookie + "\n"},
         {"b", "name = Song B\nartist = Band\ncharter = C\nalbum =\n"},
         {"c", std::string("name = Song C\nartist = Band\ncharter = C\nalbum = ") +
                   hydra::test::kTagOnlyTitle + "\n"}});
    const hydra::ui::LibraryRow& a = row_in(m, "a");
    CHECK(a.album == "Dookie");
    CHECK(hydra::ui::library_cell_text(a, hydra::ui::kColumnAlbum) == "Dookie");
    // Search reads the stored text with its tags stripped, then folded.
    CHECK(a.searchable.album == hydra::app::fold_for_search(hydra::app::strip_rich_tags(dookie)));
    CHECK(row_in(m, "b").album.empty());
    CHECK(row_in(m, "b").searchable.album.empty());
    CHECK(row_in(m, "c").album.empty());
    m.set_query("album:dookie");
    CHECK(titles(m) == std::vector<std::string>{"Song A"});
    m.set_query(unknown_word());
    CHECK(m.order().empty());
}

TEST_CASE("library model: each column's position and its user ID are one mapping") {
    // Every column has a user ID of its own, and the ID leads back to that
    // column, so a sort spec the table reports maps to the column clicked. The
    // values themselves are LibrarySort's to keep.
    using hydra::ui::column_of;
    using hydra::ui::sort_of;
    std::vector<LibrarySort> seen;
    for (int c = 0; c < hydra::ui::kLibraryColumnCount; ++c) {
        INFO("column " << c);
        const LibrarySort sort = sort_of(c);
        CHECK(std::find(seen.begin(), seen.end(), sort) == seen.end());
        seen.push_back(sort);
        CHECK(column_of(sort) == c);
    }
}

TEST_CASE("library model: Album sorts on the album its cell shows, blanks first") {
    LibraryModel m = scanned_library(
        "lm_album_sort",
        {{"burnout", "name = Burnout\nartist = Green Day\nalbum = Dookie\n"},
         {"yyz", "name = YYZ\nartist = Rush\n"},
         {"chair", "name = Chair\nartist = Sufferer\nalbum = Sufferer\n"},
         {"acid", "name = Acid Romance\nartist = Some Band\nalbum = A Quiet Place To Die\n"},
         {"halo", "name = Halo\nartist = Someone\n"},
         {"other", "name = Other\nartist = Thrice\nalbum = The Illusion of Safety\n"}});
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
    // kUnknownTitle. A present charter reads display_charter's text: tags gone,
    // the spaces at its ends trimmed (here the spaces inside the tags, which
    // the scan's own trim leaves). Song A's artist line is blank; Song B's
    // song.ini holds the scan's placeholders, as an older scan stored them.
    const LibraryModel m = scanned_library(
        "lm_missing_names",
        {{"a", "name = Song A\nartist =\ncharter = <b> Bob </b>\n"},
         {"b", std::string("name = Song B\nartist = ") + hydra::kUnknownArtist +
                   "\ncharter = " + hydra::kUnknownCharter + "\n"}});
    CHECK(row_in(m, "a").artist == hydra::kUnknownTitle);
    CHECK(row_in(m, "a").charter == "Bob");
    CHECK(row_in(m, "b").artist == hydra::kUnknownTitle);
    CHECK(row_in(m, "b").charter == hydra::kUnknownTitle);
}

TEST_CASE("library model: a name sorts on the text its cell shows, not the stored text") {
    // D114 item 3. A blank title and a blank artist show kUnknownTitle and
    // sort there; a leading space the cell trims does not move the row. The
    // space sits after an empty tag, since the scan trims a name's own ends.
    LibraryModel m = scanned_library(
        "lm_name_sort", {{"blank", "name =\nartist = 1 Band\ncharter = C\n"},
                         {"bang", "name = !!! Song\nartist =\ncharter = C\n"},
                         {"space", "name = <b></b> Zebra\nartist = Band\ncharter = C\n"},
                         {"apple", "name = Apple\nartist = Cats\ncharter = C\n"}});
    const std::string unknown = hydra::kUnknownTitle;
    CHECK(titles(m) == std::vector<std::string>{"!!! Song", unknown, "Apple", "Zebra"});
    m.set_sort(LibrarySort::Artist, true);
    CHECK(titles(m) == std::vector<std::string>{"!!! Song", unknown, "Zebra", "Apple"});
}

TEST_CASE("library model: a lookup that differs only in a field the library never reads changes nothing") {
    // One corpus chart's own result, saved and read back through the store as
    // the app reads it. Each variant below changes that real lookup in one
    // field.
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    const corpus::ChartWithPaths c = corpus::first_chart_with_paths(run.settings);
    const ChartLibraryEntry entry = corpus::scanned_entry(c.chart);
    hydra::store::RecordStore store(":memory:");
    store.save(hydra::store::prepare_record(entry.md5, run.profile, c.result.record));
    const RecordLookup found = store.lookup_one(entry.md5, run.profile);
    REQUIRE(found.status == RecordStatus::Ready);
    REQUIRE(found.record.stars.has_value());

    LibraryModel m;
    m.set_charts({entry});
    REQUIRE(m.set_summary_for(entry.md5, found) == 1);
    const hydra::ui::LibraryRow& row = m.rows()[0];

    // The best path's activation count is in the record, but no column, chip
    // or filter reads it.
    RecordLookup same = found;
    same.record.paths.front().actcount += 1;
    CHECK(row.shows_same_as(same));
    CHECK(m.set_summary_for(entry.md5, same) == 0);

    // The stars are shown, so a change there is a change.
    RecordLookup more_stars = found;
    more_stars.record.stars = *found.record.stars + 1;
    CHECK_FALSE(row.shows_same_as(more_stars));

    // The same result saved as another Hydra version's reads Stale.
    hydra::store::RecordStore old_store(":memory:");
    hydra::test::save_old_build_record(
        old_store, hydra::store::prepare_record(entry.md5, run.profile, c.result.record));
    const RecordLookup stale_found = old_store.lookup_one(entry.md5, run.profile);
    REQUIRE(stale_found.status == RecordStatus::Stale);
    CHECK_FALSE(row.shows_same_as(stale_found));
}

TEST_CASE("library model: status_label is the one source of the status words") {
    using hydra::ui::status_label;
    const RecordStatus statuses[] = {RecordStatus::Ready, RecordStatus::Stale,
                                     RecordStatus::NotAnalyzed, RecordStatus::CantAnalyze};
    const hydra::store::PathSummary none;
    std::vector<std::string> words;
    for (RecordStatus s : statuses) {
        const std::string word = status_label(s);
        INFO("status word " << word);
        // Each status has a word, and no two share one.
        CHECK_FALSE(word.empty());
        CHECK(std::find(words.begin(), words.end(), word) == words.end());
        words.push_back(word);
        // The Best path cell of a row with no current result shows that word.
        if (s != RecordStatus::Ready) CHECK(hydra::ui::best_path_label(s, "", none) == word);
    }
    // Each status's chip is named with the same word.
    for (StatusChip chip : {StatusChip::NotAnalyzed, StatusChip::Stale, StatusChip::Analyzed,
                            StatusChip::CantAnalyze}) {
        const std::optional<RecordStatus> s = hydra::ui::status_of(chip);
        REQUIRE(s.has_value());
        CHECK(hydra::ui::chip_label(chip, 1).find(status_label(*s)) == 0);
    }
}

// One chart under each status, each read back through the store the way the
// app reads it (lookup): a corpus chart's own result (Ready), another's saved
// as another Hydra version's (Stale), a third with nothing saved (Not
// analyzed), and a written chart with no drum notes whose real analysis fails
// and whose failure the batch's own step keeps (prepare_failed_chart).
TEST_CASE("library model: every status has its own chip, and All counts them all") {
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    const std::vector<corpus::ChartWithPaths> charts = corpus::charts_with_paths(run.settings, 3);
    REQUIRE_MESSAGE(charts.size() == 3, "the corpus needs three charts with paths");
    const std::string dir = testtemp::temp_dir("lm_cant");
    const std::string cant_path = dir + "\\notes.chart";
    audiochart::write_text_file(dir + "\\song.ini", "[song]\nname = Cant\n");
    audiochart::write_text_file(
        cant_path,
        testchart::chart_text(testchart::section("ExpertSingle", testchart::line(0, "N 0 0"))));

    std::vector<ChartLibraryEntry> entries;
    for (const corpus::ChartWithPaths& c : charts) entries.push_back(corpus::scanned_entry(c.chart));
    entries.push_back(corpus::scanned_entry(cant_path, "Cant"));
    const std::string cant_md5 = entries[3].md5;

    hydra::store::RecordStore store(":memory:");
    store.save(hydra::store::prepare_record(entries[0].md5, run.profile, charts[0].result.record));
    hydra::test::save_old_build_record(
        store, hydra::store::prepare_record(entries[1].md5, run.profile, charts[1].result.record));
    std::optional<hydra::store::PreparedRecord> failure;
    try {
        hydra::app::analyze_chart_file(cant_path, run.settings);
    } catch (const std::exception& e) {
        failure = hydra::app::prepare_failed_chart(cant_md5, run.profile, e);
    }
    REQUIRE_MESSAGE(failure.has_value(), "a chart with no drum notes must fail as a kept failure");
    store.save(*failure);

    LibraryModel m;
    m.set_charts(entries);
    m.set_summaries(store.lookup(m.hashes(), run.profile));

    const hydra::ui::ChipCounts& c = m.counts();
    CHECK(c.analyzed == 1);
    CHECK(c.stale == 1);
    CHECK(c.not_analyzed == 1);
    CHECK(c.cant_analyze == 1);
    CHECK(c.all == entries.size());
    CHECK(c.of(StatusChip::CantAnalyze) == 1);

    // The chip narrows the table to that one chart, which is what the
    // Analyze button counts and runs (D115).
    m.set_chip(StatusChip::CantAnalyze);
    REQUIRE(m.order().size() == 1);
    CHECK(m.rows()[m.order()[0]].entry.md5 == cant_md5);
    CHECK(m.narrowed());

    // The row keeps the stored kind and text, and its cell reads the word.
    const hydra::ui::LibraryRow& row = m.rows()[3];
    CHECK(row.failure_kind == failure->record.failure_kind);
    CHECK(row.failure_text == failure->record.failure_text);
    CHECK(hydra::ui::library_cell_text(row, hydra::ui::kColumnBestPath) ==
          hydra::ui::status_label(RecordStatus::CantAnalyze));

    // Re-analyzed after the file was fixed (two drum notes written in its
    // place): the chip empties and falls back to All.
    audiochart::write_text_file(
        cant_path, testchart::chart_text(testchart::section(
                       "ExpertDrums", testchart::line(0, "N 0 0") + testchart::line(192, "N 1 0"))));
    const hydra::app::AnalysisResult fixed = hydra::app::analyze_chart_file(cant_path, run.settings);
    store.save(hydra::store::prepare_record(cant_md5, run.profile, fixed.record));
    CHECK(m.set_summary_for(cant_md5, store.lookup_one(cant_md5, run.profile)) == 1);
    CHECK(m.chip() == StatusChip::All);
    CHECK(m.counts().cant_analyze == 0);
    CHECK_FALSE(m.rows()[3].failure_kind.has_value());
}

// The hover of a Can't analyze cell words the stored failure as a live one of
// the same kind reads: a no-notes chart's sentence quotes its raw text, so
// there is no details line under it.
TEST_CASE("library model: a stored failure's hover reads as the live failure does") {
    // Two written charts whose real analysis fails, kept and read back through
    // the store into their rows: one with no drum notes, one whose .chart text
    // has no [section] header. The hover is worded from the row's kind and
    // text (stored_failure_text), as library_table.cpp words it.
    hydra::store::RecordStore store(":memory:");
    const FailedChart no_notes =
        kept_failure(store, "lm_hover_none", "notes.chart", guitar_only_chart());
    const FailedChart unreadable =
        kept_failure(store, "lm_hover_bad", "notes.chart", "this is not a chart\n");
    LibraryModel m;
    m.set_charts({no_notes.entry, unreadable.entry});
    REQUIRE(m.set_summary_for(no_notes.entry.md5, no_notes.lookup) == 1);
    REQUIRE(m.set_summary_for(unreadable.entry.md5, unreadable.lookup) == 1);
    // The rows keep the order set_charts was given.
    const hydra::ui::LibraryRow& none_row = m.rows()[0];
    const hydra::ui::LibraryRow& bad_row = m.rows()[1];
    REQUIRE(none_row.entry.md5 == no_notes.entry.md5);
    REQUIRE(bad_row.entry.md5 == unreadable.entry.md5);

    // A no-notes chart's sentence quotes its raw text, so there is no details
    // line under it.
    REQUIRE(none_row.failure_kind == hydra::ErrorKind::NoNotes);
    const std::string hover =
        hydra::app::stored_failure_text(*none_row.failure_kind, none_row.failure_text);
    CHECK(hover == no_notes.live_block);
    CHECK(no_notes.live_detail.empty());
    CHECK(hover == no_notes.live_sentence);

    // A kind whose sentence does not quote the raw text gets the details line
    // under it.
    REQUIRE(bad_row.failure_kind.has_value());
    const std::string block =
        hydra::app::stored_failure_text(*bad_row.failure_kind, bad_row.failure_text);
    CHECK(block == unreadable.live_block);
    CHECK_FALSE(unreadable.live_detail.empty());
    CHECK(block.find(unreadable.live_sentence) == 0);
    CHECK(block.find(unreadable.live_what) != std::string::npos);
}

TEST_CASE("library model: a new failure text or kind changes the row") {
    // Three written charts whose real analysis fails, each kept and read back
    // through the store: a .chart and a .mid that are unreadable for their
    // own reasons, and a chart with no drum notes. Each variant below changes
    // the first chart's real answer in one field, to another chart's value.
    hydra::store::RecordStore store(":memory:");
    const FailedChart bad_chart =
        kept_failure(store, "lm_fail_chart", "notes.chart", "this is not a chart\n");
    const FailedChart bad_mid =
        kept_failure(store, "lm_fail_mid", "notes.mid", "this is not a MIDI file\n");
    const FailedChart no_notes =
        kept_failure(store, "lm_fail_none", "notes.chart", guitar_only_chart());
    REQUIRE(bad_mid.lookup.failure_kind == bad_chart.lookup.failure_kind);
    REQUIRE(bad_mid.lookup.failure_text != bad_chart.lookup.failure_text);
    REQUIRE(no_notes.lookup.failure_kind != bad_chart.lookup.failure_kind);

    LibraryModel m;
    m.set_charts({bad_chart.entry});
    REQUIRE(m.set_summary_for(bad_chart.entry.md5, bad_chart.lookup) == 1);
    CHECK(m.rows()[0].shows_same_as(bad_chart.lookup));
    RecordLookup other_text = bad_chart.lookup;
    other_text.failure_text = bad_mid.lookup.failure_text;
    CHECK_FALSE(m.rows()[0].shows_same_as(other_text));
    RecordLookup other_kind = bad_chart.lookup;
    other_kind.failure_kind = no_notes.lookup.failure_kind;
    CHECK_FALSE(m.rows()[0].shows_same_as(other_kind));
}

TEST_CASE("library model: the search narrows the rows and the chip counts follow it") {
    LibraryModel m = scanned_sample().model;
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
    CHECK(m.counts().all == 0);
    m.set_query("dookie");  // a plain word matches an album too
    CHECK(titles(m) == std::vector<std::string>{"Burnout"});
    // stars: reads the saved stars of the one Ready row; Chair's Stale result
    // is not current, so it has none to match.
    const std::optional<int> stars = row_in(m, "burnout").summary.stars;
    REQUIRE(stars.has_value());
    m.set_query("stars:" + std::to_string(*stars));
    CHECK(titles(m) == std::vector<std::string>{"Burnout"});
    m.set_query("stars:" + std::to_string(hydra::kMaxStars + 1));
    CHECK_FALSE(m.query().errors.empty());
    m.set_query("");
    CHECK(m.order().size() == 6);
}

TEST_CASE("library model: a status chip narrows the rows, and an emptied chip falls back to All") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    m.set_chip(StatusChip::NotAnalyzed);
    CHECK(titles(m) == std::vector<std::string>{"Acid Romance", "Halo", "Other", "YYZ"});
    CHECK(m.counts().all == 6);  // counts ignore the chip
    m.set_chip(StatusChip::Stale);
    CHECK(titles(m) == std::vector<std::string>{"Chair"});
    // What the Analyze button runs: the rows on screen, the chip applied (D115).
    CHECK(m.order().size() == 1);

    // Chair is analysed again: the Stale group is empty, so the table goes
    // back to All instead of sitting empty.
    CHECK(m.set_summary_for(lib.chair, lib.chair_ready) == 1);
    CHECK(m.chip() == StatusChip::All);
    CHECK(m.order().size() == 6);
    CHECK(m.counts().analyzed == 2);
}

TEST_CASE("library model: the search or a chip narrows the table, and nothing else does") {
    LibraryModel m = scanned_sample().model;
    CHECK_FALSE(m.narrowed());                    // neither
    m.set_query("\"tier 4\"");
    CHECK(m.narrowed());                          // the search alone
    m.set_chip(StatusChip::NotAnalyzed);
    CHECK(m.narrowed());                          // both
    m.set_query("");
    CHECK(m.narrowed());                          // the chip alone
    CHECK(m.order().size() == 4);
    m.set_chip(StatusChip::All);
    m.set_query("stars:" + std::to_string(hydra::kMaxStars + 1));  // a filter that does not parse
    CHECK_FALSE(m.query().errors.empty());
    CHECK_FALSE(m.narrowed());
    CHECK(m.order().size() == 6);
}

TEST_CASE("library model: Best path sorts by score, with unscored rows last both ways") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    m.set_summary_for(lib.yyz, lib.yyz_ready);
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

// The rows of the scanned sample under Best path, highest first, once YYZ has
// its result and its row is still holding back from the sort.
namespace {

const std::vector<std::string> kHeldOrder{"Burnout", "Chair", "Acid Romance", "Halo", "Other",
                                          "YYZ"};
const std::vector<std::string> kScoreOrder{"YYZ", "Burnout", "Chair", "Acid Romance", "Halo",
                                           "Other"};

LibraryModel sample_with_held_yyz() {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    m.set_sort(LibrarySort::BestPath, false);
    REQUIRE(m.set_summary_for(lib.yyz, lib.yyz_ready) == 1);
    return m;
}

}  // namespace

// The user's words: "Yes, hold them still". New scores fill in where each row
// already sits, and the list re-sorts when it is asked to.
TEST_CASE("library model: a new score fills in where the row sits, and one call sorts it") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = sample_with_held_yyz();
    CHECK(titles(m) == kHeldOrder);
    // The held row already shows YYZ's saved result.
    const hydra::ui::LibraryRow& yyz = m.rows()[m.order().back()];
    CHECK(yyz.entry.md5 == lib.yyz);
    CHECK(yyz.summary == lib.yyz_ready.record.summary());
    CHECK(yyz.bestpath == lib.yyz_ready.record.best_path_text());
    CHECK(m.counts().analyzed == 2);

    CHECK(m.sort_if_out_of_date());
    CHECK(titles(m) == kScoreOrder);
    CHECK_FALSE(m.sort_if_out_of_date());  // nothing is out of date now
}

TEST_CASE("library model: a whole re-read of the summaries keeps the rows in place too") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    m.set_sort(LibrarySort::BestPath, false);
    REQUIRE(m.set_summaries(lib.with_yyz) == 1);
    CHECK(titles(m) == kHeldOrder);
    CHECK(m.sort_if_out_of_date());
    CHECK(titles(m) == kScoreOrder);
}

TEST_CASE("library model: answers that change nothing leave the order up to date") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    m.set_sort(LibrarySort::BestPath, false);
    REQUIRE(m.set_summary_for(lib.burnout, lib.burnout_ready) == 0);
    CHECK_FALSE(m.sort_if_out_of_date());
}

// Only the Best path sort reads a row's summary, so no other sort has an order
// a new summary can put out of date.
TEST_CASE("library model: a new score under another sort leaves nothing to re-sort") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    const std::vector<std::string> by_title = titles(m);
    REQUIRE(m.set_summary_for(lib.yyz, lib.yyz_ready) == 1);
    CHECK(titles(m) == by_title);
    CHECK_FALSE(m.sort_if_out_of_date());
}

TEST_CASE("library model: a held row still leaves a chip it no longer belongs to") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    m.set_sort(LibrarySort::BestPath, false);
    m.set_chip(StatusChip::NotAnalyzed);
    REQUIRE(titles(m) == std::vector<std::string>{"Acid Romance", "Halo", "Other", "YYZ"});
    REQUIRE(m.set_summary_for(lib.yyz, lib.yyz_ready) == 1);
    CHECK(titles(m) == std::vector<std::string>{"Acid Romance", "Halo", "Other"});
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
    m.set_charts(one_scanned_chart("lm_new_rows"));
    CHECK_FALSE(m.sort_if_out_of_date());
}

// A chart that can't be analyzed sorts after Not analyzed in both directions:
// it is the one row that never gains a score (unscored_rank).
TEST_CASE("library model: Best path sorts a Can't analyze row after Not analyzed both ways") {
    // The scanned sample's rows with their store answers, and one more row: a
    // written chart with no drum notes, whose real failure is kept and read
    // back through the store. Its title, lm_sort_cant, sorts between Halo and
    // Other by name, so only its status puts it last.
    const ScannedSample& lib = scanned_sample();
    hydra::store::RecordStore store(":memory:");
    const FailedChart cant = kept_failure(store, "lm_sort_cant", "notes.chart", guitar_only_chart());
    std::vector<ChartLibraryEntry> entries = lib.entries;
    entries.push_back(cant.entry);
    std::vector<RecordLookup> answers = lib.answers;
    answers.push_back(cant.lookup);
    LibraryModel m;
    m.set_charts(entries);
    m.set_summaries(answers);
    // set_charts keeps its rows in the order it was given them.
    REQUIRE(m.rows().back().entry.md5 == cant.entry.md5);
    REQUIRE(m.rows().back().status == RecordStatus::CantAnalyze);
    const std::string cant_title = m.rows().back().title;
    m.set_sort(LibrarySort::BestPath, false);
    CHECK(titles(m) == std::vector<std::string>{"Burnout", "Chair", "Acid Romance", "Halo", "Other",
                                                "YYZ", cant_title});
    m.set_sort(LibrarySort::BestPath, true);
    CHECK(titles(m) == std::vector<std::string>{"Burnout", "Chair", "Acid Romance", "Halo", "Other",
                                                "YYZ", cant_title});
}

// A result whose analysis kept no path is Analyzed, but it has no best path,
// so a stars: or squeeze filter has nothing to test on it (D51 call 11). The
// empty result is the one a batch saves for a chart (save_batch_record), read
// back through the store; the scored result is a corpus chart's own.
TEST_CASE("library model: a Ready row with no paths is Analyzed but has no facts") {
    const CopiedChart& lib = copied_chart();
    LibraryModel m = lib.model;
    const hydra::store::Profile profile = hydra::app::Settings{}.batch_run().profile;
    const std::string empty_md5 = row_in(m, "first").entry.md5;
    hydra::store::RecordStore store(":memory:");
    hydra::test::save_batch_record(store, empty_md5, profile);
    const RecordLookup no_paths = store.lookup_one(empty_md5, profile);
    REQUIRE(no_paths.status == RecordStatus::Ready);
    REQUIRE_FALSE(no_paths.record.summary().has_scored_best_path());
    const std::optional<int> stars = lib.ready.record.summary().stars;
    REQUIRE(stars.has_value());
    REQUIRE(m.set_summary_for(lib.md5, lib.ready) == 2);
    REQUIRE(m.set_summary_for(empty_md5, no_paths) == 1);
    const std::vector<std::string> scored{"Song", "Song"};
    m.set_query("stars:" + std::to_string(*stars));
    CHECK(titles(m) == scored);
    m.set_query("squeeze<=100000");
    CHECK(titles(m) == scored);
    m.set_query("");
    CHECK(m.counts().analyzed == 3);
}

// The saved figure is the engine's, sign included; the squeeze filters read it
// through shown_timing (facts_of), so a figure below 0 needs no timing. The two
// results are the first corpus charts whose best path's engine figure
// (engine_hardest_ms) is below 0 and exactly 0.0, saved and read back through
// the store.
TEST_CASE("library model: the squeeze filters read the saved figure as shown") {
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    std::string inside_chart, on_end_chart;
    const hydra::HydraRecord* inside = nullptr;
    const hydra::HydraRecord* on_end = nullptr;
    double inside_figure = 0.0;
    for (const std::string& path : corpus::chart_paths()) {
        if (inside && on_end) break;
        try {
            const hydra::HydraRecord& rec = corpus::analyzed(path, run.settings);
            if (rec.paths.empty()) continue;
            const std::optional<double> figure = hydra::engine_hardest_ms(rec.best_path());
            if (!figure) continue;
            if (!inside && *figure < 0.0) {
                inside = &rec;
                inside_chart = path;
                inside_figure = *figure;
            } else if (!on_end && *figure == 0.0) {
                on_end = &rec;
                on_end_chart = path;
            }
        } catch (const std::exception&) {
            continue;
        }
    }
    REQUIRE_MESSAGE(inside, "no corpus chart's best path has a figure below 0");
    REQUIRE_MESSAGE(on_end, "no corpus chart's best path has a figure of exactly 0.0");
    const ChartLibraryEntry inside_entry = corpus::scanned_entry(inside_chart, "Inside");
    const ChartLibraryEntry on_end_entry = corpus::scanned_entry(on_end_chart, "On End");
    REQUIRE(inside_entry.md5 != on_end_entry.md5);
    hydra::store::RecordStore store(":memory:");
    store.save(hydra::store::prepare_record(inside_entry.md5, run.profile, *inside));
    store.save(hydra::store::prepare_record(on_end_entry.md5, run.profile, *on_end));
    const RecordLookup inside_answer = store.lookup_one(inside_entry.md5, run.profile);
    const RecordLookup on_end_answer = store.lookup_one(on_end_entry.md5, run.profile);
    REQUIRE(inside_answer.record.summary().hardest_ms == std::optional<double>(inside_figure));
    REQUIRE(on_end_answer.record.summary().hardest_ms == std::optional<double>(0.0));
    LibraryModel m;
    m.set_charts(std::vector<ChartLibraryEntry>{inside_entry, on_end_entry});
    REQUIRE(m.set_summary_for(inside_entry.md5, inside_answer) == 1);
    REQUIRE(m.set_summary_for(on_end_entry.md5, on_end_answer) == 1);
    // A figure below 0 needs no timing, which reaches no floor; the raw figure
    // would match a floor just under it.
    m.set_query("squeeze>" + std::to_string(static_cast<long long>(std::floor(inside_figure)) - 1));
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
    const CopiedChart& lib = copied_chart();
    LibraryModel m = lib.model;
    CHECK(m.counts().not_analyzed == 4);
    CHECK(m.set_summary_for(lib.md5, lib.ready) == 2);
    CHECK(m.counts().analyzed == 2);
    // Asking again with the same answer changes nothing.
    CHECK(m.set_summary_for(lib.md5, lib.ready) == 0);
}

namespace {

using Answers = std::vector<std::pair<std::string, RecordLookup>>;

}  // namespace

TEST_CASE("library model: set_summaries_for gives every row the answer it was given") {
    // The scanned sample's six charts and a chart whose analysis fails, all
    // listed with no answer yet. Then every row's own store answer arrives at
    // once: Burnout's and YYZ's results (Analyzed), Chair's result saved as
    // another Hydra version's (Stale), the failure the batch kept (Can't
    // analyze) and nothing saved for the other three (Not analyzed).
    const ScannedSample& lib = scanned_sample();
    hydra::store::RecordStore store(":memory:");
    const FailedChart failed =
        kept_failure(store, "lm_bulk_failure", "notes.chart", guitar_only_chart());
    std::vector<ChartLibraryEntry> entries = lib.entries;
    entries.push_back(failed.entry);
    REQUIRE(lib.with_yyz.size() == lib.model.rows().size());
    Answers answers;
    for (size_t i = 0; i < lib.model.rows().size(); ++i)
        answers.emplace_back(lib.model.rows()[i].entry.md5, lib.with_yyz[i]);
    answers.emplace_back(failed.entry.md5, failed.lookup);

    // Each status the answers hold, counted from the answers themselves.
    size_t n_ready = 0, n_stale = 0, n_cant = 0, n_none = 0;
    for (const auto& answer : answers) {
        switch (answer.second.status) {
            case RecordStatus::Ready: ++n_ready; break;
            case RecordStatus::Stale: ++n_stale; break;
            case RecordStatus::CantAnalyze: ++n_cant; break;
            case RecordStatus::NotAnalyzed: ++n_none; break;
        }
    }
    // Every status is in the batch, so a status the bulk path drops shows.
    REQUIRE(n_ready > 0);
    REQUIRE(n_stale > 0);
    REQUIRE(n_cant > 0);
    REQUIRE(n_none > 0);

    LibraryModel m;
    m.set_charts(entries);
    REQUIRE(m.rows().size() == answers.size());
    REQUIRE(m.counts().not_analyzed == answers.size());
    // Every row changes but those whose answer is Not analyzed again, which
    // show nothing new: the Analyzed, Stale and Can't analyze answers.
    const size_t n_changed = n_ready + n_stale + n_cant;
    CHECK(m.set_summaries_for(answers) == n_changed);
    for (const hydra::ui::LibraryRow& row : m.rows()) {
        INFO("row " << row.title);
        size_t given = 0;
        for (const auto& answer : answers) {
            if (answer.first != row.entry.md5) continue;
            CHECK(row.shows_same_as(answer.second));
            ++given;
        }
        CHECK(given == 1);
    }
    CHECK(m.counts().all == answers.size());
    CHECK(m.counts().analyzed == n_ready);
    CHECK(m.counts().stale == n_stale);
    CHECK(m.counts().cant_analyze == n_cant);
    CHECK(m.counts().not_analyzed == n_none);
}

TEST_CASE("library model: set_summaries_for updates both rows of a chart in two folders") {
    const CopiedChart& lib = copied_chart();
    LibraryModel m = lib.model;
    const Answers answers{{lib.md5, lib.ready}};
    CHECK(m.set_summaries_for(answers) == 2);
    size_t shown = 0;
    for (const hydra::ui::LibraryRow& row : m.rows()) {
        if (row.entry.md5 != lib.md5) continue;
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
    // The scanned sample, then YYZ's saved result and Chair's fresh one, each
    // as the store gave it, in one batch.
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    m.set_sort(LibrarySort::BestPath, false);
    const std::vector<std::string> before = titles(m);
    const size_t analyzed_before = m.counts().analyzed;
    REQUIRE(m.set_summaries_for({{lib.yyz, lib.yyz_ready}, {lib.chair, lib.chair_ready}}) == 2);
    CHECK(titles(m) == before);
    CHECK(m.counts().analyzed == analyzed_before + 2);

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
        REQUIRE(row.summary.has_scored_best_path());
        if (last_score) CHECK(*row.summary.score <= *last_score);
        last_score = row.summary.score;
    }
    CHECK(unscored_after == unscored_before);
    CHECK_FALSE(m.sort_if_out_of_date());
}

TEST_CASE("library model: set_summaries_for with answers that change nothing rebuilds nothing") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    m.set_sort(LibrarySort::BestPath, false);
    const hydra::ui::ChipCounts before = m.counts();
    // Each answer is the one the store already gave its row: Burnout's Ready
    // result, Chair's Stale one and the Not analyzed rest.
    REQUIRE(lib.answers.size() == m.rows().size());
    Answers same;
    for (size_t i = 0; i < m.rows().size(); ++i)
        same.emplace_back(m.rows()[i].entry.md5, lib.answers[i]);
    CHECK(m.set_summaries_for(same) == 0);
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
    // The names are the song.ini lines the scanned sample wrote, and the
    // folder is where it wrote Burnout's song folder, under the scan's root.
    const ScannedSample& lib = scanned_sample();
    const hydra::ui::LibraryRow& burnout = row_in(lib.model, "burnout");
    CHECK(library_cell_text(burnout, hydra::ui::kColumnTitle) == "Burnout");
    CHECK(library_cell_text(burnout, hydra::ui::kColumnArtist) == "Green Day");
    CHECK(library_cell_text(burnout, hydra::ui::kColumnAlbum) == "Dookie");
    CHECK(library_cell_text(burnout, hydra::ui::kColumnCharter) == "Hoph2o");
    CHECK(library_cell_text(burnout, hydra::ui::kColumnFolder) == "Tier 4");
    // The Best path cell is best_path_label's, worded from the store's answer.
    REQUIRE(lib.burnout_ready.record.summary().has_scored_best_path());
    CHECK(library_cell_text(burnout, hydra::ui::kColumnBestPath) ==
          hydra::ui::best_path_label(RecordStatus::Ready, lib.burnout_ready.record.best_path_text(),
                                     lib.burnout_ready.record.summary()));
    // The charter's song.ini line carries color tags; the cell shows them stripped.
    CHECK(library_cell_text(row_in(lib.model, "acid"), hydra::ui::kColumnCharter) == "Bloodline");
    CHECK(library_cell_text(row_in(lib.model, "other"), hydra::ui::kColumnBestPath) ==
          hydra::ui::status_label(RecordStatus::NotAnalyzed));
}

TEST_CASE("library model: only the Best path cell reads the row's summary") {
    // The table re-measures just the Best path column when a summary changes
    // (update_column_widths), so no other cell's text may depend on it.
    using hydra::ui::library_cell_text;
    // Chair's Stale row takes its answer from the store once it is analysed
    // again.
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    const auto found = std::find_if(m.rows().begin(), m.rows().end(),
                                    [&](const hydra::ui::LibraryRow& r) { return r.entry.md5 == lib.chair; });
    REQUIRE(found != m.rows().end());
    const size_t chair = static_cast<size_t>(std::distance(m.rows().begin(), found));
    std::vector<std::string> before;
    for (int c = 0; c < hydra::ui::kLibraryColumnCount; ++c)
        before.push_back(library_cell_text(m.rows()[chair], c));
    REQUIRE(m.set_summary_for(lib.chair, lib.chair_ready) == 1);
    for (int c = 0; c < hydra::ui::kLibraryColumnCount; ++c) {
        INFO("column " << c);
        if (c == hydra::ui::kColumnBestPath)
            CHECK(library_cell_text(m.rows()[chair], c) != before[static_cast<size_t>(c)]);
        else
            CHECK(library_cell_text(m.rows()[chair], c) == before[static_cast<size_t>(c)]);
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

// The song folders behind the pinned search text and sort keys: song.ini
// lines with tags, accents and full-width letters (the ideographic space is
// U+3000), folders with accents and full-width letters, and one song.ini that
// names nothing. Each holds a chart of its own.
std::vector<SongFolder> pinned_song_folders() {
    return {{"pack\\empty", ""},
            {"common\\Tagged\\tags",
             "name = <color=#ff0000>Red</color> Song\nartist = <b>The</b> Band\n"
             "charter = <i>Hoph2o</i>\nalbum = <size=20>Big</size> Album\n"},
            {"common\\Café\\accents",
             "name = Beyoncé Æther\nartist = Motörhead\ncharter = Łukasz\nalbum = Straße\n"},
            {"ｗｉｄｅ\\wide",
             "name = ＢＵＲＮＯＵＴ！\nartist = Ｇｒｅｅｎ" "\xE3\x80\x80" "Ｄａｙ\n"
             "charter = 東京事変\nalbum = Ａｌｂｕｍ" "\xE3\x80\x80" "\n"}};
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
    // hand; a sort key taken from the wrong text fails. The rows come from a
    // real scan of song folders whose song.ini lines hold tags, accents and
    // full-width letters (the ideographic space is U+3000), and whose folders
    // hold accents and full-width letters. The folder is where each song
    // folder sits under the scan's root. The "empty" song.ini names nothing, so
    // the scan stores its placeholders (title_or_unknown, artist_or_unknown,
    // charter_or_unknown). The rows that held spaces at the ends are not here:
    // the scan trims every field before it stores it.
    const LibraryModel m = scanned_library("lm_pinned", pinned_song_folders());
    struct Want {
        const char* folder;
        // By column: title, artist, album, charter, folder.
        std::array<const char*, hydra::ui::kColumnBestPath> keys;
        // make_searchable's fields: title, artist, album, charter, folder.
        std::array<const char*, 5> search;
    };
    const Want want[] = {
        {"empty",
         {"(unknown)", "(unknown)", "", "(unknown)", "pack"},
         {"(unknown)", "<unknown artist>", "", "<unknown charter>", "pack"}},
        {"tags",
         {"red song", "the band", "big album", "hoph2o", "common\\tagged"},
         {"red song", "the band", "big album", "hoph2o", "common\\tagged"}},
        {"accents",
         {"beyonce aether", "motorhead", "strasse", "lukasz", "common\\cafe"},
         {"beyonce aether", "motorhead", "strasse", "lukasz", "common\\cafe"}},
        {"wide",
         {"burnout!", "green day", "album ", "東京事変", "wide"},
         {"burnout!", "green day", "album ", "東京事変", "wide"}},
    };
    REQUIRE(m.rows().size() == std::size(want));
    for (size_t i = 0; i < std::size(want); ++i) {
        const hydra::ui::LibraryRow& row = row_in(m, want[i].folder);
        INFO("row " << want[i].folder);
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
                                                "Red Song"});
}

TEST_CASE("library model: rows built on threads match the one-thread rows, in the same order") {
    // Enough copies of the pinned song folders for more rows than set_charts
    // has threads, and a row count that is not a multiple of the threads, so
    // the shares are uneven. Every folder holds a chart of its own, so each
    // copy has its own hash; the rows are the store's listing of the scan.
    static constexpr int kThreads = static_cast<int>(hydra::store::kLibraryOpenThreads);
    static constexpr int kPinned = 4;  // pinned_song_folders().size(), checked below
    constexpr int kCopies = [] {
        int copies = kThreads / kPinned + 1;
        while ((copies * kPinned) % kThreads == 0) ++copies;
        return copies;
    }();
    static_assert(kCopies * kPinned > kThreads);
    static_assert((kCopies * kPinned) % kThreads != 0);
    REQUIRE(pinned_song_folders().size() == static_cast<size_t>(kPinned));
    std::vector<SongFolder> folders;
    for (int copy = 0; copy < kCopies; ++copy)
        for (SongFolder f : pinned_song_folders()) {
            f.name = "copy " + std::to_string(copy) + "\\" + f.name;
            folders.push_back(std::move(f));
        }
    std::vector<ChartLibraryEntry> charts = scanned_entries("lm_threads", folders);
    REQUIRE(charts.size() == folders.size());
    const std::vector<ChartLibraryEntry> given = charts;

    LibraryModel m;
    m.set_charts(std::move(charts));
    REQUIRE(m.rows().size() == given.size());
    for (size_t i = 0; i < given.size(); ++i) {
        // set_charts keeps its rows in the order it was given them.
        INFO("row " << i << " " << given[i].md5);
        check_same_row(m.rows()[i], one_thread_row(given[i]));
    }
    // The default sort, Title ascending: a library of one copy's order (the
    // pinned test checks it), each title once per copy.
    const LibraryModel one_copy = scanned_library("lm_threads_one", pinned_song_folders());
    std::vector<std::string> want;
    for (const std::string& title : titles(one_copy))
        want.insert(want.end(), static_cast<size_t>(kCopies), title);
    REQUIRE(want.size() == given.size());
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
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    const std::uint64_t first = m.rows_version();
    REQUIRE(m.set_summary_for(lib.chair, lib.chair_ready) == 1);
    m.set_query("burnout");
    m.set_chip(StatusChip::Analyzed);
    m.set_sort(LibrarySort::Artist, false);
    CHECK(m.rows_version() == first);
    m.set_charts(one_scanned_chart("lm_rows_version"));
    CHECK(m.rows_version() != first);
}

TEST_CASE("library model: query_version moves when the search changes and at no other time") {
    const ScannedSample& lib = scanned_sample();
    LibraryModel m = lib.model;
    const std::uint64_t first = m.query_version();
    REQUIRE(m.set_summary_for(lib.chair, lib.chair_ready) == 1);
    m.set_chip(StatusChip::Analyzed);
    m.set_sort(LibrarySort::Artist, false);
    m.set_charts(one_scanned_chart("lm_query_version"));
    CHECK(m.query_version() == first);
    m.set_query("burnout");
    const std::uint64_t searched = m.query_version();
    CHECK(searched != first);
    m.set_query("burnout");  // the same search again is no change
    CHECK(m.query_version() == searched);
    m.set_query("");
    CHECK(m.query_version() != searched);
}

TEST_CASE("library model: filtering 20,000 charts gives each search the rows it names") {
    // query_matches runs for every row on every applied keystroke. The times
    // are printed for a person to read; a timing belongs in hydra_bench, not
    // in a pass/fail check that a busy machine can fail.
    // Three corpus charts' results (testdata/input), each saved and read back
    // from the store the way the app reads it (lookup_one). Every third row
    // is given one of them in turn; the other rows have nothing saved.
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    const std::vector<corpus::ChartWithPaths> analysed = corpus::charts_with_paths(run.settings, 3);
    REQUIRE_MESSAGE(analysed.size() == 3, "the corpus needs three charts with paths");
    hydra::store::RecordStore store(":memory:");
    std::vector<RecordLookup> results;
    for (size_t k = 0; k < analysed.size(); ++k) {
        const std::string md5 = "result" + std::to_string(k);
        store.save(hydra::store::prepare_record(md5, run.profile, analysed[k].result.record));
        results.push_back(store.lookup_one(md5, run.profile));
        REQUIRE(results.back().status == RecordStatus::Ready);
        REQUIRE(results.back().record.summary().has_scored_best_path());
    }
    // The result row i is given, or none.
    const auto result_of = [&](int i) -> const RecordLookup* {
        return i % 3 == 0 ? &results[static_cast<size_t>((i / 3) % 3)] : nullptr;
    };
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
        const RecordLookup* given = result_of(i);
        summaries.push_back(given ? *given : RecordLookup{});
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
    // stars: and squeeze filters match only the rows given a result, and read
    // that result's saved summary. The stars asked for are the first result's
    // own star count, so at least a third of the given rows match.
    REQUIRE(results[0].record.summary().stars.has_value());
    const int stars = *results[0].record.summary().stars;
    const auto stars_of = [&](int i) -> std::optional<int> {
        const RecordLookup* given = result_of(i);
        if (!given) return std::nullopt;
        return given->record.summary().stars;
    };
    // A squeeze limit passes a path whose shown figure is within it
    // (timing_within_limit) and a path that needs no timing (shown_timing
    // gives none; search_tips words it for the player).
    const int squeeze_limit = 20;  // any whole number of ms; the query types it
    const auto within_squeeze_limit = [&](int i) {
        const RecordLookup* given = result_of(i);
        if (!given) return false;
        const std::optional<double> shown =
            hydra::shown_timing(given->record.summary().hardest_ms);
        return !shown || hydra::timing_within_limit(*shown, squeeze_limit);
    };
    struct QueryRows {
        std::string query;
        size_t rows;
    };
    const QueryRows queries[] = {
        {"s", count_where([&](int i) { return words_match(i, {"s"}); })},
        {"song 1", count_where([&](int i) { return words_match(i, {"song", "1"}); })},
        {"\"tier 4\"", count_where([](int i) { return i % 7 == 4; })},
        {"artist 07 song", count_where([&](int i) { return words_match(i, {"artist", "07", "song"}); })},
        {"charter", count_where([&](int i) { return words_match(i, {"charter"}); })},
        {"stars:" + std::to_string(stars),
         count_where([&](int i) { return stars_of(i) == std::optional<int>(stars); })},
        {"squeeze<=" + std::to_string(squeeze_limit) + " pack",
         count_where([&](int i) { return within_squeeze_limit(i) && words_match(i, {"pack"}); })},
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
