// The Path Builder timeline's window: which stretch of the song the timeline
// shows, and how a page turn, a wheel notch, the playhead or a seek moves it.
// Pure arithmetic over milliseconds, with no ImGui, so the tests run it with
// no context. The spec is the third mockup's script
// (docs/superpowers/specs/2026-10-31-path-builder-mockup-3/mockup_template.html);
// each function names the line it follows. The whole-song share the overview
// strip and the scrubber use stays app::song_fraction's.
#pragma once

#include <string>

namespace hydra::ui {

// How much song the window shows: the mockup's SPAN_MIN_MS (line 312), the
// three minutes the user approved as drawn. A song shorter than this gets a
// window as long as the song (window_span_ms).
inline constexpr double kTimelineWindowMs = 180000.0;

// The share of a window one wheel notch moves it: a third (mockup line 734).
inline constexpr double kWheelWindowShare = 1.0 / 3.0;

// While playing, the window turns a page once the playhead passes this share
// of it (mockup line 362).
inline constexpr double kFollowTurnShare = 0.75;

// Where a turned page, or a seek that left the window, puts the moment: this
// share into the window (mockup lines 362 and 813).
inline constexpr double kPlayheadLeadShare = 0.25;

// The stretch of the song the timeline shows: it starts `start_ms` into the
// song and runs `span_ms`.
struct TimelineView {
    double start_ms = 0.0;
    double span_ms = kTimelineWindowMs;
};

// How long the window is for a song `length_ms` long: the smaller of
// kTimelineWindowMs and the song's length (the user's answer to plan 3's Q1,
// "Fill the width"). kTimelineWindowMs when app::has_song_length refuses the
// length, since there is no song length to cap it by.
double window_span_ms(double length_ms);

// The window a song opens with: at the song's start, window_span_ms long.
TimelineView open_view(double length_ms);

// `view` with its start held between 0 and `length_ms` less its span
// (clampView, line 354). A window longer than the song starts at 0.
TimelineView clamp(TimelineView view, double length_ms);

// The window moved so `ms` sits at its middle, then clamped (centreOn,
// line 360).
TimelineView centre_on(TimelineView view, double ms, double length_ms);

// The window moved `n` whole windows, back when `n` is negative, then clamped
// (the scrollbar's buttons, lines 713 and 714).
TimelineView page(TimelineView view, int n, double length_ms);

// The window moved `notches` wheel notches, each kWheelWindowShare of a
// window, later in the song for positive notches, then clamped (line 734).
// The caller turns its wheel's direction into notches.
TimelineView wheel(TimelineView view, double notches, double length_ms);

// Where the window goes while the playhead plays at `pos_ms` (follow,
// line 362): when the playhead is before the window or past
// kFollowTurnShare of it, the window that puts it kPlayheadLeadShare in;
// otherwise `view` as it is. Whether the window is following at all is the
// caller's.
TimelineView follow_playing(TimelineView view, double pos_ms, double length_ms);

// Where the window goes when the clock jumps to `pos_ms` (seek, line 813):
// when `pos_ms` is outside the window, the window that puts it
// kPlayheadLeadShare in; otherwise `view` as it is.
TimelineView show(TimelineView view, double pos_ms, double length_ms);

// How far across the window `ms` falls: 0 at its start, 1 at its end, and
// outside 0..1 for a moment outside it, so a band half outside the window
// keeps its visible half (fr, line 355). 0 for a window with no length.
double fraction(TimelineView view, double ms);

// The moment `share` of the way across the window: fraction run backwards, so
// ms_at(view, fraction(view, ms)) is `ms` again. Not held to the window or the
// song; a caller that needs a moment inside them clamps. A point on the
// timeline, on the strip or on the scrollbar becomes a moment through here
// (the strip and the scrollbar read the whole song as one window,
// whole_song_view).
double ms_at(TimelineView view, double share);

// The whole song as one window: from 0, `length_ms` long. The overview strip
// and the scrollbar lay the song out over their width with it.
TimelineView whole_song_view(double length_ms);

// Is `ms` inside the window, its ends included (inView, line 358)?
bool in_view(TimelineView view, double ms);

// The scrollbar's thumb as shares of the bar: its left edge is the window's
// start over the song's length, its width the window's span over it, at most
// the whole bar (renderScrollbar, lines 697 and 698). Both 0 when
// app::has_song_length refuses the length.
struct ThumbShares {
    double left = 0.0;
    double width = 0.0;
};
ThumbShares thumb(TimelineView view, double length_ms);

// The scrollbar's readout, "6:00 – 9:00 of 42:02": the window's start, its
// end held at the song's length, and the length (line 699). Each time is
// app::format_duration of its whole seconds, rounded down as the mockup's
// clockShort rounds them (line 344).
std::string readout(TimelineView view, double length_ms);

// A press on the scrollbar `share` of the way along it, beside the thumb: the
// window moved one window toward the press (line 711). A press on the thumb
// itself is a drag, which the caller handles; the caller decides which it is.
TimelineView scrollbar_click(TimelineView view, double share, double length_ms);

}  // namespace hydra::ui
