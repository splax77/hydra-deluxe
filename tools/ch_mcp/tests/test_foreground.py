"""Tests for foreground.ForegroundWatch on this machine's real desktop.

The watch reads the real front window with foreground.foreground_window and
stamps events with the real clock. The window it watches as "the game" is a
real window each test creates and never shows, so it can never come to the
front: no test brings a window forward or sends input, and none reaches
Clone Hero. The count lines are checked against ch_stop replies recorded in
real runs, under testdata/ch_runs.
"""

from __future__ import annotations

import ctypes
import os
import re
import sys
import threading
import time
import unittest
from ctypes import wintypes

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import foreground as F  # noqa: E402

# How long a test waits on the watch's thread before it fails instead of hanging.
THREAD_CAP_S = 5.0

_CH_RUNS = os.path.join(_REPO_ROOT, "testdata", "ch_runs")
# The foreground lines of three real ch_stop replies, copied from the
# replies.txt of the run folders of the same name under
# hydra-data/ch-mcp-runs (2026-10-31/ls-d0-run1, 2026-10-31/ls-d1a-run2 and
# 2026-11-07/ls-r4-run1).
RECORDED_STOPS = [os.path.join(_CH_RUNS, run, "ch_stop-foreground.txt")
                  for run in ("ls-r4-run1", "ls-d0-run1", "ls-d1a-run2")]

# A user32 of the tests' own, so the argument types set here do not change the
# functions foreground.py calls through ctypes.windll.
_user32 = ctypes.WinDLL("user32", use_last_error=True)
_user32.CreateWindowExW.restype = wintypes.HWND
_user32.CreateWindowExW.argtypes = [
    wintypes.DWORD, wintypes.LPCWSTR, wintypes.LPCWSTR, wintypes.DWORD,
    ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
    wintypes.HWND, wintypes.HMENU, wintypes.HINSTANCE, wintypes.LPVOID]
_user32.DestroyWindow.argtypes = [wintypes.HWND]
_user32.DestroyWindow.restype = wintypes.BOOL
_user32.IsWindowVisible.argtypes = [wintypes.HWND]
_user32.IsWindowVisible.restype = wintypes.BOOL


class ForegroundWatchTest(unittest.TestCase):
    def _hidden_window(self) -> int:
        """A real top-level window that is never shown, so it never comes to
        the front. It is destroyed when the test ends."""
        hwnd = _user32.CreateWindowExW(0, "STATIC", "hydra test_foreground", 0,
                                       0, 0, 0, 0, None, None, None, None)
        if not hwnd:
            self.fail(f"CreateWindowExW failed: error {ctypes.get_last_error()}")
        self.addCleanup(_user32.DestroyWindow, hwnd)
        self.assertFalse(_user32.IsWindowVisible(hwnd))
        return hwnd

    def _running_watch(self, game: int, polls: int = 1):
        """Start a watch the way ch_start does (the real front-window reader,
        the real clock, the default poll time), on the window `game`, and
        return it once its thread has polled `polls` times, with the poll
        times it was asked to sleep. The wait fails at once when the thread
        ends before that, since no more polls can come."""
        slept = []
        enough = threading.Event()

        def sleep(s):
            slept.append(s)
            if len(slept) >= polls:
                enough.set()
            time.sleep(s)

        w = F.ForegroundWatch(lambda: game, F.foreground_window, time.monotonic, sleep)
        w.start()
        self.addCleanup(w.stop)
        deadline = time.monotonic() + THREAD_CAP_S
        while not enough.wait(0.01):
            if not w._thread.is_alive():
                self.fail(f"the watch's thread ended after {len(slept)} polls: {w.error}")
            if time.monotonic() > deadline:
                self.fail(f"the watch's thread polled {len(slept)} times in {THREAD_CAP_S} s")
        return w, slept

    def test_the_watch_waits_the_poll_time_between_two_polls(self):
        _w, slept = self._running_watch(self._hidden_window(), polls=2)
        self.assertEqual(slept[:2], [F.FOREGROUND_POLL_S, F.FOREGROUND_POLL_S])

    def test_a_game_window_that_stays_behind_counts_nothing(self):
        game = self._hidden_window()
        w, _slept = self._running_watch(game, polls=3)
        w.stop()
        self.assertEqual(w.count, 0)
        self.assertEqual(w.game_times, [])
        # The first poll records the real front window as a change.
        self.assertGreaterEqual(len(w.events), 1)
        self.assertNotIn(game, [h for _t, h in w.events])

    def test_the_line_is_marked_unfinished_while_the_watch_runs(self):
        w, _slept = self._running_watch(self._hidden_window())
        running = w.line()
        w.stop()
        stopped = w.line()
        self.assertEqual(stopped, F.count_line(w.count))
        self.assertTrue(running.startswith(F.count_line(w.count)))
        self.assertNotEqual(running, stopped)

    def test_the_line_has_no_times_while_the_game_never_came_forward(self):
        w, _slept = self._running_watch(self._hidden_window(), polls=2)
        w.stop()
        self.assertEqual(w.line().splitlines(), [F.count_line(0)])

    def test_nothing_is_recorded_after_stop(self):
        game = self._hidden_window()
        polled = F.ForegroundWatch(lambda: game, F.foreground_window, time.monotonic,
                                   time.sleep)
        polled.poll()
        # The first poll of a watch records the front window, whatever it is.
        self.assertEqual(len(polled.events), 1)
        stopped = F.ForegroundWatch(lambda: game, F.foreground_window, time.monotonic,
                                    time.sleep)
        stopped.stop()
        stopped.poll()
        self.assertEqual(stopped.events, [])
        self.assertEqual(stopped.count, 0)

    def test_the_count_line_reads_as_real_runs_recorded_it(self):
        for path in RECORDED_STOPS:
            with self.subTest(path=path):
                with open(path, encoding="utf-8") as f:
                    header, recorded = f.read().splitlines()
                self.assertEqual(header, "== ch_stop")
                count = int(re.search(r"\d+", recorded).group())
                self.assertEqual(F.count_line(count), recorded)


if __name__ == "__main__":
    unittest.main()
