"""Tests for watch_checks.py: the +0x2E0 and pause checks on a Star Power
watcher log. Pure logic over hand-written rows and small files; no game."""

from __future__ import annotations

import csv
import json
import math
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import watch_checks as W  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import (  # noqa: E402
    WATCH_CLOCK_COLUMNS, WATCH_COLUMNS, WATCH_COLUMNS_WITHOUT_LATE_GROUPS,
    WATCH_HALF_COLUMNS)


def row(wall_s, clock_s, **fields):
    """A log row with every column zero except the ones given."""
    r = {c: 0 for c in WATCH_COLUMNS}
    r.update(wall_s=wall_s, clock_s=clock_s, **fields)
    return r


def hit_rows(round_start_s):
    """Three rows around a chord at 1000 ms; the middle one changes the
    +0x2E0 column to `round_start_s`."""
    return [row(10.0, 0.95), row(10.05, 1.0, round_start_s=round_start_s),
            row(10.15, 1.1, round_start_s=round_start_s)]


class RoundStartCheckTest(unittest.TestCase):
    def test_a_value_9_ms_from_the_chord_counts(self):
        lines = W.round_start_check(hit_rows(1.009), [{"index": 7, "ms": 1000.0}])
        self.assertEqual(lines[0],
                         "PASS +0x2E0: 1 of 1 chords in the log have "
                         "a +0x2E0 value within 10 ms (0 chords outside the log's "
                         "clock span)")

    def test_a_value_11_ms_from_the_chord_does_not(self):
        lines = W.round_start_check(hit_rows(1.011), [{"index": 7, "ms": 1000.0}])
        self.assertTrue(lines[0].startswith("FAIL +0x2E0: 0 of 1 "))
        self.assertEqual(lines[-1],
                         "  chord 7 at 1000.0 ms: +0x2E0 1011.0 ms, +11.0 ms away")

    def test_a_value_exactly_10_ms_from_the_chord_counts(self):
        rows = [row(10.0, 0.95), row(10.05, 0.99, round_start_s=1.0),
                row(10.15, 1.1, round_start_s=1.0)]
        lines = W.round_start_check(rows, [{"index": 7, "ms": 990.0}])
        self.assertTrue(lines[0].startswith("PASS"))

    def test_a_change_more_than_50_ms_from_the_chord_is_not_searched(self):
        rows = [row(10.0, 0.9), row(10.05, 1.051, round_start_s=1.0),
                row(10.15, 1.1, round_start_s=1.0)]
        lines = W.round_start_check(rows, [{"index": 7, "ms": 1000.0}])
        self.assertTrue(lines[0].startswith("FAIL"))
        self.assertEqual(lines[-1],
                         "  chord 7 at 1000.0 ms: no +0x2E0 change within 50 ms")

    def test_two_close_chords_each_take_their_own_change(self):
        rows = [row(10.0, 0.95), row(10.05, 1.0, round_start_s=1.001),
                row(10.08, 1.03, round_start_s=1.031),
                row(10.15, 1.1, round_start_s=1.031)]
        chords = [{"index": 1, "ms": 1000.0}, {"index": 2, "ms": 1030.0}]
        self.assertTrue(W.round_start_check(rows, chords)[0].startswith(
            "PASS +0x2E0: 2 of 2 "))

    def test_a_nan_value_held_before_the_hit_is_not_a_change(self):
        nan = float("nan")
        rows = [row(10.0, 0.9, round_start_s=nan), row(10.02, 0.93, round_start_s=nan),
                row(10.05, 1.0, round_start_s=1.005),
                row(10.15, 1.1, round_start_s=1.005)]
        lines = W.round_start_check(rows, [{"index": 7, "ms": 1000.0}])
        self.assertTrue(lines[0].startswith("PASS +0x2E0: 1 of 1 "))

    def test_chords_outside_the_log_are_not_judged(self):
        chords = [{"index": 1, "ms": 500.0}, {"index": 2, "ms": 1000.0}]
        lines = W.round_start_check(hit_rows(1.009), chords)
        self.assertTrue(lines[0].startswith("PASS"))
        self.assertIn("(1 chords outside the log's clock span)", lines[0])


class PauseCheckTest(unittest.TestCase):
    def test_a_2_s_wall_gap_with_a_held_clock_reports_the_span(self):
        rows = [row(10.0, 5.0), row(10.5, 5.4, round_start_s=5.39),
                row(12.5, 5.41, round_start_s=5.39),
                row(12.6, 5.5, round_start_s=5.39)]
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
        values = [1000.25, 1.0, 0, 7680, 4294974976, 6144, 8589940736,
                  4294970368, 1, 257, 8, 520, 61.25, 1, 0.995, 1.5, 1.25, 0.125]
        self.assertEqual(len(values), len(WATCH_COLUMNS))
        path = self.write_csv(WATCH_COLUMNS, [values])
        got = W.load_watch_rows(path)
        self.assertEqual(got, [dict(zip(WATCH_COLUMNS, values))])
        self.assertIsInstance(got[0]["sp_amount_u64"], int)
        self.assertEqual(got[0]["sp_half_u64"], 4294970368)

    def test_a_log_written_before_the_half_column_loads_with_it_nan(self):
        # A log with the clock columns but no sp_half_u64, as sp_watch.py
        # wrote before the half column existed.
        old = [c for c in WATCH_COLUMNS if c not in WATCH_HALF_COLUMNS]
        path = self.write_csv(old, [list(range(len(old)))])
        got = W.load_watch_rows(path)
        self.assertEqual(set(got[0]), set(WATCH_COLUMNS))
        self.assertTrue(math.isnan(got[0]["sp_half_u64"]))
        self.assertEqual(got[0]["corr_s"], old.index("corr_s"))
        self.assertEqual(W.pause_check(got), ["pause: no pause in this log"])

    def test_a_log_without_the_clock_or_half_columns_loads(self):
        path = self.write_csv(WATCH_COLUMNS_WITHOUT_LATE_GROUPS,
                              [[7] * len(WATCH_COLUMNS_WITHOUT_LATE_GROUPS)])
        got = W.load_watch_rows(path)
        self.assertTrue(all(math.isnan(got[0][c])
                            for c in WATCH_CLOCK_COLUMNS + WATCH_HALF_COLUMNS))

    def test_a_header_missing_only_part_of_a_group_is_refused(self):
        path = self.write_csv([c for c in WATCH_COLUMNS if c != "raw_s"],
                              [[1] * (len(WATCH_COLUMNS) - 1)])
        with self.assertRaises(ValueError):
            W.load_watch_rows(path)

    def test_a_log_without_the_clock_columns_loads_with_them_nan(self):
        old = [c for c in WATCH_COLUMNS if c not in ("audio_s", "raw_s", "corr_s")]
        path = self.write_csv(old, [[7] * len(old)])
        got = W.load_watch_rows(path)
        self.assertEqual(got[0]["score"], 7)
        self.assertTrue(all(math.isnan(got[0][c])
                            for c in ("audio_s", "raw_s", "corr_s")))
        self.assertEqual(W.pause_check(got), ["pause: no pause in this log"])

    def test_a_saved_log_with_the_older_hit_time_s_header_loads(self):
        old = ["hit_time_s" if c == "round_start_s" else c
               for c in WATCH_COLUMNS_WITHOUT_LATE_GROUPS]
        values = list(range(len(old)))
        path = self.write_csv(old, [values])
        got = W.load_watch_rows(path)
        self.assertEqual(got[0]["round_start_s"], old.index("hit_time_s"))
        self.assertNotIn("hit_time_s", got[0])

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
        self.assertTrue(lines[1].startswith("PASS +0x2E0"))
        self.assertEqual(lines[-1], "pause: no pause in this log")


if __name__ == "__main__":
    unittest.main()
