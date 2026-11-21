// The store's one save and one lookup (D115): prepare_record, failure_record,
// RecordStore::save, lookup, lookup_one and settled_song_ids, the status rules
// a lookup answers by (which stamps read Ready, why a record is Stale, what reads
// Not analyzed), the store's readers (RecordStore::reader, StoreReader, and
// RecordStore::open_own_reader, OwnReader, with its hold). How saved
// records read back across caps, lenses and the corpus is in
// test_store_results.cpp.
#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <sqlite3.h>

#include "app/user_messages.h"  // plain_error, plain_error_detail
#include "core/error_kind.h"
#include "core/model.h"
#include "core/rules.h"  // RulesStamp
#include "corpus_util.h"
#include "db_file_util.h"  // scalar_on_file, exec_on_file
#include "display_fixtures.h"  // save_old_build_record, kOldBuildStamp, other_rules_record
#include "library_fixtures.h"  // library_row
#include "parse/song.h"
#include "source_tree.h"  // sourcetree::root
#include "store/chart_index.h"  // ChartIndex::song_ids
#include "store/record_store.h"
#include "store/stored_versions.h"  // kResultsStamp
#include "store_file_fixtures.h"  // StoreFile, HeldSaveGroup
#include "store_fixtures.h"  // fixture, fixture_chart, store_fixture_settings, OldLibrary, at4
#include "temp_util.h"
#include "wait_util.h"  // wait_until, WaitTimeout

using namespace hydra;
using namespace hydra::store;

using hydra::test::HeldSaveGroup;
using hydra::test::OldLibrary;
using hydra::test::StoreFile;
using hydra::test::at4;
using hydra::test::at_cap;
using hydra::test::fixture;
using hydra::test::fixture_chart;
using hydra::test::no_paths_at_cap;
using hydra::test::old_library_analysis;
using hydra::test::store_fixture_settings;
using hydra::test::scalar_on_file;

namespace {

// Whether two lookups gave the same answer, field by field: RecordLookup has
// no operator==.
bool same_answer(const RecordLookup& a, const RecordLookup& b) {
    return a.status == b.status && a.stale_build == b.stale_build &&
           a.stale_rules == b.stale_rules && a.record == b.record &&
           a.failure_kind == b.failure_kind && a.failure_text == b.failure_text;
}

// The fixture's chart analyzed under other_rules(), the rules a user's
// hydra_rules.ini might set: the record a store running the defaults reads
// Stale for "other rules".
const HydraRecord& other_rules_analysis(
    app::AnalysisSettings settings = store_fixture_settings()) {
    settings.rules = hydra::test::other_rules();
    return corpus::analyzed(fixture_chart(), settings);
}

// Two corpus charts' real records at 4 bars, both with paths, the first
// keeping more path rows than the second, so a save of one over the other
// is visible in every field and in the file's path rows.
std::pair<HydraRecord, HydraRecord> more_and_fewer_paths() {
    const std::vector<app::AnalysisResult> found =
        corpus::analyzed_with_paths(store_fixture_settings(), 20);
    for (const app::AnalysisResult& a : found) {
        for (const app::AnalysisResult& b : found) {
            if (record_of("x", a.record).paths.size() > record_of("x", b.record).paths.size())
                return {a.record, b.record};
        }
    }
    FAIL("no two corpus charts keep different numbers of path rows");
    return {};
}

// What the app files for a chart its analysis refuses: analyze_chart_file
// throws on the hand-written chart `file` in testdata/parse_edge, and
// prepare_failed_chart turns that throw into the failure record. `kind` and
// `text` are the throw's own kind and message, for checking what reads back.
struct ChartFailure {
    PreparedRecord prepared;
    ErrorKind kind = ErrorKind::ChartUnreadable;
    std::string text;
};
// The same for the chart at `path`, analyzed under `settings`.
ChartFailure failed_chart_at(const std::string& song_id, const std::string& path,
                             const app::AnalysisSettings& settings, const Profile& profile) {
    try {
        app::analyze_chart_file(path, settings);
    } catch (const KindedError& e) {
        std::optional<PreparedRecord> prepared = app::prepare_failed_chart(song_id, profile, e);
        REQUIRE_MESSAGE(prepared.has_value(), "no failure record for " << path << ": " << e.what());
        return ChartFailure{*prepared, e.kind(), e.what()};
    }
    FAIL("analyze_chart_file did not refuse " << path);
    return {};
}
ChartFailure failed_chart(const std::string& song_id, const std::string& file,
                          const Profile& profile = at4()) {
    return failed_chart_at(song_id, corpus::testdata_root() + "/parse_edge/" + file,
                           store_fixture_settings(), profile);
}

// A chart with no Expert Pro Drums notes (NoNotes) and one with no header
// (ChartUnreadable), from testdata/parse_edge/expected_expert.tsv.
const char* const kNoNotesChart = "c15_unterminated_drums.chart";
const char* const kUnreadableChart = "c25_bad_header.chart";

// The four charts the reader tests ask about: a Ready chart, a failure
// record, a record saved under another build, and a chart with no record.
const std::vector<std::string>& four_chart_ids() {
    static const std::vector<std::string> ids = {"ready", "failed", "build", "never"};
    return ids;
}
void save_four_charts(RecordStore& store) {
    store.save(prepare_record("ready", at4(), fixture().record));
    store.save(failed_chart("failed", kNoNotesChart).prepared);
    store.save_under_build_for_test(prepare_record("build", at4(), fixture().record),
                                    hydra::test::kOldBuildStamp);
}

// `reader`, one of the store's readers, gives the store's own answers for the
// four charts, by lookup, lookup_one and settled_song_ids. The expected
// values are the writer's.
void check_answers_as_store(RecordStore& store, StoreReads& reader) {
    const std::vector<std::string>& ids = four_chart_ids();
    const std::vector<RecordLookup> by_store = store.lookup(ids, at4());
    const std::vector<RecordLookup> by_reader = reader.lookup(ids, at4());
    REQUIRE(by_reader.size() == by_store.size());
    for (size_t i = 0; i < ids.size(); ++i) {
        INFO(ids[i]);
        CHECK(same_answer(by_reader[i], by_store[i]));
        CHECK(same_answer(reader.lookup_one(ids[i], at4()), store.lookup_one(ids[i], at4())));
    }
    CHECK(reader.settled_song_ids(at4()) == store.settled_song_ids(at4()));
}

// The store's window reader gives the store's own answers for the four charts.
void check_reader_answers_as_store(RecordStore& store) {
    check_answers_as_store(store, store.reader());
}

// How many profile rows the file at `path` holds under chart mode `mode`.
int64_t profiles_in_mode(const std::string& path, const std::string& mode) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM profiles WHERE chartmode='" + mode + "'");
}

// Whether `path` can be deleted now, which on Windows means no connection
// holds it open.
bool can_delete(const std::string& path) {
    std::error_code ec;
    const bool removed = std::filesystem::remove(std::filesystem::u8path(path), ec);
    return removed && !ec;
}

// The corpus as the app's library scan lists it: discover_charts over
// testdata/input, saved the app's way (save_scan_as_library). Fails on any
// scan error or a failed save.
void save_corpus_scan(RecordStore& store) {
    const auto scan = app::discover_charts({corpus::root()});
    REQUIRE_MESSAGE(scan.second.empty(), "scan error: " << scan.second.front());
    REQUIRE_FALSE(scan.first.empty());
    const std::optional<std::string> error = app::save_scan_as_library(store, scan.first);
    REQUIRE_MESSAGE(!error, "saving the scan failed: " << *error);
}

// The fixture's chart searched at the widest early-fill window the setting
// allows (kMaxEarlyFillWindowMs): a real record for a profile at that window.
const HydraRecord& searched_at_max_window() {
    app::AnalysisSettings s = store_fixture_settings();
    s.early_fill_window_ms = kMaxEarlyFillWindowMs;
    return corpus::analyzed(fixture_chart(), s);
}

}  // namespace

TEST_CASE("the store's reader answers every lookup as the store does") {
    // A real v2.12.1 library holds three records of another build; this build
    // saves a real analysis and a real failure beside them.
    OldLibrary old("reader_as_store");
    RecordStore store(old.path);
    const Profile profile = OldLibrary::profile();
    store.save(prepare_record("ready", profile,
                              corpus::analyzed(fixture_chart(), OldLibrary::settings())));
    store.save(failed_chart("failed", kNoNotesChart, profile).prepared);

    std::vector<std::string> ids = OldLibrary::song_ids();
    ids.insert(ids.end(), {"ready", "failed", "never"});
    const std::vector<RecordLookup> by_store = store.lookup(ids, profile);
    // Every kind of answer is here: Stale, Ready, CantAnalyze and NotAnalyzed.
    REQUIRE(by_store.size() == ids.size());
    CHECK(by_store[0].status == RecordStatus::Stale);
    CHECK(by_store[3].status == RecordStatus::Ready);
    CHECK(by_store[4].status == RecordStatus::CantAnalyze);
    CHECK(by_store[5].status == RecordStatus::NotAnalyzed);

    StoreReads& reader = store.reader();
    const std::vector<RecordLookup> by_reader = reader.lookup(ids, profile);
    REQUIRE(by_reader.size() == by_store.size());
    for (size_t i = 0; i < ids.size(); ++i) {
        INFO(ids[i]);
        CHECK(same_answer(by_reader[i], by_store[i]));
        CHECK(same_answer(reader.lookup_one(ids[i], profile), store.lookup_one(ids[i], profile)));
    }
    CHECK(reader.settled_song_ids(profile) == store.settled_song_ids(profile));
}

TEST_CASE("an in-memory store reads through itself") {
    RecordStore store(":memory:");
    CHECK(&store.reader() == static_cast<StoreReads*>(&store));

    // Inside an open save group the reader sees what the store sees there.
    store.begin_save_group();
    store.save(prepare_record("h1", at4(), fixture().record));
    CHECK(same_answer(store.reader().lookup_one("h1", at4()), store.lookup_one("h1", at4())));
    store.commit_save_group();
}

// ---- the reader of a store with a file (StoreReader) ------------------------

TEST_CASE("a reader's lookup does not wait while a save group is open") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    store.save(prepare_record("h1", at4(), fixture().record));
    const RecordLookup want = store.lookup_one("h1", at4());

    HeldSaveGroup group(store);
    const RecordLookup got = store.reader().lookup_one("h1", at4());
    const bool returned_while_held = group.held();
    group.let_go();
    CHECK(returned_while_held);
    CHECK(same_answer(got, want));
}

TEST_CASE("a reader sees a save group's chart only once the group commits") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    const RecordLookup before = store.reader().lookup_one("h1", at4());

    store.begin_save_group();
    store.save(prepare_record("h1", at4(), fixture().record));
    const RecordLookup during = store.reader().lookup_one("h1", at4());
    store.commit_save_group();

    CHECK(same_answer(during, before));
    CHECK(same_answer(store.reader().lookup_one("h1", at4()), store.lookup_one("h1", at4())));
}

TEST_CASE("the file store's reader gives the writer's answer for every lookup") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    save_four_charts(store);
    check_reader_answers_as_store(store);
}

TEST_CASE("a store opened under other rules reads the same through its reader") {
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        save_four_charts(store);
    }
    RecordStore store(file.path, core::RulesStamp{core::default_stamp().fixed + 1});
    check_reader_answers_as_store(store);
}

TEST_CASE("a reader's lookup reads one moment") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    // Two real corpus records saved under one song id: the second differs
    // from the first in its path rows.
    const std::pair<HydraRecord, HydraRecord> records = more_and_fewer_paths();
    store.save(prepare_record("h1", at4(), records.first));
    const PreparedRecord second = prepare_record("h1", at4(), records.second);
    const RecordLookup before = store.lookup_one("h1", at4());

    // The writer saves the second record between the reader's two reads.
    bool saved = false;
    store.set_between_lookup_reads_for_test([&] {
        if (saved) return;
        saved = true;
        store.save(second);
    });
    const RecordLookup during = store.reader().lookup_one("h1", at4());
    store.set_between_lookup_reads_for_test({});
    const RecordLookup after = store.lookup_one("h1", at4());

    REQUIRE(saved);
    REQUIRE_FALSE(same_answer(after, before));
    CHECK(same_answer(during, before));
    CHECK(same_answer(store.reader().lookup_one("h1", at4()), after));
}

TEST_CASE("a reader holds nothing open between reads") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    const std::filesystem::path wal = std::filesystem::u8path(file.path + "-wal");
    {
        const RecordStore::BatchWrites batch(store);
        save_four_charts(store);
        store.reader().lookup(four_chart_ids(), at4());
        store.reader().settled_song_ids(at4());
        REQUIRE(std::filesystem::file_size(wal) > 0);
    }
    // The guard's truncating checkpoint empties the log only when no other
    // connection holds a read open.
    CHECK(std::filesystem::file_size(wal) == 0);
}

TEST_CASE("a reader that cannot open fails the read and the next read tries again") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    store.save(prepare_record("h1", at4(), fixture().record));

    // Armed before the reader's first read, so its first open is the one that
    // fails.
    store.fail_next_reader_open_for_test();
    bool threw = false;
    try {
        store.reader().lookup_one("h1", at4());
    } catch (const KindedError& e) {
        threw = true;
        CHECK(e.kind() == ErrorKind::DatabaseRead);
    }
    CHECK(threw);
    CHECK(same_answer(store.reader().lookup_one("h1", at4()), store.lookup_one("h1", at4())));
}

TEST_CASE("closing the store closes its reader") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    store.save(prepare_record("h1", at4(), fixture().record));
    store.reader().lookup_one("h1", at4());
    store.reader().settled_song_ids(at4());
    store.close();
    CHECK(can_delete(file.path));
}

TEST_CASE("the reader counts records and library charts as the writer does") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    // The library is the corpus scan, saved before the results, since a
    // rescan deletes the results of charts it does not list.
    save_corpus_scan(store);
    save_four_charts(store);

    // The writer's counts are the expected values; neither is zero, so a
    // reader that read nothing would not pass.
    const int64_t records = store.record_count();
    const int64_t charts = store.chart_library_count();
    REQUIRE(records > 0);
    REQUIRE(charts > 0);
    CHECK(store.reader().record_count() == records);
    CHECK(store.reader().chart_library_count() == charts);
}

TEST_CASE("a reader's counts see a save only once it commits") {
    StoreFile file("store_records");
    RecordStore store(file.path);
    store.save(prepare_record("h1", at4(), fixture().record));
    const int64_t before = store.reader().record_count();

    store.begin_save_group();
    store.save(prepare_record("h2", at4(), fixture().record));
    const int64_t during = store.reader().record_count();
    store.commit_save_group();

    CHECK(during == before);
    REQUIRE(store.record_count() != before);
    CHECK(store.reader().record_count() == store.record_count());
}

// ---- a reader of the caller's own (OwnReader) and its hold ------------------

namespace {

// Three reads of one moment of the store: the second chart's answer, the
// record count, and the charts the library lists. move_on changes all three.
struct MomentReads {
    RecordLookup second;
    int64_t records = 0;
    std::vector<std::string> charts;
};
MomentReads moment_reads(StoreReads& reads) {
    return {reads.lookup_one("h2", at4()), reads.record_count(), reads.chart_index().song_ids()};
}
void check_same_moment(const MomentReads& got, const MomentReads& want) {
    CHECK(same_answer(got.second, want.second));
    CHECK(got.records == want.records);
    CHECK(got.charts == want.charts);
}

// A store whose library lists "h1", with a result saved for it.
void list_and_save_first(RecordStore& store) {
    store.rebuild_chart_library({library_row("h1", "h1")});
    store.save(prepare_record("h1", at4(), fixture().record));
}
// The writer moves on: the library lists "h2" too, and "h2" gets a result.
void move_on(RecordStore& store) {
    store.rebuild_chart_library({library_row("h1", "h1"), library_row("h2", "h2")});
    store.save(prepare_record("h2", at4(), fixture().record));
}

// The -wal file's size in bytes.
uintmax_t wal_size(const StoreFile& file) {
    return std::filesystem::file_size(std::filesystem::u8path(file.path + "-wal"));
}

}  // namespace

TEST_CASE("an own reader gives the writer's answer for every read") {
    StoreFile file("own_reader");
    RecordStore store(file.path);
    save_corpus_scan(store);
    save_four_charts(store);

    const std::unique_ptr<OwnReader> reader = store.open_own_reader();
    check_answers_as_store(store, reader->reads());
    REQUIRE(store.record_count() > 0);
    CHECK(reader->reads().record_count() == store.record_count());
    CHECK(reader->reads().chart_library_count() == store.chart_library_count());
    // The chart list reads, field for field, are in test_store_library.cpp
    // ("an own reader lists and indexes the library as the writer does").
    CHECK(reader->reads().chart_index().song_ids() == store.chart_index().song_ids());
}

TEST_CASE("a hold makes every call see one moment") {
    StoreFile file("own_reader");
    RecordStore store(file.path);
    list_and_save_first(store);
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();
    const MomentReads before = moment_reads(store);

    MomentReads held;
    {
        const OwnReader::Hold hold = reader->hold();
        // The hold's first read fixes its moment, before the writer moves on.
        reader->reads().lookup_one("h1", at4());
        move_on(store);
        held = moment_reads(reader->reads());
    }
    const MomentReads after = moment_reads(store);

    REQUIRE_FALSE(same_answer(after.second, before.second));
    REQUIRE(after.records != before.records);
    REQUIRE(after.charts != before.charts);
    check_same_moment(held, before);
    check_same_moment(moment_reads(reader->reads()), after);
}

TEST_CASE("an own reader with no hold sees each commit") {
    StoreFile file("own_reader");
    RecordStore store(file.path);
    list_and_save_first(store);
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();
    const MomentReads before = moment_reads(store);

    check_same_moment(moment_reads(reader->reads()), before);
    move_on(store);
    const MomentReads after = moment_reads(store);
    REQUIRE(after.records != before.records);
    check_same_moment(moment_reads(reader->reads()), after);
}

TEST_CASE("a held own reader does not wait for a save group, and a save group does not wait for it") {
    StoreFile file("own_reader");
    RecordStore store(file.path);
    list_and_save_first(store);
    const RecordLookup want = store.lookup_one("h1", at4());
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();

    const OwnReader::Hold hold = reader->hold();
    // A helper holds a save group (capped by kWaitCap) while the held reader
    // reads.
    HeldSaveGroup group(store);
    const RecordLookup got = reader->reads().lookup_one("h1", at4());
    const bool returned_while_held = group.held();
    group.let_go();
    CHECK(returned_while_held);
    CHECK(same_answer(got, want));

    // A save group on the writer while the hold is open. The store sets no
    // busy timeout, so a writer the hold refused would throw here rather than
    // wait.
    store.begin_save_group();
    store.save(prepare_record("h2", at4(), fixture().record));
    store.commit_save_group();
    CHECK(store.lookup_one("h2", at4()).status == RecordStatus::Ready);
}

TEST_CASE("an open hold does not stop the window's reader") {
    StoreFile file("own_reader");
    RecordStore store(file.path);
    list_and_save_first(store);
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();

    const OwnReader::Hold hold = reader->hold();
    const RecordLookup held_before = reader->reads().lookup_one("h2", at4());
    store.save(prepare_record("h2", at4(), fixture().record));
    const RecordLookup now = store.lookup_one("h2", at4());
    REQUIRE_FALSE(same_answer(now, held_before));
    CHECK(same_answer(store.reader().lookup_one("h2", at4()), now));
}

TEST_CASE("a hold open at a batch's end loses nothing, and the next batch's end empties the log") {
    StoreFile file("own_reader");
    RecordStore store(file.path);
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();
    {
        const OwnReader::Hold hold = reader->hold();
        reader->reads().record_count();  // the hold's moment
        {
            const RecordStore::BatchWrites batch(store);
            save_four_charts(store);
        }
        // The guard's checkpoint could not pass the hold, so the log is not
        // emptied; every result is still read back from the writer.
        CHECK(wal_size(file) > 0);
        CHECK(store.lookup_one("ready", at4()).status == RecordStatus::Ready);
        CHECK(store.lookup_one("failed", at4()).status == RecordStatus::CantAnalyze);
        CHECK(store.lookup_one("build", at4()).status == RecordStatus::Stale);
    }
    {
        const RecordStore::BatchWrites batch(store);
    }
    CHECK(wal_size(file) == 0);
    // The results reached the file: a second connection reads them too.
    CHECK(store.reader().lookup_one("ready", at4()).status == RecordStatus::Ready);
}

TEST_CASE("the log keeps growing while a hold is open") {
    // The same four save groups, each saving the same charts again, with and
    // without a hold open; returns the -wal file's size after each group.
    const auto sizes_after_groups = [](bool with_hold) {
        StoreFile file("own_reader_log");
        RecordStore store(file.path);
        store.save(prepare_record("h1", at4(), fixture().record));
        const std::unique_ptr<OwnReader> reader = store.open_own_reader();
        std::vector<uintmax_t> sizes;
        const auto save_groups = [&] {
            reader->reads().record_count();  // the hold's moment, when there is one
            const RecordStore::BatchWrites batch(store);
            // One page, the least there is, so every group's commit crosses
            // it and SQLite folds the log after each group.
            store.set_wal_autocheckpoint_for_test(1);
            for (int group = 0; group < 4; ++group) {
                store.begin_save_group();
                for (const char* id : {"a", "b", "c", "d"})
                    store.save(prepare_record(id, at4(), fixture().record));
                store.commit_save_group();
                sizes.push_back(wal_size(file));
            }
        };
        if (with_hold) {
            const OwnReader::Hold hold = reader->hold();
            save_groups();
        } else {
            save_groups();
        }
        return sizes;
    };

    const std::vector<uintmax_t> held = sizes_after_groups(true);
    for (size_t i = 1; i < held.size(); ++i) {
        CAPTURE(i);
        CHECK(held[i] > held[i - 1]);
    }
    // With no hold, the fold after the first group lets the second start the
    // log over, and no later group makes it larger.
    const std::vector<uintmax_t> unheld = sizes_after_groups(false);
    for (size_t i = 2; i < unheld.size(); ++i) {
        CAPTURE(i);
        CHECK(unheld[i] <= unheld[1]);
    }
    CHECK(unheld.back() < held.back());
}

TEST_CASE("an own reader that cannot open fails the read") {
    StoreFile file("own_reader");
    RecordStore store(file.path);
    store.save(prepare_record("h1", at4(), fixture().record));

    store.fail_next_reader_open_for_test();
    bool threw = false;
    try {
        store.open_own_reader();
    } catch (const KindedError& e) {
        threw = true;
        CHECK(e.kind() == ErrorKind::DatabaseRead);
    }
    CHECK(threw);
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();
    CHECK(same_answer(reader->reads().lookup_one("h1", at4()), store.lookup_one("h1", at4())));
}

TEST_CASE("closing an own reader lets go of the file") {
    StoreFile file("own_reader");
    RecordStore store(file.path);
    store.save(prepare_record("h1", at4(), fixture().record));
    std::unique_ptr<OwnReader> reader = store.open_own_reader();
    {
        const OwnReader::Hold hold = reader->hold();
        reader->reads().lookup_one("h1", at4());
        reader->reads().record_count();
    }
    reader.reset();
    store.close();
    CHECK(can_delete(file.path));
}

TEST_CASE("an in-memory store's own reader reads through the store") {
    RecordStore store(":memory:");
    save_four_charts(store);
    const std::unique_ptr<OwnReader> reader = store.open_own_reader();
    check_answers_as_store(store, reader->reads());
    CHECK(reader->reads().record_count() == store.record_count());
    CHECK(reader->reads().chart_library_count() == store.chart_library_count());

    // A hold changes nothing.
    {
        const OwnReader::Hold hold = reader->hold();
        check_answers_as_store(store, reader->reads());
    }

    // Inside an open save group the own reader sees what the store sees there.
    store.begin_save_group();
    store.save(prepare_record("h1", at4(), fixture().record));
    CHECK(same_answer(reader->reads().lookup_one("h1", at4()), store.lookup_one("h1", at4())));
    store.commit_save_group();
}

TEST_CASE("store: a saved analysis reads back Ready with its paths in rank order") {
    // Trippin on a Hole in a Paper Heart under the old library's settings: the
    // record a real v2.12.1 hydra_batch wrote for it under these settings
    // (testdata/store/v2.12.1_library.db) keeps five paths, so the ranks are
    // checked against more than one engine path.
    const std::string chart = corpus::chart_named("Trippin on a Hole in a Paper Heart");
    REQUIRE_FALSE(chart.empty());
    const HydraRecord& analysis = corpus::analyzed(chart, OldLibrary::settings());
    const std::vector<const Path*> engine_paths = analysis.all_paths();
    REQUIRE(engine_paths.size() >= 2);
    const Profile profile = OldLibrary::profile();
    RecordStore store(":memory:");
    const PreparedRecord prepared = prepare_record("h1", profile, analysis);
    store.save(prepared);

    const RecordLookup got = store.lookup_one("h1", profile);
    REQUIRE(got.status == RecordStatus::Ready);
    CHECK(got.record == prepared.record);
    // The record keeps the engine's first paths, as many as the cap allows,
    // in the engine's order, each numbered by its place.
    const size_t kept = std::min(engine_paths.size(), static_cast<size_t>(kPathsPerRecord));
    REQUIRE(got.record.paths.size() == kept);
    for (size_t i = 0; i < kept; ++i) {
        INFO("rank " << i + 1);
        CHECK(got.record.paths[i].rank == static_cast<int>(i + 1));
        CHECK(got.record.paths[i].path == engine_paths[i]->pathstring());
        CHECK(got.record.paths[i].score == engine_paths[i]->totalscore());
    }
    CHECK(got.record.pathcount == static_cast<int>(engine_paths.size()));
    // Rank 1 is the engine's best path, and the summary reads its score.
    const Path& best = analysis.best_path();
    CHECK(got.record.best_path_text() == best.pathstring());
    CHECK(got.record.summary().score == std::optional<int64_t>(best.totalscore()));
    CHECK(got.record.paths.front().optimal);
}

TEST_CASE("store: prepare_record refuses a profile the analysis did not run under") {
    // A 4-bar analysis filed at another cap. The ms limit and the fill rule
    // are refused the same way, in both directions, in test_store_results.cpp
    // ("prepare_record refuses a profile whose ms limit isn't the analysis's",
    // "prepare_record refuses a profile that names the other fill rule").
    const HydraRecord& analysis = fixture().record;
    REQUIRE(analysis.sp_cap == 4);
    CHECK_THROWS_AS(prepare_record("h1", Profile{"mode", CapQuery::at(8), Lens{}}, analysis),
                    std::invalid_argument);
}

TEST_CASE("store: prepare_record refuses a profile at another early-fill window") {
    // A record searched at the default window, filed under a profile at the
    // widest the setting allows.
    const HydraRecord& analysis = fixture().record;
    REQUIRE(analysis.early_fill_window_ms == kDefaultEarlyFillWindowMs);
    Profile at_85 = at4();
    at_85.lens.efill_window_ms = kMaxEarlyFillWindowMs;
    CHECK_THROWS_AS(prepare_record("h1", at_85, analysis), std::invalid_argument);
    // And the other way: the same chart searched at that window, filed under
    // the default profile.
    const HydraRecord& searched_at_85 = searched_at_max_window();
    REQUIRE(searched_at_85.early_fill_window_ms == kMaxEarlyFillWindowMs);
    CHECK_THROWS_AS(prepare_record("h1", at4(), searched_at_85), std::invalid_argument);
    // The matching pair files.
    CHECK(prepare_record("h1", at_85, searched_at_85).profile.lens.efill_window_ms ==
          kMaxEarlyFillWindowMs);
}

TEST_CASE("store: one chart's results at two early-fill windows are two results") {
    // Two profiles that differ only in the window. Each save keeps the other's
    // record, and each lookup gets its own back. The two records are two
    // corpus charts' real analyses, one at each window, filed under one chart
    // id so each lookup's answer shows whose record it read.
    const Profile at_default = at4();
    Profile at_85 = at4();
    at_85.lens.efill_window_ms = kMaxEarlyFillWindowMs;
    REQUIRE(at_default != at_85);
    app::AnalysisSettings settings_85 = store_fixture_settings();
    settings_85.early_fill_window_ms = kMaxEarlyFillWindowMs;
    std::string other_chart;
    for (const corpus::ChartWithPaths& c : corpus::charts_with_paths(store_fixture_settings(), 2))
        if (c.chart != fixture_chart()) other_chart = c.chart;
    REQUIRE_MESSAGE(!other_chart.empty(), "the corpus has one chart with paths");
    const PreparedRecord saved_default = prepare_record("h1", at_default, fixture().record);
    const PreparedRecord saved_85 =
        prepare_record("h1", at_85, corpus::analyzed(other_chart, settings_85));
    REQUIRE(saved_default.record != saved_85.record);

    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        store.save(saved_default);
        store.save(saved_85);
        const RecordLookup got_default = store.lookup_one("h1", at_default);
        const RecordLookup got_85 = store.lookup_one("h1", at_85);
        REQUIRE(got_default.status == RecordStatus::Ready);
        REQUIRE(got_85.status == RecordStatus::Ready);
        CHECK(got_default.record == saved_default.record);
        CHECK(got_85.record == saved_85.record);
        CHECK(store.record_count() == 2);
    }
    // Two profile rows, one per window, and a reopen reads both again.
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 2);
    RecordStore reopened(file.path);
    CHECK(reopened.lookup_one("h1", at_default).record == saved_default.record);
    CHECK(reopened.lookup_one("h1", at_85).record == saved_85.record);
}

TEST_CASE("store: every path of one analysis carries the same note count") {
    // The record keeps the note count once (record_of takes rank 1's); this is
    // why that loses nothing: every path carries the chart's own note count
    // (Song::note_count). Every corpus chart is analyzed; the one kind left
    // out is a chart with no notes at these settings, which has no paths to
    // check, and any other load failure throws and fails the test.
    const app::AnalysisSettings settings{};
    int multi_path_charts = 0;
    for (const std::string& path : corpus::chart_paths()) {
        INFO(path);
        const Song& song = corpus::song(path, settings.reading(), settings.rules);
        if (song.is_empty()) continue;
        const std::vector<const Path*> paths = corpus::analyzed(path, settings).all_paths();
        for (const Path* p : paths) CHECK(p->notecount == song.note_count());
        if (paths.size() > 1) ++multi_path_charts;
    }
    // A chart with two or more paths is the case where a path could disagree
    // with rank 1. Trippin on a Hole in a Paper Heart is one: the record a
    // real v2.12.1 hydra_batch wrote for it at these depth and cap settings,
    // with a 10 ms limit on top (testdata/store/v2.12.1_library.db), keeps
    // five paths.
    CHECK(multi_path_charts > 0);
}

TEST_CASE("store: a save under the same profile replaces the record and every path") {
    // Two real charts' analyses filed under one chart id: more path rows the
    // first time, so none may linger after the smaller save.
    const auto [more, fewer] = more_and_fewer_paths();
    const PreparedRecord first = prepare_record("h1", at4(), more);
    const PreparedRecord second = prepare_record("h1", at4(), fewer);
    StoreFile file("store_records");
    RecordStore store(file.path);
    store.save(first);
    REQUIRE(scalar_on_file(file.path, "SELECT COUNT(*) FROM record_paths") ==
            static_cast<int64_t>(first.record.paths.size()));

    store.save(second);
    const RecordLookup got = store.lookup_one("h1", at4());
    REQUIRE(got.status == RecordStatus::Ready);
    CHECK(got.record == second.record);
    CHECK(got.record.paths.size() < first.record.paths.size());
    // The file holds the second save's path rows and nothing of the first's.
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM record_paths") ==
          static_cast<int64_t>(second.record.paths.size()));
    CHECK(store.record_count() == 1);
}

TEST_CASE("store: a lookup after a second save reads the second record, not the first") {
    // lookup keeps its reads compiled between calls, so this pins that a
    // reused read carries no row or binding over from the last one.
    RecordStore store(":memory:");
    // Two real charts' analyses filed under one chart id.
    const auto [more, fewer] = more_and_fewer_paths();
    const PreparedRecord first = prepare_record("h1", at4(), more);
    store.save(first);
    const RecordLookup got_first = store.lookup_one("h1", at4());
    REQUIRE(got_first.status == RecordStatus::Ready);
    CHECK(got_first.record == first.record);

    const PreparedRecord second = prepare_record("h1", at4(), fewer);
    REQUIRE_FALSE(second.record == first.record);
    store.save(second);
    const RecordLookup got_second = store.lookup_one("h1", at4());
    REQUIRE(got_second.status == RecordStatus::Ready);
    CHECK(got_second.record == second.record);
    // Another chart through the same reads: nothing of h1's comes back.
    CHECK(store.lookup_one("h2", at4()).status == RecordStatus::NotAnalyzed);
}

TEST_CASE("store: a save purges the chart's unreadable records in every mode, and the next open "
          "deletes their emptied profiles") {
    // The real v2.12.1 library: three records another build wrote, all in
    // one profile of its chart mode. This build saves each of those charts
    // in another chart mode.
    OldLibrary old("purge_every_mode");
    const Profile old_profile = OldLibrary::profile();
    const Profile this_mode = at4("other mode");
    REQUIRE(this_mode.chartmode != old_profile.chartmode);
    REQUIRE(profiles_in_mode(old.path, old_profile.chartmode) == 1);
    {
        RecordStore store(old.path);
        for (const std::string& id : OldLibrary::song_ids()) {
            INFO(id);
            REQUIRE(store.lookup_one(id, old_profile).status == RecordStatus::Stale);
            store.save(prepare_record(id, this_mode, fixture().record));
            CHECK(store.lookup_one(id, old_profile).status == RecordStatus::NotAnalyzed);
            CHECK(store.lookup_one(id, this_mode).status == RecordStatus::Ready);
        }
        CHECK(store.record_count() == static_cast<int64_t>(OldLibrary::song_ids().size()));
    }
    { RecordStore reopened(old.path); }
    // The old build's profile lost its last record, so the reopen deleted it,
    // and no profile is left with no record.
    const std::string profiles_with_no_record =
        "SELECT COUNT(*) FROM profiles WHERE NOT EXISTS"
        " (SELECT 1 FROM records WHERE records.profile_id = profiles.id)";
    CHECK(profiles_in_mode(old.path, old_profile.chartmode) == 0);
    CHECK(scalar_on_file(old.path, profiles_with_no_record) == 0);
}

namespace {

// Two real song folders from the corpus, Young Robot and Limbo (the old
// library's two .mid charts, testdata/store/README.md), copied under a
// scratch root named by `tag`, so a test can delete one and scan again.
struct TwoSongFolders {
    std::filesystem::path root;
    std::filesystem::path gone;
    explicit TwoSongFolders(const std::string& tag) {
        testtemp::remove_dir(testtemp::temp_dir(tag));
        root = std::filesystem::u8path(testtemp::temp_dir(tag));
        const std::filesystem::path tier =
            std::filesystem::u8path(corpus::root()) / "common" / "Summer Blast _25 Setlist" / "Tier 2";
        gone = root / "Dance Gavin Dance - Young Robot";
        std::filesystem::copy(tier / "Dance Gavin Dance - Young Robot", gone,
                              std::filesystem::copy_options::recursive);
        std::filesystem::copy(tier / "Thornhill - Limbo", root / "Thornhill - Limbo",
                              std::filesystem::copy_options::recursive);
    }
    ~TwoSongFolders() { testtemp::remove_dir(root.u8string()); }
    // The app's library scan of the root (discover_charts). Fails on any
    // scan error.
    std::vector<app::ScanItem> scan() const {
        auto found = app::discover_charts({root.u8string()});
        REQUIRE_MESSAGE(found.second.empty(), "scan error: " << found.second.front());
        return std::move(found.first);
    }
};

}  // namespace

TEST_CASE("store: a profile the scan's purge empties reads as nothing, and the next open "
          "deletes it") {
    // The user deletes one of two song folders and rescans.
    TwoSongFolders folders("purge_scan");
    const std::vector<app::ScanItem> both = folders.scan();
    REQUIRE(both.size() == 2);
    std::filesystem::remove_all(folders.gone);
    const std::vector<app::ScanItem> after = folders.scan();
    REQUIRE(after.size() == 1);
    const std::string kept = after.front().md5;
    const std::string gone = both[0].md5 == kept ? both[1].md5 : both[0].md5;
    REQUIRE(gone != kept);
    const Profile gone_mode = at4("gone mode");

    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        REQUIRE_FALSE(app::save_scan_as_library(store, both).has_value());
        store.save(prepare_record(gone, gone_mode, fixture().record));
        store.save(prepare_record(kept, at4(), fixture().record));
        REQUIRE(store.lookup_one(gone, gone_mode).status == RecordStatus::Ready);

        // The rescan no longer lists the deleted chart, so its purge takes
        // that chart's only record. No read shows the profile it emptied.
        REQUIRE_FALSE(app::save_scan_as_library(store, after).has_value());
        CHECK(store.lookup_one(gone, gone_mode).status == RecordStatus::NotAnalyzed);
        CHECK(store.settled_song_ids(gone_mode).empty());
        CHECK(store.record_count() == 1);
    }
    { RecordStore reopened(file.path); }
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles WHERE chartmode='" +
                                        gone_mode.chartmode + "'") == 0);
    RecordStore store(file.path);
    CHECK(store.lookup_one(kept, at4()).status == RecordStatus::Ready);
}

TEST_CASE("store: an open while another connection holds the write lock keeps the empty "
          "profiles, and a later open deletes them") {
    const Profile gone_mode = at4("gone mode");
    const Profile kept_mode = at4();
    REQUIRE(gone_mode.chartmode != kept_mode.chartmode);
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        store.rebuild_chart_library({library_row("kept", "Kept")});
        // Not in the library, so the next scan's purge empties its profile.
        store.save(prepare_record("gone", gone_mode, fixture().record));
        store.save(prepare_record("kept", kept_mode, fixture().record));
        store.rebuild_chart_library({library_row("kept", "Kept")});
    }
    // The purge emptied the gone mode's profile; it is still in the file.
    REQUIRE(profiles_in_mode(file.path, gone_mode.chartmode) == 1);
    REQUIRE(profiles_in_mode(file.path, kept_mode.chartmode) == 1);

    // Another connection holds the write lock, as a batch's open save group
    // does; reading the file still works, as WAL allows.
    sqlite3* holder = nullptr;
    REQUIRE(sqlite3_open(file.path.c_str(), &holder) == SQLITE_OK);
    REQUIRE(sqlite3_exec(holder, "BEGIN IMMEDIATE; DELETE FROM meta WHERE key = 'none';", nullptr,
                         nullptr, nullptr) == SQLITE_OK);
    {
        RecordStore store(file.path);
        CHECK(store.lookup_one("kept", kept_mode).status == RecordStatus::Ready);
        CHECK(store.lookup_one("gone", gone_mode).status == RecordStatus::NotAnalyzed);
    }
    sqlite3_exec(holder, "ROLLBACK", nullptr, nullptr, nullptr);
    sqlite3_close(holder);
    // The open under the lock deleted nothing.
    CHECK(profiles_in_mode(file.path, gone_mode.chartmode) == 1);
    CHECK(profiles_in_mode(file.path, kept_mode.chartmode) == 1);

    // A later open deletes the emptied profile and keeps the other.
    { RecordStore reopened(file.path); }
    CHECK(profiles_in_mode(file.path, gone_mode.chartmode) == 0);
    CHECK(profiles_in_mode(file.path, kept_mode.chartmode) == 1);
}

TEST_CASE("store: a record saved under another build's stamp reads back Stale for that build") {
    // The real v2.12.1 library's records were written by another build.
    OldLibrary old("other_build");
    RecordStore store(old.path);
    const Profile profile = OldLibrary::profile();
    const std::string id = OldLibrary::song_ids().front();

    const RecordLookup got = store.lookup_one(id, profile);
    CHECK(got.status == RecordStatus::Stale);
    CHECK(got.stale_build);
    CHECK(store.record_count() == 3);
    // A save of the chart under this build's stamp replaces it (save's purge).
    store.save(prepare_record(id, profile, old_library_analysis()));
    CHECK(store.lookup_one(id, profile).status == RecordStatus::Ready);
    CHECK(store.record_count() == 3);
}

TEST_CASE("store: a save keeps a readable record made under other rules (D51 call 8)") {
    RecordStore store(":memory:");
    const HydraRecord& foreign = other_rules_analysis();
    store.save(prepare_record("h1", at4(), foreign));
    store.save(prepare_record("h2", at4(), fixture().record));
    const RecordLookup got = store.lookup_one("h1", at4());
    CHECK(got.status == RecordStatus::Stale);
    CHECK(got.stale_rules);
    CHECK_FALSE(got.stale_build);
    CHECK(got.record.paths.empty());  // a Stale record's numbers are not handed out

    // Saving the chart under these rules leaves the other rules' record: the
    // current one wins the lookup, and the file still holds both profiles.
    store.save(prepare_record("h1", at4(), fixture().record));
    CHECK(store.lookup_one("h1", at4()).status == RecordStatus::Ready);
    CHECK(store.record_count() == 3);
}

TEST_CASE("store: record_count counts every real record, under any profile, and no failure") {
    RecordStore store(":memory:");
    CHECK(store.record_count() == 0);
    store.save(prepare_record("h1", at4(), fixture().record));
    CHECK(store.record_count() == 1);
    // A save under the same profile replaces the record, so the count holds.
    store.save(prepare_record("h1", at4(), fixture().record));
    CHECK(store.record_count() == 1);
    // The same chart in another mode is a second record; another chart a third.
    store.save(prepare_record("h1", at4("other mode"), fixture().record));
    store.save(prepare_record("h2", at4(), fixture().record));
    CHECK(store.record_count() == 3);
    // A failure is not a real record.
    store.save(failed_chart("h3", kNoNotesChart).prepared);
    CHECK(store.record_count() == 3);
    // A failure that replaces a real record takes it out of the count.
    store.save(failed_chart("h2", kNoNotesChart).prepared);
    CHECK(store.record_count() == 2);
}

// ---- stored failures (REPORT-ALL-T1) ---------------------------------------

TEST_CASE("store: the remembered failures are exactly the four the chart's content causes") {
    // D249 item 1: "Every failure the chart's own content causes (option b):
    // NoNotes, ChartTimingRefused, NoteShuffleFreezes and ChartUnreadable."
    // One real chart fails each way, and its failure is stored and reads back
    // as its kind.
    const std::string edge = corpus::testdata_root() + "/parse_edge/";
    app::AnalysisSettings shuffled = store_fixture_settings();
    shuffled.noteshuffle = true;
    const struct {
        std::string path;
        app::AnalysisSettings settings;
        ErrorKind want;
    } charts[] = {
        {edge + kNoNotesChart, store_fixture_settings(), ErrorKind::NoNotes},
        {edge + "m12_zero_tempo.mid", store_fixture_settings(), ErrorKind::ChartTimingRefused},
        // The chart Clone Hero freezes on with Note Shuffle on
        // (test_song_parse.cpp, "Note Shuffle switch").
        {(sourcetree::root() /
          "docs/audit/note-shuffle/game-tests/songs/NS C1 four colour then snare/notes.mid")
             .u8string(),
         shuffled, ErrorKind::NoteShuffleFreezes},
        {edge + kUnreadableChart, store_fixture_settings(), ErrorKind::ChartUnreadable},
    };
    std::set<ErrorKind> remembered;
    RecordStore store(":memory:");
    for (const auto& c : charts) {
        CAPTURE(c.path);
        const ChartFailure failed = failed_chart_at(c.path, c.path, c.settings, at4());
        CHECK(failed.kind == c.want);
        store.save(failed.prepared);
        const RecordLookup got = store.lookup_one(c.path, at4());
        CHECK(got.status == RecordStatus::CantAnalyze);
        CHECK(got.failure_kind == std::optional<ErrorKind>(c.want));
        remembered.insert(c.want);
    }
    REQUIRE(remembered.size() == 4);

    // Every kind the enum declares, from its first to its last
    // (ErrorKind::PathFileWrite ends the list in error_kind.h).
    for (int i = static_cast<int>(ErrorKind::Cancelled);
         i <= static_cast<int>(ErrorKind::PathFileWrite); ++i) {
        const ErrorKind kind = static_cast<ErrorKind>(i);
        CAPTURE(i);
        const bool want = remembered.count(kind) == 1;
        CHECK(remembered_failure(kind) == want);
        // A remembered kind has a stored name that reads back as itself; any
        // other has none, and failure_record refuses it.
        const char* name = failure_kind_name(kind);
        CHECK((name != nullptr) == want);
        if (name) CHECK(failure_kind_from_name(name) == std::optional<ErrorKind>(kind));
        if (!want) CHECK_THROWS_AS(failure_record("h", at4(), kind, "x"), std::invalid_argument);
    }
    CHECK_FALSE(failure_kind_from_name("NotAKind").has_value());
}

TEST_CASE("store: a chart with no notes fails as NoNotes, shown as NoNotesError's sentence alone") {
    // A real chart with no Expert Pro Drums notes, analyzed at Expert Pro
    // Drums: the analysis throws NoNotesError, and the message the user reads
    // is that error's own sentence for the asked difficulty, with no detail.
    const app::AnalysisSettings settings = store_fixture_settings();
    const std::string path = corpus::testdata_root() + "/parse_edge/" + kNoNotesChart;
    bool threw = false;
    try {
        app::analyze_chart_file(path, settings);
    } catch (const NoNotesError& e) {
        threw = true;
        CHECK(e.kind() == ErrorKind::NoNotes);
        const NoNotesError owner(settings.difficulty, settings.prodrums);
        CHECK(app::plain_error(e) == std::string(owner.what()));
        CHECK(app::plain_error_detail(e).empty());
    }
    CHECK(threw);
}

TEST_CASE("store: a failure saves and reads back CantAnalyze with its kind and text") {
    RecordStore store(":memory:");
    // A real refused chart: the kind and text are what its analysis threw.
    const ChartFailure thrown = failed_chart("h1", kUnreadableChart);
    const PreparedRecord& failed = thrown.prepared;
    // A failure record is no record of numbers: no paths, no stars, no notes.
    CHECK(failed.record.paths.empty());
    CHECK(failed.record.pathcount == 0);
    CHECK_FALSE(failed.record.stars.has_value());
    CHECK(failed.record.notecount == 0);
    store.save(failed);

    const RecordLookup got = store.lookup_one("h1", at4());
    CHECK(got.status == RecordStatus::CantAnalyze);
    CHECK(got.failure_kind == std::optional<ErrorKind>(thrown.kind));
    CHECK(got.failure_text == thrown.text);
    CHECK_FALSE(got.stale_build);
    CHECK_FALSE(got.stale_rules);
    // Only a Ready answer hands out a record.
    CHECK(got.record == RecordRow{});
    // Another profile does not see it.
    CHECK(store.lookup_one("h1", at4("other mode")).status == RecordStatus::NotAnalyzed);
}

TEST_CASE("store: a failure under other rules reads Stale, and one a store with no rules refuses") {
    {
        RecordStore store(":memory:", core::RulesStamp::of(hydra::test::other_rules()));
        store.save(failed_chart("h1", kNoNotesChart).prepared);
        CHECK(store.lookup_one("h1", at4()).status == RecordStatus::CantAnalyze);
    }
    StoreFile file("store_records");
    {
        RecordStore store(file.path, core::RulesStamp::of(hydra::test::other_rules()));
        store.save(failed_chart("h1", kNoNotesChart).prepared);
    }
    {
        RecordStore store(file.path);
        const RecordLookup got = store.lookup_one("h1", at4());
        CHECK(got.status == RecordStatus::Stale);
        CHECK(got.stale_rules);
        CHECK_FALSE(got.stale_build);
    }
    RecordStore none(":memory:", core::RulesStamp::none());
    CHECK_THROWS_AS(none.save(failed_chart("h1", kNoNotesChart).prepared), std::invalid_argument);
}

TEST_CASE("store: a real save replaces a failure, and a failure replaces a real record") {
    RecordStore store(":memory:");
    const ChartFailure unreadable = failed_chart("h1", kUnreadableChart);
    store.save(unreadable.prepared);
    REQUIRE(store.lookup_one("h1", at4()).status == RecordStatus::CantAnalyze);
    CHECK(store.lookup_one("h1", at4()).failure_text == unreadable.text);

    const PreparedRecord real = prepare_record("h1", at4(), fixture().record);
    REQUIRE_FALSE(real.record.paths.empty());
    store.save(real);
    RecordLookup got = store.lookup_one("h1", at4());
    REQUIRE(got.status == RecordStatus::Ready);
    CHECK(got.record == real.record);
    CHECK_FALSE(got.failure_kind.has_value());
    CHECK(got.failure_text.empty());

    const ChartFailure no_notes = failed_chart("h1", kNoNotesChart);
    store.save(no_notes.prepared);
    got = store.lookup_one("h1", at4());
    CHECK(got.status == RecordStatus::CantAnalyze);
    CHECK(got.failure_kind == std::optional<ErrorKind>(no_notes.kind));
    CHECK(got.failure_text == no_notes.text);
    CHECK(got.record.paths.empty());
    // The failure took the real record's place: no real record is left.
    CHECK(store.record_count() == 0);
}

TEST_CASE("store: a failure replaces the real record's path rows in the file") {
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        store.save(prepare_record("h1", at4(), fixture().record));
        REQUIRE(scalar_on_file(file.path, "SELECT COUNT(*) FROM record_paths") > 0);
        // The chart is edited into one with no notes and analyzed again.
        store.save(failed_chart("h1", kNoNotesChart).prepared);
    }
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM record_paths") == 0);
    CHECK(hydra::test::record_rows_in(file.path) == 1);
}

TEST_CASE("store: settled_song_ids lists exactly lookup's Ready and CantAnalyze charts") {
    // The real v2.12.1 library's three records are another build's (Stale).
    OldLibrary old("settled_ids");
    RecordStore store(old.path);
    const Profile profile = OldLibrary::profile();
    const Profile elsewhere_profile{"other mode", profile.cap, profile.lens};
    store.save(prepare_record("ready", profile, old_library_analysis()));
    store.save(failed_chart("failed", kNoNotesChart, profile).prepared);
    store.save(prepare_record("rules", profile, other_rules_analysis(OldLibrary::settings())));
    store.save(failed_chart("elsewhere", kNoNotesChart, elsewhere_profile).prepared);

    std::vector<std::string> ids = OldLibrary::song_ids();
    ids.insert(ids.end(), {"ready", "failed", "rules", "elsewhere", "never"});
    std::unordered_map<std::string, RecordStatus> settled_by_lookup;
    const std::vector<RecordLookup> found = store.lookup(ids, profile);
    for (size_t i = 0; i < ids.size(); ++i)
        if (found[i].status == RecordStatus::Ready || found[i].status == RecordStatus::CantAnalyze)
            settled_by_lookup.emplace(ids[i], found[i].status);
    CHECK(store.settled_song_ids(profile) == settled_by_lookup);
    CHECK(settled_by_lookup == std::unordered_map<std::string, RecordStatus>{
                                   {"ready", RecordStatus::Ready},
                                   {"failed", RecordStatus::CantAnalyze}});
}

TEST_CASE("store: a profile whose only record is a failure is kept") {
    // Two real song folders, scanned the app's way; one chart's only record
    // is a real refusal, in a chart mode of its own.
    TwoSongFolders folders("failure_kept");
    const std::vector<app::ScanItem> scan = folders.scan();
    REQUIRE(scan.size() == 2);
    const std::string failed_id = scan[0].md5;
    const std::string real_id = scan[1].md5;
    const Profile failed_mode = at4("failed mode");
    const Profile real_mode = at4();
    REQUIRE(failed_mode.chartmode != real_mode.chartmode);

    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        REQUIRE_FALSE(app::save_scan_as_library(store, scan).has_value());
        store.save(failed_chart(failed_id, kNoNotesChart, failed_mode).prepared);
        store.save(prepare_record(real_id, real_mode, fixture().record));
        // The rescan's purge keeps both: each chart is still in the library.
        REQUIRE_FALSE(app::save_scan_as_library(store, scan).has_value());
        CHECK(store.lookup_one(failed_id, failed_mode).status == RecordStatus::CantAnalyze);
    }
    // The open's deletion of empty profiles counts a failure as a record.
    RecordStore reopened(file.path);
    CHECK(reopened.lookup_one(failed_id, failed_mode).status == RecordStatus::CantAnalyze);
    CHECK(profiles_in_mode(file.path, failed_mode.chartmode) == 1);
    CHECK(profiles_in_mode(file.path, real_mode.chartmode) == 1);
}

TEST_CASE("store: lookup's three answers against sibling profiles") {
    // The real v2.12.1 library's records are another build's: the "build" chart.
    OldLibrary old("siblings");
    RecordStore store(old.path);
    const Profile profile = OldLibrary::profile();
    const std::string build = OldLibrary::song_ids().front();
    const HydraRecord& analysis = old_library_analysis();
    store.save(prepare_record("ready", profile, analysis));
    store.save(prepare_record("rules", profile, other_rules_analysis(OldLibrary::settings())));
    store.save(prepare_record("elsewhere", Profile{"other mode", profile.cap, profile.lens},
                              analysis));
    store.save(prepare_record("elsewhere",
                              Profile{profile.chartmode, CapQuery::at(8), profile.lens},
                              old_library_analysis(8)));

    const std::vector<RecordLookup> got =
        store.lookup({"ready", build, "rules", "elsewhere", "never", "ready"}, profile);
    REQUIRE(got.size() == 6);
    CHECK(got[0].status == RecordStatus::Ready);
    CHECK(got[1].status == RecordStatus::Stale);
    CHECK(got[1].stale_build);
    CHECK(got[2].status == RecordStatus::Stale);
    CHECK(got[2].stale_rules);
    CHECK_FALSE(got[2].stale_build);
    CHECK(got[3].status == RecordStatus::NotAnalyzed);
    CHECK(got[4].status == RecordStatus::NotAnalyzed);
    CHECK(got[5].status == RecordStatus::Ready);
    CHECK(got[5].record == got[0].record);
    // Another lens at the same cap is not this profile either.
    CHECK(store.lookup_one("ready", Profile{profile.chartmode, profile.cap, Lens::from(20, 0, 4)})
              .status == RecordStatus::NotAnalyzed);
}

TEST_CASE("store: a save inside a save group is one SAVEPOINT; a failed one undoes only its chart") {
    RecordStore store(":memory:");
    // Two path rows at one rank break record_paths' key, so this save fails
    // after its record row went in.
    PreparedRecord broken = prepare_record("broken", at4(), fixture().record);
    broken.record.paths.push_back(broken.record.paths.front());

    store.begin_save_group();
    store.save(prepare_record("kept", at4(), fixture().record));
    // The save fails as the store's own write failure, not any throw.
    bool threw = false;
    try {
        store.save(broken);
    } catch (const KindedError& e) {
        threw = true;
        CHECK_MESSAGE(e.kind() == ErrorKind::DatabaseWrite, e.what());
    }
    CHECK(threw);
    CHECK_FALSE(store.save_group_lost());
    store.save(prepare_record("after", at4(), fixture().record));
    store.commit_save_group();

    CHECK(store.lookup_one("kept", at4()).status == RecordStatus::Ready);
    CHECK(store.lookup_one("after", at4()).status == RecordStatus::Ready);
    CHECK(store.lookup_one("broken", at4()).status == RecordStatus::NotAnalyzed);
}

TEST_CASE("store: a record's best path text is rank 1's, and empty when it kept no paths") {
    CHECK(RecordRow{}.best_path_text().empty());
    // A real analysis with several paths: the text is the engine's rank 1
    // pathstring, not rank 2's.
    const auto [more, fewer] = more_and_fewer_paths();
    const std::vector<const Path*> paths = more.all_paths();
    REQUIRE(paths.size() >= 2);
    REQUIRE(paths[0]->pathstring() != paths[1]->pathstring());
    const RecordRow row = record_of("h1", more);
    CHECK(row.best_path_text() == paths[0]->pathstring());
    CHECK(row.best_path_text() != paths[1]->pathstring());
    // An analysis that kept no paths becomes a record with no text.
    CHECK(record_of("h1", no_paths_at_cap(4)).best_path_text().empty());
}

// ---- the status rules -------------------------------------------------------
//
// Each case pins one rule lookup answers by: which stamps read Ready, why a
// Stale record is Stale, when other rules read Ready again, and what reads as
// Not analyzed.

TEST_CASE("store: every stamp this build accepts reads Ready and a retired stamp reads Stale") {
    // The stamp is not the app version (ADR 0018); kResultsStamp is its one
    // owner. This build writes kResultsStamp.written, and that is a stamp
    // it accepts.
    CHECK(kResultsStamp.is_current(kResultsStamp.written));
    CHECK(current_record_version() == std::string(kResultsStamp.written));

    // Every accepted stamp reads Ready, through lookup and the skip list it
    // backs (settled_song_ids).
    RecordStore store(":memory:");
    int n = 0;
    for (const std::string_view stamp : kResultsStamp.accepted) {
        const std::string stamp_text(stamp);
        CAPTURE(stamp_text);
        const std::string id = "accepted" + std::to_string(n++);
        store.save_under_build_for_test(prepare_record(id, at4(), fixture().record), stamp_text);
        CHECK(store.lookup_one(id, at4()).status == RecordStatus::Ready);
        const auto settled = store.settled_song_ids(at4());
        REQUIRE(settled.count(id) == 1u);
        CHECK(settled.at(id) == RecordStatus::Ready);
    }

    // kOldBuildStamp, a stamp this build does not accept, reads Stale.
    REQUIRE_FALSE(kResultsStamp.is_current(hydra::test::kOldBuildStamp));
    hydra::test::save_old_build_record(store, prepare_record("old", at4(), fixture().record));
    CHECK(store.lookup_one("old", at4()).status == RecordStatus::Stale);
    CHECK(store.settled_song_ids(at4()).count("old") == 0u);

    // A real release's stamp: the v2.12.1 library's profile carries none
    // this build accepts, so its records read Stale and none is on the skip
    // list.
    OldLibrary old("retired_stamp");
    for (const std::string_view stamp : kResultsStamp.accepted) {
        const std::string stamp_text(stamp);
        CAPTURE(stamp_text);
        CHECK(scalar_on_file(old.path, "SELECT COUNT(*) FROM profiles WHERE hyversion='" +
                                           stamp_text + "'") == 0);
    }
    RecordStore old_store(old.path);
    const Profile profile = OldLibrary::profile();
    for (const std::string& id : OldLibrary::song_ids()) {
        CAPTURE(id);
        const RecordLookup got = old_store.lookup_one(id, profile);
        CHECK(got.status == RecordStatus::Stale);
        CHECK(got.stale_build);
    }
    CHECK(old_store.settled_song_ids(profile).empty());
}

TEST_CASE("store: a record made under other rules reads Stale until the store runs those rules") {
    const core::Rules other = hydra::test::other_rules();

    // A store running the default rules sees a record made under other rules
    // as Stale everywhere a lookup can ask.
    {
        RecordStore store(":memory:");
        store.save(prepare_record("h", at4(), other_rules_analysis()));
        CHECK(store.lookup_one("h", at4()).status == RecordStatus::Stale);
        CHECK(store.settled_song_ids(at4()).empty());
    }

    // A record made under the default rules reads Stale to a store running
    // other rules, and Ready again once the rules match.
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        store.save(prepare_record("h", at4(), fixture().record));
        CHECK(store.lookup_one("h", at4()).status == RecordStatus::Ready);
    }
    {
        RecordStore store(file.path, core::RulesStamp::of(other));
        const RecordLookup got = store.lookup_one("h", at4());
        CHECK(got.status == RecordStatus::Stale);
        CHECK(got.stale_rules);
        CHECK(store.settled_song_ids(at4()).empty());
    }
    {
        // A store gated on "no usable rules" (a bad hydra_rules.ini) reads
        // nothing as Ready.
        RecordStore store(file.path, core::RulesStamp::none());
        CHECK(store.lookup_one("h", at4()).status == RecordStatus::Stale);
    }
    {
        RecordStore store(file.path);
        CHECK(store.lookup_one("h", at4()).status == RecordStatus::Ready);
    }
}

TEST_CASE("store: the other rules' record a save kept reads Ready under those rules") {
    // D51 call 8 (finding 65): a result made under other rules is kept, and
    // reads Ready again, with its own numbers, once the store runs those rules.
    const core::Rules other = hydra::test::other_rules();
    const PreparedRecord foreign = prepare_record("h", at4(), other_rules_analysis());
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        store.save(foreign);
    }
    {
        RecordStore store(file.path);
        store.save(prepare_record("h", at4(), fixture().record));
        CHECK(store.record_count() == 2);
        CHECK(store.lookup_one("h", at4()).status == RecordStatus::Ready);
    }
    {
        RecordStore store(file.path, core::RulesStamp::of(other));
        const RecordLookup got = store.lookup_one("h", at4());
        CHECK(got.status == RecordStatus::Ready);
        CHECK(got.record == foreign.record);
        CHECK(store.settled_song_ids(at4()) ==
              std::unordered_map<std::string, RecordStatus>{{"h", RecordStatus::Ready}});
    }
}

TEST_CASE("store: a Stale lookup says why (another build or other rules), a Ready one nothing") {
    // The real v2.12.1 library's records are another build's, made under the
    // default rules: today's default rules have the fingerprint text that
    // build had (core/rules.cpp, rules_fields).
    OldLibrary old("stale_why");
    RecordStore store(old.path);
    const Profile profile = OldLibrary::profile();
    const std::string build = OldLibrary::song_ids().front();
    store.save(prepare_record("rules", profile, other_rules_analysis(OldLibrary::settings())));
    store.save(prepare_record("ready", profile, old_library_analysis()));

    // This build, other rules.
    const RecordLookup by_rules = store.lookup_one("rules", profile);
    CHECK(by_rules.status == RecordStatus::Stale);
    CHECK(by_rules.stale_rules);
    CHECK_FALSE(by_rules.stale_build);

    // Another build, these rules.
    const RecordLookup by_build = store.lookup_one(build, profile);
    CHECK(by_build.status == RecordStatus::Stale);
    CHECK(by_build.stale_build);
    CHECK_FALSE(by_build.stale_rules);

    // A Ready record carries no reason.
    const RecordLookup ready = store.lookup_one("ready", profile);
    CHECK(ready.status == RecordStatus::Ready);
    CHECK_FALSE(ready.stale_build);
    CHECK_FALSE(ready.stale_rules);
}

// The fill of the saved hardest figure replays the rows the store picks
// (rows_missing_hardest). With every figure emptied, as an older build left
// it, the store must pick a row exactly when the engine's own figure
// (engine_hardest_ms) gives its path one: a row it skips keeps an empty
// figure for good, and a row with no figure costs a replay for nothing. The
// charts are the four V1 read the figure on. Allies' paths hold squeeze-outs
// and Trippin's early fills, so between them the store must pick rows that
// hold no squeeze-in.
TEST_CASE("store: rows missing the hardest figure are exactly the paths the engine gives one") {
    const app::Settings settings;
    const Profile profile = settings.profile();
    const std::vector<std::string> names = {
        "Evans Blue - Beg", "Mutemath - Allies", "Green Day - Burnout",
        "Trippin on a Hole in a Paper Heart"};

    std::vector<ChartLibraryEntry> library;
    std::vector<PreparedRecord> saves;
    // Every saved row, by chart and rank, and whether the engine gives its
    // path a figure.
    struct SavedRow {
        std::string song_id;
        int rank = 0;
        bool has_figure = false;
    };
    std::vector<SavedRow> rows;
    int figure_without_squeeze_in = 0;
    for (const std::string& name : names) {
        CAPTURE(name);
        const std::string file = corpus::chart_named(name);
        REQUIRE_FALSE(file.empty());
        const ChartLibraryEntry entry = corpus::scanned_entry(file, name);
        const HydraRecord& analysis = corpus::analyzed(file, settings.to_analysis_settings());
        const std::vector<const Path*> engine_paths = analysis.all_paths();
        PreparedRecord prepared = prepare_record(entry.md5, profile, analysis);
        REQUIRE_FALSE(prepared.record.paths.empty());
        for (PathRow& p : prepared.record.paths) {
            REQUIRE(static_cast<size_t>(p.rank) <= engine_paths.size());
            const bool has_figure =
                engine_hardest_ms(*engine_paths[static_cast<size_t>(p.rank - 1)]).has_value();
            rows.push_back({entry.md5, p.rank, has_figure});
            if (has_figure && p.sqin_count == 0) ++figure_without_squeeze_in;
            p.hardest_ms.reset();
        }
        library.push_back(entry);
        saves.push_back(std::move(prepared));
    }
    // Without such a row the case could not tell the select from one that
    // asks only for a squeeze-in.
    REQUIRE(figure_without_squeeze_in > 0);

    RecordStore store(":memory:");
    store.rebuild_chart_library(library);
    for (const PreparedRecord& p : saves) {
        store.save(p);
        REQUIRE(store.lookup_one(p.record.song_id, profile).status == RecordStatus::Ready);
    }

    std::set<std::pair<std::string, int>> picked;
    for (const MissingHardestRow& row : store.rows_missing_hardest(profile))
        picked.insert({row.song_id, row.rank});
    size_t with_figure = 0;
    for (const SavedRow& row : rows) {
        CAPTURE(row.song_id);
        CAPTURE(row.rank);
        CHECK(picked.count({row.song_id, row.rank}) == (row.has_figure ? 1u : 0u));
        if (row.has_figure) ++with_figure;
    }
    // And the store picks no row beyond the ones saved here.
    CHECK(picked.size() == with_figure);
}
