// The Path Builder's half of its headless GUI runner: the builder's window
// (PbApp) under the app-free engine (tests/ui/uitest_engine.h), with the
// engine's hooks answered from it.

#ifndef HYDRA_TESTS_PB_UI_PBTEST_HARNESS_H
#define HYDRA_TESTS_PB_UI_PBTEST_HARNESS_H

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

// The builder's main-window tests (pbtest_window.cpp).
void register_tests(Harness& h);
// The decision list's tests (pbtest_coach.cpp).
void register_coach_tests(Harness& h);
// The clicks on the highway and the strip (pbtest_strip.cpp).
void register_strip_tests(Harness& h);

}  // namespace pbtest

#endif  // HYDRA_TESTS_PB_UI_PBTEST_HARNESS_H
