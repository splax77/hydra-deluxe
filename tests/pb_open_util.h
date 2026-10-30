// The one place a unit test opens a chart in the Path Builder's state and
// waits for the open to end (test_pb_state.cpp, test_pb_coach.cpp).

#ifndef HYDRA_TESTS_PB_OPEN_UTIL_H
#define HYDRA_TESTS_PB_OPEN_UTIL_H

#include <string>

#include "scratch_settings.h"
#include "wait_util.h"

#include "pathbuilder/pb_state.h"

// Opens `path` under `settings` (the GUI tests' by default) and waits for the
// open to end, by testwait's cap.
inline void open_and_wait(hydra::pathbuilder::PbState& s, const std::string& path,
                          const hydra::app::Settings& settings = scratch_settings()) {
    s.open_with(path, settings);
    testwait::wait_until(
        [&] {
            s.poll();
            return !s.busy();
        },
        "the Path Builder's open to finish");
}

#endif  // HYDRA_TESTS_PB_OPEN_UTIL_H
