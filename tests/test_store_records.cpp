// The store's one save and one lookup (D115): prepare_record, RecordStore::save,
// lookup, lookup_one, ready_song_ids and ready_records, plus the path-row size
// measured on the corpus. The old entry points' tests stay in
// test_store_results.cpp until A1-5 deletes them.
#include "doctest.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <sqlite3.h>

#include "core/model.h"
#include "core/winstr.h"  // os_path
#include "corpus_util.h"
#include "db_file_util.h"  // scalar_on_file
#include "display_fixtures.h"  // save_old_build_record, kOldBuildStamp, other_rules_record
#include "parse/song.h"
#include "store/record_store.h"
#include "store_fixtures.h"  // fixture, at_cap, no_paths_at_cap
#include "temp_util.h"

using namespace hydra;
using namespace hydra::store;

using hydra::test::at_cap;
using hydra::test::fixture;
using hydra::test::no_paths_at_cap;
using hydra::test::scalar_on_file;

namespace {

// The fixture's own profile: 4 bars, no ms limit, the 1.1 fill rule.
Profile at4(const std::string& mode = "mode") { return Profile{mode, CapQuery::at(4), Lens{}}; }

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

// A file store in the temp folder, removed when the case ends.
struct FileStore {
    std::string path = testtemp::temp_path("store_records", ".db");
    ~FileStore() {
        for (const char* ext : {"", "-wal", "-shm"}) std::remove((path + ext).c_str());
    }
};

}  // namespace

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
    // Rank 1 is what the old summary said.
    CHECK(got.record.summary() == summarize_record(analysis));
    CHECK(got.record.paths.front().path == best_path_text(analysis));
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

TEST_CASE("store: a save purges the chart's unreadable records in every mode and their profiles") {
    FileStore file;
    {
        RecordStore store(file.path);
        // An old build's record in another mode, then a save in this one.
        hydra::test::save_old_build_record(
            store, prepare_record("h1", at4("other mode"), fixture().record));
        REQUIRE(store.lookup_one("h1", at4("other mode")).status == RecordStatus::Stale);
        store.save(prepare_record("h1", at4(), fixture().record));

        CHECK(store.lookup_one("h1", at4("other mode")).status == RecordStatus::NotAnalyzed);
        CHECK(store.lookup_one("h1", at4()).status == RecordStatus::Ready);
    }
    // The old build's profile went with its last record.
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles") == 1);
    CHECK(scalar_on_file(file.path, "SELECT COUNT(*) FROM profiles WHERE hyversion='0.0.0'") == 0);
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

TEST_CASE("store: record_count counts every record, under any profile") {
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
    FileStore file;
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
