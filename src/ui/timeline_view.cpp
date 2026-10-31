#include "ui/timeline_view.h"

#include <algorithm>
#include <cmath>

#include "app/display_format.h"
#include "app/preview_view.h"

namespace hydra::ui {

namespace {

// The window that puts `ms` kPlayheadLeadShare in, clamped.
TimelineView lead_to(TimelineView view, double ms, double length_ms) {
    view.start_ms = ms - view.span_ms * kPlayheadLeadShare;
    return clamp(view, length_ms);
}

// clockShort (mockup line 344): whole seconds, rounded down, spelled by
// format_duration.
std::string clock_short(double ms) {
    return app::format_duration(std::floor(std::max(0.0, ms) / 1000.0));
}

}  // namespace

double window_span_ms(double length_ms) {
    if (!app::has_song_length(length_ms)) return kTimelineWindowMs;
    return std::min(kTimelineWindowMs, length_ms);
}

TimelineView open_view(double length_ms) {
    return TimelineView{0.0, window_span_ms(length_ms)};
}

TimelineView clamp(TimelineView view, double length_ms) {
    view.start_ms = std::max(0.0, std::min(length_ms - view.span_ms, view.start_ms));
    return view;
}

TimelineView centre_on(TimelineView view, double ms, double length_ms) {
    view.start_ms = ms - view.span_ms / 2.0;
    return clamp(view, length_ms);
}

TimelineView page(TimelineView view, int n, double length_ms) {
    view.start_ms += n * view.span_ms;
    return clamp(view, length_ms);
}

TimelineView wheel(TimelineView view, double notches, double length_ms) {
    view.start_ms += notches * view.span_ms * kWheelWindowShare;
    return clamp(view, length_ms);
}

TimelineView follow_playing(TimelineView view, double pos_ms, double length_ms) {
    const double f = fraction(view, pos_ms);
    if (f < 0.0 || f > kFollowTurnShare) return lead_to(view, pos_ms, length_ms);
    return view;
}

TimelineView show(TimelineView view, double pos_ms, double length_ms) {
    if (!in_view(view, pos_ms)) return lead_to(view, pos_ms, length_ms);
    return view;
}

double fraction(TimelineView view, double ms) {
    if (view.span_ms <= 0.0) return 0.0;
    return (ms - view.start_ms) / view.span_ms;
}

bool in_view(TimelineView view, double ms) {
    const double f = fraction(view, ms);
    return f >= 0.0 && f <= 1.0;
}

ThumbShares thumb(TimelineView view, double length_ms) {
    if (!app::has_song_length(length_ms)) return {};
    return ThumbShares{view.start_ms / length_ms, std::min(1.0, view.span_ms / length_ms)};
}

std::string readout(TimelineView view, double length_ms) {
    const double end_ms = std::min(length_ms, view.start_ms + view.span_ms);
    // The en dash the mockup writes (U+2013).
    return clock_short(view.start_ms) + " \xE2\x80\x93 " + clock_short(end_ms) + " of " +
           clock_short(length_ms);
}

TimelineView scrollbar_click(TimelineView view, double share, double length_ms) {
    const double at_ms = share * length_ms;
    const int toward = at_ms > view.start_ms ? 1 : (at_ms < view.start_ms ? -1 : 0);
    return page(view, toward, length_ms);
}

}  // namespace hydra::ui
