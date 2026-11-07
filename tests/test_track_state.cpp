// Tests for render/track_state: the instant timeline the highway draws from
// (a port of Onyx's CommonState/DrumState maps), built from a PreviewScene.

#include "doctest.h"

#include <cstdint>
#include <cstdio>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
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

// ---- Pinned timelines -------------------------------------------------------
// Each hand-built case pins its whole timeline as text, printed once from
// build_track_state and checked by hand in the case's comment. One line per
// instant: the time in seconds, the gems ("R", "Yc" a yellow cymbal, "K" a
// kick, "g" ghost, "a" accent, "-" none), the beat line, then the six span
// states (od SP phrase, so solo, fi offered fill, ft taken fill, sp active SP,
// ln lit lane) and the lit lane's pad. A span state is "." absent, "S" start,
// "O" on, "R" restart, "E" end.

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

std::vector<std::string> instant_lines(const TrackState& st, bool show_cymbals = true) {
    std::vector<std::string> out;
    for (const TrackInstant& i : st.instants()) out.push_back(instant_line(i, show_cymbals));
    return out;
}

// The empty-window fallback between each pair of neighbouring instants: the
// one instant window() synthesizes at their midpoint.
std::vector<std::string> gap_lines(const TrackState& st) {
    std::vector<std::string> out;
    const std::vector<TrackInstant>& v = st.instants();
    for (size_t i = 0; i + 1 < v.size(); ++i) {
        TrackWindow w = st.window(v[i].t, v[i + 1].t);
        REQUIRE(w.size() == 1);
        out.push_back(instant_line(w[0]));
    }
    return out;
}

// Built lines against pinned ones, printed as literals on a mismatch
// (record_fixtures.h).
using test::check_lines;

// The scene's pinned timeline and gaps with pro drums on. With pro drums off
// the timeline is the same except that no gem is a cymbal.
void check_pinned(const PreviewScene& scene, const std::vector<std::string>& want_instants,
                  const std::vector<std::string>& want_gaps) {
    TrackState st = build_track_state(scene, TrackStateOptions{true});
    check_lines(instant_lines(st), want_instants, "instants");
    check_lines(gap_lines(st), want_gaps, "gaps");

    TrackState flat = build_track_state(scene, TrackStateOptions{false});
    CHECK(instant_lines(flat, false) == instant_lines(st, false));
    for (const TrackInstant& i : flat.instants())
        for (const TrackGem& g : i.notes) CHECK_FALSE(g.cymbal);
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
    CHECK_FALSE(TrackStateOptions{true} == TrackStateOptions{false});
}

TEST_CASE("build_track_state: gems, pro-off, and the phrase end note reads inside") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(500.0, PreviewLane::Yellow, true, true),
                   note(500.0, PreviewLane::Kick), note(1000.0, PreviewLane::Green, true, false, true)};
    scene.sp_phrases = {span(500.0, 1000.0)};

    TrackState st = build_track_state(scene, TrackStateOptions{true});
    const auto& inst = st.instants();
    // Instants: 0.0, 0.5, 1.0, 1.0005 (phrase end + half a tick).
    REQUIRE(inst.size() == 4);
    CHECK(inst[0].t == doctest::Approx(0.0));
    CHECK(inst[1].t == doctest::Approx(0.5));
    CHECK(inst[2].t == doctest::Approx(1.0));
    CHECK(inst[3].t == doctest::Approx(1.0005));

    // Two gems at 0.5 s: a yellow ghost cymbal and a kick.
    REQUIRE(inst[1].notes.size() == 2);
    CHECK(inst[1].notes[0].pad == Pad::Yellow);
    CHECK(inst[1].notes[0].cymbal);
    CHECK(inst[1].notes[0].velocity == Velocity::Ghost);
    CHECK(inst[1].notes[1].kick);
    CHECK(inst[2].notes[0].velocity == Velocity::Accent);

    // The phrase starts at 0.5, is still on at its last note (1.0), ends after.
    CHECK(inst[0].overdrive == Toggle::Empty);
    CHECK(inst[1].overdrive == Toggle::Start);
    CHECK(inst[2].overdrive == Toggle::On);
    CHECK(inst[3].overdrive == Toggle::End);

    // Pro off: no cymbals anywhere.
    TrackState flat = build_track_state(scene, TrackStateOptions{false});
    for (const TrackInstant& i : flat.instants())
        for (const TrackGem& g : i.notes) CHECK_FALSE(g.cymbal);
    // The whole timeline, and between instants the phrase reads on from its
    // start until its end edge and absent elsewhere.
    check_pinned(scene,
                 {"0.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.500000 Ycg,K - od=S so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.000000 Gca - od=O so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.000500 - - od=E so=. fi=. ft=. sp=. ln=. pad=-"},
                 {"0.250000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.750000 - - od=O so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.000250 - - od=O so=. fi=. ft=. sp=. ln=. pad=-"});
}

TEST_CASE("build_track_state: active SP window ends exactly at the deact node") {
    PreviewScene scene;
    scene.notes = {note(0.0, PreviewLane::Red), note(4000.0, PreviewLane::Red)};
    PreviewActivation a;
    a.tick = 1000;
    a.ms = 1000.0;
    a.has_sp_end = true;
    a.sp_end_tick = 3000;
    a.sp_end_ms = 3000.0;
    scene.activations = {a};

    TrackState st = build_track_state(scene, TrackStateOptions{});
    const TrackInstant* start = find(st.instants(), 1.0);
    const TrackInstant* end = find(st.instants(), 3.0);
    REQUIRE(start);
    REQUIRE(end);
    CHECK(start->sp_active == Toggle::Start);
    CHECK(end->sp_active == Toggle::End);
    CHECK(find(st.instants(), 0.0)->sp_active == Toggle::Empty);
    CHECK(find(st.instants(), 4.0)->sp_active == Toggle::Empty);
    // No instant at 3.0005: the window is exact, not epsilon-extended.
    CHECK(find(st.instants(), 3.0005) == nullptr);
    check_pinned(scene,
                 {"0.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.000000 - - od=. so=. fi=. ft=. sp=S ln=. pad=-",
                  "3.000000 - - od=. so=. fi=. ft=. sp=E ln=. pad=-",
                  "4.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-"},
                 {"0.500000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "2.000000 - - od=. so=. fi=. ft=. sp=O ln=. pad=-",
                  "3.500000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-"});
}

TEST_CASE("build_track_state: the sp_active sweep agrees with sp_running_at") {
    // The highway's sweep keeps its own walk (it serves six span kinds), so
    // this pins its sp_active answer to the owner, PreviewActivation::
    // sp_running_at, at every edge of two back-to-back windows: [1000, 2000)
    // and [2000, 3000) ms, the scene of test_sp_meter_curve's sp_running_at
    // case.
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
    for (const double t_ms : {500.0, 1000.0, 1500.0, 2000.0, 3000.0, 4000.0}) {
        INFO("t = " << t_ms << " ms");
        // The sweep's state at t: the instant at t when an edge puts one
        // there, else the one window() synthesizes at t.
        const double t_s = t_ms / 1000.0;
        const TrackWindow win = st.window(t_s - 0.1, t_s + 0.1);
        REQUIRE(win.size() == 1);
        CHECK(win[0].t == doctest::Approx(t_s));
        const bool owner = a.sp_running_at(t_ms) || b.sp_running_at(t_ms);
        CHECK(toggle_on_after(win[0].sp_active) == owner);
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

TEST_CASE("build_track_state: taken, offered and hidden fills toggle different spans") {
    PreviewScene scene = taken_offered_hidden_scene();

    TrackState st = build_track_state(scene, TrackStateOptions{});
    const TrackInstant* taken = find(st.instants(), 1.0);
    const TrackInstant* offered = find(st.instants(), 4.0);
    REQUIRE(taken);
    REQUIRE(offered);

    // The taken fill drives `fill_taken` and the lit lane, not `fill`.
    CHECK(taken->fill_taken == Toggle::Start);
    CHECK(taken->fill == Toggle::Empty);
    CHECK(taken->fill_lane == Toggle::Start);
    REQUIRE(taken->fill_lane_pad.has_value());
    CHECK(*taken->fill_lane_pad == Pad::Green);

    // The offered fill drives `fill` alone.
    CHECK(offered->fill == Toggle::Start);
    CHECK(offered->fill_taken == Toggle::Empty);
    CHECK(offered->fill_lane == Toggle::Empty);
    CHECK_FALSE(offered->fill_lane_pad.has_value());

    // The hidden fill produces no intervals at all: nothing toggles at 7.0.
    const TrackInstant* hidden = find(st.instants(), 7.0);
    CHECK(hidden == nullptr);
    const TrackInstant* hidden_end = find(st.instants(), 8.0);  // the note is there
    REQUIRE(hidden_end);
    CHECK(hidden_end->fill == Toggle::Empty);
    CHECK(hidden_end->fill_taken == Toggle::Empty);
    CHECK(hidden_end->fill_lane == Toggle::Empty);

    // The activation note itself (2.0) is inside both the taken fill and the lane.
    const TrackInstant* act = find(st.instants(), 2.0);
    REQUIRE(act);
    CHECK(act->fill_taken == Toggle::On);
    CHECK(act->fill_lane == Toggle::On);
    CHECK(act->fill == Toggle::Empty);
    // Taken [1.0, 2.0005] lights Green; offered [4.0, 5.0005]; the hidden
    // fill adds no instant at 7.0 or 8.0005.
    check_pinned(scene,
                 {"0.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.000000 - - od=. so=. fi=. ft=S sp=. ln=S pad=G",
                  "2.000000 G - od=. so=. fi=. ft=O sp=. ln=O pad=G",
                  "2.000500 - - od=. so=. fi=. ft=E sp=. ln=E pad=G",
                  "4.000000 - - od=. so=. fi=S ft=. sp=. ln=. pad=-",
                  "5.000000 G - od=. so=. fi=O ft=. sp=. ln=. pad=-",
                  "5.000500 - - od=. so=. fi=E ft=. sp=. ln=. pad=-",
                  "8.000000 G - od=. so=. fi=. ft=. sp=. ln=. pad=-"},
                 {"0.500000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.500000 - - od=. so=. fi=. ft=O sp=. ln=O pad=G",
                  "2.000250 - - od=. so=. fi=. ft=O sp=. ln=O pad=G",
                  "3.000250 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "4.500000 - - od=. so=. fi=O ft=. sp=. ln=. pad=-",
                  "5.000250 - - od=. so=. fi=O ft=. sp=. ln=. pad=-",
                  "6.500250 - - od=. so=. fi=. ft=. sp=. ln=. pad=-"});
}

// A ghost path's fills drive `fill_ghost` over their own spans, whatever the
// drawn path did with them: here the taken fill and the hidden one. Without a
// ghost no instant has the field on, and the timeline is the one pinned in
// the case above.
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
    REQUIRE(find(st.instants(), 0.0));
    CHECK(*find(st.instants(), 0.0)->beat == PreviewBeatKind::MeasureLine);
    CHECK(*find(st.instants(), 0.25)->beat == PreviewBeatKind::Half);
    CHECK(find(st.instants(), 0.25)->notes.size() == 1);
    CHECK(find(st.instants(), 0.25)->solo == Toggle::Start);
    CHECK(find(st.instants(), 0.2505)->solo == Toggle::End);
    check_pinned(scene,
                 {"0.000000 - bar od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.250000 R half od=. so=S fi=. ft=. sp=. ln=. pad=-",
                  "0.250500 - - od=. so=E fi=. ft=. sp=. ln=. pad=-",
                  "0.500000 - beat od=. so=. fi=. ft=. sp=. ln=. pad=-"},
                 {"0.125000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.250250 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "0.375250 - - od=. so=. fi=. ft=. sp=. ln=. pad=-"});
}

TEST_CASE("window: strict bounds, and a synthesized instant when empty") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(2000.0, PreviewLane::Red),
                   note(3000.0, PreviewLane::Red)};
    scene.solos = {span(900.0, 3100.0)};
    TrackState st = build_track_state(scene, TrackStateOptions{});

    TrackWindow w = st.window(1.0, 3.0);  // excludes both ends
    REQUIRE(w.size() == 1);
    CHECK(w[0].t == doctest::Approx(2.0));
    CHECK(w[0].solo == Toggle::On);

    // Nothing between 2.1 and 2.9: the synthesized midpoint instant carries
    // the ongoing solo.
    TrackWindow empty = st.window(2.1, 2.9);
    REQUIRE(empty.size() == 1);
    CHECK(empty[0].t == doctest::Approx(2.5));
    CHECK(empty[0].solo == Toggle::On);
    CHECK(empty[0].notes.empty());
    CHECK_FALSE(empty[0].beat.has_value());

    // Outside every span: synthesized Empty.
    TrackWindow before = st.window(0.1, 0.5);
    REQUIRE(before.size() == 1);
    CHECK(before[0].solo == Toggle::Empty);
    check_pinned(scene,
                 {"0.900000 - - od=. so=S fi=. ft=. sp=. ln=. pad=-",
                  "1.000000 R - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "2.000000 R - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "3.000000 R - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "3.100500 - - od=. so=E fi=. ft=. sp=. ln=. pad=-"},
                 {"0.950000 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "1.500000 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "2.500000 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "3.050250 - - od=. so=O fi=. ft=. sp=. ln=. pad=-"});
}

TEST_CASE("make_toggle_bounds: covers [near, far], merges equal neighbours") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(10000.0, PreviewLane::Red)};
    // Touching solos: with the half-tick end they overlap, so the second's start
    // and the first's end both read as On and the span never breaks.
    scene.solos = {span(1000.0, 2000.0), span(2000.0, 3000.0)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    CHECK(find(st.instants(), 2.0)->solo == Toggle::On);
    CHECK(find(st.instants(), 2.0005)->solo == Toggle::On);

    TrackWindow w = st.window(0.5, 4.0);
    std::vector<ToggleSpan> spans =
        st.make_toggle_bounds(w, 0.5, 4.0, &TrackInstant::solo);
    // off [0.5,1.0), on [1.0, 3.0005), off to 4.0
    REQUIRE(spans.size() == 3);
    CHECK(spans[0].t1 == doctest::Approx(0.5));
    CHECK(spans[0].t2 == doctest::Approx(1.0));
    CHECK_FALSE(spans[0].on);
    CHECK(spans[1].t1 == doctest::Approx(1.0));
    CHECK(spans[1].t2 == doctest::Approx(3.0005));
    CHECK(spans[1].on);
    CHECK(spans[2].t2 == doctest::Approx(4.0));
    CHECK_FALSE(spans[2].on);

    // A window opening mid-span starts "on".
    TrackWindow mid = st.window(1.5, 2.5);
    std::vector<ToggleSpan> mid_spans =
        st.make_toggle_bounds(mid, 1.5, 2.5, &TrackInstant::solo);
    REQUIRE(mid_spans.size() == 1);
    CHECK(mid_spans[0].on);
    CHECK(mid_spans[0].t1 == doctest::Approx(1.5));
    CHECK(mid_spans[0].t2 == doctest::Approx(2.5));
    // [1.0, 2.0005] and [2.0, 3.0005]: each edge at 2.0 and 2.0005 falls
    // inside the other solo, so both read On.
    check_pinned(scene,
                 {"0.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.000000 - - od=. so=S fi=. ft=. sp=. ln=. pad=-",
                  "2.000000 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "2.000500 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "3.000500 - - od=. so=E fi=. ft=. sp=. ln=. pad=-",
                  "10.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-"},
                 {"0.500000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.500000 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "2.000250 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "2.500500 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "6.500250 - - od=. so=. fi=. ft=. sp=. ln=. pad=-"});
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
    // At 300 BPM and 480 ticks a beat a tick is 1/2400 s: tick 480 is 0.2 s,
    // the phrase's end edge (tick 480.5) 0.200208 s, tick 481 0.200417 s.
    check_pinned(scene,
                 {"0.000000 R - od=S so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.200000 Y - od=O so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.200208 - - od=E so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.200417 B - od=. so=. fi=. ft=. sp=. ln=. pad=-"},
                 {"0.100000 - - od=O so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.200104 - - od=O so=. fi=. ft=. sp=. ln=. pad=-",
                  "0.200313 - - od=. so=. fi=. ft=. sp=. ln=. pad=-"});
}

TEST_CASE("build_track_state: spans need the song timing") {
    PreviewScene scene;  // no timing
    scene.notes = {note(1000.0, PreviewLane::Red)};
    scene.sp_phrases = {span(1000.0, 1000.0)};
    CHECK_THROWS_AS(build_track_state(scene, TrackStateOptions{}), std::invalid_argument);

    // A scene with no spans still builds without timing.
    PreviewScene plain;
    plain.notes = {note(1000.0, PreviewLane::Red)};
    CHECK(build_track_state(plain, TrackStateOptions{}).instants().size() == 1);
    check_pinned(plain, {"1.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-"}, {});
}

// The renderer asks for this window every frame. It used to be a vector of
// copies, each instant with its own vector of notes.
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

    // Walking it backwards reaches the same objects.
    CHECK(&*w.rbegin() == &w[w.size() - 1]);
    check_pinned(scene,
                 {"1.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "2.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "3.000000 R - od=. so=. fi=. ft=. sp=. ln=. pad=-"},
                 {"1.500000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "2.500000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-"});
}

// The one-pass builder on a messy scene, every awkward interval shape at
// least once, pinned instant by instant. 1 tick = 1 ms, so a span's end edge
// is its end tick plus 0.5 ms.
TEST_CASE("build_track_state: the one-pass sweep on overlapping, touching, empty and backwards spans") {
    PreviewScene scene = timed_scene();
    // SP phrases that overlap: [1.0, 2.0005] and [1.5, 2.5005].
    scene.sp_phrases = {span(1000.0, 2000.0), span(1500.0, 2500.0)};
    // Solos: zero-length ([3.0005, 3.0005]: it starts on its own end edge),
    // backwards ([3.2, 3.1005]), and one nested in another ([4.2, 4.8005]
    // inside [4.0, 5.0005]).
    PreviewSpan zero = span(3000.0, 3000.0);
    zero.start_ms = 3000.5;
    PreviewSpan backwards = span(3200.0, 3100.0);
    scene.solos = {zero, backwards, span(4000.0, 5000.0), span(4200.0, 4800.0)};
    // Fills: an offered one, two taken ones that touch (the second starts on
    // the first's end edge, 7.5005), and a hidden one that draws nothing.
    PreviewSpan touching = span(7500.0, 8000.0);
    touching.start_ms = 7500.5;
    scene.fills = {fill(span(6000.0, 6500.0), PreviewFillState::Offered),
                   fill(span(7000.0, 7500.0), PreviewFillState::Taken),
                   fill(touching, PreviewFillState::Taken),
                   fill(span(8500.0, 8800.0), PreviewFillState::Hidden)};
    // Activations. SP windows: an ordinary one [1.2, 3.0], one that starts
    // where it ends (dropped), one that ends before it starts (dropped), one
    // with no stored end (dropped). Lit lanes: Green then Yellow on the first
    // taken fill (the first listed wins), Blue on the touching one, a kick on
    // it too (no pad: dropped), and Red on a tick with no fill (dropped).
    auto act = [](double ms, double end_ms, bool has_end) {
        PreviewActivation a;
        a.ms = ms;
        a.tick = static_cast<int64_t>(ms);
        a.has_sp_end = has_end;
        a.sp_end_ms = end_ms;
        a.sp_end_tick = static_cast<int64_t>(end_ms);
        return a;
    };
    scene.activations = {act(1200.0, 3000.0, true), act(3500.0, 3500.0, true),
                         act(3600.0, 3400.0, true), act(3700.0, 9000.0, false),
                         lane_activation(7500, PreviewLane::Green, 1),
                         lane_activation(7500, PreviewLane::Yellow, 1),
                         lane_activation(8000, PreviewLane::Blue, 2),
                         lane_activation(8000, PreviewLane::Kick, 2),
                         lane_activation(9500, PreviewLane::Red, std::nullopt)};
    // Notes out of time order; a red cymbal (never a cymbal on the highway),
    // a ghost and an accent; one note on the SP phrase's end edge (2.0005).
    scene.notes = {note(5000.0, PreviewLane::Green, false, false, true),
                   note(1000.0, PreviewLane::Red, true),
                   note(1000.0, PreviewLane::Kick),
                   note(2000.5, PreviewLane::Blue, true, true),
                   note(7500.0, PreviewLane::Yellow, true)};
    // Two beats on one moment (the last one listed wins), and one on an edge.
    scene.beats = {{0, 2000.5, PreviewBeatKind::MeasureLine}, {0, 2000.5, PreviewBeatKind::Half},
                   {0, 6000.0, PreviewBeatKind::Beat}};

    // By hand: at 1.0 the red cymbal draws as a plain red. At 2.0005 the
    // first phrase's end edge sits inside the second phrase, so od stays On.
    // The zero-length solo restarts at 3.0005 and is absent either side. The
    // backwards solo ends at 3.1005 and starts at 3.2 but covers nothing
    // between them or after. The nested solo's edges (4.2, 4.8005) read On.
    // At 7.5005 one taken fill ends as the next starts: Restart, and the
    // starting fill's Blue wins. The three dropped SP windows, the kick lane,
    // the red lane on no fill and the hidden fill add no instant.
    check_pinned(scene,
                 {"1.000000 R,K - od=S so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.200000 - - od=O so=. fi=. ft=. sp=S ln=. pad=-",
                  "1.500000 - - od=O so=. fi=. ft=. sp=O ln=. pad=-",
                  "2.000500 Bcg half od=O so=. fi=. ft=. sp=O ln=. pad=-",
                  "2.500500 - - od=E so=. fi=. ft=. sp=O ln=. pad=-",
                  "3.000000 - - od=. so=. fi=. ft=. sp=E ln=. pad=-",
                  "3.000500 - - od=. so=R fi=. ft=. sp=. ln=. pad=-",
                  "3.100500 - - od=. so=E fi=. ft=. sp=. ln=. pad=-",
                  "3.200000 - - od=. so=S fi=. ft=. sp=. ln=. pad=-",
                  "4.000000 - - od=. so=S fi=. ft=. sp=. ln=. pad=-",
                  "4.200000 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "4.800500 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "5.000000 Ga - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "5.000500 - - od=. so=E fi=. ft=. sp=. ln=. pad=-",
                  "6.000000 - beat od=. so=. fi=S ft=. sp=. ln=. pad=-",
                  "6.500500 - - od=. so=. fi=E ft=. sp=. ln=. pad=-",
                  "7.000000 - - od=. so=. fi=. ft=S sp=. ln=S pad=G",
                  "7.500000 Yc - od=. so=. fi=. ft=O sp=. ln=O pad=G",
                  "7.500500 - - od=. so=. fi=. ft=R sp=. ln=R pad=B",
                  "8.000500 - - od=. so=. fi=. ft=E sp=. ln=E pad=B"},
                 {"1.100000 - - od=O so=. fi=. ft=. sp=. ln=. pad=-",
                  "1.350000 - - od=O so=. fi=. ft=. sp=O ln=. pad=-",
                  "1.750250 - - od=O so=. fi=. ft=. sp=O ln=. pad=-",
                  "2.250500 - - od=O so=. fi=. ft=. sp=O ln=. pad=-",
                  "2.750250 - - od=. so=. fi=. ft=. sp=O ln=. pad=-",
                  "3.000250 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "3.050500 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "3.150250 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "3.600000 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "4.100000 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "4.500250 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "4.900250 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "5.000250 - - od=. so=O fi=. ft=. sp=. ln=. pad=-",
                  "5.500250 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "6.250250 - - od=. so=. fi=O ft=. sp=. ln=. pad=-",
                  "6.750250 - - od=. so=. fi=. ft=. sp=. ln=. pad=-",
                  "7.250000 - - od=. so=. fi=. ft=O sp=. ln=O pad=G",
                  "7.500250 - - od=. so=. fi=. ft=O sp=. ln=O pad=G",
                  "7.750500 - - od=. so=. fi=. ft=O sp=. ln=O pad=B"});
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

// The path overlay never touches the drum lanes: swapping in a path keeps
// them, and the result equals a fresh build.
TEST_CASE("rebuild_overlay_fields: keeps the drum lanes") {
    PreviewScene from = taken_offered_hidden_scene();
    from.drum_lanes = {drum_lane(span(1000.0, 2000.0), {PreviewLane::Yellow, PreviewLane::Green}),
                       drum_lane(span(3500.0, 4000.0), {PreviewLane::Blue})};
    PreviewScene to = from;
    to.activations.clear();
    for (PreviewFill& f : to.fills) f.state = PreviewFillState::Hidden;
    for (const PreviewScene* s : {&from, &to}) {
        TrackState st = build_track_state(s == &from ? to : from, TrackStateOptions{});
        rebuild_overlay_fields(st, *s);
        const TrackState want = build_track_state(*s, TrackStateOptions{});
        REQUIRE(st.instants().size() == want.instants().size());
        for (size_t i = 0; i < want.instants().size(); ++i)
            CHECK(same_instant(st.instants()[i], want.instants()[i]));
        const TrackInstant* blue = find(st.instants(), 3.5);
        REQUIRE(blue);
        CHECK(blue->drum_lane_blue == Toggle::Start);
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

// synthesize(t) at each instant's own time, held to the instant's span fields
// (the instant's notes and beat are not span fields, so they are left out).
void check_synthesize_agrees(const PreviewScene& scene, const std::string& what) {
    const TrackState st = build_track_state(scene, TrackStateOptions{true});
    size_t mismatches = 0, first = 0;
    for (size_t i = 0; i < st.instants().size(); ++i) {
        TrackInstant want = st.instants()[i];
        want.notes.clear();
        want.beat.reset();
        if (!same_instant(st.synthesize(want.t), want) && mismatches++ == 0) first = i;
    }
    INFO(what << ", first mismatch at instant " << first << ": "
              << (st.instants().empty() ? std::string() : instant_line(st.instants()[first])));
    CHECK(mismatches == 0);
}

}  // namespace

TEST_CASE("synthesize at an instant's own time agrees with the sweep that filled it") {
    const std::vector<PreviewScene> scenes = random_overlay_scenes();
    for (size_t i = 0; i < scenes.size(); ++i)
        check_synthesize_agrees(scenes[i], "overlay " + std::to_string(i));

    // One corpus chart: the first with notes, its base and every stored
    // path's scene.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        check_synthesize_agrees(build_preview_base(song), chart + " base");
        for (const Path* p : rec.all_paths())
            check_synthesize_agrees(build_preview_scene(song, p, cfg.sp_cap, cfg.rules),
                                    chart + " path");
        check_synthesize_agrees(build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules),
                                chart + " no path");
        break;
    }
}

TEST_CASE("rebuild_overlay_fields: equals a full build on random overlays") {
    const std::vector<PreviewScene> scenes = random_overlay_scenes();
    // Every scene to every other, the empty overlay included both ways.
    for (size_t i = 0; i < scenes.size(); ++i)
        for (size_t j = 0; j < scenes.size(); ++j)
            check_rebuild(scenes[i], scenes[j],
                          "overlay " + std::to_string(i) + " -> " + std::to_string(j));
}

// Every stored path of every 8th corpus chart with notes, in chart_paths()
// order. The random-overlay test above covers the property; this one keeps it
// honest on real charts. A sample, not the whole corpus, because each swap
// costs about 40 ms and the whole corpus took 20 s (user decision 2026-10-10).
TEST_CASE("rebuild_overlay_fields: equals a full build on every 8th corpus chart's stored paths") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int with_notes = 0, charts = 0, swaps = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        if (with_notes++ % 8 != 0) continue;
        ++charts;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        // The base (what the Preview's first scene job builds the timeline
        // from), then each stored path's scene in turn.
        std::vector<PreviewScene> scenes{build_preview_base(song)};
        for (const Path* p : rec.all_paths())
            scenes.push_back(build_preview_scene(song, p, cfg.sp_cap, cfg.rules));
        scenes.push_back(build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules));
        for (size_t i = 0; i + 1 < scenes.size(); ++i) {
            ++swaps;
            check_rebuild(scenes[i], scenes[i + 1], chart + " scene " + std::to_string(i));
        }
    }
    CHECK(charts > 0);
    CHECK(swaps > charts);
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
