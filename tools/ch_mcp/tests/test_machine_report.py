"""The report over the machine recorder's files. The tests whose classes end in
"InARealRecording" read excerpts of a real recording under testdata/ch_runs; the
rest build their rows here from the three files' columns. No test reads the live
settings.ini.
"""
import contextlib
import csv
import io
import os
import re
import statistics
import subprocess
import sys
import tempfile
import unittest
from fractions import Fraction

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
if REPO not in sys.path:
    # Run as a script (py -I leaves the repo off sys.path).
    sys.path.insert(0, REPO)

from tools.ch_mcp import machine_recorder  # noqa: E402
from tools.ch_mcp import machine_report as R  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402

GAME, WORKER, RECORDER = 7, 8, 9
GAME_TID = 70
BASE_S = 1000
PERIOD_MS = 20

# Samples 1650 to 1700 of the three files, header rows kept, copied unchanged from
# hydra-data/ch-mcp-runs/2026-11-07/ls-t3-run-b/machine-1: Clone Hero played
# while a build ran, so compilers, ready game threads and idle processes are all in it.
REAL_RUN = os.path.join(REPO, "testdata", "ch_runs", "ls-t3-run-b", "machine-1")
# Samples 1700 to 1760 of the same run, copied the same way. The build made the
# recorder late here, so ready stretches cross late samples and the clock has long excesses.
LATE_RUN = os.path.join(REPO, "testdata", "ch_runs", "ls-t3-run-b", "machine-1-samples-1700-1760")
# Samples 82 to 88 of the same run, copied the same way: the game's threads are in
# from sample 83, and the worker's (pid 24320, the game's parent) first come at sample 88.
RUN_START = os.path.join(REPO, "testdata", "ch_runs", "ls-t3-run-b", "machine-1-samples-82-88")
GAME_NAME = "Clone Hero.exe"


def real_rows(file_name, folder=REAL_RUN):
    with open(os.path.join(folder, file_name), newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def real_game_pid(folder=REAL_RUN):
    pids = {int(row["pid"]) for row in real_rows(R.PROCESSES_FILE, folder)
            if row["name"] == GAME_NAME}
    assert len(pids) == 1, pids
    return pids.pop()


def real_walls(folder=REAL_RUN):
    """Each sample number's wall time, as the text samples.csv holds."""
    return {int(row["sample"]): row["wall_s"] for row in real_rows(R.SAMPLES_FILE, folder)}


def wall_text(ms):
    """A wall time in the file's form, `ms` after the fixture's start."""
    return f"{BASE_S + C.ms_to_s(ms):.3f}"


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
            clock = [f"{C.ms_to_s(clock_ms):.3f}", wall_text(wall_ms), wall_text(wall_ms + read_ms)]
        self.samples.append([number, wall_text(wall_ms), 1, 400, 5000, 2] + clock
                            + [RECORDER, PERIOD_MS, "", ""])
        return number

    def steady(self, count, period_ms=PERIOD_MS):
        for i in range(count):
            self.sample(i * period_ms)

    def process(self, sample, pid, name, cpu_ms=0, read=0, wrote=0, faults=0, parent=1):
        self.processes.append([sample, pid, parent, name, cpu_ms, read, wrote, faults])

    def thread(self, sample, pid=GAME, tid=GAME_TID, reads_as="running", switches=1):
        self.threads.append([sample, pid, tid, 2, 0, reads_as, switches, 1])

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

    def test_the_watchs_gap_threshold_and_the_listed_names(self):
        self.assertEqual(R.GAP_MS, 50)
        self.assertEqual(R.ALWAYS_LISTED_NAMES,
                         ("cl", "link", "MSBuild", "cmake", "ninja", "clang-cl", "lld-link", "rc",
                          "mt", "tsc", "esbuild", "Spotify.exe", "MsMpEng.exe"))

    def test_the_recorders_files_and_words_are_the_ones_read(self):
        for name in R.SAMPLE_OPTIONAL:
            self.assertIn(name, machine_recorder.SAMPLES_COLUMNS)
        for name in R.PROCESS_OPTIONAL:
            self.assertIn(name, machine_recorder.PROCESSES_COLUMNS)
        # The words the report counts are the ones a real recorder wrote.
        words = {row["reads_as"] for row in real_rows(R.THREADS_FILE)}
        self.assertIn(R.READS_READY, words)
        self.assertIn(R.READS_RUNNING, words)


class RealRecordingCase(unittest.TestCase):
    """Runs the report on FOLDER, the excerpt of a real recording."""

    FOLDER = REAL_RUN
    TABLE_ROW = re.compile(r"^\s+(\d+\.\d)%(?:\s+(\d+\.\d)%)?  (.*)$")

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.documents = tmp.name
        self.game = real_game_pid(self.FOLDER)
        self.walls = real_walls(self.FOLDER)

    def report(self, *args):
        """(exit code, output lines) of main on the real recording."""
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = R.main([self.FOLDER, str(self.game), *args], documents_dir=self.documents)
        return code, out.getvalue().splitlines()

    def lines(self, *args):
        code, lines = self.report(*args)
        self.assertEqual(code, 0, lines)
        return lines

    def rows(self, file_name):
        return real_rows(file_name, self.FOLDER)

    def thread_rows(self, pid, tid=None):
        return [row for row in self.rows(R.THREADS_FILE)
                if int(row["pid"]) == pid and (tid is None or int(row["tid"]) == tid)]

    def line_of(self, lines, pid, tid):
        """The one thread line for this pid and thread, whatever its role label."""
        found = [line for line in lines if re.match(rf"  [a-z ]+ pid {pid} thread {tid}:", line)]
        self.assertEqual(len(found), 1, lines)
        return found[0]

    def counts_from_rows(self, rows):
        """The line's "running N, ready M" for these threads.csv rows, by their word."""
        running = sum(row["reads_as"] == R.READS_RUNNING for row in rows)
        ready = sum(row["reads_as"] == R.READS_READY for row in rows)
        return f"running {running}, ready {ready}, "

    def gap_ms(self, earlier, later):
        return (Fraction(self.walls[later]) - Fraction(self.walls[earlier])) * 1000

    def late_limit_ms(self):
        return R.LONG_STEP_FACTOR * Fraction(self.rows(R.SAMPLES_FILE)[0]["period_ms"])

    def table(self, lines):
        """The CPU share table, in its printed order, as (who, shares) pairs;
        `who` is the row's text after its shares, label included."""
        start = next(at for at, line in enumerate(lines) if line.startswith("CPU share of one core"))
        rows = []
        for line in lines[start + 1:]:
            found = self.TABLE_ROW.match(line)
            if not found:
                break
            shares = [float(found.group(1))] + ([float(found.group(2))] if found.group(2) else [])
            rows.append((found.group(3), shares))
        self.assertTrue(rows, lines)
        return rows

    @staticmethod
    def who(pid, name):
        return f"pid {pid} {name or R.NO_NAME}"

    def shares_of(self, table, pid, name):
        """The shares of one process's row, or None when it has no row."""
        key = self.who(pid, name)
        found = [shares for who, shares in table if who == key or who.startswith(key + " (")]
        self.assertLessEqual(len(found), 1, key)
        return found[0] if found else None

    def recorder_pid(self):
        return int(real_rows(R.SAMPLES_FILE)[0]["recorder_pid"])

    def cpu_by_process(self, first_sample_too=True):
        """Each (pid, name) in processes.csv and the CPU its rows hold."""
        first = min(self.walls)
        totals = {}
        for row in real_rows(R.PROCESSES_FILE):
            key = (int(row["pid"]), row["name"])
            gained = Fraction(row["cpu_ms"])
            if not first_sample_too and int(row["sample"]) == first:
                gained = Fraction(0)
            totals[key] = totals.get(key, Fraction(0)) + gained
        return totals


class CpuShares(RealRecordingCase):
    def test_whole_recording(self):
        lines = self.lines()
        self.assertIn("window: the whole recording", lines)
        table = self.table(lines)
        self.assertIn(f"{self.who(self.game, GAME_NAME)} (game)", [who for who, _ in table])
        recorder = self.recorder_pid()
        recorder_row = [shares for who, shares in table
                        if who.startswith(f"pid {recorder} ") and who.endswith("(recorder)")]
        self.assertEqual(len(recorder_row), 1, table)
        # The table and the recorder cost line are two prints of one share.
        cost = next(line for line in lines if line.startswith(f"recorder cost: pid {recorder},"))
        self.assertEqual(float(re.search(r", (\d+\.\d)% of one core", cost).group(1)),
                         recorder_row[0][0])
        shares = [shares[0] for _, shares in table]
        self.assertEqual(shares, sorted(shares, reverse=True))

    def test_the_window_changes_the_shares(self):
        whole = dict(self.table(self.lines()))
        start, end = self.walls[1673], self.walls[1676]
        lines = self.lines("--from", start, "--to", end)
        self.assertTrue(any(line.startswith(f"window: wall {R._wall(Fraction(start))} to "
                                            f"{R._wall(Fraction(end))}, ") for line in lines),
                        lines)
        windowed = self.table(lines)
        self.assertEqual({who: shares[0] for who, shares in windowed},
                         {who: shares[0] for who, shares in whole.items()})
        self.assertTrue(all(len(shares) == 2 for _, shares in windowed))
        self.assertTrue([who for who, shares in windowed if shares[0] != shares[1]])

    def test_a_process_stamped_on_the_window_edge_is_inside(self):
        edge = 1675
        lines = self.lines("--from", self.walls[edge], "--to", self.walls[edge])
        self.assertIn("threads in the window, out of 1 samples:", lines)
        table = self.table(lines)
        gained_there = {(int(row["pid"]), row["name"]) for row in real_rows(R.PROCESSES_FILE)
                        if int(row["sample"]) == edge and Fraction(row["cpu_ms"]) > 0}
        self.assertTrue(gained_there)
        for pid, name in gained_there:
            self.assertGreater(self.shares_of(table, pid, name)[1], 0, (pid, name))
        elsewhere = [key for key, ms in self.cpu_by_process(first_sample_too=False).items()
                     if ms > 0 and key not in gained_there]
        self.assertTrue(elsewhere)
        for pid, name in elsewhere:
            self.assertEqual(self.shares_of(table, pid, name)[1], 0.0, (pid, name))

    def test_the_first_sample_ends_no_slice(self):
        rec = R.read_recording(REAL_RUN)
        first, rest = rec.samples[0], rec.samples[1:]
        self.assertEqual(R.window_of(rec.samples, None, None).slices,
                         {sample.number for sample in rest})
        # A window that holds only the first sample has no slice to report.
        code, lines = self.report("--from", self.walls[first.number],
                                  "--to", self.walls[first.number])
        self.assertEqual(code, 1, lines)
        second = self.walls[rest[0].number]
        self.lines("--from", second, "--to", second)

    def test_always_listed_names_print_at_zero(self):
        table = dict(self.table(self.lines()))
        stems = {R._stem(row["name"]) for row in real_rows(R.PROCESSES_FILE)}
        absent = [name for name in R.ALWAYS_LISTED_NAMES if R._stem(name) not in stems]
        present = [name for name in R.ALWAYS_LISTED_NAMES if R._stem(name) in stems]
        # This recording holds both kinds, so both halves of the rule are checked.
        self.assertTrue(absent)
        self.assertTrue(present)
        for name in absent:
            self.assertEqual(table.get(f"pid - {name}"), [0.0], name)
        for name in present:
            self.assertNotIn(f"pid - {name}", table, name)

    def test_a_listed_process_with_rows_but_no_cpu_keeps_its_pid(self):
        table = self.table(self.lines())
        idle_listed = [(pid, name) for (pid, name), ms in self.cpu_by_process().items()
                       if ms == 0 and R._always_listed(name)]
        self.assertTrue(idle_listed)
        for pid, name in idle_listed:
            self.assertEqual(self.shares_of(table, pid, name), [0.0], (pid, name))

    def test_a_process_that_gained_nothing_and_is_not_listed_stays_out(self):
        table = self.table(self.lines())
        roles = {self.game, self.recorder_pid()}
        idle = [(pid, name) for (pid, name), ms in self.cpu_by_process().items()
                if ms == 0 and not R._always_listed(name) and pid not in roles]
        self.assertTrue(idle)
        for pid, name in idle:
            self.assertIsNone(self.shares_of(table, pid, name), (pid, name))

    def test_a_window_with_no_slice_is_refused(self):
        first, last = self.walls[min(self.walls)], self.walls[max(self.walls)]
        code, lines = self.report("--from", str(Fraction(last) + 1))
        self.assertEqual(code, 1)
        refusal = lines[-1]
        self.assertTrue(refusal.startswith("refused: "), lines)
        self.assertIn(R._wall(Fraction(first)), refusal)
        self.assertIn(R._wall(Fraction(last)), refusal)
        self.assertFalse([line for line in lines if line.startswith("CPU share")])


class ReadyStretchesInARealRecording(RealRecordingCase):
    def ready_samples(self, tid):
        """The samples in which this game thread reads as ready, from threads.csv."""
        return sorted(int(row["sample"]) for row in real_rows(R.THREADS_FILE)
                      if int(row["pid"]) == self.game and int(row["tid"]) == tid
                      and row["reads_as"] == R.READS_READY)

    def thread_line(self, lines, tid):
        found = [line for line in lines if line.startswith(f"  game pid {self.game} thread {tid}:")]
        self.assertEqual(len(found), 1, lines)
        return found[0]

    def test_consecutive_ready_samples_are_one_stretch(self):
        self.assertEqual(self.ready_samples(9856), [1673, 1674, 1675, 1676])
        line = self.thread_line(self.lines(), 9856)
        self.assertIn("ready 4, longest ready stretch: ready in 4 consecutive samples", line)
        self.assertIn(f"from wall {R._wall(Fraction(self.walls[1673]))},", line)

    def test_one_ready_sample_counts_but_is_no_stretch(self):
        self.assertEqual(self.ready_samples(5080), [1681])
        self.assertTrue(self.thread_line(self.lines(), 5080).endswith(
            "ready 1, no ready stretch"))

    def test_two_ready_samples_are_the_boundary(self):
        self.assertEqual(self.ready_samples(23748), [1670, 1671, 1677, 1680])
        # Its one run of consecutive samples, 1670 and 1671, is exactly the shortest stretch.
        self.assertEqual(len([1670, 1671]), R.READY_STRETCH_MIN)
        line = self.thread_line(self.lines(), 23748)
        self.assertIn("ready 4, longest ready stretch: ready in 2 consecutive samples", line)
        self.assertIn(f"from wall {R._wall(Fraction(self.walls[1670]))},", line)

    def test_the_window_clips_a_stretch(self):
        self.assertEqual(self.ready_samples(9856), [1673, 1674, 1675, 1676])
        line = self.thread_line(self.lines("--from", self.walls[1675]), 9856)
        self.assertIn("ready 2, longest ready stretch: ready in 2 consecutive samples", line)
        self.assertIn(f"from wall {R._wall(Fraction(self.walls[1675]))},", line)
        lines = self.lines("--from", self.walls[1676])
        self.assertTrue(self.thread_line(lines, 9856).endswith("ready 1, no ready stretch"))
        inside = sum(number >= 1676 for number in self.walls)
        self.assertIn(f"threads in the window, out of {inside} samples:", lines)


class ReadyStretches(ReportCase):
    def test_a_gap_in_the_samples_breaks_a_stretch(self):
        self.assertEqual(R.ready_stretches([1, 2, 4, 6, 7, 8]), [[1, 2], [6, 7, 8]])


class ThreadWordsInARealRecording(RealRecordingCase):
    def test_a_row_is_read_by_its_word_and_never_its_state_number(self):
        tid = 4248
        mine = self.thread_rows(self.game, tid)
        # This thread's ready rows carry more than one state number, so a count by
        # one state number would differ from the count by word.
        ready_states = {row["state"] for row in mine if row["reads_as"] == R.READS_READY}
        self.assertGreater(len(ready_states), 1, ready_states)
        line = self.line_of(self.lines(), self.game, tid)
        self.assertTrue(line.startswith(f"  game pid {self.game} thread {tid}: "
                                        + self.counts_from_rows(mine)), line)


def game_parent(case):
    """The game's parent in processes.csv: the worker that launched it in this run."""
    parents = {int(row["parent_pid"]) for row in case.rows(R.PROCESSES_FILE)
               if int(row["pid"]) == case.game}
    case.assertEqual(len(parents), 1, parents)
    return parents.pop()


class ThreadRolesInARealRecording(RealRecordingCase):
    FOLDER = LATE_RUN

    def test_worker_and_recorder_threads_print_too(self):
        worker = game_parent(self)
        recorder = self.recorder_pid()
        self.assertNotIn(recorder, (worker, self.game))
        lines = self.lines("--worker", str(worker))
        for label, pid in (("worker", worker), ("recorder", recorder)):
            tids = sorted({int(row["tid"]) for row in self.thread_rows(pid)})
            self.assertTrue(tids, pid)
            for tid in tids:
                expected = (f"  {label} pid {pid} thread {tid}: "
                            + self.counts_from_rows(self.thread_rows(pid, tid)))
                self.assertTrue(self.line_of(lines, pid, tid).startswith(expected), expected)

    def test_one_pid_in_two_roles_prints_once(self):
        lines = self.lines("--worker", str(self.game))
        tids = sorted({int(row["tid"]) for row in self.thread_rows(self.game)})
        self.assertTrue(tids)
        for tid in tids:
            # line_of finds exactly one line for the thread, whatever its label.
            self.assertTrue(self.line_of(lines, self.game, tid).startswith(
                f"  game and worker pid {self.game} thread {tid}: "))


class NoThreadRowsInARealRecording(RealRecordingCase):
    FOLDER = RUN_START

    def test_a_pid_with_no_rows_in_the_window_says_so(self):
        worker = game_parent(self)
        last = max(self.walls)
        # The worker's threads first come in the excerpt's last sample, so a window
        # that ends one sample earlier holds none of them.
        self.assertEqual({int(row["sample"]) for row in self.thread_rows(worker)}, {last})
        message = f"  worker pid {worker}: no thread rows in the window"
        self.assertIn(message, self.lines("--worker", str(worker), "--to", self.walls[last - 1]))
        self.assertNotIn(message, self.lines("--worker", str(worker)))


class StretchesAcrossLateSamples(ReportCase):
    LATE_MARK = ("across late samples: the span counts the recorder's lateness, "
                 "so it is an upper bound on any wait")

    def ready_at(self, walls_ms, switches=None):
        """A thread ready in one sample at each of these times."""
        fx = Fixture()
        for at, ms in enumerate(walls_ms):
            fx.thread(fx.sample(ms), reads_as="ready", switches=switches[at] if switches else 1)
        return fx

    def stretch_line(self, fx, *args):
        return next(line for line in self.lines(fx, *args) if "thread 70" in line)

    def test_normal_wobble_under_twice_the_period_carries_no_mark(self):
        line = self.stretch_line(self.ready_at((0, 25, 45, 85)))
        self.assertNotIn("across late samples", line)

    def test_one_sample_prints_no_stretch_and_no_mark(self):
        fx = Fixture()
        fx.steady(2)
        fx.thread(1, reads_as="ready", switches=9)
        self.assertEqual(self.stretch_line(fx),
                         "  game pid 7 thread 70: running 0, ready 1, no ready stretch")


class LateStretchesInARealRecording(RealRecordingCase):
    FOLDER = LATE_RUN
    LATE_MARK = StretchesAcrossLateSamples.LATE_MARK

    def ready_in(self, tid, run):
        """Checks that this game thread is ready in exactly the samples of `run`
        within run's own bounds, and returns its rows there."""
        rows = [row for row in self.thread_rows(self.game, tid)
                if run[0] <= int(row["sample"]) <= run[-1]]
        self.assertEqual([int(row["sample"]) for row in rows
                          if row["reads_as"] == R.READS_READY], run)
        return rows

    def window_line(self, tid, run):
        lines = self.lines("--from", self.walls[run[0]], "--to", self.walls[run[-1]])
        return self.line_of(lines, self.game, tid)

    def late_gaps(self, run):
        return [self.gap_ms(a, b) > self.late_limit_ms() for a, b in zip(run, run[1:])]

    def expected_stretch(self, tid, run):
        """The stretch text for these consecutive ready samples, read off the three files."""
        switches = {int(row["sample"]): int(row["switches"]) for row in self.thread_rows(self.game, tid)}
        first = min(self.walls)
        gained = {}
        for row in self.rows(R.PROCESSES_FILE):
            # The first sample's amounts cover a slice from before the recording's start.
            if int(row["sample"]) in run and int(row["sample"]) != first:
                key = (int(row["pid"]), row["name"])
                gained[key] = gained.get(key, Fraction(0)) + Fraction(row["cpu_ms"])
        who = [f"pid {pid} {name or R.NO_NAME} {float(ms):.1f} ms"
               for (pid, name), ms in sorted(gained.items(), key=lambda kv: (-kv[1], kv[0]))
               if ms > 0]
        late = f", {self.LATE_MARK}" if any(self.late_gaps(run)) else ""
        return (f"longest ready stretch: ready in {len(run)} consecutive samples spanning "
                f"{float(self.gap_ms(run[0], run[-1])):.1f} ms "
                f"from wall {R._wall(Fraction(self.walls[run[0]]))}, "
                f"{sum(switches[n] for n in run[1:])} context switches gained inside{late}, "
                f"CPU gained in those samples: {', '.join(who) or 'by nobody'}")

    def test_a_stretch_across_late_samples(self):
        tid, run = 4248, [1723, 1724, 1725, 1726, 1727]
        rows = self.ready_in(tid, run)
        self.assertTrue(any(self.late_gaps(run)))
        self.assertEqual(self.window_line(tid, run),
                         f"  game pid {self.game} thread {tid}: " + self.counts_from_rows(rows)
                         + self.expected_stretch(tid, run))

    def test_a_window_counts_switches_from_its_own_first_sample(self):
        tid, run = 4248, [1723, 1724, 1725, 1726, 1727]
        self.ready_in(tid, run)
        switches = {int(row["sample"]): int(row["switches"]) for row in self.thread_rows(self.game, tid)}
        # The window starts one sample into the stretch. That sample's switches came
        # before the window's stretch, so the count drops by them.
        self.assertGreater(switches[run[1]], 0)
        line = self.window_line(tid, run[1:])
        self.assertIn(self.expected_stretch(tid, run[1:]), line)
        self.assertNotIn(f" {sum(switches[n] for n in run[1:])} context switches", line)

    def test_samples_just_over_twice_the_period_apart_carry_the_mark(self):
        tid, run = 4832, [1702, 1703]
        self.ready_in(tid, run)
        limit = self.late_limit_ms()
        # Just over: past the limit by less than a tenth of a period.
        self.assertTrue(limit < self.gap_ms(*run) < limit + limit / R.LONG_STEP_FACTOR / 10,
                        float(self.gap_ms(*run)))
        self.assertIn(self.LATE_MARK, self.window_line(tid, run))

    def test_one_late_gap_among_on_time_ones_carries_the_mark(self):
        tid, run = 19512, [1703, 1704, 1705]
        self.ready_in(tid, run)
        self.assertEqual(self.late_gaps(run).count(True), 1, self.late_gaps(run))
        self.assertIn(self.LATE_MARK, self.window_line(tid, run))


class ClockInARealRecording(RealRecordingCase):
    def clock_steps(self):
        """(later sample's row, step ms, wall gap ms) for each pair of neighbouring
        samples. Checks first that every sample has a clock reading the guard keeps,
        so every neighbouring pair is a step."""
        samples = self.rows(R.SAMPLES_FILE)
        period = Fraction(samples[0]["period_ms"])
        for row in samples:
            self.assertTrue(row["clock_s"], row)
            read_ms = (Fraction(row["clock_wall_after_s"]) - Fraction(row["clock_wall_before_s"])) * 1000
            self.assertLessEqual(read_ms, period, row)
        return [(later, (Fraction(later["clock_s"]) - Fraction(earlier["clock_s"])) * 1000,
                 (Fraction(later["wall_s"]) - Fraction(earlier["wall_s"])) * 1000)
                for earlier, later in zip(samples, samples[1:])]


class CalmClockInARealRecording(ClockInARealRecording):
    def test_a_calm_run_says_none_is_listed(self):
        steps = self.clock_steps()
        limit = self.late_limit_ms()
        self.assertTrue(steps)
        self.assertTrue(all(abs(step - gap) <= limit for _, step, gap in steps))
        lines = self.lines()
        self.assertIn(f"clock steps with an excess over {float(limit):g} ms by size: 0", lines)
        self.assertIn("no clock step has an excess that long, so none is listed", lines)
        self.assertFalse([line for line in lines if line.startswith("  wall ")])


class LateClockInARealRecording(ClockInARealRecording):
    FOLDER = LATE_RUN

    def test_clock_steps(self):
        steps = self.clock_steps()
        lines = self.lines()
        period = Fraction(self.rows(R.SAMPLES_FILE)[0]["period_ms"])
        self.assertIn(f"clock reads dropped, their two stamps over one period "
                      f"({float(period):g} ms) apart: 0", lines)
        sizes = [step for _, step, _ in steps]
        percentile = statistics.quantiles(sizes, n=100, method="inclusive")[R.STEP_PERCENTILE - 1]
        self.assertIn(f"clock steps: count {len(sizes)}, median {float(statistics.median(sizes)):.1f}"
                      f" ms, {R.STEP_PERCENTILE}th percentile {float(percentile):.1f} ms", lines)
        limit = self.late_limit_ms()
        long_steps = sorted([s for s in steps if abs(s[1] - s[2]) > limit],
                            key=lambda s: -abs(s[1] - s[2]))
        # More long steps than the list holds, so the cut is checked too.
        self.assertGreater(len(long_steps), R.LONGEST_LISTED)
        at = lines.index(f"clock steps with an excess over {float(limit):g} ms by size: "
                         f"{len(long_steps)}")
        self.assertEqual([line for line in lines[at:] if line.startswith("  wall ")],
                         [f"  wall {R._wall(Fraction(row['wall_s']))}: step {float(step):.1f} ms, "
                          f"wall gap {float(gap):.1f} ms, excess {float(step - gap):+.1f} ms"
                          for row, step, gap in long_steps[:R.LONGEST_LISTED]])

    def test_the_lead_line(self):
        self.clock_steps()
        offsets = [((Fraction(row["clock_s"]) - Fraction(row["wall_s"])) * 1000, row)
                   for row in self.rows(R.SAMPLES_FILE)]
        middle = statistics.median(offset for offset, _ in offsets)
        high, high_at = max(offsets, key=lambda item: item[0])
        low, low_at = min(offsets, key=lambda item: item[0])
        self.assertIn("clock lead over wall time, against its median offset: "
                      f"largest {float(high - middle):+.1f} ms at wall "
                      f"{R._wall(Fraction(high_at['wall_s']))}, "
                      f"lowest {float(low - middle):+.1f} ms at wall "
                      f"{R._wall(Fraction(low_at['wall_s']))}", self.lines())


class ClockSteps(ReportCase):
    SIX_PAIRS =((20, 20), (20, 20), (70, 20), (300, 100), (100, 100), (20, 80))

    def test_the_late_sample_is_not_listed(self):
        lines = self.lines(clock_fixture(self.SIX_PAIRS))
        self.assertFalse([line for line in lines if "wall 1000.260" in line])

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
        fx.sample(R.GAP_MS)
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


class ARecordingAsTheRecorderWritesIt(ReportCase):
    """Sample numbers start at 1, the Idle process has no name, and a process
    that gained nothing has a row in sample 1 only."""

    def fixture(self):
        fx = Fixture()
        for number, ms in ((1, 0), (2, 20), (3, 40), (4, 60)):
            fx.samples.append([number, wall_text(ms), 1, 400, 5000, 2, "", "", "", RECORDER,
                               PERIOD_MS, "", ""])
        fx.thread(1)
        fx.process(1, 0, "", cpu_ms=0, parent=0)
        fx.process(2, 0, "", cpu_ms=30, parent=0)
        fx.process(1, WORKER, "python.exe", parent=30)
        fx.process(1, 30, "py.exe", parent=31)
        fx.process(1, 31, "claude.exe", parent=32)
        fx.process(2, 31, "claude.exe", cpu_ms=2, parent=32)
        return fx

    def test_a_nameless_process_is_not_a_half_row(self):
        lines = self.lines(self.fixture())
        self.assertFalse([line for line in lines if "half-written" in line])
        self.assertIn("    50.0%  pid 0 (no name)", lines)

    def test_a_parent_with_a_row_only_in_sample_one_names_the_front(self):
        lines = self.lines(self.fixture(), "--worker", str(WORKER))
        self.assertIn("worker 8: its parent is pid 30 py.exe", lines)
        self.assertIn("front: pid 31 claude.exe, the parent of pid 30", lines)
        self.assertIn("     0.0%  pid 8 python.exe (worker)", lines)


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
