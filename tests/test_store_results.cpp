// Tests for store/record_store.{h,cpp}: a saved record reads back as it was
// saved, and each lookup picks the record its profile names. The status rules
// (Ready, Stale and why, Not analyzed) and the one save and lookup are in
// test_store_records.cpp; the library table and scan are in
// test_store_library.cpp; how the store opens, upgrades and fails is in
// test_store_open.cpp.

#include "doctest.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/stars.h"
#include "corpus_util.h"
#include "db_file_util.h"  // scalar_on_file
#include "display_fixtures.h"  // save_old_build_record, add_stale_records, other_rules_record
#include "parse/song.h"
#include "record_fixtures.h"  // tied_variant_record
#include "search/graph.h"
#include "search/pather.h"
#include "store/record_store.h"
#include "store_fixtures.h"  // fixture, at_cap, no_paths_at_cap
#include "temp_util.h"

using namespace hydra;
using namespace hydra::store;

using hydra::test::at_cap;
using hydra::test::fixture;
using hydra::test::no_paths_at_cap;
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

}  // namespace

TEST_CASE("a saved record reads back whole across the corpus and config matrix") {
    RecordStore store(":memory:");

    int checks = 0, mismatches = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;

        for (const Config& cfg : kMatrix) {
            const HydraRecord* record = nullptr;
            try {
                SearchSettings settings;
                settings.sp_cap = cfg.cap;
                settings.depth_mode = cfg.dmode;
                settings.depth_value = cfg.dvalue;
                settings.ms_filter = cfg.ms;
                record = &corpus::analyzed(path, settings);
            } catch (const ChartFileError&) {
                continue;  // charts the engine rejects have no record to store
            }
            REQUIRE(record->sp_cap.has_value());

            const std::string id = path + "|" + cfg.key;
            // prepare_record refuses a profile whose ms limit isn't the record's.
            const Lens lens = Lens::from(
                cfg.ms ? std::optional<int>(static_cast<int>(*cfg.ms)) : std::nullopt, 0, 0);
            const Profile profile = profile_at(*record->sp_cap, lens);
            const PreparedRecord prepared = prepare_record(id, profile, *record);
            store.save(prepared);

            const RecordLookup got = store.lookup_one(id, profile);
            ++checks;
            std::string d;
            if (got.status != RecordStatus::Ready) d = "no record after save";
            else if (got.record != prepared.record) d = "record";
            if (!d.empty() && ++mismatches <= 8)
                CHECK_MESSAGE(false, path << " [" << cfg.key << "] " << d);
        }
    }

    CHECK(mismatches == 0);
    REQUIRE(checks > 0);
    MESSAGE("checked " << checks << " saved records");
}

TEST_CASE("RecordStore maintenance: a lookup, the Ready list and the record count") {
    app::AnalysisSettings settings;
    settings.sp_cap = 4;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.ms_filter = std::nullopt;
    const HydraRecord record = corpus::first_analyzed_with_paths(settings).record;

    const Profile at4 = profile_at(4);
    RecordStore store(":memory:");
    CHECK(store.lookup_one("h1", at4).status == RecordStatus::NotAnalyzed);

    save_record(store, "h1", at4, record);
    CHECK(store.lookup_one("h1", at4).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h1", profile_at(8)).status == RecordStatus::NotAnalyzed);

    const std::vector<RecordRow> listing = store.ready_records(at4);
    REQUIRE(listing.size() == 1);
    CHECK(listing[0].song_id == "h1");
    CHECK(listing[0].best_path_text() == record.best_path().pathstring());
    CHECK(store.record_count() == 1);

    // A record another build saved counts as a record, but not as "already
    // analyzed".
    save_old_build(store, "h2", at4, record);
    CHECK(store.record_count() == 2);
    CHECK(store.lookup_one("h2", at4).status == RecordStatus::Stale);
    CHECK(store.ready_song_ids(at4) == std::unordered_set<std::string>{"h1"});
}

namespace {

// The two lenses the coexistence tests use: same chart, same cap, different
// searches. Lens C is a third nobody stored anything under.
const Lens kLensA = Lens::from(10, 0, 20);
const Lens kLensB = Lens::from(std::nullopt, 1, 5000);
const Lens kLensC = Lens::from(25, 0, 3);

// at_cap plus the ms limit lens A claims, so prepare_record's guard is satisfied.
HydraRecord at_cap_ms10(int cap) {
    HydraRecord r = at_cap(cap);
    r.ms_limit = 10.0;
    return r;
}

}  // namespace

TEST_CASE("records at different caps coexist; each lookup sees only its own cap") {
    RecordStore store(":memory:");
    save_record(store, "h", profile_at(4), at_cap(4));
    save_record(store, "h", profile_at(32), at_cap(32));
    CHECK(store.record_count() == 2);

    // Exact lookups see exactly their cap.
    CHECK(store.lookup_one("h", profile_at(4)).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h", profile_at(32)).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h", profile_at(8)).status == RecordStatus::NotAnalyzed);

    // A stale 64-bar record leaves the 32-bar answer alone -- for single
    // lookups and for the listing alike.
    save_old_build(store, "h", profile_at(64), at_cap(64));
    CHECK(store.lookup_one("h", profile_at(32)).status == RecordStatus::Ready);
    CHECK(ready_ids_listed(store, profile_at(32)) == std::set<std::string>{"h"});

    // Asked for cap 64 exactly, that stale record is reported as stale, with
    // no numbers handed out.
    const RecordLookup stale_lookup = store.lookup_one("h", profile_at(64));
    CHECK(stale_lookup.status == RecordStatus::Stale);
    CHECK(stale_lookup.stale_build);
    CHECK(stale_lookup.record.paths.empty());

    // The listing leaves it out. A stale record takes its chart out of the
    // listing entirely, so a report counts it as never analyzed rather than
    // reading numbers this build cannot vouch for.
    CHECK(store.ready_records(profile_at(64)).empty());
    const std::vector<RecordRow> at4 = store.ready_records(profile_at(4));
    REQUIRE(at4.size() == 1);
    CHECK(at4[0].summary().score == fixture().record.best_path().totalscore());
}

TEST_CASE("the listing and a lookup agree on which record is a chart's answer") {
    // The lock between the two paths. Each chart below holds records at
    // several caps. At every cap, whatever lookup picks is what the listing
    // must show, and when that pick is not readable the chart must not be
    // listed.
    RecordStore store(":memory:");
    const std::vector<const char*> charts = {"two_caps", "over_stale", "over_sentinel",
                                             "all_stale"};

    // Two current records at two caps.
    save_record(store, "two_caps", profile_at(32), at_cap(32));
    save_record(store, "two_caps", profile_at(16), at_cap(16));

    // A current record and a taller stale one. The stale one goes in second
    // because a current-version save purges the chart's other-version records.
    save_record(store, "over_stale", profile_at(8), at_cap(8));
    save_old_build(store, "over_stale", profile_at(64), at_cap(64));

    // A current record and a taller one an old migration left: the migrated
    // record (ms limit "unknown", -1) is no candidate at all.
    PreparedRecord sentinel = prepare_record("over_sentinel", profile_at(64), at_cap(64));
    sentinel.profile.lens.ms_enabled = -1;
    store.save(sentinel);
    save_record(store, "over_sentinel", profile_at(8), at_cap(8));

    // Nothing readable at all.
    save_old_build(store, "all_stale", profile_at(16), at_cap(16));

    for (int cap : {8, 16, 32, 64}) {
        const std::set<std::string> listed = ready_ids_listed(store, profile_at(cap));
        for (const char* id : charts) {
            INFO(id << " at " << cap);
            const RecordLookup rec = store.lookup_one(id, profile_at(cap));
            CHECK(listed.count(id) == (rec.status == RecordStatus::Ready ? 1u : 0u));
        }
    }

    // Spelled out, so a change that moves both paths together still has to
    // answer for itself.
    CHECK(ready_ids_listed(store, profile_at(16)).count("two_caps") == 1);
    CHECK(ready_ids_listed(store, profile_at(32)).count("two_caps") == 1);
    CHECK(ready_ids_listed(store, profile_at(8)).count("over_stale") == 1);
    CHECK(ready_ids_listed(store, profile_at(64)).count("over_stale") == 0);
    CHECK(ready_ids_listed(store, profile_at(8)).count("over_sentinel") == 1);
    CHECK(ready_ids_listed(store, profile_at(64)).count("over_sentinel") == 0);
    CHECK(store.lookup_one("all_stale", profile_at(16)).status == RecordStatus::Stale);
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
    CHECK(prepare_record("h", profile_at(4), at_cap(4)).rules_fp ==
          fixture().record.rules_fingerprint);
    const HydraRecord foreign = test::other_rules_record(at_cap(4));
    CHECK(prepare_record("h", profile_at(4), foreign).rules_fp == foreign.rules_fingerprint);
    CHECK(foreign.rules_fingerprint != fixture().record.rules_fingerprint);
}

TEST_CASE("the rules_fp column holds a fingerprint's bytes as one run wrote them") {
    // Every profile's key includes these bytes, so a change to how they
    // are written would orphan every result a user has. The literal is from
    // one run.
    const std::string db = testtemp::temp_path("rules_fp_bytes", ".db");
    HydraRecord rec = at_cap(4);
    rec.rules_fingerprint = 0x0102030405060708ULL;
    {
        RecordStore store(db);
        save_record(store, "h", profile_at(4), rec);
    }
    CHECK(scalar_on_file(db, "SELECT COUNT(*) FROM profiles WHERE hex(rules_fp) = '0807060504030201'") ==
          1);
    std::error_code ec;
    std::filesystem::remove(std::filesystem::u8path(db), ec);
}

TEST_CASE("Profile compares on every part of the identity") {
    const Profile profile = profile_at(4, kLensA);
    CHECK(profile == profile_at(4, kLensA));
    CHECK_FALSE(profile == profile_at(4, kLensA, "other mode"));
    CHECK_FALSE(profile == profile_at(8, kLensA));
    CHECK_FALSE(profile == profile_at(4, kLensB));
    CHECK(CapQuery::at(4) != CapQuery::at(8));
    CHECK(CapQuery{} == CapQuery::at(kCloneHeroSpCap));

    // Each of the lens's five fields is part of the identity...
    CHECK(kLensA == Lens::from(10, 0, 20));
    CHECK(kLensA != Lens::from(11, 0, 20));
    CHECK(kLensA != Lens::from(10, 1, 20));
    CHECK(kLensA != Lens::from(10, 0, 21));
    CHECK(kLensA != Lens::from(std::nullopt, 0, 20));
    CHECK(kLensA != Lens::from(10, 0, 20, /*legacy_fills=*/true));

    // ...except the ms value when the limit is off, which the engine ignores:
    // "off at 10" and "off at 42" ran the same search.
    CHECK(Lens::from(std::nullopt, 0, 4) == Lens::from(std::nullopt, 0, 4));
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
    save_record(store, "h", profile_at(4, kLensB), at_cap(4));
    CHECK(store.record_count() == 2);

    CHECK(store.lookup_one("h", profile_at(4, kLensA)).status == RecordStatus::Ready);
    CHECK(store.lookup_one("h", profile_at(4, kLensB)).status == RecordStatus::Ready);

    // A lens nobody ran under has no answer here, and no stored record stands
    // in for it -- this is what stops a batch run skipping the chart.
    CHECK(store.lookup_one("h", profile_at(4, kLensC)).status == RecordStatus::NotAnalyzed);
    CHECK(store.ready_song_ids(profile_at(4, kLensA)).count("h") == 1);
    CHECK(store.ready_song_ids(profile_at(4, kLensB)).count("h") == 1);
    CHECK(store.ready_song_ids(profile_at(4, kLensC)).empty());

    // Re-running one lens replaces that record and leaves the other alone.
    HydraRecord redone = at_cap_ms10(4);
    redone.paths.clear();
    redone.allzero_paths.clear();
    save_record(store, "h", profile_at(4, kLensA), redone);
    CHECK(store.record_count() == 2);
    CHECK_FALSE(store.lookup_one("h", profile_at(4, kLensA)).record.has_scored_best_path());
    CHECK(store.lookup_one("h", profile_at(4, kLensB)).record ==
          prepare_record("h", profile_at(4, kLensB), at_cap(4)).record);
}

TEST_CASE("a current-version save purges the chart's old-version records at every cap and lens") {
    RecordStore store(":memory:");
    for (const char* id : {"h", "other"}) save_old_build(store, id, profile_at(4, kLensB), at_cap(4));
    CHECK(store.record_count() == 2);

    HydraRecord fresh = at_cap_ms10(32);
    fresh.paths.clear();
    fresh.allzero_paths.clear();
    save_record(store, "h", profile_at(32, kLensA), fresh);

    // This build cannot read what another version wrote for this chart, so
    // the save supersedes it -- at every cap and lens...
    CHECK(store.lookup_one("h", profile_at(4, kLensB)).status == RecordStatus::NotAnalyzed);
    // ...while another chart's old record is none of this save's business.
    CHECK(store.lookup_one("other", profile_at(4, kLensB)).status == RecordStatus::Stale);
    CHECK(store.record_count() == 2);
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

HydraRecord legacy_at_cap(int cap) {
    HydraRecord r = at_cap(cap);
    r.legacy_fills = true;
    return r;
}

}  // namespace

TEST_CASE("1.0 and 1.1 results for one chart sit side by side") {
    RecordStore store(":memory:");
    const Profile ch11 = profile_at(4);
    const Profile ch10 = profile_at(4, kLegacy);

    save_record(store, "h", ch11, at_cap(4));
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

TEST_CASE("a lookup reads a whole record while another thread rewrites it") {
    // A save deletes the record and inserts the new one in one transaction,
    // so a lookup landing in between sees the old record or the new one,
    // never no record and never a mix.
    RecordStore store(":memory:");
    const Profile at4 = profile_at(4);
    const RecordRow full = prepare_record("h", at4, at_cap(4)).record;
    const RecordRow empty = prepare_record("h", at4, no_paths_at_cap(4)).record;
    save_record(store, "h", at4, at_cap(4));

    std::atomic<bool> stop{false};
    std::thread writer([&] {
        for (int i = 0; i < 50; ++i)
            save_record(store, "h", at4, i % 2 ? at_cap(4) : no_paths_at_cap(4));
        stop.store(true);
    });
    int reads = 0;
    bool all_whole = true;
    std::string failure;
    do {
        try {
            const RecordLookup r = store.lookup_one("h", at4);
            if (r.status != RecordStatus::Ready) all_whole = false;
            else if (r.record != full && r.record != empty) all_whole = false;
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

TEST_CASE("ready_song_ids names exactly the charts lookup reads as Ready, profile by profile") {
    RecordStore store(":memory:");
    const std::vector<std::string> charts = {"ready", "stale", "other_lens", "other_cap",
                                             "other_mode"};
    save_record(store, "ready", profile_at(4), at_cap(4));
    save_old_build(store, "stale", profile_at(4), at_cap(4));
    save_record(store, "other_lens", profile_at(4, kLensB), at_cap(4));
    save_record(store, "other_cap", profile_at(8), at_cap(8));
    save_record(store, "other_mode", profile_at(4, Lens{}, "other"), at_cap(4));

    for (int cap : {4, 8}) {
        const std::unordered_set<std::string> got = store.ready_song_ids(profile_at(cap));
        const std::vector<RecordLookup> found = store.lookup(charts, profile_at(cap));
        for (size_t i = 0; i < charts.size(); ++i) {
            INFO(charts[i] << " at " << cap);
            CHECK((got.count(charts[i]) == 1) == (found[i].status == RecordStatus::Ready));
        }
    }
    CHECK(store.ready_song_ids(profile_at(4)) == std::unordered_set<std::string>{"ready"});
}

// D79: the batch's skip list is the library's Analyzed chip. Every kind of
// record a chart can hold is here, including a chart with a Ready record
// beside one made under other rules, and a repeated library id.
TEST_CASE("ready_song_ids names exactly the charts a library page reads as Ready") {
    RecordStore store(":memory:");
    const Profile at8 = profile_at(8);
    const std::vector<std::string> charts = {"ready", "build", "rules", "both", "mixed", "none"};
    save_record(store, "ready", at8, at_cap(8));
    test::add_stale_records(store, at_cap(8), at8, "build", "rules", "both");
    save_record(store, "mixed", at8, at_cap(8));
    save_record(store, "mixed", at8, test::other_rules_record(at_cap(8)));

    std::vector<std::string> library = charts;
    library.push_back("ready");  // a second folder's copy
    const std::vector<RecordLookup> chips = store.lookup(library, at8);
    const std::unordered_set<std::string> skip = store.ready_song_ids(at8);
    for (size_t i = 0; i < library.size(); ++i) {
        INFO(library[i]);
        CHECK((chips[i].status == RecordStatus::Ready) == (skip.count(library[i]) == 1));
    }
    CHECK(skip == std::unordered_set<std::string>{"ready", "mixed"});
}

TEST_CASE("lookup answers a page the same as lookup_one chart by chart") {
    RecordStore store(":memory:");
    save_record(store, "ready", profile_at(4), at_cap(4));
    save_old_build(store, "stale", profile_at(4), at_cap(4));
    save_record(store, "two_caps", profile_at(32), at_cap(32));
    save_record(store, "two_caps", profile_at(16), at_cap(16));

    // "ready" twice: a page can list the same chart from two folders. Each
    // page's answers are spelled out, since lookup_one is built on lookup and
    // agreeing with it alone proves nothing.
    const std::vector<std::string> page = {"ready", "stale", "none", "two_caps", "ready"};
    using S = RecordStatus;
    const std::vector<std::pair<int, std::vector<S>>> want = {
        {4, {S::Ready, S::Stale, S::NotAnalyzed, S::NotAnalyzed, S::Ready}},
        {16, {S::NotAnalyzed, S::NotAnalyzed, S::NotAnalyzed, S::Ready, S::NotAnalyzed}},
        {32, {S::NotAnalyzed, S::NotAnalyzed, S::NotAnalyzed, S::Ready, S::NotAnalyzed}},
    };
    for (const auto& [cap, statuses] : want) {
        const std::vector<RecordLookup> got = store.lookup(page, profile_at(cap));
        REQUIRE(got.size() == page.size());
        for (size_t i = 0; i < page.size(); ++i) {
            INFO(page[i] << " at " << cap);
            CHECK(got[i].status == statuses[i]);
            CHECK(got[i].status == store.lookup_one(page[i], profile_at(cap)).status);
            // Every Ready record here holds the fixture's paths, so its best
            // path is the fixture's.
            CHECK(got[i].record.best_path_text() == (statuses[i] == S::Ready
                                                         ? fixture().record.best_path().pathstring()
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

TEST_CASE("lookup answers a whole library in chunks") {
    // SQLite refuses more than 32,766 bound values in one statement. The
    // library asks about every chart at once, so the store must split it.
    RecordStore store(":memory:");
    save_record(store, "ready", profile_at(4), at_cap(4));

    std::vector<std::string> ids;
    for (int i = 0; i < 20000; ++i) {
        char h[16];
        std::snprintf(h, sizeof(h), "fake%05d", i);
        ids.emplace_back(h);
    }
    // The real chart at the start, the middle and the end, so it lands in
    // more than one chunk's position.
    for (size_t at : {size_t{0}, size_t{10000}, size_t{19999}}) ids[at] = "ready";

    const auto t0 = std::chrono::steady_clock::now();
    const std::vector<RecordLookup> got = store.lookup(ids, profile_at(4));
    const double ms =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    MESSAGE("lookup on 20,000 ids: " << ms << " ms");

    REQUIRE(got.size() == ids.size());
    int ready = 0;
    for (size_t i = 0; i < got.size(); ++i) {
        if (ids[i] == "ready") {
            CHECK(got[i].status == RecordStatus::Ready);
            CHECK(got[i].record.stars.has_value());
            ++ready;
        } else if (got[i].status != RecordStatus::NotAnalyzed) {
            FAIL("fake id " << ids[i] << " read as analyzed");
        }
    }
    CHECK(ready == 3);
}

TEST_CASE("PathSummary equality covers every field, stars included") {
    // Finding 121: one compare for every field, unset against set counting
    // as a change.
    PathSummary filled;
    filled.score = 1000;
    filled.actcount = 2;
    filled.maxskip = 1;
    filled.hardest_ms = 12.5;
    filled.avgmult = 3.25;
    filled.notecount = 40;
    filled.sqin_count = 1;
    filled.sqout_count = 2;
    filled.pathcount = 3;
    filled.stars = 5;

    const PathSummary copy = filled;
    CHECK(copy == filled);
    CHECK_FALSE(copy != filled);

    auto differs = [&filled](void (*change)(PathSummary&)) {
        PathSummary other = filled;
        change(other);
        return other != filled && !(other == filled);
    };
    CHECK(differs([](PathSummary& s) { s.score = 1001; }));
    CHECK(differs([](PathSummary& s) { s.actcount = 3; }));
    CHECK(differs([](PathSummary& s) { s.maxskip = 2; }));
    CHECK(differs([](PathSummary& s) { s.hardest_ms = 13.0; }));
    CHECK(differs([](PathSummary& s) { s.avgmult = 3.5; }));
    CHECK(differs([](PathSummary& s) { s.notecount = 41; }));
    CHECK(differs([](PathSummary& s) { s.sqin_count = 2; }));
    CHECK(differs([](PathSummary& s) { s.sqout_count = 3; }));
    CHECK(differs([](PathSummary& s) { s.pathcount = 4; }));
    CHECK(differs([](PathSummary& s) { s.stars = 4; }));
    CHECK(differs([](PathSummary& s) { s.stars.reset(); }));
    CHECK(differs([](PathSummary& s) { s.score.reset(); }));
    CHECK(PathSummary{} == PathSummary{});
}

TEST_CASE("a record counts every kept path once") {
    // Finding 172: the pathcount column is the count the Paths tab shows
    // ("Paths kept:", HydraRecord::all_paths). The fixture's count is a
    // literal from one run on the base before the fold, so the two counts are
    // proven equal on a real record.
    CHECK(record_of("", fixture().record).pathcount == 1);
    // One root with a tied variant, plus a second root: three paths.
    CHECK(record_of("", test::tied_variant_record()).pathcount == 3);
}
