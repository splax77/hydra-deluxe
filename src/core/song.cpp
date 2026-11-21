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
    // docs/sessions/2026-10-24-a124ccd9.md). core/timing.cpp keeps its own
    // throw as a backstop.
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
// does one whose beat the grid cannot step (beat_under_a_tick in
// core/timing.h). Anything else that
// makes a measure last no time is refused by check_timing_maps.
void apply_timesig(Song& song, int64_t tick, int numerator, int denominator) {
    if (numerator == 0) return;
    // A resolution of 0 or below is check_timing_maps' to refuse, with its
    // own sentence.
    if (denominator <= 0 ||
        (song.tick_resolution() > 0 && beat_under_a_tick(song.tick_resolution(), denominator)))
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

// ---- chords_around --------------------------------------------------------

// The game's two searches are the first chord after the tick and the last at
// or before it (fill_generator.cs lines 170 to 194); one binary search finds
// both, as they sit side by side in a tick-ordered sequence. The game's
// search for the chord after starts at `search_from` (line 174), and its
// search for the chord before walks down from there to chord 0 (line 188,
// with F12), so `search_from` moves only the chord after.
ChordsAround chords_around(const std::vector<SongTimestamp>& sequence, int64_t tick,
                           int64_t earliest, int64_t latest, size_t search_from) {
    const auto it = std::upper_bound(
        sequence.begin(), sequence.end(), tick,
        [](int64_t t, const SongTimestamp& c) { return t < c.timecode.ticks(); });
    // Chords [0, split) sit at or before the tick.
    const size_t split = static_cast<size_t>(it - sequence.begin());
    const size_t first_after = (std::max)(split, search_from);
    ChordsAround out;
    if (first_after < sequence.size() && sequence[first_after].timecode.ticks() <= latest)
        out.after = first_after;
    if (split > 0 && sequence[split - 1].timecode.ticks() >= earliest) out.before = split - 1;
    out.pick = out.after.has_value() ? out.after : out.before;
    if (out.before.has_value() && out.after.has_value()) {
        const int64_t to_after = sequence[*out.after].timecode.ticks() - tick;
        const int64_t to_before = tick - sequence[*out.before].timecode.ticks();
        if (to_after > to_before) out.pick = out.before;
    }
    return out;
}

// ---- Song::check_activations -------------------------------------------

// The one owner of generated-fill placement, for a chart with no fills of its
// own, and the one place that picks which game version's generator runs: the
// rule the run is scored under (FillDeadlineRule; search/graph.h's
// fill_rule_for reads it off a setting). Both generators count measures on
// the song's grid (MeasureIndex); each keeps its own game version's fill rule.
// A written fill the coda skipped counts as the chart's own (coda_skipped_fill
// in core/song.h says why).
void Song::check_activations(const core::Rules& rules, FillDeadlineRule fills) {
    if (coda_skipped_fill) return;  // chart wrote fills; the coda skipped them.
    for (const SongTimestamp& ts : sequence)
        if (ts.has_activation()) return;  // chart already has fills.

    features.push_back(kGeneratedFillsFeature);
    generated_fills = fills;
    if (sequence.empty()) return;
    if (fills == FillDeadlineRule::Ch10)
        place_fills_ch10();
    else
        place_fills_ch11(rules);
}

namespace {

// Clone Hero 1.0's generator is F1 of docs/clone-hero/code/4045/
// fill_generator.cs (lines 31 to 79 of that file); its clock is F9 and the
// rounding F13, explained in fill_generator.md.

// After a fill, both games' generators move on this many measure lines. It is
// a literal in each: 1.0's loop step `i += 4` (fill_generator.cs line 42) and
// 1.1's `add ebx, 4` (FL-17 in docs/clone-hero/04-fills.md).
constexpr size_t kLinesAfterFill = 4;

// How far in real time, in seconds, a chord may sit from its measure line and
// still take a 1.0 generated fill: the game's `num5 > 0.25f`, so exactly this
// far still takes it (fill_generator.cs line 55). The user asked for this
// rule ("Yes, add them", docs/superpowers/plans/2026-11-21-ch10-fills.md).
constexpr float kCh10FillReachSeconds = 0.25f;

// The game's tick-to-seconds (F9, fill_generator.cs lines 535 to 545): a
// float second, not rounded. Hydra's one tick-to-milliseconds owner,
// MsIndex::at, works it out in double; the result is narrowed to the game's
// float here.
float ch10_second_at_tick(const MsIndex& ms, int64_t tick) {
    return static_cast<float>(ms.at(tick) / 1000.0);
}

// The game's seconds-to-tick (F9, fill_generator.cs lines 601 to 623): a
// negative second reads as 0; the tempo in force is the last one starting
// strictly before the second; the ticks since that tempo are rounded half to
// even (Math.Round, F13) and added to its tick. MsIndex::tick_at_ms gives the
// unrounded tick, from the last tempo starting at or before the second. On a
// second exactly at a tempo's start the two tempos give that tempo's own tick,
// so only the rounding is copied here, on the ticks since the tempo
// tick_at_ms read.
int64_t ch10_tick_at_second(const MsIndex& ms, float second) {
    if (second < 0.0f) second = 0.0f;
    const double tick = ms.tick_at_ms(static_cast<double>(second) * 1000.0);
    const std::vector<int64_t>& changes = ms.change_ticks();
    const int64_t tempo_tick = *last_at_or_before(
        changes.begin(), changes.end(), static_cast<int64_t>(std::floor(tick)),
        [](int64_t t) { return t; });
    // std::nearbyint rounds a half to even in the default rounding mode.
    return tempo_tick + static_cast<int64_t>(std::nearbyint(tick - static_cast<double>(tempo_tick)));
}

}  // namespace

// Clone Hero 1.0 mode's generator, the game's F1 loop. It walks 1.0's measure
// lines (ch10_measure_lines, up to ch10_last_line_tick) from the second one
// (`i = 1`, line 42). Each line takes the chord chords_around picks for the
// tick of the line's second, searching on from the last chord that took a
// fill (line 46). A line no chord follows takes chord 0, as the game's search
// does (F4 in fill_generator.md), so it gets a fill only when chord 0 is
// within the reach. A chord more than kCh10FillReachSeconds from the line, in
// float seconds, is passed over and the next line tried (`i -= 3`, line 57).
// Otherwise the fill starts on the tick of the second halfway between the
// line before and this one (lines 44 and 65), so its length is the chord tick
// minus that tick and can be negative; the next candidate is
// kLinesAfterFill lines on. The 64th note the game adds past the chord
// (lines 41 and 72) belongs to the deadline, activation_fill_deadline_ms.
// The game's miss branch (`num4 == -1`, line 47) never runs on a chart with a
// chord, which is the only chart check_activations hands here.
void Song::place_fills_ch10() {
    const MsIndex& ms = timing().ms_index();
    const std::vector<int64_t> lines =
        ch10_measure_lines(timing().measure_index(), ch10_last_line_tick(*this));
    size_t last_used = 0;
    size_t i = 1;
    while (i < lines.size()) {
        const float before_s = ch10_second_at_tick(ms, lines[i - 1]);
        const float line_s = ch10_second_at_tick(ms, lines[i]);
        // Float arithmetic in the game's order (line 44).
        const float start_s = before_s + (line_s - before_s) / 2.0f;
        const ChordsAround around =
            chords_around(sequence, ch10_tick_at_second(ms, line_s),
                          std::numeric_limits<int64_t>::min(),
                          std::numeric_limits<int64_t>::max(), last_used);
        // F4's first search returns chord 0 when no chord follows (line 182).
        const size_t chord = around.after.has_value() ? *around.pick : 0;
        const int64_t chord_tick = sequence[chord].timecode.ticks();
        const float off_s = std::fabs(ch10_second_at_tick(ms, chord_tick) - line_s);
        if (off_s > kCh10FillReachSeconds) {
            ++i;
            continue;
        }
        last_used = chord;
        const int64_t start = ch10_tick_at_second(ms, start_s);
        sequence[chord].activation_length = chord_tick - start;
        i += kLinesAfterFill;
    }
}

// Clone Hero 1.1.0.6142's generator (0x20D2B30), for every mode but 1.0. It reads
// the measure lines of Clone Hero's grid (MeasureIndex) and starts at
// the second line (`mov ebx, 1`). Each line takes the chord chords_around
// picks for it, when that chord is within the distance limit; the
// fill then starts on the line, so its length is the chord tick minus the
// line tick and may be 0 or negative for a chord on or before the line
// (question 6, as recommended, of
// docs/superpowers/plans/2026-10-12-one-record-per-chart.md (removed; git show 444631e6:docs/superpowers/plans/2026-10-12-one-record-per-chart.md)).
// The next candidate is kLinesAfterFill lines on after a fill
// (`add ebx, 4`), else the next line.
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

    int64_t line = 1;
    for (;;) {
        const int64_t line_tick = grid.measure_line_tick(line);
        if (line_tick > last_reachable) break;
        const ChordsAround around = chords_around(sequence, line_tick);
        const size_t best = *around.pick;  // the sequence is not empty
        // `next` is the first chord at or after the line.
        const size_t next =
            around.before.has_value() && sequence[*around.before].timecode.ticks() == line_tick
                ? *around.before
                : around.after.value_or(sequence.size());
        const int64_t chord_tick = sequence[best].timecode.ticks();
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
        sequence[best].activation_length = chord_tick - line_tick;
        line += static_cast<int64_t>(kLinesAfterFill);
    }
}

// ---- Clone Hero 1.0's measure lines ----------------------------------------

// The game adds one beat of its timing resolution, the chart's resolution
// times the song speed (F9 and F10 of fill_generator.md); Hydra scores at 100%
// speed, so that beat is the chart's resolution.
int64_t ch10_last_line_tick(const Song& song) {
    return song.last_note_end_tick.value_or(0) + song.tick_resolution();
}

// MeasureIndex::walk already restarts the grid where the game's walk does
// (fill_generator.cs lines 211 to 220). The one place the two disagree is a
// meter with 1 on top: MeasureIndex makes every beat a measure line there,
// while 1.0 keeps only the meter's first beat as one (line 223 tests the
// numerator against 1, and the beats since the change against 0).
std::vector<int64_t> ch10_measure_lines(const MeasureIndex& grid, int64_t last_tick) {
    std::vector<int64_t> lines;
    grid.walk(last_tick, [&](int64_t tick, MeasureIndex::Line line) {
        if (line != MeasureIndex::Line::Measure) return;
        const int span = grid.span_at(tick);
        if (grid.numerator_at(span) == 1 && tick != grid.restart_at(span)) return;
        lines.push_back(tick);
    });
    return lines;
}

}  // namespace hydra
