"""Tests for watch_checks.py: the hit-time and pause checks on a Star Power
watcher log. Pure logic over hand-written rows and small files; no game."""

from __future__ import annotations

import csv
import json
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import watch_checks as W  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402


def row(wall_s, clock_s, **fields):
    """A log row with every column zero except the ones given."""
    r = {c: 0 for c in WATCH_COLUMNS}
    r.update(wall_s=wall_s, clock_s=clock_s, **fields)
    return r


def hit_rows(hit_time_s):
    """Three rows around a chord at 1000 ms; the middle one changes the hit
    time to `hit_time_s`."""
    return [row(10.0, 0.95), row(10.05, 1.0, hit_time_s=hit_time_s),
            row(10.15, 1.1, hit_time_s=hit_time_s)]


class HitTimeCheckTest(unittest.TestCase):
    def test_a_hit_time_9_ms_from_the_chord_counts(self):
        lines = W.hit_time_check(hit_rows(1.009), [{"index": 7, "ms": 1000.0}])
        self.assertEqual(lines[0],
                         "PASS hit time (+0x2E0): 1 of 1 chords in the log have "
                         "a hit time within 10 ms (0 chords outside the log's "
                         "clock span)")

    def test_a_hit_time_11_ms_from_the_chord_does_not(self):
        lines = W.hit_time_check(hit_rows(1.011), [{"index": 7, "ms": 1000.0}])
        self.assertTrue(lines[0].startswith("FAIL hit time (+0x2E0): 0 of 1 "))
        self.assertEqual(lines[-1],
                         "  chord 7 at 1000.0 ms: hit time 1011.0 ms, +11.0 ms away")

    def test_a_hit_time_exactly_10_ms_from_the_chord_counts(self):
        rows = [row(10.0, 0.95), row(10.05, 0.99, hit_time_s=1.0),
                row(10.15, 1.1, hit_time_s=1.0)]
        lines = W.hit_time_check(rows, [{"index": 7, "ms": 990.0}])
        self.assertTrue(lines[0].startswith("PASS"))

    def test_a_change_more_than_50_ms_from_the_chord_is_not_searched(self):
        rows = [row(10.0, 0.9), row(10.05, 1.051, hit_time_s=1.0),
                row(10.15, 1.1, hit_time_s=1.0)]
        lines = W.hit_time_check(rows, [{"index": 7, "ms": 1000.0}])
        self.assertTrue(lines[0].startswith("FAIL"))
        self.assertEqual(lines[-1],
                         "  chord 7 at 1000.0 ms: no hit-time change within 50 ms")

    def test_two_close_chords_each_take_their_own_change(self):
        rows = [row(10.0, 0.95), row(10.05, 1.0, hit_time_s=1.001),
                row(10.08, 1.03, hit_time_s=1.031),
                row(10.15, 1.1, hit_time_s=1.031)]
        chords = [{"index": 1, "ms": 1000.0}, {"index": 2, "ms": 1030.0}]
        self.assertTrue(W.hit_time_check(rows, chords)[0].startswith(
            "PASS hit time (+0x2E0): 2 of 2 "))

    def test_a_nan_hit_time_held_before_the_hit_is_not_a_change(self):
        nan = float("nan")
        rows = [row(10.0, 0.9, hit_time_s=nan), row(10.02, 0.93, hit_time_s=nan),
                row(10.05, 1.0, hit_time_s=1.005),
                row(10.15, 1.1, hit_time_s=1.005)]
        lines = W.hit_time_check(rows, [{"index": 7, "ms": 1000.0}])
        self.assertTrue(lines[0].startswith("PASS hit time (+0x2E0): 1 of 1 "))

    def test_chords_outside_the_log_are_not_judged(self):
        chords = [{"index": 1, "ms": 500.0}, {"index": 2, "ms": 1000.0}]
        lines = W.hit_time_check(hit_rows(1.009), chords)
        self.assertTrue(lines[0].startswith("PASS"))
        self.assertIn("(1 chords outside the log's clock span)", lines[0])


class PauseCheckTest(unittest.TestCase):
    def test_a_2_s_wall_gap_with_a_held_clock_reports_the_span(self):
        rows = [row(10.0, 5.0), row(10.5, 5.4, hit_time_s=5.39),
                row(12.5, 5.41, hit_time_s=5.39),
                row(12.6, 5.5, hit_time_s=5.39)]
        self.assertEqual(W.pause_check(rows), [
            "PASS pause: wall 10.500 to 12.500 s (2.000 s), clock 5.400 to "
            "5.410 s",
            "  other fields: none moved"])

    def test_a_field_that_moved_across_the_pause_fails(self):
        rows = [row(10.0, 5.0), row(10.5, 5.4),
                row(12.5, 5.41, sp_amount_u32=96), row(12.6, 5.5)]
        self.assertEqual(W.pause_check(rows), [
            "FAIL pause: wall 10.500 to 12.500 s (2.000 s), clock 5.400 to "
            "5.410 s",
            "  other fields: sp_amount_u32 0 -> 96"])

    def test_a_pause_into_the_final_row_says_so(self):
        rows = [row(10.0, 5.0), row(15.1, 5.0)]
        self.assertEqual(W.pause_check(rows)[0],
                         "PASS pause: wall 10.000 to 15.100 s (5.100 s), clock "
                         "5.000 to 5.000 s (ends at the log's final row, where "
                         "the watcher stopped)")

    def test_a_nan_read_held_across_the_pause_is_not_a_move(self):
        nan = float("nan")
        rows = [row(10.0, 5.0), row(10.5, 5.4, sp_end_s=nan),
                row(12.5, 5.41, sp_end_s=nan), row(12.6, 5.5)]
        self.assertEqual(W.pause_check(rows)[1], "  other fields: none moved")

    def test_a_wall_gap_where_the_clock_moved_is_not_a_pause(self):
        rows = [row(10.0, 5.0), row(12.0, 7.0)]
        self.assertEqual(W.pause_check(rows), ["pause: no pause in this log"])


class FilesTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.tmp.cleanup()

    def write_csv(self, header, rows):
        path = os.path.join(self.tmp.name, "watch.csv")
        with open(path, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(header)
            w.writerows(rows)
        return path

    def test_rows_load_with_64_bit_values_exact(self):
        values = [1000.25, 1.0, 0, 7680, 4294974976, 6144, 8589940736, 1,
                  257, 8, 520, 61.25, 1, 0.995]
        path = self.write_csv(WATCH_COLUMNS, [values])
        got = W.load_watch_rows(path)
        self.assertEqual(got, [dict(zip(WATCH_COLUMNS, values))])
        self.assertIsInstance(got[0]["sp_amount_u64"], int)

    def test_a_wrong_header_is_refused(self):
        path = self.write_csv(("wall_s", "clock_s"), [[1.0, 2.0]])
        with self.assertRaises(ValueError):
            W.load_watch_rows(path)

    def test_run_reads_both_files(self):
        csv_path = self.write_csv(
            WATCH_COLUMNS, [[r[c] for c in WATCH_COLUMNS]
                            for r in hit_rows(1.009)])
        score = os.path.join(self.tmp.name, "score.json")
        with open(score, "w", encoding="utf-8") as f:
            json.dump({"chords": [{"index": 7, "ms": 1000.0}]}, f)
        lines = W.run([csv_path, "--score", score])
        self.assertEqual(lines[0], "watch.csv: 3 rows")
        self.assertTrue(lines[1].startswith("PASS hit time"))
        self.assertEqual(lines[-1], "pause: no pause in this log")


if __name__ == "__main__":
    unittest.main()
