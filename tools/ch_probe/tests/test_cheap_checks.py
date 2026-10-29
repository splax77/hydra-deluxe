"""Tests for the overlay spike's cheap checks: the session-log tail, the
topmost-window probe and the poll-cost timer.

No game: the log tail reads temporary files with a fake clock, the topmost
probe is only imported (its window is LIVE-ONLY), and the poll-cost timer runs
on fake callables and a real Process over fake memory (fakes.py).
"""

from __future__ import annotations

import datetime
import io
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import engine  # noqa: E402
from tools.ch_probe.experiments import log_tail, poll_cost, topmost_probe  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402

BASE = 0x180000000
OBJ = 0x2A0000000


class FakeClock:
    """A clock that moves only when a test says so."""

    def __init__(self, start: float = 0.0) -> None:
        self.t = start

    def now(self) -> float:
        return self.t

    def sleep(self, seconds: float) -> None:
        self.t += seconds


# --- log_tail -----------------------------------------------------------------

class ParseLogLineTest(unittest.TestCase):
    # Copied from C:\Users\Patrick\Documents\Clone Hero\Logs\
    # Session-2026-10-17_19-29-33.log (lines 37, 44, 45, 38 and 35), with the
    # CRLF the game writes.

    def test_loading_song(self):
        self.assertEqual(
            log_tail.parse_log_line(
                "[Log] Loading Song: Between the Buried and Me - Glide\r\n"),
            ("loading", "Between the Buried and Me - Glide"))

    def test_countdown_start(self):
        self.assertEqual(
            log_tail.parse_log_line("[Log] OnCountdownStart\r\n"),
            ("countdown_start", None))

    def test_countdown_end(self):
        self.assertEqual(
            log_tail.parse_log_line("[Log] OnCountdownEnd\r\n"),
            ("countdown_end", None))

    def test_seek(self):
        self.assertEqual(
            log_tail.parse_log_line("[Log] Seeking to song time:121.880544\r\n"),
            ("seek", 121.880544))

    def test_warning_line_is_not_an_event(self):
        self.assertIsNone(log_tail.parse_log_line(
            "[Warning] <Missing i18n Resource>: Current Section::en\r\n"))


class NewestSessionLogTest(unittest.TestCase):
    def test_picks_the_latest_write_not_the_latest_name(self):
        # The game's dates are not zero-padded, so the September log sorts
        # after the October one by name.
        with tempfile.TemporaryDirectory() as folder:
            older = os.path.join(folder, "Session-2026-10-17_19-29-33.log")
            newer = os.path.join(folder, "Session-2026-9-25_18-19-53.log")
            other = os.path.join(folder, "HitDetection-Standard.log")
            for path in (older, newer, other):
                with open(path, "w") as f:
                    f.write("x")
            os.utime(older, (1000, 1000))
            os.utime(newer, (2000, 2000))
            os.utime(other, (3000, 3000))
            self.assertEqual(log_tail.newest_session_log(folder), newer)

    def test_empty_folder_gives_none(self):
        with tempfile.TemporaryDirectory() as folder:
            self.assertIsNone(log_tail.newest_session_log(folder))


class TailTest(unittest.TestCase):
    WALL = datetime.datetime(2026, 10, 28, 19, 30, 1, 123456)

    def test_wall_stamp_is_to_the_millisecond(self):
        self.assertEqual(log_tail.wall_stamp(self.WALL), "19:30:01.123")

    def test_caps_are_the_plans_literals(self):
        self.assertEqual(log_tail.TAIL_CAP_S, 300.0)
        self.assertEqual(log_tail.POLL_S, 0.1)

    def test_prints_new_lines_whole_and_stops_at_the_cap(self):
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "Session-x.log")
            with open(path, "wb") as f:
                f.write(b"[Log] Starting video thumbs\r\n")  # before the tail

            clock = FakeClock()
            writes = [b"[Log] Loading Song: Between the ",   # half a line
                      b"Buried and Me - Glide\r\n[Log] OnCountdownStart\r\n"]

            def sleep(seconds):
                if writes:
                    with open(path, "ab") as f:
                        f.write(writes.pop(0))
                clock.sleep(seconds)

            out = io.StringIO()
            printed = log_tail.tail(path, now=clock.now, sleep=sleep,
                                    wall=lambda: self.WALL, out=out)

        self.assertEqual(printed, 2)
        self.assertEqual(out.getvalue().splitlines(), [
            "19:30:01.123  [Log] Loading Song: Between the Buried and Me - Glide"
            "    <= loading 'Between the Buried and Me - Glide'",
            "19:30:01.123  [Log] OnCountdownStart    <= countdown_start",
            "Stopped at the 300 s cap.",
        ])
        self.assertGreaterEqual(clock.t, 300.0)
        self.assertLess(clock.t, 300.0 + 2 * log_tail.POLL_S)


# --- topmost_probe ------------------------------------------------------------

class TopmostProbeTest(unittest.TestCase):
    def test_style_constants_are_the_win32_values(self):
        self.assertEqual(topmost_probe.WS_EX_TOPMOST, 0x00000008)
        self.assertEqual(topmost_probe.WS_EX_LAYERED, 0x00080000)
        self.assertEqual(topmost_probe.WS_EX_TRANSPARENT, 0x00000020)
        self.assertEqual(topmost_probe.WS_POPUP, 0x80000000)
        self.assertEqual(topmost_probe.EX_STYLE, 0x00080028)

    def test_size_and_lifetime_are_the_plans_literals(self):
        self.assertEqual(topmost_probe.WINDOW_W, 600)
        self.assertEqual(topmost_probe.WINDOW_H, 120)
        self.assertEqual(topmost_probe.LIFETIME_S, 60)

    def test_window_function_is_marked_live_only(self):
        self.assertIn("LIVE-ONLY", topmost_probe.run_probe.__doc__)


# --- poll_cost ----------------------------------------------------------------

class PollCostReportTest(unittest.TestCase):
    def _run(self, addresses):
        clock = FakeClock()
        find_steps = iter([0.5, 0.25, 1.0, 0.75, 2.0])
        found = iter(addresses)

        def find_once():
            clock.sleep(next(find_steps))
            return next(found)

        snap_steps = [0.001, 0.002, 0.004]
        reads = {"snap": 0, "single": 0}
        read_from = []

        def make_readers(address):
            read_from.append(address)

            def snapshot_read():
                clock.sleep(snap_steps[reads["snap"] % 3])
                reads["snap"] += 1

            def single_read():
                clock.sleep(0.0005)
                reads["single"] += 1

            return snapshot_read, single_read

        out = io.StringIO()
        code = poll_cost.measure(find_once, make_readers,
                                 [(10, 4096), (12, 8192)],
                                 perf=clock.now, out=out)
        return code, out.getvalue().splitlines(), reads, read_from

    def test_report_prints_min_median_max_of_fixed_durations(self):
        code, lines, reads, read_from = self._run([0x1000] * 5)
        self.assertEqual(code, 0)
        self.assertEqual(lines[1:], [
            "All 5 engine finds agree: 0x1000",
            "engine find: min 250.000 ms, median 750.000 ms, max 2000.000 ms (5 runs)",
            "memory scans: 2 in all; regions per scan 10 to 12; "
            "bytes per scan 4,096 to 8,192",
            "snapshot() read: min 1.000 ms, median 2.000 ms, max 4.000 ms (1000 runs)",
            "read_u32 read: min 0.500 ms, median 0.500 ms, max 0.500 ms (1000 runs)",
        ])
        self.assertEqual(reads, {"snap": 1000, "single": 1000})
        self.assertEqual(read_from, [0x1000])

    def test_two_addresses_print_the_disagreement_line(self):
        code, lines, _, read_from = self._run(
            [0x1000, 0x1000, 0x2000, 0x1000, 0x1000])
        self.assertEqual(code, 0)
        self.assertIn(
            "DISAGREE: the engine finds returned different addresses: "
            "0x1000, 0x1000, 0x2000, 0x1000, 0x1000", lines)
        self.assertNotIn("All 5 engine finds agree: 0x1000", lines)


class PollCostMainTest(unittest.TestCase):
    """main() wired to a real Process over fake memory and the real capped
    finder, with the memory scan faked."""

    def _process(self, clocks):
        consts = {C.RVA_CONST_NORMAL_BACK: C.EXPECT_NORMAL_BACK_S,
                  C.RVA_CONST_NORMAL_FRONT: C.EXPECT_NORMAL_FRONT_S}
        proc, _ = fakes.fake_process(
            {OBJ: bytes(engine.SNAPSHOT_SIZE)}, BASE,
            doubles={OBJ + C.OFF_TOTAL_WINDOW: 0.17},
            dwords={OBJ + C.OFF_SCORE: 1234},
            clocks={OBJ + C.OFF_SONG_CLOCK: clocks},
            consts=consts)
        return proc

    def _opener(self, proc, calls):
        def opener(**kwargs):
            calls.append(kwargs)
            return proc
        return opener

    def test_opens_read_only_and_reports_one_address(self):
        calls = []
        proc = self._process([i * 0.1 for i in range(40)])
        clock = FakeClock()
        out = io.StringIO()
        code = poll_cost.main(
            opener=self._opener(proc, calls),
            scan=lambda p, back, front: ([OBJ], 7, 65536),
            sleep=clock.sleep, now=clock.now, out=out)
        self.assertEqual(code, 0)
        self.assertEqual(calls, [{"read_only": True}])
        text = out.getvalue()
        self.assertIn(f"All 5 engine finds agree: {OBJ:#x}", text)
        self.assertIn("memory scans: 5 in all; regions per scan 7 to 7; "
                      "bytes per scan 65,536 to 65,536", text)

    def test_no_song_exits_2_with_the_finders_message(self):
        calls = []
        proc = self._process([0.0])
        clock = FakeClock()
        out = io.StringIO()
        code = poll_cost.main(
            opener=self._opener(proc, calls),
            scan=lambda p, back, front: ([], 0, 0),
            sleep=clock.sleep, now=clock.now, out=out)
        self.assertEqual(code, 2)
        self.assertEqual(calls, [{"read_only": True}])
        self.assertIn("no playing song found in 120 s; start the song and run again",
                      out.getvalue())

    def test_main_is_marked_live_only(self):
        self.assertIn("LIVE-ONLY", poll_cost.main.__doc__)


if __name__ == "__main__":
    unittest.main()
