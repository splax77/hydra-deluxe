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

#include "core/squeeze_rating.h"
#include "core/rules.h"
#include "corpus_util.h"
#include "parse/chart_files.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/pather.h"

using namespace hydra;
namespace fs = std::filesystem;

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

    for (const std::string folder : {"test_folded_sqin", "test_fast_tempo"}) {
        CAPTURE(folder);
        // Library discovery (corpus::chart_paths) does not list these fixture
        // folders, so every chart file in the folder is loaded by path, the
        // way test_fast_tempo.cpp and test_tied_variants.cpp load them.
        std::vector<std::string> paths;
        for (const fs::directory_entry& e :
             fs::directory_iterator(fs::u8path(corpus::root() + "/" + folder))) {
            const std::string p = e.path().u8string();
            if (e.is_regular_file() && chart_format_of(p) != ChartFormat::None)
                paths.push_back(p);
        }
        std::sort(paths.begin(), paths.end());
        int charts = 0, acts = 0, sqins = 0;
        for (const std::string& path : paths) {
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

TEST_CASE("transfer_scale_between: a measure that is not a positive finite length is refused") {
    // The load guard keeps such timing out of every chart (Task 3); a
    // hand-built SongTiming is not guarded, so this pins the function's own
    // refusal on both sides. A 0 BPM section makes a measure last forever,
    // a negative one makes it last negative time.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    SongTiming forever(480, tpm, std::map<int64_t, double>{{0, 120.0}, {7680, 0.0}},
                       {{0, {4, 4}}});
    CHECK(transfer_scale_between(0, 3840, forever).has_value());
    CHECK_FALSE(transfer_scale_between(0, 9600, forever).has_value());     // the end side
    CHECK_FALSE(transfer_scale_between(9600, 11520, forever).has_value()); // the front side
    SongTiming backwards(480, tpm, std::map<int64_t, double>{{0, 120.0}, {7680, -120.0}},
                         {{0, {4, 4}}});
    CHECK_FALSE(transfer_scale_between(0, 9600, backwards).has_value());
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
    CHECK(squeeze_budget_ms(r) == doctest::Approx(kHitWindowMs + r * kHitWindowMs).epsilon(1e-12));
    CHECK(effective_backend_ms(sqout.difficulty(), r) ==
          doctest::Approx(gap * nominal_budget_ms() / squeeze_budget_ms(r)).epsilon(1e-12));
    // An early scale under 1 shrinks the budget, so the figure is harder
    // than the raw gap.
    CHECK(effective_backend_ms(sqout.difficulty(), r) > gap);
}

TEST_CASE("field fixture: Dumpweed SqOut end anchored on the deact node") {
    // Field-verified: blink-182 - Dumpweed (Hoph2o), Expert Pro Drums 2x,
    // activation "1-" at m19.1.0 (tick 34560), sp_meter 2. The player
    // collects ONE SP phrase mid-SP, so the search's deact node sits at tick
    // 46080 (act + 6 measures), not the plain act + 4 (42240). The true end
    // gives early x0.935114.
    // An FC video shows frontend -74.1 ms + SqOut backend +75.1 ms landing
    // the squeeze on the 143.129 ms gap: 74.1*0.935114 + 75.1 = 144.4 >=
    // 143.129. A scale read at an end short of the true one would predict a
    // miss: x0.890911, an example this tempo map does not produce, gives
    // 141.1. DragonDelgar's published even split of +-74 = 143.129 /
    // (1 + 0.935114) matches the true end too.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 98.0003},
                                  {34560, 97.4999},
                                  {45120, 104.8004},
                                  {46080, 101.0002}};
    SongTiming st(480, tpm, bpm, {{0, {4, 4}}});

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

    // The SqOut gap and its displayed numbers.
    double gap = st.timecode(46080).ms() - st.timecode(45960).ms();
    CHECK(gap == doctest::Approx(143.129).epsilon(1e-4));
    const double r = scales->post.early;
    CHECK(effective_backend_ms(gap, r) == doctest::Approx(147.94).epsilon(1e-3));
    CHECK(74.1 * r + 75.1 > gap);  // the video's successful split

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
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -12.0});
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 7.0});

    REQUIRE(act.difficulty().has_value());
    CHECK(*act.difficulty() == doctest::Approx(12.0));

    // The scales are display-only; the metric must not move with them.
    test::set_transfer(act, TransferScale{0.5, 3.0});
    CHECK(*act.difficulty() == doctest::Approx(12.0));
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
    for (const double closest : {1.0 + kScaledMinDistance, 1.0 - kScaledMinDistance}) {
        CAPTURE(closest);
        REQUIRE(is_scaled(closest));
        CHECK(rounded(closest, kScaleIdentityDigits) != 1.0);
        CHECK(rounded(closest, kScaleIdentityDigits - 1) == 1.0);
    }
}

TEST_CASE("rate_note: the side of the end picks the multiplier") {
    const TransferScale s{0.5, 2.0};
    // Inside SP: an early hit is what moves the end across it.
    NoteRating in = rate_note(-50.0, true, s);
    CHECK(in.early);
    CHECK(in.scale == 0.5);
    CHECK(in.budget_ms == 127.5);
    REQUIRE(in.effective_ms.has_value());
    CHECK(*in.effective_ms == doctest::Approx(66.666666666666667).epsilon(1e-12));
    // Outside: a late hit.
    NoteRating out = rate_note(50.0, false, s);
    CHECK_FALSE(out.early);
    CHECK(out.scale == 2.0);
    REQUIRE(out.effective_ms.has_value());
    CHECK(*out.effective_ms == doctest::Approx(33.333333333333336).epsilon(1e-12));
}

TEST_CASE("rate_note: a figure whenever the multiplier is not 1, however close") {
    // Sinner's Vengeance act 3 shape: 187.5 ms at an early scale just under
    // 1 moves by under 1 ms, and it still gets a figure. Full precision.
    const double r = 0.9912;
    NoteRating n = rate_note(-187.5, true, TransferScale{r, 1.0});
    REQUIRE(n.effective_ms.has_value());
    CHECK(*n.effective_ms == doctest::Approx(188.32864604258737).epsilon(1e-12));
    // Exactly 1, or float noise around it: no figure.
    CHECK_FALSE(rate_note(-187.5, true, TransferScale{1.0, 1.0}).effective_ms.has_value());
    CHECK_FALSE(
        rate_note(-187.5, true, TransferScale{1.0 + 1e-12, 1.0}).effective_ms.has_value());
    // A note on the end itself keeps its figure when scaled: 0 ms.
    NoteRating z = rate_note(0.0, true, TransferScale{8.59, 8.76});
    REQUIRE(z.effective_ms.has_value());
    CHECK(*z.effective_ms == 0.0);
}

TEST_CASE("rate_activation: every row reads post, by its side of the end") {
    REQUIRE(core::default_rules().backend_leeway_ms > 1.5);
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    // A row never reads the SqIn scale.
    test::set_transfer(act, TransferScale{3.0, 4.0}, TransferScale{0.5, 2.0});
    auto add = [&act](int64_t tick, double off) {
        BackendSqueeze b;
        b.timecode = Timecode::raw(tick);
        b.offset_ms = off;
        act.backends.push_back(b);
    };
    add(1000, -40.0);  // counted, inside SP: early
    add(1001, 0.0);    // counted, on the end: early
    add(1002, 1.5);    // counted inside the leeway: early (D290 decision 6)
    add(1003, 50.0);   // uncounted, past the leeway: late

    ActivationRating r = rate_activation(act);
    REQUIRE(r.backends.size() == 4);
    CHECK(r.backends[0].note.early);
    CHECK(r.backends[1].note.early);
    CHECK(r.backends[2].note.early);
    CHECK(r.backends[2].note.scale == 0.5);
    CHECK_FALSE(r.backends[3].note.early);
    CHECK(r.backends[3].note.scale == 2.0);
    const double want[] = {53.333333333333336, 0.0, 2.0, 33.333333333333336};
    for (size_t i = 0; i < r.backends.size(); ++i) {
        REQUIRE(r.backends[i].note.effective_ms.has_value());
        CHECK(*r.backends[i].note.effective_ms == doctest::Approx(want[i]).epsilon(1e-12));
    }
    CHECK(r.scale_governs);
    // Rows on both sides of the end: post reads both; no SqIn, so none recorded.
    CHECK(r.post_read.early);
    CHECK(r.post_read.late);
    CHECK(r.sqins_read.empty());
}

TEST_CASE("rate_activation: the squeeze-out is rated once, as its row, at post") {
    // Sinner's Vengeance act 3 shape: a SqIn extended SP, then the path
    // squeezed out the [RY] phrase 187.5 ms before the final end. The SqOut
    // entry and the row hold the same stored number (its squeeze choice's
    // timing on the deact edge), measured from the final end, so the row's post
    // multiplier is the only one that applies.
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -92.1});
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -187.5});
    test::set_transfer(act, TransferScale{0.97, 0.974}, TransferScale{0.9912, 0.98});
    BackendSqueeze ry;
    ry.timecode = Timecode::raw(5000);
    ry.offset_ms = -187.5;
    act.backends.push_back(ry);
    act.sqout_tick = 5000;

    ActivationRating r = rate_activation(act);
    REQUIRE(r.backends.size() == 1);
    CHECK(r.backends[0].squeezed_out);
    CHECK(r.backends[0].note.early);
    CHECK(r.backends[0].note.scale == 0.9912);
    REQUIRE(r.backends[0].note.effective_ms.has_value());
    CHECK(*r.backends[0].note.effective_ms == doctest::Approx(188.32864604258737).epsilon(1e-12));
    REQUIRE(r.note_effective_ms.size() == 2);
    // The SqIn: free by 92.1 ms at its own SP end, so its SqIn scale's early side.
    REQUIRE(r.note_effective_ms[0].has_value());
    CHECK(*r.note_effective_ms[0] == doctest::Approx(93.50253807106598).epsilon(1e-12));
    // The SqOut: its row above is its only rating.
    CHECK_FALSE(r.note_effective_ms[1].has_value());

    // A free squeeze-out, already past the end, reads post's late side.
    act.backends[0].offset_ms = 40.0;
    act.sqinouts[1] = SPSqueeze{SqueezeKind::SqOut, 40.0};
    r = rate_activation(act);
    CHECK_FALSE(r.backends[0].note.early);
    CHECK(r.backends[0].note.scale == 0.98);
}

TEST_CASE("rate_activation: each SqIn reads its own stored scale") {
    // Two SqIns measured from two different ends, so two different scales.
    // Only the early side is scaled, and both are free, so each figure shows
    // which SqIn's scale was read.
    Activation act;
    act.e_offset = 300.0;
    SPSqueeze first{SqueezeKind::SqIn, -400.0};
    first.transfer = TransferScale{1.6, 1.0};
    SPSqueeze second{SqueezeKind::SqIn, -400.0};
    second.transfer = TransferScale{0.6, 1.0};
    act.sqinouts = {first, second};
    // Every scale is stored or none is read (D4), so the SP end's is stored
    // too, flat.
    act.transfer_post = TransferScale{1.0, 1.0};
    ActivationRating r = rate_activation(act);
    REQUIRE(r.scales.has_value());
    REQUIRE(r.scales->sqins.size() == 2);
    CHECK(r.scales->sqins[0].early == 1.6);
    CHECK(r.scales->sqins[1].early == 0.6);
    REQUIRE(r.note_effective_ms.size() == 2);
    REQUIRE(r.note_effective_ms[0].has_value());
    REQUIRE(r.note_effective_ms[1].has_value());
    CHECK(*r.note_effective_ms[0] == doctest::Approx(307.6923076923077).epsilon(1e-12));
    CHECK(*r.note_effective_ms[1] == doctest::Approx(500.0).epsilon(1e-12));
}

TEST_CASE("rate_activation: a SqIn reads its SqIn scale, by its side of the end") {
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, -400.0});  // free: early
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});    // to earn: late
    // A SqIn never reads the backend rows' scale.
    test::set_transfer(act, TransferScale{1.6, 0.5}, TransferScale{3.0, 4.0});
    ActivationRating r = rate_activation(act);
    REQUIRE(r.note_effective_ms.size() == 2);
    REQUIRE(r.note_effective_ms[0].has_value());
    REQUIRE(r.note_effective_ms[1].has_value());
    // Sun of Nothing act 5: 400 ms free at early x1.60 is worth 307.7 ms.
    CHECK(*r.note_effective_ms[0] == doctest::Approx(307.6923076923077).epsilon(1e-12));
    CHECK(*r.note_effective_ms[1] == doctest::Approx(66.666666666666667).epsilon(1e-12));
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
    CHECK(*figure(early_only, -0.5) == doctest::Approx(0.38461538461538464).epsilon(1e-12));
    CHECK_FALSE(figure(late_only, -0.5).has_value());
    // Just past the end: still to earn, late side.
    CHECK_FALSE(figure(early_only, 0.5).has_value());
    REQUIRE(figure(late_only, 0.5).has_value());
    CHECK(*figure(late_only, 0.5) == doctest::Approx(0.38461538461538464).epsilon(1e-12));
}

// D302: the squeezed-out row's side is read the same way, so both kinds are
// pinned at the SP end.
TEST_CASE("rate_note side at the SP end: a squeeze-out at 0 reads the early side too") {
    auto rated_row = [](TransferScale post) {
        Activation act;
        test::set_skips(act, 0);
        act.e_offset = 300.0;
        act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, 0.0});
        BackendSqueeze row;
        row.timecode = Timecode::raw(5000);
        row.offset_ms = 0.0;
        act.backends.push_back(row);
        act.sqout_tick = 5000;
        test::set_transfer(act, TransferScale{}, post);
        ActivationRating r = rate_activation(act, 85.0);
        REQUIRE(r.backends.size() == 1);
        REQUIRE(r.backends[0].squeezed_out);
        return r.backends[0].note;
    };
    const NoteRating early = rated_row(TransferScale{1.6, 1.0});
    CHECK(early.early);
    CHECK(early.scale == 1.6);
    REQUIRE(early.effective_ms.has_value());
    CHECK(*early.effective_ms == 0.0);
    const NoteRating late = rated_row(TransferScale{1.0, 1.6});
    CHECK(late.early);
    CHECK(late.scale == 1.0);
    CHECK_FALSE(late.effective_ms.has_value());
}

TEST_CASE("rate_activation: scale_governs only for a scaled multiplier on a rated note") {
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;
    // The late side is scaled, but the only row is inside SP (early, x1.00).
    test::set_transfer(act, TransferScale{1.0, 6.33});
    BackendSqueeze in;
    in.offset_ms = -40.0;
    act.backends.push_back(in);
    ActivationRating r = rate_activation(act);
    CHECK_FALSE(r.scale_governs);
    CHECK_FALSE(r.backends[0].note.effective_ms.has_value());

    // A flat activation never governs, even with a gap past the budget.
    Activation flat = act;
    test::set_transfer(flat, TransferScale{});
    flat.backends[0].offset_ms = 222.2;
    CHECK_FALSE(rate_activation(flat).scale_governs);
}

TEST_CASE("rate_activation: the stored scales are the only scales") {
    // A flat chart would recompute identity scales, but the search stored
    // 0.5s. The rating reads what the search stored and never recomputes.
    Activation act;
    act.timecode = Timecode::raw(0);
    test::set_sp_meter(act, 2);
    test::set_plain_window(act, 7680);
    test::set_transfer(act, TransferScale{0.5, 0.5});

    BackendSqueeze row;
    row.timecode = Timecode::raw(7728);
    row.offset_ms = 50.0;
    act.backends.push_back(row);

    ActivationRating r = rate_activation(act);
    REQUIRE(r.scales.has_value());
    CHECK(r.scales->post.late == doctest::Approx(0.5));
    CHECK(r.scales->post.early == doctest::Approx(0.5));
    CHECK(r.scale_governs);
}

TEST_CASE("timing_tiers: the ladder at the 85 ms hit window") {
    // The 85 ms window (kHitWindowMs) -> a 170 ms two-hit budget: the 2 ms "Normal"
    // floor, then quarters of the budget.
    std::vector<TimingTier> t85 = timing_tiers();
    REQUIRE(t85.size() == 7);

    CHECK(std::string(t85[0].name) == "Normal");
    CHECK(std::string(t85[0].tok) == "t0");
    REQUIRE(t85[0].cutoff.has_value());
    CHECK(*t85[0].cutoff == doctest::Approx(2.0));

    CHECK(std::string(t85[1].name) == "Hard");
    CHECK(std::string(t85[1].tok) == "t1");
    REQUIRE(t85[1].cutoff.has_value());
    CHECK(*t85[1].cutoff == doctest::Approx(42.5));

    CHECK(std::string(t85[2].name) == "Extreme");
    CHECK(std::string(t85[2].tok) == "t2");
    REQUIRE(t85[2].cutoff.has_value());
    CHECK(*t85[2].cutoff == doctest::Approx(85.0));
    // The Extreme edge is the window itself, read from its one owner.
    CHECK(*t85[2].cutoff == kHitWindowMs);

    CHECK(std::string(t85[3].name) == "Insane");
    CHECK(std::string(t85[3].tok) == "t3");
    REQUIRE(t85[3].cutoff.has_value());
    CHECK(*t85[3].cutoff == doctest::Approx(127.5));

    CHECK(std::string(t85[4].name) == "Insane+");
    CHECK(std::string(t85[4].tok) == "t4");
    REQUIRE(t85[4].cutoff.has_value());
    CHECK(*t85[4].cutoff == doctest::Approx(170.0));
    // The ladder's top cutoff is the two-hit budget itself, read from its one
    // owner, not twice the window written again.
    CHECK(*t85[4].cutoff == nominal_budget_ms());

    // The two open bands carry no cutoff, and "None" is last.
    CHECK(std::string(t85[5].name) == "Beyond");
    CHECK(std::string(t85[5].tok) == "t5");
    CHECK_FALSE(t85[5].cutoff.has_value());
    CHECK(std::string(t85[6].name) == "None");
    CHECK(std::string(t85[6].tok) == "tn");
    CHECK_FALSE(t85[6].cutoff.has_value());
}

TEST_CASE("display_backends: 500 ms window keeps everything the 500 ms search graph collects") {
    // The display window IS the search graph's squeeze window (one constant,
    // kSqueezeWindowMs), so nothing the graph gathers gets trimmed at
    // store/display time. 180 and -300 sit past the nominal budget
    // (nominal_budget_ms), and the window still keeps them.
    CHECK(kSqueezeWindowMs == 500.0);
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming st(480, tpm, bpm, {{0, {4, 4}}});

    Activation act;
    act.timecode = st.timecode(0);
    test::set_sp_meter(act, 2);

    const double offsets[] = {0.0, 180.0, -300.0, kSqueezeWindowMs - 1.0,
                              kSqueezeWindowMs + 20.0};
    for (double off : offsets) {
        BackendSqueeze row;
        row.timecode = st.timecode(0);
        row.offset_ms = off;
        act.backends.push_back(row);
    }

    std::vector<BackendSqueeze> kept = act.display_backends();
    REQUIRE(kept.size() == 4);
    CHECK(kept[0].offset_ms == doctest::Approx(0.0));
    CHECK(kept[1].offset_ms == doctest::Approx(180.0));
    CHECK(kept[2].offset_ms == doctest::Approx(-300.0));
    CHECK(kept[3].offset_ms == doctest::Approx(kSqueezeWindowMs - 1.0));

    // Exactly 500 ms away is outside, on both sides.
    Activation edge;
    for (double off : {-kSqueezeWindowMs, kSqueezeWindowMs}) {
        BackendSqueeze row;
        row.timecode = st.timecode(0);
        row.offset_ms = off;
        edge.backends.push_back(row);
    }
    CHECK(edge.display_backends().empty());
    CHECK(within_squeeze_window(kSqueezeWindowMs - 0.001));
    CHECK_FALSE(within_squeeze_window(-kSqueezeWindowMs));
    CHECK(offset_from_sp_end(1000.0, 1250.0) == -250.0);
}

TEST_CASE("rate_activation: cap_clamped flag") {
    // cap_clamped is only meaningful when both halves are true: the window
    // was cap-clamped (clamp_tick is set) AND the activation actually lists a
    // squeeze the frontend decides (a SqIn/SqOut, or a backend row that's
    // squeezed out or at or past the backend leeway). Either half missing means
    // there's no frontend-decided squeeze for the overfill warning to attach
    // to, so the flag stays false.

    // clamp_tick set, plus a SqOut -- both halves true, so this is the case
    // the overfill warning is for.
    // The SP end itself is never read here; 6144 is any end past the clamp.
    Activation clamped_with_squeeze;
    test::set_clamped_window(clamped_with_squeeze, 3072, 6144);
    clamped_with_squeeze.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -50.0});
    ActivationRating r1 = rate_activation(clamped_with_squeeze);
    CHECK(r1.cap_clamped);

    // clamp_tick set, but the only backend row is at -30 ms -- inside the
    // SP window, so the engine counts it and it isn't a squeeze the frontend
    // decides. No SqIn/SqOut either, so cap_clamped stays false.
    Activation clamped_no_squeeze;
    test::set_clamped_window(clamped_no_squeeze, 3072, 6144);
    BackendSqueeze mild_row;
    mild_row.offset_ms = -30.0;
    clamped_no_squeeze.backends.push_back(mild_row);
    ActivationRating r2 = rate_activation(clamped_no_squeeze);
    CHECK_FALSE(r2.cap_clamped);

    // A SqOut with no clamp_tick at all -- the window was never cap-clamped,
    // so there's nothing to warn about regardless of the squeeze.
    Activation unclamped_with_squeeze;
    unclamped_with_squeeze.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -50.0});
    ActivationRating r3 = rate_activation(unclamped_with_squeeze);
    CHECK_FALSE(r3.cap_clamped);
}

TEST_CASE("is_frontend_decided: squeezed out, or not counted without a squeeze") {
    const double leeway = core::default_rules().backend_leeway_ms;
    BackendRating counted;
    counted.row.offset_ms = -40.0;
    CHECK_FALSE(is_frontend_decided(counted, leeway));

    BackendRating uncounted;
    uncounted.row.offset_ms = leeway + 5.0;
    CHECK(is_frontend_decided(uncounted, leeway));

    BackendRating squeezed;
    squeezed.row.offset_ms = -40.0;
    squeezed.squeezed_out = true;
    CHECK(is_frontend_decided(squeezed, leeway));

    BackendRating no_offset;
    CHECK_FALSE(is_frontend_decided(no_offset, leeway));
}

// The engine counts a plain row less than the leeway past the SP end, so it
// is not a squeeze the frontend decides. The rating's early or late side and
// the overfill flag both use that same edge (D274 user decision 7).
TEST_CASE("rate_activation: a plain row inside the leeway is not a frontend squeeze") {
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    // The row below sits 2.5 ms past the end; 3077 is any end before it.
    test::set_clamped_window(act, 3072, 3077);
    // A late scale this small would give a row read on the late side a
    // figure: 2.5 ms reads as 4.2 ms.
    test::set_transfer(act, TransferScale{}, TransferScale{1.0, 0.2});
    BackendSqueeze row;
    row.timecode = Timecode::raw(3080);
    row.offset_ms = 2.5;
    act.backends.push_back(row);

    ActivationRating r = rate_activation(act);
    CHECK_FALSE(r.cap_clamped);
    // Counted inside the leeway, so the early side governs it -- x1.00 here.
    CHECK_FALSE(r.scale_governs);
    REQUIRE(r.backends.size() == 1);
    CHECK_FALSE(r.backends[0].note.effective_ms.has_value());
    CHECK(act.backends[0].summarystr(false, r.backends[0].note.effective_ms) == "Normal");

    // At the leeway edge the row is uncounted, so the frontend decides it.
    act.backends[0].offset_ms = 3.0;
    r = rate_activation(act);
    CHECK(r.cap_clamped);
    CHECK(act.backends[0].summarystr(false, r.backends[0].note.effective_ms) ==
          "Hard (uncounted)");

    // The edge is the user's rule: a 2 ms leeway makes 2.5 ms uncounted.
    const double narrow = 2.0;
    REQUIRE(narrow != core::default_rules().backend_leeway_ms);
    act.backends[0].offset_ms = 2.5;
    r = rate_activation(act, narrow);
    CHECK(r.cap_clamped);
    CHECK(act.backends[0].summarystr(false, r.backends[0].note.effective_ms, narrow) ==
          "Hard (uncounted)");
}

// D269: the Rating word bands the eff. figure when the row's scale side is
// scaled. The rows come from rate_activation, so the figure is the one the
// table shows beside the word.
TEST_CASE("BackendSqueeze::summarystr: a scaled row's word follows its eff. figure") {
    // One backend row at `offset_ms`, squeezed out or plain, under the SP
    // end's stored scale `post`; returns the word the table shows.
    auto word = [](double offset_ms, bool squeezed_out, TransferScale post) {
        Activation act;
        test::set_skips(act, 0);
        act.e_offset = 300.0;  // not e-critical
        BackendSqueeze row;
        row.timecode = Timecode::raw(3000);
        row.offset_ms = offset_ms;
        act.backends.push_back(row);
        if (squeezed_out) act.set_sqout(row.timecode.ticks());
        test::set_transfer(act, TransferScale{}, post);
        const ActivationRating r = rate_activation(act);
        REQUIRE(r.backends.size() == 1);
        REQUIRE(r.backends[0].squeezed_out == squeezed_out);
        return r.backends[0].row.summarystr(squeezed_out, r.backends[0].note.effective_ms);
    };
    // The squeezed-out row 50 ms inside SP needs a 50 ms late hit; the early
    // side of the SP end governs it.
    CHECK(word(-50.0, true, TransferScale{0.5, 1.0}) == "Extreme");
    CHECK(word(-50.0, true, TransferScale{2.0, 1.0}) == "Hard");
    CHECK(word(-50.0, true, TransferScale{}) == "Extreme");
    // A plain row 50 ms past the end is uncounted; the late side governs it.
    CHECK(word(50.0, false, TransferScale{1.0, 2.0}) == "Hard (uncounted)");
}

TEST_CASE("squeeze_budget_ms: identity scale is twice the hit window") {
    // The backend tooltip's normal-scale figure, nominal_budget_ms, is this
    // call at the identity scale.
    CHECK(squeeze_budget_ms(1.0) == 170.0);
}

TEST_CASE("beyond_edge_ms: pinned at the 85 ms hit window") {
    CHECK(beyond_edge_ms() == 170.0);
}

TEST_CASE("rate_activation: an unknown scale rates nothing") {
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;
    // transfer_post left unknown, as copy-out writes it when it can't compute.
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});
    BackendSqueeze row;
    row.offset_ms = 40.0;
    act.backends.push_back(row);

    ActivationRating r = rate_activation(act);
    CHECK_FALSE(r.scales.has_value());
    REQUIRE(r.backends.size() == 1);
    CHECK_FALSE(r.backends[0].note.effective_ms.has_value());
    REQUIRE(r.note_effective_ms.size() == 1);
    CHECK_FALSE(r.note_effective_ms[0].has_value());
    CHECK_FALSE(r.scale_governs);
}
