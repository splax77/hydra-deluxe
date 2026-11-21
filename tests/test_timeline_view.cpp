// The Path Builder timeline's window (ui/timeline_view): where it starts and
// ends, how each move shifts it and what the scrollbar shows. Pure; no ImGui
// context. The window rules run on every corpus chart's real length, the one
// app::song_length_ms gives the screen, and each expected value is a rule
// stated over those inputs: the shares come from timeline_view.h, the ends
// from ui::clamp and the song's share from app::song_fraction. Only the
// readout case pins text, because the text is what the screen shows.
#include "doctest.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"      // read_chart_timing_meta, the chart's stated length
#include "app/preview_view.h"  // has_song_length, song_fraction
#include "app/song_length.h"   // song_length_ms, the length the timeline gets
#include "ui/timeline_view.h"
#include "corpus_util.h"

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

// Two shares this close are the same share: far below one millisecond of any
// corpus song, far above a double's rounding.
constexpr double kShareGap = 1e-9;

// One corpus chart and its length as the Path Builder's timeline gets it:
// app::song_length_ms over the chart's own metadata and its parsed song, the
// length PreviewController::playback_end_ms hands the window.
struct CorpusLength {
    std::string chart;
    double ms = 0.0;
};

const std::vector<CorpusLength>& corpus_lengths() {
    static const std::vector<CorpusLength> lengths = [] {
        std::vector<CorpusLength> out;
        for (const std::string& path : corpus::chart_paths()) {
            const std::optional<double> ms = app::song_length_ms(
                app::read_chart_timing_meta(path), corpus::song(path, ChartReading{}));
            // A corpus chart the screen could give no window is a broken
            // corpus, not a case to pass over.
            if (!ms) throw std::runtime_error("no song length for " + path);
            out.push_back(CorpusLength{path, *ms});
        }
        return out;
    }();
    return lengths;
}

// Does a window from `start_ms`, `span_ms` long, lie inside a song
// `length_ms` long, so no clamp moves it?
bool fits(double start_ms, double span_ms, double length_ms) {
    return start_ms >= 0.0 && start_ms + span_ms <= length_ms;
}

// Where `view` starts once moved to `start_ms` and held to the song's ends by
// ui::clamp, which the clamp case checks on its own.
double clamped_start(TimelineView view, double start_ms, double length_ms) {
    view.start_ms = start_ms;
    return ui::clamp(view, length_ms).start_ms;
}

// The windows a song's timeline passes through: the one it opens with, one
// page on, and one centred on the song's middle.
std::vector<TimelineView> views_over(double length_ms) {
    const TimelineView open = ui::open_view(length_ms);
    return {open, ui::page(open, 1, length_ms), ui::centre_on(open, length_ms / 2.0, length_ms)};
}

// Playhead moments inside the song spread around `view`: before it, at its
// start, half way, at the turn share and a millisecond past it, at its end
// and after it.
std::vector<double> moments_around(TimelineView view, double length_ms) {
    std::vector<double> out;
    for (const double ms : {ui::ms_at(view, -0.5), view.start_ms, ui::ms_at(view, 0.5),
                            ui::ms_at(view, ui::kFollowTurnShare),
                            ui::ms_at(view, ui::kFollowTurnShare) + 1.0, ui::ms_at(view, 1.0),
                            ui::ms_at(view, 1.5)}) {
        if (ms >= 0.0 && ms <= length_ms) out.push_back(ms);
    }
    return out;
}

// `moved` is `view` turned so `ms` sits kPlayheadLeadShare in, held to the
// song's ends. Counts in `at_lead` each turn no end held back, where the
// playhead is exactly the lead share in.
void check_led_to(TimelineView view, TimelineView moved, double ms, double length_ms,
                  int& at_lead) {
    const double lead_start = ms - view.span_ms * ui::kPlayheadLeadShare;
    CHECK(moved.span_ms == view.span_ms);
    CHECK(moved.start_ms == clamped_start(view, lead_start, length_ms));
    if (fits(lead_start, view.span_ms, length_ms)) {
        ++at_lead;
        CHECK(ui::fraction(moved, ms) == doctest::Approx(ui::kPlayheadLeadShare).epsilon(kShareGap));
    }
}

}  // namespace

TEST_CASE("timeline view: the window clamps to the song's ends") {
    int longer = 0;
    int shorter = 0;
    for (const CorpusLength& c : corpus_lengths()) {
        INFO(c.chart);
        const double length = c.ms;
        const TimelineView open = ui::open_view(length);
        CHECK(open.start_ms == 0.0);
        CHECK(open.span_ms <= length);
        CHECK(open.span_ms <= ui::kTimelineWindowMs);
        if (length > ui::kTimelineWindowMs) {
            ++longer;
            CHECK(open.span_ms == ui::kTimelineWindowMs);
        } else {
            ++shorter;
            // A song shorter than the window: the window is the whole song,
            // and nothing moves it.
            CHECK(open.span_ms == length);
            CHECK(ui::page(open, 1, length).start_ms == 0.0);
            CHECK(ui::page(open, -1, length).start_ms == 0.0);
            CHECK(ui::wheel(open, 1.0, length).start_ms == 0.0);
            // A full-size window over the short song starts at its start.
            CHECK(ui::clamp({length / 2.0, ui::kTimelineWindowMs}, length).start_ms == 0.0);
        }

        const double span = open.span_ms;
        for (const double start : {-span, -1.0, 0.0, length / 2.0, length - span,
                                   length - span + 1.0, length, length + span}) {
            const TimelineView held = ui::clamp({start, span}, length);
            CHECK(held.span_ms == span);
            CHECK(held.start_ms >= 0.0);
            CHECK(held.start_ms <= length - span);
            if (fits(start, span, length)) {
                CHECK(held.start_ms == start);
            } else if (start < 0.0) {
                CHECK(held.start_ms == 0.0);
            } else {
                CHECK(held.start_ms == length - span);
            }
        }
    }
    // The corpus holds songs on both sides of the window's length: Green Day's
    // Burnout states 130303 ms in its song.ini, Lightning Bolt's Dream Genie
    // 387556 ms.
    CHECK(longer > 0);
    CHECK(shorter > 0);

    // No usable length: the window keeps its full span.
    CHECK_FALSE(app::has_song_length(0.0));
    CHECK(ui::window_span_ms(0.0) == ui::kTimelineWindowMs);
}

TEST_CASE("timeline view: a page, a wheel notch and a centre move by the mockup's shares") {
    int free_pages = 0;
    int free_notches = 0;
    for (const CorpusLength& c : corpus_lengths()) {
        INFO(c.chart);
        const double length = c.ms;
        for (const TimelineView& view : views_over(length)) {
            // A page moves the window n whole windows, then the song's ends
            // hold it.
            for (const int n : {-3, -1, 1, 3}) {
                const double to = view.start_ms + n * view.span_ms;
                const TimelineView paged = ui::page(view, n, length);
                CHECK(paged.span_ms == view.span_ms);
                CHECK(paged.start_ms == clamped_start(view, to, length));
                if (fits(to, view.span_ms, length)) ++free_pages;
            }
            // A wheel notch moves it kWheelWindowShare of a window, later in
            // the song for a positive notch.
            for (const double notches : {-1.0, 1.0}) {
                const double to = view.start_ms + notches * view.span_ms * ui::kWheelWindowShare;
                const TimelineView turned = ui::wheel(view, notches, length);
                CHECK(turned.span_ms == view.span_ms);
                CHECK(turned.start_ms == clamped_start(view, to, length));
                if (fits(to, view.span_ms, length)) ++free_notches;
            }
            // Centring puts the moment half way across, unless an end holds
            // the window back.
            for (const double ms : {0.0, length / 4.0, length / 2.0, length}) {
                const double to = ms - view.span_ms / 2.0;
                const TimelineView centred = ui::centre_on(view, ms, length);
                CHECK(centred.start_ms == clamped_start(view, to, length));
                if (fits(to, view.span_ms, length)) {
                    CHECK(ui::fraction(centred, ms) == doctest::Approx(0.5).epsilon(kShareGap));
                }
            }
        }
    }
    // Songs past two windows long page freely from the open (Dream Genie
    // states 387556 ms), and songs past 240 s turn a notch freely from it.
    CHECK(free_pages > 0);
    CHECK(free_notches > 0);
}

TEST_CASE("timeline view: the playhead turns a page past three quarters and before the window, and not between") {
    int at_lead = 0;
    int longer = 0;
    for (const CorpusLength& c : corpus_lengths()) {
        INFO(c.chart);
        const double length = c.ms;
        for (const TimelineView& view : views_over(length)) {
            for (const double ms : moments_around(view, length)) {
                const double share = ui::fraction(view, ms);
                const TimelineView moved = ui::follow_playing(view, ms, length);
                if (share >= 0.0 && share <= ui::kFollowTurnShare) {
                    CHECK(moved.start_ms == view.start_ms);
                    CHECK(moved.span_ms == view.span_ms);
                } else {
                    check_led_to(view, moved, ms, length, at_lead);
                }
            }
        }
        // The exact turn share still leaves the window; a millisecond past it
        // turns the page. The open window of a long song starts at 0 and is
        // kTimelineWindowMs long, so the turn share's moment is exact.
        if (length > ui::kTimelineWindowMs) {
            ++longer;
            const TimelineView open = ui::open_view(length);
            const double turn_ms = ui::ms_at(open, ui::kFollowTurnShare);
            CHECK(ui::follow_playing(open, turn_ms, length).start_ms == open.start_ms);
            CHECK(ui::follow_playing(open, turn_ms + 1.0, length).start_ms > open.start_ms);
        }
    }
    CHECK(longer > 0);
    // A turn a millisecond past three quarters of the open window puts the
    // window 90001 ms in, which every song past 270001 ms leaves unheld (Set
    // the World on Fire states 358957 ms).
    CHECK(at_lead > 0);
}

TEST_CASE("timeline view: show moves only when the moment is outside") {
    int at_lead = 0;
    int longer = 0;
    for (const CorpusLength& c : corpus_lengths()) {
        INFO(c.chart);
        const double length = c.ms;
        for (const TimelineView& view : views_over(length)) {
            for (const double ms : moments_around(view, length)) {
                const TimelineView moved = ui::show(view, ms, length);
                if (ui::in_view(view, ms)) {
                    CHECK(moved.start_ms == view.start_ms);
                    CHECK(moved.span_ms == view.span_ms);
                } else {
                    check_led_to(view, moved, ms, length, at_lead);
                }
            }
        }
        // The window's own ends count as inside; a millisecond beyond either
        // does not. The open window of a long song has whole-millisecond
        // ends, 0 and kTimelineWindowMs.
        if (length > ui::kTimelineWindowMs) {
            ++longer;
            const TimelineView open = ui::open_view(length);
            const double end_ms = open.start_ms + open.span_ms;
            CHECK(ui::in_view(open, open.start_ms));
            CHECK(ui::in_view(open, end_ms));
            CHECK_FALSE(ui::in_view(open, open.start_ms - 1.0));
            CHECK_FALSE(ui::in_view(open, end_ms + 1.0));
        }
    }
    CHECK(longer > 0);
    // A seek half a window before the page-on window of a song longer than
    // 315 s lands unheld a quarter in (Set the World on Fire states 358957 ms).
    CHECK(at_lead > 0);
}

TEST_CASE("timeline view: ms_at runs fraction backwards") {
    for (const CorpusLength& c : corpus_lengths()) {
        INFO(c.chart);
        const double length = c.ms;
        for (const TimelineView& view : views_over(length)) {
            CHECK(ui::ms_at(view, 0.0) == view.start_ms);
            // Not held to the window: a share outside 0..1 is a moment
            // outside it, and fraction reads it back.
            for (const double share : {-0.5, 0.0, 0.25, 0.5, 1.0, 1.5}) {
                CHECK(ui::fraction(view, ui::ms_at(view, share)) ==
                      doctest::Approx(share).epsilon(kShareGap));
            }
            for (const double ms : {0.0, length / 3.0, length}) {
                CHECK(ui::ms_at(view, ui::fraction(view, ms)) == doctest::Approx(ms).epsilon(kShareGap));
            }
        }
        // The whole song as one window: a share across it is that share of
        // the song, as the scrubber's app::song_fraction reads the song.
        const TimelineView whole = ui::whole_song_view(length);
        CHECK(whole.start_ms == 0.0);
        CHECK(whole.span_ms == length);
        for (const double share : {0.0, 0.25, 0.5, 1.0}) {
            const std::optional<double> back = app::song_fraction(ui::ms_at(whole, share), length);
            REQUIRE(back.has_value());
            CHECK(*back == doctest::Approx(share).epsilon(kShareGap));
        }
    }
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
    int clicked = 0;
    for (const CorpusLength& c : corpus_lengths()) {
        INFO(c.chart);
        const double length = c.ms;
        std::vector<TimelineView> views = views_over(length);
        // A full-size window over a short song: the thumb fills the bar and
        // no more.
        if (length < ui::kTimelineWindowMs) views.push_back(TimelineView{0.0, ui::kTimelineWindowMs});
        for (const TimelineView& view : views) {
            // The thumb spans the window's stretch of the song, as the
            // scrubber's app::song_fraction reads the song.
            const ui::ThumbShares t = ui::thumb(view, length);
            const std::optional<double> left = app::song_fraction(view.start_ms, length);
            const std::optional<double> right =
                app::song_fraction(view.start_ms + view.span_ms, length);
            REQUIRE(left.has_value());
            REQUIRE(right.has_value());
            CHECK(t.left == doctest::Approx(*left).epsilon(kShareGap));
            CHECK(t.left + t.width == doctest::Approx(*right).epsilon(kShareGap));

            // A press beside the thumb moves one window toward the press.
            if (view.start_ms > 0.0) {
                ++clicked;
                CHECK(ui::scrollbar_click(view, t.left / 2.0, length).start_ms ==
                      ui::page(view, -1, length).start_ms);
                CHECK(ui::scrollbar_click(view, 1.0, length).start_ms ==
                      ui::page(view, 1, length).start_ms);
            }
        }
    }
    // Every song longer than the window has a page-on view that starts past 0.
    CHECK(clicked > 0);

    // No usable length: no thumb.
    CHECK_FALSE(app::has_song_length(0.0));
    const ui::ThumbShares none = ui::thumb(kAtSix, 0.0);
    CHECK(none.left == 0.0);
    CHECK(none.width == 0.0);
}
