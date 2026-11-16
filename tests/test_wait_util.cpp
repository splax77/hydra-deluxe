// Tests for the test suite's one wait helper (tests/wait_util.h): a wait on
// something that never happens fails, naming what it waited for, and a wait
// that can no longer end fails at once, naming why.

#include "doctest.h"

#include <chrono>

#include "wait_util.h"

TEST_CASE("wait_until fails, naming what it waited for, when the wait never ends") {
    // A short cap, so this test takes 5 ms rather than the suite's real cap.
    CHECK_THROWS_WITH_AS(testwait::wait_until([] { return false; }, "a thing that never happens",
                                              std::chrono::milliseconds(5)),
                         "gave up after 5 ms waiting for a thing that never happens",
                         testwait::WaitTimeout);
    CHECK_NOTHROW(testwait::wait_until([] { return true; }, "a thing that already happened",
                                       std::chrono::milliseconds(5)));
}

TEST_CASE("wait_until_or_gone fails at once, naming why, when the job ends without the step") {
    // The suite's real cap: the throw has to come from `gone`, not the cap.
    CHECK_THROWS_WITH_AS(
        testwait::wait_until_or_gone([] { return false; }, [] { return true; }, "its save",
                                     "the job ended before it reached its save",
                                     std::chrono::seconds(30)),
        "stopped waiting for its save: the job ended before it reached its save",
        testwait::WaitTimeout);
}

TEST_CASE("wait_until_or_gone returns when the step and the job's end show on one ask") {
    CHECK_NOTHROW(testwait::wait_until_or_gone([] { return true; }, [] { return true; },
                                               "its save", "the job ended before it reached its save",
                                               std::chrono::seconds(30)));
}

TEST_CASE("wait_until_or_gone returns when the step shows on the ask after the job's end") {
    // The job reached the step and finished between the first ask of `done`
    // and the ask of `gone`, so only the re-ask sees the step.
    int asks = 0;
    CHECK_NOTHROW(testwait::wait_until_or_gone([&] { return ++asks > 1; }, [] { return true; },
                                               "its save", "the job ended before it reached its save",
                                               std::chrono::seconds(30)));
}

TEST_CASE("wait_until_or_gone fails at the cap, as wait_until does, when nothing ever happens") {
    // A short cap, so this test takes 5 ms rather than the suite's real cap.
    CHECK_THROWS_WITH_AS(
        testwait::wait_until_or_gone([] { return false; }, [] { return false; },
                                     "a thing that never happens", "the job ended first",
                                     std::chrono::milliseconds(5)),
        "gave up after 5 ms waiting for a thing that never happens", testwait::WaitTimeout);
}
