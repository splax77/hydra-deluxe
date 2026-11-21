"""The measured hit-window cap and floor have one home.

constants.py holds them with their evidence; the probe scripts read them.
No game needed. Two real sources stand behind the numbers: the committed
poll_windows.csv, a live run's stored windows (docs/clone-hero/03-timing.md
TM-23), and Hydra's own engine, through the hydra_replay schedule files under
testdata/replay, whose "constants" block and per-chord "side_ms" come from
src/core/note_window.h (TM-18 to TM-21).
"""

from __future__ import annotations

import csv
import json
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
from tools.ch_probe.tests.chart_reader import drum_notes  # noqa: E402

_CSV = os.path.join(_REPO_ROOT, "tools", "ch_probe", "experiments", "results",
                    "poll_windows.csv")
_REPLAY_DIR = os.path.join(_REPO_ROOT, "testdata", "replay")
# The Window Map song folder from the 2026-10-28 l3 run: the chart and the
# manifest the probe wrote and the game played.
_WINDOW_MAP_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "2026-10-28-l3",
                               "Window Map")
# Hydra's schedule for Pitch Dark, path 0 (tools/ch_probe/prep_inputs.py made
# it; see the comment above _REPLAY_DIR in test_sp_compare.py).
_PITCH_DARK_SCHEDULE = os.path.join(_REPLAY_DIR, "test_pitch_dark_ratio-schedule.json")
# Hydra's score JSON for Window Map, path 0 (test_score_compare.py names the
# command that made it).
_WINDOW_MAP_SCORE = os.path.join(_REPLAY_DIR, "window_map-score.json")
# Hydra's schedule for Window Map, path 0. From the repo root:
#   build-cpp\Release\hydra_replay.exe schedule
#       --chart "testdata/ch_runs/2026-10-28-l3/Window Map/notes.chart"
#       --path testdata/replay/window_map-dump.json --index 0
#       --out testdata/replay/window_map-schedule.json
_WINDOW_MAP_SCHEDULE = os.path.join(_REPLAY_DIR, "window_map-schedule.json")


def measured_windows() -> list[float]:
    with open(_CSV, newline="", encoding="utf-8") as f:
        return [float(r["total_window_ms"]) for r in csv.DictReader(f)]


def read_json(path: str) -> dict:
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def engine_whole_windows(schedule: dict) -> list[float]:
    """Each chord's whole window as Hydra's engine works it out. A note's side
    is half the formula's output (TM-20), and the stored whole window is twice
    the side (TM-23)."""
    return [2 * c["side_ms"] for c in schedule["chords"]]


class MeasuredConstantsTest(unittest.TestCase):
    def test_cap_and_floor_match_the_committed_measurement(self):
        tol = C.WINDOW_MATCH_TOLERANCE_MS
        # The live run's stored windows (TM-23).
        w = measured_windows()
        self.assertLess(abs(max(w) - C.WINDOW_CAP_MS), tol)
        self.assertLess(abs(min(w) - C.WINDOW_FLOOR_MS), tol)
        # Hydra's engine on a real chart reaches the same two ends, and never
        # goes past them.
        e = engine_whole_windows(read_json(_PITCH_DARK_SCHEDULE))
        self.assertLess(abs(max(e) - C.WINDOW_CAP_MS), tol)
        self.assertLess(abs(min(e) - C.WINDOW_FLOOR_MS), tol)

    def test_derived_rules_come_from_the_per_side_constants(self):
        # TM-19: each half-gap is held between the front and the back, and
        # TM-20 sums the two halves. So the summed gap tops out at twice the
        # back (the cap's edge) and bottoms out at twice the front (the
        # floor's), with the front and back Hydra's engine uses.
        engine = read_json(_PITCH_DARK_SCHEDULE)["constants"]
        self.assertAlmostEqual(C.CAP_FROM_GAP_MS, 2 * engine["window_back_ms"], places=9)
        self.assertAlmostEqual(C.FLOOR_UP_TO_GAP_MS, 2 * engine["window_front_ms"], places=9)


class ProbeSettingsHaveOneHomeTest(unittest.TestCase):
    def test_probe_chart_spacings_and_note_come_from_constants(self):
        self.assertEqual(tuple(probe_chart.DEFAULT_SPACINGS_MS), tuple(C.PROBE_SPACINGS_MS))
        self.assertEqual(probe_chart.DRUM_NOTE_KICK, C.PROBE_CHART_NOTE_KICK)

    def test_the_probe_kick_note_is_read_as_the_kick(self):
        # The Window Map chart the probe wrote holds only the probe's kick
        # note, and Hydra's parser reads every one of its notes as the kick
        # (NL-03 in docs/clone-hero/02-note-list.md: N 0 is the kick).
        with open(os.path.join(_WINDOW_MAP_DIR, "notes.chart"), encoding="utf-8") as f:
            chart = drum_notes(f.read())
        self.assertTrue(chart)
        self.assertEqual({note for _tick, note, _sustain in chart}, {C.PROBE_CHART_NOTE_KICK})
        score = read_json(_WINDOW_MAP_SCORE)
        colors = [n["color"] for chord in score["chords"] for n in chord["notes"]]
        self.assertEqual(len(colors), len(chart))
        self.assertEqual(set(colors), {"Kick"})

    def test_normal_back_ms_is_derived_from_the_seconds_value(self):
        # TM-18: the game stores the normal back as 0.085 s and the front as
        # 0.0375 s. The ms values constants.py derives from them are the ones
        # Hydra's engine uses.
        engine = read_json(_PITCH_DARK_SCHEDULE)["constants"]
        self.assertAlmostEqual(C.EXPECT_NORMAL_BACK_MS, engine["window_back_ms"], places=9)
        self.assertAlmostEqual(C.s_to_ms(C.EXPECT_NORMAL_FRONT_S), engine["window_front_ms"],
                               places=9)


class PollVerdictTest(unittest.TestCase):
    def assert_names_the_measured_ends(self, lines: list[str]) -> None:
        self.assertIn(str(C.WINDOW_CAP_MS), lines[0])
        self.assertIn(str(C.WINDOW_FLOOR_MS), lines[0])

    def test_the_real_capped_run_reads_as_capped_not_unclamped(self):
        w = measured_windows()
        tol = C.WINDOW_MATCH_TOLERANCE_MS
        # The facts the verdict must report, read from the run itself.
        self.assertLessEqual(abs(max(w) - C.WINDOW_CAP_MS), tol)
        self.assertLessEqual(abs(min(w) - C.WINDOW_FLOOR_MS), tol)
        lines = WW.window_verdict(w, back_ms=C.EXPECT_NORMAL_BACK_MS)
        self.assert_names_the_measured_ends(lines)
        text = "\n".join(lines)
        self.assertNotIn("NO CLAMP", text)
        self.assertIn("reached the measured cap", text)
        self.assertIn("reached the measured floor", text)

    def test_a_reading_past_the_cap_is_no_clamp(self):
        # The real run, plus one reading just past the cap's tolerance.
        past = C.WINDOW_CAP_MS + 2 * C.WINDOW_MATCH_TOLERANCE_MS
        lines = WW.window_verdict(measured_windows() + [past], back_ms=C.EXPECT_NORMAL_BACK_MS)
        self.assert_names_the_measured_ends(lines)
        flagged = [l for l in lines if "NO CLAMP" in l]
        self.assertEqual(len(flagged), 1)
        self.assertIn(f"{past:.3f}", flagged[0])
        self.assertFalse(any("reached the measured cap" in l for l in lines))

    def test_precision_mode_says_the_cap_is_unknown(self):
        # TM-18: precision mode's back is 0.04 s; nobody has read its cap.
        back = C.s_to_ms(C.EXPECT_PRECISION_BACK_S)
        lines = WW.window_verdict(measured_windows(), back_ms=back)
        self.assertEqual(len(lines), 1)
        self.assertIn(f"{back:.1f} ms", lines[0])
        self.assertIn("precision mode", lines[0])
        self.assertIn("no measured cap", lines[0])


class WatchFloorTest(unittest.TestCase):
    def test_floor_runs_are_checked_against_the_measured_floor(self):
        # The Window Map manifest the run used, and Hydra's engine window at
        # each of its chords as the samples, so the windows come from the
        # engine's rule, not from the one window_report checks.
        notes = read_json(os.path.join(_WINDOW_MAP_DIR, "manifest.json"))["notes"]
        schedule = read_json(_WINDOW_MAP_SCHEDULE)
        samples = [WW.Sample(float(c["ms"]), w, 0, 0.0)
                   for c, w in zip(schedule["chords"], engine_whole_windows(schedule))]
        self.assertEqual(len(samples), len(notes))
        lines = WW.window_report(samples, notes)
        matches = WW._verdict([C.WINDOW_FLOOR_MS], C.WINDOW_FLOOR_MS, "floor")
        floor_blocks = sorted({n["block"] for n in notes if n["block"].startswith("floor_")})
        # Every floor block the probe lays out has gaps narrow enough for
        # the floor, so each one reads it.
        self.assertEqual(len(floor_blocks), len(P.FLOOR_GAPS_MS))
        self.assertEqual(sum(l.endswith(matches) for l in lines), len(floor_blocks))


if __name__ == "__main__":
    unittest.main()
