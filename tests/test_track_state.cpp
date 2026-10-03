// Tests for render/track_state: the instant timeline the highway draws from
// (a port of Onyx's CommonState/DrumState maps), built from a PreviewScene.

#include "doctest.h"

#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
#include "corpus_util.h"
#include "render/track_state.h"

using namespace hydra;
using namespace hydra::app;
using namespace hydra::render;

namespace {

PreviewNote note(double ms, PreviewLane lane, bool cymbal = false, bool ghost = false,
                 bool accent = false) {
    PreviewNote n;
    n.ms = ms;
    n.tick = static_cast<int64_t>(ms);  // 1 tick per ms keeps ticks distinct
    n.lane = lane;
    n.cymbal = cymbal;
    n.ghost = ghost;
    n.accent = accent;
    return n;
}

PreviewSpan span(double start_ms, double end_ms) {
    PreviewSpan s;
    s.start_ms = start_ms;
    s.end_ms = end_ms;
    s.start_tick = static_cast<int64_t>(start_ms);
    s.end_tick = static_cast<int64_t>(end_ms);
    return s;
}

PreviewFill fill(PreviewSpan s, PreviewFillState state) {
    PreviewFill f;
    f.span = s;
    f.state = state;
    return f;
}

// One tick per millisecond (60 BPM at 1000 ticks per beat), matching note()
// and span() above, so half a tick is 0.5 ms and the edges these cases expect
// (1.0005, 0.2505, ...) are the same as before spans moved to ticks.
PreviewScene timed_scene() {
    PreviewScene s;
    s.timing = SongTiming(1000, {{0, 4000}}, {{0, 60.0}});
    s.tick_resolution = 1000;
    return s;
}

const TrackInstant* find(const std::vector<TrackInstant>& v, double t) {
    for (const TrackInstant& i : v)
        if (i.t == doctest::Approx(t)) return &i;
    return nullptr;
}

// ---- The old builder, kept here as the reference ---------------------------
// This is build_track_state as it was before the one-pass sweep: a std::map
// of instants, and every span field of every instant computed by toggle_at
// over all intervals. The real builder must match it exactly.

std::optional<Pad> ref_pad_of(PreviewLane lane) {
    switch (lane) {
        case PreviewLane::Red:    return Pad::Red;
        case PreviewLane::Yellow: return Pad::Yellow;
        case PreviewLane::Blue:   return Pad::Blue;
        case PreviewLane::Green:  return Pad::Green;
        case PreviewLane::Kick:   break;
    }
    return std::nullopt;
}

TrackGem ref_gem_of(const PreviewNote& n, bool pro) {
    TrackGem g;
    if (n.lane == PreviewLane::Kick) {
        g.kick = true;
    } else {
        g.pad = *ref_pad_of(n.lane);
        g.cymbal = pro && n.cymbal && n.lane != PreviewLane::Red;
    }
    g.velocity = n.ghost ? Velocity::Ghost : n.accent ? Velocity::Accent : Velocity::Normal;
    return g;
}

struct RefSpans {
    using Iv = std::pair<double, double>;
    std::vector<Iv> overdrive, solo, fill, fill_taken, sp_active;
    std::vector<std::pair<Iv, Pad>> fill_lane;

    explicit RefSpans(const PreviewScene& scene) {
        auto span_iv = [&scene](const PreviewSpan& s) {
            const double end_ms = scene.timing->ms_index().ms_at_tick_f(
                static_cast<double>(s.end_tick) + 0.5);
            return Iv{s.start_ms / 1000.0, end_ms / 1000.0};
        };
        for (const PreviewSpan& s : scene.sp_phrases) overdrive.push_back(span_iv(s));
        for (const PreviewSpan& s : scene.solos) solo.push_back(span_iv(s));
        for (const PreviewFill& f : scene.fills) {
            if (f.state == PreviewFillState::Offered)
                fill.push_back(span_iv(f.span));
            else if (f.state == PreviewFillState::Taken)
                fill_taken.push_back(span_iv(f.span));
        }
        for (const PreviewActivation& a : scene.activations) {
            if (a.has_sp_end && a.sp_end_ms > a.ms)
                sp_active.push_back({a.ms / 1000.0, a.sp_end_ms / 1000.0});
            if (a.has_lane) {
                std::optional<Pad> pad = ref_pad_of(a.lane);
                if (!pad) continue;
                for (const PreviewFill& f : scene.fills) {
                    if (f.state != PreviewFillState::Taken || f.span.end_tick != a.tick) continue;
                    fill_lane.push_back({span_iv(f.span), *pad});
                    break;
                }
            }
        }
    }

    TrackInstant synthesize(double t) const {
        TrackInstant inst;
        inst.t = t;
        inst.overdrive = toggle_at(overdrive, t);
        inst.solo = toggle_at(solo, t);
        inst.fill = toggle_at(fill, t);
        inst.fill_taken = toggle_at(fill_taken, t);
        inst.sp_active = toggle_at(sp_active, t);
        std::vector<Iv> lane_ivs;
        for (const auto& li : fill_lane) lane_ivs.push_back(li.first);
        inst.fill_lane = toggle_at(lane_ivs, t);
        if (inst.fill_lane != Toggle::Empty) {
            for (const auto& li : fill_lane)
                if (li.first.first == t || (li.first.first < t && t < li.first.second)) {
                    inst.fill_lane_pad = li.second;
                    break;
                }
            if (!inst.fill_lane_pad)
                for (const auto& li : fill_lane)
                    if (li.first.second == t) {
                        inst.fill_lane_pad = li.second;
                        break;
                    }
        }
        return inst;
    }
};

std::vector<TrackInstant> reference_build(const PreviewScene& scene, bool pro) {
    RefSpans spans(scene);
    std::map<double, TrackInstant> by_time;
    auto at = [&](double t) -> TrackInstant& {
        TrackInstant& inst = by_time[t];
        inst.t = t;
        return inst;
    };
    for (const PreviewNote& n : scene.notes) at(n.ms / 1000.0).notes.push_back(ref_gem_of(n, pro));
    for (const PreviewBeat& b : scene.beats) at(b.ms / 1000.0).beat = b.kind;
    for (const auto* ivs : {&spans.overdrive, &spans.solo, &spans.fill, &spans.fill_taken,
                            &spans.sp_active})
        for (const auto& iv : *ivs) {
            at(iv.first);
            at(iv.second);
        }
    for (const auto& li : spans.fill_lane) {
        at(li.first.first);
        at(li.first.second);
    }
    std::vector<TrackInstant> out;
    for (auto& kv : by_time) {
        TrackInstant inst = std::move(kv.second);
        TrackInstant filled = spans.synthesize(inst.t);
        inst.overdrive = filled.overdrive;
        inst.solo = filled.solo;
        inst.fill = filled.fill;
        inst.fill_taken = filled.fill_taken;
        inst.sp_active = filled.sp_active;
        inst.fill_lane = filled.fill_lane;
        inst.fill_lane_pad = filled.fill_lane_pad;
        out.push_back(std::move(inst));
    }
    return out;
}

bool same_gem(const TrackGem& a, const TrackGem& b) {
    return a.kick == b.kick && (a.kick || a.pad == b.pad) && a.cymbal == b.cymbal &&
           a.velocity == b.velocity;
}

// Every field of an instant: time, gems in order, beat, the seven span states.
bool same_instant(const TrackInstant& a, const TrackInstant& b) {
    if (a.t != b.t || a.notes.size() != b.notes.size() || a.beat != b.beat) return false;
    for (size_t i = 0; i < a.notes.size(); ++i)
        if (!same_gem(a.notes[i], b.notes[i])) return false;
    return a.overdrive == b.overdrive && a.solo == b.solo && a.fill == b.fill &&
           a.fill_taken == b.fill_taken && a.sp_active == b.sp_active &&
           a.fill_lane == b.fill_lane && a.fill_lane_pad == b.fill_lane_pad;
}

// The real builder against the reference, with pro drums on and off. Also
// checks the empty-window fallback between every pair of neighbouring
// instants against the reference's synthesize.
void check_same_as_reference(const PreviewScene& scene) {
    for (bool pro : {true, false}) {
        TrackState st = build_track_state(scene, TrackStateOptions{pro});
        std::vector<TrackInstant> ref = reference_build(scene, pro);
        REQUIRE(st.instants().size() == ref.size());
        size_t mismatches = 0;
        for (size_t i = 0; i < ref.size(); ++i)
            if (!same_instant(st.instants()[i], ref[i])) {
                if (mismatches++ < 5) {
                    INFO("instant " << i << " at t=" << ref[i].t);
                    CHECK(same_instant(st.instants()[i], ref[i]));
                }
            }
        CHECK(mismatches == 0);

        if (!pro) continue;
        RefSpans spans(scene);
        size_t synth_mismatches = 0;
        for (size_t i = 0; i + 1 < ref.size(); ++i) {
            const double near_s = ref[i].t, far_s = ref[i + 1].t;
            TrackWindow w = st.window(near_s, far_s);
            REQUIRE(w.size() == 1);
            if (!same_instant(w[0], spans.synthesize((near_s + far_s) / 2.0))) ++synth_mismatches;
        }
        CHECK(synth_mismatches == 0);
    }
}

}  // namespace

TEST_CASE("toggle_at: Onyx makeToggle truth table") {
    std::vector<std::pair<double, double>> ivs{{1.0, 2.0}, {2.0, 3.0}};
    CHECK(toggle_at(ivs, 0.5) == Toggle::Empty);
    CHECK(toggle_at(ivs, 1.0) == Toggle::Start);
    CHECK(toggle_at(ivs, 1.5) == Toggle::On);
    CHECK(toggle_at(ivs, 2.0) == Toggle::Restart);  // one ends, the next starts
    CHECK(toggle_at(ivs, 3.0) == Toggle::End);
    CHECK(toggle_at(ivs, 3.5) == Toggle::Empty);
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
    check_same_as_reference(scene);
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
    check_same_as_reference(scene);
}

TEST_CASE("build_track_state: taken, offered and hidden fills toggle different spans") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(2000.0, PreviewLane::Green),
                   note(5000.0, PreviewLane::Green), note(8000.0, PreviewLane::Green)};
    scene.fills = {fill(span(1000.0, 2000.0), PreviewFillState::Taken),
                   fill(span(4000.0, 5000.0), PreviewFillState::Offered),
                   fill(span(7000.0, 8000.0), PreviewFillState::Hidden)};
    PreviewActivation a;
    a.tick = 2000;
    a.ms = 2000.0;
    a.has_lane = true;
    a.lane = PreviewLane::Green;
    scene.activations = {a};

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
    check_same_as_reference(scene);
}

TEST_CASE("build_track_state: beats land on instants; solo toggles") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(250.0, PreviewLane::Red)};
    scene.beats = {{0, 0.0, PreviewBeatKind::Bar}, {0, 250.0, PreviewBeatKind::Half},
                   {0, 500.0, PreviewBeatKind::Beat}};
    scene.solos = {span(250.0, 250.0)};  // a one-note solo

    TrackState st = build_track_state(scene, TrackStateOptions{});
    REQUIRE(find(st.instants(), 0.0));
    CHECK(*find(st.instants(), 0.0)->beat == PreviewBeatKind::Bar);
    CHECK(*find(st.instants(), 0.25)->beat == PreviewBeatKind::Half);
    CHECK(find(st.instants(), 0.25)->notes.size() == 1);
    CHECK(find(st.instants(), 0.25)->solo == Toggle::Start);
    CHECK(find(st.instants(), 0.2505)->solo == Toggle::End);
    check_same_as_reference(scene);
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
    check_same_as_reference(scene);
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
    check_same_as_reference(scene);
}

TEST_CASE("build_track_state: a chord one tick after a phrase ends is not SP (480 res, 300 BPM)") {
    // One tick here is 0.417 ms, shorter than the old half-millisecond margin.
    SongTiming timing(480, {{0, 1920}}, {{0, 300.0}});
    auto at_tick = [&](int64_t tick, PreviewLane lane) {
        PreviewNote n;
        n.tick = tick;
        n.ms = timing.ms_index().at(tick);
        n.lane = lane;
        return n;
    };
    PreviewScene scene;
    scene.timing = timing;
    scene.tick_resolution = 480;
    scene.notes = {at_tick(0, PreviewLane::Red), at_tick(480, PreviewLane::Yellow),
                   at_tick(481, PreviewLane::Blue)};
    PreviewSpan phrase;
    phrase.start_tick = 0;
    phrase.end_tick = 480;
    phrase.start_ms = timing.ms_index().at(0);
    phrase.end_ms = timing.ms_index().at(480);
    scene.sp_phrases = {phrase};

    TrackState st = build_track_state(scene, TrackStateOptions{});
    const TrackInstant* last_in = find(st.instants(), timing.ms_index().at(480) / 1000.0);
    const TrackInstant* next = find(st.instants(), timing.ms_index().at(481) / 1000.0);
    REQUIRE(last_in);
    REQUIRE(next);
    CHECK(last_in->overdrive == Toggle::On);
    CHECK(next->overdrive == Toggle::Empty);
    check_same_as_reference(scene);
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
    check_same_as_reference(plain);
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
    check_same_as_reference(scene);
}

// The one-pass builder against the old per-instant one on a messy scene: a
// fixed seed, over 2,000 instants, and in every span field overlapping,
// touching, zero-length, nested and malformed (end before start) intervals.
// Notes arrive out of time order and share moments with beats and edges.
TEST_CASE("build_track_state: one pass matches the per-instant reference on a random scene") {
    std::mt19937 rng(20261003u);
    auto pick = [&rng](uint32_t n) { return static_cast<uint32_t>(rng() % n); };

    PreviewScene scene = timed_scene();
    const auto& ms_index = scene.timing->ms_index();
    constexpr uint32_t kSpanTicks = 4000;
    // Where a span with this end tick draws its end edge, in ms.
    auto edge_ms = [&](int64_t end_tick) {
        return ms_index.ms_at_tick_f(static_cast<double>(end_tick) + 0.5);
    };
    std::vector<double> edges;  // every edge so far, for touching spans and notes on edges

    auto make_span = [&](std::optional<PreviewSpan> prev) {
        PreviewSpan s;
        s.end_tick = pick(kSpanTicks);
        const double end_ms = edge_ms(s.end_tick);
        switch (pick(7)) {
            case 0:  // zero-length: starts exactly at its own end edge
                s.start_ms = end_ms;
                break;
            case 1:  // malformed: starts after its end edge
                s.start_ms = end_ms + 0.5 * (1 + pick(100));
                break;
            case 2:  // touching: starts at an earlier edge
                if (!edges.empty()) {
                    s.start_ms = edges[pick(static_cast<uint32_t>(edges.size()))];
                    if (s.start_ms > end_ms) s.end_tick = static_cast<int64_t>(s.start_ms) + 1;
                    break;
                }
                [[fallthrough]];
            case 3:  // nested inside the previous span of this field
                if (prev && prev->end_tick > 4) {
                    s.start_ms = prev->start_ms + 1.0;
                    s.end_tick = prev->end_tick - 2;
                    break;
                }
                [[fallthrough]];
            default:  // ordinary, often overlapping its neighbours
                s.start_ms = end_ms - 0.5 * (1 + pick(800));
                break;
        }
        s.start_tick = static_cast<int64_t>(s.start_ms);
        s.end_ms = edge_ms(s.end_tick);
        edges.push_back(s.start_ms);
        edges.push_back(edge_ms(s.end_tick));
        return s;
    };
    auto make_spans = [&](size_t n) {
        std::vector<PreviewSpan> v;
        std::optional<PreviewSpan> prev;
        for (size_t i = 0; i < n; ++i) {
            PreviewSpan s = make_span(prev);
            v.push_back(s);
            prev = s;
        }
        return v;
    };

    scene.sp_phrases = make_spans(60);
    scene.solos = make_spans(60);
    std::vector<PreviewSpan> offered = make_spans(60);
    std::vector<PreviewSpan> taken = make_spans(60);
    std::vector<PreviewSpan> hidden = make_spans(20);
    for (const PreviewSpan& s : offered) scene.fills.push_back(fill(s, PreviewFillState::Offered));
    for (const PreviewSpan& s : taken) scene.fills.push_back(fill(s, PreviewFillState::Taken));
    for (const PreviewSpan& s : hidden) scene.fills.push_back(fill(s, PreviewFillState::Hidden));

    // Activations: SP windows (some touching, some empty or backwards, so
    // dropped) and lit lanes on taken fills (some on a kick, so dropped; some
    // on no fill at all; several on the same fill).
    double prev_end = 0.0;
    for (int i = 0; i < 80; ++i) {
        PreviewActivation a;
        a.ms = pick(3) == 0 ? prev_end : 0.5 * pick(2 * kSpanTicks);
        a.tick = static_cast<int64_t>(a.ms);
        a.has_sp_end = pick(5) != 0;
        a.sp_end_ms = a.ms + 0.5 * (static_cast<double>(pick(1200)) - 100.0);
        a.sp_end_tick = static_cast<int64_t>(a.sp_end_ms);
        if (a.has_sp_end && a.sp_end_ms > a.ms) prev_end = a.sp_end_ms;
        if (pick(2) == 0) {
            a.has_lane = true;
            a.lane = static_cast<PreviewLane>(pick(5));
            a.tick = pick(4) == 0 ? static_cast<int64_t>(pick(kSpanTicks))
                                  : taken[pick(static_cast<uint32_t>(taken.size()))].end_tick;
        }
        scene.activations.push_back(a);
    }

    // Notes: chords, some on span edges, shuffled out of time order.
    for (int i = 0; i < 1900; ++i) {
        const double ms = pick(6) == 0 ? edges[pick(static_cast<uint32_t>(edges.size()))]
                                       : static_cast<double>(pick(kSpanTicks));
        const int chord = 1 + static_cast<int>(pick(3));
        for (int c = 0; c < chord; ++c)
            scene.notes.push_back(note(ms, static_cast<PreviewLane>(pick(5)), pick(2) == 0,
                                       pick(5) == 0, pick(5) == 0));
    }
    std::shuffle(scene.notes.begin(), scene.notes.end(), rng);

    // Beats on a quarter-millisecond grid, some on edges, some twice on one moment.
    for (int i = 0; i < 500; ++i) {
        PreviewBeat b;
        b.ms = pick(4) == 0 ? edges[pick(static_cast<uint32_t>(edges.size()))]
                            : 0.25 * pick(4 * kSpanTicks);
        b.kind = static_cast<PreviewBeatKind>(pick(3));
        scene.beats.push_back(b);
    }

    TrackState st = build_track_state(scene, TrackStateOptions{});
    CHECK(st.instants().size() >= 2000);
    check_same_as_reference(scene);
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
// lighting a lane on a taken fill.
void randomize_overlay(PreviewScene& scene, std::mt19937& rng, const std::vector<double>& moments) {
    auto pick = [&rng](uint32_t n) { return static_cast<uint32_t>(rng() % n); };
    auto moment = [&]() { return moments[pick(static_cast<uint32_t>(moments.size()))]; };
    scene.activations.clear();
    std::vector<int64_t> taken_ends;
    for (PreviewFill& f : scene.fills) {
        f.state = static_cast<PreviewFillState>(pick(3));
        if (f.state == PreviewFillState::Taken) taken_ends.push_back(f.span.end_tick);
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
            if (!taken_ends.empty() && pick(4) != 0)
                a.tick = taken_ends[pick(static_cast<uint32_t>(taken_ends.size()))];
        }
        scene.activations.push_back(a);
    }
}

}  // namespace

TEST_CASE("rebuild_overlay_fields: equals a full build on random overlays") {
    std::mt19937 rng(20261004u);
    auto pick = [&rng](uint32_t n) { return static_cast<uint32_t>(rng() % n); };

    // The path-free half: notes, beats, SP phrases, solos and fill windows,
    // many sharing moments.
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
    // Every scene to every other, the empty overlay included both ways.
    for (size_t i = 0; i < scenes.size(); ++i)
        for (size_t j = 0; j < scenes.size(); ++j)
            check_rebuild(scenes[i], scenes[j],
                          "overlay " + std::to_string(i) + " -> " + std::to_string(j));
}

TEST_CASE("rebuild_overlay_fields: equals a full build on every corpus chart and stored path") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int charts = 0, swaps = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
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
