"""Tests for report.py, on a real run folder run.py wrote.

    set HYDRA_MUTATE_RUN=<run folder>
    py -I -m unittest discover -s tools/mutate -p test_report.py

HYDRA_MUTATE_RUN names the run folder. HYDRA_MUTATE_MUTANTS names its
mutants.json when it is not where report.py looks by default. With no run
folder named, every test here skips. Tests are never run on made-up result
files, so there is no small sample here; a run folder scored by an older
run.py is re-scored first with run.py --rescore.

The report is written into a temporary folder, never into the run folder.
Each check reads the report's text files and states what they must hold
against the run's own files: the bug json states, the listener files and
map.json.
"""

import json
import os
import re
import sys
import tempfile
import unittest
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import report  # noqa: E402
import run  # noqa: E402

RUN_VAR = "HYDRA_MUTATE_RUN"
MUTANTS_VAR = "HYDRA_MUTATE_MUTANTS"

_BUG_LINE = re.compile(r"^bug (\d+): ")
_TEST_ROW = re.compile(r"^caught (\d+) \| reaches (\d+) \| (.+)$")
_CATCHES = re.compile(r"^  catches: (\d+)$")
_TOTAL = re.compile(r"^Catches, each a test and a bug it caught: (\d+)\.")


def _sections(text):
    """report.txt or tests.txt as {title: [lines]}, a title being a "== " line up to its count."""
    out, current = {}, None
    for line in text.splitlines():
        if line.startswith("== "):
            current = line[3:].rsplit(" (", 1)[0]
            out[current] = []
        elif current is not None and line:
            out[current].append(line)
    return out


def _bug_blocks(text):
    """bugs.txt as {bug number: [its indented lines]}."""
    out, current = {}, None
    for line in text.splitlines():
        hit = _BUG_LINE.match(line)
        if hit:
            current = int(hit.group(1))
            out[current] = []
        elif current is not None and line.startswith("  "):
            out[current].append(line)
    return out


def _numbers(lines):
    return sorted(int(_BUG_LINE.match(l).group(1)) for l in lines if _BUG_LINE.match(l))


def _catch_lines(lines):
    """The tests a bug block names as catches."""
    out = []
    for line in lines:
        for how in report.CATCH_HOWS:
            prefix = f"  {how}: "
            if line.startswith(prefix):
                out.append(line[len(prefix):])
    return out


@unittest.skipUnless(os.environ.get(RUN_VAR), f"set {RUN_VAR} to a run folder run.py wrote")
class RealRun(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.run_dir = Path(os.environ[RUN_VAR]).resolve()
        cls.tmp = tempfile.TemporaryDirectory()
        argv = [str(cls.run_dir), "--out", cls.tmp.name]
        if os.environ.get(MUTANTS_VAR):
            argv += ["--mutants", os.environ[MUTANTS_VAR]]
        if report.main(argv) != 0:
            raise AssertionError("report.py refused the run folder")
        out = Path(cls.tmp.name)
        cls.report = (out / "report.txt").read_text(encoding="utf-8")
        cls.blocks = _bug_blocks((out / "bugs.txt").read_text(encoding="utf-8"))
        tests_text = (out / "tests.txt").read_text(encoding="utf-8")
        cls.test_sections = _sections(tests_text)
        cls.rows = {}
        for line in tests_text.splitlines():
            hit = _TEST_ROW.match(line)
            if hit:
                if hit.group(3) in cls.rows:
                    raise AssertionError(f"tests.txt names {hit.group(3)} twice")
                cls.rows[hit.group(3)] = int(hit.group(1))
        cls.bugs = {}
        for path in (cls.run_dir / "bugs").glob("*.json"):
            bug = json.loads(path.read_text(encoding="utf-8"))
            cls.bugs[bug["number"]] = bug
        cls.map_json = json.loads((cls.run_dir / "map.json").read_text(encoding="utf-8"))
        red_file = cls.run_dir / "red_with_no_bug.txt"
        cls.red_with_no_bug = set(red_file.read_text(encoding="utf-8").splitlines()) - {""}

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def test_every_bug_run_has_its_block(self):
        self.assertEqual(sorted(self.blocks), sorted(self.bugs))

    def test_every_red_test_in_the_result_files_is_under_its_bug(self):
        for n, bug in self.bugs.items():
            lines = self.blocks[n]
            named = ([c["test"] for c in bug["caught_by"]] + bug["caught_through_shared_state"]
                     + bug["red_unexplained"] + bug.get("red_in_file", []))
            try:
                parsed = run.read_listener(self.run_dir / "bugs" / f"{n}.out")
            except run.RunError:
                parsed = None  # bug_result made this run incomplete; its json says why
            if parsed is not None:
                named += [t for t in parsed["order"] if parsed["tests"][t]["verdict"] == "failed"
                          and t not in self.red_with_no_bug]
            for tid in named:
                with self.subTest(bug=n, test=tid):
                    self.assertTrue(any(line.endswith(": " + tid) for line in lines))

    def test_a_test_the_json_credits_has_catches(self):
        for n, bug in self.bugs.items():
            credited = [c["test"] for c in bug["caught_by"]] + bug["caught_through_shared_state"]
            for tid in credited:
                with self.subTest(bug=n, test=tid):
                    self.assertIn(tid, _catch_lines(self.blocks[n]))
                    self.assertGreater(self.rows[tid], 0)

    def test_every_test_with_zero_catches_is_in_the_zero_list(self):
        map_tests = {t["id"] for t in self.map_json["tests"]}
        self.assertEqual(set(self.rows), map_tests)
        zero = {_TEST_ROW.match(l).group(3) for l in self.test_sections["Tests that caught no bug"]
                if _TEST_ROW.match(l)}
        self.assertEqual(zero, {t for t, k in self.rows.items() if k == 0})

    def test_per_test_and_per_bug_counts_add_up_to_one_total(self):
        per_bug = {}
        for n, lines in self.blocks.items():
            counts = [int(_CATCHES.match(l).group(1)) for l in lines if _CATCHES.match(l)]
            self.assertEqual(len(counts), 1, f"bug {n}")
            self.assertEqual(counts[0], len(_catch_lines(lines)), f"bug {n}")
            per_bug[n] = counts[0]
        totals = [int(_TOTAL.match(l).group(1)) for l in self.report.splitlines() if _TOTAL.match(l)]
        self.assertEqual(len(totals), 1)
        self.assertEqual(sum(per_bug.values()), totals[0])
        self.assertEqual(sum(self.rows.values()), totals[0])
        # The same pairs, not only the same sum: each test's catches are the
        # bugs whose block names it as a catch.
        by_test = {}
        for n, lines in self.blocks.items():
            for tid in _catch_lines(lines):
                by_test[tid] = by_test.get(tid, 0) + 1
        self.assertEqual({t: k for t, k in self.rows.items() if k}, by_test)

    def test_each_state_list_holds_exactly_the_bugs_in_that_state(self):
        sections = _sections(self.report)
        for title, state in report.STATE_LISTS:
            with self.subTest(state=state):
                self.assertEqual(_numbers(sections[title]),
                                 sorted(n for n, b in self.bugs.items() if b["state"] == state))

    def test_unreached_and_unrun_lists_match_the_map(self):
        sections = _sections(self.report)
        reached = {int(n) for n in self.map_json["reach"]}
        reached |= {n for ns in self.map_json["outside"].values() for n in ns}
        mutants_path = (Path(os.environ[MUTANTS_VAR]) if os.environ.get(MUTANTS_VAR)
                        else self.run_dir.parent.parent / "mutants.json")
        listed = {m["number"] for m in json.loads(mutants_path.read_text(encoding="utf-8"))["mutants"]}
        self.assertEqual(_numbers(sections["Bugs no test reaches"]), sorted(listed - reached))
        self.assertEqual(_numbers(sections["Bugs reached but never run, because the run stopped first"]),
                         sorted(reached - set(self.bugs)))

    def test_the_report_opens_with_its_limits(self):
        head = self.report.splitlines()[:12]
        for line in report.OPENING:
            self.assertIn(line, head)


if __name__ == "__main__":
    unittest.main()
