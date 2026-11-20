// Tests for app/preview_view's scene: the note-highway view-model the Preview
// tab renders. Each drawn note, span and fill state is checked against the
// Song's own data and the engine's paths, on the hand song and on the
// corpus charts, and the base plus the overlay equals the whole scene. The SP meter has its own
// file (test_sp_meter_curve.cpp), and so do the overlay boxes
// (test_preview_boxes.cpp).

#include "doctest.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
#include "app/path_view.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/scoring.h"  // solo_note_count: the solo box's note count
#include "core/timing.h"   // MeasureIndex: Clone Hero's measure grid
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

// Each of a song's chord notes, in the order the chord lists them, becomes
// one drawn note on the lane of its colour, at its chord's tick and ms, with
// the chord note's own cymbal, ghost, accent and 2x marks.
static void check_notes_follow_song(const Song& song, const PreviewScene& scene) {
    size_t k = 0;
    for (const SongTimestamp& ts : song.sequence) {
        for (const ChordNote& n : ts.chord.note_list()) {
            REQUIRE(k < scene.notes.size());
            const PreviewNote& pn = scene.notes[k++];
            CHECK(pn.tick == ts.timecode.ticks());
            CHECK(pn.ms == doctest::Approx(ts.timecode.ms()));
            CHECK(pn.lane == lane_of(n.colortype));
            CHECK(pn.cymbal == n.is_cymbal());
            CHECK(pn.ghost == n.is_ghost());
            CHECK(pn.accent == n.is_accent());
            CHECK(pn.double_kick == n.is2x);
        }
    }
    CHECK(k == scene.notes.size());
}

TEST_CASE("build_preview_scene: notes carry lane and drum attributes") {
    // The hand song holds a cymbal, a ghost, an accent and a 2x kick, so each
    // mark is drawn at least once.
    const Song song = make_hand_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.has_notes);
    check_notes_follow_song(song, scene);
    CHECK(std::any_of(scene.notes.begin(), scene.notes.end(),
                      [](const PreviewNote& n) { return n.cymbal; }));
    CHECK(std::any_of(scene.notes.begin(), scene.notes.end(),
                      [](const PreviewNote& n) { return n.ghost; }));
    CHECK(std::any_of(scene.notes.begin(), scene.notes.end(),
                      [](const PreviewNote& n) { return n.accent; }));
    CHECK(std::any_of(scene.notes.begin(), scene.notes.end(),
                      [](const PreviewNote& n) { return n.double_kick; }));

    // The same holds note for note on every corpus chart.
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& real = corpus::song(chart, cfg.reading());
        if (real.is_empty()) continue;
        check_notes_follow_song(real, build_preview_scene(real, nullptr));
    }
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
// one there is no length, and the beat lines end two measures past the last
// drawn note.
TEST_CASE("build_preview_scene: the song length is the one handed in, never a note's time") {
    Song song = make_hand_song();
    SongTimestamp empty;  // a timestamp with no notes, after the last chord
    empty.timecode = song.timecode(960);
    song.sequence.push_back(empty);

    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.has_notes);
    CHECK(last_note_ms(scene) == doctest::Approx(750.0));
    CHECK(scene.song_length_ms == 0.0);
    const PreviewScene before = build_preview_scene(make_hand_song(), nullptr);
    REQUIRE_FALSE(scene.beats.empty());
    REQUIRE_FALSE(before.beats.empty());
    CHECK(scene.beats.back().tick == before.beats.back().tick);
}

// Each chord that ends an SP phrase draws one phrase span, in chart order,
// from the start tick the parser kept on it to the chord itself.
static void check_phrases_follow_song(const Song& song, const PreviewScene& scene) {
    size_t k = 0;
    for (const SongTimestamp& ts : song.sequence) {
        if (!ts.flag_sp || !ts.sp_phrase_start.has_value()) continue;
        REQUIRE(k < scene.sp_phrases.size());
        const PreviewSpan& span = scene.sp_phrases[k++];
        CHECK(span.start_tick == *ts.sp_phrase_start);
        CHECK(span.end_tick == ts.timecode.ticks());
        CHECK(span.start_ms == doctest::Approx(song.timecode(*ts.sp_phrase_start).ms()));
        CHECK(span.end_ms == doctest::Approx(ts.timecode.ms()));
    }
    CHECK(k == scene.sp_phrases.size());
}

TEST_CASE("build_preview_scene: SP phrase spans run from the phrase's start to its end chord") {
    const Song song = make_hand_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE_FALSE(scene.sp_phrases.empty());
    check_phrases_follow_song(song, scene);

    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int phrases = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& real = corpus::song(chart, cfg.reading());
        if (real.is_empty()) continue;
        const PreviewScene s = build_preview_base(real);
        check_phrases_follow_song(real, s);
        phrases += static_cast<int>(s.sp_phrases.size());
    }
    // The corpus charts carry SP phrases, so none drawn would mean a lost span.
    CHECK(phrases > 0);
}

// A Clone Hero 1.1 generated fill draws as the half measure of Clone Hero's
// grid before its chord, whatever its engine length. An authored fill, and a
// Clone Hero 1.0 generated fill, draw their own engine span: from
// fill_start_tick to the chord.
static void check_fill_spans_follow_song(const Song& song, const PreviewScene& scene,
                                         int& half_measure_fills, int& own_span_fills) {
    const MeasureIndex& grid = song.timing().measure_index();
    size_t k = 0;
    for (const SongTimestamp& ts : song.sequence) {
        if (!ts.has_activation()) continue;
        REQUIRE(k < scene.fills.size());
        const PreviewFill& f = scene.fills[k++];
        const int64_t chord = ts.timecode.ticks();
        CHECK(f.chord_tick == chord);
        CHECK(f.span.end_tick == chord);
        if (song.generated_fills == FillDeadlineRule::Ch11) {
            const int64_t measure = grid.length_at(grid.span_at(chord));
            CHECK(chord - f.span.start_tick == measure / 2);
            ++half_measure_fills;
        } else {
            CHECK(f.span.start_tick == fill_start_tick(chord, *ts.activation_length));
            ++own_span_fills;
        }
    }
    CHECK(k == scene.fills.size());
}

TEST_CASE("build_preview_scene: a generated fill draws the half measure before its chord") {
    // Each corpus chart read with Clone Hero 1.1's fill rule (kTestFillRule)
    // and again with 1.0's. Pitch Dark's fills are generated, and
    // test_fast_tempo's charts write their own.
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int ch11_half = 0, ch11_own = 0, ch10_half = 0, ch10_own = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& ch11 = corpus::song(chart, cfg.reading());
        if (ch11.is_empty()) continue;
        check_fill_spans_follow_song(ch11, build_preview_base(ch11), ch11_half, ch11_own);
        if (ch11.generated_fills != FillDeadlineRule::Ch11) continue;
        const Song ch10 =
            load_songpath(chart, cfg.reading(), core::default_rules(), FillDeadlineRule::Ch10);
        CHECK(ch10.generated_fills == FillDeadlineRule::Ch10);
        check_fill_spans_follow_song(ch10, build_preview_base(ch10), ch10_half, ch10_own);
    }
    CHECK(ch11_half > 0);
    CHECK(ch11_own > 0);
    CHECK(ch10_own > 0);
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
    CHECK(none.solo_notes.empty());
}

TEST_CASE("build_preview_scene: each solo's notes for the solo box, path-free") {
    // One step per chord of the song's one solo section, at the chord's ms,
    // holding the solo notes counted up to and including that chord.
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(song.solo_sections.size() == 1);
    REQUIRE(scene.solo_notes.size() == 1);
    const SoloSection& section = song.solo_sections[0];
    const PreviewSoloNotes& s = scene.solo_notes[0];
    REQUIRE(s.steps.size() == section.last - section.first + 1);
    int counted = 0;
    for (size_t i = section.first; i <= section.last; ++i) {
        const SongTimestamp& ts = song.sequence[i];
        counted += solo_note_count(ts.chord, ts.flag_solo);
        CHECK(s.steps[i - section.first].ms == doctest::Approx(ts.timecode.ms()));
        CHECK(s.steps[i - section.first].notes_hit == counted);
    }
    CHECK(counted > 0);
    CHECK(s.notes == counted);
    // The bonus is the path's own solo score (Path::score_solo, priced by the
    // replay), with or without a path in the scene.
    Path path = priced_path(song, {});
    CHECK(s.bonus > 0);
    CHECK(s.bonus == path.score_solo);
    CHECK(build_preview_scene(song, &path).solo_notes[0].bonus == s.bonus);
}

TEST_CASE("build_preview_scene: drum lanes come from the song's drum lanes") {
    Song song = make_hand_song();
    CHECK(build_preview_scene(song, nullptr).drum_lanes.empty());

    song.drum_lanes = {{240, 720, true, {NoteColor::Yellow, NoteColor::Green}},
                       {720, 960, false, {NoteColor::Red}}};
    PreviewScene scene = build_preview_scene(song, nullptr);
    // One drawn lane per song lane, in order: the lane's own span, at its
    // ticks' ms, lighting the pad column of each colour the song lane names.
    REQUIRE(scene.drum_lanes.size() == song.drum_lanes.size());
    for (size_t i = 0; i < song.drum_lanes.size(); ++i) {
        const DrumLane& want = song.drum_lanes[i];
        const PreviewDrumLane& got = scene.drum_lanes[i];
        CHECK(got.span.start_tick == want.start_tick);
        CHECK(got.span.end_tick == want.end_tick);
        CHECK(got.span.start_ms == doctest::Approx(song.timecode(want.start_tick).ms()));
        CHECK(got.span.end_ms == doctest::Approx(song.timecode(want.end_tick).ms()));
        std::vector<PreviewLane> pads;
        for (NoteColor c : want.pads) pads.push_back(lane_of(c));
        CHECK(got.pads == pads);
    }
}

TEST_CASE("build_preview_scene: an unanalyzed chart offers every candidate fill") {
    Song song = make_fill_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.fills.size() == 4);
    for (const PreviewFill& f : scene.fills) CHECK(f.state == PreviewFillState::Offered);
}

// The fill states a drawn path gives, read from the path's own record. The
// fill on an activation's chord is Taken, even when a later activation lists
// it as passed over. A fill an activation, or the path after its last
// activation (Path::trailing_skipped_fill_ticks), lists as passed over is
// Offered. Every other fill is Hidden. Each activation names the fill on its
// own chord, or none when no fill ends there. Counts each state it sees.
struct FillStateCounts {
    int taken = 0;
    int offered = 0;
    int hidden = 0;
};

static void check_fill_states_follow_path(const PreviewScene& scene, const Path& path,
                                          FillStateCounts& counts) {
    std::set<int64_t> act_ticks;
    std::set<int64_t> passed(path.trailing_skipped_fill_ticks.begin(),
                             path.trailing_skipped_fill_ticks.end());
    for (const Activation& a : path.walk_activations()) {
        act_ticks.insert(a.timecode.ticks());
        passed.insert(a.skipped_fill_ticks.begin(), a.skipped_fill_ticks.end());
    }
    for (const PreviewFill& f : scene.fills) {
        CAPTURE(f.chord_tick);
        if (act_ticks.count(f.chord_tick) != 0) {
            CHECK(f.state == PreviewFillState::Taken);
            ++counts.taken;
        } else if (passed.count(f.chord_tick) != 0) {
            CHECK(f.state == PreviewFillState::Offered);
            ++counts.offered;
        } else {
            CHECK(f.state == PreviewFillState::Hidden);
            ++counts.hidden;
        }
    }
    REQUIRE(scene.activations.size() == path.walk_activations().size());
    for (const PreviewActivation& a : scene.activations) {
        CAPTURE(a.tick);
        std::optional<size_t> on_chord;
        for (size_t i = 0; i < scene.fills.size(); ++i)
            if (scene.fills[i].chord_tick == a.tick) on_chord = i;
        CHECK(a.taken_fill == on_chord);
    }
}

// On every path the engine keeps for a corpus chart, each fill reads Taken,
// Offered or Hidden as that path's record says, and each activation names
// the fill it stands on. The corpus holds all three states: an activation
// always stands on a fill, "every stored passed-over fill is a fill in the
// scene" below finds stored passed-over fills, and fills before Star Power
// is first ready are hidden.
TEST_CASE("build_preview_scene: each fill's state and each activation's fill follow the path's record") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    FillStateCounts counts;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        for (const Path* path : rec.all_paths()) {
            CAPTURE(path->pathstring());
            check_fill_states_follow_path(build_preview_scene(song, path), *path, counts);
        }
    }
    CHECK(counts.taken > 0);
    CHECK(counts.offered > 0);
    CHECK(counts.hidden > 0);
}

// Under Clone Hero 1.0's fill rule a fill's deadline can fall before an
// earlier fill's. In make_ch10_fill_song the second fill's deadline falls
// before the first's, so the second never spawns. Searched to activate on
// the third fill, the engine lists only the first as passed over, and the
// Preview draws the first offered, the second hidden and the third taken.
TEST_CASE("build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged") {
    Song song = test::make_ch10_fill_song();
    std::vector<int64_t> fill_chords;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation()) fill_chords.push_back(ts.timecode.ticks());
    REQUIRE(fill_chords.size() == 3);

    ScoreGraph graph(song, 4, FillDeadlineRule::Ch10);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{fill_chords[2]};
    std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    const Path& path = paths.front();
    REQUIRE(test::act_ticks(path) == std::vector<int64_t>{fill_chords[2]});
    CHECK(path.walk_activations()[0].skipped_fill_ticks == std::vector<int64_t>{fill_chords[0]});

    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.fills.size() == fill_chords.size());
    for (size_t i = 0; i < fill_chords.size(); ++i) CHECK(scene.fills[i].chord_tick == fill_chords[i]);
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
    // One bar of 4/4, then two bars of 3/4 (1440 ticks per measure).
    const std::map<int64_t, std::pair<int, int>> sigs{{0, {4, 4}}, {1920, {3, 4}}};
    const std::map<int64_t, int64_t> tpm{{0, 1920}, {1920, 1440}};
    const std::map<int64_t, double> bpm{{0, 120.0}};
    const SongTiming st(480, tpm, bpm, sigs);
    const int64_t end = 1920 + 1440 * 2;

    const std::vector<PreviewBeat> beats = build_beat_events(st, end);
    std::vector<int64_t> bar_lines, beat_lines;
    for (const PreviewBeat& b : beats) {
        if (b.kind == PreviewBeatKind::MeasureLine) bar_lines.push_back(b.tick);
        if (b.kind == PreviewBeatKind::Beat) beat_lines.push_back(b.tick);
    }

    // A measure line opens every bar the measure lengths lay out, from each
    // length change on, and one more sits on the end, closing the last bar.
    std::vector<int64_t> expected_bars;
    for (auto it = tpm.begin(); it != tpm.end(); ++it) {
        const bool last = std::next(it) == tpm.end();
        const int64_t stop = last ? end : std::next(it)->first;
        for (int64_t t = it->first; last ? t <= stop : t < stop; t += it->second)
            expected_bars.push_back(t);
    }
    CHECK(bar_lines == expected_bars);

    // Inside each bar its beat lines step by one beat of the time signature
    // in force at the bar's line: numerator - 1 of them after the line.
    std::vector<int64_t> expected_beats;
    for (size_t i = 0; i + 1 < bar_lines.size(); ++i) {
        const std::pair<int, int> sig = std::prev(sigs.upper_bound(bar_lines[i]))->second;
        const int64_t step = st.tick_resolution() * 4 / sig.second;
        for (int k = 1; k < sig.first; ++k) expected_beats.push_back(bar_lines[i] + k * step);
    }
    CHECK(beat_lines == expected_beats);
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

// Where the beat lines stop is the D266 test's, below.
TEST_CASE("build_preview_scene: the scene carries the song's resolution and every tempo") {
    const Song song = make_hand_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.timing.has_value());
    CHECK(scene.timing->tick_resolution() == song.timing().tick_resolution());
    REQUIRE(scene.tempos.size() == song.bpm_changes.size());
    size_t i = 0;
    for (const auto& change : song.bpm_changes) {
        CHECK(scene.tempos[i].tick == change.first);
        CHECK(scene.tempos[i].ms == doctest::Approx(song.timecode(change.first).ms()));
        CHECK(scene.tempos[i].bpm == change.second);
        ++i;
    }
}

// The beat lines stop where playback stops: at the song's length, never at
// the audio's end (D266). The audio-tail chart's last note is at tick 1920
// (1000 ms) and its audio stops 5 s later, at 6000 ms. A beat is 480 ticks
// (250 ms) and a measure 1920 ticks (1000 ms).
TEST_CASE("build_preview_scene: the beat lines stop at the song's length (D266)") {
    const test::AudioTailChart c = test::audio_tail_chart();

    SUBCASE("a length that agrees with the audio's end draws the grid it always did") {
        // 6000 ms is tick 11520, six measures in: the last line is that barline.
        const PreviewScene scene = build_preview_scene(c.song, nullptr, kCloneHeroSpCap,
                                                       core::default_rules(), c.audio_end_ms);
        REQUIRE(!scene.beats.empty());
        CHECK(scene.beats.back().tick == 11520);
        CHECK(scene.beats.back().kind == PreviewBeatKind::MeasureLine);
        CHECK(scene.beats.back().ms == doctest::Approx(6000.0));
        // The base alone gives the same grid: the overlay never touches beats.
        CHECK(build_preview_base(c.song, c.audio_end_ms).beats.back().tick == 11520);
    }

    SUBCASE("music past the song's length draws no lines past it") {
        // A 4000 ms song with 6000 ms of audio. The barline at tick 7680 sits
        // exactly on the length, so it is the last line drawn.
        const PreviewScene at = build_preview_base(c.song, 4000.0);
        REQUIRE(!at.beats.empty());
        CHECK(at.beats.back().tick == 7680);
        CHECK(at.beats.back().kind == PreviewBeatKind::MeasureLine);
        CHECK(at.beats.back().ms == 4000.0);
        for (const PreviewBeat& b : at.beats) CHECK(b.ms <= at.song_length_ms);

        // A tenth of a millisecond shorter: a playhead there shows tick 7680,
        // but that barline is past the end, so the last line is the half-beat
        // before it, at tick 7440 (3875 ms).
        const PreviewScene before = build_preview_base(c.song, 3999.9);
        REQUIRE(!before.beats.empty());
        CHECK(before.song_length_ms == 3999.9);
        CHECK(before.beats.back().tick == 7440);
        CHECK(before.beats.back().kind == PreviewBeatKind::Half);
        for (const PreviewBeat& b : before.beats) CHECK(b.ms <= before.song_length_ms);
    }
}

TEST_CASE("parser keeps the SP-phrase start on the flagged note, on every corpus chart") {
    // Every note the parser flags as an SP-phrase end, on every corpus chart
    // at the app's default reading, carries its phrase's start tick, at or
    // before the note (close_sp_phrase flags only a chord inside the phrase).
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int sp_notes = 0;
    for (const std::string& path : corpus::chart_paths()) {
        for (const SongTimestamp& ts : corpus::song(path, cfg.reading()).sequence) {
            if (!ts.flag_sp) continue;
            ++sp_notes;
            INFO(path << " | SP-phrase end at tick " << ts.timecode.ticks());
            REQUIRE(ts.sp_phrase_start.has_value());
            CHECK(*ts.sp_phrase_start <= ts.timecode.ticks());
        }
    }
    // With no flagged note anywhere the loop above checked nothing.
    CHECK(sp_notes > 0);
}

namespace {

// The corpus charts with drum notes at `cfg`'s reading, in corpus order. A
// chart without them has no record: analyzing it throws NoNotesError
// (require_notes), and it is named here rather than passed over quietly.
// At least one chart has notes, or no caller would check anything.
std::vector<std::string> charts_with_notes(const AnalysisSettings& cfg) {
    std::vector<std::string> out;
    for (const std::string& chart : corpus::chart_paths()) {
        if (!corpus::song(chart, cfg.reading()).is_empty()) {
            out.push_back(chart);
            continue;
        }
        MESSAGE("no notes at this reading, so no record: " << chart);
        CHECK_THROWS_AS(corpus::analyzed(chart, cfg), NoNotesError);
    }
    REQUIRE_FALSE(out.empty());
    return out;
}

}  // namespace

// Each activation a stored path walks is one overlay entry, in walk order,
// carrying the record's own values: its tick and ms, every SP-end step as
// stored, the window end (Activation::window_end_tick) in the song's ms, the
// lane of the chord's activation note and the chord, the fills it passed
// over and its notation. Every corpus chart with notes and every stored path,
// at the app's default settings. The same song with no path has the same
// notes and no overlay.
TEST_CASE("build_preview_scene: each overlay activation carries its record's values, on every corpus path") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int activations_checked = 0;
    for (const std::string& chart : charts_with_notes(cfg)) {
        const Song& song = corpus::song(chart, cfg.reading());
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const PreviewScene bare = build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules);
        CHECK(bare.activations.empty());
        for (const Path* p : rec.all_paths()) {
            const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
            CHECK(scene.notes.size() == bare.notes.size());
            const ActivationWalk acts = p->walk_activations();
            REQUIRE(scene.activations.size() == acts.size());
            for (size_t i = 0; i < acts.size(); ++i) {
                INFO(chart << " | path " << p->pathstring() << " | activation " << i);
                const Activation& a = acts[i];
                const PreviewActivation& pa = scene.activations[i];
                ++activations_checked;
                CHECK(pa.tick == a.timecode.ticks());
                CHECK(pa.ms == doctest::Approx(a.timecode.ms()));
                CHECK(pa.skipped_fill_ticks == a.skipped_fill_ticks);
                REQUIRE(pa.sp_end_changes.size() == a.sp_end_steps.size());
                for (size_t k = 0; k < a.sp_end_steps.size(); ++k) {
                    CHECK(pa.sp_end_changes[k].at_tick == a.sp_end_steps[k].tick);
                    CHECK(pa.sp_end_changes[k].end_tick == a.sp_end_steps[k].end_tick);
                }
                // The engine stamps an end on every activation it produces,
                // so a fresh record always has one.
                const std::optional<int64_t> d = a.window_end_tick();
                CHECK(d.has_value());
                CHECK(pa.has_sp_end == d.has_value());
                if (d) {
                    CHECK(pa.sp_end_tick == *d);
                    CHECK(pa.sp_end_ms == doctest::Approx(song.timecode(*d).ms()));
                }
                CHECK(pa.has_lane == (a.chord.count() > 0));
                if (a.chord.count() > 0) {
                    CHECK(pa.lane == lane_of(a.chord.activation_note().colortype));
                    CHECK(pa.chord == a.chord);
                }
                CHECK(pa.notation == a.notationstr_verbose());
            }
        }
    }
    // With no activation on any stored path the loop above checked nothing.
    CHECK(activations_checked > 0);
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
    if (!std::equal(a.solo_notes.begin(), a.solo_notes.end(), b.solo_notes.begin(),
                    b.solo_notes.end(), [](const PreviewSoloNotes& x, const PreviewSoloNotes& y) {
                        return x.notes == y.notes && x.bonus == y.bonus &&
                               std::equal(x.steps.begin(), x.steps.end(), y.steps.begin(),
                                          y.steps.end(),
                                          [](const PreviewSoloNotes::Step& p,
                                             const PreviewSoloNotes::Step& q) {
                                              return p.ms == q.ms && p.notes_hit == q.notes_hit;
                                          });
                    }))
        return "solo_notes";
    if (!std::equal(a.drum_lanes.begin(), a.drum_lanes.end(), b.drum_lanes.begin(),
                    b.drum_lanes.end(), [&](const PreviewDrumLane& x, const PreviewDrumLane& y) {
                        return span_eq(x.span, y.span) && x.pads == y.pads;
                    }))
        return "drum_lanes";
    if (!std::equal(a.fills.begin(), a.fills.end(), b.fills.begin(), b.fills.end(),
                    [&](const PreviewFill& x, const PreviewFill& y) {
                        return span_eq(x.span, y.span) && x.chord_tick == y.chord_tick &&
                               x.state == y.state;
                    }))
        return "fills";
    if (!std::equal(a.activations.begin(), a.activations.end(), b.activations.begin(),
                    b.activations.end(),
                    [](const PreviewActivation& x, const PreviewActivation& y) {
                        return x.tick == y.tick && x.ms == y.ms &&
                               x.skipped_fill_ticks == y.skipped_fill_ticks &&
                               x.taken_fill == y.taken_fill && x.notation == y.notation &&
                               x.has_sp_end == y.has_sp_end &&
                               x.sp_end_tick == y.sp_end_tick && x.sp_end_ms == y.sp_end_ms &&
                               x.sp_end_changes == y.sp_end_changes &&
                               x.bank_rise_ticks == y.bank_rise_ticks &&
                               x.lane == y.lane && x.has_lane == y.has_lane &&
                               x.measure == y.measure && x.chord == y.chord;
                    }))
        return "activations";
    if (a.ghost_fills != b.ghost_fills) return "ghost_fills";
    if (a.ghost_windows != b.ghost_windows) return "ghost_windows";
    if (a.trailing_bank_ticks != b.trailing_bank_ticks) return "trailing_bank_ticks";
    if (a.trailing_skipped_fill_ticks != b.trailing_skipped_fill_ticks)
        return "trailing_skipped_fill_ticks";
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
                               x.combo == y.combo &&
                               x.score_without_solo == y.score_without_solo;
                    }))
        return "score";
    if (a.score.cutoffs.chart_base_score != b.score.cutoffs.chart_base_score ||
        a.score.cutoffs.solo_bonus != b.score.cutoffs.solo_bonus ||
        a.score.cutoffs.cutoffs != b.score.cutoffs.cutoffs ||
        a.score.cutoffs.with_solo != b.score.cutoffs.with_solo)
        return "score cutoffs";
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

// The check of check_split with the scenes already built: `whole` is
// build_preview_scene for `path` and `other_scene` is the whole scene for the
// other path. The overlay goes over a copy. (build_preview_scene is itself
// the overlay laid over the song's base, so base + overlay needs no check of
// its own.)
void check_split_over(const Song& song, const Path* path, int sp_cap, const core::Rules& rules,
                      const PreviewScene& whole, const PreviewScene& other_scene,
                      const std::string& what) {
    const std::string relaid =
        scene_difference(whole, apply_preview_overlay(other_scene, song, path, sp_cap, rules));
    INFO(what << ": overlay over another path's scene differs in " << relaid);
    CHECK(relaid.empty());
}

// The overlay for one song and path laid over a scene built for `other`
// (another path, or none), as the Preview does on a path change: it must
// equal build_preview_scene for the path, member by member.
void check_split(const Song& song, const Path* path, int sp_cap, const core::Rules& rules,
                 const Path* other, const std::string& what) {
    check_split_over(song, path, sp_cap, rules, build_preview_scene(song, path, sp_cap, rules),
                     build_preview_scene(song, other, sp_cap, rules), what);
}

}  // namespace

TEST_CASE("overlay over another path's scene: equals build_preview_scene on the hand-built fixtures") {
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

TEST_CASE("overlay over another path's scene: equals build_preview_scene on every corpus chart and stored path") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int paths = 0;
    for (const std::string& chart : charts_with_notes(cfg)) {
        const Song& song = corpus::song(chart, cfg.reading());
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const std::vector<const Path*> all = rec.all_paths();
        // One whole scene per path. Walking the paths from last to first
        // makes the next path's whole the one built on the previous step; the
        // last path's other is the no-path scene.
        const PreviewScene none = build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules);
        PreviewScene next = none;
        for (size_t i = all.size(); i-- > 0;) {
            ++paths;
            // Laid over the next path's scene, so every overlay replaces a
            // different one.
            PreviewScene whole = build_preview_scene(song, all[i], cfg.sp_cap, cfg.rules);
            check_split_over(song, all[i], cfg.sp_cap, cfg.rules, whole, next,
                             chart + " path " + std::to_string(i));
            next = std::move(whole);
        }
        // `next` now holds path 0's whole, or the no-path scene if there is
        // no path.
        check_split_over(song, nullptr, cfg.sp_cap, cfg.rules, none, next, chart + " (no path)");
    }
    // With no stored path anywhere no overlay replaced another path's.
    CHECK(paths > 0);
}

// The key is how the Preview notices the Paths tab picked a different path.
// A path's key is its path_identity, for every path
// (test_path_list.cpp, "the dedupe and the overlay key read one identity"),
// and path_identity's own test covers a copy, a rescore and a trimmed path.
// What is the key's alone is the empty key for no overlay.
TEST_CASE("path_overlay_key: no overlay gives an empty key") {
    CHECK(path_overlay_key(nullptr).empty());
}

// A ghost path lists the fills its activations stand on and changes nothing
// else: the drawn path's fill states and activations are the ones built with
// no ghost. Both paths are the engine's, from one analyzed corpus chart: the
// first path drawn, the last as the ghost.
TEST_CASE("build_preview_scene: a ghost path lists its own taken fills and nothing else") {
    const AnalysisResult& r = corpus::analyzed_by_ten_scores();
    const std::vector<const Path*> paths = r.record.all_paths();
    REQUIRE_FALSE(paths.empty());
    const Path& drawn = *paths.front();
    const Path& ghost = *paths.back();

    // The ghost's fills are the fills it takes when it is the drawn path.
    std::vector<size_t> ghost_taken;
    for (const PreviewActivation& a : build_preview_scene(r.song, &ghost).activations)
        if (a.taken_fill) ghost_taken.push_back(*a.taken_fill);
    REQUIRE_FALSE(ghost_taken.empty());

    const PreviewScene plain = build_preview_scene(r.song, &drawn);
    CHECK(plain.ghost_fills.empty());
    const PreviewScene with = build_preview_scene(r.song, &drawn, kCloneHeroSpCap,
                                                  core::default_rules(), std::nullopt, &ghost);
    CHECK(with.ghost_fills == ghost_taken);
    REQUIRE(with.fills.size() == plain.fills.size());
    for (size_t i = 0; i < plain.fills.size(); ++i)
        CHECK(with.fills[i].state == plain.fills[i].state);
    REQUIRE(with.activations.size() == plain.activations.size());
    for (size_t i = 0; i < plain.activations.size(); ++i)
        CHECK(with.activations[i].taken_fill == plain.activations[i].taken_fill);

    // No drawn path: every fill still reads Offered, as for an unanalyzed chart.
    const PreviewScene alone = build_preview_scene(r.song, nullptr, kCloneHeroSpCap,
                                                   core::default_rules(), std::nullopt, &ghost);
    CHECK(alone.ghost_fills == ghost_taken);
    for (const PreviewFill& f : alone.fills) CHECK(f.state == PreviewFillState::Offered);

    // A later overlay with no ghost clears the list.
    const PreviewScene cleared = apply_preview_overlay(with, r.song, &drawn);
    CHECK(cleared.ghost_fills.empty());
    CHECK(cleared.ghost_windows.empty());
}

// On every corpus chart with notes, at the app's default settings, each
// stored path as the ghost lists, in walk order, the fill whose chord is one
// of its activations' chords, read off the record's activation ticks. They
// are the fills the same path takes when it is the drawn path, and its
// windows are that drawn path's sp_window()s; a scene with no ghost has none.
TEST_CASE("build_preview_scene: every corpus path as the ghost lists the fills its activations stand on") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int ghost_fills_seen = 0;
    for (const std::string& chart : charts_with_notes(cfg)) {
        const Song& song = corpus::song(chart, cfg.reading());
        for (const Path* p : corpus::analyzed(chart, cfg).all_paths()) {
            INFO(chart << " | path " << p->pathstring());
            const PreviewScene scene = build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules,
                                                           std::nullopt, p);
            std::vector<size_t> on_chords;
            for (const Activation& a : p->walk_activations())
                for (size_t i = 0; i < scene.fills.size(); ++i)
                    if (scene.fills[i].chord_tick == a.timecode.ticks()) on_chords.push_back(i);
            CHECK(scene.ghost_fills == on_chords);
            ghost_fills_seen += static_cast<int>(scene.ghost_fills.size());

            const PreviewScene drawn = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
            std::vector<size_t> taken;
            std::vector<std::pair<double, double>> windows;
            for (const PreviewActivation& a : drawn.activations) {
                if (a.taken_fill) taken.push_back(*a.taken_fill);
                if (const auto w = a.sp_window()) windows.push_back(*w);
            }
            CHECK(scene.ghost_fills == taken);
            CHECK(scene.ghost_windows == windows);
            CHECK(drawn.ghost_windows.empty());
        }
    }
    // With no ghost fill on any path the list checks above met only empty lists.
    CHECK(ghost_fills_seen > 0);
}

// The fills after a path's last activation: the engine lists the ones the
// path passed over (Path::trailing_skipped_fill_ticks), and the overlay
// lights exactly those. Every path of "Missed Injections" at the top 40
// scores, cap 4, that activates and lists a passed-over fill after its last
// activation: each fill after that activation reads Offered when its tick is
// in the list and Hidden when it is not.
TEST_CASE("preview scene: fills after the last activation read Offered when the engine lists them, Hidden otherwise") {
    const AnalysisSettings cfg = test::scores_settings(4);
    const std::string chart = corpus::chart_named("Hail The Sun - Missed Injections");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
    int paths_checked = 0;
    for (const Path* path : corpus::analyzed(chart, cfg).all_paths()) {
        const std::vector<int64_t> acts = test::act_ticks(*path);
        if (acts.empty() || path->trailing_skipped_fill_ticks.empty()) continue;
        INFO("path " << path->pathstring());
        ++paths_checked;
        const int64_t last_act = acts.back();
        const std::vector<int64_t>& listed = path->trailing_skipped_fill_ticks;

        const PreviewScene scene = build_preview_scene(song, path, cfg.sp_cap, cfg.rules);
        CHECK(scene.trailing_skipped_fill_ticks == listed);
        for (const PreviewFill& f : scene.fills) {
            if (f.chord_tick <= last_act) continue;
            INFO("fill at " << f.chord_tick);
            const bool is_listed =
                std::find(listed.begin(), listed.end(), f.chord_tick) != listed.end();
            CHECK(f.state == (is_listed ? PreviewFillState::Offered : PreviewFillState::Hidden));
        }

        // The same path without its list draws those fills hidden, as the
        // Preview did before the engine wrote them down.
        Path bare = *path;
        bare.trailing_skipped_fill_ticks.clear();
        const PreviewScene without = build_preview_scene(song, &bare, cfg.sp_cap, cfg.rules);
        for (const PreviewFill& f : without.fills)
            if (f.chord_tick > last_act) CHECK(f.state == PreviewFillState::Hidden);
    }
    // This chart at these settings is the corpus case where a path passes
    // over fills after its last activation; with none, nothing was checked.
    CHECK(paths_checked > 0);
}

// The Preview lights a stored passed-over fill by matching its tick to a fill
// in the scene, and drops a tick that matches none. A miss would mean the
// record and the chart disagree, so on the corpus there must be none.
TEST_CASE("preview fills: every stored passed-over fill is a fill in the scene") {
    // The app's default settings, as "base + overlay" reads the corpus.
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int ticks_checked = 0;
    for (const std::string& chart : charts_with_notes(cfg)) {
        const Song& song = corpus::song(chart, cfg.reading());
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::set<int64_t> fill_ends;
        for (const PreviewFill& f : build_preview_base(song).fills) fill_ends.insert(f.chord_tick);
        // One stored passed-over tick: it must name exactly one fill.
        auto check_tick = [&](size_t root, const std::string& where, int64_t t) {
            ++ticks_checked;
            if (fill_ends.count(t) == 0)
                MESSAGE(chart << " | root " << root << " | " << where << " | stored fill tick "
                              << t);
            CHECK(fill_ends.count(t) == 1);
        };
        for (size_t pi = 0; pi < rec.paths.size(); ++pi) {
            for (const Activation& a : rec.paths[pi].walk_activations())
                for (int64_t t : a.skipped_fill_ticks)
                    check_tick(pi, "activation " + std::to_string(a.timecode.ticks()), t);
            for (int64_t t : rec.paths[pi].trailing_skipped_fill_ticks)
                check_tick(pi, "after the last activation", t);
        }
    }
    MESSAGE("stored passed-over fill ticks checked: " << ticks_checked);
    // With no stored passed-over tick anywhere the check above never ran.
    CHECK(ticks_checked > 0);
}

// Where each bar arrived is stamped by the search (Activation::bank_rise_ticks,
// Path::trailing_bank_ticks). The scene copies those lists and works out none
// of its own, so on every corpus path they match item by item. A squeezed-out
// bar is the first entry of the next activation's list (or of the trailing
// list), so a scene that moved it would fail here.
TEST_CASE("preview scene: each activation's bar arrivals are the record's stored list") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int lists_checked = 0, squeeze_outs = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        for (const Path* p : rec.all_paths()) {
            const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
            const ActivationWalk acts = p->walk_activations();
            REQUIRE(scene.activations.size() == acts.size());
            for (size_t i = 0; i < acts.size(); ++i) {
                INFO(chart << " | path " << p->pathstring() << " | activation " << i);
                CHECK(scene.activations[i].bank_rise_ticks == acts[i].bank_rise_ticks);
                ++lists_checked;
                if (acts[i].sqout_tick) ++squeeze_outs;
            }
            INFO(chart << " | path " << p->pathstring() << " | trailing bars");
            CHECK(scene.trailing_bank_ticks == p->trailing_bank_ticks);
            ++lists_checked;
        }
    }
    MESSAGE("bar lists checked: " << lists_checked << ", squeeze-outs: " << squeeze_outs);
    CHECK(lists_checked > 0);
    // A squeeze-out's bar is the case a second rule would move; the corpus
    // must hold one, or the check above never meets it.
    CHECK(squeeze_outs > 0);
}

// Each button build_path_buttons makes for a corpus record is labelled with
// its own notation. A Within path's label is the notation alone; an Optimal
// and an all-0 path's label is the notation and then a note of its list, and
// the two lists' notes differ.
TEST_CASE("preview path label: the notation and which list it came from") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    const Settings s = Settings();
    std::set<std::string> optimal_notes, allzero_notes;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        if (corpus::song(chart, cfg.reading()).is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        for (const PathButtonView& b : build_path_buttons(rec, s.depth_mode, s.depth_value).buttons) {
            CAPTURE(b.notation);
            const std::string label = preview_path_label(b);
            REQUIRE(label.size() >= b.notation.size());
            CHECK(label.compare(0, b.notation.size(), b.notation) == 0);
            const std::string note = label.substr(b.notation.size());
            switch (b.group) {
                case PathButtonView::Group::Within:
                    CHECK(note.empty());
                    break;
                case PathButtonView::Group::Optimal:
                    CHECK_FALSE(note.empty());
                    optimal_notes.insert(note);
                    break;
                case PathButtonView::Group::AllZero:
                    CHECK_FALSE(note.empty());
                    allzero_notes.insert(note);
                    break;
            }
        }
    }
    // Every list's note is the same on every button of that list.
    CHECK(optimal_notes.size() == 1);
    CHECK(allzero_notes.size() <= 1);
    for (const std::string& n : allzero_notes) CHECK(optimal_notes.count(n) == 0);
}
