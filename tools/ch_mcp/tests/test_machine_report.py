"""The report over the machine recorder's files. Every fixture is built here from
the three files' columns; no test reads a real recording or the live settings.ini.
"""
import contextlib
import csv
import io
import os
import subprocess
import sys
import tempfile
import unittest

from tools.ch_mcp import machine_report as R
from tools.ch_mcp import quiet, sp_tools

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))

GAME, WORKER, RECORDER, COMPILER = 7, 8, 9, 11
GAME_TID = 70
BASE_S = 1000
PERIOD_MS = 20


def wall_text(ms):
    """A wall time in the file's form, `ms` after the fixture's start."""
    return f"{BASE_S + ms / 1000:.3f}"


class Fixture:
    """Rows for the three files, written by `write`. Times are ms after BASE_S."""

    def __init__(self):
        self.samples = []
        self.processes = []
        self.threads = []

    def sample(self, wall_ms, clock_ms=None, read_ms=1):
        """One samples.csv row. `read_ms` is how far apart the clock's two
        stamps are, and the first stamp is the sample's own wall time."""
        number = len(self.samples)
        clock = ["", "", ""]
        if clock_ms is not None:
            clock = [f"{clock_ms / 1000:.3f}", wall_text(wall_ms), wall_text(wall_ms + read_ms)]
        self.samples.append([number, wall_text(wall_ms), 1, 400, 5000, 2] + clock
                            + [RECORDER, PERIOD_MS, "", ""])
        return number

    def steady(self, count, period_ms=PERIOD_MS):
        for i in range(count):
            self.sample(i * period_ms)

    def process(self, sample, pid, name, cpu_ms=0, read=0, wrote=0, faults=0, parent=1):
        self.processes.append([sample, pid, parent, name, cpu_ms, read, wrote, faults])

    def thread(self, sample, pid=GAME, tid=GAME_TID, reads_as="running"):
        self.threads.append([sample, pid, tid, 2, 0, reads_as, 1, 1])

    def stop(self, reason, process_time="9.5"):
        self.samples[-1][-2:] = [reason, process_time]

    def write(self, folder):
        for name, columns, rows in ((R.SAMPLES_FILE, R.SAMPLE_COLUMNS, self.samples),
                                    (R.PROCESSES_FILE, R.PROCESS_COLUMNS, self.processes),
                                    (R.THREADS_FILE, R.THREAD_COLUMNS, self.threads)):
            with open(os.path.join(folder, name), "w", newline="", encoding="utf-8") as f:
                out = csv.writer(f)
                out.writerow(columns)
                out.writerows(rows)


def clock_fixture(pairs):
    """Samples whose steps are `pairs` of (clock gain, wall gain) in ms."""
    fx = Fixture()
    clock = wall = 0
    last = fx.sample(wall, clock)
    fx.thread(last)
    for clock_gain, wall_gain in pairs:
        clock += clock_gain
        wall += wall_gain
        fx.sample(wall, clock)
    return fx


class ReportCase(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.folder = os.path.join(tmp.name, "machine-1")
        os.mkdir(self.folder)
        self.documents = os.path.join(tmp.name, "documents")
        os.mkdir(self.documents)

    def run_report(self, fx, *args, pid=GAME):
        """(exit code, output lines) of main on this fixture."""
        fx.write(self.folder)
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = R.main([self.folder, str(pid), *args], documents_dir=self.documents)
        return code, out.getvalue().splitlines()

    def lines(self, fx, *args):
        code, lines = self.run_report(fx, *args)
        self.assertEqual(code, 0, lines)
        return lines


class Constants(unittest.TestCase):
    def test_the_questions_numbers(self):
        self.assertEqual(R.LONG_STEP_FACTOR, 2)
        self.assertEqual(R.READY_STRETCH_MIN, 2)

    def test_owners_are_used_and_not_copied(self):
        self.assertIs(R.GAP_MS, sp_tools.GAP_MS)
        self.assertEqual(R.ALWAYS_LISTED_NAMES, quiet.COMPILER_NAMES + quiet.REPORTED_NAMES)


class CpuShares(ReportCase):
    def fixture(self):
        """100 samples at 20 ms and four processes. The compiler runs only in
        samples 50 to 59, and the game drops to 5 ms a slice there."""
        fx = Fixture()
        fx.steady(100)
        for n in range(100):
            fx.thread(n)
            busy = 50 <= n <= 59
            fx.process(n, GAME, "Clone Hero.exe", cpu_ms=5 if busy else 10)
            fx.process(n, WORKER, "python.exe", cpu_ms=2)
            fx.process(n, RECORDER, "python.exe", cpu_ms=3)
            if busy:
                fx.process(n, COMPILER, "cl.exe", cpu_ms=20)
        return fx

    def test_whole_recording(self):
        lines = self.lines(self.fixture())
        self.assertIn("window: the whole recording", lines)
        self.assertIn("    47.5%  pid 7 Clone Hero.exe (game)", lines)
        self.assertIn("    10.1%  pid 11 cl.exe", lines)
        self.assertIn("    15.0%  pid 9 python.exe (recorder)", lines)

    def test_the_window_changes_the_shares(self):
        lines = self.lines(self.fixture(), "--from", wall_text(50 * 20), "--to", wall_text(59 * 20))
        self.assertIn("window: wall 1001.000 to 1001.180, 200.0 ms of slices", lines)
        self.assertIn("    47.5%   25.0%  pid 7 Clone Hero.exe (game)", lines)
        self.assertIn("    10.1%  100.0%  pid 11 cl.exe", lines)
        self.assertIn("    15.0%   15.0%  pid 9 python.exe (recorder)", lines)

    def test_a_process_stamped_on_the_window_edge_is_inside(self):
        lines = self.lines(self.fixture(), "--from", wall_text(59 * 20), "--to", wall_text(59 * 20))
        self.assertIn("    10.1%  100.0%  pid 11 cl.exe", lines)

    def test_the_first_sample_ends_no_slice(self):
        fx = self.fixture()
        fx.process(0, GAME, "Clone Hero.exe", cpu_ms=100000)
        self.assertIn("    47.5%  pid 7 Clone Hero.exe (game)", self.lines(fx))

    def test_always_listed_names_print_at_zero(self):
        lines = self.lines(self.fixture())
        for name in ("link", "MSBuild", "Spotify.exe", "MsMpEng.exe"):
            self.assertIn(f"     0.0%  pid - {name}", lines)
        self.assertNotIn("     0.0%  pid - cl", lines)

    def test_a_listed_process_with_rows_but_no_cpu_keeps_its_pid(self):
        fx = self.fixture()
        fx.process(3, 44, "Spotify.exe", read=5)
        lines = self.lines(fx)
        self.assertIn("     0.0%  pid 44 Spotify.exe", lines)
        self.assertNotIn("     0.0%  pid - Spotify.exe", lines)

    def test_a_process_that_gained_nothing_and_is_not_listed_stays_out(self):
        fx = self.fixture()
        fx.process(3, 45, "idle.exe", read=5)
        self.assertFalse([line for line in self.lines(fx) if "%" in line and "idle.exe" in line])

    def test_a_window_with_no_slice_is_refused(self):
        code, lines = self.run_report(self.fixture(), "--from", "5000")
        self.assertEqual(code, 1)
        self.assertIn("refused: no slice ends in that window; the recording runs from "
                      "wall 1000.000 to 1001.980", lines)


class ReadyStretches(ReportCase):
    def fixture(self):
        """The game's thread is ready in samples 3 to 5 and its second thread
        in sample 8 alone. Two processes gain CPU in samples 3 to 5, and one
        of them again outside the stretch."""
        fx = Fixture()
        fx.steady(12)
        for n in (3, 4, 5):
            fx.thread(n, reads_as="ready")
            fx.process(n, COMPILER, "cl.exe", cpu_ms=15)
            fx.process(n, 12, "MsMpEng.exe", cpu_ms=4)
        fx.process(9, 13, "later.exe", cpu_ms=20)
        fx.thread(8, tid=71, reads_as="ready")
        fx.thread(9)
        return fx

    def test_three_ready_samples_are_one_stretch(self):
        self.assertIn("  game pid 7 thread 70: running 1, ready 3, longest ready stretch 40.0 ms "
                      "from wall 1000.060, CPU gained in those samples: "
                      "pid 11 cl.exe 45.0 ms, pid 12 MsMpEng.exe 12.0 ms", self.lines(self.fixture()))

    def test_one_ready_sample_counts_but_is_no_stretch(self):
        self.assertIn("  game pid 7 thread 71: running 0, ready 1, no ready stretch",
                      self.lines(self.fixture()))

    def test_two_ready_samples_are_the_boundary(self):
        fx = Fixture()
        fx.steady(6)
        fx.thread(2, reads_as="ready")
        fx.thread(3, reads_as="ready")
        self.assertIn("  game pid 7 thread 70: running 0, ready 2, longest ready stretch 20.0 ms "
                      "from wall 1000.040, CPU gained in those samples: by nobody", self.lines(fx))

    def test_a_gap_in_the_samples_breaks_a_stretch(self):
        self.assertEqual(R.ready_stretches([1, 2, 4, 6, 7, 8]), [[1, 2], [6, 7, 8]])

    def test_the_window_clips_a_stretch(self):
        lines = self.lines(self.fixture(), "--from", wall_text(100), "--to", wall_text(200))
        self.assertIn("  game pid 7 thread 70: running 1, ready 1, no ready stretch", lines)
        self.assertIn("threads in the window, out of 6 samples:", lines)

    def test_a_row_is_read_by_its_word_and_never_its_state_number(self):
        fx = Fixture()
        fx.steady(4)
        for n in (1, 2):
            fx.thread(n, reads_as="other")
        self.assertIn("  game pid 7 thread 70: running 0, ready 0, no ready stretch", self.lines(fx))

    def test_worker_and_recorder_threads_print_too(self):
        fx = self.fixture()
        fx.thread(2, pid=WORKER, tid=80)
        fx.thread(2, pid=RECORDER, tid=90)
        fx.process(2, WORKER, "python.exe", parent=6)
        fx.process(2, 6, "py.exe", parent=5)
        lines = self.lines(fx, "--worker", str(WORKER))
        self.assertIn("  worker pid 8 thread 80: running 1, ready 0, no ready stretch", lines)
        self.assertIn("  recorder pid 9 thread 90: running 1, ready 0, no ready stretch", lines)

    def test_a_pid_with_no_rows_in_the_window_says_so(self):
        self.assertIn("  recorder pid 9: no thread rows in the window", self.lines(self.fixture()))

    def test_one_pid_in_two_roles_prints_once(self):
        fx = self.fixture()
        fx.process(2, GAME, "pwsh.exe", parent=6)
        lines = self.lines(fx, "--worker", str(GAME))
        self.assertEqual(sum("thread 71" in line for line in lines), 1)
        self.assertIn("  game and worker pid 7 thread 71: running 0, ready 1, no ready stretch",
                      lines)


class ClockSteps(ReportCase):
    SIX_PAIRS = ((20, 20), (20, 20), (70, 20), (300, 100), (100, 100), (20, 80))

    def test_six_pairs(self):
        lines = self.lines(clock_fixture(self.SIX_PAIRS))
        self.assertIn("clock reads dropped, their two stamps over one period (20 ms) apart: 0",
                      lines)
        self.assertIn("clock steps: count 6, median 45.0 ms, 99th percentile 290.0 ms", lines)
        self.assertIn("clock steps with an excess over 40 ms by size: 3", lines)
        at = lines.index("clock steps with an excess over 40 ms by size: 3")
        self.assertEqual(lines[at + 2:at + 5], [
            "  wall 1000.160: step 300.0 ms, wall gap 100.0 ms, excess +200.0 ms",
            "  wall 1000.340: step 20.0 ms, wall gap 80.0 ms, excess -60.0 ms",
            "  wall 1000.060: step 70.0 ms, wall gap 20.0 ms, excess +50.0 ms"])

    def test_the_late_sample_is_not_listed(self):
        lines = self.lines(clock_fixture(self.SIX_PAIRS))
        self.assertFalse([line for line in lines if "wall 1000.260" in line])

    def test_six_pairs_lead(self):
        self.assertIn("clock lead over wall time, against its median offset: "
                      "largest +200.0 ms at wall 1000.160, lowest -50.0 ms at wall 1000.000",
                      self.lines(clock_fixture(self.SIX_PAIRS)))

    def test_an_excess_of_exactly_the_limit_is_not_long(self):
        lines = self.lines(clock_fixture(((60, 20), (20, 60))))
        self.assertIn("clock steps with an excess over 40 ms by size: 0", lines)

    def test_an_excess_just_over_the_limit_is_long(self):
        lines = self.lines(clock_fixture(((61, 20), (20, 61))))
        self.assertIn("clock steps with an excess over 40 ms by size: 2", lines)

    def test_the_list_stops_at_ten_largest(self):
        pairs = [(20 + 50 + i, 20) for i in range(12)]
        lines = self.lines(clock_fixture(pairs))
        self.assertIn("clock steps with an excess over 40 ms by size: 12", lines)
        listed = [line for line in lines if line.startswith("  wall ")]
        self.assertEqual(len(listed), 10)
        self.assertIn("excess +61.0 ms", listed[0])
        self.assertIn("excess +52.0 ms", listed[-1])

    def test_steps_in_a_window_are_the_ones_ending_in_it(self):
        lines = self.lines(clock_fixture(self.SIX_PAIRS),
                           "--from", wall_text(60), "--to", wall_text(160))
        self.assertIn("clock steps: count 2, median 185.0 ms, 99th percentile 297.7 ms", lines)

    def test_a_sample_with_no_clock_ends_the_run_of_pairs(self):
        fx = Fixture()
        fx.thread(fx.sample(0, 0))
        fx.sample(20, 20)
        fx.sample(40)
        fx.sample(60, 500)
        fx.sample(80, 520)
        self.assertIn("clock steps: count 2, median 20.0 ms, 99th percentile 20.0 ms",
                      self.lines(fx))

    def test_no_clock(self):
        fx = Fixture()
        fx.steady(5)
        fx.thread(1)
        self.assertIn("no clock in this recording", self.lines(fx))

    def test_a_clock_with_no_stamps_is_refused(self):
        fx = clock_fixture(self.SIX_PAIRS)
        fx.samples[2][7] = ""
        code, lines = self.run_report(fx)
        self.assertEqual(code, 1)
        self.assertIn("sample 2 has a clock reading with no wall stamps", lines[0])


class MatchedPair(ReportCase):
    """A stall inside the clock read: the row's wall stamp comes 165 ms late, so
    its excess is -165 ms and the next row's is +165 ms, as in LS-O2's watch."""

    def fixture(self, read_ms):
        fx = Fixture()
        fx.thread(fx.sample(0, 0))
        fx.sample(20, 20)
        fx.sample(40 + 165, 40, read_ms=read_ms)
        fx.sample(60 + 165, 60 + 165)
        fx.sample(80 + 165, 80 + 165)
        return fx

    def test_it_is_dropped_and_not_listed(self):
        lines = self.lines(self.fixture(read_ms=166))
        self.assertIn("clock reads dropped, their two stamps over one period (20 ms) apart: 1",
                      lines)
        self.assertIn("clock steps with an excess over 40 ms by size: 0", lines)
        self.assertFalse([line for line in lines if "165.0 ms" in line])
        self.assertIn("clock lead over wall time, against its median offset: "
                      "largest +0.0 ms at wall 1000.000, lowest +0.0 ms at wall 1000.000", lines)

    def test_the_neighbours_of_a_dropped_sample_pair_up(self):
        self.assertIn("clock steps: count 3, median 20.0 ms, 99th percentile 201.3 ms",
                      self.lines(self.fixture(read_ms=166)))

    def test_with_tight_stamps_the_same_rows_are_kept(self):
        lines = self.lines(self.fixture(read_ms=1))
        self.assertIn("clock reads dropped, their two stamps over one period (20 ms) apart: 0",
                      lines)
        self.assertIn("clock steps with an excess over 40 ms by size: 2", lines)

    def test_stamps_exactly_one_period_apart_are_kept(self):
        self.assertIn("clock reads dropped, their two stamps over one period (20 ms) apart: 0",
                      self.lines(self.fixture(read_ms=PERIOD_MS)))

    def test_stamps_just_over_one_period_apart_are_dropped(self):
        self.assertIn("clock reads dropped, their two stamps over one period (20 ms) apart: 1",
                      self.lines(self.fixture(read_ms=PERIOD_MS + 1)))

    def test_only_drops_in_the_window_are_counted(self):
        self.assertIn("clock reads dropped, their two stamps over one period (20 ms) apart: 0",
                      self.lines(self.fixture(read_ms=166), "--from", wall_text(225)))


class SampleGaps(ReportCase):
    def test_a_120_ms_gap(self):
        fx = Fixture()
        fx.thread(fx.sample(0))
        for ms in (20, 40, 160, 180):
            fx.sample(ms)
        self.assertIn("gaps between samples: median 20.0 ms, worst 120.0 ms, 1 over 50 ms "
                      "(the recorder's own stalls)", self.lines(fx))

    def test_a_gap_of_exactly_gap_ms_is_not_over(self):
        fx = Fixture()
        fx.thread(fx.sample(0))
        fx.sample(sp_tools.GAP_MS)
        self.assertIn("gaps between samples: median 50.0 ms, worst 50.0 ms, 0 over 50 ms "
                      "(the recorder's own stalls)", self.lines(fx))


class Cost(ReportCase):
    def test_share_and_bytes_against_the_files(self):
        fx = Fixture()
        fx.steady(11)
        fx.thread(1)
        for n in range(1, 11):
            fx.process(n, RECORDER, "python.exe", cpu_ms=5, wrote=300)
        fx.write(self.folder)
        size = sum(os.path.getsize(os.path.join(self.folder, name))
                   for name in (R.SAMPLES_FILE, R.PROCESSES_FILE, R.THREADS_FILE))
        lines = self.lines(fx)
        self.assertIn("recorder cost: pid 9, 25.0% of one core over the recording", lines)
        self.assertIn(f"recorder cost: wrote 3000 bytes, its three files hold {size} bytes", lines)


class FirstLine(ReportCase):
    def test_counts_the_pids_thread_samples(self):
        fx = Fixture()
        fx.steady(4)
        fx.thread(1)
        fx.thread(2)
        fx.thread(2, tid=71)
        fx.thread(2, pid=WORKER, tid=80)
        self.assertEqual(self.lines(fx)[0], "the recording holds 3 thread samples for pid 7")

    def test_none_is_refused_naming_the_pid(self):
        fx = Fixture()
        fx.steady(4)
        fx.thread(1, pid=WORKER, tid=80)
        code, lines = self.run_report(fx)
        self.assertEqual(code, 1)
        self.assertEqual(lines, ["the recording holds 0 thread samples for pid 7",
                                 "refused: no thread rows for pid 7, "
                                 "so this recording is not of that process"])


class ByProcess(ReportCase):
    def fixture(self):
        fx = Fixture()
        fx.steady(6)
        fx.thread(1)
        fx.process(1, 20, "out.exe", read=1000, faults=9)
        fx.process(3, 21, "in.exe", read=10, wrote=20, faults=2)
        fx.process(4, 21, "in.exe", read=5, wrote=1, faults=1)
        return fx

    def test_bytes_in_the_window(self):
        lines = self.lines(self.fixture(), "--from", wall_text(60), "--to", wall_text(80))
        at = lines.index("bytes by process in the window:")
        self.assertEqual(lines[at + 1:at + 2], ["  pid 21 in.exe: read 15, wrote 21"])
        self.assertFalse([line for line in lines if "out.exe" in line])

    def test_hard_faults_carry_the_mark(self):
        lines = self.lines(self.fixture(), "--from", wall_text(60), "--to", wall_text(80))
        at = lines.index("hard faults by process in the window (meaning unchecked):")
        self.assertEqual(lines[at + 1:], ["  pid 21 in.exe: 3"])


class SettingsHeader(ReportCase):
    def fixture(self):
        fx = Fixture()
        fx.steady(3)
        fx.thread(1)
        return fx

    def write_settings(self, text):
        with open(os.path.join(self.documents, "settings.ini"), "w", encoding="utf-8") as f:
            f.write(text)

    def test_prints_the_two_lines(self):
        self.write_settings("[game]\nframe_x = 1\n[video]\nframerate = 330\nvsync = 0\nother = 1\n")
        self.assertEqual(self.lines(self.fixture())[1:3],
                         ["settings.ini: framerate = 330", "settings.ini: vsync = 0"])

    def test_a_line_that_is_not_there_is_said(self):
        self.write_settings("[video]\nvsync = 1\n")
        self.assertEqual(self.lines(self.fixture())[1:3],
                         ["settings.ini: no framerate line", "settings.ini: vsync = 1"])

    def test_a_missing_file_is_said_and_the_report_goes_on(self):
        lines = self.lines(self.fixture())
        self.assertIn("settings.ini: could not read", lines[1])
        self.assertIn("no clock in this recording", lines)


class TheFront(ReportCase):
    def fixture(self):
        fx = Fixture()
        fx.steady(4)
        fx.thread(1)
        fx.process(1, WORKER, "python.exe", cpu_ms=2, parent=30)
        fx.process(0, 30, "py.exe", parent=31)
        fx.process(2, 31, "python.exe", cpu_ms=1, parent=32)
        return fx

    def test_the_grandparent_is_the_front(self):
        lines = self.lines(self.fixture(), "--worker", str(WORKER))
        self.assertIn("worker 8: its parent is pid 30 py.exe", lines)
        self.assertIn("front: pid 31 python.exe, the parent of pid 30", lines)
        self.assertIn("     3.3%  pid 8 python.exe (worker)", lines)
        self.assertIn("     1.7%  pid 31 python.exe (front)", lines)

    def test_a_front_with_no_cpu_rows_is_still_listed(self):
        fx = self.fixture()
        fx.processes = [row for row in fx.processes if row[1] != 31]
        lines = self.lines(fx, "--worker", str(WORKER))
        self.assertIn("front: pid 31 (no rows), the parent of pid 30", lines)
        self.assertIn("     0.0%  pid 31 (no rows) (front)", lines)

    def test_a_launcher_with_no_row_says_the_front_is_not_known(self):
        fx = self.fixture()
        fx.processes = [row for row in fx.processes if row[1] != 30]
        self.assertIn("worker 8: its parent is pid 30, which has no row in processes.csv, "
                      "so the front is not known", self.lines(fx, "--worker", str(WORKER)))

    def test_a_worker_with_no_row_says_so(self):
        self.assertIn("worker 77: no row in processes.csv, so the front is not known",
                      self.lines(self.fixture(), "--worker", "77"))

    def test_no_worker_prints_no_front(self):
        self.assertFalse([line for line in self.lines(self.fixture()) if "front" in line])


class TheRecordingsEnd(ReportCase):
    def fixture(self):
        fx = Fixture()
        fx.steady(4)
        fx.thread(1)
        return fx

    def test_the_last_row_says_why_it_stopped(self):
        fx = self.fixture()
        fx.stop("seconds", "9.5")
        self.assertIn("recorder stopped: seconds, its own process time 9.5 s", self.lines(fx))

    def test_a_missing_last_row_is_said_and_is_no_failure(self):
        lines = self.lines(self.fixture())
        self.assertIn("recorder stopped: not said, the last row of samples.csv is missing, "
                      "so the recorder was probably terminated", lines)

    def test_a_half_written_row_is_left_out_and_counted(self):
        fx = self.fixture()
        fx.write(self.folder)
        with open(os.path.join(self.folder, R.PROCESSES_FILE), "a", encoding="utf-8") as f:
            f.write("3,9,1")
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = R.main([self.folder, str(GAME)], documents_dir=self.documents)
        self.assertEqual(code, 0)
        self.assertIn("left out 1 half-written rows", out.getvalue().splitlines())


class BadFolders(ReportCase):
    def test_a_missing_file_is_refused(self):
        fx = Fixture()
        fx.steady(3)
        fx.write(self.folder)
        os.remove(os.path.join(self.folder, R.THREADS_FILE))
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = R.main([self.folder, str(GAME)], documents_dir=self.documents)
        self.assertEqual(code, 1)
        self.assertIn("refused: could not read", out.getvalue())

    def test_a_missing_column_is_refused(self):
        fx = Fixture()
        fx.steady(3)
        fx.write(self.folder)
        with open(os.path.join(self.folder, R.THREADS_FILE), "w", encoding="utf-8") as f:
            f.write("sample,pid,tid,state\n")
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = R.main([self.folder, str(GAME)], documents_dir=self.documents)
        self.assertEqual(code, 1)
        self.assertIn("has no column wait_reason, reads_as, switches, cpu_ms", out.getvalue())

    def test_one_sample_is_refused(self):
        fx = Fixture()
        fx.steady(1)
        fx.thread(0)
        code, lines = self.run_report(fx)
        self.assertEqual(code, 1)
        self.assertEqual(lines[1], "refused: samples.csv holds 1 whole samples, "
                                   "and a slice needs two")


class AsAScript(ReportCase):
    def test_runs_under_py_dash_i(self):
        fx = Fixture()
        fx.steady(3)
        fx.thread(1)
        fx.write(self.folder)
        done = subprocess.run(
            [sys.executable, "-I", os.path.join(REPO, "tools", "ch_mcp", "machine_report.py"),
             self.folder, str(GAME)], capture_output=True, text=True, timeout=60, cwd=self.folder)
        self.assertEqual(done.returncode, 0, done.stderr)
        self.assertEqual(done.stdout.splitlines()[0],
                         "the recording holds 1 thread samples for pid 7")


if __name__ == "__main__":
    unittest.main()
