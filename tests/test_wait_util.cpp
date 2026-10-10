// Tests for the test suite's one wait helper (tests/wait_util.h): a wait on
// something that never happens fails, naming what it waited for.

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
