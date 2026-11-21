"""Tests for engine_finder that need no game: the scan pattern's layout and
the finder's give-up cap.

The finder's choice of the live engine (whose clock moves, which decoys it
skips, which constants it scans for) is checked only against a running Clone
Hero, because no saved real memory or clock reads exist to drive it here.
"""

from __future__ import annotations

import ctypes
import io
import os
import struct
import sys
import time
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import engine_finder as F  # noqa: E402
from tools.ch_probe import process as P  # noqa: E402


def _window_bytes(seconds: float) -> bytes:
    """One window constant as the game stores it: a little-endian double,
    the form process.decode_double reads back."""
    raw = struct.pack("<d", seconds)
    assert P.decode_double(raw) == seconds
    return raw


class ScanPatternTest(unittest.TestCase):
    def test_pattern_is_the_bytes_an_engine_holds_from_its_back_window(self):
        back = _window_bytes(C.EXPECT_NORMAL_BACK_S)
        front = _window_bytes(C.EXPECT_NORMAL_FRONT_S)
        # Lay the two constants out at the engine's own offsets.
        obj = bytearray(C.OFF_FRONT_WINDOW + len(front))
        obj[C.OFF_BACK_WINDOW:C.OFF_BACK_WINDOW + len(back)] = back
        obj[C.OFF_FRONT_WINDOW:C.OFF_FRONT_WINDOW + len(front)] = front
        pattern = F.scan_pattern(back, front)
        self.assertEqual(
            bytes(obj[C.OFF_BACK_WINDOW:C.OFF_BACK_WINDOW + len(pattern)]),
            pattern)

    def test_back_bytes_that_do_not_reach_the_front_window_raise(self):
        # A 4-byte float back value leaves a gap before the front window,
        # which one contiguous scan cannot match.
        short_back = struct.pack("<f", C.EXPECT_NORMAL_BACK_S)
        front = _window_bytes(C.EXPECT_NORMAL_FRONT_S)
        with self.assertRaises(ValueError):
            F.scan_pattern(short_back, front)


def _own_process() -> P.Process:
    """A real Process over this test's own process. The finder reads no
    memory here, because it is given no pattern to search."""
    handle = ctypes.WinDLL("kernel32").GetCurrentProcess()
    return P.Process(0, P.make_reader(handle), P.make_writer(handle),
                     pid=os.getpid(), handle=handle)


class FindLiveEngineCapTest(unittest.TestCase):
    def test_gives_up_once_the_cap_has_passed_on_the_real_clock(self):
        # With no pattern, no candidate turns up, just as when a scan finds
        # no engine. The cap allows two rounds of pauses, so the test ends
        # in about a second.
        cap = 2 * (F.REREAD_GAP_S + F.RETRY_PAUSE_S)
        events = []   # ("now", reading) and ("sleep", seconds), in order

        def now():
            t = time.monotonic()
            events.append(("now", t))
            return t

        def sleep(seconds):
            events.append(("sleep", seconds))
            time.sleep(seconds)

        with self.assertRaises(F.EngineSearchTimeout) as caught:
            F.find_live_engine(_own_process(), [], cap_s=cap,
                               sleep=sleep, now=now, out=io.StringIO())

        # The message names the cap it gave up at.
        self.assertIn(f"{cap:g} s", str(caught.exception))
        start = events[0][1]
        # The finder gives up on a clock reading at or past the cap.
        self.assertEqual(events[-1][0], "now")
        self.assertGreaterEqual(events[-1][1] - start, cap)
        # Every sleep follows a reading still under the cap.
        for before, after in zip(events, events[1:]):
            if after[0] == "sleep":
                self.assertEqual(before[0], "now")
                self.assertLess(before[1] - start, cap)
        # So the time slept stays under the cap plus one round of pauses.
        slept = sum(s for kind, s in events if kind == "sleep")
        self.assertGreater(slept, 0)
        self.assertLess(slept, cap + F.REREAD_GAP_S + F.RETRY_PAUSE_S)


if __name__ == "__main__":
    unittest.main()
