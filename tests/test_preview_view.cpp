// Tests for app/preview_view: the note-highway view-model the Preview tab
// renders. A hand-built song pins the note/lane/attribute mapping and the
// shaded spans exactly; the corpus cases confirm the SP-phrase-start the parser
// now keeps, and that an analyzed chart's overlay lines up with its path.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <limits>
#include <map>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
#include "app/path_view.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/squeeze_rating.h"
#include "core/timing.h"  // sp_bars_to_measures
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"

using namespace hydra;
using namespace hydra::app;

namespace {

// A tiny 4/4, 120 BPM song built by hand: 480 ticks/quarter, so tick t is at
// t*500/480 ms and a measure is 1920 ticks. Four timestamps exercise every
// note attribute and every shaded-span source.
Song make_hand_song() {
    Song song(480);
    song.bpm_changes[0] = 120.0;  // tpm_changes[0] defaults to 1920 (4/4)
    song.build_timing();

    auto push = [&](int64_t tick, Chord chord) -> SongTimestamp& {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord = std::move(chord);
        song.sequence.push_back(std::move(ts));
        return song.sequence.back();
    };

    // tick 0: a plain Red.
    {
        Chord c;
        c.add_note(NoteColor::Red);
        push(0, c);
    }
    // tick 240: a Yellow cymbal, ghost; start of a solo.
    {
        Chord c;
        c.add_note(NoteColor::Yellow);
        c.apply_cymbal(NoteColor::Yellow);
        c.apply_ghost(NoteColor::Yellow);
        push(240, c).flag_solo = true;
    }
    // tick 480: a Blue accent; still in the solo.
    {
        Chord c;
        c.add_note(NoteColor::Blue);
        c.apply_accent(NoteColor::Blue);
        push(480, c).flag_solo = true;
    }
    // tick 720: a 2x-kick + Green chord; ends an SP phrase that began at tick
    // 240, and sits at the end of an activation fill 480 ticks long.
    {
        Chord c;
        c.add_2x();  // adds the Kick note itself, with is2x set
        c.add_note(NoteColor::Green);
        SongTimestamp& ts = push(720, c);
        ts.flag_sp = true;
        ts.sp_phrase_start = 240;
        ts.activation_length = 480;
    }
    return song;
}

// A song with four candidate activation fills, each 240 ticks long, ending on
// the notes at ticks 480, 960, 1440 and 1920.
Song make_fill_song() {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int i = 1; i <= 4; ++i) {
        Chord c;
        c.add_note(NoteColor::Green);
        SongTimestamp ts;
        ts.timecode = song.timecode(480 * i);
        ts.chord = std::move(c);
        ts.activation_length = 240;
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// An activation at `tick` that passed over the fills ending at `skipped_fills`.
Activation act_at(const Song& song, int64_t tick, std::vector<int64_t> skipped_fills) {
    Activation a;
    a.timecode = song.timecode(tick);
    a.skipped_fill_ticks = std::move(skipped_fills);
    return a;
}

// ---- SP meter fixtures --------------------------------------------------
//
// Same 4/4, 120 BPM grid as make_hand_song: a note every `step` ticks out to
// `last_tick`, with an SP phrase ending on each tick in `phrase_ends`. One
// measure is 1920 ticks = 2000 ms, so one SP bar (two measures) burns 4000 ms
// at this tempo. `extra_bpm` adds tempo changes before the timing is built.
// A phrase can only end on a note, so `step` is there for a fixture that needs
// a phrase off the quarter-note grid. `extra_tpm` adds meter changes (ticks per
// measure) the same way.
Song make_sp_song(const std::vector<int64_t>& phrase_ends, int64_t last_tick,
                  const std::map<int64_t, double>& extra_bpm = {},
                  int64_t step = 480,
                  const std::map<int64_t, int64_t>& extra_tpm = {}) {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    for (const auto& kv : extra_bpm) song.bpm_changes[kv.first] = kv.second;
    for (const auto& kv : extra_tpm) song.tpm_changes[kv.first] = kv.second;
    song.build_timing();

    for (int64_t t = 0; t <= last_tick; t += step) {
        Chord c;
        c.add_note(NoteColor::Red);
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord = std::move(c);
        if (std::find(phrase_ends.begin(), phrase_ends.end(), t) != phrase_ends.end()) {
            ts.flag_sp = true;
            ts.sp_phrase_start = t >= step ? t - step : 0;
        }
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// An activation the engine could have recorded: a timecode, the bars it
// spends, and the SP end the search stamped on it. Nothing derives that end
// any more, so the fixture states it as a literal tick. The window is plain:
// an activation that collects no phrase mid-SP. A fixture that collects one
// states its whole SP-end history itself.
Activation sp_act_at(const Song& song, int64_t tick, int sp_meter, int64_t end_tick) {
    Activation a;
    a.timecode = song.timecode(tick);
    test::set_sp_meter(a, sp_meter);
    test::set_skips(a, 0);
    test::set_plain_window(a, end_tick);
    return a;
}

// The curve must tile the timeline with no gap and no overlap, and never leave
// the 0..cap band (every fixture here spends no more than the cap).
void check_curve_well_formed(const SpMeterCurve& curve) {
    for (size_t i = 1; i < curve.segments.size(); ++i)
        CHECK(curve.segments[i].start_ms == doctest::Approx(curve.segments[i - 1].end_ms));
    for (const SpMeterSegment& s : curve.segments) {
        CHECK(s.end_ms >= s.start_ms);
        CHECK(s.start_bars >= 0.0);
        CHECK(s.end_bars >= 0.0);
        CHECK(s.start_bars <= static_cast<double>(curve.cap) + 1e-9);
        CHECK(s.end_bars <= static_cast<double>(curve.cap) + 1e-9);
    }
}

// The engine fixture from test_search.cpp ("SP cap overfill: a second clamp
// in the same window replaces clamp_tick"), rebuilt with each phrase's start
// tick set so the Preview draws the phrases. 192 ticks per beat, 4/4, 120
// BPM: a measure is 768 ticks and 2000 ms.
Song make_overfill_song() {
    struct N { int64_t tick; bool phrase; bool fill; };
    const std::vector<N> notes = {{0, true, false},    {768, true, false},
                                  {2304, false, true}, {3072, true, false},
                                  {3840, true, false}, {4608, false, false},
                                  {5376, false, false}, {6000, false, false},
                                  {6768, false, false}, {7500, false, false}};
    Song song(192);
    song.tpm_changes[0] = 768;
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (const N& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        ts.flag_sp = n.phrase;
        if (n.phrase) ts.sp_phrase_start = n.tick;
        if (n.fill) ts.activation_length = 384;
        song.sequence.push_back(ts);
    }
    return song;
}

// One analyzed corpus chart (the first that yields paths), shared across cases.
const AnalysisResult& analyzed() {
    static const AnalysisResult result = [] {
        AnalysisSettings settings;
        settings.depth_mode = DepthMode::Scores;
        settings.depth_value = 10;
        settings.ms_filter = 10.0;
        for (const std::string& path : corpus::chart_paths()) {
            try {
                AnalysisResult r = analyze_chart_file(path, settings);
                if (!r.song.is_empty() && !r.record.paths.empty()) return r;
            } catch (const std::exception&) {
                continue;
            }
        }
        throw std::runtime_error("no analyzable corpus chart");
    }();
    return result;
}

// A path whose stored score is what the replay prices it at, so the scene
// trusts it. The corpus case proves the replay against the engine; these
// hand-built fixtures only test the Preview's plumbing.
Path priced_path(const Song& song, std::vector<Activation> acts) {
    Path p;
    p.activations = std::move(acts);
    const ReplayScore s = replay_stored_path(song, p).result.final;
    p.score_base = s.base;
    p.score_combo = s.combo;
    p.score_sp = s.sp;
    p.score_solo = s.solo;
    p.score_accents = s.accent;
    p.score_ghosts = s.ghost;
    return p;
}

// The detail line's separator: a middle dot, U+00B7, in UTF-8.
const std::string kDot = "\xC2\xB7";

}  // namespace

TEST_CASE("build_preview_scene: notes carry lane and drum attributes") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);

    REQUIRE(scene.has_notes);
    REQUIRE(scene.notes.size() == 5);  // 1 + 1 + 1 + 2 (Kick+Green)

    const PreviewNote& red = scene.notes[0];
    CHECK(red.lane == PreviewLane::Red);
    CHECK(red.tick == 0);
    CHECK(red.ms == doctest::Approx(0.0));
    CHECK_FALSE(red.solo);

    const PreviewNote& yellow = scene.notes[1];
    CHECK(yellow.lane == PreviewLane::Yellow);
    CHECK(yellow.cymbal);
    CHECK(yellow.ghost);
    CHECK(yellow.solo);
    CHECK(yellow.ms == doctest::Approx(250.0));

    const PreviewNote& blue = scene.notes[2];
    CHECK(blue.lane == PreviewLane::Blue);
    CHECK(blue.accent);
    CHECK(blue.solo);

    // The chord at tick 720 expands to Kick (2x) then Green, in KRYBG order.
    CHECK(scene.notes[3].lane == PreviewLane::Kick);
    CHECK(scene.notes[3].double_kick);
    CHECK(scene.notes[4].lane == PreviewLane::Green);

    CHECK(scene.song_length_ms == doctest::Approx(750.0));
}

TEST_CASE("build_preview_scene: SP phrase, solo, and fill spans") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);

    REQUIRE(scene.sp_phrases.size() == 1);
    CHECK(scene.sp_phrases[0].start_tick == 240);
    CHECK(scene.sp_phrases[0].end_tick == 720);
    CHECK(scene.sp_phrases[0].start_ms == doctest::Approx(250.0));
    CHECK(scene.sp_phrases[0].end_ms == doctest::Approx(750.0));

    REQUIRE(scene.solos.size() == 1);
    CHECK(scene.solos[0].start_tick == 240);
    CHECK(scene.solos[0].end_tick == 480);

    REQUIRE(scene.fills.size() == 1);
    CHECK(scene.fills[0].span.start_tick == 240);  // 720 - 480
    CHECK(scene.fills[0].span.end_tick == 720);
}

TEST_CASE("build_preview_scene: an unanalyzed chart offers every candidate fill") {
    Song song = make_fill_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.fills.size() == 4);
    for (const PreviewFill& f : scene.fills) CHECK(f.state == PreviewFillState::Offered);
}

TEST_CASE("build_preview_scene: skips say which fills the path was offered") {
    Song song = make_fill_song();
    Path path;
    // The path activates on the third fill after passing over the one at 960.
    path.activations = {act_at(song, 1440, {960})};

    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.fills.size() == 4);
    CHECK(scene.fills[0].state == PreviewFillState::Hidden);   // not enough SP
    CHECK(scene.fills[1].state == PreviewFillState::Offered);  // the one skip
    CHECK(scene.fills[2].state == PreviewFillState::Taken);
    CHECK(scene.fills[3].state == PreviewFillState::Hidden);   // past the last act
    CHECK(scene.fills[2].span.end_tick == 1440);
}

TEST_CASE("build_preview_scene: a second activation with skips 0 hides what lies between") {
    Song song = make_fill_song();
    Path path;
    path.activations = {act_at(song, 960, {}), act_at(song, 1920, {})};

    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.fills.size() == 4);
    CHECK(scene.fills[0].state == PreviewFillState::Hidden);
    CHECK(scene.fills[1].state == PreviewFillState::Taken);
    CHECK(scene.fills[2].state == PreviewFillState::Hidden);  // SP was still active
    CHECK(scene.fills[3].state == PreviewFillState::Taken);

    // With the fill at 1440 passed over before the second activation, the fill
    // between them is offered instead.
    Path skipped;
    skipped.activations = {act_at(song, 960, {}), act_at(song, 1920, {1440})};
    PreviewScene s2 = build_preview_scene(song, &skipped);
    CHECK(s2.fills[1].state == PreviewFillState::Taken);
    CHECK(s2.fills[2].state == PreviewFillState::Offered);
    CHECK(s2.fills[3].state == PreviewFillState::Taken);
}

TEST_CASE("build_preview_scene: a fill an activation took stays taken when a later one lists it") {
    // A tied variant takes its activations after the fold, and their stored
    // passed-over fills, from its leader (finding 97). If the variant
    // activated on a fill its leader passed over, the variant's next
    // activation lists that fill as passed over. The fill was taken; it must
    // not turn offered.
    Song song = make_fill_song();
    Path path;
    path.activations = {act_at(song, 960, {}), act_at(song, 1920, {960, 1440})};

    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.fills.size() == 4);
    CHECK(scene.fills[0].state == PreviewFillState::Hidden);
    CHECK(scene.fills[1].state == PreviewFillState::Taken);    // taken, though listed later
    CHECK(scene.fills[2].state == PreviewFillState::Offered);
    CHECK(scene.fills[3].state == PreviewFillState::Taken);
}

TEST_CASE("build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged") {
    // Finding 52. Fill B's 1.0 deadline falls before fill A's, so B never
    // spawned and A was shown and passed over. The nearest-n guess lit B.
    Song song = test::make_ch10_fill_song();
    ScoreGraph graph(song, 4, FillDeadlineRule::Ch10);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{28800};
    std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    PreviewScene scene = build_preview_scene(song, &paths.front());
    REQUIRE(scene.fills.size() == 3);
    CHECK(scene.fills[0].span.end_tick == 19200);
    CHECK(scene.fills[0].state == PreviewFillState::Offered);
    CHECK(scene.fills[1].state == PreviewFillState::Hidden);
    CHECK(scene.fills[2].state == PreviewFillState::Taken);
}

TEST_CASE("build_beat_events: bars, beats, and half-beats from the timing alone") {
    // 4/4 at 480 ticks/quarter: bars every 1920, beats every 480.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming st(480, tpm, bpm);

    std::vector<PreviewBeat> beats = build_beat_events(st, 1920);
    // ticks: 0 Bar, 240 Half, 480 Beat, 720 Half, 960 Beat, 1200 Half,
    // 1440 Beat, 1680 Half, 1920 Bar  (no Half before tick 0)
    REQUIRE(beats.size() == 9);
    CHECK(beats[0].tick == 0);
    CHECK(beats[0].kind == PreviewBeatKind::Bar);
    CHECK(beats[1].tick == 240);
    CHECK(beats[1].kind == PreviewBeatKind::Half);
    CHECK(beats[2].tick == 480);
    CHECK(beats[2].kind == PreviewBeatKind::Beat);
    CHECK(beats[6].tick == 1440);
    CHECK(beats[6].kind == PreviewBeatKind::Beat);
    CHECK(beats[8].tick == 1920);
    CHECK(beats[8].kind == PreviewBeatKind::Bar);
    CHECK(beats[8].ms == doctest::Approx(2000.0));
    for (size_t i = 1; i < beats.size(); ++i) CHECK(beats[i].tick > beats[i - 1].tick);
}

TEST_CASE("build_beat_events: a 3/4 section changes the beat count per bar") {
    // One bar of 4/4, then 3/4 (1440 ticks per measure).
    std::map<int64_t, int64_t> tpm{{0, 1920}, {1920, 1440}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming st(480, tpm, bpm);

    std::vector<PreviewBeat> beats = build_beat_events(st, 1920 + 1440 * 2);
    int bars = 0, quarter_beats = 0;
    for (const PreviewBeat& b : beats) {
        if (b.kind == PreviewBeatKind::Bar) ++bars;
        if (b.kind == PreviewBeatKind::Beat) ++quarter_beats;
    }
    CHECK(bars == 4);              // ticks 0, 1920, 3360, 4800
    CHECK(quarter_beats == 3 + 2 + 2);  // 3 in the 4/4 bar, 2 in each 3/4 bar
    // Every Beat line lies strictly inside its bar.
    bool saw_3360 = false;
    for (const PreviewBeat& b : beats)
        if (b.tick == 3360) saw_3360 = b.kind == PreviewBeatKind::Bar;
    CHECK(saw_3360);
}

TEST_CASE("build_preview_scene fills beats, tempos and resolution") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.tick_resolution == 480);
    REQUIRE(scene.tempos.size() == 1);
    CHECK(scene.tempos[0].bpm == doctest::Approx(120.0));
    REQUIRE(!scene.beats.empty());
    CHECK(scene.beats.front().tick == 0);
    // Extends two measures past the last note (tick 720 -> through 4560; the
    // last line at or before that is the beat at 4320).
    CHECK(scene.beats.back().tick == 4320);
}

TEST_CASE("build_time_box: timestamp, measure, tempo") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);

    // 1300 ms is tick 1248: bar 1, third beat, 288 ticks into it. 5000 ms is
    // tick 4800: the third bar's third beat, exactly on the line.
    PreviewTimeBox box = build_time_box(scene, 1300.0, 5000.0);
    CHECK(box.timestamp == "0:01.300 / 0:05.000");
    CHECK(box.position == "m1.3.288");
    CHECK(box.length == "m3.3.0");
    CHECK(box.tempo == "BPM 120.000 " + kDot + " 4/4");
    CHECK(box.section_line.empty());

    CHECK(build_time_box(scene, 0.0, 5000.0).position == "m1.1.0");

    PreviewTimeBox later = build_time_box(scene, 64000.0, 64000.0);
    CHECK(later.timestamp == "1:04.000 / 1:04.000");

    // The playhead is clamped to the length.
    CHECK(build_time_box(scene, 9999.0, 5000.0).timestamp == "0:05.000 / 0:05.000");
}

TEST_CASE("build_time_box: the end measure runs past the last beat line") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    // The beat grid stops at tick 4320, but the last tempo and meter hold
    // forever: 30 s is tick 28800, which is bar 16 on the nose.
    REQUIRE(scene.beats.back().tick == 4320);
    PreviewTimeBox box = build_time_box(scene, 30000.0, 30000.0);
    CHECK(box.position == "m16.1.0");
    CHECK(box.length == "m16.1.0");
    CHECK(box.timestamp == "0:30.000 / 0:30.000");
}

TEST_CASE("build_time_box: the practice section in force") {
    Song song = make_hand_song();
    song.practice_sections.push_back({480, "Verse 1"});
    song.practice_sections.push_back({2400, "Chorus"});
    PreviewScene scene = build_preview_scene(song, nullptr);

    REQUIRE(scene.sections.size() == 2);
    CHECK(scene.sections[0].tick == 480);
    CHECK(scene.sections[0].ms == doctest::Approx(500.0));
    CHECK(scene.sections[0].name == "Verse 1");
    // Past the last note (tick 720), where the ms index has no entry.
    CHECK(scene.sections[1].ms == doctest::Approx(2500.0));

    const double len = 10000.0;
    CHECK(build_time_box(scene, 0.0, len).section_line.empty());
    CHECK(build_time_box(scene, 400.0, len).section_line.empty());
    CHECK(build_time_box(scene, 500.0, len).section_line == "Section Verse 1");
    CHECK(build_time_box(scene, 2000.0, len).section_line == "Section Verse 1");
    CHECK(build_time_box(scene, 2500.0, len).section_line == "Section Chorus");
    CHECK(build_time_box(scene, 9000.0, len).section_line == "Section Chorus");
}

TEST_CASE("build_time_box: a mid-measure meter change follows the engine") {
    // 4/4 from tick 0, 3/4 from tick 2880 — beat 3 of the second bar, not a
    // barline. A section's bars count from the last barline at or before its
    // first tick (tick 1920 here), so the bar lines run 0, 1920, 3360, 4800 and
    // the second bar is cut short. The box must name ticks the way the engine
    // does rather than by a count of its own.
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.tpm_changes[2880] = 1440;
    song.build_timing();
    {
        Chord c;
        c.add_note(NoteColor::Red);
        SongTimestamp ts;
        ts.timecode = song.timecode(3600);
        ts.chord = std::move(c);
        song.sequence.push_back(std::move(ts));
    }
    PreviewScene scene = build_preview_scene(song, nullptr);

    // 120 BPM at 480 ticks/quarter: tick 3600 is 3750 ms, past the change.
    const Timecode tc = song.timing().timecode(3600);
    const int64_t* mbt = tc.measure_beats_ticks();
    CHECK(mbt[0] + 1 == 3);  // the engine's measure, 1-based for display
    CHECK(mbt[1] + 1 == 1);
    CHECK(mbt[2] == 240);

    PreviewTimeBox box = build_time_box(scene, 3750.0, 3750.0);
    CHECK(box.position == "m3.1.240");

    // ...and that measure is the one the drawn bar lines put tick 3600 in: the
    // third line is at 3360 and the fourth at 4800.
    std::vector<int64_t> bars;
    for (const PreviewBeat& b : scene.beats)
        if (b.kind == PreviewBeatKind::Bar) bars.push_back(b.tick);
    REQUIRE(bars.size() >= 4);
    CHECK(bars[1] == 1920);
    CHECK(bars[2] == 3360);
    CHECK(bars[3] == 4800);
}

TEST_CASE("build_time_box: the time signature in force, as the chart wrote it") {
    // 120 BPM, 480 ticks per quarter. 6/8 from tick 1920 (2000 ms), then 3/4
    // from tick 3360 (3500 ms). Both are 1440 ticks a measure, so only the
    // stored signature tells them apart.
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.tpm_changes[1920] = 1440;
    song.timesig_changes[1920] = {6, 8};
    song.tpm_changes[3360] = 1440;
    song.timesig_changes[3360] = {3, 4};
    song.build_timing();
    {
        Chord c;
        c.add_note(NoteColor::Red);
        SongTimestamp ts;
        ts.timecode = song.timecode(4800);
        ts.chord = std::move(c);
        song.sequence.push_back(std::move(ts));
    }
    PreviewScene scene = build_preview_scene(song, nullptr);

    CHECK(build_time_box(scene, 0.0, 5000.0).tempo == "BPM 120.000 " + kDot + " 4/4");
    CHECK(build_time_box(scene, 1999.0, 5000.0).tempo == "BPM 120.000 " + kDot + " 4/4");
    CHECK(build_time_box(scene, 2000.0, 5000.0).tempo == "BPM 120.000 " + kDot + " 6/8");
    CHECK(build_time_box(scene, 3500.0, 5000.0).tempo == "BPM 120.000 " + kDot + " 3/4");
    // A scene built from nothing reads the chart default.
    CHECK(build_time_box(PreviewScene{}, 0.0, 0.0).tempo == "BPM 0.000 " + kDot + " 4/4");
    CHECK(build_time_box(PreviewScene{}, 0.0, 0.0).position == "m1.1.0");
}

TEST_CASE("build_preview_scene: no path means no overlay") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.activations.empty());
}

TEST_CASE("parser keeps the SP-phrase start on the flagged note") {
    // Find the first corpus chart with an SP phrase and confirm every note the
    // parser flags as an SP-phrase end now also carries its start tick.
    bool checked_a_chart = false;
    for (const std::string& path : corpus::chart_paths()) {
        Song song = load_songpath(path, /*pro=*/true, /*bass2x=*/true);
        int sp_notes = 0;
        for (const SongTimestamp& ts : song.sequence) {
            if (!ts.flag_sp) continue;
            ++sp_notes;
            REQUIRE(ts.sp_phrase_start.has_value());
            CHECK(*ts.sp_phrase_start <= ts.timecode.ticks());
        }
        if (sp_notes > 0) {
            checked_a_chart = true;
            break;
        }
    }
    REQUIRE(checked_a_chart);  // the corpus must contain an SP-bearing chart
}

TEST_CASE("build_preview_scene: an analyzed chart's overlay matches its path") {
    const AnalysisResult& r = analyzed();
    const Path& best = r.record.best_path();
    PreviewScene scene = build_preview_scene(r.song, &best);

    REQUIRE(scene.has_notes);
    REQUIRE_FALSE(scene.notes.empty());
    CHECK(scene.song_length_ms > 0.0);

    // Notes are in non-decreasing tick order.
    for (size_t i = 1; i < scene.notes.size(); ++i)
        CHECK(scene.notes[i].tick >= scene.notes[i - 1].tick);

    // The scrubber's right edge is the last note's onset.
    CHECK(scene.song_length_ms == doctest::Approx(scene.notes.back().ms));

    // Every activation the path takes (those with a resolved timecode) appears
    // in the overlay, inside the song, in ms that share the notes' timing.
    const size_t expected = best.all_activations().size();
    CHECK(scene.activations.size() == expected);
    size_t i = 0;
    for (const Activation& a : best.all_activations()) {
        const PreviewActivation& pa = scene.activations[i++];
        CHECK(pa.tick >= 0);
        CHECK(pa.ms >= 0.0);
        CHECK(pa.ms <= scene.song_length_ms + 1.0);
        // The active SP window ends exactly at the deact node the record
        // carries, in the song's own ms. The engine stamps that node on every
        // activation it produces, so it is always there on a fresh record.
        std::optional<int64_t> d = activation_deact_tick(a);
        CHECK(d.has_value());
        CHECK(pa.has_sp_end == d.has_value());
        if (d) {
            CHECK(pa.sp_end_tick == *d);
            CHECK(pa.sp_end_ms == doctest::Approx(r.song.timing().ms_index().at(*d)));
            CHECK(pa.sp_end_ms > pa.ms);
        }
        // The activation note's lane is the chord's highest-priority note.
        if (a.chord.count() > 0) {
            CHECK(pa.has_lane);
            CHECK(pa.lane == lane_of(a.chord.activation_note().colortype));
        }
        CHECK(pa.measure == format_measure(r.song.timing(), pa.tick));
        CHECK(pa.chord == (a.chord.count() > 0 ? a.chord.rowstr() : std::string()));
    }

    // Same song, no path: identical notes, empty overlay.
    PreviewScene bare = build_preview_scene(r.song, nullptr);
    CHECK(bare.notes.size() == scene.notes.size());
    CHECK(bare.activations.empty());
}

// ---- base + overlay equals the whole scene --------------------------------

namespace {

// The first PreviewScene member where `a` and `b` differ, named, or "" when
// every member is equal. Exact compares: both sides run the same arithmetic.
// SongTiming has no operator==, so its indexes are compared entry by entry
// and its ms index is read back at every tick the scene names.
std::string scene_difference(const PreviewScene& a, const PreviewScene& b) {
    auto span_eq = [](const PreviewSpan& x, const PreviewSpan& y) {
        return x.start_tick == y.start_tick && x.end_tick == y.end_tick &&
               x.start_ms == y.start_ms && x.end_ms == y.end_ms;
    };
    auto spans_eq = [&](const std::vector<PreviewSpan>& x, const std::vector<PreviewSpan>& y) {
        return std::equal(x.begin(), x.end(), y.begin(), y.end(), span_eq);
    };
    if (!std::equal(a.notes.begin(), a.notes.end(), b.notes.begin(), b.notes.end(),
                    [](const PreviewNote& x, const PreviewNote& y) {
                        return x.tick == y.tick && x.ms == y.ms && x.measure == y.measure &&
                               x.lane == y.lane && x.cymbal == y.cymbal && x.ghost == y.ghost &&
                               x.accent == y.accent && x.double_kick == y.double_kick &&
                               x.solo == y.solo;
                    }))
        return "notes";
    if (!spans_eq(a.sp_phrases, b.sp_phrases)) return "sp_phrases";
    if (!spans_eq(a.solos, b.solos)) return "solos";
    if (!std::equal(a.fills.begin(), a.fills.end(), b.fills.begin(), b.fills.end(),
                    [&](const PreviewFill& x, const PreviewFill& y) {
                        return span_eq(x.span, y.span) && x.state == y.state;
                    }))
        return "fills";
    if (!std::equal(a.activations.begin(), a.activations.end(), b.activations.begin(),
                    b.activations.end(),
                    [](const PreviewActivation& x, const PreviewActivation& y) {
                        return x.tick == y.tick && x.ms == y.ms &&
                               x.skipped_fill_ticks == y.skipped_fill_ticks &&
                               x.has_sp_end == y.has_sp_end &&
                               x.sp_end_tick == y.sp_end_tick && x.sp_end_ms == y.sp_end_ms &&
                               x.sp_end_changes == y.sp_end_changes &&
                               x.bank_rise_ticks == y.bank_rise_ticks &&
                               x.lane == y.lane && x.has_lane == y.has_lane &&
                               x.measure == y.measure && x.chord == y.chord;
                    }))
        return "activations";
    if (a.trailing_bank_ticks != b.trailing_bank_ticks) return "trailing_bank_ticks";
    if (!std::equal(a.beats.begin(), a.beats.end(), b.beats.begin(), b.beats.end(),
                    [](const PreviewBeat& x, const PreviewBeat& y) {
                        return x.tick == y.tick && x.ms == y.ms && x.kind == y.kind;
                    }))
        return "beats";
    if (!std::equal(a.tempos.begin(), a.tempos.end(), b.tempos.begin(), b.tempos.end(),
                    [](const PreviewTempo& x, const PreviewTempo& y) {
                        return x.tick == y.tick && x.ms == y.ms && x.bpm == y.bpm;
                    }))
        return "tempos";
    if (!std::equal(a.sections.begin(), a.sections.end(), b.sections.begin(), b.sections.end(),
                    [](const PreviewSection& x, const PreviewSection& y) {
                        return x.tick == y.tick && x.ms == y.ms && x.name == y.name;
                    }))
        return "sections";
    if (!std::equal(a.meters.begin(), a.meters.end(), b.meters.begin(), b.meters.end(),
                    [](const PreviewMeter& x, const PreviewMeter& y) {
                        return x.tick == y.tick && x.tpm == y.tpm && x.first_bar == y.first_bar;
                    }))
        return "meters";
    if (!std::equal(a.time_sigs.begin(), a.time_sigs.end(), b.time_sigs.begin(),
                    b.time_sigs.end(), [](const PreviewTimeSig& x, const PreviewTimeSig& y) {
                        return x.tick == y.tick && x.numerator == y.numerator &&
                               x.denominator == y.denominator;
                    }))
        return "time_sigs";
    if (a.sp_meter.cap != b.sp_meter.cap ||
        !std::equal(a.sp_meter.segments.begin(), a.sp_meter.segments.end(),
                    b.sp_meter.segments.begin(), b.sp_meter.segments.end(),
                    [](const SpMeterSegment& x, const SpMeterSegment& y) {
                        return x.start_ms == y.start_ms && x.end_ms == y.end_ms &&
                               x.start_bars == y.start_bars && x.end_bars == y.end_bars;
                    }))
        return "sp_meter";
    if (a.score.state != b.score.state ||
        !std::equal(a.score.steps.begin(), a.score.steps.end(), b.score.steps.begin(),
                    b.score.steps.end(), [](const PreviewScoreStep& x, const PreviewScoreStep& y) {
                        return x.ms == y.ms && x.total == y.total &&
                               x.multiplier == y.multiplier &&
                               x.multiplier_plain == y.multiplier_plain &&
                               x.combo == y.combo;
                    }))
        return "score";
    if (a.timing.has_value() != b.timing.has_value()) return "timing (presence)";
    if (a.timing) {
        const SongTiming& x = *a.timing;
        const SongTiming& y = *b.timing;
        if (x.tick_resolution() != y.tick_resolution()) return "timing (resolution)";
        const MeasureIndex& mx = x.measure_index();
        const MeasureIndex& my = y.measure_index();
        if (mx.count() != my.count()) return "timing (meter sections)";
        for (int i = 0; i < mx.count(); ++i)
            if (mx.keys_at(i) != my.keys_at(i) || mx.tpm_at(i) != my.tpm_at(i) ||
                mx.starts_at(i) != my.starts_at(i) || mx.measures_at(i) != my.measures_at(i))
                return "timing (meter section " + std::to_string(i) + ")";
        std::vector<int64_t> ticks{0};
        for (const PreviewTempo& t : a.tempos) ticks.push_back(t.tick);
        for (const PreviewNote& n : a.notes) ticks.push_back(n.tick);
        for (int64_t t : ticks)
            if (x.ms_index().at(t) != y.ms_index().at(t) ||
                x.ms_index().tps_at(t) != y.ms_index().tps_at(t))
                return "timing (ms index at tick " + std::to_string(t) + ")";
    }
    if (a.tick_resolution != b.tick_resolution) return "tick_resolution";
    if (a.song_length_ms != b.song_length_ms) return "song_length_ms";
    if (a.has_notes != b.has_notes) return "has_notes";
    return "";
}

// build_preview_scene against base + overlay for one song and path, and the
// overlay laid over a scene built for `other` (another path, or none): both
// must equal the whole scene, member by member.
void check_split(const Song& song, const Path* path, int sp_cap, const core::Rules& rules,
                 const Path* other, const std::string& what) {
    const PreviewScene whole = build_preview_scene(song, path, sp_cap, rules);
    const std::string split = scene_difference(
        whole, apply_preview_overlay(build_preview_base(song), song, path, sp_cap, rules));
    INFO(what << ": base + overlay differs in " << split);
    CHECK(split.empty());
    const std::string relaid = scene_difference(
        whole, apply_preview_overlay(build_preview_scene(song, other, sp_cap, rules), song, path,
                                     sp_cap, rules));
    INFO(what << ": overlay over another path's scene differs in " << relaid);
    CHECK(relaid.empty());
}

}  // namespace

TEST_CASE("base + overlay: equals build_preview_scene on the hand-built fixtures") {
    const core::Rules& rules = core::default_rules();

    // Not analyzed: no path at all.
    for (const Song& song : {make_hand_song(), make_fill_song(), make_overfill_song()})
        check_split(song, nullptr, kCloneHeroSpCap, rules, nullptr, "no path");

    // The fill fixtures: which fills a path was offered or took.
    {
        Song song = make_fill_song();
        Path one, two;
        one.activations = {act_at(song, 1440, {960})};
        two.activations = {act_at(song, 960, {}), act_at(song, 1920, {1440})};
        check_split(song, &one, kCloneHeroSpCap, rules, &two, "fills, one activation");
        check_split(song, &two, kCloneHeroSpCap, rules, &one, "fills, two activations");
        check_split(song, nullptr, kCloneHeroSpCap, rules, &two, "fills, path dropped");
    }

    // The SP meter fixtures: a collection mid-SP, a squeezed-out phrase, a
    // tempo change inside the window, and a what-if cap.
    {
        Song song = make_sp_song({960, 5760}, /*last_tick=*/17280);
        Path collected;
        Activation act = sp_act_at(song, 3840, 2, 11520);
        act.sp_end_steps = {{3840, 11520, SpEndKind::Activation}, {5760, 15360, SpEndKind::Collected}};
        collected.activations = {act};
        Path plain = priced_path(song, {sp_act_at(song, 3840, 2, 11520)});
        for (int cap : {kCloneHeroSpCap, 2, 6}) {
            check_split(song, &collected, cap, rules, &plain, "sp, collected");
            check_split(song, &plain, cap, rules, nullptr, "sp, priced");
            check_split(song, nullptr, cap, rules, &plain, "sp, no path");
        }
    }
    {
        Song song = make_sp_song({960}, /*last_tick=*/15360, /*extra_bpm=*/{{5760, 180.0}});
        Path path = priced_path(song, {sp_act_at(song, 3840, 1, 7680)});
        check_split(song, &path, kCloneHeroSpCap, rules, nullptr, "sp, tempo change");
    }
    {
        Song song = make_overfill_song();
        Path path;
        path.activations = {act_at(song, 2304, {})};
        check_split(song, &path, kCloneHeroSpCap, rules, nullptr, "overfill");
    }

    // An empty song gives the empty scene either way.
    Song empty(480);
    empty.bpm_changes[0] = 120.0;
    empty.build_timing();
    Path on_empty;
    check_split(empty, nullptr, kCloneHeroSpCap, rules, nullptr, "empty song");
    check_split(empty, &on_empty, 6, rules, nullptr, "empty song, path");
}

TEST_CASE("base + overlay: equals build_preview_scene on every corpus chart and stored path") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int charts = 0, paths = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        ++charts;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const std::vector<const Path*> all = rec.all_paths();
        check_split(song, nullptr, cfg.sp_cap, cfg.rules, all.empty() ? nullptr : all.front(),
                    chart + " (no path)");
        for (size_t i = 0; i < all.size(); ++i) {
            ++paths;
            // Laid over the next path's scene, so every overlay replaces a
            // different one.
            const Path* other = i + 1 < all.size() ? all[i + 1] : nullptr;
            check_split(song, all[i], cfg.sp_cap, cfg.rules, other,
                        chart + " path " + std::to_string(i));
        }
    }
    CHECK(charts > 0);
    CHECK(paths > charts);
}

TEST_CASE("sp meter curve: each phrase's last note banks one bar") {
    // Phrases ending at ticks 960 and 2880, i.e. 1000 ms and 3000 ms.
    Song song = make_sp_song({960, 2880}, /*last_tick=*/5760);
    PreviewScene scene = build_preview_scene(song, nullptr);
    const SpMeterCurve& c = scene.sp_meter;

    REQUIRE_FALSE(c.segments.empty());
    check_curve_well_formed(c);
    CHECK(c.cap == 4);

    CHECK(sp_meter_bars_at(c, 0.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 999.0) == doctest::Approx(0.0));
    // The step lands ON the phrase's last note: the later segment owns the
    // shared boundary, so the bar is already banked at exactly 1000 ms.
    CHECK(sp_meter_bars_at(c, 1000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 1000.001) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 2999.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 3000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(2.0));
}

TEST_CASE("sp meter curve: the bank between windows is the record's, not a phrase count") {
    // The chart has one phrase, at 960 (1000 ms). The record says bars
    // arrived at 960 and at 1920 (2000 ms), and one more after the window at
    // 12480 (13000 ms). Neither of the last two has a phrase. The gauge
    // follows the record.
    Song song = make_sp_song({960}, /*last_tick=*/15360);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.bank_rise_ticks = {960, 1920};            // two bars; no phrase at 1920
    path.activations = {act};
    path.trailing_bank_ticks = {12480};           // a bar after the window; no phrase there

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    // Two bars is four measures: 8000 ms at 120 BPM 4/4, ending at tick 11520.
    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].has_sp_end);
    CHECK(scene.activations[0].sp_end_tick == 11520);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(12000.0));

    CHECK(sp_meter_bars_at(c, 999.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 1000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 1999.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 2000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    // One bar's worth of drain is two measures = 4000 ms.
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 13000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 13000.0) == doctest::Approx(1.0));
}

TEST_CASE("sp meter curve: a phrase collected mid-activation jumps the meter a bar") {
    // Phrases at 1000 ms and 6000 ms; the second lands inside the activation's
    // window. The record's second SP-end step is what says it was collected
    // during SP: at 5760 the end moves two measures later, so the window runs
    // six measures instead of four and the meter steps up a bar there.
    Song song = make_sp_song({960, 5760}, /*last_tick=*/17280);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 0.0});
    // 4 measures banked, then 2 more for the collection at 5760.
    act.sp_end_steps = {{3840, 11520, SpEndKind::Activation}, {5760, 15360, SpEndKind::Collected}};
    path.activations = {act};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    REQUIRE(scene.activations.size() == 1);
    std::optional<int64_t> deact = activation_deact_tick(path.activations[0]);
    REQUIRE(deact.has_value());
    CHECK(*deact == 15360);  // 3840 + 6 measures
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(16000.0));

    // One bar per two measures = 0.25 bars per second here, before the jump...
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 5000.0) == doctest::Approx(1.75));
    // Exactly one bar more at the phrase's last note than just before it.
    CHECK(sp_meter_bars_at(c, 6000.0 - 1e-6) == doctest::Approx(1.5));
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(2.5));
    // ...and the same rate after it, all the way to the extended deact node.
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 16000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 16000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: a squeezed-out phrase does not bank mid-drain") {
    // The phrase at tick 9600 (10000 ms) has its last note inside the
    // activation's window, but the deact node sits at the plain act + 4
    // measures: the engine records no extension, so nothing was collected
    // during SP. That phrase is the squeezed-out one -- hit late, just after
    // SP ends, and banked for the next activation. The drain must stay on its
    // plain line across it, and the bar must arrive when the window closes.
    Song song = make_sp_song({960, 9600}, /*last_tick=*/15360);
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};
    path.trailing_bank_ticks = {11520};  // its bar arrives at the deact node

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].sp_end_tick == 11520);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(12000.0));

    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    // Straight through the phrase at one bar per two measures: no step.
    CHECK(sp_meter_bars_at(c, 10000.0 - 1e-6) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 10000.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 11000.0) == doctest::Approx(0.25));
    // Empty at the deact node, then the squeezed-out phrase's bar lands.
    CHECK(sp_meter_bars_at(c, 12000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 13000.0) == doctest::Approx(1.0));
}

TEST_CASE("sp meter curve: a full bank that collects a phrase and stores no row") {
    // Regression, from a gameplay video the user checked against the Preview.
    //
    // A four-bar activation collects one SP phrase while Star Power is
    // running, and no note lands within the engine's 500 ms squeeze window
    // after the deactivation node. So the record stores no backend row at all,
    // and the old code had nothing to read the node out of: it fell back to
    // "two measures per banked bar", which cannot see the collection. The
    // meter emptied two measures early and then showed a bar it had never
    // banked.
    //
    // Now the search stamps the node and the Preview just reads it. Eight
    // measures for the four banked bars plus two for the collection: ten.
    //
    // The phrase's last note sits 3 and 15/16 measures into the window, off
    // the quarter-note grid, so the drain is caught mid-measure on both sides
    // of the step.
    const int64_t act_tick = 3840;                 // 4000 ms
    const int64_t phrase_tick = act_tick + 7560;   // 11400 -> 11875 ms
    const int64_t deact = act_tick + 10 * 1920;    // 23040 -> 24000 ms
    Song song = make_sp_song({phrase_tick}, /*last_tick=*/28800, /*extra_bpm=*/{},
                             /*step=*/120);
    Path path;
    Activation act = sp_act_at(song, act_tick, /*sp_meter=*/4, /*end_tick=*/19200);
    act.sp_end_steps = {{act_tick, act_tick + 8 * 1920, SpEndKind::Activation},
                        {phrase_tick, deact, SpEndKind::Collected}};
    REQUIRE(act.backends.empty());
    path.activations = {act};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(c.cap == 4);

    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].has_sp_end);
    CHECK(scene.activations[0].sp_end_tick == deact);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(24000.0));

    // Four bars at the activation, then eight measures of drain to burn them.
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(4.0));
    // Just before the phrase's last note: 3.9375 measures gone of eight.
    CHECK(sp_meter_bars_at(c, 11875.0 - 1e-6) == doctest::Approx(2.03125));
    // The collection hands back a whole bar on the spot.
    CHECK(sp_meter_bars_at(c, 11875.0) == doctest::Approx(3.03125));
    // The remaining 6.0625 bars' worth of measures runs out exactly at D.
    CHECK(sp_meter_bars_at(c, 24000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 24000.0) == doctest::Approx(0.0));
    // Still 0 after D. The phrase was collected during SP, not squeezed out,
    // so there is no bar waiting at the window's close.
    CHECK(sp_meter_bars_at(c, 26000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: two clamped collections refill twice and empty at the deact node") {
    // Cap 2. Two phrases bank 2 bars; the activation at tick 2304 (6000 ms)
    // spends them. The phrases at 3072 (8000 ms) and 3840 (10000 ms) are both
    // collected during SP, and each one clamps at the cap. The engine puts
    // the deact node at 6912 (18000 ms). The old gauge counted the
    // collections off the deact node: (9 - 3) / 2 - 2 = 1, so it drew one
    // refill and then showed a bar after SP ended that was never banked.
    Song song = make_overfill_song();
    ScoreGraph graph(song, 2);
    std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    REQUIRE(act.timecode.ticks() == 2304);
    REQUIRE(activation_deact_tick(act) == std::optional<int64_t>(6912));
    // Extra parentheses: the braced list's comma would split the macro.
    REQUIRE((act.collected_phrase_ticks() == std::vector<int64_t>{3072, 3840}));

    PreviewScene scene = build_preview_scene(song, &paths.front(), /*sp_cap=*/2);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(c.cap == 2);

    // The window starts at the 2 bars the record's first SP end leaves.
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(2.0));
    // One measure of drain, then the first collection tops back up to the cap.
    CHECK(sp_meter_bars_at(c, 8000.0 - 1e-6) == doctest::Approx(1.5));
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(2.0));
    // Another measure of drain, then the second collection does the same.
    CHECK(sp_meter_bars_at(c, 10000.0 - 1e-6) == doctest::Approx(1.5));
    CHECK(sp_meter_bars_at(c, 10000.0) == doctest::Approx(2.0));
    // Four measures from 10000 ms burn the 2 bars exactly at the deact node.
    CHECK(sp_meter_bars_at(c, 14000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 18000.0) == doctest::Approx(0.0));
    // Both window phrases were collected, so nothing banks when SP ends.
    CHECK(sp_meter_bars_at(c, 19000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: a bar that arrives on the activation note is spent there, not counted twice") {
    // The second phrase's last note sits exactly on the activation tick, and
    // the record says its bar arrived there. That arrival and the spend are
    // at the same instant: the window starts at the record's 2 bars, not 3,
    // and the bar is not left in the bank once the window closes.
    Song song = make_sp_song({960, 3840}, /*last_tick=*/13440);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.bank_rise_ticks = {960, 3840};  // the second bar arrives on the activation note
    path.activations = {act};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    // Two bars is four measures: 8000 ms at 120 BPM 4/4, ending at tick 11520.
    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].has_sp_end);
    CHECK(scene.activations[0].sp_end_tick == 11520);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(12000.0));

    CHECK(sp_meter_bars_at(c, 1000.0) == doctest::Approx(1.0));         // the first bar
    CHECK(sp_meter_bars_at(c, 4000.0 - 1e-6) == doctest::Approx(1.0));  // the second not yet in
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));         // 2, not 3
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(1.0));   // one bar's drain later
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.0));  // the deact node
    CHECK(sp_meter_bars_at(c, 14000.0) == doctest::Approx(0.0));  // the bar was spent, not banked
}

TEST_CASE("sp meter curve: the drain is linear in measures across a tempo change") {
    // 120 BPM until tick 5760 (6000 ms), then 60 BPM. A measure costs 2000 ms
    // before the change and 4000 ms after, so the ms slope halves there.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {{5760, 60.0}});
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    REQUIRE(scene.tempos.size() == 2);
    CHECK(scene.tempos[1].tick == 5760);
    CHECK(scene.tempos[1].ms == doctest::Approx(6000.0));
    // Four measures of SP still, but they now stretch to tick 11520 = 18000 ms.
    CHECK(scene.activations[0].sp_end_tick == 11520);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(18000.0));

    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 5000.0) == doctest::Approx(1.75));  // 0.25 bars per second
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(1.5));   // the tempo change
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.375)); // 0.125 bars per second
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.75));
    CHECK(sp_meter_bars_at(c, 18000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: an activation the record stamped nothing on draws nothing") {
    // A hand-built activation with a timecode and nothing else: no bar
    // arrivals, no SP end steps. The gauge reads the record and counts no
    // phrase, so there is nothing to draw. A fresh record always stamps both
    // (the search tests prove it).
    Song song = make_sp_song({960, 1920, 5760}, /*last_tick=*/9600);
    Path path;
    path.activations = {act_at(song, 3840, /*skipped_fills=*/{})};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    REQUIRE(scene.activations.size() == 1);
    CHECK_FALSE(scene.activations[0].has_sp_end);

    CHECK(sp_meter_bars_at(c, 3999.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 5999.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: the bank stops at the cap") {
    // Six phrases, at 1000 ms through 6000 ms.
    Song song = make_sp_song({960, 1920, 2880, 3840, 4800, 5760}, /*last_tick=*/7680);

    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.sp_meter.cap == 4);
    CHECK(sp_meter_bars_at(scene.sp_meter, 3000.0) == doctest::Approx(3.0));
    CHECK(sp_meter_bars_at(scene.sp_meter, 4000.0) == doctest::Approx(4.0));
    CHECK(sp_meter_bars_at(scene.sp_meter, 5000.0) == doctest::Approx(4.0));
    CHECK(sp_meter_bars_at(scene.sp_meter, 8000.0) == doctest::Approx(4.0));

    // The same chart on a record analyzed at a cap of 2.
    PreviewScene capped = build_preview_scene(song, nullptr, /*sp_cap=*/2);
    CHECK(capped.sp_meter.cap == 2);
    check_curve_well_formed(capped.sp_meter);
    CHECK(sp_meter_bars_at(capped.sp_meter, 1000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(capped.sp_meter, 2000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(capped.sp_meter, 6000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(capped.sp_meter, 8000.0) == doctest::Approx(2.0));
}

TEST_CASE("sp meter curve: without a path the meter fills and never drains") {
    Song song = make_sp_song({960, 1920, 2880, 3840, 4800}, /*last_tick=*/7680);
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.activations.empty());
    check_curve_well_formed(scene.sp_meter);

    // Never decreasing anywhere: nothing spends the bank.
    for (const SpMeterSegment& s : scene.sp_meter.segments) CHECK(s.end_bars >= s.start_bars);
    double prev = 0.0;
    for (double ms = 0.0; ms <= 9000.0; ms += 250.0) {
        const double v = sp_meter_bars_at(scene.sp_meter, ms);
        CHECK(v >= prev - 1e-9);
        prev = v;
    }
    CHECK(sp_meter_bars_at(scene.sp_meter, 9000.0) == doctest::Approx(4.0));
}

TEST_CASE("sp meter curve: a chart with no SP and no path has no curve at all") {
    Song song = make_fill_song();  // fills but no SP phrases
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.sp_meter.segments.empty());
    CHECK(sp_meter_bars_at(scene.sp_meter, 1000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: a late squeeze-in refills at the old SP end") {
    // Finding 5. The player hits the late phrase early, so SP never stops:
    // the engine extends one bar from the old end (7000 ms). At the phrase
    // (7250 ms) a quarter measure of that bar is gone: 0.875, not 1.0.
    Song song = test::make_late_sqin_song();
    ScoreGraph graph(song, 4);
    std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    const Path& best = paths.front();
    REQUIRE(best.activations.size() == 1);
    REQUIRE(best.activations.front().deact_tick() == std::optional<int64_t>(17280));

    PreviewScene scene = build_preview_scene(song, &best);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(sp_meter_bars_at(c, 3000.0) == doctest::Approx(2.0));   // the activation
    CHECK(sp_meter_bars_at(c, 5000.0) == doctest::Approx(1.0));   // a bar per 2000 ms
    CHECK(sp_meter_bars_at(c, 7000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // the refill, at the old end
    CHECK(sp_meter_bars_at(c, 7250.0) == doctest::Approx(0.875)); // the late phrase
    CHECK(sp_meter_readout(c, 7250.0) == "0.9/4");
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 9000.0) == doctest::Approx(0.0));   // the deact node
    CHECK(sp_meter_bars_at(c, 9500.0) == doctest::Approx(0.0));   // collected: nothing banks after
}

TEST_CASE("sp meter curve: a squeezed-out bar arrives at the deact node") {
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    // The best path collects 12960 instead and keeps SP running over the
    // second fill, so search wide and pick the path that squeezed out.
    EngineOptions opts = test::wide_search();
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    std::vector<Path> paths = run_search(graph, opts);
    const Path* found = nullptr;
    for (const Path& p : paths)
        if (!p.activations.empty() && p.activations.front().sqout_tick) found = &p;
    REQUIRE(found != nullptr);
    REQUIRE(found->activations.size() == 2);
    PreviewScene scene = build_preview_scene(song, found);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    // Straight through the squeezed-out phrase (6750 ms): no step there.
    CHECK(sp_meter_bars_at(c, 6750.0 - 1e-6) == doctest::Approx(0.125));
    CHECK(sp_meter_bars_at(c, 6750.0) == doctest::Approx(0.125));
    CHECK(sp_meter_bars_at(c, 7000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // its bar, as SP ends
    CHECK(sp_meter_bars_at(c, 7500.0) == doctest::Approx(2.0));   // the next phrase
    CHECK(sp_meter_bars_at(c, 9000.0) == doctest::Approx(2.0));   // the second activation
    CHECK(sp_meter_bars_at(c, 11000.0) == doctest::Approx(1.0));
}

TEST_CASE("sp meter curve: the drain follows the stored steps, not the collected list") {
    // The record moves the end at 7680 (8000 ms) from 11520 to 15360, and no
    // phrase ends there. The gauge still steps there.
    Song song = make_sp_song({960}, /*last_tick=*/17280);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.bank_rise_ticks = {960, 1920};
    act.sp_end_steps = {{3840, 11520, SpEndKind::Activation},
                        {7680, 15360, SpEndKind::Collected}};  // no phrase ends at 7680
    path.activations = {act};
    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(sp_meter_bars_at(c, 8000.0 - 1e-6) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 16000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: the path gauge reads stored facts only") {
    // Findings 147 and 159, held in place: the path gauge's own body never
    // touches the chart's phrases, the collected list, the bank count, the
    // cap rule or the squeeze-in rule (that one lives in refill_tick). A copy
    // of any of them coming back fails here.
    std::ifstream in(std::string(HYDRA_SOURCE_DIR) + "/src/app/preview_view.cpp");
    REQUIRE(in);
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string src = ss.str();
    const size_t begin = src.find("SpMeterCurve build_sp_meter_curve(");
    REQUIRE(begin != std::string::npos);
    const size_t end = src.find("\n}\n", begin);
    REQUIRE(end != std::string::npos);
    const std::string body = src.substr(begin, end - begin);
    for (const char* banned : {"sp_phrases", "collected_phrase_ticks", "sp_meter()",
                               "sp_bars_to_measures", "std::min(", "SqIn"}) {
        INFO(std::string(banned));
        CHECK(body.find(banned) == std::string::npos);
    }
}

TEST_CASE("sp_meter_bars_at: before the curve, after it, and on a shared boundary") {
    CHECK(sp_meter_bars_at(SpMeterCurve{}, 123.0) == doctest::Approx(0.0));

    SpMeterCurve c;
    c.segments = {{100.0, 200.0, 0.0, 0.0}, {200.0, 400.0, 1.0, 0.0}};

    CHECK(sp_meter_bars_at(c, 50.0) == doctest::Approx(0.0));   // before the first
    CHECK(sp_meter_bars_at(c, 100.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 150.0) == doctest::Approx(0.0));
    // 200 ms is the boundary: the later segment's 1.0, not the earlier's 0.0.
    CHECK(sp_meter_bars_at(c, 200.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 300.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 400.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 99999.0) == doctest::Approx(0.0));  // holds the last value

    // A zero-width final segment is how a curve carries its end value when the
    // last step lands exactly at the end of the chart.
    SpMeterCurve z;
    z.segments = {{0.0, 100.0, 0.0, 1.0}, {100.0, 100.0, 3.0, 3.0}};
    CHECK(sp_meter_bars_at(z, 50.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(z, 100.0) == doctest::Approx(3.0));
    CHECK(sp_meter_bars_at(z, 500.0) == doctest::Approx(3.0));
}

TEST_CASE("path_overlay_key: no overlay, the same path, and a changed path") {
    // The key is how the Preview notices the Paths tab picked a different
    // path; Path itself has no operator==.
    CHECK(path_overlay_key(nullptr).empty());

    const AnalysisResult& r = analyzed();
    std::vector<const Path*> paths = r.record.all_paths();
    REQUIRE_FALSE(paths.empty());

    const Path& first = *paths.front();
    Path copy = first;
    CHECK_FALSE(path_overlay_key(&first).empty());
    CHECK(path_overlay_key(&first) == path_overlay_key(&copy));

    // Same activations, a different score: still a different overlay.
    Path rescored = first;
    rescored.score_base += 1;
    CHECK(path_overlay_key(&first) != path_overlay_key(&rescored));

    // Different activations: different key.
    Path trimmed = first;
    if (!trimmed.activations.empty()) {
        trimmed.activations.pop_back();
        CHECK(path_overlay_key(&first) != path_overlay_key(&trimmed));
    }
}

TEST_CASE("step_tick_ms: one tick from the tick the time box shows") {
    // 120 BPM, 480 ticks per quarter: a tick is 500/480 ms.
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    const MsIndex& ms = song.timing().ms_index();

    // 1300 ms is the time box's own example: tick 1248, "m1.3.288".
    const double fwd = step_tick_ms(scene, 1300.0, 5000.0, 1);
    const double back = step_tick_ms(scene, 1300.0, 5000.0, -1);
    CHECK(fwd == doctest::Approx(ms.at(1249)));
    CHECK(back == doctest::Approx(ms.at(1247)));
    CHECK(build_time_box(scene, fwd, 5000.0).position == "m1.3.289");
    CHECK(build_time_box(scene, back, 5000.0).position == "m1.3.287");

    // Between ticks it steps from the rounded tick: 1300.6 ms rounds to 1249.
    CHECK(step_tick_ms(scene, 1300.6, 5000.0, 1) == doctest::Approx(ms.at(1250)));

    // Never before tick 0.
    CHECK(step_tick_ms(scene, 0.0, 5000.0, -1) == doctest::Approx(0.0));
    CHECK(step_tick_ms(scene, 0.5, 5000.0, -3) == doctest::Approx(0.0));

    // Past the end the time box shows the end (5000 ms, tick 4800), and the
    // step starts there too.
    CHECK(build_time_box(scene, 6000.0, 5000.0).position == "m3.3.0");
    CHECK(step_tick_ms(scene, 6000.0, 5000.0, -1) == doctest::Approx(ms.at(4799)));
}

TEST_CASE("step_tick_ms: a tempo change moves the tick length with it") {
    // 120 BPM until tick 1920 (2000 ms), then 240 BPM: a tick shrinks from
    // 500/480 ms to 250/480 ms.
    Song song = make_sp_song({}, 3840, {{1920, 240.0}});
    PreviewScene scene = build_preview_scene(song, nullptr);
    const MsIndex& ms = song.timing().ms_index();
    REQUIRE(ms.at(1920) == doctest::Approx(2000.0));

    CHECK(step_tick_ms(scene, 2000.0, 5000.0, 1) == doctest::Approx(ms.at(1921)));
    CHECK(step_tick_ms(scene, 2000.0, 5000.0, -1) == doctest::Approx(ms.at(1919)));
    CHECK(step_tick_ms(scene, 2000.0, 5000.0, 1) - 2000.0 == doctest::Approx(250.0 / 480.0));
    CHECK(2000.0 - step_tick_ms(scene, 2000.0, 5000.0, -1) == doctest::Approx(500.0 / 480.0));
}

TEST_CASE("step_tick_ms: a scene with no song leaves the time alone") {
    PreviewScene empty;
    CHECK(step_tick_ms(empty, 1234.5, 5000.0, 1) == doctest::Approx(1234.5));
}

TEST_CASE("score box: no path hides the box") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.score.state == PreviewScore::State::None);
    CHECK(scene.score.steps.empty());
    CHECK_FALSE(build_score_box(scene, 600.0).shown);
}

TEST_CASE("score box: a solo's bonus lands on its last note") {
    // Hand song: chords at 0, 250, 500 and 750 ms. The solo is the chords at
    // 250 and 500 ms, so its bonus is withheld at 250 and paid at 500.
    Song song = make_hand_song();
    Path path = priced_path(song, {});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.score.steps.size() == 4);

    const ReplayResult r = replay_path(song, {});
    REQUIRE(r.chords[1].points.solo > 0);
    CHECK(scene.score.steps[1].total == r.chords[1].cum.total() - r.chords[1].points.solo);
    CHECK(scene.score.steps[2].total == r.chords[2].cum.total());
    CHECK(scene.score.steps[3].total == path.totalscore());

    // Between the solo's two chords the box shows the withheld total.
    PreviewScoreBox mid = build_score_box(scene, 400.0);
    CHECK(mid.shown);
    CHECK(mid.available);
    CHECK(mid.score == group_thousands(scene.score.steps[1].total));
    CHECK(mid.detail == "x1 " + kDot + " combo 2");

    // A chord exactly at the playhead counts as hit.
    PreviewScoreBox on = build_score_box(scene, scene.score.steps[2].ms);
    CHECK(on.score == group_thousands(scene.score.steps[2].total));
    CHECK(on.detail == "x1 " + kDot + " combo 3");
}

TEST_CASE("score box: before the first note nothing is hit yet") {
    // The fill song's first note is at tick 480, 500 ms.
    Song song = make_fill_song();
    Path path = priced_path(song, {});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    PreviewScoreBox box = build_score_box(scene, 100.0);
    CHECK(box.shown);
    CHECK(box.available);
    CHECK(box.score == "0");
    CHECK(box.detail == "x1 " + kDot + " combo 0");
}

TEST_CASE("score box: the multiplier is the replay's, doubled on chords Star Power pays") {
    // A Red note every 500 ms (tick 480 steps). One bar of SP activated at
    // tick 2400 (2500 ms) runs two measures, to tick 6240 (6500 ms).
    Song song = make_sp_song({1920}, 9600);
    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.activations.size() == 1);
    REQUIRE(scene.activations[0].has_sp_end);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(6500.0));

    // Each step carries exactly what the replay says the disc shows.
    const ReplayResult r = replay_stored_path(song, path).result;
    REQUIRE(r.chords.size() == scene.score.steps.size());
    for (size_t i = 0; i < r.chords.size(); ++i) {
        CHECK(scene.score.steps[i].multiplier == r.chords[i].multiplier_shown);
        // And the plain disc, which the box shows once SP has ended.
        CHECK(scene.score.steps[i].multiplier_plain == r.chords[i].multiplier_after);
    }

    // The SP end itself: the chord on it (6500 ms) was paid doubled, but from
    // this instant on the box is plain. A hair earlier it is still doubled.
    CHECK(build_score_box(scene, 6499.0).detail == "x4 " + kDot + " combo 13");
    CHECK(build_score_box(scene, 6500.0).detail == "x2 " + kDot + " combo 14");
    CHECK(build_score_box(scene, scene.activations[0].sp_end_ms).detail ==
          "x2 " + kDot + " combo 14");

    // 2000 ms: five notes hit, before the activation.
    CHECK(build_score_box(scene, 2000.0).detail == "x1 " + kDot + " combo 5");
    // 2500 ms: the activation chord is hit and paid, so x1 shows as x2.
    CHECK(build_score_box(scene, 2500.0).detail == "x2 " + kDot + " combo 6");
    CHECK(build_score_box(scene, 3000.0).detail == "x2 " + kDot + " combo 7");
    // 6600 ms: Star Power has ended, so the disc is plain even though the
    // last chord hit (6500 ms, on the deactivation node) was paid doubled.
    CHECK(build_score_box(scene, 6600.0).detail == "x2 " + kDot + " combo 14");
    // 7000 ms: the first chord Star Power doesn't pay: the plain x2 of combo 15.
    CHECK(build_score_box(scene, 7000.0).detail == "x2 " + kDot + " combo 15");
}

// User decision D11 (docs/audit/2026-10-03-fix-decisions.md): a chord just past
// the SP end that Hydra's leeway still pays doubled is paid doubled in the
// score, but the box reads plain from the SP end on.
TEST_CASE("score box: a leeway chord past the SP end is paid doubled, the box stays plain") {
    // Same song as above, plus one extra chord 1 tick (about 1 ms) after the
    // SP end at tick 6240, inside the 3 ms leeway.
    Song song = make_sp_song({1920}, 9600);
    const int64_t leeway_tick = 6241;
    const auto at = std::find_if(song.sequence.begin(), song.sequence.end(),
                                 [&](const SongTimestamp& ts) {
                                     return ts.timecode.ticks() > 6240;
                                 });
    REQUIRE(at != song.sequence.end());
    SongTimestamp extra;
    extra.timecode = song.timecode(leeway_tick);
    extra.chord.add_note(NoteColor::Red);
    song.sequence.insert(at, std::move(extra));

    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.activations.size() == 1);
    const double sp_end = scene.activations[0].sp_end_ms;
    CHECK(sp_end == doctest::Approx(6500.0));

    const ReplayResult r = replay_stored_path(song, path).result;
    REQUIRE(r.chords.size() == scene.score.steps.size());
    size_t idx = r.chords.size();
    for (size_t i = 0; i < r.chords.size(); ++i)
        if (r.chords[i].tick == leeway_tick) idx = i;
    REQUIRE(idx < r.chords.size());
    const ReplayChord& leeway = r.chords[idx];

    // The replay paid it doubled, a hair after the SP end.
    CHECK(leeway.ms > sp_end);
    CHECK(leeway.ms - sp_end < core::default_rules().backend_leeway_ms);
    CHECK(leeway.in_sp);
    CHECK(leeway.multiplier_shown == leeway.multiplier_after * kStarPowerMultiplier);
    CHECK(scene.score.steps[idx].multiplier == leeway.multiplier_shown);
    CHECK(scene.score.steps[idx].multiplier_plain == leeway.multiplier_after);

    // The box, at its own instant, reads plain.
    const PreviewScoreBox box = build_score_box(scene, leeway.ms);
    CHECK(box.detail == "x" + std::to_string(leeway.multiplier_after) + " " + kDot +
                            " combo " + std::to_string(leeway.combo_after));
}

TEST_CASE("score box: a path the replay can't reproduce says so") {
    Song song = make_hand_song();

    // The stored score disagrees with the replay by one point.
    Path wrong = priced_path(song, {});
    wrong.score_base += 1;
    PreviewScene a = build_preview_scene(song, &wrong);
    CHECK(a.score.state == PreviewScore::State::Unavailable);
    PreviewScoreBox box = build_score_box(a, 600.0);
    CHECK(box.shown);
    CHECK_FALSE(box.available);
    CHECK(box.score == "Score unavailable");
    CHECK(box.detail.empty());

    // An activation with no deactivation node (a record from before blob v4)
    // yields no window, so the replay can't stand for the path.
    Path old;
    old.activations.push_back(act_at(song, 720, {}));
    PreviewScene b = build_preview_scene(song, &old);
    CHECK(b.score.state == PreviewScore::State::Unavailable);
    CHECK(build_score_box(b, 600.0).score == "Score unavailable");
}

TEST_CASE("score box: the analyzed chart ends on the path's total") {
    const AnalysisResult& r = analyzed();
    const Path& best = r.record.best_path();
    PreviewScene scene = build_preview_scene(r.song, &best);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.score.steps.size() == r.song.sequence.size());
    CHECK(scene.score.steps.back().total == best.totalscore());
    for (size_t i = 1; i < scene.score.steps.size(); ++i) {
        CHECK(scene.score.steps[i].ms >= scene.score.steps[i - 1].ms);
        CHECK(scene.score.steps[i].combo > scene.score.steps[i - 1].combo);
    }
    PreviewScoreBox end = build_score_box(scene, scene.song_length_ms);
    CHECK(end.score == group_thousands(best.totalscore()));
}

// ---- SP drain box -------------------------------------------------------

TEST_CASE("drain box: hidden without an SP gauge") {
    CHECK_FALSE(build_drain_box(PreviewScene{}, 1000.0).shown);
    // Notes but no SP phrase and no path: no gauge, so no box.
    Song song = make_sp_song({}, /*last_tick=*/3840);
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.sp_meter.segments.empty());
    CHECK_FALSE(build_drain_box(scene, 1000.0).shown);
}

TEST_CASE("drain box: idle at a steady tempo reads the rate and a full meter") {
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    PreviewScene scene = build_preview_scene(song, nullptr);

    PreviewDrainBox box = build_drain_box(scene, 2000.0);
    CHECK(box.shown);
    CHECK_FALSE(box.active);
    CHECK(box.header == "SP drain (if activated)");
    CHECK(box.rate == "1 bar / 4.0 s");
    // Cap 4: eight measures, 16000 ms.
    CHECK(box.detail == "full meter 16.0 s");
}

TEST_CASE("drain box: the rate switches exactly at a tempo change") {
    // 120 BPM until tick 5760 (6000 ms), then 60 BPM: a bar goes 4 s -> 8 s.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {{5760, 60.0}});
    PreviewScene scene = build_preview_scene(song, nullptr);

    CHECK(build_drain_box(scene, 5990.0).rate == "1 bar / 4.0 s");
    CHECK(build_drain_box(scene, 6000.0).rate == "1 bar / 8.0 s");

    // "full meter" is the cap at the bar time in force, so it jumps with the
    // rate, at the change and not before: 4 x 4 s, then 4 x 8 s.
    CHECK(build_drain_box(scene, 2000.0).detail == "full meter 16.0 s");
    CHECK(build_drain_box(scene, 5990.0).detail == "full meter 16.0 s");
    CHECK(build_drain_box(scene, 6000.0).detail == "full meter 32.0 s");
}

TEST_CASE("drain box: a 7/8 section drains faster at the same BPM") {
    // 7/8 from tick 3840 (4000 ms, a barline): 1680 ticks = 1750 ms a
    // measure, so a bar lasts 3500 ms instead of 4000.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {}, 480, {{3840, 1680}});
    PreviewScene scene = build_preview_scene(song, nullptr);

    CHECK(build_drain_box(scene, 2000.0).rate == "1 bar / 4.0 s");
    CHECK(build_drain_box(scene, 4000.0).rate == "1 bar / 3.5 s");
    CHECK(build_drain_box(scene, 4000.0).detail == "full meter 14.0 s");
}

TEST_CASE("drain box: active inside the stored SP window, idle outside it") {
    // Two bars at tick 3840 (4000 ms): the stored end is tick 11520 (12000 ms).
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};
    PreviewScene scene = build_preview_scene(song, &path);

    PreviewDrainBox before = build_drain_box(scene, 3999.0);
    CHECK_FALSE(before.active);
    CHECK(before.header == "SP drain (if activated)");

    PreviewDrainBox at = build_drain_box(scene, 4000.0);
    CHECK(at.active);
    CHECK(at.header == "SP drain");
    CHECK(at.rate == "1 bar / 4.0 s");
    CHECK(at.detail == "empties in 8.0 s");
    CHECK(build_drain_box(scene, 9000.0).detail == "empties in 3.0 s");

    // At the stored end SP is over, as the gauge reads that boundary too.
    PreviewDrainBox after = build_drain_box(scene, 12000.0);
    CHECK_FALSE(after.active);
    CHECK(after.detail.rfind("full meter ", 0) == 0);
}

TEST_CASE("drain box: empties in reads the stored end, not a recount") {
    // A phrase collected mid-SP pushes the stored end two measures past the
    // plain act + 4 measures: 16000 ms, not 12000. Only the record knows.
    Song song = make_sp_song({960, 5760}, /*last_tick=*/17280);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.sp_end_steps = {{3840, 11520, SpEndKind::Activation}, {5760, 15360, SpEndKind::Collected}};
    path.activations = {act};
    PreviewScene scene = build_preview_scene(song, &path);

    PreviewDrainBox box = build_drain_box(scene, 12000.0);
    CHECK(box.active);
    CHECK(box.detail == "empties in 4.0 s");
}

TEST_CASE("drain box: an activation with no stored end stays idle") {
    // A pre-v4 record: no deact node, so nothing says SP is running.
    Song song = make_sp_song({960, 1920}, /*last_tick=*/9600);
    Path path;
    path.activations = {act_at(song, 3840, /*skipped_fills=*/{})};
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE_FALSE(scene.activations[0].has_sp_end);

    PreviewDrainBox box = build_drain_box(scene, 5000.0);
    CHECK(box.shown);
    CHECK_FALSE(box.active);
    CHECK(box.detail.rfind("full meter ", 0) == 0);
}

namespace {

// Two one-bar activations on make_sp_song's timing (120 BPM, 4/4, 480 ticks a
// beat): tick 1920 is 2000 ms, measure 2; tick 7680 is 8000 ms, measure 5.
// One phrase ends at tick 960 (1000 ms) to fill the first bar. The notes run
// to tick 13440 so the second activation's deact node (tick 11520) is inside.
struct TwoActs {
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    PreviewScene scene;
    TwoActs() {
        Activation a = sp_act_at(song, 1920, /*sp_meter=*/1, /*end_tick=*/5760);
        a.bank_rise_ticks = {960};  // the phrase's bar, as the engine stamps it
        a.chord.add_note(NoteColor::Red);
        Activation b = sp_act_at(song, 7680, /*sp_meter=*/1, /*end_tick=*/11520);
        b.chord.add_note(NoteColor::Red);
        path.activations = {a, b};
        scene = build_preview_scene(song, &path);
    }
};

}  // namespace

TEST_CASE("scrub marks: each activation's onset over the scrubber's length") {
    TwoActs t;
    std::vector<double> marks = build_scrub_marks(t.scene, 10000.0);
    REQUIRE(marks.size() == 2);
    CHECK(marks[0] == doctest::Approx(0.2));
    CHECK(marks[1] == doctest::Approx(0.8));
    // Clamped into the bar when the length is shorter than the path.
    CHECK(build_scrub_marks(t.scene, 4000.0)[1] == doctest::Approx(1.0));
    CHECK(build_scrub_marks(t.scene, 0.0).empty());
    CHECK(build_scrub_marks(build_preview_scene(t.song, nullptr), 10000.0).empty());
}

TEST_CASE("activation jumps: nearest activation before or after the playhead") {
    TwoActs t;
    CHECK(activation_jump_ms(t.scene, 0.0, +1) == doctest::Approx(2000.0));
    // Parked on an activation, "next" moves on and "previous" goes back past it.
    CHECK(activation_jump_ms(t.scene, 2000.0, +1) == doctest::Approx(8000.0));
    CHECK(activation_jump_ms(t.scene, 2000.3, +1) == doctest::Approx(8000.0));
    CHECK_FALSE(activation_jump_ms(t.scene, 2000.0, -1).has_value());
    CHECK(activation_jump_ms(t.scene, 5000.0, -1) == doctest::Approx(2000.0));
    CHECK(activation_jump_ms(t.scene, 8000.0, -1) == doctest::Approx(2000.0));
    CHECK_FALSE(activation_jump_ms(t.scene, 8000.0, +1).has_value());
    CHECK_FALSE(activation_jump_ms(build_preview_scene(t.song, nullptr), 0.0, +1).has_value());
}

TEST_CASE("next activation box: the activation at or after the playhead") {
    TwoActs t;
    PreviewNextActBox box = build_next_act_box(t.scene, 0.0);
    CHECK(box.shown);
    CHECK(box.header == "Next: activation 1 of 2");
    CHECK(box.detail == "at m2.1.0 " + kDot + " [Red]");
    CHECK(build_next_act_box(t.scene, 2000.0).header == "Next: activation 1 of 2");
    CHECK(build_next_act_box(t.scene, 2001.0).header == "Next: activation 2 of 2");
    CHECK(build_next_act_box(t.scene, 2001.0).detail == "at m5.1.0 " + kDot + " [Red]");
    CHECK_FALSE(build_next_act_box(t.scene, 9000.0).shown);
    CHECK_FALSE(build_next_act_box(build_preview_scene(t.song, nullptr), 0.0).shown);
}

// ---- Old scan versus new search ------------------------------------------
//
// The time box, the next-activation box, the fill states and the SP meter
// used to scan their lists from the front (and the fills and the meter did so
// once per activation). They now search or walk with a moving index. The old
// scans are copied here verbatim as the reference, and every result must
// match them exactly: on every corpus chart with each of its paths, and on a
// busy synthetic chart with random tempos, meters, sections and activations.

namespace {

namespace old_scan {

constexpr double kOnActivationMs = 0.5;

std::string clock_str(double ms) {
    double secs = ms / 1000.0;
    int minutes = static_cast<int>(secs / 60.0);
    double rem = secs - minutes * 60.0;
    char buf[48];
    std::snprintf(buf, sizeof buf, "%d:%06.3f", minutes, rem);
    return buf;
}

int64_t tick_at(const SongTiming& timing, double ms) {
    const int64_t tick = std::llround(timing.ms_index().tick_at_ms(ms));
    return tick < 0 ? 0 : tick;
}

double shown_length(double length_ms) { return length_ms < 0.0 ? 0.0 : length_ms; }

double shown_ms(double now_ms, double length_ms) {
    const double len = shown_length(length_ms);
    return now_ms < 0.0 ? 0.0 : (now_ms > len ? len : now_ms);
}

PreviewTimeBox time_box(const PreviewScene& scene, double now_ms, double length_ms) {
    PreviewTimeBox box;
    const double len = shown_length(length_ms);
    const double now = shown_ms(now_ms, length_ms);
    box.timestamp = clock_str(now) + " / " + clock_str(len);
    const int64_t now_tick = scene.timing ? tick_at(*scene.timing, now) : 0;
    const int64_t end_tick = scene.timing ? tick_at(*scene.timing, len) : 0;
    box.position = scene.timing ? format_measure(*scene.timing, now_tick) : "m1.1.0";
    box.length = scene.timing ? format_measure(*scene.timing, end_tick) : "m1.1.0";
    double bpm = scene.tempos.empty() ? 0.0 : scene.tempos.front().bpm;
    for (const PreviewTempo& t : scene.tempos) {
        if (t.ms > now) break;
        bpm = t.bpm;
    }
    int ts_num = 4, ts_den = 4;
    for (const PreviewTimeSig& t : scene.time_sigs) {
        if (t.tick > now_tick) break;
        ts_num = t.numerator;
        ts_den = t.denominator;
    }
    char buf[64];
    std::snprintf(buf, sizeof buf, "BPM %.3f \xC2\xB7 %d/%d", bpm, ts_num, ts_den);
    box.tempo = buf;
    std::string section;
    for (const PreviewSection& s : scene.sections) {
        if (s.tick > now_tick) break;
        section = s.name;
    }
    if (!section.empty()) box.section_line = "Section " + section;
    return box;
}

PreviewNextActBox next_act_box(const PreviewScene& scene, double now_ms) {
    PreviewNextActBox box;
    const size_t count = scene.activations.size();
    for (size_t i = 0; i < count; ++i) {
        const PreviewActivation& a = scene.activations[i];
        if (a.ms < now_ms - kOnActivationMs) continue;
        box.shown = true;
        box.header = "Next: activation " + std::to_string(i + 1) + " of " + std::to_string(count);
        box.detail = "at " + a.measure;
        if (!a.chord.empty()) box.detail += " \xC2\xB7 " + a.chord;
        break;
    }
    return box;
}

// The fill states as the old nested scans set them, from scratch.
std::vector<PreviewFillState> fill_states(const PreviewScene& scene, bool has_path) {
    std::vector<PreviewFill> fills = scene.fills;
    for (PreviewFill& f : fills) f.state = PreviewFillState::Hidden;
    if (!has_path) {
        for (PreviewFill& f : fills) f.state = PreviewFillState::Offered;
    } else {
        int64_t prev_tick = std::numeric_limits<int64_t>::min();
        for (const PreviewActivation& a : scene.activations) {
            for (PreviewFill& f : fills)
                if (f.span.end_tick == a.tick) {
                    f.state = PreviewFillState::Taken;
                    break;
                }
            int left = static_cast<int>(a.skipped_fill_ticks.size());
            for (auto it = fills.rbegin(); it != fills.rend() && left > 0; ++it) {
                if (it->span.end_tick >= a.tick || it->span.end_tick <= prev_tick) continue;
                it->state = PreviewFillState::Offered;
                --left;
            }
            prev_tick = a.tick;
        }
    }
    std::vector<PreviewFillState> out;
    for (const PreviewFill& f : fills) out.push_back(f.state);
    return out;
}

struct DrainSplit {
    int64_t tick = 0;
    double ms = 0.0;
    bool collection = false;
};

void push_segment(SpMeterCurve& curve, double start_ms, double end_ms, double start_bars,
                  double end_bars) {
    if (end_ms <= start_ms) return;
    curve.segments.push_back({start_ms, end_ms, start_bars, end_bars});
}

// What the old scan read off each activation, which PreviewActivation no
// longer carries: the bars it spent and the phrases it collected. Taken from
// the Path itself (Activation::sp_meter() and collected_phrase_ticks()).
struct ActFacts {
    int sp_meter = 0;
    std::vector<int64_t> collected_phrase_ticks;
};

std::vector<ActFacts> act_facts(const Path* path) {
    std::vector<ActFacts> out;
    if (path == nullptr) return out;
    for (const Activation& a : path->walk_activations())
        out.push_back({a.sp_meter(), a.collected_phrase_ticks()});
    return out;
}

SpMeterCurve sp_meter_curve(const PreviewScene& scene, const SongTiming& timing, int sp_cap,
                            const std::vector<ActFacts>& facts) {
    SpMeterCurve curve;
    curve.cap = sp_cap < 1 ? 1 : sp_cap;
    if (scene.sp_phrases.empty() && scene.activations.empty()) return curve;
    const double cap = static_cast<double>(curve.cap);
    const std::vector<PreviewSpan>& phrases = scene.sp_phrases;
    size_t next_phrase = 0;
    double bank = 0.0;
    double cursor_ms = 0.0;
    auto run_flat_to = [&](double until_ms) {
        while (next_phrase < phrases.size() && phrases[next_phrase].end_ms < until_ms) {
            push_segment(curve, cursor_ms, phrases[next_phrase].end_ms, bank, bank);
            cursor_ms = phrases[next_phrase].end_ms;
            bank = std::min(bank + 1.0, cap);
            ++next_phrase;
        }
        push_segment(curve, cursor_ms, until_ms, bank, bank);
        if (until_ms > cursor_ms) cursor_ms = until_ms;
    };
    REQUIRE(facts.size() == scene.activations.size());
    for (size_t ai = 0; ai < scene.activations.size(); ++ai) {
        const PreviewActivation& act = scene.activations[ai];
        const ActFacts& f = facts[ai];
        run_flat_to(act.ms);
        bank = static_cast<double>(f.sp_meter);
        if (f.sp_meter <= 0 || !act.has_sp_end) {
            bank = 0.0;
            continue;
        }
        std::vector<const PreviewSpan*> window;
        for (size_t i = next_phrase; i < phrases.size(); ++i) {
            if (phrases[i].end_ms >= act.sp_end_ms) break;
            if (phrases[i].end_tick > act.tick) window.push_back(&phrases[i]);
            ++next_phrase;
        }
        std::vector<DrainSplit> splits;
        for (int64_t t : f.collected_phrase_ticks)
            if (t > act.tick && t < act.sp_end_tick)
                splits.push_back({t, timing.ms_index().at(t), true});
        int64_t squeezed_out = 0;
        for (const PreviewSpan* p : window)
            if (std::find(f.collected_phrase_ticks.begin(), f.collected_phrase_ticks.end(),
                          p->end_tick) == f.collected_phrase_ticks.end())
                ++squeezed_out;
        for (const PreviewTempo& t : scene.tempos)
            if (t.tick > act.tick && t.tick < act.sp_end_tick)
                splits.push_back({t.tick, t.ms, false});
        for (const PreviewMeter& m : scene.meters)
            if (m.tick > act.tick && m.tick < act.sp_end_tick)
                splits.push_back({m.tick, timing.ms_index().at(m.tick), false});
        std::stable_sort(splits.begin(), splits.end(),
                         [](const DrainSplit& a, const DrainSplit& b) { return a.tick < b.tick; });
        double remaining = static_cast<double>(sp_bars_to_measures(f.sp_meter));
        int64_t prev_tick = act.tick;
        double prev_ms = act.ms;
        double prev_measures = timing.measures_at_tick_f(static_cast<double>(prev_tick));
        for (const DrainSplit& s : splits) {
            const double measures = timing.measures_at_tick_f(static_cast<double>(s.tick));
            const double elapsed = measures - prev_measures;
            const double left = std::max(0.0, remaining - elapsed);
            push_segment(curve, prev_ms, s.ms, remaining / static_cast<double>(kMeasuresPerSpBar),
                         left / static_cast<double>(kMeasuresPerSpBar));
            remaining = left;
            if (s.collection)
                remaining = std::min(remaining + static_cast<double>(kMeasuresPerSpBar),
                                     static_cast<double>(sp_bars_to_measures(curve.cap)));
            prev_tick = s.tick;
            prev_ms = s.ms;
            prev_measures = measures;
        }
        push_segment(curve, prev_ms, act.sp_end_ms,
                     remaining / static_cast<double>(kMeasuresPerSpBar), 0.0);
        bank = std::min(static_cast<double>(squeezed_out), cap);
        cursor_ms = std::max(cursor_ms, act.sp_end_ms);
    }
    while (next_phrase < phrases.size()) {
        push_segment(curve, cursor_ms, phrases[next_phrase].end_ms, bank, bank);
        cursor_ms = std::max(cursor_ms, phrases[next_phrase].end_ms);
        bank = std::min(bank + 1.0, cap);
        ++next_phrase;
    }
    const double end_ms = std::max(cursor_ms, scene.song_length_ms);
    curve.segments.push_back({cursor_ms, end_ms, bank, bank});
    return curve;
}

}  // namespace old_scan

// One place the record-read gauge and the old scan disagree.
struct GaugeDiff {
    double ms = 0.0;
    double got = 0.0;   // the gauge
    double want = 0.0;  // the old scan
};

// The gauge against the old scan, by value. The two cut the curve at
// different places, so segments are not compared one by one. Every segment
// boundary of either curve (and a hair either side) and `random_times` random
// times are probed. A late squeeze-in window is left out: the old scan drew it
// wrong (finding 5). A window is one when some step's refill tick sits before
// the step's own note, read off the Path's own activations.
//
// Between windows the two must agree exactly. Inside a window they may differ
// by one tick per stored SP end step, and no more. The engine stores each SP
// end as a whole tick, truncating the fraction of a measure
// (SongTiming::plusmeasure). The old scan started at exactly the banked bars
// and bent its last stretch to reach empty at the stored end. The new gauge
// drains at exactly one bar per two measures to that stored end, so it starts
// up to one tick's worth lower. One tick is 1 / (2 * ticks per measure) bars.
std::vector<GaugeDiff> gauge_differences(const PreviewScene& scene, const Path* path,
                                         int sp_cap, std::mt19937& rng,
                                         int random_times = 300) {
    const SpMeterCurve want_curve = old_scan::sp_meter_curve(scene, *scene.timing, sp_cap,
                                                             old_scan::act_facts(path));
    CHECK(scene.sp_meter.cap == want_curve.cap);
    int64_t min_tpm = std::numeric_limits<int64_t>::max();
    for (const PreviewMeter& m : scene.meters) min_tpm = std::min(min_tpm, m.tpm);
    const double one_tick_bars =
        min_tpm > 0 && min_tpm != std::numeric_limits<int64_t>::max()
            ? 1.0 / (static_cast<double>(kMeasuresPerSpBar) * static_cast<double>(min_tpm))
            : 0.0;
    struct Window {
        double from, to;
        double slack;  // bars
    };
    std::vector<std::pair<double, double>> late_windows;
    std::vector<Window> windows;
    if (path != nullptr) {
        size_t i = 0;
        for (const Activation& a : path->walk_activations()) {
            const PreviewActivation& pa = scene.activations.at(i++);
            if (pa.has_sp_end)
                windows.push_back({pa.ms, pa.sp_end_ms,
                                   one_tick_bars * static_cast<double>(a.sp_end_steps.size())});
            for (size_t k = 0; k < a.sp_end_steps.size(); ++k)
                if (a.refill_tick(k) < a.sp_end_steps[k].tick) {
                    late_windows.push_back({pa.ms, pa.sp_end_ms});
                    break;
                }
        }
    }
    auto in_late_sqin_window = [&](double ms) {
        for (const auto& [from, to] : late_windows)
            if (from <= ms && ms <= to) return true;
        return false;
    };
    // The slack at `ms`: inside a window (activation up to, not including,
    // its deact node) one tick per step; anywhere else none.
    auto slack_at = [&](double ms) {
        for (const Window& w : windows)
            if (w.from <= ms && ms < w.to) return w.slack;
        return 0.0;
    };
    std::vector<double> probe;
    for (const SpMeterCurve* cv : {&scene.sp_meter, &want_curve})
        for (const SpMeterSegment& s : cv->segments)
            for (double ms : {s.start_ms, s.end_ms})
                for (double t : {ms, std::nextafter(ms, -1e300), std::nextafter(ms, 1e300)})
                    probe.push_back(t);
    std::uniform_real_distribution<double> any(-1000.0, scene.song_length_ms + 3000.0);
    for (int i = 0; i < random_times; ++i) probe.push_back(any(rng));
    std::vector<GaugeDiff> out;
    for (double t : probe) {
        if (in_late_sqin_window(t)) continue;
        const double got = sp_meter_bars_at(scene.sp_meter, t);
        const double want = sp_meter_bars_at(want_curve, t);
        if (std::fabs(got - want) > slack_at(t) + 1e-9) out.push_back({t, got, want});
    }
    return out;
}

// Every lookup on `scene` against the old scans: the fill states and (when
// `compare_gauge`) the SP meter once, then the time box and next-activation
// box at random times plus every boundary where a lookup's answer can flip
// (tempo, signature, section and activation times, and a hair either side).
// `path` is the path the scene was built from, or null.
void check_lookups_match_old_scans(const PreviewScene& scene, const Path* path, int sp_cap,
                                   std::mt19937& rng, bool compare_gauge) {
    const std::vector<PreviewFillState> want_fills =
        old_scan::fill_states(scene, path != nullptr);
    REQUIRE(want_fills.size() == scene.fills.size());
    for (size_t i = 0; i < want_fills.size(); ++i) {
        CAPTURE(i);
        CHECK(scene.fills[i].state == want_fills[i]);
    }

    REQUIRE(scene.timing.has_value());
    if (compare_gauge) {
        for (const GaugeDiff& d : gauge_differences(scene, path, sp_cap, rng)) {
            CAPTURE(d.ms);
            CAPTURE(d.got);
            CAPTURE(d.want);
            FAIL_CHECK("the gauge differs from the old scan by more than the stored end's tick");
        }
    }

    // At most this many boundaries per list, spread evenly, so a chart with
    // thousands of tempo changes stays quick.
    constexpr size_t kPerList = 150;
    std::vector<double> times;
    auto around = [&times](double ms) {
        times.push_back(ms);
        times.push_back(std::nextafter(ms, -1e300));
        times.push_back(std::nextafter(ms, 1e300));
    };
    auto sample = [&](size_t n, auto&& ms_of) {
        const size_t step = n > kPerList ? n / kPerList : 1;
        for (size_t i = 0; i < n; i += step) around(ms_of(i));
    };
    const SongTiming& timing = *scene.timing;
    sample(scene.tempos.size(), [&](size_t i) { return scene.tempos[i].ms; });
    sample(scene.time_sigs.size(), [&](size_t i) { return timing.ms_index().at(scene.time_sigs[i].tick); });
    sample(scene.sections.size(), [&](size_t i) { return scene.sections[i].ms; });
    sample(scene.activations.size(), [&](size_t i) {
        const double ms = scene.activations[i].ms;
        around(ms + old_scan::kOnActivationMs);
        return ms - old_scan::kOnActivationMs;
    });
    const double len = scene.song_length_ms;
    std::uniform_real_distribution<double> any_time(-1000.0, len + 3000.0);
    for (int i = 0; i < 300; ++i) times.push_back(any_time(rng));

    for (double length_ms : {len, len + 2500.0}) {
        for (double now : times) {
            CAPTURE(now);
            CAPTURE(length_ms);
            const PreviewTimeBox g = build_time_box(scene, now, length_ms);
            const PreviewTimeBox w = old_scan::time_box(scene, now, length_ms);
            CHECK(g.timestamp == w.timestamp);
            CHECK(g.position == w.position);
            CHECK(g.length == w.length);
            CHECK(g.tempo == w.tempo);
            CHECK(g.section_line == w.section_line);
        }
    }
    for (double now : times) {
        CAPTURE(now);
        const PreviewNextActBox g = build_next_act_box(scene, now);
        const PreviewNextActBox w = old_scan::next_act_box(scene, now);
        CHECK(g.shown == w.shown);
        CHECK(g.header == w.header);
        CHECK(g.detail == w.detail);
    }
}

// A busy chart for the comparison: a tempo change every one to six beats, a
// meter change (with its written signature) every few measures, sections
// every few measures, an SP phrase every eight notes and a fill on every
// third note. Then a path of activations on fills, in time order, each with
// a random bar count, skip count and some phrases collected inside it.
struct BusyChart {
    Song song{480};
    Path path;
};

BusyChart make_busy_chart(std::mt19937& rng, int64_t measures) {
    BusyChart c;
    Song& song = c.song;
    const int64_t last = measures * 1920;
    song.bpm_changes[0] = 120.0;
    std::uniform_int_distribution<int> beats(1, 6);
    std::uniform_int_distribution<int> bpm(60, 240);
    for (int64_t t = 480 * 3; t < last; t += 480 * beats(rng)) song.bpm_changes[t] = bpm(rng);
    std::uniform_int_distribution<int> bars(2, 6);
    for (int64_t t = 1920 * 4; t < last; t += 1920 * bars(rng)) {
        const bool three = (t / 1920) % 2 == 0;
        song.tpm_changes[t] = three ? 1440 : 1920;
        song.timesig_changes[t] = three ? std::make_pair(3, 4) : std::make_pair(4, 4);
    }
    for (int64_t t = 1920 * 2, n = 1; t < last; t += 1920 * bars(rng), ++n)
        song.practice_sections.push_back({t, "part " + std::to_string(n)});
    song.build_timing();

    int note = 0;
    for (int64_t t = 0; t <= last; t += 240, ++note) {
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord.add_note(NoteColor::Red);
        if (note % 8 == 7) {
            ts.flag_sp = true;
            ts.sp_phrase_start = t - 240 * 7;
        }
        if (note % 3 == 2) ts.activation_length = 480;
        song.sequence.push_back(std::move(ts));
    }

    std::uniform_int_distribution<int> sp(1, 4);
    std::uniform_int_distribution<int> skips(0, 3);
    std::uniform_int_distribution<int> gap(0, 12);
    std::bernoulli_distribution collect(0.5);
    int64_t after = 0;
    int64_t prev_act = std::numeric_limits<int64_t>::min();
    for (const SongTimestamp& ts : song.sequence) {
        const int64_t t = ts.timecode.ticks();
        if (!ts.activation_length || t < after) continue;
        if (gap(rng) != 0) continue;
        // Only lookups are tested here, so the end need not follow this chart's
        // meter changes: a flat two measures of 1920 ticks per bar spent.
        const int bars_spent = sp(rng);
        Activation a = sp_act_at(song, t, bars_spent, t + 3840 * bars_spent);
        // The passed-over fills are the nearest ones before the activation,
        // the ticks the old scan would light, so the two agree on made-up data.
        std::vector<int64_t> before;  // fills after the previous activation, before this one
        for (const SongTimestamp& f : song.sequence) {
            const int64_t ft = f.timecode.ticks();
            if (f.activation_length && ft > prev_act && ft < t) before.push_back(ft);
        }
        const int n = std::min(skips(rng), static_cast<int>(before.size()));
        a.skipped_fill_ticks.assign(before.end() - n, before.end());
        prev_act = t;
        a.chord.add_note(NoteColor::Red);
        for (const SongTimestamp& p : song.sequence) {
            const int64_t pt = p.timecode.ticks();
            // A collection step that keeps the end: only lookups are tested.
            if (p.flag_sp && pt > t && pt < *a.deact_tick() && collect(rng))
                a.sp_end_steps.push_back({pt, *a.deact_tick(), SpEndKind::Collected});
        }
        after = *a.deact_tick() + 1;
        c.path.activations.push_back(a);
    }
    return c;
}

}  // namespace

TEST_CASE("preview lookups: searches match the old scans on a busy synthetic chart") {
    std::mt19937 rng(20261003);
    for (int round = 0; round < 3; ++round) {
        CAPTURE(round);
        const BusyChart c = make_busy_chart(rng, 400);
        REQUIRE(c.path.activations.size() > 20);
        // Its activations are made up and carry no engine facts, so the gauge
        // is not compared here; the corpus case below compares it.
        for (int cap : {1, 4, 6}) {
            CAPTURE(cap);
            check_lookups_match_old_scans(build_preview_scene(c.song, &c.path, cap), &c.path, cap,
                                          rng, /*compare_gauge=*/false);
        }
        check_lookups_match_old_scans(build_preview_scene(c.song, nullptr, 4), nullptr, 4, rng,
                                      /*compare_gauge=*/false);

        // Sections out of tick order (a MIDI with two EVENTS tracks lists each
        // track's in turn) keep the old front-to-back answer.
        PreviewScene shuffled = build_preview_scene(c.song, &c.path, 4);
        REQUIRE(shuffled.sections.size() > 4);
        std::rotate(shuffled.sections.begin(),
                    shuffled.sections.begin() + static_cast<std::ptrdiff_t>(shuffled.sections.size() / 2),
                    shuffled.sections.end());
        check_lookups_match_old_scans(shuffled, &c.path, 4, rng, /*compare_gauge=*/false);
    }
}

TEST_CASE("preview lookups: searches match the old scans on every corpus chart and path") {
    AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 3;
    settings.ms_filter = 10.0;
    std::mt19937 rng(7);
    int charts_with_paths = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        std::optional<AnalysisResult> r;
        try {
            r.emplace(analyze_chart_file(chart, settings));
        } catch (const std::exception&) {
            continue;  // a chart the analysis rejects has no scene to compare
        }
        if (r->song.is_empty()) continue;
        // Without a path the chart-only gauge must not move at all (Q10).
        check_lookups_match_old_scans(build_preview_scene(r->song, nullptr, 4), nullptr, 4, rng,
                                      /*compare_gauge=*/true);
        if (!r->record.paths.empty()) ++charts_with_paths;
        // Root paths. The gauge reads the record and must equal the old scan
        // everywhere outside a late squeeze-in window: the proof that nothing
        // else on screen moved.
        for (const Path& p : r->record.paths)
            check_lookups_match_old_scans(build_preview_scene(r->song, &p, 4), &p, 4, rng,
                                          /*compare_gauge=*/true);
    }
    CHECK(charts_with_paths > 10);
}

// The Preview lights a stored passed-over fill by matching its tick to a fill
// in the scene, and drops a tick that matches none. A miss would mean the
// record and the chart disagree, so on the corpus there must be none.
TEST_CASE("preview fills: every stored passed-over fill is a fill in the scene") {
    // The app's default settings, as "base + overlay" reads the corpus.
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int ticks_checked = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        if (rec.paths.empty()) continue;
        std::set<int64_t> fill_ends;
        for (const PreviewFill& f : build_preview_base(song).fills) fill_ends.insert(f.span.end_tick);
        for (size_t pi = 0; pi < rec.paths.size(); ++pi) {
            for (const Activation& a : rec.paths[pi].walk_activations()) {
                for (int64_t t : a.skipped_fill_ticks) {
                    ++ticks_checked;
                    if (fill_ends.count(t) == 0)
                        MESSAGE(chart << " | root " << pi << " | activation "
                                      << a.timecode.ticks() << " | stored fill tick " << t);
                    CHECK(fill_ends.count(t) == 1);
                }
            }
        }
    }
    MESSAGE("stored passed-over fill ticks checked: " << ticks_checked);
    CHECK(ticks_checked > 0);
}

// The same gauge comparison on every tied variant. Each variant keeps its own
// bank (Task 18), so its gauge reads its own bar arrivals. Any difference is
// printed before the check fails.
TEST_CASE("preview lookups: every corpus variant's gauge matches the old scan") {
    // The app's default settings, as "base + overlay" reads the corpus.
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    std::mt19937 rng(7);
    int variants = 0, differing = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        for (size_t pi = 0; pi < rec.paths.size(); ++pi) {
            const Path& root = rec.paths[pi];
            for (size_t vi = 0; vi < root.variants.size(); ++vi) {
                const Path& v = root.variants[vi];
                ++variants;
                const std::vector<GaugeDiff> diffs = gauge_differences(
                    build_preview_scene(song, &v, cfg.sp_cap), &v, cfg.sp_cap, rng);
                if (diffs.empty()) continue;
                ++differing;
                for (const GaugeDiff& d : diffs) {
                    if (d.ms < 0.0) continue;
                    const int64_t tick = std::llround(song.timing().ms_index().tick_at_ms(d.ms));
                    MESSAGE(chart << " | root " << pi << " variant " << vi << " (" << v.pathstring()
                                  << ") | tick " << tick << " | ms " << d.ms << " | gauge "
                                  << d.got << " | old scan " << d.want);
                }
                CHECK(diffs.empty());
            }
        }
    }
    MESSAGE("variants compared: " << variants << ", differing: " << differing);
}

TEST_CASE("sp meter readout: bars banked over the cap") {
    TwoActs t;
    // The phrase at 1000 ms banks a bar; the activation at 2000 ms drains it
    // to empty at its deact node, two measures later (6000 ms).
    CHECK(sp_meter_readout(t.scene.sp_meter, 1500.0) == "1.0/4");
    CHECK(sp_meter_readout(t.scene.sp_meter, 4000.0) == "0.5/4");
    CHECK(sp_meter_readout(SpMeterCurve{}, 0.0).empty());
}

TEST_CASE("preview path label: the notation and which list it came from") {
    PathButtonView b;
    b.notation = "3- 1 2";
    b.group = PathButtonView::Group::Optimal;
    CHECK(preview_path_label(b) == "3- 1 2  (optimal)");
    b.notation = "0 4 1";
    b.group = PathButtonView::Group::Within;
    CHECK(preview_path_label(b) == "0 4 1");
    b.notation = "0 0 0 0";
    b.group = PathButtonView::Group::AllZero;
    CHECK(preview_path_label(b) == "0 0 0 0  (best all-0)");
}
