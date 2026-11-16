"""Tests for run.py's verdicts: a crash doctest caught, and a survived bug.

    py -I -m unittest discover -s tools/mutate -p test_run.py

Each test writes a small listener file and console output in the shape a
real run leaves (listener.cpp's header owns the file format) and calls
run.py's own functions on them. No rule is worked out again here; each
check states the verdict the evidence must get.
"""

import argparse
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import run  # noqa: E402

FILE = "tests/test_a.cpp"
SWITCH = 5
LISTING = {"format": run.MUTANTS_FORMAT, "complete": True, "mutants": [{"number": SWITCH}], "skipped": []}


def _line(*fields):
    return "\t".join(str(f) for f in fields) + "\n"


def _passed(name, line, switches=""):
    return _line("start", name, FILE, line) + _line("test", name, FILE, line, "passed", switches)


# The mapping run: three tests in one file, the first two reach the switch.
MAP_TEXT = (_passed("first", 10, SWITCH) + _passed("second", 20, SWITCH) + _passed("third", 30))


def _crash_run(message):
    """A bug run whose second test crashed with message, as bug 46's files show it."""
    return (_passed("first", 10, SWITCH)
            + _line("start", "second", FILE, 20)
            + _line("exception", "second", message)
            + _line("test", "second", FILE, 20, "failed", SWITCH))


def _crash_log(message):
    return (f"{FILE}(20): FATAL ERROR: test case CRASHED: {message}\n"
            "[doctest] test cases:  2 |  1 passed | 1 failed | 0 skipped\n"
            "[doctest] Status: FAILURE!\n")


SIGABRT = run.DOCTEST_SIGABRT_MESSAGE
SIGSEGV = "SIGSEGV - Segmentation violation signal"


def _map():
    return run.build_map(run.parse_listener(MAP_TEXT), LISTING)


def _result(listener_text, exit_code, log_text):
    m = _map()
    files = run.files_for(SWITCH, m)
    return run.bug_result(SWITCH, m, files, run.parse_listener(listener_text), exit_code, 0.5, False, log_text)


class DoctestCaughtCrash(unittest.TestCase):
    def test_sigabrt_with_exit_3_is_a_crash(self):
        r = _result(_crash_run(SIGABRT), run.EXIT_ABORT, _crash_log(SIGABRT))
        self.assertEqual(r["state"], "crashed")
        self.assertEqual(r["crash"]["test"], run.test_id("second", FILE, 20))

    def test_exit_3_with_no_crash_line_is_not_a_crash(self):
        log = "[doctest] test cases:  2 |  1 passed | 1 failed | 0 skipped\n"
        r = _result(_crash_run(SIGABRT), run.EXIT_ABORT, log)
        self.assertNotEqual(r["state"], "crashed")
        self.assertIsNone(r["crash"])

    def test_exit_3_with_another_crash_message_is_not_a_crash(self):
        r = _result(_crash_run(SIGSEGV), run.EXIT_ABORT, _crash_log(SIGSEGV))
        self.assertNotEqual(r["state"], "crashed")

    def test_crash_with_exit_1_is_still_a_crash(self):
        for message in (SIGABRT, SIGSEGV):
            with self.subTest(message=message):
                r = _result(_crash_run(message), run.EXIT_FAILED, _crash_log(message))
                self.assertEqual(r["state"], "crashed")
                self.assertEqual(r["crash"]["test"], run.test_id("second", FILE, 20))


class SurvivedBug(unittest.TestCase):
    def test_a_survived_bug_runs_once(self):
        calls = []

        def fake_run_tests(exe, args, out_file, log_file, cwd, mutant, timeout):
            calls.append({"args": args, "mutant": mutant})
            # Every run, mapping run and bug run alike, passes every test.
            out_file.write_bytes(MAP_TEXT.encode("utf-8"))
            log_file.write_bytes(b"")
            return run.EXIT_PASSED, 1.0, False

        with tempfile.TemporaryDirectory() as tmp:
            build = Path(tmp) / "build"
            (build / "Release").mkdir(parents=True)
            mutants = build / "mutants.json"
            mutants.write_text(json.dumps(LISTING), encoding="utf-8")
            exe = build / "Release" / "hydra_tests.exe"
            exe.write_bytes(b"")
            later = mutants.stat().st_mtime + 10
            os.utime(exe, (later, later))
            out = Path(tmp) / "out"
            args = argparse.Namespace(build=build, processes=1, out=out, map_only=False)
            real = run.run_tests
            run.run_tests = fake_run_tests
            try:
                self.assertEqual(run._run(args), 0)
            finally:
                run.run_tests = real
            bug = json.loads((out / "bugs" / f"{SWITCH}.json").read_text(encoding="utf-8"))
            summary = json.loads((out / "summary.json").read_text(encoding="utf-8"))

        self.assertEqual(bug["state"], "survived")
        self.assertEqual(summary["bug_runs_by_state"], {"survived": 1})
        bug_calls = [c for c in calls if c["mutant"] == SWITCH]
        self.assertEqual(len(bug_calls), 1)
        self.assertEqual(bug_calls[0]["args"], run.filter_arg(run.files_for(SWITCH, _map())))
        self.assertEqual(len(calls), 2)


class Rescore(unittest.TestCase):
    def test_rescore_rewrites_a_stale_verdict_and_starts_no_process(self):
        def no_process(*a, **k):
            raise AssertionError("rescore started a process")

        with tempfile.TemporaryDirectory() as tmp:
            run_dir = Path(tmp) / "run"
            bugs = run_dir / "bugs"
            bugs.mkdir(parents=True)
            (run_dir / "map.out").write_bytes(MAP_TEXT.encode("utf-8"))
            (bugs / f"{SWITCH}.out").write_bytes(_crash_run(SIGABRT).encode("utf-8"))
            (bugs / f"{SWITCH}.log").write_bytes(_crash_log(SIGABRT).encode("utf-8"))
            # What an older run.py left: the process facts, and a verdict the
            # current rules do not give these files.
            old = {"number": SWITCH, "files": run.files_for(SWITCH, _map()), "exit_code": run.EXIT_ABORT,
                   "seconds": 0.5, "timed_out": False, "state": "incomplete", "why": ["old rule"],
                   "time_limit_seconds": 9.0, "verdict_from": "filtered run"}
            (bugs / f"{SWITCH}.json").write_text(json.dumps(old), encoding="utf-8")
            saved = (run.run_tests, run.subprocess.Popen, run.subprocess.run)
            run.run_tests = run.subprocess.Popen = run.subprocess.run = no_process
            try:
                results = run.rescore(run_dir, LISTING)
            finally:
                run.run_tests, run.subprocess.Popen, run.subprocess.run = saved
            written = json.loads((bugs / f"{SWITCH}.json").read_text(encoding="utf-8"))

        expected = run.result_from_files(SWITCH, _map(), old["files"], Path("unused.out"), Path("unused.log"),
                                         run.EXIT_ABORT, 0.5, False)
        self.assertEqual(written["state"], "crashed")
        self.assertEqual(written["crash"]["test"], run.test_id("second", FILE, 20))
        self.assertEqual(written["time_limit_seconds"], 9.0)
        self.assertNotIn("verdict_from", written)
        self.assertEqual(results, [written])
        # The same call with no files is what the stored facts alone give,
        # so the new verdict came from the run's files.
        self.assertEqual(expected["state"], "incomplete")


if __name__ == "__main__":
    unittest.main()
