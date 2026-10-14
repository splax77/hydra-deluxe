// Tests for app/preview_view's scene: the note-highway view-model the Preview
// tab renders. A hand-built song pins the note/lane/attribute mapping and the
// shaded spans exactly; the corpus cases confirm the SP-phrase-start the parser
// now keeps, that an analyzed chart's overlay lines up with its path, and that
// the base plus the overlay equals the whole scene. The SP meter has its own
// file (test_sp_meter_curve.cpp), and so do the overlay boxes
// (test_preview_boxes.cpp).

#include "doctest.h"

#include <algorithm>
#include <cstddef>
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
#include "corpus_util.h"
#include "parse/song.h"
#include "preview_song_fixtures.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"

using namespace hydra;
using namespace hydra::app;
using namespace hydra::preview_fixtures;

TEST_CASE("build_preview_scene: notes carry lane and drum attributes") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);

    REQUIRE(scene.has_notes);
    REQUIRE(scene.notes.size() == 5);  // 1 + 1 + 1 + 2 (Kick+Green)

    const PreviewNote& red = scene.notes[0];
    CHECK(red.lane == PreviewLane::Red);
    CHECK(red.tick == 0);
    CHECK(red.ms == doctest::Approx(0.0));

    const PreviewNote& yellow = scene.notes[1];
    CHECK(yellow.lane == PreviewLane::Yellow);
    CHECK(yellow.cymbal);
    CHECK(yellow.ghost);
    CHECK(yellow.ms == doctest::Approx(250.0));

    const PreviewNote& blue = scene.notes[2];
    CHECK(blue.lane == PreviewLane::Blue);
    CHECK(blue.accent);

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
    // section there is no span, though the notes still carry the solo flag.
    song.solo_sections.clear();
    PreviewScene none = build_preview_scene(song, nullptr);
    CHECK(none.solos.empty());
}

TEST_CASE("build_preview_scene: drum lanes come from the song's drum lanes") {
    Song song = make_hand_song();
    CHECK(build_preview_scene(song, nullptr).drum_lanes.empty());

    song.drum_lanes = {{240, 720, true, {NoteColor::Yellow, NoteColor::Green}},
                       {720, 960, false, {NoteColor::Red}}};
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.drum_lanes.size() == 2);
    CHECK(scene.drum_lanes[0].span.start_tick == 240);
    CHECK(scene.drum_lanes[0].span.end_tick == 720);
    CHECK(scene.drum_lanes[0].span.start_ms == doctest::Approx(250.0));
    CHECK(scene.drum_lanes[0].span.end_ms == doctest::Approx(750.0));
    CHECK(scene.drum_lanes[0].pads ==
          std::vector<PreviewLane>{PreviewLane::Yellow, PreviewLane::Green});
    CHECK(scene.drum_lanes[1].span.start_tick == 720);
    CHECK(scene.drum_lanes[1].pads == std::vector<PreviewLane>{PreviewLane::Red});
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
    SongTiming st(480, tpm, bpm, {{0, {4, 4}}});

    std::vector<PreviewBeat> beats = build_beat_events(st, 1920);
    // ticks: 0 Bar, 240 Half, 480 Beat, 720 Half, 960 Beat, 1200 Half,
    // 1440 Beat, 1680 Half, 1920 Bar  (no Half before tick 0)
    REQUIRE(beats.size() == 9);
    CHECK(beats[0].tick == 0);
    CHECK(beats[0].kind == PreviewBeatKind::MeasureLine);
    CHECK(beats[1].tick == 240);
    CHECK(beats[1].kind == PreviewBeatKind::Half);
    CHECK(beats[2].tick == 480);
    CHECK(beats[2].kind == PreviewBeatKind::Beat);
    CHECK(beats[6].tick == 1440);
    CHECK(beats[6].kind == PreviewBeatKind::Beat);
    CHECK(beats[8].tick == 1920);
    CHECK(beats[8].kind == PreviewBeatKind::MeasureLine);
    CHECK(beats[8].ms == doctest::Approx(2000.0));
    for (size_t i = 1; i < beats.size(); ++i) CHECK(beats[i].tick > beats[i - 1].tick);
}

TEST_CASE("build_beat_events: a 3/4 section changes the beat count per bar") {
    // One bar of 4/4, then 3/4 (1440 ticks per measure).
    std::map<int64_t, int64_t> tpm{{0, 1920}, {1920, 1440}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming st(480, tpm, bpm, {{0, {4, 4}}, {1920, {3, 4}}});

    std::vector<PreviewBeat> beats = build_beat_events(st, 1920 + 1440 * 2);
    int bars = 0, quarter_beats = 0;
    for (const PreviewBeat& b : beats) {
        if (b.kind == PreviewBeatKind::MeasureLine) ++bars;
        if (b.kind == PreviewBeatKind::Beat) ++quarter_beats;
    }
    CHECK(bars == 4);              // ticks 0, 1920, 3360, 4800
    CHECK(quarter_beats == 3 + 2 + 2);  // 3 in the 4/4 bar, 2 in each 3/4 bar
    // Every Beat line lies strictly inside its bar.
    bool saw_3360 = false;
    for (const PreviewBeat& b : beats)
        if (b.tick == 3360) saw_3360 = b.kind == PreviewBeatKind::MeasureLine;
    CHECK(saw_3360);
}

TEST_CASE("build_beat_events: 6/8 steps by the eighth note") {
    // 6/8 at 480 ticks/quarter: Clone Hero's grid steps every 240 ticks and
    // puts a measure line every six steps. Its weak beats (the 2nd, 4th and
    // 6th eighth) are drawn as ordinary beat lines, and the fainter line sits
    // half a step (120 ticks) before each line.
    std::map<int64_t, int64_t> tpm{{0, 1440}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming st(480, tpm, bpm, {{0, {6, 8}}});

    std::vector<PreviewBeat> beats = build_beat_events(st, 1440);
    std::vector<int64_t> lines, beat_lines, halves;
    for (const PreviewBeat& b : beats) {
        if (b.kind == PreviewBeatKind::MeasureLine) lines.push_back(b.tick);
        if (b.kind == PreviewBeatKind::Beat) beat_lines.push_back(b.tick);
        if (b.kind == PreviewBeatKind::Half) halves.push_back(b.tick);
    }
    CHECK(lines == std::vector<int64_t>{0, 1440});
    CHECK(beat_lines == std::vector<int64_t>{240, 480, 720, 960, 1200});
    CHECK(halves == std::vector<int64_t>{120, 360, 600, 840, 1080, 1320});
    for (size_t i = 1; i < beats.size(); ++i) CHECK(beats[i].tick > beats[i - 1].tick);
}

TEST_CASE("build_beat_events: the fainter line is halfway back to the line before, at a restart too") {
    // 4/4, then 6/8 from tick 1920, then 4/4 again from tick 2400. The grid
    // restarts at each change, so the gap before a restart line is the step
    // of the meter that ends there: 480 before 1920, 240 before 2400.
    std::map<int64_t, int64_t> tpm{{0, 1920}, {1920, 1440}, {2400, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming st(480, tpm, bpm, {{0, {4, 4}}, {1920, {6, 8}}, {2400, {4, 4}}});

    std::vector<PreviewBeat> beats = build_beat_events(st, 2880);
    std::vector<int64_t> halves;
    for (const PreviewBeat& b : beats)
        if (b.kind == PreviewBeatKind::Half) halves.push_back(b.tick);
    CHECK(halves == std::vector<int64_t>{240, 720, 1200, 1680, 2040, 2280, 2640});
    for (size_t i = 1; i < beats.size(); ++i) CHECK(beats[i].tick > beats[i - 1].tick);
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
    CHECK(scene.beats.back().kind == PreviewBeatKind::MeasureLine);
    CHECK(scene.beats.back().ms == doctest::Approx(6000.0));
    // The base alone gives the same grid: the overlay never touches beats.
    CHECK(build_preview_base(c.song, c.audio_end_ms).beats.back().tick == 11520);
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
        Song song = load_songpath(path, ChartReading{});
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
    const AnalysisResult& r = corpus::analyzed_by_ten_scores();
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
        std::optional<int64_t> d = a.deact_tick();
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
                               x.accent == y.accent && x.double_kick == y.double_kick;
                    }))
        return "notes";
    if (!spans_eq(a.sp_phrases, b.sp_phrases)) return "sp_phrases";
    if (!spans_eq(a.solos, b.solos)) return "solos";
    if (!std::equal(a.drum_lanes.begin(), a.drum_lanes.end(), b.drum_lanes.begin(),
                    b.drum_lanes.end(), [&](const PreviewDrumLane& x, const PreviewDrumLane& y) {
                        return span_eq(x.span, y.span) && x.pads == y.pads;
                    }))
        return "drum_lanes";
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
    if (!std::equal(a.measure_spans.begin(), a.measure_spans.end(), b.measure_spans.begin(),
                    b.measure_spans.end(),
                    [](const PreviewMeasureSpan& x, const PreviewMeasureSpan& y) {
                        return x.tick == y.tick && x.tpm == y.tpm;
                    }))
        return "measure_spans";
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
            if (mx.restart_at(i) != my.restart_at(i) || mx.step_at(i) != my.step_at(i) ||
                mx.numerator_at(i) != my.numerator_at(i) ||
                mx.denominator_at(i) != my.denominator_at(i) ||
                mx.lines_before_at(i) != my.lines_before_at(i))
                return "timing (meter section " + std::to_string(i) + ")";
        const MeasureCount& cx = x.measure_count();
        const MeasureCount& cy = y.measure_count();
        if (cx.count() != cy.count()) return "timing (measure count spans)";
        for (int i = 0; i < cx.count(); ++i)
            if (cx.start_at(i) != cy.start_at(i) || cx.tpm_at(i) != cy.tpm_at(i))
                return "timing (measure count span " + std::to_string(i) + ")";
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
        const Song& song = corpus::song(chart, cfg.reading());
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

TEST_CASE("path_overlay_key: no overlay, the same path, and a changed path") {
    // The key is how the Preview notices the Paths tab picked a different
    // path; Path itself has no operator==.
    CHECK(path_overlay_key(nullptr).empty());

    const AnalysisResult& r = corpus::analyzed_by_ten_scores();
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

// The Preview lights a stored passed-over fill by matching its tick to a fill
// in the scene, and drops a tick that matches none. A miss would mean the
// record and the chart disagree, so on the corpus there must be none.
TEST_CASE("preview fills: every stored passed-over fill is a fill in the scene") {
    // The app's default settings, as "base + overlay" reads the corpus.
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int ticks_checked = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
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
