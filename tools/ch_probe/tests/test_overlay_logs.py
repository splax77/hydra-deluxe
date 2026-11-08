"""Pins the overlay spike's two CSV column lists as literals, so a change to
either list is a deliberate edit here too.

Run from the repo root with:
    py -m unittest tools.ch_probe.tests.test_overlay_logs
"""

from __future__ import annotations

import os
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import overlay_logs  # noqa: E402


class ColumnListsTest(unittest.TestCase):
    def test_watch_columns(self):
        self.assertEqual(overlay_logs.WATCH_COLUMNS, (
            "wall_s", "clock_s", "score", "sp_amount_u32", "sp_amount_u64",
            "sp_max_u32", "sp_max_u64", "sp_active_byte", "sp_active_u32",
            "sp_mult_byte", "sp_mult_u32", "sp_end_s", "sp_overflow_u32",
            "round_start_s", "audio_s", "raw_s", "corr_s"))

    def test_the_saved_watch_logs_headers_read_as_watch_columns(self):
        # The saved logs keep their own header, hit_time_s included; read
        # through watch_header, each must be one the readers accept.
        folder = os.path.join(_REPO_ROOT, "tools", "ch_probe", "experiments",
                              "results", "overlay-spike")
        names = sorted(n for n in os.listdir(folder) if n.endswith("_sp_watch.csv"))
        self.assertEqual(len(names), 4)
        for name in names:
            with open(os.path.join(folder, name), encoding="utf-8") as f:
                header = tuple(f.readline().strip().split(","))
            self.assertIn("hit_time_s", header, name)
            self.assertEqual(overlay_logs.watch_header(header),
                             overlay_logs.WATCH_COLUMNS_WITHOUT_CLOCK, name)

    def test_press_log_columns(self):
        self.assertEqual(overlay_logs.PRESS_LOG_COLUMNS, (
            "index", "chart_ms", "clock_ms_at_press", "score_before",
            "score_after", "hit", "down_frame", "up_frame", "down_ms", "up_ms",
            "wall_s_at_read", "wall_s_at_reply", "scored_between",
            "end_read_ms", "probe"))

    def test_the_press_columns_before_the_wall_times(self):
        self.assertEqual(overlay_logs.PRESS_COLUMNS_WITHOUT_WALL, (
            "index", "chart_ms", "clock_ms_at_press", "score_before",
            "score_after", "hit", "down_frame", "up_frame", "down_ms", "up_ms"))


if __name__ == "__main__":
    unittest.main()
