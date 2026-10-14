#include "core/timing.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string>

namespace hydra {

int to_multiplier(int combo) {
    if (combo < 10) return 1;
    if (combo < 20) return 2;
    if (combo < 30) return 3;
    return 4;
}

MultiplierStep multiplier_step(int combo, int note_count) {
    if (note_count <= 0) return {to_multiplier(combo), to_multiplier(combo)};
    return {to_multiplier(combo + 1), to_multiplier(combo + note_count)};
}

bool crosses_multiplier_step(int combo, int note_count) {
    const MultiplierStep step = multiplier_step(combo, note_count);
    return step.first < step.last;
}

int shown_multiplier(int combo_multiplier, bool in_sp) {
    return in_sp ? combo_multiplier * kStarPowerMultiplier : combo_multiplier;
}

namespace {

// The index of the entry in force at `key` (last_at_or_before), or 0 before
// the first entry: a key before the first one reads the opening entry.
template <class V, class Key, class KeyOf>
int index_at(const std::vector<V>& v, const Key& key, KeyOf key_of) {
    const auto it = last_at_or_before(v.begin(), v.end(), key, key_of);
    return it == v.end() ? 0 : static_cast<int>(it - v.begin());
}

constexpr auto kSelf = [](const auto& x) { return x; };

// Integer division rounded down and up, for a positive divisor.
int64_t floor_div(int64_t a, int64_t b) {
    const int64_t q = a / b;
    return (a % b != 0 && a < 0) ? q - 1 : q;
}
int64_t ceil_div(int64_t a, int64_t b) { return -floor_div(-a, b); }

}  // namespace

// ---- MsIndex ------------------------------------------------------------

MsIndex::MsIndex(const std::map<int64_t, double>& bpm_map, int64_t tick_r) {
    // A song's opening tempo is the mark at tick 0; without it the original
    // walk was a KeyError, so this is too.
    if (bpm_map.find(0) == bpm_map.end())
        throw std::out_of_range("bpm map has no tick-0 entry");

    // std::map already iterates in sorted key order.
    keys_.reserve(bpm_map.size());
    bpm_.reserve(bpm_map.size());
    tps_.reserve(bpm_map.size());
    for (const auto& kv : bpm_map) {
        keys_.push_back(kv.first);
        bpm_.push_back(kv.second);
        // tps = bpm * tick_r / 60, in this order: reordering the float operations
        // changes the last bit, and stored results were made with this order.
        tps_.push_back(kv.second * static_cast<double>(tick_r) / 60.0);
    }

    // Accumulated section by section, with the same arithmetic as the walk it
    // replaces so the two agree bit-for-bit.
    elapsed_.assign(keys_.size(), 0.0);
    for (size_t i = 1; i < keys_.size(); ++i) {
        elapsed_[i] = elapsed_[i - 1] +
            static_cast<double>(keys_[i] - keys_[i - 1]) / tps_[i - 1] * 1000.0;
    }
}

int MsIndex::span_at(int64_t ticks) const { return index_at(keys_, ticks, kSelf); }

double MsIndex::at(int64_t ticks) const {
    const int i = span_at(ticks);
    return elapsed_[i] +
        static_cast<double>(ticks - keys_[i]) / tps_[i] * 1000.0;
}

double MsIndex::tps_at(int64_t ticks) const { return tps_[span_at(ticks)]; }

double MsIndex::bpm_at(int64_t ticks) const { return bpm_[span_at(ticks)]; }

double MsIndex::ms_at_tick_f(double ticks) const {
    // Same right-continuous section rule as at(): a tick exactly on a tempo
    // change reads the new tempo. floor() keeps fractional ticks just below a
    // boundary in the earlier section.
    const int i = span_at(static_cast<int64_t>(std::floor(ticks)));
    return elapsed_[i] +
           (ticks - static_cast<double>(keys_[i])) / tps_[i] * 1000.0;
}

double MsIndex::tick_at_ms(double ms) const {
    // Inverse of ms_at_tick_f: last section whose elapsed start <= ms, then
    // the same slope inverted.
    const int i = index_at(elapsed_, ms, kSelf);
    return static_cast<double>(keys_[i]) + (ms - elapsed_[i]) * tps_[i] / 1000.0;
}

// ---- MeasureIndex (the grid) --------------------------------------------

MeasureIndex::MeasureIndex(const std::map<int64_t, std::pair<int, int>>& sig_map,
                           int64_t tick_r) {
    if (sig_map.find(0) == sig_map.end())
        throw std::out_of_range("time signature map has no tick-0 entry");

    for (const auto& [tick, sig] : sig_map) {
        if (tick < 0) continue;  // the grid starts at tick 0
        Span s;
        s.numerator = sig.first;
        s.denominator = sig.second;
        // Clone Hero's beat step: trunc(resolution * 4 / denominator).
        s.step = tick_r * 4 / static_cast<int64_t>(sig.second);
        if (s.step <= 0)
            throw std::out_of_range("the time signature at tick " + std::to_string(tick) +
                                    " makes a beat shorter than one tick");
        s.length = s.step * static_cast<int64_t>(sig.first);
        if (spans_.empty()) {
            spans_.push_back(s);
            continue;
        }
        // The grid first sees a signature at the first step of the span in
        // force at or after its tick. When that step is the restart of the
        // span before (two signatures between the same two steps), this one
        // is the one in force there, and the earlier one never shows.
        int64_t restart;
        if (spans_.size() > 1 && tick <= spans_.back().restart) {
            restart = spans_.back().restart;
            spans_.pop_back();
        } else {
            const Span& p = spans_.back();
            restart = p.restart + ceil_div(tick - p.restart, p.step) * p.step;
        }
        const Span& prev = spans_.back();
        // Clone Hero compares the values: the same meter again changes nothing.
        if (prev.numerator == s.numerator && prev.denominator == s.denominator) continue;
        s.restart = restart;
        // A cut-short measure before the restart still has its measure line.
        s.lines_before = prev.lines_before + ceil_div(restart - prev.restart, prev.length);
        spans_.push_back(s);
    }
}

int MeasureIndex::span_at(int64_t ticks) const {
    return index_at(spans_, ticks, [](const Span& s) { return s.restart; });
}

MeasureIndex::Position MeasureIndex::at(int64_t ticks) const {
    const Span& s = spans_[span_at(ticks)];
    const int64_t rel = ticks - s.restart;
    const int64_t in_measure = rel % s.length;
    Position p;
    p.measure = s.lines_before + rel / s.length;
    p.beat = in_measure / s.step;
    p.tick = in_measure % s.step;
    return p;
}

void MeasureIndex::walk(int64_t last_tick,
                        const std::function<void(int64_t, Line)>& fn) const {
    for (int i = 0; i < count(); ++i) {
        const Span& s = spans_[i];
        const int64_t end = i + 1 < count() ? spans_[i + 1].restart : last_tick + 1;
        int64_t beat = 0;
        for (int64_t t = s.restart; t < end && t <= last_tick; t += s.step, ++beat) {
            if (beat == s.numerator) beat = 0;
            Line line = Line::Beat;
            if (beat == 0)
                line = Line::Measure;
            else if (s.denominator > 4 && beat % (s.denominator / 4) != 0)
                line = Line::WeakBeat;
            fn(t, line);
        }
    }
}

// ---- MeasureCount (the continuous count) ---------------------------------

MeasureCount::MeasureCount(const std::map<int64_t, int64_t>& tpm_map) {
    if (tpm_map.find(0) == tpm_map.end())
        throw std::out_of_range("tpm map has no tick-0 entry");

    starts_.reserve(tpm_map.size());
    tpm_.reserve(tpm_map.size());
    for (const auto& kv : tpm_map) {
        starts_.push_back(kv.first);
        tpm_.push_back(kv.second);
    }
    elapsed_.assign(starts_.size(), 0.0);
    for (size_t i = 1; i < starts_.size(); ++i) {
        const double tpm = static_cast<double>(tpm_[i - 1]);
        elapsed_[i] = elapsed_[i - 1] + static_cast<double>(starts_[i] - starts_[i - 1]) / tpm;
    }
}

int MeasureCount::span_at(int64_t ticks) const { return index_at(starts_, ticks, kSelf); }

double MeasureCount::measures_at(double ticks) const {
    const int i = span_at(static_cast<int64_t>(std::floor(ticks)));
    const double tpm = static_cast<double>(tpm_[i]);
    // Whole measures, then the part measure: on whole ticks this keeps the
    // values stored before the count was continuous, bit for bit, wherever
    // meter changes land on measure lines.
    const double rel = ticks - static_cast<double>(starts_[i]);
    const double whole = std::floor(rel / tpm);
    return elapsed_[i] + whole + (rel - whole * tpm) / tpm;
}

double MeasureCount::tick_at(double measures) const {
    const int i = index_at(elapsed_, measures, kSelf);
    return static_cast<double>(starts_[i]) +
           (measures - elapsed_[i]) * static_cast<double>(tpm_[i]);
}

int64_t MeasureCount::tick_after(int64_t ticks, int64_t add) const {
    if (add == 0) return ticks;
    // The measures still to walk, as the reduced fraction num / den.
    int64_t num = add > 0 ? add : -add;
    int64_t den = 1;
    int64_t at = ticks;
    int i = span_at(ticks);
    auto spend = [&](int64_t room, int64_t tpm) {
        num = num * tpm - room * den;
        den *= tpm;
        const int64_t g = std::gcd(num, den);
        num /= g;
        den /= g;
    };
    if (add > 0) {
        for (;; ++i) {
            const int64_t tpm = tpm_[i];
            if (i + 1 < count()) {
                const int64_t room = starts_[i + 1] - at;
                if (num * tpm > room * den) {
                    spend(room, tpm);
                    at = starts_[i + 1];
                    continue;
                }
            }
            return at + floor_div(num * tpm, den);
        }
    }
    for (;; --i) {
        const int64_t tpm = tpm_[i];
        if (i > 0) {
            const int64_t room = at - starts_[i];
            if (num * tpm > room * den) {
                spend(room, tpm);
                at = starts_[i];
                continue;
            }
        }
        return at + floor_div(-(num * tpm), den);
    }
}

// ---- Timecode -----------------------------------------------------------

Timecode::Timecode(int64_t ticks, const MeasureIndex& grid, const MeasureCount& count,
                   const MsIndex& ms)
    : ticks_(ticks) {
    const MeasureIndex::Position p = grid.at(ticks_);
    mbt_[0] = p.measure;
    mbt_[1] = p.beat;
    mbt_[2] = p.tick;
    measures_decimal_ = count.measures_at(static_cast<double>(ticks_));
    ms_ = ms.at(ticks_);
}

// ---- SongTiming ---------------------------------------------------------

namespace {
// The time signature map, once it names exactly the tpm map's ticks.
const std::map<int64_t, std::pair<int, int>>& same_ticks(
    const std::map<int64_t, int64_t>& tpm_map,
    const std::map<int64_t, std::pair<int, int>>& sig_map) {
    const bool same =
        tpm_map.size() == sig_map.size() &&
        std::equal(tpm_map.begin(), tpm_map.end(), sig_map.begin(),
                   [](const auto& a, const auto& b) { return a.first == b.first; });
    if (!same) throw std::out_of_range("time signature and tpm maps name different ticks");
    return sig_map;
}
}  // namespace

SongTiming::SongTiming(int64_t tick_r,
                       const std::map<int64_t, int64_t>& tpm_map,
                       const std::map<int64_t, double>& bpm_map,
                       const std::map<int64_t, std::pair<int, int>>& sig_map)
    : tick_r_(tick_r),
      grid_(same_ticks(tpm_map, sig_map), tick_r),
      count_(tpm_map),
      ms_(bpm_map, tick_r) {}

Timecode SongTiming::plusmeasure(const Timecode& tc, int64_t add_measures) const {
    return timecode(count_.tick_after(tc.ticks(), add_measures));
}

double SongTiming::tempo_at(int64_t ticks) const { return ms_.bpm_at(ticks); }

std::pair<int, int> SongTiming::time_signature_at(int64_t ticks) const {
    const int i = grid_.span_at(ticks);
    return {grid_.numerator_at(i), grid_.denominator_at(i)};
}

double SongTiming::ms_per_measure_at(int64_t ticks) const {
    const int64_t tpm = count_.tpm_at(count_.span_at(ticks));
    return static_cast<double>(tpm) / ms_.tps_at(ticks) * 1000.0;
}

double SongTiming::measures_at_tick_f(double ticks) const { return count_.measures_at(ticks); }

double SongTiming::tick_at_measures_f(double measures) const { return count_.tick_at(measures); }

double SongTiming::sp_end_ms(double act_hit_ms, int64_t end_measures) const {
    // ms -> tick -> measure position, + the SP length in measures, and back.
    // Every step is continuous piecewise-linear, so the composition is the
    // exact E(h) map the transfer-scale ratio linearizes.
    double t = ms_.tick_at_ms(act_hit_ms);
    double m = measures_at_tick_f(t) + static_cast<double>(end_measures);
    return ms_.ms_at_tick_f(tick_at_measures_f(m));
}

int64_t SongTiming::display_tick_at_ms(double ms) const {
    const int64_t tick = std::llround(ms_.tick_at_ms(ms));
    return tick < 0 ? 0 : tick;
}

}  // namespace hydra
