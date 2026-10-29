"""Tests for engine_finder: the memory scan's match maths and the "whose clock
moves" check that picks the live engine.

No game: a real Process over fake memory (the shared builder in fakes.py)
answers the reads, and a fake scan hands back the candidate addresses a real
scan would find.
"""

from __future__ import annotations

import io
import os
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import engine_finder as F  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402

BASE = 0x180000000
W = C.OFF_TOTAL_WINDOW
K = C.OFF_SONG_CLOCK


def FakeProc(doubles=None, clocks=None, consts=None):
    """A real Process at module base BASE over fake memory: doubles by live
    address, clock lists read one value per call, .rdata bytes by RVA."""
    proc, _ = fakes.fake_process(None, BASE, doubles=doubles, clocks=clocks,
                                 consts=consts)
    return proc


class ScanPatternTest(unittest.TestCase):
    def test_front_window_sits_eight_bytes_after_back(self):
        # A guard: the scan's one 16-byte pattern is right only while the
        # front window starts right where the back window's 8 bytes end.
        self.assertEqual(C.OFF_FRONT_WINDOW - C.OFF_BACK_WINDOW, 8)
        self.assertEqual(F.FRONT_AFTER_BACK, 8)


class HitsInRegionTest(unittest.TestCase):
    def test_each_match_backs_up_to_the_object_start(self):
        pattern = b"B" * 8 + b"F" * 8
        data = b"\0" * 0x40 + pattern + b"\0" * 0x20 + pattern
        self.assertEqual(F.hits_in_region(0x5000, data, pattern),
                         [0x5000 + 0x40 - C.OFF_BACK_WINDOW,
                          0x5000 + 0x70 - C.OFF_BACK_WINDOW])


class FindLiveEngineTest(unittest.TestCase):
    def test_picks_the_engine_whose_clock_moves(self):
        frozen, live, in_module, empty = 0x10000, 0x20000, BASE + 0x100, 0x30000
        proc = FakeProc(
            doubles={frozen + W: 0.17, live + W: 0.17, in_module + W: 0.17,
                     empty + W: 0.0},
            clocks={frozen + K: [21.95], live + K: [1.0, 1.1],
                    in_module + K: [0.0, 5.0]})
        sleeps = []

        def scan(p, back, front):
            return [frozen, in_module, empty, live], 0, 0

        got = F.find_live_engine(proc, [(b"b", b"f")], scan=scan,
                                 sleep=sleeps.append, out=io.StringIO())
        self.assertEqual(got, live)
        self.assertEqual(sleeps, [F.REREAD_GAP_S])

    def test_gives_up_at_the_default_cap_with_the_exact_message(self):
        # The bare finder is capped too: the wall clock jumps past the
        # default cap before the first sleep, while no song clock moves.
        frozen = 0x20000
        proc = FakeProc(doubles={frozen + W: 0.17}, clocks={frozen + K: [3.0]})
        wall = iter([1000.0, 1120.0])
        sleeps = []
        with self.assertRaises(F.EngineSearchTimeout) as caught:
            F.find_live_engine(proc, [(b"b", b"f")],
                               scan=lambda p, b, f: ([frozen], 0, 0),
                               sleep=sleeps.append, now=lambda: next(wall),
                               out=io.StringIO())
        self.assertEqual(
            str(caught.exception),
            "no playing song found in 120 s; start the song and run again")
        self.assertEqual(sleeps, [])

    def test_keeps_waiting_until_a_clock_moves(self):
        live = 0x20000
        proc = FakeProc(doubles={live + W: 0.17},
                        clocks={live + K: [1.0, 1.0, 1.0, 1.2]})
        sleeps, out = [], io.StringIO()
        got = F.find_live_engine(proc, [(b"b", b"f")],
                                 scan=lambda p, b, f: ([live], 0, 0),
                                 sleep=sleeps.append, out=out)
        self.assertEqual(got, live)
        self.assertEqual(sleeps, [F.REREAD_GAP_S, F.RETRY_PAUSE_S, F.REREAD_GAP_S])
        self.assertEqual(out.getvalue(), ".")

    def test_a_candidate_two_patterns_find_is_checked_once_in_scan_order(self):
        a, b = 0x10000, 0x20000
        proc = FakeProc(doubles={a + W: 0.17, b + W: 0.17},
                        clocks={a + K: [1.0, 1.1], b + K: [2.0, 2.1]})
        answers = iter([([a, b], 0, 0), ([b, a], 0, 0)])
        got = F.find_live_engine(proc, [(b"n", b"n"), (b"p", b"p")],
                                 scan=lambda p, back, front: next(answers),
                                 sleep=lambda s: None, out=io.StringIO())
        self.assertEqual(got, a)


class FakeClock:
    """A wall clock that moves only when the code under test sleeps."""

    def __init__(self):
        self.t = 1000.0
        self.sleeps = []

    def now(self):
        return self.t

    def sleep(self, seconds):
        self.sleeps.append(seconds)
        self.t += seconds


class FindLiveEngineCappedTest(unittest.TestCase):
    def test_gives_up_after_the_cap_with_the_exact_message(self):
        frozen = 0x20000
        proc = FakeProc(doubles={frozen + W: 0.17}, clocks={frozen + K: [3.0]})
        clock = FakeClock()
        with self.assertRaises(F.EngineSearchTimeout) as caught:
            F.find_live_engine_capped(
                proc, [(b"b", b"f")], 2.0,
                scan=lambda p, b, f: ([frozen], 0, 0),
                sleep=clock.sleep, now=clock.now, out=io.StringIO())
        self.assertEqual(
            str(caught.exception),
            "no playing song found in 2 s; start the song and run again")
        # Each try sleeps 0.12 s then 0.4 s; the sleep that would start at or
        # past 2 s after the start raises instead.
        self.assertEqual(clock.sleeps, [F.REREAD_GAP_S, F.RETRY_PAUSE_S] * 4)

    def test_default_cap_message_names_120_seconds(self):
        frozen = 0x20000
        proc = FakeProc(doubles={frozen + W: 0.17}, clocks={frozen + K: [3.0]})
        clock = FakeClock()
        with self.assertRaises(F.EngineSearchTimeout) as caught:
            F.find_live_engine_capped(
                proc, [(b"b", b"f")],
                scan=lambda p, b, f: ([frozen], 0, 0),
                sleep=clock.sleep, now=clock.now, out=io.StringIO())
        self.assertEqual(
            str(caught.exception),
            "no playing song found in 120 s; start the song and run again")
        self.assertGreaterEqual(clock.t - 1000.0, 120.0)

    def test_returns_the_engine_found_inside_the_cap(self):
        live = 0x20000
        proc = FakeProc(doubles={live + W: 0.17},
                        clocks={live + K: [1.0, 1.0, 1.0, 1.2]})
        clock = FakeClock()
        got = F.find_live_engine_capped(
            proc, [(b"b", b"f")], 2.0,
            scan=lambda p, b, f: ([live], 0, 0),
            sleep=clock.sleep, now=clock.now, out=io.StringIO())
        self.assertEqual(got, live)
        self.assertEqual(clock.sleeps,
                         [F.REREAD_GAP_S, F.RETRY_PAUSE_S, F.REREAD_GAP_S])


class PatternsTest(unittest.TestCase):
    def test_all_patterns_tries_the_precision_pair_both_ways(self):
        consts = {
            C.RVA_CONST_NORMAL_BACK: b"NB" * 4,
            C.RVA_CONST_NORMAL_FRONT: b"NF" * 4,
            C.RVA_CONST_PRECISION_BACK: b"PB" * 4,
            C.RVA_CONST_PRECISION_FRONT: b"PF" * 4,
        }
        proc = FakeProc(consts=consts)
        self.assertEqual(F.normal_pattern(proc), (b"NB" * 4, b"NF" * 4))
        self.assertEqual(F.all_patterns(proc), [
            (b"NB" * 4, b"NF" * 4), (b"PB" * 4, b"PF" * 4), (b"PF" * 4, b"PB" * 4)])


if __name__ == "__main__":
    unittest.main()
