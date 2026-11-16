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


def _timed(file, line, name, seconds):
    """One test's lines in a --duration run's console output."""
    return ("=" * 79 + "\n" + f"{file}({line}):\n" + f"TEST CASE:  {name}\n\n"
            + f"{seconds:.6f} s: {name}\n")


# The mapping run's console output, giving each of its tests a time.
MAP_LOG = _timed(FILE, 10, "first", 0.5) + _timed(FILE, 20, "second", 0.25) + _timed(FILE, 30, "third", 0.125)

# Lines copied from a --duration run of hydra_tests. The MESSAGE line came
# from a later test of that run; it stands here for a test's own output
# between its header and its time.
REAL_DURATION_LOG = (
    '[doctest] doctest version is "2.4.11"\r\n'
    '[doctest] run with "--help" for options\r\n'
    "===============================================================================\r\n"
    "tests\\test_activation_rows.cpp(60):\r\n"
    "TEST CASE:  display format: the average multiplier rounds the exact double\r\n"
    "\r\n"
    "0.000062 s: display format: the average multiplier rounds the exact double\r\n"
    "===============================================================================\r\n"
    "tests\\test_activation_rows.cpp(118):\r\n"
    "TEST CASE:  build_activations: rows and backend rows line up\r\n"
    "\r\n"
    "tests\\test_analysis.cpp(65): MESSAGE: discovered 98 chart(s)\r\n"
    "0.007121 s: build_activations: rows and backend rows line up\r\n"
    "===============================================================================\r\n"
    "[doctest] test cases:    408 |    408 passed | 0 failed | 1823 skipped\r\n"
    "[doctest] Status: SUCCESS!\r\n")


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


class ProcessEndedCrash(unittest.TestCase):
    def test_the_test_the_process_died_inside_caught_the_bug(self):
        # A fast fail: Windows ends the process with STATUS_STACK_BUFFER_OVERRUN
        # while "second" runs, so its record is a start with no ending.
        status_stack_buffer_overrun = 0xC0000409
        text = _passed("first", 10, SWITCH) + _line("start", "second", FILE, 20)
        r = _result(text, status_stack_buffer_overrun, "")
        second = run.test_id("second", FILE, 20)
        self.assertEqual(r["state"], "crashed")
        self.assertEqual(r["crash"]["test"], second)
        self.assertEqual([c["test"] for c in r["caught_by"]], [second])


class SurvivedBug(unittest.TestCase):
    def test_a_survived_bug_runs_once(self):
        calls = []

        def fake_run_tests(exe, args, out_file, log_file, cwd, mutant, timeout):
            calls.append({"args": args, "mutant": mutant, "timeout": timeout})
            # Every run, mapping run and bug run alike, passes every test.
            out_file.write_bytes(MAP_TEXT.encode("utf-8"))
            log_file.write_bytes(MAP_LOG.encode("utf-8") if mutant is None else b"")
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
        map_calls = [c for c in calls if c["mutant"] is None]
        self.assertEqual(map_calls[0]["args"], [run.DURATION_ARG])
        # The switch's file holds all three tests, so its clean time is
        # their three times, and its limit 5 s more.
        self.assertAlmostEqual(bug["clean_seconds"], 0.5 + 0.25 + 0.125)
        self.assertAlmostEqual(bug_calls[0]["timeout"], 0.875 + 5.0)
        self.assertAlmostEqual(bug["time_limit_seconds"], 5.875)
        self.assertEqual(summary["time_limit"], {"rule": run.TIME_LIMIT_RULE, "lowest_seconds": 5.875,
                                                 "highest_seconds": 5.875})


class TestTimes(unittest.TestCase):
    def test_a_real_duration_log_parses(self):
        times = run.parse_durations(REAL_DURATION_LOG)
        file = "tests\\test_activation_rows.cpp"
        self.assertEqual(times, {
            run.test_id("display format: the average multiplier rounds the exact double", file, 60): 0.000062,
            run.test_id("build_activations: rows and backend rows line up", file, 118): 0.007121,
        })

    def test_a_time_with_no_header_stops(self):
        with self.assertRaises(run.RunError):
            run.parse_durations("0.000062 s: first\n")

    def test_a_time_naming_another_test_stops(self):
        with self.assertRaises(run.RunError):
            run.parse_durations(f"{FILE}(10):\nTEST CASE:  first\n\n0.5 s: second\n")

    def test_the_clean_time_sums_the_files_tests(self):
        m = _map()
        run.add_test_seconds(m, run.parse_durations(MAP_LOG))
        self.assertAlmostEqual(run.clean_seconds([FILE], m, 99.0), 0.875)
        self.assertEqual(run.clean_seconds(["tests/other.cpp"], m, 99.0), 0)
        # A run of every file is the whole mapping run.
        self.assertEqual(run.clean_seconds(None, m, 99.0), 99.0)

    def test_a_test_with_no_time_stops(self):
        m = _map()
        log = _timed(FILE, 10, "first", 0.5) + _timed(FILE, 30, "third", 0.125)
        with self.assertRaises(run.RunError) as caught:
            run.add_test_seconds(m, run.parse_durations(log))
        self.assertIn(run.test_id("second", FILE, 20), str(caught.exception))

    def test_a_time_for_a_test_the_listener_did_not_record_stops(self):
        m = _map()
        with self.assertRaises(run.RunError):
            run.add_test_seconds(m, run.parse_durations(MAP_LOG + _timed(FILE, 40, "fourth", 0.1)))

    def test_the_limit_is_the_clean_time_plus_five_seconds(self):
        self.assertEqual(run.test_timeout_seconds(0.0), 5.0)
        self.assertEqual(run.test_timeout_seconds(29.9), 29.9 + 5.0)


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
            # Its summary holds one limit for every bug run, and its map.out
            # and map.log give no test times.
            old_summary = {"mapping_run_seconds": 7.2, "mapping_run_exit_code": 0, "processes": 2,
                           "bug_phase_wall_seconds": 3.0, "time_limit_seconds": 9.0}
            (run_dir / "summary.json").write_text(json.dumps(old_summary), encoding="utf-8")
            saved = (run.run_tests, run.subprocess.Popen, run.subprocess.run)
            run.run_tests = run.subprocess.Popen = run.subprocess.run = no_process
            try:
                results = run.rescore(run_dir, LISTING)
            finally:
                run.run_tests, run.subprocess.Popen, run.subprocess.run = saved
            written = json.loads((bugs / f"{SWITCH}.json").read_text(encoding="utf-8"))
            summary = json.loads((run_dir / "summary.json").read_text(encoding="utf-8"))

        self.assertEqual(summary["time_limit"], {"every_bug_run_seconds": 9.0})
        self.assertEqual(summary["bug_runs_by_state"], {"crashed": 1})
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
