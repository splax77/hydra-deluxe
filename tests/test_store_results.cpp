// Tests for store/record_store.{h,cpp}: a result's summary row reads back as
// it was saved, and each lookup picks the row its settings name. The library
// table and scan are in test_store_library.cpp; how the store opens,
// upgrades and fails is in test_store_open.cpp.

#include "doctest.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <system_error>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/stars.h"
#include "corpus_util.h"
#include "db_file_util.h"  // scalar_on_file
#include "display_fixtures.h"  // add_stale_rows, old_build_row, other_rules_record
#include "library_fixtures.h"  // library_row
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/graph.h"
#include "search/pather.h"
#include "store/record_store.h"
#include "store/stored_versions.h"
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

}  // namespace

TEST_CASE("a saved result reads back its best path and summary across the corpus and config matrix") {
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
                continue;  // charts the engine rejects have no row to store
            }
            REQUIRE(record->sp_cap.has_value());
            const CapQuery cap = CapQuery::at(*record->sp_cap);

            const std::string hyhash = path + "|" + cfg.key;
            // The store refuses a key whose ms limit isn't the record's.
            const Lens lens = Lens::from(
                cfg.ms ? std::optional<int>(static_cast<int>(*cfg.ms)) : std::nullopt, 0, 0);
            const RecordKey key{hyhash, "mode", cap, lens};
            store.add_record(key, *record);

            const SummaryLookup got = store.get_summary(key);
            ++checks;
            std::string d;
            if (got.status != RecordStatus::Ready) d = "no row after add";
            else if (got.bestpath != best_path_text(*record)) d = "bestpath";
            else if (got.summary != summarize_record(*record)) d = "summary";
            if (!d.empty() && ++mismatches <= 8)
                CHECK_MESSAGE(false, path << " [" << cfg.key << "] " << d);
        }
    }

    CHECK(mismatches == 0);
    REQUIRE(checks > 0);
    MESSAGE("checked " << checks << " saved summaries");
}

TEST_CASE("RecordStore maintenance: has_record, list_records, counts") {
    app::AnalysisSettings settings;
    settings.sp_cap = 4;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.ms_filter = std::nullopt;
    const HydraRecord record = corpus::first_analyzed_with_paths(settings).record;

    const CapQuery at4 = CapQuery::at(4);
    RecordStore store(":memory:");
    CHECK_FALSE(store.has_record(RecordKey{"h1", "Expert Pro Drums, 2x Bass", at4}));

    store.rebuild_chart_library({library_row("h1", "Song A")});
    store.add_record(RecordKey{"h1", "Expert Pro Drums, 2x Bass", at4}, record);
    CHECK(store.has_record(RecordKey{"h1", "Expert Pro Drums, 2x Bass", at4}));
    CHECK_FALSE(store.has_record(RecordKey{"h1", "Expert Pro Drums, 2x Bass", CapQuery::at(8)}));

    std::vector<RecordListing> listing =
        store.list_records(std::nullopt, at4, Lens{}, SortColumn::Score, true);
    REQUIRE(listing.size() == 1);
    CHECK(listing[0].ref_name == "Song A");
    CHECK(listing[0].bestpath == record.best_path().pathstring());

    auto [ncharts, nrecords] = store.counts();
    CHECK(ncharts == 1);
    CHECK(nrecords == 1);

    // A row stamped with a different version is stale for this store: it
    // doesn't count as "already analyzed".
    store.add_row(test::old_build_row(RecordKey{"h2", "Expert Pro Drums, 2x Bass", at4}, record));
    CHECK(store.counts().second == 2);
    CHECK_FALSE(store.has_record(RecordKey{"h2", "Expert Pro Drums, 2x Bass", at4}));
}

TEST_CASE("RecordStore results stamp: every accepted stamp reads Ready, others Stale") {
    // The stamp is not the app version (ADR 0018). Only "2.8.0" reads
    // Ready. Results stamped "1.8.2" (every release from 1.8.4 to 2.0.0) or
    // "1.8.3" hold values the SP-end history changed (ADR 0021), earlier
    // 2.1.0 builds' "2.1.0" can miss the all-0 path (D85),
    // "2.1.0+allzero" predates the 2x kick merge (D105), and "2.4.0"
    // predates the measure-line fills and Star Power count (D115), so they
    // read Stale, through the C++ rule (get_summary) and its SQL twin
    // (has_record).
    CHECK(kResultsStamp.is_current("2.8.0"));
    CHECK_FALSE(kResultsStamp.is_current("2.4.0"));
    CHECK_FALSE(kResultsStamp.is_current("2.1.0+allzero"));
    CHECK_FALSE(kResultsStamp.is_current("2.1.0"));
    CHECK_FALSE(kResultsStamp.is_current("2.0.0"));
    CHECK_FALSE(kResultsStamp.is_current("1.8.2"));
    CHECK_FALSE(kResultsStamp.is_current("1.8.3"));
    CHECK(current_record_version() == std::string(kResultsStamp.written));

    const CapQuery at4 = CapQuery::at(4);
    const HydraRecord& record = fixture().record;
    REQUIRE_FALSE(record.paths.empty());

    RecordStore store(":memory:");
    const struct {
        const char* hash;
        const char* stamp;
        RecordStatus want;
    } cases[] = {{"a", "2.8.0", RecordStatus::Ready},
                 {"g", "2.4.0", RecordStatus::Stale},
                 {"f", "2.1.0+allzero", RecordStatus::Stale},
                 {"e", "2.1.0", RecordStatus::Stale},
                 {"b", "1.8.3", RecordStatus::Stale},
                 {"c", "1.8.2", RecordStatus::Stale},
                 {"d", "0.0.0", RecordStatus::Stale}};
    for (const auto& c : cases) {
        CAPTURE(c.stamp);
        const RecordKey key{c.hash, "Expert Pro Drums, 2x Bass", at4};
        PreparedRow row = prepare_row(key, record);
        row.hyversion = c.stamp;
        store.add_row(row);
        CHECK(store.get_summary(key).status == c.want);
        CHECK(store.has_record(key) == (c.want == RecordStatus::Ready));
    }
}

namespace {

// The two lenses the coexistence tests use: same chart, same cap, different
// searches. Lens C is a third nobody stored anything under.
const Lens kLensA = Lens::from(10, 0, 20);
const Lens kLensB = Lens::from(std::nullopt, 1, 5000);
const Lens kLensC = Lens::from(25, 0, 3);

// at_cap plus the ms limit lens A claims, so prepare_row's guard is satisfied.
HydraRecord at_cap_ms10(int cap) {
    HydraRecord r = at_cap(cap);
    r.ms_limit = 10.0;
    return r;
}

}  // namespace

TEST_CASE("records at different caps coexist; each lookup sees only its own cap") {
    RecordStore store(":memory:");
    store.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4));
    store.add_record(RecordKey{"h", "mode", CapQuery::at(32)}, at_cap(32));
    CHECK(store.counts().second == 2);

    // Exact lookups see exactly their cap.
    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(4)}).status ==
          RecordStatus::Ready);
    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(32)}).status ==
          RecordStatus::Ready);
    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(8)}).status ==
          RecordStatus::NotAnalyzed);

    // A stale 64-bar row leaves the 32-bar answer alone -- for single
    // lookups and for the listing alike.
    store.add_row(test::old_build_row(RecordKey{"h", "mode", CapQuery::at(64)}, at_cap(64)));
    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(32)}).status ==
          RecordStatus::Ready);
    std::vector<RecordListing> listed =
        store.list_records(std::nullopt, CapQuery::at(32), Lens{}, SortColumn::Score, true);
    REQUIRE(listed.size() == 1);
    CHECK(listed[0].sp_cap == 32);

    // Asked for cap 64 exactly, that stale row is reported as stale, with no
    // numbers handed out.
    const SummaryLookup stale_lookup = store.get_summary(RecordKey{"h", "mode", CapQuery::at(64)});
    CHECK(stale_lookup.status == RecordStatus::Stale);
    CHECK(stale_lookup.stale_build);
    CHECK_FALSE(stale_lookup.summary.score.has_value());

    // The listing leaves it out. A stale row takes its chart out of the
    // listing entirely, so a report counts it as never analyzed rather than
    // reading numbers this build cannot vouch for.
    CHECK(store.list_records(std::nullopt, CapQuery::at(64), Lens{}, SortColumn::Score, true)
              .empty());
    CHECK(store.list_records(std::nullopt, CapQuery::at(4), Lens{}, SortColumn::Score, true)[0]
              .summary.score == fixture().record.best_path().totalscore());
}

TEST_CASE("a row an old migration marked with unknown settings reads Not analyzed") {
    // Hydra 1.7 to 1.8.1 migrated older rows in with ms_enabled = -1: "a
    // result, settings unknown". No lens has -1, so such a row is no
    // candidate for any lookup. It reads as no row at all, not as Stale
    // (user decision 2026-09-26), and a real run of the chart replaces it.
    RecordStore store(":memory:");
    PreparedRow migrated = prepare_row(RecordKey{"h", "mode", CapQuery::at(8)}, at_cap(8));
    migrated.lens.ms_enabled = -1;
    migrated.hyversion = "1.6.0";
    store.add_row(migrated);
    CHECK(store.counts().second == 1);

    const RecordKey key{"h", "mode", CapQuery::at(8)};
    CHECK(store.get_summary(key).status == RecordStatus::NotAnalyzed);
    CHECK_FALSE(store.has_record(key));
    CHECK(store.list_records(std::nullopt, CapQuery::at(8), Lens{}, SortColumn::Score, true)
              .empty());

    store.add_record(key, at_cap(8));
    CHECK(store.counts().second == 1);
    CHECK(store.get_summary(key).status == RecordStatus::Ready);
}

TEST_CASE("a row analyzed under other rules reads Stale until the rules match again") {
    const core::Rules other = test::other_rules();
    const RecordKey key{"h", "mode", CapQuery::at(8)};

    // A store running the default rules sees a row stamped with other rules
    // as Stale everywhere a lookup can ask.
    {
        RecordStore store(":memory:");
        store.add_row(prepare_row(key, test::other_rules_record(at_cap(8))));
        CHECK_FALSE(store.has_record(key));
        CHECK(store.get_summary(key).status == RecordStatus::Stale);
        CHECK(store.list_records(std::nullopt, CapQuery::at(8), Lens{}, SortColumn::Score, true)
                  .empty());
    }

    // The same row reads Ready again once the store runs those rules.
    const std::string db = testtemp::temp_path("rules_fp", ".db");
    {
        RecordStore store(db);
        store.add_record(key, at_cap(8));
        CHECK(store.get_summary(key).status == RecordStatus::Ready);
    }
    {
        RecordStore store(db, core::RulesStamp::of(other));
        CHECK(store.get_summary(key).status == RecordStatus::Stale);
        CHECK_FALSE(store.has_record(key));
    }
    {
        // A store gated on "no usable rules" (a bad hydra_rules.ini) reads
        // nothing as Ready.
        RecordStore store(db, core::RulesStamp::none());
        CHECK(store.get_summary(key).status == RecordStatus::Stale);
    }
    {
        RecordStore store(db);
        CHECK(store.get_summary(key).status == RecordStatus::Ready);
    }
    std::error_code ec;
    std::filesystem::remove(std::filesystem::u8path(db), ec);
}

TEST_CASE("a write under rules A keeps the rules-B row") {
    // D51 call 8 (finding 65): a result made under other rules is kept, and
    // reads Ready again once the store runs those rules.
    const core::Rules other = test::other_rules();
    const RecordKey key{"h", "mode", CapQuery::at(8)};
    const std::string db = testtemp::temp_path("keep_rules_b", ".db");
    std::remove(db.c_str());
    {
        RecordStore store(db);
        store.add_row(prepare_row(key, test::other_rules_record(at_cap(8))));
    }
    {
        RecordStore store(db);
        store.add_record(key, at_cap(8));
        CHECK(store.counts().second == 2);
        CHECK(store.get_summary(key).status == RecordStatus::Ready);
    }
    {
        RecordStore store(db, core::RulesStamp::of(other));
        const SummaryLookup lookup = store.get_summary(key);
        CHECK(lookup.status == RecordStatus::Ready);
        CHECK(lookup.summary == summarize_record(fixture().record));
        CHECK(store.has_record(key));
    }
    std::remove(db.c_str());
}

TEST_CASE("a Stale lookup says why: another build, other rules, or both") {
    RecordStore store(":memory:");
    const RecordKey build_key{"h", "build", CapQuery::at(8)};
    const RecordKey rules_key{"h", "rules", CapQuery::at(8)};
    const RecordKey both_key{"h", "both", CapQuery::at(8)};
    test::add_stale_rows(store, at_cap(8), build_key, rules_key, both_key);

    // This build, other rules.
    const SummaryLookup by_rules = store.get_summary(rules_key);
    CHECK(by_rules.status == RecordStatus::Stale);
    CHECK(by_rules.stale_rules);
    CHECK_FALSE(by_rules.stale_build);

    // Another build, these rules.
    const SummaryLookup by_build = store.get_summary(build_key);
    CHECK(by_build.status == RecordStatus::Stale);
    CHECK(by_build.stale_build);
    CHECK_FALSE(by_build.stale_rules);

    // Another build and other rules: both reasons.
    const SummaryLookup by_both = store.get_summary(both_key);
    CHECK(by_both.status == RecordStatus::Stale);
    CHECK(by_both.stale_build);
    CHECK(by_both.stale_rules);

    // A Ready row carries no reason.
    const RecordKey ready_key{"h", "ready", CapQuery::at(8)};
    store.add_record(ready_key, at_cap(8));
    const SummaryLookup ready = store.get_summary(ready_key);
    CHECK(ready.status == RecordStatus::Ready);
    CHECK_FALSE(ready.stale_build);
    CHECK_FALSE(ready.stale_rules);
}

TEST_CASE("the listing and a lookup agree on which row is a chart's answer") {
    // The lock between the two paths. Each chart below holds rows at several
    // caps. At every cap, whatever get_summary picks is what the listing must
    // show, and when that pick is not readable the chart must not be listed.
    RecordStore store(":memory:");
    const std::vector<const char*> charts = {"two_caps", "over_stale", "over_sentinel",
                                             "all_stale"};

    // Two current rows at two caps.
    store.add_record(RecordKey{"two_caps", "mode", CapQuery::at(32)}, at_cap(32));
    store.add_record(RecordKey{"two_caps", "mode", CapQuery::at(16)}, at_cap(16));

    // A current row and a taller stale one. The stale row goes in second
    // because a current-version write purges the chart's other-version rows.
    store.add_record(RecordKey{"over_stale", "mode", CapQuery::at(8)}, at_cap(8));
    store.add_row(test::old_build_row(RecordKey{"over_stale", "mode", CapQuery::at(64)}, at_cap(64)));

    // A current row and a taller row an old migration left: the migrated row
    // is no candidate at all.
    PreparedRow sentinel =
        prepare_row(RecordKey{"over_sentinel", "mode", CapQuery::at(64)}, at_cap(64));
    sentinel.lens.ms_enabled = -1;
    store.add_row(sentinel);
    store.add_record(RecordKey{"over_sentinel", "mode", CapQuery::at(8)}, at_cap(8));

    // Nothing readable at all.
    store.add_row(test::old_build_row(RecordKey{"all_stale", "mode", CapQuery::at(16)}, at_cap(16)));

    auto listed_at = [&](int cap) {
        std::unordered_map<std::string, int> listed;
        for (const RecordListing& r : store.list_records(std::nullopt, CapQuery::at(cap),
                                                         Lens{}, SortColumn::Score, true))
            listed[r.hyhash] = r.sp_cap;
        return listed;
    };

    for (int cap : {8, 16, 32, 64}) {
        std::unordered_map<std::string, int> listed = listed_at(cap);
        for (const char* hash : charts) {
            INFO(hash << " at " << cap);
            const SummaryLookup rec = store.get_summary(RecordKey{hash, "mode", CapQuery::at(cap)});
            if (rec.status == RecordStatus::Ready) {
                REQUIRE(listed.count(hash) == 1);
                CHECK(listed[hash] == cap);
            } else {
                CHECK(listed.count(hash) == 0);
            }
        }
    }

    // Spelled out, so a change that moves both paths together still has to
    // answer for itself.
    CHECK(listed_at(16).at("two_caps") == 16);
    CHECK(listed_at(32).at("two_caps") == 32);
    CHECK(listed_at(8).at("over_stale") == 8);
    CHECK(listed_at(64).count("over_stale") == 0);
    CHECK(listed_at(8).at("over_sentinel") == 8);
    CHECK(listed_at(64).count("over_sentinel") == 0);
    CHECK(store.get_summary(RecordKey{"all_stale", "mode", CapQuery::at(16)}).status ==
          RecordStatus::Stale);
}

TEST_CASE("prepare_row refuses a key whose exact cap isn't the record's") {
    HydraRecord rec = at_cap(4);

    // An exact key that disagrees with the record would file the result under
    // a cap it was never analyzed at.
    CHECK_THROWS_AS(prepare_row(RecordKey{"h", "mode", CapQuery::at(8)}, rec),
                    std::invalid_argument);

    // The matching key is fine.
    CHECK(prepare_row(RecordKey{"h", "mode", CapQuery::at(4)}, rec).sp_cap == 4);
}

TEST_CASE("prepare_row refuses a key whose ms limit isn't the record's") {
    // Same failure mode as the cap guard: the row would claim settings the
    // search never ran under, and every later lookup would believe it.
    HydraRecord none = at_cap(4);  // analyzed with no ms limit
    CHECK_THROWS_AS(prepare_row(RecordKey{"h", "mode", CapQuery::at(4), kLensA}, none),
                    std::invalid_argument);

    HydraRecord ten = at_cap_ms10(4);
    CHECK_THROWS_AS(prepare_row(RecordKey{"h", "mode", CapQuery::at(4), kLensC}, ten),
                    std::invalid_argument);

    // The matching lens is fine, and rides onto the row.
    PreparedRow row = prepare_row(RecordKey{"h", "mode", CapQuery::at(4), kLensA}, ten);
    CHECK(row.lens == kLensA);

    // The limit is checked both ways (finding 130): a record analyzed with a
    // limit is refused under a lens that has the limit off, too.
    CHECK_THROWS_AS(prepare_row(RecordKey{"h", "mode", CapQuery::at(4), kLensB}, ten),
                    std::invalid_argument);
}

TEST_CASE("prepare_row files the row under the record's own rules fingerprint") {
    // The rules_fp column is what rank_row compares against this process's
    // rules, so it must be the record's, never the store's.
    CHECK(prepare_row(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4)).rules_fp ==
          fixture().record.rules_fingerprint);
    const HydraRecord foreign = test::other_rules_record(at_cap(4));
    CHECK(prepare_row(RecordKey{"h", "mode", CapQuery::at(4)}, foreign).rules_fp ==
          foreign.rules_fingerprint);
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
        store.add_row(prepare_row(RecordKey{"h", "mode", CapQuery::at(4)}, rec));
    }
    CHECK(scalar_on_file(db, "SELECT COUNT(*) FROM profiles WHERE hex(rules_fp) = '0807060504030201'") ==
          1);
    std::error_code ec;
    std::filesystem::remove(std::filesystem::u8path(db), ec);
}

TEST_CASE("RecordKey compares on every part of the identity") {
    const RecordKey key{"h", "mode", CapQuery::at(4), kLensA};
    CHECK(key == RecordKey{"h", "mode", CapQuery::at(4), kLensA});
    CHECK_FALSE(key == RecordKey{"other", "mode", CapQuery::at(4), kLensA});
    CHECK_FALSE(key == RecordKey{"h", "other mode", CapQuery::at(4), kLensA});
    CHECK_FALSE(key == RecordKey{"h", "mode", CapQuery::at(8), kLensA});
    CHECK_FALSE(key == RecordKey{"h", "mode", CapQuery::at(4), kLensB});
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

TEST_CASE("a current-version record with no paths is Ready, not Stale") {
    RecordStore store(":memory:");
    store.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, no_paths_at_cap(4));

    // "Analyzed, and nothing survived" is a real result, so it reads back as
    // Ready with no summary -- the status, not the path count, is what says
    // whether a row is usable.
    const SummaryLookup lookup = store.get_summary(RecordKey{"h", "mode", CapQuery::at(4)});
    CHECK(lookup.status == RecordStatus::Ready);
    CHECK(lookup.bestpath.empty());
    CHECK(lookup.summary == PathSummary{});
    CHECK(store.has_record(RecordKey{"h", "mode", CapQuery::at(4)}));

    // D51 call 11 (finding 88): such a record has no scored best path, and
    // the summary says so in one place. A record with paths has one. The
    // listing's row, read back from the summary columns, says the same.
    CHECK_FALSE(summarize_record(no_paths_at_cap(4)).has_scored_best_path());
    CHECK(summarize_record(at_cap(4)).has_scored_best_path());
    store.add_record(RecordKey{"g", "mode", CapQuery::at(4)}, at_cap(4));
    const std::vector<RecordListing> rows =
        store.list_records(std::nullopt, CapQuery::at(4), Lens{}, SortColumn::RefName, false);
    REQUIRE(rows.size() == 2);
    for (const RecordListing& row : rows)
        CHECK(row.summary.has_scored_best_path() == (row.hyhash == "g"));
}

// ---- lens identity --------------------------------------------------------
//
// The lens is the rest of a result's identity: the ms limit and the score
// range it ran under. These cases pin that two lenses coexist and that each
// lookup gets its own answer.

TEST_CASE("the same chart at the same cap keeps one result per lens") {
    RecordStore store(":memory:");

    // The two records differ in the one field that says which search ran:
    // lens A's ms limit is 10, lens B's is off.
    store.add_record(RecordKey{"h", "mode", CapQuery::at(4), kLensA}, at_cap_ms10(4));
    store.add_record(RecordKey{"h", "mode", CapQuery::at(4), kLensB}, at_cap(4));
    CHECK(store.counts().second == 2);

    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(4), kLensA}).status ==
          RecordStatus::Ready);
    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(4), kLensB}).status ==
          RecordStatus::Ready);

    // A lens nobody ran under has no answer here, and no stored row stands in
    // for it -- this is what stops a batch run skipping the chart.
    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(4), kLensC}).status ==
          RecordStatus::NotAnalyzed);
    CHECK(store.has_record(RecordKey{"h", "mode", CapQuery::at(4), kLensA}));
    CHECK(store.has_record(RecordKey{"h", "mode", CapQuery::at(4), kLensB}));
    CHECK_FALSE(store.has_record(RecordKey{"h", "mode", CapQuery::at(4), kLensC}));

    // Re-running one lens replaces that row and leaves the other alone.
    HydraRecord redone = at_cap_ms10(4);
    redone.paths.clear();
    redone.allzero_paths.clear();
    store.add_record(RecordKey{"h", "mode", CapQuery::at(4), kLensA}, redone);
    CHECK(store.counts().second == 2);
    CHECK_FALSE(store.get_summary(RecordKey{"h", "mode", CapQuery::at(4), kLensA})
                    .summary.has_scored_best_path());
    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(4), kLensB}).summary ==
          summarize_record(at_cap(4)));
}

TEST_CASE("a current-version write purges the chart's old-version rows") {
    RecordStore store(":memory:");
    for (const char* hash : {"h", "other"}) {
        store.add_row(
            test::old_build_row(RecordKey{hash, "mode", CapQuery::at(4), kLensB}, at_cap(4)));
    }
    CHECK(store.counts().second == 2);

    HydraRecord fresh = at_cap_ms10(32);
    fresh.paths.clear();
    fresh.allzero_paths.clear();
    store.add_record(RecordKey{"h", "mode", CapQuery::at(32), kLensA}, fresh);

    // This build cannot read what another version wrote for this chart, so
    // the write supersedes it -- at every cap and lens...
    CHECK(store.get_summary(RecordKey{"h", "mode", CapQuery::at(4), kLensB}).status ==
          RecordStatus::NotAnalyzed);
    // ...while another chart's old row is none of this write's business.
    CHECK(store.get_summary(RecordKey{"other", "mode", CapQuery::at(4), kLensB}).status ==
          RecordStatus::Stale);
    CHECK(store.counts().second == 2);
}

// ---- store correctness (2026-09-26 audit, Task 1) --------------------------

TEST_CASE("store: list_records reads its summary columns from the one list") {
    // list_records works out where the cap, version, id and rules sit from
    // kSummaryColumnList's count. A slot that drifts from the list reads
    // another column, so the listing stops matching the lookup.
    RecordStore store(":memory:");
    const RecordKey key{"h", "mode", CapQuery::at(4)};
    store.add_record(key, at_cap(4));
    const SummaryLookup lookup = store.get_summary(key);
    REQUIRE(lookup.status == RecordStatus::Ready);
    const std::vector<RecordListing> listing =
        store.list_records(std::nullopt, CapQuery::at(4), Lens{}, SortColumn::Score, true);
    REQUIRE(listing.size() == 1);
    CHECK(listing[0].summary == lookup.summary);
    CHECK(listing[0].sp_cap == 4);
    CHECK(listing[0].hyhash == "h");
    CHECK(listing[0].bestpath == lookup.bestpath);
}

// ---- the fill rule in the key (docs/adr/0010) ------------------------------

namespace {

// The default Lens{} but for the fill rule, so a key differs from its 1.1
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
    const RecordKey ch11{"h", "mode", CapQuery::at(4)};
    const RecordKey ch10{"h", "mode", CapQuery::at(4), kLegacy};

    store.add_record(ch11, at_cap(4));
    CHECK(store.get_summary(ch10).status == RecordStatus::NotAnalyzed);
    CHECK_FALSE(store.has_record(ch10));

    // Writing the 1.0 result keeps the 1.1 one, and each lookup finds its own.
    store.add_record(ch10, legacy_at_cap(4));
    CHECK(store.counts().second == 2);
    CHECK(store.get_summary(ch10).status == RecordStatus::Ready);
    CHECK(store.get_summary(ch11).status == RecordStatus::Ready);
    CHECK(store.analyzed_hashes(ResultSettings{"mode", CapQuery::at(4), kLegacy}).count("h") == 1);

    // Re-analyzing under 1.0 replaces only the 1.0 row.
    store.add_record(ch10, legacy_at_cap(4));
    CHECK(store.counts().second == 2);
    CHECK(store.get_summary(ch11).status == RecordStatus::Ready);
}

TEST_CASE("prepare_row refuses a key that names the other fill rule") {
    const RecordKey ch10{"h", "mode", CapQuery::at(4), kLegacy};
    const RecordKey ch11{"h", "mode", CapQuery::at(4)};
    CHECK_THROWS_AS(prepare_row(ch10, at_cap(4)), std::invalid_argument);
    CHECK_THROWS_AS(prepare_row(ch11, legacy_at_cap(4)), std::invalid_argument);
    CHECK(prepare_row(ch10, legacy_at_cap(4)).lens.legacy_fills == 1);
}

TEST_CASE("get_summary reads a whole row while another thread rewrites it") {
    // A rewrite deletes the row and inserts the new one in one transaction,
    // so a lookup landing in between sees the old row or the new one, never
    // no row and never a mix.
    RecordStore store(":memory:");
    const RecordKey key{"h", "mode", CapQuery::at(4)};
    store.add_record(key, at_cap(4));
    const PathSummary full = summarize_record(at_cap(4));
    const PathSummary empty = summarize_record(no_paths_at_cap(4));

    std::atomic<bool> stop{false};
    std::thread writer([&] {
        for (int i = 0; i < 50; ++i) store.add_record(key, i % 2 ? at_cap(4) : no_paths_at_cap(4));
        stop.store(true);
    });
    int reads = 0;
    bool all_whole = true;
    std::string failure;
    do {
        try {
            const SummaryLookup r = store.get_summary(key);
            if (r.status != RecordStatus::Ready) all_whole = false;
            else if (r.summary != full && r.summary != empty) all_whole = false;
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

TEST_CASE("has_record and a lookup agree on which rows are readable") {
    // has_record asks get_summary (D79). This pins that across every kind of
    // row, so a second spelling of the Ready rule there would show up here.
    RecordStore store(":memory:");

    const RecordKey ready{"h", "ready", CapQuery::at(8)};
    store.add_record(ready, at_cap(8));

    const RecordKey old_build{"h", "build", CapQuery::at(8)};
    const RecordKey other_rules{"h", "rules", CapQuery::at(8)};
    const RecordKey both{"h", "both", CapQuery::at(8)};
    test::add_stale_rows(store, at_cap(8), old_build, other_rules, both);

    for (const RecordKey& key : {ready, old_build, other_rules, both}) {
        INFO(key.chartmode);
        CHECK(store.has_record(key) ==
              (store.get_summary(key).status == RecordStatus::Ready));
    }
    CHECK(store.has_record(ready));
}

TEST_CASE("save_analysis writes the result") {
    RecordStore store(":memory:");
    const RecordKey key{"h", "mode", CapQuery::at(4)};
    store.save_analysis(prepare_row(key, at_cap(4)));
    CHECK(store.counts() == std::pair<int64_t, int64_t>{1, 1});
    const SummaryLookup got = store.get_summary(key);
    CHECK(got.status == RecordStatus::Ready);
    CHECK(got.summary == summarize_record(at_cap(4)));
}

TEST_CASE("analyzed_hashes names exactly the charts has_record would skip") {
    RecordStore store(":memory:");
    const std::vector<const char*> charts = {"ready", "stale", "other_lens", "other_cap",
                                             "other_mode"};
    store.add_record(RecordKey{"ready", "mode", CapQuery::at(4)}, at_cap(4));
    store.add_row(test::old_build_row(RecordKey{"stale", "mode", CapQuery::at(4)}, at_cap(4)));
    store.add_record(RecordKey{"other_lens", "mode", CapQuery::at(4), kLensB}, at_cap(4));
    store.add_record(RecordKey{"other_cap", "mode", CapQuery::at(8)}, at_cap(8));
    store.add_record(RecordKey{"other_mode", "other", CapQuery::at(4)}, at_cap(4));

    for (const CapQuery& cap : {CapQuery::at(4), CapQuery::at(8)}) {
        const std::unordered_set<std::string> got =
            store.analyzed_hashes(ResultSettings{"mode", cap, Lens{}});
        for (const char* h : charts) {
            INFO(h);
            CHECK((got.count(h) == 1) == store.has_record(RecordKey{h, "mode", cap}));
        }
    }
    CHECK(store.analyzed_hashes(ResultSettings{"mode", CapQuery::at(4), Lens{}}) ==
          std::unordered_set<std::string>{"ready"});
}

// D79: the batch's skip list is the library's Analyzed chip. Every kind of row
// a chart can hold is here, including a chart with a Ready row beside a row
// made under other rules, and a repeated library hash.
TEST_CASE("analyzed_hashes names exactly the charts get_summaries reads as Ready") {
    RecordStore store(":memory:");
    const CapQuery cap = CapQuery::at(8);
    const std::vector<std::string> charts = {"ready", "build", "rules", "both", "mixed", "none"};
    store.add_record(RecordKey{"ready", "mode", cap}, at_cap(8));
    test::add_stale_rows(store, at_cap(8), RecordKey{"build", "mode", cap},
                         RecordKey{"rules", "mode", cap}, RecordKey{"both", "mode", cap});
    store.add_record(RecordKey{"mixed", "mode", cap}, at_cap(8));
    store.add_row(prepare_row(RecordKey{"mixed", "mode", cap}, test::other_rules_record(at_cap(8))));

    std::vector<std::string> library = charts;
    library.push_back("ready");  // a second folder's copy
    const ResultSettings want{"mode", cap, Lens{}};
    const std::vector<SummaryLookup> chips = store.get_summaries(library, want);
    const std::unordered_set<std::string> skip = store.analyzed_hashes(want);
    for (size_t i = 0; i < library.size(); ++i) {
        INFO(library[i]);
        CHECK((chips[i].status == RecordStatus::Ready) == (skip.count(library[i]) == 1));
    }
    CHECK(skip == std::unordered_set<std::string>{"ready", "mixed"});
}

TEST_CASE("get_summaries answers a page the same as get_summary row by row") {
    RecordStore store(":memory:");
    store.add_record(RecordKey{"ready", "mode", CapQuery::at(4)}, at_cap(4));
    store.add_row(test::old_build_row(RecordKey{"stale", "mode", CapQuery::at(4)}, at_cap(4)));
    store.add_record(RecordKey{"two_caps", "mode", CapQuery::at(32)}, at_cap(32));
    store.add_record(RecordKey{"two_caps", "mode", CapQuery::at(16)}, at_cap(16));

    // "ready" twice: a page can list the same chart from two folders. Each
    // page's answers are spelled out, since get_summary is built on
    // get_summaries and agreeing with it alone proves nothing.
    const std::vector<std::string> page = {"ready", "stale", "none", "two_caps", "ready"};
    using S = RecordStatus;
    const std::vector<std::pair<int, std::vector<S>>> want = {
        {4, {S::Ready, S::Stale, S::NotAnalyzed, S::NotAnalyzed, S::Ready}},
        {16, {S::NotAnalyzed, S::NotAnalyzed, S::NotAnalyzed, S::Ready, S::NotAnalyzed}},
        {32, {S::NotAnalyzed, S::NotAnalyzed, S::NotAnalyzed, S::Ready, S::NotAnalyzed}},
    };
    for (const auto& [cap, statuses] : want) {
        const std::vector<SummaryLookup> got =
            store.get_summaries(page, ResultSettings{"mode", CapQuery::at(cap), Lens{}});
        REQUIRE(got.size() == page.size());
        for (size_t i = 0; i < page.size(); ++i) {
            INFO(page[i] << " at " << cap);
            CHECK(got[i].status == statuses[i]);
            CHECK(got[i].status ==
                  store.get_summary(RecordKey{page[i], "mode", CapQuery::at(cap)}).status);
            // Every Ready row here holds the fixture's paths, so its best
            // path is the fixture's.
            CHECK(got[i].bestpath == (statuses[i] == S::Ready
                                          ? fixture().record.best_path().pathstring()
                                          : std::string()));
        }
    }
    CHECK(store.get_summaries({}, ResultSettings{"mode", CapQuery::at(4), Lens{}}).empty());
}

// ---- library summaries (interface redesign, Task 7) ------------------------

TEST_CASE("a saved result stores its best path's star count") {
    RecordStore store(":memory:");
    store.add_record(RecordKey{"ready", "mode", CapQuery::at(4)}, at_cap(4));
    store.add_row(test::old_build_row(RecordKey{"stale", "mode", CapQuery::at(4)}, at_cap(4)));

    const Path& best = fixture().record.best_path();
    const PathSummary expected = summarize_record(fixture().record);
    REQUIRE(expected.stars.has_value());
    CHECK(*expected.stars == path_stars(best));

    const std::vector<SummaryLookup> got =
        store.get_summaries({"ready", "stale", "none"},
                            ResultSettings{"mode", CapQuery::at(4), Lens{}});
    REQUIRE(got.size() == 3);
    CHECK(got[0].status == RecordStatus::Ready);
    CHECK(got[0].summary.score == best.totalscore());
    CHECK(got[0].summary.hardest_ms == expected.hardest_ms);
    CHECK(got[0].summary.stars == expected.stars);
    CHECK(got[0].summary == expected);
    // Only a Ready answer carries a summary.
    CHECK(got[1].status == RecordStatus::Stale);
    CHECK_FALSE(got[1].summary.score.has_value());
    CHECK(got[2].status == RecordStatus::NotAnalyzed);
    CHECK_FALSE(got[2].summary.stars.has_value());

    const std::vector<RecordListing> listing =
        store.list_records(std::nullopt, CapQuery::at(4), Lens{}, SortColumn::Score, true);
    REQUIRE(listing.size() == 1);
    CHECK(listing[0].summary.stars == expected.stars);
    CHECK(listing[0].sp_cap == 4);
}

TEST_CASE("get_summaries answers a whole library in chunks") {
    // SQLite refuses more than 32,766 bound values in one statement. The
    // library asks about every chart at once, so the store must split it.
    RecordStore store(":memory:");
    store.add_record(RecordKey{"ready", "mode", CapQuery::at(4)}, at_cap(4));

    std::vector<std::string> hashes;
    for (int i = 0; i < 20000; ++i) {
        char h[16];
        std::snprintf(h, sizeof(h), "fake%05d", i);
        hashes.emplace_back(h);
    }
    // The real chart at the start, the middle and the end, so it lands in
    // more than one chunk's position.
    for (size_t at : {size_t{0}, size_t{10000}, size_t{19999}}) hashes[at] = "ready";

    const auto t0 = std::chrono::steady_clock::now();
    const std::vector<SummaryLookup> got =
        store.get_summaries(hashes, ResultSettings{"mode", CapQuery::at(4), Lens{}});
    const double ms =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    MESSAGE("get_summaries on 20,000 hashes: " << ms << " ms");

    REQUIRE(got.size() == hashes.size());
    int ready = 0;
    for (size_t i = 0; i < got.size(); ++i) {
        if (hashes[i] == "ready") {
            CHECK(got[i].status == RecordStatus::Ready);
            CHECK(got[i].summary.stars.has_value());
            ++ready;
        } else if (got[i].status != RecordStatus::NotAnalyzed) {
            FAIL("fake hash " << hashes[i] << " read as analyzed");
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

TEST_CASE("summarize_record counts every kept path once") {
    // Finding 172: the pathcount column is the count the Paths tab shows
    // ("Paths kept:", HydraRecord::all_paths). The fixture's count is a
    // literal from one run on the base before the fold, so the two counts are
    // proven equal on a real record.
    CHECK(summarize_record(fixture().record).pathcount.value() == 1);
    // One root with a tied variant, plus a second root: three paths.
    CHECK(summarize_record(test::tied_variant_record()).pathcount == 3);
}
