"""RecorderLink: the worker's end of the machine recorder. Where a test can run
the real recorder (tools/ch_mcp/machine_recorder.py) it does, and reads the pid
and folder from the recorder's own log; the rest still run on a fake Popen."""

from __future__ import annotations

import csv
import io
import os
import re
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import machine_recorder as MR  # noqa: E402
from tools.ch_mcp import recorder_link as R  # noqa: E402
from tools.ch_mcp.tests.test_machine_recorder import REAL_SAMPLES  # noqa: E402

# How long a fake waits for the close thread before the test fails.
THREAD_CAP_S = 5.0

# How long a test waits for a real recorder's first log line before it fails.
RECORDER_CAP_S = 10.0
# The run cap a test hands the real recorder. Its parent (this process) ending
# stops it sooner.
RUN_CAP_S = 30.0
# How long a recorder run by hand records before it ends by itself.
SELF_END_S = 0.5


class Started(list):
    """The real subprocess.Popen, keeping each process it starts so a test can
    read its exit code. `extra` is added to the command, the way a person who
    runs the recorder by hand adds --seconds."""

    def __init__(self, extra=()):
        super().__init__()
        self.extra = list(extra)

    def __call__(self, command, **kwargs):
        self.append(subprocess.Popen(command + self.extra, **kwargs))
        return self[-1]


def real_samples() -> bytes:
    """samples.csv from a real run, as test_machine_recorder's REAL_SAMPLES
    says where it came from."""
    with open(REAL_SAMPLES, "rb") as f:
        return f.read()


def sample_rows(data: bytes) -> list:
    """The whole rows of a samples.csv, read the way the recorder wrote them."""
    whole = data[:data.rindex(b"\n") + 1].decode("utf-8")
    return list(csv.DictReader(io.StringIO(whole, newline="")))


class FakeStdin:
    """A pipe that records each write as an event, or fails when told to."""

    def __init__(self, events: list):
        self.events = events
        self.fail = False
        self.closed = threading.Event()
        self.unflushed = b""

    def write(self, data: bytes) -> int:
        if self.fail:
            raise OSError("the pipe is closed")
        self.unflushed += data
        return len(data)

    def flush(self) -> None:
        if self.unflushed:
            self.events.append(("stdin", self.unflushed))
        self.unflushed = b""

    def close(self) -> None:
        self.events.append("stdin closed")
        self.closed.set()


class FakeProcess:
    """A recorder that exits when its stdin closes."""

    def __init__(self, events: list, pid: int):
        self.events = events
        self.pid = pid
        self.stdin = FakeStdin(events)
        self.returncode = None

    def poll(self):
        return self.returncode

    def wait(self, timeout=None):
        self.events.append(("wait", timeout))
        if self.returncode is None and self.events.count("terminate") == 0:
            assert self.stdin.closed.wait(THREAD_CAP_S)
            self.returncode = 0
        return self.returncode

    def terminate(self):
        self.events.append("terminate")
        self.returncode = 1


class FakePopen:
    """Stands in for subprocess.Popen. `error` is what the next start raises."""

    def __init__(self, events: list = None):
        self.events = events if events is not None else []
        self.calls = []
        self.procs = []
        self.error = None

    def __call__(self, command, **kwargs):
        self.calls.append((command, kwargs))
        if self.error is not None:
            raise self.error
        self.events.append("recorder start")
        self.procs.append(FakeProcess(self.events, 777 + len(self.procs)))
        return self.procs[-1]


class RecorderLinkTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.session = self._tmp.name
        self.popen = FakePopen()
        self.link = R.RecorderLink(popen=self.popen)
        self.folder = os.path.join(self.session, "machine-1")
        self.log = os.path.join(self.folder, R.LOG_NAME)

    def start(self):
        return self.link.start(self.session, 55, 120.5)

    def cut_samples(self, data: bytes) -> str:
        """`data` written as a samples.csv in a folder of its own; its path."""
        folder = os.path.join(self.session, f"cut-{len(os.listdir(self.session))}")
        os.makedirs(folder)
        path = os.path.join(folder, MR.SAMPLES_FILE)
        with open(path, "wb") as f:
            f.write(data)
        return path

    def self_ended(self):
        """A real recorder run by hand for SELF_END_S, which ends by itself:
        the link, its started processes and the recorder's pid. The recorder
        not ending within RECORDER_CAP_S fails the test."""
        link, started = self.real(extra=["--seconds", str(SELF_END_S)])
        self.real_start(link)
        pid, _ = self.recorder_says(link, self.folder)
        started[0].wait(RECORDER_CAP_S)
        self.assertTrue(self.last_log_line(self.folder).endswith(MR.STOP_SECONDS))
        return link, started, pid

    def real(self, extra=()):
        """A link on the real Popen, stopped when the test ends, and the list of
        the real processes it started."""
        started = Started(extra)
        link = R.RecorderLink(popen=started)
        self.addCleanup(link.stop)
        return link, started

    def real_start(self, link):
        return link.start(self.session, os.getpid(), RUN_CAP_S)

    def recorder_says(self, link, folder):
        """The pid and folder the real recorder's first log line names. The
        recorder ending before it writes the line fails at once, and so does the
        cap passing."""
        log = os.path.join(folder, R.LOG_NAME)
        deadline = time.monotonic() + RECORDER_CAP_S
        while True:
            alive = link.running()
            with open(log, encoding="utf-8", errors="replace") as f:
                first = f.readline()
            if first.endswith("\n"):
                said = re.match(r"machine recorder: pid (\d+), sampling thread \d+, folder (.+)$",
                                first.rstrip("\r\n"))
                self.assertIsNotNone(said, first)
                return int(said[1]), said[2]
            if not alive:
                self.fail(f"the recorder in {folder} ended before its first log line")
            if time.monotonic() > deadline:
                self.fail(f"no first log line in {log} within {RECORDER_CAP_S} s")
            time.sleep(1 / MR.RATE_PER_S)

    def last_log_line(self, folder):
        with open(os.path.join(folder, R.LOG_NAME), encoding="utf-8", errors="replace") as f:
            return f.read().strip().splitlines()[-1]

    def assert_names_pid(self, line, pid):
        self.assertRegex(line, rf"\bpid {pid}\b")

    def test_the_command_line_is_the_fixed_contract(self):
        self.start()
        command, _kwargs = self.popen.calls[0]
        self.assertEqual(command, [
            sys.executable, "-I",
            os.path.join(_REPO_ROOT, "tools", "ch_mcp", "machine_recorder.py"),
            "--out", self.folder, "--parent", "55", "--run-cap-s", "120.5"])

    def test_its_stdin_is_a_pipe_and_its_output_goes_to_a_log_in_the_folder(self):
        self.start()
        _command, kwargs = self.popen.calls[0]
        self.assertEqual(kwargs["stdin"], subprocess.PIPE)
        self.assertEqual(kwargs["stderr"], subprocess.STDOUT)
        self.assertEqual(os.path.dirname(kwargs["stdout"].name), self.folder)
        self.assertTrue(kwargs["stdout"].closed)

    def test_start_neither_sleeps_nor_waits_and_names_the_pid_and_folder(self):
        link, _started = self.real()
        with mock.patch("time.sleep") as sleep:
            lines = self.real_start(link)
        sleep.assert_not_called()
        # start came back while the recorder still runs: it did not wait for it.
        self.assertTrue(link.running())
        pid, folder = self.recorder_says(link, self.folder)
        self.assertEqual(folder, self.folder)
        self.assertEqual(len(lines), 1)
        self.assertIn("started", lines[0])
        self.assert_names_pid(lines[0], pid)
        self.assertIn(f"folder {folder}", lines[0])

    def test_a_second_start_stops_the_first_and_takes_the_next_folder(self):
        link, started = self.real()
        self.real_start(link)
        first_pid, _ = self.recorder_says(link, self.folder)
        stopped, second_started = self.real_start(link)
        second = os.path.join(self.session, "machine-2")
        second_pid, folder = self.recorder_says(link, second)
        self.assertEqual(folder, second)
        # The first recorder ended on its stdin closing, not on a terminate.
        self.assertTrue(self.last_log_line(self.folder).endswith(MR.STOP_STDIN))
        self.assertIn("stopped", stopped)
        self.assert_names_pid(stopped, first_pid)
        self.assertIn(f"exit code {started[0].returncode}", stopped)
        self.assertIn(f"folder {self.folder}", stopped)
        self.assertIn("started", second_started)
        self.assert_names_pid(second_started, second_pid)
        self.assertIn(f"folder {second}", second_started)
        self.assertTrue(link.running())

    def test_a_folder_that_already_holds_a_recording_is_skipped(self):
        os.makedirs(self.folder)
        link, _started = self.real()
        lines = self.real_start(link)
        second = os.path.join(self.session, "machine-2")
        _pid, folder = self.recorder_says(link, second)
        self.assertEqual(folder, second)
        self.assertIn(f"folder {second}", lines[0])
        self.assertEqual(os.listdir(self.folder), [])

    def test_a_start_that_raises_says_so_in_line_and_samples_line(self):
        link, started = self.real()
        no_python = os.path.join(self.session, "no-python", "python.exe")
        with mock.patch.object(sys, "executable", no_python):
            with self.assertRaises(OSError) as raised:
                self.real_start(link)
        self.assertEqual(started, [])
        said = R.start_failed_line(raised.exception)
        self.assertEqual(link.line(), said)
        self.assertEqual(link.samples_line(), said)
        self.assertFalse(link.running())

    def test_send_writes_one_flushed_line_and_never_waits(self):
        self.start()
        self.assertIsNone(self.link.send("pid 7"))
        self.assertIsNone(self.link.send("engine 0x1000"))
        self.assertEqual(self.popen.events[1:],
                         [("stdin", b"pid 7\n"), ("stdin", b"engine 0x1000\n")])

    def test_a_failed_write_is_a_line_not_an_exception(self):
        # A recorder that died leaves its stdin pipe with no reader, so the
        # write fails for real.
        link, started = self.real()
        self.real_start(link)
        self.recorder_says(link, self.folder)
        started[0].terminate()
        started[0].wait(RECORDER_CAP_S)
        said = link.send("pid 7")
        self.assertIsInstance(said, str)
        self.assertIn('"pid 7" not sent', said)
        # The run goes on: the link still answers, naming the recorder's end.
        self.assertIn(f"exit code {started[0].returncode}", link.line())

    def test_send_with_no_recorder_has_nothing_to_say(self):
        self.assertIsNone(self.link.send("pid 7"))

    def test_stop_closes_stdin_and_the_recorder_exits_on_it(self):
        link, started = self.real()
        self.real_start(link)
        pid, _ = self.recorder_says(link, self.folder)
        began = time.monotonic()
        said = link.stop()
        # It ended inside the cap, on its stdin closing, with no terminate.
        self.assertLess(time.monotonic() - began, R.RECORDER_EXIT_CAP_S)
        self.assertTrue(self.last_log_line(self.folder).endswith(MR.STOP_STDIN))
        self.assertEqual(started[0].returncode, 0)
        self.assertIn("stopped", said)
        self.assert_names_pid(said, pid)
        self.assertIn(f"exit code {started[0].returncode}", said)
        self.assertIn(f"folder {self.folder}", said)
        self.assertFalse(link.running())
        self.assertEqual(link.line(), said)

    def test_stop_terminates_one_that_does_not_exit_within_the_cap_and_says_so(self):
        # A recorder run by hand for a fixed time leaves its stdin unread, so
        # closing it does not end the recorder.
        link, started = self.real(extra=["--seconds", str(RUN_CAP_S)])
        self.real_start(link)
        pid, _ = self.recorder_says(link, self.folder)
        began = time.monotonic()
        said = link.stop()
        self.assertGreaterEqual(time.monotonic() - began, R.RECORDER_EXIT_CAP_S)
        self.assertIsNotNone(started[0].returncode)
        self.assertFalse(self.last_log_line(self.folder).endswith(MR.STOP_STDIN))
        self.assert_names_pid(said, pid)
        self.assertIn(f"({R.RECORDER_EXIT_CAP_S} s)", said)
        self.assertIn(f"terminated, exit code {started[0].returncode}", said)
        self.assertIn(f"folder {self.folder}", said)

    def test_one_that_exited_is_not_running_and_stop_says_so_and_a_second_stop_is_harmless(self):
        link, started, pid = self.self_ended()
        said = link.line()
        self.assertFalse(link.running())
        self.assertIn("exited by itself", said)
        self.assert_names_pid(said, pid)
        self.assertIn(f"exit code {started[0].returncode}", said)
        self.assertIn(f"folder {self.folder}", said)
        self.assertIn(f"log {self.log}", said)
        # A stop on it neither waits nor terminates: it says what became of it.
        began = time.monotonic()
        self.assertEqual(link.stop(), said)
        self.assertLess(time.monotonic() - began, R.RECORDER_EXIT_CAP_S)
        self.assertIsNone(link.stop())
        self.assertEqual(link.line(), said)

    def test_line_names_no_recorder_before_any_start_and_stop_has_nothing_to_stop(self):
        link = R.RecorderLink()
        line = link.line()
        self.assertTrue(line.startswith(R._LINE_PREFIX))
        self.assertNotRegex(line, r"\bpid\b|\bfolder\b|\brunning\b")
        self.assertFalse(link.running())
        self.assertIsNone(link.stop())
        self.assertEqual(link.line(), line)

    def test_line_names_a_running_recorders_pid(self):
        link, _started = self.real()
        self.real_start(link)
        pid, _ = self.recorder_says(link, self.folder)
        self.assertIn("running", link.line())
        self.assert_names_pid(link.line(), pid)

    def test_samples_line_says_the_run_is_not_recorded_when_no_start_was_tried(self):
        link = R.RecorderLink()
        said = link.samples_line()
        self.assertTrue(said.startswith(link.line()))
        self.assertIn("not recorded", said)
        self.assertNotRegex(said, r"\bpid\b|\bsamples\b")

    def test_samples_line_with_no_file_yet(self):
        path = os.path.join(self.session, MR.SAMPLES_FILE)
        said = R.samples_text(path)
        self.assertIn("no samples yet", said)
        self.assertIn(path, said)
        self.assertNotRegex(said, r"\d+ samples")

    def test_samples_line_with_the_header_only_or_half_a_header(self):
        # The file as the recorder leaves it after writing its header, and
        # part way through writing it.
        data = real_samples()
        header = data[:data.index(b"\n") + 1]
        for cut in (header, header[:len(header) // 2]):
            with self.subTest(cut=cut):
                path = self.cut_samples(cut)
                said = R.samples_text(path)
                self.assertIn("no samples yet", said)
                self.assertIn(path, said)

    def test_samples_line_counts_rows_and_reads_the_last_wall_time(self):
        data = real_samples()
        rows = sample_rows(data)
        said = R.samples_text(self.cut_samples(data))
        self.assertIn(f"{len(rows)} samples so far", said)
        self.assertIn(f"last at {rows[-1][MR.WALL_COLUMN]}", said)

    def test_samples_line_leaves_out_a_last_line_still_being_written(self):
        data = real_samples()
        last = data.rstrip(b"\n").rsplit(b"\n", 1)[1]
        # The last row half written, as the recorder may be writing it now.
        cut = data[:len(data) - 1 - len(last) // 2]
        rows = sample_rows(data)[:-1]
        said = R.samples_text(self.cut_samples(cut))
        self.assertIn(f"{len(rows)} samples so far", said)
        self.assertIn(f"last at {rows[-1][MR.WALL_COLUMN]}", said)

    def test_samples_line_on_one_that_exited_says_both(self):
        link, _started, _pid = self.self_ended()
        with open(os.path.join(self.folder, MR.SAMPLES_FILE), "rb") as f:
            rows = sample_rows(f.read())
        # The recorder's own last row says why it ended.
        self.assertEqual(rows[-1]["stop_reason"], MR.STOP_SECONDS)
        said = link.samples_line()
        self.assertTrue(said.startswith(link.line() + "; "), said)
        self.assertIn(f"{len(rows)} samples so far", said)
        self.assertIn(f"last at {rows[-1][MR.WALL_COLUMN]}", said)

    def test_samples_line_after_the_stop_is_the_stop_line(self):
        self.start()
        said = self.link.stop()
        self.assertEqual(self.link.samples_line(), said)

    def test_a_real_child_gets_the_lines_and_exits_when_stdin_closes(self):
        script = os.path.join(self.session, "stand_in.py")
        heard = os.path.join(self.session, "heard.txt")
        with open(script, "w", encoding="utf-8") as f:
            f.write("import sys\n"
                    f"data = sys.stdin.buffer.read()\n"
                    f"open({heard!r}, 'wb').write(data)\n"
                    "print('done')\n")
        link = R.RecorderLink()
        with mock.patch.object(R, "RECORDER_SCRIPT", script):
            link.start(self.session, os.getpid(), 1.0)
        self.assertIsNone(link.send("pid 7"))
        self.assertIsNone(link.send("engine 0x1000"))
        self.assertIn("stopped", link.stop())
        with open(heard, "rb") as f:
            self.assertEqual(f.read(), b"pid 7\nengine 0x1000\n")
        with open(os.path.join(self.folder, R.LOG_NAME), "rb") as f:
            self.assertEqual(f.read().strip(), b"done")


if __name__ == "__main__":
    unittest.main()
