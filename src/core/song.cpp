#include "core/song.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <optional>
#include <string>

#include "core/error_kind.h"

namespace hydra {

// The timing maps Hydra can measure time with: a positive resolution, a tempo
// at tick 0, every measure at least one tick long, every tempo a positive,
// finite BPM. Both
// parsers reach this through Song::build_timing, so no chart with a zero,
// negative or infinite measure length reaches the engine (D4, R7.6).
void check_timing_maps(int64_t tick_resolution,
                       const std::map<int64_t, int64_t>& tpm_changes,
                       const std::map<int64_t, double>& bpm_changes) {
    constexpr ErrorKind kRefused = ErrorKind::ChartTimingRefused;
    if (tick_resolution <= 0)
        throw ChartFileError(kRefused, std::string(kResolutionRefusalPrefix) +
                                           std::to_string(tick_resolution) +
                                           ", and it must be above 0");
    // A song's time is measured from its opening tempo, so a chart without one
    // is refused rather than given a guessed tempo (the user's ruling,
    // PLAINERR-RES). core/timing.cpp keeps its own throw as a backstop.
    if (bpm_changes.find(0) == bpm_changes.end())
        throw ChartFileError(kRefused, "it has no tempo at tick 0");
    for (const auto& [tick, len] : tpm_changes)
        if (len <= 0)
            throw ChartFileError(kRefused, std::string(kTimeSignatureRefusalPrefix) +
                                               std::to_string(tick) + " makes a measure " +
                                               std::to_string(len) + " ticks long");
    for (const auto& [tick, bpm] : bpm_changes) {
        // A .mid tempo of 0 microseconds per beat reads back as an infinite BPM
        // (D12): say so, rather than "not above 0 BPM".
        if (std::isinf(bpm) && bpm > 0.0)
            throw ChartFileError(kRefused, std::string(kTempoRefusalPrefix) +
                                               std::to_string(tick) +
                                               " is infinite (0 microseconds per beat)");
        if (!std::isfinite(bpm) || bpm <= 0.0)
            throw ChartFileError(kRefused, std::string(kTempoRefusalPrefix) +
                                               std::to_string(tick) + " is not above 0 BPM");
    }
}

// A time signature: ticks per measure = resolution * 4 * num / den. The
// signature itself is kept too, for display. A numerator of 0 names no meter,
// so the line is ignored and the previous meter stays, in both formats
// (finding 100). A bottom number the readers couldn't hold (0, from
// timesig_denominator) refuses the chart here, where the tick is known, and so
// does one whose beat would be shorter than one tick, which Clone Hero's grid
// cannot step (MeasureIndex; user decision 2026-10-14). Anything else that
// makes a measure last no time is refused by check_timing_maps.
void apply_timesig(Song& song, int64_t tick, int numerator, int denominator) {
    if (numerator == 0) return;
    // A resolution of 0 or below is check_timing_maps' to refuse, with its
    // own sentence.
    const bool beat_under_a_tick =
        song.tick_resolution() > 0 && song.tick_resolution() * 4 < denominator;
    if (denominator <= 0 || beat_under_a_tick)
        throw ChartFileError(ErrorKind::ChartTimingRefused,
                             std::string(kTimeSignatureRefusalPrefix) + std::to_string(tick) +
                                 " has a bottom number that is out of range");
    song.tpm_changes[tick] = song.tick_resolution() * static_cast<int64_t>(numerator) * 4 /
                             static_cast<int64_t>(denominator);
    song.timesig_changes[tick] = {numerator, denominator};
}

bool cymbal_counts(NoteColor pad, bool pro) {
    return pro && allows_cymbals(pad);
}

Song::Song(int64_t resolution) : tick_resolution_(resolution) {
    apply_timesig(*this, 0, kDefaultTimeSigNumerator, kDefaultTimeSigDenominator);
}

// ---- Song::sp_phrase_count ----------------------------------------------

int Song::sp_phrase_count() const {
    int n = 0;
    for (const SongTimestamp& ts : sequence)
        if (ts.flag_sp) ++n;
    return n;
}

// ---- Song::note_count -----------------------------------------------------

int Song::note_count() const {
    int n = 0;
    for (const SongTimestamp& ts : sequence) n += ts.chord.count();
    return n;
}

// ---- Song::check_activations -------------------------------------------

// The one owner of generated-fill placement, for a chart with no fills of its
// own, and the one place that picks which game version's generator runs: the
// rule the run is scored under (FillDeadlineRule; search/graph.h's
// fill_rule_for reads it off a setting). Both generators count measures on
// the song's grid (MeasureIndex); each keeps its own game version's fill rule.
void Song::check_activations(const core::Rules& rules, FillDeadlineRule fills) {
    for (const SongTimestamp& ts : sequence)
        if (ts.has_activation()) return;  // chart already has fills.

    features.push_back("Auto-Generated Fills");
    generated_fills = fills;
    if (sequence.empty()) return;
    if (fills == FillDeadlineRule::Ch10)
        place_fills_ch10(rules);
    else
        place_fills_ch11(rules);
}

namespace {

// How long a Clone Hero 1.0 generated fill is, in measures of the grid
// measure in force at its chord (CONTEXT.md, "Generated fill"). Only the 1.0
// deadline (activation_fill_deadline_ms's Ch10 branch) reads the length.
constexpr double kCh10FillLengthMeasures = 0.5;

}  // namespace

// Clone Hero 1.0 mode's generator. It reads the measure lines of Clone Hero's
// grid (MeasureIndex), as place_fills_ch11 does; only the fill rule is 1.0's.
// Each chord is a candidate for the measure line that starts its own measure
// and for the next one; each line keeps its nearest chord, a tie going to the
// later chord. Lines are then taken in order: one whose chord is within
// resolution x fill_max_distance_beats (no extra tick) gets a fill of
// kCh10FillLengthMeasures of the grid measure in force at the chord, and the
// next fill_cooldown_measures lines are skipped.
void Song::place_fills_ch10(const core::Rules& rules) {
    const MeasureIndex& grid = timing().measure_index();

    struct Cell {
        size_t best = 0;        // index into sequence of the closest chord
        int64_t bestdist = 0;
    };
    // Keyed by measure line number, so the lines are visited in order.
    std::map<int64_t, Cell> lines;
    for (size_t i = 0; i < sequence.size(); ++i) {
        const int64_t tick = sequence[i].timecode.ticks();
        const int64_t own = grid.at(tick).measure;
        for (int64_t line : {own, own + 1}) {
            const int64_t dist = std::llabs(grid.measure_line_tick(line) - tick);
            const auto [it, added] = lines.try_emplace(line, Cell{i, dist});
            if (!added && dist <= it->second.bestdist) it->second = Cell{i, dist};
        }
    }

    const int64_t cooldown_measures = rules.fill_cooldown_measures;
    const int64_t max_distance =
        static_cast<int64_t>(tick_resolution_ * rules.fill_max_distance_beats);
    std::optional<int64_t> last_fill_line;
    for (const auto& [line, cell] : lines) {
        if (last_fill_line.has_value() && line < *last_fill_line + cooldown_measures)
            continue;
        if (cell.bestdist > max_distance) continue;
        SongTimestamp& chord = sequence[cell.best];
        const int64_t measure = grid.length_at(grid.span_at(chord.timecode.ticks()));
        chord.activation_length = static_cast<int64_t>(measure * kCh10FillLengthMeasures);
        last_fill_line = line;
    }
}

// Clone Hero 1.1.0.6142's generator (0x20D2B30), for every mode but 1.0. It reads
// the measure lines of Clone Hero's grid (MeasureIndex) and starts at
// the second line (`mov ebx, 1`). Each line takes the chord nearest it, a tie
// going to the later chord, when that chord is within the distance limit; the
// fill then starts on the line, so its length is the chord tick minus the
// line tick and may be 0 or negative for a chord on or before the line (user
// answer 6, 2026-10-12). The next candidate is rules.fill_cooldown_measures
// lines on after a fill (`add ebx, 4`), else the next line.
//
// The limit is Clone Hero's: the whole ticks of resolution x
// fill_max_distance_beats, plus one tick (inc at 0x20D2C91), and a chord
// exactly at that distance still takes the fill. The +1 sits on top of the
// rules value, as place_authored_fills' (parse/song.cpp) does.
void Song::place_fills_ch11(const core::Rules& rules) {
    const int64_t max_distance =
        static_cast<int64_t>(tick_resolution_ * rules.fill_max_distance_beats) + 1;

    // Lines are visited by number, so the cost follows the chords, not the
    // measure lines between them. The grid runs to the chart's end in Clone
    // Hero; past the last chord plus the limit no line can take a chord.
    const MeasureIndex& grid = timing().measure_index();
    const int64_t last_reachable = sequence.back().timecode.ticks() + max_distance;

    // `next` is the first chord at or after the line, searched onward from
    // the last chord looked at; the chord before it is the other candidate.
    size_t next = 0;
    int64_t line = 1;
    for (;;) {
        const int64_t line_tick = grid.measure_line_tick(line);
        if (line_tick > last_reachable) break;
        while (next < sequence.size() && sequence[next].timecode.ticks() < line_tick) ++next;
        std::optional<size_t> best;
        if (next < sequence.size()) best = next;
        if (next > 0) {
            const size_t before = next - 1;
            if (!best.has_value() ||
                line_tick - sequence[before].timecode.ticks() <
                    sequence[*best].timecode.ticks() - line_tick)
                best = before;
        }
        const int64_t chord_tick = sequence[*best].timecode.ticks();
        if (std::llabs(chord_tick - line_tick) > max_distance) {
            // Out of reach here. The chords before this line only get farther
            // from later lines, so no line takes a chord until one is within
            // the limit of `next`: skip straight to the first such line, the
            // same one stepping a line at a time would stop on.
            if (next == sequence.size()) break;
            line = std::max(line + 1, grid.first_measure_line_at_or_after(
                                          sequence[next].timecode.ticks() - max_distance));
            continue;
        }
        sequence[*best].activation_length = chord_tick - line_tick;
        // hydra_rules.ini refuses a cooldown below 1 (rules_fields); a Rules
        // value built in code gets the same floor, so the walk always moves.
        line += static_cast<int64_t>(std::max(rules.fill_cooldown_measures, 1));
    }
}

}  // namespace hydra
