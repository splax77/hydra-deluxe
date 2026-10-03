#include "render/track_state.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <stdexcept>

namespace hydra::render {

using app::PreviewActivation;
using app::PreviewBeat;
using app::PreviewLane;
using app::PreviewNote;
using app::PreviewScene;
using app::PreviewSpan;

namespace {

// Hydra marks an SP phrase, a solo or a fill by the tick of its last note;
// Onyx's span reaches past that note. The drawn edge sits half a tick after
// the last note: past it, and short of any note on the next tick, at every
// resolution and tempo. (A fixed half millisecond was not: at 480 ticks per
// beat and 300 BPM one tick is 0.417 ms.)
constexpr double kSpanEndTicks = 0.5;

using Iv = std::pair<double, double>;

double s_of(double ms) { return ms / 1000.0; }

std::optional<Pad> pad_of(PreviewLane lane) {
    switch (lane) {
        case PreviewLane::Red:    return Pad::Red;
        case PreviewLane::Yellow: return Pad::Yellow;
        case PreviewLane::Blue:   return Pad::Blue;
        case PreviewLane::Green:  return Pad::Green;
        case PreviewLane::Kick:   break;
    }
    return std::nullopt;
}

TrackGem gem_of(const PreviewNote& n, bool pro) {
    TrackGem g;
    if (n.lane == PreviewLane::Kick) {
        g.kick = true;  // 2x kicks are plain kicks (Onyx has no 2x visual)
    } else {
        g.pad = *pad_of(n.lane);
        g.cymbal = pro && n.cymbal && n.lane != PreviewLane::Red;
    }
    g.velocity = n.ghost ? Velocity::Ghost : n.accent ? Velocity::Accent : Velocity::Normal;
    return g;
}

// toggle_at for every instant of one span field, in one pass. Ask it for
// times in increasing order.
//
// At time t the reference rule needs three facts: does some interval start
// at t, does some end at t, and is t strictly inside some interval. The first
// two are lookups in the sorted starts and ends. For the third, count over
// the well-formed intervals (start <= end): the number inside is
//   (starts < t) - (ends <= t) + (zero-length intervals sitting at t),
// because an interval that ends at or before t also started at or before t,
// and it started exactly at t only when it is a zero-length one at t.
// Malformed intervals (end < start) can never hold t inside, so they stay out
// of that count; their edges still count as starts and ends, as before.
class SpanSweep {
public:
    explicit SpanSweep(const std::vector<Iv>& ivs) {
        for (const Iv& iv : ivs) {
            if (!std::isnan(iv.first)) all_starts_.push_back(iv.first);
            if (!std::isnan(iv.second)) all_ends_.push_back(iv.second);
            if (iv.first <= iv.second) {  // false for NaN as well
                starts_.push_back(iv.first);
                ends_.push_back(iv.second);
                if (iv.first == iv.second) zeros_.push_back(iv.first);
            }
        }
        std::sort(all_starts_.begin(), all_starts_.end());
        std::sort(all_ends_.begin(), all_ends_.end());
        std::sort(starts_.begin(), starts_.end());
        std::sort(ends_.begin(), ends_.end());
        std::sort(zeros_.begin(), zeros_.end());
    }

    Toggle at(double t) {
        while (i_s_ < starts_.size() && starts_[i_s_] < t) ++i_s_;
        while (i_e_ < ends_.size() && ends_[i_e_] <= t) ++i_e_;
        while (i_z_ < zeros_.size() && zeros_[i_z_] < t) ++i_z_;
        size_t zeros_at = 0;
        while (i_z_ + zeros_at < zeros_.size() && zeros_[i_z_ + zeros_at] == t) ++zeros_at;
        while (i_as_ < all_starts_.size() && all_starts_[i_as_] < t) ++i_as_;
        while (i_ae_ < all_ends_.size() && all_ends_[i_ae_] < t) ++i_ae_;

        const bool inside = static_cast<int64_t>(i_s_) - static_cast<int64_t>(i_e_) +
                                static_cast<int64_t>(zeros_at) > 0;
        const bool starts = i_as_ < all_starts_.size() && all_starts_[i_as_] == t;
        const bool ends = i_ae_ < all_ends_.size() && all_ends_[i_ae_] == t;
        if (inside) return Toggle::On;
        if (starts && ends) return Toggle::Restart;
        if (starts) return Toggle::Start;
        if (ends) return Toggle::End;
        return Toggle::Empty;
    }

private:
    std::vector<double> starts_, ends_, zeros_;  // well-formed intervals only
    std::vector<double> all_starts_, all_ends_;  // every interval's edges
    size_t i_s_ = 0, i_e_ = 0, i_z_ = 0, i_as_ = 0, i_ae_ = 0;
};

// The activated fill's lane at each instant, by the same rule as synthesize:
// the first interval (in list order) that starts at or contains t wins;
// failing that, the first one that ends at t. Ask it for times in increasing
// order. Well-formed intervals join a small active set when they start and
// leave it once they have ended (fills rarely overlap, so it stays tiny);
// malformed ones (end < start, or NaN) are checked every time, because their
// edges sit out of order.
class LaneSweep {
public:
    explicit LaneSweep(const std::vector<Iv>& ivs) : ivs_(ivs) {
        for (size_t i = 0; i < ivs.size(); ++i) {
            if (ivs[i].first <= ivs[i].second)
                order_.push_back(i);
            else
                malformed_.push_back(i);
        }
        std::stable_sort(order_.begin(), order_.end(),
                         [&ivs](size_t a, size_t b) { return ivs[a].first < ivs[b].first; });
    }

    // The index of the winning interval, or none.
    std::optional<size_t> at(double t) {
        while (next_ < order_.size() && ivs_[order_[next_]].first <= t)
            active_.push_back(order_[next_++]);
        active_.erase(std::remove_if(active_.begin(), active_.end(),
                                     [&](size_t i) { return ivs_[i].second < t; }),
                      active_.end());
        constexpr size_t kNone = std::numeric_limits<size_t>::max();
        size_t starts_or_holds = kNone, ends = kNone;
        auto consider = [&](size_t i) {
            const Iv& iv = ivs_[i];
            if (iv.first == t || (iv.first < t && t < iv.second))
                starts_or_holds = std::min(starts_or_holds, i);
            else if (iv.second == t)
                ends = std::min(ends, i);
        };
        for (size_t i : active_) consider(i);
        for (size_t i : malformed_) consider(i);
        if (starts_or_holds != kNone) return starts_or_holds;
        if (ends != kNone) return ends;
        return std::nullopt;
    }

private:
    const std::vector<Iv>& ivs_;
    std::vector<size_t> order_;      // well-formed, by start
    std::vector<size_t> malformed_;
    std::vector<size_t> active_;
    size_t next_ = 0;
};

// One moment something happens: a note, a beat, or a span edge. An edge is
// an SP phrase or solo edge (path-free) or an overlay edge (fill, taken fill,
// active SP, fill lane).
struct Event {
    enum class Kind : uint8_t { Note, Beat, Edge, OverlayEdge };
    double t;
    Kind kind;
    size_t index;  // into scene.notes or scene.beats; unused for an edge
};

// A span's [start, end] in seconds. The end edge comes from the span's end
// tick through the song's own timing (the display-only sub-tick lookup),
// never from its end ms.
std::pair<double, double> span_interval(const PreviewScene& scene, const PreviewSpan& s) {
    if (!scene.timing)
        throw std::invalid_argument("build_track_state: a scene with spans needs its song timing");
    const double end_ms =
        scene.timing->ms_index().ms_at_tick_f(static_cast<double>(s.end_tick) + kSpanEndTicks);
    return {s_of(s.start_ms), s_of(end_ms)};
}

// The overlay's span edges in the order build_track_state lists them, sorted
// by time with that order kept among equal times. NaN edges have no place in
// time order and are left out; no real scene produces one.
std::vector<double> sorted_edges(std::initializer_list<const std::vector<Iv>*> lists) {
    std::vector<double> edges;
    for (const std::vector<Iv>* ivs : lists)
        for (const Iv& iv : *ivs) {
            if (!std::isnan(iv.first)) edges.push_back(iv.first);
            if (!std::isnan(iv.second)) edges.push_back(iv.second);
        }
    std::stable_sort(edges.begin(), edges.end());
    return edges;
}

}  // namespace

void TrackState::set_overlay_intervals(const PreviewScene& scene) {
    fill_.clear();
    fill_taken_.clear();
    sp_active_.clear();
    fill_lane_.clear();
    fill_lane_ivs_.clear();
    // A hidden fill is one the game never showed, so it draws nothing.
    for (const app::PreviewFill& f : scene.fills) {
        if (f.state == app::PreviewFillState::Offered)
            fill_.push_back(span_interval(scene, f.span));
        else if (f.state == app::PreviewFillState::Taken)
            fill_taken_.push_back(span_interval(scene, f.span));
    }
    for (const PreviewActivation& a : scene.activations) {
        if (a.has_sp_end && a.sp_end_ms > a.ms)
            sp_active_.push_back({s_of(a.ms), s_of(a.sp_end_ms)});
        if (a.has_lane) {
            std::optional<Pad> pad = pad_of(a.lane);
            if (!pad) continue;
            for (const app::PreviewFill& f : scene.fills) {
                if (f.state != app::PreviewFillState::Taken || f.span.end_tick != a.tick) continue;
                fill_lane_.push_back({span_interval(scene, f.span), *pad});
                break;
            }
        }
    }
    fill_lane_ivs_.reserve(fill_lane_.size());
    for (const LaneInterval& li : fill_lane_) fill_lane_ivs_.push_back(li.span);
}

void TrackState::sweep_span_fields() {
    SpanSweep overdrive(overdrive_), solo(solo_), fill(fill_), fill_taken(fill_taken_),
        sp_active(sp_active_), fill_lane(fill_lane_ivs_);
    LaneSweep lane_pad(fill_lane_ivs_);
    for (TrackInstant& inst : instants_) {
        const double t = inst.t;
        inst.overdrive = overdrive.at(t);
        inst.solo = solo.at(t);
        inst.fill = fill.at(t);
        inst.fill_taken = fill_taken.at(t);
        inst.sp_active = sp_active.at(t);
        inst.fill_lane = fill_lane.at(t);
        inst.fill_lane_pad.reset();
        if (inst.fill_lane != Toggle::Empty)
            if (std::optional<size_t> w = lane_pad.at(t)) inst.fill_lane_pad = fill_lane_[*w].pad;
    }
}

Toggle toggle_at(const std::vector<std::pair<double, double>>& intervals, double t) {
    bool starts = false, ends = false, inside = false;
    for (const auto& iv : intervals) {
        if (iv.first == t) starts = true;
        if (iv.second == t) ends = true;
        if (iv.first < t && t < iv.second) inside = true;
    }
    // An edge that falls inside another interval of the same span is not an
    // edge of the merged span: the span simply goes on.
    if (inside) return Toggle::On;
    if (starts && ends) return Toggle::Restart;
    if (starts) return Toggle::Start;
    if (ends) return Toggle::End;
    return Toggle::Empty;
}

TrackState build_track_state(const PreviewScene& scene, const TrackStateOptions& opts) {
    TrackState st;
    // SP phrases and solos are path-free; the overlay's spans come from the
    // fill states and activations (set_overlay_intervals).
    for (const PreviewSpan& s : scene.sp_phrases) st.overdrive_.push_back(span_interval(scene, s));
    for (const PreviewSpan& s : scene.solos) st.solo_.push_back(span_interval(scene, s));
    st.set_overlay_intervals(scene);

    // Every moment anything happens becomes an instant. Gather the moments in
    // one list (notes, then beats, then span edges, each in scene order), sort
    // it by time keeping that order among equal times, and group equal times:
    // a moment's gems keep scene order and its last beat wins.
    std::vector<Event> events;
    size_t n_edges = 2 * (st.overdrive_.size() + st.solo_.size() + st.fill_.size() +
                          st.fill_taken_.size() + st.sp_active_.size() + st.fill_lane_ivs_.size());
    events.reserve(scene.notes.size() + scene.beats.size() + n_edges);
    // A NaN time has no place in time order; no real scene produces one.
    auto push = [&events](double t, Event::Kind kind, size_t index) {
        if (!std::isnan(t)) events.push_back({t, kind, index});
    };
    for (size_t i = 0; i < scene.notes.size(); ++i)
        push(s_of(scene.notes[i].ms), Event::Kind::Note, i);
    for (size_t i = 0; i < scene.beats.size(); ++i)
        push(s_of(scene.beats[i].ms), Event::Kind::Beat, i);
    for (const std::vector<TrackState::Interval>* ivs : {&st.overdrive_, &st.solo_})
        for (const auto& iv : *ivs) {
            push(iv.first, Event::Kind::Edge, 0);
            push(iv.second, Event::Kind::Edge, 0);
        }
    for (const std::vector<TrackState::Interval>* ivs :
         {&st.fill_, &st.fill_taken_, &st.sp_active_, &st.fill_lane_ivs_})
        for (const auto& iv : *ivs) {
            push(iv.first, Event::Kind::OverlayEdge, 0);
            push(iv.second, Event::Kind::OverlayEdge, 0);
        }
    std::stable_sort(events.begin(), events.end(),
                     [](const Event& a, const Event& b) { return a.t < b.t; });

    size_t groups = 0;
    for (size_t i = 0; i < events.size(); ++i)
        if (i == 0 || events[i - 1].t < events[i].t) ++groups;
    st.instants_.reserve(groups);
    st.base_t_.reserve(groups);

    for (size_t lo = 0; lo < events.size();) {
        size_t hi = lo + 1;
        while (hi < events.size() && !(events[lo].t < events[hi].t)) ++hi;

        TrackInstant inst;
        // The time of the group's last moment, as the old per-moment map kept
        // it (only 0.0 and -0.0 can differ, and they compare equal anyway).
        inst.t = events[hi - 1].t;
        size_t n_notes = 0;
        for (size_t k = lo; k < hi; ++k)
            if (events[k].kind == Event::Kind::Note) ++n_notes;
        inst.notes.reserve(n_notes);
        // The time this instant would have without the overlay: its last
        // path-free moment's (edges sort after notes and beats, and overlay
        // edges after the others, so that is the last non-overlay event).
        double base_t = std::numeric_limits<double>::quiet_NaN();
        for (size_t k = lo; k < hi; ++k) {
            const Event& e = events[k];
            if (e.kind != Event::Kind::OverlayEdge) base_t = e.t;
            if (e.kind == Event::Kind::Note)
                inst.notes.push_back(gem_of(scene.notes[e.index], opts.pro));
            else if (e.kind == Event::Kind::Beat)
                inst.beat = scene.beats[e.index].kind;
        }
        st.instants_.push_back(std::move(inst));
        st.base_t_.push_back(base_t);
        lo = hi;
    }
    st.sweep_span_fields();
    return st;
}

void rebuild_overlay_fields(TrackState& st, const PreviewScene& scene) {
    st.set_overlay_intervals(scene);
    // The new overlay's edges, in the order and with the times
    // build_track_state would push them.
    const std::vector<double> edges =
        sorted_edges({&st.fill_, &st.fill_taken_, &st.sp_active_, &st.fill_lane_ivs_});

    // Merge the kept (path-free) instants with the new edges, grouping equal
    // times as build_track_state does. A group's time is its last moment's:
    // the last overlay edge when there is one (they sort last), else the
    // path-free time. Instants only the old overlay made are dropped here.
    std::vector<TrackInstant> old = std::move(st.instants_);
    std::vector<double> old_base = std::move(st.base_t_);
    st.instants_.clear();
    st.base_t_.clear();
    st.instants_.reserve(old.size() + edges.size());
    st.base_t_.reserve(old.size() + edges.size());
    size_t i = 0, j = 0;
    for (;;) {
        while (i < old.size() && std::isnan(old_base[i])) ++i;
        if (i == old.size() && j == edges.size()) break;
        if (i < old.size() && (j == edges.size() || !(edges[j] < old_base[i]))) {
            const double bt = old_base[i];
            TrackInstant inst = std::move(old[i++]);
            inst.t = bt;
            while (j < edges.size() && !(bt < edges[j])) inst.t = edges[j++];
            st.instants_.push_back(std::move(inst));
            st.base_t_.push_back(bt);
        } else {
            const double first = edges[j];
            TrackInstant inst;
            while (j < edges.size() && !(first < edges[j])) inst.t = edges[j++];
            st.instants_.push_back(std::move(inst));
            st.base_t_.push_back(std::numeric_limits<double>::quiet_NaN());
        }
    }
    st.sweep_span_fields();
}

TrackInstant TrackState::synthesize(double t) const {
    TrackInstant inst;
    inst.t = t;
    inst.overdrive = toggle_at(overdrive_, t);
    inst.solo = toggle_at(solo_, t);
    inst.fill = toggle_at(fill_, t);
    inst.fill_taken = toggle_at(fill_taken_, t);
    inst.sp_active = toggle_at(sp_active_, t);
    inst.fill_lane = toggle_at(fill_lane_ivs_, t);
    if (inst.fill_lane != Toggle::Empty) {
        // The lane of the interval starting at, ending at, or containing t
        // (a start wins over an end when two fills touch).
        for (const LaneInterval& li : fill_lane_) {
            if (li.span.first == t || (li.span.first < t && t < li.span.second)) {
                inst.fill_lane_pad = li.pad;
                break;
            }
        }
        if (!inst.fill_lane_pad)
            for (const LaneInterval& li : fill_lane_)
                if (li.span.second == t) {
                    inst.fill_lane_pad = li.pad;
                    break;
                }
    }
    return inst;
}

TrackWindow TrackState::window(double near_s, double far_s) const {
    auto lo = std::upper_bound(instants_.begin(), instants_.end(), near_s,
                               [](double t, const TrackInstant& i) { return t < i.t; });
    auto hi = lo;
    while (hi != instants_.end() && hi->t < far_s) ++hi;
    // Onyx writes the synthesized key as `t1 + (t2 + t1) / 2`, which lands
    // past t2; it never reads that key, only the state (its neighbours'
    // before/after). We place it at the midpoint so the state is evaluated
    // strictly inside the window, which is the same state Onyx carries.
    if (lo == hi) return TrackWindow(synthesize((near_s + far_s) / 2.0));
    const TrackInstant* base = instants_.data();
    return TrackWindow(base + (lo - instants_.begin()), base + (hi - instants_.begin()));
}

std::vector<ToggleSpan> TrackState::make_toggle_bounds(const TrackWindow& win,
                                                       double near_s, double far_s,
                                                       Toggle TrackInstant::*field) const {
    std::vector<ToggleSpan> spans;
    if (win.empty()) return spans;
    // State entering the window: on if the first instant ends or continues a
    // span (End / Restart / On), off otherwise.
    Toggle first = win.front().*field;
    bool on = first == Toggle::End || first == Toggle::Restart || first == Toggle::On;
    double t = near_s;
    auto push = [&](double t2, bool state) {
        if (t2 <= t) return;
        if (!spans.empty() && spans.back().on == state)
            spans.back().t2 = t2;
        else
            spans.push_back({t, t2, state});
        t = t2;
    };
    for (const TrackInstant& inst : win) {
        push(inst.t, on);
        Toggle tg = inst.*field;
        on = tg == Toggle::Start || tg == Toggle::Restart || tg == Toggle::On;
    }
    push(far_s, on);
    return spans;
}

}  // namespace hydra::render
