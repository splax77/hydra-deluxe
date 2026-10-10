// The one place a test waits for another thread or process, and the one home
// of how long it may wait. Every wait stops at a cap, so a hung job fails the
// test that waited instead of freezing the run until CI's own limit. The user
// chose both caps on 2026-10-10.
//
// No doctest here: the GUI test runner (tests/ui) uses it too.

#ifndef HYDRA_TESTS_WAIT_UTIL_H
#define HYDRA_TESTS_WAIT_UTIL_H

#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>

namespace testwait {

// How long any one wait inside a test process may take.
inline constexpr std::chrono::milliseconds kWaitCap = std::chrono::seconds(30);

// How long one hydra_uitest --jobs child process may run.
inline constexpr std::chrono::milliseconds kUitestProcessCap = std::chrono::minutes(5);

// A cap as the failure messages print it: "30 s", or "5 ms" below a second.
inline std::string cap_text(std::chrono::milliseconds cap) {
    if (cap.count() % 1000 == 0) return std::to_string(cap.count() / 1000) + " s";
    return std::to_string(cap.count()) + " ms";
}

// What a wait throws when it runs past its cap.
struct WaitTimeout : std::runtime_error {
    using std::runtime_error::runtime_error;
};

[[noreturn]] inline void give_up(std::chrono::milliseconds cap, const std::string& what) {
    throw WaitTimeout("gave up after " + cap_text(cap) + " waiting for " + what);
}

// Calls `done` every millisecond until it returns true. Past `cap` it throws
// WaitTimeout, whose text names `what`. On a test's own thread, doctest
// reports the throw as that test case's failure. Inside a fake running on a
// job's thread, the throw ends the job, so the app can still join that thread
// when the test unwinds.
template <class Pred>
void wait_until(Pred done, const std::string& what, std::chrono::milliseconds cap = kWaitCap) {
    const auto until = std::chrono::steady_clock::now() + cap;
    while (!done()) {
        if (std::chrono::steady_clock::now() > until) give_up(cap, what);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Calls `tick` every millisecond until it throws, the way a fake analysis
// waits for its progress callback to throw once Stop is pressed. Past `cap`
// it throws WaitTimeout, as wait_until does.
template <class Tick>
[[noreturn]] void tick_until_thrown(Tick tick, const std::string& what,
                                    std::chrono::milliseconds cap = kWaitCap) {
    const auto until = std::chrono::steady_clock::now() + cap;
    for (;;) {
        tick();
        if (std::chrono::steady_clock::now() > until) give_up(cap, what);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

}  // namespace testwait

#endif  // HYDRA_TESTS_WAIT_UTIL_H
