// Tests for store/record_store.{h,cpp}: a saved record reads back as it was
// saved, and each lookup picks the record its profile names. The status rules
// (Ready, Stale and why, Not analyzed) and the one save and lookup are in
// test_store_records.cpp; the library table and scan are in
// test_store_library.cpp; how the store opens, upgrades and fails is in
// test_store_open.cpp.

#include "doctest.h"

#include <sqlite3.h>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/rules.h"  // default_stamp
#include "core/stars.h"
#include "core/strutil.h"  // hex_lower
#include "corpus_util.h"
#include "db_file_util.h"  // scalar_on_file
#include "display_fixtures.h"  // save_old_build_record, add_stale_records, other_rules_record
#include "parse/song.h"
#include "record_fixtures.h"  // tied_variant_record
#include "search/graph.h"
#include "search/pather.h"
#include "store/record_store.h"
#include "store_fixtures.h"  // fixture, at_cap, fixture_chart, store_fixture_settings, OldLibrary
#include "temp_util.h"

using namespace hydra;
using namespace hydra::store;

using hydra::test::OldLibrary;
using hydra::test::at_cap;
using hydra::test::fixture;
using hydra::test::fixture_chart;
using hydra::test::old_library_analysis;
using hydra::test::store_fixture_settings;
// One integer straight out of a database file: how these tests look at a
// table without the store growing an accessor for it.
using hydra::test::scalar_on_file;

namespace {

struct Config {
    const char* key;
    int cap;
    DepthMode dmode;
    int dvalue;
    std::optional<double> ms;
    bool legacy_fills = false;  // Clone Hero 1.0's fill deadline
};

// The config matrix the GUI/CLI expose: score depth, points depth, the ms
// filter, and a fixed what-if cap.
const std::vector<Config> kMatrix = {
    {"cap4.scores.10", 4, DepthMode::Scores, 10, std::nullopt},
    {"cap4.scores.200", 4, DepthMode::Scores, 200, std::nullopt},
    {"cap4.scores.0", 4, DepthMode::Scores, 0, std::nullopt},
    {"cap4.scores.1", 4, DepthMode::Scores, 1, std::nullopt},
    {"cap4.scores.3", 4, DepthMode::Scores, 3, std::nullopt},
    {"cap4.points.5000", 4, DepthMode::Points, 5000, std::nullopt},
    {"cap4.scores.200.ms5", 4, DepthMode::Scores, 200, 5.0},
    {"cap4.scores.200.ms20", 4, DepthMode::Scores, 200, 20.0},
    {"cap8.scores.200", 8, DepthMode::Scores, 200, std::nullopt},
};

// The profile these tests file a record under: chart mode `mode`, SP cap
// `cap` and `lens`.
Profile profile_at(int cap, const Lens& lens = {}, const std::string& mode = "mode") {
    return Profile{mode, CapQuery::at(cap), lens};
}

// The profile a chart analyzed under `cfg` is saved under: its cap, and its
// ms limit (prepare_record refuses a profile whose ms limit isn't the
// record's).
Profile config_profile(const Config& cfg) {
    const Lens lens = Lens::from(
        cfg.ms ? std::optional<int>(static_cast<int>(*cfg.ms)) : std::nullopt, 0, 0);
    return profile_at(cfg.cap, lens);
}

// Saves `analysis` for chart `id` under `profile`, the way every caller saves.
void save_record(RecordStore& store, const std::string& id, const Profile& profile,
                 const HydraRecord& analysis) {
    store.save(prepare_record(id, profile, analysis));
}

// Saves `analysis` for chart `id` under `profile` as another Hydra build would
// have, so it reads Stale for "another build".
void save_old_build(RecordStore& store, const std::string& id, const Profile& profile,
                    const HydraRecord& analysis) {
    hydra::test::save_old_build_record(store, prepare_record(id, profile, analysis));
}

// The charts ready_records lists under `profile`, by song id.
std::set<std::string> ready_ids_listed(RecordStore& store, const Profile& profile) {
    std::set<std::string> out;
    for (const RecordRow& r : store.ready_records(profile)) out.insert(r.song_id);
    return out;
}

// The fixture's chart really analyzed under store_fixture_settings(...): a
// record that ran at the cap, ms limit and fill rule its profile names.
const HydraRecord& analysis_at(int cap, std::optional<double> ms = std::nullopt,
                               bool legacy_fills = false) {
    return corpus::analyzed(fixture_chart(), store_fixture_settings(cap, ms, legacy_fills));
}

// The fixture's chart analyzed at `cap` bars under other_rules()
// (display_fixtures.h), the rules a user's hydra_rules.ini might set. Saved in
// a store running the defaults, it reads Stale for "other rules".
const HydraRecord& other_rules_at(int cap) {
    return corpus::analyzed(fixture_chart(),
                            store_fixture_settings(cap, std::nullopt, false, test::other_rules()));
}

// A second corpus chart's real analysis under store_fixture_settings(cap, ms):
// a record that differs from analysis_at(cap, ms) in its paths.
HydraRecord second_chart_at(int cap, std::optional<double> ms = std::nullopt) {
    const std::vector<app::AnalysisResult> two =
        corpus::analyzed_with_paths(store_fixture_settings(cap, ms), 2);
    REQUIRE(two.size() == 2);
    return two[1].record;
}

}  // namespace

TEST_CASE("a saved record reads back whole across the corpus and config matrix") {
    // Every corpus chart under every setting is saved and read back. A chart
    // the engine refuses is not skipped: it goes through the app's own
    // failure path (app::prepare_failed_chart), and what that path stores is
    // read back too. A refusal the store does not remember reads back as
    // never analyzed.
    RecordStore store(":memory:");
    const std::vector<std::string>& paths = corpus::chart_paths();
    // The corpus is this test's input; with no chart there is nothing to read.
    REQUIRE_FALSE(paths.empty());

    // One chart saved under one setting: its record as saved or, for a
    // refused chart, the failure record the app stores (none when the store
    // does not remember that kind of failure).
    struct Saved {
        std::string id;
        std::optional<PreparedRecord> prepared;
        bool refused = false;
    };
    // Every chart saved, by setting: all are saved first, then each setting's
    // are read back in one lookup, so every record is read after every save.
    std::vector<std::vector<Saved>> saved(kMatrix.size());

    for (const std::string& path : paths) {
        for (size_t c = 0; c < kMatrix.size(); ++c) {
            const Config& cfg = kMatrix[c];
            Saved entry;
            entry.id = path + "|" + cfg.key;
            try {
                SearchSettings settings;
                settings.sp_cap = cfg.cap;
                settings.depth_mode = cfg.dmode;
                settings.depth_value = cfg.dvalue;
                settings.ms_filter = cfg.ms;
                const HydraRecord& record = corpus::analyzed(path, settings);
                entry.prepared = prepare_record(entry.id, config_profile(cfg), record);
            } catch (const ChartFileError& e) {
                entry.refused = true;
                entry.prepared = app::prepare_failed_chart(entry.id, config_profile(cfg), e);
                MESSAGE(entry.id << " refused: " << e.what());
            }
            if (entry.prepared) store.save(*entry.prepared);
            saved[c].push_back(std::move(entry));
        }
    }

    size_t checks = 0;
    for (size_t c = 0; c < kMatrix.size(); ++c) {
        std::vector<std::string> ids;
        ids.reserve(saved[c].size());
        for (const Saved& s : saved[c]) ids.push_back(s.id);
        const std::vector<RecordLookup> found = store.lookup(ids, config_profile(kMatrix[c]));
        REQUIRE(found.size() == saved[c].size());
        for (size_t i = 0; i < found.size(); ++i) {
            const Saved& s = saved[c][i];
            const RecordLookup& got = found[i];
            INFO(s.id);
            ++checks;
            if (!s.prepared) {
                CHECK(got.status == RecordStatus::NotAnalyzed);
            } else if (s.refused) {
                CHECK(got.status == RecordStatus::CantAnalyze);
                CHECK(got.failure_kind == s.prepared->record.failure_kind);
                CHECK(got.failure_text == s.prepared->record.failure_text);
            } else {
                CHECK(got.status == RecordStatus::Ready);
                CHECK(got.record == s.prepared->record);
            }
        }
    }

    // Every chart was read back under every setting, refused or not.
    CHECK(checks == paths.size() * kMatrix.size());
}

TEST_CASE("RecordStore maintenance: a lookup, the Ready list and the record count") {
    // The real v2.12.1 library: every record in it is another build's. Each
    // counts as a record, but not as "already analyzed".
    const OldLibrary old("maintenance");
    const Profile at4 = OldLibrary::profile();
    const HydraRecord& record = old_library_analysis();
    RecordStore store(old.path);
    const int64_t old_records = store.record_count();
    REQUIRE(old_records > 0);
    for (const std::string& id : OldLibrary::song_ids()) {
        INFO(id);
        CHECK(store.lookup_one(id, at4).status == RecordStatus::Stale);
    }
    CHECK(store.ready_records(at4).empty());
    CHECK(store.ready_song_ids(at4).empty());

    CHECK(store.lookup_one("h1", at4).status == RecordStatus::NotAnalyzed);
    save_record(store, "h1", at4, record);
    CHECK(store.lookup_one("h1", at4).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h1", OldLibrary::profile(8)).status == RecordStatus::NotAnalyzed);

    const std::vector<RecordRow> listing = store.ready_records(at4);
    REQUIRE(listing.size() == 1);
    CHECK(listing[0].song_id == "h1");
    CHECK(listing[0].best_path_text() == record.best_path().pathstring());
    CHECK(store.record_count() == old_records + 1);
    CHECK(store.ready_song_ids(at4) == std::unordered_set<std::string>{"h1"});
}

namespace {

// The two lenses the coexistence tests use: same chart, same cap, different
// searches. Lens C is a third nobody stored anything under.
const Lens kLensA = Lens::from(10, 0, 20);
const Lens kLensB = Lens::from(std::nullopt, 1, 5000);
const Lens kLensC = Lens::from(25, 0, 3);

// The fixture's chart analyzed at `cap` bars with the ms limit lens A claims.
const HydraRecord& at_cap_ms10(int cap) { return analysis_at(cap, 10.0); }

}  // namespace

TEST_CASE("records at different caps coexist; each lookup sees only its own cap") {
    RecordStore store(":memory:");
    save_record(store, "h", profile_at(4), analysis_at(4));
    save_record(store, "h", profile_at(8), analysis_at(8));
    CHECK(store.record_count() == 2);

    // Exact lookups see exactly their cap.
    CHECK(store.lookup_one("h", profile_at(4)).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h", profile_at(8)).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h", profile_at(5)).status == RecordStatus::NotAnalyzed);

    // A stale 6-bar record, analyzed under other rules, leaves the 8-bar
    // answer alone -- for single lookups and for the listing alike.
    save_record(store, "h", profile_at(6), other_rules_at(6));
    CHECK(store.lookup_one("h", profile_at(8)).status == RecordStatus::Ready);
    CHECK(ready_ids_listed(store, profile_at(8)) == std::set<std::string>{"h"});

    // Asked for cap 6 exactly, that stale record is reported as stale, with
    // no numbers handed out.
    const RecordLookup stale_lookup = store.lookup_one("h", profile_at(6));
    CHECK(stale_lookup.status == RecordStatus::Stale);
    CHECK(stale_lookup.stale_rules);
    CHECK(stale_lookup.record.paths.empty());

    // The listing leaves it out. A stale record takes its chart out of the
    // listing entirely, so a report counts it as never analyzed rather than
    // reading numbers this build cannot vouch for.
    CHECK(store.ready_records(profile_at(6)).empty());
    const std::vector<RecordRow> at4 = store.ready_records(profile_at(4));
    REQUIRE(at4.size() == 1);
    CHECK(at4[0].summary().score == analysis_at(4).best_path().totalscore());
}

TEST_CASE("the listing and a lookup agree on which record is a chart's answer") {
    // The lock between the two paths. Each chart below holds records at
    // several caps. At every cap, whatever lookup picks is what the listing
    // must show, and when that pick is not readable the chart must not be
    // listed.
    RecordStore store(":memory:");
    const std::vector<const char*> charts = {"two_caps", "over_stale", "all_stale"};

    // Two current records at two caps.
    save_record(store, "two_caps", profile_at(8), analysis_at(8));
    save_record(store, "two_caps", profile_at(6), analysis_at(6));

    // A current record and a taller stale one, analyzed under other rules.
    save_record(store, "over_stale", profile_at(4), analysis_at(4));
    save_record(store, "over_stale", profile_at(8), other_rules_at(8));

    // Nothing readable at all.
    save_record(store, "all_stale", profile_at(6), other_rules_at(6));

    for (int cap : {4, 6, 8}) {
        const std::set<std::string> listed = ready_ids_listed(store, profile_at(cap));
        for (const char* id : charts) {
            INFO(id << " at " << cap);
            const RecordLookup rec = store.lookup_one(id, profile_at(cap));
            CHECK(listed.count(id) == (rec.status == RecordStatus::Ready ? 1u : 0u));
        }
    }

    // Spelled out, so a change that moves both paths together still has to
    // answer for itself.
    CHECK(ready_ids_listed(store, profile_at(6)).count("two_caps") == 1);
    CHECK(ready_ids_listed(store, profile_at(8)).count("two_caps") == 1);
    CHECK(ready_ids_listed(store, profile_at(4)).count("over_stale") == 1);
    CHECK(ready_ids_listed(store, profile_at(8)).count("over_stale") == 0);
    CHECK(store.lookup_one("all_stale", profile_at(6)).status == RecordStatus::Stale);
}

TEST_CASE("prepare_record refuses a profile whose ms limit isn't the analysis's") {
    // The record would claim settings the search never ran under, and every
    // later lookup would believe it.
    HydraRecord none = at_cap(4);  // analyzed with no ms limit
    CHECK_THROWS_AS(prepare_record("h", profile_at(4, kLensA), none), std::invalid_argument);

    HydraRecord ten = at_cap_ms10(4);
    CHECK_THROWS_AS(prepare_record("h", profile_at(4, kLensC), ten), std::invalid_argument);

    // The matching lens is fine, and rides onto the prepared record.
    CHECK(prepare_record("h", profile_at(4, kLensA), ten).profile.lens == kLensA);

    // The limit is checked both ways (finding 130): an analysis made with a
    // limit is refused under a lens that has the limit off, too.
    CHECK_THROWS_AS(prepare_record("h", profile_at(4, kLensB), ten), std::invalid_argument);
}

TEST_CASE("prepare_record files the record under the analysis's own rules fingerprint") {
    // The rules_fp column is what rank_row compares against this process's
    // rules, so it must be the analysis's, never the store's.
    const HydraRecord& own = analysis_at(4);
    CHECK(prepare_record("h", profile_at(4), own).rules_fp == own.rules_fingerprint);
    const HydraRecord& foreign = other_rules_at(4);
    CHECK(prepare_record("h", profile_at(4), foreign).rules_fp == foreign.rules_fingerprint);
    CHECK(foreign.rules_fingerprint != own.rules_fingerprint);
}

TEST_CASE("the rules_fp column holds the bytes a lookup under the same rules reads as Ready") {
    // Every profile's key includes these bytes, so a save that wrote them
    // any other way than the lookup expects would orphan every result a user
    // has. The bytes a row must hold are ready_rules_fp's answer.
    const std::string db = testtemp::temp_path("rules_fp_bytes", ".db");
    const HydraRecord& rec = analysis_at(4);
    REQUIRE(rec.rules_fingerprint == core::default_stamp().fixed);
    {
        RecordStore store(db);
        save_record(store, "h", profile_at(4), rec);
    }
    const std::vector<uint8_t> want = ready_rules_fp(core::default_stamp());
    REQUIRE_FALSE(want.empty());
    CHECK(scalar_on_file(db, "SELECT COUNT(*) FROM profiles WHERE lower(hex(rules_fp)) = '" +
                                 hex_lower(want.data(), want.size()) + "'") == 1);
    {
        // A store opened on the file, under the same rules, reads it back.
        RecordStore reopened(db);
        CHECK(reopened.lookup_one("h", profile_at(4)).status == RecordStatus::Ready);
    }
    std::error_code ec;
    for (const char* suffix : {"", "-wal", "-shm"})
        std::filesystem::remove(std::filesystem::u8path(db + suffix), ec);
}

TEST_CASE("Profile compares on every part of the identity") {
    const Profile profile = profile_at(4, kLensA);
    CHECK(profile == profile_at(4, kLensA));
    CHECK_FALSE(profile == profile_at(4, kLensA, "other mode"));
    CHECK_FALSE(profile == profile_at(8, kLensA));
    CHECK_FALSE(profile == profile_at(4, kLensB));
    CHECK(CapQuery::at(4) != CapQuery::at(8));
    CHECK(CapQuery{} == CapQuery::at(kCloneHeroSpCap));

    // Each of the lens's six fields is part of the identity...
    CHECK(kLensA == Lens::from(10, 0, 20));
    CHECK(kLensA != Lens::from(11, 0, 20));
    CHECK(kLensA != Lens::from(10, 1, 20));
    CHECK(kLensA != Lens::from(10, 0, 21));
    CHECK(kLensA != Lens::from(std::nullopt, 0, 20));
    CHECK(kLensA != Lens::from(10, 0, 20, /*legacy_fills=*/true));
    CHECK(kLensA != Lens::from(10, 0, 20, false, kMaxEarlyFillWindowMs));
    // A lens left to its default names the setting's default window.
    CHECK(Lens{}.efill_window_ms == static_cast<int>(kDefaultEarlyFillWindowMs));
    CHECK(kLensA.efill_window_ms == static_cast<int>(kDefaultEarlyFillWindowMs));

    // ...except the ms value when the limit is off: an off limit stores 0.
    CHECK(Lens::from(std::nullopt, 0, 4).ms_value == 0);
}

// ---- lens identity --------------------------------------------------------
//
// The lens is the rest of a result's identity: the ms limit and the score
// range it ran under. These cases pin that two lenses coexist and that each
// lookup gets its own answer.

TEST_CASE("the same chart at the same cap keeps one result per lens") {
    RecordStore store(":memory:");

    // The two analyses differ in the one field that says which search ran:
    // lens A's ms limit is 10, lens B's is off.
    save_record(store, "h", profile_at(4, kLensA), at_cap_ms10(4));
    save_record(store, "h", profile_at(4, kLensB), analysis_at(4));
    CHECK(store.record_count() == 2);

    CHECK(store.lookup_one("h", profile_at(4, kLensA)).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h", profile_at(4, kLensB)).status == RecordStatus::Ready);

    // A lens nobody ran under has no answer here, and no stored record stands
    // in for it -- this is what stops a batch run skipping the chart.
    CHECK(store.lookup_one("h", profile_at(4, kLensC)).status == RecordStatus::NotAnalyzed);
    CHECK(store.ready_song_ids(profile_at(4, kLensA)).count("h") == 1);
    CHECK(store.ready_song_ids(profile_at(4, kLensB)).count("h") == 1);
    CHECK(store.ready_song_ids(profile_at(4, kLensC)).empty());

    // Re-running one lens replaces that record and leaves the other alone. The
    // re-run's record is a second chart's analysis, so it differs from the
    // first save in its paths.
    const HydraRecord redone = second_chart_at(4, 10.0);
    const RecordRow first_a = prepare_record("h", profile_at(4, kLensA), at_cap_ms10(4)).record;
    const RecordRow redone_a = prepare_record("h", profile_at(4, kLensA), redone).record;
    REQUIRE(redone_a != first_a);
    save_record(store, "h", profile_at(4, kLensA), redone);
    CHECK(store.record_count() == 2);
    CHECK(store.lookup_one("h", profile_at(4, kLensA)).record == redone_a);
    CHECK(store.lookup_one("h", profile_at(4, kLensB)).record ==
          prepare_record("h", profile_at(4, kLensB), analysis_at(4)).record);
}

TEST_CASE("a current-version save purges the chart's old-version records at every cap and lens") {
    // The real v2.12.1 library: its records are another build's, at 4 bars.
    const OldLibrary old("purge_old_versions");
    const std::vector<std::string> ids = OldLibrary::song_ids();
    RecordStore store(old.path);
    const int64_t before = store.record_count();
    for (const std::string& id : ids) {
        INFO(id);
        REQUIRE(store.lookup_one(id, OldLibrary::profile()).status == RecordStatus::Stale);
    }

    // A save at another cap, under the same lens.
    save_record(store, ids[0], OldLibrary::profile(8), old_library_analysis(8));

    // This build cannot read what another version wrote for this chart, so
    // the save supersedes it -- at every cap and lens...
    CHECK(store.lookup_one(ids[0], OldLibrary::profile()).status == RecordStatus::NotAnalyzed);
    CHECK(store.lookup_one(ids[0], OldLibrary::profile(8)).status == RecordStatus::Ready);
    // ...while another chart's old record is none of this save's business.
    for (size_t i = 1; i < ids.size(); ++i) {
        INFO(ids[i]);
        CHECK(store.lookup_one(ids[i], OldLibrary::profile()).status == RecordStatus::Stale);
    }
    // One old record gone, one current record in.
    CHECK(store.record_count() == before);
}

// ---- the fill rule in the profile (docs/adr/0010) ---------------------------

namespace {

// The default Lens{} but for the fill rule, so a profile differs from its 1.1
// twin in nothing else.
const Lens kLegacy = [] {
    Lens lens;
    lens.legacy_fills = 1;
    return lens;
}();

// The fixture's chart analyzed at `cap` bars under Clone Hero 1.0's fill rule.
const HydraRecord& legacy_at_cap(int cap) { return analysis_at(cap, std::nullopt, true); }

}  // namespace

TEST_CASE("1.0 and 1.1 results for one chart sit side by side") {
    RecordStore store(":memory:");
    const Profile ch11 = profile_at(4);
    const Profile ch10 = profile_at(4, kLegacy);

    save_record(store, "h", ch11, analysis_at(4));
    CHECK(store.lookup_one("h", ch10).status == RecordStatus::NotAnalyzed);
    CHECK(store.ready_song_ids(ch10).empty());

    // Saving the 1.0 result keeps the 1.1 one, and each lookup finds its own.
    save_record(store, "h", ch10, legacy_at_cap(4));
    CHECK(store.record_count() == 2);
    CHECK(store.lookup_one("h", ch10).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h", ch11).status == RecordStatus::Ready);
    CHECK(store.ready_song_ids(ch10).count("h") == 1);

    // Re-analyzing under 1.0 replaces only the 1.0 record.
    save_record(store, "h", ch10, legacy_at_cap(4));
    CHECK(store.record_count() == 2);
    CHECK(store.lookup_one("h", ch11).status == RecordStatus::Ready);
}

TEST_CASE("prepare_record refuses a profile that names the other fill rule") {
    const Profile ch10 = profile_at(4, kLegacy);
    const Profile ch11 = profile_at(4);
    CHECK_THROWS_AS(prepare_record("h", ch10, at_cap(4)), std::invalid_argument);
    CHECK_THROWS_AS(prepare_record("h", ch11, legacy_at_cap(4)), std::invalid_argument);
    CHECK(prepare_record("h", ch10, legacy_at_cap(4)).profile.lens.legacy_fills == 1);
}

TEST_CASE("lookups on one thread and saves on another share one store without error") {
    // The store has one connection, and a read on it would see a save's
    // half-done work, transaction or not. What keeps a lookup out of a save
    // is the store lock (StoreConnection::mutex in store/record_store.h), held
    // for the whole save frame and for the lookup. This case runs the two
    // side by side and checks every lookup came back whole and nothing
    // threw. It does not force a lookup to land inside a save, so it cannot
    // prove the lock covers the whole save; it would likely, not surely,
    // catch a store that lost its lock.
    // The writer saves two corpus charts' real analyses in turn under one
    // chart id, so a lookup that read half of one save would match neither.
    RecordStore store(":memory:");
    const Profile at4 = profile_at(4);
    const HydraRecord& first = analysis_at(4);
    const HydraRecord second = second_chart_at(4);
    const RecordRow first_row = prepare_record("h", at4, first).record;
    const RecordRow second_row = prepare_record("h", at4, second).record;
    REQUIRE(first_row != second_row);
    save_record(store, "h", at4, first);

    std::atomic<bool> stop{false};
    std::thread writer([&] {
        for (int i = 0; i < 50; ++i) save_record(store, "h", at4, i % 2 ? first : second);
        stop.store(true);
    });
    int reads = 0;
    bool all_whole = true;
    std::string failure;
    do {
        try {
            const RecordLookup r = store.lookup_one("h", at4);
            if (r.status != RecordStatus::Ready) all_whole = false;
            else if (r.record != first_row && r.record != second_row) all_whole = false;
            ++reads;
        } catch (const std::exception& e) {
            failure = e.what();
            break;
        }
    } while (!stop.load());
    writer.join();

    // doctest's assertions are not thread-safe, so every check is out here.
    CHECK(failure.empty());
    CHECK(all_whole);
    CHECK(reads > 0);
}

// D79: the batch's skip list (settled_song_ids, which app::charts_with_result
// reads) is the library's chip. Every kind of record a chart can hold is
// here: a Ready record, a stored failure, stale records, a Ready record beside
// one made under other rules, a record under another chart mode, and a
// repeated library id.
TEST_CASE("settled_song_ids names exactly the charts a library page reads as Ready or Can't analyze") {
    RecordStore store(":memory:");
    const Profile at8 = profile_at(8);
    const std::vector<std::string> charts = {"ready", "failed", "build", "rules",
                                             "both",  "mixed",  "other_mode", "none"};
    save_record(store, "ready", at8, at_cap(8));
    test::add_stale_records(store, at_cap(8), at8, "build", "rules", "both");
    save_record(store, "mixed", at8, at_cap(8));
    save_record(store, "mixed", at8, test::other_rules_record(at_cap(8)));
    save_record(store, "other_mode", profile_at(8, Lens{}, "other"), at_cap(8));

    // A chart the analysis refuses, stored the app's way: the hand-written
    // parse_edge chart with no Expert Pro Drums notes (expected_expert.tsv).
    const std::string no_notes =
        corpus::testdata_root() + "/parse_edge/c15_unterminated_drums.chart";
    std::optional<PreparedRecord> failure;
    try {
        app::analyze_chart_file(no_notes, store_fixture_settings(8));
    } catch (const KindedError& e) {
        failure = app::prepare_failed_chart("failed", at8, e);
    }
    REQUIRE_MESSAGE(failure.has_value(), "no failure record for " << no_notes);
    store.save(*failure);

    std::vector<std::string> library = charts;
    library.push_back("ready");  // a second folder's copy
    const std::vector<RecordLookup> chips = store.lookup(library, at8);
    REQUIRE(chips.size() == library.size());
    const std::unordered_map<std::string, RecordStatus> settled = store.settled_song_ids(at8);
    for (size_t i = 0; i < library.size(); ++i) {
        INFO(library[i]);
        const RecordStatus chip = chips[i].status;
        const bool chip_settled = chip == RecordStatus::Ready || chip == RecordStatus::CantAnalyze;
        const auto it = settled.find(library[i]);
        CHECK((it != settled.end()) == chip_settled);
        if (it != settled.end()) CHECK(it->second == chip);
    }
    CHECK(settled == std::unordered_map<std::string, RecordStatus>{
                         {"ready", RecordStatus::Ready},
                         {"mixed", RecordStatus::Ready},
                         {"failed", RecordStatus::CantAnalyze}});
}

TEST_CASE("lookup answers a page the same as lookup_one chart by chart") {
    RecordStore store(":memory:");
    save_record(store, "ready", profile_at(4), analysis_at(4));
    // Analyzed under other rules, so Stale in this store.
    save_record(store, "stale", profile_at(4), other_rules_at(4));
    save_record(store, "two_caps", profile_at(8), analysis_at(8));
    save_record(store, "two_caps", profile_at(6), analysis_at(6));

    // "ready" twice: a page can list the same chart from two folders. Each
    // page's answers are spelled out, since lookup_one is built on lookup and
    // agreeing with it alone proves nothing.
    const std::vector<std::string> page = {"ready", "stale", "none", "two_caps", "ready"};
    using S = RecordStatus;
    const std::vector<std::pair<int, std::vector<S>>> want = {
        {4, {S::Ready, S::Stale, S::NotAnalyzed, S::NotAnalyzed, S::Ready}},
        {6, {S::NotAnalyzed, S::NotAnalyzed, S::NotAnalyzed, S::Ready, S::NotAnalyzed}},
        {8, {S::NotAnalyzed, S::NotAnalyzed, S::NotAnalyzed, S::Ready, S::NotAnalyzed}},
    };
    for (const auto& [cap, statuses] : want) {
        const std::vector<RecordLookup> got = store.lookup(page, profile_at(cap));
        REQUIRE(got.size() == page.size());
        for (size_t i = 0; i < page.size(); ++i) {
            INFO(page[i] << " at " << cap);
            CHECK(got[i].status == statuses[i]);
            CHECK(got[i].status == store.lookup_one(page[i], profile_at(cap)).status);
            // Every Ready record here is the fixture's chart analyzed at the
            // page's cap, so its best path is that analysis's.
            CHECK(got[i].record.best_path_text() == (statuses[i] == S::Ready
                                                         ? analysis_at(cap).best_path().pathstring()
                                                         : std::string()));
        }
    }
    CHECK(store.lookup({}, profile_at(4)).empty());
}

// ---- library summaries (interface redesign, Task 7) ------------------------

TEST_CASE("a saved record stores its best path's star count") {
    RecordStore store(":memory:");
    const Profile at4 = profile_at(4);
    save_record(store, "ready", at4, at_cap(4));
    save_old_build(store, "stale", at4, at_cap(4));

    const Path& best = fixture().record.best_path();
    const int stars = path_stars(best);

    const std::vector<RecordLookup> got = store.lookup({"ready", "stale", "none"}, at4);
    REQUIRE(got.size() == 3);
    CHECK(got[0].status == RecordStatus::Ready);
    CHECK(got[0].record == prepare_record("ready", at4, at_cap(4)).record);
    CHECK(got[0].record.stars == stars);
    CHECK(got[0].record.summary().score == best.totalscore());
    // The saved figure is the engine's own; shown, it is the live path's
    // difficulty.
    CHECK(got[0].record.summary().hardest_ms == engine_hardest_ms(best));
    CHECK(shown_timing(got[0].record.summary().hardest_ms) == best.difficulty());
    CHECK(got[0].record.summary().stars == stars);
    // Only a Ready answer carries a record.
    CHECK(got[1].status == RecordStatus::Stale);
    CHECK(got[1].record.paths.empty());
    CHECK_FALSE(got[1].record.stars.has_value());
    CHECK(got[2].status == RecordStatus::NotAnalyzed);
    CHECK_FALSE(got[2].record.stars.has_value());

    const std::vector<RecordRow> listing = store.ready_records(at4);
    REQUIRE(listing.size() == 1);
    CHECK(listing[0].stars == stars);
    CHECK(listing[0].song_id == "ready");
}

TEST_CASE("store: the saved hardest figure is the engine's, sign included") {
    // Every corpus chart's real analysis is saved and read back. Each saved
    // path's figure must be engine_hardest_ms of the path it was made from,
    // compared exactly, so a figure below zero (a timing with room to spare,
    // which shown_timing hides) must come back with its sign.
    RecordStore store(":memory:");
    const app::AnalysisSettings settings = store_fixture_settings();
    const Profile at4 = profile_at(4);
    size_t compared = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const HydraRecord* record = nullptr;
        try {
            record = &corpus::analyzed(path, settings);
        } catch (const ChartFileError&) {
            // A refused chart has no path and so no figure; the corpus round
            // trip above checks what the store keeps for it.
            continue;
        }
        INFO(path);
        save_record(store, path, at4, *record);
        const RecordLookup got = store.lookup_one(path, at4);
        REQUIRE(got.status == RecordStatus::Ready);
        // The saved paths, in rank order, are the first of all_paths.
        const std::vector<const Path*> kept = record->all_paths();
        REQUIRE(got.record.paths.size() <= kept.size());
        for (size_t i = 0; i < got.record.paths.size(); ++i) {
            CHECK(got.record.paths[i].hardest_ms == engine_hardest_ms(*kept[i]));
            ++compared;
        }
    }
    // fixture() is the first corpus chart with paths at these settings, so
    // the walk meets at least the paths its record saves.
    const size_t fixture_rows = record_of("", fixture().record).paths.size();
    REQUIRE(fixture_rows > 0);
    REQUIRE(compared >= fixture_rows);
}

TEST_CASE("lookup answers a whole library in chunks") {
    // SQLite refuses a statement with more bound values than its variable
    // limit. The library asks about every chart at once, so the store must
    // split the ask. The page here holds one id more than that limit, read
    // from SQLite itself, so a store that sent it in one statement fails.
    sqlite3* probe = nullptr;
    REQUIRE(sqlite3_open(":memory:", &probe) == SQLITE_OK);
    const int bind_limit = sqlite3_limit(probe, SQLITE_LIMIT_VARIABLE_NUMBER, -1);
    sqlite3_close(probe);
    REQUIRE(bind_limit > 0);

    RecordStore store(":memory:");
    save_record(store, "ready", profile_at(4), at_cap(4));
    const RecordRow saved = prepare_record("ready", profile_at(4), at_cap(4)).record;

    // Every other id is a chart the store has no record for, as most of a
    // library is before its first batch.
    const size_t count = static_cast<size_t>(bind_limit) + 1;
    std::vector<std::string> ids;
    ids.reserve(count);
    for (size_t i = 0; i < count; ++i) ids.push_back("never_analyzed_" + std::to_string(i));
    // The analyzed chart at the start, the middle and the end, so it lands
    // in more than one chunk.
    for (size_t at : {size_t{0}, count / 2, count - 1}) ids[at] = "ready";

    const std::vector<RecordLookup> got = store.lookup(ids, profile_at(4));
    REQUIRE(got.size() == ids.size());
    int ready = 0;
    for (size_t i = 0; i < got.size(); ++i) {
        if (ids[i] == "ready") {
            CHECK(got[i].status == RecordStatus::Ready);
            CHECK(got[i].record == saved);
            ++ready;
        } else if (got[i].status != RecordStatus::NotAnalyzed) {
            FAIL("id " << ids[i] << " read as analyzed");
        }
    }
    CHECK(ready == 3);
}

TEST_CASE("PathSummary equality covers every field, stars included") {
    // Finding 121: one compare for every field, unset against set counting
    // as a change. The summary is a real one: the fixture's record saved and
    // read back, the way the library reads it.
    RecordStore store(":memory:");
    save_record(store, "ready", profile_at(4), at_cap(4));
    const RecordLookup got = store.lookup_one("ready", profile_at(4));
    REQUIRE(got.status == RecordStatus::Ready);
    const PathSummary real = got.record.summary();
    REQUIRE(real.has_scored_best_path());
    // The read-back summary is the one the saved record makes.
    CHECK(real == prepare_record("ready", profile_at(4), at_cap(4)).record.summary());

    const PathSummary copy = real;
    CHECK(copy == real);
    CHECK_FALSE(copy != real);

    // Two changes per field, each made from the field's own value: one that
    // moves a set value (or sets an unset one) and one that unsets a set
    // value (or sets an unset one). Each must count as a difference.
    auto moved = [](auto& field) {
        using T = typename std::decay_t<decltype(field)>::value_type;
        field = field ? *field + T{1} : T{};
    };
    auto toggled = [](auto& field) {
        using T = typename std::decay_t<decltype(field)>::value_type;
        if (field) field.reset();
        else field = T{};
    };
    auto differs_after = [&real](const auto& change) {
        PathSummary other = real;
        change(other);
        return other != real && !(other == real);
    };
    CHECK(differs_after([&](PathSummary& s) { moved(s.score); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.actcount); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.maxskip); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.hardest_ms); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.avgmult); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.notecount); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.sqin_count); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.sqout_count); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.pathcount); }));
    CHECK(differs_after([&](PathSummary& s) { moved(s.stars); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.score); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.actcount); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.maxskip); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.hardest_ms); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.avgmult); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.notecount); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.sqin_count); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.sqout_count); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.pathcount); }));
    CHECK(differs_after([&](PathSummary& s) { toggled(s.stars); }));
    // A record with no paths has no facts, and two such summaries agree.
    CHECK(RecordRow{}.summary() == RecordRow{}.summary());
}

TEST_CASE("a record counts every kept path once") {
    // Finding 172: the pathcount column counts every path the Paths tab lists
    // (HydraRecord::all_paths), on a real record and on one with a tied
    // variant under its first root.
    const HydraRecord& real = fixture().record;
    CHECK(record_of("", real).pathcount == static_cast<int>(real.all_paths().size()));
    const HydraRecord tied = test::tied_variant_record();
    CHECK(record_of("", tied).pathcount == static_cast<int>(tied.all_paths().size()));
    // all_paths lists the variant beside the roots, so a count of the roots
    // alone fails the check above.
    CHECK(tied.all_paths().size() > tied.paths.size());
}
