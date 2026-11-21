// Tests for app/preview_clock: Onyx's display clock on the real monotonic
// source it runs on in the Preview (PreviewClock::steady_now). Each song time
// a playing clock gives is checked against reads of that same source taken
// just before and just after, so the check is exact without a fake time.

#include "doctest.h"

#include <chrono>
#include <thread>

#include "app/preview_clock.h"

using hydra::app::PreviewClock;

namespace {

// Two reads of the real source, taken around a call to the clock.
struct Span {
    double lo = 0.0;
    double hi = 0.0;
};

template <class F>
Span around(F&& f) {
    Span s;
    s.lo = PreviewClock::steady_now();
    f();
    s.hi = PreviewClock::steady_now();
    return s;
}

// Lets real time pass on the source, so a clock that should not move would
// show it moved. Fails at once if the source did not advance.
void let_time_pass() {
    const double before = PreviewClock::steady_now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE(PreviewClock::steady_now() > before);
}

// A playing clock's song time: `from_ms` plus the real time since `started`
// (the reads around the play or seek that set it), as seen by a read now.
// The time it gives must lie between the shortest and the longest real time
// those reads allow.
double check_playing(const PreviewClock& clock, double from_ms, const Span& started) {
    double ms = 0.0;
    const Span read = around([&] { ms = clock.now_ms(); });
    CHECK(ms >= from_ms + (read.lo - started.hi) * 1000.0);
    CHECK(ms <= from_ms + (read.hi - started.lo) * 1000.0);
    return ms;
}

}  // namespace

TEST_CASE("PreviewClock: frozen while paused, wall-clock while playing") {
    PreviewClock clock;

    CHECK(clock.now_ms() == 0.0);
    CHECK_FALSE(clock.playing());
    let_time_pass();
    CHECK(clock.now_ms() == 0.0);  // paused: no advance

    const Span started = around([&] { clock.play(); });
    CHECK(clock.playing());
    let_time_pass();
    check_playing(clock, 0.0, started);
    clock.play();  // already playing: the start time is kept
    let_time_pass();
    check_playing(clock, 0.0, started);

    const Span paused = around([&] { clock.pause(); });
    CHECK_FALSE(clock.playing());
    const double paused_at = clock.now_ms();
    // The frozen time is the song time at the pause.
    CHECK(paused_at >= (paused.lo - started.hi) * 1000.0);
    CHECK(paused_at <= (paused.hi - started.lo) * 1000.0);
    let_time_pass();
    CHECK(clock.now_ms() == paused_at);
    clock.pause();  // already paused: nothing changes
    CHECK(clock.now_ms() == paused_at);
}

TEST_CASE("PreviewClock: seek sets the song time, playing or not") {
    PreviewClock clock;

    // Paused: the seek moves the frozen time, and real time does not move it.
    clock.seek_ms(30000.0);
    let_time_pass();
    const double sought = clock.now_ms();
    CHECK(sought == 30000.0);

    // Play continues from the sought time.
    const Span started = around([&] { clock.play(); });
    let_time_pass();
    const double before_seek = check_playing(clock, sought, started);

    // While playing, a seek back continues from the new time.
    const Span reseek = around([&] { clock.seek_ms(10000.0); });
    let_time_pass();
    const double after_seek = check_playing(clock, 10000.0, reseek);
    CHECK(after_seek < before_seek);
    CHECK(clock.playing());

    clock.toggle();
    CHECK_FALSE(clock.playing());
    const double frozen = clock.now_ms();
    let_time_pass();
    CHECK(clock.now_ms() == frozen);
    clock.toggle();
    CHECK(clock.playing());
}
