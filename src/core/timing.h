// Timing math: converting between ticks, milliseconds and measures.
//
// A song measures time in ticks; the app also needs milliseconds (from the
// tempo map) and measure positions. Clone Hero counts measures two ways, so
// Hydra has two measure owners, both built from the chart's time signatures:
// MeasureIndex is the grid (where the measure and beat lines are, and the
// measure/beat/tick a screen shows) and MeasureCount is the continuous count
// (how many measures lie between two ticks, which Star Power's length reads).
// Timecode carries all of these for one tick. MsIndex, MeasureIndex and
// MeasureCount are built once per song into SongTiming and reused.
//
// The arithmetic keeps a fixed operation order and integer floor semantics:
// stored records and pathstrings depend on these ms/measure values
// bit-for-bit, so any change here silently invalidates stored results.

#ifndef HYDRA_CORE_TIMING_H
#define HYDRA_CORE_TIMING_H

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

namespace hydra {

// combo -> score multiplier: x1 below 10, x2 below 20, x3 below 30, else x4.
int to_multiplier(int combo);

// The combo multiplier at each end of a chord of `note_count` notes hit at
// `combo`: `first` pays its first note, `last` its last. A chord with no notes
// pays nothing, so both ends stay at the combo's own multiplier.
struct MultiplierStep {
    int first;
    int last;
};
MultiplierStep multiplier_step(int combo, int note_count);

// Does a chord of `note_count` notes hit at `combo` straddle a multiplier
// step, so its first and last notes pay at different multipliers?
bool crosses_multiplier_step(int combo, int note_count);

// Star Power pays a chord's value once more (CategoryScores::sp), so the game
// shows the combo multiplier doubled while it runs.
inline constexpr int kStarPowerMultiplier = 2;

// Clone Hero's rule: an activation needs two banked bars of Star Power.
inline constexpr int kSpActivationBars = 2;

// The multiplier the game's disc shows for a chord: its combo multiplier,
// times kStarPowerMultiplier when Star Power pays the chord.
int shown_multiplier(int combo_multiplier, bool in_sp);

// The one "which entry is in force at this key" search: the last element of
// the sorted range [first, last) whose key_of is at or before `key`, or
// `last` when none is. A key exactly on an entry reads that entry. MsIndex,
// MeasureIndex, MeasureCount and the Preview's section lookup all call it;
// sections live outside SongTiming because the .mid reader fills them after
// build_timing.
template <class It, class Key, class KeyOf>
It last_at_or_before(It first, It last, const Key& key, KeyOf key_of) {
    const It it = std::upper_bound(
        first, last, key, [&](const Key& k, const auto& e) { return k < key_of(e); });
    return it == first ? last : std::prev(it);
}

// Where each tempo section starts and the time elapsed by then.
// Built from a bpm map (tick -> BPM) and the tick resolution.
class MsIndex {
public:
    MsIndex(const std::map<int64_t, double>& bpm_map, int64_t tick_r);

    // Milliseconds at an absolute tick (defined for negative ticks too: they
    // read back at the opening tempo, as the original walk did).
    double at(int64_t ticks) const;

    // Ticks-per-second of the tempo section containing `ticks` (a tick exactly
    // on a tempo change reads the new tempo, same rule as at()).
    double tps_at(int64_t ticks) const;

    // The chart's BPM in force at `ticks`, read the same way as tps_at.
    double bpm_at(int64_t ticks) const;

    // Continuous (sub-tick) variants of at(): piecewise-linear, monotone,
    // mutually inverse, and agreeing with at() on integer ticks. NOT part of
    // the bit-for-bit scoring surface (see the header comment) -- never use
    // them in the graph or the engine's scoring path. tick_at_ms serves
    // SongTiming::display_tick_at_ms, the tick a screen shows at a time,
    // which the Preview's time box, drain box, tick steps and beat-line end
    // and the Paths timeline's end (app/path_view.cpp) call. ms_at_tick_f
    // serves the end of a shaded span (render/track_state.cpp) and the Clone
    // Hero 1.0 fill deadline (search/graph.cpp), which is off the scoring
    // surface on purpose. SongTiming::sp_end_ms (which only tests call) uses
    // both.
    double ms_at_tick_f(double ticks) const;
    double tick_at_ms(double ms) const;

private:
    int span_at(int64_t ticks) const;

    std::vector<int64_t> keys_;
    std::vector<double> bpm_;
    std::vector<double> tps_;
    std::vector<double> elapsed_;
};

// Clone Hero's measure grid (its beat-line walk, 0x20DACA0): where the
// measure lines and beat lines are, and the measure/beat/tick a screen shows.
// Built from the chart's time signatures (tick -> numerator, denominator) and
// the resolution, as one span per meter in force. A span's beat step is
// Clone Hero's truncated quarter-note share; a time signature takes effect at
// the first beat of the span before it at or after its own tick, and one with
// the same numerator and denominator as the span in force changes nothing.
// The constructor owns those rules. A grid measure (numerator x step) can be
// a few ticks shorter than apply_timesig's ticks per measure only when the
// step is not whole (resolution 25 in 7/8, say); no real chart does that.
class MeasureIndex {
public:
    MeasureIndex(const std::map<int64_t, std::pair<int, int>>& sig_map, int64_t tick_r);

    // The span a tick sits in: the last one restarting at or before it, so a
    // tick exactly on a restart is in the new span.
    int span_at(int64_t ticks) const;

    int64_t restart_at(int i) const { return spans_[i].restart; }
    int64_t step_at(int i) const { return spans_[i].step; }
    int numerator_at(int i) const { return spans_[i].numerator; }
    int denominator_at(int i) const { return spans_[i].denominator; }
    int64_t length_at(int i) const { return spans_[i].length; }
    int64_t lines_before_at(int i) const { return spans_[i].lines_before; }
    int count() const { return static_cast<int>(spans_.size()); }

    // A tick's 0-based measure, beat and tick within the beat. Meaningful
    // for ticks >= 0.
    struct Position {
        int64_t measure = 0;
        int64_t beat = 0;
        int64_t tick = 0;
    };
    Position at(int64_t ticks) const;

    // What a grid line is, as Clone Hero's walk decides it.
    enum class Line { Measure, Beat, WeakBeat };

    // Calls fn(tick, line) for every grid line from tick 0 to last_tick
    // inclusive, in tick order.
    void walk(int64_t last_tick, const std::function<void(int64_t, Line)>& fn) const;

private:
    struct Span {
        int64_t restart = 0;
        int64_t step = 0;
        int numerator = 0;
        int denominator = 0;
        int64_t length = 0;
        int64_t lines_before = 0;
    };
    std::vector<Span> spans_;
};

// Clone Hero's continuous measure count (its Star Power drain): how many
// measures lie between two ticks. One span per time signature at the
// signature's own tick, measured with apply_timesig's ticks per measure, so a
// part measure before a mid-measure change counts as its share of the old
// measure. Built from a tpm map (tick -> ticks per measure).
class MeasureCount {
public:
    explicit MeasureCount(const std::map<int64_t, int64_t>& tpm_map);

    // The span a tick is counted in: a tick exactly on a change reads the new
    // span (the count is continuous, so only the slope differs).
    int span_at(int64_t ticks) const;

    int64_t start_at(int i) const { return starts_[i]; }
    int64_t tpm_at(int i) const { return tpm_[i]; }
    double elapsed_at(int i) const { return elapsed_[i]; }
    int count() const { return static_cast<int>(starts_.size()); }

    // Continuous measure position <-> tick, inverse of each other.
    double measures_at(double ticks) const;
    double tick_at(double measures) const;

    // The tick `add` whole measures after (or, negative, before) `ticks`,
    // walked span by span in exact fractions and floored to a whole tick
    // once, at the end.
    int64_t tick_after(int64_t ticks, int64_t add) const;

private:
    std::vector<int64_t> starts_;
    std::vector<int64_t> tpm_;
    std::vector<double> elapsed_;
};

// A point in time in a song, in multiple representations. Compares/hashes on
// ticks alone.
class Timecode {
public:
    Timecode() = default;
    // Build from prebuilt indexes (the common path — indexes are shared).
    Timecode(int64_t ticks, const MeasureIndex& grid, const MeasureCount& count,
             const MsIndex& ms);

    // Raw ticks only, mbt/ms left at their defaults: for a caller with no
    // SongTiming at hand, which needs only the ticks.
    static Timecode raw(int64_t ticks) {
        Timecode tc;
        tc.ticks_ = ticks;
        return tc;
    }

    int64_t ticks() const { return ticks_; }
    // {measure, beat, tick} on the grid (MeasureIndex::at). Meaningful for
    // ticks >= 0.
    const int64_t* measure_beats_ticks() const { return mbt_; }
    // Measures since tick 0 on the continuous count (MeasureCount).
    double measures_decimal() const { return measures_decimal_; }
    double ms() const { return ms_; }

    bool operator<(const Timecode& o) const { return ticks_ < o.ticks_; }
    bool operator<=(const Timecode& o) const { return ticks_ <= o.ticks_; }
    bool operator>(const Timecode& o) const { return ticks_ > o.ticks_; }
    bool operator>=(const Timecode& o) const { return ticks_ >= o.ticks_; }
    bool operator==(const Timecode& o) const { return ticks_ == o.ticks_; }
    bool operator!=(const Timecode& o) const { return ticks_ != o.ticks_; }

private:
    int64_t ticks_ = 0;
    int64_t mbt_[3] = {0, 0, 0};
    double measures_decimal_ = 0.0;
    double ms_ = 0.0;
};

// One bar of Star Power lasts two measures. Every "how long does this much SP
// run" call to plusmeasure goes through this, so the rule has one home.
inline constexpr int64_t kMeasuresPerSpBar = 2;
constexpr int64_t sp_bars_to_measures(int64_t bars) { return kMeasuresPerSpBar * bars; }

// A song's timing context: resolution plus the three indexes, built once and
// reused to make Timecodes. The maps must contain a tick-0 entry (as every
// real chart does), and the time signature map must have the tpm map's keys,
// or construction throws std::out_of_range — matching the KeyError(0) the
// Python indexes raise.
class SongTiming {
public:
    SongTiming(int64_t tick_r,
               const std::map<int64_t, int64_t>& tpm_map,
               const std::map<int64_t, double>& bpm_map,
               const std::map<int64_t, std::pair<int, int>>& sig_map);

    int64_t tick_resolution() const { return tick_r_; }
    const MeasureIndex& measure_index() const { return grid_; }
    const MeasureCount& measure_count() const { return count_; }
    const MsIndex& ms_index() const { return ms_; }

    Timecode timecode(int64_t ticks) const {
        return Timecode(ticks, grid_, count_, ms_);
    }

    // The Timecode `add_measures` measures after `tc` on the continuous
    // count (MeasureCount::tick_after). Star Power's length and the
    // generated-fill placement read it. Not cached here — the caller caches
    // if the call volume warrants it.
    Timecode plusmeasure(const Timecode& tc, int64_t add_measures) const;

    // The BPM and the time signature {numerator, denominator} in force at a
    // tick; a tick exactly on a change reads the new one.
    double tempo_at(int64_t ticks) const;
    std::pair<int, int> time_signature_at(int64_t ticks) const;

    // Local measure duration in ms for time just AFTER `ticks`: the
    // continuous count's ticks-per-measure over the tempo section's
    // ticks-per-second. A tick exactly on a meter or tempo change reads the
    // NEW section; pass `ticks - 1` for the duration just before the tick.
    // Display-layer helper (frontend->SP-end squeeze scaling); not part of
    // the scoring surface.
    double ms_per_measure_at(int64_t ticks) const;

    // Continuous measure position <-> tick on MeasureCount, and the exact
    // (sub-tick, piecewise-composed) SP end for an activation hit at
    // `act_hit_ms` holding SP for `end_measures` measures. All display-layer
    // only, like ms_per_measure_at: they interpolate in doubles with no
    // plusmeasure tick rounding, so they must never feed the
    // graph/engine/stored records.
    double measures_at_tick_f(double ticks) const;
    double tick_at_measures_f(double measures) const;
    double sp_end_ms(double act_hit_ms, int64_t end_measures) const;

    // The tick a playhead at `ms` shows: tick_at_ms rounded to the nearest
    // tick, and never below 0. This is the one rule for which tick the
    // screens show at a time (D48, Q23), so all four lines of the Preview's
    // time box switch on the same tick. Display-layer only, like the helpers
    // above.
    int64_t display_tick_at_ms(double ms) const;

private:
    int64_t tick_r_;
    MeasureIndex grid_;
    MeasureCount count_;
    MsIndex ms_;
};

}  // namespace hydra

#endif  // HYDRA_CORE_TIMING_H
