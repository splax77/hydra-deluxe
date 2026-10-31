"""Tests for foreground.ForegroundWatch on scripted front windows.

No real window is read: the front window and the game's window are scripts.
"""

from __future__ import annotations

import os
import sys
import threading
import time
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import foreground as F  # noqa: E402

GAME = 0x1234
USER = 0x0AAA
OTHER = 0x0BBB
# How long a test waits on the watch's thread before it fails instead of hanging.
THREAD_CAP_S = 5.0


class Script:
    """Hands out the scripted front windows one per poll, then repeats the last."""

    def __init__(self, handles):
        self.handles = list(handles)
        self.i = 0

    def __call__(self) -> int:
        handle = self.handles[min(self.i, len(self.handles) - 1)]
        self.i += 1
        return handle


def _watch(handles, game=lambda: GAME):
    clock = iter(range(1000))
    return F.ForegroundWatch(game, Script(handles), lambda: float(next(clock)),
                             lambda s: None)


class ForegroundWatchTest(unittest.TestCase):
    def test_the_poll_is_the_users_tenth_of_a_second(self):
        self.assertEqual(F.FOREGROUND_POLL_S, 0.1)
        self.assertEqual(_watch([USER]).poll_s, 0.1)

    def test_counts_two_changes_to_the_game_and_none_for_other_windows(self):
        w = _watch([USER, GAME, GAME, OTHER, USER, GAME, GAME, OTHER])
        for _ in range(8):
            w.poll()
        self.assertEqual(w.count, 2)
        self.assertEqual([h for _t, h in w.events], [USER, GAME, OTHER, USER, GAME, OTHER])

    def test_other_windows_alone_count_nothing(self):
        w = _watch([USER, OTHER, USER, OTHER])
        for _ in range(4):
            w.poll()
        self.assertEqual(w.count, 0)
        self.assertEqual(len(w.events), 4)

    def test_a_game_in_front_at_the_first_poll_counts(self):
        w = _watch([GAME])
        w.poll()
        self.assertEqual(w.count, 1)

    def test_a_game_window_found_one_poll_late_still_counts_once(self):
        found = Script([0, GAME, GAME])
        w = _watch([GAME, GAME, GAME], game=found)
        for _ in range(3):
            w.poll()
        self.assertEqual(w.count, 1)

    def test_no_game_window_never_counts_an_empty_front(self):
        w = _watch([0, 0], game=lambda: 0)
        w.poll()
        w.poll()
        self.assertEqual(w.count, 0)

    def test_nothing_counts_after_stop(self):
        w = _watch([USER, GAME])
        w.poll()
        w.stop()
        w.poll()
        self.assertEqual(w.count, 0)
        self.assertEqual(w.events, [(0.0, USER)])

    def test_the_thread_polls_until_stop_and_the_line_says_the_count(self):
        handles = [USER, GAME, USER]
        done = threading.Event()
        script = Script(handles)

        def sleep(_s):
            if script.i >= len(handles):
                done.set()
            time.sleep(0.001)

        w = F.ForegroundWatch(lambda: GAME, script, time.monotonic, sleep)
        w.start()
        self.assertTrue(done.wait(THREAD_CAP_S))
        self.assertEqual(w.line(), "foreground went to the game 1 time so far")
        w.stop()
        self.assertEqual(w.count, 1)
        self.assertEqual(w.line(), "foreground went to the game 1 time")

    def test_a_poll_that_raises_ends_the_watch_and_the_line_says_why(self):
        def broken():
            raise OSError("no desktop")

        w = F.ForegroundWatch(lambda: GAME, broken, time.monotonic, lambda s: None)
        w._loop()  # the thread's body, run here so the test needs no wait
        self.assertEqual(w.line(), "foreground went to the game 0 times before the watch "
                                   "failed: OSError: no desktop")

    def test_count_line_words(self):
        self.assertEqual(F.count_line(0), "foreground went to the game 0 times")
        self.assertEqual(F.count_line(2), "foreground went to the game 2 times")


if __name__ == "__main__":
    unittest.main()
