"""The measured hit-window cap and floor have one home.

constants.py holds them with their evidence; the probe scripts read them.
No game needed: the committed poll_windows.csv is the measurement.
"""

from __future__ import annotations

import csv
import os
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_chart  # noqa: E402
from tools.ch_probe import probe_songs as P  # noqa: E402
from tools.ch_probe.experiments import watch_window as WW  # noqa: E402

_CSV = os.path.join(_REPO_ROOT, "tools", "ch_probe", "experiments", "results",
                    "poll_windows.csv")


def measured_windows() -> list[float]:
    with open(_CSV, newline="", encoding="utf-8") as f:
        return [float(r["total_window_ms"]) for r in csv.DictReader(f)]


class MeasuredConstantsTest(unittest.TestCase):
    def test_cap_and_floor_match_the_committed_measurement(self):
        w = measured_windows()
        self.assertLess(abs(max(w) - C.WINDOW_CAP_MS), C.WINDOW_MATCH_TOLERANCE_MS)
        self.assertLess(abs(min(w) - C.WINDOW_FLOOR_MS), C.WINDOW_MATCH_TOLERANCE_MS)

    def test_derived_rules_come_from_the_per_side_constants(self):
        # Pinned values, not the formulas: half the cap, and the two gap edges.
        self.assertAlmostEqual(C.ONE_SIDE_CAP_MS, 85.71565, places=9)
        self.assertAlmostEqual(C.CAP_FROM_GAP_MS, 170.0, places=9)
        self.assertAlmostEqual(C.FLOOR_UP_TO_GAP_MS, 75.0, places=9)


class ProbeSettingsHaveOneHomeTest(unittest.TestCase):
    def test_probe_chart_spacings_and_note_come_from_constants(self):
        self.assertEqual(tuple(probe_chart.DEFAULT_SPACINGS_MS), tuple(C.PROBE_SPACINGS_MS))
        self.assertEqual(probe_chart.DRUM_NOTE_KICK, C.PROBE_CHART_NOTE_KICK)
        self.assertEqual(C.PROBE_CHART_NOTE_KICK, 0)   # a .chart note, not an input lane

    def test_normal_back_ms_is_derived_from_the_seconds_value(self):
        # constants.py derives the ms edges from the seconds values the game
        # stores; these pin what that gives.
        self.assertEqual(C.EXPECT_NORMAL_BACK_MS, 85.0)
        self.assertEqual(C.EXPECT_PRECISION_BACK_MS, 40.0)
        self.assertEqual(C.s_to_ms(0.085), 85.0)


class PollVerdictTest(unittest.TestCase):
    def test_the_real_capped_run_reads_as_capped_not_unclamped(self):
        lines = WW.window_verdict(measured_windows(), back_ms=85.0)
        text = "\n".join(lines)
        self.assertNotIn("NO CLAMP", text)
        self.assertIn("reached the measured cap", text)
        self.assertIn("reached the measured floor", text)

    def test_a_reading_past_the_cap_is_no_clamp(self):
        lines = WW.window_verdict([100.0, C.WINDOW_CAP_MS + 1.0], back_ms=85.0)
        self.assertTrue(any("NO CLAMP" in l for l in lines))

    def test_precision_mode_says_the_cap_is_unknown(self):
        lines = WW.window_verdict([60.0, 70.0], back_ms=40.0)
        self.assertTrue(any("no measured cap" in l for l in lines))


class WatchFloorTest(unittest.TestCase):
    def test_floor_runs_are_checked_against_the_measured_floor(self):
        notes = P.manifest("x", P.window_map().notes)["notes"]
        samples = [WW.Sample(0.0, 0.0, 0, 0.0)] + [
            WW.Sample(float(n["time_ms"]),
                      C.WINDOW_FLOOR_MS if (n["gap_after_ms"] or 999) <= 75 else 100.0, 0, 0.0)
            for n in notes]
        lines = WW.window_report(samples, notes)
        self.assertEqual(sum("matches the floor" in l for l in lines), 4)


if __name__ == "__main__":
    unittest.main()
