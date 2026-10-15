// Shared test fixtures that carry display-side facts: how long a song's audio
// runs, a result stored behind the app's back, and a title. Phase 3's display
// tests all need the same few awkward inputs; they live here once so no test
// file builds its own. Records and paths built by hand live in
// record_fixtures.h, which includes this file at its end.
#ifndef HYDRA_TESTS_DISPLAY_FIXTURES_H
#define HYDRA_TESTS_DISPLAY_FIXTURES_H

#include <algorithm>
#include <string>
#include <vector>

#include "doctest.h"

#include "app/config.h"
#include "app/library_query.h"
#include "app/preview_view.h"  // last_note_ms, for the audio-tail sanity case
#include "core/model.h"
#include "core/rules.h"
#include "library_fixtures.h"  // library_row
#include "parse/song.h"
#include "record_fixtures.h"
#include "store/record_store.h"

namespace hydra::test {

// ---- a song whose audio runs past its last note -----------------------------

// How far the audio runs past the last note: 5 s.
inline constexpr double kAudioTailMs = 5000.0;

// A chart whose audio keeps playing after its last note. `last_note_ms` is
// the last note's onset; `audio_end_ms` is where the audio stops, kAudioTailMs
// later. Tests that need a song's length take that number as the length the
// chart states (D75). Numbers only, no playhead.
struct AudioTailChart {
    Song song;
    double last_note_ms = 0.0;
    double audio_end_ms = 0.0;
};

// beat_song with a note every beat to tick 1920 (one measure, 1000 ms).
inline AudioTailChart audio_tail_chart() {
    AudioTailChart c{beat_song({}, {}, 1920)};
    c.last_note_ms = 1000.0;
    c.audio_end_ms = c.last_note_ms + kAudioTailMs;
    return c;
}

// ---- a batch record saved for one chart --------------------------------------

// An analysis that kept no paths, made under `profile`: its SP cap, ms limit
// and fill rule are read from the profile, so the two never disagree
// (prepare_record refuses a mismatch).
inline HydraRecord empty_analysis_under(const store::Profile& profile) {
    HydraRecord record;
    record.sp_cap = profile.cap.exact;
    if (profile.lens.ms_enabled) record.ms_limit = profile.lens.ms_value;
    record.legacy_fills = profile.lens.legacy_fills != 0;
    return record;
}

// The profile a result is filed under at SP cap `cap`, with every other
// setting at its default. Settings::profile builds it, so a test never spells
// the profile's parts out itself.
inline store::Profile batch_profile(int cap) {
    app::Settings settings;
    settings.sp_cap = cap;
    return settings.profile();
}

// An empty current-version record for chart `md5` under `profile`, saved the
// way a batch saves one, behind the app's back. It reads back Ready, with no
// paths and so no score. This form is for a test that files under its own
// chartmode, lens or fill rule.
inline void save_batch_record(store::RecordStore& store, const std::string& md5,
                              const store::Profile& profile) {
    store.save(store::prepare_record(md5, profile, empty_analysis_under(profile)));
}

// The same for chart `md5` under batch_profile(cap). Returns the profile it
// saved under.
inline store::Profile save_batch_record(store::RecordStore& store, const std::string& md5,
                                        int cap) {
    const store::Profile profile = batch_profile(cap);
    save_batch_record(store, md5, profile);
    return profile;
}

// ---- a Stale result for each cause ----------------------------------------------

// The stamp a record carries when another Hydra version wrote it.
inline constexpr const char* kOldBuildStamp = "0.0.0";

// Saves `prepared` through `store` as if another Hydra version had saved it,
// under kOldBuildStamp, through RecordStore::save_under_build_for_test. Stored
// this way it reads Stale for "another build".
inline void save_old_build_record(store::RecordStore& store,
                                  const store::PreparedRecord& prepared) {
    store.save_under_build_for_test(prepared, kOldBuildStamp);
}

// Rules other than the defaults, as a user's hydra_rules.ini might set them:
// max_tied_paths 2 instead of the default.
inline core::Rules other_rules() {
    core::Rules other = core::default_rules();
    other.max_tied_paths = 2;
    return other;
}

// `record` stamped as analyzed under other_rules(). Stored, it reads Stale for
// "other rules" in a store running the defaults.
inline HydraRecord other_rules_record(const HydraRecord& record) {
    HydraRecord foreign = record;
    foreign.rules_fingerprint = other_rules().fingerprint();
    return foreign;
}

// Saves `record` under `profile` for three charts, one for each reason a saved
// result reads Stale: chart `build` as another build's, chart `rules` as an
// other_rules_record, and chart `both` with both.
inline void add_stale_records(store::RecordStore& store, const HydraRecord& record,
                              const store::Profile& profile, const std::string& build,
                              const std::string& rules, const std::string& both) {
    const HydraRecord foreign = other_rules_record(record);
    save_old_build_record(store, store::prepare_record(build, profile, record));
    save_old_build_record(store, store::prepare_record(both, profile, foreign));
    store.save(store::prepare_record(rules, profile, foreign));
}

// Lists chart `md5` in `store`'s library under these names, keeping every
// other chart's library row and replacing this chart's. A result's names come
// from the library (ChartIndex::naming), and a report leaves out a result the
// library doesn't list, so a test that reads names or reports calls this
// before it saves the chart's results.
inline void name_chart(store::RecordStore& store, const std::string& md5,
                       const std::string& title, const std::string& artist = "Artist",
                       const std::string& charter = "Charter") {
    std::vector<store::ChartLibraryEntry> rows = store.list_chart_library(0, -1);
    rows.erase(std::remove_if(rows.begin(), rows.end(),
                              [&](const store::ChartLibraryEntry& e) { return e.md5 == md5; }),
               rows.end());
    rows.push_back(library_row(md5, title, artist, charter));
    store.rebuild_chart_library(rows);
}

// ---- a title made only of tags ------------------------------------------------

// A song title that is nothing but Clone Hero rich-text tags: a <color=...>
// pair and a bare <b></b> pair. Stripping the tags leaves nothing to show.
inline constexpr const char* kTagOnlyTitle = "<color=#FF8000></color><b></b>";

// ---- sanity cases -------------------------------------------------------------

TEST_CASE("fixtures: audio runs 5 s past the last note") {
    const AudioTailChart c = audio_tail_chart();
    CHECK(c.last_note_ms == app::last_note_ms(app::build_preview_base(c.song)));
    CHECK(c.audio_end_ms - c.last_note_ms == 5000.0);
}

TEST_CASE("fixtures: a batch record saved for one chart") {
    store::RecordStore db(":memory:");
    const store::Profile profile = save_batch_record(db, "fixture-chart", app::Settings{}.sp_cap);
    CHECK(profile == app::Settings{}.profile());
    CHECK(db.lookup_one("fixture-chart", profile).status == store::RecordStatus::Ready);
    CHECK(db.lookup_one("another-chart", profile).status == store::RecordStatus::NotAnalyzed);
}

TEST_CASE("fixtures: a Stale record for each cause") {
    store::RecordStore db(":memory:");
    const store::Profile profile = batch_profile(app::Settings{}.sp_cap);
    const HydraRecord record = empty_analysis_under(profile);
    add_stale_records(db, record, profile, "build", "rules", "both");
    save_batch_record(db, "ready", profile);

    const store::RecordLookup build = db.lookup_one("build", profile);
    CHECK(build.status == store::RecordStatus::Stale);
    CHECK(build.stale_build);
    CHECK_FALSE(build.stale_rules);
    const store::RecordLookup rules = db.lookup_one("rules", profile);
    CHECK(rules.status == store::RecordStatus::Stale);
    CHECK_FALSE(rules.stale_build);
    CHECK(rules.stale_rules);
    const store::RecordLookup both = db.lookup_one("both", profile);
    CHECK(both.status == store::RecordStatus::Stale);
    CHECK(both.stale_build);
    CHECK(both.stale_rules);
    CHECK(db.lookup_one("ready", profile).status == store::RecordStatus::Ready);
}

TEST_CASE("fixtures: a title made only of tags") {
    const std::string stripped = app::strip_rich_tags(kTagOnlyTitle);
    CHECK(stripped == "");
    CHECK(title_or_unknown(stripped) == "(unknown)");
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_DISPLAY_FIXTURES_H
