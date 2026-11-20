"""quiet.py: the verdict and its lines, on a real recorded process list and
reading where one fits and on fake readers elsewhere, the folder scan on real
temp folders, and the live readers on the real commands they run. Those read
only what does not change with what the user is doing: this test's own process
row, the System row, the active power scheme's name and a child process's
console. Nothing here reads this machine's counters or worktrees."""

from __future__ import annotations

import ctypes
import json
import os
import re
import subprocess
import time
import sys
import tempfile
import unittest
import unittest.mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import quiet as Q  # noqa: E402

GIB = 1024 ** 3


class FakeReaders:
    """A QuietEnv's readers, with what each answers and a record of the calls."""

    def __init__(self, processes=(), cpu=3.0, folders=(), memory=12 * GIB, scheme="Balanced",
                 scan=None):
        self.processes = list(processes)
        self.cpu = cpu
        self.scan = scan or Q.FolderScan(folders=tuple(folders), entries=40, took_s=0.25)
        self.memory = memory
        self.scheme = scheme
        self.sampled = []

    def sample_cpu(self, seconds):
        self.sampled.append(seconds)
        return self.cpu

    def env(self, **overrides):
        readers = dict(list_processes=lambda: self.processes, sample_cpu=self.sample_cpu,
                       scan_folders=lambda: self.scan, free_memory_bytes=lambda: self.memory,
                       power_scheme=lambda: self.scheme)
        readers.update(overrides)
        return Q.QuietEnv(**readers)


def report(**fake_args):
    return Q.quiet_report(FakeReaders(**fake_args).env())


def raises(error):
    def reader(*_args):
        raise error
    return reader


# Copied from hydra-data's ch-mcp-runs/2026-11-07/ls-t3-run-b. compilers.txt is
# the compiler rows of a process-list read 30 s into that run, while another
# session's build ran: two cmake rows, the build's own MSBuild, 15 MSBuild
# helpers and 24 cl.exe rows, of which the first two are kept. The account
# folder in the cl.exe rows is renamed. ch_start-reading.txt is the run's quiet
# ch_start reading, line 8 of its replies.txt.
RUN_LS_T3 = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-t3-run-b")
# The recording's main MSBuild: its command line builds ALL_BUILD.vcxproj.
RECORDED_MAIN_MSBUILD_PID = 27444


def recorded_processes():
    """The recorded rows, each 'pid name command line', read by the real
    list_processes from the JSON shape it parses."""
    with open(os.path.join(RUN_LS_T3, "compilers.txt"), encoding="utf-8") as f:
        rows = f.read().splitlines()[1:]
    listed = []
    for row in rows:
        pid, name, command_line = row.split(" ", 2)
        listed.append({"Name": name, "ProcessId": int(pid), "CommandLine": command_line})
    return Q.list_processes(run=lambda _cmd: json.dumps(listed))


def recorded_helpers():
    """The recording's MSBuild rows other than the build's own MSBuild."""
    return [p for p in recorded_processes()
            if p.name == "MSBuild.exe" and p.pid != RECORDED_MAIN_MSBUILD_PID]


def recorded_reading():
    """The CPU share, free memory and power scheme of the recorded ch_start."""
    with open(os.path.join(RUN_LS_T3, "ch_start-reading.txt"), encoding="utf-8") as f:
        said = f.read()
    cpu = re.search(r"CPU ([\d.]+) percent", said).group(1)
    memory = re.search(r"free memory ([\d.]+) GB", said).group(1)
    scheme = said.split("; ")[4]
    return cpu, memory, scheme


class RecordedTest(unittest.TestCase):
    """The verdict on the recorded process rows and reading, with the real
    folder scan on a temp home folder."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.home = self._tmp.name
        self.cpu, self.memory, self.scheme = recorded_reading()

    def tearDown(self):
        self._tmp.cleanup()

    def report(self, processes):
        return Q.quiet_report(Q.QuietEnv(
            list_processes=lambda: processes, sample_cpu=lambda _s: float(self.cpu),
            scan_folders=lambda: Q.scan_folders(self.home),
            free_memory_bytes=lambda: int(float(self.memory) * GIB),
            power_scheme=lambda: self.scheme))

    def build_file(self, age_s):
        """A file in the main checkout's build folder, last written age_s ago."""
        folder = os.path.join(self.home, "Downloads", "Hydra", "hydra-test", "build-cpp")
        os.makedirs(folder)
        path = os.path.join(folder, "a.obj")
        with open(path, "wb"):
            pass
        written = time.time() - age_s
        os.utime(path, (written, written))
        return folder

    def lines_naming(self, got, text):
        return [line for line in got.lines if text in line]

    def test_a_quiet_machine_is_quiet_and_names_its_readings(self):
        helpers = recorded_helpers()
        got = self.report(helpers)
        self.assertTrue(got.quiet)
        self.assertIn("no compiler running", got.lines)
        for process in helpers:
            self.assertEqual(self.lines_naming(got, f"pid {process.pid}"), [])
        cpu_line = self.lines_naming(got, self.cpu)
        self.assertEqual(len(cpu_line), 1)
        self.assertIn(f"{Q.QUIET_SAMPLE_S} s", cpu_line[0])
        self.assertIn(f"{Q.QUIET_CPU_PCT} percent", cpu_line[0])
        self.assertEqual(len(self.lines_naming(got, f"{self.memory} GB")), 1)
        self.assertEqual(len(self.lines_naming(got, self.scheme)), 1)
        for name in Q.REPORTED_NAMES:
            self.assertIn(f"{name}: not running", got.lines)

    def test_a_compiler_stops_it_and_its_line_names_the_pid_and_its_folder(self):
        cl = next(p for p in recorded_processes() if p.name == "cl.exe")
        folder = Q.folder_named(cl.command_line)
        self.assertIsNotNone(folder)
        self.assertNotIn("CL.exe", folder)
        got = self.report([cl])
        self.assertFalse(got.quiet)
        named = self.lines_naming(got, f"pid {cl.pid}")
        self.assertEqual(len(named), 1)
        self.assertIn(cl.name, named[0])
        self.assertIn(folder, named[0])
        self.assertNotIn("no compiler running", got.lines)

    def test_msbuild_helpers_alone_are_quiet_and_one_line_gives_their_count(self):
        helpers = recorded_helpers()
        got = self.report(helpers)
        self.assertTrue(got.quiet)
        counted = self.lines_naming(got, "helpers")
        self.assertEqual(len(counted), 1)
        self.assertTrue(counted[0].endswith(f": {len(helpers)}"))

    def test_the_builds_own_msbuild_stops_it_and_only_it_is_a_compiler(self):
        msbuilds = [p for p in recorded_processes() if p.name == "MSBuild.exe"]
        got = self.report(msbuilds)
        self.assertFalse(got.quiet)
        for process in msbuilds:
            want = 1 if process.pid == RECORDED_MAIN_MSBUILD_PID else 0
            self.assertEqual(len(self.lines_naming(got, f"pid {process.pid}")), want)
        counted = self.lines_naming(got, "helpers")
        self.assertEqual(len(counted), 1)
        self.assertTrue(counted[0].endswith(f": {len(msbuilds) - 1}"))

    def test_every_recorded_compiler_but_the_helpers_stops_it_on_its_own(self):
        helpers = {p.pid for p in recorded_helpers()}
        others = [p for p in recorded_processes() if p.pid not in helpers]
        self.assertEqual({p.name for p in others}, {"cmake.exe", "MSBuild.exe", "cl.exe"})
        for process in others:
            with self.subTest(process=process.pid):
                self.assertFalse(self.report([process]).quiet)

    def test_a_build_folder_written_to_just_now_stops_it_and_its_line_names_it(self):
        folder = self.build_file(0)
        got = self.report(recorded_helpers())
        self.assertFalse(got.quiet)
        named = self.lines_naming(got, folder)
        self.assertEqual(len(named), 1)
        self.assertIn(f"{Q.BUILD_RECENT_S} s", named[0])

    def test_the_refusal_holds_every_line_in_order_and_says_to_call_ch_start_again(self):
        cl = next(p for p in recorded_processes() if p.name == "cl.exe")
        got = self.report([cl])
        self.assertFalse(got.quiet)
        said = "; ".join(got.lines)
        before, found, after = Q.refusal_text(got).partition(said)
        self.assertEqual(found, said)
        self.assertIn("not quiet", before)
        self.assertIn("ch_start", after)

    def test_a_build_folder_last_written_the_limit_ago_is_quiet(self):
        folder = self.build_file(Q.BUILD_RECENT_S)
        got = self.report(recorded_helpers())
        self.assertTrue(got.quiet)
        self.assertEqual(self.lines_naming(got, folder), [])


class VerdictTest(unittest.TestCase):
    def test_every_listed_compiler_stops_it_whatever_its_case(self):
        for name in Q.COMPILER_NAMES:
            for exe in (f"{name}.exe", f"{name.upper()}.EXE"):
                with self.subTest(exe=exe):
                    self.assertFalse(report(processes=[Q.Proc(exe, 1, None)]).quiet)

    def test_the_servers_own_runtimes_and_near_names_are_not_compilers(self):
        for exe in ("node.exe", "python.exe", "py.exe", "cloud.exe", "linker.exe", "Code.exe"):
            with self.subTest(exe=exe):
                self.assertTrue(report(processes=[Q.Proc(exe, 1, None)]).quiet)

    def test_no_helpers_means_no_helper_line(self):
        self.assertFalse(any("helper" in line for line in report().lines))

    def test_cpu_exactly_at_the_threshold_is_quiet_and_one_above_is_not(self):
        self.assertTrue(report(cpu=Q.QUIET_CPU_PCT).quiet)
        self.assertFalse(report(cpu=Q.QUIET_CPU_PCT + 1).quiet)

    def test_a_folder_with_no_files_is_quiet(self):
        self.assertTrue(report(folders=[Q.FolderAge("f", None)]).quiet)

    def test_the_sampler_waits_quiet_sample_s_and_the_line_says_so(self):
        fake = FakeReaders()
        got = Q.quiet_report(fake.env())
        self.assertEqual(fake.sampled, [Q.QUIET_SAMPLE_S])
        self.assertIn(f"averaged over {Q.QUIET_SAMPLE_S} s", got.lines[1])

    def test_spotify_and_defender_are_reported_and_never_stop_it(self):
        got = report(processes=[Q.Proc("Spotify.exe", 7, None), Q.Proc("MsMpEng.exe", 8, None)])
        self.assertTrue(got.quiet)
        self.assertIn("Spotify.exe: running", got.lines)
        self.assertIn("MsMpEng.exe: running", got.lines)


class FailingReaderTest(unittest.TestCase):
    """A check that could not look never says quiet."""

    def test_no_process_list_is_not_quiet_and_says_so(self):
        got = Q.quiet_report(FakeReaders().env(list_processes=raises(OSError("no list"))))
        self.assertFalse(got.quiet)
        self.assertEqual(got.lines[0], "process list: could not read it (no list), so a compiler "
                                       "may be running")
        self.assertIn("Spotify.exe: unknown", got.lines)

    def test_no_cpu_reading_is_not_quiet_and_says_so(self):
        got = Q.quiet_report(FakeReaders().env(sample_cpu=raises(OSError("no counter"))))
        self.assertFalse(got.quiet)
        self.assertEqual(got.lines[1], "CPU: could not read it (no counter)")

    def test_a_scan_that_failed_whole_is_not_quiet_and_says_so(self):
        got = Q.quiet_report(FakeReaders().env(scan_folders=raises(OSError("no disk"))))
        self.assertFalse(got.quiet)
        self.assertEqual(got.lines[2], "build folders: could not read them (no disk)")

    def test_a_scan_with_an_unreadable_folder_is_not_quiet_and_names_it(self):
        scan = Q.FolderScan(folders=(), entries=1, took_s=0.1, errors=("f: denied",))
        got = report(scan=scan)
        self.assertFalse(got.quiet)
        self.assertIn("build folders: could not read f: denied", got.lines)

    def test_a_scan_that_hit_its_cap_is_not_quiet_and_says_so_plainly(self):
        scan = Q.FolderScan(folders=(Q.FolderAge("f", 500.0),), entries=9, took_s=3.1, unread=4)
        got = report(scan=scan)
        self.assertFalse(got.quiet)
        self.assertIn("build folders: the scan hit its cap of 3 s and left 4 of 5 folders unread, "
                      "so a build may be writing there", got.lines)
        self.assertNotIn("no build folder written to in the last 60 s", got.lines)

    def test_no_memory_or_power_reading_is_a_line_and_never_stops_it(self):
        got = Q.quiet_report(FakeReaders().env(free_memory_bytes=raises(OSError("m")),
                                               power_scheme=raises(OSError("p"))))
        self.assertTrue(got.quiet)
        self.assertIn("free memory: could not read it (m)", got.lines)
        self.assertIn("power scheme: could not read it (p)", got.lines)


class FolderNamedTest(unittest.TestCase):
    """folder_named on the recorded command lines of ls-t3-run-b. None of them
    names a worktree: the build's cl.exe rows carry only a response file."""

    def test_the_programs_own_path_is_not_the_folder(self):
        # Every recorded row but cl.exe's has no path besides its program's.
        others = [p for p in recorded_processes() if p.name != "cl.exe"]
        self.assertTrue(others)
        for process in others:
            with self.subTest(process=process.pid):
                self.assertRegex(process.command_line, r'^"[A-Za-z]:[\\/]')
                self.assertIsNone(Q.folder_named(process.command_line))

    def test_with_no_worktree_the_first_path_is_named(self):
        cls = [p for p in recorded_processes() if p.name == "cl.exe"]
        self.assertTrue(cls)
        for process in cls:
            with self.subTest(process=process.pid):
                response_file = process.command_line.rsplit("@", 1)[1]
                self.assertEqual(Q.folder_named(process.command_line), response_file)

    def test_no_command_line_names_no_folder_and_the_line_says_so(self):
        self.assertIsNone(Q.folder_named(None))
        got = report(processes=[Q.Proc("cl.exe", 5, None)])
        self.assertIn("compiler running: cl.exe, pid 5, no folder in its command line", got.lines)


class ClockFake:
    """A monotonic clock that moves `step` seconds at each read."""

    def __init__(self, step=0.0):
        self.now = 0.0
        self.step = step

    def __call__(self):
        self.now += self.step
        return self.now


NOW = 2_000_000_000.0


class ScanTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.home = self._tmp.name
        self.checkout = os.path.join(self.home, "Downloads", "Hydra", "hydra-test")
        self.root = os.path.join(self.checkout, ".claude", "worktrees")

    def tearDown(self):
        self._tmp.cleanup()

    def file(self, relative, age_s, under=None):
        path = os.path.join(under or self.root, *relative.split("/"))
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "wb"):
            pass
        os.utime(path, (NOW - age_s, NOW - age_s))
        return path

    def scan(self, clock=None):
        return Q.scan_folders(self.home, wall=lambda: NOW, clock=clock or ClockFake())

    def folder(self, name):
        return os.path.join(self.root, name, "build-cpp")

    def test_the_age_is_the_newest_files_at_any_depth(self):
        self.file("a/build-cpp/old.txt", 9000)
        self.file("a/build-cpp/x.dir/Release/deep.obj", 500)
        got = self.scan()
        self.assertEqual(got.folders, (Q.FolderAge(self.folder("a"), 500.0),))
        self.assertEqual(got.unread, 0)
        self.assertEqual(got.errors, ())

    def test_only_the_named_sub_folder_of_each_worktree_is_read(self):
        self.file("a/src/new.cpp", 1)
        self.file("a/build-cpp/old.obj", 9000)
        self.assertEqual(self.scan().folders, (Q.FolderAge(self.folder("a"), 9000.0),))

    def test_a_worktree_with_no_such_sub_folder_is_skipped(self):
        self.file("a/src/new.cpp", 1)
        self.assertEqual(self.scan().folders, ())

    def test_the_home_folders_worktrees_are_not_read(self):
        dropped = os.path.join(self.home, ".claude", "worktrees")
        self.file("hook-work/gates/new.py", 1, under=dropped)
        self.file("hook-work/build-cpp/new.obj", 1, under=dropped)
        got = self.scan()
        self.assertEqual(got.folders, ())
        # No directory entry was read, so nothing under the home folder's
        # worktrees was opened. Why they stay out: quiet.py, above _CHECKOUT.
        self.assertEqual(got.entries, 0)

    def test_the_main_checkouts_build_folder_is_read_first(self):
        self.file("build-cpp/old.obj", 9000, under=self.checkout)
        self.file("a/build-cpp/old.obj", 500)
        self.assertEqual(self.scan().folders,
                         (Q.FolderAge(os.path.join(self.checkout, "build-cpp"), 9000.0),
                          Q.FolderAge(self.folder("a"), 500.0)))

    def test_a_recent_file_in_the_main_checkouts_build_folder_refuses(self):
        self.file("build-cpp/x.dir/new.obj", 2, under=self.checkout)
        got = Q.quiet_report(FakeReaders().env(scan_folders=self.scan))
        self.assertFalse(got.quiet)
        self.assertIn("build folder written to in the last 60 s: "
                      f"{os.path.join(self.checkout, 'build-cpp')} (a file 2 s old)", got.lines)

    def test_no_worktrees_folder_still_reads_the_main_checkout(self):
        self.file("build-cpp/old.obj", 9000, under=self.checkout)
        self.assertEqual(len(self.scan().folders), 1)

    def test_a_missing_checkout_reads_nothing(self):
        self.assertEqual(self.scan().folders, ())

    def test_a_folder_stops_at_its_first_recent_file_and_is_still_recent(self):
        recent = ["new.obj"]
        sub_folders = [f"d{i}" for i in range(20)]
        for name in recent:
            self.file(f"a/build-cpp/{name}", 2)
        for name in sub_folders:
            self.file(f"a/build-cpp/{name}/old.obj", 9000)
        got = self.scan()
        self.assertTrue(Q.is_recent(got.folders[0].age_s))
        # The top directory's entries only; none of its sub-folders is opened.
        self.assertEqual(got.entries, len(recent) + len(sub_folders))

    def test_a_scan_past_its_cap_stops_and_counts_the_unread_folders(self):
        for name in "abc":
            self.file(f"{name}/build-cpp/old.obj", 9000)
        got = self.scan(clock=ClockFake(step=Q.SCAN_CAP_S))
        self.assertEqual(got.unread, 3)
        self.assertEqual(got.folders, ())

    def test_a_scan_under_its_cap_reports_the_time_it_took(self):
        self.file("a/build-cpp/old.obj", 9000)
        got = self.scan(clock=ClockFake(step=0.01))
        self.assertEqual(got.unread, 0)
        self.assertGreater(got.took_s, 0)

    def test_an_unreadable_folder_is_an_error_not_a_quiet_folder(self):
        self.file("a/build-cpp/old.obj", 9000)
        real = os.scandir
        denied = self.folder("a")

        def scandir(path):
            if path == denied:
                raise PermissionError("denied")
            return real(path)

        with unittest.mock.patch.object(Q.os, "scandir", scandir):
            got = self.scan()
        self.assertEqual(len(got.errors), 1)
        self.assertIn(denied, got.errors[0])

    def test_a_sub_folder_that_vanishes_during_the_scan_is_no_error(self):
        self.file("a/build-cpp/old.obj", 9000)
        gone = os.path.join(self.folder("a"), "tmp")
        os.makedirs(gone)
        real = os.scandir

        def scandir(path):
            if path == gone:
                raise FileNotFoundError("gone")
            return real(path)

        with unittest.mock.patch.object(Q.os, "scandir", scandir):
            got = self.scan()
        self.assertEqual(got.errors, ())
        self.assertEqual(got.folders, (Q.FolderAge(self.folder("a"), 9000.0),))


class LiveParsingTest(unittest.TestCase):
    def test_the_process_list_reads_this_process_and_the_system_rows(self):
        listed = {p.pid: p for p in Q.list_processes()}
        own = listed[os.getpid()]
        self.assertEqual(own.name, os.path.basename(sys.executable))
        command_line = ctypes.WinDLL("kernel32").GetCommandLineW
        command_line.restype = ctypes.c_wchar_p
        self.assertEqual(own.command_line, command_line())
        # Windows' System process has no command line, so its row reads None.
        self.assertEqual(listed[4], Q.Proc("System", 4, None))

    def test_the_power_scheme_is_the_name_in_brackets(self):
        said = Q.run_command(["powercfg", "/getactivescheme"])
        got = Q.power_scheme()
        self.assertTrue(got)
        self.assertNotIn("(", got)
        self.assertIn(f"({got})", said)

    def test_a_command_that_fails_or_overruns_raises_oserror_naming_it(self):
        def failing(cmd, **_kw):
            return subprocess.CompletedProcess(cmd, 3, stdout="", stderr="bad")

        def slow(cmd, **_kw):
            raise subprocess.TimeoutExpired(cmd, 1)

        with self.assertRaisesRegex(OSError, "powercfg.*exited 3: bad"):
            Q.run_command(["powercfg"], runner=failing)
        with self.assertRaisesRegex(OSError, "powercfg.*did not finish"):
            Q.run_command(["powercfg"], runner=slow)

    def test_a_command_runs_with_no_window_so_it_cannot_take_the_focus(self):
        # The child prints its console's window and how many processes share
        # that console. Started with no window, it is alone on a new console
        # that has none. Started without, it shares this test's console, or
        # gets a new console with a window when this test has none.
        child = ("import ctypes; k = ctypes.windll.kernel32; ids = (ctypes.c_uint * 8)(); "
                 "print(k.GetConsoleWindow(), k.GetConsoleProcessList(ids, 8))")
        said = Q.run_command([sys.executable, "-I", "-c", child])
        self.assertEqual(said.split(), ["0", "1"])


if __name__ == "__main__":
    unittest.main()
