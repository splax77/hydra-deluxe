// The Path Builder's half of its headless GUI runner: the builder's window
// (PbApp) under the app-free engine (tests/ui/uitest_engine.h), with the
// engine's hooks answered from it.

#ifndef HYDRA_TESTS_PB_UI_PBTEST_HARNESS_H
#define HYDRA_TESTS_PB_UI_PBTEST_HARNESS_H

#include <cstdint>
#include <memory>
#include <string>

#include "uitest_engine.h"

#include "pathbuilder/pb_window.h"

namespace pbtest {

struct Harness : uitest::Engine {
    Harness();

    // The builder, rebuilt by reset_app.
    std::unique_ptr<hydra::pathbuilder::PbApp> app;
    // True for one frame while reset_app lets the last frame's highway image
    // render before freeing it: the frame draws nothing.
    bool app_hidden = false;
    // The chart a fresh builder opens, as its command line would name it.
    // Empty: it opens none.
    std::string chart_arg;
    // The scratch hydra_settings.ini the builder reads.
    std::string ini_path;

    bool init();
    void stop() override;
    // Queues the test named `what`, or the script file at that path.
    bool queue(const std::string& what);
};

inline Harness& harness(ImGuiTestContext* ctx) {
    return static_cast<Harness&>(uitest::engine_of(ctx));
}

// A fresh builder on the GUI tests' settings (tests/scratch_settings.h),
// opening h.chart_arg when it is set.
void reset_app(Harness& h);

// Waits until the builder has nothing running (PbApp::busy), at most
// `seconds`; false when the wait ran out or the test failed.
bool wait_idle(ImGuiTestContext* ctx, double seconds = 60.0);

// Waits for an edit of the player's path: its search, then the new path's
// ladder and overlay, which the edit's frame asks for. False as wait_idle.
bool wait_for_edit(ImGuiTestContext* ctx);

// Starts a window at `act_tick` on the player's path, as a click on a fill
// does, and waits for the edit to land (wait_for_edit).
bool click_fill(ImGuiTestContext* ctx, int64_t act_tick);

// Burnout's first two fills the player can activate on (m20 and m40), by chord
// tick, from one run under the GUI tests' settings.
inline constexpr int64_t kFirstFillTick = 36480;
inline constexpr int64_t kSecondFillTick = 74880;

// The test engine's reference for the widget named `label` anywhere in the
// builder's windows.
std::string any_ref(const std::string& label);

// The timeline's item for the fill at chord tick `tick`.
std::string fill_ref(int64_t tick);

// The list row the screen names after the fill's chord tick (pb_window.cpp's
// draw_choices).
std::string choice_ref(int64_t tick);

// Moves the pointer off the timeline and the list, onto the legend's
// checkbox, so no fill or row stays hovered and a fill's tooltip goes.
void move_off_timeline(ImGuiTestContext* ctx);

// Clicks the row of step `index` (from 0) in "Your activations", which opens
// it to its sentence, and gives the click a frame to land. It leaves the
// test's ref on the steps panel.
bool click_step(ImGuiTestContext* ctx, size_t index);

// The builder's main-window tests (pbtest_window.cpp).
void register_tests(Harness& h);
// The decision list's tests (pbtest_coach.cpp).
void register_coach_tests(Harness& h);
// The whole path built by clicking (pbtest_build.cpp).
void register_build_tests(Harness& h);
// Copy, paste and the path files' tests (pbtest_files.cpp).
void register_file_tests(Harness& h);

}  // namespace pbtest

#endif  // HYDRA_TESTS_PB_UI_PBTEST_HARNESS_H
