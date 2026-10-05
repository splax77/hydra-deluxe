// Unit tests for core/squeeze_rating: the deactivation node, the frontend
// transfer scales computed from it, and the display-layer judgement built on
// top — materiality, per-row effective ms, and the exact SP-end solver.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "core/squeeze_rating.h"
#include "core/rules.h"
#include "record_fixtures.h"

using namespace hydra;

TEST_CASE("frontend_transfer_scales: measure-rate ratio, both directions") {
    // The Tom Sawyer (Onyxite) shape: ten 7/8 measures at 87.35 BPM, then
    // 7/16 at 85.1 -- no boundary at the query points, so early == late.
    // The activations here carry no backend rows, so this case pins the
    // measure-count fallback path (the deact-node derivation is covered by
    // the Dumpweed regression case below).
    std::map<int64_t, int64_t> tpm{{0, 1680}, {16800, 840}};
    std::map<int64_t, double> bpm{{0, 87.35}, {16800, 85.1}};
    SongTiming st(480, tpm, bpm);

    Activation act;
    act.timecode = st.timecode(0);
    test::set_sp_meter(act, 8);  // 16 measures: 10 of 7/8 + 6 of 7/16 -> tick 21840
    // No SqIn yet: the end is the plain 16-measure end.
    test::set_plain_window(act, 21840);

    // The literal end is the 16-measure end.
    CHECK(st.plusmeasure(act.timecode, 16).ticks() == 21840);

    auto scales = frontend_transfer_scales(act, st);
    REQUIRE(scales.has_value());
    // (1.75 * 87.35) / (3.5 * 85.1) = 0.51322...
    CHECK(scales->post.late == doctest::Approx(0.5132197).epsilon(1e-6));
    CHECK(scales->post.early == doctest::Approx(scales->post.late));
    // No SqIn: no SqIn scales.
    CHECK(scales->sqins.empty());

    // Two SqIns, each a SqIn step two 7/16 measures (840 ticks) further on.
    // Each is measured from the end before it (21840, then 23520); every end
    // stays inside the 7/16 section, so every ratio is unchanged.
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 5.0});
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 6.0});
    act.sp_end_steps = {{0, 21840, SpEndKind::Activation},
                        {21800, 23520, SpEndKind::SqIn},
                        {23500, 25200, SpEndKind::SqIn}};
    auto sqin_scales = frontend_transfer_scales(act, st);
    REQUIRE(sqin_scales.has_value());
    REQUIRE(sqin_scales->sqins.size() == 2);
    CHECK(sqin_scales->sqins[0].late == doctest::Approx(scales->post.late));
    CHECK(sqin_scales->sqins[1].late == doctest::Approx(scales->post.late));
    CHECK(sqin_scales->post.late == doctest::Approx(scales->post.late));

    // A SqOut does not move the end and stores no scale of its own.
    act.sqinouts.clear();
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -5.0});
    test::set_plain_window(act, 21840);
    auto sqout_scales = frontend_transfer_scales(act, st);
    REQUIRE(sqout_scales.has_value());
    CHECK(sqout_scales->sqins.empty());
    CHECK(sqout_scales->post.late == doctest::Approx(scales->post.late));


    // Has both timecode and sp_meter, but no deact_tick: a pre-v4 record
    // cannot say where SP ended, and nothing guesses any more.
    Activation partial;
    partial.timecode = st.timecode(0);
    test::set_sp_meter(partial, 2);
    CHECK(!frontend_transfer_scales(partial, st).has_value());
}

TEST_CASE("frontend_transfer_scales: each SqIn reads its own stored end") {
    using K = SpEndKind;
    // Flat 4/4 at 480: a measure is 1920 ticks. The tempo changes at 9600.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}, {9600, 150.0}};
    SongTiming st(480, tpm, bpm);

    Activation act;
    act.timecode = st.timecode(0);
    test::set_sp_meter(act, 2);
    act.sp_end_steps = {{0, 7680, K::Activation}, {7600, 11520, K::SqIn}};
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 5.0});

    auto scales = frontend_transfer_scales(act, st);
    REQUIRE(scales.has_value());
    REQUIRE(scales->sqins.size() == 1);
    CHECK(scales->sqins[0].late == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(scales->sqins[0].early == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(scales->post.late == doctest::Approx(0.8).epsilon(1e-9));
    CHECK(scales->post.early == doctest::Approx(0.8).epsilon(1e-9));

    // Nothing is stepped back from D any more: a phrase collected after the
    // SqIn moves D, and the SqIn's scale does not change.
    act.sp_end_steps.push_back({13000, 15360, K::Collected});
    auto later = frontend_transfer_scales(act, st);
    REQUIRE(later.has_value());
    REQUIRE(later->sqins.size() == 1);
    CHECK(later->sqins[0].late == doctest::Approx(1.0).epsilon(1e-12));

    // A SqIn with no SqIn step (an old record) is not guessed at.
    act.sp_end_steps = {{0, 11520, K::Activation}};
    CHECK_FALSE(frontend_transfer_scales(act, st).has_value());
}

TEST_CASE("frontend_transfer_scales: a clamp moves the front anchor (D1)") {
    using K = SpEndKind;
    // 120 BPM until 2700, then 60 (finding 28's worked example, at 192 tpq).
    std::map<int64_t, int64_t> tpm{{0, 768}};
    std::map<int64_t, double> bpm{{0, 120.0}, {2700, 60.0}};
    SongTiming st(192, tpm, bpm);

    Activation act;
    act.timecode = st.timecode(2304);
    act.sp_end_steps = {{2304, 6144, K::Activation}};
    auto unclamped = frontend_transfer_scales(act, st);
    REQUIRE(unclamped.has_value());
    CHECK(unclamped->post.late == doctest::Approx(2.0).epsilon(1e-12));

    act.sp_end_steps = {{2304, 5376, K::Activation}, {3072, 6144, K::Clamped}};
    auto clamped = frontend_transfer_scales(act, st);
    REQUIRE(clamped.has_value());
    CHECK(clamped->post.late == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(clamped->post.early == doctest::Approx(1.0).epsilon(1e-12));
}

TEST_CASE("transfer_scale_between: a measure that is not a positive finite length is refused") {
    // The load guard keeps such timing out of every chart (Task 3); a
    // hand-built SongTiming is not guarded, so this pins the function's own
    // refusal on both sides. A 0 BPM section makes a measure last forever,
    // a negative one makes it last negative time.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    SongTiming forever(480, tpm, std::map<int64_t, double>{{0, 120.0}, {7680, 0.0}});
    CHECK(transfer_scale_between(0, 3840, forever).has_value());
    CHECK_FALSE(transfer_scale_between(0, 9600, forever).has_value());     // the end side
    CHECK_FALSE(transfer_scale_between(9600, 11520, forever).has_value()); // the front side
    SongTiming backwards(480, tpm, std::map<int64_t, double>{{0, 120.0}, {7680, -120.0}});
    CHECK_FALSE(transfer_scale_between(0, 9600, backwards).has_value());
}

TEST_CASE("frontend_transfer_scales: direction-dependent at boundaries") {
    // Synthetic boundary-on-both-ends shape (like As I Am's m81 activation):
    // the activation sits exactly on a 6/4 -> 7/8 change and its SP end
    // exactly on a 7/8 -> 9/8 change, with a tempo change on the end tick
    // too. Early (-) hits move into the long 6/4 measure; late (+) hits into
    // the 7/8 one -- two different ratios.
    std::map<int64_t, int64_t> tpm{{0, 2880}, {2880, 1680}, {12960, 2160}};
    std::map<int64_t, double> bpm{{0, 130.0}, {11280, 133.0}, {12960, 129.0}};
    SongTiming st(480, tpm, bpm);

    Activation act;
    act.timecode = st.timecode(2880);
    test::set_sp_meter(act, 3);  // 6 measures of 7/8 -> end tick 12960
    // No SqIn: the plain act + 2*B-measure end, as a literal tick.
    test::set_plain_window(act, 12960);

    CHECK(st.plusmeasure(act.timecode, 6).ticks() == 12960);

    auto scale = frontend_transfer_scales(act, st);
    REQUIRE(scale.has_value());
    // early: (3.5 * 130) / (6 * 133) = 0.570175...
    CHECK(scale->post.early == doctest::Approx(0.5701754).epsilon(1e-6));
    // late: (4.5 * 130) / (3.5 * 129) = 1.295681...
    CHECK(scale->post.late == doctest::Approx(1.2956811).epsilon(1e-6));

    // The Dumpweed shape: constant 4/4, tempo changes exactly on both the
    // activation tick and the SP end tick. No backend rows here, so this is
    // the plain act + 2*B-measure fallback; the REAL Dumpweed activation
    // collects a phrase mid-SP and lands its deact node 2 measures later --
    // that shape is pinned by the regression case below.
    std::map<int64_t, int64_t> tpm44{{0, 1920}};
    std::map<int64_t, double> bpm2{{0, 98.0},   {1920, 97.5},
                                   {8640, 110.0}, {9600, 102.0}};
    SongTiming st2(480, tpm44, bpm2);

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
    SongTiming flat(480, tpm44, bpm120);
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

TEST_CASE("field fixture: What's My Age Again? (Sync Chart) SqOut") {
    // Field-verified: Hoph2o's Sync Chart, Expert Pro Drums 2x, path
    // "1- 1 0". The activation at m31.1.0 (tick 57600) sits exactly on a
    // 155 -> 160 BPM change; the 3-bar SP end at m37.1.0 (tick 69120) sits
    // exactly on a 157 -> 160 change. The SqOut note is the last note of the
    // SP phrase, 240 ticks before the end, inside the 157 section. A player
    // at 130% speed hit the frontend 75.4 real ms early and the note 72.3
    // real ms late and the squeeze still failed -- by the ~0.25 ms the
    // frontend transfer scale (x0.9873) removes from the early arm.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{
        {0, 155.0}, {57600, 160.0}, {67200, 157.0}, {69120, 160.0}};
    SongTiming st(480, tpm, bpm);

    Activation act;
    act.timecode = st.timecode(57600);
    test::set_sp_meter(act, 3);  // 6 measures -> tick 69120
    // This activation collects no phrase mid-SP, so its SP end is the plain
    // 6-measure end, stated as a literal tick.
    test::set_plain_window(act, 69120);
    CHECK(st.plusmeasure(act.timecode, 6).ticks() == 69120);

    auto scales = frontend_transfer_scales(act, st);
    REQUIRE(scales.has_value());
    CHECK(scales->post.early == doctest::Approx(0.987263).epsilon(1e-6));
    CHECK(scales->post.late == doctest::Approx(1.0).epsilon(1e-12));

    // This activation collects no phrase mid-SP (its deact node IS the plain
    // 6-measure end), so the D-anchored derivation from a 0.0-offset backend
    // row agrees with the fallback exactly -- field-verified cross-check.
    Activation with_backend = act;
    BackendSqueeze d0;
    d0.timecode = st.timecode(69120);
    d0.offset_ms = 0.0;
    with_backend.backends.push_back(d0);
    auto anchored = frontend_transfer_scales(with_backend, st);
    REQUIRE(anchored.has_value());
    CHECK(anchored->post.early == doctest::Approx(scales->post.early).epsilon(1e-12));
    CHECK(anchored->post.late == doctest::Approx(scales->post.late).epsilon(1e-12));

    // The gap: 240 ticks at 157 BPM.
    double gap = st.timecode(69120).ms() - st.timecode(68880).ms();
    CHECK(gap == doctest::Approx(191.0825).epsilon(1e-5));

    SPSqueeze sqout{SqueezeKind::SqOut, -gap};
    const double r = scales->post.early;
    CHECK(sqout.difficulty() == doctest::Approx(191.0825).epsilon(1e-5));
    CHECK(effective_backend_ms(sqout.difficulty(), r) == doctest::Approx(192.308).epsilon(1e-4));
    CHECK(squeeze_budget_ms(r, 85.0) == doctest::Approx(168.92).epsilon(1e-4));
    CHECK(squeeze_budget_ms(r, 70.0) == doctest::Approx(139.11).epsilon(1e-4));

    // The failed 130% attempt: real-ms displacements scale by 1.3 into chart
    // ms; the early arm is then discounted by r. 75.4/72.3 misses the gap by
    // a quarter of a chart ms; 78/75 covers it.
    double failed = 75.4 * 1.3 * r + 72.3 * 1.3;
    double landed = 78.0 * 1.3 * r + 75.0 * 1.3;
    CHECK(failed < gap);
    CHECK(failed == doctest::Approx(190.76).epsilon(1e-3));
    CHECK(landed > gap);

    // The description keeps the legacy single-hit line.
    CHECK(sqout.description() == "SqOut: Note timing must be later than 191.1ms.");
    SPSqueeze easy{SqueezeKind::SqOut, 5.0};
    CHECK(easy.description() == "SqOut: Note timing must be later than -5.0ms.");

    // Stored transfer scales are display-only: difficulty stays the raw gap.
    Activation stamped = act;
    test::set_skips(stamped, 1);
    stamped.e_offset = 300.0;  // not e-critical
    stamped.sqinouts.push_back(sqout);
    test::set_transfer(stamped, TransferScale{r, 1.0});
    REQUIRE(stamped.difficulty().has_value());
    CHECK(*stamped.difficulty() == doctest::Approx(191.0825).epsilon(1e-5));
    CHECK(stamped.is_difficult());
}

TEST_CASE("activation_deact_tick: the stored node, read back") {
    // activation_deact_tick derives nothing any more: it just hands back
    // act.deact_tick(). No SongTiming is needed to test that.

    // Set: returns exactly what was stored.
    Activation act;
    test::set_plain_window(act, 46080);
    CHECK(activation_deact_tick(act) == 46080);

    // A timecode and sp_meter at hand but no deact_tick: nullopt. This is the
    // point of the change -- there is no measure-count fallback any more.
    Activation stale;
    stale.timecode = Timecode::raw(3840);
    test::set_sp_meter(stale, 2);
    CHECK(!activation_deact_tick(stale).has_value());

    // A bare, default-constructed activation: nullopt.
    Activation bare;
    CHECK(!activation_deact_tick(bare).has_value());
}

TEST_CASE("field fixture: Dumpweed SqOut end anchored on the deact node") {
    // Field-verified: blink-182 - Dumpweed (Hoph2o), Expert Pro Drums 2x,
    // activation "1-" at m19.1.0 (tick 34560), sp_meter 2. The player
    // collects ONE SP phrase mid-SP, so the search's deact node sits at tick
    // 46080 (act + 6 measures), not the plain act + 4 (42240) -- and a tempo
    // change on each tick makes the difference visible: the old short-end
    // reconstruction said early x0.890911, the true end gives x0.935114.
    // An FC video shows frontend -74.1 ms + SqOut backend +75.1 ms landing
    // the squeeze on the 143.129 ms gap: 74.1*0.935114 + 75.1 = 144.4 >=
    // 143.129, while the old scale predicts a miss (141.1). DragonDelgar's
    // published even split of +-74 = 143.129 / (1 + 0.935114) matches the
    // true end too.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 98.0003},
                                  {34560, 97.4999},
                                  {45120, 104.8004},
                                  {46080, 101.0002}};
    SongTiming st(480, tpm, bpm);

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
    // The budget at this scale for a 70 ms window, pinned from one run.
    CHECK(squeeze_budget_ms(r, 70.0) == doctest::Approx(135.458).epsilon(1e-4));
    CHECK(74.1 * r + 75.1 > gap);         // the video's successful split
    CHECK(74.1 * 0.890911 + 75.1 < gap);  // the old scale called it a miss

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

TEST_CASE("is_scaled: exactly 1 up to float noise, nothing coarser") {
    CHECK_FALSE(is_scaled(1.0));
    CHECK_FALSE(is_scaled(1.0 + 1e-12));
    CHECK(is_scaled(0.999));
    CHECK(is_scaled(1.0001));
    // D14: the tolerance is 1e-9, and the scale line's digit cap follows it.
    CHECK(kScaleIdentityTolerance == 1e-9);
    CHECK(kScaleIdentityDigits == 9);
}

TEST_CASE("rate_note: the side of the end picks the multiplier") {
    const TransferScale s{0.5, 2.0};
    // Inside SP: an early hit is what moves the end across it.
    NoteRating in = rate_note(-50.0, true, s, 85.0);
    CHECK(in.early);
    CHECK(in.scale == 0.5);
    CHECK(in.budget_ms == 127.5);
    REQUIRE(in.effective_ms.has_value());
    CHECK(*in.effective_ms == doctest::Approx(66.666666666666667).epsilon(1e-12));
    // Outside: a late hit.
    NoteRating out = rate_note(50.0, false, s, 85.0);
    CHECK_FALSE(out.early);
    CHECK(out.scale == 2.0);
    REQUIRE(out.effective_ms.has_value());
    CHECK(*out.effective_ms == doctest::Approx(33.333333333333336).epsilon(1e-12));
}

TEST_CASE("rate_note: a figure whenever the multiplier is not 1, however close") {
    // Sinner's Vengeance act 3 shape: 187.5 ms at an early scale just under
    // 1 moves by under 1 ms, which the old 1 ms floor hid. Full precision.
    const double r = 0.9912;
    NoteRating n = rate_note(-187.5, true, TransferScale{r, 1.0}, 85.0);
    REQUIRE(n.effective_ms.has_value());
    CHECK(*n.effective_ms == doctest::Approx(188.32864604258737).epsilon(1e-12));
    // Exactly 1, or float noise around it: no figure.
    CHECK_FALSE(rate_note(-187.5, true, TransferScale{1.0, 1.0}, 85.0).effective_ms.has_value());
    CHECK_FALSE(
        rate_note(-187.5, true, TransferScale{1.0 + 1e-12, 1.0}, 85.0).effective_ms.has_value());
    // A note on the end itself keeps its figure when scaled: 0 ms.
    NoteRating z = rate_note(0.0, true, TransferScale{8.59, 8.76}, 85.0);
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
    add(1002, 1.5);    // counted inside the leeway: early (decision 6)
    add(1003, 50.0);   // uncounted, past the leeway: late

    ActivationRating r = rate_activation(act, 85.0);
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

    ActivationRating r = rate_activation(act, 85.0);
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
    r = rate_activation(act, 85.0);
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
    ActivationRating r = rate_activation(act, 85.0);
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
    ActivationRating r = rate_activation(act, 85.0);
    REQUIRE(r.note_effective_ms.size() == 2);
    REQUIRE(r.note_effective_ms[0].has_value());
    REQUIRE(r.note_effective_ms[1].has_value());
    // Sun of Nothing act 5: 400 ms free at early x1.60 is worth 307.7 ms.
    CHECK(*r.note_effective_ms[0] == doctest::Approx(307.6923076923077).epsilon(1e-12));
    CHECK(*r.note_effective_ms[1] == doctest::Approx(66.666666666666667).epsilon(1e-12));
    CHECK(r.scale_governs);
}

TEST_CASE("rate_activation: a SqIn on the SP end is free, so it reads its SqIn scale's early side (D13)") {
    // Only one side scaled, so the figure shows which side was read.
    auto figure = [](TransferScale sqin_scale, double offset) {
        Activation act;
        test::set_skips(act, 0);
        act.e_offset = 300.0;
        act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, offset});
        test::set_transfer(act, sqin_scale, TransferScale{});
        ActivationRating r = rate_activation(act, 85.0);
        REQUIRE(r.note_effective_ms.size() == 1);
        return r.note_effective_ms[0];
    };
    const TransferScale early_only{1.6, 1.0};
    const TransferScale late_only{1.0, 1.6};
    // Exactly on the end: free, early side.
    REQUIRE(figure(early_only, 0.0).has_value());
    CHECK(*figure(early_only, 0.0) == 0.0);
    CHECK_FALSE(figure(late_only, 0.0).has_value());
    // Just before the end: free, early side.
    REQUIRE(figure(early_only, -0.5).has_value());
    CHECK(*figure(early_only, -0.5) == doctest::Approx(0.38461538461538464).epsilon(1e-12));
    CHECK_FALSE(figure(late_only, -0.5).has_value());
    // Just past the end: still to earn, late side.
    CHECK_FALSE(figure(early_only, 0.5).has_value());
    REQUIRE(figure(late_only, 0.5).has_value());
    CHECK(*figure(late_only, 0.5) == doctest::Approx(0.38461538461538464).epsilon(1e-12));
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
    ActivationRating r = rate_activation(act, 85.0);
    CHECK_FALSE(r.scale_governs);
    CHECK_FALSE(r.backends[0].note.effective_ms.has_value());

    // A flat activation never governs, even with a gap past the budget.
    Activation flat = act;
    test::set_transfer(flat, TransferScale{});
    flat.backends[0].offset_ms = 222.2;
    CHECK_FALSE(rate_activation(flat, 85.0).scale_governs);
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

    ActivationRating r = rate_activation(act, 85.0);
    REQUIRE(r.scales.has_value());
    CHECK(r.scales->post.late == doctest::Approx(0.5));
    CHECK(r.scales->post.early == doctest::Approx(0.5));
    CHECK(r.scale_governs);
}

TEST_CASE("timing_tiers: the ladder at W=85 and at W=70") {
    // The default 85 ms window -> a 170 ms two-hit budget: the 2 ms "Normal"
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

    CHECK(std::string(t85[3].name) == "Insane");
    CHECK(std::string(t85[3].tok) == "t3");
    REQUIRE(t85[3].cutoff.has_value());
    CHECK(*t85[3].cutoff == doctest::Approx(127.5));

    CHECK(std::string(t85[4].name) == "Insane+");
    CHECK(std::string(t85[4].tok) == "t4");
    REQUIRE(t85[4].cutoff.has_value());
    CHECK(*t85[4].cutoff == doctest::Approx(170.0));

    // The two open bands carry no cutoff, and "None" is last.
    CHECK(std::string(t85[5].name) == "Beyond");
    CHECK(std::string(t85[5].tok) == "t5");
    CHECK_FALSE(t85[5].cutoff.has_value());
    CHECK(std::string(t85[6].name) == "None");
    CHECK(std::string(t85[6].tok) == "tn");
    CHECK_FALSE(t85[6].cutoff.has_value());

    // The historical 70 ms window reproduces the original 2/35/70/105/140
    // ladder exactly; only the cutoffs move.
    std::vector<TimingTier> t70 = timing_tiers(70.0);
    REQUIRE(t70.size() == 7);
    const double want70[5] = {2.0, 35.0, 70.0, 105.0, 140.0};
    for (size_t i = 0; i < 5; ++i) {
        CHECK(std::string(t70[i].name) == std::string(t85[i].name));
        CHECK(std::string(t70[i].tok) == std::string(t85[i].tok));
        REQUIRE(t70[i].cutoff.has_value());
        CHECK(*t70[i].cutoff == doctest::Approx(want70[i]));
    }
    CHECK_FALSE(t70[5].cutoff.has_value());
    CHECK_FALSE(t70[6].cutoff.has_value());
}

TEST_CASE("timing_tiers: the Insane+ cutoff is the identity squeeze budget") {
    // The ladder's top cutoff is the two-hit budget itself, read from its
    // one owner, not twice the window written again.
    std::vector<TimingTier> t85 = timing_tiers(kDefaultHitWindowMs);
    REQUIRE(t85.size() == 7);
    CHECK(std::string(t85[4].name) == "Insane+");
    REQUIRE(t85[4].cutoff.has_value());
    CHECK(*t85[4].cutoff == nominal_budget_ms(kDefaultHitWindowMs));
    CHECK(*t85[4].cutoff == 170.0);

    std::vector<TimingTier> t70 = timing_tiers(70.0);
    REQUIRE(t70.size() == 7);
    REQUIRE(t70[4].cutoff.has_value());
    CHECK(*t70[4].cutoff == nominal_budget_ms(70.0));

    // Beyond and None still carry no cutoff.
    CHECK_FALSE(t85[5].cutoff.has_value());
    CHECK_FALSE(t85[6].cutoff.has_value());
    CHECK_FALSE(t70[5].cutoff.has_value());
    CHECK_FALSE(t70[6].cutoff.has_value());
}

TEST_CASE("effective_backend_ms: the factor of two is the identity budget over the scaled budget") {
    // At the identity scale the figure is the raw gap: the budget there is
    // 170.0 ms at the default window, the same as the nominal scale.
    CHECK(squeeze_budget_ms(1.0, kDefaultHitWindowMs) == 170.0);
    CHECK(effective_backend_ms(170.0, 1.0) == 170.0);
    // A scaled row is pinned against the owner in the Dumpweed field
    // fixture above, at the scale that fixture computes.
}

TEST_CASE("display_backends: 500 ms window keeps everything the 500 ms search graph collects") {
    // The display window IS the search graph's squeeze window (one constant,
    // kSqueezeWindowMs), so nothing the graph gathers gets trimmed at
    // store/display time. 180 and -300 are the regression: both used to fall
    // outside the old +-170 window and get dropped.
    CHECK(kSqueezeWindowMs == 500.0);
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}};
    SongTiming st(480, tpm, bpm);

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
    ActivationRating r1 = rate_activation(clamped_with_squeeze, 85.0);
    CHECK(r1.cap_clamped);

    // clamp_tick set, but the only backend row is at -30 ms -- inside the
    // SP window, so the engine counts it and it isn't a squeeze the frontend
    // decides. No SqIn/SqOut either, so cap_clamped stays false.
    Activation clamped_no_squeeze;
    test::set_clamped_window(clamped_no_squeeze, 3072, 6144);
    BackendSqueeze mild_row;
    mild_row.offset_ms = -30.0;
    clamped_no_squeeze.backends.push_back(mild_row);
    ActivationRating r2 = rate_activation(clamped_no_squeeze, 85.0);
    CHECK_FALSE(r2.cap_clamped);

    // A SqOut with no clamp_tick at all -- the window was never cap-clamped,
    // so there's nothing to warn about regardless of the squeeze.
    Activation unclamped_with_squeeze;
    unclamped_with_squeeze.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -50.0});
    ActivationRating r3 = rate_activation(unclamped_with_squeeze, 85.0);
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
// is not a squeeze the frontend decides. The rating, the late-row warn and
// the overfill flag all use that same edge (user decision 7).
TEST_CASE("rate_activation: a plain row inside the leeway is not a frontend squeeze") {
    Activation act;
    test::set_skips(act, 0);
    act.e_offset = 300.0;  // not e-critical
    // The row below sits 2.5 ms past the end; 3077 is any end before it.
    test::set_clamped_window(act, 3072, 3077);
    // A late scale this small would make a row treated as late material:
    // 2.5 ms reads as 4.2 ms, past the 1 ms impact floor.
    test::set_transfer(act, TransferScale{}, TransferScale{1.0, 0.2});
    BackendSqueeze row;
    row.timecode = Timecode::raw(3080);
    row.offset_ms = 2.5;
    act.backends.push_back(row);

    ActivationRating r = rate_activation(act, 85.0);
    CHECK_FALSE(r.cap_clamped);
    // Counted inside the leeway, so the early side governs it -- x1.00 here.
    CHECK_FALSE(r.scale_governs);
    REQUIRE(r.backends.size() == 1);
    CHECK_FALSE(r.backends[0].note.effective_ms.has_value());
    CHECK(act.backends[0].summarystr(false, 85.0) == "Standard");

    // At the leeway edge the row is uncounted, so the frontend decides it.
    act.backends[0].offset_ms = 3.0;
    r = rate_activation(act, 85.0);
    CHECK(r.cap_clamped);
    CHECK(act.backends[0].summarystr(false, 85.0) == "Hard (uncounted)");

    // The edge is the user's rule: a 2 ms leeway makes 2.5 ms uncounted.
    const double narrow = 2.0;
    REQUIRE(narrow != core::default_rules().backend_leeway_ms);
    act.backends[0].offset_ms = 2.5;
    r = rate_activation(act, 85.0, narrow);
    CHECK(r.cap_clamped);
    CHECK(act.backends[0].summarystr(false, 85.0, narrow) == "Hard (uncounted)");
}

TEST_CASE("squeeze_budget_ms: identity scale is twice the hit window") {
    // The backend tooltip's "not %.0fms" figure is this call, not 2.0 * W.
    CHECK(squeeze_budget_ms(1.0, 85.0) == 170.0);
    CHECK(squeeze_budget_ms(1.0, 40.0) == 80.0);
}

TEST_CASE("beyond_edge_ms: pinned at W=85 and W=40") {
    CHECK(beyond_edge_ms(85.0) == 170.0);
    CHECK(beyond_edge_ms(40.0) == 80.0);
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

    ActivationRating r = rate_activation(act, 85.0);
    CHECK_FALSE(r.scales.has_value());
    REQUIRE(r.backends.size() == 1);
    CHECK_FALSE(r.backends[0].note.effective_ms.has_value());
    REQUIRE(r.note_effective_ms.size() == 1);
    CHECK_FALSE(r.note_effective_ms[0].has_value());
    CHECK_FALSE(r.scale_governs);
}
