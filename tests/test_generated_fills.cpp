// Tests for generated fills (Song::check_activations, core/song.cpp). A chart
// with no fills of its own gets fills placed on the measure lines of Clone
// Hero's grid (MeasureIndex). Each expected tick here is read from that grid,
// and each rule value from core::Rules or search/graph.h, never typed. The
// one exception is the 1.0 measure-line cases: their ticks are typed, each
// worked by hand from the 1.0 game's walk in the case's comment.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <optional>
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
bool one_measure_share(const Song& a, const SongTimestamp& fa, const Song& b,
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
        CHECK(one_measure_share(old, ts, ref, ref.sequence[0]));
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
    CHECK(*song.sequence[0].activation_length == 0);
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
        CHECK(one_measure_share(song, ts, ref, ref.sequence[0]));
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
    CHECK(one_measure_share(song, song.sequence[0], ref, ref.sequence[0]));
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
    CHECK(std::find(song.features.begin(), song.features.end(), kGeneratedFillsFeature) ==
          song.features.end());
}

TEST_CASE("chords_around: the nearer chord, a tie going to the later one, as the game picks") {
    // The game's pick (docs/clone-hero/code/4045/fill_generator.cs, section
    // F4): `after` is the first chord after the tick, `before` the last at or
    // before it, and line 162 returns `before` only when
    // after - tick > tick - before. Otherwise it returns `after`.
    const int64_t tick = 100;
    {
        // Chords at 90 and 110: after - tick = 10, tick - before = 10.
        // 10 > 10 is false, so the game returns the later chord, 110.
        const Song song = song_on_grid({90, 110});
        const ChordsAround around = chords_around(song.sequence, tick);
        CHECK(around.before == std::optional<size_t>{0});
        CHECK(around.after == std::optional<size_t>{1});
        CHECK(around.pick == std::optional<size_t>{1});
    }
    {
        // Chords at 95 and 110: after - tick = 10, tick - before = 5.
        // 10 > 5 is true, so the game returns the earlier chord, 95.
        const Song song = song_on_grid({95, 110});
        const ChordsAround around = chords_around(song.sequence, tick);
        CHECK(around.pick == std::optional<size_t>{0});
    }
    {
        // Chords at 90, 100 and 110: the chord at 100 is the last at or
        // before the tick, so before = 100 and after = 110. after - tick = 10,
        // tick - before = 0, and 10 > 0 is true: the game returns the chord
        // on the tick itself.
        const Song song = song_on_grid({90, tick, 110});
        const ChordsAround around = chords_around(song.sequence, tick);
        CHECK(around.before == std::optional<size_t>{1});
        CHECK(around.after == std::optional<size_t>{2});
        CHECK(around.pick == std::optional<size_t>{1});
    }
}

// ---- Clone Hero 1.0's measure lines ----------------------------------------
//
// Each expected list below is worked from the game's walk,
// docs/clone-hero/code/4045/fill_generator.cs section F5 (lines 199 to 257):
// from tick 0, while tick <= last tick (line 207), read the meter at the tick;
// on a new meter restart both counters (lines 211 to 216); wrap the beat in
// the measure at the numerator (lines 217 to 220); a beat whose count is 0
// is stored as a measure (line 240), except in a meter with 1 on top after
// its first beat (line 223); then step resolution x 4 / bottom number ticks
// (line 253). Only a stored measure reaches the list (F11, lines 661 to 669).

namespace {

// The resolution the plan's 1.0 line cases use.
constexpr int64_t kCh10Res = 192;

struct Meter {
    int64_t tick;
    int numerator;
    int denominator;
};

// A song at kCh10Res and 120 BPM, 4/4 until the first of `meters`.
Song song_with_meters(const std::vector<Meter>& meters) {
    Song song(kCh10Res);
    song.bpm_changes[0] = 120.0;
    for (const Meter& m : meters) apply_timesig(song, m.tick, m.numerator, m.denominator);
    song.build_timing();
    return song;
}

std::vector<int64_t> ch10_lines(const Song& song, int64_t last_tick) {
    return ch10_measure_lines(song.timing().measure_index(), last_tick);
}

}  // namespace

TEST_CASE("1.0 measure lines: in 4/4 a line every four beats, none past the last tick") {
    // Inputs: resolution 192, 4/4 throughout, last tick 3000.
    // The step is 192 x 4 / 4 = 192 ticks. The beat count runs 0, 1, 2, 3 and
    // wraps at 4, so every fourth beat is a measure: 0, 768, 1536, 2304. The
    // next would be 3072, and 3072 <= 3000 is false, so the walk stops.
    CHECK(ch10_lines(song_with_meters({}), 3000) == std::vector<int64_t>{0, 768, 1536, 2304});
}

TEST_CASE("1.0 measure lines: a meter with 1 on top keeps only its first beat") {
    // Inputs: resolution 192, 4/4, then 1/4 from tick 1536; last tick 2400.
    // 4/4 gives measures at 0 and 768. At 1536 the meter changes, so both
    // counters restart at 0 and the beat is a measure (beats since the
    // change is 0). The step stays 192 x 4 / 4 = 192. At 1728, 1920, 2112 and
    // 2304 the beat count wraps to 0 again, but the numerator is 1 and the
    // beats since the change are above 0, so line 223 stores each as a beat,
    // not a measure. 2496 is past 2400.
    CHECK(ch10_lines(song_with_meters({{1536, 1, 4}}), 2400) ==
          std::vector<int64_t>{0, 768, 1536});
}

TEST_CASE("1.0 measure lines: a mid-measure meter change puts a line on the change") {
    // Inputs: resolution 192, 4/4, then 7/8 from tick 1152; last tick 2600.
    // 4/4 steps 192: measures at 0 and 768, and 1152 is the third beat of
    // the second measure. At 1152 the meter changes, so the counters restart
    // and 1152 is a measure. 7/8 steps 192 x 4 / 8 = 96, and the beat count
    // wraps at 7, so the next measures are 1152 + 7 x 96 = 1824 and
    // 1824 + 672 = 2496. The next, 3168, is past 2600.
    CHECK(ch10_lines(song_with_meters({{1152, 7, 8}}), 2600) ==
          std::vector<int64_t>{0, 768, 1152, 1824, 2496});
}

TEST_CASE("1.0 measure lines: the last tick is a line when a measure lands on it") {
    // Inputs: resolution 192, 4/4, last tick 1536. The walk's test is
    // tick <= last tick (line 207), so the measure at 1536 is walked.
    CHECK(ch10_lines(song_with_meters({}), 1536) == std::vector<int64_t>{0, 768, 1536});
    // One tick less and it is not.
    CHECK(ch10_lines(song_with_meters({}), 1535) == std::vector<int64_t>{0, 768});
}

TEST_CASE("1.0 measure lines: the walk ends one beat past the chart's last note end") {
    // The game's last tick is the largest note tick plus sustain over every
    // track and difficulty, plus the resolution (F10, fill_generator.cs lines
    // 626 to 658). Inputs: resolution 192, last note end 2808: 2808 + 192 = 3000.
    Song song = song_with_meters({});
    song.last_note_end_tick = 2808;
    CHECK(ch10_last_line_tick(song) == 3000);
    // A chart with no note: the game's largest end starts at 0, so 0 + 192.
    song.last_note_end_tick.reset();
    CHECK(ch10_last_line_tick(song) == kCh10Res);
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
                                      kGeneratedFillsFeature) != song.features.end();
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
