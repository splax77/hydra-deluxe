// Tests for app/preview_view: the note-highway view-model the Preview tab
// renders. A hand-built song pins the note/lane/attribute mapping and the
// shaded spans exactly; the corpus cases confirm the SP-phrase-start the parser
// now keeps, and that an analyzed chart's overlay lines up with its path.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
#include "app/path_view.h"
#include "app/song_length.h"
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
        test::mark_phrase_end(ts, 720, 480);
        ts.activation_length = 480;
    }
    // The one solo section: the chords at ticks 240 and 480 (sequence
    // positions 1 and 2). The parsers build this list from the flags
    // (find_solo_sections in song.cpp, private there); a hand-built song
    // states it.
    song.solo_sections = {{1, 2}};
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
// a phrase off the quarter-note grid. `extra_sigs` adds time signatures (tick ->
// numerator, denominator) the same way.
Song make_sp_song(const std::vector<int64_t>& phrase_ends, int64_t last_tick,
                  const std::map<int64_t, double>& extra_bpm = {},
                  int64_t step = 480,
                  const std::map<int64_t, std::pair<int, int>>& extra_sigs = {}) {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    for (const auto& kv : extra_bpm) song.bpm_changes[kv.first] = kv.second;
    for (const auto& [tick, sig] : extra_sigs) apply_timesig(song, tick, sig.first, sig.second);
    song.build_timing();

    for (int64_t t = 0; t <= last_tick; t += step) {
        Chord c;
        c.add_note(NoteColor::Red);
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord = std::move(c);
        if (std::find(phrase_ends.begin(), phrase_ends.end(), t) != phrase_ends.end())
            test::mark_phrase_end(ts, t, step);
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
    return test::build_fixture_song(192, 120.0,
                                    {{0, true},     {768, true},  {2304, false, 384}, {3072, true},
                                     {3840, true},  {4608},       {5376},             {6000},
                                     {6768},        {7500}});
}

// One analyzed corpus chart (the first that yields paths), shared across cases.
const AnalysisResult& analyzed() {
    static const AnalysisResult result = [] {
        AnalysisSettings settings;
        settings.depth_mode = DepthMode::Scores;
        settings.depth_value = 10;
        settings.ms_filter = 10.0;
        return corpus::first_analyzed_with_paths(settings);
    }();
    return result;
}

// A path whose stored score is what the replay prices it at, so the scene
// trusts it. The corpus case proves the replay against the engine; these
// hand-built fixtures only test the Preview's plumbing.
Path priced_path(const Song& song, std::vector<Activation> acts) {
    Path p;
    p.activations = std::move(acts);
    assign_score(p, replay_stored_path(song, p).result.final);
    return p;
}

// The detail line's separator: a middle dot, U+00B7, in UTF-8.
const std::string kDot = "\xC2\xB7";

// The overlay boxes at `now_ms` with no end to playback, so shown_ms leaves
// the playhead where it is. The cases that pin what a playhead past the end
// reads call the builders with a length.
constexpr double kNoPlaybackEnd = std::numeric_limits<double>::infinity();
PreviewScoreBox score_box_at(const PreviewScene& sc, double now_ms) {
    return build_score_box(sc, now_ms, kNoPlaybackEnd);
}
PreviewDrainBox drain_box_at(const PreviewScene& sc, double now_ms) {
    return build_drain_box(sc, now_ms, kNoPlaybackEnd);
}
PreviewNextActBox next_act_box_at(const PreviewScene& sc, double now_ms, bool pro_drums) {
    return build_next_act_box(sc, now_ms, kNoPlaybackEnd, pro_drums);
}

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

    // The song's length is the one the load hands in (D75); none was given.
    CHECK(scene.song_length_ms == 0.0);
}

// color_of reads lane_of backwards, so a drawn lane can ask the core's colour
// rules (allows_cymbals) without a second lane table.
TEST_CASE("color_of: each lane maps back to the colour it was drawn from") {
    for (NoteColor c : {NoteColor::Kick, NoteColor::Red, NoteColor::Yellow, NoteColor::Blue,
                        NoteColor::Green})
        CHECK(color_of(lane_of(c)) == c);
}

// The scene's song length is the length it is given (D75), never a note's
// time: not the last drawn note, and not a later note-less timestamp. Without
// one there is no length. The beat lines without an audio end still end two
// measures past the last drawn note, as before.
TEST_CASE("build_preview_scene: the song length is the one handed in, never the audio's end") {
    Song song = make_hand_song();
    SongTimestamp empty;  // a timestamp with no notes, after the last chord
    empty.timecode = song.timecode(960);
    song.sequence.push_back(empty);

    // An audio end alone is not a length (D75): the song's length is handed
    // in on its own.
    const PreviewScene with_audio =
        build_preview_scene(song, nullptr, kCloneHeroSpCap, core::default_rules(), 4321.0);
    CHECK(with_audio.song_length_ms == 0.0);

    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.has_notes);
    CHECK(last_note_ms(scene) == doctest::Approx(750.0));
    // The chart-level answer ignores the note-less timestamp the same way.
    CHECK(app::last_note_start_ms(song) == last_note_ms(scene));
    CHECK(scene.song_length_ms == 0.0);
    const PreviewScene before = build_preview_scene(make_hand_song(), nullptr);
    REQUIRE_FALSE(scene.beats.empty());
    REQUIRE_FALSE(before.beats.empty());
    CHECK(scene.beats.back().tick == before.beats.back().tick);
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

TEST_CASE("build_preview_scene: solo spans come from the song's solo sections") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.solos.size() == 1);
    CHECK(scene.solos[0].start_tick == 240);
    CHECK(scene.solos[0].end_tick == 480);

    // The span is the song's section, not a join of the flags: with no
    // section there is no span, though each note still says it is in a solo.
    song.solo_sections.clear();
    PreviewScene none = build_preview_scene(song, nullptr);
    CHECK(none.solos.empty());
    REQUIRE(none.notes.size() > 2);
    CHECK(none.notes[1].solo);
    CHECK(none.notes[2].solo);
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
    // The Preview passes the audio's end, as the load does: here the audio
    // runs the plan's 5 s past the last note (tick 720, 750 ms), to 5750 ms.
    PreviewScene scene = build_preview_scene(song, nullptr, kCloneHeroSpCap,
                                             core::default_rules(), 5750.0);
    REQUIRE(scene.timing.has_value());
    CHECK(scene.timing->tick_resolution() == 480);
    REQUIRE(scene.tempos.size() == 1);
    CHECK(scene.tempos[0].bpm == doctest::Approx(120.0));
    REQUIRE(!scene.beats.empty());
    CHECK(scene.beats.front().tick == 0);
    // 5750 ms is tick 5520; the last line at or before it is the beat at
    // 5280 (no half-beat line is drawn before a beat the grid does not reach).
    CHECK(scene.beats.back().tick == 5280);
    CHECK(scene.beats.back().kind == PreviewBeatKind::Beat);
}

TEST_CASE("build_preview_scene: the beat lines run to the end of the audio") {
    // The audio-tail chart: the last note is at tick 1920 (1000 ms) and the
    // audio stops 5 s later, at 6000 ms, which is tick 11520 (six measures of
    // 1920 ticks). The beat lines keep scrolling through that tail and stop at
    // the barline on the audio's end (D48, Q25).
    const test::AudioTailChart c = test::audio_tail_chart();
    const PreviewScene scene =
        build_preview_scene(c.song, nullptr, kCloneHeroSpCap, core::default_rules(), c.audio_end_ms);
    REQUIRE(!scene.beats.empty());
    CHECK(scene.beats.back().tick == 11520);
    CHECK(scene.beats.back().kind == PreviewBeatKind::Bar);
    CHECK(scene.beats.back().ms == doctest::Approx(6000.0));
    // The base alone gives the same grid: the overlay never touches beats.
    CHECK(build_preview_base(c.song, c.audio_end_ms).beats.back().tick == 11520);
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

TEST_CASE("build_time_box: the timestamp rounds 59,999.6 ms to 1:00.000") {
    // The clock rounds to the whole ms before it splits off the minutes, so
    // it never reads 0:60.000 (D48, Q20).
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(build_time_box(scene, 59999.6, 64000.0).timestamp == "1:00.000 / 1:04.000");
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
    apply_timesig(song, 2880, 3, 4);
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
    apply_timesig(song, 1920, 6, 8);
    apply_timesig(song, 3360, 3, 4);
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
    const double last_note = last_note_ms(scene);
    CHECK(last_note > 0.0);

    // Notes are in non-decreasing tick order.
    for (size_t i = 1; i < scene.notes.size(); ++i)
        CHECK(scene.notes[i].tick >= scene.notes[i - 1].tick);

    // Every activation the path takes (those with a resolved timecode) appears
    // in the overlay, inside the song, in ms that share the notes' timing.
    const size_t expected = best.all_activations().size();
    CHECK(scene.activations.size() == expected);
    size_t i = 0;
    for (const Activation& a : best.all_activations()) {
        const PreviewActivation& pa = scene.activations[i++];
        CHECK(pa.tick >= 0);
        CHECK(pa.ms >= 0.0);
        CHECK(pa.ms <= last_note + 1.0);
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
        CHECK(pa.chord == a.chord);  // named by the box, in the Pro Drums setting's words
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

// The smallest cap is typed once, in the settings key table; both meters
// floor through Settings::clamp (finding 139). A cap of 0 reads as the
// owner's floor with a path and without one.
TEST_CASE("sp meter curve: the cap floor is the settings owner's") {
    const int floor_cap = Settings::clamp(&Settings::sp_cap, 0);
    CHECK(floor_cap == 1);
    Song song = make_sp_song({960, 1920, 5760}, /*last_tick=*/9600);

    const PreviewScene unanalyzed = build_preview_scene(song, nullptr, /*sp_cap=*/0);
    CHECK(unanalyzed.sp_meter.cap == floor_cap);

    Path path;
    path.activations = {act_at(song, 3840, /*skipped_fills=*/{})};
    const PreviewScene with_path = build_preview_scene(song, &path, /*sp_cap=*/0);
    CHECK(with_path.sp_meter.cap == floor_cap);
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
    CHECK_FALSE(score_box_at(scene, 600.0).shown);
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
    // replay_path owns the withheld total; 180 was pinned from one run on
    // 2026-10-05 as a fixed oracle beside it.
    CHECK(scene.score.steps[1].total == r.chords[1].cum_onscreen_total);
    CHECK(scene.score.steps[1].total == 180);
    CHECK(scene.score.steps[2].total == r.chords[2].cum.total());
    CHECK(scene.score.steps[3].total == path.totalscore());

    // Between the solo's two chords the box shows the withheld total.
    PreviewScoreBox mid = score_box_at(scene, 400.0);
    CHECK(mid.shown);
    CHECK(mid.available);
    CHECK(mid.score == group_thousands(scene.score.steps[1].total));
    CHECK(mid.detail == "x1 " + kDot + " combo 2");

    // A chord exactly at the playhead counts as hit.
    PreviewScoreBox on = score_box_at(scene, scene.score.steps[2].ms);
    CHECK(on.score == group_thousands(scene.score.steps[2].total));
    CHECK(on.detail == "x1 " + kDot + " combo 3");
}

TEST_CASE("score box: before the first note nothing is hit yet") {
    // The fill song's first note is at tick 480, 500 ms.
    Song song = make_fill_song();
    Path path = priced_path(song, {});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    PreviewScoreBox box = score_box_at(scene, 100.0);
    CHECK(box.shown);
    CHECK(box.available);
    CHECK(box.score == "0");
    CHECK(box.detail == "x1 " + kDot + " combo 0");
}

TEST_CASE("struck_at: a chord exactly on the playhead counts as hit") {
    // 2500 ms is the activation chord (tick 2400) of the make_sp_song({1920},
    // 9600) score-box case below: a jump to that activation lands on it.
    CHECK(struck_at(2500.0, 2500.0));
    CHECK_FALSE(struck_at(2499.9, 2500.0));
    CHECK(struck_at(2500.1, 2500.0));
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
    CHECK(score_box_at(scene, 6499.0).detail == "x4 " + kDot + " combo 13");
    CHECK(score_box_at(scene, 6500.0).detail == "x2 " + kDot + " combo 14");
    CHECK(score_box_at(scene, scene.activations[0].sp_end_ms).detail ==
          "x2 " + kDot + " combo 14");

    // 2000 ms: five notes hit, before the activation.
    CHECK(score_box_at(scene, 2000.0).detail == "x1 " + kDot + " combo 5");
    // 2500 ms: the activation chord is hit and paid, so x1 shows as x2.
    CHECK(score_box_at(scene, 2500.0).detail == "x2 " + kDot + " combo 6");
    CHECK(score_box_at(scene, 3000.0).detail == "x2 " + kDot + " combo 7");
    // 6600 ms: Star Power has ended, so the disc is plain even though the
    // last chord hit (6500 ms, on the deactivation node) was paid doubled.
    CHECK(score_box_at(scene, 6600.0).detail == "x2 " + kDot + " combo 14");
    // 7000 ms: the first chord Star Power doesn't pay: the plain x2 of combo 15.
    CHECK(score_box_at(scene, 7000.0).detail == "x2 " + kDot + " combo 15");
}

TEST_CASE("score box: past the length it reads the moment the time box shows") {
    // The fixture above: 2000 ms is five notes in, 7000 ms fifteen.
    Song song = make_sp_song({1920}, 9600);
    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);

    CHECK(build_score_box(scene, 7000.0, /*length_ms=*/2000.0).detail ==
          "x1 " + kDot + " combo 5");
    CHECK(build_score_box(scene, 7000.0, /*length_ms=*/7000.0).detail ==
          "x2 " + kDot + " combo 15");
}

TEST_CASE("score box: a chord exactly on the playhead counts as hit") {
    // The case above: "Act >" from the start lands on the activation chord at
    // 2500 ms, and the box counts that chord as hit and paid (D48, Q27). The
    // box already counted it before struck_at existed, so this case guards the
    // shared rule rather than a fix: make struck_at exclusive and it reads
    // "x1 · combo 5". The highway's case is the one the fix turned green.
    Song song = make_sp_song({1920}, 9600);
    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    const std::optional<double> jump = activation_jump_ms(scene, 0.0, +1);
    REQUIRE(jump.has_value());
    CHECK(score_box_at(scene, *jump).detail == "x2 " + kDot + " combo 6");
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
    const PreviewScoreBox box = score_box_at(scene, leeway.ms);
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
    PreviewScoreBox box = score_box_at(a, 600.0);
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
    CHECK(score_box_at(b, 600.0).score == "Score unavailable");
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
    PreviewScoreBox end = score_box_at(scene, last_note_ms(scene));
    CHECK(end.score == group_thousands(best.totalscore()));
}

// ---- SP drain box -------------------------------------------------------

TEST_CASE("drain box: hidden without an SP gauge") {
    CHECK_FALSE(drain_box_at(PreviewScene{}, 1000.0).shown);
    // Notes but no SP phrase and no path: no gauge, so no box.
    Song song = make_sp_song({}, /*last_tick=*/3840);
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.sp_meter.segments.empty());
    CHECK_FALSE(drain_box_at(scene, 1000.0).shown);
}

TEST_CASE("drain box: idle at a steady tempo reads the rate and a full meter") {
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    PreviewScene scene = build_preview_scene(song, nullptr);

    PreviewDrainBox box = drain_box_at(scene, 2000.0);
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

    CHECK(drain_box_at(scene, 5990.0).rate == "1 bar / 4.0 s");
    CHECK(drain_box_at(scene, 6000.0).rate == "1 bar / 8.0 s");

    // "full meter" is the cap at the bar time in force, so it jumps with the
    // rate, at the change and not before: 4 x 4 s, then 4 x 8 s.
    CHECK(drain_box_at(scene, 2000.0).detail == "full meter 16.0 s");
    CHECK(drain_box_at(scene, 5990.0).detail == "full meter 16.0 s");
    CHECK(drain_box_at(scene, 6000.0).detail == "full meter 32.0 s");
}

TEST_CASE("drain box: past the length it reads the moment the time box shows") {
    // The fixture above. A playhead past where playback ends reads the
    // moment the time box shows (shown_ms), so it lands on the length.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {{5760, 60.0}});
    PreviewScene scene = build_preview_scene(song, nullptr);

    const PreviewDrainBox held = build_drain_box(scene, 6000.0, /*length_ms=*/5990.0);
    CHECK(held.rate == "1 bar / 4.0 s");
    CHECK(held.detail == "full meter 16.0 s");
    const PreviewDrainBox at = build_drain_box(scene, 6000.0, /*length_ms=*/6000.0);
    CHECK(at.rate == "1 bar / 8.0 s");
    CHECK(at.detail == "full meter 32.0 s");
}

TEST_CASE("drain box: a 7/8 section drains faster at the same BPM") {
    // 7/8 from tick 3840 (4000 ms, a barline): 1680 ticks = 1750 ms a
    // measure, so a bar lasts 3500 ms instead of 4000.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {}, 480, {{3840, {7, 8}}});
    PreviewScene scene = build_preview_scene(song, nullptr);

    CHECK(drain_box_at(scene, 2000.0).rate == "1 bar / 4.0 s");
    CHECK(drain_box_at(scene, 4000.0).rate == "1 bar / 3.5 s");
    CHECK(drain_box_at(scene, 4000.0).detail == "full meter 14.0 s");
}

TEST_CASE("drain box: active inside the stored SP window, idle outside it") {
    // Two bars at tick 3840 (4000 ms): the stored end is tick 11520 (12000 ms).
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};
    PreviewScene scene = build_preview_scene(song, &path);

    PreviewDrainBox before = drain_box_at(scene, 3999.0);
    CHECK_FALSE(before.active);
    CHECK(before.header == "SP drain (if activated)");

    PreviewDrainBox at = drain_box_at(scene, 4000.0);
    CHECK(at.active);
    CHECK(at.header == "SP drain");
    CHECK(at.rate == "1 bar / 4.0 s");
    CHECK(at.detail == "empties in 8.0 s");
    CHECK(drain_box_at(scene, 9000.0).detail == "empties in 3.0 s");

    // At the stored end SP is over, as the gauge reads that boundary too.
    PreviewDrainBox after = drain_box_at(scene, 12000.0);
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

    PreviewDrainBox box = drain_box_at(scene, 12000.0);
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

    PreviewDrainBox box = drain_box_at(scene, 5000.0);
    CHECK(box.shown);
    CHECK_FALSE(box.active);
    CHECK(box.detail.rfind("full meter ", 0) == 0);
}

TEST_CASE("has_sp_gauge: timing and a curve together") {
    CHECK_FALSE(PreviewScene{}.has_sp_gauge());

    // Timing but no curve: the scene of "drain box: hidden without an SP gauge".
    PreviewScene no_curve = build_preview_scene(make_sp_song({}, /*last_tick=*/3840), nullptr);
    REQUIRE(no_curve.timing.has_value());
    CHECK_FALSE(no_curve.has_sp_gauge());

    // A curve but no timing.
    PreviewScene no_timing;
    no_timing.sp_meter.segments.push_back(SpMeterSegment{});
    CHECK_FALSE(no_timing.has_sp_gauge());

    // The scene of "drain box: active inside the stored SP window, idle outside it".
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};
    CHECK(build_preview_scene(song, &path).has_sp_gauge());
}

TEST_CASE("sp_window: the stored window, none without an end, none with no length") {
    // The numbers of test_track_state's "active SP window ends exactly at the
    // deact node".
    PreviewActivation a;
    a.ms = 1000.0;
    a.has_sp_end = true;
    a.sp_end_ms = 3000.0;
    const std::optional<std::pair<double, double>> w = a.sp_window();
    REQUIRE(w.has_value());
    CHECK(w->first == 1000.0);
    CHECK(w->second == 3000.0);

    PreviewActivation no_end = a;
    no_end.has_sp_end = false;
    CHECK_FALSE(no_end.sp_window().has_value());

    PreviewActivation no_length = a;
    no_length.sp_end_ms = 1000.0;
    CHECK_FALSE(no_length.sp_window().has_value());
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

TEST_CASE("song_fraction: a clamped share, none without a length") {
    // The scrub marks' numbers: 2000 ms of 10,000 ms, 8000 ms of 4000 ms.
    REQUIRE(song_fraction(2000.0, 10000.0).has_value());
    CHECK(*song_fraction(2000.0, 10000.0) == doctest::Approx(0.2));
    REQUIRE(song_fraction(8000.0, 4000.0).has_value());
    CHECK(*song_fraction(8000.0, 4000.0) == doctest::Approx(1.0));
    CHECK_FALSE(song_fraction(2000.0, 0.0).has_value());
}

TEST_CASE("song_fraction: has_song_length takes only a positive length") {
    CHECK(has_song_length(10000.0));
    CHECK_FALSE(has_song_length(0.0));
    CHECK_FALSE(has_song_length(-1.0));
}

// The audio-tail chart: the last note is at 1000 ms and the audio stops 5 s
// later, at 6000 ms. One activation on the note at tick 1440 (750 ms); one
// bar of SP runs two measures, so its window ends at tick 5280. The scrubber
// is built the way the Preview builds it (its right edge from scrub_end_ms
// over the song's length) and the Paths timeline over the same length
// (app::song_length_ms, D75; this chart's stated length is its 6000 ms), and
// the two marks must be the same number.
TEST_CASE("scrub marks: an activation sits at the Paths timeline's fraction when the audio outlasts the notes") {
    const test::AudioTailChart c = test::audio_tail_chart();
    Path path;
    path.activations = {sp_act_at(c.song, 1440, /*sp_meter=*/1, /*end_tick=*/5280)};
    const PreviewScene scene = build_preview_scene(c.song, &path, kCloneHeroSpCap,
                                                   core::default_rules(), c.audio_end_ms);
    const std::vector<double> marks =
        build_scrub_marks(scene, scrub_end_ms(c.audio_end_ms, c.audio_end_ms));
    REQUIRE(marks.size() == 1);
    CHECK(marks[0] == doctest::Approx(0.125));

    const ActivationsView view =
        build_activations(path, HydraRecord{}, &c.song.timing(), Settings{}.hit_window_ms,
                          std::nullopt, core::default_rules(), c.audio_end_ms);
    REQUIRE(view.acts.size() == 1);
    REQUIRE(view.acts[0].song_fraction.has_value());
    CHECK(marks[0] == *view.acts[0].song_fraction);
}

// The scrubber's right end is the song's length (D75; this chart states
// 6000 ms), so past the last note the thumb keeps following the playhead, all
// the way to that end.
TEST_CASE("scrub marks: a playhead past the last note parks the thumb at the right end") {
    const test::AudioTailChart c = test::audio_tail_chart();
    const double end = scrub_end_ms(c.audio_end_ms, c.audio_end_ms);
    CHECK(end == doctest::Approx(6000.0));
    CHECK(scrub_thumb_ms(c.audio_end_ms, end) == doctest::Approx(6000.0));
    CHECK(scrub_thumb_ms(3000.0, end) == doctest::Approx(3000.0));  // in the tail
    CHECK(scrub_thumb_ms(500.0, end) == doctest::Approx(500.0));
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
    PreviewNextActBox box = next_act_box_at(t.scene, 0.0, /*pro_drums=*/true);
    CHECK(box.shown);
    CHECK(box.header == "Next: activation 1 of 2");
    CHECK(box.detail == "at m2.1.0 " + kDot + " [Red snare]");
    CHECK(next_act_box_at(t.scene, 2000.0, true).header == "Next: activation 1 of 2");
    CHECK(next_act_box_at(t.scene, 2001.0, true).header == "Next: activation 2 of 2");
    CHECK(next_act_box_at(t.scene, 2001.0, true).detail == "at m5.1.0 " + kDot + " [Red snare]");
    CHECK_FALSE(next_act_box_at(t.scene, 9000.0, true).shown);
    CHECK_FALSE(next_act_box_at(build_preview_scene(t.song, nullptr), 0.0, true).shown);
}

TEST_CASE("next activation box: past the length it reads the moment the time box shows") {
    TwoActs t;
    CHECK(build_next_act_box(t.scene, 9000.0, /*length_ms=*/2000.0, true).header ==
          "Next: activation 1 of 2");
    CHECK_FALSE(build_next_act_box(t.scene, 9000.0, /*length_ms=*/9000.0, true).shown);
}

TEST_CASE("next activation box: the note names follow the Pro Drums setting") {
    // The Dynamics wording (note_label): with Pro Drums off the red pad is
    // plain "Red", with it on "Red snare" (D48, Q11).
    TwoActs t;
    CHECK(next_act_box_at(t.scene, 0.0, /*pro_drums=*/false).detail ==
          "at m2.1.0 " + kDot + " [Red]");
    CHECK(next_act_box_at(t.scene, 0.0, /*pro_drums=*/true).detail ==
          "at m2.1.0 " + kDot + " [Red snare]");
}

TEST_CASE("next activation box: on an activation within half a millisecond, and no chord") {
    TwoActs t;
    // The playhead counts as on an activation up to half a millisecond past
    // it: at 2000.5 ms the box still names activation 1, a hair later 2.
    CHECK(next_act_box_at(t.scene, -100.0, true).header == "Next: activation 1 of 2");
    CHECK(next_act_box_at(t.scene, 2000.5, true).header == "Next: activation 1 of 2");
    CHECK(next_act_box_at(t.scene, std::nextafter(2000.5, 1e300), true).header ==
          "Next: activation 2 of 2");
    CHECK(next_act_box_at(t.scene, 8000.5, true).header == "Next: activation 2 of 2");
    CHECK_FALSE(next_act_box_at(t.scene, std::nextafter(8000.5, 1e300), true).shown);

    // An activation with no chord names only its measure.
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    path.activations = {sp_act_at(song, 1920, /*sp_meter=*/1, /*end_tick=*/5760)};
    const PreviewNextActBox box = next_act_box_at(build_preview_scene(song, &path), 0.0, true);
    CHECK(box.header == "Next: activation 1 of 1");
    CHECK(box.detail == "at m2.1.0");
}

namespace {

// A chart whose lookups have many entries to search: four tempos, three
// meters (with the signature the chart wrote) and four sections, at 480
// ticks a beat. Tempo 120 BPM to tick 1920, 90 to 4800, 150 to 7680, then 60.
// 4/4, then 3/4 from tick 3840, then 4/4 again from 6720 (both barlines).
// So ticks 960, 1920, 2880, 3840, 4800, 5760, 6720, 7680, 8640 and 9600 are
// 1000, 2000, 3333.3, 4666.7, 6000, 6800, 7600, 8400, 10400 and 12400 ms.
Song make_lookup_song() {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.bpm_changes[1920] = 90.0;
    song.bpm_changes[4800] = 150.0;
    song.bpm_changes[7680] = 60.0;
    apply_timesig(song, 3840, 3, 4);
    apply_timesig(song, 6720, 4, 4);
    song.practice_sections = {{960, "Intro"}, {2880, "Verse"}, {5760, "Chorus"},
                              {8640, "Outro"}};
    song.build_timing();
    for (int64_t t = 0; t <= 9600; t += 960) {
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// The time box at `now` as one line: timestamp | position | length | tempo
// line | section line.
std::string time_box_line(const PreviewScene& scene, double now, double length_ms) {
    const PreviewTimeBox b = build_time_box(scene, now, length_ms);
    return b.timestamp + " | " + b.position + " | " + b.length + " | " + b.tempo + " | " +
           b.section_line;
}

// Built lines against pinned ones, printed as literals on a mismatch
// (record_fixtures.h).
using test::check_lines;

}  // namespace

TEST_CASE("build_time_box: every lookup at its boundaries on a chart of many changes") {
    const Song song = make_lookup_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.tempos.size() == 4);
    REQUIRE(scene.time_sigs.size() == 3);
    REQUIRE(scene.sections.size() == 4);
    const double len = 12400.0;

    // Each change at its own moment and a hair before it. The playhead's
    // tick is rounded to the nearest one, so a section, signature or measure
    // shows from half a tick before its tick: at 90 BPM a tick is 1.389 ms,
    // so 3332.7 ms is tick 2879.54 and shows Verse at m2.3.0, while 3332.6 ms
    // is tick 2879.47, still Intro at m2.2.479. Likewise 999.9 ms is tick
    // 959.9 (Intro) and 4666.0 ms tick 3839.5 (3/4). The tempo is looked up
    // by that same tick, so a hair before 2000 ms the box reads tick 1920
    // (m2.1.0) at the new 90 BPM (D48, Q23). In 3/4 a measure is 1440 ticks: measure 3 starts at 3840,
    // 4 at 5280, so tick 5759 (6799 ms at 150 BPM) is m4.1.479. Past the
    // length the playhead is held at 12400 ms, tick 9600, m6.3.0.
    std::vector<std::string> got;
    for (double now : {-500.0, 0.0, 999.9, 1000.0, std::nextafter(2000.0, -1e300), 2000.0,
                       3332.6, 3332.7, 4666.0, 4666.7, std::nextafter(6000.0, -1e300), 6000.0,
                       6799.0, 6800.0, 7600.0, std::nextafter(8400.0, -1e300), 8400.0, 10400.0,
                       20000.0})
        got.push_back(time_box_line(scene, now, len));
    const std::string d = " " + kDot + " ";
    check_lines(got,
                {"0:00.000 / 0:12.400 | m1.1.0 | m6.3.0 | BPM 120.000" + d + "4/4 | ",
                 "0:00.000 / 0:12.400 | m1.1.0 | m6.3.0 | BPM 120.000" + d + "4/4 | ",
                 "0:01.000 / 0:12.400 | m1.3.0 | m6.3.0 | BPM 120.000" + d + "4/4 | Section Intro",
                 "0:01.000 / 0:12.400 | m1.3.0 | m6.3.0 | BPM 120.000" + d + "4/4 | Section Intro",
                 "0:02.000 / 0:12.400 | m2.1.0 | m6.3.0 | BPM 90.000" + d + "4/4 | Section Intro",
                 "0:02.000 / 0:12.400 | m2.1.0 | m6.3.0 | BPM 90.000" + d + "4/4 | Section Intro",
                 "0:03.333 / 0:12.400 | m2.2.479 | m6.3.0 | BPM 90.000" + d + "4/4 | Section Intro",
                 "0:03.333 / 0:12.400 | m2.3.0 | m6.3.0 | BPM 90.000" + d + "4/4 | Section Verse",
                 "0:04.666 / 0:12.400 | m3.1.0 | m6.3.0 | BPM 90.000" + d + "3/4 | Section Verse",
                 "0:04.667 / 0:12.400 | m3.1.0 | m6.3.0 | BPM 90.000" + d + "3/4 | Section Verse",
                 "0:06.000 / 0:12.400 | m3.3.0 | m6.3.0 | BPM 150.000" + d + "3/4 | Section Verse",
                 "0:06.000 / 0:12.400 | m3.3.0 | m6.3.0 | BPM 150.000" + d + "3/4 | Section Verse",
                 "0:06.799 / 0:12.400 | m4.1.479 | m6.3.0 | BPM 150.000" + d + "3/4 | Section Verse",
                 "0:06.800 / 0:12.400 | m4.2.0 | m6.3.0 | BPM 150.000" + d + "3/4 | Section Chorus",
                 "0:07.600 / 0:12.400 | m5.1.0 | m6.3.0 | BPM 150.000" + d + "4/4 | Section Chorus",
                 "0:08.400 / 0:12.400 | m5.3.0 | m6.3.0 | BPM 60.000" + d + "4/4 | Section Chorus",
                 "0:08.400 / 0:12.400 | m5.3.0 | m6.3.0 | BPM 60.000" + d + "4/4 | Section Chorus",
                 "0:10.400 / 0:12.400 | m6.1.0 | m6.3.0 | BPM 60.000" + d + "4/4 | Section Outro",
                 "0:12.400 / 0:12.400 | m6.3.0 | m6.3.0 | BPM 60.000" + d + "4/4 | Section Outro"},
                "time box");

    // A negative length shows zero. A length past the last note keeps
    // counting at the last tempo and meter: 60 BPM from tick 9600 (12400
    // ms), so 15000 ms is tick 10848, 288 into measure 7 (from 10560), and
    // 16400 ms is tick 11520, its third beat.
    check_lines({time_box_line(scene, 500.0, -1.0), time_box_line(scene, 15000.0, 16400.0)},
                {"0:00.000 / 0:00.000 | m1.1.0 | m1.1.0 | BPM 120.000" + d + "4/4 | ",
                 "0:15.000 / 0:16.400 | m7.1.288 | m7.3.0 | BPM 60.000" + d + "4/4 | Section Outro"},
                "time box lengths");

    // Sections out of tick order never reach the time box: both parsers sort
    // them (sort_practice_sections, D27 R7.7), and the parser tests pin that.
}

TEST_CASE("build_time_box: inside the half tick before a tempo change every line shows the new values") {
    // make_lookup_song's tempo goes 120 -> 90 BPM at tick 1920 (2000 ms). A
    // hair before 2000 ms the playhead rounds to tick 1920, so the measure
    // line already reads m2.1.0; the BPM line must read that tick's tempo
    // too, not the old one (D48, Q23).
    const Song song = make_lookup_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    const PreviewTimeBox box = build_time_box(scene, std::nextafter(2000.0, -1e300), 12400.0);
    CHECK(box.position == "m2.1.0");
    CHECK(box.tempo == "BPM 90.000 " + kDot + " 4/4");
}

TEST_CASE("build_preview_scene: passed-over fills on several activations, and a tick that is no fill") {
    // Fills end at 480, 960, 1440 and 1920. The first activation takes 480,
    // the second passes over 960 and takes 1440, the third lists a tick (1700)
    // that is no fill, which lights nothing, and takes 1920.
    Song song = make_fill_song();
    Path path;
    path.activations = {act_at(song, 480, {}), act_at(song, 1440, {960}),
                        act_at(song, 1920, {1700})};
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.fills.size() == 4);
    CHECK(scene.fills[0].state == PreviewFillState::Taken);
    CHECK(scene.fills[1].state == PreviewFillState::Offered);
    CHECK(scene.fills[2].state == PreviewFillState::Taken);
    CHECK(scene.fills[3].state == PreviewFillState::Taken);
}

TEST_CASE("build_preview_scene: each activation names the fill it took") {
    // The fixtures of the three fill cases above; fills end at 480, 960,
    // 1440 and 1920.
    Song song = make_fill_song();

    // "skips say which fills the path was offered": the activation takes 1440.
    Path skips;
    skips.activations = {act_at(song, 1440, {960})};
    PreviewScene s1 = build_preview_scene(song, &skips);
    REQUIRE(s1.activations.size() == 1);
    CHECK(s1.activations[0].taken_fill == std::optional<size_t>{2});

    // "a fill an activation took stays taken when a later one lists it": the
    // first still names 960, though the second lists it as passed over.
    Path tied;
    tied.activations = {act_at(song, 960, {}), act_at(song, 1920, {960, 1440})};
    PreviewScene s2 = build_preview_scene(song, &tied);
    REQUIRE(s2.activations.size() == 2);
    CHECK(s2.activations[0].taken_fill == std::optional<size_t>{1});
    CHECK(s2.activations[1].taken_fill == std::optional<size_t>{3});

    // "passed-over fills on several activations, and a tick that is no fill".
    Path several;
    several.activations = {act_at(song, 480, {}), act_at(song, 1440, {960}),
                           act_at(song, 1920, {1700})};
    PreviewScene s3 = build_preview_scene(song, &several);
    REQUIRE(s3.activations.size() == 3);
    CHECK(s3.activations[0].taken_fill == std::optional<size_t>{0});
    CHECK(s3.activations[1].taken_fill == std::optional<size_t>{2});
    CHECK(s3.activations[2].taken_fill == std::optional<size_t>{3});

    // An activation on a tick that is no fill names none.
    Path off_fill;
    off_fill.activations = {act_at(song, 1700, {})};
    PreviewScene s4 = build_preview_scene(song, &off_fill);
    REQUIRE(s4.activations.size() == 1);
    CHECK_FALSE(s4.activations[0].taken_fill.has_value());
}

TEST_CASE("sp meter curve: a meter change inside the window changes the drain rate") {
    // 4/4 to tick 7680 (8000 ms, a barline), then 3/4: 1440 ticks, 1500 ms a
    // measure. Two bars from tick 3840 (measure 2) are four measures: two of
    // 4/4 to tick 7680, then two of 3/4 to tick 10560 (11000 ms). One bar is
    // 4000 ms of 4/4 and 3000 ms of 3/4.
    Song song = make_sp_song({960, 1920}, /*last_tick=*/13440, {}, 480, {{7680, {3, 4}}});
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/10560);
    act.bank_rise_ticks = {960, 1920};
    path.activations = {act};
    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(11000.0));

    CHECK(sp_meter_bars_at(c, 2000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(1.5));   // 2000 ms of 4/4
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.25));
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(1.0));   // the meter change
    CHECK(sp_meter_bars_at(c, 8750.0) == doctest::Approx(0.75));  // 750 ms of 3/4
    CHECK(sp_meter_bars_at(c, 9500.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, std::nextafter(11000.0, -1e300)) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 11000.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.0));
}

// D15 and D42. The engine stores each SP end as a whole tick, dropping the
// fraction of a measure (SongTiming::plusmeasure). The gauge drains at
// exactly one bar per two measures to that stored end, so inside the window
// it may sit up to one tick's worth of drain per stored SP-end step below a
// gauge that starts at exactly the banked bars. Between windows it is exact.
TEST_CASE("sp meter curve: a stored SP end that drops a fraction of a tick") {
    // 120 BPM, 480 ticks a beat; 4/4 to tick 7680 (8000 ms), then 3/4 (1440
    // ticks a measure). The activation is two ticks into measure 3: measure
    // 2 + 2/1920. Two bars later is measure 6 + 2/1920, which in 3/4 is tick
    // 7680 + (2 + 2/1920) x 1440 = 10561.5. The engine stores 10561.
    Song song(480);
    song.bpm_changes[0] = 120.0;
    apply_timesig(song, 7680, 3, 4);
    song.build_timing();
    for (int64_t t : {0, 960, 1920, 3842, 4800, 7680, 9000, 12000, 14000}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord.add_note(NoteColor::Red);
        if (t == 960 || t == 1920) test::mark_phrase_end(ts, t, 480);
        song.sequence.push_back(std::move(ts));
    }
    CHECK(song.timing().plusmeasure(song.timecode(3842), 4).ticks() == 10561);

    Path path;
    Activation act = sp_act_at(song, 3842, /*sp_meter=*/2, /*end_tick=*/10561);
    act.bank_rise_ticks = {960, 1920};
    path.activations = {act};
    path.trailing_bank_ticks = {12000};  // a bar after the window (12500 ms)
    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    REQUIRE(scene.activations.size() == 1);
    // Tick 3842 is 4002.083 ms; tick 10561 is 8000 + 2881 x 500/480 =
    // 11001.042 ms.
    const double act_ms = scene.activations[0].ms;
    const double end_ms = scene.activations[0].sp_end_ms;
    CHECK(act_ms == doctest::Approx(4002.0833333));
    CHECK(end_ms == doctest::Approx(11001.0416667));

    // One tick of drain at the smallest measure here (1440 ticks): a bar is
    // two measures, so 1 / 2880 bars. The window has one stored step.
    const double one_tick = 1.0 / 2880.0;
    // At the activation: the stored end is measure 4 + 2881/1440 = 6.0006944
    // and the activation 2 + 2/1920 = 2.0010417, so 3.9996528 measures are
    // left, 1.9998264 bars. Exactly 2 bars would start at 2.
    const double at_act = sp_meter_bars_at(c, act_ms);
    CHECK(at_act == doctest::Approx(1.99982638889).epsilon(1e-10));
    CHECK(std::fabs(at_act - 2.0) <= one_tick);
    // At the meter change (tick 7680, 8000 ms): 2881/1440 measures left =
    // 1.0003472 bars. Exactly 2 bars less the 1.9989583 measures gone would
    // be 1.0005208.
    const double at_meter = sp_meter_bars_at(c, 8000.0);
    CHECK(at_meter == doctest::Approx(1.00034722222).epsilon(1e-10));
    CHECK(std::fabs(at_meter - 1.0005208) <= one_tick);
    // Empty at the stored end, as both would be.
    CHECK(sp_meter_bars_at(c, std::nextafter(end_ms, -1e300)) == doctest::Approx(0.0));
    // Between windows, exact whole bars: two before, none after, then the
    // trailing bar.
    CHECK(sp_meter_bars_at(c, 3000.0) == 2.0);
    CHECK(sp_meter_bars_at(c, end_ms) == 0.0);
    CHECK(sp_meter_bars_at(c, 12000.0) == 0.0);
    CHECK(sp_meter_bars_at(c, 12500.0) == 1.0);
}

// The gauge on every stored path of every corpus chart, held to what the
// record says, through the record's own facts. At each activation it starts
// at the bars the activation spent (Activation::sp_meter), up to one tick's
// worth of drain lower (D15: the stored end drops a fraction of a tick); it
// is empty at the stored deact node; and between windows it holds whole bars,
// never over the cap. Without a path it holds whole bars that never fall.
TEST_CASE("sp meter curve: every corpus gauge starts at its spent bars and empties at its deact node") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int charts = 0, windows = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules);
        if (song.is_empty()) continue;
        ++charts;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);

        const PreviewScene bare = build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules);
        check_curve_well_formed(bare.sp_meter);
        double prev = 0.0;
        for (const SpMeterSegment& s : bare.sp_meter.segments) {
            CHECK(s.start_bars == s.end_bars);
            CHECK(s.start_bars == std::floor(s.start_bars));
            CHECK(s.start_bars >= prev);
            prev = s.start_bars;
        }

        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
            const SpMeterCurve& c = scene.sp_meter;
            check_curve_well_formed(c);
            int64_t min_tpm = std::numeric_limits<int64_t>::max();
            for (const PreviewMeter& m : scene.meters) min_tpm = std::min(min_tpm, m.tpm);
            REQUIRE(min_tpm > 0);
            const double one_tick = 1.0 / (static_cast<double>(kMeasuresPerSpBar) *
                                           static_cast<double>(min_tpm));
            size_t i = 0;
            for (const Activation& a : p->walk_activations()) {
                const PreviewActivation& pa = scene.activations.at(i++);
                REQUIRE(pa.has_sp_end);
                ++windows;
                const double at_act = sp_meter_bars_at(c, pa.ms);
                CHECK(at_act <= a.sp_meter() + 1e-9);
                CHECK(at_act >= a.sp_meter() - one_tick - 1e-9);
                CHECK(sp_meter_bars_at(c, std::nextafter(pa.sp_end_ms, -1e300)) < 1e-6);
            }
            for (const SpMeterSegment& s : c.segments) {
                bool in_window = false;
                for (const PreviewActivation& pa : scene.activations)
                    if (s.end_ms > pa.ms && s.start_ms < pa.sp_end_ms) in_window = true;
                if (in_window) continue;
                CHECK(s.start_bars == s.end_bars);
                CHECK(s.start_bars == std::floor(s.start_bars));
            }
        }
    }
    CHECK(charts > 0);
    CHECK(windows > charts);
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
