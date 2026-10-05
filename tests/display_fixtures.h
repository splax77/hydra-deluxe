// Shared test fixtures that carry display-side facts: how long a song's audio
// runs, a result stored behind the app's back, and a title. Phase 3's display
// tests all need the same few awkward inputs; they live here once so no test
// file builds its own. Records and paths built by hand live in
// record_fixtures.h, which includes this file at its end.
#ifndef HYDRA_TESTS_DISPLAY_FIXTURES_H
#define HYDRA_TESTS_DISPLAY_FIXTURES_H

#include <string>

#include "doctest.h"

#include "app/config.h"
#include "app/library_query.h"
#include "app/preview_view.h"  // last_note_ms, for the audio-tail sanity case
#include "core/model.h"
#include "core/rules.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "store/record_store.h"

namespace hydra::test {

// ---- a song whose audio runs past its last note -----------------------------

// How far the audio runs past the last note: 5 s.
inline constexpr double kAudioTailMs = 5000.0;

// A chart whose audio keeps playing after its last note. `last_note_ms` is
// the last note's onset; `audio_end_ms` is where the audio stops, kAudioTailMs
// later, which is the song's length (D69). Numbers only, no playhead.
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

// ---- a batch result stored for one chart -------------------------------------

// The key a result for chart `md5` is filed under at SP cap `cap`, with every
// other setting at its default. Settings::record_key builds it, so a test
// never spells the key's parts out itself.
inline store::RecordKey batch_result_key(const std::string& md5, int cap) {
    app::Settings settings;
    settings.sp_cap = cap;
    return settings.record_key(md5);
}

// Stores a finished result the way a batch does, behind the app's back: an
// empty current-version record under `key`. An empty current-version record
// reads back Ready, with no paths and so no score. The record's SP cap, ms
// limit and fill rule are read from the key, so the two never disagree
// (prepare_row refuses a mismatch); this form is for a test that files under
// its own chartmode, lens or fill rule.
inline void store_batch_result(store::RecordStore& store, const store::RecordKey& key) {
    HydraRecord record;
    record.sp_cap = key.cap.exact;
    if (key.lens.ms_enabled) record.ms_limit = key.lens.ms_value;
    record.legacy_fills = key.lens.legacy_fills != 0;
    store.add_record(key, record);
}

// The same empty record for chart `md5` at SP cap `cap`, under
// batch_result_key (every other setting at its default). Returns the key it
// stored under.
inline store::RecordKey store_batch_result(store::RecordStore& store, const std::string& md5,
                                           int cap) {
    const store::RecordKey key = batch_result_key(md5, cap);
    store_batch_result(store, key);
    return key;
}

// ---- a Stale result for each cause ----------------------------------------------

// The row `record` makes under `key`, as if another Hydra version wrote it
// (hyversion "0.0.0"). Stored, it reads Stale for "another build".
inline store::PreparedRow old_build_row(const store::RecordKey& key, const HydraRecord& record) {
    store::PreparedRow row = store::prepare_row(key, record);
    row.hyversion = "0.0.0";
    return row;
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

// Stores `record` three times, once for each reason a saved result reads
// Stale: under `build` as an old_build_row, under `rules` as an
// other_rules_record, and under `both` with both.
inline void add_stale_rows(store::RecordStore& store, const HydraRecord& record,
                           const store::RecordKey& build, const store::RecordKey& rules,
                           const store::RecordKey& both) {
    const HydraRecord foreign = other_rules_record(record);
    store.add_row(old_build_row(build, record));
    store.add_row(store::prepare_row(rules, foreign));
    store.add_row(old_build_row(both, foreign));
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

TEST_CASE("fixtures: a batch result stored for one chart") {
    store::RecordStore db(":memory:");
    const int cap = app::Settings{}.sp_cap;
    const store::RecordKey key = store_batch_result(db, "fixture-chart", cap);
    CHECK(db.get_record(key).status == store::RecordStatus::Ready);
    store::RecordKey other = key;
    other.hyhash = "another-chart";
    CHECK(db.get_record(other).status == store::RecordStatus::NotAnalyzed);
}

TEST_CASE("fixtures: a title made only of tags") {
    const std::string stripped = app::strip_rich_tags(kTagOnlyTitle);
    CHECK(stripped == "");
    CHECK(title_or_unknown(stripped) == "(unknown)");
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_DISPLAY_FIXTURES_H
