// Invariant + unit tests for core/timing.
//
// The corpus half converts every chart timestamp through the chart's real
// tempo/meter maps and asserts the conversions are monotone and consistent.
// The unit half pins the edge cases the corpus may not contain.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <utility>

#include "app/preview_view.h"  // PreviewTimeSig
#include "core/timing.h"
#include "corpus_util.h"
#include "parse/song.h"

TEST_CASE("timing: corpus conversions are monotone and consistent") {
    size_t charts = 0, samples = 0, off_line_charts = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const hydra::Song& song = corpus::song(path, hydra::ChartReading{});
        if (song.is_empty()) continue;
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

    REQUIRE(charts > 0);
    MESSAGE("timing invariants: " << charts << " charts, " << samples << " samples, "
                                  << off_line_charts << " with a mid-measure meter change");
}

TEST_CASE("timing: to_multiplier thresholds") {
    using hydra::to_multiplier;
    CHECK(to_multiplier(0) == 1);
    CHECK(to_multiplier(9) == 1);
    CHECK(to_multiplier(10) == 2);
    CHECK(to_multiplier(19) == 2);
    CHECK(to_multiplier(20) == 3);
    CHECK(to_multiplier(29) == 3);
    CHECK(to_multiplier(30) == 4);
    CHECK(to_multiplier(1000) == 4);
}

TEST_CASE("timing: a map without a tick-0 entry throws") {
    std::map<int64_t, double> bpm{{480, 120.0}};
    CHECK_THROWS_AS(hydra::MsIndex(bpm, 480), std::out_of_range);

    std::map<int64_t, int64_t> tpm{{480, 1920}};
    CHECK_THROWS_AS(hydra::MeasureCount{tpm}, std::out_of_range);

    std::map<int64_t, std::pair<int, int>> sigs{{480, {4, 4}}};
    CHECK_THROWS_AS(hydra::MeasureIndex(sigs, 480), std::out_of_range);
}

TEST_CASE("timing: the time signature map must name the tpm map's ticks") {
    std::map<int64_t, int64_t> tpm{{0, 1920}, {1920, 1440}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    CHECK_THROWS_AS(hydra::SongTiming(480, tpm, bpm, {{0, {4, 4}}}), std::out_of_range);
    CHECK_NOTHROW(hydra::SongTiming(480, tpm, bpm, {{0, {4, 4}}, {1920, {3, 4}}}));
}

TEST_CASE("timing: a tick before the first tempo mark reads at the opening tempo") {
    // 120 BPM, resolution 480 -> 2 ticks/ms, so one beat (480 ticks) = 500 ms.
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

TEST_CASE("timing: ms_per_measure_at reads local measure durations") {
    // The Tom Sawyer (Onyxite) shape: ten 7/8 measures at 87.35 BPM, then
    // 7/16 at 85.1. 7/8 = 1680 ticks at res 480; 7/16 = 840.
    std::map<int64_t, int64_t> tpm{{0, 1680}, {16800, 840}};
    std::map<int64_t, double> bpm{{0, 87.35}, {16800, 85.1}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {7, 8}}, {16800, {7, 16}}});

    // 3.5 quarters * 60000/87.35 and 1.75 quarters * 60000/85.1.
    CHECK(st.ms_per_measure_at(0) == doctest::Approx(2404.1214).epsilon(1e-6));
    CHECK(st.ms_per_measure_at(16800) == doctest::Approx(1233.8425).epsilon(1e-6));

    // A tick exactly on the change reads the new section; tick-1 the old one.
    CHECK(st.ms_per_measure_at(16799) == doctest::Approx(2404.1214).epsilon(1e-6));
    CHECK(st.ms_per_measure_at(16801) == doctest::Approx(1233.8425).epsilon(1e-6));

    // Uniform map: the same value everywhere.
    std::map<int64_t, int64_t> tpm44{{0, 1920}};
    std::map<int64_t, double> bpm120{{0, 120.0}};
    hydra::SongTiming flat(480, tpm44, bpm120, {{0, {4, 4}}});
    CHECK(flat.ms_per_measure_at(0) == doctest::Approx(2000.0));
    CHECK(flat.ms_per_measure_at(12345) == doctest::Approx(2000.0));
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

TEST_CASE("timing: continuous helpers are exact, inverse, and monotone") {
    // The field-verified map (What's My Age Again? Sync Chart shape): 4/4,
    // 155 -> 160 at 57600, 160 -> 157 at 67200, 157 -> 160 at 69120.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{
        {0, 155.0}, {57600, 160.0}, {67200, 157.0}, {69120, 160.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}});

    // ms_at_tick_f agrees with the integer-tick scoring path...
    for (int64_t tick : {int64_t(0), int64_t(57600), int64_t(67200),
                         int64_t(68880), int64_t(69120), int64_t(90000)})
        CHECK(st.ms_index().ms_at_tick_f(static_cast<double>(tick)) ==
              doctest::Approx(st.timecode(tick).ms()).epsilon(1e-12));

    // ...and round-trips through its inverse at fractional ticks too.
    for (double x : {0.0, 1234.5, 57599.9, 57600.0, 68880.25, 100000.75}) {
        double ms = st.ms_index().ms_at_tick_f(x);
        CHECK(st.ms_index().tick_at_ms(ms) == doctest::Approx(x).epsilon(1e-9));
    }

    // sp_end_ms: monotone in the hit time, continuous across the 57600
    // boundary, and its interior slope is the mspm ratio the linearized
    // transfer scale samples.
    const double h0 = st.timecode(50000).ms();
    double prev = -1e18;
    bool monotone = true;
    for (double h = h0; h < h0 + 20000.0; h += 37.0) {
        double e = st.sp_end_ms(h, 6);
        if (e < prev) monotone = false;
        prev = e;
    }
    CHECK(monotone);

    const double hb = st.timecode(57600).ms();
    CHECK(st.sp_end_ms(hb + 1e-6, 6) - st.sp_end_ms(hb - 1e-6, 6) <
          doctest::Approx(1e-4));

    // act tick 57000 sits inside the 155 section; its 6-measure end lands
    // inside the 157 section, so the local slope is tps(155)/tps(157).
    const double hi = st.timecode(57000).ms();
    double slope = (st.sp_end_ms(hi + 5.0, 6) - st.sp_end_ms(hi - 5.0, 6)) / 10.0;
    CHECK(slope == doctest::Approx(1240.0 / 1256.0).epsilon(1e-9));

    // Flat tempo, on-tick activation: the continuous map reproduces the
    // tick-rounded scoring path exactly.
    std::map<int64_t, double> bpm120{{0, 120.0}};
    hydra::SongTiming flat(480, tpm, bpm120, {{0, {4, 4}}});
    hydra::Timecode tc = flat.timecode(1920);
    CHECK(flat.sp_end_ms(tc.ms(), 4) ==
          doctest::Approx(flat.plusmeasure(tc, 4).ms()).epsilon(1e-12));
}

TEST_CASE("timing: the measure count runs on through a mid-measure meter change (D115)") {
    // 4/4 (1920 ticks/measure) until tick 2400, 1.25 measures in, then 3/4
    // (1440 ticks/measure). The count keeps the 1.25 measures and goes on in
    // the new meter, so it no longer jumps at the change; a Timecode gives
    // the same number.
    std::map<int64_t, int64_t> tpm{{0, 1920}, {2400, 1440}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}, {2400, {3, 4}}});

    CHECK(st.measures_at_tick_f(2400.0) == 1.25);
    CHECK(st.measures_at_tick_f(2400.0) == st.timecode(2400).measures_decimal());
    CHECK(st.measures_at_tick_f(2399.0) == 1.2494791666666667);
    CHECK(st.measures_at_tick_f(2401.0) == 1.2506944444444446);
    CHECK(st.measures_at_tick_f(2401.0) == st.timecode(2401).measures_decimal());
}

TEST_CASE("timing: tick_at_measures_f stays the inverse across a mid-measure change") {
    std::map<int64_t, int64_t> tpm{{0, 1920}, {2400, 1440}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}, {2400, {3, 4}}});

    for (double t : {2399.0, 2400.0, 2401.0, 2400.5})
        CHECK(st.tick_at_measures_f(st.measures_at_tick_f(t)) ==
              doctest::Approx(t).epsilon(1e-9));
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

TEST_CASE("timing: one SP bar is two measures") {
    CHECK(hydra::kMeasuresPerSpBar == 2);
    CHECK(hydra::sp_bars_to_measures(1) == 2);
    CHECK(hydra::sp_bars_to_measures(4) == 8);
    CHECK(hydra::sp_bars_to_measures(-1) == -2);  // one phrase back, as squeeze_rating uses it

    // 4/4 at 480 ticks per beat: a measure is 1920 ticks, so two bars of SP
    // run four measures, 7680 ticks.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}});
    CHECK(st.plusmeasure(st.timecode(0), hydra::sp_bars_to_measures(2)).ticks() == 7680);
}

TEST_CASE("timing: plusmeasure keeps the old measure's share of a mid-measure change (D115)") {
    // 4/4 until tick 2880, one and a half measures in, then 7/8 (1680 ticks).
    // Two measures from tick 1920 are half a 4/4 measure (960 ticks) to the
    // change, then one and a half 7/8 measures (2520 ticks): tick 5400.
    std::map<int64_t, int64_t> tpm{{0, 1920}, {2880, 1680}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    hydra::SongTiming st(480, tpm, bpm, {{0, {4, 4}}, {2880, {7, 8}}});
    CHECK(st.plusmeasure(st.timecode(1920), 2).ticks() == 5400);
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

// "No meter is written by hand outside apply_timesig" and "the Preview's time
// box keeps no 4/4 literal of its own" are rows in
// tests/single_owner/rules/parse.json, read by the one scan of the source tree.
