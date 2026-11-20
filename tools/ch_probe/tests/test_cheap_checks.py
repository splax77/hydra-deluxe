"""Tests for the overlay spike's session-log tail (log_tail.py).

No game: the tail reads real Clone Hero session logs saved under
testdata/ch_runs/ch-session-logs, on the real clock, and its printed lines are
checked against the printout of a real run kept in
tools/ch_probe/experiments/results/overlay-spike/run1_cal0_log_tail.txt.
"""

from __future__ import annotations

import datetime
import io
import os
import sys
import tempfile
import time
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import log_tail  # noqa: E402

# Two real session logs, copied from Clone Hero's own Logs folder without the
# header lines (log id and machine details) the game writes first.
LOGS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ch-session-logs")
# The 2026-10-17 practice-mode session: seeks, a song load and the countdown.
PRACTICE_LOG = os.path.join(LOGS, "Session-2026-10-17_19-29-33.log")
# The log the real run below watched: the lines before its start, then the
# two lines that arrived while it ran.
WATCHED_LOG = os.path.join(LOGS, "Session-2026-10-28_20-34-47.log")
# What log_tail.py printed while it watched that log on 2026-10-29.
REAL_PRINTOUT = os.path.join(
    _REPO_ROOT, "tools", "ch_probe", "experiments", "results",
    "overlay-spike", "run1_cal0_log_tail.txt")


def read_lines(path):
    with open(path, encoding="utf-8") as f:
        return f.read().splitlines()


def printed_log_lines(printout):
    """The printout's stamped lines, as (stamp, rest) pairs."""
    pairs = []
    for line in printout:
        stamp, sep, rest = line.partition("  ")
        try:
            datetime.datetime.strptime(stamp, "%H:%M:%S.%f")
        except ValueError:
            continue
        pairs.append((stamp, rest))
    return pairs


# --- log_tail -----------------------------------------------------------------

class ParseLogLineTest(unittest.TestCase):
    def test_loading_song_prints_as_the_real_run_printed_it(self):
        # Each stamped line of the real printout is the watched log's line,
        # marked with the event parse_log_line named. Rebuilding it from the
        # log's own line must give the same text, the song's name included.
        printed = printed_log_lines(read_lines(REAL_PRINTOUT))
        log_lines = read_lines(WATCHED_LOG)
        self.assertEqual(len(printed), 2)
        for stamp, rest in printed:
            source = [line for line in log_lines if rest.startswith(line)]
            self.assertEqual(len(source), 1, rest)
            self.assertEqual(log_tail.format_line(stamp, source[0]),
                             f"{stamp}  {rest}")
        loading = log_tail.parse_log_line(log_lines[-2])
        self.assertIsNotNone(loading)
        self.assertIsNone(log_tail.parse_log_line(log_lines[-1]))

    def test_countdown_start_and_end_are_two_different_events(self):
        lines = read_lines(PRACTICE_LOG)
        # The real log's two countdown lines, in the order the game wrote them.
        countdown = [line for line in lines
                     if line.startswith("[Log] OnCountdown")]
        self.assertEqual(len(countdown), 2)
        start, end = (log_tail.parse_log_line(line) for line in countdown)
        self.assertIsNotNone(start)
        self.assertIsNotNone(end)
        self.assertIsNone(start[1])
        self.assertIsNone(end[1])
        self.assertNotEqual(start[0], end[0])
        # No other line of the real log is named as either of them.
        other_kinds = {log_tail.parse_log_line(line)[0] for line in lines
                       if log_tail.parse_log_line(line) is not None
                       and not line.startswith("[Log] OnCountdown")}
        self.assertNotIn(start[0], other_kinds)
        self.assertNotIn(end[0], other_kinds)

    def test_seek_times_are_numbers_of_seconds(self):
        lines = read_lines(PRACTICE_LOG)
        seeks = [log_tail.parse_log_line(line) for line in lines
                 if line.startswith("[Log] Seeking to song time:")]
        self.assertGreater(len(seeks), 1)
        self.assertEqual(len({kind for kind, _ in seeks}), 1)
        times = [value for _, value in seeks]
        for value in times:
            self.assertIsInstance(value, float)
        # The log's first seek ("20") is its earliest song time; compared as
        # text it would sort after "108.896256", so only a number puts it first.
        self.assertEqual(min(times), times[0])
        self.assertLess(min(str(t) for t in times), str(times[0]))

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
    # 123999 microseconds: cutting to the millisecond gives .123, rounding .124.
    WALL = datetime.datetime(2026, 10, 28, 19, 30, 1, 123999)

    def test_wall_stamp_is_to_the_millisecond(self):
        self.assertEqual(log_tail.wall_stamp(self.WALL), "19:30:01.123")

    def test_prints_new_lines_whole_and_stops_at_the_cap(self):
        # Replays the real run: the watched log's earlier lines are on disk
        # when the tail starts, then its two new lines arrive, the first one
        # cut in half across two writes. The tail runs on the real clock.
        with open(WATCHED_LOG, "rb") as f:
            data = f.read()
        arrived_at = data.index(b"[Log] Loading Song: ")
        cut = arrived_at + len(b"[Log] Loading Song: ")
        writes = [data[arrived_at:cut], data[cut:]]
        cap_s = 5 * log_tail.POLL_S
        moments = []

        def wall():
            moments.append(datetime.datetime.now())
            return moments[-1]

        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, os.path.basename(WATCHED_LOG))
            with open(path, "wb") as f:
                f.write(data[:arrived_at])

            def sleep(seconds):
                if writes:
                    with open(path, "ab") as f:
                        f.write(writes.pop(0))
                time.sleep(seconds)

            out = io.StringIO()
            started = time.monotonic()
            printed = log_tail.tail(path, cap_s=cap_s, sleep=sleep,
                                    wall=wall, out=out)
            took = time.monotonic() - started

        real = read_lines(REAL_PRINTOUT)
        real_printed = printed_log_lines(real)
        lines = out.getvalue().splitlines()
        self.assertEqual(printed, len(real_printed))
        got = printed_log_lines(lines)
        # The same lines as the real run, each whole, none of the earlier ones.
        self.assertEqual([rest for _, rest in got],
                         [rest for _, rest in real_printed])
        stamps = {log_tail.wall_stamp(m) for m in moments}
        for stamp, _ in got:
            self.assertIn(stamp, stamps)
        # The real run stopped at the default cap; this one at its own.
        real_stop = [line for line in real if line.startswith("Stopped")]
        self.assertEqual(lines[len(got):], [
            real_stop[0].replace(f"{log_tail.TAIL_CAP_S:g}", f"{cap_s:g}")])
        self.assertGreaterEqual(took, cap_s)
        self.assertLess(took, cap_s + 2 * log_tail.POLL_S)


if __name__ == "__main__":
    unittest.main()
