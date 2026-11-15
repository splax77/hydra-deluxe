// The store's one save and one lookup (D115): prepare_record, failure_record,
// RecordStore::save, lookup, lookup_one, settled_song_ids, ready_song_ids and
// ready_records, the status rules a
// lookup answers by (which stamps read Ready, why a record is Stale, what reads
// Not analyzed), the store's reader (RecordStore::reader, StoreReader), plus
// the path-row size measured on the corpus. How saved
// records read back across caps, lenses and the corpus is in
// test_store_results.cpp.
#include "doctest.h"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <unordered_set>
#include <vector>

#include <sqlite3.h>

#include "app/user_messages.h"  // plain_error, plain_error_detail
#include "core/error_kind.h"
#include "core/model.h"
#include "core/rules.h"  // RulesStamp
#include "core/winstr.h"  // os_path
#include "corpus_util.h"
#include "db_file_util.h"  // scalar_on_file, exec_on_file
#include "display_fixtures.h"  // save_old_build_record, kOldBuildStamp, other_rules_record
#include "library_fixtures.h"  // library_row
#include "parse/song.h"
#include "store/record_store.h"
#include "store/stored_versions.h"  // kResultsStamp
#include "store_file_fixtures.h"  // StoreFile, HeldSaveGroup
#include "store_fixtures.h"  // fixture, at_cap, at4, no_paths_at_cap
#include "temp_util.h"
#include "wait_util.h"  // wait_until, WaitTimeout

using namespace hydra;
using namespace hydra::store;

using hydra::test::HeldSaveGroup;
using hydra::test::StoreFile;
using hydra::test::at4;
using hydra::test::at_cap;
using hydra::test::fixture;
using hydra::test::no_paths_at_cap;
using hydra::test::scalar_on_file;

namespace {

// Every corpus chart the engine analyzes, under the default search settings.
std::vector<const HydraRecord*> corpus_analyses() {
    std::vector<const HydraRecord*> out;
    for (const std::string& path : corpus::chart_paths()) {
        if (corpus::song(path, ChartReading{}).is_empty()) continue;
        try {
            out.push_back(&corpus::analyzed(path, SearchSettings{}));
        } catch (const ChartFileError&) {
            // a chart the engine rejects has no record to save
        }
    }
    return out;
}

// Whether two lookups gave the same answer, field by field: RecordLookup has
// no operator==.
bool same_answer(const RecordLookup& a, const RecordLookup& b) {
    return a.status == b.status && a.stale_build == b.stale_build &&
           a.stale_rules == b.stale_rules && a.record == b.record &&
           a.failure_kind == b.failure_kind && a.failure_text == b.failure_text;
}

// The four charts the reader tests ask about: a Ready chart, a failure
// record, a record saved under another build, and a chart with no record.
const std::vector<std::string>& four_chart_ids() {
    static const std::vector<std::string> ids = {"ready", "failed", "build", "never"};
    return ids;
}
void save_four_charts(RecordStore& store) {
    store.save(prepare_record("ready", at4(), fixture().record));
    store.save(failure_record("failed", at4(), ErrorKind::NoNotes, "No notes."));
    store.save_under_build_for_test(prepare_record("build", at4(), fixture().record),
                                    hydra::test::kOldBuildStamp);
}

// The store's reader gives the store's own answers for the four charts, by
// lookup, lookup_one and settled_song_ids. The expected values are the
// writer's.
void check_reader_answers_as_store(RecordStore& store) {
    const std::vector<std::string>& ids = four_chart_ids();
    StoreReads& reader = store.reader();
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

// Whether `path` can be deleted now, which on Windows means no connection
// holds it open.
bool can_delete(const std::string& path) {
    std::error_code ec;
    const bool removed = std::filesystem::remove(std::filesystem::u8path(path), ec);
    return removed && !ec;
}

}  // namespace

TEST_CASE("the store's reader answers every lookup as the store does") {
    RecordStore store(":memory:");
    save_four_charts(store);
    check_reader_answers_as_store(store);
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
    const PreparedRecord first = prepare_record("h1", at4(), fixture().record);
    store.save(first);
    PreparedRecord second = first;
    second.record.paths.resize(1);
    second.record.paths.front().path = "second";
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
    // The library lists the four charts first, since a rescan deletes the
    // results of charts it does not list.
    std::vector<ChartLibraryEntry> library;
    for (const std::string& id : four_chart_ids()) library.push_back(library_row(id, id));
    store.rebuild_chart_library(library);
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

TEST_CASE("store: a saved analysis reads back Ready with its paths in rank order") {
    const HydraRecord& analysis = fixture().record;
    RecordStore store(":memory:");
    const PreparedRecord prepared = prepare_record("h1", at4(), analysis);
    store.save(prepared);

    const RecordLookup got = store.lookup_one("h1", at4());
    REQUIRE(got.status == RecordStatus::Ready);
    CHECK(got.record == prepared.record);
    REQUIRE_FALSE(got.record.paths.empty());
    for (size_t i = 0; i < got.record.paths.size(); ++i)
        CHECK(got.record.paths[i].rank == static_cast<int>(i + 1));
    CHECK(got.record.paths.size() <= static_cast<size_t>(kPathsPerRecord));
    CHECK(got.record.pathcount == static_cast<int>(analysis.all_paths().size()));
    // Rank 1 is the engine's best path, and the summary reads its score.
    const Path& best = analysis.best_path();
    CHECK(got.record.best_path_text() == best.pathstring());
    CHECK(got.record.summary().score == std::optional<int64_t>(best.totalscore()));
    CHECK(got.record.paths.front().optimal);
}

TEST_CASE("store: prepare_record refuses a profile the analysis did not run under") {
    const HydraRecord& analysis = fixture().record;
    CHECK_THROWS_AS(prepare_record("h1", Profile{"mode", CapQuery::at(8), Lens{}}, analysis),
                    std::invalid_argument);
    CHECK_THROWS_AS(prepare_record("h1", Profile{"mode", CapQuery::at(4), Lens::from(20, 0, 0)},
                                   analysis),
                    std::invalid_argument);
    CHECK_THROWS_AS(prepare_record("h1",
                                   Profile{"mode", CapQuery::at(4), Lens::from(std::nullopt, 0, 0, true)},
                                   analysis),
                    std::invalid_argument);
}

TEST_CASE("store: prepare_record refuses a profile at another early-fill window") {
    // A record searched at the default window, filed under a profile at the
    // widest the setting allows.
    const HydraRecord& analysis = fixture().record;
    REQUIRE(analysis.early_fill_window_ms == kDefaultEarlyFillWindowMs);
    Profile at_85 = at4();
    at_85.lens.efill_window_ms = static_cast<int>(kHitWindowMs);
    CHECK_THROWS_AS(prepare_record("h1", at_85, analysis), std::invalid_argument);
    // And the other way: a record searched at 85 under the default profile.
    HydraRecord searched_at_85 = at_cap(4);
    searched_at_85.early_fill_window_ms = kHitWindowMs;
    CHECK_THROWS_AS(prepare_record("h1", at4(), searched_at_85), std::invalid_argument);
    // The matching pair files.
    CHECK(prepare_record("h1", at_85, searched_at_85).profile.lens.efill_window_ms ==
          static_cast<int>(kHitWindowMs));
}

TEST_CASE("store: one chart's results at two early-fill windows are two results") {
    // Two profiles that differ only in the window. Each save keeps the other's
    // record, and each lookup gets its own back.
    const Profile at_default = at4();
    Profile at_85 = at4();
    at_85.lens.efill_window_ms = static_cast<int>(kHitWindowMs);
    REQUIRE(at_default != at_85);
    HydraRecord searched_at_85 = no_paths_at_cap(4);
    searched_at_85.early_fill_window_ms = kHitWindowMs;
    const PreparedRecord saved_default = prepare_record("h1", at_default, at_cap(4));
    const PreparedRecord saved_85 = prepare_record("h1", at_85, searched_at_85);
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
    // The record keeps the note count once; this is why that loses nothing.
    int checked = 0;
    for (const HydraRecord* analysis : corpus_analyses()) {
        const std::vector<const Path*> paths = analysis->all_paths();
        for (const Path* p : paths) CHECK(p->notecount == paths.front()->notecount);
        if (paths.size() > 1) ++checked;
    }
    CHECK(checked > 0);
}

TEST_CASE("store: a save under the same profile replaces the record and every path") {
    RecordStore store(":memory:");
    PreparedRecord first = prepare_record("h1", at4(), fixture().record);
    // Five kept paths the first time, so none may linger after a smaller save.
    first.record.paths.clear();
    for (int rank = 1; rank <= kPathsPerRecord; ++rank) {
        PathRow p;
        p.rank = rank;
        p.path = "p" + std::to_string(rank);
        p.score = 1000 - rank;
        first.record.paths.push_back(p);
    }
    store.save(first);
    REQUIRE(store.lookup_one("h1", at4()).record.paths.size() ==
            static_cast<size_t>(kPathsPerRecord));

    PreparedRecord second = first;
    second.record.paths.resize(2);
    store.save(second);
    const RecordLookup got = store.lookup_one("h1", at4());
    REQUIRE(got.status == RecordStatus::Ready);
    CHECK(got.record == second.record);
}

TEST_CASE("store: a lookup after a second save reads the second record, not the first") {
    // lookup keeps its reads compiled between calls, so this pins that a
    // reused read carries no row or binding over from the last one.
    RecordStore store(":memory:");
    const PreparedRecord first = prepare_record("h1", at4(), fixture().record);
    store.save(first);
    const RecordLookup got_first = store.lookup_one("h1", at4());
    REQUIRE(got_first.status == RecordStatus::Ready);
    CHECK(got_first.record == first.record);

    PreparedRecord second = first;
    second.record.pathcount = first.record.pathcount + 1;
    second.record.notecount = first.record.notecount + 1;
    second.record.paths.resize(1);
    second.record.paths.front().path = "second";
    store.save(second);
    const RecordLookup got_second = store.lookup_one("h1", at4());
    REQUIRE(got_second.status == RecordStatus::Ready);
    CHECK(got_second.record == second.record);
    // Another chart through the same reads: nothing of h1's comes back.
    CHECK(store.lookup_one("h2", at4()).status == RecordStatus::NotAnalyzed);
}

TEST_CASE("store: a save purges the chart's unreadable records in every mode, and the next open "
          "deletes their emptied profiles") {
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        // An old build's record in another mode, then a save in this one.
        hydra::test::save_old_build_record(
            store, prepare_record("h1", at4("other mode"), fixture().record));
        REQUIRE(store.lookup_one("h1", at4("other mode")).status == RecordStatus::Stale);
        store.save(prepare_record("h1", at4(), fixture().record));

        CHECK(store.lookup_one("h1", at4("other mode")).status == RecordStatus::NotAnalyzed);
        CHECK(store.lookup_one("h1", at4()).status == RecordStatus::Ready);
        CHECK(store.record_count() == 1);
        // The save leaves the old build's emptied profile in place.
        CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles WHERE hyversion='0.0.0'") ==
              1);
    }
    // Closing alone deletes nothing; the next open does.
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles WHERE hyversion='0.0.0'") == 1);
    { RecordStore reopened(file.path); }
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 1);
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles WHERE hyversion='0.0.0'") == 0);
}

TEST_CASE("store: a profile the scan's purge empties reads as nothing, and the next open "
          "deletes it") {
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        store.rebuild_chart_library({library_row("gone", "Gone"), library_row("kept", "Kept")});
        store.save(prepare_record("gone", at4("gone mode"), fixture().record));
        store.save(prepare_record("kept", at4(), fixture().record));
        REQUIRE(store.lookup_one("gone", at4("gone mode")).status == RecordStatus::Ready);

        // The library no longer lists "gone", so the scan's purge takes its
        // only record and leaves its profile empty.
        store.rebuild_chart_library({library_row("kept", "Kept")});
        CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 2);
        // No read shows the empty profile.
        CHECK(store.lookup_one("gone", at4("gone mode")).status == RecordStatus::NotAnalyzed);
        CHECK(store.settled_song_ids(at4("gone mode")).empty());
        CHECK(store.ready_song_ids(at4("gone mode")).empty());
        CHECK(store.ready_records(at4("gone mode")).empty());
        CHECK(store.record_count() == 1);
    }
    { RecordStore reopened(file.path); }
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 1);
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles WHERE chartmode='gone mode'") ==
          0);
    RecordStore store(file.path);
    CHECK(store.lookup_one("kept", at4()).status == RecordStatus::Ready);
}

TEST_CASE("store: an open while another connection holds the write lock keeps the empty "
          "profiles, and a later open deletes them") {
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        store.rebuild_chart_library({library_row("kept", "Kept")});
        // Not in the library, so the next scan's purge empties its profile.
        store.save(prepare_record("gone", at4("gone mode"), fixture().record));
        store.save(prepare_record("kept", at4(), fixture().record));
        store.rebuild_chart_library({library_row("kept", "Kept")});
    }
    REQUIRE(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 2);

    // Another connection holds the write lock, as a batch's open save group
    // does; reading the file still works, as WAL allows.
    sqlite3* holder = nullptr;
    REQUIRE(sqlite3_open(file.path.c_str(), &holder) == SQLITE_OK);
    REQUIRE(sqlite3_exec(holder, "BEGIN IMMEDIATE; DELETE FROM meta WHERE key = 'none';", nullptr,
                         nullptr, nullptr) == SQLITE_OK);
    {
        RecordStore store(file.path);
        CHECK(store.lookup_one("kept", at4()).status == RecordStatus::Ready);
        CHECK(store.lookup_one("gone", at4("gone mode")).status == RecordStatus::NotAnalyzed);
    }
    sqlite3_exec(holder, "ROLLBACK", nullptr, nullptr, nullptr);
    sqlite3_close(holder);
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 2);

    { RecordStore reopened(file.path); }
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 1);
}

TEST_CASE("store: a record saved under another build's stamp reads back Stale for that build") {
    RecordStore store(":memory:");
    const PreparedRecord prepared = prepare_record("h1", at4(), fixture().record);
    store.save_under_build_for_test(prepared, hydra::test::kOldBuildStamp);

    const RecordLookup got = store.lookup_one("h1", at4());
    CHECK(got.status == RecordStatus::Stale);
    CHECK(got.stale_build);
    CHECK_FALSE(got.stale_rules);
    CHECK(store.record_count() == 1);
    // A save of the chart under this build's stamp replaces it (save's purge).
    store.save(prepared);
    CHECK(store.lookup_one("h1", at4()).status == RecordStatus::Ready);
    CHECK(store.record_count() == 1);
}

TEST_CASE("store: a save keeps a readable record made under other rules (D51 call 8)") {
    RecordStore store(":memory:");
    const HydraRecord foreign = hydra::test::other_rules_record(fixture().record);
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
    store.save(failure_record("h3", at4(), ErrorKind::NoNotes, "No notes."));
    CHECK(store.record_count() == 3);
    // A failure that replaces a real record takes it out of the count.
    store.save(failure_record("h2", at4(), ErrorKind::NoNotes, "No notes."));
    CHECK(store.record_count() == 2);
}

// ---- stored failures (REPORT-ALL-T1) ---------------------------------------

TEST_CASE("store: the remembered failures are exactly the four the chart's content causes") {
    // The user's answer 1 (2026-10-15).
    const std::set<ErrorKind> remembered = {ErrorKind::NoNotes, ErrorKind::ChartTimingRefused,
                                            ErrorKind::NoteShuffleFreezes,
                                            ErrorKind::ChartUnreadable};
    for (ErrorKind kind :
         {ErrorKind::Cancelled, ErrorKind::DatabaseOpen, ErrorKind::DatabaseUpgrade,
          ErrorKind::OldLibraryRefused, ErrorKind::OldLibrarySetAsideFailed,
          ErrorKind::DatabaseWrite, ErrorKind::DatabaseRead, ErrorKind::SongFileMissing,
          ErrorKind::HashFailed, ErrorKind::ChartUnreadable, ErrorKind::ChartTimingRefused,
          ErrorKind::NoteShuffleFreezes, ErrorKind::NoNotes, ErrorKind::AlreadyPlain,
          ErrorKind::SearchBroken, ErrorKind::NetUnreachable, ErrorKind::NetTimeout,
          ErrorKind::NetHttpStatus, ErrorKind::NetBadReply, ErrorKind::NoScores,
          ErrorKind::NoRecords, ErrorKind::ReportWrite, ErrorKind::RulesFile,
          ErrorKind::SettingsFile, ErrorKind::AudioDecode, ErrorKind::PreviewAssets,
          ErrorKind::UpdateUnverified, ErrorKind::PathTextRefused, ErrorKind::PathFileRefused,
          ErrorKind::PathFileUnreadable, ErrorKind::PathFileWrite}) {
        CAPTURE(static_cast<int>(kind));
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

TEST_CASE("store: a chart with no notes throws NoNotes, worded as its own sentence") {
    try {
        throw NoNotesError(Difficulty::Expert, true);
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::NoNotes);
        CHECK(app::plain_error(e) == "No Expert Pro Drums notes in this chart.");
        CHECK(app::plain_error_detail(e).empty());
    }
}

TEST_CASE("store: a failure saves and reads back CantAnalyze with its kind and text") {
    RecordStore store(":memory:");
    const PreparedRecord failed =
        failure_record("h1", at4(), ErrorKind::ChartTimingRefused, "line 12: bad tempo");
    // A failure record is no record of numbers: no paths, no stars, no notes.
    CHECK(failed.record.paths.empty());
    CHECK(failed.record.pathcount == 0);
    CHECK_FALSE(failed.record.stars.has_value());
    CHECK(failed.record.notecount == 0);
    store.save(failed);

    const RecordLookup got = store.lookup_one("h1", at4());
    CHECK(got.status == RecordStatus::CantAnalyze);
    CHECK(got.failure_kind == std::optional<ErrorKind>(ErrorKind::ChartTimingRefused));
    CHECK(got.failure_text == "line 12: bad tempo");
    CHECK_FALSE(got.stale_build);
    CHECK_FALSE(got.stale_rules);
    // Only a Ready answer hands out a record.
    CHECK(got.record == RecordRow{});
    // Another profile does not see it.
    CHECK(store.lookup_one("h1", at4("other mode")).status == RecordStatus::NotAnalyzed);
}

TEST_CASE("store: a failure saved under another build's stamp reads Stale") {
    RecordStore store(":memory:");
    store.save_under_build_for_test(failure_record("h1", at4(), ErrorKind::NoNotes, "No notes."),
                                    hydra::test::kOldBuildStamp);
    const RecordLookup got = store.lookup_one("h1", at4());
    CHECK(got.status == RecordStatus::Stale);
    CHECK(got.stale_build);
    CHECK_FALSE(got.stale_rules);
    CHECK_FALSE(got.failure_kind.has_value());
    CHECK(got.failure_text.empty());
    CHECK(store.settled_song_ids(at4()).empty());
}

TEST_CASE("store: a failure under other rules reads Stale, and one a store with no rules refuses") {
    {
        RecordStore store(":memory:", core::RulesStamp{core::default_stamp().fixed + 1});
        store.save(failure_record("h1", at4(), ErrorKind::NoNotes, "No notes."));
        CHECK(store.lookup_one("h1", at4()).status == RecordStatus::CantAnalyze);
    }
    StoreFile file("store_records");
    {
        RecordStore store(file.path, core::RulesStamp{core::default_stamp().fixed + 1});
        store.save(failure_record("h1", at4(), ErrorKind::NoNotes, "No notes."));
    }
    {
        RecordStore store(file.path);
        const RecordLookup got = store.lookup_one("h1", at4());
        CHECK(got.status == RecordStatus::Stale);
        CHECK(got.stale_rules);
        CHECK_FALSE(got.stale_build);
    }
    RecordStore none(":memory:", core::RulesStamp::none());
    CHECK_THROWS_AS(none.save(failure_record("h1", at4(), ErrorKind::NoNotes, "No notes.")),
                    std::invalid_argument);
}

TEST_CASE("store: a real save replaces a failure, and a failure replaces a real record") {
    RecordStore store(":memory:");
    store.save(failure_record("h1", at4(), ErrorKind::ChartUnreadable, "expected a [section]"));
    REQUIRE(store.lookup_one("h1", at4()).status == RecordStatus::CantAnalyze);

    const PreparedRecord real = prepare_record("h1", at4(), fixture().record);
    REQUIRE_FALSE(real.record.paths.empty());
    store.save(real);
    RecordLookup got = store.lookup_one("h1", at4());
    REQUIRE(got.status == RecordStatus::Ready);
    CHECK(got.record == real.record);
    CHECK_FALSE(got.failure_kind.has_value());
    CHECK(got.failure_text.empty());

    store.save(failure_record("h1", at4(), ErrorKind::NoNotes, "No notes."));
    got = store.lookup_one("h1", at4());
    CHECK(got.status == RecordStatus::CantAnalyze);
    CHECK(got.failure_kind == std::optional<ErrorKind>(ErrorKind::NoNotes));
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
        store.save(failure_record("h1", at4(), ErrorKind::NoNotes, "No notes."));
    }
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM record_paths") == 0);
    CHECK(hydra::test::record_rows_in(file.path) == 1);
}

TEST_CASE("store: settled_song_ids lists exactly lookup's Ready and CantAnalyze charts") {
    RecordStore store(":memory:");
    const HydraRecord& analysis = fixture().record;
    store.save(prepare_record("ready", at4(), analysis));
    store.save(prepare_record("empty", at4(), no_paths_at_cap(4)));
    store.save(failure_record("failed", at4(), ErrorKind::NoNotes, "No notes."));
    hydra::test::save_old_build_record(store, prepare_record("stale", at4(), analysis));
    store.save_under_build_for_test(
        failure_record("stale failure", at4(), ErrorKind::NoNotes, "No notes."),
        hydra::test::kOldBuildStamp);
    store.save(prepare_record("rules", at4(), hydra::test::other_rules_record(analysis)));
    store.save(failure_record("elsewhere", at4("other mode"), ErrorKind::NoNotes, "No notes."));

    const std::vector<std::string> ids = {"ready",  "empty", "failed",    "stale",
                                          "stale failure", "rules", "elsewhere", "never"};
    std::unordered_map<std::string, RecordStatus> settled_by_lookup;
    const std::vector<RecordLookup> found = store.lookup(ids, at4());
    for (size_t i = 0; i < ids.size(); ++i)
        if (found[i].status == RecordStatus::Ready || found[i].status == RecordStatus::CantAnalyze)
            settled_by_lookup.emplace(ids[i], found[i].status);
    CHECK(store.settled_song_ids(at4()) == settled_by_lookup);
    CHECK(settled_by_lookup == std::unordered_map<std::string, RecordStatus>{
                                   {"ready", RecordStatus::Ready},
                                   {"empty", RecordStatus::Ready},
                                   {"failed", RecordStatus::CantAnalyze}});
    // ready_song_ids and ready_records leave the failure out.
    CHECK(store.ready_song_ids(at4()) == std::unordered_set<std::string>{"ready", "empty"});
    CHECK(store.ready_records(at4()).size() == 2);
}

TEST_CASE("store: a profile whose only record is a failure is kept") {
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        const std::vector<ChartLibraryEntry> library = {library_row("failed", "Failed"),
                                                        library_row("real", "Real")};
        store.rebuild_chart_library(library);
        store.save(failure_record("failed", at4("failed mode"), ErrorKind::NoNotes, "No notes."));
        store.save(prepare_record("real", at4(), fixture().record));
        store.rebuild_chart_library(library);
        CHECK(store.lookup_one("failed", at4("failed mode")).status == RecordStatus::CantAnalyze);
    }
    // The open's deletion of empty profiles counts a failure as a record.
    RecordStore reopened(file.path);
    CHECK(reopened.lookup_one("failed", at4("failed mode")).status == RecordStatus::CantAnalyze);
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 2);
}

TEST_CASE("store: a failure of a kind this build cannot name reads Stale for another build") {
    StoreFile file("store_records");
    {
        RecordStore store(file.path);
        store.save(failure_record("h1", at4(), ErrorKind::NoNotes, "No notes."));
    }
    // What a later build that remembers one more kind would leave.
    hydra::test::exec_on_file(file.path, "UPDATE records SET failure_kind = 'LaterKind'");
    RecordStore store(file.path);
    const RecordLookup got = store.lookup_one("h1", at4());
    CHECK(got.status == RecordStatus::Stale);
    CHECK(got.stale_build);
    CHECK_FALSE(got.failure_kind.has_value());
    CHECK(store.settled_song_ids(at4()).empty());
}

TEST_CASE("store: lookup's three answers against sibling profiles") {
    RecordStore store(":memory:");
    const HydraRecord& analysis = fixture().record;
    store.save(prepare_record("ready", at4(), analysis));
    hydra::test::save_old_build_record(store, prepare_record("build", at4(), analysis));
    store.save(prepare_record("rules", at4(), hydra::test::other_rules_record(analysis)));
    store.save(prepare_record("elsewhere", at4("other mode"), analysis));
    store.save(prepare_record("elsewhere", Profile{"mode", CapQuery::at(8), Lens{}}, at_cap(8)));

    const std::vector<RecordLookup> got =
        store.lookup({"ready", "build", "rules", "elsewhere", "never", "ready"}, at4());
    REQUIRE(got.size() == 6);
    CHECK(got[0].status == RecordStatus::Ready);
    CHECK(got[1].status == RecordStatus::Stale);
    CHECK(got[1].stale_build);
    CHECK_FALSE(got[1].stale_rules);
    CHECK(got[2].status == RecordStatus::Stale);
    CHECK(got[2].stale_rules);
    CHECK_FALSE(got[2].stale_build);
    CHECK(got[3].status == RecordStatus::NotAnalyzed);
    CHECK(got[4].status == RecordStatus::NotAnalyzed);
    CHECK(got[5].status == RecordStatus::Ready);
    CHECK(got[5].record == got[0].record);
    // Another lens at the same cap is not this profile either.
    CHECK(store.lookup_one("ready", Profile{"mode", CapQuery::at(4), Lens::from(20, 0, 0)}).status ==
          RecordStatus::NotAnalyzed);
}

TEST_CASE("store: ready_song_ids and ready_records list exactly lookup's Ready charts") {
    RecordStore store(":memory:");
    const HydraRecord& analysis = fixture().record;
    PreparedRecord low = prepare_record("low", at4(), analysis);
    low.record.paths.front().score -= 100;
    store.save(low);
    store.save(prepare_record("high", at4(), analysis));
    store.save(prepare_record("empty", at4(), no_paths_at_cap(4)));
    hydra::test::save_old_build_record(store, prepare_record("stale", at4(), analysis));
    store.save(prepare_record("rules", at4(), hydra::test::other_rules_record(analysis)));

    const std::vector<std::string> ids = {"low", "high", "empty", "stale", "rules", "never"};
    std::unordered_set<std::string> ready_by_lookup;
    const std::vector<RecordLookup> found = store.lookup(ids, at4());
    for (size_t i = 0; i < ids.size(); ++i)
        if (found[i].status == RecordStatus::Ready) ready_by_lookup.insert(ids[i]);
    CHECK(store.ready_song_ids(at4()) == ready_by_lookup);
    CHECK(ready_by_lookup == std::unordered_set<std::string>{"low", "high", "empty"});

    const std::vector<RecordRow> listed = store.ready_records(at4());
    REQUIRE(listed.size() == 3);
    CHECK(listed[0].song_id == "high");
    CHECK(listed[1].song_id == "low");
    CHECK(listed[2].song_id == "empty");
    CHECK(listed[1] == low.record);
}

TEST_CASE("store: a save inside a save group is one SAVEPOINT; a failed one undoes only its chart") {
    RecordStore store(":memory:");
    // Two path rows at one rank break record_paths' key, so this save fails
    // after its record row went in.
    PreparedRecord broken = prepare_record("broken", at4(), fixture().record);
    broken.record.paths.push_back(broken.record.paths.front());

    store.begin_save_group();
    store.save(prepare_record("kept", at4(), fixture().record));
    CHECK_THROWS(store.save(broken));
    CHECK_FALSE(store.save_group_lost());
    store.save(prepare_record("after", at4(), fixture().record));
    store.commit_save_group();

    CHECK(store.lookup_one("kept", at4()).status == RecordStatus::Ready);
    CHECK(store.lookup_one("after", at4()).status == RecordStatus::Ready);
    CHECK(store.lookup_one("broken", at4()).status == RecordStatus::NotAnalyzed);
}

TEST_CASE("store: path rows measured on the corpus") {
    // Every corpus chart under the default profile, saved through save into
    // a file. The sizes come from compacted copies (VACUUM INTO): with the
    // path rows (A), without them (B), and without the records too (C). The
    // vendored SQLite has no dbstat, so a size is a difference of files.
    // Nothing here is pinned but the read-back; the numbers are printed.
    StoreFile file("store_records");
    constexpr int kCopies = 10;
    int64_t records = 0, path_rows = 0;
    {
        RecordStore store(file.path);
        std::vector<PreparedRecord> saved;
        for (const HydraRecord* analysis : corpus_analyses()) {
            REQUIRE(analysis->sp_cap.has_value());
            const Profile profile{"mode", CapQuery::at(*analysis->sp_cap), Lens{}};
            // Each chart under kCopies ids, so page rounding weighs less. An
            // id is 32 hex digits, the length of a real one (an md5).
            for (int copy = 0; copy < kCopies; ++copy) {
                char id[33];
                std::snprintf(id, sizeof id, "%032llx",
                              static_cast<unsigned long long>(saved.size()));
                PreparedRecord prepared = prepare_record(id, profile, *analysis);
                store.save(prepared);
                ++records;
                path_rows += static_cast<int64_t>(prepared.record.paths.size());
                saved.push_back(std::move(prepared));
            }
        }
        REQUIRE(records > 0);
        int mismatches = 0;
        for (const PreparedRecord& p : saved) {
            const RecordLookup got = store.lookup_one(p.record.song_id, p.profile);
            if ((got.status != RecordStatus::Ready || got.record != p.record) && ++mismatches <= 8)
                CHECK_MESSAGE(false, "record " << p.record.song_id << " reads back different");
        }
        CHECK(mismatches == 0);
    }

    // Runs `sql` on the file at `db_path`.
    const auto exec_on = [](const std::string& db_path, const std::string& sql) {
        sqlite3* db = nullptr;
        REQUIRE(open_sqlite(db_path, &db, SQLITE_OPEN_READWRITE) == SQLITE_OK);
        char* err = nullptr;
        const int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
        if (err) MESSAGE(err);
        sqlite3_free(err);
        sqlite3_close(db);
        REQUIRE(rc == SQLITE_OK);
    };
    // The size of a copy of the file after `del` ran on it and it was
    // compacted at SQLite's smallest page (512 bytes), so a size moves in
    // small steps.
    const auto compacted_size = [&](const std::string& tag, const std::string& del) {
        const std::string copy = file.path + "." + tag;
        std::remove(copy.c_str());
        exec_on(file.path, "VACUUM INTO '" + copy + "';");
        exec_on(copy, "PRAGMA journal_mode=DELETE; PRAGMA page_size=512; " + del + "VACUUM;");
        const auto size = static_cast<int64_t>(std::filesystem::file_size(hydra::os_path(copy)));
        std::remove(copy.c_str());
        return size;
    };
    const int64_t size_a = compacted_size("a", "");
    const int64_t size_b = compacted_size("b", "DELETE FROM record_paths; ");
    const int64_t size_c = compacted_size("c", "DELETE FROM record_paths; DELETE FROM records; ");

    MESSAGE("records " << records << ", path rows " << path_rows << ", files A " << size_a
                       << " B " << size_b << " C " << size_c << " bytes");
    MESSAGE("bytes per path row " << static_cast<double>(size_a - size_b) / path_rows
                                  << ", bytes per record "
                                  << static_cast<double>(size_b - size_c) / records);
}

TEST_CASE("store: a record's best path text is rank 1's, and empty when it kept no paths") {
    RecordRow record;
    CHECK(record.best_path_text().empty());
    PathRow first;
    first.rank = 1;
    first.path = "3- 1 2";
    PathRow second;
    second.rank = 2;
    second.path = "0 E3+ E5 E1";
    record.paths = {first, second};
    CHECK(record.best_path_text() == "3- 1 2");
    // An analysis that kept no paths becomes a record with no text.
    CHECK(record_of("h1", no_paths_at_cap(4)).best_path_text().empty());
}

// ---- the status rules -------------------------------------------------------
//
// Each case pins one rule lookup answers by: which stamps read Ready, why a
// Stale record is Stale, when other rules read Ready again, and what reads as
// Not analyzed.

TEST_CASE("store: every accepted results stamp reads Ready, others Stale") {
    // The stamp is not the app version (ADR 0018). Only "3.1.2+efillwindow"
    // reads Ready. Results stamped "1.8.2" (every release from 1.8.4 to
    // 2.0.0) or "1.8.3" hold values the SP-end history changed (ADR 0021),
    // earlier 2.1.0 builds' "2.1.0" can miss the all-0 path (D85),
    // "2.1.0+allzero" predates the 2x kick merge (D105), "2.4.0" predates
    // the measure-line fills and Star Power count (D115), "2.8.0" predates
    // the 85 ms early fill (D250), "2.8.0+efill85" predates 1.0 fills on
    // the shared grid, and "2.12.1+ch10grid" predates the coda skip of
    // written fills (FL-08), and "2.14.1+coda" predates the five-lane drum
    // conversion (D337), "2.14.1+fivelane" predates the all-toms step
    // for a drum chart with neither track flag up, and "2.14.1+alltoms"
    // predates the early-fill E mark reading the hit window (D344), and
    // "3.1.2+efillhitwindow" was analyzed at 85 ms on both sides, before the
    // early-fill window became a setting saved per result (the user's answer
    // "Mark them Stale"), so they read Stale, through lookup and the skip list
    // it backs (ready_song_ids).
    CHECK(kResultsStamp.is_current("3.1.2+efillwindow"));
    CHECK(std::string(kResultsStamp.written) == "3.1.2+efillwindow");
    CHECK_FALSE(kResultsStamp.is_current("3.1.2+efillhitwindow"));
    CHECK_FALSE(kResultsStamp.is_current("2.14.1+alltoms"));
    CHECK_FALSE(kResultsStamp.is_current("2.14.1+fivelane"));
    CHECK_FALSE(kResultsStamp.is_current("2.14.1+coda"));
    CHECK_FALSE(kResultsStamp.is_current("2.12.1+ch10grid"));
    CHECK_FALSE(kResultsStamp.is_current("2.8.0+efill85"));
    CHECK_FALSE(kResultsStamp.is_current("2.8.0"));
    CHECK_FALSE(kResultsStamp.is_current("2.4.0"));
    CHECK_FALSE(kResultsStamp.is_current("2.1.0+allzero"));
    CHECK_FALSE(kResultsStamp.is_current("2.1.0"));
    CHECK_FALSE(kResultsStamp.is_current("2.0.0"));
    CHECK_FALSE(kResultsStamp.is_current("1.8.2"));
    CHECK_FALSE(kResultsStamp.is_current("1.8.3"));
    CHECK(current_record_version() == std::string(kResultsStamp.written));

    RecordStore store(":memory:");
    const struct {
        const char* id;
        const char* stamp;
        RecordStatus want;
    } cases[] = {{"a", "3.1.2+efillwindow", RecordStatus::Ready},
                 {"n", "3.1.2+efillhitwindow", RecordStatus::Stale},
                 {"m", "2.14.1+alltoms", RecordStatus::Stale},
                 {"l", "2.14.1+fivelane", RecordStatus::Stale},
                 {"k", "2.14.1+coda", RecordStatus::Stale},
                 {"j", "2.12.1+ch10grid", RecordStatus::Stale},
                 {"i", "2.8.0+efill85", RecordStatus::Stale},
                 {"h", "2.8.0", RecordStatus::Stale},
                 {"g", "2.4.0", RecordStatus::Stale},
                 {"f", "2.1.0+allzero", RecordStatus::Stale},
                 {"e", "2.1.0", RecordStatus::Stale},
                 {"b", "1.8.3", RecordStatus::Stale},
                 {"c", "1.8.2", RecordStatus::Stale},
                 {"d", "0.0.0", RecordStatus::Stale}};
    for (const auto& c : cases) {
        CAPTURE(c.stamp);
        store.save_under_build_for_test(prepare_record(c.id, at4(), fixture().record), c.stamp);
        CHECK(store.lookup_one(c.id, at4()).status == c.want);
        CHECK(store.ready_song_ids(at4()).count(c.id) == (c.want == RecordStatus::Ready ? 1u : 0u));
    }
}

TEST_CASE("store: a record an old migration filed with unknown settings reads Not analyzed") {
    // Hydra 1.7 to 1.8.1 migrated older rows in with ms_enabled = -1: "a
    // result, settings unknown". No lens has -1, so such a record is no
    // candidate for any lookup. It reads as no record at all, not as Stale
    // (user decision 2026-09-26), and a real run of the chart replaces it.
    RecordStore store(":memory:");
    const Profile at8{"mode", CapQuery::at(8), Lens{}};
    PreparedRecord migrated = prepare_record("h", at8, at_cap(8));
    migrated.profile.lens.ms_enabled = -1;
    store.save_under_build_for_test(migrated, "1.6.0");
    CHECK(store.record_count() == 1);

    CHECK(store.lookup_one("h", at8).status == RecordStatus::NotAnalyzed);
    CHECK(store.ready_song_ids(at8).empty());
    CHECK(store.ready_records(at8).empty());

    store.save(prepare_record("h", at8, at_cap(8)));
    CHECK(store.record_count() == 1);
    CHECK(store.lookup_one("h", at8).status == RecordStatus::Ready);
}

TEST_CASE("store: a record made under other rules reads Stale until the store runs those rules") {
    const core::Rules other = hydra::test::other_rules();

    // A store running the default rules sees a record made under other rules
    // as Stale everywhere a lookup can ask.
    {
        RecordStore store(":memory:");
        store.save(prepare_record("h", at4(), hydra::test::other_rules_record(fixture().record)));
        CHECK(store.lookup_one("h", at4()).status == RecordStatus::Stale);
        CHECK(store.ready_song_ids(at4()).empty());
        CHECK(store.ready_records(at4()).empty());
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
        CHECK(store.ready_song_ids(at4()).empty());
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
    const PreparedRecord foreign =
        prepare_record("h", at4(), hydra::test::other_rules_record(fixture().record));
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
        CHECK(store.ready_song_ids(at4()).count("h") == 1);
    }
}

TEST_CASE("store: a Stale lookup says why (another build, other rules, or both), a Ready one nothing") {
    RecordStore store(":memory:");
    hydra::test::add_stale_records(store, fixture().record, at4(), "build", "rules", "both");
    store.save(prepare_record("ready", at4(), fixture().record));

    // This build, other rules.
    const RecordLookup by_rules = store.lookup_one("rules", at4());
    CHECK(by_rules.status == RecordStatus::Stale);
    CHECK(by_rules.stale_rules);
    CHECK_FALSE(by_rules.stale_build);

    // Another build, these rules.
    const RecordLookup by_build = store.lookup_one("build", at4());
    CHECK(by_build.status == RecordStatus::Stale);
    CHECK(by_build.stale_build);
    CHECK_FALSE(by_build.stale_rules);

    // Another build and other rules: both reasons.
    const RecordLookup by_both = store.lookup_one("both", at4());
    CHECK(by_both.status == RecordStatus::Stale);
    CHECK(by_both.stale_build);
    CHECK(by_both.stale_rules);

    // A Ready record carries no reason.
    const RecordLookup ready = store.lookup_one("ready", at4());
    CHECK(ready.status == RecordStatus::Ready);
    CHECK_FALSE(ready.stale_build);
    CHECK_FALSE(ready.stale_rules);
}

TEST_CASE("store: a current record with no paths is Ready, not Stale") {
    // "Analyzed, and nothing survived" is a real result, so it reads back as
    // Ready with no summary -- the status, not the path count, is what says
    // whether a record is usable.
    RecordStore store(":memory:");
    store.save(prepare_record("h", at4(), no_paths_at_cap(4)));
    const RecordLookup got = store.lookup_one("h", at4());
    CHECK(got.status == RecordStatus::Ready);
    CHECK(got.record.paths.empty());
    CHECK(got.record.best_path_text().empty());
    CHECK(got.record.summary() == PathSummary{});
    CHECK(store.ready_song_ids(at4()).count("h") == 1);

    // D51 call 11 (finding 88): such a record has no scored best path, and
    // the summary says so in one place. A record with paths has one. The
    // listing's records say the same.
    CHECK_FALSE(record_of("h", no_paths_at_cap(4)).summary().has_scored_best_path());
    CHECK(record_of("h", at_cap(4)).summary().has_scored_best_path());
    store.save(prepare_record("g", at4(), at_cap(4)));
    const std::vector<RecordRow> rows = store.ready_records(at4());
    REQUIRE(rows.size() == 2);
    for (const RecordRow& row : rows)
        CHECK(row.summary().has_scored_best_path() == (row.song_id == "g"));
}
