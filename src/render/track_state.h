// Track state — the chart as a timeline of instants, the way Onyx's previewer
// holds it (its `Map Double (CommonState DrumState)`): one entry per moment
// anything happens, carrying the gems struck then and, for every span-like
// thing (SP phrase, solo, fill, Hydra's active SP window and activation
// lane), whether that span starts, ends, restarts, is ongoing, or is absent at
// that moment. The draw code walks the slice of instants inside the visible
// time window and never touches the PreviewScene directly.
//
// Times are seconds (Onyx's unit). Device-free and unit-tested.

#ifndef HYDRA_RENDER_TRACK_STATE_H
#define HYDRA_RENDER_TRACK_STATE_H

#include <optional>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

#include "app/preview_view.h"

namespace hydra::render {

// Onyx's SustainState for a span with no payload: what a span does at an
// instant. Between instants a span is On (ongoing) or Empty (absent).
enum class Toggle { Empty, Start, End, Restart, On };

// Is the span on just after an instant where it does `t`: it starts, restarts
// or goes on there. (End and Empty leave it off.)
inline bool toggle_on_after(Toggle t) {
    return t == Toggle::Start || t == Toggle::Restart || t == Toggle::On;
}

// Hydra marks an SP phrase, a solo or a fill by the tick of its last note;
// Onyx's span reaches past that note. The drawn edge sits this many ticks
// after the last note: past it, and short of any note on the next tick, at
// every resolution and tempo. (A fixed half millisecond was not: at 480 ticks
// per beat and 300 BPM one tick is 0.417 ms.)
constexpr double kSpanEndTicks = 0.5;

// The four pads, left to right. Kick is separate (TrackGem::kick).
enum class Pad { Red = 0, Yellow = 1, Blue = 2, Green = 3 };

enum class Velocity { Ghost, Normal, Accent };

struct TrackGem {
    bool kick = false;
    Pad pad = Pad::Red;    // meaningless when kick
    bool cymbal = false;   // pro-drums cymbal (never for Red or kick)
    Velocity velocity = Velocity::Normal;
};

struct TrackInstant {
    double t = 0.0;  // seconds
    std::vector<TrackGem> notes;
    Toggle overdrive = Toggle::Empty;  // SP phrase (Onyx: overdrive)
    Toggle solo = Toggle::Empty;
    Toggle fill = Toggle::Empty;        // an offered fill (drawn like Onyx's BRE)
    Toggle fill_taken = Toggle::Empty;  // Hydra: a fill the path activates on
    Toggle sp_active = Toggle::Empty;   // Hydra: the path's active SP window
    Toggle fill_lane = Toggle::Empty;   // Hydra: the activated fill's lit lane
    std::optional<Pad> fill_lane_pad;  // which lane, while fill_lane != Empty
    std::optional<app::PreviewBeatKind> beat;
};

struct TrackStateOptions {
    bool pro = true;  // false: every pad draws as a tom (Onyx's 4-lane mode)

    friend bool operator==(const TrackStateOptions& a, const TrackStateOptions& b) {
        return a.pro == b.pro;
    }
    friend bool operator!=(const TrackStateOptions& a, const TrackStateOptions& b) {
        return !(a == b);
    }
};

// A [t1, t2) stretch of the visible window with one span state, from
// make_toggle_bounds.
struct ToggleSpan {
    double t1 = 0.0, t2 = 0.0;
    bool on = false;
};

// A [t1, t2) stretch of the visible window where the activated fill's lane is
// lit, and which pad it lights, from make_lane_bounds.
struct LaneSpan {
    double t1 = 0.0, t2 = 0.0;
    Pad pad = Pad::Red;
};

// Onyx's makeToggle for one span field: the state of the span at time t,
// given its [start, end] intervals. One sweep answers it for many times in one
// pass; ask it for times in increasing order. At t it needs three facts: does
// some interval start at t, does some end at t, and is t strictly inside some
// interval. Inside wins (an edge inside another interval of the same span is
// not an edge of the merged span: the span simply goes on), then start and
// end together are a Restart, then Start, then End, else Empty.
class SpanSweep {
public:
    explicit SpanSweep(const std::vector<std::pair<double, double>>& intervals);
    Toggle at(double t);

private:
    std::vector<double> starts_, ends_, zeros_;  // well-formed intervals only
    std::vector<double> all_starts_, all_ends_;  // every interval's edges
    size_t i_s_ = 0, i_e_ = 0, i_z_ = 0, i_as_ = 0, i_ae_ = 0;
};

// The instants one frame draws, near < t < far, as a view into the track
// state: nothing is copied. When no instant falls inside, it holds one
// synthesized instant at the window's midpoint instead (Onyx's zoomMap
// fallback). Valid while the TrackState it came from is alive and unchanged.
class TrackWindow {
public:
    TrackWindow(const TrackInstant* first, const TrackInstant* last)
        : first_(first), last_(last) {}
    explicit TrackWindow(TrackInstant synthesized) : synth_(std::move(synthesized)) {}

    const TrackInstant* begin() const { return synth_ ? &*synth_ : first_; }
    const TrackInstant* end() const { return synth_ ? &*synth_ + 1 : last_; }
    std::reverse_iterator<const TrackInstant*> rbegin() const {
        return std::reverse_iterator<const TrackInstant*>(end());
    }
    std::reverse_iterator<const TrackInstant*> rend() const {
        return std::reverse_iterator<const TrackInstant*>(begin());
    }
    size_t size() const { return static_cast<size_t>(end() - begin()); }
    bool empty() const { return size() == 0; }
    const TrackInstant& front() const { return *begin(); }
    const TrackInstant& operator[](size_t i) const { return begin()[i]; }

private:
    const TrackInstant* first_ = nullptr;
    const TrackInstant* last_ = nullptr;
    std::optional<TrackInstant> synth_;
};

class TrackState {
public:
    TrackState() = default;

    const std::vector<TrackInstant>& instants() const { return instants_; }

    // Onyx's zoomMap: the instants with near < t < far. When none fall inside,
    // one synthesized instant at the window's midpoint carries the
    // ongoing/absent state of every span so the floor and lanes still draw.
    TrackWindow window(double near_s, double far_s) const;

    // Onyx's makeToggleBounds over one span field: consecutive spans covering
    // [near, far] with that field on or off, adjacent equal states merged.
    std::vector<ToggleSpan> make_toggle_bounds(const TrackWindow& win,
                                               double near_s, double far_s,
                                               Toggle TrackInstant::*field) const;

    // The stretches of [near, far] where the activated fill's lane is lit,
    // each with the pad it lights. They cover the same time as
    // make_toggle_bounds's lit fill_lane stretches, but a change of pad at an
    // instant ends one stretch and starts the next, so two taken fills that
    // touch each light their own lane. A stretch entering the window takes
    // the pad in force before it: the pad of the last instant at or before
    // near.
    std::vector<LaneSpan> make_lane_bounds(const TrackWindow& win, double near_s,
                                           double far_s) const;

    // The times inside `win` where active SP runs out: every instant whose
    // sp_active field ends there (End, or Restart where one window ends as
    // the next starts, so back-to-back windows give one end each). In time
    // order. The window decides what is visible; the edges come from the
    // activations' sp_window, through the sp_active intervals.
    std::vector<double> sp_active_ends(const TrackWindow& win) const;

private:
    friend TrackState build_track_state(const app::PreviewScene&, const TrackStateOptions&);
    friend void rebuild_overlay_fields(TrackState&, const app::PreviewScene&);
    class FieldSweep;

    using Interval = std::pair<double, double>;  // [start, end] seconds
    struct LaneInterval {
        Interval span;
        Pad pad;
    };

    std::vector<TrackInstant> instants_;
    // One entry per instant: the time the instant would have from the scene's
    // path-free moments alone (its notes, its beat, SP phrase and solo
    // edges), or NaN when only overlay edges put it there. This is what lets
    // merge_overlay_edges drop an old overlay's instants and restore the
    // time of the ones it keeps.
    std::vector<double> base_t_;
    std::vector<Interval> overdrive_, solo_, fill_, fill_taken_, sp_active_;
    std::vector<LaneInterval> fill_lane_;
    std::vector<Interval> fill_lane_ivs_;  // fill_lane_'s spans alone, built once

    // The overlay's intervals (fill, fill_taken, sp_active, fill_lane), cleared
    // and rebuilt from the scene's fill states and activations.
    void set_overlay_intervals(const app::PreviewScene& scene);
    // Merge the overlay's edges into the path-free instants (dropping any
    // instant only an old overlay made), then sweep every span field. The
    // one merge both build_track_state and rebuild_overlay_fields use.
    void merge_overlay_edges();
    // Every instant's span fields from the intervals, in one sweep.
    void sweep_span_fields();

    // The state of every span at t, by the same sweep sweep_span_fields runs,
    // asked once. window() uses it for the empty-window fallback.
    TrackInstant synthesize(double t) const;

    // The first instant with t > near_s (Onyx's zoomMap's lower bound).
    std::vector<TrackInstant>::const_iterator first_after(double near_s) const;
};

// Build the timeline from a scene. SP phrases, solos and fills end
// kSpanEndTicks past their last note (through scene.timing), so that note reads as inside
// and a note on the next tick reads as outside (Hydra marks a phrase by its
// last note; Onyx's phrase extends past it). A scene with spans must carry
// its timing; build_preview_scene always sets it, and a scene with spans but
// no timing throws std::invalid_argument. Active SP
// windows end exactly at the deact node. Offered fills drive `fill` and taken
// ones `fill_taken`; hidden fills produce no intervals at all, since the game
// would never have shown them. The activated fill's lane comes from the
// activation at the fill's end note.
TrackState build_track_state(const app::PreviewScene& scene, const TrackStateOptions& opts);

// Swap a timeline's path overlay for `scene`'s, keeping everything else. The
// overlay is the fill, fill_taken, sp_active and fill_lane spans (they read the
// fill states and activations, see app::apply_preview_overlay). `state` must
// have been built by build_track_state from a scene with the same path-free
// half as `scene` (same notes, beats, SP phrases, solos, fill windows and
// timing), under the options `state` should keep. The result equals
// build_track_state(scene, those options): instants only the old overlay put
// there are dropped, the new overlay's edges become instants, and every
// instant's span fields are swept again. Notes and beats are moved, never
// rebuilt.
void rebuild_overlay_fields(TrackState& state, const app::PreviewScene& scene);

// Is `note` inside `span` by the track state's own rule: the span's edges as
// build_track_state places them (its end kSpanEndTicks past its last note,
// through scene.timing), and on at the note's time the way the highway reads
// it (toggle_on_after of SpanSweep's state there). So a phrase's last note is
// inside and a note on the next tick is outside. Throws
// std::invalid_argument when the scene has no timing, like build_track_state.
bool note_in_span(const app::PreviewScene& scene, const app::PreviewSpan& span,
                  const app::PreviewNote& note);

}  // namespace hydra::render

#endif  // HYDRA_RENDER_TRACK_STATE_H
