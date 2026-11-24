// Unit tests for core/squeeze_rating: the frontend transfer scales read from
// the stored SP-end steps, and the display-layer judgement built on top: each
// row's multiplier and effective ms, the activation's flags and the timing
// tiers.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "app/path_view.h"
#include "core/squeeze_rating.h"
#include "core/rules.h"
#include "corpus_util.h"
#include "parse/chart_files.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/pather.h"

using namespace hydra;
namespace fs = std::filesystem;

namespace {

// The chart files in one testdata/input fixture folder, sorted. Library
// discovery (corpus::chart_paths) does not list these fixture folders, so
// each file is loaded by path, the way test_fast_tempo.cpp and
// test_tied_variants.cpp load them.
std::vector<std::string> fixture_chart_paths(const std::string& folder) {
    std::vector<std::string> paths;
    for (const fs::directory_entry& e :
         fs::directory_iterator(fs::u8path(corpus::root() + "/" + folder))) {
        const std::string p = e.path().u8string();
        if (e.is_regular_file() && chart_format_of(p) != ChartFormat::None)
            paths.push_back(p);
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

// The fixture folders this file analyzes beside the library's charts.
const std::vector<std::string>& squeeze_fixture_folders() {
    static const std::vector<std::string> folders{"test_folded_sqin", "test_fast_tempo"};
    return folders;
}

// One activation the search wrote, with the chart file it came from.
struct ChartActivation {
    std::string chart;
    Activation act;
};

// Every activation on every path the search keeps (HydraRecord::all_paths),
// under the app's default settings, for every chart library discovery lists
// (corpus::chart_paths) and every chart in the squeeze fixture folders. A
// chart with no notes under those settings has no activation and is passed
// over. Built once per run.
const std::vector<ChartActivation>& searched_activations() {
    static const std::vector<ChartActivation> all = [] {
        const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
        std::vector<std::string> charts = corpus::chart_paths();
        for (const std::string& folder : squeeze_fixture_folders())
            for (const std::string& p : fixture_chart_paths(folder)) charts.push_back(p);
        std::vector<ChartActivation> out;
        for (const std::string& chart : charts) {
            if (corpus::song(chart, cfg.reading(), cfg.rules).is_empty()) continue;
            for (const Path* p : corpus::analyzed(chart, cfg).all_paths())
                for (const Activation& a : p->all_activations()) out.push_back({chart, a});
        }
        return out;
    }();
    return all;
}

// The eff. figure worked out by hand, the rule the Paths tab shows: the
// note's distance from the end, rescaled by the two-hit budget at no scale
// (1 + 1 windows) over the budget at this scale (1 + scale windows).
double figure_by_rule(double offset_ms, double scale) {
    return std::abs(offset_ms) * (1.0 + kIdentityScale) / (1.0 + scale);
}

}  // namespace

TEST_CASE("frontend_transfer_scales: measure-rate ratio, both directions") {
    // Ten 7/8 measures at 87.35 BPM, then 7/16 at 85.1, then two tempo
    // changes inside the 7/16 section, to 90 and to 95 BPM. No boundary sits
    // on any end, so early == late at each. A measure lasts its quarter-note
    // count times 60000 / BPM ms, so a scale is the end's quarters / BPM over
    // the activation's quarters / BPM.
    const double q78 = 3.5, q716 = 1.75;  // quarter notes in a 7/8 and a 7/16 measure
    const double bpm_a = 87.35, bpm_b = 85.1, bpm_c = 90.0, bpm_d = 95.0;
    const int64_t m78 = 1680, m716 = 840;  // those measures in ticks at 480 tpq
    const int64_t section_b = 10 * m78;    // tick 16800
    const int64_t tempo_c = 23000, tempo_d = 24500;  // inside the 7/16 section
    std::map<int64_t, int64_t> tpm{{0, m78}, {section_b, m716}};
    std::map<int64_t, double> bpm{
        {0, bpm_a}, {section_b, bpm_b}, {tempo_c, bpm_c}, {tempo_d, bpm_d}};
    SongTiming st(480, tpm, bpm, {{0, {7, 8}}, {section_b, {7, 16}}});

    // An 8-bar meter is 16 measures: the ten 7/8 ones, then six of 7/16.
    const int64_t plain_end = section_b + 6 * m716;  // tick 21840
    Activation act;
    act.timecode = st.timecode(0);
    test::set_sp_meter(act, 8);
    // No SqIn yet: the end is the plain 16-measure end.
    test::set_plain_window(act, plain_end);

    auto scales = frontend_transfer_scales(act, st);
    REQUIRE(scales.has_value());
    const double a_to_b = (q716 / bpm_b) / (q78 / bpm_a);
    CHECK(scales->post.late == doctest::Approx(a_to_b).epsilon(1e-12));
    CHECK(scales->post.early == doctest::Approx(a_to_b).epsilon(1e-12));
    CHECK(scales->sqins.empty());

    // Two SqIns, each a SqIn step two 7/16 measures further on. Each SqIn is
    // read at the end before its own step: the first at the plain end (85.1
    // BPM), the second at the end in the 90 BPM part, and the SP end after
    // both sits in the 95 BPM part. Three ends, three different scales.
    const int64_t end1 = plain_end + 2 * m716;  // 23520
    const int64_t end2 = end1 + 2 * m716;       // 25200
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 5.0});
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 6.0});
    act.sp_end_steps = {{0, plain_end, SpEndKind::Activation},
                        {plain_end - 40, end1, SpEndKind::SqIn},
                        {end1 - 20, end2, SpEndKind::SqIn}};
    auto sqin_scales = frontend_transfer_scales(act, st);
    REQUIRE(sqin_scales.has_value());
    REQUIRE(sqin_scales->sqins.size() == 2);
    CHECK(sqin_scales->sqins[0].late == doctest::Approx(a_to_b).epsilon(1e-12));
    const double a_to_c = (q716 / bpm_c) / (q78 / bpm_a);
    const double a_to_d = (q716 / bpm_d) / (q78 / bpm_a);
    CHECK(sqin_scales->sqins[1].late == doctest::Approx(a_to_c).epsilon(1e-12));
    CHECK(sqin_scales->sqins[1].early == doctest::Approx(a_to_c).epsilon(1e-12));
    CHECK(sqin_scales->post.late == doctest::Approx(a_to_d).epsilon(1e-12));
    CHECK(sqin_scales->post.early == doctest::Approx(a_to_d).epsilon(1e-12));

    // A SqOut does not move the end and stores no scale of its own.
    act.sqinouts.clear();
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -5.0});
    test::set_plain_window(act, plain_end);
    auto sqout_scales = frontend_transfer_scales(act, st);
    REQUIRE(sqout_scales.has_value());
    CHECK(sqout_scales->sqins.empty());
    CHECK(sqout_scales->post.late == doctest::Approx(a_to_b).epsilon(1e-12));

    // Has both timecode and sp_meter, but no stored SP-end steps: the record
    // cannot say where SP ended, and nothing guesses.
    Activation partial;
    partial.timecode = st.timecode(0);
    test::set_sp_meter(partial, 2);
    CHECK(!frontend_transfer_scales(partial, st).has_value());
}

TEST_CASE("frontend_transfer_scales: each SqIn reads its own stored end") {
    using K = SpEndKind;
    // Flat 4/4 at 480: a measure is 1920 ticks. The tempo changes at 9600,
    // between the plain end and the SqIn's end. In one count a measure's
    // length is 1 / BPM, so a scale is the anchor's BPM over the end's.
    const double bpm_front = 120.0, bpm_back = 150.0;
    const int64_t tempo_change = 9600, plain_end = 7680, sqin_end = 11520;
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, bpm_front}, {tempo_change, bpm_back}};
    SongTiming st(480, tpm, bpm, {{0, {4, 4}}});

    Activation act;
    act.timecode = st.timecode(0);
    test::set_sp_meter(act, 2);
    act.sp_end_steps = {{0, plain_end, K::Activation}, {plain_end - 80, sqin_end, K::SqIn}};
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 5.0});

    auto scales = frontend_transfer_scales(act, st);
    REQUIRE(scales.has_value());
    REQUIRE(scales->sqins.size() == 1);
    // The SqIn is read at the plain end, before the tempo change.
    CHECK(scales->sqins[0].late == doctest::Approx(bpm_front / bpm_front).epsilon(1e-12));
    CHECK(scales->sqins[0].early == doctest::Approx(bpm_front / bpm_front).epsilon(1e-12));
    // The SP end is read past it.
    CHECK(scales->post.late == doctest::Approx(bpm_front / bpm_back).epsilon(1e-12));
    CHECK(scales->post.early == doctest::Approx(bpm_front / bpm_back).epsilon(1e-12));

    // Nothing is stepped back from D: a phrase collected after the SqIn moves
    // D, and the SqIn's scale does not change.
    act.sp_end_steps.push_back({13000, sqin_end + 3840, K::Collected});
    auto later = frontend_transfer_scales(act, st);
    REQUIRE(later.has_value());
    REQUIRE(later->sqins.size() == 1);
    CHECK(later->sqins[0].late == doctest::Approx(bpm_front / bpm_front).epsilon(1e-12));

    // A SqIn with no SqIn step (an old record) is not guessed at.
    act.sp_end_steps = {{0, sqin_end, K::Activation}};
    CHECK_FALSE(frontend_transfer_scales(act, st).has_value());
}

TEST_CASE("frontend_transfer_scales: a clamp moves the front anchor") {
    using K = SpEndKind;
    // 120 BPM until tick 2700, then 60, at 192 tpq. The activation sits in
    // the 120 part, the clamped note and the SP end in the 60 part. In one
    // count a scale is the anchor's BPM over the end's.
    const double bpm_front = 120.0, bpm_back = 60.0;
    const int64_t act_tick = 2304, clamp_tick = 3072, end_tick = 6144;
    std::map<int64_t, int64_t> tpm{{0, 768}};
    std::map<int64_t, double> bpm{{0, bpm_front}, {2700, bpm_back}};
    SongTiming st(192, tpm, bpm, {{0, {4, 4}}});

    Activation act;
    act.timecode = st.timecode(act_tick);
    act.sp_end_steps = {{act_tick, end_tick, K::Activation}};
    auto unclamped = frontend_transfer_scales(act, st);
    REQUIRE(unclamped.has_value());
    // No clamp: measured from the activation.
    CHECK(unclamped->post.late == doctest::Approx(bpm_front / bpm_back).epsilon(1e-12));

    act.sp_end_steps = {{act_tick, 5376, K::Activation}, {clamp_tick, end_tick, K::Clamped}};
    auto clamped = frontend_transfer_scales(act, st);
    REQUIRE(clamped.has_value());
    // A clamp: measured from the clamped note, in the same tempo as the end.
    CHECK(clamped->post.late == doctest::Approx(bpm_back / bpm_back).epsilon(1e-12));
    CHECK(clamped->post.early == doctest::Approx(bpm_back / bpm_back).epsilon(1e-12));
}

// On every activation of these corpus charts, each stored scale is the
// measure length at its end over the measure length at the note that moves
// that end, read from the chart's own tempo map: at the tick for the late
// side and one tick before it for the early side. The last step gives the
// stored transfer_post, and the step squeeze_end_step(k) names gives SqIn
// k's stored transfer; step_transfer_scale gives the same for each step.
TEST_CASE("step_transfer_scale: equals every stored scale on the squeeze corpus charts") {
    SearchSettings cfg;
    cfg.sp_cap = 4;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 4;
    cfg.ms_filter = std::nullopt;

    // The measure-length ratio of one step, read straight from the tempo map.
    auto ratio_of_step = [](const Activation& act, size_t step, const SongTiming& t) {
        const int64_t anchor = act.end_anchor_tick(step);
        const int64_t end = act.sp_end_steps[step].end_tick;
        return TransferScale{t.ms_per_measure_at(end - 1) / t.ms_per_measure_at(anchor - 1),
                             t.ms_per_measure_at(end) / t.ms_per_measure_at(anchor)};
    };
    auto same = [](const TransferScale& a, const TransferScale& b) {
        return a.early == doctest::Approx(b.early).epsilon(1e-12) &&
               a.late == doctest::Approx(b.late).epsilon(1e-12);
    };

    for (const std::string& folder : squeeze_fixture_folders()) {
        CAPTURE(folder);
        int charts = 0, acts = 0, sqins = 0;
        for (const std::string& path : fixture_chart_paths(folder)) {
            CAPTURE(path);
            ++charts;
            const Song& song = corpus::song(path, ChartReading{});
            const HydraRecord& record = corpus::analyzed(path, cfg);
            for (const Path* p : record.all_paths()) {
                for (const Activation& act : p->all_activations()) {
                    ++acts;
                    REQUIRE_FALSE(act.sp_end_steps.empty());
                    const size_t last = act.sp_end_steps.size() - 1;
                    REQUIRE(act.transfer_post.has_value());
                    const std::optional<TransferScale> post =
                        step_transfer_scale(act, last, song.timing());
                    REQUIRE(post.has_value());
                    const TransferScale post_want = ratio_of_step(act, last, song.timing());
                    CHECK(same(*act.transfer_post, post_want));
                    CHECK(same(*post, post_want));

                    for (size_t k = 0; k < act.sqinouts.size(); ++k) {
                        if (act.sqinouts[k].kind != SqueezeKind::SqIn) continue;
                        ++sqins;
                        const std::optional<size_t> step = act.squeeze_end_step(k);
                        REQUIRE(step.has_value());
                        REQUIRE(*step < act.sp_end_steps.size());
                        REQUIRE(act.sqinouts[k].transfer.has_value());
                        const TransferScale want = ratio_of_step(act, *step, song.timing());
                        const std::optional<TransferScale> s =
                            step_transfer_scale(act, *step, song.timing());
                        REQUIRE(s.has_value());
                        CHECK(same(*act.sqinouts[k].transfer, want));
                        CHECK(same(*s, want));
                    }

                    // A step past the list's end is unset, never a number.
                    CHECK_FALSE(step_transfer_scale(act, last + 1, song.timing()).has_value());
                    CHECK_FALSE(step_transfer_scale(act, act.sp_end_steps.size() + 7,
                                                    song.timing()).has_value());
                }
            }
        }
        // The walk saw charts, activations and SqIns, so the checks above ran.
        CHECK(charts > 0);
        CHECK(acts > 0);
        CHECK(sqins > 0);
    }

    // A hand-built activation with no steps has no step scale at all.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming st(480, tpm, bpm, {{0, {4, 4}}});
    Activation empty;
    empty.timecode = st.timecode(0);
    CHECK_FALSE(step_transfer_scale(empty, 0, st).has_value());
}

TEST_CASE("frontend_transfer_scales: direction-dependent at boundaries") {
    // The activation sits exactly on a 6/4 -> 7/8 change and its SP end
    // exactly on a 7/8 -> 9/8 change, with a tempo change on the end tick
    // too. Early (-) hits move into the long 6/4 measure; late (+) hits into
    // the 7/8 one -- two different ratios. A measure lasts its quarter-note
    // count over the BPM, so each side is the end's quarters / BPM over the
    // activation's.
    const double q64 = 6.0, q78 = 3.5, q98 = 4.5;  // quarter notes per measure
    const double bpm_front = 130.0, bpm_before_end = 133.0, bpm_end = 129.0;
    const int64_t act_tick = 2880, m78 = 1680;
    const int64_t end_tick = act_tick + 6 * m78;  // a 3-bar meter is 6 measures: 12960
    std::map<int64_t, int64_t> tpm{{0, 2880}, {act_tick, m78}, {end_tick, 2160}};
    std::map<int64_t, double> bpm{{0, bpm_front}, {11280, bpm_before_end}, {end_tick, bpm_end}};
    SongTiming st(480, tpm, bpm, {{0, {6, 4}}, {act_tick, {7, 8}}, {end_tick, {9, 8}}});

    Activation act;
    act.timecode = st.timecode(act_tick);
    test::set_sp_meter(act, 3);
    // No SqIn: the plain end.
    test::set_plain_window(act, end_tick);

    auto scale = frontend_transfer_scales(act, st);
    REQUIRE(scale.has_value());
    // Early: the 7/8 measure at 133 before the end over the 6/4 one at 130.
    CHECK(scale->post.early ==
          doctest::Approx((q78 / bpm_before_end) / (q64 / bpm_front)).epsilon(1e-12));
    // Late: the 9/8 measure at 129 from the end over the 7/8 one at 130.
    CHECK(scale->post.late ==
          doctest::Approx((q98 / bpm_end) / (q78 / bpm_front)).epsilon(1e-12));
    CHECK(scale->post.early != doctest::Approx(scale->post.late));

    // The Dumpweed shape: constant 4/4, tempo changes exactly on both the
    // activation tick and the SP end tick. No backend rows here, so this is
    // the plain act + 2*B-measure fallback; the REAL Dumpweed activation
    // collects a phrase mid-SP and lands its deact node 2 measures later --
    // that shape is pinned by the regression case below.
    std::map<int64_t, int64_t> tpm44{{0, 1920}};
    std::map<int64_t, double> bpm2{{0, 98.0},   {1920, 97.5},
                                   {8640, 110.0}, {9600, 102.0}};
    SongTiming st2(480, tpm44, bpm2, {{0, {4, 4}}});

    Activation act2;
    act2.timecode = st2.timecode(1920);
    test::set_sp_meter(act2, 2);  // 4 measures -> end tick 9600
    // No SqIn: the plain act + 2*B-measure end, as a literal tick.
    test::set_plain_window(act2, 9600);

    auto scale2 = frontend_transfer_scales(act2, st2);
    REQUIRE(scale2.has_value());
    CHECK(scale2->post.early == doctest::Approx(98.0 / 110.0).epsilon(1e-9));
    CHECK(scale2->post.late == doctest::Approx(97.5 / 102.0).epsilon(1e-9));

    // Uniform map: exactly 1.0 both ways.
    std::map<int64_t, double> bpm120{{0, 120.0}};
    SongTiming flat(480, tpm44, bpm120, {{0, {4, 4}}});
    Activation act3;
    act3.timecode = flat.timecode(0);
    test::set_sp_meter(act3, 2);
    // No SqIn: the plain act + 2*B-measure end, as a literal tick.
    test::set_plain_window(act3, 7680);
    auto flat_scale = frontend_transfer_scales(act3, flat);
    REQUIRE(flat_scale.has_value());
    CHECK(flat_scale->post.early == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(flat_scale->post.late == doctest::Approx(1.0).epsilon(1e-12));
}

TEST_CASE("frontend_transfer_scales: a SqOut whose activation and SP end both sit on tempo changes") {
    // Constant 4/4. The activation sits exactly on a 155 -> 160 BPM change,
    // and the plain 6-measure SP end exactly on a 157 -> 160 change. The
    // SqOut note is 240 ticks before the end, inside the 157 section. In one
    // count a scale is the anchor's BPM over the end's, per side.
    const double bpm_before_act = 155.0, bpm_after_act = 160.0;
    const double bpm_before_end = 157.0, bpm_after_end = 160.0;
    const int64_t tpq = 480, measure = 4 * tpq;
    const int64_t act_tick = 30 * measure;               // 57600
    const int64_t end_tick = act_tick + 6 * measure;     // a 3-bar meter is 6 measures: 69120
    const int64_t sqout_tick = end_tick - tpq / 2;       // 68880
    std::map<int64_t, int64_t> tpm{{0, measure}};
    std::map<int64_t, double> bpm{{0, bpm_before_act},
                                  {act_tick, bpm_after_act},
                                  {end_tick - measure, bpm_before_end},
                                  {end_tick, bpm_after_end}};
    SongTiming st(tpq, tpm, bpm, {{0, {4, 4}}});

    Activation act;
    act.timecode = st.timecode(act_tick);
    test::set_sp_meter(act, 3);
    // No phrase is collected mid-SP, so the SP end is the plain one.
    test::set_plain_window(act, end_tick);

    auto scales = frontend_transfer_scales(act, st);
    REQUIRE(scales.has_value());
    CHECK(scales->post.early ==
          doctest::Approx(bpm_before_act / bpm_before_end).epsilon(1e-12));
    CHECK(scales->post.late == doctest::Approx(bpm_after_act / bpm_after_end).epsilon(1e-12));

    // A 0.0-offset backend row at the plain end names the same end, so the
    // scales do not move.
    Activation with_backend = act;
    BackendSqueeze d0;
    d0.timecode = st.timecode(end_tick);
    d0.offset_ms = 0.0;
    with_backend.backends.push_back(d0);
    auto anchored = frontend_transfer_scales(with_backend, st);
    REQUIRE(anchored.has_value());
    CHECK(anchored->post.early == doctest::Approx(scales->post.early).epsilon(1e-12));
    CHECK(anchored->post.late == doctest::Approx(scales->post.late).epsilon(1e-12));

    // The SqOut gap is half a beat at 157 BPM. The eff. figure rescales it
    // by the nominal budget over the budget at the early scale, and that
    // budget is a full late window plus an early window scaled by r.
    const double gap = st.timecode(end_tick).ms() - st.timecode(sqout_tick).ms();
    CHECK(gap == doctest::Approx(0.5 * 60000.0 / bpm_before_end).epsilon(1e-9));
    SPSqueeze sqout{SqueezeKind::SqOut, -gap};
    const double r = scales->post.early;
    CHECK(sqout.difficulty() == doctest::Approx(gap).epsilon(1e-12));
    CHECK(squeeze_budget_ms(r) == doctest::Approx(kTimingStepMs + r * kTimingStepMs).epsilon(1e-12));
    // At the identity scale the figure is the raw gap.
    CHECK(effective_backend_ms(sqout.difficulty(), kIdentityScale) ==
          doctest::Approx(gap).epsilon(1e-12));
    // An early scale under 1 shrinks the budget, so the figure is harder
    // than the raw gap.
    CHECK(effective_backend_ms(sqout.difficulty(), r) > gap);
}

TEST_CASE("field fixture: Dumpweed SqOut end anchored on the deact node") {
    // The tempo map of blink-182 - Dumpweed (Hoph2o), Expert Pro Drums 2x,
    // around its activation at tick 34560 with sp_meter 2. The player
    // collects one SP phrase mid-SP, so the search's deact node sits at tick
    // 46080 (act + 6 measures), not the plain act + 4 (42240), and the scale
    // is read there.
    const int64_t resolution = 480;
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 98.0003},
                                  {34560, 97.4999},
                                  {45120, 104.8004},
                                  {46080, 101.0002}};
    SongTiming st(resolution, tpm, bpm, {{0, {4, 4}}});

    Activation act;
    act.timecode = st.timecode(34560);
    test::set_sp_meter(act, 2);
    BackendSqueeze d0;  // the 0.0-offset row at the deact node
    d0.timecode = st.timecode(46080);
    d0.offset_ms = 0.0;
    act.backends.push_back(d0);
    // The node the search actually stamped: the mid-SP collection pushes it
    // to act + 6 measures (tick 46080), matching the 0.0-offset row above --
    // this is the whole point of the fixture (act + 2*B alone would be
    // 42240).
    test::set_plain_window(act, 46080);

    auto scales = frontend_transfer_scales(act, st);
    REQUIRE(scales.has_value());
    CHECK(scales->post.early ==
          doctest::Approx(98.0003 / 104.8004).epsilon(1e-9));  // 0.935114
    CHECK(scales->post.late ==
          doctest::Approx(97.4999 / 101.0002).epsilon(1e-9));  // 0.965344
    // No SqIn: no SqIn scales.
    CHECK(scales->sqins.empty());

    // The SqOut gap: the SqOut phrase note at tick 45960 and the deact node
    // at 46080 both sit in the 104.8004 BPM section (45120 to 46080), so the
    // gap is that many beats at that tempo.
    const int64_t sqout_tick = 45960;
    const int64_t deact_tick = 46080;
    const double gap_beats = double(deact_tick - sqout_tick) / double(resolution);
    const double gap = st.timecode(deact_tick).ms() - st.timecode(sqout_tick).ms();
    CHECK(gap == doctest::Approx(gap_beats * 60000.0 / 104.8004).epsilon(1e-9));
    // The figure is the gap rescaled to the early scale read at that node.
    const double r = scales->post.early;
    CHECK(effective_backend_ms(gap, r) == doctest::Approx(figure_by_rule(gap, r)).epsilon(1e-12));

    // The same D recovered through a nonzero-offset row (the ms -> tick
    // rounding path): the SqOut phrase note itself, 143.129 ms before D.
    Activation act2 = act;
    act2.backends.clear();
    BackendSqueeze dq;
    dq.timecode = st.timecode(45960);
    dq.offset_ms = offset_from_sp_end(st.timecode(45960).ms(), st.timecode(46080).ms());
    act2.backends.push_back(dq);
    auto scales2 = frontend_transfer_scales(act2, st);
    REQUIRE(scales2.has_value());
    CHECK(scales2->post.early == doctest::Approx(scales->post.early).epsilon(1e-12));
    CHECK(scales2->post.late == doctest::Approx(scales->post.late).epsilon(1e-12));

    // A SqIn is measured from its own stored end: the SqIn step moved the
    // end from 42240 to D, so its scale is read at 42240, inside the 97.4999
    // section -> {early 98.0003/97.4999, late 1.0}.
    Activation act3 = act;
    act3.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 5.0});
    act3.sp_end_steps = {{34560, 42240, SpEndKind::Activation},
                         {42200, 46080, SpEndKind::SqIn}};
    auto scales3 = frontend_transfer_scales(act3, st);
    REQUIRE(scales3.has_value());
    CHECK(scales3->post.early == doctest::Approx(scales->post.early));
    CHECK(scales3->post.late == doctest::Approx(scales->post.late));
    REQUIRE(scales3->sqins.size() == 1);
    CHECK(scales3->sqins[0].early == doctest::Approx(98.0003 / 97.4999).epsilon(1e-9));
    CHECK(scales3->sqins[0].late == doctest::Approx(1.0).epsilon(1e-12));
}

TEST_CASE("difficulty is the raw gap, untouched by stored transfer scales") {
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    // A SqOut 12 ms before the end needs a 12 ms late hit; a SqIn 7 ms
    // past it needs a 7 ms early one. The harder of the two is the
    // activation's difficulty.
    const double sqout_gap = 12.0;
    const double sqin_gap = 7.0;
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -sqout_gap});
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, sqin_gap});
    const double hardest_gap = std::max(sqout_gap, sqin_gap);

    REQUIRE(act.difficulty().has_value());
    CHECK(*act.difficulty() == doctest::Approx(hardest_gap));

    // The scales are display-only; the metric must not move with them.
    test::set_transfer(act, TransferScale{0.5, 3.0});
    CHECK(*act.difficulty() == doctest::Approx(hardest_gap));
}

TEST_CASE("is_scaled: less than the tolerance from 1 is no scale") {
    const double tol = kScaleIdentityTolerance;
    CHECK_FALSE(is_scaled(1.0));
    CHECK_FALSE(is_scaled(1.0 + 1e-12));
    // Two bars meant to be equal, apart only by whole-microsecond MIDI
    // tempos (a quarter note of 1,714,284 us against one of 1,714,287 us).
    CHECK_FALSE(is_scaled(1714287.0 / 1714284.0));
    CHECK_FALSE(is_scaled(1.0 + 0.9 * tol));
    CHECK_FALSE(is_scaled(1.0 - 0.9 * tol));
    CHECK(is_scaled(1.0 + 1.1 * tol));
    CHECK(is_scaled(1.0 - 1.1 * tol));
    // A difference of exactly the tolerance is scaled, on both sides, even
    // when the stored double sits just under it.
    CHECK(is_scaled(1.0 + tol));
    CHECK(is_scaled(1.0 - tol));
    // The allowance for storage error does not reach a value a hair inside.
    CHECK_FALSE(is_scaled(1.0 + tol * (1.0 - 1e-4)));
    CHECK_FALSE(is_scaled(1.0 - tol * (1.0 - 1e-4)));
    CHECK_FALSE(is_scaled(1.0 + tol * (1.0 - 1e-8)));
    // The scale line's digit cap: the closest scaled value, rounded to that
    // many decimals, never prints as 1, and one decimal fewer would.
    auto rounded = [](double v, int digits) {
        const double unit = std::pow(10.0, digits);
        return std::round(v * unit) / unit;
    };
    // 1 plus or minus kScaledMinDistance is not always scaled once stored as
    // a double, so find the closest scaled double on each side by stepping
    // one unit in the last place at a time. 64 steps is far more than the
    // few that storage rounding can need, and the loop stops there.
    auto closest_scaled = [](double start, double outward) {
        double v = start;
        for (int i = 0; i < 64 && is_scaled(v); ++i) v = std::nextafter(v, kIdentityScale);
        for (int i = 0; i < 64 && !is_scaled(v); ++i) v = std::nextafter(v, outward);
        return v;
    };
    for (const double closest : {closest_scaled(1.0 + kScaledMinDistance, 2.0),
                                 closest_scaled(1.0 - kScaledMinDistance, 0.0)}) {
        CAPTURE(closest);
        REQUIRE(is_scaled(closest));
        // One step toward 1 is no scale, so this really is the closest.
        REQUIRE_FALSE(is_scaled(std::nextafter(closest, kIdentityScale)));
        CHECK(rounded(closest, kScaleIdentityDigits) != 1.0);
        CHECK(rounded(closest, kScaleIdentityDigits - 1) == 1.0);
    }
}

TEST_CASE("rate_note: the side of the end picks the multiplier") {
    const double early_scale = 0.5;
    const double late_scale = 2.0;
    const TransferScale s{early_scale, late_scale};
    const double gap = 50.0;
    const bool inside_sp = true;
    // Inside SP an early hit is what moves the end across the note, so the
    // early scale applies.
    NoteRating in = rate_note(-gap, inside_sp, s);
    CHECK(in.early);
    CHECK(in.scale == early_scale);
    // The two-hit budget at that scale: one window for each hit, the
    // second shrunk by the scale.
    CHECK(in.budget_ms == doctest::Approx(kTimingStepMs * (1.0 + early_scale)));
    REQUIRE(in.effective_ms.has_value());
    CHECK(*in.effective_ms == doctest::Approx(figure_by_rule(gap, early_scale)).epsilon(1e-12));
    // Outside SP a late hit moves it, so the late scale applies.
    NoteRating out = rate_note(gap, !inside_sp, s);
    CHECK_FALSE(out.early);
    CHECK(out.scale == late_scale);
    REQUIRE(out.effective_ms.has_value());
    CHECK(*out.effective_ms == doctest::Approx(figure_by_rule(gap, late_scale)).epsilon(1e-12));
}

TEST_CASE("rate_note: a figure whenever the multiplier is not 1, however close") {
    // An early scale just under 1 that moves a 187.5 ms gap by under 1 ms
    // still gets a figure, at full precision.
    const double gap = 187.5;
    const double r = 0.9912;
    REQUIRE(is_scaled(r));
    NoteRating n = rate_note(-gap, true, TransferScale{r, kIdentityScale});
    REQUIRE(n.effective_ms.has_value());
    CHECK(*n.effective_ms == doctest::Approx(figure_by_rule(gap, r)).epsilon(1e-12));
    CHECK(std::abs(*n.effective_ms - gap) < 1.0);
    // Exactly 1, or float noise around it: no figure.
    CHECK_FALSE(
        rate_note(-gap, true, TransferScale{kIdentityScale, kIdentityScale}).effective_ms.has_value());
    CHECK_FALSE(rate_note(-gap, true, TransferScale{kIdentityScale + 1e-12, kIdentityScale})
                    .effective_ms.has_value());
    // A note on the end itself keeps its figure when scaled: 0 ms. Any scale
    // far from 1 on either side shows it; these two are just that.
    const double far_early = 8.59;
    const double far_late = 8.76;
    REQUIRE(is_scaled(far_early));
    NoteRating z = rate_note(0.0, true, TransferScale{far_early, far_late});
    REQUIRE(z.effective_ms.has_value());
    CHECK(*z.effective_ms == 0.0);
}

TEST_CASE("rate_activation: every row reads post, by its side of the end") {
    const double leeway = core::default_rules().backend_leeway_ms;
    // A row past the end but less than the leeway after it: counted.
    const double inside_leeway = leeway / 2.0;
    // A row well past the leeway: uncounted.
    const double past_leeway = leeway + 47.0;
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    // A row never reads the SqIn scale, so the SqIn's differs from post's.
    const TransferScale sqin_scale{3.0, 4.0};
    const TransferScale post{0.5, 2.0};
    test::set_transfer(act, sqin_scale, post);
    auto add = [&act](int64_t tick, double off) {
        BackendSqueeze b;
        b.timecode = Timecode::raw(tick);
        b.offset_ms = off;
        act.backends.push_back(b);
    };
    const double offsets[] = {-40.0, 0.0, inside_leeway, past_leeway};
    // Inside SP, on the end and inside the leeway all read the early side
    // (D290 decision 6); past the leeway reads the late side.
    const bool want_early[] = {true, true, true, false};
    for (size_t i = 0; i < 4; ++i) add(1000 + int64_t(i), offsets[i]);

    ActivationRating r = rate_activation(act);
    REQUIRE(r.backends.size() == 4);
    for (size_t i = 0; i < r.backends.size(); ++i) {
        CAPTURE(offsets[i]);
        const NoteRating& n = r.backends[i].note;
        CHECK(n.early == want_early[i]);
        const double want_scale = want_early[i] ? post.early : post.late;
        CHECK(n.scale == want_scale);
        REQUIRE(n.effective_ms.has_value());
        CHECK(*n.effective_ms ==
              doctest::Approx(figure_by_rule(offsets[i], want_scale)).epsilon(1e-12));
    }
    CHECK(r.scale_governs);
    // Rows on both sides of the end: post reads both; no SqIn, so none recorded.
    CHECK(r.post_read.early);
    CHECK(r.post_read.late);
    CHECK(r.sqins_read.empty());
}

TEST_CASE("rate_activation: the squeeze-out is rated once, as its row, at post") {
    // A SqIn extended SP, then the path squeezed out a phrase before the
    // final end. The SqOut entry and the row hold the same stored number,
    // measured from the final end, so the row's post multiplier is the only
    // one that applies. Every scale differs, so each figure names the scale
    // it read.
    const double sqin_gap = 92.1;
    const double sqout_gap = 187.5;
    const TransferScale sqin_scale{0.97, 0.974};
    const TransferScale post{0.9912, 0.98};
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -sqin_gap});
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -sqout_gap});
    test::set_transfer(act, sqin_scale, post);
    BackendSqueeze squeezed_row;
    squeezed_row.timecode = Timecode::raw(5000);
    squeezed_row.offset_ms = -sqout_gap;
    act.backends.push_back(squeezed_row);
    act.sqout_tick = 5000;

    ActivationRating r = rate_activation(act);
    REQUIRE(r.backends.size() == 1);
    CHECK(r.backends[0].squeezed_out);
    CHECK(r.backends[0].note.early);
    CHECK(r.backends[0].note.scale == post.early);
    REQUIRE(r.backends[0].note.effective_ms.has_value());
    CHECK(*r.backends[0].note.effective_ms ==
          doctest::Approx(figure_by_rule(sqout_gap, post.early)).epsilon(1e-12));
    REQUIRE(r.note_effective_ms.size() == 2);
    // The SqIn: free before its own SP end, so its SqIn scale's early side.
    REQUIRE(r.note_effective_ms[0].has_value());
    CHECK(*r.note_effective_ms[0] ==
          doctest::Approx(figure_by_rule(sqin_gap, sqin_scale.early)).epsilon(1e-12));
    // The SqOut: its row above is its only rating.
    CHECK_FALSE(r.note_effective_ms[1].has_value());

    // A free squeeze-out, already past the end, reads post's late side.
    const double past_end = 40.0;
    act.backends[0].offset_ms = past_end;
    act.sqinouts[1] = SPSqueeze{SqueezeKind::SqOut, past_end};
    r = rate_activation(act);
    CHECK_FALSE(r.backends[0].note.early);
    CHECK(r.backends[0].note.scale == post.late);
}

TEST_CASE("rate_activation: each SqIn reads its own stored scale") {
    // Two SqIns measured from two different ends, so two different scales.
    // Only the early side is scaled, and both are free, so each figure shows
    // which SqIn's scale was read.
    const double gap = 400.0;
    const double first_early = 1.6;
    const double second_early = 0.6;
    Activation act;
    act.e_offset = 300.0;
    SPSqueeze first{SqueezeKind::SqIn, -gap};
    first.transfer = TransferScale{first_early, kIdentityScale};
    SPSqueeze second{SqueezeKind::SqIn, -gap};
    second.transfer = TransferScale{second_early, kIdentityScale};
    act.sqinouts = {first, second};
    // Every scale is stored or none is read (D4), so the SP end's is stored
    // too, flat.
    act.transfer_post = TransferScale{kIdentityScale, kIdentityScale};
    ActivationRating r = rate_activation(act);
    REQUIRE(r.scales.has_value());
    REQUIRE(r.scales->sqins.size() == 2);
    CHECK(r.scales->sqins[0].early == first_early);
    CHECK(r.scales->sqins[1].early == second_early);
    REQUIRE(r.note_effective_ms.size() == 2);
    REQUIRE(r.note_effective_ms[0].has_value());
    REQUIRE(r.note_effective_ms[1].has_value());
    CHECK(*r.note_effective_ms[0] ==
          doctest::Approx(figure_by_rule(gap, first_early)).epsilon(1e-12));
    CHECK(*r.note_effective_ms[1] ==
          doctest::Approx(figure_by_rule(gap, second_early)).epsilon(1e-12));
}

TEST_CASE("rate_activation: a SqIn reads its SqIn scale, by its side of the end") {
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;
    const double free_gap = 400.0;  // before the end: free, early side
    const double earn_gap = 50.0;   // past the end: to earn, late side
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -free_gap});
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, earn_gap});
    // A SqIn never reads the backend rows' scale, so post's differs.
    const TransferScale sqin_scale{1.6, 0.5};
    test::set_transfer(act, sqin_scale, TransferScale{3.0, 4.0});
    ActivationRating r = rate_activation(act);
    REQUIRE(r.note_effective_ms.size() == 2);
    REQUIRE(r.note_effective_ms[0].has_value());
    REQUIRE(r.note_effective_ms[1].has_value());
    CHECK(*r.note_effective_ms[0] ==
          doctest::Approx(figure_by_rule(free_gap, sqin_scale.early)).epsilon(1e-12));
    CHECK(*r.note_effective_ms[1] ==
          doctest::Approx(figure_by_rule(earn_gap, sqin_scale.late)).epsilon(1e-12));
    CHECK(r.scale_governs);
    // Each SqIn marks its own side of its own end; no row reads the SP end.
    CHECK_FALSE(r.post_read.early);
    CHECK_FALSE(r.post_read.late);
    REQUIRE(r.sqins_read.size() == 2);
    CHECK(r.sqins_read[0].early);
    CHECK_FALSE(r.sqins_read[0].late);
    CHECK_FALSE(r.sqins_read[1].early);
    CHECK(r.sqins_read[1].late);
}

// D302: the side is where the note sits (core::paid_by_sp_walk), not whether
// the squeeze needs timing.
TEST_CASE("rate_activation: a SqIn on the SP end is inside Star Power, so it reads its SqIn scale's early side") {
    // Only one side scaled, so the figure shows which side was read.
    auto figure = [](TransferScale sqin_scale, double offset) {
        Activation act;
        test::set_skips(act, 0);
        act.e_offset = 300.0;
        act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, offset});
        test::set_transfer(act, sqin_scale, TransferScale{});
        ActivationRating r = rate_activation(act);
        REQUIRE(r.note_effective_ms.size() == 1);
        return r.note_effective_ms[0];
    };
    const TransferScale early_only{1.6, 1.0};
    const TransferScale late_only{1.0, 1.6};
    // Exactly on the end: inside Star Power, early side.
    REQUIRE(figure(early_only, 0.0).has_value());
    CHECK(*figure(early_only, 0.0) == 0.0);
    CHECK_FALSE(figure(late_only, 0.0).has_value());
    // Just before the end: inside Star Power, early side.
    REQUIRE(figure(early_only, -0.5).has_value());
    CHECK(*figure(early_only, -0.5) ==
          doctest::Approx(figure_by_rule(-0.5, early_only.early)).epsilon(1e-12));
    CHECK_FALSE(figure(late_only, -0.5).has_value());
    // Just past the end: still to earn, late side.
    CHECK_FALSE(figure(early_only, 0.5).has_value());
    REQUIRE(figure(late_only, 0.5).has_value());
    CHECK(*figure(late_only, 0.5) ==
          doctest::Approx(figure_by_rule(0.5, late_only.late)).epsilon(1e-12));
}

// D302: the squeezed-out row's side is read the same way, so both kinds are
// pinned at the SP end.
// The search squeezes a phrase out exactly on the SP end on real charts:
// Evans Blue - Beg [highfine] (testdata/input/common/IB24/T1) does it on its
// best path's first activation, so the walk below meets at least one.
TEST_CASE("rate_note side at the SP end: a squeeze-out at 0 reads the early side too") {
    int at_end = 0;
    for (const ChartActivation& ca : searched_activations()) {
        CAPTURE(ca.chart);
        const ActivationRating r = rate_activation(ca.act);
        REQUIRE(r.scales.has_value());
        for (const BackendRating& br : r.backends) {
            if (!br.squeezed_out || br.row.offset() != 0.0) continue;
            ++at_end;
            // On the end is inside Star Power, so the early side of the SP
            // end's stored scale governs it, and it needs no ms to move.
            CHECK(br.note.early);
            CHECK(br.note.scale == r.scales->post.early);
            CHECK(br.note.effective_ms.has_value() == is_scaled(r.scales->post.early));
            if (br.note.effective_ms) {
                CHECK(*br.note.effective_ms == 0.0);
            }
        }
    }
    REQUIRE(at_end > 0);
}

// Real cases of both ends: Pitch Dark (testdata/input/test_pitch_dark_ratio)
// has an activation whose SqIn sits past its own end at a scaled stored
// scale, so a scaled side governs it; Evans Blue - Beg [highfine]
// (testdata/input/common/IB24/T1) has activations with every stored scale at
// x1.00.
TEST_CASE("rate_activation: scale_governs only for a scaled multiplier on a rated note") {
    int governs = 0, flat = 0;
    for (const ChartActivation& ca : searched_activations()) {
        CAPTURE(ca.chart);
        const ActivationRating r = rate_activation(ca.act);
        REQUIRE(r.scales.has_value());
        // A rated row has a figure exactly when the side it read is scaled,
        // and the activation governs exactly when a row or SqIn has a figure.
        bool want = false;
        for (const BackendRating& br : r.backends) {
            CHECK(br.note.effective_ms.has_value() == is_scaled(br.note.scale));
            want = want || br.note.effective_ms.has_value();
        }
        for (const std::optional<double>& figure : r.note_effective_ms)
            want = want || figure.has_value();
        CHECK(r.scale_governs == want);
        if (r.scale_governs) ++governs;

        // An activation with no scaled side never governs, whatever its gaps.
        bool any_scaled = is_scaled(r.scales->post.early) || is_scaled(r.scales->post.late);
        for (const TransferScale& s : r.scales->sqins)
            any_scaled = any_scaled || is_scaled(s.early) || is_scaled(s.late);
        if (!any_scaled) {
            ++flat;
            CHECK_FALSE(r.scale_governs);
        }
    }
    REQUIRE(governs > 0);
    REQUIRE(flat > 0);
}

TEST_CASE("timing_tiers: the ladder's edges follow the timing step") {
    // The 2 ms difficult floor, then one half, one, one and a half and two
    // timing steps (the last is the two-hit budget), then the two open bands.
    const double w = kTimingStepMs;
    const std::vector<TimingTier> tiers = timing_tiers();
    const std::vector<double> want_cutoffs{kDifficultMs, w / 2, w, 3 * w / 2, nominal_budget_ms()};
    REQUIRE(tiers.size() == want_cutoffs.size() + 2);
    for (size_t i = 0; i < want_cutoffs.size(); ++i) {
        CAPTURE(i);
        REQUIRE(tiers[i].cutoff.has_value());
        CHECK(*tiers[i].cutoff == want_cutoffs[i]);
        // The edges rise up the ladder.
        if (i > 0) {
            CHECK(*tiers[i].cutoff > *tiers[i - 1].cutoff);
        }
    }

    // The two open bands are the last two entries, carry no cutoff, and are
    // the ones beyond_tier and none_tier name.
    const TimingTier& beyond = tiers[tiers.size() - 2];
    const TimingTier& none = tiers.back();
    CHECK_FALSE(beyond.cutoff.has_value());
    CHECK_FALSE(none.cutoff.has_value());
    CHECK(&beyond_tier(tiers) == &beyond);
    CHECK(&none_tier(tiers) == &none);

    // Every band has its own name and token.
    for (size_t i = 0; i < tiers.size(); ++i)
        for (size_t j = i + 1; j < tiers.size(); ++j) {
            CHECK(std::string(tiers[i].name) != tiers[j].name);
            CHECK(std::string(tiers[i].tok) != tiers[j].tok);
        }

    // tier_for reads the ladder: a timing on an edge is the band below it,
    // a hair past it is the next band (past the top edge, Beyond), and no
    // timing is None.
    for (size_t i = 0; i < want_cutoffs.size(); ++i) {
        CAPTURE(i);
        CHECK(tier_for(want_cutoffs[i], tiers).first == tiers[i].name);
        CHECK(tier_for(want_cutoffs[i] + 0.001, tiers).first == tiers[i + 1].name);
    }
    CHECK(tier_for(std::nullopt, tiers).first == none.name);
}

// The search gathers a deact edge's rows strictly inside kSqueezeWindowMs of
// its SP end (within_squeeze_window, in the graph and the engine's copy-out),
// and the table trims by that same window, so on every activation the search
// writes the table keeps every row the engine stored, in order. Only the
// squeezed-out row may sit farther out.
TEST_CASE("display_backends: the table keeps every row the search stored") {
    int rows = 0;
    for (const ChartActivation& ca : searched_activations()) {
        CAPTURE(ca.chart);
        const std::vector<BackendSqueeze> kept = ca.act.display_backends();
        REQUIRE(kept.size() == ca.act.backends.size());
        for (size_t i = 0; i < kept.size(); ++i) {
            ++rows;
            CAPTURE(kept[i].offset());
            CHECK(kept[i].timecode.ticks() == ca.act.backends[i].timecode.ticks());
            if (!ca.act.is_sqout_backend(kept[i])) {
                CHECK(within_squeeze_window(kept[i].offset()));
            }
        }
    }
    REQUIRE(rows > 0);
}

TEST_CASE("within_squeeze_window: exactly the window away is outside, on both sides") {
    CHECK(within_squeeze_window(0.0));
    CHECK(within_squeeze_window(kSqueezeWindowMs - 0.001));
    CHECK(within_squeeze_window(-(kSqueezeWindowMs - 0.001)));
    CHECK_FALSE(within_squeeze_window(kSqueezeWindowMs));
    CHECK_FALSE(within_squeeze_window(-kSqueezeWindowMs));
}

TEST_CASE("offset_from_sp_end: a note before the end is negative, one after it positive") {
    const double end_ms = 1250.0;
    const double before_ms = 1000.0;
    const double after_ms = 1400.0;
    CHECK(offset_from_sp_end(before_ms, end_ms) == before_ms - end_ms);
    CHECK(offset_from_sp_end(before_ms, end_ms) < 0.0);
    CHECK(offset_from_sp_end(after_ms, end_ms) == after_ms - end_ms);
    CHECK(offset_from_sp_end(after_ms, end_ms) > 0.0);
    CHECK(offset_from_sp_end(end_ms, end_ms) == 0.0);
}

// cap_clamped needs both halves: the SP cap clamped the window (clamp_tick
// is set) and the frontend decides at least one squeeze (a SqIn or SqOut, or
// a row is_frontend_decided accepts). Real cases of both outcomes: Pitch Dark
// (testdata/input/test_pitch_dark_ratio) has clamped activations that hold a
// SqIn; Evans Blue - Beg [highfine] (testdata/input/common/IB24/T1) has a
// SqOut on an activation the cap never clamped.
TEST_CASE("rate_activation: cap_clamped flag") {
    const double leeway = core::default_rules().backend_leeway_ms;
    int clamped_flagged = 0, unclamped_with_squeeze = 0;
    for (const ChartActivation& ca : searched_activations()) {
        CAPTURE(ca.chart);
        const ActivationRating r = rate_activation(ca.act);
        bool decided = !ca.act.sqinouts.empty();
        for (const BackendRating& br : r.backends)
            decided = decided || is_frontend_decided(br, leeway);
        const bool clamped = ca.act.clamp_tick().has_value();
        CHECK(r.cap_clamped == (clamped && decided));
        if (clamped && r.cap_clamped) ++clamped_flagged;
        if (!clamped && !ca.act.sqinouts.empty()) {
            ++unclamped_with_squeeze;
            CHECK_FALSE(r.cap_clamped);
        }
    }
    REQUIRE(clamped_flagged > 0);
    REQUIRE(unclamped_with_squeeze > 0);
}

// The rows the search writes: Pitch Dark (testdata/input/test_pitch_dark_ratio)
// has chords inside Star Power and past the leeway beside one SP end, and
// Evans Blue - Beg [highfine] (testdata/input/common/IB24/T1) squeezes a
// phrase out, so the walk meets every kind of row.
TEST_CASE("is_frontend_decided: squeezed out, or not counted without a squeeze") {
    const double leeway = core::default_rules().backend_leeway_ms;
    int counted = 0, uncounted = 0, squeezed = 0;
    for (const ChartActivation& ca : searched_activations()) {
        CAPTURE(ca.chart);
        const ActivationRating r = rate_activation(ca.act, leeway);
        REQUIRE(r.scales.has_value());
        for (const BackendRating& br : r.backends) {
            CAPTURE(br.row.offset());
            if (br.squeezed_out) {
                ++squeezed;
                CHECK(is_frontend_decided(br, leeway));
                continue;
            }
            // A plain row's side of the end is the engine's count of it:
            // rate_activation reads the early side exactly when the engine
            // counts the row without a squeeze.
            if (br.note.early) {
                ++counted;
                CHECK_FALSE(is_frontend_decided(br, leeway));
            } else {
                ++uncounted;
                CHECK(is_frontend_decided(br, leeway));
            }
        }
    }
    REQUIRE(counted > 0);
    REQUIRE(uncounted > 0);
    REQUIRE(squeezed > 0);
}

// The engine counts a plain row less than the leeway past the SP end, so it
// is not a squeeze the frontend decides. The rating's side of the end, the
// Rating word and is_frontend_decided (the overfill flag's row test) all
// move at that one edge, Rules::backend_leeway_ms. folded_sqin.chart
// (testdata/input/test_folded_sqin) has a chord just past its first
// activation's SP end, inside the default leeway, so the walk meets one.
TEST_CASE("rate_activation: a plain row inside the leeway is not a frontend squeeze") {
    const double leeway = core::default_rules().backend_leeway_ms;
    const std::vector<TimingTier> tiers = timing_tiers();
    int inside = 0;
    for (const ChartActivation& ca : searched_activations()) {
        CAPTURE(ca.chart);
        const ActivationRating r = rate_activation(ca.act, leeway);
        for (size_t i = 0; i < r.backends.size(); ++i) {
            const BackendRating& br = r.backends[i];
            const double off = br.row.offset();
            if (br.squeezed_out || off <= 0.0 || off >= leeway) continue;
            ++inside;
            CAPTURE(off);
            // Counted, so it needs nothing: the early side, the band of a
            // timing that needs nothing, and no tag.
            CHECK(br.note.early);
            CHECK_FALSE(is_frontend_decided(br, leeway));
            const double need = backend_row_timing(off, false, leeway);
            const std::string counted_word = tier_for(need, tiers).first;
            CHECK(br.row.summarystr(false, br.note.effective_ms, leeway) == counted_word);

            // With the leeway edge on the row itself, the engine no longer
            // counts it: the late side, decided by the frontend, banded on
            // its raw offset and tagged.
            const double edge = off;
            const ActivationRating at_edge = rate_activation(ca.act, edge);
            REQUIRE(at_edge.backends.size() == r.backends.size());
            const BackendRating& moved = at_edge.backends[i];
            CHECK_FALSE(moved.note.early);
            CHECK(is_frontend_decided(moved, edge));
            const double need_at_edge = backend_row_timing(off, false, edge);
            CHECK(need_at_edge == off);
            const std::string band = tier_for(moved.note.effective_ms ? *moved.note.effective_ms
                                                                      : need_at_edge,
                                              tiers)
                                         .first;
            const std::string word = moved.row.summarystr(false, moved.note.effective_ms, edge);
            CHECK(word.rfind(band, 0) == 0);
            CHECK(word.size() > band.size());
            if (ca.act.clamp_tick()) {
                CHECK(at_edge.cap_clamped);
            }
        }
    }
    REQUIRE(inside > 0);
}

// The Rating word bands the eff. figure in place of the raw need when the
// row's side of the end is scaled and the row needs a timing. The rows come
// from rate_activation, so the figure is the one the table shows beside the
// word. Pitch Dark (testdata/input/test_pitch_dark_ratio) has uncounted rows
// past SP ends whose late side is scaled, so the walk meets such a row.
TEST_CASE("BackendSqueeze::summarystr: a scaled row's word follows its eff. figure") {
    const double leeway = core::default_rules().backend_leeway_ms;
    const std::vector<TimingTier> tiers = timing_tiers();
    int banded_on_figure = 0;
    for (const ChartActivation& ca : searched_activations()) {
        CAPTURE(ca.chart);
        const ActivationRating r = rate_activation(ca.act, leeway);
        for (const BackendRating& br : r.backends) {
            const double off = br.row.offset();
            CAPTURE(off);
            const std::string word =
                br.row.summarystr(br.squeezed_out, br.note.effective_ms, leeway);
            // The same row with no figure: banded on its raw need.
            const std::string raw_word = br.row.summarystr(br.squeezed_out, std::nullopt, leeway);
            const double need = backend_row_timing(off, br.squeezed_out, leeway);
            if (need > 0.0 && br.note.effective_ms) {
                ++banded_on_figure;
                // The band is the figure's; the tag after it is the raw word's.
                const std::string raw_band = tier_for(need, tiers).first;
                REQUIRE(raw_word.rfind(raw_band, 0) == 0);
                const std::string tag = raw_word.substr(raw_band.size());
                CHECK(word == tier_for(*br.note.effective_ms, tiers).first + tag);
            } else {
                // No timing to scale, or a side at x1.00: the raw word stands.
                CHECK(word == raw_word);
            }
        }
    }
    REQUIRE(banded_on_figure > 0);
}

TEST_CASE("squeeze_budget_ms: identity scale is twice the timing step") {
    // The backend tooltip's normal-scale figure, nominal_budget_ms, is this
    // call at the identity scale: one full timing step for each of the two
    // hits.
    CHECK(squeeze_budget_ms(kIdentityScale) == 2 * kTimingStepMs);
    CHECK(nominal_budget_ms() == squeeze_budget_ms(kIdentityScale));
}

TEST_CASE("beyond_edge_ms: where the ladder's Beyond band starts") {
    // The report's Beyond edge is the two-hit budget, the ladder's top
    // finite cutoff: a timing on it is still a finite band, a hair past it
    // is Beyond.
    const std::vector<TimingTier> tiers = timing_tiers();
    const std::string beyond = beyond_tier(tiers).name;
    CHECK(beyond_edge_ms() == nominal_budget_ms());
    CHECK(tier_for(beyond_edge_ms(), tiers).first != beyond);
    CHECK(tier_for(beyond_edge_ms() + 0.001, tiers).first == beyond);
}

// The Paths tab's backend rows print the figures rate_activation worked out,
// never figures of their own. Pitch Dark (testdata/input/test_pitch_dark_ratio)
// has backend rows on scaled sides of its SP ends under the app's default
// settings, so some rows carry an eff. figure and a scaled budget. Each such
// row's "(eff. X)" suffix and tooltip must name that row's effective_ms and
// budget_ms from the rating, and the tooltip's normal budget must be
// nominal_budget_ms; a row the rating gave no figure shows none.
TEST_CASE("build_activations: the Paths tab's backend rows print rate_activation's figures") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::string chart = corpus::chart_named("test_pitch_dark_ratio");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    const SongTiming& timing = song.timing();
    const ChordTimes chords = chord_times(song);
    const core::Rules& rules = core::default_rules();
    const std::string normal_budget = format_ms(nominal_budget_ms());
    int rows_with_figure = 0;
    for (const Path* path : rec.all_paths()) {
        CAPTURE(path->pathstring());
        const std::vector<Activation> acts = path->all_activations();
        // No backend limit, so every stored row shows, in the rating's order.
        const app::ActivationsView view =
            app::build_activations(*path, rec, &timing, chords, std::nullopt, rules);
        REQUIRE(view.acts.size() == acts.size());
        for (size_t i = 0; i < acts.size(); ++i) {
            CAPTURE(i);
            const ActivationRating rate = rate_activation(acts[i], rules.backend_leeway_ms);
            const std::vector<app::BackendRowView>& rows = view.acts[i].backends;
            REQUIRE(rows.size() == rate.backends.size());
            for (size_t r = 0; r < rows.size(); ++r) {
                const NoteRating& note = rate.backends[r].note;
                CAPTURE(rows[r].rating);
                CAPTURE(rows[r].tooltip);
                if (!note.effective_ms) {
                    CHECK(rows[r].rating.find("eff.") == std::string::npos);
                    CHECK(rows[r].tooltip.empty());
                    continue;
                }
                ++rows_with_figure;
                const std::string eff = format_ms(*note.effective_ms);
                CHECK(rows[r].rating.find("(eff. " + eff + ")") != std::string::npos);
                CHECK(rows[r].tooltip.find("Effectively " + eff + " on the normal " +
                                           normal_budget) != std::string::npos);
                CHECK(rows[r].tooltip.find("squeeze budget is " + format_ms(note.budget_ms) +
                                           ", not " + normal_budget) != std::string::npos);
            }
        }
    }
    REQUIRE(rows_with_figure > 0);
}
