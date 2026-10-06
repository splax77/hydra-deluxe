#include "render/track_state.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iterator>
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
    // A lane with no pad (pad_of) is the kick.
    if (const std::optional<Pad> pad = pad_of(n.lane)) {
        g.pad = *pad;
        g.cymbal = pro && n.cymbal && allows_cymbals(app::color_of(n.lane));
    } else {
        g.kick = true;  // 2x kicks are plain kicks (Onyx has no 2x visual)
    }
    g.velocity = n.ghost ? Velocity::Ghost : n.accent ? Velocity::Accent : Velocity::Normal;
    return g;
}

}  // namespace

// The sweep keeps sorted starts and ends and walks them forward. At time t,
// "does some interval start (end) at t" is a lookup in the sorted starts
// (ends). For "is t strictly inside some interval", count over the
// well-formed intervals (start <= end): the number inside is
//   (starts < t) - (ends <= t) + (zero-length intervals sitting at t),
// because an interval that ends at or before t also started at or before t,
// and it started exactly at t only when it is a zero-length one at t.
// Malformed intervals (end < start) can never hold t inside, so they stay out
// of that count; their edges still count as starts and ends.
SpanSweep::SpanSweep(const std::vector<Iv>& ivs) {
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

Toggle SpanSweep::at(double t) {
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

namespace {

// The activated fill's lane at each instant: the first interval (in list
// order) that starts at or contains t wins; failing that, the first one that
// ends at t. Ask it for times in increasing order. Well-formed intervals join
// a small active set when they start and leave it once they have ended (fills
// rarely overlap, so it stays tiny); malformed ones (end < start, or NaN) are
// checked every time, because their edges sit out of order.
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

// One path-free moment: a note, a beat, or an SP phrase or solo edge.
struct Event {
    enum class Kind : uint8_t { Note, Beat, Edge };
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

// The overlay's span edges in list order, sorted by time with that order kept
// among equal times. NaN edges have no place in time order and are left out;
// no real scene produces one.
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

// Onyx's makeToggleBounds walk over one span field: the consecutive
// stretches [t1, t2) covering [near, far], each with whether the span is on
// through it and the instant that opened it (null for the stretch entering
// the window). Stretches of no length are skipped.
template <class Emit>
void walk_window(const TrackWindow& win, double near_s, double far_s,
                 Toggle TrackInstant::*field, Emit emit) {
    if (win.empty()) return;
    // State entering the window: on if the first instant ends or continues a
    // span (End / Restart / On), off otherwise.
    Toggle first = win.front().*field;
    bool on = first == Toggle::End || first == Toggle::Restart || first == Toggle::On;
    const TrackInstant* opened = nullptr;
    double t = near_s;
    auto push = [&](double t2) {
        if (t2 <= t) return;
        emit(t, t2, on, opened);
        t = t2;
    };
    for (const TrackInstant& inst : win) {
        push(inst.t);
        on = toggle_on_after(inst.*field);
        opened = &inst;
    }
    push(far_s);
}

}  // namespace

// Every span field of an instant from the state's intervals: one SpanSweep
// per span field and one LaneSweep for the lit lane's pad. Ask it for
// instants in increasing time. sweep_span_fields runs it over every instant;
// synthesize asks it once.
class TrackState::FieldSweep {
public:
    explicit FieldSweep(const TrackState& st)
        : lanes_(st.fill_lane_), overdrive_(st.overdrive_), solo_(st.solo_), fill_(st.fill_),
          fill_taken_(st.fill_taken_), sp_active_(st.sp_active_), fill_lane_(st.fill_lane_ivs_),
          lane_pad_(st.fill_lane_ivs_) {}

    void set(TrackInstant& inst) {
        const double t = inst.t;
        inst.overdrive = overdrive_.at(t);
        inst.solo = solo_.at(t);
        inst.fill = fill_.at(t);
        inst.fill_taken = fill_taken_.at(t);
        inst.sp_active = sp_active_.at(t);
        inst.fill_lane = fill_lane_.at(t);
        inst.fill_lane_pad.reset();
        if (inst.fill_lane != Toggle::Empty)
            if (std::optional<size_t> w = lane_pad_.at(t)) inst.fill_lane_pad = lanes_[*w].pad;
    }

private:
    const std::vector<LaneInterval>& lanes_;
    SpanSweep overdrive_, solo_, fill_, fill_taken_, sp_active_, fill_lane_;
    LaneSweep lane_pad_;
};

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
        // The tinted floor: the activation's sp_window, its owner.
        if (const std::optional<std::pair<double, double>> w = a.sp_window())
            sp_active_.push_back({s_of(w->first), s_of(w->second)});
        // The taken fill lights the activation note's lane. The scene
        // builder names that fill (taken_fill).
        if (a.has_lane && a.taken_fill) {
            std::optional<Pad> pad = pad_of(a.lane);
            if (!pad) continue;
            fill_lane_.push_back({span_interval(scene, scene.fills[*a.taken_fill].span), *pad});
        }
    }
    fill_lane_ivs_.reserve(fill_lane_.size());
    for (const LaneInterval& li : fill_lane_) fill_lane_ivs_.push_back(li.span);
}

void TrackState::sweep_span_fields() {
    FieldSweep sweep(*this);
    for (TrackInstant& inst : instants_) sweep.set(inst);
}

TrackState build_track_state(const PreviewScene& scene, const TrackStateOptions& opts) {
    TrackState st;
    // SP phrases and solos are path-free; the overlay's spans come from the
    // fill states and activations (set_overlay_intervals).
    for (const PreviewSpan& s : scene.sp_phrases) st.overdrive_.push_back(span_interval(scene, s));
    for (const PreviewSpan& s : scene.solos) st.solo_.push_back(span_interval(scene, s));

    // Every path-free moment becomes an instant. Gather the moments in one
    // list (notes, then beats, then span edges, each in scene order), sort it
    // by time keeping that order among equal times, and group equal times: a
    // moment's gems keep scene order and its last beat wins.
    std::vector<Event> events;
    events.reserve(scene.notes.size() + scene.beats.size() +
                   2 * (st.overdrive_.size() + st.solo_.size()));
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
        for (size_t k = lo; k < hi; ++k) {
            const Event& e = events[k];
            if (e.kind == Event::Kind::Note)
                inst.notes.push_back(gem_of(scene.notes[e.index], opts.pro));
            else if (e.kind == Event::Kind::Beat)
                inst.beat = scene.beats[e.index].kind;
        }
        st.base_t_.push_back(inst.t);
        st.instants_.push_back(std::move(inst));
        lo = hi;
    }

    // Then the overlay's edges, by the same merge rebuild_overlay_fields uses.
    st.set_overlay_intervals(scene);
    st.merge_overlay_edges();
    return st;
}

void TrackState::merge_overlay_edges() {
    // The overlay's edges, sorted by time with list order kept among equal
    // times.
    const std::vector<double> edges =
        sorted_edges({&fill_, &fill_taken_, &sp_active_, &fill_lane_ivs_});

    // Merge the kept (path-free) instants with the edges, grouping equal
    // times. A group's time is its last moment's: the last overlay edge when
    // there is one (they sort after the path-free moments), else the
    // path-free time. Instants only an old overlay made are dropped here.
    std::vector<TrackInstant> old = std::move(instants_);
    std::vector<double> old_base = std::move(base_t_);
    instants_.clear();
    base_t_.clear();
    instants_.reserve(old.size() + edges.size());
    base_t_.reserve(old.size() + edges.size());
    size_t i = 0, j = 0;
    for (;;) {
        while (i < old.size() && std::isnan(old_base[i])) ++i;
        if (i == old.size() && j == edges.size()) break;
        if (i < old.size() && (j == edges.size() || !(edges[j] < old_base[i]))) {
            const double bt = old_base[i];
            TrackInstant inst = std::move(old[i++]);
            inst.t = bt;
            while (j < edges.size() && !(bt < edges[j])) inst.t = edges[j++];
            instants_.push_back(std::move(inst));
            base_t_.push_back(bt);
        } else {
            const double first = edges[j];
            TrackInstant inst;
            while (j < edges.size() && !(first < edges[j])) inst.t = edges[j++];
            instants_.push_back(std::move(inst));
            base_t_.push_back(std::numeric_limits<double>::quiet_NaN());
        }
    }
    sweep_span_fields();
}

void rebuild_overlay_fields(TrackState& st, const PreviewScene& scene) {
    st.set_overlay_intervals(scene);
    st.merge_overlay_edges();
}

bool note_in_span(const PreviewScene& scene, const PreviewSpan& span, const PreviewNote& note) {
    SpanSweep sweep(std::vector<Iv>{span_interval(scene, span)});
    return toggle_on_after(sweep.at(s_of(note.ms)));
}

TrackInstant TrackState::synthesize(double t) const {
    TrackInstant inst;
    inst.t = t;
    FieldSweep(*this).set(inst);
    return inst;
}

std::vector<TrackInstant>::const_iterator TrackState::first_after(double near_s) const {
    return std::upper_bound(instants_.begin(), instants_.end(), near_s,
                            [](double t, const TrackInstant& i) { return t < i.t; });
}

TrackWindow TrackState::window(double near_s, double far_s) const {
    auto lo = first_after(near_s);
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
    walk_window(win, near_s, far_s, field,
                [&](double t1, double t2, bool on, const TrackInstant*) {
                    if (!spans.empty() && spans.back().on == on)
                        spans.back().t2 = t2;
                    else
                        spans.push_back({t1, t2, on});
                });
    return spans;
}

std::vector<double> TrackState::sp_active_ends(const TrackWindow& win) const {
    std::vector<double> ends;
    for (const TrackInstant& inst : win)
        if (inst.sp_active == Toggle::End || inst.sp_active == Toggle::Restart)
            ends.push_back(inst.t);
    return ends;
}

std::vector<LaneSpan> TrackState::make_lane_bounds(const TrackWindow& win, double near_s,
                                                   double far_s) const {
    std::vector<LaneSpan> spans;
    if (win.empty()) return spans;
    // The pad in force entering the window is the one the last instant at or
    // before near carries (an instant's pad holds until the next instant). The
    // first instant inside cannot say: at a Restart it carries the pad of the
    // fill that starts there, not the one that ends. With no instant before
    // the window, fall back to the first one inside.
    const auto before = first_after(near_s);
    std::optional<Pad> entering;
    if (before != instants_.begin()) entering = std::prev(before)->fill_lane_pad;
    if (!entering) entering = win.front().fill_lane_pad;
    walk_window(win, near_s, far_s, &TrackInstant::fill_lane,
                [&](double t1, double t2, bool on, const TrackInstant* opened) {
                    const std::optional<Pad> pad = opened ? opened->fill_lane_pad : entering;
                    if (!on || !pad) return;
                    if (!spans.empty() && spans.back().t2 == t1 && spans.back().pad == *pad)
                        spans.back().t2 = t2;
                    else
                        spans.push_back({t1, t2, *pad});
                });
    return spans;
}

}  // namespace hydra::render
