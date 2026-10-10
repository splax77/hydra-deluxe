// Tests for store/record_store.{h,cpp}: a result's summary row reads back as
// it was saved, each lookup picks the row its settings name, and an older
// file upgrades by copy-and-swap with real progress (D87, DBUP), whatever
// point a stop leaves it at.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <sqlite3.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <system_error>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/user_messages.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/stars.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "db_file_util.h"  // exec_on_file, scalar_on_file, write_junk_db
#include "display_fixtures.h"  // add_stale_rows, old_build_row, other_rules_record
#include "library_fixtures.h"  // library_row
#include "old_layout_fixture.h"  // detail_layout_sql and the old layout's counts
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/graph.h"
#include "search/pather.h"
#include "store/record_store.h"
#include "store/stored_versions.h"
#include "store/upgrade_files.h"
#include "temp_util.h"

using namespace hydra;
using namespace hydra::store;

using hydra::test::exec_on_file;
using hydra::test::write_junk_db;
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
        const Song& song = corpus::song(path, true, true);
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

namespace {

// One analyzed corpus chart, for the stamp and cap-identity tests below: the
// first chart with paths (corpus_util.h), its record at 4 bars.
const app::AnalysisResult& fixture() {
    static const app::AnalysisResult f = [] {
        app::AnalysisSettings settings;
        settings.sp_cap = 4;
        settings.depth_mode = DepthMode::Scores;
        settings.depth_value = 0;
        settings.ms_filter = std::nullopt;
        return corpus::first_analyzed_with_paths(settings);
    }();
    return f;
}

}  // namespace

TEST_CASE("RecordStore results stamp: every accepted stamp reads Ready, others Stale") {
    // The stamp is not the app version (ADR 0018). Only "2.4.0" reads
    // Ready. Results stamped "1.8.2" (every release from 1.8.4 to 2.0.0) or
    // "1.8.3" hold values the SP-end history changed (ADR 0021), earlier
    // 2.1.0 builds' "2.1.0" can miss the all-0 path (D85), and
    // "2.1.0+allzero" predates the 2x kick merge (D105), so they read Stale,
    // through the C++ rule (get_summary) and its SQL twin (has_record).
    CHECK(kResultsStamp.is_current("2.4.0"));
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
    } cases[] = {{"a", "2.4.0", RecordStatus::Ready},
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

// The same record relabeled as if it had run at another cap. The paths are
// the 4-bar paths, which is fine: these tests check which row a lookup picks,
// not what is in it.
HydraRecord at_cap(int cap) {
    HydraRecord r = fixture().record;
    r.sp_cap = cap;
    return r;
}

// at_cap with every path taken out: "analyzed, and nothing survived".
HydraRecord no_paths_at_cap(int cap) {
    HydraRecord r = at_cap(cap);
    r.paths.clear();
    r.allzero_paths.clear();
    return r;
}

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
    // Every stored row's key includes these bytes, so a change to how they
    // are written would orphan every result a user has. The literal is from
    // one run.
    const std::string db = testtemp::temp_path("rules_fp_bytes", ".db");
    HydraRecord rec = at_cap(4);
    rec.rules_fingerprint = 0x0102030405060708ULL;
    {
        RecordStore store(db);
        store.add_row(prepare_row(RecordKey{"h", "mode", CapQuery::at(4)}, rec));
    }
    CHECK(scalar_on_file(db, "SELECT COUNT(*) FROM results WHERE hex(rules_fp) = '0807060504030201'") ==
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

TEST_CASE("a database from Hydra 1.6 or older opens with nothing to show") {
    // User decision 2026-09-26: the pre-1.7 migrations are gone. The old
    // `records` table is left where it is and never read, so its charts read
    // Not analyzed until they are analyzed again. Those rows could not be
    // read since 1.8.1 anyway.
    const std::string path = testtemp::temp_path("old_records", ".db");
    std::remove(path.c_str());
    { RecordStore seed(path); }
    exec_on_file(path,
                 "DROP TABLE results;"
                 "PRAGMA user_version = 1;"
                 "CREATE TABLE records (hyhash TEXT NOT NULL, chartmode TEXT NOT NULL,"
                 " hyversion TEXT NOT NULL, sp_cap INTEGER NOT NULL, bestpath TEXT NOT NULL,"
                 " blob BLOB NOT NULL, score INTEGER, actcount INTEGER, maxskip INTEGER,"
                 " hardest_ms REAL, avgmult REAL, notecount INTEGER, sqin_count INTEGER,"
                 " sqout_count INTEGER, pathcount INTEGER,"
                 " PRIMARY KEY (hyhash, chartmode, sp_cap));"
                 "INSERT INTO records (hyhash, chartmode, hyversion, sp_cap, bestpath, blob,"
                 " score) VALUES ('old', 'mode', '1.6.0', 8, '1 2 3', x'00', 100);");

    const RecordKey key{"old", "mode", CapQuery::at(8)};
    {
        RecordStore store(path);
        CHECK(store.counts().second == 0);
        CHECK(store.get_summary(key).status == RecordStatus::NotAnalyzed);
        CHECK_FALSE(store.has_record(key));
        // A fresh analysis lands as usual.
        store.add_record(key, at_cap(8));
        CHECK(store.get_summary(key).status == RecordStatus::Ready);
    }
    // The old table is left alone: nothing read it and nothing rewrote it.
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM records") == 1);
    std::remove(path.c_str());
}

TEST_CASE("store: a new database carries 0 in user_version and reads its own rows") {
    // D53 item 1: Hydra never writes the user_version slot. The column checks
    // in the constructor are the one upgrade gate, so a new file keeps the 0
    // SQLite gives a file nobody wrote it on.
    const std::string path = testtemp::temp_path("user_version", ".db");
    std::remove(path.c_str());
    const RecordKey key{"h", "mode", CapQuery::at(4)};
    {
        RecordStore store(path);
        store.add_record(key, at_cap(4));
    }
    CHECK(scalar_on_file(path,"PRAGMA user_version") == 0);
    {
        RecordStore reopened(path);
        CHECK(reopened.get_summary(key).status == RecordStatus::Ready);
    }
    std::remove(path.c_str());
}

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

// ---- the summary-only upgrade (D87 items 1 and 7, task storage-T5) ---------

namespace {

// The old layout's SQL lives in old_layout_fixture.h; these run it here.
void to_detail_layout(const std::string& path) {
    exec_on_file(path, test::detail_layout_sql().c_str());
}

int64_t detail_tables_in(const std::string& path) {
    return scalar_on_file(path, test::kDetailTablesCountSql);
}

int64_t structure_columns_in(const std::string& path) {
    return scalar_on_file(path, test::kStructureColumnCountSql);
}

}  // namespace

namespace {

// The keys the old-layout seed writes, all at an 8-bar cap.
const CapQuery kAt8 = CapQuery::at(8);
const RecordKey kReady{"ready", "mode", kAt8};
const RecordKey kOtherMode{"ready", "other mode", kAt8};
const RecordKey kNoPaths{"empty", "mode", kAt8};
const RecordKey kBuild{"build", "mode", kAt8};
const RecordKey kRules{"rules", "mode", kAt8};
const RecordKey kBoth{"both", "mode", kAt8};
const RecordKey kNoStars{"nostars", "mode", kAt8};

// What the seed holds, counted once by hand: one library row per chart (six
// charts), seven results, and three meta rows (the library's reader stamp,
// the fill-rule stamp and the Auto-results mark).
constexpr int64_t kSeedCharts = 6;
constexpr int64_t kSeedResults = 7;
constexpr int64_t kSeedMeta = 3;
// The results the upgrade keeps: all but the no-stars row.
constexpr int64_t kKeptResults = 6;
// The fill-rule stamp the seed writes, so the test can see meta come across.
constexpr const char* kSeedEngineMode = "seeded-engine-mode";

// A database at `path` in the layout before summary-only storage, with one
// row of every kind the upgrade treats differently, its files closed.
void seed_old_layout(const std::string& path) {
    for (const std::string& f : with_side_files(path)) std::remove(f.c_str());
    {
        RecordStore seed(path);
        seed.rebuild_chart_library({library_row("ready", "Ready"), library_row("empty", "Empty"),
                                    library_row("build", "Build"), library_row("rules", "Rules"),
                                    library_row("both", "Both"),
                                    library_row("nostars", "No stars")});
        seed.set_engine_mode(kSeedEngineMode);
        seed.add_record(kReady, at_cap(8));
        seed.add_record(kOtherMode, at_cap(8));
        seed.add_record(kNoPaths, no_paths_at_cap(8));
        test::add_stale_rows(seed, at_cap(8), kBuild, kRules, kBoth);
        seed.add_record(kNoStars, at_cap(8));
    }
    to_detail_layout(path);
    // A row from before the stars column: a score and no stars. Its stars
    // needed the stored paths to fill, so the upgrade deletes it. The
    // no-paths row has neither, by design, and stays (R17).
    exec_on_file(path, "UPDATE results SET stars = NULL WHERE hyhash = 'nostars'");
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM charts") == kSeedCharts);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM results") == kSeedResults);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM meta") == kSeedMeta);
    REQUIRE(scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE hyhash = 'empty'"
                                 " AND stars IS NULL AND score IS NULL") == 1);
    REQUIRE(detail_tables_in(path) == 5);
    REQUIRE(structure_columns_in(path) == 1);
}

// What every row reads after the upgrade: Ready stays Ready with its
// numbers, Stale stays Stale for the same reason, the no-stars row is gone,
// and the library and meta came across whole.
void check_upgraded_rows(RecordStore& store) {
    const SummaryLookup got_ready = store.get_summary(kReady);
    CHECK(got_ready.status == RecordStatus::Ready);
    CHECK(got_ready.summary == summarize_record(at_cap(8)));
    CHECK(got_ready.bestpath == best_path_text(at_cap(8)));
    CHECK(store.get_summary(kOtherMode).status == RecordStatus::Ready);
    const SummaryLookup got_empty = store.get_summary(kNoPaths);
    CHECK(got_empty.status == RecordStatus::Ready);
    CHECK_FALSE(got_empty.summary.has_scored_best_path());
    const SummaryLookup got_build = store.get_summary(kBuild);
    CHECK(got_build.status == RecordStatus::Stale);
    CHECK(got_build.stale_build);
    CHECK_FALSE(got_build.stale_rules);
    const SummaryLookup got_rules = store.get_summary(kRules);
    CHECK(got_rules.status == RecordStatus::Stale);
    CHECK_FALSE(got_rules.stale_build);
    CHECK(got_rules.stale_rules);
    const SummaryLookup got_both = store.get_summary(kBoth);
    CHECK(got_both.status == RecordStatus::Stale);
    CHECK(got_both.stale_build);
    CHECK(got_both.stale_rules);
    CHECK(store.get_summary(kNoStars).status == RecordStatus::NotAnalyzed);
    CHECK(store.counts().second == kKeptResults);
    CHECK(store.chart_library_count() == kSeedCharts);
    // The scan cache reads only under a current reader stamp, so a full one
    // shows the stamp came across with meta.
    CHECK(store.chart_library_cache().size() == static_cast<size_t>(kSeedCharts));
    CHECK(store.engine_mode() == std::optional<std::string>(kSeedEngineMode));
}

// The file is in this build's layout and nothing of the upgrade is left
// beside it.
void check_upgraded_files(const std::string& path) {
    CHECK(detail_tables_in(path) == 0);
    CHECK(structure_columns_in(path) == 0);
    CHECK_FALSE(file_exists_utf8(upgrading_path(path)));
    CHECK_FALSE(file_exists_utf8(old_path(path)));
}

// The file is still the old layout with every seeded row: an upgrade that
// failed or stopped changed nothing that matters, and left no fresh file.
void check_old_file_whole(const std::string& path) {
    CHECK(detail_tables_in(path) == 5);
    CHECK(structure_columns_in(path) == 1);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM charts") == kSeedCharts);
    CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM results") == kSeedResults);
    CHECK_FALSE(file_exists_utf8(upgrading_path(path)));
    CHECK_FALSE(file_exists_utf8(old_path(path)));
}

void remove_db(const std::string& path) {
    for (const std::string& f : with_side_files(path)) std::remove(f.c_str());
    for (const std::string& f : with_side_files(upgrading_path(path))) std::remove(f.c_str());
    for (const std::string& f : with_side_files(old_path(path))) std::remove(f.c_str());
}

// Every report an open made, in order.
struct ProgressLog {
    std::vector<OpenProgress> reports;
    OpenProgressFn fn() {
        return [this](const OpenProgress& p) {
            reports.push_back(p);
            return true;
        };
    }
    // The steps in the order they began, each once.
    std::vector<OpenStep> steps() const {
        std::vector<OpenStep> out;
        for (const OpenProgress& p : reports)
            if (out.empty() || out.back() != p.step) out.push_back(p.step);
        return out;
    }
};

}  // namespace

TEST_CASE("the first open of a file with stored path details drops them and keeps every status") {
    const std::string path = testtemp::temp_path("summary_only_upgrade", ".db");
    seed_old_layout(path);
    const int64_t ids_before = scalar_on_file(path, "SELECT SUM(result_id) FROM results"
                                                    " WHERE hyhash <> 'nostars'");
    const int64_t chart_rowids_before = scalar_on_file(path, "SELECT SUM(rowid) FROM charts");

    ProgressLog log;
    {
        RecordStore store(path, core::default_stamp(), log.fn());
        check_upgraded_rows(store);
    }
    check_upgraded_files(path);
    // A freshly written file has no free pages from the dropped tables.
    CHECK(scalar_on_file(path, "PRAGMA freelist_count") == 0);
    // Every kept row keeps its id, and every library row its rowid (the
    // naming copy is the smallest one, kNamingCopiesSql).
    CHECK(scalar_on_file(path, "SELECT SUM(result_id) FROM results") == ids_before);
    CHECK(scalar_on_file(path, "SELECT SUM(rowid) FROM charts") == chart_rowids_before);

    // The bar's numbers: the steps in order, a total of every row the copy
    // copies, counted one by one up to that total.
    CHECK(log.steps() ==
          std::vector<OpenStep>{OpenStep::Opening, OpenStep::Copying, OpenStep::Finishing});
    const int64_t total = kSeedCharts + kKeptResults + kSeedMeta;
    int64_t last_done = -1;
    for (const OpenProgress& p : log.reports) {
        if (p.step == OpenStep::Opening) continue;
        CHECK(p.rows_total == total);
        CHECK(p.rows_done >= last_done);
        last_done = p.rows_done;
    }
    CHECK(last_done == total);
    CHECK(log.reports.back().step == OpenStep::Finishing);
    CHECK(log.reports.back().rows_done == total);

    // A second open finds no detail tables and does nothing: it doesn't even
    // apply the delete rule, which a row like this one would fail.
    exec_on_file(path, "UPDATE results SET stars = NULL WHERE hyhash = 'ready'"
                       " AND chartmode = 'other mode'");
    ProgressLog again;
    {
        RecordStore store(path, core::default_stamp(), again.fn());
        CHECK(store.counts().second == kKeptResults);
    }
    CHECK(again.steps() == std::vector<OpenStep>{OpenStep::Opening});
    CHECK(scalar_on_file(path, "SELECT SUM(result_id) FROM results") == ids_before);
    CHECK(detail_tables_in(path) == 0);
    remove_db(path);
}

TEST_CASE("a fresh file left by an earlier run is removed and the upgrade runs") {
    const std::string path = testtemp::temp_path("upgrade_stale_temp", ".db");
    seed_old_layout(path);
    write_junk_db(upgrading_path(path));
    write_junk_db(upgrading_path(path) + "-wal");
    {
        RecordStore store(path);
        check_upgraded_rows(store);
    }
    check_upgraded_files(path);
    CHECK_FALSE(file_exists_utf8(upgrading_path(path) + "-wal"));
    remove_db(path);
}

TEST_CASE("a run stopped between the two renames opens as the upgraded file") {
    const std::string path = testtemp::temp_path("upgrade_between_renames", ".db");
    seed_old_layout(path);
    stop_copy_upgrade_after_step_aside_for_test(true);
    CHECK_THROWS(RecordStore(path));
    stop_copy_upgrade_after_step_aside_for_test(false);
    // The state a kill there leaves: the original stepped aside, the fresh
    // file complete, nothing under the database's name.
    REQUIRE_FALSE(file_exists_utf8(path));
    REQUIRE(file_exists_utf8(old_path(path)));
    REQUIRE(file_exists_utf8(upgrading_path(path)));
    {
        RecordStore store(path);
        check_upgraded_rows(store);
    }
    check_upgraded_files(path);
    remove_db(path);
}

TEST_CASE("a stepped-aside original with no fresh file comes back and upgrades") {
    const std::string path = testtemp::temp_path("upgrade_old_alone", ".db");
    seed_old_layout(path);
    REQUIRE(MoveFileW(win32_path(path).c_str(), win32_path(old_path(path)).c_str()));
    {
        RecordStore store(path);
        check_upgraded_rows(store);
    }
    check_upgraded_files(path);
    remove_db(path);
}

TEST_CASE("a stop asked for mid-copy leaves the old file whole and throws Cancelled") {
    const std::string path = testtemp::temp_path("upgrade_cancelled", ".db");
    seed_old_layout(path);
    const OpenProgressFn stop_at_two = [](const OpenProgress& p) {
        return !(p.step == OpenStep::Copying && p.rows_done == 2);
    };
    try {
        RecordStore store(path, core::default_stamp(), stop_at_two);
        FAIL("the open finished although the callback asked it to stop");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::Cancelled);
    }
    check_old_file_whole(path);
    remove_db(path);
}

TEST_CASE("a fresh file that cannot be made fails as an upgrade error with the old file whole") {
    const std::string path = testtemp::temp_path("upgrade_no_temp", ".db");
    seed_old_layout(path);
    // A folder where the fresh file goes: nothing can create it.
    std::filesystem::create_directories(os_path(upgrading_path(path)));
    try {
        RecordStore store(path);
        FAIL("the open finished with no fresh file to copy into");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseUpgrade);
        CHECK(app::plain_error(e) ==
              "Hydra couldn't update its library file (hydra.db) for this version. Your charts "
              "and results were not changed. Check that no other copy of Hydra or hydra_batch "
              "is running and that the disk isn't full, then start Hydra again.");
    }
    std::filesystem::remove(os_path(upgrading_path(path)));
    check_old_file_whole(path);
    remove_db(path);
}

TEST_CASE("another connection on the old file stops the swap as an upgrade error") {
    const std::string path = testtemp::temp_path("upgrade_held_open", ".db");
    seed_old_layout(path);
    sqlite3* holder = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &holder) == SQLITE_OK);
    REQUIRE(sqlite3_exec(holder, "SELECT COUNT(*) FROM charts", nullptr, nullptr, nullptr) ==
            SQLITE_OK);
    try {
        RecordStore store(path);
        FAIL("the upgrade swapped a file another connection holds");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseUpgrade);
    }
    sqlite3_close(holder);
    check_old_file_whole(path);
    remove_db(path);
}

TEST_CASE("every open caps the WAL file the log keeps after a checkpoint") {
    // journal_size_limit is per connection, so the store sets it on every
    // open. 4194304 bytes is the value D93 chose.
    const std::string path = testtemp::temp_path("journal_size_limit", ".db");
    std::remove(path.c_str());
    for (int open = 0; open < 2; ++open) {
        RecordStore store(path);
        CHECK(store.journal_size_limit_for_test() == 4194304);
    }
    std::remove(path.c_str());
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
    CHECK(store.analyzed_hashes("mode", CapQuery::at(4), kLegacy).count("h") == 1);

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

// ---- real old databases (testdata/store, test fidelity plan task 1) --------

namespace {

// One file in testdata/store and what today's open makes of it. The fields
// before the open are the README's "Facts the tests pin"; the fields after it
// were pinned from one run on 2026-10-10 at a5935954 (v2.0.0, v2.1.0) and at
// d6c48705 (the two v1.8.4 files).
struct OldDatabase {
    const char* file;
    int64_t user_version;
    bool has_legacy_fills_column;
    bool has_rules_fp_column;
    // The results rows the first open keeps: all 3, or none.
    int64_t rows_after_open;
    // The fill rule every kept row is filed under (0 where none is kept).
    int legacy_fills;
    // The steps the first open reported, each once.
    std::vector<OpenStep> steps;
};

// Every fixture row's chart mode (README "Schema and stamps").
const char* const kOldDatabaseChartMode = "Expert Pro Drums, 2x Bass";

// The three charts every fixture analysed (README "The three charts").
const char* const kOldDatabaseHashes[] = {"0b647b1570475f18d463466f2c4d72a7",
                                          "0d4fd734fe5f2bf6ff1fdbc4ff55be8f",
                                          "bef0759746740f9c1045f64e9a5a2938"};

// The rules fingerprint every fixture row holds, its bytes in file order as
// SQLite's lower(hex()) spells them (README "Rules fingerprint").
const char* const kOldDatabaseRulesFpHex = "f24c606966e9d270";

// The settings every fixture row was analysed under (README "Results rows by
// chart"): a 4-bar cap, the ms limit on at 10, depth "scores 4".
RecordKey old_database_key(const char* hyhash, int legacy_fills) {
    Lens lens;
    lens.ms_enabled = 1;
    lens.ms_value = 10;
    lens.depth_mode = 0;
    lens.depth_value = 4;
    lens.legacy_fills = legacy_fills;
    return RecordKey{hyhash, kOldDatabaseChartMode, CapQuery::at(4), lens};
}

using ChartPairs = std::set<std::pair<std::string, std::string>>;

// The (hyhash, chartmode) pairs the fixtures' results tables hold. Rows are
// matched by chart, since result_ids differ between the files.
ChartPairs old_database_pairs() {
    ChartPairs out;
    for (const char* h : kOldDatabaseHashes) out.insert({h, kOldDatabaseChartMode});
    return out;
}

// The (hyhash, chartmode) pairs the results table of the file at `path` holds.
ChartPairs chart_pairs_on_file(const std::string& path) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, "SELECT hyhash, chartmode FROM results", -1, &s, nullptr) ==
            SQLITE_OK);
    ChartPairs out;
    while (sqlite3_step(s) == SQLITE_ROW)
        out.insert({reinterpret_cast<const char*>(sqlite3_column_text(s, 0)),
                    reinterpret_cast<const char*>(sqlite3_column_text(s, 1))});
    sqlite3_finalize(s);
    sqlite3_close(db);
    return out;
}

int64_t results_columns_named(const std::string& path, const std::string& column) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM pragma_table_info('results') WHERE name = '" +
                                    column + "'");
}

int64_t results_rows_in(const std::string& path) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM results");
}

}  // namespace

TEST_CASE("a database a real old release wrote opens with no path details, its results Stale or,"
          " before the stars column, left for a re-run") {
    using S = OpenStep;
    const std::vector<OldDatabase> fixtures = {
        // v1.8.4 had no stars column, so the copy keeps none of its rows
        // (docs/adr/0026-the-store-keeps-summaries-the-engine-gives-details.md).
        {"v1.8.4-schema2.db", 2, false, false, 0, 0,
         {S::Opening, S::UpdatingResultsKey, S::Copying, S::Finishing}},
        {"v1.8.4-schema2-legacy-fills.db", 2, false, false, 0, 0,
         {S::Opening, S::UpdatingResultsKey, S::Copying, S::Finishing}},
        {"v2.0.0-schema3.db", 3, true, false, 3, 0,
         {S::Opening, S::UpdatingResultsKey, S::Copying, S::Finishing}},
        // v2.1.0 stopped setting user_version (README "Schema and stamps"),
        // so its rules_fp column is what tells this layout apart.
        {"v2.1.0-schema4.db", 0, true, true, 3, 0, {S::Opening, S::Copying, S::Finishing}},
    };
    for (const OldDatabase& f : fixtures) {
        INFO(std::string(f.file));
        const std::string source = std::string(HYDRA_TESTDATA_DIR) + "/store/" + f.file;
        REQUIRE(std::filesystem::exists(source));
        // The store opens in WAL mode, which would write beside the checked-in
        // file, so it opens a copy.
        const std::string path = testtemp::temp_path("old_database", ".db");
        remove_db(path);
        std::filesystem::copy_file(source, path, std::filesystem::copy_options::overwrite_existing);

        // The file is the layout the README says, before anything opens it.
        CHECK(scalar_on_file(path, "PRAGMA user_version") == f.user_version);
        CHECK(results_columns_named(path, "legacy_fills") == (f.has_legacy_fills_column ? 1 : 0));
        CHECK(results_columns_named(path, "rules_fp") == (f.has_rules_fp_column ? 1 : 0));
        REQUIRE(chart_pairs_on_file(path) == old_database_pairs());

        const bool kept = f.rows_after_open > 0;
        const ChartPairs pairs_after = kept ? old_database_pairs() : ChartPairs{};
        ProgressLog log;
        {
            RecordStore store(path, core::default_stamp(), log.fn());
            CHECK(store.counts().second == f.rows_after_open);
            for (const char* h : kOldDatabaseHashes) {
                INFO(std::string(h));
                if (kept) {
                    // Each was written by an older build, so the store's own
                    // status call reads it Stale, never Ready.
                    const SummaryLookup got =
                        store.get_summary(old_database_key(h, f.legacy_fills));
                    CHECK(got.status == RecordStatus::Stale);
                    CHECK(got.stale_build);
                } else {
                    // No row is left under either fill rule.
                    for (int legacy_fills : {0, 1})
                        CHECK(store.get_summary(old_database_key(h, legacy_fills)).status ==
                              RecordStatus::NotAnalyzed);
                }
            }
        }
        CHECK(log.steps() == f.steps);
        CHECK(results_rows_in(path) == f.rows_after_open);
        CHECK(chart_pairs_on_file(path) == pairs_after);
        CHECK(scalar_on_file(path, std::string("SELECT COUNT(*) FROM results WHERE lower(hex(rules_fp))"
                                               " <> '") +
                                       kOldDatabaseRulesFpHex + "'") == 0);
        CHECK(scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE legacy_fills <> " +
                                       std::to_string(f.legacy_fills)) == 0);
        check_upgraded_files(path);

        // A second open finds nothing to upgrade and keeps what the first kept.
        ProgressLog again;
        {
            RecordStore store(path, core::default_stamp(), again.fn());
            CHECK(store.counts().second == f.rows_after_open);
        }
        CHECK(again.steps() == std::vector<OpenStep>{S::Opening});
        CHECK(results_rows_in(path) == f.rows_after_open);
        CHECK(chart_pairs_on_file(path) == pairs_after);
        remove_db(path);
    }
}

TEST_CASE("a failed library rebuild keeps the previous scan") {
    // The rebuild used to drop the table before opening its transaction, so
    // an insert that failed left the library empty, and took the rescan cache
    // with it. A trigger that refuses one md5 makes an insert fail partway.
    const std::string path = testtemp::temp_path("rebuild_fail", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B")});
    }
    exec_on_file(path,
                 "CREATE TRIGGER refuse_boom BEFORE INSERT ON charts WHEN NEW.md5 = 'boom'"
                 " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    {
        RecordStore store(path);
        CHECK_THROWS(
            store.rebuild_chart_library({library_row("c", "C"), library_row("boom", "Boom")}));
        CHECK(store.chart_library_count() == 2);
        const ChartLibraryCache cache = store.chart_library_cache();
        CHECK(cache.count("C:\\charts\\a\\notes.chart") == 1);
        CHECK(cache.count("C:\\charts\\b\\notes.chart") == 1);
        // No transaction was left open: the next rebuild goes through.
        store.rebuild_chart_library({library_row("c", "C")});
        CHECK(store.chart_library_count() == 1);
    }
    std::remove(path.c_str());
}

TEST_CASE("a charts table from before the sig column still rebuilds") {
    // The rebuild empties the table instead of recreating it, so an old
    // table has to gain the column when the store opens.
    const std::string path = testtemp::temp_path("charts_nosig", ".db");
    std::remove(path.c_str());
    exec_on_file(path,
                 "CREATE TABLE charts (md5 TEXT, name TEXT, artist TEXT, charter TEXT,"
                 " path TEXT, folder TEXT);"
                 "INSERT INTO charts VALUES ('a', 'A', 'Artist', 'Charter',"
                 " 'C:\\charts\\a\\notes.chart', 'C:\\charts');");
    {
        RecordStore store(path);
        CHECK(store.chart_library_count() == 1);
        CHECK(store.chart_library_cache().empty());  // no fingerprints yet
        store.rebuild_chart_library({library_row("b", "B")});
        CHECK(store.chart_library_count() == 1);
        CHECK(store.chart_library_cache().at("C:\\charts\\b\\notes.chart").sig == "sig-b");
    }
    std::remove(path.c_str());
}

TEST_CASE("a result's names follow the latest scan") {
    // User decision 2026-09-26: fixing song.ini reaches the reports. A
    // result's names come from the library (kNamingCopiesSql), so a rescan
    // after song.ini changed renames it.
    RecordStore store(":memory:");
    const ChartLibraryEntry first = library_row("h", "Scanned Title");
    ChartLibraryEntry second = library_row("h", "Second Copy");
    second.notespath = "C:\\charts\\copy\\notes.chart";
    store.rebuild_chart_library({first, second});
    store.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4));
    auto listed = [&] {
        std::vector<RecordListing> rows =
            store.list_records(std::nullopt, CapQuery::at(4), Lens{}, SortColumn::Score, true);
        REQUIRE(rows.size() == 1);
        return rows[0];
    };
    // The scan found two copies of the chart; the first one it listed names
    // it.
    CHECK(listed().ref_name == "Scanned Title");

    ChartLibraryEntry renamed = library_row("h", "New Title");
    renamed.artist = "New Artist";
    renamed.charter = "New Charter";
    store.rebuild_chart_library({renamed, second});
    CHECK(listed().ref_name == "New Title");
    CHECK(listed().ref_artist == "New Artist");
    CHECK(listed().ref_charter == "New Charter");

    // A chart the scan found but nobody analyzed has no result to count.
    store.rebuild_chart_library({renamed, library_row("x", "Never Analyzed")});
    CHECK(store.counts().first == 1);
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

// SQLite switches a feature on when its SQLITE_ENABLE_* macro is defined at
// all, whatever its value (sqlite.org/compile.html), so the old
// "SQLITE_ENABLE_FTS5=0" compiled full-text search in. Hydra never uses it.
TEST_CASE("the vendored SQLite is built without FTS5") {
    CHECK(sqlite3_compileoption_used("ENABLE_FTS5") == 0);
}

// ---- store speed (2026-09-26 audit, Task 9) --------------------------------

TEST_CASE("a file store runs in WAL mode with an index on chart names") {
    // WAL: a commit appends to a log instead of rewriting the file in place,
    // so it costs far fewer disk syncs (https://www.sqlite.org/wal.html).
    // The journal mode is stored in the file, so a fresh connection sees it.
    const std::string path = testtemp::temp_path("wal", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, "PRAGMA journal_mode", -1, &s, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(s) == SQLITE_ROW);
    const std::string mode = reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
    sqlite3_finalize(s);
    sqlite3_close(db);
    CHECK(mode == "wal");
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM sqlite_master"
                       " WHERE type='index' AND name='charts_by_name'") == 1);
    std::remove(path.c_str());
}

TEST_CASE("the library table's md5 index exists") {
    // D76: the scan's purge and list_records find a chart's library rows by
    // md5; without the index each lookup read the whole library table.
    const std::string path = testtemp::temp_path("md5_index", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM sqlite_master"
                       " WHERE type='index' AND name='charts_by_md5'") == 1);
    std::remove(path.c_str());
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

TEST_CASE("a save_analysis that fails leaves nothing behind and reads as a save error") {
    // A trigger that refuses one chart's result makes the save fail.
    const std::string path = testtemp::temp_path("save_fail", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    exec_on_file(path,
                 "CREATE TRIGGER refuse_boom BEFORE INSERT ON results WHEN NEW.hyhash = 'boom'"
                 " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    {
        RecordStore store(path);
        const RecordKey boom{"boom", "mode", CapQuery::at(4)};
        try {
            store.save_analysis(prepare_row(boom, at_cap(4)));
            FAIL("the refused save went through");
        } catch (const std::exception& e) {
            CHECK(hydra::app::plain_error(e) ==
                  "Hydra couldn't save to its database (hydra.db). Check that the disk isn't "
                  "full and that no other copy of Hydra is running, then try again.");
        }
        CHECK(store.counts().second == 0);
        // The store is still usable: no transaction was left open.
        const RecordKey ok{"ok", "mode", CapQuery::at(4)};
        store.save_analysis(prepare_row(ok, at_cap(4)));
        CHECK(store.get_summary(ok).status == RecordStatus::Ready);
    }
    std::remove(path.c_str());
}

// D72 item 2: SQLite notices a file that isn't a database only at its first
// statement, after sqlite3_open_v2 said yes. It still reads "couldn't open".
TEST_CASE("a database file that isn't a database fails to open as DatabaseOpen") {
    const std::string path = testtemp::temp_path("junk_db", ".db");
    write_junk_db(path);
    try {
        RecordStore store(path);
        FAIL("a file of junk bytes opened as a database");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseOpen);
        CHECK(std::string(e.what()) == "sqlite exec failed: file is not a database");
    }
    CHECK(std::remove(path.c_str()) == 0);
}

// D72 items 2 and 3: a file another connection has locked fails at once, as
// "couldn't open", and the failed open lets go of the file.
TEST_CASE("a database another connection has locked fails to open as DatabaseOpen, and lets "
          "go of the file") {
    const std::string path = testtemp::temp_path("locked_db", ".db");
    std::remove(path.c_str());
    // A rollback-journal file (not WAL), so an exclusive lock keeps readers out.
    sqlite3* holder = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &holder) == SQLITE_OK);
    REQUIRE(sqlite3_exec(holder, "CREATE TABLE t (x); BEGIN EXCLUSIVE; INSERT INTO t VALUES (1);",
                         nullptr, nullptr, nullptr) == SQLITE_OK);
    try {
        RecordStore store(path);
        FAIL("a locked file opened");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseOpen);
        CHECK(std::string(e.what()) == "sqlite exec failed: database is locked");
    }
    REQUIRE(sqlite3_exec(holder, "ROLLBACK", nullptr, nullptr, nullptr) == SQLITE_OK);
    sqlite3_close(holder);

    { RecordStore store(path); }
    // Nothing holds the file now: SQLite opens it without delete sharing, so a
    // handle the failed open left behind would make this fail.
    std::remove((path + "-wal").c_str());
    std::remove((path + "-shm").c_str());
    CHECK(std::remove(path.c_str()) == 0);
}

namespace {

// Every table the store makes, dropped by a second connection while a store
// has the file open. The store's connection still holds the old schema, so
// its next read compiles and then fails when it steps.
constexpr const char* kDropEveryTable = "DROP TABLE results; DROP TABLE charts; DROP TABLE meta;";

// The read must throw the database read error, never answer.
void check_read_fails(const std::function<void()>& read) {
    try {
        read();
        FAIL_CHECK("a read on a failing database answered");
    } catch (const hydra::KindedError& e) {
        INFO(e.what());
        CHECK(e.kind() == hydra::ErrorKind::DatabaseRead);
        CHECK(hydra::app::plain_error(e) == hydra::app::kDatabaseReadSentence);
    }
}

}  // namespace

// D72's DB1 found this one: analyzed_hashes on a failing database answered
// with no charts, so a batch treated every chart as not analyzed.
TEST_CASE("a read on a failing database throws instead of answering empty") {
    const std::string path = testtemp::temp_path("read_fail", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4));
        REQUIRE(store.analyzed_hashes("mode", CapQuery::at(4), Lens{}).count("h") == 1);
        exec_on_file(path, "DROP TABLE results;");
        // The first read fails when it steps; by then the connection has
        // reloaded the schema, so the second fails when it compiles.
        for (const char* when : {"step", "compile"}) {
            INFO(std::string(when));
            check_read_fails([&] { store.analyzed_hashes("mode", CapQuery::at(4), Lens{}); });
        }
    }
    std::remove(path.c_str());
}

TEST_CASE("every store read throws a database read error when its step fails") {
    const std::string path = testtemp::temp_path("every_read_fail", ".db");
    std::remove(path.c_str());
    const RecordKey key{"h", "mode", CapQuery::at(4)};
    const CapQuery cap = CapQuery::at(4);
    const std::pair<const char*, std::function<void(RecordStore&)>> reads[] = {
        {"engine_mode", [](RecordStore& s) { s.engine_mode(); }},
        {"stamped_fill_rule", [](RecordStore& s) { s.stamped_fill_rule(); }},
        {"get_summaries", [&](RecordStore& s) { s.get_summaries({"h"}, "mode", cap, Lens{}); }},
        {"get_summary", [&](RecordStore& s) { s.get_summary(key); }},
        {"has_record", [&](RecordStore& s) { s.has_record(key); }},
        {"analyzed_hashes", [&](RecordStore& s) { s.analyzed_hashes("mode", cap, Lens{}); }},
        {"list_records",
         [&](RecordStore& s) {
             s.list_records(std::nullopt, cap, Lens{}, SortColumn::Score, false, std::nullopt);
         }},
        {"counts", [](RecordStore& s) { s.counts(); }},
        {"library_copies", [](RecordStore& s) { s.library_copies(); }},
        {"naming_copy_paths", [](RecordStore& s) { s.naming_copy_paths(); }},
        {"chart_library_cache", [](RecordStore& s) { s.chart_library_cache(); }},
        {"chart_library_count", [](RecordStore& s) { s.chart_library_count(); }},
        {"list_chart_library", [](RecordStore& s) { s.list_chart_library(0, 10); }},
    };
    for (const auto& [name, read] : reads) {
        INFO(std::string(name));
        // A fresh open makes the tables again, so each read fails at its own
        // first step.
        RecordStore store(path);
        exec_on_file(path, kDropEveryTable);
        check_read_fails([&] { read(store); });
    }
    std::remove(path.c_str());
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
        const std::unordered_set<std::string> got = store.analyzed_hashes("mode", cap, Lens{});
        for (const char* h : charts) {
            INFO(h);
            CHECK((got.count(h) == 1) == store.has_record(RecordKey{h, "mode", cap}));
        }
    }
    CHECK(store.analyzed_hashes("mode", CapQuery::at(4), Lens{}) ==
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
    const std::vector<SummaryLookup> chips = store.get_summaries(library, "mode", cap, Lens{});
    const std::unordered_set<std::string> skip = store.analyzed_hashes("mode", cap, Lens{});
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
            store.get_summaries(page, "mode", CapQuery::at(cap), Lens{});
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
    CHECK(store.get_summaries({}, "mode", CapQuery::at(4), Lens{}).empty());
}

// ---- Auto removed (2026-09-27, interface redesign Task 6) ------------------

namespace {

// A result the way Hydra 1.8.4's Auto stamped it: the fixture's paths at
// `cap`, under the default rules' Auto fingerprint.
HydraRecord auto_run_at(int cap) {
    HydraRecord r = at_cap(cap);
    r.rules_fingerprint = core::default_rules().retired_auto_fingerprint();
    return r;
}

}  // namespace

TEST_CASE("the first open deletes the results Auto saved, once") {
    const std::string path = testtemp::temp_path("auto_delete", ".db");
    std::remove(path.c_str());
    const RecordKey kept{"h", "mode", CapQuery::at(4)};
    const RecordKey whatif{"h", "mode", CapQuery::at(8)};
    const RecordKey auto_same_chart{"h", "mode", CapQuery::at(16)};
    const RecordKey auto_only{"a", "mode", CapQuery::at(32)};

    {
        RecordStore store(path);
        store.add_record(kept, at_cap(4));
        store.add_record(whatif, at_cap(8));
        store.add_record(auto_same_chart, auto_run_at(16));
        store.add_record(auto_only, auto_run_at(32));
        // This build accepts only the fixed-cap fingerprint, so an Auto row
        // reads Stale even before anything deletes it.
        CHECK(store.get_summary(auto_only).status == RecordStatus::Stale);
        CHECK(store.counts().second == 4);
    }
    // A database Hydra 1.8.4 wrote has no mark yet.
    exec_on_file(path, "DELETE FROM meta WHERE key='auto_results_deleted'");

    {
        RecordStore store(path);
        CHECK(store.counts().second == 2);
        CHECK(store.get_summary(auto_only).status == RecordStatus::NotAnalyzed);
        CHECK(store.get_summary(auto_same_chart).status == RecordStatus::NotAnalyzed);
        // A fixed-cap what-if above 4 bars is not an Auto result and stays.
        CHECK(store.get_summary(whatif).status == RecordStatus::Ready);
        const SummaryLookup left = store.get_summary(kept);
        REQUIRE(left.status == RecordStatus::Ready);
        CHECK(left.summary == summarize_record(at_cap(4)));
        CHECK(left.bestpath == best_path_text(at_cap(4)));
    }
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM meta WHERE key='auto_results_deleted'") == 1);

    // Marked done: a later open never deletes again.
    {
        RecordStore store(path);
        store.add_record(auto_only, auto_run_at(32));
    }
    {
        RecordStore store(path);
        CHECK(store.counts().second == 3);
        CHECK(store.get_summary(auto_only).status == RecordStatus::Stale);
    }
    std::error_code ec;
    std::filesystem::remove(std::filesystem::u8path(path), ec);
}

TEST_CASE("a start with a bad rules file leaves the Auto results for the next good start") {
    const std::string path = testtemp::temp_path("auto_delete_none", ".db");
    std::remove(path.c_str());
    const RecordKey auto_only{"a", "mode", CapQuery::at(32)};
    {
        RecordStore store(path);
        store.add_record(auto_only, auto_run_at(32));
    }
    exec_on_file(path, "DELETE FROM meta WHERE key='auto_results_deleted'");
    {
        // A bad hydra_rules.ini: there is no fingerprint to look for, so
        // nothing is deleted and nothing is marked done.
        RecordStore store(path, core::RulesStamp::none());
        CHECK(store.counts().second == 1);
    }
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM meta WHERE key='auto_results_deleted'") == 0);
    {
        RecordStore store(path);
        CHECK(store.counts().second == 0);
    }
    std::error_code ec;
    std::filesystem::remove(std::filesystem::u8path(path), ec);
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
        store.get_summaries({"ready", "stale", "none"}, "mode", CapQuery::at(4), Lens{});
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
        store.get_summaries(hashes, "mode", CapQuery::at(4), Lens{});
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

TEST_CASE("a file from before AL loses its songlength table") {
    // D69 replaced D58 item 5: the per-difficulty length table goes.
    const std::string path = testtemp::temp_path("song_length_old", ".db");
    std::remove(path.c_str());
    { RecordStore store(path); }
    exec_on_file(path,
                 "CREATE TABLE IF NOT EXISTS songlength (hyhash TEXT NOT NULL,"
                 " chartmode TEXT NOT NULL, length_ms REAL NOT NULL,"
                 " PRIMARY KEY (hyhash, chartmode));"
                 "INSERT OR REPLACE INTO songlength VALUES ('h', 'mode', 1234.0);");
    { RecordStore store(path); }
    CHECK(scalar_on_file(path,"SELECT COUNT(*) FROM sqlite_master WHERE name = 'songlength'") == 0);
    std::remove(path.c_str());
}

TEST_CASE("the scan's first copy names a chart whatever copy was analyzed") {
    // D51 call 10 (finding 63): one rule names a chart the scan found twice,
    // the first copy it listed. Every chart takes its names from the library
    // (kNamingCopiesSql), a single copy included.
    RecordStore store(":memory:");
    const ChartLibraryEntry first = library_row("h", "Scanned Title");
    ChartLibraryEntry second = library_row("h", "Second Copy");
    second.notespath = "C:\\charts\\copy\\notes.chart";
    store.rebuild_chart_library({first, second, library_row("s", "Single Copy")});

    store.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4));
    store.add_record(RecordKey{"s", "mode", CapQuery::at(4)}, at_cap(4));
    std::map<std::string, std::string> names;
    for (const RecordListing& row :
         store.list_records(std::nullopt, CapQuery::at(4), Lens{}, SortColumn::Score, true))
        names[row.hyhash] = row.ref_name;
    CHECK(names ==
          std::map<std::string, std::string>{{"h", "Scanned Title"}, {"s", "Single Copy"}});
}

TEST_CASE("the scan cache is dropped when its reader stamp is not current") {
    // D51 call 12 (finding 345): the rescan cache carries kChartMetaStamp. A
    // file without a current stamp hands back no cache, so the next scan
    // reads every chart again and stamps the table it writes.
    const std::string path = testtemp::temp_path("chart_meta_stamp", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry entry = library_row("a", "A");
    {
        RecordStore store(path);
        store.rebuild_chart_library({entry});
    }
    {
        RecordStore store(path);
        CHECK(store.chart_library_cache().count(entry.notespath) == 1);
    }
    exec_on_file(path, "DELETE FROM meta WHERE key = 'chart_meta_version'");
    {
        RecordStore store(path);
        CHECK(store.chart_library_cache().empty());
        store.rebuild_chart_library({entry});
        CHECK(store.chart_library_cache().count(entry.notespath) == 1);
    }
    std::remove(path.c_str());
}

TEST_CASE("the store names the fill rule a file holds") {
    // Finding 54: the store reads its own engine_mode stamp. A file with
    // results and no stamp was written under the normal 1.1 rule; an empty
    // unstamped file holds no rule at all.
    RecordStore ch10(":memory:");
    ch10.set_engine_mode(engine_mode_stamp(FillDeadlineRule::Ch10));
    RecordStore ch11(":memory:");
    ch11.set_engine_mode(engine_mode_stamp(FillDeadlineRule::Ch11));
    RecordStore unstamped(":memory:");
    unstamped.add_record(RecordKey{"h", "mode", CapQuery::at(4)}, at_cap(4));
    RecordStore empty(":memory:");

    CHECK(ch10.stamped_fill_rule() == FillDeadlineRule::Ch10);
    CHECK(ch11.stamped_fill_rule() == FillDeadlineRule::Ch11);
    CHECK(unstamped.stamped_fill_rule() == FillDeadlineRule::Ch11);
    CHECK_FALSE(empty.stamped_fill_rule().has_value());
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

// D86 (build settings): CMakeLists.txt builds the vendored SQLite without its
// memory-use counters and without shared cache, two options that trim its own
// CPU. A sibling of the FTS5 case above.
TEST_CASE("the vendored SQLite is built without memory counters or shared cache") {
    CHECK(sqlite3_compileoption_used("DEFAULT_MEMSTATUS=0") == 1);
    CHECK(sqlite3_compileoption_used("OMIT_SHARED_CACHE") == 1);
}

// ---- the batch writer (D86 item 3, task W1) --------------------------------

TEST_CASE("the batch guard sets the checkpoint threshold and truncates the log at the end") {
    // The threshold is per connection, so only the store's own getter can
    // read it. 1,000 is SQLite's own default, read off one run.
    const std::string path = testtemp::temp_path("batch_guard", ".db");
    std::remove(path.c_str());
    const std::filesystem::path wal = std::filesystem::u8path(path + "-wal");
    {
        RecordStore store(path);
        CHECK(store.wal_autocheckpoint_for_test() == 1000);
        {
            const RecordStore::BatchWrites batch(store);
            CHECK(store.wal_autocheckpoint_for_test() == kBatchWalAutocheckpointPages);
            const RecordKey key{"h", "mode", CapQuery::at(4)};
            store.save_analysis(prepare_row(key, at_cap(4)));
            CHECK(std::filesystem::file_size(wal) > 0);
        }
        CHECK(std::filesystem::file_size(wal) == 0);
        CHECK(store.wal_autocheckpoint_for_test() == 1000);
        CHECK(store.counts().second == 1);
    }
    std::remove(path.c_str());
}

// ---- charts the library no longer lists (D87 items 3 and 4, task storage-T1) --

namespace {

// How many results one chart holds, read straight out of the file.
int64_t results_of(const std::string& path, const std::string& hash) {
    return scalar_on_file(path, "SELECT COUNT(*) FROM results WHERE hyhash = '" + hash + "'");
}

// One analyzed chart as the click saves it.
void save_chart(RecordStore& store, const std::string& hash, const std::string& mode = "mode") {
    store.save_analysis(prepare_row(RecordKey{hash, mode, CapQuery::at(4)}, at_cap(4)));
}

// library_row's row for another folder holding the same chart.
ChartLibraryEntry second_copy(const char* md5) {
    ChartLibraryEntry copy = library_row(md5, "Copy");
    copy.notespath = std::string("C:\\other\\") + md5 + "\\notes.chart";
    copy.rootfolder = "C:\\other";
    return copy;
}

}  // namespace

TEST_CASE("a scan that drops a chart deletes its stored rows") {
    const std::string path = testtemp::temp_path("scan_drops_chart", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B")});
        save_chart(store, "a");
        save_chart(store, "b");
    }
    REQUIRE(results_of(path, "a") == 1);
    REQUIRE(results_of(path, "b") == 1);
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A")});
    }
    CHECK(results_of(path, "b") == 0);
    CHECK(results_of(path, "a") == 1);
    std::remove(path.c_str());
}

TEST_CASE("a chart with two library copies keeps its rows when one copy leaves") {
    const std::string path = testtemp::temp_path("scan_drops_copy", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), second_copy("a")});
        save_chart(store, "a");
    }
    REQUIRE(results_of(path, "a") == 1);
    {
        RecordStore store(path);
        store.rebuild_chart_library({second_copy("a")});
        CHECK(store.library_copies() == std::unordered_map<std::string, int>{{"a", 1}});
    }
    CHECK(results_of(path, "a") == 1);
    std::remove(path.c_str());
}

TEST_CASE("a library chart keeps its rows for other chart modes, including Stale ones") {
    const std::string path = testtemp::temp_path("scan_keeps_modes", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B")});
        save_chart(store, "a", "mode");
        save_chart(store, "a", "other mode");
        test::add_stale_rows(store, at_cap(8), RecordKey{"a", "build", CapQuery::at(8)},
                             RecordKey{"a", "rules", CapQuery::at(8)},
                             RecordKey{"a", "both", CapQuery::at(8)});
        save_chart(store, "b");
        REQUIRE(store.get_summary(RecordKey{"a", "build", CapQuery::at(8)}).status ==
                RecordStatus::Stale);
    }
    REQUIRE(results_of(path, "a") == 5);
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A")});
    }
    CHECK(results_of(path, "a") == 5);
    CHECK(results_of(path, "b") == 0);
    std::remove(path.c_str());
}

TEST_CASE("reidentify_chart moves the library row to the new md5 and deletes the old md5's rows") {
    const std::string path = testtemp::temp_path("reidentify_moves", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry a = library_row("a", "A");
    {
        RecordStore store(path);
        store.rebuild_chart_library({a, library_row("b", "B")});
        save_chart(store, "a");
        save_chart(store, "b");
    }
    REQUIRE(results_of(path, "a") == 1);
    REQUIRE(results_of(path, "b") == 1);
    {
        RecordStore store(path);
        store.reidentify_chart(a.notespath, "a2", "sig-a2");
        CHECK(store.library_copies() == std::unordered_map<std::string, int>{{"a2", 1}, {"b", 1}});
        const ChartLibraryCache cache = store.chart_library_cache();
        REQUIRE(cache.count(a.notespath) == 1);
        CHECK(cache.at(a.notespath).md5 == "a2");
        CHECK(cache.at(a.notespath).sig == "sig-a2");
    }
    CHECK(results_of(path, "a") == 0);
    CHECK(results_of(path, "b") == 1);
    std::remove(path.c_str());
}

TEST_CASE("reidentify_chart keeps the old md5's rows while another library row has it") {
    const std::string path = testtemp::temp_path("reidentify_keeps", ".db");
    std::remove(path.c_str());
    const ChartLibraryEntry copy = second_copy("a");
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), copy});
        save_chart(store, "a");
    }
    REQUIRE(results_of(path, "a") == 1);
    {
        RecordStore store(path);
        store.reidentify_chart(copy.notespath, "a2", "sig-a2");
        CHECK(store.library_copies() == std::unordered_map<std::string, int>{{"a", 1}, {"a2", 1}});
    }
    CHECK(results_of(path, "a") == 1);
    std::remove(path.c_str());
}

TEST_CASE("a scan that fails partway through its deletes keeps the dropped chart's rows") {
    // The deletes run inside the scan's own transaction. A trigger that
    // refuses the results delete fails it after the library rows changed.
    const std::string path = testtemp::temp_path("scan_delete_fails", ".db");
    std::remove(path.c_str());
    {
        RecordStore store(path);
        store.rebuild_chart_library({library_row("a", "A"), library_row("b", "B")});
        save_chart(store, "a");
        save_chart(store, "b");
    }
    REQUIRE(results_of(path, "b") == 1);
    exec_on_file(path,
                 "CREATE TRIGGER refuse_result_delete BEFORE DELETE ON results"
                 " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    {
        RecordStore store(path);
        CHECK_THROWS(store.rebuild_chart_library({library_row("a", "A")}));
        CHECK(store.chart_library_count() == 2);
    }
    CHECK(results_of(path, "b") == 1);
    std::remove(path.c_str());
}
