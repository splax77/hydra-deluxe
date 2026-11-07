// The Path Builder timeline's window (ui/timeline_view): where it starts and
// ends, how each move shifts it and what the scrollbar shows. Pure; no ImGui
// context. Every expected value is a literal pinned from one run.
#include "doctest.h"

#include "ui/timeline_view.h"

using namespace hydra;
using ui::TimelineView;

namespace {

// 42:02 and a half, so a readout that rounded instead of flooring would read
// 42:03.
constexpr double kLongMs = 2522500.0;
// Set the World on Fire's song_length (testdata/input/common/IB24/T5/), the
// GUI tests' long chart.
constexpr double kSetTheWorldMs = 358957.0;
// Burnout's playback end, a song shorter than the window.
constexpr double kShortMs = 130088.0;

// A 3-minute window six minutes into the long song.
constexpr TimelineView kAtSix{360000.0, 180000.0};

}  // namespace

TEST_CASE("timeline view: the window clamps to the song's ends") {
    CHECK(ui::kTimelineWindowMs == 180000.0);

    const TimelineView before = ui::clamp({-5000.0, 180000.0}, kLongMs);
    CHECK(before.start_ms == 0.0);
    CHECK(before.span_ms == 180000.0);
    CHECK(ui::clamp({2500000.0, 180000.0}, kLongMs).start_ms == 2342500.0);
    CHECK(ui::clamp(kAtSix, kLongMs).start_ms == 360000.0);

    // A song shorter than the window gets a window as long as the song, which
    // never moves (the user's answer to Q1).
    const TimelineView short_open = ui::open_view(kShortMs);
    CHECK(short_open.start_ms == 0.0);
    CHECK(short_open.span_ms == 130088.0);
    CHECK(ui::clamp({50000.0, short_open.span_ms}, kShortMs).start_ms == 0.0);
    CHECK(ui::page(short_open, 1, kShortMs).start_ms == 0.0);

    const TimelineView long_open = ui::open_view(kLongMs);
    CHECK(long_open.start_ms == 0.0);
    CHECK(long_open.span_ms == 180000.0);

    // No usable length: the window keeps its full span.
    CHECK(ui::window_span_ms(0.0) == 180000.0);
}

TEST_CASE("timeline view: a page, a wheel notch and a centre move by the mockup's shares") {
    CHECK(ui::page(kAtSix, 1, kLongMs).start_ms == 540000.0);
    CHECK(ui::page(kAtSix, -1, kLongMs).start_ms == 180000.0);
    CHECK(ui::page(kAtSix, -3, kLongMs).start_ms == 0.0);

    CHECK(ui::wheel(kAtSix, 1.0, kLongMs).start_ms == 420000.0);
    CHECK(ui::wheel(kAtSix, -1.0, kLongMs).start_ms == 300000.0);

    CHECK(ui::centre_on(kAtSix, 1000000.0, kLongMs).start_ms == 910000.0);
    CHECK(ui::centre_on(kAtSix, 30000.0, kLongMs).start_ms == 0.0);

    // One page forward from the open on the GUI tests' long chart stops at
    // the length less the window.
    const TimelineView forward = ui::page(ui::open_view(kSetTheWorldMs), 1, kSetTheWorldMs);
    CHECK(forward.start_ms == 178957.0);
}

TEST_CASE("timeline view: the playhead turns a page past three quarters and before the window, and not between") {
    // Half way in: the window stays.
    CHECK(ui::follow_playing(kAtSix, 450000.0, kLongMs).start_ms == 360000.0);
    // 0.76 of the way in: a page turn puts the playhead a quarter in.
    CHECK(ui::follow_playing(kAtSix, 496800.0, kLongMs).start_ms == 451800.0);
    // A whole window before it: the same.
    CHECK(ui::follow_playing(kAtSix, 180000.0, kLongMs).start_ms == 135000.0);
}

TEST_CASE("timeline view: show moves only when the moment is outside") {
    CHECK(ui::show(kAtSix, 450000.0, kLongMs).start_ms == 360000.0);
    CHECK(ui::show(kAtSix, 496800.0, kLongMs).start_ms == 360000.0);
    CHECK(ui::show(kAtSix, 540000.0, kLongMs).start_ms == 360000.0);
    CHECK(ui::show(kAtSix, 600000.0, kLongMs).start_ms == 555000.0);
    CHECK(ui::show(kAtSix, 100000.0, kLongMs).start_ms == 55000.0);
    CHECK(ui::show(kAtSix, 10000.0, kLongMs).start_ms == 0.0);

    CHECK(ui::fraction(kAtSix, 450000.0) == doctest::Approx(0.5));
    CHECK(ui::fraction(kAtSix, 270000.0) == doctest::Approx(-0.5));
    CHECK(ui::in_view(kAtSix, 360000.0));
    CHECK(ui::in_view(kAtSix, 540000.0));
    CHECK_FALSE(ui::in_view(kAtSix, 359999.0));
    CHECK_FALSE(ui::in_view(kAtSix, 540001.0));
}

TEST_CASE("timeline view: ms_at runs fraction backwards") {
    CHECK(ui::ms_at(kAtSix, 0.0) == 360000.0);
    CHECK(ui::ms_at(kAtSix, 0.5) == 450000.0);
    CHECK(ui::ms_at(kAtSix, 1.0) == 540000.0);
    // Not held to the window: a share outside 0..1 is a moment outside it.
    CHECK(ui::ms_at(kAtSix, -0.5) == 270000.0);
    CHECK(ui::ms_at(kAtSix, ui::fraction(kAtSix, 496800.0)) == doctest::Approx(496800.0));
    // The whole song as one window: a share is that share of the song.
    const TimelineView whole = ui::whole_song_view(kLongMs);
    CHECK(whole.start_ms == 0.0);
    CHECK(whole.span_ms == 2522500.0);
    CHECK(ui::ms_at(whole, 0.5) == 1261250.0);
}

TEST_CASE("timeline view: the readout reads start, capped end and length") {
    CHECK(ui::readout(kAtSix, kLongMs) == "6:00 \xE2\x80\x93 9:00 of 42:02");
    CHECK(ui::readout(ui::clamp({9e9, 180000.0}, kLongMs), kLongMs) ==
          "39:02 \xE2\x80\x93 42:02 of 42:02");
    // A 3-minute window over a shorter song reads the song's end.
    CHECK(ui::readout({0.0, 180000.0}, kShortMs) == "0:00 \xE2\x80\x93 2:10 of 2:10");
    CHECK(ui::readout(ui::open_view(kShortMs), kShortMs) == "0:00 \xE2\x80\x93 2:10 of 2:10");
    // 178.957 s reads 2:58, rounded down.
    const TimelineView forward = ui::page(ui::open_view(kSetTheWorldMs), 1, kSetTheWorldMs);
    CHECK(ui::readout(forward, kSetTheWorldMs) == "2:58 \xE2\x80\x93 5:58 of 5:58");
}

TEST_CASE("timeline view: the thumb's shares") {
    const ui::ThumbShares t = ui::thumb(kAtSix, 2400000.0);
    CHECK(t.left == doctest::Approx(0.15));
    CHECK(t.width == doctest::Approx(0.075));

    const ui::ThumbShares whole = ui::thumb(ui::open_view(kShortMs), kShortMs);
    CHECK(whole.left == 0.0);
    CHECK(whole.width == 1.0);

    const ui::ThumbShares none = ui::thumb(kAtSix, 0.0);
    CHECK(none.left == 0.0);
    CHECK(none.width == 0.0);

    // A press beside the thumb moves one window toward it.
    CHECK(ui::scrollbar_click(kAtSix, 0.9, 2400000.0).start_ms == 540000.0);
    CHECK(ui::scrollbar_click(kAtSix, 0.05, 2400000.0).start_ms == 180000.0);
}
