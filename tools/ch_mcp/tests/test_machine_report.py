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
import shutil
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
# Samples 876 to 882 of the same run, copied the same way. The recorder first read
# the game's clock at sample 878, so samples 876 and 877 have no clock.
CLOCK_START = os.path.join(REPO, "testdata", "ch_runs", "ls-t3-run-b", "machine-1-samples-876-882")
# Samples 2898 to 2904 of the same run, copied the same way. Sample 2900's clock
# read took longer than one period, and the game's clock stood still throughout.
DROPPED_READ = os.path.join(REPO, "testdata", "ch_runs", "ls-t3-run-b", "machine-1-samples-2898-2904")
# Samples 9742 to 9746 of hydra-data/ch-mcp-runs/2026-11-11/ls-a2-run1/machine-2,
# copied unchanged. Sample 9744's clock read took a little over one period.
JUST_OVER_READ = os.path.join(REPO, "testdata", "ch_runs", "ls-a2-run1", "machine-2-samples-9742-9746")
# Samples 1 to 84 of hydra-data/ch-mcp-runs/2026-11-07/ls-t3-run-b/machine-1, copied the
# same way, except that processes.csv lacks the rows of the user's own chat, music, browser,
# store and vendor-tool programs (the tests do not read them). Sample 1 holds a row
# for every process, so the game's parent (the worker), its parent and theirs are all
# named; some other processes' parents have no row.
FAMILY_RUN = os.path.join(REPO, "testdata", "ch_runs", "ls-t3-run-b", "machine-1-samples-1-84")
# The game's settings.ini as one real run backed it up, copied unchanged from
# hydra-data/ch-mcp-runs/2026-11-12/ls-g1-run1/backup-20261112-182956.
REAL_SETTINGS = os.path.join(REPO, "testdata", "ch_runs", "ls-g1-run1", "backup-20261112-182956",
                             R.game_files.SETTINGS)
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

    def report(self, *args, pid=None):
        """(exit code, output lines) of main on the real recording, for the game
        unless `pid` names another process."""
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = R.main([self.FOLDER, str(self.game if pid is None else pid), *args],
                          documents_dir=self.documents)
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
        return R._ms(Fraction(self.walls[later]) - Fraction(self.walls[earlier]))

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

    def gained(self, field, samples=None):
        """Each (pid, name) in processes.csv and what its rows in `samples` (all
        samples when None) hold of `field`."""
        totals = {}
        for row in self.rows(R.PROCESSES_FILE):
            if samples is not None and int(row["sample"]) not in samples:
                continue
            key = (int(row["pid"]), row["name"])
            totals[key] = totals.get(key, Fraction(0)) + Fraction(row[field])
        return totals

    def slices(self):
        """The samples that end a slice of the whole recording: all but the first."""
        return set(self.walls) - {min(self.walls)}

    def cpu_by_process(self, first_sample_too=True):
        """Each (pid, name) in processes.csv and the CPU its rows hold."""
        return self.gained("cpu_ms", None if first_sample_too else self.slices())

    def share_text(self, ms):
        """A CPU share of the whole recording, as the table prints it."""
        return f"{float(ms / self.gap_ms(min(self.walls), max(self.walls)) * 100):.1f}"

    def first_rows(self):
        """Each pid's first processes.csv row, the row the report's family table keeps."""
        rows = {}
        for row in self.rows(R.PROCESSES_FILE):
            rows.setdefault(int(row["pid"]), row)
        return rows


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
        message = f"  worker pid {worker}: {R.NO_THREAD_ROWS}"
        self.assertIn(message, self.lines("--worker", str(worker), "--to", self.walls[last - 1]))
        self.assertNotIn(message, self.lines("--worker", str(worker)))


class StretchesAcrossLateSamples(ReportCase):
    LATE_MARK = R.LATE_MARK

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
        self.assertNotIn(self.LATE_MARK, line)

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
            read_ms = R._ms(Fraction(row["clock_wall_after_s"]) - Fraction(row["clock_wall_before_s"]))
            self.assertLessEqual(read_ms, period, row)
        return [(later, R._ms(Fraction(later["clock_s"]) - Fraction(earlier["clock_s"])),
                 R._ms(Fraction(later["wall_s"]) - Fraction(earlier["wall_s"])))
                for earlier, later in zip(samples, samples[1:])]


class CalmClockInARealRecording(ClockInARealRecording):
    def test_a_calm_run_says_none_is_listed(self):
        steps = self.clock_steps()
        limit = self.late_limit_ms()
        self.assertTrue(steps)
        self.assertTrue(all(abs(step - gap) <= limit for _, step, gap in steps))
        lines = self.lines()
        self.assertIn(f"clock steps with an excess over {float(limit):g} ms by size: 0", lines)
        self.assertIn(R.NO_LONG_STEP, lines)
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
        offsets = [(R._ms(Fraction(row["clock_s"]) - Fraction(row["wall_s"])), row)
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


class ClockRowsCase(RealRecordingCase):
    """Works out from FOLDER's samples.csv what the clock section should print."""

    def samples(self):
        return {int(row["sample"]): row for row in self.rows(R.SAMPLES_FILE)}

    def period_ms(self):
        return Fraction(self.rows(R.SAMPLES_FILE)[0]["period_ms"])

    @staticmethod
    def read_ms(row):
        """How far apart the clock read's two wall stamps are."""
        return R._ms(Fraction(row["clock_wall_after_s"]) - Fraction(row["clock_wall_before_s"]))

    def over_one_period(self):
        """The samples whose clock read took longer than one period."""
        return sorted(n for n, row in self.samples().items()
                      if row["clock_s"] and self.read_ms(row) > self.period_ms())

    def step(self, earlier, later):
        """(clock step ms, wall gap ms) from sample `earlier` to sample `later`."""
        rows = self.samples()
        return (R._ms(Fraction(rows[later]["clock_s"]) - Fraction(rows[earlier]["clock_s"])),
                R._ms(Fraction(rows[later]["wall_s"]) - Fraction(rows[earlier]["wall_s"])))

    def steps_line(self, pairs):
        steps = [self.step(a, b)[0] for a, b in pairs]
        percentile = statistics.quantiles(steps, n=100, method="inclusive")[R.STEP_PERCENTILE - 1]
        return (f"clock steps: count {len(steps)}, median {float(statistics.median(steps)):.1f}"
                f" ms, {R.STEP_PERCENTILE}th percentile {float(percentile):.1f} ms")

    def listed_line(self, earlier, later):
        step, gap = self.step(earlier, later)
        return (f"  wall {R._wall(Fraction(self.walls[later]))}: step {float(step):.1f} ms, "
                f"wall gap {float(gap):.1f} ms, excess {float(step - gap):+.1f} ms")

    def long_count_line(self, pairs):
        limit = self.late_limit_ms()
        count = sum(abs(step - gap) > limit for step, gap in (self.step(*p) for p in pairs))
        return f"clock steps with an excess over {float(limit):g} ms by size: {count}"

    def dropped_line(self, count):
        return (f"clock reads dropped, their two stamps over one period "
                f"({float(self.period_ms()):g} ms) apart: {count}")

    def lead_line(self, numbers):
        rows = self.samples()
        offsets = [(R._ms(Fraction(rows[n]["clock_s"]) - Fraction(rows[n]["wall_s"])), n)
                   for n in numbers]
        middle = statistics.median(offset for offset, _ in offsets)
        high, high_at = max(offsets, key=lambda item: item[0])
        low, low_at = min(offsets, key=lambda item: item[0])
        return ("clock lead over wall time, against its median offset: "
                f"largest {float(high - middle):+.1f} ms at wall "
                f"{R._wall(Fraction(self.walls[high_at]))}, "
                f"lowest {float(low - middle):+.1f} ms at wall "
                f"{R._wall(Fraction(self.walls[low_at]))}")


class StepsInAWindowInARealRecording(ClockRowsCase):
    FOLDER = LATE_RUN

    def test_steps_in_a_window_are_the_ones_ending_in_it(self):
        self.assertFalse(self.over_one_period())
        numbers = sorted(self.walls)
        self.assertTrue(all(self.samples()[n]["clock_s"] for n in numbers))
        first, last = numbers[30], numbers[40]
        pairs = [(a, b) for a, b in zip(numbers, numbers[1:]) if first <= b <= last]
        # The step into the window's first sample ends inside it, so it counts.
        self.assertEqual(pairs[0], (numbers[29], first))
        lines = self.lines("--from", self.walls[first], "--to", self.walls[last])
        self.assertIn(self.steps_line(pairs), lines)
        self.assertNotIn(self.steps_line(pairs[1:]), lines)


class ClockStartInARealRecording(ClockRowsCase):
    FOLDER = CLOCK_START

    def test_a_sample_with_no_clock_pairs_with_nothing(self):
        rows = self.samples()
        without = [n for n in sorted(rows) if not rows[n]["clock_s"]]
        clocked = [n for n in sorted(rows) if rows[n]["clock_s"]]
        self.assertTrue(without)
        self.assertLess(max(without), min(clocked))
        self.assertFalse(self.over_one_period())
        self.assertIn(self.steps_line(list(zip(clocked, clocked[1:]))), self.lines())


class ClockWithoutStampsInARealRecording(RealRecordingCase):
    """The real excerpt with one clocked sample's second wall stamp blanked."""
    FOLDER = CLOCK_START

    def test_a_clock_with_no_stamps_is_refused(self):
        rows = self.rows(R.SAMPLES_FILE)
        clocked = next(row for row in rows if row["clock_s"])
        clocked["clock_wall_after_s"] = ""
        folder = os.path.join(self.documents, "blanked")
        shutil.copytree(self.FOLDER, folder)
        with open(os.path.join(folder, R.SAMPLES_FILE), "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = R.main([folder, str(self.game)], documents_dir=self.documents)
        self.assertEqual(code, 1)
        self.assertIn(f"sample {clocked['sample']} has a clock reading with no wall stamps",
                      out.getvalue())


class NoClockInARealRecording(RealRecordingCase):
    FOLDER = RUN_START

    def test_no_clock(self):
        self.assertFalse([row for row in self.rows(R.SAMPLES_FILE) if row["clock_s"]])
        lines = self.lines()
        self.assertIn("no clock in this recording", lines)
        self.assertFalse([line for line in lines if line.startswith("clock ")])


class DroppedReadInARealRecording(ClockRowsCase):
    FOLDER = DROPPED_READ

    def dropped_and_kept(self):
        """The one sample the guard drops, and the samples it keeps, in order."""
        numbers = sorted(self.walls)
        self.assertTrue(all(self.samples()[n]["clock_s"] for n in numbers))
        dropped = self.over_one_period()
        self.assertEqual(len(dropped), 1, dropped)
        return dropped[0], [n for n in numbers if n not in dropped]

    def test_it_is_dropped_and_not_listed(self):
        gone, kept = self.dropped_and_kept()
        lines = self.lines()
        self.assertIn(self.dropped_line(1), lines)
        self.assertFalse([line for line in lines
                          if line.startswith(f"  wall {R._wall(Fraction(self.walls[gone]))}:")])
        # The dropped sample's reading would move the lead line, so leaving it out shows.
        self.assertNotEqual(self.lead_line(kept), self.lead_line(sorted(self.walls)))
        self.assertIn(self.lead_line(kept), lines)

    def test_the_neighbours_of_a_dropped_sample_pair_up(self):
        gone, kept = self.dropped_and_kept()
        self.assertIn(gone - 1, kept)
        self.assertIn(gone + 1, kept)
        lines = self.lines()
        self.assertIn(self.steps_line(list(zip(kept, kept[1:]))), lines)
        # The step across the dropped sample is long, so it is listed.
        step, gap = self.step(gone - 1, gone + 1)
        self.assertGreater(abs(step - gap), self.late_limit_ms())
        self.assertIn(self.listed_line(gone - 1, gone + 1), lines)

    def test_an_excess_just_over_the_limit_is_long(self):
        _, kept = self.dropped_and_kept()
        pairs = list(zip(kept, kept[1:]))
        limit = self.late_limit_ms()
        # Just over: past the limit by less than a tenth of a period.
        just_over = [p for p in pairs
                     if limit < abs(self.step(*p)[0] - self.step(*p)[1]) < limit + self.period_ms() / 10]
        self.assertTrue(just_over, [self.step(*p) for p in pairs])
        lines = self.lines()
        self.assertIn(self.long_count_line(pairs), lines)
        for pair in just_over:
            self.assertIn(self.listed_line(*pair), lines)

    def test_only_drops_in_the_window_are_counted(self):
        gone, _ = self.dropped_and_kept()
        self.assertIn(self.dropped_line(0), self.lines("--from", self.walls[gone + 1]))


class JustOverOnePeriodInARealRecording(ClockRowsCase):
    FOLDER = JUST_OVER_READ

    def test_stamps_just_over_one_period_apart_are_dropped(self):
        period = self.period_ms()
        dropped = self.over_one_period()
        self.assertEqual(len(dropped), 1, dropped)
        # Just over: past the period by less than a quarter of it.
        self.assertLess(self.read_ms(self.samples()[dropped[0]]), period + period / 4)
        self.assertIn(self.dropped_line(1), self.lines())


class SampleGapsInARealRecording(RealRecordingCase):
    FOLDER = DROPPED_READ

    def test_gaps_over_gap_ms_are_the_recorders_stalls(self):
        numbers = sorted(self.walls)
        gaps = [self.gap_ms(a, b) for a, b in zip(numbers, numbers[1:])]
        # Read off samples.csv: the gaps into samples 2901 and 2902 are 80.4 and
        # 80.6 ms, so two are over 50 ms; the gap into 2904 is 49.4 ms, just under.
        self.assertIn(f"gaps between samples: median {float(statistics.median(gaps)):.1f} ms, "
                      f"worst {float(max(gaps)):.1f} ms, 2 over {R.GAP_MS} ms "
                      "(the recorder's own stalls)", self.lines())


class CostInARealRecording(RealRecordingCase):
    def test_share_and_bytes_against_the_files(self):
        recorder = self.recorder_pid()
        slices = self.slices()
        cpu = sum(ms for (pid, _), ms in self.gained("cpu_ms", slices).items() if pid == recorder)
        wrote = sum(n for (pid, _), n in self.gained("write_bytes", slices).items()
                    if pid == recorder)
        # The recorder ran and wrote in this excerpt, so neither number is zero.
        self.assertGreater(cpu, 0)
        self.assertGreater(wrote, 0)
        size = sum(os.path.getsize(os.path.join(self.FOLDER, name))
                   for name in (R.SAMPLES_FILE, R.PROCESSES_FILE, R.THREADS_FILE))
        lines = self.lines()
        self.assertIn(f"recorder cost: pid {recorder}, {self.share_text(cpu)}% of one core "
                      "over the recording", lines)
        self.assertIn(f"recorder cost: wrote {int(wrote)} bytes, its three files hold {size} bytes",
                      lines)


class FirstLineInARealRecording(RealRecordingCase):
    def test_counts_the_pids_thread_samples(self):
        mine = self.thread_rows(self.game)
        # Other processes have thread rows too, so counting every row would show.
        self.assertTrue([row for row in self.rows(R.THREADS_FILE) if int(row["pid"]) != self.game])
        self.assertEqual(self.lines()[0],
                         f"the recording holds {len(mine)} thread samples for pid {self.game}")

    def test_none_is_refused_naming_the_pid(self):
        threaded = {int(row["pid"]) for row in self.rows(R.THREADS_FILE)}
        # A process the recorder saw whose threads it did not record.
        others = sorted(set(self.first_rows()) - threaded)
        self.assertTrue(others)
        pid = others[0]
        code, lines = self.report(pid=pid)
        self.assertEqual(code, 1)
        self.assertEqual(lines, [f"the recording holds 0 thread samples for pid {pid}",
                                 f"refused: no thread rows for pid {pid}, "
                                 "so this recording is not of that process"])


class ByProcessInARealRecording(RealRecordingCase):
    # Build processes took hard faults in these samples.
    FIRST, LAST = 1667, 1669

    def window_lines(self):
        return self.lines("--from", self.walls[self.FIRST], "--to", self.walls[self.LAST])

    @staticmethod
    def section(lines, heading):
        """The indented lines under the one line that starts with `heading`."""
        at = next(i for i, line in enumerate(lines) if line.startswith(heading))
        rows = []
        for line in lines[at + 1:]:
            if not line.startswith("  "):
                break
            rows.append(line)
        return lines[at], rows

    @staticmethod
    def largest_first(gains):
        return sorted((key for key, n in gains.items() if n > 0), key=lambda key: (-gains[key], key))

    def test_bytes_in_the_window(self):
        window = set(range(self.FIRST, self.LAST + 1))
        read = self.gained("read_bytes", window)
        wrote = self.gained("write_bytes", window)
        both = {key: read.get(key, 0) + wrote.get(key, 0) for key in set(read) | set(wrote)}
        expected = [f"  {self.who(*key)}: read {int(read.get(key, 0))}, wrote {int(wrote.get(key, 0))}"
                    for key in self.largest_first(both)]
        self.assertTrue(expected)
        _, rows = self.section(self.window_lines(), "bytes by process in the window")
        self.assertEqual(rows, expected)
        # Processes that moved bytes only outside the window are left out.
        whole = self.gained("read_bytes")
        whole_wrote = self.gained("write_bytes")
        outside = [key for key in set(whole) | set(whole_wrote)
                   if whole.get(key, 0) + whole_wrote.get(key, 0) > 0 and not both.get(key)]
        self.assertTrue(outside)
        for key in outside:
            self.assertFalse([row for row in rows if row.startswith(f"  {self.who(*key)}:")], key)

    def test_hard_faults_carry_the_mark(self):
        faults = self.gained("hard_faults", set(range(self.FIRST, self.LAST + 1)))
        expected = [f"  {self.who(*key)}: {int(faults[key])}" for key in self.largest_first(faults)]
        self.assertTrue(expected)
        heading, rows = self.section(self.window_lines(), "hard faults by process in the window")
        self.assertIn("(meaning unchecked)", heading)
        self.assertEqual(rows, expected)


class SettingsHeaderInARealRecording(RealRecordingCase):
    def settings_lines(self):
        with open(REAL_SETTINGS, encoding="utf-8") as f:
            return [line.strip() for line in f]

    def key_lines(self, lines, key):
        return [line for line in lines if line.split("=", 1)[0].strip() == key]

    def test_prints_the_two_lines(self):
        shutil.copy(REAL_SETTINGS, self.documents)
        real = self.settings_lines()
        wanted = [line for key in R.SETTINGS_KEYS for line in self.key_lines(real, key)]
        self.assertEqual(len(wanted), len(R.SETTINGS_KEYS), real)
        self.assertEqual(self.lines()[1:3],
                         [f"{R.game_files.SETTINGS}: {line}" for line in wanted])

    def test_a_line_that_is_not_there_is_said(self):
        # The real file with its framerate line taken out.
        gone, kept = R.SETTINGS_KEYS
        real = self.settings_lines()
        with open(os.path.join(self.documents, R.game_files.SETTINGS), "w", encoding="utf-8") as f:
            f.write("\n".join(line for line in real if not self.key_lines([line], gone)) + "\n")
        self.assertEqual(self.lines()[1:3],
                         [f"{R.game_files.SETTINGS}: no {gone} line",
                          f"{R.game_files.SETTINGS}: {self.key_lines(real, kept)[0]}"])


class TheFrontInARealRecording(RealRecordingCase):
    FOLDER = FAMILY_RUN

    def family(self):
        """The worker (the game's parent), its parent and the front, from processes.csv."""
        rows = self.first_rows()
        worker = int(rows[self.game]["parent_pid"])
        launcher = int(rows[worker]["parent_pid"])
        front = int(rows[launcher]["parent_pid"])
        self.assertIn(front, rows)
        return rows, worker, launcher, front

    def test_the_grandparent_is_the_front(self):
        rows, worker, launcher, front = self.family()
        lines = self.lines("--worker", str(worker))
        self.assertIn(f"worker {worker}: its parent is pid {launcher} {rows[launcher]['name']}", lines)
        self.assertIn(f"front: pid {front} {rows[front]['name']}, the parent of pid {launcher}", lines)
        table = dict(self.table(lines))
        cpu = self.cpu_by_process(first_sample_too=False)
        for pid, role in ((worker, "worker"), (front, "front")):
            key = (pid, rows[pid]["name"])
            self.assertEqual(table.get(f"{self.who(*key)} ({role})"),
                             [float(self.share_text(cpu.get(key, 0)))], key)

    def test_a_front_with_no_cpu_rows_is_still_listed(self):
        rows = self.first_rows()
        # Processes whose parent has a row but whose parent's parent has none.
        cases = [(pid, int(row["parent_pid"])) for pid, row in rows.items()
                 if int(row["parent_pid"]) in rows
                 and int(rows[int(row["parent_pid"])]["parent_pid"]) not in rows]
        self.assertTrue(cases)
        worker, launcher = cases[0]
        front = int(rows[launcher]["parent_pid"])
        lines = self.lines("--worker", str(worker))
        self.assertIn(f"front: pid {front} {R.NO_ROWS_NAME}, the parent of pid {launcher}", lines)
        self.assertEqual(dict(self.table(lines)).get(f"pid {front} {R.NO_ROWS_NAME} (front)"), [0.0])

    def test_a_launcher_with_no_row_says_the_front_is_not_known(self):
        rows = self.first_rows()
        cases = [(pid, int(row["parent_pid"])) for pid, row in rows.items()
                 if int(row["parent_pid"]) not in rows]
        self.assertTrue(cases)
        worker, launcher = cases[0]
        self.assertIn(f"worker {worker}: its parent is pid {launcher}, which has no row in "
                      f"{R.PROCESSES_FILE}, so the front is not known",
                      self.lines("--worker", str(worker)))

    def test_a_worker_with_no_row_says_so(self):
        rows = self.first_rows()
        # A pid the recording names only as some process's parent.
        unrowed = sorted({int(row["parent_pid"]) for row in rows.values()} - set(rows))
        self.assertTrue(unrowed)
        self.assertIn(f"worker {unrowed[0]}: no row in {R.PROCESSES_FILE}, so the front is not known",
                      self.lines("--worker", str(unrowed[0])))

    def test_no_worker_prints_no_front(self):
        _, worker, _, _ = self.family()
        # With the worker given, this recording does name a front.
        self.assertTrue([line for line in self.lines("--worker", str(worker)) if "front" in line])
        self.assertFalse([line for line in self.lines() if "front" in line])


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
