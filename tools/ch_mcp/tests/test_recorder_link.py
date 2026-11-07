"""RecorderLink: the worker's end of the machine recorder, on a fake Popen."""

from __future__ import annotations

import os
import subprocess
import sys
import tempfile
import threading
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import recorder_link as R  # noqa: E402

# How long a fake waits for the close thread before the test fails.
THREAD_CAP_S = 5.0

HEADER = (b"sample,wall_s,sample_ms,processes,threads,ready_threads,clock_s,clock_wall_before_s,"
          b"clock_wall_after_s,recorder_pid,period_ms,stop_reason,process_time_s\n")


def sample_row(number: int, wall: str) -> bytes:
    return f"{number},{wall},1.2,300,4000,2,,,,777,20,,\n".encode()


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
    """A recorder that exits when its stdin closes, unless `hangs`."""

    def __init__(self, events: list, pid: int, hangs: bool):
        self.events = events
        self.pid = pid
        self.hangs = hangs
        self.stdin = FakeStdin(events)
        self.returncode = None

    def poll(self):
        return self.returncode

    def wait(self, timeout=None):
        self.events.append(("wait", timeout))
        if self.returncode is None and self.events.count("terminate") == 0:
            assert self.stdin.closed.wait(THREAD_CAP_S)
            if self.hangs:
                raise subprocess.TimeoutExpired("recorder", timeout)
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
        self.hangs = False
        self.error = None

    def __call__(self, command, **kwargs):
        self.calls.append((command, kwargs))
        if self.error is not None:
            raise self.error
        self.events.append("recorder start")
        self.procs.append(FakeProcess(self.events, 777 + len(self.procs), self.hangs))
        return self.procs[-1]


class RecorderLinkTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.session = self._tmp.name
        self.popen = FakePopen()
        self.link = R.RecorderLink(popen=self.popen)
        self.folder = os.path.join(self.session, "machine-1")

    def start(self):
        return self.link.start(self.session, 55, 120.5)

    def samples(self, data: bytes) -> None:
        with open(os.path.join(self.folder, "samples.csv"), "wb") as f:
            f.write(data)

    def test_the_exit_cap_is_the_users_two_seconds(self):
        self.assertEqual(R.RECORDER_EXIT_CAP_S, 2)

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
        with mock.patch("time.sleep") as sleep:
            lines = self.start()
        sleep.assert_not_called()
        self.assertEqual(self.popen.events, ["recorder start"])
        self.assertEqual(lines, [f"machine recorder: started, pid 777, folder {self.folder}"])

    def test_a_second_start_stops_the_first_and_takes_the_next_folder(self):
        self.start()
        lines = self.start()
        self.assertTrue(self.popen.procs[0].stdin.closed.is_set())
        self.assertEqual(lines, [
            f"machine recorder: stopped, pid 777, exit code 0, folder {self.folder}",
            "machine recorder: started, pid 778, folder "
            + os.path.join(self.session, "machine-2")])
        self.assertTrue(self.link.running())

    def test_a_start_that_raises_says_so_in_line_and_samples_line(self):
        self.popen.error = OSError("no python")
        with self.assertRaises(OSError):
            self.start()
        said = "machine recorder: not started (OSError: no python); the run goes on with no recording"
        self.assertEqual(self.link.line(), said)
        self.assertEqual(self.link.samples_line(), said)
        self.assertFalse(self.link.running())

    def test_send_writes_one_flushed_line_and_never_waits(self):
        self.start()
        self.assertIsNone(self.link.send("pid 7"))
        self.assertIsNone(self.link.send("engine 0x1000"))
        self.assertEqual(self.popen.events[1:],
                         [("stdin", b"pid 7\n"), ("stdin", b"engine 0x1000\n")])

    def test_a_failed_write_is_a_line_not_an_exception(self):
        self.start()
        self.popen.procs[0].stdin.fail = True
        self.assertEqual(self.link.send("pid 7"),
                         'machine recorder: "pid 7" not sent (OSError: the pipe is closed); '
                         "the run goes on")

    def test_send_with_no_recorder_has_nothing_to_say(self):
        self.assertIsNone(self.link.send("pid 7"))

    def test_stop_closes_stdin_and_waits_the_cap(self):
        self.start()
        said = self.link.stop()
        self.assertEqual(said, f"machine recorder: stopped, pid 777, exit code 0, folder {self.folder}")
        self.assertIn(("wait", R.RECORDER_EXIT_CAP_S), self.popen.events)
        self.assertNotIn("terminate", self.popen.events)
        self.assertFalse(self.link.running())
        self.assertEqual(self.link.line(), said)

    def test_stop_terminates_one_that_does_not_exit_and_says_so(self):
        self.popen.hangs = True
        self.start()
        self.assertEqual(self.link.stop(),
                         "machine recorder: pid 777 did not exit within RECORDER_EXIT_CAP_S (2 s) "
                         f"of its stdin closing; terminated, exit code 1, folder {self.folder}")
        self.assertEqual([e for e in self.popen.events if e == "terminate"], ["terminate"])

    def test_stop_says_when_terminate_does_not_end_it(self):
        self.popen.hangs = True
        self.start()
        proc = self.popen.procs[0]
        proc.terminate = lambda: None
        self.assertEqual(self.link.stop(),
                         "machine recorder: pid 777 did not exit within RECORDER_EXIT_CAP_S (2 s) "
                         "of its stdin closing; terminated, but it still runs 2 s later")

    def test_stop_says_when_terminate_fails(self):
        self.popen.hangs = True
        self.start()

        def refused():
            raise PermissionError("denied")

        self.popen.procs[0].terminate = refused
        self.assertIn("terminating it failed (PermissionError: denied)", self.link.stop())

    def test_a_close_that_blocks_does_not_hold_up_stop(self):
        self.popen.hangs = True
        self.start()
        proc = self.popen.procs[0]
        release = threading.Event()
        self.addCleanup(release.set)
        proc.stdin.close = lambda: release.wait(THREAD_CAP_S)
        proc.wait = lambda timeout=None: (_ for _ in ()).throw(
            subprocess.TimeoutExpired("recorder", timeout))
        proc.terminate = lambda: None
        self.assertIn("still runs", self.link.stop())
        self.assertFalse(release.is_set())

    def test_one_that_exited_is_not_running_and_stop_says_so_and_a_second_stop_is_harmless(self):
        self.start()
        proc = self.popen.procs[0]
        proc.returncode = 3
        said = f"machine recorder: exited by itself, pid 777, exit code 3, folder {self.folder}"
        self.assertFalse(self.link.running())
        self.assertEqual(self.link.line(), said)
        self.assertEqual(self.link.stop(), said)
        self.assertNotIn(("wait", R.RECORDER_EXIT_CAP_S), self.popen.events)
        self.assertNotIn("terminate", self.popen.events)
        self.assertIsNone(self.link.stop())
        self.assertEqual(self.link.line(), said)

    def test_line_says_none_before_any_start_and_stop_has_nothing_to_stop(self):
        self.assertEqual(self.link.line(), "machine recorder: none")
        self.assertIsNone(self.link.stop())

    def test_line_names_a_running_recorders_pid(self):
        self.start()
        self.assertEqual(self.link.line(), "machine recorder: running, pid 777")

    def test_samples_line_is_none_when_no_start_was_tried(self):
        self.assertIsNone(self.link.samples_line())

    def test_samples_line_with_no_file_yet(self):
        self.start()
        path = os.path.join(self.folder, "samples.csv")
        self.assertEqual(self.link.samples_line(), f"machine recorder: no samples yet in {path}")

    def test_samples_line_with_the_header_only_or_half_a_header(self):
        self.start()
        path = os.path.join(self.folder, "samples.csv")
        for data in (HEADER, HEADER[:10]):
            with self.subTest(data=data):
                self.samples(data)
                self.assertEqual(self.link.samples_line(),
                                 f"machine recorder: no samples yet in {path}")

    def test_samples_line_counts_rows_and_reads_the_last_wall_time(self):
        self.start()
        self.samples(HEADER + sample_row(0, "1000.25") + sample_row(1, "1000.27"))
        self.assertEqual(self.link.samples_line(),
                         "machine recorder: 2 samples so far, last at 1000.27")

    def test_samples_line_leaves_out_a_last_line_still_being_written(self):
        self.start()
        self.samples(HEADER + sample_row(0, "1000.25") + sample_row(1, "1000.27")[:9])
        self.assertEqual(self.link.samples_line(),
                         "machine recorder: 1 samples so far, last at 1000.25")

    def test_samples_line_reads_windows_line_ends(self):
        self.start()
        self.samples((HEADER + sample_row(0, "1000.25")).replace(b"\n", b"\r\n"))
        self.assertEqual(self.link.samples_line(),
                         "machine recorder: 1 samples so far, last at 1000.25")

    def test_samples_line_says_when_a_row_has_no_wall_time(self):
        self.start()
        self.samples(b"sample,other\n0,1\n")
        self.assertIn("1 samples so far, but no wall_s", self.link.samples_line())

    def test_samples_line_on_one_that_exited_says_both(self):
        self.start()
        self.samples(HEADER + sample_row(0, "1000.25"))
        self.popen.procs[0].returncode = 3
        self.assertEqual(self.link.samples_line(),
                         f"machine recorder: exited by itself, pid 777, exit code 3, folder "
                         f"{self.folder}; 1 samples so far, last at 1000.25")

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
