// Small PreviewScene builders shared by the highway-draw and track-state
// tests, so the two files build their scenes from one copy.
#pragma once

#include <cstdint>
#include <initializer_list>
#include <utility>

#include "app/preview_view.h"
#include "core/timing.h"

namespace hydra::test_fixtures {

// Tick equals the millisecond, which keeps every note's tick distinct.
inline app::PreviewNote note(double ms, app::PreviewLane lane, bool cymbal = false,
                             bool ghost = false, bool accent = false) {
    app::PreviewNote n;
    n.ms = ms;
    n.tick = static_cast<int64_t>(ms);
    n.lane = lane;
    n.cymbal = cymbal;
    n.ghost = ghost;
    n.accent = accent;
    return n;
}

inline app::PreviewSpan span(double start_ms, double end_ms) {
    app::PreviewSpan s;
    s.start_ms = start_ms;
    s.end_ms = end_ms;
    s.start_tick = static_cast<int64_t>(start_ms);
    s.end_tick = static_cast<int64_t>(end_ms);
    return s;
}

inline app::PreviewFill fill(app::PreviewSpan s, app::PreviewFillState state) {
    app::PreviewFill f;
    f.span = s;
    f.state = state;
    return f;
}

// One tick per millisecond (60 BPM at 1000 ticks per beat), matching note()
// and span() above, so half a tick is 0.5 ms and the edges the tests expect
// (1.0005, 0.2505, ...) are the same as before spans moved to ticks.
inline app::PreviewScene timed_scene() {
    app::PreviewScene s;
    s.timing = SongTiming(1000, {{0, 4000}}, {{0, 60.0}});
    return s;
}

// A note on `tick`, with its time read from `timing`.
inline app::PreviewNote note_at_tick(const SongTiming& timing, int64_t tick,
                                     app::PreviewLane lane) {
    app::PreviewNote n;
    n.tick = tick;
    n.ms = timing.ms_index().at(tick);
    n.lane = lane;
    return n;
}

// At 480 ticks a beat and 300 BPM, a scene with an SP phrase from tick 0 to
// tick 480 and `notes` (tick, lane) on their ticks. One tick here is 0.417 ms,
// shorter than the old half-millisecond margin, so a note on tick 481 sits
// just outside the phrase.
inline app::PreviewScene phrase_to_tick_480_scene(
    std::initializer_list<std::pair<int64_t, app::PreviewLane>> notes) {
    SongTiming timing(480, {{0, 1920}}, {{0, 300.0}});
    app::PreviewScene scene;
    scene.timing = timing;
    for (const auto& [tick, lane] : notes) scene.notes.push_back(note_at_tick(timing, tick, lane));
    app::PreviewSpan phrase;
    phrase.start_tick = 0;
    phrase.end_tick = 480;
    phrase.start_ms = timing.ms_index().at(0);
    phrase.end_ms = timing.ms_index().at(480);
    scene.sp_phrases = {phrase};
    return scene;
}

}  // namespace hydra::test_fixtures
