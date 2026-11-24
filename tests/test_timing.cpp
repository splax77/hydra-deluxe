// Invariant + unit tests for core/timing.
//
// The corpus half converts every chart timestamp through the chart's real
// tempo/meter maps and asserts the conversions are monotone and consistent.
// The unit half pins the edge cases the corpus may not contain.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/preview_view.h"  // PreviewTimeSig
#include "core/note_window.h"
#include "core/squeeze_rating.h"
#include "core/timing.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"

TEST_CASE("timing: corpus conversions are monotone and consistent") {
    // A walk over no charts would pass every check below, so it fails at once.
    const std::vector<std::string>& paths = corpus::chart_paths();
    REQUIRE_FALSE(paths.empty());

    // Every chart is walked. A chart that fails to load throws out of
    // corpus::song; one with no notes at this reading has no timestamps, so
    // its walk below checks nothing and needs no skip.
    size_t charts = 0, samples = 0, off_line_charts = 0;
    for (const std::string& path : paths) {
        const hydra::Song& song = corpus::song(path, hydra::ChartReading{});
        ++charts;

        // The grid and the continuous count tell the same measure only while
        // every meter change sits on a grid measure line; a mid-measure
        // change makes them part on purpose (D115).
        // A change on a measure line starts a count span on a whole number.
        const hydra::MeasureIndex& grid = song.timing().measure_index();
        const hydra::MeasureCount& count = song.timing().measure_count();
        bool on_lines = true;
        for (int i = 0; i < count.count(); ++i)
            if (std::floor(count.elapsed_at(i)) != count.elapsed_at(i)) on_lines = false;
        if (!on_lines) ++off_line_charts;

        bool ms_ok = true, md_ok = true, mbt_ok = true;
        double prev_ms = -1e18, prev_md = -1e18;
        for (const hydra::SongTimestamp& ts : song.sequence) {
            const hydra::Timecode& tc = ts.timecode;
            ++samples;

            // Later ticks never map to earlier times or fewer measures.
            if (tc.ms() < prev_ms) ms_ok = false;
            prev_ms = tc.ms();
            const double md = tc.measures_decimal();
            if (md < prev_md) md_ok = false;
            prev_md = md;

            // The grid's position sits inside its span's measure and beat.
            const int64_t* mbt = tc.measure_beats_ticks();
            const int s = grid.span_at(tc.ticks());
            if (mbt[0] < 0 || mbt[1] < 0 || mbt[2] < 0 || mbt[1] >= grid.numerator_at(s) ||
                mbt[2] >= grid.step_at(s))
                mbt_ok = false;
            // measures_decimal = whole measures + fraction, all 0-based.
            if (on_lines && (static_cast<double>(mbt[0]) > md ||
                             md > static_cast<double>(mbt[0] + 1)))
                mbt_ok = false;
        }
        CHECK_MESSAGE(ms_ok, path << ": ms not monotone");
        CHECK_MESSAGE(md_ok, path << ": measures_decimal not monotone");
        CHECK_MESSAGE(mbt_ok, path << ": measure_beats_ticks inconsistent");
    }

    // With no timestamp at all, the three checks above checked nothing.
    CHECK(samples > 0);
    MESSAGE("timing invariants: " << charts << " charts, " << samples << " samples, "
                                  << off_line_charts << " with a mid-measure meter change");
}

TEST_CASE("timing: the combo multiplier steps up by one at evenly spaced combos and then stays") {
    using hydra::to_multiplier;

    // A note hit with no combo scores its plain value: a factor of one.
    const int no_bonus = 1;
    CHECK(to_multiplier(0) == no_bonus);

    // The top is what the longest combo reads; the multiplier must climb to it.
    const int top = to_multiplier((std::numeric_limits<int>::max)());
    REQUIRE(top > to_multiplier(0));

    // The steps come from the owner: walk the combo up until the multiplier
    // reaches its top, and note each combo where it changes. Each change is a
    // rise of exactly one, and no run is longer than the first, so a walk that
    // can no longer reach the top fails at once instead of running on.
    std::vector<int> steps;
    for (int combo = 1; to_multiplier(combo - 1) < top; ++combo) {
        CAPTURE(combo);
        const int rise = to_multiplier(combo) - to_multiplier(combo - 1);
        REQUIRE(rise >= 0);
        REQUIRE(rise <= 1);
        if (rise == 1) steps.push_back(combo);
        REQUIRE((steps.empty() || combo - steps.back() <= steps.front()));
    }

    // Each rise is one, so the top is the start plus the number of steps.
    REQUIRE_FALSE(steps.empty());
    CHECK(top == to_multiplier(0) + static_cast<int>(steps.size()));
    // Every run of one multiplier is as long as the first.
    for (size_t i = 0; i < steps.size(); ++i) {
        CAPTURE(i);
        CHECK(steps[i] == static_cast<int>(i + 1) * steps.front());
    }
    // Past the last step the multiplier stays at the top.
    for (int past : {steps.back(), steps.back() + steps.front(), 2 * steps.back()}) {
        CAPTURE(past);
        CHECK(to_multiplier(past) == top);
    }
}

TEST_CASE("timing: a map without a tick-0 entry throws") {
    std::map<int64_t, double> bpm{{480, 120.0}};
    CHECK_THROWS_AS(hydra::MsIndex(bpm, 480), std::out_of_range);

    std::map<int64_t, int64_t> tpm{{480, 1920}};
    CHECK_THROWS_AS(hydra::MeasureCount{tpm}, std::out_of_range);

    std::map<int64_t, std::pair<int, int>> sigs{{480, {4, 4}}};
    CHECK_THROWS_AS(hydra::MeasureIndex(sigs, 480), std::out_of_range);
}

TEST_CASE("timing: a signature that makes a beat shorter than one tick is refused (D115)") {
    // 480 ticks a quarter note cannot be cut into 4096ths: the beat would be 0
    // ticks, and the grid would never advance.
    std::map<int64_t, std::pair<int, int>> sigs{{0, {4, 4}}, {1920, {4, 4096}}};
    CHECK_THROWS_AS(hydra::MeasureIndex(sigs, 480), std::out_of_range);
    CHECK_NOTHROW(hydra::MeasureIndex({{0, {4, 4}}, {1920, {4, 128}}}, 480));
}

TEST_CASE("timing: the time signature map must name the tpm map's ticks") {
    std::map<int64_t, int64_t> tpm{{0, 1920}, {1920, 1440}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    CHECK_THROWS_AS(hydra::SongTiming(480, tpm, bpm, {{0, {4, 4}}}), std::out_of_range);
    CHECK_NOTHROW(hydra::SongTiming(480, tpm, bpm, {{0, {4, 4}}, {1920, {3, 4}}}));
}

TEST_CASE("timing: a tick before the first tempo mark reads at the opening tempo") {
    // At resolution 480 one beat is 480 ticks, and at 120 BPM it lasts 500 ms.
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::MsIndex ms(bpm, 480);
    CHECK(ms.at(0) == doctest::Approx(0.0));
    CHECK(ms.at(480) == doctest::Approx(500.0));
    CHECK(ms.at(-480) == doctest::Approx(-500.0));  // extrapolated backwards
}

// Every grid line up to `last_tick`, as (tick, line) pairs.
static std::vector<std::pair<int64_t, hydra::MeasureIndex::Line>> grid_lines(
    const hydra::MeasureIndex& mi, int64_t last_tick) {
    std::vector<std::pair<int64_t, hydra::MeasureIndex::Line>> out;
    mi.walk(last_tick, [&](int64_t t, hydra::MeasureIndex::Line l) { out.push_back({t, l}); });
    return out;
}

static std::vector<int64_t> measure_lines(const hydra::MeasureIndex& mi, int64_t last_tick) {
    std::vector<int64_t> out;
    for (const auto& [t, l] : grid_lines(mi, last_tick))
        if (l == hydra::MeasureIndex::Line::Measure) out.push_back(t);
    return out;
}

TEST_CASE("timing: the grid restarts at a mid-measure change (D115)") {
    // 4/4 until tick 2880, one and a half measures in, then 7/8. The grid
    // cuts the 4/4 measure short there and restarts.
    std::map<int64_t, std::pair<int, int>> sigs{{0, {4, 4}}, {2880, {7, 8}}};
    hydra::MeasureIndex mi(sigs, 480);
    REQUIRE(mi.count() == 2);
    CHECK(mi.restart_at(1) == 2880);
    CHECK(mi.step_at(1) == 240);
    CHECK(mi.length_at(1) == 1680);
    CHECK(mi.lines_before_at(1) == 2);
    CHECK(mi.span_at(2879) == 0);
    CHECK(mi.span_at(2880) == 1);  // a tick on the restart is in the new span
    CHECK(measure_lines(mi, 4560) == std::vector<int64_t>{0, 1920, 2880, 4560});

    std::map<int64_t, int64_t> tpm{{0, 1920}, {2880, 1680}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(480, tpm, bpm, sigs);
    const int64_t* on = st.timecode(2880).measure_beats_ticks();
    CHECK(on[0] == 2);
    CHECK(on[1] == 0);
    CHECK(on[2] == 0);
    const int64_t* before = st.timecode(2879).measure_beats_ticks();
    CHECK(before[0] == 1);
    CHECK(before[1] == 1);
    CHECK(before[2] == 479);
}

TEST_CASE("timing: in 6/8 the grid's beat is an eighth note (D115)") {
    std::map<int64_t, int64_t> tpm{{0, 1440}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {6, 8}}});
    const int64_t* p = st.timecode(500).measure_beats_ticks();
    CHECK(p[0] == 0);
    CHECK(p[1] == 2);
    CHECK(p[2] == 20);

    // Clone Hero's line kinds: the off-beats of 6/8 are weak beats.
    using L = hydra::MeasureIndex::Line;
    const auto lines = grid_lines(st.measure_index(), 1440);
    const std::vector<std::pair<int64_t, L>> want{
        {0, L::Measure},    {240, L::WeakBeat}, {480, L::Beat},   {720, L::WeakBeat},
        {960, L::Beat},     {1200, L::WeakBeat}, {1440, L::Measure}};
    CHECK(lines == want);
}

TEST_CASE("timing: the same meter again makes no new grid span; a change between beats "
          "waits for the next beat (D115)") {
    std::map<int64_t, std::pair<int, int>> same{{0, {4, 4}}, {1000, {4, 4}}};
    hydra::MeasureIndex flat(same, 480);
    CHECK(flat.count() == 1);
    CHECK(measure_lines(flat, 3840) == std::vector<int64_t>{0, 1920, 3840});

    // 3/4 written at tick 1000, between the beats at 960 and 1440.
    std::map<int64_t, std::pair<int, int>> off{{0, {4, 4}}, {1000, {3, 4}}};
    hydra::MeasureIndex mi(off, 480);
    REQUIRE(mi.count() == 2);
    CHECK(mi.restart_at(1) == 1440);
    CHECK(mi.lines_before_at(1) == 1);
    CHECK(measure_lines(mi, 2880) == std::vector<int64_t>{0, 1440, 2880});

    // Two signatures between the same two beats: the later one is in force
    // at the beat, and the earlier never shows.
    std::map<int64_t, std::pair<int, int>> two{{0, {4, 4}}, {1000, {7, 8}}, {1200, {3, 4}}};
    hydra::MeasureIndex m2(two, 480);
    REQUIRE(m2.count() == 2);
    CHECK(m2.restart_at(1) == 1440);
    CHECK(m2.numerator_at(1) == 3);
    // And when the later one is the meter already in force, nothing changes.
    std::map<int64_t, std::pair<int, int>> back{{0, {4, 4}}, {1000, {7, 8}}, {1200, {4, 4}}};
    CHECK(hydra::MeasureIndex(back, 480).count() == 1);
}

TEST_CASE("timing: a measure line's number and tick agree with the grid walk") {
    // Mid-measure restart, a change between beats, two signatures between the
    // same beats, the same meter again, and a few changes in a row.
    const std::vector<std::map<int64_t, std::pair<int, int>>> grids{
        {{0, {4, 4}}, {2880, {7, 8}}},
        {{0, {4, 4}}, {1000, {3, 4}}},
        {{0, {4, 4}}, {1000, {7, 8}}, {1200, {3, 4}}},
        {{0, {4, 4}}, {1000, {4, 4}}},
        {{0, {6, 8}}, {2000, {5, 4}}, {2500, {7, 16}}, {9000, {4, 4}}},
    };
    const int64_t last_tick = 20000;
    for (const auto& sigs : grids) {
        hydra::MeasureIndex mi(sigs, 480);
        const std::vector<int64_t> lines = measure_lines(mi, last_tick);
        REQUIRE(lines.size() > 3);
        for (size_t n = 0; n < lines.size(); ++n) {
            CAPTURE(n);
            CHECK(mi.measure_line_tick(static_cast<int64_t>(n)) == lines[n]);
        }
        // Every tick up to the last line maps to the first line at or after it.
        size_t want = 0;
        for (int64_t t = -10; t <= lines.back(); ++t) {
            while (lines[want] < t) ++want;
            CAPTURE(t);
            CHECK(mi.first_measure_line_at_or_after(t) == static_cast<int64_t>(want));
        }
    }
}

TEST_CASE("timing: ms_per_measure_at reads local measure durations") {
    // The Tom Sawyer (Onyxite) shape: ten 7/8 measures at 87.35 BPM, then
    // 7/16 at 85.1. 7/8 = 1680 ticks at res 480; 7/16 = 840.
    std::map<int64_t, int64_t> tpm{{0, 1680}, {16800, 840}};
    std::map<int64_t, double> bpm{{0, 87.35}, {16800, 85.1}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {7, 8}}, {16800, {7, 16}}});

    // The length of one whole measure, read on the scoring path: the ms
    // between a measure line and the line one measure on.
    const auto measure_ms_from = [](const hydra::SongTiming& t, int64_t line) {
        const hydra::Timecode start = t.timecode(line);
        return t.plusmeasure(start, 1).ms() - start.ms();
    };
    const double old_measure_ms = measure_ms_from(st, 0);
    const double new_measure_ms = measure_ms_from(st, 16800);
    // The two sections differ, so the boundary checks below can tell them apart.
    REQUIRE(old_measure_ms != doctest::Approx(new_measure_ms));

    CHECK(st.ms_per_measure_at(0) == doctest::Approx(old_measure_ms));
    CHECK(st.ms_per_measure_at(16800) == doctest::Approx(new_measure_ms));

    // A tick exactly on the change reads the new section; tick-1 the old one.
    CHECK(st.ms_per_measure_at(16799) == doctest::Approx(old_measure_ms));
    CHECK(st.ms_per_measure_at(16801) == doctest::Approx(new_measure_ms));

    // One meter and one tempo: every tick reads the same measure length.
    std::map<int64_t, int64_t> tpm44{{0, 1920}};
    std::map<int64_t, double> bpm120{{0, 120.0}};
    hydra::SongTiming flat(480, tpm44, bpm120, {{0, {4, 4}}});
    const double flat_measure_ms = measure_ms_from(flat, 0);
    CHECK(flat.ms_per_measure_at(0) == doctest::Approx(flat_measure_ms));
    CHECK(flat.ms_per_measure_at(12345) == doctest::Approx(flat_measure_ms));
}

TEST_CASE("timing: tempo_at and time_signature_at read the one in force (D115)") {
    // The Tom Sawyer shape above: 7/8 at 87.35 BPM, then 7/16 at 85.1 from
    // tick 16800, a measure line in both meters.
    std::map<int64_t, int64_t> tpm{{0, 1680}, {16800, 840}};
    std::map<int64_t, double> bpm{{0, 87.35}, {16800, 85.1}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {7, 8}}, {16800, {7, 16}}});
    CHECK(st.tempo_at(0) == 87.35);
    CHECK(st.tempo_at(16799) == 87.35);
    CHECK(st.tempo_at(16800) == 85.1);
    CHECK(st.time_signature_at(0) == std::make_pair(7, 8));
    CHECK(st.time_signature_at(16799) == std::make_pair(7, 8));
    CHECK(st.time_signature_at(16800) == std::make_pair(7, 16));
    CHECK(st.time_signature_at(99999) == std::make_pair(7, 16));
}

TEST_CASE("timing: the sub-tick ms helpers match the scoring path and invert each other") {
    // The field-verified map (What's My Age Again? Sync Chart shape): 4/4,
    // 155 -> 160 at 57600, 160 -> 157 at 67200, 157 -> 160 at 69120.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{
        {0, 155.0}, {57600, 160.0}, {67200, 157.0}, {69120, 160.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}});
    const hydra::MsIndex& ms_index = st.ms_index();

    // On a whole tick, the sub-tick time is the scoring path's time.
    for (int64_t tick : {int64_t(0), int64_t(57600), int64_t(67200),
                         int64_t(68880), int64_t(69120), int64_t(90000)}) {
        CAPTURE(tick);
        const double scoring_ms = st.timecode(tick).ms();
        CHECK(ms_index.ms_at_tick_f(static_cast<double>(tick)) ==
              doctest::Approx(scoring_ms).epsilon(1e-12));
    }

    // At a fractional tick, the time read back as a tick is the same tick.
    for (double tick : {0.0, 1234.5, 57599.9, 57600.0, 68880.25, 100000.75}) {
        CAPTURE(tick);
        const double ms_at_tick = ms_index.ms_at_tick_f(tick);
        const double tick_back = ms_index.tick_at_ms(ms_at_tick);
        CHECK(tick_back == doctest::Approx(tick).epsilon(1e-9));
    }
}

TEST_CASE("timing: the measure count runs on through a mid-measure meter change (D115)") {
    // 4/4 (1920 ticks/measure) until tick 2400, 1.25 measures in, then 3/4
    // (1440 ticks/measure). The count keeps the 1.25 measures and goes on in
    // the new meter, so it does not jump at the change; a Timecode gives
    // the same number.
    const int64_t change = 2400;
    const int64_t old_measure_ticks = 1920;
    const int64_t new_measure_ticks = 1440;
    std::map<int64_t, int64_t> tpm{{0, old_measure_ticks}, {change, new_measure_ticks}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}, {change, {3, 4}}});

    // The count at the change is the old meter's share: 2400 / 1920 = 1.25.
    const double at_change =
        static_cast<double>(change) / static_cast<double>(old_measure_ticks);
    CHECK(st.measures_at_tick_f(static_cast<double>(change)) == at_change);
    CHECK(st.measures_at_tick_f(static_cast<double>(change)) ==
          st.timecode(change).measures_decimal());
    // One tick before the change is one tick of the old meter less; one tick
    // after is one tick of the new meter more.
    CHECK(st.measures_at_tick_f(static_cast<double>(change - 1)) ==
          doctest::Approx(at_change - 1.0 / static_cast<double>(old_measure_ticks))
              .epsilon(1e-12));
    CHECK(st.measures_at_tick_f(static_cast<double>(change + 1)) ==
          doctest::Approx(at_change + 1.0 / static_cast<double>(new_measure_ticks))
              .epsilon(1e-12));
    CHECK(st.measures_at_tick_f(static_cast<double>(change + 1)) ==
          st.timecode(change + 1).measures_decimal());
}

TEST_CASE("timing: the measure count starts at tick 0, as the grid does, past a signature "
          "before tick 0") {
    // Resolution 192: 3/4 written at tick -192 (576 ticks/measure), then 4/4
    // at tick 0 (768 ticks/measure). The grid ignores the 3/4, so tick 0 is
    // measure 1, beat 1; the count must read 0 there too, and tick 192 is a
    // quarter of a 4/4 measure in.
    std::map<int64_t, int64_t> tpm{{-192, 576}, {0, 768}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(192, tpm, bpm, {{-192, {3, 4}}, {0, {4, 4}}});

    CHECK(st.timecode(0).measure_beats_ticks()[0] == 0);
    CHECK(st.timecode(0).measures_decimal() == 0.0);
    CHECK(st.timecode(192).measures_decimal() == 0.25);
    CHECK(st.measure_count().count() == 1);
    CHECK(st.plusmeasure(st.timecode(0), 1).ticks() == 768);

    // A map with only a negative entry still has no tick-0 entry.
    CHECK_THROWS_AS(hydra::MeasureCount(std::map<int64_t, int64_t>{{-192, 576}}),
                    std::out_of_range);
}

TEST_CASE("timing: plusmeasure is exact inside one meter and across two changes (D115)") {
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming flat(480, {{0, 1680}}, bpm, {{0, {7, 8}}});
    for (int64_t tick = 0; tick < 20000; tick += 37)
        for (int64_t n : {int64_t(1), int64_t(2), int64_t(8)})
            CHECK(flat.plusmeasure(flat.timecode(tick), n).ticks() == tick + n * 1680);

    // 4/4, then 7/8 from tick 2880, then 3/4 from tick 3720 (half a 7/8
    // measure later). Three measures from tick 1920: half a 4/4 measure, half
    // a 7/8 measure, then two 3/4 measures (2880 ticks): tick 6600.
    hydra::SongTiming st(480, {{0, 1920}, {2880, 1680}, {3720, 1440}}, bpm,
                         {{0, {4, 4}}, {2880, {7, 8}}, {3720, {3, 4}}});
    CHECK(st.plusmeasure(st.timecode(1920), 3).ticks() == 6600);
    // And back again.
    CHECK(st.plusmeasure(st.timecode(6600), -3).ticks() == 1920);
}

TEST_CASE("timing: display_tick_at_ms rounds to the nearest tick and never goes below 0") {
    // Flat 120 BPM at resolution 480: one beat (480 ticks) is 500 ms, so
    // 1000 ms is tick 960.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}});

    // Less than half a tick before tick 960 already shows tick 960.
    CHECK(st.display_tick_at_ms(999.9) == 960);
    CHECK(st.display_tick_at_ms(1000.0) == 960);
    // Before the song starts the shown tick stays at 0.
    CHECK(st.display_tick_at_ms(-250.0) == 0);
}

TEST_CASE("timing: SP held for some bars runs kMeasuresPerSpBar measures per bar") {
    // 4/4 at 480 ticks per beat. The measure length is read from the count's
    // own map, so the expected end does not restate it.
    const std::map<int64_t, int64_t> tpm{{0, 1920}};
    const std::map<int64_t, double> bpm{{0, 120.0}};
    const hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}});
    const int64_t measure_ticks = st.measure_count().tpm_at(0);
    const hydra::Timecode start = st.timecode(0);

    for (int64_t bars : {int64_t(1), int64_t(2), int64_t(4)}) {
        CAPTURE(bars);
        const hydra::Timecode end = st.plusmeasure(start, hydra::sp_bars_to_measures(bars));
        const int64_t bar_measures = bars * hydra::kMeasuresPerSpBar;
        CHECK(end.ticks() == bar_measures * measure_ticks);
        CHECK(end.measures_decimal() == static_cast<double>(bar_measures));
    }
}

// Findings 258 and 319; D27.
TEST_CASE("timing: a new Song and an empty Preview scene read the one default meter, and "
          "apply_timesig sets a meter with its signature") {
    const hydra::Song song(192);
    CHECK(song.tpm_changes == std::map<int64_t, int64_t>{{0, 768}});
    CHECK(song.timesig_changes.at(0) ==
          std::make_pair(hydra::kDefaultTimeSigNumerator, hydra::kDefaultTimeSigDenominator));

    // Fixtures set a meter and its signature together, through the owner.
    hydra::Song fixture(480);
    hydra::apply_timesig(fixture, 2880, 3, 4);
    CHECK(fixture.tpm_changes.at(2880) == 1440);
    CHECK(fixture.timesig_changes.at(2880) == std::make_pair(3, 4));

    // A scene built from nothing reads Song's default, not a literal of its own.
    const hydra::app::PreviewTimeSig none;
    CHECK(none.numerator == hydra::kDefaultTimeSigNumerator);
    CHECK(none.denominator == hydra::kDefaultTimeSigDenominator);
}

// ---- song speed (TM-56 on docs/clone-hero/03-timing.md) --------------------

namespace {

// A Song with `src`'s tempo and meter maps and no notes, timed at `percent`.
hydra::Song maps_at_speed(const hydra::Song& src, int percent) {
    hydra::Song s(src.tick_resolution());
    s.tpm_changes = src.tpm_changes;
    s.bpm_changes = src.bpm_changes;
    s.timesig_changes = src.timesig_changes;
    s.set_speed_percent(percent);
    s.build_timing();
    return s;
}

// The corpus charts with at least one tempo change, so a reordered float
// operation in a later tempo section would show.
std::vector<const hydra::Song*> tempo_change_charts() {
    std::vector<const hydra::Song*> out;
    for (const std::string& path : corpus::chart_paths()) {
        const hydra::Song& song = corpus::song(path, hydra::ChartReading{});
        if (song.bpm_changes.size() > 1) out.push_back(&song);
    }
    return out;
}

// Every tick the checks below probe: each chord's and each tempo change's.
std::vector<int64_t> probe_ticks(const hydra::Song& song) {
    std::vector<int64_t> ticks;
    for (const hydra::SongTimestamp& ts : song.sequence) ticks.push_back(ts.timecode.ticks());
    for (const auto& kv : song.bpm_changes) ticks.push_back(kv.first);
    return ticks;
}

constexpr int kHalfSpeed = 50;
constexpr int kDoubleSpeed = 200;

}  // namespace

TEST_CASE("song speed: at 100% every time is the time the file's resolution gives, "
          "bit for bit") {
    const std::vector<const hydra::Song*> charts = tempo_change_charts();
    REQUIRE_FALSE(charts.empty());
    for (const hydra::Song* song : charts) {
        // The parsed song is timed at the default speed, and its working
        // resolution is the file's own.
        CHECK(song->speed_percent() == hydra::kNormalSpeedPercent);
        CHECK(song->timing().working_resolution() ==
              static_cast<double>(song->tick_resolution()));
        const hydra::MsIndex no_ratio(song->bpm_changes,
                                      static_cast<double>(song->tick_resolution()));
        const hydra::Song set_100 = maps_at_speed(*song, hydra::kNormalSpeedPercent);
        bool same = true;
        for (const int64_t t : probe_ticks(*song)) {
            const double want = no_ratio.at(t);
            if (song->timing().ms_index().at(t) != want ||
                set_100.timing().ms_index().at(t) != want)
                same = false;
        }
        for (const hydra::SongTimestamp& ts : song->sequence)
            if (ts.timecode.ms() != no_ratio.at(ts.timecode.ticks())) same = false;
        CHECK(same);
    }
}

TEST_CASE("song speed: at 200% a tick's time is half its 100% time, at 50% twice, and "
          "tick_at_ms reads each back") {
    const std::vector<const hydra::Song*> charts = tempo_change_charts();
    REQUIRE_FALSE(charts.empty());
    for (const hydra::Song* song : charts) {
        const hydra::Song half = maps_at_speed(*song, kHalfSpeed);
        const hydra::Song twice = maps_at_speed(*song, kDoubleSpeed);
        const hydra::MsIndex& at100 = song->timing().ms_index();
        const hydra::MsIndex& at50 = half.timing().ms_index();
        const hydra::MsIndex& at200 = twice.timing().ms_index();
        bool scaled = true, inverse = true;
        for (const int64_t t : probe_ticks(*song)) {
            // Halving and doubling a double is exact, so these are equal, not
            // close.
            if (at200.at(t) * 2.0 != at100.at(t) || at50.at(t) != at100.at(t) * 2.0)
                scaled = false;
            const double tick = static_cast<double>(t);
            if (at200.tick_at_ms(at200.at(t)) != doctest::Approx(tick) ||
                at50.tick_at_ms(at50.at(t)) != doctest::Approx(tick))
                inverse = false;
        }
        CHECK(scaled);
        CHECK(inverse);
        CHECK(half.timing().speed_ratio() * 2.0 == song->timing().speed_ratio());
        CHECK(twice.timing().speed_ratio() == song->timing().speed_ratio() * 2.0);
    }
}

TEST_CASE("song speed: the file's resolution and Star Power's length in ticks do not "
          "move with the speed") {
    const std::vector<const hydra::Song*> charts = tempo_change_charts();
    REQUIRE_FALSE(charts.empty());
    const int64_t sp_measures = hydra::sp_bars_to_measures(hydra::kSpActivationBars);
    for (const hydra::Song* song : charts) {
        for (const int percent : {kHalfSpeed, kDoubleSpeed}) {
            CAPTURE(percent);
            const hydra::Song other = maps_at_speed(*song, percent);
            CHECK(other.timing().tick_resolution() == song->timing().tick_resolution());
            bool same = true;
            for (const hydra::SongTimestamp& ts : song->sequence) {
                const int64_t t = ts.timecode.ticks();
                if (other.timing().plusmeasure(other.timecode(t), sp_measures).ticks() !=
                    song->timing().plusmeasure(song->timecode(t), sp_measures).ticks())
                    same = false;
            }
            CHECK(same);
        }
    }
}

TEST_CASE("song speed: the 1.1 fill deadline falls on the same tick at every speed") {
    // A tempo change between the deadline and the fill, so the four beats
    // cross two tempos.
    const std::map<int64_t, int64_t> tpm{{0, 1920}};
    const std::map<int64_t, double> bpm{{0, 120.0}, {7200, 150.0}};
    const int64_t fill_end = 7680;
    const int64_t fill_length = 480;
    const hydra::SongTiming at100(480, tpm, bpm, {{0, {4, 4}}});
    const double deadline100 = hydra::activation_fill_deadline_ms(
        at100, fill_end, fill_length, hydra::FillDeadlineRule::Ch11);
    const double tick100 = at100.ms_index().tick_at_ms(deadline100);
    for (const int percent : {kHalfSpeed, kDoubleSpeed}) {
        CAPTURE(percent);
        hydra::Song song(480);
        song.bpm_changes = bpm;
        song.set_speed_percent(percent);
        song.build_timing();
        const hydra::SongTiming& timing = song.timing();
        const double deadline = hydra::activation_fill_deadline_ms(
            timing, fill_end, fill_length, hydra::FillDeadlineRule::Ch11);
        CHECK(deadline != deadline100);
        CHECK(timing.ms_index().tick_at_ms(deadline) == doctest::Approx(tick100));
    }
}

namespace {

// The Epidermis shape of tests/record_fixtures.h (make_late_sqin_song), timed
// at `percent`: a note every beat at 240 BPM and 480 ticks a beat, phrases
// ending at 480, 1920 and 13920, and a fill at 5760. Built here because the
// speed has to be set before build_timing.
hydra::Song late_sqin_song_at(int percent) {
    hydra::Song song(480);
    song.bpm_changes[0] = 240.0;
    song.set_speed_percent(percent);
    song.build_timing();
    const std::vector<int64_t> phrases{480, 1920, 13920};
    for (int64_t t = 0; t <= 21120; t += 480) {
        hydra::SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord.add_note(hydra::NoteColor::Red);
        if (std::find(phrases.begin(), phrases.end(), t) != phrases.end())
            hydra::test::mark_phrase_end(ts, t, song.tick_resolution());
        if (t == 5760) ts.activation_length = 960;
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

}  // namespace

TEST_CASE("song speed: at 200% a squeeze needs half the ms, and the timing step, budgets "
          "and tiers do not move") {
    const hydra::Song song100 = late_sqin_song_at(hydra::kNormalSpeedPercent);
    const hydra::Song song200 = late_sqin_song_at(kDoubleSpeed);
    const hydra::ScoreGraph graph100(song100, 4);
    const hydra::ScoreGraph graph200(song200, 4);
    const std::vector<hydra::Path> paths100 = hydra::run_search(graph100, hydra::EngineOptions{});
    const std::vector<hydra::Path> paths200 = hydra::run_search(graph200, hydra::EngineOptions{});
    REQUIRE_FALSE(paths100.empty());
    REQUIRE_FALSE(paths200.empty());
    REQUIRE(paths100.front().activations.size() == 1);
    REQUIRE(paths200.front().activations.size() == 1);
    const hydra::Activation& a100 = paths100.front().activations.front();
    const hydra::Activation& a200 = paths200.front().activations.front();
    CHECK(a200.timecode.ticks() == a100.timecode.ticks());
    REQUIRE(a100.sqinouts.size() == 1);
    REQUIRE(a200.sqinouts.size() == 1);

    // The squeeze's stored ms is played time: half at twice the speed.
    CHECK(a100.sqinouts[0].offset_ms != 0.0);
    CHECK(a200.sqinouts[0].offset_ms * 2.0 == doctest::Approx(a100.sqinouts[0].offset_ms));

    // The budget the squeeze is rated on reads the stored transfer scales, which
    // are ratios of two measure lengths and so hold no speed.
    const auto scales100 = hydra::stored_transfer_scales(a100);
    const auto scales200 = hydra::stored_transfer_scales(a200);
    REQUIRE(scales100.has_value());
    REQUIRE(scales200.has_value());
    REQUIRE(scales100->sqins.size() == 1);
    REQUIRE(scales200->sqins.size() == 1);
    for (const auto& [s100, s200] :
         {std::make_pair(scales100->sqins[0], scales200->sqins[0]),
          std::make_pair(scales100->post, scales200->post)}) {
        CHECK(s200.early == doctest::Approx(s100.early));
        CHECK(s200.late == doctest::Approx(s100.late));
        CHECK(hydra::squeeze_budget_ms(s200.early) ==
              doctest::Approx(hydra::squeeze_budget_ms(s100.early)));
        CHECK(hydra::squeeze_budget_ms(s200.late) ==
              doctest::Approx(hydra::squeeze_budget_ms(s100.late)));
    }
    // kTimingStepMs, nominal_budget_ms, timing_tiers and tier_for take no song
    // and no timing, so no speed can reach them: a tier read off the squeeze's
    // ms moves only because the ms does.
}

namespace {

// Sixteenth notes at 180 BPM (TM-43's worked example on
// docs/clone-hero/03-timing.md), 192 ticks a beat, timed at `percent`.
hydra::Song sixteenths_at(int percent) {
    hydra::Song song(192);
    song.bpm_changes[0] = 180.0;
    song.set_speed_percent(percent);
    song.build_timing();
    const int64_t sixteenth = song.tick_resolution() / 4;
    for (int64_t i = 0; i < 9; ++i) {
        hydra::SongTimestamp ts;
        ts.timecode = song.timecode(i * sixteenth);
        ts.chord.add_note(hydra::NoteColor::Red);
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

}  // namespace

TEST_CASE("song speed: the Dynamic Hit Window side follows the formula on played gaps at "
          "50%, 100% and 200%") {
    std::vector<double> sides;
    std::vector<double> gaps;
    for (const int percent : {kHalfSpeed, hydra::kNormalSpeedPercent, kDoubleSpeed}) {
        CAPTURE(percent);
        const hydra::Song song = sixteenths_at(percent);
        const hydra::MsIndex& ms = song.timing().ms_index();
        const hydra::ChordTimes ct = hydra::chord_times(song);
        REQUIRE(ct.ms.size() == song.sequence.size());
        // A middle chord: a neighbour either side.
        const size_t i = ct.ms.size() / 2;
        const double before = ms.at(ct.ticks[i]) - ms.at(ct.ticks[i - 1]);
        const double after = ms.at(ct.ticks[i + 1]) - ms.at(ct.ticks[i]);
        CHECK(ct.ms[i] == ms.at(ct.ticks[i]));
        CHECK(ct.sides_ms[i] == doctest::Approx(hydra::window_side_ms(before, after)));
        sides.push_back(ct.sides_ms[i]);
        gaps.push_back(before);
    }
    // The gaps are played time, so they halve as the speed doubles; the
    // window, built from them, narrows from 50% to 200%.
    CHECK(gaps[0] == doctest::Approx(gaps[1] * 2.0));
    CHECK(gaps[2] * 2.0 == doctest::Approx(gaps[1]));
    CHECK(sides[0] > sides[1]);
    CHECK(sides[1] > sides[2]);
}

TEST_CASE("song speed: a speed set after the timing is built, or of 0, is refused") {
    hydra::Song built(192);
    built.bpm_changes[0] = 120.0;
    built.build_timing();
    CHECK_THROWS_AS(built.set_speed_percent(kDoubleSpeed), std::logic_error);

    hydra::Song fresh(192);
    CHECK_THROWS_AS(fresh.set_speed_percent(0), std::invalid_argument);
    CHECK(fresh.speed_percent() == hydra::kNormalSpeedPercent);
}

TEST_CASE("song speed: a chart read through the analysis settings at 150% is timed at "
          "a ratio of 1.5, in both parsers") {
    hydra::app::Settings at150;
    at150.speed_percent = 150;
    const hydra::app::AnalysisSettings analysis = at150.to_analysis_settings();
    for (const char* suffix : {".chart", ".mid"}) {
        INFO(suffix);
        const hydra::Song& song = corpus::song(corpus::first_chart_with_suffix(suffix),
                                               analysis.reading(), analysis.rules);
        CHECK(song.speed_percent() == 150);
        CHECK(song.timing().speed_ratio() == 1.5);
    }
}

// "Who writes a meter or a signature into Song's maps?" and "Does the Preview
// type its own 4/4 meter?" are rows in tests/single_owner/rules/parse.json,
// read by the one scan of the source tree.
