"""Tests for watch_checks.py: the pause check on a Star Power watcher log. The
tests read real watcher logs under testdata/ch_runs; no game."""

from __future__ import annotations

import csv
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


_RUNS_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs")

# The end of a real Star Power watch with every column: the last three reads
# before the song ended, and the final row the watcher wrote once the clock
# had held. Copied from lines 1 and 50242-50245 of
# hydra-data\ch-mcp-runs\2026-11-13\pitch-dark-ratios-run1\sp_watch-1.csv.
FROZEN_END_CSV = os.path.join(_RUNS_DIR, "pitch-dark-ratios-run1",
                              "sp_watch-1-frozen-end.csv")

# The end of a real watch written before the half column existed: the last
# two reads and the final row. Copied unchanged from lines 1 and 50641-50643
# of hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run1\sp_watch-1.csv.
NO_HALF_END_CSV = os.path.join(_RUNS_DIR, "ls-r1-run1", "sp_watch-1-tail.csv")

# The first three rows of a real watch written before the clock and half
# columns existed, which also calls round_start_s by its older name. Copied
# unchanged from hydra-data\ch-mcp-runs\2026-11-06\ls-d2b-run1\sp_watch-1.csv.
NO_CLOCK_HEAD_CSV = os.path.join(_RUNS_DIR, "ls-d2b-run1", "sp_watch-1-head.csv")

# The first three rows of a real watch written before the half column existed.
# Copied unchanged from hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run1\sp_watch-1.csv.
NO_HALF_HEAD_CSV = os.path.join(_RUNS_DIR, "ls-r1-run1", "sp_watch-1-head.csv")

# Seven rows of a real watch with every column, from
# hydra-data\ch-mcp-runs\2026-11-12\ls-g3-run1\sp_watch-1.csv (lines 2 and
# 20842-20847).
FULL_ROWS_CSV = os.path.join(_RUNS_DIR, "ls-g3-run1", "sp_watch-1.csv")


class PauseCheckTest(unittest.TestCase):
    def test_the_held_clock_at_a_real_logs_end_is_one_pause_with_its_span(self):
        rows = W.load_watch_rows(FROZEN_END_CSV)
        a, b = rows[-2], rows[-1]
        self.assertGreater(b["wall_s"] - a["wall_s"], W.PAUSE_MIN_WALL_S)
        self.assertEqual(W.find_pauses(rows), [(len(rows) - 2, len(rows) - 1)])
        lines = W.pause_check(rows)
        self.assertEqual(len(lines), 2)
        self.assertTrue(lines[0].startswith("PASS pause: "), lines[0])
        for value in (a["wall_s"], b["wall_s"], a["clock_s"], b["clock_s"]):
            self.assertIn(f"{value:.3f}", lines[0])
        self.assertEqual(lines[1], "  other fields: none moved")

    def test_a_pause_into_the_final_row_says_so(self):
        rows = W.load_watch_rows(FROZEN_END_CSV)
        end_note = "(ends at the log's final row, where the watcher stopped)"
        self.assertTrue(W.pause_check(rows)[0].endswith(end_note))
        # Without the watcher's final row the same reads hold no pause.
        self.assertEqual(W.pause_check(rows[:-1]), ["pause: no pause in this log"])

    def test_a_nan_read_held_across_the_pause_is_not_a_move(self):
        # This log has no half column, so sp_half_u64 loads NaN in every row,
        # on both sides of the watcher's final pause.
        rows = W.load_watch_rows(NO_HALF_END_CSV)
        self.assertTrue(all(math.isnan(r["sp_half_u64"]) for r in rows[-2:]))
        lines = W.pause_check(rows)
        self.assertTrue(lines[0].startswith("PASS pause: "), lines[0])
        self.assertEqual(lines[1], "  other fields: none moved")


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

    @staticmethod
    def header_and_cells(path):
        with open(path, newline="", encoding="utf-8") as f:
            reader = csv.reader(f)
            return next(reader), list(reader)

    def test_whole_number_cells_load_as_ints_with_every_digit(self):
        # sp_watch.py writes a 64-bit field as its whole number; a float would
        # drop low bits once a value passes 2**53, so such cells load as ints.
        header, cells = self.header_and_cells(FULL_ROWS_CSV)
        self.assertEqual(tuple(header), WATCH_COLUMNS)
        got = W.load_watch_rows(FULL_ROWS_CSV)
        self.assertEqual(len(got), len(cells))
        for loaded, texts in zip(got, cells):
            for column, text in zip(header, texts):
                value = loaded[column]
                if "." in text or "e" in text:
                    self.assertIsInstance(value, float, column)
                    self.assertEqual(value, float(text), column)
                else:
                    self.assertIsInstance(value, int, column)
                    self.assertEqual(str(value), text, column)
        for column in ("sp_amount_u64", "sp_max_u64", "sp_half_u64"):
            self.assertIsInstance(got[0][column], int, column)

    def test_a_log_written_before_the_half_column_loads_with_it_nan(self):
        header, _ = self.header_and_cells(NO_HALF_HEAD_CSV)
        self.assertNotIn("sp_half_u64", header)
        got = W.load_watch_rows(NO_HALF_HEAD_CSV)
        self.assertEqual(set(got[0]), set(WATCH_COLUMNS))
        self.assertTrue(all(math.isnan(r["sp_half_u64"]) for r in got))
        # The clock columns the log did write keep their reads.
        self.assertFalse(any(math.isnan(r[c]) for r in got
                             for c in WATCH_CLOCK_COLUMNS))
        self.assertEqual(W.pause_check(got), ["pause: no pause in this log"])

    def test_a_log_without_the_clock_or_half_columns_loads(self):
        header, _ = self.header_and_cells(NO_CLOCK_HEAD_CSV)
        self.assertFalse(set(header) & set(WATCH_CLOCK_COLUMNS + WATCH_HALF_COLUMNS))
        got = W.load_watch_rows(NO_CLOCK_HEAD_CSV)
        self.assertTrue(all(math.isnan(r[c]) for r in got
                            for c in WATCH_CLOCK_COLUMNS + WATCH_HALF_COLUMNS))
        self.assertFalse(any(math.isnan(r[c]) for r in got
                             for c in WATCH_COLUMNS_WITHOUT_LATE_GROUPS))

    def test_a_header_missing_only_part_of_a_group_is_refused(self):
        # A real full log with only raw_s, one of the three clock columns,
        # taken out of its header and rows.
        header, cells = self.header_and_cells(FULL_ROWS_CSV)
        drop = header.index("raw_s")
        path = self.write_csv(header[:drop] + header[drop + 1:],
                              [c[:drop] + c[drop + 1:] for c in cells])
        with self.assertRaises(ValueError):
            W.load_watch_rows(path)

    def test_a_saved_log_with_the_older_hit_time_s_header_loads(self):
        # The real ls-d2b log calls round_start_s by its older name.
        header, cells = self.header_and_cells(NO_CLOCK_HEAD_CSV)
        self.assertIn("hit_time_s", header)
        self.assertNotIn("round_start_s", header)
        got = W.load_watch_rows(NO_CLOCK_HEAD_CSV)
        old = header.index("hit_time_s")
        for loaded, texts in zip(got, cells):
            self.assertEqual(loaded["round_start_s"], W.parse_value(texts[old]))
            self.assertNotIn("hit_time_s", loaded)

    def test_run_reads_the_log_and_reports_its_pauses(self):
        lines = W.run([FULL_ROWS_CSV])
        rows = W.load_watch_rows(FULL_ROWS_CSV)
        self.assertEqual(lines[0],
                         f"{os.path.basename(FULL_ROWS_CSV)}: {len(rows)} rows")
        self.assertEqual(lines[1:], W.pause_check(rows))


if __name__ == "__main__":
    unittest.main()
