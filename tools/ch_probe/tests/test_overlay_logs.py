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
            "hit_time_s"))

    def test_press_log_columns(self):
        self.assertEqual(overlay_logs.PRESS_LOG_COLUMNS, (
            "index", "chart_ms", "clock_ms_at_press", "score_before",
            "score_after", "hit"))


if __name__ == "__main__":
    unittest.main()
