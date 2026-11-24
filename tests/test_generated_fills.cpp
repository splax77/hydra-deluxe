// Tests for generated fills (Song::check_activations, core/song.cpp). A chart
// with no fills of its own gets fills placed on the measure lines of Clone
// Hero's grid (MeasureIndex). Each expected tick here is read from that grid,
// and each rule value from core::Rules or search/graph.h, never typed. The
// exceptions are kLinesAfterFill below, and the 1.0 cases, the measure lines
// and the generator: their ticks are typed, each worked by hand from the 1.0
// game's code in the case's comment.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <map>
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

// How many measure lines on the next candidate sits after a generated fill.
// It is fixed in both games, so no header exposes it: 1.0's `i += 4`
// (docs/clone-hero/code/4045/fill_generator.cs line 42) and 1.1's
// `add ebx, 4` (FL-17 in docs/clone-hero/04-fills.md). core/song.cpp's
// kLinesAfterFill is the generators' copy.
constexpr int64_t kLinesAfterFill = 4;

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

// A chord on every measure line from `first` to `last` (4/4). The 1.1 fills
// sit on the first such line from the second on, then every kLinesAfterFill
// lines on.
void check_chord_on_every_line(int64_t first, int64_t last) {
    const int64_t cooldown = kLinesAfterFill;
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
    const int64_t cooldown = kLinesAfterFill;
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

TEST_CASE("generated fills: Clone Hero 1.0 mode starts fills at the midpoint, 1.1 starts them on the line") {
    const int64_t cooldown = kLinesAfterFill;
    // One chord 10 ticks after each of two lines a cooldown apart.
    const std::vector<int64_t> chords = {line_tick(1) + 10, line_tick(1 + cooldown) + 10};
    CHECK(fills_of(song_with_chords(chords, 0, FillDeadlineRule::Ch11)) ==
          Fills{{line_tick(1), chords[0]}, {line_tick(1 + cooldown), chords[1]}});
    // 1.0, worked from fill_generator.cs F1. Inputs: resolution 480, 120 BPM,
    // 4/4, so a second is 960 ticks and line n is tick 1920 n at 2 n s;
    // chords at 1930 and 9610; the last note ends at 9610, so the lines run
    // to 9610 + 480 = 10090 (F10): lines 0 to 5. Line 1 (2 s) takes 1930
    // (10 ticks, about 0.0104 s, within 0.25 s, line 55); its fill starts at
    // the midpoint of 0 s and 2 s, 1 s, tick 960 (lines 44 and 65). The next
    // candidate is line 1 + 4 = 5 (line 42): 9610 is nearer line 5 (9600)
    // than 1930 is, so it takes the fill from the midpoint of 8 s and 10 s,
    // 9 s, tick 8640.
    Song old = song_on_grid(chords);
    old.last_note_end_tick = chords.back();
    old.check_activations(core::default_rules(), FillDeadlineRule::Ch10);
    CHECK(fills_of(old) == Fills{{960, 1930}, {8640, 9610}});
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
    const int64_t cooldown = kLinesAfterFill;
    REQUIRE(cooldown > 1);
    // The grid restarts at the meter change: a measure line sits on it.
    CHECK(line_tick(restart, kMidMeasureChange) == kMidMeasureChange);
    const std::vector<int64_t> chords = restart_chords();
    const Song song = song_with_chords(chords, kMidMeasureChange);
    CHECK(fills_of(song) == Fills{{line_tick(restart, kMidMeasureChange), chords[0]},
                                  {line_tick(restart + cooldown, kMidMeasureChange), chords[2]}});
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

// ---- Clone Hero 1.0 at another song speed ---------------------------------
//
// FL-49 and FL-50 on docs/clone-hero/04-fills.md: the 1.0 measure lines and
// Star Power sizes stay on the file's ticks at every speed, and only the
// walk's last beat is counted in the speed-scaled resolution (F10,
// fill_generator.cs line 638). These cases load one corpus chart at 100%, 50%
// and 200% through ChartReading::speed_percent and compare the answers.

namespace {

constexpr int kHalfSpeed = 50;
constexpr int kDoubleSpeed = 200;

Song ch10_at_speed(const std::string& path, int percent) {
    ChartReading reading;
    reading.speed_percent = percent;
    return load_songpath(path, reading, core::default_rules(), FillDeadlineRule::Ch10);
}

// The first corpus chart that gets 1.0 fills generated and has a last note
// end, or an empty string when none does.
const std::string& ch10_generated_chart() {
    static const std::string path = [] {
        for (const std::string& p : corpus::chart_paths()) {
            const Song song = ch10_at_speed(p, kDefaultSpeedPercent);
            if (song.generated_fills == FillDeadlineRule::Ch10 && song.last_note_end_tick.has_value())
                return p;
        }
        return std::string{};
    }();
    return path;
}

}  // namespace

TEST_CASE("1.0 measure lines: the walk's last beat is the file's resolution times the song speed") {
    const std::string& path = ch10_generated_chart();
    REQUIRE_FALSE(path.empty());
    CAPTURE(path);
    const Song normal = ch10_at_speed(path, kDefaultSpeedPercent);
    const int64_t end = *normal.last_note_end_tick;
    // At 100% the beat is the file's own resolution.
    const int64_t normal_beat = ch10_last_line_tick(normal) - end;
    CHECK(normal_beat == normal.tick_resolution());
    // Half of it must be a whole tick for the 50% case to be exact.
    REQUIRE(normal.tick_resolution() % 2 == 0);
    for (int percent : {kHalfSpeed, kDoubleSpeed}) {
        CAPTURE(percent);
        const Song song = ch10_at_speed(path, percent);
        REQUIRE(song.generated_fills == FillDeadlineRule::Ch10);
        REQUIRE(song.last_note_end_tick == normal.last_note_end_tick);
        const int64_t beat = ch10_last_line_tick(song) - end;
        // Half the file's beat at 50%, twice it at 200%.
        CHECK(beat * kDefaultSpeedPercent == normal_beat * percent);
    }
}

TEST_CASE("1.0 measure lines: a beat that is not a whole tick is cut to the tick below") {
    // Resolution 192 at 110%: the game's timing resolution is 211.2, and its
    // `(uint)` cast (fill_generator.cs line 638) drops the fraction.
    Song song(kCh10Res);
    song.bpm_changes[0] = 120.0;
    song.set_speed_percent(110);
    song.build_timing();
    song.last_note_end_tick = 2808;
    const double working = song.timing().working_resolution();
    const int64_t beat = ch10_last_line_tick(song) - 2808;
    REQUIRE(working != static_cast<double>(static_cast<int64_t>(working)));
    CHECK(static_cast<double>(beat) < working);
    CHECK(working < static_cast<double>(beat + 1));
}

TEST_CASE("1.0 measure lines: the lines sit on the file's ticks at 50% and 200%, each at its played second") {
    // FL-49: the walk steps by the file's resolution, so a speed moves only
    // where the walk ends, and each line's second is its played second.
    const std::string& path = ch10_generated_chart();
    REQUIRE_FALSE(path.empty());
    CAPTURE(path);
    const Song normal = ch10_at_speed(path, kDefaultSpeedPercent);
    for (int percent : {kHalfSpeed, kDoubleSpeed}) {
        CAPTURE(percent);
        const Song song = ch10_at_speed(path, percent);
        const int64_t last = ch10_last_line_tick(song);
        const std::vector<int64_t> lines = ch10_lines(song, last);
        REQUIRE_FALSE(lines.empty());
        // The same ticks the 100% grid gives up to this speed's last tick.
        CHECK(lines == ch10_lines(normal, last));
        for (int64_t line : lines) {
            CAPTURE(line);
            CHECK(song.timing().timecode(line).ms() * song.timing().speed_ratio() ==
                  doctest::Approx(normal.timing().timecode(line).ms()));
        }
    }
}

TEST_CASE("1.0 Star Power: a bar lasts the same measures of file ticks at 50% and 200%") {
    // FL-50: the 1.0 drain counts the file's own ticks, so a bar of Star
    // Power ends on the same tick at every speed, and only its played time
    // changes. Hydra's length of a bar is SongTiming::plusmeasure over
    // sp_bars_to_measures(1).
    const std::string& path = ch10_generated_chart();
    REQUIRE_FALSE(path.empty());
    CAPTURE(path);
    const Song normal = ch10_at_speed(path, kDefaultSpeedPercent);
    const int64_t one_bar = sp_bars_to_measures(1);
    for (int percent : {kHalfSpeed, kDoubleSpeed}) {
        CAPTURE(percent);
        const Song song = ch10_at_speed(path, percent);
        REQUIRE(song.sequence.size() == normal.sequence.size());
        for (const SongTimestamp& ts : song.sequence) {
            const int64_t tick = ts.timecode.ticks();
            CAPTURE(tick);
            const Timecode end = song.timing().plusmeasure(ts.timecode, one_bar);
            const Timecode normal_end = normal.timing().plusmeasure(normal.timecode(tick), one_bar);
            CHECK(end.ticks() == normal_end.ticks());
            CHECK(end.ms() * song.timing().speed_ratio() == doctest::Approx(normal_end.ms()));
        }
    }
}

// ---- Clone Hero 1.0's generator -------------------------------------------
//
// Each expected fill below is worked by hand from the game's loop, F1 of
// docs/clone-hero/code/4045/fill_generator.cs (lines 31 to 79), with its
// clock (F9, lines 534 to 623) and its chord search (F4, lines 145 to 196).
// The loop: i starts at 1 and steps 4 after a fill (line 42), 1 after a
// reject (`i -= 3`, line 57). Line i's second is its tick's second; the fill
// starts at the tick of the midpoint second of lines i-1 and i (lines 44 and
// 65). The chord is the nearest to the line's tick, the search for the chord
// after starting at the last chord used; a line no chord follows takes
// chord 0 (lines 172 to 183). A chord more than 0.25 s from the line is
// rejected; exactly 0.25 s is kept (`num5 > 0.25f`, line 55). Hydra's length
// is the chord tick minus the start tick. Unless a case says otherwise, the
// chart is resolution 192, 4/4, at 120 BPM, so a second is 384 ticks, line n
// is tick 768 n at 2 n s, and the last note ends on the last chord, so the
// lines run to the last chord + 192 (F10).

namespace {

// A song at kCh10Res with tempos `bpms` (tick -> BPM), 4/4, one red note at
// each tick in `chords`, its last note ending at `last_end` (the last chord
// when not given), and its fills generated by Clone Hero 1.0's rule under
// `rules`.
Song ch10_song(const std::map<int64_t, double>& bpms, const std::vector<int64_t>& chords,
               std::optional<int64_t> last_end = std::nullopt,
               const core::Rules& rules = core::default_rules()) {
    Song song(kCh10Res);
    song.bpm_changes = bpms;
    song.build_timing();
    for (int64_t tick : chords) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    song.last_note_end_tick = last_end.has_value() ? last_end : std::optional<int64_t>{chords.back()};
    song.check_activations(rules, FillDeadlineRule::Ch10);
    return song;
}

const std::map<int64_t, double> k120Bpm = {{0, 120.0}};

}  // namespace

TEST_CASE("1.0 generator: the first candidate is the second line, and the fill starts at the midpoint") {
    // A chord on tick 0, the last note ending at 768: lines run to 960, so
    // they are 0 and 768. Line 1 (2 s) has no chord after it and takes
    // chord 0, at 0 s: 2 s away, more than 0.25 s, so no fill. Line 0 is
    // never tried.
    CHECK(fills_of(ch10_song(k120Bpm, {0}, 768)).empty());
    // A chord on tick 768: line 1 takes it, 0 s away. The midpoint of 0 s and
    // 2 s is 1 s, tick 384, so the length is 768 - 384 = 384.
    const Song song = ch10_song(k120Bpm, {768});
    CHECK(fills_of(song) == Fills{{384, 768}});
    REQUIRE(song.sequence[0].has_activation());
    CHECK(*song.sequence[0].activation_length == 384);
}

TEST_CASE("1.0 generator: after a fill the next candidate is four lines on") {
    // A chord on every line from 0 to 12 (ticks 0 to 9216): lines run to
    // 9408, so 0 to 12. Line 1 takes the chord on it (nearer than 1536):
    // start 1 s, tick 384. Then line 5 (3840, 10 s): start at 9 s, tick 3456.
    // Then line 9 (6912, 18 s): start at 17 s, tick 6528. Line 13 is past
    // the list.
    std::vector<int64_t> chords;
    for (int64_t n = 0; n <= 12; ++n) chords.push_back(768 * n);
    CHECK(fills_of(ch10_song(k120Bpm, chords)) ==
          Fills{{384, 768}, {3456, 3840}, {6528, 6912}});
}

TEST_CASE("1.0 generator: the step is the game's 4, whatever the rules file says") {
    // A chord on every line from 0 to 8: lines 0 to 8. Fills on lines 1 and
    // 5, the game's `i = 1; i += 4` (line 42), though these rules ask for a
    // reach of two beats.
    core::Rules rules = core::default_rules();
    rules.fill_max_distance_beats = 2.0;
    std::vector<int64_t> chords;
    for (int64_t n = 0; n <= 8; ++n) chords.push_back(768 * n);
    CHECK(fills_of(ch10_song(k120Bpm, chords, std::nullopt, rules)) ==
          Fills{{384, 768}, {3456, 3840}});
}

TEST_CASE("1.0 generator: a rejected line tries the next one") {
    // Chords on lines 2, 6 and 8 (1536, 4608, 6144): lines 0 to 8. Line 1
    // (2 s) takes 1536 (4 s), 2 s away: rejected, so line 2 is next. Line 2
    // takes 1536: start at the midpoint of 2 s and 4 s, 3 s, tick 1152. Line
    // 6 (4608, 12 s) takes 4608 (nearer than 6144): start at 11 s, tick 4224.
    // Line 10 is past the list.
    CHECK(fills_of(ch10_song(k120Bpm, {1536, 4608, 6144})) ==
          Fills{{1152, 1536}, {4224, 4608}});
}

TEST_CASE("1.0 generator: a line no chord follows takes chord 0") {
    // F4: the search for the chord after returns 0 when there is none (line
    // 182), so the search for the chord before starts at index 0 and finds
    // chord 0 (line 188 with F12). Both are chord 0, so chord 0 comes back.
    {
        // Chords on lines 2 and 6 (1536, 4608): line 2 takes 1536 as in the
        // case above. Line 6 sits on the last chord, so no chord follows it:
        // it takes chord 0 (1536, 4 s), 8 s from 12 s, and is rejected. Lines
        // run to 4800, so there is no line 7.
        CHECK(fills_of(ch10_song(k120Bpm, {1536, 4608})) == Fills{{1152, 1536}});
    }
    {
        // Chords at 1000 and 1500: lines run to 1692, so 0, 768, 1536. Line 1
        // (2 s) takes 1000 (about 2.604 s): rejected. Line 2 (4 s) is
        // 36 ticks past the last chord, 1500 (3.90625 s, 0.09375 s away), but
        // takes chord 0, 1000, about 1.396 s away: rejected. No fill.
        CHECK(fills_of(ch10_song(k120Bpm, {1000, 1500})).empty());
    }
    {
        // Chords at 1500 and 1510: line 2 (4 s) takes chord 0, 1500
        // (3.90625 s), 0.09375 s away, so chord 0 takes the fill, not the
        // nearer 1510. Start at 3 s, tick 1152: length 348. Line 1 (2 s) took
        // 1500 too and was rejected (1.90625 s).
        CHECK(fills_of(ch10_song(k120Bpm, {1500, 1510})) == Fills{{1152, 1500}});
    }
}

TEST_CASE("1.0 generator: the reach is 0.25 s of real time") {
    // At 60 BPM a second is 192 ticks, and line 1 is tick 768 at 4 s.
    const std::map<int64_t, double> bpm60 = {{0, 60.0}};
    // 47 ticks after line 1, tick 815: 815 / 192 = 4.2448 s, 0.2448 s away.
    // Kept. Start at the midpoint of 0 s and 4 s, 2 s, tick 384.
    CHECK(fills_of(ch10_song(bpm60, {815})) == Fills{{384, 815}});
    // 49 ticks after, tick 817: 4.2552 s, 0.2552 s away. Rejected, though
    // half a beat (96 ticks) would take it.
    CHECK(fills_of(ch10_song(bpm60, {817})).empty());
}

TEST_CASE("1.0 generator: a chord exactly 0.25 s from its line is kept") {
    // 120 BPM: 96 ticks is 0.25 s. Tick 864 is 864 / 384 = 2.25 s, and 768 is
    // 2 s; both are exact in a float, so the difference is exactly 0.25f, and
    // 0.25f > 0.25f is false (line 55). Start tick 384, length 480.
    CHECK(fills_of(ch10_song(k120Bpm, {864})) == Fills{{384, 864}});
    // The same before the line: tick 672 is 1.75 s, exactly 0.25 s away.
    CHECK(fills_of(ch10_song(k120Bpm, {672}, 768)) == Fills{{384, 672}});
    // One tick further, 865 (about 2.2526 s), is rejected.
    CHECK(fills_of(ch10_song(k120Bpm, {865})).empty());
}

TEST_CASE("1.0 generator: the midpoint is in seconds, across a tempo change") {
    // 120 BPM to tick 384, then 60 BPM. Line 1 (768) is at 384 / 384 = 1 s
    // plus 384 / 192 = 2 s, so 3 s. The midpoint of 0 s and 3 s is 1.5 s.
    // The last tempo starting before 1.5 s is the 60 BPM one at 1 s (F9,
    // line 610), so the start tick is 384 + 0.5 x 192 = 480 (line 622). A
    // chord on 768 has length 768 - 480 = 288, not the 384 of a midpoint in
    // ticks.
    const Song song = ch10_song({{0, 120.0}, {384, 60.0}}, {768});
    CHECK(fills_of(song) == Fills{{480, 768}});
    REQUIRE(song.sequence[0].has_activation());
    CHECK(*song.sequence[0].activation_length == 288);
}

TEST_CASE("1.0 generator: in a short measure the fill can start after its chord") {
    // 600 BPM: a second is 1920 ticks, a beat 0.1 s and a measure 0.4 s, so
    // line 1 is tick 768 at 0.4 s and the midpoint 0.2 s, tick 384. A chord
    // 432 ticks before line 1, tick 336, is at 0.175 s: 0.225 s away, kept.
    // The last note ends at 768, so the lines are 0 and 768. The length is
    // 336 - 384 = -48.
    const Song song = ch10_song({{0, 600.0}}, {336}, 768);
    CHECK(fills_of(song) == Fills{{384, 336}});
    REQUIRE(song.sequence[0].has_activation());
    CHECK(*song.sequence[0].activation_length == -48);
}

TEST_CASE("1.0 generator: a mid-measure meter change puts the midpoint before the restart line") {
    // 4/4, then 7/8 from tick 1152; chords at 1152 and 2600. Lines run to
    // 2792: 0, 768, 1152, 1824, 2496 (the third 1.0 measure-line case). Line
    // 1 (768, 2 s) takes 1152 (3 s): rejected. Line 2 (1152, 3 s) takes 1152
    // (nearer than 2600): start at the midpoint of 2 s and 3 s, 2.5 s, tick
    // 960, length 192. Line 6 is past the list.
    Song song(kCh10Res);
    song.bpm_changes[0] = 120.0;
    apply_timesig(song, 1152, 7, 8);
    song.build_timing();
    for (int64_t tick : {int64_t{1152}, int64_t{2600}}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    song.last_note_end_tick = 2600;
    song.check_activations(core::default_rules(), FillDeadlineRule::Ch10);
    CHECK(fills_of(song) == Fills{{960, 1152}});
}

TEST_CASE("1.0 generator: the longest tick at a corpus fill's start, beside the timing step") {
    // A log line, no check: a 1.0 fill's start is a midpoint second rounded
    // to a tick, so it can sit up to half this far from the exact second.
    double longest_tick_ms = 0.0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song song =
            load_songpath(path, ChartReading{}, core::default_rules(), FillDeadlineRule::Ch10);
        if (song.generated_fills != FillDeadlineRule::Ch10) continue;
        for (const SongTimestamp& ts : song.sequence) {
            if (!ts.has_activation()) continue;
            const int64_t start = fill_start_tick(ts.timecode.ticks(), *ts.activation_length);
            longest_tick_ms =
                (std::max)(longest_tick_ms, 1000.0 / song.timing().ms_index().tps_at(start));
        }
    }
    MESSAGE("longest tick at a 1.0 generated fill's start: " << longest_tick_ms
                                                             << " ms; kTimingStepMs " << kTimingStepMs);
}

// ---- the corpus -----------------------------------------------------------

namespace {

// Checks one song's generated 1.1 fills against the placement rule. Each fill
// starts on a measure line from the second on; its chord is the one nearest
// that line (a tie going to the later chord) and within the reach; the next
// fill sits at least kLinesAfterFill lines on; and no
// line the cooldown leaves open with a chord in reach is passed over.
void check_ch11_placement(const Song& song, const core::Rules& rules) {
    if (song.sequence.empty()) return;
    const MeasureIndex& grid = song.timing().measure_index();
    const int64_t reach = ch11_reach(song.tick_resolution(), rules);
    const int64_t cooldown = kLinesAfterFill;
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
