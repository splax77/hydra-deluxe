"""Tests for first_use.py: its "Bugs it caught" is report.py's count, shared-state catches included.

    py -I -m unittest discover -s tools/mutate -p test_first_use.py

The run folder is a small one in the shape run.py leaves: a listener file
for the mapping run (listener.cpp's header owns its format), an empty
red_with_no_bug.txt, a mutants.json and one bug json. In the bug's run one
test that reaches the bug went red, and one that does not reach it went red
through state an earlier test left. Each check compares first_use.py's
figure with report.py's own result for the same test.
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
import first_use  # noqa: E402
import report  # noqa: E402
import run  # noqa: E402

FILE = "tests/test_a.cpp"
SWITCH = 5
LINES = {"first": 10, "second": 20, "third": 30}


def _line(*fields):
    return "\t".join(str(f) for f in fields) + "\n"


def _passed(name, switches=""):
    ln = LINES[name]
    return _line("start", name, FILE, ln) + _line("test", name, FILE, ln, "passed", switches)


class CaughtIsReportsCount(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        self.repo = root / "repo"
        (self.repo / "tests").mkdir(parents=True)
        src = [""] * 40
        for name, ln in LINES.items():
            src[ln - 1] = f'TEST_CASE("{name}") {{'
            src[ln] = "    CHECK(true);"
            src[ln + 1] = "}"
        (self.repo / FILE).write_text("\n".join(src) + "\n", encoding="utf-8")

        self.run_dir = root / "build" / "runs" / "run1"
        (self.run_dir / "bugs").mkdir(parents=True)
        self.mutants = root / "build" / "mutants.json"
        self.mutants.write_text(json.dumps({"format": run.MUTANTS_FORMAT, "complete": True,
                                            "mutants": [{"number": SWITCH}], "skipped": []}), encoding="utf-8")
        # The mapping run: the first two tests reach the switch, the third does not.
        (self.run_dir / "map.out").write_bytes(
            (_passed("first", SWITCH) + _passed("second", SWITCH) + _passed("third")).encode("utf-8"))
        (self.run_dir / "red_with_no_bug.txt").write_text("", encoding="utf-8")
        self.ids = {n: run.test_id(n, FILE, ln) for n, ln in LINES.items()}
        bug = {"number": SWITCH, "state": "caught", "why": [], "crash": None,
               "caught_by": [{"test": self.ids["first"], "checks": [[FILE, 11]], "exceptions": []}],
               "caught_through_shared_state": [self.ids["third"]], "red_unexplained": []}
        (self.run_dir / "bugs" / f"{SWITCH}.json").write_text(json.dumps(bug), encoding="utf-8")
        # One copied line inside each test, and one outside every test.
        self.entries = [("S", "1", FILE, 11), ("S", "2", FILE, 21), ("S", "3", FILE, 31), ("S", "4", FILE, 5)]

    def tearDown(self):
        self.tmp.cleanup()

    def _rows(self):
        r = report.build_report(self.run_dir, self.mutants)
        return r, {row["md"]: row for row in first_use.table_rows(self.entries, r, self.repo)}

    def test_each_tests_caught_is_reports(self):
        r, rows = self._rows()
        for row in rows.values():
            if row["test_id"] is not None:
                self.assertEqual(row["caught"], r["tests"][row["test_id"]]["caught"], row["test"])

    def test_a_shared_state_catch_counts_and_is_shown(self):
        r, rows = self._rows()
        third = rows["3"]
        self.assertEqual(third["test_id"], self.ids["third"])
        self.assertEqual(third["caught"], [SWITCH])
        self.assertEqual(third["shared"], [SWITCH])
        self.assertEqual(rows["1"]["shared"], [])
        text = first_use.table_text(list(rows.values()), [], r["run"])
        self.assertIn(f"| {FILE}:31 | third | 0 | 1 (1 through shared state) | none |", text)
        self.assertIn(f"| {FILE}:11 | first | 1 | 1 | 1 bugs ({SWITCH}) |", text)
        self.assertIn(f"| {FILE}:21 | second | 1 | 0: not shown to fail by this run | none |", text)

    def test_a_line_outside_every_test_gets_no_test(self):
        _, rows = self._rows()
        self.assertIsNone(rows["4"]["test"])


if __name__ == "__main__":
    unittest.main()
