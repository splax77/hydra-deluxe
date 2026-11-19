"""Tests for timeout_split.py, on a small run folder and test tree written to a temp folder.

    py -I -m unittest discover -s tools/mutate -p test_timeout_split.py

Each bug json holds only the fields run.py's bug_result writes for a
timed-out run, and map.json only what run.py writes for each test. The test
tree has its own tests/wait_util.h, so the waits come from that file the way
they come from the real one. Each check states which group or side a bug or
test must land on.
"""

import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import timeout_split  # noqa: E402

REPO = Path(HERE).parents[1]

WAIT_UTIL = """\
namespace testwait {
inline std::string cap_text(int cap) { return std::to_string(cap); }
[[noreturn]] inline void give_up(int cap, const std::string& what) { throw 1; }
template <class Pred>
void wait_until(Pred done, const std::string& what, int cap = 30) {
    while (!done()) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
}
template <class Owner>
void poll_until_idle(Owner& owner, const std::string& what) {
    wait_until([&] { owner.poll(); return !owner.busy(); }, what);
}
}
"""

# A helper two headers deep: the test file includes outer.h, which includes inner.h.
INNER = """\
#include "wait_util.h"
inline void settle_deep(App& app) {
    testwait::wait_until([&] { return app.done(); }, "the app to settle");
}
"""

OUTER = """\
#include "inner.h"
inline void settle(App& app) { settle_deep(app); }
inline int compute(int x) { return x + 1; }
"""

TEST_FILE = """\
#include "doctest.h"
#include "outer.h"

struct Gate {
    void wait_reached(Owner& o) { testwait::poll_until_idle(o, "the gate"); }
};

TEST_CASE("names a wait itself") {
    testwait::wait_until([] { return true; }, "nothing");
}

TEST_CASE("reaches a wait through a header helper") {
    App app;
    settle(app);
}

TEST_CASE("reaches a wait through a struct method") {
    Gate gate;
    Owner o;
    gate.wait_reached(o);
}

TEST_CASE("reaches no wait") {
    CHECK(compute(1) == 2);
}
"""


def _case_lines(text):
    """Each TEST_CASE's name -> its line, read from the text."""
    out = {}
    for i, line in enumerate(text.splitlines(), 1):
        if line.startswith("TEST_CASE("):
            out[line.split('"')[1]] = i
    return out


class Split(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        self.repo = root / "repo"
        (self.repo / "tests").mkdir(parents=True)
        for name, text in (("wait_util.h", WAIT_UTIL), ("inner.h", INNER), ("outer.h", OUTER),
                           ("test_x.cpp", TEST_FILE)):
            (self.repo / "tests" / name).write_text(text, encoding="utf-8")
        lines = _case_lines(TEST_FILE)
        # Clean seconds: the struct-method test is the one slow test.
        seconds = {"names a wait itself": 0.01, "reaches a wait through a header helper": 0.2,
                   "reaches a wait through a struct method": 4.0, "reaches no wait": 0.001}
        self.ids = {n: f"tests\\test_x.cpp:{ln} {n}" for n, ln in lines.items()}
        tests = [{"name": n, "file": "tests\\test_x.cpp", "line": lines[n], "seconds": seconds[n],
                  "id": self.ids[n]} for n in lines]
        self.run_dir = root / "run"
        (self.run_dir / "bugs").mkdir(parents=True)
        (self.run_dir / "map.json").write_text(json.dumps({"tests": tests}), encoding="utf-8")
        direct, helper = self.ids["names a wait itself"], self.ids["reaches a wait through a header helper"]
        method, none = self.ids["reaches a wait through a struct method"], self.ids["reaches no wait"]
        bugs = [
            # Already red, stopped in the header-helper test.
            {"number": 1, "state": "timed_out", "red_in_file": [none], "running_at_limit": helper},
            # No red test: fast, fast, slow, between tests.
            {"number": 2, "state": "timed_out", "red_in_file": [], "running_at_limit": helper},
            {"number": 3, "state": "timed_out", "red_in_file": [], "running_at_limit": none},
            {"number": 4, "state": "timed_out", "red_in_file": [], "running_at_limit": method},
            {"number": 5, "state": "timed_out", "red_in_file": [], "running_at_limit": None},
            {"number": 6, "state": "timed_out", "red_in_file": [], "running_at_limit": direct},
            # Its run left no result file, so bug_result wrote neither field.
            {"number": 7, "state": "timed_out"},
            # Not timed out: in no group.
            {"number": 8, "state": "caught", "caught_by": [], "caught_through_shared_state": []},
        ]
        for b in bugs:
            (self.run_dir / "bugs" / f"{b['number']}.json").write_text(json.dumps(b), encoding="utf-8")

    def tearDown(self):
        self.tmp.cleanup()

    def _run(self, fast_below):
        tests = timeout_split.read_map_tests(self.run_dir)
        bugs = timeout_split.timed_out_bugs(self.run_dir)
        src = timeout_split.Sources(self.repo)
        return (timeout_split.split_groups(bugs, tests, fast_below),
                timeout_split.wait_split(bugs, tests, src))

    def test_each_bug_lands_in_its_group(self):
        groups, _ = self._run(0.5)
        self.assertEqual(groups["red_already"], [1])
        self.assertEqual(groups["fast"], [2, 3, 6])
        self.assertEqual(groups["slow"], [4])
        self.assertEqual(groups["between"], [5])
        self.assertEqual(groups["no_result"], [7])

    def test_the_line_is_the_argument(self):
        groups, _ = self._run(0.1)
        self.assertEqual(groups["fast"], [3, 6])
        self.assertEqual(groups["slow"], [2, 4])

    def test_the_waits_are_the_ones_wait_util_defines(self):
        src = timeout_split.Sources(self.repo)
        self.assertEqual(src.waits, {"wait_until", "poll_until_idle"})

    def test_each_test_lands_on_its_side(self):
        _, waits = self._run(0.5)
        side = {r["test"]: r["side"] for r in waits["rows"]}
        self.assertEqual(side[self.ids["names a wait itself"]], "waits")
        self.assertEqual(side[self.ids["reaches a wait through a header helper"]], "waits")
        self.assertEqual(side[self.ids["reaches a wait through a struct method"]], "waits")
        self.assertEqual(side[self.ids["reaches no wait"]], "no wait")
        # Bug 5 was between tests and bug 7 named no test, so neither side holds them.
        self.assertEqual(waits["totals"], {"waits": [3, 4, 3], "no wait": [1, 1, 1]})

    def test_the_header_helper_and_the_method_are_named(self):
        _, waits = self._run(0.5)
        helpers = {r["test"]: r["helpers"] for r in waits["rows"]}
        self.assertIn("settle", helpers[self.ids["reaches a wait through a header helper"]])
        self.assertIn("wait_reached", helpers[self.ids["reaches a wait through a struct method"]])

    def test_the_real_wait_home_gives_its_waits(self):
        src = timeout_split.Sources(REPO)
        self.assertTrue({"wait_until", "poll_until_idle", "tick_until_thrown"} <= src.waits)
        self.assertNotIn("give_up", src.waits)

    def test_a_loop_that_sleeps_nowhere_known_fails_loudly(self):
        (self.repo / "tests" / "wait_util.h").write_text(
            WAIT_UTIL + "inline void spin(Pred p) { while (!p()) { other_pause(); } }\n", encoding="utf-8")
        with self.assertRaises(timeout_split.SplitError):
            timeout_split.Sources(self.repo)


if __name__ == "__main__":
    unittest.main()
