// Tests for render/track_state: the instant timeline the highway draws from
// (a port of Onyx's CommonState/DrumState maps), built from a PreviewScene.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <map>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
#include "core/song.h"  // cymbal_counts: the engine's rule for which pads are cymbals
#include "corpus_util.h"
#include "record_fixtures.h"
#include "preview_scene_fixtures.h"
#include "render/track_state.h"

using namespace hydra;
using namespace hydra::app;
using namespace hydra::render;
using namespace hydra::test_fixtures;

namespace {

const TrackInstant* find(const std::vector<TrackInstant>& v, double t) {
    for (const TrackInstant& i : v)
        if (i.t == doctest::Approx(t)) return &i;
    return nullptr;
}

// The instant at `t`, which the case requires to be there.
const TrackInstant& instant_at(const TrackState& st, double t) {
    const TrackInstant* i = find(st.instants(), t);
    INFO("no instant at " << t << " s");
    REQUIRE(i != nullptr);
    return *i;
}

// The drawn end edge, in seconds, of a span whose last note is on `end_tick`:
// kSpanEndTicks past that note through the scene's own timing, the edge
// build_track_state draws (track_state.h, above build_track_state).
double end_edge_s(const PreviewScene& scene, int64_t end_tick) {
    return scene.timing->ms_index().ms_at_tick_f(static_cast<double>(end_tick) + kSpanEndTicks) /
           1000.0;
}

// ---- One instant as a line of text ------------------------------------------
// For comparing two instants field by field and for failure messages. The
// time in seconds, the gems ("R", "Yc" a yellow cymbal, "K" a kick, "g" ghost,
// "a" accent, "-" none), the beat line, then the six span states (od SP
// phrase, so solo, fi offered fill, ft taken fill, sp active SP, ln lit lane)
// and the lit lane's pad. A span state is "." absent, "S" start, "O" on, "R"
// restart, "E" end.

const char* toggle_code(Toggle t) {
    switch (t) {
        case Toggle::Empty:   return ".";
        case Toggle::Start:   return "S";
        case Toggle::On:      return "O";
        case Toggle::Restart: return "R";
        case Toggle::End:     return "E";
    }
    return "?";
}

const char* pad_code(Pad p) {
    switch (p) {
        case Pad::Red:    return "R";
        case Pad::Yellow: return "Y";
        case Pad::Blue:   return "B";
        case Pad::Green:  return "G";
    }
    return "?";
}

std::string instant_line(const TrackInstant& i, bool show_cymbals = true) {
    char t[32];
    std::snprintf(t, sizeof t, "%.6f", i.t);
    std::string s = t;
    s += " ";
    if (i.notes.empty()) s += "-";
    for (size_t k = 0; k < i.notes.size(); ++k) {
        const TrackGem& g = i.notes[k];
        if (k > 0) s += ",";
        s += g.kick ? "K" : pad_code(g.pad);
        if (show_cymbals && g.cymbal) s += "c";
        if (g.velocity == Velocity::Ghost) s += "g";
        if (g.velocity == Velocity::Accent) s += "a";
    }
    s += " ";
    if (!i.beat) s += "-";
    else if (*i.beat == PreviewBeatKind::MeasureLine) s += "bar";
    else if (*i.beat == PreviewBeatKind::Beat) s += "beat";
    else s += "half";
    s += std::string(" od=") + toggle_code(i.overdrive) + " so=" + toggle_code(i.solo) +
         " fi=" + toggle_code(i.fill) + " ft=" + toggle_code(i.fill_taken) +
         " sp=" + toggle_code(i.sp_active) + " ln=" + toggle_code(i.fill_lane) +
         " pad=" + (i.fill_lane_pad ? pad_code(*i.fill_lane_pad) : "-");
    return s;
}

// Every field of an instant: time, gems in order, beat, every span state.
bool same_gem(const TrackGem& a, const TrackGem& b) {
    return a.kick == b.kick && (a.kick || a.pad == b.pad) && a.cymbal == b.cymbal &&
           a.velocity == b.velocity;
}

bool same_instant(const TrackInstant& a, const TrackInstant& b) {
    if (a.t != b.t || a.notes.size() != b.notes.size() || a.beat != b.beat) return false;
    for (size_t i = 0; i < a.notes.size(); ++i)
        if (!same_gem(a.notes[i], b.notes[i])) return false;
    return a.overdrive == b.overdrive && a.solo == b.solo && a.fill == b.fill &&
           a.fill_taken == b.fill_taken && a.fill_ghost == b.fill_ghost &&
           a.sp_active == b.sp_active &&
           a.fill_lane == b.fill_lane && a.fill_lane_pad == b.fill_lane_pad &&
           a.drum_lane_red == b.drum_lane_red && a.drum_lane_yellow == b.drum_lane_yellow &&
           a.drum_lane_blue == b.drum_lane_blue && a.drum_lane_green == b.drum_lane_green;
}

// The app's default analysis settings, which the corpus walks analyse under.
const app::AnalysisSettings& default_cfg() {
    static const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    return cfg;
}

// The song of a corpus chart. Every chart the corpus scan lists has Expert
// drum notes, so a chart that loads empty fails the test by name instead of
// being passed over.
const Song& corpus_song_with_notes(const std::string& chart) {
    const Song& song = corpus::song(chart, default_cfg().reading());
    INFO("corpus chart " << chart);
    REQUIRE_FALSE(song.is_empty());
    return song;
}

// One corpus chart and the Preview's scene for it: the record's first stored
// path (the scene the Preview opens on), or the no-path scene when the record
// keeps none.
struct ChartScene {
    std::string chart;
    PreviewScene scene;
};

// Every corpus chart's scene, built once per run through the real analysis
// and the real scene builder.
const std::vector<ChartScene>& corpus_path_scenes() {
    static const std::vector<ChartScene> scenes = [] {
        std::vector<ChartScene> out;
        const app::AnalysisSettings& cfg = default_cfg();
        for (const std::string& chart : corpus::chart_paths()) {
            const Song& song = corpus_song_with_notes(chart);
            const std::vector<const Path*> paths = corpus::analyzed(chart, cfg).all_paths();
            const Path* first = paths.empty() ? nullptr : paths.front();
            out.push_back(ChartScene{chart, build_preview_scene(song, first, cfg.sp_cap, cfg.rules)});
        }
        return out;
    }();
    return scenes;
}

}  // namespace

TEST_CASE("SpanSweep: Onyx makeToggle truth table") {
    std::vector<std::pair<double, double>> ivs{{1.0, 2.0}, {2.0, 3.0}};
    SpanSweep sweep(ivs);
    CHECK(sweep.at(0.5) == Toggle::Empty);
    CHECK(sweep.at(1.0) == Toggle::Start);
    CHECK(sweep.at(1.5) == Toggle::On);
    CHECK(sweep.at(2.0) == Toggle::Restart);  // one ends, the next starts
    CHECK(sweep.at(3.0) == Toggle::End);
    CHECK(sweep.at(3.5) == Toggle::Empty);
}

TEST_CASE("SpRunSweep: the sp_active states, overlaps and the window just past an edge") {
    // Two back-to-back windows and one inside the first: an edge inside a
    // window is no edge of the running SP, which simply goes on.
    SpRunSweep sweep({{1.0, 2.0}, {2.0, 3.0}, {1.2, 1.4}});
    CHECK(sweep.at(0.5) == Toggle::Empty);
    CHECK(sweep.at(1.0) == Toggle::Start);
    CHECK(sweep.at(1.2) == Toggle::On);
    CHECK(sweep.at(1.4) == Toggle::On);
    CHECK(sweep.at(1.5) == Toggle::On);
    CHECK(sweep.at(2.0) == Toggle::Restart);  // one ends, the next starts
    CHECK(sweep.at(3.0) == Toggle::End);
    CHECK(sweep.at(3.5) == Toggle::Empty);
    // The double just inside each edge is On; the one just outside is Empty.
    CHECK(sweep.at(std::nextafter(1.0, 2.0)) == Toggle::On);
    CHECK(sweep.at(std::nextafter(1.0, 0.0)) == Toggle::Empty);
    CHECK(sweep.at(std::nextafter(3.0, 2.0)) == Toggle::On);
    CHECK(sweep.at(std::nextafter(3.0, 4.0)) == Toggle::Empty);
    CHECK(SpRunSweep{}.at(1.0) == Toggle::Empty);
}

TEST_CASE("toggle_on_after: on after Start, Restart and On; off after End and Empty") {
    CHECK(toggle_on_after(Toggle::Start));
    CHECK(toggle_on_after(Toggle::Restart));
    CHECK(toggle_on_after(Toggle::On));
    CHECK_FALSE(toggle_on_after(Toggle::End));
    CHECK_FALSE(toggle_on_after(Toggle::Empty));
}

TEST_CASE("TrackStateOptions: equal when pro matches") {
    CHECK(TrackStateOptions{true} == TrackStateOptions{true});
    CHECK(TrackStateOptions{true} != TrackStateOptions{false});
}

TEST_CASE("build_track_state: gems, pro-off, and the phrase end note reads inside") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(500.0, PreviewLane::Yellow, true, true),
                   note(500.0, PreviewLane::Kick), note(1000.0, PreviewLane::Green, true, false, true)};
    scene.sp_phrases = {span(500.0, 1000.0)};

    const double first_note = 0.0, chord = 0.5, phrase_last_note = 1.0;
    const double phrase_end_edge = end_edge_s(scene, scene.sp_phrases[0].end_tick);

    TrackState st = build_track_state(scene, TrackStateOptions{true});
    const auto& inst = st.instants();
    // One instant per moment: the three note moments and the phrase's drawn
    // end edge.
    REQUIRE(inst.size() == 4);
    CHECK(inst[0].t == doctest::Approx(first_note));
    CHECK(inst[1].t == doctest::Approx(chord));
    CHECK(inst[2].t == doctest::Approx(phrase_last_note));
    CHECK(inst[3].t == doctest::Approx(phrase_end_edge));

    const TrackInstant& chord_inst = inst[1];
    REQUIRE(chord_inst.notes.size() == 2);
    const TrackGem& yellow = chord_inst.notes[0];
    CHECK(yellow.pad == Pad::Yellow);
    CHECK(yellow.cymbal);
    CHECK(yellow.velocity == Velocity::Ghost);
    CHECK(chord_inst.notes[1].kick);
    REQUIRE(inst[2].notes.size() == 1);
    CHECK(inst[2].notes[0].velocity == Velocity::Accent);

    const TrackInstant& phrase_last = inst[2];
    const TrackInstant& phrase_edge = inst[3];
    CHECK(inst[0].overdrive == Toggle::Empty);
    CHECK(chord_inst.overdrive == Toggle::Start);
    CHECK(phrase_last.overdrive == Toggle::On);
    CHECK(phrase_edge.overdrive == Toggle::End);
    const double before_phrase = (first_note + chord) / 2.0;
    const double inside_phrase = (chord + phrase_last_note) / 2.0;
    const double past_last_note = (phrase_last_note + phrase_end_edge) / 2.0;
    CHECK(st.synthesize(before_phrase).overdrive == Toggle::Empty);
    CHECK(st.synthesize(inside_phrase).overdrive == Toggle::On);
    CHECK(st.synthesize(past_last_note).overdrive == Toggle::On);

    // Pro off: the same timeline, with no gem a cymbal.
    TrackState flat = build_track_state(scene, TrackStateOptions{false});
    REQUIRE(flat.instants().size() == inst.size());
    for (size_t k = 0; k < inst.size(); ++k) {
        CAPTURE(k);
        CHECK(instant_line(flat.instants()[k], false) == instant_line(inst[k], false));
        for (const TrackGem& g : flat.instants()[k].notes) CHECK_FALSE(g.cymbal);
    }
}

// The same promises on every corpus chart's real scene: each note is drawn
// once, at its own moment; each SP phrase still reads on at its last note;
// with Pro Drums off no gem is a cymbal.
TEST_CASE("build_track_state: every corpus chart's notes drawn once at their moment") {
    size_t phrases = 0;
    for (const ChartScene& cs : corpus_path_scenes()) {
        INFO("corpus chart " << cs.chart);
        const PreviewScene& scene = cs.scene;
        std::map<double, size_t> notes_at;
        for (const PreviewNote& n : scene.notes) ++notes_at[n.ms / 1000.0];
        for (bool pro : {true, false}) {
            CAPTURE(pro);
            const TrackState st = build_track_state(scene, TrackStateOptions{pro});
            size_t drawn = 0, wrong_count = 0, cymbals = 0;
            for (const TrackInstant& i : st.instants()) {
                const auto it = notes_at.find(i.t);
                if (i.notes.size() != (it == notes_at.end() ? 0 : it->second)) ++wrong_count;
                drawn += i.notes.size();
                for (const TrackGem& g : i.notes)
                    if (g.cymbal) ++cymbals;
            }
            CHECK(wrong_count == 0);
            CHECK(drawn == scene.notes.size());
            if (!pro) CHECK(cymbals == 0);
            for (const PreviewSpan& p : scene.sp_phrases) {
                CAPTURE(p.end_tick);
                CHECK(toggle_on_after(st.synthesize(p.end_ms / 1000.0).overdrive));
                if (pro) ++phrases;
            }
        }
    }
    // Pitch Dark's record activates SP, so its chart holds SP phrases.
    CHECK(phrases > 0);
}

// On every corpus chart's Preview path, each activation's Star Power window
// (its sp_window, the record's window end carried into the scene) ends on the
// highway at that end itself: the floor reads End there and is off from the
// very next moment, with no half-tick added the way a phrase's end gets one.
// A window whose end another window starts at or runs over goes on there, so
// only the others are held to it.
TEST_CASE("build_track_state: active SP window ends exactly at the deact node") {
    size_t windows = 0;
    for (const ChartScene& cs : corpus_path_scenes()) {
        INFO("corpus chart " << cs.chart);
        const TrackState st = build_track_state(cs.scene, TrackStateOptions{});
        std::vector<std::pair<double, double>> ws;  // seconds
        for (const PreviewActivation& a : cs.scene.activations)
            if (const std::optional<std::pair<double, double>> w = a.sp_window())
                ws.push_back({w->first / 1000.0, w->second / 1000.0});
        for (size_t k = 0; k < ws.size(); ++k) {
            const double end = ws[k].second;
            bool another_runs_on = false;
            for (size_t j = 0; j < ws.size(); ++j)
                if (j != k && ws[j].first <= end && end <= ws[j].second) another_runs_on = true;
            if (another_runs_on) continue;
            ++windows;
            CAPTURE(end);
            CHECK(st.synthesize(end).sp_active == Toggle::End);
            const double just_after = std::nextafter(end, std::numeric_limits<double>::infinity());
            CHECK(st.synthesize(just_after).sp_active == Toggle::Empty);
            const double just_before = std::nextafter(end, 0.0);
            CHECK(st.synthesize(just_before).sp_active == Toggle::On);
        }
    }
    // Pitch Dark's record activates SP with a stored window end.
    CHECK(windows > 0);
}

TEST_CASE("build_track_state: the sp_active field reads sp_running_in") {
    // The highway's sp_active field asks the one SP-running rule
    // (app::sp_running_in, through SpRunSweep) on each activation's
    // sp_window. Two back-to-back windows, [1000, 2000) and [2000, 3000) ms,
    // hand-built here (test_sp_meter_curve's sp_running_at case reads the
    // corpus paths' windows instead): SP runs from the first strike, through
    // the join, and stops at the second's end.
    PreviewScene scene;
    scene.notes = {note(0.0, PreviewLane::Red), note(5000.0, PreviewLane::Red)};
    PreviewActivation a;
    a.tick = 1000;
    a.ms = 1000.0;
    a.has_sp_end = true;
    a.sp_end_tick = 2000;
    a.sp_end_ms = 2000.0;
    PreviewActivation b = a;
    b.tick = 2000;
    b.ms = 2000.0;
    b.sp_end_tick = 3000;
    b.sp_end_ms = 3000.0;
    scene.activations = {a, b};

    const TrackState st = build_track_state(scene, TrackStateOptions{});
    // Before, at the start, inside, at the join, at the end, after.
    struct Want {
        double t_ms;
        Toggle state;
    };
    for (const Want& w : {Want{500.0, Toggle::Empty}, Want{1000.0, Toggle::Start},
                          Want{1500.0, Toggle::On}, Want{2000.0, Toggle::Restart},
                          Want{3000.0, Toggle::End}, Want{4000.0, Toggle::Empty}}) {
        INFO("t = " << w.t_ms << " ms");
        // The field at t: the instant at t when an edge puts one there, else
        // the one window() synthesizes at t.
        const double t_s = w.t_ms / 1000.0;
        const TrackWindow win = st.window(t_s - 0.1, t_s + 0.1);
        REQUIRE(win.size() == 1);
        CHECK(win[0].t == doctest::Approx(t_s));
        CHECK(win[0].sp_active == w.state);
    }
}

namespace {

// A lit-lane activation on `tick`: the activation that lights a taken fill
// ending there. `fill` is that fill's index in the scene's fills, as the
// scene builder stores it (taken_fill); none when no fill ends there.
PreviewActivation lane_activation(int64_t tick, PreviewLane lane, std::optional<size_t> fill) {
    PreviewActivation a;
    a.tick = tick;
    a.ms = static_cast<double>(tick);
    a.has_lane = true;
    a.lane = lane;
    a.taken_fill = fill;
    return a;
}

// One taken fill [1.0, 2.0] lit Green, one offered [4.0, 5.0], one hidden
// [7.0, 8.0].
PreviewScene taken_offered_hidden_scene() {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(2000.0, PreviewLane::Green),
                   note(5000.0, PreviewLane::Green), note(8000.0, PreviewLane::Green)};
    scene.fills = {fill(span(1000.0, 2000.0), PreviewFillState::Taken),
                   fill(span(4000.0, 5000.0), PreviewFillState::Offered),
                   fill(span(7000.0, 8000.0), PreviewFillState::Hidden)};
    scene.activations = {lane_activation(2000, PreviewLane::Green, 0)};
    return scene;
}

}  // namespace

// On every corpus chart's Preview path, each fill the scene builder marked
// (from the record) is drawn by its state, read halfway through its drawn
// span: a Taken fill as the taken strip, and the lit lane when an activation
// on a pad stands on it; an Offered fill as the offered strip alone; a Hidden
// fill not at all. A fill whose span overlaps another fill's is left out,
// since the two would share the strip there.
TEST_CASE("build_track_state: taken, offered and hidden fills toggle different spans") {
    size_t taken = 0, offered = 0, hidden = 0;
    for (const ChartScene& cs : corpus_path_scenes()) {
        INFO("corpus chart " << cs.chart);
        const PreviewScene& scene = cs.scene;
        const TrackState st = build_track_state(scene, TrackStateOptions{});
        std::vector<std::pair<double, double>> drawn;  // each fill's [start, end edge], s
        for (const PreviewFill& f : scene.fills)
            drawn.push_back({f.span.start_ms / 1000.0, end_edge_s(scene, f.span.end_tick)});
        for (size_t i = 0; i < scene.fills.size(); ++i) {
            const double mid = (drawn[i].first + drawn[i].second) / 2.0;
            bool shared = false;
            for (size_t j = 0; j < drawn.size(); ++j)
                if (j != i && drawn[j].first <= mid && mid <= drawn[j].second) shared = true;
            if (shared) continue;
            CAPTURE(scene.fills[i].chord_tick);
            const TrackInstant at = st.synthesize(mid);
            bool lit_lane = false;
            for (const PreviewActivation& a : scene.activations)
                if (a.has_lane && a.taken_fill == i && a.lane != PreviewLane::Kick) lit_lane = true;
            switch (scene.fills[i].state) {
                case PreviewFillState::Taken:
                    ++taken;
                    CHECK(at.fill_taken == Toggle::On);
                    CHECK(at.fill == Toggle::Empty);
                    CHECK(at.fill_lane == (lit_lane ? Toggle::On : Toggle::Empty));
                    CHECK(at.fill_lane_pad.has_value() == lit_lane);
                    CHECK(st.fill_at(mid) == std::optional<size_t>(i));
                    break;
                case PreviewFillState::Offered:
                    ++offered;
                    CHECK(at.fill == Toggle::On);
                    CHECK(at.fill_taken == Toggle::Empty);
                    CHECK(at.fill_lane == Toggle::Empty);
                    CHECK(st.fill_at(mid) == std::optional<size_t>(i));
                    break;
                case PreviewFillState::Hidden:
                    ++hidden;
                    CHECK(at.fill == Toggle::Empty);
                    CHECK(at.fill_taken == Toggle::Empty);
                    CHECK(at.fill_lane == Toggle::Empty);
                    CHECK_FALSE(st.fill_at(mid).has_value());
                    break;
            }
        }
    }
    // Every drum activation stands on a fill, so a path with an activation
    // takes one; Pitch Dark's best path activates, and its first activation
    // passes over fills on ticks 86400, 94080 and 101760 (its
    // skipped_fill_ticks) before the fill it takes. A fill after a path's
    // last activation that it was never shown is hidden.
    CHECK(taken > 0);
    CHECK(offered > 0);
    CHECK(hidden > 0);
}

// A ghost path's fills drive `fill_ghost` over their own spans, whatever the
// drawn path did with them: here the taken fill and the hidden one. Without a
// ghost no instant has the field on, and every other field reads as the
// plain scene's timeline does.
TEST_CASE("build_track_state: the ghost sweep holds the ghost fills and nothing else") {
    PreviewScene plain = taken_offered_hidden_scene();
    PreviewScene ghosted = plain;
    ghosted.ghost_fills = {0, 2};

    const TrackState none = build_track_state(plain, TrackStateOptions{});
    for (const TrackInstant& i : none.instants()) CHECK(i.fill_ghost == Toggle::Empty);

    const TrackState st = build_track_state(ghosted, TrackStateOptions{});
    // Taken [1.0, 2.0005] and hidden [7.0, 8.0005]; the offered fill at
    // [4.0, 5.0005] is no ghost's.
    std::vector<std::pair<double, Toggle>> ghost_edges;
    for (const TrackInstant& i : st.instants())
        if (i.fill_ghost != Toggle::Empty) ghost_edges.push_back({i.t, i.fill_ghost});
    REQUIRE(ghost_edges.size() == 6);
    const std::vector<std::pair<double, Toggle>> want = {
        {1.0, Toggle::Start}, {2.0, Toggle::On}, {2.0005, Toggle::End},
        {7.0, Toggle::Start}, {8.0, Toggle::On}, {8.0005, Toggle::End}};
    for (size_t k = 0; k < want.size(); ++k) {
        CAPTURE(k);
        CHECK(ghost_edges[k].first == doctest::Approx(want[k].first));
        CHECK(ghost_edges[k].second == want[k].second);
    }
    // Between the edges the sweep reads on inside a ghost fill and off
    // elsewhere, through the empty-window fallback too.
    CHECK(st.synthesize(1.5).fill_ghost == Toggle::On);
    CHECK(st.synthesize(4.5).fill_ghost == Toggle::Empty);
    CHECK(st.synthesize(7.5).fill_ghost == Toggle::On);

    // Every other field is the plain timeline's: each of its instants reads
    // the same line in the ghosted one (instant_line leaves fill_ghost out).
    for (const TrackInstant& i : none.instants()) {
        const TrackInstant* g = find(st.instants(), i.t);
        REQUIRE(g);
        CHECK(instant_line(*g) == instant_line(i));
    }
}

TEST_CASE("build_track_state: beats land on instants; solo toggles") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(250.0, PreviewLane::Red)};
    scene.beats = {{0, 0.0, PreviewBeatKind::MeasureLine}, {0, 250.0, PreviewBeatKind::Half},
                   {0, 500.0, PreviewBeatKind::Beat}};
    scene.solos = {span(250.0, 250.0)};  // a one-note solo

    TrackState st = build_track_state(scene, TrackStateOptions{});
    const TrackInstant* bar = find(st.instants(), 0.0);
    const TrackInstant* solo_note = find(st.instants(), 0.25);
    const TrackInstant* solo_end = find(st.instants(), end_edge_s(scene, scene.solos[0].end_tick));
    const TrackInstant* beat = find(st.instants(), 0.5);
    REQUIRE(bar);
    REQUIRE(solo_note);
    REQUIRE(solo_end);
    REQUIRE(beat);
    REQUIRE(bar->beat.has_value());
    CHECK(*bar->beat == PreviewBeatKind::MeasureLine);
    REQUIRE(solo_note->beat.has_value());
    CHECK(*solo_note->beat == PreviewBeatKind::Half);
    REQUIRE(beat->beat.has_value());
    CHECK(*beat->beat == PreviewBeatKind::Beat);
    CHECK(solo_note->notes.size() == 1);
    CHECK(solo_note->solo == Toggle::Start);
    CHECK(solo_end->solo == Toggle::End);
    CHECK_FALSE(solo_end->beat.has_value());
    CHECK(solo_end->notes.empty());
}

TEST_CASE("window: strict bounds, and a synthesized instant when empty") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(2000.0, PreviewLane::Red),
                   note(3000.0, PreviewLane::Red)};
    scene.solos = {span(900.0, 3100.0)};
    TrackState st = build_track_state(scene, TrackStateOptions{});

    const double first_note = 1.0, middle_note = 2.0, last_note = 3.0;

    // A window from the first note to the last holds only the note between.
    TrackWindow w = st.window(first_note, last_note);
    REQUIRE(w.size() == 1);
    CHECK(w[0].t == doctest::Approx(middle_note));
    CHECK(w[0].solo == Toggle::On);

    // A window between two notes holds one instant at its midpoint, which
    // carries the ongoing solo and no gems or beat.
    const double empty_near = 2.1, empty_far = 2.9;
    TrackWindow empty = st.window(empty_near, empty_far);
    REQUIRE(empty.size() == 1);
    CHECK(empty[0].t == doctest::Approx((empty_near + empty_far) / 2.0));
    CHECK(empty[0].solo == Toggle::On);
    CHECK(empty[0].notes.empty());
    CHECK_FALSE(empty[0].beat.has_value());

    // A window before the solo starts holds one instant with the solo absent.
    const double solo_start = scene.solos[0].start_ms / 1000.0;
    TrackWindow before = st.window(0.1, 0.5);
    REQUIRE(before.size() == 1);
    CHECK(before[0].t < solo_start);
    CHECK(before[0].solo == Toggle::Empty);
}

TEST_CASE("make_toggle_bounds: covers [near, far], merges equal neighbours") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(10000.0, PreviewLane::Red)};
    // Touching solos: with the half-tick end they overlap, so the second's start
    // and the first's end both read as On and the span never breaks.
    scene.solos = {span(1000.0, 2000.0), span(2000.0, 3000.0)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    const double first_start = scene.solos[0].start_ms / 1000.0;
    const double second_start = scene.solos[1].start_ms / 1000.0;
    const double first_end_edge = end_edge_s(scene, scene.solos[0].end_tick);
    const double second_end_edge = end_edge_s(scene, scene.solos[1].end_tick);
    // Each solo's edge where they meet falls inside the other solo.
    const TrackInstant* at_second_start = find(st.instants(), second_start);
    const TrackInstant* at_first_end = find(st.instants(), first_end_edge);
    REQUIRE(at_second_start);
    REQUIRE(at_first_end);
    CHECK(at_second_start->solo == Toggle::On);
    CHECK(at_first_end->solo == Toggle::On);

    // The window [0.5, 4.0] reads off before the first solo, one lit stretch
    // over both solos, and off after the second.
    const double near_s = 0.5, far_s = 4.0;
    TrackWindow w = st.window(near_s, far_s);
    std::vector<ToggleSpan> spans =
        st.make_toggle_bounds(w, near_s, far_s, &TrackInstant::solo);
    REQUIRE(spans.size() == 3);
    CHECK(spans[0].t1 == doctest::Approx(near_s));
    CHECK(spans[0].t2 == doctest::Approx(first_start));
    CHECK_FALSE(spans[0].on);
    CHECK(spans[1].t1 == doctest::Approx(first_start));
    CHECK(spans[1].t2 == doctest::Approx(second_end_edge));
    CHECK(spans[1].on);
    CHECK(spans[2].t1 == doctest::Approx(second_end_edge));
    CHECK(spans[2].t2 == doctest::Approx(far_s));
    CHECK_FALSE(spans[2].on);

    // A window opening inside the lit stretch starts lit.
    const double mid_near = 1.5, mid_far = 2.5;
    TrackWindow mid = st.window(mid_near, mid_far);
    std::vector<ToggleSpan> mid_spans =
        st.make_toggle_bounds(mid, mid_near, mid_far, &TrackInstant::solo);
    REQUIRE(mid_spans.size() == 1);
    CHECK(mid_spans[0].on);
    CHECK(mid_spans[0].t1 == doctest::Approx(mid_near));
    CHECK(mid_spans[0].t2 == doctest::Approx(mid_far));
}

TEST_CASE("make_lane_bounds: one taken fill equals the fill_lane toggle bounds and carries its pad") {
    TrackState st = build_track_state(taken_offered_hidden_scene(), TrackStateOptions{});
    // Whole lanes and windows that open inside the lit lane, where the pad
    // comes from before the window.
    for (std::pair<double, double> w : {std::pair<double, double>{0.5, 9.0}, {1.5, 2.5}, {2.0002, 3.5}}) {
        TrackWindow win = st.window(w.first, w.second);
        std::vector<ToggleSpan> lit;
        for (const ToggleSpan& s :
             st.make_toggle_bounds(win, w.first, w.second, &TrackInstant::fill_lane))
            if (s.on) lit.push_back(s);
        std::vector<LaneSpan> lanes = st.make_lane_bounds(win, w.first, w.second);
        REQUIRE(lanes.size() == lit.size());
        REQUIRE(lanes.size() == 1);
        for (size_t i = 0; i < lanes.size(); ++i) {
            CHECK(lanes[i].t1 == lit[i].t1);
            CHECK(lanes[i].t2 == lit[i].t2);
            CHECK(lanes[i].pad == Pad::Green);
        }
    }
}

// Two taken fills that touch: the second starts on the first's end tick, so
// with the half-tick end edge they overlap. Fill A is [1.0, 2.0005] lit Green
// (its activation is listed first), fill B [2.0, 3.0005] lit Yellow.
//
// By hand, by LaneSweep's rule (the earlier-listed interval wins while it
// starts at or holds t, else the first that ends at t): at 1.0 A starts,
// Green. At 2.0 B starts but A still holds 2.0, and A is listed first, so
// Green. At 2.0005 A ends and B holds, so Yellow. At 3.0005 B ends, Yellow.
// So the lane is Green from 1.0 to 2.0005 and Yellow from 2.0005 to 3.0005,
// where make_toggle_bounds sees one merged lit stretch [1.0, 3.0005).
TEST_CASE("make_lane_bounds: two touching taken fills each light their own lane") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(4000.0, PreviewLane::Red)};
    scene.fills = {fill(span(1000.0, 2000.0), PreviewFillState::Taken),
                   fill(span(2000.0, 3000.0), PreviewFillState::Taken)};
    scene.activations = {lane_activation(2000, PreviewLane::Green, 0),
                         lane_activation(3000, PreviewLane::Yellow, 1)};
    TrackState st = build_track_state(scene, TrackStateOptions{});

    TrackWindow win = st.window(0.5, 3.5);
    std::vector<ToggleSpan> merged = st.make_toggle_bounds(win, 0.5, 3.5, &TrackInstant::fill_lane);
    REQUIRE(merged.size() == 3);
    CHECK(merged[1].on);
    CHECK(merged[1].t1 == doctest::Approx(1.0));
    CHECK(merged[1].t2 == doctest::Approx(3.0005));

    std::vector<LaneSpan> lanes = st.make_lane_bounds(win, 0.5, 3.5);
    REQUIRE(lanes.size() == 2);
    CHECK(lanes[0].t1 == doctest::Approx(1.0));
    CHECK(lanes[0].t2 == doctest::Approx(2.0005));
    CHECK(lanes[0].pad == Pad::Green);
    CHECK(lanes[1].t1 == doctest::Approx(2.0005));
    CHECK(lanes[1].t2 == doctest::Approx(3.0005));
    CHECK(lanes[1].pad == Pad::Yellow);

    // A window opening between the two edges at 2.0 and 2.0005 enters Green.
    TrackWindow mid = st.window(2.0002, 3.5);
    std::vector<LaneSpan> mid_lanes = st.make_lane_bounds(mid, 2.0002, 3.5);
    REQUIRE(mid_lanes.size() == 2);
    CHECK(mid_lanes[0].t1 == doctest::Approx(2.0002));
    CHECK(mid_lanes[0].pad == Pad::Green);
    CHECK(mid_lanes[1].pad == Pad::Yellow);
}

namespace {

// A note on tick 0, the phrase's last note on tick 480, and one a tick later
// on 481 (see phrase_to_tick_480_scene for the timing and the phrase).
PreviewScene phrase_then_next_tick_scene() {
    return phrase_to_tick_480_scene(
        {{0, PreviewLane::Red}, {480, PreviewLane::Yellow}, {481, PreviewLane::Blue}});
}

}  // namespace

TEST_CASE("build_track_state: a chord one tick after a phrase ends is not SP (480 res, 300 BPM)") {
    PreviewScene scene = phrase_then_next_tick_scene();
    const SongTiming& timing = *scene.timing;

    TrackState st = build_track_state(scene, TrackStateOptions{});
    const TrackInstant* last_in = find(st.instants(), timing.ms_index().at(480) / 1000.0);
    const TrackInstant* next = find(st.instants(), timing.ms_index().at(481) / 1000.0);
    REQUIRE(last_in);
    REQUIRE(next);
    CHECK(last_in->overdrive == Toggle::On);
    CHECK(next->overdrive == Toggle::Empty);
    // The phrase's drawn end falls between the two notes.
    const TrackInstant* edge = find(st.instants(), end_edge_s(scene, scene.sp_phrases[0].end_tick));
    REQUIRE(edge);
    CHECK(edge->overdrive == Toggle::End);
    CHECK(last_in->t < edge->t);
    CHECK(edge->t < next->t);
}

// A span's drawn end is read through the song's timing, so every scene the
// Preview builds carries it: on every corpus chart the scene holds its timing
// and its timeline builds.
TEST_CASE("build_track_state: spans need the song timing") {
    for (const ChartScene& cs : corpus_path_scenes()) {
        INFO("corpus chart " << cs.chart);
        CHECK(cs.scene.timing.has_value());
        CHECK_NOTHROW(build_track_state(cs.scene, TrackStateOptions{}));
    }
}

// The renderer asks for this window every frame, so it copies no instant and
// no notes.
TEST_CASE("window: a view into the state, not a copy") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(2000.0, PreviewLane::Red),
                   note(3000.0, PreviewLane::Red)};
    TrackState st = build_track_state(scene, TrackStateOptions{});

    TrackWindow w = st.window(0.5, 3.5);
    const TrackInstant* in_state = find(st.instants(), 2.0);
    REQUIRE(in_state != nullptr);
    const TrackInstant* in_window = nullptr;
    for (const TrackInstant& i : w)
        if (i.t == doctest::Approx(2.0)) in_window = &i;
    CHECK(in_window == in_state);

    // Walking it backwards starts at the state's own last instant.
    const TrackInstant* last_in_state = &st.instants().back();
    CHECK(&*w.rbegin() == last_in_state);
    CHECK(&w[w.size() - 1] == last_in_state);
}

// The one-pass builder on each awkward interval shape, one case per shape.
// One tick is one millisecond (timed_scene), and each edge is named by what
// it is.

// Two SP phrases whose notes overlap, as a chart editor saves them: the
// first's end edge sits inside the second, so the phrase reads as one span
// from the first's start to the second's end.
TEST_CASE("build_track_state: overlapping SP phrases read as one span") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Red),
                   note(2000.0, PreviewLane::Red), note(2500.0, PreviewLane::Red)};
    scene.sp_phrases = {span(1000.0, 2000.0), span(1500.0, 2500.0)};
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    const double first_end = end_edge_s(scene, scene.sp_phrases[0].end_tick);
    const double second_end = end_edge_s(scene, scene.sp_phrases[1].end_tick);
    CHECK(instant_at(st, scene.sp_phrases[0].start_ms / 1000.0).overdrive == Toggle::Start);
    CHECK(instant_at(st, scene.sp_phrases[1].start_ms / 1000.0).overdrive == Toggle::On);
    CHECK(instant_at(st, first_end).overdrive == Toggle::On);
    CHECK(instant_at(st, second_end).overdrive == Toggle::End);
    CHECK(st.synthesize((first_end + second_end) / 2.0).overdrive == Toggle::On);
}

// A solo nested inside another, as a chart editor saves a second solo marker
// pair inside the first: the inner solo's edges are no edges of the drawn
// solo, which simply goes on.
TEST_CASE("build_track_state: a solo nested in another reads on at the inner edges") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(4000.0, PreviewLane::Red), note(4200.0, PreviewLane::Red),
                   note(4800.0, PreviewLane::Red), note(5000.0, PreviewLane::Red)};
    const PreviewSpan outer = span(4000.0, 5000.0), inner = span(4200.0, 4800.0);
    scene.solos = {outer, inner};
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    CHECK(instant_at(st, outer.start_ms / 1000.0).solo == Toggle::Start);
    CHECK(instant_at(st, inner.start_ms / 1000.0).solo == Toggle::On);
    CHECK(instant_at(st, end_edge_s(scene, inner.end_tick)).solo == Toggle::On);
    CHECK(instant_at(st, end_edge_s(scene, outer.end_tick)).solo == Toggle::End);
}

// The fill lane comes only from an activation standing on a taken fill, on a
// pad: with several such activations on one fill the first listed lights it,
// an activation on the kick lights nothing, and one on no fill lights
// nothing. A hidden fill draws nothing at all.
TEST_CASE("build_track_state: a fill lane needs a taken fill and a pad") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(7000.0, PreviewLane::Red), note(7500.0, PreviewLane::Yellow, true),
                   note(8500.0, PreviewLane::Red), note(9500.0, PreviewLane::Red)};
    scene.fills = {fill(span(7000.0, 7500.0), PreviewFillState::Taken),
                   fill(span(8500.0, 8800.0), PreviewFillState::Hidden)};
    scene.activations = {lane_activation(7500, PreviewLane::Kick, 0),
                         lane_activation(7500, PreviewLane::Green, 0),
                         lane_activation(7500, PreviewLane::Yellow, 0),
                         lane_activation(9500, PreviewLane::Red, std::nullopt)};
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    const PreviewSpan& taken = scene.fills[0].span;
    const PreviewSpan& hidden = scene.fills[1].span;

    const TrackInstant& lane_start = instant_at(st, taken.start_ms / 1000.0);
    CHECK(lane_start.fill_lane == Toggle::Start);
    REQUIRE(lane_start.fill_lane_pad.has_value());
    CHECK(*lane_start.fill_lane_pad == Pad::Green);
    const double near_s = 6.5, far_s = 10.0;
    const std::vector<LaneSpan> lanes = st.make_lane_bounds(st.window(near_s, far_s), near_s, far_s);
    REQUIRE(lanes.size() == 1);
    CHECK(lanes[0].pad == Pad::Green);
    CHECK(lanes[0].t1 == doctest::Approx(taken.start_ms / 1000.0));
    CHECK(lanes[0].t2 == doctest::Approx(end_edge_s(scene, taken.end_tick)));

    const TrackInstant& no_fill = instant_at(st, 9.5);
    CHECK(no_fill.fill_lane == Toggle::Empty);
    CHECK_FALSE(no_fill.fill_lane_pad.has_value());

    CHECK(find(st.instants(), end_edge_s(scene, hidden.end_tick)) == nullptr);
    const TrackInstant inside_hidden = st.synthesize((hidden.start_ms + hidden.end_ms) / 2000.0);
    CHECK(inside_hidden.fill == Toggle::Empty);
    CHECK(inside_hidden.fill_taken == Toggle::Empty);
    CHECK(inside_hidden.fill_lane == Toggle::Empty);
}

namespace {

// An activation with an SP window from `ms` to `end_ms`, stored when
// `has_end`.
PreviewActivation sp_activation(double ms, double end_ms, bool has_end) {
    PreviewActivation a;
    a.ms = ms;
    a.tick = static_cast<int64_t>(ms);
    a.has_sp_end = has_end;
    a.sp_end_ms = end_ms;
    a.sp_end_tick = static_cast<int64_t>(end_ms);
    return a;
}

}  // namespace

// A window of no length, one that ends before it starts and one with no
// stored end come from no record: they stand for sp_window's guard
// (preview_view.h), which gives none of them a window. The highway then draws
// no Star Power for them, beside an ordinary window that it does draw.
TEST_CASE("build_track_state: an activation with no SP window draws no Star Power") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(9500.0, PreviewLane::Red)};
    const PreviewActivation ordinary = sp_activation(1200.0, 3000.0, true);
    const std::vector<PreviewActivation> refused = {sp_activation(3500.0, 3500.0, true),
                                                    sp_activation(3600.0, 3400.0, true),
                                                    sp_activation(3700.0, 9000.0, false)};
    scene.activations = {ordinary};
    scene.activations.insert(scene.activations.end(), refused.begin(), refused.end());
    const TrackState st = build_track_state(scene, TrackStateOptions{});

    CHECK(instant_at(st, ordinary.ms / 1000.0).sp_active == Toggle::Start);
    CHECK(instant_at(st, ordinary.sp_end_ms / 1000.0).sp_active == Toggle::End);
    for (const PreviewActivation& a : refused) {
        CAPTURE(a.ms);
        CHECK_FALSE(a.sp_window().has_value());
        CHECK(find(st.instants(), a.ms / 1000.0) == nullptr);
        CHECK(st.synthesize(a.ms / 1000.0).sp_active == Toggle::Empty);
    }
    // Inside where the window with no stored end would have run.
    CHECK(st.synthesize(5.0).sp_active == Toggle::Empty);
}

// A solo of no length and one that ends before it starts: no chart editor
// saves either, so they stand for SpanSweep's guard for malformed intervals
// (track_state.cpp, above SpanSweep's constructor): such an interval holds no
// time inside, and only its edges show.
TEST_CASE("build_track_state: a zero-length or backwards solo covers nothing") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(2000.0, PreviewLane::Red), note(4000.0, PreviewLane::Red)};
    PreviewSpan zero = span(3000.0, 3000.0);
    zero.start_ms = 3000.5;  // starts on its own end edge
    const PreviewSpan backwards = span(3200.0, 3100.0);
    scene.solos = {zero, backwards};
    const TrackState st = build_track_state(scene, TrackStateOptions{});

    const double zero_at = end_edge_s(scene, zero.end_tick);
    REQUIRE(zero_at == doctest::Approx(zero.start_ms / 1000.0));
    CHECK(instant_at(st, zero_at).solo == Toggle::Restart);
    const double backwards_end = end_edge_s(scene, backwards.end_tick);
    const double backwards_start = backwards.start_ms / 1000.0;
    CHECK(instant_at(st, backwards_end).solo == Toggle::End);
    CHECK(instant_at(st, backwards_start).solo == Toggle::Start);
    for (double t : {2.5, (zero_at + backwards_end) / 2.0, (backwards_end + backwards_start) / 2.0,
                     3.5}) {
        CAPTURE(t);
        CHECK(st.synthesize(t).solo == Toggle::Empty);
    }
}

// The scene lists notes in chart order by lane, not by time, and two beat
// lines can share a moment: each instant holds its own moment's gems in scene
// order, its last listed beat, and a cymbal flag only where the engine's
// cymbal rule (cymbal_counts) makes the pad a cymbal.
TEST_CASE("build_track_state: notes out of order, a red cymbal flag, and two beats on one moment") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(5000.0, PreviewLane::Green, false, false, true),
                   note(1000.0, PreviewLane::Red, true), note(1000.0, PreviewLane::Kick)};
    scene.beats = {{0, 2000.0, PreviewBeatKind::MeasureLine}, {0, 2000.0, PreviewBeatKind::Half}};
    const TrackState st = build_track_state(scene, TrackStateOptions{true});
    REQUIRE(st.instants().size() == 3);
    CHECK(st.instants()[0].t < st.instants()[1].t);
    CHECK(st.instants()[1].t < st.instants()[2].t);

    const TrackInstant& chord = instant_at(st, 1.0);
    REQUIRE(chord.notes.size() == 2);
    CHECK(chord.notes[0].pad == Pad::Red);
    CHECK(chord.notes[0].cymbal == cymbal_counts(app::color_of(PreviewLane::Red), true));
    CHECK(chord.notes[1].kick);

    const TrackInstant& beat = instant_at(st, 2.0);
    REQUIRE(beat.beat.has_value());
    CHECK(*beat.beat == scene.beats.back().kind);

    const TrackInstant& accent = instant_at(st, 5.0);
    REQUIRE(accent.notes.size() == 1);
    CHECK(accent.notes[0].velocity == Velocity::Accent);
}

// A single Yellow lane [1.0, 2.0], a double Red + Blue lane [3.0, 4.0], a Red
// lane touching it at 4.0005 (its start is the double lane's end, kSpanEndTicks
// past tick 4000) and running to 5.0, and a kick lane at [0.5, 0.6]. Hand
// check: each pad is lit only over its own lanes, the two touching Red lanes
// read as one lit stretch (a Restart where they meet), Green is never lit, and
// the kick lane adds no instant at all.
TEST_CASE("build_track_state: drum lanes light their own pads over their spans") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(6000.0, PreviewLane::Red)};
    scene.drum_lanes = {drum_lane(span(500.0, 600.0), {PreviewLane::Kick}),
                        drum_lane(span(1000.0, 2000.0), {PreviewLane::Yellow}),
                        drum_lane(span(3000.0, 4000.0), {PreviewLane::Red, PreviewLane::Blue}),
                        drum_lane(span(4000.5, 5000.0), {PreviewLane::Red})};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    CHECK(find(st.instants(), 0.5) == nullptr);
    CHECK(find(st.instants(), 0.6005) == nullptr);
    const TrackInstant* meet = find(st.instants(), 4.0005);
    REQUIRE(meet);
    CHECK(meet->drum_lane_red == Toggle::Restart);
    CHECK(meet->drum_lane_blue == Toggle::End);
    CHECK(meet->drum_lane_yellow == Toggle::Empty);

    const double near_s = -1.0, far_s = 7.0;
    const TrackWindow win = st.window(near_s, far_s);
    auto lit = [&](Pad p) {
        std::vector<std::pair<double, double>> out;
        for (const ToggleSpan& s : st.make_toggle_bounds(win, near_s, far_s, drum_lane_field(p)))
            if (s.on) out.push_back({s.t1, s.t2});
        return out;
    };
    auto check_lit = [&](Pad p, const std::vector<std::pair<double, double>>& want) {
        const auto got = lit(p);
        INFO("pad " << static_cast<int>(p));
        REQUIRE(got.size() == want.size());
        for (size_t i = 0; i < want.size(); ++i) {
            CHECK(got[i].first == doctest::Approx(want[i].first));
            CHECK(got[i].second == doctest::Approx(want[i].second));
        }
    };
    check_lit(Pad::Red, {{3.0, 5.0005}});
    check_lit(Pad::Yellow, {{1.0, 2.0005}});
    check_lit(Pad::Blue, {{3.0, 4.0005}});
    check_lit(Pad::Green, {});
    // No lane touches the other span fields.
    for (const TrackInstant& i : st.instants()) {
        CHECK(i.overdrive == Toggle::Empty);
        CHECK(i.fill == Toggle::Empty);
        CHECK(i.fill_lane == Toggle::Empty);
    }
}

// ---- rebuild_overlay_fields -------------------------------------------------

namespace {

// Build from `from`, swap in `to`'s overlay, and hold the result to a full
// build of `to`: every instant, and the empty-window fallback between every
// pair of neighbours (which reads the intervals). Then swap back to `from` and
// hold that to a full build of `from`, so instants only `to` made are seen to
// go. Both pro settings.
void check_rebuild(const PreviewScene& from, const PreviewScene& to, const std::string& what) {
    auto same_as_full = [&what](const TrackState& got, const TrackState& want, const char* step) {
        const std::vector<TrackInstant>& g = got.instants();
        const std::vector<TrackInstant>& w = want.instants();
        INFO(what << ", " << step);
        REQUIRE(g.size() == w.size());
        size_t mismatches = 0, first = 0;
        for (size_t i = 0; i < w.size(); ++i)
            if (!same_instant(g[i], w[i]) && mismatches++ == 0) first = i;
        INFO("first mismatch at instant " << first);
        CHECK(mismatches == 0);
        size_t synth_mismatches = 0;
        for (size_t i = 0; i + 1 < w.size(); ++i) {
            TrackWindow gw = got.window(w[i].t, w[i + 1].t);
            TrackWindow ww = want.window(w[i].t, w[i + 1].t);
            if (gw.size() != 1 || ww.size() != 1 || !same_instant(gw[0], ww[0])) ++synth_mismatches;
        }
        CHECK(synth_mismatches == 0);
    };
    for (bool pro : {true, false}) {
        TrackState st = build_track_state(from, TrackStateOptions{pro});
        rebuild_overlay_fields(st, to);
        same_as_full(st, build_track_state(to, TrackStateOptions{pro}), "rebuilt to the new scene");
        rebuild_overlay_fields(st, from);
        same_as_full(st, build_track_state(from, TrackStateOptions{pro}), "rebuilt back again");
    }
}

// A random overlay over `scene`'s fills: each fill Hidden, Offered or Taken,
// and activations whose SP windows start and end on notes, beats, fill
// edges, each other or nowhere in particular, some empty or backwards, some
// lighting a lane on a taken fill, and a ghost path's fills.
void randomize_overlay(PreviewScene& scene, std::mt19937& rng, const std::vector<double>& moments) {
    auto pick = [&rng](uint32_t n) { return static_cast<uint32_t>(rng() % n); };
    auto moment = [&]() { return moments[pick(static_cast<uint32_t>(moments.size()))]; };
    scene.activations.clear();
    std::vector<int64_t> taken_ends;
    std::vector<size_t> taken_index;  // each taken fill's index, beside its end
    for (size_t k = 0; k < scene.fills.size(); ++k) {
        PreviewFill& f = scene.fills[k];
        f.state = static_cast<PreviewFillState>(pick(3));
        if (f.state == PreviewFillState::Taken) {
            taken_ends.push_back(f.span.end_tick);
            taken_index.push_back(k);
        }
    }
    for (int i = 0; i < 40; ++i) {
        PreviewActivation a;
        a.ms = pick(2) == 0 ? moment() : 0.5 * pick(8000);
        a.tick = static_cast<int64_t>(a.ms);
        a.has_sp_end = pick(6) != 0;
        a.sp_end_ms = pick(3) == 0 ? moment() : a.ms + 0.5 * (static_cast<double>(pick(1200)) - 100.0);
        a.sp_end_tick = static_cast<int64_t>(a.sp_end_ms);
        if (pick(2) == 0) {
            a.has_lane = true;
            a.lane = static_cast<PreviewLane>(pick(5));
            if (!taken_ends.empty() && pick(4) != 0) {
                const uint32_t j = pick(static_cast<uint32_t>(taken_ends.size()));
                a.tick = taken_ends[j];
                a.taken_fill = taken_index[j];
            }
        }
        scene.activations.push_back(a);
    }
    // A ghost path's fills: any fill, whatever its state.
    scene.ghost_fills.clear();
    for (size_t k = 0; k < scene.fills.size(); ++k)
        if (pick(3) == 0) scene.ghost_fills.push_back(k);
}

// A path-free scene and four random overlays over it, the base itself first.
std::vector<PreviewScene> random_overlay_scenes() {
    std::mt19937 rng(20261004u);
    auto pick = [&rng](uint32_t n) { return static_cast<uint32_t>(rng() % n); };

    // The path-free half: notes, beats, SP phrases, solos, drum lanes and fill
    // windows, many sharing moments.
    PreviewScene base = timed_scene();
    const auto& ms_index = base.timing->ms_index();
    std::vector<double> moments;
    auto random_span = [&]() {
        const int64_t end = 1 + pick(3999);
        PreviewSpan s = span(static_cast<double>(end) - 0.5 * (1 + pick(600)), 0.0);
        s.end_tick = end;
        s.end_ms = static_cast<double>(end);
        moments.push_back(s.start_ms);
        moments.push_back(ms_index.ms_at_tick_f(static_cast<double>(end) + 0.5));
        return s;
    };
    for (int i = 0; i < 30; ++i) base.sp_phrases.push_back(random_span());
    for (int i = 0; i < 30; ++i) base.solos.push_back(random_span());
    // Single and double lanes on any column, the kick included (it lights
    // nothing).
    for (int i = 0; i < 30; ++i) {
        const PreviewSpan s = random_span();
        if (pick(2) == 0)
            base.drum_lanes.push_back(drum_lane(s, {static_cast<PreviewLane>(pick(5))}));
        else
            base.drum_lanes.push_back(drum_lane(
                s, {static_cast<PreviewLane>(pick(5)), static_cast<PreviewLane>(pick(5))}));
    }
    for (int i = 0; i < 60; ++i) base.fills.push_back(fill(random_span(), PreviewFillState::Hidden));
    for (int i = 0; i < 1500; ++i) {
        const double ms = pick(5) == 0 ? moments[pick(static_cast<uint32_t>(moments.size()))]
                                       : static_cast<double>(pick(4000));
        base.notes.push_back(note(ms, static_cast<PreviewLane>(pick(5)), pick(2) == 0));
        moments.push_back(ms);
    }
    for (int i = 0; i < 400; ++i) {
        PreviewBeat b;
        b.ms = pick(4) == 0 ? moments[pick(static_cast<uint32_t>(moments.size()))]
                            : 0.25 * pick(16000);
        b.kind = static_cast<PreviewBeatKind>(pick(3));
        base.beats.push_back(b);
    }

    std::vector<PreviewScene> scenes{base};
    for (int k = 0; k < 4; ++k) {
        PreviewScene s = base;
        randomize_overlay(s, rng, moments);
        scenes.push_back(std::move(s));
    }
    return scenes;
}

// The drawn [start, end edge] of each span, in seconds.
std::vector<std::pair<double, double>> drawn_intervals(const PreviewScene& scene,
                                                       const std::vector<PreviewSpan>& spans) {
    std::vector<std::pair<double, double>> out;
    for (const PreviewSpan& s : spans) out.push_back({s.start_ms / 1000.0, end_edge_s(scene, s.end_tick)});
    return out;
}

// Does `t` lie strictly inside one of `ivs`?
bool strictly_inside_any(const std::vector<std::pair<double, double>>& ivs, double t) {
    for (const std::pair<double, double>& iv : ivs)
        if (iv.first < t && t < iv.second) return true;
    return false;
}

// The empty-window fallback between each pair of neighbouring instants, held
// to the scene's own spans rather than to the sweep that filled the instants.
// No edge lies between two neighbouring instants, so there each span field
// reads On exactly where one of its spans' drawn intervals holds the
// fallback's time, and Empty elsewhere: SP phrases, solos, offered fills,
// taken fills and the ghost path's fills. The active SP field reads On where
// the one SP-running rule (app::sp_running_in) says an activation's sp_window
// runs.
void check_fallback_reads_spans(const PreviewScene& scene, const std::string& what) {
    const TrackState st = build_track_state(scene, TrackStateOptions{true});
    std::vector<PreviewSpan> offered, taken, ghost;
    for (const PreviewFill& f : scene.fills) {
        if (f.state == PreviewFillState::Offered) offered.push_back(f.span);
        if (f.state == PreviewFillState::Taken) taken.push_back(f.span);
    }
    for (size_t k : scene.ghost_fills) ghost.push_back(scene.fills[k].span);
    const auto phrase_ivs = drawn_intervals(scene, scene.sp_phrases);
    const auto solo_ivs = drawn_intervals(scene, scene.solos);
    const auto offered_ivs = drawn_intervals(scene, offered);
    const auto taken_ivs = drawn_intervals(scene, taken);
    const auto ghost_ivs = drawn_intervals(scene, ghost);
    std::vector<std::pair<double, double>> windows;  // seconds
    for (const PreviewActivation& a : scene.activations)
        if (const std::optional<std::pair<double, double>> w = a.sp_window())
            windows.push_back({w->first / 1000.0, w->second / 1000.0});
    auto want = [](bool on) { return on ? Toggle::On : Toggle::Empty; };

    const std::vector<TrackInstant>& v = st.instants();
    size_t mismatches = 0;
    std::string first;
    for (size_t i = 0; i + 1 < v.size(); ++i) {
        const TrackWindow gap = st.window(v[i].t, v[i + 1].t);
        if (gap.size() != 1) {
            if (mismatches++ == 0) first = "no single fallback after instant " + std::to_string(i);
            continue;
        }
        const TrackInstant& f = gap[0];
        bool sp = false;
        for (const std::pair<double, double>& w : windows) sp = sp || app::sp_running_in(w, f.t);
        const bool reads_spans = f.overdrive == want(strictly_inside_any(phrase_ivs, f.t)) &&
                                 f.solo == want(strictly_inside_any(solo_ivs, f.t)) &&
                                 f.fill == want(strictly_inside_any(offered_ivs, f.t)) &&
                                 f.fill_taken == want(strictly_inside_any(taken_ivs, f.t)) &&
                                 f.fill_ghost == want(strictly_inside_any(ghost_ivs, f.t)) &&
                                 f.sp_active == want(sp);
        if (!reads_spans && mismatches++ == 0) first = instant_line(f);
    }
    INFO(what << ", first mismatch: " << first);
    CHECK(mismatches == 0);
}

}  // namespace

TEST_CASE("synthesize between instants reads the scene's own spans") {
    const std::vector<PreviewScene> scenes = random_overlay_scenes();
    for (size_t i = 0; i < scenes.size(); ++i)
        check_fallback_reads_spans(scenes[i], "overlay " + std::to_string(i));

    // Pitch Dark, whose record keeps paths that activate SP and pass over
    // fills: its base, every stored path's scene, and the no-path scene.
    const app::AnalysisSettings& cfg = default_cfg();
    const std::string chart = corpus::chart_named("test_pitch_dark_ratio");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus_song_with_notes(chart);
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    check_fallback_reads_spans(build_preview_base(song), chart + " base");
    for (const Path* p : rec.all_paths())
        check_fallback_reads_spans(build_preview_scene(song, p, cfg.sp_cap, cfg.rules),
                                   chart + " path");
    check_fallback_reads_spans(build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules),
                               chart + " no path");
}

TEST_CASE("rebuild_overlay_fields: equals a full build on random overlays") {
    const std::vector<PreviewScene> scenes = random_overlay_scenes();
    // Every scene to every other, the empty overlay included both ways.
    for (size_t i = 0; i < scenes.size(); ++i)
        for (size_t j = 0; j < scenes.size(); ++j)
            check_rebuild(scenes[i], scenes[j],
                          "overlay " + std::to_string(i) + " -> " + std::to_string(j));
}

// Every stored path of the corpus charts at every 8th place in chart_paths()
// order (places 0, 8, 16, ...). The random-overlay test above covers the
// property; this one keeps it honest on real charts. A sample, not the whole
// corpus, because each swap costs about 40 ms and the whole corpus took 20 s
// (user decision, D183). Every chart the corpus scan lists has notes, so a
// chart in the sample that loads empty fails rather than being passed over.
TEST_CASE("rebuild_overlay_fields: equals a full build on every 8th corpus chart's stored paths") {
    constexpr size_t kEvery = 8;
    const app::AnalysisSettings& cfg = default_cfg();
    const std::vector<std::string>& charts = corpus::chart_paths();
    // An empty list would check nothing: chart_paths throws on a broken tree,
    // so an empty one means the corpus is gone.
    REQUIRE_FALSE(charts.empty());
    for (size_t place = 0; place < charts.size(); place += kEvery) {
        const std::string& chart = charts[place];
        INFO("corpus chart " << chart);
        const Song& song = corpus_song_with_notes(chart);
        const std::vector<const Path*> paths = corpus::analyzed(chart, cfg).all_paths();
        // The base (what the Preview's first scene job builds the timeline
        // from), then each stored path's scene in turn, then the no-path
        // scene, so every path's scene is swapped in from the one before it
        // and out to the one after.
        std::vector<PreviewScene> scenes{build_preview_base(song)};
        for (const Path* p : paths) scenes.push_back(build_preview_scene(song, p, cfg.sp_cap, cfg.rules));
        scenes.push_back(build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules));
        for (size_t i = 0; i + 1 < scenes.size(); ++i)
            check_rebuild(scenes[i], scenes[i + 1], chart + " scene " + std::to_string(i));
    }
}

// One tick per millisecond (timed_scene), so a fill's drawn end edge sits half
// a millisecond past its last note and the next tick a millisecond past it.
// Scene fill 0 is hidden, 1 offered over [1.0, 2.0] s, 2 taken over [3.0, 4.0]
// s; the answer is the scene's index, not the place among the lit fills.
TEST_CASE("fill_at names the fill lit at t and none between fills") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red),    note(600.0, PreviewLane::Red),
                   note(1000.0, PreviewLane::Red), note(2000.0, PreviewLane::Green),
                   note(3000.0, PreviewLane::Red), note(4000.0, PreviewLane::Green),
                   note(5000.0, PreviewLane::Red)};
    scene.fills = {fill(span(500.0, 800.0), PreviewFillState::Hidden),
                   fill(span(1000.0, 2000.0), PreviewFillState::Offered),
                   fill(span(3000.0, 4000.0), PreviewFillState::Taken)};
    scene.activations = {lane_activation(4000, PreviewLane::Green, 2)};
    const TrackState st = build_track_state(scene, TrackStateOptions{});

    // The drawn end edges, read off the timeline: the instant where each
    // fill's span ends.
    const TrackInstant* offered_end = nullptr;
    const TrackInstant* taken_end = nullptr;
    for (const TrackInstant& i : st.instants()) {
        if (i.fill == Toggle::End) offered_end = &i;
        if (i.fill_taken == Toggle::End) taken_end = &i;
    }
    REQUIRE(offered_end);
    REQUIRE(taken_end);
    CHECK(offered_end->t == doctest::Approx(2.0005));
    CHECK(taken_end->t == doctest::Approx(4.0005));

    // The hidden fill is never lit.
    CHECK_FALSE(st.fill_at(0.6).has_value());
    // The offered fill: lit from its first note through its last, up to the
    // drawn end edge, which the strip stops at.
    CHECK(st.fill_at(1.0) == std::optional<size_t>(1));
    CHECK(st.fill_at(2.0) == std::optional<size_t>(1));
    CHECK(st.fill_at(2.00025) == std::optional<size_t>(1));
    CHECK_FALSE(st.fill_at(offered_end->t).has_value());
    CHECK_FALSE(st.fill_at(2.001).has_value());  // one tick past the last note
    // Between the fills.
    CHECK_FALSE(st.fill_at(2.5).has_value());
    // The taken fill, the same way.
    CHECK(st.fill_at(3.0) == std::optional<size_t>(2));
    CHECK(st.fill_at(4.0) == std::optional<size_t>(2));
    CHECK(st.fill_at(4.00025) == std::optional<size_t>(2));
    CHECK_FALSE(st.fill_at(taken_end->t).has_value());
    CHECK_FALSE(st.fill_at(4.001).has_value());

    // Every stretch the highway lights answers its own fill at the stretch's
    // start and middle.
    const double near_s = 0.0, far_s = 5.5;
    const TrackWindow win = st.window(near_s, far_s);
    auto lit_answers = [&](Toggle TrackInstant::*field, size_t want) {
        int lit = 0;
        for (const ToggleSpan& s : st.make_toggle_bounds(win, near_s, far_s, field)) {
            if (!s.on) continue;
            ++lit;
            CAPTURE(s.t1);
            CHECK(st.fill_at(s.t1) == std::optional<size_t>(want));
            CHECK(st.fill_at((s.t1 + s.t2) / 2.0) == std::optional<size_t>(want));
        }
        CHECK(lit == 1);
    };
    lit_answers(&TrackInstant::fill, 1);
    lit_answers(&TrackInstant::fill_taken, 2);
}
