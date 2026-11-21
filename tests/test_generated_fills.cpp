// Tests for generated fills (Song::check_activations, core/song.cpp). A chart
// with no fills of its own gets fills placed on the measure lines of Clone
// Hero's grid (MeasureIndex). Each expected tick here is read from that grid,
// and each rule value from core::Rules or search/graph.h, never typed.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "corpus_util.h"
#include "parse/song.h"
#include "search/graph.h"

using namespace hydra;

namespace {

// The resolution of the songs these tests build.
constexpr int64_t kRes = 480;

// A song at resolution kRes and 120 BPM with one red note at each tick in
// `chords`, and no fills yet. `seven_eight_at`, when above 0, switches 4/4 to
// 7/8 there.
Song song_on_grid(const std::vector<int64_t>& chords, int64_t seven_eight_at = 0) {
    Song song(kRes);
    song.bpm_changes[0] = 120.0;
    if (seven_eight_at > 0) apply_timesig(song, seven_eight_at, 7, 8);
    song.build_timing();
    for (int64_t tick : chords) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    return song;
}

// song_on_grid's song with its fills generated the way the game version
// `fills` names would.
Song song_with_chords(const std::vector<int64_t>& chords, int64_t seven_eight_at = 0,
                      FillDeadlineRule fills = FillDeadlineRule::Ch11) {
    Song song = song_on_grid(chords, seven_eight_at);
    song.check_activations(core::default_rules(), fills);
    return song;
}

// Each generated fill as {start tick, chord tick}.
std::vector<std::pair<int64_t, int64_t>> fills_of(const Song& song) {
    std::vector<std::pair<int64_t, int64_t>> out;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation())
            out.emplace_back(fill_start_tick(ts.timecode.ticks(), *ts.activation_length),
                             ts.timecode.ticks());
    return out;
}

using Fills = std::vector<std::pair<int64_t, int64_t>>;

// The tick of measure line `n` (0-based) on the grid song_on_grid builds for
// the same meter map, read from that song's MeasureIndex.
int64_t line_tick(int64_t n, int64_t seven_eight_at = 0) {
    const Song grid = song_on_grid({}, seven_eight_at);
    return grid.timing().measure_index().measure_line_tick(n);
}

// The number of the first measure line at or after `tick` on that grid.
int64_t line_at_or_after(int64_t tick, int64_t seven_eight_at = 0) {
    const Song grid = song_on_grid({}, seven_eight_at);
    return grid.timing().measure_index().first_measure_line_at_or_after(tick);
}

// How far from its measure line a chord may sit and still take a Clone Hero
// 1.1 generated fill, in ticks of `resolution`. place_fills_ch11 works this
// out inline and exposes no function for it, so this follows its form: the
// whole ticks of resolution x core::Rules::fill_max_distance_beats, plus the
// one tick Clone Hero adds.
int64_t ch11_reach(int64_t resolution, const core::Rules& rules) {
    return static_cast<int64_t>(static_cast<double>(resolution) * rules.fill_max_distance_beats) +
           1;
}

// The length of the grid measure in force at a tick of `song`.
int64_t measure_length_at(const Song& song, int64_t tick) {
    const MeasureIndex& grid = song.timing().measure_index();
    return grid.length_at(grid.span_at(tick));
}

// Whether two Clone Hero 1.0 fills are the same share of the grid measure in
// force at their chords. 1.0 sizes a fill by that measure alone, so every
// 1.0 fill has one share whatever its meter or its chord's distance to the
// line.
bool same_measure_share(const Song& a, const SongTimestamp& fa, const Song& b,
                        const SongTimestamp& fb) {
    return *fa.activation_length * measure_length_at(b, fb.timecode.ticks()) ==
           *fb.activation_length * measure_length_at(a, fa.timecode.ticks());
}

// A 4/4 song whose one chord sits on the second measure line, with its 1.0
// fill: the share every other 1.0 fill is compared with.
Song ch10_reference() {
    return song_with_chords({line_tick(1)}, 0, FillDeadlineRule::Ch10);
}

// A chord on every measure line from `first` to `last` (4/4). The 1.1 fills
// sit on the first such line from the second on, then every
// core::Rules::fill_cooldown_measures lines on.
void check_chord_on_every_line(int64_t first, int64_t last) {
    const int64_t cooldown = core::default_rules().fill_cooldown_measures;
    std::vector<int64_t> chords;
    for (int64_t n = first; n <= last; ++n) chords.push_back(line_tick(n));
    Fills want;
    for (int64_t n = (std::max)(first, int64_t{1}); n <= last; n += cooldown)
        want.emplace_back(line_tick(n), line_tick(n));
    CHECK(fills_of(song_with_chords(chords)) == want);
}

// The 7/8 change of the restarted-grid tests: one and a half 4/4 measures in.
constexpr int64_t kMidMeasureChange = 6 * kRes;

// Three chords on the grid restarted at kMidMeasureChange: just after the
// restart line, just after the next line (inside the cooldown) and just after
// the line the cooldown reaches.
std::vector<int64_t> restart_chords() {
    const int64_t restart = line_at_or_after(kMidMeasureChange, kMidMeasureChange);
    const int64_t cooldown = core::default_rules().fill_cooldown_measures;
    return {line_tick(restart, kMidMeasureChange) + 120,
            line_tick(restart + 1, kMidMeasureChange) + 100,
            line_tick(restart + cooldown, kMidMeasureChange) + 50};
}

}  // namespace

TEST_CASE("generated fills: a chord at the 1.1 reach after the second measure line gets a fill") {
    const int64_t line = line_tick(1);
    const int64_t reach = ch11_reach(kRes, core::default_rules());
    const Song song = song_with_chords({line + reach});
    REQUIRE(song.sequence.size() == 1);
    const SongTimestamp& ts = song.sequence[0];
    REQUIRE(ts.has_activation());
    CHECK(fill_start_tick(ts.timecode.ticks(), *ts.activation_length) == line);
    CHECK(*ts.activation_length == reach);
    // The 1.1 deadline sits kCh11FillLeadBeats beats before the fill's line.
    const double lead_ms = song.timing().timecode(line - kCh11FillLeadBeats * kRes).ms();
    CHECK(activation_fill_deadline_ms(song.timing(), ts.timecode.ticks(), *ts.activation_length,
                                      FillDeadlineRule::Ch11) == lead_ms);
}

TEST_CASE("generated fills: Clone Hero 1.0 mode sizes fills by the measure, 1.1 starts them on the line") {
    const int64_t cooldown = core::default_rules().fill_cooldown_measures;
    // One chord 10 ticks after each of two lines a cooldown apart.
    const std::vector<int64_t> chords = {line_tick(1) + 10, line_tick(1 + cooldown) + 10};
    CHECK(fills_of(song_with_chords(chords, 0, FillDeadlineRule::Ch11)) ==
          Fills{{line_tick(1), chords[0]}, {line_tick(1 + cooldown), chords[1]}});
    // 1.0: each fill is the same share of its measure as the reference fill,
    // whose chord sits on its line, so the chord's distance does not size it.
    const Song old = song_with_chords(chords, 0, FillDeadlineRule::Ch10);
    const Song ref = ch10_reference();
    REQUIRE(ref.sequence[0].has_activation());
    for (const SongTimestamp& ts : old.sequence) {
        INFO("chord tick " << ts.timecode.ticks());
        REQUIRE(ts.has_activation());
        CHECK(same_measure_share(old, ts, ref, ref.sequence[0]));
    }
}

TEST_CASE("generated fills: a chord one tick past the 1.1 reach from its line gets none") {
    const int64_t line = line_tick(1);
    const int64_t reach = ch11_reach(kRes, core::default_rules());
    CHECK(fills_of(song_with_chords({line + reach + 1})).empty());
    CHECK(fills_of(song_with_chords({line - (reach + 1)})).empty());
}

TEST_CASE("generated fills: a chord exactly on the line has length 0") {
    const int64_t line = line_tick(1);
    const Song song = song_with_chords({line});
    CHECK(fills_of(song) == Fills{{line, line}});
    REQUIRE(song.sequence[0].has_activation());
    CHECK(*song.sequence[0].activation_length == song.sequence[0].timecode.ticks() - line);
}

TEST_CASE("generated fills: a chord just before its line has a negative length") {
    const int64_t line = line_tick(1);
    const int64_t chord = line - 100;
    const Song song = song_with_chords({chord});
    REQUIRE(song.sequence[0].has_activation());
    // The fill starts on the line, so its length is the chord tick minus the
    // line tick, which is below zero here.
    CHECK(fill_start_tick(chord, *song.sequence[0].activation_length) == line);
    CHECK(*song.sequence[0].activation_length < 0);
}

TEST_CASE("generated fills: the 1.0 deadline of a 0 or negative length fill depends only on its line") {
    // Both chords take a 1.1 fill that starts on the same line: one sits on
    // it (length 0), one before it (a negative length).
    const int64_t line = line_tick(1);
    std::vector<double> deadlines;
    double line_ms = 0.0;
    for (int64_t chord : {line, line - 100}) {
        INFO("chord tick " << chord);
        const Song song = song_with_chords({chord});
        REQUIRE(song.sequence[0].has_activation());
        REQUIRE(fill_start_tick(chord, *song.sequence[0].activation_length) == line);
        line_ms = song.timing().timecode(line).ms();
        deadlines.push_back(activation_fill_deadline_ms(
            song.timing(), chord, *song.sequence[0].activation_length, FillDeadlineRule::Ch10));
    }
    REQUIRE(deadlines.size() == 2);
    CHECK(deadlines[0] == doctest::Approx(deadlines[1]));
    CHECK(deadlines[0] < line_ms);
}

TEST_CASE("generated fills: the nearer chord wins, and a tie goes to the later one") {
    const int64_t line = line_tick(1);
    const int64_t after = 10;
    // The chord after the line is nearer.
    CHECK(fills_of(song_with_chords({line - 2 * after, line + after})) ==
          Fills{{line, line + after}});
    // Both chords are as far from the line: the later one takes the fill.
    CHECK(fills_of(song_with_chords({line - after, line + after})) == Fills{{line, line + after}});
    // The chord before the line is nearer.
    CHECK(fills_of(song_with_chords({line - after / 2, line + after})) ==
          Fills{{line, line - after / 2}});
}

TEST_CASE("generated fills: after a fill the next candidate is a cooldown of lines on") {
    check_chord_on_every_line(0, 12);
}

TEST_CASE("generated fills: the first measure line is never a candidate") {
    CHECK(fills_of(song_with_chords({line_tick(0)})).empty());
}

TEST_CASE("generated fills: a first chord in measure 3 still counts lines from the second") {
    check_chord_on_every_line(3, 12);
}

TEST_CASE("generated fills: a 4/4 to 7/8 change places fills on the restarted grid") {
    const int64_t restart = line_at_or_after(kMidMeasureChange, kMidMeasureChange);
    const int64_t cooldown = core::default_rules().fill_cooldown_measures;
    REQUIRE(cooldown > 1);
    // The grid restarts at the meter change: a measure line sits on it.
    CHECK(line_tick(restart, kMidMeasureChange) == kMidMeasureChange);
    const std::vector<int64_t> chords = restart_chords();
    const Song song = song_with_chords(chords, kMidMeasureChange);
    CHECK(fills_of(song) == Fills{{line_tick(restart, kMidMeasureChange), chords[0]},
                                  {line_tick(restart + cooldown, kMidMeasureChange), chords[2]}});
}

TEST_CASE("generated fills: Clone Hero 1.0 mode reads the same restarted grid") {
    const std::vector<int64_t> chords = restart_chords();
    const Song song = song_with_chords(chords, kMidMeasureChange, FillDeadlineRule::Ch10);
    std::vector<int64_t> filled;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation()) filled.push_back(ts.timecode.ticks());
    // The chords nearest the restart line and the line a cooldown on.
    CHECK(filled == std::vector<int64_t>{chords[0], chords[2]});
    // Each fill is sized by the 7/8 measure at its chord, in the same share
    // as the 4/4 reference fill.
    const Song ref = ch10_reference();
    REQUIRE(ref.sequence[0].has_activation());
    for (const SongTimestamp& ts : song.sequence) {
        if (!ts.has_activation()) continue;
        INFO("chord tick " << ts.timecode.ticks());
        CHECK(measure_length_at(song, ts.timecode.ticks()) !=
              measure_length_at(ref, ref.sequence[0].timecode.ticks()));
        CHECK(same_measure_share(song, ts, ref, ref.sequence[0]));
    }
}

TEST_CASE("generated fills: a 1.0 fill whose chord sits on a meter change takes the new meter") {
    // 7/8 from the third 4/4 measure line, with the one chord on it.
    const int64_t change = line_tick(2);
    const Song song = song_with_chords({change}, change, FillDeadlineRule::Ch10);
    REQUIRE(song.sequence[0].has_activation());
    // The chord is in the new span, whose measure differs from the old one.
    CHECK(measure_length_at(song, change) != measure_length_at(song, change - 1));
    const Song ref = ch10_reference();
    REQUIRE(ref.sequence[0].has_activation());
    CHECK(same_measure_share(song, song.sequence[0], ref, ref.sequence[0]));
}

TEST_CASE("generated fills: a chart with its own fills gets none generated") {
    const std::vector<int64_t> chords = {line_tick(1), line_tick(2)};
    Song song = song_on_grid(chords);
    // A written fill, one beat long, on the second chord.
    const int64_t written_length = kRes;
    song.sequence[1].activation_length = written_length;
    song.check_activations(core::default_rules(), kTestFillRule);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == written_length);
    CHECK_FALSE(song.generated_fills.has_value());
    CHECK(std::find(song.features.begin(), song.features.end(), "Auto-Generated Fills") ==
          song.features.end());
}

// ---- the corpus -----------------------------------------------------------

namespace {

// Checks one song's generated 1.1 fills against the placement rule. Each fill
// starts on a measure line from the second on; its chord is the one nearest
// that line (a tie going to the later chord) and within the reach; the next
// fill sits at least core::Rules::fill_cooldown_measures lines on; and no
// line the cooldown leaves open with a chord in reach is passed over.
void check_ch11_placement(const Song& song, const core::Rules& rules) {
    if (song.sequence.empty()) return;
    const MeasureIndex& grid = song.timing().measure_index();
    const int64_t reach = ch11_reach(song.tick_resolution(), rules);
    const int64_t cooldown = rules.fill_cooldown_measures;
    std::vector<int64_t> ticks;
    for (const SongTimestamp& ts : song.sequence) ticks.push_back(ts.timecode.ticks());

    // The chord nearest a tick: the first at or after it, unless the one
    // before is strictly nearer.
    auto nearest = [&ticks](int64_t at) -> int64_t {
        const auto after = std::lower_bound(ticks.begin(), ticks.end(), at);
        if (after == ticks.begin()) return *after;
        const int64_t before = *(after - 1);
        if (after == ticks.end() || at - before < *after - at) return before;
        return *after;
    };
    // Every line from `from` up to, not including, `to` has no chord in reach.
    auto check_open_lines = [&](int64_t from, int64_t to) {
        for (int64_t n = from; n < to; ++n) {
            const int64_t line = grid.measure_line_tick(n);
            INFO("measure line " << n);
            CHECK(std::llabs(nearest(line) - line) > reach);
        }
    };

    int64_t open_from = 1;  // the second measure line
    for (const SongTimestamp& ts : song.sequence) {
        if (!ts.has_activation()) continue;
        const int64_t chord = ts.timecode.ticks();
        const int64_t start = fill_start_tick(chord, *ts.activation_length);
        const int64_t n = grid.at(start).measure;
        INFO("fill on chord " << chord);
        CHECK(grid.measure_line_tick(n) == start);
        CHECK(n >= open_from);
        CHECK(nearest(start) == chord);
        CHECK(std::llabs(chord - start) <= reach);
        check_open_lines(open_from, n);
        open_from = n + cooldown;
    }
    check_open_lines(open_from, grid.first_measure_line_at_or_after(ticks.back() + reach + 1));
}

}  // namespace

TEST_CASE("generated fills: every corpus chart's generated fills follow the 1.1 placement rule") {
    const core::Rules& rules = core::default_rules();
    int generated = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        INFO("chart " << path);
        const bool listed = std::find(song.features.begin(), song.features.end(),
                                      "Auto-Generated Fills") != song.features.end();
        CHECK(listed == song.generated_fills.has_value());
        if (!song.generated_fills.has_value()) continue;
        ++generated;
        CHECK((*song.generated_fills == kTestFillRule));
        check_ch11_placement(song, rules);
    }
    // The corpus holds charts with no fills of their own, so some get fills
    // generated; none found means the corpus or the loader changed.
    CHECK(generated > 0);
}
